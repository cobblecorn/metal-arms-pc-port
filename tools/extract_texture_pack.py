"""Export every vanilla GameCube texture for the optional HD texture pack.

The native helper compiles the current production gcdata_DecodeTga and its
dependencies without modifying game source. One helper process decodes the
whole archive, streaming each base mip to Python for lossless PNG export.
Nothing is installed into the game or written to the source archive.
"""
from __future__ import annotations

import argparse
from collections import Counter
from datetime import datetime, timezone
import hashlib
import json
from pathlib import Path
import struct
import subprocess
import time
import zlib

from PIL import Image

from mst_list import read_mst
from test_coop_checkpoint_rat import method


ROOT = Path(__file__).resolve().parents[1]
PACK = ROOT / "HD Texture Pack"
BASE = ROOT / "gamedata/files/mettlearms_gc.mst"
EXPECTED_VANILLA = "4dd3516e49a32dd5d47ad992cbd5e4faf33b4505fc1b742a6f890aff76045ab0"
FUNCTIONS = (
    "static u16 _ReadBE16(", "static u8 _Expand5(",
    "static u8 _Expand6(", "static u8 _Expand4(",
    "static u32 _PackArgb(", "static u32 _GCTileBytes(",
    "static void _DecodeCmprPlane(", "static BOOL _DecodeUncompressedGCTile(",
    "BOOL gcdata_DecodeTga(",
)


def digest(path: Path) -> str:
    with path.open("rb") as handle:
        return hashlib.file_digest(handle, "sha256").hexdigest()


