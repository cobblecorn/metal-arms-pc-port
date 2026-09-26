#ifndef _AITHOUGHTSGENERIC_H_
#define _AITHOUGHTSGENERIC_H_ 1

#include "AIThought.h"
#include "AIGraphSearcher.h"
#include "fmath.h"

class CAIBrain;
class CEntity;
class CESpline;
class CPatrolPath;
class CPatrolZone;
class CAIMemoryLarge;



//
// Ground Wait
//
///////////////////////
//
// ParamPack
//
//	  - Large
///////////////////////
FCLASS_ALIGN_PREFIX class CGenericWait : public CAIThought
{
public:
	CGenericWait(void) : CAIThought(),
						m_pLookAtObj(NULL),
						m_uNextLookAround(0),
						m_uLookAroundChance(0),
						m_uLookAroundChanceFreqMin(25),
						m_uLookAroundChanceFreqMax(35)
						{
							m_LookAtLoc.Zero();
							m_CurLookAroundLoc.Zero();
						};
	virtual void Init(CAIBrain* pBrain, u8 uThoughtFlags, u8 uThoughtType, CAIMemorySmall* pParamPack = NULL);
	virtual void Cleanup(void);
	virtual void Work(void);

	// RTTI
	virtual BOOL SupportsClassInterface(const s32 nInterfaceId);
	virtual s32 GetClassInterface(void);

	static BOOL InitBank(s32 nHowMany);
	static void CleanupBank(void);
	static CAIThought* BankAccess(CAIThought*);
	static CNiBank<CGenericWait> *s_pAllocBank;

	enum
	{
		NUM_VISRAYS = 8,		//I know of at least two places that assume NUM_VISRAYS is a power of 2
	};
	static CFVec3A s_aVisRay[NUM_VISRAYS];
	static BOOL s_bVisRaysInitialized;
	static void InitializeVisRays(void);
	static u8 GetClosestVisRay_Pt(const CFVec3A& Pt, const CFVec3A& Origin);
	static u8 GetClosestVisRay_Vec(const CFVec3A& Vec);
	static const CFVec3A& VisRayToLookAtLoc(u8 uVisRay, const CFVec3A& Origin,  f32 fScanDist);
	
	CFVec3A m_LookAtLoc;
	CFVec3A m_CurLookAroundLoc;
	CEntity* m_pLookAtObj;

	u16	m_uNextLookAround;			// Time of the next look-around diceroll
	u8 m_uLookAroundChanceFreqMin;	// (0-255) Minimum between look-around-chances
	u8 m_uLookAroundChanceFreqMax;	// (0-255) Maximum between look-around-chances
	u8 m_uLookAroundChance;			// (0-100) Odds of a look around happening at every look-around-chance
	enum
	{
		VISRAY_UNSET = 0x00,
		VISRAY_FAILED = 0x01,
		VISRAY_PASSED = 0x02,
	};
	enum
	{
		INVALID_VISRAY = NUM_VISRAYS+1,
	};

	enum
	{
		NUM_VISRAYS_IN_MAX_RANGE  = 2,
	};

	void ResetVisRayScan(void);
	void UpdateVisRays(const CFVec3A& Origin, f32 fScanDist);
	void ForceLookAtWhileScanning(const CFVec3A& Pt);

	BOOL IsVisRayScanPaused(void);
	void DoTorsoScanWork(void);
	void DoVisRayScan(void);

	u8	m_auScannerStatus[NUM_VISRAYS];
	u8	m_uCurScanner;
	u8	m_uCurScanRangeCenterMark;
	s8	m_nCurScanDir;
	u8	m_uCurScan;
	u8	m_uLastScan;
	u8	m_uDirChange;
	s8	m_uScansSinceDirChange;
	u8	m_uLOSFrameDelay;
	u8	m_uDirChangeCount;
	u8	m_uPausedVisRayScanMin;			//min length of a VisRayScan pause
	u8	m_uPausedVisRayScanMax;			//max length of a VisRayScan pause
	u32 m_uNextLOSFrameTimeOut;
	f32	m_fPausedVisRayScanTimeOut;

	//torso scanning
	f32 m_fTorsoScanTimer;
	f32	m_fPausedTorsoScanTimeOut;
	u8	m_uPausedTorsoScanMin;			//min length of a TorsoScan pause
	u8	m_uPausedTorsoScanMax;			//max length of a TorsoScan pause
	u8  m_uPausedTorsoScanOdds;

	enum
	{
		WAIT_NONE								= 0x00000000,
		WAIT_LOOKAT_WAS_SPECIFIED				= 0x00000001,		//When not looking around, look at either m_LookAtLoc or m_pLookAtObj

		WAIT_LOOKING_AROUND						= 0x00000002,
		WAIT_USE_MIN_LOS_DIST					= 0x00000004,		//when looking around, make sure your not looking at a wall or something.

		WAIT_AUTOSCAN_TORSO						= 0x00000008,
		WAIT_AUTOSCAN_TORSO_WHILE_WALKING		= 0x00000010,
		WAIT_AUTOSCAN_TORSO_WHILE_STANDING		= 0x00000020,

		WAIT_VISRAYS_ENABLED					= 0x00000040,		//maintain a set of visibility ray tests in 8 compass directions
		WAIT_SMART_UPDATE_VISRAYS				= 0x00000080,		//update the visrays once when the bot has been stopped for a while
		WAIT_TIMER_UPDATE_VISRAYS				= 0x00000100,		//one visrays will be re-tested on a timer of m_uLOSFrameDelay frames

		WAIT_TORSO_SCAN_DIR_POS					= 0x00000200,		//internal vbl
		WAIT_ATTEMPTED_INITVEHICLEACCESS		= 0x00000400,		//internal vbl

	};
	u32 m_uWaitFlags;
	u16 m_uAutoVisRayUpdateTimeOut;

	void BeginLookAround(void);
	void ForceTakeLookAroundChance(void);
protected:
	void SetNextLookAround(void);
	BOOL TakeLookAroundChance(void);
	FCLASS_STACKMEM_ALIGN(CGenericWait);
} FCLASS_ALIGN_SUFFIX;
extern const f32 _WAIT_VISRAYSCAN_LOS_DIST;	   //pMover->GetRadiusXZ() is added to this to determine length of vis ray



