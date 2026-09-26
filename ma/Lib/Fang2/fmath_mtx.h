//////////////////////////////////////////////////////////////////////////////////////
// fmath_mtx.h - Fang matrix library.
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

#ifndef _FMATH_MTX_H_
#define _FMATH_MTX_H_ 1

#include "fang.h"
#include "fmath_vec.h"



typedef enum {
	FMTX_ROTTYPE_0,

	FMTX_ROTTYPE_90X,
	FMTX_ROTTYPE_180X,
	FMTX_ROTTYPE_270X,

	FMTX_ROTTYPE_90Y,
	FMTX_ROTTYPE_180Y,
	FMTX_ROTTYPE_270Y,

	FMTX_ROTTYPE_90Z,
	FMTX_ROTTYPE_180Z,
	FMTX_ROTTYPE_270Z,

	FMTX_ROTTYPE_Z180X90,

	FMTX_ROTTYPE_COUNT
} FmtxRotType_e;




class CFMtx33;
class CFMtx43;
class CFMtx44;
class CFMtx43A;
class CFMtx44A;



//--------------------------------------------------------------------
// CFMtx33 Definition:
//--------------------------------------------------------------------
class CFMtx33 {
public:
	union {
		f32 a[9];
		f32 aa[3][3];
		struct { CFVec3 m_vX, m_vY, m_vZ; };
		struct { CFVec3 m_vRight, m_vUp, m_vFront; };
	};

	static const CFMtx33 m_IdentityMtx;

	// Constructors:
	FINLINE CFMtx33( void );
	FINLINE CFMtx33( const CFVec3 &vRight, const CFVec3 &vUp, const CFVec3 &vFront );
	FINLINE CFMtx33( const CFMtx33 &m );
	FINLINE CFMtx33( const CFMtx43 &m );
	FINLINE CFMtx33( const CFMtx44 &m );
	FINLINE CFMtx33( const f32 &f00, const f32 &f01, const f32 &f02, const f32 &f10, const f32 &f11, const f32 &f12, const f32 &f20, const f32 &f21, const f32 &f22 );

	// Assignment:
	FINLINE CFMtx33 &operator = ( const CFMtx33 &m );
	FINLINE CFMtx33 &operator = ( const CFMtx43 &m );
	FINLINE CFMtx33 &operator = ( const CFMtx44 &m );
	FINLINE CFMtx33 &operator += ( const CFMtx33 &m );
	FINLINE CFMtx33 &operator -= ( const CFMtx33 &m );
	FINLINE CFMtx33 &operator *= ( const f32 &fS );
	FINLINE CFMtx33 &operator *= ( const CFMtx33 &m );

	// Binary Operators:
	FINLINE CFMtx33 operator + ( const CFMtx33 &m ) const;
	FINLINE CFMtx33 operator - ( const CFMtx33 &m ) const;
	FINLINE CFMtx33 operator * ( const CFMtx33 &m ) const;
	FINLINE CFMtx33 operator * ( const f32 &fS ) const;
	FINLINE CFVec3 operator * ( const CFVec3 &v ) const;
	FINLINE CFVec4 operator * ( const CFVec4 &v ) const;

	// Comparison:
	FINLINE BOOL operator == ( const CFMtx33 &m ) const;
	FINLINE BOOL operator != ( const CFMtx33 &m ) const;

	// Matrix Operations
	FINLINE CFMtx33 &Zero( void );
	FINLINE CFMtx33 &Identity( void );
	CFMtx33 &Invert( void );
	CFMtx33 GetInverse( void ) const;
	FINLINE CFMtx33 &Transpose( CFMtx33 &rDestMtx ) const;
	FINLINE CFMtx33 &Transpose( void );
	FINLINE CFMtx33 &UnitizeWithUniformScale( void );

	void ChangeEndian() { 
		m_vRight.ChangeEndian();
		m_vUp.ChangeEndian();
		m_vFront.ChangeEndian();
	}

	FCLASS_STACKMEM_NOALIGN( CFMtx33 );
};



