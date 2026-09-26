#ifndef _AITHOUGHTSGROUND_H_
#define _AITHOUGHTSGROUND_H_ 1

#include "AIThought.h"
#include "AIGraphSearcher.h"
#include "fmath.h"
#include "AIFSM.h"

class CGenericWait;
class CGenericWander;
class CAIBrain;
class CEntity;
class CAIMemoryLarge;
class CGroundCombat;
class CAIMover;
class CGroundCombatWorkContext;
class CESpline;
class CGenericGoto;
class CAlarmSwitch;
class CBot;

//
//  Ground Search
//
///////////////////////
//
// Parameters 
//
//	ParamPackSize: Large
//
///////////////////////
FCLASS_ALIGN_PREFIX class CGroundSearch : public CAIThought
{
public:
	CGroundSearch(void) : CAIThought() {}
	virtual void Init(CAIBrain* pBrain, u8 uThoughtFlags, u8 uThoughtType, CAIMemorySmall* pParamPack = NULL);
	virtual void Cleanup(void);
	virtual void Work(void);

	// RTTI
	virtual BOOL SupportsClassInterface(const s32 nInterfaceId);
	virtual s32 GetClassInterface(void);

	CSearchQuery	m_SearchQuery;
	CSearchResults	m_SearchResults;

	void Investigate(const CFVec3A& Pt_WS);
	void SetLookAtMgr(CGenericWait* pLookAtMgr);

	enum
	{
		SEARCHSTATE_WANDER = 1,
		SEARCHSTATE_INVESTIGATE_PF,
		SEARCHSTATE_INVESTIGATE_ENROUTE,
	};

	enum
	{
		SEARCHFLAG_NONE					= 0x0000,
		SEARCHFLAG_RECYCLE_LOOKATMGR	= 0x0001,
	};
	CFVec3A	m_InvestigatePt;
	f32 m_fInvestigateTimeOut;
	CGenericWait* m_pLookAtMgr;
	u8 m_uSearchState;
	u8 m_uSearchFlags;

	static BOOL InitBank(s32 nHowMany);
	static void CleanupBank(void);
	static CAIThought* BankAccess(CAIThought*);
	static CNiBank<CGroundSearch>* s_pAllocBank;

protected:
	CGenericWander *m_pWander;

	FCLASS_STACKMEM_ALIGN(CGroundSearch);
} FCLASS_ALIGN_SUFFIX;




//
//  Pill Bot Wait
//
///////////////////////
//
// Parameters 
//
//	ParamPackSize: Large
//
///////////////////////
FCLASS_ALIGN_PREFIX class CPillWait : public CAIThought
{
public:
	CPillWait(void) : CAIThought() {}
	virtual void Init(CAIBrain* pBrain, u8 uThoughtFlags, u8 uThoughtType, CAIMemorySmall* pParamPack = NULL);
	virtual void Cleanup(void);
	virtual void Work(void);

	// RTTI
	virtual BOOL SupportsClassInterface(const s32 nInterfaceId);
	virtual s32 GetClassInterface(void);

	static BOOL InitBank(s32 nHowMany);
	static void CleanupBank(void);
	static CAIThought* BankAccess(CAIThought*);
	static CNiBank<CPillWait>* s_pAllocBank;

protected:

	FCLASS_STACKMEM_ALIGN(CPillWait);
} FCLASS_ALIGN_SUFFIX;



//
//  Scout Alert
//
///////////////////////
//
// Parameters 
//
//	ParamPackSize: Large
//
///////////////////////
FCLASS_ALIGN_PREFIX class CScoutAlert : public CAIThought
{
public:
	CScoutAlert(void) : CAIThought() {}
	virtual void Init(CAIBrain* pBrain, u8 uThoughtFlags, u8 uThoughtType, CAIMemorySmall* pParamPack = NULL);
	virtual void Cleanup(void);
	virtual void Work(void);

	// RTTI
	virtual BOOL SupportsClassInterface(const s32 nInterfaceId);
	virtual s32 GetClassInterface(void);

	static BOOL InitBank(s32 nHowMany);
	static void CleanupBank(void);
	static CAIThought* BankAccess(CAIThought*);
	static CNiBank<CScoutAlert>* s_pAllocBank;

	BOOL HasLOSToSwitch(CBot* pBot);

	enum
	{
		SCOUTSTAGE_SEE_TARGET = 0,
		SCOUTSTAGE_SCANFOR_TARGET,
		SCOUTSTAGE_SURPRISED,
		SCOUTSTAGE_FIND_PATH,
		SCOUTSTAGE_GOTO,
		SCOUTSTAGE_SWITCH,
		SCOUTSTAGE_NIRVANA,
		NUM_SCOUTSTAGES,
	};

	CGenericGoto* m_pGotoThought;
	CEntity* m_pIntruder;
	CAlarmSwitch* m_pSwitch;
	u8 m_uScoutStage;
	u8 m_uAlarmGotoFailCount;
	u16 m_uNirvanaTimeOut;


protected:

	FCLASS_STACKMEM_ALIGN(CScoutAlert);
} FCLASS_ALIGN_SUFFIX;



