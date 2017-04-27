#ifndef SERIALPORTTHREAD_H
#define SERIALPORTTHREAD_H

#include <QThread>
#include <QtSerialPort/QSerialPort>
#include <QMutex>
#include <QWaitCondition>

class SerialPortThread : public QThread
{
    Q_OBJECT
public:
    SerialPortThread();
    ~SerialPortThread();
    void run();
    void setCOM(QString COM, int baud, int dataB, int parity, int stopB, int flowC);

private:
    QString COM;
    int baud;
    int dataB;
    int parity;
    int stopB;
    int flowC;

    QByteArray requestData;
    int bytesWritten;
    bool quit;
    QMutex mutex;
    QWaitCondition cond;

public slots:
    void serial_write(QByteArray request);
    void stop();

signals:
    void status(QString value);
    void END();
};

#endif // SERIALPORTTHREAD_H
