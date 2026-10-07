#include "fang.h"
#include "fclib.h"
#include "fcheckpoint.h"
#include "fScriptSystem.h"
#include "FXMeshBuild.h"
#include "AlarmSys.h"
#include "bot.h"
#include "BuilderHelp.h"
#include "Door.h"
#include "Entity.h"
#include "Espline.h"
#include "gstring.h"
#include "MeshEntity.h"
#include "MultiplayerMgr.h"
#include "AI\AIBrainman.h"
#include "AI\AIBrainUtils.h"
#include "AI\AiApi.h"
#include "AI\AIEnviro.h"
#include "AI\AIGameUtils.h"
#include "AI\AIGraph.h"
#include "AI\AIGroup.h"
#include "AI\AIMain.h"

const u32 _kuAlarmOnMeshId = 1;
const u32 _kuAlarmOffMeshId = 0;
const f32 _kfAlarmOnAnimSpeed = 1.0f;
const f32 _kfAlarmOffAnimSpeed = 0.0f;
cchar* _pszAutoBotNameTag = "XA_AUTOBOT_";
#define IGNORE_LIVING_AUTOBOT_LIMIT -2

cchar *_pszDispenserSlopBucketName = "Dispense_Net";


// static vars
#define _SECS_TO_WAIT_BEFORE_TURNING_SLOP_DISPENSER_NET_ON		0.5f

static f32 _fSecsBeforeTurningDispenserNetOn;
static CAlarmNet *_pGlobalDispenserNet;

///////////////////
// tables of data
//////////////////
BuilderHelp_EnumTagSpec _aAlarmDoorCtrl_Use[] = 
{
	"ALARM_ON_UNLOCKS",		CAlarmDoor::ALARMDOORFLAG_ALARM_ON_UNLOCKS,
	"ALARM_ON_OPENS",		CAlarmDoor::ALARMDOORFLAG_ALARM_ON_OPENS,
	"ALARM_ON_LOCKS",		CAlarmDoor::ALARMDOORFLAG_ALARM_ON_LOCKS,
	"ALARM_ON_SHUTS",		CAlarmDoor::ALARMDOORFLAG_ALARM_ON_SHUTS,
	"ALARM_OFF_UNLOCKS",	CAlarmDoor::ALARMDOORFLAG_ALARM_OFF_LOCKS,
	"ALARM_OFF_OPENS",		CAlarmDoor::ALARMDOORFLAG_ALARM_OFF_OPENS,
	"ALARM_OFF_LOCKS",		CAlarmDoor::ALARMDOORFLAG_ALARM_OFF_LOCKS,
	"ALARM_OFF_SHUTS",		CAlarmDoor::ALARMDOORFLAG_ALARM_OFF_SHUTS,
	BUILDER_ERROR_STRING("CAlarmSysBuilder::InterpretTable() : Invalid value '%s' for 'XA_ALARMDOOR_CTRL' table. Use (ALARM_ON_UNLOCKS or ALARM_ON_OPENS or ALARM_ON_LOCKS or ALARM_ON_SHUTS or ALARM_OFF_*).\n"), 0,
};


BuilderHelp_EnumTagSpec _aAlarmNetCtrl_Use[] = 
{
	"AUTO_RESPAWN",			CAlarmNet::ALARMNETFLAG_AUTORESPAWN,
	"USE_ALLBOTSINPOOL",	CAlarmNet::ALARMNETFLAG_TRIGGERALLBOTSINPOOL,
	"NO_TIMER",				CAlarmNet::ALARMNETFLAG_NO_TIMER,
	BUILDER_ERROR_STRING("CAlarmSysBuilder::InterpretTable() : Invalid value '%s' for 'XA_ALARMNET_CTRL' table. Use atleast on of (AUTO_RESPAWN or USE_ALLBOTSINPOOL or NO_TIMER).\n"), 0,
};


