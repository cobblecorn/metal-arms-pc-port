#include "fang.h"
#include "AIBrainMems.h"
#include "AIKnowledge.h"

BrainMemDef AIBrainMem_aMemDefs[] = 
{
	"S_PARAMPACK",			NOSAVE,
	"S_GREETEDPLAYER",		NOSAVE,
	"S_PLAYERPROMPTED",		NOSAVE,
	"S_GRENADETOSS",		NOSAVE,
	"M_PARAMPACK",			NOSAVE,
	"M_VISCHECK",			NOSAVE,
	"M_THOUGHTENDED",	    NOSAVE,
	"M_THOUGHTBEGAN",	    NOSAVE,
	"M_SPHEREMARKER",		NOSAVE,
	"M_PROBEATKDATA",		NOSAVE,
	"M_EGATTACKDATA",		NOSAVE,
	"M_ZBATTACKDATA",		NOSAVE,
	"M_JCOMBATDATA",		SAVE,
	"M_JCOMBATCONFIG",		NOSAVE,
	"M_TEAMGRENADE",		NOSAVE,
	"M_NUM_GRENADES",		SAVE,
	"M_PREDFIRELOGIC",		NOSAVE,
	"M_FAILED_STRAFE",		NOSAVE,
	"L_PARAMPACK",			NOSAVE,
	"L_ATTACKSPECS",		NOSAVE,
	"L_SEEN",			    NOSAVE,
	"L_SEEN_PRO",		    NOSAVE,
	"L_HEARD",				NOSAVE,
	"L_DAMAGE_DEALT",		NOSAVE,
	"L_DAMAGED_BY",			NOSAVE,
	"L_REPORTED",			NOSAVE,
	"L_COLLIDED",			NOSAVE,
	"L_RAT0_RULESET_DATA",  NOSAVE,
	"L_JMELEECDATA",	    NOSAVE,
	"L_STRAFERUN",			NOSAVE,
	"L_WAITORIGIN",			NOSAVE,
	"L_PROBESTRIKE",		NOSAVE,
};

void AIBrainMems_Init(void)
{
	FASSERT(NUM_MEMORY_LARGE_IDS == sizeof(AIBrainMem_aMemDefs) / sizeof(BrainMemDef));
}



u8 AIKnowledge_MemoryId_To_MemorySize(u8 uMemoryId)
{
	if (uMemoryId < NUM_MEMORY_SMALL_IDS)
	{
		return MEMORY_SIZE_SMALL;
	}
	else if (uMemoryId < NUM_MEMORY_MEDIUM_IDS)
	{
		return MEMORY_SIZE_MEDIUM;
	}
	else if (uMemoryId < NUM_MEMORY_LARGE_IDS)
	{
		return MEMORY_SIZE_LARGE;
	}
	FASSERT(0);
	return MEMORY_SIZE_SMALL;
}


u8 AIKnowledge_ShouldBrainSaveMemoryId(u8 uMemoryId)
{
	return	AIBrainMem_aMemDefs[uMemoryId].bRequireSaveRestoreWithBrain;
}
