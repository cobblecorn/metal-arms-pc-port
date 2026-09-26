// MSVC-compatible <xmmintrin.h> for building Fang's SSE math with clang on Linux.
//
// MSVC's __m128 is a union with m128_f32[] (and friends) that the Fang code indexes directly;
// clang's is a vector type. This declares MSVC's form and implements the intrinsics Fang uses
// with clang builtins, so the results are the real SSE instruction results (including the
// approximate rcpps/rsqrtps).
#pragma once

typedef float _mathdiff_v4sf __attribute__(( vector_size( 16 ) ));
typedef int _mathdiff_v4si __attribute__(( vector_size( 16 ) ));

typedef union __attribute__(( aligned( 16 ) )) __m128 {
	float m128_f32[4];
	unsigned int m128_u32[4];
	int m128_i32[4];
	_mathdiff_v4sf v;
} __m128;

static inline __m128 _mathdiff_wrap( _mathdiff_v4sf v ) { __m128 r; r.v = v; return r; }

static inline __m128 _mm_add_ps( __m128 a, __m128 b ) { return _mathdiff_wrap( a.v + b.v ); }
static inline __m128 _mm_sub_ps( __m128 a, __m128 b ) { return _mathdiff_wrap( a.v - b.v ); }
static inline __m128 _mm_mul_ps( __m128 a, __m128 b ) { return _mathdiff_wrap( a.v * b.v ); }
static inline __m128 _mm_max_ps( __m128 a, __m128 b ) { return _mathdiff_wrap( __builtin_ia32_maxps( a.v, b.v ) ); }
static inline __m128 _mm_min_ps( __m128 a, __m128 b ) { return _mathdiff_wrap( __builtin_ia32_minps( a.v, b.v ) ); }
static inline __m128 _mm_rcp_ps( __m128 a ) { return _mathdiff_wrap( __builtin_ia32_rcpps( a.v ) ); }
static inline __m128 _mm_rsqrt_ps( __m128 a ) { return _mathdiff_wrap( __builtin_ia32_rsqrtps( a.v ) ); }
static inline __m128 _mm_or_ps( __m128 a, __m128 b ) { return _mathdiff_wrap( (_mathdiff_v4sf)( (_mathdiff_v4si)a.v | (_mathdiff_v4si)b.v ) ); }
static inline __m128 _mm_setzero_ps( void ) { return _mathdiff_wrap( (_mathdiff_v4sf){ 0.0f, 0.0f, 0.0f, 0.0f } ); }
static inline __m128 _mm_load1_ps( const float *p ) { return _mathdiff_wrap( (_mathdiff_v4sf){ *p, *p, *p, *p } ); }
#define _mm_load_ps1 _mm_load1_ps
#define _mm_shuffle_ps( a, b, imm ) _mathdiff_wrap( __builtin_ia32_shufps( (a).v, (b).v, (imm) ) )
#define _MM_SHUFFLE( z, y, x, w ) ( ( (z) << 6 ) | ( (y) << 4 ) | ( (x) << 2 ) | (w) )

// Scalar (lowest lane) forms and the 64-bit half loads/stores used by dx/fdx8math_mtx.cpp.
typedef union __m64 { float m64_f32[2]; unsigned long long m64_u64; } __m64;
static inline __m128 _mm_add_ss( __m128 a, __m128 b ) { a.m128_f32[0] += b.m128_f32[0]; return a; }
static inline __m128 _mm_sub_ss( __m128 a, __m128 b ) { a.m128_f32[0] -= b.m128_f32[0]; return a; }
static inline __m128 _mm_mul_ss( __m128 a, __m128 b ) { a.m128_f32[0] *= b.m128_f32[0]; return a; }
static inline __m128 _mm_rcp_ss( __m128 a ) { return _mathdiff_wrap( __builtin_ia32_rcpss( a.v ) ); }
static inline __m128 _mm_loadl_pi( __m128 a, const __m64 *p ) { a.m128_f32[0] = p->m64_f32[0]; a.m128_f32[1] = p->m64_f32[1]; return a; }
static inline __m128 _mm_loadh_pi( __m128 a, const __m64 *p ) { a.m128_f32[2] = p->m64_f32[0]; a.m128_f32[3] = p->m64_f32[1]; return a; }
static inline void _mm_storel_pi( __m64 *p, __m128 a ) { p->m64_f32[0] = a.m128_f32[0]; p->m64_f32[1] = a.m128_f32[1]; }
static inline void _mm_storeh_pi( __m64 *p, __m128 a ) { p->m64_f32[0] = a.m128_f32[2]; p->m64_f32[1] = a.m128_f32[3]; }
