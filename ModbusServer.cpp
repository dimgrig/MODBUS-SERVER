#include "ModbusServer.h"
//#include "ui_modbus_server.h"

#include <QModbusRtuSerialSlave>
#include <QModbusTcpServer>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QStatusBar>
#include <QUrl>

#include "Config.h"

#include "functions.h"

//sudo setcap 'cap_net_bind_service=+ep' HIMIK-SERVER-TEST

ModbusServer::ModbusServer(QWidget *parent, QString config, ModbusSettings modbus_settings)
    : QWidget(parent)
    , _modbus_settings(modbus_settings)
//    , ui(new Ui::fmodbus_server)
{
//    ui->setupUi(this);
    this->on_read_json(config);

    // this->ui->cb_connect_type->addItems(ModbusConnection_sl);
    // this->ui->cb_baudrate->addItems(ModbusBaudRate_sl);
    // this->ui->cb_data_bits->addItems(ModbusDataBits_sl);
    // this->ui->cb_stop_bits->addItems(ModbusStopBits_sl);
    // this->ui->cb_parity->addItems(ModbusParity_sl);

    // connect(this->ui->cb_connect_type, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ModbusServer::on_connect_type);
    // connect(this->ui->le_address, &QLineEdit::editingFinished, this, &ModbusServer::settings_change);
    // connect(this->ui->le_ip, &QLineEdit::editingFinished, this, &ModbusServer::settings_change);
    // connect(this->ui->le_port, &QLineEdit::editingFinished, this, &ModbusServer::settings_change);
    // connect(this->ui->le_com_port, &QLineEdit::editingFinished, this, &ModbusServer::settings_change);
    // connect(this->ui->cb_baudrate, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ModbusServer::on_connect_type);
    // connect(this->ui->cb_data_bits, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ModbusServer::on_connect_type);
    // connect(this->ui->cb_stop_bits, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ModbusServer::on_connect_type);
    // connect(this->ui->cb_parity, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ModbusServer::on_connect_type);

    this->on_connect_type(static_cast<int>(ModbusConnection_from_text(modbus_settings.MODBUS_TYPE.toStdString().c_str())));

    // _reconnectTimer = new QTimer(this);
    // _reconnectTimer->setSingleShot(true);
    // connect(_reconnectTimer, &QTimer::timeout, this, &ModbusServer::on_connect);

    _portCheckTimer = new QTimer(this);
    connect(_portCheckTimer, &QTimer::timeout, this, &ModbusServer::checkSerialPorts);

    if (_connection_type == ModbusSerial) {
        _portCheckTimer->start(1000); // Проверка каждую секунду
    }
}

ModbusServer::~ModbusServer() {
    if (modbusDevice)
        modbusDevice->disconnectDevice();
    delete modbusDevice;
}

void ModbusServer::set_modbus_settings(ModbusSettings modbus_settings) {
    _modbus_settings = modbus_settings;

    this->on_connect_type(static_cast<int>(ModbusConnection_from_text(modbus_settings.MODBUS_TYPE.toStdString().c_str())));

    // if (_connection_type == ModbusSerial) {
    //     _portCheckTimer->start(1000); // Проверка каждую секунду
    // }
};

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

    // connect(modbusDevice, &QModbusDevice::errorOccurred, [this](QModbusDevice::Error) {
    //     qCDebug(logDebug()) << modbusDevice->errorString();
    //     QTimer::singleShot(1000, [=]() {
    //         this->on_connect();

    //         _request = false;
    //     });
    // });

    if (!modbusDevice) {
        if (_connection_type == ModbusSerial) {
            qCDebug(logDebug) << (tr("Could not create Modbus master."));
        } else if (_connection_type == ModbusTCP) {
            qCDebug(logDebug) << (tr("Could not create Modbus client."));
        }
    } else {
        connect(modbusDevice, &QModbusServer::stateChanged, this, &ModbusServer::onModbusStateChanged);
        connect(modbusDevice, &QModbusServer::stateChanged, this, &ModbusServer::on_state_changed);
        connect(modbusDevice, &QModbusServer::errorOccurred, this, &ModbusServer::onModbusErrorOccurred);
        connect(modbusDevice, &QModbusServer::dataWritten, this, &ModbusServer::updateRegisters);
    }
}

