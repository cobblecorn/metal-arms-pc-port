//////////////////////////////////////////////////////////////////////////////////////
// AIFormations.h - 
//
//
//
//	Formations are used by AI leaders to help manage the positioning/orienting of 
//  following units
//
//		- Leader Units own formations.
//		- Formations manage the position of posts within them
//      - Followers ask their leaders formation for a current Goal location and orientation
//
// Author: Pat MacKellar 
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
// 04/30/02 MacKellar   Created.
// 07/07/02 MacKellar	Re-Created
//////////////////////////////////////////////////////////////////////////////////////
#ifndef _AIFORMATIONS_H_
#define _AIFORMATIONS_H_ 1

#include "fmath.h"
#include "ftl.h"

class CAIBrain;

FCLASS_NOALIGN_PREFIX class CAIPost
{
public:
	CAIPost(void)
		:	m_pFollower(NULL),
			m_uPostFlags(POSTFLAG_NONE),
			m_uPostAcquirePathFailureCount(0)
	{
	};
	CAIPost(const CAIPost& CopyThis)
		:	m_pFollower(CopyThis.m_pFollower),
			m_uPostFlags(CopyThis.m_uPostFlags),
			m_uPostAcquirePathFailureCount(0),
			m_Pos_WS(CopyThis.m_Pos_WS)
	{
	}
	CAIPost(CAIBrain* pFollower)
		:	m_pFollower(pFollower),
			m_uPostFlags(POSTFLAG_NONE),
			m_uPostAcquirePathFailureCount(0)
	{
	}

	void Reset(void);
		
	enum 
	{
		POSTFLAG_NONE,
		POSTFLAG_FORMATIONSLOT,		 //this post is a specific formation slot z
	};


	CFVec3 m_Pos_WS;
	CAIBrain* m_pFollower;
	u8 m_uPostFlags;
	u8 m_uPostAcquirePathFailureCount;
	FCLASS_STACKMEM_NOALIGN(CAIPost);
} FCLASS_NOALIGN_SUFFIX;


FCLASS_ALIGN_PREFIX class CAIFormation
{
public:

	CAIFormation(void);
	virtual ~CAIFormation(void);
	void _ClearData(void);

	enum
	{
		FORMATIONTYPE_INVALID = 0,
		FORMATIONTYPE_GLITCHBUDDIES,
		NUM_FORMATIONTYPES
	};

	void Init(u8 uFormationType, CAIBrain* pLeaderBrain);
	void Work(void);
	void Cleanup(void);
	void DebugRender(void);

	CAIPost* Join(CAIBrain* pBrain);
	BOOL Quit(CAIBrain* pBrain);

	CFMtx43A m_FormationMtx;
	CAIBrain* m_pLeaderBrain;
	f32 m_fFormationMarkerResetRad;
	u8 m_uFormationType;

	//
	//	data for FORMATIONTYPE_GLITCHBUDDIES,
	//
	enum
	{
		NUM_FORMATION_POSTS = 4,
	};
	BOOL HasAssignment(CAIBrain* pFollower, u32* puPost = NULL);
	f32 m_fFormationYaw;
	CAIBrain* m_apPostAssignment[NUM_FORMATION_POSTS];	  //which follower Brain is assigned to each Post
	CFVec3A m_aPostVec_MS[NUM_FORMATION_POSTS];
	
	FCLASS_STACKMEM_ALIGN(CAIFormation);
} FCLASS_ALIGN_SUFFIX;


enum
{
	AIFORMATION_DEFAULT_GLOBAL_FORMATION_BANK_SIZE = 4,   //max total number of leaders using formations at any given time
	AIFORMATION_DEFAULT_GLOBAL_POST_BANK_SIZE = 25,	      //max total number of followers of leaders using formations at any given time
};


BOOL AIFormations_InitSystem(	struct FLinkRoot_s* pGlobalPtrNodePool,
								u32 uFormationBankSize = AIFORMATION_DEFAULT_GLOBAL_FORMATION_BANK_SIZE,
								u32 uPostBankSize = AIFORMATION_DEFAULT_GLOBAL_POST_BANK_SIZE);
void AIFormations_UninitSystem(void);
void AIFormations_Work(void);

CAIPost* AIFormations_GetPost(void);
CAIFormation* AIFormations_RecyclePost(CAIPost* pPost);

CAIFormation* AIFormations_Get(void);
void AIFormations_Recycle(CAIFormation* pFormation);

#endif

