#!/usr/bin/env python3
"""Find and list retail GameCube code that references a data address in main.dol.

A minimal PowerPC decoder for recovering how the retail game uses a structure (for example which
retail user-property offsets feed which runtime fields). It decodes the D-form loads/stores,
address arithmetic, branches and the common X-form integer operations; everything else is shown
as a raw word. Register constants built with lis/addi/ori are tracked within the listing.

    python tools/dol_xref.py gamedata/sys/main.dol 0x8048dd38 [--before 64] [--after 400]

Only reads the DOL. Listings describe retail code: keep saved output under ignored build/.
"""

import argparse
import struct

D_FORM = {
    14: "addi", 15: "addis", 7: "mulli", 8: "subfic", 10: "cmplwi", 11: "cmpwi", 12: "addic", 24: "ori", 25: "oris",
    28: "andi.", 32: "lwz", 33: "lwzu", 34: "lbz", 35: "lbzu", 36: "stw", 37: "stwu", 38: "stb", 39: "stbu",
    40: "lhz", 41: "lhzu", 42: "lha", 44: "sth", 45: "sthu", 46: "lmw", 47: "stmw",
    48: "lfs", 49: "lfsu", 50: "lfd", 51: "lfdu", 52: "stfs", 53: "stfsu", 54: "stfd", 55: "stfdu",
}
FLOAT_MEM = {48, 49, 50, 51, 52, 53, 54, 55}
X31 = {266: "add", 444: "or", 235: "mullw", 40: "subf", 23: "lwzx", 151: "stwx", 535: "lfsx", 663: "stfsx",
       87: "lbzx", 215: "stbx", 24: "slw", 536: "srw", 824: "srawi", 28: "and", 0: "cmpw", 32: "cmplw",
       339: "mfspr", 467: "mtspr", 459: "divwu", 491: "divw", 104: "neg", 954: "extsb", 922: "extsh"}
FP63 = {18: "fdiv", 20: "fsub", 21: "fadd", 25: "fmul", 72: "fmr", 40: "fneg", 0: "fcmpu", 12: "frsp", 15: "fctiwz"}
FP59 = {18: "fdivs", 20: "fsubs", 21: "fadds", 25: "fmuls", 24: "fres", 28: "fmsubs", 29: "fmadds"}


def sext16(v):
    return v - 0x10000 if v & 0x8000 else v


class Dol:
    def __init__(self, data):
        self.data = data
        offs = struct.unpack_from(">18I", data, 0x00)
        addrs = struct.unpack_from(">18I", data, 0x48)
        sizes = struct.unpack_from(">18I", data, 0x90)
        self.sections = [(a, o, s, i < 7) for i, (o, a, s) in enumerate(zip(offs, addrs, sizes)) if s]

    def word(self, addr):
        for a, o, s, _ in self.sections:
            if a <= addr < a + s:
                return struct.unpack_from(">I", self.data, o + addr - a)[0]
        return None

    def text_words(self):
        for a, o, s, is_text in self.sections:
            if is_text:
                for i in range(0, s, 4):
                    yield a + i, struct.unpack_from(">I", self.data, o + i)[0]


