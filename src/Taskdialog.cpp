//
// Created by admin on 2026/3/23.
//

// You may need to build the project (run Qt uic code generator) to get "ui_TaskDialog.h" resolved

#include "Taskdialog.h"
#include "ui_TaskDialog.h"
#include <QtConcurrent>
#include "libimagequant.h"
#include <utility>
#include <QFutureWatcher>

#define FUN_ERROR(fnName) qDebug() << #fnName " 执行失败"
#define TASK_TIME(NAME, BEGIN)    \
            qDebug() << "[" << path << "]"  \
                     << NAME" used time : "   \
                     << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - BEGIN)
#define TASK_TIME_WITHOUT_PATH(NAME, BEGIN) \
            qDebug() << NAME" used time : "   \
                     << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - BEGIN)

template<typename T, void(*Deleter)(T *)>
struct LiqPtr : std::unique_ptr<T, decltype(Deleter)> {
    explicit LiqPtr(T *ptr = nullptr)
        : std::unique_ptr<T, decltype(Deleter)>(ptr, Deleter) {}
};

using liq_attr_ptr = LiqPtr<liq_attr, liq_attr_destroy>;
using liq_image_ptr = LiqPtr<liq_image, liq_image_destroy>;
using liq_result_ptr = LiqPtr<liq_result, liq_result_destroy>;

liq_result_ptr liq_image_quantize(liq_image *image, liq_attr *attr) {
    liq_result *res = nullptr;
    liq_error err = liq_image_quantize(image, attr, &res);

    if (err != LIQ_OK || !res) {
        qDebug() << "quantize failed, err =" << err;
        return liq_result_ptr(nullptr);
    }
    return liq_result_ptr(res);
}

using namespace std::chrono_literals;

void reduceAlphaPrecision(QImage &img, int step = 8) {
    // step = 8 → 256 / 8 = 32 levels
    // step = 16 → 16 levels（更狠）

    if (img.format() != QImage::Format_RGBA8888)
        img = img.convertToFormat(QImage::Format_RGBA8888);

    for (int y = 0; y < img.height(); ++y) {
        QRgb *line = reinterpret_cast<QRgb *>(img.scanLine(y));

        for (int x = 0; x < img.width(); ++x) {
            int r = qRed(line[x]);
            int g = qGreen(line[x]);
            int b = qBlue(line[x]);
            int a = qAlpha(line[x]);

            // ===== 🔥 关键优化（避免边缘损伤）=====
            if (a == 0 || a == 255) {
                // 完全透明 / 完全不透明 → 不动
                continue;
            }

            // ===== 🔥 量化 alpha =====
            int newA = (a / step) * step;

            // 防止变成 0（导致边缘断裂）
            if (newA == 0) newA = step;

            line[x] = qRgba(r, g, b, newA);
        }
    }
}

TaskDialog::TaskDialog(QFileInfoList fileInfoList, QWidget *parent)
    : QWidget(parent), ui(new Ui::TaskDialog), fileInfoList(std::move(fileInfoList)) {
    ui->setupUi(this);

    QThreadPool::globalInstance()->setMaxThreadCount(QThread::idealThreadCount());

    setAttribute(Qt::WA_DeleteOnClose);
    ui->pushButton->hide();
    ui->listWidget->clear();
    ui->listWidget->setAttribute(Qt::WA_TransparentForMouseEvents);
    setWindowTitle(tr("执行中..."));
}

TaskDialog::~TaskDialog() {
    delete ui;
}

