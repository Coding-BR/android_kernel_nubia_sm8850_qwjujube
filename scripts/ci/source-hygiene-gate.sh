#!/usr/bin/env bash
set -euo pipefail

ROOT="$(git rev-parse --show-toplevel)"
cd "$ROOT"

failures=0

while read -r mode object stage path; do
  [[ "$mode" == "120000" ]] || continue
  target="$(git cat-file -p "$object")"
  if [[ "$target" == /* ]]; then
    printf '[SOURCE_HYGIENE_FAILED] absolute tracked symlink: %s -> %s\n' "$path" "$target" >&2
    failures=1
    continue
  fi
  if [[ ! -e "$(dirname "$path")/$target" ]]; then
    printf '[SOURCE_HYGIENE_FAILED] broken tracked symlink: %s -> %s\n' "$path" "$target" >&2
    failures=1
  fi
done < <(git ls-files -s)

while IFS= read -r path; do
  case "$path" in
    kernel/prebuilts/*/System.map)
      continue
      ;;
    *.o|*.o.d|*.cmd|*/vmlinux|*/vmlinux.o|*/Module.symvers|*/modules.order)
      printf '[SOURCE_HYGIENE_FAILED] tracked build artifact: %s\n' "$path" >&2
      failures=1
      ;;
    *.ko)
      case "$path" in
        stock_rom_modules/modules/*.ko|evidence/stock-modules/*.ko|evidence/stock-modules/*/*.ko)
          ;;
        *)
          printf '[SOURCE_HYGIENE_FAILED] unclassified tracked kernel module: %s\n' "$path" >&2
          failures=1
          ;;
      esac
      ;;
  esac
done < <(git ls-files)

oversized="$(git ls-tree -r -l HEAD | awk '$4 != "-" && $4 >= 50 * 1024 * 1024 {print $4 "\t" substr($0, index($0, "\t") + 1)}')"
if [[ -n "$oversized" ]]; then
  printf '[SOURCE_HYGIENE_FAILED] tracked blobs at or above 50 MiB:\n%s\n' "$oversized" >&2
  failures=1
fi

base="${1:-}"
if [[ -z "$base" || "$base" =~ ^0+$ ]] || ! git cat-file -e "$base^{commit}" 2>/dev/null; then
  if git show-ref --verify --quiet refs/remotes/upstream/main; then
    base="$(git merge-base HEAD upstream/main)"
  elif git rev-parse --verify HEAD^ >/dev/null 2>&1; then
    base="HEAD^"
  else
    base="HEAD"
  fi
fi

if [[ "$base" != "HEAD" ]]; then
  git diff --check "$base..HEAD"
fi

python3 scripts/ci/check-source-provenance.py
python3 scripts/toolchains/resolve_android_clang.py --metadata-only

stale_host_paths="$(git grep -n -I -E '(/home/[^/[:space:]]+/|file:///home/)' -- \
  README.md README_PT.md COMPILATION_GUIDE.md COMPILATION_GUIDE_PT.md \
  NEXT_STEPS.md REVERSE_ENGINEERING_MASTER_PLAN.md super_build.sh scratch \
  'vendor/qcom/opensource/*/Makefile' || true)"
if [[ -n "$stale_host_paths" ]]; then
  printf '[SOURCE_HYGIENE_FAILED] host-specific absolute paths:\n%s\n' "$stale_host_paths" >&2
  failures=1
fi

if (( failures != 0 )); then
  exit 1
fi

printf '[SOURCE_HYGIENE_OK] artifacts, symlinks, host paths, toolchain lock, blob sizes, diff, and provenance verified\n'
