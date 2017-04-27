#ifndef GPIO_H
#define GPIO_H

#include <QThread>

class GPIO : public QThread
{
    Q_OBJECT
public:
    GPIO();
    ~GPIO();
    void run();
    void stop();
    void setGPIO(int delay_gpio);

public slots:
    void CollectEND();

private:
    bool quit;
    bool isCollect; //收集中時，不會再丟 Collect
    bool isGPIO; //是否啟動成功
    int delay_gpio;

signals:
    void Collect(QString value);
    void status(QString value);
};

#endif // GPIO_H
