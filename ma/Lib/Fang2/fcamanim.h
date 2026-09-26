//////////////////////////////////////////////////////////////////////////////////////
// fcamanim.h - Fang camera animation module.
//
// Author: Russell Foushee     
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
// 01/22/03 Foushee     Created.
//////////////////////////////////////////////////////////////////////////////////////

#ifndef _FCAMANIM_H_
#define _FCAMANIM_H_ 1

#include "fang.h"
#include "fmath.h"
#include "flinklist.h"

#define FCAMANIM_ANIMNAME_LEN	15

//-------------------------------------------------------------------------------------------------------------------
// Camera Animation Resource Object:
//-------------------------------------------------------------------------------------------------------------------


//---------------------------------------------------------------------------------------------------------------------------
// Camera Animation Data Layout (.cam files):
//
// Data laid out as follows:
//    A) One FCamAnim_t
//	  B) Array of FCamAnimKeyTime_t (The size of this array is FCamAnim_t::nTotalKeyTimes)
//	  C) Array of FCamAnimKeyTimeIdx_t (The size of this array is FCamAnim_t::nFKeyCount)
//	  D) Array of FCamAnimKeyTimeIdx_t (The size of this array is FCamAnim_t::nTKeyCount)
//	  E) Array of FCamAnimKeyTimeIdx_t (The size of this array is FCamAnim_t::nOKeyCount)
//	  F) Array of FCamAnimFKey_t (The size of this array is FCamAnim_t::nFKeyCount)
//	  G) Array of FCamAnimTKey_t (The size of this array is FCamAnim_t::nTKeyCount)
//	  H) Array of FCamAnimOKey_t (The size of this array is FCamAnim_t::nOKeyCount)
//---------------------------------------------------------------------------------------------------------------------------

// Uncompressed Time key frame
typedef struct 
{
	////////////////////////////////////////////////////////////////////////////
	// WARNING:  This structure is embedded in data exported by PASM
	////////////////////////////////////////////////////////////////////////////

	f32 fTime;

	void ChangeEndian( void )
	{
		fTime = fang_ConvertEndian( fTime );
	}
} FCamAnimKeyTime_t;


// Uncompressed Time index key frame
typedef struct 
{
	////////////////////////////////////////////////////////////////////////////
	// WARNING:  This structure is embedded in data exported by PASM
	////////////////////////////////////////////////////////////////////////////

	u16 uIdx;

	void ChangeEndian( void )
	{
		uIdx = fang_ConvertEndian( uIdx );
	}
} FCamAnimKeyTimeIdx_t;


// Uncompressed Translation key frame
typedef struct 
{
	////////////////////////////////////////////////////////////////////////////
	// WARNING:  This structure is embedded in data exported by PASM
	////////////////////////////////////////////////////////////////////////////

	CFVec3 vPosition;

	void Set( CFVec3 &vPos )
	{
		vPosition = vPos;
	}
	void ChangeEndian( void )
	{
		vPosition.ChangeEndian();
	}
} FCamAnimTKey_t;


// Uncompressed Orientation Key Frame
FCLASS_ALIGN_PREFIX struct FCamAnimOKey_t
{
	////////////////////////////////////////////////////////////////////////////
	// WARNING:  This structure is embedded in data exported by PASM
	////////////////////////////////////////////////////////////////////////////

	CFQuatA qOrientation;
	
	void Set( CFQuatA &qOrient )
	{
		qOrientation.Set( qOrient );
	}
	void ChangeEndian( void )
	{
		qOrientation.ChangeEndian();
	}

	FCLASS_STACKMEM_ALIGN( FCamAnimOKey_t );
} FCLASS_ALIGN_SUFFIX;

// Uncompressed FOV Key Frame
typedef struct
{
	////////////////////////////////////////////////////////////////////////////
	// WARNING:  This structure is embedded in data exported by PASM
	////////////////////////////////////////////////////////////////////////////

	f32 fFOV; //stored in radians

	void ChangeEndian( void )
	{
		fFOV = fang_ConvertEndian( fFOV );
	}

} FCamAnimFKey_t;

