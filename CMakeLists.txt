cmake_minimum_required(VERSION 3.18)

# ── Версия из VERSION ─────────────────────
file(STRINGS "${CMAKE_CURRENT_SOURCE_DIR}/VERSION" _RAW_VER LIMIT_COUNT 1)
string(STRIP "${_RAW_VER}" MODBUS_VERSION)
if(NOT MODBUS_VERSION MATCHES "^[0-9]+\\.[0-9]+\\.[0-9]+$")
    message(FATAL_ERROR "VERSION must be X.Y.Z, got '${MODBUS_VERSION}'")
endif()

project(modbus-server VERSION ${MODBUS_VERSION} LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC_SEARCH_PATHS "${CMAKE_CURRENT_SOURCE_DIR}/uis")

# ── Qt major: явный -D > Qt Creator kit > qmake path > default 5 ──
set(QT_VERSION_MAJOR "" CACHE STRING "Qt major version (5 or 6). Empty = autodetect")

if(NOT QT_VERSION_MAJOR)
    if(DEFINED QT_DEFAULT_MAJOR_VERSION AND NOT "${QT_DEFAULT_MAJOR_VERSION}" STREQUAL "")
        set(QT_VERSION_MAJOR "${QT_DEFAULT_MAJOR_VERSION}")
        message(STATUS "modbus-server: QT_VERSION_MAJOR=${QT_VERSION_MAJOR} (from Qt Creator kit)")
    elseif(DEFINED QT_QMAKE_EXECUTABLE AND "${QT_QMAKE_EXECUTABLE}" MATCHES "/qt6/")
        set(QT_VERSION_MAJOR 6)
        message(STATUS "modbus-server: QT_VERSION_MAJOR=6 (from QT_QMAKE_EXECUTABLE)")
    else()
        set(QT_VERSION_MAJOR 5)
        message(STATUS "modbus-server: QT_VERSION_MAJOR=5 (default)")
    endif()
endif()

# ── Опции ───────────────────────────────────────────────────────
option(CORE_SDK_FETCH       "Download Core SDK from GitHub releases" OFF)
option(MODBUS_SERVER_DEPLOY "Copy runtime DLLs next to exe (Windows only)" ON)

set(CORE_SDK_VERSION "1.1.0"        CACHE STRING "Core SDK version to fetch")
set(CORE_SDK_REPO    "dimgrig/Core" CACHE STRING "GitHub repo with Core releases")

# ── Резолвинг Core SDK (паритет с Saks/cmake/Paths.cmake) ───────
if(CORE_SDK_FETCH)
    include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/FetchCoreSDK.cmake)
elseif(NOT DEFINED Core_DIR)
    set(_core_sdk_candidates)

    # 1) env CORE_SDK_ROOT — высший приоритет
    if(DEFINED ENV{CORE_SDK_ROOT} AND NOT "$ENV{CORE_SDK_ROOT}" STREQUAL "")
        list(APPEND _core_sdk_candidates "$ENV{CORE_SDK_ROOT}")
    endif()

    # 2) раскладка cmake --install: qt<major>/<ver> и qt<major>/latest
    if(WIN32)
        if(DEFINED ENV{LOCALAPPDATA} AND NOT "$ENV{LOCALAPPDATA}" STREQUAL "")
            list(APPEND _core_sdk_candidates
                "$ENV{LOCALAPPDATA}/Core/qt${QT_VERSION_MAJOR}/${CORE_SDK_VERSION}"
                "$ENV{LOCALAPPDATA}/Core/qt${QT_VERSION_MAJOR}/latest")
        endif()
    else()
        if(DEFINED ENV{HOME} AND NOT "$ENV{HOME}" STREQUAL "")
            list(APPEND _core_sdk_candidates
                "$ENV{HOME}/.local/core/qt${QT_VERSION_MAJOR}/${CORE_SDK_VERSION}"
                "$ENV{HOME}/.local/core/qt${QT_VERSION_MAJOR}/latest")
        endif()
    endif()

    # 3) legacy-раскладка (без qt-подкаталога)
    if(WIN32)
        if(DEFINED ENV{LOCALAPPDATA})
            list(APPEND _core_sdk_candidates
                "$ENV{LOCALAPPDATA}/Core/${CORE_SDK_VERSION}"
                "$ENV{LOCALAPPDATA}/Core/latest")
        endif()
    else()
        if(DEFINED ENV{HOME})
            list(APPEND _core_sdk_candidates
                "$ENV{HOME}/.local/core/${CORE_SDK_VERSION}"
                "$ENV{HOME}/.local/core/latest")
        endif()
        list(APPEND _core_sdk_candidates
            "/opt/core/qt${QT_VERSION_MAJOR}/${CORE_SDK_VERSION}"
            "/opt/core/qt${QT_VERSION_MAJOR}/latest"
            "/opt/core/${CORE_SDK_VERSION}"
            "/opt/core/latest")
    endif()

    foreach(_cand IN LISTS _core_sdk_candidates)
        if(EXISTS "${_cand}/lib/cmake/Core/CoreConfig.cmake")
            set(Core_DIR "${_cand}/lib/cmake/Core"
                CACHE PATH "Core SDK cmake directory" FORCE)
            message(STATUS "modbus-server: Core SDK (Qt${QT_VERSION_MAJOR}) found at ${_cand}")
            break()
        endif()
    endforeach()
    unset(_core_sdk_candidates)
    unset(_cand)
endif()

# find_dependency(Qt5 …) уже внутри CoreConfig.cmake — отдельный
# find_package(Qt5) не нужен (но и не помешает).
find_package(Core CONFIG REQUIRED)

