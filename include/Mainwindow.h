//
// Created by admin on 2026/3/2.
//

#ifndef DECRYPTIONQT_MAINWINDOW_H
#define DECRYPTIONQT_MAINWINDOW_H

#include <QAbstractButton>
#include <QWidget>
#include <QSystemTrayIcon>

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

private:
    void initTrayIcon();

    QSystemTrayIcon* trayIcon{};

    Ui::MainWindow *ui;
};


#endif //DECRYPTIONQT_MAINWINDOW_H