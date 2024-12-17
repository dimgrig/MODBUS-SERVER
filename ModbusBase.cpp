#include "ModbusBase.h"

#include "Logger.h"

void mdum_v_to_reg_v(const std::vector<std::pair<QString, QModbusDataUnit>> &mdum_v, std::vector<QModbusDataUnitMap> &reg_v) {
    for (const auto &_mdu : mdum_v) {
        if (reg_v.empty()) {
            QModbusDataUnitMap mdum;
            mdum[_mdu.second.registerType()] = _mdu.second;
            reg_v.push_back(mdum);
        } else {
            //qCDebug(logDebug()) << reg_v.back()[_mdu.second.registerType()].startAddress() + reg_v.back()[_mdu.second.registerType()].valueCount() << _mdu.second.startAddress();
            if ((reg_v.back()[_mdu.second.registerType()].startAddress() + reg_v.back()[_mdu.second.registerType()].valueCount()) ==
                (_mdu.second.startAddress())) {
                reg_v.back()[_mdu.second.registerType()].setValueCount(reg_v.back()[_mdu.second.registerType()].valueCount() + _mdu.second.valueCount());
//                qCDebug(logDebug()) << reg_v.back()[_mdu.second.registerType()].registerType() << reg_v.back()[_mdu.second.registerType()].startAddress() <<
//                                       reg_v.back()[_mdu.second.registerType()].valueCount() << reg_v.back()[_mdu.second.registerType()].values();
            } else {
                QModbusDataUnitMap mdum;
                mdum[_mdu.second.registerType()] = _mdu.second;
                reg_v.push_back(mdum);
            }
        }
    }
}

ModbusBase::ModbusBase()
{

}

void ModbusBase::setupDeviceData() {
    QModbusDataUnitMap reg;

    for (const auto &value : this->_mdum) {
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

    this->_reg = reg;

    ////*************************

    std::vector<QModbusDataUnitMap> reg_di_v;
    std::vector<QModbusDataUnitMap> reg_coil_v;
    std::vector<QModbusDataUnitMap> reg_ir_v;
    std::vector<QModbusDataUnitMap> reg_hr_v;

    auto mdum_v = this->get_sort_by_start_address_mdum_v();
    std::vector<std::pair<QString, QModbusDataUnit>> mdum_di_v;
    std::vector<std::pair<QString, QModbusDataUnit>> mdum_coil_v;
    std::vector<std::pair<QString, QModbusDataUnit>> mdum_ir_v;
    std::vector<std::pair<QString, QModbusDataUnit>> mdum_hr_v;

    for (const auto &_mdu : mdum_v) {
        auto mdu = _mdu.second;
        switch (mdu.registerType()) {
            case QModbusDataUnit::DiscreteInputs:
                mdum_di_v.push_back(_mdu);
            break;
            case QModbusDataUnit::Coils:
                mdum_coil_v.push_back(_mdu);
            break;
            case QModbusDataUnit::InputRegisters:
                mdum_ir_v.push_back(_mdu);
            break;
            case QModbusDataUnit::HoldingRegisters:
                mdum_hr_v.push_back(_mdu);
            break;
            default:

            break;
        }
    }

//    qCDebug(logDebug()) << "mdum_di_v";
//    for (const auto &mdum : mdum_di_v) {
//        auto r = mdum.second;
//        qCDebug(logDebug()) << r.registerType() << r.startAddress() << r.valueCount() << r.values();
//    }
//    qCDebug(logDebug()) << "mdum_coil_v";
//    for (const auto &mdum : mdum_coil_v) {
//        auto r = mdum.second;
//        qCDebug(logDebug()) << r.registerType() << r.startAddress() << r.valueCount() << r.values();
//    }
//    qCDebug(logDebug()) << "mdum_ir_v";
//    for (const auto &mdum : mdum_ir_v) {
//        auto r = mdum.second;
//        qCDebug(logDebug()) << r.registerType() << r.startAddress() << r.valueCount() << r.values();
//    }
//    qCDebug(logDebug()) << "mdum_hr_v";
//    for (const auto &mdum : mdum_hr_v) {
//        auto r = mdum.second;
//        qCDebug(logDebug()) << r.registerType() << r.startAddress() << r.valueCount() << r.values();
//    }

    mdum_v_to_reg_v(mdum_di_v, reg_di_v);
    mdum_v_to_reg_v(mdum_coil_v, reg_coil_v);
    mdum_v_to_reg_v(mdum_ir_v, reg_ir_v);
    mdum_v_to_reg_v(mdum_hr_v, reg_hr_v);

    _reg_v.insert(_reg_v.end(), reg_di_v.begin(), reg_di_v.end());
    _reg_v.insert(_reg_v.end(), reg_coil_v.begin(), reg_coil_v.end());
    _reg_v.insert(_reg_v.end(), reg_ir_v.begin(), reg_ir_v.end());
    _reg_v.insert(_reg_v.end(), reg_hr_v.begin(), reg_hr_v.end());

//    qCDebug(logDebug()) << "std::vector<QModbusDataUnitMap>" << _reg_v.size() << reg_di_v.size() << reg_coil_v.size() << reg_ir_v.size() << reg_hr_v.size();
//    for (const auto &reg : _reg_v) {
//        for (const auto &r : reg) {
//            qCDebug(logDebug()) << r.registerType() << r.startAddress() << r.valueCount() << r.values();
//        }
//    }
}


std::map<QString, QModbusDataUnit> ModbusBase::update_mdu(QModbusDataUnit::RegisterType table, int address, int size,
                                                          std::map<QString, QModbusDataUnit> &mdum) {
    std::map<QString, QModbusDataUnit> res;
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

void ModbusBase::on_read_json() {
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

        this->_mdum.emplace(reg_name, mdu);

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

    qCDebug(logDebug()) << "mdum" <<  this->_mdum.size();
    for (const auto &value : this->_mdum) {
        qCDebug(logDebug()) << value.first << value.second.startAddress() << value.second.valueCount() << value.second.values();
    }
}

std::vector<std::pair<QString, QModbusDataUnit>> ModbusBase::get_sort_by_start_address_mdum_v() const {
    std::vector<std::pair<QString, QModbusDataUnit>> mdum_v;
    for (const auto &_mdu : this->_mdum) {
        mdum_v.push_back({_mdu.first, _mdu.second});
    }
    std::sort(mdum_v.begin(), mdum_v.end(), [](const std::pair<QString, QModbusDataUnit> &l, const std::pair<QString, QModbusDataUnit> &r){
        return l.second.startAddress() < r.second.startAddress();
    });
//    for (const auto &_mdu : mdum_v) {
//        qCDebug(logDebug()) << _mdu.first;
//    }
    return mdum_v;
}
