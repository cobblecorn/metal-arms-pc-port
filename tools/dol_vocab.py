#!/usr/bin/env python3
"""Recover retail game-data vocabularies from the GameCube main.dol.

Fang reads binary .csv tables through FGameDataMap_t arrays:
    { const char *pszTableName; const FGameData_TableEntry_t *pVocab; u32 nBytes; void *pDest; }
and each vocabulary is an array of 8-byte FGameData_TableEntry_t records
    { u32 nFlags; u16 nBytesForData; u8 uMinValueLookup; u8 uMaxValueLookup; }
terminated by an entry whose data type is FGAMEDATA_VAR_TYPE_COUNT (see fgamedata.h).

This tool finds each map entry whose name matches a requested table, follows its vocabulary
pointer, and decodes the retail field list (type, conversion flags, output bytes, clamp range).
It only reads the DOL. The report describes retail code/data: keep it under ignored build/.

    python tools/dol_vocab.py gamedata/sys/main.dol LaserL1 BlasterL1 --output build/logs/vocab.txt
"""

import argparse
import struct
import sys

TYPE_STRING, TYPE_FLOAT, TYPE_WIDESTRING, TYPE_COUNT = 0, 1, 2, 3
TYPE_NAMES = {TYPE_STRING: "string", TYPE_FLOAT: "float", TYPE_WIDESTRING: "wstring"}

STRING_FLAGS = [
    (0x00000100, "PTR_ONLY"), (0x00000200, "FAIL_ON_OVERFLOW"), (0x00000400, "PTR_TO_MAIN_STR_TBL"),
    (0x00000800, "SFX_HANDLE"), (0x00001000, "NONE_TO_NULL"), (0x00002000, "TEXDEF"),
    (0x00004000, "FPART_HANDLE"), (0x00008000, "MESH"), (0x00010000, "DEBRIS_GROUP"),
    (0x00020000, "DEBRIS_MESH_SET"), (0x00040000, "EXPLODE_GROUP"), (0x00080000, "SOUND_GROUP"),
    (0x00100000, "DAMAGE_PROFILE"), (0x00200000, "ARMOR_PROFILE"), (0x00400000, "MOTIF_BASE"),
    (0x00800000, "DECAL_DEF"),
]
FLOAT_CHECK_FLAGS = [(0x00000100, "CLAMP_AND_GO"), (0x00000200, "CHECK_RANGE_AND_FAIL")]
# Output conversions: each one requested stores one f32/u32 in low-to-high bit order.
FLOAT_OUTPUT_FLAGS = [
    (0x00000400, "X"), (0x00000800, "OO_X"), (0x00001000, "RADS_TO_DEGS"), (0x00002000, "DEGS_TO_RADS"),
    (0x00004000, "PERCENT_TO_UNIT"), (0x00008000, "UNIT_FLOAT"), (0x00010000, "BIPOLAR_UNIT"),
    (0x00020000, "SQRT_X"), (0x00040000, "OO_SQRT_X"), (0x00080000, "SIN_DEG"), (0x00100000, "COS_DEG"),
    (0x00200000, "X_SQUARED"), (0x00400000, "X_CUBED"), (0x00800000, "ABS_X"), (0x01000000, "OO_X_SQUARED"),
    (0x02000000, "TO_U32"), (0x04000000, "TO_S32"), (0x08000000, "UNCLAMPED_PERCENT_TO_UNIT"),
]
# Source afDataTable (fgamedata.cpp). Verified against the DOL copy when that copy is found.
SOURCE_F32_TABLE = [
    0.0, 1e-7, 1e-5, 1e-4, 1e-3, 0.01, 0.1, 0.95, -1.0, 1.0, 2.0, 3.0, 5.0, 6.0, 10.0, 15.0, 20.0,
    30.0, 50.0, 60.0, -100.0, 100.0, 255.0, 500.0, -1000.0, 1000.0, 5000.0, -10000.0, 10000.0,
    65534.0, 65535.0, 100000.0, 10000000.0, -1e9, 1e9, 4294967295.0, 1e11,
]


class Dol:
    def __init__(self, data):
        self.data = data
        offs = struct.unpack_from(">18I", data, 0x00)
        addrs = struct.unpack_from(">18I", data, 0x48)
        sizes = struct.unpack_from(">18I", data, 0x90)
        self.sections = [(a, o, s, i >= 7) for i, (o, a, s) in enumerate(zip(offs, addrs, sizes)) if s]

    def to_off(self, addr):
        for a, o, s, _ in self.sections:
            if a <= addr < a + s:
                return o + addr - a
        return None

    def to_addr(self, off):
        for a, o, s, _ in self.sections:
            if o <= off < o + s:
                return a + off - o
        return None

    def cstr(self, addr, limit=128):
        off = self.to_off(addr)
        if off is None:
            return None
        end = self.data.find(b"\0", off, off + limit)
        if end < 0:
            return None
        raw = self.data[off:end]
        if not raw or any(c < 0x20 or c > 0x7e for c in raw):
            return None
        return raw.decode("ascii")