#define FMATH_USE_MEMBER_MATRIX_ARRAY		FALSE
//--------------------------------------------------------------------
// CFMtx43 Definition:
//--------------------------------------------------------------------
class CFMtx43 {
public:
	union {
		f32 a[12];
		f32 aa[4][3];
		struct { CFMtx33 m33; };
		struct { CFVec3 m_vX, m_vY, m_vZ; };
		struct { CFVec3 m_vRight, m_vUp, m_vFront, m_vPos; };
	};

	static const CFMtx43 m_IdentityMtx;

	// Constructors:
	FINLINE CFMtx43( void );
	FINLINE CFMtx43( const CFVec3 &vRight, const CFVec3 &vUp, const CFVec3 &vFront, const CFVec3 &vPos = CFVec3(0.0f, 0.0f, 0.0f) );
	FINLINE CFMtx43( const CFMtx33 &m );
	FINLINE CFMtx43( const CFMtx43 &m );
	FINLINE CFMtx43( const CFMtx44 &m );
	FINLINE CFMtx43( const f32 &ff00, const f32 &ff01, const f32 &ff02, const f32 &ff10, const f32 &ff11, const f32 &ff12, const f32 &ff20, const f32 &ff21, const f32 &ff22, const f32 &ff30, const f32 &ff31, const f32 &ff32 );

	// Assignment:
	FINLINE CFMtx43 &operator = ( const CFMtx33 &m );
	FINLINE CFMtx43 &operator = ( const CFMtx43 &m );
	FINLINE CFMtx43 &operator = ( const CFMtx44 &m );
	FINLINE CFMtx43 &operator += ( const CFMtx43 &m );
	FINLINE CFMtx43 &operator -= ( const CFMtx43 &m );
	FINLINE CFMtx43 &operator *= ( const f32 &fS );
	FINLINE CFMtx43 &operator *= ( const CFMtx43 &m );

	// Binary Operators:
	FINLINE CFMtx43 operator + ( const CFMtx43 &m ) const;
	FINLINE CFMtx43 operator - ( const CFMtx43 &m ) const;
	FINLINE CFMtx43 operator * ( const CFMtx43 &m ) const;
	FINLINE CFMtx43 operator * ( const f32 &fS ) const;
	FINLINE CFVec3 MultPoint( const CFVec3 &v ) const;
	FINLINE CFVec4 MultPoint( const CFVec4 &v ) const;
	FINLINE CFVec3 MultDir( const CFVec3 &v ) const;
	FINLINE CFVec4 MultDir( const CFVec4 &v ) const;

	// Comparison:
	FINLINE BOOL operator == ( const CFMtx43 &m ) const;
	FINLINE BOOL operator != ( const CFMtx43 &m ) const;
	FINLINE BOOL TestTolerance( const CFMtx43 &m, f32 fTolerance=0.0000001f ) const;

	// Matrix Operations
	FINLINE CFMtx43 &Zero( void );
	FINLINE CFMtx43 &Identity( void );
	CFMtx43 &Invert( void );
	CFMtx43 GetInverse( void ) const;

	FINLINE void RotateX( const f32 fRadians );
	FINLINE void RotateY( const f32 fRadians );
	FINLINE void RotateZ( const f32 fRadians );
	FINLINE void SetRotationX( const f32 fRadians );
	FINLINE void SetRotationY( const f32 fRadians );
	FINLINE void SetRotationZ( const f32 fRadians );
	FINLINE void SetRotationX( const f32 fRadians, f32 fScale );
	FINLINE void SetRotationY( const f32 fRadians, f32 fScale );
	FINLINE void SetRotationZ( const f32 fRadians, f32 fScale );
	FINLINE void SetPitchYawRoll( f32 &rfPitchRadians, f32 &rfYawRadians, f32 &rfRollRadians );
	FINLINE void GetPitchYawRoll( f32 &rfPitchDegrees, f32 &rfYawDegrees, f32 &rfRollDegrees ) const;

	void ChangeEndian() { 
		m_vRight.ChangeEndian();
		m_vUp.ChangeEndian();
		m_vFront.ChangeEndian();
		m_vPos.ChangeEndian();
	}

	FCLASS_STACKMEM_NOALIGN( CFMtx43 );
};

