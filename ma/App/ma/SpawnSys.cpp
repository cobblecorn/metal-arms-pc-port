#include "fang.h"
#include "spawnsys.h"
#include "fgamedata.h"
#include "fclib.h"
#include "fvis.h"
#include "botswarmer.h"
#include "builderhelp.h"
#include "entity.h"
#include "espline.h"
#include "door.h"
#include "player.h"
#include "AI\AIgameutils.h"
#include "AI\AIEnviro.h"
#include "gstring.h"


//
//
//	CSpawnSys
//
//	 statics
CBot* CSpawnSys::m_paAutoBotAlloc[SPAWNSYS_NUM_BOTTYPES] = {NULL,NULL};
CBot** CSpawnSys::m_papAutoBotPool[SPAWNSYS_NUM_BOTTYPES] = {NULL,NULL};
u16* CSpawnSys::m_pauAutoBotNetUseage[SPAWNSYS_NUM_BOTTYPES] = {0,0};
u16 CSpawnSys::m_auAutoBotPoolSize[SPAWNSYS_NUM_BOTTYPES] = {0,0};
CSpawnSys::CSpawnBotSave* CSpawnSys::m_paSavePool = NULL;
u32 CSpawnSys::m_uSavePoolSize = 0;;
FLinkRoot_t CSpawnSys::m_SpawnNetList;
extern FGameDataFileHandle_t Level_hLevelDataFile;

BOOL CSpawnSys::InitLevel(void)
{
	FResFrame_t ResFrame = fres_GetFrame();
	for (u32 uBotType = 0; uBotType <  SPAWNSYS_NUM_BOTTYPES; uBotType++)
	{
		m_paAutoBotAlloc[uBotType] = NULL;
		m_papAutoBotPool[uBotType] = NULL;
		m_pauAutoBotNetUseage[uBotType] = NULL;
		m_auAutoBotPoolSize[uBotType] = 0;
	}
	m_paSavePool = NULL;	   //get's initialized later.
	m_uSavePoolSize = 0;
	flinklist_InitRoot(&m_SpawnNetList, FANG_OFFSETOF(CSpawnNet, m_SpawnNetLink));
	return TRUE;

//ExitSpawnSysWithError:
	fres_ReleaseFrame(ResFrame);
	return FALSE;
}


void CSpawnSys::UninitLevel(void)
{
	for (u32 uBotType = 0; uBotType <  SPAWNSYS_NUM_BOTTYPES; uBotType++)
	{
		if (m_auAutoBotPoolSize[uBotType])
		{
			if (m_paAutoBotAlloc[uBotType])
			{
//ARG - >>>>>
#if FANG_PLATFORM_PS2
//ARG - FIX for weird crash freeing NULL pointer
				fdelete_array((CBotSwarmer *)m_paAutoBotAlloc[uBotType]);
#else
//ARG - <<<<<
				fdelete_array(m_paAutoBotAlloc[uBotType]);
#endif	//ARG
			}
			m_papAutoBotPool[uBotType] = NULL;			//no delete since fres'd it
			m_pauAutoBotNetUseage[uBotType] = NULL;		//no delete since fres'd it
			m_auAutoBotPoolSize[uBotType] = 0;			//no delete since fres'd it
		}
	}


	CSpawnNet* pNet = (CSpawnNet*) flinklist_RemoveHead(&m_SpawnNetList);
	while (pNet)
	{
		fdelete (pNet);
		pNet = (CSpawnNet*) flinklist_RemoveHead(&m_SpawnNetList);
	}


}


BOOL CSpawnSys::InitAutoBots(void)
{
	FGameData_VarType_e nDataType;
	FGameDataTableHandle_t hTableHandle;

	FResFrame_t ResFrame = fres_GetFrame();

	cchar* pszTag[] = {"Num_SwarmerBots",
						"Num_VerminBots"};
	cchar* pszErr1[] = {"CSpawnSys() : Wrong number of parameters for Num_SwarmerBots table in level.csv. Use only one.\n",
						"CSpawnSys() : Wrong number of parameters for Num_VerminBots table in level.csv. Use only one.\n"};
	cchar* pszErr2[] = {"CSpawnSys() : Wrong type of parameter for Num_SwarmerBots table in level.csv. Make it a number\n",
						"CSpawnSys() : Wrong type of parameter for Num_VerminBots table in level.csv. Make it a number\n"};

	for (u32 uBotType = 0; uBotType <  SPAWNSYS_NUM_BOTTYPES; uBotType++)
	{
		m_auAutoBotPoolSize[uBotType] = 0;
		hTableHandle = fgamedata_GetFirstTableHandle( Level_hLevelDataFile, pszTag[uBotType] );
		if( hTableHandle != FGAMEDATA_INVALID_TABLE_HANDLE )
		{
			// get the number of fields in the table
			if( fgamedata_GetNumFields(hTableHandle) != 1 )
			{
				DEVPRINTF( pszErr1[uBotType] );
			}
			else
			{
				m_auAutoBotPoolSize[uBotType] = (u16) (*(f32*)fgamedata_GetPtrToFieldData( hTableHandle, 0, nDataType ));
				if( nDataType == FGAMEDATA_VAR_TYPE_FLOAT)
				{
					if (m_auAutoBotPoolSize[uBotType])
					{
						CBotSwarmer* paSwarmers = fnew CBotSwarmer[m_auAutoBotPoolSize[uBotType]];
						m_paAutoBotAlloc[uBotType] = paSwarmers;
						m_papAutoBotPool[uBotType] = (CBot**) fres_AlignedAllocAndZero(m_auAutoBotPoolSize[uBotType]*sizeof(CBot*), 16);
						m_pauAutoBotNetUseage[uBotType] = (u16*) fres_AlignedAllocAndZero(m_auAutoBotPoolSize[uBotType]*sizeof(u16), 16);
						if (!paSwarmers || !m_papAutoBotPool[uBotType] || !m_pauAutoBotNetUseage[uBotType])
						{
							goto ExitAutoBotInitWithError;
						}

						for (u32 i = 0; i < m_auAutoBotPoolSize[uBotType]; i++)
						{
							m_papAutoBotPool[uBotType][i] = paSwarmers+i;
							paSwarmers[i].Create(-1,//s32 nPlayerIndex=-1,
												FALSE, //BOOL bInstallDataPort=FALSE,
												"SPAWNSYS",//cchar *pszEntityName=NULL,
												NULL,//const CFMtx43A *pMtx=NULL,
												NULL,//cchar *pszAIBuilderName=NULL );
												(CBotSwarmer::BotType_e) uBotType);		  //!BotType means vermin, or swarmer
							//findfix: need some code here to make a swarmer into a vermin
							paSwarmers[i].RemoveFromWorld(TRUE);
						}
					}
				}
				else
				{
					DEVPRINTF( pszErr2[uBotType] );
					m_auAutoBotPoolSize[uBotType] = 0;
				}
			}
		}
	}

	return TRUE;
ExitAutoBotInitWithError:

	DEVPRINTF("Swarmsys: out of memory\n");

	for (u32 uBotType = 0; uBotType <  SPAWNSYS_NUM_BOTTYPES; uBotType++)
	{
		m_paAutoBotAlloc[uBotType] = NULL;
		m_papAutoBotPool[uBotType] = NULL;
		m_pauAutoBotNetUseage[uBotType] = NULL;
		m_auAutoBotPoolSize[uBotType] = NULL;
	}
	fres_ReleaseFrame(ResFrame);
	return FALSE;
}


void CSpawnSys::Work(void)
{
	UpdateAutoBotUseage();

	CSpawnNet* pNet = (CSpawnNet*) flinklist_GetHead(&m_SpawnNetList);
	while (pNet)
	{
		pNet->Work();
		pNet =(CSpawnNet*) flinklist_GetNext(&m_SpawnNetList, pNet);
	}
}


void CSpawnSys::CheckpointRestore(void)
{

	CSpawnNet* pNet = (CSpawnNet*) flinklist_GetHead(&m_SpawnNetList);
	while (pNet)
	{
		pNet->CheckPointRestore();
		pNet =(CSpawnNet*) flinklist_GetNext(&m_SpawnNetList, pNet);
	}

	for (u32 uBotType = 0; uBotType <  SPAWNSYS_NUM_BOTTYPES; uBotType++)
	{
		for (u32 i= 0; i < m_auAutoBotPoolSize[uBotType]; i++)
		{
			if (m_papAutoBotPool[uBotType][i]->IsInWorld() &&
					 (m_papAutoBotPool[uBotType][i]->TypeBits() & ENTITY_BIT_BOT && ((CBot*) (m_papAutoBotPool[uBotType][i]))->IsDead()))
			{
				m_pauAutoBotNetUseage[uBotType][i] = 0;	//up for grabs by any net
				m_papAutoBotPool[uBotType][i]->RemoveFromWorld();
			}
		}
	}
}


void CSpawnSys::CheckpointSave(void)
{
}


void CSpawnSys::ResolveEntityPointerFixups( void )		//   After all entities have been created, this function gets called so entities that are to be attached to other entities can be done
{

	InitAutoBots();

	CSpawnNet* pNet = (CSpawnNet*) flinklist_GetHead(&m_SpawnNetList);
	while (pNet)
	{
		pNet->InitSpawnPts();
		pNet =(CSpawnNet*) flinklist_GetNext(&m_SpawnNetList, pNet);
	}
}


void CSpawnSys::TripwireInside(CEntity* pTripwire, CEntity* pTripper)
{
	CSpawnNetTripwire* pSpawnNetTripwire = FindSpawnNetTripwire(pTripwire);
	if (pSpawnNetTripwire && pSpawnNetTripwire->m_pNet && pSpawnNetTripwire->IsActive())
	{
		pSpawnNetTripwire->m_pNet->SetDefconLevel(pSpawnNetTripwire->m_uForceDefconLevel);
		pSpawnNetTripwire->m_pNet->m_pIntruder = pTripper;
	}
}



