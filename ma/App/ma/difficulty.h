//////////////////////////////////////////////////////////////////////////////////////
// Difficulty.h - Difficulty module.
//
// Author: Steve Ranck     
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
// 06/02/03 Ranck       Created.
//////////////////////////////////////////////////////////////////////////////////////
#ifndef _DIFFICULTY_H_
#define _DIFFICULTY_H_ 1

#include "fang.h"
#include "gamesave.h"
#include "fgamedata.h"




//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CDifficulty - Difficulty class
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

FCLASS_NOALIGN_PREFIX class CDifficulty {
//----------------------------------------------------------------------------------------------------------------------------------
// Public Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	enum {
		DIFFICULTY_COUNT = GAMESAVE_DIFFICULTY_COUNT,	// Number of difficulty levels (0=easiest)
		DEFAULT_DIFFICULTY_LEVEL = 1,					// Default difficulty level
	};


	class CInfo {									// Values in this class are obtained from a CSV file:
	public:
		f32 m_fDeltaUnitSusceptabilityPlayer;		// The delta to apply to the player's hitpoint susceptability during damage result computation
		f32 m_fDeltaUnitSusceptabilityNPC;			// The delta to apply to the NPC's hitpoint susceptability during damage result computation
#if FANG_WINGC
		// Retail additions (Difficulty.csv has 5 fields per level; vocabulary from main.dol). Loaded,
		// not used yet. Retail reads them at: +8 compared against an object's f32 at +360
		// (0x800f63b4); +12 in a calculation at 0x8005ff18; +16 added to a float (0x801595d4,
		// 0x8015de20). Values Easy..Insane: 2/1.5/1/0.25, 120/60/40/20, 0/30/90/180.
		f32 m_fRetailField2;						// [0, 6]
		f32 m_fRetailField3;						// [0, 10000]
		f32 m_fRetailField4;						// [-10000, 10000]
#endif
	};




//----------------------------------------------------------------------------------------------------------------------------------
// Private Data:
//----------------------------------------------------------------------------------------------------------------------------------
private:

	static const FGameData_TableEntry_t m_aGameDataVocab[];
	static u32 m_nDifficultyLevel;						// 0-based, where 0 is easiest
	static CInfo m_aInfo[DIFFICULTY_COUNT];				// Array of damage data: one per damage level
	static CInfo *m_pInfo;								// Points to the current info entry

	#if !FANG_PRODUCTION_BUILD
		static cchar *m_apszDevModeEnglishName[DIFFICULTY_COUNT];
	#endif




//----------------------------------------------------------------------------------------------------------------------------------
// Public Functions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	static BOOL LoadData( void );
	static void Reset( void );

	static void SetLevel( u32 nDifficultyLevel );
	static FINLINE u32 GetLevel( void ) { return m_nDifficultyLevel; }
	static FINLINE const CInfo *GetInfo( void ) { return m_pInfo; }

	#if !FANG_PRODUCTION_BUILD
		static cchar *DevMode_GetEnglishNameForLevel( u32 nDifficultyLevel ) { FASSERT( nDifficultyLevel < DIFFICULTY_COUNT ); return m_apszDevModeEnglishName[nDifficultyLevel]; }
	#else
		static cchar *DevMode_GetEnglishNameForLevel( u32 nDifficultyLevel ) { return ""; }
	#endif




//----------------------------------------------------------------------------------------------------------------------------------
// Private Functions:
//----------------------------------------------------------------------------------------------------------------------------------
private:

	static void _InitAllDefaultInfoData( void );
	static void _InitDefaultInfoData( CInfo *pInfo );


	FCLASS_STACKMEM_NOALIGN( CDifficulty );
} FCLASS_NOALIGN_SUFFIX;


#endif

