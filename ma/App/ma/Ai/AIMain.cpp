#include "fang.h"
#include "AIGraph.h"
#include "AIGameUtils.h"
#include "AIRooms.h"
#include "AIBrainman.h"
#include "AIBrain.h"
#include "AIMain.h"
#include "AIMover.h"
#include "AIEdgeLock.h"
#include "AIGraphSearcher.h"
#include "AIFormations.h"
#include "AIEnviro.h"
#include "AiApi.h"
#include "AIPatrolPath.h"
#include "AIBTATable.h"
#include "AIBotMover.h"
#include "AI3DBotMover.h"
#include "AIThoughtsGround.h"
#include "apenew.h"
#include "flinklist.h"
#include "fdraw.h"
#include "ffile.h"
#include "fclib.h"
#include "ftext.h"
#include "fres.h"
#include "frenderer.h"
#include "gamepad.h"
#include "aigroup.h"
#include "../player.h"
#include "../Protrack.h"
#include "../bot.h"  //player position
#include "../botgrunt.h"
#include "../gameloop.h"
#include "../level.h"
#include "../weapon.h"
#include "../Door.h"
#define NIPTR_NODE_POOL_SIZE 5000
FLinkRoot_t			aimain_NodePool;
void				*aimain_pNodePoolMem = NULL;
FLinkRoot_t			*aimain_pNodePool = NULL;

//some things for debugging the graph path finder
CAIGraph			*aimain_pAIGraph = NULL;
CGraphSearcher		*aimain_pGraphSearcher = NULL;
void				*aimain_pGraphFileData = NULL;
CAIGraphDataAccess	*aimain_pGraphAccess = NULL;

static FDrawVtx_t			_aoDrawGuidesVertsBuffer[2];

float _fCuRad = 1.0f;
s32 _searcherCycles = 0;
CFVec3 _CurGoalLoc;
GraphVert* _CurGoalVert = NULL;
CFVec3 _CurStartLoc;
GraphVert* _CurStartVert = NULL;
BOOL _bAIDebugMenuOn = FALSE;
char _szAIDebugMenuMode[128];
char _szAIDebugMenuSubMode[128];
char _szTextDisplayBuffer[128];
BOOL bDidLevelMemDump = FALSE;
#define BLINK_MASK  0x4
BOOL _bPrevMenuModeButtonStatus = 0;
BOOL _bPrevMenuSubModeButtonStatus = 0;
BOOL _bMenuModeButtonStatus = 0;
BOOL _bMenuSubModeButtonStatus = 0;
u32 uMenuOnOffCounter = 0;



u32 _nAIDebugMenuModes = 0;
u32 _nGraphRenderStyle = 0;
u32 _nBrainRenderStyle = 0;


static void _AISystemDestroyed( void *pResMem );

BOOL _LOSTestIgnoreBots(const CFVec3A &rRayStart, const CFVec3A &rRayEnd)
{

	return CEntity::IsLOSObstructed(&rRayStart, &rRayEnd, ENTITY_BITS_ALLBOTBITS, 0, NULL);
}




