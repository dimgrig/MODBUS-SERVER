#!/usr/bin/env bash
# ============================================================================
# docker-export-core.sh — dev-экспорт modbus-server из Docker.
#
# PLATFORMS (пробел-разделённый), default: "linux-qt6"
#   linux-qt6 → ${OUT_ROOT}/linux-qt6/
#   linux-qt5 → ${OUT_ROOT}/linux-qt5/   (LEGACY)
#   win-qt6   → ${OUT_ROOT}/win-qt6/
#   win-qt5   → ${OUT_ROOT}/win-qt5/     (LEGACY)
#
# Env:
#   PLATFORMS          — список платформ (default: "linux-qt6")
#   DEPLOY_ROOT        — базовый каталог (default: $HOME/deploy/modbus-server)
#   CORE_SDK_VERSION   — версия Core SDK (default: 1.1.0)
#   CORE_SDK_URL       — прямой URL SDK ("" → сгенерируется)
# ============================================================================
set -euo pipefail
cd "$(dirname "$0")"

if [ ! -f VERSION ]; then
    echo "ERROR: VERSION file not found in $(pwd)" >&2
    exit 1
fi

APP_VERSION="$(tr -d '[:space:]' < VERSION)"
DEPLOY_ROOT="${DEPLOY_ROOT:-$HOME/deploy/modbus-server}"
OUT_ROOT="${DEPLOY_ROOT}/${APP_VERSION}"
CORE_SDK_VERSION="${CORE_SDK_VERSION:-1.1.0}"

export APP_VERSION
export OUT_ROOT
export CORE_SDK_VERSION

PLATFORMS="${PLATFORMS:-linux-qt6}"

platform_info() {
    case "$1" in
        linux-qt6) echo "modbus-server-linux-qt6  linux-qt6  base-deps-linux-qt6:latest" ;;
        linux-qt5) echo "modbus-server-linux-qt5  linux-qt5  base-deps-linux:latest"     ;;
        win-qt6)   echo "modbus-server-win64-qt6  win-qt6    base-deps-win64-qt6:latest" ;;
        win-qt5)   echo "modbus-server-win-qt5    win-qt5    base-deps-win:latest"       ;;
        *) return 1 ;;
    esac
}

read -ra _plats <<< "${PLATFORMS}"

SERVICES=()
OUT_DIRS=()
BASE_IMAGES=()

for p in "${_plats[@]}"; do
    if ! info="$(platform_info "$p")"; then
        echo "ERROR: неизвестная платформа '$p'" >&2
        echo "  допустимые: linux-qt6 linux-qt5 win-qt6 win-qt5" >&2
        exit 1
    fi
    read -r svc dir img <<< "$info"
    SERVICES+=("$svc")
    OUT_DIRS+=("$dir")
    BASE_IMAGES+=("$img")
done

MISSING=()
for img in "${BASE_IMAGES[@]}"; do
    docker image inspect "$img" >/dev/null 2>&1 || MISSING+=("$img")
done
if [ ${#MISSING[@]} -gt 0 ]; then
    echo "ERROR: отсутствуют base-образы:" >&2
    printf '  - %s\n' "${MISSING[@]}" >&2
    echo "Соберите: bash ../Core/docker-build-base-deps.sh" >&2
    exit 1
fi

echo "========================================"
echo "  modbus-server version : ${APP_VERSION}"
echo "  Output                : ${OUT_ROOT}"
echo "  Core SDK              : ${CORE_SDK_VERSION}"
echo "  Platforms             : ${_plats[*]}"
echo "  Services              : ${SERVICES[*]}"
echo "========================================"

for d in "${OUT_DIRS[@]}"; do
    mkdir -p "${OUT_ROOT}/${d}"
done

echo
echo "=== [1/2] Building modbus-server images ==="
docker compose -f docker-compose.export.yml build "${SERVICES[@]}"

echo
echo "=== [2/2] Exporting artifacts ==="
for svc in "${SERVICES[@]}"; do
    echo
    echo "--- ${svc} ---"
    docker compose -f docker-compose.export.yml run --rm "${svc}"
done

echo
echo "=== Updating latest symlink ==="
ln -sfn "${APP_VERSION}" "${DEPLOY_ROOT}/latest"

echo
echo "=== Done ==="
echo "Deployed to: ${DEPLOY_ROOT}"
find "${OUT_ROOT}" -maxdepth 2 -type d | sort