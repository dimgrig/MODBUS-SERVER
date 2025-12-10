#include "ModbusClientSettings.h"
#include "ui_modbus_client.h"

#include <QModbusDevice>

#include "ModbusBase.h"

ModbusClientSettings::ModbusClientSettings(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::fmodbus_client)
{
    ui->setupUi(this);

    this->initializeModbusComboboxes();

    connect(this->ui->cb_connect_type, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ModbusClientSettings::settings_changed);
    connect(this->ui->le_address, &QLineEdit::editingFinished, this, &ModbusClientSettings::settings_changed);
    connect(this->ui->le_ip, &QLineEdit::editingFinished, this, &ModbusClientSettings::settings_changed);
    connect(this->ui->le_port, &QLineEdit::editingFinished, this, &ModbusClientSettings::settings_changed);
    connect(this->ui->le_com_port, &QLineEdit::editingFinished, this, &ModbusClientSettings::settings_changed);
    connect(this->ui->cb_baudrate, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ModbusClientSettings::settings_changed);
    connect(this->ui->cb_data_bits, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ModbusClientSettings::settings_changed);
    connect(this->ui->cb_stop_bits, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ModbusClientSettings::settings_changed);
    connect(this->ui->cb_parity, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ModbusClientSettings::settings_changed);
    connect(this->ui->sb_poll_timer, QOverload<int>::of(&QSpinBox::valueChanged), [this](int value){
        emit poll_timer_changed(value);
    });
}

ModbusClientSettings::~ModbusClientSettings() {
    delete ui;
}

int ModbusClientSettings::get_poll_timer() const {
    return this->ui->sb_poll_timer->value();
}

void ModbusClientSettings::onModbusStateChanged(int state) {
    QString state_str;
    switch (state) {
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
}


void ModbusClientSettings::initializeModbusComboboxes() {
    this->ui->cb_connect_type->addItems(ModbusConnection_sl);
    this->ui->cb_baudrate->addItems(ModbusBaudRate_sl);
    this->ui->cb_data_bits->addItems(ModbusDataBits_sl);
    this->ui->cb_stop_bits->addItems(ModbusStopBits_sl);
    this->ui->cb_parity->addItems(ModbusParity_sl);
}

ModbusSettings ModbusClientSettings::getSettingsFromUi() {
    ModbusSettings settings;

    settings.MODBUS_POLL_TIMER = ui->sb_poll_timer->value();
    settings.MODBUS_ADDRESS = ui->le_address->text().toInt();

    settings.MODBUS_TYPE = ModbusConnection_to_text(ui->cb_connect_type->currentIndex());

    settings.MODBUS_IP = ui->le_ip->text();
    settings.MODBUS_PORT = ui->le_port->text().toInt();

    settings.MODBUS_SERIAL_PORT = ui->le_com_port->text();
    settings.MODBUS_BAUDRATE =  ModbusBaudRate_from_text(this->ui->cb_baudrate->currentText().toStdString().c_str());
    settings.MODBUS_DATA_BITS = ModbusDataBits_from_text(this->ui->cb_data_bits->currentText().toStdString().c_str());
    settings.MODBUS_STOP_BITS = ModbusStopBits_from_text(this->ui->cb_stop_bits->currentText().toStdString().c_str());
    settings.MODBUS_PARITY = ModbusParity_to_text(this->ui->cb_parity->currentIndex());

    return settings;
}

void ModbusClientSettings::setSettingsToUi(const ModbusSettings& settings) {
    ui->sb_poll_timer->setValue(settings.MODBUS_POLL_TIMER);
    ui->le_address->setText(QString::number(settings.MODBUS_ADDRESS));

    ui->cb_connect_type->setCurrentIndex(static_cast<int>(ModbusConnection_from_text(settings.MODBUS_TYPE.toStdString().c_str())));

    ui->le_ip->setText(settings.MODBUS_IP);
    ui->le_port->setText(QString::number(settings.MODBUS_PORT));

    ui->le_com_port->setText(settings.MODBUS_SERIAL_PORT);
    ui->cb_baudrate->setCurrentText(ModbusBaudRate_to_text(static_cast<ModbusBaudRate>(settings.MODBUS_BAUDRATE)));
    ui->cb_data_bits->setCurrentText(ModbusDataBits_to_text(static_cast<ModbusDataBits>(settings.MODBUS_DATA_BITS)));
    ui->cb_stop_bits->setCurrentText(ModbusStopBits_to_text(static_cast<ModbusStopBits>(settings.MODBUS_STOP_BITS)));
    ui->cb_parity->setCurrentText(ModbusParity_to_text(ModbusParity_from_text(settings.MODBUS_PARITY.toStdString().c_str())));
}

void ModbusClientSettings::read_config(){
    this->setSettingsToUi(cfg.MODBUS_SETTINGS);
}

void ModbusClientSettings::write_config(){
    cfg.MODBUS_SETTINGS = this->getSettingsFromUi();

    QSettings config("config.ini", QSettings::IniFormat);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    config.setIniCodec("UTF-8"); //Windows-1251
#endif

    config.beginGroup("modbus");
    config.setValue("MODBUS_POLL_TIMER", cfg.MODBUS_SETTINGS.MODBUS_POLL_TIMER);
    config.setValue("MODBUS_ADDRESS", cfg.MODBUS_SETTINGS.MODBUS_ADDRESS);
    config.setValue("MODBUS_TYPE", cfg.MODBUS_SETTINGS.MODBUS_TYPE);
    config.setValue("MODBUS_IP", cfg.MODBUS_SETTINGS.MODBUS_IP);
    config.setValue("MODBUS_PORT", cfg.MODBUS_SETTINGS.MODBUS_PORT);
    config.setValue("MODBUS_SERIAL_PORT", cfg.MODBUS_SETTINGS.MODBUS_SERIAL_PORT);
    config.setValue("MODBUS_BAUDRATE", cfg.MODBUS_SETTINGS.MODBUS_BAUDRATE);
    config.setValue("MODBUS_DATA_BITS", cfg.MODBUS_SETTINGS.MODBUS_DATA_BITS);
    config.setValue("MODBUS_STOP_BITS", cfg.MODBUS_SETTINGS.MODBUS_STOP_BITS);
    config.setValue("MODBUS_PARITY", cfg.MODBUS_SETTINGS.MODBUS_PARITY);
    config.endGroup();
}
