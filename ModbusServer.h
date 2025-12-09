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

//namespace Ui { class fmodbus_server; }

QT_END_NAMESPACE

class ModbusServer : public QWidget, public ModbusBase {
Q_OBJECT
public:
    explicit ModbusServer(QWidget *parent = nullptr, QString config = "", ModbusSettings = ModbusSettings());
    ~ModbusServer();

    std::map<QString, QModbusDataUnit> get_mdum() const {return _mdum;};
    QModbusDataUnitMap get_reg() const {return _reg;};

    void set_modbus_settings(ModbusSettings modbus_settings);
signals:
    void host_error(QString error);
    void host_connected();
    void host_connecting();
    void host_disconnected();

    void on_connected();
    void on_connecting();
    void on_disconnected();

    void on_state_changed(int state);

    void read_ready(QString reg_name, QModbusDataUnit mdu);
public slots:
    void on_connect_type(int index);
    void on_connect();
    void connectToHost() {this->on_connect();};
    void disconnectFromHost();

    void setRegister(QModbusDataUnit::RegisterType table, const QString &reg_name, double value);
    void setRegisterString(QModbusDataUnit::RegisterType table, const QString &reg_name, const QString &value);
    void updateRegisters(QModbusDataUnit::RegisterType table, int address, int size);

    // void read_config();
    // void write_config();
private slots:
    void onModbusStateChanged(int state);
    void onModbusErrorOccurred(QModbusDevice::Error error);
    void settings_change();

    void checkSerialPorts();
private:
    bool isSerialPortAvailable(const QString &portName);

    //Ui::fmodbus_server *ui;

    //QModbusReply *lastRequest = nullptr;
    QModbusServer *modbusDevice = nullptr;

    ModbusSettings _modbus_settings;
    ModbusConnection _connection_type = ModbusTCP;

    // bool _requestInProgress = false;
    // QTimer *_reconnectTimer = nullptr;
    // int _failedAttempts = 0;
    // const int MAX_FAILED_ATTEMPTS = 3;

    // QQueue<QModbusDataUnit> _requestQueue;
    // bool _processingQueue = false;
    // const int MAX_QUEUE_SIZE = 3;

    QTimer *_portCheckTimer;  // Таймер для проверки портов
    QString _lastKnownPortState;  // Для отслеживания изменений состояния порта
};

#endif // MODBUSSERVER_H
