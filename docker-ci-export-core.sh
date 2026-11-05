#!/usr/bin/env bash
# ============================================================================
# docker-ci-export-core.sh — CI-экспорт артефактов MODBUS-* через
# docker buildx --output type=local.
# НЕ для локальной разработки — используй ./docker-export-core.sh.
#
# PLATFORMS (пробел-разделённый список), по умолчанию "linux-qt6":
#   linux-qt6   linux-qt5   win-qt6   win-qt5
# (ARM-платформы в MODBUS-* не поддерживаются.)
#
# Env:
#   OUT_DIR                    — куда писать (default: $PWD/dist)
#   PLATFORMS                  — список платформ (default: "linux-qt6")
#   CORE_SDK_VERSION           — default: 1.1.0
#   CORE_SDK_URL               — "" → fetch-core-sdk.sh соберёт URL сам
#   CORE_BASE_DEPS_LINUX_QT6   — default: base-deps-linux-qt6:latest
#   CORE_BASE_DEPS_LINUX_QT5   — default: base-deps-linux:latest
#   CORE_BASE_DEPS_WIN64_QT6   — default: base-deps-win64-qt6:latest
#   CORE_BASE_DEPS_WIN_QT5     — default: base-deps-win:latest
# ============================================================================
set -euo pipefail
cd "$(dirname "$0")"

APP_NAME="${APP_NAME:-$(basename "$PWD" | tr '[:upper:]' '[:lower:]')}"
APP_VERSION="$(tr -d '[:space:]' < VERSION)"
OUT_DIR="${OUT_DIR:-$PWD/dist}"
PLATFORMS="${PLATFORMS:-linux-qt6}"

CORE_SDK_VERSION="${CORE_SDK_VERSION:-1.1.0}"
CORE_SDK_URL="${CORE_SDK_URL:-}"

CORE_BASE_DEPS_LINUX_QT6="${CORE_BASE_DEPS_LINUX_QT6:-base-deps-linux-qt6:latest}"
CORE_BASE_DEPS_LINUX_QT5="${CORE_BASE_DEPS_LINUX_QT5:-base-deps-linux:latest}"
CORE_BASE_DEPS_WIN64_QT6="${CORE_BASE_DEPS_WIN64_QT6:-base-deps-win64-qt6:latest}"
CORE_BASE_DEPS_WIN_QT5="${CORE_BASE_DEPS_WIN_QT5:-base-deps-win:latest}"

mkdir -p "$OUT_DIR"

# ── platform → pkg_suffix | out_subdir | dockerfile | base_deps_image |
#                buildx_platform | cmake_cmd | qt_major |
#                qt6_install_prefix | cmake_toolchain_file ──
platform_conf() {
    case "$1" in
        linux-qt6) echo "linux-qt6|linux-qt6|Dockerfile.linux-qt6|${CORE_BASE_DEPS_LINUX_QT6}|linux/amd64|cmake|6|/opt/Qt/6.8.0/gcc_64|" ;;
        linux-qt5) echo "linux-qt5|linux-qt5|Dockerfile.linux|${CORE_BASE_DEPS_LINUX_QT5}|linux/amd64|cmake|5||" ;;
        win-qt6)   echo "win-qt6|win-qt6|Dockerfile.win64-qt6|${CORE_BASE_DEPS_WIN64_QT6}|-|cmake|6|/opt/Qt/6.8.0/mingw_64|/opt/x86_64-w64-mingw32.cmake" ;;
        win-qt5)   echo "win-qt5|win-qt5|Dockerfile.win|${CORE_BASE_DEPS_WIN_QT5}|-|i686-w64-mingw32-cmake|5||" ;;
        *)         return 1 ;;
    esac
}

echo "=== ${APP_NAME} ${APP_VERSION} → ${OUT_DIR} ==="
echo "=== Core SDK ${CORE_SDK_VERSION} ==="
echo "=== PLATFORMS: ${PLATFORMS} ==="

read -ra _plats <<< "${PLATFORMS}"

for plat in "${_plats[@]}"; do
    if ! conf="$(platform_conf "$plat")"; then
        echo "ERROR: неизвестная платформа '$plat'" >&2
        echo "  допустимые: linux-qt6 linux-qt5 win-qt6 win-qt5" >&2
        exit 1
    fi

    conf="$(echo "$conf" | sed -E 's/ *\| */|/g; s/^\| *//; s/ *\|$//')"

    IFS='|' read -r pkg_suffix out_subdir dockerfile base_img buildx_plat \
                    cmake_cmd qt_major qt6_prefix toolchain <<< "$conf"

    echo
    echo "--- ${plat} (→ ${out_subdir}/, ${dockerfile}) ---"

    bargs=( --build-arg "BASE_DEPS_IMAGE=${base_img}"
            --build-arg "CMAKE_CMD=${cmake_cmd}"
            --build-arg "QT_VERSION_MAJOR=${qt_major}"
            --build-arg "CORE_SDK_VERSION=${CORE_SDK_VERSION}"
            --build-arg "CORE_SDK_URL=${CORE_SDK_URL}" )

    [ -n "$qt6_prefix" ] && bargs+=( --build-arg "QT6_INSTALL_PREFIX=${qt6_prefix}" )
    [ -n "$toolchain"  ] && bargs+=( --build-arg "CMAKE_TOOLCHAIN_FILE=${toolchain}" )

    bargs+=( --target export
             --output "type=local,dest=${OUT_DIR}/${out_subdir}" )

    [ "$buildx_plat" != "-" ] && bargs+=( --platform "$buildx_plat" )

    docker buildx build "${bargs[@]}" -f "${dockerfile}" .

    case "$pkg_suffix" in
        win-*) fmt="zip"    ;;
        *)     fmt="tar.gz" ;;
    esac
    pkg="${APP_NAME}-${APP_VERSION}-${pkg_suffix}.${fmt}"

    if [ "$fmt" = "zip" ]; then
        ( cd "$OUT_DIR/$out_subdir" && zip -qr "../$pkg" . )
    else
        tar -czf "$OUT_DIR/$pkg" -C "$OUT_DIR/$out_subdir" .
    fi
    echo "  → $OUT_DIR/$pkg ($(du -h "$OUT_DIR/$pkg" | cut -f1))"
done

echo
echo "=== Done ==="
find "$OUT_DIR" -maxdepth 1 -type f \
    \( -name "${APP_NAME}-*.tar.gz" -o -name "${APP_NAME}-*.zip" \) \
    -exec ls -lh {} \; 2>/dev/null || true