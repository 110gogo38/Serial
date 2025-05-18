#include "widget.h"
#include "ui_widget.h"
#include <QSerialPortInfo>
#include <QDebug>
#include <QSerialPort>
#include <QMessageBox>
#include <QFileDialog>
#include <QDateTime>

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
    this->setLayout(ui->gridLayoutAll);
    writeCntTotal=0;
    readCntTotal=0;
    serialStatus=false;
    ui->btnSendContext->setEnabled(false);
    ui->checkBSendInTime->setEnabled(false);

    serialPort=new QSerialPort(this);

    QTimer *getSysTimeTimer=new QTimer(this);
    connect(getSysTimeTimer,SIGNAL(timeout()),this,SLOT(time_reflash()));
    getSysTimeTimer->start(1000);

    timer=new QTimer(this);

    connect(serialPort,&QSerialPort::readyRead,this,&Widget::on_SerialData_readyToRead);
    connect(timer,&QTimer::timeout,[=](){
        on_btnSendContext_clicked();
    });

    ui->comboBox_boautrate->setCurrentIndex(1);
    ui->comboBox_databit->setCurrentIndex(3);

    QList<QSerialPortInfo> serialList= QSerialPortInfo::availablePorts();
    qDebug() << "检测到的串口数量:" << serialList.size();
    for(QSerialPortInfo serialInfo:serialList){
        qDebug()<<serialInfo.portName();
        ui->comboBox_serialNum->addItem(serialInfo.portName());
    }
}

Widget::~Widget()
{
    delete ui;
}

void Widget::on_btnCloseOrOpenSerial_clicked()
{
    if(!serialStatus){
        //1.选择端口号
        serialPort->setPortName(ui->comboBox_serialNum->currentText());
        //2.配置波特率
        serialPort->setBaudRate(ui->comboBox_boautrate->currentText().toInt());
        //3.配置数据位
        serialPort->setDataBits(QSerialPort::DataBits(ui->comboBox_databit->currentText().toUInt()));
        //4.配置校验位
        switch (ui->comboBox_jiaoyan->currentIndex()) {
        case 0:
            serialPort->setParity(QSerialPort::NoParity);
            break;
        case 1:
            serialPort->setParity(QSerialPort::EvenParity);
            break;
        case 2:
            serialPort->setParity(QSerialPort::MarkParity);
            break;
        case 3:
            serialPort->setParity(QSerialPort::OddParity);
            break;
        case 4:
            serialPort->setParity(QSerialPort::SpaceParity);
            break;
        default:
            //serialPort->setParity(QSerialPort::UnknownParity);
            break;
        }
        //5.配置停止位
        serialPort->setStopBits(QSerialPort::StopBits(ui->comboBox_databit->currentText().toUInt()));
        //6.流控
        if(ui->comboBox_fileCon->currentText()=="None"){
            serialPort->setFlowControl(QSerialPort::NoFlowControl);
        }
        //7.打开串口
        if(serialPort->open(QIODevice::ReadWrite)){
            qDebug()<<"serial open success";
            ui->comboBox_databit->setEnabled(false);
            ui->comboBox_serialNum->setEnabled(false);
            ui->comboBox_boautrate->setEnabled(false);
            ui->comboBox_fileCon->setEnabled(false);
            ui->comboBox_jiaoyan->setEnabled(false);
            ui->comboBox_stopbit->setEnabled(false);
            ui->btnCloseOrOpenSerial->setText("关闭串口");
            ui->btnSendContext->setEnabled(true);
            ui->checkBSendInTime->setEnabled(true);
            serialStatus=true;
        }else{
            QMessageBox msgBox;
            msgBox.setWindowTitle("打开串口错误");
            msgBox.setText("串口打开失败，串口被占用！");
            msgBox.exec();
        }

    }else{
        serialPort->close();
        ui->btnCloseOrOpenSerial->setText("打开串口");
        ui->comboBox_databit->setEnabled(true);
        ui->comboBox_serialNum->setEnabled(true);
        ui->comboBox_boautrate->setEnabled(true);
        ui->comboBox_fileCon->setEnabled(true);
        ui->comboBox_jiaoyan->setEnabled(true);
        ui->comboBox_stopbit->setEnabled(true);
        serialStatus=false;
        ui->btnSendContext->setEnabled(false);
        ui->checkBSendInTime->setEnabled(false);
        ui->checkBSendInTime->setCheckState(Qt::Unchecked);
        timer->stop();
        ui->lineEditTimeeach->setEnabled(true);
        ui->lineEditSendContext->setEnabled(true);
    }
}


