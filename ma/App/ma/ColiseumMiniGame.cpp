
#include "ColiseumMiniGame.h"
#include "fang.h"
#include "fworld.h"
#include "fresload.h"
#include "game.h"
#include "player.h"
#include "entity.h"
#include "meshentity.h"
#include "bot.h"
#include "botgrunt.h"
#include "botglitch.h"
#include "botminer.h"
#include "botcorrosive.h"
#include "door.h"
#include "gamesave.h"
#include "item.h"
#include "weapon.h"
#include "frenderer.h"
#include "fdebris.h"
#include "fscriptsystem.h"
#include "hud2.h"
#include "multiplayermgr.h"
#include "pausescreen.h"

#include "gamepad.h"

#include "fshadow.h"

#if FANG_PLATFORM_GC
#include "fgctex.h"
#endif

#include "fsound.h"

#include "./AI/AiCorrosive.h"
#include "./AI/AiBrainMan.h"
#include "./AI/AiBrain.h"
#include "./AI/AiApi.h"

//weapons here
#include "weapon_rivet.h"
#include "weapon_laser.h"
#include "weapon_rocket.h"
#include "weapon_spew.h"
#include "weapon_ripper.h"
//

#define COLISEUM_PRINT_DEBUG 0
#define COLISEUM_ENABLE_AUTOJEER 0

#define _USE_MAGNET_DEVICE 1
#define _WEAPON_PARTICLE_NAME "PEMCwpnpop1"
#define _DEBRIS_MESH_NAME_0 "GFDBmecha01"
#define _DEBRIS_MESH_NAME_1 "GFDBmecha02"
#define _DEBRIS_MESH_NAME_2 "GFDBmecha03"
#define _DEBRIS_MESH_NAME_3 "GFDBmecha04"
#define _HUT_EXPL_NAME	"BigWalkway"
#define _CELLMATE_NAME "blink_talk1"
#define _USE_SCRIPT_EVENT 1

#if FANG_PLATFORM_GC
	#define _TVScr_Res 128
#else
	#define _TVScr_Res 256
#endif

#define _BOO_SMALL_THRESHOLD 20.0f
#define _BOO_MED_THRESHOLD 40.0f
#define _BOO_LARGE_THRESHOLD 60.0f

#define SCORE_HIGHLIGHT_R 0.01f
#define SCORE_HIGHLIGHT_G 1.00f
#define SCORE_HIGHLIGHT_B 0.10f

#define SCORE_DROID_R 1.00f
#define SCORE_DROID_G 1.00f
#define SCORE_DROID_B 0.10f

#define SCORE_MIL_R 1.00f
#define SCORE_MIL_G 0.10f
#define SCORE_MIL_B 0.10f

#define EXCITED_MIN 30
#define ECSTATIC_MIN 80

#if FANG_PLATFORM_GC
	#define ROW_SIZE 2
	#define ROW_1 2
	#define ROW_2 4
#else
	#define ROW_SIZE 5
	#define ROW_1 5
	#define ROW_2 10
#endif

#define _IS_BOT_DEAD(pBot) (pBot->IsDead() || pBot->IsTorsoBlownOff())

CInventory CColiseumMiniGame::m_InvCopy;

#if FANG_WINGC
// Local co-op (PC): partners enter the arena unarmed too, each picks their own weapon in the
// selection room, and they get their own weapons back at the end (retail does this for player 1).
static CInventory _aPortPartnerInv[MAX_PLAYERS];

static void _PortStripPartners( void ) {
	for( s32 i = 1; i < CPlayer::m_nPlayerCount; ++i ) {
		CInventory *pInv = CPlayer::GetInventory( i );
		_aPortPartnerInv[i].CopyInv( pInv );
		pInv->SetToUnarmed( TRUE );
	}
}

static void _PortRestorePartners( void ) {
	for( s32 i = 1; i < CPlayer::m_nPlayerCount; ++i ) {
		CPlayer::PortSetSaveInventory( i, &_aPortPartnerInv[i] );
	}
}

// Player nPlayer holds a weapon from the selection room (more than the unarmed slot).
static BOOL _PortHasArenaWeapon( s32 nPlayer ) {
	return CPlayer::GetInventory( nPlayer )->m_auNumWeapons[0] > 1;
}

// The battle script's arena intro (a cutscene after the weapon room) walks the story Glitch onto the
// field and closes the tunnel door behind it, leaving partners outside. When that first cutscene
// ends, put the partners beside the story Glitch; later cutscenes leave everyone where they are.
static BOOL _bPortPartnersInArena = FALSE;
static BOOL _bPortWasCutscene = FALSE;
static s32 _nPortCorrosiveWinner = 0;
static u32 _aPortHutPlayers[CColiseumMiniGame::MAX_TRIPWIRES];

static CBotGlitch *_PortAliveArenaGlitch( s32 nPlayer ) {
	CEntity *pEntity = Player_aPlayer[nPlayer].m_pEntityOrig;
	if( !pEntity || !(pEntity->TypeBits() & ENTITY_BIT_BOTGLITCH) ||
		!pEntity->IsInWorld() || pEntity->IsMarkedForWorldRemove() || ((CBot *)pEntity)->IsDeadOrDying() ) return NULL;
	return (CBotGlitch *)pEntity;
}

static BOOL _PortAnyArenaPlayerAlive( void ) {
	for( s32 n = 0; n < CPlayer::m_nPlayerCount; ++n ) if( _PortAliveArenaGlitch(n) ) return TRUE;
	return FALSE;
}

static void _PortRetargetCorrosive( CBot *pBoss, CGroundCombat *pCombat ) {
	if( !pBoss || !pCombat ) return;
	CEntity *pEnemy = pCombat->m_pEnemy;
	if( pEnemy && pEnemy->IsInWorld() && !pEnemy->IsMarkedForWorldRemove() &&
		(!(pEnemy->TypeBits() & ENTITY_BIT_BOT) || !((CBot *)pEnemy)->IsDeadOrDying()) ) return;
	CBotGlitch *pClosest = NULL;
	f32 fClosest = 1.0e30f;
	for( s32 n = 0; n < CPlayer::m_nPlayerCount; ++n ) {
		CBotGlitch *pGlitch = _PortAliveArenaGlitch(n);
		if( pGlitch ) {
			CFVec3 vOffs = pGlitch->MtxToWorld()->m_vPos.v3 - pBoss->MtxToWorld()->m_vPos.v3;
			if( vOffs.Mag2() < fClosest ) { fClosest = vOffs.Mag2(); pClosest = pGlitch; }
		}
	}
	if( pClosest ) {
		pCombat->ChangeEnemy(pClosest);
		DEVPRINTF("Co-op: Corrosive retargets surviving player %d.\n", pClosest->m_nPossessionPlayerIndex + 1);
	}
}

static void _PortPartnersIntoArena( BOOL bWeaponsChosen ) {
	const BOOL bCutscene = CBot::m_bCutscenePlaying;
	if( bWeaponsChosen && !_bPortPartnersInArena && _bPortWasCutscene && !bCutscene && MultiplayerMgr.IsLocalCoop() ) {
		_bPortPartnersInArena = TRUE;
		DEVPRINTF( "Co-op: Coliseum arena intro over; partners join player %d on the field.\n", game_GetStoryPlayerIndex() + 1 );
		CPlayer::CoopPlaceStartingPartners( TRUE, game_GetStoryPlayerIndex() );
	}
	_bPortWasCutscene = bCutscene;
}
#endif

static cchar* _Coliseum_pszSoundEffectBank = "Level_26";
static cchar* _Coliseum_pszLevelBanks[] =
{
	"Level_26a",
	"Level_26b",
	"Level_26c",
	"Level_26d"
};

const u64 ENTITY_BITS_ENEMYBOTS = ( 
									ENTITY_BIT_BOTTITAN		 |
								    ENTITY_BIT_BOTGRUNT		 | 
								    ENTITY_BIT_BOTPROBE		 | 
									ENTITY_BIT_BOTPRED		 | 
								    ENTITY_BIT_BOTSWARMER	 |
								    ENTITY_BIT_BOTCHEMBOT	 |
								    ENTITY_BIT_BOTELITEGUARD |
								    ENTITY_BIT_KRUNK         |
								    ENTITY_BIT_BOTZOM        |
								    ENTITY_BIT_BOTJUMPER     |
								 	ENTITY_BIT_BOTMORTAR     |
								    ENTITY_BIT_BOTMOZER		 |
								    ENTITY_BIT_BOTSCOUT		 |
								    ENTITY_BIT_BOTCORROSIVE
								   );

const u64 ENTITY_BITS_FRIENDBOTS = ( 
  								     ENTITY_BIT_BOTGLITCH |
									 ENTITY_BIT_BOTMINER
								   );

const CBotBuilder::NPCWeapon_e _aNPCWeapon[] =
{
	CBotBuilder::_NPC_WEAPON_HAND,//COLLECTABLE_UNKNOWN = 0
	CBotBuilder::_NPC_WEAPON_HAND,//COLLECTABLE_WASHER,
	CBotBuilder::_NPC_WEAPON_HAND,//COLLECTABLE_CHIP,
	CBotBuilder::_NPC_WEAPON_HAND,//COLLECTABLE_PUP_HIJUMP,
	CBotBuilder::_NPC_WEAPON_HAND,//COLLECTABLE_PUP_SPEED,
	CBotBuilder::_NPC_WEAPON_HAND,//COLLECTABLE_PUP_ARMOR,
	CBotBuilder::_NPC_WEAPON_HAND,//COLLECTABLE_PUP_WEAPON,
	CBotBuilder::_NPC_WEAPON_HAND,//COLLECTABLE_PUP_ENERGY,
	CBotBuilder::_NPC_WEAPON_HAND,//COLLECTABLE_WEAPON_START,
	CBotBuilder::_NPC_WEAPON_LASER,//COLLECTABLE_WEAPON_LASER_L1,
	CBotBuilder::_NPC_WEAPON_LASER,//COLLECTABLE_WEAPON_LASER_L2,
	CBotBuilder::_NPC_WEAPON_LASER,//COLLECTABLE_WEAPON_LASER_L3,
	CBotBuilder::_NPC_WEAPON_RIVET,//COLLECTABLE_WEAPON_RIPPER_L1,
	CBotBuilder::_NPC_WEAPON_RIVET,//COLLECTABLE_WEAPON_RIPPER_L2,
	CBotBuilder::_NPC_WEAPON_RIVET,//COLLECTABLE_WEAPON_RIPPER_L3,
	CBotBuilder::_NPC_WEAPON_RIVET,//COLLECTABLE_WEAPON_RIVET_GUN_L1,
	CBotBuilder::_NPC_WEAPON_RIVET,//COLLECTABLE_WEAPON_RIVET_GUN_L2,
	CBotBuilder::_NPC_WEAPON_RIVET,//COLLECTABLE_WEAPON_RIVET_GUN_L3,
	CBotBuilder::_NPC_WEAPON_FLAMER,//COLLECTABLE_WEAPON_ROCKET_L1,
	CBotBuilder::_NPC_WEAPON_FLAMER,//COLLECTABLE_WEAPON_ROCKET_L2,
	CBotBuilder::_NPC_WEAPON_FLAMER,//COLLECTABLE_WEAPON_ROCKET_L3,
	CBotBuilder::_NPC_WEAPON_BLASTER,//COLLECTABLE_WEAPON_BLASTER_L1,
	CBotBuilder::_NPC_WEAPON_BLASTER,//COLLECTABLE_WEAPON_BLASTER_L2,
	CBotBuilder::_NPC_WEAPON_BLASTER,//COLLECTABLE_WEAPON_BLASTER_L3,
	CBotBuilder::_NPC_WEAPON_HAND,//COLLECTABLE_WEAPON_TETHER_L1,
	CBotBuilder::_NPC_WEAPON_HAND,//COLLECTABLE_WEAPON_TETHER_L2,
	CBotBuilder::_NPC_WEAPON_HAND,//COLLECTABLE_WEAPON_TETHER_L3,
	CBotBuilder::_NPC_WEAPON_SPEW,//COLLECTABLE_WEAPON_SPEW_L1,
	CBotBuilder::_NPC_WEAPON_SPEW,//COLLECTABLE_WEAPON_SPEW_L2,
	CBotBuilder::_NPC_WEAPON_SPEW,//COLLECTABLE_WEAPON_SPEW_L3,
	CBotBuilder::_NPC_WEAPON_HAND,//COLLECTABLE_WEAPON_SCOPE_L1,
	CBotBuilder::_NPC_WEAPON_HAND,//COLLECTABLE_WEAPON_SCOPE_L2,
	CBotBuilder::_NPC_WEAPON_GRENADE,//COLLECTABLE_WEAPON_MORTAR_L1,
	CBotBuilder::_NPC_WEAPON_FLAMER,//COLLECTABLE_WEAPON_FLAMER_L1,
	CBotBuilder::_NPC_WEAPON_GRENADE,//COLLECTABLE_WEAPON_CORING_CHARGE,
	CBotBuilder::_NPC_WEAPON_GRENADE,//COLLECTABLE_WEAPON_EMP,
	CBotBuilder::_NPC_WEAPON_GRENADE,//COLLECTABLE_WEAPON_MAGMA_BOMB,
	CBotBuilder::_NPC_WEAPON_GRENADE,//COLLECTABLE_WEAPON_CLEANER,
	CBotBuilder::_NPC_WEAPON_HAND,//COLLECTABLE_WEAPON_WRENCH,
};

static cchar* _pszSoundGroupTable_Common[] = 
{
	"L26_blade",	//  COLISEUMSND_BLADE
	"L26_magnet",	//  COLISEUMSND_MAGNET
	"L26_Boo_L01",
	"L26_Boo_L02",
	"L26_Boo_M01",
	"L26_Boo_M02",
	"L26_Boo_S01",
	"L26_Boo_S02",
	"L26_Che_L01",
	"L26_Che_L02",
	"L26_Che_L03",
	"L26_Che_M01",
	"L26_Che_M02",
	"L26_Che_M03",
	"L26_Che_S01",
	"L26_Che_S02",
	"L26_Che_S03",
	"L26_Oww_L01",
	"L26_Oww_L02",
	"L26_Oww_M01",
	"L26_Oww_M02",
	"L26_StingA1",
	"L26_StingA2",
	"L26_StingA3",
	"L26_StingB1",
	"L26_StingB2",
	"L26_StingB3",
	"L26_StingC1",
	"L26_StingC2",
	"L26_StingC3"
};

static cchar* _pszSoundGroupTable_L1[] = 
{
	"L261_Ann01",
	"L261_Ann02",
	"L261_Ann03",
	"L261_Ann04",
	"L261_Ann05",
	"L261_Ann06",
	"L261_Ann07",
	"L261_Cal_01",
	"L261_Cal_02",
	"L261_Cal_03",
	"L261_Cal_04",
	"L261_Cal_05",
	"L261_Cal_06",
	"L261_Cal_07",
	"L261_Cal_08",
	"L261_Cal_09",
	"L261_Cal_10",
	"L261_Jer_01",
	"L261_Jer_02",
	"L261_Jer_03",
	"L261_Jer_04",
	"L261_Jer_05",
	"L261_Jer_06",
	"L261_Crowd"
};

static cchar* _pszSoundGroupTable_L2[] = 
{
	"L262_Ann01",
	"L262_Ann02",
	"L262_Ann03",
	"L262_Ann04",
	"L262_Ann05",
	"L262_Ann06",
	"L262_Ann07",
	"L262_Cal_01",
	"L262_Cal_02",
	"L262_Cal_03",
	"L262_Cal_04",
	"L262_Cal_05",
	"L262_Cal_06",
	"L262_Cal_07",
	"L262_Cal_08",
	"L262_Cal_09",
	"L262_Cal_10",
	"L262_Jer_01",
	"L262_Jer_02",
	"L262_Jer_03",
	"L262_Jer_04",
	"L262_Jer_05",
	"L262_Jer_06",
	"L262_Crowd"
};

static cchar* _pszSoundGroupTable_L3[] = 
{
	"L263_Ann01",
	"L263_Ann02",
	"L263_Ann03",
	"L263_Ann04",
	"L263_Ann05",
	"L263_Ann06",
	"L263_Ann06",
	"L263_Cal_01",
	"L263_Cal_02",
	"L263_Cal_03",
	"L263_Cal_04",
	"L263_Cal_05",
	"L263_Cal_06",
	"L263_Cal_07",
	"L263_Cal_08",
	"L263_Cal_09",
	"L263_Cal_09",
	"L263_Jer_01",
	"L263_Jer_02",
	"L263_Jer_03",
	"L263_Jer_04",
	"L263_Jer_05",
	"L263_Jer_05",
	"L263_Crowd"
};

static cchar* _pszSoundGroupTable_L4[] = 
{
	"L264_Ann01",
	"L264_Ann02",
	"L264_Ann03",
	"L264_Ann04",
	"L264_Ann05",
	"L264_Ann06",
	"L264_Ann06",
	"L264_Cal_01",
	"L264_Cal_02",
	"L264_Cal_03",
	"L264_Cal_04",
	"L264_Cal_05",
	"L264_Cal_06",
	"L264_Cal_07",
	"L264_Cal_08",
	"L264_Cal_08",
	"L264_Cal_08",
	"L264_Jer_01",
	"L264_Jer_02",
	"L264_Jer_03",
	"L264_Jer_04",
	"L264_Jer_05",
	"L264_Jer_05",
	"L264_Crowd"
};

CColiseumMiniGame *CColiseumMiniGame::m_pColiseumMiniGame=NULL;
static BOOL _bMultiPlayer=FALSE;

const f32 CColiseumMiniGame::m_fCutSceneLength[4] = 
{
	5.0f,
	5.0f,
	5.0f,
	10.0f
};

void CColiseumMiniGame::CrowdCallback(void *pUserData)
{
	if (!m_pColiseumMiniGame) { return; }
	if (m_pColiseumMiniGame->m_nNumFakeCrowd < 1) { return; }
	if (m_pColiseumMiniGame->m_nNumFrontCrowd < 1) { return; }

	u32 nTemplateIdx = (u32)pUserData;
	nTemplateIdx = ( nTemplateIdx % m_pColiseumMiniGame->m_nNumFrontCrowd );
	
	//Make sure we have a viewport for the crowd.
	if (!m_pColiseumMiniGame->m_pSView)
	{
		m_pColiseumMiniGame->m_pSView = fviewport_Create();
	}

	fviewport_Clear(FVIEWPORT_CLEARFLAG_ALL, 0.0f, 0.0f, 0.0f, 1.0f, 0);

	if (m_pColiseumMiniGame->m_pFrontCrowd[nTemplateIdx])
	{
		//FOV for crowd template to fill render target.
		fviewport_InitPersp(m_pColiseumMiniGame->m_pSView, 0.524f, 0.7071f, 1.0f, 1000.0f, 0, 0, 64, 64);
		fviewport_SetActive(m_pColiseumMiniGame->m_pSView);

		CFVec3 vPos = m_pColiseumMiniGame->m_pFrontCrowd[nTemplateIdx]->GetBoundingSphere().m_Pos;
		CFVec3 vDir = m_pColiseumMiniGame->m_pFrontCrowd[nTemplateIdx]->m_Xfm.m_MtxF.m_vFront.v3;
		CFVec3 vUp(0, 1, 0);

		//position my camera 12' away from the crowd template.
		vDir.Unitize();
		vPos = vPos - (vDir*12.0f);

		//Fix uv orientation, make sure they keep it consistent!
		vUp = -m_pColiseumMiniGame->m_pFrontCrowd[nTemplateIdx]->m_Xfm.m_MtxF.m_vRight.v3;
		vUp.Unitize();
		
		//setup camera.
        m_pColiseumMiniGame->m_VXfm.BuildLookatFromDirVec(vPos, vDir, vUp);
		m_pColiseumMiniGame->m_VXfm.InitStackWithView();

		BOOL bFogEnable = fsh_Fog_IsEnabled();
		//allow alpha output (used for cutout when rendering with rendertarget), special fog constant settings.
		fsh_EnableAlphaOut(TRUE);
		//disable fog.
		fsh_Fog_Enable(FALSE, TRUE);
		//render crowd template mesh.
		frenderer_Push( FRENDERER_MESH );
		m_pColiseumMiniGame->m_pFrontCrowd[nTemplateIdx]->Draw( FVIEWPORT_PLANESMASK_NONE, TRUE );
		frenderer_Pop();
		//disable alpha output
		fsh_EnableAlphaOut(FALSE);
		//re-enable fog if it was previously enabled.
		fsh_Fog_Enable(bFogEnable, TRUE);
	}
}