void TaskDialog::runTask() {
    ui->progressBar->setFormat(tr("%v/%1").arg(fileInfoList.size()));
    ui->progressBar->setValue(0);
    ui->progressBar->setMaximum(fileInfoList.size());

    auto begin_main = std::chrono::system_clock::now();
    auto uiInfo = std::make_shared<UiInfo>();
    auto future = QtConcurrent::map(fileInfoList, [&, uiInfo](const QFileInfo &fileInfo) {
        auto path = fileInfo.absoluteFilePath();
        qDebug() << "开始压缩：" << path << ".....";
        QImage image(path);
        if (image.isNull()) {
            FUN_ERROR(QImage);
            ++uiInfo->fail;
            return;
        }
        uiInfo->originTotalSize += fileInfo.size();
        image = image.convertToFormat(QImage::Format_RGBA8888);
        image.detach();
        if (!compressWithImagequant(path, image.bits(), image.width(), image.height())) {
            ++uiInfo->fail;
            return;
        }

        if (!compressWithOxipng(path)) {
            FUN_ERROR(compressWithOxipng);
            ++uiInfo->fail;
            return;
        }
        auto begin_png2webp = std::chrono::system_clock::now();
        QImage img(path);
        QString newPath = fileInfo.absolutePath() + QDir::separator() + fileInfo.baseName() + ".webp";
        if (!img.save(newPath, "WEBP", 80)) {
            FUN_ERROR("png 2 webp ");
            ++uiInfo->fail;
            return;
        }
        TASK_TIME("png2webp", begin_png2webp);
        qDebug() << path << " 压缩成功!";
        ++uiInfo->success;

        QFileInfo info(path);
        uiInfo->totalSize += info.size();
    });

    auto watcher = new QFutureWatcher<void>(this);
    connect(watcher, &QFutureWatcher<void>::finished, this, [&, uiInfo, watcher, begin_main]() {
        finishTask(uiInfo, std::chrono::duration_cast<std::chrono::milliseconds>(
                       std::chrono::system_clock::now() - begin_main));
        watcher->deleteLater();
        TASK_TIME_WITHOUT_PATH("total", begin_main);
    });
    connect(watcher, &QFutureWatcher<void>::progressValueChanged, ui->progressBar, &QProgressBar::setValue);
    watcher->setFuture(future);
}

void TaskDialog::showEvent(QShowEvent *event) {
    QWidget::showEvent(event);
    QTimer::singleShot(0, this, [this]() {
        runTask();
    });
}

bool TaskDialog::compressWithImagequant(const QString &path, uchar *data, int w, int h) {
    QImage src(data, w, h, QImage::Format_RGBA8888);

    reduceAlphaPrecision(src);
    // ===== 1️⃣ imagequant 初始化 =====
    liq_attr_ptr attr(liq_attr_create());
    if (!attr) return false;

    liq_set_speed(attr.get(), 1);
    liq_set_min_opacity(attr.get(), 0);
    // liq_set_quality(attr.get(), 50, 70);
    liq_set_last_index_transparent(attr.get(), -1);

    liq_image_ptr image(
        liq_image_create_rgba(attr.get(), src.bits(), w, h, 0));

    if (!image) {
        FUN_ERROR(liq_image_create_rgba);
        return false;
    }

    // ===== 2️⃣ 尝试量化（只试安全颜色数）=====
    std::vector tryColors = {
        256
        // , 240, 224, 208, 192
    };
    std::vector dithers = {
        .0f
        // , .3f, .6f
    };

    QByteArray bestData;
    qint64 bestSize = LLONG_MAX;
    bool quantOk = false;

    auto begin = std::chrono::system_clock::now();
    for (int colors: tryColors) {
        liq_set_max_colors(attr.get(), colors);

        liq_result_ptr res = liq_image_quantize(image.get(), attr.get());

        if (!res) {
            qWarning() << path << " quantize error, colors : " << colors;
            if (colors < 256) break;
            continue; // ❗ 不满足质量，直接跳过
        }

        auto begin_dithersTest = std::chrono::system_clock::now();
        for (float d: dithers) {
            liq_set_dithering_level(res.get(), d);
            // ===== 3️⃣ 生成 Indexed8 =====
            std::vector<uint8_t> indexData(w * h);
            if (liq_write_remapped_image(res.get(), image.get(),
                                         indexData.data(), indexData.size()) != LIQ_OK)
                continue;

            const liq_palette *pal = liq_get_palette(res.get());

            QImage img(w, h, QImage::Format_Indexed8);

            QVector<QRgb> qtPalette;
            qtPalette.reserve(pal->count);

            for (int i = 0; i < static_cast<int>(pal->count); ++i) {
                const auto &[r, g, b, a] = pal->entries[i];
                qtPalette.append(qRgba(
                    r,
                    g,
                    b,
                    a
                ));
            }

            img.setColorTable(qtPalette);

            for (int y = 0; y < h; ++y) {
                memcpy(img.scanLine(y),
                       indexData.data() + y * w,
                       w);
            }
            for (int y = 0; y < h; ++y) {
                QRgb *line = reinterpret_cast<QRgb *>(src.scanLine(y));
                for (int x = 0; x < w; ++x) {
                    int a = qAlpha(line[x]);
                    a = (a / 8) * 8; // 256 → 32 levels
                    line[x] = qRgba(qRed(line[x]), qGreen(line[x]), qBlue(line[x]), a);
                }
            }

            // ===== 4️⃣ 写入内存比较大小 =====
            QByteArray buffer;
            QBuffer buf(&buffer);
            buf.open(QIODevice::WriteOnly);
            img.save(&buf, "PNG", 9);

            if (buffer.size() < bestSize) {
                bestSize = buffer.size();
                bestData = buffer;
                quantOk = true;
            }
        }
        TASK_TIME("try dithers", begin_dithersTest);
    }
    TASK_TIME("try colors", begin);

    // ===== 5️⃣ fallback（保证视觉无损）=====
    QByteArray finalData;

    if (quantOk) {
        finalData = bestData;
    } else {
        // ❗ 不适合量化 → 保持原图
        QBuffer buf(&finalData);
        buf.open(QIODevice::WriteOnly);
        src.save(&buf, "PNG");
    }

    auto begin_writeFile = std::chrono::system_clock::now();
    // ===== 6️⃣ 写文件 =====
    QFile f(path);
    if (!f.open(QIODevice::WriteOnly)) {
        FUN_ERROR(f.open(QIODevice::WriteOnly));
        return false;
    }

    f.write(finalData);
    f.close();
    TASK_TIME("write file", begin_writeFile);
    return true;
}

