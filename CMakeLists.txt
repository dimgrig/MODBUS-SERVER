cmake_minimum_required(VERSION 3.18)
project(modbus-server VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC_SEARCH_PATHS "${CMAKE_CURRENT_SOURCE_DIR}/uis")

# ── Опции ───────────────────────────────────────────────────────
option(CORE_SDK_FETCH       "Download Core SDK from GitHub releases" OFF)
option(MODBUS_SERVER_DEPLOY "Copy runtime DLLs next to exe (Windows only)" ON)

set(CORE_SDK_VERSION "1.1.0"        CACHE STRING "Core SDK version to fetch")
set(CORE_SDK_REPO    "dimgrig/Core" CACHE STRING "GitHub repo with Core releases")

# ── Резолвинг Core SDK ──────────────────────────────────────────
if(CORE_SDK_FETCH)
    include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/FetchCoreSDK.cmake)
elseif(DEFINED ENV{CORE_SDK_ROOT} AND NOT DEFINED Core_DIR)
    list(PREPEND CMAKE_PREFIX_PATH "$ENV{CORE_SDK_ROOT}")
    message(STATUS "modbus-server: Core SDK from env CORE_SDK_ROOT=$ENV{CORE_SDK_ROOT}")
elseif(NOT DEFINED Core_DIR)
    if(WIN32)
        set(_core_base "$ENV{LOCALAPPDATA}/Core")
    else()
        set(_core_base "$ENV{HOME}/.local/core")
    endif()

    foreach(_cand "${_core_base}/${CORE_SDK_VERSION}" "${_core_base}/latest")
        if(EXISTS "${_cand}/lib/cmake/Core/CoreConfig.cmake")
            list(APPEND CMAKE_PREFIX_PATH "${_cand}")
            message(STATUS "modbus-server: fallback Core SDK at ${_cand}")
            break()
        endif()
    endforeach()
    unset(_core_base)
    unset(_cand)
endif()

# find_dependency(Qt5 …) уже внутри CoreConfig.cmake — отдельный
# find_package(Qt5) не нужен (но и не помешает).
find_package(Core CONFIG REQUIRED)

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

target_include_directories(modbus-server PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(modbus-server PRIVATE
    Core::core-lib
    Core::core-journal-lib
    Core::core-modbus-lib
    Core::core-widgets-lib)

# ── RPATH (Linux: исполняемый файл сам находит <sdk>/lib) ───────
set_target_properties(modbus-server PROPERTIES
    BUILD_RPATH   "$<TARGET_FILE_DIR:Core::core-widgets-lib>"
    INSTALL_RPATH "$ORIGIN;$ORIGIN/../lib")

# ── Windows: самодостаточный набор DLL ──────────────────────────
if(WIN32 AND MODBUS_SERVER_DEPLOY)
    # 1) Core DLL из $SDK_ROOT/bin (libcore-*.dll, libqcustomplot*.dll, libQtWebApp*.dll)
    get_target_property(_core_widgets_dll Core::core-widgets-lib IMPORTED_LOCATION_RELEASE)
    if(NOT _core_widgets_dll)
        get_target_property(_core_widgets_dll Core::core-widgets-lib IMPORTED_LOCATION)
    endif()
    get_filename_component(_sdk_bin "${_core_widgets_dll}" DIRECTORY)

    file(GLOB _core_dlls "${_sdk_bin}/libcore-*.dll" "${_sdk_bin}/libqcustomplot*.dll" "${_sdk_bin}/libQtWebApp*.dll")
    if(_core_dlls)
        add_custom_command(TARGET modbus-server POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                ${_core_dlls} "$<TARGET_FILE_DIR:modbus-server>"
            COMMENT "Copy Core SDK DLLs")
    endif()

    # 2) Qt5 DLL + плагины через windeployqt
    get_target_property(_qmake_exe Qt5::qmake IMPORTED_LOCATION)
    if(_qmake_exe)
        get_filename_component(_qt_bin "${_qmake_exe}" DIRECTORY)
        find_program(WINDEPLOYQT_EXECUTABLE windeployqt
                     HINTS "${_qt_bin}" NO_DEFAULT_PATH)
        if(WINDEPLOYQT_EXECUTABLE)
            add_custom_command(TARGET modbus-server POST_BUILD
                COMMAND "${WINDEPLOYQT_EXECUTABLE}"
                    --no-translations --no-system-d3d-compiler
                    --no-opengl-sw --no-quick-import
                    "$<TARGET_FILE:modbus-server>"
                COMMENT "windeployqt: copying Qt5 DLLs")
        else()
            message(WARNING
                "windeployqt не найден рядом с ${_qt_bin}. "
                "Скопируйте Qt5 DLL и плагины вручную "
                "или передайте -DWINDEPLOYQT_EXECUTABLE=…")
        endif()
    endif()

    # 3) mingw runtime (libstdc++-6.dll, libgcc_s_*-1.dll, libwinpthread-1.dll)
    get_filename_component(_cxx_dir "${CMAKE_CXX_COMPILER}" DIRECTORY)
    foreach(_rt libstdc++-6.dll libgcc_s_seh-1.dll libgcc_s_dw2-1.dll libwinpthread-1.dll)
        if(EXISTS "${_cxx_dir}/${_rt}")
            add_custom_command(TARGET modbus-server POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    ${_core_dlls} "$<TARGET_FILE_DIR:modbus-server>"
                COMMENT "Copy Core SDK DLLs")
        endif()
    endforeach()
endif()
