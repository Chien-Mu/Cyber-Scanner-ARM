#ifndef SCANTHREAD_H
#define SCANTHREAD_H

#include <QObject>
#include <QThread>
#include <QImage>
#include <QTimer>
#include <dmtx.h>

#include "mainwindow.h"
#include "shared.h"

class scanthread : public QThread
{
    Q_OBJECT
public:
    scanthread(MainWindow *ref);
    void setScan(int length_SN, int delay_loop, int delay_dmtx, int timeoutNum);
    void run();
    void stop();

public slots:
    void Collect(QString value);

private:
    MainWindow *ref;
    bool quit;
    QByteArray scan(QImage *currentImage);
    QPoint point_null;
    bool isCollecting;
    QByteArray currentSN;
    int length_SN;
    int delay_loop;
    int delay_dmtx;
    QTimer *timer;
    unsigned timeoutNum;

private slots:
    void timeoutProcess();

signals:
    void throwInfo(const INFO value);
    void throwSN(QByteArray SN);
    void timeout();
    void timeout(QString value);
};

#endif // SCANTHREAD_H
