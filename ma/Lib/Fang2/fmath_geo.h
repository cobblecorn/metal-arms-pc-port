//////////////////////////////////////////////////////////////////////////////////////
// fmath_geo.h - Fang geometry math library.
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

#ifndef _FMATH_GEO_H_
#define _FMATH_GEO_H_ 1

#include "fang.h"
#include "fmath_vec.h"



//
//
//
FCLASS_ALIGN_PREFIX class CFRay
{
	public:
		CFVec3A vStart_WS;	
		CFVec3A vEnd_WS;	
		CFVec3A vNormal_WS;	
		f32		fLengthSq;
		f32		fLength;

	private:
		FINLINE void CalculateNormalAndLength( void )
		{
			fLengthSq = vNormal_WS.Sub( vEnd_WS, vStart_WS ).MagSq();
			if ( fLengthSq < 0.00001f )
			{
				fLength = 0.f;
				vNormal_WS.Set( 0.f, 0.f, 0.f );
				return;
			}
			fLength = fmath_Sqrt( fLengthSq );
			vNormal_WS.Mul( fmath_Inv( fLength ) );
		}
		
	public:
		FINLINE void Init( const CFVec3A *pStart, const CFVec3A *pEnd )
		{
			vStart_WS.Set( *pStart );
			vEnd_WS.Set( *pEnd );
			CalculateNormalAndLength();
		}
		FINLINE void Init( const CFVec3 *pStart, const CFVec3 *pEnd )
		{
			vStart_WS.Set( *pStart );
			vEnd_WS.Set( *pEnd );
			CalculateNormalAndLength();
		}
		FINLINE void Init( f32 fStartX, f32 fStartY, f32 fStartZ, f32 fEndX, f32 fEndY, f32 fEndZ )
		{
			vStart_WS.Set( fStartX, fStartY, fStartZ );
			vEnd_WS.Set( fEndX, fEndY, fEndZ );
			CalculateNormalAndLength();
		}

	FCLASS_STACKMEM_ALIGN( CFRay );
} FCLASS_ALIGN_SUFFIX;


//
//
//
FCLASS_ALIGN_PREFIX class CFCylinder
{
	public:
		CFVec3A 	vStart_WS;	
		CFVec3A 	vEnd_WS;
		CFVec3A		vCenterPoint_WS;
		CFVec3A		vNormal_WS;
		CFMtx43A	mtxOrient;
		CFMtx43A	mtxInvOrient;
		f32			fHeight;
		f32			fRadius;
		f32			fRadiusSq;
		BOOL		bVertical;
		
	private:
		FINLINE void CalculateAttributes( void )
		{
			FASSERT( vStart_WS != vEnd_WS );
			fRadiusSq = fRadius * fRadius;
			bVertical = (vStart_WS.x == vEnd_WS.x && vStart_WS.z == vEnd_WS.z);
			fHeight = vNormal_WS.Sub( vEnd_WS, vStart_WS ).MagSq();
			if ( fHeight == 0.f )
			{
				fHeight = 0.f;
				vNormal_WS.Set( 0.f, 0.f, 0.f );
				vCenterPoint_WS.Set( vStart_WS );
				return;
			}
			fHeight = fmath_Sqrt( fHeight );
			vNormal_WS.Mul( fmath_Inv( fHeight ) );
			vCenterPoint_WS.Mul( vNormal_WS, fHeight * 0.5f).Add( vStart_WS );
			// Calculate matrix
			mtxOrient.m_vUp.Set( vNormal_WS );
			if ( vNormal_WS.y > 0.9996f )
			{
				mtxOrient.m_vRight.Set( 1.f, 0.f, 0.f );
				mtxOrient.m_vFront.Set( 0.f, 0.f, 1.f );
			}
			else if ( vNormal_WS.y < -0.9996f )
			{
				mtxOrient.m_vRight.Set( 1.f, 0.f, 0.f );
				mtxOrient.m_vFront.Set( 0.f, 0.f, -1.f );
			}
			else if ( vNormal_WS.x > 0.9996f )
			{
				mtxOrient.m_vRight.Set( 0.f, -1.f, 0.f );
				mtxOrient.m_vFront.Set( 0.f, 0.f, 1.f );
			}
			else if ( vNormal_WS.x < -0.9996f )
			{
				mtxOrient.m_vRight.Set( 0.f, 1.f, 0.f );
				mtxOrient.m_vFront.Set( 0.f, 0.f, -1.f );
			}
			else if ( vNormal_WS.z > 0.9996f )
			{
				mtxOrient.m_vRight.Set( 1.f, 0.f, 0.f );
				mtxOrient.m_vFront.Set( 0.f, -1.f, 0.f );
			}
			else if ( vNormal_WS.z < -0.9996f )
			{
				mtxOrient.m_vRight.Set( 1.f, 0.f, 0.f );
				mtxOrient.m_vFront.Set( 0.f, 1.f, 0.f );
			}
			else
			{
				mtxOrient.m_vRight.Cross( mtxOrient.m_vUp, CFVec3A::m_UnitAxisY );
				mtxOrient.m_vRight.Unitize();
				mtxOrient.m_vFront.Cross( mtxOrient.m_vRight, mtxOrient.m_vUp );
			}
			mtxOrient.m_vPos.Set( vStart_WS );
			mtxInvOrient.ReceiveInverse( mtxOrient );
		}
		
	public:
		FINLINE void Init( const CFVec3A *pStart, const CFVec3A *pEnd, f32 fCylRadius )
		{
			vStart_WS.Set( *pStart );
			vEnd_WS.Set( *pEnd );
			fRadius = fCylRadius;
			CalculateAttributes();
		}
		FINLINE void Init( const CFVec3 *pStart, const CFVec3 *pEnd, f32 fCylRadius )
		{
			vStart_WS.Set( *pStart );
			vEnd_WS.Set( *pEnd );
			fRadius = fCylRadius;
			CalculateAttributes();
		}
		FINLINE void Init( f32 fStartX, f32 fStartY, f32 fStartZ, f32 fEndX, f32 fEndY, f32 fEndZ, f32 fCylRadius )
		{
			vStart_WS.Set( fStartX, fStartY, fStartZ );
			vEnd_WS.Set( fEndX, fEndY, fEndZ );
			fRadius = fCylRadius;
			CalculateAttributes();
		}

	FCLASS_STACKMEM_ALIGN( CFCylinder );
} FCLASS_ALIGN_SUFFIX;


