// d3dx8.h - minimal D3DX8 replacement for the Fang DX backend.
//
// The D3DX utility library is not part of the modern Windows SDK. Fang uses only
// a handful of its math types/functions plus two texture helpers, so those are
// reimplemented here (math inline, texture helpers in d3d8_compat.cpp).

#ifndef _PORT_D3DX8_H_
#define _PORT_D3DX8_H_ 1

#include "d3d8.h"
#include <math.h>
#include <string.h>

#define D3DX_PI				3.14159265358979323846f
#define D3DX_DEFAULT		((UINT)-1)

#define D3DX_FILTER_NONE	1
#define D3DX_FILTER_POINT	2
#define D3DX_FILTER_LINEAR	3
#define D3DX_FILTER_TRIANGLE 4
#define D3DX_FILTER_BOX		5

// ---------------------------------------------------------------------------
// Vectors
// ---------------------------------------------------------------------------
struct D3DXVECTOR3 : public D3DVECTOR
{
	D3DXVECTOR3() {}
	D3DXVECTOR3( const D3DVECTOR &v ) { x = v.x; y = v.y; z = v.z; }
	D3DXVECTOR3( const float *p ) { x = p[0]; y = p[1]; z = p[2]; }
	D3DXVECTOR3( float fx, float fy, float fz ) { x = fx; y = fy; z = fz; }

	operator float *()				{ return &x; }
	operator const float *() const	{ return &x; }

	D3DXVECTOR3 &operator = ( const D3DVECTOR &v ) { x = v.x; y = v.y; z = v.z; return *this; }
	D3DXVECTOR3 operator - () const { return D3DXVECTOR3( -x, -y, -z ); }
	D3DXVECTOR3 operator + ( const D3DXVECTOR3 &v ) const { return D3DXVECTOR3( x + v.x, y + v.y, z + v.z ); }
	D3DXVECTOR3 operator - ( const D3DXVECTOR3 &v ) const { return D3DXVECTOR3( x - v.x, y - v.y, z - v.z ); }
	D3DXVECTOR3 operator * ( float f ) const { return D3DXVECTOR3( x * f, y * f, z * f ); }
	D3DXVECTOR3 &operator += ( const D3DXVECTOR3 &v ) { x += v.x; y += v.y; z += v.z; return *this; }
	D3DXVECTOR3 &operator -= ( const D3DXVECTOR3 &v ) { x -= v.x; y -= v.y; z -= v.z; return *this; }
	D3DXVECTOR3 &operator *= ( float f ) { x *= f; y *= f; z *= f; return *this; }
	D3DXVECTOR3 &operator /= ( float f ) { float oo = 1.0f / f; x *= oo; y *= oo; z *= oo; return *this; }
};

struct D3DXVECTOR4
{
	float x, y, z, w;

	D3DXVECTOR4() {}
	D3DXVECTOR4( const float *p ) { x = p[0]; y = p[1]; z = p[2]; w = p[3]; }
	D3DXVECTOR4( float fx, float fy, float fz, float fw ) { x = fx; y = fy; z = fz; w = fw; }

	// Lets a temporary be passed straight to SetVertexShaderConstant()'s void*.
	operator float *()				{ return &x; }
	operator const float *() const	{ return &x; }
};

// ---------------------------------------------------------------------------
// Matrix (row-major, row-vector convention, like D3DMATRIX)
// ---------------------------------------------------------------------------
struct D3DXMATRIX : public D3DMATRIX
{
	D3DXMATRIX() {}
	D3DXMATRIX( const D3DMATRIX &m ) { memcpy( this, &m, sizeof(D3DMATRIX) ); }
	D3DXMATRIX( const float *p ) { memcpy( this, p, sizeof(D3DMATRIX) ); }
	D3DXMATRIX( float f11, float f12, float f13, float f14,
				float f21, float f22, float f23, float f24,
				float f31, float f32, float f33, float f34,
				float f41, float f42, float f43, float f44 )
	{
		_11 = f11; _12 = f12; _13 = f13; _14 = f14;
		_21 = f21; _22 = f22; _23 = f23; _24 = f24;
		_31 = f31; _32 = f32; _33 = f33; _34 = f34;
		_41 = f41; _42 = f42; _43 = f43; _44 = f44;
	}

