#!/usr/bin/env bash
# scripts/build-linux.sh — сборка MODBUS-* на Linux (или в Docker).
#
# Использование:
#   ./scripts/build-linux.sh                         # fetch SDK + build
#   ./scripts/build-linux.sh --smoke                 # + headless smoke-test
#   ./scripts/build-linux.sh --no-fetch              # использовать env CORE_SDK_ROOT
#   ./scripts/build-linux.sh --version 1.1.0         # другая версия Core SDK
#   ./scripts/build-linux.sh --build-type Debug
#   ./scripts/build-linux.sh --clean                 # rm -rf build перед сборкой
#   ./scripts/build-linux.sh --install-sdk           # установить SDK в ~/.local/core/<ver>/
#
# Переменные окружения:
#   CORE_SDK_VERSION (def 1.1.0)         — версия Core SDK
#   CORE_SDK_REPO    (def dimgrig/Core)  — репо релизов
#   CORE_SDK_ROOT    — путь к SDK, если --no-fetch
#
set -euo pipefail

# ── Значения по умолчанию ────────────────────────────────────────
VERSION="${CORE_SDK_VERSION:-1.1.0}"
REPO="${CORE_SDK_REPO:-dimgrig/Core}"
BUILD_TYPE="Release"
BUILD_DIR="build"
DO_CLEAN=0
DO_SMOKE=0
NO_FETCH=0
INSTALL_SDK=0

# ── Парсинг аргументов ──────────────────────────────────────────
while [[ $# -gt 0 ]]; do
    case "$1" in
        --clean)        DO_CLEAN=1 ;;
        --smoke)        DO_SMOKE=1 ;;
        --no-fetch)     NO_FETCH=1 ;;
        --install-sdk)  INSTALL_SDK=1 ;;
        --version)      VERSION="$2"; shift ;;
        --build-type)   BUILD_TYPE="$2"; shift ;;
        --build-dir)    BUILD_DIR="$2"; shift ;;
        -h|--help)
            sed -n '2,20p' "$0"; exit 0 ;;
        *) echo "Неизвестный аргумент: $1" >&2; exit 1 ;;
    esac
    shift
done

cd "$(dirname "$0")/.."
REPO_ROOT="$(pwd)"
echo "== MODBUS-* build =="
echo "  source : $REPO_ROOT"
echo "  sdk    : $VERSION (repo ${REPO})"
echo "  type   : $BUILD_TYPE"
echo "  build  : $BUILD_DIR"

# ── 1. Опционально — установить SDK в домашний каталог ──────────
if [[ "$INSTALL_SDK" == "1" ]]; then
    DEST="$HOME/.local/core/$VERSION"
    ./scripts/fetch-core-sdk.sh "$VERSION" "$DEST"
    mkdir -p "$(dirname "$DEST")"
    ln -sfn "$VERSION" "$(dirname "$DEST")/latest"
    export CORE_SDK_ROOT="$DEST"
    NO_FETCH=1
fi

# ── 2. Чистка ────────────────────────────────────────────────────
if [[ "$DO_CLEAN" == "1" && -d "$BUILD_DIR" ]]; then
    echo "→ rm -rf $BUILD_DIR"
    rm -rf "$BUILD_DIR"
fi

# ── 3. Поиск Qt5 ─────────────────────────────────────────────────
if ! command -v qmake >/dev/null 2>&1 && ! command -v qmake-qt5 >/dev/null 2>&1; then
    echo "ОШИБКА: qmake не найден в PATH." >&2
    echo "Установите: sudo apt install qtbase5-dev qtbase5-dev-tools qttools5-dev-tools \\" >&2
    echo "            libqt5serialport5-dev libqt5serialbus5-dev libqt5svg5-dev" >&2
    exit 1
fi

CMAKE_ARGS=(-DCMAKE_BUILD_TYPE="$BUILD_TYPE" -DCMAKE_COLOR_DIAGNOSTICS=ON)

if [[ "$NO_FETCH" == "1" ]]; then
    if [[ -z "${CORE_SDK_ROOT:-}" ]]; then
        echo "ОШИБКА: --no-fetch требует env CORE_SDK_ROOT" >&2
        exit 1
    fi
    if [[ ! -f "$CORE_SDK_ROOT/lib/cmake/Core/CoreConfig.cmake" ]]; then
        echo "ОШИБКА: не найден CoreConfig.cmake в $CORE_SDK_ROOT" >&2
        exit 1
    fi
    echo "→ использование SDK из $CORE_SDK_ROOT"
    CMAKE_ARGS+=(-DCMAKE_PREFIX_PATH="$CORE_SDK_ROOT")
else
    echo "→ fetch Core SDK $VERSION с GitHub"
    CMAKE_ARGS+=(-DCORE_SDK_FETCH=ON -DCORE_SDK_VERSION="$VERSION"
                 -DCORE_SDK_REPO="$REPO")
fi

# ── 4. Configure ─────────────────────────────────────────────────
echo "→ cmake -B $BUILD_DIR ${CMAKE_ARGS[*]}"
cmake -B "$BUILD_DIR" "${CMAKE_ARGS[@]}"

# ── 5. Build ─────────────────────────────────────────────────────
echo "→ cmake --build $BUILD_DIR -j$(nproc)"
cmake --build "$BUILD_DIR" -j"$(nproc)"

# ── 6. Smoke-test ────────────────────────────────────────────────
if [[ "$DO_SMOKE" == "1" ]]; then
    EXE="$BUILD_DIR/modbus-client"
    [[ -x "$EXE" ]] || EXE="$BUILD_DIR/modbus-server"
    if [[ ! -x "$EXE" ]]; then
        echo "ОШИБКА: executable не найден в $BUILD_DIR" >&2
        exit 1
    fi
    echo "→ smoke: $EXE"
    set +e
    QT_QPA_PLATFORM=offscreen MODBUS_SMOKE=1 "$EXE"
    RC=$?
    set -e
    if [[ $RC -ne 0 ]]; then
        echo "ОШИБКА: smoke-test провален (exit $RC)" >&2
        exit $RC
    fi
    echo "✓ smoke OK"
fi

echo
echo "== Готово =="
echo "  exe : $REPO_ROOT/$BUILD_DIR/modbus-client"
echo "  libs: $REPO_ROOT/$BUILD_DIR (Core SDK rpath \$ORIGIN подхватит сам)"
