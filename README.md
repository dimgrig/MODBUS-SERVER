# MODBUS-SERVER

Modbus RTU/TCP server for Qt 5.15.

## Requirements

- Qt 5.15.x — Core, Gui, Widgets, Network, PrintSupport, SerialPort, SerialBus, Svg
- C++17 compiler
- CMake ≥ 3.18 or qmake (Qt 5.15)
- **Win32**: MinGW-w64 i686 (`mingw81_32` as shipped with Qt 5.15.2). MSVC is not supported.
- **Linux**: x86_64, g++ ≥ 8

> Windows: `modbus-server.exe` and Core SDK must be built with the **same** MinGW and the **same** Qt. Mixing produces `entry point not found in libcore-*.dll`.

## Build (Linux)

```bash
git clone https://github.com/dimgrig/MODBUS-SERVER.git
cd MODBUS-SERVER
./scripts/build-linux.sh --clean --smoke
```

## Build (Windows)

```cmd
git clone https://github.com/dimgrig/MODBUS-SERVER.git
cd MODBUS-SERVER
scripts\build-win.bat --clean --smoke
```

## Core SDK

### Auto-fetch (CMake)

```bash
cmake -B build -DCORE_SDK_FETCH=ON
cmake --build build -j
```

Downloads `Core-Releases-<ver>-<plat>-qt5.<ext>` from `https://github.com/dimgrig/Core/releases`, extracts to `build/_deps/`.

### Local SDK

If SDK is installed at `%LOCALAPPDATA%\Core\latest` (Windows) or `~/.local/core/latest` (Linux):

```bash
cmake -B build
cmake --build build -j
```

Override via env: `CORE_SDK_ROOT=<path>`.

### Explicit prefix

```cmd
cmake -B build -G "MinGW Makefiles" ^
    -DCMAKE_PREFIX_PATH="C:/Qt/5.15.2/mingw81_32;C:/Users/<user>/AppData/Local/Core/1.1.0" ^
    -DCMAKE_BUILD_TYPE=Release
```

## qmake

qmake requires an installed SDK (env `CORE_SDK_ROOT` or `<base>/latest`):

```bash
./scripts/fetch-core-sdk.sh 1.1.0 $HOME/.local/core/1.1.0
export CORE_SDK_ROOT=$HOME/.local/core/latest
mkdir build && cd build
qmake .. && make -j
```

## Configuration: JSON and CSV

Two formats supported. Both describe the same Modbus register map.

### JSON

```json
{
  "mdum": [
    {
      "RegisterName": "hr_setpoint",
      "RegisterType": "HoldingRegister",
      "Address": 0,
      "DataLength": 1,
      "RegisterDataType": "WORD",
      "Access": "ReadWrite",
      "Scale": 0.1
    },
    {
      "RegisterName": "ir_temperature",
      "RegisterType": "InputRegister",
      "Address": 10,
      "DataLength": 2,
      "RegisterDataType": "REAL",
      "Access": "ReadOnly"
    },
    {
      "RegisterName": "coil_start",
      "RegisterType": "Coil",
      "Address": 0,
      "DataLength": 1,
      "RegisterDataType": "BOOL",
      "Access": "ReadWrite"
    }
  ]
}
```

Fields per entry:

| Field | Values | Notes |
|---|---|---|
| `RegisterName` | string | display name, unique |
| `RegisterType` | `DiscreteInput`, `Coil`, `InputRegister`, `HoldingRegister` | |
| `Address` | 0..65535 | Modbus register address |
| `DataLength` | 1..N | register count (see type table below) |
| `RegisterDataType` | `BOOL`, `WORD`, `INT`, `DWORD`, `DINT`, `LWORD`, `LINT`, `REAL`, `LREAL`, `STRING` | |
| `Access` | `ReadOnly`, `ReadWrite` | |
| `Scale` | number `0.1` or string `"0;100;0;1"` | Factor or linear range (see below) |
| `TransactionGroup` | string | optional; registers in one group are written atomically |

### CSV

Header row (exact column names, order free, `;` separator, all fields in quotes):

```
"Name";"Region";"Address";"DataTypeDevice";"DataTypeScada";"Access";"TypeString";"LengthString";"Scale";"Comment"
```

Example:

```
"Name";"Region";"Address";"DataTypeDevice";"DataTypeScada";"Access";"TypeString";"LengthString";"Scale";"Comment"
"hr_setpoint";"HOLDING_REGISTERS";"0";"WORD";"WORD";"ReadWrite";"";"";"0.1";"Setpoint"
"ir_temperature";"INPUT_REGISTERS";"10";"REAL";"REAL";"ReadOnly";"";"";"";"Temperature"
"coil_start";"COILS";"0";"BOOL";"BOOL";"ReadWrite";"";"";"";"Start"
"input_serial";"INPUT_REGISTERS";"20";"STRING";"STRING";"ReadOnly";"ASCII";"16";"";"Serial number"
```

Column meaning:

| Column | Values |
|---|---|
| `Name` | display name (unique) |
| `Region` | `COILS`, `DISCRETE_INPUTS`, `INPUT_REGISTERS`, `HOLDING_REGISTERS` |
| `Address` | 0..65535 |
| `DataTypeDevice` | `BOOL`, `WORD`, `INT`, `DWORD`, `DINT`, `LWORD`, `LINT`, `REAL`, `LREAL`, `STRING` |
| `DataTypeScada` | informational (same as `DataTypeDevice`, used by external SCADA) |
| `Access` | `ReadOnly`, `ReadWrite` |
| `TypeString` | for `STRING`: `ASCII`, `UTF-8` |
| `LengthString` | for `STRING`: length in bytes |
| `Scale` | `0.1` (factor) or `0;100;0;1` (range) or empty |
| `Comment` | free text |

### Data types → register count

| `DataTypeDevice` | `RegisterDataType` | Registers |
|---|---|---|
| `BOOL` | `bool` | 1 |
| `WORD` | `ushort` | 1 |
| `INT` | `short` | 1 |
| `DWORD` | `uint` | 2 |
| `DINT` | `int` | 2 |
| `LWORD` | `ulong` | 4 |
| `LINT` | `long` | 4 |
| `REAL` | `float` | 2 |
| `LREAL` | `double` | 4 |
| `STRING` | `string` | `ceil(LengthString / 2)` |

### Scale

| Value | Meaning |
|---|---|
| `0.1` | Raw value multiplied by 0.1 on read, divided on write |
| `0;100;0;1` | Linear map: raw 0..100 → value 0..1 |
| (empty) | No scaling |

### Load in app

`Settings → Configuration file: …` — select `.json` or `.csv`. Last used path is remembered in `config.ini` (`modbus-server/LAST_CONFIG`).

## Variables

| Name | Meaning | Default |
|---|---|---|
| `-DCORE_SDK_FETCH=ON` | Download SDK from GitHub at configure | OFF |
| `CORE_SDK_VERSION` | SDK version | `1.1.0` |
| `CORE_SDK_REPO` | Release repo | `dimgrig/Core` |
| `CORE_SDK_ROOT` (env) | SDK path | — |
| `-DMODBUS_SERVER_DEPLOY=ON` | Copy runtime DLLs (Win) | ON |

## Smoke test

```bash
QT_QPA_PLATFORM=offscreen MODBUS_SMOKE=1 ./build/modbus-server
echo $?   # 0 = ok
```

## License

MIT. See `LICENSE`.