	float &operator () ( UINT r, UINT c )				{ return m[r][c]; }
	float  operator () ( UINT r, UINT c ) const			{ return m[r][c]; }
	operator float *()									{ return &_11; }
	operator const float *() const						{ return &_11; }

	D3DXMATRIX &operator = ( const D3DMATRIX &o ) { memcpy( this, &o, sizeof(D3DMATRIX) ); return *this; }
};

inline D3DXMATRIX *D3DXMatrixTranspose( D3DXMATRIX *pOut, const D3DXMATRIX *pM )
{
	D3DXMATRIX t;		// safe when pOut == pM
	for( int r=0; r<4; r++ ) for( int c=0; c<4; c++ ) t.m[r][c] = pM->m[c][r];
	*pOut = t;
	return pOut;
}

inline D3DXMATRIX *D3DXMatrixMultiply( D3DXMATRIX *pOut, const D3DXMATRIX *pA, const D3DXMATRIX *pB )
{
	D3DXMATRIX t;		// safe when pOut aliases an input
	for( int r=0; r<4; r++ )
		for( int c=0; c<4; c++ )
			t.m[r][c] = pA->m[r][0]*pB->m[0][c] + pA->m[r][1]*pB->m[1][c] + pA->m[r][2]*pB->m[2][c] + pA->m[r][3]*pB->m[3][c];
	*pOut = t;
	return pOut;
}

