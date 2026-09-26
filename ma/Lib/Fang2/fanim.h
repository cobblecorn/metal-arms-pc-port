//////////////////////////////////////////////////////////////////////////////////////
// fanim.h - Fang animation module.
//
// Author: Steve Ranck     
//////////////////////////////////////////////////////////////////////////////////////
// THIS CODE IS PROPRIETARY PROPERTY OF SWINGIN' APE STUDIOS, INC.
// Copyright (c) 2001
//
// The contents of this file may not be disclosed to third
// parties, copied or duplicated in any form, in whole or in part,
// without the prior written permission of Swingin' Ape Studios, Inc.
//////////////////////////////////////////////////////////////////////////////////////
// Modification History:
//
// Date     Who         Description
// -------- ----------  --------------------------------------------------------------
// 01/29/01 Ranck       Created.
//////////////////////////////////////////////////////////////////////////////////////

#ifndef _FANIM_H_
#define _FANIM_H_ 1

#include "fang.h"

//ARG - >>>>>
#if FANG_PLATFORM_PS2
	#pragma	interface
#endif
//ARG - <<<<<

#include "fworld.h"
#include "fmath.h"
#include "flinklist.h"

// if you are looking for FANIM_RESNAME, see fres.h - Mike

#define FANIM_ANIMNAME_LEN						15
#define FANIM_STREAMNAME_LEN					15
#define FANIM_MIXERNAME_LEN						15

#define FANIM_SIMPLE_COMBINER_CONFIG_TAPNAME	"OnlyTap"
#define FANIM_SUMMER_COMBINER_CONFIG_MIXERNAME	"SummerMixer"
#define FANIM_SUMMER_COMBINER_CONFIG_TAPNAME1	"SummerTap1"
#define FANIM_SUMMER_COMBINER_CONFIG_TAPNAME2	"SummerTap2"
#define FANIM_BLENDER_COMBINER_CONFIG_MIXERNAME	"BlenderMixer"
#define FANIM_BLENDER_COMBINER_CONFIG_TAPNAME1	"BlenderTap1"
#define FANIM_BLENDER_COMBINER_CONFIG_TAPNAME2	"BlenderTap2"

#define FANIM_BONE_BITARRAY_ELEMENT_COUNT		( (FDATA_MAX_BONE_COUNT + 31) / 32 )

// Animation compression defines - CHANGING ANY OF THESE WILL REQUIRE RE-PASM OF ANIMATION ASSETS
#define FANIM_USE_ANIM_COMPRESSION				TRUE
#define FANIM_TRANSLATION_FRAC_BITS				8
#define FANIM_COMP_TRANSLATION_CONVERTER		(1.f / (f32)(1<<FANIM_TRANSLATION_FRAC_BITS))
#define FANIM_ORIENTATION_FRAC_BITS				14
#define FANIM_COMP_ORIENTATION_CONVERTER		(1.f / (f32)(1<<FANIM_ORIENTATION_FRAC_BITS))


// Forward declarations
class CFAnimInstState;


//-------------------------------------------------------------------------------------------------------------------
// CFAnimFrame:
//-------------------------------------------------------------------------------------------------------------------
FCLASS_ALIGN_PREFIX class CFAnimFrame : public CFTQuatA {
public:
	// Construction/Destruction:
	FINLINE CFAnimFrame() {}

	FCLASS_STACKMEM_ALIGN( CFAnimFrame );
} FCLASS_ALIGN_SUFFIX;



