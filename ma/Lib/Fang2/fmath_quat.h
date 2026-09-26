//////////////////////////////////////////////////////////////////////////////////////
// fmath_quat.h - Fang quaternion library.
//
// Author: Steve Ranck     
//////////////////////////////////////////////////////////////////////////////////////
// THIS CODE IS PROPRIETARY PROPERTY OF SWINGIN' APE STUDIOS, INC.
// Copyright (c) 2002
//
// The contents of this file may not be disclosed to third
// parties, copied or duplicated in any form, in whole or in part,
// without the prior written permission of Swingin' Ape Studios, Inc.
//////////////////////////////////////////////////////////////////////////////////////
// Modification History:
//
// Date     Who         Description
// -------- ----------  --------------------------------------------------------------
// 02/07/02 Ranck       Created.
//////////////////////////////////////////////////////////////////////////////////////

#ifndef _FMATH_QUAT_H_
#define _FMATH_QUAT_H_ 1

#include "fang.h"
#include "fmath_vec.h"




class CFMtx33;
class CFMtx43;
class CFMtx44A;

#define _ENABLE_OLD_QUATS 1

#if _ENABLE_OLD_QUATS
//--------------------------------------------------------------------
// CFQuat Definition:
//--------------------------------------------------------------------
class CFQuat : public CFVec4 {
public:
	static const CFQuat m_Null;
	static const CFQuat m_Identity;

	// Construction:
	CFQuat();
	CFQuat( const CFQuat &rQuat );
	CFQuat( const f32 fX, const f32 fY, const f32 fZ, const f32 fW );
	CFQuat( const CFMtx33 &rSrcMtx );
	CFQuat( const CFMtx43 &rSrcMtx );

	// Operations:
	CFQuat &BuildQuat( const CFMtx33 &rSrcMtx );
	CFQuat &BuildQuat( const CFMtx43 &rSrcMtx );
	CFQuat &BuildQuat( const CFVec3 &rUnitVecToRotateAbout, f32 fRadiansToRotateBy );
	void Zero( void );
	CFQuat &Unitize( void );
	CFQuat Unit( void ) const;
	CFQuat &Invert( void );
	CFQuat Inverse( void ) const;
	CFQuat &operator = ( const CFQuat &rQuat );
	BOOL operator == ( const CFQuat &rQuat ) const;
	CFQuat operator + ( const CFQuat &rQuat ) const;
	CFQuat operator - ( const CFQuat &rQuat ) const;
	CFQuat operator * ( const CFQuat &rQuat ) const;
	CFQuat &operator += ( const CFQuat &rQuat );
	CFQuat &operator -= ( const CFQuat &rQuat );
	CFQuat &operator *= ( const CFQuat &rQuat );
	CFQuat operator - ( void );

	// Extraction:
	CFMtx33 &BuildMtx( CFMtx33 &rDestMtx ) const;
	CFMtx33 &BuildMtx( CFMtx33 &rDestMtx, f32 fScale ) const;
	CFVec3 &BuildAxisX( CFVec3 &rDestVec ) const;
	CFVec3 &BuildAxisY( CFVec3 &rDestVec ) const;
	CFVec3 &BuildAxisZ( CFVec3 &rDestVec ) const;

	// Interpolation:
	CFQuat &ReceiveSlerpOf( f32 fUnitSlerp, const CFQuat &rQuat0, const CFQuat &rQuat1 );
	CFQuat &ReceiveSlerpOf( f32 fUnitSlerp0, f32 fUnitSlerp1, const CFQuat &rQuat0, const CFQuat &rQuat1 );

	// Sabertooth Compatibility:
	void Init( const f32 fX, const f32 fY, const f32 fZ, const f32 fAngleRadians );
	CFMtx43 GetMatrix( void );
	CFQuat Slerp( const CFQuat &rQuat, const f32 &fUnitSlerp ) const;
	void Rotate( const f32 fRadiansX, const f32 fRadiansY, const f32 fRadiansZ, int nType );

