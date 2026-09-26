//////////////////////////////////////////////////////////////////////////////////////
// fAntenna.cpp - Antenna animation module
//
// Author: Chris MacDonald
//////////////////////////////////////////////////////////////////////////////////////
// THIS CODE IS PROPRIETARY PROPERTY OF SWINGIN' APE STUDIOS, INC.
// Copyright (c) 2003
//
// The contents of this file may not be disclosed to third
// parties, copied or duplicated in any form, in whole or in part,
// without the prior written permission of Swingin' Ape Studios, Inc.
//////////////////////////////////////////////////////////////////////////////////////
// Modification History:
//
// Date     Who         Description
// -------- ----------  --------------------------------------------------------------
// 04/01/03 MacDonald    Created.
//////////////////////////////////////////////////////////////////////////////////////

#include "fAntenna.h"
#include "fres.h"
#include "fworld.h"
#include "floop.h"

// for debugging only
#include "ftext.h"
#include "fdraw.h"

#define _NUM_FILTER_ELEMENTS	( 4 )

//-----------------------------------------------------------------------------
CFAntenna::CFAntenna()
{
	m_paBoneData = NULL;
}

//-----------------------------------------------------------------------------
// allocates memory and initializes instance of CFAntenna.
// Returns TRUE if successful, FALSE otherwise.
// pMesh is pointer to CFWorldMesh containing the antenna bones.
// uNumBones is number of bones (segments+1) in antenna.
BOOL CFAntenna::Create( CFWorldMesh *pMesh, u32 uNumBones, f32 fKSpring, f32 fKDamping, f32 fMagnitude )
{
	u32 uIndex;
	FASSERT( pMesh );
	FASSERT( uNumBones < 100 );	// sanity check

	m_fKSpring = fKSpring;
	m_fKDamping = fKDamping;
	m_fMagnitude = fMagnitude;
	m_fOOMagnitude = 1.0f / fMagnitude;
	m_pMesh = pMesh;
	m_uNumBones = uNumBones;
	m_fOONumBones = 1.0f / (f32) uNumBones;
	m_paBoneData = (BoneData_t*) fres_AlignedAlloc( m_uNumBones * sizeof( BoneData_t ), 32 );

	if( m_paBoneData == NULL )
	{
		FASSERT_NOW;
		return FALSE;
	}

	for( uIndex = 0; uIndex < m_uNumBones; uIndex++ )
	{
		m_paBoneData[ uIndex ].mOr.Identity();
		m_paBoneData[ uIndex ].fLength = 0.0f;
		m_paBoneData[ uIndex ].uBoneIndex = _INVALID_BONE_INDEX;
	}

	m_vSpringPos.Zero();
	m_vSpringVel.Zero();
	m_vBasePos.Zero();
	m_vBaseVel.Zero();
	m_bSetPos = FALSE;

	m_hFramerateFilter = fboxfilter_Create_f32( _NUM_FILTER_ELEMENTS );

	if( !m_hFramerateFilter )
	{
		FASSERT_NOW;
		return FALSE;
	}

	return TRUE;
}

//-----------------------------------------------------------------------------
void CFAntenna::Destroy( void )
{
}

//-----------------------------------------------------------------------------
// registers bones with the antenna.
// paBones is a pointer to an array of mesh bone indices.
void CFAntenna::RegisterBones( s32 *paBones )
{
	CFVec3A vDelta;
	u32 uIndex;

	// copy bone indices
	for( uIndex = 0; uIndex < m_uNumBones; uIndex++ )
	{
		FASSERT( paBones[uIndex] >= 0 );
		m_paBoneData[uIndex].uBoneIndex = (u32) paBones[uIndex];
	}

	// find bone lengths
	for( uIndex = 1; uIndex < m_uNumBones; uIndex++ )
	{
		vDelta.Set( m_pMesh->m_pMesh->pBoneArray[ m_paBoneData[uIndex].uBoneIndex ].AtRestBoneToModelMtx.m_vPos );
		vDelta.Sub( m_pMesh->m_pMesh->pBoneArray[ m_paBoneData[uIndex-1].uBoneIndex ].AtRestBoneToModelMtx.m_vPos );
		m_paBoneData[uIndex-1].fLength = vDelta.SafeUnitAndMag( vDelta );
		FASSERT( m_paBoneData[uIndex-1].fLength > 0.0f );
	}

	// last bone length slot is empty.
	m_paBoneData[m_uNumBones-1].fLength = 0.0f;
}

//-----------------------------------------------------------------------------
// Sets the antenna at the current bone position and orientation with antenna at rest.
void CFAntenna::SetPos( void )
{
	FASSERT( _ValidateRegistration() );

	m_bSetPos = TRUE;
	// actual work done in bone callback to get latest position
}