BOOL _InitPathFinding(const char* pszLevelFilename)
{
	FFileHandle hFile;
// code to test load/save of ascii and binary graph files	
//	aimain_pAIGraph = new CAIGraph();
//	aimain_pAIGraph->LoadFromAsciiFile("c:\\graph1.gt");
//	aimain_pAIGraph->AutoSetDimensions();

/*	aimain_pAIGraph->SaveToAsciiFile("c:\\graph2.gt");

	aimain_pAIGraph->SaveToBinaryFile("c:\\graph2.gb");

	CAIGraph *aimain_pAIGraphQuickLoad = NULL;

	FILE* fp = fopen("c:\\graph2.gb", "rb");

	if (fp)
	{
		fseek(fp, 0, SEEK_END);
		s32 fileLen = ftell(fp);
		fseek(fp, 0, SEEK_SET);
		void* pFileData = aiutil_GetMem(fileLen);
		fread(pFileData, fileLen, 1, fp);
		fclose(fp);
		aimain_pAIGraphQuickLoad = (CAIGraph*) pFileData;
		aimain_pAIGraphQuickLoad->PointerizeBinaryData();
	}

*/
	char szGraphFileName[128];
	if (!pszLevelFilename || fclib_strlen(pszLevelFilename) < 1 || fclib_strlen(pszLevelFilename) > 128)
	{
		goto _ExitInitPathFindingWithError;
	}
	FASSERT(pszLevelFilename); //you better give me a good level name!
	fclib_strcpy(szGraphFileName, pszLevelFilename);
	fclib_strcat(szGraphFileName, ".gt");
	hFile = ffile_Open( szGraphFileName, FFILE_OPEN_RONLY );
	if (FFILE_IS_VALID_HANDLE(hFile))
	{
		s32 nSizeBytes = ffile_GetFileSize( hFile );
		if (nSizeBytes < (s32)sizeof(CAIGraph))
		{
			DEVPRINTF( "_InitPathFinding(): Invalid graph file '%s' (%d bytes).\n", szGraphFileName, nSizeBytes );
			ffile_Close( hFile );
			goto _ExitInitPathFindingWithError;
		}
		aimain_pGraphFileData = fres_AlignedAlloc(nSizeBytes, 32); //important to align this since caigraph is requires it
		if (!aimain_pGraphFileData)
		{
			ffile_Close( hFile );
			goto _ExitInitPathFindingWithError;
		}
		s32 nCount = ffile_Read( hFile, nSizeBytes, aimain_pGraphFileData );
		ffile_Close( hFile );
		if (nCount != nSizeBytes)
		{
			DEVPRINTF( "_InitPathFinding(): Short read on graph file '%s' (%d/%d bytes).\n", szGraphFileName, nCount, nSizeBytes );
			goto _ExitInitPathFindingWithError;
		}
		aimain_pAIGraph = (CAIGraph*) aimain_pGraphFileData;
		#if FANG_WINGC
		if (!aimain_pAIGraph->ConvertGameCubeBinaryData((u32)nSizeBytes))
		{
			goto _ExitInitPathFindingWithError;
		}
		#endif
		aimain_pAIGraph->PointerizeBinaryData();
	}
	else
	{	//there isn't any graph for this level.
		aimain_pAIGraph = APE_NEW CAIGraph();
	}


	AIGraph_SetLOSTestFunc(_LOSTestIgnoreBots);

	aimain_pGraphSearcher = APE_NEW CGraphSearcher();
	if (!aimain_pGraphSearcher)
	{
		goto _ExitInitPathFindingWithError;
	}
	if (!aimain_pGraphSearcher->BindToGraph(aimain_pAIGraph))
	{
		goto _ExitInitPathFindingWithError;
	}
	aimain_pGraphSearcher->UsePNodePool(aimain_pNodePool);

	return TRUE;
_ExitInitPathFindingWithError:
	return FALSE;
}


void _UninitPathFinding(void)
{
	APE_DELETE(aimain_pGraphSearcher); aimain_pGraphSearcher = NULL;
	if (aimain_pGraphFileData)
	{
	//	fres_Free(aimain_pGraphFileData);
		aimain_pGraphFileData = NULL;
		aimain_pAIGraph = NULL;
	}
	else if (aimain_pAIGraph)
	{
		APE_DELETE(aimain_pAIGraph); aimain_pAIGraph = NULL;
	}
	if (aimain_pGraphAccess)
	{
		APE_DELETE(aimain_pGraphAccess); aimain_pGraphAccess = NULL;
	}

}


