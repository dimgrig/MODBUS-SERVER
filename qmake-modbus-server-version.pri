# qmake-modbus-server-version.pri — резолвинг Core SDK для qmake-сборки.
#
# Приоритет:
#   1. env CORE_SDK_ROOT
#   2. CONFIG+=core_sdk_fetch → запуск scripts/fetch-core-sdk.{sh,bat}
#   3. платформенный дефолт ($HOME/.local/core/latest или %LOCALAPPDATA%/Core/latest)
#
# Версия Core SDK:
#   env CORE_SDK_VERSION (напр. export CORE_SDK_VERSION=1.1.0)
#   иначе 1.1.0 (константа ниже; менять при апгрейде SDK)

CORE_SDK_VERSION = $$(CORE_SDK_VERSION)
isEmpty(CORE_SDK_VERSION): CORE_SDK_VERSION = 1.1.0

CORE_SDK_ROOT = $$(CORE_SDK_ROOT)
isEmpty(CORE_SDK_ROOT) {
    win32|win64: CORE_SDK_ROOT = $$(LOCALAPPDATA)/Core/latest
    else:        CORE_SDK_ROOT = $$(HOME)/.local/core/latest
}

# Опционально: скачать SDK, если ещё нет
CONFIG(core_sdk_fetch) {
    !exists($${CORE_SDK_ROOT}/lib/cmake/Core/CoreConfig.cmake) {
        win32|win64: FETCH = $$shell_path($$PWD/scripts/fetch-core-sdk.bat)
        else:        FETCH = $$shell_path($$PWD/scripts/fetch-core-sdk.sh)
        message(modbus-client: fetching Core SDK $${CORE_SDK_VERSION} → $${CORE_SDK_ROOT})
        system($$FETCH $${CORE_SDK_VERSION} $${CORE_SDK_ROOT})
    }
}

!exists($${CORE_SDK_ROOT}/lib/cmake/Core/CoreConfig.cmake) {
    error("Core SDK not found at $${CORE_SDK_ROOT}. "
          "Set CORE_SDK_ROOT env var or run `qmake CONFIG+=core_sdk_fetch`.")
}

message(modbus-server: CORE_SDK_ROOT=$$CORE_SDK_ROOT)

LINUX_LIBS_PATH = $${CORE_SDK_ROOT}/lib
WIN_LIBS_PATH   = $$shell_path($${CORE_SDK_ROOT}/lib)