//////////////////////////////////////////////////////////////////////////////////////
// fcolor.h - Fang color module.
//
// Author: Steve Ranck     
//////////////////////////////////////////////////////////////////////////////////////
// THIS CODE IS PROPRIETARY PROPERTY OF SWINGIN' APE STUDIOS, INC.
// Copyright (c) 2000
//
// The contents of this file may not be disclosed to third
// parties, copied or duplicated in any form, in whole or in part,
// without the prior written permission of Swingin' Ape Studios, Inc.
//////////////////////////////////////////////////////////////////////////////////////
// Modification History:
//
// Date     Who         Description
// -------- ----------  --------------------------------------------------------------
// 09/13/00 Ranck       Created.
//////////////////////////////////////////////////////////////////////////////////////

#ifndef _FCOLOR_H_
#define _FCOLOR_H_ 1

#include "fang.h"
#include "fmath.h"


// Unless specified otherwise, f32 RGBA values range from 0.0f to 1.0f
// and u8 RGBA values range from 0 to 255.

class CFColorRGB;


typedef enum {
	FCOLORMOTIF_CONSTANT,

	FCOLORMOTIF_FLAME0,
	FCOLORMOTIF_FLAME1,
	FCOLORMOTIF_FLAME2,
	FCOLORMOTIF_FLAME3,
	FCOLORMOTIF_FLAME4,
	FCOLORMOTIF_FLAME5,
	FCOLORMOTIF_FLAME6,
	FCOLORMOTIF_FLAME7,

	FCOLORMOTIF_FLICKERON0,
	FCOLORMOTIF_FLICKERON1,
	FCOLORMOTIF_FLICKERON2,
	FCOLORMOTIF_FLICKERON3,
	FCOLORMOTIF_FLICKERON4,
	FCOLORMOTIF_FLICKERON5,
	FCOLORMOTIF_FLICKERON6,
	FCOLORMOTIF_FLICKERON7,

	FCOLORMOTIF_FLICKEROFF0,
	FCOLORMOTIF_FLICKEROFF1,
	FCOLORMOTIF_FLICKEROFF2,
	FCOLORMOTIF_FLICKEROFF3,
	FCOLORMOTIF_FLICKEROFF4,
	FCOLORMOTIF_FLICKEROFF5,
	FCOLORMOTIF_FLICKEROFF6,
	FCOLORMOTIF_FLICKEROFF7,

	FCOLORMOTIF_LIGHTNING0,
	FCOLORMOTIF_LIGHTNING1,
	FCOLORMOTIF_LIGHTNING2,
	FCOLORMOTIF_LIGHTNING3,
	FCOLORMOTIF_LIGHTNING4,
	FCOLORMOTIF_LIGHTNING5,
	FCOLORMOTIF_LIGHTNING6,
	FCOLORMOTIF_LIGHTNING7,

	FCOLORMOTIF_PULSATELOW0,
	FCOLORMOTIF_PULSATELOW1,
	FCOLORMOTIF_PULSATELOW2,
	FCOLORMOTIF_PULSATELOW3,
	FCOLORMOTIF_PULSATELOW4,
	FCOLORMOTIF_PULSATELOW5,
	FCOLORMOTIF_PULSATELOW6,
	FCOLORMOTIF_PULSATELOW7,

	FCOLORMOTIF_PULSATEMED0,
	FCOLORMOTIF_PULSATEMED1,
	FCOLORMOTIF_PULSATEMED2,
	FCOLORMOTIF_PULSATEMED3,
	FCOLORMOTIF_PULSATEMED4,
	FCOLORMOTIF_PULSATEMED5,
	FCOLORMOTIF_PULSATEMED6,
	FCOLORMOTIF_PULSATEMED7,

	FCOLORMOTIF_PULSATEHI0,
	FCOLORMOTIF_PULSATEHI1,
	FCOLORMOTIF_PULSATEHI2,
	FCOLORMOTIF_PULSATEHI3,
	FCOLORMOTIF_PULSATEHI4,
	FCOLORMOTIF_PULSATEHI5,
	FCOLORMOTIF_PULSATEHI6,
	FCOLORMOTIF_PULSATEHI7,

	FCOLORMOTIF_ROCKET0,
	FCOLORMOTIF_ROCKET1,
	FCOLORMOTIF_ROCKET2,
	FCOLORMOTIF_ROCKET3,
	FCOLORMOTIF_ROCKET4,
	FCOLORMOTIF_ROCKET5,
	FCOLORMOTIF_ROCKET6,
	FCOLORMOTIF_ROCKET7,

	FCOLORMOTIF_COUNT
} FColorMotif_e;




#define FCOLOR_FASSERT_UNIT_RGB( pCFColorRGB ) \
					FASSERT_UNIT_FLOAT( (pCFColorRGB)->fRed ); \
					FASSERT_UNIT_FLOAT( (pCFColorRGB)->fGreen ); \
					FASSERT_UNIT_FLOAT( (pCFColorRGB)->fBlue );

