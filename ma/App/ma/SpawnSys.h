//////////////////////////////////////////////////////////////////////////////////////
// SpawnSys.h - System for assisting in the automated spawning of bots
//
//		Classes:
//			CSpawnSys				- all static.  Defines external API and module-scope globals. Maintains list of all CSpawnNets.
//			CSpawnNet				- a network of spawn pts and properties governing spawn rate and such.
//			CEntitySpawner			- a wrapper for an entity that spawns things and is an extension of a spawn network.  (think: moving spawn pt)
//			CSpawnNetTripwire		- a wrapper for a tripwire entity that can force the defcon-level of a spawn net
//			CSwarmerBotObstacle		- an obstacle that CSwarmerBots will avoid if the are created vie the spawn system.
//			CSpawnSysBuilder		- all in one builder that parses user props,creates and configures: CSpawnNet, CEntitySpawner, and CSwarmerBotObstacle
//
// Author: Pat M
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
// 02/20/03 PGM       Created.
//////////////////////////////////////////////////////////////////////////////////////

#ifndef _SPAWNSYS_H_
#define	_SPAWNSYS_H_ 1

#include "flinklist.h"
#include "fmath.h"
#include "AI\AIWeaponCtrl.h"

class CEntity;
class CBot;
class CEntityBuilder;
class CSpawnNet;
class CEntitySpawner;
class CESpline;
class CSpawnNetTripwire;
class CSwarmerBotObstacle;
struct FVisVolume_t;
class CDoorEntity;
class CBotSwarmer;

//
//
//	CSpawnSys
//
//
FCLASS_NOALIGN_PREFIX class CSpawnSys
{
public:
	static BOOL InitLevel(void);
	static void UninitLevel(void);
	static void Work(void);
	static void CheckpointRestore(void);
	static void CheckpointSave(void);
	static void ResolveEntityPointerFixups( void );		//   After all entities have been created, that function gets called so entities that are to be attached to other entities can be done.
	static void TripwireInside(CEntity* pTripwire, CEntity* pTripper);
	static BOOL InitAutoBots(void);

	enum
	{
		SPAWNSYS_BOTTYPE_SWARMER = 0,			// this net make swarmers
		SPAWNSYS_BOTTYPE_VERMIN,				// this net makes vermin
		SPAWNSYS_NUM_BOTTYPES
	};

	static CSpawnNet* FindSpawnNet(u8 uSpawnNetId);
	static CSpawnNet* FindSpawnNet(cchar* pszSpawnNetName);
	static CEntitySpawner* FindEntitySpawner(CEntity* pEntitySpawner);
	static CSpawnNetTripwire* FindSpawnNetTripwire(CEntity* pTripwireEntity);
	static CSwarmerBotObstacle* FindSwarmerObstacle(CEntity* pSwarmObstacleEntity);
	static CSpawnNet* FindSwarmersSpawnNet(CBotSwarmer* pSwarmer);		//if pSwarmer is part of a spawn net, this should return which.

	static void AddSpawnNet(CSpawnNet* pSpawnNet);

	//Autobot access
	static void UpdateAutoBotUseage(void);
	static CBot* RequestAutoBot(u8 uSpawnNetId, u32* puAutoBotId, u8* puAutoBotType);				// a net is disiring a bot because it wants to spawn one.
	static u16 CountAutoBotsUseByNet(u8 uNetId);
	static CBot* RequestAutoBot(u8 uSpawnNetId, u8 uBotType);


	class CSpawnBotSave
	{
	public:
		enum
		{
			SPAWNNETMUNGE_MASK = 0xfc,
			BOTTYPEMUNGE_MASK = 0x03,
		};
		u8 m_uNormal[3];
		u8 m_uBotTypeAndSpawnNetMunge;
		f32 m_fLoc[3];
		u8 m_uNetId;	 //derg, can't fit it.
		u8 m_uPad;
	};

//data:
	static CBot* m_paAutoBotAlloc[SPAWNSYS_NUM_BOTTYPES];
	static CBot** m_papAutoBotPool[SPAWNSYS_NUM_BOTTYPES];
	static u16* m_pauAutoBotNetUseage[SPAWNSYS_NUM_BOTTYPES];
	static u16 m_auAutoBotPoolSize[SPAWNSYS_NUM_BOTTYPES];


	static CSpawnBotSave* m_paSavePool;
	static u32 m_uSavePoolSize;

	static FLinkRoot_t m_SpawnNetList;

	FCLASS_STACKMEM_NOALIGN(CSpawnSys);
} FCLASS_NOALIGN_SUFFIX;


