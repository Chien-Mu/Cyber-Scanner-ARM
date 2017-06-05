#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>
#include <QMessageBox>
#include <QCameraInfo>
#include <QSpacerItem>

#include "scanthread.h"
scanthread *scanner;

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent),ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->currentImage_ptr = 0;

    //status
    la_status = new QLabel;
    la_status->setAlignment(Qt::AlignHCenter); //文字置中
    ui->statusBar->addWidget(la_status,2); //2 是為了佔滿整個form
    isMode(false);

    //為了讓camera在中心
    QSpacerItem * H_spacer = new QSpacerItem(1000,0, QSizePolicy::Expanding, QSizePolicy::Expanding);
    QSpacerItem * H_spacer_2 = new QSpacerItem(1000,0, QSizePolicy::Expanding, QSizePolicy::Expanding);
    ui->gridLayout->addItem(H_spacer,0,0);
    ui->gridLayout->addItem(H_spacer_2,0,2);


    //camera
    camera = new Camera;

    //列出相機
    QList<QCameraInfo> cameras = QCameraInfo::availableCameras();
    //qDebug() << cameras[0].deviceName(); //設備id
    //qDebug() << cameras[0].description(); //設備名稱

    //camera set
    //camera->setCamera(cameras[0].deviceName().toLocal8Bit());
    camera->setCamera("");
    connect(camera,SIGNAL(Error(int,QCameraImageCapture::Error,QString)),
            this,SLOT(displayCaptureError(int,QCameraImageCapture::Error,QString)));
    camera->CameraStrat();

    //-------------------------------------------------------------
    about = new About;
    shared = new Shared;
    config = new Config;
    connect(this,SIGNAL(closeWindow()),config,SLOT(setConfig()));
    connect(camera,SIGNAL(throwROI_Rect(QRect)),config,SLOT(saveROI(QRect))); //丟ROI 到 config class 紀錄
    camera->readROI(config->get_ROI()); //draw txt 中紀錄的 ROI


    //comport
    com = new SerialPortThread;
    connect(com,SIGNAL(status(QString)),this,SLOT(setStatus(QString)),Qt::QueuedConnection);
    connect(com,SIGNAL(started()),this,SLOT(isStart()),Qt::QueuedConnection);
    connect(com,SIGNAL(finished()),this,SLOT(isStart()),Qt::QueuedConnection);

    //scanner
    scanner = new scanthread(this);
    connect(scanner,SIGNAL(throwSN(QByteArray)),com,SLOT(serial_write(QByteArray)));
    connect(scanner,SIGNAL(started()),this,SLOT(isStart()),Qt::QueuedConnection);
    connect(scanner,SIGNAL(finished()),this,SLOT(isStart()),Qt::QueuedConnection);
    connect(scanner,SIGNAL(timeout(QString)),this,SLOT(setStatus(QString))); //timeout 訊息

    //GPIO
    gpio = new GPIO;
    connect(com,SIGNAL(END()),gpio,SLOT(CollectEND())); //觸發 CollectEND() GPIO 才會送下一個 Collect()
    connect(gpio,SIGNAL(Collect(QString)),scanner,SLOT(Collect(QString)));
    connect(gpio,SIGNAL(Collect(QString)),this,SLOT(setStatus(QString)),Qt::QueuedConnection);
    connect(gpio,SIGNAL(started()),this,SLOT(isStart()),Qt::QueuedConnection);
    connect(gpio,SIGNAL(finished()),this,SLOT(isStart()),Qt::QueuedConnection);
    connect(scanner,SIGNAL(timeout()),gpio,SLOT(CollectEND())); //Scanner timeout,觸發 CollectEND()

    //initialization
    Start();
    ui->actionStart->setEnabled(false);

    //draw(一定要在 initialization() 之後)
    qRegisterMetaType<INFO>("INFO");
    connect(scanner,SIGNAL(throwInfo(INFO)),camera,SLOT(drawVideoWidget(INFO)),Qt::BlockingQueuedConnection);
    ui->gridLayout->addWidget(camera->getVideoWidget(),0,1);

    //plain
    formText = new QPlainTextEdit(this);
    ui->gridLayout->addWidget(formText,1,0,1,3);
    p.setColor(QPalette::Base, Qt::black);
    p.setColor(QPalette::Text, Qt::green);
    formText->setPalette(p);
    formText->setReadOnly(true); //限制輸入
    formText->document()->setMaximumBlockCount(20000); //限制內容行數
    connect(this,SIGNAL(setPaint(QString)),formText,SLOT(appendPlainText(QString))
            ,Qt::QueuedConnection);

    //menu
    connect(ui->menuBar,SIGNAL(triggered(QAction*)),this,SLOT(trigerMenu(QAction*)));
}