#define FCOLOR_FASSERT_UNIT_RGBA( pCFColorRGBA ) \
					FASSERT_UNIT_FLOAT( (pCFColorRGBA)->fRed ); \
					FASSERT_UNIT_FLOAT( (pCFColorRGBA)->fGreen ); \
					FASSERT_UNIT_FLOAT( (pCFColorRGBA)->fBlue ); \
					FASSERT_UNIT_FLOAT( (pCFColorRGBA)->fAlpha );

#define FCOLOR_FASSERT_VALID_MOTIF( pCFColorMotif ) \
					FASSERT( (pCFColorMotif)->nMotifIndex>=0 && (pCFColorMotif)->nMotifIndex<FCOLORMOTIF_COUNT ); \
					FCOLOR_FASSERT_UNIT_RGBA( pCFColorMotif );



class CFColorPackedRGB {
public:
	union {
		u8 rgb[3];			// r[0], g[1], b[2]

		struct {
			u8 nRed;
			u8 nGreen;
			u8 nBlue;
		};

		u8 xyz[3];			// x[0], y[1], z[2]

		struct {
			u8 nX;
			u8 nY;
			u8 nZ;
		};
	};

	FINLINE CFColorPackedRGB() {}

	FINLINE CFColorPackedRGB& Set( const CFColorRGB &ColorRGB );

	FCLASS_STACKMEM_NOALIGN( CFColorPackedRGB );
};


class CFColorPackedRGBA {
public:
	union {
		struct {
			CFColorPackedRGB PackedRGB;
		};

		u32 nABGR;			// Bits 31-24=A,  23-16=B,  15-8=G,  7-0=R

		u8 rgba[4];			// r[0], g[1], b[2], a[3]

		struct {
			u8 nRed;
			u8 nGreen;
			u8 nBlue;
			u8 nAlpha;
		};

		u8 xyzq[4];			// x[0], y[1], z[2], q[3]

		struct {
			u8 nX;
			u8 nY;
			u8 nZ;
			u8 nQ;
		};
	};

	FINLINE CFColorPackedRGBA() {}

	FCLASS_STACKMEM_NOALIGN( CFColorPackedRGBA );
};



class CFColorRGB {
public:
	union {
		f32 rgb[3];			// r[0], g[1], b[2]

		struct {
			f32 fRed;
			f32 fGreen;
			f32 fBlue;
		};

		f32 xyz[3];			// x[0], y[1], z[2]

		struct {
			f32 fX;
			f32 fY;
			f32 fZ;
		};

		struct { CFVec3 v3; };
	};

	FINLINE CFColorRGB() {}
	FINLINE CFColorRGB( const f32& fR, const f32& fG, const f32& fB ) { fRed=fR; fGreen=fG; fBlue=fB; }
	FINLINE CFColorRGB( const f32& fIntensity ) { fRed = fGreen = fBlue = fIntensity; }
	FINLINE CFColorRGB( const CFColorRGB& Color ) { fRed=Color.fRed; fGreen=Color.fGreen; fBlue=Color.fBlue; }
	FINLINE CFColorRGB( const CFColorPackedRGB& PackedColor ) { fRed=(f32)PackedColor.nRed*(1.0f/255.0f); fGreen=(f32)PackedColor.nGreen*(1.0f/255.0f); fBlue=(f32)PackedColor.nBlue*(1.0f/255.0f); }

	FINLINE CFColorRGB& Black( void ) { fRed = fGreen = fBlue = 0.0f; return *this; }
	FINLINE CFColorRGB& White( void ) { fRed = fGreen = fBlue = 1.0f; return *this; }
	FINLINE CFColorRGB& Red( void ) { fRed = 1.0f; fGreen = fBlue = 0.0f; return *this; }
	FINLINE CFColorRGB& Green( void ) { fGreen = 1.0f; fRed = fBlue = 0.0f; return *this; }
	FINLINE CFColorRGB& Blue( void ) { fRed = fGreen = 0.0f; fBlue = 1.0f; return *this; }

	FINLINE CFColorRGB& Zero( void ) { fRed = fGreen = fBlue = 0.0f; return *this; }
	FINLINE CFColorRGB& Set( const f32& fR, const f32& fG, const f32& fB ) { fRed=fR; fGreen=fG; fBlue=fB; return *this; }
	FINLINE CFColorRGB& Set( const f32& fIntensity ) { fRed = fGreen = fBlue = fIntensity; return *this; }
	FINLINE CFColorRGB& Set( const CFColorPackedRGB& PackedColor ) { fRed=(f32)PackedColor.nRed*(1.0f/255.0f); fGreen=(f32)PackedColor.nGreen*(1.0f/255.0f); fBlue=(f32)PackedColor.nBlue*(1.0f/255.0f); return *this; }