BOOL _InitGlobalNodePools(void)
{
	s32 nNiPtrNodeSize;

	// initialize the void* Node Pool 
	nNiPtrNodeSize = sizeof(CNiNode<void*>);
	aimain_pNodePoolMem = fres_Alloc(NIPTR_NODE_POOL_SIZE * nNiPtrNodeSize );
	if (!aimain_pNodePoolMem)
	{
		goto _ExitInitGlobalNodePoolsWithError;
	}
	flinklist_InitRoot(&aimain_NodePool, 0);
	flinklist_InitPool(&aimain_NodePool, aimain_pNodePoolMem, nNiPtrNodeSize, NIPTR_NODE_POOL_SIZE);
	aimain_pNodePool = &aimain_NodePool;
	

	return TRUE;
_ExitInitGlobalNodePoolsWithError:
	return FALSE;
}


void _UninitGlobalNodePools(void)
{
	if (aimain_pNodePoolMem)
	{
		//	fres_Free(aimain_pNodePoolMem);
		aimain_pNodePoolMem = NULL;
	}
}


BOOL aimain_InitSystem(const char* pszLevelFilename)
{
	FResFrame_t ResFrame = fres_GetFrame();

	if( fres_CreateWithCallback( NULL, NULL, _AISystemDestroyed ) == FRES_NULLHANDLE )
	{
		goto _ExitAIMain_InitSysWithError;
	}

	if (!_InitGlobalNodePools())
	{
		goto _ExitAIMain_InitSysWithError;
	}

	if (!_InitPathFinding(pszLevelFilename))
	{
		goto _ExitAIMain_InitSysWithError;
	}

	if (!airooms_InitSystem(aimain_pAIGraph))
	{
		goto _ExitAIMain_InitSysWithError;
	}

	if (!ResLock_InitSystem())
	{
		goto _ExitAIMain_InitSysWithError;
	}

	if (!CAIPath::InitSystem(aimain_pNodePool))
	{
		goto _ExitAIMain_InitSysWithError;
	}

	if (!CPatrolPath::InitSystem())
	{
		goto _ExitAIMain_InitSysWithError;
	}

	if (!CAIKnowledge::InitSystem(aimain_pNodePool))
	{
		goto _ExitAIMain_InitSysWithError;
	}

	if (!AIFormations_InitSystem(aimain_pNodePool))
	{
		goto _ExitAIMain_InitSysWithError;
	}

	if (!AIEnviro_InitSystem(aimain_pNodePool))
	{
		goto _ExitAIMain_InitSysWithError;
	}

	if (!aibrainman_InitSys())
	{
		goto _ExitAIMain_InitSysWithError;
	}

	if (!CAIBrain::InitLevel(aimain_pNodePool))
	{
		goto _ExitAIMain_InitSysWithError;
	}

	if (!aiutils_InitSystem())
	{
		goto _ExitAIMain_InitSysWithError;
	}
	
	aigroup_InitSystem();

	aigroup_InitLevel();

	AIBTA_InitSystem();

#if (SAS_ACTIVE_USER == SAS_USER_PAT)
	DEVPRINTF("Size Of CEntity: %d bytes\n", sizeof(CEntity));
	DEVPRINTF("Size Of CBot: %d bytes\n", sizeof(CBot));
	DEVPRINTF("Size Of CBotGrunt: %d bytes\n", sizeof(CBotGrunt));
	DEVPRINTF("Size Of CWeapon: %d bytes\n", sizeof(CWeapon));
	DEVPRINTF("Size Of CSearchQuery: %d bytes\n", sizeof(CSearchQuery));
	DEVPRINTF("Size Of CAIBrain: %d bytes\n", sizeof(CAIBrain));
	DEVPRINTF("Size Of CAIBotMover: %d bytes\n", sizeof(CAIBotMover));
	DEVPRINTF("Size Of CAI3DBotMover: %d bytes\n", sizeof(CAI3DBotMover));
	DEVPRINTF("Size Of CAIControlGoals: %d bytes\n", sizeof(CAIControlGoals));
//	DEVPRINTF("Size Of CSearchResults: %d bytes\n", sizeof(CSearchResults));
	DEVPRINTF("Size Of CGroundCombat: %d bytes\n", sizeof(CGroundCombat));
#endif // SAS_USER_PAT

	bDidLevelMemDump = FALSE;

	ai_DisableUnarmedPlayerRule();

#if !FANG_PRODUCTION_BUILD	
	_nNumRaysDebugTracking = 0;
#endif	

	return TRUE;

_ExitAIMain_InitSysWithError:
	DEVPRINTF( "aimain_InitSystem(): Could not create AI system resource.\n" );
	fres_ReleaseFrame( ResFrame );
	return FALSE;
}

