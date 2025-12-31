# Файл настроек для Astra Linux с GCC 10

# Пути к компилятору
ASTRA_GCC_PATH = /home/upkd/opt/gcc-10.1.0
ASTRA_QT_PATH = /opt/qt5.15.12

# Настройки компилятора
QMAKE_CC  = $${ASTRA_GCC_PATH}/bin/gcc
QMAKE_CXX = $${ASTRA_GCC_PATH}/bin/g++

# Флаги компилятора
QMAKE_CFLAGS   += -I$${ASTRA_GCC_PATH}/include
QMAKE_CXXFLAGS += -I$${ASTRA_GCC_PATH}/include -std=c++17

QMAKE_LIBDIR += $${ASTRA_QT_PATH}/lib
QMAKE_INCDIR += $${ASTRA_QT_PATH}/include
QMAKE_INCDIR_QT = $${ASTRA_QT_PATH}/include/qt5
QMAKE_LIBDIR_QT = $${ASTRA_QT_PATH}/lib

# ЯВНО указываем компиляторы
QMAKE_CC = $${ASTRA_GCC_PATH}/bin/gcc
QMAKE_CXX = $${ASTRA_GCC_PATH}/bin/g++
QMAKE_LINK = $${ASTRA_GCC_PATH}/bin/g++
QMAKE_LINK_SHLIB = $${ASTRA_GCC_PATH}/bin/g++


# Указываем правильные пути для moc
QMAKE_MOC = /$${ASTRA_QT_PATH}/bin/moc
QMAKE_MOC_FLAGS += -D__GNUC__=10 -D__GNUC_MINOR__=3 -D__GNUC_PATCHLEVEL__=0

# Очищаем переменные окружения для moc
QMAKE_MOC_ENV += "CPLUS_INCLUDE_PATH=$${ASTRA_GCC_PATH}/include/c++/10.1.0:$${ASTRA_GCC_PATH}/include/c++/10.1.0/x86_64-pc-linux-gnu:$${ASTRA_GCC_PATH}/include/c++/10.1.0/backward:$${ASTRA_GCC_PATH}/include:/usr/include"

# Добавляем флаги для компилятора
QMAKE_CXXFLAGS += -isystem $${ASTRA_GCC_PATH}/include/c++/10.1.0 \
                  -isystem $${ASTRA_GCC_PATH}/include/c++/10.1.0/x86_64-pc-linux-gnu \
                  -isystem $${ASTRA_GCC_PATH}/include/c++/10.1.0/backward \
                  -isystem $${ASTRA_GCC_PATH}/include

unix {
    LIBS += -L$${ASTRA_QT_PATH}
    LIBS += -L$${ASTRA_GCC_PATH}/lib64 -lstdc++ -lgcc_s -ldl -lpthread -lGL -lm \
        -lc -lpng16 -lz -ldouble-conversion -licui18n -licuuc -licudata -lzstd \
        -lGLdispatch -lGLX -lpcre -lX11 -lxcb -lXau -lXdmcp -lrt
        #-lgthread -lglib -lbsd

    QMAKE_RPATHDIR += $${ASTRA_QT_PATH}/lib
    QMAKE_RPATHDIR += $${ASTRA_GCC_PATH}/lib64

    QMAKE_LFLAGS   += -L$${ASTRA_GCC_PATH}/lib64 -Wl,-rpath,$${ASTRA_GCC_PATH}/lib64
}
#########