//
// class CAttackPtSearch
//
// Helper class used to
// search for a new attack pt
// since happens over multiple frames
//
//	CGroundCombat class
//   contains one of these that can be configured
//   for useage by the current tactic
FCLASS_NOALIGN_PREFIX class CAttackPtSearch
{
public:
	BOOL IsTimeForNewSearch(void);
	void ClearData(void);

	enum
	{
		BOOL_LAST_SEARCH_FAILED = 0,
		BOOL_LAST_SEARCH_SUCCEEDED = 1,
	};

	enum
	{
		ATTACKPT_SEARCHTYPE_LOCAL_RUNTO = 0, 
		ATTACKPT_SEARCHTYPE_LOCAL_HOPTO,
		ATTACKPT_SEARCHTYPE_LOCAL_ROLLTO,
		NUM_ATTACKPT_SEARCHTYPES,
	};

	void SetAttackPtSearchParams(u8 uRepeatTimeMin,
								 u8 uRepeatTimeMax,
								 s8 nDirInc,
								 u8 uDirMin,
								 u8 uDirMax,
								 s8 nRangeXZInc,
								 u8 uRangeXZMin,
								 u8 uRangeXZMax,
								 s8 nRangeYInc,
								 u8 uRangeYMin,
								 u8 uRangeYMax);
	void InitEnemyRel(void);
	void AdvanceEnemyRel(BOOL bLastSearchSuccessful);
	void InitLocal(void);
	void AdvanceLocal(BOOL bLastSearchSuccessful);


	// Configureable params
	u8 m_uRepeatTimeMin;		//How often a search will be initiated.  
	u8 m_uRepeatTimeMax;		//Regardless of; 1)the need for a new attack pt, 2)previous search success or failure, and 3) environment changes
	s8 m_nDirInc;																  
	u8 m_uDirMin;
	u8 m_uDirMax;
	s8 m_nRangeXZInc;
	u8 m_uRangeXZMax;
	u8 m_uRangeXZMin;
	s8 m_nRangeYInc;
	u8 m_uRangeYMax;
	u8 m_uRangeYMin;

	u8 m_uSearchType;

	// search in progress
	u8 m_uFailureCount;
	u8 m_uSucceedCount;
	s8 m_nCurDir;			 //which relative dir is currently being tested?
	s16 m_nCurRangeXZ;		 //which range is currently being tested?
	s16 m_nCurRangeY;
	u16 m_uRepeatTimeOut;

	FCLASS_STACKMEM_NOALIGN(CAttackPtSearch);
} FCLASS_NOALIGN_SUFFIX;

//
// class CCoverPtSearch
//
// Helper class used to
// search for a new cover pt
// since happens over multiple frames
//
//
FCLASS_NOALIGN_PREFIX class CCoverPtSearch
{
public:
	enum
	{
		BOOL_LAST_SEARCH_FAILED = 0,
		BOOL_LAST_SEARCH_SUCCEEDED = 1,
	};
	f32 m_fCoverSearchAngle;
	f32 m_fCoverSearchRange;
	u8 m_uCoverPtFailureCount;

	FCLASS_STACKMEM_NOALIGN(CCoverPtSearch);
} FCLASS_NOALIGN_SUFFIX;


//
//  Ground Attack 
//
///////////////////////
//
// Parameters 
//
//	ParamPackSize: Large
//
///////////////////////

