

#ifndef ASTRA_SLEEPER_H
#define ASTRA_SLEEPER_H

#include <QThread>

class Sleeper : public QThread
{
public:
    static void usleep(unsigned long usecs){
        QThread::usleep(usecs);
    }
    static void msleep(unsigned long msecs){
        QThread::msleep(msecs);
    }
    static void sleep(unsigned long secs){
        QThread::sleep(secs);
    }
};


#endif //ASTRA_SLEEPER_H
