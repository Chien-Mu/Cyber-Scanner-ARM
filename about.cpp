#include "about.h"
#include "ui_about.h"

About::About(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::About)
{
    ui->setupUi(this);
    connect(ui->btn_close,SIGNAL(clicked()),this,SLOT(on_close()));

    //QPixmap img(":/img/Logo.png");
    //ui->la_show->setPixmap(img);
}

void About::on_close(){
    this->close();
}

About::~About()
{
    delete ui;
}
