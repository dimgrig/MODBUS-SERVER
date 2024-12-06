#include "ModbusServer.h"
#include "ui_modbus_server.h"

#include <QModbusRtuSerialSlave>
#include <QModbusTcpServer>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
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
    this->on_read_json(this->_mdum);
    //setupWidgetContainers();

#if QT_CONFIG(modbus_serialport)
    //ui->connectType->setCurrentIndex(0);
    //onCurrentConnectTypeChanged(0);
#else
    // lock out the serial port option
    ui->connectType->setCurrentIndex(1);
    onCurrentConnectTypeChanged(1);
    ui->connectType->setEnabled(false);
#endif

    //m_settingsDialog = new SettingsDialog(this);
    //initActions();

    connect(this->ui->le_IP, &QLineEdit::editingFinished, this, &ModbusServer::settings_change);
    connect(this->ui->le_PORT, &QLineEdit::editingFinished, this, &ModbusServer::settings_change);
}

ModbusServer::~ModbusServer() {
    if (modbusDevice)
        modbusDevice->disconnectDevice();
    delete modbusDevice;
}

//void ModbusServer::initActions() {
//    ui->actionConnect->setEnabled(true);
//    ui->actionDisconnect->setEnabled(false);
//    ui->actionExit->setEnabled(true);
//    ui->actionOptions->setEnabled(true);

//    connect(ui->connectButton, &QPushButton::clicked,
//            this, &MainWindow::onConnectButtonClicked);
//    connect(ui->actionConnect, &QAction::triggered,
//            this, &MainWindow::onConnectButtonClicked);
//    connect(ui->actionDisconnect, &QAction::triggered,
//            this, &MainWindow::onConnectButtonClicked);
//    connect(ui->connectType, QOverload<int>::of(&QComboBox::currentIndexChanged),
//            this, &MainWindow::onCurrentConnectTypeChanged);

//    connect(ui->actionExit, &QAction::triggered, this, &QMainWindow::close);
//    connect(ui->actionOptions, &QAction::triggered, m_settingsDialog, &QDialog::show);
//}

void ModbusServer::on_connect_type(int index) {
    if (modbusDevice) {
        modbusDevice->disconnectDevice();
        delete modbusDevice;
        modbusDevice = nullptr;
    }

    auto type = static_cast<ModbusConnection>(index);
    if (type == Serial) {
        modbusDevice = new QModbusRtuSerialSlave(this);
    } else if (type == Tcp) {
        modbusDevice = new QModbusTcpServer(this);
    }

    connect(modbusDevice, &QModbusDevice::errorOccurred, [this](QModbusDevice::Error) {
        qCDebug(logDebug()) << modbusDevice->errorString();
        QTimer::singleShot(1000, [=]() {
            this->on_connect();

            _request = false;
        });
        //statusBar()->showMessage(modbusDevice->errorString(), 5000);
    });

    if (!modbusDevice) {
        //ui->connectButton->setDisabled(true);
        if (type == Serial)
            qCDebug(logDebug) << (tr("Could not create Modbus master."), 5000);
        else
            qCDebug(logDebug) << (tr("Could not create Modbus client."), 5000);
    } else {
        connect(modbusDevice, &QModbusServer::stateChanged,
                this, &ModbusServer::onModbusStateChanged);
        connect(modbusDevice, &QModbusServer::errorOccurred,
                this, &ModbusServer::onModbusErrorOccurred);
        connect(modbusDevice, &QModbusServer::dataWritten,
                this, &ModbusServer::updateRegisters);
    }
}

