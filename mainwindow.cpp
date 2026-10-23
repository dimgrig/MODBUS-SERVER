#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QNetworkProxy>
#include <QScrollArea>

#include <float.h>
#include <locale.h>

#include "Config.h"
#include <core-lib/Random.h>
#include <core-modbus-lib/ModbusDataConverter.h>
#include <core-modbus-lib/ModbusScale.h>
#include <core-widgets-lib/ConnectionButton.h>

const int StatusTimeout = AQP::MSecPerSecond * 10;

//#define PROCESSEVENTS
#define SINGLESHOT

#define SINGLESHOT_TIMER    100

void create_gb(QGridLayout *gl, QGroupBox *gb) {
    QGroupBox* gbi = new QGroupBox;
    //QGridLayout *li = new QGridLayout; //gl
    //QGroupBox *gbo = new QGroupBox; //gb
    QVBoxLayout *lo = new QVBoxLayout;
    QScrollArea* sa = new QScrollArea();

    gbi->setLayout(gl);
    sa->setWidget(gbi);
    sa->setWidgetResizable(true);
    lo->addWidget(sa);
    gb->setLayout(lo);

    sa->setWidgetResizable(true);
    sa->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    sa->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    //qCDebug(logDebug()) << this->ui->dw_batch_parameters->height();

    TestRunner tr;
    RUN_TEST(tr, MacrosesTest);

    this->setFont(QFont("Noto Sans", 10)); //10
    //qCDebug(logDebug()) << this->font();

    QLocale::setDefault(QLocale::Russian);
    QLocale locale = QLocale::system(); // current locale

    // Создаем новые layout'ы
    gl1 = new QGridLayout();
    gl2 = new QGridLayout();
    gl3 = new QGridLayout();
    gl4 = new QGridLayout();

    // Создаем структуру с прокруткой
    create_gb(gl1, this->ui->gb1);
    create_gb(gl2, this->ui->gb2);
    create_gb(gl3, this->ui->gb3);
    create_gb(gl4, this->ui->gb4);

    this->setAttribute(Qt::WA_AlwaysShowToolTips);

    if (QFile(QSS).exists()) {
        QScopedPointer<QFile> styleFile(new QFile(QSS));
        styleFile.data()->open(QFile::ReadOnly);
        QString qssStr = styleFile.data()->readAll();
        styleFile.data()->close();
        this->setStyleSheet(qssStr);
    } else {
        qCDebug(logWarning()) << "style file does not exist";
    }

    //this->ui->centralwidget->hide();

    //setWindowTitle(QString("%1[*]").arg(QApplication::applicationDisplayName()));
    setWindowTitle(QString("[*]"));

    //this->about_panel->hide();

    //this->installEventFilter(this);

    this->docks();
    this->mvc();

    this->menu_bar();
    this->tool_bar();

    this->connect_signal_slot();

    this->read_config();

    this->modbus_settings_ui->setObjectName("modbusSettingsUI");
    //this->ui->modbus_settings_layout->addWidget(this->modbus_client, 3, 0, 2, 0);
    this->ui->modbus_settings_layout->addWidget(this->modbus_settings_ui, 3, 0, 2, 0);

    modbus_server->set_modbus_settings(this->modbus_settings_ui->getSettingsFromUi());
    modbus_server->connectToHost();
    modbus_settings_ui->set_config(modbus_server->get_config());

    this->modbus_ui();
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange) {
        ui->retranslateUi(this);
    }
}

void MainWindow::closeEvent(QCloseEvent *event) {
    if (okToClearData())
        event->accept();
    else
        event->ignore();

    this->_saveGeometry();

    this->write_config();

    QMainWindow::closeEvent(event);
}

bool MainWindow::eventFilter(QObject *target, QEvent *event) {
    Q_UNUSED(target);
    Q_UNUSED(event);

    return false;
}

