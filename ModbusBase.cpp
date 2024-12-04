#include "ModbusBase.h"

#include "Logger.h"

ModbusBase::ModbusBase()
{

}

QModbusDataUnitMap ModbusBase::setupDeviceData(std::map<QString, QModbusDataUnit> &mdum) {
    QModbusDataUnitMap reg;

    for (const auto &value : mdum) {
        if (reg.count(value.second.registerType()) == 0) {
            reg.insert(value.second.registerType(), value.second);
            reg[value.second.registerType()].setStartAddress(reg[value.second.registerType()].startAddress()); // + 1
        } else {
            if (reg[value.second.registerType()].startAddress() > value.second.startAddress()) {
                reg[value.second.registerType()].setValueCount(reg[value.second.registerType()].startAddress() -
                                                               value.second.startAddress() +
                                                               reg[value.second.registerType()].valueCount());
                reg[value.second.registerType()].setStartAddress(value.second.startAddress()); // + 1
            } else {
                if ((reg[value.second.registerType()].startAddress() +
                     reg[value.second.registerType()].valueCount()) >=
                    (value.second.startAddress() + value.second.valueCount())) {
                    ; //reg[value.second.registerType()].setValueCount(value.second.startAddress());
                } else {
                    reg[value.second.registerType()].setValueCount(value.second.startAddress() -
                                                                   reg[value.second.registerType()].startAddress() +
                                                                   value.second.valueCount());
                }
            }
        }
    }
    {
        //size fix
        for (const auto &r : reg) {
            QModbusDataUnit mdu = QModbusDataUnit(r.registerType(), r.startAddress(), r.valueCount());
            reg.insert(r.registerType(), mdu);
        }
    }
    //manually - not calculated from json
    //reg.insert(QModbusDataUnit::HoldingRegisters, { QModbusDataUnit::HoldingRegisters, 0, 28 });

//    qCDebug(logDebug()) << "QModbusDataUnitMap" << reg.size();
//    for (const auto &r : reg) {
//        qCDebug(logDebug()) << r.registerType() << r.startAddress() << r.valueCount() << r.values();
//    }

    return reg;
}


std::map<QString, QModbusDataUnit> ModbusBase::update_mdu(QModbusDataUnit::RegisterType table, int address, int size,
                                                          std::map<QString, QModbusDataUnit> &mdum) {
    std::map<QString, QModbusDataUnit> res ;
    for (const auto & mdu : mdum) {
        if (mdu.second.registerType() == table) {
            if ((mdu.second.startAddress() <= address) &&
                ((mdu.second.startAddress() + mdu.second.valueCount()) > address)) {
                res[mdu.first] = mdu.second;
                break;
            }
            if ((mdu.second.startAddress() >= address) &&
                (mdu.second.startAddress() < (address + size))) {
                res[mdu.first] = mdu.second;
            }
        }
    }
    return res;
}

void ModbusBase::on_read_json(std::map<QString, QModbusDataUnit> &mdum) {
    std::map<std::string, std::string> map;

    QScopedPointer<QFile> mapFile(new QFile(":json/json/modbus.json"));
    mapFile.data()->open(QIODevice::ReadOnly);
    QString val = mapFile.data()->readAll();
    mapFile.data()->close();

    QJsonDocument doc = QJsonDocument::fromJson(val.toUtf8());
    //QJsonObject jObject = doc.object();
    QJsonArray json_array = doc.array();
    //qCDebug(logDebug()) << doc.isArray() << json_array.size();

//    QVariantMap dataMap = (((mainMap[(index + "_map").c_str()].toMap())["mappings"].toMap())[index.c_str()].toMap())["properties"].toMap();
//    QVariantMap dataMap = mainMap;

//    for(QVariantMap::const_iterator it = dataMap.begin(); it != dataMap.end(); ++it) {
//        qCDebug(logDebug()) << it.key() << it.value();
//        //map[it.key().toStdString()] = ((it.value().toMap())["type"]).toString().toStdString();
//    }

//    foreach (const QJsonValue & value, json_array) {
//        QJsonObject obj = value.toObject();
//        qCDebug(logDebug()) << obj["RegisterName"].toString() << obj["ObjName"].toString() << obj["Address"].toInt() << obj["DataLength"].toInt();
//    }

    QString name;

    foreach (const QJsonValue & value, json_array) {
        QJsonObject obj = value.toObject();

        QString reg_name = obj["RegisterName"].toString();
        QString obj_name = obj["ObjName"].toString();
        int address = obj["Address"].toInt();
        QString reg_type = obj["RegisterType"].toString();
        quint16 size = obj["DataLength"].toInt();

        QModbusDataUnit::RegisterType register_type = QModbusDataUnit::RegisterType::Invalid;
        if (obj_name == "COIL") {
            register_type = QModbusDataUnit::RegisterType::Coils;
        } else if (obj_name == "INPUT") {
            register_type = QModbusDataUnit::RegisterType::InputRegisters;
        } else if (obj_name == "HOLDING") {
            register_type = QModbusDataUnit::RegisterType::HoldingRegisters;
        } else if (obj_name == "DISCRETEINPUT") {
            register_type = QModbusDataUnit::RegisterType::DiscreteInputs;
        }

        QModbusDataUnit mdu(register_type, address, size); // - 1

        mdum.emplace(reg_name, mdu);

    }


    //filter complex regs
//    auto _mdum_copy = _mdum;
//    _mdum.clear();
//    for (auto &value : _mdum_copy) {
//        if ((value.first.end() - 1)->isDigit()) {
//            QStringList sl = value.first.split("_");
//            sl.pop_back();
//            QString reg_name = sl.join("_");
//            if (_mdum.count(reg_name) > 0) {
//                QModbusDataUnit mdu = _mdum.at(reg_name);
//                mdu.setValueCount(mdu.valueCount() + value.second.valueCount());
//                _mdum[reg_name] = mdu;
//            } else {
//                _mdum[reg_name] = value.second;
//            }
//        } else {
//            _mdum[value.first] = value.second;
//        }
//    }

    qCDebug(logDebug()) << "mdum" <<  mdum.size();
    for (const auto &mdu : mdum) {
        qCDebug(logDebug()) << mdu.first << mdu.second.registerType() << mdu.second.startAddress() << mdu.second.valueCount() << mdu.second.values();
    }
}