CSpawnNet* CSpawnSys::FindSpawnNet(u8 uSpawnNetId)
{
	CSpawnNet* pNet = (CSpawnNet*) flinklist_GetHead(&m_SpawnNetList);
	while (pNet)
	{
		if (pNet->m_uSpawnNetId == uSpawnNetId)
		{
			return pNet;
		}
		pNet =(CSpawnNet*) flinklist_GetNext(&m_SpawnNetList, pNet);
	}
	return NULL;
}


CSpawnNet* CSpawnSys::FindSpawnNet(cchar* pszSpawnNetName)
{
	if (pszSpawnNetName)
	{
		CSpawnNet* pNet = (CSpawnNet*) flinklist_GetHead(&m_SpawnNetList);
		while (pNet)
		{
			if (pNet->m_pszSpawnNetName && !fclib_stricmp(pNet->m_pszSpawnNetName, pszSpawnNetName))
			{
				return pNet;
			}
			pNet =(CSpawnNet*) flinklist_GetNext(&m_SpawnNetList, pNet);
		}
	}
	return NULL;
}


CEntitySpawner* CSpawnSys::FindEntitySpawner(CEntity* pEntity)
{
	CSpawnNet* pNet = (CSpawnNet*) flinklist_GetHead(&m_SpawnNetList);
	while (pNet)
	{
		CEntitySpawner* pEntitySpawner = (CEntitySpawner*) flinklist_GetHead(&(pNet->m_EntitySpawnerList));
		while (pEntitySpawner)
		{
			if (pEntitySpawner->m_pEntity == pEntity)
			{
				return pEntitySpawner;
			}
			pEntitySpawner = (CEntitySpawner*) flinklist_GetNext(&(pNet->m_EntitySpawnerList), pEntitySpawner);
		}
		pNet = (CSpawnNet*) flinklist_GetNext(&m_SpawnNetList, pNet);
	}
	return NULL;
}


CSpawnNetTripwire* CSpawnSys::FindSpawnNetTripwire(CEntity* pEntity)
{
	CSpawnNet* pNet = (CSpawnNet*) flinklist_GetHead(&m_SpawnNetList);
	while (pNet)
	{
		CSpawnNetTripwire* pSpawnNetTripwire = (CSpawnNetTripwire*) flinklist_GetHead(&(pNet->m_TripwireList));
		while (pSpawnNetTripwire)
		{
			if (pSpawnNetTripwire->m_pEntity == pEntity)
			{
				return pSpawnNetTripwire;
			}
			pSpawnNetTripwire = (CSpawnNetTripwire*) flinklist_GetNext(&(pNet->m_TripwireList), pSpawnNetTripwire);
		}
		pNet = (CSpawnNet*) flinklist_GetNext(&m_SpawnNetList, pNet);
	}
	return NULL;
}


CSwarmerBotObstacle* CSpawnSys::FindSwarmerObstacle(CEntity* pEntity)
{
	CSpawnNet* pNet = (CSpawnNet*) flinklist_GetHead(&m_SpawnNetList);
	while (pNet)
	{
		CSwarmerBotObstacle* pObstacle = (CSwarmerBotObstacle*) flinklist_GetHead(&(pNet->m_SwarmerObstacleList));
		while (pObstacle)
		{
			if (pObstacle->m_pEntity == pEntity)
			{
				return pObstacle;
			}
			pObstacle = (CSwarmerBotObstacle*) flinklist_GetNext(&(pNet->m_SwarmerObstacleList), pObstacle);
		}
		pNet = (CSpawnNet*) flinklist_GetNext(&m_SpawnNetList, pNet);
	}
	return NULL;
}


//if pSwarmer is part of a spawn net, this should return which.
CSpawnNet* CSpawnSys::FindSwarmersSpawnNet(CBotSwarmer* pSwarmer)
{
	for (u32 uBotType = 0; uBotType <  SPAWNSYS_NUM_BOTTYPES; uBotType++)
	{
		for (u32 i= 0; i < m_auAutoBotPoolSize[uBotType]; i++)
		{
			if (m_papAutoBotPool[uBotType][i] == pSwarmer)
			{
				return CSpawnSys::FindSpawnNet((u8) m_pauAutoBotNetUseage[uBotType][i]);
			}
		}
	}
	return NULL;
}


void CSpawnSys::AddSpawnNet(CSpawnNet* pSpawnNet)
{
	pSpawnNet->m_uSpawnNetId = m_SpawnNetList.nCount+1;
	flinklist_AddTail(&m_SpawnNetList, pSpawnNet);
}


void CSpawnSys::UpdateAutoBotUseage(void)
{
	for (u32 uBotType = 0; uBotType <  SPAWNSYS_NUM_BOTTYPES; uBotType++)
	{
		for (u32 i= 0; i < m_auAutoBotPoolSize[uBotType]; i++)
		{
			if (!m_papAutoBotPool[uBotType][i]->IsInWorld())
			{
				m_pauAutoBotNetUseage[uBotType][i] = 0;	//up for grabs by any net
			}
			else if (m_papAutoBotPool[uBotType][i]->IsInWorld() &&
					 (m_papAutoBotPool[uBotType][i]->TypeBits() & ENTITY_BIT_BOT && ((CBot*) (m_papAutoBotPool[uBotType][i]))->IsDead()))
			{
				m_papAutoBotPool[uBotType][i]->RemoveFromWorld();
			}
		}
	}
}


CBot* CSpawnSys::RequestAutoBot(u8 uSpawnNetId, u8 uBotType)
{
	CBot* pBot = NULL;
	CBot* pRipeForRecycling = NULL;

	for (u32 i= 0; !pBot && i < m_auAutoBotPoolSize[uBotType]; i++)
	{
		if (!m_papAutoBotPool[uBotType][i]->IsInWorld())
		{
			pBot = m_papAutoBotPool[uBotType][i];	//up for grabs by any net
			m_pauAutoBotNetUseage[uBotType][i] = uSpawnNetId;
		}
		else if (!pRipeForRecycling &&
					m_papAutoBotPool[uBotType][i]->IsInWorld() &&
					(!m_papAutoBotPool[uBotType][i]->IsInVisibleVolumeList()) &&
					!m_papAutoBotPool[uBotType][i]->IsMarkedForWorldRemove())
		{
			pRipeForRecycling = m_papAutoBotPool[uBotType][i];
		}
	}

	if (!pBot && pRipeForRecycling )
	{
		pRipeForRecycling->RemoveFromWorld();	   //can't use bot now, but atleast maybe next frame. it will be ready
	}

	return pBot;
}


u16 CSpawnSys::CountAutoBotsUseByNet(u8 uNetId)
{
	u16 uCount = 0;
	for (u32 uBotType = 0; uBotType < SPAWNSYS_NUM_BOTTYPES; uBotType++)
	{
		for (u32 i= 0; i < m_auAutoBotPoolSize[uBotType]; i++)
		{
			if (m_pauAutoBotNetUseage[uBotType][i] == uNetId)
			{
				uCount++;
			}
		}
	}

	return uCount;
}


//
//
//	 CSpawnNet
//
//

CSpawnNet::CSpawnNet(void)
{
	_ClearData();
}

CSpawnNet::~CSpawnNet(void)
{
	CEntitySpawner *pSpawner =  (CEntitySpawner *) flinklist_RemoveHead(&m_EntitySpawnerList);
	while (pSpawner)
	{
		fdelete(pSpawner);
		pSpawner =  (CEntitySpawner *) flinklist_RemoveHead(&m_EntitySpawnerList);
	}

	CSpawnNetTripwire *pTrip =  (CSpawnNetTripwire *) flinklist_RemoveHead(&m_TripwireList);
	while (pTrip)
	{
		fdelete(pTrip);
		pTrip = (CSpawnNetTripwire *) flinklist_RemoveHead(&m_TripwireList);
	}

	CSwarmerBotObstacle *pObstacle =  (CSwarmerBotObstacle *) flinklist_RemoveHead(&m_SwarmerObstacleList);
	while (pObstacle)
	{
		fdelete(pObstacle);
		pObstacle = (CSwarmerBotObstacle *) flinklist_RemoveHead(&m_SwarmerObstacleList);
	}
}


void CSpawnNet::_ClearData(void)
{
	m_pszSpawnNetName = NULL;
	m_uActiveTimeMin = 0;  
	m_uActiveTimeMax = 0;
	m_uSpawnNetFlags = SPAWNNET_FLAG_NONE;
	m_uBotType = CSpawnSys::SPAWNSYS_BOTTYPE_SWARMER;
	m_uNumToSpawnLifetime = 100;
	m_uSwarmerBotWanderDist = 100;		//How far bots spawned by this net are allowed to wander from their spawn pt.
	m_uSwarmerBotAttackDist = 100;		//How far bots spawned by this net are allowed to attack from their spawn pt.
	m_fSwarmerBotAttackSpeedNorm = 0.;
	m_pHiddenSpawnPts = NULL;
	m_pVisibleSpawnPts = NULL;
	m_pDoorwaySpawnPts = NULL;
	m_pPreplacedSpawnPts = NULL;
	m_pauHiddenSpawnDirs = NULL;
	m_pauVisibleSpawnDirs = NULL;
	m_pauDoorwaySpawnDirs = NULL;
	m_pauPreplacedSpawnDirs = NULL;
	m_papDoors = NULL;
	
	m_aSpawnRateCtrl[0].m_fBurstDelay = 1.0f;
	m_aSpawnRateCtrl[0].m_fFireDelay = 0.24f;
	m_aSpawnRateCtrl[0].m_uNumFiresPerBurst = 4;
	m_aSpawnRateCtrl[0].m_fBurstDelayBonus = 0.0f;
	m_aSpawnRateCtrl[0].m_fFireDelayBonus = 0.0f;
	m_aSpawnRateCtrl[0].m_uNumFiresPerBurstBonus = 0;
	m_anNumLivingLimit[0] = 0;

	m_aSpawnRateCtrl[1].m_fBurstDelay = 1.0f;
	m_aSpawnRateCtrl[1].m_fFireDelay = 0.24f;
	m_aSpawnRateCtrl[1].m_uNumFiresPerBurst = 4;
	m_aSpawnRateCtrl[1].m_fBurstDelayBonus = 0.0f;
	m_aSpawnRateCtrl[1].m_fFireDelayBonus = 0.0f;
	m_aSpawnRateCtrl[1].m_uNumFiresPerBurstBonus = 0;
	m_anNumLivingLimit[1] = 0;

	m_aSpawnRateCtrl[2].m_fBurstDelay = 1.0f;
	m_aSpawnRateCtrl[2].m_fFireDelay = 0.24f;
	m_aSpawnRateCtrl[2].m_uNumFiresPerBurst = 4;
	m_aSpawnRateCtrl[2].m_fBurstDelayBonus = 0.0f;
	m_aSpawnRateCtrl[2].m_fFireDelayBonus = 0.0f;
	m_aSpawnRateCtrl[2].m_uNumFiresPerBurstBonus = 0;
	m_anNumLivingLimit[2] = 5;

	m_aSpawnRateCtrl[3].m_fBurstDelay = 1.0f;
	m_aSpawnRateCtrl[3].m_fFireDelay = 0.24f;
	m_aSpawnRateCtrl[3].m_uNumFiresPerBurst = 4;
	m_aSpawnRateCtrl[3].m_fBurstDelayBonus = 0.0f;
	m_aSpawnRateCtrl[3].m_fFireDelayBonus = 0.0f;
	m_aSpawnRateCtrl[3].m_uNumFiresPerBurstBonus = 0;
	m_anNumLivingLimit[3] = 50;

	//internal data
	m_uSpawnNetId = 0;
	flinklist_InitRoot(&m_EntitySpawnerList, FANG_OFFSETOF(CEntitySpawner, m_NetLink));
	flinklist_InitRoot(&m_TripwireList, FANG_OFFSETOF(CSpawnNetTripwire, m_NetLink));
	flinklist_InitRoot(&m_SwarmerObstacleList, FANG_OFFSETOF(CSwarmerBotObstacle, m_NetLink));

	m_SpawnNetLink.pPrevLink = NULL;
	m_SpawnNetLink.pNextLink = NULL;
	m_pauVolume = NULL;
	m_uNumVolumes = 0;

	CheckPointRestore();
}


