QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets network printsupport serialport serialbus

CONFIG += c++17

#QMAKE_CXXFLAGS += -Wno-deprecated #-std=c++2a

DISTFILES += \
    qmake-destination-path.pri \
    qmake-modbus-server-version.pri \
    qmake-target-platform.pri

include(qmake-target-platform.pri)
include(qmake-modbus-server-version.pri)
include(qmake-destination-path.pri)

INCLUDEPATH += "$$_PRO_FILE_PWD_/"
INCLUDEPATH += "$${CORE_SDK_ROOT}/include"

# Линковка
unix {
    LIBS += -L$${CORE_SDK_ROOT}/lib \
            -lcore-lib \
            -lcore-journal-lib \
            -lcore-modbus-lib \
            -lcore-widgets-lib \
            -ldl
}
win32|win64 {
    LIBS += -L$${WIN_LIBS_PATH} \
            -lcore-lib \
            -lcore-journal-lib \
            -lcore-modbus-lib \
            -lcore-widgets-lib
}

SOURCES += \
    Config.cpp \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    Config.h \
    mainwindow.h

FORMS += \
    uis/mainwindow.ui

RESOURCES += \
    resources/resources.qrc

TRANSLATIONS += QtLanguage_ru.ts
CODECFORSRC = UTF-8
RC_ICONS = resources/app_acs.ico

DESTDIR     = $$PWD/../$${TARGET}-binaries/$$DESTINATION_PATH
OBJECTS_DIR = $$PWD/../$${TARGET}-build/$$DESTINATION_PATH/.obj
MOC_DIR     = $$PWD/../$${TARGET}-build/$$DESTINATION_PATH/.moc
RCC_DIR     = $$PWD/../$${TARGET}-build/$$DESTINATION_PATH/.qrc
UI_DIR      = $$PWD/../$${TARGET}-build/$$DESTINATION_PATH/.ui
