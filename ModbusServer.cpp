#include "ModbusServer.h"
#include "ui_modbus_server.h"

#include <QModbusRtuSerialSlave>
#include <QModbusTcpServer>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QSerialPort>
#include <QStatusBar>
#include <QUrl>

#include "Config.h"

#include "functions.h"

//sudo setcap 'cap_net_bind_service=+ep' HIMIK-SERVER-TEST

ModbusServer::ModbusServer(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::fmodbus_server)
{
    ui->setupUi(this);

    this->ui->cb_connect_type->addItems(ModbusConnection_sl);
    this->ui->cb_baudrate->addItems(ModbusBaudRate_sl);
    this->ui->cb_data_bits->addItems(ModbusDataBits_sl);
    this->ui->cb_stop_bits->addItems(ModbusStopBits_sl);
    this->ui->cb_parity->addItems(ModbusParity_sl);

    this->on_read_json();

    connect(this->ui->cb_connect_type, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ModbusServer::on_connect_type);
    connect(this->ui->le_address, &QLineEdit::editingFinished, this, &ModbusServer::settings_change);
    connect(this->ui->le_ip, &QLineEdit::editingFinished, this, &ModbusServer::settings_change);
    connect(this->ui->le_port, &QLineEdit::editingFinished, this, &ModbusServer::settings_change);
    connect(this->ui->le_com_port, &QLineEdit::editingFinished, this, &ModbusServer::settings_change);
    connect(this->ui->cb_baudrate, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ModbusServer::on_connect_type);
    connect(this->ui->cb_data_bits, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ModbusServer::on_connect_type);
    connect(this->ui->cb_stop_bits, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ModbusServer::on_connect_type);
    connect(this->ui->cb_parity, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ModbusServer::on_connect_type);
}

ModbusServer::~ModbusServer() {
    if (modbusDevice)
        modbusDevice->disconnectDevice();
    delete modbusDevice;
}

void ModbusServer::on_connect_type(int index) {
    _connection_type = static_cast<ModbusConnection>(index);

    if (modbusDevice) {
        modbusDevice->disconnectDevice();
        delete modbusDevice;
        modbusDevice = nullptr;
    }

    if (_connection_type == ModbusSerial) {
        modbusDevice = new QModbusRtuSerialSlave(this);
    } else if (_connection_type == ModbusTCP) {
        modbusDevice = new QModbusTcpServer(this);
    }

    connect(modbusDevice, &QModbusDevice::errorOccurred, [this](QModbusDevice::Error) {
        qCDebug(logDebug()) << modbusDevice->errorString();
        QTimer::singleShot(1000, [=]() {
            this->on_connect();

            _request = false;
        });
    });

    if (!modbusDevice) {
        if (_connection_type == ModbusSerial) {
            qCDebug(logDebug) << (tr("Could not create Modbus master."));
        } else if (_connection_type == ModbusTCP) {
            qCDebug(logDebug) << (tr("Could not create Modbus client."));
        }
    } else {
        connect(modbusDevice, &QModbusServer::stateChanged, this, &ModbusServer::onModbusStateChanged);
        connect(modbusDevice, &QModbusServer::errorOccurred, this, &ModbusServer::onModbusErrorOccurred);
        connect(modbusDevice, &QModbusServer::dataWritten, this, &ModbusServer::updateRegisters);
    }
}

