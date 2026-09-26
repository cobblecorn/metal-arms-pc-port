//////////////////////////////////////////////////////////////////////////////////////
// GeneralCorrosiveGame.h
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
// 04/07/03 Foushee		Created.
//////////////////////////////////////////////////////////////////////////////////////

#ifndef _GENERALCORROSIVEGAME_H_
#define _GENERALCORROSIVEGAME_H_ 1


// =============================================================================================================

#include "fang.h"
#include "level.h"
#include "eboomer.h"
#include "collectable.h"
#include "ai\\aicorrosive.h"
#include "BotCorrosive.h"
#include "BotGlitch.h"

#include "sas_user.h"

FCLASS_ALIGN_PREFIX class CGeneralCorrosiveGame {
//----------------------------------------------------------------------------------------------------------------------------------
// Public Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	typedef enum {
		MGSOUND_ALLOYNODAMAGE,

		MGSOUND_COUNT
	} MiniGameSounds_e;

//----------------------------------------------------------------------------------------------------------------------------------
// Protected Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
protected:


//----------------------------------------------------------------------------------------------------------------------------------
// Private Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
private:

	typedef struct {
		CEntity* pTripwireEntity;
		u32 nGrateEntities;
		CEBoomer** papGrateEntities;
	} _TrenchTripwireInfo_t;

	typedef struct {
		CEntity* pTripwireEntity;
		CEntity* pAttackLocationEntity;
		CEntity* pBridgeSectionEntity;
	} _BridgeTripwireInfo_t;

//----------------------------------------------------------------------------------------------------------------------------------
// Public Data:
//----------------------------------------------------------------------------------------------------------------------------------
public:


//----------------------------------------------------------------------------------------------------------------------------------
// Private Data:
//----------------------------------------------------------------------------------------------------------------------------------
private:
	static CGeneralCorrosiveGame*	m_pGeneralCorrosiveGame;			// the single instance of this class
	
	BOOL m_bCreated;

	// General Gameplay Parameter factors
	// Note -- If you add a parameter to this field, be sure to change the #Define in  GeneralCorrosiveGame.cpp 
	// which defines how many csv gameplay parameters there are to be loaded in!
	f32 m_fMinAggressionLevel;									// The baseline aggression level corrosive starts at
	f32 m_fMaxAggressionLevel;									// The maximum aggression level corrosive can achieve
	f32 m_fAggressionPerSecondIncrementFactor;					// How fast his aggression level increments on a per second basis
	f32 m_fAggressionPerChipAquiredIncrementFactor;				// How much his aggression level increases with each chip aquired
	f32 m_fAggressionPerChipExposedIncrementFactor;				// How much his aggression level increases when the crate the chip is in gets destroyed.
	f32 m_fSkipPeerAggressionThreshold;							// The level at which corrosive skips the peer move and goes straight to stomping
	u32 m_uNoDamageNumCallbacks;								// The number of times corrosive gets hit before Alloy informs glitch that corrosive isn't taking damage
	f32 m_fChipScale;											// The scale used for the chips placed in this level.
	f32 m_fAlloyMessageMinDelay;								// The number of seconds in the level that must pass before the Alloy no damage message can be played.

	CBotCorrosive* m_pBotCorrosive;								// Pointer to corrosive
	CBotGlitch* m_pBotGlitch;									// Pointer to Glitch
	CEntity* m_pConsole;										// Pointer to the transmit/recieve console.
	CEntity* m_pCheckpointTripwire;								// Pointer to the tripwire which will trigger a checkpoint save
	CEntity* m_pLift;											// Pointer to the lift
	CEntity* m_apChipCrates[ 4 ];								// Pointer to the four crates that contain the chips

	static FSndFx_FxHandle_t m_hSounds[ MGSOUND_COUNT ];		// Array of sound handles that need to be played 
	u32					  m_nNumTrenchTripwireInfos;	

	_TrenchTripwireInfo_t *m_paTrenchTripwireInfo;				// Array count == m_nNumTrenchTripwireInfos
	_TrenchTripwireInfo_t *m_pTrenchTripwireInfoGlitchIsInside;	// Stores off the pointer to the tripwire info structure that Glitch is currently inside

	u32 m_nNumBridgeTripwireInfos;
	_BridgeTripwireInfo_t *m_paBridgeTripwireInfo;				// Array count == m_nNumBridgeTripwireInfos

	CFVec3A m_vPeerLocation;

	BOOL m_nChipsCollected;										// The number of chips collected by Glitch
	BOOL m_bGlitchInPossessionOfCorrosive;						// Is glitch currently in possession of corrosive
	u32 m_nNumTimesCorrosiveHasBeenHit;							// The number of times the Corrosive Damage Callback has been invoked.

	CFCheckPoint::ObjectDataHandle_t m_hSaveData[ FCHECKPOINT_MAX_CHECKPOINTS ]; // The checkpoints to save	

