//////////////////////////////////////////////////////////////////////////////////////
// gamesave.h - game load and save functionality.
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
// 09/28/02 MacDonald   Created.
//////////////////////////////////////////////////////////////////////////////////////

#ifndef _GAMESAVE_H_
#define _GAMESAVE_H_ 1

#include "fang.h"
#include "fres.h"
#include "ftext.h"
#include "fstorage.h"
#include "fAMem.h"
#include "iteminst.h"
#include "level.h"
#include "FCheckPoint.h"

class CESphere;

//----------------------------------------------------------------------------------------------------------------------------------
// master checkpoint save & restore functionality
//----------------------------------------------------------------------------------------------------------------------------------
BOOL checkpoint_InitSystem( void );
void checkpoint_UninitSystem( void );
BOOL checkpoint_LevelInit( void );
BOOL checkpoint_Save( s32 nCheckpoint, BOOL bPrintText, CESphere* pESphere = NULL );
BOOL checkpoint_Restore( s32 nCheckpoint, BOOL bPrintText );
void checkpoint_Work( void );
void checkpoint_Draw( void );

BOOL checkpoint_Saved( s32 nCheckpoint );
void checkpoint_SetUnsaved( s32 nCheckpoint );




//----------------------------------------------------------------------------------------------------------------------------------
// player profile class
//----------------------------------------------------------------------------------------------------------------------------------
#define PROFILE_NAME_MAX_LENGTH		( 12 )
#define PROFILE_SIGNATURE			( 0x55501234 )
#define PROFILE_VERSION_NUMBER		( 12 )


typedef struct {
	u32 nPrimaryWeaponCRC;		// set to 0 if not used
	u32 nSecondaryWeaponCRC;	// set to 0 if not used
} GameSave_QuickSelectAxis_t;

typedef enum {
	GAMESAVE_QUICK_SELECT_AXIS_UP = 0,
	GAMESAVE_QUICK_SELECT_AXIS_DOWN,
	GAMESAVE_QUICK_SELECT_AXIS_LEFT,
	GAMESAVE_QUICK_SELECT_AXIS_RIGHT,

	GAMESAVE_QUICK_SELECT_AXIS_COUNT
} GameSave_QuickSelectAxis_e;

typedef struct {
	u32 nNumValidAxis;
	GameSave_QuickSelectAxis_t aAxisInfo[GAMESAVE_QUICK_SELECT_AXIS_COUNT];
} GameSave_QuickSelectRestore_t;

// written out before the profile data to ensure that we read the proper data
typedef struct {
	u32 nSignature;		// a unique tag that verfies that we have the correct struct
	u32 nFileVersion;	// what version is this file
	u32 nProfileCRC;	// a CRC of the rest of the data in the profile
	u32 nProfileBytes;	// the size of the GameSave_ProfileData_t struct
} GameSave_ProfileHeader_t;

// this structure is saved in the profile. It holds a players stats about how they have done on a level
typedef struct {
	f32 fTimeToComplete;		// how many seconds did it take to complete
	u16 nSecretChipsCollected;	// how many secret chips did the player collect on this level
	u16 nBonusSecretChipEarned;	// how many bonus secret chips were earned?
	u16 nWashersCollected;		// how many washers did the player collect on this level	
	u16 nEnemiesKilled;			// how many enemies did the player kill on this level
	CMemCardInventory Inventory;// what weapons and items do you have 
} GameSave_LevelProgress_t;

enum {
	GAMESAVE_RULE_FLAGS_RADAR_ON					= 0x00000001,
	GAMESAVE_RULE_FLAGS_MARKERS_ON					= 0x00000002,
	GAMESAVE_RULE_FLAGS_FRIENDLY_FIRE_DAMAGE		= 0x00000004,
	GAMESAVE_RULE_FLAGS_BOTS_ON_WHEN_NOT_TETHERED	= 0x00000008,
	GAMESAVE_RULE_FLAGS_POSSESSABLE_BOTS			= 0x00000010,

	GAMESAVE_RULE_FLAGS_NONE						= 0
};

typedef enum {
	GAMESAVE_PRIMARY_WEAPON_LIMIT_NO_LIMIT = 0,
	GAMESAVE_PRIMARY_WEAPON_LIMIT_ROCKETS_ONLY,
	GAMESAVE_PRIMARY_WEAPON_LIMIT_LASER_ONLY,
	GAMESAVE_PRIMARY_WEAPON_LIMIT_RIVET_ONLY,
	GAMESAVE_PRIMARY_WEAPON_LIMIT_TOASTER_ONLY,
	GAMESAVE_PRIMARY_WEAPON_LIMIT_RIPPER_ONLY,
	GAMESAVE_PRIMARY_WEAPON_LIMIT_SPEW_ONLY,
	GAMESAVE_PRIMARY_WEAPON_LIMIT_SCATTER_BLASTER_ONLY,
	GAMESAVE_PRIMARY_WEAPON_LIMIT_SLINGSHOT_ONLY,
	GAMESAVE_PRIMARY_WEAPON_LIMIT_NO_WEAPONS,

	GAMESAVE_PRIMARY_WEAPON_LIMIT_COUNT
} GameSave_PrimaryWeaponLimit_e;