void CColiseumMiniGame::TvScreenCallback(void *pUserData)
{
	if (m_pColiseumMiniGame)
	{
		CFVec3 vPos(-79.6f, 149.0f, 306.6f), vDir, vUp(1,0,0), vLookAt;
		f32 fZoom=150.0f;

		//Make sure we have a viewport for the crowd.
		if (!m_pColiseumMiniGame->m_pSView)
		{
			m_pColiseumMiniGame->m_pSView = fviewport_Create();
		}

		fviewport_InitPersp(m_pColiseumMiniGame->m_pSView, 0.5759587f, 1.0f, 1000.0f); //33 deg half angle.
		fviewport_SetActive(m_pColiseumMiniGame->m_pSView);

		fviewport_Clear(FVIEWPORT_CLEARFLAG_ALL, 0.0f, 0.0f, 0.0f, 1.0f, 0);

		if (m_pColiseumMiniGame->m_nCamIdx == 0)
		{
			//This is a distance view of the Coliseum.
			vPos = m_pColiseumMiniGame->m_vCamPos[0];
			vLookAt = m_pColiseumMiniGame->m_vCamTarget[0];

			if (vPos == vLookAt) { return; }

			vDir = m_pColiseumMiniGame->m_vCamTarget[0] - vPos;
			if (FMATH_FABS(vDir.x) > 0.01f || FMATH_FABS(vDir.y) > 0.01f || FMATH_FABS(vDir.z) > 0.01f)
			{
				vDir.Unitize();
			}
			else
			{
				vDir.Set(1, 0, 0);
			}
			vUp.Set(0, 1, 0);
		}
		else
		{
			//This is a bot view camera, it switches between bots.
			CBot *pBot;
			f32 fDist=2.5f;

			if (m_pColiseumMiniGame->m_nCurBot >= (m_pColiseumMiniGame->m_nNumFriendly+m_pColiseumMiniGame->m_nNumEnemies))
			{
				m_pColiseumMiniGame->m_nCurBot = 0;
			}

			if (m_pColiseumMiniGame->m_nCurBot < m_pColiseumMiniGame->m_nNumFriendly)
			{
				pBot = m_pColiseumMiniGame->m_pFriendly[m_pColiseumMiniGame->m_nCurBot];
			}
			else
			{
				pBot = m_pColiseumMiniGame->m_pEnemies[m_pColiseumMiniGame->m_nCurBot-m_pColiseumMiniGame->m_nNumFriendly];
			}

			if (!pBot) return;

			fDist *= pBot->GetMesh()->GetBoundingSphere().m_fRadius;

			vUp.Set(0, 1, 0);
			if (!pBot->GetMesh()) return;
			vDir = pBot->GetMesh()->GetBoundingSphere().m_Pos;
			vDir.Set(0.7071f, -0.7071f, -0.7071f);
			
			f32 fYOffs = 0.0f;

			if (m_pColiseumMiniGame->m_nLevelID == 2)
			{
				if ( pBot->GetMesh()->GetBoundingSphere().m_fRadius > 30.0f )
				{
					vDir.Set(0.0f, 0.0f, 1.0f);
					fDist *= 0.5f;
					
					fYOffs = 10.0f;
				}
			}
			
			vDir.Unitize();
			
			if (m_pColiseumMiniGame->m_bSwitched)
			{
				m_pColiseumMiniGame->m_bSwitched = FALSE;
				m_pColiseumMiniGame->m_vLastCamLookAt = pBot->GetMesh()->GetBoundingSphere().m_Pos;
			}

			vPos = m_pColiseumMiniGame->m_vLastCamLookAt;
			vPos = vPos - (vDir*fDist);
			vPos.y += fYOffs;
			
			m_pColiseumMiniGame->m_vLastCamLookAt = pBot->GetMesh()->GetBoundingSphere().m_Pos;
		}
		
		m_pColiseumMiniGame->m_VXfm.BuildLookatFromDirVec(vPos, vDir, vUp);
		m_pColiseumMiniGame->m_VXfm.InitStackWithView();

		//render the skybox.
		if (FVis_pSkyBox&&m_pColiseumMiniGame->m_nCamIdx == 0)
		{
			frenderer_Push( FRENDERER_MESH, NULL );
		 	FVis_pSkyBox->Draw( FVIEWPORT_PLANESMASK_ALL, TRUE );
			frenderer_Pop();
		}

		//render the scene.
		fvis_Draw( NULL, FALSE, 0, FALSE );
	}
}

void CColiseumMiniGame::CorrosiveDamageCallback(void)
{
	if (m_pColiseumMiniGame)
	{
		if (m_pColiseumMiniGame->m_fFireTime == 0.0f)
		{
			CCorrosiveCombat::FreeToFireRockets(TRUE);
			CCorrosiveCombat::SetRoarThisFrame(TRUE);

			m_pColiseumMiniGame->m_fFireTime = 21.0f;
		}
		else
		{
			m_pColiseumMiniGame->m_fFireTime = 21.0f;
		}
	}
}

void CColiseumMiniGame::CorrosiveRoarCallback(u32 nUserData)
{
	if (m_pColiseumMiniGame)
	{
		m_pColiseumMiniGame->PlayCheer(CHEER_MAX);

		if (nUserData == 0)
		{
			m_pColiseumMiniGame->PlayCheer(CHEER_LARGE);
		}
		else
		{
			m_pColiseumMiniGame->PlayCheer(CHEER_MED);
		}

		m_pColiseumMiniGame->m_fTimeSinceLastDamage = 0.0f;

		m_pColiseumMiniGame->m_nExcitedLevel += 10;
		FMATH_CLAMP(m_pColiseumMiniGame->m_nExcitedLevel, 0, 100);
	}
}

void CColiseumMiniGame::CorrosiveRangeNoRocketsCallback(u32 nUserData)
{
	if (m_pColiseumMiniGame)
	{
		CMeshEntity *pMeshEntity = (CMeshEntity *)m_pColiseumMiniGame->m_pHutInside;

		if (pMeshEntity)
		{
			if (m_pColiseumMiniGame->m_bDoPeer)
			{
				CFVec3A vHutLoc;
				vHutLoc.v3 = pMeshEntity->GetMesh()->GetBoundingSphere().m_Pos;

				CCorrosiveCombat::DoFingerFlick( pMeshEntity->GetMesh()->GetBoundingSphere(), m_pColiseumMiniGame->m_nHitIdx, (AttackCompleteCallback_t)HutDestroyedCallback );

				u32 nRnd = fmath_RandomRange(0, 5);
				m_pColiseumMiniGame->m_bDoPeer = TRUE;
			}
			else
			{
				CCorrosiveCombat::DoFingerFlick( pMeshEntity->GetMesh()->GetBoundingSphere(), m_pColiseumMiniGame->m_nHitIdx, (AttackCompleteCallback_t)HutDestroyedCallback );

				u32 nRnd = fmath_RandomRange(0, 5);
				m_pColiseumMiniGame->m_bDoPeer = TRUE;
			}
		}
	}
}

CColiseumMiniGame::CColiseumMiniGame()
{
	//Setup data that is used for every Coliseum level.
	m_pScoreBoard[0] = NULL;
	//m_pAudioStream = NULL;

	m_bDoPeer=TRUE;
	m_bDoFlickNextFrame=FALSE;
	
	m_hWeaponParticleDef = (FParticle_DefHandle_t)fresload_Load( FPARTICLE_RESTYPE, _WEAPON_PARTICLE_NAME );

	m_nMagnetDevices = 0;
	//Score data.
	m_fScored = 0.0f;
	m_fScoredMil = 0.0f;
	m_eScoreType = SCORE_NONE;
	m_nNumWeapons = 0;

	//Setup TV screen rendertargets.
	m_pSView=NULL;
	m_pTVScrTex = NULL;

	m_pCorrosiveThrone = NULL;
	
	if ( !_bMultiPlayer )
	{
		m_pTVScrTex = ftex_CreateRenderTarget(_TVScr_Res, _TVScr_Res, FTEX_RENDERTARGET_FMT_C24_A8_D24_S8, "TVScr", TRUE, 0, NULL, FTEX_2D, TRUE);
		if (m_pTVScrTex)
		{
			ftex_AddRenderTarget(m_pTVScrTex, TvScreenCallback, TRUE, 0, FALSE, FALSE, NULL, TRUE, FALSE);
		}
	}

	m_pTVScr[0] = m_pTVScr[1] = m_pTVScr[2] = m_pTVScr[3] = NULL;
	m_nCamIdx = 0;
	m_nNumSoundEmitters = 0;

	m_bPlayMagnetSnd = m_bPlayBladeSnd = FALSE;

	m_hExpGroup=FEXPLOSION_INVALID_HANDLE;

	//Setup crowd rendering.
	m_nNumFrontCrowd = 0;
	m_nNumFakeCrowd = 0;
	m_nNumSoundsPlaying = 0;

	u32 i;
	
	if ( !_bMultiPlayer )
	{
		char szTexName[32];
		for (i=0; i<MAX_CROWD_TEMPLATE; i++)
		{
			#if !FANG_PLATFORM_GC
				u32 uStagger = (i/3) + 1;
			#endif

			m_pCrowdTex[i] = m_pCrowdAlpha[i] = NULL;

			sprintf(szTexName, "FakeCrowd%d", i);
			#if FANG_PLATFORM_GC
				m_pCrowdTex[i] = ftex_CreateRenderTarget(64, 64, FTEX_RENDERTARGET_FMT_C16_D24_S8, szTexName, TRUE, 0, NULL, FTEX_2D, FALSE);
				sprintf(szTexName, "CrowdAlpha%d", i);
				m_pCrowdAlpha[i] = ftex_CreateRenderTarget(64, 64, FTEX_RENDERTARGET_FMT_I8_D24, szTexName, TRUE, 0, NULL, FTEX_2D, FALSE);
				
				//Set to combat overbright rendertargets on GC.
				fgctex_SetCopyMul(0.50f);
			#else
				m_pCrowdTex[i] = ftex_CreateRenderTarget(64, 64, FTEX_RENDERTARGET_FMT_C24_A8_D24_S8, szTexName, TRUE, 0, NULL, FTEX_2D, FALSE);
				m_pCrowdAlpha[i] = NULL;
			#endif
			if (m_pCrowdTex[i])
			{
				ftex_AddRenderTarget(m_pCrowdTex[i], CrowdCallback, TRUE, 30, FALSE, FALSE, (void *)i, TRUE, TRUE);
				#if !FANG_PLATFORM_GC
					ftex_StaggerRenderTarget(uStagger);
				#endif
			}
			if (m_pCrowdAlpha[i])
			{
				ftex_RenderTarget_SetAlphaTex(m_pCrowdTex[i], m_pCrowdAlpha[i]);
			}
		}

		m_pDebrisSndGroup = CFSoundGroup::RegisterGroup("DebMtlMed");
	}
}

BOOL CColiseumMiniGame::LoadLevel( u32 nLevel )
{
	_bMultiPlayer = FALSE;
	//This is called from every Coliseum level.
    m_pColiseumMiniGame = fnew CColiseumMiniGame;
    
	if (m_pColiseumMiniGame)
	{
		//Set initial state of State machine.
		m_pColiseumMiniGame->m_State = STATE_INIT;
		//Clear Data.
		m_pColiseumMiniGame->m_nCurBot = 0;
		m_pColiseumMiniGame->m_nNumAmmo = 0;
		m_pColiseumMiniGame->m_nNumEnemies = 0;
		m_pColiseumMiniGame->m_nNumFriendly = 0;
		m_pColiseumMiniGame->m_nNumFakeCrowd = 0;
		m_pColiseumMiniGame->m_nNumFrontCrowd = 0;
		m_pColiseumMiniGame->m_nNumHuts = 0;
		m_pColiseumMiniGame->m_nNumSoundEmitters = 0;
		m_pColiseumMiniGame->m_nNumTripwires = 0;

		m_pColiseumMiniGame->m_vCamPos[0].Set(0,0,0);
		m_pColiseumMiniGame->m_vCamPos[1].Set(0,0,0);
		m_pColiseumMiniGame->m_vCamTarget[0].Set(0,0,0);
		m_pColiseumMiniGame->m_vCamTarget[1].Set(0,0,0);

		fang_MemSet(m_pColiseumMiniGame->m_hSounds, 0, 4*COLISEUMSND_COUNT);
	}

	// Load the sound effects bank(s) for this level...
	if( !fresload_Load( FSNDFX_RESTYPE, _Coliseum_pszSoundEffectBank) )
	{
		DEVPRINTF( "CColiseumMiniGame::LoadLevel(): Could not load sound effect bank '%s'\n", _Coliseum_pszSoundEffectBank);
	}

	if( !fresload_Load( FSNDFX_RESTYPE, _Coliseum_pszLevelBanks[nLevel]) )
	{
		DEVPRINTF( "CColiseumMiniGame::LoadLevel(): Could not load sound effect bank '%s'\n", _Coliseum_pszLevelBanks[nLevel]);
	}

	//Setup common sounds.
	ColiseumSounds_e iSndIdx;
	for (iSndIdx = COLISEUMSND_COMMON_BLADE; iSndIdx < COLISEUMSND_COMMON_COUNT; iSndIdx = (ColiseumSounds_e)(iSndIdx+1))
	{
		m_pColiseumMiniGame->m_hSounds[iSndIdx] = fsndfx_GetFxHandle( _pszSoundGroupTable_Common[iSndIdx] );
		if (!m_pColiseumMiniGame->m_hSounds[iSndIdx])
			DEVPRINTF( "CColiseumMiniGame::LoadLevel(): Could not load sound effect '%s'\n", _pszSoundGroupTable_Common[iSndIdx]);
	}

	//setup level specific sounds.
	for (iSndIdx = COLISEUMSND_COMMON_COUNT; iSndIdx < COLISEUMSND_COUNT; iSndIdx = (ColiseumSounds_e)(iSndIdx+1))
	{
		if (nLevel == 0)
		{
			m_pColiseumMiniGame->m_hSounds[iSndIdx] = fsndfx_GetFxHandle( _pszSoundGroupTable_L1[iSndIdx-COLISEUMSND_COMMON_COUNT] );

			if (!m_pColiseumMiniGame->m_hSounds[iSndIdx])
				DEVPRINTF( "CColiseumMiniGame::LoadLevel(): Could not load sound effect '%s'\n", _pszSoundGroupTable_L1[iSndIdx-COLISEUMSND_COMMON_COUNT]);
		}
		else if (nLevel == 1)
		{
			m_pColiseumMiniGame->m_hSounds[iSndIdx] = fsndfx_GetFxHandle( _pszSoundGroupTable_L2[iSndIdx-COLISEUMSND_COMMON_COUNT] );

			if (!m_pColiseumMiniGame->m_hSounds[iSndIdx])
				DEVPRINTF( "CColiseumMiniGame::LoadLevel(): Could not load sound effect '%s'\n", _pszSoundGroupTable_L2[iSndIdx-COLISEUMSND_COMMON_COUNT]);
		}
		else if (nLevel == 2)
		{
			m_pColiseumMiniGame->m_hSounds[iSndIdx] = fsndfx_GetFxHandle( _pszSoundGroupTable_L3[iSndIdx-COLISEUMSND_COMMON_COUNT] );

			if (!m_pColiseumMiniGame->m_hSounds[iSndIdx])
				DEVPRINTF( "CColiseumMiniGame::LoadLevel(): Could not load sound effect '%s'\n", _pszSoundGroupTable_L3[iSndIdx-COLISEUMSND_COMMON_COUNT]);
		}
		else
		{
			m_pColiseumMiniGame->m_hSounds[iSndIdx] = fsndfx_GetFxHandle( _pszSoundGroupTable_L4[iSndIdx-COLISEUMSND_COMMON_COUNT] );

			if (!m_pColiseumMiniGame->m_hSounds[iSndIdx])
				DEVPRINTF( "CColiseumMiniGame::LoadLevel(): Could not load sound effect '%s'\n", _pszSoundGroupTable_L4[iSndIdx-COLISEUMSND_COMMON_COUNT]);
		}
	}

	fang_MemSet(m_pColiseumMiniGame->m_pSoundEmitters, 0, sizeof(m_pColiseumMiniGame->m_pSoundEmitters[0])*COLISEUMSND_COUNT);
	m_pColiseumMiniGame->m_nNumStoredWpn = 0;
	//

	CMagnetDevice::SetCollisionTest(CMagnetDevice::MCOL_LIST);

	return m_pColiseumMiniGame?TRUE:FALSE;
}

BOOL CColiseumMiniGame::LoadMP( LevelEvent_e eEvent )
{
	BOOL bRet = FALSE;

	_bMultiPlayer = TRUE;

	m_pColiseumMiniGame = fnew CColiseumMiniGame;
    
	if (m_pColiseumMiniGame)
	{
		bRet = TRUE;

		//Setup manget traps.
		#if _USE_MAGNET_DEVICE

		BOOL bFound;
		char szDeviceName[32];
		if ( !m_pColiseumMiniGame->m_nMagnetDevices )
		{
			do
			{
				bFound = FALSE;

				sprintf(szDeviceName, "spiketrap%d", m_pColiseumMiniGame->m_nMagnetDevices+1);
				CMeshEntity *pMagnetDevice = (CMeshEntity *)CEntity::FindInWorld(szDeviceName);
				if (pMagnetDevice)
				{
					m_pColiseumMiniGame->m_apMagnetDevice[ m_pColiseumMiniGame->m_nMagnetDevices ] = fnew CMagnetDevice;
					m_pColiseumMiniGame->m_apMagnetDevice[ m_pColiseumMiniGame->m_nMagnetDevices ]->SetMeshEntity( pMagnetDevice );
					m_pColiseumMiniGame->m_apMagnetDevice[ m_pColiseumMiniGame->m_nMagnetDevices ]->SetActivateCallback(m_pColiseumMiniGame, m_pColiseumMiniGame->MagnetDeviceCallback);
					m_pColiseumMiniGame->m_apMagnetDevice[ m_pColiseumMiniGame->m_nMagnetDevices ]->SetMagnetFXCallback(m_pColiseumMiniGame->MagnetFXCallback);

					m_pColiseumMiniGame->m_nMagnetDevices++;
					bFound = TRUE;
				}
			} while (bFound);
		}
		else
		{
			s32 i;
			for (i=0; i<m_pColiseumMiniGame->m_nMagnetDevices; i++)
			{
				m_pColiseumMiniGame->m_apMagnetDevice[i]->Reset();
			}
		}

		CMagnetDevice::SetCollisionTest(CMagnetDevice::MCOL_ALLBOTS);
		CMagnetDevice::ClearCollisionEntity();
        
		#endif

		//Clear Data.
		m_pColiseumMiniGame->m_nCurBot = 0;
		m_pColiseumMiniGame->m_nNumAmmo = 0;
		m_pColiseumMiniGame->m_nNumEnemies = 0;
		m_pColiseumMiniGame->m_nNumFriendly = 0;
		m_pColiseumMiniGame->m_nNumFakeCrowd = 0;
		m_pColiseumMiniGame->m_nNumFrontCrowd = 0;
		m_pColiseumMiniGame->m_nNumHuts = 0;
		m_pColiseumMiniGame->m_nNumSoundEmitters = 0;
		m_pColiseumMiniGame->m_nNumTripwires = 0;

		m_pColiseumMiniGame->m_vCamPos[0].Set(0,0,0);
		m_pColiseumMiniGame->m_vCamPos[1].Set(0,0,0);
		m_pColiseumMiniGame->m_vCamTarget[0].Set(0,0,0);
		m_pColiseumMiniGame->m_vCamTarget[1].Set(0,0,0);

		fang_MemSet(m_pColiseumMiniGame->m_hSounds, 0, 4*COLISEUMSND_COUNT);

		fang_MemSet(m_pColiseumMiniGame->m_pSoundEmitters, 0, sizeof(m_pColiseumMiniGame->m_pSoundEmitters[0])*COLISEUMSND_COUNT);
		m_pColiseumMiniGame->m_nNumStoredWpn = 0;

		// Load the sound effects bank(s) for this level...
		if( !fresload_Load( FSNDFX_RESTYPE, _Coliseum_pszSoundEffectBank) )
		{
			DEVPRINTF( "CColiseumMiniGame::LoadLevel(): Could not load sound effect bank '%s'\n", _Coliseum_pszSoundEffectBank);
		}

		//Setup common sounds.
		ColiseumSounds_e iSndIdx;
		for (iSndIdx = COLISEUMSND_COMMON_BLADE; iSndIdx < COLISEUMSND_COMMON_COUNT; iSndIdx = (ColiseumSounds_e)(iSndIdx+1))
		{
			m_pColiseumMiniGame->m_hSounds[iSndIdx] = fsndfx_GetFxHandle( _pszSoundGroupTable_Common[iSndIdx] );
			if (!m_pColiseumMiniGame->m_hSounds[iSndIdx])
				DEVPRINTF( "CColiseumMiniGame::LoadLevel(): Could not load sound effect '%s'\n", _pszSoundGroupTable_Common[iSndIdx]);
		}

		CBot::m_bColiseumMiniGame = TRUE;
	}

	return bRet;
}