void ModbusServer::on_connect() {

    if (!modbusDevice)
        return;

    if (modbusDevice->state() != QModbusDevice::ConnectedState) {
        if (_connection_type == ModbusSerial) {
            modbusDevice->setConnectionParameter(QModbusDevice::SerialPortNameParameter, this->ui->le_com_port->text());
            modbusDevice->setConnectionParameter(QModbusDevice::SerialBaudRateParameter,
                                                 ModbusBaudRate_from_text(this->ui->cb_baudrate->currentText().toStdString().c_str())); //QSerialPort::Baud19200
            modbusDevice->setConnectionParameter(QModbusDevice::SerialDataBitsParameter,
                                                 ModbusDataBits_from_text(this->ui->cb_data_bits->currentText().toStdString().c_str())); //QSerialPort::Data8
            modbusDevice->setConnectionParameter(QModbusDevice::SerialStopBitsParameter,
                                                 ModbusStopBits_from_text(this->ui->cb_stop_bits->currentText().toStdString().c_str())); //QSerialPort::OneStop
            modbusDevice->setConnectionParameter(QModbusDevice::SerialParityParameter,
                                                 ModbusParity_from_text(this->ui->cb_parity->currentText().toStdString().c_str())); //QSerialPort::EvenParity
        } else if (_connection_type == ModbusTCP) {
            modbusDevice->setConnectionParameter(QModbusDevice::NetworkAddressParameter, this->ui->le_ip->text());
            modbusDevice->setConnectionParameter(QModbusDevice::NetworkPortParameter, this->ui->le_port->text());
        }
        modbusDevice->setServerAddress(1);
//        modbusDevice->setTimeout(2000);
//        modbusDevice->setNumberOfRetries(3);
        if (!modbusDevice->connectDevice()) {
            qCDebug(logDebug) << tr("Connect failed: ") + modbusDevice->errorString();
        } else {
            if (_connection_type == ModbusSerial) {
                qCDebug(logDebug) << tr("Connected: ") + this->ui->le_com_port->text() << modbusDevice->state();
            } else if (_connection_type == ModbusTCP) {
                qCDebug(logDebug) << tr("Connected: ") + this->ui->le_ip->text() << modbusDevice->state();
            }
            this->setupDeviceData();
            modbusDevice->setMap(this->_reg);
            for (const auto &value : this->_mdum) {
                this->modbusDevice->setData(value.second);
                //qCDebug(logDebug()) << value.first << value.second.startAddress() << value.second.valueCount() << value.second.values();
            }
        }
    } else {
        modbusDevice->disconnectDevice();
    }
}

void ModbusServer::onModbusStateChanged(int state) {
    QString state_str;
    switch (modbusDevice->state()) {
        case QModbusDevice::UnconnectedState:
            state_str = "Отключено";
        break;
        case QModbusDevice::ConnectingState:
            state_str = "Подключение";
        break;
        case QModbusDevice::ConnectedState:
            state_str = "Подключено";
        break;
        case QModbusDevice::ClosingState:
            state_str = "Отключение";
        break;
    }

    this->ui->le_state->setText(state_str);
    //qCDebug(logDebug()) << modbusDevice->state();

    if (state == QModbusDevice::UnconnectedState)
        qCDebug(logDebug) << tr("Disconnect: ") << modbusDevice->state();
    else if (state == QModbusDevice::ConnectedState) {
        if (_connection_type == ModbusSerial) {
            qCDebug(logDebug) << tr("Connected: ") + this->ui->le_com_port->text() << modbusDevice->state();
        } else if (_connection_type == ModbusTCP) {
            qCDebug(logDebug) << tr("Connected: ") + this->ui->le_ip->text() << modbusDevice->state();
        }
    }
}

void ModbusServer::onModbusErrorOccurred(QModbusDevice::Error error) {
    qCDebug(logDebug) << error;
}

void ModbusServer::settings_change() {
    if (this->modbusDevice) {
        this->modbusDevice->disconnectDevice();
        this->on_connect();
    }
}