	FINLINE CFColorRGB& operator =  ( const CFColorRGB& Color ) { fRed=Color.fRed; fGreen=Color.fGreen; fBlue=Color.fBlue; return *this; }
	FINLINE CFColorRGB& operator += ( const CFColorRGB& Color ) { fRed+=Color.fRed; fGreen+=Color.fGreen; fBlue+=Color.fBlue; return *this; }
	FINLINE CFColorRGB& operator -= ( const CFColorRGB& Color ) { fRed-=Color.fRed; fGreen-=Color.fGreen; fBlue-=Color.fBlue; return *this; }
	FINLINE CFColorRGB& operator *= ( const CFColorRGB& Color ) { fRed*=Color.fRed; fGreen*=Color.fGreen; fBlue*=Color.fBlue; return *this; }
	FINLINE CFColorRGB& operator /= ( const CFColorRGB& Color ) { fRed/=Color.fRed; fGreen/=Color.fGreen; fBlue/=Color.fBlue; return *this; }

	FINLINE CFColorRGB& operator =  ( const f32& fIntensity ) { fRed = fGreen = fBlue = fIntensity; return *this; }
	FINLINE CFColorRGB& operator += ( const f32& fIntensity ) { fRed+=fIntensity; fGreen+=fIntensity; fBlue+=fIntensity; return *this; }
	FINLINE CFColorRGB& operator -= ( const f32& fIntensity ) { fRed-=fIntensity; fGreen-=fIntensity; fBlue-=fIntensity; return *this; }
	FINLINE CFColorRGB& operator *= ( const f32& fIntensity ) { fRed*=fIntensity; fGreen*=fIntensity; fBlue*=fIntensity; return *this; }
	FINLINE CFColorRGB& operator /= ( const f32& fIntensity ) { f32 fInv=1.0f/fIntensity; fRed*=fInv; fGreen*=fInv; fBlue*=fInv; return *this; }

	FINLINE CFColorRGB operator + ( const CFColorRGB& Color ) const { return CFColorRGB( fRed+Color.fRed, fGreen+Color.fGreen, fBlue+Color.fBlue ); }
	FINLINE CFColorRGB operator - ( const CFColorRGB& Color ) const { return CFColorRGB( fRed-Color.fRed, fGreen-Color.fGreen, fBlue-Color.fBlue ); }
	FINLINE CFColorRGB operator * ( const CFColorRGB& Color ) const { return CFColorRGB( fRed*Color.fRed, fGreen*Color.fGreen, fBlue*Color.fBlue ); }
	FINLINE CFColorRGB operator / ( const CFColorRGB& Color ) const { return CFColorRGB( fRed/Color.fRed, fGreen/Color.fGreen, fBlue/Color.fBlue ); }

	FINLINE CFColorRGB operator + ( const f32& fIntensity ) const { return CFColorRGB( fRed+fIntensity, fGreen+fIntensity, fBlue+fIntensity ); }
	FINLINE CFColorRGB operator - ( const f32& fIntensity ) const { return CFColorRGB( fRed-fIntensity, fGreen-fIntensity, fBlue-fIntensity ); }
	FINLINE CFColorRGB operator * ( const f32& fIntensity ) const { return CFColorRGB( fRed*fIntensity, fGreen*fIntensity, fBlue*fIntensity ); }
	FINLINE CFColorRGB operator / ( const f32& fIntensity ) const { f32 fInv=1.0f/fIntensity; return CFColorRGB( fRed*fInv, fGreen*fInv, fBlue*fInv ); }

	// Comparison:
	FINLINE BOOL operator == ( const CFColorRGB& Color ) const { return ((Color.fRed==fRed) & (Color.fGreen==fGreen) & (Color.fBlue==fBlue)); }
	FINLINE BOOL operator != ( const CFColorRGB& Color ) const { return !(*this == Color); }

	FINLINE f32 CalcIntensity( void ) const { return (fRed + fGreen + fBlue)*(1.0f/3.0f); }
	FINLINE f32 GetWeight( void ) { return v3.Mag2(); }
	FINLINE CFColorRGB& Clamp0( void ) { FMATH_CLAMPMIN(fRed,0.0f); FMATH_CLAMPMIN(fGreen,0.0f); FMATH_CLAMPMIN(fBlue,0.0f); return *this; }
	FINLINE CFColorRGB& Clamp1( void ) { FMATH_CLAMPMAX(fRed,1.0f); FMATH_CLAMPMAX(fGreen,1.0f); FMATH_CLAMPMAX(fBlue,1.0f); return *this; }
	FINLINE CFColorRGB& Clamp( void ) { FMATH_CLAMP(fRed,0.0f,1.0f); FMATH_CLAMP(fGreen,0.0f,1.0f); FMATH_CLAMP(fBlue,0.0f,1.0f); return *this; }
	FINLINE CFColorRGB& RandomRGB( void ) { fRed=fmath_RandomFloat(); fGreen=fmath_RandomFloat(); fBlue=fmath_RandomFloat(); return *this; }
	
