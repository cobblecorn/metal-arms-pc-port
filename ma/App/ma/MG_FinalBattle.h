//////////////////////////////////////////////////////////////////////////////////////
// MG_FinalBattle.h
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
// 05/09/03 Foushee		Created.
//////////////////////////////////////////////////////////////////////////////////////

#ifndef _MG_FINALBATTLE_H_
#define _MG_FINALBATTLE_H_ 1


// =============================================================================================================

#include "fang.h"
#include "level.h"
#include "ai\\aicorrosive.h"
#include "BotCorrosive.h"
#include "BotGlitch.h"

#include "sas_user.h"

FCLASS_ALIGN_PREFIX class CMGFinalBattle {
//----------------------------------------------------------------------------------------------------------------------------------
// Public Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

//----------------------------------------------------------------------------------------------------------------------------------
// Protected Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
protected:


//----------------------------------------------------------------------------------------------------------------------------------
// Private Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
private:

//----------------------------------------------------------------------------------------------------------------------------------
// Public Data:
//----------------------------------------------------------------------------------------------------------------------------------
public:


//----------------------------------------------------------------------------------------------------------------------------------
// Private Data:
//----------------------------------------------------------------------------------------------------------------------------------
private:
	static CMGFinalBattle*	m_pMGFinalBattle;					// the single instance of this class
	
	BOOL m_bCreated;

	// General Gameplay Parameter factors
	f32 m_fMinAggressionLevel;									// The baseline aggression level corrosive starts at
	f32 m_fMaxAggressionLevel;									// The maximum aggression level corrosive can achieve
	f32 m_fAggressionPerSecondIncrementFactor;					// How fast his aggression level increments on a per second basis
	f32 m_fDamageTakenAggressionSummer;							// value which gets multiplied by the normalized amount of damage taken and added to the baseline aggression
	f32 m_fBotTossLevelStartDelay;								// The amount of time to forbid corrosive from bot tossing at the start of the level
	f32 m_fBotTossDamageDelayOverride;							// If corrosive takes more than this amount of normalized damage during the BotTossLevelStartDelay, then he overrides the delay and starts tossin bots. 
	f32 m_fGlitchArmorModifier;									// The armor modifier to apply to glitch for this mini-game

	// Realtime Game variables
	f32 m_fAggressionFactor;									// Corrosives current aggression factor
	f32 m_fAggressionTimer;										// Timer used to compute the aggression levels of corrosive

	CBotCorrosive* m_pBotCorrosive;								// Pointer to corrosive
	CBotGlitch* m_pBotGlitch;									// Pointer to Glitchy Mike E's man... :)

//----------------------------------------------------------------------------------------------------------------------------------
// Public Functions:
//----------------------------------------------------------------------------------------------------------------------------------
public:
	static BOOL LoadLevel( LevelEvent_e eEvent );
	static void UnloadLevel( void );
	static void Save( s32 nCheckpointID );
	static void Restore( s32 nCheckpointID );

	static FINLINE void Work( void )	{ FASSERT( m_pMGFinalBattle->IsCreated() ); m_pMGFinalBattle->_Work(); };
	static FINLINE BOOL IsCreated( void ) { return (m_pMGFinalBattle && m_pMGFinalBattle->m_bCreated ); }
	

//----------------------------------------------------------------------------------------------------------------------------------
// Protected Functions:
//----------------------------------------------------------------------------------------------------------------------------------
protected:


//----------------------------------------------------------------------------------------------------------------------------------
// Private Functions:
//----------------------------------------------------------------------------------------------------------------------------------
private:
	CMGFinalBattle();
//	~CMGFinalBattle();	

	//////////////////////////////
	// Initialization Functions //
	//////////////////////////////
	void _ClearDataMembers( void );
	BOOL _Create( void );
	BOOL _InitFromCSV( void );
	BOOL _LoadMiscGameplayParams( FGameDataFileHandle_t hFile );

	///////////////
	// Callbacks //
	///////////////

	// Cleanup Functions
	void _Destroy( void );

	// Runtime Functions
	void _Work( void );

	
#if SAS_ACTIVE_USER == SAS_USER_RUSS
	void _DebugFn( void );
#endif

	
	FCLASS_STACKMEM_ALIGN(CMGFinalBattle);
} FCLASS_ALIGN_SUFFIX;

// =============================================================================================================

#endif