//-------------------------------------------------------------------------------------------------------------------
// Animation Resource Object:
//-------------------------------------------------------------------------------------------------------------------
	FINLINE s16 fanim_CompTransf32( f32 fVar )
	{
		return (s16)((fVar * (f32)(1<<FANIM_TRANSLATION_FRAC_BITS)) + 0.5f);
	}
	FINLINE s16 fanim_CompOrientf32( f32 fVar )
	{
		return (s16)((fVar * (f32)(1<<FANIM_ORIENTATION_FRAC_BITS)) + 0.5f);
	}
	
	FINLINE BOOL fanim_SignificantTranslationDiff( CFVec3 *pPos1, CFVec3 *pPos2, BOOL bCompress )
	{
#if FANIM_USE_ANIM_COMPRESSION
		if ( bCompress )
		{
			if ( fanim_CompTransf32( pPos1->x ) == fanim_CompTransf32( pPos2->x ) 
			&& fanim_CompTransf32( pPos1->y ) == fanim_CompTransf32( pPos2->y ) 
			&& fanim_CompTransf32( pPos1->z ) == fanim_CompTransf32( pPos2->z ) )
			{
				return FALSE;
			}
		}
		else
#endif // FANIM_USE_ANIM_COMPRESSION
		{
			CFVec3 vDiff;
			vDiff = *pPos1 - *pPos2;
			if ( vDiff.Mag2() < 0.0001f )
			{
				return FALSE;
			}
		}
		
		return TRUE;
	}
	
	FINLINE BOOL fanim_SignificantOrientationDiff( CFQuatA *pOrient1, CFQuatA *pOrient2, BOOL bCompress, f32 fThreshhold )
	{
#if FANIM_USE_ANIM_COMPRESSION
		if ( bCompress )
		{
			if ( fanim_CompOrientf32( pOrient1->x ) == fanim_CompOrientf32( pOrient2->x ) 
				&& fanim_CompOrientf32( pOrient1->y ) == fanim_CompOrientf32( pOrient2->y ) 
				&& fanim_CompOrientf32( pOrient1->z ) == fanim_CompOrientf32( pOrient2->z )
				&& fanim_CompOrientf32( pOrient1->w ) == fanim_CompOrientf32( pOrient2->w ) )
			{
				return FALSE;
			}
			f32 fDot = (pOrient1->x * pOrient2->x) + (pOrient1->y * pOrient2->y) 
						+ (pOrient1->z * pOrient2->z) + (pOrient1->w * pOrient2->w);
			if ( fDot > fThreshhold )
			{
				return FALSE;
			}
		}
		else
#endif // FANIM_USE_ANIM_COMPRESSION
		{
			f32 fX = pOrient1->x - pOrient2->x;
			f32 fY = pOrient1->y - pOrient2->y;
			f32 fZ = pOrient1->z - pOrient2->z;
			f32 fW = pOrient1->w - pOrient2->w;

			f32 fDiff2 = (fX * fX) + (fY * fY) + (fZ * fZ) + (fW * fW);
//			if ( fDiff2 < 0.00001f )
			if ( fDiff2 < 0.0000001f )
			{
				return FALSE;
			}
		}
		
		return TRUE;
	}
	
	// Compressed translation key frame
	typedef struct 
	{
		////////////////////////////////////////////////////////////////////////////
		// WARNING:  This structure is embedded in data exported by PASM
		////////////////////////////////////////////////////////////////////////////

		s16 nCX, nCY, nCZ;							// Compressed x, y, z parent relative position

		void Set( CFVec3 &vPos )
		{
			// Make sure we don't have an invalid range
			FASSERT( vPos.x > -(1<<(15 - FANIM_TRANSLATION_FRAC_BITS)) && vPos.x < (1<<(15 - FANIM_TRANSLATION_FRAC_BITS)) );
			FASSERT( vPos.y > -(1<<(15 - FANIM_TRANSLATION_FRAC_BITS)) && vPos.y < (1<<(15 - FANIM_TRANSLATION_FRAC_BITS)) );
			FASSERT( vPos.z > -(1<<(15 - FANIM_TRANSLATION_FRAC_BITS)) && vPos.z < (1<<(15 - FANIM_TRANSLATION_FRAC_BITS)) );
			nCX = fanim_CompTransf32( vPos.x );
			nCY = fanim_CompTransf32( vPos.y );
			nCZ = fanim_CompTransf32( vPos.z );
		}
		void ChangeEndian( void )
		{
			nCX = fang_ConvertEndian( nCX );
			nCY = fang_ConvertEndian( nCY );
			nCZ = fang_ConvertEndian( nCZ );
		}
	} FAnimCTKey_t;

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
	} FAnimTKey_t;

	// Compressed Orientation Key Frame
	typedef struct 
	{
		////////////////////////////////////////////////////////////////////////////
		// WARNING:  This structure is embedded in data exported by PASM
		////////////////////////////////////////////////////////////////////////////

		s16 nCX, nCY, nCZ, nCW;
		
		void Set( CFQuatA &qOrient )
		{
			nCX = fanim_CompOrientf32( qOrient.x );
			nCY = fanim_CompOrientf32( qOrient.y );
			nCZ = fanim_CompOrientf32( qOrient.z );
			nCW = fanim_CompOrientf32( qOrient.w );
		}
		void ChangeEndian( void )
		{
			nCX = fang_ConvertEndian( nCX );
			nCY = fang_ConvertEndian( nCY );
			nCZ = fang_ConvertEndian( nCZ );
			nCW = fang_ConvertEndian( nCW );
		}
	} FAnimCOKey_t;

	// Uncompressed Orientation Key Frame
	FCLASS_ALIGN_PREFIX struct FAnimOKey_t
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

		FCLASS_STACKMEM_ALIGN( FAnimOKey_t );
	} FCLASS_ALIGN_SUFFIX;

	// Uncompressed Scale Key Frame
	typedef struct
	{
		////////////////////////////////////////////////////////////////////////////
		// WARNING:  This structure is embedded in data exported by PASM
		////////////////////////////////////////////////////////////////////////////

		f32 fScale;
	} FAnimSKey_t;
	
	typedef struct 
	{
		char *pszName;					// ASCIIZ name of this bone
		
		u16		nSKeyCount;			// Number of scale key frames for this bone (the last frame always equals the first)
		u16		nTKeyCount;			// Number of translation key frames for this bone (the last frame always equals the first)
		u16		nOKeyCount;			// Number of orientation key frames for this bone (the last frame always equals the first)
		u8		__nPad[2];
		void	*paSKeyUnitTime;	// Array of the unit time at which each Scale key frame occurs
		void	*paTKeyUnitTime;	// Array of the unit time at which each Translation key frame occurs
		void	*paOKeyUnitTime;	// Array of the unit time at which each Orientation key frame occurs
		void	*paSKeyData;		// Scale key data array
		void	*paTKeyData;		// Translation key data array
		void	*paOKeyData;		// Orientation key data array
		
		FINLINE void SetUnitKeySlider( CFAnimInstState *pState, f32 fTotalSecs, f32 fAnimSecs, f32 fDeltaSecs );
		
		void ChangeEndian( void )
		{
			pszName = (char *)fang_ConvertEndian( pszName );
			nSKeyCount = fang_ConvertEndian( nSKeyCount );
			nTKeyCount = fang_ConvertEndian( nTKeyCount );
			nOKeyCount = fang_ConvertEndian( nOKeyCount );
			paSKeyUnitTime = fang_ConvertEndian( paSKeyUnitTime );
			paTKeyUnitTime = fang_ConvertEndian( paTKeyUnitTime );
			paOKeyUnitTime = fang_ConvertEndian( paOKeyUnitTime );
			paSKeyData = fang_ConvertEndian( paSKeyData );
			paTKeyData = fang_ConvertEndian( paTKeyData );
			paOKeyData = fang_ConvertEndian( paOKeyData );
		}
	} FAnimBone_t;

// FAnim_t Flags
enum
{
	FANIM_BONEFLAGS_NONE =				0x0000,
	FANIM_BONEFLAGS_COMP_TRANSLATION = 	0x0001,
	FANIM_BONEFLAGS_COMP_ORIENTATION = 	0x0002,
	FANIM_BONEFLAGS_8BIT_SECS = 		0x0010,
	FANIM_BONEFLAGS_16BIT_SECS = 		0x0020,
	FANIM_BONEFLAGS_8BIT_FRAMECOUNT = 	0x0040,	// Default is an 16-bit frame count
};

struct FAnim_t 
{
	char szName[FANIM_ANIMNAME_LEN+1];		// ASCIIZ name of this animation
	u16 nFlags;
	u16 nBoneCount;							// Number of bones in this animation
	f32 fTotalSeconds;						// Total time for this animation in seconds
	f32 fOOTotalSeconds;					// 1.0f / fTotalSeconds
	FAnimBone_t *pBoneArray;				// Pointer to the array of bones
	void ChangeEndian( void )
	{
		nFlags = fang_ConvertEndian( nFlags );
		fTotalSeconds = fang_ConvertEndian( fTotalSeconds );
		fOOTotalSeconds = fang_ConvertEndian( fOOTotalSeconds );
		nBoneCount = fang_ConvertEndian( nBoneCount );
		pBoneArray = (FAnimBone_t *)fang_ConvertEndian( pBoneArray );
	}
};



