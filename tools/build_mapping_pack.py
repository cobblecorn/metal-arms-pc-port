#!/usr/bin/env python3
"""Build a game-named texture pack (for the port's textures folder) from the Metal Arms Mapping Project.

The Mapping Project (zstorm4) has two halves:
- GM5E7D/: Dolphin custom textures, named tex1_WxH[_m]_<hash>_<fmt>.png, where <hash> is XXH64 of the
  texture's base level as it sits in GameCube memory. Each GameCube texture in gamedata/mst is hashed
  the same way, so these files are renamed to the game's texture names.
- DEV DUMP/: images already named after game textures (TGA keeps alpha; PNG copies often lack it).

For each game texture the pack takes the Dolphin-mapped image if there is one (the curated, mapped
work), else the DEV DUMP image (TGA, then PNG, BMP, JPG). Only images named for a real game texture,
of the same shape, are kept. Writes <out>/textures/... plus README.txt and report.json.

    python tools/build_mapping_pack.py --pack "<Mapping Project>" --out build/dist/texture-pack [--check]
"""
import argparse
import json
import os
import pathlib
import re
import shutil
import struct
import sys

ROOT = pathlib.Path(__file__).resolve().parent.parent
MASK = (1 << 64) - 1
P1, P2, P3, P4, P5 = (0x9E3779B185EBCA87, 0xC2B2AE3D27D4EB4F, 0x165667B19E3779F9,
                      0x85EBCA77C2B2AE63, 0x27D4EB2F165667C5)


def _rotl(x, r):
    return ((x << r) | (x >> (64 - r))) & MASK


def _round(acc, lane):
    acc = (acc + lane * P2) & MASK
    return (_rotl(acc, 31) * P1) & MASK


def _merge(acc, val):
    acc ^= _round(0, val)
    return (acc * P1 + P4) & MASK


