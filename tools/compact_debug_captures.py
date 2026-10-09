"""Compact debug capture evidence; creates a checked deletion plan, never deletes.

Run from the repository after creating build/cleanup-records/inventory-before.json.
Selected raw frames are ZIP-compressed losslessly; source and release files stay
outside the cleanup plan. Actual deletion uses the checked PowerShell executor.
"""
import hashlib
import json
import re
import zipfile
from collections import defaultdict
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
RECORDS = ROOT / 'build/cleanup-records'
INVENTORY = RECORDS / 'inventory-before.json'
STAMP = '20261008'


def main():
    inventory = json.loads(INVENTORY.read_text())
    assert Path(inventory['root']).resolve() == ROOT.resolve()
    groups = defaultdict(list)
    for f in inventory['files']:
        p = Path(f['path'])
        if p.parts[0] == 'build' and re.fullmatch(r'shot_\d+\.bmp', p.name, re.I):
            groups[p.parent].append(f)
    notes = '\n'.join(p.read_text(errors='replace') for p in
                      list((ROOT/'docs').rglob('*.md')) + [ROOT/'HANDOFF.md', ROOT/'PORTING.md'])
    named = {n.lower() for n in re.findall(r'\bshot_\d+\.bmp\b', notes, re.I)}
    plan = []
    evidence = []
    archives = []
    for i, (parent, files) in enumerate(sorted(groups.items()), 1):
        files.sort(key=lambda f: int(re.search(r'\d+', Path(f['path']).stem).group()))
        keep = {files[n]['path'] for n in {0, len(files)//2, len(files)-1}}
        keep.update(f['path'] for f in files if Path(f['path']).name.lower() in named)
        archive = ROOT/parent/f'capture-evidence-{STAMP}.zip'
        if archive.exists():
            raise RuntimeError(f'Refusing to overwrite {archive}')
        selected = [f for f in files if f['path'] in keep]
        with zipfile.ZipFile(archive, 'x', zipfile.ZIP_DEFLATED, compresslevel=6) as z:
            for f in selected:
                src = ROOT/f['path']
                assert src.stat().st_size == f['size']
                z.write(src, src.name)
        with zipfile.ZipFile(archive) as z:
            assert set(z.namelist()) == {Path(f['path']).name for f in selected}
            assert z.testzip() is None
        for f in files:
            plan.append({**f, 'reason': 'capture evidence archived' if f['path'] in keep else 'redundant automatic capture'})
        for f in selected:
            evidence.append({'original': f['path'], 'archive': str(archive.relative_to(ROOT)), 'entry': Path(f['path']).name})
        archives.append({'archive': str(archive.relative_to(ROOT)), 'frames': len(selected), 'bytes': archive.stat().st_size})
        if i % 25 == 0:
            print(f'Archived {i}/{len(groups)} capture folders; {len(evidence)} evidence frames verified.', flush=True)
    (RECORDS/'capture-delete-plan.json').write_text(json.dumps({'files': plan}))
    (RECORDS/'capture-evidence.json').write_text(json.dumps({'frames': evidence, 'archives': archives}, indent=2))
    print(f'Capture plan ready: {len(plan)} raw BMPs; {len(evidence)} frames preserved in {len(archives)} verified ZIPs ({sum(a["bytes"] for a in archives)/2**20:.1f} MiB).', flush=True)


if __name__ == '__main__':
    main()