void MainWindow::connect_signal_slot() {
    connect(this->modbus_server, &ModbusServer::read_ready, this, &MainWindow::modbus_read);

    connect(this->random_timer, &QTimer::timeout, this, &MainWindow::on_random_timer);
    this->random_timer->start(1000);


    connect(this->modbus_settings_ui, &ModbusSettingsUI::settings_changed, [this](){
        bool connected = modbus_server->isConnected();
        if (connected) {
            modbus_server->disconnectFromHost();
        }
        modbus_server->set_modbus_settings(this->modbus_settings_ui->getSettingsFromUi());
        if (connected) {
            modbus_server->connectToHost();
        }
    });

    connect(this->modbus_settings_ui, &ModbusSettingsUI::poll_timer_changed, [this](int value){
        this->timer->stop();
        this->timer->start(value);
    });

    connect(this->modbus_server, &ModbusServer::on_state_changed, this->modbus_settings_ui, &ModbusSettingsUI::onModbusStateChanged);

    this->modbus_settings_ui->set_reconfig(true);
    connect(this->modbus_settings_ui, &ModbusSettingsUI::config_changed, [this](QString path){
        bool connected = modbus_server->isConnected();
        if (connected) {
            modbus_server->disconnectFromHost();
        }

        modbus_server->set_modbus_config(path);

        if (connected) {
            modbus_server->connectToHost();
        }

        this->modbus_ui();

        QSettings s(CoreConfig::configFile(), QSettings::IniFormat);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        s.setIniCodec("UTF-8");
#endif
        s.setValue("modbus-server/LAST_CONFIG", path);
    });
    connect(this->modbus_server, &ModbusServer::config_changed, this->modbus_settings_ui, &ModbusSettingsUI::set_config);

    connect(this->modbus_server, &ModbusServer::host_connecting, this, [this]() {
        this->modbus_settings_ui->setSettingsEnabled(false);
    });
    connect(this->modbus_server, &ModbusServer::host_connected, this, [this]() {
        this->modbus_settings_ui->setSettingsEnabled(false);
    });
    connect(this->modbus_server, &ModbusServer::host_disconnected, this, [this]() {
        this->modbus_settings_ui->setSettingsEnabled(true);
    });
}

void MainWindow::read_config(){
    this->modbus_settings_ui->read_config(cfg.MODBUS_SETTINGS);

    // Загрузка последнего конфига (JSON или CSV), если он сохранён
    // и файл существует. Выполняется до первого connectToHost() в конструкторе,
    // поэтому connect() уже будет с готовыми _mdum/типами.
    QSettings s(CoreConfig::configFile(), QSettings::IniFormat);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    s.setIniCodec("UTF-8");
#endif
    const QString last = s.value("modbus-server/LAST_CONFIG", QString()).toString();
    if (!last.isEmpty() && QFile::exists(last)) {
        this->modbus_settings_ui->set_config(last);
        this->modbus_server->set_modbus_config(last);
    }
}

void MainWindow::write_config(){
    QSettings config(CoreConfig::configFile(), QSettings::IniFormat);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    config.setIniCodec("UTF-8"); //Windows-1251
#endif

    config.beginGroup("parameters");
    config.endGroup();

    cfg.MODBUS_SETTINGS = this->modbus_settings_ui->write_config();
}

void MainWindow::menu_bar() {

}

void MainWindow::tool_bar() {
    if (!this->ui->toolbar) {
        return;
    }

    // Общие иконки (connected/connecting/disconnected) — из Core-ресурсов.
    auto* connBtn = new ConnectionButton(this->ui->toolbar);
    connBtn->setObjectName("connBtn");
    connBtn->setToolTip(tr("Подключение"));
    this->ui->toolbar->addWidget(connBtn);

    // Состояния ModbusServer → визуализация кнопки
    connect(this->modbus_server, &ModbusServer::host_connected,
            connBtn, &ConnectionButton::onConnected);
    connect(this->modbus_server, &ModbusServer::host_disconnected,
            connBtn, &ConnectionButton::onDisconnected);
    connect(this->modbus_server, &ModbusServer::host_connecting,
            connBtn, &ConnectionButton::onConnecting);

    // Нажатия кнопки → действия
    connect(connBtn, &ConnectionButton::connectRequested,
            this->modbus_server, &ModbusServer::connectToHost);
    connect(connBtn, &ConnectionButton::disconnectRequested,
            this->modbus_server, &ModbusServer::disconnectFromHost);

    QTimer::singleShot(1000, connBtn, [this, connBtn]() {
        int s = 0;
        switch (this->modbus_server->state()) {
        case QModbusDevice::ConnectedState:  s = 2; break;
        case QModbusDevice::ConnectingState: s = 1; break;
        default:                             s = 0; break;
        }
        connBtn->forceState(s);
    });
}

