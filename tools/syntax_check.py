"""Syntax-check the port's C/C++ sources without MSVC (for Linux/cloud sessions).

Runs `clang -fsyntax-only` against MinGW-w64's Windows headers with flags that mimic the
MSVC build in CMakeLists.txt (32-bit, /Zc:wchar_t-, MS extensions/compatibility, the same
defines and include paths). It catches type errors, typos, and wrong API use; it does NOT
prove the MSVC build succeeds (MSVC and clang accept different corners of old C++), and it
never links or runs anything. MASM files are skipped.

The shader headers (CompiledVShader*.h / CompiledPShader*.h) are normally assembled on
Windows by tools/build_shaders.py; this writes empty stand-ins with the same array names.

Windows accepts #include names in any case and with backslashes, so the sources are compiled
from a mirror of ma/ and port/ (symlinks under build/syntax-check/tree) that adds aliases for
those spellings. Error paths are reported relative to the repository.

Requires clang and the MinGW-w64 i686 headers:
    apt-get install clang mingw-w64-i686-dev

Usage:
    python3 tools/syntax_check.py                  # every source in the CMake targets
    python3 tools/syntax_check.py FILE...          # just these files
    python3 tools/syntax_check.py --changed REF    # files changed since git REF (e.g. HEAD~1)
    python3 tools/syntax_check.py --baseline OUT   # also write the failing-file list to OUT
    python3 tools/syntax_check.py --known FILE     # report only failures not listed in FILE
"""
import argparse, concurrent.futures, os, re, shutil, subprocess, sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
OUT_DIR = os.path.join(ROOT, "build", "syntax-check")
TREE = os.path.join(OUT_DIR, "tree")
SYS_ALIAS = os.path.join(OUT_DIR, "sysalias")
MINGW_INCLUDE = "/usr/i686-w64-mingw32/include"
SOURCE_ROOTS = ("ma", "port")

# Pre-included in every translation unit.
PRELUDE = """\
// /Zc:wchar_t-: wchar_t is a typedef of unsigned short, not a builtin type.
#ifndef MA_SYNTAX_NATIVE_WCHAR
typedef unsigned short wchar_t;
#define _WCHAR_T_DEFINED
#endif
// MinGW's crtdbg.h does not declare the MSVC debug report function Fang's asserts call.
#ifdef __cplusplus
extern "C" {
#endif
int __cdecl _CrtDbgReport( int, const char *, int, const char *, const char *, ... );
#ifdef __cplusplus
}
#endif
// MSVC's runtime headers bring in offsetof everywhere; MinGW's only define it in stddef.h.
#include <stddef.h>
"""

INCLUDES = [
    "port/compat", "port", "ma/Lib/SmallAMX", "ma/Lib/Fang2", "ma/Lib/Fang2/dx",
    "ma/Lib/Fang2/dx/win", "ma/Lib/Bink",
]
GAME_INCLUDES = ["ma/App/ma", "ma/App/ma/win"]
DEFINES = [
    "WIN32", "_WINDOWS", "_CRT_SECURE_NO_WARNINGS", "_CRT_NONSTDC_NO_WARNINGS", "NOMINMAX",
    "_FANGDEF_PLATFORM_WIN", "_FANGDEF_ENABLE_DEV_FEATURES", "_FANGDEF_WINGC", "_DEBUG",
]

# MSVC accepts these; clang rejects them by default.
LENIENCY = [
    "-fdelayed-template-parsing",     # templates are checked when instantiated, as MSVC does
    "-Wno-invalid-token-paste",       # ## producing a non-token (MSVC just concatenates)
    "-Wno-address-of-temporary",      # &Type( ... ) in fdraw.h
    "-Wno-c++11-narrowing",           # narrowing in braced initializers (a warning in MSVC)
]

_cmake_src_re = re.compile(r"\b(ma/[^\s)]+\.(?:cpp|c))\b")
_shader_inc_re = re.compile(r'#\s*include\s+"(Compiled([VP])Shader(\w+))\.h"')
_include_re = re.compile(r'^[ \t]*#[ \t]*include[ \t]*([<"])([^>"\r\n]+)[>"]', re.M)
_SOURCE_EXTS = (".c", ".cpp", ".h", ".hpp", ".inl")
# GCC's C++ library always specializes its traits for wchar_t, which clashes with wchar_t as a
# typedef of unsigned short. Files that include it are checked with a native wchar_t instead.
_cxx_std_include_re = re.compile(r"^[ \t]*#[ \t]*include[ \t]*<[a-z_]+>", re.M)