void CColiseumMiniGame::UnloadMP( void )
{
	CBot::m_bColiseumMiniGame = FALSE;

	ColiseumSounds_e iSndIdx;
	for (iSndIdx = COLISEUMSND_COMMON_BLADE; iSndIdx < COLISEUMSND_COMMON_COUNT; iSndIdx = (ColiseumSounds_e)(iSndIdx+1))
	{
		if (m_pColiseumMiniGame->m_pSoundEmitters[iSndIdx])
		{
			m_pColiseumMiniGame->m_pSoundEmitters[iSndIdx]->Destroy();
			m_pColiseumMiniGame->m_pSoundEmitters[iSndIdx] = NULL;
		}
	}

	fdelete(m_pColiseumMiniGame);
	m_pColiseumMiniGame = NULL;
}

BOOL CColiseumMiniGame::LoadLevel1( LevelEvent_e eEvent )
{
	// Only load the data on LEVEL_EVENT_POST_WORLD_LOAD, this was the original behavior
	if (eEvent != LEVEL_EVENT_POST_WORLD_LOAD)
	{
		if (eEvent == LEVEL_EVENT_PRE_PLAYER_BOT_INIT)
		{
			CInventory *pInv = Player_aPlayer[0].GetInventory(0);
			m_InvCopy.CopyInv(pInv);
			pInv->SetToUnarmed(TRUE);
#if FANG_WINGC
			_PortStripPartners();
#endif
		}	
		return TRUE;
	}

	BOOL bRet;
	bRet = LoadLevel(0);

	if (bRet)
	{
		m_pColiseumMiniGame->m_nLevelID = 0;
	}

	m_pColiseumMiniGame->m_fMaxStreamVolume = 0.66f;

	return bRet;
}

BOOL CColiseumMiniGame::LoadLevel2( LevelEvent_e eEvent )
{
	// Only load the data on LEVEL_EVENT_POST_WORLD_LOAD, this was the original behavior
	if (eEvent != LEVEL_EVENT_POST_WORLD_LOAD)
	{
		if (eEvent == LEVEL_EVENT_PRE_PLAYER_BOT_INIT)
		{
			CInventory *pInv = Player_aPlayer[0].GetInventory(0);
			m_InvCopy.CopyInv(pInv);
			pInv->SetToUnarmed(TRUE);
#if FANG_WINGC
			_PortStripPartners();
#endif
		}	
		return TRUE;
	}

	BOOL bRet;
	bRet = LoadLevel(1);

	if (bRet)
	{
		m_pColiseumMiniGame->m_nLevelID = 1;

		s32 i;
		CEntity *pEntity;
		char szName[32];
		for (i=0; i<6; i++)
		{
			sprintf(szName, "weapon%d", i+1);
			pEntity = CEntity::FindInWorld(szName);
			if (pEntity)
			{
				CWeapon *pWeapon = m_pColiseumMiniGame->CreateWeapon( ((CCollectable*)pEntity)->GetCollectableType()->m_eType );
				if (pWeapon)
				{
					m_pColiseumMiniGame->m_anWpnIdx[m_pColiseumMiniGame->m_nNumWeapons] = i+1;
					m_pColiseumMiniGame->m_apWeapons[m_pColiseumMiniGame->m_nNumWeapons++] = pWeapon;
				}
			}
		}
		
	}

	m_pColiseumMiniGame->m_fMaxStreamVolume = 0.80f;

	return bRet;
}

BOOL CColiseumMiniGame::LoadLevel3( LevelEvent_e eEvent )
{
	// Only load the data on LEVEL_EVENT_POST_WORLD_LOAD, this was the original behavior
	if (eEvent != LEVEL_EVENT_POST_WORLD_LOAD)
	{
		if (eEvent == LEVEL_EVENT_PRE_PLAYER_BOT_INIT)
		{
			CInventory *pInv = Player_aPlayer[0].GetInventory(0);
			m_InvCopy.CopyInv(pInv);
			pInv->SetToUnarmed(TRUE);
#if FANG_WINGC
			_PortStripPartners();
#endif
		}	
		return TRUE;
	}

	BOOL bRet;
	bRet = LoadLevel(2);

	if (bRet)
	{
		m_pColiseumMiniGame->m_nLevelID = 2;
	}

	m_pColiseumMiniGame->m_fMaxStreamVolume = 1.0f;

	return bRet;
}

BOOL CColiseumMiniGame::LoadLevel4( LevelEvent_e eEvent )
{
	// Only load the data on LEVEL_EVENT_POST_WORLD_LOAD, this was the original behavior
	if (eEvent != LEVEL_EVENT_POST_WORLD_LOAD)
	{
		if (eEvent == LEVEL_EVENT_PRE_PLAYER_BOT_INIT)
		{
			CInventory *pInv = Player_aPlayer[0].GetInventory(0);
			m_InvCopy.CopyInv(pInv);
			pInv->SetToUnarmed(TRUE);
#if FANG_WINGC
			_PortStripPartners();
#endif
		}	
		return TRUE;
	}

	BOOL bRet;
	bRet = LoadLevel(3);

	if (bRet)
	{
		m_pColiseumMiniGame->m_nLevelID = 3;

		m_pColiseumMiniGame->RegisterForTripwireEvents();
	}

	m_pColiseumMiniGame->m_fMaxStreamVolume = 1.0f;

	return bRet;
}

void CColiseumMiniGame::StopAllSounds()
{
	ColiseumSounds_e iSndIdx, iSndEnd;

	if ( _bMultiPlayer )
	{
		iSndEnd = COLISEUMSND_COMMON_COUNT;
	}
	else
	{
		iSndEnd = COLISEUMSND_COUNT;
	}

	for (iSndIdx = (ColiseumSounds_e)(COLISEUMSND_COMMON_BLADE); iSndIdx < COLISEUMSND_COUNT; iSndIdx = (ColiseumSounds_e)(iSndIdx+1))
	{
		if (m_pColiseumMiniGame->m_pSoundEmitters[iSndIdx])
		{
			m_pColiseumMiniGame->m_pSoundEmitters[iSndIdx]->Stop(FALSE);
			if (m_nNumSoundsPlaying) { m_nNumSoundsPlaying--; }
		}
	}
}

void CColiseumMiniGame::SetSndPan(ColiseumSounds_e eSnd, f32 fPan)
{
	if (m_pSoundEmitters[eSnd])
	{
		m_pSoundEmitters[eSnd]->SetPan(fPan);
	}
}

void CColiseumMiniGame::CleanUpSounds()
{
	ColiseumSounds_e iSndIdx, iSndEnd;

	if ( _bMultiPlayer )
	{
		iSndEnd = COLISEUMSND_COMMON_COUNT;
	}
	else
	{
		iSndEnd = COLISEUMSND_COUNT;
	}

	for (iSndIdx = (ColiseumSounds_e)(COLISEUMSND_COMMON_BLADE); iSndIdx < COLISEUMSND_COUNT-1; iSndIdx = (ColiseumSounds_e)(iSndIdx+1))
	{
		if (m_pColiseumMiniGame->m_pSoundEmitters[iSndIdx])
		{
			if (m_pSoundEmitters[iSndIdx]->GetState() == FAUDIO_EMITTER_STATE_NONE || m_pSoundEmitters[iSndIdx]->GetState() == FAUDIO_EMITTER_STATE_STOPPED)
			{
				m_pSoundEmitters[iSndIdx]->Destroy();
				m_pSoundEmitters[iSndIdx]=NULL;
				
				if (m_nNumSoundsPlaying) { m_nNumSoundsPlaying--; }
			}
		}
	}
}

BOOL CColiseumMiniGame::PlaySnd2D(ColiseumSounds_e eSnd, BOOL bPlay, BOOL bOverride, f32 fVolumeMul)
{
	BOOL bRet = FALSE;
	if (eSnd >= COLISEUMSND_COUNT)
	{
		DEVPRINTF("PlaySnd2D() attempted sound past limit\n");
		return bRet;
	}
	if (!m_hSounds[eSnd])
	{
		DEVPRINTF("PlaySnd2D() attempted sound not loaded\n");
		return bRet;
	}

	if (bOverride)
	{
		if (m_pSoundEmitters[eSnd])
		{
			m_pSoundEmitters[eSnd]->Destroy();
			m_pSoundEmitters[eSnd]=NULL;
			
			if (m_nNumSoundsPlaying) { m_nNumSoundsPlaying--; }
		}
	}
	else
	{
		if (m_pSoundEmitters[eSnd])
		{
			//Destroy the sound if not playing or stopped.
			if (m_pSoundEmitters[eSnd]->GetState() == FAUDIO_EMITTER_STATE_NONE || m_pSoundEmitters[eSnd]->GetState() == FAUDIO_EMITTER_STATE_STOPPED)
			{
				m_pSoundEmitters[eSnd]->Destroy();
				m_pSoundEmitters[eSnd]=NULL;
				
				if (m_nNumSoundsPlaying) { m_nNumSoundsPlaying--; }
			}
		}
	}

	if (bPlay && !m_pSoundEmitters[eSnd] && m_nNumSoundsPlaying < MAX_SOUNDS_PLAYING)
	{
		m_pSoundEmitters[eSnd] = FSNDFX_ALLOCNPLAY2D(m_hSounds[eSnd], 1.0f, 1.0f, FAudio_EmitterDefaultPriorityLevel, 0.f, TRUE );
		if ( m_pSoundEmitters[eSnd] )
		{
			m_pSoundEmitters[eSnd]->SetVolume(fVolumeMul);
		}
		m_nNumSoundsPlaying++;
		bRet = TRUE;
	}
	else if (bPlay && m_pSoundEmitters[eSnd])
	{
		m_pSoundEmitters[eSnd]->SetVolume(fVolumeMul);
	}

	return bRet;
}

BOOL CColiseumMiniGame::PlaySnd3D(ColiseumSounds_e eSnd, CFVec3A *pPos_WS, f32 fOuterRadius, f32 fVolumeMul, f32 fFreqMul, BOOL bPlay, BOOL bOverride)
{
	BOOL bRet = FALSE;
	if (eSnd >= COLISEUMSND_COUNT)
	{
		DEVPRINTF("PlaySnd3D() attempted sound past limit\n");
		return bRet;
	}
	if (!m_hSounds[eSnd])
	{
		DEVPRINTF("PlaySnd3D() attempted sound not loaded\n");
		return bRet;
	}

	if (bOverride)
	{
		if (m_pSoundEmitters[eSnd])
		{
			m_pSoundEmitters[eSnd]->Destroy();
			m_pSoundEmitters[eSnd]=NULL;
			
			if (m_nNumSoundsPlaying) { m_nNumSoundsPlaying--; }
		}
	}
	else
	{
		if (m_pSoundEmitters[eSnd])
		{
			//Destroy the sound if not playing or stopped.
			if (m_pSoundEmitters[eSnd]->GetState() == FAUDIO_EMITTER_STATE_NONE || m_pSoundEmitters[eSnd]->GetState() == FAUDIO_EMITTER_STATE_STOPPED)
			{
				m_pSoundEmitters[eSnd]->Destroy();
				m_pSoundEmitters[eSnd]=NULL;
				
				if (m_nNumSoundsPlaying) { m_nNumSoundsPlaying--; }
			}
		}
	}

	if ( bPlay && !m_pSoundEmitters[eSnd] && pPos_WS && m_nNumSoundsPlaying < MAX_SOUNDS_PLAYING )
	{
		m_pSoundEmitters[eSnd] = FSNDFX_ALLOCNPLAY3D(m_hSounds[eSnd], pPos_WS, fOuterRadius, fVolumeMul, fFreqMul, FAudio_EmitterDefaultPriorityLevel, TRUE);
		if ( m_pSoundEmitters[eSnd] )
		{
			m_pSoundEmitters[eSnd]->SetVolume(fVolumeMul);
			
			m_nNumSoundsPlaying++;
		}
		bRet = TRUE;
	}
	else if ( bPlay && m_pSoundEmitters[eSnd] )
	{
		m_pSoundEmitters[eSnd]->SetVolume(fVolumeMul);
		m_pSoundEmitters[eSnd]->SetFrequencyFactor(fFreqMul);
	}
	return bRet;
}

BOOL CColiseumMiniGame::StopSnd(ColiseumSounds_e eSnd)
{
	BOOL bRet = FALSE;
	if (eSnd >= COLISEUMSND_COUNT)
	{
		DEVPRINTF("PlaySnd3D() attempted sound past limit\n");
		return bRet;
	}
	if (!m_hSounds[eSnd])
	{
		DEVPRINTF("PlaySnd3D() attempted sound not loaded\n");
		return bRet;
	}
	if (m_pSoundEmitters[eSnd])
	{
		m_pSoundEmitters[eSnd]->Stop(FALSE);
		bRet = TRUE;
	}
	return bRet;
}

void CColiseumMiniGame::UnloadLevel( void )
{
	CHud2::EnableAudioPause(TRUE);
	CBot::m_bColiseumMiniGame = FALSE;
	
	Player_aPlayer[0].m_bForceReady = FALSE;

	ColiseumSounds_e iSndIdx;
	for (iSndIdx = COLISEUMSND_COMMON_BLADE; iSndIdx < COLISEUMSND_COUNT; iSndIdx = (ColiseumSounds_e)(iSndIdx+1))
	{
		if (m_pColiseumMiniGame->m_pSoundEmitters[iSndIdx])
		{
			m_pColiseumMiniGame->m_pSoundEmitters[iSndIdx]->Destroy();
			m_pColiseumMiniGame->m_pSoundEmitters[iSndIdx] = NULL;
		}
	}

	s32 i;
	for (i=0; i<m_pColiseumMiniGame->m_nNumWeapons; i++)
	{
		if ( m_pColiseumMiniGame->m_apWeapons[i]->IsInWorld() )
		{
			m_pColiseumMiniGame->m_apWeapons[i]->Destroy();
		}

		if ( !m_pColiseumMiniGame->m_apWeapons[i]->IsAutoDelete() )
		{
			fdelete(m_pColiseumMiniGame->m_apWeapons[i]);
		}
		m_pColiseumMiniGame->m_apWeapons[i] = NULL;
	}

	for (i=0; i<m_pColiseumMiniGame->m_nNumStoredWpn; i++)
	{
		m_pColiseumMiniGame->m_apStoredWpn[i]->Destroy();
		m_pColiseumMiniGame->m_apStoredWpn[i] = NULL;
	}

	if ( m_pColiseumMiniGame->m_nMagnetDevices )
	{
		for (i=0; i<m_pColiseumMiniGame->m_nMagnetDevices; i++)
		{
			fdelete(m_pColiseumMiniGame->m_apMagnetDevice[i]);
			m_pColiseumMiniGame->m_apMagnetDevice[i] = NULL;
		}
		m_pColiseumMiniGame->m_nMagnetDevices = 0;
	}
	
	if (m_pColiseumMiniGame->m_nLevelID == 2)
	{
		CBotCorrosive::SetForce2DRoarRocket(FALSE);
	}

	fdelete(m_pColiseumMiniGame);
	m_pColiseumMiniGame = NULL;
}

void CColiseumMiniGame::Restore( void )
{
	CHud2::EnableAudioPause(TRUE);
	Player_aPlayer[0].m_bForceReady = FALSE;

	CBot::m_bColiseumMiniGame = FALSE;

	if (m_pColiseumMiniGame)
	{
		level_StopAllStreams();

		m_pColiseumMiniGame->m_State = STATE_INIT;

		m_pColiseumMiniGame->m_bDoPeer=TRUE;
		m_pColiseumMiniGame->m_bDoFlickNextFrame=FALSE;

		m_pColiseumMiniGame->StopAllSounds();
		m_pColiseumMiniGame->m_fStreamVolume = 0.2f;
		m_pColiseumMiniGame->m_nNumSoundsPlaying = 0;
		
		if (m_pColiseumMiniGame->m_nLevelID == 2)
		{
			CBotCorrosive::SetForce2DRoarRocket(FALSE);
		}
	}
}

//Called when a bot is caught by a trap.
void CColiseumMiniGame::MagnetDeviceCallback(void *pUserData, void *pBot, CFVec3& vPos)
{
	if ( _bMultiPlayer ) return;

	CColiseumMiniGame *pSelf = (CColiseumMiniGame *)pUserData;

	if (pSelf)
	{
		pSelf->m_fTimeSinceLastDamage = 0.0f;

		m_pColiseumMiniGame->m_nExcitedLevel += 20;
		FMATH_CLAMP(m_pColiseumMiniGame->m_nExcitedLevel, 0, 100);

		// Crowd reacts here, they really like it when someone gets caught in the machine.
		s32 nRndIdx = fmath_RandomRange(0, 4);
		switch (nRndIdx)
		{
			case 0:
				pSelf->PlaySnd2D(CColiseumMiniGame::COLISEUMSND_COMMON_OWW_L01, TRUE, FALSE);
			break;
			case 1: 
				pSelf->PlaySnd2D(CColiseumMiniGame::COLISEUMSND_COMMON_OWW_L02, TRUE, FALSE);
			break;
			case 2:
				pSelf->PlaySnd2D(CColiseumMiniGame::COLISEUMSND_COMMON_CHEER_L02, TRUE, FALSE);
			break;
            case 3:
				pSelf->PlaySnd2D(CColiseumMiniGame::COLISEUMSND_LEVEL_ANN01, TRUE, FALSE);
			break;
			case 4:
				pSelf->PlaySnd2D(CColiseumMiniGame::COLISEUMSND_COMMON_CHEER_L01, TRUE, FALSE);
			break;
		}
	}
}