FCLASS_ALIGN_PREFIX class CGroundCombat : public CAIThought
{
public:
	CGroundCombat(void);
	~CGroundCombat(void);
	virtual void Init(CAIBrain* pBrain, u8 uThoughtFlags, u8 uThoughtType, CAIMemorySmall* pParamPack = NULL);
	virtual void Cleanup(void);
	virtual void Work(void);

	// RTTI
	virtual BOOL SupportsClassInterface(const s32 nInterfaceId);
	virtual s32 GetClassInterface(void);

	static BOOL InitBank(s32 nHowMany);
	static void CleanupBank(void);
	static CAIThought* BankAccess(CAIThought*);
	static CNiBank<CGroundCombat> *s_pAllocBank;

	enum
	{
		FLAG_LASTKNOWNVEL_VALID				= 0x00000001,
		FLAG_LASTKNOWNSAFEPOS_VALID			= 0x00000002,
		FLAG_HASTARGETLOS					= 0x00000004,
		FLAG_HASEVERSEENENEMY				= 0X00000008,
		FLAG_CANTFINDENEMY					= 0x00000010,
		FLAG_NOPATH							= 0x00000020,	   //the unit is not currenly following a path
		FLAG_TRIGGERHAPPY					= 0x00000040,	   //Set this in ruleset, or tactic logic to make bot trigger happy for the current frame
		FLAG_TORSO_NO_WORK					= 0x00000080,	   //Set this in ruleset, or tactic logic CGroundCombat should not do any torso work
		FLAG_TORSO_DIR_FORWARD				= 0x00000100,	   //Set this in ruleset, or tactic logic
		FLAG_TORSO_DIR_ENEMY				= 0x00000200,	   //Set this in ruleset, or tactic logic
		FLAG_TORSO_DIR_ALTERNATE			= 0x00000400,	   //Set this in ruleset, or tactic logic
		FLAG_TORSO_SCAN						= 0x00000800,	   //Set this in ruleset, or tactic logic
		FLAG_HEAD_NO_WORK					= 0x00001000,	   //Set this in ruleset, or tactic logic
		FLAG_HEAD_DIR_FORWARD				= 0x00002000,	   //Set this in ruleset, or tactic logic
		FLAG_HEAD_DIR_ENEMY					= 0x00004000,	   //Set this in ruleset, or tactic logic
		FLAG_HEAD_DIR_ALTERNATE				= 0x00008000,	   //Set this in ruleset, or tactic logic
		FLAG_HEAD_SCAN						= 0x00010000,	   //Set this in ruleset, or tactic logic
		FLAG_TRIGGEROVERRIDE_STOPSHOOT		= 0x00020000,	   //Set this in ruleset, or tactic logic
		FLAG_PANIC_ON						= 0x00040000,
		FLAG_KAMIKAZEE						= 0x00080000,
		FLAG_STAYPUTBARRIER_BROKEN			= 0x00100000,	   //the bot's stayput barrier has been broken
		FLAG_STRATEGIC_ATTACK_PT_REACHED	= 0x00200000,	   //the bot has completed a path to an attack pt
		FLAG_BOT_WAS_DOING_SPECIAL_MOVE		= 0x00400000,
		FLAG_3DBOT							= 0x00800000,
		FLAG_FOLLOW_PATHS_WITH_PRECISION	= 0x01000000,
		FLAG_JUST_DID_SUCCESSFUL_MELEE		= 0x02000000,
		FLAG_JUST_DID_FAILED_MELEE			= 0x04000000,
		FLAG_DID_DAMAGE_DURING_SPECIAL_MOVE = 0x08000000,
		FLAG_MANUAL_MOVE_CONTROL			= 0x10000000,		//Call this if your ai ruleset, or tactic wants to move the mover without having a path
		FLAG_COMBAT_CANT_MOVE_BOT			= 0x20000000,
		FLAG_DISABLE_ENEMY_RELATED_EXIT_CONDITIONS = 0x40000000,  //call this in ruleset if CGroundCombat should never end the attack based on anything about the enemy
		FLAGS_TO_CLEAR_EVERY_FRAME = (FLAG_TRIGGERHAPPY | FLAG_TRIGGEROVERRIDE_STOPSHOOT | FLAG_HASTARGETLOS | FLAG_TORSO_NO_WORK | FLAG_HEAD_NO_WORK | FLAG_MANUAL_MOVE_CONTROL)
	};

	//
	// Tactics are often used by rulesets
	//	they can be configured with many parameters see AITactics.h
	//  The AI has only one tactic at a time
	//
	enum
	{
		TACTIC_NONE = 0,
		TACTIC_STANDGROUND,
		TACTIC_RANGEATTACK,
		TACTIC_CIRCLESTRAFE,
		TACTIC_PEEKABOO,
		TACTIC_SEARCH,
		TACTIC_RANGE3D,
		NUM_TACTICS,
	};

	//
	// RuleSetStates
	//	 RuleSets can have substates if desired.
	//		  RulesetState cb funcs are specified in .cpp file in a large table
	enum
	{
		ARS_0_BASE,
		ARS_0_HIDE,
		ARS_0_HIDEFROMDAMAGE,
		ARS_0_INVESTIGATEMARK,
		ARS_0_CHALLENGE,
		ARS_0_SEARCH,
		ARS_0_SITEGUNNER,
		ARS_0_PASSENGER,
		ARS_0_DRIVER,
		ARS_0_NUMSTATES,
		ARS_1_BASE						= ARS_0_NUMSTATES,
		ARS_1_NUMSTATES,
		ARS_NPC_BASE					= ARS_1_NUMSTATES,
		ARS_NPC_INVESTIGATEMARK,
		ARS_NPC_NUMSTATES,
		ARS_TITAN0_BASE					= ARS_NPC_NUMSTATES,
		ARS_TITAN0_INVESTIGATEMARK,
		ARS_TITAN0_SEARCH,
		ARS_TITAN0_NUMSTATES,
		ARS_PRED0_BASE					= ARS_TITAN0_NUMSTATES,
		ARS_PRED0_INVESTIGATEMARK,
		ARS_PRED0_SEARCH,
		ARS_PRED0_STRAFE,
		ARS_PRED0_NUMSTATES,
		ARS_PROBE0_BASE					= ARS_PRED0_NUMSTATES,
		ARS_PROBE0_NUMSTATES,
		ARS_RAT0_BASE					= ARS_PROBE0_NUMSTATES,
		ARS_RAT0_NUMSTATES,
		ARS_ELITEGUARD0_BASE			= ARS_RAT0_NUMSTATES,
		ARS_ELITEGUARD0_HIDEFROMDAMAGE,
		ARS_ELITEGUARD0_USECOVER,
		ARS_ELITEGUARD0_WATCHDOG,
		ARS_ELITEGUARD0_INVESTIGATEMARK,
		ARS_ELITEGUARD0_SEARCH,
		ARS_ELITEGUARD0_NUMSTATES,
		ARS_ZOMBIE0_BASE				= ARS_ELITEGUARD0_NUMSTATES,
		ARS_ZOMBIE0_QUITTER,
		ARS_ZOMBIE0_HIDEFROMDAMAGE,
		ARS_ZOMBIE0_SEARCH,
		ARS_ZOMBIE0_NUMSTATES,
		ARS_CORROSIVE0_BASE				= ARS_ZOMBIE0_NUMSTATES,
		ARS_CORROSIVE0_INVESTIGATEMARK,
		ARS_CORROSIVE0_SEARCH,
		ARS_CORROSIVE0_ATTACKLOCATION,
		ARS_CORROSIVE0_FINGERFLICK,
		ARS_CORROSIVE0_PEERHERE,
		ARS_CORROSIVE0_GETENEMYOFFME,
		ARS_CORROSIVE0_NUMSTATES,
		ARS_ZOMBIEBOSS_SEARCH			= ARS_CORROSIVE0_NUMSTATES,
		ARS_ZOMBIEBOSS_GRAB,
		ARS_ZOMBIEBOSS_SMASH,
		ARS_ZOMBIEBOSS_FISH,
		ARS_ZOMBIEBOSS_LURCH,
		ARS_ZOMBIEBOSS_GOTO,
		ARS_ZOMBIEBOSS_NUMSTATES,
		ARS_JUMPER0_BASE				= ARS_ZOMBIEBOSS_NUMSTATES,
		ARS_JUMPER0_NUMSTATES,
		ARS_SCIENTIST0_BASE				= ARS_JUMPER0_NUMSTATES,
		ARS_SCIENTIST0_NUMSTATES,
		ARS_NUM_TOTALSTATES				= ARS_SCIENTIST0_NUMSTATES
	};

	//work frame context data
	//initialized at top of every CGroundAttack::Work.
	//only valid until the end of that work
	static CGroundCombatWorkContext* s_pWD;

	CGenericWait* m_pLookAtMgr;							
	CGroundSearch *m_pSearchTactic;						
	
	CFVec3A m_LastEnemyMark;						
	CFVec3A m_LastKnownVel;								
	CFVec3A m_BadShotBonusUnit;							
	CFVec3A m_LastKnownSafePos;							
	CFVec3A m_EnemyMarkAtLastKnownSafeLoc;				
	CFVec3A m_TargPosAtLastTacticReset;					
	CFVec3A m_TacticOrigin;								
	CFVec3A m_LastAim;									
	CFVec3A m_ExposedPos;								
	CFVec3A m_DodgeInLoc;								
	CFVec3A m_AttackOrigin;								
	CFVec3A m_StrategicAttackPt;						
														
	CAttackPtSearch m_AttackPtSearch;					
	CCoverPtSearch m_CoverPtSearch;

	void ChangeEnemy(CEntity* pNewEnemy);
	CEntity *m_pEnemy;									
	CEntity *m_pLastKnownEnemyVehicle;							
	s32 m_nPossessionPlayerIndexOfEnemy;

	u32 m_uAttackFlags;									
	f32 m_fTimeWithoutLOS;						//how long consecutively enemy has been out of sight
	f32 m_fTimeWithLOS;									
	f32 m_fTimeWithoutTargetLock;
	f32 m_fTimeWithTargetLock;
	f32 m_fEnemyDamageTakenPerSec;						
	f32 m_fDamageDealtPerSec;							
	f32 m_fDamageTakenPerSec;							
	f32 m_fAttackThoughtInitTime;						
	f32 m_fEnemyMarkLastSmallGridChangeTime;	//divide world into a 2x2x2 grid, track movement on this grid
	f32 m_fUpdateBadShotBonusVecTime;					
	
	CAIMemoryLarge* m_pAttackSpecsParamPack;			
	CAIThought* m_pSubThought;						
	u16 m_uDamageSec;									
	u16 m_uLastPeekABooBailOut;							
	u16 m_uLostEnemyTime;								
	u16 m_uTacticInitTime;								
	u16 m_uRuleSetStateInitTime;
	u16 m_uLastPanicOnDelayTimeOut;						
	u16 m_uLastAlertOnDelayTimeOut;						
	u16 m_uBlindResponseToDamageTimeOut;				
	u16 m_uEnemyEvalTimeOut;
	u16 m_uAttackAfterKillingEnemyTimeOut;
	u16 m_uFireOddsTimeOut;
	u16 m_uEnemySelectTimeOut;
	u16 m_uLosStatusChangeCounter;			  //Number of time in this attack that the los-visiblity of the player has changed
	u8 m_uRuleSet;										
	u8 m_bPathJustCompleted;							
	s8 m_nLastPathReason;								
	u8 m_uAttackMoveSpeed;
	u8 m_uEnemyRace;		   //the race of my enemy she became my enemy


	//ruleset specific data
	u8 m_uARS_0_flags;		//findfix: move to parampack

	//Common Tactic Functionality
	void ResetCommonTacticVbls(void);
	f32 GetEnemyMoveDistSqThisTactic(void);
	f32 GetMoveDistSqThisTactic(void);

	//
	//  Dodging Locally
	//
	enum
	{
		DODGESTATE_IN = 0,
		DODGESTATE_OUT,
		NUM_DODGESTATES
	};
	u16 m_uDodgeTimeOut;								
	u16 m_uDodgeOutTimeOut;								
	u16 m_uDamageDodgeTimeOut;
	u8 m_uDodgeState;									
	u8 m_uDamageDodgeTimeMin;
	u8 m_uDodgeTimeMin;									
	u8 m_uDodgeTimeMax;									
	u8 m_uDodgeOutTimeMin;								
	u8 m_uDodgeOutTimeMax;								
	u8 m_uDodgePathFailureCount;
	void ResetDodge(void);
	void DoDodgeWork(BOOL bOkToRequestDodgePaths);	 //bOkToRequestPaths means it is O.K to look, Dodgework will NEVER assign a path if the mover is already on a path

	//
	// Stand Ground	Tactic
	//
	enum
	{
		STANDGROUNDSTATE_VEHICLESEARCH = 0,
		STANDGROUNDSTATE_LOCALATTACKPT,
		STANDGROUNDSTATE_ENEMYRELATTACKPT,
	};
	u8 m_uStandGroundState;								  
	
	//
	//  PeekABoo Tactic
	//
	enum
	{
		TCSTATE_COVER	= 0,
		TCSTATE_EXPOSED	= 1,
		TCSTATE_ATTACK_FROM_COVER = 2,
	};
	u16 m_uLastPeekABooSupriseTime;								
	u16 m_uAttackFromCoverTimeOut;						
	u8 m_uAttackFromCoverTimeMin;						//when attacking from cover, how long to stay exposed for (under normal conditions, ie. if not getting shot, or encroached apon, etc)
	u8 m_uAttackFromCoverTimeMax;						
	u8 m_uPeekABooState;								

	//
	// Range Tactic vbls
	//
	f32 m_fRangeMin;									
	f32 m_fRangeMax;									
	enum 
	{
		ERAPTUSEAGECTRL_NEVER,				 //range tactic will never do an ERAPT (enemy relative attack pt search)
		ERAPTUSEAGECTRL_TIMER_BASED,		 //range tactic will do an ERAPT basd on a timer
		ERAPTUSEAGECTRL_SMART,				 //range tactic will do an ERAPT whenever Line of sight is lost, or current position is otherwise ineffective
	};
	u8 m_uRangeTactic_EraptUseageCtrl;		//How and when will the range tactic look for and find enemy relative Attack Pts (ERAPTS)
	u8 m_uRangePathFailureCount;

	enum
	{
		RANGETACTICFLAG_NONE						= 0x00,
	};
	u8 m_uRangeTacticFlags;
	u16 m_uRangeTacticSmartEraptDecisionTimeOut;
	f32 m_fEraptLosIsLostWaitTime;


	//Rule Sets, Rule Set States and Tactics
	CFSM m_RuleSetFSM;									//manange the current rule set
	CFSM m_RuleSetStateFSM;								//manange the current rule set state
	CFSM m_TacticFSM;									//manange the current tactic
	void ChangeTactic(FSMStateHandle uTactic);
	void ChangeRuleSet(FSMStateHandle uRuleSet);
	void ChangeRuleSetState(FSMStateHandle uRuleSetState);
	cchar* GetTacticName(void);
	cchar* GetRuleSetName(void);
	cchar* GetRuleSetStateName(void);

	//
	// Taunt Timer
	//
	u16 m_uTauntTimeOut;
	u8 m_uTauntTimeMin;										 
	u8 m_uTauntTimeMax;										 
															 
	//
	// Melee Timer
	//
	u8 m_uMeleeTimeMin;										 
	u8 m_uMeleeTimeMax;										 
	u8 m_uMeleeRad;											 
	u16 m_uMeleeTimeOut;									 

	FINLINE void ClearTorsoFlags(void)			{ m_uAttackFlags &=~ (FLAG_TORSO_NO_WORK | FLAG_TORSO_DIR_FORWARD | FLAG_TORSO_DIR_ENEMY | FLAG_TORSO_DIR_ALTERNATE | FLAG_TORSO_SCAN);}
	FINLINE void ClearHeadFlags(void)			{ m_uAttackFlags &=~ (FLAG_HEAD_NO_WORK | FLAG_HEAD_DIR_FORWARD | FLAG_HEAD_DIR_ENEMY | FLAG_HEAD_DIR_ALTERNATE | FLAG_HEAD_SCAN);}

	//
	// Path's can be requested for various reasons
	//
	enum
	{
		SEARCHREASON_INVALID = -1,
		SEARCHREASON_TAKINGCOVER = 0,
		SEARCHREASON_LEAVINGCOVER,
		SEARCHREASON_NEWATTACKPOS,
		SEARCHREASON_DODGEOUT,
		SEARCHREASON_DODGEIN,
		SEARCHREASON_RETREAT,
		SEARCHREASON_RANGE,
		SEARCHREASON_PRE_MELEE,
		SEARCHREASON_STRAFE,
		NUM_SEARCHREASONS
	};

	//several paths can be "ready to go" at any one time.
	//attack logic can decide which one to use at any given time based
	//on the reason the path was requested
	//search results will be automatically kicked out
	//as the conditions for its usefulness fail. (either attacker or attackee move too far or simple timeout)
	//otherwise good search results can also just be purged manually for other reasons.
	enum
	{
		NUM_PATH_REQUEST_SLOTS = 4,
	};

	BOOL RequestSearch(const CFVec3A& WhereTo, s8 uReason, f32 fFudgeDestDist);
	void Attack_AssignPath(u8 uSlot, u32 uAssignPathFlags = 0);

	s8 FindFreeRequestSlot(void);
	void FreeRequestSlot(s32 nSlot);
	BOOL FindRequestSlotInUse(s8 nReason, u8* puSlot = NULL);
	BOOL IsSearchDone(s8 nReason, u8*puSlot = NULL);
	BOOL IsSearchFailed(s8 nReason, u8*puSlot = NULL);
	BOOL IsFollowingPath(BOOL bCheckReason = FALSE, s8 uReason = 0);
	void DoFollowPathWork(void);

	CSearchQuery m_aSearchQuery[NUM_PATH_REQUEST_SLOTS];					//structure used for interfacing with the path finder
	CSearchResults m_aSearchResults[NUM_PATH_REQUEST_SLOTS];		
	s8 m_anSearchReason[NUM_PATH_REQUEST_SLOTS];				
	s8 m_nCurPathRequestSlot;	//if following a path, this will be the slot of the currentpath


	enum
	{
		RELATIVE_DIR_RIGHT = 0,
		RELATIVE_DIR_LEFT,
		RELATIVE_DIR_TOWARD,
		RELATIVE_DIR_AWAY,
		NUM_RELATIVE_DIRS
	};
	BOOL FindEnemyRelPos_Ground(CFVec3A *pResult,
								const CFVec3A& EnemyPos,
								const CFVec3A& CurPos,
								f32 fRadius,
								f32 fYOff,
								u32 uRelDir,
								BOOL bCheckLOSToEnemy);
	BOOL FindEnemyRelPos_3D(CFVec3A *pResult,
								const CFVec3A& EnemyPos,
								const CFVec3A& CurPos,
								f32 fRadius,
								f32 fYOff,
								u32 uRelDir,
								BOOL bCheckLOSToEnemy);
	BOOL FindCurRelPos_Ground(CFVec3A *pResult,
								const CFVec3A& EnemyPos,
								const CFVec3A& CurPos,
								f32 fLocalDist,
								f32 fYOff,
								u32 uLocalDir,
								BOOL bCheckLOSToEnemy,
								f32 fMinDistToEnemy,
								f32 fMaxDistToEnemy);
	BOOL FindCurRelPos_3D(CFVec3A *pResult,
								const CFVec3A& EnemyPos,
								const CFVec3A& CurPos,
								f32 fLocalDist,
								f32 fYOff,
								u32 uLocalDir,
								BOOL bCheckLOSToEnemy,
								f32 fMinDistToEnemy,
								f32 fMaxDistToEnemy);
	BOOL FindLocalPoi_Ground(CFVec3A *pResult,
								CGraphPoi** pPoiToLockIfUsed,
								const CFVec3A& EnemyPos,
								const CFVec3A& CurPos,
								f32 fMinRad,
								f32 fMaxRad,
								f32 fYOff,
								BOOL bCheckLOSToEnemy);
	BOOL FindLocalPoi_3D(CFVec3A *pResult,
							CGraphPoi** pPoiToLockIfUsed,
							const CFVec3A& EnemyPos,
							const CFVec3A& CurPos,
							f32 fMinRad,
							f32 fMaxRad,
							f32 fYOff,
							BOOL bCheckLOSToEnemy);
	BOOL FindCoverPos_Ground(CFVec3A *pResult,
								const CFVec3A& EnemyPos,
								const CFVec3A& CurPos,
								f32 fXZRot,
								f32 fYOff,
								f32 fTestDist);
	BOOL FindCoverPos_3D(CFVec3A *pResult,
							const CFVec3A& EnemyPos,
							const CFVec3A& CurPos,
							f32 fXZRot,
							f32 fYOff,
							f32 fTestDist);
	BOOL FindRetreatPos_Ground(CFVec3A *pResult,
								const CFVec3A& From,
								const CFVec3A& LookAt,
								f32 fDist,
								f32 fYOff);
	BOOL FindRetreatPos_3D(CFVec3A *pResult,
							const CFVec3A& From,
							const CFVec3A& LookAt,
							f32 fDist,
							f32 fYOff);
	BOOL FindRangeAcquirePos_Ground(CFVec3A *pResult,
								const CFVec3A& EnemyPos,
								f32 fYOff);
	BOOL FindRangeAcquirePos_3D(CFVec3A *pResult,
							const CFVec3A& EnemyPos,
							f32 fYOff);
	BOOL FindAdvancePos_Ground(CFVec3A *pResult,
								const CFVec3A& From,
								const CFVec3A& LookAt,
								f32 fDist,
								f32 fYOff);
	BOOL FindAdvancePos_3D(CFVec3A *pResult,
							const CFVec3A& From,
							const CFVec3A& LookAt,
							f32 fDist,
							f32 fYOff);

	void LockPoi(CGraphPoi* pPoi);
	void ReleaseLockedPoi(void);
	CGraphPoi* m_pLockedPoi;
	BOOL IsLastSafePosUsefull(const CFVec3A& EnemyLoc);

	BOOL DoEnemyRelatedExitConditions(void);
	void DoGenericTauntWork(void);		  //some generic rules that kick of taunts based
	void DoGenericEnviroReactRules(BOOL bDodgeGrenades, BOOL bDodgeVehicles, BOOL bDamageBTAReaction);	 //some generic rules control reaction to environmental happenings
	void CombatMode_PlayBTA(cchar* pszBTAEvent, BOOL bStopFirst, BOOL bLookFirst);

protected:
	
	virtual BOOL WhereIsEnemy(CAIMemoryLarge* pSeenMem,
			    				 CAIMemoryLarge* pHeardMem,
								 CAIMemoryLarge* pDamagedByMem,
								 CFVec3A* pEnemyMountPos,
								 BOOL *pbHasLOSThisFrame);
	void InitRuleSet(void);
	void AttackWorkBeginFrame(void);
	void DoRuleSet(void);
	void DoTorsoAndHeadWork(void);
	void DoWeaponAimAndFireWork(void);
	void AttackWorkEndFrame(void);

	//custom aim and fire work functions (findfix: make the cb funcs, like rulestate functions)
	void DoWeaponAimAndFireWork_Generic(f32 fCheckLosFromMuzzleDist = 0.0f, BOOL bDoAimLogic = TRUE,  BOOL bCheckForFriendsBlockingWay = TRUE);
	void DoWeaponAimAndFireWork_Grunt(void);
	void DoWeaponAimAndFireWork_SiteGunner(void);
	void DoWeaponAimAndFireWork_Predator(void);
	void DoWeaponAimAndFireWork_Snarq(void);
	void DoWeaponAimAndFireWork_Titan(void);
	void DoWeaponAimAndFireWork_Probe(void);
	void DoWeaponAimAndFireWork_Sentinel(void);
	void DoWeaponAimAndFireWork_Rat(void);
	void DoWeaponAimAndFireWork_Loader(void);
	void DoWeaponAimAndFireWork_RatGun(void);
	void DoWeaponAimAndFireWork_EliteGuard(void);
	void DoWeaponAimAndFireWork_Corrosive(void);
	void DoWeaponAimAndFireWork_ZombieBoss(void);
	void DoWeaponAimAndFireWork_Jumper(void);
	void DoWeaponAimAndFireWork_Scientist(void);
	void Titan_RocketAiming(void);
	void DoLagWeaponAiming(u8 uWeaponId);   //uWeaponId is used for BotMover->GetWeaponCtrlPtr()
	void DoLeadAimWithError(f32 fMaxErrorDist, f32 fBaseAccuracy, const CFVec3A& ErrorOrigin);



	FCLASS_STACKMEM_ALIGN(CGroundCombat);
} FCLASS_ALIGN_SUFFIX;

