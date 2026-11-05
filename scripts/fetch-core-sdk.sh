#!/usr/bin/env bash
# ============================================================================
# fetch-core-sdk.sh — скачать и распаковать Core SDK.
#
# Интерфейс совместим с Core/Saks Dockerfile.core:
#   fetch-core-sdk.sh [VERSION] [URL] [DEST] [QT_MAJOR]
#
#   VERSION   — версия Core SDK (default: $CORE_SDK_VERSION или 1.1.0)
#   URL       — прямой URL архива ("" → собирается из REPO/VERSION/PLATFORM/QT)
#   DEST      — куда распаковать ("" → $HOME/.local/core/qt${QT_MAJOR}/${VERSION})
#   QT_MAJOR  — 5 или 6 (default: $QT_VERSION_MAJOR или 5)
#
# Env:
#   CORE_SDK_REPO      default: dimgrig/Core
#   CORE_SDK_PLATFORM  default: auto по uname (linux-amd64 / linux-arm64 / win64)
#   CORE_SDK_VERSION   default: 1.1.0 (перебивается $1)
#   QT_VERSION_MAJOR   default: 5     (перебивается $4)
# ============================================================================
set -euo pipefail

VERSION="${1:-${CORE_SDK_VERSION:-1.1.0}}"
URL_ARG="${2:-}"
DEST_ARG="${3:-}"
QT_MAJOR="${4:-${QT_VERSION_MAJOR:-5}}"

REPO="${CORE_SDK_REPO:-dimgrig/Core}"
PLATFORM="${CORE_SDK_PLATFORM:-}"

# ── auto-detect platform ────────────────────────────────────────────────────
if [ -z "$PLATFORM" ]; then
    ARCH="$(uname -m 2>/dev/null || echo unknown)"
    case "$ARCH" in
        x86_64|amd64)   PLATFORM="linux-amd64" ;;
        aarch64|arm64)  PLATFORM="linux-arm64" ;;
        *)
            # git-bash / msys на Windows отдаёт x86_64, но OSTYPE=msys
            case "${OSTYPE:-}" in
                msys*|cygwin*|win32*) PLATFORM="win64" ;;
                *)                    PLATFORM="linux-amd64" ;;
            esac
            ;;
    esac
fi

ART="Core-Releases-${VERSION}-${PLATFORM}-qt${QT_MAJOR}.tar.gz"

# ── URL ─────────────────────────────────────────────────────────────────────
if [ -n "$URL_ARG" ]; then
    URL="$URL_ARG"
else
    URL="https://github.com/${REPO}/releases/download/v${VERSION}/${ART}"
fi

# ── DEST ────────────────────────────────────────────────────────────────────
if [ -z "$DEST_ARG" ]; then
    DEST="$HOME/.local/core/qt${QT_MAJOR}/${VERSION}"
else
    DEST="$DEST_ARG"
fi

case "$URL" in *.zip) EXT="zip" ;; *) EXT="tar.gz" ;; esac

# ── Идемпотентность ─────────────────────────────────────────────────────────
if [ -f "$DEST/lib/cmake/Core/CoreConfig.cmake" ]; then
    echo "Core SDK ${VERSION} (Qt${QT_MAJOR}) already at ${DEST}"
    exit 0
fi

mkdir -p "$DEST"
TMP="$(mktemp -d)"
trap 'rm -rf "$TMP"' EXIT

# ── Downloader: curl → wget ─────────────────────────────────────────────────
_download() {
    local url="$1" out="$2"
    if command -v curl >/dev/null 2>&1; then
        curl -fL --retry 3 --retry-delay 2 --connect-timeout 30 -o "$out" "$url"
    elif command -v wget >/dev/null 2>&1; then
        wget -q --tries=3 --timeout=60 -O "$out" "$url"
    else
        echo "ERROR: neither curl nor wget available" >&2
        return 1
    fi
}

echo "Downloading ${URL}"
_download "$URL" "$TMP/$ART"

# ── SHA256 (опционально) ────────────────────────────────────────────────────
SHA_URL="https://github.com/${REPO}/releases/download/v${VERSION}/SHA256SUMS"
if _download "$SHA_URL" "$TMP/SHA256SUMS" 2>/dev/null; then
    EXPECTED="$(awk -v f="$ART" '$2 == f || $2 == "*"f {print $1; exit}' "$TMP/SHA256SUMS")"
    if [ -n "${EXPECTED:-}" ]; then
        ACTUAL="$(sha256sum "$TMP/$ART" | awk '{print $1}')"
        if [ "$ACTUAL" != "$EXPECTED" ]; then
            echo "ERROR: SHA256 mismatch for ${ART}" >&2
            echo "  expected: ${EXPECTED}" >&2
            echo "  actual:   ${ACTUAL}" >&2
            exit 1
        fi
        echo "SHA256 OK"
    fi
fi

# ── Extract ─────────────────────────────────────────────────────────────────
case "$EXT" in
    zip)
        if command -v unzip >/dev/null 2>&1; then
            unzip -q "$TMP/$ART" -d "$DEST"
        elif command -v 7z >/dev/null 2>&1; then
            7z x -y -o"$DEST" "$TMP/$ART" >/dev/null
        else
            echo "ERROR: unzip nor 7z available for .zip" >&2
            exit 1
        fi
        ;;
    *)
        tar -xzf "$TMP/$ART" -C "$DEST"
        ;;
esac

# ── Flatten (если архив распаковался в подкаталог) ──────────────────────────
if [ ! -f "$DEST/lib/cmake/Core/CoreConfig.cmake" ]; then
    SUB="$(find "$DEST" -maxdepth 3 -name CoreConfig.cmake -path '*/cmake/Core/*' | head -1 || true)"
    if [ -n "${SUB:-}" ]; then
        SUBROOT="$(dirname "$(dirname "$(dirname "$SUB")")")"
        if [ "$SUBROOT" != "$DEST" ]; then
            echo "Flattening extracted dir ${SUBROOT} → ${DEST}"
            shopt -s dotglob
            mv "$SUBROOT"/* "$DEST"/ 2>/dev/null || true
            rmdir "$SUBROOT" 2>/dev/null || true
            shopt -u dotglob
        fi
    fi
fi

if [ ! -f "$DEST/lib/cmake/Core/CoreConfig.cmake" ]; then
    echo "ERROR: CoreConfig.cmake not found in ${DEST}" >&2
    exit 1
fi

# ── Symlink latest (только если DEST авто) ─────────────────────────────────
if [ -z "$DEST_ARG" ]; then
    PARENT="$(dirname "$DEST")"
    ln -sfn "$(basename "$DEST")" "${PARENT}/latest"
    echo "Symlink: ${PARENT}/latest → ${DEST}"
fi

echo "Core SDK ${VERSION} (Qt${QT_MAJOR}) installed to ${DEST}"