#include "config.h"
#include "ui_config.h"
#include <QtSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include <QMessageBox>
#include <QDebug>

Config::Config(QWidget *parent) : QWidget(parent),ui(new Ui::Config)
{
    ui->setupUi(this);
    connect(ui->btn_close,SIGNAL(clicked()),this,SLOT(closeWindow()));

    //CREATE DIR
    if(!dir.mkpath(QDir::homePath()+"/ScannerConfig"))
        QMessageBox::critical(this,tr("Error"),"!.. " + QDir::homePath()+"/ScannerConfig -> The dir an not be created.");

    //驗證器
    validInt = new QIntValidator(this); //限制只能 int

    getSerialInfo();

    //ui載入設定選項
    setUI();

    //load config
    getConfig(); 
}

void Config::getSerialInfo(){
    ui->cb_SP->clear();
    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts())
        ui->cb_SP->addItem(info.portName(),info.serialNumber());
}

void Config::getConfig(){
    QString txtPath = QDir::homePath() + "/ScannerConfig/config.txt";
    file.setFileName(txtPath);
    if(!file.open(QIODevice::ReadOnly|QIODevice::Text)){
        QMessageBox::critical(this, tr("Error"), txtPath+ "..does not exist.");
        return;
    }

    QTextStream in(&file);
    QString line,item;
    while(!in.atEnd()){
        line = "";
        item = "";
        line = in.readLine();
        item = line.left(3);

        if(item == "COM"){
            line = line.mid(4,line.length());
            for(int i=0;i<ui->cb_SP->count();i++)
                if(line == ui->cb_SP->itemText(i)){
                    ui->cb_SP->setCurrentIndex(i);
                    break;
                }
        }else if(item == "BAR"){
            line = line.mid(4,line.length());
            for(int i=0;i<ui->cb_Baud->count();i++)
                if(line == ui->cb_Baud->itemData(i).toString()){
                    ui->cb_Baud->setCurrentIndex(i);
                    break;
                }
        }else if(item == "DAB"){
            line = line.mid(4,line.length());
            for(int i=0;i<ui->cb_DataBits->count();i++)
                if(line == ui->cb_DataBits->itemData(i).toString()){
                    ui->cb_DataBits->setCurrentIndex(i);
                    break;
                }
        }else if(item == "PAY"){
            line = line.mid(4,line.length());
            for(int i=0;i<ui->cb_Parity->count();i++)
                if(line == ui->cb_Parity->itemData(i).toString()){
                    ui->cb_Parity->setCurrentIndex(i);
                    break;
                }
        }else if(item == "STB"){
            line = line.mid(4,line.length());
            for(int i=0;i<ui->cb_StopBits->count();i++)
                if(line == ui->cb_StopBits->itemData(i).toString()){
                    ui->cb_StopBits->setCurrentIndex(i);
                    break;
                }
        }else if(item == "FLC"){
            line = line.mid(4,line.length());
            for(int i=0;i<ui->cb_FlowControl->count();i++)
                if(line == ui->cb_FlowControl->itemData(i).toString()){
                    ui->cb_FlowControl->setCurrentIndex(i);
                    break;
                }
        }else if(item == "LSN"){
            ui->cb_length_SN->setCurrentText(line.mid(4,line.length()));
        }else if(item == "DEL"){
            ui->cb_delay_loop->setCurrentText(line.mid(4,line.length()));
        }else if(item == "DED"){
            ui->cb_delay_dmtx->setCurrentText(line.mid(4,line.length()));
        }else if(item == "TMO"){
            ui->cb_scanTimeout->setCurrentText(line.mid(4,line.length()));
        }else if(item == "DEP"){
            ui->cb_gpio_delay->setCurrentText(line.mid(4,line.length()));
        }else if(item == "ROX"){
            this->ROI.setX(line.mid(4,line.length()).toInt());
        }else if(item == "ROY"){
            this->ROI.setY(line.mid(4,line.length()).toInt());
        }else if(item == "ROW"){
            this->ROI.setWidth(line.mid(4,line.length()).toInt());
        }else if(item == "ROH"){
            this->ROI.setHeight(line.mid(4,line.length()).toInt());
        }
    }

    file.close();
}

void Config::setConfig(){
    QString txtPath = QDir::homePath() + "/ScannerConfig/config.txt";
    file.setFileName(txtPath);
    if(!file.open(QIODevice::WriteOnly|QIODevice::Text)){ //檔案會覆蓋
        QMessageBox::critical(this, tr("Error"), txtPath+ "..does not exist.");
        return;
    }
    QTextStream out(&file);

    out << "LSN=" + QString::number(get_length_SN()) + "\n";
    out << "DEL=" + QString::number(get_delay_loop()) + "\n";
    out << "DED=" + QString::number(get_delay_dmtx()) + "\n";
    out << "TMO=" + QString::number(get_scanTimout()) + "\n";

    out << "COM=" + get_COM() + "\n";
    out << "BAR=" + QString::number(get_Baud()) + "\n";
    out << "DAB=" + QString::number(get_DataB()) + "\n";
    out << "PAY=" + QString::number(get_Parity()) + "\n";
    out << "STB=" + QString::number(get_StopB()) + "\n";
    out << "FLC=" + QString::number(get_FlowC()) + "\n";

    out << "DEP=" + QString::number(get_delay_gpio()) + "\n";

    out << "ROX=" + QString::number(this->ROI.x()) + "\n";
    out << "ROY=" + QString::number(this->ROI.y()) + "\n";
    out << "ROW=" + QString::number(this->ROI.width()) + "\n";
    out << "ROH=" + QString::number(this->ROI.height()) + "\n";
    file.close();
}

