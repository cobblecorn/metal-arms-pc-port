"""Differential test of Fang's two Windows math implementations.

The port builds the runtime with _FANGDEF_WINGC (GameCube struct layout), which uses the scalar
dx/fdx8gcmath_*.inl. That code was tools-only in the original project, so it was never exercised
by the shipped game. The shipped PC/Xbox code, dx/fdx8math_*.inl (SSE/x87), implements the same
aligned classes (CFVec3A, CFVec4A, CFMtx43A, CFMtx44A, CFQuatA, CFTQuatA).

This generates a program that calls every inline method both files define on the same
deterministic inputs and prints the results, builds it twice (with and without _FANGDEF_WINGC)
as 32-bit Linux code, runs both, and reports methods whose results differ.

The SSE build needs two adjustments for clang (tools/mathdiff/shim):
- xmmintrin.h: MSVC's union __m128 (the code indexes m128_f32[]), with the intrinsics mapped to
  clang builtins, so results are the real SSE results;
- in MSVC inline assembly "mov eax, rV" with a reference parameter loads the reference (the
  object's address), but clang loads the object's first dword. The PC math files are copied
  with those instructions rewritten to "lea eax, rV", which means the same thing under clang.

Requires clang with 32-bit support (apt-get install clang gcc-multilib g++-multilib).

Usage:
    python3 tools/mathdiff/mathdiff.py [--cases N] [--filter REGEX] [--keep]
Output goes to build/mathdiff/ (report.txt). Exit status 1 when a method differs in a way
KNOWN_DIFFERENCES doesn't explain.
"""
import argparse, math, os, re, shutil, subprocess, sys, zlib

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(os.path.dirname(HERE))
FANG = os.path.join(ROOT, "ma", "Lib", "Fang2")
OUT = os.path.join(ROOT, "build", "mathdiff")

# Out-of-line code the inline methods call.
LINK_SOURCES = ("fmath.cpp", "fmath_rand.cpp", "fsintbl.cpp", "dx/fdx8math.cpp", "dx/fdx8math_vec.cpp", "dx/fdx8math_mtx.cpp", "dx/fdx8math_quat.cpp")
# Both implementations contain MSVC inline assembly (the GC-layout quaternions use SSE too).
REWRITTEN = (["dx/fdx8math_%s.inl" % f for f in ("vec", "mtx", "quat")] + ["dx/fdx8gcmath_%s.inl" % f for f in ("vec", "mtx", "quat")]
             + [s for s in LINK_SOURCES if s.startswith("dx/")])

# Differences checked against the retail GameCube code (gc/fGCmath_*.inl), which is what the
# GC-layout build should match. Reported separately; they don't fail the run.
_XZ_Y = "retail GC sets y = 0 (as the GC layout does); the SSE code copies y from the argument"
_NO_GC = "retail GC doesn't define this for CFVec4A (commented out), so game code can't rely on it"
_SSE_BUG_LANES = "SSE bug: loads [eax] instead of [eax+8] for z and w; the GC layout matches retail GC"
KNOWN_DIFFERENCES = {
    "CFMtx43A::Mul33(const CFMtx43A &, const f32 &)": "retail GC copies rM's position row (as the GC layout does); SSE leaves its own",
    "CFMtx44A::MulDir(CFVec4A &) const": _SSE_BUG_LANES,
    "CFMtx44A::MulDir(CFVec4A &, const CFVec4A &) const": _SSE_BUG_LANES,
    "CFMtx44A::MulPoint(CFVec4A &) const": _SSE_BUG_LANES,
    "CFMtx44A::MulPoint(CFVec4A &, const CFVec4A &) const": _SSE_BUG_LANES,
    "CFVec3A::SafeUnitAndInvMagXZ(const CFVec3A &)": _XZ_Y,
    "CFVec3A::SafeUnitAndMagXZ(const CFVec3A &)": _XZ_Y,
    "CFVec3A::UnitAndMagXZ(const CFVec3A &)": _XZ_Y,
    "CFVec4A::SafeUnitAndInvMagXZ(const CFVec4A &)": _NO_GC,
    "CFVec4A::SafeUnitAndMagXZ(const CFVec4A &)": _NO_GC,
    "CFVec4A::UnitAndInvMagXZ(const CFVec4A &)": _NO_GC,
    "CFVec4A::UnitAndMagXZ(const CFVec4A &)": _NO_GC,
    "CFVec4A::Sub(const CFVec4A &, const f32 &)": "SSE bug: computes fVal - rV; retail GC and the GC layout compute rV - fVal",
}