void CSpawnNet::CheckPointRestore(void)
{
//internal data to be cleared by checkpoint restore
	m_pIntruder = NULL;
	m_uIntruderGUID = 0;
	m_fTurnOnTime = 0.0f;
	m_fActiveTime = 0.0f;
	m_uNumSpawnedSoFarLifetime = 0;
	m_uNetState = SPAWNNET_STATE_OFF;
	m_uDefConLevel = SPAWNNET_DEFCON_0_INACTIVE;
	m_fDefConTimeOut = 0.0f;
	m_uSavedBots = 0;
}


void CSpawnNet::SetDefconLevel(u8 uDefConLevel)
{
	m_uDefConLevel = uDefConLevel;
	m_fDefConTimeOut = aiutils_GetCurTimeSecs() + 10.0f + 3.0f*fmath_RandomFloat();
}


void CSpawnNet::AddEntitySpawner(CEntitySpawner* pSpawner)
{
	flinklist_AddTail(&m_EntitySpawnerList, pSpawner);
}


void CSpawnNet::AddTripwire(CSpawnNetTripwire* pTripwire)
{
	flinklist_AddTail(&m_TripwireList, pTripwire);
}


void CSpawnNet::AddSwarmerObstacle(CSwarmerBotObstacle* pObstacle)
{
	flinklist_AddTail(&m_SwarmerObstacleList, pObstacle);
}

#define MAX_EVALS 5
void CSpawnNet::DeployAutoBot(CBot* pBot)
{
	//where to put it
	u8 uSpawnDirHint = 0;
	if (m_pVisibleSpawnPts && m_pVisibleSpawnPts->PointCount())
	{
		u32 uSpawnEval;

		CFVec3A aSpawnPtEvals[MAX_EVALS];
		u8 aSpawnPtDirEvals[MAX_EVALS];
		u32 uNumEvals = 0;


		if (this->m_uDefConLevel == 0 && m_pPreplacedSpawnPts)
		{
			//
			//	Go through preplaced spawn pts 
			//      and use one eval slot for each
			if (uNumEvals < MAX_EVALS && m_pPreplacedSpawnPts)
			{
				uSpawnEval = fmath_RandomChoice(m_pPreplacedSpawnPts->PointCount());
				f32 fMinDist = 10.0f;
				for (u32 j= 0; (uNumEvals < MAX_EVALS ) && j < m_pPreplacedSpawnPts->PointCount(); j++)
				{
					aSpawnPtEvals[uNumEvals] = m_pPreplacedSpawnPts->PointArray()[uSpawnEval];
					if (m_pauPreplacedSpawnDirs)
					{
						aSpawnPtDirEvals[uNumEvals] = m_pauPreplacedSpawnDirs[uSpawnEval];
					}
					uNumEvals++;
					uSpawnEval++;
					if (uSpawnEval >=m_pPreplacedSpawnPts->PointCount())
					{
						uSpawnEval = 0;
					}
				}
			}
		}

		//
		//	Go through entity spawners
		//
		if (m_EntitySpawnerList.nCount > 0)
		{
			CEntitySpawner* pEntitySpawner = (CEntitySpawner*) flinklist_GetHead(&(m_EntitySpawnerList));
			while (uNumEvals < MAX_EVALS &&
					pEntitySpawner &&
					pEntitySpawner->m_pEntity &&
					(pEntitySpawner->m_uEntitySpawnerFlags & CEntitySpawner::ENTITYSPAWNER_FLAG_SPAWNWITHNET) &&
					pEntitySpawner->m_pEntity->IsInWorld())
			{
				pEntitySpawner->m_pEntity->GetGoodieDistributionOrigin(&(aSpawnPtEvals[uNumEvals]));
				aSpawnPtDirEvals[uNumEvals] = 0;   //zero direction
				uNumEvals++;
				pEntitySpawner = (CEntitySpawner*) flinklist_GetNext(&(m_EntitySpawnerList), pEntitySpawner);
			}
		}

		//
		//	Go through visible spawn pts 
		//      and find all that are at least 10 feet away.
		if (uNumEvals < MAX_EVALS && m_pVisibleSpawnPts)
		{
			uSpawnEval = fmath_RandomChoice(m_pVisibleSpawnPts->PointCount());
			f32 fMinDist = 10.0f;
			for (u32 j= 0; (uNumEvals < MAX_EVALS ) && j < m_pVisibleSpawnPts->PointCount(); j++)
			{
				if (!m_pIntruder || m_pVisibleSpawnPts->PointArray()[uSpawnEval].DistSq(m_pIntruder->MtxToWorld()->m_vPos) > fMinDist*fMinDist)   //minimum feet to player to spawn
				{
					aSpawnPtEvals[uNumEvals] = m_pVisibleSpawnPts->PointArray()[uSpawnEval];
					if (m_pauVisibleSpawnDirs)
					{
						aSpawnPtDirEvals[uNumEvals] = m_pauVisibleSpawnDirs[uSpawnEval];
					}
					uNumEvals++;
				}
				uSpawnEval++;
				if (uSpawnEval >=m_pVisibleSpawnPts->PointCount())
				{
					uSpawnEval = 0;
				}
			}
		}

		//
		//	Go through Hidden spawn pts 
		//		and find any that are not currently visible
		if (uNumEvals < MAX_EVALS  && m_pHiddenSpawnPts && m_pauHiddenSpawnPtVol)
		{
			uSpawnEval = fmath_RandomChoice(m_pHiddenSpawnPts->PointCount());
			for (u32 j= 0; (uNumEvals < MAX_EVALS) && j < m_pHiddenSpawnPts->PointCount(); j++)
			{
				if (!(FVis_anVolumeFlags[m_pauHiddenSpawnPtVol[uSpawnEval]] & FVIS_VOLUME_IN_VISIBLE_LIST)) 
				{
					aSpawnPtEvals[uNumEvals] = m_pHiddenSpawnPts->PointArray()[uSpawnEval];
					if (m_pauHiddenSpawnDirs)
					{
						aSpawnPtDirEvals[uNumEvals] = m_pauHiddenSpawnDirs[uSpawnEval];
					}
					uNumEvals++;
				}
				uSpawnEval++;
				if (uSpawnEval >= m_pHiddenSpawnPts->PointCount())
				{
					uSpawnEval = 0;
				}
			}
		}

		//
		//	Go through Doorway spawn pts 
		//		and find any that are behind closed doors
		//
		if (uNumEvals < MAX_EVALS  && m_pDoorwaySpawnPts && m_papDoors)
		{
			uSpawnEval = fmath_RandomChoice(m_pDoorwaySpawnPts->PointCount());
			for (u32 j= 0; (uNumEvals < MAX_EVALS) && j < m_pDoorwaySpawnPts->PointCount(); j++)
			{
				if (m_papDoors[uSpawnEval] &&
					!m_papDoors[uSpawnEval]->AI_IsOpen(m_pDoorwaySpawnPts->PointArray()[uSpawnEval]) &&
					!m_papDoors[uSpawnEval]->AI_IsOpening(m_pDoorwaySpawnPts->PointArray()[uSpawnEval]))
				{
					aSpawnPtEvals[uNumEvals] = m_pDoorwaySpawnPts->PointArray()[uSpawnEval];
					if (m_pauDoorwaySpawnDirs)
					{
						aSpawnPtDirEvals[uNumEvals] = m_pauDoorwaySpawnDirs[uSpawnEval];
					}
					uNumEvals++;
				}
				uSpawnEval++;
				if (uSpawnEval >= m_pDoorwaySpawnPts->PointCount())
				{
					uSpawnEval = 0;
				}
			}
		}

		if (uNumEvals > 0)
		{
			s32 nWinner = 0;
		
			if (m_pIntruder)
			{
				nWinner = aiutils_FindClosestTo(m_pIntruder->MtxToWorld()->m_vPos, aSpawnPtEvals, uNumEvals);
			}

			if (pBot->TypeBits() & ENTITY_BIT_BOTSWARMER)
			{
				((CBotSwarmer*) pBot)->PositionForSpawn(aSpawnPtEvals[nWinner], aiutils_FindRandomBitIndex(aSpawnPtDirEvals[nWinner]));
			}
			else
			{
				pBot->Relocate_Xlat_WS(aSpawnPtEvals+nWinner);
			}

     		pBot->AddToWorld();
			pBot->DrawEnable(TRUE,TRUE);
			m_uNumSpawnedSoFarLifetime++;
		}
	}
}


