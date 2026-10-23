@echo off
setlocal enabledelayedexpansion

set VERSION=%~1
set DEST=%~2
if "%VERSION%"=="" set VERSION=1.1.0
if "%DEST%"==""    set DEST=%LOCALAPPDATA%\Core\%VERSION%
if "%CORE_SDK_REPO%"=="" set CORE_SDK_REPO=dimgrig/Core

set ART=Core-Releases-%VERSION%-win32-qt5.zip
set BASE=https://github.com/%CORE_SDK_REPO%/releases/download/v%VERSION%
set URL=%BASE%/%ART%
set SHA_URL=%BASE%/SHA256SUMS

if exist "%DEST%\lib\cmake\Core\CoreConfig.cmake" (
    echo Core SDK %VERSION% already at %DEST%
    exit /b 0
)

if not exist "%DEST%" mkdir "%DEST%"
set TMP=%TEMP%\core-sdk-%RANDOM%%RANDOM%
mkdir "%TMP%"

echo Downloading %URL%
powershell -NoProfile -Command ^
  "Invoke-WebRequest -UseBasicParsing -Uri '%URL%' -OutFile '%TMP%\%ART%'"
if errorlevel 1 (
    echo ERROR: Download failed
    rmdir /s /q "%TMP%"
    exit /b 1
)

echo Downloading SHA256SUMS
powershell -NoProfile -Command ^
  "try { Invoke-WebRequest -UseBasicParsing -Uri '%SHA_URL%' -OutFile '%TMP%\SHA256SUMS' } catch { Write-Host 'WARN: SHA256SUMS not available' }"

if exist "%TMP%\SHA256SUMS" (
    set "FOUND="
    for /f "tokens=1,2" %%A in ('findstr /i /c:"%ART%" "%TMP%\SHA256SUMS"') do (
        set "EXPECTED=%%A"
        set "FOUND=1"
    )
    if defined FOUND (
        for /f "tokens=*" %%H in ('powershell -NoProfile -Command "(Get-FileHash -Algorithm SHA256 '%TMP%\%ART%'^).Hash.ToLower(^)"') do set "ACTUAL=%%H"
        if /i not "!ACTUAL!"=="!EXPECTED!" (
            echo ERROR: SHA256 mismatch for %ART%
            echo   expected: !EXPECTED!
            echo   actual:   !ACTUAL!
            rmdir /s /q "%TMP%"
            exit /b 1
        )
        echo SHA256 OK
    )
)

echo Extracting
powershell -NoProfile -Command ^
  "Expand-Archive -Force '%TMP%\%ART%' '%DEST%'"
if errorlevel 1 (
    echo ERROR: Extraction failed
    rmdir /s /q "%TMP%"
    exit /b 1
)

if not exist "%DEST%\lib\cmake\Core\CoreConfig.cmake" (
    echo ERROR: CoreConfig.cmake not found in %DEST%
    rmdir /s /q "%TMP%"
    exit /b 1
)

rmdir /s /q "%TMP%"
echo Core SDK %VERSION% installed to %DEST%

REM Junction "latest" — для паритета с Linux (symlink).
REM mklink /J работает без elevation (для директорий).
for %%P in ("%DEST%\..") do set CORE_ROOT=%%~fP
if /i not "%DEST%"=="%CORE_ROOT%\latest" (
    if exist "%CORE_ROOT%\latest" (
        rmdir "%CORE_ROOT%\latest" >nul 2>&1
    )
    mklink /J "%CORE_ROOT%\latest" "%DEST%" >nul 2>&1
    if errorlevel 1 (
        echo WARN: could not create junction %CORE_ROOT%\latest
    ) else (
        echo Junction: %CORE_ROOT%\latest -^> %DEST%
    )
)
endlocal
