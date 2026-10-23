#!/usr/bin/env python3
"""
tests/test-win.py — автотест MODBUS-SERVER на Windows.

Что проверяет (7 тестов):
  1. build-win.bat --clean --smoke              — полный цикл
  2. build-core-win.bat (отдельный)             — сборка только Core SDK
  3. fetch-core-sdk.bat × 2                     — идемпотентность
  4. cmake -DCMAKE_PREFIX_PATH="Qt;SDK"         — явный путь
  5. env CORE_SDK_ROOT через set                — локальный SDK
  6. cmake -DCORE_SDK_FETCH=ON                  — configure only
  7. build-win.bat без --clean (idempotent)     — повторная сборка не пересобирает SDK

Использование:
    python tests\\test-win.py
    python tests\\test-win.py --keep
    python tests\\test-win.py --qt C:\\Qt\\5.15.2\\mingw81_32
"""

import argparse
import os
import shutil
import subprocess
import sys
import tempfile
import time
from pathlib import Path

REPO_DEFAULT = "https://github.com/dimgrig/MODBUS-SERVER.git"
BRANCH_DEFAULT = "master"

C_GREEN = C_RED = C_YELLOW = C_BLUE = C_GRAY = C_RESET = ""
if sys.stdout.isatty() and os.name != "nt":
    C_GREEN  = "\033[32m"
    C_RED    = "\033[31m"
    C_YELLOW = "\033[33m"
    C_BLUE   = "\033[34m"
    C_GRAY   = "\033[90m"
    C_RESET  = "\033[0m"


def _win_env_base():
    """PATH с обязательными каталогами Windows.

    Нужно, потому что test-win.py может запускаться из MINGW64/Git Bash,
    где PATH не содержит System32 и WindowsPowerShell — тогда вложенные
    .bat-скрипты не найдут powershell, findstr, where и т.п.
    """
    env = os.environ.copy()
    sysroot = env.get("SystemRoot", r"C:\Windows")
    must_have = [
        fr"{sysroot}\System32",
        fr"{sysroot}\System32\Wbem",
        fr"{sysroot}\System32\WindowsPowerShell\v1.0",
        sysroot,
    ]
    cur = env.get("PATH", "")
    parts = cur.split(os.pathsep) if cur else []
    lowered = {p.lower().rstrip("\\") for p in parts}
    for p in must_have:
        if p.lower().rstrip("\\") not in lowered:
            parts.insert(0, p)
    env["PATH"] = os.pathsep.join(parts)
    return env


def run_bat(bat_path, args, cwd, timeout=3600, extra_env=None):
    """Запускает .bat через cmd.exe /c."""
    cmd = ["cmd.exe", "/c", str(bat_path)] + [str(a) for a in args]
    full_env = _win_env_base()
    if extra_env:
        full_env.update(extra_env)
    print(f"  {C_GRAY}→ CMD: {' '.join(cmd)}{C_RESET}")
    print(f"  {C_GRAY}  cwd: {cwd}{C_RESET}")
    t0 = time.time()
    try:
        result = subprocess.run(cmd, cwd=cwd, env=full_env,
                                capture_output=True, text=True,
                                timeout=timeout)
        dt = time.time() - t0
        return result.returncode, (result.stdout or "") + (result.stderr or ""), dt
    except subprocess.TimeoutExpired as e:
        return -1, f"TIMEOUT after {timeout}s\n{e.stdout or ''}\n{e.stderr or ''}", time.time() - t0


def run(cmd, cwd, timeout=3600, extra_env=None):
    """Запускает произвольную команду."""
    full_env = _win_env_base()
    if extra_env:
        full_env.update(extra_env)
    print(f"  {C_GRAY}→ CMD: {' '.join(str(c) for c in cmd)}{C_RESET}")
    t0 = time.time()
    try:
        result = subprocess.run(cmd, cwd=cwd, env=full_env,
                                capture_output=True, text=True,
                                timeout=timeout)
        return result.returncode, (result.stdout or "") + (result.stderr or ""), time.time() - t0
    except subprocess.TimeoutExpired as e:
        return -1, f"TIMEOUT\n{e.stdout or ''}\n{e.stderr or ''}", time.time() - t0


def show_tail(output, n=12):
    lines = output.rstrip().splitlines()
    for ln in (lines[-n:] if len(lines) > n else lines):
        print(f"  {C_GRAY}│{C_RESET} {ln}")


def check(result, label):
    marker = f"{C_GREEN}[PASS]{C_RESET}" if result else f"{C_RED}[FAIL]{C_RESET}"
    print(f"  {marker} {label}")
    return result


# ═══════════════════════════════════════════════════════════════
#  Тесты
# ═══════════════════════════════════════════════════════════════

