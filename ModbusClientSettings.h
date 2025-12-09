#ifndef MODBUSCLIENTSETTINGS_H
#define MODBUSCLIENTSETTINGS_H

#include <QWidget>

#include "Config.h"

namespace Ui { class fmodbus_client; }

class ModbusClientSettings : public QWidget {
    Q_OBJECT
public:
    ModbusClientSettings(QWidget *parent = nullptr);
    ~ModbusClientSettings();


signals:
    void settings_changed();
    void poll_timer_changed(int value);
public slots:
    ModbusSettings getSettingsFromUi();
    void setSettingsToUi(const ModbusSettings& settings);

    int get_poll_timer() const;

    void onModbusStateChanged(int state);

    void read_config();
    void write_config();
private slots:

private:
    void initializeModbusComboboxes();

    Ui::fmodbus_client *ui;

};

#endif // MODBUSCLIENTSETTINGS_H
