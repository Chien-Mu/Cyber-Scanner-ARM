#include "scanthread.h"
#include <QDebug>

scanthread::scanthread(MainWindow *ref)
{
    this->ref = ref;
    this->quit = false;
    this->isCollecting = false;
    point_null.setX(0);
    point_null.setY(0);

    //timeout object
    timer = new QTimer;
    connect(timer,SIGNAL(timeout()),this,SLOT(timeoutProcess()));
    timer->setSingleShot(true); //不使用循環觸發

    //ROI
    this->enableROI = true;
}

void scanthread::stop(){
    quit = true;
}

void scanthread::Collect(QString value){
    Q_UNUSED(value);

    this->isCollecting = true;  //要求收集

    if(timeoutNum != 0)         //timeoutNum 設定0則取消此機制
        timer->start(timeoutNum*1000);
}

void scanthread::timeoutProcess(){
    this->isCollecting = false; //取消收集(傳送)
    emit timeout();
    emit timeout("X.. Scanning -> Timeout.");
}

void scanthread::setScan(int length_SN,int delay_loop,int delay_dmtx,int timeoutNum){
    this->length_SN = length_SN;
    this->delay_loop = delay_loop;
    this->delay_dmtx = delay_dmtx;
    this->timeoutNum = timeoutNum;
}

void scanthread::setROI(QRect ROI, bool enable){
    this->enableROI = enable;
    this->ROI = ROI;
}

void scanthread::run(){
    this->quit = false;

    while(!quit){
        QImage currentImage = ref->on_Capture(); //by MyVideoSurface 的 currentImage value(copy)

        //check
        if(currentImage.isNull()){
            msleep(200);
            continue;
        }

        //scanner
        if(!quit){
            //ROI
            if(this->enableROI){
                float Wratio = (float)PAINT_WIDTH/(float)currentImage.width();
                float Hratio = (float)PAINT_HEIGTH/(float)currentImage.height();
                QRect R(this->ROI.x()/Wratio,
                        this->ROI.y()/Hratio,
                        this->ROI.width()/Wratio,
                        this->ROI.height()/Hratio);
                QImage imgROI = currentImage.copy(R); //因為scan() 參數是指標，所以這邊要宣告出來
                this->currentSN = scan(&imgROI);
            }else
                this->currentSN = scan(&currentImage);

            //要求收集成立、且條碼長度符合，才會結束收集 並傳出此次條碼
            if(isCollecting && currentSN.length() == length_SN){
                timer->stop(); //已成功，所以取消 timeout 計數
                this->isCollecting = false;
                emit throwSN(this->currentSN);
            }
        }
        if(this->delay_loop != 0)
            msleep(delay_loop);
    }
}

QByteArray scanthread::scan(QImage *currentImage){
    QByteArray SN = "";
    QByteArray SendSN = "";

    DmtxImage *dmImg;
    DmtxDecode *dmDec;
    DmtxRegion *dmReg;
    DmtxMessage *dmMsg;
    DmtxTime timeout;

    //position
    DmtxVector2 p00, p10, p11, p01;
    p00.X = p00.Y = p10.Y = p01.X = 0.0;
    p10.X = p01.Y = p11.X = p11.Y = 1.0;
    int p_height;

    dmImg = dmtxImageCreate(currentImage->bits(),currentImage->bytesPerLine(),currentImage->height(),DmtxPack8bppK);
    if(dmImg == NULL)
        return "-1";

    //Image 屬性
    dmtxImageSetProp(dmImg, DmtxPropImageFlip, DmtxFlipNone); //不翻轉？

    dmDec = dmtxDecodeCreate(dmImg,1); //shrink;
    if(dmDec == NULL)
        return "-1";

    //Decode 屬性
    //dmtxDecodeSetProp(dmDec, DmtxPropEdgeMin, 70); //minimum-edge
    //dmtxDecodeSetProp(dmDec, DmtxPropEdgeMax, 220); //maximum-edge
    //dmtxDecodeSetProp(dmDec, DmtxPropScanGap, 2); //gap
    //dmtxDecodeSetProp(dmDec, DmtxPropSymbolSize, DmtxSymbolShapeAuto);
    //dmtxDecodeSetProp(dmDec, DmtxPropEdgeThresh, 100); //threshold 越高速度越快


    // dmtxTimeAdd()再迴圈內定義成找一個條碼時間
    // dmtxTimeAdd()再迴圈外定義成找 ScanLimit 個條碼時間
    int ScanCount=0;
    int ScanLimit=SCANTOTAL;
    INFO info;
    info.total = 0;
    for(int i =0;i<ScanLimit ;i++){
        info.SN[i].text = "";
        info.SN[i].A = point_null;
        info.SN[i].B = point_null;
        info.SN[i].C = point_null;
        info.SN[i].D = point_null;
    }

    for(int i=0;i<ScanLimit;i++){
        timeout = dmtxTimeAdd(dmtxTimeNow(),this->delay_dmtx); // timeout
        dmReg = dmtxRegionFindNext(dmDec,&timeout); //find

        if(dmReg == NULL)
            break;

        dmMsg = dmtxDecodeMatrixRegion(dmDec,dmReg,DmtxUndefined); //decode
        if(dmMsg != NULL){
            SN = (char*)dmMsg->output;
            info.SN[i].text = SN;
            if(SN.length() == length_SN) //長度有到達才回傳
                SendSN = SN;

            //position information
            p00.X = p00.Y = p10.Y = p01.X = 0.0;
            p10.X = p01.Y = p11.X = p11.Y = 1.0;
            p_height = dmtxDecodeGetProp(dmDec, DmtxPropHeight);
            dmtxMatrix3VMultiplyBy(&p00, dmReg->fit2raw);
            dmtxMatrix3VMultiplyBy(&p10, dmReg->fit2raw);
            dmtxMatrix3VMultiplyBy(&p11, dmReg->fit2raw);
            dmtxMatrix3VMultiplyBy(&p01, dmReg->fit2raw);
            //qDebug() << (int)(p00.X + 0.5) << p_height - 1 - (int)(p00.Y + 0.5);
            //qDebug() << (int)(p10.X + 0.5) << p_height - 1 - (int)(p10.Y + 0.5);
            //qDebug() << (int)(p11.X + 0.5) << p_height - 1 - (int)(p11.Y + 0.5);
            //qDebug() << (int)(p01.X + 0.5) << p_height - 1 - (int)(p01.Y + 0.5);
            info.SN[i].A.setX((int)(p01.X + 0.5));
            info.SN[i].A.setY(p_height - 1 - (int)(p01.Y + 0.5));
            info.SN[i].B.setX((int)(p11.X + 0.5));
            info.SN[i].B.setY(p_height - 1 - (int)(p11.Y + 0.5));
            info.SN[i].C.setX((int)(p00.X + 0.5));
            info.SN[i].C.setY(p_height - 1 - (int)(p00.Y + 0.5));
            info.SN[i].D.setX((int)(p10.X + 0.5));
            info.SN[i].D.setY(p_height - 1 - (int)(p10.Y + 0.5));
            ScanCount++; //要在資料確定近來這才累加

            //release dmMsg
            dmtxMessageDestroy(&dmMsg);
        }
        //default
        dmtxRegionDestroy(&dmReg);
    }
    info.total = ScanCount;
    emit throwInfo(info);

    dmtxDecodeDestroy(&dmDec);
    dmtxImageDestroy(&dmImg);
    return SendSN;
}