void ModbusServer::setRegister(QModbusDataUnit::RegisterType table, const QString &reg_name, double value) {
    if (!modbusDevice)
        return;

    if (_mdum.count(reg_name) > 0) {
        bool ok = true;

        int address = _mdum.at(reg_name).startAddress(); // + 1
        int size = _mdum.at(reg_name).valueCount();

        qCDebug(logDebug) << "setRegister" << "reg_name" << reg_name << "registerType" << table  << "value" << value << "startAddress" << address;

        switch (table) {
            case QModbusDataUnit::Coils:
            {
                modbusDevice->setData(table, address, value);                
            }
            break;
            case QModbusDataUnit::DiscreteInputs:
            {
                modbusDevice->setData(table, address, value);
            }
            break;
            case QModbusDataUnit::InputRegisters:
            case QModbusDataUnit::HoldingRegisters:
            {
                auto reg_type_m = this->get_reg_type_m();
                QString reg_type = "double";
                if (reg_type_m.count(reg_name) > 0) {
                    reg_type = reg_type_m[reg_name];
                }

                if ((size == 4) && (reg_type.toLower().contains("double"))) {
                    QVector<quint16> values(4, 0);
                    double_to_usa(invert_double_order(value), &values[0]);
                    _mdum[reg_name].setValues(values);
                } else if ((size == 2) && (reg_type.toLower().contains("float"))) {
                    QVector<quint16> values(2, 0);
                    float_to_usa(invert_float_order(value), &values[0]);
                    _mdum[reg_name].setValues(values);
                } else if ((size == 4) && (reg_type.toLower().contains("ulong"))) {
                    QVector<quint16> values(4, 0);
                    ulong_to_usa(invert_ulong_order(value), &values[0]);
                    _mdum[reg_name].setValues(values);
                } else if ((size == 4) && (reg_type.toLower().contains("long"))) {
                    QVector<quint16> values(4, 0);
                    long_to_usa(invert_long_order(value), &values[0]);
                    _mdum[reg_name].setValues(values);
                } else if ((size == 2) && (reg_type.toLower().contains("uint"))) {
                    QVector<quint16> values(2, 0);
                    uint_to_usa(invert_uint_order(value), &values[0]);
                    _mdum[reg_name].setValues(values);
                } else if ((size == 2) && (reg_type.toLower().contains("int"))) {
                    QVector<quint16> values(2, 0);
                    int_to_usa(invert_int_order(value), &values[0]);
                    _mdum[reg_name].setValues(values);
                } else if (size == 1) {
                    QVector<quint16> values(1, value);
                    _mdum[reg_name].setValues(values);
                } else {

                }

                modbusDevice->setData(_mdum[reg_name]);
            }
            break;
            default:

            break;
        }
//        if (!ok)
//            statusBar()->showMessage(tr("Could not set register: ") + modbusDevice->errorString());
    }

    // qCDebug(logDebug()) << _mdum.size();
    // for (const auto &value : _mdum) {
    //     qCDebug(logDebug()) << value.first << value.second.startAddress() << value.second.valueCount() << value.second.values();
    // }
}

void ModbusServer::setRegisterString(QModbusDataUnit::RegisterType table, const QString &reg_name, const QString &value) {
    if (!modbusDevice)
        return;

    if (_mdum.count(reg_name) > 0) {
        int address = _mdum.at(reg_name).startAddress();
        int size = _mdum.at(reg_name).valueCount();
        int maxBytes = size * 2;

        qCDebug(logDebug) << "setRegisterString" << "reg_name" << reg_name << "registerType" << table << "value" << value << "startAddress" << address;

        switch (table) {
        case QModbusDataUnit::InputRegisters:
        case QModbusDataUnit::HoldingRegisters:
        {
            // Преобразуем строку в массив quint16 (каждый регистр = 2 байта)
            QByteArray byteArray = value.toUtf8();

            // Обрезаем строку, если она слишком длинная для доступного места
            if (byteArray.size() > maxBytes - 1) { // -1 для нулевого терминатора
                byteArray.resize(maxBytes - 1);
                qCDebug(logWarning) << "String truncated to fit in registers:" << reg_name;
            }

            byteArray.append('\0');

            int registerCount = (byteArray.size() + 1) / 2; // +1 для округления вверх

            QVector<quint16> values(registerCount, 0);

            for (int i = 0; i < byteArray.size(); i++) {
                int registerIndex = i / 2;
                if (i % 2 == 0) {
                    // Старший байт
                    values[registerIndex] = (byteArray[i] << 8) & 0xFF00;
                } else {
                    // Младший байт
                    values[registerIndex] |= (byteArray[i] & 0x00FF);
                }
            }

            // Заполняем оставшиеся регистры нулями (если нужно)
            while (values.size() < size) {
                values.append(0);
            }

            _mdum[reg_name].setValues(values);
            modbusDevice->setData(_mdum[reg_name]);
            //qCDebug(logDebug) << "setRegisterString" << reg_name << value << "as" << values;
        }
        break;
        default:
            qCDebug(logWarning) << "String write not supported for register type:" << table;
            break;
        }
    }
}