//
//
//	 CSpawnNet
//
//
FCLASS_ALIGN_PREFIX class CSpawnNet
{
public:
	CSpawnNet(void);
	~CSpawnNet(void);

	void _ClearData(void);
	enum
	{
		SPAWNNET_DEFCON_0_INACTIVE = 0,			// no part of the swarmnet is even in the active list
		SPAWNNET_DEFCON_1_ACTIVE,				// some part of the swarmnet is in the active volume list
		SPAWNNET_DEFCON_2_VISIBLE,				// some part of the swarmnet is in the visible volume list
		SPAWNNET_DEFCON_3_INTRUDER,				// An intruder is in the same voluem as some spawn pt of the swarmnet
		NUM_SPANNET_DEFCONS,
	};

	void CheckPointRestore(void);
	void SetDefconLevel(u8 uDefConLevel);

	void AddEntitySpawner(CEntitySpawner* pSpawner);	//this entity
	void AddTripwire(CSpawnNetTripwire* pTripwire);
	void AddSwarmerObstacle(CSwarmerBotObstacle* pObstacle);
	void InitVolumeLookup(void);
	void InitSpawnPts(void);

	enum
	{
		SPAWNNET_STATE_OFF  = 0,
		SPAWNNET_STATE_TURNON,
		SPAWNNET_STATE_RUNNING,
		SPAWNNET_STATE_SHUTDOWN,
	};

	enum
	{
		SPAWNNET_FLAG_NONE						= 0x00,
		SPAWNNET_FLAG_TIMER						= 0x01,		// incase this spawn net is turns off after a certain amount of tim
		SPAWNNET_FLAG_USE_LIFETIME_LIMIT		= 0x02,		// incase this spawn net has a max number to spawn in its lifetime	
		SPAWNNET_FLAG_USE_LIVING_LIMIT			= 0x04,		// incase this spawn net has a max number to spawn at anyone time
		SPAWNNET_FLAG_USE_SWARMER_ATTACK_DIST	= 0x08,
		SPAWNNET_NO_AUTO_DEFCON					= 0x10,
	};

	FINLINE BOOL IsOn(void)			{ return m_uNetState != SPAWNNET_STATE_OFF;}
	
	void DeployAutoBot(CBot* pBot);
 	void Work(void);

//protected:
//configurable vbls
	cchar* m_pszSpawnNetName;
	u8 m_uActiveTimeMin;									   
	u8 m_uActiveTimeMax;
	u8 m_uSpawnNetFlags;
	u8 m_uBotType;
	u16 m_uNumToSpawnLifetime;
	u16 m_uSwarmerBotWanderDist;		//How far bots spawned by this net are allowed to wander from their spawn pt.
	u16 m_uSwarmerBotAttackDist;		//How far bots spawned by this net are allowed to attack from their spawn pt.
	f32 m_fSwarmerBotAttackSpeedNorm;	//Normalized speed of the swarmers that come from this net
	CESpline* m_pHiddenSpawnPts;		//Hidden spawn pts must be hidden (ie not in visible list, or behind camera view) to be spawned	from
	u8* m_pauHiddenSpawnDirs;
	CESpline* m_pVisibleSpawnPts;		//Visible spawn pts can be used regardles of visibility info.  In pipes, behind trapdoors, etc.
	u8* m_pauVisibleSpawnDirs;
	CESpline* m_pDoorwaySpawnPts;		//pts whose uselfullness is linked to nearby doors. (ptrs to doors are automatically resolved at load time.
	u8* m_pauDoorwaySpawnDirs;
	CESpline* m_pPreplacedSpawnPts;		//for preplaced swarmers. (defcon_0 will be for preplaced units if there is a preplaces spawn pt array
	u8* m_pauPreplacedSpawnDirs;
	CDoorEntity** m_papDoors;
	CAIWeaponCtrl m_aSpawnRateCtrl[NUM_SPANNET_DEFCONS];
	u16 m_anNumLivingLimit[NUM_SPANNET_DEFCONS];
	u16 *m_pauVolume;
	u16 m_uNumVolumes;
	u16 *m_pauHiddenSpawnPtVol;

//internal data
	u8 m_uSpawnNetId;
	FLinkRoot_t m_EntitySpawnerList;			// Entities that can act as spawners in this network
	FLinkRoot_t m_TripwireList;					// Tripwire boxes that can be used to force defcon level setting of a spawnet
	FLinkRoot_t m_SwarmerObstacleList;			// Swarmer bot obstacles in this network
	FLink_t m_SpawnNetLink;						// all spawn nets are linked in the spawnsys list

//internal data to be cleared by checkpoint restore
	CEntity* m_pIntruder;
	u32 m_uIntruderGUID;
	f32 m_fTurnOnTime;
	f32 m_fActiveTime;
	u16 m_uNumSpawnedSoFarLifetime;
	u8 m_uDefConLevel;
	u8 m_uNetState;
	f32 m_fDefConTimeOut;
	u16 m_uSavedBots;

	FCLASS_STACKMEM_ALIGN(CSpawnNet);
} FCLASS_ALIGN_SUFFIX;



