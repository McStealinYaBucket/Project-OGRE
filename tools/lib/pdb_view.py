"""Dump what reccmp sees in a recompiled PDB: one row per entity reccmp creates from it.

Run with the reccmp venv python (it imports reccmp; cvdump runs under wine, a few seconds):
    toolchain/venv/bin/python -I tools/lib/pdb_view.py build/RobloxPlayerBeta.exe > view.tsv

Columns (tab separated): address (hex), type (reccmp EntityType name or -), decorated (PUBLICS name or -),
name (reccmp's name() for the node or -). reccmp's match_symbols pairs a CSV `symbol` with `decorated`,
match_functions/match_variables pair a CSV `name` with `name` (compare/match_msvc.py), so a CSV built
from this view pairs exactly. An ICF-folded address keeps only one decorated name here (the last
PUBLICS entry), which is why tools/ida/gen_reccmp_csv.py reads this instead of the link map.
"""
import sys

from reccmp.cvdump import Cvdump, CvdumpAnalysis
from reccmp.formats import detect_image


def main(exe):
    img = detect_image(exe)
    dump = Cvdump(exe[:-4] + ".pdb").globals().publics().symbols().section_contributions().run()
    for n in CvdumpAnalysis(dump, img).nodes:
        t = n.node_type.name if n.node_type is not None else "-"
        print(f"{n.addr:x}\t{t}\t{n.decorated_name or '-'}\t{n.name() or '-'}")


if __name__ == "__main__":
    main(sys.argv[1])