	void ChangeEndian( void )
	{
		rgb[0] = fang_ConvertEndian( rgb[0] );
		rgb[1] = fang_ConvertEndian( rgb[1] );
		rgb[2] = fang_ConvertEndian( rgb[2] );
	}

	FCLASS_STACKMEM_NOALIGN( CFColorRGB );
};


class CFColorRGBA {
public:
	union {
		struct {
			CFColorRGB ColorRGB;
		};

		f32 rgba[4];		// r[0], g[1], b[2], a[3]

		struct {
			f32 fRed;
			f32 fGreen;
			f32 fBlue;
			f32 fAlpha;
		};

		f32 xyzq[4];		// x[0], y[1], z[2], q[3]

		struct {
			f32 fX;
			f32 fY;
			f32 fZ;
			f32 fQ;
		};

		struct { CFVec4 v3; };
	};

	FINLINE CFColorRGBA() {}
	FINLINE CFColorRGBA( const f32& fR, const f32& fG, const f32& fB, const f32& fA = 1.0f ) { ColorRGB.Set( fR, fG, fB ); fAlpha=fA; }
	FINLINE CFColorRGBA( const f32& fIntensity, const f32& fA = 1.0f ) { ColorRGB.Set( fIntensity ); fAlpha=fA; }
	FINLINE CFColorRGBA( const CFColorRGBA& Color ) { fRed=Color.fRed; fGreen=Color.fGreen; fBlue=Color.fBlue; fAlpha=Color.fAlpha; }
	FINLINE CFColorRGBA( const CFColorRGB& Color, const f32& fA = 1.0f ) { fRed=Color.fRed; fGreen=Color.fGreen; fBlue=Color.fBlue; fAlpha=fA; }
	FINLINE CFColorRGBA( const CFColorPackedRGBA& PackedColor ) { fRed=(f32)PackedColor.nRed*(1.0f/255.0f); fGreen=(f32)PackedColor.nGreen*(1.0f/255.0f); fBlue=(f32)PackedColor.nBlue*(1.0f/255.0f); fAlpha=(f32)PackedColor.nAlpha*(1.0f/255.0f); }
	FINLINE CFColorRGBA( const CFColorPackedRGB& PackedColor, const f32& fA = 1.0f ) { fRed=(f32)PackedColor.nRed*(1.0f/255.0f); fGreen=(f32)PackedColor.nGreen*(1.0f/255.0f); fBlue=(f32)PackedColor.nBlue*(1.0f/255.0f); fAlpha=fA; }

	FINLINE CFColorRGBA& Transparent( void ) { fAlpha = 0.0f; return *this; }
	FINLINE CFColorRGBA& Opaque( void ) { fAlpha = 1.0f; return *this; }

	FINLINE CFColorRGBA& Black( void ) { ColorRGB.Black(); return *this; }
	FINLINE CFColorRGBA& White( void ) { ColorRGB.White(); return *this; }
	FINLINE CFColorRGBA& Red( void ) { ColorRGB.Red(); return *this; }
	FINLINE CFColorRGBA& Green( void ) { ColorRGB.Green(); return *this; }
	FINLINE CFColorRGBA& Blue( void ) { ColorRGB.Blue(); return *this; }

	FINLINE CFColorRGBA& OpaqueBlack  ( void ) { Set( 0.0f, 0.0f, 0.0f, 1.0f ); return *this; }
	FINLINE CFColorRGBA& OpaqueWhite  ( void ) { Set( 1.0f, 1.0f, 1.0f, 1.0f ); return *this; }
	FINLINE CFColorRGBA& OpaqueRed    ( void ) { Set( 1.0f, 0.0f, 0.0f, 1.0f ); return *this; }
	FINLINE CFColorRGBA& OpaqueGreen  ( void ) { Set( 0.0f, 1.0f, 0.0f, 1.0f ); return *this; }
	FINLINE CFColorRGBA& OpaqueBlue   ( void ) { Set( 0.0f, 0.0f, 1.0f, 1.0f ); return *this; }
	FINLINE CFColorRGBA& OpaqueYellow ( void ) { Set( 1.0f, 1.0f, 0.0f, 1.0f ); return *this; }
	FINLINE CFColorRGBA& OpaqueMagenta( void ) { Set( 1.0f, 0.0f, 1.0f, 1.0f ); return *this; }
	FINLINE CFColorRGBA& OpaqueCyan   ( void ) { Set( 0.0f, 1.0f, 1.0f, 1.0f ); return *this; }

