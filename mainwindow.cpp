#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QNetworkProxy>
#include <QScrollArea>

#include <float.h>
#include <locale.h>

#include "Config.h"
#include "ModbusDataConverter.h"
#include "Random.h"

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

//    QLocale locale = QLocale::system(); // current locale
//    QLocale russian(QLocale::Russian);
//    locale.setNumberOptions(russian.numberOptions()); // borrow number options from the Russian locale
//    QLocale::setDefault(locale); // set as default
    QLocale::setDefault(QLocale::Russian);
    QLocale::setDefault(QLocale::Russian);
    QLocale::setDefault(QLocale::Russian);
    QLocale locale = QLocale::system(); // current locale
    //if (locale.name() != "ru_RU") {
//        qCDebug(logDebug()) << std::locale().name() << locale.name() << QLocale().name();
//        AQP::warning(this, "Ошибка", "Ошибка установки локали. Выполнить sudo dpkg-reconfigure locales и выбрать локаль ru_RU");
//        exit(1);
//    }

//    qCDebug(logDebug()) << std::locale().name() << locale.name();
//    setlocale(LC_ALL, "ru_RU");
//    //std::locale::global(std::locale(std::locale("ru_RU"), "C", std::locale::numeric));
    //qCDebug(logDebug()) << std::locale().name() << locale.name();

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

    //this->ui->pb_test->setVisible(false);
    //this->ui->pb_test_oscillogram->setVisible(false);
    //this->ui->pb_quit->setVisible(false);

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

    //this->ui->modbus_settings_layout->addWidget(this->modbus_client, 3, 0, 2, 0);
    this->ui->modbus_settings_layout->addWidget(this->modbus_settings_ui, 3, 0, 2, 0);

    // if (cfg.PROXY) {
    //     QNetworkProxy proxy;
    //     proxy.setType(QNetworkProxy::DefaultProxy);
    //     proxy.setHostName(cfg.PROXY_HOST); //"192.168.7.1"
    //     proxy.setPort(cfg.PROXY_PORT); //3128
    //     //proxy.setUser("username");
    //     //proxy.setPassword("password");

    //     QNetworkProxy::setApplicationProxy(proxy);
    // }

    modbus_server->set_modbus_settings(this->modbus_settings_ui->getSettingsFromUi());
    modbus_server->connectToHost();
    modbus_settings_ui->set_config(modbus_server->get_config());

    this->modbus_ui();


//    this->ui->cb_language->addItems(QStringList() << "ru_RU" << "en_US");
//    this->ui->cb_language->setCurrentText("en_US");

//#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
//    connect(this->ui->cb_language, static_cast<void (QComboBox::*)(const QString &)>(&QComboBox::currentIndexChanged),
//           [=](const QString &str){
//       qtLanguageTranslator.load("QtLanguage_" + str, QCoreApplication::applicationDirPath());   // Загружаем перевод
//       qApp->installTranslator(&qtLanguageTranslator);        // Устанавливаем перевод в приложение
//    });
//#endif
//#if QT_VERSION > QT_VERSION_CHECK(5, 15, 8)
//    connect(this->ui->cb_language, &QComboBox::currentTextChanged,
//           [=](const QString &str){
//       qtLanguageTranslator.load("QtLanguage_" + str, QCoreApplication::applicationDirPath());   // Загружаем перевод
//       qApp->installTranslator(&qtLanguageTranslator);        // Устанавливаем перевод в приложение
//    });
//#endif

//    qtLanguageTranslator.load(QString("QtLanguage_") + QString("en_US"), QCoreApplication::applicationDirPath());
//    qApp->installTranslator(&qtLanguageTranslator);
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
//    if (event->type() == QEvent::Shortcut) {
//        // If I care about this shortcut, then return true to intercept
//        // Else, return false to let the application process it
//        if (!cfg.ENABLE_SHORTCUTS) {
//            qCDebug(logDebug()) << "SHORTCUT IGNORES";
//            return true;
//        }
//        qCDebug(logDebug()) << "SHORTCUT RUNS";
//    }

    return false;
}

void MainWindow::connect_signal_slot() {
//    connect(this->timer, &QTimer::timeout, this, &MainWindow::on_timer);
//    this->timer->start(this->ui->sb_poll_timer->value());

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
    });
    connect(this->modbus_server, &ModbusServer::config_changed, this->modbus_settings_ui, &ModbusSettingsUI::set_config);
}

void MainWindow::read_config(){
    //this->ui->dsb_ph->setValue(cfg.ph);

    //this->ui->sb_poll_timer->setValue(cfg.MODBUS_POLL_TIMER);

    this->modbus_settings_ui->read_config(cfg.MODBUS_SETTINGS);
}

void MainWindow::write_config(){
    //cfg.ph = this->ui->dsb_ph->value();

    //cfg.MODBUS_POLL_TIMER = this->ui->sb_poll_timer->value();

//     QSettings config("config.ini", QSettings::IniFormat);
// #if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
//     config.setIniCodec("UTF-8"); //Windows-1251
// #endif

    // config.beginGroup("parameters");

    // config.setValue("MODBUS_POLL_TIMER", cfg.MODBUS_POLL_TIMER);
    // config.endGroup();

    cfg.MODBUS_SETTINGS = this->modbus_settings_ui->write_config();
}

void MainWindow::menu_bar() {

}

void MainWindow::tool_bar() {

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
//    if (isWindowModified())
//        return AQP::okToClearData(&MainWindow::save_as, this, tr("Несохраненные изменения"), tr("Тестовая карта содержит несохраненные изменения. Сохранить изменения?"));
    return true;
}

void MainWindow::auth_ok() {

}

void MainWindow::on_timer() {
    //qCDebug(logDebug()) << "modbus_read_request";
    //this->modbus_client->on_read(QModbusDataUnit::RegisterType::HoldingRegisters, MODBUS_UPDATE_START_INDEX, MODBUS_UPDATE_CNT);
}

void MainWindow::on_random_timer() {
    if (this->ui->cb_random->isChecked()) {
        double sko = this->ui->dsb_sko->value();
        for (auto &dsb : hr_v) {
            double value = sko * dsb->value() / 100;
            double random = Random::get(-value, value);
            //qCDebug(logDebug()) << sko << value << random;
            dsb->setValue(dsb->value() + random);
        }
    }
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

    if (dataType.toLower().contains("string")) {
        // Обработка строкового регистра
        auto le = this->ui->gb4->findChild<QLineEdit*>("le_" + regName);
        if (le) {
            disconnect(le, &QLineEdit::textChanged, this, &MainWindow::StringHoldingRegistersChanged);
            //le->setText(str);
            QVariant value = ModbusDataConverter::registersToVariant(mdu.values(), regName, dataType, this->modbus_server->modbus_format());
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
                    QVariant value = ModbusDataConverter::registersToVariant(mdu.values(), regName, dataType, this->modbus_server->modbus_format());
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

void create_dsb(QDoubleSpinBox* dsb, const QString &name, const QString &reg_type, const QModbusDataUnit &mdu) {
    dsb->setObjectName(QString("dsb_") + name);
    dsb->setMinimumSize({50, 20});
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
}

void MainWindow::modbus_ui() {
    // Очищаем старый UI
    clear_modbus_ui();

    this->_mdum = modbus_server->get_mdum();
    auto mdum_m = modbus_server->get_sort_by_start_address_mdum_v();
    auto reg_data_type_m = modbus_server->get_reg_data_type_m();

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
                        create_dsb(dsb, regName, reg_data_type, mdu);

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
                        create_dsb(dsb, regName, reg_data_type, mdu);

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