//
//  Generic Wander
//
///////////////////////
//
// ParamPack Useage  : Large
//
//	 - CAIMemoryLarge::m_fLargeData		: Radius 
//	 - CAIMemorySmall::m_uSmallData		: StayInRoom
//	 - CAIMemoryLarge::m_Loc.x			: PauseFrequency(Min/Max)  Seconds  (will be randomized slightly, but never more than this)
//   - CAIMemoryLarge::m_Loc.y				
//	 - CAIMemoryLarge::m_EntityLoc.x	: SpeedRange(Min/Max)	   Seconds  (will be randomized slightly, but never more than this)
//	 - CAIMemoryLarge::m_EntityLoc.y	
//	 - CAIMEmoryLarge::m_pEntity   (u16): m_uTakeBreakChance
//
///////////////////////
FCLASS_ALIGN_PREFIX class CGenericWander : public CAIThought
{
public:
	CGenericWander(void) :	CAIThought(),
							m_uWanderFlags(0)
							{
								m_WanderOrigin.Zero();
								m_WanderGoal.Zero();
							};
	virtual void Init(CAIBrain* pBrain, u8 uThoughtFlags, u8 uThoughtType, CAIMemorySmall* pParamPack = NULL);
	virtual void Cleanup(void);
	virtual void Work(void);

	// RTTI
	virtual BOOL SupportsClassInterface(const s32 nInterfaceId);
	virtual s32 GetClassInterface(void);

	static BOOL InitBank(s32 nHowMany);
	static void CleanupBank(void);
	static CAIThought* BankAccess(CAIThought*);
	static CNiBank<CGenericWander>* s_pAllocBank;

	enum
	{
		WANDERSTAGE_NEEDS_PATH  = 0,
		WANDERSTAGE_WAITINFOR_PATH,
		WANDERSTAGE_EN_ROUTE,
		WANDERSTAGE_START_TAKING_A_BREAK,
		WANDERSTAGE_TAKING_A_BREAK,
	};

	enum
	{
		WANDER_SEARCHRESULTTOGGLE		= 0x0001,	  //please keep as 1, makes toggle code easier
		WANDER_IGNORERADIUSLIMIT		= 0x0002,
		WANDER_3DBOT					= 0x0004,
		WANDER_STAYINROOM				= 0x0008,
		WANDER_TURNEDAROUND_THISFRAME	= 0x0010,		//cleared at top of every frame
	};

	CFVec3A			m_WanderOrigin;
	CFVec3A			m_WanderGoal;
	CSearchQuery	m_SearchQuery;
	CSearchResults	m_SearchResults[2];				//2 search results so that we can be working on one while using another. Check wander flags (WANDER_SEARCHRESULTTOGGLE) to determine the one in use
	f32				m_fWanderRadius;
	u16				m_uWanderFlags;
	u16				m_uReturnToWorkTime;
	u16				m_uTakeBreakDecisionTimeOut;	// internal vbl, used to manage how often break decision is made and wanderlookat points decision is made
	u16				m_uLastGoalVertId;
	u16				m_uLastLastGoalVertId;
	u8				m_uStage;
	u8				m_uTakeBreakChance;
	u8				m_uSociable;					// (0-100)	How likely they are to stop and talk to other bots
	u8				m_uCuriosity;					// (0-100)	How likely they are to stop and look around at pretty things
	u8				m_uFailedEdgeCount;
	u8				m_uWanderPt;
	u8				m_uWanderPtCounter;
	u8				m_uSpeedPctMin;
	u8				m_uSpeedPctMax;

protected:
	CESpline* m_pWanderPts;		// This entity contains some points to wander amongst
	BOOL Find3DGraphWanderPt(CFVec3A* pWanderLoc);
	BOOL FindXZGraphWanderPt(CFVec3A* pWanderLoc);
	BOOL FindGraphWanderPt(CFVec3A* pWanderLoc);
	BOOL FindWanderPtFromPtArray(CFVec3A* pWanderLoc);
	void FindLookAtPtWhileTakingABreak(CFVec3A* pLookAt);

	FCLASS_STACKMEM_ALIGN(CGenericWander);
} FCLASS_ALIGN_SUFFIX;


