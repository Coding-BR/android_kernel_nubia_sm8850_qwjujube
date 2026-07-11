import sys
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]


def main():
    log_path = Path(sys.argv[1]) if len(sys.argv) > 1 else ROOT / "new-console-ramoops-clean.txt"
    output_path = Path(sys.argv[2]) if len(sys.argv) > 2 else ROOT / "scratch/t1_lines.txt"
    with open(log_path, 'r', errors='ignore') as f:
        lines = f.readlines()
        
    t1_lines = []
    for i, line in enumerate(lines):
        if "T1]" in line:
            t1_lines.append(f"L{i+1}: {line.strip()}")
            
    with open(output_path, "w") as out:
        out.write("\n".join(t1_lines))
        
    print(f"Extracted {len(t1_lines)} lines to scratch/t1_lines.txt")

if __name__ == '__main__':
    main()