typedef struct 
{
	////////////////////////////////////////////////////////////////////////////
	// WARNING:  This structure is embedded in data exported by PASM
	////////////////////////////////////////////////////////////////////////////

	char szName[FCAMANIM_ANIMNAME_LEN+1];		// ASCIIZ name of this animation
	
	f32						fTotalSeconds;		// Total time for this animation in seconds
	f32						fOOTotalSeconds;	// 1.0f / fTotalSeconds
	u16						nFlags;				// currently unused
	u16						nFKeyCount;			// Number of FOV key frames for this animation
	u16						nTKeyCount;			// Number of translation key frames for this animation
	u16						nOKeyCount;			// Number of orientation key frames for this animation

	u16						nTotalKeyTimes;		// The total number of Key times stored in paKeyTimes;
	u8						__uPad[2];

	FCamAnimKeyTime_t		*paKeyTimes;		// Array of times at which particular keyframes occur
	FCamAnimKeyTimeIdx_t	*paFKeyTimeIdx;		// Array of time indexes into paKeyTimes at which each FOV key frame occurs
	FCamAnimKeyTimeIdx_t	*paTKeyTimeIdx;		// Array of time indexes into paKeyTimes at which each Translation key frame occurs
	FCamAnimKeyTimeIdx_t	*paOKeyTimeIdx;		// Array of time indexes into paKeyTimes at which each Orientation key frame occurs
	FCamAnimFKey_t			*paFKeyData;		// FOV key data array
	FCamAnimTKey_t			*paTKeyData;		// Translation key data array
	FCamAnimOKey_t			*paOKeyData;		// Orientation key data array
	
	void ChangeEndian( void )
	{
		for( u32 i=0; i < FCAMANIM_ANIMNAME_LEN+1; i++ ) {
			szName[i] = fang_ConvertEndian( szName[i] );
		}
		fTotalSeconds = fang_ConvertEndian( fTotalSeconds );
		fOOTotalSeconds = fang_ConvertEndian( fOOTotalSeconds );
		nFlags = fang_ConvertEndian( nFlags );
		nFKeyCount = fang_ConvertEndian( nFKeyCount );
		nTKeyCount = fang_ConvertEndian( nTKeyCount );
		nOKeyCount = fang_ConvertEndian( nOKeyCount );
		nTotalKeyTimes = fang_ConvertEndian( nTotalKeyTimes );

		paKeyTimes = (FCamAnimKeyTime_t *)fang_ConvertEndian( paKeyTimes );
		paFKeyTimeIdx = (FCamAnimKeyTimeIdx_t *)fang_ConvertEndian( paFKeyTimeIdx );
		paTKeyTimeIdx = (FCamAnimKeyTimeIdx_t *)fang_ConvertEndian( paTKeyTimeIdx );
		paOKeyTimeIdx = (FCamAnimKeyTimeIdx_t *)fang_ConvertEndian( paOKeyTimeIdx );
		paFKeyData = (FCamAnimFKey_t *)fang_ConvertEndian( paFKeyData );
		paTKeyData = (FCamAnimTKey_t *)fang_ConvertEndian( paTKeyData );
		paOKeyData = (FCamAnimOKey_t *)fang_ConvertEndian( paOKeyData );
	}
} FCamAnim_t;


//-------------------------------------------------------------------------------------------------------------------
// Camera Animation Instance Fang Module Interface :
//-------------------------------------------------------------------------------------------------------------------


FCLASS_ALIGN_PREFIX class CFCamAnimInst  {
public:
	CFCamAnimInst();

	static CFCamAnimInst *Load( cchar *pszCamAnimResName );
	BOOL Create( FCamAnim_t *pAnim );

	FINLINE const FCamAnim_t *GetCamAnim( void ) const { return m_pCamAnim; }
	FINLINE f32 GetTotalTime( void ) const { FASSERT(m_pCamAnim); return m_pCamAnim->fTotalSeconds; }
	FINLINE f32 GetOOTotalTime( void ) const { FASSERT(m_pCamAnim); return m_pCamAnim->fOOTotalSeconds; }

	FINLINE f32 GetTime( void ) const { FASSERT(m_pCamAnim); return m_fCamAnimSeconds; }
	void UpdateTime( f32 fNewTime );
	void DeltaTime( f32 fDeltaTime, BOOL bClamp=FALSE );
	void Reset(); //resets the animation back to the zero'th frame

	FINLINE f32 GetUnitTime( void ) const { return GetTime() * m_pCamAnim->fOOTotalSeconds; }
	FINLINE void UpdateUnitTime( f32 fNewUnitTime ) { UpdateTime( fNewUnitTime * m_pCamAnim->fTotalSeconds ); }
	FINLINE void DeltaUnitTime( f32 fDeltaUnitTime, BOOL bClamp=FALSE ) { DeltaTime( fDeltaUnitTime * m_pCamAnim->fTotalSeconds, bClamp ); }

	void ComputeFrame( f32 &fHalfFOV, CFQuatA &qOrient, CFVec3 &vPos );

	// Data:
protected:

	FCamAnim_t *m_pCamAnim;						// Pointer to the Camera animation resource object
	f32 m_fCamAnimSeconds;						// How many seconds into the animation we are
	f32 m_fUnitCamAnimSecs;					

	u16 m_uCurrentFIdx;
	u16 m_uCurrentTIdx;
	u16 m_uCurrentOIdx;

	FCLASS_STACKMEM_ALIGN( CFCamAnimInst );
} FCLASS_ALIGN_SUFFIX;


//-------------------------------------------------------------------------------------------------------------------
// Camera Animation System Fang Module Interface :
//-------------------------------------------------------------------------------------------------------------------

extern BOOL fcamanim_ModuleStartup( void );
extern void fcamanim_ModuleShutdown( void );

#endif
