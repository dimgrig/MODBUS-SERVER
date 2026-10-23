#!/usr/bin/env bash
set -euo pipefail

VERSION="${1:-1.1.0}"
DEST="${2:-$HOME/.local/core/$VERSION}"
REPO="${CORE_SDK_REPO:-dimgrig/Core}"
PLATFORM="${CORE_SDK_PLATFORM:-linux-amd64}"

ART="Core-Releases-${VERSION}-${PLATFORM}-qt5.tar.gz"
BASE_URL="https://github.com/${REPO}/releases/download/v${VERSION}"
URL="${BASE_URL}/${ART}"
SHA_URL="${BASE_URL}/SHA256SUMS"

# Идемпотентно: если уже распаковано — выходим
if [ -f "$DEST/lib/cmake/Core/CoreConfig.cmake" ]; then
    echo "Core SDK ${VERSION} already at ${DEST}"
    exit 0
fi

mkdir -p "$DEST"
TMP="$(mktemp -d)"
trap 'rm -rf "$TMP"' EXIT

echo "Downloading ${URL}"
curl -fL --retry 3 -o "$TMP/${ART}" "$URL"
curl -fL --retry 3 -o "$TMP/SHA256SUMS" "$SHA_URL" || true

if [ -f "$TMP/SHA256SUMS" ]; then
    EXPECTED="$(awk -v f="$ART" '$2 == f || $2 == "*"f {print $1; exit}' "$TMP/SHA256SUMS")"
    if [ -n "${EXPECTED:-}" ]; then
        ACTUAL="$(sha256sum "$TMP/${ART}" | awk '{print $1}')"
        if [ "$ACTUAL" != "$EXPECTED" ]; then
            echo "ERROR: SHA256 mismatch for ${ART}" >&2
            echo "  expected: ${EXPECTED}" >&2
            echo "  actual:   ${ACTUAL}" >&2
            exit 1
        fi
        echo "SHA256 OK"
    else
        echo "WARN: ${ART} not listed in SHA256SUMS; skipping hash check" >&2
    fi
fi

tar -xzf "$TMP/${ART}" -C "$DEST"

if [ ! -f "$DEST/lib/cmake/Core/CoreConfig.cmake" ]; then
    echo "ERROR: CoreConfig.cmake not found in ${DEST}" >&2
    exit 1
fi

# Симлинк latest — для совместимости со старым .pri
ROOT_PARENT="$(dirname "$DEST")"
ln -sfn "$(basename "$DEST")" "${ROOT_PARENT}/latest"

echo "Core SDK ${VERSION} installed to ${DEST}"
echo "Symlink: ${ROOT_PARENT}/latest -> ${DEST}"