void MainWindow::auth_users() {

}

void MainWindow::init_auth_users() {

}

void MainWindow::mvc() {

}

void MainWindow::docks() {

}

void MainWindow::_saveGeometry() {

}

void MainWindow::test() {

}

bool MainWindow::okToClearData()
{
    return true;
}

void MainWindow::auth_ok() {

}

void MainWindow::on_timer() {

}

void MainWindow::on_random_timer() {
    if (!this->ui->cb_random->isChecked()) return;

    const double sko = this->ui->dsb_sko->value();

    // Применяем рандомизацию к HoldingRegisters и InputRegisters.
    // На сервере IR — внутренние регистры, доступные только на чтение
    // со стороны клиента; сервер вправе менять их локально.
    auto apply_random = [sko](std::vector<QDoubleSpinBox*>& spins) {
        for (auto* dsb : spins) {
            if (!dsb) continue;
            const double delta = sko * dsb->value() / 100.0;
            if (delta == 0.0) continue;
            dsb->setValue(dsb->value() + Random::get(-delta, delta));
        }
    };

    apply_random(hr_v);
    apply_random(ir_v);
}

void MainWindow::modbus_read(int serverAddress, QString regName, QModbusDataUnit mdu) {
    qCDebug(logDebug()) << "modbus_read" << regName << mdu.startAddress() << mdu.valueCount() << mdu.values();

    auto reg_data_type_m = modbus_server->get_reg_data_type_m();
    QString dataType = "double";
    if (reg_data_type_m.count(serverAddress) > 0) {
        if (reg_data_type_m.at(serverAddress).count(regName) > 0) {
            dataType = reg_data_type_m.at(serverAddress).at(regName);
        }
    }

    auto reg_scale_m = modbus_server->get_reg_scale_m();
    auto scaleFor = [&](const QString& regName) -> ModbusScale {
        if (reg_scale_m.count(serverAddress) > 0 &&
            reg_scale_m.at(serverAddress).count(regName) > 0) {
            return reg_scale_m.at(serverAddress).at(regName);
        }
        return ModbusScale();
    };

    if (dataType.toLower().contains("string")) {
        // Обработка строкового регистра
        auto le = this->ui->gb4->findChild<QLineEdit*>("le_" + regName);
        if (le) {
            disconnect(le, &QLineEdit::textChanged, this, &MainWindow::StringHoldingRegistersChanged);
            //le->setText(str);
            QVariant value = ModbusDataConverter::registersToVariant(mdu.values(), regName, dataType, this->modbus_server->modbus_format(), scaleFor(regName));
            le->setText(value.toString());
            connect(le, &QLineEdit::textChanged, this, &MainWindow::StringHoldingRegistersChanged);
        }
    } else {
        switch (mdu.registerType()) {
            case QModbusDataUnit::Coils:
            {
                auto cb = this->ui->gb2->findChild<QCheckBox*>("cb_" + regName);
                if (cb) {
                    disconnect(cb, &QCheckBox::clicked, this, &MainWindow::CoilsChanged);
                    cb->setChecked(mdu.value(0));
                    connect(cb, &QCheckBox::clicked, this, &MainWindow::CoilsChanged);
                }
            }
            break;
            case QModbusDataUnit::HoldingRegisters:
            {
                auto dsb = this->ui->gb4->findChild<QDoubleSpinBox*>("dsb_" + regName);
                if (dsb) {

#if QT_VERSION > QT_VERSION_CHECK(6, 0, 0)
                disconnect(dsb, &QDoubleSpinBox::valueChanged, this, &MainWindow::HoldingRegistersChanged);
#else
    #if QT_VERSION > QT_VERSION_CHECK(5, 15, 8)
                    disconnect(dsb, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &MainWindow::HoldingRegistersChanged);
    #else
                    disconnect(dsb, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &MainWindow::HoldingRegistersChanged);
    #endif
#endif
                    QVariant value = ModbusDataConverter::registersToVariant(mdu.values(), regName, dataType, this->modbus_server->modbus_format(), scaleFor(regName));
                    dsb->setValue(value.toDouble());
#if QT_VERSION > QT_VERSION_CHECK(6, 0, 0)
                    connect(dsb, &QDoubleSpinBox::valueChanged, this, &MainWindow::HoldingRegistersChanged);
#else
    #if QT_VERSION > QT_VERSION_CHECK(5, 15, 8)
                    connect(dsb, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &MainWindow::HoldingRegistersChanged);
    #else
                    connect(dsb, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &MainWindow::HoldingRegistersChanged);
    #endif
#endif
                }
            }
            break;
            default:

            break;
        }
    }
}