void ModbusServer::on_connect() {
    if (!modbusDevice)
        return;

    if (_connection_type == ModbusSerial) {
        QString portName = _modbus_settings.MODBUS_SERIAL_PORT;

        // Проверяем наличие порта в системе
        if (!isSerialPortAvailable(portName)) {
            QString errorMsg = tr("Serial port %1 not found in system").arg(portName);

            // Генерируем сигнал об ошибке
            emit host_error(errorMsg);

            // Планируем повторную проверку через некоторое время
            // _failedAttempts++;
            // scheduleReconnect();
            return;
        }
    }

    if (modbusDevice->state() != QModbusDevice::ConnectedState) {
        if (_connection_type == ModbusSerial) {
            modbusDevice->setConnectionParameter(QModbusDevice::SerialPortNameParameter, _modbus_settings.MODBUS_SERIAL_PORT);
            modbusDevice->setConnectionParameter(QModbusDevice::SerialBaudRateParameter, _modbus_settings.MODBUS_BAUDRATE); //QSerialPort::Baud19200
            modbusDevice->setConnectionParameter(QModbusDevice::SerialDataBitsParameter, _modbus_settings.MODBUS_DATA_BITS); //QSerialPort::Data8
            modbusDevice->setConnectionParameter(QModbusDevice::SerialStopBitsParameter, _modbus_settings.MODBUS_STOP_BITS); //QSerialPort::OneStop
            modbusDevice->setConnectionParameter(QModbusDevice::SerialParityParameter, _modbus_settings.MODBUS_PARITY); //QSerialPort::EvenParity
        } else if (_connection_type == ModbusTCP) {
            modbusDevice->setConnectionParameter(QModbusDevice::NetworkAddressParameter, _modbus_settings.MODBUS_IP);
            modbusDevice->setConnectionParameter(QModbusDevice::NetworkPortParameter, _modbus_settings.MODBUS_PORT);
        }

        emit host_connecting();
        emit on_connecting();

        modbusDevice->setServerAddress(_modbus_settings.MODBUS_ADDRESS);
//        modbusDevice->setTimeout(2000);
//        modbusDevice->setNumberOfRetries(3);
        if (!modbusDevice->connectDevice()) {
            qCDebug(logDebug) << tr("Connect failed: ") + modbusDevice->errorString();
        } else {
            if (_connection_type == ModbusSerial) {
                qCDebug(logDebug) << tr("Connected: ") + _modbus_settings.MODBUS_SERIAL_PORT << modbusDevice->state();
            } else if (_connection_type == ModbusTCP) {
                qCDebug(logDebug) << tr("Connected: ") + _modbus_settings.MODBUS_IP << modbusDevice->state();
            }
            this->setupDeviceData();
            modbusDevice->setMap(this->_reg);
            for (const auto &value : this->_mdum) {
                this->modbusDevice->setData(value.second);
                //qCDebug(logDebug()) << value.first << value.second.startAddress() << value.second.valueCount() << value.second.values();
            }
        }
    } else {
        //modbusDevice->disconnectDevice();
    }
}

void ModbusServer::disconnectFromHost() {
    if (modbusDevice)
        modbusDevice->disconnectDevice();
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

    //this->ui->le_state->setText(state_str);
    //qCDebug(logDebug()) << modbusDevice->state();

    if (state == QModbusDevice::UnconnectedState) {
        emit on_disconnected();
        emit host_disconnected();
        qCDebug(logDebug) << tr("Disconnect: ") << modbusDevice->state();
    } else if (state == QModbusDevice::ConnectedState) {
        emit on_connected();
        emit host_connected();
        if (_connection_type == ModbusSerial) {
            qCDebug(logDebug) << tr("Connected: ") + _modbus_settings.MODBUS_SERIAL_PORT << modbusDevice->state();
        } else if (_connection_type == ModbusTCP) {
            qCDebug(logDebug) << tr("Connected: ") + _modbus_settings.MODBUS_IP << modbusDevice->state();
        }
    }

    // if (state == QModbusDevice::ConnectedState) {
    //     _failedAttempts = 0;
    // }
}