	FCLASS_STACKMEM_NOALIGN( CFQuat );
};
#endif



//--------------------------------------------------------------------
// CFQuatA Definition:
//--------------------------------------------------------------------
typedef struct {
	struct { s8 x, y, z, w; };
	struct { s8 a[4]; };
} FPackedQuat_t;


FCLASS_ALIGN_PREFIX class CFQuatA {
public:
	union {
		struct { CFVec4A v; };					// CFVec4A form
		struct { f32 x, y, z, w; };				// Individual components form
		f32 a[4];								// Array form
	};

	// Constructors:
	FINLINE CFQuatA( void );
	FINLINE CFQuatA( const CFQuatA &rQ );
	FINLINE CFQuatA( const CFVec4A &rV );
	FINLINE CFQuatA( const f32 &fX, const f32 &fY, const f32 &fZ, const f32 &fW );

	// Comparison:
	FINLINE BOOL operator == ( const CFQuatA &rQ ) const;
	FINLINE BOOL operator != ( const CFQuatA &rQ ) const;

	// Initialization:
	FINLINE CFQuatA &Zero( void );
	FINLINE CFQuatA &Identity( void );

	FINLINE CFQuatA &operator = ( const CFQuatA &rQ );
	FINLINE CFQuatA &Set( const CFQuatA &rQ );
	FINLINE CFQuatA &Set( const CFVec3A &rV );	// Important: This must set the quat's W to 0
	FINLINE CFQuatA &Set( const CFVec4A &rV );
	FINLINE CFQuatA &Set( const f32 &fX, const f32 &fY, const f32 &fZ, const f32 &fW );

	FINLINE CFQuatA &BuildQuat( const FPackedQuat_t &rPackedQuat );
	CFQuatA &BuildQuat( const CFMtx33 &rSrcMtx );
	FINLINE CFQuatA &BuildQuat( const CFMtx43 &rSrcMtx );
	CFQuatA &BuildQuat( const CFMtx44 &rSrcMtx );
	CFQuatA &BuildQuat( const CFMtx43A &rSrcMtx );
	CFQuatA &BuildQuat( const CFMtx43A &rSrcMtx, f32 fOOMtxScale );
	CFQuatA &BuildQuat( const CFMtx44A &rSrcMtx );
	FINLINE CFQuatA &BuildQuat( const CFVec3A &rUnitVecToRotateAbout, const f32 &fRadiansToRotateBy );
	FINLINE CFQuatA &BuildQuat( const CFVec4A &rUnitVecToRotateAbout, const f32 &fRadiansToRotateBy );
	FINLINE CFQuatA &BuildQuat( const CFVec3A &rUnitVecToRotateFrom, const CFVec3A &rUnitVecToRotateTo );
	FINLINE CFQuatA &BuildQuatRotX( const f32 &fRadiansToRotateBy );
	FINLINE CFQuatA &BuildQuatRotY( const f32 &fRadiansToRotateBy );
	FINLINE CFQuatA &BuildQuatRotZ( const f32 &fRadiansToRotateBy );

	// More accurate version of this incarnation of BuildQuat for clean rotations
	CFQuatA &AcuBuildQuat( const CFMtx43A &rSrcMtx );

	// Negation:
	FINLINE CFQuatA &ReceiveNegative( const CFQuatA &rQ );
	FINLINE CFQuatA &Negate( void );

	// Multiplication:
	FINLINE CFQuatA &Mul( const CFQuatA &rQ1, const CFQuatA &rQ2 );
	FINLINE CFQuatA &Mul( const CFQuatA &rQ );

	FINLINE CFVec3A &MulPoint( CFVec3A &rRV, const CFVec3A &rV ) const;
	FINLINE CFVec4A &MulPoint( CFVec4A &rRV, const CFVec4A &rV ) const;
	FINLINE CFVec3A &MulPoint( CFVec3A &rV ) const;
	FINLINE CFVec4A &MulPoint( CFVec4A &rV ) const;

	FINLINE CFVec3A &InvMulPoint( CFVec3A &rRV, const CFVec3A &rV ) const;
	FINLINE CFVec4A &InvMulPoint( CFVec4A &rRV, const CFVec4A &rV ) const;
	FINLINE CFVec3A &InvMulPoint( CFVec3A &rV ) const;
	FINLINE CFVec4A &InvMulPoint( CFVec4A &rV ) const;

	// Inverse:
	FINLINE CFQuatA &ReceiveInverse( const CFQuatA &rQ );
	FINLINE CFQuatA &Invert( void );

	FINLINE CFQuatA &ReceiveNonUnitInverse( const CFQuatA &rQ );
	FINLINE CFQuatA &NonUnitInvert( void );

	// Unit:
	FINLINE CFQuatA &ReceiveUnit( const CFQuatA &rQ );
	FINLINE CFQuatA &Unitize( void );

	// Extraction:
	FINLINE CFMtx33 &BuildMtx( CFMtx33 &rDestMtx ) const;
	FINLINE CFMtx33 &BuildMtx( CFMtx33 &rDestMtx, const f32 &rfScale ) const;
	FINLINE CFMtx43A &BuildMtx( CFMtx43A &rDestMtx ) const;
	FINLINE CFMtx43A &BuildMtx( CFMtx43A &rDestMtx, const f32 &rfScale ) const;
	FINLINE CFMtx43A &BuildMtx33( CFMtx43A &rDestMtx33 ) const;
	FINLINE CFMtx43A &BuildMtx33( CFMtx43A &rDestMtx33, const f32 &rfScale ) const;
	FINLINE CFVec3A &BuildAxisX( CFVec3A &rDestVec ) const;
	FINLINE CFVec3A &BuildAxisY( CFVec3A &rDestVec ) const;
	FINLINE CFVec3A &BuildAxisZ( CFVec3A &rDestVec ) const;

	// Interpolation:
	CFQuatA &ReceiveSlerpOf( const f32 &fUnitSlerp, const CFQuatA &rQ0, const CFQuatA &rQ1 );
	CFQuatA &ReceiveSlerpOf( const f32 &fUnitSlerp0, const f32 &fUnitSlerp1, const CFQuatA &rQ0, const CFQuatA &rQ1 );
	CFQuatA &ReceiveSlerpedStep( f32 fRadiansToStep, const CFQuatA &rQuat0, const CFQuatA &rQuat1 );

	// Debug:
	FMathFcheckResult_e FCheck( void ) const;

	void ChangeEndian( void )
	{
		v.ChangeEndian();
	}

	FCLASS_STACKMEM_ALIGN( CFQuatA );
} FCLASS_ALIGN_SUFFIX;


