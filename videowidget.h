#ifndef VIDEOWIDGET_H
#define VIDEOWIDGET_H

#include <QWidget>
#include <QVector>
#include "myvideosurface.h"
#include "shared.h"

class VideoWidget : public QWidget
{
public:
    explicit VideoWidget(int W,int H, QWidget *parent = 0);
    ~VideoWidget();
    QAbstractVideoSurface *refVideoSurface() const { return surface; }
    MyVideoSurface *refImageSurface() { return surface; }
    void lock();
    void draw(INFO info);

protected:
    void paintEvent(QPaintEvent *event);

private:
    MyVideoSurface *surface;
    int W;
    int H;
    float Wratio;
    float Hratio;
    QPen pen;
    INFO info;
    QVector<QPoint> point_SN;
    QPoint point_null;

    volatile bool isPush;
    volatile bool isdraw;
};

#endif // VIDEOWIDGET_H
