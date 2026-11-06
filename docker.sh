#!/usr/bin/env bash
# ============================================================================
# modbus-server/docker.sh — единый скрипт сборки/экспорта modbus-server.
#
# Команды:
#   base     — собрать base-deps (делегирует в ../Core/docker.sh base)
#   build    — docker compose build (pre-built core-builder-cmake-* из Core)
#   export   — экспорт дистрибутива через buildx (dev или ci режим)
#   test     — ctest в контейнере (--target test; linux-qt5/linux-qt6)
#   all      — base + build + export
#   mode     — показать детектированный режим (dev|ci)
#   help     — справка
#
# ВАЖНО: modbus-server НЕ собирает Core SDK. Только скачивает готовый
# tarball из GitHub releases (fetch-core-sdk.sh в Dockerfile) и бандлит
# 4 core-lib .so, с которыми линкуется исполняемый файл. Headers и
# cmake-конфиги Core в артефакт НЕ попадают — SDK публикуется отдельным
# релизом dimgrig/Core@v<CORE_SDK_VERSION>.
#
# Режимы (auto-detect, если не задан EXPORT_MODE):
#   dev  → OUT_DIR=$HOME/deploy/modbus-server/<ver>/<platform>/  + latest symlink
#   ci   → OUT_DIR=$PWD/dist/<platform>/                         + package .tar.gz/.zip
#
# Env:
#   PLATFORMS                     — пробел-разделённый список (default: linux-qt6)
#   EXPORT_MODE                   — dev | ci (auto)
#   OUT_DIR                       — override выходного каталога
#   DEPLOY_ROOT                   — override deploy root (только dev)
#   CI                            — "true" → ci в auto-detect
#   CLEAN                         — 1 → rm -rf OUT_DIR перед сборкой
#   CACHE_FROM / CACHE_TO         — buildx cache (type=gha,scope=modbus-server-…)
#   CORE_SDK_VERSION              — default: из ../Core/VERSION или 1.1.0
#   CORE_SDK_URL                  — override URL tarball (напр. локальный HTTP-сервер)
#   QT6_INSTALL_PREFIX            — для test (--target test)
#   CORE_BASE_DEPS_LINUX_QT6      — default: base-deps-linux-qt6:latest
#   CORE_BASE_DEPS_LINUX_QT5      — default: base-deps-linux:latest
#   CORE_BASE_DEPS_WIN64_QT6      — default: base-deps-win64-qt6:latest
#   CORE_BASE_DEPS_WIN_QT5        — default: base-deps-win:latest
# ============================================================================
set -euo pipefail

APP_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

usage() { sed -n '3,42p' "$0"; exit "${1:-0}"; }
die()   { echo "ERROR: $*" >&2; exit 1; }

# ── общий детектор ────────────────────────────────────────────────────────
# shellcheck source=scripts/detect-export-mode.sh
source "$APP_ROOT/scripts/detect-export-mode.sh"

# ── base-deps env (defaults: latest) ───────────────────────────────────────
_base_deps_env() {
    CORE_BASE_DEPS_LINUX_QT6="${CORE_BASE_DEPS_LINUX_QT6:-base-deps-linux-qt6:latest}"
    CORE_BASE_DEPS_LINUX_QT5="${CORE_BASE_DEPS_LINUX_QT5:-base-deps-linux:latest}"
    CORE_BASE_DEPS_WIN64_QT6="${CORE_BASE_DEPS_WIN64_QT6:-base-deps-win64-qt6:latest}"
    CORE_BASE_DEPS_WIN_QT5="${CORE_BASE_DEPS_WIN_QT5:-base-deps-win:latest}"
    INNOSETUP_IMAGE="${INNOSETUP_IMAGE:-ghcr.io/dimgrig/innosetup-base:latest}"
}

# ── Core SDK version (env → ../Core/VERSION → default) ─────────────────────
_core_sdk_version() {
    if [ -n "${CORE_SDK_VERSION:-}" ]; then
        echo "$CORE_SDK_VERSION"; return 0
    fi
    local core_ver_file="$APP_ROOT/../Core/VERSION"
    if [ -f "$core_ver_file" ]; then
        tr -d '[:space:]' < "$core_ver_file"; return 0
    fi
    echo "1.1.0"
}

