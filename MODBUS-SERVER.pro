QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets network printsupport serialport serialbus

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

#QMAKE_CXXFLAGS += -Wno-deprecated #-std=c++2a

DISTFILES += \
    qmake-destination-path.pri \
    qmake-modbus-server-version.pri \
    qmake-target-platform.pri

include(qmake-target-platform.pri)
include(qmake-modbus-server-version.pri)
include(qmake-destination-path.pri)

DEFINES += NOCRYPT #fix DOMDocument ambigious with qcustomplot
DEFINES += NOUSER

message(VERSION: $${RMO_TYPE})

SOURCES += \
    Config.cpp \
    Logger.cpp \
    MessageBox.cpp \
    ModbusBase.cpp \
    ModbusServer.cpp \
    Profiler.cpp \
    Sleeper.cpp \
    TestRunner.cpp \
    aqp.cpp \
    functions.cpp \
    macroses.cpp \
    main.cpp \
    mainwindow.cpp \
    templates.cpp \
    templates_cout.cpp

HEADERS += \
    Config.h \
    Logger.h \
    MessageBox.h \
    ModbusBase.h \
    ModbusServer.h \
    Profiler.h \
    Random.h \
    Sleeper.h \
    TestRunner.h \
    aqp.h \
    functions.h \
    icons.h \
    interfaces.h \
    macroses.h \
    mainwindow.h \
    templates.h \
    templates_cout.h


FORMS += \
    uis/Auth.ui \
    uis/User.ui \
    uis/Users.ui \
    uis/about.ui \
    uis/mainwindow.ui \
    uis/modbus_server.ui \
    uis/settings.ui

RESOURCES += \
    resources/resources.qrc

TRANSLATIONS += widgets.ts

RC_ICONS = resources/app_acs.ico

TRANSLATIONS += QtLanguage_ru.ts
CODECFORSRC     = UTF-8

# Default rules for deployment.
#qnx: target.path = /tmp/$${TARGET}/bin
#else: unix:!android: target.path = /opt/$${TARGET}/bin
#!isEmpty(target.path): INSTALLS += target

DESTDIR = $$PWD/../$${TARGET}-binaries/$$DESTINATION_PATH
OBJECTS_DIR = $$PWD/../$${TARGET}-build/$$DESTINATION_PATH/.obj
MOC_DIR = $$PWD/../$${TARGET}-build/$$DESTINATION_PATH/.moc
RCC_DIR = $$PWD/../$${TARGET}-build/$$DESTINATION_PATH/.qrc
UI_DIR = $$PWD/../$${TARGET}-build/$$DESTINATION_PATH/.ui