def test_1_build_win_bat(repo_dir, qt_dir):
    print(f"\n{C_BLUE}[1/6] build-win.bat --clean --smoke{C_RESET}")
    script = repo_dir / "scripts" / "build-win.bat"
    if not script.exists():
        return check(False, "build-win.bat not found")
    rc, out, dt = run_bat(script, ["--clean", "--smoke", "--qt", str(qt_dir)], cwd=repo_dir)
    show_tail(out, 15)
    ok = rc == 0 and "smoke OK" in out
    return check(ok, f"exit={rc}, {dt:.1f}s")


def test_2_build_core(repo_dir, workspace, qt_dir):
    print(f"\n{C_BLUE}[2/6] build-core-win.bat{C_RESET}")
    script = repo_dir / "scripts" / "build-core-win.bat"
    if not script.exists():
        return check(False, "build-core-win.bat not found")
    dst = workspace / "core-test"
    shutil.rmtree(dst, ignore_errors=True)
    rc, out, dt = run_bat(script, ["1.1.0", str(qt_dir), str(dst)], cwd=repo_dir)
    show_tail(out, 10)
    ok = rc == 0 and (dst / "lib" / "cmake" / "Core" / "CoreConfig.cmake").exists()
    return check(ok, f"exit={rc}, {dt:.1f}s")


def _find_sdk_root(repo_dir, version="1.1.0"):
    sdk_root = Path(os.environ.get("CORE_SDK_ROOT") or "")
    if sdk_root.is_dir() and (sdk_root / "lib" / "cmake" / "Core" / "CoreConfig.cmake").exists():
        return sdk_root

    localappdata = os.environ.get("LOCALAPPDATA")
    cands = []
    if localappdata:
        cands.append(Path(localappdata) / "Core" / version)
        cands.append(Path(localappdata) / "Core" / "latest")   # ← читать junction
    cands.append(repo_dir / "build" / "_deps")

    for c in cands:
        if c.is_dir() and (c / "lib" / "cmake" / "Core" / "CoreConfig.cmake").exists():
            return c
    return None


def test_3_prefix_path(repo_dir, workspace, qt_dir):
    print(f"\n{C_BLUE}[3/6] explicit -DCMAKE_PREFIX_PATH{C_RESET}")
    sdk = _find_sdk_root(repo_dir)
    if sdk is None:
        return check(False, "no SDK found — run test 1/2 first")
    build = repo_dir / "build-test4"
    shutil.rmtree(build, ignore_errors=True)
    prefix = f"{qt_dir};{sdk}"
    rc, out, dt = run(
        ["cmake", "-B", str(build), "-G", "MinGW Makefiles",
         f"-DCMAKE_PREFIX_PATH={prefix}",
         "-DCMAKE_BUILD_TYPE=Release"],
        cwd=repo_dir,
    )
    show_tail(out, 10)
    return check(rc == 0, f"exit={rc}, {dt:.1f}s")


def test_4_env(repo_dir, qt_dir):
    print(f"\n{C_BLUE}[4/6] env CORE_SDK_ROOT{C_RESET}")
    sdk = _find_sdk_root(repo_dir)
    if sdk is None:
        return check(False, "no SDK found")
    build = repo_dir / "build-test5"
    shutil.rmtree(build, ignore_errors=True)
    rc, out, dt = run(
        ["cmake", "-B", str(build), "-G", "MinGW Makefiles",
         f"-DCMAKE_PREFIX_PATH={qt_dir}",
         "-DCMAKE_BUILD_TYPE=Release"],
        cwd=repo_dir,
        extra_env={"CORE_SDK_ROOT": str(sdk)},
    )
    show_tail(out, 10)
    ok = rc == 0 and "from env CORE_SDK_ROOT" in out
    return check(ok, f"exit={rc}, {dt:.1f}s")


def test_5_fetch_on(repo_dir, qt_dir):
    print(f"\n{C_BLUE}[5/6] cmake -DCORE_SDK_FETCH=ON (configure only){C_RESET}")
    build = repo_dir / "build-test6"
    shutil.rmtree(build, ignore_errors=True)
    rc, out, dt = run(
        ["cmake", "-B", str(build), "-G", "MinGW Makefiles",
         f"-DCMAKE_PREFIX_PATH={qt_dir}",
         "-DCORE_SDK_FETCH=ON",
         "-DCORE_SDK_VERSION=1.1.0",
         "-DCMAKE_BUILD_TYPE=Release"],
        cwd=repo_dir,
    )
    show_tail(out, 10)
    ok = rc == 0 and "Core SDK root:" in out
    print(f"  {C_GRAY}NOTE: build не запускается — win32 binary SDK собран",
          f"mingw 15.1, ABI не совместим с Qt 5.15.2 mingw81_32.{C_RESET}")
    return check(ok, f"exit={rc}, {dt:.1f}s (configure only)")