void MainWindow::trigerMenu(QAction *act){
    if (act->text() == "Start"){
        this->Start();
        ui->actionStart->setEnabled(false);
        ui->actionStop->setEnabled(true);
    }else if(act->text() == "Stop"){
        this->stop();
        ui->actionStart->setEnabled(true);
        ui->actionStop->setEnabled(false);
    }else if (act->text() == "Close"){
        this->close();
    }else if (act->text() == "Setting..."){
        config->show();
        config->raise();
    }else if(act->text() == "About"){
        about->show();
        about->raise();
    }

}

bool MainWindow::Start(){
    //CREATE DIR
    if(!dir.mkpath(QDir::homePath()+"/log"))
        setStatus("!.. " + QDir::homePath()+"/log -> The dir an not be created.");
    //-------------------------------

    //comport
    com->setCOM(config->get_COM(),
                config->get_Baud(),
                config->get_DataB(),
                config->get_Parity(),
                config->get_StopB(),
                config->get_FlowC());
    com->start();

    //gpio
#if defined ( RASPBERRY )
    gpio->setGPIO(config->get_delay_gpio());
    gpio->start();
#endif

    //scanner
    scanner->setScan(config->get_length_SN(),
                     config->get_delay_loop(),
                     config->get_delay_dmtx(),
                     config->get_scanTimout());
    scanner->setROI(config->get_ROI(),true);
    scanner->start();

    return true;
}

void MainWindow::stop(bool isCloseCamera){
    if(scanner->isRunning())
        scanner->stop();
    if(com->isRunning())
        com->stop();
    if(gpio->isRunning())
        gpio->stop();

    if(isCloseCamera){
        camera->CameraStop();
    }
    //qDebug() << "MainWindow::stop()";
}

QImage* MainWindow::on_Capture(){
    this->currentImage =  camera->getCurrentImage(); //by value
    this->currentImage_ptr = &this->currentImage;
    return currentImage_ptr;
}

void MainWindow::setStatus(QString value){
    if(value.isNull())
        return;

    QString time = shared->getDateTime_print();

    //file
    QString txtPath = QDir::homePath() + "/log/" + shared->getDate() +"_log.txt";
    file.setFileName(txtPath);
    if(!file.open(QIODevice::Append|QIODevice::Text))
        QMessageBox::critical(this, tr("Error"), txtPath+ "..does not exist.");
    QTextStream out(&file);

    //status
    switch (value.at(0).toLatin1()) {
    case '>':  //一般資訊
        emit setPaint(time + value);
        out << time << value << "\n";
        break;
    case 'x': //小錯誤
        emit setPaint(time + value);
        out << time << value << "\n";
        break;
    case 'X': //大錯誤(有視窗)
        emit setPaint(time + value);
        out << time << "MessageBox: " << value << "\n";
        QMessageBox::critical(this, tr("Error"), value);
        break;
    case '!': //須全部停止的錯誤
        this->stop();
        emit setPaint(time + value);
        out << time << "MessageBox: " << value << "\n";
        QMessageBox::critical(this, tr("Error"), value);
        break;
    case '-': //只記錄txt 不顯示
        out << time << value << "\n";
        break;
    }
    file.close();

}

void MainWindow::isStart(){
    bool isAllRun = true;
    if(!scanner->isRunning() || !com->isRunning())
        isAllRun = false;

#if defined ( RASPBERRY )
    if(!gpio->isRunning())
        isAllRun = false;
#endif

    isMode(isAllRun);
}

void MainWindow::isMode(bool value){
    this->isDetecting = value;
    if(value){
        la_status->setStyleSheet("QLabel { background-color : green; color : yellow }");
        la_status->setText(QString::fromLocal8Bit("Detection Mode. (RUN)"));
    }else{
        la_status->setStyleSheet("QLabel { background-color : red; color : yellow }");
        la_status->setText(QString::fromLocal8Bit("Alarm. (STOP)"));
    }
}

void MainWindow::displayCaptureError(int id, QCameraImageCapture::Error error, const QString &errorString){
    Q_UNUSED(id);
    Q_UNUSED(error);
    QMessageBox::warning(this, tr("Image Capture Error"), errorString);
}

void MainWindow::closeEvent(QCloseEvent *e){
    Q_UNUSED(e);
    stop(true);
    emit closeWindow();
}

MainWindow::~MainWindow()
{
    delete ui;
}
