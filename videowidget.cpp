#include "videowidget.h"
#include <QDebug>

VideoWidget::VideoWidget(int W, int H, QWidget *parent) : QWidget(parent)
{
    this->W = W;
    this->H = H;
    this->Wratio = 640.0/(float)W;
    this->Hratio = 480.0/(float)H;

    this->isPush = true;
    this->isdraw = false;

    rect_null.setX(0);
    rect_null.setY(0);
    rect_null.setWidth(0);
    rect_null.setHeight(0);
    point_null.setX(0);
    point_null.setY(0);
    for(int i=0;i<SCANTOTAL;i++){
        this->rects.push_back(rect_null);
        this->SN.push_back("");
        this->point_SN.push_back(point_null);
    }

    surface = new MyVideoSurface(this,W,H);
    this->setFixedSize(640,480); //如果沒設畫板大小，可能顯示會有問題。
}

VideoWidget::~VideoWidget(){
    delete surface;
}

void VideoWidget::lock(){
    this->isPush = true;
}

void VideoWidget::draw(INFO info){
    while(isdraw)
        continue;

    for(int i=0;i<SCANTOTAL && !isdraw;i++){
        if(i<info.total || !info.SN[i].rect.isNull() || !info.SN[i].rect.isEmpty()){
            this->rects[i] = QRect(info.SN[i].rect.x()*Wratio,
                                   info.SN[i].rect.y()*Hratio,
                                   info.SN[i].rect.width()*Wratio,
                                   info.SN[i].rect.height()*Hratio);
            this->SN[i] = info.SN[i].text;
            this->point_SN[i] = QPoint(rects[i].x(),rects[i].y()-10);
        }else{
            this->rects[i] = rect_null;
            this->SN[i] = "";
            this->point_SN[i] = point_null;
        }
    }

    isPush = false;
}

void VideoWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    if (surface->isActive()) {
        surface->Drawing(true);
        QPainter painter;
        painter.begin(this); //實驗這樣把 .begin() .end() 寫出來效率會更快
        surface->paintImage(&painter);

        //draw
        pen.setBrush(Qt::black);
        pen.setWidth(1);
        painter.setPen(pen);
        painter.drawLine(0,240,640,240);
        painter.drawLine(320,0,320,480);

        pen.setBrush(Qt::red);
        pen.setWidth(4);
        painter.setPen(pen);
        if(!isPush){
            isdraw = true;
            painter.drawRects(rects); //在畫上矩形
            painter.drawText(point_SN[0],SN[0]);
            painter.drawText(point_SN[1],SN[1]);
            isdraw = false;
        }

        painter.end();
        surface->Drawing(false);
    }
}
