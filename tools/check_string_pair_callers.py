"""Caller-side check of string-class name proposals: for each string-unique / string-set row of
ida/exports/PLAYER-names-proposed.csv (or the classes given), test whether any named PLAYER caller
is also a caller (or caller of a caller) of the oracle function. A disagreement flags a possible
inlined-wrapper or platform pairing. Reads only the evidence exports.

  python3 -P tools/check_string_pair_callers.py string-unique,string-set
"""
import csv,json,collections,re,sys
def load(T):
    F={};by=collections.defaultdict(list);rc=collections.defaultdict(set)
    for l in open(f'ida/exports/{T}-evidence/functions.jsonl'):
        d=json.loads(l);F[d['address']]=d;by[d['name'].lstrip('_')].append(d['address'])
        for c in d['callees']: rc[c].add(d['address'])
    return F,by,rc
P,_,Prc=load('PLAYER'); O={'ios1029':load('IOS_1029'),'mac':load('MACSTUDIO')}
rows=list(csv.DictReader(open('ida/exports/PLAYER-names-proposed.csv')))
pname=collections.defaultdict(set)
for r in rows: pname[int(r['address'],16)].add(r['mangled'].lstrip('_'))
def canon(n): return re.sub(r'(?<=[0-9A-Za-z_])(C[1-3]|D[0-2])(?=E)','C*',n)
classes=sys.argv[1].split(',')
stats=collections.Counter(); bad=[]
for r in rows:
    if r['evidence_class'] not in classes: continue
    p=int(r['address'],16)
    pc={n for c in Prc[p] for n in pname.get(c,())}
    if not pc: stats['no named caller']+=1; continue
    for tag in (['ios1029','mac'] if r['oracle']=='both' else [r['oracle']]):
        F,by,rc=O[tag]
        oc=set()
        for oa in by.get(r['mangled'].lstrip('_'),[]):
            for c in rc[oa]:
                n=F[c]['name'].lstrip('_'); oc.add(canon(n))
                for c2 in rc[c]: oc.add(canon(F[c2]['name'].lstrip('_')))  # wrappers / thunks
        pcc={canon(x) for x in pc}
        if pcc & oc: stats['caller agrees']+=1
        else:
            stats['caller disagrees']+=1; bad.append((r['address'],tag,r['demangled'][:80],sorted(pc)[:2]))
print(stats)
for b in bad[:60]: print(b)
