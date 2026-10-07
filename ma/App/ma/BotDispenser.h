//////////////////////////////////////////////////////////////////////////////////////
// BotDispenser.h - Bot Dispenser system
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
// 03/11/03 Starich     Created.
//////////////////////////////////////////////////////////////////////////////////////
#ifndef _BOT_DISPENSER_H_
#define _BOT_DISPENSER_H_ 1

#include "fang.h"
#include "bot.h"
#include "meshentity.h"
#include "FXMeshBuild.h"
#include "fsound.h"

class CAlarmNet;

////////////////////////////////////////////////////////////////////////////
// TERMS:
//	dispenser - the actual hardware that spawns bots
//	spawn - act of creating a single bot
//	session - the period of time that a dispenser is actively spawning bots
////////////////////////////////////////////////////////////////////////////

enum {
	// various counts
	BOT_DISPENSER_MAX_UNIQUE_TYPES_PER_DISPENSER = 4,
	BOT_DISPENSER_NUM_HAND_BONES = 2,
	BOT_DISPENSER_BONE_NOT_FOUND = 255,
	
	// bone id related
	BOT_DISPENSER_BONE_ID_LEFT_ARM = 0,	// must be 0
	BOT_DISPENSER_BONE_ID_RIGHT_ARM,	// must be 1
	BOT_DISPENSER_BONE_ID_DROP,
	BOT_DISPENSER_BONE_ID_TARGET,
	BOT_DISPENSER_BONE_ID_BEAM,

	BOT_DISPENSER_BONE_ID_COUNT,
	
	// flag related
	BOT_DISPENSER_FLAGS_PUMP_OUT_BOTS		= 0x01,// don't wait for a spawned bot to die before starting the next one
	BOT_DISPENSER_FLAGS_RANDOM_BOTS			= 0x02,// randomize the various type of robots spawned 
	BOT_DISPENSER_FLAGS_ACTIVE				= 0x04,// the dispenser has been turned on (only if this flag is set can the dispenser spawn bots)
	BOT_DISPENSER_FLAGS_NEED_BOT			= 0x08,// the dispenser is ready to dispense a bot (can only be set if ACTIVE is set)
	BOT_DISPENSER_FLAGS_LEFT_HAND_SPARK		= 0x10,// emit sparks from the left hand
	BOT_DISPENSER_FLAGS_NAME_MUST_MATCH		= 0x20,// only dispense bots whose name is an exact match to the init specified ones
		
	BOT_DISPENSER_FLAGS_NONE				= 0x00
};

typedef struct {
	// related to the type & count of bots dispensed
    cchar *apszBotNamesToSpawn[BOT_DISPENSER_MAX_UNIQUE_TYPES_PER_DISPENSER];
	BOOL bWaitForBotDeath;
	BOOL bRandomizeBots;			
	u32 nBotsPerSession;			// how many bots should be spawned during a session
	f32 fSecsBetweenSpawns;			// how many seconds should there be between spawns (only used if !bWaitForBotDeath)
	f32 fSecsBetweenSession;		// how long should the dispenser wait after the last spawn before starting a new one
	BOOL bMatchBotNames;			// only dispense bots with exact name match

	// related to the beam fx
	cchar *pszBeamMeshName;			// what is the name of the mesh used during spawn (the beam fx)
	cchar *pszBeamBoneName;			// at what dispenser bone should the beam be attached
	f32 fMinBeamScale;
	f32 fMaxBeamScale;
	f32 fSecsToMaxBeamScale;
	f32 fSecsToMinBeamScale;
    
	// related to the parts falling
	cchar *pszFallingPartsBoneName;	// at what dispenser bone should the pieces fall from
    cchar *pszBotPositionBone;		// at what dispenser bone should the bot be positioned
	f32 fSpawnSecs;					// how long will each bot take to spawn
	f32 fUnitPartOverlap;			// a unit factor to control how much part overlap there is
	
	// related to the arm animation
	cchar *pszArmAnimationResName;	// the name of the arm animation file
	cchar *pszSparkResName;			// what spark fx should be played 
	cchar *pszSmokeResName;			// what spark fx should be played 
	cchar *apszArmBoneNames[BOT_DISPENSER_NUM_HAND_BONES];// the names of the dummy bones of the 2 arms
	cchar *pszGenParticleFxResName;	// what particle generation fx should be played
	f32 fGenFxIntensity;			// what intensity should the gen fx be played at
	f32 fSecsBeforeInitialHandContact;
	f32 fSecsOfFinalArmStow;	

	// related to sounds
	cchar *pszSoundBankToLoad;		// what sound bank needs to be loaded
	cchar *pszArmSound;				// what sound should be played while the arms are moving
	cchar *pszBeamSound;			// what sound should be played while the beam is up
	cchar *pszPartsSound;			// what sound should be played while the parts are dropping
	cchar *pszDingSound;			// what sound should be played when all finished (microwave sound)	

} BotDispenser_Init_t;


