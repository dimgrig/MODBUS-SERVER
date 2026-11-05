#!/usr/bin/env bash
# ============================================================================
# docker-build-all.sh — локальная сборка modbus-server образов.
#
# Env:
#   PLATFORMS          — список платформ (default: "linux-qt6")
#   BUILD_BASE_DEPS    — 1 → пересобрать base-deps
#   CORE_SDK_VERSION   — версия Core SDK (default: 1.1.0)
#   CORE_SDK_URL       — прямой URL SDK ("" → сгенерируется по версии/платформе)
# ============================================================================
set -euo pipefail
cd "$(dirname "$0")"

BUILD_BASE_DEPS="${BUILD_BASE_DEPS:-0}"
PLATFORMS="${PLATFORMS:-linux-qt6}"
CORE_SDK_VERSION="${CORE_SDK_VERSION:-1.1.0}"
export CORE_SDK_VERSION

read -ra PLATFORMS_V <<< "${PLATFORMS}"

command -v docker >/dev/null 2>&1 || { echo "ERROR: docker not found" >&2; exit 1; }

echo "============================================================"
echo "  modbus-server — docker build"
echo "  Platforms       : ${PLATFORMS_V[*]}"
echo "  Core SDK        : ${CORE_SDK_VERSION}"
echo "  BUILD_BASE_DEPS : ${BUILD_BASE_DEPS}"
echo "============================================================"

if [ "${BUILD_BASE_DEPS}" = "1" ]; then
    echo
    echo "=== [1/2] base-deps: rebuilding ==="
    bash ../Core/docker-build-base-deps.sh
else
    echo
    echo "=== [1/2] base-deps: use existing (BUILD_BASE_DEPS=0) ==="
    MISSING=()
    for p in "${PLATFORMS_V[@]}"; do
        case "$p" in
            linux-qt6) img="base-deps-linux-qt6:latest" ;;
            linux-qt5) img="base-deps-linux:latest" ;;
            win-qt6)   img="base-deps-win64-qt6:latest" ;;
            win-qt5)   img="base-deps-win:latest" ;;
            *) echo "ERROR: unknown platform '$p'" >&2; exit 1 ;;
        esac
        docker image inspect "$img" >/dev/null 2>&1 || MISSING+=("$img  (platform ${p})")
    done
    if [ ${#MISSING[@]} -gt 0 ]; then
        echo "ERROR: missing base images:" >&2
        printf '  - %s\n' "${MISSING[@]}" >&2
        echo "Run:  BUILD_BASE_DEPS=1 $0" >&2
        exit 1
    fi
fi

echo
echo "=== [2/2] Building modbus-server ==="
docker compose -f docker-compose.build.yml build

echo
echo "=== Done ==="
docker images | grep -E 'modbus-server|REPOSITORY'