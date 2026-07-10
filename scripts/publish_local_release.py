#!/usr/bin/env python3
"""Publish a locally compiled NX809J kernel build to GitHub Releases.

This script is intentionally dependency-free so it can run inside the Docker
kernel build container. It creates an opensource tarball from the exact Git
commit, collects build outputs, creates a GitHub Release, and uploads every
asset.
"""

from __future__ import annotations

import argparse
import json
import mimetypes
import os
import shutil
import subprocess
import sys
import tarfile
import time
import urllib.error
import urllib.parse
import urllib.request
from pathlib import Path


DEFAULT_REPOSITORY = "Coding-BR/android_kernel_nubia_sm8850_qwjujube"


def run(command: list[str], cwd: Path | None = None) -> str:
    completed = subprocess.run(
        command,
        cwd=str(cwd) if cwd else None,
        check=True,
        text=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
    )
    return completed.stdout.strip()


def github_request(
    method: str,
    url: str,
    token: str,
    *,
    data: bytes | None = None,
    content_type: str = "application/json",
) -> tuple[int, bytes]:
    headers = {
        "Accept": "application/vnd.github+json",
        "Authorization": f"Bearer {token}",
        "User-Agent": "nx809j-local-kernel-release",
        "X-GitHub-Api-Version": "2022-11-28",
    }
    if data is not None:
        headers["Content-Type"] = content_type
    request = urllib.request.Request(url, data=data, headers=headers, method=method)
    try:
        with urllib.request.urlopen(request, timeout=120) as response:
            return response.status, response.read()
    except urllib.error.HTTPError as error:
        body = error.read()
        message = body.decode("utf-8", errors="replace")
        raise RuntimeError(f"GitHub API {method} {url} failed: {error.code} {message}") from error


def github_json(
    method: str,
    url: str,
    token: str,
    payload: dict | None = None,
) -> dict:
    data = None if payload is None else json.dumps(payload).encode("utf-8")
    _, body = github_request(method, url, token, data=data)
    if not body:
        return {}
    return json.loads(body.decode("utf-8"))


def copy_asset(src: Path, dest_dir: Path, dest_name: str | None = None) -> Path | None:
    if not src.is_file():
        return None
    dest = dest_dir / (dest_name or src.name)
    if src.resolve() != dest.resolve():
        shutil.copy2(src, dest)
    return dest


def make_tar_from_dir(source_dir: Path, output_file: Path, arcname: str) -> Path | None:
    if not source_dir.is_dir():
        return None
    with tarfile.open(output_file, "w:gz") as tar:
        tar.add(source_dir, arcname=arcname)
    return output_file


def collect_assets(
    repo_root: Path,
    artifact_dir: Path,
    release_dir: Path,
    build_log: Path | None,
    short_sha: str,
) -> list[Path]:
    release_dir.mkdir(parents=True, exist_ok=True)
    assets: list[Path] = []

    candidates: list[tuple[Path, str | None]] = [
        (artifact_dir / "dev_reverse_nodtb_stockcmd.img", None),
        (artifact_dir / "dev_reverse_nodtb_stockcmd_raw.img", None),
        (artifact_dir / "dev_reverse_perfect.img", None),
        (artifact_dir / "Image", None),
        (artifact_dir / "vmlinux", None),
        (artifact_dir / "kernel.config", None),
        (artifact_dir / "zte_tpd.ko", None),
        (artifact_dir / "zte_custom_drivers.zip", None),
        (repo_root / "dev_reverse_perfect.img", None),
        (repo_root / "zte_custom_drivers.zip", None),
        (repo_root / "rm11pro_gpu_oc_1250.zip", None),
        (repo_root / "kernel_platform/common/System.map", "System.map"),
        (repo_root / "kernel_platform/common/.config", "kernel.config"),
        (repo_root / "kernel_platform/common/arch/arm64/boot/Image", "Image"),
        (repo_root / "kernel_platform/common/vmlinux", "vmlinux"),
        (repo_root / "kernel_platform/common/drivers/soc/qcom/zte/zte_tpd/zte_tpd.ko", "zte_tpd.ko"),
        (repo_root / "vendor/qcom/opensource/graphics-kernel/msm_kgsl.ko", "msm_kgsl.ko"),
        (
            repo_root / "vendor/qcom/opensource/zte-drivers/zte_adreno_overclock/adreno_overclock.ko",
            "adreno_overclock.ko",
        ),
    ]

    seen_names: set[str] = set()
    for src, dest_name in candidates:
        name = dest_name or src.name
        if name in seen_names:
            continue
        copied = copy_asset(src, release_dir, name)
        if copied:
            assets.append(copied)
            seen_names.add(name)

    if build_log and build_log.is_file() and "kernel-build.log" not in seen_names:
        copied = copy_asset(build_log, release_dir, "kernel-build.log")
        if copied:
            assets.append(copied)
            seen_names.add(copied.name)

    dtbs_archive = make_tar_from_dir(
        artifact_dir / "dtbs",
        release_dir / f"dtbs-{short_sha}.tar.gz",
        f"dtbs-{short_sha}",
    )
    if dtbs_archive:
        assets.append(dtbs_archive)

    opensource = release_dir / f"opensource-{short_sha}.tar.gz"
    run(
        [
            "git",
            "archive",
            "--format=tar.gz",
            f"--prefix=android_kernel_nubia_sm8850_qwjujube-{short_sha}/",
            "-o",
            str(opensource),
            "HEAD",
        ],
        cwd=repo_root,
    )
    assets.append(opensource)

    return sorted({asset.resolve() for asset in assets}, key=lambda path: path.name)