void CSpawnNet::InitVolumeLookup(void)
{
	u32 i;
	if (m_pHiddenSpawnPts || m_pVisibleSpawnPts)
	{
		//count up the unique ones so that we can get a minimal set that encompases the entire spawnnet
		u32 uUniqueCount= 0;
		FMemFrame_t tmp = fmem_GetFrame( );
		u16 * pauTmp = (u16 *) fmem_Alloc(1024, 16);

		if (m_pVisibleSpawnPts)
		{
			for (i = 0; i < m_pVisibleSpawnPts->PointCount(); i++)
			{
				FVisVolume_t* pVol = NULL;
				if (fworld_GetVolumeContainingPoint(&(m_pVisibleSpawnPts->PointArray()[i]), &pVol) > 0.0f && pVol)
				{
					BOOL bFound = FALSE;
					for (u32 j = 0; j < uUniqueCount; j++)
					{
						if (pauTmp[j] == pVol->nVolumeID)
						{
							bFound = TRUE;
							break;
						}
					}

					if (!bFound)
					{
						pauTmp[uUniqueCount++] = pVol->nVolumeID;
					}
				}
			}
		}

		if (m_pHiddenSpawnPts && m_pauHiddenSpawnPtVol)
		{
			for (i = 0; i < m_pHiddenSpawnPts->PointCount(); i++)
			{
				BOOL bFound = FALSE;
				for (u32 j = 0; j < uUniqueCount; j++)
				{
					if (pauTmp[j] == m_pauHiddenSpawnPtVol[i])
					{
						BOOL bFound = TRUE;
						break;
					}
				}

				if (!bFound)
				{
				  pauTmp[uUniqueCount++] = m_pauHiddenSpawnPtVol[i];
				}

			}
		}

		m_pauVolume = (u16 *) fres_AlignedAllocAndZero(uUniqueCount*sizeof(u16), 16);
		if (m_pauVolume)
		{
			//if this doesn't allocate, it aint gonna crash or nothing.  Just probably wont get that many swarmers. :)
			// no need to not finishe loading the level
			for (u32 i = 0; i < uUniqueCount; i++)
			{
				m_pauVolume[i] = pauTmp[i];
			}
			m_uNumVolumes = uUniqueCount;
		}
		else
		{
			DEVPRINTF("SPAWNSYS: No Memory to allocate the tiniest of arrays.");
		}

		fmem_ReleaseFrame(tmp);
	}
}


void CSpawnNet::InitSpawnPts(void)
{
	//
	// create lookup table from spawn pts to their volumes.
	//
	InitVolumeLookup();

	//
	// Pre-calculate and store the surfaces that are nearby the spawn pts.
	//
	CESpline* apSplines[4] = {	m_pHiddenSpawnPts,
								m_pVisibleSpawnPts,
								m_pDoorwaySpawnPts,
								m_pPreplacedSpawnPts};


	u8** appauSpawnDirs[4] = {	&m_pauHiddenSpawnDirs,
								&m_pauVisibleSpawnDirs,
								&m_pauDoorwaySpawnDirs,
								&m_pauPreplacedSpawnDirs};

	for (u32 i = 0; i < 4; i++)
	{
		if (apSplines[i] && apSplines[i]->PointCount())
		{
			*(appauSpawnDirs[i]) = (u8*)fres_AllocAndZero(sizeof(u8)*apSplines[i]->PointCount());

			if (*(appauSpawnDirs[i]))
			{
				for (u32 j = 0; j < apSplines[i]->PointCount(); j++)
				{
					for (u32 k = 0; k < 6; k++)
					{
						CFVec3A End;
						CBotSwarmer::TestDirToRay(k, &End);
						End.Mul(10.0f);
						End.Add(apSplines[i]->PointArray()[j]);
						if (fworld_IsLineOfSightObstructed(apSplines[i]->PointArray()+j, &End))
						{
							(*(appauSpawnDirs[i]))[j] |= 1<<k;
						}
					}
				}
			}
		}
	}


	//
	// Doorway spawn pts can each have a ptr to a door.
	//
	if (m_pDoorwaySpawnPts && m_pDoorwaySpawnPts->PointCount())
	{
		m_papDoors = (CDoorEntity**) fres_AllocAndZero(sizeof(CDoorEntity*)*m_pDoorwaySpawnPts->PointCount());
		if (m_papDoors)
		{
			for (u32 i= 0; i < m_pDoorwaySpawnPts->PointCount(); i++)
			{
				m_papDoors[i] = CDoorEntity::AI_FindWithNearestPad(m_pDoorwaySpawnPts->PointArray()[i], CDoorEntity::USERTYPE_DOOR);
				if (!m_papDoors[i])
				{
					DEVPRINTF("Can't find door anywhere near door spawn pt at (%d, %d, %d)\n", (s32)m_pDoorwaySpawnPts->PointArray()[i].x, (s32)m_pDoorwaySpawnPts->PointArray()[i].y, (s32)m_pDoorwaySpawnPts->PointArray()[i].z);
				}
			}
			
		}
	}


}

void CSpawnNet::Work(void)
{
	u8 uDefConLevel = SPAWNNET_DEFCON_0_INACTIVE;
	CBot* pIntruder = NULL;

	if (!(m_uSpawnNetFlags & SPAWNNET_NO_AUTO_DEFCON))
	{
		//
		//automatically change defcon level based on the volume flags in my pt arrays
		//
		for (u32 i =0; uDefConLevel != SPAWNNET_DEFCON_3_INTRUDER && i < m_uNumVolumes; i++)
		{
			if (uDefConLevel < SPAWNNET_DEFCON_1_ACTIVE && FVis_anVolumeFlags[m_pauVolume[i]] & FVIS_VOLUME_IN_ACTIVE_LIST)
			{
				uDefConLevel = SPAWNNET_DEFCON_1_ACTIVE;
			}
			if (uDefConLevel < SPAWNNET_DEFCON_2_VISIBLE && FVis_anVolumeFlags[m_pauVolume[i]] & FVIS_VOLUME_IN_VISIBLE_LIST)
			{
				uDefConLevel = SPAWNNET_DEFCON_2_VISIBLE;
			}
			if (uDefConLevel < SPAWNNET_DEFCON_3_INTRUDER)
			{
				for (s32 uPlayer = 0; uPlayer < CPlayer::m_nPlayerCount; uPlayer++)
				{
					if (Player_aPlayer[uPlayer].m_pEntityCurrent &&
						Player_aPlayer[uPlayer].m_pEntityCurrent->TypeBits() & ENTITY_BIT_BOT)
					{
						CFWorldIntersect *pSrcIntersect = NULL;
						while ( uDefConLevel != SPAWNNET_DEFCON_3_INTRUDER &&
								(pSrcIntersect = ((CBot*) Player_aPlayer[uPlayer].m_pEntityCurrent)->m_pWorldMesh->GetNextIntersect( pSrcIntersect )) )
						{
							if (m_pauVolume[i] == pSrcIntersect->GetVolume()->nVolumeID)
							{
								uDefConLevel = SPAWNNET_DEFCON_3_INTRUDER;
								pIntruder = (CBot*) Player_aPlayer[uPlayer].m_pEntityCurrent;
							}
						}
					}
				}
			}
		}
	}

	if (uDefConLevel > m_uDefConLevel)
	{
		m_fDefConTimeOut = aiutils_GetCurTimeSecs() + 10.0f + 3.0f*fmath_RandomFloat();
		m_uDefConLevel = uDefConLevel;
	}
	else if (uDefConLevel < m_uDefConLevel &&
		m_fDefConTimeOut < aiutils_GetCurTimeSecs())
	{
		m_uDefConLevel = uDefConLevel;
	}

	//
	// spawners work
	//
	CSwarmerBotObstacle* pObstacle = (CSwarmerBotObstacle*) flinklist_GetHead(&(m_SwarmerObstacleList));
	while (pObstacle)
	{
		pObstacle->Work(this);
		pObstacle = (CSwarmerBotObstacle*) flinklist_GetNext(&(m_SwarmerObstacleList), pObstacle);
	}


	//
	// Based on DefConLevel
	//	spawn 
	//

	u32 uMyLivingBots = CSpawnSys::CountAutoBotsUseByNet(m_uSpawnNetId);

	if (m_uSavedBots > 0 && uMyLivingBots < m_anNumLivingLimit[m_uDefConLevel])
	{
		u32 uNumNeeded = m_anNumLivingLimit[m_uDefConLevel]-uMyLivingBots;
		BOOL bNoneAvailable = FALSE;
		for (u32 i = 0; i < uNumNeeded && !bNoneAvailable; i++)
		{
			CBot* pBot = CSpawnSys::RequestAutoBot(m_uSpawnNetId, m_uBotType);
			if (pBot)
			{
				//DeployAutoBot(pBot);		unsave a bot
			}
			else
			{
				bNoneAvailable = TRUE;
			}
		}
	}

	//try to get as many as I need above and beyond those that are saved
	if (uMyLivingBots+m_uSavedBots < m_anNumLivingLimit[m_uDefConLevel])
	{
		u32 uNumNeeded = m_anNumLivingLimit[m_uDefConLevel]-(uMyLivingBots+m_uSavedBots);
		BOOL bNoneAvailable = FALSE;
		for (u32 i = 0; i < uNumNeeded && !bNoneAvailable; i++)
		{
			CBot* pBot = CSpawnSys::RequestAutoBot(m_uSpawnNetId, m_uBotType);
			if (pBot)
			{
				DeployAutoBot(pBot);
			}
			else
			{
				bNoneAvailable = TRUE;
			}
		}
	}

}


//
//	CEntitySpawner
//
CEntitySpawner::CEntitySpawner(void)
{
	_ClearData();
}


CEntitySpawner::~CEntitySpawner(void)
{
}

