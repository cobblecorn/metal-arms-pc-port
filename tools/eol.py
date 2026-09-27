"""Check and fix line endings.

The original sources under ma/ are CRLF, and so are some port files; others (most of port/, tools/, docs)
are LF. Some editing tools (sed -i in Git Bash, some patch tools) silently turn a CRLF file into LF, or
leave the lines they touched with the other ending. That churns every line of the file in git.

usage: python tools/eol.py check [FILE...]   report files with mixed endings, or whose ending differs
                                             from the committed version; default: files changed vs HEAD
       python tools/eol.py fix [FILE...]     rewrite each file with the ending its committed version
                                             uses (or its own majority ending if it is new)
"""
import os
import subprocess
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))


def endings(data):
    crlf = data.count(b"\r\n")
    return crlf, data.count(b"\n") - crlf


def committed_ending(path):
    """'crlf', 'lf' or None (new file, or no line endings)."""
    relative = os.path.relpath(path, ROOT).replace("\\", "/")
    result = subprocess.run(["git", "show", "HEAD:" + relative], cwd=ROOT, capture_output=True)
    if result.returncode != 0:
        return None
    crlf, lf = endings(result.stdout)
    if crlf == 0 and lf == 0:
        return None
    return "crlf" if crlf >= lf else "lf"


def changed_files():
    result = subprocess.run(["git", "diff", "--name-only", "HEAD"], cwd=ROOT, capture_output=True, text=True)
    names = [line for line in result.stdout.splitlines() if line.strip()]
    return [os.path.join(ROOT, name) for name in names if os.path.isfile(os.path.join(ROOT, name))]


def main(argv):
    if not argv or argv[0] not in ("check", "fix"):
        print(__doc__)
        return 2
    files = [os.path.abspath(name) for name in argv[1:]] or changed_files()
    problems = 0
    for path in files:
        with open(path, "rb") as handle:
            data = handle.read()
        if b"\0" in data[:4096]:
            continue  # binary
        crlf, lf = endings(data)
        want = committed_ending(path) or ("crlf" if crlf >= lf else "lf")
        ok = (lf == 0) if want == "crlf" else (crlf == 0)
        if argv[0] == "check":
            if not ok:
                problems += 1
                print("%s: %d CRLF, %d LF lines; the committed file is %s" % (os.path.relpath(path, ROOT), crlf, lf, want.upper()))
        elif not ok:
            data = data.replace(b"\r\n", b"\n")
            if want == "crlf":
                data = data.replace(b"\n", b"\r\n")
            with open(path, "wb") as handle:
                handle.write(data)
            print("fixed %s -> %s" % (os.path.relpath(path, ROOT), want.upper()))
    if argv[0] == "check" and not problems:
        print("line endings ok (%d files)" % len(files))
    return 1 if problems else 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