//-----------------------------------------------------------------------------
// updates the antenna position, orientation and physics.  Call every frame.
#define _MAX_ITERATIONS		( 1 )
#define _OO_MAX_ITERATIONS	( 1.0f / (f32) _MAX_ITERATIONS )
void CFAntenna::Work( void )
{
	CFVec3A vVel, vAccel, vXZAccel, vSpringForce, vSpringDamping, vSpringVel;
	CFMtx43A mInvOr;
	s32 nIterations;

	FASSERT( _ValidateRegistration() );

	if( m_bSetPos )
	{
		// null out variables on a position snap.
		// work also occurs in AnimateBone.
		m_vBasePos.Set( m_paBoneData[0].mOr.m_vPos );
		m_vBaseVel.Zero();
		m_vSpringPos.Zero();
		m_vSpringVel.Zero();
		fboxfilter_Reset_f32( m_hFramerateFilter );
	}

	fboxfilter_Add_f32( m_hFramerateFilter, FLoop_fPreviousLoopOOSecs );

	// compute velocity of antenna base
	vVel.Set( m_paBoneData[0].mOr.m_vPos );
	vVel.Sub( m_vBasePos );
	FMATH_BIPOLAR_CLAMPMAX( vVel.x, 10.0f );
	FMATH_BIPOLAR_CLAMPMAX( vVel.y, 10.0f );
	FMATH_BIPOLAR_CLAMPMAX( vVel.z, 10.0f );

	// remember new base position
	m_vBasePos.Set( m_paBoneData[0].mOr.m_vPos );

	// compute acceleration of antenna base
	vAccel.Set( vVel );
	vAccel.Sub( m_vBaseVel );
	FMATH_BIPOLAR_CLAMPMAX( vAccel.x, 10.0f );
	FMATH_BIPOLAR_CLAMPMAX( vAccel.y, 10.0f );
	FMATH_BIPOLAR_CLAMPMAX( vAccel.z, 10.0f );

	// remember new base velocity
	m_vBaseVel.Set( vVel );

	// transform acceleration to model space
	mInvOr.Identity(); // shouldn't be necessary...
	mInvOr.ReceiveTranspose33( m_paBoneData[0].mOr );
	mInvOr.MulDir( vXZAccel, vAccel );
	vXZAccel.PlanarProjection( CFVec3A::m_UnitAxisY );
	vXZAccel.y = 0.0f;

	// convert xz accel to units/sec
	vXZAccel.Mul( FLoop_fPreviousLoopOOSecs );

	for( nIterations = 0; nIterations < _MAX_ITERATIONS; nIterations++ )
	{
		// compute spring force on spring
		vSpringForce.Set( m_vSpringPos );
		vSpringForce.Mul( -m_fKSpring );

		// compute damping force on spring
		vSpringDamping.Set( m_vSpringVel );
		vSpringDamping.Mul( -m_fKDamping );

		// integrate forces into spring velocity
		vSpringForce.Add( vSpringDamping );
		vSpringForce.Add( vXZAccel );
		vSpringForce.Mul( FLoop_fPreviousLoopSecs * _OO_MAX_ITERATIONS );
		m_vSpringVel.Add( vSpringForce );
		m_vSpringVel.y = 0.0f;

		// integrate velocity into spring position
		vSpringVel.Set( m_vSpringVel );
		vSpringVel.Mul( FLoop_fPreviousLoopSecs * _OO_MAX_ITERATIONS );
		m_vSpringPos.Add( vSpringVel );
		m_vSpringPos.y = 0.0f;
	}

	// limit spring length so that antenna can't bend around more than roughly 180 degrees total.
	f32 fMax = FMATH_PI * m_fOOMagnitude;
	FMATH_BIPOLAR_CLAMPMAX( m_vSpringPos.x, fMax );
	FMATH_BIPOLAR_CLAMPMAX( m_vSpringPos.y, fMax );
	FMATH_BIPOLAR_CLAMPMAX( m_vSpringPos.z, fMax );
}

//-----------------------------------------------------------------------------
// if uBoneIndex is in Antenna, returns position in antenna (0 = root), or -1 if not found
s32 CFAntenna::FindPositionFromBoneIndex( u32 uBoneIndex )
{
	u32 uIndex;
	FASSERT( _ValidateRegistration() );

	for( uIndex = 0; uIndex < m_uNumBones; uIndex++ )
	{
		if( m_paBoneData[ uIndex ].uBoneIndex == uBoneIndex )
		{
			return (s32) uIndex;
		}
	}

	return -1;
}
	
