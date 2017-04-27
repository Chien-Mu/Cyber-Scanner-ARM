#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QPlainTextEdit>
#include <QImage>
#include <QFile>
#include <QDir>

#include "camera.h"
#include "shared.h"
#include "config.h"
#include "serialportthread.h"
#include "gpio.h"
#include "about.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    bool Start();
    QImage *on_Capture();
    QImage currentImage;
    QImage *currentImage_ptr;
    void stop(bool isCloseCamera = false);

public slots:
    void setStatus(QString value);

private:
    //ui
    Ui::MainWindow *ui;
    QLabel *la_status;
    QPlainTextEdit *formText;
    QPalette p;

    //camera
    Camera *camera;
    VideoWidget *videoWidget;

    //other
    About *about;
    Shared *shared;
    Config *config;
    GPIO *gpio;
    SerialPortThread *com;
    QFile file;
    QDir dir;

    //function
    void isMode(bool value);

    //property
    bool isDetecting; //主狀態

protected:
    virtual void closeEvent(QCloseEvent *e);

private slots:
    void trigerMenu(QAction* act);
    void displayCaptureError(int id,QCameraImageCapture::Error error,const QString &errorString);
    void isStart();

signals:
    void setPaint(QString value);
    void closeWindow();
};

#endif // MAINWINDOW_H