def decode_entry(flags, nbytes, lo, hi, f32_table):
    vtype = flags & 0xFF
    if vtype == TYPE_COUNT:
        return None
    names = []
    if vtype in (TYPE_STRING, TYPE_WIDESTRING):
        names = [n for bit, n in STRING_FLAGS if flags & bit]
        rng = ""
    elif vtype == TYPE_FLOAT:
        checks = [n for bit, n in FLOAT_CHECK_FLAGS if flags & bit]
        outputs = [n for bit, n in FLOAT_OUTPUT_FLAGS if flags & bit]
        names = checks + outputs
        needs_range = flags & (0x00008000 | 0x00010000 | 0x00000100 | 0x00000200)
        def val(i):
            return repr(f32_table[i]) if i < len(f32_table) else "?%d" % i
        rng = "[%s, %s]" % (val(lo), val(hi)) if needs_range else ""
    else:
        return "invalid type %d" % vtype
    return "%-7s %3dB %-40s %s" % (TYPE_NAMES.get(vtype, "?"), nbytes, "|".join(names), rng)


def valid_entry(flags, nbytes, lo, hi):
    vtype = flags & 0xFF
    if vtype == TYPE_COUNT:
        return nbytes == 0 and (flags >> 8) == 0
    if vtype not in (TYPE_STRING, TYPE_FLOAT, TYPE_WIDESTRING):
        return False
    if vtype == TYPE_FLOAT:
        outputs = sum(1 for bit, _ in FLOAT_OUTPUT_FLAGS if flags & bit)
        return (flags & ~0x0FFFFFFF) == 0 and outputs >= 1 and nbytes == 4 * outputs and lo < 64 and hi < 64
    return (flags & ~0x00FFFFFF) == 0 and 0 < nbytes <= 1024


def read_vocab(dol, addr, limit=512):
    off = dol.to_off(addr)
    if off is None:
        return None
    entries = []
    for i in range(limit):
        flags, nbytes, lo, hi = struct.unpack_from(">IHBB", dol.data, off + 8 * i)
        if not valid_entry(flags, nbytes, lo, hi):
            return None
        if flags & 0xFF == TYPE_COUNT:
            return entries
        entries.append((flags, nbytes, lo, hi))
    return None


def find_f32_table(dol):
    needle = struct.pack(">%df" % 12, *SOURCE_F32_TABLE[:12])
    off = dol.data.find(needle)
    if off < 0:
        return SOURCE_F32_TABLE, None
    # The table starts with 0.0 and ends before the zero padding that follows it.
    vals = [0.0]
    for i in range(1, 64):
        (v,) = struct.unpack_from(">f", dol.data, off + 4 * i)
        if v == 0.0:
            break
        vals.append(v)
    # Retail inserts entries (e.g. 4.0 after 3.0), shifting later indices: always use the DOL copy.
    same = len(vals) == len(SOURCE_F32_TABLE) and all(
        abs(a - b) <= abs(b) * 1e-6 for a, b in zip(vals, SOURCE_F32_TABLE))
    return vals, (dol.to_addr(off), same, vals)


def find_maps(dol, names):
    """Return {table name: [(map entry addr, vocab addr, dest bytes, dest addr)]}."""
    wanted = {n.lower() for n in names}
    found = {}
    string_addrs = {}
    # Locate candidate string addresses first.
    for a, o, s, _ in dol.sections:
        blob = dol.data[o:o + s]
        for name in names:
            pos = blob.find(name.encode() + b"\0")
            while pos >= 0:
                if pos == 0 or blob[pos - 1] == 0:
                    string_addrs.setdefault(a + pos, name)
                pos = blob.find(name.encode() + b"\0", pos + 1)
    # Scan data sections for pointers to those strings followed by a valid vocabulary pointer.
    for a, o, s, is_data in dol.sections:
        if not is_data:
            continue
        for rel in range(0, s - 16, 4):
            p_name, p_vocab, n_bytes, p_dest = struct.unpack_from(">4I", dol.data, o + rel)
            name = string_addrs.get(p_name)
            if not name or name.lower() not in wanted:
                continue
            vocab = read_vocab(dol, p_vocab)
            if vocab is None:
                continue
            found.setdefault(name, []).append((a + rel, p_vocab, n_bytes, p_dest, vocab))
    return found


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("dol")
    ap.add_argument("tables", nargs="+", help="table names used by FGameDataMap_t arrays (e.g. LaserL1)")
    ap.add_argument("--output", help="write the report here instead of stdout")
    args = ap.parse_args()

    dol = Dol(open(args.dol, "rb").read())
    f32_table, where = find_f32_table(dol)
    out = []
    if where:
        out.append("afDataTable at 0x%08x (%s source values): %s" % (
            where[0], "matches" if where[1] else "DIFFERS FROM", ", ".join("%g" % v for v in where[2])))
    else:
        out.append("afDataTable not found; ranges use source values")
    maps = find_maps(dol, args.tables)
    for name in args.tables:
        hits = maps.get(name, [])
        if not hits:
            out.append("\n%s: no FGameDataMap_t entry found" % name)
            continue
        for map_addr, vocab_addr, n_bytes, dest, vocab in hits:
            out.append("\n%s: map 0x%08x vocab 0x%08x fields %d dest 0x%08x bytes %d"
                       % (name, map_addr, vocab_addr, len(vocab), dest, n_bytes))
            offset = 0
            for i, (flags, nbytes, lo, hi) in enumerate(vocab):
                out.append("  %2d +%-4d %08x %s" % (i, offset, flags, decode_entry(flags, nbytes, lo, hi, f32_table)))
                offset += nbytes
            out.append("  struct bytes from vocabulary: %d" % offset)
    text = "\n".join(out) + "\n"
    if args.output:
        open(args.output, "w").write(text)
    sys.stdout.write(text)


if __name__ == "__main__":
    main()
