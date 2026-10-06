//////////////////////////////////////////////////////////////////////////////////////
// AIBuilder.cpp - 
//
// Author: Pat MacKellar 
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
// 04/30/02 MacKellar   Created.
//////////////////////////////////////////////////////////////////////////////////////
#include "fang.h"
#include "fclib.h"
#include "AIBuilder.h"
#include "AIBrain.h"
#include "AIBrainMems.h"
#include "AIBrainutils.h"
#include "AIBrainman.h"
#include "AIPatrolPath.h"
#include "AiApi.h"
#include "AIMover.h"
#include "AIBotMover.h"
#include "../botgrunt.h"
#include "../bottitan.h"
#include "../builderhelp.h"
#include "../entity.h"
#include "../gstring.h"
#include "../player.h"
#include "../weapon.h"
#include "../MultiplayerMgr.h"

//====================
// private definitions
//=================
// public variables

//==================
// private variables

//===================
// private prototypes

//=================
// public functions


BuilderHelp_EnumTagSpec _aJobEnumTags[] = 
{
	"WAIT",	 CAIBrain::TT_WAIT,
	"WANDER", CAIBrain::TT_WANDER,
	"PATROL", CAIBrain::TT_PATROL,
	BUILDER_ERROR_STRING("CAIBuilder::InterpretTable() : Invalid value '%s' for 'AI_JOB' table. Use (WAIT, WANDER, PATROL).\n"), 0
};

BuilderHelp_EnumTagSpec _aBrainTypeTags[] = 
{
	"AUTO", -1,
	"GENERICBIPED", BRAINTYPE_GENERICBIPED,
	"GENERICTREADED", BRAINTYPE_GENERICTREADED,
	"GENERICHOVERCRAFT", BRAINTYPE_GENERICHOVERCRAFT,
	"BRAINTYPE_PILL", BRAINTYPE_PILL,
	"BRAINTYPE_SCOUT", BRAINTYPE_SCOUT,

	BUILDER_ERROR_STRING("CAIBuilder::InterpretTable() : Invalid value '%s' for 'AI_BRAINTYPE' table. Use (GENERICBIPED, GENERICTREADED, GENERICHOVERCRAFT).\n"), 0
};

BuilderHelp_EnumTagSpec _aMoverTypeTags[] = 
{
	"AUTO", -1,
	"XZ", MOVERCLASS_XZ,
	"3D", MOVERCLASS_3D,
	"CAR", MOVERCLASS_CAR,
	"XZ_HOVER",	MOVERCLASS_XZANDHOVER,
	BUILDER_ERROR_STRING("CAIBuilder::InterpretTable() : Invalid value '%s' for 'AI_MOVERTYPE' table. Use (AUTO, GROUND_MOVER, HOVER_MOVER.\n"), 0
};

BuilderHelp_EnumTagSpec _aRaceTags[] = 
{
	"MIL", AIRACE_MIL,
	"DROID", AIRACE_DROID,
	"ZOMBIE", AIRACE_ZOMBIE,
	"AMBIENT", AIRACE_AMBIENT,
	BUILDER_ERROR_STRING("CAIBuilder::InterpretTable() : Invalid value '%s' for 'AI_RACE' table. Use (MIL, DROID, ZOMBIE, AMBIENT).\n"), 0
};

BuilderHelp_EnumTagSpec _aPatrolDirTags[] = 
{
	"FORWARD", 0,
	"REVERSE", 1,
	BUILDER_ERROR_STRING("CAIBuilder::InterpretTable() : Invalid value '%s' for 'AI_PATROLDIR' table. Use (FORWARD or REVERSE).\n"), 0
};

BuilderHelp_EnumTagSpec _aBrainFlagTags[] = 
{
	"ENEMY",	CAIBrain::BRAINFLAG_ATTACKS_ENEMY,
	"NEUTRAL",	CAIBrain::BRAINFLAG_ATTACKS_NEUTRAL,
	"FRIENDLY", CAIBrain::BRAINFLAG_ATTACKS_FRIENDLY,
	"PLAYERS",	CAIBrain::BRAINFLAG_ATTACKS_PLAYERS,
	"NPC",		CAIBrain::BRAINFLAG_ATTACKS_NPC,
	BUILDER_ERROR_STRING("CAIBuilder::InterpretTable() : Invalid value '%s' for 'AI_ATTACKWHO' table. Use at least one of (ENEMY, NEUTRAL, FRIENDLY) and at least one of (PLAYERS, NPC).\n"), 0
};

BuilderHelp_EnumTagSpec _aMechUseageCtrlTags[] = 
{
	"NEVER", CAIBrain::MECH_USEAGE_CTRL_NEVER,
	"AUTO", CAIBrain::MECH_USEAGE_CTRL_AUTO,
	"INIT", CAIBrain::MECH_USEAGE_CTRL_INIT,
	BUILDER_ERROR_STRING("CAIBuilder::InterpretTable() : Invalid value '%s' for 'AI_USEVEHICLEWHEN' table. Use (NEVER or AUTO or INIT).\n"), 0
};

BuilderHelp_EnumTagSpec _aMechUseageTypeTags[] = 
{
	"NONE", CAIBrain::MECH_USEAGE_TYPE_FLAG_NONE,
	"PILL", CAIBrain::MECH_USEAGE_TYPE_FLAG_PILL,
	"LOADER", CAIBrain::MECH_USEAGE_TYPE_FLAG_LOADER,
	"SENTINEL", CAIBrain::MECH_USEAGE_TYPE_FLAG_SENTINEL,
	"RAT", CAIBrain::MECH_USEAGE_TYPE_FLAG_RAT,
	"RATGUN", CAIBrain::MECH_USEAGE_TYPE_FLAG_RATGUN,
	BUILDER_ERROR_STRING("CAIBuilder::InterpretTable() : Invalid value '%s' for 'AI_USEVEHICLETYPES' table. Use (NONE or PILL or LOADER or SENTINEL or RAT or RATGUN).\n"), 0
};

BuilderHelp_U32MinMaxSpec _GreetingLevelMMS = 
{
	0, 100, TRUE,
};


BuilderHelp_EnumTagSpec _aBuddyCtrlTags[] = 
{
	"NEVER",	CAIBrain::BRAINFLAG_NONE,
	"AUTO",		CAIBrain::BRAINFLAG_BUDDY_CTRL_AUTO,
	"ONACTION", CAIBrain::BRAINFLAG_BUDDY_CTRL_ONACTION,
	BUILDER_ERROR_STRING("CAIBuilder::InterpretTable() : Invalid value '%s' for 'AI_BUDDY_CTRL' table. Use (NEVER or AUTO or ONACTION).\n"), 0
};


BuilderHelp_EnumTagSpec _aGrenadeCtrlTags[] = 
{
	"NEVER",					AI_GRENADE_USE_NEVER,				
	"ENEMYINSIGHT",				AI_GRENADE_USE_ENEMYINSIGHT,
	"SELF_DEFENSE",				AI_GRENADE_USE_SELF_DEFENSE,
	"FIRST_CONTACT",			AI_GRENADE_USE_FIRST_CONTACT,
	"ENEMY_EVASIVE",			AI_GRENADE_USE_ENEMY_EVASIVE,
	"RETURN_GRENADE_FIRE",		AI_GRENADE_USE_RETURN_GRENADE_FIRE,	
	"WHENEVER",					AI_GRENADE_USE_WHENEVER,
	"SMART",					AI_GRENADE_USE_SMART,

	BUILDER_ERROR_STRING("CAIBuilder::InterpretTable() : Invalid value '%s' for 'AI_GRENADE_USE' table. Use NEVER or one or more of (ENEMYINSIGHT, SELF_DEFENSE, FIRST_CONTACT, ENEMY_EVASIVE, RETURN_GRENADE_FIRE, WHENEVER, SMART).\n"), 0
};
	

BuilderHelp_EnumTagSpec _aARSNames[] =
{
	"GENERIC0",		 CAIBrain::ATTACKRULESET_0,	
	"GENERIC1",		 CAIBrain::ATTACKRULESET_1,		
	"NPC",			 CAIBrain::ATTACKRULESET_NPC,		
	"TITAN0",		 CAIBrain::ATTACKRULESET_TITAN0,	
	"PRED0",		 CAIBrain::ATTACKRULESET_PRED0,	
	"PROBE0",		 CAIBrain::ATTACKRULESET_PROBE0,	
	"RAT0",			 CAIBrain::ATTACKRULESET_RAT0,		
	"ELITEGUARD0",	 CAIBrain::ATTACKRULESET_ELITEGUARD0,
	"ZOMBIE0",		 CAIBrain::ATTACKRULESET_ZOMBIE0,	
	"CORROSIVE0",	 CAIBrain::ATTACKRULESET_CORROSIVE0,
	"ZOMBIEBOSS0",	 CAIBrain::ATTACKRULESET_ZOMBIEBOSS0,
	"JUMPER0",		 CAIBrain::ATTACKRULESET_JUMPER0,
	"JUMPER1",		 CAIBrain::ATTACKRULESET_JUMPER1,
	"SCIENTIST0",	 CAIBrain::ATTACKRULESET_SCIENTIST0,
	"NONE",			 CAIBrain::ATTACKRULESET_NONE,
};