//
//
//
FCLASS_ALIGN_PREFIX class CFCapsule
{
	public:
		CFVec3A m_vPoint1;
		CFVec3A m_vPoint2;
		f32 m_fRadius;

	FCLASS_STACKMEM_ALIGN( CFCylinder );
} FCLASS_ALIGN_SUFFIX;


//
//
//
class CFCone 
{
	public:
		CFVec3 m_Vertex;		// The cone's one and only vertex
		CFVec3 m_UnitDir;		// The direction the cone points from its vertex (defines the cone's axis)
		f32 m_fAngle;			// Angle from the cone's axis to its edge (must be from 0.0f to less than pi/2)

	FCLASS_STACKMEM_NOALIGN( CFCone );
};


//
//
//
class CFSphere 
{
	public:
		f32 m_fRadius;			// Radius of sphere

		CFVec3 m_Pos;	// Position of sphere

		CFSphere();
		CFSphere( const CFVec3& rCenter, const f32 fRadius );

		void Zero( void );
		void Set( const CFVec3& rCenter, const f32 fRadius );

		BOOL operator == ( const CFSphere& s ) const;
		BOOL operator != ( const CFSphere& s ) const;
		BOOL IsIntersecting( const CFSphere& s ) const;
		BOOL IsIntersecting( const CFVec3& rPoint ) const;
		BOOL IsIntersecting( const CFVec3& rPoint1, const CFVec3& rPoint2, CFVec3 *pIntersectionPoint=NULL, f32 *pfUnitDistFromStartToIntersection=NULL ) const;
		BOOL IsIntersecting( const CFCapsule *pCapsule ) const;
		BOOL IsIntersecting( const CFCone& rCone ) const;
		void BuildLooseTriangleBound( const CFVec3 &rPoint1, const CFVec3 &rPoint2, const CFVec3 &rPoint3 );
		BOOL AreEqual_XZ( const CFSphere& s ) const;
		void BuildFromMinMaxBox( const CFVec3 &rMin, const CFVec3 &rMax );
		void ChangeEndian() 
		{
			m_Pos.ChangeEndian();
			m_fRadius = fang_ConvertEndian( m_fRadius );
		}

	FCLASS_STACKMEM_NOALIGN( CFSphere );
};



//
//
//
FCLASS_ALIGN_PREFIX class CFSphereA 
{
	public:
		CFVec3A m_Pos;	// Position of sphere

		f32 m_fRadius;			// Radius of sphere

		CFSphereA();
		CFSphereA( const CFVec3A& rCenter, const f32 fRadius );

		void Zero( void );
		void Set( const CFVec3A& rCenter, const f32 fRadius );

		BOOL operator == ( const CFSphereA& s ) const;
		BOOL operator != ( const CFSphereA& s ) const;
		BOOL IsIntersecting( const CFSphereA& s ) const;
		BOOL IsIntersecting( const CFVec3A& rPoint ) const;
		BOOL IsIntersecting( const CFVec3A& rPoint1, const CFVec3A& rPoint2, CFVec3A *pIntersectionPoint=NULL, f32 *pfUnitDistFromStartToIntersection=NULL ) const;
	//	BOOL IsIntersecting( const CFCone& rCone ) const;
		void BuildLooseTriangleBound( const CFVec3A &rPoint1, const CFVec3A &rPoint2, const CFVec3A &rPoint3 );
		BOOL AreEqual_XZ( const CFSphereA& s ) const;
		void BuildFromMinMaxBox( const CFVec3A &rMin, const CFVec3A &rMax );
		void ChangeEndian() 
		{
			m_Pos.ChangeEndian();
			m_fRadius = fang_ConvertEndian( m_fRadius );
		}

	FCLASS_STACKMEM_ALIGN( CFSphereA );
} FCLASS_ALIGN_SUFFIX;