//--------------------------------------------------------------------
// CFTQuatA Definition:
//--------------------------------------------------------------------
class FCLASS_ALIGN_PREFIX CFTQuatA : public CFQuatA {
public:
	union {
		struct { CFVec4A m_PosScale; };
		struct {
			CFVec3 m_Pos;
			f32 m_fScale;
		};
	};

	// Constructors:
	FINLINE CFTQuatA( void );
	FINLINE CFTQuatA( const CFTQuatA &rTQ );
	FINLINE CFTQuatA( const CFQuatA &rQ, const CFVec3A &rPos, const f32 &fScale=1.0f );
	FINLINE CFTQuatA( const CFVec4A &rV, const CFVec3A &rPos, const f32 &fScale=1.0f );
	FINLINE CFTQuatA( const f32 &fQX, const f32 &fQY, const f32 &fQZ, const f32 &fQW, const f32 &fPX, const f32 &fPY, const f32 &fPZ, const f32 &fScale=1.0f );

	// Comparison:
	FINLINE BOOL operator == ( const CFTQuatA &rQ ) const;
	FINLINE BOOL operator != ( const CFTQuatA &rQ ) const;

	// Initialization:
	FINLINE CFTQuatA &Zero( void );
	FINLINE CFTQuatA &Identity( void );