static InterpretTableDataSpec _aItableDataSpec[] = 
{
//	Tag;									Address of Vbl To Be set											DataType		uNumEnumTags in Spec												pMinMaxSpec							BuilderHelp_EnumTagSpec				Group Bool											internal use     internal use     
																																																																																				 
	"AI_JOB",									FANG_OFFSETOF(CAIBuilder, m_uInitialJob),							BDT_ENUM,		sizeof(_aJobEnumTags)/sizeof(BuilderHelp_EnumTagSpec),				NULL,									_aJobEnumTags,				NO_TRACKGROUPEDBOOLOFFSET,								NULL,		 	NULL,		
	"AI_AGGRESSION",							FANG_OFFSETOF(CAIBuilder, m_uAggression),							BDT_U32,		0,																	&BuilderHelp_ZeroToHunWithClamp,		NULL,						NO_TRACKGROUPEDBOOLOFFSET,								NULL,		 	NULL,		
	"AI_COURAGE",								FANG_OFFSETOF(CAIBuilder, m_uCourage),								BDT_U32,		0,																	&BuilderHelp_ZeroToHunWithClamp,		NULL,						NO_TRACKGROUPEDBOOLOFFSET,								NULL,		 	NULL,		
	"AI_INTELLIGENCE",							FANG_OFFSETOF(CAIBuilder, m_uIntelligence),							BDT_U32,		0,																	&BuilderHelp_ZeroToHunWithClamp, 		NULL,						NO_TRACKGROUPEDBOOLOFFSET,								NULL,		 	NULL,		
	"AI_BASESPEED",								FANG_OFFSETOF(CAIBuilder, m_uBaseSpeed),							BDT_U32,		0,																	&BuilderHelp_ZeroToHunWithClamp, 		NULL,						NO_TRACKGROUPEDBOOLOFFSET,								NULL,		 	NULL,		
	"AI_TORSOALIGNMENTSPEEDCONSTRAINT",			FANG_OFFSETOF(CAIBuilder, m_uTorsoAlignmentSpeedConstraint),		BDT_U32,		0,																	&BuilderHelp_ZeroToHunWithClamp, 		NULL,						NO_TRACKGROUPEDBOOLOFFSET,								NULL,		 	NULL,		
	"AI_TORSOALIGNMENTVELCONSTRAINT",			FANG_OFFSETOF(CAIBuilder, m_uTorsoAlignmentVelConstraint),			BDT_U32,		0,																	&BuilderHelp_ZeroToHunWithClamp, 		NULL,						NO_TRACKGROUPEDBOOLOFFSET,								NULL,		 	NULL,		
	"AI_TORSOALIGNMENTCONSTRAINTIGNORABLE",		FANG_OFFSETOF(CAIBuilder, m_bTorsoAlignmentConstraintIgnorable),	BDT_BOOL,		0,																	NULL,									NULL,						NO_TRACKGROUPEDBOOLOFFSET,								NULL,		 	NULL,		
	"AI_GREETINGLEVEL",							FANG_OFFSETOF(CAIBuilder, m_uInitialGreetingLevel),					BDT_U32,		0,																	&_GreetingLevelMMS,						NULL,						NO_TRACKGROUPEDBOOLOFFSET,								NULL,		 	NULL,		
	"AI_FOLLOWLEADER",							FANG_OFFSETOF(CAIBuilder, m_pszLeaderName),							BDT_STRING,		0,																	NULL,									NULL,						NO_TRACKGROUPEDBOOLOFFSET,								NULL,		 	NULL,		
	"AI_LOD_OVERRIDE_ALWAYS",					FANG_OFFSETOF(CAIBuilder, m_bLODOverrideAlways),					BDT_BOOL,		0,																	NULL,									NULL,						NO_TRACKGROUPEDBOOLOFFSET,								NULL,		 	NULL,		
	"AI_LOD_OVERRIDE_WHEN_CONVENIENT",			FANG_OFFSETOF(CAIBuilder, m_bLODOverrideWhenConvienient),			BDT_BOOL,		0,																	NULL,									NULL,						NO_TRACKGROUPEDBOOLOFFSET,								NULL,		 	NULL,		
	"AI_BRAINTYPE",								FANG_OFFSETOF(CAIBuilder, m_nBrainType),							BDT_ENUM,		sizeof(_aBrainTypeTags)/sizeof(BuilderHelp_EnumTagSpec),			NULL,									_aBrainTypeTags,			NO_TRACKGROUPEDBOOLOFFSET,								NULL,		 	NULL,		
	"AI_MOVERTYPE",								FANG_OFFSETOF(CAIBuilder, m_nMoverType),							BDT_ENUM,		sizeof(_aMoverTypeTags)/sizeof(BuilderHelp_EnumTagSpec),			NULL,									_aMoverTypeTags,			NO_TRACKGROUPEDBOOLOFFSET,								NULL,		 	NULL,		
	"AI_RACE",									FANG_OFFSETOF(CAIBuilder, m_uRace),									BDT_ENUM,		sizeof(_aRaceTags)/sizeof(BuilderHelp_EnumTagSpec),					NULL,									_aRaceTags,					NO_TRACKGROUPEDBOOLOFFSET,								NULL,		 	NULL,		
	"AI_JUMPDIST",								FANG_OFFSETOF(CAIBuilder, m_uJumpDist),								BDT_U32,		0,																	&BuilderHelp_U16,						NULL,						NO_TRACKGROUPEDBOOLOFFSET,								NULL,		 	NULL,		
																																																																																				 
	"AI_HEARINGSCALE",							FANG_OFFSETOF(CAIBuilder, m_fHearingMagUnalert),					BDT_F32,		0,																	&BuilderHelp_PosF32MMS,					NULL,						NO_TRACKGROUPEDBOOLOFFSET,								NULL,		 	NULL,		
	"AI_EYE_SCAN_DIST",							FANG_OFFSETOF(CAIBuilder, m_fEyeScanDistUnalert),					BDT_F32,		0,																	&BuilderHelp_PosF32MMS,					NULL,						NO_TRACKGROUPEDBOOLOFFSET,								NULL,		 	NULL,		
	"AI_BONUS_EYE_SCAN_DIST_FOR_PLAYERS",		FANG_OFFSETOF(CAIBuilder, m_fBonusEyeScanDistForPlayersUnalert),	BDT_F32,		0,																	&BuilderHelp_PosF32MMS,					NULL,						NO_TRACKGROUPEDBOOLOFFSET,								NULL,		 	NULL,		
	"AI_EYECOSHALFFOV",							FANG_OFFSETOF(CAIBuilder, m_fEyeCosHalfFOVUnalert),					BDT_F32,		0,																	&BuilderHelp_PosF32MMS,					NULL,						NO_TRACKGROUPEDBOOLOFFSET,								NULL,		 	NULL,		
	"AI_HEARINGSCALE_ALERT",					FANG_OFFSETOF(CAIBuilder, m_fHearingMagAlert),						BDT_F32,		0,																	&BuilderHelp_PosF32MMS,					NULL,						NO_TRACKGROUPEDBOOLOFFSET,								NULL,		 	NULL,		
	"AI_EYE_SCAN_DIST_ALERT",					FANG_OFFSETOF(CAIBuilder, m_fEyeScanDistAlert),						BDT_F32,		0,																	&BuilderHelp_PosF32MMS,					NULL,						NO_TRACKGROUPEDBOOLOFFSET,								NULL,		 	NULL,		
	"AI_BONUS_EYE_SCAN_DIST_FOR_PLAYERS_ALERT",	FANG_OFFSETOF(CAIBuilder, m_fBonusEyeScanDistForPlayersAlert),		BDT_F32,		0,																	&BuilderHelp_PosF32MMS,					NULL,						NO_TRACKGROUPEDBOOLOFFSET,								NULL,		 	NULL,		
	"AI_EYECOSHALFFOV_ALERT",					FANG_OFFSETOF(CAIBuilder, m_fEyeCosHalfFOVAlert),					BDT_F32,		0,																	&BuilderHelp_PosF32MMS,					NULL,						NO_TRACKGROUPEDBOOLOFFSET,								NULL,		 	NULL,		
	"AI_IGNOREPLAYERODDS_AT_SCANDIST",			FANG_OFFSETOF(CAIBuilder, m_uOddsOfIgnoringPlayerAtEyeScanDist),	BDT_U32,		0,																	&BuilderHelp_ZeroToHunWithClamp,		NULL,						NO_TRACKGROUPEDBOOLOFFSET,								NULL,		 	NULL,		
	"AI_INNER_RADIUS_FOR_IGNOREPLAYERODDS",		FANG_OFFSETOF(CAIBuilder, m_uInnerRadiusForIgnorePlayerOdds),		BDT_U32,		0,																	&BuilderHelp_ZeroToHunWithClamp,		NULL,						NO_TRACKGROUPEDBOOLOFFSET,								NULL,		 	NULL,		
	"AI_ODDSOF_IGNORING_NEARBY_GRENADES",		FANG_OFFSETOF(CAIBuilder, m_uOddsOfIgnoringNearbyGrenades),			BDT_U32,		0,																	&BuilderHelp_ZeroToHunWithClamp,		NULL,						NO_TRACKGROUPEDBOOLOFFSET,								NULL,		 	NULL,		
	"AI_ODDSOF_IGNORING_BOTFOOD_SOUNDS",		FANG_OFFSETOF(CAIBuilder, m_uOddsOfNotGoingToSeeSounds),			BDT_U32,		0,																	&BuilderHelp_ZeroToHunWithClamp,		NULL,						NO_TRACKGROUPEDBOOLOFFSET,								NULL,		 	NULL,		
	"AI_ATTACK_NOTIFY_RADIUS",					FANG_OFFSETOF(CAIBuilder, m_uAttackNotifyRadius),					BDT_U32,		0,																	&BuilderHelp_ZeroToHunWithClamp,		NULL,						NO_TRACKGROUPEDBOOLOFFSET,								NULL,		 	NULL,		
	"AI_WAITLOOKAT",							FANG_OFFSETOF(CAIBuilder, m_pszWaitLookAtObj),						BDT_STRING,		0,																	NULL,									NULL,						NO_TRACKGROUPEDBOOLOFFSET,								NULL,		 	NULL,		
	"AI_WAITLOOKAROUNDODDS",					FANG_OFFSETOF(CAIBuilder, m_uWaitScanOdds),							BDT_U32,		0,																	&BuilderHelp_ZeroToHunWithClamp,		NULL,						NO_TRACKGROUPEDBOOLOFFSET,								NULL,		 	NULL,		
	"AI_WANDERRADIUS",							FANG_OFFSETOF(CAIBuilder, m_fWanderRadius),							BDT_F32,		0,																	&BuilderHelp_PosF32MMS,					NULL,						NO_TRACKGROUPEDBOOLOFFSET,								NULL,		 	NULL,		
	"AI_WANDERSTAYINROOM",						FANG_OFFSETOF(CAIBuilder, m_bWanderStayInRoom),						BDT_BOOL,		0,																	NULL,									NULL,						NO_TRACKGROUPEDBOOLOFFSET,								NULL,		 	NULL,		
	"AI_WANDERSPEEDMIN",						FANG_OFFSETOF(CAIBuilder, m_uWanderSpeedMin),						BDT_U32,		0,																	&BuilderHelp_ZeroToHunWithClamp, 		NULL,						NO_TRACKGROUPEDBOOLOFFSET,								NULL,		 	NULL,		
	"AI_WANDERSPEEDMAX",						FANG_OFFSETOF(CAIBuilder, m_uWanderSpeedMax),						BDT_U32,		0,																	&BuilderHelp_ZeroToHunWithClamp, 		NULL,						NO_TRACKGROUPEDBOOLOFFSET,								NULL,		 	NULL,		
	"AI_WANDERSOCIABLE",						FANG_OFFSETOF(CAIBuilder, m_uWanderSociable),						BDT_U32,		0,																	&BuilderHelp_ZeroToHunWithClamp, 		NULL,						NO_TRACKGROUPEDBOOLOFFSET,								NULL,		 	NULL,		
	"AI_WANDERCURIOSITY",						FANG_OFFSETOF(CAIBuilder, m_uWanderCuriosity),						BDT_U32,		0,																	&BuilderHelp_ZeroToHunWithClamp, 		NULL,						NO_TRACKGROUPEDBOOLOFFSET,								NULL,		 	NULL,		
	"AI_WANDERCURIOSITY",						FANG_OFFSETOF(CAIBuilder, m_uWanderPauseOdds),						BDT_U32,		0,																	&BuilderHelp_ZeroToHunWithClamp, 		NULL,						NO_TRACKGROUPEDBOOLOFFSET,								NULL,		 	NULL,		
	"AI_WANDERPOINTS",							FANG_OFFSETOF(CAIBuilder, m_pszWanderPts),							BDT_STRING,		0,																	NULL,									NULL,						NO_TRACKGROUPEDBOOLOFFSET,								NULL,		 	NULL,		
	"AI_PATROLPATH",							FANG_OFFSETOF(CAIBuilder, m_pszPatrolPath),							BDT_STRING,		0,																	NULL,									NULL,						NO_TRACKGROUPEDBOOLOFFSET,								NULL,		 	NULL,		
	"AI_PATROLDIR",								FANG_OFFSETOF(CAIBuilder, m_bPatrolDirection),						BDT_ENUM,		sizeof(_aPatrolDirTags)/sizeof(BuilderHelp_EnumTagSpec),			NULL,									_aPatrolDirTags,			NO_TRACKGROUPEDBOOLOFFSET,								NULL,		 	NULL,		
	"AI_PATROLSPEED",							FANG_OFFSETOF(CAIBuilder, m_uPatrolSpeed),							BDT_U32,		0,																	&BuilderHelp_ZeroToHunWithClamp, 		NULL,						NO_TRACKGROUPEDBOOLOFFSET,								NULL,		 	NULL,		
																																																																																				 
	"AI_WEAPON1TIMEDBURSTS",					FANG_OFFSETOF(CAIBuilder, m_uTimedBursts),							BDT_BOOL,		0,																	NULL,									NULL,						NO_TRACKGROUPEDBOOLOFFSET,								NULL,		 	NULL,		
	"AI_WEAPON1BURSTDELAY_MIN",					FANG_OFFSETOF(CAIBuilder, m_fBurstDelay),							BDT_F32,		0,																	&BuilderHelp_PosF32MMS,					NULL,						NO_TRACKGROUPEDBOOLOFFSET,								NULL,		 	NULL,		
	"AI_WEAPON1BURSTDELAY_BONUS",				FANG_OFFSETOF(CAIBuilder, m_fBurstDelayBonus),						BDT_F32,		0,																	&BuilderHelp_PosF32MMS,					NULL,						NO_TRACKGROUPEDBOOLOFFSET,								NULL,		 	NULL,		
	"AI_WEAPON1BURSTCOUNT_MIN",					FANG_OFFSETOF(CAIBuilder, m_uNumFiresPerBurst),						BDT_U32,		0,																	&BuilderHelp_ZeroToHunWithClamp, 		NULL,						NO_TRACKGROUPEDBOOLOFFSET,								NULL,		 	NULL,		
	"AI_WEAPON1BURSTCOUNT_BONUS",				FANG_OFFSETOF(CAIBuilder, m_uNumFiresPerBurstBonus),				BDT_U32,		0,																	&BuilderHelp_ZeroToHunWithClamp, 		NULL,						NO_TRACKGROUPEDBOOLOFFSET,								NULL,		 	NULL,		
	"AI_WEAPON1FIREDELAY_MIN",					FANG_OFFSETOF(CAIBuilder, m_fFireDelay),							BDT_F32,		0,																	&BuilderHelp_PosF32MMS,					NULL,						NO_TRACKGROUPEDBOOLOFFSET,								NULL,		 	NULL,		
	"AI_WEAPON1FIREDELAY_BONUS",				FANG_OFFSETOF(CAIBuilder, m_fFireDelayBonus),						BDT_F32,		0,																	&BuilderHelp_PosF32MMS,					NULL,						NO_TRACKGROUPEDBOOLOFFSET,								NULL,		 	NULL,		
	"AI_WEAPON1BURST_MIN",						FANG_OFFSETOF(CAIBuilder, m_fFireDelay),							BDT_F32,		0,																	&BuilderHelp_PosF32MMS,					NULL,						NO_TRACKGROUPEDBOOLOFFSET,								NULL,		 	NULL,		
	"AI_WEAPON1BURST_BONUS",					FANG_OFFSETOF(CAIBuilder, m_fFireDelayBonus),						BDT_F32,		0,																	&BuilderHelp_PosF32MMS,					NULL,						NO_TRACKGROUPEDBOOLOFFSET,								NULL,		 	NULL,		
	"AI_WEAPON1ACCURACY",						FANG_OFFSETOF(CAIBuilder, m_fAccuracy),								BDT_F32,		0,																	&BuilderHelp_PosUnitF32MMS,				NULL,						NO_TRACKGROUPEDBOOLOFFSET,								NULL,		 	NULL,		
																																																																																				 
	"AI_WEAPON2TIMEDBURSTS",					FANG_OFFSETOF(CAIBuilder, m_uTimedBursts2),							BDT_BOOL,		0,																	NULL,									NULL,						NO_TRACKGROUPEDBOOLOFFSET,								NULL,		 	NULL,		
	"AI_WEAPON2BURSTDELAY_MIN",					FANG_OFFSETOF(CAIBuilder, m_fBurstDelay2),							BDT_F32,		0,																	&BuilderHelp_PosF32MMS,					NULL,						NO_TRACKGROUPEDBOOLOFFSET,								NULL,		 	NULL,		
	"AI_WEAPON2BURSTDELAY_BONUS",				FANG_OFFSETOF(CAIBuilder, m_fBurstDelayBonus2),						BDT_F32,		0,																	&BuilderHelp_PosF32MMS,					NULL,						NO_TRACKGROUPEDBOOLOFFSET,								NULL,		 	NULL,		
	"AI_WEAPON2BURSTCOUNT_MIN",					FANG_OFFSETOF(CAIBuilder, m_uNumFiresPerBurst2),					BDT_U32,		0,																	&BuilderHelp_U16, 						NULL,						NO_TRACKGROUPEDBOOLOFFSET,								NULL,		 	NULL,		
	"AI_WEAPON2BURSTCOUNT_BONUS",				FANG_OFFSETOF(CAIBuilder, m_uNumFiresPerBurstBonus2),				BDT_U32,		0,																	&BuilderHelp_ZeroToHunWithClamp, 		NULL,						NO_TRACKGROUPEDBOOLOFFSET,								NULL,		 	NULL,		
	"AI_WEAPON2FIREDELAY_MIN",					FANG_OFFSETOF(CAIBuilder, m_fFireDelay2),							BDT_F32,		0,																	&BuilderHelp_PosF32MMS,					NULL,						NO_TRACKGROUPEDBOOLOFFSET,								NULL,		 	NULL,		
	"AI_WEAPON2FIREDELAY_BONUS",				FANG_OFFSETOF(CAIBuilder, m_fFireDelayBonus2),						BDT_F32,		0,																	&BuilderHelp_PosF32MMS,					NULL,						NO_TRACKGROUPEDBOOLOFFSET,								NULL,		 	NULL,		
	"AI_WEAPON2BURST_MIN",						FANG_OFFSETOF(CAIBuilder, m_fFireDelay2),							BDT_F32,		0,																	&BuilderHelp_PosF32MMS,					NULL,						NO_TRACKGROUPEDBOOLOFFSET,								NULL,		 	NULL,		
	"AI_WEAPON2BURST_BONUS",					FANG_OFFSETOF(CAIBuilder, m_fFireDelayBonus2),						BDT_F32,		0,																	&BuilderHelp_PosF32MMS,					NULL,						NO_TRACKGROUPEDBOOLOFFSET,								NULL,		 	NULL,		
	"AI_WEAPON2ACCURACY",						FANG_OFFSETOF(CAIBuilder, m_fAccuracy2),							BDT_F32,		0,																	&BuilderHelp_PosUnitF32MMS,				NULL,						NO_TRACKGROUPEDBOOLOFFSET,								NULL,		 	NULL,		
																																																																																				 
	"AI_NUMGRENADESATSTART",					FANG_OFFSETOF(CAIBuilder, m_uNumGrenadesAtStart),					BDT_U32,		0,																	&BuilderHelp_ZeroToHunWithClamp, 		NULL,						NO_TRACKGROUPEDBOOLOFFSET,								NULL,		 	NULL,		
	"AI_GRENADE_USE",							FANG_OFFSETOF(CAIBuilder, m_uGrenadeUse),							BDT_ENUMS2BITS,	sizeof(_aGrenadeCtrlTags)/sizeof(BuilderHelp_EnumTagSpec),			NULL, 									_aGrenadeCtrlTags,			NO_TRACKGROUPEDBOOLOFFSET,								NULL,		 	NULL,		
	"AI_GRENADE_USE_ODDS",						FANG_OFFSETOF(CAIBuilder, m_uGrenadeOdds),							BDT_U32,		0,																	&BuilderHelp_ZeroToHunWithClamp,		NULL,						NO_TRACKGROUPEDBOOLOFFSET,								NULL,		 	NULL,		
																																																																																				 
	"AI_PREFERSTOPANDSHOOT",					FANG_OFFSETOF(CAIBuilder, m_uPreferStopAndShoot),					BDT_BOOL,		0,																	NULL,				 					NULL,						NO_TRACKGROUPEDBOOLOFFSET,								NULL,		 	NULL,		
	"AI_ATTACKRULESET",							FANG_OFFSETOF(CAIBuilder, m_uNormalRuleSet),						BDT_ENUM,		sizeof(_aARSNames)/sizeof(BuilderHelp_EnumTagSpec),					NULL,									_aARSNames,					FANG_OFFSETOF(CAIBuilder, m_bAttackParamsSpecified),	NULL,		 	NULL,		
	"AI_ALTATTACKRULESET",						FANG_OFFSETOF(CAIBuilder, m_uAlternateRuleSet),						BDT_ENUM,		sizeof(_aARSNames)/sizeof(BuilderHelp_EnumTagSpec),					NULL,									_aARSNames,					FANG_OFFSETOF(CAIBuilder, m_bAttackParamsSpecified),	NULL,		 	NULL,		
	"AI_ALTATTACKRULESETODDS",					FANG_OFFSETOF(CAIBuilder, m_uAlternateRuleSetOdds),					BDT_U32,		0,																	&BuilderHelp_ZeroToHunWithClamp,		NULL,						FANG_OFFSETOF(CAIBuilder, m_bAttackParamsSpecified),	NULL,		 	NULL,		
	"AI_ATTACKSPEEDMIN",						FANG_OFFSETOF(CAIBuilder, m_uAttackSpeedMin),						BDT_U32,		0,																	&BuilderHelp_ZeroToHunWithClamp, 		NULL,						FANG_OFFSETOF(CAIBuilder, m_bAttackParamsSpecified),	NULL,		 	NULL,		
	"AI_ATTACKSPEEDMAX",						FANG_OFFSETOF(CAIBuilder, m_uAttackSpeedMax),						BDT_U32,		0,																	&BuilderHelp_ZeroToHunWithClamp, 		NULL,						FANG_OFFSETOF(CAIBuilder, m_bAttackParamsSpecified),	NULL,		 	NULL,		
	"AI_ATTACKMELEERAD",						FANG_OFFSETOF(CAIBuilder, m_uAttackMeleeMinRad),					BDT_U32,		0,																	&BuilderHelp_U8,						NULL,						FANG_OFFSETOF(CAIBuilder, m_bAttackParamsSpecified),	NULL,		 	NULL,		
	"AI_STAYPUTDIST",							FANG_OFFSETOF(CAIBuilder, m_fStayPutDist),							BDT_F32,		0,																	&BuilderHelp_NegOneToPosUnitF32MMS, 	NULL,						FANG_OFFSETOF(CAIBuilder, m_bAttackParamsSpecified),	NULL,		 	NULL,		
	"AI_ATTACKMAXDISTFROMORIGIN",				FANG_OFFSETOF(CAIBuilder, m_fAttackMaxDistFromOrigin),				BDT_F32,		0,																	&BuilderHelp_NegOneToPosUnitF32MMS, 	NULL,						FANG_OFFSETOF(CAIBuilder, m_bAttackParamsSpecified),	NULL,		 	NULL,		
	"AI_STARTLEMIN",							FANG_OFFSETOF(CAIBuilder, m_uStartleMin),							BDT_U32,		0,																	&BuilderHelp_U8,						NULL,						NO_TRACKGROUPEDBOOLOFFSET,								NULL,		 	NULL,		
	"AI_STARTLEMAX",							FANG_OFFSETOF(CAIBuilder, m_uStartleMax),							BDT_U32,		0,																	&BuilderHelp_U8,						NULL,						NO_TRACKGROUPEDBOOLOFFSET,								NULL,		 	NULL,		
	"AI_ATTACKWHO",								FANG_OFFSETOF(CAIBuilder, m_uBrainFlags),							BDT_ENUMS2BITS,	sizeof(_aBrainFlagTags)/sizeof(BuilderHelp_EnumTagSpec),			NULL,									_aBrainFlagTags,			NO_TRACKGROUPEDBOOLOFFSET,								NULL,		 	NULL,		
	"AI_USEVEHICLERADIUS",						FANG_OFFSETOF(CAIBuilder, m_uMechUseageRadius),						BDT_U32,		0,																	&BuilderHelp_U8,						NULL,						NO_TRACKGROUPEDBOOLOFFSET,								NULL,		 	NULL,		
	"AI_USEVEHICLEWHEN",						FANG_OFFSETOF(CAIBuilder, m_uMechUseageCtrl),						BDT_ENUM,		sizeof(_aMechUseageCtrlTags)/sizeof(BuilderHelp_EnumTagSpec),		NULL,									_aMechUseageCtrlTags,		NO_TRACKGROUPEDBOOLOFFSET,								NULL,		 	NULL,		
	"AI_USEVEHICLETYPE",						FANG_OFFSETOF(CAIBuilder, m_uMechUseageTypeFlags),					BDT_ENUMS2BITS,	sizeof(_aMechUseageTypeTags)/sizeof(BuilderHelp_EnumTagSpec),		NULL,									_aMechUseageTypeTags,		NO_TRACKGROUPEDBOOLOFFSET,								NULL,		 	NULL,		
	"AI_NEVERUNARMED",							FANG_OFFSETOF(CAIBuilder, m_bNeverUnarmed),							BDT_BOOL,		NULL,																NULL,									NULL,						NO_TRACKGROUPEDBOOLOFFSET,								NULL,		 	NULL,		
	"AI_WAKEUP_MIN_RAD",						FANG_OFFSETOF(CAIBuilder, m_uWakeupRadius),							BDT_U32,		NULL,																&BuilderHelp_U8,						NULL,						NO_TRACKGROUPEDBOOLOFFSET,								NULL,		 	NULL,		
	"AI_COMM_GROUP",							FANG_OFFSETOF(CAIBuilder, m_pszStringTableCommGroupName),			BDT_STRING,		0,																	NULL,									NULL,						NO_TRACKGROUPEDBOOLOFFSET,								NULL,		 	NULL,		
	"AI_BUDDY_CTRL",							FANG_OFFSETOF(CAIBuilder, m_uBuddyCtrl),							BDT_ENUM,		sizeof(_aBuddyCtrlTags)/sizeof(BuilderHelp_EnumTagSpec),			NULL,									_aBuddyCtrlTags,			NO_TRACKGROUPEDBOOLOFFSET,								NULL,		 	NULL,		
};																																																																																	 
																																																																																	 