def decode(addr, ins, regs):
    op = ins >> 26
    rd, ra, rb = (ins >> 21) & 31, (ins >> 16) & 31, (ins >> 11) & 31
    imm = ins & 0xFFFF
    note = ""
    if op in D_FORM:
        name = D_FORM[op]
        d = sext16(imm)
        if op == 15:  # addis / lis
            base = 0 if ra == 0 else regs.get(ra)
            if base is not None:
                regs[rd] = (base + (imm << 16)) & 0xFFFFFFFF
            else:
                regs.pop(rd, None)
            text = ("lis r%d, 0x%x" % (rd, imm)) if ra == 0 else ("addis r%d, r%d, 0x%x" % (rd, ra, imm))
        elif op == 14:  # addi / li
            if ra == 0:
                regs[rd] = d & 0xFFFFFFFF
                text = "li r%d, %d" % (rd, d)
            else:
                base = regs.get(ra)
                if base is not None:
                    regs[rd] = (base + d) & 0xFFFFFFFF
                    note = "r%d=0x%08x" % (rd, regs[rd])
                else:
                    regs.pop(rd, None)
                text = "addi r%d, r%d, %d" % (rd, ra, d)
        elif op == 24:  # ori
            if regs.get(rd) is not None:
                regs[ra] = regs[rd] | imm
            text = "ori r%d, r%d, 0x%x" % (ra, rd, imm)
        elif op in (10, 11):
            text = "%s cr%d, r%d, %d" % (name, rd >> 2, ra, d if op == 11 else imm)
        elif op in (7, 8, 12, 28, 25):
            text = "%s r%d, r%d, %d" % (name, rd if op not in (28, 25) else ra, ra if op not in (28, 25) else rd, d)
            regs.pop(rd if op not in (28, 25) else ra, None)
        else:
            reg = ("f%d" if op in FLOAT_MEM else "r%d") % rd
            text = "%s %s, %d(r%d)" % (name, reg, d, ra)
            base = regs.get(ra)
            if base is not None:
                note = "[0x%08x]" % ((base + d) & 0xFFFFFFFF)
            if name.startswith("l") and op not in FLOAT_MEM:
                regs.pop(rd, None)
        return text, note
    if op == 18:
        li = ins & 0x03FFFFFC
        if li & 0x02000000:
            li -= 0x04000000
        target = (li if ins & 2 else addr + li) & 0xFFFFFFFF
        return ("bl" if ins & 1 else "b") + " 0x%08x" % target, ""
    if op == 16:
        bd = sext16(ins & 0xFFFC)
        return "bc %d,%d, 0x%08x" % (rd, ra, (addr + bd) & 0xFFFFFFFF), ""
    if op == 19:
        xo = (ins >> 1) & 0x3FF
        if xo == 16:
            return "blr" if rd == 20 else "bclr %d,%d" % (rd, ra), ""
        if xo == 528:
            return "bctr" + ("l" if ins & 1 else ""), ""
        return "op19.%d" % xo, ""
    if op == 31:
        xo = (ins >> 1) & 0x3FF
        name = X31.get(xo)
        if name == "or" and rd == rb:
            if rd in regs:
                regs[ra] = regs[rd]
            else:
                regs.pop(ra, None)
            return "mr r%d, r%d" % (ra, rd), ""
        if name == "mfspr" or name == "mtspr":
            spr = ((ins >> 16) & 31) | (((ins >> 11) & 31) << 5)
            sprname = {8: "lr", 9: "ctr"}.get(spr, str(spr))
            return ("mf%s r%d" if name == "mfspr" else "mt%s r%d") % (sprname, rd), ""
        if name:
            regs.pop(rd, None)
            return "%s r%d, r%d, r%d" % (name, rd, ra, rb), ""
        return "op31.%d" % xo, ""
    if op == 63:
        xo = (ins >> 1) & 0x3FF
        name = FP63.get(xo) or FP63.get(xo & 31)
        return ("%s f%d, f%d, f%d" % (name, rd, ra, rb)) if name else "op63.%d" % xo, ""
    if op == 59:
        name = FP59.get((ins >> 1) & 31)
        return ("%s f%d, f%d, f%d" % (name, rd, ra, rb)) if name else "op59", ""
    if op == 21:
        return "rlwinm r%d, r%d, %d, %d, %d" % (ra, rd, rb, (ins >> 6) & 31, (ins >> 1) & 31), ""
    return ".word 0x%08x" % ins, ""


def find_refs(dol, target, span):
    hi = ((target + 0x8000) >> 16) & 0xFFFF
    lo = sext16(target & 0xFFFF)
    words = list(dol.text_words())
    hits = []
    for i, (addr, ins) in enumerate(words):
        if ins >> 26 == 15 and (ins >> 16) & 31 == 0 and ins & 0xFFFF == hi:
            reg = (ins >> 21) & 31
            for addr2, ins2 in words[i + 1:i + 24]:
                op2 = ins2 >> 26
                if op2 in D_FORM and (ins2 >> 16) & 31 == reg and op2 not in (15,):
                    d = sext16(ins2 & 0xFFFF)
                    if 0 <= (((hi << 16) + d - target) & 0xFFFFFFFF) < span:
                        hits.append((addr, addr2, ((hi << 16) + d) & 0xFFFFFFFF))
                        break
    return hits


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("dol")
    ap.add_argument("address", help="data address, e.g. 0x8048dd38")
    ap.add_argument("--before", type=int, default=48, help="instructions to list before a reference")
    ap.add_argument("--after", type=int, default=160, help="instructions to list after a reference")
    ap.add_argument("--span", type=lambda s: int(s, 0), default=4, help="bytes after the address that count as a reference")
    ap.add_argument("--list", help="list code at this address instead of searching")
    args = ap.parse_args()
    dol = Dol(open(args.dol, "rb").read())
    target = int(args.address, 0)
    starts = [int(args.list, 0)] if args.list else None
    if starts is None:
        hits = find_refs(dol, target, args.span)
        print("references near 0x%08x: %d" % (target, len(hits)))
        for lis_addr, use_addr, value in hits:
            print("  lis at 0x%08x, use at 0x%08x -> 0x%08x (%+d)" % (lis_addr, use_addr, value, value - target))
        starts = [h[0] for h in hits]
    for start in starts:
        print("\n==== 0x%08x ====" % start)
        regs = {}
        for addr in range(start - 4 * args.before, start + 4 * args.after, 4):
            ins = dol.word(addr)
            if ins is None:
                break
            text, note = decode(addr, ins, regs)
            print("%08x  %08x  %-36s %s" % (addr, ins, text, note))


if __name__ == "__main__":
    main()