	FINLINE CFColorRGBA& TransparentBlack  ( void ) { Set( 0.0f, 0.0f, 0.0f, 0.0f ); return *this; }
	FINLINE CFColorRGBA& TransparentWhite  ( void ) { Set( 1.0f, 1.0f, 1.0f, 0.0f ); return *this; }
	FINLINE CFColorRGBA& TransparentRed    ( void ) { Set( 1.0f, 0.0f, 0.0f, 0.0f ); return *this; }
	FINLINE CFColorRGBA& TransparentGreen  ( void ) { Set( 0.0f, 1.0f, 0.0f, 0.0f ); return *this; }
	FINLINE CFColorRGBA& TransparentBlue	  ( void ) { Set( 0.0f, 0.0f, 1.0f, 0.0f ); return *this; }
	FINLINE CFColorRGBA& TransparentYellow ( void ) { Set( 1.0f, 1.0f, 0.0f, 0.0f ); return *this; }
	FINLINE CFColorRGBA& TransparentMagenta( void ) { Set( 1.0f, 0.0f, 1.0f, 0.0f ); return *this; }
	FINLINE CFColorRGBA& TransparentCyan   ( void ) { Set( 0.0f, 1.0f, 1.0f, 0.0f ); return *this; }

	FINLINE CFColorRGBA& Zero( void ) { TransparentBlack(); return *this; }
	FINLINE CFColorRGBA& ZeroColor( void ) { Black(); return *this; }
	FINLINE CFColorRGBA& ZeroAlpha( void ) { Transparent(); return *this; }

	FINLINE CFColorRGBA& Set( const f32& fR, const f32& fG, const f32& fB, const f32& fA ) { ColorRGB.Set( fR, fG, fB ); fAlpha=fA; return *this; }
	FINLINE CFColorRGBA& Set( const f32& fIntensity, const f32& fA ) { ColorRGB=fIntensity; fAlpha=fA; return *this; }
	FINLINE CFColorRGBA& Set( const CFColorRGBA& NewColorRGBA ) { ColorRGB=NewColorRGBA.ColorRGB; fAlpha=NewColorRGBA.fAlpha; return *this; }
	FINLINE CFColorRGBA& Set( const CFColorRGB& NewColorRGB, const f32& fA ) { ColorRGB=NewColorRGB; fAlpha=fA; return *this; }
	FINLINE CFColorRGBA& Set( const CFColorPackedRGBA& PackedColor ) { fRed=(f32)PackedColor.nRed*(1.0f/255.0f); fGreen=(f32)PackedColor.nGreen*(1.0f/255.0f); fBlue=(f32)PackedColor.nBlue*(1.0f/255.0f); fAlpha=(f32)PackedColor.nAlpha*(1.0f/255.0f); return *this; }
	FINLINE CFColorRGBA& Set( const CFColorPackedRGB& PackedColor, const f32& fA ) { fRed=(f32)PackedColor.nRed*(1.0f/255.0f); fGreen=(f32)PackedColor.nGreen*(1.0f/255.0f); fBlue=(f32)PackedColor.nBlue*(1.0f/255.0f); fAlpha=fA; return *this; }
	FINLINE CFColorRGBA& SetColor( const f32& fR, const f32& fG, const f32& fB ) { ColorRGB.Set( fR, fG, fB); return *this; }
	FINLINE CFColorRGBA& SetColor( const f32& fIntensity ) { ColorRGB.Set( fIntensity ); return *this; }
	FINLINE CFColorRGBA& SetColor( const CFColorRGBA& NewColorRGBA ) { ColorRGB=NewColorRGBA.ColorRGB; return *this; }
	FINLINE CFColorRGBA& SetColor( const CFColorRGB& NewColorRGB, f32& fA ) { fA; ColorRGB=NewColorRGB; return *this; } //added fA, since CW is much stricter than MSDev...bd.
	FINLINE CFColorRGBA& SetColor( const CFColorPackedRGBA& PackedColor ) { fRed=(f32)PackedColor.nRed*(1.0f/255.0f); fGreen=(f32)PackedColor.nGreen*(1.0f/255.0f); fBlue=(f32)PackedColor.nBlue*(1.0f/255.0f); return *this; }
	FINLINE CFColorRGBA& SetColor( const CFColorPackedRGB& PackedColor, f32& fA ) { fA; fRed=(f32)PackedColor.nRed*(1.0f/255.0f); fGreen=(f32)PackedColor.nGreen*(1.0f/255.0f); fBlue=(f32)PackedColor.nBlue*(1.0f/255.0f); return *this; } //added fA, since CW is much stricter than MSDev...bd.
	FINLINE CFColorRGBA& SetAlpha( const f32& fA ) { fAlpha=fA; return *this; }
	FINLINE CFColorRGBA& SetAlpha( const CFColorRGBA& NewColorRGBA ) { fAlpha=NewColorRGBA.fAlpha; return *this; }
	FINLINE CFColorRGBA& SetAlpha( const CFColorPackedRGBA& PackedColor ) { fAlpha=(f32)PackedColor.nAlpha*(1.0f/255.0f); return *this; }