# Standard C++ rejects a few things MSVC's permissive mode accepts and no clang flag relaxes.
# These rewrites apply to the mirror's copies only; the sources are never modified.
_default_arg_ctx_re = re.compile(r"[(,]\s*[\w:<>\s\*&]*$")


def _fix_default_arg(match):
    """"u32& Name=Name": the default argument names the global the parameter shadows."""
    line_start = match.string.rfind("\n", 0, match.start()) + 1
    if not _default_arg_ctx_re.search(match.string[line_start:match.start()]):
        return match.group(0)   # not in a parameter list
    return match.expand(r"\1\2=\3\4::\1")


REWRITES = [
    # "T *CNiIterator<T *>::s_ReturnError;": an explicit specialization without "template<>".
    (re.compile(r"^([A-Za-z_][^\n;(]*?\b\w+<[^\n;]*>::s_\w+\s*;)", re.M), lambda m: "template<> " + m.group(1)),
    (re.compile(r"\b(\w+)(\s*)=(\s*)(&?)\1\b(?!\s*\()"), _fix_default_arg),
]


class Mirror:
    """Symlink mirror of the source roots with aliases for Windows-style include names."""

    def __init__(self):
        self.entries = {}         # mirror dir -> {lower-case name: actual name}

    def build(self):
        if os.path.lexists(TREE):
            shutil.rmtree(TREE)
        if os.path.lexists(SYS_ALIAS):
            shutil.rmtree(SYS_ALIAS)
        os.makedirs(SYS_ALIAS)
        for top in SOURCE_ROOTS:
            for dirpath, dirs, files in os.walk(os.path.join(ROOT, top)):
                mdir = self.mirror_path(dirpath)
                os.makedirs(mdir, exist_ok=True)
                self.entries[mdir] = {n.lower(): n for n in dirs + files}
                for n in files:
                    self._link_or_rewrite(os.path.join(dirpath, n), os.path.join(mdir, n))
        include_dirs = [self.mirror_path(os.path.join(ROOT, d)) for d in INCLUDES + GAME_INCLUDES]
        for mdir, names in list(self.entries.items()):
            for n in names.values():
                if n.lower().endswith(_SOURCE_EXTS):
                    self._alias_includes(os.path.join(mdir, n), mdir, include_dirs)

    @staticmethod
    def _link_or_rewrite(real, mirrored):
        if real.lower().endswith(_SOURCE_EXTS):
            with open(real, encoding="latin1", newline="") as f:
                text = f.read()
            patched = text
            for pattern, fix in REWRITES:
                patched = pattern.sub(fix, patched)
            if patched != text:
                with open(mirrored, "w", encoding="latin1", newline="") as f:
                    f.write(patched)
                return
        os.symlink(real, mirrored)

    @staticmethod
    def mirror_path(real):
        return os.path.join(TREE, os.path.relpath(real, ROOT))

    def _resolve(self, base, parts):
        """Case-insensitive walk of the path parts from base; returns the real-cased path."""
        cur = base
        for part in parts:
            if part == "..":
                cur = os.path.dirname(cur)
                continue
            actual = self.entries.get(cur, {}).get(part.lower())
            if actual is None:
                return None
            cur = os.path.join(cur, actual)
        return cur

    def _add_alias(self, base, parts):
        """Adds a symlink for each path component spelled with a different case."""
        cur = base
        for part in parts:
            if part == "..":
                cur = os.path.dirname(cur)
                continue
            actual = self.entries.get(cur, {}).get(part.lower(), part)
            if actual != part and not os.path.lexists(os.path.join(cur, part)):
                os.symlink(actual, os.path.join(cur, part))
            cur = os.path.join(cur, actual)

    def _alias_includes(self, path, mdir, include_dirs):
        try:
            with open(path, encoding="latin1") as f:
                text = f.read()
        except OSError:
            return
        for kind, spelled in _include_re.findall(text):
            # clang, like MSVC, treats backslashes in include names as separators.
            normalized = spelled.strip().replace("\\", "/")
            parts = [p for p in normalized.split("/") if p not in ("", ".")]
            bases = ([mdir] if kind == '"' else []) + include_dirs
            if not parts or any(os.path.exists(os.path.join(b, normalized)) for b in bases):
                continue
            for b in bases:
                if self._resolve(b, parts):
                    self._add_alias(b, parts)
                    break
            else:
                if kind == "<" and not os.path.exists(os.path.join(MINGW_INCLUDE, normalized)):
                    self._alias_system(parts)

    @staticmethod
    def _alias_system(parts):
        """<Xinput.h> -> MinGW's xinput.h."""
        cur = MINGW_INCLUDE
        for part in parts:
            try:
                match = {n.lower(): n for n in os.listdir(cur)}.get(part.lower())
            except OSError:
                return
            if match is None:
                return
            cur = os.path.join(cur, match)
        alias = os.path.join(SYS_ALIAS, *parts)
        if not os.path.lexists(alias):
            os.makedirs(os.path.dirname(alias), exist_ok=True)
            os.symlink(cur, alias)