//-------------------------------------------------------------------------------------------------------------------
// CFAnimSource:
//-------------------------------------------------------------------------------------------------------------------
class CFAnimSource {
public:
	typedef enum {
		TYPE_INST,
		TYPE_MAN_FRAME,
		TYPE_MAN_MTX,
		TYPE_MESH_REST,

		TYPE_COUNT
	} Type_e;

	// Lerp Types
	typedef enum {
		USE_LOW_KEY,
		USE_HIGH_KEY,
		LERP_KEYS,
	};

private:
	Type_e m_nType;


public:
	FINLINE CFAnimSource( Type_e nType ) { m_nType = nType; }

	FINLINE Type_e GetAnimSourceType( void ) const { return m_nType; }

	virtual u32 GetBoneCount( void ) const = 0;
	virtual cchar *GetBoneName( u32 nBoneIndex ) const = 0;
	s32 FindBoneIndex( cchar *pszBoneName ) const;
	virtual void ComputeFrame( CFAnimFrame &rDestFrame, u32 nBoneIndex, u32 nLerpType ) const = 0;
};



//-------------------------------------------------------------------------------------------------------------------
// CFAnimInst:
//-------------------------------------------------------------------------------------------------------------------
struct CFAnimInstState16 
{
	public:
		u16 m_nLowSKeyIndex;					// Current index of the low translation key (the high key is this value plus one)
		u16 m_nLowTKeyIndex;					// Current index of the low translation key (the high key is this value plus one)
		u16 m_nLowOKeyIndex;					// Current index of the low orientation key (the high key is this value plus one)
};

struct CFAnimInstState8 
{
	public:
		u8 m_nLowSKeyIndex;						// Current index of the low translation key (the high key is this value plus one)
		u8 m_nLowTKeyIndex;						// Current index of the low translation key (the high key is this value plus one)
		u8 m_nLowOKeyIndex;						// Current index of the low orientation key (the high key is this value plus one)
};

class CFAnimInst : public CFAnimSource {
public:
	FINLINE CFAnimInst() : CFAnimSource( TYPE_INST ) { m_pAnim=NULL; m_pBoneStateArray=NULL; m_fAnimSeconds=0.0f; m_fUnitAnimSecs = 0.f; }

	static CFAnimInst *Load( cchar *pszAnimResName );
	BOOL Create( const FAnim_t *pAnim );

	FINLINE const FAnim_t *GetAnim( void ) const { return m_pAnim; }
	FINLINE f32 GetTotalTime( void ) const { FASSERT(m_pAnim); return m_pAnim->fTotalSeconds; }
	FINLINE f32 GetOOTotalTime( void ) const { FASSERT(m_pAnim); return m_pAnim->fOOTotalSeconds; }

	FINLINE f32 GetTime( void ) const { FASSERT(m_pAnim); return m_fAnimSeconds; }
	void UpdateTime( f32 fNewTime );
	BOOL DeltaTime( f32 fDeltaTime, BOOL bClamp=FALSE );
	void UpdateLowKeys8( f32 fDeltaTime );
	void UpdateLowKeys16( f32 fDeltaTime );

	FINLINE f32 GetUnitTime( void ) const { return GetTime() * m_pAnim->fOOTotalSeconds; }
	FINLINE void UpdateUnitTime( f32 fNewUnitTime ) { UpdateTime( fNewUnitTime * m_pAnim->fTotalSeconds ); }
	FINLINE BOOL DeltaUnitTime( f32 fDeltaUnitTime, BOOL bClamp=FALSE ) { return DeltaTime( fDeltaUnitTime * m_pAnim->fTotalSeconds, bClamp ); }

	virtual FINLINE u32 GetBoneCount( void ) const { FASSERT(m_pAnim); return m_pAnim->nBoneCount; }
	virtual FINLINE cchar *GetBoneName( u32 nBoneIndex ) const { FASSERT(m_pAnim); FASSERT( nBoneIndex < GetBoneCount() ); return m_pAnim->pBoneArray[nBoneIndex].pszName; }
	virtual void ComputeFrame( CFAnimFrame &rDestFrame, u32 nBoneIndex, u32 nLerpType ) const;

	// Data:
protected:
	const FAnim_t *m_pAnim;					// Pointer to the animation resource object (NULL if this is a manually-driven object)
	void *m_pBoneStateArray;				// Array of current states (one per bone) (used only if m_pAnim is not NULL)
	f32 m_fAnimSeconds;						// How many seconds into the animation we are
	f32 m_fUnitAnimSecs;					

	FCLASS_STACKMEM_NOALIGN( CFAnimInst );
};



//-------------------------------------------------------------------------------------------------------------------
// CFAnimManFrame:
//-------------------------------------------------------------------------------------------------------------------
class CFAnimManFrame : public CFAnimSource {
public:
	FINLINE CFAnimManFrame() : CFAnimSource( TYPE_MAN_FRAME ) { m_nBoneCount=0; m_ppszBoneNameArray=NULL; m_pBoneFrameArray=NULL; }

	BOOL Create( u32 nBoneCount, const cchar **ppszBoneNameArray );
	BOOL Create( const u8 *pnBoneNameIndexTable, cchar **ppszBoneNameTable );

	void Reset( void );
	FINLINE void UpdateFrame( u32 nBoneIndex, const CFAnimFrame &rAnimFrame ) { FASSERT(m_pBoneFrameArray); FASSERT( nBoneIndex < GetBoneCount() ); m_pBoneFrameArray[nBoneIndex] = rAnimFrame; }
	FINLINE const CFAnimFrame *GetFrameArray( void ) const { return m_pBoneFrameArray; }
	virtual FINLINE u32 GetBoneCount( void ) const { return m_nBoneCount; }
	virtual FINLINE cchar *GetBoneName( u32 nBoneIndex ) const { FASSERT(m_pBoneFrameArray); FASSERT( nBoneIndex < GetBoneCount() ); return m_ppszBoneNameArray[nBoneIndex]; }
	virtual FINLINE void ComputeFrame( CFAnimFrame &rDestFrame, u32 nBoneIndex, u32 nLerpType ) const { FASSERT( m_pBoneFrameArray ); rDestFrame = m_pBoneFrameArray[nBoneIndex]; }

	// Data:
protected:
	u32 m_nBoneCount;								// Number of bones
	cchar **m_ppszBoneNameArray;					// Pointer to an array of string pointers (one per bone) with the name of each bone
	CFAnimFrame *m_pBoneFrameArray;					// Array of frame data (one per bone)

	FCLASS_STACKMEM_NOALIGN( CFAnimManFrame );
};



