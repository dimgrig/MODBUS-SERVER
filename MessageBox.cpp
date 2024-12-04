#include "MessageBox.h"

#include "Logger.h"

#include <QTimer>

MessageBox::MessageBox(QWidget *parent) :
    QMessageBox(parent)
{

}

void MessageBox::showEvent(QShowEvent * event) {
    QTimer::singleShot(_delay, [=](){
        _informative_text = this->informativeText();
        if (_close != 0) {
            this->setInformativeText(_informative_text + " (" + QString::number((unsigned int)(_close / 1000)) + ")");
            this->startTimer(1000);
        }

        QMessageBox::showEvent(event);
    });
}

void MessageBox::timerEvent(QTimerEvent *event) {
    int close = (int)_close - 1000;
    if (close <= 0) {
        this->done(0);
    } else {
        _close -= 1000;
        //qCDebug(logDebug) << _close;
        this->setInformativeText(_informative_text + " (" + QString::number((unsigned int)(_close / 1000)) + ")");
    }

    QMessageBox::timerEvent(event);
}