void Config::setUI(){
    //com
    QVector<QByteArray> list;
    list.push_back("BaudRate");
    list.push_back("DataBits");
    list.push_back("Parity");
    list.push_back("StopBits");
    list.push_back("FlowControl");

    QSerialPort myObj;
    const QMetaObject* metaObj =  myObj.metaObject();

    QMetaEnum enumType =  metaObj->enumerator(metaObj->indexOfEnumerator(list[0]));
    for(int i=0; i < enumType.keyCount(); ++i)
        ui->cb_Baud->addItem(QString::fromLocal8Bit(enumType.key(i)),QVariant::fromValue(enumType.value(i)));
    ui->cb_Baud->setCurrentText("Baud9600"); //default

    enumType =  metaObj->enumerator(metaObj->indexOfEnumerator(list[1]));
    for(int i=0; i < enumType.keyCount(); ++i)
        ui->cb_DataBits->addItem(QString::fromLocal8Bit(enumType.key(i)),QVariant::fromValue(enumType.value(i)));
    ui->cb_DataBits->setCurrentText("Data8"); //default

    enumType =  metaObj->enumerator(metaObj->indexOfEnumerator(list[2]));
    for(int i=0; i < enumType.keyCount(); ++i)
        ui->cb_Parity->addItem(QString::fromLocal8Bit(enumType.key(i)),QVariant::fromValue(enumType.value(i)));
    ui->cb_Parity->setCurrentText("NoParity"); //default

    enumType =  metaObj->enumerator(metaObj->indexOfEnumerator(list[3]));
    for(int i=0; i < enumType.keyCount(); ++i)
        ui->cb_StopBits->addItem(QString::fromLocal8Bit(enumType.key(i)),QVariant::fromValue(enumType.value(i)));
    ui->cb_StopBits->setCurrentText("OneStop"); //default

    enumType =  metaObj->enumerator(metaObj->indexOfEnumerator(list[4]));
    for(int i=0; i < enumType.keyCount(); ++i)
        ui->cb_FlowControl->addItem(QString::fromLocal8Bit(enumType.key(i)),QVariant::fromValue(enumType.value(i)));
    ui->cb_FlowControl->setCurrentText("NoFlowControl"); //default

    //scanner
    for(int i=1;i<=30;i++)
        ui->cb_length_SN->addItem(QString::number(i));
    for(int i=0;i<=1000;i+=50){
        ui->cb_delay_dmtx->addItem(QString::number(i));
        ui->cb_delay_loop->addItem(QString::number(i));
        ui->cb_gpio_delay->addItem(QString::number(i)); //gpio
    }
    for(int i=0;i<30;i++)
        ui->cb_scanTimeout->addItem(QString::number(i));
    //default
    ui->cb_length_SN->setCurrentText(QString::number(15));
    ui->cb_delay_dmtx->setCurrentText(QString::number(200));
    ui->cb_delay_loop->setCurrentText(QString::number(0));
    ui->cb_gpio_delay->setCurrentText(QString::number(300));
    ui->cb_scanTimeout->setCurrentText(QString::number(8));
}

void Config::saveROI(QRect ROI){
    this->ROI = ROI;
}

void Config::closeWindow(){
    this->close();
}

int Config::get_length_SN(){ return ui->cb_length_SN->currentText().toInt(); }
int Config::get_delay_loop(){ return ui->cb_delay_loop->currentText().toInt(); }
int Config::get_delay_dmtx(){ return ui->cb_delay_dmtx->currentText().toInt(); }
int Config::get_scanTimout(){ return ui->cb_scanTimeout->currentText().toInt(); }

QString Config::get_COM(){ return ui->cb_SP->currentText(); }
int Config::get_Baud(){ return ui->cb_Baud->currentData().toInt(); }
int Config::get_DataB(){ return ui->cb_DataBits->currentData().toInt(); }
int Config::get_Parity(){ return ui->cb_Parity->currentData().toInt(); }
int Config::get_StopB(){ return ui->cb_StopBits->currentData().toInt(); }
int Config::get_FlowC(){ return ui->cb_FlowControl->currentData().toInt(); }

int Config::get_delay_gpio(){ return ui->cb_gpio_delay->currentText().toInt(); }
QRect Config::get_ROI(){ return this->ROI; }

Config::~Config()
{
    delete ui;
}
