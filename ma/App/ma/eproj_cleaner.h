//////////////////////////////////////////////////////////////////////////////////////
// eproj_cleaner.h - cleaner canister projectiles.
//
// Author: Chris MacDonald
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
// 09/09/02 MacDonald   Created.
//////////////////////////////////////////////////////////////////////////////////////

#ifndef _EPROJ_CLEANER_H_
#define _EPROJ_CLEANER_H_ 1

#include "fang.h"
#include "eproj.h"
#include "fmotion.h"
#include "ftext.h"
#include "player.h"

#define CLEANER_CANISTER_COS_OF_HALF_TARGETING_FOV	( 0.83868f )	// 33 degrees
#define CLEANER_CANISTER_MAX_DETONATION_HEIGHT		( 20.0f )
#define CLEANER_CANISTER_DETONATION_RADIUS_SQUARED	( 36.0f )
#define CLEANER_CANISTER_MAX_ROCKETS					( 3 )
#define CLEANER_CANISTER_MAX_RANGE					( 200.0f )
#define CLEANER_CANISTER_MAX_RANGE_SQ				( CLEANER_CANISTER_MAX_RANGE  * CLEANER_CANISTER_MAX_RANGE )
#define CLEANER_RETICLE_NUM_VERTICES				( 24 )
#define CLEANER_RETICLE_MAX_TRAIL					( 4 )



class CEProj;
class CWeaponCleaner;

//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CEProj_Cleaner
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

FCLASS_ALIGN_PREFIX class CEProj_Cleaner : public CEProjExt {
private:
typedef struct
{
	CEntity	*pEntity;		// pointer to inserted entity
	u32		nGuid;			// entity guid at insertion time
	f32		fInsertionTime;	// time entity was inserted into list
} _EntityTarget_t;

typedef struct
{
	CFVec3A	vPos_WS;
	f32		fDelta;
	f32		fAngle;
	BOOL	bDraw;
} _ReticleHistory_t;

//----------------------------------------------------------------------------------------------------------------------------------
// Public Data:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	_ReticleHistory_t m_aReticleHistory[ CLEANER_CANISTER_MAX_ROCKETS ][ CLEANER_RETICLE_MAX_TRAIL ];
	s32 m_nReticleHistoryIndex;
	static FTextAreaHandle_t m_hTextBoxSelectTargets;
	static FTextAreaHandle_t m_hTextBoxNumTargets;
	static CFVec3A m_avReticleVertex[ CLEANER_RETICLE_NUM_VERTICES ];

//----------------------------------------------------------------------------------------------------------------------------------
// Private Data:
//----------------------------------------------------------------------------------------------------------------------------------
private:
	static CEProj_Cleaner *m_apTargetingCleaner[MAX_PLAYERS];	// Only one cleaner per player, none for AI

	CEProj_Cleaner_Params_t m_SetupParms;				// Parameters specified by launcher
	s32 m_nTargets;
	_EntityTarget_t m_aTargets[ CLEANER_CANISTER_MAX_ROCKETS ];
	CFMotion m_Motion;
	CFVec3A m_vTarget_WS;
	f32 m_fSecsUntilNextSound;
	f32 m_fTextBlinkTimer;
	f32 m_fAngle;
	u32 m_uAISoundHandle;
	BOOL m_bLaunched;
	CFAudioEmitter *m_pWhirSoundEmitter;


//----------------------------------------------------------------------------------------------------------------------------------
// Public Functions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	// Construct/Destruct:
	CEProj_Cleaner() {}
	~CEProj_Cleaner() {}


	// Extension interface:
	virtual BOOL Create( CEProj *pProj );
	virtual void Init( CEProj *pProj );
	static BOOL InitLevel( void );
	static void UnInitLevel( void );
	static void ClearTargetingCleaners( void );

	void ClearTargetingCleaner(CWeaponCleaner* pCleanerWeapon);

	virtual void SetCleanerParams( CEProj *pProj, const CEProj_Cleaner_Params_t *pParams );
	virtual void Launched( CEProj *pProj );
	virtual BOOL Detonated( CEProj *pProj, BOOL bMakeEffect, u32 nEvent, FCollImpact_t *pCollImpact );

	virtual BOOL Work( CEProj *pProj );
	BOOL AssignTarget( s32 nPlayer, CEntity *pCleanerEntity, CEntity *pTargetEntity, CFSoundGroup *pSoundGroupLock );
	FINLINE BOOL IsLaunched( void ) { return m_bLaunched; };

	static BOOL _FindBotsCallback( CFWorldTracker *pTracker, FVisVolume_t *pVolume );
	static void DrawTargets( s32 nPlayer );
	static BOOL InitSystem( void );

	FINLINE s32 NumTargetsInList( void ) { return m_nTargets; };
	void InitTargetList( void );
	BOOL AddToTargetList( CEntity *pEntity );
	BOOL RemoveFromTargetList( CEntity *pEntity );
	CEntity *GetTargetByIndex( s32 index );
	BOOL InTargetList( CEntity *pEntity );
	BOOL TargetListGuidMatches( CEntity *pEntity );
	f32 GetTargetTimeByIndex( s32 index );


	FCLASS_STACKMEM_ALIGN( CEProj_Cleaner );
} FCLASS_ALIGN_SUFFIX;




#endif

