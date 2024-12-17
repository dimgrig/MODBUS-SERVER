#include "Config.h"

#include <QApplication>
#include <QProcess>

#include "Profiler.h"
#include "TestRunner.h"

Config::Config() {
    //std::cout  <<  "Config::Config()" << std::endl;
    // must be in build-astra-Desktop_Qt_5_15_2-Debug folder for QT CREATOR
    // must be in cmake-build-debug-mingw folder for cmake
    if (QFile("config.ini").exists()) {
        QSettings config("config.ini", QSettings::IniFormat);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        config.setIniCodec("UTF-8"); //Windows-1251
#endif

        config.beginGroup("calibrator");
        if (config.contains(QString("CALIBRATOR")))
            CALIBRATOR = config.value("CALIBRATOR", false).toBool();
        if (config.contains(QString("CALIBRATOR_PORT")))
            CALIBRATOR_UART_PORT = config.value("CALIBRATOR_PORT", "").toString().toUtf8().constData();
        if (config.contains(QString("CALIBRATOR_SPEED")))
            CALIBRATOR_UART_SPEED = config.value("CALIBRATOR_SPEED", 115200).toInt();
        if (config.contains(QString("CALIBRATOR_TOLERANCE")))
            CALIBRATOR_TOLERANCE = config.value("CALIBRATOR_TOLERANCE", 2).toInt();
        if (config.contains(QString("DEBUG_MSG")))
            DEBUG_MSG_CALIBRATOR = config.value("DEBUG_MSG", false).toBool();
        config.endGroup();

        config.beginGroup("dirs");
        if (config.contains(QString("SELFTEST")))
            SELFTEST = QString::fromUtf8(config.value("SELFTEST", "").toByteArray());
        if (config.contains(QString("TEST_RESULT_DIR")))
            TEST_RESULT_DIR = QString::fromUtf8(config.value("TEST_RESULT_DIR", "").toByteArray());
        if (config.contains(QString("WORKSPACE_DIR")))
            WORKSPACE_DIR = config.value("WORKSPACE_DIR", "").toString().toUtf8().constData();
        if (config.contains(QString("XSD_DIR")))
            XSD_DIR = config.value("XSD_DIR", "").toString().toUtf8().constData();
        config.endGroup();

        config.beginGroup("ethernet");
        if (config.contains(QString("DEBUG_MSG")))
            DEBUG_MSG_ETHERNET = config.value("DEBUG_MSG", false).toBool();
        if (config.contains(QString("IP")))
            IP = config.value("IP", "192.168.1.100").toString().toUtf8().constData();
        if (config.contains(QString("PORT")))
            PORT = config.value("PORT", 80).toInt();
        config.endGroup();

        config.beginGroup("filter");
        if (config.contains(QString("ADAPTIVE_AVERAGE_FAST")))
            ADAPTIVE_AVERAGE_FAST = config.value("ADAPTIVE_AVERAGE_FAST", 1.0).toDouble();
        if (config.contains(QString("ADAPTIVE_AVERAGE_FILTER")))
            ADAPTIVE_AVERAGE_FILTER = config.value("ADAPTIVE_AVERAGE_FILTER", false).toBool();
        if (config.contains(QString("ADAPTIVE_AVERAGE_GAP")))
            ADAPTIVE_AVERAGE_GAP  = config.value(QString("ADAPTIVE_AVERAGE_GAP"), 10).toInt();
        if (config.contains(QString("ADAPTIVE_AVERAGE_SLOW")))
            ADAPTIVE_AVERAGE_SLOW  = config.value(QString("ADAPTIVE_AVERAGE_SLOW"), 0.01).toDouble();
        if (config.contains(QString("AVERAGE_FILTER")))
            AVERAGE_FILTER = config.value("AVERAGE_FILTER", 0).toInt();
        if (config.contains(QString("FILTER")))
            FILTER = config.value("FILTER", false).toBool();
        if (config.contains(QString("MEDIAN_FILTER")))
            MEDIAN_FILTER = config.value("MEDIAN_FILTER", 0).toInt();
        config.endGroup();

        config.beginGroup("interface");
        if (config.contains("INTERFACE_TYPE")) {
            std::string interface_type_str = config.value("INTERFACE_TYPE", "UART").toString().toUtf8().constData();
            if (interface_type_str == "ETHERNET") {
                INTERFACE_TYPE = InterfaceType_ETHERNET;
            } else {
                INTERFACE_TYPE = InterfaceType_UART;
            }
        }
        if (config.contains(QString("MAX_RETRIES")))
            MAX_RETRIES  = config.value(QString("MAX_RETRIES"), 3).toInt();
        if (config.contains(QString("MAX_WAITS")))
            MAX_WAITS  = config.value(QString("MAX_WAITS"), 100).toInt();
        if (config.contains(QString("POLL_TIMER")))
            POLL_TIMER  = config.value(QString("POLL_TIMER"), 10).toInt();
        if (config.contains(QString("STATUS")))
            STATUS = config.value("STATUS", false).toBool();
        if (config.contains(QString("SYNC")))
            SYNC = config.value("SYNC", false).toBool();
        if (config.contains(QString("SYNC_TIMER")))
            SYNC_TIMER  = config.value(QString("SYNC_TIMER"), 100).toInt();
        config.endGroup();

        config.beginGroup("log");
        if (config.contains("FILE"))
            FILE = config.value("FILE", "log.txt").toString().toUtf8().constData();
        if (config.contains("LOG"))
            LOG = config.value("LOG", false).toBool();
        if (config.contains("LOG_DATE"))
            LOG_DATE = config.value("LOG_DATE", false).toBool();
        if (config.contains(QString("LOG_KEEP_DAYS")))
            LOG_KEEP_DAYS  = config.value(QString("LOG_KEEP_DAYS"), 90).toInt();
        if (config.contains("LOG_LEVEL"))
            str_LOG_LEVEL = config.value("LOG_LEVEL", "CRITICAL").toString().toUtf8().constData();
        if (config.contains("LOG_TO_FILE"))
            LOG_TO_FILE = config.value("LOG_TO_FILE", false).toBool();
        if (config.contains("PROFILER"))
            PROFILER = config.value("PROFILER", false).toBool();
        if (config.contains("UNITTEST"))
            UNITTEST = config.value("UNITTEST", false).toBool();
        config.endGroup();

        config.beginGroup("meter");
        if (config.contains(QString("DEBUG_MSG")))
            DEBUG_MSG = config.value("DEBUG_MSG", false).toBool();
        if (config.contains(QString("PXI")))
            PXI = config.value("PXI", false).toBool();
        if (config.contains(QString("PXI_IP")))
            PXI_IP = config.value("PXI_IP", "192.168.1.100").toString().toUtf8().constData();
        if (config.contains(QString("PXI_PORT")))
            PXI_PORT = config.value("PXI_PORT", 5025).toInt();
        config.endGroup();

        config.beginGroup("opi");
        if (config.contains(QString("GROUP_AHEAD_DURATION")))
            GROUP_AHEAD_DURATION = config.value("GROUP_AHEAD_DURATION", 5000).toInt();
        if (config.contains(QString("GROUP1_POLARITY")))
            GROUP_POLARITY = config.value("GROUP_POLARITY", true).toBool();
        if (config.contains(QString("STOP_DURATION")))
            STOP_DURATION = config.value("STOP_DURATION", 5000).toInt();
        config.endGroup();

        //parameters
        config.beginGroup("parameters");
        if (config.contains(QString("MODBUS_POLL_TIMER")))
            MODBUS_POLL_TIMER = config.value("MODBUS_POLL_TIMER", 500).toInt();
        if (config.contains(QString("MODBUS_ADDRESS")))
            MODBUS_ADDRESS = config.value("MODBUS_ADDRESS", 1).toInt();
        if (config.contains(QString("MODBUS_TYPE")))
            MODBUS_TYPE = config.value("MODBUS_TYPE", "TCP").toString();
        if (config.contains(QString("MODBUS_IP")))
            MODBUS_IP = config.value("MODBUS_IP", "192.168.3.1").toString();
        if (config.contains(QString("MODBUS_PORT")))
            MODBUS_PORT = config.value("MODBUS_PORT", 502).toInt();
        if (config.contains(QString("MODBUS_SERIAL_PORT")))
            MODBUS_SERIAL_PORT = config.value("MODBUS_SERIAL_PORT", "COM1").toString();
        if (config.contains(QString("MODBUS_BAUDRATE")))
            MODBUS_BAUDRATE = config.value("MODBUS_BAUDRATE", 115200).toInt();
        if (config.contains(QString("MODBUS_DATA_BITS")))
            MODBUS_DATA_BITS = config.value("MODBUS_DATA_BITS", 8).toInt();
        if (config.contains(QString("MODBUS_STOP_BITS")))
            MODBUS_STOP_BITS = config.value("MODBUS_STOP_BITS", 1).toInt();
        if (config.contains(QString("MODBUS_PARITY")))
            MODBUS_PARITY = config.value("MODBUS_PARITY", "NoParity").toString();
        config.endGroup();

        config.beginGroup("proxy");
        if (config.contains(QString("PROXY")))
            PROXY = config.value("PROXY", true).toBool();
        if (config.contains(QString("PROXY_HOST")))
            PROXY_HOST = config.value("PROXY_HOST", "192.168.7.1").toString();
        if (config.contains(QString("PROXY_PORT")))
            PROXY_PORT = config.value("PROXY_PORT", 3128).toInt();
        config.endGroup();

        config.beginGroup("system");
        if (config.contains("APPLICATION_NAME")) {
            APPLICATION_NAME = config.value("APPLICATION_NAME", "ACS").toString().toUtf8().constData();
#ifdef DIODE
            APPLICATION_NAME = "РМО Диоды";
#endif
#ifdef FET2
            APPLICATION_NAME = "РМО ПТ2";
#endif
#ifdef FET5
            APPLICATION_NAME = "РМО ПТ5";
#endif
        }
        if (config.contains(QString("INIT_USER_LEVEL")))
            INIT_USER_LEVEL = config.value("INIT_USER_LEVEL", "Испытатель").toString().toUtf8().constData();
        if (config.contains("ORGANIZATION_NAME"))
            ORGANIZATION_NAME = config.value("ORGANIZATION_NAME", "ACS").toString().toUtf8().constData();
        if (config.contains("OS"))
            OS = config.value("OS", "WIN").toString().toUtf8().constData();
        if (config.contains("VERSION"))
            VERSION = config.value("VERSION", "0.0.0").toString().toUtf8().constData();
        config.endGroup();

        config.beginGroup("test");
        if (config.contains(QString("DEBUG_MSG")))
            DEBUG_MSG_TEST = config.value("DEBUG_MSG", false).toBool();
        if (config.contains(QString("IGNORE_LOCK")))
            IGNORE_LOCK = config.value("IGNORE_LOCK", false).toBool();
        if (config.contains(QString("INGORE_MODEL_TYPE")))
            INGORE_MODEL_TYPE = config.value("INGORE_MODEL_TYPE", false).toBool();
        if (config.contains(QString("RESULT_REQUEST_TIMEOUT")))
            RESULT_REQUEST_TIMEOUT = config.value("RESULT_REQUEST_TIMEOUT", 100).toInt();
        if (config.contains(QString("RMO_diode_stm")))
            RMO_diode_stm = config.value("RMO_diode_stm", false).toBool();
        if (config.contains(QString("STM_READY_IMIT")))
            STM_READY_IMIT = config.value("STM_READY_IMIT", false).toBool();
        if (config.contains(QString("TEST_RESULT_KEEP_DAYS")))
            TEST_RESULT_KEEP_DAYS  = config.value(QString("TEST_RESULT_KEEP_DAYS"), 90).toInt();
        config.endGroup();

        config.beginGroup("uart");
        if (config.contains(QString("DEBUG_MSG")))
            DEBUG_MSG_UART = config.value("DEBUG_MSG", false).toBool();
        if (config.contains(QString("PORT")))
            UART_PORT = config.value("PORT", "").toString().toUtf8().constData();
        if (config.contains(QString("SPEED")))
            UART_SPEED = config.value("SPEED", 115200).toInt();
        config.endGroup();

        config.beginGroup("user_settings");
        if (config.contains(QString("CALIBRATION_ADC")))
            TEST_CALIBRATION_ADC = config.value("CALIBRATION_ADC", true).toBool();
        if (config.contains(QString("CALIBRATION_DAC")))
            TEST_CALIBRATION_DAC = config.value("CALIBRATION_DAC", true).toBool();
        if (config.contains(QString("CALIBRATION_DURATION")))
            TEST_CALIBRATION_DURATION = config.value("CALIBRATION_DURATION", true).toBool();
        if (config.contains(QString("ENABLE_SHORTCUTS")))
            ENABLE_SHORTCUTS = config.value("ENABLE_SHORTCUTS", false).toBool();
        if (config.contains(QString("IGNORE_FAILED")))
            IGNORE_FAILED = config.value("IGNORE_FAILED", false).toBool();
        if (config.contains(QString("RESET_BATCH_NUMBERS")))
            RESET_BATCH_NUMBERS = config.value("RESET_BATCH_NUMBERS", false).toBool();
        if (config.contains(QString("SAVE_TEST_RESULTS")))
            SAVE_TEST_RESULTS = config.value("SAVE_TEST_RESULTS", false).toBool();
        config.endGroup();
    }
    else {
        std::cout << "Config file does not exist";
        //qCDebug(logFatal()) << "Config file does not exist";
    }

    LOG_LEVEL = LOG_LEV::NONE_LOG;
    if (str_LOG_LEVEL == "DEBUG")
        LOG_LEVEL = LOG_LEV::DEBUG;
    if (str_LOG_LEVEL == "INFO")
        LOG_LEVEL = LOG_LEV::INFO;
    if (str_LOG_LEVEL == "WARNING")
        LOG_LEVEL = LOG_LEV::WARNING;
    if (str_LOG_LEVEL == "CRITICAL")
        LOG_LEVEL = LOG_LEV::CRITICAL;
    if (str_LOG_LEVEL == "FATAL")
        LOG_LEVEL = LOG_LEV::FATAL;

    {
        Logger::__log_enable = LOG;
        Logger::__log_to_file = LOG_TO_FILE;
        Logger::__log_date = LOG_DATE;
        Logger::__log_file = FILE;
        Logger::__log_keep_days = LOG_KEEP_DAYS;
        Logger::__log_level = static_cast<LOG_LEV>(LOG_LEVEL);

        Logger::log_init();

        Profiler::_enable = PROFILER;
        TestRunner::_enable = UNITTEST;
    }

    QString script = "/bin/bash";
    QString passwd = "echo '123' | sudo -S /bin/bash -c ";
    QString app = QFileInfo(QApplication::applicationFilePath()).filePath();
    QString cmd;

    cmd = QString("'setcap 'cap_net_bind_service=+ep' ") + app + QString("'");
    QProcess::startDetached(script, QStringList() << "-c" << passwd + cmd);
    qCDebug(logDebug) << passwd << cmd;

    if (OS == "WIN"){
        //PORT = "COM14"; // #"COM7"
    }
    else if ((OS == "ASTRA") || (OS == "UBUNTU")) {
        /*
        #sudo dmesg | grep tty
        #ls /dev/serial/by-id/
        sudo_password = '123'
        command = 'chmod 777 /dev/ttyUSB0'
        os.system('echo {}|sudo -S {}'.format(sudo_password, command))
        #self.port = "/dev/serial/by-id/usb-Silicon_Labs_CP2103_USB_to_UART_Bridge_Controller_IAB1306851-if00-port0"
        self.port = "/dev/serial/by-id/usb-1a86_USB2.0-Serial-if00-port0"
        #ln -s /dev/ttyUSB1 ~/.wine/dosdevices/com6
         */
    }
}