//-----------------------------------------------------------------------------
// anim bone callback function to animate an antenna bone.
// Returns TRUE if bone was animated by antenna.
BOOL CFAntenna::AnimateBone( u32 uBoneIndex, CFMtx43A &rNewMtx, const CFMtx43A &rParentMtx, const CFMtx43A &rBoneMtx )
{
	u32 uIndex;
	s32 nPosition;
	FASSERT( _ValidateRegistration() );

	nPosition = FindPositionFromBoneIndex( uBoneIndex );
	if( nPosition < 0 )
	{
		// don't animate bones not in antenna
		return FALSE;
	}

	if( nPosition == 0 )
	{
		// transform antenna root bone
		rNewMtx.Mul( rParentMtx, rBoneMtx );

		// copy mesh version of antenna root bone matrix to bone data array
		m_paBoneData[0].mOr.Set( rNewMtx );

		if( m_bSetPos )
		{
			// snap was requested this frame, set base position and 
			// zero out spring pos.  Additional work done in Work().
			m_bSetPos = FALSE;
			m_vBasePos.Set( m_paBoneData[0].mOr.m_vPos );
			m_vSpringPos.Zero();
		}

		CFMtx43A mRot, mApply, mTemp;
		CFQuatA qRot;
		CFVec3A vUnitAxis;
		f32 fLength;

		// compute WS axis of rotation for antenna segment rotation.
		// This will be vector perpendicular to current spring pos.
		// (Perp vector chosen so that antenna will rotate in direction
		// opposite to spring movement.)

		// construct perpendicular vector.
		vUnitAxis.Set( -m_vSpringPos.z, 0.0f, m_vSpringPos.x );

		// transform into world space
		m_paBoneData[0].mOr.MulDir( vUnitAxis );

		// unitize and construct quat
		fLength = vUnitAxis.SafeUnitAndMag( vUnitAxis );
		if( fLength > 0.0f )
		{
			f32 fFilteredPreviousLoopOOSecs;

			fboxfilter_Get_f32( m_hFramerateFilter, NULL, &fFilteredPreviousLoopOOSecs, NULL, NULL );
			qRot.BuildQuat( vUnitAxis, fLength * (m_fMagnitude * m_fOONumBones ) * (fFilteredPreviousLoopOOSecs * 0.01666f)/*hack to reduce sway amount at low framerates*/ );
		}
		else
		{
			// spring pos is zero
			qRot.Identity();
		}

		// construct matrix that will cumulatively rotate mApply once for each segment
		qRot.BuildMtx( mRot );
		mRot.m_vPos.Zero();

		// construct matrix that will be transformed for each bone segment.
		// start with antenna base matrix.
		mApply.Identity();
		mApply.Set( m_paBoneData[0].mOr );
		mApply.m_vPos.Zero();

		// transform root bone
		{
			// rotate matrix for this segment (rotations will accumulate)
			mTemp.Mul( mRot, mApply );
			mApply.Set( mTemp );

			// save bone position
			CFVec3A vPos;
			vPos.Set( m_paBoneData[0].mOr.m_vPos );

			// set this bone's orientation matrix
			m_paBoneData[0].mOr.Set( mApply );

			// restore position
			m_paBoneData[0].mOr.m_vPos.Set( vPos );
		}


		// step through bones (except root bone) and construct their transforms.
		for( uIndex = 1; uIndex < m_uNumBones; uIndex++ )
		{
			// rotate matrix for this segment (rotations will accumulate)
			mTemp.Mul( mRot, mApply );
			mApply.Set( mTemp );

			// set this bone's orientation matrix
			m_paBoneData[uIndex].mOr.Set( mApply );

			// Calculate the position of this bone by projecting along the y axis 
			// of the previous bone matrix by the length of the previous bone,
			// then adding the position of the previous bone.
			m_paBoneData[uIndex].mOr.m_vPos.Set( m_paBoneData[uIndex-1].mOr.m_vY );
			m_paBoneData[uIndex].mOr.m_vPos.Mul( m_paBoneData[uIndex-1].fLength );
			m_paBoneData[uIndex].mOr.m_vPos.Add( m_paBoneData[uIndex-1].mOr.m_vPos );
		}

		// copy root bone transform to anim palette.
		rNewMtx.Set( m_paBoneData[nPosition].mOr );
	}
	else
	{
		// Copy non-root bone orientations stored in Antenna to anim palette.
		// (Relies on callback getting called on root bone before being called 
		// on other bones. Safe I believe, due to bone hierarchy.)
		FASSERT( nPosition < (s32) m_uNumBones );

		rNewMtx.Set( m_paBoneData[nPosition].mOr );
	}


	return TRUE;
}


//-----------------------------------------------------------------------------
// ensure that all bone positions have been registered
BOOL CFAntenna::_ValidateRegistration( void )
{
	u32 uIndex;

	if( m_paBoneData == NULL )
	{
		return FALSE;
	}

	for( uIndex = 0; uIndex < m_uNumBones; uIndex++ )
	{
		if( m_paBoneData[ uIndex ].uBoneIndex == _INVALID_BONE_INDEX )
		{
			return FALSE;
		}
	}

	return TRUE;
}