# Out-of-line methods with separate SSE and GC-layout versions (dx/fdx8math_mtx.cpp).
EXTRA_METHODS = [
    "FINLINE CFMtx43A &CFMtx43A::ReceiveInverse( const CFMtx43A &rM ) {",
    "FINLINE CFMtx43A &CFMtx43A::Invert( void ) {",
]

# Engine functions the math sources reference but the test never needs.
LINK_STUBS = r"""
#include <stdarg.h>
#include <stdio.h>
extern "C++" int fang_DevPrintf( const char *pszFormat, ... ) { va_list a; va_start( a, pszFormat ); int n = vfprintf( stderr, pszFormat, a ); va_end( a ); return n; }
"""

CLASSES = ("CFVec3A", "CFVec4A", "CFMtx43A", "CFMtx44A", "CFQuatA", "CFTQuatA")
FILES = ("vec", "mtx", "quat")
ARG_TYPES = ("f32", "u32", "s32", "BOOL", "CFVec2", "CFVec3", "CFVec4", "CFVec3A", "CFVec4A",
             "CFMtx33", "CFMtx43", "CFMtx44", "CFMtx43A", "CFMtx44A", "CFQuatA", "CFTQuatA")
KNOWN_TYPES = set(ARG_TYPES) | {"void", "int", "float", "CFQuat", "CFSphere", "CFVec3A_t"}

_def_re = re.compile(
    r"FINLINE\s+(?P<ret>[\w\s&*:]*?)\s*\b(?P<cls>" + "|".join(CLASSES) + r")::"
    r"(?P<name>operator\s*(?:\(\s*\)|\[\s*\]|[^\s(]+)|~?\w+)\s*"
    r"\((?P<params>[^)]*)\)\s*(?P<const>const)?\s*\{")


class Param:
    def __init__(self, text):
        text = text.split("=")[0].strip()
        tokens = re.findall(r"[A-Za-z_]\w*|[&*]", text)
        self.name = ""
        if len(tokens) > 1 and re.match(r"[A-Za-z_]", tokens[-1]) and tokens[-1] not in KNOWN_TYPES:
            self.name = tokens.pop()
        self.const = "const" in tokens
        self.ref = "&" in tokens
        self.ptr = "*" in tokens
        base = [t for t in tokens if t not in ("const", "&", "*")]
        self.base = base[0] if len(base) == 1 else " ".join(base)

    def key(self):
        return "%s%s%s%s" % ("const " if self.const else "", self.base, " &" if self.ref else "", " *" if self.ptr else "")

    def is_output(self):
        return self.ref and not self.const


class Method:
    def __init__(self, m):
        self.cls = m.group("cls")
        self.name = re.sub(r"\s+", "", m.group("name"))
        self.ret = re.sub(r"\s+", " ", m.group("ret")).strip()
        params = m.group("params").strip()
        self.params = [] if params in ("", "void") else [Param(p) for p in params.split(",")]
        self.const = bool(m.group("const"))

    def key(self):
        return (self.cls, self.name, tuple(p.key() for p in self.params), self.const)

    def label(self):
        return "%s::%s(%s)%s" % (self.cls, self.name, ", ".join(p.key() for p in self.params), " const" if self.const else "")


def read(path):
    with open(path, encoding="latin1") as f:
        return f.read().replace("\r\n", "\n")


