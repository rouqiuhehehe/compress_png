//
// Created by admin on 2026/3/23.
//

#ifndef PNGCOMPRESS2WEBP_TASKDIALOG_H
#define PNGCOMPRESS2WEBP_TASKDIALOG_H

#include <QFileInfo>
#include <QWidget>


QT_BEGIN_NAMESPACE

namespace Ui {
    class TaskDialog;
}

QT_END_NAMESPACE

struct UiInfo {
    QAtomicInteger<size_t> success {};
    QAtomicInteger<size_t> fail {};
    QAtomicInteger<size_t> originTotalSize {};
    QAtomicInteger<size_t> totalSize {};
    std::chrono::milliseconds usedTime;
};

class TaskDialog : public QWidget {
    Q_OBJECT

public:
    explicit TaskDialog(QFileInfoList fileInfoList, QWidget *parent = nullptr);

    ~TaskDialog() override;

    void runTask();

protected:
    void showEvent(QShowEvent *event) override;

private:
    static bool compressWithImagequant(const QString &, uchar *, int, int);

    static bool compressWithOxipng(const QString &);

    void finishTask(const std::shared_ptr<UiInfo> &uiInfo, std::chrono::milliseconds usedTime);

    Ui::TaskDialog *ui;
    QFileInfoList fileInfoList;
};


#endif //PNGCOMPRESS2WEBP_TASKDIALOG_H
