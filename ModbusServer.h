#ifndef MODBUSSERVER_H
#define MODBUSSERVER_H

#include <QButtonGroup>
#include <QLineEdit>
#include <QModbusServer>

#include <QMainWindow>
#include <QModbusDataUnit>
#include <QModbusDevice>

#include "Config.h"

#include "ModbusBase.h"

QT_BEGIN_NAMESPACE

namespace Ui { class fmodbus_server; }

QT_END_NAMESPACE

class ModbusServer : public QWidget, public ModbusBase {
Q_OBJECT
public:
    explicit ModbusServer(QWidget *parent = nullptr);
    ~ModbusServer();

    std::map<QString, QModbusDataUnit> get_mdum() const {return _mdum;};
    QModbusDataUnitMap get_reg() const {return _reg;};
signals:
    void read_ready(QString reg_name, QModbusDataUnit mdu);
public slots:
    void on_connect_type(int index);
    void on_connect();
    //void onCurrentConnectTypeChanged(int);

    void setRegister(QModbusDataUnit::RegisterType table, const QString &reg_name, double value);
    void updateRegisters(QModbusDataUnit::RegisterType table, int address, int size);

    void read_config();
    void write_config();
private slots:
    void onModbusStateChanged(int state);
    void onModbusErrorOccurred(QModbusDevice::Error error);
    void settings_change();
private:
    Ui::fmodbus_server *ui;

    ModbusConnection _connection_type = Tcp;
    int _server_address = 1;
    QString _address = "192.168.1.3:502";

    bool _request = false;

    QModbusServer *modbusDevice = nullptr;
};

#endif // MODBUSSERVER_H