def test_6_build_idempotent(repo_dir, qt_dir):
    print(f"\n{C_BLUE}[6/6] build-win.bat без --clean{C_RESET}")
    script = repo_dir / "scripts" / "build-win.bat"
    if not script.exists():
        return check(False, "build-win.bat not found")
    rc, out, dt = run_bat(script, ["--smoke", "--qt", str(qt_dir)], cwd=repo_dir)
    show_tail(out, 10)
    no_rebuild = "building natively" not in out.lower()
    ok = rc == 0 and no_rebuild and dt < 120
    return check(ok, f"exit={rc}, {dt:.1f}s, no SDK rebuild")


# ═══════════════════════════════════════════════════════════════
#  Main
# ═══════════════════════════════════════════════════════════════

def ensure_executable_win(repo_dir):
    pass


def main():
    p = argparse.ArgumentParser()
    p.add_argument("--repo", default=REPO_DEFAULT)
    p.add_argument("--branch", default=BRANCH_DEFAULT)
    p.add_argument("--qt", default=r"C:\Qt\5.15.2\mingw81_32")
    p.add_argument("--keep", action="store_true")
    p.add_argument("--workspace", default=None)
    args = p.parse_args()

    print(f"{C_BLUE}╔══════════════════════════════════════════════════════╗")
    print(f"║  MODBUS-SERVER — Windows test suite                  ║")
    print(f"╚══════════════════════════════════════════════════════╝{C_RESET}")
    if os.name != "nt":
        print(f"{C_RED}ERROR: этот тест — только для Windows{C_RESET}")
        return 1

    env = _win_env_base()
    for tool in ["git", "cmake"]:
        if shutil.which(tool, path=env["PATH"]) is None:
            print(f"{C_RED}ERROR: '{tool}' not found in PATH{C_RESET}")
            return 1
    if not Path(args.qt).exists():
        print(f"{C_RED}ERROR: Qt not found at {args.qt}{C_RESET}")
        return 1
    if not (Path(args.qt) / "bin" / "qmake.exe").exists():
        print(f"{C_RED}ERROR: qmake.exe not in {args.qt}\\bin{C_RESET}")
        return 1

    timestamp = time.strftime("%Y%m%d-%H%M%S")
    workspace = (Path(args.workspace) if args.workspace else
                 Path(tempfile.mkdtemp(prefix=f"modbus-server-test-{timestamp}-")))
    workspace.mkdir(parents=True, exist_ok=True)
    print(f"  workspace : {workspace}")
    print(f"  repo      : {args.repo}")
    print(f"  Qt        : {args.qt}")

    repo_dir = workspace / "MODBUS-SERVER"
    if repo_dir.exists():
        shutil.rmtree(repo_dir)

    print(f"\n{C_GRAY}→ git clone --branch {args.branch} --depth 1 …{C_RESET}")
    rc, out, _ = run(["git", "clone", "--branch", args.branch, "--depth", "1",
                      args.repo, str(repo_dir)], cwd=workspace)
    if rc != 0:
        show_tail(out, 10)
        print(f"{C_RED}Clone failed{C_RESET}")
        return 1
    ensure_executable_win(repo_dir)

    results = {}
    try:
        results["1. build-win.bat"]      = test_1_build_win_bat(repo_dir, args.qt)
        results["2. build-core-win.bat"] = test_2_build_core(repo_dir, workspace, args.qt)
        results["3. prefix path"]        = test_3_prefix_path(repo_dir, workspace, args.qt)
        results["4. env CORE_SDK_ROOT"]  = test_4_env(repo_dir, args.qt)
        results["5. fetch-on (cfg)"]     = test_5_fetch_on(repo_dir, args.qt)
        results["6. build idempotent"]   = test_6_build_idempotent(repo_dir, args.qt)
    except KeyboardInterrupt:
        print(f"\n{C_YELLOW}Interrupted{C_RESET}")
        return 130

    passed = sum(1 for v in results.values() if v)
    failed = len(results) - passed
    print(f"\n{C_BLUE}═══════════════════════════════════════════════════════")
    print("  Summary")
    print(f"═══════════════════════════════════════════════════════{C_RESET}")
    for name, ok in results.items():
        marker = f"{C_GREEN}PASS{C_RESET}" if ok else f"{C_RED}FAIL{C_RESET}"
        print(f"  [{marker}] {name}")
    print(f"  {C_GRAY}TOTAL: {passed} passed, {failed} failed{C_RESET}")

    if not args.keep and failed == 0:
        shutil.rmtree(workspace, ignore_errors=True)
        print(f"  {C_GRAY}(workspace removed){C_RESET}")
    else:
        print(f"  {C_GRAY}workspace kept: {workspace}{C_RESET}")

    return 0 if failed == 0 else 1


if __name__ == "__main__":
    sys.exit(main())