def cmake_sources():
    """Sources of the smallamx, fang2, ma_game and ma_port targets."""
    files = []
    for name in ("sources_fang2.cmake", "sources_ma.cmake"):
        with open(os.path.join(ROOT, "cmake", name)) as f:
            files += _cmake_src_re.findall(f.read())
    files = [f for f in files if not f.startswith("ma/App/ma/win/")]
    files += [
        "ma/Lib/SmallAMX/amx.c", "ma/Lib/SmallAMX/amxcons.c", "ma/Lib/SmallAMX/amxcore.c",
        "port/compat/d3d8_compat.cpp", "port/gcaudio.cpp", "port/gcdata.cpp", "port/gcmesh.cpp",
        "port/pc_input.cpp", "port/main_win.cpp", "port/screenshot_port.cpp",
    ]
    return files


def write_support_files():
    """Prelude, plus empty shader headers named as tools/build_shaders.py names the real ones."""
    shader_dir = os.path.join(OUT_DIR, "shaders")
    if os.path.lexists(shader_dir):
        shutil.rmtree(shader_dir)
    os.makedirs(os.path.join(shader_dir, "win"))
    with open(os.path.join(OUT_DIR, "prelude.h"), "w") as f:
        f.write(PRELUDE)

    def stub(path, symbol):
        with open(path, "w") as out:
            out.write("// Syntax-check stand-in; the real header comes from tools/build_shaders.py.\n")
            out.write("const DWORD %s[] = { 0 };\n" % symbol)

    dx = os.path.join(ROOT, "ma", "Lib", "Fang2", "dx")
    for fn in os.listdir(dx):
        base, ext = os.path.splitext(fn)
        cap = base[0].upper() + base[1:]
        if ext.lower() == ".nvv":
            stub(os.path.join(shader_dir, "CompiledVShader%s.h" % base), "dw%sVertexShader" % cap)
        elif ext.lower() == ".nvp":
            stub(os.path.join(shader_dir, "CompiledPShader%s.h" % base), "dw%sPixelShader" % cap)
        elif fn.lower() == "fdxsh_psprite.vsh":
            stub(os.path.join(shader_dir, "win", "fdxsh_psprite.cvs"), "dw%sVertexShader" % cap)

    # The sources include some of these with a different case than the shader file names.
    generated = {n.lower(): n for n in os.listdir(shader_dir)}
    for dirpath, _, names in os.walk(dx):
        for n in names:
            if not n.lower().endswith((".cpp", ".h")):
                continue
            with open(os.path.join(dirpath, n), encoding="latin1") as src:
                for header, _, _ in _shader_inc_re.findall(src.read()):
                    actual = generated.get((header + ".h").lower())
                    alias = os.path.join(shader_dir, header + ".h")
                    if actual and not os.path.lexists(alias):
                        os.symlink(actual, alias)
    return shader_dir