#define AUTO_BRAINTYPE -1
#define AUTO_MOVERTYPE -1
#define DEFAULT_RACE 0xffff

void CAIBuilder::SetDefaults( CEntityBuilder* pBuilder )
{
	// Note for Pat: Get parameter for "AI=" in pBuilder->m_pszAIBuilderName (will never be NULL).
	// Always check for "Default" which is what the builder system will pass if no "AI=" table was found on a bot.
	CAIBuilder* pAIBuilder = pBuilder->m_pAIBuilder;
	FASSERT(pAIBuilder);

	m_uInitialJob = CAIBrain::TT_WAIT; 
	m_uAggression = 50;
	m_uCourage = 50;
	m_uIntelligence = 50;
	m_uBaseSpeed = 50;								// (0-100)			How fast they will move (pct of what bot is capable of)
	m_nBrainType = AUTO_BRAINTYPE;
	m_nMoverType = AUTO_MOVERTYPE;
	m_uTorsoAlignmentSpeedConstraint = 25;			// (0-100) How much speed is limited by poor "forward" torso alignment
	m_uTorsoAlignmentVelConstraint = 35;			// (0-100) How much the direction of movement is limited by poor "forward" torso alignment
	m_bTorsoAlignmentConstraintIgnorable = TRUE;	// (T/F)	 Can m_fTorsoAlignmentSpeedConstraint be ignored when given conflicting FaceGoals and MoveGoals
	m_uRace = AIRACE_MIL;
	m_pszLeaderName = NULL;
	m_uInitialGreetingLevel = CAIBrain::GREETINGLEVEL_0;
	m_bLODOverrideAlways = FALSE;			   //if true, the brain will never go inactive due to volume LOD system
	m_bLODOverrideWhenConvienient = FALSE;	   //if true, the brain will go inactive due to volume LOD system, but occasionally wake up when convenient performance-wise
	m_uJumpDist = 50;

	//Brain Perception/Reaction
	m_fHearingMagUnalert = m_fHearingMagAlert = 1.0f;
	m_fEyeScanDistUnalert = m_fEyeScanDistAlert = 50.0f;
	m_fBonusEyeScanDistForPlayersUnalert = m_fBonusEyeScanDistForPlayersAlert = 100.0f;
	m_fEyeCosHalfFOVUnalert = m_fEyeCosHalfFOVAlert = 0.3f;		//  72 degree half angle, or 140 degree total

	m_uOddsOfNotGoingToSeeSounds = 50;
	m_uOddsOfIgnoringPlayerAtEyeScanDist = 50;	
	m_uInnerRadiusForIgnorePlayerOdds = 75;		
	m_uOddsOfIgnoringNearbyGrenades = 0;		
	m_uAttackNotifyRadius = 40;					
	m_uStartleMin = 10;							//Min time between startles
	m_uStartleMax = 30;							//Max time between startles
	m_uWakeupRadius = 0xff;

	//Generic Wait
	m_pszWaitLookAtObj = NULL;		//				The name of an entity to keep looking at (0 means ignore)
	m_uWaitScanOdds = 0;			// (0-100)		Odds that unit will look around every now and again.

	//Generic Wander
	m_fWanderRadius = 0.0f;			// (0-??)
	m_bWanderStayInRoom = TRUE;		// (T/F)
	m_uWanderSpeedMin = 0;			// (0-100)	//zero means use base speed setting
	m_uWanderSpeedMax = 0;			// (0-100)	//zero means use base speed setting 
	m_uWanderSociable = 0;			// (0-100)
	m_uWanderCuriosity = 0;			// (0-100)
	m_uWanderPauseOdds = 0;			// (0-100)
	m_pszWanderPts = NULL;

	//Generic Patrol
	m_pszPatrolPath = NULL;			// (T/F)
	m_bPatrolDirection = 0;			// 0=FORWARD, 1=REVERSE
	m_uPatrolSpeed = 0;				// (0-100)		How fast to move while on patrol 0 means use base speed
	m_uNumIgnorePatrolZones = 0;
	fang_MemZero(m_apszIgnorePatrolZone, sizeof(m_apszIgnorePatrolZone));

	//Attack Configuration
	m_bAttackParamsSpecified = FALSE;
	m_uNormalRuleSet = CAIBrain::ATTACKRULESET_0;
	m_uAlternateRuleSet = CAIBrain::ATTACKRULESET_NONE;
	m_uAlternateRuleSetOdds = 0; //(0-100) pct chance that the alternate will be used
	m_fAttackMaxDistFromOrigin = -1.0f;
	m_fStayPutDist = -1.0f;
	m_uAttackSpeedMin = 80;
	m_uAttackSpeedMax = 90;
	m_uAttackMeleeMinRad = 5;
	m_bNeverUnarmed = FALSE;

	m_uBrainFlags = CAIBrain::BRAINFLAG_ATTACKS_ENEMY | CAIBrain::BRAINFLAG_ATTACKS_PLAYERS | CAIBrain::BRAINFLAG_ATTACKS_NPC;
	m_uMechUseageCtrl = CAIBrain::MECH_USEAGE_CTRL_NEVER;
	m_uPillUseageCtrl = CAIBrain::MECH_USEAGE_CTRL_NEVER;
	m_uMechUseageRadius = 50;

	if( !MultiplayerMgr.IsSinglePlayer() )
	{
		//every bot will automatically buddy up with players that it isn't going to fight and is of the same race
		m_uBuddyCtrl = CAIBrain::BRAINFLAG_BUDDY_CTRL_AUTO;
	}
	else
	{
		m_uBuddyCtrl = 0;
	}

	//weapon ctrls and fire styles
	//can't set defaults until we know what weapon bot will have.
	m_fBurstDelay				= m_fBurstDelay2 = -1.0f;
	m_fBurstDelayBonus			= m_fBurstDelayBonus2= -1.0f;
	m_fFireDelay				= m_fFireDelay2	= -1.0f;
	m_fFireDelayBonus			= m_fFireDelayBonus2= -1.0f;
	m_uNumFiresPerBurst			= m_uNumFiresPerBurst2 = 0xffffffff;
	m_uNumFiresPerBurstBonus	= m_uNumFiresPerBurstBonus2 = 0xffffffff;
	m_fAccuracy					= m_fAccuracy2 = -1.0f;
	m_uPreferStopAndShoot		= m_uPreferStopAndShoot2 = 0xffffffff;
	m_uTimedBursts				= m_uTimedBursts2 = 0xffffffff;

	m_uMechUseageTypeFlags		= 0xffffffff;
	
	m_uNumGrenadesAtStart = 0;
	m_uGrenadeUse = 0xffffffff;
	m_uGrenadeOdds = 0;


	m_uNumWeapons = 1;
	m_uMeleeRad = 5;
	m_uMeleeTimeMin = 1;	 
	m_uMeleeTimeMax = 5;
	m_pszStringTableCommGroupName = NULL;

	if ( pBuilder->m_nEC_LeafTypeBit & ENTITY_BIT_BOTGRUNT)
	{   //ai defaults for grunts
		m_uMechUseageRadius = 50;
		m_uMechUseageCtrl = CAIBrain::MECH_USEAGE_CTRL_AUTO;
		m_uOddsOfNotGoingToSeeSounds = 0;

		m_uNumGrenadesAtStart = 10;
//		m_uGrenadeUse = AI_GRENADE_USE_NEVER;  	//can't set defaults until we know what weapon bot will have.
		m_uGrenadeOdds = 50;

		//Can't set weapon defaults until we know what weapon the grunt will have.
	}
	else if ( pBuilder->m_nEC_LeafTypeBit & ENTITY_BIT_BOTELITEGUARD)
	{   
		m_bNeverUnarmed = TRUE;

		//Stun Gun
		m_uTimedBursts = FALSE;
		m_fBurstDelay = 1.0f;
		m_fBurstDelayBonus = 1.0f;
		m_fFireDelay = 0.7f;
		m_fFireDelayBonus = 1.4f;
		m_uPreferStopAndShoot = FALSE;
		m_uNormalRuleSet = CAIBrain::ATTACKRULESET_ELITEGUARD0;//CAIBrain::ATTACKRULESET_0;/

		m_uNumFiresPerBurst = 1;
		m_uNumFiresPerBurstBonus = 0;

		m_uMeleeRad = 12;
		m_uMeleeTimeMin = 0;	 
		m_uMeleeTimeMax = 3;
		
	}
	else if ( pBuilder->m_nEC_LeafTypeBit & ENTITY_BIT_BOTZOM)
	{   
		m_bNeverUnarmed = TRUE;
		m_uRace = AIRACE_ZOMBIE;
		m_uNormalRuleSet = CAIBrain::ATTACKRULESET_ZOMBIE0;
		m_uWakeupRadius = 35;
		m_uAttackMeleeMinRad =  25;

		m_fHearingMagUnalert = 6.0f;
		m_fHearingMagAlert = 6.0f;

		m_uBrainFlags |= CAIBrain::BRAINFLAG_ATTACKS_NPC;


		//Fire 1 is melee
		m_uTimedBursts = TRUE;
		m_fBurstDelay = 0.0f;
		m_fBurstDelayBonus = 1.0f;
		m_fFireDelay = 2.8f;
		m_fFireDelayBonus = 3.0f;
		m_uPreferStopAndShoot = FALSE;

		m_uMeleeRad = 15;
		m_uMeleeTimeMin = 0;	 
		m_uMeleeTimeMax = 3;

	}
	else if ( pBuilder->m_nEC_LeafTypeBit & ENTITY_BIT_BOTJUMPER)
	{   
		m_bNeverUnarmed = TRUE;
		m_uMechUseageCtrl = CAIBrain::MECH_USEAGE_CTRL_NEVER;

		m_uMeleeRad = 8;
		m_uMeleeTimeMin = 0;	 
		m_uMeleeTimeMax = 3;
		m_uInitialJob = CAIBrain::TT_WANDER;

		//Fire 1 is pulse cannon with spash damage
		m_uTimedBursts = TRUE;
		m_fBurstDelay = 0.5f;
		m_fBurstDelayBonus =  1.5f;
		m_fFireDelay = 0.29f;	//length of burst min
		m_fFireDelayBonus = 0.45f;
		m_fAccuracy= 0.5f;
		m_uJumpDist = 150;

		m_uNormalRuleSet = CAIBrain::ATTACKRULESET_JUMPER0;


		m_fEyeScanDistUnalert = 50;
		m_fBonusEyeScanDistForPlayersUnalert = 150.0f;

		m_fEyeScanDistAlert = 80;
		m_fBonusEyeScanDistForPlayersAlert = 250.0f;
	}
	else if ( pBuilder->m_nEC_LeafTypeBit & ENTITY_BIT_KRUNK)
	{ 
		m_uMechUseageCtrl = CAIBrain::MECH_USEAGE_CTRL_NEVER;
		m_uRace = AIRACE_DROID;
	}
	else if ( pBuilder->m_nEC_LeafTypeBit & ENTITY_BIT_VEHICLERAT)
	{
		m_uInitialJob = CAIBrain::TT_WAIT;
		m_uWaitScanOdds = 0;

		m_uNormalRuleSet = CAIBrain::ATTACKRULESET_RAT0;
		m_uBaseSpeed = 100;
		m_uAttackSpeedMin = 90;
		m_uAttackSpeedMax = 100;
		m_uMechUseageCtrl = CAIBrain::MECH_USEAGE_CTRL_NEVER;
		m_uRace = AIRACE_AMBIENT;	  //rat's take on the race of the driver, but are ambient until then
		//m_bLODOverrideAlways = TRUE;
	}
	else if ( pBuilder->m_nEC_LeafTypeBit & ENTITY_BIT_BOTSCOUT)
	{
		m_bNeverUnarmed = TRUE;

		// unalert eye settings
		m_fEyeCosHalfFOVUnalert = 0.86f;				// min dot product value to see target, roughly 30 degs half angle
		m_fEyeScanDistUnalert = 50;						// normal vision distance
		m_fBonusEyeScanDistForPlayersUnalert = 60.0f;	// bonus distance if to see player

		// alert eye settings
		m_fEyeCosHalfFOVAlert = 0.64f; //roughly 50 degs half angle
		m_fEyeScanDistAlert = 50;
		m_fBonusEyeScanDistForPlayersAlert = 75.0f;

		m_uBaseSpeed =50;
		m_uAttackSpeedMin = 90;
		m_uAttackSpeedMax = 100;
		m_uMechUseageCtrl = CAIBrain::MECH_USEAGE_CTRL_NEVER;

		m_uNumWeapons = 0;
	}
	else if ( pBuilder->m_nEC_LeafTypeBit & ENTITY_BIT_VEHICLESENTINEL)
	{
		m_uInitialJob = CAIBrain::TT_WAIT;
		m_uWaitScanOdds = 0;
		m_uBaseSpeed =100;

		m_fBonusEyeScanDistForPlayersAlert = 250.0f;

		//cannon
		m_uTimedBursts = FALSE;
		m_fBurstDelay = 1.0;
		m_fBurstDelayBonus = 3.0f;
		m_fFireDelay = 0.0f;
		m_fFireDelayBonus = 0.0f;
		m_uNumFiresPerBurst = 1;
		m_uNumFiresPerBurstBonus = 0;
		m_uPreferStopAndShoot = FALSE;

		//machine gun
		m_uTimedBursts2 = TRUE;
		m_fBurstDelay2 = 0.25f;
		m_fBurstDelayBonus2 = 1.5f;
		m_fFireDelay2 = 0.3f;
		m_fFireDelayBonus2 = 1.25f;
		m_uPreferStopAndShoot2 = FALSE;
		m_uMechUseageCtrl = CAIBrain::MECH_USEAGE_CTRL_NEVER;

		m_uRace = AIRACE_AMBIENT;	  //rat's take on the race of the driver, but are ambient until then

		m_uNumWeapons  = 2;		//chain gun and rocket launcher for example
	}
	else if ( pBuilder->m_nEC_LeafTypeBit & ENTITY_BIT_VEHICLELOADER)
	{
		m_uInitialJob = CAIBrain::TT_WAIT;
		m_uWaitScanOdds = 0;

		m_uRace = AIRACE_AMBIENT;	  //rat's take on the race of the driver, but are ambient until then
		m_uBaseSpeed = 50;

		m_fBonusEyeScanDistForPlayersAlert = 300.0f;

		//claw
		m_uTimedBursts = TRUE;
		m_fBurstDelay = 0.0f;
		m_fBurstDelayBonus = 2.0f;
		m_fFireDelay = 0.35f;
		m_fFireDelayBonus = 0.5f;
		m_uPreferStopAndShoot = FALSE;
		m_bNeverUnarmed = TRUE;

		//machine gun
		m_uTimedBursts2 = TRUE;
		m_fBurstDelay2 = 0.4f;
		m_fBurstDelayBonus2 = 3.0f;
		m_fFireDelay2 = 0.5f;
		m_fFireDelayBonus2 = 4.25f;
		m_uPreferStopAndShoot2 = FALSE;

		m_uMechUseageCtrl = CAIBrain::MECH_USEAGE_CTRL_NEVER;

		m_uNumWeapons  = 2;	//claw and machine gun
		m_uMeleeRad = 20;
		m_uMeleeTimeMin = 0;	 
		m_uMeleeTimeMax = 1;
	}
	else if ( pBuilder->m_nEC_LeafTypeBit & ENTITY_BIT_BOTPRED)
	{  //ai defaults for the predator
		m_fEyeCosHalfFOVUnalert = 0.86f; //roughly 30 degs half angle
		m_fEyeScanDistUnalert = 50;
		m_fBonusEyeScanDistForPlayersUnalert = 150.0f;

		m_fEyeCosHalfFOVAlert = 0.64f; //roughly 50 degs half angle
		m_fEyeScanDistAlert = 50;
		m_fBonusEyeScanDistForPlayersAlert = 250.0f;

		//primary quad laser
		m_uTimedBursts = TRUE;
		m_fBurstDelay = 0.75f;
		m_fBurstDelayBonus = 2.2f;
		m_fFireDelay = 0.3f;
		m_fFireDelayBonus = 1.25f;
		m_uPreferStopAndShoot = FALSE;

		//secondary volly
		m_uTimedBursts2 = TRUE;
		m_fBurstDelay2 = 0.0;
		m_fBurstDelayBonus2 = 4.0f;
		m_fFireDelay2 = 2.0f;
		m_fFireDelayBonus2 = 3.75f;
		m_uPreferStopAndShoot2 = FALSE;

		m_uNormalRuleSet = CAIBrain::ATTACKRULESET_PRED0;

		m_uNumWeapons  = 2;		//chain gun and rocket launcher for example

	}
	else if ( pBuilder->m_nEC_LeafTypeBit & ENTITY_BIT_BOTPROBE)
	{	 //ai defaults for the probe
		m_uNormalRuleSet = CAIBrain::ATTACKRULESET_PROBE0;
		m_uAttackMeleeMinRad = 5;

		m_fEyeCosHalfFOVUnalert = 0.5f; 
		m_fEyeScanDistUnalert = 50;
		m_fBonusEyeScanDistForPlayersUnalert = 150.0f;

		m_fEyeCosHalfFOVAlert = -1.0f; //roughly 50 degs half angle
		m_fEyeScanDistAlert = 50;
		m_fBonusEyeScanDistForPlayersAlert = 150.0f;

		m_uMeleeRad = 3;
		m_uMeleeTimeMin = 0;	 
		m_uMeleeTimeMax = 3;
	}
	else if ( pBuilder->m_nEC_LeafTypeBit & ENTITY_BIT_BOTTITAN)
	{	//ai defaults for titan
		//Chain Gun
		m_uTimedBursts = TRUE;
		m_fBurstDelay = 1.0f;
		m_fBurstDelayBonus = 2.0f;
		m_fFireDelay = 2.0f;
		m_fFireDelayBonus = 3.75f;
		m_uPreferStopAndShoot = FALSE;
		m_uJumpDist = 60;

		//Rocket
		m_uTimedBursts2 = FALSE;
		m_fBurstDelay2 = 2.0f;
		m_fBurstDelayBonus2 = 2.5f;
		m_fFireDelay2 = 0.25f;
		m_fFireDelayBonus2 = 0.1f;
		m_uNumFiresPerBurst2 = 1;
		m_uNumFiresPerBurstBonus2 = 2;
		m_uPreferStopAndShoot2 = FALSE;
		m_uAttackMeleeMinRad = 19;	  //25 - titanxzrad = 19
		m_uNormalRuleSet = CAIBrain::ATTACKRULESET_TITAN0;
		m_uAlternateRuleSet = CAIBrain::ATTACKRULESET_TITAN0;
		m_uAlternateRuleSetOdds = 100;	   //cheese this is odds that titan will do a huge jump when he wants to
		m_bAttackParamsSpecified = TRUE;
		m_uMechUseageCtrl = CAIBrain::MECH_USEAGE_CTRL_NEVER;
		m_uMechUseageRadius = 0;

		m_fEyeScanDistAlert = 50;
		m_fBonusEyeScanDistForPlayersAlert = 350.0f;

		m_uNumWeapons = 2;		//chain gun and rocket launcher
		m_uMeleeRad = 19;
		m_uMeleeTimeMin = 0;	 
		m_uMeleeTimeMax = 3;

	}
	else if ( pBuilder->m_nEC_LeafTypeBit & ENTITY_BIT_SITEWEAPON)
	{   //ai defaults for site weapons with brains
		m_uMechUseageCtrl = CAIBrain::MECH_USEAGE_CTRL_NEVER;
		m_uMechUseageRadius = 0;

		//configure how this site weapon will be used when a grunt jumps in.
		m_uTimedBursts = TRUE;
		m_fBurstDelay = 0.3f;
		m_fBurstDelayBonus = 1.4f;
		m_fFireDelay = 2.0f;
		m_fFireDelayBonus = 3.75f;
		m_uPreferStopAndShoot = FALSE;

		m_uMechUseageCtrl = CAIBrain::MECH_USEAGE_CTRL_NEVER;
		m_uRace = AIRACE_AMBIENT;
	}
	else if ( pBuilder->m_nEC_LeafTypeBit & ENTITY_BIT_BOTMORTAR)
	{   //ai defaults for mortars
		m_uMechUseageCtrl = CAIBrain::MECH_USEAGE_CTRL_NEVER;
		m_uMechUseageRadius = 0;
		m_bNeverUnarmed = 1;

		m_fEyeScanDistUnalert = 50.0f;
		m_fBonusEyeScanDistForPlayersUnalert = 200.0f;
		m_fHearingMagUnalert = 1.0f;
		m_fEyeCosHalfFOVUnalert = -1.0f;

		m_fHearingMagAlert = 1.0f;
		m_fEyeCosHalfFOVAlert = -1.0f;

		m_uNumWeapons = 0;
	}
	else if ( pBuilder->m_nEC_LeafTypeBit & ENTITY_BIT_BOTMINER)
	{   //ai defaults for miner
		m_uRace = AIRACE_DROID;

		m_fEyeScanDistUnalert = 50.0f;
		m_fBonusEyeScanDistForPlayersUnalert = 25.0f;

		m_fEyeScanDistAlert = 75.0f;
		m_fBonusEyeScanDistForPlayersAlert = 130.0f;
		
		m_uInnerRadiusForIgnorePlayerOdds = 50;

		m_uBuddyCtrl = CAIBrain::BRAINFLAG_BUDDY_CTRL_AUTO;	  //miners automatically buddy up with players of the same race, unless they are set to attack friendly players
		//Can't set weapon defaults until we know what weapon the bot will have.
	}
	else if ( pBuilder->m_nEC_LeafTypeBit & ENTITY_BIT_BOTCHEMBOT)
	{   //ai defaults for slosh
		m_uRace = AIRACE_DROID;
		//Can't set weapon defaults until we know what weapon the bot will have.
	}
	else if ( pBuilder->m_nEC_LeafTypeBit & ENTITY_BIT_BOTCORROSIVE)
	{   //ai defaults for corrosive
		m_nBrainType = BRAINTYPE_CORROSIVE;
		m_uNormalRuleSet = CAIBrain::ATTACKRULESET_CORROSIVE0;
		m_uMechUseageCtrl = CAIBrain::MECH_USEAGE_CTRL_NEVER;
		m_uMechUseageRadius = 0;

		m_uInitialJob = CAIBrain::TT_WANDER;

		m_bNeverUnarmed = TRUE;
		m_uJumpDist = 100;
		m_fEyeScanDistUnalert = 150;
		m_fBonusEyeScanDistForPlayersUnalert = 350.0f;
		m_fEyeScanDistAlert = 0;
		m_fBonusEyeScanDistForPlayersAlert = 350.0f;
		m_fHearingMagUnalert = m_fHearingMagAlert = 20.0f; // Corrosive can hear everything!

		//how often will he fire weapon one
		m_uTimedBursts = FALSE;
		m_fBurstDelay = 0.5f;		   //TimeBetweenBursts = m_fBurstDelay1 + fmathRandom()*m_fBurstDelayBonus1;
		m_fBurstDelayBonus = 10.0f;
		m_uNumFiresPerBurst = 1;
		m_uNumFiresPerBurstBonus = 0;
		m_fFireDelay = 0.25f;		//doesn't really apply if you only have one shot per burst
		m_fFireDelayBonus = 0.1f;	//doesn't really apply if you only have one shot per burst
	}
	else if ( pBuilder->m_nEC_LeafTypeBit & ENTITY_BIT_BOTZOMBIEBOSS)
	{   //ai defaults for corrosive
		m_uRace = AIRACE_ZOMBIE;
		m_nBrainType = BRAINTYPE_ZOMBIEBOSS;
		m_uNormalRuleSet = CAIBrain::ATTACKRULESET_ZOMBIEBOSS0;
		m_uMechUseageCtrl = CAIBrain::MECH_USEAGE_CTRL_NEVER;
		m_uMechUseageRadius = 0;

		m_uInitialJob = CAIBrain::TT_WANDER;

		m_bNeverUnarmed = TRUE;

		m_fEyeCosHalfFOVUnalert = -1.f; 
		m_fEyeCosHalfFOVAlert = -1.f; 

		m_fEyeScanDistUnalert = 150;
		m_fBonusEyeScanDistForPlayersUnalert = 350.0f;
		m_fEyeScanDistAlert = 0;
		m_fBonusEyeScanDistForPlayersAlert = 350.0f;


		//how often will he fire weapon one
		m_uTimedBursts = FALSE;
		m_fBurstDelay = 0.5f;		   //TimeBetweenBursts = m_fBurstDelay1 + fmathRandom()*m_fBurstDelayBonus1;
		m_fBurstDelayBonus = 10.0f;
		m_uNumFiresPerBurst = 1;
		m_uNumFiresPerBurstBonus = 0;
		m_fFireDelay = 0.25f;		//doesn't really apply if you only have one shot per burst
		m_fFireDelayBonus = 0.1f;	//doesn't really apply if you only have one shot per burst
	}
	else if ( pBuilder->m_nEC_LeafTypeBit & ENTITY_BIT_BOTGLITCH)
	{
		m_uRace = AIRACE_DROID;
	} 
	else if ( pBuilder->m_nEC_LeafTypeBit & ENTITY_BIT_BOTSCIENTIST)
	{
		m_uRace = AIRACE_MIL;
		m_uMechUseageCtrl = CAIBrain::MECH_USEAGE_CTRL_NEVER;
		m_bNeverUnarmed = TRUE;
		m_uInitialJob = CAIBrain::TT_WAIT;
		m_uNormalRuleSet = CAIBrain::ATTACKRULESET_SCIENTIST0;

		//how often will he fire weapon one
		m_uTimedBursts = TRUE;
		m_fBurstDelay = 3.0f;		   //TimeBetweenBursts = m_fBurstDelay1 + fmathRandom()*m_fBurstDelayBonus1;
		m_fBurstDelayBonus = 10.0f;
		m_fFireDelay = 1.5f;		
		m_fFireDelayBonus = 1.15f;	

		m_uMeleeRad = 15;
		m_uMeleeTimeMin = 1;
		m_uMeleeTimeMax = 6;

		m_fAccuracy= 1.0f;
		m_uBaseSpeed = 30;
		m_uAttackSpeedMin = 30;
		m_uAttackSpeedMax = 30;

		m_uOddsOfIgnoringNearbyGrenades = 100;
		m_uOddsOfNotGoingToSeeSounds = 100;
	} 
	else if( pBuilder->m_nEC_LeafTypeBit & ENTITY_BIT_BOTSNARQ)
	{
		m_uRace = AIRACE_MIL;
		m_bNeverUnarmed = TRUE;
		m_uMechUseageCtrl = CAIBrain::MECH_USEAGE_CTRL_NEVER;
		m_uInitialJob = CAIBrain::TT_WAIT;

		//Fire 1 is pulse cannon with spash damage
		m_uPreferStopAndShoot = FALSE;
		m_uTimedBursts = TRUE;
		m_fBurstDelay = 0.5f;
		m_fBurstDelayBonus =  1.5f;
		m_fFireDelay = 1.5f;	//length of burst min
		m_fFireDelayBonus = 4.0f;
		m_fAccuracy= 0.5f;
		m_uJumpDist = 150;

//		m_uNormalRuleSet = CAIBrain::ATTACKRULESET_0;
		m_uNormalRuleSet = CAIBrain::ATTACKRULESET_PRED0;

		m_fEyeScanDistUnalert = 50;
		m_fBonusEyeScanDistForPlayersUnalert = 150.0f;

		m_fEyeScanDistAlert = 80;
		m_fBonusEyeScanDistForPlayersAlert = 250.0f;

		m_nMoverType = MOVERCLASS_3D;

		m_uBaseSpeed = 50;
		m_uAttackSpeedMin = 50;
		m_uAttackSpeedMax = 51;

//		m_fAttackMaxDistFromOrigin = 1.0f;
//		m_bAttackParamsSpecified = TRUE;
	}
	else if ( pBuilder->m_nEC_LeafTypeBit & ENTITY_BIT_BOTMOZER)
	{
		m_uRace = AIRACE_DROID;
	}	
	


	//Prepare for many calls to CAIBuilder::InterpretTable
	BuilderHelp_SetDefaults(pAIBuilder, _aItableDataSpec, sizeof(_aItableDataSpec)/sizeof(InterpretTableDataSpec));
}
																																																						  