//Called when trap fx should be played/rendered
void CColiseumMiniGame::MagnetFXCallback(void *pUserData, void *pSpd, CFVec3& vPos)
{
	f32 fSpd;
	CColiseumMiniGame *pSelf = (CColiseumMiniGame *)pUserData;

	if (pSelf)
	{
		fSpd = *((f32*)pSpd);

		FMATH_CLAMP(fSpd, 0.0f, 1.0f);
		
		pSelf->m_tmpVec3A.v3 = vPos;
		pSelf->PlaySnd3D(COLISEUMSND_COMMON_MAGNET, &pSelf->m_tmpVec3A, 500.0f, fSpd, 1.0f, TRUE, FALSE);
		pSelf->PlaySnd3D(COLISEUMSND_COMMON_BLADE, &pSelf->m_tmpVec3A, 200.0f, fSpd, 1.0f, TRUE, FALSE);

		pSelf->m_bPlayMagnetSnd = TRUE;
		pSelf->m_bPlayBladeSnd = TRUE;
	}
}

BOOL CColiseumMiniGame::Targeting( const CFVec3A *pRayStart_WS, const CFVec3A *pRayEnd_WS, CFVec3A *TargetedPointOnTarget_WS, CFWorldMesh **pTargetedTargetWorldMesh )
{
	s32 i;
	BOOL bRet = FALSE;

	if (m_pColiseumMiniGame)
	{
		for (i=0; i<m_pColiseumMiniGame->m_nMagnetDevices; i++)
		{
			if ( m_pColiseumMiniGame->m_apMagnetDevice[i]->RayCollidesWithTarget(pRayStart_WS, pRayEnd_WS, TargetedPointOnTarget_WS) )
			{
				*pTargetedTargetWorldMesh = m_pColiseumMiniGame->m_apMagnetDevice[i]->GetWorldMesh();
				bRet = TRUE;
				break;
			}
		}
	}

	return (bRet);
}

//Do game work
void CColiseumMiniGame::Work( void )
{
	if (MultiplayerMgr.IsSinglePlayer())
	{
		//Disable Glitch's radar when he's in the Coliseum levels.
		Player_aPlayer[0].m_Hud.EnableRadar( FALSE );
#if FANG_WINGC
		// local co-op: every player's, not only player 1's
		for( s32 nPlayer = 1; nPlayer < CPlayer::m_nPlayerCount; ++nPlayer ) {
			Player_aPlayer[nPlayer].m_Hud.EnableRadar( FALSE );
		}
#endif
	}

	if (m_pColiseumMiniGame)
	{
#if FANG_WINGC
		_PortPartnersIntoArena( m_pColiseumMiniGame->m_State >= STATE_ESCORT_OUT && m_pColiseumMiniGame->m_State <= STATE_MAIN_GAME );
#endif
		switch (m_pColiseumMiniGame->m_State)
		{
			case STATE_INIT:
				{
#if FANG_WINGC
					_bPortPartnersInArena = FALSE;
					_bPortWasCutscene = FALSE;
#endif
					m_pColiseumMiniGame->InitGame();
					m_pColiseumMiniGame->m_State = STATE_ESCORT;
					
					#if FANG_PLATFORM_GC
						//fshadow_ActivateShadows(TRUE);
					#endif
				}
				break;
			case STATE_ESCORT:
				if ( m_pColiseumMiniGame->Escort() )
				{
					m_pColiseumMiniGame->m_State = STATE_WEAPON_SELECT;
				}
				break;
			case STATE_WEAPON_SELECT:
				if ( m_pColiseumMiniGame->WeaponSelect() )
				{
					m_pColiseumMiniGame->m_State = STATE_ESCORT_OUT;
				}
				break;
			case STATE_ESCORT_OUT:
				if ( m_pColiseumMiniGame->EscortOut() )
				{
					m_pColiseumMiniGame->m_State = STATE_MAIN_GAME;
					
					CHud2::EnableAudioPause(FALSE);
					
					#if FANG_PLATFORM_GC
						//fshadow_ActivateShadows(FALSE);
					#endif
					
					CBot::m_bColiseumMiniGame = TRUE;
					
					if (m_pColiseumMiniGame->m_nLevelID == 2)
					{
						CBotCorrosive::SetForce2DRoarRocket(TRUE);
					}
				}
				break;
			case STATE_MAIN_GAME:
				if ( m_pColiseumMiniGame->MainGame() )
				{
					if (m_pColiseumMiniGame->m_nLevelID == 3)
					{
						m_pColiseumMiniGame->m_State = STATE_END_CUTSCENE;
						m_pColiseumMiniGame->m_fCutSceneTime = 0.0f;
					}
					else
					{
						m_pColiseumMiniGame->m_State = STATE_END_GAME;
					}
				}
				break;
			case STATE_END_CUTSCENE:
				if ( m_pColiseumMiniGame->HandleEndCutscene() )
				{
					m_pColiseumMiniGame->m_State = STATE_END_GAME;
				}
				break;
			case STATE_END_GAME:
				m_pColiseumMiniGame->EndGame();
				m_pColiseumMiniGame->m_State = STATE_WAIT;
				
				break;
			case STATE_WAIT:
				break;
			default:
				FASSERT_NOW;
				break;
		}
	}
}

//do multiplayer work
void CColiseumMiniGame::WorkMP( void )
{
	//Update devices - magnetic
	s32 i;
	if (!m_pColiseumMiniGame) return;
	if (m_pColiseumMiniGame->m_nMagnetDevices)
	{
		for (i=0; i<m_pColiseumMiniGame->m_nMagnetDevices; i++)
		{
			m_pColiseumMiniGame->m_apMagnetDevice[i]->Work();
		}
	}
	m_pColiseumMiniGame->CleanUpSounds();
}

void CColiseumMiniGame::InitGame()
{
	//disable unarmed player rule to keep AI buddies from putting their weapons away.
	ai_DisableUnarmedPlayerRule();
	
	m_fStreamVolume = 0.2f;

	m_bHitWhole = FALSE;
#if FANG_WINGC
	_nPortCorrosiveWinner = 0;
	fang_MemSet(_aPortHutPlayers, 0, sizeof(_aPortHutPlayers));
#endif
	m_nScore = 0;
	m_nMilScore = 0;
	m_fScored = 0.0f;
	m_fScoredMil = 0.0f;
	m_eScoreType = SCORE_NONE;
	m_nNumAmmo = 0;
	m_nCurBot = 0;
	m_fTimeSinceLastDamage = 0.0f;

	m_nLastCheerIdx=0;
	m_nLastBooIdx=0;
	m_nLastStingerIdx=0;
	m_nLastAnnouncerIdx=0;
	m_nLastCallIdx=0;
	m_nLastJeerIdx=0;
	m_nLastTexIdx=-1;
	m_nGarbageIdx = 0;
	m_fJeerTime=0;

	m_fFireTime = 0.0f;

	m_nCamIdx = 0; 

	m_bSwitched = FALSE;

	m_fSwitchTime = 0.0f;
	m_fBotSwitch = 10.0f;

	switch (m_nLevelID)
	{
		case 0:
			m_nExcitedLevel = 10;
		break;
		case 1:
			m_nExcitedLevel = 20;
		break;
		case 2:
			m_nExcitedLevel = 50;
		break;
		case 3:
			m_nExcitedLevel = 90;
		break;
		default:
			m_nExcitedLevel = 10;
		break;
	}

	m_fTimeUntilDec = 30.0f;

	if (m_nLevelID == 2)
	{
		CCorrosiveCombat::FreeToFireRockets(FALSE);
	}
	else if (m_nLevelID == 3)
	{
		SetupTripwires();
		CCorrosiveCombat::SetPreferredMeleeAttack(CCorrosiveCombat::PREFERED_MELEE_SWIPE);
		CCorrosiveCombat::SetAggressiveness(1.0f);
		//CBotGlitch::EnableCollisionWithPieces(TRUE);
		FASSERT( Player_aPlayer[0].m_pEntityCurrent->TypeBits() & ENTITY_BIT_BOT );
		((CBot*)Player_aPlayer[0].m_pEntityCurrent)->EnableCollisionWithPieces( TRUE );
	}

	//Initialize the number of kills.
	Player_aPlayer[0].OverrideUnitsDestroyed(m_nScore);

	//Setup Scoreboard.
	m_pScoreBoard[0] = m_pScoreBoard[1] = m_pScoreBoard[2] = m_pScoreBoard[3] = NULL;
	CFVec2 vZero(0,0);

	CMeshEntity *pScore = (CMeshEntity *)CEntity::FindInWorld("scoreboard");
	if (pScore)
	{
		m_pScoreBoard[0] = pScore->GetMeshInst();

		m_aTexLayerHandle[0] = m_pScoreBoard[0]->GetTexLayerHandle(1);
		m_aTexLayerHandle[1] = m_pScoreBoard[0]->GetTexLayerHandle(2);

		m_pScoreBoard[0]->AnimTC_SetScrollRate(m_aTexLayerHandle[0], vZero);
		m_pScoreBoard[0]->AnimTC_SetScrollRate(m_aTexLayerHandle[1], vZero);

		pScore = (CMeshEntity *)CEntity::FindInWorld_Next(pScore, "scoreboard");

		if (pScore)
		{
			m_pScoreBoard[1] = pScore->GetMeshInst();

			m_aTexLayerHandle[2] = m_pScoreBoard[1]->GetTexLayerHandle(1);
			m_aTexLayerHandle[3] = m_pScoreBoard[1]->GetTexLayerHandle(2);

			m_pScoreBoard[1]->AnimTC_SetScrollRate(m_aTexLayerHandle[2], vZero);
			m_pScoreBoard[1]->AnimTC_SetScrollRate(m_aTexLayerHandle[3], vZero);
		}
	}

	m_pScoreBoard[2] = m_pScoreBoard[3] = NULL;
	pScore = (CMeshEntity *)CEntity::FindInWorld("scoreboard2");
	if (pScore)
	{
		m_pScoreBoard[2] = pScore->GetMeshInst();

		m_aTexLayerHandle[4] = m_pScoreBoard[2]->GetTexLayerHandle(1);
		m_aTexLayerHandle[5] = m_pScoreBoard[2]->GetTexLayerHandle(2);

		m_pScoreBoard[2]->AnimTC_SetScrollRate(m_aTexLayerHandle[4], vZero);
		m_pScoreBoard[2]->AnimTC_SetScrollRate(m_aTexLayerHandle[5], vZero);

		pScore = (CMeshEntity *)CEntity::FindInWorld_Next(pScore, "scoreboard2");

		if (pScore)
		{
			m_pScoreBoard[3] = pScore->GetMeshInst();

			m_aTexLayerHandle[6] = m_pScoreBoard[3]->GetTexLayerHandle(1);
			m_aTexLayerHandle[7] = m_pScoreBoard[3]->GetTexLayerHandle(2);

			m_pScoreBoard[3]->AnimTC_SetScrollRate(m_aTexLayerHandle[6], vZero);
			m_pScoreBoard[3]->AnimTC_SetScrollRate(m_aTexLayerHandle[7], vZero);
		}
	}
	
	//Find Grunts that throw garbage.
	m_pGarbageGrunt[0] = (CBot *)CEntity::FindInWorld("garbage_grunt1");
	m_pGarbageGrunt[1] = (CBot *)CEntity::FindInWorld("garbage_grunt2");
	if (!m_pGarbageGrunt[1])
	{
		m_pGarbageGrunt[1] = (CBot *)CEntity::FindInWorld_Next(m_pGarbageGrunt[0], "garbage_grunt1");
	}
	m_pGarbageGrunt[2] = (CBot *)CEntity::FindInWorld("garbage_grunt3");
	m_pGarbageGrunt[3] = (CBot *)CEntity::FindInWorld("garbage_grunt4");
	if (!m_pGarbageGrunt[3])
	{
		m_pGarbageGrunt[3] = (CBot *)CEntity::FindInWorld_Next(m_pGarbageGrunt[2], "garbage_grunt3");
	}

	m_pGarbageMesh[0] = (FMesh_t *)fresload_Load( FMESH_RESTYPE, _DEBRIS_MESH_NAME_0 );
	m_pGarbageMesh[1] = (FMesh_t *)fresload_Load( FMESH_RESTYPE, _DEBRIS_MESH_NAME_1 );
	m_pGarbageMesh[2] = (FMesh_t *)fresload_Load( FMESH_RESTYPE, _DEBRIS_MESH_NAME_2 );
	m_pGarbageMesh[3] = (FMesh_t *)fresload_Load( FMESH_RESTYPE, _DEBRIS_MESH_NAME_3 );

	m_pGarbageMesh[0]->aMtl[0].MaterialTint.Set( 1.0f, 0.5f, 0.5f );
	m_pGarbageMesh[1]->aMtl[0].MaterialTint.Set( 0.5f, 1.0f, 0.5f );
	m_pGarbageMesh[2]->aMtl[0].MaterialTint.Set( 1.0f, 1.0f, 0.5f );
	m_pGarbageMesh[3]->aMtl[0].MaterialTint.Set( 1.0f, 0.5f, 1.0f );
		
	m_DebrisDef = CFDebrisDef::m_DefaultDebrisDef;
	m_DebrisDef.m_nFlags = CFDebrisDef::FLAG_ROTATIONAL_MOTION | CFDebrisDef::FLAG_RANDOM_ORIENTATION | CFDebrisDef::FLAG_PLAY_IMPACT_SOUND;
	m_DebrisDef.m_pMesh = m_pGarbageMesh[0];
	m_DebrisDef.m_fMeshScale = 1.5f;
	m_DebrisDef.m_fRotSpeed = 10.0f;
	m_DebrisDef.m_fUnitDustKickUp = 0.5f;
	m_DebrisDef.m_fGravity *= 2.0f;
	m_DebrisDef.m_nCollType = CFDebrisDef::COLL_TYPE_RAY;
	m_DebrisDef.m_fUnitHitpointDamage = 2.0f;
	m_DebrisDef.m_fAliveSecs = 3.0f;
	m_DebrisDef.m_pSoundGroupImpact = m_pDebrisSndGroup;
			
	//Setup TV screens
	CMeshEntity *pTVScr = (CMeshEntity *)CEntity::FindInWorld("TV01");
	if (pTVScr)
	{
		m_pTVScr[0] = pTVScr->GetMeshInst();
		m_aTVLayerHandle[0] = m_pTVScr[0]->GetTexLayerHandle(0);

		if (m_pTVScrTex)
		{
			m_pTVScr[0]->TexFlip_SetSingleTexture(m_aTVLayerHandle[0], m_pTVScrTex);
		}

		pTVScr = (CMeshEntity *)CEntity::FindInWorld_Next(pTVScr, "TV01");
		if (pTVScr)
		{
			m_pTVScr[3] = pTVScr->GetMeshInst();
			m_aTVLayerHandle[3] = m_pTVScr[3]->GetTexLayerHandle(0);

			if (m_pTVScrTex)
			{
				m_pTVScr[3]->TexFlip_SetSingleTexture(m_aTVLayerHandle[3], m_pTVScrTex);
			}
		}
	}

	pTVScr = (CMeshEntity *)CEntity::FindInWorld("TV02");
	if (pTVScr)
	{
		m_pTVScr[1] = pTVScr->GetMeshInst();
		m_aTVLayerHandle[1] = m_pTVScr[1]->GetTexLayerHandle(0);

		if (m_pTVScrTex)
		{
			m_pTVScr[1]->TexFlip_SetSingleTexture(m_aTVLayerHandle[1], m_pTVScrTex);
		}
	}

	pTVScr = (CMeshEntity *)CEntity::FindInWorld("TV03");
	if (pTVScr)
	{
		m_pTVScr[2] = pTVScr->GetMeshInst();
		m_aTVLayerHandle[2] = m_pTVScr[2]->GetTexLayerHandle(0);

		if (m_pTVScrTex)
		{
			m_pTVScr[2]->TexFlip_SetSingleTexture(m_aTVLayerHandle[2], m_pTVScrTex);
		}
	}

	if (m_nLevelID == 3)
	{
		m_pColiseumMiniGame->m_pCorrosiveThrone = (CDoorEntity *)CEntity::FindInWorld("Corrosive_chair1");
	}

	//Find all enemies & friendlies
	fang_MemSet(m_pEnemies, 0, MAX_ENEMIES*4);
	u32 i, j;
	u32 nNumEntity = CEntity::GetInWorldCount();
	m_nNumEnemies = 0;
	m_nNumFriendly = 0;
	m_nNumFrontCrowd = 0;
	m_nNumFakeCrowd = 0;
	u32 nBored=0, nExcited=0, nEcstatic=0;
	if (nNumEntity)
	{
		CEntity *pEntity = CEntity::InWorldList_GetHead();
		for (i=0; i<nNumEntity; i++)
		{
			if (pEntity)
			{
				if (pEntity->LeafTypeBit()&ENTITY_BITS_ENEMYBOTS)
				{
					if ( ((CBot *)pEntity)->AIBrain()->GetRace() == AIRACE_MIL || ((CBot *)pEntity)->AIBrain()->GetRace() == AIRACE_ZOMBIE )
					{
						m_pEnemies[m_nNumEnemies++] = (CBot *)pEntity;
					}
				}
				else if (pEntity->LeafTypeBit()&ENTITY_BITS_FRIENDBOTS)
				{
					if ( pEntity == Player_aPlayer[0].m_pEntityOrig )
					{
						if (i != 0)
						{
							m_pFriendly[m_nNumFriendly] = m_pFriendly[0];
						}
						m_pFriendly[0] = (CBot *)pEntity;
						m_nNumFriendly++;
					}
#if FANG_WINGC
					// Local co-op: partner Glitches are players, not the droid buddies this list holds.
					// Counted as buddies, they got buddy AI settings and, in Coliseum 2, CBotMiner-only
					// weapon/armor writes that corrupted the Glitch (crash when it is freed at level end).
					else if ( pEntity->TypeBits() & ENTITY_BIT_BOTGLITCH )
					{
					}
#endif
					else if ( pEntity != Player_aPlayer[0].m_pEntityOrig )
					{
						m_pFriendly[m_nNumFriendly++] = (CBot *)pEntity;

						if (((CBot *)pEntity)->AIBrain())
						{
							((CBot *)pEntity)->AIBrain()->SetFlag_Buddy_Ctrl_NoWeaponStow();
						}
						ai_ChangeBuddyCtrl( ((CBot *)pEntity)->AIBrain(), AI_BUDDY_CTRL_NEVER );
					}
				}
				else
				{
					if ( pEntity->Name() )
					{
						if ( fclib_stricmp(pEntity->Name(), "bored_crowd")==0 )
						{
							if (nBored < ROW_SIZE)
							{
								m_pFrontCrowd[nBored++] = ((CMeshEntity *)pEntity)->GetMeshInst();
								m_nNumFrontCrowd++;
							}
							else
							{
								((CMeshEntity *)pEntity)->RemoveFromWorld();
							}
						}
						else if ( fclib_stricmp(pEntity->Name(), "excited_crowd")==0 )
						{
							if (nExcited < ROW_SIZE)
							{
								m_pFrontCrowd[nExcited + ROW_1] = ((CMeshEntity *)pEntity)->GetMeshInst();
								nExcited++;
								m_nNumFrontCrowd++;
							}
							else
							{
								((CMeshEntity *)pEntity)->RemoveFromWorld();
							}
						}
						else if ( fclib_stricmp(pEntity->Name(), "ecstatic_crowd")==0 )
						{
							if (nEcstatic < ROW_SIZE)
							{
								m_pFrontCrowd[nEcstatic + ROW_2] = ((CMeshEntity *)pEntity)->GetMeshInst();
								nEcstatic++;
								m_nNumFrontCrowd++;
							}
							else
							{
								((CMeshEntity *)pEntity)->RemoveFromWorld();
							}
						}
						else if ( fclib_stricmp(pEntity->Name(), "fake_crowd")==0 )
						{
							m_pFakeCrowd[m_nNumFakeCrowd] = ((CMeshEntity *)pEntity)->GetMeshInst();

							for (j=0; j<5; j++)
							{
								s32 nTexLayer = m_pFakeCrowd[m_nNumFakeCrowd]->GetTexLayerHandle(j);
								if (m_pCrowdTex && nTexLayer > -1)
								{
									s32 nTexIdx;

									do
									{
										nTexIdx = fmath_RandomRange(0, ROW_SIZE-1);
									} while (nTexIdx == m_nLastTexIdx);
									m_nLastTexIdx = nTexIdx;

									m_pFakeCrowd[m_nNumFakeCrowd]->TexFlip_SetSingleTexture(nTexLayer, m_pCrowdTex[nTexIdx]);

									m_auCrowdIdx[m_nNumFakeCrowd*5+j] = nTexIdx;
									m_anCrowdOffs[m_nNumFakeCrowd*5+j] = fmath_RandomRange(-20, +20);
								}
							}
							#if FANG_PLATFORM_GC
							m_pFakeCrowd[m_nNumFakeCrowd]->m_nFlags |= FMESHINST_FLAG_ENABLECOLORKEY;
							#endif
							m_nNumFakeCrowd++;
						}
					}
				}
			}
			pEntity = pEntity->InWorldList_GetNext();
		}

		if (m_nNumFriendly > 1 && m_nLevelID != 1)
		{
			m_nNumFriendly = 1;
		}

	}

	//Setup manget traps.
#if _USE_MAGNET_DEVICE
	BOOL bFound;
	char szDeviceName[32];
	if ( !m_nMagnetDevices )
	{
		do
		{
			bFound = FALSE;

			sprintf(szDeviceName, "spiketrap%d", m_nMagnetDevices+1);
			CMeshEntity *pMagnetDevice = (CMeshEntity *)CEntity::FindInWorld(szDeviceName);
			if (pMagnetDevice)
			{
				m_apMagnetDevice[ m_nMagnetDevices ] = fnew CMagnetDevice;
				m_apMagnetDevice[ m_nMagnetDevices ]->SetMeshEntity( pMagnetDevice );
				m_apMagnetDevice[ m_nMagnetDevices ]->SetActivateCallback(this, MagnetDeviceCallback);
				m_apMagnetDevice[ m_nMagnetDevices ]->SetMagnetFXCallback(MagnetFXCallback);

				m_nMagnetDevices++;
				bFound = TRUE;
			}
		} while (bFound);
	}
	else
	{
		s32 i;
		for (i=0; i<m_nMagnetDevices; i++)
		{
			m_apMagnetDevice[i]->Reset();
		}
	}

	CMagnetDevice::ClearCollisionEntity();
	if (m_nNumEnemies > 0 || m_nNumFriendly > 0)
	{
		s32 i;
		for (i=0; i<m_nNumFriendly; i++)
		{
			if ( m_pFriendly[i] )
			{
				CMagnetDevice::AddCollisionEntity(m_pFriendly[i]);
			}
		}

		for (i=0; i<m_nNumEnemies; i++)
		{
			if ( m_pEnemies[i] )
			{
				CMagnetDevice::AddCollisionEntity(m_pEnemies[i]);
			}
		}
	}
#endif

	//Setup Ammo
	m_nNumAmmo = 0;
	char szAmmoName[16];
	for (i=0; i<MAX_AMMO; i++)
	{
		sprintf(szAmmoName, "ammo%d", i+1);
		CEntity *pAmmo = CEntity::FindInWorld(szAmmoName);
		if (pAmmo)
		{
			m_pAmmoPlacement[ m_nNumAmmo++ ] = pAmmo;
		}
	}

	//Setup TV/Camera Settings
	m_fOffsX=0.0f;
	m_fMaxOffsX=100.0f;
	m_fMinOffsX=-100.0f;
	m_fDeltaX=10.0f;

	m_fCamScale=0.0f;
	m_fDeltaScale=15.0f;
	m_fMaxScale=250.0f;

	if (m_nLevelID == 2)
	{
		CBotCorrosive::SetInflictDamageCallback(CorrosiveDamageCallback);
	}
	else if (m_nLevelID == 3)
	{
		CCorrosiveCombat::SetRoarCallback(CorrosiveRoarCallback);
		CCorrosiveCombat::SetRangeNoRocketsCallback(CorrosiveRangeNoRocketsCallback);
		CCorrosiveCombat::UseAlternateAggressionCalculations( TRUE );
	}
}