void CEntitySpawner::Work(CSpawnNet* pNet)
{
}


BOOL CEntitySpawner::IsActive(void)
{
	return m_pEntity && m_pEntity->IsInWorld() && (!(m_pEntity->TypeBits() & ENTITY_BIT_BOT) || !(((CBot*)m_pEntity)->IsDeadOrDying()));
}


void CEntitySpawner::_ClearData(void)
{
	m_uEntitySpawnerFlags = ENTITYSPAWNER_FLAG_NONE;
	m_NetLink.pNextLink = NULL;
	m_NetLink.pPrevLink = NULL;
	m_pEntity = NULL;
	m_uSpawnNetId = 0;
	m_uNumToSpawnOnDeath = 0;
}


//
//	CSpawnNetTripwire
//
CSpawnNetTripwire::CSpawnNetTripwire(void)
{
	_ClearData();
}


CSpawnNetTripwire::~CSpawnNetTripwire(void)
{
}


void CSpawnNetTripwire::_ClearData(void)
{
	m_NetLink.pPrevLink = NULL;
	m_NetLink.pNextLink = NULL;
	m_pEntity = NULL;
	m_pNet = NULL;
	m_uTripwireFlags = SPAWNNETTRIPWIRE_FLAG_NONE;
	m_uForceDefconLevel = CSpawnNet::SPAWNNET_DEFCON_0_INACTIVE;
}


BOOL CSpawnNetTripwire::IsActive(void)
{
	return !!(m_uTripwireFlags & SPAWNNETTRIPWIRE_FLAG_DISABLE);
}


//
//	CSwarmerBotObstacle
//
CSwarmerBotObstacle::CSwarmerBotObstacle(void)
{
	_ClearData();
}


CSwarmerBotObstacle::~CSwarmerBotObstacle(void)
{
}


void CSwarmerBotObstacle::_ClearData(void)
{
	m_NetLink.pNextLink = NULL;
	m_NetLink.pPrevLink = NULL;
	m_pEntity = NULL; 
	m_uSpawnNetId = 0;
	m_uSwarmerBotObstacleFlags = SWARMERBOTOBSTACLE_FLAG_NONE;
	m_uAISoundHandle = 0;
	m_fRadius = 10.0f;
}

void CSwarmerBotObstacle::Work(CSpawnNet* pNet)
{
	if (this->IsActive())
	{
		f32 fDuration = 1.0f; 

		if (!m_uAISoundHandle || !AIEnviro_ModifySound(m_uAISoundHandle, m_pEntity->MtxToWorld()->m_vPos, m_fRadius,fDuration, AISOUNDTYPE_SWARMERBARRIER, NULL))
		{
			m_uAISoundHandle = AIEnviro_AddSound(m_pEntity->MtxToWorld()->m_vPos, m_fRadius, fDuration, AISOUNDTYPE_SWARMERBARRIER, AISOUNDCTRL_NONE, NULL);
		}
	}
}


BOOL CSwarmerBotObstacle::IsActive(void)
{
	return  m_pEntity &&
			m_pEntity->IsInWorld() &&
			(!(m_pEntity->TypeBits() & ENTITY_BIT_BOT) || !((CBot*) m_pEntity)->IsDeadOrDying()) && //Either it isn't a bot, or it is a living bot
			!(m_uSwarmerBotObstacleFlags & SWARMERBOTOBSTACLE_FLAG_DISABLE);
}


//
//
//	CSpawnSysBuilder
//
//
//
//
//	CSpawnSysBuilder
//
//
BuilderHelp_EnumTagSpec _aBotType_Use[] = 
{
	"SWARMER", CSpawnSys::SPAWNSYS_BOTTYPE_SWARMER,
	"VERMIN", CSpawnSys::SPAWNSYS_BOTTYPE_VERMIN,
	BUILDER_ERROR_STRING("CSpawnSysBuilder::InterpretTable() : Invalid value '%s' for 'SS_NET_BOTTYPE' table. Use (SWARMER or VERMIN).\n"), 0,
};

BuilderHelp_EnumTagSpec _aNetCtrl_Use[] = 
{
	"NO_AUTO_DEFCON",		CSpawnNet::SPAWNNET_NO_AUTO_DEFCON,
	BUILDER_ERROR_STRING("CSpawnSysBuilder::InterpretTable() : Invalid value '%s' for 'SS_NET_CTRL' table. Use atleast on of (NO_AUTO_DEFCON).\n"), 0,
};


BuilderHelp_EnumTagSpec _aEntitySpawnnerCtrl_Use[] = 
{
	"SPAWNONDEATH",			CEntitySpawner::ENTITYSPAWNER_FLAG_SPAWNONDEATH,
	"SPAWNWITHNET",			CEntitySpawner::ENTITYSPAWNER_FLAG_SPAWNWITHNET,
	"DISABLE",				CEntitySpawner::ENTITYSPAWNER_FLAG_DISABLE,
	BUILDER_ERROR_STRING("CSpawnSysBuilder::InterpretTable() : Invalid value '%s' for 'SS_ENTIYSPAWNER_CTRL' table. Use atleast on of (SPAWNONDEATH or SPAWNWITHNET or DISABLE).\n"), 0,
};

BuilderHelp_EnumTagSpec _aObstacleCtrl_Use[] = 
{
	"DISABLE",				CSwarmerBotObstacle::SWARMERBOTOBSTACLE_FLAG_DISABLE,
	BUILDER_ERROR_STRING("CSpawnSysBuilder::InterpretTable() : Invalid value '%s' for 'SS_NET_SWARMEROBSTACLE_CTRL' table. Use atleast on of (DISABLE).\n"), 0,
};

BuilderHelp_EnumTagSpec _aEntityTripCtrl_Use[] = 
{
	BUILDER_ERROR_STRING("CSpawnSysBuilder::InterpretTable() : Invalid value '%s' for 'SS_ENTIYTRIPWIRE_CTRL' table. Use atleast on of (asdfasdfasdf).\n"), 0,
};

