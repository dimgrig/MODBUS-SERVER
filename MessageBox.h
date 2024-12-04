#ifndef MESSAGEBOX_H
#define MESSAGEBOX_H

#include <QMessageBox>

class MessageBox : public QMessageBox {
public:
    MessageBox(QWidget *parent);

    void set_delay(int delay = 0) {_delay = delay;};
    void set_close(int close = 0) {_close = close;};
private:
    void showEvent(QShowEvent *event);
    void timerEvent(QTimerEvent *event);

    unsigned int _delay = 0;
    unsigned int _close = 0;
    QString _informative_text;
};

#endif // MESSAGEBOX_H