// General 4x4 inverse (cofactor expansion). Returns NULL if singular, like D3DX.
inline D3DXMATRIX *D3DXMatrixInverse( D3DXMATRIX *pOut, float *pDeterminant, const D3DXMATRIX *pM )
{
	const float *a = &pM->_11;
	float inv[16];

	inv[0]  =  a[5]*a[10]*a[15] - a[5]*a[11]*a[14] - a[9]*a[6]*a[15] + a[9]*a[7]*a[14] + a[13]*a[6]*a[11] - a[13]*a[7]*a[10];
	inv[4]  = -a[4]*a[10]*a[15] + a[4]*a[11]*a[14] + a[8]*a[6]*a[15] - a[8]*a[7]*a[14] - a[12]*a[6]*a[11] + a[12]*a[7]*a[10];
	inv[8]  =  a[4]*a[9]*a[15]  - a[4]*a[11]*a[13] - a[8]*a[5]*a[15] + a[8]*a[7]*a[13] + a[12]*a[5]*a[11] - a[12]*a[7]*a[9];
	inv[12] = -a[4]*a[9]*a[14]  + a[4]*a[10]*a[13] + a[8]*a[5]*a[14] - a[8]*a[6]*a[13] - a[12]*a[5]*a[10] + a[12]*a[6]*a[9];
	inv[1]  = -a[1]*a[10]*a[15] + a[1]*a[11]*a[14] + a[9]*a[2]*a[15] - a[9]*a[3]*a[14] - a[13]*a[2]*a[11] + a[13]*a[3]*a[10];
	inv[5]  =  a[0]*a[10]*a[15] - a[0]*a[11]*a[14] - a[8]*a[2]*a[15] + a[8]*a[3]*a[14] + a[12]*a[2]*a[11] - a[12]*a[3]*a[10];
	inv[9]  = -a[0]*a[9]*a[15]  + a[0]*a[11]*a[13] + a[8]*a[1]*a[15] - a[8]*a[3]*a[13] - a[12]*a[1]*a[11] + a[12]*a[3]*a[9];
	inv[13] =  a[0]*a[9]*a[14]  - a[0]*a[10]*a[13] - a[8]*a[1]*a[14] + a[8]*a[2]*a[13] + a[12]*a[1]*a[10] - a[12]*a[2]*a[9];
	inv[2]  =  a[1]*a[6]*a[15]  - a[1]*a[7]*a[14]  - a[5]*a[2]*a[15] + a[5]*a[3]*a[14] + a[13]*a[2]*a[7]  - a[13]*a[3]*a[6];
	inv[6]  = -a[0]*a[6]*a[15]  + a[0]*a[7]*a[14]  + a[4]*a[2]*a[15] - a[4]*a[3]*a[14] - a[12]*a[2]*a[7]  + a[12]*a[3]*a[6];
	inv[10] =  a[0]*a[5]*a[15]  - a[0]*a[7]*a[13]  - a[4]*a[1]*a[15] + a[4]*a[3]*a[13] + a[12]*a[1]*a[7]  - a[12]*a[3]*a[5];
	inv[14] = -a[0]*a[5]*a[14]  + a[0]*a[6]*a[13]  + a[4]*a[1]*a[14] - a[4]*a[2]*a[13] - a[12]*a[1]*a[6]  + a[12]*a[2]*a[5];
	inv[3]  = -a[1]*a[6]*a[11]  + a[1]*a[7]*a[10]  + a[5]*a[2]*a[11] - a[5]*a[3]*a[10] - a[9]*a[2]*a[7]   + a[9]*a[3]*a[6];
	inv[7]  =  a[0]*a[6]*a[11]  - a[0]*a[7]*a[10]  - a[4]*a[2]*a[11] + a[4]*a[3]*a[10] + a[8]*a[2]*a[7]   - a[8]*a[3]*a[6];
	inv[11] = -a[0]*a[5]*a[11]  + a[0]*a[7]*a[9]   + a[4]*a[1]*a[11] - a[4]*a[3]*a[9]  - a[8]*a[1]*a[7]   + a[8]*a[3]*a[5];
	inv[15] =  a[0]*a[5]*a[10]  - a[0]*a[6]*a[9]   - a[4]*a[1]*a[10] + a[4]*a[2]*a[9]  + a[8]*a[1]*a[6]   - a[8]*a[2]*a[5];

	float det = a[0]*inv[0] + a[1]*inv[4] + a[2]*inv[8] + a[3]*inv[12];
	if( pDeterminant ) *pDeterminant = det;
	if( det == 0.0f ) return NULL;

	float oo = 1.0f / det;
	float *o = &pOut->_11;
	for( int i=0; i<16; i++ ) o[i] = inv[i] * oo;
	return pOut;
}

inline D3DXVECTOR3 *D3DXVec3Normalize( D3DXVECTOR3 *pOut, const D3DXVECTOR3 *pV )
{
	float len = sqrtf( pV->x*pV->x + pV->y*pV->y + pV->z*pV->z );
	if( len > 0.0f )
	{
		float oo = 1.0f / len;
		pOut->x = pV->x * oo; pOut->y = pV->y * oo; pOut->z = pV->z * oo;
	}
	else
	{
		pOut->x = pOut->y = pOut->z = 0.0f;
	}
	return pOut;
}

// ---------------------------------------------------------------------------
// Texture helpers (implemented in d3d8_compat.cpp)
// ---------------------------------------------------------------------------
HRESULT D3DXCreateCubeTexture( IDirect3DDevice8 *pDevice, UINT Size, UINT MipLevels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DCubeTexture8 **ppCubeTexture );

HRESULT D3DXLoadSurfaceFromMemory( IDirect3DSurface8 *pDestSurface, CONST PALETTEENTRY *pDestPalette, CONST RECT *pDestRect,
								   LPCVOID pSrcMemory, D3DFORMAT SrcFormat, UINT SrcPitch, CONST PALETTEENTRY *pSrcPalette,
								   CONST RECT *pSrcRect, DWORD Filter, D3DCOLOR ColorKey );

#endif	// _PORT_D3DX8_H_