static InterpretTableDataSpec _aItableDataSpec[] = 
{
//	     Tag								Address of Vbl To Be set												DataType			uNumEnumTags in Spec												pMinMaxSpec								BuilderHelp_EnumTagSpec					Group Bool								internal use	internal use
	"SS_NET_NAME",							FANG_OFFSETOF(CSpawnSysBuilder, m_pszSpawnNetName),						BDT_STRING,			0,																	NULL,									NULL,									NO_TRACKGROUPEDBOOLOFFSET,				NULL,			NULL,			
	"SS_NET_ONTIME_MIN",					FANG_OFFSETOF(CSpawnSysBuilder, m_uActiveTimeMin),						BDT_U32,			0,																	&BuilderHelp_U8,						NULL,									NO_TRACKGROUPEDBOOLOFFSET,				NULL,			NULL,			
	"SS_NET_ONTIME_MAX",					FANG_OFFSETOF(CSpawnSysBuilder, m_uActiveTimeMax),						BDT_U32,			0,																	&BuilderHelp_U8,						NULL,									NO_TRACKGROUPEDBOOLOFFSET,				NULL,			NULL,			
	"SS_NET_BOTTYPE",						FANG_OFFSETOF(CSpawnSysBuilder, m_uBotType),							BDT_ENUM,			sizeof(_aBotType_Use)/sizeof(BuilderHelp_EnumTagSpec),				NULL,									_aBotType_Use,							NO_TRACKGROUPEDBOOLOFFSET,				NULL,			NULL,		    
	"SS_NET_NUMTOSPAWN_LIFE",				FANG_OFFSETOF(CSpawnSysBuilder, m_uNumToSpawnLifetime),					BDT_U32,			0,																	&BuilderHelp_U16,						NULL,									NO_TRACKGROUPEDBOOLOFFSET,				NULL,			NULL,			
	"SS_NET_CTRL",							FANG_OFFSETOF(CSpawnSysBuilder, m_uSpawnNetFlags),						BDT_ENUMS2BITS,		sizeof(_aNetCtrl_Use)/sizeof(BuilderHelp_EnumTagSpec),				NULL,									_aNetCtrl_Use,							NO_TRACKGROUPEDBOOLOFFSET,				NULL,			NULL,			
																																																																																					
	"SS_NET_NUMLIVING_LIMIT_0",				FANG_OFFSETOF(CSpawnSysBuilder, m_uNumLivingLimit_0),					BDT_U32,			0,																	&BuilderHelp_U16,						NULL,									NO_TRACKGROUPEDBOOLOFFSET,				NULL,			NULL,			
	"SS_NET_BURSTDELAY_0",					FANG_OFFSETOF(CSpawnSysBuilder, m_fBurstDelay_0),						BDT_F32,			0,																	&BuilderHelp_PosF32MMS,					NULL,									NO_TRACKGROUPEDBOOLOFFSET,				NULL,			NULL,			
	"SS_NET_SPAWNDELAY_0",					FANG_OFFSETOF(CSpawnSysBuilder, m_fFireDelay_0),						BDT_F32,			0,																	&BuilderHelp_PosF32MMS,					NULL,									NO_TRACKGROUPEDBOOLOFFSET,				NULL,			NULL,			
	"SS_NET_SPAWNSPERBURST_0",				FANG_OFFSETOF(CSpawnSysBuilder, m_uNumFiresPerBurst_0),					BDT_U32,			0,																	&BuilderHelp_U16,						NULL,									NO_TRACKGROUPEDBOOLOFFSET,				NULL,			NULL,			
	"SS_NET_NUMLIVING_LIMIT_BONUS_0",		FANG_OFFSETOF(CSpawnSysBuilder, m_uNumLivingLimitBonus_0),				BDT_U32,			0,																	&BuilderHelp_U16,						NULL,									NO_TRACKGROUPEDBOOLOFFSET,				NULL,			NULL,			
	"SS_NET_BURSTDELAY_BONUS_0",			FANG_OFFSETOF(CSpawnSysBuilder, m_fBurstDelayBonus_0),					BDT_F32,			0,																	&BuilderHelp_PosF32MMS,					NULL,									NO_TRACKGROUPEDBOOLOFFSET,				NULL,			NULL,			
	"SS_NET_SPAWNDELAY_BONUS_0",			FANG_OFFSETOF(CSpawnSysBuilder, m_fFireDelayBonus_0),					BDT_F32,			0,																	&BuilderHelp_PosF32MMS,					NULL,									NO_TRACKGROUPEDBOOLOFFSET,				NULL,			NULL,			
	"SS_NET_SPAWNSPERBURST_BONUS_0",		FANG_OFFSETOF(CSpawnSysBuilder, m_uNumFiresPerBurstBonus_0),			BDT_U32,			0,																	&BuilderHelp_U16,						NULL,									NO_TRACKGROUPEDBOOLOFFSET,				NULL,			NULL,			
																																																																																					
	"SS_NET_NUMLIVING_LIMIT_1",				FANG_OFFSETOF(CSpawnSysBuilder, m_uNumLivingLimit_1),					BDT_U32,			0,																	&BuilderHelp_U16,						NULL,									NO_TRACKGROUPEDBOOLOFFSET,				NULL,			NULL,			
	"SS_NET_BURSTDELAY_1",					FANG_OFFSETOF(CSpawnSysBuilder, m_fBurstDelay_1),						BDT_F32,			0,																	&BuilderHelp_PosF32MMS,					NULL,									NO_TRACKGROUPEDBOOLOFFSET,				NULL,			NULL,			
	"SS_NET_SPAWNDELAY_1",					FANG_OFFSETOF(CSpawnSysBuilder, m_fFireDelay_1),						BDT_F32,			0,																	&BuilderHelp_PosF32MMS,					NULL,									NO_TRACKGROUPEDBOOLOFFSET,				NULL,			NULL,			
	"SS_NET_SPAWNSPERBURST_1",				FANG_OFFSETOF(CSpawnSysBuilder, m_uNumFiresPerBurst_1),					BDT_U32,			0,																	&BuilderHelp_U16,						NULL,									NO_TRACKGROUPEDBOOLOFFSET,				NULL,			NULL,			
	"SS_NET_NUMLIVING_LIMIT_BONUS_1",		FANG_OFFSETOF(CSpawnSysBuilder, m_uNumLivingLimitBonus_1),				BDT_U32,			0,																	&BuilderHelp_U16,						NULL,									NO_TRACKGROUPEDBOOLOFFSET,				NULL,			NULL,			
	"SS_NET_BURSTDELAY_BONUS_1",			FANG_OFFSETOF(CSpawnSysBuilder, m_fBurstDelayBonus_1),					BDT_F32,			0,																	&BuilderHelp_PosF32MMS,					NULL,									NO_TRACKGROUPEDBOOLOFFSET,				NULL,			NULL,			
	"SS_NET_SPAWNDELAY_BONUS_1",			FANG_OFFSETOF(CSpawnSysBuilder, m_fFireDelayBonus_1),					BDT_F32,			0,																	&BuilderHelp_PosF32MMS,					NULL,									NO_TRACKGROUPEDBOOLOFFSET,				NULL,			NULL,			
	"SS_NET_SPAWNSPERBURST_BONUS_1",		FANG_OFFSETOF(CSpawnSysBuilder, m_uNumFiresPerBurstBonus_1),			BDT_U32,			0,																	&BuilderHelp_U16,						NULL,									NO_TRACKGROUPEDBOOLOFFSET,				NULL,			NULL,			
																																																																																					
	"SS_NET_NUMLIVING_LIMIT_2",				FANG_OFFSETOF(CSpawnSysBuilder, m_uNumLivingLimit_2),					BDT_U32,			0,																	&BuilderHelp_U16,						NULL,									NO_TRACKGROUPEDBOOLOFFSET,				NULL,			NULL,			
	"SS_NET_BURSTDELAY_2",					FANG_OFFSETOF(CSpawnSysBuilder, m_fBurstDelay_2),						BDT_F32,			0,																	&BuilderHelp_PosF32MMS,					NULL,									NO_TRACKGROUPEDBOOLOFFSET,				NULL,			NULL,			
	"SS_NET_SPAWNDELAY_2",					FANG_OFFSETOF(CSpawnSysBuilder, m_fFireDelay_2),						BDT_F32,			0,																	&BuilderHelp_PosF32MMS,					NULL,									NO_TRACKGROUPEDBOOLOFFSET,				NULL,			NULL,			
	"SS_NET_SPAWNSPERBURST_2",				FANG_OFFSETOF(CSpawnSysBuilder, m_uNumFiresPerBurst_2),					BDT_U32,			0,																	&BuilderHelp_U16,						NULL,									NO_TRACKGROUPEDBOOLOFFSET,				NULL,			NULL,			
	"SS_NET_NUMLIVING_LIMIT_BONUS_2",		FANG_OFFSETOF(CSpawnSysBuilder, m_uNumLivingLimitBonus_2),				BDT_U32,			0,																	&BuilderHelp_U16,						NULL,									NO_TRACKGROUPEDBOOLOFFSET,				NULL,			NULL,			
	"SS_NET_BURSTDELAY_BONUS_2",			FANG_OFFSETOF(CSpawnSysBuilder, m_fBurstDelayBonus_2),					BDT_F32,			0,																	&BuilderHelp_PosF32MMS,					NULL,									NO_TRACKGROUPEDBOOLOFFSET,				NULL,			NULL,			
	"SS_NET_SPAWNDELAY_BONUS_2",			FANG_OFFSETOF(CSpawnSysBuilder, m_fFireDelayBonus_2),					BDT_F32,			0,																	&BuilderHelp_PosF32MMS,					NULL,									NO_TRACKGROUPEDBOOLOFFSET,				NULL,			NULL,			
	"SS_NET_SPAWNSPERBURST_BONUS_2",		FANG_OFFSETOF(CSpawnSysBuilder, m_uNumFiresPerBurstBonus_2),			BDT_U32,			0,																	&BuilderHelp_U16,						NULL,									NO_TRACKGROUPEDBOOLOFFSET,				NULL,			NULL,			
																																																																																					
	"SS_NET_NUMLIVING_LIMIT_3",				FANG_OFFSETOF(CSpawnSysBuilder, m_uNumLivingLimit_3),					BDT_U32,			0,																	&BuilderHelp_U16,						NULL,									NO_TRACKGROUPEDBOOLOFFSET,				NULL,			NULL,			
	"SS_NET_BURSTDELAY_3",					FANG_OFFSETOF(CSpawnSysBuilder, m_fBurstDelay_3),						BDT_F32,			0,																	&BuilderHelp_PosF32MMS,					NULL,									NO_TRACKGROUPEDBOOLOFFSET,				NULL,			NULL,			
	"SS_NET_SPAWNDELAY_3",					FANG_OFFSETOF(CSpawnSysBuilder, m_fFireDelay_3),						BDT_F32,			0,																	&BuilderHelp_PosF32MMS,					NULL,									NO_TRACKGROUPEDBOOLOFFSET,				NULL,			NULL,			
	"SS_NET_SPAWNSPERBURST_3",				FANG_OFFSETOF(CSpawnSysBuilder, m_uNumFiresPerBurst_3),					BDT_U32,			0,																	&BuilderHelp_U16,						NULL,									NO_TRACKGROUPEDBOOLOFFSET,				NULL,			NULL,			
	"SS_NET_NUMLIVING_LIMIT_BONUS_3",		FANG_OFFSETOF(CSpawnSysBuilder, m_uNumLivingLimitBonus_3),				BDT_U32,			0,																	&BuilderHelp_U16,						NULL,									NO_TRACKGROUPEDBOOLOFFSET,				NULL,			NULL,			
	"SS_NET_BURSTDELAY_BONUS_3",			FANG_OFFSETOF(CSpawnSysBuilder, m_fBurstDelayBonus_3),					BDT_F32,			0,																	&BuilderHelp_PosF32MMS,					NULL,									NO_TRACKGROUPEDBOOLOFFSET,				NULL,			NULL,			
	"SS_NET_SPAWNDELAY_BONUS_3",			FANG_OFFSETOF(CSpawnSysBuilder, m_fFireDelayBonus_3),					BDT_F32,			0,																	&BuilderHelp_PosF32MMS,					NULL,									NO_TRACKGROUPEDBOOLOFFSET,				NULL,			NULL,			
	"SS_NET_SPAWNSPERBURST_BONUS_3",		FANG_OFFSETOF(CSpawnSysBuilder, m_uNumFiresPerBurstBonus_3),			BDT_U32,			0,																	&BuilderHelp_U16,						NULL,									NO_TRACKGROUPEDBOOLOFFSET,				NULL,			NULL,			
																																																																																					
	"SS_NET_BOT_WANDER_DIST",				FANG_OFFSETOF(CSpawnSysBuilder, m_uSwarmerBotWanderDist),				BDT_U32,			0,																	&BuilderHelp_U16,						NULL,									NO_TRACKGROUPEDBOOLOFFSET,				NULL,			NULL,			
	"SS_NET_BOT_ATTACK_DIST",				FANG_OFFSETOF(CSpawnSysBuilder, m_uSwarmerBotAttackDist),				BDT_U32,			0,																	&BuilderHelp_U16,						NULL,									NO_TRACKGROUPEDBOOLOFFSET,				NULL,			NULL,			
	"SS_NET_BOT_SPEED",						FANG_OFFSETOF(CSpawnSysBuilder, m_fSwarmerBotAttackSpeedNorm),			BDT_F32,			0,																	&BuilderHelp_PosUnitF32MMS,				NULL,									NO_TRACKGROUPEDBOOLOFFSET,				NULL,			NULL,			
	"SS_NET_HIDDEN_PT_SPLINE",				FANG_OFFSETOF(CSpawnSysBuilder, m_bIsSpawnSpline_Hidden),				BDT_BOOL,			0,																	NULL,									NULL,									NO_TRACKGROUPEDBOOLOFFSET,				NULL,			NULL,			
	"SS_NET_VISIBLE_PT_SPLINE",				FANG_OFFSETOF(CSpawnSysBuilder, m_bIsSpawnSpline_Visible),				BDT_BOOL,			0,																	NULL,									NULL,									NO_TRACKGROUPEDBOOLOFFSET,				NULL,			NULL,			
	"SS_NET_DOORWAY_PT_SPLINE",				FANG_OFFSETOF(CSpawnSysBuilder, m_bIsSpawnSpline_Doorway),				BDT_BOOL,			0,																	NULL,									NULL,									NO_TRACKGROUPEDBOOLOFFSET,				NULL,			NULL,			
	"SS_NET_PREPLACED_PT_SPLINE",			FANG_OFFSETOF(CSpawnSysBuilder, m_bIsSpawnSpline_Preplaced),			BDT_BOOL,			0,																	NULL,									NULL,									NO_TRACKGROUPEDBOOLOFFSET,				NULL,			NULL,			
	"SS_NET_DEFCONTRIPWIRE",				FANG_OFFSETOF(CSpawnSysBuilder, m_bIsSpawnNetTripwire),					BDT_BOOL,			0,																	NULL,									NULL,									NO_TRACKGROUPEDBOOLOFFSET,				NULL,			NULL,			
	"SS_NET_SWARMEROBSTACLE",				FANG_OFFSETOF(CSpawnSysBuilder, m_bIsSwarmerObstacle),					BDT_BOOL,			0,																	NULL,									NULL,									NO_TRACKGROUPEDBOOLOFFSET,				NULL,			NULL,			
	"SS_NET_SWARMEROBSTACLE_CTRL",			FANG_OFFSETOF(CSpawnSysBuilder, m_uObstacleFlags),						BDT_ENUMS2BITS,		sizeof(_aObstacleCtrl_Use)/sizeof(BuilderHelp_EnumTagSpec),			NULL,									_aObstacleCtrl_Use,						NO_TRACKGROUPEDBOOLOFFSET,				NULL,			NULL,			
	"SS_NET_SWARMEROBSTACLE_RADIUS",		FANG_OFFSETOF(CSpawnSysBuilder, m_fObstacleRadius),						BDT_F32,			0,																	&BuilderHelp_PosF32MMS,					NULL,									NO_TRACKGROUPEDBOOLOFFSET,				NULL,			NULL,			
																																																																																					
																																																																																					
	"SS_NET_ENTIYSPAWNER",					FANG_OFFSETOF(CSpawnSysBuilder, m_bIsEntitySpawner),					BDT_BOOL,			0,																	NULL,									NULL,									NO_TRACKGROUPEDBOOLOFFSET,				NULL,			NULL,			
	"SS_ENTITYSPAWNER_CTRL",				FANG_OFFSETOF(CSpawnSysBuilder, m_uEntitySpawnerFlags),					BDT_ENUMS2BITS,		sizeof(_aEntitySpawnnerCtrl_Use)/sizeof(BuilderHelp_EnumTagSpec),	NULL,									_aEntitySpawnnerCtrl_Use,				NO_TRACKGROUPEDBOOLOFFSET,				NULL,			NULL,			
	"SS_ENTITYSPAWNER_NUM",					FANG_OFFSETOF(CSpawnSysBuilder, m_uEntitySpawnerNumToSpawnOnDeath),		BDT_U32,			0,																	&BuilderHelp_U8,						NULL,									NO_TRACKGROUPEDBOOLOFFSET,				NULL,			NULL,			
	"SS_DEFCONTRIPWIRE_CTRL",				FANG_OFFSETOF(CSpawnSysBuilder, m_uSpawnNetTripwireFlags),				BDT_ENUMS2BITS,		sizeof(_aEntityTripCtrl_Use)/sizeof(BuilderHelp_EnumTagSpec),		NULL,									_aEntityTripCtrl_Use,					NO_TRACKGROUPEDBOOLOFFSET,				NULL,			NULL,			
	"SS_DEFCONTRIPWIRE_DEFCON",				FANG_OFFSETOF(CSpawnSysBuilder, m_uSpawnNetTripwireForceDefconLevel),	BDT_U32,			0,																	&BuilderHelp_U8,						NULL,									NO_TRACKGROUPEDBOOLOFFSET,				NULL,			NULL,			
};																																																																																					
																																																																																					
																																																																																					
void CSpawnSysBuilder::SetDefaults( CEntityBuilder* pBuilder )
{
	BuilderHelp_SetDefaults(this, _aItableDataSpec, sizeof(_aItableDataSpec)/sizeof(InterpretTableDataSpec));


	//SpawnNet
	m_uSpawnNetFlags = 0xffffffff;
	m_pszSpawnNetName = NULL;
	m_uActiveTimeMin = 0xffffffff; 
	m_uActiveTimeMax = 0xffffffff;
	m_uBotType = 0xffffffff;
	m_uNumToSpawnLifetime = 0xffffffff;
	m_uSwarmerBotWanderDist = 0xffffffff;		//How far bots spawned by this net are allowed to wander from their spawn pt.
	m_uSwarmerBotAttackDist = 0xffffffff;		//How far bots spawned by this net are allowed to attack from their spawn pt.
	m_fSwarmerBotAttackSpeedNorm = 0.5f;
	m_bIsSpawnSpline_Hidden = FALSE;
	m_bIsSpawnSpline_Visible = FALSE;

	m_fBurstDelay_0 = -1.0f;
	m_fFireDelay_0 = -1.0f;
	m_uNumFiresPerBurst_0 = 0xffffffff;
	m_uNumLivingLimit_0 = 0xffffffff;

	m_fBurstDelayBonus_0 = -1.0f;
	m_fFireDelayBonus_0 = -1.0f;
	m_uNumFiresPerBurstBonus_0 = 0xffffffff;

	m_fBurstDelay_1 = -1.0f;
	m_fFireDelay_1 = -1.0f;
	m_uNumFiresPerBurst_1 = 0xffffffff;
	m_uNumLivingLimit_1 = 0xffffffff;

	m_fBurstDelayBonus_1 = -1.0f;
	m_fFireDelayBonus_1 = -1.0f;
	m_uNumFiresPerBurstBonus_1 = 0xffffffff;

	m_fBurstDelay_2 = -1.0f;
	m_fFireDelay_2 = -1.0f;
	m_uNumFiresPerBurst_2 = 0xffffffff;
	m_uNumLivingLimit_2 = 0xffffffff;

	m_fBurstDelayBonus_2 = -1.0f;
	m_fFireDelayBonus_2 = -1.0f;
	m_uNumFiresPerBurstBonus_2 = 0xffffffff;

	m_fBurstDelay_3 = -1.0f;
	m_fFireDelay_3 = -1.0f;
	m_uNumFiresPerBurst_3 = 0xffffffff;
	m_uNumLivingLimit_3 = 0xffffffff;

	m_fBurstDelayBonus_3 = -1.0f;
	m_fFireDelayBonus_3 = -1.0f;
	m_uNumFiresPerBurstBonus_3 = 0xffffffff;


	//CEntitySpawner
	m_bIsEntitySpawner = FALSE;
	m_uEntitySpawnerFlags = 0xffffffff;
	m_uEntitySpawnerNumToSpawnOnDeath = 0xffffffff;

	//CSpawnNetTripwire
	m_bIsSpawnNetTripwire = FALSE;
	m_uSpawnNetTripwireForceDefconLevel = 0xffffffff;
	m_uSpawnNetTripwireFlags = 0xffffffff;

	//CSwarmerObstacle
	m_bIsSwarmerObstacle = FALSE;
	m_uObstacleFlags = 0xffffffff;
	m_fObstacleRadius = -1.0f;
	
}


