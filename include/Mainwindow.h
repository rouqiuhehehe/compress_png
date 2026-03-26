//
// Created by admin on 2026/3/2.
//

#ifndef DECRYPTIONQT_MAINWINDOW_H
#define DECRYPTIONQT_MAINWINDOW_H

#include <QAbstractButton>
#include <QFileInfo>
#include <QWidget>
#include <QSystemTrayIcon>
#include <QStandardPaths>

QT_BEGIN_NAMESPACE

namespace Ui {
    class MainWindow;
}

QT_END_NAMESPACE

class MainWindow final : public QWidget {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

    ~MainWindow() override;

protected:
    void changeEvent(QEvent* event) override;

private slots:
    void buttonClicked(QAbstractButton *);
    void trayIconActive(QSystemTrayIcon::ActivationReason reason);
    void dirSelect();

private:
    void initTrayIcon();
    void startTask();

    QSystemTrayIcon* trayIcon{};

    Ui::MainWindow *ui;
    class TaskDialog *taskDialog;
    // QString initialDir = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
    QString initialDir = "D:\\testimg";
};


#endif //DECRYPTIONQT_MAINWINDOW_H