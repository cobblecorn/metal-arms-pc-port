//////////////////////////////////////////////////////////////////////////////////////
// AlarmSys.h - Alarms and their Switches
//
//		Classes:
//			CAlarm				- a siren, a flashing light, that type of thing
//			CAlarmNet			- a network of alarms
//			CAlarmSwitch		- a switch that activates a network of alarms
//			CBotDispenser		- a device that dispenses bots (duh!)
//
//			CAlarmSysBuilder	- parses user props and creates and configures CAlarms, CAlarmNets and CAlarmSwitches
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
// 02/6/03 PGM       Created.
//////////////////////////////////////////////////////////////////////////////////////

#ifndef _ALARMSYS_H_
#define	_ALARMSYS_H_ 1

#include "flinklist.h"
#include "fmath.h"
#include "BotDispenser.h"

class CEntity;
class CBot;
class CEntityBuilder;
class CAlarmNet;
class CFAudioEmitter;
class CESpline;
class CDoorEntity;


//
//
//	CAlarm
//
//
FCLASS_NOALIGN_PREFIX class CAlarm
{
public:
	CAlarm(void);
	~CAlarm(void);

	void TurnOn(CAlarmNet* pNet);
	void TurnOff(CAlarmNet* pNet, BOOL bImmediately=FALSE);
	void Work(CAlarmNet* pNet);
//protected:
	CEntity* m_pAlarmEntity;
	CFAudioEmitter* pAlarmSound;
	u8 m_uAlarmNetId;
	FLink_t m_AlarmLink;				 //Alarms are linked within
	u16 m_uAlarmNotifyRad;
	u32 m_uAISoundHandle;				//for the AI sound.  real sounds played through CEntity ambient emitter system

//static
public:

protected:

	FCLASS_STACKMEM_NOALIGN(CAlarm);
} FCLASS_NOALIGN_SUFFIX;


//
//
//	  CAlarmSwitch
//
//
FCLASS_NOALIGN_PREFIX class CAlarmSwitch
{
public:
	CAlarmSwitch(void);
	~CAlarmSwitch(void);

	BOOL PressButton(CEntity* pFromWho, BOOL* pbDone = NULL);			   //returns FALSE if broken, pbDone == TRUE when done
	void Work(void);

	BOOL IsWorking(void);
	void _ClearData(void);

//protected:
	u8 m_uAlarmNetId;
	f32 m_fPressButtonTime;
	f32 m_fPressButtonTimeLeft;

	FLink_t m_AlarmSwitchLink;
	CEntity* m_pAlarmSwitchEntity;

//static
public:
	static CAlarmSwitch* Find(CEntity* pAlarmSwitchEntity);
	static void AddAlarmSwitch(CAlarmSwitch* pAlarmSwitch);
	static void InitSys(void);
	static void WorkSys(void);

//protected:

	static FLinkRoot_t m_AlarmSwitchList;

	FCLASS_STACKMEM_NOALIGN(CAlarmSwitch);
} FCLASS_NOALIGN_SUFFIX;


//
//
//	  CAlarmDoor
//
//
FCLASS_NOALIGN_PREFIX class CAlarmDoor
{
public:
	CAlarmDoor(void);
	~CAlarmDoor(void);

	void TurnOn(CAlarmNet* pNet);
	void TurnOff(CAlarmNet* pNet, BOOL bImmediately=FALSE);

//protected:
	u8 m_uAlarmNetId;

	enum
	{
		ALARMDOORFLAG_ALARM_NONE			= 0x00,
		ALARMDOORFLAG_ALARM_ON_UNLOCKS		= 0x01,
		ALARMDOORFLAG_ALARM_ON_OPENS		= 0x02,
		ALARMDOORFLAG_ALARM_ON_LOCKS		= 0x04,
		ALARMDOORFLAG_ALARM_ON_SHUTS		= 0x08,
		ALARMDOORFLAG_ALARM_OFF_UNLOCKS		= 0x10,
		ALARMDOORFLAG_ALARM_OFF_OPENS		= 0x20,
		ALARMDOORFLAG_ALARM_OFF_LOCKS		= 0x40,
		ALARMDOORFLAG_ALARM_OFF_SHUTS		= 0x80,
	};

	u8 m_uAlarmDoorFlags;
	FLink_t m_AlarmDoorLink;
	CEntity* m_pAlarmDoorEntity;

//static
public:
//protected:
	FCLASS_STACKMEM_NOALIGN(CAlarmDoor);
} FCLASS_NOALIGN_SUFFIX;


