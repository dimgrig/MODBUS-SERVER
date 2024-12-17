//
// Created by Dima on 07.03.2021.
//

#ifndef ASTRA_CONFIG_H
#define ASTRA_CONFIG_H

#include <iostream>

#include <QtDebug>
#include <QCoreApplication>
#include <QFile>
#include <QSettings>

//#define QT_5_15

#include "Logger.h"

#ifdef QT_5_15
    #define QT_HEX Qt::hex
#else
    #define QT_HEX ""
#endif

#define _US 1000000

#define TIME_FORMAT "HH:mm:ss"
#define TIME_FORMAT_SHORT "mm:ss"
#define TIME_FORMAT_MS "HH:mm:ss.zzz"
#define DT_FORMAT "yyyy-MM-dd HH:mm:ss.zzz"
#define DT_FORMAT_WITHOUT_MS "yyyy-MM-dd HH-mm-ss"
#define DT_FORMAT_SHORT "HH:mm:ss"
#define DT_FORMAT_WRAP "yyyy-MM-dd\nHH:mm:ss.zzz"

#define SETTINGS_PATH "settings/"

#define DEGREE_SYMBOL QChar(176)

#define PXI_ERROR_VALUE -999.999

#define CONTROL_MINIMUM_WIDTH 150
#define CONTROL_MINIMUM_HEIGHT 25

#define _TRANSPARENT QColor(0, 0, 0, 0)
#define BLACK_COLOR "#000000"
#define DGRAY_COLOR "#333333"
#define GRAY_COLOR "#999999"
#define LGRAY_COLOR "#CCCCCC"
#define WHITE_COLOR "#FFFFFF"

#define R_COLOR "#FF3E3E" //#"#CD5C5C"
#define G_COLOR "#32CD32"
#define B_COLOR "#4169E1"

#define LO_COLOR "#FFBE00"
#define O_COLOR "#FFA500" //#for TextItems Avaria
#define DO_COLOR "#FF8C00" //#FF953E

#define LG_COLOR "#90EE90" //#for TextItems
#define DG_COLOR "#006400"

#define LB_COLOR "#ADD8E6" //#for TextItems
#define DB_COLOR "#00008B"

#define _LR_COLOR "#FA8072" //#for TextItems
#define DR_COLOR "#B22222" //#8B0000

#define R_BG_COLOR QColor(255, 0, 0, 20) //"#FF6666"


#define V_COLOR "#EE82EE" //for VP
#define DV_COLOR "#9400D3"

#define SB_COLOR "#87CEEB" //for O2
#define LSB_COLOR "#B0E0E6" //87CEFA
#define DSB_COLOR "#00BFFF"


#define COLOR_Silver	"#C0C0C0"
#define COLOR_Gray	    "#808080"
#define COLOR_Black	    "#000000"
#define COLOR_Red	    "#FF0000"
#define COLOR_Maroon	"#800000"
#define COLOR_Yellow	"#FFFF00"
#define COLOR_Olive	    "#808000"
#define COLOR_Lime	    "#00FF00"
#define COLOR_Green	    "#008000"
#define COLOR_Aqua	    "#00FFFF"
#define COLOR_Teal	    "#008080"
#define COLOR_Blue	    "#0000FF"
#define COLOR_Navy	    "#000080"
#define COLOR_Fuchsia	"#FF00FF"
#define COLOR_Purple	"#800080"
#define COLOR_Orange	    "#FFA500"
#define COLOR_DeepPink	    "#FF1493"
#define COLOR_Chocolate	    "#D2691E"
#define COLOR_DarkSlateGray	"#2F4F4F"
#define COLOR_SandyBrown	"#F4A460"
#define COLOR_LightSteelBlue "#B0C4DE"
#define COLOR_Aquamarine	"#7FFFD4"
#define COLOR_PaleGreen	    "#98FB98"
#define COLOR_Violet	    "#EE82EE"

enum InterfaceType {
    InterfaceType_UART,
    InterfaceType_ETHERNET
};

#define cfg Config::instance()

class Config {
public:
    static Config& instance() {
        static Config inst;
        return inst;
    }

