//////////////////////////////////////////////////////////////////////////////////////
// eproj_grenade.h - Grenade projectiles.
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

#ifndef _EPROJ_GRENADE_H_
#define _EPROJ_GRENADE_H_ 1

#include "fang.h"
#include "eproj.h"
#include "fmotion.h"
#include "explosion.h"


class CEProj;
class CFCollInfo;
class CFTrackerCollideProjSphereInfo;




//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CEProj_Grenade
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

FCLASS_ALIGN_PREFIX class CEProj_Grenade : public CEProjExt {
//----------------------------------------------------------------------------------------------------------------------------------
// Private Data:
//----------------------------------------------------------------------------------------------------------------------------------
private:
	BOOL m_bStopped;
	BOOL m_bEMPDet;

	f32 m_fSecsUntilNextRecruit;
	f32 m_fSecsUntilNextSound;
	f32 m_fCantHitThrowerTime;
	u32 m_uAISoundHandle;		// this projectile may be visible to AI, and if so, this u32 AISoundHandle is crucial to the functionality

	CFVec3A m_CurP1, m_CurP2;	// Current position of the two end points

	CFMotion m_Motion;
	CEProj_Grenade_Params_t m_SetupParams;

	
	// This is so we can pass the value in to the collision callback
	static f32 m_fCallbackBotShutdownTime;

	static CEProj *m_pCallbackProj;
	static CFCollInfo m_CollInfo;
	static CFCollInfo m_CollInfoFat;
	static CFCollInfo m_CollInfoEMP;
	static CFTrackerCollideProjSphereInfo m_CollProjSphereInfo;




//----------------------------------------------------------------------------------------------------------------------------------
// Public Functions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	// Construct/Destruct:
	CEProj_Grenade() {}
	~CEProj_Grenade() {}

	// System:
	static BOOL InitSystem( void );
	static void UninitSystem( void );

	// Extension interface:
	virtual BOOL Create( CEProj *pProj );
	virtual void Init( CEProj *pProj );

	virtual void Launched( CEProj *pProj );
	virtual BOOL Detonated( CEProj *pProj, BOOL bMakeEffect, u32 nEvent, FCollImpact_t *pCollImpact );

	virtual BOOL Work( CEProj *pProj );

	virtual void SetGrenadeParams( const CEProj_Grenade_Params_t *pParms );
	
	virtual u8 ComputeProjectileReaction( const FCollImpact_t *pCollImpact );

	FCLASS_STACKMEM_ALIGN( CEProj_Grenade );




//----------------------------------------------------------------------------------------------------------------------------------
// Private Functions:
//----------------------------------------------------------------------------------------------------------------------------------
private:

	static BOOL _TrackersCallbackProjSphere( CFWorldTracker *pTracker, FVisVolume_t *pWorldLeafNode );
	static BOOL _EMPTrackerCallback( CFWorldTracker *pTracker, FVisVolume_t *pWorldLeafNode );

	BOOL _HandleDetonation( CEProj *pProj, FCollImpact_t *pNearImpact );
	void _HandleCollision( CEProj *pProj, FCollImpact_t *pNearImpact );
	BOOL _WorkDetonatedEMP( CEProj *pProj );


} FCLASS_ALIGN_SUFFIX;

#endif

