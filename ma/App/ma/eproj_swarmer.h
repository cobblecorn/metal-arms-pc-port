//////////////////////////////////////////////////////////////////////////////////////
// eproj_swarmer.h - Swarmer projectiles.
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
// 06/19/02 Ranck       Created.
//////////////////////////////////////////////////////////////////////////////////////

#ifndef _EPROJ_SWARMER_H_
#define _EPROJ_SWARMER_H_ 1

#include "fang.h"
#include "eproj.h"


#define EPROJ_SWARMER_MERV_ROCKET_COUNT		3


class CEProj;



//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CEProj_Swarmer
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

FCLASS_ALIGN_PREFIX class CEProj_Swarmer : public CEProjExt {
//----------------------------------------------------------------------------------------------------------------------------------
// Private Data:
//----------------------------------------------------------------------------------------------------------------------------------
private:

	f32 m_fSwarmAmplitude_WS;				// Maximum deviation away from CEProj::m_LinUnitDir_WS (0=no swarming)
	f32 m_fRotateZ;							// Rotation of our swarm plane around its Z axis
	f32 m_fDistScale;						// For randomness: Multiplied by the projectile's distance
	f32 m_fDistOffset;						// For randomness: Added to the projectile's distance
	f32 m_fStartDist;						// Snapshot of the distance travelled at the time when swarming starts
	f32 m_fMervSpawnSecs;					// Time until we spawn our Mervs
	f32 m_fIgniteSecs;						// Time until we ignite the Merv
	u32 m_uAISoundHandle;					// some of these projectiles may be set to make AI aware of them. this u32 is the backbone to that desired functionality.	

	BOOL8 m_bMervRocket;					// TRUE if this is a Merv rocket
	BOOL8 m_bMervDestIsPoint;				// TRUE if m_MervDest is a position, FALSE if m_MervDest is a unit direction
	CEProj *m_apMervProj[EPROJ_SWARMER_MERV_ROCKET_COUNT];	// For swarmers, this points to the Mervs it launched. For Mervs, [0] points to the swarmer that launched it.

	CFVec3A m_LaunchPos_WS;					// The point at which the projectile was last launched
	CFQuatA m_LaunchQuat;					// Quaternion representation of original matrix when launched
	CFVec3A m_MervDest;						// Merv's destination unit direction or position (world space)

	CEProj_Swarmer_Params_t m_SetupParams;




//----------------------------------------------------------------------------------------------------------------------------------
// Public Functions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	// Construct/Destruct:
	CEProj_Swarmer() {}
	~CEProj_Swarmer() {}


	// Extension interface:
	virtual BOOL Create( CEProj *pProj );

	virtual void Init( CEProj *pProj );
	virtual void Launched( CEProj *pProj );
	virtual BOOL Detonated( CEProj *pProj, BOOL bMakeEffect, u32 nEvent, FCollImpact_t *pCollImpact );

	virtual BOOL Work( CEProj *pProj );

	virtual void SetSwarmerParams( CEProj *pProj, const CEProj_Swarmer_Params_t *pParms );




//----------------------------------------------------------------------------------------------------------------------------------
// Private Functions:
//----------------------------------------------------------------------------------------------------------------------------------
private:

	void _StartSwarming( CEProj *pProj );
	void _LaunchMervs( CEProj *pProj );
	void _MervWork( CEProj *pProj );
	void _InformMervOfTarget( u32 nMervIndex, CEProj *pMervProj, CEProj *pOwnerProj );
	void _InformOwnerRocketThatMervDetonated( CEProj *pDetonatedMervProj );


	FCLASS_STACKMEM_ALIGN( CEProj_Swarmer );
} FCLASS_ALIGN_SUFFIX;




#endif

