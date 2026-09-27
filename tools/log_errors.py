"""Group the errors and warnings in a set of game logs, most widespread first.

usage: python tools/log_errors.py LOG [LOG ...]        e.g. python tools/log_errors.py build/logs/sweep1_*.log

Each distinct message (numbers, addresses and quoted names folded, so the same problem in different
places groups together) is listed once with how many logs and lines have it, and one example line.
Crash, assert and run-time-check reports are listed separately with the log they are in, since each one
matters. Asset logs (*-assets.log) are skipped.
"""
import collections
import glob
import os
import re
import sys

PATTERNS = re.compile(
    r"error|warning|unknown command|could not|couldn't|cannot|can't|failed|invalid|not found|missing|"
    r"unrecognized|unsupported|overflow|exceeded|out of|no free|too many|bad |corrupt",
    re.IGNORECASE)
# engine chatter that matches the words above but is not a problem
IGNORE = re.compile(
    r"^PORT-|^\[ FMOVIE2 \] _Movie|LOAD MARKER|---TIMING|Found event name|^    #|"
    r"Discord: |PC input:|^gcdata: loaded|^gcdata: converted|^gcdata: decoded",
    re.IGNORECASE)
REPORT = re.compile(r"^\*\*\* ")	# *** CRASH, *** CRT ASSERT, *** RUN-TIME CHECK FAILURE, ...


def normalize(line):
    line = re.sub(r"0x[0-9a-fA-F]+|\b[0-9A-F]{6,8}\b", "#", line)
    line = re.sub(r"-?\d+(\.\d+)?", "#", line)
    line = re.sub(r"'[^']*'", "'…'", line)
    line = re.sub(r'"[^"]*"', '"…"', line)
    return line.strip()


def main(paths):
    files = []
    for pattern in paths:
        files += glob.glob(pattern)
    files = sorted(f for f in set(files) if not f.endswith("-assets.log"))
    if not files:
        print(__doc__)
        return 2

    groups = collections.OrderedDict()
    reports = []
    for path in files:
        name = os.path.basename(path)
        with open(path, "r", encoding="latin-1") as handle:
            lines = handle.read().splitlines()
        for index, line in enumerate(lines):
            if REPORT.match(line):
                stack = [l.strip() for l in lines[index + 1:index + 6] if l.startswith("    ")]
                reports.append((name, line.strip(), stack))
                continue
            if IGNORE.search(line) or not PATTERNS.search(line):
                continue
            key = normalize(line)
            entry = groups.setdefault(key, {"logs": set(), "lines": 0, "example": line.strip(), "where": name})
            entry["logs"].add(name)
            entry["lines"] += 1

    print("%d logs" % len(files))
    print("\n== crashes / asserts / run-time checks: %d" % len(reports))
    for name, line, stack in reports:
        print("  [%s] %s" % (name, line))
        for frame in stack[:4]:
            print("        " + frame)

    print("\n== messages (logs, lines, example)")
    ordered = sorted(groups.items(), key=lambda item: (-len(item[1]["logs"]), -item[1]["lines"]))
    for key, entry in ordered:
        print("%3d %5d  %s   [e.g. %s]" % (len(entry["logs"]), entry["lines"], entry["example"][:170], entry["where"]))
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