	FINLINE CFTQuatA &operator = ( const CFTQuatA &rTQ );
	FINLINE CFTQuatA &Set( const CFTQuatA &rTQ );
	FINLINE CFTQuatA &Set( const CFQuatA &rQ );
	FINLINE CFTQuatA &Set( const CFQuatA &rQ, const CFVec3A &rPos, const f32 &fScale=1.0f );
	FINLINE CFTQuatA &Set( const CFVec4A &rV, const CFVec3A &rPos, const f32 &fScale=1.0f );
	FINLINE CFTQuatA &Set( const f32 &fQX, const f32 &fQY, const f32 &fQZ, const f32 &fQW, const f32 &fPX, const f32 &fPY, const f32 &fPZ, const f32 &fScale=1.0f );

	FINLINE CFTQuatA &BuildQuat( const CFMtx33 &rSrcMtx, const CFVec3A &rPos, BOOL bMtxHasNonUnitScale=FALSE );
	FINLINE CFTQuatA &BuildQuat( const CFMtx43 &rSrcMtx, BOOL bMtxHasNonUnitScale=FALSE );
	FINLINE CFTQuatA &BuildQuat( const CFMtx44 &rSrcMtx, BOOL bMtxHasNonUnitScale=FALSE );
	FINLINE CFTQuatA &BuildQuat( const CFMtx43A &rSrcMtx, BOOL bMtxHasNonUnitScale=FALSE );
	FINLINE CFTQuatA &BuildQuat( const CFMtx44A &rSrcMtx, BOOL bMtxHasNonUnitScale=FALSE );
	FINLINE CFTQuatA &BuildQuat( const CFVec3A &rUnitVecToRotateAbout, const f32 &fRadiansToRotateBy, const CFVec3A &rPos, const f32 &fScale=1.0f );
	FINLINE CFTQuatA &BuildQuat( const CFVec4A &rUnitVecToRotateAbout, const f32 &fRadiansToRotateBy, const CFVec3A &rPos, const f32 &fScale=1.0f );

	// Multiplication:
	FINLINE CFTQuatA &Mul( const CFTQuatA &rQ1, const CFTQuatA &rQ2 );
	FINLINE CFTQuatA &Mul( const CFTQuatA &rQ );

	FINLINE CFVec3A &MulPoint( CFVec3A &rRV, const CFVec3A &rV ) const;
	FINLINE CFVec4A &MulPoint( CFVec4A &rRV, const CFVec4A &rV ) const;
	FINLINE CFVec3A &MulPoint( CFVec3A &rV ) const;
	FINLINE CFVec4A &MulPoint( CFVec4A &rV ) const;

	// Extraction:
	FINLINE CFMtx43 &BuildMtx( CFMtx43 &rDestMtx ) const;
	FINLINE CFMtx44 &BuildMtx( CFMtx44 &rDestMtx ) const;
	FINLINE CFMtx43A &BuildMtx( CFMtx43A &rDestMtx ) const;
	FINLINE CFMtx44A &BuildMtx( CFMtx44A &rDestMtx ) const;

	// Interpolation:
	FINLINE CFTQuatA &ReceiveSlerpOf( const f32 &fUnitSlerp, const CFTQuatA &rQuat0, const CFTQuatA &rQuat1 );
	FINLINE CFTQuatA &ReceiveSlerpOf( const f32 &fUnitSlerp0, const f32 &fUnitSlerp1, const CFTQuatA &rQuat0, const CFTQuatA &rQuat1 );

	// Debug:
	FMathFcheckResult_e FCheck( void ) const;

	FCLASS_STACKMEM_ALIGN( CFTQuatA );
} FCLASS_ALIGN_SUFFIX;


#endif

