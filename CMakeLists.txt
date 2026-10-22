cmake_minimum_required(VERSION 3.18)
project(modbus-server VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC_SEARCH_PATHS "${CMAKE_CURRENT_SOURCE_DIR}/uis")

# ── Core SDK: env CORE_SDK_ROOT > CMAKE_PREFIX_PATH > per-platform default ──
if(DEFINED ENV{CORE_SDK_ROOT} AND NOT DEFINED Core_DIR)
    list(PREPEND CMAKE_PREFIX_PATH "$ENV{CORE_SDK_ROOT}")
    message(STATUS "modbus-server: Core SDK from env CORE_SDK_ROOT=$ENV{CORE_SDK_ROOT}")
elseif(NOT DEFINED Core_DIR)
    if(WIN32)
        set(_core_default "$ENV{LOCALAPPDATA}/Core/latest")
    else()
        set(_core_default "$ENV{HOME}/.local/core/latest")
    endif()
    if(EXISTS "${_core_default}/lib/cmake/Core/CoreConfig.cmake")
        list(APPEND CMAKE_PREFIX_PATH "${_core_default}")
        message(STATUS "modbus-server: fallback Core SDK at ${_core_default}")
    endif()
endif()

find_package(Core CONFIG REQUIRED)

add_executable(modbus-server WIN32
    main.cpp
    mainwindow.cpp
    mainwindow.h
    Config.cpp
    Config.h
    uis/mainwindow.ui
    resources/resources.qrc
)

target_include_directories(modbus-server PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(modbus-server PRIVATE
    Core::core-lib
    Core::core-journal-lib
    Core::core-modbus-lib
    Core::core-widgets-lib
)

# RPATH: чтобы приложение запускалось без LD_LIBRARY_PATH.
# $<TARGET_FILE_DIR:...> работает для imported targets и не требует LOCATION.
set_target_properties(modbus-server PROPERTIES
    BUILD_RPATH   "$<TARGET_FILE_DIR:Core::core-widgets-lib>"
    INSTALL_RPATH "$ORIGIN")
