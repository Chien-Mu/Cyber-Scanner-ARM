#ifndef MYVIDEOSURFACE_H
#define MYVIDEOSURFACE_H

#include <QAbstractVideoSurface>
#include <QWidget>
#include <QVideoSurfaceFormat>
#include <QPainter>
#include <QImage>

//實作 QAbstractVideoSurface 抽象類別
class MyVideoSurface : public QAbstractVideoSurface
{
    Q_OBJECT
public:
    MyVideoSurface(QWidget *widget, int W, int H, QObject *parent = 0);

    QList<QVideoFrame::PixelFormat> supportedPixelFormats(
            QAbstractVideoBuffer::HandleType handleType = QAbstractVideoBuffer::NoHandle) const;

    bool present(const QVideoFrame &frame);

    void paintImage(QPainter *painter);
    void Drawing(bool isDraw);
    void getlock();
    QImage getCurrentImage();

private:
    QWidget *widget;
    QVideoFrame currentFrame;
    QImage _image;
    int W;
    int H;
    volatile bool isDraw;
    volatile bool isGet;
};

#endif // MYVIDEOSURFACE_H