//
//  GenericPatrol
//
///////////////////////
//
// ParamPack Useage  : Large
//
//						cchar *pszPathName,				// String table Name of espline entity that contains the path points
//						u8 uPatrolSpeed,				// (0-100)	Pct of speed patrolling unit should travel at
//						u8 uDirection,					// (0,1)	0= Clockwise, 1 = CounterClockWise
//						u8 uNumIgnoreZones,				// how many elements in the next parameter
//						cchar** paszIgonoreZones)		// array of names of patrol zones to ignore

FCLASS_ALIGN_PREFIX class CGenericPatrol : public CAIThought
{
public:
	// flags  see m_uPatrolFlags
	enum
	{
		PATROLFLAG_NONE			= 0x0000,
		PATROLFLAG_ONPATH		= 0x0001,
		PATROLFLAG_PINGPONG		= 0x0002,	// Path's that aren't closed are assumed to be ping-pong paths
		PATROLFLAG_REVERSE		= 0x0004,	// Begin pingpong in reverse, or Follow the looping path in revers of the order in the pointArray(which is the order knots were made in 3ds max)
		PATROLFLAG_3DBOT		= 0x0008,
	};

	//stages
	enum
	{		  //see m_uPatrolStage
		PATROLSTAGE_OFF_PATH_NEEDS_PATH  = 0,
		PATROLSTAGE_OFF_PATH_WAITINFOR_PATH,
		PATROLSTAGE_OFF_PATH_EN_ROUTE,
		PATROLSTAGE_PATROLPATH_INIT,
		PATROLSTAGE_ON_PATH_EN_ROUTE,
	};

	CGenericPatrol(void) :	CAIThought(),
							m_uPatrolFlags(PATROLFLAG_NONE),
							m_pPatrolPts(NULL),
							m_uPatrolStage(PATROLSTAGE_OFF_PATH_NEEDS_PATH),
							m_uPatrolSpeed(0),
							m_pPatrolPath(NULL)
							{
							}
	virtual void Init(CAIBrain* pBrain, u8 uThoughtFlags, u8 uThoughtType, CAIMemorySmall* pParamPack = NULL);
	virtual void Cleanup(void);
	virtual void Work(void);

	// RTTI
	virtual BOOL SupportsClassInterface(const s32 nInterfaceId);
	virtual s32 GetClassInterface(void);

	static BOOL InitBank(s32 nHowMany);
	static void CleanupBank(void);
	static CAIThought* BankAccess(CAIThought*);
	static CNiBank<CGenericPatrol>* s_pAllocBank;

	
	void EnterZone(CPatrolZone* pZone);
	void ExitZone(CPatrolZone* pZone);

	CSearchQuery m_SearchQuery;			// path finder: 
	CSearchResults m_SearchResults;		// path finder: used for each link along the way
	CESpline* m_pPatrolPts;				// This entity contains the points to go between
	CPatrolPath* m_pPatrolPath;
	CEntity* m_pLookAtObj;
	u16	m_uPatrolFlags;

	u8 m_uStartPatrolPt;
	u8 m_uPatrolStage;
	u8 m_uPatrolSpeed;
	u8 m_uZoneSpeed;
	u8 m_uWaitZoneTimeout;

	// Zone Behavior Data
	CNiList<cchar*> m_IgnoreZoneList;
	// Is unit in any zone
	CPatrolZone* m_pCurSpeedZone;
	CPatrolZone* m_pCurWaitZone;
	CPatrolZone* m_pCurLookZone;
	CPatrolZone* m_pCurAnimZone;
	CGenericWait m_WaitZoneData;

protected:

	void EnterSpeedZone(CPatrolZone* pZone);
	void ExitSpeedZone(CPatrolZone* pZone);
	void EnterWaitZone(CPatrolZone* pZone);
	void ExitWaitZone(CPatrolZone* pZone);
	void EnterLookZone(CPatrolZone* pZone);
	void ExitLookZone(CPatrolZone* pZone);
	void EnterAnimZone(CPatrolZone* pZone);
	void ExitAnimZone(CPatrolZone* pZone);
	
	FCLASS_STACKMEM_ALIGN(CGenericPatrol);
} FCLASS_ALIGN_SUFFIX;


