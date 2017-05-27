#include "serialportthread.h"
#include <QDebug>

SerialPortThread::SerialPortThread()
{
    this->quit = false;
}

SerialPortThread::~SerialPortThread(){
    this->quit = true;
    this->cond.wakeOne();
}

void SerialPortThread::setCOM(QString COM,int baud,int dataB,int parity,int stopB,int flowC){
    this->COM = COM;
    this->baud = baud;
    this->dataB = dataB;
    this->parity = parity;
    this->stopB = stopB;
    this->flowC = flowC;
}

void SerialPortThread::serial_write(QByteArray request){
    if(this->isFinished()){
        emit status("X.. COM PORT 無開啟：執行緒因關閉無法傳輸條碼");
        this->quit = true;
    }
    this->requestData = request;
    this->cond.wakeOne();
}

void SerialPortThread::stop(){
    this->quit = true;
    this->cond.wakeOne();
}

void SerialPortThread::run(){
    this->quit = false;

    QSerialPort serial;
    //set
    serial.setPortName(COM);
    serial.setBaudRate((QSerialPort::BaudRate)baud);
    serial.setDataBits((QSerialPort::DataBits)dataB);
    serial.setParity((QSerialPort::Parity)parity);
    serial.setStopBits((QSerialPort::StopBits)stopB);
    serial.setFlowControl((QSerialPort::FlowControl)flowC);
    //open
    if (!serial.open(QIODevice::ReadWrite)) {
        emit status("X.. COM PORT 開啟失敗：" + serial.errorString());
        return;
    }

    //write
    while(!quit){
        mutex.lock();
        cond.wait(&mutex);

        if(quit){
            mutex.unlock();
            break;
        }
        emit status(">.. send -> " + requestData); //sending

        //add buffer
        bytesWritten = serial.write(requestData);

        //check buffer
        if (bytesWritten == -1)
            emit status(QString("x.. Failed to write the data to port %1, error: %2").
                        arg(serial.portName()).arg(serial.errorString()));
        else if (bytesWritten != requestData.size())
            emit status(QString("x.. Failed to write all the data to port %1, error: %2").
                        arg(serial.portName()).arg(serial.errorString()));

        //push buffer
        if(!serial.waitForBytesWritten(5000)){
            emit status(QString("X.. Sent failed. waitForBytesWritten() timed out for port %1, error: %2").
                        arg(serial.portName()).arg(serial.errorString()));
        }else
            emit status(">.. Sent successfully."); //sended

        emit END(); //告知已傳出成功
        mutex.unlock();
    }

    if (serial.isOpen())
        serial.close();
}