static InterpretTableDataSpec _aItableDataSpec[] = 
{
//	Tag												Address of Vbl To Be set																				DataType			uNumEnumTags in Spec											pMinMaxSpec								BuilderHelp_EnumTagSpec			Group Bool								internal use	 internal use
	"XA_ISALARM",									FANG_OFFSETOF(CAlarmSysBuilder, m_bIsAlarm),															BDT_BOOL,			0,																NULL,									NULL,							NO_TRACKGROUPEDBOOLOFFSET,				NULL,			 NULL,
	"XA_ALARM_NOTIFY_RAD",							FANG_OFFSETOF(CAlarmSysBuilder, m_uAlarmNotifyRad),														BDT_U32,			0,																&BuilderHelp_U8,						NULL,							NO_TRACKGROUPEDBOOLOFFSET,				NULL,			 NULL,
	"XA_ALARMNETNAME",								FANG_OFFSETOF(CAlarmSysBuilder, m_pszAlarmNetName),														BDT_STRING,			0,																NULL,									NULL,							NO_TRACKGROUPEDBOOLOFFSET,				NULL,			 NULL,
	"XA_ISALARMSWITCH",								FANG_OFFSETOF(CAlarmSysBuilder, m_bIsAlarmSwitch),														BDT_BOOL,			0,																NULL,									NULL,							NO_TRACKGROUPEDBOOLOFFSET,				NULL,			 NULL,
	"XA_ISALARMDOOR",								FANG_OFFSETOF(CAlarmSysBuilder, m_bIsAlarmDoor),														BDT_BOOL,			0,																NULL,									NULL,							NO_TRACKGROUPEDBOOLOFFSET,				NULL,			 NULL,
	"XA_ALARMNET_CTRL",								FANG_OFFSETOF(CAlarmSysBuilder, m_uAlarmNetFlags),														BDT_ENUMS2BITS,		sizeof(_aAlarmNetCtrl_Use)/sizeof(BuilderHelp_EnumTagSpec),		NULL,									_aAlarmNetCtrl_Use,				NO_TRACKGROUPEDBOOLOFFSET,				NULL,			 NULL,
	"XA_ALARMDOOR_CTRL",							FANG_OFFSETOF(CAlarmSysBuilder, m_uAlarmDoorFlags),														BDT_ENUMS2BITS,		sizeof(_aAlarmDoorCtrl_Use)/sizeof(BuilderHelp_EnumTagSpec),	NULL,									_aAlarmDoorCtrl_Use,			NO_TRACKGROUPEDBOOLOFFSET,				NULL,			 NULL,
	"XA_ALARMNET_ONTIME_MIN",						FANG_OFFSETOF(CAlarmSysBuilder, m_uActiveTimeMin),														BDT_U32,			0,																&BuilderHelp_U8,						NULL,							NO_TRACKGROUPEDBOOLOFFSET,				NULL,			 NULL,
	"XA_ALARMNET_ONTIME_MAX",						FANG_OFFSETOF(CAlarmSysBuilder, m_uActiveTimeMax),														BDT_U32,			0,																&BuilderHelp_U8,						NULL,							NO_TRACKGROUPEDBOOLOFFSET,				NULL,			 NULL,
	"XA_ALARMNET_TOTALBOTSTOAUTOSPAWN",				FANG_OFFSETOF(CAlarmSysBuilder, m_uTotalBotsToAutoSpawn),												BDT_U32,			0,																&BuilderHelp_U8,						NULL,							NO_TRACKGROUPEDBOOLOFFSET,				NULL,			 NULL,
	"XA_SWITCH_PRESSTIME",							FANG_OFFSETOF(CAlarmSysBuilder, m_fSwitchPressTime),													BDT_F32,			0,																&BuilderHelp_PosF32MMS,					NULL,							NO_TRACKGROUPEDBOOLOFFSET,				NULL,			 NULL,
	"XA_ISHIDDEN_SPAWN_PTS",						FANG_OFFSETOF(CAlarmSysBuilder, m_bIsHiddenAlarmSpawnSpline),											BDT_BOOL,			0,																NULL,									NULL,							NO_TRACKGROUPEDBOOLOFFSET,				NULL,			 NULL,
	"XA_ISVISIBLE_SPAWN_PTS",						FANG_OFFSETOF(CAlarmSysBuilder, m_bIsVisibleAlarmSpawnSpline),											BDT_BOOL,			0,																NULL,									NULL,							NO_TRACKGROUPEDBOOLOFFSET,				NULL,			 NULL,
	"XA_ISDOORWAY_SPAWN_PTS",						FANG_OFFSETOF(CAlarmSysBuilder, m_bIsDoorwayAlarmSpawnSpline),											BDT_BOOL,			0,																NULL,									NULL,							NO_TRACKGROUPEDBOOLOFFSET,				NULL,			 NULL,
	"XA_ALARMNET_MAX_LIVING_AUTOBOTS",				FANG_OFFSETOF(CAlarmSysBuilder, m_nMaxNumLivingAutoBots),												BDT_S32,			0,																&BuilderHelp_S32,						NULL,							NO_TRACKGROUPEDBOOLOFFSET,				NULL,			 NULL,
	"XA_ALARMNET_BOT_GROUP",						FANG_OFFSETOF(CAlarmSysBuilder, m_pszBotGroupName),														BDT_STRING,			0,																NULL,									NULL,							NO_TRACKGROUPEDBOOLOFFSET,				NULL,			 NULL,
	"XA_ISDISPENSER",								FANG_OFFSETOF(CAlarmSysBuilder, m_bIsDispenser),														BDT_BOOL,			0,																NULL,									NULL,							NO_TRACKGROUPEDBOOLOFFSET,				NULL,			 NULL,
///////////////////////																																																																																							 
// dispenser properties																																																																																							 
	"XA_DISPENSER_BOT1NAME",						FANG_OFFSETOF(CAlarmSysBuilder, m_DispenserInit.apszBotNamesToSpawn[0]),								BDT_STRING,			0,																NULL,									NULL,							NO_TRACKGROUPEDBOOLOFFSET,				NULL,			 NULL,
	"XA_DISPENSER_BOT2NAME",						FANG_OFFSETOF(CAlarmSysBuilder, m_DispenserInit.apszBotNamesToSpawn[1]),								BDT_STRING,			0,																NULL,									NULL,							NO_TRACKGROUPEDBOOLOFFSET,				NULL,			 NULL,
    "XA_DISPENSER_BOT3NAME",						FANG_OFFSETOF(CAlarmSysBuilder, m_DispenserInit.apszBotNamesToSpawn[2]),								BDT_STRING,			0,																NULL,									NULL,							NO_TRACKGROUPEDBOOLOFFSET,				NULL,			 NULL,
    "XA_DISPENSER_BOT4NAME",						FANG_OFFSETOF(CAlarmSysBuilder, m_DispenserInit.apszBotNamesToSpawn[3]),								BDT_STRING,			0,																NULL,									NULL,							NO_TRACKGROUPEDBOOLOFFSET,				NULL,			 NULL,
    "XA_DISPENSER_WAIT_FOR_BOT_DEATH",				FANG_OFFSETOF(CAlarmSysBuilder, m_DispenserInit.bWaitForBotDeath),										BDT_BOOL,			0,																NULL,									NULL,							NO_TRACKGROUPEDBOOLOFFSET,				NULL,			 NULL,
	"XA_DISPENSER_RANDOMIZE_BOTS",					FANG_OFFSETOF(CAlarmSysBuilder, m_DispenserInit.bRandomizeBots),										BDT_BOOL,			0,																NULL,									NULL,							NO_TRACKGROUPEDBOOLOFFSET,				NULL,			 NULL,
    "XA_DISPENSER_BOTS_PER_SESSION",				FANG_OFFSETOF(CAlarmSysBuilder, m_DispenserInit.nBotsPerSession),										BDT_U32,			0,																&BuilderHelp_U32,						NULL,							NO_TRACKGROUPEDBOOLOFFSET,				NULL,			 NULL,
	"XA_DISPENSER_SECS_BETWEEN_SPAWNS",				FANG_OFFSETOF(CAlarmSysBuilder, m_DispenserInit.fSecsBetweenSpawns),									BDT_F32,			0,																&BuilderHelp_PosF32MMS,					NULL,							NO_TRACKGROUPEDBOOLOFFSET,				NULL,			 NULL,
	"XA_DISPENSER_SECS_BETWEEN_SESSIONS",			FANG_OFFSETOF(CAlarmSysBuilder, m_DispenserInit.fSecsBetweenSession),									BDT_F32,			0,																&BuilderHelp_PosF32MMS,					NULL,							NO_TRACKGROUPEDBOOLOFFSET,				NULL,			 NULL,
	"XA_DISPENSER_FX_MESH_NAME",					FANG_OFFSETOF(CAlarmSysBuilder, m_DispenserInit.pszBeamMeshName),										BDT_STRING,			0,																NULL,									NULL,							NO_TRACKGROUPEDBOOLOFFSET,				NULL,			 NULL,
	"XA_DISPENSER_MIN_FX_SCALE",					FANG_OFFSETOF(CAlarmSysBuilder, m_DispenserInit.fMinBeamScale),											BDT_F32,			0,																&BuilderHelp_PosF32MMS,					NULL,							NO_TRACKGROUPEDBOOLOFFSET,				NULL,			 NULL,
	"XA_DISPENSER_MAX_FX_SCALE",					FANG_OFFSETOF(CAlarmSysBuilder, m_DispenserInit.fMaxBeamScale),											BDT_F32,			0,																&BuilderHelp_PosF32MMS,					NULL,							NO_TRACKGROUPEDBOOLOFFSET,				NULL,			 NULL,
	"XA_DISPENSER_SECS_TO_MAX_SCALE",				FANG_OFFSETOF(CAlarmSysBuilder, m_DispenserInit.fSecsToMaxBeamScale),									BDT_F32,			0,																&BuilderHelp_PosF32MMS,					NULL,							NO_TRACKGROUPEDBOOLOFFSET,				NULL,			 NULL,
	"XA_DISPENSER_SECS_BACK_TO_MIN_SCALE",			FANG_OFFSETOF(CAlarmSysBuilder, m_DispenserInit.fSecsToMinBeamScale),									BDT_F32,			0,																&BuilderHelp_PosF32MMS,					NULL,							NO_TRACKGROUPEDBOOLOFFSET,				NULL,			 NULL,
	"XA_DISPENSER_DROP_PARTS_BONENAME",				FANG_OFFSETOF(CAlarmSysBuilder, m_DispenserInit.pszFallingPartsBoneName),								BDT_STRING,			0,																NULL,									NULL,							NO_TRACKGROUPEDBOOLOFFSET,				NULL,			 NULL,
	"XA_DISPENSER_BOT_POSITION_BONENAME",			FANG_OFFSETOF(CAlarmSysBuilder, m_DispenserInit.pszBotPositionBone),									BDT_STRING,			0,																NULL,									NULL,							NO_TRACKGROUPEDBOOLOFFSET,				NULL,			 NULL,
	"XA_DISPENSER_SECS_TO_DROP_PARTS",				FANG_OFFSETOF(CAlarmSysBuilder, m_DispenserInit.fSpawnSecs),											BDT_F32,			0,																&BuilderHelp_PosF32MMS,					NULL,							NO_TRACKGROUPEDBOOLOFFSET,				NULL,			 NULL,
	"XA_DISPENSER_UNIT_PART_OVERLAP",				FANG_OFFSETOF(CAlarmSysBuilder, m_DispenserInit.fUnitPartOverlap),										BDT_F32,			0,																&BuilderHelp_PosUnitF32MMS,				NULL,							NO_TRACKGROUPEDBOOLOFFSET,				NULL,			 NULL,
	"XA_DISPENSER_ARM_ANIMATION_FILENAME",			FANG_OFFSETOF(CAlarmSysBuilder, m_DispenserInit.pszArmAnimationResName),								BDT_STRING,			0,																NULL,									NULL,							NO_TRACKGROUPEDBOOLOFFSET,				NULL,			 NULL,
	"XA_DISPENSER_SPARK_NAME",						FANG_OFFSETOF(CAlarmSysBuilder, m_DispenserInit.pszSparkResName),										BDT_STRING,			0,																NULL,									NULL,							NO_TRACKGROUPEDBOOLOFFSET,				NULL,			 NULL,
	"XA_DISPENSER_SMOKE_NAME",						FANG_OFFSETOF(CAlarmSysBuilder, m_DispenserInit.pszSmokeResName),										BDT_STRING,			0,																NULL,									NULL,							NO_TRACKGROUPEDBOOLOFFSET,				NULL,			 NULL,
	"XA_DISPENSER_LEFT_ARM_BONENAME",				FANG_OFFSETOF(CAlarmSysBuilder, m_DispenserInit.apszArmBoneNames[BOT_DISPENSER_BONE_ID_LEFT_ARM]),		BDT_STRING,			0,																NULL,									NULL,							NO_TRACKGROUPEDBOOLOFFSET,				NULL,			 NULL,
	"XA_DISPENSER_RIGHT_ARM_BONENAME",				FANG_OFFSETOF(CAlarmSysBuilder, m_DispenserInit.apszArmBoneNames[BOT_DISPENSER_BONE_ID_RIGHT_ARM]),		BDT_STRING,			0,																NULL,									NULL,							NO_TRACKGROUPEDBOOLOFFSET,				NULL,			 NULL,
	"XA_DISPENSER_SECS_BEFORE_FIRST_HAND_CONTACT",	FANG_OFFSETOF(CAlarmSysBuilder, m_DispenserInit.fSecsBeforeInitialHandContact),							BDT_F32,			0,																&BuilderHelp_PosF32MMS,					NULL,							NO_TRACKGROUPEDBOOLOFFSET,				NULL,			 NULL,
	"XA_DISPENSER_SECS_OF_ARM_STOW",				FANG_OFFSETOF(CAlarmSysBuilder, m_DispenserInit.fSecsOfFinalArmStow),									BDT_F32,			0,																&BuilderHelp_PosF32MMS,					NULL,							NO_TRACKGROUPEDBOOLOFFSET,				NULL,			 NULL,
	"XA_DISPENSER_BEAM_BONENAME",					FANG_OFFSETOF(CAlarmSysBuilder, m_DispenserInit.pszBeamBoneName),										BDT_STRING,			0,																NULL,									NULL,							NO_TRACKGROUPEDBOOLOFFSET,				NULL,			 NULL,
	"XA_DISPENSER_SPARKLE_FX_NAME",					FANG_OFFSETOF(CAlarmSysBuilder, m_DispenserInit.pszGenParticleFxResName),								BDT_STRING,			0,																NULL,									NULL,							NO_TRACKGROUPEDBOOLOFFSET,				NULL,			 NULL,
	"XA_DISPENSER_SPARKLE_FX_INTENSITY",			FANG_OFFSETOF(CAlarmSysBuilder, m_DispenserInit.fGenFxIntensity),										BDT_F32,			0,																&BuilderHelp_PosUnitF32MMS,				NULL,							NO_TRACKGROUPEDBOOLOFFSET,				NULL,			 NULL,
	"XA_DISPENSER_SOUND_BANK",						FANG_OFFSETOF(CAlarmSysBuilder, m_DispenserInit.pszSoundBankToLoad),									BDT_STRING,			0,																NULL,									NULL,							NO_TRACKGROUPEDBOOLOFFSET,				NULL,			 NULL,
	"XA_DISPENSER_ARMS_SOUND_GROUP_NAME",			FANG_OFFSETOF(CAlarmSysBuilder, m_DispenserInit.pszArmSound),											BDT_STRING,			0,																NULL,									NULL,							NO_TRACKGROUPEDBOOLOFFSET,				NULL,			 NULL,
	"XA_DISPENSER_BEAMS_SOUND_GROUP_NAME",			FANG_OFFSETOF(CAlarmSysBuilder, m_DispenserInit.pszBeamSound),											BDT_STRING,			0,																NULL,									NULL,							NO_TRACKGROUPEDBOOLOFFSET,				NULL,			 NULL,
	"XA_DISPENSER_PARTS_SOUND_GROUP_NAME",			FANG_OFFSETOF(CAlarmSysBuilder, m_DispenserInit.pszPartsSound),											BDT_STRING,			0,																NULL,									NULL,							NO_TRACKGROUPEDBOOLOFFSET,				NULL,			 NULL,
	"XA_DISPENSER_BELL_DING",						FANG_OFFSETOF(CAlarmSysBuilder, m_DispenserInit.pszDingSound),											BDT_STRING,			0,																NULL,									NULL,							NO_TRACKGROUPEDBOOLOFFSET,				NULL,			 NULL,
	"XA_DISPENSER_ONLY_SPECIFIED_BOTS",				FANG_OFFSETOF(CAlarmSysBuilder, m_DispenserInit.bMatchBotNames),										BDT_BOOL,			0,																NULL,									NULL,							NO_TRACKGROUPEDBOOLOFFSET,				NULL,			 NULL,
//////////////////////////////////																																																																																				 
};																																												
																																												
CAlarm::CAlarm(void)
:	m_pAlarmEntity(NULL),
	m_uAlarmNetId(0)

{
	m_AlarmLink.pPrevLink = NULL;
	m_AlarmLink.pNextLink = NULL;
}


CAlarm::~CAlarm(void)
{
}