//
//  Ground Goto
//
///////////////////////
//
// Parameters 
//
//	ParamPackSize: Large
//
//	const CFVec3A& GotoLoc		// Where
//	const CFVec3& LookAtLoc		// A pt in world space to aim torso and head at.(only used if uLookAtObjGUID == 0 and GOTOFLAG_LOOKAT specified as with uGotoFlags)
//	u32	uLookAtObjGUID			// The GUID of an entity to keep looking at (0 means none).
//	u8 uPctToLookat,			// (0-100) Percent to Goal that look at order will turn on  
//  u8 uFudgeDest				// How many feet the pathfinder is allowed to fudge the destination to make a valid path.
//	u8 uSpeedPct				// (0-100) pct speed this unit should travel at while on this GOTO  (ignored if 0 is specified)
//  u16 uGotoFlags				// GOTOFLAG_*
///////////////////////
FCLASS_ALIGN_PREFIX class CGenericGoto : public CAIThought
{
public:
	CGenericGoto(void) : CAIThought()
						{
							_ClearData();
						};

	//use this when all the params stuck into a param pack
	//and goto thought is to be added to a brains Pending Job Queue
	virtual void Init(	CAIBrain* pBrain,
						u8 uThoughtFlags,
						u8 uThoughtType, 
						CAIMemorySmall* pParamPack = NULL);		   //for help building a Param Pack that CGenericGoto see StuffGotoParams(..) and ExtractGotoParams(..)

	//use this Init when manually calling  init, work, and cleanup
	//for a goto thought with nothing specific to look at on the way.
	void Init(	CAIBrain* pBrain,
				u8 uThoughtFlags,				// see AITHOUGHT::THOUGHFLAG_*
				u8 uThoughtType,				// see CAIBrain::TT_*
				const CFVec3A& GotoLoc,
				u8 uFudgeDest = 0,				// How many feet the pathfinder is allowed to fudge the destination to make a valid path.
				u8 uSpeedPct = 0,				// (0-100) pct speed this unit should travel at while on this GOTO  (ignored if 0 is specified)
				u16 uGotoFlags = 0);				// from enum GOTOFLAG_
								


	//use this Init when manually calling  init, work, and cleanup
	//for a gotothought and there is a desired object or pt_ws to look at.
	void InitWithLookAt(	CAIBrain* pBrain,
							u8 uThoughtFlags,				// see AITHOUGHT::THOUGHFLAG_*
							u8 uThoughtType,				// see CAIBrain::TT_*
							const CFVec3A& GotoLoc,
							const CFVec3A& LookAtLoc,		// A pt in world space to aim torso and head at. (only used if uLookAtObjGUID == 0 and GOTOFLAG_LOOKAT specified as with uGotoFlags)
							u32	uLookAtObjGUID,				// The GUID of an entity to keep looking at (0 means none).
							u8 uLookAtPct = 0,				// pct of the way there that look at will begin
							u8 uFudgeDest = 0,				// How many feet the pathfinder is allowed to fudge the destination to make a valid path.
							u8 uSpeedPct = 0,				// (0-100) pct speed this unit should travel at while on this GOTO  (ignored if 0 is specified)
							u16 uGotoFlags = 0);				// from enum GOTOFLAG_

	virtual void Cleanup(void);
	virtual void Work(void);

	// RTTI
	virtual BOOL SupportsClassInterface(const s32 nInterfaceId);
	virtual s32 GetClassInterface(void);

	static BOOL InitBank(s32 nHowMany);
	static void CleanupBank(void);
	static CAIThought* BankAccess(CAIThought*);
	static CNiBank<CGenericGoto> *s_pAllocBank;
	

	//working vbls
	enum
	{
		GOTOSTAGE_NEEDS_PATH  = 0,
		GOTOSTAGE_WAITINFOR_PATH,
		GOTOSTAGE_EN_ROUTE,
		GOTOSTAGE_FAILED_PATH,
		GOTOSTAGE_BLIND_GOTO,
		GOTOSTAGE_CLOSEENOUGH
	};


	//parameters
	CFVec3A m_GoalLoc;
	CFVec3A m_LookAtLoc;
	CFVec3A m_OriginLoc;
	CEntity* m_pLookAtObj;
	f32	m_fPctToLookAt;
	u16 m_uGotoFlags;
	u8 m_uFudgeDest;
	u8 m_uStage;		//GOTOSTAGE_*
	
	CSearchQuery	m_SearchQuery;
	CSearchResults	m_SearchResults;

protected:
	void _ClearData(void);

	FCLASS_STACKMEM_ALIGN(CGenericGoto);
} FCLASS_ALIGN_SUFFIX;