typedef enum {
	GAMESAVE_SECONDARY_WEAPON_LIMIT_NO_LIMIT = 0,
	GAMESAVE_SECONDARY_WEAPON_LIMIT_CORING_CHARGE_ONLY,
	GAMESAVE_SECONDARY_WEAPON_LIMIT_MAGMA_BOMB_ONLY,
	GAMESAVE_SECONDARY_WEAPON_LIMIT_EMP_ONLY,
	GAMESAVE_SECONDARY_WEAPON_LIMIT_SCOPE_ONLY,
	GAMESAVE_SECONDARY_WEAPON_LIMIT_CLEANER_ONLY,
	GAMESAVE_SECONDARY_WEAPON_LIMIT_RECRUITER_ONLY,
	GAMESAVE_SECONDARY_WEAPON_LIMIT_NO_WEAPONS,

	GAMESAVE_SECONDARY_WEAPON_LIMIT_COUNT
} GameSave_SecondaryWeaponLimit_e;

typedef enum {
	GAMESAVE_VEHICLE_LIMIT_ALL_VEHICLES = 0,
	GAMESAVE_VEHICLE_LIMIT_LOADER_ONLY,
	GAMESAVE_VEHICLE_LIMIT_SENTINEL_ONLY,
	GAMESAVE_VEHICLE_LIMIT_RAT_ONLY,
	GAMESAVE_VEHICLE_LIMIT_NO_VEHICLES,

	GAMESAVE_VEHICLE_LIMIT_COUNT
} GameSave_AllowedVehicles_e;

// this structure is saved in the profile. It hold a players multiplayer rules settings.
// The name length should be long enough to hold any of the derived game type names
#define META_GAME_NAME_LEN	17

typedef struct {
	u16 nFlags;								// see GAMESAVE_RULE_FLAGS_...
	wchar wszGameName[META_GAME_NAME_LEN];	// Name of this meta game type
	
	s8 nGameType;							// Base game type, see Game_Multiplayer_Game_Types_e
	s8 nDMKillsPerRound;
	s8 nTimeLimit;							// Game type-dependent time limit
	s8 nLimitPrimaryWeapon;					// see GameSave_PrimaryWeaponLimit_e

	s8 nLimitSecondaryWeapon;				// see GameSave_SecondaryWeaponLimit_e	
	s8 nLimitVehicles;						// see GameSave_AllowedVehicles_e
	s8 nTimeBetweenHillSwaps;

	u16 ___nPadding;// available for use	
} GameSave_MPRules_t;

typedef enum {
	GAMESAVE_MP_COLORS_YELLOW = 0,
	GAMESAVE_MP_COLORS_BLUE,
	GAMESAVE_MP_COLORS_PURPLE,
	GAMESAVE_MP_COLORS_RED,
	GAMESAVE_MP_COLORS_GREEN,
	GAMESAVE_MP_COLORS_BLACK,

	GAMESAVE_MP_COLORS_COUNT
} GameSave_MPColors_e;

enum {
	GAMESAVE_PROFILE_FLAGS_VISITED_BARTER_DRIODS	= 0x00000001,
	GAMESAVE_PROFILE_FLAGS_VIRTUAL_PROFILE			= 0x00000002,// profile is not real, it should never be saved to disk
	GAMESAVE_PROFILE_FLAGS_FINISHED_SINGLE_PLAYER	= 0x00000004,
	GAMESAVE_PROFILE_FLAGS_100_PERCENT_FINISHED		= 0x00000008,
	GAMESAVE_PROFILE_FLAGS_INVERT_ANALOG			= 0x00000010,
	GAMESAVE_PROFILE_FLAGS_AUTO_CENTER				= 0x00000020,
	GAMESAVE_PROFILE_FLAGS_ASSISTED_TARGETING		= 0x00000040,
	GAMESAVE_PROFILE_FLAGS_FOUR_WAY_QUICK_SELECT	= 0x00000080,
    		
	GAMESAVE_PROFILE_FLAGS_NONE						= 0x00000000
};

typedef enum {
	GAMESAVE_DIFFICULTY_EASY = 0,
	GAMESAVE_DIFFICULTY_NORMAL,// default setting
	GAMESAVE_DIFFICULTY_HARD,
	GAMESAVE_DIFFICULTY_INSANE,
	// if you add to this list, you will need to update the wrapper screen

	GAMESAVE_DIFFICULTY_COUNT
} GameSave_Difficulty_e;

#define MP_RULE_SET_MAX 16		// Max number of custom multiplayer rule sets