BOOL CAIBuilder::InterpretTable( CEntityBuilder* pBuilder )
{
	BOOL bDidInterpret = FALSE;
	FGameDataTableHandle_t hCurTable = CEntityParser::m_hTable;
	FGameData_VarType_e eVarType;

	if (CEntityParser::m_pszTableName && !fclib_strnicmp(CEntityParser::m_pszTableName, "AI_", 3))
	{
		BuilderHelp_InterpretTable(_aItableDataSpec, sizeof(_aItableDataSpec)/sizeof(InterpretTableDataSpec), &bDidInterpret);

		if (!bDidInterpret)
		{
			//
			// Patrol
			//
			if ( !fclib_stricmp( CEntityParser::m_pszTableName, "AI_PATROLIGNOREZONE" ) )
			{
				u32 uCurEntry, uNumEntries = CEntityParser::m_nFieldCount;
				if(uNumEntries > MAXNUM_PATROL_IGNOREZONES)
				{
					DEVPRINTF("CAIBuilder::InterpretTable() : Too many entries in 'AI_PATROLIGNOREZONE' table (max = %d).\n", MAXNUM_PATROL_IGNOREZONES);
					uNumEntries = MAXNUM_PATROL_IGNOREZONES;
				}

				for(uCurEntry = 0; uCurEntry < uNumEntries; ++uCurEntry)
				{
					fgamedata_GetPtrToFieldData(hCurTable, uCurEntry, eVarType);
					CEntityParser::Interpret_String( &(m_apszIgnorePatrolZone[uCurEntry]) );
					m_uNumIgnorePatrolZones++;
				}
				bDidInterpret = TRUE;
			}
		}
	}

	return bDidInterpret;
}