void CAlarm::TurnOn(CAlarmNet* pNet)
{
	if (m_pAlarmEntity &&
		m_pAlarmEntity->IsInWorld() &&
		m_pAlarmEntity->TypeBits() & ENTITY_BIT_MESHENTITY)
	{
		CMeshEntity* pME = (CMeshEntity* )m_pAlarmEntity;
		if (pME->GetMeshCount() > _kuAlarmOnMeshId &&
			pME->GetCurrentMeshIndex() != _kuAlarmOnMeshId)
		{
			pME->SelectMesh( _kuAlarmOnMeshId );
			pME->UserAnim_SetSpeedMult(_kfAlarmOnAnimSpeed);
		}
	}

	if (m_pAlarmEntity->GetAmbientSFXHandle( ) != 0 )//FSNDFX_INVALID_FX_HANDLE==0
	{
		m_pAlarmEntity->PlayAmbientSFX( );
	}
}


void CAlarm::TurnOff(CAlarmNet* pNet, BOOL bImmediately/*=FALSE*/)
{
	if (m_pAlarmEntity &&
		m_pAlarmEntity->IsInWorld() &&
		m_pAlarmEntity->TypeBits() & ENTITY_BIT_MESHENTITY)
	{
		CMeshEntity* pME = (CMeshEntity* )m_pAlarmEntity;
		if (pME->GetMeshCount() > _kuAlarmOffMeshId &&
			pME->GetCurrentMeshIndex() != _kuAlarmOffMeshId)
		{
			pME->SelectMesh( _kuAlarmOffMeshId );
			pME->UserAnim_SetSpeedMult(_kfAlarmOffAnimSpeed);
		}
	}

	//safe even if not playing
	if (m_pAlarmEntity)
	{
		m_pAlarmEntity->StopAmbientSFX();
	}

	m_uAISoundHandle = 0;
}

void CAlarm::Work(CAlarmNet* pNet)
{
	//incase the alarm moves
	CFVec3A Goto;
	if (pNet->m_pIntruder)
	{
		Goto = pNet->m_pIntruder->MtxToWorld()->m_vPos;
	}
	else
	{
		Goto = m_pAlarmEntity->MtxToWorld()->m_vPos;
		f32 fNewY = 0.0f;
		if (aiutils_FloorElevationAt(Goto, &fNewY))
		{
			Goto.y = fNewY;
		}
		aimain_pAIGraph->FindClosestLOSVert2D_AdjustWithinRange(&Goto, m_uAlarmNotifyRad*0.5f);
	}

	if( !m_uAISoundHandle || !AIEnviro_ModifySound( m_uAISoundHandle,  Goto , (f32) m_uAlarmNotifyRad, 1.0f, AISOUNDTYPE_ALARM, m_pAlarmEntity) ) 
	{
		m_uAISoundHandle = AIEnviro_AddSound( Goto , (f32) m_uAlarmNotifyRad, 1.0f, AISOUNDTYPE_ALARM, 0, m_pAlarmEntity);
	}
}



//
//
//	CAlarmNet
//
//
FLinkRoot_t CAlarmNet::m_AlarmNetList;
CBot** CAlarmNet::m_papAutoBotPool = NULL;
u8* CAlarmNet::m_pauAutoBotNetUseage = NULL;
u8 CAlarmNet::m_uAutoBotPoolSize = 0;

CAlarmNet::CAlarmNet(void)
:
	m_pszAlarmNetName(NULL),
	m_pszBotGroupName(NULL),
	m_uActiveTimeMin(0),
	m_uActiveTimeMax(0),
	m_uTotalBotsToAutoSpawn(0),
	m_uAlarmNetFlags(ALARMNETFLAG_NONE),
	m_uAlarmNetId(0),
	m_nMaxNumLivingAutoBots(IGNORE_LIVING_AUTOBOT_LIMIT),
	m_pIntruder(NULL),
	m_uNetState(NETSTATE_OFF),
	m_fTurnOnTime(0.0f),
	m_fActiveTime(0.0f),
	m_uIntruderGUID(0),
	m_pHiddenSpawnPts(NULL),
	m_pVisibleSpawnPts(NULL),
	m_pDoorwaySpawnPts(NULL),
	m_papDoors(NULL)
{
	flinklist_InitRoot(&m_AlarmList, FANG_OFFSETOF(CAlarm, m_AlarmLink));
	flinklist_InitRoot(&m_AlarmDoorList, FANG_OFFSETOF(CAlarmDoor, m_AlarmDoorLink));
	flinklist_InitRoot(&m_DispenserList, FANG_OFFSETOF(CBotDispenser, m_DispenserLink));
	m_AlarmNetLink.pPrevLink = NULL;
	m_AlarmNetLink.pNextLink = NULL;
	m_LastIntruderPos_WS.Zero();

	_ClearData();
}

void CAlarmNet::_ClearData(void)
{
	m_uActiveTimeMin = 15;
	m_uActiveTimeMax = 45;
	m_uTotalBotsToAutoSpawn = 100;
	m_uAlarmNetFlags = CAlarmNet::ALARMNETFLAG_TRIGGERALLBOTSINPOOL | CAlarmNet::ALARMNETFLAG_AUTORESPAWN;
}


CAlarmNet::~CAlarmNet(void)
{
}


BOOL CAlarmNet::InitAutoBots(void)
{
	FASSERT(!m_papAutoBotPool);
	FASSERT(!m_uAutoBotPoolSize);
	FASSERT(!m_pauAutoBotNetUseage);

	CEntity* pEntity = NULL;
	u8 uAutoBotNameTagLen = fclib_strlen(_pszAutoBotNameTag);

	pEntity = CEntity::InWorldList_GetHead();
	while (pEntity)
	{
		if ((pEntity->TypeBits() & ENTITY_BIT_BOT) &&
			pEntity->Name() &&
			!fclib_strnicmp(pEntity->Name(), _pszAutoBotNameTag, uAutoBotNameTagLen))
		{
			m_uAutoBotPoolSize++;
			pEntity->RemoveFromWorld();	 //can't really see why autobots should ever start in the world.
			pEntity->SetAlarmSysUse(TRUE);
		}
		pEntity = pEntity->InWorldList_GetNext();
	}


	pEntity = CEntity::OutOfWorldList_GetHead();
	while (pEntity)
	{
		if ((pEntity->TypeBits() & ENTITY_BIT_BOT) &&
			 pEntity->Name() &&
			!fclib_strnicmp(pEntity->Name(), _pszAutoBotNameTag, uAutoBotNameTagLen))
		{
			m_uAutoBotPoolSize++;
			pEntity->SetAlarmSysUse(TRUE);
		}
		pEntity = pEntity->OutOfWorldList_GetNext();
	}
	
	if (m_uAutoBotPoolSize >0)
	{
		m_papAutoBotPool = (CBot**) fres_AlignedAlloc(m_uAutoBotPoolSize*sizeof(CBot*), 16);
		m_pauAutoBotNetUseage = (u8*) fres_AlignedAllocAndZero(m_uAutoBotPoolSize*sizeof(u8), 16);
		m_uAutoBotPoolSize = 0;

		if (m_papAutoBotPool && m_pauAutoBotNetUseage)
		{
			pEntity = CEntity::InWorldList_GetHead();
			while (pEntity)
			{
				if ((pEntity->TypeBits() & ENTITY_BIT_BOT) &&
					pEntity->Name() && 
					!fclib_strnicmp(pEntity->Name(), _pszAutoBotNameTag, uAutoBotNameTagLen))
				{
					m_papAutoBotPool[m_uAutoBotPoolSize++] = (CBot*) pEntity;
				}
				pEntity = pEntity->InWorldList_GetNext();
			}

			pEntity = CEntity::OutOfWorldList_GetHead();
			while (pEntity)
			{
				if ((pEntity->TypeBits() & ENTITY_BIT_BOT) &&
					pEntity->Name() && 
					!fclib_strnicmp(pEntity->Name(), _pszAutoBotNameTag, uAutoBotNameTagLen))
				{
					m_papAutoBotPool[m_uAutoBotPoolSize++] = (CBot*) pEntity;
				}
				pEntity = pEntity->OutOfWorldList_GetNext();
			}
		}
		else
		{
			DEVPRINTF("XA_Error: AlarmSys WARNING: Low Memory, couldn't allocate tiny array of pointers to autobots.\n");
			m_uAutoBotPoolSize = 0;
			m_papAutoBotPool = NULL;
			m_pauAutoBotNetUseage = NULL;
		}

	}

	return TRUE;
}


BOOL CAlarmNet::InitAllSpawnPts()
{

	return TRUE;
}


void CAlarmNet::CleanupAutoBots(void)
{
	if (m_papAutoBotPool)
	{  
		//no delete since we use the auto-cleaning up stack. fres_alloc
	}
	m_papAutoBotPool = NULL;
	m_uAutoBotPoolSize = 0;
	m_pauAutoBotNetUseage = NULL;
}

// This function is used for NON-DISPENSED BOTS.
s32 CAlarmNet::FindAvailableAutoBot(void)
{
	for (s32 i= 0; i < m_uAutoBotPoolSize; i++)
	{
		if (!m_papAutoBotPool[i]->IsInWorld() && !m_papAutoBotPool[i]->m_pCanOnlyBeDispensedBy && ( m_pauAutoBotNetUseage[i] == 0) )
		{
			if ((!this->m_pszBotGroupName && !fclib_strstr(m_papAutoBotPool[i]->Name(), "group_")) ||		  //can't use autobots that have the word "group" in their name
				(this->m_pszBotGroupName && fclib_strstr(m_papAutoBotPool[i]->Name(), m_pszBotGroupName)))	//unless, the net also has the word "group_" and a matching other string
			{
				return i;
			}
		}
	}
	return -1;
}


void CAlarmNet::UpdateAutoBotUseage(void)
{
	CBot *pBot;

	for (u32 i= 0; i < m_uAutoBotPoolSize; i++)
	{
		pBot = m_papAutoBotPool[i];

		if( !pBot->IsInWorld() )
		{
			if(!pBot->IsDead() && m_pauAutoBotNetUseage[i] != 0)
			{
				CAlarmNet* pWhichNet = CAlarmNet::FindAlarmNet(m_pauAutoBotNetUseage[i]);
				
				if (pWhichNet && pWhichNet->m_uNumAutoBotsSoFar > 0)
				{  //hmmm, credit the net since this bot was not removed from the world because it died
					pWhichNet->m_uNumAutoBotsSoFar--;
				}
			}
			if (m_pauAutoBotNetUseage[i])
			{
				int rr = 44;
				rr++;

			}
			m_pauAutoBotNetUseage[i] = 0;	//up for grabs by any net
		}
		else if (!pBot->IsPlayerBot())
		{
			if( pBot->IsDead())
			{
				pBot->RemoveFromWorld();	   //request world removal...
			}
			else if (!aiutils_IsInBoundsOfWorld(pBot))	  
			{
				pBot->Die(FALSE,FALSE);	 //request world removal...
			}
	/*			  //findfix: could improve this to include more cases that an autobot has become useless, and kill him off.
			else if( pBot->IsInWorld() &&
					pBot->AIBrain() &&
					!pBot->IsUnderConstruction() &&
					!pBot->AIBrain()->IsLODActive() )
			{
				pBot->>Die(FALSE,FALSE);
			}
	*/
		}
	}
}


