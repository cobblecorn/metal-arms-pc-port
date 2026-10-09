#!/usr/bin/env python3
"""Build the release zip: the port's program files only, never retail data.

Players run ma_setup.exe to extract the game files from their own disc. The zip is checked
before it is written: anything that looks like retail data (disc images, master files, audio
banks, movies, textures, extracted folders) stops the build.

    python tools/package_release.py [--version 1.0.0] [--out <zip>]

The zip holds one folder, "Metal Arms PC Port", so players can extract it anywhere.
"""
import argparse
import pathlib
import re
import sys
import zipfile

ROOT = pathlib.Path(__file__).resolve().parent.parent
RELEASE = ROOT / 'build' / 'Release'

FOLDER = 'Metal Arms PC Port'

# Published path in the zip -> source file. Nothing else goes in.
FILES = {
    'ma_port.exe': RELEASE / 'ma_port.exe',
    'ma_setup.exe': RELEASE / 'ma_setup.exe',
    'binkw32.dll': RELEASE / 'binkw32.dll',
    'README.txt': ROOT / 'port' / 'setup' / 'README.txt',
}

RETAIL = re.compile(r'(gamedata|\.mst$|\.wvs$|\.bik$|\.tga$|\.dol$|\.iso$|\.gcm$|\.rvz$|\.gcz$|\.wia$|'
                    r'\.ciso$|\.csv$|\.sav$|^sys/|^files/|movies/)', re.IGNORECASE)
MAX_FILE_BYTES = 32 << 20	# the program files are a few MB; retail archives are hundreds


def main():
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument('--version', default='1.0.0')
    parser.add_argument('--out', type=pathlib.Path)
    args = parser.parse_args()
    if not args.out:
        args.out = ROOT / 'build' / 'dist' / f'MetalArmsPC-{args.version}.zip'

    problems = []
    for name, source in FILES.items():
        if not source.is_file():
            problems.append(f'{name}: missing ({source}); build the Release configuration first')
        elif RETAIL.search(name) or RETAIL.search(source.name):
            problems.append(f'{name}: looks like retail data')
        elif source.stat().st_size > MAX_FILE_BYTES:
            problems.append(f'{name}: {source.stat().st_size >> 20} MB is too large for a program file')
    if problems:
        sys.exit('Release not built:\n  ' + '\n  '.join(problems))

    args.out.parent.mkdir(parents=True, exist_ok=True)
    with zipfile.ZipFile(args.out, 'w', zipfile.ZIP_DEFLATED, compresslevel=9) as archive:
        for name, source in FILES.items():
            archive.write(source, f'{FOLDER}/{name}')

    with zipfile.ZipFile(args.out) as archive:
        names = archive.namelist()
    if sorted(names) != sorted(f'{FOLDER}/{n}' for n in FILES) or any(RETAIL.search(n) for n in names):
        args.out.unlink()
        sys.exit(f'Release check failed; zip removed. Contents were: {names}')
    print(f'{args.out} ({args.out.stat().st_size / 2**20:.1f} MB):')
    for name in names:
        print(f'  {name}')


if __name__ == '__main__':
    main()