BOOL aimain_InitSystemPostWorldLoad(void)
{
	BOOL bDidIt;
	aimain_pGraphAccess = APE_NEW CAIGraphDataAccess;
	if (aimain_pGraphAccess)
	{
		bDidIt = aimain_pGraphAccess->Init(aimain_pAIGraph);
		if (bDidIt)
		{
			aimain_pAIGraph->BindWithAccessData(aimain_pGraphAccess);

			//find any site weapons in the world and register them.
			CEntity* pEntity = CEntity::InWorldList_GetHead();
			while (pEntity)
			{
				if ((pEntity->TypeBits() & ENTITY_BIT_VEHICLESENTINEL) ||
				    (pEntity->TypeBits() & ENTITY_BIT_VEHICLELOADER) ||
				    (pEntity->TypeBits() & ENTITY_BIT_BOT && (((CBot*)pEntity)->IsPillBox() || ((CBot*)pEntity)->IsRatGun())) )
				{
					aigroup_RegisterAsMech((CBot*) pEntity);
				}
				pEntity = pEntity->InWorldList_GetNext();
			}
		}
	}
	return bDidIt;
}


static void _AISystemDestroyed( void *pResMem )
{
	aigroup_UninitLevel();
	aigroup_UninitSystem();
	aiutils_UninitSystem();
	CAIBrain::UninitLevel();
	aibrainman_UninitSys();
	AIEnviro_UninitSystem();
	AIFormations_UninitSystem();
	CAIKnowledge::CleanupSystem();
	CPatrolPath::UninitSystem();
	CAIPath::UninitSystem();
	ResLock_UninitSystem();
	airooms_CleanupSystem();
	AIBTA_UninitSystem();
	_UninitPathFinding();
	_UninitGlobalNodePools();  	//should be very last
}

void aimain_Work(void)
{
	PROTRACK_BEGINBLOCK("AI_WORK");

	AIEnviro_Work();

	aibrainman_Work();

	aigroup_Work();

	PROTRACK_BEGINBLOCK("AI_PF");
	 _searcherCycles = 0;
	aimain_pGraphSearcher->Work(&_searcherCycles);
	PROTRACK_ENDBLOCK();//"AI_PF");

	CPatrolPath::Work();

	PROTRACK_ENDBLOCK();//"AI");
}

BOOL DebugRenderCBot(CBot* pBot)
{
	BOOL bInView = FALSE;

	FViewport_t * pView = fviewport_GetActive();
	if (pView)
	{
		CFVec3A ScreenPos;
		ScreenPos.z = -1.0f;
		f32 fLineHeight = 0.02f;

		CFVec3A Point_VS;
		Point_VS = *pBot->GetTagPoint(0);
		FXfm_pView->m_MtxF.MulPoint(Point_VS);
		if (Point_VS.z > 0.0f)
		{
			CFVec3A loc = pBot->MtxToWorld()->m_vPos;
			loc.y += pBot->m_fCollCylinderHeight_WS;

			fviewport_ComputeUnitOrtho3DScreenPoint_WS(pView, &(FXfm_pView->m_MtxF), &loc, &ScreenPos);
			if (ScreenPos.z >0.0f && ScreenPos.x < 1.0f && ScreenPos.x > -1.0f && ScreenPos.y < 1.0f && ScreenPos.y > -1.0f)
			{
				ScreenPos.x = 0.5f+0.5f*ScreenPos.x;
				ScreenPos.y = (0.375f -ScreenPos.y*0.375f);
				bInView = TRUE;
			}
		}
		if (bInView)
		{
			pBot->DebugRender(ScreenPos.x, ScreenPos.y+fLineHeight);
		}
	}

	return bInView;
}