//
//
//
FCLASS_ALIGN_PREFIX class CFProjSphere
{
	public:
		CFVec3A 	m_vCenterStart_WS;
		CFVec3A 	m_vCenterHalfway_WS;
		CFVec3A 	m_vCenterEnd_WS;
		CFVec3A		m_vMoveDelta_WS;
		CFVec3A		m_vMoveNormal_WS;
		f32			m_fRadius;
		f32			m_fRadiusSq;
		f32			m_fMoveDistance;
		f32			m_fMoveDistanceSq;
		
	private:
		FINLINE void CalculateAttributes( void )
		{
			m_fRadiusSq = m_fRadius * m_fRadius;
			m_fMoveDistanceSq = m_vMoveDelta_WS.Sub( m_vCenterEnd_WS, m_vCenterStart_WS ).MagSq();
			if ( m_fMoveDistanceSq < 0.00001f )
			{
				m_vCenterHalfway_WS.Set( m_vCenterStart_WS );
				m_fMoveDistanceSq = 0.f;
				m_fMoveDistance = 0.f;
				m_vMoveNormal_WS.Set( 0.f, 0.f, 0.f );
				return;
			}
			m_fMoveDistance = fmath_Sqrt( m_fMoveDistanceSq );
			m_vMoveNormal_WS.Mul( m_vMoveDelta_WS, fmath_Inv( m_fMoveDistance ) );
			m_vCenterHalfway_WS.Mul( m_vMoveNormal_WS, m_fMoveDistance * 0.5f ).Add( m_vCenterStart_WS );
		}
		
	public:
		FINLINE void Init( const CFVec3A *pStart, const CFVec3A *pEnd, f32 fSphereRadius )
		{
			m_vCenterStart_WS.Set( *pStart );
			m_vCenterEnd_WS.Set( *pEnd );
			m_fRadius = fSphereRadius;
			CalculateAttributes();
		}
		FINLINE void Init( const CFVec3 *pStart, const CFVec3 *pEnd, f32 fSphereRadius )
		{
			m_vCenterStart_WS.Set( *pStart );
			m_vCenterEnd_WS.Set( *pEnd );
			m_fRadius = fSphereRadius;
			CalculateAttributes();
		}
		FINLINE void Init( f32 fStartX, f32 fStartY, f32 fStartZ, f32 fEndX, f32 fEndY, f32 fEndZ, f32 fSphereRadius )
		{
			m_vCenterStart_WS.Set( fStartX, fStartY, fStartZ );
			m_vCenterEnd_WS.Set( fEndX, fEndY, fEndZ );
			m_fRadius = fSphereRadius;
			CalculateAttributes();
		}

		FINLINE void ModifyMoveDistance( f32 fNewDistance )
		{
			if ( m_fMoveDistance == fNewDistance )
			{
				return;
			}
			
			// Adjust for the new distance
			m_fMoveDistance = fNewDistance;
			m_fMoveDistanceSq = fNewDistance * fNewDistance;
			m_vMoveDelta_WS.Mul( m_vMoveNormal_WS, fNewDistance );
			m_vCenterHalfway_WS.Mul( m_vMoveDelta_WS, 0.5f ).Add( m_vCenterStart_WS );
			m_vCenterEnd_WS.Add( m_vMoveDelta_WS, m_vCenterStart_WS );
		}
	FCLASS_STACKMEM_ALIGN( CFCylinder );
} FCLASS_ALIGN_SUFFIX;



class CFRect2D {
public:
	union {
		struct {
			CFVec2 UpperLeft;
			CFVec2 LowerRight;
		};
		struct {
			CFVec2 m_UpperLeft;
			CFVec2 m_LowerRight;
		};
	};

	CFRect2D();
	CFRect2D( const CFVec2 &UpperLeftVec, const CFVec2 &LowerRightVec );
	CFRect2D( const CFRect2D &r ) : UpperLeft( r.UpperLeft ), LowerRight( r.LowerRight ) {}
	CFRect2D &operator = ( const CFRect2D &r ) { UpperLeft = r.UpperLeft; LowerRight = r.LowerRight; return *this; }

	CFRect2D &Zero( void );

	FCLASS_STACKMEM_NOALIGN( CFRect2D );
};




#endif