u8 CAlarmNet::CountAutoBotsUseByNet(u8 uNetId)
{
	u8 uCount = 0;
	for (s32 i= 0; i < m_uAutoBotPoolSize; i++)
	{
		uCount += (u8) (m_pauAutoBotNetUseage[i] == uNetId);
	}

	return uCount;
}


BOOL CAlarmNet::DeployAutoBot(s32 uAutoBotId)
{
	BOOL bDeployed = FALSE;

	if (uAutoBotId >= 0 && 
		uAutoBotId < m_uAutoBotPoolSize && 
		m_papAutoBotPool[uAutoBotId])
	{

		//where to put it
		BOOL bFoundSpawnPt = FALSE;
		CFVec3A SpawnPt;

		if (m_pHiddenSpawnPts || m_pVisibleSpawnPts || m_pDoorwaySpawnPts)
		{
			// Hidden spawn pts must be hidden (ie not in visible list, or behind camera view) to be spawned
			if (m_pHiddenSpawnPts && m_pHiddenSpawnPts->PointCount())
			{
				u32 uSpawnEval = fmath_RandomChoice(m_pHiddenSpawnPts->PointCount());
				for (u32 j= 0; !bFoundSpawnPt && j < m_pHiddenSpawnPts->PointCount(); j++)
				{
					if (!m_pIntruder || m_pHiddenSpawnPts->PointArray()[uSpawnEval].DistSq(m_LastIntruderPos_WS) > 30.0f*30.0f)   //minimum feet to player to spawn
					{
						FVisVolume_t* pVol = NULL;
						if (fworld_GetVolumeContainingPoint(&(m_pHiddenSpawnPts->PointArray()[uSpawnEval]), &pVol) > 0.0f && pVol)
						{
							if (!(FVis_anVolumeFlags[pVol->nVolumeID] & FVIS_VOLUME_IN_VISIBLE_LIST)) 
							{
								SpawnPt = m_pHiddenSpawnPts->PointArray()[uSpawnEval];
								bFoundSpawnPt = TRUE;
								break;
							}
						}
						uSpawnEval %= m_pHiddenSpawnPts->PointCount();
					}
				}
			}

			// Visible spawn pts can be used regardles of visibility info.  In pipes, behind trapdoors, etc.
			if (!bFoundSpawnPt && m_pVisibleSpawnPts && m_pVisibleSpawnPts->PointCount())
			{
				u32 uSpawnEval = fmath_RandomChoice(m_pVisibleSpawnPts->PointCount());
				for (u32 j= 0; !bFoundSpawnPt && j < m_pVisibleSpawnPts->PointCount(); j++)
				{
					if (!m_pIntruder || m_pVisibleSpawnPts->PointArray()[uSpawnEval].DistSq(m_LastIntruderPos_WS) > 10.0f*10.0f)   //minimum feet to player to spawn
					{
						SpawnPt = m_pVisibleSpawnPts->PointArray()[uSpawnEval];
						bFoundSpawnPt = TRUE;
					}
					uSpawnEval %= m_pVisibleSpawnPts->PointCount();
				}
			}
/* NYI
			// Spawn pts that are allegedly near a door.  Check to see if the door is closed to see if the spawn pt is valid.
			if (!bFoundSpawnPt && m_pDoorwaySpawnPts)
			{
				if (m_papDoors[uSpawnEval] &&
					!m_papDoors[uSpawnEval]->AI_IsOpen(m_pDoorwaySpawnPts->PointArray()[uSpawnEval]) &&
					!m_papDoors[uSpawnEval]->AI_IsOpening(m_pDoorwaySpawnPts->PointArray()[uSpawnEval]))
				{

			}
*/
			if (bFoundSpawnPt)
			{
				CBot* pBot = m_papAutoBotPool[uAutoBotId];

				if ( CFCheckPoint::SetCheckPoint( 0 ) )
				{
					// set checkpoint system state to "load"
					if ( CFCheckPoint::SetLoadState() )
					{
						pBot->CheckpointRestore();
						pBot->Relocate_Xlat_WS(&SpawnPt);
     					pBot->AddToWorld();
						pBot->DrawEnable(TRUE,TRUE);
						m_pauAutoBotNetUseage[uAutoBotId] = m_uAlarmNetId;

						ConfigureAI( pBot, 10.0f );

						m_uNumAutoBotsSoFar++;
						bDeployed = TRUE;

						CFScriptSystem::TriggerEvent(CFScriptSystem::GetEventNumFromName("AutoBot"), AUTOBOT_BORN, (u32) pBot, (u32) this);
					}
				}
			}
		}
	}
	return bDeployed;
}

void CAlarmNet::ConfigureAI( CBot *pBot, f32 fFeetToGotoIfNoAlarm ) {
	CFVec3A Goto;
	f32 fNewY;
	BOOL bUseJobRules;

	if (m_pIntruder)
	{
		Goto = m_LastIntruderPos_WS;
		bUseJobRules = TRUE;
	}
	else
	{
		CAlarm* pAlarm = (CAlarm*) flinklist_GetHead(&m_AlarmList);
		if (pAlarm && m_pIntruder)	 //there must be an intruder for this bot to come running out to the alarm
		{
			Goto = pAlarm->m_pAlarmEntity->MtxToWorld()->m_vPos;
			bUseJobRules = TRUE;
		} else {
			// Goto is uninitialized, just set it to 10 feet in front of the bot's current position
			Goto.Set( pBot->MtxToWorld()->m_vFront );
			Goto.Mul( fFeetToGotoIfNoAlarm );
			Goto.Add( pBot->MtxToWorld()->m_vPos );
			bUseJobRules = FALSE;
		}
		
		fNewY = 0.0f;
		if (aiutils_FloorElevationAt(Goto, &fNewY))
		{
			Goto.y = fNewY;
		}
		if( pAlarm ) {
            aimain_pAIGraph->FindClosestLOSVert2D_AdjustWithinRange(&Goto, pAlarm->m_uAlarmNotifyRad*0.5f);
		} else {
			aimain_pAIGraph->FindClosestLOSVert2D_AdjustWithinRange(&Goto, fFeetToGotoIfNoAlarm );
		}
	}

	ai_AssignGoal_Goto( pBot->AIBrain(), Goto, 10, 100, GOTOFLAG_USE_JOB_REACT_RULES);//GOTOFLAG_POWERDOWN_AT_END);

	if (m_pIntruder)
	{
		//if the net has an intruder, set job of autobots to wander.

		ai_AssignJob_Wander( pBot->AIBrain(),
							50.0f,	// (0.0 - Inf)	Radius (in feet) to stay within while wandering
							FALSE,	// (0/1)		Use radius, but always stay in the original room
							0,		// (0-100)		Min Pct of this unit's ability that it should travel at	 (0 means wander should never change units speed)
							0,		// (0-100)		Max             "      "								 (0 means wander should never change units speed)
							0,		// (0-100)		How likely it is to stop and talk to other bots
							0,		// (0-100)		Breaks will last 5 seconds + random(curiosity)
							0,		// (0-100)		Percent Chance that that bot will take a break. (evaluated at every grid point)
							NULL );	// String table Name of espline entity that contains the path points
	}
}


void CAlarmNet::TurnOn(CEntity* pIntruder, CFVec3A* pLastIntruderPos_WS)
{
	if (m_uNetState == CAlarmNet::NETSTATE_OFF)
	{
		m_uNetState = NETSTATE_TURNON;
		m_pIntruder = NULL;
		m_uIntruderGUID = 0;
		m_uNumAutoBotsSoFar = 0;
		if (pIntruder && pLastIntruderPos_WS)
		{
			m_pIntruder = pIntruder;
			m_LastIntruderPos_WS = *pLastIntruderPos_WS;
			m_uIntruderGUID = pIntruder->Guid();
		}

		CFScriptSystem::TriggerEvent(CFScriptSystem::GetEventNumFromName("alarm"), (u32) this, 1, (u32) m_pIntruder);

	}

}


void CAlarmNet::TurnOff( BOOL bImmediately/*=FALSE*/ )
{
	if (m_uNetState != NETSTATE_OFF)
	{
		m_uNetState = NETSTATE_OFF;

		//go through and turn off all of the alarms in this net
		CAlarm* pAlarm = (CAlarm*) flinklist_GetHead(&m_AlarmList);
		while (pAlarm)
		{
			pAlarm->TurnOff( this, bImmediately );
			pAlarm = (CAlarm*) flinklist_GetNext(&m_AlarmList, pAlarm);
		}

		CAlarmDoor* pAlarmDoor = (CAlarmDoor*) flinklist_GetHead(&m_AlarmDoorList);
		while (pAlarmDoor)
		{
			pAlarmDoor->TurnOff( this, bImmediately );
			pAlarmDoor = (CAlarmDoor*) flinklist_GetNext(&m_AlarmDoorList, pAlarmDoor);
		}

		CBotDispenser* pBotDispenser = (CBotDispenser*) flinklist_GetHead(&m_DispenserList);
		while (pBotDispenser)
		{
			pBotDispenser->TurnOff( this, bImmediately );
			pBotDispenser = (CBotDispenser*) flinklist_GetNext(&m_DispenserList, pBotDispenser);
		}

		//free up any autobots I may have spawned, so that
		//they can be used by other nets if needed
		for (u32 i= 0; i < m_uAutoBotPoolSize; i++)
		{
			if (m_pauAutoBotNetUseage[i] == m_uAlarmNetId)
			{
				m_pauAutoBotNetUseage[i] = 0;
			}
		}

		m_pIntruder = NULL;
		m_uNetState = NETSTATE_OFF;
		m_fTurnOnTime = 0.0f;
		m_fActiveTime = 0.0f;
		m_uIntruderGUID = 0;
		m_uNumAutoBotsSoFar = 0;

		CFScriptSystem::TriggerEvent(CFScriptSystem::GetEventNumFromName("alarm"), (u32) this, 0, (u32) NULL);
	}
}


