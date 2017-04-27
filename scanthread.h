#ifndef SCANTHREAD_H
#define SCANTHREAD_H

#include <QObject>
#include <QThread>
#include <QImage>
#include <dmtx.h>

#include "mainwindow.h"
#include "shared.h"

class scanthread : public QThread
{
    Q_OBJECT
public:
    scanthread(MainWindow *ref);
    void setScan(int length_SN,int delay_loop,int delay_dmtx);
    void run();
    void stop();

public slots:
    void Collect(QString value);

private:
    MainWindow *ref;
    bool quit;
    QByteArray scan(QImage *currentImage);
    bool isCollecting;
    QByteArray currentSN;
    int length_SN;
    int delay_loop;
    int delay_dmtx;

signals:
    void throwInfo(const INFO value);
    void throwSN(QByteArray SN);
};

#endif // SCANTHREAD_H
