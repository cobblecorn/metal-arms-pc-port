//////////////////////////////////////////////////////////////////////////////////////
// RoboBuddy.h - Class for Blink's robo buddy.
//
// Author: Justin Link      
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
// 07/09/02 Link        Created.
//////////////////////////////////////////////////////////////////////////////////////
#ifndef _ROBOBUDDY_H_
#define _ROBOBUDDY_H_ 1

#include "fang.h"
#include "fmath.h"
#include "fworld.h"

class CMeshEntity;
class CBot;
class CFAnimManMtx;
class CFAnimMeshRest;
class CFAnimCombiner;
class CFWorldTracker;

FCLASS_ALIGN_PREFIX class CRoboBuddy
{
public:
	CRoboBuddy();
	~CRoboBuddy();

	BOOL Create();
	void Destroy();

	enum State_e
	{
		STATE_STOWED	=	0,
		STATE_DEPLOYING,
		STATE_DEPLOYED,
		STATE_STOWING,
	};

	BOOL StartDeploy();
	void Work(CFVec3A &vecTargetPos);
	void SetUnitEffectEnd(f32 fUnitEffectEnd);
	void StartStow();
	CFVec3A *GetFirePos();		// Returns NULL if not in a state when it can fire.
	CBot *m_pOwnerBot;
private:
	void _Reset();
	void _DoMovement(const CFVec3A &vecGoal/*, f32 fDamp, f32 fRestore, f32 fRepel*/);
	void _PropWork();
	void _CalcPosFromOfs(CFVec3A &vecOut, u32 uPtIdx);

	static BOOL _SphereCollideCallback(CFWorldTracker *pTracker, FVisVolume_t *pVolume, u32 nSphereIndex);
	static CFSphere *s_poSphere;				// So that the callback can access this.
	static CBot *s_pCurOwnerBot;
	static BOOL s_bHitMesh;						// The callback uses this to tell us if we hit any meshes.

	/////////////////////////////////////////////////////////////
	// Used for animating the propeller.
	CFAnimManMtx *m_pManMtx;
	CFAnimMeshRest *m_pRestMtx;
	CFAnimCombiner *m_pAnimCombiner;
	//
	/////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////
	//
	f32 m_fThetaY, m_fOmegaY;		// Yaw
	f32 m_fThetaX, m_fOmegaX;		// Pitch
	//
	/////////////////////////////////////////////////////////////

	f32 m_fUnitEffectEnd;

	CMeshEntity *m_pMeshEntity;
	CFVec3A m_vecVel;

	f32 m_fPropThetaY;

//	f32 m_fSecsSinceLastMove;
	u32 m_uLastHoverPos;

	s32 m_nFireBoneIdx;

	State_e m_eCurState;
	f32 m_fStateTimer;

	FCLASS_STACKMEM_ALIGN(CRoboBuddy);
} FCLASS_ALIGN_SUFFIX;



#endif
