#-------------------------------------------------
#
# Project created by QtCreator 2017-04-10T14:03:20
#
#-------------------------------------------------

QT       += core gui
QT       += opengl
QT       += multimedia multimediawidgets
QT       += serialport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Scanner-Arm
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    myvideosurface.cpp \
    videowidget.cpp \
    camera.cpp \
    scanthread.cpp \
    serialportthread.cpp \
    config.cpp \
    shared.cpp \
    gpio.cpp \
    about.cpp

HEADERS  += mainwindow.h \
    myvideosurface.h \
    videowidget.h \
    camera.h \
    scanthread.h \
    serialportthread.h \
    config.h \
    shared.h \
    gpio.h \
    about.h

FORMS    += mainwindow.ui \
    config.ui \
    about.ui

macx{
    #libdmtx
    INCLUDEPATH += /usr/local/include
    LIBS += /usr/local/lib/libdmtx.dylib
}
unix:!macx{
    #preprocessor definition
    DEFINES += RASPBERRY

    #libdmtx
    INCLUDEPATH += /home/levy/raspi/sysroot/usr/include
    LIBS += -L/home/levy/raspi/sysroot/usrlib -ldmtx

    #WiringPi
    INCLUDEPATH += /home/levy/raspi/sysroot/usr/include
    LIBS += -L/home/levy/raspi/sysroot/usr/lib -lwiringPi

    #Cross Compile
    target.path = /home/pi/test
    INSTALLS += target
}
win32{
    #libdmtx
    INCLUDEPATH += C:\libdmtx-libdmtx\include
    LIBS += C:\libdmtx-libdmtx\libdmtx.lib
}

RESOURCES += \
    resource.qrc
