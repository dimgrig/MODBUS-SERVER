#!/usr/bin/env python3
"""
tests/test-linux.py — автотест MODBUS-SERVER на Linux.

Что проверяет (6 тестов):

  1. build-linux.sh --clean --smoke          — полный цикл сборки приложения
  2. fetch-core-sdk.sh × 2                    — идемпотентность fetch-скрипта
  3. cmake -DCORE_SDK_FETCH=ON               — автоскачивание SDK
  4. повторный cmake -DCORE_SDK_FETCH=ON     — кэш (без перекачивания)
  5. env CORE_SDK_ROOT                        — локальный SDK через переменную
  6. явный -DCMAKE_PREFIX_PATH                — ручной путь к SDK

Использование:
    python3 tests/test-linux.py                 # все тесты
    python3 tests/test-linux.py --keep          # не удалять workspace
    python3 tests/test-linux.py --level 1       # только базовый
    python3 tests/test-linux.py --repo URL      # другой репозиторий
    python3 tests/test-linux.py --branch master # другая ветка

Требования на хосте: python3, git, cmake, g++, Qt5 dev, curl, tar.
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

C_GREEN  = "\033[32m"
C_RED    = "\033[31m"
C_YELLOW = "\033[33m"
C_BLUE   = "\033[34m"
C_GRAY   = "\033[90m"
C_RESET  = "\033[0m"

if not sys.stdout.isatty():
    C_GREEN = C_RED = C_YELLOW = C_BLUE = C_GRAY = C_RESET = ""


def run(cmd, cwd=None, env=None, timeout=1200, extra_env=None):
    """Запускает команду, возвращает (returncode, output, elapsed)."""
    full_env = os.environ.copy()
    if env:
        full_env.update(env)
    if extra_env:
        full_env.update(extra_env)

    print(f"  {C_GRAY}→ CMD: {' '.join(str(c) for c in cmd)}{C_RESET}")
    print(f"  {C_GRAY}  cwd: {cwd or os.getcwd()}{C_RESET}")

    t0 = time.time()
    try:
        result = subprocess.run(
            cmd, cwd=cwd, env=full_env,
            capture_output=True, text=True, timeout=timeout,
        )
        elapsed = time.time() - t0
        output = (result.stdout or "") + (result.stderr or "")
        return result.returncode, output, elapsed
    except subprocess.TimeoutExpired as e:
        elapsed = time.time() - t0
        out = (e.stdout or "") + (e.stderr or "")
        return -1, f"TIMEOUT after {timeout}s\n{out}", elapsed
    except FileNotFoundError as e:
        return -2, f"NOT FOUND: {e}", time.time() - t0


def show_tail(output, n=10):
    lines = output.rstrip().splitlines()
    tail = lines[-n:] if len(lines) > n else lines
    for ln in tail:
        print(f"  {C_GRAY}│{C_RESET} {ln}")


def check(result, label):
    if result:
        print(f"  {C_GREEN}[PASS]{C_RESET} {label}")
    else:
        print(f"  {C_RED}[FAIL]{C_RESET} {label}")
    return result


# ═══════════════════════════════════════════════════════════════
#  Тесты
# ═══════════════════════════════════════════════════════════════

def test_1_build_script(repo_dir):
    """build-linux.sh --clean --smoke — полный цикл."""
    print(f"\n{C_BLUE}[1/6] build-linux.sh --clean --smoke{C_RESET}")
    script = repo_dir / "scripts" / "build-linux.sh"
    if not script.exists():
        return check(False, "script not found")

    rc, out, dt = run(
        ["bash", str(script), "--clean", "--smoke"],
        cwd=repo_dir,
        env={"QT_QPA_PLATFORM": "offscreen"},
    )
    show_tail(out, 15)
    ok = rc == 0 and "smoke OK" in out
    return check(ok, f"exit={rc}, {dt:.1f}s")


def test_2_fetch_idempotent(workspace):
    """fetch-core-sdk.sh дважды — второй раз без скачивания."""
    print(f"\n{C_BLUE}[2/6] fetch-core-sdk.sh idempotent{C_RESET}")
    repo_dir = workspace / "MODBUS-SERVER"
    script = repo_dir / "scripts" / "fetch-core-sdk.sh"
    if not script.exists():
        return check(False, "script not found")

    sdk_dst = workspace / "sdk-test"
    shutil.rmtree(sdk_dst, ignore_errors=True)

    # 1-й раз — скачивание
    rc, out1, _ = run(["bash", str(script), "1.1.0", str(sdk_dst)], cwd=repo_dir)
    if rc != 0:
        show_tail(out1)
        return check(False, "first fetch failed")

    ok1 = (sdk_dst / "lib" / "cmake" / "Core" / "CoreConfig.cmake").exists()

    # 2-й раз — должен быть быстрый выход
    t0 = time.time()
    rc, out2, _ = run(["bash", str(script), "1.1.0", str(sdk_dst)], cwd=repo_dir)
    dt2 = time.time() - t0

    ok2 = rc == 0 and "already at" in out2 and dt2 < 2.0

    print(f"  {C_GRAY}first fetch: cfg exists = {ok1}{C_RESET}")
    print(f"  {C_GRAY}second fetch: 'already at' + fast = {ok2} ({dt2:.2f}s){C_RESET}")
    if not (ok1 and ok2):
        print(f"  {C_GRAY}second output:{C_RESET}")
        show_tail(out2, 5)

    return check(ok1 and ok2, "idempotent + fast second call")


def test_3_fetch_on(repo_dir):
    """cmake -DCORE_SDK_FETCH=ON — автоскачивание SDK."""
    print(f"\n{C_BLUE}[3/6] cmake -DCORE_SDK_FETCH=ON{C_RESET}")
    build_dir = repo_dir / "build-test3"
    shutil.rmtree(build_dir, ignore_errors=True)

    rc, out, dt = run(
        ["cmake", "-B", str(build_dir),
         "-DCMAKE_BUILD_TYPE=Release",
         "-DCORE_SDK_FETCH=ON",
         "-DCORE_SDK_VERSION=1.1.0"],
        cwd=repo_dir,
    )
    show_tail(out, 10)
    ok = rc == 0 and "Core SDK root:" in out
    return check(ok, f"exit={rc}, {dt:.1f}s")


def test_4_fetch_cache(repo_dir):
    """Повторный cmake -DCORE_SDK_FETCH=ON — кэш без перекачивания."""
    print(f"\n{C_BLUE}[4/6] cmake -DCORE_SDK_FETCH=ON (cache){C_RESET}")
    build_dir = repo_dir / "build-test3"  # тот же, что в тесте 3
    if not build_dir.exists():
        return check(False, "build-test3 missing (test 3 must pass first)")

    deps = build_dir / "_deps"
    if not deps.exists():
        return check(False, "_deps missing (test 3 must pass first)")

    # Находим архив SDK до запуска cmake, запоминаем mtime и размер
    archives_before = list(deps.glob("core-sdk-*/*.tar.gz")) + \
                      list(deps.glob("core-sdk-*/*.zip"))
    if not archives_before:
        return check(False, "no SDK archive in _deps")
    archive = archives_before[0]
    stat_before = archive.stat()

    # Удаляем CMakeCache.txt, но оставляем _deps
    cache_file = build_dir / "CMakeCache.txt"
    if cache_file.exists():
        cache_file.unlink()
    cmake_files = build_dir / "CMakeFiles"
    if cmake_files.exists():
        shutil.rmtree(cmake_files)

    t0 = time.time()
    rc, out, _ = run(
        ["cmake", "-B", str(build_dir),
         "-DCMAKE_BUILD_TYPE=Release",
         "-DCORE_SDK_FETCH=ON",
         "-DCORE_SDK_VERSION=1.1.0"],
        cwd=repo_dir,
    )
    dt = time.time() - t0

    show_tail(out, 8)

    # Проверяем по mtime архива: если перекачали — mtime обновится
    stat_after = archive.stat() if archive.exists() else None
    same_archive = (
        stat_after is not None
        and stat_after.st_mtime == stat_before.st_mtime
        and stat_after.st_size == stat_before.st_size
    )

    # Дополнительно: сообщение "using cached" должно быть в выводе
    msg_ok = "using cached" in out.lower()

    print(f"  {C_GRAY}archive mtime before: {stat_before.st_mtime}{C_RESET}")
    print(f"  {C_GRAY}archive mtime after:  "
          f"{stat_after.st_mtime if stat_after else 'missing'}{C_RESET}")
    print(f"  {C_GRAY}message 'using cached': {msg_ok}{C_RESET}")

    ok = rc == 0 and same_archive and msg_ok
    return check(ok, f"exit={rc}, {dt:.1f}s, cached (mtime unchanged)")


def test_5_env(workspace, repo_dir):
    """env CORE_SDK_ROOT — локальный SDK через переменную."""
    print(f"\n{C_BLUE}[5/6] env CORE_SDK_ROOT{C_RESET}")

    # Используем уже скачанный SDK из теста 3
    sdk_dir = repo_dir / "build-test3" / "_deps"
    if not sdk_dir.exists():
        return check(False, "test 3 must pass first (no _deps)")
    sdk_candidates = list(sdk_dir.glob("core-sdk-*/extracted"))
    if not sdk_candidates:
        return check(False, "no extracted SDK")

    sdk_root = sdk_candidates[0]
    # Может быть вложенным
    if not (sdk_root / "lib" / "cmake" / "Core" / "CoreConfig.cmake").exists():
        inner = list(sdk_root.iterdir())
        if inner and (inner[0] / "lib" / "cmake" / "Core" / "CoreConfig.cmake").exists():
            sdk_root = inner[0]

    build_dir = repo_dir / "build-test5"
    shutil.rmtree(build_dir, ignore_errors=True)

    rc, out, dt = run(
        ["cmake", "-B", str(build_dir), "-DCMAKE_BUILD_TYPE=Release"],
        cwd=repo_dir,
        extra_env={"CORE_SDK_ROOT": str(sdk_root)},
    )
    show_tail(out, 8)

    ok = rc == 0 and "from env CORE_SDK_ROOT" in out
    return check(ok, f"exit={rc}, {dt:.1f}s")


def test_6_prefix_path(workspace, repo_dir):
    """Явный -DCMAKE_PREFIX_PATH."""
    print(f"\n{C_BLUE}[6/6] explicit -DCMAKE_PREFIX_PATH{C_RESET}")

    sdk_dir = repo_dir / "build-test3" / "_deps"
    if not sdk_dir.exists():
        return check(False, "test 3 must pass first")
    sdk_candidates = list(sdk_dir.glob("core-sdk-*/extracted"))
    if not sdk_candidates:
        return check(False, "no extracted SDK")
    sdk_root = sdk_candidates[0]
    if not (sdk_root / "lib" / "cmake" / "Core" / "CoreConfig.cmake").exists():
        inner = list(sdk_root.iterdir())
        if inner and (inner[0] / "lib" / "cmake" / "Core" / "CoreConfig.cmake").exists():
            sdk_root = inner[0]

    build_dir = repo_dir / "build-test6"
    shutil.rmtree(build_dir, ignore_errors=True)

    rc, out, dt = run(
        ["cmake", "-B", str(build_dir),
         "-DCMAKE_BUILD_TYPE=Release",
         f"-DCMAKE_PREFIX_PATH={sdk_root}"],
        cwd=repo_dir,
    )
    show_tail(out, 8)
    ok = rc == 0
    return check(ok, f"exit={rc}, {dt:.1f}s")


# ═══════════════════════════════════════════════════════════════
#  Main
# ═══════════════════════════════════════════════════════════════

def ensure_executable(repo_dir):
    """chmod +x scripts/*.sh после clone — для git-клонов с другой ОС."""
    scripts_dir = repo_dir / "scripts"
    if not scripts_dir.exists():
        return
    for f in scripts_dir.iterdir():
        if f.suffix in (".sh", ".bat") or f.suffix == "":
            try:
                os.chmod(f, 0o755)
            except OSError:
                pass


def main():
    p = argparse.ArgumentParser()
    p.add_argument("--repo", default=REPO_DEFAULT)
    p.add_argument("--branch", default=BRANCH_DEFAULT)
    p.add_argument("--keep", action="store_true",
                   help="не удалять workspace после тестов")
    p.add_argument("--workspace", default=None,
                   help="где создать workspace (по умолчанию /tmp)")
    args = p.parse_args()

    print(f"{C_BLUE}╔══════════════════════════════════════════════════════╗")
    print(f"║  MODBUS-SERVER — Linux test suite                    ║")
    print(f"╚══════════════════════════════════════════════════════╝{C_RESET}")

    # ── Проверка зависимостей ─────────────────────────────────
    for tool in ["git", "cmake", "g++", "bash", "curl", "tar"]:
        if shutil.which(tool) is None:
            print(f"{C_RED}ERROR: '{tool}' not found in PATH{C_RESET}")
            return 1
    if shutil.which("qmake") is None and shutil.which("qmake-qt5") is None:
        print(f"{C_RED}ERROR: qmake not found (install qtbase5-dev qttools5-dev-tools){C_RESET}")
        return 1

    # ── Workspace ─────────────────────────────────────────────
    timestamp = time.strftime("%Y%m%d-%H%M%S")
    if args.workspace:
        workspace = Path(args.workspace).expanduser().resolve()
    else:
        workspace = Path(tempfile.mkdtemp(prefix=f"modbus-server-test-{timestamp}-"))
    workspace.mkdir(parents=True, exist_ok=True)
    print(f"  workspace : {workspace}")
    print(f"  repo      : {args.repo}")
    print(f"  branch    : {args.branch}")

    # ── Clone ─────────────────────────────────────────────────
    repo_dir = workspace / "MODBUS-SERVER"
    if repo_dir.exists():
        shutil.rmtree(repo_dir)

    print(f"\n{C_GRAY}→ git clone --branch {args.branch} --depth 1 …{C_RESET}")
    rc, out, _ = run(
        ["git", "clone", "--branch", args.branch, "--depth", "1",
         args.repo, str(repo_dir)],
        cwd=workspace,
    )
    if rc != 0:
        show_tail(out, 10)
        print(f"{C_RED}Clone failed{C_RESET}")
        return 1
    ensure_executable(repo_dir)

    # ── Run tests ─────────────────────────────────────────────
    results = {}

    try:
        results["1. build script"]     = test_1_build_script(repo_dir)
        results["2. fetch idempotent"] = test_2_fetch_idempotent(workspace)
        results["3. fetch-on"]         = test_3_fetch_on(repo_dir)
        results["4. fetch-cache"]      = test_4_fetch_cache(repo_dir)
        results["5. env CORE_SDK_ROOT"] = test_5_env(workspace, repo_dir)
        results["6. prefix path"]      = test_6_prefix_path(workspace, repo_dir)
    except KeyboardInterrupt:
        print(f"\n{C_YELLOW}Interrupted{C_RESET}")
        return 130

    # ── Summary ───────────────────────────────────────────────
    passed = sum(1 for v in results.values() if v)
    failed = len(results) - passed
    print(f"\n{C_BLUE}═══════════════════════════════════════════════════════")
    print(f"  Summary")
    print(f"═══════════════════════════════════════════════════════{C_RESET}")
    for name, ok in results.items():
        marker = f"{C_GREEN}PASS{C_RESET}" if ok else f"{C_RED}FAIL{C_RESET}"
        print(f"  [{marker}] {name}")
    print(f"  {C_GRAY}TOTAL: {passed} passed, {failed} failed{C_RESET}")

    if not args.keep and failed == 0:
        shutil.rmtree(workspace, ignore_errors=True)
        print(f"  {C_GRAY}(workspace removed; use --keep to keep){C_RESET}")
    else:
        print(f"  {C_GRAY}workspace kept: {workspace}{C_RESET}")

    return 0 if failed == 0 else 1


if __name__ == "__main__":
    sys.exit(main())