# ── Qt6 + MinGW: host rcc (gcc_64) собран с zstd, target Qt6Core (mingw_64) — без ──
# ── rcc по умолчанию сжимает .qrc через Zstd и генерирует вызов              ──
# ── qResourceFeatureZstd(), которого нет в mingw Qt6Core → undefined reference. ──
# ── Отключаем сжатие при генерации ресурсов.                                 ──
if(QT_VERSION_MAJOR EQUAL 6 AND MINGW)
    set(CMAKE_AUTORCC_OPTIONS "-compress-algo;none")
    message(STATUS "modbus-client: AUTORCC_OPTIONS = -compress-algo none (Qt6 mingw zstd workaround)")
endif()

# ── Исполняемый файл ────────────────────────────────────────────
add_executable(modbus-server WIN32
    main.cpp
    mainwindow.cpp
    mainwindow.h
    Config.cpp
    Config.h
    uis/mainwindow.ui
    resources/resources.qrc
)

install(TARGETS modbus-server
    RUNTIME DESTINATION "bin")

target_include_directories(modbus-server PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(modbus-server PRIVATE
    Core::core-lib
    Core::core-journal-lib
    Core::core-modbus-lib
    Core::core-widgets-lib)

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/appinfo.env.in
    ${CMAKE_CURRENT_BINARY_DIR}/appinfo.env
    @ONLY
)

file(READ "${CMAKE_CURRENT_BINARY_DIR}/appinfo.env" _appinfo_check)
if(_appinfo_check MATCHES "APP_VERSION=\"\"")
    message(FATAL_ERROR
        "appinfo.env generated with empty APP_VERSION. "
        "Template uses @VERSION@ but CMake defines PROJECT_VERSION/MODBUS_VERSION. "
        "Use @PROJECT_VERSION@ in appinfo.env.in.")
endif()

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/appinfo.env DESTINATION .)

# ── RPATH (Linux: исполняемый файл сам находит <sdk>/lib) ───────
set_target_properties(modbus-server PROPERTIES
    BUILD_RPATH   "$<TARGET_FILE_DIR:Core::core-widgets-lib>"
    INSTALL_RPATH "$ORIGIN;$ORIGIN/../lib")

# ── Windows: самодостаточный набор DLL ──────────────────────────
if(WIN32 AND MODBUS_SERVER_DEPLOY)
    # 1) Core DLL из <sdk>/bin
    get_target_property(_core_widgets_dll Core::core-widgets-lib IMPORTED_LOCATION_RELEASE)
    if(NOT _core_widgets_dll)
        get_target_property(_core_widgets_dll Core::core-widgets-lib IMPORTED_LOCATION)
    endif()
    get_filename_component(_sdk_bin "${_core_widgets_dll}" DIRECTORY)

    file(GLOB _core_dlls
        "${_sdk_bin}/libcore-*.dll"   "${_sdk_bin}/core-*.dll"
        "${_sdk_bin}/libqcustomplot*.dll" "${_sdk_bin}/qcustomplot*.dll"
        "${_sdk_bin}/libQtWebApp*.dll"    "${_sdk_bin}/QtWebApp*.dll")
    if(_core_dlls)
        add_custom_command(TARGET modbus-server POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                ${_core_dlls} "$<TARGET_FILE_DIR:modbus-server>"
            COMMENT "Copy Core SDK DLLs")
    endif()

    # 2) Qt DLL через windeployqt.
    #    Ищем qmake через Qt${QT_VERSION_MAJOR}::qmake (Qt5 и Qt6),
    #    fallback: qmake из PATH.
    set(_qmake_exe "")
    if(TARGET Qt${QT_VERSION_MAJOR}::qmake)
        get_target_property(_qmake_exe Qt${QT_VERSION_MAJOR}::qmake IMPORTED_LOCATION)
    endif()
    if(NOT _qmake_exe)
        find_program(_qmake_exe qmake)
    endif()

    set(WINDEPLOYQT_EXECUTABLE "")
    if(_qmake_exe)
        get_filename_component(_qt_bin "${_qmake_exe}" DIRECTORY)
        find_program(WINDEPLOYQT_EXECUTABLE windeployqt
                     HINTS "${_qt_bin}" NO_DEFAULT_PATH)
    endif()
    if(NOT WINDEPLOYQT_EXECUTABLE)
        find_program(WINDEPLOYQT_EXECUTABLE windeployqt)
    endif()

    if(WINDEPLOYQT_EXECUTABLE)
        add_custom_command(TARGET modbus-server POST_BUILD
            COMMAND "${WINDEPLOYQT_EXECUTABLE}"
                --no-translations --no-system-d3d-compiler
                --no-opengl-sw --no-quick-import
                "$<TARGET_FILE:modbus-server>"
            COMMENT "windeployqt: copying Qt${QT_VERSION_MAJOR} DLLs")
    else()
        message(WARNING
            "windeployqt не найден (ни рядом с qmake, ни в PATH). "
            "Скопируйте Qt${QT_VERSION_MAJOR} DLL и плагины вручную.")
    endif()

    # 3) mingw runtime
    get_filename_component(_cxx_dir "${CMAKE_CXX_COMPILER}" DIRECTORY)
    foreach(_rt libstdc++-6.dll libgcc_s_seh-1.dll libgcc_s_dw2-1.dll libwinpthread-1.dll)
        if(EXISTS "${_cxx_dir}/${_rt}")
            add_custom_command(TARGET modbus-server POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "${_cxx_dir}/${_rt}" "$<TARGET_FILE_DIR:modbus-server>"
                COMMENT "Copy mingw runtime ${_rt}")
        endif()
    endforeach()
endif()