//-------------------------------------------------------------------------------------------------------------------
// CFAnimManMtx:
//-------------------------------------------------------------------------------------------------------------------
FCLASS_ALIGN_PREFIX class CFAnimManMtx : public CFAnimSource {
public:
	FINLINE CFAnimManMtx() : CFAnimSource( TYPE_MAN_MTX ) { m_nBoneCount=0; m_ppszBoneNameArray=NULL; m_pBoneMtxArray=NULL; m_pFrameArray=NULL; m_abMtxChanged=NULL; }
	FINLINE ~CFAnimManMtx() { Destroy(); }

	BOOL Create( u32 nBoneCount, cchar **ppszBoneNameArray );
	BOOL Create( cchar *pszBoneName );
	void Destroy( void );
	FINLINE BOOL IsCreated( void ) const { return (m_ppszBoneNameArray != NULL); }
	void Reset( void );

	FINLINE void UpdateFrame( u32 nBoneIndex, const CFMtx43A &rMtx43A );
	FINLINE CFMtx43A *GetFrameAndFlagAsChanged( u32 nBoneIndex ) const;
	FINLINE const CFMtx43A *GetFrameArray( void ) const { return m_pBoneMtxArray; }
	virtual FINLINE u32 GetBoneCount( void ) const { return m_nBoneCount; }
	virtual FINLINE cchar *GetBoneName( u32 nBoneIndex ) const { FASSERT(m_pBoneMtxArray); FASSERT( nBoneIndex < GetBoneCount() ); return m_ppszBoneNameArray[nBoneIndex]; }
	virtual FINLINE void ComputeFrame( CFAnimFrame &rDestFrame, u32 nBoneIndex, u32 nLerpType ) const;

	// Data:
protected:
	u32 m_nBoneCount;								// Number of bones
	cchar **m_ppszBoneNameArray;					// Pointer to an array of string pointers (one per bone) with the name of each bone
	CFMtx43A *m_pBoneMtxArray;						// Array of matrix data (one per bone)
	CFAnimFrame *m_pFrameArray;						// Array of frames computed from the matrix data
	u8 *m_abMtxChanged;								// TRUE when the matrix has changed and the TQuat needs to be recomputed

	FCLASS_STACKMEM_ALIGN( CFAnimManMtx );
} FCLASS_ALIGN_SUFFIX;


FINLINE void CFAnimManMtx::UpdateFrame( u32 nBoneIndex, const CFMtx43A &rMtx43A ) {
	FASSERT( m_pBoneMtxArray );
	FASSERT( nBoneIndex < m_nBoneCount );

	if( rMtx43A != m_pBoneMtxArray[nBoneIndex] ) {
		m_pBoneMtxArray[nBoneIndex] = rMtx43A;
		m_abMtxChanged[nBoneIndex] = TRUE;
	}
}

FINLINE CFMtx43A *CFAnimManMtx::GetFrameAndFlagAsChanged( u32 nBoneIndex ) const {
	FASSERT(m_pBoneMtxArray);
	FASSERT( nBoneIndex < GetBoneCount() );

	m_abMtxChanged[nBoneIndex] = TRUE;
	return &m_pBoneMtxArray[nBoneIndex];
}

FINLINE void CFAnimManMtx::ComputeFrame( CFAnimFrame &rDestFrame, u32 nBoneIndex, u32 nLerpType ) const {
	FASSERT( m_pBoneMtxArray );
	FASSERT( nBoneIndex < m_nBoneCount );

	if( !m_abMtxChanged[nBoneIndex] ) {
		rDestFrame = m_pFrameArray[nBoneIndex];
	} else {
		m_abMtxChanged[nBoneIndex] = FALSE;
		rDestFrame.BuildQuat( m_pBoneMtxArray[nBoneIndex] );
		m_pFrameArray[nBoneIndex] = rDestFrame;
	}
}



//-------------------------------------------------------------------------------------------------------------------
// CFAnimMeshRest
//-------------------------------------------------------------------------------------------------------------------
FCLASS_ALIGN_PREFIX class CFAnimMeshRest : public CFAnimSource {
public:
	FINLINE CFAnimMeshRest() : CFAnimSource( TYPE_MESH_REST ) { m_nBoneCount=0; m_paTQuat = NULL; }
	FINLINE ~CFAnimMeshRest() { Destroy(); }

	BOOL Create( FMesh_t *pMesh );
	void Destroy( void );

	virtual FINLINE u32 GetBoneCount( void ) const { return m_nBoneCount; }
	virtual FINLINE cchar *GetBoneName( u32 nBoneIndex ) const { FASSERT(m_paTQuat); FASSERT( nBoneIndex < GetBoneCount() ); return m_pMesh->pBoneArray[nBoneIndex].szName; }
	s32 FINLINE FindBoneIndex( cchar *pszBoneName ) const;
	virtual FINLINE void ComputeFrame( CFAnimFrame &rDestFrame, u32 nBoneIndex, u32 nLerpType ) const { rDestFrame = m_paTQuat[nBoneIndex]; }

	// Data:
protected:
	u32 m_nBoneCount;								// Number of bones
	FMesh_t *m_pMesh;
	CFAnimFrame *m_paTQuat;

	FCLASS_STACKMEM_ALIGN( CFAnimMeshRest );
} FCLASS_ALIGN_SUFFIX;





//-------------------------------------------------------------------------------------------------------------------
// CFAnimMixer:
//-------------------------------------------------------------------------------------------------------------------
class CFAnimMixer {
	// Friends:
	friend class CFAnimCombinerConfig;
	friend class CFAnimCombiner;

public:
	typedef enum {
		TYPE_TAP,									// This mixer is a tap (not a mixer)
		TYPE_BLENDER,								// This mixer is a blender mixer
		TYPE_SUMMER,								// This mixer is a summer mixer

		TYPE_COUNT,

		TYPE_MAX_TAP_COUNT = 253 - TYPE_COUNT		// (The combiner reserves a few values out of a byte-index into a tap table)
	} Type_e;

public:
	// Interface:
	FINLINE cchar *GetName( void ) const { return m_szName; }

	FINLINE CFAnimMixer *GetUpstreamMixer( u32 nInputIndex ) const { FASSERT( nInputIndex < 2 ); return m_apUpstreamMixer[nInputIndex]; }
	FINLINE CFAnimMixer *GetDownstreamMixer( void ) const { return m_pDownstreamMixer; }
	FINLINE u32 GetDownstreamMixerInputIndex( void ) const { return m_nDownstreamMixerInputIndex; }

