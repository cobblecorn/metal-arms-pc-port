#ifndef _AICORROSIVE_H
#define _AICORROSIVE_H 1


#include "AIThoughtsGround.h"

//
//  CCorrosiveCombat
//
///////////////////////
//
// Parameters 
//
//	ParamPackSize: Large
//
//	CAIMemoryLarge::m_pEntity:	CEntity *m_pEnemy
//	 - f32      CloseEnoughRadius
//	 - f32      Time
//
///////////////////////

enum AIBehaviorReason_e {
	AI_BEHAVIOR_REASON_LOST_LOS,
	AI_BEHAVIOR_REASON_ATTACK_COMPLETE,
	AI_BEHAVIOR_REASON_FINGERFLICK_COMPLETE,
	AI_BEHAVIOR_REASON_PEER_COMPLETE,
};

// BehaviorRequestCallback -- Callback function that is supposed
// to return TRUE if it commands an overridden behavior.
typedef BOOL BehaviorRequestCallback_t( AIBehaviorReason_e eReason );
typedef void (*AttackCompleteCallback_t)(u32 nUserData);

FCLASS_ALIGN_PREFIX class CCorrosiveCombat : public CGroundCombat
{
//----------------------------------------------------------------------------------------------------------------------------------
// Public Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	// These strategies only apply when in the BASE active rule set
	enum BaseStrategyState_e {
		BASE_STRATEGY_STATE_MELEE_ATTACK,	// Get into melee attack range and Melee away
		BASE_STRATEGY_STATE_RANGE_ATTACK,	// Put some distance between the enemy and fire away with rockets

		BASE_STRATEGY_STATE_NONE			// No current strategy, or NOT IN THE BASE STATE.
	};

	enum AIOverrideState_e {
		OVERRIDE_STATE_NONE = 0,
		OVERRIDE_STATE_ATTACK_LOCATION,
		OVERRIDE_STATE_FINGER_FLICK,
		OVERRIDE_STATE_PEER_HERE,
	};

	enum AttackLocationState_e {
		ATTACKLOCATION_STATE_GOTOATTACKPOINT = 0,
		ATTACKLOCATION_STATE_FACEATTACKPOINT,
		ATTACKLOCATION_STATE_ISSUEATTACK,
		ATTACKLOCATION_STATE_WAITFORATTACKTOCOMPLETE,
		ATTACKLOCATION_STATE_ATTACKCOMPLETE,
	};

	enum FingerFlickState_e {
		FINGERFLICK_STATE_GOTOATTACKPOINT=0,
		FINGERFLICK_STATE_FACEATTACKPOINT,
		FINGERFLICK_STATE_ISSUEATTACK,
		FINGERFLICK_STATE_WAITFORATTACKTOCOMPLETE,
		FINGERFLICK_STATE_ATTACKCOMPLETE,
	};

	enum PeerHereState_e {
		PEERHERE_STATE_GOTOPOINT,
		PEERHERE_STATE_FACEPEERPOINT,
		PEERHERE_STATE_ISSUEPEER,
		PEERHERE_STATE_WAITFORPEERTOCOMPLETE,
		PEERHERE_STATE_PEERCOMPLETE,
	};

	enum PreferredMeleeAttack_e {
		PREFERED_MELEE_STOMP = 0,
		PREFERED_MELEE_SWIPE,

		PREFERED_MELEE_COUNT
	};

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

	// aim/attack variables
	f32 m_fTimeSinceLastFiredRockets;
	f32 m_fTimeSinceLastTriedToAttack;
	u32 m_nNextTagPointToLOSCheck;

	// Base state variables
	f32 m_fTimeInCurrentStrategyState;
	BaseStrategyState_e m_eBaseStrategyState;
	CFVec3A m_vCachedEnemyMark;
	BOOL m_bGoDirectlyToRangeAttack;

	// Emotional variables
	f32 m_fTimeSinceLastBeatChest;
	f32 m_fTimeSinceLastCheckedToBeatChest;
	BOOL m_bRoaredBecauseNoLOS;
	BOOL m_bAllowRandomRoar;
	BOOL m_bPlayLostLimbOuch;
	f32 m_fTimeSinceOuchBTAPlayed;

	// Bot Toss variables
	f32 m_fTimeSinceLastBotToss;
	f32 m_fTimeSinceLastCheckedToTossBots;

	static BehaviorRequestCallback_t *m_pBehaviorRequestCallbackFcn;
	const static f32 m_fMeleeRange[PREFERED_MELEE_COUNT];

	static BOOL m_bFreeToFireRockets; // If TRUE, then corrosive is allowed to fire rockets, else he can't

	// Search ruleset variables
	f32 m_fSearchTimeSinceRequestedHint;

	// Command Override Variables
	BOOL m_bNewOverrideState;				// Set to true when an external override command comes in.
	BOOL m_bAllowEnemyLOSToAbortOverride;	// Set to allow LOS with enemy bot to abort overridden behavior and revert back to attack mode.
	AIOverrideState_e m_eOverrideState;		// Short Circuits normal AI behavior and commands AI to do something specific
	AttackLocationState_e m_eAttackLocationState;
	FingerFlickState_e m_eFingerFlickState;
	CFVec3A m_vOverrideAttackLocation;
	CFSphere m_FingerFlickSphere;
	u32 m_nCallbackUserData;
	AttackCompleteCallback_t m_pAttackCompleteCallback;
	AttackCompleteCallback_t m_pRoarCallback;
	AttackCompleteCallback_t m_pRangeNoRocketsCallback;
	PreferredMeleeAttack_e m_ePreferedMeleeAttack;
	
	// Peer Here variables...
	PeerHereState_e m_ePeerHereState;
	CFVec3A m_vPeerHereLocation;

	// Pathing variables
	s8 m_uCurrPathReason;
	s8 m_uNextPathReason;
	CFVec3A m_vPathDestination; // Do not modify this value directly!  Use the PathSetDestination() public function!
	BOOL m_bNewPathDestination;	// Defines if a new path destination value has been specified.
	f32 m_fTimeSpentOnPath;

	// Enemy on me tracking variables
	f32 m_fTimeEnemyOnMe;

	// Investigate mark variables
	u32 m_uIMPC;				// Investigate Mark Point Counter... from 0 to 11
	f32 m_fIMPD;				// Investigate mark point distance...  from 20 to 100

//----------------------------------------------------------------------------------------------------------------------------------
// Private Data:
//----------------------------------------------------------------------------------------------------------------------------------
private:

	static CCorrosiveCombat* m_pCorrosiveCombat;
	static f32 m_aSinTable[ 24 ];
	static f32 m_aCosTable[ 24 ];

	static BOOL m_bRoarThisFrame;
	static BOOL m_bLaughThisFrame;
	static BOOL m_bFreeToBotToss;	  // If TRUE, then corrosive is allowed to toss bots.

	//Aggressiveness: 0.0f = half asleep, 1.0f = default, 2.0f = very aggressive.
	static BOOL m_bUseAlternateAggressionCalculations;
	static f32 m_fAggressiveness;
	static f32 m_fFireDelayMul;
	static f32 m_fFireOddsMul;
	static f32 m_fNormRunSpeed;
	static f32 m_fRunSpeed;

	static BOOL m_bForceRangeAttack; // Used to command Corrosive to not even try to issue a melee attack.

//----------------------------------------------------------------------------------------------------------------------------------
// Public Functions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	CCorrosiveCombat(void);
	~CCorrosiveCombat(void);
	virtual void Init(CAIBrain* pBrain, u8 uThoughtFlags, u8 uThoughtType, CAIMemorySmall* pParamPack = NULL);
	virtual void Cleanup(void);
	virtual void Work(void);

	// RTTI
	virtual BOOL SupportsClassInterface(const s32 nInterfaceId);
	virtual s32 GetClassInterface(void);

	static BOOL InitBank(s32 nHowMany);
	static void CleanupBank(void);
	static CAIThought* BankAccess(CAIThought*);
	static CNiBank<CCorrosiveCombat> *s_pAllocBank;

	// Firing functions
	void DoWeaponFireWork_Rocket( void );
	void DoWeaponFireWork_Melee( void );
	BOOL HasClearRocketShot( void );

	// Bot Toss functions
	FINLINE static void AllowBotToss( BOOL bAllow ) { m_bFreeToBotToss = bAllow; }
	void BotTossWork( void );
	void ClearBotTossTimers( void );

	// External AI Behavior override functions.
	static FINLINE BOOL IsBehaviorRequestCallbackRegistered( void )							{ return m_pBehaviorRequestCallbackFcn != NULL; }
	static FINLINE void SetBehaviorRequestCallback( BehaviorRequestCallback_t *pCallback )	{ m_pBehaviorRequestCallbackFcn = pCallback; }

	FINLINE static void FreeToFireRockets( BOOL bFreeToFireRockets ) { m_bFreeToFireRockets = bFreeToFireRockets; }
	FINLINE static void SetRoarThisFrame( BOOL bRoar ) { m_bRoarThisFrame = bRoar; }
	FINLINE static void SetLaughThisFrame( BOOL bLaugh ) { m_bLaughThisFrame = bLaugh; }
	static void AttackThisLocation( CFVec3A *pLocationToAttack, BOOL bLOSOverride );
	static void PeerAtThisLocation( CFVec3A *pLocationToPeerAt, BOOL bLOSOverride );
	static void DoFingerFlick( const CFSphere& rSphere, u32 nUserData, AttackCompleteCallback_t pAttackCallback );
	static void CancelFingerFlick();
	static void CancelPeer(BOOL bForce=FALSE);
	static void SetPreferredMeleeAttack(PreferredMeleeAttack_e eAttack);
	static void SetRoarCallback(AttackCompleteCallback_t pRoarCallback);
	//This callback is for when Corrosive is in ranged attack mode but cannot fire rockets.
	static void SetRangeNoRocketsCallback(AttackCompleteCallback_t pRangeNoRocketsCallback);
	
	// Pathing variables
	void HandlePathing( BOOL bStop = FALSE ); // For internal use only.  Do not call externally
	void PathSetDestination( CFVec3A *pNewDestination );
	BOOL FindInvestigatePt( void );

	//Aggressiveness settings, 0.0f = half asleep, 1.0f = default, 2.0f = mad with fury.
	FINLINE static void UseAlternateAggressionCalculations( BOOL bUseAlternate ) { m_bUseAlternateAggressionCalculations = bUseAlternate; }  // FALSE == RUSS's calculations, TRUE = JEREMEYS calculations
	static void SetAggressiveness(f32 fAggressive);
 	FINLINE static f32 GetAggressiveness() { return m_fAggressiveness; }

	// Used when corrosive is in his Base attack ruleset, if this is true, he will never issue a melee attack
	static void ForceRangeAttack( BOOL bForceRangeAttack ) { m_bForceRangeAttack = bForceRangeAttack; }
	static BOOL IsRangeAttackForced( void ) { return m_bForceRangeAttack; }
	
	FINLINE static CGroundCombat *GetGroundCombatPtr() { return m_pCorrosiveCombat; }

	void IsEnemyOnMeWork( void );

	// Emotions
	void DoCorrosiveTauntWork( void );
	void DoOuchWork( void );
	void BeatChestWork( void );
	void DoAggressionWork( void );

//----------------------------------------------------------------------------------------------------------------------------------
// Protected Functions:
//----------------------------------------------------------------------------------------------------------------------------------
protected:

	BOOL WhereIsEnemy(CAIMemoryLarge* pSeenMem,
								 CAIMemoryLarge* pHeardMem,
								 CAIMemoryLarge* pDamagedByMem,
								 CFVec3A* pEnemyMountPos,
								 BOOL *pbHasLOSThisFrame);

//----------------------------------------------------------------------------------------------------------------------------------
// Private Functions:
//----------------------------------------------------------------------------------------------------------------------------------
private:

	FCLASS_STACKMEM_ALIGN(CCorrosiveCombat);
} FCLASS_ALIGN_SUFFIX;

#endif