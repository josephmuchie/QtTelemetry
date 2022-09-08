#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "qcustomplot.h"
#include <QMainWindow>
#include <QSerialPort>
#include <QByteArray>
#include <QTimer>


namespace Ui { class MainWindow; }


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void readSerial();
    void updateTemperature(QString);
    void updateTemperaturefah(QString);
    void realtimeDataSlot();

private:
    Ui::MainWindow *ui;
    QSerialPort *arduino;
    static const quint16 arduino_uno_vendor_id = 9025;
    static const quint16 arduino_uno_product_id = 67;

    QByteArray serialData;
    QString serialBuffer;
    QString parsed_data;
     QString parsed_data1;
    double temperature_value;
    double faren_value;
    QTimer dataTimer;


};
#endif // MAINWINDOW_H
