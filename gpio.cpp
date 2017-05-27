#include "gpio.h"

#if defined ( RASPBERRY )
#include <wiringPi.h>
#endif

#include <QDebug>

#define BUTTON 1  //wiringPi 1 is GPIO 18 is pin 12

GPIO::GPIO()
{
    this->quit = false;
    this->isCollect = false;
    this->isGPIO = true;

#if defined ( RASPBERRY )
    if(wiringPiSetup() == -1)
        this->isGPIO = false;
    pinMode(BUTTON,INPUT);
#endif
}

void GPIO::setGPIO(int delay_gpio){
    this->delay_gpio = delay_gpio;
}

void GPIO::CollectEND(){
    this->isCollect = false;    //關閉收尋狀態(開啟要求)
}

void GPIO::stop(){
    this->quit = true;
}

void GPIO::run(){
    this->quit = false;

#if defined ( RASPBERRY )
    if(!isGPIO){
        this->quit = true;
        emit status("X.. GPIO 啟動失敗,請重開程式並確認問題: wiringPiSetup() == -1");
    }

    while(!quit){
        if(digitalRead(BUTTON) == 1 && !isCollect){     //如果沒有在收集中
            emit Collect(">.. GPIO -> Scanning ->");    //發出收集訊號要求條碼傳出
            this->isCollect = true;
        }
        msleep(delay_gpio);
    }
#endif
}

GPIO::~GPIO(){
    this->quit = true;
}
