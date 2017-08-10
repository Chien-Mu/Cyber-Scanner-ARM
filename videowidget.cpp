#include "videowidget.h"
#include <QDebug>
#include <QMouseEvent>

VideoWidget::VideoWidget(QSize imageSize, QWidget *parent) : QWidget(parent)
{
    this->imageSize = imageSize; //設定圖像大小
    QSize widgetSize(PAINT_WIDTH,PAINT_HEIGTH); //設定畫版大小(非圖像大小)

    this->Wratio = (float)widgetSize.width()/(float)imageSize.width();
    this->Hratio = (float)widgetSize.height()/(float)imageSize.height();

    this->isPush = true;
    this->isdraw = false;
    this->EnableDraw = false;

    point_null.setX(0);
    point_null.setY(0);
    for(int i=0;i<SCANTOTAL;i++){
        this->point_SN.push_back(point_null);
        info.SN[i].text = "";
        info.SN[i].A = point_null;
        info.SN[i].B = point_null;
        info.SN[i].C = point_null;
        info.SN[i].D = point_null;
    }
    info.total = 0;

    surface = new MyVideoSurface(this,widgetSize);
    this->setFixedSize(widgetSize); //如果沒設畫板大小，可能顯示會有問題。
}

VideoWidget::~VideoWidget(){
    delete surface;
}

void VideoWidget::setEnableDraw(bool value){
    this->EnableDraw = value;
}

void VideoWidget::lock(){
    this->isPush = true;
}

void VideoWidget::draw(INFO info){
    while(isdraw)
        continue;

    for(int i=0;i<SCANTOTAL && !isdraw;i++){
        if(i<info.total || !info.SN[i].A.isNull()){
            this->info.SN[i].A.setX( (info.SN[i].A.x()*Wratio) + this->paintROI.ROI.x());
            this->info.SN[i].A.setY( (info.SN[i].A.y()*Hratio) + this->paintROI.ROI.y());
            this->info.SN[i].B.setX( (info.SN[i].B.x()*Wratio) + this->paintROI.ROI.x());
            this->info.SN[i].B.setY( (info.SN[i].B.y()*Hratio) + this->paintROI.ROI.y());
            this->info.SN[i].C.setX( (info.SN[i].C.x()*Wratio) + this->paintROI.ROI.x());
            this->info.SN[i].C.setY( (info.SN[i].C.y()*Hratio) + this->paintROI.ROI.y());
            this->info.SN[i].D.setX( (info.SN[i].D.x()*Wratio) + this->paintROI.ROI.x());
            this->info.SN[i].D.setY( (info.SN[i].D.y()*Hratio) + this->paintROI.ROI.y());
            this->info.SN[i].text = info.SN[i].text;

            //找XY相加最小的，找出左上角，並顯示SN
            int min = this->info.SN[i].A.x() + this->info.SN[i].A.y();
            this->point_SN[i] = QPoint(this->info.SN[i].A.x(),this->info.SN[i].A.y());
            if(min > (this->info.SN[i].B.x() + this->info.SN[i].B.y())){
                min = this->info.SN[i].B.x() + this->info.SN[i].B.y();
                this->point_SN[i] = QPoint(this->info.SN[i].B.x(),this->info.SN[i].B.y());
            }
            if(min > (this->info.SN[i].C.x() + this->info.SN[i].C.y())){
                min = this->info.SN[i].C.x() + this->info.SN[i].C.y();
                this->point_SN[i] = QPoint(this->info.SN[i].C.x(),this->info.SN[i].C.y());
            }
            if(min > (this->info.SN[i].D.x() + this->info.SN[i].D.y())){
                min = this->info.SN[i].D.x() + this->info.SN[i].D.y();
                this->point_SN[i] = QPoint(this->info.SN[i].D.x(),this->info.SN[i].D.y());
            }
            this->point_SN[i].setY(this->point_SN[i].y()-10);

        }else{
            this->info.SN[i].A = point_null;
            this->info.SN[i].B = point_null;
            this->info.SN[i].C = point_null;
            this->info.SN[i].D = point_null;
            this->info.SN[i].text = "";
            this->point_SN[i] = point_null;
        }
    }
    this->info.total = info.total;

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
        pen.setBrush((Qt::green));
        pen.setWidth(2);
        painter.setPen(pen);
        painter.drawText(this->paintROI.ROI.x(),this->paintROI.ROI.y()-5,"ROI");
        painter.drawRect(this->paintROI.ROI);

        //draw detetion
        pen.setBrush(Qt::red);
        pen.setWidth(4);
        painter.setPen(pen);
        if(!isPush){
            isdraw = true;
            if(info.total != 0)
                for(int i=0;i<SCANTOTAL;i++){
                    //在畫任意四邊形
                    painter.drawLine(info.SN[i].A,info.SN[i].B);
                    painter.drawLine(info.SN[i].B,info.SN[i].D);
                    painter.drawLine(info.SN[i].D,info.SN[i].C);
                    painter.drawLine(info.SN[i].C,info.SN[i].A);
                    //畫SN
                    painter.drawText(point_SN[i],info.SN[i].text);
                }
            isdraw = false;
        }

        painter.end();
        surface->Drawing(false);
    }
}

void VideoWidget::readROI(QRect ROI){
    this->paintROI.ROI = ROI;
}

void VideoWidget::mousePressEvent(QMouseEvent *event){
    if(event->button() == Qt::LeftButton && EnableDraw){
        this->paintROI.MP = event->pos(); //default
        this->paintROI.PP = event->pos();
    }
}

void VideoWidget::mouseMoveEvent(QMouseEvent *event){
    if(EnableDraw){
        this->paintROI.MP = event->pos();

        this->paintROI.ROI.setX(this->paintROI.PP.x());
        this->paintROI.ROI.setY(this->paintROI.PP.y());
        this->paintROI.ROI.setWidth(this->paintROI.MP.x() - this->paintROI.PP.x());
        this->paintROI.ROI.setHeight(this->paintROI.MP.y() - this->paintROI.PP.y());
    }
}

void VideoWidget::mouseReleaseEvent(QMouseEvent *event){
    if(event->button() == Qt::LeftButton && EnableDraw)
        emit throwROI_Rect(paintROI.ROI);  //sent ROI Rect
}