void MainWindow::DiscreteInputsChanged(bool value) {
    auto sender = QObject::sender();
    if (sender) {
        QString reg_name = sender->objectName();
        reg_name = reg_name.remove("cb_");
        //qCDebug(logDebug()) << reg_name << value;
        this->modbus_server->setRegister(this->modbus_settings_ui->getSettingsFromUi().MODBUS_ADDRESS, reg_name, value);
    }
}

void MainWindow::CoilsChanged(bool value) {
    auto sender = QObject::sender();
    if (sender) {
        QString reg_name = sender->objectName();
        reg_name = reg_name.remove("cb_");
        //qCDebug(logDebug()) << reg_name << value;
        this->modbus_server->setRegister(this->modbus_settings_ui->getSettingsFromUi().MODBUS_ADDRESS, reg_name, value);
    }
}

void MainWindow::InputRegistersChanged(double value) {
    auto sender = QObject::sender();
    if (sender) {
        QString reg_name = sender->objectName();
        reg_name = reg_name.remove("dsb_");
        //qCDebug(logDebug()) << reg_name << value;
        this->modbus_server->setRegister(this->modbus_settings_ui->getSettingsFromUi().MODBUS_ADDRESS, reg_name, value);
    }
}

void MainWindow::HoldingRegistersChanged(double value) {
    auto sender = QObject::sender();
    if (sender) {
        QString reg_name = sender->objectName();
        reg_name = reg_name.remove("dsb_");
        //qCDebug(logDebug()) << reg_name << value;
        this->modbus_server->setRegister(this->modbus_settings_ui->getSettingsFromUi().MODBUS_ADDRESS, reg_name, value);
    }
}

void MainWindow::StringInputRegistersChanged(const QString &value) {
    auto sender = QObject::sender();
    if (sender) {
        QString reg_name = sender->objectName();
        reg_name = reg_name.remove("le_");
        //qCDebug(logDebug()) << reg_name << value;
        this->modbus_server->setRegisterString(this->modbus_settings_ui->getSettingsFromUi().MODBUS_ADDRESS, reg_name, value);
    }
}

void MainWindow::StringHoldingRegistersChanged(const QString &value) {
    auto sender = QObject::sender();
    if (sender) {
        QString reg_name = sender->objectName();
        reg_name = reg_name.remove("le_");
        //qCDebug(logDebug()) << reg_name << value;
        this->modbus_server->setRegisterString(this->modbus_settings_ui->getSettingsFromUi().MODBUS_ADDRESS, reg_name, value);
    }
}