FCLASS_NOALIGN_PREFIX class CBotDispenser
{
public:
	typedef enum {
		STATE_UNUSED = 0,// the dispenser has not been properly inited, basically just allocated and nothing more
		STATE_NEED_TO_RESOLVE,// the dispenser has been inited, but needs to resolve pointers before it is ready to be used
		STATE_DESTROYED,// the dispenser is destroyed
		STATE_READY,	// the dispenser is inited properly but not on ( waiting for TurnOn() call )
		STATE_SPAWNING,	// the dispenser is currently assembling a bot
		STATE_RETRACT_ARMS,// the dispenser is currently retracting the arms
		STATE_STANDBY,	// the dispenser is on but not spawing, either waiting to start, or waiting for a bots death

		STATE_COUNT
	} State_e;

	typedef struct {
		u8 nBotType;//BotClass_e
		BOOL8 bTetherPortRequired;
		u8 nWeaponType;// WeaponType_e
	} BotTypeInfo_t;

	CBotDispenser();
	~CBotDispenser();

	static void SetInitDataToDefaults( BotDispenser_Init_t *pInit );

	// initializes a dispenser, does tons of error checking on the init data and will
	// return TRUE if dispenser inited ok and is ready to be used, FALSE if there was a problem
	BOOL Init( BotDispenser_Init_t *pInitData, CEntity *pDispenserEntity );

	// called after all entities have been setup and the auto bot pool is setup, init must have been called before this
	BOOL ResolveFixups( CBot **papAutoBots, u32 uNumAutoBots, u32 &rnMaxBotBoneCount );		

	void Uninit();

	void TurnOn( CAlarmNet *pNet );
	void TurnOff( CAlarmNet *pNet, BOOL bImmediately=FALSE );
	void Work( CAlarmNet *pNet );

	// Returns TRUE If the dispenser wants an autobot
	BOOL NeedsAutoBot();  
	BOOL IsBotExiting( const CBot *pBot ) const;
	
	// Returns the index into papAutoBots that was used by the dispenser, -1 in none were used
    s32 DeployAutoBot( CBot **papAutoBots, const u8 *pauAutoBotUseageFlags, u32 uNumAutoBots );
	
	static void MeshEntityDieCallback( CMeshEntity *pEntity, void *pUserParam );
	void EntityIsDead();

	FLink_t m_DispenserLink;				 // Dispensers are linked within an alarm net
	CMeshEntity *m_pDispenserEntity;

private:
	// misc vars:	
	f32 m_fTimer;
	f32 m_fSecsToWait;// if negative, wait forever
	u8 m_aBoneIDs[BOT_DISPENSER_BONE_ID_COUNT];
	u8 m_nState;
    u8 m_nNumTypes;
	s8 m_nBotIndexToDispenseNext;
	u8 m_nFlags;
	BOOL8 m_bRestoreTargertable;
	BOOL8 m_bRestoreInvincible;
	BOOL8 m_bRestoreActionable;
	BOOL8 m_bPlayDingSound;
	CFWorldAttachedLight *m_pLight;

	BotTypeInfo_t m_aBotInfo[BOT_DISPENSER_MAX_UNIQUE_TYPES_PER_DISPENSER];
	cchar *m_apszBotNames[BOT_DISPENSER_MAX_UNIQUE_TYPES_PER_DISPENSER];
	
	// bot spawning vars:
	CFXMeshBuildPartMgr *m_pMeshBuilder;
	CBot *m_pBotInConstruction;
	u16 m_nBotsPerSession;			// how many bots should be spawned during a session
	u16 m_nNumBotsThisSession;		// how many bots have been dispensed this session?
	f32 m_fSecsBetweenSpawns;		// how many seconds should there be between spawns
	f32 m_fSecsBetweenSession;		// how long should the dispenser wait after the last spawn before starting a new one

	// watched bot
	CBot *m_pWatchedBot;
	u32 m_nBotGUID;

	// beam fx related:
	CFWorldMesh *m_pBeamFxMesh;		// if NULL, there is no beam work done
	f32 m_fMinBeamScale;
	f32 m_fMaxBeamScale;
	f32 m_fSecsToMaxBeamScale;
	f32 m_fSecsToMinBeamScale;
	f32 m_fBeamTimer;

	f32 m_fSpawnSecs;				// how long will each bot take to spawn
	f32 m_fUnitPartOverlap;			// a unit factor to control how much part overlap there is

	// animating arm related:
	CFAnimInst *m_pBuildAnim;		// if NULL, there is no arm work done
	CFAnimCombiner *m_pAnimCombiner;
		// vars based solely on the animation data & csv values
	f32 m_fArmAnim_SecsTillFirstContact;
	f32 m_fArmAnim_SecsRegPartDrop;
	f32 m_fArmAnim_PercentOfAnim1stContact;
	f32 m_fArmAnim_PercentOfAnimPartDrop;
	f32 m_fArmAnim_SecsOfArmStow;
		// vars based one the actual bot being dispensed and the above ArmAnim values
	f32 m_fBotArm_UnitSecsTillFirstContact;
	f32 m_fBotArm_OODropNormalizer;
	f32 m_fSecsTillNextSpark;
	f32 m_fSecsBetweenPartLandings;
	
	// particle related vars:
	FParticle_DefHandle_t m_hContactParticle;	// if FPARTICLE_INVALID_HANDLE, no spark work is done
	FParticle_DefHandle_t m_hSmokeParticle;		// if FPARTICLE_INVALID_HANDLE, no smoke work is done
	FParticle_DefHandle_t m_hGenFXParticle;
	FParticle_EmitterHandle_t m_hGenFXEmitter;
	f32 m_fGenFXIntensity;

	// sound related vars
	CFSoundGroup *m_pArmSndGrp;
	CFSoundGroup *m_pBeamSndGrp;
	CFSoundGroup *m_pPartsSndGrp;
	CFSoundGroup *m_pDingSndGrp;
	CFAudioEmitter *m_pArmSound;
	CFAudioEmitter *m_pBeamSound;
	f32 m_fSecsTillNextPartSound;
		
public:

protected:

private:
	BOOL InitBoneIndices( CFWorldMesh *pWorldMesh, BotDispenser_Init_t *pInitData );
	BOOL InitBotInfo( BotDispenser_Init_t *pInitData );
	void SetBotProperties( BOOL bStartingSpawn, CBot *pBot );
	BOOL IsWatchedBotIsDead();
	BOOL IsWatchedBotInsideDispenser() const;
	BOOL HasWatchedBotExitedTheDispenser();
	void SetWaitTimes();
	void SetupNeedBotVars();
	BOOL ShouldBotBeDispensedByThisMachine( CBot *pBot, BotTypeInfo_t *pBotType, cchar *pszNameOfDesiredBot );

	void StartBeam();
	void UpdateBeam( f32 fUnitDropTime );
	void EndBeam();

	void StartArmAnimation();
	void UpdateArmAnimation( f32 fUnitDropTime );
	void EndArmAnimation();

	void ConfigureAI( CBot *pBot, f32 fFeetToWalkInZDir );

	void ToggleArmSound( BOOL bOn );
	void ToggleBeamSound( BOOL bOn );
	void TogglePartSound( BOOL bOn );
	void UpdateVolumes( f32 fUnitDropTime );

	FCLASS_STACKMEM_NOALIGN( CBotDispenser );
} FCLASS_NOALIGN_SUFFIX;




#endif
