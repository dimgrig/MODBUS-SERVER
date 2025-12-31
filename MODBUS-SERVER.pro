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

QMAKE_LFLAGS += -Wl,-rpath,'/usr/lib/qt'

exists(/etc/astra_version) {
    LIBS += -L/usr/lib/qt/ -lcore-lib
    LIBS += -L/usr/lib/qt/ -lmodbus-core-lib

    DISTFILES += core-gcc10.pri
    include(core-gcc10.pri)

    message($${ASTRA_GCC_PATH})
    message($${ASTRA_QT_PATH})
} else {
    unix {
        LIBS += -L/usr/lib/qt/ -lcore-lib
        LIBS += -L/usr/lib/qt/ -lmodbus-core-lib
    }

    win32 {
        DEFINES += win32
        LIBS += -L$${WIN_LIBS_PATH} -lcore-lib
        LIBS += -L$${WIN_LIBS_PATH} -lmodbus-core-lib
    }
    win64 {
        LIBS += -L$${WIN_LIBS_PATH} -lcore-lib
        LIBS += -L$${WIN_LIBS_PATH} -lmodbus-core-lib
    }
}

INCLUDEPATH += "$$_PRO_FILE_PWD_/"
INCLUDEPATH += "$$_PRO_FILE_PWD_/../Core"
INCLUDEPATH += "$$_PRO_FILE_PWD_/../ModbusCore"

SOURCES += \
    Config.cpp \
    MessageBox.cpp \
    aqp.cpp \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    Config.h \
    Random.h \
    aqp.h \
    icons.h \
    mainwindow.h

FORMS += \
    uis/mainwindow.ui

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