void create_dsb(QDoubleSpinBox* dsb, const QString &name, const QString &reg_type, const QModbusDataUnit &mdu, const ModbusScale &scale = ModbusScale()) {
    dsb->setObjectName(QString("dsb_") + name);
    dsb->setMinimumSize({50, 20});

    ModbusScale eff = scale;
    if (eff.isNone()) {
        if (name.contains("* 0.01"))      eff = ModbusScale::makeFactor(0.01);
        else if (name.contains("* 0.1"))  eff = ModbusScale::makeFactor(0.1);
    }

    if (reg_type.toLower().contains("float") || reg_type.toLower().contains("double")) {
        //float or double
        dsb->setDecimals(2);
        dsb->setSingleStep(0.01);
        dsb->setMaximum(DBL_MAX);
        dsb->setMinimum(-DBL_MAX);
    } else if (reg_type.toLower().contains("ushort")) {
        //ushort
        dsb->setDecimals(0);
        dsb->setSingleStep(1);
        dsb->setMaximum((double)USHRT_MAX);
        dsb->setMinimum(0);
    } else if (reg_type.toLower().contains("short")) {
        //short
        dsb->setDecimals(0);
        dsb->setSingleStep(1);
        dsb->setMaximum((double)SHRT_MAX);
        dsb->setMinimum((double)-SHRT_MAX);
    } else if (reg_type.toLower().contains("uint")) {
        //uint
        dsb->setDecimals(0);
        dsb->setSingleStep(1);
        dsb->setMaximum((double)UINT_MAX);
        dsb->setMinimum(0);
    } else if (reg_type.toLower().contains("int")) {
        //int
        dsb->setDecimals(0);
        dsb->setSingleStep(1);
        dsb->setMaximum((double)INT_MAX);
        dsb->setMinimum((double)-INT_MAX);
    } else if (reg_type.toLower().contains("ulong")) {
        //ulong
        dsb->setDecimals(0);
        dsb->setSingleStep(1);
        dsb->setMaximum((double)ULONG_MAX);
        dsb->setMinimum(0);
    } else if (reg_type.toLower().contains("long")) {
        //long
        dsb->setDecimals(0);
        dsb->setSingleStep(1);
        dsb->setMaximum((double)LONG_MAX);
        dsb->setMinimum((double)-LONG_MAX);
    } else {
        //TODO
    }

    if (eff.isFactor()) {
        const double f = eff.factor;
        dsb->setMaximum(dsb->maximum() * f);
        dsb->setMinimum(dsb->minimum() * f);
        dsb->setSingleStep(f);
        int dec = 0;
        if (f >= 0.1) dec = 1;
        else if (f >= 0.01) dec = 2;
        else if (f >= 0.001) dec = 3;
        else dec = 4;
        if (dec > dsb->decimals())
            dsb->setDecimals(dec);
    } else if (eff.isRange()) {
        dsb->setMinimum(eff.dstMin);
        dsb->setMaximum(eff.dstMax);
        const double span = eff.dstMax - eff.dstMin;
        dsb->setSingleStep(span > 0.0 ? span / 100.0 : 0.01);
        int dec = 0;
        if (span >= 100.0) dec = 0;
        else if (span >= 10.0) dec = 1;
        else if (span >= 1.0)  dec = 2;
        else                   dec = 3;
        dsb->setDecimals(dec);
    }
}