# ── platform → pkg_suffix|out_subdir|dockerfile|base_image|buildx_plat|
#              cmake_cmd|qt_major|qt6_prefix|toolchain|sdk_platform|
#              mingw_triplet|mingw_pei|mingw_gcc_suffix ──
_platform_conf() {
    case "$1" in
        linux-qt6) echo "linux-qt6|linux-qt6|Dockerfile.linux|${CORE_BASE_DEPS_LINUX_QT6}|linux/amd64|cmake|6|/opt/Qt/6.8.0/gcc_64||linux-amd64|||" ;;
        linux-qt5) echo "linux-qt5|linux-qt5|Dockerfile.linux|${CORE_BASE_DEPS_LINUX_QT5}|linux/amd64|cmake|5|||linux-amd64|||" ;;
        win-qt6)   echo "win-qt6|win-qt6|Dockerfile.win|${CORE_BASE_DEPS_WIN64_QT6}|-|cmake|6|/opt/Qt/6.8.0/mingw_64|/opt/x86_64-w64-mingw32.cmake|win64|x86_64-w64-mingw32|pei-x86-64|seh-1" ;;
        win-qt5)   echo "win-qt5|win-qt5|Dockerfile.win|${CORE_BASE_DEPS_WIN_QT5}|-|i686-w64-mingw32-cmake|5|||win32|i686-w64-mingw32|pei-i386|dw2-1" ;;
        *) return 1 ;;
    esac
}

# ── cache args через `if` (не ломает set -e) ──────────────────────────────
_cache_args() {
    CACHE_ARGS=()
    if [ -n "${CACHE_FROM:-}" ]; then CACHE_ARGS+=( --cache-from "$CACHE_FROM" ); fi
    if [ -n "${CACHE_TO:-}"   ]; then CACHE_ARGS+=( --cache-to   "$CACHE_TO"   ); fi
}

# ── _cmd_build ─────────────────────────────────────────────────────────────
_cmd_build() {
    _base_deps_env
    export CORE_SDK_VERSION="$(_core_sdk_version)"
    export CORE_SDK_URL="${CORE_SDK_URL:-}"
    docker compose -f docker-compose.build.yml build "$@"
}

