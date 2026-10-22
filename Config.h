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

#include <core-lib/Logger.h>
#include <core-modbus-lib/ModbusSettings.h>

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

    //modbus
    ModbusSettings MODBUS_SETTINGS;

    //system
    QString APPLICATION_NAME = "";
    QString INIT_USER_LEVEL = "Испытатель";
    QString ORGANIZATION_NAME = "";    
    QString OS = "WIN";
    QString VERSION = "1.0.0";
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
