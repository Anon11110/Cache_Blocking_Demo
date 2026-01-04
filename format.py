#!/usr/bin/env python3
"""
Code formatter script for C/C++ and shader files.
Uses clang-format for formatting.
"""

import argparse
import os
import sys
from pathlib import Path
from shutil import which
from subprocess import CalledProcessError, check_output


# Simple terminal output helpers
class Term:
    USE_COLORS = sys.stdout.isatty()
    RESET = "\033[0m" if USE_COLORS else ""
    BOLD = "\033[1m" if USE_COLORS else ""
    DIM = "\033[2m" if USE_COLORS else ""
    RED = "\033[31m" if USE_COLORS else ""
    GREEN = "\033[32m" if USE_COLORS else ""
    YELLOW = "\033[33m" if USE_COLORS else ""
    BLUE = "\033[34m" if USE_COLORS else ""
    CYAN = "\033[36m" if USE_COLORS else ""

    @staticmethod
    def section(title: str):
        print(f"\n{Term.BOLD}{Term.CYAN}{'=' * 60}{Term.RESET}")
        print(f"{Term.BOLD}{Term.CYAN}  {title}{Term.RESET}")
        print(f"{Term.BOLD}{Term.CYAN}{'=' * 60}{Term.RESET}")

    @staticmethod
    def sep():
        print(f"{Term.DIM}{'-' * 60}{Term.RESET}")

    @staticmethod
    def kv(key: str, value: str):
        print(f"{Term.BOLD}{key}:{Term.RESET} {value}")

    @staticmethod
    def info(msg: str):
        print(f"{Term.BLUE}[INFO]{Term.RESET} {msg}")

    @staticmethod
    def success(msg: str):
        print(f"{Term.GREEN}[SUCCESS]{Term.RESET} {msg}")

    @staticmethod
    def warn(msg: str):
        print(f"{Term.YELLOW}[WARN]{Term.RESET} {msg}")

    @staticmethod
    def error(msg: str):
        print(f"{Term.RED}[ERROR]{Term.RESET} {msg}")


ALLOWED_EXTENSIONS = {
    ".h", ".hpp", ".cpp",  # C++
    ".vert", ".frag", ".comp", ".geom", ".tesc", ".tese",  # Shaders
}

SKIPPED_DIRS = {"third-party", "build", ".git", ".vscode", "CMakeFiles", "__pycache__"}
SKIPPED_FILES = {".clang-format", "CMakeLists.txt"}
FORMAT_DIRS = ["src/", "shaders/"]


def get_ext(file_path: str) -> str:
    return os.path.splitext(os.path.basename(file_path))[1]


def is_allowed_source(path: Path) -> bool:
    if get_ext(str(path)) not in ALLOWED_EXTENSIONS:
        return False
    if path.name in SKIPPED_FILES:
        return False
    if any(part in SKIPPED_DIRS for part in path.parts):
        return False
    return True


def is_in_format_dirs(path: Path, root: Path) -> bool:
    if not FORMAT_DIRS:
        return True
    for rel_dir in FORMAT_DIRS:
        try:
            path.resolve().relative_to((root / rel_dir).resolve())
            return True
        except Exception:
            continue
    return False


def git_dirty_files() -> list[str]:
    out_m = check_output(["git", "ls-files", "-m"]).decode("utf-8")
    out_c = check_output(["git", "diff", "--name-only", "--cached"]).decode("utf-8")
    out_u = check_output(["git", "ls-files", "-o", "--exclude-standard"]).decode("utf-8")
    return sorted(set(filter(None, out_m.splitlines() + out_c.splitlines() + out_u.splitlines())))