	FINLINE CFColorRGBA& operator =  ( const CFColorRGBA& Color ) { fRed=Color.fRed; fGreen=Color.fGreen; fBlue=Color.fBlue; fAlpha=Color.fAlpha; return *this; }
	FINLINE CFColorRGBA& operator += ( const CFColorRGBA& Color ) { fRed+=Color.fRed; fGreen+=Color.fGreen; fBlue+=Color.fBlue; fAlpha+=Color.fAlpha; return *this; }
	FINLINE CFColorRGBA& operator -= ( const CFColorRGBA& Color ) { fRed-=Color.fRed; fGreen-=Color.fGreen; fBlue-=Color.fBlue; fAlpha-=Color.fAlpha; return *this; }
	FINLINE CFColorRGBA& operator *= ( const CFColorRGBA& Color ) { fRed*=Color.fRed; fGreen*=Color.fGreen; fBlue*=Color.fBlue; fAlpha*=Color.fAlpha; return *this; }
	FINLINE CFColorRGBA& operator /= ( const CFColorRGBA& Color ) { fRed/=Color.fRed; fGreen/=Color.fGreen; fBlue/=Color.fBlue; fAlpha/=Color.fAlpha; return *this; }

	FINLINE CFColorRGBA operator + ( const CFColorRGBA& Color ) const { return CFColorRGBA( fRed+Color.fRed, fGreen+Color.fGreen, fBlue+Color.fBlue, fAlpha+Color.fAlpha ); }
	FINLINE CFColorRGBA operator - ( const CFColorRGBA& Color ) const { return CFColorRGBA( fRed-Color.fRed, fGreen-Color.fGreen, fBlue-Color.fBlue, fAlpha-Color.fAlpha ); }
	FINLINE CFColorRGBA operator * ( const CFColorRGBA& Color ) const { return CFColorRGBA( fRed*Color.fRed, fGreen*Color.fGreen, fBlue*Color.fBlue, fAlpha*Color.fAlpha ); }
	FINLINE CFColorRGBA operator / ( const CFColorRGBA& Color ) const { return CFColorRGBA( fRed/Color.fRed, fGreen/Color.fGreen, fBlue/Color.fBlue, fAlpha/Color.fAlpha ); }

	FINLINE BOOL operator == ( const CFColorRGBA& Color ) const { return ((Color.fRed==fRed) & (Color.fGreen==fGreen) & (Color.fBlue==fBlue) & (Color.fAlpha==fAlpha)); }
	FINLINE BOOL operator != ( const CFColorRGBA& Color ) const { return !(*this == Color); }

	FINLINE f32 CalcIntensity( void ) const { return (fRed + fGreen + fBlue)*(1.0f/3.0f); }

	FINLINE CFColorRGBA& Clamp0( void ) { FMATH_CLAMPMIN(fRed,0.0f); FMATH_CLAMPMIN(fGreen,0.0f); FMATH_CLAMPMIN(fBlue,0.0f); FMATH_CLAMPMIN(fAlpha,0.0f); return *this; }
	FINLINE CFColorRGBA& Clamp1( void ) { FMATH_CLAMPMAX(fRed,1.0f); FMATH_CLAMPMAX(fGreen,1.0f); FMATH_CLAMPMAX(fBlue,1.0f); FMATH_CLAMPMAX(fAlpha,1.0f); return *this; }
	FINLINE CFColorRGBA& Clamp( void ) { FMATH_CLAMP(fRed,0.0f,1.0f); FMATH_CLAMP(fGreen,0.0f,1.0f); FMATH_CLAMP(fBlue,0.0f,1.0f); FMATH_CLAMP(fAlpha,0.0f,1.0f); return *this; }

	FINLINE CFColorRGBA& ClampColor0( void ) { FMATH_CLAMPMIN(fRed,0.0f); FMATH_CLAMPMIN(fGreen,0.0f); FMATH_CLAMPMIN(fBlue,0.0f); return *this; }
	FINLINE CFColorRGBA& ClampColor1( void ) { FMATH_CLAMPMAX(fRed,1.0f); FMATH_CLAMPMAX(fGreen,1.0f); FMATH_CLAMPMAX(fBlue,1.0f); return *this; }
	FINLINE CFColorRGBA& ClampColor( void ) { FMATH_CLAMP(fRed,0.0f,1.0f); FMATH_CLAMP(fGreen,0.0f,1.0f); FMATH_CLAMP(fBlue,0.0f,1.0f); return *this; }

	FINLINE CFColorRGBA& ClampAlpha0( void ) { FMATH_CLAMPMIN(fRed,0.0f); FMATH_CLAMPMIN(fAlpha,0.0f); return *this; }
	FINLINE CFColorRGBA& ClampAlpha1( void ) { FMATH_CLAMPMAX(fRed,1.0f); FMATH_CLAMPMAX(fAlpha,1.0f); return *this; }
	FINLINE CFColorRGBA& ClampAlpha( void ) { FMATH_CLAMP(fRed,0.0f,1.0f); FMATH_CLAMP(fAlpha,0.0f,1.0f); return *this; }
	FINLINE CFColorRGBA& RandomRGBA( void ) { fRed=fmath_RandomFloat(); fGreen=fmath_RandomFloat(); fBlue=fmath_RandomFloat(); fAlpha=fmath_RandomFloat(); return *this; }

