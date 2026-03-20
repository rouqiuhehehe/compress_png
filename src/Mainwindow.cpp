//
// Created by admin on 2026/3/2.
//

// You may need to build the project (run Qt uic code generator) to get "ui_MainWindow.h" resolved

#include "ui_MainWindow.h"
#include "Mainwindow.h"
#include <QPushButton>
#include <QMenu>

MainWindow::MainWindow(QWidget *parent) : QWidget(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);

    setWindowFlags(Qt::WindowMinMaxButtonsHint);

    setAttribute(Qt::WA_DeleteOnClose);

    initTrayIcon();
}

MainWindow::~MainWindow() {
    delete ui;

    delete trayIcon;
}

void MainWindow::buttonClicked(QAbstractButton *button) {
    if (button == ui->buttonBox->button(QDialogButtonBox::Close)) {
        close();
    } else if (button == ui->buttonBox->button(QDialogButtonBox::Ok)) {
        qDebug() << "OK BUTTON CLICK";
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

void MainWindow::changeEvent(QEvent *event) {
    if (event->type() == QEvent::WindowStateChange && windowState() == Qt::WindowMinimized) {
        trayIcon->showMessage("托盘", "dsadas");
    }
}