//
// Ground FaceIt
//
///////////////////////
//
// ParamPack
//
//	  - Large
///////////////////////
FCLASS_ALIGN_PREFIX class CGenericFaceIt : public CAIThought
{
public:
	CGenericFaceIt(void) : CAIThought(),
						m_uTimeOut(0),
						m_pLookAtObj(NULL),
						m_uFaceItFlags(0)
						{
							m_LookAtLoc.Zero();
						};
	virtual void Init(CAIBrain* pBrain, u8 uThoughtFlags, u8 uThoughtType, CAIMemorySmall* pParamPack = NULL);
	virtual void Cleanup(void);
	virtual void Work(void);

	// RTTI
	virtual BOOL SupportsClassInterface(const s32 nInterfaceId);
	virtual s32 GetClassInterface(void);

	static BOOL InitBank(s32 nHowMany);
	static void CleanupBank(void);
	static CAIThought* BankAccess(CAIThought*);
	static CNiBank<CGenericFaceIt> *s_pAllocBank;

	CFVec3A m_LookAtLoc;
	CEntity* m_pLookAtObj;
	u8 m_uTimeOut;			// (0-256) Even if the thing is there, quit after this number of seconds (0 means never quit)
	u8 m_uTrack;
	enum
	{
		FACEIT_DID_ALIGN = 0x01,
		FACEIT_HAS_TIMEOUT = 0x02,
	};
	u8 m_uFaceItFlags;

	FCLASS_STACKMEM_ALIGN(CGenericFaceIt);
} FCLASS_ALIGN_SUFFIX;



//
// Ground TalkTo
//
///////////////////////
//
// ParamPack
//
//	  - Large
///////////////////////
FCLASS_ALIGN_PREFIX class CGenericTalkTo : public CAIThought
{
public:
	CGenericTalkTo(void) : CAIThought(),
						m_uTimeOut(0),
						m_pLookAtObj(NULL),
						m_uTalkToFlags(0),
						m_pTalkModeFunc(NULL),
						m_pvData1(NULL),
						m_pvData2(NULL)
						{
						}
	virtual void Init(CAIBrain* pBrain, u8 uThoughtFlags, u8 uThoughtType, CAIMemorySmall* pParamPack = NULL);
	virtual void Cleanup(void);
	virtual void Work(void);

	// RTTI
	virtual BOOL SupportsClassInterface(const s32 nInterfaceId);
	virtual s32 GetClassInterface(void);

	static BOOL InitBank(s32 nHowMany);
	static void CleanupBank(void);
	static CAIThought* BankAccess(CAIThought*);
	static CNiBank<CGenericTalkTo> *s_pAllocBank;

	CEntity* m_pLookAtObj;
	void *m_pTalkModeFunc;
	void *m_pvData1;
	void *m_pvData2;
	u32 m_uTimeOut;			
	enum
	{
		TALKTO_MUST_STOP	= 0x01,
		TALKTO_MUST_FACE	= 0x02,
		TALKTO_DID_STOP		= 0x04,
		TALKTO_DID_FACE		= 0x08,
		TALKTO_HAS_TIMEOUT	= 0x10,
		TALKTO_DID_TALK		= 0x20,
	};
	u8 m_uTalkToFlags;

	FCLASS_STACKMEM_ALIGN(CGenericTalkTo);
} FCLASS_ALIGN_SUFFIX;


