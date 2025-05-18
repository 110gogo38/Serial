#ifndef WIDGET_H
#define WIDGET_H

#include <QSerialPort>
#include <QTimer>
#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui {
class Widget;
}
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

private slots:
    void on_btnCloseOrOpenSerial_clicked();

    void on_btnSendContext_clicked();

    void on_SerialData_readyToRead();

    void on_checkBSendInTime_clicked(bool checked);

    void on_btnrevClear_clicked();

    void on_btnrevSave_clicked();

    void time_reflash();

    void on_checkBHexDisplay_clicked(bool checked);

private:
    Ui::Widget *ui;
    QSerialPort *serialPort;
    int writeCntTotal;
    int readCntTotal;
    QString sendBak;
    //QString myTime;
    bool serialStatus;
    QTimer* timer;

    QString getSysTime();
};
#endif // WIDGET_H
