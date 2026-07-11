#!/usr/bin/env python3
import hashlib
import json
import subprocess
import sys
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
LEDGER = ROOT / "SOURCE_PROVENANCE.json"


def fail(message: str) -> None:
    print(f"[SOURCE_PROVENANCE_FAILED] {message}", file=sys.stderr)
    raise SystemExit(1)


def tracked_files(path: str) -> list[str]:
    result = subprocess.run(
        ["git", "ls-files", path],
        cwd=ROOT,
        check=True,
        text=True,
        capture_output=True,
    )
    return [line for line in result.stdout.splitlines() if line]


def sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as handle:
        for block in iter(lambda: handle.read(1024 * 1024), b""):
            digest.update(block)
    return digest.hexdigest()


def main() -> None:
    try:
        data = json.loads(LEDGER.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError) as error:
        fail(f"cannot read {LEDGER.name}: {error}")

    if data.get("schema_version") != 1:
        fail("schema_version must be 1")
    if data.get("ledger_status") != "HYBRID_SOURCE_TREE":
        fail("ledger_status must be HYBRID_SOURCE_TREE")

    source_sets = {entry["id"]: entry for entry in data.get("source_sets", [])}
    corpus = source_sets.get("ghidra-module-corpus")
    touchscreen = source_sets.get("zte-touchscreen-reconstruction")
    if corpus is None or touchscreen is None:
        fail("required source sets are missing")

    corpus_path = ROOT / corpus["path"]
    corpus_files = tracked_files(corpus["path"])
    module_count = sum(1 for path in corpus_path.iterdir() if path.is_dir())
    if len(corpus_files) != corpus["tracked_file_count"]:
        fail("decompiled tracked_file_count does not match Git")
    if module_count != corpus["top_level_module_count"]:
        fail("decompiled top_level_module_count does not match the tree")

    touch_path = ROOT / touchscreen["path"]
    touch_files = tracked_files(touchscreen["path"])
    touch_c_files = [path for path in touch_files if path.endswith(".c")]
    if len(touch_files) != touchscreen["tracked_file_count"]:
        fail("zte_tpd tracked_file_count does not match Git")
    if len(touch_c_files) != touchscreen["c_file_count"]:
        fail("zte_tpd c_file_count does not match Git")
    if not (touch_path / "Makefile").is_file() or not (touch_path / "analysis.md").is_file():
        fail("zte_tpd reconstruction metadata is incomplete")

    for module in data.get("reconstructed_modules", []):
        module_path = ROOT / module["path"]
        if not module_path.is_dir():
            fail(f"missing reconstructed module path: {module['path']}")
        if not any(module_path.glob("*.c")):
            fail(f"reconstructed module has no C source: {module['path']}")
        if not (module_path / "Makefile").is_file() or not (module_path / "Kbuild").is_file():
            fail(f"reconstructed module lacks Makefile/Kbuild: {module['path']}")

    for evidence in data.get("binary_evidence", []):
        evidence_path = ROOT / evidence["path"]
        if not evidence_path.is_file():
            fail(f"missing binary evidence: {evidence['path']}")
        if sha256(evidence_path) != evidence["sha256"]:
            fail(f"checksum mismatch: {evidence['path']}")
        if evidence.get("build_input") is not False:
            fail(f"binary evidence must not be a build input: {evidence['path']}")

    for missing in data.get("missing_integrated_source_form", []):
        if missing.get("status") != "ABSENT":
            fail(f"unexpected missing-source status: {missing['component']}")
        if (ROOT / missing["expected_path"]).exists():
            fail(f"ledger marks a present source path absent: {missing['expected_path']}")

    print("[SOURCE_PROVENANCE_OK] ledger paths, counts, and hashes verified")


if __name__ == "__main__":
    main()
