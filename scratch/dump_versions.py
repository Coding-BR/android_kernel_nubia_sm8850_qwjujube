import struct
import sys
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]

def dump_versions(filename):
    with open(filename, 'rb') as f:
        data = f.read()
        
    assert data[:4] == b'\x7fELF'
    e_shoff = struct.unpack('<Q', data[40:48])[0]
    e_shentsize = struct.unpack('<H', data[58:60])[0]
    e_shnum = struct.unpack('<H', data[60:62])[0]
    e_shstrndx = struct.unpack('<H', data[62:64])[0]
    
    shstrtab_offset = struct.unpack('<Q', data[e_shoff + e_shstrndx * e_shentsize + 24 : e_shoff + e_shstrndx * e_shentsize + 32])[0]
    
    versions_offset = None
    versions_size = None
    
    for i in range(e_shnum):
        sh_entry = e_shoff + i * e_shentsize
        sh_name_idx = struct.unpack('<I', data[sh_entry : sh_entry + 4])[0]
        name_offset = shstrtab_offset + sh_name_idx
        section_name = data[name_offset:].split(b'\x00', 1)[0].decode('utf-8', errors='ignore')
        
        if section_name == '__versions':
            versions_offset = struct.unpack('<Q', data[sh_entry + 24 : sh_entry + 32])[0]
            versions_size = struct.unpack('<Q', data[sh_entry + 32 : sh_entry + 40])[0]
            break
            
    if versions_offset is None:
        print("Could not find __versions section!")
        sys.exit(1)
        
    entry_size = 64
    num_entries = versions_size // entry_size
    
    print("OFFICIAL_CRCS = {")
    for i in range(num_entries):
        offset = versions_offset + i * entry_size
        crc, padding = struct.unpack('<II', data[offset : offset + 8])
        name = data[offset + 8 : offset + 64].split(b'\x00', 1)[0].decode('utf-8', errors='ignore')
        print(f"    \"{name}\": {hex(crc)},")
    print("}")

if __name__ == '__main__':
    module = Path(sys.argv[1]) if len(sys.argv) > 1 else ROOT / 'vendor/zte/zte_tpd/official_zte_tpd.ko'
    dump_versions(module)