bool TaskDialog::compressWithOxipng(const QString &path) {
    QString exe = "oxipng.exe";
    // ===== 7️⃣ oxipng（无损榨干）=====
    QProcess p;
    QStringList args;

    args << "-o" << "3";
    args << "--strip" << "all";
    args << "-t" << "1";
    args << path;

    auto begin = std::chrono::system_clock::now();
    qDebug().noquote() << tr("[%1]").arg(path) << " start task : " << exe;
    // 设置输出通道合并，这样标准输出和错误输出都会一起捕获
    p.setProcessChannelMode(QProcess::MergedChannels);
    p.start(exe, args);

    if (!p.waitForStarted()) {
        qDebug() << "start failed";
        return false;
    }

    while (p.state() == QProcess::Running) {
        p.waitForReadyRead(100);
        auto out = p.readAll();
        if (!out.isEmpty())
            qDebug().noquote() << out;
    }

    p.waitForFinished(-1);

    TASK_TIME("run pxipng", begin);
    return p.exitCode() == 0;
}

void TaskDialog::finishTask(const std::shared_ptr<UiInfo> &uiInfo, std::chrono::milliseconds usedTime) {
    setWindowTitle(tr("压缩完成"));

    ui->pushButton->show();
    QStringList items;
    QLocale local;

    items << tr("成功数量：%1").arg(uiInfo->success)
          << tr("失败数量：%1").arg(uiInfo->fail)
          << tr("压缩前大小：%1").arg(local.formattedDataSize(uiInfo->originTotalSize))
          << tr("压缩后大小：%1").arg(local.formattedDataSize(uiInfo->totalSize))
          << tr("压缩率：%1%").arg(
          (1 - static_cast<double>(uiInfo->totalSize) / static_cast<double>(uiInfo->originTotalSize)) * 100)
          << tr("压缩总用时：%1ms").arg(usedTime.count());

    ui->listWidget->addItems(items);
    // qDebug() << "fail : " << uiInfo->fail << "\nsuccess : " << uiInfo->success << "\norigin size : " << uiInfo.
    // originTotalSize << "\ncompress size : " << uiInfo.totalSize;
}