//extern const CFMtx43A FMath_aRotMtx43A[FMTX_ROTTYPE_COUNT];


//--------------------------------------------------------------------
// CFMtx44 Definition:
//--------------------------------------------------------------------
class CFMtx44 {
public:
	union {
		f32 a[16];
		f32 aa[4][4];
		struct { CFVec4 m_vX, m_vY, m_vZ; };
		struct { CFVec4 m_vRight, m_vUp, m_vFront, m_vPos; };
	};

	static const CFMtx44 m_IdentityMtx;

	// Constructors:
	FINLINE CFMtx44( void );
	FINLINE CFMtx44( const CFVec3 &vRight, const CFVec3 &vUp, const CFVec3 &vFront, const CFVec3 &vPos = CFVec3(0.0f, 0.0f, 0.0f) );
	FINLINE CFMtx44( const CFVec4 &vRight, const CFVec4 &vUp, const CFVec4 &vFront, const CFVec4 &vPos = CFVec4(0.0f, 0.0f, 0.0f, 1.0f) );
	FINLINE CFMtx44( const CFMtx33 &m );
	FINLINE CFMtx44( const CFMtx43 &m );
	FINLINE CFMtx44( const CFMtx44 &m );
	FINLINE CFMtx44( const f32 &ff00, const f32 &ff01, const f32 &ff02, const f32 &ff03, const f32 &ff10, const f32 &ff11, const f32 &ff12, const f32 &ff13, const f32 &ff20, const f32 &ff21, const f32 &ff22, const f32 &ff23, const f32 &ff30, const f32 &ff31, const f32 &ff32, const f32 &ff33 );

	// Assignment:
	FINLINE CFMtx44 &operator = ( const CFMtx33 &m );
	FINLINE CFMtx44 &operator = ( const CFMtx43 &m );
	FINLINE CFMtx44 &operator = ( const CFMtx44 &m );
	FINLINE CFMtx44 &operator += ( const CFMtx44 &m );
	FINLINE CFMtx44 &operator -= ( const CFMtx44 &m );
	FINLINE CFMtx44 &operator *= ( const f32 &fS );
	FINLINE CFMtx44 &operator *= ( const CFMtx44 &m );

	// Binary Operators:
	FINLINE CFMtx44 operator + ( const CFMtx44 &m ) const;
	FINLINE CFMtx44 operator - ( const CFMtx44 &m ) const;
	FINLINE CFMtx44 operator * ( const CFMtx44 &m ) const;
	FINLINE CFMtx44 operator * ( const f32 &fS ) const;
	FINLINE CFVec3 MultPoint( const CFVec3 &v ) const;
	FINLINE CFVec4 MultPoint( const CFVec4 &v ) const;
	FINLINE CFVec3 MultDir( const CFVec3 &v ) const;
	FINLINE CFVec4 MultDir( const CFVec4 &v ) const;

	// Comparison:
	FINLINE BOOL operator == ( const CFMtx44 &m ) const;
	FINLINE BOOL operator != ( const CFMtx44 &m ) const;

	// Matrix Operations
	FINLINE CFMtx44 &Zero( void );
	FINLINE CFMtx44 &Identity( void );
	FINLINE CFMtx44 &Transpose( CFMtx44 &rDestMtx ) const;
	FINLINE CFMtx44 &Transpose( void );

	void ChangeEndian() { 
		m_vRight.ChangeEndian();
		m_vUp.ChangeEndian();
		m_vFront.ChangeEndian();
		m_vPos.ChangeEndian();
	}

	FCLASS_STACKMEM_NOALIGN( CFMtx44 );
};