BOOL DebugRenderCBot_ActiveBrainCBFunc(CAIBrain* pBrain, void* pData)
{
	if (aiutils_IsCBotBrain(pBrain))
	{
		CBot* pBot = (CBot*) pBrain->GetAIMover()->GetEntity();
		DebugRenderCBot(pBot);
	}

	return AIBRAINMAN_CONTINUE_ITERATION;
}


void aimain_SetDebugMenuModeString(cchar* pszString)
{
	FASSERT(pszString && fclib_strlen(pszString)+16 < sizeof(_szAIDebugMenuMode));
	fclib_strcpy(_szAIDebugMenuMode, pszString);
}

void aimain_SetDebugMenuSubModeString(cchar* pszString)
{
	FASSERT(pszString && fclib_strlen(pszString)+16 < sizeof(_szAIDebugMenuSubMode));
	fclib_strcpy(_szAIDebugMenuSubMode, pszString);
}


enum
{
	AIMENUMODE_SHOW_NOTHING,
	AIMENUMODE_BRAINS,
	AIMENUMODE_GRAPH,
	AIMENUMODE_BRAINS_GRAPH,
	AIMENUMODE_B_G_LOS,
	AIMENUMODE_LOS_TESTS,
	AIMENUMODE_ACTIVE_INACTIVE,
	AIMENUMODE_HEARING,
	AIMENUMODE_VISION,
	AIMENUMODE_HEAPMEM,
	AIMENUMODE_BOTLAYER,
	AIMENUMODE_DOORS_LIFTS,
	NUM_AIMENUMODES,
};

void _IncAIMenuMode(void)
{
	_nAIDebugMenuModes++;
	if (_nAIDebugMenuModes >= NUM_AIMENUMODES)
	{
		_nAIDebugMenuModes = 0;
	}
}

static cchar *_apszAiDebugModes[NUM_AIMENUMODES] = 
{
	"Show Nothing",
	"Brains",
	"Graph",
	"Brains & Graph",
	"B+G+LOS",
	"LOS Tests",
	"ActiveBrains",
	"Reaction",
	"Vision",
	"HeapMem",
	"BotLayer",
	"Doors/Lifts"
};


