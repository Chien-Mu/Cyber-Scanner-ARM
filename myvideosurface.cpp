#include "myvideosurface.h"
#include <QDebug>

MyVideoSurface::MyVideoSurface(QWidget *widget,int W,int H, QObject *parent) : QAbstractVideoSurface(parent)
{
    this->widget = widget;
    this->W = W;
    this->H = H;
    this->isDraw = false; //一開始要讓他進去
    this->isGet = true; //一開始不能讓他進去
}

QList<QVideoFrame::PixelFormat> MyVideoSurface::supportedPixelFormats(
        QAbstractVideoBuffer::HandleType handleType) const
{
    if (handleType == QAbstractVideoBuffer::NoHandle) {
        return QList<QVideoFrame::PixelFormat>()
                << QVideoFrame::Format_RGB24 //raspberry pi Format_RGB32不能用，會跳到 Format_ARGB32
                << QVideoFrame::Format_RGB32
                << QVideoFrame::Format_ARGB32
                << QVideoFrame::Format_ARGB32_Premultiplied
                << QVideoFrame::Format_RGB565
                << QVideoFrame::Format_RGB555;
    } else {
        return QList<QVideoFrame::PixelFormat>();
    }
}

bool MyVideoSurface::present(const QVideoFrame &frame)
{
    Q_UNUSED(frame);

    // Handle the frame and do your processing
    if (frame.isValid()){
        currentFrame = frame;

        if(!this->isDraw)
            widget->repaint();

        return true;
    }
    return false;
}

void MyVideoSurface::paintImage(QPainter *painter)
{
    if (currentFrame.map(QAbstractVideoBuffer::ReadOnly)) {

        QImage image(
                    currentFrame.bits(),
                    currentFrame.width(),
                    currentFrame.height(),
                    currentFrame.bytesPerLine(),
                    QVideoFrame::imageFormatFromPixelFormat(currentFrame.pixelFormat()));

        if(image.isNull()){
            qDebug() << "paintImage image.isNull()";
            return;
        }

        image = image.scaled(W,H);
        //image = image.mirrored();
        painter->drawImage(0,0,image);

        //如果沒有在擷取
        if(!isGet)
            _image = image;

        currentFrame.unmap();
    }
}

void MyVideoSurface::Drawing(bool isDraw){
    this->isDraw = isDraw;
}

//不可兩個執行緒同時呼叫此函式，不然就算加了isGet也會不穩
void MyVideoSurface::getlock(){
    this->isGet = true;
}

QImage MyVideoSurface::getCurrentImage(){
    QImage currentImage;
    currentImage = _image.convertToFormat(QImage::Format_Grayscale8);
    this->isGet = false;

    return currentImage;
}
