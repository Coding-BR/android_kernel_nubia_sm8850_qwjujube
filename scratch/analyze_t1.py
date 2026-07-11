import sys
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]


def main():
    log_path = Path(sys.argv[1]) if len(sys.argv) > 1 else ROOT / "new-console-ramoops-clean.txt"
    with open(log_path, 'r', errors='ignore') as f:
        lines = f.readlines()
        
    t1_lines = []
    for i, line in enumerate(lines):
        if "[    T1]" in line or "[   T1]" in line or "T1]" in line:
            # Let's filter out standard module lists and warning call traces if they are redundant,
            # but let's keep them if they are short.
            t1_lines.append((i+1, line.strip()))
            
    print(f"Total T1 lines found: {len(t1_lines)}")
    # Print the first 100 and last 100
    for idx, line in t1_lines[:100]:
        print(f"L{idx}: {line}")
    if len(t1_lines) > 200:
        print("...")
        for idx, line in t1_lines[-100:]:
            print(f"L{idx}: {line}")

if __name__ == '__main__':
    main()