def pixel_digest(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def write_json(path: Path, data: dict) -> None:
    temp = path.with_suffix(".json.tmp")
    temp.write_text(json.dumps(data, indent=2), encoding="utf-8")
    temp.replace(path)


def build_decoder() -> tuple[Path, dict]:
    work = PACK / "work/texture-decoder"
    work.mkdir(parents=True, exist_ok=True)
    source_path = ROOT / "port/gcdata.cpp"
    source = source_path.read_text(encoding="utf-8")
    bodies = [method(source, signature) for signature in FUNCTIONS]
    code = r'''
#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <cstring>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <io.h>
#include <fcntl.h>
using u8=uint8_t;using u16=uint16_t;using u32=uint32_t;using BOOL=int;
constexpr BOOL TRUE=1,FALSE=0;
#define FANG_PLATFORM_WIN 1
#define FANG_WINGC 1
#define DEVPRINTF(...) std::fprintf(stderr,__VA_ARGS__)
constexpr u32 FGCDATA_TEXFMT_COUNT=12,FTEX_FMT_A8R8G8B8=0,FTEX_PALFMT_NONE=0,FTEX_FLAG_NONE=0;
struct FTexInfo_t {
 char szName[16];u32 pUserData;
 u8 nTexFmt,nPalFmt,nFlags,nLodCount,nStencil,nDepth,reserved[2];
 u16 nTexelsAcross,nTexelsDown;
 void ChangeEndian(){nTexelsAcross=(nTexelsAcross>>8)|(nTexelsAcross<<8);nTexelsDown=(nTexelsDown>>8)|(nTexelsDown<<8);}
};
static_assert(sizeof(FTexInfo_t)==32,"disk header");
void* fmem_Alloc(u32 bytes,u32){return std::malloc(bytes);}
'''
    code += "\n".join(bodies).replace("pTexInfo->pUserData = NULL;", "pTexInfo->pUserData = 0;")
    code += r'''
int main(int argc,char** argv){
 if(argc!=3)return 2;
 _setmode(_fileno(stdout),_O_BINARY);
 FILE* archive=std::fopen(argv[1],"rb");if(!archive)return 3;
 std::ifstream jobs(argv[2]);if(!jobs)return 4;
 std::string line;
 while(std::getline(jobs,line)){
  std::istringstream fields(line);u32 offset=0,size=0;std::string name;
  if(!(fields>>offset>>size>>name))return 5;
  std::vector<u8> input(size);FTexInfo_t info{};void* pixels=nullptr;u32 bytes=0;
  u32 frame[4]={0,0,0,0};
  if(std::fseek(archive,(long)offset,SEEK_SET)!=0||std::fread(input.data(),1,size,archive)!=size){
   frame[0]=1;std::fprintf(stderr,"%s: archive read failed\n",name.c_str());
  }else if(!gcdata_DecodeTga(input.data(),size,&info,&pixels,&bytes)){
   frame[0]=2;std::fprintf(stderr,"%s: production decoder rejected texture\n",name.c_str());
  }else{
   frame[1]=info.nTexelsAcross;frame[2]=info.nTexelsDown;
   frame[3]=frame[1]*frame[2]*4;
   if(frame[3]>bytes){frame[0]=3;frame[1]=frame[2]=frame[3]=0;}
  }
  if(std::fwrite(frame,sizeof(u32),4,stdout)!=4)return 6;
  if(!frame[0]&&std::fwrite(pixels,1,frame[3],stdout)!=frame[3])return 7;
  std::free(pixels);
 }
 std::fclose(archive);return 0;
}
'''
    cpp = work / "decoder.cpp"
    cpp.write_text(code, encoding="utf-8")
    (work / "CMakeLists.txt").write_text(
        "cmake_minimum_required(VERSION 3.20)\n"
        "project(texture_pack_decoder LANGUAGES CXX)\n"
        "add_executable(texture_pack_decoder decoder.cpp)\n", encoding="utf-8")
    log = PACK / "logs/vanilla-decoder-build.log"
    with log.open("w", encoding="utf-8") as output:
        for args in (
            ["cmake", "-S", str(work), "-B", str(work / "out")],
            ["cmake", "--build", str(work / "out"), "--config", "Release"],
        ):
            result = subprocess.run(args, stdout=output, stderr=subprocess.STDOUT)
            if result.returncode:
                raise RuntimeError(f"Decoder build failed; inspect {log}")
    return work / "out/Release/texture_pack_decoder.exe", {
        "production_source": str(source_path),
        "production_source_sha256": digest(source_path),
        "function_body_sha256": pixel_digest("\n".join(bodies).encode("utf-8")),
        "generated_harness_sha256": digest(cpp),
        "functions": list(FUNCTIONS),
        "method": "Current production function bodies; native 32-byte disk header; streamed BGRA base mip",
        "log": str(log.relative_to(PACK)).replace("\\", "/"),
    }


def read_exact(handle, size: int) -> bytes:
    data = bytearray()
    while len(data) < size:
        block = handle.read(size - len(data))
        if not block:
            raise EOFError(f"Decoder stream ended after {len(data)} of {size} bytes")
        data.extend(block)
    return bytes(data)


def save_png(image: Image.Image, path: Path) -> str:
    image.save(path, compress_level=2)
    # Verify the persisted file, not merely the in-memory pixels.
    with Image.open(path) as persisted:
        if persisted.mode != image.mode or persisted.size != image.size:
            raise AssertionError(f"PNG mode/dimensions changed: {path}")
        if persisted.tobytes() != image.tobytes():
            raise AssertionError(f"PNG pixels changed: {path}")
    return digest(path)


def priority(entry) -> tuple[int, str]:
    name = entry[0].lower()
    first = ("te_1metal03.tga", "te_1metal01.tga", "te_1concr02.tga", "tewj_rust01.tga",
             "tedm_met03.tga", "trdggltch00.tga", "trmrrata_01.tga", "trmrrata_02.tga", "twdm4wep.tga")
    if name in first:
        return first.index(name), name
    # Named sheets are most useful as image generation references; numbered
    # baked lighting textures remain in the complete export later in the batch.
    rank = next((i for i, prefix in enumerate(("tr", "te", "tc", "tw", "to")) if name.startswith(prefix)), 5)
    return rank + len(first), name


def extract(archive: Path, expected_hash: str) -> dict:
    started = time.time()
    folders = {key: PACK / "vanilla" / key for key in ("packed", "rgba", "color", "alpha")}
    for folder in (*folders.values(), PACK / "catalog", PACK / "work", PACK / "logs"):
        folder.mkdir(parents=True, exist_ok=True)
    archive_hash = digest(archive)
    if archive_hash != expected_hash:
        raise RuntimeError(f"Archive is not the requested vanilla base: {archive_hash}")
    handle, header, entries = read_mst(str(archive))
    texture_entries = sorted((entry for entry in entries if entry[0].lower().endswith(".tga")), key=priority)
    if len({entry[0].lower() for entry in texture_entries}) != len(texture_entries):
        raise RuntimeError("Duplicate case-insensitive texture names require separate export paths")
    exe, decoder_info = build_decoder()
    jobs = PACK / "work/vanilla-decoder-jobs.txt"
    jobs.write_text("".join(f"{entry[2]} {entry[3]} {entry[0]}\n" for entry in texture_entries), encoding="ascii")
    manifest_path = PACK / "catalog/vanilla-textures.json"
    records = []
    manifest = {
        "schema_version": 1,
        "created_utc": datetime.now(timezone.utc).isoformat(),
        "source_archive": {"path": str(archive), "sha256": archive_hash, "bytes": archive.stat().st_size,
                           "directory_entries": len(entries), "texture_entries": len(texture_entries)},
        "decoder": decoder_info,
        "summary": {"status": "extracting", "requested": len(texture_entries), "processed": 0},
        "textures": records,
    }
    write_json(manifest_path, manifest)
    color_seen, rgba_seen = {}, {}
    errors = PACK / "logs/vanilla-decoder-errors.log"
    with errors.open("wb") as error_log:
        proc = subprocess.Popen([str(exe), str(archive), str(jobs)], stdout=subprocess.PIPE, stderr=error_log)
        try:
            for index, entry in enumerate(texture_entries):
                name, flags, offset, size, modified, crc = entry
                if Path(name).name != name or "/" in name or "\\" in name:
                    raise RuntimeError(f"Unsafe archive resource name: {name!r}")
                stem = Path(name).stem
                handle.seek(offset)
                blob = handle.read(size)
                if len(blob) != size:
                    raise EOFError(f"Archive texture truncated: {name}")
                packed_path = folders["packed"] / name
                packed_path.write_bytes(blob)
                source_hash = pixel_digest(blob)
                if digest(packed_path) != source_hash:
                    raise AssertionError(f"Packed copy differs: {name}")
                png_paths = {key: folders[key] / (stem + ".png") for key in ("rgba", "color", "alpha")}
                fmt = blob[20] if size >= 32 else None
                width, height = struct.unpack_from(">HH", blob, 28) if size >= 32 else (None, None)
                record = {
                    "resource": name, "stem": stem, "width": width, "height": height,
                    "format": fmt, "lods": blob[23] if size >= 32 else None,
                    "mips": blob[23] if size >= 32 else None,
                    "bytes": size, "source_sha256": source_hash,
                    "archive": {"offset": offset, "flags": flags, "modified": modified,
                                "directory_crc32": crc, "computed_crc32": zlib.crc32(blob)},
                    "paths": {"packed": str(packed_path.relative_to(PACK)).replace("\\", "/")},
                    "duplicate_color_of": None, "duplicate_rgba_of": None,
                }
                status, decoded_width, decoded_height, decoded_bytes = struct.unpack("<IIII", read_exact(proc.stdout, 16))
                if status:
                    record.update(status="decoder_failed", decoder_status=status,
                                  failure={1: "Archive read failed", 2: "Production decoder rejected texture", 3: "Invalid base mip length"}.get(status, "Unknown decoder failure"))
                else:
                    if (decoded_width, decoded_height, decoded_bytes) != (width, height, width * height * 4):
                        raise AssertionError(f"Production decoder dimensions disagree: {name}")
                    bgra = read_exact(proc.stdout, decoded_bytes)
                    rgba = Image.frombytes("RGBA", (width, height), bgra, "raw", "BGRA")
                    color, alpha = rgba.convert("RGB"), rgba.getchannel("A")
                    rgba_hash, color_hash, alpha_hash = map(pixel_digest, (rgba.tobytes(), color.tobytes(), alpha.tobytes()))
                    color_key, rgba_key = ((width, height, color_hash), (width, height, rgba_hash))
                    record["duplicate_color_of"] = color_seen.get(color_key)
                    record["duplicate_rgba_of"] = rgba_seen.get(rgba_key)
                    color_seen.setdefault(color_key, name)
                    rgba_seen.setdefault(rgba_key, name)
                    record["hashes"] = {"rgba_pixels": rgba_hash, "color_pixels": color_hash,
                                        "alpha_pixels": alpha_hash}
                    for key, image in (("rgba", rgba), ("color", color), ("alpha", alpha)):
                        record["hashes"][key + "_png"] = save_png(image, png_paths[key])
                        record["paths"][key] = str(png_paths[key].relative_to(PACK)).replace("\\", "/")
                    lo, hi = alpha.getextrema()
                    record["alpha"] = {"min": lo, "max": hi,
                                       "non_opaque_pixels": width * height - alpha.histogram()[255]}
                    record["status"] = "decoded"
                records.append(record)
                if (index + 1) % 100 == 0 or index + 1 == len(texture_entries):
                    manifest["summary"].update(processed=len(records), decoded=sum(r["status"] == "decoded" for r in records))
                    write_json(manifest_path, manifest)
                    print(f"Exported {index + 1}/{len(texture_entries)}: {name}; {time.time() - started:.1f}s", flush=True)
            if proc.stdout.read(1):
                raise AssertionError("Unexpected trailing native decoder data")
            exit_code = proc.wait()
            if exit_code:
                raise RuntimeError(f"Native decoder exited {exit_code}")
        except BaseException:
            proc.kill()
            proc.wait()
            manifest["summary"].update(status="interrupted", processed=len(records))
            write_json(manifest_path, manifest)
            raise
        finally:
            handle.close()
            proc.stdout.close()
    archive_after = digest(archive)
    if archive_after != archive_hash:
        raise AssertionError("Source archive changed during extraction")
    failures = [record for record in records if record["status"] != "decoded"]
    counts = {key: len(list(folder.glob("*.tga" if key == "packed" else "*.png"))) for key, folder in folders.items()}
    decoded = len(records) - len(failures)
    if counts != {"packed": len(texture_entries), "rgba": decoded, "color": decoded, "alpha": decoded}:
        raise AssertionError(f"Exported folder counts disagree with manifest: {counts}")
    manifest["source_archive"]["sha256_after"] = archive_after
    manifest["summary"] = {
        "status": "complete" if not failures else "complete_with_decoder_failures",
        "requested": len(texture_entries), "processed": len(records), "decoded": decoded,
        "failures": len(failures), "failure_resources": [record["resource"] for record in failures],
        "files": counts, "unique_color_images": len(color_seen), "unique_rgba_images": len(rgba_seen),
        "duplicate_color_images": sum(record["duplicate_color_of"] is not None for record in records),
        "duplicate_rgba_images": sum(record["duplicate_rgba_of"] is not None for record in records),
        "format_counts": dict(Counter(record["format"] for record in records)),
        "all_pngs_reopened_and_pixel_verified": True,
        "all_packed_copies_sha256_verified": True,
        "source_archive_unchanged": archive_after == archive_hash,
        "duration_seconds": round(time.time() - started, 2),
        "decoder_error_log": str(errors.relative_to(PACK)).replace("\\", "/"),
    }
    write_json(manifest_path, manifest)
    print(json.dumps(manifest["summary"], indent=2), flush=True)
    return manifest


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--archive", type=Path, default=BASE)
    parser.add_argument("--expected-sha256", default=EXPECTED_VANILLA)
    args = parser.parse_args()
    extract(args.archive, args.expected_sha256)
