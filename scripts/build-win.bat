@echo off
REM scripts/build-win.bat
REM
REM Windows-эквивалент build-linux.sh:
REM   1. Проверяет наличие Core SDK
REM   2. Если нет — собирает Core нативно (build-core-native-win.bat)
REM   3. Собирает modbus-client (или modbus-server)
REM   4. (опционально) smoke-тест
REM
REM Использование:
REM   scripts\build-win.bat                    -- debug быстрый цикл
REM   scripts\build-win.bat --clean --smoke    -- как CI
REM   scripts\build-win.bat --qt C:\Qt\5.15.2\mingw81_32
REM
setlocal enabledelayedexpansion

REM ── Параметры ─────────────────────────────────────────────
set APP_NAME=modbus-client
if exist "%CD%\modbus-server.pro" set APP_NAME=modbus-server

set VERSION=1.1.0
set QT_DIR=C:\Qt\5.15.2\mingw81_32
set BUILD_TYPE=Release
set BUILD_DIR=build
set DO_CLEAN=0
set DO_SMOKE=0

:parse_args
if "%~1"=="" goto after_args
if /i "%~1"=="--clean"      (set DO_CLEAN=1  & shift & goto parse_args)
if /i "%~1"=="--smoke"      (set DO_SMOKE=1  & shift & goto parse_args)
if /i "%~1"=="--version"    (set VERSION=%~2 & shift & shift & goto parse_args)
if /i "%~1"=="--qt"         (set QT_DIR=%~2  & shift & shift & goto parse_args)
if /i "%~1"=="--build-type" (set BUILD_TYPE=%~2 & shift & shift & goto parse_args)
if /i "%~1"=="-h"           goto print_help
if /i "%~1"=="--help"       goto print_help
echo Unknown argument: %~1 & exit /b 1

:print_help
echo Usage: scripts\build-win.bat [options]
echo   --clean                  -- rmdir build first
echo   --smoke                  -- run smoke test after build
echo   --version X.Y.Z          -- Core SDK version (default 1.1.0)
echo   --qt PATH                -- Qt mingw dir (default C:\Qt\5.15.2\mingw81_32)
echo   --build-type TYPE        -- Release^|Debug (default Release)
exit /b 0

:after_args
cd /d "%~dp0.."
echo == %APP_NAME% Windows build ==
echo   source    : %CD%
echo   Qt        : %QT_DIR%
echo   version   : %VERSION%
echo   build type: %BUILD_TYPE%
echo.

REM ── 1. Проверка Qt ────────────────────────────────────────
if not exist "%QT_DIR%\bin\qmake.exe" (
    echo ERROR: Qt not found at %QT_DIR%
    echo        Install Qt 5.15.2 mingw81_32 or pass --qt PATH
    exit /b 1
)

REM ── 2. Проверка/сборка Core SDK ───────────────────────────
set CORE_SDK_ROOT=%LOCALAPPDATA%\Core\%VERSION%
if exist "%CORE_SDK_ROOT%\lib\cmake\Core\CoreConfig.cmake" (
    echo === Core SDK found: %CORE_SDK_ROOT% ===
) else (
    echo === Core SDK not found, building natively ===
    call "%~dp0build-core-native-win.bat" %VERSION% "%QT_DIR%" "%CORE_SDK_ROOT%"
    if errorlevel 1 (
        echo ERROR: failed to build Core SDK
        exit /b 1
    )
)

REM ── 3. Clean ──────────────────────────────────────────────
if "%DO_CLEAN%"=="1" (
    if exist "%BUILD_DIR%" (
        echo === rm -rf %BUILD_DIR% ===
        rmdir /s /q "%BUILD_DIR%"
    )
)

REM ── 4. Configure ──────────────────────────────────────────
echo === Configure ===
cmake -B "%BUILD_DIR%" -G "MinGW Makefiles" ^
    -DCMAKE_PREFIX_PATH="%QT_DIR%;%CORE_SDK_ROOT%" ^
    -DCMAKE_BUILD_TYPE=%BUILD_TYPE%
if errorlevel 1 exit /b 1

REM ── 5. Build ──────────────────────────────────────────────
echo === Build ===
cmake --build "%BUILD_DIR%" -j
if errorlevel 1 exit /b 1

REM ── 6. Smoke ──────────────────────────────────────────────
if "%DO_SMOKE%"=="1" (
    echo === Smoke ===
    set MODBUS_SMOKE=1
    "%BUILD_DIR%\%APP_NAME%.exe"
    set RC=!errorlevel!
    set MODBUS_SMOKE=
    if !RC! neq 0 (
        echo ERROR: smoke failed (exit=!RC!)
        exit /b !RC!
    )
    echo ✓ smoke OK
)

echo.
echo == Done ==
echo   exe: %CD%\%BUILD_DIR%\%APP_NAME%.exe
echo.
endlocal