//
//
//	 CAlarmNet
//
//
FCLASS_ALIGN_PREFIX class CAlarmNet
{
public:
	CAlarmNet(void);
	~CAlarmNet(void);

	void Work(void);
	void TurnOn(CEntity* pIntruder, CFVec3A* pLastIntruderPos_WS);
	void TurnOff(BOOL bImmediately=FALSE);

	enum
	{
		NETSTATE_OFF  = 0,
		NETSTATE_TURNON,
		NETSTATE_RUNNING,
		NETSTATE_SHUTDOWN,
	};

	enum
	{
		ALARMNETFLAG_NONE						= 0x0000,
		ALARMNETFLAG_AUTORESPAWN				= 0x0001,
		ALARMNETFLAG_TRIGGERALLBOTSINPOOL		= 0x0002,
		ALARMNETFLAG_NO_TIMER					= 0x0004,
	};

	void AddAlarm(CAlarm* pAlarm);
	void AddAlarmDoor(CAlarmDoor* pAlarmDoor);
	void AddDispenser(CBotDispenser* pDispenser);
	void _ClearData(void);
	FINLINE BOOL IsOn(void)			{ return m_uNetState != NETSTATE_OFF;}
	BOOL NetNeedsABot(void);
	void HandleBotDeploys(void);

//protected:
//configurable vbls
	cchar* m_pszAlarmNetName;
	cchar* m_pszBotGroupName;
	u8 m_uActiveTimeMin;
	u8 m_uActiveTimeMax;
	u8 m_uTotalBotsToAutoSpawn;
	s8 m_nMaxNumLivingAutoBots;
	CESpline* m_pHiddenSpawnPts;		// Hidden spawn pts must be hidden (ie not in visible list, or behind camera view) to be spawned
	CESpline* m_pVisibleSpawnPts;		// Visible spawn pts can be used regardles of visibility info.  In pipes, behind trapdoors, etc.
	CESpline* m_pDoorwaySpawnPts;		// Spawn pts that are allegedly near a door.  Check to see if the door is closed to see if the spawn pt is valid.
	CDoorEntity* m_papDoors;			// the alleged doors.
	u16 m_uAlarmNetFlags;
	u8 m_uNumAutoBotsSoFar;

//internal data
	u8 m_uAlarmNetId;
	FLinkRoot_t m_AlarmList;				//Alarms in this network
	FLinkRoot_t m_AlarmDoorList;			//AlarmDoors in this network
	FLinkRoot_t m_DispenserList;			//Dispensers in this network
	FLink_t m_AlarmNetLink;					//Alarm Nets are linked globally

//temporary internal data
	CEntity* m_pIntruder;
	u8 m_uNetState;
	f32 m_fTurnOnTime;
	f32 m_fActiveTime;
	u32 m_uIntruderGUID;
	CFVec3A m_LastIntruderPos_WS;			//the last know place of the intruder.

//static
public:
	static CAlarmNet* FindAlarmNet(u8 uAlarmNetId);
	static CAlarmNet* FindAlarmNet(cchar* pszAlarmNetName);
	static CAlarmNet* FindAlarmNet(CBotDispenser* pDispenser); // Find the alarm net a dispenser belongs to
	static CAlarm* FindAlarm(CEntity* pAlarmEntity);
	static CBotDispenser* FindDispenser(CEntity* pDispenserEntity);
	static CAlarmDoor* FindAlarmDoor(CEntity* pAlarmEntity);
	static void AddAlarmNet(CAlarmNet* pAlarmNet);
	static BOOL InitAutoBots(void);
	static BOOL InitAllSpawnPts(void);

	static void CleanupAutoBots(void);
	s32 FindAvailableAutoBot(void);
	BOOL DeployAutoBot(s32 uAutoBotId);
	void ConfigureAI( CBot *pBot, f32 fFeetToGotoIfNoAlarm );

	static void InitSys(void);
	static void WorkSys(void);
//protected:
	static void UpdateAutoBotUseage(void);
	static u8 CountAutoBotsUseByNet(u8 uAlarmNetId);

	static CBot** m_papAutoBotPool;
	static u8* m_pauAutoBotNetUseage;
	static u8 m_uAutoBotPoolSize;
	static FLinkRoot_t m_AlarmNetList;


	FCLASS_STACKMEM_ALIGN(CAlarmNet);
} FCLASS_ALIGN_SUFFIX;



//
//
//	CAlarmBuilder
//
//
FCLASS_NOALIGN_PREFIX class CAlarmSysBuilder
{
public:

	void SetDefaults( CEntityBuilder* pBuilder );
	BOOL InterpretTable( CEntityBuilder* pBuilder );
	BOOL PostInterpretFixup( CEntityBuilder* pBuilder );
	BOOL BuildAlarmEntityIfNeeded( CEntity* pEntity);

	CAlarmNet* _AllocOrFindExistingAlarmNet(CEntity* pEntity);

	BOOL m_bIsAlarm;
	u32 m_uAlarmNotifyRad;
	cchar* m_pszAlarmNetName;
	cchar* m_pszBotGroupName;
	u32 m_uActiveTimeMin;
	u32 m_uActiveTimeMax;
	u32 m_uTotalBotsToAutoSpawn;
	u32 m_uAlarmNetFlags;
	s32 m_nMaxNumLivingAutoBots;


	BOOL m_bIsAlarmSwitch;
	f32 m_fSwitchPressTime;

	BOOL m_bIsAlarmDoor;
	u32 m_uAlarmDoorFlags;

	BOOL m_bIsHiddenAlarmSpawnSpline;
	BOOL m_bIsVisibleAlarmSpawnSpline;
	BOOL m_bIsDoorwayAlarmSpawnSpline;

	BOOL m_bIsDispenser;
	BotDispenser_Init_t m_DispenserInit;


	FCLASS_STACKMEM_NOALIGN(CAlarmSysBuilder);
} FCLASS_NOALIGN_SUFFIX;

//
//    AlarmSys API
//

BOOL AlarmSys_InitLevel(void);
void AlarmSys_UninitLevel(void);
void AlarmSys_Work(void);
void AlarmSys_ActionNearby(CEntity* pFromEntity, CEntity* pToEntity);
void AlarmSys_CheckpointRestore(void);
void AlarmSys_CheckpointRestorePostamble(void);
void AlarmSys_CheckpointSave(void);
void AlarmSys_ActionNearby(CEntity* pEntity);
void AlarmSys_ResolveEntityPointerFixups( void );		//   After all entities have been created, that function gets called so entities that are to be attached to other entities can be done.
CAlarmNet* AlarmSys_GetAutoBotNet(CBot* pBot);
u8 AlarmSys_GetAutoBotNetId(CBot* pBot);


#define AUTOBOT_BORN 1
#define AUTOBOT_DIED 2
#endif