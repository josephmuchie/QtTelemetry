#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.setFixedSize(1305,950);
    w.setWindowTitle("Telemetry : Temperature");
    w.show();
    return a.exec();
}
