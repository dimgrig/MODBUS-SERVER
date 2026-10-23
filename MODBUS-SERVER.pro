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
RC_ICONS = resources/modbus.ico

DESTDIR     = $$PWD/../$${TARGET}-binaries/$$DESTINATION_PATH
OBJECTS_DIR = $$PWD/../$${TARGET}-build/$$DESTINATION_PATH/.obj
MOC_DIR     = $$PWD/../$${TARGET}-build/$$DESTINATION_PATH/.moc
RCC_DIR     = $$PWD/../$${TARGET}-build/$$DESTINATION_PATH/.qrc
UI_DIR      = $$PWD/../$${TARGET}-build/$$DESTINATION_PATH/.ui

# ── Windows: копируем DLL рядом с exe ──────────────────────────
win32|win64 {
    CORE_SDK_BIN = $${CORE_SDK_ROOT}/bin
    CORE_DLLS = $$files($${CORE_SDK_BIN}/libcore-*.dll) \
                $$files($${CORE_SDK_BIN}/libqcustomplot*.dll) \
                $$files($${CORE_SDK_BIN}/libQtWebApp*.dll)
    for(dll, CORE_DLLS) {
        # $$QMAKE_COPY на MinGW = "cp -f" (через sh.exe) — /Y невалиден
        QMAKE_POST_LINK += $$QMAKE_COPY $$shell_path($$dll) $$shell_path($$DESTDIR) $$escape_expand(\\n\\t)
    }

    # windeployqt — Windows-binary, ему нужен Windows-путь (C:/..., не /C/...)
    WINDEPLOYQT = $$clean_path($$[QT_INSTALL_BINS]/windeployqt.exe)
    QMAKE_POST_LINK += $$WINDEPLOYQT --no-translations --no-opengl-sw --no-quick-import $$clean_path($$DESTDIR/$${TARGET}.exe) $$escape_expand(\\n\\t)

    MINGW_DIR = $$dirname(QMAKE_CXX)
    for(rt, libstdc++-6.dll libgcc_s_seh-1.dll libgcc_s_dw2-1.dll libwinpthread-1.dll) {
        exists($${MINGW_DIR}/$$rt) {
            QMAKE_POST_LINK += $$QMAKE_COPY $$shell_path($${MINGW_DIR}/$$rt) $$shell_path($$DESTDIR) $$escape_expand(\\n\\t)
        }
    }
}