BOOL CColiseumMiniGame::Escort()
{
	BOOL bRet = FALSE;
	CFVec3 vWeaponSelect(34.0f, 0.0f, -105.0f);
	f32 fRoomRad2=20.0f*20.0f;

	//Poll current position, am I in weapon select area?
	CFWorldMesh *pMesh = Player_aPlayer[0].m_pEntityOrig->GetMesh();
	CFVec3 vOffs = pMesh->GetBoundingSphere().m_Pos - vWeaponSelect;
	vOffs.y = 0.0f;

	if ( vOffs.Mag2() <= fRoomRad2 )
	{
		bRet = TRUE;
	}
	else
	{
		CInventory *pInv = Player_aPlayer[0].GetInventory(0);
		if (pInv->m_auNumWeapons[0] > 1)
		{
			bRet = TRUE;
		}
	}
	//

	return bRet;
}

CWeapon *CColiseumMiniGame::CreateWeapon( CollectableType_e eType )
{
	CWeapon *pWeapon=NULL;
	switch (eType)
	{
		case COLLECTABLE_WEAPON_RIPPER_L1:
		case COLLECTABLE_WEAPON_RIPPER_L2:
		case COLLECTABLE_WEAPON_RIPPER_L3:
			{
				pWeapon = fnew CWeaponRipper();

				if( pWeapon == NULL )
				{
					return NULL;
				}

				if ( !((CWeaponRipper*)pWeapon)->Create() )
				{
					return NULL;
				}

				pWeapon->EnableAutoWork(FALSE);
			}
			break;
		case COLLECTABLE_WEAPON_RIVET_GUN_L1:
		case COLLECTABLE_WEAPON_RIVET_GUN_L2:
		case COLLECTABLE_WEAPON_RIVET_GUN_L3:
			{
				pWeapon = fnew CWeaponRivet();
				if( pWeapon == NULL )
				{
					return NULL;
				}

				if( !((CWeaponRivet *)(pWeapon))->Create() )
				{
					return NULL;
				}
				
				pWeapon->EnableAutoWork(FALSE);
			}
			break;
		case COLLECTABLE_WEAPON_ROCKET_L1:
		case COLLECTABLE_WEAPON_ROCKET_L2:
		case COLLECTABLE_WEAPON_ROCKET_L3:
			{
				pWeapon = fnew CWeaponRocket();
				if( pWeapon == NULL )
				{
					return NULL;
				}

				if( !((CWeaponRocket *)(pWeapon))->Create() )
				{
					return NULL;
				}

				pWeapon->EnableAutoWork(FALSE);
			}
			break;
		case COLLECTABLE_WEAPON_SPEW_L1:
		case COLLECTABLE_WEAPON_SPEW_L2:
		case COLLECTABLE_WEAPON_SPEW_L3:
			{
				pWeapon = fnew CWeaponSpew();
				if( pWeapon == NULL )
				{
					return NULL;
				}

				if( !((CWeaponSpew *)(pWeapon))->Create() )
				{
					return NULL;
				}

				pWeapon->EnableAutoWork(FALSE);
			}
			break;
		default:
			pWeapon = NULL;
			break;
	};

	return (pWeapon);
}

BOOL CColiseumMiniGame::WeaponSelect()
{
	BOOL bRet = FALSE;
	CFVec3 vPartOrig;
	CWeapon *pCurWpn=NULL;

	if (m_nLevelID == 3) { bRet = TRUE; }

	//Have I picked up a weapon yet?
	CInventory *pInv = Player_aPlayer[0].GetInventory(0);
#if FANG_WINGC
	// Local co-op: every standing player picks their own weapon (each pedestal once); the room
	// stays open until they all have.
	BOOL bEveryonePicked = (pInv->m_auNumWeapons[0] > 1);
	for( s32 nPartner = 1; nPartner < CPlayer::m_nPlayerCount; ++nPartner ) {
		CEntity *pPartner = Player_aPlayer[nPartner].m_pEntityOrig;
		if( pPartner && (pPartner->TypeBits() & ENTITY_BIT_BOT) && !((CBot *)pPartner)->IsDeadOrDying() &&
			!_PortHasArenaWeapon( nPartner ) ) {
			bEveryonePicked = FALSE;
		}
	}
	CWeapon *apPlayerWpn[MAX_PLAYERS] = { NULL };
	if (bEveryonePicked)
#else
	if (pInv->m_auNumWeapons[0] > 1)
#endif
	{
		bRet = TRUE;

		//Max out the ammo for the weapon picked up.
		u32 i;
#if FANG_WINGC
		for( s32 nPlayer = 0; nPlayer < CPlayer::m_nPlayerCount; ++nPlayer ) {
		pInv = CPlayer::GetInventory( nPlayer );
		if( !Player_aPlayer[nPlayer].m_pEntityOrig || !(Player_aPlayer[nPlayer].m_pEntityOrig->TypeBits() & ENTITY_BIT_BOTGLITCH) ) {
			continue;
		}
		CBotGlitch *pPlayer = (CBotGlitch *)Player_aPlayer[nPlayer].m_pEntityOrig;
#else
		CBotGlitch *pPlayer = (CBotGlitch *)Player_aPlayer[0].m_pEntityOrig;
#endif
		for (i=0; i<ItemInst_uMaxInventoryWeapons; i++)
		{
			if (pInv->m_aoWeapons[0][i].m_pItemData && pInv->m_aoWeapons[0][i].m_nReserveAmmo >= 0)
			{
				pPlayer->m_WeaponInv[0].m_apWeapon[i]->AddToReserve( 1000 );
				pPlayer->m_WeaponInv[0].m_apWeapon[i]->AddToClip( 1000 );
				pInv->m_aoWeapons[0][i].NotifyAmmoMightHaveChanged( pPlayer->m_WeaponInv[0].m_apWeapon[i]->GetClipAmmo(), pPlayer->m_WeaponInv[0].m_apWeapon[i]->GetReserveAmmo() );

				pCurWpn = pPlayer->m_WeaponInv[0].m_apWeapon[i];
			}
		}
#if FANG_WINGC
		apPlayerWpn[nPlayer] = pCurWpn;
		pCurWpn = NULL;
		}
		pCurWpn = apPlayerWpn[0];
#endif

		char szName[32];
		CFMtx43A *pMtx;
		CEntity *pEntity=NULL;

		if (m_nLevelID == 1)
		{
			u32 nRand;
			u32 nWeaponIdx;
			BOOL bWpnFound=FALSE;

			if ( m_nNumFriendly > 1 )
			{
				for (i=1; i<(u32)m_nNumFriendly; i++)
				{
					bWpnFound=FALSE;
#if FANG_WINGC
					// Co-op players take pedestals too: give up (the droid keeps its own gun) rather
					// than loop forever once none is left.
					u32 nTries = 0;
#endif
					do
					{
						nRand = fmath_RandomInt32();
						//make this bot select a random weapon
						nWeaponIdx = nRand%m_nNumWeapons;//(nRand%6)+1;

						sprintf(szName, "weapon%d", m_anWpnIdx[nWeaponIdx]);//nWeaponIdx);
						pEntity = CEntity::FindInWorld(szName);
						if (pEntity)
						{
							bWpnFound = TRUE;
						}
#if FANG_WINGC
						if( ++nTries >= 256 ) {
							break;
						}
#endif
					} while (bWpnFound == FALSE);

					if (bWpnFound && pEntity)
					{
						//give the bot the weapon
						CWeapon *pWeapon = m_apWeapons[nWeaponIdx];
						
						if (pWeapon)
						{
							if ( !pWeapon->IsInWorld() )
							{
								pWeapon->AddToWorld();
							}
							//walk the bot to the weapon
							ai_AssignGoal_Goto( m_pFriendly[i]->AIBrain(), pEntity->MtxToWorld()->m_vPos, 6, 100, GOTOFLAG_LOOKAT );//| GOTOFLAG_BLIND_OK );

							m_apStoredWpn[m_nNumStoredWpn++] = ((CBotMiner*)m_pFriendly[i])->SetStoredWeapon( pWeapon );
							pWeapon->SetOwner( m_pFriendly[i] );
							
							((CBotMiner*)m_pFriendly[i])->SwitchPrimaryWeapon(FALSE);
							ai_WeaponChanged(m_pFriendly[i]->AIBrain(), _aNPCWeapon[ ((CCollectable*)pEntity)->GetCollectableType()->m_eType ]);

							pWeapon->OverrideMaxClipAmmo(32767);
							pWeapon->SetDesiredState(CWeapon::STATE_DEPLOYED);

							//Buff up the buddy.
							((CBotMiner*)m_pFriendly[i])->SetArmorModifier(+0.50f);

							//Remove the weapon from the world
							pEntity->RemoveFromWorld(TRUE);
						}
					}
				}
			}
		}

		//Remove the other weapons from the world, only one is selectable.
		for (i=1; i<6; i++)
		{
			sprintf(szName, "weapon%d", i);
			pEntity = CEntity::FindInWorld(szName);
			if (pEntity)
			{
				pMtx = pEntity->MtxToWorld();
				if (pMtx)
				{
					vPartOrig.Set(pMtx->m_vPos.x, pMtx->m_vPos.y, pMtx->m_vPos.z);
				}
				//Play one-shot particle effect at weapon location.
				if ( m_hWeaponParticleDef != FPARTICLE_INVALID_HANDLE && pMtx )
				{
					fparticle_SpawnEmitter( m_hWeaponParticleDef, vPartOrig,  &CFVec3::m_UnitAxisY, 1.0f ); 
				}
				//Remove the weapon from the world
				pEntity->RemoveFromWorld(TRUE);
			}
		}

		//open door here.
		CDoorEntity *pDoor = (CDoorEntity *)CEntity::FindInWorld("door_lower");
		if (pDoor)
		{
			pDoor->SetLockState(FALSE);
			pDoor->GotoPos(1, CDoorEntity::GOTOREASON_DESTINATION);
		}

		//replace "ammo" entities with the correct weapon.
#if FANG_WINGC
		// Co-op: the ammo spots take turns between the players' chosen weapons.
		CWeapon *apAmmoWpn[MAX_PLAYERS];
		u32 nAmmoWpns = 0;
		for( s32 nPlayer = 0; nPlayer < CPlayer::m_nPlayerCount; ++nPlayer ) {
			if( apPlayerWpn[nPlayer] ) {
				apAmmoWpn[nAmmoWpns++] = apPlayerWpn[nPlayer];
			}
		}
		if( nAmmoWpns > 1 && m_nNumAmmo )
		{
			for (i=0; i<(u32)m_nNumAmmo; i++)
			{
				CWeapon *pAmmoWpn = apAmmoWpn[i % nAmmoWpns];
				const char *pszName = CItem::GetWeaponName( pAmmoWpn );
				sprintf(szName, "%s l%d", pszName, pAmmoWpn->GetUpgradeLevel()+1 );
				if (fclib_stricmp(pszName, "scatter blaster")==0)
				{
					sprintf(szName, "blaster l%d", pAmmoWpn->GetUpgradeLevel()+1 );
				}
				CCollectable::PlaceIntoWorld(szName, m_pAmmoPlacement[i]->MtxToWorld());
			}
		}
		else
#endif
		if (pCurWpn)
		{
			const char *pszName = CItem::GetWeaponName( pCurWpn );
			sprintf(szName, "%s l%d", pszName, pCurWpn->GetUpgradeLevel()+1 );
			if (fclib_stricmp(pszName, "scatter blaster")==0)
			{
				sprintf(szName, "blaster l%d", pCurWpn->GetUpgradeLevel()+1 );
			}

			if (m_nNumAmmo)
			{
				//fill ammo locations with player ammo.
				for (i=0; i<(u32)m_nNumAmmo; i++)
				{
					CCollectable::PlaceIntoWorld(szName, m_pAmmoPlacement[i]->MtxToWorld());
				}
			}
		}
	}

	return bRet;
}

BOOL CColiseumMiniGame::EscortOut()
{
	BOOL bRet = TRUE;
	
	return bRet;
}

BOOL CColiseumMiniGame::IsCurBotDead()
{
	if (m_nCurBot < m_nNumFriendly)
	{
		if (m_pFriendly[m_nCurBot])
		{
			return ( _IS_BOT_DEAD(m_pFriendly[m_nCurBot]) );
		}
	}
	else if (m_nCurBot < (m_nNumFriendly+m_nNumEnemies))
	{
		if (m_pEnemies[m_nCurBot-m_nNumFriendly])
		{
			return ( _IS_BOT_DEAD(m_pEnemies[m_nCurBot-m_nNumFriendly]) );
		}
	}
	return TRUE;
}

void CColiseumMiniGame::GruntsThrowGarbage()
{
	CFVec3 vOffs;

	if ( m_pGarbageGrunt[0] && m_pFriendly[0] )
	{
		vOffs = m_pFriendly[0]->GetMesh()->GetBoundingSphere().m_Pos - m_pGarbageGrunt[0]->GetMesh()->GetBoundingSphere().m_Pos;

		if ( FMATH_FABS(vOffs.x) < 75.0f && FMATH_FABS(vOffs.z) < 75.0f )
		{
			CFVec3A vPos, vDir;
			s32 nRndIdx;
			
			if (m_nGarbageIdx < 4)
			{
				if (m_pGarbageGrunt[m_nGarbageIdx])
				{
					vPos.v3 = m_pGarbageGrunt[m_nGarbageIdx]->GetMesh()->GetBoundingSphere().m_Pos;
					vPos.y += 3.0f;
					vDir = m_pGarbageGrunt[m_nGarbageIdx]->GetMesh()->m_Xfm.m_MtxF.m_vFront;
					vDir.x += fmath_RandomFloatRange(-0.2f, +0.3f);
					vDir.z += fmath_RandomFloatRange(-0.3f, +0.2f);
					vDir.y += 1.0f + fmath_RandomFloatRange(-0.3f, +0.3f);
					vDir.Unitize();

					m_dMtx.UnitMtxFromUnitVec(&vDir);
					m_dMtx.m_vPos = vPos;

					m_DebrisDef.m_LinVel_WS = vDir;
					m_DebrisDef.m_LinVel_WS.v3 *= 20.0f;
					m_DebrisDef.m_pSoundGroupImpact = m_pDebrisSndGroup;

					m_DebrisDef.m_pMesh = NULL;
					while ( !m_DebrisDef.m_pMesh )
					{
						nRndIdx = fmath_RandomRange(0, 3);
						m_DebrisDef.m_pMesh = m_pGarbageMesh[nRndIdx];
					};

					CFDebris::Spawn(&m_DebrisDef, &m_dMtx);
				}
			}

#if COLISEUM_ENABLE_AUTOJEER
			if (m_nGarbageIdx == 0 || m_nGarbageIdx == 18)
			{
				PlayJeer(TRUE);
			}
#endif

			m_nGarbageIdx = (m_nGarbageIdx+1)%36;
		}
	}
}

