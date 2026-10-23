@echo off
REM scripts/build-core-native-win.bat
REM
REM Нативно собирает Core SDK на Windows (тот же mingw + та же Qt,
REM что и у modbus-client.exe). Гарантирует ABI-совместимость.
REM
REM Использование:
REM   build-core-native-win.bat [VERSION] [QT_DIR] [INSTALL_DIR]
REM
REM Дефолты:
REM   VERSION     = 1.1.0
REM   QT_DIR      = C:\Qt\5.15.2\mingw81_32
REM   INSTALL_DIR = %LOCALAPPDATA%\Core\%VERSION%

setlocal enabledelayedexpansion

set VERSION=%~1
set QT_DIR=%~2
set INSTALL_DIR=%~3

if "%VERSION%"==""     set VERSION=1.1.0
if "%QT_DIR%"==""       set QT_DIR=C:\Qt\5.15.2\mingw81_32
if "%INSTALL_DIR%"=="" set INSTALL_DIR=%LOCALAPPDATA%\Core\%VERSION%

echo === Core SDK native build ===
echo   version : %VERSION%
echo   Qt      : %QT_DIR%
echo   install : %INSTALL_DIR%
echo.

if not exist "%QT_DIR%\bin\qmake.exe" (
    echo ERROR: Qt not found at %QT_DIR%
    exit /b 1
)

REM Проверим, что mingw из Qt на месте (иначе CMake возьмёт другой)
set QMAKE_BIN=
for /f "tokens=*" %%Q in ('where qmake 2^>nul') do if not defined QMAKE_BIN set QMAKE_BIN=%%Q
if not defined QMAKE_BIN (
    echo WARN: qmake не в PATH. Убедитесь, что используется
    echo       %QT_DIR%\..\..\..\Tools\mingw810_32\bin\g++.exe
    echo       иначе ABI может не совпасть.
)

set CORE_TMP=%TEMP%\Core-%VERSION%-native
if exist "%CORE_TMP%" rmdir /s /q "%CORE_TMP%"

echo === Cloning Core v%VERSION% ===
git clone --branch v%VERSION% --depth 1 https://github.com/dimgrig/Core.git "%CORE_TMP%"
if errorlevel 1 exit /b 1

echo === Building ===
cd /d "%CORE_TMP%"
cmake -B build -G "MinGW Makefiles" ^
    -DCMAKE_PREFIX_PATH="%QT_DIR%" ^
    -DCMAKE_INSTALL_PREFIX="%INSTALL_DIR%" ^
    -DCMAKE_BUILD_TYPE=Release ^
    -DBUILD_TESTING=OFF
if errorlevel 1 exit /b 1
cmake --build build -j
if errorlevel 1 exit /b 1
cmake --install build
if errorlevel 1 exit /b 1

echo.
echo === Done ===
echo Core SDK %VERSION% installed at: %INSTALL_DIR%
echo.
echo Для сборки modbus-client:
echo   set CORE_SDK_ROOT=%INSTALL_DIR%
echo   cmake -B build -G "MinGW Makefiles" ^^
echo         -DCMAKE_PREFIX_PATH="%QT_DIR%" ^^
echo         -DCMAKE_BUILD_TYPE=Release
endlocal