BOOL CAlarmNet::NetNeedsABot(void)
{
	BOOL bNeeds = m_uNumAutoBotsSoFar < m_uTotalBotsToAutoSpawn &&
					(m_nMaxNumLivingAutoBots == IGNORE_LIVING_AUTOBOT_LIMIT || CountAutoBotsUseByNet(m_uAlarmNetId) < m_nMaxNumLivingAutoBots);

	return bNeeds;
}

void CAlarmNet::HandleBotDeploys(void)
{
	CBotDispenser *pDispenser;
	s32 uAutoBotId = -1;

	if (aiutils_GetCurTimeSecs()<1)
	{
		//pgm added this so that there can be no autobots deployeed in the first secs of a level
		return; 
	}

	//dispensers can make autobots (they get priority so that pool doesn't get drained of dispenser-able bots by 
	//regular spawn points.
	if (NetNeedsABot())
	{
		pDispenser = (CBotDispenser *) flinklist_GetHead(&m_DispenserList);
		while (pDispenser)
		{
			if (pDispenser->NeedsAutoBot())
			{
				uAutoBotId = pDispenser->DeployAutoBot( m_papAutoBotPool, m_pauAutoBotNetUseage, m_uAutoBotPoolSize );
				if (uAutoBotId >-1)
				{
					m_uNumAutoBotsSoFar++;
					m_pauAutoBotNetUseage[uAutoBotId] = m_uAlarmNetId;
					if (!NetNeedsABot())
					{
						break;
					}
				}
			}
			pDispenser = (CBotDispenser *) flinklist_GetNext(&m_DispenserList, pDispenser);
		}
	}

	if (NetNeedsABot())
	{
		uAutoBotId = FindAvailableAutoBot();
		if (uAutoBotId >= 0)
		{
			DeployAutoBot((u8) uAutoBotId);
		}
	}
}


#include "player.h"
void CAlarmNet::Work(void)
{
	// always call the dispensers works, they may be finishing up despite the state of the net
	CBotDispenser *pDispenser = (CBotDispenser *) flinklist_GetHead(&m_DispenserList);
	while (pDispenser)
	{
		pDispenser->Work(this);
		pDispenser = (CBotDispenser *) flinklist_GetNext(&m_DispenserList, pDispenser);
	}

	switch (m_uNetState)
	{
		case NETSTATE_OFF:
#if 0// enable this to test alarms by randomly turning them on as if by script
			if( fmath_RandomChance( 0.01f ) ) {
				TurnOn( CPlayer::m_pCurrent->m_pEntityCurrent, &CPlayer::m_pCurrent->m_pEntityCurrent->MtxToWorld()->m_vPos);
			}
#endif
			break;
		case NETSTATE_TURNON:
			m_fTurnOnTime = aiutils_GetCurTimeSecs();
			m_fActiveTime = (f32) m_uActiveTimeMin;
			if (m_uActiveTimeMax > m_uActiveTimeMin)
			{
				m_fActiveTime += (f32) fmath_RandomChoice(m_uActiveTimeMax - m_uActiveTimeMin);
			}

			{
				//go through and turn on all of the alarms in this net
				CAlarm* pAlarm = (CAlarm*) flinklist_GetHead(&m_AlarmList);
				while (pAlarm)
				{
					pAlarm->TurnOn(this);
					pAlarm = (CAlarm*) flinklist_GetNext(&m_AlarmList, pAlarm);
				}

				//go through and turn on all of the dispensers in this net
				CBotDispenser *pDispenser = (CBotDispenser *) flinklist_GetHead(&m_DispenserList);
				while (pDispenser)
				{
					pDispenser->TurnOn(this);
					pDispenser = (CBotDispenser *) flinklist_GetNext(&m_DispenserList, pDispenser);
				}

				//Auto Bots - do this here before the doors start to open
				HandleBotDeploys();				

				//open doors
				CAlarmDoor* pAlarmDoor = (CAlarmDoor*) flinklist_GetHead(&m_AlarmDoorList);
				while (pAlarmDoor)
				{
					pAlarmDoor->TurnOn(this);
					pAlarmDoor = (CAlarmDoor*) flinklist_GetNext(&m_AlarmDoorList, pAlarmDoor);
				}
			}

			m_uNetState = NETSTATE_RUNNING;

			break;
		case NETSTATE_RUNNING:

			if ((!(m_uAlarmNetFlags & ALARMNETFLAG_NO_TIMER) && aiutils_GetCurTimeSecs() < m_fTurnOnTime + m_fActiveTime) ||
				((m_uAlarmNetFlags & ALARMNETFLAG_NO_TIMER) && m_uNumAutoBotsSoFar < m_uTotalBotsToAutoSpawn))
			{
				//go through all of the alarms in this net
				CAlarm* pAlarm = (CAlarm*) flinklist_GetHead(&m_AlarmList);
				while (pAlarm)
				{
					pAlarm->Work(this);
					pAlarm = (CAlarm*) flinklist_GetNext(&m_AlarmList, pAlarm);
				}

				/*	  //use this if doors ever need to do work

				CAlarmDoor* pAlarmDoor = (CAlarmDoor*) flinklist_GetHead(&m_AlarmDoorList);
				while (pAlarmDoor)
				{
					pAlarmDoor->Work(this);
					pAlarmDoor = (CAlarmDoor*) flinklist_GetNext(&m_AlarmDoorList, pAlarmDoor);
				}
				
				*/


				//Auto Bots
				HandleBotDeploys();


				//
				//   Update AlarmNet Knowledge about the Intruder
				//
				//rules for turning off early, or
				//extending minimum time
				if (m_pIntruder &&
					(!m_pIntruder->IsInWorld()  ||
					  m_pIntruder->Guid() != m_uIntruderGUID))
				{
					m_pIntruder = NULL;
				}
				else 
				{
					if (m_pIntruder)
					{
						if (aigroup_ShareKnowledgeOfEnemyLoc(m_pIntruder, &m_LastIntruderPos_WS))
						{
							int i  = 3;
						}
					}


					if (!m_pIntruder)
					{
						//see if we can find one
						for (u32 i= 0; i < m_uAutoBotPoolSize && !m_pIntruder; i++)
						{
							if (m_pauAutoBotNetUseage[i] == m_uAlarmNetId)
							{
								if (m_papAutoBotPool[i]->AIBrain())
								{
									m_pIntruder = ai_GetLastEnemySeen(m_papAutoBotPool[i]->AIBrain(), &m_LastIntruderPos_WS);
								}
							}
						}
					}
    			}

				if (m_pIntruder)
				{
					
					for (u32 i= 0; i < m_uAutoBotPoolSize; i++)
					{
						if (m_pauAutoBotNetUseage[i] == m_uAlarmNetId)
						{
							if (m_papAutoBotPool[i]->AIBrain() &&
								aibrainman_IsActive(m_papAutoBotPool[i]->AIBrain()) &&
								!m_papAutoBotPool[i]->IsUnderConstruction() &&
								!m_papAutoBotPool[i]->Power_IsPoweredDown())
							{
								if (!aiutils_IsFriendly(m_papAutoBotPool[i], m_pIntruder) &&
									!ai_IsAttacking(m_papAutoBotPool[i]->AIBrain()) )
								{
									// A freshly fabricated bot must clear the machine before
									// the alarm replaces its release movement with combat.
									BOOL bExiting = FALSE;
									for( CBotDispenser *pDispenser = (CBotDispenser *)flinklist_GetHead(&m_DispenserList);
										pDispenser; pDispenser = (CBotDispenser *)flinklist_GetNext(&m_DispenserList,pDispenser) ) {
										if( pDispenser->IsBotExiting(m_papAutoBotPool[i]) ) { bExiting = TRUE; break; }
									}
									if( bExiting ) continue;
									ai_AssignGoal_Attack(m_papAutoBotPool[i]->AIBrain(), m_pIntruder->Guid(), 0);
								}
								else
								{
									/*
									findfix: need some way for already fighting AI units to be told of updated
									         enemy location

									u16 uLastSeenWhen = 0;
									CEntity* pHisEnemy = ai_GetLastEnemySeen(m_papAutoBotPool[i]->AIBrain(),
																NULL,
																NULL,
																&uLastSeenWhen);
									if (!pHisEnemy || pHisEnemy->)
									{
										if (aiutils_GetCurTimeSecsU16() - uLastSeenWhen) > 5)
										{

										}
									}
									*/
								}
							}
						}
					}
				}
			}
			else
			{
				m_uNetState = NETSTATE_SHUTDOWN;
			}
			break;
		case NETSTATE_SHUTDOWN:
			TurnOff( FALSE );
			FASSERT(m_uNetState == NETSTATE_OFF);
			break;
	}

}


void CAlarmNet::AddAlarm(CAlarm* pAlarm)
{
	flinklist_AddTail(&m_AlarmList, pAlarm);
}


void CAlarmNet::AddAlarmDoor(CAlarmDoor* pAlarmDoor)
{
	flinklist_AddTail(&m_AlarmDoorList, pAlarmDoor);
}

void CAlarmNet::AddDispenser(CBotDispenser* pDispenser)
{
	flinklist_AddTail(&m_DispenserList, pDispenser);
}

void CAlarmNet::AddAlarmNet(CAlarmNet* pAlarmNet)
{
	pAlarmNet->m_uAlarmNetId = m_AlarmNetList.nCount+1;
	flinklist_AddTail(&m_AlarmNetList, pAlarmNet);
}


void CAlarmNet::InitSys(void)
{
	flinklist_InitRoot(&m_AlarmNetList, FANG_OFFSETOF(CAlarmNet, m_AlarmNetLink));
}


CAlarmNet* CAlarmNet::FindAlarmNet(u8 uAlarmNetId)
{
	CAlarmNet* pNet = (CAlarmNet*) flinklist_GetHead(&m_AlarmNetList);
	while (pNet)
	{
		if (pNet->m_uAlarmNetId == uAlarmNetId)
		{
			return pNet;
		}
		pNet =(CAlarmNet*) flinklist_GetNext(&m_AlarmNetList, pNet);
	}
	return NULL;
}


CAlarmNet* CAlarmNet::FindAlarmNet(cchar* pszAlarmNetName)
{
	if (pszAlarmNetName)
	{
		CAlarmNet* pNet = (CAlarmNet*) flinklist_GetHead(&m_AlarmNetList);
		while (pNet)
		{
			if (pNet->m_pszAlarmNetName && !fclib_stricmp(pNet->m_pszAlarmNetName, pszAlarmNetName))
			{
				return pNet;
			}
			pNet =(CAlarmNet*) flinklist_GetNext(&m_AlarmNetList, pNet);
		}
	}
	return NULL;
}