def strip_disabled(text):
    """Blanks out #if 0 regions (honouring #else), keeping line numbers."""
    out, stack = [], []   # stack entries: True while the region is disabled
    for line in text.split("\n"):
        d = re.match(r"\s*#\s*(if|ifdef|ifndef|elif|else|endif)\b\s*(.*)", line)
        disabled = any(stack)
        if d:
            kind, arg = d.group(1), d.group(2).strip()
            if kind in ("if", "ifdef", "ifndef"):
                stack.append(kind == "if" and re.match(r"0\b", arg) is not None)
            elif kind == "else" and stack:
                stack[-1] = not stack[-1] if not any(stack[:-1]) else stack[-1]
            elif kind == "endif" and stack:
                stack.pop()
            out.append("")
            continue
        out.append("" if disabled else line)
    return "\n".join(out)


def parse(path):
    return {m.key(): m for m in (Method(x) for x in _def_re.finditer(strip_disabled(read(path))))}


# Out-of-line definitions too, for rewriting the .cpp files.
_any_def_re = re.compile(
    r"^(?:FINLINE\s+)?(?P<ret>[\w\s&*:]*?)\s*\b(?P<cls>\w+)::"
    r"(?P<name>operator\s*(?:\(\s*\)|\[\s*\]|[^\s(]+)|~?\w+)\s*"
    r"\((?P<params>[^)]*)\)\s*(?P<const>const)?\s*\{", re.M)


def function_bodies(text):
    """Yields (match, body start, body end) for each member function definition."""
    for m in _any_def_re.finditer(text):
        depth, i = 0, m.end() - 1
        while i < len(text):
            if text[i] == "{":
                depth += 1
            elif text[i] == "}":
                depth -= 1
                if depth == 0:
                    break
            i += 1
        yield m, m.end(), i


_asm_block_re = re.compile(r"__asm\s*\{(.*?)\}", re.S)


def rewrite_ref_loads(text):
    """mov reg, <reference param> -> lea reg, <reference param> inside __asm blocks (see the doc)."""
    out, last, count = [], 0, 0
    for m, start, end in function_bodies(text):
        refs = [p.name for p in Method(m).params if p.ref and p.name]
        if not refs:
            continue
        body = text[start:end]

        def fix_block(b):
            nonlocal count
            block = b.group(0)
            pattern = re.compile(r"\bmov(\s+)(e[a-z]{2})(\s*),(\s*)(" + "|".join(map(re.escape, refs)) + r")\b(?!\s*[\[.+])")
            block, n = pattern.subn(r"lea\1\2\3,\4\5", block)
            count += n
            return block

        out.append(text[last:start])
        out.append(_asm_block_re.sub(fix_block, body))
        last = end
    out.append(text[last:])
    return "".join(out), count


def build_mirror():
    """Symlink mirror of Fang2 with the PC math files' reference loads rewritten."""
    mirror = os.path.join(OUT, "fang2")
    if os.path.lexists(mirror):
        shutil.rmtree(mirror)
    rewritten = 0
    for dirpath, _, names in os.walk(FANG):
        mdir = os.path.join(mirror, os.path.relpath(dirpath, FANG))
        os.makedirs(mdir, exist_ok=True)
        for n in names:
            src = os.path.join(dirpath, n)
            if os.path.relpath(src, FANG).replace(os.sep, "/") in REWRITTEN:
                text, n_fixed = rewrite_ref_loads(read(src))
                rewritten += n_fixed
                with open(os.path.join(mdir, n), "w", encoding="latin1") as f:
                    f.write(text)
            else:
                os.symlink(src, os.path.join(mdir, n))
    return mirror, rewritten


# ---------------------------------------------------------------------------------------------
# Test program generation
# ---------------------------------------------------------------------------------------------