//
// Follow
//
///////////////////////
//
// ParamPack
//
//	  - Large
///////////////////////
FCLASS_ALIGN_PREFIX class CGenericFollow : public CAIThought
{
public:
	CGenericFollow(void) : CAIThought()	{	}
	virtual void Init(CAIBrain* pBrain, u8 uThoughtFlags, u8 uThoughtType, CAIMemorySmall* pParamPack = NULL);
	virtual void Cleanup(void);
	virtual void Work(void);

	// RTTI
	virtual BOOL SupportsClassInterface(const s32 nInterfaceId);
	virtual s32 GetClassInterface(void);

	static BOOL InitBank(s32 nHowMany);
	static void CleanupBank(void);
	static CAIThought* BankAccess(CAIThought*);
	static CNiBank<CGenericFollow> *s_pAllocBank;

	enum
	{
		FOLLOWFLAG_NONE						= 0x0000,
		FOLLOWFLAG_LOOK_AT_LEADER			= 0x0001,
		FOLLOWFLAG_LOOK_AWAYFROM_LEADER		= 0x0002,
		FOLLOWFLAG_LOOK_WITH_LEADER			= 0x0004,
		FOLLOWFLAG_HADGOAL					= 0x0008,
		FOLLOWFLAG_WASMOVING				= 0x0010,
		FOLLOWFLAG_FORMATION_POST_PATH		= 0x0020,
		FOLLOWFLAG_NEARBY_LEADER_PATH		= 0x0040,
		FOLLOWFLAG_GREETED_LEADER			= 0x0080,
	};

	enum
	{
		STUCKSTAGE_NOTSTUCK = 0,
		STUCKSTAGE_STUCK_WAITING,
		STUCKSTAGE_REQUESTING_PATH,
		STUCKSTAGE_ONPATH,
	};
	FINLINE void SetLook_AtLeader(void)							{ m_uFollowFlags |= FOLLOWFLAG_LOOK_AT_LEADER;} 
	FINLINE void SetLook_AwayFromLeader(void)					{ m_uFollowFlags |= FOLLOWFLAG_LOOK_AWAYFROM_LEADER;} 
	FINLINE void SetLook_WithLeader(void)						{ m_uFollowFlags |= FOLLOWFLAG_LOOK_WITH_LEADER;} 
	FINLINE void SetLook_None(void)								{ m_uFollowFlags &= ~(FOLLOWFLAG_LOOK_AT_LEADER|FOLLOWFLAG_LOOK_AWAYFROM_LEADER|FOLLOWFLAG_LOOK_AWAYFROM_LEADER|FOLLOWFLAG_LOOK_WITH_LEADER);} 
	FINLINE BOOL IsStuck(void)									{ return m_uStuckStage != STUCKSTAGE_NOTSTUCK;}


//protected:  commented out for debugging c
	void _DoSearchWork(void);
	BOOL _IsMoverOnFollowPath(void);

	CFVec3A			m_LastGoodPt_WS;
	CFVec3A			m_LastMoveToLoc;

	CGenericWait*	m_pLookAtMgr;
	f32				m_fTimeSinceMoved;
	u16				m_uNextLookAtLeaderTimeOut;
	u16				m_uLastLookAtLeaderTime;
	u16				m_uLookAtLeaderTimeOut;
	u16				m_uNextLookAheadTimeOut;
	u16				m_uLastLookAheadTime;
	u16				m_uLookAheadTimeOut;
	u16				m_uFormationPostPathFailureCount;
	u16				m_uPathFailureCount;
	u16				m_uStuckPathRetryTimeOut;
	u8				m_uStuckStage;


	CSearchQuery	m_SearchQuery;
	CSearchResults	m_aSearchResults[2];			// 2 search results so that we can be working on one, while using another.
	u32				m_uLeaderKills;					// check wander flags (WANDER_SEARCHRESULTTOGGLE) to determine the one in use
	u16				m_uFollowFlags;
	u8				m_uAvailableResultsSlot;		// Which of the results is free to use


	FCLASS_STACKMEM_ALIGN(CGenericFollow);
} FCLASS_ALIGN_SUFFIX;

#endif //_AIBIPED_H