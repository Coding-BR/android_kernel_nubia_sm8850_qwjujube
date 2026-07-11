import struct
import sys
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]

def extract_kcfi(filename):
    with open(filename, 'rb') as f:
        data = f.read()
        
    assert data[:4] == b'\x7fELF'
    e_shoff = struct.unpack('<Q', data[40:48])[0]
    e_shentsize = struct.unpack('<H', data[58:60])[0]
    e_shnum = struct.unpack('<H', data[60:62])[0]
    e_shstrndx = struct.unpack('<H', data[62:64])[0]
    
    shstrtab_offset = struct.unpack('<Q', data[e_shoff + e_shstrndx * e_shentsize + 24 : e_shoff + e_shstrndx * e_shentsize + 32])[0]
    
    sections = {}
    symtab_offset = None
    symtab_size = None
    strtab_offset = None
    
    for i in range(e_shnum):
        entry = e_shoff + i * e_shentsize
        sh_name_idx = struct.unpack('<I', data[entry : entry + 4])[0]
        sh_type = struct.unpack('<I', data[entry + 4 : entry + 8])[0]
        sh_offset = struct.unpack('<Q', data[entry + 24 : entry + 32])[0]
        sh_size = struct.unpack('<Q', data[entry + 32 : entry + 40])[0]
        
        name_offset = shstrtab_offset + sh_name_idx
        section_name = data[name_offset:].split(b'\x00', 1)[0].decode('utf-8', errors='ignore')
        
        sections[i] = {
            'offset': sh_offset,
            'size': sh_size,
            'name': section_name
        }
        
        if sh_type == 2: # SHT_SYMTAB
            symtab_offset = sh_offset
            symtab_size = sh_size
            sh_link = struct.unpack('<I', data[entry + 40 : entry + 44])[0]
            strtab_entry = e_shoff + sh_link * e_shentsize
            strtab_offset = struct.unpack('<Q', data[strtab_entry + 24 : strtab_entry + 32])[0]

    if not symtab_offset or not strtab_offset:
        print("Could not find symtab or strtab!")
        return

    sym_entry_size = 24
    num_symbols = symtab_size // sym_entry_size
    
    kcfi_hashes = {}
    
    for i in range(num_symbols):
        offset = symtab_offset + i * sym_entry_size
        st_name = struct.unpack('<I', data[offset : offset + 4])[0]
        st_info = data[offset + 4]
        st_shndx = struct.unpack('<H', data[offset + 6 : offset + 8])[0]
        st_value = struct.unpack('<Q', data[offset + 8 : offset + 16])[0]
        st_size = struct.unpack('<Q', data[offset + 16 : offset + 24])[0]
        
        name = data[strtab_offset + st_name:].split(b'\x00', 1)[0].decode('utf-8', errors='ignore')
        
        # Check if it is a function (STT_FUNC)
        if (st_info & 0xf) == 2 and st_size > 0:
            if st_shndx in sections:
                sec = sections[st_shndx]
                if sec['name'] in ('.text', '.init.text', '.exit.text'):
                    if st_value >= 4:
                        func_file_offset = sec['offset'] + st_value
                        hash_file_offset = func_file_offset - 4
                        
                        # Read KCFI hash (4 bytes)
                        kcfi_hash = struct.unpack('<I', data[hash_file_offset : hash_file_offset + 4])[0]
                        
                        # A valid KCFI hash is normally non-zero and doesn't look like code (usually 0xXXXXXXXX)
                        # Let's collect it
                        kcfi_hashes[name] = kcfi_hash

    # Print out as python dictionary
    print("KCFI_HASHES = {")
    for name, val in sorted(kcfi_hashes.items()):
        print(f"    \"{name}\": {hex(val)},")
    print("}")

if __name__ == '__main__':
    module = Path(sys.argv[1]) if len(sys.argv) > 1 else ROOT / 'vendor/zte/zte_tpd/official_zte_tpd.ko'
    extract_kcfi(module)
