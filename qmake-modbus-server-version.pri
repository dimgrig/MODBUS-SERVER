# Core SDK location: env CORE_SDK_ROOT > per-platform default.
CORE_SDK_ROOT = $$(CORE_SDK_ROOT)
isEmpty(CORE_SDK_ROOT) {
    win32: CORE_SDK_ROOT = $$(LOCALAPPDATA)/Core/latest
    win64: CORE_SDK_ROOT = $$(LOCALAPPDATA)/Core/latest
    else:  CORE_SDK_ROOT = $$(HOME)/.local/core/latest
}
message(modbus-server: CORE_SDK_ROOT=$$CORE_SDK_ROOT)

LINUX_LIBS_PATH = $${CORE_SDK_ROOT}/lib
WIN_LIBS_PATH   = $$shell_path($${CORE_SDK_ROOT}/lib)