CAlarmNet* CAlarmNet::FindAlarmNet(CBotDispenser *pBotDispenser)
{
	CAlarmNet* pNet = (CAlarmNet*) flinklist_GetHead(&m_AlarmNetList);
	while (pNet)
	{
		CBotDispenser* pTempDispenser = (CBotDispenser*) flinklist_GetHead(&(pNet->m_DispenserList));
		while (pTempDispenser)
		{
			if (pTempDispenser == pBotDispenser)
			{
				return pNet;
			}
			pTempDispenser = (CBotDispenser*) flinklist_GetNext(&(pNet->m_DispenserList), pTempDispenser);
		}
		pNet = (CAlarmNet*) flinklist_GetNext(&m_AlarmNetList, pNet);
	}
	return NULL;
}


CAlarm* CAlarmNet::FindAlarm(CEntity* pAlarmEntity)
{
	CAlarmNet* pNet = (CAlarmNet*) flinklist_GetHead(&m_AlarmNetList);
	while (pNet)
	{
		CAlarm* pAlarm = (CAlarm*) flinklist_GetHead(&(pNet->m_AlarmList));
		while (pAlarm)
		{
			if (pAlarm->m_pAlarmEntity == pAlarmEntity)
			{
				return pAlarm;
			}
			pAlarm = (CAlarm*) flinklist_GetNext(&(pNet->m_AlarmList), pAlarm);
		}
		pNet = (CAlarmNet*) flinklist_GetNext(&m_AlarmNetList, pNet);
	}
	return NULL;
}


CBotDispenser* CAlarmNet::FindDispenser(CEntity* pDispenserEntity)
{
	CAlarmNet* pNet = (CAlarmNet*) flinklist_GetHead(&m_AlarmNetList);
	while (pNet)
	{
		CBotDispenser* pBotDispenser = (CBotDispenser*) flinklist_GetHead(&(pNet->m_DispenserList));
		while (pBotDispenser)
		{
			if (pBotDispenser->m_pDispenserEntity == pDispenserEntity)
			{
				return pBotDispenser;
			}
			pBotDispenser = (CBotDispenser*) flinklist_GetNext(&(pNet->m_DispenserList), pBotDispenser);
		}
		pNet = (CAlarmNet*) flinklist_GetNext(&m_AlarmNetList, pNet);
	}
	return NULL;
}


CAlarmDoor* CAlarmNet::FindAlarmDoor(CEntity* pAlarmEntity)
{
	CAlarmNet* pNet = (CAlarmNet*) flinklist_GetHead(&m_AlarmNetList);
	while (pNet)
	{
		CAlarmDoor* pAlarmDoor = (CAlarmDoor*) flinklist_GetHead(&(pNet->m_AlarmDoorList));
		while (pAlarmDoor)
		{
			if (pAlarmDoor->m_pAlarmDoorEntity == pAlarmEntity)
			{
				return pAlarmDoor;
			}
			pAlarmDoor = (CAlarmDoor*) flinklist_GetNext(&(pNet->m_AlarmDoorList), pAlarmDoor);
		}
		pNet = (CAlarmNet*) flinklist_GetNext(&m_AlarmNetList, pNet);
	}
	return NULL;
}


void CAlarmNet::WorkSys(void)
{
	CAlarmNet* pNet = (CAlarmNet*) flinklist_GetHead(&m_AlarmNetList);
	while (pNet)
	{
		pNet->Work();
		pNet =(CAlarmNet*) flinklist_GetNext(&m_AlarmNetList, pNet);
	}
}



//
//
//	CAlarmSwitch
//
//
FLinkRoot_t CAlarmSwitch::m_AlarmSwitchList;

CAlarmSwitch::CAlarmSwitch(void)
:	m_uAlarmNetId(0),
	m_fPressButtonTime(0.0f),
	m_fPressButtonTimeLeft(0.0f)
{
	m_AlarmSwitchLink.pPrevLink = NULL;
	m_AlarmSwitchLink.pNextLink = NULL;

}

void CAlarmSwitch::_ClearData(void)
{
	m_fPressButtonTimeLeft = 0.0f;
}


CAlarmSwitch::~CAlarmSwitch(void)
{
}


void CAlarmSwitch::AddAlarmSwitch(CAlarmSwitch* pAlarmSwitch)
{
	flinklist_AddTail(&m_AlarmSwitchList, pAlarmSwitch);
}


BOOL CAlarmSwitch::PressButton(CEntity* pFromWho, BOOL *pbDone /* NULL*/)					   //call this every frame, until it returns bDone = TRUE, returns false if switch is broken
{
	BOOL bDone = TRUE;

	if (pFromWho &&
		(pFromWho->TypeBits() & ENTITY_BIT_BOT) &&
		((CBot*) pFromWho)->m_nPossessionPlayerIndex == -1 &&		 //npc's must press the button for pressbuttontime.  humans on the first try.
		m_fPressButtonTimeLeft < m_fPressButtonTime)
	{
		m_fPressButtonTimeLeft += FLoop_fPreviousLoopSecs;
	}
	else
	{
		if (pbDone)
		{
			*pbDone = bDone;
		}
	}
	return TRUE;
}


BOOL CAlarmSwitch::IsWorking(void)
{
	return (m_pAlarmSwitchEntity && 
			m_pAlarmSwitchEntity->IsInWorld() && 
			m_pAlarmSwitchEntity->NormHealth() > 0.0f && 
			!((m_pAlarmSwitchEntity->TypeBits() & ENTITY_BIT_BOT) && ((CBot*) m_pAlarmSwitchEntity)->IsDeadOrDying()));

}


void CAlarmSwitch::Work(void)
{
}


CAlarmSwitch* CAlarmSwitch::Find(CEntity* pAlarmSwitchEntity)
{
	CAlarmSwitch* pSwitch = (CAlarmSwitch*) flinklist_GetHead(&m_AlarmSwitchList);
	while (pSwitch)
	{
		if (pSwitch->m_pAlarmSwitchEntity == pAlarmSwitchEntity)
		{
			return pSwitch;
		}
		pSwitch = (CAlarmSwitch*) flinklist_GetNext(&m_AlarmSwitchList, pSwitch);
	}
	return NULL;
}


void CAlarmSwitch::InitSys(void)
{
	flinklist_InitRoot(&CAlarmSwitch::m_AlarmSwitchList,FANG_OFFSETOF(CAlarmSwitch, m_AlarmSwitchLink));
}


void CAlarmSwitch::WorkSys(void)
{
	CAlarmSwitch* pSwitch = (CAlarmSwitch*) flinklist_GetHead(&m_AlarmSwitchList);
	while (pSwitch)
	{
		pSwitch->Work();
		pSwitch = (CAlarmSwitch*) flinklist_GetNext(&m_AlarmSwitchList, pSwitch);
	}
}


//
//
//	  CAlarmDoor
//
//
CAlarmDoor::CAlarmDoor(void)
:   m_uAlarmNetId(0),
	m_uAlarmDoorFlags(ALARMDOORFLAG_ALARM_NONE),
	m_pAlarmDoorEntity(NULL)
{
	m_AlarmDoorLink.pNextLink = NULL;
	m_AlarmDoorLink.pPrevLink = NULL;
}


CAlarmDoor::~CAlarmDoor(void)
{
}


void CAlarmDoor::TurnOn(CAlarmNet* pNet)
{
	CDoorEntity* pDoor = NULL;

	if (m_pAlarmDoorEntity && m_pAlarmDoorEntity->TypeBits() & ENTITY_BIT_DOOR)
	{
		pDoor = (CDoorEntity*) m_pAlarmDoorEntity;

		if (m_uAlarmDoorFlags & ALARMDOORFLAG_ALARM_ON_UNLOCKS)
		{
			pDoor->SetLockState(FALSE);
		}
		else if (m_uAlarmDoorFlags & ALARMDOORFLAG_ALARM_ON_LOCKS)
		{
			pDoor->SetLockState(TRUE);
		}

		if (m_uAlarmDoorFlags & ALARMDOORFLAG_ALARM_ON_OPENS)
		{
			pDoor->GotoPos(1, CDoorEntity::GOTOREASON_DESTINATION);
		}
		else if (m_uAlarmDoorFlags & ALARMDOORFLAG_ALARM_ON_SHUTS)
		{
			pDoor->GotoPos(0, CDoorEntity::GOTOREASON_DESTINATION);
		}
	}
}


void CAlarmDoor::TurnOff(CAlarmNet* pNet, BOOL bImmediately/*=FALSE*/)
{
	CDoorEntity* pDoor = NULL;

	if (m_pAlarmDoorEntity && m_pAlarmDoorEntity->TypeBits() & ENTITY_BIT_DOOR)
	{
		pDoor = (CDoorEntity*) m_pAlarmDoorEntity;

		if (m_uAlarmDoorFlags & ALARMDOORFLAG_ALARM_OFF_UNLOCKS)
		{
			pDoor->SetLockState(TRUE);
		}
		else if (m_uAlarmDoorFlags & ALARMDOORFLAG_ALARM_OFF_LOCKS)
		{
			pDoor->SetLockState(FALSE);
		}

		if (m_uAlarmDoorFlags & ALARMDOORFLAG_ALARM_OFF_OPENS)
		{
			pDoor->GotoPos(0, CDoorEntity::GOTOREASON_DESTINATION);
		}
		else if (m_uAlarmDoorFlags & ALARMDOORFLAG_ALARM_OFF_SHUTS)
		{
			pDoor->GotoPos(1, CDoorEntity::GOTOREASON_DESTINATION);
		}
	}
}


//
//
//	CAlarmSysBuilder
//
//
void CAlarmSysBuilder::SetDefaults( CEntityBuilder* pBuilder )
{
	BuilderHelp_SetDefaults(this, _aItableDataSpec, sizeof(_aItableDataSpec)/sizeof(InterpretTableDataSpec));

	m_bIsAlarm = FALSE;
	m_uAlarmNotifyRad = 25;
	m_pszAlarmNetName = NULL;
	m_uActiveTimeMin = 0xffffffff;
	m_uActiveTimeMax = 0xffffffff;
	m_uTotalBotsToAutoSpawn = 0xffffffff;
	m_uAlarmNetFlags = 0x0;
	m_bIsAlarmSwitch = FALSE;
	m_bIsAlarmDoor = FALSE;
	m_bIsHiddenAlarmSpawnSpline = FALSE;
	m_bIsVisibleAlarmSpawnSpline = FALSE;
	m_bIsDoorwayAlarmSpawnSpline = FALSE;
	m_fSwitchPressTime = 0.0f;
	m_nMaxNumLivingAutoBots = 0xffffffff;
	m_pszBotGroupName = NULL;
	m_bIsDispenser = FALSE;
	CBotDispenser::SetInitDataToDefaults( &m_DispenserInit );
}


