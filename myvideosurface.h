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
    MyVideoSurface(QWidget *widget, QSize widgetSize, QObject *parent = 0); //為了觸發外界的 eventpaint，所以建構子讓 widget 帶進來

    QList<QVideoFrame::PixelFormat> supportedPixelFormats(
            QAbstractVideoBuffer::HandleType handleType = QAbstractVideoBuffer::NoHandle) const;

    bool present(const QVideoFrame &frame);

    void paintImage(QPainter *painter);
    void Drawing(bool isDraw);
    void getlock();
    QImage &getCurrentImage();

private:
    QWidget *widget;
    QVideoFrame currentFrame;
    QImage currentImage;
    QImage _image;
    QSize widgetSize;
    volatile bool isDraw;
    volatile bool isGet;
};

#endif // MYVIDEOSURFACE_H
