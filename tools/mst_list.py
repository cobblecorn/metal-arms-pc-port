"""List/inspect a Metal Arms .mst master file (GameCube, big-endian).

Layout (from ma/Lib/Fang2/fdata.h):
  FDataPrjFile_Header_t   108 bytes
  FDataPrjFile_Entry_t    36 bytes  * (nNumEntries + nNumFreeEntries)
  FDataPrjFile_SupportEntry_t 20 bytes * (nNumSupport + nNumFreeSupport)
  data (files start on 2048-byte boundaries)
"""
import struct, sys, collections, os

def read_mst(path, be=True):
    e = ">" if be else "<"
    f = open(path, "rb")
    h = f.read(108)
    sig = h[:4]
    ver, nbytes, nent, nfree, nsup, nfsup, doff = struct.unpack(e + "IIIIIII", h[4:32])
    comp = struct.unpack(e + "10I", h[32:72])
    names = ["tga", "ape", "mtx", "csv", "fnt", "sma", "gt", "wvb", "fpr", "cam"]
    hdr = dict(sig=sig, version=ver, nbytes=nbytes, nentries=nent, nfree=nfree,
               nsupport=nsup, nfreesupport=nfsup, dataoff=doff,
               compiler=dict(zip(names, comp)))
    f.seek(108)
    raw = f.read(36 * nent)
    entries = []
    for i in range(nent):
        n, flags, pad, start, size, mtime, crc = struct.unpack(e + "16sHHIIII", raw[i*36:(i+1)*36])
        entries.append((n.split(b"\0")[0].decode("latin1"), flags, start, size, mtime, crc))
    return f, hdr, entries

if __name__ == "__main__":
    path = sys.argv[1]
    f, hdr, ents = read_mst(path)
    print("signature      :", hdr["sig"])
    print("version word   : 0x%08x" % hdr["version"])
    print("bytes in file  : %d (actual %d)" % (hdr["nbytes"], os.path.getsize(path)))
    print("entries        : %d used, %d free" % (hdr["nentries"], hdr["nfree"]))
    print("support entries: %d used" % hdr["nsupport"])
    print("data offset    : 0x%x" % hdr["dataoff"])
    print("compiler ver   :", {k: hex(v) for k, v in hdr["compiler"].items()})
    print()
    by = collections.defaultdict(lambda: [0, 0])
    for n, fl, st, sz, mt, crc in ents:
        ext = n.rsplit(".", 1)[-1].lower() if "." in n else "(none)"
        by[ext][0] += 1; by[ext][1] += sz
    print("%-8s %6s %12s" % ("ext", "count", "MB"))
    for ext, (c, s) in sorted(by.items(), key=lambda kv: -kv[1][1]):
        print("%-8s %6d %12.2f" % (ext, c, s / 1048576))
    # sanity: every entry inside the file?
    bad = [e for e in ents if e[2] + e[3] > os.path.getsize(path)]
    print("\nentries out of bounds:", len(bad))
    if len(sys.argv) > 2:
        with open(sys.argv[2], "w") as o:
            o.write("name,flags,offset,size,mtime,crc\n")
            for n, fl, st, sz, mt, crc in ents:
                o.write("%s,0x%x,%d,%d,%d,0x%08x\n" % (n, fl, st, sz, mt, crc))
        print("wrote", sys.argv[2])


def extract_all(path, outdir):
    """Dump every entry to outdir/<name>. Returns (written, duplicates)."""
    f, hdr, ents = read_mst(path)
    os.makedirs(outdir, exist_ok=True)
    seen, dups, n = set(), [], 0
    for name, fl, st, sz, mt, crc in ents:
        key = name.lower()
        if key in seen:
            dups.append(name); continue
        seen.add(key)
        f.seek(st)
        with open(os.path.join(outdir, name), "wb") as o:
            o.write(f.read(sz))
        n += 1
    return n, dups