//
//	CEntitySpawner
//
FCLASS_ALIGN_PREFIX class CEntitySpawner
{
public:
	CEntitySpawner(void);
	~CEntitySpawner(void);

	void Work(CSpawnNet* pNet);
	BOOL IsActive(void);
	void _ClearData(void);

	enum
	{
		ENTITYSPAWNER_FLAG_NONE				= 0x0000,
		ENTITYSPAWNER_FLAG_SPAWNONDEATH		= 0x0001,	   //when this entity dies, it will spawn
		ENTITYSPAWNER_FLAG_SPAWNWITHNET		= 0x0002,	   //the span net can use this entity as a regular spawn pt, whenever it wants
		ENTITYSPAWNER_FLAG_DISABLE			= 0x0004,
	};
	FLink_t m_NetLink;						// Entity spawers are linked in a spawn net
	CEntity* m_pEntity;						// the entity (used for location)
	u8 m_uSpawnNetId;						// Which spawn net this spawner is part of
	u8 m_uEntitySpawnerFlags;
	u8 m_uNumToSpawnOnDeath;

	FCLASS_STACKMEM_ALIGN(CEntitySpawner);
} FCLASS_ALIGN_SUFFIX;


//
//	CSpawnNetTripwire
//
FCLASS_ALIGN_PREFIX class CSpawnNetTripwire
{
public:
	CSpawnNetTripwire(void);
	~CSpawnNetTripwire(void);

	BOOL IsActive(void);
	void _ClearData(void);

	enum
	{
		SPAWNNETTRIPWIRE_FLAG_NONE				= 0x0000,
		SPAWNNETTRIPWIRE_FLAG_DISABLE			= 0x0004,
	};
	FLink_t m_NetLink;						// Entity spawers can be linked in a spawn net
	CEntity* m_pEntity;
	CSpawnNet* m_pNet;
	u8 m_uTripwireFlags;
	u8 m_uForceDefconLevel;
	FCLASS_STACKMEM_ALIGN(CSpawnNetTripwire);
} FCLASS_ALIGN_SUFFIX;


//
//	CSwarmerBotObstacle
//
FCLASS_ALIGN_PREFIX class CSwarmerBotObstacle
{
public:
	CSwarmerBotObstacle(void);
	~CSwarmerBotObstacle(void);

	void Work(CSpawnNet* pNet);
	BOOL IsActive(void);
	void _ClearData(void);

	enum
	{
		SWARMERBOTOBSTACLE_FLAG_NONE	= 0x0000,
		SWARMERBOTOBSTACLE_FLAG_DISABLE	= 0x0001,
	};
	FLink_t m_NetLink;						// Entity spawers can be linked in a spawn net
	CEntity* m_pEntity;
	u32 m_uAISoundHandle;
	f32 m_fRadius;
	u8 m_uSpawnNetId;
	u8 m_uSwarmerBotObstacleFlags;


	FCLASS_STACKMEM_ALIGN(CSwarmerBotObstacle);
} FCLASS_ALIGN_SUFFIX;


