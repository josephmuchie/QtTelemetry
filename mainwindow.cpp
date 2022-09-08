#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QMessageBox>
#include <QRandomGenerator>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QSharedPointer>
#include <QTime>
#include <QTimer>
#include <QtWidgets>
#include <string>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
  ui->setupUi(this);
  ui->temp_lcdNumber->display("-------");
  arduino = new QSerialPort(this);
  serialBuffer = "";
  parsed_data = "";
  parsed_data1 = "";
  temperature_value = 0.0;
  faren_value = 0.0;

  /*
  qDebug() << "Number of availbale ports: " <<
  QSerialPortInfo::availablePorts().length(); foreach(const QSerialPortInfo
  &serialPortInfo, QSerialPortInfo::availablePorts()){ qDebug() << "Has vendor
  ID: "<< serialPortInfo.hasVendorIdentifier();
      if(serialPortInfo.hasVendorIdentifier()){
            qDebug() << "Vendor ID: "<< serialPortInfo.vendorIdentifier();
      }
      qDebug() << "Has Product ID: "<< serialPortInfo.hasProductIdentifier();
      if(serialPortInfo.hasProductIdentifier()){
            qDebug() << "Product ID: "<< serialPortInfo.productIdentifier();
      }
  }
*/
  bool arduino_is_available = false;
  QString arduino_port_name = "";
  foreach (const QSerialPortInfo &serialPortInfo,
           QSerialPortInfo::availablePorts()) {
    if (serialPortInfo.hasVendorIdentifier() &&
        serialPortInfo.hasProductIdentifier()) {
      if ((serialPortInfo.vendorIdentifier() == arduino_uno_vendor_id) &&
          (serialPortInfo.productIdentifier() == arduino_uno_product_id)) {
        arduino_is_available = true;
        arduino_port_name = serialPortInfo.portName();
      }
    }
  }

  if (arduino_is_available) {
    // open and configure the port
    qDebug() << "Arduino port has been found! \n";
    arduino->setPortName(arduino_port_name);
    arduino->open(QSerialPort::ReadOnly);
    arduino->setBaudRate(QSerialPort::Baud9600);
    arduino->setDataBits(QSerialPort::Data8);
    arduino->setParity(QSerialPort::NoParity);
    arduino->setStopBits(QSerialPort::OneStop);
    arduino->setFlowControl(QSerialPort::NoFlowControl);
    QObject::connect(arduino, SIGNAL(readyRead()), this, SLOT(readSerial()));

  } else {
    // Give error message
    QMessageBox::warning(this, "Port error", "Couldn't find the arduino ");
  }
}

MainWindow::~MainWindow() {
  if (arduino->isOpen()) {
    arduino->close();
  }
  delete ui;
}

void MainWindow::readSerial() {
  qDebug() << "reading serial";
  QStringList buffer_split = serialBuffer.split(",");
  if (buffer_split.length() < 3) {
    // no parsed value yet so continue accumulating bytes from serial in the
    // buffer.
    serialData = arduino->readAll();
    serialBuffer =
        serialBuffer + QString::fromStdString(serialData.toStdString());
    serialData.clear();
  } else {

    serialBuffer = "";
    qDebug() << buffer_split << "\n";
    parsed_data = buffer_split[1];
    temperature_value = (parsed_data.toDouble()); // convert to double
    faren_value = (9 / 5.0) * (parsed_data.toDouble()) + 32;
    qDebug() << "Temperature: " << temperature_value << "\n";
    parsed_data = QString::number(
        temperature_value, 'g',
        4);
    parsed_data1 = QString::number(faren_value, 'g', 4);
    MainWindow::updateTemperature(parsed_data);
    MainWindow::updateTemperaturefah(parsed_data1);

    ui->customPlot->addGraph(); // blue line
    ui->customPlot->graph(0)->setPen(QPen(QColor(40, 110, 255)));
    ui->customPlot->graph(0)->setLineStyle(QCPGraph::lsLine);

    ui->customPlot->addGraph(); // red line
    ui->customPlot->graph(1)->setPen(QPen(QColor(255, 110, 40)));

    QSharedPointer<QCPAxisTickerTime> timeTicker(new QCPAxisTickerTime);
    timeTicker->setTimeFormat("%h:%m:%s");
    ui->customPlot->xAxis->setTicker(timeTicker);
    ui->customPlot->axisRect()->setupFullAxesBox();

    // make left and bottom axes transfer their ranges to right and top axes:
    connect(ui->customPlot->xAxis, SIGNAL(rangeChanged(QCPRange)),
            ui->customPlot->xAxis2, SLOT(setRange(QCPRange)));
    connect(ui->customPlot->yAxis, SIGNAL(rangeChanged(QCPRange)),
            ui->customPlot->yAxis2, SLOT(setRange(QCPRange)));

    // setup a timer that repeatedly calls MainWindow::realtimeDataSlot:

    connect(&dataTimer, SIGNAL(timeout()), this, SLOT(realtimeDataSlot()));
    dataTimer.start(0); // Interval 0 means to refresh as fast as possible
  }
}
void MainWindow::realtimeDataSlot() {

  static QTime time(QTime::currentTime());
  // calculate two new data points:
  double key = time.msecsTo(QTime::currentTime()) /
               10000.0; // time elapsed since start of demo, in seconds
  static double lastPointKey = 0;

  if (key - lastPointKey > 0.100) // at most add point every 1 s
  {
    // add data to lines:
    ui->customPlot->graph(0)->addData(key, parsed_data.toDouble());
    ui->customPlot->graph(1)->addData(key, parsed_data1.toDouble());

    ui->customPlot->graph(0)->rescaleValueAxis(true);
    ui->customPlot->graph(1)->rescaleValueAxis(true);

    qDebug() << "Key = " << key;
    qDebug() << "Parse val = " << parsed_data.toDouble();
    qDebug() << "Parse val 1 = " << parsed_data1.toDouble();
    ui->listWidget->scrollToBottom();
    ui->listWidget->autoScrollMargin();
    QString keyStr = QString::number(key);

    ui->listWidget->addItem("Time (s): " + keyStr + "\t Temp: " + parsed_data);
  }
  // make key axis range scroll with the data (at a constant range size of 8):
  ui->customPlot->xAxis->setRange(key, 20, Qt::AlignRight);
  ui->customPlot->replot();
  ui->customPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom |
                                  QCP::iSelectPlottables);
}

void MainWindow::updateTemperature(QString sensor_reading) {
  //  update the value displayed on the lcdNumber
  qDebug() << sensor_reading;
  ui->temp_lcdNumber->display(sensor_reading);
}

void MainWindow::updateTemperaturefah(QString sensor_reading) {
  //  update the value displayed on the lcdNumber
  qDebug() << sensor_reading;
  ui->temp_lcdNumber_2->display(sensor_reading);
}