def command(path, shader_dir):
    is_c = path.endswith(".c")
    cmd = [
        "clang" if is_c else "clang++", "-fsyntax-only", "--target=i686-w64-mingw32",
        "-fms-extensions", "-fms-compatibility", "-fms-compatibility-version=19.44", "-fgnuc-version=4.2.1",
        "-Xclang", "-fno-wchar", "-w", "-ferror-limit=20",
        "-include", os.path.join(OUT_DIR, "prelude.h"),
    ] + LENIENCY
    native_wchar = False
    if not is_c:
        with open(os.path.join(ROOT, path), encoding="latin1") as f:
            native_wchar = bool(_cxx_std_include_re.search(f.read()))
    if native_wchar:
        cmd.remove("-fno-wchar")
        cmd.remove("-Xclang")
        cmd.append("-DMA_SYNTAX_NATIVE_WCHAR")
    if is_c:
        cmd += ["-include", "stdint.h",   # amx.h only includes it for compilers it knows
                "-Wno-int-conversion", "-Wno-implicit-function-declaration", "-Wno-implicit-int"]
    else:
        cmd.append("-std=c++14")
    cmd += ["-D" + d for d in DEFINES]
    if path.startswith(("ma/Lib/Fang2/", "ma/App/", "port/")):
        cmd.append("-DMA_PC_INPUT")
    incs = INCLUDES + (GAME_INCLUDES if path.startswith(("ma/App/", "port/main_win", "port/screenshot")) else [])
    cmd += ["-I" + os.path.join(TREE, i) for i in incs]
    cmd += ["-I" + shader_dir]
    cmd += ["-isystem", SYS_ALIAS, "-isystem", MINGW_INCLUDE, os.path.join(TREE, path)]
    return cmd


def check(path, shader_dir):
    proc = subprocess.run(command(path, shader_dir), cwd=ROOT, capture_output=True, text=True, errors="replace")
    errors = [l.replace(TREE + "/", "").replace(ROOT + "/", "")
              for l in proc.stderr.splitlines() if ": error:" in l or "fatal error:" in l]
    return path, proc.returncode == 0, errors


def changed_files(ref):
    out = subprocess.run(["git", "diff", "--name-only", ref, "--"], cwd=ROOT, capture_output=True, text=True, check=True).stdout
    return [f for f in out.split() if f.endswith((".c", ".cpp"))]


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("files", nargs="*")
    ap.add_argument("--changed", metavar="REF", help="check C/C++ files changed since this git ref")
    ap.add_argument("--baseline", metavar="OUT", help="write the list of failing files here")
    ap.add_argument("--known", metavar="FILE", help="known failures (one path per line) to leave out of the result")
    ap.add_argument("-j", "--jobs", type=int, default=os.cpu_count() or 4)
    ap.add_argument("-v", "--verbose", action="store_true", help="print every error line, not just the first few")
    args = ap.parse_args()

    if not os.path.isfile(os.path.join(MINGW_INCLUDE, "windows.h")):
        sys.exit("MinGW-w64 headers not found at %s (apt-get install mingw-w64-i686-dev)" % MINGW_INCLUDE)

    files = args.files or (changed_files(args.changed) if args.changed else cmake_sources())
    files = [os.path.relpath(os.path.abspath(f), ROOT).replace(os.sep, "/") for f in files]
    known = set()
    if args.known:
        with open(args.known) as f:
            known = {l.strip() for l in f if l.strip() and not l.startswith("#")}

    shader_dir = write_support_files()
    mirror = Mirror()
    mirror.build()
    failed = []
    with concurrent.futures.ThreadPoolExecutor(args.jobs) as pool:
        for path, ok, errors in pool.map(lambda p: check(p, shader_dir), files):
            if ok:
                continue
            failed.append(path)
            if path in known:
                continue
            print("FAIL %s" % path)
            for line in errors if args.verbose else errors[:3]:
                print("    " + line)

    if args.baseline:
        with open(args.baseline, "w") as f:
            f.write("".join(p + "\n" for p in sorted(failed)))
    new = [p for p in failed if p not in known]
    print("%d file(s) checked, %d passed, %d failed%s" % (
        len(files), len(files) - len(failed), len(failed),
        " (%d not in the known list)" % len(new) if args.known else ""))
    return 1 if new else 0


if __name__ == "__main__":
    sys.exit(main())