void ModbusServer::onModbusErrorOccurred(QModbusDevice::Error error) {
    qCDebug(logDebug) << error << modbusDevice->errorString();

    // Для критических ошибок увеличиваем счетчик
    // if (error != QModbusDevice::NoError) {
    //     _failedAttempts++;
    //     scheduleReconnect();
    // }

    emit host_error(modbusDevice->errorString());
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
                } else if ((size == 2) && (reg_type.toLower().contains("ushort"))) {
                    QVector<quint16> values(1, value);
                    _mdum[reg_name].setValues(values);
                } else if ((size == 2) && (reg_type.toLower().contains("short"))) {
                    QVector<quint16> values(1, value);
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

// void ModbusServer::read_config(){
//     //qCDebug(logDebug()) << "cfg.MODBUS_IP" << cfg.MODBUS_TYPE << cfg.MODBUS_IP << cfg.MODBUS_PORT;
//     this->ui->cb_connect_type->setCurrentText(cfg.MODBUS_TYPE);
//     this->ui->le_address->setText(QString::number(cfg.MODBUS_ADDRESS));
//     this->ui->le_ip->setText(cfg.MODBUS_IP);
//     this->ui->le_port->setText(QString::number(cfg.MODBUS_PORT));
//     this->ui->le_com_port->setText(cfg.MODBUS_SERIAL_PORT);
//     this->ui->cb_baudrate->setCurrentText(ModbusBaudRate_to_text(cfg.MODBUS_BAUDRATE));
//     this->ui->cb_data_bits->setCurrentText(ModbusDataBits_to_text(cfg.MODBUS_DATA_BITS));
//     this->ui->cb_stop_bits->setCurrentText(ModbusStopBits_to_text(cfg.MODBUS_STOP_BITS));
//     this->ui->cb_parity->setCurrentText(cfg.MODBUS_PARITY);

//     this->on_connect_type(this->ui->cb_connect_type->currentIndex());
// }

// void ModbusServer::write_config(){
//     cfg.MODBUS_TYPE = this->ui->cb_connect_type->currentText();
//     cfg.MODBUS_ADDRESS = this->ui->le_address->text().toInt();
//     cfg.MODBUS_IP = this->ui->le_ip->text();
//     cfg.MODBUS_PORT = this->ui->le_port->text().toInt();
//     cfg.MODBUS_SERIAL_PORT = this->ui->le_com_port->text();
//     cfg.MODBUS_BAUDRATE = ModbusBaudRate_from_text(this->ui->cb_baudrate->currentText().toStdString().c_str());
//     cfg.MODBUS_DATA_BITS = ModbusDataBits_from_text(this->ui->cb_data_bits->currentText().toStdString().c_str());
//     cfg.MODBUS_STOP_BITS = ModbusStopBits_from_text(this->ui->cb_stop_bits->currentText().toStdString().c_str());
//     cfg.MODBUS_PARITY = this->ui->cb_parity->currentText();

//     QSettings config("config.ini", QSettings::IniFormat);
// #if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
//     config.setIniCodec("UTF-8"); //Windows-1251
// #endif

//     config.beginGroup("parameters");
//     config.setValue("MODBUS_TYPE", cfg.MODBUS_TYPE);
//     config.setValue("MODBUS_ADDRESS", cfg.MODBUS_ADDRESS);
//     config.setValue("MODBUS_IP", cfg.MODBUS_IP);
//     config.setValue("MODBUS_PORT", cfg.MODBUS_PORT);
//     config.setValue("MODBUS_SERIAL_PORT", cfg.MODBUS_SERIAL_PORT);
//     config.setValue("MODBUS_BAUDRATE", cfg.MODBUS_BAUDRATE);
//     config.setValue("MODBUS_DATA_BITS", cfg.MODBUS_DATA_BITS);
//     config.setValue("MODBUS_STOP_BITS", cfg.MODBUS_STOP_BITS);
//     config.setValue("MODBUS_PARITY", cfg.MODBUS_PARITY);
//     config.endGroup();
// }

bool ModbusServer::isSerialPortAvailable(const QString &portName) {
    if (portName.isEmpty()) {
        return false;
    }

    // Получаем список доступных портов
    QList<QSerialPortInfo> ports = QSerialPortInfo::availablePorts();

    // Ищем нужный порт
    for (const QSerialPortInfo &port : ports) {
        if (port.portName() == portName ||
            port.systemLocation() == portName) {
            qCDebug(logDebug) << "Serial port found:" << port.portName();
            return true;
        }
    }

    // qCDebug(logWarning) << "Serial port not found:" << portName
    //                     << "Available ports:";
    // for (const QSerialPortInfo &port : ports) {
    //     qCDebug(logWarning) << "  -" << port.portName()
    //     << "(" << port.description() << ")";
    // }

    return false;
}

void ModbusServer::checkSerialPorts() {
    if (_connection_type != ModbusSerial) {
        _portCheckTimer->stop();
        return;
    }

    QString portName = _modbus_settings.MODBUS_SERIAL_PORT;
    bool isAvailable = isSerialPortAvailable(portName);

    // Если порт стал доступен и мы не подключены - пытаемся подключиться
    if (isAvailable && modbusDevice->state() != QModbusDevice::ConnectedState) {
        qCDebug(logInfo) << "Serial port" << portName << "became available, attempting to connect";
        on_connect();
    }
    // Если порт исчез и мы подключены - отключаемся
    else if (!isAvailable && modbusDevice->state() == QModbusDevice::ConnectedState) {
        qCDebug(logWarning) << "Serial port" << portName << "disappeared, disconnecting";
        modbusDevice->disconnectDevice();
        emit host_error(tr("Serial port %1 disconnected").arg(portName));
    }
}