PRELUDE = r"""
#include "fang.h"
#include "fmath.h"
#include <math.h>
#include <setjmp.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>

// A method that crashes is reported as "crash" and the run continues with the next one.
static sigjmp_buf g_Jmp;
static void OnCrash( int nSig ) { siglongjmp( g_Jmp, nSig ); }

static u32 g_nSeed;
static f32 R( f32 fLo, f32 fHi ) {
	g_nSeed = g_nSeed * 1664525u + 1013904223u;
	return fLo + ( fHi - fLo ) * ( (f32)( g_nSeed >> 8 ) * ( 1.0f / 16777216.0f ) );
}

// Hints (from parameter names): 0 general, 1 [-1,1], 2 [0,1], 3 radians, 4 positive scale,
// 5 unit vector, 6 bit mask, 7 small index, 8 zero (edge case)
static f32 F( int h ) {
	switch( h ) {
	case 1: return R( -1.0f, 1.0f );
	case 2: return R( 0.0f, 1.0f );
	case 3: return R( -3.0f, 3.0f );
	case 4: return R( 0.25f, 3.0f );
	case 8: return 0.0f;
	default: return R( -5.0f, 5.0f );
	}
}
static void Unit3( f32 *p ) { f32 m; do { p[0] = R( -1, 1 ); p[1] = R( -1, 1 ); p[2] = R( -1, 1 ); m = p[0]*p[0] + p[1]*p[1] + p[2]*p[2]; } while( m < 0.01f ); m = 1.0f / sqrtf( m ); p[0] *= m; p[1] *= m; p[2] *= m; }
static void Quat( f32 *p ) { f32 m; do { p[0] = R( -1, 1 ); p[1] = R( -1, 1 ); p[2] = R( -1, 1 ); p[3] = R( -1, 1 ); m = p[0]*p[0] + p[1]*p[1] + p[2]*p[2] + p[3]*p[3]; } while( m < 0.01f ); m = 1.0f / sqrtf( m ); for( int i = 0; i < 4; i++ ) p[i] *= m; }
// Rotation (rows) from a unit quaternion, times fScale.
static void Rot( f32 r[3][3], f32 fScale ) {
	f32 q[4]; Quat( q ); f32 x = q[0], y = q[1], z = q[2], w = q[3];
	f32 m[3][3] = { { 1 - 2*(y*y + z*z), 2*(x*y + w*z), 2*(x*z - w*y) },
					{ 2*(x*y - w*z), 1 - 2*(x*x + z*z), 2*(y*z + w*x) },
					{ 2*(x*z + w*y), 2*(y*z - w*x), 1 - 2*(x*x + y*y) } };
	for( int i = 0; i < 3; i++ ) for( int j = 0; j < 3; j++ ) r[i][j] = m[i][j] * fScale;
}

static void G( f32 &v, int h ) { v = F( h ); }
static void G( u32 &v, int h ) { v = ( h == 6 ) ? ( g_nSeed = g_nSeed * 1664525u + 1013904223u ) : (u32)R( 0.0f, 2.99f ); }
static void G( s32 &v, int h ) { v = (s32)R( 0.0f, 2.99f ); }
static void G( CFVec2 &v, int h ) { v.x = F( h ); v.y = F( h ); }
static void G( CFVec3 &v, int h ) { if( h == 5 ) Unit3( &v.x ); else { v.x = F( h ); v.y = F( h ); v.z = F( h ); } }
static void G( CFVec4 &v, int h ) { if( h == 5 ) { Unit3( &v.x ); v.w = 0.0f; } else { v.x = F( h ); v.y = F( h ); v.z = F( h ); v.w = F( h ); } }
static void G( CFVec3A &v, int h ) { if( h == 5 ) Unit3( &v.x ); else { v.x = F( h ); v.y = F( h ); v.z = F( h ); } v.w = 0.0f; }
static void G( CFVec4A &v, int h ) { if( h == 5 ) { Unit3( &v.x ); v.w = 0.0f; } else { v.x = F( h ); v.y = F( h ); v.z = F( h ); v.w = F( h ); } }
static void G( CFQuatA &q, int h ) { if( h == 8 ) { q.x = q.y = q.z = 0.0f; q.w = 1.0f; } else Quat( &q.x ); }
static void G( CFTQuatA &q, int h ) { G( (CFQuatA &)q, h ); q.m_Pos.x = F( h ); q.m_Pos.y = F( h ); q.m_Pos.z = F( h ); q.m_fScale = ( h == 8 ) ? 1.0f : R( 0.5f, 2.0f ); }
static void G( CFMtx33 &m, int h ) { f32 r[3][3]; Rot( r, 1.0f ); memcpy( m.aa, r, sizeof( r ) ); }
static void G( CFMtx43 &m, int h ) { f32 r[3][3]; Rot( r, 1.0f ); for( int i = 0; i < 3; i++ ) for( int j = 0; j < 3; j++ ) m.aa[i][j] = r[i][j]; m.m_vPos.x = F( 0 ); m.m_vPos.y = F( 0 ); m.m_vPos.z = F( 0 ); }
static void G( CFMtx44 &m, int h ) { f32 r[3][3]; Rot( r, 1.0f ); for( int i = 0; i < 3; i++ ) { for( int j = 0; j < 3; j++ ) m.aa[i][j] = r[i][j]; m.aa[i][3] = 0.0f; } m.aa[3][0] = F( 0 ); m.aa[3][1] = F( 0 ); m.aa[3][2] = F( 0 ); m.aa[3][3] = 1.0f; }
// Matrices are rotations (scaled by hint 4) plus a translation, built on each layout's identity.
static void G( CFMtx43A &m, int h ) { m.Identity(); f32 r[3][3]; Rot( r, h == 4 ? R( 0.5f, 2.0f ) : 1.0f ); for( int i = 0; i < 3; i++ ) for( int j = 0; j < 3; j++ ) m.aa[i][j] = r[i][j]; m.m_vPos.x = F( 0 ); m.m_vPos.y = F( 0 ); m.m_vPos.z = F( 0 ); }
static void G( CFMtx44A &m, int h ) { m.Identity(); f32 r[3][3]; Rot( r, h == 4 ? R( 0.5f, 2.0f ) : 1.0f ); for( int i = 0; i < 3; i++ ) for( int j = 0; j < 3; j++ ) m.aa[i][j] = r[i][j]; m.m_vPos.x = F( 0 ); m.m_vPos.y = F( 0 ); m.m_vPos.z = F( 0 ); }

static void P( const char *pszId, const char *pszWhat, int n, const f32 *pf ) {
	printf( "%s|%s:", pszId, pszWhat );
	for( int i = 0; i < n; i++ ) printf( " %.9g", pf[i] );
	printf( "\n" );
}
static void D( const char *i, const char *w, f32 v ) { P( i, w, 1, &v ); }
static void D( const char *i, const char *w, int v ) { f32 f = (f32)v; P( i, w, 1, &f ); }
static void D( const char *i, const char *w, u32 v ) { f32 f = (f32)v; P( i, w, 1, &f ); }
static void D( const char *i, const char *w, const CFVec2 &v ) { P( i, w, 2, &v.x ); }
static void D( const char *i, const char *w, const CFVec3 &v ) { P( i, w, 3, &v.x ); }
static void D( const char *i, const char *w, const CFVec4 &v ) { P( i, w, 4, &v.x ); }
static void D( const char *i, const char *w, const CFVec3A &v ) { P( i, w, 3, &v.x ); }
static void D( const char *i, const char *w, const CFVec4A &v ) { P( i, w, 4, &v.x ); }
static void D( const char *i, const char *w, const CFQuatA &q ) { P( i, w, 4, &q.x ); }
static void D( const char *i, const char *w, const CFTQuatA &q ) { f32 f[8] = { q.x, q.y, q.z, q.w, q.m_Pos.x, q.m_Pos.y, q.m_Pos.z, q.m_fScale }; P( i, w, 8, f ); }
static void D( const char *i, const char *w, const CFMtx33 &m ) { P( i, w, 9, &m.aa[0][0] ); }
static void D( const char *i, const char *w, const CFMtx43 &m ) { P( i, w, 12, &m.aa[0][0] ); }
static void D( const char *i, const char *w, const CFMtx44 &m ) { P( i, w, 16, &m.aa[0][0] ); }
static void D( const char *i, const char *w, const CFMtx43A &m ) { f32 f[12]; for( int r = 0; r < 4; r++ ) for( int c = 0; c < 3; c++ ) f[r*3 + c] = m.aa[r][c]; P( i, w, 12, f ); }
static void D( const char *i, const char *w, const CFMtx44A &m ) { P( i, w, 16, &m.aa[0][0] ); }
"""

