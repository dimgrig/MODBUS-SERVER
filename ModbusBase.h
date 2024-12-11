#ifndef MODBUSBASE_H
#define MODBUSBASE_H

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFrame>
#include <QObject>
#include <QTimer>
#include <QToolButton>
#include <QVariant>

#include <QModbusDataUnit>

enum ModbusConnection {
    Serial,
    Tcp
};

class ModbusBase {
public:
    ModbusBase();

    std::map<QString, QModbusDataUnit> get_mdum() const {return _mdum;};
    QModbusDataUnitMap get_reg() const {return _reg;};
    std::vector<QModbusDataUnitMap> get_reg_v() const {return _reg_v;};

    std::vector<std::pair<QString, QModbusDataUnit>> get_sort_by_start_address_mdum_v() const;
protected:
    void setupDeviceData();
    std::map<QString, QModbusDataUnit> update_mdu(QModbusDataUnit::RegisterType table, int address, int size, std::map<QString, QModbusDataUnit> &mdum);

    void on_read_json();

    std::map<QString, QModbusDataUnit> _mdum;
    QModbusDataUnitMap _reg; //no blanc space between registers
    std::vector<QModbusDataUnitMap> _reg_v; //vector for splitting blanc space between registers
};

#endif // MODBUSBASE_H
