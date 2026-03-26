#include <QTranslator>
#include "Mainwindow.h"
#include "SingleApplication.h"
#include <QImageWriter>

int main(int argc, char *argv[]) {
    qDebug() << QImageWriter::supportedImageFormats();
    SingleApplication a(argc, argv, "__TcpConnectorsByVarian__");
    QTranslator translator;
    if (!translator.load(":/language/mylang.qm")) {
        return 1;
    }
    QApplication::installTranslator(&translator);

    if (a.isRunning()) return 0;

    auto *mainForm = new MainWindow;
    a.setWidget(mainForm);
    return QApplication::exec();
}