	void ChangeEndian( void )
	{
		rgba[0] = fang_ConvertEndian( rgba[0] );
		rgba[1] = fang_ConvertEndian( rgba[1] );
		rgba[2] = fang_ConvertEndian( rgba[2] );
		rgba[3] = fang_ConvertEndian( rgba[3] );
	}

	FCLASS_STACKMEM_NOALIGN( CFColorRGBA );
};



#define FCOLOR_MOTIFNAME_LEN 7

class CFColorMotifEntry {
public:
	CFColorRGBA ColorRGBA;

	FCLASS_STACKMEM_NOALIGN( CFColorMotifEntry );
};



extern CFColorMotifEntry FColor_aMotifTable[FCOLORMOTIF_COUNT];



class CFColorMotif : public CFColorRGBA {
public:
	u32 nMotifIndex;

	FINLINE CFColorMotif() { nMotifIndex=0; }

	FINLINE CFColorMotif( const f32& fR, const f32& fG, const f32& fB, const f32& fA = 1.0f ) { Set( fR, fG, fB, fA ); nMotifIndex=0; }
	FINLINE CFColorMotif( const f32& fIntensity, const f32& fA = 1.0f ) { Set( fIntensity, fA ); nMotifIndex=0; }
	FINLINE CFColorMotif( const CFColorRGBA& Color ) { Set( Color ); nMotifIndex=0; }
	FINLINE CFColorMotif( const CFColorRGB& Color, const f32& fA = 1.0f ) { Set( Color, fA ); nMotifIndex=0; }
	FINLINE CFColorMotif( const CFColorPackedRGBA& PackedColor ) { Set( PackedColor ); nMotifIndex=0; }
	FINLINE CFColorMotif( const CFColorPackedRGB& PackedColor, const f32& fA = 1.0f ) { Set( PackedColor, fA ); nMotifIndex=0; }

	FINLINE CFColorMotif( const u32& nNewMotifIndex, const f32& fR, const f32& fG, const f32& fB, const f32& fA = 1.0f ) { Set( fR, fG, fB, fA ); nMotifIndex=nNewMotifIndex; }
	FINLINE CFColorMotif( const u32& nNewMotifIndex, const f32& fIntensity, const f32& fA = 1.0f ) { Set( fIntensity, fA ); nMotifIndex=nNewMotifIndex; }
	FINLINE CFColorMotif( const u32& nNewMotifIndex, const CFColorRGBA& Color ) { Set( Color ); nMotifIndex=nNewMotifIndex; }
	FINLINE CFColorMotif( const u32& nNewMotifIndex, const CFColorRGB& Color, const f32& fA = 1.0f ) { Set( Color, fA ); nMotifIndex=nNewMotifIndex; }
	FINLINE CFColorMotif( const u32& nNewMotifIndex, const CFColorPackedRGBA& PackedColor ) { Set( PackedColor ); nMotifIndex=nNewMotifIndex; }
	FINLINE CFColorMotif( const u32& nNewMotifIndex, const CFColorPackedRGB& PackedColor, const f32& fA = 1.0f ) { Set( PackedColor, fA ); nMotifIndex=nNewMotifIndex; }

	FINLINE void Constant( void ) { nMotifIndex = 0; }
	FINLINE void SetIndex( const u32& nNewIndex ) { FASSERT( nNewIndex>=0 && nNewIndex<FCOLORMOTIF_COUNT ); nMotifIndex = nNewIndex; }

	FINLINE CFColorRGBA Compute( void ) const;
	FINLINE CFColorRGBA *Compute( CFColorRGBA *pDestColor ) const;
	FINLINE CFColorRGBA *ComputeColor( CFColorRGBA *pDestColor ) const;
	FINLINE CFColorRGB ComputeColor( void ) const;
	FINLINE CFColorRGB *ComputeColor( CFColorRGB *pDestColor ) const;
	FINLINE CFColorRGBA *ComputeAlpha( CFColorRGBA *pDestColor ) const;
	FINLINE f32 ComputeAlpha( void ) const;

	FINLINE CFColorMotif& operator = ( const CFColorMotif& Motif );
	FINLINE BOOL operator == ( const CFColorMotif& Motif ) const { return (nMotifIndex==Motif.nMotifIndex) && ((CFColorRGBA &)*this == Motif); }
	
	void ChangeEndian( void )
	{
		rgba[0] = fang_ConvertEndian( rgba[0] );
		rgba[1] = fang_ConvertEndian( rgba[1] );
		rgba[2] = fang_ConvertEndian( rgba[2] );
		rgba[3] = fang_ConvertEndian( rgba[3] );
		nMotifIndex = fang_ConvertEndian( nMotifIndex );
	}