#define CHECK_LOS_TO_ENEMY_MARK TRUE				  //go with find attack pos
#define DONT_CHECK_LOS_TO_ENEMY_MARK FALSE

#define OK_TOREQUESTENEMYRELATTACKPTPATHS TRUE
#define NOT_OK_TOREQUESTENEMYRELATTACKPTPATHS FALSE
#define SEARCH_IS_SUCCESS_ON_PATHCOMPLETE TRUE
#define NOT_SEARCH_IS_SUCCESS_ON_PATHCOMPLETE FALSE

BOOL _LocalPoiSearchWork(CGroundCombat* pT, BOOL *pbNewPtReached);
void _EnemyRelAttackPtWork(CGroundCombat* pT, BOOL bOkToRequestEnemyRelAttackPtPaths, BOOL bSearchIsSuccessOnPathComplete, BOOL *pbNewPtReached = NULL);
void _InitCoverPtSearch(CCoverPtSearch* pS, BOOL bPanic);
void _AdvanceCoverPtSearch(CCoverPtSearch* pS, BOOL bLastSearchSuccessful, BOOL bPanic);

typedef	BOOL (CGroundCombat::*PFindEnemyRelPos_Func)(CFVec3A *pResult, const CFVec3A& EnemyPos, const CFVec3A& CurPos,  f32 fRadius, f32 fYOff, u32 uRelDir, BOOL bCheckLOSToEnemy);
typedef	BOOL (CGroundCombat::*PFindCurRelPos_Func)(CFVec3A *pResult, const CFVec3A& EnemyPos, const CFVec3A& CurPos,  f32 fLocalDist, f32 fYOff, u32 uLocalDir, BOOL bCheckLOSToEnemy, f32 fMinDistToEnemy, f32 fMaxDistToEnemy);
typedef	BOOL (CGroundCombat::*PFindLocalPoi_Func)(CFVec3A *pResult, CGraphPoi** pPoiToLockIfUsed, const CFVec3A& EnemyPos, const CFVec3A& CurPos,  f32 fMinRad, f32 fMaxRad, f32 fYOff, BOOL bCheckLOSToEnemy);
typedef	BOOL (CGroundCombat::*PFindCoverPos_Func)(CFVec3A *pResult, const CFVec3A& EnemyPos, const CFVec3A& CurPos, f32 fXZRot, f32 fYOff, f32 fTestDist);
typedef	BOOL (CGroundCombat::*PFindRetreatPos_Func)(CFVec3A *pResult, const CFVec3A& From, const CFVec3A& LookAt, f32 fDist, f32 fYOff);
typedef	BOOL (CGroundCombat::*PFindAdvancePos_Func)(CFVec3A *pResult, const CFVec3A& From, const CFVec3A& LookAt, f32 fDist, f32 fYOff);
typedef	BOOL (CGroundCombat::*PFindRangeAcquirePos_Func)(CFVec3A *pResult, const CFVec3A& EnemyPos,  f32 fYOff);