void CColiseumMiniGame::PlayCheer(CheerLevel_e eCheer)
{
	s32 nRndIdx;

	switch (eCheer)
	{
		case CHEER_MAX:
		{
			nRndIdx = fmath_RandomRange(0, 4);

			while ( m_nLastCheerIdx == nRndIdx )
			{
				nRndIdx = fmath_RandomRange(0, 4);
			};
			m_nLastCheerIdx = nRndIdx;
			
			switch (nRndIdx)
			{
				case 0:
					PlaySnd2D(COLISEUMSND_COMMON_CHEER_L01, TRUE, FALSE);
				break;
				case 1:
					PlaySnd2D(COLISEUMSND_COMMON_CHEER_L02, TRUE, FALSE);
				break;
				case 2:
					PlaySnd2D(COLISEUMSND_COMMON_CHEER_L03, TRUE, FALSE);
				break;
				case 3:
					PlaySnd2D(COLISEUMSND_COMMON_OWW_L01, TRUE, FALSE);
				break;
				case 4:
					PlaySnd2D(COLISEUMSND_COMMON_OWW_L02, TRUE, FALSE);
				break;
			};
		}
		break;
		case CHEER_LARGE:
		{
			nRndIdx = fmath_RandomRange(0, 4);

			while ( m_nLastCheerIdx == nRndIdx )
			{
				nRndIdx = fmath_RandomRange(0, 4);
			};
			m_nLastCheerIdx = nRndIdx;

			switch (nRndIdx)
			{
				case 0:
					PlaySnd2D(COLISEUMSND_COMMON_CHEER_L01, TRUE, FALSE);
				break;
				case 1:
					PlaySnd2D(COLISEUMSND_COMMON_CHEER_L02, TRUE, FALSE);
				break;
				case 2:
					PlaySnd2D(COLISEUMSND_COMMON_CHEER_L03, TRUE, FALSE);
				break;
				case 3:
					PlaySnd2D(COLISEUMSND_COMMON_OWW_M01, TRUE, FALSE);
				break;
				case 4:
					PlaySnd2D(COLISEUMSND_COMMON_OWW_M02, TRUE, FALSE);
				break;
			};
		}
		break;
		case CHEER_MED:
		{
			nRndIdx = fmath_RandomRange(0, 2);

			while ( m_nLastCheerIdx == nRndIdx )
			{
				nRndIdx = fmath_RandomRange(0, 2);
			};
			m_nLastCheerIdx = nRndIdx;

			switch (nRndIdx)
			{
				case 0:
					PlaySnd2D(COLISEUMSND_COMMON_CHEER_M01, TRUE, FALSE);
				break;
				case 1:
					PlaySnd2D(COLISEUMSND_COMMON_CHEER_M02, TRUE, FALSE);
				break;
				case 2:
					PlaySnd2D(COLISEUMSND_COMMON_CHEER_M03, TRUE, FALSE);
				break;
			};
		}
		break;
		case CHEER_SMALL:
		{
			nRndIdx = fmath_RandomRange(0, 2);

			while ( m_nLastCheerIdx == nRndIdx )
			{
				nRndIdx = fmath_RandomRange(0, 2);
			};
			m_nLastCheerIdx = nRndIdx;

			switch (nRndIdx)
			{
				case 0:
					PlaySnd2D(COLISEUMSND_COMMON_CHEER_S01, TRUE, FALSE);
				break;
				case 1:
					PlaySnd2D(COLISEUMSND_COMMON_CHEER_S02, TRUE, FALSE);
				break;
				case 2:
					PlaySnd2D(COLISEUMSND_COMMON_CHEER_S03, TRUE, FALSE);
				break;
			};
		}
		break;
	}
}

void CColiseumMiniGame::PlayBoo(BooLevel_e eBoo)
{
	s32 nRndIdx;

	switch (eBoo)
	{
		case BOO_SMALL:
		{
			nRndIdx = fmath_RandomRange(0, 20);

			if ( m_nLastBooIdx == nRndIdx )
			{
				nRndIdx = (nRndIdx==1)?(2):(1);
			};
			m_nLastBooIdx = nRndIdx;
			
			if (nRndIdx == 1)
			{
				PlaySnd2D(COLISEUMSND_COMMON_BOO_S01, TRUE, FALSE);
			}
			else if (nRndIdx == 2)
			{
				PlaySnd2D(COLISEUMSND_COMMON_BOO_S02, TRUE, FALSE);
				if (m_fJeerTime <= 0.0f)
				{
					PlayJeer();
					m_fJeerTime = 2.0f;
				}
			}
		}
		break;
		case BOO_MED:
		{
			nRndIdx = fmath_RandomRange(0, 10);

			if ( m_nLastBooIdx == nRndIdx )
			{
				nRndIdx = (nRndIdx==1)?(2):(1);
			};
			m_nLastBooIdx = nRndIdx;

			if (nRndIdx == 1)
			{
				PlaySnd2D(COLISEUMSND_COMMON_BOO_M01, TRUE, FALSE);
				if (m_fJeerTime <= 0.0f)
				{
					PlayJeer();
					m_fJeerTime = 2.0f;
				}
			}
			else if (nRndIdx == 2)
			{
				PlaySnd2D(COLISEUMSND_COMMON_BOO_M02, TRUE, FALSE);
			}
		}
		break;
		case BOO_MAX:
		{
			nRndIdx = fmath_RandomRange(0, 5);

			if ( m_nLastBooIdx == nRndIdx )
			{
				nRndIdx = (nRndIdx==1)?(2):(1);
			};
			m_nLastBooIdx = nRndIdx;

			nRndIdx = fmath_RandomRange(0, 5);
			if (nRndIdx == 1)
			{
				PlaySnd2D(COLISEUMSND_COMMON_BOO_L01, TRUE, FALSE);
			}
			else if (nRndIdx == 2)
			{
				PlaySnd2D(COLISEUMSND_COMMON_BOO_L02, TRUE, FALSE);
				if (m_fJeerTime <= 0.0f)
				{
					PlayJeer();
					m_fJeerTime = 2.0f;
				}
			}
		}
		break;
	}

	if (m_fJeerTime > 0.0f)
	{
		m_fJeerTime -= FLoop_fPreviousLoopSecs;
		if (m_fJeerTime < 0.0f) { m_fJeerTime = 0.0f; }
	}
}

void CColiseumMiniGame::PlayMusicSinger()
{
	s32 nRndIdx2 = fmath_RandomRange(1, 9);
	
	while ( m_nLastStingerIdx == nRndIdx2 )
	{
		nRndIdx2 = fmath_RandomRange(1, 9);
	};
	m_nLastStingerIdx = nRndIdx2;
				
	switch (nRndIdx2)
	{
		case 1:
			PlaySnd2D(COLISEUMSND_COMMON_STINGER_A1, TRUE, FALSE);
		break;
		case 2:
			PlaySnd2D(COLISEUMSND_COMMON_STINGER_A2, TRUE, FALSE);
		break;
		case 3:
			PlaySnd2D(COLISEUMSND_COMMON_STINGER_A3, TRUE, FALSE);
		break;
		case 4:
			PlaySnd2D(COLISEUMSND_COMMON_STINGER_B1, TRUE, FALSE);
		break;
		case 5:
			PlaySnd2D(COLISEUMSND_COMMON_STINGER_B2, TRUE, FALSE);
		break;
		case 6:
			PlaySnd2D(COLISEUMSND_COMMON_STINGER_B3, TRUE, FALSE);
		break;
		case 7:
			PlaySnd2D(COLISEUMSND_COMMON_STINGER_C1, TRUE, FALSE);
		break;
		case 8:
			PlaySnd2D(COLISEUMSND_COMMON_STINGER_C2, TRUE, FALSE);
		break;
		case 9:
			PlaySnd2D(COLISEUMSND_COMMON_STINGER_C3, TRUE, FALSE);
		break;
	};
}

void CColiseumMiniGame::PlayAnnouncer()
{
	s32 nRndIdx2 = fmath_RandomRange(0, 8);
	
	while ( m_nLastAnnouncerIdx == nRndIdx2 )
	{
		nRndIdx2 = fmath_RandomRange(1, 7);
	};
	m_nLastAnnouncerIdx = nRndIdx2;
				
	switch (nRndIdx2)
	{
		//Announcers
		case 1:
			PlaySnd2D(COLISEUMSND_LEVEL_ANN01, TRUE, FALSE);
		break;
		case 2:
			PlaySnd2D(COLISEUMSND_LEVEL_ANN02, TRUE, FALSE);
		break;
		case 3:
			PlaySnd2D(COLISEUMSND_LEVEL_ANN03, TRUE, FALSE);
		break;
		case 4:
			PlaySnd2D(COLISEUMSND_LEVEL_ANN04, TRUE, FALSE);
		break;
		case 5:
			PlaySnd2D(COLISEUMSND_LEVEL_ANN05, TRUE, FALSE);
		break;
		case 6:
			PlaySnd2D(COLISEUMSND_LEVEL_ANN06, TRUE, FALSE);
		break;
		case 7:
			PlaySnd2D(COLISEUMSND_LEVEL_ANN07, TRUE, FALSE);
		break;
	}
}

void CColiseumMiniGame::PlayCall()
{
	s32 nRndIdx2 = fmath_RandomRange(0, 100);

	if ( nRndIdx2 >= 1 && nRndIdx2 <= 10 )
	{
		while ( m_nLastCallIdx == nRndIdx2 )
		{
			nRndIdx2 = fmath_RandomRange(1, 10);
		};
		m_nLastCallIdx = nRndIdx2;
	}

	s32 nStinger = fmath_RandomRange(0, 100);

	if (nStinger < 25) //25%
	{
		PlayMusicSinger();
	}
	
	switch (nRndIdx2)
	{
		case 1:
			PlaySnd2D(COLISEUMSND_LEVEL_CAL_01, TRUE, FALSE);
		break;
		case 2:
			PlaySnd2D(COLISEUMSND_LEVEL_CAL_02, TRUE, FALSE);
		break;
		case 3:
			PlaySnd2D(COLISEUMSND_LEVEL_CAL_03, TRUE, FALSE);
		break;
		case 4:
			PlaySnd2D(COLISEUMSND_LEVEL_CAL_04, TRUE, FALSE);
		break;
		case 5:
			PlaySnd2D(COLISEUMSND_LEVEL_CAL_05, TRUE, FALSE);
		break;
		case 6:
			PlaySnd2D(COLISEUMSND_LEVEL_CAL_06, TRUE, FALSE);
		break;
		case 7:
			PlaySnd2D(COLISEUMSND_LEVEL_CAL_07, TRUE, FALSE);
		break;
		case 8:
			PlaySnd2D(COLISEUMSND_LEVEL_CAL_08, TRUE, FALSE);
		break;
		case 9:
			PlaySnd2D(COLISEUMSND_LEVEL_CAL_09, TRUE, FALSE);
		break;
		case 10:
			PlaySnd2D(COLISEUMSND_LEVEL_CAL_10, TRUE, FALSE);
		break;
	}
}

void CColiseumMiniGame::PlayJeer(BOOL bRndPan)
{
	s32 nRndIdx2 = fmath_RandomRange(1, 6);
	ColiseumSounds_e eSnd=COLISEUMSND_COUNT;

	while ( m_nLastJeerIdx == nRndIdx2 )
	{
		nRndIdx2 = fmath_RandomRange(1, 6);
	};
	m_nLastJeerIdx = nRndIdx2;
		
	switch (nRndIdx2)
	{
		case 1:
			eSnd = COLISEUMSND_LEVEL_JER_01;
		break;
		case 2:
			eSnd = COLISEUMSND_LEVEL_JER_02;
		break;
		case 3:
			eSnd = COLISEUMSND_LEVEL_JER_03;
		break;
		case 4:
			eSnd = COLISEUMSND_LEVEL_JER_04;
		break;
		case 5:
			eSnd = COLISEUMSND_LEVEL_JER_05;
		break;
		case 6:
			eSnd = COLISEUMSND_LEVEL_JER_06;
		break;
	}

	if (eSnd < COLISEUMSND_COUNT)
	{
		if ( PlaySnd2D( eSnd, TRUE, FALSE ) && bRndPan )
		{
			f32 fPan = fmath_RandomBipolarUnitFloat();
			SetSndPan( eSnd, fPan );
		}
	}

}

void CColiseumMiniGame::SetupTripwires()
{
	m_nNumTripwires = 0;
	m_nNumHuts = 0;

	m_pTripwireInside = NULL;
	m_pHutInside = NULL;

	//1. Find Huts
	u32 i;
	char szName[32];
	for (i=0; i<MAX_TRIPWIRES; i++)
	{
		sprintf(szName, "Hut%02d", i+1);
		m_paHuts[i] = (CMeshEntity *)CEntity::FindInWorld(szName);
		if (m_paHuts[i])
		{
			m_paHuts[i]->GetMesh()->m_nFlags &= ~FMESHINST_FLAG_NOCOLLIDE;
			m_paHuts[i]->GetMesh()->SetCollisionFlag( TRUE );
			m_nNumHuts++;
		}
		else
		{
			break;
		}
	}

	//2. Now find tripwires.
	for (i=0; i<MAX_TRIPWIRES; i++)
	{
		sprintf(szName, "trig_hut%02d", i+1);
		m_paTripwireEntity[i] = (CEntity *)CEntity::FindInWorld(szName);
		if (m_paTripwireEntity[i])
		{
			m_nNumTripwires++;
		}
		else
		{
			break;
		}
	}

	FASSERT( m_nNumHuts == m_nNumTripwires );
}

BOOL CColiseumMiniGame::RegisterForTripwireEvents() 
{
	// This function should register ourself with the scripting event system
	// for tripwire events.  Then every time glitch enters and exits
	// a tripwire, we will track it!

	s32 nTriggerEvent = CFScriptSystem::GetEventNumFromName( "tripwire" );
	if(nTriggerEvent == -1) 
	{
		DEVPRINTF( "CColiseumMiniGame::RegisterForTripwireEvents() : Could not find tripwire event.\n" );
		return FALSE;
	}

	u64 uEventFlags = fevent_Bit( nTriggerEvent );

	if( !CFScriptSystem::RegisterEventListener(CheckTripWireEvents, NULL, &uEventFlags, (u32)nTriggerEvent) ) 
	{
		DEVPRINTF( "CColiseumMiniGame::RegisterForTripwireEvents() : Could not register Tripwire listener callback.\n" );
		return FALSE;
	}

	m_pColiseumMiniGame->m_hExpGroup = fexplosion_GetExplosionGroup(_HUT_EXPL_NAME);

	return TRUE;
}

void CColiseumMiniGame::CrushParent(CEntity *pParent)
{
	u32 i;
	CMeshEntity *pParentME = (CMeshEntity *)pParent;
	for (i=0; i<m_nNumHuts; i++)
	{
		if ( m_paHuts[i] == pParentME )
		{
			if (m_pColiseumMiniGame->m_pHutInside == m_paHuts[i])
			{
				m_pColiseumMiniGame->m_pTripwireInside = NULL;
				
				CCorrosiveCombat::CancelPeer(TRUE);
				CCorrosiveCombat::CancelFingerFlick();
			}

			HutDestroyedCallback(i);
		}
	}
}

void CColiseumMiniGame::HutDestroyedCallback(u32 nUserData)
{
	if (m_pColiseumMiniGame)
	{
		if (nUserData >= 0 && nUserData < m_pColiseumMiniGame->m_nNumHuts)
		{
			if (!m_pColiseumMiniGame->m_paHuts[ nUserData ]) { return; }

			f32 fAggressive = CCorrosiveCombat::GetAggressiveness();
			fAggressive += 0.25f;
			fAggressive = (fAggressive < 2.0f)?(fAggressive):(2.0f);
			CCorrosiveCombat::SetAggressiveness(fAggressive);

			Player_aPlayer[0].m_pEntityCurrent->ShakeCamera(0.25f, 2.0f);

			if( m_pColiseumMiniGame->m_hExpGroup != FEXPLOSION_INVALID_HANDLE )
			{
				CFVec3A SpawnPos_WS;
				FExplosionSpawnParams_t SpawnParams;

				SpawnPos_WS.v3 = m_pColiseumMiniGame->m_paHuts[ nUserData ]->GetMesh()->GetBoundingSphere().m_Pos;

				SpawnParams.InitToDefaults();

				SpawnParams.uFlags = FEXPLOSION_SPAWN_NO_DAMAGE;
				SpawnParams.Pos_WS = SpawnPos_WS;
				SpawnParams.uSurfaceType = 0;
				SpawnParams.pDamageProfile = NULL;
				SpawnParams.pDamager = NULL;
				
				FExplosion_SpawnerHandle_t hSpawn = fexplosion_GetExplosionSpawner();
				fexplosion_SpawnExplosion(hSpawn, m_pColiseumMiniGame->m_hExpGroup, &SpawnParams);
			}

			if (m_pColiseumMiniGame->m_paHuts[ nUserData ]->GetMeshCount() > 1)
			{
				m_pColiseumMiniGame->m_paHuts[ nUserData ]->GetMesh()->m_nFlags |= FMESHINST_FLAG_NOCOLLIDE;
				m_pColiseumMiniGame->m_paHuts[ nUserData ]->GetMesh()->SetCollisionFlag( FALSE );
				m_pColiseumMiniGame->m_paHuts[ nUserData ]->SelectMesh(1);
			}
			else
			{
				m_pColiseumMiniGame->m_paHuts[ nUserData ]->RemoveFromWorld(TRUE);
			}

			//Make sure that the hut that was destroyed is the hut Glitch is hiding in before clearing the m_pHutInside pointer.
			if (m_pColiseumMiniGame->m_paHuts[nUserData] == m_pColiseumMiniGame->m_pHutInside)
			{
				m_pColiseumMiniGame->m_pHutInside = NULL;

#if FANG_WINGC
				if( MultiplayerMgr.IsLocalCoop() ) {
					for( s32 n = 0; n < CPlayer::m_nPlayerCount; ++n ) {
						CBotGlitch *pGlitch = _PortAliveArenaGlitch(n);
						if( (_aPortHutPlayers[nUserData] & (1U << n)) && pGlitch && pGlitch->IsInPieces() ) {
							m_pColiseumMiniGame->PortSetCorrosiveWinner(n);
							m_pColiseumMiniGame->m_State = STATE_END_CUTSCENE;
							m_pColiseumMiniGame->m_fCutSceneTime = 0.0f;
							break;
						}
					}
				} else
#endif
				{
				CBotGlitch *pGlitch = (CBotGlitch *)m_pColiseumMiniGame->m_pFriendly[0];
				if (pGlitch)
				{
					if ( pGlitch->IsInPieces() )
					{
						m_pColiseumMiniGame->m_nScore = 4;
						m_pColiseumMiniGame->m_State = STATE_END_CUTSCENE;
						m_pColiseumMiniGame->m_fCutSceneTime = 0.0f;
					}
				}
				}
			}

			m_pColiseumMiniGame->m_paHuts[ nUserData ] = NULL;
			
			m_pColiseumMiniGame->PlayCheer(CHEER_MAX);

			m_pColiseumMiniGame->m_fTimeSinceLastDamage = 0.0f;

			m_pColiseumMiniGame->m_nExcitedLevel += 20;
			FMATH_CLAMP(m_pColiseumMiniGame->m_nExcitedLevel, 0, 100);

			s32 nRndIdx = fmath_RandomRange(0, 100);
			if (nRndIdx < 25) //25% chance whenever Corrosive destroys a Hut.
			{
				m_pColiseumMiniGame->PlayAnnouncer();
			}
			else
			{
				m_pColiseumMiniGame->PlayMusicSinger();
			}
		}
	}
}

BOOL CColiseumMiniGame::PeerDone( AIBehaviorReason_e eReason )
{
	BOOL bRetVal = FALSE;

	switch( eReason ) 
	{
		case AI_BEHAVIOR_REASON_LOST_LOS:
			bRetVal = FALSE;
		break;

		case AI_BEHAVIOR_REASON_ATTACK_COMPLETE:
			// Let him revert back to his base attack state...
			// He'll be requesting a hint soon enough...
			bRetVal = FALSE;
		break;

		case AI_BEHAVIOR_REASON_PEER_COMPLETE:
			// The peer has been completed... We should have corrosive
			// flick that location now...
		
			m_pColiseumMiniGame->m_bDoPeer = FALSE;
			m_pColiseumMiniGame->m_bDoFlickNextFrame = TRUE;

			bRetVal = FALSE;
		break;
	}

	return bRetVal;
}