_hint_rules = [
    (r"Cos|Sin|Dot", 1), (r"Unit|Lerp|Percent|^fT$|Slerp", 2), (r"Radian|Angle|Yaw|Pitch|Roll", 3),
    (r"Scale|Mag|Dist|Radius|Len", 4), (r"Mask", 6), (r"Index|Axis|Component|^n", 7),
]


def hint_for(param, method, case, edge):
    if edge:
        return 8
    name = param.name
    if param.base in ("CFVec3", "CFVec4", "CFVec3A", "CFVec4A") and re.search(r"Unit|Dir|Normal|Axis", name):
        return 5
    for pattern, h in _hint_rules:
        if re.search(pattern, name):
            return h
    if param.base in ("CFMtx43A", "CFMtx44A") and case == 3:
        return 4
    return 0


def self_hint(method, case, edge):
    if edge:
        return 8
    if method.cls in ("CFMtx43A", "CFMtx44A") and case == 3:
        return 4
    if method.cls in ("CFVec3A", "CFVec4A") and re.search(r"Unit(?!ize)|Slerp", method.name) and "Unit" in method.name and "Safe" not in method.name:
        return 0
    return 0


def supported(method):
    if method.name in (method.cls, "~" + method.cls) or method.name.startswith("operator") and method.name in ("operatornew", "operatordelete"):
        return False
    if "ChangeEndian" in method.name or "*" in method.ret:
        return False
    for p in method.params:
        if p.ptr and p.base != "f32":
            return False
        if not p.ptr and p.base not in ARG_TYPES:
            return False
    return True


