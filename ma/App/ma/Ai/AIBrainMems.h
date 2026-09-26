#ifndef _AIBRAINMEMS_H
#define _AIBRAINMEMS_H

//memory Id's
enum
{
	MEMORY_SMALL_PARAMPACK = 0,		// Param packs are stored in memory so that thoughts can refer to them when they get initialized
	MEMORY_SMALL_GREETEDPLAYER,		// this brain has greeted the player
	MEMORY_SMALL_PLAYERPROMPTED,	// Player has prompted communication.
	MEMORY_SMALL_GRENADETOSS,		
	NUM_MEMORY_SMALL_IDS,
	MEMORY_MEDIUM_PARAMPACK = NUM_MEMORY_SMALL_IDS,		// Param packs are stored in memory so that thoughts can refer to them when they get initialized
	MEMORY_MEDIUM_VISCHECK,								// Used when a mult-frame visibility check is in progress on an entity
	MEMORY_MEDIUM_THOUGHTENDED,
	MEMORY_MEDIUM_THOUGHTBEGAN,	
	MEMORY_MEDIUM_SPHEREMARKER,							// marking locations
	MEMORY_MEDIUM_PROBEATTACKDATA,						// little memory used by the probe when it is attacking
	MEMORY_MEDIUM_ELITEGUARDATTACKDATA,					// little memory used by the EG when it is attacking
	MEMORY_MEDIUM_ZOMBIEBOSSATTACKDATA,					// little memory used by the ZB when it is attacking
	MEMORY_MEDIUM_JUMPERCOMBATDATA,						// little memory used by the J when it is attacking
	MEMORY_MEDIUM_JUMPERCOMBATCONFIG,					// little memory used by designers to configure the j
	MEMORY_MEDIUM_TEAMGRENADE,
	MEMORY_MEDIUM_NUM_GRENADES,
	MEMORY_MEDIUM_PREDFIRELOGIC,		
	MEMORY_FAILED_STRAFE,
	NUM_MEMORY_MEDIUM_IDS,
	MEMORY_LARGE_PARAMPACK = NUM_MEMORY_MEDIUM_IDS,		// Param packs are stored in memory so that thoughts can refer to them when they get initialized
	MEMORY_LARGE_ATTACKSPECS,							// Param pack for the Attack Order, should it be triggered by the CAIBrain::DoReactions
	MEMORY_LARGE_SEEN,									// An Entity was seen
	MEMORY_LARGE_SEEN_PROJECTILE,						// A projectile was seen
	MEMORY_LARGE_HEARD,									// Heard
	MEMORY_LARGE_DAMAGE_DEALT,							// Damaged was caused TO some other entity
	MEMORY_LARGE_DAMAGED_BY,							// Damaged was caused BY some entity
	MEMORY_LARGE_REPORTED,								// An Entity was Reported to be somewhere
	MEMORY_LARGE_COLLIDED,								// An Entity was run into	
	MEMORY_LARGE_RAT0_RULESET_DATA,
	MEMORY_LARGE_JUMPERMELEECOMBATDATA,					// little memory used by the J when it is in melee strategy
	MEMORY_LARGE_STRAFERUNDATA,							// little memory used by the snqarq when it is doing a strafing run
	MEMORY_LARGE_WAITORIGIN,							// first time this bot was on wait, what was the location and orientation
	MEMORY_LARGE_PROBESTRIKE,							// A probe is striking
	NUM_MEMORY_LARGE_IDS
};


struct BrainMemDef
{
	cchar* pszLabel;
	BOOL8  bRequireSaveRestoreWithBrain;
};

//
//	Class CAIMemorySmall
//

#define SAVE TRUE
#define NOSAVE FALSE

extern BrainMemDef AIBrainMem_aMemDefs[];
void AIBrainMems_Init(void);


#endif//#define _AIBRAINMEMS_H