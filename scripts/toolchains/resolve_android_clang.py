#!/usr/bin/env python3
import argparse
import hashlib
import json
import os
import re
import subprocess
import sys
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
LOCK_PATH = ROOT / "toolchains.lock.json"


def fail(message: str) -> None:
    print(f"[TOOLCHAIN_FAILED] {message}", file=sys.stderr)
    raise SystemExit(1)


def load_entry() -> dict:
    try:
        data = json.loads(LOCK_PATH.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError) as error:
        fail(f"cannot read {LOCK_PATH.name}: {error}")

    if data.get("schema_version") != 1:
        fail("schema_version must be 1")
    entries = data.get("toolchains", [])
    if len(entries) != 1 or entries[0].get("id") != "android-clang-r536225":
        fail("exactly one android-clang-r536225 entry is required")

    entry = entries[0]
    required = {
        "repository",
        "repository_commit",
        "relative_path",
        "binary",
        "binary_sha256",
        "compiler_version",
        "compiler_identity_fragments",
        "default_cache_root",
    }
    missing = sorted(required - entry.keys())
    if missing:
        fail(f"missing lock fields: {', '.join(missing)}")
    if not re.fullmatch(r"[0-9a-f]{40}", entry["repository_commit"]):
        fail("repository_commit must be a 40-character lowercase Git object ID")
    if not re.fullmatch(r"[0-9a-f]{64}", entry["binary_sha256"]):
        fail("binary_sha256 must be a lowercase SHA-256 digest")
    for key in ("relative_path", "binary", "default_cache_root"):
        value = Path(entry[key])
        if value.is_absolute() or ".." in value.parts:
            fail(f"{key} must be a safe relative path")
    return entry


def file_sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as handle:
        for block in iter(lambda: handle.read(1024 * 1024), b""):
            digest.update(block)
    return digest.hexdigest()


def default_toolchain_dir(entry: dict) -> Path:
    configured_root = os.environ.get("REVERSA_TOOLCHAIN_ROOT")
    if configured_root:
        cache_root = Path(configured_root).expanduser()
    else:
        cache_home = Path(os.environ.get("XDG_CACHE_HOME", Path.home() / ".cache"))
        cache_root = cache_home / entry["default_cache_root"]
    return cache_root / entry["relative_path"]


def verify_toolchain(entry: dict, toolchain_dir: Path) -> None:
    compiler = toolchain_dir / entry["binary"]
    if not compiler.is_file() or not os.access(compiler, os.X_OK):
        fail(f"compiler is missing or not executable: {compiler}")
    actual_sha256 = file_sha256(compiler)
    if actual_sha256 != entry["binary_sha256"]:
        fail(
            f"compiler hash mismatch: expected {entry['binary_sha256']}, "
            f"got {actual_sha256}"
        )

    result = subprocess.run(
        [str(compiler), "--version"],
        check=True,
        capture_output=True,
        text=True,
    )
    identity = result.stdout.splitlines()[0]
    missing = [part for part in entry["compiler_identity_fragments"] if part not in identity]
    if missing:
        fail(f"compiler identity is missing: {', '.join(missing)}")

    cache_repo = toolchain_dir.parent
    if (cache_repo / ".git").exists():
        revision = subprocess.run(
            ["git", "-C", str(cache_repo), "rev-parse", "HEAD"],
            check=True,
            capture_output=True,
            text=True,
        ).stdout.strip()
        if revision != entry["repository_commit"]:
            fail(
                f"cache revision mismatch: expected {entry['repository_commit']}, "
                f"got {revision}"
            )

    print(f"[TOOLCHAIN_OK] {identity}", file=sys.stderr)


def main() -> None:
    parser = argparse.ArgumentParser(description="Resolve and verify the pinned Android Clang toolchain")
    parser.add_argument("--clang-dir", type=Path)
    parser.add_argument("--metadata-only", action="store_true")
    args = parser.parse_args()

    entry = load_entry()
    if args.metadata_only:
        print("[TOOLCHAIN_LOCK_OK] pinned Android Clang metadata verified")
        return

    configured_dir = args.clang_dir or (
        Path(os.environ["CLANG_DIR"]) if os.environ.get("CLANG_DIR") else None
    )
    toolchain_dir = (configured_dir or default_toolchain_dir(entry)).expanduser().resolve()
    verify_toolchain(entry, toolchain_dir)
    print(toolchain_dir)


if __name__ == "__main__":
    main()
