#include "Config.h"

#include <QApplication>
#include <QProcess>

#include <core-lib/Profiler.h>
#include <core-lib/TestRunner.h>

Config::Config() {
    //std::cout  <<  "Config::Config()" << std::endl;
    // must be in build-astra-Desktop_Qt_5_15_2-Debug folder for QT CREATOR
    // must be in cmake-build-debug-mingw folder for cmake
    if (QFile(CoreConfig::configFile()).exists()) {
        QSettings config(CoreConfig::configFile(), QSettings::IniFormat);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        config.setIniCodec("UTF-8"); //Windows-1251
#endif
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

        config.beginGroup("modbus");
        if (config.contains(QString("MODBUS_POLL_TIMER")))
            MODBUS_SETTINGS.MODBUS_POLL_TIMER = config.value("MODBUS_POLL_TIMER", 500).toInt();
        if (config.contains(QString("MODBUS_ADDRESS")))
            MODBUS_SETTINGS.MODBUS_ADDRESS = config.value("MODBUS_ADDRESS", 1).toInt();
        if (config.contains(QString("MODBUS_TYPE")))
            MODBUS_SETTINGS.MODBUS_TYPE = config.value("MODBUS_TYPE", "TCP").toString();
        if (config.contains(QString("MODBUS_IP")))
            MODBUS_SETTINGS.MODBUS_IP = config.value("MODBUS_IP", "192.168.1.5").toString();
        if (config.contains(QString("MODBUS_PORT")))
            MODBUS_SETTINGS.MODBUS_PORT = config.value("MODBUS_PORT", 502).toInt();
        if (config.contains(QString("MODBUS_SERIAL_PORT")))
            MODBUS_SETTINGS.MODBUS_SERIAL_PORT = config.value("MODBUS_SERIAL_PORT", "COM1").toString();
        if (config.contains(QString("MODBUS_BAUDRATE")))
            MODBUS_SETTINGS.MODBUS_BAUDRATE = config.value("MODBUS_BAUDRATE", 115200).toInt();
        if (config.contains(QString("MODBUS_DATA_BITS")))
            MODBUS_SETTINGS.MODBUS_DATA_BITS = config.value("MODBUS_DATA_BITS", 8).toInt();
        if (config.contains(QString("MODBUS_STOP_BITS")))
            MODBUS_SETTINGS.MODBUS_STOP_BITS = config.value("MODBUS_STOP_BITS", 1).toInt();
        if (config.contains(QString("MODBUS_PARITY")))
            MODBUS_SETTINGS.MODBUS_PARITY = config.value("MODBUS_PARITY", "NoParity").toString();
        if (config.contains(QString("MODBUS_FORMAT")))
            MODBUS_SETTINGS.MODBUS_FORMAT = config.value("MODBUS_FORMAT", "ABCD").toString();
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

    // QString script = "/bin/bash";
    // QString passwd = "echo '123' | sudo -S /bin/bash -c ";
    // QString app = QFileInfo(QApplication::applicationFilePath()).filePath();
    // QString cmd;

    // cmd = QString("'setcap 'cap_net_bind_service=+ep' ") + app + QString("'");
    // QProcess::startDetached(script, QStringList() << "-c" << passwd + cmd);
    // qCDebug(logDebug) << passwd << cmd;

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