//
// Context Class, for data that is calculated once per frame per Attack Work function.
//
class CGroundCombatWorkContext
{
public:
	CGroundCombatWorkContext(void) : uLastFrameNowTime(1) {}

	void Init(CGroundCombat* pT);

	CFVec3A EnemyMark;				  //enemy mark is that last place that I perceeived the player to be (Saw, heard, damagedby etc)
	CFVec3A EnemyMarkTagPos;
	CFVec3A DeltaToEnemyMarkUnit;
	CAIMemoryLarge* pSeenMem;
	CAIMemoryLarge* pHeardMem;
	CAIMemoryLarge* pDamagedByMem;
	CAIMemoryLarge* pDamageDealtMem;
	CAIMemoryLarge* pEnemyDamagedByMem;
	BOOL bHasLOSThisFrame;
	CAIMover* pMover;
	u16 uLastFrameNowTime;
	u16 uNowTime;
	f32 fNowTime;
	f32 fAlignment;
	f32 fDistToEnemyMark;			 //how far to the enemy mark (calc'd every frame and cached)
	f32 fDistToEnemyMarkXZ;
	BOOL bLostTheEnemy;
	BOOL bSensesDanger;
	f32 fAggression;
	f32 fIntelligence;
	f32 fCourage;
	CGraphPoi* pEnemyPoi;
	f32 fDistToEnemyPoi;
	CBot* pEnemyBot;				//if the enemy is a bot, this will be a pointer to it.
	CBot* pBot;						//if m_pBrain->GetAIMover()->GetEntity()->TypeBits() & ENTITY_BIT_BOT this will be set to it

	//these only valid if pT->m_pAttackSpecsParamPack is non NULL
	u8 uNormalRuleSet;					// which ruleset the brain will use when attacking
	u8 uAlternateRuleSet;
	u8 uAlternateRuleSetOdds;			// (0-100) pct chance that the alternate will be used
	u8 uAttackSpeedMin;					// (0-100)
	u8 uAttackSpeedMax;					// (0-100)
	u16 uMaxDistFromOrigin;
	u16 uStayPutDist;

	PFindEnemyRelPos_Func pFindEnemyRelPos_Func;
	PFindCurRelPos_Func pFindCurRelPos_Func;
	PFindLocalPoi_Func pFindLocalPoi_Func;
	PFindCoverPos_Func pFindCoverPos_Func;
	PFindRetreatPos_Func pFindRetreatPos_Func;
	PFindAdvancePos_Func pFindAdvancePos_Func;
	PFindRangeAcquirePos_Func pFindRangeAcquirePos_Func;
};

#endif //_AITHOUGHTSGROUND_H_