BOOL CSpawnSysBuilder::InterpretTable( CEntityBuilder* pBuilder )
{
	BOOL bDidInterpret = FALSE;
	if (CEntityParser::m_pszTableName && !fclib_strnicmp(CEntityParser::m_pszTableName, "SS_", 3))
	{
		BuilderHelp_InterpretTable(_aItableDataSpec, sizeof(_aItableDataSpec)/sizeof(InterpretTableDataSpec), &bDidInterpret);
	}
	return bDidInterpret;
}


BOOL CSpawnSysBuilder::PostInterpretFixup( CEntityBuilder* pBuilder )
{
	if (m_pszSpawnNetName)
	{
		int i = 4;
	}

	if (m_pszSpawnNetName && !(m_pszSpawnNetName = gstring_Main.AddString( m_pszSpawnNetName)) )
	{  //couldn't fit that string into the global table
		goto _XOutOfMemory;
	}

	return TRUE;

_XOutOfMemory:
	CEntityParser::Error_OutOfMemory();
	return FALSE;
}


CSpawnNet* CSpawnSysBuilder::_AllocOrFindExistingSpawnNet(CEntity* pEntity)
{
	CSpawnNet* pSpawnNet = NULL;
	if (!m_pszSpawnNetName)
	{
		DEVPRINTF("Error: Spawn (%s) with no Spawn Net Name", (pEntity && pEntity->Name()) ? pEntity->Name(): "NONAME");
	}
	pSpawnNet = CSpawnSys::FindSpawnNet(m_pszSpawnNetName);
	if (!pSpawnNet)
	{
		pSpawnNet = fnew CSpawnNet();
		if (pSpawnNet)
		{
			CSpawnSys::AddSpawnNet(pSpawnNet);
			pSpawnNet->m_pszSpawnNetName = m_pszSpawnNetName;
		}
	}
	return pSpawnNet;
}