void CColiseumMiniGame::CheckTripWireEvents(s32 nWhichEvent, u32 uUserData, u32 uEventData1, u32 uEventData2, u32 uEventData3)
{
	if (m_pColiseumMiniGame)
	{
		u32 i;
		// Decode the EventData parameters into more usefull classes:
		CEntity *pTripwireEntity = (CEntity*)uEventData2;
		CEntity *pTripperEntity = (CEntity*)uEventData3;
		if(pTripwireEntity == NULL || pTripperEntity == NULL) 
		{
			return;
		}

		// First, we are only interested in tripwires that bots enter and exits...
		if ( !(pTripperEntity->TypeBits()&ENTITY_BIT_BOT) ) 
		{
			// This entity is NOT a bot.. bail out
			return;
		}

		// Safe to cast this guy as a bot
		CBot *pBot = (CBot*)pTripperEntity;

		if( pBot->m_nPossessionPlayerIndex < 0 ) 
		{
			// This bot is not player controlled... bail out
			return;
		}

		// Next, we want to see if this is a tripwire in our list...
		CEntity *pCurTripwireEntity = NULL;
		for(i = 0; i < m_pColiseumMiniGame->m_nNumTripwires; i++)
		{
			if( m_pColiseumMiniGame->m_paTripwireEntity[i] == pTripwireEntity ) 
			{
				pCurTripwireEntity = m_pColiseumMiniGame->m_paTripwireEntity[i];
				break;
			}
		}

		if( !pCurTripwireEntity ) 
		{
			// This is NOT a tripwire that we are interested in.  Bail out
			return;
		}

		if ( !m_pColiseumMiniGame->m_paHuts[i] )
		{
			//The hut has already been destroyed, move along.
			return;
		}

#if FANG_WINGC
		if( m_pColiseumMiniGame->m_nLevelID == 3 && MultiplayerMgr.IsLocalCoop() &&
			pBot->m_nPossessionPlayerIndex < CPlayer::m_nPlayerCount ) {
			const u32 nPlayerBit = 1U << pBot->m_nPossessionPlayerIndex;
			if( uEventData1 == 0 ) _aPortHutPlayers[i] |= nPlayerBit;
			else if( uEventData1 == 1 ) {
				_aPortHutPlayers[i] &= ~nPlayerBit;
				if( _aPortHutPlayers[i] ) return;
			}
		}
#endif

		// The tripwire that was triggered is one of the tripwires we are interested in,
		// so lets do some further processing on it.
		if(uEventData1 == 0) 
		{
			// Glitch is entering this tripwire.  Record off the tripwire info structure in the mini game class
			m_pColiseumMiniGame->m_pTripwireInside = pCurTripwireEntity;
			m_pColiseumMiniGame->m_pHutInside = m_pColiseumMiniGame->m_paHuts[i];
			m_pColiseumMiniGame->m_nHitIdx = i;

			CMeshEntity *pMeshEntity = (CMeshEntity *)m_pColiseumMiniGame->m_pHutInside;

			//CCorrosiveCombat::SetRoarThisFrame(TRUE);

			if (m_pColiseumMiniGame->m_bDoPeer)
			{
				CFVec3A vHutLoc;
				vHutLoc.v3 = pMeshEntity->GetMesh()->GetBoundingSphere().m_Pos;

				if ( 1 )//(pMeshEntity->GetMesh()->GetBoundingSphere().m_Pos - m_pColiseumMiniGame->m_pFriendly[0]->GetMesh()->GetBoundingSphere().m_Pos).MagXZ() > 30.0f )
				{
					CCorrosiveCombat::PeerAtThisLocation( &vHutLoc, FALSE );
					CCorrosiveCombat::SetBehaviorRequestCallback( PeerDone );
				}
				else
				{
					CCorrosiveCombat::DoFingerFlick( pMeshEntity->GetMesh()->GetBoundingSphere(), i, (AttackCompleteCallback_t)HutDestroyedCallback );

					u32 nRnd = fmath_RandomRange(0, 5);
					m_pColiseumMiniGame->m_bDoPeer = TRUE;
				}
			}
			else
			{
				CCorrosiveCombat::DoFingerFlick( pMeshEntity->GetMesh()->GetBoundingSphere(), i, (AttackCompleteCallback_t)HutDestroyedCallback );

				u32 nRnd = fmath_RandomRange(0, 5);
				m_pColiseumMiniGame->m_bDoPeer = TRUE;
			}
		} 
		else if (uEventData1 == 1) 
		{
			// Glitch is LEAVING the tripwire... Set the tripwire info structure to null 
			// if the current entity pointer == pTripwireEntity.
			if( m_pColiseumMiniGame->m_pTripwireInside == pCurTripwireEntity ) 
			{
				m_pColiseumMiniGame->m_pTripwireInside = NULL;
				m_pColiseumMiniGame->m_pHutInside = NULL;

				CCorrosiveCombat::CancelPeer();
				CCorrosiveCombat::CancelFingerFlick();
			}
		}
	}
}

void CColiseumMiniGame::PlayPlayerDeathSound()
{
	if (m_nLevelID < 2)
	{
		PlayCheer(CHEER_MAX);
		PlayCheer(CHEER_LARGE);
	}
	else
	{
		PlayBoo(BOO_MAX);
		CCorrosiveCombat::SetLaughThisFrame(TRUE);
	}
}

void CColiseumMiniGame::UpdateScoreBoard()
{
	u32 i;
	//Update scoreboard
	for (i=0; i<4; i++)
	{
		if (m_pScoreBoard[i])
		{
			s32 nS10, nS1;
			f32 fDelta=0.1f;
			if (i == 0 || i == 1)
			{
				nS10 = (m_nScore/10)*10;
				nS1 = m_nScore - nS10;
			}
			else
			{
				nS10 = (m_nMilScore/10)*10;
				nS1 = m_nMilScore - nS10;
			}

			m_vScore10.y = 0.0f;
			m_vScore10.x = (f32)nS10 * 0.1f * fDelta;
			m_vScore1.y = 0.0f;
			m_vScore1.x = (f32)nS1 * fDelta;

			m_pScoreBoard[i]->AnimTC_SetScrollST(m_aTexLayerHandle[i*2 + 0], m_vScore10);
			m_pScoreBoard[i]->AnimTC_SetScrollST(m_aTexLayerHandle[i*2 + 1], m_vScore1);

			if ( (m_fScored&&i<2)||(m_fScoredMil&&i>1))
			{
				m_pScoreBoard[i]->SetMeshTint( SCORE_HIGHLIGHT_R, SCORE_HIGHLIGHT_G, SCORE_HIGHLIGHT_B );
			}
			else
			{
				if (i<2)
				{
					m_pScoreBoard[i]->SetMeshTint(SCORE_DROID_R, SCORE_DROID_G, SCORE_DROID_B);
				}
				else
				{
					m_pScoreBoard[i]->SetMeshTint(SCORE_MIL_R, SCORE_MIL_G, SCORE_MIL_B);
				}
				m_pScoreBoard[i]->SetMeshAlpha(1.0f);
			}
		}
	}
}

BOOL CColiseumMiniGame::MainGame()
{
	CWeapon *pWpn=NULL;
	BOOL bRet = FALSE;
	s32 i;
	s32 nRndIdx;

#if COLISEUM_PRINT_DEBUG && !FANG_PRODUCTION_BUILD
	ftext_DebugPrintf( 0.1f, 0.331f, "~C99999999:Num Sounds %d", m_nNumSoundsPlaying);
#endif
	
	m_pColiseumMiniGame->PlaySnd2D(COLISEUMSND_LEVEL_CROWD, TRUE, FALSE, m_fStreamVolume);
	
	if (m_pColiseumMiniGame->m_nLevelID >= 2)
	{
		CGroundCombat *pCombat = CCorrosiveCombat::GetGroundCombatPtr();
		if( pCombat ) {
			pCombat->m_uAttackFlags |= CGroundCombat::FLAG_DISABLE_ENEMY_RELATED_EXIT_CONDITIONS;
#if FANG_WINGC
			// The retail script starts one persistent attack against P1. Keep it alive
			// against a surviving partner after P1 dies, without changing other bosses.
			if( m_nLevelID == 3 && MultiplayerMgr.IsLocalCoop() ) _PortRetargetCorrosive(m_pEnemies[0], pCombat);
#endif
		}
	}

	
	UpdateScoreBoard();
	
	if (m_fScored)
	{
		m_fScored -= FLoop_fPreviousLoopSecs;
		if (m_fScored < 0.0f) { m_fScored = 0.0f; }
	}

	if (m_fScoredMil)
	{
		m_fScoredMil -= FLoop_fPreviousLoopSecs;
		if (m_fScoredMil < 0.0f) { m_fScoredMil = 0.0f; }
	}

	if (m_pTVScr[1] || m_pTVScr[2])
	{
		if (m_nCamIdx == 0)
		{
			m_vCamPos[0].Set(-96, 186, 456);
			m_vCamTarget[0].Set(-96+m_fOffsX, -3, 228);
						
			CFVec3 vDir;
			if (m_fCamScale > 0.0f)
			{
				vDir = m_vCamTarget[0] - m_vCamPos[0];
				vDir.Unitize();

				vDir = vDir * m_fCamScale;
				m_vCamPos[0] += vDir;
			}

			//pan the camera
			m_fOffsX += (m_fDeltaX*FLoop_fPreviousLoopSecs);
			if (m_fOffsX > m_fMaxOffsX && m_fDeltaX > 0.0f) 
			{ 
				m_fOffsX = m_fMaxOffsX; 
				m_fDeltaX = -m_fDeltaX; 
			}
			else if (m_fOffsX < m_fMinOffsX && m_fDeltaX < 0.0f) 
			{ 
				m_fOffsX = m_fMinOffsX; 
				m_fDeltaX = -m_fDeltaX; 
			}

			m_fCamScale += m_fDeltaScale*FLoop_fPreviousLoopSecs;
			if (m_fCamScale > m_fMaxScale && m_fDeltaScale > 0.0f)
			{
				m_fCamScale = m_fMaxScale;
				m_fDeltaScale = -m_fDeltaScale;
			}
			else if (m_fCamScale < 0.0f && m_fDeltaScale < 0.0f)
			{
				m_fCamScale = 0.0f;
				m_fDeltaScale = -m_fDeltaScale;
			}

			m_fSwitchTime = 0.0f;
			//
		}
		else
		{
			m_fSwitchTime += FLoop_fPreviousLoopSecs;
			if ( m_fSwitchTime > m_fBotSwitch || IsCurBotDead() )
			{
				m_fSwitchTime = 0;
				m_bSwitched = TRUE;
				m_nCurBot++;

				BOOL bGoodBot;
				do
				{
					bGoodBot = TRUE;
					if (m_nCurBot >= m_nNumFriendly && m_nCurBot < (m_nNumFriendly+m_nNumEnemies))
					{
						if (m_pEnemies[m_nCurBot-m_nNumFriendly])
						{
							if ( _IS_BOT_DEAD(m_pEnemies[m_nCurBot-m_nNumFriendly])) { m_nCurBot++; bGoodBot = FALSE; }
							else if (m_pEnemies[m_nCurBot-m_nNumFriendly]->IsImmobileOrPending()) { m_nCurBot++; bGoodBot = FALSE; }
							else if (!m_pEnemies[m_nCurBot-m_nNumFriendly]->IsInWorld()) { m_nCurBot++; bGoodBot = FALSE; }
						}
					}
					else if (m_nCurBot < m_nNumFriendly)
					{
						if (m_pFriendly[m_nCurBot])
						{
							if (_IS_BOT_DEAD(m_pFriendly[m_nCurBot])) { m_nCurBot++; bGoodBot = FALSE; }
							else if (m_pFriendly[m_nCurBot]->IsImmobileOrPending()) { m_nCurBot++; bGoodBot = FALSE; }
							else if (!m_pFriendly[m_nCurBot]->IsInWorld()) { m_nCurBot++; bGoodBot = FALSE; }
						}
					}
				} while (!bGoodBot);

				if (m_nCurBot >= (m_nNumFriendly+m_nNumEnemies) )
				{
					m_nCurBot = 0;
				}
			}
		}
	}

	//Check for my camera switch
	if (m_nCamIdx == 0 && Player_aPlayer[0].m_pEntityOrig)
	{
		CFVec3 vCamSwitch(8, 6, 91);
		f32 fCamSwitchRad2 = 14.0f*14.0f;

		GruntsThrowGarbage();

		CFWorldMesh *pMesh = Player_aPlayer[0].m_pEntityOrig->GetMesh();
		CFVec3 vOffs = pMesh->GetBoundingSphere().m_Pos - vCamSwitch;

		if (vOffs.Mag2() < fCamSwitchRad2)
		{
			m_nCamIdx = 1;
			if (m_nLevelID == 0)
			{
				//PlaySnd2D(COLISEUMSND_COMMON_CHEER_S03, TRUE);
				PlaySnd2D(COLISEUMSND_COMMON_BOO_L01, TRUE);
			}
			else if (m_nLevelID == 1)
			{
				//PlaySnd2D(COLISEUMSND_COMMON_CHEER_M03, TRUE);
				PlaySnd2D(COLISEUMSND_COMMON_CHEER_L02, TRUE);
			}
			else if (m_nLevelID == 2)
			{
				PlaySnd2D(COLISEUMSND_COMMON_CHEER_L03, TRUE);
			}
			else if (m_nLevelID == 3)
			{
				PlaySnd2D(COLISEUMSND_COMMON_CHEER_L01, TRUE);
				PlaySnd2D(COLISEUMSND_LEVEL_CAL_01, TRUE);
			}
		}
	}
	//

	if (m_nCamIdx == 1)
	{
		if (m_fStreamVolume < m_fMaxStreamVolume)
		{
			m_fStreamVolume += FLoop_fPreviousLoopSecs*0.20f;
			if (m_fStreamVolume > m_fMaxStreamVolume) { m_fStreamVolume = m_fMaxStreamVolume; }
		}
		if (m_nLevelID == 3 && m_pCorrosiveThrone)
		{
			f32 fUnitPos = m_pCorrosiveThrone->GetUnitPos();
			if (fUnitPos > 0.10f)
			{
				m_pCorrosiveThrone->SetCollisionFlag(FALSE);
			}
			else
			{
				m_pCorrosiveThrone->SetCollisionFlag(TRUE);
			}
		}
	}

	//Update devices - magnetic
	if (m_nMagnetDevices)
	{
		for (i=0; i<m_nMagnetDevices; i++)
		{
			m_apMagnetDevice[i]->Work();
		}
	}

    //Corrosive fire window, make sure he doesn't fire too many rockets at the player.
	if (m_nLevelID == 2 && m_fFireTime > 0.0f)
	{
		m_fFireTime -= FLoop_fPreviousLoopSecs;
		if (m_fFireTime <= 0.0f)
		{
			m_fFireTime = 0.0f;
			CCorrosiveCombat::FreeToFireRockets(FALSE);
		}
	}

	if (m_nLevelID == 3)
	{
		if ( m_pTripwireInside && m_pColiseumMiniGame->m_pHutInside )
		{
			CCorrosiveCombat::FreeToFireRockets(FALSE);
		}
		else
		{
			CCorrosiveCombat::FreeToFireRockets(TRUE);
		}

		if (m_pColiseumMiniGame->m_bDoFlickNextFrame)
		{
			if (m_pColiseumMiniGame->m_pHutInside)
			{
				CMeshEntity *pMeshEntity = (CMeshEntity *)m_pColiseumMiniGame->m_pHutInside;
				CCorrosiveCombat::DoFingerFlick( pMeshEntity->GetMesh()->GetBoundingSphere(), m_pColiseumMiniGame->m_nHitIdx, (AttackCompleteCallback_t)HutDestroyedCallback );
			}

			m_pColiseumMiniGame->m_bDoFlickNextFrame = FALSE;
		}

		CBotCorrosive *pCorrosive = (CBotCorrosive *)m_pEnemies[0];

		u32 i;
		if (pCorrosive)
		{
			CEntity *pParent = pCorrosive->GetParent();
			if (pParent)
			{
				CrushParent(pParent);
			}

			if ( pCorrosive->IsInMiddleOfSwat() )
			{
				for (i=0; i<m_nNumHuts; i++)
				{
					if (pCorrosive->IsEntityHit(m_paHuts[i]))
					{
						if (m_pColiseumMiniGame->m_pHutInside == m_paHuts[i])
						{
							m_pColiseumMiniGame->m_pTripwireInside = NULL;
							
							CCorrosiveCombat::CancelPeer(TRUE);
							CCorrosiveCombat::CancelFingerFlick();
						}

						HutDestroyedCallback(i);

						pCorrosive->ClearEntityHitList();
					}
				}
			}

			f32 fAggressive = CCorrosiveCombat::GetAggressiveness();
			fAggressive += ( 0.01f * FLoop_fPreviousLoopSecs );
			fAggressive = (fAggressive < 2.0f)?(fAggressive):(2.0f);
			CCorrosiveCombat::SetAggressiveness(fAggressive);
		}

#if FANG_WINGC
		if( MultiplayerMgr.IsLocalCoop() ) {
			if( PortCheckCorrosivePlayers(pCorrosive) ) return TRUE;
		} else
#endif
		{
		//Check for game completion
		CBotGlitch *pGlitch = (CBotGlitch *)m_pFriendly[0];
		//Is Glitch in pieces?
		if ( !pGlitch->IsInPieces() )
		{
			CCorrosiveCombat::SetPreferredMeleeAttack(CCorrosiveCombat::PREFERED_MELEE_SWIPE);
		}

		if ( pGlitch->IsInPieces() )
		{
			if (m_pEnemies[0])
			{
				//Is Corrosive in the middle of his swing?
				pCorrosive->SetSwatDamageRadiusMul(2.0f);

				if ( pCorrosive->IsInMiddleOfSwat() )
				{
					pGlitch->SetArmorModifier(0.99f);
				}
				else
				{
					pGlitch->SetArmorModifier(0.0f);
				}

				if ( pCorrosive->IsInMiddleOfSwat() && pCorrosive->IsEntityHit(pGlitch) && !m_bHitWhole && !m_pFriendly[0]->IsDeadOrDying() )
				{
					m_nScore = 4;
					return TRUE;
				}
				else if ( pCorrosive->IsInMiddleOfSwat() && pCorrosive->IsEntityHit(pGlitch) )
				{
					m_bHitWhole = FALSE;
				}
				else if ( !pCorrosive->IsInMiddleOfSwat() )
				{
					pGlitch->SetTargetable(TRUE);
					CCorrosiveCombat::SetPreferredMeleeAttack(CCorrosiveCombat::PREFERED_MELEE_STOMP);
				}
			}
		}
		else
		{
			pGlitch->SetArmorModifier(0.0f);
			
			if (m_pEnemies[0])
			{
				CBotCorrosive *pCorrosive = (CBotCorrosive *)m_pEnemies[0];
				pCorrosive->SetSwatDamageRadiusMul();

				if ( pCorrosive->IsInMiddleOfSwat() && pCorrosive->IsEntityHit(pGlitch) )
				{
					pCorrosive->ClearEntityHitList();
				}
				else
				{
					m_bHitWhole = FALSE;
				}
			}
		}
		}
	}

	//Check if enemies are still alive, is game over?
	if (m_nNumEnemies > 0)
	{
		BOOL bAllDead=TRUE;
		s32 nDead=0;
		for (i=0; i<m_nNumEnemies; i++)
		{
			if ( m_pEnemies[i] )
			{
				if ( !_IS_BOT_DEAD(m_pEnemies[i]) )
				{
					if ( !(m_pEnemies[i]->LeafTypeBit()&ENTITY_BIT_BOTCORROSIVE) )
					{
						bAllDead = FALSE;
					}
				}
				else
				{
					nDead++;
				}
			}
			else
			{
				bAllDead = FALSE;
			}
		}
		bRet = ( bAllDead && m_nNumEnemies>1 );
		
		//You get 1 point per enemy killed.
		if (nDead > m_nScore)
		{
			if (m_nScore == 0 && m_nLevelID == 1)
			{
				for (i=1; i<m_nNumFriendly; i++)
				{
					ai_ChangeBuddyCtrl( m_pFriendly[i]->AIBrain(), AI_BUDDY_CTRL_NEVER );
					ai_AssignJob_Wander( m_pFriendly[i]->AIBrain(), 1000.0f, TRUE, 75, 100, 0, 100, 0 );
				}
			}

			m_fScored = 2.0f;
			m_eScoreType = SCORE_DIRECT;

			PlayCheer(CHEER_MAX);
			PlayAnnouncer();

			m_nExcitedLevel += 50;

			if (m_nScore > 1 && m_nLevelID == 2)
			{
				CCorrosiveCombat::FreeToFireRockets(TRUE);
				CCorrosiveCombat::SetRoarThisFrame(TRUE);

				m_fFireTime = 7.0f;
			}
		}

		m_nScore = nDead;
		//Make sure that the level kill total includes indirect kills, so override the kill counter.
		Player_aPlayer[0].OverrideUnitsDestroyed(m_nScore);
		
		if ( bRet == TRUE )
		{
			UpdateScoreBoard();			
		}

		//Mil Score
		nDead=0;
		for (i=0; i<m_nNumFriendly; i++)
		{
			if ( m_pFriendly[i] )
			{
				if ( _IS_BOT_DEAD(m_pFriendly[i]) )
				{
					nDead++;
				}
			}
		}
		if (nDead > m_nMilScore)
		{
			m_fScoredMil = 2.0f;

			PlayCheer(CHEER_MAX);
			PlayAnnouncer();

			m_nExcitedLevel += 50;
		}
		m_nMilScore = nDead;
	}

	for (i=1; i<m_nNumFriendly; i++)
	{
		if (m_pFriendly[i])
		{
			pWpn = ((CBotMiner*)m_pFriendly[i])->GetPrimaryWeapon();
			if (pWpn->GetReserveAmmo() < 1)
			{
				pWpn->SetReserveAmmo(32767, TRUE);
			}
			
			CAIBrain *pBrain = m_pFriendly[i]->AIBrain();
			if (pBrain)
			{
				pBrain->SetBaseSpeed(100);
				pBrain->SetEyeCosHalfFOVAlert(0.7071f);		//45 degree half angle
				pBrain->SetEyeCosHalfFOVUnalert(0.7071f);
				pBrain->SetEyeScanDistAlert(1000.0f);
				pBrain->SetEyeScanDistUnalert(1000.0f);
				pBrain->SetHearingMagAlert(1000.0f);
				pBrain->SetHearingMagUnalert(1000.0f);
			}
		}
	}

	//Check if I damaged anybody
	if (m_pFriendly[0] && m_nCamIdx == 1)
	{
		if (CDamage::m_LastEntityDamaged.nTimeStamp == FVid_nFrameCounter-1) //Is the damage current?
		{
			//Cheerlevel == damage inflicted. 
			//Note that to get the highest cheerlevel, you will have to hit them directly with grenades or rockets.
			m_fTimeSinceLastDamage = 0.0f;

			nRndIdx = fmath_RandomRange(0, 4);

			if (FMATH_FABS(CDamage::m_LastEntityDamaged.fDeltaHitPoints) >= 75.0f)
			{
				PlayCheer(CHEER_MAX);

				m_nExcitedLevel += 10;
				FMATH_CLAMP(m_nExcitedLevel, 0, 100);
			}
			else if (FMATH_FABS(CDamage::m_LastEntityDamaged.fDeltaHitPoints) >= 50.0f)
			{
				PlayCheer(CHEER_LARGE);

				m_nExcitedLevel += 5;
				FMATH_CLAMP(m_nExcitedLevel, 0, 100);
			}
			else if (FMATH_FABS(CDamage::m_LastEntityDamaged.fDeltaHitPoints) >= 10.0f)
			{
				PlayCheer(CHEER_MED);

				m_nExcitedLevel += 2;
				FMATH_CLAMP(m_nExcitedLevel, 0, 100);
			}
			else if (FMATH_FABS(CDamage::m_LastEntityDamaged.fDeltaHitPoints) > 0.0f)
			{
				PlayCheer(CHEER_SMALL);

				m_nExcitedLevel += 1;
				FMATH_CLAMP(m_nExcitedLevel, 0, 100);
			}

			if (FMATH_FABS(CDamage::m_LastEntityDamaged.fDeltaHitPoints) > 0.0f)
			{
				PlayCall();
			}

			if ( m_nLevelID == 3 ) //Corrosive level
			{
				nRndIdx = fmath_RandomRange(0, 100);
				if (nRndIdx < 5) //5% chance whenever Corrosive damages Glitch.
				{
					PlayAnnouncer();
				}
			}
		}
		else
		{
			m_fTimeSinceLastDamage += FLoop_fPreviousLoopSecs;

			if (m_nExcitedLevel > 0)
			{
				if (m_fTimeUntilDec <= 0.0f)
				{
					m_nExcitedLevel--;
					FMATH_CLAMP(m_nExcitedLevel, 0, 100);

					if ( m_nExcitedLevel > 50 )
					{
						m_fTimeUntilDec = 0.10f;
					}
					else
					{
						m_fTimeUntilDec = 1.0f;
					}
				}
				else
				{
					m_fTimeUntilDec -= FLoop_fPreviousLoopSecs;
				}
			}
		}
	}

	if ( (_IS_BOT_DEAD(m_pFriendly[0]) || m_pFriendly[0]->IsDying())
#if FANG_WINGC
		&& !(m_nLevelID == 3 && MultiplayerMgr.IsLocalCoop() && _PortAnyArenaPlayerAlive())
#endif
		)
	{
		m_fTimeSinceLastDamage = 0.0f;
		m_nExcitedLevel = 100;

		PlayPlayerDeathSound();
	}

	if ( m_fTimeSinceLastDamage > _BOO_LARGE_THRESHOLD )
	{
		PlayBoo(BOO_MAX);

		m_nExcitedLevel = 0;
	}
	else if ( m_fTimeSinceLastDamage > _BOO_MED_THRESHOLD )
	{
		PlayBoo(BOO_MED);

		m_nExcitedLevel = 0;
	}
	else if ( m_fTimeSinceLastDamage > _BOO_SMALL_THRESHOLD )
	{
		PlayBoo(BOO_SMALL);

		m_nExcitedLevel = 0;
	}

	//Should the Blade chopping sound play this frame?
	if (!m_bPlayBladeSnd)
	{
		//If not then clear (stop) the sound.
		PlaySnd3D(COLISEUMSND_COMMON_BLADE, NULL, 0);
	}
	m_bPlayBladeSnd = FALSE;

	if (!m_bPlayMagnetSnd)
	{
		//If not then clear (stop) the sound.
		PlaySnd3D(COLISEUMSND_COMMON_MAGNET, NULL, 0);
	}
	m_bPlayMagnetSnd = FALSE;

	UpdateCrowdAnim();
	CleanUpSounds();

	return bRet;
}