	FINLINE BOOL IsTap( void ) const { return IsTap(m_nType); }
	FINLINE BOOL IsMixer( void ) const { return IsMixer(m_nType); }
	FINLINE BOOL IsBlender( void ) const { return IsBlender(m_nType); }
	FINLINE BOOL IsSummer( void ) const { return IsSummer(m_nType); }

	static FINLINE BOOL IsTap( Type_e nType ) { return (nType == TYPE_TAP); }
	static FINLINE BOOL IsMixer( Type_e nType ) { return (nType != TYPE_TAP); }
	static FINLINE BOOL IsBlender( Type_e nType ) { return (nType == TYPE_BLENDER); }
	static FINLINE BOOL IsSummer( Type_e nType ) { return (nType == TYPE_SUMMER); }

protected:
	// Construction/Destruction:
	FINLINE CFAnimMixer() {}

	// Data:
protected:
	FLink_t m_Link;							// Link to other mixers
	char m_szName[FANIM_MIXERNAME_LEN+1];	// Mixer or tap name

	Type_e m_nType;							// Mixer type
	CFAnimMixer *m_apUpstreamMixer[2];		// Pointer to the mixers whose outputs are driving each of our inputs ([]=NULL if this is a tap)

	u32 m_nDownstreamMixerInputIndex;		// The input index of the mixer we're driving (must be 0 if this is the root mixer)
	CFAnimMixer *m_pDownstreamMixer;		// Which mixer the output of this mixer drives (NULL if this is the root mixer)

	FCLASS_STACKMEM_NOALIGN( CFAnimMixer );
};



//-------------------------------------------------------------------------------------------------------------------
// CFAnimCombinerConfig:
//-------------------------------------------------------------------------------------------------------------------
class CFAnimCombinerConfig {
public:
	typedef struct {
		u8 nMixerType;						// Set to TYPE_COUNT to terminate the table
		u8 nMixerNameIndex;
		u8 nDownstreamMixerNameIndex;
		u8 nDownstreamMixerInputIndex;
	} ConfigNet_t;


	typedef struct {
		u8 nMixerType;						// Set to TYPE_COUNT to terminate the table
		u8 nMixerNameIndex;
	} ConfigStack_t;


	typedef struct {
		u8 nTapNameIndex;
		u8 nMixerNameIndex;
		u8 nMixerInputIndex;				// Set to 255 to terminate the table
	} ConfigTap_t;


	// Construction/Destruction:
	CFAnimCombinerConfig();
	~CFAnimCombinerConfig() { Destroy(); }

	BOOL BeginCreation( cchar *pszRootMixerName, CFAnimMixer::Type_e nRootMixerType );
	BOOL EndCreation( void );
	FINLINE BOOL IsCreationComplete( void ) const { return m_bCreationComplete; }
	void Destroy( void );

	BOOL AddMixer( cchar *pszMixerName, CFAnimMixer::Type_e nMixerType, cchar *pszDownstreamMixerName, u32 nDownstreamMixerInputIndex );
	FINLINE BOOL AddTap( cchar *pszTapName, cchar *pszDownstreamMixerName, u32 nDownstreamMixerInputIndex ) { return AddMixer( pszTapName, CFAnimMixer::TYPE_TAP, pszDownstreamMixerName, nDownstreamMixerInputIndex ); }
	FINLINE BOOL AddBlender( cchar *pszTapName, cchar *pszDownstreamMixerName, u32 nDownstreamMixerInputIndex ) { return AddMixer( pszTapName, CFAnimMixer::TYPE_BLENDER, pszDownstreamMixerName, nDownstreamMixerInputIndex ); }
	FINLINE BOOL AddSummer( cchar *pszTapName, cchar *pszDownstreamMixerName, u32 nDownstreamMixerInputIndex ) { return AddMixer( pszTapName, CFAnimMixer::TYPE_SUMMER, pszDownstreamMixerName, nDownstreamMixerInputIndex ); }
	BOOL AddNet( const ConfigNet_t *pConfigNet, BOOL bCreateRoot, cchar **apszMixerNameTable, cchar *pszDownstreamMixerName=NULL, u32 nDownstreamMixerInputIndex=0 );
	cchar *AddStack( const ConfigStack_t *pConfigStack, BOOL bCreateRoot, cchar **apszMixerNameTable, cchar *pszDownstreamMixerName, u32 nDownstreamMixerInputIndex );
	BOOL AddTaps( const ConfigTap_t *pConfigTap, cchar **apszTapNameTable, cchar **apszMixerNameTable );

	// Interface:
	CFAnimMixer *FindName( cchar *pszName, BOOL bIsTap ) const;
	CFAnimMixer *FindName( cchar *pszName, CFAnimMixer::Type_e nMixerType ) const;
	CFAnimMixer *FindMixer( cchar *pszMixerName ) const;
	CFAnimMixer *FindTap( cchar *pszTapName ) const;

	u32 GetMixerCount( void ) const { return m_nMixerCount; }
	u32 GetTapCount( void ) const { return m_nTapCount; }

	CFAnimMixer *GetRoot( void ) const;

	FINLINE CFAnimMixer *GetHead( void ) const { return (CFAnimMixer *)flinklist_GetHead(&m_RootMixer); }
	FINLINE CFAnimMixer *GetTail( void ) const { return (CFAnimMixer *)flinklist_GetTail(&m_RootMixer); }
	FINLINE CFAnimMixer *GetNext( CFAnimMixer *pMixer ) const { return (CFAnimMixer *)flinklist_GetNext(&m_RootMixer,pMixer); }
	FINLINE CFAnimMixer *GetPrev( CFAnimMixer *pMixer ) const { return (CFAnimMixer *)flinklist_GetPrev(&m_RootMixer,pMixer); }

	CFAnimMixer *FindMixerHead( void ) const;
	CFAnimMixer *FindMixerTail( void ) const;
	CFAnimMixer *FindMixerNext( CFAnimMixer *pMixer ) const;
	CFAnimMixer *FindMixerPrev( CFAnimMixer *pMixer ) const;

	CFAnimMixer *FindTapHead( void ) const;
	CFAnimMixer *FindTapTail( void ) const;
	CFAnimMixer *FindTapNext( CFAnimMixer *pTap ) const;
	CFAnimMixer *FindTapPrev( CFAnimMixer *pTap ) const;

protected:
	BOOL ConfirmInputsAreHookedUp( const CFAnimMixer *pMixer ) const;