def xxh64(data, seed=0):
    """XXH64, as Dolphin names custom textures with (seed 0)."""
    n = len(data)
    i = 0
    if n >= 32:
        v1 = (seed + P1 + P2) & MASK
        v2 = (seed + P2) & MASK
        v3 = seed
        v4 = (seed - P1) & MASK
        limit = n - 32
        lanes = struct.unpack_from('<%dQ' % ((limit // 32 + 1) * 4), data, 0)
        for k in range(0, len(lanes), 4):
            v1 = _round(v1, lanes[k])
            v2 = _round(v2, lanes[k + 1])
            v3 = _round(v3, lanes[k + 2])
            v4 = _round(v4, lanes[k + 3])
        i = len(lanes) * 8
        h = (_rotl(v1, 1) + _rotl(v2, 7) + _rotl(v3, 12) + _rotl(v4, 18)) & MASK
        h = _merge(h, v1)
        h = _merge(h, v2)
        h = _merge(h, v3)
        h = _merge(h, v4)
    else:
        h = (seed + P5) & MASK
    h = (h + n) & MASK
    while i + 8 <= n:
        (k1,) = struct.unpack_from('<Q', data, i)
        h ^= _round(0, k1)
        h = (_rotl(h, 27) * P1 + P4) & MASK
        i += 8
    if i + 4 <= n:
        (k1,) = struct.unpack_from('<I', data, i)
        h ^= (k1 * P1) & MASK
        h = (_rotl(h, 23) * P2 + P3) & MASK
        i += 4
    while i < n:
        h ^= (data[i] * P5) & MASK
        h = (_rotl(h, 11) * P1) & MASK
        i += 1
    h ^= h >> 33
    h = (h * P2) & MASK
    h ^= h >> 29
    h = (h * P3) & MASK
    h ^= h >> 32
    return h


assert xxh64(b'') == 0xEF46DB3751D8E999, 'XXH64 self-test failed'

# gcdata.cpp's _GCTileBytes(): (tile across, tile down, bytes per tile) for each Fang GameCube format.
TILES = {0: (4, 4, 64), 1: (4, 4, 64), 2: (4, 4, 32), 3: (4, 4, 32), 4: (4, 4, 32), 9: (4, 4, 32),
         5: (8, 8, 32), 6: (8, 8, 32), 7: (8, 8, 32), 11: (8, 8, 32), 8: (8, 4, 32), 10: (8, 4, 32)}
MIN_DIM = {5: 8, 6: 8, 7: 8}
HEADER = 32
IMAGE_EXTS = {'.tga': 0, '.png': 1, '.bmp': 2, '.jpg': 3, '.jpeg': 3}


def level_bytes(fmt, w, h):
    across, down, per = TILES[fmt]
    return ((w + across - 1) // across) * ((h + down - 1) // down) * per


def game_textures(mst_dir):
    """name -> (width, height, [level-0 hashes]) for every GameCube texture."""
    out = {}
    for path in sorted(mst_dir.glob('*.tga')):
        data = path.read_bytes()
        if len(data) < HEADER:
            continue
        name = data[:16].split(b'\0', 1)[0].decode('latin-1').lower()
        fmt, lods = data[20], data[23]
        w, h = struct.unpack_from('>HH', data, 28)
        if fmt not in TILES or not w or not h or not name:
            continue
        hashes = []
        first = level_bytes(fmt, w, h)
        hashes.append(xxh64(data[HEADER:HEADER + first]))
        if fmt == 7:
            # the alpha plane is its own CMPR texture to Dolphin, after all colour mips
            total, lw, lh = 0, w, h
            for _ in range(max(lods, 1)):
                total += level_bytes(fmt, lw, lh)
                lw, lh = max(lw // 2, MIN_DIM.get(fmt, 1)), max(lh // 2, MIN_DIM.get(fmt, 1))
            hashes.append(xxh64(data[HEADER + total:HEADER + total + first]))
        out[name] = (w, h, hashes)
    return out


DOLPHIN_NAME = re.compile(r'^tex1_(\d+)x(\d+)(_m)?_([0-9a-f]{16})(?:_([0-9a-f]{16}|\$))?_(\d+)(?:_mip\d+)?(?:_arb)?$', re.I)


def image_size(path):
    from PIL import Image
    with Image.open(path) as im:
        return im.size


def main():
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument('--pack', type=pathlib.Path, required=True, help='the Metal Arms Mapping Project folder')
    parser.add_argument('--mst', type=pathlib.Path, default=ROOT / 'gamedata' / 'mst')
    parser.add_argument('--out', type=pathlib.Path, default=ROOT / 'build' / 'dist' / 'texture-pack')
    parser.add_argument('--check', action='store_true', help='only report how many images match')
    parser.add_argument('--zip', type=pathlib.Path, help='also write this zip: textures/... and README.txt')
    args = parser.parse_args()

    print('Hashing GameCube textures...', flush=True)
    textures = game_textures(args.mst)
    by_hash = {}
    for name, (_w, _h, hashes) in textures.items():
        for index, value in enumerate(hashes):
            by_hash.setdefault(value, []).append((name, index))
    print(f'  {len(textures)} textures')

    # Dolphin half
    dolphin = {}   # game name -> (path, size)
    unmatched = []
    alpha_planes = 0
    for path in sorted((args.pack / 'GM5E7D').rglob('*')):
        if path.suffix.lower() not in IMAGE_EXTS:
            continue
        m = DOLPHIN_NAME.match(path.stem)
        if not m:
            unmatched.append(str(path.relative_to(args.pack)))
            continue
        if m.group(4) and m.group(0).endswith(tuple(f'_mip{i}' for i in range(1, 16))):
            continue
        hits = by_hash.get(int(m.group(4), 16))
        if not hits:
            unmatched.append(str(path.relative_to(args.pack)))
            continue
        for name, index in hits:
            if (int(m.group(1)), int(m.group(2))) != textures[name][:2]:
                continue    # identical data (a flat colour) at another size: Dolphin names include the size
            if index:
                alpha_planes += 1
                continue
            size = image_size(path)
            if name not in dolphin or size[0] > dolphin[name][1][0]:
                dolphin[name] = (path, size)
    print(f'  Dolphin-named: {len(dolphin)} matched game textures, {len(unmatched)} unmatched, '
          f'{alpha_planes} separate alpha planes (not used)')

    # DEV DUMP half
    dev = {}
    for path in (args.pack / 'DEV DUMP').rglob('*'):
        rank = IMAGE_EXTS.get(path.suffix.lower())
        name = path.stem.lower()
        if rank is None or name not in textures:
            continue
        if name not in dev or rank < IMAGE_EXTS[dev[name].suffix.lower()]:
            dev[name] = path
    print(f'  DEV DUMP: {len(dev)} images named after game textures')

    chosen, refused = {}, []
    for name in sorted(set(dolphin) | set(dev)):
        w, h = textures[name][:2]
        candidates = ([dolphin[name][0]] if name in dolphin else []) + ([dev[name]] if name in dev else [])
        for path in candidates:
            try:
                iw, ih = image_size(path)
            except Exception:
                refused.append((name, str(path), 'unreadable'))
                continue
            if abs(iw * h - ih * w) > max(w, h):
                refused.append((name, str(path), f'{iw}x{ih} vs original {w}x{h}'))
                continue
            chosen[name] = (path, (iw, ih), 'dolphin' if name in dolphin and path == dolphin[name][0] else 'devdump')
            break
    bigger = sum(1 for name, (_p, (iw, _ih), _s) in chosen.items() if iw > textures[name][0])
    print(f'Pack: {len(chosen)} textures ({bigger} higher resolution than the GameCube), {len(refused)} refused')
    if args.check:
        return

    target = args.out / 'textures'
    if target.exists():
        shutil.rmtree(target)
    total = 0
    for name, (path, _size, source) in chosen.items():
        dest = target / ('dolphin-mapped' if source == 'dolphin' else 'dev-dump') / (name + path.suffix.lower())
        dest.parent.mkdir(parents=True, exist_ok=True)
        shutil.copyfile(path, dest)
        total += dest.stat().st_size
    report = {
        'textures': len(chosen),
        'higher_resolution': bigger,
        'from_dolphin_mapped': sum(1 for v in chosen.values() if v[2] == 'dolphin'),
        'from_dev_dump': sum(1 for v in chosen.values() if v[2] == 'devdump'),
        'refused': refused,
        'unmatched_dolphin_files': unmatched,
    }
    (args.out / 'report.json').write_text(json.dumps(report, indent=1))
    print(f'Wrote {len(chosen)} images ({total / 2**20:.0f} MB) to {target}')
    if args.zip:
        import zipfile
        args.zip.parent.mkdir(parents=True, exist_ok=True)
        with zipfile.ZipFile(args.zip, 'w', zipfile.ZIP_DEFLATED, compresslevel=6) as archive:
            archive.write(ROOT / 'port' / 'setup' / 'texture_pack_README.txt', 'README - Mapping Project textures.txt')
            for path in sorted(target.rglob('*')):
                if path.is_file():
                    archive.write(path, path.relative_to(args.out).as_posix())
        print(f'Zip: {args.zip} ({args.zip.stat().st_size / 2**20:.0f} MB)')


if __name__ == '__main__':
    main()
