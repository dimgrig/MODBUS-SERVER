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

#include "enum_macro.h"

ENUM_MACRO2(ModbusConnection, ModbusSerial, ModbusTCP,
            Serial, TCP,
            Serial, TCP,
            0, 1);

ENUM_MACRO8(ModbusBaudRate, ModbusBaud1200 = 1200, ModbusBaud2400 = 2400, ModbusBaud4800 = 4800, ModbusBaud9600 = 9600,
            ModbusBaud19200 = 19200, ModbusBaud38400 = 38400, ModbusBaud57600 = 57600, ModbusBaud115200 = 115200,
            1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200,
            1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200,
            1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200);

ENUM_MACRO4(ModbusDataBits, ModbusDataBits5 = 5, ModbusDataBits6 = 6, ModbusDataBits7 = 7, ModbusDataBits8 = 8,
            5, 6, 7, 8,
            5, 6, 7, 8,
            5, 6, 7, 8);

ENUM_MACRO3(ModbusStopBits, ModbusStopBits_OneStop = 1, ModbusStopBits_TwoStop = 2, ModbusStopBits_OneAndHalfStop = 3,
            1, 2, 1.5,
            1, 2, 1.5,
            1, 2, 3);

ENUM_MACRO5(ModbusParity, ModbusParity_NoParity = 0, ModbusParity_EvenParity = 2, ModbusParity_OddParity = 3,
            ModbusParity_SpaceParity = 4, ModbusParity_MarkParity = 5,
            NoParity, EvenParity, OddParity, SpaceParity, MarkParity,
            NoParity, EvenParity, OddParity, SpaceParity, MarkParity,
            0, 2, 3, 4, 5);

static QStringList ModbusConnection_sl = {{ModbusConnection_to_text(ModbusSerial),
                                           ModbusConnection_to_text(ModbusTCP)}};
static QStringList ModbusBaudRate_sl = {{ModbusBaudRate_to_text(ModbusBaud1200),
                                         ModbusBaudRate_to_text(ModbusBaud2400),
                                         ModbusBaudRate_to_text(ModbusBaud4800),
                                         ModbusBaudRate_to_text(ModbusBaud9600),
                                         ModbusBaudRate_to_text(ModbusBaud19200),
                                         ModbusBaudRate_to_text(ModbusBaud38400),
                                         ModbusBaudRate_to_text(ModbusBaud57600),
                                         ModbusBaudRate_to_text(ModbusBaud115200)}};
static QStringList ModbusDataBits_sl = {{ModbusDataBits_to_text(ModbusDataBits5),
                                         ModbusDataBits_to_text(ModbusDataBits6),
                                         ModbusDataBits_to_text(ModbusDataBits7),
                                         ModbusDataBits_to_text(ModbusDataBits8)}};
static QStringList ModbusStopBits_sl = {{ModbusStopBits_to_text(ModbusStopBits_OneStop),
                                         ModbusStopBits_to_text(ModbusStopBits_TwoStop),
                                         ModbusStopBits_to_text(ModbusStopBits_OneAndHalfStop)}};
static QStringList ModbusParity_sl = {{ModbusParity_to_text(ModbusParity_NoParity),
                                       ModbusParity_to_text(ModbusParity_EvenParity),
                                       ModbusParity_to_text(ModbusParity_OddParity),
                                       ModbusParity_to_text(ModbusParity_SpaceParity),
                                       ModbusParity_to_text(ModbusParity_MarkParity)}};

class ModbusBase {
public:
    ModbusBase();

    std::map<QString, QModbusDataUnit> get_mdum() const {return _mdum;};
    QModbusDataUnitMap get_reg() const {return _reg;};
    std::vector<QModbusDataUnitMap> get_reg_v() const {return _reg_v;};

    std::vector<std::pair<QString, QModbusDataUnit>> get_sort_by_start_address_mdum_v() const;
    std::map<QString, QString> get_reg_type_m() const {return _reg_type_m;};
protected:
    void setupDeviceData();
    std::map<QString, QModbusDataUnit> update_mdu(QModbusDataUnit::RegisterType table, int address, int size, std::map<QString, QModbusDataUnit> &mdum);

    void on_read_json();

    std::map<QString, QModbusDataUnit> _mdum;
    QModbusDataUnitMap _reg; //no blanc space between registers
    std::vector<QModbusDataUnitMap> _reg_v; //vector for splitting blanc space between registers
    std::map<QString, QString> _reg_type_m; //map reg_name to type
};

#endif // MODBUSBASE_H
