//
// Created by admin on 2026/3/2.
//

// You may need to build the project (run Qt uic code generator) to get "ui_MainWindow.h" resolved

#include "ui_MainWindow.h"
#include "Mainwindow.h"

#include <QLockFile>
#include <QPushButton>
#include <QMenu>
#include <QFileDialog>
#include <QMessageBox>
#include "Taskdialog.h"

MainWindow::MainWindow(QWidget *parent) : QWidget(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);

    setWindowFlags(Qt::WindowMinMaxButtonsHint);

    setAttribute(Qt::WA_DeleteOnClose);

    ui->label->setText(initialDir);

    initTrayIcon();
}

MainWindow::~MainWindow() {
    delete ui;

    delete trayIcon;
    delete taskDialog;
}

void MainWindow::buttonClicked(QAbstractButton *button) {
    if (button == ui->buttonBox->button(QDialogButtonBox::Close)) {
        close();
    } else if (button == ui->buttonBox->button(QDialogButtonBox::Ok)) {
        qDebug() << "OK BUTTON CLICK";
        startTask();
    }
}

void MainWindow::trayIconActive(QSystemTrayIcon::ActivationReason reason) {
    switch (reason) {
        case QSystemTrayIcon::DoubleClick:
            showNormal();
            activateWindow();
            break;
        default: ;
    }
}

void MainWindow::dirSelect() {
    QString dirPath = QFileDialog::getExistingDirectory(this, tr("选择文件夹"), initialDir);
    if (dirPath.isEmpty()) {
        return;
    }
    qDebug() << "用户选择的目录：" << dirPath;

    ui->label->setText(dirPath);
};

void MainWindow::initTrayIcon() {
    trayIcon = new QSystemTrayIcon(this);
    trayIcon->setIcon(QIcon(":/Filosofem.png"));

    trayIcon->setToolTip(windowTitle());
    trayIcon->show();

    auto *quitAction = new QAction("退出", this);
    auto *restoreAction = new QAction("最大化", this);
    connect(quitAction, &QAction::triggered, this, &MainWindow::close);
    connect(restoreAction, &QAction::triggered, this, &QWidget::showNormal);

    auto *menu = new QMenu(this);
    menu->addAction(restoreAction);
    menu->addSeparator();
    menu->addAction(quitAction);
    trayIcon->setContextMenu(menu);

    connect(trayIcon, &QSystemTrayIcon::activated, this, &MainWindow::trayIconActive);
}

void MainWindow::startTask() {
    QDir dir(ui->label->text());
    QStringList filters;
    filters << "*.png";

    QFileInfoList fileInfo = dir.entryInfoList(filters, QDir::Files | QDir::NoSymLinks);
    if (fileInfo.isEmpty()) {
        QMessageBox::critical(this, tr("错误"), tr("无需要处理的图片（暂只支持png）"), QMessageBox::Ok);
        return;
    }

    taskDialog = new TaskDialog(fileInfo, nullptr);
    taskDialog->show();
    connect(taskDialog, &TaskDialog::destroyed, this, [&]() {
        taskDialog = nullptr;
    });
}

void MainWindow::changeEvent(QEvent *event) {
    if (event->type() == QEvent::WindowStateChange && windowState() == Qt::WindowMinimized) {
        trayIcon->showMessage("托盘", "dsadas");
    }
}