BOOL CAIBuilder::PostInterpretFixup( CEntityBuilder* pBuilder )
{
	u32 i;
	if (m_pszWanderPts && !(m_pszWanderPts = gstring_Main.AddString( m_pszWanderPts)) )
	{  //coulnd't fit that string into the global table
		goto _XOutOfMemory;
	}
	if (m_pszPatrolPath && !(m_pszPatrolPath = gstring_Main.AddString( m_pszPatrolPath )) )
	{  //coulnd't fit that string into the global table
		goto _XOutOfMemory;
	}
	if ( m_pszWaitLookAtObj && !(m_pszWaitLookAtObj = gstring_Main.AddString( m_pszWaitLookAtObj )) )
	{  //coulnd't fit that string into the global table
		goto _XOutOfMemory;
	}

	if ( m_pszLeaderName && !(m_pszLeaderName = gstring_Main.AddString( m_pszLeaderName )) )
	{  //coulnd't fit that string into the global table
		goto _XOutOfMemory;
	}

	if ( m_pszStringTableCommGroupName && !(m_pszStringTableCommGroupName = gstring_Main.AddString( m_pszStringTableCommGroupName )) )
	{  //coulnd't fit that string into the global table
		goto _XOutOfMemory;
	}
	
	for(i = 0; i < m_uNumIgnorePatrolZones; i++)
	{
		if (m_apszIgnorePatrolZone[i] && !(m_apszIgnorePatrolZone[i] = gstring_Main.AddString( m_apszIgnorePatrolZone[i] )))
		{  //coulnd't fit that string into the global table
			goto _XOutOfMemory;
		}
	}

	return TRUE;

_XOutOfMemory:
	CEntityParser::Error_OutOfMemory();
	return FALSE;
}