//
//
//	CSpawnSysBuilder
//
//
FCLASS_NOALIGN_PREFIX class CSpawnSysBuilder
{
public:

	void SetDefaults( CEntityBuilder* pBuilder );
	BOOL InterpretTable( CEntityBuilder* pBuilder );
	BOOL PostInterpretFixup( CEntityBuilder* pBuilder );
	BOOL BuildSpawnSysObjIfNeeded( CEntity* pEntity);

	CSpawnNet* _AllocOrFindExistingSpawnNet(CEntity* pEntity);

	//SpawnNet
	cchar* m_pszSpawnNetName;
	u32 m_uActiveTimeMin;									   
	u32 m_uActiveTimeMax;
	u32 m_uSpawnNetFlags;
	u32 m_uBotType;					   //which type of bots to spawn enum SwarmSys_BotType_*
	u32 m_uNumToSpawnLifetime;
	u32 m_uSwarmerBotWanderDist;		//How far bots spawned by this net are allowed to wander from their spawn pt.
	u32 m_uSwarmerBotAttackDist;		//How far bots spawned by this net are allowed to attack from their spawn pt.
	f32 m_fSwarmerBotAttackSpeedNorm;	//
	BOOL m_bIsSpawnSpline_Hidden;
	BOOL m_bIsSpawnSpline_Visible;
	BOOL m_bIsSpawnSpline_Doorway;
	BOOL m_bIsSpawnSpline_Preplaced;


	u32 m_uNumLivingLimit_0;		
	f32 m_fBurstDelay_0;			
	f32 m_fFireDelay_0;			
	u32 m_uNumFiresPerBurst_0;		

	u32 m_uNumLivingLimitBonus_0;
	f32 m_fBurstDelayBonus_0;		
	f32 m_fFireDelayBonus_0;		
	u32 m_uNumFiresPerBurstBonus_0;

	u32 m_uNumLivingLimit_1;		
	f32 m_fBurstDelay_1;			
	f32 m_fFireDelay_1;			
	u32 m_uNumFiresPerBurst_1;		

	u32 m_uNumLivingLimitBonus_1;
	f32 m_fBurstDelayBonus_1;		
	f32 m_fFireDelayBonus_1;		
	u32 m_uNumFiresPerBurstBonus_1;

	u32 m_uNumLivingLimit_2;		
	f32 m_fBurstDelay_2;			
	f32 m_fFireDelay_2;			
	u32 m_uNumFiresPerBurst_2;	

	u32 m_uNumLivingLimitBonus_2;
	f32 m_fBurstDelayBonus_2;		
	f32 m_fFireDelayBonus_2;		
	u32 m_uNumFiresPerBurstBonus_2;

	u32 m_uNumLivingLimit_3;		
	f32 m_fBurstDelay_3;			
	f32 m_fFireDelay_3;			
	u32 m_uNumFiresPerBurst_3;		

	u32 m_uNumLivingLimitBonus_3;
	f32 m_fBurstDelayBonus_3;		
	f32 m_fFireDelayBonus_3;		
	u32 m_uNumFiresPerBurstBonus_3;

	//CEntitySpawner
	BOOL m_bIsEntitySpawner;
	u32 m_uEntitySpawnerFlags;
	u32 m_uEntitySpawnerNumToSpawnOnDeath;

	//CSpawnNetTripwire
	BOOL m_bIsSpawnNetTripwire;
	u32 m_uSpawnNetTripwireFlags;
	u32 m_uSpawnNetTripwireForceDefconLevel;

	//CSwarmerObstacle
	BOOL m_bIsSwarmerObstacle;
	u32 m_uObstacleFlags;
	f32 m_fObstacleRadius;


	FCLASS_STACKMEM_NOALIGN(CSpawnSysBuilder);
} FCLASS_NOALIGN_SUFFIX;

#endif