BOOL CSpawnSysBuilder::BuildSpawnSysObjIfNeeded( CEntity* pEntity)
{
	FResFrame_t ResFrame = fres_GetFrame();
	CSpawnNet* pNet = NULL;
   
	if (m_bIsSpawnSpline_Hidden ||
		m_bIsSpawnSpline_Visible ||
		m_bIsEntitySpawner ||
		m_bIsSpawnNetTripwire ||
		m_bIsSwarmerObstacle)
	{
		pNet = _AllocOrFindExistingSpawnNet(pEntity);
		if (!pNet)
		{
			goto  _ExitBuildSpawnSysIfNeeded_WithError;
		}
	}
	else
	{
		return TRUE;
	}

	if (m_bIsSpawnSpline_Hidden)
	{
		if (!(pEntity->TypeBits() & ENTITY_BIT_SPLINE))
		{
			DEVPRINTF("Error: spawnsys spline (%s) is not a spline object", (pEntity && pEntity->Name()) ? pEntity->Name(): "NONAME");
		}
		else 
		{
			pNet->m_pHiddenSpawnPts = (CESpline*) pEntity;

			pNet->m_pauHiddenSpawnPtVol = (u16*) fres_AlignedAllocAndZero(pNet->m_pHiddenSpawnPts->PointCount()*sizeof(u16), 16);
			if (!pNet->m_pauHiddenSpawnPtVol)
			{
				goto _ExitBuildSpawnSysIfNeeded_WithError;
			}

			for (u32 i = 0; i < pNet->m_pHiddenSpawnPts->PointCount(); i++)
			{
				FVisVolume_t* pVol = NULL;
				if (fworld_GetVolumeContainingPoint(&(pNet->m_pHiddenSpawnPts->PointArray()[i]), &pVol) > 0.0f && pVol)
				{
					pNet->m_pauHiddenSpawnPtVol[i] = pVol->nVolumeID;
				}
				else
				{
					pNet->m_pauHiddenSpawnPtVol[i] = 0xffff;
				}
			}
		}
	}
	else if (m_bIsSpawnSpline_Visible)
	{
		if (!(pEntity->TypeBits() & ENTITY_BIT_SPLINE))
		{
			DEVPRINTF("Error: spawnsys spline (%s) is not a spline object", (pEntity && pEntity->Name()) ? pEntity->Name(): "NONAME");
		}
		else 
		{
			pNet->m_pVisibleSpawnPts = (CESpline*) pEntity;
		}
	}
	else if (m_bIsSpawnSpline_Doorway)
	{
		if (!(pEntity->TypeBits() & ENTITY_BIT_SPLINE))
		{
			DEVPRINTF("Error: spawnsys spline (%s) is not a spline object", (pEntity && pEntity->Name()) ? pEntity->Name(): "NONAME");
		}
		else 
		{
			pNet->m_pDoorwaySpawnPts = (CESpline*) pEntity;
		}
	}
	else if (m_bIsSpawnSpline_Preplaced)
	{
		if (!(pEntity->TypeBits() & ENTITY_BIT_SPLINE))
		{
			DEVPRINTF("Error: spawnsys spline (%s) is not a spline object", (pEntity && pEntity->Name()) ? pEntity->Name(): "NONAME");
		}
		else 
		{
			pNet->m_pPreplacedSpawnPts = (CESpline*) pEntity;
		}
	}
	else if (m_bIsEntitySpawner)
	{
		CEntitySpawner* pSpawner = fnew CEntitySpawner();
		if (!pSpawner)
		{
			goto _ExitBuildSpawnSysIfNeeded_WithError;
		}
		pSpawner->m_pEntity = pEntity;
		BuilderHelp_SetU8IfValid(&(pSpawner->m_uEntitySpawnerFlags ), m_uEntitySpawnerFlags );
		BuilderHelp_SetU8IfValid(&(pSpawner->m_uNumToSpawnOnDeath), m_uEntitySpawnerNumToSpawnOnDeath );
		pNet->AddEntitySpawner(pSpawner);
	}
	else if (m_bIsSpawnNetTripwire)
	{
		CSpawnNetTripwire* pTrip = fnew CSpawnNetTripwire();
		if (!pTrip)
		{
			goto _ExitBuildSpawnSysIfNeeded_WithError;
		}
		pTrip->m_pEntity = pEntity;
		BuilderHelp_SetU8IfValid(&(pTrip->m_uTripwireFlags ), m_uSpawnNetTripwireFlags );
		BuilderHelp_SetU8IfValid(&(pTrip->m_uForceDefconLevel), m_uSpawnNetTripwireForceDefconLevel );
		pNet->AddTripwire(pTrip);
	}
	else if (m_bIsSwarmerObstacle)
	{
		CSwarmerBotObstacle* pObstacle = fnew CSwarmerBotObstacle();
		if (!pObstacle)
		{
			goto _ExitBuildSpawnSysIfNeeded_WithError;
		}

		pObstacle->m_pEntity = pEntity;
		BuilderHelp_SetU8IfValid(&(pObstacle->m_uSwarmerBotObstacleFlags), m_uObstacleFlags );
		pObstacle->m_fRadius = m_fObstacleRadius;
		pNet->AddSwarmerObstacle(pObstacle);
	}

	if (pNet)
	{
		BuilderHelp_SetU8IfValid(&(pNet->m_uSpawnNetFlags ),m_uSpawnNetFlags );
		BuilderHelp_SetU8IfValid(&(pNet->m_uActiveTimeMin) , m_uActiveTimeMin );
		BuilderHelp_SetU8IfValid(&(pNet->m_uActiveTimeMax) , m_uActiveTimeMax );
		BuilderHelp_SetU8IfValid(&(pNet->m_uBotType) , m_uBotType );
		BuilderHelp_SetU16IfValid(&(pNet->m_uNumToSpawnLifetime ), m_uNumToSpawnLifetime );
		BuilderHelp_SetU16IfValid(&(pNet->m_uSwarmerBotWanderDist) , m_uSwarmerBotWanderDist );
		BuilderHelp_SetU16IfValid(&(pNet->m_uSwarmerBotAttackDist ), m_uSwarmerBotAttackDist );
		BuilderHelp_SetF32IfValid(&(pNet->m_aSpawnRateCtrl[0].m_fBurstDelay), m_fBurstDelay_0 );
		BuilderHelp_SetF32IfValid(&(pNet->m_aSpawnRateCtrl[0].m_fFireDelay), m_fFireDelay_0 );
		BuilderHelp_SetU8IfValid(&(pNet->m_aSpawnRateCtrl[0].m_uNumFiresPerBurst), m_uNumFiresPerBurst_0 );
		BuilderHelp_SetU16IfValid(&(pNet->m_anNumLivingLimit[0]), m_uNumLivingLimit_0 );
		BuilderHelp_SetF32IfValid(&(pNet->m_aSpawnRateCtrl[0].m_fBurstDelayBonus), m_fBurstDelayBonus_0 );
		BuilderHelp_SetF32IfValid(&(pNet->m_aSpawnRateCtrl[0].m_fFireDelayBonus), m_fFireDelayBonus_0 );
		BuilderHelp_SetU8IfValid(&(pNet->m_aSpawnRateCtrl[0].m_uNumFiresPerBurstBonus), m_uNumFiresPerBurstBonus_0 );
		BuilderHelp_SetF32IfValid(&(pNet->m_aSpawnRateCtrl[1].m_fBurstDelay), m_fBurstDelay_1 );
		BuilderHelp_SetF32IfValid(&(pNet->m_aSpawnRateCtrl[1].m_fFireDelay), m_fFireDelay_1 );
		BuilderHelp_SetU8IfValid(&(pNet->m_aSpawnRateCtrl[1].m_uNumFiresPerBurst), m_uNumFiresPerBurst_1 );
		BuilderHelp_SetU16IfValid(&(pNet->m_anNumLivingLimit[1]), m_uNumLivingLimit_1 );
		BuilderHelp_SetF32IfValid(&(pNet->m_aSpawnRateCtrl[1].m_fBurstDelayBonus), m_fBurstDelayBonus_1 );
		BuilderHelp_SetF32IfValid(&(pNet->m_aSpawnRateCtrl[1].m_fFireDelayBonus), m_fFireDelayBonus_1 );
		BuilderHelp_SetU8IfValid(&(pNet->m_aSpawnRateCtrl[1].m_uNumFiresPerBurstBonus), m_uNumFiresPerBurstBonus_1 );
		BuilderHelp_SetF32IfValid(&(pNet->m_aSpawnRateCtrl[2].m_fBurstDelay), m_fBurstDelay_2 );
		BuilderHelp_SetF32IfValid(&(pNet->m_aSpawnRateCtrl[2].m_fFireDelay), m_fFireDelay_2 );
		BuilderHelp_SetU8IfValid(&(pNet->m_aSpawnRateCtrl[2].m_uNumFiresPerBurst), m_uNumFiresPerBurst_2 );
		BuilderHelp_SetU16IfValid(&(pNet->m_anNumLivingLimit[2]), m_uNumLivingLimit_2 );
		BuilderHelp_SetF32IfValid(&(pNet->m_aSpawnRateCtrl[2].m_fBurstDelayBonus), m_fBurstDelayBonus_2 );
		BuilderHelp_SetF32IfValid(&(pNet->m_aSpawnRateCtrl[2].m_fFireDelayBonus), m_fFireDelayBonus_2 );
		BuilderHelp_SetU8IfValid(&(pNet->m_aSpawnRateCtrl[2].m_uNumFiresPerBurstBonus), m_uNumFiresPerBurstBonus_2 );
		BuilderHelp_SetF32IfValid(&(pNet->m_aSpawnRateCtrl[3].m_fBurstDelay), m_fBurstDelay_3 );
		BuilderHelp_SetF32IfValid(&(pNet->m_aSpawnRateCtrl[3].m_fFireDelay), m_fFireDelay_3 );
		BuilderHelp_SetU8IfValid(&(pNet->m_aSpawnRateCtrl[3].m_uNumFiresPerBurst), m_uNumFiresPerBurst_3 );
		BuilderHelp_SetU16IfValid(&(pNet->m_anNumLivingLimit[3]), m_uNumLivingLimit_3 );
		BuilderHelp_SetF32IfValid(&(pNet->m_aSpawnRateCtrl[3].m_fBurstDelayBonus), m_fBurstDelayBonus_3 );
		BuilderHelp_SetF32IfValid(&(pNet->m_aSpawnRateCtrl[3].m_fFireDelayBonus), m_fFireDelayBonus_3 );
		BuilderHelp_SetU8IfValid(&(pNet->m_aSpawnRateCtrl[3].m_uNumFiresPerBurstBonus), m_uNumFiresPerBurstBonus_3 );
	}

	return TRUE;

_ExitBuildSpawnSysIfNeeded_WithError:
	DEVPRINTF( "spawnsys builder(): No Memory to allocate something small.\n" );
	fres_ReleaseFrame(ResFrame);
	return FALSE;
}
