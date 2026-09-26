//////////////////////////////////////////////////////////////////////////////////////
// aiBuilder.h - 
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
#ifndef _AIBUILDER_H_
#define _AIBUILDER_H_ 1

class CAIBrain;
class CEntity;
class CEntityBuilder;

class CAIBuilder
{
public:
	void SetDefaults( CEntityBuilder* pBuilder);
	BOOL InterpretTable( CEntityBuilder* pBuilder );
	BOOL PostInterpretFixup( CEntityBuilder* pBuilder );

	enum
	{
		NUM_DEFAULT_GLOBAL_BUILDERS = 2,
	};

	u32	m_uInitialJob;			// 0=WAIT, 1=WANDER, 2=PATROL
	u32	m_uAggression;			// (0-100)	How likely the unit is to go on attack when it sees an enemy
	u32	m_uCourage;				// (0-100)	How likely the unit	is to flee when threatened
	u32	m_uIntelligence;		// (0-100)	How well it retains and applys knowledge
	u32 m_uBaseSpeed;			// (0-100)	How fast they will move (pct of what bot is physically capable of)
	s32 m_nBrainType;			// -1=AUTO, 0=BRAINTYPE_GENERICBIPED, 1=BRAINTYPE_GENERICTREADED, 2=BRAINTYPE_GENERICHOVERCRAFT
	s32 m_nMoverType;			// -1=AUTO, 0=GROUND_MOVER, 1=HOVER_MOVER
	u32	m_uTorsoAlignmentSpeedConstraint;	   // (0-100) How much speed is limited by poor "forward" torso alignment
	u32	m_uTorsoAlignmentVelConstraint;		   // (0-100) How much the direction of movement is limited by poor "forward" torso alignment
	BOOL m_bTorsoAlignmentConstraintIgnorable; // (T/F)	 Can m_fTorsoAlignmentSpeedConstraint be ignored when given conflicting FaceGoals and MoveGoals
	u32 m_uRace;				 // AIRACE_MIL, AIRACE_DROID, AIRACE_ZOMBIE
	u32 m_uInitialGreetingLevel; //	CAIBrain::NUM_GREETING_INITIAL, CAIBrain::NUM_GREETING_P1, CAIBrain::NUM_GREETING_P2, CAIBrain::NUM_GREETING_P3, CAIBrain::NUM_GREETING_P4,
	BOOL m_bLODOverrideAlways;			   //if true, the brain will never go inactive due to volume LOD system
	BOOL m_bLODOverrideWhenConvienient;	   //if true, the brain will go inactive due to volume LOD system, but occasionally wake up when convenient performance-wise

	//Brain Perception/Reaction
	f32 m_fHearingMagUnalert;
	f32 m_fEyeScanDistUnalert;
	f32 m_fBonusEyeScanDistForPlayersUnalert;
	f32	m_fEyeCosHalfFOVUnalert; 
	f32 m_fHearingMagAlert;
	f32 m_fEyeScanDistAlert;
	f32 m_fBonusEyeScanDistForPlayersAlert;
	f32	m_fEyeCosHalfFOVAlert; 
	u32 m_uOddsOfIgnoringPlayerAtEyeScanDist;	//When player is seen at this range, what are the odds of ignoring it?
	u32 m_uOddsOfNotGoingToSeeSounds;
	u32 m_uInnerRadiusForIgnorePlayerOdds;		//From this m_uIgnorePlayerOddInnerRadius to m_fEyeScanDist+m_fBonusEyeScanDistForPlayers, Odds of ignoring 0% -m_uOddsOfIgnoringPlayerAtEyeScanDist% will be squared distance interpolation
	u32 m_uOddsOfIgnoringNearbyGrenades;		//When a greande sound is heard, these are the odds that a grenade reaction will NOT be done
	u32 m_uAttackNotifyRadius;					//How large a radius will the bot notify other bots at the time attack begins
	u32	m_uStartleMin;							//Min time between startles
	u32	m_uStartleMax;							//Max time between startles
	u32 m_uBrainFlags;							//Attacks players, NPCS, Enemy, Neutral, Friendly?
	u32 m_uMechUseageRadius;					//What is the radius that this bot is aware of potentially useable vehicles
	u32 m_uMechUseageCtrl;					//0=never uses vehicles, 1 = use vehicles apon entering world, 2 = auto use vehicles during battle, see MECH_USEAGE_CTRL_NEVER
	u32 m_uPillUseageCtrl;						//0=never uses pills, 1 = use pills apon entering world, 2 = auto use vehicles during battle, see MECH_USEAGE_CTRL_NEVER
	u32 m_uMechUseageTypeFlags;				//which type of vehicles to use see brain.h	MECH_USEAGE_CTRL_NEVER
	u32 m_uJumpDist;						//How far can this bot Jump (0=CANT, 1-0xffff)

