"""Pack the reviewed yellow RAT atlases for the existing PC port; no game launch.

Uses an independent MST selected by -mst, preserving the retail archive and EXE.
The generated colors retain the original texture's material alpha. This is file
format conversion and mip generation, not an additional creative repaint.
"""
from pathlib import Path
import hashlib
import json
import shutil
import struct
import subprocess
import sys
import time
import zlib

import numpy as np
from PIL import Image

from mst_list import read_mst
from test_coop_checkpoint_rat import method

ROOT = Path(__file__).resolve().parents[1]
ART = ROOT / 'build/export/RAT-HD-yellow'
OUT = ROOT / 'build/export/Yellow_RAT_HD_Test'
DATA = ROOT / 'gamedata/files'
BASE = DATA / 'mettlearms_gc.mst'
ARCHIVE = DATA / 'rat_hd_yellow.mst'
ASSETS = (('trdrrata_01', (1024, 512)), ('trdrrata_02', (512, 512)))


def digest(path):
    with path.open('rb') as handle:
        return hashlib.file_digest(handle, 'sha256').hexdigest()


def tiled_rgba(image):
    """GC RGBA8: 4x4 blocks, A/R plane then G/B plane; row-major blocks."""
    pixels = np.asarray(image, dtype=np.uint8)
    h, w, _ = pixels.shape
    assert w % 4 == 0 and h % 4 == 0
    tiles = pixels.reshape(h // 4, 4, w // 4, 4, 4).transpose(0, 2, 1, 3, 4)
    ar = tiles[..., [3, 0]].reshape(h // 4, w // 4, 32)
    gb = tiles[..., [1, 2]].reshape(h // 4, w // 4, 32)
    return np.concatenate((ar, gb), axis=-1).tobytes()


def pack_texture(name, size):
    source = ROOT / 'build/export/RAT-original-textures' / (name + '.tga')
    header = bytearray(source.read_bytes()[:32])
    assert header[20] == 7
    color = Image.open(ART / (name + '-hd-preview.png')).convert('RGB')
    color = color.resize(size, Image.Resampling.LANCZOS)
    alpha = Image.open(ART / (name + '-original-alpha.png')).convert('L')
    assert size == (alpha.width * 4, alpha.height * 4)
    # Preserve every original alpha sample exactly across its 4x4 replacement.
    alpha = alpha.resize(size, Image.Resampling.NEAREST)
    image = color.copy()
    image.putalpha(alpha)
    image.save(OUT / (name + '-runtime.png'))
    lod_count = header[23] + 2
    levels = []
    mip = image
    for _ in range(lod_count):
        assert min(mip.size) >= 8
        levels.append(mip)
        mip = mip.resize((mip.width // 2, mip.height // 2), Image.Resampling.BOX)
    header[16:20] = bytes(4)
    header[20:24] = bytes((0, 0, 0, lod_count))
    struct.pack_into('>HH', header, 28, *size)
    blob = bytes(header) + b''.join(tiled_rgba(level) for level in levels)
    path = OUT / (name + '.tga')
    path.write_bytes(blob)
    return path, levels


def decoder():
    """Compile the actual production header validator and full GC decoder."""
    work = ROOT / 'build/rat-hd-decoder'
    work.mkdir(parents=True, exist_ok=True)
    source = (ROOT / 'port/gcdata.cpp').read_text()
    code = r'''
#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <cstring>
#include <vector>
using u8=uint8_t;using u16=uint16_t;using u32=uint32_t;using BOOL=int;
constexpr BOOL TRUE=1,FALSE=0;
#define FANG_PLATFORM_WIN 1
#define FANG_WINGC 1
#define DEVPRINTF(...) std::fprintf(stderr,__VA_ARGS__)
constexpr u32 FGCDATA_TEXFMT_COUNT=12,FTEX_FMT_A8R8G8B8=0,FTEX_PALFMT_NONE=0,FTEX_FLAG_NONE=0;
// This offline harness uses the native 32-byte disk header with a 32-bit pointer.
struct FTexInfo_t {
 char szName[16];u32 pUserData;
 u8 nTexFmt,nPalFmt,nFlags,nLodCount,nStencil,nDepth,reserved[2];
 u16 nTexelsAcross,nTexelsDown;
 void ChangeEndian(){nTexelsAcross=(nTexelsAcross>>8)|(nTexelsAcross<<8);nTexelsDown=(nTexelsDown>>8)|(nTexelsDown<<8);}
};
static_assert(sizeof(FTexInfo_t)==32,"disk header");
void* fmem_Alloc(u32 bytes,u32){return std::malloc(bytes);}
'''
    for signature in ('static u16 _ReadBE16(', 'static u8 _Expand5(',
                      'static u8 _Expand6(', 'static u8 _Expand4(',
                      'static u32 _PackArgb(', 'static u32 _GCTileBytes(',
                      'static void _DecodeCmprPlane(', 'static BOOL _DecodeUncompressedGCTile(',
                      'BOOL gcdata_DecodeTga('):
        code += method(source, signature) + '\n'
    code += r'''
int main(int argc,char** argv){
 if(argc!=3)return 2;
 FILE* f=std::fopen(argv[1],"rb");if(!f)return 3;
 std::fseek(f,0,SEEK_END);long size=std::ftell(f);std::rewind(f);
 std::vector<u8> input(size);if(std::fread(input.data(),1,size,f)!=(size_t)size)return 4;std::fclose(f);
 FTexInfo_t info{};void* pixels=nullptr;u32 bytes=0;
 if(!gcdata_DecodeTga(input.data(),(u32)size,&info,&pixels,&bytes))return 5;
 f=std::fopen(argv[2],"wb");if(!f){std::free(pixels);return 6;}
 if(std::fwrite(pixels,1,bytes,f)!=bytes)return 7;
 std::fclose(f);std::free(pixels);
 std::printf("%s %ux%u mips=%u decoded=%u\n",info.szName,info.nTexelsAcross,info.nTexelsDown,info.nLodCount,bytes);
 return 0;
}
'''
    # The standalone disk header's pointer field is deliberately an integer.
    code = code.replace('pTexInfo->pUserData = NULL;', 'pTexInfo->pUserData = 0;')
    (work / 'decoder.cpp').write_text(code)
    (work / 'CMakeLists.txt').write_text('cmake_minimum_required(VERSION 3.20)\nproject(rat_hd_decoder LANGUAGES CXX)\nadd_executable(rat_hd_decoder decoder.cpp)\n')
    for args in (['cmake', '-S', str(work), '-B', str(work / 'out')],
                 ['cmake', '--build', str(work / 'out'), '--config', 'Release']):
        result = subprocess.run(args, capture_output=True, text=True)
        if result.returncode:
            raise RuntimeError(result.stdout + result.stderr)
    return work / 'out/Release/rat_hd_decoder.exe'


def validate_texture(exe, path, levels):
    output = path.with_suffix('.bgra')
    subprocess.run([str(exe), str(path), str(output)], check=True)
    expected = b''.join(level.tobytes('raw', 'BGRA') for level in levels)
    assert output.read_bytes() == expected, 'production decoder pixel or mip mismatch'
    original_alpha = np.asarray(Image.open(ART / (path.stem + '-original-alpha.png')))
    actual_alpha = np.asarray(levels[0])[..., 3]
    assert np.array_equal(actual_alpha, original_alpha.repeat(4, 0).repeat(4, 1))
    return {'resource': path.name, 'width': levels[0].width,
            'height': levels[0].height, 'mips': len(levels),
            'format': 'GC RGBA8 uncompressed, original material alpha',
            'bytes': path.stat().st_size, 'sha256': digest(path),
            'production_decode': 'exact BGRA match for every pixel of every mip',
            'base_alpha': 'exact 4x nearest expansion of original samples'}


def make_archive(textures):
    file, header, entries = read_mst(str(BASE))
    file.close()
    assert header['nbytes'] == BASE.stat().st_size
    assert not ARCHIVE.exists(), 'test archive already exists; refusing to overwrite it'
    shutil.copyfile(BASE, ARCHIVE)
    indices = {entry[0].lower(): i for i, entry in enumerate(entries)}
    changed = []
    with ARCHIVE.open('r+b') as dest:
        for path in textures:
            blob = path.read_bytes()
            i = indices[path.name.lower()]
            old = entries[i]
            dest.seek(0, 2)
            padding = (-dest.tell()) % 2048
            dest.write(bytes(padding))
            offset = dest.tell()
            dest.write(blob)
            dest.write(bytes((-dest.tell()) % 2048))
            # Keep name/flags; only update this entry's offset, size, date and CRC.
            dest.seek(108 + i * 36 + 20)
            dest.write(struct.pack('>IIII', offset, len(blob), int(time.time()), zlib.crc32(blob)))
            changed.append(i)
        dest.seek(0, 2)
        total = dest.tell()
        dest.seek(8)
        dest.write(struct.pack('>I', total))
    # Compare every original byte outside the three deliberately changed ranges.
    allowed = [(8, 12)] + [(108 + i * 36 + 20, 108 + i * 36 + 36) for i in changed]
    with BASE.open('rb') as original, ARCHIVE.open('rb') as modified:
        pos = 0
        while block := original.read(1024 * 1024):
            comparison = bytearray(modified.read(len(block)))
            for begin, end in allowed:
                lo, hi = max(begin, pos), min(end, pos + len(block))
                if lo < hi:
                    comparison[lo-pos:hi-pos] = block[lo-pos:hi-pos]
            assert bytes(comparison) == block, 'unexpected archive modification'
            pos += len(block)
    file, updated_header, updated = read_mst(str(ARCHIVE))
    assert updated_header['nbytes'] == ARCHIVE.stat().st_size
    for i, (old, new) in enumerate(zip(entries, updated)):
        if i not in changed:
            assert new == old
        else:
            file.seek(new[2])
            blob = file.read(new[3])
            assert blob == (OUT / new[0]).read_bytes()
            assert new[2] % 2048 == 0
    assert all(entry[2] + entry[3] <= updated_header['nbytes'] for entry in updated)
    file.close()
    return {'path': str(ARCHIVE), 'sha256': digest(ARCHIVE),
            'bytes': ARCHIVE.stat().st_size, 'base_sha256': digest(BASE),
            'changed_resources': [entries[i][0] for i in changed],
            'unchanged_resources': len(entries) - len(changed),
            'verification': 'all unrelated directory entries and original data bytes identical'}


def main():
    OUT.mkdir(parents=True, exist_ok=True)
    source_hash = digest(BASE)
    packed = [pack_texture(name, size) for name, size in ASSETS]
    exe = decoder()
    records = [validate_texture(exe, path, levels) for path, levels in packed]
    archive = make_archive([path for path, _ in packed])
    assert source_hash == digest(BASE), 'original archive changed'
    launcher = ROOT / 'Yellow RAT HD Test.cmd'
    save_root = ROOT / 'build/test-saves/rat-hd-user'
    if not save_root.exists():
        live_saves = Path.home() / 'AppData/Roaming/MAGITS'
        if live_saves.exists():
            shutil.copytree(live_saves, save_root)
        else:
            save_root.mkdir(parents=True)
    text = ('@echo off\r\n'
            'cd /d "' + str(ROOT) + '"\r\n'
            'start "" "' + str(ROOT / 'build/Release/ma_port.exe') + '" '
            '-data "' + str(DATA) + '" -mst rat_hd_yellow.mst '
            '-save-dir "' + str(save_root) + '" '
            '-log "' + str(ROOT / 'build/logs/rat-hd-user-test.log') + '" '
            '-asset-log "' + str(ROOT / 'build/logs/rat-hd-user-assets.log') + '"\r\n')
    launcher.write_bytes(text.encode('utf-8'))
    desktop = Path('D:/Desktop') / 'Metal Arms - Yellow RAT HD Test.cmd'
    shutil.copyfile(launcher, desktop)
    readme = '''Yellow RAT HD test - 2026-10-07

Close your current game, then open Metal Arms - Yellow RAT HD Test.cmd on the
Desktop. It starts the installed root Release EXE at the normal menu using the
separate rat_hd_yellow.mst archive. Select Replay > 5 RAT race for the yellow RAT.
The yellow RAT variant also loads in Wasteland Chase.

Only trdrrata_01 and trdrrata_02 change. Sizes are 1024x512 and 512x512, with seven
mips. The generated colors use the original material alpha, expanded exactly 4x.
The original archive, EXEs, all meshes, red RAT and other textures remain
unchanged. The launcher uses a private copy of your saves under
build/test-saves/rat-hd-user. Use your regular EXE/shortcut for the normal game.

All replacement pixels and mip levels pass the production GC decoder offline.
Every unrelated archive entry and original data byte was checked unchanged.
In-game appearance, UV fit and GPU rendering remain for your manual test.

This test uses a full independent archive to work with the current unmodified
EXE. A future public texture pack should use a smaller replacement loader or
distribute only the replacement assets and a local patcher.
'''
    (OUT / 'README.txt').write_text(readme)
    manifest = {'status': 'installed as an optional test archive; manual visual test pending',
                'textures': records, 'archive': archive,
                'launcher': str(desktop), 'game_launched': False,
                'normal_game_archive_unchanged': True, 'game_executables_changed': False}
    (OUT / 'manifest.json').write_text(json.dumps(manifest, indent=2))
    print(json.dumps(manifest, indent=2))


if __name__ == '__main__':
    main()