void ModbusServer::updateRegisters(QModbusDataUnit::RegisterType table, int address, int size) {
    qCDebug(logDebug()) << "updateRegisters" << "registerType" << table << "startAddress" << address << "valueCount" << size;
    QModbusDataUnit mdu(table, address, size);

    auto mdum = this->update_mdu(table, address, size, this->_mdum);

//    qCDebug(logDebug()) << "mdum" <<  mdum.size();
//    for (const auto &value : mdum) {
//        qCDebug(logDebug()) << value.first << value.second.startAddress() << value.second.valueCount() << value.second.values();
//    }

    for (auto &mdu : mdum) {
        quint16 value;
        switch (table) {
            case QModbusDataUnit::Coils:
            {
                QVector<quint16> values;
                for (unsigned int i = 0; i < mdu.second.valueCount(); ++i) {
                    modbusDevice->data(mdu.second.registerType(), quint16(mdu.second.startAddress() + i), &value);
                    values.push_back(value);
                }
                mdu.second.setValues(values);
            }
            break;
            case QModbusDataUnit::HoldingRegisters:
            {
                QVector<quint16> values;
                for (unsigned int i = 0; i < mdu.second.valueCount(); ++i) {
                    modbusDevice->data(mdu.second.registerType(), quint16(mdu.second.startAddress() + i), &value);
                    values.push_back(value);
                }
                mdu.second.setValues(values);
            }
            break;
        default:
            break;
        }

//        qCDebug(logDebug()) << "updateRegisters" << "reg_name" << mdu.first << "registerType" << mdu.second.registerType() << "startAddress" << mdu.second.startAddress() <<
//                               "valueCount" << mdu.second.valueCount() << "values" << mdu.second.values();
        emit read_ready(mdu.first, mdu.second);
    }
}

void ModbusServer::read_config(){
    //qCDebug(logDebug()) << "cfg.MODBUS_IP" << cfg.MODBUS_TYPE << cfg.MODBUS_IP << cfg.MODBUS_PORT;
    this->ui->cb_connect_type->setCurrentText(cfg.MODBUS_TYPE);
    this->ui->le_address->setText(QString::number(cfg.MODBUS_ADDRESS));
    this->ui->le_ip->setText(cfg.MODBUS_IP);
    this->ui->le_port->setText(QString::number(cfg.MODBUS_PORT));
    this->ui->le_com_port->setText(cfg.MODBUS_SERIAL_PORT);
    this->ui->cb_baudrate->setCurrentText(ModbusBaudRate_to_text(cfg.MODBUS_BAUDRATE));
    this->ui->cb_data_bits->setCurrentText(ModbusDataBits_to_text(cfg.MODBUS_DATA_BITS));
    this->ui->cb_stop_bits->setCurrentText(ModbusStopBits_to_text(cfg.MODBUS_STOP_BITS));
    this->ui->cb_parity->setCurrentText(cfg.MODBUS_PARITY);

    this->on_connect_type(this->ui->cb_connect_type->currentIndex());
}

void ModbusServer::write_config(){
    cfg.MODBUS_TYPE = this->ui->cb_connect_type->currentText();
    cfg.MODBUS_ADDRESS = this->ui->le_address->text().toInt();
    cfg.MODBUS_IP = this->ui->le_ip->text();
    cfg.MODBUS_PORT = this->ui->le_port->text().toInt();
    cfg.MODBUS_SERIAL_PORT = this->ui->le_com_port->text();
    cfg.MODBUS_BAUDRATE = ModbusBaudRate_from_text(this->ui->cb_baudrate->currentText().toStdString().c_str());
    cfg.MODBUS_DATA_BITS = ModbusDataBits_from_text(this->ui->cb_data_bits->currentText().toStdString().c_str());
    cfg.MODBUS_STOP_BITS = ModbusStopBits_from_text(this->ui->cb_stop_bits->currentText().toStdString().c_str());
    cfg.MODBUS_PARITY = this->ui->cb_parity->currentText();

    QSettings config("config.ini", QSettings::IniFormat);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    config.setIniCodec("UTF-8"); //Windows-1251
#endif

    config.beginGroup("parameters");
    config.setValue("MODBUS_TYPE", cfg.MODBUS_TYPE);
    config.setValue("MODBUS_ADDRESS", cfg.MODBUS_ADDRESS);
    config.setValue("MODBUS_IP", cfg.MODBUS_IP);
    config.setValue("MODBUS_PORT", cfg.MODBUS_PORT);
    config.setValue("MODBUS_SERIAL_PORT", cfg.MODBUS_SERIAL_PORT);
    config.setValue("MODBUS_BAUDRATE", cfg.MODBUS_BAUDRATE);
    config.setValue("MODBUS_DATA_BITS", cfg.MODBUS_DATA_BITS);
    config.setValue("MODBUS_STOP_BITS", cfg.MODBUS_STOP_BITS);
    config.setValue("MODBUS_PARITY", cfg.MODBUS_PARITY);
    config.endGroup();
}