	//Generic Wait Parameters
	cchar* m_pszWaitLookAtObj;	//				The name of an entity to keep looking at (NULL means ignore)
	u32 m_uWaitScanOdds;		// (0-100)		Odds that unit will look around every now and again.

	//Generic Wander Parameters
	f32 m_fWanderRadius;
	BOOL m_bWanderStayInRoom;
	u32 m_uWanderSpeedMin;
	u32 m_uWanderSpeedMax;
	u32 m_uWanderSociable;
	u32 m_uWanderCuriosity;
	u32 m_uWanderPauseOdds;
	cchar* m_pszWanderPts;

	//Generic Patrol Parameters
	cchar* m_pszPatrolPath;			//				Name of a spline path contianing points to patrol
	BOOL m_bPatrolDirection;		// 0=CLOCK, 1=CCLOCK
	u32 m_uPatrolSpeed;				// (0-100)		How fast they will move
	u32 m_uNumIgnorePatrolZones;
	enum {MAXNUM_PATROL_IGNOREZONES = 10};
	cchar* m_apszIgnorePatrolZone[MAXNUM_PATROL_IGNOREZONES];

	cchar* m_pszLeaderName;	//				The name of an entity to follow around

	//Attack
	BOOL m_bAttackParamsSpecified;
	u32 m_uNormalRuleSet;	 // which ruleset the brain will use when attacking
	u32 m_uAlternateRuleSet;
	u32 m_uAlternateRuleSetOdds; //(0-100) pct chance that the alternate will be used
	u32 m_uAttackMeleeMinRad;
	f32 m_fAttackMaxDistFromOrigin;
	f32 m_fStayPutDist;
	u32 m_uAttackSpeedMin;  //(0-100)
	u32 m_uAttackSpeedMax;	//(0-100)
	BOOL m_bNeverUnarmed;


	//weapon configuration
	f32 m_fBurstDelay;			// 0 means no burst delay (in seconds)
	f32 m_fFireDelay;			// 0 means fire every frame (delay in seconds)
	u32 m_uNumFiresPerBurst;			// >0
	f32 m_fBurstDelayBonus;		// max bonus to be ranomly added to burstdelay
	f32 m_fFireDelayBonus;		// max bonus to be ranomly added to firedelay
	u32 m_uNumFiresPerBurstBonus;		// max bonus to be ranomly added to burstCount
	f32 m_fAccuracy;  //0-1
	u32 m_uNumGrenadesAtStart;
	u32 m_uGrenadeUse;
	u32 m_uGrenadeOdds;
	u32 m_uPreferStopAndShoot;		//this unit prefers to stand and shoot
	u32 m_uTimedBursts;				//If true, the FireDelay+fireDelaybonus is how long a burst is, not decided in shots
	//version 2 (fields must be in the same order as version 1, since an offset is counted on
	f32 m_fBurstDelay2;			
	f32 m_fFireDelay2;			
	u32 m_uNumFiresPerBurst2;			
	f32 m_fBurstDelayBonus2;	
	f32 m_fFireDelayBonus2;		
	u32 m_uNumFiresPerBurstBonus2;		
	f32 m_fAccuracy2;  //0-1
	u32 m_uNumGrenadesAtStart2;
	u32 m_uPreferStopAndShoot2;	
	u32 m_uTimedBursts2;			

	u32 m_uNumWeapons;
	u32 m_uWakeupRadius;
	u32 m_uMeleeRad;
	u32 m_uMeleeTimeMin;
	u32 m_uMeleeTimeMax;
	cchar* m_pszStringTableCommGroupName;
	u32 m_uBuddyCtrl;
	
	static BOOL InitSystem(void);
	static void UninitSystem(void);
	static CAIBuilder* Request(void);
	static void Recycle(CAIBuilder* pBuilder);

protected:
	static CAIBuilder* s_paBuilderPool;
	static BOOL* s_panBuilderStatus;
	static s32 s_nBuilderPoolAllocCount;

	FCLASS_STACKMEM_NOALIGN(CAIBuilder);
};



#endif  //_AIBUILDER_H_