void Widget::on_btnSendContext_clicked()
{
    int writeCnt=0;
    QByteArray sendData=ui->lineEditSendContext->text().toUtf8();
    writeCnt=serialPort->write(sendData+'\n');
    if(writeCnt==-1){
        ui->labelSendStatus->setText("SendError!");
    }else{
        writeCntTotal+=(writeCnt-1);
        qDebug()<<"SendOK"<<sendData;
        ui->labelSendStatus->setText("SendOK!");
        ui->labelSendcnt->setText("Send:"+QString::number(writeCntTotal));
    }
}

void Widget::on_SerialData_readyToRead()
{
    QByteArray revMessage=serialPort->readAll();
    qDebug() << "接收到数据，大小:" << revMessage.size();
    if(!revMessage.isEmpty()){
        if(ui->checkBrevTime->isChecked()){
            QString timeStamp=getSysTime()+"  ";
            if(strcmp(revMessage,sendBak.toUtf8())!=0){
                ui->textEditRecord->append(revMessage);
                sendBak=QString(revMessage);
            }
            if(ui->checkBHexDisplay->isChecked()){
                QByteArray timeStampHex=timeStamp.toUtf8().toHex();
                QByteArray tmpHexString=revMessage.toHex();
                QString tmpStringHex=ui->textEditRev->toPlainText();
                tmpHexString=tmpStringHex.toUtf8()+tmpHexString;
                ui->textEditRev->setText(timeStampHex+QString::fromUtf8(tmpHexString));
            }else{
                ui->textEditRev->append(timeStamp+revMessage);
            }
        }else{
            if(strcmp(revMessage,sendBak.toUtf8())!=0){
                ui->textEditRecord->append(revMessage);
                sendBak=QString(revMessage);
            }

                if(ui->checkBHexDisplay->isChecked()){
                    QByteArray tmpHexString=revMessage.toHex();
                    QString tmpStringHex=ui->textEditRev->toPlainText();
                    tmpHexString=tmpStringHex.toUtf8()+tmpHexString;
                    ui->textEditRev->setText(QString::fromUtf8(tmpHexString));
                }else{
                    ui->textEditRev->append(revMessage);
                }
            //}
        }
                readCntTotal+=revMessage.size();
                ui->labelRevcnt->setText("Received:"+QString::number(readCntTotal));
    }
}


void Widget::on_checkBSendInTime_clicked(bool checked)
{
    if(checked){
        ui->lineEditTimeeach->setEnabled(false);
        ui->lineEditSendContext->setEnabled(false);
        timer->start(ui->lineEditTimeeach->text().toInt());
    }else{
        timer->stop();
        ui->lineEditTimeeach->setEnabled(true);
        ui->lineEditSendContext->setEnabled(true);
    }
}

void Widget::on_btnrevClear_clicked()
{
    ui->textEditRev->clear();
}


void Widget::on_btnrevSave_clicked()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"),
                                                    "E:/QT6/code/Serial/serialData.txt",
                                                    tr("Text (*.txt)"));
    if(!fileName.isNull()){
        QFile file(fileName);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
            return;

        QTextStream out(&file);
        out << ui->textEditRev->toPlainText();
        file.close();
    }
}

void Widget::time_reflash()
{
    ui->labelCurrentTime->setText(getSysTime());
}

QString Widget::getSysTime()
{
    QDateTime currentTime=QDateTime::currentDateTime();
    QString myTime =currentTime.toString("yyyy-MM-dd  hh:mm:ss");
    return myTime;
}

void Widget::on_checkBHexDisplay_clicked(bool checked)
{
    if(checked){
        QString tmp=ui->textEditRev->toPlainText();
        QByteArray qtmp=tmp.toUtf8();
        qtmp=qtmp.toHex();
        ui->textEditRev->setText(qtmp);
    }else{
        QString tmpHexString=ui->textEditRev->toPlainText();
        QByteArray tmpHexQBytearray=tmpHexString.toUtf8();
        QByteArray tmpQByeString =QByteArray::fromHex(tmpHexQBytearray);
        ui->textEditRev->setText(tmpQByeString);
    }
}