//----------------------------------------------------------------------------------------------------------------------------------
// Public Functions:
//----------------------------------------------------------------------------------------------------------------------------------
public:
	static BOOL LoadLevel( LevelEvent_e eEvent );
	static void UnloadLevel( void );
	static void Save( s32 nCheckpointID );
	static void Restore( s32 nCheckpointID );

	static FINLINE void Work( void )	{ FASSERT( m_pGeneralCorrosiveGame->IsCreated() ); m_pGeneralCorrosiveGame->_Work(); };
	static FINLINE BOOL IsCreated( void ) { return (m_pGeneralCorrosiveGame && m_pGeneralCorrosiveGame->m_bCreated ); }
	

//----------------------------------------------------------------------------------------------------------------------------------
// Protected Functions:
//----------------------------------------------------------------------------------------------------------------------------------
protected:


//----------------------------------------------------------------------------------------------------------------------------------
// Private Functions:
//----------------------------------------------------------------------------------------------------------------------------------
private:
	CGeneralCorrosiveGame();
//	~CGeneralCorrosiveGame();	

	//////////////////////////////
	// Initialization Functions //
	//////////////////////////////
	void _ClearDataMembers( void );
	BOOL _Create( void );
	BOOL _InitFromCSV( void );
	BOOL _LoadMiscGameplayParams( FGameDataFileHandle_t hFile );
	BOOL _PlaceGoodiesInCrateGroups( FGameDataFileHandle_t hFile, cchar *pszCrateTableName, cchar *pszGoodieTableName );

	// Tripwire CSV init functions
	BOOL _LoadTrenchTripwireRegions( FGameDataFileHandle_t hFile );
	BOOL _ParseTrenchTripwireTable( FGameDataFileHandle_t hFile, cchar *pszTripwireTableName, _TrenchTripwireInfo_t *pTrenchTripwireInfo );
	BOOL _LoadBridgeTripwireRegions( FGameDataFileHandle_t hFile );
	BOOL _ParseBridgeTripwireTable( FGameDataFileHandle_t hFile, cchar *pszTripwireTableName, _BridgeTripwireInfo_t *pBridgeTripwireInfo );

	// Event functions
	BOOL _RegisterForScriptEvents( void );

	///////////////
	// Callbacks //
	///////////////
	static void _TripwireEventCallback(s32 nWhichEvent, u32 uUserData, u32 uEventData1, u32 uEventData2, u32 uEventData3);
	static void _PossessEventCallback(s32 nWhichEvent, u32 uUserData, u32 uEventData1, u32 uEventData2, u32 uEventData3);
	static void _SwitchEventCallback(s32 nWhichEvent, u32 uUserData, u32 uEventData1, u32 uEventData2, u32 uEventData3);
	static void _DestructEventCallback(s32 nWhichEvent, u32 uUserData, u32 uEventData1, u32 uEventData2, u32 uEventData3);
	BOOL _GlitchHintLocEvaluateTripwireCallback( CEntity* pTripwireEntity, CEntity* pTripperEntity, u32 uEventData1 );
	BOOL _CorrosiveAttackBridgeEvaluateTripwireCallback( CEntity* pTripwireEntity, CEntity* pTripperEntity, u32 uEventData1 );
	BOOL _CommandPeerIfGlitchInTrenches( void ); 

	static void _CollectableCallback( CollectableType_e eType, CBot *pPickupBot );
	static BOOL _CorrosiveBehaviorRequestCallback( AIBehaviorReason_e eReason );
	static void _CorrosiveDamageCallback( void );

	// Cleanup Functions
	void _Destroy( void );

	// Runtime Functions
	void _Work( void );

	
#if SAS_ACTIVE_USER == SAS_USER_RUSS
	void _DebugFn( void );
#endif

	
	FCLASS_STACKMEM_ALIGN(CGeneralCorrosiveGame);
} FCLASS_ALIGN_SUFFIX;

// =============================================================================================================

#endif