void ModbusServer::on_connect()
{
    if (!modbusDevice)
        return;

    if (modbusDevice->state() != QModbusDevice::ConnectedState) {
        if (_connection_type == Serial) {
//            modbusDevice->setConnectionParameter(QModbusDevice::SerialPortNameParameter,
//                ui->portEdit->text());
//#if QT_CONFIG(modbus_serialport)
//            modbusDevice->setConnectionParameter(QModbusDevice::SerialParityParameter,
//                m_settingsDialog->settings().parity);
//            modbusDevice->setConnectionParameter(QModbusDevice::SerialBaudRateParameter,
//                m_settingsDialog->settings().baud);
//            modbusDevice->setConnectionParameter(QModbusDevice::SerialDataBitsParameter,
//                m_settingsDialog->settings().dataBits);
//            modbusDevice->setConnectionParameter(QModbusDevice::SerialStopBitsParameter,
//                m_settingsDialog->settings().stopBits);
//#endif
        } else {
            const QUrl url = QUrl::fromUserInput(_address);
            modbusDevice->setConnectionParameter(QModbusDevice::NetworkPortParameter, url.port());
            modbusDevice->setConnectionParameter(QModbusDevice::NetworkAddressParameter, url.host());
        }
        modbusDevice->setServerAddress(1);
//        modbusDevice->setTimeout(2000);
//        modbusDevice->setNumberOfRetries(3);
        if (!modbusDevice->connectDevice()) {
            qCDebug(logDebug) << tr("Connect failed: ") + modbusDevice->errorString();
        } else {
            //Sleeper sl;
            //sl.sleep(5);
            qCDebug(logDebug) << tr("Connected: ") + _address << modbusDevice->state();
            //ui->actionConnect->setEnabled(false);
            //ui->actionDisconnect->setEnabled(true);
        }

        this->_reg = this->setupDeviceData(this->_mdum);
        modbusDevice->setMap(_reg);
        for (const auto &value : _mdum) {
            this->modbusDevice->setData(value.second);
            //qCDebug(logDebug()) << value.first << value.second.startAddress() << value.second.valueCount() << value.second.values();
        }
    } else {
        modbusDevice->disconnectDevice();
        //ui->actionConnect->setEnabled(true);
        //ui->actionDisconnect->setEnabled(false);
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
    qCDebug(logDebug()) << modbusDevice->state();

    bool connected = (state != QModbusDevice::UnconnectedState);
    //ui->actionConnect->setEnabled(!connected);
    //ui->actionDisconnect->setEnabled(connected);

    if (state == QModbusDevice::UnconnectedState)
        qCDebug(logDebug) << tr("Disconnect: ") + _address << modbusDevice->state();
        //ui->connectButton->setText(tr("Connect"));
    else if (state == QModbusDevice::ConnectedState) {
        qCDebug(logDebug) << tr("Connect: ") + _address << modbusDevice->state();
        //ui->connectButton->setText(tr("Disconnect"));
    }
}

void ModbusServer::onModbusErrorOccurred(QModbusDevice::Error error) {
    qCDebug(logDebug) << error;
}

void ModbusServer::settings_change() {
    this->_address = this->ui->le_IP->text() + ":" + this->ui->le_PORT->text();
    if (this->modbusDevice) {
        this->modbusDevice->disconnectDevice();
    }
}

void ModbusServer::setRegister(QModbusDataUnit::RegisterType table, const QString &reg_name, double value) {
    if (!modbusDevice)
        return;

    //if (registers.contains(reg_name)) {
    if (_mdum.count(reg_name) > 0) {
        bool ok = true;

        int address = _mdum.at(reg_name).startAddress(); // + 1

        qCDebug(logDebug) << "setRegister" << table << reg_name << value << address;

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
            {
                QVector<quint16> values(4, 0);
                double_to_usa(invert_double_order(value), &values[0]);
                _mdum[reg_name].setValues(values);
                modbusDevice->setData(_mdum[reg_name]);
            }
            case QModbusDataUnit::HoldingRegisters:
            {
                QVector<quint16> values(4, 0);
                double_to_usa(invert_double_order(value), &values[0]);
                _mdum[reg_name].setValues(values);
                modbusDevice->setData(_mdum[reg_name]);
            }
            break;
            default:

            break;
        }
//        if (!ok)
//            statusBar()->showMessage(tr("Could not set register: ") + modbusDevice->errorString(),
//                                     5000);
    }

    // qCDebug(logDebug()) << _mdum.size();
    // for (const auto &value : _mdum) {
    //     qCDebug(logDebug()) << value.first << value.second.startAddress() << value.second.valueCount() << value.second.values();
    // }
}

void ModbusServer::updateRegisters(QModbusDataUnit::RegisterType table, int address, int size) {
    //qCDebug(logDebug()) << "updateRegisters" << table << address << size;
    QModbusDataUnit mdu(table, address, size);

    auto mdum = this->update_mdu(table, address, size, this->_mdum);

    for (auto &mdu : mdum) {
        quint16 value;
        switch (table) {
            case QModbusDataUnit::Coils:
            {
                QVector<quint16> values;
                for (int i = 0; i < size; ++i) {
                    modbusDevice->data(mdu.second.registerType(), quint16(address + i), &value);
                    values.push_back(value);
                }
                mdu.second.setValues(values);
            }
            break;
            case QModbusDataUnit::HoldingRegisters:
            {
                QVector<quint16> values;
                for (int i = 0; i < size; ++i) {
                    modbusDevice->data(mdu.second.registerType(), quint16(address + i), &value);
                    values.push_back(value);
                }
                mdu.second.setValues(values);
            }
            break;
        default:
            break;
        }

        qCDebug(logDebug()) << "updateRegisters" << mdu.first << mdu.second.registerType() << mdu.second.startAddress() << mdu.second.valueCount() << mdu.second.values();
        emit read_ready(mdu.first, mdu.second);
    }
}

void ModbusServer::read_config(){
    //qCDebug(logDebug()) << "cfg.MODBUS_IP" << cfg.MODBUS_IP << cfg.MODBUS_PORT;
    this->ui->le_IP->setText(cfg.MODBUS_IP);
    this->ui->le_PORT->setText(QString::number(cfg.MODBUS_PORT));
    this->settings_change();
}

void ModbusServer::write_config(){
    cfg.MODBUS_IP = this->ui->le_IP->text();
    cfg.MODBUS_PORT = this->ui->le_PORT->text().toInt();

    QSettings config("config.ini", QSettings::IniFormat);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    config.setIniCodec("UTF-8"); //Windows-1251
#endif

    config.beginGroup("parameters");
    config.setValue("MODBUS_IP", cfg.MODBUS_IP);
    config.setValue("MODBUS_PORT", cfg.MODBUS_PORT);
    config.endGroup();
}