BOOL CAIBrain::InitFromBuilder(CEntityBuilder* pBuilder)
{
	CAIBuilder* pAIBuilder = pBuilder->m_pAIBuilder;


	switch (((CBotBuilder*) pBuilder)->m_anNPCWeaponType[0])
	{
	case CBotBuilder::_NPC_WEAPON_LASER:
		BuilderHelp_SetSafeDefaultU32(&(pAIBuilder->m_uTimedBursts), FALSE);
		BuilderHelp_SetSafeDefaultF32(&(pAIBuilder->m_fBurstDelay), 0.5f);
		BuilderHelp_SetSafeDefaultF32(&(pAIBuilder->m_fBurstDelayBonus), 1.5f);
		BuilderHelp_SetSafeDefaultF32(&(pAIBuilder->m_fFireDelay), 0.15f);	//0 means every frame
		BuilderHelp_SetSafeDefaultF32(&(pAIBuilder->m_fFireDelayBonus), 0.05f);
		BuilderHelp_SetSafeDefaultU32(&(pAIBuilder->m_uNumFiresPerBurst), 3);
		BuilderHelp_SetSafeDefaultU32(&(pAIBuilder->m_uNumFiresPerBurstBonus), 7);
		BuilderHelp_SetSafeDefaultF32(&(pAIBuilder->m_fAccuracy), 0.5f);
		break;
	case CBotBuilder::_NPC_WEAPON_SPEW:
		BuilderHelp_SetSafeDefaultU32(&(pAIBuilder->m_uTimedBursts), TRUE);
		BuilderHelp_SetSafeDefaultF32(&(pAIBuilder->m_fBurstDelay), 0.25f);
		BuilderHelp_SetSafeDefaultF32(&(pAIBuilder->m_fBurstDelayBonus), 2.0f);
		BuilderHelp_SetSafeDefaultF32(&(pAIBuilder->m_fFireDelay), 0.5f);	//0 means every frame
		BuilderHelp_SetSafeDefaultF32(&(pAIBuilder->m_fFireDelayBonus), 3.032f);
		BuilderHelp_SetSafeDefaultU32(&(pAIBuilder->m_uNumFiresPerBurstBonus), 3);
		BuilderHelp_SetSafeDefaultF32(&(pAIBuilder->m_fAccuracy), 0.5f);
		break;
	case CBotBuilder::_NPC_WEAPON_FLAMER:
		BuilderHelp_SetSafeDefaultU32(&(pAIBuilder->m_uTimedBursts), TRUE);
		BuilderHelp_SetSafeDefaultF32(&(pAIBuilder->m_fBurstDelay), 2.0f);
		BuilderHelp_SetSafeDefaultF32(&(pAIBuilder->m_fBurstDelayBonus), 2.0f);
		BuilderHelp_SetSafeDefaultF32(&(pAIBuilder->m_fFireDelay), 2.0f);
		BuilderHelp_SetSafeDefaultF32(&(pAIBuilder->m_fFireDelayBonus), 2.0f);
		BuilderHelp_SetSafeDefaultU32(&(pAIBuilder->m_uPreferStopAndShoot), FALSE);
		break;
	case CBotBuilder::_NPC_WEAPON_RIVET:
		BuilderHelp_SetSafeDefaultU32(&(pAIBuilder->m_uTimedBursts), FALSE);
		BuilderHelp_SetSafeDefaultF32(&(pAIBuilder->m_fBurstDelay), 0.5f);
		BuilderHelp_SetSafeDefaultF32(&(pAIBuilder->m_fBurstDelayBonus), 1.5f);
		BuilderHelp_SetSafeDefaultF32(&(pAIBuilder->m_fFireDelay), 0.15f);	//0 means every frame
		BuilderHelp_SetSafeDefaultF32(&(pAIBuilder->m_fFireDelayBonus), 0.05f);
		BuilderHelp_SetSafeDefaultU32(&(pAIBuilder->m_uNumFiresPerBurst), 2);
		BuilderHelp_SetSafeDefaultU32(&(pAIBuilder->m_uNumFiresPerBurstBonus), 3);
		BuilderHelp_SetSafeDefaultF32(&(pAIBuilder->m_fAccuracy), 0.5f);
		break;
	case CBotBuilder::_NPC_WEAPON_ROCKET:
		BuilderHelp_SetSafeDefaultU32(&(pAIBuilder->m_uTimedBursts), FALSE);
		BuilderHelp_SetSafeDefaultF32(&(pAIBuilder->m_fBurstDelay), 0.5f);
		BuilderHelp_SetSafeDefaultF32(&(pAIBuilder->m_fBurstDelayBonus), 1.5f);
		BuilderHelp_SetSafeDefaultF32(&(pAIBuilder->m_fFireDelay), 0.15f);	//0 means every frame
		BuilderHelp_SetSafeDefaultF32(&(pAIBuilder->m_fFireDelayBonus), 0.05f);
		BuilderHelp_SetSafeDefaultU32(&(pAIBuilder->m_uNumFiresPerBurst), 3);
		BuilderHelp_SetSafeDefaultU32(&(pAIBuilder->m_uNumFiresPerBurstBonus), 7);
		BuilderHelp_SetSafeDefaultF32(&(pAIBuilder->m_fAccuracy), 0.5f);
		break;
	}

	//default defaults
	BuilderHelp_SetSafeDefaultF32(&(pAIBuilder->m_fBurstDelay), 0.5f);
	BuilderHelp_SetSafeDefaultF32(&(pAIBuilder->m_fBurstDelayBonus), 1.5f);
	BuilderHelp_SetSafeDefaultF32(&(pAIBuilder->m_fFireDelay), 0.15f);
	BuilderHelp_SetSafeDefaultF32(&(pAIBuilder->m_fFireDelayBonus), 0.05f);
	BuilderHelp_SetSafeDefaultU32(&(pAIBuilder->m_uNumFiresPerBurst), 3);
	BuilderHelp_SetSafeDefaultU32(&(pAIBuilder->m_uNumFiresPerBurstBonus),7);
	BuilderHelp_SetSafeDefaultF32(&(pAIBuilder->m_fBurstDelay2), 0.5f);
	BuilderHelp_SetSafeDefaultF32(&(pAIBuilder->m_fBurstDelayBonus2), 1.5f);
	BuilderHelp_SetSafeDefaultF32(&(pAIBuilder->m_fFireDelay2), 0.15f);	//0 means every frame
	BuilderHelp_SetSafeDefaultF32(&(pAIBuilder->m_fFireDelayBonus2), 0.05f);
	BuilderHelp_SetSafeDefaultU32(&(pAIBuilder->m_uNumFiresPerBurst2), 3);
	BuilderHelp_SetSafeDefaultU32(&(pAIBuilder->m_uNumFiresPerBurstBonus2), 7);
	BuilderHelp_SetSafeDefaultF32(&(pAIBuilder->m_fAccuracy), 0.5f);
	BuilderHelp_SetSafeDefaultF32(&(pAIBuilder->m_fAccuracy2), 0.5f);
	BuilderHelp_SetSafeDefaultU32(&(pAIBuilder->m_uPreferStopAndShoot), TRUE);
	BuilderHelp_SetSafeDefaultU32(&(pAIBuilder->m_uPreferStopAndShoot2), TRUE);
	BuilderHelp_SetSafeDefaultU32(&(pAIBuilder->m_uTimedBursts), FALSE);
	BuilderHelp_SetSafeDefaultU32(&(pAIBuilder->m_uTimedBursts2), FALSE);

	SetAttrib(CAIBrain::ATTRIB_AGGRESSION, (u8) pAIBuilder->m_uAggression);
	SetAttrib(CAIBrain::ATTRIB_COURAGE, (u8) pAIBuilder->m_uCourage);
	SetAttrib(CAIBrain::ATTRIB_INTELLIGENCE, (u8) pAIBuilder->m_uIntelligence);
	SetAttrib(CAIBrain::ATTRIB_SPEED, (u8) pAIBuilder->m_uBaseSpeed);
	SetAttackRuleSet((u8) pAIBuilder->m_uNormalRuleSet);
	if (pAIBuilder->m_bLODOverrideAlways)
	{
		SetLODOverrideAlways();
	}
	if (pAIBuilder->m_bLODOverrideWhenConvienient)
	{
		SetLODOverrideWhenConvenient();
	}
	if (pAIBuilder->m_uInitialGreetingLevel > CAIBrain::GREETINGLEVEL_0)
	{
		OverrideGreetingLevel(0, (u8) pAIBuilder->m_uInitialGreetingLevel);
	}


	//bit fields are a bit tricky....
	if (pAIBuilder->m_uMechUseageTypeFlags == 0xffffffff)
	{
		//no user props specified. so put in a default
		if (GetAIMover()->GetEntity()->TypeBits() & ENTITY_BIT_BOTGRUNT)
		{
			pAIBuilder->m_uMechUseageTypeFlags = CAIBrain::MECH_USEAGE_TYPE_ALL;
		}
		else
		{
			pAIBuilder->m_uMechUseageTypeFlags = CAIBrain::MECH_USEAGE_TYPE_FLAG_NONE;
		}
	}

	if (pAIBuilder->m_uGrenadeUse == 0xffffffff)
	{
		if ( GetAIMover()->GetEntity()->TypeBits() & ENTITY_BIT_BOTGRUNT)
		{   //ai defaults for grunts

			pAIBuilder->m_uGrenadeUse = AI_GRENADE_USE_NEVER;
//			pAIBuilder->m_uGrenadeUse = AI_GRENADE_USE_WHENEVER;
//			pAIBuilder->m_uGrenadeUse = (0 |
//										AI_GRENADE_USE_ENEMYINSIGHT | 
//										AI_GRENADE_USE_SELF_DEFENSE | 
///										AI_GRENADE_USE_FIRST_CONTACT | 
//										AI_GRENADE_USE_ENEMY_EVASIVE |
//										AI_GRENADE_USE_RETURN_GRENADE_FIRE |
//										0);
		}
		else
		{
			pAIBuilder->m_uGrenadeUse = AI_GRENADE_USE_NEVER;
		}
	}




	SetRace((u8) pAIBuilder->m_uRace);
	SetBaseSpeed((u8) pAIBuilder->m_uBaseSpeed);
	m_uJumpDist = (u16) pAIBuilder->m_uJumpDist;

	GetAIMover()->m_uNumWeaponCtrls = pAIBuilder->m_uNumWeapons;
	if (GetAIMover()->m_uNumWeaponCtrls)
	{
		GetAIMover()->m_paWeaponCtrl = fnew CAIWeaponCtrl[GetAIMover()->m_uNumWeaponCtrls];
		if (!GetAIMover()->m_paWeaponCtrl)
		{
			GetAIMover()->m_uNumWeaponCtrls = 0;  //not fatal, just set this to cover ass of the programmer
		}
	}

	for (u32 i = 0; i < GetAIMover()->GetNumWeaponCtrls(); i++)
	{
		GetAIMover()->GetWeaponCtrlPtr(i)->InitFromBuilder(pAIBuilder, this, i);

		if (i == 0 && 
			GetAIMover()->GetEntity()->TypeBits() & ENTITY_BIT_BOTTITAN)
		{
			CBotTitan* pTitan = (CBotTitan*) GetAIMover()->GetEntity();
			pTitan->SetGunsHeatupPerSecond( 1.0f/(pAIBuilder->m_fBurstDelay*0.9f) );
			pTitan->SetGunsCooldownPerSecond( 1.0f/(pAIBuilder->m_fBurstDelay*0.1f) );
			pTitan->SetGunsCooldownOverheatPenalty( 1.0f );	//no penalty
		}
	}

	m_uBrainFlags &=~(BRAINFLAG_ATTACKS_ENEMY | BRAINFLAG_ATTACKS_NEUTRAL | BRAINFLAG_ATTACKS_FRIENDLY | BRAINFLAG_ATTACKS_PLAYERS | BRAINFLAG_ATTACKS_NPC);
	m_uBrainFlags |= pAIBuilder->m_uBrainFlags;

	m_uBrainFlags |= pAIBuilder->m_uBuddyCtrl;
	if (GetFlag_Buddy_Ctrl_OnAction() || GetFlag_Buddy_Ctrl_Auto())
	{
		GetAIMover()->GetEntity()->SetActionable(TRUE);
	}

	if (GetAIMover()->GetEntity()->TypeBits() & ENTITY_BIT_BOTSNARQ)
	{
		if (GetAIMover()->CanHover())
		{
			((CAIHoverBotMover*)GetAIMover())->m_fMinAltitude = 7.0f;
		}
	}

	SetHearingMagUnalert(pAIBuilder->m_fHearingMagUnalert);
	SetEyeScanDistUnalert(pAIBuilder->m_fEyeScanDistUnalert);
	SetBonusEyeScanDistForPlayersUnalert(pAIBuilder->m_fBonusEyeScanDistForPlayersUnalert);
	SetEyeCosHalfFOVUnalert(pAIBuilder->m_fEyeCosHalfFOVUnalert);

	SetHearingMagAlert(pAIBuilder->m_fHearingMagAlert);
	SetEyeScanDistAlert(pAIBuilder->m_fEyeScanDistAlert);
	SetBonusEyeScanDistForPlayersAlert(pAIBuilder->m_fBonusEyeScanDistForPlayersAlert);
	SetEyeCosHalfFOVAlert(pAIBuilder->m_fEyeCosHalfFOVAlert);

	m_uNumWeapons = (u8)pAIBuilder->m_uNumWeapons;
	m_uWakeupRadius = (u8)pAIBuilder->m_uWakeupRadius;
	m_uMeleeRad = (u8)pAIBuilder->m_uMeleeRad;
	m_uMeleeTimeMin = (u8)pAIBuilder->m_uMeleeTimeMin;
	m_uMeleeTimeMax = (u8)pAIBuilder->m_uMeleeTimeMax;
	m_pszStringTableCommGroupName = pAIBuilder->m_pszStringTableCommGroupName;			//a string table entry for the communication group name this brain is in.

	m_uOddsOfIgnoringPlayerAtEyeScanDist = (u8) pAIBuilder->m_uOddsOfIgnoringPlayerAtEyeScanDist;
	m_uInnerRadiusForIgnorePlayerOdds = (u8) pAIBuilder->m_uInnerRadiusForIgnorePlayerOdds;	
	m_uOddsOfIgnoringNearbyGrenades = (u8) pAIBuilder->m_uOddsOfIgnoringNearbyGrenades;
	m_uOddsOfNotGoingToSeeSounds = (u8) pAIBuilder->m_uOddsOfNotGoingToSeeSounds;
	m_uAttackNotifyRadius = (u8) pAIBuilder->m_uAttackNotifyRadius;				
	m_uStartleMin = (u8) pAIBuilder->m_uStartleMin;
	m_uStartleMax = (u8) pAIBuilder->m_uStartleMax;
	m_uMechUseageRadius = (u8) pAIBuilder->m_uMechUseageRadius;
	m_uMechUseageCtrl = (u8) pAIBuilder->m_uMechUseageCtrl;
	m_uMechUseageTypeFlags = (u8) pAIBuilder->m_uMechUseageTypeFlags;

	if (pAIBuilder->m_bNeverUnarmed)
	{
		SetFlag_NeverUnarmed();
	}
	else
	{
		ClearFlag_NeverUnarmed();
	}

	if (pAIBuilder->m_uPillUseageCtrl != MECH_USEAGE_CTRL_NEVER)
	{
		m_uMechUseageCtrl = (u8) pAIBuilder->m_uPillUseageCtrl;
		m_uMechUseageTypeFlags |= MECH_USEAGE_TYPE_FLAG_PILL;
	}

	if (pAIBuilder->m_bAttackParamsSpecified)
	{
		CAIMemoryLarge* pAttackSpec = GetKnowledge().RememberLarge(MEMORY_LARGE_ATTACKSPECS, 0, CAIMemoryLarge::CONTROL_FLAG_NOTIMER);

		if (pAttackSpec)
		{
			StuffAttackSpecs(	pAttackSpec,
								(u8) pAIBuilder->m_uNormalRuleSet,	 // which ruleset the brain will use when attacking
								(u8) pAIBuilder->m_uAlternateRuleSet,
								(u8) pAIBuilder->m_uAlternateRuleSetOdds, //(0-100) pct chance that the alternate will be used
								(u8) pAIBuilder->m_uAttackSpeedMin,  //(0-100)
								(u8) pAIBuilder->m_uAttackSpeedMax,
								(u16) pAIBuilder->m_fAttackMaxDistFromOrigin,
								(u16) pAIBuilder->m_fStayPutDist,
								(u8) pAIBuilder->m_uAttackMeleeMinRad);	//(0-100)
		}
	}

	if (m_uMechUseageCtrl == MECH_USEAGE_CTRL_INIT &&
		m_uMechUseageTypeFlags != MECH_USEAGE_TYPE_FLAG_NONE &&
		pAIBuilder->m_uInitialJob != TT_WAIT)
	{
		DEVPRINTF("bots with MECH_USEAGE_INIT should always have initial job of WAIT");
		pAIBuilder->m_uInitialJob = TT_WAIT; 
	}

	if (m_uMechUseageCtrl == MECH_USEAGE_CTRL_INIT &&
		m_uMechUseageTypeFlags != MECH_USEAGE_TYPE_FLAG_NONE)
	{
		SetLODOverrideForOneWork();
	}

	switch (pAIBuilder->m_uInitialJob)
	{
		case TT_WAIT:
			{
				CEntity* pEntity = NULL;
				u32 uGuid = 0;
				if (pAIBuilder->m_pszWaitLookAtObj)
				{
					pEntity = CEntity::Find(pAIBuilder->m_pszWaitLookAtObj);
					if (pEntity)
					{
						uGuid = pEntity->Guid();
					}
				}
				
				ai_AssignJob_Wait(	this,	
									FALSE,
									CFVec3A::m_Null,
									uGuid,
									(u8) pAIBuilder->m_uWaitScanOdds);
			}
			break;
		case CAIBrain::TT_WANDER:
			{
				ai_AssignJob_Wander(this,
									pAIBuilder->m_fWanderRadius,
									pAIBuilder->m_bWanderStayInRoom, 
									(u8) pAIBuilder->m_uWanderSpeedMin,
									(u8) pAIBuilder->m_uWanderSpeedMax,
									(u8) pAIBuilder->m_uWanderSociable,
									(u8) pAIBuilder->m_uWanderCuriosity,
									(u8) pAIBuilder->m_uWanderPauseOdds,
									pAIBuilder->m_pszWanderPts);
			}
			break;
		case CAIBrain::TT_PATROL:
			{
				ai_AssignJob_Patrol(this,
									pAIBuilder->m_pszPatrolPath,
									(u8) pAIBuilder->m_uPatrolSpeed,
									(u8) pAIBuilder->m_bPatrolDirection, 
									(u8) pAIBuilder->m_uNumIgnorePatrolZones,
									pAIBuilder->m_apszIgnorePatrolZone);
			}
			break;
	}

	if (pAIBuilder->m_pszLeaderName)
	{
		CEntity* pLeaderEntity = CEntity::Find(pAIBuilder->m_pszLeaderName);
		if (pLeaderEntity &&
			pLeaderEntity->AIBrain() && 
			pLeaderEntity->AIBrain()->CanLead(this) &&
			this->m_apThoughtBankAccessFunc && 
			this->m_apThoughtBankAccessFunc[TT_FOLLOW])
		{
			ai_FollowLeader(this, pLeaderEntity->Guid(), 0);
		}
	}

	if (pAIBuilder->m_uNumGrenadesAtStart)
	{
		CAIMemoryMedium* pMem = NULL;
		if (pMem = GetKnowledge().RememberMedium(MEMORY_MEDIUM_NUM_GRENADES, 0, CAIMemorySmall::CONTROL_FLAG_NOTIMER))
		{
			pMem->m_uSmallData = pAIBuilder->m_uNumGrenadesAtStart;
			pMem->m_uMediumData = pAIBuilder->m_uGrenadeUse;
			pMem->m_fMediumData = (f32) pAIBuilder->m_uGrenadeOdds;
		}
	}
	return TRUE;
}