# ── _cmd_export ────────────────────────────────────────────────────────────
_cmd_export() {
    PLATFORMS="${PLATFORMS:-linux-qt6}"
    CLEAN="${CLEAN:-0}"

    local APP_NAME="modbus-server"
    local APP_VERSION
    APP_VERSION="$(tr -d '[:space:]' < "$APP_ROOT/VERSION")"

    detect_export_mode || exit 1

    case "$EXPORT_MODE" in
        dev) DEPLOY_ROOT="${DEPLOY_ROOT:-$HOME/deploy/modbus-server}"; OUT_DIR="${OUT_DIR:-$DEPLOY_ROOT/$APP_VERSION}" ;;
        ci)  DEPLOY_ROOT="${DEPLOY_ROOT:-}";                            OUT_DIR="${OUT_DIR:-$PWD/dist}" ;;
    esac
    echo "=== mode: ${EXPORT_MODE} (OUT_DIR=${OUT_DIR}, DEPLOY_ROOT=${DEPLOY_ROOT:-<none>}) ==="

    _base_deps_env
    local sdk_ver; sdk_ver="$(_core_sdk_version)"
    echo "=== Core SDK version: ${sdk_ver} (fetch from GitHub releases; local Core is NOT built) ==="

    if [ "$CLEAN" = "1" ]; then
        echo "=== CLEAN=1 → rm -rf $OUT_DIR ==="
        rm -rf "$OUT_DIR"
    fi
    mkdir -p "$OUT_DIR"

    echo "=== ${APP_NAME} ${APP_VERSION} → ${OUT_DIR} ==="
    echo "=== PLATFORMS: ${PLATFORMS} ==="
    read -ra _plats <<< "${PLATFORMS}"

    # pre-flight: base-image locally OR in registry
    local MISSING=() plat conf base_img
    for plat in "${_plats[@]}"; do
        conf="$(_platform_conf "$plat")" || die "неизвестная платформа '$plat' (linux-qt6 linux-qt5 win-qt6 win-qt5)"
        conf="$(echo "$conf" | sed -E 's/ *\| */|/g; s/^\| *//; s/ *\|$//')"
        base_img="$(echo "$conf" | cut -d'|' -f4)"
        [ -z "$base_img" ] && continue
        local ok=0
        if docker image inspect "$base_img" >/dev/null 2>&1; then
            ok=1
        elif docker manifest inspect "$base_img" >/dev/null 2>&1; then
            ok=1
        fi
        if [ "$ok" != "1" ]; then MISSING+=("$base_img  (platform $plat)"); fi
    done
    if [ ${#MISSING[@]} -gt 0 ]; then
        echo "ERROR: base-deps образы недоступны ни локально, ни в registry:" >&2
        printf '  - %s\n' "${MISSING[@]}" >&2
        echo >&2
        echo "В dev:   соберите — ./docker.sh base" >&2
        echo "В CI:    проверьте, что образ опубликован (workflow: 'Build base-deps images')" >&2
        echo "         и совпадает с тегом в .github/workflows/release.yml" >&2
        exit 1
    fi

    local pkg_suffix out_subdir dockerfile buildx_plat cmake_cmd qt_major \
          qt6_prefix toolchain sdk_platform fmt pkg
    for plat in "${_plats[@]}"; do
        conf="$(_platform_conf "$plat")"
        conf="$(echo "$conf" | sed -E 's/ *\| */|/g; s/^\| *//; s/ *\|$//')"
        IFS='|' read -r pkg_suffix out_subdir dockerfile base_img buildx_plat \
                        cmake_cmd qt_major qt6_prefix toolchain sdk_platform \
                        mingw_triplet mingw_pei mingw_gcc_suffix <<< "$conf"

        echo
        echo "--- ${plat} (→ ${out_subdir}/, ${dockerfile}) ---"

        # Build args: modbus-server зависит от Core как ВНЕШНЕГО артефакта.
        # CORE_SDK_URL="" → fetch-core-sdk.sh соберёт URL из версии/платформы.
        local bargs=( --build-arg "BASE_DEPS_IMAGE=${base_img}"
                      --build-arg "CMAKE_CMD=${cmake_cmd}"
                      --build-arg "QT_VERSION_MAJOR=${qt_major}"
                      --build-arg "CORE_SDK_VERSION=${sdk_ver}"
                      --build-arg "CORE_SDK_URL=${CORE_SDK_URL:-}"
                      --build-arg "CORE_SDK_PLATFORM=${sdk_platform}"
                      # Всегда передаём — иначе Qt6-дефолты из Dockerfile.win
                      # протекают в Qt5-сборку.
                      --build-arg "QT6_INSTALL_PREFIX=${qt6_prefix:-}"
                      --build-arg "CMAKE_TOOLCHAIN_FILE=${toolchain:-}" )
        if [ -n "$qt6_prefix" ]; then bargs+=( --build-arg "QT6_INSTALL_PREFIX=${qt6_prefix}" ); fi
        if [ -n "$toolchain"  ]; then bargs+=( --build-arg "CMAKE_TOOLCHAIN_FILE=${toolchain}" ); fi
        # Windows-specific
        if [[ "$plat" == win-* ]]; then
            local qt_host_path=""
            [ "$qt_major" = "6" ] && qt_host_path="/opt/Qt/6.8.0/gcc_64"
            bargs+=( --build-arg "QT_HOST_PATH=${qt_host_path}" )
            bargs+=( --build-arg "MINGW_TRIPLET=${mingw_triplet}" )
            bargs+=( --build-arg "MINGW_PEI=${mingw_pei}" )
            bargs+=( --build-arg "MINGW_GCC_SUFFIX=${mingw_gcc_suffix}" )
            bargs+=( --build-arg "INNOSETUP_IMAGE=${INNOSETUP_IMAGE}" )
        fi
        bargs+=( --target export --output "type=local,dest=${OUT_DIR}/${out_subdir}" )
        if [ "$buildx_plat" != "-" ]; then bargs+=( --platform "$buildx_plat" ); fi

        # dev: локальный SDK через http://host.docker.internal:<port>/...
        if [ -n "${CORE_SDK_URL:-}" ] && [[ "$CORE_SDK_URL" == http://host.docker.internal:* ]]; then
            bargs+=( --add-host host.docker.internal:host-gateway )
        fi

        _cache_args
        docker buildx build "${bargs[@]}" "${CACHE_ARGS[@]}" -f "${dockerfile}" "$APP_ROOT"

        case "$pkg_suffix" in win-*) fmt="zip" ;; *) fmt="tar.gz" ;; esac
        pkg="${APP_NAME}-${APP_VERSION}-${pkg_suffix}.${fmt}"

        if [ "$fmt" = "zip" ]; then
            ( cd "$OUT_DIR/$out_subdir" && zip -qr "../$pkg" . )
        else
            tar -czf "$OUT_DIR/$pkg" -C "$OUT_DIR/$out_subdir" .
        fi
        echo "  → $OUT_DIR/$pkg ($(du -h "$OUT_DIR/$pkg" | cut -f1))"
    done

    if [ -n "$DEPLOY_ROOT" ]; then
        mkdir -p "$DEPLOY_ROOT"
        ln -sfn "$OUT_DIR" "${DEPLOY_ROOT}/latest"
        echo; echo "=== latest -> ${OUT_DIR} ==="
    fi

    echo; echo "=== Done ==="
    find "$OUT_DIR" -maxdepth 1 -type f \
        \( -name "${APP_NAME}-*.tar.gz" -o -name "${APP_NAME}-*.zip" \) \
        -exec ls -lh {} \; 2>/dev/null || true
}

# ── _cmd_test — только linux-qt6/linux-qt5, --target test ─────────────────
_cmd_test() {
    PLATFORMS="${PLATFORMS:-linux-qt6}"
    read -ra _plats <<< "${PLATFORMS}"
    _base_deps_env
    local sdk_ver; sdk_ver="$(_core_sdk_version)"

    local tested=0 plat base_img qt_major dockerfile
    for plat in "${_plats[@]}"; do
        case "$plat" in
            linux-qt6) base_img="$CORE_BASE_DEPS_LINUX_QT6"; qt_major=6; dockerfile="Dockerfile.linux" ;;
            linux-qt5) base_img="$CORE_BASE_DEPS_LINUX_QT5"; qt_major=5; dockerfile="Dockerfile.linux" ;;
            *) echo "NOTE: test supports only linux-qt5/linux-qt6, skipping '$plat'"; continue ;;
        esac

        echo
        echo "--- test ${plat} (base: ${base_img}) ---"
        local targs=( --target test
                      --build-arg "BASE_DEPS_IMAGE=${base_img}"
                      --build-arg "CMAKE_CMD=cmake"
                      --build-arg "QT_VERSION_MAJOR=${qt_major}"
                      --build-arg "CORE_SDK_VERSION=${sdk_ver}" )
        if [ -n "${QT6_INSTALL_PREFIX:-}" ]; then
            targs+=( --build-arg "QT6_INSTALL_PREFIX=${QT6_INSTALL_PREFIX}" )
        fi
        if [ -n "${CORE_SDK_URL:-}" ] && [[ "$CORE_SDK_URL" == http://host.docker.internal:* ]]; then
            targs+=( --add-host host.docker.internal:host-gateway )
        fi

        _cache_args
        docker buildx build "${targs[@]}" "${CACHE_ARGS[@]}" -f "${dockerfile}" "$APP_ROOT"
        tested=$((tested + 1))
    done
    [ "$tested" -gt 0 ] || die "no linux-qt5/linux-qt6 in PLATFORMS='$PLATFORMS'"
    echo; echo "=== test OK (${tested} platform(s)) ==="
}

# ── _main ──────────────────────────────────────────────────────────────────
_main() {
    cd "$APP_ROOT"
    [ -f VERSION ] || die "VERSION file not found in $APP_ROOT"

    local cmd="${1:-}"; shift || true
    case "$cmd" in
        base)
            if [ -x "$APP_ROOT/../Core/docker.sh" ]; then
                bash "$APP_ROOT/../Core/docker.sh" base "$@"
            else
                echo "ERROR: ../Core/docker.sh not found." >&2
                echo "  expected sibling repo: $(dirname "$APP_ROOT")/Core/docker.sh" >&2
                echo "  In CI use prebuilt base-deps images (CORE_BASE_DEPS_*)." >&2
                exit 1
            fi
            ;;
        build)  _cmd_build "$@" ;;
        export) _cmd_export "$@" ;;
        test)   _cmd_test   "$@" ;;
        mode)   detect_export_mode || exit 1; echo "$EXPORT_MODE" ;;
        all)    bash "$APP_ROOT/../Core/docker.sh" base; _cmd_build; _cmd_export "$@" ;;
        help|-h|--help) usage 0 ;;
        "")     usage 1 ;;
        *)      die "неизвестная команда '$cmd' (base|build|export|test|mode|all|help)" ;;
    esac
}

if [[ "${BASH_SOURCE[0]}" == "${0}" ]]; then
    _main "$@"
fi