def call_name(method):
    return method.name if not method.name.startswith("operator") else "operator" + method.name[len("operator"):]


def generate(methods, cases):
    lines = [PRELUDE, "int main( void ) {",
             "\tsetvbuf( stdout, NULL, _IOLBF, 0 );",
             "\tfmath_ModuleStartup();   // fills the sine interpolation tables",
             "\tstruct sigaction sa; memset( &sa, 0, sizeof( sa ) ); sa.sa_handler = OnCrash; sa.sa_flags = SA_NODEFER;",
             "\tsigaction( SIGSEGV, &sa, NULL ); sigaction( SIGFPE, &sa, NULL ); sigaction( SIGILL, &sa, NULL ); sigaction( SIGBUS, &sa, NULL );"]
    for method in methods:
        for case in range(cases):
            edge = case == cases - 1
            tid = "%s #%d%s" % (method.label(), case, " edge" if edge else "")
            seed = zlib.crc32(tid.encode()) & 0x7FFFFFFF
            body = ["g_nSeed = %du;" % seed]
            body.append("%s self; G( self, %d );" % (method.cls, self_hint(method, case, edge)))
            args = []
            for i, p in enumerate(method.params):
                var = "a%d" % i
                if p.ptr:
                    body.append("f32 %s[16]; for( int k = 0; k < 16; k++ ) %s[k] = F( %d );" % (var, var, 8 if edge else 0))
                else:
                    body.append("%s %s; G( %s, %d );" % (p.base, var, var, hint_for(p, method, case, edge)))
                args.append(var)
            # Keep generated bounds ordered (min <= max per component) for Clamp-style methods.
            names = [p.name for p in method.params]
            lo = [i for i, n in enumerate(names) if re.search(r"Min", n)]
            hi = [i for i, n in enumerate(names) if re.search(r"Max", n)]
            if len(lo) == 1 and len(hi) == 1 and method.params[lo[0]].base == method.params[hi[0]].base \
                    and method.params[lo[0]].base in ("CFVec3A", "CFVec4A"):
                body.append("for( int k = 0; k < 4; k++ ) if( a%d.a[k] > a%d.a[k] ) { f32 t = a%d.a[k]; a%d.a[k] = a%d.a[k]; a%d.a[k] = t; }"
                            % (lo[0], hi[0], lo[0], lo[0], hi[0], hi[0]))
            call = "self.%s( %s )" % (call_name(method), ", ".join(args))
            if method.ret in ("void", ""):
                body.append(call + ";")
            elif method.ret == "BOOL":
                body.append("int r = %s ? 1 : 0;   /* BOOL: compare truth, not the exact value */" % call)
                body.append("D( id, \"ret\", r );")
            else:
                body.append("auto r = %s;" % call)
                body.append("D( id, \"ret\", r );")
            if not method.const:
                body.append("D( id, \"self\", self );")
            for i, p in enumerate(method.params):
                if p.is_output() or (p.ptr and not p.const):
                    body.append("D( id, \"a%d\", %s );" % (i, "a%d" % i) if not p.ptr else "P( id, \"a%d\", 16, a%d );" % (i, i))
            lines.append("\t{ static const char *id = \"%s\"; if( !sigsetjmp( g_Jmp, 1 ) ) { %s } else printf( \"%%s|crash: nan\\n\", id ); }"
                         % (tid.replace('"', '\\"'), " ".join(body)))
    lines.append("\treturn 0;\n}")
    return "\n".join(lines) + "\n"