	FCLASS_STACKMEM_NOALIGN( CFColorMotif );
};


FINLINE CFColorRGBA CFColorMotif::Compute( void ) const {
	FASSERT( nMotifIndex>=0 && nMotifIndex<FCOLORMOTIF_COUNT );

	if( nMotifIndex == 0 ) {
		return *this;
	} else {
		return (*this * FColor_aMotifTable[nMotifIndex].ColorRGBA);
	}
}

FINLINE CFColorRGBA *CFColorMotif::Compute( CFColorRGBA *pDestColor ) const {
	FASSERT( nMotifIndex>=0 && nMotifIndex<FCOLORMOTIF_COUNT );

	if( nMotifIndex == 0 ) {
		*pDestColor = *this;
	} else {
		*pDestColor = *this * FColor_aMotifTable[nMotifIndex].ColorRGBA;
	}

	return pDestColor;
}

FINLINE CFColorRGB CFColorMotif::ComputeColor( void ) const {
	FASSERT( nMotifIndex>=0 && nMotifIndex<FCOLORMOTIF_COUNT );

	if( nMotifIndex == 0 ) {
		return this->ColorRGB;
	} else {
		return (this->ColorRGB * FColor_aMotifTable[nMotifIndex].ColorRGBA.ColorRGB);
	}
}

FINLINE CFColorRGB *CFColorMotif::ComputeColor( CFColorRGB *pDestColor ) const {
	FASSERT( nMotifIndex>=0 && nMotifIndex<FCOLORMOTIF_COUNT );

	if( nMotifIndex == 0 ) {
		*pDestColor = this->ColorRGB;
	} else {
		*pDestColor = this->ColorRGB * FColor_aMotifTable[nMotifIndex].ColorRGBA.ColorRGB;
	}

	return pDestColor;
}

FINLINE CFColorRGBA *CFColorMotif::ComputeColor( CFColorRGBA *pDestColor ) const {
	FASSERT( nMotifIndex>=0 && nMotifIndex<FCOLORMOTIF_COUNT );

	if( nMotifIndex == 0 ) {
		pDestColor->ColorRGB = this->ColorRGB;
	} else {
		pDestColor->ColorRGB = this->ColorRGB * FColor_aMotifTable[nMotifIndex].ColorRGBA.ColorRGB;
	}

	pDestColor->fAlpha = this->fAlpha;
	return pDestColor;
}

FINLINE CFColorRGBA *CFColorMotif::ComputeAlpha( CFColorRGBA *pDestColor ) const {
	FASSERT( nMotifIndex>=0 && nMotifIndex<FCOLORMOTIF_COUNT );

	if( nMotifIndex == 0 ) {
		pDestColor->fAlpha = this->fAlpha;
	} else {
		pDestColor->fAlpha = this->fAlpha * FColor_aMotifTable[nMotifIndex].ColorRGBA.fAlpha;
	}

	return pDestColor;
}

FINLINE f32 CFColorMotif::ComputeAlpha( void ) const {
	FASSERT( nMotifIndex>=0 && nMotifIndex<FCOLORMOTIF_COUNT );

	if( nMotifIndex == 0 ) {
		return this->fAlpha;
	} else {
		return this->fAlpha * FColor_aMotifTable[nMotifIndex].ColorRGBA.fAlpha;
	}
}

FINLINE CFColorMotif& CFColorMotif::operator = ( const CFColorMotif& Motif ) {
	FASSERT( Motif.nMotifIndex>=0 && Motif.nMotifIndex<FCOLORMOTIF_COUNT );

	nMotifIndex = Motif.nMotifIndex;
	Set( Motif );

	return *this;
}



FINLINE CFColorPackedRGB& CFColorPackedRGB::Set( const CFColorRGB &ColorRGB ) {
	nRed = fmath_FloatToU32( ColorRGB.fRed * 255.0f );
	nGreen = fmath_FloatToU32( ColorRGB.fGreen * 255.0f );
	nBlue = fmath_FloatToU32( ColorRGB.fBlue * 255.0f );

	return *this;
}


extern CFColorMotif FColor_MotifBlack;
extern CFColorMotif FColor_MotifWhite;
extern CFColorMotif FColor_MotifRed;
extern CFColorMotif FColor_MotifGreen;
extern CFColorMotif FColor_MotifBlue;


extern BOOL fcolor_ModuleStartup( void );
extern void fcolor_ModuleShutdown( void );

extern u32 fcolor_ConvertStringToBaseMotif( cchar *pszMotifName );
extern u32 fcolor_GetRandomMotif( u32 nBaseMotif );

extern void fcolor_GenerateMotifs( void );

extern void fcolor_Motif_SetLightningIntensity( f32 fIntensity );


#endif

