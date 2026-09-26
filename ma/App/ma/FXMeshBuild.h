//////////////////////////////////////////////////////////////////////////////////////
// FXMeshBuild.h - General bot part movement system.
//
// Author: Michael Starich
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
// 03/06/03 Starich     Created.
//////////////////////////////////////////////////////////////////////////////////////
#ifndef _FXMESH_BUILD_H_
#define _FXMESH_BUILD_H_ 1

#include "fang.h"
#include "fmath.h"
#include "fworld.h"
#include "entity.h"
#include "falignedpool.h"


/////////////////////////////////////////////
FCLASS_NOALIGN_PREFIX class CFXMeshBuildPart {
public:
	typedef enum {
		STATE_UNUSED = 0,	// not initialized yet
		STATE_READY,		// part has been initialized but hasn't started moving
		STATE_MOVE,			// the part is moving to the final pos
		STATE_DONE,			// the part has reached the destination

		STATE_COUNT
	} State_e;

	CFXMeshBuildPart();
	~CFXMeshBuildPart();

	static BOOL InitSystem();
	static void UninitSystem();

	BOOL InitPart( const CFVec3A &rFrom, CFMtx43A **ppMtxPalette, u32 nBoneIndex, f32 fUnitBoneSize );
	BOOL Start( CFMtx43A **ppMtxPalette );
	BOOL Work( CFMtx43A **ppMtxPalette, f32 fOOSecsToAnimate, FMeshBone_t *pBoneArray );
	BOOL ForceFinish( CFMtx43A **ppMtxPalette );

	f32 m_fDistSqToTravel;	// used by the mgr to sort the parts

private:
    u16 m_nBoneIndex;		
	u8 m_nState;
	u8 m_nPad;

	f32 m_fAnimTime;
	f32 m_fRadiansToRotate;
	CFVec3A *m_pAxisToRotateAbout;
	CFVec3A *m_pStartPos;
	CFVec3A *m_pDestPos;

	CFQuatA *m_pRotStart;
	CFQuatA *m_pRotDest;

	void ReturnPooledMemory();

	static CFAlignedPool *m_pAlignedMemPool;

	FCLASS_STACKMEM_NOALIGN( CFXMeshBuildPart );
} FCLASS_NOALIGN_SUFFIX;


///////////////////////////////////////////////////
FCLASS_NOALIGN_PREFIX class CFXMeshBuildPartMgr {
public:
	typedef enum {
		STATE_NEED_INIT = 0,
		STATE_AVAILABLE,
		STATE_IN_USE,
		
		STATE_COUNT
	} State_e;

	CFXMeshBuildPartMgr();
	~CFXMeshBuildPartMgr();

	static BOOL InitSystem();
	static void UninitSystem();
	static void SubmitInitNeeds( u8 nMaxBones, u32 nMaxSimultaneousMangers );
	static BOOL InitLevel();
	static void UninitLevel();
	static CFXMeshBuildPartMgr *InitMgr( const CFVec3A &rFrom, CFWorldMesh *pWorldMesh,
										f32 fSecsToComplete, f32 fUnitOverlap=0.0f,
										u32 nNumAdditionalBones=0, u32 *panAdditionalBones=NULL );
	static CFXMeshBuildPartMgr *InitMgr( const CFVec3A &rFrom, CEntity *pEntity, CFWorldMesh *pWorldMesh,
										f32 fSecsToComplete, f32 fUnitOverlap=0.0f );
	static void CalculateDropSecs( f32 fSecsToComplete, f32 fUnitOverlap, f32 fNumParts, f32 &rfSecsBetweensParts, f32 &rfSecsPerPart );
	
	FINLINE BOOL IsMgrAvailable()	{ return (m_nState == STATE_AVAILABLE); }

    BOOL Work( f32 &rfUnitCompletePercent );
	void ForceFinish();
	u32 GetTotalPartCount();

private:
	BOOL Init( const CFVec3A &rFrom, CFWorldMesh *pWorldMesh,
			f32 fSecsToComplete, f32 fUnitOverlap=0.0f,
			u32 nNumAdditionalBones=0, u32 *panAdditionalBones=NULL );
	BOOL AllocateMemory( u8 nMaxBonesToSupport );
	void ReleaseMemory(); 
	void UpdateBoundingInfo();

	u8 m_nNumParts;	// how many parts are used
	u8 m_nState;	// what state are we in
	u8 m_nNextBoneToStart;// the index of the next bone to start animating
	u8 m_nOldestWorkingBone;// the index of the oldest bone still animating

	u32 m_nMaxParts;	// the max number of parts allowed
	CFXMeshBuildPart *m_paParts;// there are m_nMaxPart of these
	CFXMeshBuildPart **m_papSortedParts;// the part list in the order that they will be started (there are m_nMaxPart of these)

	f32 m_fOOPartSecsToComplete;// 1 / (the number of secs each part takes to complete)
	f32 m_fSecsTillNextStart;
	f32 m_fSecsBetweenStarts;
	f32 m_fTimeSoFar;
	f32 m_fOOTotalSecsToComplete;
	CFWorldMesh *m_pWorldMesh;

	static u32 m_nRequestedMaxBones;
	static u32 m_nRequestedMaxSimultaneousMangers; 
	
	static u32 m_nSizeOfMgrPool;
	static CFXMeshBuildPartMgr *m_paMgrPool;// points to m_nSizeOfMgrPool elements

	FCLASS_STACKMEM_NOALIGN( CFXMeshBuildPartMgr );
} FCLASS_NOALIGN_SUFFIX;

#endif