typedef struct {
	//////////////////////////////////////////
	// THIS FIELD MUST BE FIRST IN THE PROFILE
	GameSave_ProfileHeader_t Header;
	///////////////////////////////////////////

	///////////////
	// general data
	u32 nFlags;					// see GAMESAVE_PROFILE_FLAGS_...
	u8 nDifficulty;// see GAMESAVE_DIFFICULTY_...
	u8 nCurrentLevel;			// what is the current level that the player has reached
	u8 nControllerConfigIndex;	// the index of the controller configuration
	u8 nColorIndex;				// see GAMESAVE_MP_COLORS_... for options
	s8 nMPRulesCount;			// How many sets of rules are defined in aMultiPlayerRules
	u8 nTotalSecretChipsGathered;	// how many secret chips has the player gathered (not reset if you restart your game)
	u8 nNumMPLevelsUnlocked;		// how many MP levels have been unlocked (not reset if you restart your game)
	
	u8 _____nPadding;// available for use

	f32 fUnitVibrationIntensity;// how much should the controller vibrate
	f32 fUnitLookSensitivity;	// how sensitive should the look controls be
	f32 fUnitSoundLevel;		// how loud should sound fxs be played
	f32 fUnitMusicLevel;		// how loud should music be played

	GameSave_QuickSelectRestore_t SinglePlayerQuickSelect;	// used to store the quick select settings for the single player game
	GameSave_QuickSelectRestore_t MultiPlayerQuickSelect;	// used to store the multiplayer quick select settings

	/////////////////////
	// single player data
	GameSave_LevelProgress_t aLevelProgress[LEVEL_SINGLE_PLAYER_COUNT];// a detail of where the player is in the levels 

	/////////////////////
	// multi player data
	GameSave_MPRules_t aMultiPlayerRules[MP_RULE_SET_MAX];

} GameSave_ProfileData_t;

enum {
	GAMESAVE_SAVE_INFO_FLAGS_NEEDS_SAVING	= 0x00000001,
	GAMESAVE_SAVE_INFO_FLAGS_CREATE_NEW		= 0x00000002,
	GAMESAVE_SAVE_INFO_FLAGS_RENAME			= 0x00000004,
	
	GAMESAVE_SAVE_INFO_FLAGS_NONE			= 0x00000000
};

// this structure records where to read/save the profile to
typedef struct {
	u32 nFlags;									// see _SAVE_INFO_FLAGS_		
	FStorage_DeviceID_e nStorageDeviceID;		// FSTORAGE_DEVICE_ID_NONE == no card was selected
	wchar wszCardName[FSTORAGE_MAX_NAME_LEN];	// Unicode.
	wchar wszProfileName[FSTORAGE_MAX_NAME_LEN];// Unicode.	
} GameSave_SaveInfo_t;

	

FCLASS_NOALIGN_PREFIX class CPlayerProfile
{
//----------------------------------------------------------------------------------------------------------------------------------
// Public Data:
//----------------------------------------------------------------------------------------------------------------------------------
public:
	GameSave_ProfileData_t	m_Data;	// this is what gets saved to disk
	GameSave_SaveInfo_t m_SaveInfo;	// this tells us where to save m_Data to, what memcard, what filename...
	s32 m_nControllerIndex;			// this tells us what controller this player is using

//----------------------------------------------------------------------------------------------------------------------------------
// Public Functions:
//----------------------------------------------------------------------------------------------------------------------------------
public:
	// inits the profile data to the defaults
	BOOL InitData();

	// loads the named profile into the object instance.
	BOOL LoadFromCard();			
	
	// checks the memcard specified in m_SaveInfo for various errors.
	BOOL IsDesiredCardReady( BOOL bCheckForProfiles, BOOL bCheckForNewProfileRoom );
	
	// saves current profile.
	BOOL SaveToCard();				
	
	// deletes the profile, but doesn't affect the data in memory.
	BOOL DeleteFromCard();

	// check to see whether this profile already exists on this card and whether it is ok
	BOOL IsOnCard( void );

	// this function will NOT read the profile from the card before deleting it.
	// this function will also save the current profile in memory to the new name and
	// update the SaveInfo with the new name.
	BOOL RenameProfile( cwchar *pwszNewProfileName );
	
	// inits a profile with all of the default values as if the user has just created a new profile
	void InitNewProfile( BOOL bVirtualProfile=FALSE );	

	// set the multiplayer rules to the default settings.
	static void SetMultiPlayerRulesToDefault( GameSave_MPRules_t *pRules, cwchar *pwszName = NULL );

	// resets a profile to the 1st level in the single player game, but
	// will not reset any setting changes in the profile
	void ResetToBeginning();

	// has this player visited the barter droids before?
	BOOL HasVisitedBarterDroidsBefore();

	// mark this player as having seen the barter droids before
	void VisitedBarterDroids();

	// is this profile virtual (meaning don't save to memory card)
	BOOL IsVirtual() const;

	// counts up the earned secret chips on the completed levels
	u32 CountEarnedSecretChips( void ) const;

	FCLASS_STACKMEM_NOALIGN( CPlayerProfile );	
} FCLASS_NOALIGN_SUFFIX;


BOOL playerprofile_InitSystem( void );
void playerprofile_UninitSystem( void );
void playerprofile_Set( s32 nPlayer, CPlayerProfile *pProfile );
CPlayerProfile *playerprofile_Get( s32 nPlayer );



#endif	//_GAMESAVE_H_