def git_branch_diff_files() -> list[str]:
    try:
        upstream = check_output(
            ["git", "rev-parse", "--abbrev-ref", "--symbolic-full-name", "@{u}"]
        ).decode("utf-8").strip()
    except CalledProcessError:
        upstream = None

    merge_base = None
    if upstream:
        try:
            merge_base = check_output(["git", "merge-base", "HEAD", upstream]).decode("utf-8").strip()
        except CalledProcessError:
            pass

    if not merge_base:
        try:
            origin_head = check_output(["git", "symbolic-ref", "refs/remotes/origin/HEAD"]).decode("utf-8").strip()
            merge_base = check_output(["git", "merge-base", "HEAD", origin_head]).decode("utf-8").strip()
        except CalledProcessError:
            pass

    if not merge_base:
        try:
            merge_base = check_output(["git", "rev-list", "--max-parents=0", "HEAD"]).decode("utf-8").splitlines()[0]
        except CalledProcessError:
            return []

    try:
        out = check_output(["git", "diff", "--name-only", f"{merge_base}..HEAD"]).decode("utf-8")
        return [f for f in out.splitlines() if f]
    except CalledProcessError:
        return []


def get_all_files(root: Path) -> list[str]:
    files = []
    for format_dir in FORMAT_DIRS:
        dir_path = root / format_dir
        if dir_path.exists():
            for file_path in dir_path.rglob("*"):
                if file_path.is_file():
                    files.append(str(file_path.relative_to(root)))
    return sorted(set(files))


def main() -> int:
    parser = argparse.ArgumentParser(description="Format C/C++ and shader files using clang-format")
    group = parser.add_mutually_exclusive_group()
    group.add_argument("-i", "--input", nargs="+", help="Specific files or directories to format")
    group.add_argument("-m", "--modified", action="store_true", help="Format only dirty files")
    group.add_argument("-a", "--all", action="store_true", help="Format all files in FORMAT_DIRS")
    args = parser.parse_args()

    Term.section("Code Formatter")
    Term.kv("Format dirs", ", ".join(FORMAT_DIRS) if FORMAT_DIRS else "<all>")

    if not which("clang-format"):
        Term.error("Missing clang-format")
        return 1

    root = Path(__file__).resolve().parent
    has_git = which("git") is not None
    is_git_repo = (root / ".git").exists()

    # Gather candidates
    if args.input:
        candidates = []
        for item in args.input:
            path = Path(item)
            if path.is_file():
                candidates.append(str(path))
            elif path.is_dir():
                for fp in path.rglob("*"):
                    if fp.is_file():
                        candidates.append(str(fp))
        Term.kv("Mode", "explicit files/directories")
    elif args.all:
        Term.kv("Mode", "all files in FORMAT_DIRS")
        candidates = get_all_files(root)
    elif args.modified:
        if not has_git or not is_git_repo:
            Term.error("Git not available or not a git repository")
            return 1
        Term.kv("Mode", "git dirty files")
        candidates = git_dirty_files()
    else:
        if not has_git or not is_git_repo:
            Term.kv("Mode", "all files (git not available)")
            candidates = get_all_files(root)
        else:
            Term.kv("Mode", "current branch changes")
            candidates = git_branch_diff_files()

    # Filter
    paths = [root / f for f in candidates]
    if FORMAT_DIRS:
        paths = [p for p in paths if is_in_format_dirs(p, root)]
    files = [str(p) for p in paths if p.is_file() and is_allowed_source(p)]

    Term.kv("Files detected", str(len(files)))
    Term.sep()

    if files:
        Term.info("Formatting files:")
        for f in files:
            print(f"  {os.path.relpath(f, start=os.getcwd())}")

        failures = []
        for f in files:
            try:
                check_output(["clang-format", "-i", f, "-style", "file", "-fallback-style", "none"], cwd=str(root))
            except CalledProcessError as e:
                failures.append((f, str(e)))

        Term.sep()
        if failures:
            Term.warn("Some files failed to format:")
            for f, reason in failures:
                print(f"  {f}: {reason}")
            return 2
        else:
            Term.success("Formatting completed")
            return 0
    else:
        Term.info("No files to format")
        return 0


if __name__ == "__main__":
    sys.exit(main())
