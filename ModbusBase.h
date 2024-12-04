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

    QModbusDataUnitMap setupDeviceData(std::map<QString, QModbusDataUnit> &mdum);
    std::map<QString, QModbusDataUnit> update_mdu(QModbusDataUnit::RegisterType table, int address, int size, std::map<QString, QModbusDataUnit> &mdum);

    void on_read_json(std::map<QString, QModbusDataUnit> &mdum);
};

#endif // MODBUSBASE_H