	// Data:
protected:
	BOOL m_bCreationComplete;				// Set to TRUE when EndCreation() is called
	FLinkRoot_t m_RootMixer;				// Linklist of CFAnimMixer objects
	u32 m_nMixerCount;						// Number of mixers in this combiner config
	u32 m_nTapCount;						// Number of taps in this combiner config

	FCLASS_STACKMEM_NOALIGN( CFAnimCombinerConfig );
};


FINLINE CFAnimMixer *CFAnimCombinerConfig::GetRoot( void ) const {
	CFAnimMixer *pMixer;

	pMixer = (CFAnimMixer *)flinklist_GetHead(&m_RootMixer);
	FASSERT( pMixer );

	return pMixer;
}




//-------------------------------------------------------------------------------------------------------------------
// CFAnimCombinerNode:
//-------------------------------------------------------------------------------------------------------------------
class CFAnimCombinerNode {
	// Friends:
	friend class CFAnimCombiner;

protected:
	// Construction/Destruction:
	FINLINE CFAnimCombinerNode() {}

protected:
	// Data:
	const CFAnimMixer *m_pMixer;				// Pointer to the mixer or tap object
	CFAnimCombinerNode *m_pDownstreamNode;		// Points to downstream node (NULL=none)
	CFAnimCombinerNode *m_apUpstreamNode[2];	// Points to the two upstream nodes (NULL if this is a tap)
	u8 m_nDownstreamNodeInputIndex;				// Which input of the downstream node this combiner is connected to
	u8 m_nCombinerArrayIndex;					// Index into CFAnimCombiner::m_pCombinerTapArray or CFAnimCombiner::m_pCombinerMixArray

	// This is the bone mask for the input source if this node is a tap,
	// or the bone mask for the output of the mixer if this is a mixer.
	// Set bits allow source to drive the bone.
	// Bits correspond to the bone array in FMesh_t::pBoneArray.
	// Bit 0 of m_anBoneMask[0] is the mask for FMesh_t::pBoneArray[0].
	// Bit 0 of m_anBoneMask[1] is the mask for FMesh_t::pBoneArray[32].
	u32 m_anBoneMask[FANIM_BONE_BITARRAY_ELEMENT_COUNT];

	FCLASS_STACKMEM_NOALIGN( CFAnimCombinerNode );
};



//-------------------------------------------------------------------------------------------------------------------
// CFAnimCombinerTap:
//-------------------------------------------------------------------------------------------------------------------
class CFAnimCombinerTap : public CFAnimCombinerNode {
	// Friends:
	friend class CFAnimCombiner;

protected:
	// Construction/Destruction:
	FINLINE CFAnimCombinerTap() {}

protected:
	// Data:
	CFAnimSource *m_pAnimSource;				// Pointer to this tap's animation source
	u8 *m_pBoneIndexMeshToAnimTable;			// Index into this array with a mesh bone index and it'll provide the anim source index for that bone

	FCLASS_STACKMEM_NOALIGN( CFAnimCombinerTap );
};



//-------------------------------------------------------------------------------------------------------------------
// CFAnimCombinerMix:
//-------------------------------------------------------------------------------------------------------------------
class CFAnimCombinerMix : public CFAnimCombinerNode {
	// Friends:
	friend class CFAnimCombiner;

protected:
	enum {
		FLAG_SMOOTH				= 0x00000001,		// Runs m_fValue through a smoothing function before using it for mixing (ignored for summers)

		FLAG_NONE				= 0x00000000
	};


protected:
	// Construction/Destruction:
	FINLINE CFAnimCombinerMix() {}


	// Data:
protected:
	f32 m_fValue;							// Current value of this mixer's control
//	f32 m_fPrevValue;						// Previous value of this mixer's control
	u32 m_nFlags;							// See FLAG_* for info

	FCLASS_STACKMEM_NOALIGN( CFAnimCombinerMix );
};



//-------------------------------------------------------------------------------------------------------------------
// CFAnimCombiner:
//-------------------------------------------------------------------------------------------------------------------
class CFAnimCombiner {
protected:
	// Definitions:
	typedef enum {
		EXECCMD_BLEND_LINEAR = CFAnimMixer::TYPE_MAX_TAP_COUNT,
		EXECCMD_BLEND_SMOOTH,
		EXECCMD_SUM,
		EXECCMD_SUM_BLEND,
		EXECCMD_SUM_BLEND_SMOOTH,
		EXECCMD_END
	} ExecCmd_e;

	// Flags:
	enum {
		EXECUTE_BUFFER_REBUILD_NEEDED = 0x0001,
	};

public:

	typedef struct {
		u8 nAnimSourceIndex;			// Index into ppAnimSourceTable
		u8 nTapIDIndex;					// Index into pnAnimTapTable. Set to 255 to terminate table
	} AttachList_t;


	// Normally rNewMtx.Mul( rParentMtx, rBoneMtx) is performed if a callback is not provided.
	typedef void BoneCallbackFcn_t( u32 nBoneIndex, CFMtx43A &rNewMtx, const CFMtx43A &rParentMtx, const CFMtx43A &rBoneMtx );


	// Construction/Destruction:
	CFAnimCombiner();

	// Creation/Destruction:
	BOOL Create( const CFAnimCombinerConfig *pCombinerConfig, CFMeshInst *pMesh );
	BOOL CreateSimple( CFAnimSource *pAnimSource, CFMeshInst *pMesh );
	BOOL CreateSummer( CFMeshInst *pMesh );
	BOOL CreateBlender( CFMeshInst *pMesh );

	// Tap Interface:
	s32 GetTapID( cchar *pszTapName );
	FINLINE CFAnimSource *GetSourceDrivingTap( s32 nTapID ) const;
	void AttachToTap( s32 nTapID, CFAnimSource *pAnimSource, cchar **ppszUnmaskedBoneNames=NULL );
	void AttachToTaps( const AttachList_t *pAttachList, CFAnimSource *pAnimSourceTable, const u8 *pnAnimTapTable );

	// Bone Masking:
	void Mask_EnableAllBones( s32 nTapID );
	void Mask_DisableAllBones( s32 nTapID );
	void Mask_UpdateTapBoneMask( s32 nTapID, cchar **ppszBoneNames, BOOL bDriveBonesInList );
	void Mask_UpdateTapBoneMask( s32 nTapID, const u8 *pnBoneNameIndexTable, cchar **ppszBoneNameTable, BOOL bDriveBonesInList );
	void Mask_UpdateTapBoneMask( s32 nTapID, cchar *pszBoneName, BOOL bDriveBone );
	BOOL Mask_IsDrivingBone( s32 nTapID, cchar *pszBoneName );

