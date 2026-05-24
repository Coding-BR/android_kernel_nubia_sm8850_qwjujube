import os
import struct

def main():
    vmlinux_path = "kernel_platform/common/vmlinux"
    if not os.path.exists(vmlinux_path):
        print(f"Error: {vmlinux_path} not found")
        return

    # We search for the pattern of instructions
    # Pattern 1 (Crash 4): ret, tbz, str, ldur, str
    # Hex: c0 03 5f d6   c2 00 18 36   26 00 00 f9   87 80 5f f8   06 06 00 f9
    pattern1 = b"\xc0\x03\x5f\xd6\xc2\x00\x18\x36\x26\x00\x00\xf9"

    # Pattern 2 (Crash 7): tbz, ldr, ?, stlur
    # Hex: c2 00 18 36   22 00 40 f9
    pattern2 = b"\xc2\x00\x18\x36\x22\x00\x40\xf9"

    print("Reading vmlinux...")
    with open(vmlinux_path, "rb") as f:
        data = f.read()

    print(f"vmlinux size: {len(data)} bytes")

    # Search for Pattern 1
    idx = data.find(pattern1)
    if idx != -1:
        print(f"Found Pattern 1 at offset 0x{idx:x}")
        # Let's find symbol
        find_symbol_near_offset(vmlinux_path, idx)
    else:
        print("Pattern 1 not found")

    # Search for Pattern 2
    idx2 = data.find(pattern2)
    if idx2 != -1:
        print(f"Found Pattern 2 at offset 0x{idx2:x}")
        find_symbol_near_offset(vmlinux_path, idx2)
    else:
        print("Pattern 2 not found")

def find_symbol_near_offset(vmlinux_path, offset):
    # We can run nm to list symbols with addresses, or we can use readelf to locate the section and map offset to virtual address.
    # But wait, nm output lists virtual addresses.
    # To map offset in vmlinux file to virtual address:
    # Let's run readelf -S to get the section offsets and virtual addresses.
    import subprocess
    cmd = ["readelf", "-S", vmlinux_path]
    res = subprocess.run(cmd, capture_output=True, text=True)
    
    # We look for the section containing the offset
    # Typically it is .text
    text_addr = None
    text_offset = None
    for line in res.stdout.splitlines():
        if ".text" in line and "PROGBITS" in line:
            # Format: [Nr] Name              Type             Address           Offset
            # Line example:  [ 2] .text             PROGBITS         ffffffc080090000  00090000
            parts = line.split()
            for i, p in enumerate(parts):
                if p == ".text":
                    text_addr = int(parts[i+2], 16)
                    text_offset = int(parts[i+3], 16)
                    break
            break

    if text_addr is not None and text_offset is not None:
        # Map offset to virtual address
        virt_addr = text_addr + (offset - text_offset)
        print(f"Mapped offset 0x{offset:x} to virtual address 0x{virt_addr:x}")
        
        # Now run nm to find the symbol closest to this address
        cmd_nm = ["nm", "-n", vmlinux_path]
        res_nm = subprocess.run(cmd_nm, capture_output=True, text=True)
        prev_sym = None
        prev_addr = 0
        for line in res_nm.stdout.splitlines():
            parts = line.split()
            if len(parts) >= 3:
                try:
                    addr = int(parts[0], 16)
                    sym = parts[2]
                    if addr > virt_addr:
                        if prev_sym:
                            diff = virt_addr - prev_addr
                            print(f"Closest symbol: {prev_sym} at 0x{prev_addr:x} (offset +0x{diff:x})")
                        break
                    prev_sym = sym
                    prev_addr = addr
                except ValueError:
                    pass
    else:
        print("Could not map offset to virtual address (could not parse .text section)")

if __name__ == "__main__":
    main()