    //calibrator
    bool CALIBRATOR = false;
    QString CALIBRATOR_UART_PORT = "";
    int CALIBRATOR_UART_SPEED = 115200;
    int CALIBRATOR_TOLERANCE = 2;
    bool DEBUG_MSG_CALIBRATOR = false;

    //dirs
    QString SELFTEST = "";
    QString TEST_RESULT_DIR = "";
    QString WORKSPACE_DIR = "";
    QString XSD_DIR = "";

    //ethernet
    bool DEBUG_MSG_ETHERNET = false;
    QString IP = "";
    int PORT = 80;

    //filter
    double ADAPTIVE_AVERAGE_FAST = 1.0;
    bool ADAPTIVE_AVERAGE_FILTER = false;
    int ADAPTIVE_AVERAGE_GAP = 200;
    double ADAPTIVE_AVERAGE_SLOW = 0.01;
    int AVERAGE_FILTER = 0;
    bool FILTER = false;
    int MEDIAN_FILTER = 0;

    //interface
    InterfaceType INTERFACE_TYPE = InterfaceType_UART;
    int MAX_RETRIES = 3;
    int MAX_WAITS = 100;
    int POLL_TIMER = 10;
    int STATUS = 1;
    int SYNC = 1;
    int SYNC_TIMER = 100;

    //log
    std::string FILE = "";
    bool LOG = false;
    bool LOG_DATE = false;
    int LOG_KEEP_DAYS = 90;
    LOG_LEV LOG_LEVEL;
    std::string str_LOG_LEVEL;
    bool LOG_TO_FILE = false;
    bool PROFILER = false;
    bool UNITTEST = false;

    //opi
    int GROUP_AHEAD_DURATION = 5000;
    bool GROUP_POLARITY = false;
    int STOP_DURATION = 5000;

    //meter
    bool DEBUG_MSG = false;
    bool PXI = false;
    QString PXI_IP;
    int PXI_PORT = 5025;

    //parameters
    int MODBUS_POLL_TIMER = 500;
    int MODBUS_ADDRESS = 1;
    QString MODBUS_TYPE = "TCP";
    QString MODBUS_IP = "192.168.3.1";
    int MODBUS_PORT = 502;
    QString MODBUS_SERIAL_PORT = "COM1";
    int MODBUS_BAUDRATE = 115200;
    int MODBUS_DATA_BITS = 8;
    int MODBUS_STOP_BITS = 1;
    QString MODBUS_PARITY = "NoParity";

    //proxy
    bool PROXY = true;
    QString PROXY_HOST = "192.168.1.7";
    int PROXY_PORT = 3128;

    //system
    QString APPLICATION_NAME = "";
    QString INIT_USER_LEVEL = "Испытатель";
    QString ORGANIZATION_NAME = "";    
    QString OS = "WIN";
    QString VERSION = "1.0.0";

    //test
    bool DEBUG_MSG_TEST = false;
    bool IGNORE_LOCK = false;
    bool INGORE_MODEL_TYPE = false;
    bool LOCAL_SERVER = false;
    int RESULT_REQUEST_TIMEOUT = 100;
    bool RMO_diode_stm = false;
    bool STM_READY_IMIT = false;
    int TEST_RESULT_KEEP_DAYS = 90;

    //uart
    bool DEBUG_MSG_UART = false;
    QString UART_PORT = "";
    int UART_SPEED = 115200;

    //user_settings
    bool TEST_CALIBRATION_ADC = true;
    bool TEST_CALIBRATION_DAC = true;
    bool TEST_CALIBRATION_DURATION = true;
    bool ENABLE_SHORTCUTS = false;
    bool IGNORE_FAILED = false;
    bool RESET_BATCH_NUMBERS = false;
    bool SAVE_TEST_RESULTS = false;
private:
    Config();
    Config(const Config&) = delete;
    Config& operator=(const Config&) = delete;
    ~Config() {
        //std::cout << "Config::~Config()" << std::endl;
    }
};

//inline Config const cfg;

#endif //ASTRA_CONFIG_H