# ---------------------------------------------------------------------------------------------
# Build, run and compare
# ---------------------------------------------------------------------------------------------

def build(src, exe, mirror, gc_layout):
    flags = ["-m32", "-msse2", "-mfpmath=sse", "-O1", "-std=c++14", "-fms-extensions", "-fasm-blocks",
             "-w", "-D_FANGDEF_PLATFORM_WIN", "-D_DEBUG", "-I" + os.path.join(HERE, "shim"),
             "-I" + mirror, "-I" + os.path.join(mirror, "dx")]
    if gc_layout:
        flags.append("-D_FANGDEF_WINGC")
    objs = []
    for s in (src,) + tuple(os.path.join(mirror, l) for l in LINK_SOURCES):
        obj = "%s.%s.o" % (exe, os.path.basename(s))
        proc = subprocess.run(["clang++", "-c"] + flags + [s, "-o", obj], capture_output=True, text=True)
        if proc.returncode:
            sys.exit("compile failed (%s layout, %s):\n%s" % ("GC" if gc_layout else "SSE", s, proc.stderr[-6000:]))
        objs.append(obj)
    proc = subprocess.run(["clang++", "-m32"] + objs + [os.path.join(OUT, "link_stubs.o"), "-o", exe], capture_output=True, text=True)
    if proc.returncode:
        sys.exit("link failed (%s layout):\n%s" % ("GC" if gc_layout else "SSE", proc.stderr[-6000:]))


def run(exe):
    proc = subprocess.run([exe], capture_output=True, text=True, errors="replace")
    results = {}
    other = []
    for line in proc.stdout.splitlines():
        key, _, values = line.rpartition(":")
        try:
            if "|" not in key:
                raise ValueError
            results[key] = [float(v) for v in values.split()]
        except ValueError:
            other.append(line)   # not ours: something the code under test printed
    return results, proc.returncode, proc.stderr + "".join("stdout: %s\n" % l for l in other)


