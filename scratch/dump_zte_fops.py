import struct
import sys
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]

def parse_elf(filename):
    with open(filename, 'rb') as f:
        elf_data = f.read()

    # Read ELF64 header
    e_shoff = struct.unpack('<Q', elf_data[40:48])[0]
    e_shentsize = struct.unpack('<H', elf_data[58:60])[0]
    e_shnum = struct.unpack('<H', elf_data[60:62])[0]
    e_shstrndx = struct.unpack('<H', elf_data[62:64])[0]

    sections = []
    for i in range(e_shnum):
        sh_offset = e_shoff + i * e_shentsize
        sh_hdr = elf_data[sh_offset : sh_offset + e_shentsize]
        sh_name, sh_type, sh_flags, sh_addr, sh_offset_file, sh_size = struct.unpack('<IIQQQQ', sh_hdr[:40])
        sections.append({
            'idx': i,
            'name_idx': sh_name,
            'type': sh_type,
            'flags': sh_flags,
            'addr': sh_addr,
            'offset': sh_offset_file,
            'size': sh_size
        })

    shstrtab_offset = sections[e_shstrndx]['offset']
    def get_sh_name(idx):
        end = shstrtab_offset + idx
        while elf_data[end] != 0:
            end += 1
        return elf_data[shstrtab_offset + idx : end].decode('utf-8')

    for s in sections:
        s['name'] = get_sh_name(s['name_idx'])

    symtab = next((s for s in sections if s['name'] == '.symtab'), None)
    strtab = next((s for s in sections if s['name'] == '.strtab'), None)
    rela_rodata = next((s for s in sections if s['name'] == '.rela.rodata'), None)

    strtab_data = elf_data[strtab['offset'] : strtab['offset'] + strtab['size']]
    def get_str(idx):
        if idx >= len(strtab_data):
            return ""
        end = idx
        while end < len(strtab_data) and strtab_data[end] != 0:
            end += 1
        return strtab_data[idx:end].decode('utf-8')

    symbols = []
    sym_entry_size = 24
    for offset in range(symtab['offset'], symtab['offset'] + symtab['size'], sym_entry_size):
        entry = elf_data[offset : offset + sym_entry_size]
        st_name, st_info, st_other, st_shndx, st_value, st_size = struct.unpack('<IBBHQQ', entry)
        symbols.append({
            'name': get_str(st_name),
            'value': st_value,
            'size': st_size,
            'shndx': st_shndx,
            'info': st_info
        })

    if rela_rodata:
        rela_entry_size = 24
        print("Relocations in .rodata around 0xf38:")
        for offset in range(rela_rodata['offset'], rela_rodata['offset'] + rela_rodata['size'], rela_entry_size):
            entry = elf_data[offset : offset + rela_entry_size]
            r_offset, r_info, r_addend = struct.unpack('<QQq', entry)
            # zte_fops starts at 0xf38. Let's dump relocations from 0xf38 to 0xf38 + 256
            if 0xf38 <= r_offset < 0xf38 + 256:
                sym_idx = r_info >> 32
                sym = symbols[sym_idx]
                print(f"Offset 0x{r_offset:04x} (offset from zte_fops: {r_offset - 0xf38}): sym_idx={sym_idx} name='{sym['name']}' val=0x{sym['value']:x} addend={r_addend}")

if __name__ == '__main__':
    module = Path(sys.argv[1]) if len(sys.argv) > 1 else ROOT / 'vendor/zte/zte_tpd/official_zte_tpd.ko'
    parse_elf(module)