void MainWindow::modbus_ui() {
    // Очищаем старый UI
    clear_modbus_ui();

    this->_mdum = modbus_server->get_mdum();
    auto mdum_m = modbus_server->get_sort_by_start_address_mdum_v();
    auto reg_data_type_m = modbus_server->get_reg_data_type_m();
    auto reg_scale_m = modbus_server->get_reg_scale_m();

    for (const auto &[serverAddress, mdum_v] : mdum_m) {
        for (const auto &_mdu : mdum_v) {
            QString regName = _mdu.first;
            auto mdu = _mdu.second;

            QString reg_data_type = "double";
            if (reg_data_type_m.count(serverAddress) > 0) {
                if (reg_data_type_m.at(serverAddress).count(regName) > 0) {
                    reg_data_type = reg_data_type_m.at(serverAddress).at(regName);
                }
            }
            //qCDebug(logDebug) << name << mdu.startAddress() << mdu.valueCount() << mdu.values();
            switch (mdu.registerType()) {
                case QModbusDataUnit::DiscreteInputs:
                {
                    auto label = new QLabel(regName, this);
                    auto cb = new QCheckBox(this);
                    cb->setObjectName(QString("cb_") + regName);

                    gl1->addWidget(label, di_v.size(), 0);
                    gl1->addWidget(cb, di_v.size(), 1);

                    di_v.push_back(cb);
                    connect(cb, &QCheckBox::clicked, this, &MainWindow::DiscreteInputsChanged);
                }
                break;
                case QModbusDataUnit::Coils:
                {
                    auto label = new QLabel(regName, this);
                    auto cb = new QCheckBox(this);
                    cb->setObjectName(QString("cb_") + regName);

                    gl2->addWidget(label, coil_v.size(), 0);
                    gl2->addWidget(cb, coil_v.size(), 1);

                    coil_v.push_back(cb);
                    connect(cb, &QCheckBox::clicked, this, &MainWindow::CoilsChanged);
                }
                break;
                case QModbusDataUnit::InputRegisters:
                {
                    if (reg_data_type.toLower().contains("string")) {
                        auto label = new QLabel(regName, this);
                        auto le = new QLineEdit(this);
                        le->setObjectName(QString("le_") + regName);

                        gl3->addWidget(label, ir_le_v.size() + ir_v.size(), 0);
                        gl3->addWidget(le, ir_le_v.size() + ir_v.size(), 1);

                        ir_le_v.push_back(le);

                        connect(le, &QLineEdit::textChanged, this, &MainWindow::StringInputRegistersChanged);
                    } else {
                        auto label = new QLabel(regName, this);
                        auto dsb = new QDoubleSpinBox(this);
                        ModbusScale regScale;
                        if (reg_scale_m.count(serverAddress) > 0 &&
                            reg_scale_m.at(serverAddress).count(regName) > 0) {
                            regScale = reg_scale_m.at(serverAddress).at(regName);
                        }
                        create_dsb(dsb, regName, reg_data_type, mdu, regScale);

                        gl3->addWidget(label, ir_le_v.size() + ir_v.size(), 0);
                        gl3->addWidget(dsb, ir_le_v.size() + ir_v.size(), 1);

                        ir_v.push_back(dsb);

    #if QT_VERSION > QT_VERSION_CHECK(6, 0, 0)
                        connect(dsb, &QDoubleSpinBox::valueChanged, this, &MainWindow::InputRegistersChanged);
    #else
        #if QT_VERSION > QT_VERSION_CHECK(5, 15, 8)
                        connect(dsb, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &MainWindow::InputRegistersChanged);
        #else
                        connect(dsb, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &MainWindow::InputRegistersChanged);
        #endif
    #endif
                    }
                }
                break;
                case QModbusDataUnit::HoldingRegisters:
                {
                    if (reg_data_type.toLower().contains("string")) {
                        auto label = new QLabel(regName, this);
                        auto le = new QLineEdit(this);
                        le->setObjectName(QString("le_") + regName);

                        gl4->addWidget(label, hr_le_v.size() + hr_v.size(), 0);
                        gl4->addWidget(le, hr_le_v.size() + hr_v.size(), 1);

                        hr_le_v.push_back(le);

                        connect(le, &QLineEdit::textChanged, this, &MainWindow::StringHoldingRegistersChanged);
                    } else {
                        auto label = new QLabel(regName, this);
                        auto dsb = new QDoubleSpinBox(this);
                        ModbusScale regScale;
                        if (reg_scale_m.count(serverAddress) > 0 &&
                            reg_scale_m.at(serverAddress).count(regName) > 0) {
                            regScale = reg_scale_m.at(serverAddress).at(regName);
                        }
                        create_dsb(dsb, regName, reg_data_type, mdu, regScale);

                        gl4->addWidget(label, hr_le_v.size() + hr_v.size(), 0);
                        gl4->addWidget(dsb, hr_le_v.size() + hr_v.size(), 1);

                        hr_v.push_back(dsb);

    #if QT_VERSION > QT_VERSION_CHECK(6, 0, 0)
                        connect(dsb, &QDoubleSpinBox::valueChanged, this, &MainWindow::HoldingRegistersChanged);
    #else
        #if QT_VERSION > QT_VERSION_CHECK(5, 15, 8)
                        connect(dsb, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &MainWindow::HoldingRegistersChanged);
        #else
                        connect(dsb, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &MainWindow::HoldingRegistersChanged);
        #endif
    #endif
                    }
                }
                break;
                default:

                break;
            }
        }

        {
            //init values
            int cnt = 1;
            for (auto &cb : di_v) {
                cb->setChecked(cnt++ % 2);
                emit cb->clicked(cb->checkState());
            }
            cnt = 1;
            for (auto &cb : coil_v) {
                cb->setChecked(cnt++ % 2);
                emit cb->clicked(cb->checkState());
            }
            double value = 1;
            for (auto &dsb : ir_v) {
                dsb->setValue(value + value / 10);
                value++;
            }
            value = 1;
            for (auto &dsb : hr_v) {
                dsb->setValue(value + value / 10);
                value++;
            }
            for (auto &le : ir_le_v) {
                QString reg_name = le->objectName().remove("le_");
                le->setText(reg_name); // Начальное значение, например, имя регистра
                emit le->textChanged(le->text());
            }
            for (auto &le : hr_le_v) {
                QString reg_name = le->objectName().remove("le_");
                le->setText(reg_name); // Начальное значение, например, имя регистра
                emit le->textChanged(le->text());
            }
        }
    }
}