	void Mask_BatchUpdateTapBoneMask_Open( s32 nTapID, BOOL bDriveBonesInList );
	void Mask_BatchUpdateTapBoneMask_Close( void );
	void Mask_BatchUpdateTapBoneMask( const u8 *pnBoneNameIndexTable, cchar **ppszBoneNameTable );

	// Control Interface:
	s32 GetControlID( cchar *pszMixerName );
	FINLINE f32 GetControlValue( s32 nControlID );
	FINLINE void SetControlValue( s32 nControlID, f32 fValue );
	FINLINE const CFAnimMixer *GetControlMixer( s32 nControlID ) const;
	FINLINE BOOL IsControlSmoothingEnabled( s32 nControlID ) const;
	FINLINE void EnableControlSmoothing( s32 nControlID );
	FINLINE void DisableControlSmoothing( s32 nControlID );

	// Bone Callback:
	FINLINE void SetBoneCallback( BoneCallbackFcn_t *pFcnCallback ) { m_pFcnBoneCallback = pFcnCallback; }
	FINLINE BoneCallbackFcn_t *GetBoneCallback( void ) const { return m_pFcnBoneCallback; }
	void EnableBoneCallback( cchar *pszBoneName );
	void DisableBoneCallback( cchar *pszBoneName );
	BOOL IsBoneCallbackEnabled( cchar *pszBoneName );
	void EnableBoneCallback( u32 nBoneIndex );
	void DisableBoneCallback( u32 nBoneIndex );
	BOOL IsBoneCallbackEnabled( u32 nBoneIndex );
	void DisableAllBoneCallbacks( void );

	// Matrix Palette:
	void ComputeMtxPalette( BOOL bAllowOffscreenOptimizations=FALSE, u32 nTranslatorBoneIndexToSkip=0xffffffff, CFMtx43A *pDestTranslatorBoneIndex=NULL );
	FINLINE void AtRestMtxPalette( void ) { FASSERT( m_pMeshInst ); m_pMeshInst->AtRestMtxPalette(); }

private:
	void _ComputeDefaultMtxPalette( u32 nBoneIndex, const CFMtx43A &rParentMtx, u32 nTranslatorBoneIndexToSkip=0xffffffff, CFMtx43A *pDestTranslatorBoneIndex=NULL );
	void _ComputeMtxPalette( u32 nBoneIndex, const CFMtx43A &rParentMtx, u32 nTranslatorBoneIndexToSkip=0xffffffff, CFMtx43A *pDestTranslatorBoneIndex=NULL );
	void _ComputeFrame( CFMtx43A &rDestFrameMtx44, u32 nBoneIndex );

	void _Mask_EnableAllBones( CFAnimCombinerTap *pCombinerTap );
	void _Mask_DisableAllBones( CFAnimCombinerTap *pCombinerTap );

	FINLINE void _UpdateBoneBitMask( CFAnimCombinerNode *pCombinerNode, u32 nBoneIndex, BOOL bNewBitValue );
	FINLINE void _SetBoneBitMask( CFAnimCombinerNode *pCombinerNode, u32 nBoneIndex );
	FINLINE void _ClearBoneBitMask( CFAnimCombinerNode *pCombinerNode, u32 nBoneIndex );
	FINLINE BOOL _GetBoneBitMask( CFAnimCombinerNode *pCombinerNode, u32 nBoneIndex );

	CFAnimCombinerNode *_FindCombinerNodeDrivingInput( const CFAnimCombinerNode *pCombinerNode, u32 nInputIndex );
	void _UpdateMixerBoneMasksFromTapToRoot( const CFAnimCombinerTap *pCombinerTap );
	void _BuildBoneExecBuffer( u32 nBoneIndex );
	void _BuildBoneExecBufferAndRecurse( CFAnimCombinerNode *pCombinerNode );
	FINLINE void _AddTokenToExecBuf( u8 nCmdToken );

	// Data:
protected:
	const CFAnimCombinerConfig *m_pCombinerConfig;	// Pointer to the combiner configuration object
	CFMeshInst *m_pMeshInst;						// Pointer to the mesh which this combiner drives
	BoneCallbackFcn_t *m_pFcnBoneCallback;

	u32 m_nBoneCount;								// Number of bones in the mesh instance
	u32 m_nBoneBitArrayElementCount;				// Number of bit array elements = (m_nBoneCount+31)/32
	u32 m_nCombinerTapCount;						// Number of elements in m_pCombinerTap
	u32 m_nCombinerMixCount;						// Number of elements in m_pCombinerMix
	s32 m_nMaskBatchTapID;							// Saved tap ID for Mask_BatchUpdateTapBoneMask*() functions
	BOOL8 m_bMaskBatchDriveBonesInList;				// Saved bDriveBonesInList for Mask_BatchUpdateTapBoneMask*() functions
	u8  m_nKeyInterpolationMode;					// Used for LOD optimization

	u16 m_nFlags;

	u32 m_anBoneCallbackMask[FANIM_BONE_BITARRAY_ELEMENT_COUNT];	// Set bits indicate which bones to call the callback function for

	u8 **m_ppnBoneMixExecBuf;						// Array of bone mix execute buffers (one buffer of u8's per bone)

	CFAnimCombinerNode *m_pRootCombinerNode;		// Root combiner node
	CFAnimCombinerTap *m_pCombinerTapArray;			// Array of CFAnimCombinerTap
	CFAnimCombinerMix *m_pCombinerMixArray;			// Array of CFAnimCombinerMix

	static u32 m_nBoneIndex;						// Used when building the bone exec buffer
	static u32 m_nNextCmdIndex;						// Used when building the bone exec buffer
	static u8 *m_pnExecBuf;							// Used when building the bone exec buffer

#if !FANG_PRODUCTION_BUILD
	u64 uLastComputeMtxPaletteTime;
#endif

	FCLASS_STACKMEM_NOALIGN( CFAnimCombiner );
};


FINLINE f32 CFAnimCombiner::GetControlValue( s32 nControlID ) {
	FASSERT( m_pCombinerConfig );
	FASSERT( (u32)nControlID < m_nCombinerMixCount );
	FASSERT( m_pCombinerMixArray[nControlID].m_pMixer->IsMixer() );
	return m_pCombinerMixArray[nControlID].m_fValue;
}