BOOL CAlarmSysBuilder::InterpretTable( CEntityBuilder* pBuilder )
{
	BOOL bDidInterpret = FALSE;
	if (CEntityParser::m_pszTableName && !fclib_strnicmp(CEntityParser::m_pszTableName, "XA_", 3))
	{
		BuilderHelp_InterpretTable(_aItableDataSpec, sizeof(_aItableDataSpec)/sizeof(InterpretTableDataSpec), &bDidInterpret);
	}

	if( !bDidInterpret && !fclib_stricmp( CEntityParser::m_pszTableName, "XA_ISAUTOBOT_SPAWN_PTS" ) )
	{
		DEVPRINTF("XA_Error: is using \"XA_ISAUTOBOT_SPAWN_PTS\". It is old as mold. Please use \"XA_ISHIDDEN_SPAWN_PTS\" .\n");
		m_bIsHiddenAlarmSpawnSpline = TRUE;
		bDidInterpret = TRUE;
	}

	return bDidInterpret;
}


BOOL CAlarmSysBuilder::PostInterpretFixup( CEntityBuilder* pBuilder )
{
	if (m_pszAlarmNetName)
	{
		int i = 4;
	}

	if (m_pszAlarmNetName && !(m_pszAlarmNetName = gstring_Main.AddString( m_pszAlarmNetName)) )
	{  //couldn't fit that string into the global table
		goto _XOutOfMemory;
	}


	if (m_pszBotGroupName && !(m_pszBotGroupName = gstring_Main.AddString( m_pszBotGroupName)) )
	{  //couldn't fit that string into the global table
		goto _XOutOfMemory;
	}

	return TRUE;

_XOutOfMemory:
	CEntityParser::Error_OutOfMemory();
	return FALSE;
}

CAlarmNet* CAlarmSysBuilder::_AllocOrFindExistingAlarmNet(CEntity* pEntity)
{
	CAlarmNet* pAlarmNet = NULL;
	if (!m_pszAlarmNetName)
	{
		DEVPRINTF("XA_Error: Alarm (%s) with no Alarm Net Name.\n", (pEntity && pEntity->Name()) ? pEntity->Name(): "NONAME");
		if( !m_bIsDispenser ) {
			return NULL;
		} else {
			m_pszAlarmNetName = _pszDispenserSlopBucketName;
		}
	}
	pAlarmNet = CAlarmNet::FindAlarmNet(m_pszAlarmNetName);
	if (!pAlarmNet)
	{
		pAlarmNet = fnew CAlarmNet();
		if (pAlarmNet)
		{
			CAlarmNet::AddAlarmNet(pAlarmNet);
			pAlarmNet->m_pszAlarmNetName = m_pszAlarmNetName;
		}
	}
	return pAlarmNet;
}



BOOL CAlarmSysBuilder::BuildAlarmEntityIfNeeded( CEntity* pEntity)
{
	FResFrame_t ResFrame = fres_GetFrame();
	CAlarmNet* pAlarmNet = NULL;

	if (m_bIsAlarm ||
		m_bIsAlarmSwitch ||
		m_bIsAlarmDoor ||
		m_bIsHiddenAlarmSpawnSpline ||
		m_bIsVisibleAlarmSpawnSpline ||
		m_bIsDoorwayAlarmSpawnSpline ||
		m_bIsDispenser)
	{
		pAlarmNet = _AllocOrFindExistingAlarmNet(pEntity);
		if (!pAlarmNet)
		{
			goto _ExitBuildAlarmEntityIfNeeded_WithError;
		}
	}

	if (m_bIsAlarm)
	{
		CAlarm* pAlarm = fnew CAlarm();
		if (!pAlarm)
		{
			goto _ExitBuildAlarmEntityIfNeeded_WithError;
		}

		//Initialize the alarm
		pAlarm->m_uAlarmNetId = pAlarmNet->m_uAlarmNetId;
		pAlarm->m_pAlarmEntity = pEntity;
		pAlarm->m_uAlarmNotifyRad = (u16) m_uAlarmNotifyRad;
		pAlarmNet->AddAlarm(pAlarm);

		pEntity->SetAlarmSysUse(TRUE);
	}
	else if (m_bIsAlarmSwitch)
	{
		CAlarmSwitch* pSwitch = fnew CAlarmSwitch();
		if (!pSwitch)
		{
			goto _ExitBuildAlarmEntityIfNeeded_WithError;
		}

		//Initialize the switch
		pSwitch->m_uAlarmNetId = pAlarmNet->m_uAlarmNetId;
		pSwitch->m_pAlarmSwitchEntity = pEntity;
		pSwitch->m_fPressButtonTime = m_fSwitchPressTime;
		pSwitch->m_fPressButtonTimeLeft = 0.0f;
		CAlarmSwitch::AddAlarmSwitch(pSwitch);
		pEntity->SetAlarmSysUse(TRUE);
	}
	else if (m_bIsAlarmDoor)
	{
		CAlarmDoor* pDoor = fnew CAlarmDoor();
		if (!pDoor)
		{
			goto _ExitBuildAlarmEntityIfNeeded_WithError;
		}

		//Initialize the Door
		pDoor->m_uAlarmNetId = pAlarmNet->m_uAlarmNetId;
		pDoor->m_pAlarmDoorEntity = pEntity;
		pDoor->m_uAlarmDoorFlags = m_uAlarmDoorFlags & 0xff;
		pAlarmNet->AddAlarmDoor(pDoor);
		pEntity->SetAlarmSysUse(TRUE);
	}
	else if (m_bIsVisibleAlarmSpawnSpline)
	{
		if (m_bIsHiddenAlarmSpawnSpline || m_bIsDoorwayAlarmSpawnSpline)
		{
			DEVPRINTF("XA_Error: Alarm autospawnp spline (%s) can't be more than one type at a time.\n", pEntity->Name() ? pEntity->Name(): "NONAME");
		}
		if (!(pEntity->TypeBits() & ENTITY_BIT_SPLINE))
		{
			DEVPRINTF("XA_Error: Alarm autospawnp spline (%s) not a spline object.\n", pEntity->Name() ? pEntity->Name(): "NONAME");
		}
		else 
		{
			pAlarmNet->m_pVisibleSpawnPts = (CESpline*) pEntity;
		}
	}
	else if (m_bIsHiddenAlarmSpawnSpline)
	{
		if (m_bIsVisibleAlarmSpawnSpline || m_bIsDoorwayAlarmSpawnSpline)
		{
			DEVPRINTF("XA_Error: Alarm autospawnp spline (%s) can't be more than one type at a time.\n", pEntity->Name() ? pEntity->Name(): "NONAME");
		}

		if (!(pEntity->TypeBits() & ENTITY_BIT_SPLINE))
		{
			DEVPRINTF("XA_Error: Alarm autospawnp spline (%s) not a spline object.\n", pEntity->Name() ? pEntity->Name(): "NONAME");
		}
		else 
		{
			pAlarmNet->m_pHiddenSpawnPts = (CESpline*) pEntity;
		}
	}
	else if (m_bIsDoorwayAlarmSpawnSpline)
	{
		if (m_bIsHiddenAlarmSpawnSpline || m_bIsVisibleAlarmSpawnSpline)
		{
			DEVPRINTF("XA_Error: Alarm autospawnp spline (%s) can't be more than one type at a time.\n", pEntity->Name() ? pEntity->Name(): "NONAME");
		}
		if (!(pEntity->TypeBits() & ENTITY_BIT_SPLINE))
		{
			DEVPRINTF("XA_Error: Alarm autospawnp spline (%s) not a spline object\n", pEntity->Name() ? pEntity->Name(): "NONAME");
		}
		else 
		{
			pAlarmNet->m_pDoorwaySpawnPts = (CESpline*) pEntity;
		}
	}
	else if (m_bIsDispenser)
	{
		if (!(pEntity->TypeBits() & ENTITY_BIT_MESHENTITY))
		{
			DEVPRINTF("XA_Error: Dispenser object (%s) is not a mesh entity\n", pEntity->Name() ? pEntity->Name(): "NONAME");
		}
		else 
		{
			CBotDispenser *pDispense = fnew CBotDispenser();
			if (!pDispense)
			{
				goto _ExitBuildAlarmEntityIfNeeded_WithError;
			}
			//Initialize the dispenser
			if( !pDispense->Init( &m_DispenserInit, pEntity ) ) {
				fdelete( pDispense );
				goto _ExitBuildAlarmEntityIfNeeded_WithError;
			}
			pAlarmNet->AddDispenser(pDispense);
			pEntity->SetAlarmSysUse(TRUE);
		}		
	}

	if (pAlarmNet)
	{
		BuilderHelp_SetU8IfValid(&(pAlarmNet->m_uActiveTimeMax), m_uActiveTimeMax);
		BuilderHelp_SetU8IfValid(&(pAlarmNet->m_uActiveTimeMin), m_uActiveTimeMin);
		BuilderHelp_SetU8IfValid(&(pAlarmNet->m_uTotalBotsToAutoSpawn), m_uTotalBotsToAutoSpawn);
		BuilderHelp_SetU16IfValid(&(pAlarmNet->m_uAlarmNetFlags), m_uAlarmNetFlags);
		BuilderHelp_SetS8IfValid(&(pAlarmNet->m_nMaxNumLivingAutoBots), m_nMaxNumLivingAutoBots);
		pAlarmNet->m_pszBotGroupName = m_pszBotGroupName;
	}

	return TRUE;
_ExitBuildAlarmEntityIfNeeded_WithError:
	DEVPRINTF("XA_Error alarm builder(): No Memory to allocate alarm object.\n" );
	fres_ReleaseFrame(ResFrame);
	return FALSE;
}


//
//
//	Alarm API
//
//

BOOL AlarmSys_InitLevel(void)
{
	_fSecsBeforeTurningDispenserNetOn = -1.0f;
	_pGlobalDispenserNet = NULL;

	CAlarmNet::InitSys();
	CAlarmSwitch::InitSys();	
	return TRUE;
}