def close(a, b, rel, abs_tol):
    if math.isnan(a) or math.isnan(b):
        return math.isnan(a) and math.isnan(b)
    if math.isinf(a) or math.isinf(b):
        return a == b
    return abs(a - b) <= abs_tol + rel * max(abs(a), abs(b))


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--cases", type=int, default=6, help="input sets per method; the last is all zeros")
    ap.add_argument("--filter", help="only methods whose label matches this regex")
    ap.add_argument("--rel", type=float, default=4e-3, help="relative tolerance (SSE uses ~12-bit rcp/rsqrt)")
    ap.add_argument("--abs", type=float, default=1e-3, dest="abs_tol", help="absolute tolerance")
    ap.add_argument("--show-edge", action="store_true", help="also report differences on the all-zero inputs")
    args = ap.parse_args()

    os.makedirs(OUT, exist_ok=True)
    stubs = os.path.join(OUT, "link_stubs.cpp")
    with open(stubs, "w") as f:
        f.write(LINK_STUBS)
    subprocess.run(["clang++", "-m32", "-c", "-w", stubs, "-o", os.path.join(OUT, "link_stubs.o")], check=True)
    gc_defs, sse_defs = {}, {}
    for f in FILES:
        gc_defs.update(parse(os.path.join(FANG, "dx", "fdx8gcmath_%s.inl" % f)))
        sse_defs.update(parse(os.path.join(FANG, "dx", "fdx8math_%s.inl" % f)))
    common = [gc_defs[k] for k in gc_defs if k in sse_defs]
    common += [Method(m) for m in (_def_re.search(sig) for sig in EXTRA_METHODS) if m and Method(m).key() not in gc_defs]
    methods = [m for m in common if supported(m) and (not args.filter or re.search(args.filter, m.label()))]
    only_gc = sorted(gc_defs[k].label() for k in gc_defs if k not in sse_defs)
    only_sse = sorted(sse_defs[k].label() for k in sse_defs if k not in gc_defs)

    mirror, rewritten = build_mirror()
    src = os.path.join(OUT, "mathdiff_test.cpp")
    with open(src, "w") as f:
        f.write(generate(methods, args.cases))
    build(src, os.path.join(OUT, "test_sse"), mirror, False)
    build(src, os.path.join(OUT, "test_gc"), mirror, True)
    sse, rc_sse, err_sse = run(os.path.join(OUT, "test_sse"))
    gc, rc_gc, err_gc = run(os.path.join(OUT, "test_gc"))

    report = []
    diffs = {}
    for key in sorted(set(sse) | set(gc)):
        if " edge|" in key and not args.show_edge:
            continue
        a, b = sse.get(key), gc.get(key)
        if a is None or b is None or len(a) != len(b):
            diffs.setdefault(key.split(" #")[0], []).append((key, a, b, float("inf")))
            continue
        worst = 0.0
        for x, y in zip(a, b):
            if not close(x, y, args.rel, args.abs_tol):
                worst = max(worst, abs(x - y) if not (math.isnan(x) or math.isnan(y)) else float("inf"))
        if worst:
            diffs.setdefault(key.split(" #")[0], []).append((key, a, b, worst))

    report.append("methods compared: %d (of %d GC-layout / %d SSE definitions); asm reference loads rewritten: %d"
                  % (len(methods), len(gc_defs), len(sse_defs), rewritten))
    report.append("exit codes: sse %d, gc %d" % (rc_sse, rc_gc))
    for name, stderr in (("sse", err_sse), ("gc", err_gc)):
        asserts = sorted(set(l for l in stderr.splitlines() if l.startswith("ASSERT")))
        if asserts:
            report.append("%s asserts: %d distinct (first: %s)" % (name, len(asserts), asserts[0]))
    unexplained = sorted(n for n in diffs if n not in KNOWN_DIFFERENCES)
    explained = sorted(n for n in diffs if n in KNOWN_DIFFERENCES)
    report.append("methods that differ: %d unexplained, %d known (checked against retail GC)" % (len(unexplained), len(explained)))
    for name in unexplained + explained:
        report.append("\n%s%s" % (name, "  [known: %s]" % KNOWN_DIFFERENCES[name] if name in KNOWN_DIFFERENCES else ""))
        for key, a, b, worst in diffs[name][:3]:
            report.append("  %s  (max |diff| %.4g)" % (key.split("|", 1)[1] if "|" in key else key, worst))
            report.append("    sse: %s" % (" ".join("%.6g" % v for v in a) if a else "-"))
            report.append("    gc : %s" % (" ".join("%.6g" % v for v in b) if b else "-"))
    report.append("\nonly in fdx8gcmath (not compared): %d\n  %s" % (len(only_gc), "\n  ".join(only_gc)))
    report.append("only in fdx8math (not compared): %d\n  %s" % (len(only_sse), "\n  ".join(only_sse)))
    text = "\n".join(report) + "\n"
    with open(os.path.join(OUT, "report.txt"), "w") as f:
        f.write(text)
    print(text if len(text) < 20000 else text[:20000] + "\n... (see build/mathdiff/report.txt)")
    return 1 if unexplained else 0


if __name__ == "__main__":
    sys.exit(main())
