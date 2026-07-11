import re
import sys
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]

def main():
    log_path = Path(sys.argv[1]) if len(sys.argv) > 1 else ROOT / "new-console-ramoops-clean.txt"
    with open(log_path, 'r', errors='ignore') as f:
        lines = f.readlines()
    
    print(f"Total lines: {len(lines)}")
    
    # 1. Print all occurrences of 'insmod' or 'failed' or 'duplicate'
    interesting_patterns = [
        re.compile(r'insmod', re.IGNORECASE),
        re.compile(r'duplicate', re.IGNORECASE),
        re.compile(r'verification failed', re.IGNORECASE),
        re.compile(r'exec format error', re.IGNORECASE),
        re.compile(r'InitFatalReboot', re.IGNORECASE),
    ]
    
    found_any = False
    for i, line in enumerate(lines):
        for pattern in interesting_patterns:
            if pattern.search(line):
                print(f"Line {i+1}: {line.strip()}")
                found_any = True
                break
                
    if not found_any:
        print("No matching patterns found.")

if __name__ == '__main__':
    main()
