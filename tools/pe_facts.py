"""Phase 0: dump the facts-table inputs for every PE given on the command line.
Usage: python3 -P tools/pe_facts.py bin/player/*.exe bin/player/*.dll"""
import hashlib, sys, pefile

def facts(path):
    data = open(path, "rb").read()
    pe = pefile.PE(data=data)
    oh, fh = pe.OPTIONAL_HEADER, pe.FILE_HEADER
    print(f"=== {path}")
    print(f"sha256 {hashlib.sha256(data).hexdigest()}  size {len(data)}")
    print(f"machine {fh.Machine:#x} timestamp {fh.TimeDateStamp} linker {oh.MajorLinkerVersion}.{oh.MinorLinkerVersion}"
          f" imagebase {oh.ImageBase:#x} entry {oh.AddressOfEntryPoint:#x}"
          f" dynamicbase {bool(oh.DllCharacteristics & 0x40)} nx {bool(oh.DllCharacteristics & 0x100)}"
          f" subsystem {oh.Subsystem} osver {oh.MajorOperatingSystemVersion}.{oh.MinorOperatingSystemVersion}")
    for s in pe.sections:
        print(f"  sec {s.Name.rstrip(b'\\0').decode('latin1'):8} va {s.VirtualAddress:#010x} vsz {s.Misc_VirtualSize:#010x}"
              f" raw {s.SizeOfRawData:#010x} chr {s.Characteristics:#010x} entropy {s.get_entropy():.2f}")
    rich = pe.parse_rich_header()
    if rich:
        v = rich["values"]
        for i in range(0, len(v), 2):
            print(f"  rich prodid {v[i] >> 16:#06x} build {v[i] & 0xffff:5d} count {v[i + 1]}")
    else:
        print("  rich: none")
    print("  imports:", ", ".join(sorted(e.dll.decode() for e in getattr(pe, "DIRECTORY_ENTRY_IMPORT", []))))
    for fi in getattr(pe, "FileInfo", []) or []:
        for e in fi:
            for st in getattr(e, "StringTable", []):
                for k, val in st.entries.items():
                    print(f"  ver {k.decode(errors='replace')} = {val.decode(errors='replace')}")
    if hasattr(pe, "DIRECTORY_ENTRY_DEBUG"):
        for d in pe.DIRECTORY_ENTRY_DEBUG:
            print(f"  debug type {d.struct.Type} {getattr(d.entry, 'PdbFileName', b'')}")

for p in sys.argv[1:]:
    try:
        facts(p)
    except Exception as e:
        print(f"=== {p}\n  ERROR {e}")