#if FANG_WINGC
void CColiseumMiniGame::PortSetCorrosiveWinner( s32 nPlayer ) {
	_nPortCorrosiveWinner = nPlayer;
	m_pFriendly[0] = _PortAliveArenaGlitch(nPlayer);
	m_nScore = 4;
	DEVPRINTF("Co-op: player %d completes Corrosive's wrench escape.\n", nPlayer + 1);
}

BOOL CColiseumMiniGame::PortCheckCorrosivePlayers( CBotCorrosive *pCorrosive ) {
	if( !pCorrosive ) return FALSE;
	const BOOL bSwat = pCorrosive->IsInMiddleOfSwat();
	BOOL bHitWhole = FALSE;
	CBotGlitch *pTarget = NULL;
	CGroundCombat *pCombat = CCorrosiveCombat::GetGroundCombatPtr();
	for( s32 n = 0; n < CPlayer::m_nPlayerCount; ++n ) {
		CBotGlitch *pGlitch = _PortAliveArenaGlitch(n);
		if( !pGlitch ) continue;
		if( pCombat && pCombat->m_pEnemy == pGlitch ) pTarget = pGlitch;
		if( pGlitch->IsInPieces() ) {
			pGlitch->SetArmorModifier(bSwat ? 0.99f : 0.0f);
			if( bSwat && pCorrosive->IsEntityHit(pGlitch) && !m_bHitWhole ) {
				PortSetCorrosiveWinner(n);
				return TRUE;
			}
			if( !bSwat ) pGlitch->SetTargetable(TRUE);
		} else {
			pGlitch->SetArmorModifier(0.0f);
			if( bSwat && pCorrosive->IsEntityHit(pGlitch) ) bHitWhole = TRUE;
		}
	}
	// Wait until every player's win check has seen this swing before clearing its hits.
	if( bHitWhole ) pCorrosive->ClearEntityHitList();
	else m_bHitWhole = FALSE;
	if( pTarget && pTarget->IsInPieces() ) {
		pCorrosive->SetSwatDamageRadiusMul(2.0f);
		if( !bSwat ) CCorrosiveCombat::SetPreferredMeleeAttack(CCorrosiveCombat::PREFERED_MELEE_STOMP);
	} else {
		pCorrosive->SetSwatDamageRadiusMul();
		CCorrosiveCombat::SetPreferredMeleeAttack(CCorrosiveCombat::PREFERED_MELEE_SWIPE);
	}
	return FALSE;
}
#endif

BOOL CColiseumMiniGame::HandleEndCutscene()
{
	BOOL bRet = FALSE;
	s32 nScenePlayer = 0;
#if FANG_WINGC
	if( MultiplayerMgr.IsLocalCoop() ) nScenePlayer = _nPortCorrosiveWinner;
#endif
	CPlayer *pScenePlayer = &Player_aPlayer[nScenePlayer];
		
	if (m_fCutSceneTime == 0.0f) //Init the cutscene
	{
		//Turn off collisions with Glitch in peices to avoid being destroyed.
		//CBotGlitch::EnableCollisionWithPieces(FALSE);
		FASSERT( pScenePlayer->m_pEntityCurrent->TypeBits() & ENTITY_BIT_BOT );
		((CBot*)pScenePlayer->m_pEntityCurrent)->EnableCollisionWithPieces( FALSE );

		((CBotGlitch *)m_pFriendly[0])->SetArmorModifier(1.0f);
		//Override Glitch's time in pieces to avoid him getting up in the middle of the cutscene.
		CBotGlitch *pGlitch = (CBotGlitch *)m_pFriendly[0];
		pGlitch->OverrideTimeInPieces(-1.0f);
		//go into letterbox mode... (same code as game begin cutscene)
		pScenePlayer->DisableEntityControl();
#if FANG_WINGC
		if( MultiplayerMgr.IsLocalCoop() ) {
			for( s32 n = 0; n < CPlayer::m_nPlayerCount; ++n ) Player_aPlayer[n].DisableEntityControl();
		}
#endif
		CAIBrain* pBrain = pScenePlayer->m_pEntityCurrent->AIBrain();
		if (pScenePlayer->m_pEntityCurrent->TypeBits() & ENTITY_BIT_BOT)
		{
			((CBot*)pScenePlayer->m_pEntityCurrent)->HeadStopLook();
		}
		aibrainman_Activate(pBrain);
		ai_NotifyCutSceneBegin();
		CBot::DisableBotDamageGlobally();
		CBot::SetCutscenePlaying( TRUE );
		game_EnterLetterbox( "EndColiseum4", TRUE, FALSE );

		//Make Corrosive Roar victoriously
		CCorrosiveCombat::FreeToFireRockets(FALSE);
		CCorrosiveCombat::SetRoarThisFrame(TRUE);

		//Make Crowd go crazy (yes its called twice on purpose, hopefully having 2 different cheer sounds playing at the same time will make it seem
		//like the crowd is in more of a frenzy.
		PlayCheer(CHEER_MAX);
		PlayCheer(CHEER_MAX);

		//Setup desired camera loc&direction.
		CFCamera *pCamera;

		//Get my camera info.
		fcamera_Work();
		pCamera = gamecam_GetActiveCamera();
		CFMtx43A mtxCamera = pCamera->GetFinalXfm()->m_MtxR;
		CFMtx43A mtxCameraF = pCamera->GetFinalXfm()->m_MtxF;
		m_qCamera.BuildQuat( mtxCamera );
						
		m_CamManSimpleInfo.m_Pos_WS = mtxCamera.m_vPos;
		m_CamManSimpleInfo.m_Quat_WS = m_qCamera;

		//Desired direction and location.
		m_vDesDir = mtxCameraF.m_vFront;
		if (m_vDesDir.y > 0.0f) { m_vDesDir.y = -m_vDesDir.y; }

		if (m_vDesDir.y > -0.80f) { m_vDesDir.y = -0.80f; }

		f32 fCenOffsX = mtxCamera.m_vPos.x + 80.0f;
		f32 fCenOffsZ = mtxCamera.m_vPos.z - 350.0f;

		if (fCenOffsX*m_vDesDir.x < 0.0f) { m_vDesDir.x = -m_vDesDir.x; }
		if (fCenOffsZ*m_vDesDir.z < 0.0f) { m_vDesDir.z = -m_vDesDir.z; }

		m_vDesDir.Unitize();

		m_vDesLoc.x = mtxCamera.m_vPos.x - m_vDesDir.x*200.0f;
		m_vDesLoc.y = mtxCamera.m_vPos.y - m_vDesDir.y*200.0f;
		m_vDesLoc.z = mtxCamera.m_vPos.z - m_vDesDir.z*200.0f;

		m_vCurLoc = mtxCamera.m_vPos;
		m_vStartLoc = mtxCamera.m_vPos;

		gamecam_SwitchPlayerToSimpleCamera( GAME_CAM_PLAYER_1, &m_CamManSimpleInfo );

		CCorrosiveCombat::FreeToFireRockets(FALSE);
		CCorrosiveCombat::SetLaughThisFrame(TRUE);

		m_nExcitedLevel = 100;
		UpdateCrowdAnim();
	}
	else
	{
		//Move my camera toward my target location and orientation.
		f32 fU = (m_fCutSceneTime)/(m_fCutSceneLength[ m_nLevelID ]*0.5f);
		if (fU > 1.0f) { fU = 1.0f; }

		m_vCurLoc.x = m_vStartLoc.x + fU*( m_vDesLoc.x - m_vStartLoc.x );
		m_vCurLoc.y = m_vStartLoc.y + fU*( m_vDesLoc.y - m_vStartLoc.y );
		m_vCurLoc.z = m_vStartLoc.z + fU*( m_vDesLoc.z - m_vStartLoc.z );

		CFVec3A vOffs;
		vOffs = m_pFriendly[0]->GetMesh()->GetBoundingSphere().m_Pos - m_vCurLoc.v3;
		vOffs.Unitize();
		
		m_VXfm.BuildLookatFromDirVec(m_vCurLoc.v3, vOffs.v3, CFVec3(0, 1, 0));
		m_qCamera.BuildQuat(m_VXfm.m_MtxR);
		
		m_CamManSimpleInfo.m_Pos_WS = m_vCurLoc;
		m_CamManSimpleInfo.m_Quat_WS = m_qCamera;
		gamecam_SwitchPlayerToSimpleCamera( GAME_CAM_PLAYER_1, &m_CamManSimpleInfo );

		((CBotCorrosive*)m_pEnemies[0])->RequestBehavior(CBotCorrosive::BEHAVIOR_STATE_BEAT_CHEST_AND_LAUGH);

		if( Gamepad_aapSample[0][GAMEPAD_MAIN_PAUSE]->uLatches & GAMEPAD_BUTTON_1ST_PRESS_MASK )
		{
			//Abort the cutscene.
			m_fCutSceneTime = m_fCutSceneLength[ m_nLevelID ] + 1.0f;
		}
	}

	m_fCutSceneTime += FLoop_fPreviousLoopSecs;

	if (m_fCutSceneTime > m_fCutSceneLength[ m_nLevelID ]) //The cutscene is over, end the level.
	{
		bRet = TRUE;

		ai_NotifyCutSceneEnd();
		CBot::EnableBotDamageGlobally();
		CBot::SetCutscenePlaying( FALSE );
	}
	
	return (bRet);
}

void CColiseumMiniGame::EndGame()
{
	//Stop the slicers sounds.
	if (m_pSoundEmitters[COLISEUMSND_COMMON_BLADE])
	{
		m_pSoundEmitters[COLISEUMSND_COMMON_BLADE]->Stop(FALSE);
	}
	if (m_pSoundEmitters[COLISEUMSND_COMMON_MAGNET])
	{
		m_pSoundEmitters[COLISEUMSND_COMMON_MAGNET]->Stop(FALSE);
	}
	
	//Update devices - magnetic
	if (m_nMagnetDevices)
	{
		s32 i;
		for (i=0; i<m_nMagnetDevices; i++)
		{
			m_apMagnetDevice[i]->Disable();
		}
	}
    	
	if ( m_nScore >= SCORE_MINIMUM )
	{
		m_nExcitedLevel = 100;
		UpdateCrowdAnim();

		if ( m_nLevelID == 3 )
		{
			//Corrosive isn't happy that Glitch won.
			CCorrosiveCombat::SetRoarThisFrame(TRUE);
		}

		//restore players inventory, so that at the end of all Coliseums the player has his/her original inventory items/weapons.
		CPlayer::SetSaveInventory(&m_InvCopy);
#if FANG_WINGC
		_PortRestorePartners();
#endif

		//continue to next level
	#if _USE_SCRIPT_EVENT
		s32 nEventID = CFScriptSystem::GetEventNumFromName("movie");
		if (nEventID > -1 || m_nLevelID == 3)
		{
			//Fire off script event, which is responsible for ending the level.
			CFScriptSystem::TriggerEvent(nEventID, 0, 0, 0);
		}
		else
		{
			//No script event is found (or this is Coliseum4), go to the backup plan: end the level ourselves.
			CPlayer::m_pCurrent->DisableEntityControl();
			CBot::DisableBotDamageGlobally();

			game_GotoLevel( "next" );
		}
	#else
		CPlayer::m_pCurrent->DisableEntityControl();
		CBot::DisableBotDamageGlobally();

		game_GotoLevel( "next" );
	#endif
	}
	else
	{
		//I don't have enough points to continue, restart the level.
		checkpoint_Restore( 0, FALSE, "coliseum:restart" );
	}
}

void CColiseumMiniGame::UpdateCrowdAnim()
{
	s32 i, j, nIdx;
	BOOL bActive0=FALSE, bActive1=FALSE, bActive2=FALSE;

	if (m_nNumFakeCrowd < 1) { return; }
	if (m_nNumFrontCrowd < 1) { return; }
	
	//Update the crowd animations here
	for (i=0; i<m_nNumFakeCrowd; i++)
	{
		for (j=0; j<5; j++)
		{
			s32 nTexLayer = m_pFakeCrowd[i]->GetTexLayerHandle(j);
			if (m_pCrowdTex && nTexLayer > -1)
			{
				nIdx = (i*5) + j;
				s32 nTexIdx=0;
			
				if ( (s32)m_nExcitedLevel + m_anCrowdOffs[nIdx] >= ECSTATIC_MIN )
				{
					nTexIdx = ROW_2;
					bActive2 = TRUE;
				}
				else if ( (s32)m_nExcitedLevel + m_anCrowdOffs[nIdx] >= EXCITED_MIN )
				{
					nTexIdx = ROW_1;
					bActive1 = TRUE;
				}
				else
				{
					nTexIdx = 0;
					bActive0 = TRUE;
				}

				nTexIdx += m_auCrowdIdx[nIdx];
				m_pFakeCrowd[i]->TexFlip_SetSingleTexture(nTexLayer, m_pCrowdTex[ nTexIdx ]);
			}
		}
	}
	
	BOOL bActive;
	for (i=0; i<MAX_CROWD_TEMPLATE; i++)
	{
		if (i < ROW_1) { bActive = bActive0; }
		else if (i < ROW_2) { bActive = bActive1; }
		else { bActive = bActive2; }
		
		ftex_ActivateRenderTarget( m_pCrowdTex[i], bActive );
	}
}
