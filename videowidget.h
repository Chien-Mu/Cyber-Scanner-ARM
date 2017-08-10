#ifndef VIDEOWIDGET_H
#define VIDEOWIDGET_H

#include <QWidget>
#include <QVector>
#include "myvideosurface.h"
#include "shared.h"

struct PaintROI{
    QPoint PP;  //press point
    QPoint MP;  //move point
    QRect ROI;
};

class VideoWidget : public QWidget
{
    Q_OBJECT
public:
    explicit VideoWidget(QSize imageSize, QWidget *parent = 0);
    ~VideoWidget();
    QAbstractVideoSurface *refVideoSurface() const { return surface; }
    MyVideoSurface *refImageSurface() { return surface; }
    void lock();
    void draw(INFO info);
    void readROI(QRect ROI);
    void setEnableDraw(bool value);

protected:
    void paintEvent(QPaintEvent *event);
    virtual void mousePressEvent(QMouseEvent *event);
    virtual void mouseMoveEvent(QMouseEvent *event);
    virtual void mouseReleaseEvent(QMouseEvent *event);

private:
    MyVideoSurface *surface;
    QSize imageSize;
    float Wratio;
    float Hratio;
    QPen pen;
    INFO info;
    QVector<QPoint> point_SN;
    QPoint point_null;

    volatile bool isPush;
    volatile bool isdraw;

    PaintROI paintROI;
    bool EnableDraw;

signals:
    void throwROI_Rect(QRect ROI);
};

#endif // VIDEOWIDGET_H