void MainWindow::clear_modbus_ui() {
    for (auto &cb : di_v) {
        if (cb) {
            disconnect(cb, &QCheckBox::clicked, this, &MainWindow::DiscreteInputsChanged);
        }
    }

    for (auto &cb : coil_v) {
        if (cb) {
            disconnect(cb, &QCheckBox::clicked, this, &MainWindow::CoilsChanged);
        }
    }

    for (auto &dsb : ir_v) {
        if (dsb) {
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
            disconnect(dsb, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
                       this, &MainWindow::InputRegistersChanged);
#else
            disconnect(dsb, &QDoubleSpinBox::valueChanged,
                       this, &MainWindow::HoldingRegistersChanged);
#endif
        }
    }

    for (auto &dsb : hr_v) {
        if (dsb) {
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
            disconnect(dsb, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
                       this, &MainWindow::HoldingRegistersChanged);
#else
            disconnect(dsb, &QDoubleSpinBox::valueChanged,
                       this, &MainWindow::HoldingRegistersChanged);
#endif
        }
    }

    for (auto &le : hr_le_v) {
        if (le) {
            disconnect(le, &QLineEdit::textChanged, this, &MainWindow::StringHoldingRegistersChanged);
        }
    }

    for (auto &le : ir_le_v) {
        if (le) {
            disconnect(le, &QLineEdit::textChanged, this, &MainWindow::StringInputRegistersChanged);
        }
    }

    // 2. Удаляем layout'ы и их содержимое
    if (gl1) {
        QLayoutItem *item;
        while ((item = gl1->takeAt(0)) != nullptr) {
            if (item->widget()) {
                delete item->widget();
            }
            delete item;
        }
        //delete gl1;
        //gl1 = nullptr;
    }

    if (gl2) {
        QLayoutItem *item;
        while ((item = gl2->takeAt(0)) != nullptr) {
            if (item->widget()) {
                delete item->widget();
            }
            delete item;
        }
        //delete gl2;
        //gl2 = nullptr;
    }

    if (gl3) {
        QLayoutItem *item;
        while ((item = gl3->takeAt(0)) != nullptr) {
            if (item->widget()) {
                delete item->widget();
            }
            delete item;
        }
        //delete gl3;
        //gl3 = nullptr;
    }

    if (gl4) {
        QLayoutItem *item;
        while ((item = gl4->takeAt(0)) != nullptr) {
            if (item->widget()) {
                delete item->widget();
            }
            delete item;
        }
        //delete gl4;
        //gl4 = nullptr;
    }

    // 3. Очищаем векторы указателей
    di_v.clear();
    coil_v.clear();
    ir_v.clear();
    hr_v.clear();
    ir_le_v.clear();
    hr_le_v.clear();

    // 4. Очищаем внутреннюю структуру данных
    _mdum.clear();
}