//--------------------------------------------------------------------
// CFMtx44A Definition:
//--------------------------------------------------------------------
FCLASS_ALIGN_PREFIX class CFMtx44A {
public:
	union {
		struct { CFVec4A m_vX, m_vY, m_vZ, m_vP; };
		struct { CFVec4A m_vRight, m_vUp, m_vFront, m_vPos; };
		struct { CFMtx44 m44; };
		f32 a[16];
		f32 aa[4][4];
	};

	static const CFMtx44A m_ZeroMtx;
	static const CFMtx44A m_IdentityMtx;


	// Constructors:
	FINLINE CFMtx44A( void );
	FINLINE CFMtx44A( const CFVec3A &rRight, const CFVec3A &rUp, const CFVec3A &rFront, const CFVec3A &rPos );
	FINLINE CFMtx44A( const CFVec4A &vRight, const CFVec4A &vUp, const CFVec4A &vFront, const CFVec4A &vPos );
	FINLINE CFMtx44A( const CFMtx44A &rM );
	FINLINE CFMtx44A( const f32 &ff00, const f32 &ff01, const f32 &ff02, const f32 &ff03, const f32 &ff10, const f32 &ff11, const f32 &ff12, const f32 &ff13, const f32 &ff20, const f32 &ff21, const f32 &ff22, const f32 &ff23, const f32 &ff30, const f32 &ff31, const f32 &ff32, const f32 &ff33 );


	// Comparison:
	FINLINE BOOL operator == ( const CFMtx44A &rM ) const;
	FINLINE BOOL operator != ( const CFMtx44A &rM ) const;


	// Initialization:
	FINLINE CFMtx44A &Zero( void );
	FINLINE CFMtx44A &Identity( void );

	FINLINE CFMtx44A &operator = ( const CFMtx44A &rM );
	FINLINE CFMtx44A &Set( const CFMtx33 &rM );
	FINLINE CFMtx44A &Set( const CFMtx43 &rM );
	FINLINE CFMtx44A &Set( const CFMtx44 &rM );
	FINLINE CFMtx44A &Set( const CFMtx43A &rM );
	FINLINE CFMtx44A &Set( const CFMtx44A &rM );
	FINLINE CFMtx44A &Set( const CFVec3A &rRight, const CFVec3A &rUp, const CFVec3A &rFront, const CFVec3A &rPos );
	FINLINE CFMtx44A &Set( const CFVec4A &rRight, const CFVec4A &rUp, const CFVec4A &rFront, const CFVec4A &rPos );
	FINLINE CFMtx44A &Set( const f32 &ff00, const f32 &ff01, const f32 &ff02, const f32 &ff03, const f32 &ff10, const f32 &ff11, const f32 &ff12, const f32 &ff13, const f32 &ff20, const f32 &ff21, const f32 &ff22, const f32 &ff23, const f32 &ff30, const f32 &ff31, const f32 &ff32, const f32 &ff33 );


	// Multiplication:
	FINLINE CFMtx44A &Mul( const CFMtx44A &rM1, const CFMtx44A &rM2 );
	FINLINE CFMtx44A &Mul( const CFMtx44A &rM );

	FINLINE CFMtx44A &Mul( const CFMtx44A &rM, const f32 &fVal );
	FINLINE CFMtx44A &Mul( const f32 &fVal );

	FINLINE CFVec3A &MulPoint( CFVec3A &rRV, const CFVec3A &rV ) const;
	FINLINE CFVec4A &MulPoint( CFVec4A &rRV, const CFVec4A &rV ) const;
	FINLINE CFVec3A &MulPoint( CFVec3A &rV ) const;
	FINLINE CFVec4A &MulPoint( CFVec4A &rV ) const;

	FINLINE CFVec3A &MulDir( CFVec3A &rRV, const CFVec3A &rV ) const;
	FINLINE CFVec4A &MulDir( CFVec4A &rRV, const CFVec4A &rV ) const;
	FINLINE CFVec3A &MulDir( CFVec3A &rV ) const;
	FINLINE CFVec4A &MulDir( CFVec4A &rV ) const;


	// Inversion:
	CFMtx44A &ReceiveInverse( const CFMtx44A &rM );
	CFMtx44A &Invert( void );


	// Transpose:
	FINLINE CFMtx44A &ReceiveTranspose( const CFMtx44A &rM );
	FINLINE CFMtx44A &Transpose( void );


	// Misc:
	FINLINE u32 GenKey( void ) const;
	FINLINE void RotateX( const f32 fRadians );
	FINLINE void RotateY( const f32 fRadians );
	FINLINE void RotateZ( const f32 fRadians );
	FINLINE void SetRotationX( const f32 fRadians );
	FINLINE void SetRotationY( const f32 fRadians );
	FINLINE void SetRotationZ( const f32 fRadians );
	FINLINE void SetRotationX( const f32 fRadians, f32 fScale );
	FINLINE void SetRotationY( const f32 fRadians, f32 fScale );
	FINLINE void SetRotationZ( const f32 fRadians, f32 fScale );
	FINLINE void GetPitchYawRoll( f32 &rfPitchDegrees, f32 &rfYawDegrees, f32 &rfRollDegrees ) const;


	void ChangeEndian() { 
		m44.ChangeEndian();		
	}


	FCLASS_STACKMEM_ALIGN( CFMtx44A );
} FCLASS_ALIGN_SUFFIX;