def delete_existing_asset(release: dict, token: str, asset_name: str) -> None:
    assets_url = release.get("assets_url")
    if not assets_url:
        return
    existing_assets = github_json("GET", assets_url, token)
    for existing in existing_assets:
        if existing.get("name") == asset_name:
            github_request("DELETE", existing["url"], token)


def upload_asset(upload_url_template: str, token: str, asset: Path) -> None:
    upload_url = upload_url_template.split("{", 1)[0]
    query = urllib.parse.urlencode({"name": asset.name})
    url = f"{upload_url}?{query}"
    content_type = mimetypes.guess_type(asset.name)[0] or "application/octet-stream"
    command = [
        "curl",
        "--fail",
        "--show-error",
        "--silent",
        "--location",
        "--retry",
        "3",
        "--retry-delay",
        "5",
        "--request",
        "POST",
        "--header",
        f"Authorization: Bearer {token}",
        "--header",
        "Accept: application/vnd.github+json",
        "--header",
        "X-GitHub-Api-Version: 2022-11-28",
        "--header",
        f"Content-Type: {content_type}",
        "--data-binary",
        f"@{asset}",
        url,
    ]
    subprocess.run(command, check=True)


def create_or_get_release(
    repository: str,
    token: str,
    tag_name: str,
    target_commitish: str,
    release_name: str,
    body: str,
) -> dict:
    api_root = f"https://api.github.com/repos/{repository}"
    create_payload = {
        "tag_name": tag_name,
        "target_commitish": target_commitish,
        "name": release_name,
        "body": body,
        "draft": False,
        "prerelease": False,
        "make_latest": "true",
    }
    try:
        return github_json("POST", f"{api_root}/releases", token, create_payload)
    except RuntimeError as error:
        if "already_exists" not in str(error) and "Validation Failed" not in str(error):
            raise
        encoded_tag = urllib.parse.quote(tag_name, safe="")
        return github_json("GET", f"{api_root}/releases/tags/{encoded_tag}", token)


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("artifact_dir", type=Path)
    parser.add_argument("--build-log", type=Path)
    parser.add_argument("--repository", default=os.environ.get("GITHUB_REPOSITORY", DEFAULT_REPOSITORY))
    parser.add_argument("--tag")
    parser.add_argument("--release-name")
    args = parser.parse_args()

    token = os.environ.get("GITHUB_TOKEN") or os.environ.get("GH_TOKEN")
    if not token:
        print("GITHUB_TOKEN or GH_TOKEN is required to publish a GitHub Release.", file=sys.stderr)
        return 2

    repo_root = Path(run(["git", "rev-parse", "--show-toplevel"])).resolve()
    artifact_dir = args.artifact_dir.resolve()
    if not artifact_dir.is_dir():
        print(f"Artifact directory not found: {artifact_dir}", file=sys.stderr)
        return 2

    full_sha = run(["git", "rev-parse", "HEAD"], cwd=repo_root)
    short_sha = full_sha[:12]
    timestamp = time.strftime("%Y%m%d-%H%M%S", time.gmtime())
    tag_name = args.tag or f"local-build-{timestamp}-{short_sha}"
    release_name = args.release_name or f"Local Build {timestamp} NX809J"
    release_dir = artifact_dir / "github-release"

    build_info = release_dir / "build-info.txt"
    release_dir.mkdir(parents=True, exist_ok=True)
    build_info.write_text(
        "\n".join(
            [
                "NX809J local kernel build",
                f"Repository: {args.repository}",
                f"Commit: {full_sha}",
                f"Tag: {tag_name}",
                f"Artifact directory: {artifact_dir}",
                f"Built/published at UTC: {time.strftime('%Y-%m-%dT%H:%M:%SZ', time.gmtime())}",
                "",
                "This release was compiled locally in the maintainer Docker environment, not by GitHub Actions.",
                "Assets include boot images, modules, kernel config, System.map, build log, DTBs, and the exact opensource tree.",
            ]
        )
        + "\n"
    )

    assets = collect_assets(repo_root, artifact_dir, release_dir, args.build_log, short_sha)
    if build_info not in assets:
        assets.append(build_info)
    assets = sorted({asset.resolve() for asset in assets}, key=lambda path: path.name)

    release = create_or_get_release(
        args.repository,
        token,
        tag_name,
        full_sha,
        release_name,
        build_info.read_text(),
    )

    print(f"[release] {release.get('html_url') or release.get('url')}", flush=True)
    for asset in assets:
        print(f"[release] uploading {asset.name} ({asset.stat().st_size} bytes)", flush=True)
        delete_existing_asset(release, token, asset.name)
        upload_asset(release["upload_url"], token, asset)

    print("[release] done", flush=True)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
