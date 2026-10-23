# MODBUS-SERVER

Клиент Modbus RTU/TCP на Qt 5.15.

## Требования

- **Qt 5.15.x**: Core, Gui, Widgets, Network, PrintSupport, SerialPort,
  SerialBus, Svg (Mqtt — только если нужен `core-io-lib`)
- C++17 компилятор
- CMake ≥ 3.18 **или** qmake (Qt 5.15)
- **Win32**: MinGW-w64 **i686** (тот, что идёт с Qt 5.15.2 `mingw81_32`).
  MSVC не поддерживается.
- **Linux**: x86_64, g++ ≥ 8

> ⚠️ **Windows**: `modbus-server.exe` и Core SDK **обязаны** быть собраны
> **одним и тем же mingw** и **одной и той же Qt**. Смешивание даёт
> `entry point not found in libcore-*.dll` в рантайме.

## Быстрый старт (Linux)

```bash
git clone https://github.com/dimgrig/MODBUS-SERVER.git
cd MODBUS-SERVER
./scripts/build-linux.sh --clean --smoke
```

`build-linux.sh` скачает Core SDK с GitHub releases, соберёт проект
и прогонит smoke-тест в `offscreen`-режиме.

## Быстрый старт (Windows, native)

Сначала соберите Core SDK **нативно** (тем же mingw + Qt):

```cmd
git clone --branch v1.1.0 https://github.com/dimgrig/Core.git Core-native
cd Core-native
cmake -B build -G "MinGW Makefiles" ^
    -DCMAKE_PREFIX_PATH=C:/Qt/5.15.2/mingw81_32 ^
    -DCMAKE_INSTALL_PREFIX=%LOCALAPPDATA%/Core/1.1.0 ^
    -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=OFF
cmake --build build -j
cmake --install build
```

Затем — сам клиент:

```cmd
git clone https://github.com/dimgrig/MODBUS-SERVER.git
cd MODBUS-SERVER
cmake -B build -G "MinGW Makefiles" ^
    -DCMAKE_PREFIX_PATH="C:/Qt/5.15.2/mingw81_32;%LOCALAPPDATA%/Core/1.1.0" ^
    -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
build\modbus-server.exe
```

**Про Qt5Mqtt на Windows.** `core-io-lib` требует `Qt5::Mqtt`. Если
`C:\Qt\5.15.2\mingw81_32\lib\cmake\Qt5Mqtt\` есть, а `bin\Qt5Mqtt.dll` — нет,
сборка Core упадёт с `_qt5_Mqtt_check_file_exists`. Починить:

- либо `Qt Maintenance Tool` → Add/Remove → Qt 5.15.2 → MQTT,
- либо скопировать `Qt5Mqtt.dll` от Qt 5.15.2 (не 5.15.17!) в `bin\`,
- либо переименовать `lib/cmake/Qt5Mqtt` → `Qt5Mqtt.disabled` и собирать
  Core без `core-io-lib`.

## Способы получить Core SDK

### 1. Auto-fetch через CMake

```bash
cmake -B build -DCORE_SDK_FETCH=ON
cmake --build build -j
```

Кэшируется в `build/_deps/`. Переопределить версию:
`-DCORE_SDK_VERSION=1.1.0 -DCORE_SDK_REPO=dimgrig/Core`.

> ⚠️ binary SDK под win32 собирается в Docker mingw 15.1 + Qt 5.15.17 —
> **несовместим** с нативным Qt 5.15.2 `mingw81_32`. Для Windows
> предпочтительна native-сборка (§«Быстрый старт Windows») или Linux
> `build-linux.sh`.

### 2. Локальный SDK через env

```bash
export CORE_SDK_ROOT=$HOME/.local/core/1.1.0     # Linux
set    CORE_SDK_ROOT=%LOCALAPPDATA%\Core\1.1.0    # Windows cmd
```

CMake найдёт его автоматически. Скачать вручную:

```bash
./scripts/fetch-core-sdk.sh 1.1.0 $HOME/.local/core/1.1.0
scripts\fetch-core-sdk.bat 1.1.0 %LOCALAPPDATA%\Core\1.1.0
```

### 3. Явный `CMAKE_PREFIX_PATH`

Самый надёжный и «в лоб»:

```bash
cmake -B build -G "MinGW Makefiles" \
    -DCMAKE_PREFIX_PATH="C:/Qt/5.15.2/mingw81_32;C:/Users/<user>/AppData/Local/Core/1.1.0" \
    -DCMAKE_BUILD_TYPE=Release
```

## Windows: DLL рядом с exe

POST_BUILD автоматически копирует в `build/`:
- `libcore-*.dll`, `libqcustomplot*.dll`, `libQtWebApp*.dll` — из SDK
- Qt5 DLL + плагины через `windeployqt`
- mingw runtime (`libstdc++-6.dll`, `libgcc_s_*.dll`, `libwinpthread-1.dll`)

Отключить: `-DMODBUS_SERVER_DEPLOY=OFF`.

## qmake

qmake **не** поддерживает автоскачивание. Требуется уже установленный
`CORE_SDK_ROOT`:

```bash
./scripts/fetch-core-sdk.sh 1.1.0 $HOME/.local/core/1.1.0
export CORE_SDK_ROOT=$HOME/.local/core/1.1.0
mkdir build && cd build
qmake .. && make -j
```

В QtCreator: `Projects → Build → Environment → CORE_SDK_ROOT=…`.

## Переменные / опции

| Имя | Смысл | По умолчанию |
|---|---|---|
| `-DCORE_SDK_FETCH=ON` | скачать SDK при configure | OFF |
| `CORE_SDK_VERSION` | версия SDK | `1.1.0` |
| `CORE_SDK_REPO` | репо релизов | `dimgrig/Core` |
| `CORE_SDK_PLATFORM` | override платформы | auto |
| `-DMODBUS_SERVER_DEPLOY=ON` | копировать DLL (Win) | ON |
| `CORE_SDK_ROOT` (env) | путь к SDK | — |

## Smoke-тест

```bash
QT_QPA_PLATFORM=offscreen MODBUS_SMOKE=1 ./build/modbus-server
echo $?   # 0 = ok
```

## Лицензия

MIT. См. `LICENSE`.