//--------------------------------------------------------------------
// CFMtx43A Definition:
//--------------------------------------------------------------------
FCLASS_ALIGN_PREFIX class CFMtx43A {
public:
	union {
		struct { CFVec3A m_vX, m_vY, m_vZ, m_vP; };
		struct { CFVec3A m_vRight, m_vUp, m_vFront, m_vPos; };
		struct { CFMtx44 m44; };
		struct { CFMtx44A m44a; };		// (careful using this because bottom row of CFMtx43A[0-3][3] MUST always be 0)
		f32 a[16];
		f32 aa[4][4];
	};

	static const CFMtx43A m_ZeroMtx;
	static const CFMtx43A m_IdentityMtx;


	// Temporary usage matrices:
	static CFMtx43A m_Temp;			// General purpose temporary matrix
	static CFMtx43A m_Xlat;			// Users of this matrix must modify only the m_vPos vector
	static CFMtx43A m_RotX;			// Users of this matrix must only make it a rotation-around-X matrix
	static CFMtx43A m_RotY;			// Users of this matrix must only make it a rotation-around-Y matrix
	static CFMtx43A m_RotZ;			// Users of this matrix must only make it a rotation-around-Z matrix
	static CFMtx43A m_XlatRotX;		// Users of this matrix must only make it a rotation-around-X matrix with a translation
	static CFMtx43A m_XlatRotY;		// Users of this matrix must only make it a rotation-around-Y matrix with a translation
	static CFMtx43A m_XlatRotZ;		// Users of this matrix must only make it a rotation-around-Z matrix with a translation


	// Constructors:
	FINLINE CFMtx43A( void );
	FINLINE CFMtx43A( const CFVec3A &rRight, const CFVec3A &rUp, const CFVec3A &rFront, const CFVec3A &rPos );
	FINLINE CFMtx43A( const CFMtx43A &rM );
	FINLINE CFMtx43A( const f32 &ff00, const f32 &ff01, const f32 &ff02, const f32 &ff10, const f32 &ff11, const f32 &ff12, const f32 &ff20, const f32 &ff21, const f32 &ff22, const f32 &ff30, const f32 &ff31, const f32 &ff32 );


	// Comparison:
	FINLINE BOOL operator == ( const CFMtx43A &rM ) const;
	FINLINE BOOL operator != ( const CFMtx43A &rM ) const;


	// Initialization:
	FINLINE CFMtx43A &Zero( void );
	FINLINE CFMtx43A &Identity( void );
	FINLINE CFMtx43A &Identity33( void );

	FINLINE CFMtx43A &operator = ( const CFMtx43A &rM );
	FINLINE CFMtx43A &Set( const CFMtx33 &rM );
	FINLINE CFMtx43A &Set( const CFMtx43 &rM );
	FINLINE CFMtx43A &Set( const CFMtx43A &rM );
	FINLINE CFMtx43A &Set( const CFVec3A &rRight, const CFVec3A &rUp, const CFVec3A &rFront, const CFVec3A &rPos );
	FINLINE CFMtx43A &Set( const f32 &ff00, const f32 &ff01, const f32 &ff02, const f32 &ff10, const f32 &ff11, const f32 &ff12, const f32 &ff20, const f32 &ff21, const f32 &ff22, const f32 &ff30, const f32 &ff31, const f32 &ff32 );


	// Multiplication:
	FINLINE CFMtx43A &Mul( const CFMtx43A &rM1, const CFMtx43A &rM2 );
	FINLINE CFMtx43A &Mul( const CFMtx43A &rM );
	FINLINE CFMtx43A &RevMul( const CFMtx43A &rM );

	FINLINE CFMtx43A &Mul33( const CFMtx43A &rM1, const CFMtx43A &rM2 );
	FINLINE CFMtx43A &Mul33( const CFMtx43A &rM );
	FINLINE CFMtx43A &RevMul33( const CFMtx43A &rM );

	FINLINE CFMtx43A &Mul( const CFMtx43A &rM, const f32 &fVal );
	FINLINE CFMtx43A &Mul( const f32 &fVal );

	FINLINE CFMtx43A &Mul33( const CFMtx43A &rM, const f32 &fVal );
	FINLINE CFMtx43A &Mul33( const f32 &fVal );

	FINLINE CFVec3A &MulPoint( CFVec3A &rRV, const CFVec3A &rV ) const;
	FINLINE CFVec3A &MulPoint( CFVec3A &rV ) const;
	FINLINE CFVec3  &MulPoint( CFVec3 &rRV, const CFVec3 &rV ) const;

	FINLINE CFVec3A &MulDir( CFVec3A &rRV, const CFVec3A &rV ) const;
	FINLINE CFVec3A &MulDir( CFVec3A &rV ) const;


	// Inversion:
	FINLINE CFMtx43A &ReceiveAffineInverse( const CFMtx43A &rM, BOOL bProvidedMatrixMayNotHaveUnitScale );
	FINLINE CFMtx43A &AffineInvert( BOOL bProvidedMatrixMayNotHaveUnitScale );

	FINLINE CFMtx43A &ReceiveAffineInverse_KnowScale2( const CFMtx43A &rM, const f32 &fScaleOfSourceMtx2 );
	FINLINE CFMtx43A &AffineInvert_KnowScale2( const f32 &fScaleOfSourceMtx2 );

	FINLINE CFMtx43A &ReceiveAffineInverse_KnowOOScale2( const CFMtx43A &rM, const f32 &fOOScaleOfSourceMtx2 );
	FINLINE CFMtx43A &AffineInvert_KnowOOScale2( const f32 &fOOScaleOfSourceMtx2 );

	CFMtx43A &ReceiveInverse( const CFMtx43A &rM );
	CFMtx43A &Invert( void );


	// Transpose:
	FINLINE CFMtx43A &ReceiveTranspose33( const CFMtx43A &rM );
	FINLINE CFMtx43A &Transpose33( void );


	// Misc:
	FINLINE u32 GenKey( void ) const;
	FINLINE void RotateX( const f32 fRadians );
	FINLINE void RotateY( const f32 fRadians );
	FINLINE void RotateZ( const f32 fRadians );
	FINLINE void SetRotationX( const f32 fRadians );
	FINLINE void SetRotationY( const f32 fRadians );
	FINLINE void SetRotationZ( const f32 fRadians );
	FINLINE void SetRotationX( const f32 fRadians, f32 fScale );
	FINLINE void SetRotationY( const f32 fRadians, f32 fScale );
	FINLINE void SetRotationZ( const f32 fRadians, f32 fScale );
	void SetRotationYXZ( f32 fRadiansY, f32 fRadiansX, f32 fRadiansZ );
	FINLINE void GetPitchYawRoll( f32 &rfPitchDegrees, f32 &rfYawDegrees, f32 &rfRollDegrees ) const;

	CFMtx43A &UnitMtxFromUnitVec( const CFVec3A *pUnitVecZ );
	CFMtx43A &UnitMtxFromNonUnitVec( const CFVec3A *pVecZ );
	
	CFMtx43A &UnitMtxFromUnitVecPreserveAxes( const CFVec3A *pVecZ );


	void ChangeEndian() { 
		m44.ChangeEndian();		
	}


	FCLASS_STACKMEM_ALIGN( CFMtx43A );
} FCLASS_ALIGN_SUFFIX;

extern const CFMtx43A FMath_aRotMtx43A[FMTX_ROTTYPE_COUNT];

#endif