CAIBuilder* CAIBuilder::s_paBuilderPool = NULL;
BOOL	*CAIBuilder::s_panBuilderStatus = NULL;
s32     CAIBuilder::s_nBuilderPoolAllocCount = 0;
BOOL CAIBuilder::InitSystem(void)
{
	FASSERT(!s_paBuilderPool);
	s_paBuilderPool = APE_NEW CAIBuilder[NUM_DEFAULT_GLOBAL_BUILDERS];
	s_panBuilderStatus = (BOOL*) fres_AllocAndZero(sizeof(BOOL)*NUM_DEFAULT_GLOBAL_BUILDERS);
	s_nBuilderPoolAllocCount = NUM_DEFAULT_GLOBAL_BUILDERS;
	return (s_paBuilderPool && s_panBuilderStatus);
}


void CAIBuilder::UninitSystem(void)
{
	APE_ARRAYDELETE(s_paBuilderPool); s_paBuilderPool = NULL;
}


CAIBuilder* CAIBuilder::Request(void)
{
	for (s32 i = 0; i < s_nBuilderPoolAllocCount; i++)
	{
		if (!s_panBuilderStatus[i])
		{
			s_panBuilderStatus[i] = 1;
			return s_paBuilderPool + i;
		}
	}
	return NULL;
}


void CAIBuilder::Recycle(CAIBuilder* pAIBuilder)
{
	s32 nIndex = pAIBuilder - s_paBuilderPool;
	FASSERT( nIndex>=0 && nIndex< s_nBuilderPoolAllocCount);
	FASSERT( s_panBuilderStatus[nIndex] == 1);
	FASSERT(&(s_paBuilderPool[nIndex]) == pAIBuilder);
	
	s_panBuilderStatus[nIndex] = 0;
}