void aimain_Draw(void)
{
#if !FANG_PRODUCTION_BUILD
	//UI

	// use the debug controller port index
	s32 nAIDebugControllerId = Gamepad_nDebugPortIndex;
	static u32 uBlindDeafAndDumbControllerHoldCounter = 0;

	//
	//	Blind Deaf And Dumb Mode.. makes AI just that
	//
	if ( (Gamepad_aapSample[nAIDebugControllerId][GAMEPAD_MAIN_FIRE_PRIMARY]->uLatches & FPAD_LATCH_ON) &&
		(Gamepad_aapSample[nAIDebugControllerId][GAMEPAD_MAIN_FIRE_SECONDARY]->uLatches & FPAD_LATCH_ON))
	{
		if (uBlindDeafAndDumbControllerHoldCounter < 30)
		{
			uBlindDeafAndDumbControllerHoldCounter++;
			if (uBlindDeafAndDumbControllerHoldCounter == 30)
			{
				CAIBrain::m_bGlobalBlindDefAndDumb ^=1;
			}
		}
	}
	else
	{
		if (uBlindDeafAndDumbControllerHoldCounter > 0)
		{
			uBlindDeafAndDumbControllerHoldCounter--;
		}
	}

	if (CAIBrain::m_bGlobalBlindDefAndDumb)
	{
		ftext_DebugPrintf( 0.1f+0.001f, 0.331f+0.001f, "~C00000099BLIND DEAF AND DUMB :ON");
		ftext_DebugPrintf( 0.1f, 0.331f, "~C99999999BLIND DEAF AND DUMB :ON");
	}

	if (CAIBrain::m_bGlobalIgnoreMode)
	{
		ftext_DebugPrintf( 0.1f+0.001f, 0.331f-0.011f, "~C00000099G:GLOBAL IGNORE :ON");
		ftext_DebugPrintf( 0.1f, 0.331f, "~C99999999GLOBAL IGNORE :ON");
	}

	//
	//	AI Debug Menu
	//
	_bPrevMenuModeButtonStatus = _bMenuModeButtonStatus;
	_bPrevMenuSubModeButtonStatus = _bMenuSubModeButtonStatus;
	_bMenuModeButtonStatus = (Gamepad_aapSample[nAIDebugControllerId][GAMEPAD_MAIN_JUMP]->uLatches & FPAD_LATCH_ON);
	_bMenuSubModeButtonStatus = (Gamepad_aapSample[nAIDebugControllerId][GAMEPAD_MAIN_ACTION]->uLatches & FPAD_LATCH_ON);
	BOOL bDrawTestRays = FALSE;
	if (_bMenuModeButtonStatus && _bMenuSubModeButtonStatus)
	{
		if (uMenuOnOffCounter < 10)
		{
			uMenuOnOffCounter++;
			if (uMenuOnOffCounter == 10)
			{
				_bAIDebugMenuOn = 1 - _bAIDebugMenuOn;
				return; //just so other ui doesn't happen when the menu on doesn't 
			}
		}
	}
	else if (!_bMenuModeButtonStatus && !_bMenuSubModeButtonStatus)
	{
		if (uMenuOnOffCounter > 0)
		{
			uMenuOnOffCounter--;
		}
	}

	if (_bAIDebugMenuOn)
	{
		if (uMenuOnOffCounter < 5 &&
			!_bMenuModeButtonStatus && _bPrevMenuModeButtonStatus)	 //on up
		{
		   _IncAIMenuMode();
		}
		aimain_SetDebugMenuModeString(_apszAiDebugModes[_nAIDebugMenuModes]);
		aimain_SetDebugMenuSubModeString("");

		//RENDER
		frenderer_Push( FRENDERER_DRAW, NULL );
		
		if (_nAIDebugMenuModes > AIMENUMODE_SHOW_NOTHING && _nAIDebugMenuModes < AIMENUMODE_LOS_TESTS)
		{
			if (_nAIDebugMenuModes & (AIMENUMODE_BRAINS | AIMENUMODE_B_G_LOS))
			{
				u32 anBrainRenderStyles[] = 
				{
					AIBM_DISPLAY_BRAIN_STATES | AIBM_DISPLAY_CURTHOUGHT_DATA | AIBM_DISPLAY_NEXT_WAYPOINT | AIBM_DISPLAY_PATROLPATHS,
					AIBM_DISPLAY_BRAIN_NAME | AIBM_DISPLAY_MOVER_INFO,
					AIBM_DISPLAY_KNOWLEDGE_INFO,
					AIBM_DISPLAY_WEAPON_INFO,
					AIBM_DISPLAY_PLAYER_BRAIN,
				};
				if (uMenuOnOffCounter < AIMENUMODE_LOS_TESTS && 
					!_bMenuSubModeButtonStatus && _bPrevMenuSubModeButtonStatus)	//on up
				{
					_nBrainRenderStyle++;
					if (_nBrainRenderStyle >=sizeof(anBrainRenderStyles)/sizeof(u32))
					{
						_nBrainRenderStyle = 0;
					}
				}
				cchar* _aszBrainRenderStyleNames[]   =
				{
					"  JOB/GOAL",
					"  MOVER",
					"  KNOWLEDGE",
					"  WEAPON",
					"  PLAYERBRAIN"
				};
				aimain_SetDebugMenuSubModeString(_aszBrainRenderStyleNames[_nBrainRenderStyle]);
				
				aibrainman_Render(anBrainRenderStyles[_nBrainRenderStyle]);
			}

			if (_nAIDebugMenuModes & (AIMENUMODE_GRAPH|AIMENUMODE_B_G_LOS))
			{
				u32 anGraphRenderStyles[] = 
				{
					AIGRAPH_DEBUG_RENDER_2DMODE | AIGRAPH_DEBUG_RENDER_3DMODE | AIGRAPH_DEBUG_RENDER_EDGES,
					AIGRAPH_DEBUG_RENDER_2DMODE | AIGRAPH_DEBUG_RENDER_2DEDGEVOLS | AIGRAPH_DEBUG_RENDER_2DVERTVOLS,
					AIGRAPH_DEBUG_RENDER_2DMODE | AIGRAPH_DEBUG_RENDER_VERTVOLS,
					AIGRAPH_DEBUG_RENDER_2DMODE | AIGRAPH_DEBUG_RENDER_EDGEVOLS,
					AIGRAPH_DEBUG_RENDER_3DMODE | AIGRAPH_DEBUG_RENDER_VERTVOLS,
					AIGRAPH_DEBUG_RENDER_3DMODE | AIGRAPH_DEBUG_RENDER_EDGEVOLS,
				};
				if (uMenuOnOffCounter < AIMENUMODE_LOS_TESTS &&
					!_bMenuSubModeButtonStatus && _bPrevMenuSubModeButtonStatus)	//on up
				{
					_nGraphRenderStyle++;
					if (_nGraphRenderStyle >= sizeof(anGraphRenderStyles)/sizeof(u32))
					{
						_nGraphRenderStyle = 0;
					}
				}

				cchar* _aszGraphRenderStyleNames[]   =
				{
					"2D+3D Edges",
					"2D Volume",
					"2D Volume Vert Only",
					"2D Volume Edge Only",
					"3D Volume Vert Only",
					"3D Volume Edge Only",
				};
				aimain_SetDebugMenuSubModeString(_aszGraphRenderStyleNames[_nGraphRenderStyle]);

				CFVec3A CamPos;
				CFVec3A CamLookAt;
				if (aiutils_GetCamPos(&CamPos) && aiutils_GetCamLookAt(&CamLookAt))
				{
					
					aimain_pAIGraph->Render(CamPos,
											CamLookAt,
											0.0f, /* cos 90 degs*/
											anGraphRenderStyles[_nGraphRenderStyle]);
				}

			}

			if (_nAIDebugMenuModes == AIMENUMODE_B_G_LOS)
			{
				bDrawTestRays  =TRUE;
			}
		}
		else if (_nAIDebugMenuModes == AIMENUMODE_LOS_TESTS)
		{
				bDrawTestRays  =TRUE;
		}
		else if (_nAIDebugMenuModes == AIMENUMODE_ACTIVE_INACTIVE)
		{
			aibrainman_Render(AIBM_DISPLAY_ACTIVE_LIST);
		}
		else if (_nAIDebugMenuModes == AIMENUMODE_HEARING)
		{
			aibrainman_Render(AIBM_DISPLAY_HEARING | AIBM_DISPLAY_BRAIN_STATES);
			AIEnviro_DebugRender();
		}				
		else if (_nAIDebugMenuModes == AIMENUMODE_VISION)
		{
			aibrainman_Render(AIBM_DISPLAY_VISION | AIBM_DISPLAY_BRAIN_STATES);
			bDrawTestRays = TRUE;
		}
		else if (_nAIDebugMenuModes == AIMENUMODE_HEAPMEM)
		{
			if (uMenuOnOffCounter < 5 &&
				!bDidLevelMemDump && !_bMenuSubModeButtonStatus && _bPrevMenuSubModeButtonStatus)	//on up)
			{
	#if FHEAP_DETAILED_TRACKING
				char szDumpFileName[256];

				sprintf(szDumpFileName,
	#if FANG_PLATFORM_XB
					"d:\\dump_%s",
	#else
					"c:\\dump_%s",
	#endif
				Level_aInfo[Level_nLoadedIndex].pszWorldResName);
				fheap_ShowMemoryTracking(TRUE,//BOOL bShowModuleDetails,
									TRUE,//BOOL bShowByModule, 
									TRUE,//BOOL bShowTexAllocs,
									TRUE,//BOOL bDumpToFile,
									szDumpFileName ); 
	#endif
				bDidLevelMemDump = TRUE;
			}
		}
		else if (_nAIDebugMenuModes == AIMENUMODE_BOTLAYER)
		{
			CEntity* pEntity = CEntity::InWorldList_GetHead();
			while (pEntity)
			{
				if ((pEntity->TypeBits() & ENTITY_BIT_BOT))
				{
					DebugRenderCBot((CBot*) pEntity);
				}
				pEntity = pEntity->InWorldList_GetNext();
			}
		}
		else if (_nAIDebugMenuModes == AIMENUMODE_DOORS_LIFTS)
		{
			CDoorEntity* pDoor = CDoorEntity::m_pDoorListHead;
			while (pDoor)
			{
				if (pDoor->IsInWorld())
				{
					pDoor->DebugRender();
				}
				pDoor = pDoor->m_pDoorListNext;
			}
		}

		if (bDrawTestRays)
		{
			CFColorRGBA _rgb[3] = {FColor_MotifRed, FColor_MotifGreen, FColor_MotifBlue};
			for (s32 i = 0; i < _nNumRaysDebugTracking; i++)
			{
				_aoDrawGuidesVertsBuffer[ 0 ].ColorRGBA = _rgb[_aRayDestDebugData[i]];	//FColor_MotifWhite;//
				_aoDrawGuidesVertsBuffer[ 0 ].Pos_MS = _aRayOriginDebugTracking[i].v3;
				_aoDrawGuidesVertsBuffer[ 1 ].ColorRGBA = _rgb[_aRayDestDebugData[i]];
				_aoDrawGuidesVertsBuffer[ 1 ].Pos_MS = _aRayDestDebugTracking[i].v3;
				fdraw_Line(_aoDrawGuidesVertsBuffer, _aoDrawGuidesVertsBuffer+1);
			}
		}
		_nNumRaysDebugTracking = 0;

		//Draw Menu Mode
		f32 fLineHeight = 0.02f;
		f32 fCurLine = 0.0f;
		f32 fShadX = 0.001f;																									   
		f32 fShadY = 0.001f;																									   
		f32 fMenuX = 0.1f;																										   
		f32 fMenuY = 0.371f;																									   

		if (_szAIDebugMenuMode[0] != '\0')
		{

			fclib_strcpy(_szTextDisplayBuffer,"~w1~c00000099");
			fclib_strcat(_szTextDisplayBuffer, _szAIDebugMenuMode);
			ftext_DebugPrintf( fMenuX+fShadX, fMenuY+fShadY+(fCurLine*fLineHeight), _szTextDisplayBuffer);						   
			fclib_strcpy(_szTextDisplayBuffer,"~w1~c08995799");																	   
			fclib_strcat(_szTextDisplayBuffer, _szAIDebugMenuMode);																   
			ftext_DebugPrintf( fMenuX, fMenuY+(fCurLine*fLineHeight), _szTextDisplayBuffer);									   
		}
		fCurLine +=1.0f;

		//Draw Sub Mode
		if (_szAIDebugMenuSubMode[0] != '\0')
		{

			fclib_strcpy(_szTextDisplayBuffer,"~w1~c00000099");
			fclib_strcat(_szTextDisplayBuffer, _szAIDebugMenuSubMode);
			ftext_DebugPrintf( fMenuX+fShadX, fMenuY+fShadY+(fCurLine*fLineHeight), _szTextDisplayBuffer);
			fclib_strcpy(_szTextDisplayBuffer,"~w1~c99800099");
			fclib_strcat(_szTextDisplayBuffer, _szAIDebugMenuSubMode);
			ftext_DebugPrintf( fMenuX, fMenuY+(fCurLine*fLineHeight), _szTextDisplayBuffer);
		}


		frenderer_Pop();
	}
#endif // !FANG_PRODUCTION_BUILD
}



////
//
//	  Graph Utilities
//
//
//
//