FINLINE void CFAnimCombiner::SetControlValue( s32 nControlID, f32 fValue ) {
	FASSERT( m_pCombinerConfig );
	FASSERT( (u32)nControlID < m_nCombinerMixCount );
	FASSERT( m_pCombinerMixArray[nControlID].m_pMixer->IsMixer() );

	if ( m_pCombinerMixArray[nControlID].m_fValue == 0.f || m_pCombinerMixArray[nControlID].m_fValue == 1.f ) {
		if ( fValue != m_pCombinerMixArray[nControlID].m_fValue ) {
			// We are currently at 0.f or 1.f and now we're not going to be at that value, so update
			m_nFlags |= EXECUTE_BUFFER_REBUILD_NEEDED;
		}
	} else {
		if ( fValue == 0.f || fValue == 1.f ) {
			// We are currently between 0.f and 1.f and now we are arriving at one of those extremes, so update
			m_nFlags |= EXECUTE_BUFFER_REBUILD_NEEDED;
		}
	}

	m_pCombinerMixArray[nControlID].m_fValue = fValue;
}

FINLINE const CFAnimMixer *CFAnimCombiner::GetControlMixer( s32 nControlID ) const {
	FASSERT( m_pCombinerConfig );
	FASSERT( (u32)nControlID < m_nCombinerMixCount );
	return m_pCombinerMixArray[nControlID].m_pMixer;
}

FINLINE CFAnimSource *CFAnimCombiner::GetSourceDrivingTap( s32 nTapID ) const {
	FASSERT( m_pCombinerConfig );
	FASSERT( nTapID>=0 && nTapID<(s32)m_nCombinerTapCount );
	return m_pCombinerTapArray[nTapID].m_pAnimSource;
}

FINLINE BOOL CFAnimCombiner::IsControlSmoothingEnabled( s32 nControlID ) const {
	FASSERT( m_pCombinerConfig );
	FASSERT( (u32)nControlID < m_nCombinerMixCount );
	return (BOOL)(m_pCombinerMixArray[nControlID].m_nFlags & CFAnimCombinerMix::FLAG_SMOOTH);
}

FINLINE void CFAnimCombiner::EnableControlSmoothing( s32 nControlID ) {
	FASSERT( m_pCombinerConfig );
	FASSERT( (u32)nControlID < m_nCombinerMixCount );
	if ( !IsControlSmoothingEnabled( nControlID ) ) {
		m_nFlags |= EXECUTE_BUFFER_REBUILD_NEEDED;
	}
	FMATH_SETBITMASK( m_pCombinerMixArray[nControlID].m_nFlags, CFAnimCombinerMix::FLAG_SMOOTH );
}

FINLINE void CFAnimCombiner::DisableControlSmoothing( s32 nControlID ) {
	FASSERT( m_pCombinerConfig );
	FASSERT( (u32)nControlID < m_nCombinerMixCount );
	if ( IsControlSmoothingEnabled( nControlID ) ) {
		m_nFlags |= EXECUTE_BUFFER_REBUILD_NEEDED;
	}
	FMATH_CLEARBITMASK( m_pCombinerMixArray[nControlID].m_nFlags, CFAnimCombinerMix::FLAG_SMOOTH );
}

FINLINE void CFAnimCombiner::_UpdateBoneBitMask( CFAnimCombinerNode *pCombinerNode, u32 nBoneIndex, BOOL bNewBitValue ) {

	FASSERT( m_pCombinerConfig );
	FASSERT( nBoneIndex < m_nBoneCount );

	u32 nShiftedValue = (u32)((!!bNewBitValue) << (nBoneIndex & 31));
	u32 *pnArrayElementAddress = &pCombinerNode->m_anBoneMask[ nBoneIndex >> 5 ];

	*pnArrayElementAddress &= ~nShiftedValue;
	if ( !(*pnArrayElementAddress & nShiftedValue) ) {
		m_nFlags |= EXECUTE_BUFFER_REBUILD_NEEDED;
		*pnArrayElementAddress |= nShiftedValue;
	}
}

FINLINE void CFAnimCombiner::_SetBoneBitMask( CFAnimCombinerNode *pCombinerNode, u32 nBoneIndex ) {
	FASSERT( m_pCombinerConfig );
	FASSERT( nBoneIndex < m_nBoneCount );

	u32 nShiftedValue = (u32)(1 << (nBoneIndex & 31));
	u32 *pnArrayElementAddress = &pCombinerNode->m_anBoneMask[ nBoneIndex >> 5 ];

	if ( !(*pnArrayElementAddress & nShiftedValue) ) {
		m_nFlags |= EXECUTE_BUFFER_REBUILD_NEEDED;
		*pnArrayElementAddress |= nShiftedValue;
	}
//	pCombinerNode->m_anBoneMask[ nBoneIndex >> 5 ] |= (1 << (nBoneIndex & 31));
}

FINLINE void CFAnimCombiner::_ClearBoneBitMask( CFAnimCombinerNode *pCombinerNode, u32 nBoneIndex ) {
	FASSERT( m_pCombinerConfig );
	FASSERT( nBoneIndex < m_nBoneCount );

	u32 nShiftedValue = (u32)(1 << (nBoneIndex & 31));
	u32 *pnArrayElementAddress = &pCombinerNode->m_anBoneMask[ nBoneIndex >> 5 ];

	if ( *pnArrayElementAddress & nShiftedValue ) {
		m_nFlags |= EXECUTE_BUFFER_REBUILD_NEEDED;
		*pnArrayElementAddress &= ~nShiftedValue;
	}
//	pCombinerNode->m_anBoneMask[ nBoneIndex >> 5 ] &= ~(1 << (nBoneIndex & 31));
}

FINLINE BOOL CFAnimCombiner::_GetBoneBitMask( CFAnimCombinerNode *pCombinerNode, u32 nBoneIndex ) {
	FASSERT( m_pCombinerConfig );
	FASSERT( nBoneIndex < m_nBoneCount );
	FASSERT( pCombinerNode );

	return (BOOL)((pCombinerNode->m_anBoneMask[ nBoneIndex >> 5 ] >> (nBoneIndex & 31)) & 1);
}

FINLINE void CFAnimCombiner::_AddTokenToExecBuf( u8 nCmdToken ) {
	FASSERT( m_nNextCmdIndex < (m_pCombinerConfig->GetTapCount() * 3 * sizeof(u8) - sizeof(u8)) );
	m_pnExecBuf[m_nNextCmdIndex] = nCmdToken;
	m_nNextCmdIndex++;
}


extern BOOL fanim_ModuleStartup( void );
extern void fanim_ModuleShutdown( void );




#endif

