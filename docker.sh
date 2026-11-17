#!/usr/bin/env bash
# modbus-server/docker.sh — thin client на .ci/scripts/docker-lib.sh (CoreCI v0.2.1+).
set -euo pipefail

APP_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
[ -f "$APP_ROOT/.ci/scripts/docker-lib.sh" ] || {
    echo "ERROR: CoreCI submodule not initialized (need v0.2.1+).
  Run: git submodule update --init --recursive
       (cd .ci && git checkout v0.2.1)" >&2
    exit 1
}

APP_NAME="modbus-server"
APP_VERSION="$(tr -d '[:space:]' < "$APP_ROOT/VERSION")"

# Defaults из env (совместимо с тем, что было раньше)
CORE_BASE_DEPS_LINUX_QT6="${CORE_BASE_DEPS_LINUX_QT6:-base-deps-linux-qt6:latest}"
CORE_BASE_DEPS_LINUX_QT5="${CORE_BASE_DEPS_LINUX_QT5:-base-deps-linux:latest}"
CORE_BASE_DEPS_WIN64_QT6="${CORE_BASE_DEPS_WIN64_QT6:-base-deps-win64-qt6:latest}"
CORE_BASE_DEPS_WIN_QT5="${CORE_BASE_DEPS_WIN_QT5:-base-deps-win:latest}"
INNOSETUP_IMAGE="${INNOSETUP_IMAGE:-ghcr.io/dimgrig/innosetup-base:latest}"

# CI передаёт PLATFORMS как env (строка). Сохраняем ДО declare -A,
# иначе declare -A затрёт env-значение.
PLATFORMS_LIST="${PLATFORMS:-${PLATFORMS_LIST:-linux-qt6}}"

# Формат (13 полей через '|'):
#   pkg_suffix|out_subdir|dockerfile|base_image|buildx_platform|
#   cmake_cmd|qt_major|qt6_prefix|toolchain|sdk_platform|
#   mingw_triplet|mingw_pei|mingw_gcc_suffix
declare -A PLATFORMS=(
    [linux-qt6]="linux-qt6|linux-qt6|Dockerfile.linux|${CORE_BASE_DEPS_LINUX_QT6}|linux/amd64|cmake|6|/opt/Qt/6.8.0/gcc_64||linux-amd64|||"
    [linux-qt5]="linux-qt5|linux-qt5|Dockerfile.linux|${CORE_BASE_DEPS_LINUX_QT5}|linux/amd64|cmake|5|||linux-amd64|||"
    [win-qt6]="win-qt6|win-qt6|Dockerfile.win|${CORE_BASE_DEPS_WIN64_QT6}||cmake|6|/opt/Qt/6.8.0/mingw_64|/opt/x86_64-w64-mingw32.cmake|win64|x86_64-w64-mingw32|pei-x86-64|seh-1"
    [win-qt5]="win-qt5|win-qt5|Dockerfile.win|${CORE_BASE_DEPS_WIN_QT5}||i686-w64-mingw32-cmake|5|||win32|i686-w64-mingw32|pei-i386|dw2-1"
)

source "$APP_ROOT/.ci/scripts/docker-lib.sh"

package_name() {
    local pkg_suffix="$1" sdk_platform="$2" qt_major="$3" fmt="$4"
    if [ "${EXPORT_MODE:-}" = "ci" ]; then
        echo "Core-Releases-${APP_VERSION}-${sdk_platform}-qt${qt_major}.${fmt}"
    else
        echo "core-${APP_VERSION}-${pkg_suffix}.${fmt}"
    fi
}

core_docker_main "$@"