void AlarmSys_UninitLevel(void)
{
	//go through and delete all alarms, doors and
	CAlarmNet* pNet = (CAlarmNet*) flinklist_GetHead(&CAlarmNet::m_AlarmNetList);
	while (pNet)
	{
		CAlarmDoor* pAlarmDoor = (CAlarmDoor*) flinklist_GetHead(&(pNet->m_AlarmDoorList));
		while (pAlarmDoor)
		{
			CAlarmDoor* pDeleteDoor = pAlarmDoor;
			pAlarmDoor = (CAlarmDoor*) flinklist_GetNext(&(pNet->m_AlarmDoorList), pAlarmDoor);
			fdelete (pDeleteDoor);
		}

		CBotDispenser* pDispenser = (CBotDispenser*) flinklist_GetHead(&(pNet->m_DispenserList));
		while (pDispenser)
		{
			CBotDispenser* pDeleteDispenser = pDispenser;
			pDispenser = (CBotDispenser*) flinklist_GetNext(&(pNet->m_DispenserList), pDispenser);
			fdelete (pDeleteDispenser);
		}

		CAlarm* pAlarm = (CAlarm*) flinklist_GetHead(&(pNet->m_AlarmList));
		while (pAlarm)
		{
			CAlarm* pDeleteAlarm = pAlarm;
			pAlarm = (CAlarm*) flinklist_GetNext(&(pNet->m_AlarmList), pAlarm);
			fdelete ( pDeleteAlarm);
		}

		CAlarmNet* pDeleteNet = pNet;
		pNet =(CAlarmNet*) flinklist_GetNext(&CAlarmNet::m_AlarmNetList, pNet);
		fdelete (pDeleteNet);
	}
	flinklist_InitRoot(&CAlarmNet::m_AlarmNetList, FANG_OFFSETOF(CAlarmNet, m_AlarmNetLink));
	
	//go through and delete all switches
	CAlarmSwitch* pAlarmSwitch = (CAlarmSwitch*) flinklist_GetHead(&CAlarmSwitch::m_AlarmSwitchList);
	while (pAlarmSwitch)
	{
		CAlarmSwitch* pDeleteSwitch = pAlarmSwitch;
		pAlarmSwitch =(CAlarmSwitch*) flinklist_GetNext(&CAlarmSwitch::m_AlarmSwitchList, pAlarmSwitch);
		fdelete (pDeleteSwitch);
	}
	flinklist_InitRoot(&CAlarmSwitch::m_AlarmSwitchList, FANG_OFFSETOF(CAlarmSwitch, m_AlarmSwitchLink));

	//delete mem for managing autobots
	CAlarmNet::CleanupAutoBots();

	_fSecsBeforeTurningDispenserNetOn = -1.0f;
	_pGlobalDispenserNet = NULL;
}


void AlarmSys_Work(void)
{
	// see if we should turn on the global dispenser network
	if( _pGlobalDispenserNet && _fSecsBeforeTurningDispenserNetOn >= 0.0f ) {
		_fSecsBeforeTurningDispenserNetOn -= FLoop_fPreviousLoopSecs;
		if( _fSecsBeforeTurningDispenserNetOn <= 0.0f ) {
			// turn on the global net
			_fSecsBeforeTurningDispenserNetOn = -1.0f;
			_pGlobalDispenserNet->TurnOn( NULL,NULL );
		}
	}

	CAlarmNet::UpdateAutoBotUseage();
	CAlarmNet::WorkSys();
	CAlarmSwitch::WorkSys();
}

void AlarmSys_CheckpointRestore(void)
{
	CAlarmNet* pNet = (CAlarmNet*) flinklist_GetHead(&CAlarmNet::m_AlarmNetList);
	while (pNet)
	{
		pNet->TurnOff( TRUE );
		pNet =(CAlarmNet*) flinklist_GetNext(&CAlarmNet::m_AlarmNetList, pNet);
	}

	CAlarmSwitch* pSwitch = (CAlarmSwitch*) flinklist_GetHead(&CAlarmSwitch::m_AlarmSwitchList);
	while (pSwitch)
	{
		pSwitch->_ClearData();
		pSwitch =(CAlarmSwitch*) flinklist_GetNext(&CAlarmSwitch::m_AlarmSwitchList, pSwitch);
	}
	
	for (u32 i= 0; i < CAlarmNet::m_uAutoBotPoolSize; i++)
	{
		CBot* pBot = CAlarmNet::m_papAutoBotPool[i];

		if( pBot->IsInWorld() )
		{
			pBot->RemoveFromWorld(TRUE);
		}
	}

	//update vbls that were tracking autobots
	CAlarmNet::UpdateAutoBotUseage();

	if( _pGlobalDispenserNet ) {
		_fSecsBeforeTurningDispenserNetOn = _SECS_TO_WAIT_BEFORE_TURNING_SLOP_DISPENSER_NET_ON;
	}
}


void AlarmSys_CheckpointRestorePostamble(void)
{
	//just incase any autobots got brought back
	//remove them again
	for (u32 i= 0; i < CAlarmNet::m_uAutoBotPoolSize; i++)
	{
		CBot* pBot = CAlarmNet::m_papAutoBotPool[i];

		if( pBot->IsInWorld() )
		{
			pBot->RemoveFromWorld(TRUE);
		}
	}
}


void AlarmSys_CheckpointSave(void)
{
	//findfix:
}


void AlarmSys_ActionNearby(CEntity* pFromEntity, CEntity* pToEntity)
{
	CAlarmSwitch* pSwitch = CAlarmSwitch::Find(pToEntity);
	if (pSwitch)
	{
		BOOL bDone = FALSE;
		pSwitch->PressButton(pFromEntity, &bDone);
		if (bDone)
		{
			CAlarmNet* pNet = CAlarmNet::FindAlarmNet(pSwitch->m_uAlarmNetId);
			FASSERT(pNet);
			CFVec3A* pIntruderPos_WS = NULL;
			CEntity* pIntruder = NULL;

			if (pFromEntity)
			{
				if (aiutils_IsPlayer(pFromEntity))
				{
					pIntruder = pFromEntity;
					if (pIntruder)
					{
						pIntruderPos_WS = &(pNet->m_LastIntruderPos_WS);
					}
				}
				else
				{	
					if (pFromEntity->AIBrain())
					{
						pIntruder = ai_GetLastEnemySeen(pFromEntity->AIBrain(), &(pNet->m_LastIntruderPos_WS));
						if (pIntruder)
						{
							pIntruderPos_WS = &(pNet->m_LastIntruderPos_WS);
						}
					}
				}
			}
			pNet->TurnOn(pIntruder, pIntruderPos_WS);
		}

	}
}


CAlarmNet* AlarmSys_GetAutoBotNet(CBot* pBot)
{
	CAlarmNet* pNet = NULL;
	for (s32 i = 0; i < CAlarmNet::m_uAutoBotPoolSize; i++)
	{
		if (CAlarmNet::m_papAutoBotPool[i] == pBot)
		{
			if (pBot->IsInWorld() && CAlarmNet::m_pauAutoBotNetUseage[i] != 0)
			{
				pNet = CAlarmNet::FindAlarmNet(CAlarmNet::m_pauAutoBotNetUseage[i]);
				break;
			}
		}
	}
	return pNet;
}

u8 AlarmSys_GetAutoBotNetId(CBot* pBot)
{
	if (pBot)
	{
		CAlarmNet* pNet = NULL;
		for (s32 i = 0; i < CAlarmNet::m_uAutoBotPoolSize; i++)
		{
			if (CAlarmNet::m_papAutoBotPool[i] == pBot)
			{
				return CAlarmNet::m_pauAutoBotNetUseage[i];
			}
		}
	}
	return 0;
}


//   After all entities have been created, this function gets called so entities that are to be attached to other entities can be done.
void AlarmSys_ResolveEntityPointerFixups( void )
{
	CBotDispenser *pRemove, *pBotDispenser;
	u32 nTotalDispenserCount;
	u32 nMaxDispensersInOneNet, nNetDispenserCount;
	u32 nMaxDispensedBotBoneCount, nBoneCount;

	// resolve the auto bots
	CAlarmNet::InitAutoBots();

	CAlarmNet::InitAllSpawnPts();

	// resolve the dispensers across the nets (gather some info while we are in the process)
	nTotalDispenserCount = nMaxDispensersInOneNet = nMaxDispensedBotBoneCount = nNetDispenserCount = 0;
	CAlarmNet* pNet = (CAlarmNet*) flinklist_GetHead(&CAlarmNet::m_AlarmNetList);
	while (pNet)
	{
		// walk all of the dispensers in this net

		pBotDispenser = (CBotDispenser *)flinklist_GetHead( &pNet->m_DispenserList );
		while (pBotDispenser)
		{
			if( !pBotDispenser->ResolveFixups( pNet->m_papAutoBotPool, pNet->m_uAutoBotPoolSize, nBoneCount ) ) {
				pRemove = pBotDispenser;
			} else {
				pRemove = NULL;
				nMaxDispensedBotBoneCount = FMATH_MAX( nMaxDispensedBotBoneCount, nBoneCount );
			}
			pBotDispenser = (CBotDispenser*) flinklist_GetNext(&(pNet->m_DispenserList), pBotDispenser);

			if( pRemove ) {
				flinklist_Remove( &pNet->m_DispenserList, pRemove );
				fdelete( pRemove );
			}
		}

		nNetDispenserCount = pNet->m_DispenserList.nCount;
		nTotalDispenserCount += nNetDispenserCount;
		nMaxDispensersInOneNet = FMATH_MAX( nNetDispenserCount, nMaxDispensersInOneNet );

		// move to the next net
		pNet = (CAlarmNet*) flinklist_GetNext(&CAlarmNet::m_AlarmNetList, pNet);
	}

	// now that we know which bots are going to be dispensed, we can init the mesh builder system
	if( nTotalDispenserCount ) {
		if( nTotalDispenserCount == nMaxDispensersInOneNet ) {
			// all of the dispensers are in a single net, enable them all
            nNetDispenserCount = nTotalDispenserCount;
		} else {
			// allow 1 extra dispenser than the max net size
			nNetDispenserCount = nMaxDispensersInOneNet + 1;
		}
		CFXMeshBuildPartMgr::SubmitInitNeeds( nMaxDispensedBotBoneCount, nNetDispenserCount );

		if (MultiplayerMgr.PossessionEnabled()) {
			_pGlobalDispenserNet = CAlarmNet::FindAlarmNet(_pszDispenserSlopBucketName);
			if( _pGlobalDispenserNet ) {
				//this one is always on
				_pGlobalDispenserNet->m_uAlarmNetFlags |= CAlarmNet::ALARMNETFLAG_NO_TIMER;
				//_pGlobalDispenserNet->TurnOn( NULL,NULL );
				_fSecsBeforeTurningDispenserNetOn = _SECS_TO_WAIT_BEFORE_TURNING_SLOP_DISPENSER_NET_ON;
			}
		}
	} else {
		_pGlobalDispenserNet = NULL;
	}
}



