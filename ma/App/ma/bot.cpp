//////////////////////////////////////////////////////////////////////////////////////
// bot.cpp - Bot module.
//
// Author: Steve Ranck     
//////////////////////////////////////////////////////////////////////////////////////
// THIS CODE IS PROPRIETARY PROPERTY OF SWINGIN' APE STUDIOS, INC.
// Copyright (c) 2001
//
// The contents of this file may not be disclosed to third
// parties, copied or duplicated in any form, in whole or in part,
// without the prior written permission of Swingin' Ape Studios, Inc.
//////////////////////////////////////////////////////////////////////////////////////
// Modification History:
//
// Date     Who         Description
// -------- ----------  --------------------------------------------------------------
// 02/18/02 Ranck       Created.
//////////////////////////////////////////////////////////////////////////////////////
#include "fang.h"
#if defined(MA_PC_INPUT)
#include "pc_input.h"
#endif
#include "bot.h"

#include "fanim.h"
#include "FScriptSystem.h"
#include "ftext.h"
#include "fworld_coll.h"
#include "fclib.h"
#include "fresload.h"
#include "frenderer.h"
#include "FCheckPoint.h"
#include "fwire.h"
#include "fperf.h"

#include "alarmsys.h"
#include "bartersystem.h"
#include "BotTalkInst.h"
#include "botfx.h"
#include "cambot.h"
#include "gamecam.h"
#include "gamepad.h"
#include "gstring.h"
#include "hud2.h"
#include "item.h"
#include "itemrepository.h"
#include "MAScriptTypes.h"
#include "meshentity.h"
#include "meshtypes.h"
#include "player.h"
#include "pausescreen.h"
#include "protrack.h"
#include "TalkSystem2.h"
#include "weapon.h"
#include "weapons.h"
#include "weapon_chaingun.h"
#include "weapon_staff.h"
#include "zipline.h"
#include "botpart.h"
#include "game.h"
#include "fsound.h"
#include "botglitch.h"
#include "vehiclerat.h"
#include "eshield.h"
#include "MultiplayerMgr.h"
#include "botaagun.h"
#include "botcorrosive.h"

#include "ColiseumMiniGame.h"

#include "AI\AiApi.h"
#include "AI\AIBrainMan.h"
#include "AI\AIBTATable.h"
#include "AI\AIEnviro.h"
#include "AI\AIGameUtils.h"
#include "AI\AIBrain.h"
#include "user_elliott.h"


#define _SCOPE_ZOOM_CANCEL_HITPOINT_THRESH			0.1f
#define _SCOPE_ZOOM_CANCEL_NO_HITPOINT_SECS			0.1f
#define _SCOPE_ZOOM_CANCEL_NO_HITPOINT_DELTA		0.005f

#define _TEXTURE_STATIC								"TFH1static1"
#define _TEXTURE_GLOW								"TEM_glow01"

#define _SOUND_GROUP_PULL_THE_PLUG					"TethUnplug"

#define _START_STATIC_DIST_FROM_LIMIT				BOT_MIN_TETHER_DIST
#define _STATIC_HYSTERISIS_DIST						5.0f
#define _STATIC_UNIT_BASE							0.2f
#define _STATIC_BASE_DECAY_RATE						1.0f
#define _STATIC_SURGE_TIME							0.5f
#define _STATIC_SURGE_UNIT_AMP						0.8f
#define _STATIC_TEXTURE_SCALE						1.5f

#define _POSSESSION_OUTOFRANGE_TBOX_WIDTH2			0.42f
#define _POSSESSION_OUTOFRANGE_TBOX_HEIGHT			0.04f
#define _POSSESSION_OUTOFRANGE_BORDER_THICKNESS		0.005f
#define _POSSESSION_OUTOFRANGE_FONT_CODE			'3'
#define _POSSESSION_OUTOFRANGE_WARNING_FREQ			1.0f

#define _POSSESSION_SIGNALLOST_TBOX_WIDTH			0.42f
#define _POSSESSION_SIGNALLOST_TBOX_HEIGHT			0.04f
#define _POSSESSION_SIGNALLOST_BORDER_THICKNESS		0.005f
#define _POSSESSION_SIGNALLOST_FONT_CODE			'3'

#define _PANIC_MAX_POSDELTA_PER_SEC					8.0f			   //multiplied by the frame time to get the frame-by-frame delta by which the fUnitPanic can change
#define _PANIC_MAX_NEGDELTA_PER_SEC					3.0f			   //multiplied by the frame time to get the frame-by-frame delta by which the fUnitPanic can change
#define _ALERT_MAX_DELTA_PER_SEC					5.0f			   //multiplied by the frame time to get the frame-by-frame delta by which the fUnitAlert can change
#define _HOP_DELTA_PER_SEC							10.0f		
#define _WAKE_DELTA_PER_SEC							10.0f				//how fast the wakup is blended in as it is playing
#define _UASLOTABORTLOCK_DELTA_PER_SEC				1.0f				//how fast to blend out user anims while aborting

#define _HALF_XLAT_SPEED_WHEN_IN_ZOOM_MODE			FALSE

#define _BOT_WEIGHT_LIGHT							"light"
#define _BOT_WEIGHT_MEDIUM							"medium"
#define _BOT_WEIGHT_HEAVY							"heavy"
#define _BOT_WEIGHT_CORROSIVE						"Corrosive"
#define _BOT_WEIGHT_ZOMBIEBOSS						"ZombieBoss"
#define _BOT_WEIGHT_VEHICLE							"Vehicle"
#define _SOUND_BANK_LIGHT_WEIGHT					"Bots_Light"
#define _SOUND_BANK_MEDIUM_WEIGHT					"Bots_Med"
#define _SOUND_BANK_HEAVY_WEIGHT					"Bots_Heavy"
#define _SOUND_BANK_CORROSIVE_WEIGHT				"Corrosive"
#define _SOUND_BANK_ZOMBIEBOSS_WEIGHT				"ZombieBoss"
#define _SOUND_BANK_VEHICLE_WEIGHT					"Vehicle"
#define _SOUND_NAMES_LIGHT_CSV						"snd_light.csv"
#define _SOUND_NAMES_MEDIUM_CSV						"snd_medium.csv"
#define _SOUND_NAMES_HEAVY_CSV						"snd_heavy.csv"
#define _SOUND_NAMES_CORROSIVE_CSV					"snd_corr.csv"
#define _SOUND_NAMES_ZOMBIEBOSS_CSV					"snd_zboss.csv"
#define _SOUND_NAMES_VEHICLE_CSV					"snd_vehicle.csv"
#define _SOUND_VOLUME_SCALE							0.25f
#define _SOUND_VOLUME_MIN							0.5f
#define _SOUND_VOLUME_MAX							1.0f

#define _IDLE_TABLE_NAME							"Idles"
#define _IDLE_BLENDOUT_TIME							0.25f
#define _IDLE_OO_BLENDOUT_TIME						1.0f / _IDLE_BLENDOUT_TIME

#define _PLAYER_SELF_DAMAGE_MODIFIER				0.1f						// the percent of damage a player owned bot can inflict on itself

// respawn
#define _RESPAWN_MESH								( "GFD_respwn1" )
#define _RESPAWN_PARTICLE							( "PRD_respwn1" )
#define _RESPAWN_TIME								( 1.0f )
#define _BOT_RESPAWN_FADEOUT_TIME					( 1.0f )
#define _BOT_RESPAWN_BLENDOUT_TIME					( 0.25f )

#define _ROT_IMPACT_DUR								0.125f


#if FANG_PLATFORM_XB
	extern BOOL fxbforce_DecayCoarse( u32 nDeviceIndex, f32 fSecs, f32 fUnitIntensity, FForceHandle_t *phDestHandle=NULL, FForceDomain_e nDomain=FFORCE_DOMAIN_NORMAL );
	extern BOOL fxbforce_DecayFine( u32 nDeviceIndex, f32 fSecs, f32 fUnitIntensity, FForceHandle_t *phDestHandle=NULL, FForceDomain_e nDomain=FFORCE_DOMAIN_NORMAL );
#endif




//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CBotBuilder
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

static CBotBuilder *_pBotBuilder = NULL;// Starich - made dynamically allocated instead of static
static BOOL _bPlayedSoundThisJerk = FALSE;


void CBotBuilder::SetDefaults( u64 nEntityTypeBits, u64 nEntityLeafTypeBit, cchar *pszEntityType ) {
	ENTITY_BUILDER_SET_PARENT_CLASS_DEFAULTS( CEntityBuilder, ENTITY_BIT_BOT, pszEntityType );

	// All bots detonate projectiles by default...
	m_nProjectileReaction = CEntity::PROJECTILE_REACTION_HURT_ME;

	m_pBotDef = NULL;
	m_nOwnerPlayerIndex = -1;
	m_bDataPortInstalled = FALSE;
	m_bDataPortOpen = FALSE;

	m_fDisguiseTime = -1.0f;
	m_fPossessionDist = -1.0f;
	m_bSleepAtStart = FALSE;
	m_bSetBotFlag_PowerDownUntilPosessed = FALSE;
	m_fRunMultiplier = 1.0f;
	m_bBabble = FALSE;

	m_pszTalkDataFileName = NULL;
	m_pBuilderTalkInst = NULL;
	m_uBuilderTalkInstFlags = 0;
	m_bBabble = FALSE;

	// By default, bots are trippers...
	m_nEC_Flags |= CEntity::ENTITY_FLAG_TRIPPER;

	m_uMeshVersionOverride = 0;
	m_pszMeshReplacement = NULL;
	m_nInvincibleLimbCodeBitMask = 0;

	m_anNPCWeaponType[0] = _NPC_WEAPON_LASER;			// what will weapon 0 be?
	m_anNPCWeaponType[1] = _NPC_WEAPON_HAND;			// what will weapon 1 be?
	m_uFlags = 0;

	if( m_nEC_LeafTypeBit & ENTITY_BIT_BOTZOM ) {
		// Default for ZombieBots is to sleep at start...
		m_bSleepAtStart = TRUE;
	}
}



BOOL CBotBuilder::PostInterpretFixup( void ) {
	if( m_pszAIBuilderName == NULL ) {
		m_pszAIBuilderName = "Default";
	}

	if( m_pszMeshReplacement && !(m_pszMeshReplacement = gstring_Main.AddString( m_pszMeshReplacement)) ) {
		// Coulnd't fit that string into the global table...
		goto _YOutOfMemory;
	}

	return CEntityBuilder::PostInterpretFixup();

_YOutOfMemory:
	CEntityParser::Error_OutOfMemory();
	return FALSE;
}


BOOL CBotBuilder::InterpretTable( void ) {
	FGameData_VarType_e nVarType;
	const void *pData;
	cchar *pszString;

	if( !fclib_stricmp( CEntityParser::m_pszTableName, "TalkData" ) ) {

		CEntityParser::Interpret_String(&m_pszTalkDataFileName);

		if( (m_pszTalkDataFileName != NULL) && (m_pBuilderTalkInst == NULL) ) {
			m_pBuilderTalkInst = fnew CBotTalkInst;

			if( m_pBuilderTalkInst == NULL ) {
				DEVPRINTF("CBotBuilder::InterpretTable() : Out of memory.\n");
				return FALSE;
			}
		}

		return TRUE;

	} else if( !fclib_stricmp( CEntityParser::m_pszTableName, "TalkInst_HighPri" ) ) {

		CEntityParser::Interpret_Flag( &m_uBuilderTalkInstFlags, BOTTALKINSTFLAG_HIGHPRIORITY );

		return TRUE;

	} else if( !fclib_stricmp( CEntityParser::m_pszTableName, "TalkInst_GiveChip" ) ) {

		CEntityParser::Interpret_Flag( &m_uBuilderTalkInstFlags, BOTTALKINSTFLAG_GIVECHIP );

		return TRUE;

	} else if( !fclib_stricmp( CEntityParser::m_pszTableName, "TalkInst_TriggerEventStart" ) ) {


		CEntityParser::Interpret_Flag( &m_uBuilderTalkInstFlags, BOTTALKINSTFLAG_TRIGGEREVENTSTART );

		return TRUE;

	} else if( !fclib_stricmp( CEntityParser::m_pszTableName, "TalkInst_TriggerEventEnd" ) ) {

		CEntityParser::Interpret_Flag( &m_uBuilderTalkInstFlags, BOTTALKINSTFLAG_TRIGGEREVENTEND );

		return TRUE;

	} else if( !fclib_stricmp( CEntityParser::m_pszTableName, "DataPort" ) ) {

		if( CEntityParser::Interpret_String( &pszString ) ) {
			if( !fclib_stricmp( pszString, "None" ) ) {
				m_bDataPortInstalled = FALSE;
				m_bDataPortOpen = FALSE;
			} else if( !fclib_stricmp( pszString, "Close" ) || !fclib_stricmp( pszString, "Closed" ) ) {
				m_bDataPortInstalled = TRUE;
				m_bDataPortOpen = FALSE;
			} else if( !fclib_stricmp( pszString, "Open" ) || !fclib_stricmp( pszString, "Opened" ) ) {
				m_bDataPortInstalled = TRUE;
				m_bDataPortOpen = TRUE;
			} else {
				CEntityParser::Error_InvalidParameterValue();
			}
		}

		return TRUE;

	} else if( !fclib_stricmp( CEntityParser::m_pszTableName, "PossessDist" ) ) {

		if( CEntityParser::m_nFieldCount != 1 ) {
			CEntityParser::Error_InvalidParameterCount();
			return TRUE;
		}

		pData = fgamedata_GetPtrToFieldData( CEntityParser::m_hTable, 0, nVarType );

		switch( nVarType ) {
		case FGAMEDATA_VAR_TYPE_STRING:
			pszString = (cchar *)pData;

			if( !fclib_stricmp( pszString, "BotClassCSV" ) ) {
				m_fPossessionDist = -1.0f;
			} else {
				CEntityParser::Error_InvalidParameterValue();
			}

			return TRUE;

		case FGAMEDATA_VAR_TYPE_FLOAT:
			CEntityParser::Interpret_F32( &m_fPossessionDist, BOT_MIN_TETHER_DIST, 100000.0f, TRUE );
			return TRUE;

		default:
			FASSERT_NOW;
			return TRUE;
		}

	} else if( !fclib_stricmp( CEntityParser::m_pszTableName, "DisguiseTime" ) ) {

		if( CEntityParser::m_nFieldCount != 1 ) {
			CEntityParser::Error_InvalidParameterCount();
			return TRUE;
		}

		pData = fgamedata_GetPtrToFieldData( CEntityParser::m_hTable, 0, nVarType );

		switch( nVarType ) {
		case FGAMEDATA_VAR_TYPE_STRING:
			pszString = (cchar *)pData;

			if( !fclib_stricmp( pszString, "BotClassCSV" ) ) {
				m_fDisguiseTime = -1.0f;
			} else {
				CEntityParser::Error_InvalidParameterValue();
			}

			return TRUE;

		case FGAMEDATA_VAR_TYPE_FLOAT:
			CEntityParser::Interpret_F32( &m_fDisguiseTime, 0.0f, 100000.0f, TRUE );
			return TRUE;

		default:
			FASSERT_NOW;
			return TRUE;
		}
	} else if( !fclib_stricmp( CEntityParser::m_pszTableName, "MeshVersion" ) ) {
		CEntityParser::Interpret_U32( &m_uMeshVersionOverride, 0, 100, FALSE );
		return TRUE;
	} else if( !fclib_stricmp( CEntityParser::m_pszTableName, "MeshReplace" ) ) {
		CEntityParser::Interpret_String(&m_pszMeshReplacement);
		return TRUE;
	} else if( !fclib_stricmp( CEntityParser::m_pszTableName, "Sleeping" ) ) {
		cchar *pszValue;
		CEntityParser::Interpret_String(&pszValue);

		if( !fclib_stricmp(pszValue, "TRUE") || !fclib_stricmp(pszValue, "ON") || !fclib_stricmp(pszValue, "YES") ) {
			m_bSleepAtStart = TRUE;
		} else if(!fclib_stricmp(pszValue, "FALSE") || !fclib_stricmp(pszValue, "OFF") || !fclib_stricmp(pszValue, "NO")) {
			m_bSleepAtStart = FALSE;
		} else {
			DEVPRINTF("CAIBuilder::InterpretTable() : Invalid value '%s' for 'Sleeping' table. Use (TRUE, FALSE).\n", pszValue);
			return TRUE;
		}

		return TRUE;
	} else if( !fclib_stricmp( CEntityParser::m_pszTableName, "PowerDownUntilPosessed" ) ) {
		cchar *pszValue;
		CEntityParser::Interpret_String(&pszValue);

		if( !fclib_stricmp(pszValue, "TRUE") || !fclib_stricmp(pszValue, "ON") || !fclib_stricmp(pszValue, "YES") ) {
			m_bSetBotFlag_PowerDownUntilPosessed = TRUE;
		} else if(!fclib_stricmp(pszValue, "FALSE") || !fclib_stricmp(pszValue, "OFF") || !fclib_stricmp(pszValue, "NO")) {
			m_bSetBotFlag_PowerDownUntilPosessed = FALSE;
		} else {
			DEVPRINTF("CAIBuilder::InterpretTable() : Invalid value '%s' for 'PowerDownUntilPosessed' table. Use (TRUE, FALSE).\n", pszValue);
			return TRUE;
		}

		return TRUE;
	} else if (!fclib_stricmp( CEntityParser::m_pszTableName, "RunMultiplier")) {
	} else if (!fclib_stricmp( CEntityParser::m_pszTableName, "RunMultiplier")) {
		CEntityParser::Interpret_F32( &m_fRunMultiplier, 0.75f, 100.0f, TRUE );
		return TRUE;
	} else if( !fclib_stricmp( CEntityParser::m_pszTableName, "NPCWeapon0" ) ) {
		cchar *pszString = NULL;
		if( CEntityParser::Interpret_String( &pszString ) ) {
			if (!fclib_stricmp( pszString, "HAND")) {
				m_anNPCWeaponType[0] = _NPC_WEAPON_HAND;
			} else if (!fclib_stricmp( pszString, "LASER")) {
				m_anNPCWeaponType[0] = _NPC_WEAPON_LASER;
			} else if (!fclib_stricmp( pszString, "SPEW")) {
				m_anNPCWeaponType[0] = _NPC_WEAPON_SPEW;
			} else if (!fclib_stricmp( pszString, "FLAMER")) {
				m_anNPCWeaponType[0] = _NPC_WEAPON_FLAMER;
			} else if (!fclib_stricmp( pszString, "BLASTER")) {
				m_anNPCWeaponType[0] = _NPC_WEAPON_BLASTER;
			} else if (!fclib_stricmp( pszString, "ROCKET")) {
				m_anNPCWeaponType[0] = _NPC_WEAPON_ROCKET;
			} else if (!fclib_stricmp( pszString, "RIVET")) {
				m_anNPCWeaponType[0] = _NPC_WEAPON_RIVET;
			} else {
				CEntityParser::Error_InvalidParameterValue();
			}
		}

		return TRUE;

	} else if( !fclib_stricmp( CEntityParser::m_pszTableName, "KeepLimb" ) ) {
		// KeepLimb Head | Torso | Arms | Legs | | Wings | RL | Light | Misc | All

		cchar *pszLimbString = NULL;

		if( CEntityParser::Interpret_String( &pszString ) ) {
			if( !fclib_stricmp( pszString, "Head" ) ) {
				m_nInvincibleLimbCodeBitMask |= (1 << CBot::LIMB_CODE_HEAD);
			} else if( !fclib_stricmp( pszString, "Torso" ) ) {
				m_nInvincibleLimbCodeBitMask |= (1 << CBot::LIMB_CODE_TORSO);
			} else if( !fclib_stricmp( pszString, "Arms" ) ) {
				m_nInvincibleLimbCodeBitMask |= (1 << CBot::LIMB_CODE_ARMS);
			} else if( !fclib_stricmp( pszString, "Legs" ) ) {
				m_nInvincibleLimbCodeBitMask |= (1 << CBot::LIMB_CODE_LEGS);
			} else if( !fclib_stricmp( pszString, "Wings" ) ) {
				m_nInvincibleLimbCodeBitMask |= (1 << CBot::LIMB_CODE_WINGS);
			} else if( !fclib_stricmp( pszString, "RL" ) ) {
				m_nInvincibleLimbCodeBitMask |= (1 << CBot::LIMB_CODE_ROCKET_LAUNCHER);
			} else if( !fclib_stricmp( pszString, "Light" ) ) {
				m_nInvincibleLimbCodeBitMask |= (1 << CBot::LIMB_CODE_SPOTLIGHT);
			} else if( !fclib_stricmp( pszString, "Misc" ) ) {
				m_nInvincibleLimbCodeBitMask |= (1 << CBot::LIMB_CODE_MISC);
			} else if( !fclib_stricmp( pszString, "All" ) ) {
				m_nInvincibleLimbCodeBitMask |= ( (1 << CBot::LIMB_CODE_HEAD) |
												  (1 << CBot::LIMB_CODE_TORSO) |
												  (1 << CBot::LIMB_CODE_ARMS) |
												  (1 << CBot::LIMB_CODE_LEGS) |
												  (1 << CBot::LIMB_CODE_WINGS) |
												  (1 << CBot::LIMB_CODE_ROCKET_LAUNCHER) |
												  (1 << CBot::LIMB_CODE_SPOTLIGHT) |
												  (1 << CBot::LIMB_CODE_MISC) );
			} else {
				CEntityParser::Error_InvalidParameterValue();
			}
		}

		return TRUE;

	} else if( !fclib_stricmp( CEntityParser::m_pszTableName, "NPCWeapon1" ) ) {

		cchar *pszString = NULL;
		if( CEntityParser::Interpret_String( &pszString ) ) {
			if (!fclib_stricmp( pszString, "HAND")) {
				m_anNPCWeaponType[1] = _NPC_WEAPON_HAND;
			} else if (!fclib_stricmp( pszString, "GRENADE")) {
				m_anNPCWeaponType[1] = _NPC_WEAPON_GRENADE;
			} else {
				CEntityParser::Error_InvalidParameterValue();
			}
		}

		return TRUE;

	} else if( !fclib_stricmp( CEntityParser::m_pszTableName, "SpotLightOn" ) ) {
		CEntityParser::Interpret_Flag( &m_uFlags, BOT_BUILDER_FLAG_SPOTLIGHT_ON );

		return TRUE;

#if FANG_WINGC
	} else if( !fclib_stricmp( CEntityParser::m_pszTableName, "DisableVelocityImpulses" ) ) {
		CEntityParser::Interpret_Flag( &m_uFlags, BOT_BUILDER_FLAG_DISABLE_VELOCITY_IMPULSES );

		return TRUE;
#endif

	} else if( !fclib_stricmp( CEntityParser::m_pszTableName, "CanBeRecruited" ) ) {
		CEntityParser::Interpret_Flag( &m_uFlags, BOT_BUILDER_INST_CANNOT_BE_RECRUITED, TRUE );

		return TRUE;
	
	} else if( !fclib_stricmp( CEntityParser::m_pszTableName, "Babble" ) ) {
		cchar *pszValue;
		CEntityParser::Interpret_String(&pszValue);
	
		if(!fclib_stricmp(pszValue, "TRUE") || !fclib_stricmp(pszValue, "ON") || !fclib_stricmp(pszValue, "YES")) {
			m_bBabble = TRUE;
		} else if(!fclib_stricmp(pszValue, "FALSE") || !fclib_stricmp(pszValue, "OFF") || !fclib_stricmp(pszValue, "NO")) {
			m_bBabble = FALSE;
		} else {
			DEVPRINTF("CAIBuilder::InterpretTable() : Invalid value '%s' for 'Babble' table. Use (TRUE, FALSE).\n", pszValue);
			return TRUE;
		}

		return TRUE;
	}

	return CEntityBuilder::InterpretTable();
}

CWeapon* CBotBuilder::AllocNPCWeaponOfType( NPCWeapon_e nNPCWeaponType ) {
	CWeapon* pWeap = NULL;

	switch( nNPCWeaponType ) {
    case _NPC_WEAPON_HAND:
		pWeap = fnew CWeaponHand();
		break;
	case _NPC_WEAPON_LASER:
		pWeap = fnew CWeaponLaser();
		break;
	case _NPC_WEAPON_SPEW:
		pWeap = fnew CWeaponSpew();

		break;
	case _NPC_WEAPON_FLAMER:
		pWeap = fnew CWeaponFlamer();
		break;
	case _NPC_WEAPON_GRENADE:
		pWeap = fnew CWeaponGren();
		break;
	case _NPC_WEAPON_BLASTER:
		pWeap = fnew CWeaponBlaster();
		break;
	case _NPC_WEAPON_RIVET:
		pWeap = fnew CWeaponRivet();
		break;
	case _NPC_WEAPON_MAGMABOMB:
		pWeap = fnew CWeaponMagmaBomb();
		break;
	case _NPC_WEAPON_TETHER:
		pWeap = fnew CWeaponTether();
		break;
	case _NPC_WEAPON_EMP:
		pWeap = fnew CWeaponEMP();
		break;
	case _NPC_WEAPON_RECRUITER:
		pWeap = fnew CWeaponRecruiter();
		break;
	case _NPC_WEAPON_STAFF:
		pWeap = fnew CWeaponStaff();
		break;
	case _NPC_WEAPON_ROCKET:
		pWeap = fnew CWeaponRocket();
		break;
	}

	return pWeap;
}


BOOL CBotBuilder::CreateNPCWeaponOfType( CWeapon* pWeap, NPCWeapon_e nNPCWeaponType ) {
	BOOL bDidIt = FALSE;

	FASSERT(pWeap);

	switch( nNPCWeaponType ) {
	case _NPC_WEAPON_HAND:
		bDidIt = ((CWeaponHand*) pWeap)->Create();
		break;
	case _NPC_WEAPON_LASER:
		bDidIt = ((CWeaponLaser*) pWeap)->Create();
		break;
	case _NPC_WEAPON_SPEW:
		bDidIt = ((CWeaponSpew*) pWeap)->Create();
		break;
	case _NPC_WEAPON_FLAMER:
		bDidIt = ((CWeaponFlamer*) pWeap)->Create();
		break;
	case _NPC_WEAPON_GRENADE:
		bDidIt = ((CWeaponGren*) pWeap)->Create();
		break;
	case _NPC_WEAPON_BLASTER:
		bDidIt = ((CWeaponBlaster*) pWeap)->Create();
		break;
	case _NPC_WEAPON_RIVET:
		bDidIt = ((CWeaponRivet*) pWeap)->Create();
		break;
	case _NPC_WEAPON_MAGMABOMB:
		bDidIt = ((CWeaponMagmaBomb*) pWeap)->Create();
		break;
	case _NPC_WEAPON_TETHER:
		bDidIt = ((CWeaponTether *)pWeap)->Create();
		break;
	case _NPC_WEAPON_EMP:
		bDidIt = ((CWeaponEMP *)pWeap)->Create();
		break;
	case _NPC_WEAPON_RECRUITER:
		bDidIt = ((CWeaponRecruiter *)pWeap)->Create();
		break;
	case _NPC_WEAPON_STAFF:
		bDidIt = ((CWeaponStaff *)pWeap)->Create();
		break;
	case _NPC_WEAPON_ROCKET:
		bDidIt = ((CWeaponRocket *)pWeap)->Create();
		break;


	}

	return bDidIt;
}


//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CBot
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************
BOOL CBot::m_bDisableBotDamageGlobally = FALSE;
BOOL CBot::m_bCutscenePlaying = FALSE;
BOOL CBot::m_bAllowPlayerDeath = TRUE;
f32 CBot::m_fPossessionTerminatedPowerDownTime = 1.0f;

BOOL CBot::m_bBotSystemInitialized;
s32 CBot::m_nBotDieEvent;
CFCollInfo CBot::m_CollInfo;			// Used for collision detection
CFSphere CBot::m_CollSphere;			// Used for collision detection
CBot *CBot::m_pCollBot;					// Used for collision detection
CFVec3A CBot::m_vMoveNormal;			// Used for collision detection
f32 CBot::m_fMoveLength;				// Used for collision detection
CFMtx43A CBot::m_aTetherMtx43A[TETHER_RANDOM_MTX_COUNT];
CFTexInst CBot::m_StaticTexInst;
CFTexInst CBot::m_GlowTexInst;
u8 CBot::m_uBotSoundFlags = 0;
CBot::BotInfo_Sounds_t CBot::m_BotInfo_SoundLight;
CBot::BotInfo_Sounds_t CBot::m_BotInfo_SoundMedium;
CBot::BotInfo_Sounds_t CBot::m_BotInfo_SoundHeavy;
CBot::BotInfo_Sounds_t CBot::m_BotInfo_SoundCorrosive;
CBot::BotInfo_Sounds_t CBot::m_BotInfo_SoundZombieBoss;
CBot::BotInfo_Sounds_t CBot::m_BotInfo_SoundVehicle;
CFSoundGroup *CBot::m_pSoundGroupPullThePlug;

BOOL CBot::m_bColiseumMiniGame=FALSE;
FCollImpact_t		CBot::m_aMeleeCollisionThisFrameBuffer[MELEE_MAX_ENTITIES_PER_SWING];
u32					CBot::m_uMeleeCollisionThisFrameCount;
CBot::MeleeData_t*	CBot::m_pMeleeDataCB = NULL;


const FGameData_TableEntry_t m_aGameDataVocab_IdleGameDataInfo[] = {
	// pszIdleName
	FGAMEDATA_VAR_TYPE_STRING|
	FGAMEDATA_FLAGS_STRING_PTR_TO_MAIN_STR_TBL | FGAMEDATA_FLAGS_STRING_NONE_TO_NULL,
	sizeof( cchar * ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,
	
	// fIdleToIdleDeltaSpeed
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_65535,

	// fIdleMinimumDelay
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_65535,

	// fIdleProbability
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_1,

	// End of table:
	FGAMEDATA_VAR_TYPE_COUNT| 0, 0, F32_DATATABLE_0, F32_DATATABLE_0
};


void Bot_DeathBTAEndCB( CBotTalkInst *pTalkInst, CBot *pBot );




BOOL CBot::InitSystem( void ) {
	#define __RAND_MTX_ROT_DELTA	0.1f
	#define __RAND_MTX_XLAT_DELTA	0.1f

	u32 i;
	FTexDef_t *pTexDef;

	FASSERT( !m_bBotSystemInitialized );

	_pBotBuilder = fnew CBotBuilder;
	if( !_pBotBuilder ) {
		DEVPRINTF( "CBot::InitSystem() : Could not allocate the builder object.\n");
		return FALSE;
	}

	for( i=0; i<TETHER_RANDOM_MTX_COUNT; ++i ) {
		m_aTetherMtx43A[i].Identity();

		m_aTetherMtx43A[i].m_vRight.x = fmath_RandomFloatRange( -__RAND_MTX_ROT_DELTA, __RAND_MTX_ROT_DELTA ) + 1.0f;
		m_aTetherMtx43A[i].m_vRight.y = fmath_RandomFloatRange( -__RAND_MTX_ROT_DELTA, __RAND_MTX_ROT_DELTA );
		m_aTetherMtx43A[i].m_vRight.z = fmath_RandomFloatRange( -__RAND_MTX_ROT_DELTA, __RAND_MTX_ROT_DELTA );
		m_aTetherMtx43A[i].m_vRight.Unitize();

		m_aTetherMtx43A[i].m_vUp.x = fmath_RandomFloatRange( -__RAND_MTX_ROT_DELTA, __RAND_MTX_ROT_DELTA );
		m_aTetherMtx43A[i].m_vUp.y = fmath_RandomFloatRange( -__RAND_MTX_ROT_DELTA, __RAND_MTX_ROT_DELTA ) + 1.0f;
		m_aTetherMtx43A[i].m_vUp.z = fmath_RandomFloatRange( -__RAND_MTX_ROT_DELTA, __RAND_MTX_ROT_DELTA );
		m_aTetherMtx43A[i].m_vUp.Unitize();

		m_aTetherMtx43A[i].m_vFront.x = fmath_RandomFloatRange( -__RAND_MTX_ROT_DELTA, __RAND_MTX_ROT_DELTA );
		m_aTetherMtx43A[i].m_vFront.y = fmath_RandomFloatRange( -__RAND_MTX_ROT_DELTA, __RAND_MTX_ROT_DELTA );
		m_aTetherMtx43A[i].m_vFront.z = fmath_RandomFloatRange( -__RAND_MTX_ROT_DELTA, __RAND_MTX_ROT_DELTA ) + 1.0f;
		m_aTetherMtx43A[i].m_vFront.Unitize();

		m_aTetherMtx43A[i].m_vPos.x = fmath_RandomFloatRange( -__RAND_MTX_XLAT_DELTA, __RAND_MTX_XLAT_DELTA );
		m_aTetherMtx43A[i].m_vPos.y = fmath_RandomFloatRange( -__RAND_MTX_XLAT_DELTA, __RAND_MTX_XLAT_DELTA );
		m_aTetherMtx43A[i].m_vPos.z = fmath_RandomFloatRange( -__RAND_MTX_XLAT_DELTA, __RAND_MTX_XLAT_DELTA );
	}

	// Load static texture...
	pTexDef = (FTexDef_t *)fresload_Load( FTEX_RESNAME, _TEXTURE_STATIC );
	if( pTexDef == NULL ) {
		DEVPRINTF( "CBot::InitSystem(): Could not find texture '%s'. Possession static won't be drawn.\n", _TEXTURE_STATIC );
	}
	m_StaticTexInst.SetTexDef( pTexDef );
	m_StaticTexInst.SetFlags( CFTexInst::FLAG_WRAP_S | CFTexInst::FLAG_WRAP_T );

	// Load static glow texture...
	pTexDef = (FTexDef_t *)fresload_Load( FTEX_RESNAME, _TEXTURE_GLOW );
	if( pTexDef == NULL ) {
		DEVPRINTF( "CBot::InitSystem(): Could not find texture '%s'. Possession static glow won't be drawn.\n", _TEXTURE_GLOW );
	}
	m_GlowTexInst.SetTexDef( pTexDef );
	m_GlowTexInst.SetFlags( CFTexInst::FLAG_NONE );

#if FANG_WINGC
	// The event names come from each level's events table, loaded after this runs, so this lookup always
	// failed (and logged twice); nothing uses m_nBotDieEvent: Die() looks the event up when it fires.
	m_nBotDieEvent = -1;
#else
	m_nBotDieEvent = CFScriptSystem::GetEventNumFromName( "BotDie" );
	if( m_nBotDieEvent == -1 ) {
		DEVPRINTF( "CBot::InitSystem() : Could not find botdie event.\n");
	}
#endif

	m_uBotSoundFlags = 0;
	fang_MemZero(&m_BotInfo_SoundLight, sizeof(m_BotInfo_SoundLight));
	fang_MemZero(&m_BotInfo_SoundMedium, sizeof(m_BotInfo_SoundMedium));
	fang_MemZero(&m_BotInfo_SoundHeavy, sizeof(m_BotInfo_SoundHeavy));
	fang_MemZero(&m_BotInfo_SoundCorrosive, sizeof(m_BotInfo_SoundCorrosive));
	fang_MemZero(&m_BotInfo_SoundZombieBoss, sizeof(m_BotInfo_SoundZombieBoss));
	fang_MemZero(&m_BotInfo_SoundVehicle, sizeof(m_BotInfo_SoundVehicle));

	m_pSoundGroupPullThePlug = CFSoundGroup::RegisterGroup( _SOUND_GROUP_PULL_THE_PLUG );

	m_bBotSystemInitialized = TRUE;

	return TRUE;

	#undef __RAND_MTX_XLAT_DELTA
	#undef __RAND_MTX_DELTA
}


void CBot::UninitSystem( void ) {
	if( m_bBotSystemInitialized ) {
		m_bBotSystemInitialized = FALSE;
		fdelete( _pBotBuilder );
		_pBotBuilder = NULL;
	}
}


CBot::CBot() : CEntity() {
	_ClearDataMembers();
}


CBot::~CBot() {
	if( IsSystemInitialized() && IsCreated() ) {
		DetachFromParent();
		DetachAllChildren();
		RemoveFromWorld( TRUE );
		ClassHierarchyDestroy();
	}
}


CFMtx43A* CBot::ClassHierarchyAttachChild( CEntity *pChildEntity, cchar *pszAttachBoneName ){
	s32 nBoneIndex;
	if( pszAttachBoneName == NULL ) {
		// No bone to attach to...
		return NULL;
	}

	nBoneIndex = m_pWorldMesh->FindBone( pszAttachBoneName );

	if( nBoneIndex < 0 ) {
		DEVPRINTF( "CBot::ClassHierarchyAttachChild(): Bone '%s' doesn't exist in object '%s'.\n", pszAttachBoneName, Name() );
		return NULL;
	}

	return m_pWorldMesh->GetBoneMtxPalette()[nBoneIndex];
}


void CBot::ClassHierarchyAddToWorld( void ) {
	FASSERT( IsCreated() );
	FASSERT( !IsInWorld() );

	CEntity::ClassHierarchyAddToWorld();

	// set mesh transform to be at mount position
	m_pWorldMesh->m_Xfm.BuildFromMtx( m_MtxToWorld );
	m_pWorldMesh->UpdateTracker();

	if( m_Anim.m_pAnimCombiner != NULL )
	{
		AtRestMatrixPalette();
	}

	if( IsDead() ) {
		UnDie();
	}

	if( m_pDataPortMeshEntity ) {
		m_pDataPortMeshEntity->Attach_UnitMtxToParent_PS( this, m_pBotInfo_Gen->pszDataPortBoneName );
		m_pDataPortMeshEntity->AddToWorld();
		m_pDataPortMeshEntity->SelectMesh( !!m_bDataPortOpen );
	}

	if( m_pDataPortEffectMeshEntity && m_bDataPortOpen && (m_nPossessionPlayerIndex < 0) ) {
		m_pDataPortEffectMeshEntity->Attach_UnitMtxToParent_PS( this, m_pBotInfo_Gen->pszDataPortBoneName );
		m_pDataPortEffectMeshEntity->AddToWorld();
	}

	if( m_anAnimStackIndex[ASI_STOOP] != -1 ) {
		SetControlValue( ASI_STOOP, 0.0f );
	}

	m_pStickModifier_MS = NULL;
	m_fLastFireTime = 0.0f;

	m_Velocity_WS = CFVec3A::m_Null;
	VelocityHasChanged();

	TagPoint_Update();
}


void CBot::ClassHierarchyRemoveFromWorld( void ) {
	FASSERT( IsCreated() );
	FASSERT( IsInWorld() );

	if( m_nRecruitID >= 0 ) {
		CWeaponRecruiter::UnrecruitBot( this, FALSE );
	}

	if (GetCurMech()) {
		// If mech-driving bot gets removed from world before the mech itself does,
		// call the appropriate exit-vehicle function so that, in the case of
		// a checkpoint restore, the bot won't be in the vehicle when it restores
		// itself.  The vehicles will restore their drivers (if appropriate) during 
		// their restore.
		if (GetCurMech()->TypeBits() & ENTITY_BIT_VEHICLE) {
			((CVehicle*) GetCurMech())->ExitStation(this, TRUE /*bImmediately*/);

		} else if (GetCurMech()->TypeBits() & ENTITY_BIT_SITEWEAPON) {
			CBotSiteWeapon *pSite;
			pSite = (CBotSiteWeapon*) GetCurMech();
			pSite->SetSiteWeaponDriver(NULL, TRUE);
			FASSERT(pSite->GetDriverBot()==NULL);

		} else if( GetCurMech()->TypeBits() & ENTITY_BIT_BOTAAGUN ) {
			CBotAAGun *pGun = (CBotAAGun *)GetCurMech();
			pGun->SwiftKickOutDriver();
			FASSERT( pGun->GetDriverBot() == NULL );
		}
		FASSERT(!GetCurMech());
	}

	if( m_pDataPortMeshEntity ) {
		m_pDataPortMeshEntity->DetachFromParent();
		m_pDataPortMeshEntity->RemoveFromWorld();
	}

	if( m_pDataPortEffectMeshEntity ) {
		m_pDataPortEffectMeshEntity->DetachFromParent();
		m_pDataPortEffectMeshEntity->RemoveFromWorld();
	}

	if( m_pStickyEntity ) {
		m_pStickyEntity	= NULL;
		m_pszStickyEntityBoneName = NULL;
		DetachFromParent();
	}

	if( m_pActiveTalkInst ) {
		CTalkSystem2::TerminateActiveTalk(m_pActiveTalkInst);
		m_pActiveTalkInst = NULL;
	}

	m_nThrowState = THROWSTATE_NONE;  //pgm added this, because I noticed that if you die with a grenade in hand, then the game would crash
	m_nScopeState = SCOPESTATE_NONE;

	if (m_pDeathTalkInst && m_pDeathTalkInst->GetTalkData() ) {
		// CJM: Added to fix crash on exit, as talk pool is destroyed before entities are...

		CTalkSystem2::BTIPool_Recycle(m_pDeathTalkInst);
		m_pDeathTalkInst = NULL;
	}

	_EnablePossessionTextBox( FALSE );

	// If we have an EMP effect going on, disable it
	if( IsStunned() ) {
		StunBot( -1.0f );
	}

	// If we have an paralysis effect going on, disable it
	if( IsSlowed() ) {
		SlowBot( 0.0f );
	}

	m_pStickModifier_MS = NULL;

	// Return all of this bot's parts to the part pool...
	if( m_pPartMgr && m_pPartMgr->IsCreated() ) {
		m_pPartMgr->ResetAllToIntact();
	}

	CEntity::ClassHierarchyRemoveFromWorld();
}


void CBot::ClassHierarchyDrawEnable( BOOL bDrawingHasBeenEnabled ) {
	CEntity::ClassHierarchyDrawEnable( bDrawingHasBeenEnabled );

	if( bDrawingHasBeenEnabled ) {
		// Enable drawing of this bot...
		FMATH_CLEARBITMASK( m_pWorldMesh->m_nFlags, FMESHINST_FLAG_DONT_DRAW );
	} else {
		// Disable drawing of this bot...
		FMATH_SETBITMASK( m_pWorldMesh->m_nFlags, FMESHINST_FLAG_DONT_DRAW );
	}

	WeaponsDrawEnable( bDrawingHasBeenEnabled );

	if( m_nRecruitID >= 0 ) {
		CWeaponRecruiter::BotDrawEnable( this, bDrawingHasBeenEnabled );
	}
}


BOOL CBot::ReadBotInfoFile( const FGameDataMap_t *pMapTable, cchar *pszFileName ) {
	const FGameDataMap_t *pMap;
	BotInfo_Walk_t *pBotInfo_Walk;
	BotInfo_Weapon_t *pBotInfo_Weapon;
	BotInfo_Gen_t *pBotInfo_Gen;

	if( !fgamedata_ReadFileUsingMap( pMapTable, pszFileName ) ) {
		return FALSE;
	}

	// Game data file read successfully.
	// Fix up some special data fields...

	for( pMap=pMapTable; pMap->pszTableName; ++pMap ) {
		if( pMap->pVocabTable == m_aBotInfoVocab_Walk ) {
			pBotInfo_Walk = (BotInfo_Walk_t *)pMap->pDestTableData;

			pBotInfo_Walk->fSneakNormVelocity = pBotInfo_Walk->fSneakVelocity / pBotInfo_Walk->fMaxXlatVelocity;
			pBotInfo_Walk->fMinWalkNormVelocity = pBotInfo_Walk->fMinWalkVelocity / pBotInfo_Walk->fMaxXlatVelocity;
			pBotInfo_Walk->fMinRunBlendNormVelocity = pBotInfo_Walk->fMinRunBlendVelocity / pBotInfo_Walk->fMaxXlatVelocity;
			pBotInfo_Walk->fMaxRunBlendNormVelocity = pBotInfo_Walk->fMaxRunBlendVelocity / pBotInfo_Walk->fMaxXlatVelocity;

		} else if( pMap->pVocabTable == m_aBotInfoVocab_Weapon ) {
			pBotInfo_Weapon = (BotInfo_Weapon_t *)pMap->pDestTableData;

			pBotInfo_Weapon->fOOInvThrowBlendOutUnitTime = 1.0f / (1.0f - pBotInfo_Weapon->fThrowBlendOutUnitTime);
		} else if( pMap->pVocabTable == m_aBotInfoVocab_Gen ) {
			pBotInfo_Gen = (BotInfo_Gen_t *)pMap->pDestTableData;

			pBotInfo_Gen->pDebrisBurstImpulseDamageProfile = CDamage::FindDamageProfile( pBotInfo_Gen->pszDebrisBurstImpulseDamageProfile );
			pBotInfo_Gen->pTorsoBlowOffArmorProfile = CDamage::FindArmorProfile( pBotInfo_Gen->pszTorsoBlowOffArmorProfile );
		}
	}
	return TRUE;
}


BOOL CBot::ReadBotIdleFile( BotInfo_Idle_t** ppBotIdleArray, u32* pnBotIdleCount, cchar *pszFileName ) {
	s32 nStructElementCount, nTableFieldCount, nIdleCount;
	FGameDataTableHandle_t hIdleTable;

	// grab a frame
	FMemFrame_t hMemFrame = fmem_GetFrame();

	// Load game data file...
	FGameDataFileHandle_t hFile = fgamedata_LoadFileToFMem( pszFileName );
	if( hFile == FGAMEDATA_INVALID_FILE_HANDLE ) {
		DEVPRINTF( "CBot::ReadBotIdleFile(): Could not load file '%s.csv'.\n", pszFileName );
		goto _ExitWithError;
	}

	// Find Idles table...
	hIdleTable = fgamedata_GetFirstTableHandle( hFile, _IDLE_TABLE_NAME );
	if( hIdleTable == FGAMEDATA_INVALID_TABLE_HANDLE ) {
		DEVPRINTF( "CBot::ReadBotIdleFile(): Could not locate idle info table '%s' in file '%s.csv'.\n", _IDLE_TABLE_NAME, pszFileName );
		goto _ExitWithError;
	}
	
	nStructElementCount = sizeof( m_aGameDataVocab_IdleGameDataInfo ) / sizeof( FGameData_TableEntry_t );		
	FASSERT( nStructElementCount > 0 );
	--nStructElementCount;
	nTableFieldCount = fgamedata_GetNumFields( hIdleTable );
	if( nTableFieldCount % nStructElementCount ) {
		DEVPRINTF( "CBot::ReadBotIdleFile(): Invalid number of fields in hIdle table '%s' in file '%s.csv'.\n", _IDLE_TABLE_NAME, pszFileName );
		goto _ExitWithError;
	}
	nIdleCount = nTableFieldCount / nStructElementCount;
	*pnBotIdleCount = nIdleCount;

	*ppBotIdleArray = (BotInfo_Idle_t *)fres_Alloc( sizeof(BotInfo_Idle_t) * nIdleCount);
	if( *ppBotIdleArray == NULL ) {
		DEVPRINTF( "CBot::ReadBotIdleFile(): Not enough memory to create Idle array.\n" );
		goto _ExitWithError;
	}

	if( !fgamedata_GetTableData_ArrayOfStructures( hIdleTable, m_aGameDataVocab_IdleGameDataInfo, *ppBotIdleArray, sizeof(BotInfo_Idle_t), nIdleCount) ) {
		DEVPRINTF( "CBot::ReadBotIdleFile(): Problem encountered while parsing idle info table '%s' in file '%s.csv'.\n", _IDLE_TABLE_NAME, pszFileName );
		goto _ExitWithError;
	}

	return TRUE;

_ExitWithError:
	fmem_ReleaseFrame( hMemFrame );
	return FALSE;
}


void CBot::MakeLimbInvincible( LimbCode_e nLimbCode, u32 nLimbType, CBotBuilder *pBuilder ) {
	if( m_pPartMgr && m_pPartMgr->IsCreated() ) {
		if( pBuilder->m_nInvincibleLimbCodeBitMask & (1 << nLimbCode) ) {
			m_pPartMgr->MakeLimbInvincible( nLimbType );
		}
	}
}


// Note: We don't pass along damage results to CEntity.
void CBot::InflictDamageResult( const CDamageResult *pDamageResult ) {
	FASSERT( IsCreated() );

	BOOL bReduceDamage = FALSE;

#if 0
	if( pDamageResult->m_fDeltaHitpoints > 0.05f ) {
		AbortScopeMode();
	}
#endif

	if( MultiplayerMgr.IsSinglePlayer() ) {
		// Single-player game...

#if FANG_WINGC
		// Local co-op: partners can't hurt each other (a melee blew a partner's upper body off).
		if( (m_nPossessionPlayerIndex >= 0) && (pDamageResult->m_pDamageData->m_Damager.nDamagerPlayerIndex >= 0) &&
			(pDamageResult->m_pDamageData->m_Damager.nDamagerPlayerIndex != m_nPossessionPlayerIndex) ) {
			return;
		}
#endif

		if( pDamageResult->m_pDamageData->m_Damager.pBot ) {
			// Damage is from a bot...

			if( (pDamageResult->m_pDamageData->m_Damager.nDamagerPlayerIndex < 0) && (m_nPossessionPlayerIndex < 0) ) {
				// Both damager and damagee are NPC bots...

				if( (pDamageResult->m_pDamageData->m_Damager.pBot->m_nRecruitID >= 0) && (m_nRecruitID >= 0) ) {
					// Both bots are recruited...

					return;
				}
			}
		}

		// in single player games, if this bot is driving a RAT, and the RAT gunner is a player, then don't let this bot take damage
		if( m_pDrivingVehicle && m_pDrivingVehicle->GetDriverBot() == this && m_pDrivingVehicle->GetGunnerBot() && m_pDrivingVehicle->GetGunnerBot()->IsPlayerBot() )
		{
			volatile f32 fTemp = 0.0f;
			fTemp += pDamageResult->m_fDeltaHitpoints;
			return;
		}

	} else {
		// Multi-player game

		if( !MultiplayerMgr.FriendlyFireDamage() ) {
			// Friendly bots shouldn't damage each other

			if( pDamageResult->m_pDamageData->m_Damager.pBot && aiutils_IsFriendly( this, pDamageResult->m_pDamageData->m_Damager.pBot ) ) {
				// Both damager and damagee are friendly to each other

				return;
			}
		}
	}

	if( GetPowerupFx() && GetPowerupFx()->IsArmorActive() ) {
		return;
	}

	if( pDamageResult->m_pDamageData->m_nDamageLocale == CDamageForm::DAMAGE_LOCALE_BLAST && m_pDrivingVehicle )
	{
		//If bot is driving a vehicle, don't pass blast damage to the bot
		// since the bot will most likely receive damage from the vehicle.
		return;
	}


	if( m_pDrivingVehicle && m_pDrivingVehicle->TypeBits() & ENTITY_BIT_VEHICLERAT ) {
		CVehicleRat *pRat = (CVehicleRat*) m_pDrivingVehicle;

		#if RAT_GUN
			if( pDamageResult->m_pDamageData->m_Damager.pBot == pRat->RatGun() ) {
				// rat gunner mustn't damage rat driver
				return;
			}
		#endif
	}

	if( pDamageResult->m_pDamageData->m_Damager.pBot == this ) {
		// If it's a computer controlled bot, ignore damage...
		if( m_nPossessionPlayerIndex < 0 ) {
			return;
		}

		// It's a player controlled bot.  Adjust the damage down unless it's the original bot...
		if( m_nPossessionPlayerIndex != m_nOwnerPlayerIndex ) {
			bReduceDamage = TRUE;
		}
	}

	f32 fNormHealth = NormHealth();

	if( fNormHealth > 0.0f ) {
		f32 fDeltaHitpoints;

		if( bReduceDamage ) {
			fDeltaHitpoints = pDamageResult->m_fDeltaHitpoints * _PLAYER_SELF_DAMAGE_MODIFIER;
		} else {
			fDeltaHitpoints = pDamageResult->m_fDeltaHitpoints;
		}

		fNormHealth -= fDeltaHitpoints;

		// Handle scope resetting for player bots only...
		if( m_nPossessionPlayerIndex >= 0 ) {
			if( m_nScopeState != SCOPESTATE_NONE ) {
				m_fScopeUnzoomHitpointTimer = 0.0f;
				m_fScopeUnzoomHitpointAccumulator += fDeltaHitpoints;

				if( m_fScopeUnzoomHitpointAccumulator > _SCOPE_ZOOM_CANCEL_HITPOINT_THRESH ) {
					m_fScopeUnzoomHitpointAccumulator = 0.0f;
					AbortScopeMode();
				}
			}
		}

//		if( m_bAllowPlayerDeath ) {	   //test code to make no bots take damage
		{
			SetNormHealth( fNormHealth );
		}

		fNormHealth = NormHealth();
		if( fNormHealth > 0.0f ) {
			InformAIOfDamageResult( pDamageResult );

			if( m_pPartMgr->IsCreated() && !bReduceDamage ) {
				m_pPartMgr->InflictLimbDamage( pDamageResult->m_pDamageData, pDamageResult );
			}

			if( pDamageResult->m_fImpulseMag != 0.0f ) {
				if( m_nPossessionPlayerIndex < 0 ) {
					// AI-controlled bot...

					if( !(TypeBits() & ENTITY_BIT_VEHICLE) ) {
						if( pDamageResult->m_pDamageData->m_nDamageLocale != CDamageForm::DAMAGE_LOCALE_AMBIENT ) {
							CFVec3A BotToImpactPoint_WS, ImpulseVec_WS;
							f32 fDot, fImpulseUnitMag;

							BotToImpactPoint_WS.Sub( pDamageResult->m_pDamageData->m_ImpactPoint_WS, m_MtxToWorld.m_vPos );

							fDot = BotToImpactPoint_WS.z*pDamageResult->m_pDamageData->m_AttackUnitDir_WS.x - BotToImpactPoint_WS.x*pDamageResult->m_pDamageData->m_AttackUnitDir_WS.z;

							fImpulseUnitMag = pDamageResult->m_fImpulseMag * (1.0f / 10000.0f);
							FMATH_CLAMPMAX( fImpulseUnitMag, 1.0f );

							AddRotationalImpact( fDot * fImpulseUnitMag );

							ImpulseVec_WS.Mul( pDamageResult->m_pDamageData->m_AttackUnitDir_WS, fImpulseUnitMag * 20.0f );

							ApplyVelocityImpulse_WS( ImpulseVec_WS );
						}
					}
				}
			}
		} else {
			// Let the part manager kill the bot (unless it's glitch or vehicle). They do their own thing.
			if( m_pPartMgr->IsCreated() && !((TypeBits() & ENTITY_BIT_BOTGLITCH) || (TypeBits() & ENTITY_BIT_BOTPRED) || 
				(TypeBits() & ENTITY_BIT_BOTCORROSIVE) || (TypeBits() & ENTITY_BIT_VEHICLE) /*|| (TypeBits() & ENTITY_BIT_BOTPROBE)*/) ) {
				m_pPartMgr->DestroyBot( pDamageResult->m_pDamageData );
			} else {
				Die();
			}

			if( fNormHealth <= 0.0f ) {
				// Give the player who killed us credit for the kill...
				if( pDamageResult->m_pDamageData ) {
					s32 nPlayer = pDamageResult->m_pDamageData->m_Damager.nDamagerPlayerIndex;
					if( nPlayer >= 0 ) {
						Player_aPlayer[nPlayer].CreditKill(m_nPossessionPlayerIndex, this);
					}

					// If the damager is recruited, give the recruiter credit also
					CBot* pDamagerBot = pDamageResult->m_pDamageData->m_Damager.pBot;
					if ( pDamagerBot && pDamagerBot->Recruit_IsRecruited() ) {
						nPlayer = pDamagerBot->Recruit_GetRecruiter();
						if( nPlayer >= 0 ) {
							Player_aPlayer[nPlayer].CreditKill(m_nPossessionPlayerIndex, this);
						}
					}
				}
			}
		}
	}

	CEntity::ShakeCamera( pDamageResult );
}


void CBot::AddRotationalImpact( f32 fUnitIntensity ) {
	FASSERT( IsCreated() );

	if( fUnitIntensity == 0.0f ) {
		return;
	}

	if( fUnitIntensity > 0.0f ) {
		FMATH_CLAMPMAX( fUnitIntensity, 0.6f );

		m_fRotationalImpactDeltaYaw = FMATH_FPOT( fUnitIntensity, FMATH_DEG2RAD( 90.0f ), FMATH_DEG2RAD( 2000.0f ) );
		m_fRotationalImpactSpeedMult = m_fRotationalImpactDeltaYaw * (-1.0f / _ROT_IMPACT_DUR);
	} else {
		FMATH_CLAMPMIN( fUnitIntensity, -0.6f );

		m_fRotationalImpactDeltaYaw = -FMATH_FPOT( -fUnitIntensity, FMATH_DEG2RAD( 90.0f ), FMATH_DEG2RAD( 2000.0f ) );
		m_fRotationalImpactSpeedMult = m_fRotationalImpactDeltaYaw * (-1.0f / _ROT_IMPACT_DUR);
	}
}


//
//	 CBot::Die gets called when a bot first starts dying.
//
//	   m_uLifeCycleState  will be set to BOTLIFECYCLE_DYING and then CBot::DeathWork will start getting called every frame.
//	   Check CBot::DeathWork to see when, why and how m_uLifeCycleState is advanced to from BOTLIFECYCLE_DYING to BOTLIFECYCLE_DEAD.
//	   When BOTLIFECYCLE_DYING is reached, bots will be removed from the world, unless 
//	   they have either of (BOTDEATHFLAG_PERSISTAFTERDEATH or	BOTDEATHFLAG_AUTOPERSISTAFTERDEATH) flags in	m_uBotDeathFlags.
//
void CBot::Die( BOOL bSpawnDeathEffects/*=TRUE*/, BOOL bSpawnGoodies ) {
	if( m_nRecruitID >= 0 ) {
		CWeaponRecruiter::UnrecruitBot( this, bSpawnDeathEffects );
	}

	if( bSpawnDeathEffects ) {
		SpawnDeathEffects();
	}

	AbortScopeMode( TRUE );


	if( m_nPossessionPlayerIndex > -1 || m_pBotDef->m_nRace == BOTRACE_DROID) {
		if( !m_bAllowPlayerDeath ) {
			// This makes it so that the player bot never dies!!!
			return;
		}

		// Turn off the reticle...
		if( m_nOwnerPlayerIndex > -1 ) {
			ReticleEnable( FALSE );
		}
	}

	if( m_uLifeCycleState != BOTLIFECYCLE_LIVING ) {
		// Why die again if already dead or dying?
		return;
	}

	if( IsPlayerBot() && m_pPlayerDeathData ) {
		bSpawnDeathEffects = FALSE;
	}
	
	if( !(m_uBotDeathFlags & CBot::BOTDEATHFLAG_WALKING_DEAD ) ) {
		SetBotFlag_IgnoreControls();
	}

	// Advance the LifeCycleState to dying...
	m_uLifeCycleState = BOTLIFECYCLE_DYING;
	m_uBotDeathFlags &= ~BOTDEATHFLAG_PLAYDEATHANIM_STARTED;

	CFScriptSystem::TriggerEvent( CFScriptSystem::GetEventNumFromName( "BotDie" ), 0, (u32)this, 0 );
	if (HasAlarmSysUse())
	{
		CAlarmNet* pNet = AlarmSys_GetAutoBotNet(this);
		CFScriptSystem::TriggerEvent( CFScriptSystem::GetEventNumFromName( "AutoBot" ), AUTOBOT_DIED, (u32)this, (u32)pNet);
	}

	CEntity::Die( bSpawnDeathEffects, bSpawnGoodies );

	DataPort_Shock( FALSE );
	DataPort_Open( FALSE );

	if( m_pActiveTalkInst ) {
		CTalkSystem2::TerminateActiveTalk(m_pActiveTalkInst);
		m_pActiveTalkInst = NULL;
	}

	_EnablePossessionTextBox( FALSE );

	m_fRotationalImpactDeltaYaw = 0.0f;
	m_fRotationalImpactSpeedMult = 0.0f;

	// If we have an EMP effect going on, disable it...
	if( IsStunned() ) {
		StunBot( -1.0f );
	}

	// If we have a paralysis effect going on, disable it...
	if( IsSlowed() ) {
		SlowBot( 0.0f );
	}

	m_pStickModifier_MS = NULL;

	if( IsPlayerBot() && m_pPlayerDeathData ) {
		PlayerDamage_StartDeath();
	}

}


void CBot::UnDie() { 
	m_uLifeCycleState = BOTLIFECYCLE_LIVING;
	m_uBotDeathFlags &= ~BOTDEATHFLAG_PLAYDEATHANIM_STARTED;

	if( m_pActiveTalkInst && (m_pActiveTalkInst == m_pDeathTalkInst) ) {
		CTalkSystem2::TerminateActiveTalk(m_pActiveTalkInst);
		m_pActiveTalkInst = NULL;
	}

	if( m_pDeathTalkInst ) {
		CTalkSystem2::BTIPool_Recycle( m_pDeathTalkInst );
		m_pDeathTalkInst = NULL;
	}

	m_pWorldMesh->SetCollisionFlag( TRUE );

	if( m_bDataPortInstalled ) {
		DataPort_Open( TRUE );
	}

	ClearBotFlag_IgnoreControls();
	SetCameraOverride( FALSE, 0.0f );

	SetUnitHealth( 1.0f );	

	m_uLimbBreakCounter = m_pBotInfo_Gen->uNumHitsToBreakLimb;

	// reset the bot to the neutral
	ResetToNeutral();
}

void CBot::ResetToNeutral( void ) {
	// Reticle is enabled
	if( m_nOwnerPlayerIndex > -1 ) {
		ReticleEnable(TRUE);
	}

	// This must follow the restoration of the bot's health!
	if( m_pPartMgr && m_pPartMgr->IsCreated() ) {
		m_pPartMgr->ResetAllToIntact();
	}

	// Always put legs back on...
	FMATH_CLEARBITMASK( m_nBotFlags, BOTFLAG_NO_LEGS );
	if( m_pNoLegsDebrisPileWorldMesh ) {
		m_pNoLegsDebrisPileWorldMesh->RemoveFromWorld();
	}

	// put torso back on...
	SetBlownOffTorsoMode( FALSE );

	// set the bot to not be stunned
	FMATH_CLEARBITMASK( m_nBotFlags, BOTFLAG_IS_STUNNED );

	SetLimpState( BOTLIMPSTATE_NONE );

	// Don't move when we spawn
	m_Velocity_MS.Zero();
	m_Velocity_WS.Zero();
	m_ImpulseVelocity_WS.Zero();
	FMATH_CLEARBITMASK( m_nBotFlags, BOTFLAG_VELOCITY_IMPULSE_VALID );

	m_fRotationalImpactDeltaYaw = 0.0f;
	m_fRotationalImpactSpeedMult = 0.0f;

	// Hips face forward
	m_fLegsYaw_MS = 0.0f;

	// jumping vars
	m_nJumpState = BOTJUMPSTATE_NONE;
	m_uJumpFlags = 0;
	m_fFlipPitch = 0.0f;
	m_bPlayLandAnim = FALSE;
	m_pDisguisedBot = NULL;
	m_fUnitDisguised = 0;
    m_fPanicSecs = 0.0f;
	m_nState = STATE_GROUND;
	m_nPrevState = m_nState;
	m_fDoubleJumpTimer = 0.0f;

	m_vWallImpactUnitVec.Zero();

	// If we have an EMP effect going on, disable it
	if( IsStunned() ) {
		StunBot( -1.0f );
	}

	// If we have an paralysis effect going on, disable it
	if( IsSlowed() ) {
		SlowBot( 0.0f );
	}

	// reset the animation stack setting the lowest priority as fully in control
	u32 i;
	for( i=0; i < ASI_COUNT; i++ ) {
		if( m_anAnimStackIndex[i] != -1 ) {
			SetControlValue( (AnimStackIndex_e)i, 0.0f );
		}
	}
	if( m_anAnimStackIndex[ASI_STAND] != -1 ) {
		SetControlValue( ASI_STAND, 1.0f );
	}

	// reset weapon reload state
	m_nThrowState = THROWSTATE_NONE;
	m_nScopeState = SCOPESTATE_NONE;
	m_abReloadRequest[0] = FALSE;
	m_abReloadRequest[1] = FALSE;
	m_nWSState = WS_STATE_NONE;
	m_nWRState = WR_STATE_NONE;
	m_pActionableEntityNearby = NULL;

	m_fScopeUnzoomHitpointAccumulator = 0.0f;
	m_fScopeUnzoomHitpointTimer = 0.0f;

	m_fDataPortShockSecsRemaining = 0.0f;
}


//
//  Default implementation
//	   Will get called if m_uLifeCycleState is not LIVING
//
void CBot::DeathWork( void ) {
	if( m_uLifeCycleState == BOTLIFECYCLE_LIVING ) {
		// Why is this called when not dead or dying?
		return;
	}

	if( PlayerDamage_DeathWork() ) {
		return;
	}

	BOOL bDidWork = FALSE;
	while( !bDidWork ) {
		// This while loop is to make sure that dead gets called if there is no usefull work to be done in dying case...

		switch( m_uLifeCycleState ) {
		case BOTLIFECYCLE_DYING:

			if( (m_uBotDeathFlags & BOTDEATHFLAG_PLAYDEATHANIM) && AIBrain() ) {
				if( !(m_uBotDeathFlags & BOTDEATHFLAG_PLAYDEATHANIM_STARTED) ) {
					// Start the anim...
					m_uBotDeathFlags |= BOTDEATHFLAG_PLAYDEATHANIM_STARTED;

					if( m_nState == STATE_AIR ) {
						m_uBotDeathFlags &= ~(BOTDEATHFLAG_PERSISTAFTERDEATH | BOTDEATHFLAG_AUTOPERSISTAFTERDEATH | BOTDEATHFLAG_PLAYDEATHANIM);
						SpawnDeathEffects();
						m_uLifeCycleState = BOTLIFECYCLE_DEAD;
					} else {
						cchar* pszBTAName = AIBTA_EventToBTAName("***_DEAD", AIBrain());
						if( pszBTAName ) {
							CBotTalkInst *pInst = CTalkSystem2::BTIPool_Get(pszBTAName);
							if( pInst && (bDidWork = CTalkSystem2::SubmitTalkRequest( pInst, this )) ) {
								// The request went through...

								FASSERT( pInst );

								// Hey inst, if you stop for whatever reason, please call this CB...
								pInst->SetTalkDoneCB( Bot_DeathBTAEndCB );
								m_pDeathTalkInst = pInst;
							} else {
								if( pInst ) {
									CTalkSystem2::BTIPool_Recycle(pInst);
								}

								m_uBotDeathFlags &= ~(BOTDEATHFLAG_PERSISTAFTERDEATH | BOTDEATHFLAG_AUTOPERSISTAFTERDEATH | BOTDEATHFLAG_PLAYDEATHANIM);
								SpawnDeathEffects();
								m_uLifeCycleState = BOTLIFECYCLE_DEAD;
							}
						} else {
							m_uBotDeathFlags &= ~(BOTDEATHFLAG_PERSISTAFTERDEATH | BOTDEATHFLAG_AUTOPERSISTAFTERDEATH | BOTDEATHFLAG_PLAYDEATHANIM);
							SpawnDeathEffects();
							m_uLifeCycleState = BOTLIFECYCLE_DEAD;
						}
					}
				} else {
					bDidWork = TRUE;
				}
			} else if (m_uBotDeathFlags & BOTDEATHFLAG_WALKING_DEAD) {
				bDidWork = TRUE;
			} else if (m_uBotDeathFlags & BOTDEATHFLAG_BOTPARTMGR_DEATHANIM) {
				// We're not dead until all the debris spawned by the part mgr is gone...
				
				if (m_pPartMgr && m_pPartMgr->IsCreated() && !m_pPartMgr->CurrentlyOwnsDebris())	{
					m_uLifeCycleState = BOTLIFECYCLE_DEAD;
				}

				bDidWork = TRUE;
			} else {
				m_uLifeCycleState = BOTLIFECYCLE_DEAD;
			}

			break;
		case BOTLIFECYCLE_DEAD:
		case BOTLIFECYCLE_BURIED:	// Special really-dead state for Glitch
			if( (m_uBotDeathFlags & BOTDEATHFLAG_PERSISTAFTERDEATH) ) {
				if (!(m_uBotDeathFlags & BOTDEATHFLAG_COLLIDES_AFTER_DEATH))
					m_pWorldMesh->SetCollisionFlag( FALSE );
			} else {
				if( m_uBotDeathFlags & BOTDEATHFLAG_AUTOPERSISTAFTERDEATH ) {
					// Put some logic here that eventually eliminates deadguys after some timeout and distance from camera, etc...
					if (!(m_uBotDeathFlags & BOTDEATHFLAG_COLLIDES_AFTER_DEATH))
						m_pWorldMesh->SetCollisionFlag( FALSE );
				} else {
					// Remove me from the world at the end of this frame...
					RemoveFromWorld();
				}
			}

			bDidWork = TRUE;

			break;
		}
	}
}


void CBot::DeathBTAEndCB( CBotTalkInst *pTalkInst ) {
	if( IsDeadOrDying() && IsDeathAnimStarted() ) {
	  	m_uLifeCycleState = BOTLIFECYCLE_DEAD;
	}
}


void Bot_DeathBTAEndCB( CBotTalkInst *pTalkInst, CBot *pBot ) {
	pBot->DeathBTAEndCB(pTalkInst);
}



void CBot::SpawnDeathEffects( void ) {
	if( m_pPartMgr && m_pPartMgr->IsCreated() ) {
		m_pPartMgr->DestroyBot( NULL );
	}
}


// This is to be called whenever the bot sees a disguized bot.
void CBot::NotifyPossessedBotSeen( CBot *pDisguisedBot, f32 fAddDisguiseDecrease ) {
	if( m_pDisguisedBot != pDisguisedBot ) {
		m_pDisguisedBot = pDisguisedBot;
		m_fUnitDisguised = 1.0f;
	} else if( m_pDisguisedBot && (m_fUnitDisguised > 0.0f) ) {
		// Adjust the unit disguise...

		m_fUnitDisguised -= FLoop_fPreviousLoopSecs * m_fUnitDisguisedDecreaseRate;
		m_fUnitDisguised -= fAddDisguiseDecrease;
		FMATH_CLAMPMIN( m_fUnitDisguised, 0.0f );
	}
}


// Retrieve info about the currently disguise bot.
CBot *CBot::GetDisguisedBot( f32 *pfDisguiseTimeLeft ) {
	if( pfDisguiseTimeLeft ) {
		if( m_pDisguisedBot ) {
			*pfDisguiseTimeLeft = m_fUnitDisguised;
		} else {
			*pfDisguiseTimeLeft = 0.0f;
		}
	}

	return m_pDisguisedBot;
}

// If the current weapon has someone targeted, return that someone.
CBot* CBot::GetTargetedBot( void ) const
{
	CBot* pBot = NULL;

	if ( m_pTargetedMesh && ( m_pTargetedMesh->m_nUser == MESHTYPES_ENTITY ) ) {
		CEntity* pEntity = (CEntity*)m_pTargetedMesh->m_pUser;
		if ( pEntity && ( pEntity->TypeBits() & ENTITY_BIT_BOT ) )
			pBot = (CBot*)pEntity;
	}
	return pBot;
}


void CBot::NotifyActionButton() {
	// Create a sphere to check for intersecting robots.
	CFSphere sphTest;

	// TODO: Should this be the center of his bounding sphere?
	sphTest.m_Pos = m_MtxToWorld.m_vPos.v3;

	sphTest.m_fRadius = ACTION_BUTTON_TEST_RADIUS;

	// Add a dummy tracker to the world
	CFWorldUser UserTracker;
	UserTracker.MoveTracker(sphTest);

	m_pCollBot = this;
	UserTracker.FindIntersectingTrackers( ActionCallback );
	UserTracker.RemoveFromWorld();
}


BOOL CBot::ActionNearby( CEntity *pEntity ) {

	CEntity::ActionNearby(pEntity);

	// If it's not a bot, then we don't have anything to do...
	if( pEntity != NULL ) {
		if( !(pEntity->TypeBits() & ENTITY_BIT_BOT) ) {
			// It's not a bot.
			return FALSE;
		}
	}

	if( AIBrain() ) {
		aibrainman_NotifyActionEnable(AIBrain(), pEntity);
	}

	if( m_pTalkInst != NULL ) {
		if( m_nPossessionPlayerIndex == -1 ) {
			// It's an AI bot.
			BOOL bRetVal = ai_TalkModeBegin( AIBrain(), _AITalkStartCallback, m_pTalkInst, this, 1, m_uActionTalkedCnt < 1, m_uActionTalkedCnt < 2, pEntity->Guid() );
			return bRetVal;
		} else {
			_AITalkStartCallback( TMCB_READY, (void *)m_pTalkInst, (void *)this );
			return TRUE;
		}
	}

//	return TRUE;
	return FALSE;	// why return true in this case?  Prevents further bots from getting ActionNearby callbacks. -cjm
}


BOOL CBot::Create( const CBotDef *pBotDef, s32 nPlayerIndex, BOOL bInstallDataPort, cchar *pszEntityName, const CFMtx43A *pMtx, cchar *pszAIBuilderName ) {
	FASSERT( !IsCreated() );
	FASSERT( FWorld_pWorld );

	// Get pointer to the leaf class's builder object...
	CBotBuilder *pBuilder = (CBotBuilder *)GetLeafClassBuilder();

	// If we're the leaf class, set the builder defaults...
	if( pBuilder == _pBotBuilder ) {
		pBuilder->SetDefaults( 0, 0, ENTITY_TYPE_BOT );
	}

	// Set our builder parameters...
	pBuilder->m_pBotDef = pBotDef;
	pBuilder->m_nOwnerPlayerIndex = nPlayerIndex;
	pBuilder->m_bDataPortInstalled = bInstallDataPort;
	pBuilder->m_bDataPortOpen = FALSE;

	// Create an entity...
	return CEntity::Create( pszEntityName, pMtx, pszAIBuilderName );
}


// Called from the leaf down to CEntity.
// Returns FALSE if the object could not be built.
BOOL CBot::ClassHierarchyBuild( void ) {
	CBotBuilder *pBuilder;
	f32 fStartHeading;

	FASSERT( IsSystemInitialized() );
	FASSERT( !IsCreated() );
	FASSERT( FWorld_pWorld );
	FASSERT( (TypeBits() & ENTITY_BITS_ALLBOTBITS) != ENTITY_BIT_BOT);
	// Get a frame...
	FResFrame_t ResFrame = fres_GetFrame();

	// Build parent class...
	if( !CEntity::ClassHierarchyBuild() ) {
		// Parent class could not be built...
		goto _ExitWithError;
	}

	// Get pointer to the leaf class's builder object...
	pBuilder = (CBotBuilder *)GetLeafClassBuilder();

	// Set defaults...
	_ClearDataMembers();

	// Initialize from builder object...
	m_MountPos_WS = m_MtxToWorld.m_vPos;
	m_pBotDef = pBuilder->m_pBotDef;
	m_nOwnerPlayerIndex = pBuilder->m_nOwnerPlayerIndex;
	m_nPossessionPlayerIndex = pBuilder->m_nOwnerPlayerIndex;

	fStartHeading = fmath_Atan( m_MtxToWorld.m_vFront.x, m_MtxToWorld.m_vFront.z );
	ChangeMountYaw( fStartHeading );
	m_MovingSurfaceUnitVec_WS = m_MountUnitFrontXZ_WS;
	m_TargetLockUnitVec_WS = m_MtxToWorld.m_vFront;
	m_TargetedPoint_WS.Zero();
	m_pTargetedMesh = NULL;

	m_MountPrevPos_WS = m_MtxToWorld.m_vPos;

	//////////////////////////////////////////////////////////////////////
	// Get our CTalkInst set up from the builder.
	m_pTalkInst = NULL;
	if( pBuilder->m_pBuilderTalkInst != NULL ) {
		m_pTalkInst = pBuilder->m_pBuilderTalkInst;
		m_pTalkInst->Init( pBuilder->m_pszTalkDataFileName);
		m_pTalkInst->SetFlags(pBuilder->m_uBuilderTalkInstFlags);
		m_pTalkInst->SetTalkDoneCB(_AITalkEndCallback);
		SetActionable( TRUE );
	}
	m_uActionTalkedCnt = 0;
	m_pActiveTalkInst = NULL;
	m_pDeathTalkInst = NULL;

	// Store data port variables...
	m_bDataPortInstalled = pBuilder->m_bDataPortInstalled;
	m_bDataPortOpen = pBuilder->m_bDataPortOpen;
	m_fDisguiseTime = pBuilder->m_fDisguiseTime;
	m_fPossessionDist = pBuilder->m_fPossessionDist;
	m_fRunMultiplier = pBuilder->m_fRunMultiplier;
	if( pBuilder->m_bSleepAtStart ) {
		m_nSleepState = BOTSLEEPSTATE_INIT;
	}
	if (pBuilder->m_bSetBotFlag_PowerDownUntilPosessed)
	{
		SetBotFlag_PowerDownUntilPosessed();
	}

	if( pBuilder->m_bBabble ) {
		m_nBotFlags2 |= BOTFLAG2_BABBLE;
	}

	m_pPartMgr = fnew CBotPartMgr;
	if( m_pPartMgr == NULL ) {
		DEVPRINTF( "CBot::ClassHierarchyBuild(): Not enough memory to allocate m_pPartMgr.\n" );
		goto _ExitWithError;
	}

	// Declare immunity from world removal upon parents' world removal
	SetRemovedOnParentsDetach(FALSE);

	// Success...
	return TRUE;

	// Error:
_ExitWithError:
	Destroy();
	fres_ReleaseFrame( ResFrame );
	return FALSE;
}


BOOL CBot::LoadAuxMeshEntity( CMeshEntity **ppMeshEntity, cchar **pszMeshName, u32 uMeshCount, cchar *pszEntityName ) {
	FResFrame_t ResFrame = fres_GetFrame();

	*ppMeshEntity = fnew CMeshEntity;
	if( *ppMeshEntity == NULL ) {
		DEVPRINTF( "CBot::LoadAuxMeshEntity(): Not enough memory to create mesh %s.\n", pszMeshName );
		goto _ExitWithError;
	}

	if( !(*ppMeshEntity)->Create( uMeshCount, pszMeshName, 0, NULL, "DataPort" ) ) {
		DEVPRINTF( "CBot::LoadAuxMeshEntity(): Trouble creating mesh %s.\n", pszMeshName );
		goto _ExitWithError;
	}

	(*ppMeshEntity)->SetCollisionFlag( FALSE );
	(*ppMeshEntity)->SetLineOfSightFlag( FALSE );
	(*ppMeshEntity)->SetCullDist( m_pWorldMesh->m_fCullDist );
	(*ppMeshEntity)->MeshFlip_SetFlipsPerSec( 0.0f );
	(*ppMeshEntity)->RemoveFromWorld();

	// Success...

	return TRUE;

	// Failure...
_ExitWithError:
	fdelete( *ppMeshEntity );
	*ppMeshEntity = NULL;

	fres_ReleaseFrame( ResFrame );

	return FALSE;
}


void CBot::SetCameraOverride( BOOL bOverride, f32 fBlendTime, BOOL bPitched, const CFVec3A &vLookAt, const CFVec3A &vCamPos ) {
	FASSERT( IsCreated() );
	m_fCamTransTime	 = fBlendTime;

	if( bOverride ) {
		FMATH_SETBITMASK( m_nBotCameraFlags, BOTCAMERAFLAG_OVERRIDDEN );
		m_vecCamLookAdj.Set( vLookAt );
		m_vecCamPosAdj.Set( vCamPos );

		if( bPitched ) {
			FMATH_SETBITMASK( m_nBotCameraFlags, BOTCAMERAFLAG_OVERRIDEPITCH_TRICK );
		}
	} else {
		FMATH_CLEARBITMASK( m_nBotCameraFlags, BOTCAMERAFLAG_OVERRIDDEN );
		FMATH_CLEARBITMASK( m_nBotCameraFlags, BOTCAMERAFLAG_OVERRIDEPITCH_TRICK );
	}

	FMATH_SETBITMASK( m_nBotCameraFlags,BOTCAMERAFLAG_NEW_SETTINGS );
}


// Returns FALSE if the data port resources were not loaded (not necessesarily an error).
BOOL CBot::_LoadDataPortResources( void ) {
	FResFrame_t ResFrame = fres_GetFrame();

	FASSERT( m_pDataPortMeshEntity == NULL );
	FASSERT( m_pDataPortEffectMeshEntity == NULL );

	// Handle data port construction...
	if( !m_bDataPortInstalled || (m_pBotInfo_Gen == NULL) || !DoesBoneExist( m_pBotInfo_Gen->pszDataPortBoneName ) ) {
		// Cannot install data port...
		goto _DataPortResourcesNotLoaded;
	}

	// Install data port!

	// Loaded open and closed data port meshes...
	if( !LoadAuxMeshEntity( &m_pDataPortMeshEntity, &m_pBotInfo_Gen->apszDataPortMeshName[0], 2, "DataPort" ) ) {
		// Resources could not be loaded...
		goto _DataPortResourcesNotLoaded;
	}

	// Loaded data port effect meshe...
	if( !LoadAuxMeshEntity( &m_pDataPortEffectMeshEntity, &m_pBotInfo_Gen->apszDataPortMeshName[2], 1, "DataPortFX" ) ) {
		// Resources could not be loaded...
		goto _DataPortResourcesNotLoaded;
	}

	// Data port resources loaded ok...

	if( IsInWorld() ) {
		m_pDataPortMeshEntity->Attach_UnitMtxToParent_PS( this, m_pBotInfo_Gen->pszDataPortBoneName );
		m_pDataPortMeshEntity->AddToWorld();
		m_pDataPortMeshEntity->SelectMesh( !!m_bDataPortOpen );

		if( m_bDataPortOpen ) {
			m_pDataPortEffectMeshEntity->Attach_UnitMtxToParent_PS( this, m_pBotInfo_Gen->pszDataPortBoneName );
			m_pDataPortEffectMeshEntity->AddToWorld();
		}
	}

	return TRUE;

_DataPortResourcesNotLoaded:
	fres_ReleaseFrame( ResFrame );

	m_bDataPortInstalled = FALSE;
	m_bDataPortOpen = FALSE;

	return FALSE;
}


BOOL CBot::ClassHierarchyBuilt( void ) {
	CBotBuilder *pBuilder;

	if( !CEntity::ClassHierarchyBuilt() ) {
		goto _ExitWithError;
	}

	// All bots, by default, cast shadows
	m_pWorldMesh->m_nFlags |= FMESHINST_FLAG_CAST_SHADOWS;

	// Get pointer to the leaf class's builder object...
	pBuilder = (CBotBuilder *)GetLeafClassBuilder();

	// Load data port resources...
	_LoadDataPortResources();

	if( m_pBotInfo_Gen ) {
		// Set up possession data...
		if( m_fDisguiseTime < 0.0f ) {
			// Get from bot class CSV...
			m_fDisguiseTime = m_pBotInfo_Gen->fDisguiseTime;
		}

		if( m_fPossessionDist < 0.0f ) {
			// Get from bot class CSV...
			m_fPossessionDist = m_pBotInfo_Gen->fPossessionDist;
		}

		if( !fclib_stricmp( m_pBotInfo_Gen->pszWeightClass, _BOT_WEIGHT_LIGHT ) ) {
			_LoadSoundResource( BOTSOUND_LIGHT_WEIGHT );
			m_eWeightClass = BOTWEIGHT_LIGHT;

		} else if( !fclib_stricmp( m_pBotInfo_Gen->pszWeightClass, _BOT_WEIGHT_MEDIUM ) ) {
			_LoadSoundResource( BOTSOUND_MEDIUM_WEIGHT );
			m_eWeightClass = BOTWEIGHT_MEDIUM;

		} else if( !fclib_stricmp( m_pBotInfo_Gen->pszWeightClass, _BOT_WEIGHT_HEAVY ) ) {
			_LoadSoundResource( BOTSOUND_HEAVY_WEIGHT );
			m_eWeightClass = BOTWEIGHT_HEAVY;

		} else if( !fclib_stricmp( m_pBotInfo_Gen->pszWeightClass, _BOT_WEIGHT_CORROSIVE ) ) {
			_LoadSoundResource( BOTSOUND_CORROSIVE_WEIGHT );
			m_eWeightClass = BOTWEIGHT_CORROSIVE;
		
		} else if( !fclib_stricmp( m_pBotInfo_Gen->pszWeightClass, _BOT_WEIGHT_ZOMBIEBOSS ) ) {
			_LoadSoundResource( BOTSOUND_ZOMBIEBOSS_WEIGHT );
			m_eWeightClass = BOTWEIGHT_ZOMBIEBOSS;

		} else if( !fclib_stricmp( m_pBotInfo_Gen->pszWeightClass, _BOT_WEIGHT_VEHICLE ) ) {
			_LoadSoundResource( BOTSOUND_VEHICLE_WEIGHT );
			m_eWeightClass = BOTWEIGHT_VEHICLE;

		} else {
#if FANG_WINGC
			// Some retail bot tables say 'none', which retail's code (the same six classes as here) also
			// treated as medium; only report names that aren't that.
			if( fclib_stricmp( m_pBotInfo_Gen->pszWeightClass, "none" ) )
#endif
			DEVPRINTF( "CBot::ClassHierarchyBuilt() : Invalid weight class '%s'!  Using default of 'medium'\n", m_pBotInfo_Gen->pszWeightClass);
			_LoadSoundResource( BOTSOUND_MEDIUM_WEIGHT );
			m_eWeightClass = BOTWEIGHT_MEDIUM;
		}

		m_uLimbBreakCounter = m_pBotInfo_Gen->uNumHitsToBreakLimb;
	}

	if( m_fDisguiseTime > 0.0f ) {
		m_fUnitDisguisedDecreaseRate = fmath_Inv( m_fDisguiseTime );
	} else {
		// m_fDisguiseTime == 0 at build time means that this bot doesn't really believe any disguises, so set the decrease time to be one second
		m_fUnitDisguisedDecreaseRate = fmath_Inv( 1.0f );

	}

	if( AIBrain() ) {
		// There's an AI brain driving this bot.
		// Disable auto-work...
		EnableAutoWork( FALSE );
	}

	SetTargetable( TRUE );

	// turn spotlight on or off based on user prop
	if( pBuilder->m_uFlags & CBotBuilder::BOT_BUILDER_FLAG_SPOTLIGHT_ON ) {
		SetSpotLightOn( TRUE );
	} else {
		SetSpotLightOff( TRUE );
	}

	if( pBuilder->m_uFlags & CBotBuilder::BOT_BUILDER_CLASS_CAN_BE_RECRUITED ) {
		FMATH_SETBITMASK( m_nBotFlags2, BOTFLAG2_CLASS_CAN_BE_RECRUITED );
	}

	if( pBuilder->m_uFlags & CBotBuilder::BOT_BUILDER_DPORT_NOT_NEEDED_FOR_RECRUITMENT ) {
		FMATH_SETBITMASK( m_nBotFlags2, BOTFLAG2_DPORT_NOT_NEEDED_FOR_RECRUITMENT );
	}

	if( pBuilder->m_uFlags & CBotBuilder::BOT_BUILDER_INST_CANNOT_BE_RECRUITED ) {
		FMATH_SETBITMASK( m_nBotFlags2, BOTFLAG2_INST_CANNOT_BE_RECRUITED );
	}

	if( pBuilder->m_uFlags & CBotBuilder::BOT_BUILDER_SHOWS_UP_ON_RADAR ) {
		FMATH_SETBITMASK( m_nBotFlags2, BOTFLAG2_SHOWS_UP_ON_RADAR );
	}

#if FANG_WINGC
	if( pBuilder->m_uFlags & CBotBuilder::BOT_BUILDER_FLAG_DISABLE_VELOCITY_IMPULSES ) {
		FMATH_SETBITMASK( m_nBotFlags2, BOTFLAG2_DISABLE_VELOCITY_IMPULSES );
	}
#endif

	m_eIdleState = IDLESTATE_NO_IDLES_TO_PLAY;
	if( m_nCountIdles > 0 ) {
		m_eIdleState = IDLESTATE_NONE;
	}

	EnableOurWorkBit();
	//if( m_pPartMgr->IsCreated() ) {
	//	EnableOurWorkBit();
	//} else {
	//	DisableOurWorkBit();
	//}

	// Success...

	return TRUE;

_ExitWithError:
	return FALSE;
}


void CBot::ClassHierarchyDestroy( void ) {
	fdelete( m_pNoLegsDebrisPileWorldMesh );
	fdelete( m_pPartMgr );
	fdelete( m_pDataPortMeshEntity );
	fdelete( m_pDataPortEffectMeshEntity );
	fdelete( m_pTalkInst );

	_ClearDataMembers();

	CEntity::ClassHierarchyDestroy();
}


CEntityBuilder *CBot::GetLeafClassBuilder( void ) {
	return _pBotBuilder;
}


void CBot::_ClearDataMembers( void ) {
	u32 i;

	// Public members:
	m_pBotDef = NULL;
	m_pWorldMesh = NULL;
	m_pNoLegsDebrisPileWorldMesh = NULL;
	m_pPartMgr = NULL;

	m_nOwnerPlayerIndex = -1;
	m_nPossessionPlayerIndex = -1;

	m_uJumpFlags = 0;
	m_uLandingVelocityOverride = (u16)-1;
	m_fCamZoomFactor =	0.0f;
	m_vecCamLookAdj.Zero();	
	m_vecCamPosAdj.Zero();		
	m_fCamTransTime		= 0.0f;	
	m_nBotCameraFlags = BOTCAMERAFLAG_NONE;	

	// note that these four variables are reset to these defaults in CBotFx::KillAll()
	m_fRunMultiplier = 1.0f;
	m_fOORunMultiplier = 1.0f;
	m_fJumpMultiplier = 1.0f;
	m_fGravityMultiplier = 1.0f;

	m_fUnitPanic = 0.0f;
	m_fUnitAlert = 0.0f;

	m_fMountPitch_WS = 0.0f;
	m_fMountPitchMin_WS = 0.0f;
	m_fMountPitchMax_WS = 0.0f;
	m_fMountYaw_WS = 0.0f;
	m_fMountYawSin_WS = fmath_Sin( m_fMountYaw_WS );
	m_fMountYawCos_WS = fmath_Cos( m_fMountYaw_WS );
	m_MountUnitFrontXZ_WS = CFVec3A::m_UnitAxisX;
	m_MountPrevPos_WS.Zero();

	m_TargetLockUnitVec_WS = m_MountUnitFrontXZ_WS;
	m_TargetedPoint_WS.Zero();

	m_pApproxEyePoint_WS = &m_MtxToWorld.m_vPos;
	m_GazeUnitVec_WS = m_MountUnitFrontXZ_WS;
	m_pAISteerMtx = &m_MtxToWorld;

	m_XlatStickNormVecXZ_MS.Zero();
	m_XlatStickNormVecXZ_WS.Zero();
	m_XlatStickUnitVecXZ_MS.Zero();
	m_XlatStickUnitVecXZ_WS.Zero();
	m_fXlatStickNormMag = m_XlatStickNormVecXZ_MS.Mag();

	m_fBestCaseSurfaceUnitNormY_WS = 1.0f;
	m_SurfaceUnitNorm_WS = CFVec3A::m_UnitAxisY;
	m_MovingSurfaceUnitVec_WS.Zero();

	m_Velocity_MS.Zero();
	m_Velocity_WS.Zero();
	m_UnitVelocity_WS.Zero();
	m_VelocityXZ_WS.Zero();
	m_UnitVelocityXZ_WS.Zero();
	m_NormVelocityXZ_WS.Zero();
	m_NormVelRotatedToXZ_WS.Zero();

	m_fSpeed_WS = 0.0f;
	m_fSpeedXZ_WS = 0.0f;
	m_fNormSpeedXZ_WS = 0.0f;
	m_fClampedNormSpeedXZ_WS = 0.0f;
	m_fSpeedAlongSurface_WS = 0.0f;
	m_fNormSpeedAlongSurface_WS = 0.0f;
	m_fClampedNormSpeedAlongSurface_WS = 0.0f;

	m_fMaxVerticalSpeed_WS		= 0.0f;
	m_fMaxFlatSurfaceSpeed_WS = 0.0f;
	m_fMaxDeltaJumpVelY_WS = 0.0f;
	m_fGravity = 0.0f;

	m_fCollCylinderRadius_WS = 1.0f;
	m_fCollCylinderHeight_WS = 1.0f;

	m_fStoopMinHeight = 0.0f;
	m_fStoopMaxHeight = 0.0f;
	m_bStooping = FALSE;

	m_fRotationalImpactDeltaYaw = 0.0f;
	m_fRotationalImpactSpeedMult = 0.0f;

	m_apWeapon[0] = NULL;
	m_apWeapon[1] = NULL;

	m_anNumDupWeapons[0] = 0;
	m_anNumDupWeapons[1] = 0;
	
	for( i=0; i<BOT_MAX_DUPLICATE_WPNS; i++ ) {
		m_aapDupWeapons[0][i] = NULL;
		m_aapDupWeapons[1][i] = NULL;
	}

	m_pInventory = NULL;
	m_pTalkInst = NULL;
	m_pActiveTalkInst = NULL;
	m_pDeathTalkInst = NULL;
	m_uActionTalkedCnt = 0;

	// Protected members:
	m_nBotFlags = BOTFLAG_NONE;
	m_nBotFlags2 = BOTFLAG2_NONE;
	m_pMoveIdentifier = NULL;
	m_bDataPortInstalled = FALSE;
	m_bDataPortOpen = FALSE;
	m_pDataPortMeshEntity = NULL;
	m_pDataPortEffectMeshEntity = NULL;
	m_pDataPortBoneMtx = NULL;
	m_fTetherShockAnimUnitBlend = 0.0f;

	m_fOrigArmorModifier = 0.0f;

	m_fDisguiseTime = 0.0f;
	m_pDisguisedBot = NULL;
	m_fPossessionDist = 0.0f;

	m_PossessedPos_WS.Zero();
	m_fUnitDisguised = 0.0f;
	m_fUnitDisguisedDecreaseRate = 0.0f;

	m_fPanicSecs = 0.0f;

	m_nRecruitID = -1;
	m_nRecruitPlayerIndex = -1;

	m_fStaticOnDistThreshold = 0.0f;
	m_bHysterisisLargerThreshold = FALSE;
	m_fUnitStaticSurge = 1.0f;
	m_fUnitBaseStatic = 0.0f;
	m_fUnitHudStatic = 0.0f;
	m_fUnitStatic = 0.0f;
	m_fUnitDrawStatic = 0.0f;

	m_fUnitStaticFluxSlider = 0.0f;
	m_fUnitStaticFluxSliderRate = 0.0f;
	m_fStartUnitStaticFlux = 0.0f;
	m_fEndUnitStaticFlux = 0.0f;
	m_fUnitOutOfRangeWarningMsg = 0.0f;
	m_nUnplugState = UNPLUG_STATE_NONE;
	m_fUnitUnplug = 0.0f;
	m_pCurMech = NULL;

	m_pnEnableBoneNameIndexTableForSummer_Normal = NULL;
	m_pnEnableBoneNameIndexTableForSummer_TetherShock = NULL;

	for( i=0; i<FDATA_MAX_BONE_COUNT; ++i ) {
		m_anDriveManMtxIndexForSummer_TetherShock[i] = -1;
	}
	m_nTetherShockBoneCount = 0;

	m_nPowerState = POWERSTATE_POWERED_UP;
	m_bOppositePowerStatePending = FALSE;
	m_fPowerAnimUnitBlend = 0.0f;
	m_fPowerOffOnTime = 0.0f;
	m_fPowerOffOnAnimSpeedMult = 1.0f;
	m_fPendingOffOnTime = 0.0f;
	m_fPendingPowerOffOnAnimSpeedMult = 1.0f;
	m_fDataPortShockSecsRemaining = 0.0f;

	m_pBotInfo_Gen = NULL;
	m_pBotInfo_MountAim = NULL;
	m_pBotInfo_Walk = NULL;
	m_pBotInfo_Jump = NULL;
	m_pBotInfo_Weapon = NULL;
	m_pBotInfo_Sound = NULL;

	m_nSurfaceType = GCOLL_SURF_TYPE_NORMAL;
	m_pSurfaceMtl = &CGColl::m_aMaterialTable[0];

	m_nSurfaceTypeOn = SURFACE_TYPE_NONE;
	m_fNextScuffSound = fmath_RandomFloatRange( BOT_SOUND_SCUFF_MIN_TIME, BOT_SOUND_SCUFF_MAX_TIME );
	m_fStepVolumeScale = 1.0f;
	m_fStepVolumeScaleDir = 1.0f;

	m_ImpulseVelocity_WS.Zero();

	m_vWallImpactUnitVec.Zero();

	m_bControls_Human = TRUE;
	ZeroControls();

	m_nTractionMode = TRACTIONMODE_NORMAL;
	m_fSlipGravityStickBias = 0.0f;
	m_fUnitTraction = 1.0f;
	m_fUnitTopSpeedLimiter = 1.0f;

	m_nJumpState = BOTJUMPSTATE_NONE;
	m_fTuckUntuckAnimTimeMult = 1.0f;
	m_fUntuckStartCountdownSecs = 0.0f;
	m_fFlipPitchVelocity = 0.0f;
	m_fFlipPitch = 0.0f;
	m_bPlayLandAnim = FALSE;
	m_bPlayLandAnimLower = FALSE;
	m_fMaxLandUnitBlend = 1.0f;
	m_fDoubleJumpTimer = 0.0f;

	m_nMoveState	= BOTMOVESTATE_NONE;
	m_nMoveType = BOTMOVETYPE_HOP_LEFT;
	m_fUnitHop = 0.0f;

	m_nState = STATE_GROUND;
	m_nPrevState = m_nState;
	m_FeetToGroundCollImpact.UnitFaceNormal.v3 = CFVec3::m_UnitAxisY;
	m_FeetToGroundCollImpact.fImpactDistInfo = 0.0f;
	m_FeetToGroundCollImpact.PushUnitVec.v3 = CFVec3::m_UnitAxisY;
	m_fSecsSinceLastFootCollision = 0.0f;
	m_uCollisionStatus	= COLLSTATE_NO_COLLISION;

	for( i=0; i<ASI_COUNT; ++i ) {
		m_anAnimStackIndex[i] = -1;
	}

	m_fMaxSneakStickMag = 0.0f;
	m_fUnitRunBlend = 0.0f;
	m_fUnitWalkBlend = 0.0f;
	m_fUnitSneakBlend = 0.0f;
	m_fUnitCableBlend = 0.0f;
	m_fUnitFlyBlend = 0.0f;
	m_fUnitJump1Jump2Blend = 0.0f;
	m_fUnitFeetAnim = 0.0f;
	m_eIdleState=IDLESTATE_NO_IDLES_TO_PLAY;						
	m_nCurrentIdle=0;						
	m_nCountIdles = 0;

	m_nThrowState = THROWSTATE_NONE;
	m_nScopeState = SCOPESTATE_NONE;
	m_abReloadRequest[0] = FALSE;
	m_abReloadRequest[1] = FALSE;
	m_nWSState = WS_STATE_NONE;
	m_nWRState = WR_STATE_NONE;
	m_nWRAbortASI = ASI_RELOAD_SINGLE_BODY;
	m_fWRAbortUnitControl = 0.0f;

	m_fScopeUnzoomHitpointAccumulator = 0.0f;
	m_fScopeUnzoomHitpointTimer = 0.0f;

	m_fYawUnitOverdrive = 0.0f;
	m_fAimPitch_WS = 0.0f;
	m_fAimDeltaYaw_MS = 0.0f;
	m_fMountPitchClampedNormDelta = 0.0f;
	m_fMountYawClampedNormDelta = 0.0f;

	m_LastStickyEntityPos.Zero();
	m_pStickyEntity = NULL;
	m_pszStickyEntityBoneName = NULL;

	m_fUnitTorsoTwistBlend = 0.0f;
	m_fUnitReloadSingleBlend = 0.0f;
	m_fLegsYaw_MS = 0.0f;

	m_pCableHook = NULL;
	m_pCablePrev = NULL;
	m_fCableGrabTime = 0.0f;
	m_fZipVerletRestDistance = 0.0f;
	m_ZipVerletCurr.Zero();
	m_ZipVerletPrev.Zero();
	m_ZipHandPos_MS.Zero();

	m_vHeadLookPoint_WS.Zero();
	m_fHeadLookSlerp = 0.0f;
	m_nHeadLookInterruptCount = 0;
	m_HeadLookCurrentQuat.Identity();
	m_HeadLookOriginQuat.Identity();
	SetHeadLookRanges( FMATH_DEG2RAD( 60.0f ), FMATH_DEG2RAD( 45.0f ) );

	m_fReverseFacingYaw_MS = 0.0f;
	//sleep state vbls
	m_nSleepState = BOTSLEEPSTATE_NONE;
	m_fUnitWake = 0.0f;
	m_fUnitScope = 0.0f;
	m_fUnitNapJerk = 0.0f;
	m_fNapJerkTimeOut = 0.0f;

	// UASlot management...
	m_pUserAnimAbortCB = NULL;
	m_pAbortReqCompleteCB = NULL;
	m_fUnitAbortUserAnimLock = 0.0f;	
	m_pUserAnimSlotLockId = NULL;
	m_nUASlotFlags = 0;					

	m_uLifeCycleState = BOTLIFECYCLE_LIVING;		// BotLifeCycle_e
	m_uBotDeathFlags = 0;							// BotDeathParamFlags_e
	m_fLastFireTime = -1.0f;
	m_bStooping = FALSE;

	m_pLeftFootDustPos_WS = NULL;
	m_pRightFootDustPos_WS = NULL;

	m_fStunTime = 0.0f;
	
	m_fSlowEffectMult = 0.f;				// what's the effective slowdown
	m_fSlowTimeSinceHit = 0.0f;				// how long have we been paralyzed
	m_fSlowTimeSpeedUp = 0.0f;					// when do we start moving again?
	m_fSlowTimeOver = 0.0f;					// when do we attain full speed
	m_fSlowUnitParalysis = 0.0f;			// how frozen are we?

	m_eSpotLightState			= LIGHT_STATE_OFF;
	m_pSpotLight				= NULL;
	m_pSpotLightMesh			= NULL;
	m_eSpotLightOverrideState	= LIGHTOVERRIDE_STATE_NOT_OVERRIDDEN;
	m_fSpotLightOverrideTimer = 0.0f;

	m_nLimpState				= BOTLIMPSTATE_NONE;
	m_fUnitLimpSpeedModifier	= 1.0f;
	m_fUnitLimpBlend			= 0.0f;

	m_pDrivingVehicle = NULL;
	m_pActionableEntityNearby = NULL;
	m_uShockwaveResetTicks = 0;

	m_pStickModifier_MS = NULL;
	m_pShield = NULL;

	m_pPowerupFx = NULL;
	m_uNumAttachedSwarmers = 0;

	m_uLimbBreakCounter = 0;

	TagPoint_CreateFromMountPos();

	m_hRespawnEffectPartDef = FPARTICLE_INVALID_HANDLE;
	m_hRespawnEffectParticle = FPARTICLE_INVALID_HANDLE;
	m_pRespawnEffectMesh = NULL;

	m_bRespawnEffectInitialized = FALSE;
	m_fRespawnEffectTimer = 0.0f;
	m_nRespawnEffectMeshBoneIdx = -1;
	m_vRespawnEffectMeshPos.Zero();
	m_fRespawnEffectPartIntensity = 0.0f;
	m_fRespawnEffectMeshAlpha = 0.0f;
	m_fRespawnEffectMeshStartScale = 1.0f;
	m_fRespawnEffectMeshFinishScale = 1.0f;

	m_nTeamNum = 0;

	// in pieces
	m_bInPieces = FALSE;
	m_bFallDown = FALSE;
	m_bFallAnimDone = TRUE;
	m_bOnGround = FALSE;
	m_fTimeInPieces = 0.0f;
	m_fUnitPiecesBlend = 0.0f;


	m_pCanOnlyBeDispensedBy = NULL;

	m_pPlayerDeathData = NULL;
}


void CBot::TagPoint_CreateFromMountPos( void ) {
	FMATH_CLEARBITMASK( m_nBotFlags2, BOTFLAG2_TAG_POINT_IS_BOUND_CENTER );

	m_nTagPointCount = 1;

	m_pTagPointArray_WS = &m_MtxToWorld.m_vPos;
	m_pTagPointArray_MS = NULL;
}


BOOL CBot::TagPoint_CreateFromBoundingSphereCenter( void ) {
	FASSERT( IsCreated() );

	FMATH_SETBITMASK( m_nBotFlags2, BOTFLAG2_TAG_POINT_IS_BOUND_CENTER );

	m_nTagPointCount = 1;
	m_pTagPointArray_MS = NULL;

	m_pTagPointArray_WS = fnew CFVec3A;
	if( m_pTagPointArray_WS == NULL ) {
		DEVPRINTF( "TagPoint_CreateFromBuiltInVector(): Not enough memory to allocate m_pTagPointArray_WS.\n" );
		TagPoint_CreateFromMountPos();
		return FALSE;
	}

	TagPoint_Update();

	return TRUE;
}


void CBot::TagPoint_CreateFromBuiltInVector( const CFVec3A *pTagPoint_WS ) {
	FASSERT( IsCreated() );

	FMATH_CLEARBITMASK( m_nBotFlags2, BOTFLAG2_TAG_POINT_IS_BOUND_CENTER );

	m_nTagPointCount = 1;

	m_pTagPointArray_MS = NULL;
	m_pTagPointArray_WS = (CFVec3A *)pTagPoint_WS;
}


BOOL CBot::TagPoint_CreateFromBoneArray( const u8 *pnTagBoneIndexArray, cchar **apszBoneNameTable ) {
	FASSERT( IsCreated() );
	FASSERT( pnTagBoneIndexArray );
	FASSERT( apszBoneNameTable );

	if( m_pWorldMesh == NULL ) {
		TagPoint_CreateFromMountPos();
		return TRUE;
	}

	s32 i, nBoneIndex;

	FMATH_CLEARBITMASK( m_nBotFlags2, BOTFLAG2_TAG_POINT_IS_BOUND_CENTER );

	// Count the number of tag points in pnTagBoneIndexArray...
	m_nTagPointCount = 0;
	for( i=0; pnTagBoneIndexArray[i] != 255; ++i ) {
		nBoneIndex = m_pWorldMesh->FindBone( apszBoneNameTable[ pnTagBoneIndexArray[i] ] );

		if( nBoneIndex == -1 ) {
			DEVPRINTF( "CreateTagPoint_CreateFromBoneArray(): Bone '%s' not found in mesh. Bone will not be included as a tag point.\n" );
			continue;
		}

		++m_nTagPointCount;
	}

	if( m_nTagPointCount == 0 ) {
		// No tag points. Use bounding sphere center...

		return TagPoint_CreateFromBoundingSphereCenter();
	}

	FResFrame_t ResFrame = fres_GetFrame();

	m_pTagPointArray_WS = fnew CFVec3A[m_nTagPointCount];
	if( m_pTagPointArray_WS == NULL ) {
		DEVPRINTF( "CreateTagPoint_CreateFromBoneArray(): Not enough memory to allocate m_pTagPointArray_WS.\n" );
		goto _ExitWithError;
	}

	m_pTagPointArray_MS = fnew CFVec3A[m_nTagPointCount];
	if( m_pTagPointArray_MS == NULL ) {
		DEVPRINTF( "CreateTagPoint_CreateFromBoneArray(): Not enough memory to allocate m_pTagPointArray_MS.\n" );
		goto _ExitWithError;
	}

	for( i=0; i<m_nTagPointCount; ++i ) {
		nBoneIndex = m_pWorldMesh->FindBone( apszBoneNameTable[ pnTagBoneIndexArray[i] ] );
		FASSERT( nBoneIndex >= 0 );

		m_pTagPointArray_MS[i] = m_pWorldMesh->m_pMesh->pBoneArray[ nBoneIndex ].AtRestBoneToModelMtx.m_vPos;
	}

	TagPoint_Update();

	return TRUE;

_ExitWithError:
	fres_ReleaseFrame( ResFrame );

	TagPoint_CreateFromMountPos();

	return FALSE;
}


// Updates m_pTagPointArray_WS.
void CBot::TagPoint_Update( void ) {
	FASSERT( IsCreated() );
	FASSERT( m_pTagPointArray_WS );

	if( m_nBotFlags2 & BOTFLAG2_TAG_POINT_IS_BOUND_CENTER ) {
		// Use bounding sphere as tag point...

		m_pTagPointArray_WS->Set( m_pWorldMesh->GetBoundingSphere().m_Pos );

		return;
	}

	if( m_pTagPointArray_MS == NULL ) {
		// Nothing to do for built-in vectors...
		return;
	}

	// Array of tag points...

	u32 i;

	for( i=0; i<m_nTagPointCount; ++i ) {
		m_pWorldMesh->m_Xfm.m_MtxF.MulPoint( m_pTagPointArray_WS[i], m_pTagPointArray_MS[i] );
	}
}


// Called by the AI system when it has put the bot into a state in which the talking can start.
BOOL CBot::_AITalkStartCallback( u32 uTalkModeCBControl, void *pvData1, void *pvData2 ) {
	BOOL bDidIt = FALSE;
	if (uTalkModeCBControl == TMCB_READY) {
		if( bDidIt = CTalkSystem2::SubmitTalkRequest( (CBotTalkInst *)(pvData1), (CBot *)(pvData2) ) ) {
			// The request went through.
			++(((CBot *)(pvData2))->m_uActionTalkedCnt);
		}
	}
	return bDidIt;
}


// Called by a CTalkInst when a talking instance has completed.
void CBot::_AITalkEndCallback( CBotTalkInst *pTalkInst, CBot *pBot ) {
	ai_TalkModeEnd(pBot->AIBrain());
}


BOOL CBot::ActionCallback( CFWorldTracker *pTracker, FVisVolume_t *pVolume ) {
	// Check to see if it's a CEntity...
	if( pTracker->m_nUser != MESHTYPES_ENTITY ) {
		return TRUE;
	}

	CEntity *pEntity = (CEntity *)pTracker->m_pUser;
	CBot* ActionGuy = m_pCollBot;
	CEntityControl *pControls     = m_pCollBot->Controls();
	CHumanControl  *pHumanControl = NULL;
	if (ActionGuy->m_bControls_Human)
		pHumanControl = (CHumanControl *) pControls;

	if( (pEntity->IsActionable()) ) {
		BOOL bContinue = !pEntity->ActionNearby( m_pCollBot );
#if FANG_WINGC
		if( Fang_bPortDiag ) {
			DEVPRINTF("PORT-ACTION actor='%s' target='%s' consumed=%d\n",
				ActionGuy->Name(), pEntity->Name(), !bContinue);
		}
#endif

		if(	(bContinue==FALSE) && // new def, if a human actions, and the action is swallowed, the code here will eat the action msg
			pHumanControl ) {
			
			// Human controls... only once per frame per control if anyone is listening
			FASSERT(pHumanControl->m_nPadFlagsAction & GAMEPAD_BUTTON_1ST_PRESS_MASK);
			pHumanControl->m_nPadFlagsAction &= ~GAMEPAD_BUTTON_1ST_PRESS_MASK;
			ActionGuy->m_bControls_Action = 0;
		}

		return bContinue;
	} else {
		return TRUE;
	}
}


void CBot::ClassHierarchyRelocated( void *pIdentifier ) {
	CEntity::ClassHierarchyRelocated( pIdentifier );

	if( pIdentifier != m_pMoveIdentifier ) {
		// Ah, we were not responsible for this relocation...

		// Build the new xfm...
		m_pWorldMesh->m_Xfm.BuildFromMtx( m_MtxToWorld );

		// Store new position...
		m_MountPos_WS = m_MtxToWorld.m_vPos;
		m_MountPrevPos_WS = m_MountPos_WS;

		if( !(m_pStickyEntity && m_pStickyEntity->GetVerlet()) ) {
			// Compute new yaw and yaw-based vectors...
			f32 fYaw_WS = fmath_Atan( m_MtxToWorld.m_vFront.x, m_MtxToWorld.m_vFront.z );
			ChangeMountYaw( fYaw_WS );
		}

		// Update tracker...
		if( IsInWorld() ) {
			m_pWorldMesh->UpdateTracker();
		}
	}

	TagPoint_Update();

	if( m_nRecruitID >= 0 ) {
		if( IsDrawEnabled() ) {
			CWeaponRecruiter::UpdateRecruiterIconPos( this );
		}
	}
}


void CBot::ClassHierarchyWork( void ) {
	CEntity::ClassHierarchyWork();

	if( !IsOurWorkBitSet() ) {
		return;
	}

#if FANG_WINGC
	if( TypeBits() & ENTITY_BIT_BOTZOM ) {
		aiutils_EnsureFreedWastelandZombieFollowsPlayer( this );
	}
#endif

	if( m_pPartMgr->IsCreated() ) {
		m_pPartMgr->Work();
	}

	if( m_fPanicSecs > 0.0f ) {
		m_fPanicSecs -= FLoop_fPreviousLoopSecs;
		FMATH_CLAMPMIN( m_fPanicSecs, 0.0f );
	}

	// NKM  - So we can continue the tether animation when Glitch's work isn't being called
	if( m_nPossessionPlayerIndex != -1 && m_nOwnerPlayerIndex == -1 && m_pCurMech == NULL ) {
		if( Player_aPlayer[m_nPossessionPlayerIndex].m_pEntityOrig->TypeBits() & (ENTITY_BIT_BOTGLITCH | ENTITY_BIT_KRUNK) ) {
			CBot *pBot = (CBot*) Player_aPlayer[m_nPossessionPlayerIndex].m_pEntityOrig;

			if( !pBot->IsFallAnimationDone() ) {
				pBot->ClassHierarchyWork();
			}
		}
	}

	if (m_nPossessionPlayerIndex == -1 &&
		m_nRecruitID < 0 &&
		GetBotFlag_PowerDownUntilPosessed() &&
		!Power_IsPoweredDown() && 
		!Power_IsPoweringDown())
	{
		Power(FALSE);
	}

	if( m_fDataPortShockSecsRemaining > 0.0f ) {
		m_fDataPortShockSecsRemaining -= FLoop_fPreviousLoopSecs;

		if( m_fDataPortShockSecsRemaining <= 0.0f ) {
			m_fDataPortShockSecsRemaining = 0.0f;

			DataPort_Shock( FALSE );
			MobilizeBot();
		}
	}

	if( m_nRecruitID >= 0 ) {
		// We're recruited...

		CWeaponRecruiter::RecruitWork( this );
	}

	if( m_nPossessionPlayerIndex >= 0 ) {
		if( m_nScopeState != SCOPESTATE_NONE ) {
			if( m_fScopeUnzoomHitpointAccumulator > 0.0f ) {
				m_fScopeUnzoomHitpointTimer += FLoop_fPreviousLoopSecs;

				if( m_fScopeUnzoomHitpointTimer >= _SCOPE_ZOOM_CANCEL_NO_HITPOINT_SECS ) {
					m_fScopeUnzoomHitpointTimer -= _SCOPE_ZOOM_CANCEL_NO_HITPOINT_SECS;
					FMATH_CLAMPMAX( m_fScopeUnzoomHitpointTimer, _SCOPE_ZOOM_CANCEL_NO_HITPOINT_SECS );

					m_fScopeUnzoomHitpointAccumulator -= _SCOPE_ZOOM_CANCEL_NO_HITPOINT_DELTA;
					FMATH_CLAMPMIN( m_fScopeUnzoomHitpointAccumulator, 0.0f );
				}
			}
		}
	}

	// ME:  this check is in to prevent a situation where you are stuck in a bot that may be disabled, with no way to progress
	if( IsPossessed() && !IsPossessionExitable() && IsUseless() ) {
		Die();
	}

	if( m_fDoubleJumpTimer > 0.0f ) {
		m_fDoubleJumpTimer -= FLoop_fPreviousLoopSecs;
		FMATH_CLAMPMIN( m_fDoubleJumpTimer, 0.0f );
	}
}


// pUnitMountAimVec must not point straight up or straight down.
// If it does, this function simply returns without making any
// modifications. The same is true if the specified aim vector
// is outside the bot's pitch range.
//
// Returns TRUE if the new mount aim vector has been applied.
BOOL CBot::SetNewMountAimVec( const CFVec3A *pUnitMountAimVec ) {
	FASSERT( IsCreated() );

	if( fmath_Abs( pUnitMountAimVec->y ) >= 0.9999f ) {
		// Invalid vector...
		return FALSE;
	}

	// Valid vector...

	CFMtx43A Mtx;
	f32 fSin, fCos, fPitch;

	// Compute new mount matrix...
	Mtx.m_vFront = *pUnitMountAimVec;
	Mtx.m_vRight.UnitCrossYWithVec( Mtx.m_vFront );
	Mtx.m_vUp.Cross( Mtx.m_vFront, Mtx.m_vRight );
	Mtx.m_vPos = m_MtxToWorld.m_vPos;

	// Compute new pitch...
	fCos = Mtx.m_vFront.y;
	FMATH_CLAMP( fCos, -1.0f, 1.0f );
	fSin = fmath_Sqrt( 1.0f - fCos*fCos );
	fPitch = fmath_Atan( fSin, fCos ) - FMATH_HALF_PI;

	// Make sure the pitch isn't outside the bot's capable range...
	if( fPitch > m_pBotInfo_MountAim->fMountPitchDownLimit ) {
		return FALSE;
	}
	if( fPitch < m_pBotInfo_MountAim->fMountPitchUpLimit ) {
		return FALSE;
	}

	m_fMountPitch_WS = fPitch;

	// Compute new yaw and yaw-based vectors...
	f32 fYaw_WS = fmath_Atan( Mtx.m_vFront.x, Mtx.m_vFront.z );
	ChangeMountYaw( fYaw_WS );

	// Build the new xfm...
	m_pWorldMesh->m_Xfm.BuildFromMtx( Mtx );

	// Update tracker...
	if( IsInWorld() ) {
		m_pWorldMesh->UpdateTracker();
	}

	Relocate_RotXlatFromUnitMtx_WS( &Mtx, FALSE, m_pMoveIdentifier );

	return TRUE;
}


BOOL CBot::DoesBoneExist( cchar *pszBoneName ) {
	FASSERT( IsCreated() );

	if( pszBoneName == NULL ) {
		return FALSE;
	}

	if( m_pWorldMesh == NULL ) {
		return FALSE;
	}

	return (m_pWorldMesh->FindBone( pszBoneName ) != -1);
}


void CBot::DataPort_Open( BOOL bOpen ) {
	FASSERT( IsCreated() );

	if( m_bDataPortInstalled ) {
		if( !m_bDataPortOpen != !bOpen ) {
			m_bDataPortOpen = !!bOpen;

			if( IsInWorld() ) {
				if( m_pDataPortMeshEntity ) {
					m_pDataPortMeshEntity->SelectMesh( m_bDataPortOpen );
				}

				if( m_pDataPortEffectMeshEntity ) {
					if( bOpen && (m_nPossessionPlayerIndex < 0) ) {
						m_pDataPortEffectMeshEntity->Attach_UnitMtxToParent_PS( this, m_pBotInfo_Gen->pszDataPortBoneName );
						m_pDataPortEffectMeshEntity->AddToWorld();
					} else {
						m_pDataPortEffectMeshEntity->DetachFromParent();
						m_pDataPortEffectMeshEntity->RemoveFromWorld();
					}
				}
			}
		}
	}
}


void CBot::DataPort_SetDisguiseTime( f32 fDisguiseTime ) {
	FASSERT( IsCreated() );

	m_fDisguiseTime = fDisguiseTime;

	if( m_fDisguiseTime > 0.0f ) {
		m_fUnitDisguisedDecreaseRate = fmath_Inv( m_fDisguiseTime );
	}
}


void CBot::DataPort_ComputeUnitNormal( CFVec3A *pUnitNormal ) const {
	FASSERT( IsCreated() );

	f32 fMag2 = m_pDataPortBoneMtx->m_vY.MagSq();
	if( fMag2 > 0.0001f ) {
		pUnitNormal->Mul( m_pDataPortBoneMtx->m_vY, fmath_InvSqrt( fMag2 ) );
	} else {
		*pUnitNormal = CFVec3A::m_UnitAxisX;
	}
}


BOOL CBot::Recruit_CanBeRecruited( void ) const {
	FASSERT( IsCreated() );

	//  Because multiplayer is oh-so-special
	BOOL bSinglePlayer = MultiplayerMgr.IsSinglePlayer();

	if( bSinglePlayer && !(m_nBotFlags2 & BOTFLAG2_CLASS_CAN_BE_RECRUITED) ) {
		// This bot class cannot be recruited...
		return FALSE;
	}

	if( m_nBotFlags2 & BOTFLAG2_INST_CANNOT_BE_RECRUITED ) {
		// This bot instance cannot be recruited...
		return FALSE;
	}

	if( bSinglePlayer && !(m_nBotFlags2 & BOTFLAG2_DPORT_NOT_NEEDED_FOR_RECRUITMENT) && !DataPort_IsOpen() ) {
		// Cannot recruite bots with closed data ports...
		return FALSE;
	}

	if( DataPort_IsBeingShocked() ) {
		// Already being shocked...
		return FALSE;
	}

	if( DataPort_IsReserved() ) {
		// Data port is already reserved...
		return FALSE;
	}

	if( bSinglePlayer && (m_nOwnerPlayerIndex >= 0) ) {
		// Can't recruit original player bots...
		return FALSE;
	}

	return TRUE;
}


// If recruitment was successful, returns TRUE.
// If recruitment could not be done, returns FALSE.
BOOL CBot::Recruit( CBot *pRecruiterBot, const CFVec3A *pEpicenter_WS ) {
	if ( pRecruiterBot == this ) {
		// I can't recruit myself (can happen in multiplayer)
		return FALSE;
	}

	if( !Recruit_CanBeRecruited()
#if FANG_WINGC
		&& !aiutils_CanScriptRecruitWastelandZombie( this, pRecruiterBot, pEpicenter_WS )
#endif
		) {
		// This bot cannot be recruited...
		return FALSE;
	}

	if( CWeaponRecruiter::WasRecruitedBy( this, pRecruiterBot ) ) {
		// This bot already recruited me...
		return FALSE;
	}

	// Need to deal with sentries
	if( TypeBits() & ENTITY_BIT_SITEWEAPON ) {
		// Rat guns and pillboxes shouldn't be here...
		if( !IsFloorSentry() )
			return FALSE;

		// We are a floor sentry. If there is someone driving, kick him out
		((CBotSiteWeapon*)this)->SetSiteWeaponDriver( NULL, FALSE );
	}

	//if( aiutils_IsFriendly( this, pRecruiterBot ) ) {
	//	// This guy's already my buddy
	//	return FALSE;
	//}

	if( (m_nOwnerPlayerIndex < 0) && (m_nPossessionPlayerIndex >= 0) ) {
		// A player has control of this bot...
		ForceQuickDataPortUnPlug();
	}

	if( m_nRecruitID >= 0 ) {
		// This bot is already recruited...
		Unrecruit();
	}

	return CWeaponRecruiter::RecruitBot( this, pRecruiterBot, pEpicenter_WS );
}


void CBot::Unrecruit( void ) {
	if( m_nRecruitID < 0 ) {
		// Not recruited...
		return;
	}

	CWeaponRecruiter::UnrecruitBot( this, TRUE );
}




//--------------------------------------------------------------------------------------------------------------------------------
// Bot Velocity:
//--------------------------------------------------------------------------------------------------------------------------------

// Applies a velocity impulse in model space. The impulse will be applied first thing next frame.
void CBot::ApplyVelocityImpulse_MS( const CFVec3A &rVelocityImpulseVec_MS ) {
	// if we're out of body, ignore  (ME:  I added this so the titan's stomp rings wouldn't stack up impulses while glitch possessed them)
	if( !IsPlayerOutOfBody() ) {
		CFVec3A VelocityImpulseVec_WS;

		// Apply velocity impulse vector to our world space velocity...
		m_ImpulseVelocity_WS.Add( MS2WS( VelocityImpulseVec_WS, rVelocityImpulseVec_MS ) );
		FMATH_SETBITMASK( m_nBotFlags, BOTFLAG_VELOCITY_IMPULSE_VALID );
	}
}


// Applies a velocity impulse in world space. The impulse will be applied first thing next frame.
void CBot::ApplyVelocityImpulse_WS( const CFVec3A &rVelocityImpulseVec_WS ) {
	// Apply velocity impulse vector to our world space velocity...

	// if we're out of body, ignore  (ME:  I added this so the titan's stomp rings wouldn't stack up impulses while glitch possessed them)
	if( !IsPlayerOutOfBody() ) {
		m_ImpulseVelocity_WS.Add( rVelocityImpulseVec_WS );
		FMATH_SETBITMASK( m_nBotFlags, BOTFLAG_VELOCITY_IMPULSE_VALID );
	}
}


BOOL CBot::HandleVelocityImpulses( void ) {
	if( (m_nBotFlags & BOTFLAG_VELOCITY_IMPULSE_VALID) &&
#if FANG_WINGC
		!(m_nBotFlags2 & BOTFLAG2_DISABLE_VELOCITY_IMPULSES) &&
#endif
		!(m_nBotFlags & BOTFLAG_GLUED_TO_PARENT) &&
		!IsImmobileOrPending() ) {
		m_Velocity_WS.Add( m_ImpulseVelocity_WS );
		m_ImpulseVelocity_WS.Zero();

		FMATH_CLEARBITMASK( m_nBotFlags, BOTFLAG_VELOCITY_IMPULSE_VALID );

		return TRUE;
	} else {
		m_ImpulseVelocity_WS.Zero();

		FMATH_CLEARBITMASK( m_nBotFlags, BOTFLAG_VELOCITY_IMPULSE_VALID );

		return FALSE;
	}
}


void CBot::ZeroVelocity( void ) {
	FASSERT( IsCreated() );
	m_Velocity_WS.Zero();
	m_Velocity_MS.Zero();
	VelocityHasChanged();
}


// Call when m_Velocity_WS and m_Velocity_MS have changed.
// m_VelocityXZ_WS, m_VelocityXZ_MS must be be valid before calling this function.
// Does not update m_NormVelRotatedToXZ_WS, m_fSpeedAlongSurface_WS and m_fNormSpeedAlongSurface_WS.
void CBot::VelocityHasChanged( void ) {
	// Update m_VelocityXZ_WS...

	f32 fRunSpeedMult = m_pBotInfo_Walk ? m_pBotInfo_Walk->fInvMaxXlatVelocity * m_fOORunMultiplier : 1.0f;
	
	m_VelocityXZ_WS = m_Velocity_WS;
	m_VelocityXZ_WS.y = 0.0f;

	// Update m_NormVelocityXZ_WS...
	m_NormVelocityXZ_WS = m_Velocity_WS;
	m_NormVelocityXZ_WS.y = 0.0f;
	m_NormVelocityXZ_WS.Mul( fRunSpeedMult );

	// Compute speeds...
	m_fSpeed_WS = m_Velocity_WS.Mag();
	m_fSpeedXZ_WS = m_Velocity_WS.MagXZ();
	m_fNormSpeedXZ_WS = m_fSpeedXZ_WS * fRunSpeedMult; 
	m_fClampedNormSpeedXZ_WS = m_fNormSpeedXZ_WS;
	FMATH_CLAMPMAX( m_fClampedNormSpeedXZ_WS, 1.0f );

	// Update m_UnitVelocity_WS...
	if( m_fSpeed_WS > 0.0f ) {
		m_UnitVelocity_WS.Div( m_Velocity_WS, m_fSpeed_WS );
	} else {
		m_UnitVelocity_WS.Zero();
	}

	// Update m_UnitVelocityXZ_WS...
	if( m_fSpeedXZ_WS > 0.0f ) {
		m_UnitVelocityXZ_WS.Div( m_Velocity_WS, m_fSpeedXZ_WS );
		m_UnitVelocityXZ_WS.y = 0.0f;
	} else {
		m_UnitVelocityXZ_WS.Zero();
	}
}




//--------------------------------------------------------------------------------------------------------------------------------
// Bot Rotation:
//--------------------------------------------------------------------------------------------------------------------------------
f32 CBot::TakeMouseLookDelta( BOOL bPitch ) {
#if defined(MA_PC_INPUT)
	// A bot operating a vehicle or gun leaves the mouse to it (taking a delta consumes it).
	if( m_pCurMech ) {
		return 0.0f;
	}
	if( m_bControls_Human && m_nPossessionPlayerIndex >= 0 && !IsImmobileOrPending() &&
		!IsSleeping() && !IgnoreControls() && !MultiplayerMgr.IgnoreControls( m_nPossessionPlayerIndex ) ) {
		CPlayer &Player = Player_aPlayer[m_nPossessionPlayerIndex];
		// Vehicles and manned guns are operated, not possessed: the player's current entity stays the
		// driver, whose m_pCurMech is this.
		const BOOL bOperatedByPlayer = Player.m_pEntityCurrent && ( Player.m_pEntityCurrent->TypeBits() & ENTITY_BIT_BOT ) &&
			((CBot *)Player.m_pEntityCurrent)->GetCurMech() == this;
		if( Player.HasEntityControl() && ( Player.m_pEntityCurrent == this || bOperatedByPlayer ) ) {
			f32 fDelta = pcinput_TakeMouseAxis( Player.m_nControllerIndex, bPitch != FALSE );
			fDelta *= Player.ComputeLookSensitivityMultiplier();
			return bPitch && Player.GetInvertLook() ? -fDelta : fDelta;
		}
	}
#endif
	return 0.0f;
}

void CBot::HandlePitchMovement( void ) {
	const f32 fMousePitch = TakeMouseLookDelta( TRUE );
	f32 fZoomRevsPerSec, fMaxRevsPerSec;

	fMaxRevsPerSec = m_pBotInfo_MountAim->fMountPitchUpDownRevsPerSec;

	if( m_nScopeState == SCOPESTATE_LOOKING_THROUGH_SCOPE ) {
		// We're in scope mode...

		FASSERT( m_apWeapon[1] && m_apWeapon[1]->Type() == CWeapon::WEAPON_TYPE_SCOPE );
		fZoomRevsPerSec = ((CWeaponScope *)m_apWeapon[1])->GetBotAimRevsPerSec();

		if( fZoomRevsPerSec != 0.0f ) {
			fMaxRevsPerSec = fZoomRevsPerSec;
		}
	}

	if( m_nPossessionPlayerIndex >= 0 ) {
		fMaxRevsPerSec *= Player_aPlayer[m_nPossessionPlayerIndex].ComputeLookSensitivityMultiplier();
	}

	// Should we be applying bias?
	BOOL bApplyBias = ( m_nPossessionPlayerIndex >= 0 && (m_fControls_AimDown || m_fControls_RotateCW || m_fControlsHuman_Forward || m_fControlsHuman_StrafeRight) );
	if( !bApplyBias && m_nPossessionPlayerIndex >= 0 ) {
		// If we shouldn't be applying bias, then clear the pitch adjust so we don't think we've applied any
		Player_aPlayer[ m_nPossessionPlayerIndex ].m_fPitchAdjust = 0.f;
	}

	m_fMountPitchClampedNormDelta = fmath_Abs( m_fControls_AimDown );

	// Update look pitch...
	if( m_fControls_AimDown != 0.0f || bApplyBias || fMousePitch != 0.0f ) {
		f32 fAdjustment = fMousePitch;
		if( fMousePitch != 0.0f ) m_fMountPitchClampedNormDelta = 1.0f;
		if ( m_fControls_AimDown ) {
			fAdjustment += fMaxRevsPerSec * FMATH_2PI * m_fControls_AimDown * fmath_Abs(m_fControls_AimDown) * FLoop_fPreviousLoopSecs;
		}
		if ( bApplyBias ) {
			// This is a human player, so we want to apply biasing
			fAdjustment += Player_aPlayer[ m_nPossessionPlayerIndex ].m_fPitchAdjust;
		}

		m_fMountPitch_WS += fAdjustment;

		if( fAdjustment > 0.0f ) {
			if( m_fMountPitch_WS > m_pBotInfo_MountAim->fMountPitchDownLimit ) {
				m_fMountPitch_WS = m_pBotInfo_MountAim->fMountPitchDownLimit;
				m_fMountPitchClampedNormDelta = 0.0f;
			}
		} else {
			if( m_fMountPitch_WS < m_pBotInfo_MountAim->fMountPitchUpLimit ) {
				m_fMountPitch_WS = m_pBotInfo_MountAim->fMountPitchUpLimit;
				m_fMountPitchClampedNormDelta = 0.0f;
			}
		}
	}
}


void CBot::HandleYawMovement( void ) {
	f32 fAbsControlsRotateCW, fBaseRevsPerSec, fDelta = 0.0f, fZoomRevsPerSec;
	const f32 fMouseYaw = TakeMouseLookDelta( FALSE );
	f32 fYawRevsPerSec_WS;
	BOOL bUseScopeZoomSpeed = FALSE;

	fAbsControlsRotateCW = FMATH_FABS( m_fControls_RotateCW );
	m_fMountYawClampedNormDelta = 0.0f;

	// Should we be applying bias?
	BOOL bApplyBias = ( m_nPossessionPlayerIndex >= 0 && (m_fControls_AimDown || m_fControls_RotateCW || m_fControlsHuman_Forward || m_fControlsHuman_StrafeRight) );
	if( !bApplyBias && m_nPossessionPlayerIndex >= 0 ) {
		// If we shouldn't be applying bias, then clear the yaw adjust so we don't think we've applied any
		Player_aPlayer[ m_nPossessionPlayerIndex ].m_fYawAdjust = 0.f;
	}

	if( m_nScopeState == SCOPESTATE_LOOKING_THROUGH_SCOPE ) {
		// We're in scope mode...

		FASSERT( m_apWeapon[1] && m_apWeapon[1]->Type() == CWeapon::WEAPON_TYPE_SCOPE );
		fZoomRevsPerSec = ((CWeaponScope *)m_apWeapon[1])->GetBotAimRevsPerSec();

		if( fZoomRevsPerSec != 0.0f ) {
			bUseScopeZoomSpeed = TRUE;

			if( m_nPossessionPlayerIndex >= 0 ) {
				fZoomRevsPerSec *= Player_aPlayer[m_nPossessionPlayerIndex].ComputeLookSensitivityMultiplier();
			}

			// Shut off overdrive...
			m_fYawUnitOverdrive = 0.0f;
			FMATH_CLEARBITMASK( m_nBotFlags, BOTFLAG_YAW_CONTROL_OVERDRIVE );

			// Update yaw...
			if( fAbsControlsRotateCW > 0.0f || bApplyBias ) {
				fYawRevsPerSec_WS = fAbsControlsRotateCW * fAbsControlsRotateCW * fAbsControlsRotateCW * fZoomRevsPerSec;
				m_fMountYawClampedNormDelta = fmath_Div( fYawRevsPerSec_WS, m_pBotInfo_MountAim->fMountYawBaseRevsPerSec + m_pBotInfo_MountAim->fMountYawOverdriveRevsPerSec );

				// Restore sign to rotation rate
				if( m_fControls_RotateCW < 0.0f ) {
					fYawRevsPerSec_WS = -fYawRevsPerSec_WS;
				}

				// compute rotation delta for this frame and convert to radians
				fDelta = fYawRevsPerSec_WS * FLoop_fPreviousLoopSecs * FMATH_2PI;
				if( bApplyBias ) {
					// This is a human player, so we want to apply biasing
					fDelta += Player_aPlayer[ m_nPossessionPlayerIndex ].m_fYawAdjust;
				}

				ChangeMountYaw( m_fMountYaw_WS + fDelta );
			}
		}
	}

	if( !bUseScopeZoomSpeed ) {
		// Not in scope mode...

		if( fAbsControlsRotateCW > 0.0f || bApplyBias ) {
			if( !m_bControls_Human &&
				(m_nControlsBot_Flags & CBotControl::FLAG_QUICKROTATE_XZ) &&
				m_pBotInfo_MountAim->fMountYawAIQuickTurnRevsPerSec > 0.0f &&
				m_fControlsQuickRotateRads > 0.0f )
			{
				// AI is doing a Quick Turn...

				fYawRevsPerSec_WS = m_pBotInfo_MountAim->fMountYawAIQuickTurnRevsPerSec;
				m_fMountYawClampedNormDelta = 1.0f;

				m_fControlsQuickRotateRads -= fYawRevsPerSec_WS*FLoop_fPreviousLoopSecs * FMATH_2PI;
				if( m_fControlsQuickRotateRads < 0.0f ) {

					fDelta = m_fControlsQuickRotateRads+(fYawRevsPerSec_WS*FLoop_fPreviousLoopSecs * FMATH_2PI);
					FASSERT(fDelta >= 0.0f);

					fYawRevsPerSec_WS = 0.0f;
					m_fMountYawClampedNormDelta = 1.0f;

					// Restore sign to rotation rate
					if( m_fControls_RotateCW < 0.0f ) {
						fDelta = -fDelta;
					}
					
					ChangeMountYaw( m_fMountYaw_WS + fDelta );
					fDelta = 0.0f;
					m_fControlsQuickRotateRads = 0.0f;
				} else {
					// Restore sign to rotation rate
					if( m_fControls_RotateCW < 0.0f ) {
						fYawRevsPerSec_WS = -fYawRevsPerSec_WS;
					}

					// compute rotation delta for this frame and convert to radians
					fDelta = fYawRevsPerSec_WS * FLoop_fPreviousLoopSecs * FMATH_2PI;

					ChangeMountYaw( m_fMountYaw_WS + fDelta );
				}
			} else {
				fDelta = 0.f;

				if( fAbsControlsRotateCW > 0.0f ) {
					if (m_bControls_Human || IsPlayerBot()) {
						// Set overdrive mode bit flags.
						// (overdrive is faster yaw rotation, kicks in
						//  when yaw control stick is highly deflected).
						if( m_nBotFlags & BOTFLAG_YAW_CONTROL_OVERDRIVE ) {
							// We're in overdrive mode...

							if( fAbsControlsRotateCW < m_pBotInfo_MountAim->fMountYawUnitOutofControlOverdriveThresh ) {
								// Drop out of overdrive mode...
								FMATH_CLEARBITMASK( m_nBotFlags, BOTFLAG_YAW_CONTROL_OVERDRIVE );
							}
						} else {
							// We're not in overdrive mode...

							if( fAbsControlsRotateCW > m_pBotInfo_MountAim->fMountYawUnitIntoControlOverdriveThresh ) {
								// Enter overdrive mode...
								FMATH_SETBITMASK( m_nBotFlags, BOTFLAG_YAW_CONTROL_OVERDRIVE );
							}
						}

						// Handle transition in/out of overdrive,
						// and set base yaw rotation rate in either mode
						if( m_nBotFlags & BOTFLAG_YAW_CONTROL_OVERDRIVE ) {
							// We're in overdrive mode...

							if( m_fYawUnitOverdrive < 1.0f ) {
								m_fYawUnitOverdrive += m_pBotInfo_MountAim->fMountYawDeltaUnitIntoOverdrivePerSec * FLoop_fPreviousLoopSecs;
								FMATH_CLAMPMAX( m_fYawUnitOverdrive, 1.0f );
							}

							fBaseRevsPerSec = m_pBotInfo_MountAim->fMountYawBaseRevsPerSec;
						} else {
							// We're not in overdrive mode...

							if( m_fYawUnitOverdrive > 0.0f ) {
								m_fYawUnitOverdrive -= m_pBotInfo_MountAim->fMountYawDeltaUnitOutofOverdrivePerSec * FLoop_fPreviousLoopSecs;
								FMATH_CLAMPMIN( m_fYawUnitOverdrive, 0.0f );
							}

							fBaseRevsPerSec = fmath_Div( fAbsControlsRotateCW * fAbsControlsRotateCW * fAbsControlsRotateCW, m_pBotInfo_MountAim->fMountYawUnitIntoControlOverdriveThresh ) * m_pBotInfo_MountAim->fMountYawBaseRevsPerSec;
						}

						fYawRevsPerSec_WS = fBaseRevsPerSec + m_fYawUnitOverdrive*m_pBotInfo_MountAim->fMountYawOverdriveRevsPerSec;

						if( m_nPossessionPlayerIndex >= 0 ) {
							fYawRevsPerSec_WS *= Player_aPlayer[m_nPossessionPlayerIndex].ComputeLookSensitivityMultiplier();
						}

					} else {//AI control is much simpler and simply lerps between 0.0f and max yaw speed
						fYawRevsPerSec_WS = fAbsControlsRotateCW*fAbsControlsRotateCW*(m_pBotInfo_MountAim->fMountYawBaseRevsPerSec + m_pBotInfo_MountAim->fMountYawOverdriveRevsPerSec);
					}

					m_fMountYawClampedNormDelta = fmath_Div( fYawRevsPerSec_WS, m_pBotInfo_MountAim->fMountYawBaseRevsPerSec + m_pBotInfo_MountAim->fMountYawOverdriveRevsPerSec );

					// Restore sign to rotation rate
					if( m_fControls_RotateCW < 0.0f ) {
						fYawRevsPerSec_WS = -fYawRevsPerSec_WS;
					}

					// Compute rotation delta for this frame and convert to radians
					fDelta = fYawRevsPerSec_WS * FLoop_fPreviousLoopSecs * FMATH_2PI;
				}

				if( bApplyBias ) {
					// This is a human player, so we want to apply biasing
					fDelta += Player_aPlayer[ m_nPossessionPlayerIndex ].m_fYawAdjust;
				}

				ChangeMountYaw( m_fMountYaw_WS + fDelta );
			}
		}
	}

	// Raw mouse displacement is an angular delta, independent of the stick's
	// cubic response, acceleration/overdrive and frame duration.
	if( fMouseYaw != 0.0f ) {
		ChangeMountYaw( m_fMountYaw_WS + fMouseYaw );
		fDelta += fMouseYaw;
		fAbsControlsRotateCW = 1.0f; // Preserve the existing hip/torso turn response.
		m_fMountYawClampedNormDelta = 1.0f;
	}

	// pgm added this code so that bots torso will counter
	// rotate as his mount yaw is changed.
	// check HandleHipsAnimation to see the second half 
	// of this code
	if( fAbsControlsRotateCW > 0.0f && (m_pBotInfo_Walk != NULL) ) {
		if( !(m_nBotFlags & BOTFLAG_HIPSLACK_OVERRIDE) ) {
			if( IsReverseFacingForward() ) {
				if( m_fSpeedAlongSurface_WS == 0.0f) {
					if ((FMATH_FABS(m_fLegsYaw_MS) < m_pBotInfo_Walk->fHipYawSlackWhileStoppedThreshold) ||
						(FMATH_FABS(m_fLegsYaw_MS - fDelta) < m_pBotInfo_Walk->fHipYawSlackWhileStoppedThreshold) ) {
						m_fLegsYaw_MS -= fDelta;
					}
				}
			} else {
				if( FMATH_DEG2RAD( 180.0f ) - FMATH_FABS( m_fLegsYaw_MS ) < m_pBotInfo_Walk->fHipYawSlackWhileStoppedThreshold ||
					FMATH_DEG2RAD( 180.0f ) - FMATH_FABS( m_fLegsYaw_MS -fDelta) < m_pBotInfo_Walk->fHipYawSlackWhileStoppedThreshold )
				{
					m_fLegsYaw_MS -= fDelta;
				}
			}
		}
	}

	if( m_fRotationalImpactDeltaYaw != 0.0f ) {
		if( !IsImmobileOrPending() ) {
			if( m_fRotationalImpactDeltaYaw > 0.0f ) {
				m_fRotationalImpactDeltaYaw += m_fRotationalImpactSpeedMult * FLoop_fPreviousLoopSecs;
				FMATH_CLAMPMIN( m_fRotationalImpactDeltaYaw, 0.0f );
			} else {
				m_fRotationalImpactDeltaYaw += m_fRotationalImpactSpeedMult * FLoop_fPreviousLoopSecs;
				FMATH_CLAMPMAX( m_fRotationalImpactDeltaYaw, 0.0f );
			}

			ChangeMountYaw( m_fMountYaw_WS + m_fRotationalImpactDeltaYaw*FLoop_fPreviousLoopSecs );
		} else {
			m_fRotationalImpactDeltaYaw = 0.0f;
			m_fRotationalImpactSpeedMult = 0.0f;
		}
	}
}


void CBot::ChangeMountYaw( f32 fNewMountYaw_WS ) {
	// Normalize the angle from -PI to +PI...
	if( fNewMountYaw_WS > FMATH_PI ) {
		do {
			fNewMountYaw_WS -= FMATH_2PI;
		} while( fNewMountYaw_WS > FMATH_PI );
	} else if( fNewMountYaw_WS < -FMATH_PI ) {
		do {
			fNewMountYaw_WS += FMATH_2PI;
		} while( fNewMountYaw_WS < -FMATH_PI );
	}

	m_fMountYaw_WS = fNewMountYaw_WS;
	m_fMountYawSin_WS = fmath_Sin( fNewMountYaw_WS );
	m_fMountYawCos_WS = fmath_Cos( fNewMountYaw_WS );
	m_MountUnitFrontXZ_WS.x = m_fMountYawSin_WS;
	m_MountUnitFrontXZ_WS.z = m_fMountYawCos_WS;
}




//--------------------------------------------------------------------------------------------------------------------------------
// Bot Controls:
//--------------------------------------------------------------------------------------------------------------------------------

void CBot::SetControls( CEntityControl *pControl ) {
	FASSERT( pControl==NULL || pControl->Type()==CEntityControl::TYPE_HUMAN || pControl->Type()==CEntityControl::TYPE_BOT || pControl->Type()==CEntityControl::TYPE_NONE );

	CEntity::SetControls( pControl );

	if( Controls()->Type() == CEntityControl::TYPE_HUMAN ) {
		m_bControls_Human = TRUE;
	} else {
		m_bControls_Human = FALSE;
	}

	ZeroControls();
}


void CBot::ZeroControls( void ) {
	m_fControls_AimDown = 0.0f;
	m_fControls_RotateCW = 0.0f;
	m_fControls_Fire1 = 0.0f;
	m_fControls_Fire2 = 0.0f;
	m_bControls_Jump = FALSE;
	m_bControls_Melee = FALSE;
	m_bControls_Action = FALSE;
	m_fControlsHuman_Forward = 0.0f;
	m_fControlsHuman_StrafeRight = 0.0f;
	m_ControlBot_TargetPoint_WS.Zero();
	m_ControlsBot_XlatNormSpeedXZ_WS.Zero();
	m_ControlsBot_JumpVelocity_WS.Zero();
	m_bControlsBot_Jump2 = FALSE;
	m_bControlsBot_JumpVec = FALSE;
	m_nControlsBot_Flags = CBotControl::FLAG_NONE;
	m_nControlsBot_Buttons = CBotControl::BUTTONFLAG_NONE;
	m_fControlsQuickRotateRads = 0.0f;
	m_fControls_FlyUp = 0.0f;

	m_fControls_PrevFrameFire2 = 0.0f;
	m_fControlsQuickRotateRads = 0.0f;
	m_fInputIdleTime = 0.0f;
}

void CBot::UpdateIdle( void ) {
	//Exit out if we don't have any idle states available
	if( m_eIdleState == IDLESTATE_NO_IDLES_TO_PLAY ) {
		return;
	}

	// If we are flagged to not use idle animations, but we happen to be in an idle animation,
	// let the animation finish up
	if( m_eIdleState == IDLESTATE_NONE && !GetBotFlag_UseIdleAnimations() ) {
		return;
	}

	/////////////////////////////////////////////////////////////////////////////////////////////
	// FIRST, check to see what conditions dictate our terminating the idle animation immediately
	/////////////////////////////////////////////////////////////////////////////////////////////

	// not idle if fire buttons depressed AND ammo's available
	if ( (m_apWeapon[0] && (m_fControls_Fire1 > 0.0f) && m_apWeapon[0]->GetClipAmmo()) || 
		 (m_apWeapon[1] && (m_fControls_Fire2 > 0.0f) && m_apWeapon[1]->GetClipAmmo()) )
	{
		goto _EXIT_IMMEDIATE_NOT_IDLE;
	}

	// not idle if computer controlled
	if ( m_nPossessionPlayerIndex == -1 ) {
		goto _EXIT_IMMEDIATE_NOT_IDLE;
	}

	// not idle if zombiebotboss controlled
	if ( GetParent() && (GetParent()->TypeBits() & ENTITY_BIT_BOTZOMBIEBOSS) ) {
		goto _EXIT_IMMEDIATE_NOT_IDLE;
	}

	///////////////////////////////////////////////////////////////////////////////////////////
	// NEXT, check to see what conditions dictate our transitioning from the idle animations
	///////////////////////////////////////////////////////////////////////////////////////////

	// if we are already in the transtion state, continue to transition
	if (m_eIdleState==IDLESTATE_TRANSITIONING_TO_NONE){
		goto _EXIT_TRANSITION_NOT_IDLE;
	}
	// transition out if main buttons depressed
	if (m_bControls_Jump)
		goto _EXIT_TRANSITION_NOT_IDLE;

	if (m_bControls_Melee) 
		goto _EXIT_TRANSITION_NOT_IDLE;

	// transition out when walking
	if (m_fUnitWalkBlend != 0.0f) // this is currently set to run... should it be set to walk?
		goto _EXIT_TRANSITION_NOT_IDLE;
	// transition out when running
	if (m_fUnitRunBlend != 0.0f)
		goto _EXIT_TRANSITION_NOT_IDLE;
	// transition when flying
	if (m_fUnitFlyBlend != 0.0f)
		goto _EXIT_TRANSITION_NOT_IDLE;
	// transition out when cabling
	if (m_fUnitCableBlend != 0.0f)
		goto _EXIT_TRANSITION_NOT_IDLE;
	// transition out when driving
	if (m_pDrivingVehicle != NULL)
		goto _EXIT_TRANSITION_NOT_IDLE;
	// transition out when hand's aren't free
	if( m_apWeapon[0]->m_pInfo->nStanceType == CWeapon::STANCE_TYPE_TWO_HANDED_TORSO_TWIST )
		goto _EXIT_TRANSITION_NOT_IDLE;
	//transition out if we are in the air...
	if( IsInAir() )
		goto _EXIT_TRANSITION_NOT_IDLE; 
	//transition out if a user animation is playing
	if( UserAnim_IsLocked() )
		goto _EXIT_TRANSITION_NOT_IDLE;
	//transition out if a cutscene is being played...
	if( m_bCutscenePlaying )
		goto _EXIT_TRANSITION_NOT_IDLE;

	///////////////////////////////////////////////////////////////////////////////////////////
	// IF we are here, then it's valid to be in an idle state, we are playing or not
	///////////////////////////////////////////////////////////////////////////////////////////

	if (m_eIdleState==IDLESTATE_PLAYING) { // play out the idle

		f32 fUnitIdleBlend = m_Anim.IdleAnim_GetControlValue(m_nCurrentIdle);
		if (fUnitIdleBlend < 1.0f) {
			m_fIdleTransitionTimer += FLoop_fPreviousLoopSecs;
			fUnitIdleBlend = (m_pBotInfo_Idle[m_nCurrentIdle].fIdleToIdleDeltaSpeed * m_fIdleTransitionTimer);
			if (fUnitIdleBlend > 1.0f) {
				fUnitIdleBlend = 1.0f;
			}
			fUnitIdleBlend = fmath_UnitLinearToSCurve( fUnitIdleBlend );
			m_Anim.IdleAnim_SetControlValue(m_nCurrentIdle,fUnitIdleBlend);
			SetControlValue(ASI_STAND, 1.01f - fUnitIdleBlend);
		}

		//check to see if we are in the last quarter second of our animation
		//if we are then, set IDLESTATE_TRANSITIONING_TO_NONE so that we blend out 
		//of the last quarter second of the animation.
		f32 fTotalTimeRemaining = m_Anim.IdleAnim_GetTotalTime(m_nCurrentIdle) - m_Anim.IdleAnim_GetTime(m_nCurrentIdle);
		if( fTotalTimeRemaining < _IDLE_BLENDOUT_TIME ) {
			goto _EXIT_TRANSITION_NOT_IDLE;
		}

		//if we are here, we are not transitioning out yet, so update the animation and move on!
		m_Anim.IdleAnim_DeltaTime(m_nCurrentIdle);
	}
	else if (m_eIdleState==IDLESTATE_NONE) {// accumulate idle time
		// guess we are idle;
		m_fInputIdleTime += FLoop_fPreviousLoopSecs;
		for (u32 uIndex=0; uIndex<m_nCountIdles;uIndex++) {
			BotInfo_Idle_t *pIdle = &m_pBotInfo_Idle[uIndex];
			if (m_fInputIdleTime > pIdle->fIdleMinimumDelay) {
				if (fmath_RandomChance(pIdle->fIdleProbability)) {
					// do this idle
					SetHeadLookInterrupt();
					m_nCurrentIdle = uIndex;
					m_Anim.IdleAnim_SetControlValue(m_nCurrentIdle,0.0f);
					m_Anim.IdleAnim_UpdateUnitTime(m_nCurrentIdle,0.0f);
					m_eIdleState = IDLESTATE_PLAYING;
					m_fIdleTransitionTimer = 0.0f; //clear our transition timer.
					m_fInputIdleTime = 0.0f; // reset idle counter
					break; // picked one, don't keep searchin'
				}
			}
		}
	}
	return;

_EXIT_TRANSITION_NOT_IDLE:
	//if we are here, we want to transition OUT of the idle state over a period of a quarter of a second.
	if(m_eIdleState == IDLESTATE_PLAYING){
		m_fIdleTransitionTimer = 0.0f;
		m_eIdleState = IDLESTATE_TRANSITIONING_TO_NONE;
	}

	if( m_eIdleState == IDLESTATE_TRANSITIONING_TO_NONE ) {
		//figure out our blending values.
		m_fIdleTransitionTimer += FLoop_fPreviousLoopSecs;
		f32 fUnitIdleBlend = 1.0f - (m_fIdleTransitionTimer * _IDLE_OO_BLENDOUT_TIME );
		if( fUnitIdleBlend < 0.0f ){
			fUnitIdleBlend = 0.0f;
		}
		fUnitIdleBlend = fmath_UnitLinearToSCurve( fUnitIdleBlend ); //now run it through the s curve
		m_Anim.IdleAnim_SetControlValue(m_nCurrentIdle, fUnitIdleBlend);	//set the idle blend value
		SetControlValue(ASI_STAND, 1.01f - fUnitIdleBlend);					//set the standing control value 
		//now update the animation
		if (m_Anim.IdleAnim_DeltaTime(m_nCurrentIdle))	{
			//we have wrapped, which means we are done!
			goto _EXIT_IMMEDIATE_NOT_IDLE;
		}
	}
	else {
		//we are not in the transtion state, so do the work required when we are in a non-idle state
		goto _EXIT_IMMEDIATE_NOT_IDLE;
	}

	return;

_EXIT_IMMEDIATE_NOT_IDLE:
	if (m_eIdleState!=IDLESTATE_NONE)
	{
		ClearHeadLookInterrupt();
		m_eIdleState=IDLESTATE_NONE;
	}
	if (m_anAnimStackIndex[ASI_STAND] != -1)
		SetControlValue(ASI_STAND, 1.0f);
	m_Anim.IdleAnim_SetControlValue(m_nCurrentIdle,0.0f);
	m_fInputIdleTime = 0.0f;
	return;
}

void CBot::ParseControls( void ) {
	m_fControls_PrevFrameFire2 = m_fControls_Fire2;

	// Are we stunned?
	if( IsStunned() ) {
		m_fStunTime -= FLoop_fPreviousLoopSecs;

		if( m_fStunTime < 0.0f ) {
			// No longer stunned
			FMATH_CLEARBITMASK( m_nBotFlags, BOTFLAG_IS_STUNNED );
		}
	}

	if (IsSlowed() ) {
 		m_fSlowTimeSinceHit += FLoop_fPreviousLoopSecs;
		if( m_fSlowTimeSinceHit >= m_fSlowTimeOver) {
			// No longer slowed
			FMATH_CLEARBITMASK( m_nBotFlags, BOTFLAG_IS_SLOWED );
			m_fSlowUnitParalysis = 0.0f;
		}
		else if( m_fSlowTimeSinceHit > m_fSlowTimeSpeedUp) {
			FASSERT(m_fSlowTimeOver!=m_fSlowTimeSinceHit);
			m_fSlowUnitParalysis = 1.01f - fmath_Div((m_fSlowTimeSinceHit - m_fSlowTimeSpeedUp),(m_fSlowTimeOver-m_fSlowTimeSpeedUp));
		}
		else {
			FASSERT(m_fSlowUnitParalysis==1.0f);
		}
	}

	// Parse controls...
	if( !IsImmobileOrPending() && !IsSleeping() && (Controls()->Type() != CEntityControl::TYPE_NONE) ) {
		if( m_bControls_Human ) {
			// Human controls...

			CHumanControl *pHumanControl = (CHumanControl *)Controls();
			FASSERT( m_nPossessionPlayerIndex >= 0 );
			CPlayer *pPlayer = &Player_aPlayer[ m_nPossessionPlayerIndex ];

			m_fControls_AimDown = pPlayer->GetInvertLook() ? pHumanControl->m_fAimDown : -pHumanControl->m_fAimDown;
			m_fControls_RotateCW = pHumanControl->m_fRotateCW;

			if( IsReverseFacingForward() ) {
				m_fControls_Fire1 = pHumanControl->m_fFire1;
				m_fControls_Fire2 = pHumanControl->m_fFire2;
				m_bControls_Action = pHumanControl->m_nPadFlagsAction & GAMEPAD_BUTTON_1ST_PRESS_MASK;
			} else {
				m_fControls_Fire1 = 0.0f;
				m_fControls_Fire2 = 0.0f;
				m_bControls_Action = FALSE;
			}

			m_bControls_Jump = pHumanControl->m_nPadFlagsJump & GAMEPAD_BUTTON_1ST_PRESS_MASK;
			m_bControls_Melee = pHumanControl->m_nPadFlagsMelee & GAMEPAD_BUTTON_1ST_PRESS_MASK;

			m_fControlsHuman_Forward = pHumanControl->m_fForward;
			m_fControlsHuman_StrafeRight = pHumanControl->m_fStrafeRight;

			if( m_bControls_Action ) {
				NotifyActionButton();
			}

			m_fControls_FlyUp = 0.0f;
			if( pHumanControl->m_nPadFlagsJump ) {
				m_fControls_FlyUp -= 1.0f;
			} 
			
			if( pHumanControl->m_nPadFlagsAction ) {
				m_fControls_FlyUp += 1.0f;
			}

			#if _HALF_XLAT_SPEED_WHEN_IN_ZOOM_MODE
				if( m_nScopeState == SCOPESTATE_LOOKING_THROUGH_SCOPE ) {
					m_fControlsHuman_Forward *= 0.5f;
					m_fControlsHuman_StrafeRight *= 0.5f;
				}
			#endif

			if( !GetBotFlag_AllowFire() ) {
				m_fControls_Fire1 = 0.0f;
				m_fControls_Fire2 = 0.0f;
			}
		} else {
			// Bot controls...
			CBotControl *pBotControl = (CBotControl *)Controls();

			m_fControls_AimDown = pBotControl->m_fAimDown;
			m_fControls_RotateCW = pBotControl->m_fRotateCW;
			m_fControls_Fire1 = pBotControl->m_fFire1;
			m_fControls_Fire2 = pBotControl->m_fFire2;
			m_bControls_Jump = pBotControl->m_nButtons & CBotControl::BUTTONFLAG_JUMP1;
			m_bControls_Melee = pBotControl->m_nButtons & CBotControl::BUTTONFLAG_MELEE;

			m_bControls_Action = !!(pBotControl->m_nButtons & CBotControl::BUTTONFLAG_ACTION);
			
			if( m_bControls_Action ) {
				NotifyActionButton();
			}
			
			m_fControls_FlyUp = pBotControl->m_fFlyUp;

			BOOL bWasQuickRotating = !!(m_nControlsBot_Flags & CBotControl::FLAG_QUICKROTATE_XZ);
			m_ControlBot_TargetPoint_WS = pBotControl->m_TargetPoint_WS;
			m_ControlsBot_XlatNormSpeedXZ_WS = pBotControl->m_XlatNormSpeedXZ_WS;
			m_bControlsBot_Jump2 = pBotControl->m_nButtons & CBotControl::BUTTONFLAG_JUMP2;
			m_bControlsBot_JumpVec = pBotControl->m_nButtons & CBotControl::BUTTONFLAG_JUMPVEC;
			m_ControlsBot_JumpVelocity_WS = pBotControl->m_JumpVelocity_WS;
			m_nControlsBot_Flags = pBotControl->m_nFlags;
			m_nControlsBot_Buttons = pBotControl->m_nButtons;
			m_vControlBot_HeadLookPoint_WS = pBotControl->m_HeadLookPt_WS;

			if( !bWasQuickRotating &&
				m_nControlsBot_Flags & CBotControl::FLAG_QUICKROTATE_XZ ) {
				m_fControlsQuickRotateRads = 3.14f;	   //safety valve
			}

			#if _HALF_XLAT_SPEED_WHEN_IN_ZOOM_MODE
				if( m_nScopeState == SCOPESTATE_LOOKING_THROUGH_SCOPE ) {
					m_ControlsBot_XlatNormSpeedXZ_WS.Mul( 0.5f );
				}
			#endif

			if( pBotControl->m_nFlags & CBotControl::FLAG_FORCE_JUMP1 ) {
				pBotControl->m_nFlags &= ~CBotControl::FLAG_FORCE_JUMP1;
				m_bControls_Jump = TRUE;
			}

			if( pBotControl->m_nFlags & CBotControl::FLAG_FORCE_JUMP2 ) {
				pBotControl->m_nFlags &= ~CBotControl::FLAG_FORCE_JUMP2;
				m_bControlsBot_Jump2 = TRUE;
			}

			if( !GetBotFlag_AllowFire() ) {
				m_fControls_Fire1 = 0.0f;
				m_fControls_Fire2 = 0.0f;
			}
		}

		if( HasNoLegs() ) {
			m_bControls_Jump = FALSE;
			m_bControls_Melee = FALSE;
			m_fControlsHuman_Forward = 0.0f;
			m_fControlsHuman_StrafeRight = 0.0f;
			m_ControlsBot_XlatNormSpeedXZ_WS.Zero();
			m_ControlsBot_JumpVelocity_WS.Zero();
			m_bControlsBot_Jump2 = FALSE;
			m_bControlsBot_JumpVec = FALSE;

			FMATH_CLEARBITMASK( m_nControlsBot_Flags, CBotControl::FLAG_FORCE_SNEAK | CBotControl::FLAG_PANIC | CBotControl::FLAG_ALERT | CBotControl::FLAG_QUICKROTATE_XZ );
			FMATH_CLEARBITMASK( m_nControlsBot_Flags, CBotControl::FLAG_HOP_LEFT | CBotControl::FLAG_HOP_RIGHT | CBotControl::FLAG_HOP_VERT | CBotControl::FLAG_ROLL_LEFT | CBotControl::FLAG_ROLL_RIGHT );
		}
	} else {
		ZeroControls();
	}
}


// Computes the following from the input controls:
//   m_XlatStickNormVecXZ_MS
//   m_XlatStickNormVecXZ_WS
//   m_fXlatStickNormMag

void CBot::ComputeXlatStickInfo( void ) {
	// Compute control stick vector and magnitude...

	// limit the stick xlat if alert mode is on, so that bot won't ever try to run
	f32 fMaxStickXlatLimit;

	if( m_pBotInfo_Walk ) {
		fMaxStickXlatLimit = m_pBotInfo_Walk->fMinWalkNormVelocity + (1.0f-m_pBotInfo_Walk->fMinWalkNormVelocity)*!IsAlertOn();
	} else {
		fMaxStickXlatLimit	= 1.0f;
	}

	if( m_bControls_Human || (m_nMoveState != BOTMOVESTATE_NONE)) {
		// Human control interface... or hopping which slams controls in model space

		m_XlatStickNormVecXZ_MS.x = m_fControlsHuman_StrafeRight;
		m_XlatStickNormVecXZ_MS.z = m_fControlsHuman_Forward;

		// NKM - Added to fake player input
		if( m_pStickModifier_MS ) {
			if( m_XlatStickNormVecXZ_MS.x == 0.0f && m_XlatStickNormVecXZ_MS.z == 0.0f && 
				m_pStickModifier_MS->x != 0.0f && m_pStickModifier_MS->z != 0.0f ) {
				// No input on the stick, but we are being pushed
				FMATH_SETBITMASK( m_nBotFlags, BOTFLAG_NO_FEET_ANIM );	
			}

			m_XlatStickNormVecXZ_MS.x += m_pStickModifier_MS->x;
			m_XlatStickNormVecXZ_MS.z += m_pStickModifier_MS->z;
		}

		//slam the controls to 0.0 if hopping. since all movement will come via an impulse
		if (m_nMoveState != BOTMOVESTATE_NONE) {
			m_XlatStickNormVecXZ_MS.z  = 0.0f;
			m_XlatStickNormVecXZ_MS.x = 0.0f;

			if (m_nMoveState == BOTMOVESTATE_BLENDIN && m_fUnitHop > 0.85f)
			{
				// bots without walk info shouldn't be hopping
				FASSERT( m_pBotInfo_Walk != NULL );

				if (m_nMoveType == BOTMOVETYPE_ROLL_LEFT) {
					m_XlatStickNormVecXZ_MS.x = -m_pBotInfo_Walk->fRollStickForceWhileRolling;
				}
				else if (m_nMoveType == BOTMOVETYPE_ROLL_RIGHT) {
					m_XlatStickNormVecXZ_MS.x = m_pBotInfo_Walk->fRollStickForceWhileRolling;;
				}
			}

		}

		m_fXlatStickNormMag = m_XlatStickNormVecXZ_MS.MagXZ();
		MS2WS( m_XlatStickNormVecXZ_WS, m_XlatStickNormVecXZ_MS );

		// Compute control stick unit vector...
		if( m_fXlatStickNormMag > 1.0f ) {
			// The physical hole where the analog stick resides isn't exactly round, so it's possible the magnitude
			// of our control stick vector is greater than one...

			m_XlatStickNormVecXZ_MS.Div( m_fXlatStickNormMag );
			m_fXlatStickNormMag = 1.0f;
			m_XlatStickUnitVecXZ_MS = m_XlatStickNormVecXZ_MS;
		} else if( m_fXlatStickNormMag > 0.00001f ) {
			// Control stick pushed, but its magnitude is not greater than one...

			m_XlatStickUnitVecXZ_MS.Div( m_XlatStickNormVecXZ_MS, m_fXlatStickNormMag );
		} else {
			// Control stick isn't being pushed at all. We just set it to the unit Z axis so it continues to have unit length...
			m_XlatStickNormVecXZ_MS.Zero();
			m_XlatStickUnitVecXZ_MS.Zero();
			m_fXlatStickNormMag = 0.0f;
		}

		// Compute world space version of m_XlatStickUnitVecXZ_MS...
		MS2WS( m_XlatStickUnitVecXZ_WS, m_XlatStickUnitVecXZ_MS );
	} else {
		// Bot control interface...

		m_XlatStickNormVecXZ_WS.x = m_ControlsBot_XlatNormSpeedXZ_WS.x;
		m_XlatStickNormVecXZ_WS.z = m_ControlsBot_XlatNormSpeedXZ_WS.z;

		// NKM - Added to fake player input
		if( m_pStickModifier_MS ) {
			if( m_XlatStickNormVecXZ_MS.x == 0.0f && m_XlatStickNormVecXZ_MS.z == 0.0f && 
				m_pStickModifier_MS->x != 0.0f && m_pStickModifier_MS->z != 0.0f ) {
				// No input on the stick, but we are being pushed
				FMATH_SETBITMASK( m_nBotFlags, BOTFLAG_NO_FEET_ANIM );	
			}

			m_XlatStickNormVecXZ_MS.x += m_pStickModifier_MS->x;
			m_XlatStickNormVecXZ_MS.z += m_pStickModifier_MS->z;
		}

		m_fXlatStickNormMag = m_XlatStickNormVecXZ_WS.MagXZ();
		WS2MS( m_XlatStickNormVecXZ_MS, m_XlatStickNormVecXZ_WS );

		// Compute control stick unit vector...
		if( m_fXlatStickNormMag > 0.00001f ) {
			// Control stick is pushed...

			if( m_fXlatStickNormMag > 1.0f ) {
				// m_XlatStickNormVecXZ_WS need not be unit, so it's possible the magnitude is greater than one...

				m_XlatStickNormVecXZ_WS.Div( m_fXlatStickNormMag );
				m_fXlatStickNormMag = 1.0f;
				m_XlatStickUnitVecXZ_WS = m_XlatStickNormVecXZ_WS;
			} else if( m_fXlatStickNormMag > fMaxStickXlatLimit ) {
				// m_XlatStickNormVecXZ_WS need not be unit, so it's possible the magnitude is greater than one...
				m_XlatStickNormVecXZ_WS.SafeUnitAndMag(m_XlatStickNormVecXZ_WS);
				m_fXlatStickNormMag = fMaxStickXlatLimit;
				m_XlatStickUnitVecXZ_WS = m_XlatStickNormVecXZ_WS;
			} else {
				// Control stick pushed, but its magnitude is not greater than one...

				m_XlatStickUnitVecXZ_WS.Div( m_XlatStickNormVecXZ_WS, m_fXlatStickNormMag );
			}

			if( (m_nControlsBot_Flags & CBotControl::FLAG_FORCE_SNEAK) && m_anAnimStackIndex[ASI_SNEAK]!=-1 ) {
				FASSERT( m_pBotInfo_Walk != NULL );
				// Force-sneak mode is enabled...

				m_fXlatStickNormMag = m_pBotInfo_Walk->fMaxSneakStickMag * 0.5f;
				m_XlatStickNormVecXZ_WS.Mul( m_XlatStickUnitVecXZ_WS, m_fXlatStickNormMag );
			}
		} else {
			// Control stick isn't being pushed at all. We just set it to the unit Z axis so it continues to have unit length...

			m_XlatStickNormVecXZ_WS.Zero();
			m_XlatStickUnitVecXZ_WS.Zero();
			m_fXlatStickNormMag = 0.0f;
		}

		// Compute model space version of m_XlatStickUnitVecXZ_WS...
		WS2MS( m_XlatStickUnitVecXZ_MS, m_XlatStickUnitVecXZ_WS );
	}

	// Apply the paralysis effect 
	if (IsSlowed())
	{
		m_XlatStickUnitVecXZ_WS.Mul(1.01f-m_fSlowUnitParalysis * m_fSlowEffectMult); 
		m_XlatStickUnitVecXZ_MS.Mul(1.01f-m_fSlowUnitParalysis * m_fSlowEffectMult); 
	}
}


//--------------------------------------------------------------------------------------------------------------------------------
// Bot Ground Movement:
//--------------------------------------------------------------------------------------------------------------------------------

#define _SHOW_DEBUG_GRAPHICS		(FALSE & (!FANG_PRODUCTION_BUILD))

// Computes the following:
//   m_nState
//   m_SurfaceUnitNorm_WS
//   m_fBestCaseSurfaceUnitNormY_WS
//   m_FeetToGroundCollImpact
//   m_pStickyEntity
//   m_LastStickyEntityPos
void CBot::HandleCollision( void ) {
	BOOL bIsOnGround = (m_nState == STATE_GROUND);
	m_nState = STATE_AIR;
	m_uCollisionStatus = COLLSTATE_NO_COLLISION;
	m_pActionableEntityNearby = NULL;

	if( m_pWorldMesh->IsCollisionFlagSet() == FALSE ) {

		if( IsNoCollideStateAir() ) {
			m_nState = STATE_AIR;
		} else {
			m_nState = STATE_GROUND;
		}

		// (fixes a bug where ComputeMovingSurfaceVector() references an uninitialized m_SurfaceUnitNorm_WS.)
		m_SurfaceUnitNorm_WS.Set( CFVec3A::m_UnitAxisY );

		return;
	}

	// ME:  if we're glued to our parent, the parent will tell us where we need to be
	if( (m_nBotFlags & BOTFLAG_GLUED_TO_PARENT) || (m_nBotFlags2 & BOTFLAG2_UNDER_CONSTRUCTION) ) {
		// If we are Glitch and are panicking, this is multiplayer, and we want
		// to force the air animation.
		m_nState = ((TypeBits() & ENTITY_BIT_BOTGLITCH) && (m_nBotFlags & BOTFLAG_FORCED_PANIC)) ? STATE_AIR : STATE_GROUND;

		// Update immobile flag, fixes bug where glued bots get hung in power state POWERSTATE_WAITING_FOR_IMMOBILIZED.
		if( m_nBotFlags & BOTFLAG_IMMOBILIZE_PENDING ) {
			if( (m_nState == STATE_GROUND) && (m_fSpeed_WS == 0.0f) ) {
				FMATH_CLEARBITMASK( m_nBotFlags, BOTFLAG_IMMOBILIZE_PENDING );
				FMATH_SETBITMASK( m_nBotFlags, BOTFLAG_IS_IMMOBILE );
			}
		}

		return;
	}

	// We leave m_nTractionMode, m_fUnitTraction, and m_fNormSpeedAlongSurface_WS alone so that 
	// we can continue to drive our ground animations until we transition into the air animation!
	CEntity* pStickyParent = NULL;
	cchar* pszStickyParentBoneName = NULL;
	FCollImpact_t *pFootImpact, *pImpact, *pWallImpact;
	CFVec3A PrevToCurrentPos_WS, PushAmount_WS, PrevSphereCenter_WS;
	u32 i, j;
	f32 fDot, fCollSphereY;
	s32 nSurfaceType = 0xffffffff;

	m_pCollBot = this;
	FWorld_nTrackerSkipListCount = 0;
	AppendTrackerSkipList();

	fCollSphereY = ComputeCollSphereY();

#if _SHOW_DEBUG_GRAPHICS
	BOOL bShowDebugGraphics = FALSE;
	if ( stricmp( m_pWorldMesh->m_pMesh->szName, "grdggltch00" ) == 0 ) {

		bShowDebugGraphics = TRUE;
	}
#endif

	// Feet per second that we force the player downward when on the ground to make sure 
	// that he stays connected to the ground at all times
	#define __GROUND_STICKY_HACKY		10.0f 
	#define __MAX_BOTS_COLLIDED_WITH	4

	CFVec3A vMovement, vPushVec_WS, vAdjust;
	CFCollData CollData;

	m_CollSphere.m_Pos.x = m_pBotInfo_Gen->fCollSphere1X_MS + m_MountPrevPos_WS.x;
	m_CollSphere.m_Pos.y = fCollSphereY + m_MountPrevPos_WS.y - 0.9f;
	m_CollSphere.m_Pos.z = m_pBotInfo_Gen->fCollSphere1Z_MS + m_MountPrevPos_WS.z;
	m_CollSphere.m_fRadius = m_pBotInfo_Gen->fCollSphere1Radius_MS;

	// For recording the bots we collide with so that we don't add multiple forces to them
	CBot *apBotsCollided[__MAX_BOTS_COLLIDED_WITH];
	u32 nBotsCollidedCount = 0;

	f32 fSnapToSurfaceDist;
	if ( bIsOnGround ) {
		fSnapToSurfaceDist = __GROUND_STICKY_HACKY * FLoop_fPreviousLoopSecs; 
	} else {
		fSnapToSurfaceDist = 0.f; 
	}
	m_MountPos_WS.y -= fSnapToSurfaceDist;

	// Determine the movement that is to be applied
	vMovement.Sub( m_MountPos_WS, m_MountPrevPos_WS );

	#if _SHOW_DEBUG_GRAPHICS
		u32 nTests = 0;
		u32 nTestsWithResults = 0;
		BOOL bInitiallyInCollision = FALSE;
		if ( bShowDebugGraphics ) {
			CFColorRGBA MoveColor( 1.f, 0.f, 1.f, 1.f );
			CFVec3 vEnd = vMovement.v3;
			if ( vEnd.Mag2() ) {
				vEnd.Unitize();
				vEnd.Mul( m_CollSphere.m_fRadius * 2.f ).Add( m_CollSphere.m_Pos );
				fdraw_DevLine( &m_CollSphere.m_Pos, &vEnd, &MoveColor, &MoveColor );
			}
		}
	#endif

	CollData.pMovement = &vMovement;

	// Fill out the CollData
	CollData.nCollMask = (m_nPossessionPlayerIndex < 0) ? FCOLL_MASK_COLLIDE_WITH_NPCS : FCOLL_MASK_COLLIDE_WITH_PLAYER;
	CollData.nFlags = FCOLL_DATA_IGNORE_BACKSIDE;
	CollData.nStopOnFirstOfCollMask = FCOLL_MASK_NONE;
	CollData.pCallback = NewTrackerCollisionCallback;
	CollData.nTrackerUserTypeBitsMask = ~(ENTITY_BIT_BOT/*|ENTITY_BIT_BOTPROBE*/|ENTITY_BIT_BOTSWARMER);
	CollData.pLocationHint = m_pWorldMesh;
	CollData.nTrackerSkipCount = FWorld_nTrackerSkipListCount;
	CollData.ppTrackerSkipList = FWorld_apTrackerSkipList;

	#if _SHOW_DEBUG_GRAPHICS
		if ( bShowDebugGraphics ) {
			CFColorRGBA Color( 0.f, 1.f, 0.f, 1.f ); 
			fdraw_DevSphere( &m_CollSphere.m_Pos, m_CollSphere.m_fRadius, &Color, 2, 2, 2 );
			Color.Set( 1.f, 0.f, 0.f, 1.f );
			CFVec3A vDestination;
			vDestination.v3 = m_CollSphere.m_Pos + vMovement.v3;
			fdraw_DevSphere( &vDestination.v3, m_pBotInfo_Gen->fCollSphere1Radius_MS, &Color, 2, 2, 2 );
		}
	#endif

	// We're going to attempt to resolve the move by performing up to three collision detections
	CFVec3A vMoveAttempt, vOriginalPos;

	vOriginalPos.v3 = m_CollSphere.m_Pos;

	// Distance from sphere center in XZ of surfaces the bot can walk onto
	f32 fWalkableRange = m_CollSphere.m_fRadius * m_CollSphere.m_fRadius * 0.8f;

	f32 fXZDistToFootImpactSq;
	CFVec3A vSpherePosAtFootImpact;

	// We are going to iterate up to 5 times to attempt to find a safe move to perform
	for( i = 0; i < 5 && CollData.pMovement; i++ ) {
		vMoveAttempt.Set( *CollData.pMovement );

		m_fMoveLength = vMoveAttempt.MagSq();
		if ( m_fMoveLength > 0.001f ) {
			m_vMoveNormal.Mul( vMoveAttempt, fmath_InvSqrt( m_fMoveLength ) );
		} else {
			m_fMoveLength = 0.f;
		}

		#if _SHOW_DEBUG_GRAPHICS
			nTests++;
		#endif

		// Clear the collision buffer and check the collision
		fcoll_Clear();
		fcoll_Check( &CollData, &m_CollSphere );

		// If there is no collision, we can just exit the loop
		if( FColl_nImpactCount == 0 ) {
			m_CollSphere.m_Pos += CollData.pMovement->v3;
			#if _SHOW_DEBUG_GRAPHICS
				if ( bShowDebugGraphics ) {
					CFColorRGBA MoveColor( 0.f, 1.f, 0.f, 1.f );
					CFVec3 vEnd = vMovement.v3;
					if ( vEnd.Mag2() ) {
						vEnd.Unitize();
						vEnd.Mul( m_CollSphere.m_fRadius * 2.f ).Add( m_CollSphere.m_Pos );
						fdraw_DevLine( &m_CollSphere.m_Pos, &vEnd, &MoveColor, &MoveColor );
					}
				}
			#endif
			break;
		}

		#if _SHOW_DEBUG_GRAPHICS
			if ( bShowDebugGraphics ) {
				if ( i > 0 )
				{
					CFColorRGBA MoveColor( 1.f, 1.f, 0.f, 1.f );
					CFVec3 vEnd = vMovement.v3;
					if ( vEnd.Mag2() ) {
						vEnd.Unitize();
						vEnd.Mul( m_CollSphere.m_fRadius * 2.f ).Add( m_CollSphere.m_Pos );
						fdraw_DevLine( &m_CollSphere.m_Pos, &vEnd, &MoveColor, &MoveColor );
					}
				}
				nTestsWithResults++;
			}
		#endif

		// Gather the relevant collisions
		CFVec3A vDiff;
		pFootImpact = NULL;
		pWallImpact = NULL;
		pImpact = NULL;
		if ( i == 0 ) {
			m_fBestCaseSurfaceUnitNormY_WS = FColl_aImpactBuf[0].UnitFaceNormal.y;
		}

		// Collect the relevant walkable impact
		for ( j = 0; j < FColl_nImpactCount; j++ ) {

			pImpact = &FColl_aImpactBuf[j];

			#if _SHOW_DEBUG_GRAPHICS
				if ( bShowDebugGraphics ) {
					fdraw_DevSphere( &pImpact->ImpactPoint.v3, 0.1f );
				}
			#endif

			if ( i == 0 ) {
				if( pImpact->UnitFaceNormal.y > m_fBestCaseSurfaceUnitNormY_WS ) {
					m_fBestCaseSurfaceUnitNormY_WS = pImpact->UnitFaceNormal.y;
				}
			}

			if ( pImpact->UnitFaceNormal.y < 0.5f || pImpact->nSourceBoneIndex == 0xff ) {
				// Consider only walkable surfaces, based on the normal of the poly and
				// if pImpact->nSphereIndex == 0xff then we know that this collision
				// came from the tracker callback, is a bot collision and should not
				// be something that this bot can stand on, regardless of the normal
				continue;
			}

			f32 fXZDistSq, fTemp;
			if ( pImpact->fUnitImpactTime != -1.f ) {

				vDiff.Mul( *CollData.pMovement, pImpact->fUnitImpactTime );
				vDiff.v3 += m_CollSphere.m_Pos;
			} else {

				vDiff.Set( m_CollSphere.m_Pos );
			}
			fXZDistSq = pImpact->ImpactPoint.x - vDiff.x;
			fXZDistSq *= fXZDistSq;
			fTemp = pImpact->ImpactPoint.z - vDiff.z;
			fXZDistSq += (fTemp * fTemp);

			CFWorldTracker *pHitTracker = (CFWorldTracker *)pImpact->pTag;
			if ( pHitTracker && pHitTracker->m_nUser == MESHTYPES_ENTITY ) {
				if ( !((CEntity *)pHitTracker->m_pUser)->IsWalkable() ) {
					// Cannot walk on this entity so force fXZDistSq to a really big number so we will not walk 
					// on it and it will get passed through to get the wall collision treatment
					fXZDistSq = FMATH_MAX_FLOAT;
				}
			}

			if ( fXZDistSq > fWalkableRange ) {
				// This impact is not within the walkable range, so use it as a walkable collision
				continue;
			}

			// This impact should not be used for non-walkable collision so set a sign
			pImpact->nSourceBoneIndex = 0xfe;

			if ( pFootImpact ) {
				// Is this impact more important than the current foot impact?

				fTemp = (pImpact->fUnitImpactTime - pFootImpact->fUnitImpactTime);
				if ( fTemp > 0.0025f ) {
					// This collision occurs later than our current
					continue;
				}

				if ( fmath_Abs( fTemp ) < 0.0025f ) {
					fTemp = (pImpact->fImpactDistInfo - pFootImpact->fImpactDistInfo);
					if ( fTemp < -0.0025f ) {
						// This collision requires less push than the others
						continue;
					}
					
					if ( fmath_Abs( fTemp ) < 0.0025f ) {
						if ( pImpact->UnitFaceNormal.y < pFootImpact->UnitFaceNormal.y ) {
							// This collision has a less vertical face normal.
							continue;
						}
					}
				}
			}

			vAdjust.v3 = m_CollSphere.m_Pos - pImpact->aTriVtx[0].v3;

			fDot = vAdjust.Dot( pImpact->UnitFaceNormal );
			if( fDot < 0.0f ) {
				continue;
			}

			fDot = pImpact->UnitFaceNormal.Dot( vMoveAttempt );
			if ( fDot > 0.f ) {
				continue;
			}

			pFootImpact = pImpact;
			fXZDistToFootImpactSq = fXZDistSq;
			vSpherePosAtFootImpact.Set( vDiff );
		}

		// Collect the relevant non-walkable impact
		for ( j = 0; j < FColl_nImpactCount; j++ ) {

			pImpact = &FColl_aImpactBuf[j];

			if ( pImpact->nSourceBoneIndex == 0xfe ) {
				// This is a walkable impact and should therefore not be used for non-walkable collision
				continue;
			}

			// Otherwise we treat this as a poly that is not walkable because it is too steep or too far away
			if ( pWallImpact ) {
				if ( pImpact->fUnitImpactTime > pWallImpact->fUnitImpactTime ) {
					continue;
				}

				if ( pImpact->fUnitImpactTime == pWallImpact->fUnitImpactTime && pImpact->fImpactDistInfo > pWallImpact->fImpactDistInfo ) {
					continue;
				}
			}

			// Bot collisions should always be used
//				if ( pImpact->nSphereIndex != 0xff ) 
			{
				fDot = pImpact->UnitFaceNormal.Dot( vMoveAttempt );
				if ( fDot > 0.f ) {
					// We are moving away from this poly, so we will ignore it
					continue;
				}

				fDot = pImpact->PushUnitVec.Dot( vMoveAttempt );
				if( fDot > 0.01f ) {
					// The push is in the direction of the movement, so we will ignore it
					continue;
				}
			}

			#if 0
				// This piece of code makes sure that non-walkable collisions that are at 
				// the same point as the current walkable collision are ignored:
				if ( pFootImpact && (pFootImpact->ImpactPoint.y - pImpact->ImpactPoint.y) > -0.01f ) {
					continue;
				}
			#else
				// This piece of code makes sure that non-walkable collisions that are below 
				// the center of the collision sphere are ignored:
				f32 fDiffY = pImpact->PushUnitVec.y - pImpact->UnitFaceNormal.y;
				if( fmath_Abs( fDiffY ) > 0.01f ) {
					if( pImpact->PushUnitVec.y >= 0.07f ) {
						continue;
					}
				}
			#endif

			pWallImpact = pImpact;
		}

		if ( !pWallImpact && !pFootImpact ) {
			m_CollSphere.m_Pos += CollData.pMovement->v3;
			break;
		}

		#if _SHOW_DEBUG_GRAPHICS
			if ( bShowDebugGraphics && ((pFootImpact && pFootImpact->fUnitImpactTime == -1.f)
				|| (pWallImpact && pWallImpact->fUnitImpactTime == -1.f)) ) {
				bInitiallyInCollision = TRUE;
			}
		#endif

		if ( (pWallImpact && pWallImpact->fUnitImpactTime == -1.f) || (pFootImpact && pFootImpact->fUnitImpactTime == -1.f) ) {

			// When we are already in collision, we apply both collisions, if they are both already in collision
			pImpact = pFootImpact;
			if ( pWallImpact && pWallImpact->fUnitImpactTime == -1.f ) {
				if ( !pFootImpact || pFootImpact->fUnitImpactTime != -1.f || pFootImpact->fImpactDistInfo < pWallImpact->fImpactDistInfo ) {
					f32 fTemp;
					// We are currently in a wall impact

					if ( bIsOnGround ) {

						// The bot is on the ground.  Is the push vector trying to push us into the ground?
						fTemp = m_SurfaceUnitNorm_WS.Dot( pWallImpact->PushUnitVec );
						if ( fTemp < -0.0001f ) {

							// Force the push vector along the surface of the poly we're standing on.
							// An exactly downward/upward push has no tangent component, so this
							// projection is undefined (1 + fTemp == 0). This can occur for a bot
							// placed in an initial floor overlap; use the original finite push to
							// let the collision solver move it out instead of propagating NaNs.
							if( (1.0f + fTemp) > 0.0001f ) {
								vDiff.Mul( m_SurfaceUnitNorm_WS, -fTemp );
								vPushVec_WS.Add( pWallImpact->PushUnitVec, vDiff );
								vPushVec_WS.Mul( fmath_Div( pWallImpact->fImpactDistInfo, (1 + fTemp) ) + 0.001f );
							} else {
								vPushVec_WS.Mul( pWallImpact->PushUnitVec, pWallImpact->fImpactDistInfo + 0.001f );
							}
						} else {
							vPushVec_WS.Mul( pWallImpact->PushUnitVec, pWallImpact->fImpactDistInfo + 0.001f );
						}
					} else {
						vPushVec_WS.Mul( pWallImpact->PushUnitVec, pWallImpact->fImpactDistInfo + 0.001f );
					}

					m_CollSphere.m_Pos += vPushVec_WS.v3;
					CollData.pMovement->Add( vPushVec_WS );

					pImpact = NULL;
				}
			}

			if ( pImpact && pImpact->fUnitImpactTime == -1.f ) {
				// We are currently in a foot impact
				// The bot is initially in a collision state.

				if ( m_CollSphere.m_fRadius > pFootImpact->fImpactDistInfo ) {
					f32 fRadSq = (m_CollSphere.m_fRadius *  m_CollSphere.m_fRadius);
					FASSERT( fXZDistToFootImpactSq < fRadSq );
					if ( fXZDistToFootImpactSq > 0.0001f ) {
						f32 fA = fmath_Sqrt( fRadSq - fXZDistToFootImpactSq );
						fA = fA - (vSpherePosAtFootImpact.y - pFootImpact->ImpactPoint.y);

						vPushVec_WS.x = 0.f;
						vPushVec_WS.y = fA + 0.005f;
						vPushVec_WS.z = 0.f;
					} else {
						vPushVec_WS.Set( 0.f, pFootImpact->fImpactDistInfo + 0.005f, 0.f );
					}
					FMATH_CLAMPMIN( vPushVec_WS.y, 0.0f );
				} else {
					if ( pFootImpact->UnitFaceNormal.y > 0.707f ) { 
						vPushVec_WS.Set( 0.f, (pFootImpact->fImpactDistInfo / pFootImpact->UnitFaceNormal.y), 0.f );
						FMATH_CLAMPMIN( vPushVec_WS.y, 0.0f );
					} else {
						vPushVec_WS.Mul( pFootImpact->PushUnitVec, pFootImpact->fImpactDistInfo + 0.001f );
					}
				}

				CollData.pMovement->Add( vPushVec_WS );
				m_CollSphere.m_Pos += vPushVec_WS.v3;
			}
		} else {
			// The bot is not initially in a collision state but collides with something
			// along the way.  The collision system will provide the information we need to
			// determine where the bot would be if we slid it along the collided surface
			// (using the PushUnitVec and the fImpactDistInfo to counteract the pMove).  But
			// moving the bot to this position does not guarantee that we are not ending up
			// in a collision state or that we have not passed through something.  In order to
			// guarantee that it doesn't end up in collision and that it doesn't pass through something
			// we will move the bot as far as we can down the move vector by using the
			// fUnitImpactTime as the time to collision.  Once we have moved it as far as we can
			// down this vector, we can attempt to move, again, to the final position that the
			// collision system recommended by iterating again through the collision check.

			if ( pFootImpact && (!pWallImpact || pFootImpact->fUnitImpactTime <= pWallImpact->fUnitImpactTime) ) {
				// We hit the foot impact first
				pImpact = pFootImpact;
			} else {
				// We hit the wall impact first
				pImpact = pWallImpact;
			}

			// Determine the move vector up to the point of collision
			vPushVec_WS.Mul( *CollData.pMovement, pImpact->fUnitImpactTime * 0.9999f );

			if ( pImpact == pFootImpact ) {
				// Stick the bot to the ground
				vAdjust.x = 0.f;
				vAdjust.y = (pImpact->fImpactDistInfo / pImpact->UnitFaceNormal.y) + 0.025f;
				vAdjust.z = 0.f;
				CollData.pMovement->Add( *CollData.pMovement, vAdjust).v3 += m_CollSphere.m_Pos;
			} else {
				// Calculate the collision system recommended final position (after sliding along the collision)
				if ( bIsOnGround ) {

					// Is the push vector trying to push us into the ground?
					f32 fTemp = m_SurfaceUnitNorm_WS.Dot( pImpact->PushUnitVec );
					if ( fTemp < 0.0001f ) {

						// Force the push vector along the surface of the poly we're standing on
						vDiff.Mul( m_SurfaceUnitNorm_WS, -fTemp );
						vAdjust.Add( pImpact->PushUnitVec, vDiff );
						vAdjust.Mul( pImpact->fImpactDistInfo + 0.001f );
					} else {
						vAdjust.Mul( pImpact->PushUnitVec, pImpact->fImpactDistInfo + 0.001f );
					}
				} else {
					vAdjust.Mul( pImpact->PushUnitVec, pImpact->fImpactDistInfo + 0.001f );
				}

				CollData.pMovement->Add( vAdjust ).v3 += m_CollSphere.m_Pos;
			}

			// Move the object as far as we can down the original pMove without collision
			m_CollSphere.m_Pos += vPushVec_WS.v3;

			// Calculate move to recommended position from the object's new "safe" position
			CollData.pMovement->v3 -= m_CollSphere.m_Pos;
		}

		// Handle special collision functions for non-walkable surfaces
		if ( pWallImpact ) {
			FMATH_SETBITMASK( m_uCollisionStatus, COLLSTATE_WALL );

			// Check for our sneaky indicator that this is a bot collision (sphere instead of triangle)
			if ( pWallImpact->nSourceBoneIndex == 0xff ) {
				FASSERT( pWallImpact->pTag );
				FASSERT( ((CFWorldTracker *)pWallImpact->pTag)->m_pUser );
				FASSERT( ((CFWorldTracker *)pWallImpact->pTag)->m_nUser == MESHTYPES_ENTITY );
				FASSERT( ((CEntity *)((CFWorldTracker *)pWallImpact->pTag)->m_pUser)->TypeBits() & ENTITY_BIT_BOT );

				// This is a bot collision so we need to "bump" a little velocity into the bot we collided with
				CBot *pBot = (CBot *)((CFWorldTracker *)pWallImpact->pTag)->m_pUser;

				u8 nBot;
				for ( nBot = 0; i < nBotsCollidedCount; i++ ) {
					if( apBotsCollided[nBot] == pBot ) {
						break;
					}
				}

				if ( nBot == nBotsCollidedCount ) {

					// This is a bot to which we haven't already applied a force

					if ( nBotsCollidedCount < __MAX_BOTS_COLLIDED_WITH ) {
						// Record this bot so that we don't reapply an impulse in a later iteration
						apBotsCollided[nBotsCollidedCount++] = pBot;
					}

					f32 fRelMass, fPushMag;
					fRelMass = fmath_Div( m_pBotInfo_Gen->fBotCollisionMass, m_pBotInfo_Gen->fBotCollisionMass + pBot->m_pBotInfo_Gen->fBotCollisionMass );
					FMATH_CLAMP_UNIT_FLOAT( fRelMass );

					if( m_Velocity_WS.Dot( pWallImpact->PushUnitVec ) < 0.0f ) {
						fPushMag = m_Velocity_WS.Mag(); 
						vDiff.Mul( pWallImpact->PushUnitVec, -fPushMag * fRelMass );
						pBot->ApplyVelocityImpulse_WS ( vDiff );
					} else {
						fPushMag = 0.0f;
					}

					// Now change our velocity.  If the bots are headed the same direction, lower the velocity penalty
					if( m_Velocity_WS.Dot( pBot->m_Velocity_WS ) >= 0.0f ) {
						fPushMag *= 0.5f;
					}

					vDiff.Mul( pWallImpact->PushUnitVec, fPushMag * (1.0f - fRelMass) );

					if( vDiff.MagSq() < m_Velocity_WS.MagSq() ) {
						m_Velocity_WS.Add( vDiff );
						WS2MS( m_Velocity_MS, m_Velocity_WS );
					} else {
						m_Velocity_WS.Zero();
						m_Velocity_MS.Zero();
					}
					VelocityHasChanged();

					if( pBot->AIBrain() && m_nPossessionPlayerIndex > -1 ) {
						aibrainman_RammedByNotify(pBot->AIBrain(), this);
					}
				}
			} else {
				// Record this vector for adjusting "nudge"
				m_vWallImpactUnitVec = pWallImpact->PushUnitVec.v3;

				vDiff.Cross( pWallImpact->PushUnitVec, m_Velocity_WS );
				m_Velocity_WS.Cross( vDiff, pWallImpact->PushUnitVec );
				WS2MS( m_Velocity_MS, m_Velocity_WS );
				VelocityHasChanged();
			}
		}

		// Handle special collision functions for walkable surfaces
		if ( pFootImpact ) {

			FASSERT( pFootImpact->aTriVtx[0].x != FMATH_MAX_FLOAT );

			#if _SHOW_DEBUG_GRAPHICS
				if ( bShowDebugGraphics ) {
					fdraw_DevLine( &pFootImpact->aTriVtx[0].v3, &pFootImpact->aTriVtx[1].v3 );
					fdraw_DevLine( &pFootImpact->aTriVtx[1].v3, &pFootImpact->aTriVtx[2].v3 );
					fdraw_DevLine( &pFootImpact->aTriVtx[2].v3, &pFootImpact->aTriVtx[0].v3 );

					CFColorRGBA ImpactColor( 1.f, 0.f, 1.f, 1.f );
					fdraw_DevSphere( &pFootImpact->ImpactPoint.v3, 0.1f, &ImpactColor );
				}
			#endif

			CFWorldTracker *pHitTracker = (CFWorldTracker *)pFootImpact->pTag;

			if ( pHitTracker ) {
				// We hit a tracker...
				if ( pHitTracker->m_nUser == MESHTYPES_ENTITY ) {
					// We hit an entity...
					pStickyParent = (CEntity *)pHitTracker->m_pUser;
					if ( pStickyParent->TypeBits() & ENTITY_BIT_MESHENTITY ) {
						CMeshEntity *pME = (CMeshEntity *)(pStickyParent);
						if ( pFootImpact->nBoneIndex != 0xff ) {
							pszStickyParentBoneName = pME->GetMeshInst()->m_pMesh->pBoneArray[pFootImpact->nBoneIndex].szName;
						} else {
							pszStickyParentBoneName = NULL;
						}
					} else if (pStickyParent->TypeBits() & ENTITY_BIT_BOT) { // stick to the pillbox and ratgun
						CBot *pME = (CBot*)(pStickyParent);
						if ( pME->IsMeshCollideOnly() ) {
							if (pFootImpact->nBoneIndex != 0xff) {
								pszStickyParentBoneName = pME->m_pWorldMesh->m_pMesh->pBoneArray[pFootImpact->nBoneIndex].szName;
							} else {
								pszStickyParentBoneName = NULL;
							}
						}
					}
				}
			}

			m_FeetToGroundCollImpact = *pFootImpact;

			// Save the surface unit normal...
			m_SurfaceUnitNorm_WS.Set( m_FeetToGroundCollImpact.UnitFaceNormal );

			// Get material and surface type...
			m_pSurfaceMtl = CGColl::GetMaterial( &m_FeetToGroundCollImpact );
			m_nSurfaceType = CGColl::GetSurfaceType( &m_FeetToGroundCollImpact );

			// Get the surface type the bot is on
			nSurfaceType = m_FeetToGroundCollImpact.nUserType >> 3;

			FMATH_SETBITMASK( m_uCollisionStatus, COLLSTATE_FLOOR );
			m_nState = STATE_GROUND;

			m_CollSphere.m_Pos.y += 0.001f;
		}

		if ( CollData.pMovement->MagSq() < 0.00001f ) {
			CollData.pMovement = NULL;
		}
	}

	if ( nSurfaceType == 0xffffffff ) {
		m_MountPos_WS.y = m_CollSphere.m_Pos.y - fCollSphereY + 0.9f + fSnapToSurfaceDist;
	} else {
		m_MountPos_WS.y = m_CollSphere.m_Pos.y - fCollSphereY + 0.9f;
	}
	m_MountPos_WS.x = m_MountPrevPos_WS.x + m_CollSphere.m_Pos.x - vOriginalPos.x;
	m_MountPos_WS.z = m_MountPrevPos_WS.z + m_CollSphere.m_Pos.z - vOriginalPos.z;

	#if _SHOW_DEBUG_GRAPHICS
		if ( bShowDebugGraphics ) {
			f32 fX = 0.6f, fY = 0.07f;
			ftext_DebugPrintf( fX + 0.002f, fY + 0.002f, "~C00000099~al~w1Y VLCTY= %f", m_Velocity_WS.y );
			ftext_DebugPrintf( fX + 0.002f, fY + 0.022f, "~C00000099~al~w1TESTS  = %d (%d)", nTests, nTestsWithResults );
			ftext_DebugPrintf( fX + 0.002f, fY + 0.042f, "~C00000099~al~w1HEIGHT = %f", m_MountPos_WS.y );
			ftext_DebugPrintf( fX + 0.002f, fY + 0.062f, "~C00000099~al~w1RESULT = %f", m_CollSphere.m_Pos.y );
			ftext_DebugPrintf( fX, fY + 0.00f, "~C99999999~al~w1Y VLCTY= %f", m_Velocity_WS.y );
			ftext_DebugPrintf( fX, fY + 0.02f, "~C99999999~al~w1TESTS  = %d (%d)", nTests, nTestsWithResults );
			ftext_DebugPrintf( fX, fY + 0.04f, "~C99999999~al~w1HEIGHT = %f", m_MountPos_WS.y );
			ftext_DebugPrintf( fX, fY + 0.06f, "~C99999999~al~w1RESULT = %f", m_CollSphere.m_Pos.y );
			if ( bInitiallyInCollision )
			{
				ftext_DebugPrintf( fX + 0.002f, fY + 0.082f, "~C00000099~al~w1INITIALLY IN COLLISION" );
				ftext_DebugPrintf( fX, fY + 0.08f, "~C99999999~al~w1INITIALLY IN COLLISION" );
			}
		}
	#endif

#undef __GROUND_STICKY_HACKY
#undef __MAX_BOTS_COLLIDED_WITH

	// Set surface parameters, if we had collision with the floor
	if ( nSurfaceType != 0xffffffff )
	{
		FMATH_CLEARBIT( nSurfaceType, 7 );

		nSurfaceType -= 1;

		if( (nSurfaceType < SURFACE_TYPE_COUNT) && (nSurfaceType >= 0) ) {
			m_nSurfaceTypeOn = (SurfaceType_e)nSurfaceType;
		} else {
			// Always assume we are on a metal surface
			m_nSurfaceTypeOn = SURFACE_TYPE_METAL;
		}

		// Set traction...
		if( m_nSurfaceType == GCOLL_SURF_TYPE_SLIPPERY ) {
			m_fUnitTraction = 0.2f;
		} else {
			m_fUnitTraction = 1.0f;

			// NKM - Set the traction to a slippery surface
			if( m_pStickModifier_MS ) {
				CFVec3A UnitVelMS;

				if( UnitVelMS.Set( m_Velocity_MS ).MagSq() > 0.001f ) {
					UnitVelMS.Unitize();
				
					if( m_pStickModifier_MS->Dot( UnitVelMS ) < 0.0f ) {
						m_fUnitTraction = 0.2f;
					}
				}
			}
		}
	}
	// Handle changes in the entity that we are sticking to.
	// m_pStickyEntity should match this->GetParent(), but just in case
	// something strange is happening, I don't want to be 
	// responsible for detaching the wrong parent
	if( (pStickyParent != m_pStickyEntity) || (pszStickyParentBoneName != m_pszStickyEntityBoneName) ) {
		m_pszStickyEntityBoneName = pszStickyParentBoneName;

		if( m_pStickyEntity && (m_pStickyEntity == GetParent()) && !(m_nBotFlags & BOTFLAG_GLUED_TO_PARENT) ) {
			if( IsJumping() ) {
//				if( m_pStickyEntity->GetVerlet() == NULL ) {
					CFVec3A LastVel;
					LastVel.Sub(m_pStickyEntity->MtxToWorld()->m_vPos, m_LastStickyEntityPos);
					LastVel.Mul(FLoop_fPreviousLoopOOSecs);
					ApplyVelocityImpulse_WS(LastVel);
//				}
			}		

			// This nulls m_pStickyEntity...
			DetachFromParent();
		}

		if( pStickyParent && (!(m_nBotFlags&BOTFLAG_GLUED_TO_PARENT)) ){
			FASSERT( !GetParent() || (GetParent() != m_pStickyEntity) );
			m_pStickyEntity = pStickyParent;
			Attach_ToParent_WS(m_pStickyEntity, m_pszStickyEntityBoneName);
		}
	}
	if( m_pStickyEntity ) {
		// Force state+ground when we are standing on an entity (our sticky parent)...
		m_nState = STATE_GROUND;
		m_LastStickyEntityPos = m_pStickyEntity->MtxToWorld()->m_vPos;
	}
	if( m_nBotFlags&BOTFLAG_GLUED_TO_PARENT ) {
		// If we are Glitch and are panicking, this is multiplayer, and we want
		// to force the air animation. Otherwise,
		// force state+ground when we are glued on an entity (our glued-to parent)...
		m_nState = ((TypeBits() & ENTITY_BIT_BOTGLITCH) && (m_nBotFlags & BOTFLAG_FORCED_PANIC)) ? STATE_AIR : STATE_GROUND;
	}

	// Update immobile flag...
	if( m_nBotFlags & BOTFLAG_IMMOBILIZE_PENDING ) {
		if( (m_nState == STATE_GROUND) && (m_fSpeed_WS == 0.0f) ) {
			FMATH_CLEARBITMASK( m_nBotFlags, BOTFLAG_IMMOBILIZE_PENDING );
			FMATH_SETBITMASK( m_nBotFlags, BOTFLAG_IS_IMMOBILE );
		}
	}

	if( m_pStickyEntity && m_pStickyEntity->GetVerlet() ) {
		CFVec3A ForceVec_WS;

		ForceVec_WS.Set( 0.0f, -m_pBotInfo_Gen->fPhysForce_Stand, 0.0f );
		m_pStickyEntity->GetVerlet()->ApplyForce( &m_MtxToWorld.m_vPos, &ForceVec_WS );

		if( m_fXlatStickNormMag > 0.0f ) {
			ForceVec_WS.Mul( m_XlatStickUnitVecXZ_WS, -m_pBotInfo_Gen->fPhysForce_Lateral );
		} else {
			ForceVec_WS.Mul( m_UnitVelocityXZ_WS, m_pBotInfo_Gen->fPhysForce_Lateral );
		}

		m_pStickyEntity->GetVerlet()->ApplyForce( &m_MtxToWorld.m_vPos, &ForceVec_WS );
	}
}


u32 CBot::NewTrackerCollisionCallback( CFWorldTracker *pTracker ) {
	CFSphere HisCollSphere_WS;
	CBot *pBot;
	CFVec3A HisToMySphereCenter_WS;

	CFWorldMesh *pWorldMesh = (CFWorldMesh *)pTracker;

	if (!m_pCollBot)
	{
		return FCOLL_CHECK_CB_DO_NOT_CHECK_TRACKER;   //how, why, who!
	}

	if( !pWorldMesh->IsCollisionFlagSet() ) {
		// Not collidable...
		return FCOLL_CHECK_CB_DO_NOT_CHECK_TRACKER;
	}

	pBot = NULL;

	if( pTracker->m_nUser == MESHTYPES_ENTITY ) {


		// We shouldn't have any collisions with probes or swarmers
		//		FASSERT( !(((CEntity *)pTracker->m_pUser)->TypeBits() & ENTITY_BIT_BOTPROBE ) );
		FASSERT( !(((CEntity *)pTracker->m_pUser)->TypeBits() & ENTITY_BIT_BOTSWARMER ) );

		// Don't collide with weapons that are attached to bots...
		if( ((CEntity *)pTracker->m_pUser)->TypeBits() & ENTITY_BIT_WEAPON ) {
			if( ((CWeapon *)pTracker->m_pUser)->GetOwner() ) {
				return FCOLL_CHECK_CB_DO_NOT_CHECK_TRACKER;
			}
		}

		if (((CEntity *)pTracker->m_pUser)->IsActionable())
		{
			//bot has come close to an actionable entity, record that in case the bot cares to do something about it.
			m_pCollBot->m_pActionableEntityNearby = (CEntity *) pTracker->m_pUser;
		}

		if( ((CEntity *)pTracker->m_pUser)->TypeBits() & ENTITY_BIT_BOT ) {
			pBot = (CBot *)pTracker->m_pUser;
			FASSERT( m_pCollBot != NULL );
			FMATH_SETBITMASK( m_pCollBot->m_uCollisionStatus, COLLSTATE_BOT );
		} else {
			FASSERT( m_pCollBot != NULL );
//pgm says me told him to do this to fix jumptrooper's dive move on spacestation3			FMATH_SETBITMASK( m_pCollBot->m_uCollisionStatus, COLLSTATE_OBJECT );
		}

		if( ((CEntity *)pTracker->m_pUser)->TypeBits() & ENTITY_BIT_MESHENTITY ) {
			CMeshEntity *pMeshEntity = (CMeshEntity *)pTracker->m_pUser;

			if( pMeshEntity->IsVehicleCollOnly() && !(m_pCollBot->TypeBits() & ENTITY_BIT_VEHICLE) ) {
				// if m_pCollBot is not a vehicle, and mesh entity is marked
				// for vehicle-only collision, then don't collide.  -CJM
				return FCOLL_CHECK_CB_DO_NOT_CHECK_TRACKER;
			}
			// detpacks are actionable, not collidable. 
			if( ((CEntity *)pTracker->m_pUser)->TypeBits() & ENTITY_BIT_DETPACKDROP){
				return FCOLL_CHECK_CB_DO_NOT_CHECK_TRACKER;
			}
		}
	}

	if( !pBot ) {
		// We will only apply the adjustments, below, to bots
		return FCOLL_CHECK_CB_ALL_IMPACTS;
	}

	if( m_pCollBot && ((pBot->m_nBotFlags | m_pCollBot->m_nBotFlags) & BOTFLAG_SKIP_BOT_V_BOT_COLLISIONS)){   
		// One of the two bots in question has been flagged to ignore all bot collisions (pgm added this for bots that are mounting (eheem) other bots
		return FCOLL_CHECK_CB_DO_NOT_CHECK_TRACKER;
	}

	if (m_pCollBot->IsMeshCollideOnly()) {
		// if I'm so big I got glitch's walking on me, I don't want to get pushed around by walking on thems
		return FCOLL_CHECK_CB_DO_NOT_CHECK_TRACKER;
	}
	
	if( (pBot->TypeBits() & ENTITY_BIT_SITEWEAPON) ||
		(pBot->TypeBits() & ENTITY_BIT_BOTAAGUN ) ) {
		m_pCollBot->m_pActionableEntityNearby = pBot;
	}

	if( pBot->IsMeshCollideOnly() )	{
		if( m_pCollBot->m_nBotFlags & BOTFLAG_GLUED_TO_PARENT ) {
			// In this case, we probably don't want to collide...
			return FCOLL_CHECK_CB_DO_NOT_CHECK_TRACKER;
		} else {
			return FCOLL_CHECK_CB_ALL_IMPACTS;
		}
	}

	if( !pBot->m_pBotInfo_Gen ) {
		// This bot lacks a general info struct
		return FCOLL_CHECK_CB_ALL_IMPACTS;
	}

	//
	// We now know we are colliding with a bot that we want to use to adjust 
	// the movement of the moving bot.  So, let's determine that adjustment
	//

	// Establish the colliding bot's sphere
	HisCollSphere_WS.m_Pos.x = pBot->m_pBotInfo_Gen->fCollSphere1X_MS + pBot->m_MountPos_WS.x;
	HisCollSphere_WS.m_Pos.y = pBot->ComputeCollSphereY() + pBot->m_MountPos_WS.y;
	HisCollSphere_WS.m_Pos.z = pBot->m_pBotInfo_Gen->fCollSphere1Z_MS + pBot->m_MountPos_WS.z;
	HisCollSphere_WS.m_fRadius = pBot->m_pBotInfo_Gen->fCollSphere1Radius_MS;

	// ...to bring the colliding bot's sphere down to our level
	HisCollSphere_WS.m_Pos.y -= 0.9f;

	if( pBot && (pBot->TypeBits() & ENTITY_BIT_BOTPRED ) ) {
		// Other bots can stand on the pred.  Do triangle collision
		return FCOLL_CHECK_CB_ALL_IMPACTS;
	}

	if( (pBot->TypeBits() & ENTITY_BIT_VEHICLE ) ) {
		// Single sphere collision doesn't work well with vehicle shapes
		m_pCollBot->m_pActionableEntityNearby = pBot;

		return FCOLL_CHECK_CB_ALL_IMPACTS;
//		if( m_pCollBot->m_fCollCylinderHeight_WS < 6.0f )
//		{
//			// short bots can walk on the vehicle, but large bots do regular sphere-sphere collision
//			return TRUE;
//		}
	}

	//
	// Here (below) we're basically providing sphere v. projected sphere collision:
	//

	HisToMySphereCenter_WS.v3 = m_CollSphere.m_Pos - HisCollSphere_WS.m_Pos;
	
	f32 fDistToImpact;
	f32 fDistSq = HisToMySphereCenter_WS.MagSq();
	f32 fTotalRad = (m_CollSphere.m_fRadius + HisCollSphere_WS.m_fRadius);
	f32 fTotalRadSq = fTotalRad * fTotalRad;

	// If we have a collision, we want to drop a collision into the collision buffer to indicate 
	// to the requesting bot that he has impacted another bot:
	FCollImpact_t *pImpact = &FColl_aImpactBuf[FColl_nImpactCount];

	if ( fDistSq > fTotalRadSq ) {

		// Spheres are not initially in collision

		if ( !m_fMoveLength ) {
			// There is no movement, so there can be no collision
			return FCOLL_CHECK_CB_DO_NOT_CHECK_TRACKER;
		}

		f32 fDot = -HisToMySphereCenter_WS.Dot( m_vMoveNormal );
		if ( fDot <= 0.f || (fDot - HisCollSphere_WS.m_fRadius) > (m_fMoveLength + m_CollSphere.m_fRadius) ) {
			// The sphere is either behind or too far in front of the movement
			return FCOLL_CHECK_CB_DO_NOT_CHECK_TRACKER;
		}

		f32 fDistFromCenterlineSq = fDistSq - (fDot * fDot );
		if ( fDistFromCenterlineSq > fTotalRadSq ) {
			// Sphere is too far from centerline
			return FCOLL_CHECK_CB_DO_NOT_CHECK_TRACKER;
		}

		if ( fDistFromCenterlineSq > 0.001f ) {
			fDistToImpact = fDot - fmath_Sqrt( fTotalRadSq - fDistFromCenterlineSq );
		} else {
			fDistToImpact = fDot - fTotalRad;
		}

		// Reset the direction to sphere to the direction from the collision point
		CFVec3A vCollPos;
		vCollPos.Mul( m_vMoveNormal, m_fMoveLength - fDistToImpact ).v3 += m_CollSphere.m_Pos;
		HisToMySphereCenter_WS.v3 = HisCollSphere_WS.m_Pos - vCollPos.v3;
		HisToMySphereCenter_WS.Unitize();

		pImpact->fUnitImpactTime = fmath_Div( fDistToImpact, m_fMoveLength );
		pImpact->fImpactDistInfo = m_vMoveNormal.Dot( HisToMySphereCenter_WS ) * (1.f - pImpact->fUnitImpactTime );
		pImpact->ImpactPoint.v3 = vCollPos.v3 - (HisToMySphereCenter_WS.v3 * m_CollSphere.m_fRadius);

		// Sneaky (hacky) way of letting the HandleCollision() routine know that this is a bot collision
		pImpact->nSourceBoneIndex = 0xff;
	} else {
		// Spheres initially collide
		if ( fDistSq < 0.001f ) {
			return FCOLL_CHECK_CB_DO_NOT_CHECK_TRACKER;
		}

		fDistSq = fmath_Sqrt( fDistSq );

		fDistToImpact = (HisCollSphere_WS.m_fRadius + m_CollSphere.m_fRadius) - fDistSq;

		HisToMySphereCenter_WS.Unitize();

		pImpact->fUnitImpactTime = -1.f;
		pImpact->fImpactDistInfo = fDistToImpact;
		pImpact->ImpactPoint.v3 = m_CollSphere.m_Pos - (HisToMySphereCenter_WS.v3 * fDistToImpact);

		// Sneaky (hacky) way of letting the HandleCollision() routine know that this is a bot collision
		pImpact->nSourceBoneIndex = 0xff;
	}

	if( m_pCollBot->NotifyBotCollision( pBot ) ) {

		// This is a collision we want, so increment the impact count and fill in remaining data
		FColl_pNextImpactSlot++;
		FColl_nImpactCount++;
		pImpact->PushUnitVec.Set( HisToMySphereCenter_WS );
		pImpact->UnitFaceNormal.Set( HisToMySphereCenter_WS );
		pImpact->pTag = pBot->m_pWorldMesh;
	}

	return FCOLL_CHECK_CB_DO_NOT_CHECK_TRACKER;
}


// Returns the approximate instantaneous time until the bot will change from his current velocity to
// a stop if the control stick were released at this moment.
//
// IMPORTANT:
//   1) If the bot is in the air or if there is another reason that this value cannot
//      be computed, -1 is returned.
//   2) If the current velocity is very close to zero, 0 is returned.
f32 CBot::ComputeEstimatedControlledStopTimeXZ( void ) {

	CFVec3A CurrentToDesiredNormPlaneVelocityVec_WS;
	f32 fStopSeconds, fDot, fSlopeStepFactor, fNormVelocityStepSize;
	f32 fCurrentToDesiredNormPlaneVelocityVecMag, fCurrentToDesiredNormPlaneVelocityVecMag2;

	FASSERT( IsCreated() );

	fStopSeconds = -1.0f;

	if( m_nState == STATE_GROUND ) {
		if( m_nTractionMode != TRACTIONMODE_SLIDE ) {
			CurrentToDesiredNormPlaneVelocityVec_WS.ReceiveNegative( m_NormVelRotatedToXZ_WS );
			fCurrentToDesiredNormPlaneVelocityVecMag2 = CurrentToDesiredNormPlaneVelocityVec_WS.MagSq();

			if( fCurrentToDesiredNormPlaneVelocityVecMag2 > 0.00001f ) {
				fCurrentToDesiredNormPlaneVelocityVecMag = fmath_Sqrt( fCurrentToDesiredNormPlaneVelocityVecMag2 );
				CurrentToDesiredNormPlaneVelocityVec_WS.Div( fCurrentToDesiredNormPlaneVelocityVecMag );

				// Compute step size as though stick not pushed...
				if( m_fClampedNormSpeedXZ_WS > 0.0f ) {
					// Bot is moving...
					fDot = 0.5f * (m_UnitVelocityXZ_WS.Dot( m_SurfaceUnitNorm_WS ) + 1.0f);
					fSlopeStepFactor = FMATH_FPOT( fDot, m_pBotInfo_Walk->fStepSizeSlopeFactor, m_pBotInfo_Walk->fInvStepSizeSlopeFactor );
				} else {
					// Bot is stationary...
					fSlopeStepFactor = 1.0f;
				}

				fNormVelocityStepSize = fSlopeStepFactor * m_pBotInfo_Walk->fNormVelocityStepSize * FLoop_fPreviousLoopSecs;
				fNormVelocityStepSize *= m_fUnitTraction;
				FMATH_CLAMPMAX( fNormVelocityStepSize, fCurrentToDesiredNormPlaneVelocityVecMag );

				if( fNormVelocityStepSize >= 0.0001f ) {
					fStopSeconds = FLoop_fPreviousLoopSecs * fmath_Div( m_fNormSpeedAlongSurface_WS, fNormVelocityStepSize );
				} else {
					fStopSeconds = 0.0f;
				}
			} else {
				// The current and new velocities are close enough together to call the same...
				fStopSeconds = 0.0f;
			}
		}
	}

	return fStopSeconds;
}


// Computes the following:
//   m_nTractionMode
//   m_fSlipGravityStickBias
//   m_MovingSurfaceUnitVec_WS
//   m_fMaxFlatSurfaceSpeed_WS
//   m_NormVelRotatedToXZ_WS
//   m_fNormSpeedAlongSurface_WS
//   m_fClampedNormSpeedAlongSurface_WS
//   m_fSpeedAlongSurface_WS
//   m_fSecsSinceLastFootCollision
void CBot::HandleGroundTranslation( void ) {
	if( m_nBotFlags & BOTFLAG_GLUED_TO_PARENT ) {
		// No ground trans when glued to your parent...

		m_nTractionMode = TRACTIONMODE_NORMAL;
		m_fSlipGravityStickBias = 0.0f;
		m_fNormSpeedAlongSurface_WS = 0.0f;
		m_fClampedNormSpeedAlongSurface_WS = 0.0f;
		m_fSpeedAlongSurface_WS = 0.0f;

		// Forced panic together with glue means we are being carried, so
		// we don't want to zero out this value, because it allows us to
		// transition into the air animation
		if ( !((TypeBits() & ENTITY_BIT_BOTGLITCH) && (m_nBotFlags & BOTFLAG_FORCED_PANIC)) )
			m_fSecsSinceLastFootCollision = 0.0f;

		// Can't take a risk that this function moves us when we are glued to something.
		// Glued to something is much stronger than just having a sticky parent.

		return;
	}

	if( HasNoLegs() ) {
		m_Velocity_WS.Zero();
		m_Velocity_MS.Zero();
		VelocityHasChanged();
	}

	CFVec3A TempVec3A;
	f32 fDot, fTemp;

	FASSERT( IsCreated() );

	m_fMaxFlatSurfaceSpeed_WS = m_pBotInfo_Walk->fMaxXlatVelocity * m_fRunMultiplier;

	UpdateTractionMode();

	if( (m_nJumpState != BOTJUMPSTATE_LAUNCH) && (m_nPrevState==STATE_GROUND || m_nTractionMode==TRACTIONMODE_SLIDE) ) {
		// We were on the ground last frame, or we just landed and immediately began to slide.
		// Project velocity onto surface...

		TempVec3A.Cross( m_SurfaceUnitNorm_WS, m_Velocity_WS );
		m_Velocity_WS.Cross( TempVec3A, m_SurfaceUnitNorm_WS );

		// Recompute velocities...
		WS2MS( m_Velocity_MS, m_Velocity_WS );
		VelocityHasChanged();
	}

	if( m_nTractionMode == TRACTIONMODE_NORMAL ) {
		// Compute m_MovingSurfaceUnitVec_WS...
		if( ComputeMovingSurfaceVector() ) {
			// m_MovingSurfaceUnitVec_WS was computed without issue...

			if( m_nPrevState == STATE_AIR ) {
				// We were in the air last frame, and we just hit the ground this frame...

				fDot = m_MovingSurfaceUnitVec_WS.Dot( m_Velocity_WS );

				if( m_MovingSurfaceUnitVec_WS.y>=0.0f && m_fNormSpeedXZ_WS<1.1f ) {
					// We hit the surface going uphill at a typical speed.
					// We want to preserve the bot's forward motion, so
					// simply project the XZ velocity onto the surface plane...

					fTemp = m_MovingSurfaceUnitVec_WS.Dot( m_VelocityXZ_WS );
					fDot = FMATH_FPOT( m_fUnitTraction, fDot, fTemp );
				}

				// Compute our new velocity...
				m_Velocity_WS.Mul( m_MovingSurfaceUnitVec_WS, fDot );

				// Update other velocity components...
				WS2MS( m_Velocity_MS, m_Velocity_WS );
				VelocityHasChanged();
			}
		} else {
			// Velocity vector and surface normal are both vertical, or the velocity is zero.
			// m_MovingSurfaceUnitVec_WS is NULL.

			if( m_nPrevState == STATE_AIR ) {
				// We were in the air on the previous frame, and we just hit the ground this frame.

				// Since the velocity vector and surface normal are both vertical,
				// just zero the velocity vector...
				ZeroVelocity();
			}
		}
	} else if( m_nPrevState == STATE_AIR ) {
		m_nTractionMode = TRACTIONMODE_SLIDE;
	}

	// Update m_NormVelRotatedToXZ_WS from m_Velocity_MS...
	if( m_fSpeedXZ_WS > 0.0f ) {
		// World space velocity has a non-zero XZ component.
		// Scale the XZ unit velocity vector by the world space speed and normalize it...

		m_NormVelRotatedToXZ_WS.Mul( m_UnitVelocityXZ_WS, m_fSpeed_WS * m_pBotInfo_Walk->fInvMaxXlatVelocity * m_fOORunMultiplier );
	} else {
		// Model space XZ velocity is zero, so our normalized rotated velocity is zero too...
		m_NormVelRotatedToXZ_WS.Zero();
	}

	if( m_nJumpState != BOTJUMPSTATE_LAUNCH ) {
		ComputeNewGroundVelocity();
	}

	// Update m_fNormSpeedAlongSurface_WS, m_fClampedNormSpeedAlongSurface_WS and m_fSpeedAlongSurface_WS...
	m_fNormSpeedAlongSurface_WS = m_NormVelRotatedToXZ_WS.MagXZ();
	m_fClampedNormSpeedAlongSurface_WS = FMATH_MIN( m_fNormSpeedAlongSurface_WS, 1.0f );
	m_fSpeedAlongSurface_WS = m_fNormSpeedAlongSurface_WS * m_fMaxFlatSurfaceSpeed_WS;
	if( m_fSpeedAlongSurface_WS < 0.01f ) {
		m_fSpeedAlongSurface_WS = 0.0f;
	}


	m_fSecsSinceLastFootCollision = 0.0f;

	if( m_fSpeed_WS == 0.0f ) {
		m_fStepVolumeScaleDir = 1.0f;
	} else {
		m_fStepVolumeScaleDir = -1.0f;
	}
}


// returning FALSE aborts the collision test.
BOOL CBot::TrackerCollisionCallback( CFWorldTracker *pTracker, FVisVolume_t *pVolume ) {

	CFSphere HisCollSphere_WS;
	CBot *pBot;
	CFVec3A HisToMySphereCenter_WS;
	CFVec3A vPushVel;
	u32 i;
	f32 fDistFromHisSphereToMine_WS, fIntersectDist;

	CFWorldMesh *pWorldMesh = (CFWorldMesh *)pTracker;

	if (!m_pCollBot)
	{
		return FALSE;   //how, why, who!
	}
	if( !pWorldMesh->IsCollisionFlagSet() ) {
		// Not collidable...
		return TRUE;
	}

	for( i=0; i<FWorld_nTrackerSkipListCount; i++ ) {
		if( pTracker == FWorld_apTrackerSkipList[i] ) {
			return TRUE;
		}
	}

	pBot = NULL;

	if( pTracker->m_nUser == MESHTYPES_ENTITY ) {

		if (((CEntity *)pTracker->m_pUser)->IsActionable())
		{
			//bot has come close to an actionable entity, record that in case the bot cares to do something about it.
			m_pCollBot->m_pActionableEntityNearby = (CEntity *) pTracker->m_pUser;
		}

		// NKM - If we collide with a Probe bot or a swarmer, don't change anything
		if( /*( ((CEntity *)pTracker->m_pUser)->TypeBits() & ENTITY_BIT_BOTPROBE ) || */
			( ((CEntity *)pTracker->m_pUser)->TypeBits() & ENTITY_BIT_BOTSWARMER ) ) {
			return TRUE;
		}

		// Don't collide with weapons that are attached to bots...
		if( ((CEntity *)pTracker->m_pUser)->TypeBits() & ENTITY_BIT_WEAPON ) {
			if( ((CWeapon *)pTracker->m_pUser)->GetOwner() ) {
				return TRUE;
			}
		}

		if( ((CEntity *)pTracker->m_pUser)->TypeBits() & ENTITY_BIT_BOT ) {
			pBot = (CBot *)pTracker->m_pUser;
			FASSERT( m_pCollBot != NULL );
			FMATH_SETBITMASK( m_pCollBot->m_uCollisionStatus, COLLSTATE_BOT );
		} else {
			FASSERT( m_pCollBot != NULL );
//pgm says me told him to do this to fix jumptrooper's dive move on spacestation3			FMATH_SETBITMASK( m_pCollBot->m_uCollisionStatus, COLLSTATE_OBJECT );
		}

		if( ((CEntity *)pTracker->m_pUser)->TypeBits() & ENTITY_BIT_MESHENTITY ) {
			CMeshEntity *pMeshEntity = (CMeshEntity *)pTracker->m_pUser;

			if( pMeshEntity->IsVehicleCollOnly() && !(m_pCollBot->TypeBits() & ENTITY_BIT_VEHICLE) ) {
				// if m_pCollBot is not a vehicle, and mesh entity is marked
				// for vehicle-only collision, then don't collide.  -CJM
				return TRUE;
			}
		}
	}
	

	if (pBot &&
		((pBot->m_nBotFlags | m_pCollBot->m_nBotFlags) & BOTFLAG_SKIP_BOT_V_BOT_COLLISIONS))
	{   //one of the two bots in question has been flagged to ignore all bot collisions (pgm added this for bots that are mounting (eheem) other bots
		return TRUE;
	}

	if (m_pCollBot->IsMeshCollideOnly())
	{
		// if I'm so big I got glitch's walking on me, I don't want to get pushed around by walking on thems
		return TRUE;
	}
	
	if( pBot && (pBot->TypeBits() & ENTITY_BIT_SITEWEAPON ) ) {
		m_pCollBot->m_pActionableEntityNearby = pBot;
	}

	if( pBot && (pBot->TypeBits() & ENTITY_BIT_BOTAAGUN ) ) {
		m_pCollBot->m_pActionableEntityNearby = pBot;
	}

	if( pBot && pBot->IsMeshCollideOnly() )
	{
		if( m_pCollBot->m_nBotFlags & BOTFLAG_GLUED_TO_PARENT ) {
			// In this case, we probably don't want to collide...

			return TRUE;
		} else {
			m_CollInfo.pTag = pTracker;	  //any impacts that are generated will be marked with this pTag
			pWorldMesh->CollideWithMeshTris( &m_CollInfo );
			return TRUE;
		}
	}

	if( pBot && pBot->m_pBotInfo_Gen ) {

		// Bot...
		HisCollSphere_WS.m_Pos.x = pBot->m_pBotInfo_Gen->fCollSphere1X_MS + pBot->m_MountPos_WS.x;
		HisCollSphere_WS.m_Pos.y = pBot->ComputeCollSphereY() + pBot->m_MountPos_WS.y;
		HisCollSphere_WS.m_Pos.z = pBot->m_pBotInfo_Gen->fCollSphere1Z_MS + pBot->m_MountPos_WS.z;
		HisCollSphere_WS.m_fRadius = pBot->m_pBotInfo_Gen->fCollSphere1Radius_MS;

		HisCollSphere_WS.m_Pos.y -= 1.0f;

		if( !m_CollSphere.IsIntersecting( HisCollSphere_WS ) ) {
			// No collision...
			return TRUE;
		}

		if( pBot && (pBot->TypeBits() & ENTITY_BIT_VEHICLE ) ) {
			m_pCollBot->m_pActionableEntityNearby = (CVehicle*) pBot;

			// single sphere collision doesn't work well with vehicle shapes
			m_CollInfo.pTag = pTracker;	  //any impacts that are generated will be marked with this pTag
			pWorldMesh->CollideWithMeshTris( &m_CollInfo );
			return TRUE;
		}

		//// other bots can stand on the pred.  Do triangle collision
		//if( pBot && (pBot->TypeBits() & ENTITY_BIT_BOTPRED ) ) {
		//	m_CollInfo.pTag = pTracker;	  //any impacts that are generated will be marked with this pTag
		//	pWorldMesh->CollideWithMeshTris( &m_CollInfo );
		//	return TRUE;
		//}

		HisToMySphereCenter_WS.v3 = m_CollSphere.m_Pos - HisCollSphere_WS.m_Pos;
		fDistFromHisSphereToMine_WS = HisToMySphereCenter_WS.Mag();
		if( fDistFromHisSphereToMine_WS == 0.0f ) {
			return TRUE;
		}

		fIntersectDist = (HisCollSphere_WS.m_fRadius + m_CollSphere.m_fRadius) - fDistFromHisSphereToMine_WS;

		if( fIntersectDist < 0.0f ) {
			return TRUE;
		}

		HisToMySphereCenter_WS.Unitize();
		vPushVel = HisToMySphereCenter_WS;
		HisToMySphereCenter_WS.Mul( fIntersectDist );

		if( m_pCollBot->NotifyBotCollision( pBot ) ) {

			f32 fRelMass, fPushMag;
			fRelMass = fmath_Div( m_pCollBot->m_pBotInfo_Gen->fBotCollisionMass, m_pCollBot->m_pBotInfo_Gen->fBotCollisionMass + pBot->m_pBotInfo_Gen->fBotCollisionMass );
			FMATH_CLAMP_UNIT_FLOAT( fRelMass );

			if( m_pCollBot->m_Velocity_WS.Dot( HisToMySphereCenter_WS ) < 0.0f ) {
				fPushMag = m_pCollBot->m_Velocity_WS.Mag(); 
				vPushVel.Mul( -fPushMag * fRelMass );
				pBot->ApplyVelocityImpulse_WS ( vPushVel );
			} else {
				fPushMag = 0.0f;
			}

			// Adjust for collision
			m_pCollBot->m_MountPos_WS.Add( HisToMySphereCenter_WS );


			////Now change our velocity.  If the bots are headed the same direction, lower the velocity penalty
			if( m_pCollBot->m_Velocity_WS.Dot( pBot->m_Velocity_WS ) >= 0.0f ) {
				fPushMag *= 0.5f;
			}

			HisToMySphereCenter_WS.Unitize().Mul( fPushMag * (1.0f - fRelMass) );

			if( HisToMySphereCenter_WS.MagSq() < m_pCollBot->m_Velocity_WS.MagSq() ) {
				m_pCollBot->m_Velocity_WS.Add( HisToMySphereCenter_WS );
				m_pCollBot->WS2MS( m_pCollBot->m_Velocity_MS, m_pCollBot->m_Velocity_WS );
			} else {
				m_pCollBot->m_Velocity_WS.Zero();
				m_pCollBot->m_Velocity_MS.Zero();
			}
			m_pCollBot->VelocityHasChanged();
		}

		if( pBot->AIBrain() && m_pCollBot->m_nPossessionPlayerIndex > -1 ) {
			aibrainman_RammedByNotify(pBot->AIBrain(), m_pCollBot);
		}
	} else {
		// Other type of object...
		m_CollInfo.pTag = pTracker;	  //any impacts that are generated will be marked with this pTag
		pWorldMesh->CollideWithMeshTris( &m_CollInfo );
		return TRUE;
	}

	return TRUE;
}


void CBot::HandleAirTranslation( void ) {
	m_fSecsSinceLastFootCollision += FLoop_fPreviousLoopSecs;

	if( m_nBotFlags & BOTFLAG_GLUED_TO_PARENT ) {
		// No air trans when manning the pillbox.
		// Glued to parent means that we can not ever be moved.
		return;
	}

	// So we can freeze a bot in the air when grabbed by the Probe
	if( m_nBotFlags & BOTFLAG_NO_GRAVITY ) {
		return;
	}

	f32 fNudgeSpeedX, fNudgeSpeedZ;

	// If we're moving in the air, m_MovingSurfaceUnitVec_WS is simply our unit velocity vector.
	// Otherwise, we'll just set it to m_MountUnitFrontXZ_WS...
	m_MovingSurfaceUnitVec_WS = (m_fSpeed_WS != 0.0f) ? m_UnitVelocity_WS : m_MountUnitFrontXZ_WS;

	// No surface normal...
	m_SurfaceUnitNorm_WS.Zero();

	// Since we're in the air, reset walk mode...
	m_nTractionMode = TRACTIONMODE_NORMAL;

	if( m_nJumpState != BOTJUMPSTATE_CABLE ) {
		// We're in the air. Apply gravity to both model and world space velocities...
		m_Velocity_MS.y += m_pBotInfo_Gen->fGravity * m_fGravityMultiplier * FLoop_fPreviousLoopSecs;
		m_Velocity_WS.y = m_Velocity_MS.y;

		// Apply air translation control...
		if( m_fXlatStickNormMag > 0.0f && (m_fSpeedXZ_WS < m_pBotInfo_Walk->fAirControlSpeedThreshold) ) {
			// Stick is being applied...

			fNudgeSpeedX = m_pBotInfo_Walk->fAirControlNudgeSpeed * m_XlatStickUnitVecXZ_WS.x;
			fNudgeSpeedZ = m_pBotInfo_Walk->fAirControlNudgeSpeed * m_XlatStickUnitVecXZ_WS.z;

			// If the bot is colliding with a wall, kill any nudge in the direction of the wall collision
			if ( m_pBotInfo_Walk->fAirControlNudgeSpeed != 0.f && (m_uCollisionStatus & COLLSTATE_WALL) )
			{
				// First we need to unitize the wall impact vector in XZ
				f32 fTemp = (m_vWallImpactUnitVec.x * m_vWallImpactUnitVec.x) + (m_vWallImpactUnitVec.z * m_vWallImpactUnitVec.z);
				if ( fTemp > 0.0001f )
				{
					fTemp = fmath_InvSqrt( fTemp );
					f32 fWallImpactX = m_vWallImpactUnitVec.x * fTemp;
					f32 fWallImpactZ = m_vWallImpactUnitVec.z * fTemp;

					fTemp = (fWallImpactX * fNudgeSpeedX) + (fWallImpactZ * fNudgeSpeedZ);
					if ( fTemp < 0.f )
					{
						// We're nudging towards the wall, so kill the portion of the nudge that is in that direction:
						// We kill only 90% of the nudge because we want to make sure we don't fluctuate between wall
						// collision and no wall collision by killing all nudge.
						fNudgeSpeedX -= fTemp * fWallImpactX * 0.9f;
						fNudgeSpeedZ -= fTemp * fWallImpactZ * 0.9f;
					}
				}
			}

			if( m_nPossessionPlayerIndex < 0 ) {
				// NPC...

				if( fNudgeSpeedX < 0.0f ) {
					if( m_Velocity_WS.x>fNudgeSpeedX && m_Velocity_WS.x<-fNudgeSpeedX ) {
						m_Velocity_WS.x += fNudgeSpeedX;
						FMATH_CLAMPMIN( m_Velocity_WS.x, fNudgeSpeedX );
					}
				} else if( fNudgeSpeedX > 0.0f ) {
					if( m_Velocity_WS.x<fNudgeSpeedX && m_Velocity_WS.x>-fNudgeSpeedX ) {
						m_Velocity_WS.x += fNudgeSpeedX;
						FMATH_CLAMPMAX( m_Velocity_WS.x, fNudgeSpeedX );
					}
				}

				if( fNudgeSpeedZ < 0.0f ) {
					if( m_Velocity_WS.z>fNudgeSpeedZ && m_Velocity_WS.z<-fNudgeSpeedZ ) {
						m_Velocity_WS.z += fNudgeSpeedZ;
						FMATH_CLAMPMIN( m_Velocity_WS.z, fNudgeSpeedZ );
					}
				} else if( fNudgeSpeedZ > 0.0f ) {
					if( m_Velocity_WS.z<fNudgeSpeedZ && m_Velocity_WS.z>-fNudgeSpeedZ ) {
						m_Velocity_WS.z += fNudgeSpeedZ;
						FMATH_CLAMPMAX( m_Velocity_WS.z, fNudgeSpeedZ );
					}
				}
			} else {
				// Human...

				if( fNudgeSpeedX < 0.0f ) {
					if( m_Velocity_WS.x > fNudgeSpeedX ) {
						m_Velocity_WS.x += fNudgeSpeedX;
						FMATH_CLAMPMIN( m_Velocity_WS.x, fNudgeSpeedX );
					}
				} else if( fNudgeSpeedX > 0.0f ) {
					if( m_Velocity_WS.x < fNudgeSpeedX ) {
						m_Velocity_WS.x += fNudgeSpeedX;
						FMATH_CLAMPMAX( m_Velocity_WS.x, fNudgeSpeedX );
					}
				}

				if( fNudgeSpeedZ < 0.0f ) {
					if( m_Velocity_WS.z > fNudgeSpeedZ ) {
						m_Velocity_WS.z += fNudgeSpeedZ;
						FMATH_CLAMPMIN( m_Velocity_WS.z, fNudgeSpeedZ );
					}
				} else if( fNudgeSpeedZ > 0.0f ) {
					if( m_Velocity_WS.z < fNudgeSpeedZ ) {
						m_Velocity_WS.z += fNudgeSpeedZ;
						FMATH_CLAMPMAX( m_Velocity_WS.z, fNudgeSpeedZ );
					}
				}
			}
		}

		// Update velocities...
		WS2MS( m_Velocity_MS, m_Velocity_WS );
		VelocityHasChanged();
	}
}


// Returns TRUE if the bot should enter its fly mode.
BOOL CBot::HandleAirAnimations( void ) {
	if( m_nJumpState == BOTJUMPSTATE_NONE ) {
		// We're in the air, but not jumping...
		if( m_nPrevState == STATE_GROUND ) {
			m_fUnitFlyBlend = 0.0f;
		}

		DeltaTime( ASI_FALL );

		if( m_fSecsSinceLastFootCollision < 1.0f ) {
			HandleGroundAnimations();
		}

		if( m_fSecsSinceLastFootCollision>0.3f && m_fUnitFlyBlend<1.0f ) {
			m_fUnitFlyBlend += m_pBotInfo_Jump->fGroundToAirBlendSpeed * FLoop_fPreviousLoopSecs;

			if( m_fUnitFlyBlend < 1.0f ) {
				SetControlValue( ASI_FALL, m_fUnitFlyBlend );
			} else {
				m_fUnitFlyBlend = 1.0f;
				return TRUE;
			}
		}
	}

	return FALSE;
}


// Computes the following:
//   m_nTractionMode
//   m_fSlipGravityStickBias
void CBot::UpdateTractionMode( void ) {
	CFVec3A TempVec3A;
	f32 fTemp;

	switch( m_nTractionMode ) {
	case TRACTIONMODE_NORMAL:
		if( m_fNormSpeedAlongSurface_WS >= m_pBotInfo_Walk->fFasterThanUsualNormSpeed ) {
			// We're moving faster than usual...
			m_nTractionMode = TRACTIONMODE_SLIDE;
		} else {
			if( m_fBestCaseSurfaceUnitNormY_WS < m_pBotInfo_Walk->fSteepSlopeUnitNormY ) {
				// We're on a steep slope...

				if( m_Velocity_WS.y > 0.0f ) {
					// We're moving uphill...

					if( m_fUnitTraction == 1.0f ) {
						// Normal traction...

						if( m_fNormSpeedAlongSurface_WS < m_pBotInfo_Walk->fWithinUsualNormSpeed ) {
							// We're moving within normal speed...
							m_nTractionMode = TRACTIONMODE_SLIP;
							m_fSlipGravityStickBias = 0.0f;
						}
					} else {
						// Low traction...
						m_nTractionMode = TRACTIONMODE_LOW_TRACTION;
					}
				} else {
					// We're moving downhill...
					m_nTractionMode = TRACTIONMODE_SLIDE;
				}
			} else {
				if( m_fUnitTraction < 1.0f ) {
					// Low traction...
					m_nTractionMode = TRACTIONMODE_LOW_TRACTION;
				}
			}
		}

		break;

	case TRACTIONMODE_LOW_TRACTION:
		if( (m_fNormSpeedAlongSurface_WS >= m_pBotInfo_Walk->fFasterThanUsualNormSpeed) || (m_fBestCaseSurfaceUnitNormY_WS<m_pBotInfo_Walk->fSteepSlopeUnitNormY && m_Velocity_WS.y<=0.0f) ) {
			m_nTractionMode = TRACTIONMODE_SLIDE;
		} else if( m_fUnitTraction == 1.0f ) {
			m_nTractionMode = TRACTIONMODE_NORMAL;
		} else {
			fTemp = m_pBotInfo_Gen->fGravity * m_fGravityMultiplier * FLoop_fPreviousLoopSecs;
			m_Velocity_WS.y += fTemp;
			m_Velocity_MS.y += fTemp;
		}

		break;

	case TRACTIONMODE_SLIP:
		if( m_fBestCaseSurfaceUnitNormY_WS >= m_pBotInfo_Walk->fSteepSlopeUnitNormY ) {
			// Steepness is no longer great...
			m_nTractionMode = TRACTIONMODE_NORMAL;
		} else if( m_Velocity_WS.y<=0.0f || m_fNormSpeedAlongSurface_WS>=m_pBotInfo_Walk->fWithinUsualNormSpeed ) {
			// Switch to slide mode...
			m_nTractionMode = TRACTIONMODE_SLIDE;
		} else {
			// Still in slip mode...
			m_fSlipGravityStickBias += m_pBotInfo_Walk->fSlipStickBiasSpeed * (1.1f - m_fBestCaseSurfaceUnitNormY_WS) * FLoop_fPreviousLoopSecs;
			FMATH_CLAMPMAX( m_fSlipGravityStickBias, m_pBotInfo_Walk->fMaxSlipStickBias );
		}

		break;

	case TRACTIONMODE_SLIDE:
		if( m_fNormSpeedAlongSurface_WS<1.0f && (m_fBestCaseSurfaceUnitNormY_WS>m_pBotInfo_Walk->fSteepSlopeUnitNormY || m_Velocity_WS.y>0.0f) ) {
			// Time to leave slide mode...
			m_nTractionMode = TRACTIONMODE_NORMAL;
		} else {
			if( m_fUnitTraction == 1.0f ) {
				fTemp = 60.0f * FLoop_fPreviousLoopSecs;
				if( fTemp < m_fSpeed_WS ) {
					TempVec3A.Mul( m_UnitVelocity_WS, fTemp );
					m_Velocity_WS.Sub( TempVec3A );
					WS2MS( m_Velocity_MS, m_Velocity_WS );
				} else {
					m_Velocity_WS.Zero();
					m_Velocity_MS.Zero();
				}

				m_Velocity_MS.y += m_pBotInfo_Gen->fGravity * m_fGravityMultiplier * m_pBotInfo_Walk->fSlideGravityMult * FLoop_fPreviousLoopSecs;
				m_Velocity_WS.y = m_Velocity_MS.y;

				if( (FVid_nFrameCounter & 1) == 0 ) {
					TempVec3A = m_MountPos_WS;
					TempVec3A.x += fmath_RandomFloatRange( -0.5f, 0.5f );
					TempVec3A.y += fmath_RandomFloatRange(  0.5f, 1.0f );
					TempVec3A.z += fmath_RandomFloatRange( -0.5f, 0.5f );

//					fpart_SpawnParticleEmitter( FPART_TYPE_DUST_PUFF, 0.3f, TempVec3A, &CFVec3A::m_UnitAxisY, FALSE );
				}
			} else {
				m_Velocity_MS.y += m_pBotInfo_Gen->fGravity * m_fGravityMultiplier * FLoop_fPreviousLoopSecs;
				m_Velocity_WS.y = m_Velocity_MS.y;
			}
		}

		break;
	}
}


void CBot::ComputeNewGroundVelocity( void ) {
	CFVec3A TempVec3A, TempVec3B, StickNormVelocityVecXZ_WS, CurrentToDesiredNormPlaneVelocityVec_WS;
	CFQuatA TempQuat;
	f32 fDot, fUnit, fTemp, fSlopeStepFactor, fNormVelocityStepSize, fMaxRunNormVelocity;
	f32 fCurrentToDesiredNormPlaneVelocityVecMag, fCurrentToDesiredNormPlaneVelocityVecMag2;

	m_fMaxSneakStickMag = m_pBotInfo_Walk->fMaxSneakStickMag;

	if( m_nTractionMode != TRACTIONMODE_SLIDE ) {
		if( (m_anAnimStackIndex[ASI_SNEAK] == -1) || IsAlertOn() || ((m_nControlsBot_Flags & (CBotControl::FLAG_NO_AUTO_SNEAK | CBotControl::FLAG_FORCE_SNEAK)) == CBotControl::FLAG_NO_AUTO_SNEAK) ) {
			// Sneak not allowed...
			m_fMaxSneakStickMag = 0.0f;
		}

		// From the control stick vector, compute the corresponding desired unit velocity in model space...
		if( m_fXlatStickNormMag == 0.0f ) {
			// Stick is not being pushed...
			StickNormVelocityVecXZ_WS.Zero();
		} else if( m_fXlatStickNormMag < m_fMaxSneakStickMag/* && m_fRunMultiplier == 1.0f*/ ) {
			// Stick is pushed, but is within the sneak region...
			StickNormVelocityVecXZ_WS.Mul( m_XlatStickUnitVecXZ_WS, m_pBotInfo_Walk->fSneakNormVelocity );
		} else {
			// Stick is pushed beyond the sneak region...

			// Determine the steepness of the plane in the direction the stick is being pushed...
			fDot = m_XlatStickUnitVecXZ_WS.Dot( m_SurfaceUnitNorm_WS );

			if( fDot >= 0.0f ) {
				fDot *= fDot;
			} else {
				fDot *= -fDot;
			}

			// Compute the maximum allowed stick magnitude, based on the steepness...
//			fMaxRunNormVelocity = 1.0f + fDot * m_pBotInfo_Walk->fTopSpeedMultiplierForSlope;
			fMaxRunNormVelocity = GetLimitedTopSpeed() + fDot * m_pBotInfo_Walk->fTopSpeedMultiplierForSlope;

			// Make sure we're never walking slower than the minimum walk speed...
			FMATH_CLAMPMIN( fMaxRunNormVelocity, m_pBotInfo_Walk->fMinWalkNormVelocity );

			// Scale the stick magnitude so that 0 maps to the min walk speed, and 1 maps to the max run speed...
			fUnit = fmath_Div( m_fXlatStickNormMag - m_fMaxSneakStickMag, 1.0f - m_fMaxSneakStickMag );

			// Compute our normalized stick plane velocity...
			StickNormVelocityVecXZ_WS.Mul( m_XlatStickUnitVecXZ_WS, FMATH_FPOT( fUnit, m_pBotInfo_Walk->fMinWalkNormVelocity, fMaxRunNormVelocity ) );
		}

		if( m_nTractionMode == TRACTIONMODE_SLIP ) {
			// Compute downhill vector lying on plane...
			TempVec3B.CrossYWithVec( m_SurfaceUnitNorm_WS );
			TempVec3A.Cross( TempVec3B, m_SurfaceUnitNorm_WS );

			// Bring this into plane model space...
			TempVec3A.y = 0.0f;
			if( TempVec3B.SafeUnitAndMagXZ( TempVec3A ) != -1.0f ) {
				TempVec3B.Mul( m_fSlipGravityStickBias );
				StickNormVelocityVecXZ_WS.Add( TempVec3B );
			}
		}

		// Compute new model space normalized velocity by taking a step from our current
		// normalized velocity to StickNormVelocityVecXZ_WS...
		CurrentToDesiredNormPlaneVelocityVec_WS.Sub( StickNormVelocityVecXZ_WS, m_NormVelRotatedToXZ_WS );
		fCurrentToDesiredNormPlaneVelocityVecMag2 = CurrentToDesiredNormPlaneVelocityVec_WS.MagSq();

		if( fCurrentToDesiredNormPlaneVelocityVecMag2 > 0.00001f ) {
			fCurrentToDesiredNormPlaneVelocityVecMag = fmath_Sqrt( fCurrentToDesiredNormPlaneVelocityVecMag2 );
			CurrentToDesiredNormPlaneVelocityVec_WS.Div( fCurrentToDesiredNormPlaneVelocityVecMag );

			// Compute step size...
			if( m_fXlatStickNormMag == 0.0f ) {
				// Stick not pushed...

				if( m_fClampedNormSpeedXZ_WS > 0.0f ) {
					// Bot is moving...
					fDot = 0.5f * (m_UnitVelocityXZ_WS.Dot( m_SurfaceUnitNorm_WS ) + 1.0f);
					fSlopeStepFactor = FMATH_FPOT( fDot, m_pBotInfo_Walk->fStepSizeSlopeFactor, m_pBotInfo_Walk->fInvStepSizeSlopeFactor );
				} else {
					// Bot is stationary...
					fSlopeStepFactor = 1.0f;
				}
			} else {
				// Stick pushed...
				fDot = m_XlatStickUnitVecXZ_WS.Dot( m_SurfaceUnitNorm_WS );
				FMATH_CLAMPMIN( fDot, 0.0f );
				fSlopeStepFactor = FMATH_FPOT( fDot, 1.0f, m_pBotInfo_Walk->fStepSizeSlopeFactor );
			}

			fNormVelocityStepSize = fSlopeStepFactor * m_pBotInfo_Walk->fNormVelocityStepSize * FLoop_fPreviousLoopSecs;
			fNormVelocityStepSize *= m_fUnitTraction;
			FMATH_CLAMPMAX( fNormVelocityStepSize, fCurrentToDesiredNormPlaneVelocityVecMag );
			CurrentToDesiredNormPlaneVelocityVec_WS.Mul( fNormVelocityStepSize );
			m_NormVelRotatedToXZ_WS.Add( CurrentToDesiredNormPlaneVelocityVec_WS );
		} else {
			// The current and new velocities are close enough together to call the same...
			m_NormVelRotatedToXZ_WS = StickNormVelocityVecXZ_WS;
		}

		// Rotate our new plane velocity out of plane space...
		TempVec3A.CrossYWithVec( m_NormVelRotatedToXZ_WS );
		TempVec3B.Cross( TempVec3A, m_SurfaceUnitNorm_WS );

		if( TempVec3A.SafeUnitAndMag( TempVec3B ) != -1.0f ) {
			TempVec3A.Mul( m_NormVelRotatedToXZ_WS.MagXZ() );
		} else {
			TempVec3A.Zero();
		}

		// Finally, update model space velocity...
		m_Velocity_WS.Mul( TempVec3A, m_fMaxFlatSurfaceSpeed_WS );
	} else if( m_nTractionMode == TRACTIONMODE_SLIDE ) {
		// Sliding...

		fTemp = m_pBotInfo_Walk->fSlideStrafeSpeed * FLoop_fPreviousLoopSecs * m_XlatStickNormVecXZ_MS.x * m_UnitVelocityXZ_WS.Dot( m_MountUnitFrontXZ_WS );
		FMATH_BIPOLAR_CLAMPMAX( fTemp, FMATH_HALF_PI );

		TempQuat.BuildQuat( m_SurfaceUnitNorm_WS, fTemp );
		TempQuat.MulPoint( m_Velocity_WS );
	}

	WS2MS( m_Velocity_MS, m_Velocity_WS );

	// Due to floating point precision errors, we need to snap the model space X and Z velocity
	// components to 0 if they are close to 0. This prevents the bot from oscillating when the X and Z
	// components alternate between >0 and <0 every other frame...
	if( m_Velocity_MS.x>=-0.001f && m_Velocity_MS.x<0.001f ) {
		m_Velocity_MS.x = 0.0f;
	}
	if( m_Velocity_MS.z>=-0.001f && m_Velocity_MS.z<0.001f ) {
		m_Velocity_MS.z = 0.0f;
	}
	MS2WS( m_Velocity_WS, m_Velocity_MS );

	VelocityHasChanged();
}


// Computes m_MovingSurfaceUnitVec_WS assuming that we're on a surface.
// Returns TRUE if m_MovingSurfaceUnitVec_WS was computed without issue.
// Returns FALSE if either the velocity was zero or if the velocity is
// coincident with the world Y axis. In this case, m_MovingSurfaceUnitVec_WS
// is set to NULL.
BOOL CBot::ComputeMovingSurfaceVector( void ) {
	CFVec3A Right_WS;

	// Attempt to find Right velocity vector using the world Y axis...
	Right_WS.CrossYWithVec( m_Velocity_WS );

	if( Right_WS.MagSq() > 0.00001f ) {
		// We computed a valid Right vector.
		// Compute m_MovingSurfaceUnitVec_WS...
		m_MovingSurfaceUnitVec_WS.UnitCross( Right_WS, m_SurfaceUnitNorm_WS );

		// Fixup the vector so that it always points in the general direction of
		// our velocity...
		if( m_MovingSurfaceUnitVec_WS.Dot( m_Velocity_WS ) < 0.0f ) {
			m_MovingSurfaceUnitVec_WS.Negate();
		}

		return TRUE;
	}

	// Velocity vector is nearly vertical, or is zero.

	m_MovingSurfaceUnitVec_WS.Zero();

	return FALSE;
}


void CBot::HandleGroundAnimations( void ) {
	f32 fPrevWalkUnitTime/*, fTemp*/;
	f32 fAlertModeUnitRunBlenScale;
	f32 fWalkUnitDelta;
	
	f32 fLastUnitRunBlend	= m_fUnitRunBlend;
	f32 fLastUnitWalkBlend	= m_fUnitWalkBlend;

	FMATH_CLEARBITMASK( m_nBotFlags, BOTFLAG_SNEAKING );

	if( m_nTractionMode == TRACTIONMODE_NORMAL ) {
		// NKM - So that I can pull the bot along and not animate the feet
		if( ( m_nBotFlags & BOTFLAG_NO_FEET_ANIM ) ) {
			m_fUnitRunBlend -= m_pBotInfo_Walk->fIdleToSneakDeltaSpeed * FLoop_fPreviousLoopSecs;
			m_fUnitFeetAnim = m_pBotInfo_Walk->fAnimAtRestWalkRunUnitTime;
			m_fUnitWalkBlend -= m_pBotInfo_Walk->fIdleToWalkDeltaSpeed * FLoop_fPreviousLoopSecs;
			m_fUnitSneakBlend -= m_pBotInfo_Walk->fIdleToSneakDeltaSpeed * FLoop_fPreviousLoopSecs;
			FMATH_CLAMPMIN( m_fUnitSneakBlend, 0.0f );
			FMATH_CLAMPMIN( m_fUnitWalkBlend, 0.0f );
			FMATH_CLAMPMIN( m_fUnitRunBlend, 0.0f );
			FMATH_CLEARBITMASK( m_nBotFlags, BOTFLAG_NO_FEET_ANIM );
		} else {
			if( m_fNormSpeedAlongSurface_WS>0.0f && m_fNormSpeedAlongSurface_WS<=m_pBotInfo_Walk->fMinWalkNormVelocity && m_fXlatStickNormMag>0.0f && m_fXlatStickNormMag<m_fMaxSneakStickMag ) {
				// Sneaking...

				m_fUnitSneakBlend += m_pBotInfo_Walk->fIdleToSneakDeltaSpeed * FLoop_fPreviousLoopSecs;
				FMATH_CLAMPMAX( m_fUnitSneakBlend, 1.0f );

				FMATH_SETBITMASK( m_nBotFlags, BOTFLAG_SNEAKING );
			} else {
				// Not sneaking...

				m_fUnitSneakBlend -= m_pBotInfo_Walk->fIdleToSneakDeltaSpeed * FLoop_fPreviousLoopSecs;
				FMATH_CLAMPMIN( m_fUnitSneakBlend, 0.0f );
			}

			// Compute idle/sneak/walk/run blending...
			if( m_fNormSpeedAlongSurface_WS <= m_pBotInfo_Walk->fMinWalkNormVelocity ) {
				// We're moving slower than minimum walk speed...

				// no run blend at this speed
				m_fUnitRunBlend = 0.0f;

				//ME:  not sure about this, but changing it so that feet get reset.  Was m_fNormSpeedAlongSurface_WS==0.0f
				if( m_fNormSpeedAlongSurface_WS < m_pBotInfo_Walk->fMinWalkNormVelocity && m_fControls_RotateCW==0.0f && m_fUnitSneakBlend==0.0f && m_fUnitWalkBlend==0.0f ) {
					// We're not moving and the player is not rotating...
					// Reset anim start point so feet look correct when
					// bot begins walking again.
					m_fUnitFeetAnim = m_pBotInfo_Walk->fAnimAtRestWalkRunUnitTime;
				}

				if( IsReverseTransition() ) {
					// The player is rotating to face camera, increase walk blend towards full
					m_fUnitWalkBlend += m_pBotInfo_Walk->fIdleToWalkDeltaSpeed * FLoop_fPreviousLoopSecs;
					FMATH_CLAMPMAX( m_fUnitWalkBlend, 1.0f );
				} else if( (m_fXlatStickNormMag==0.0f && m_fControls_RotateCW==0.0f) ) {
					// The player is not moving nor rotating, reduce walk blend towards zero
					m_fUnitWalkBlend -= m_pBotInfo_Walk->fIdleToWalkDeltaSpeed * FLoop_fPreviousLoopSecs;
					FMATH_CLAMPMIN( m_fUnitWalkBlend, 0.0f );
				} else if( m_fSpeedAlongSurface_WS == 0.0f && IsReverseFacingForward() && (!(m_nBotFlags & BOTFLAG_HIPSLACK_OVERRIDE) && fmath_Abs(m_fLegsYaw_MS) < m_pBotInfo_Walk->fHipYawSlackWhileStoppedThreshold )) {
					// The player is not moving and hips are within slack threshold, reduce walk blend towards zero
					m_fUnitWalkBlend -= m_pBotInfo_Walk->fIdleToWalkDeltaSpeed * FLoop_fPreviousLoopSecs;
					FMATH_CLAMPMIN( m_fUnitWalkBlend, 0.0f );
				} else if( m_fSpeedAlongSurface_WS == 0.0f && !IsReverseFacingForward() && (!(m_nBotFlags & BOTFLAG_HIPSLACK_OVERRIDE) && (FMATH_DEG2RAD( 180.0f) - fmath_Abs(m_fLegsYaw_MS)) < m_pBotInfo_Walk->fHipYawSlackWhileStoppedThreshold )) {
					// The player is not moving and hips are within slack threshold, reduce walk blend towards zero
					m_fUnitWalkBlend -= m_pBotInfo_Walk->fIdleToWalkDeltaSpeed * FLoop_fPreviousLoopSecs;
					FMATH_CLAMPMIN( m_fUnitWalkBlend, 0.0f );
				} else {
					// The player is moving or rotating, increase walk blend towards full
					m_fUnitWalkBlend += m_pBotInfo_Walk->fIdleToWalkDeltaSpeed * FLoop_fPreviousLoopSecs;
					FMATH_CLAMPMAX( m_fUnitWalkBlend, 1.0f );
				}
			} else if( m_fNormSpeedAlongSurface_WS <= m_pBotInfo_Walk->fMinRunBlendNormVelocity ) {
				// We're moving somewhere between min walk and min run blend speed...

				// no run blend at this speed
				m_fUnitRunBlend = 0.0f;
				m_fUnitWalkBlend = 1.0f;
			}
			else if( m_fNormSpeedAlongSurface_WS <= m_pBotInfo_Walk->fMaxRunBlendNormVelocity ) {
				// We're moving somewhere between min run blend and max run speed...
				f32 fRunBlendInRange = m_pBotInfo_Walk->fMaxRunBlendNormVelocity - m_pBotInfo_Walk->fMinRunBlendNormVelocity;
				if (IsPanicOn())
				{	//special case, slams the
					fRunBlendInRange *= 0.25f;//    //in panic mode, the run blend in speed range is quartered
				}
				m_fUnitRunBlend = fmath_Div( m_fNormSpeedAlongSurface_WS - m_pBotInfo_Walk->fMinRunBlendNormVelocity, fRunBlendInRange );
				FMATH_CLAMPMAX(m_fUnitRunBlend, 1.0f);
				m_fUnitWalkBlend = 1.0f;
			} else {
				// We're moving at full run speed...
				m_fUnitRunBlend = 1.0f;
				m_fUnitWalkBlend = 1.0f;
			}
		}
	} else {
		// Low-traction, slipping and sliding...
		if( m_nTractionMode==TRACTIONMODE_LOW_TRACTION && m_fXlatStickNormMag==0.0f && m_fControls_RotateCW==0.0f ) {
			m_fUnitWalkBlend -= m_pBotInfo_Walk->fIdleToWalkDeltaSpeed * FLoop_fPreviousLoopSecs;
			FMATH_CLAMPMIN( m_fUnitWalkBlend, 0.0f );
		} else {
			m_fUnitWalkBlend += m_pBotInfo_Walk->fIdleToWalkDeltaSpeed * FLoop_fPreviousLoopSecs;
			FMATH_CLAMPMAX( m_fUnitWalkBlend, 1.0f );
		}

		m_fUnitSneakBlend -= m_pBotInfo_Walk->fIdleToSneakDeltaSpeed * FLoop_fPreviousLoopSecs;
		FMATH_CLAMPMIN( m_fUnitSneakBlend, 0.0f );

		if( m_nTractionMode==TRACTIONMODE_SLIP || m_nTractionMode==TRACTIONMODE_LOW_TRACTION && (m_fXlatStickNormMag!=0.0f || m_fControls_RotateCW!=0.0f) ) {
			m_fUnitRunBlend += m_pBotInfo_Walk->fIdleToSneakDeltaSpeed * FLoop_fPreviousLoopSecs;
			FMATH_CLAMPMAX( m_fUnitRunBlend, 1.0f );
		} else {
			m_fUnitRunBlend -= m_pBotInfo_Walk->fIdleToSneakDeltaSpeed * FLoop_fPreviousLoopSecs;
			FMATH_CLAMPMIN( m_fUnitRunBlend, 0.0f );
		}
	}

	// Update the unitPanic value. It controls wether run will look panic'ed or not...
	if( IsPanicOn() ) {
		m_fUnitPanic += _PANIC_MAX_POSDELTA_PER_SEC*FLoop_fPreviousLoopSecs;
	} else if (m_fUnitPanic> 0.0f){
		m_fUnitPanic -= _PANIC_MAX_NEGDELTA_PER_SEC*FLoop_fPreviousLoopSecs;
	}
	FMATH_CLAMP(m_fUnitPanic, 0.0f, 1.0f);

	// Update the unitAlert value. It controls wether run will look alerted or not.
	if( !IsBackBroken() ) {
		m_fUnitAlert += _ALERT_MAX_DELTA_PER_SEC*FLoop_fPreviousLoopSecs*(-1.0f+2.0f*(IsAlertOn()));
	} else {
		m_fUnitAlert = 0.0f;
	}
	
	FMATH_CLAMP(m_fUnitAlert, 0.0f, 1.0f);

	if( (IsRolling() || IsHopping()) && (m_fUnitHop > 0.0f) ) {
		// If Rolling, no walk, no panic, etc...

		m_fUnitWalkBlend = 0.0f;
		m_fUnitRunBlend = 0.0f;
		m_fUnitPanic = 0.0f;
	}

	// Advance idle animations...
	UpdateIdle();
	if( m_anAnimStackIndex[ASI_STAND] != -1 ) {
		DeltaTime( ASI_STAND );
	}

	if( m_anAnimStackIndex[ASI_STAND_ALERT] != -1 ) {
//		DeltaTime( ASI_STAND_ALERT );
		SetControlValue( ASI_STAND_ALERT, m_fUnitAlert );
	}

	// Deal with limp animations...
	if( m_nLimpState == BOTLIMPSTATE_LEFT ) {
		FASSERT( (m_anAnimStackIndex[ASI_STAND_LIMP_LEFT] != -1) &&
				 (m_anAnimStackIndex[ASI_LIMP_LEFT] != -1) );

		DeltaTime( ASI_STAND_LIMP_LEFT );
		if( m_fUnitLimpBlend < 1.0f ) {
			m_fUnitLimpBlend += FLoop_fPreviousLoopSecs * m_pBotInfo_Walk->fLimpOOBlendinTime;
			FMATH_CLAMP_MAX1( m_fUnitLimpBlend );
			SetControlValue( ASI_STAND_LIMP_LEFT, m_fUnitLimpBlend );
		}
	} else if( m_nLimpState == BOTLIMPSTATE_RIGHT ) {
		FASSERT( (m_anAnimStackIndex[ASI_STAND_LIMP_RIGHT] != -1) &&
				 (m_anAnimStackIndex[ASI_LIMP_RIGHT] != -1) );
				
		DeltaTime( ASI_STAND_LIMP_RIGHT );
		if( m_fUnitLimpBlend < 1.0f ) {
			m_fUnitLimpBlend += FLoop_fPreviousLoopSecs * m_pBotInfo_Walk->fLimpOOBlendinTime;
			FMATH_CLAMP_MAX1( m_fUnitLimpBlend );
			SetControlValue( ASI_STAND_LIMP_RIGHT, m_fUnitLimpBlend );
		}
	}

	// Set idle/walk/run blending...
	fAlertModeUnitRunBlenScale = (f32) !IsAlertOn();	// When alert mode is on, the run animations(regular or panic) should never blend in. It just ain't right.

	m_fUnitRunBlend *= fAlertModeUnitRunBlenScale;

	if( FMATH_FABS( m_fUnitWalkBlend - fLastUnitWalkBlend ) > m_pBotInfo_Walk->fOOMaxWalkRunBlendTime * FLoop_fPreviousLoopSecs ) {
		f32 fTmp = m_fUnitWalkBlend;
		if( m_fUnitWalkBlend > fLastUnitWalkBlend ) {
			m_fUnitWalkBlend = fLastUnitWalkBlend + (m_pBotInfo_Walk->fOOMaxWalkRunBlendTime * FLoop_fPreviousLoopSecs);
			FMATH_CLAMPMAX( m_fUnitWalkBlend, fTmp );
		} else {
			m_fUnitWalkBlend = fLastUnitWalkBlend - (m_pBotInfo_Walk->fOOMaxWalkRunBlendTime * FLoop_fPreviousLoopSecs);
			FMATH_CLAMPMIN( m_fUnitWalkBlend, fTmp );
		}
		FMATH_CLAMP_UNIT_FLOAT( m_fUnitWalkBlend );
	}

	if( FMATH_FABS( m_fUnitRunBlend - fLastUnitRunBlend ) > m_pBotInfo_Walk->fOOMaxWalkRunBlendTime * FLoop_fPreviousLoopSecs ) {
		f32 fTmp = m_fUnitRunBlend;
		if( m_fUnitRunBlend > fLastUnitRunBlend ) {
			m_fUnitRunBlend = fLastUnitRunBlend + (m_pBotInfo_Walk->fOOMaxWalkRunBlendTime * FLoop_fPreviousLoopSecs);
		} else {
			m_fUnitRunBlend = fLastUnitRunBlend - (m_pBotInfo_Walk->fOOMaxWalkRunBlendTime * FLoop_fPreviousLoopSecs);
		}
		FMATH_CLAMP_UNIT_FLOAT( m_fUnitRunBlend );
	}


	if( m_anAnimStackIndex[ASI_RUN] != -1 ) {
		SetControlValue( ASI_RUN, m_fUnitRunBlend );
	}

	SetControlValue( ASI_WALK, m_fUnitWalkBlend );

	if( m_anAnimStackIndex[ASI_WALK_ALERT] != -1 ) {
		SetControlValue( ASI_WALK_ALERT, m_fUnitWalkBlend*m_fUnitAlert );
	}
	if( m_anAnimStackIndex[ASI_RUN_PANIC] != -1 ) {
		f32 fPanicControlVal = m_fUnitPanic*fAlertModeUnitRunBlenScale;
		SetControlValue( ASI_RUN_PANIC, fPanicControlVal );
	}
	if( m_anAnimStackIndex[ASI_SNEAK] != -1 ) {
		SetControlValue( ASI_SNEAK, m_fUnitSneakBlend*(1.0f - m_fUnitAlert) );
	}

	f32 fLimpBlend;
	if( m_nLimpState == BOTLIMPSTATE_LEFT ) {
		fLimpBlend = FMATH_MAX( m_fUnitRunBlend, m_fUnitWalkBlend );
		FMATH_CLAMPMAX( fLimpBlend, m_fUnitLimpBlend );
		SetControlValue( ASI_LIMP_LEFT, fLimpBlend );
	}

	if( m_nLimpState == BOTLIMPSTATE_RIGHT ) {
		fLimpBlend = FMATH_MAX( m_fUnitRunBlend, m_fUnitWalkBlend );
		FMATH_CLAMPMAX( fLimpBlend, m_fUnitLimpBlend );
		SetControlValue( ASI_LIMP_RIGHT, fLimpBlend );
	}

	static f32 __Boost2AnimPct = 0.5f;
	if( m_nMoveState != BOTMOVESTATE_NONE) {
		static AnimStackIndex_e auMoveTypeToASISlot[NUM_BOTMOVETYPES] = {ASI_HOP_LEFT, ASI_HOP_RIGHT, ASI_STARTLE, ASI_ROLL_LEFT, ASI_ROLL_RIGHT};
		AnimStackIndex_e uASI = auMoveTypeToASISlot[m_nMoveType];
		switch (m_nMoveState) {
			case BOTMOVESTATE_BLENDIN:
				{
					FASSERT(m_anAnimStackIndex[uASI]);	 // BOTMOVESTATE left or right should not be enabled if corresponding ASI is not valid

					if (m_fUnitHop == 0.0f) {
						UpdateTime( uASI, 0.0f );
					}
					
					if (!(m_nMoveType == BOTMOVETYPE_HOP_LEFT || m_nMoveType==BOTMOVETYPE_HOP_RIGHT || m_nMoveType==BOTMOVETYPE_ROLL_RIGHT || m_nMoveType==BOTMOVETYPE_ROLL_LEFT) ||
						(m_fUnitHop != 0.0f ||
						m_fSpeedXZ_WS == 0.0f))	{	 // Stop before you hop...
						// Blend in the hop really quick...
						f32 fOldUnitHop = m_fUnitHop;
						m_fUnitHop += _HOP_DELTA_PER_SEC*FLoop_fPreviousLoopSecs;
						FMATH_CLAMPMAX(m_fUnitHop, 1.0f);
						SetControlValue( uASI, m_fUnitHop );

						if ((m_nMoveType == BOTMOVETYPE_HOP_LEFT || m_nMoveType==BOTMOVETYPE_HOP_RIGHT) &&
							fOldUnitHop < 1.0f && m_fUnitHop >=1.0f)
						{
							CFVec3A Impulse_WS;
							Impulse_WS.Set(MtxToWorld()->m_vRight);
							Impulse_WS.Mul(1.0f - 2.0f*(m_nMoveType == BOTMOVETYPE_HOP_LEFT));
							Impulse_WS.Add(MtxToWorld()->m_vUp);
							Impulse_WS.Mul(m_pBotInfo_Walk->fHopLRImpulseMag);
							ApplyVelocityImpulse_WS(Impulse_WS);
						}
					
						else if ((m_nMoveType == BOTMOVETYPE_ROLL_LEFT || m_nMoveType == BOTMOVETYPE_ROLL_RIGHT) &&
								fOldUnitHop == 0.0f && m_fUnitHop>0.0f)
						{
							static f32 __XZMag = 23.0f;
							static f32 __YMag = 6.0f;
							CFVec3A Impulse_WS;
							Impulse_WS.Set(MtxToWorld()->m_vRight);
							Impulse_WS.Mul(1.0f - 2.0f*(m_nMoveType == BOTMOVETYPE_ROLL_LEFT));
							Impulse_WS.Mul(m_pBotInfo_Walk->fRollLRImpulseMagXZ);
							Impulse_WS.y = m_pBotInfo_Walk->fRollLRImpulseMagY;
							ApplyVelocityImpulse_WS(Impulse_WS);
						}
						
						f32 fOldAnimPct = GetTime(uASI)*GetOOTotalTime(uASI);
						//wait for hop anim to be finished
						if (DeltaTime( uASI, FLoop_fPreviousLoopSecs, TRUE ) ||
							((m_nMoveType == BOTMOVETYPE_HOP_LEFT || m_nMoveType == BOTMOVETYPE_HOP_RIGHT) && GetTime(uASI)*GetOOTotalTime(uASI) > m_pBotInfo_Walk->fHopAnimLandPct) ||
							((m_nMoveType == BOTMOVETYPE_ROLL_LEFT || m_nMoveType == BOTMOVETYPE_ROLL_RIGHT) && GetTime(uASI)*GetOOTotalTime(uASI) > m_pBotInfo_Walk->fRollAnimLandPct))
						{ //advance move state 
							m_nMoveState = BOTMOVESTATE_BLENDOUT;

						}
					}
				}
				break;
			case BOTMOVESTATE_BLENDOUT:
				{
					m_fUnitHop -= _HOP_DELTA_PER_SEC*FLoop_fPreviousLoopSecs;
					FMATH_CLAMPMIN(m_fUnitHop, 0.0f);
					SetControlValue( uASI, m_fUnitHop );

					if( m_fUnitHop == 0.0f ) {
					   m_nMoveState = BOTMOVESTATE_NONE;
					}
				}
				break;
		}
	}


	// Compute sneak/run/walk animation delta...
	BOOL bWasMovingBackWards = IsMovingBackwards();
	ClearBotFlag_MovingBackwards();

	//ME:	modified this section so that rather than updating m_fUnitFeetAnim directly, each part changes fWalkUnitDelta, which is then checked,
	//		multiplied by previousloopsecs, and then added to m_fUnitFeetAnim

	fWalkUnitDelta = 0.0f;

	if( !HasNoLegs() ) {
		switch( m_nTractionMode ) {
		case TRACTIONMODE_NORMAL:
			if( m_fSpeedAlongSurface_WS ) {
				// bot is moving
				f32 fDelatFeetAnimPerFootRun;

				if( m_anAnimStackIndex[ASI_WALK_ALERT] == -1 ) {
					fWalkUnitDelta = m_fUnitWalkBlend * m_pBotInfo_Walk->fDeltaFeetAnimPerFootWalk;
				}
				else {
					//this bot supports alertwalk
					fWalkUnitDelta = m_fUnitWalkBlend * FMATH_FPOT( m_fUnitAlert, m_pBotInfo_Walk->fDeltaFeetAnimPerFootWalk, m_pBotInfo_Walk->fDeltaFeetAnimPerFootAlertWalk );
				}

				if( m_anAnimStackIndex[ASI_RUN_PANIC] == -1 ) {
					fDelatFeetAnimPerFootRun = m_pBotInfo_Walk->fDeltaFeetAnimPerFootRun;
				}
				else {
					fDelatFeetAnimPerFootRun = FMATH_FPOT( m_fUnitPanic, m_pBotInfo_Walk->fDeltaFeetAnimPerFootRun, m_pBotInfo_Walk->fDeltaFeetAnimPerFootPanicRun );
				}
				fWalkUnitDelta = FMATH_FPOT( m_fUnitRunBlend, fWalkUnitDelta, fDelatFeetAnimPerFootRun );
				fWalkUnitDelta = FMATH_FPOT( m_fUnitSneakBlend, fWalkUnitDelta, m_pBotInfo_Walk->fDeltaFeetAnimPerFootSneak );
				fWalkUnitDelta *= m_fSpeedAlongSurface_WS;

				if( IsReversePastHalfway() )
				{
					// bot is in reverse-facing mode, so
					// cycling of anims will be reversed
					if( m_Velocity_MS.z > 0.0f ) {
						// cycle animation backwards if
						// moving backwards 
						SetBotFlag_MovingBackwards();

						fWalkUnitDelta *= -1.0f;
					}
				}
				else
				{
					static f32 fHipFlipHysteresis = 2.0f;

					if ( bWasMovingBackWards) {
						if (m_Velocity_MS.z < fHipFlipHysteresis ) {
							SetBotFlag_MovingBackwards();
						}
						else {
							ClearBotFlag_MovingBackwards();
						}
					}
					else {//if (!bWasMovingBackWards) {
						if (m_Velocity_MS.z >-fHipFlipHysteresis) {
							ClearBotFlag_MovingBackwards();
						} 
						else {
							SetBotFlag_MovingBackwards();
						}
					}
				}
			} else {
				// bot is not moving
				if( m_fControls_RotateCW < 0.0f ) {	
					// bot turning left
					fWalkUnitDelta =  m_pBotInfo_Walk->fDeltaFeetAnimPerRadian
									* FMATH_FPOT( -m_fControls_RotateCW,
								 				m_pBotInfo_Walk->fDeltaFeetAnimPerFootRun,
								 				m_pBotInfo_Walk->fDeltaFeetAnimPerFootWalk
												);

				} else if( m_fControls_RotateCW > 0.0f ) {	
					// bot turning right
					fWalkUnitDelta = m_pBotInfo_Walk->fDeltaFeetAnimPerRadian
										* FMATH_FPOT( m_fControls_RotateCW,
													m_pBotInfo_Walk->fDeltaFeetAnimPerFootRun,
													m_pBotInfo_Walk->fDeltaFeetAnimPerFootWalk
													);

				} else if( IsReverseTransition() ) {
					// bot turning to face camera
					fWalkUnitDelta = m_pBotInfo_Walk->fDeltaFeetAnimPerRadian
										* FMATH_FPOT( m_fControls_RotateCW,
													m_pBotInfo_Walk->fDeltaFeetAnimPerFootRun,
													m_pBotInfo_Walk->fDeltaFeetAnimPerFootWalk
													);

				}
			}

			break;

		case TRACTIONMODE_SLIP:
		case TRACTIONMODE_LOW_TRACTION:
			if( m_fXlatStickNormMag > 0.0f ) {
				/*fWalkUnitDelta = m_fUnitWalkBlend * m_pBotInfo_Walk->fDeltaFeetAnimPerFootWalk;
				fWalkUnitDelta = FMATH_FPOT( m_fUnitRunBlend, fWalkUnitDelta, m_pBotInfo_Walk->fDeltaFeetAnimPerFootRun );
				fWalkUnitDelta = FMATH_FPOT( m_fUnitSneakBlend, fWalkUnitDelta, m_pBotInfo_Walk->fDeltaFeetAnimPerFootSneak );
				fWalkUnitDelta *= FMATH_MAX( m_fXlatStickNormMag, m_pBotInfo_Walk->fMinWalkNormVelocity ) * 25.0f;*/
				
				// NKM - Made this change so that when on a slippery surface your feet move as if you were running at full speed
				fWalkUnitDelta = m_fUnitWalkBlend * m_pBotInfo_Walk->fDeltaFeetAnimPerFootWalk;
				fWalkUnitDelta *= 25.0f;

				if( m_XlatStickUnitVecXZ_MS.z < 0.0f ) {
					SetBotFlag_MovingBackwards();
				}
			} else {
				/*if( m_fControls_RotateCW < 0.0f ) {
					fWalkUnitDelta -= m_pBotInfo_Walk->fDeltaFeetAnimPerRadian * m_fControls_RotateCW
										* FMATH_FPOT( m_fUnitRunBlend,
													m_pBotInfo_Walk->fDeltaFeetAnimPerFootWalk,
													m_pBotInfo_Walk->fDeltaFeetAnimPerFootRun
													);
				} else if( m_fControls_RotateCW > 0.0f ) {
					fWalkUnitDelta += m_pBotInfo_Walk->fDeltaFeetAnimPerRadian * m_fControls_RotateCW
										* FMATH_FPOT( m_fUnitRunBlend,
													m_pBotInfo_Walk->fDeltaFeetAnimPerFootWalk,
													m_pBotInfo_Walk->fDeltaFeetAnimPerFootRun
													);
				}*/
				
				// NKM - Made this change so that when on a slippery surface your feet move as if you were running at full speed
				if( m_fControls_RotateCW < 0.0f ||  m_fControls_RotateCW > 0.0f ) {
					fWalkUnitDelta += m_pBotInfo_Walk->fDeltaFeetAnimPerRadian
						* FMATH_FPOT( m_fUnitRunBlend,
								m_pBotInfo_Walk->fDeltaFeetAnimPerFootWalk,
								m_pBotInfo_Walk->fDeltaFeetAnimPerFootRun
								) * 25.0f;
				}
			}

			break;

		case TRACTIONMODE_SLIDE:
			fWalkUnitDelta = m_pBotInfo_Walk->fDeltaFeetAnimSlideWalkMult * m_pBotInfo_Walk->fDeltaFeetAnimPerFootWalk;

			if (m_Velocity_MS.z < 0 ) {
				SetBotFlag_MovingBackwards();
			}
			break;

		default:
			FASSERT_NOW;
		}

		if( m_nState == STATE_GROUND ) {
			// Advance run/walk animation and sound...
			fPrevWalkUnitTime = GetUnitTime( ASI_WALK );
		}

		// now update our foot time, either backwards or forwards...
		if( m_nLimpState != BOTLIMPSTATE_NONE ) {
			fWalkUnitDelta *= m_pBotInfo_Walk->fLimpAnimSpeedMult;
		}

		FMATH_CLAMPMAX( fWalkUnitDelta, 2.0f );

		fWalkUnitDelta *= FLoop_fPreviousLoopSecs;

		if( IsMovingBackwards() ) {
			m_fUnitFeetAnim -= fWalkUnitDelta;

			while( m_fUnitFeetAnim < 1.0f ) {
				m_fUnitFeetAnim += 1.0f;
			}
		} else {
			m_fUnitFeetAnim += fWalkUnitDelta;

			while( m_fUnitFeetAnim > 1.0f ) {
				m_fUnitFeetAnim -= 1.0f;
			}
		}


		// Advance sneak/run/walk animation and sound...
		if( m_anAnimStackIndex[ASI_RUN] != -1 ) {
			UpdateUnitTime( ASI_RUN, m_fUnitFeetAnim );
		}
		UpdateUnitTime( ASI_WALK, m_fUnitFeetAnim );
		if( m_anAnimStackIndex[ASI_WALK_ALERT] != -1 ) {
			UpdateUnitTime( ASI_WALK_ALERT, m_fUnitFeetAnim );
		}
		if( m_anAnimStackIndex[ASI_RUN_PANIC] != -1 ) {
			UpdateUnitTime( ASI_RUN_PANIC, m_fUnitFeetAnim );
		}

		if( m_anAnimStackIndex[ASI_SNEAK] != -1 ) {
			UpdateUnitTime( ASI_SNEAK, m_fUnitFeetAnim );
		}

		if( m_nLimpState == BOTLIMPSTATE_LEFT ) {
			UpdateUnitTime( ASI_LIMP_LEFT, m_fUnitFeetAnim );
		}

		if( m_nLimpState == BOTLIMPSTATE_RIGHT ) {
			UpdateUnitTime( ASI_LIMP_RIGHT, m_fUnitFeetAnim );
		}

		if( m_nState == STATE_GROUND ) {
			m_fStepVolumeScale += ( m_fStepVolumeScaleDir * _SOUND_VOLUME_SCALE * FLoop_fPreviousLoopSecs );
			FMATH_CLAMP( m_fStepVolumeScale, _SOUND_VOLUME_MIN, _SOUND_VOLUME_MAX );

			// Play footstep sounds...
			if( !(m_nBotFlags & BOTFLAG_SNEAKING) && (m_nJumpState == BOTJUMPSTATE_NONE) ) {
				BOOL bRightFootDown = FALSE;
				BOOL bFootDown = FALSE;

				f32 fNewWalkUnitTime = GetUnitTime( ASI_WALK );

				if( IsMovingBackwards() ) {
					FMATH_FSWAP( fNewWalkUnitTime, fPrevWalkUnitTime );
				}

				if( fNewWalkUnitTime >= fPrevWalkUnitTime ) {
					if( m_pBotInfo_Walk->fAnimLeftFootDownUnitTime>fPrevWalkUnitTime && m_pBotInfo_Walk->fAnimLeftFootDownUnitTime<fNewWalkUnitTime ) {
						// Left foot is down...
						bFootDown = TRUE;
						bRightFootDown = FALSE;
					} else if( m_pBotInfo_Walk->fAnimRightFootDownUnitTime>fPrevWalkUnitTime && m_pBotInfo_Walk->fAnimRightFootDownUnitTime<fNewWalkUnitTime ) {
						// Right foot is down...
						bFootDown = TRUE;
						bRightFootDown = TRUE;
					}
				} else {
					if( m_pBotInfo_Walk->fAnimLeftFootDownUnitTime>fPrevWalkUnitTime || m_pBotInfo_Walk->fAnimLeftFootDownUnitTime<fNewWalkUnitTime ) {
						// Left foot is down...
						bFootDown = TRUE;
						bRightFootDown = FALSE;
					} else if( m_pBotInfo_Walk->fAnimRightFootDownUnitTime>fPrevWalkUnitTime || m_pBotInfo_Walk->fAnimRightFootDownUnitTime<fNewWalkUnitTime ) {
						// Right foot is down...
						bFootDown = TRUE;
						bRightFootDown = TRUE;
					}
				}

				if( bFootDown ) {
					if( m_pBotInfo_Gen->fUnitDustKickup > 0.0f ) {
						NotifyFootDown( !bRightFootDown, bRightFootDown, m_fNormSpeedAlongSurface_WS );

						if( m_fNormSpeedAlongSurface_WS > 0.4f ) {
							MakeFootImpactDust( !bRightFootDown, bRightFootDown, m_pBotInfo_Gen->fUnitDustKickup, &m_FeetToGroundCollImpact.UnitFaceNormal );

							if( m_pStickyEntity && m_pStickyEntity->GetVerlet() ) {
								CFVec3A ImpulseVec_WS;

								ImpulseVec_WS.Set( 0.0f, -5000.0f, 0.0f );
								m_pStickyEntity->GetVerlet()->ApplyImpulse( &m_MtxToWorld.m_vPos, &ImpulseVec_WS );
							}
						}
					}
				}

				PlayFootSound( ( bFootDown && !bRightFootDown ), bRightFootDown, m_nSurfaceTypeOn );
			}
		}
	}

	UserAnim_LockWork();
}


void CBot::MakeFootImpactDust( BOOL bLeftFoot, BOOL bRightFoot, f32 fUnitIntensity, const CFVec3A *pEmissionUnitDir_WS ) {
	if( m_pSurfaceMtl->HasLooseDust() ) {
		const CFVec3A *pLeftPos_WS;
		const CFVec3A *pRightPos_WS;

		if( m_pLeftFootDustPos_WS && m_pRightFootDustPos_WS ) {
			if( bLeftFoot ) {
				pLeftPos_WS = m_pLeftFootDustPos_WS;
			}

			if( bRightFoot ) {
				pRightPos_WS = m_pRightFootDustPos_WS;
			}
		} else {
			bLeftFoot = TRUE;
			bRightFoot = FALSE;
			pLeftPos_WS = &m_MtxToWorld.m_vPos;
		}

		if( bLeftFoot ) {
			m_pSurfaceMtl->DrawParticle( CGCollMaterial::PARTICLE_TYPE_DUST, pLeftPos_WS, pEmissionUnitDir_WS, fUnitIntensity );
		}

		if( bRightFoot ) {
			m_pSurfaceMtl->DrawParticle( CGCollMaterial::PARTICLE_TYPE_DUST, pRightPos_WS, pEmissionUnitDir_WS, fUnitIntensity );
		}
	}
}


void CBot::HandleHipsAnimation( void ) {
	CFVec3A *pTempVec;
	f32 fUnit, fVelocityYaw_MS, fPrevLegsYaw_MS;

	// Compute where we want the hips to point...
	// first find model-space target angle for hips
	if( !IsJumping() ) {
		// not jumping
		if( (m_nJumpState == BOTJUMPSTATE_CABLE) ) {
			// on cable
			fVelocityYaw_MS = 0.0f;
		} else if (m_fSpeedAlongSurface_WS == 0.0f) {
			// not moving
			if( IsReverseFacingForward() )
			{
				if (m_nBotFlags & BOTFLAG_HIPSLACK_OVERRIDE)
				{
					fVelocityYaw_MS = 0.0f;
				}
				else if (FMATH_FABS(m_fLegsYaw_MS) > m_pBotInfo_Walk->fHipYawSlackWhileStoppedThreshold)
				{
					// if current hips angle exceeds allowed slack angle,
					// target angle becomes slack angle
				   fVelocityYaw_MS = FMATH_FSIGN(m_fLegsYaw_MS)*m_pBotInfo_Walk->fHipYawSlackWhileStoppedThreshold;
				}
				else
				{
					// hips within slack amount, so they don't move
					fVelocityYaw_MS = m_fLegsYaw_MS;
				}
			}
			else
			{
				if( IsReverseTransition() )
				{
					fVelocityYaw_MS = m_fReverseFacingYaw_MS;
				}
				else if (m_nBotFlags & BOTFLAG_HIPSLACK_OVERRIDE)
				{
					fVelocityYaw_MS = FMATH_DEG2RAD( 180.0f );
				}
				else if((FMATH_DEG2RAD( 180.0f ) - FMATH_FABS( m_fLegsYaw_MS )) > m_pBotInfo_Walk->fHipYawSlackWhileStoppedThreshold)
				{
				   fVelocityYaw_MS = FMATH_FSIGN(m_fLegsYaw_MS)*(FMATH_DEG2RAD( 180.0f )-m_pBotInfo_Walk->fHipYawSlackWhileStoppedThreshold);
				}
				else
				{
					fVelocityYaw_MS = m_fLegsYaw_MS;
				}
			}

		} else {
			// moving
			if( m_nTractionMode!=TRACTIONMODE_LOW_TRACTION && m_nTractionMode!=TRACTIONMODE_SLIP ) {
				// not low traction or slipping
				// get a target vector aligned with bot velocity
				pTempVec = &m_Velocity_MS;
			} else {
				// low traction or slipping
				// get a target vector aligned with stick direction
				pTempVec = &m_XlatStickNormVecXZ_MS;
			}

			// find target angle from vector
			if( !IsMovingBackwards() ) {   //moving backwards is relative to blinks reversefacing thing
				fVelocityYaw_MS = fmath_Atan( pTempVec->x, pTempVec->z );
			} else {
				// (rotate hips 180 degrees when moving backwards or exactly sideways)
				fVelocityYaw_MS = fmath_Atan( -pTempVec->x, -pTempVec->z );
			}

			// attempt to return hips to face forward if bot
			// is slowing to a stop
			if( m_fXlatStickNormMag == 0.0f ) {
				if( m_fSpeedAlongSurface_WS < m_pBotInfo_Walk->fSwivelHipsReturnSpeedThreshold ) {
					fUnit = fmath_Div( m_fSpeedAlongSurface_WS, m_pBotInfo_Walk->fSwivelHipsReturnSpeedThreshold );
					if( IsReversePastHalfway() )
					{
						// rotate towards 180 degrees if reverse facing
						fVelocityYaw_MS = FMATH_FPOT( fUnit, FMATH_DEG2RAD(180.0f), fVelocityYaw_MS );
					}
					else
					{
						// rotate towards 0 degrees
						fVelocityYaw_MS = FMATH_FPOT( fUnit, 0.0f, fVelocityYaw_MS );
					}
				}
			}
		}

		// Rotate hips yaw toward desired target angle...
		if( IsReversePastHalfway() )
		{
			f32 fInvLegs, fInvVel;

			fPrevLegsYaw_MS = m_fLegsYaw_MS;

			// invert yaw angles so that 180 = 0
			fInvLegs = FMATH_DEG2RAD( 180.0f ) - m_fLegsYaw_MS;
			if( fInvLegs > FMATH_DEG2RAD( 180.0f ) )
			{
				fInvLegs = fInvLegs - FMATH_DEG2RAD( 360.0f );
			}
			fInvVel = FMATH_DEG2RAD( 180.0f ) - fVelocityYaw_MS;
			if( fInvVel > FMATH_DEG2RAD( 180.0f ) )
			{
				fInvVel = fInvVel - FMATH_DEG2RAD( 360.0f );
			}

			if( fInvVel > fInvLegs )
			{
				fInvLegs += m_pBotInfo_Walk->fSwivelHipsDeltaRadiansPerSec * FLoop_fPreviousLoopSecs;
				FMATH_CLAMPMAX(fInvLegs, fInvVel );
			}
			else if( fInvVel < fInvLegs )
			{
				fInvLegs -= m_pBotInfo_Walk->fSwivelHipsDeltaRadiansPerSec * FLoop_fPreviousLoopSecs;
				FMATH_CLAMPMIN( fInvLegs, fInvVel );
			}

			// invert back to normal
			m_fLegsYaw_MS = FMATH_DEG2RAD( 180.0f ) - fInvLegs;
			if( m_fLegsYaw_MS > FMATH_DEG2RAD( 180.0f ) )
			{
				m_fLegsYaw_MS = m_fLegsYaw_MS - FMATH_DEG2RAD( 360.0f );
			}
		}
		else
		{
			f32 fBoostHipSwivelSpeed = 1.0f;
			if (m_nMoveState != BOTMOVESTATE_NONE)
			{
				//hips must unwind for hop to work, try to do it fast like
				fVelocityYaw_MS = 0.0f;
				fBoostHipSwivelSpeed = 2.0f;
			}

			fPrevLegsYaw_MS = m_fLegsYaw_MS;
			if( fVelocityYaw_MS > m_fLegsYaw_MS ) {
				m_fLegsYaw_MS += fBoostHipSwivelSpeed * m_pBotInfo_Walk->fSwivelHipsDeltaRadiansPerSec * FLoop_fPreviousLoopSecs;
				FMATH_CLAMPMAX( m_fLegsYaw_MS, fVelocityYaw_MS );
			} else if( fVelocityYaw_MS < m_fLegsYaw_MS ) {
				m_fLegsYaw_MS -= fBoostHipSwivelSpeed * m_pBotInfo_Walk->fSwivelHipsDeltaRadiansPerSec * FLoop_fPreviousLoopSecs;
				FMATH_CLAMPMIN( m_fLegsYaw_MS, fVelocityYaw_MS );
			}

		}
	}
	m_nBotFlags &= ~BOTFLAG_HIPSLACK_OVERRIDE;	//clear every frame
}


//--------------------------------------------------------------------------------------------------------------------------------
// Bot Weapon:
//--------------------------------------------------------------------------------------------------------------------------------

void CBot::ComputeApproxMuzzlePoint_WS( CFVec3A *pApproxMuzzlePoint_WS ) {
	CFVec3A Right, Front;
	FASSERT(m_pBotInfo_Weapon);
	Right.Mul( m_MtxToWorld.m_vRight, m_pBotInfo_Weapon->fOriginToApproxMuzzleX );
	Front.Mul( m_MountUnitFrontXZ_WS, m_pBotInfo_Weapon->fOriginToApproxMuzzleZ );

	pApproxMuzzlePoint_WS->Add( Right, Front ).Add( m_MtxToWorld.m_vPos );
	pApproxMuzzlePoint_WS->y += m_pBotInfo_Weapon->fOriginToApproxMuzzleY;
}


// Computes the following:
//   m_TargetLockUnitVec_WS
//   m_TargetedPoint_WS
//   m_fAimPitch_WS
//   m_fAimDeltaYaw_MS
//   m_nBotFlags::BOTFLAG_TARGET_LOCKED
void CBot::HandleTargeting( void ) {
	//ME:  Assuming all bots that do targeting have one of these...
	FASSERT( m_pBotInfo_MountAim );

	f32 fDesiredPitch_WS, fDesiredYaw_WS, fUnitMovement, fTimeStep;
	FMATH_CLEARBITMASK( m_nBotFlags, BOTFLAG_TARGET_LOCKED );

	// if we're driving a vehicle, either the vehicle doesn't have a weapon, or it will handle targeting
	if( m_pDrivingVehicle != NULL ) {
		return;
	}

	if( m_nPossessionPlayerIndex < 0 ) {
		// NPC-controlled bot...

		CFVec3A MuzzlePoint_WS;

		if( m_nControlsBot_Flags & CBotControl::FLAG_AIM_AT_TARGET_POINT ) {
			// AI wants us to aim at m_ControlBot_TargetPoint_WS...

			// Compute approximate muzzle point...
			ComputeApproxMuzzlePoint_WS( &MuzzlePoint_WS );

			// Compute vector from approx muzzle point to desired target...
			m_TargetLockUnitVec_WS.Sub( m_ControlBot_TargetPoint_WS, MuzzlePoint_WS );

			// Find out if desired target is in range...
			f32 fMag2 = m_TargetLockUnitVec_WS.MagSq();
			if( fMag2 > 0.001f ) {
				f32 fOOMag = fmath_InvSqrt( fMag2 );
				f32 fDot = fOOMag * m_TargetLockUnitVec_WS.Dot( m_MtxToWorld.m_vFront );

				if( fDot >= m_pBotInfo_MountAim->fAllowableLockAngleCos ) {
					// We can adjust to target the desired point...

					fDesiredPitch_WS = -fmath_Atan( m_TargetLockUnitVec_WS.y, m_TargetLockUnitVec_WS.MagXZ() );

					// If target point is behind us, don't adjust yaw...
					f32 fDot = m_TargetLockUnitVec_WS.x*m_MountUnitFrontXZ_WS.x + m_TargetLockUnitVec_WS.z*m_MountUnitFrontXZ_WS.z;
					if( fDot >= 0.1f ) {
						fDesiredYaw_WS = fmath_Atan( m_TargetLockUnitVec_WS.x, m_TargetLockUnitVec_WS.z );
					} else {
						fDesiredYaw_WS = m_fMountYaw_WS;
					}

					// Unitize the target lock unit vector...
					m_TargetLockUnitVec_WS.Mul( fOOMag );

					// Announce that we're locked...
					FMATH_SETBITMASK( m_nBotFlags, BOTFLAG_TARGET_LOCKED );
				}
			}

			if( !(m_nBotFlags & BOTFLAG_TARGET_LOCKED) ) {
				// Target point is out of reach...

				m_TargetLockUnitVec_WS = m_MtxToWorld.m_vFront;
				fDesiredPitch_WS = m_fMountPitch_WS;
				fDesiredYaw_WS = m_fMountYaw_WS;
				m_TargetedPoint_WS = m_MtxToWorld.m_vFront;
				m_TargetedPoint_WS.Mul(100.0f);
				ComputeWeaponMuzzlePoint_WS( &MuzzlePoint_WS );
				m_TargetedPoint_WS.Add(MuzzlePoint_WS);
			} else {
				//m_apWeapon[0]->ComputeMuzzlePoint_WS( &MuzzlePoint_WS );
				ComputeWeaponMuzzlePoint_WS( &MuzzlePoint_WS );
				m_TargetLockUnitVec_WS.Sub( m_ControlBot_TargetPoint_WS, MuzzlePoint_WS );
				if( m_TargetLockUnitVec_WS.SafeUnitAndMag(m_TargetLockUnitVec_WS) <= 0.0f ) {
					m_TargetLockUnitVec_WS = m_MtxToWorld.m_vFront;
				}
				m_TargetedPoint_WS = m_ControlBot_TargetPoint_WS;
			}

		} else {
			// AI wants us to just aim down mount's Z axis...

			m_TargetLockUnitVec_WS = m_MtxToWorld.m_vFront;
			fDesiredPitch_WS = m_fMountPitch_WS;
			fDesiredYaw_WS = m_fMountYaw_WS;
			FMATH_SETBITMASK( m_nBotFlags, BOTFLAG_TARGET_LOCKED );

			// !Unsure what to do here really, so let's just set the targeted point to
			// 100 feet away...
			//m_apWeapon[0]->ComputeMuzzlePoint_WS( &MuzzlePoint_WS );
			ComputeWeaponMuzzlePoint_WS( &MuzzlePoint_WS );
			m_TargetedPoint_WS.Mul( m_TargetLockUnitVec_WS, 100.0f ).Add( MuzzlePoint_WS );
		}
	} else {
		// Human-controlled bot...

//		if( m_apWeapon[0]->HasAmmo() ) {
			CFVec3A /*TargetPoint_WS, */MuzzlePoint_WS, MuzzleToTarget_WS;

			// Ask the weapon for the point that the player is targeting...
			PROTRACK_BEGINBLOCK("FindTargetPt");

				// Make sure we clear out the targeted mesh before we start, in
				// case we are calling a weapon function that doesn't do it.
				m_pTargetedMesh = NULL;

				//m_apWeapon[0]->HumanTargeting_FindDesiredTargetPoint_WS( &m_TargetedPoint_WS );
				// If the Bot has a weapon, use it's targeting routines
				if( m_apWeapon[0] != NULL ) {
					m_apWeapon[0]->HumanTargeting_FindDesiredTargetPoint_WS( &m_TargetedPoint_WS );

					if( m_apWeapon[1] != NULL ) {
						m_apWeapon[1]->SetTargetedEntity( m_apWeapon[0]->GetTargetedEntity() );
						if( m_apWeapon[1]->NeedsTargetedEntityForUse() && ( m_apWeapon[0]->GetTargetedEntity() == NULL ) ) {
							//go ahead and calculate a targeted entity for the second target as well
							m_apWeapon[1]->HumanTargeting_FindDesiredTargetPoint_WS( &m_TargetedPoint_WS );
						}
					}
				} else {
					// Bot does not have a weapon, but might have a "built in" weapon, so calculate the target point
					ComputeHumanTargetPoint_WS( &m_TargetedPoint_WS, NULL );
				}
//				fdraw_DevSphere( &m_TargetedPoint_WS, 0.25 );
			PROTRACK_ENDBLOCK();	// FindTargetPt

			// Compute vector from real muzzle point to target point...
			//m_apWeapon[0]->ComputeMuzzlePoint_WS( &MuzzlePoint_WS );
			ComputeWeaponMuzzlePoint_WS( &MuzzlePoint_WS );
			m_TargetLockUnitVec_WS.Sub( m_TargetedPoint_WS, MuzzlePoint_WS );
			f32 fMag2 = m_TargetLockUnitVec_WS.MagSq();
			f32 fDot = m_TargetLockUnitVec_WS.x*m_MountUnitFrontXZ_WS.x + m_TargetLockUnitVec_WS.z*m_MountUnitFrontXZ_WS.z;
			if( fMag2 > 0.001f && fDot > 0.5f ) {
				m_TargetLockUnitVec_WS.Mul( fmath_InvSqrt(fMag2) );
			} else {
				//m_TargetLockUnitVec_WS = m_apWeapon[0]->MtxToWorld()->m_vFront;
				ComputeWeaponMuzzleVec_WS( &m_TargetLockUnitVec_WS );
			}

			// Compute approximate muzzle point...
			ComputeApproxMuzzlePoint_WS( &MuzzlePoint_WS );

			// Compute vector from approx muzzle point to desired target...
			MuzzleToTarget_WS.Sub( m_TargetedPoint_WS, MuzzlePoint_WS );

			fDesiredPitch_WS = -fmath_Atan( MuzzleToTarget_WS.y, MuzzleToTarget_WS.MagXZ() );

			// If target point is behind us, don't adjust yaw...
			fDot = MuzzleToTarget_WS.x*m_MountUnitFrontXZ_WS.x + MuzzleToTarget_WS.z*m_MountUnitFrontXZ_WS.z;
			if( fDot >= 0.1f ) {
				fDesiredYaw_WS = fmath_Atan( MuzzleToTarget_WS.x, MuzzleToTarget_WS.z );
			} else {
				fDesiredYaw_WS = m_fMountYaw_WS;
			}
//		} else {
			// No ammo, so no aiming...
//			fDesiredPitch_WS = m_fMountPitch_WS;
//			fDesiredYaw_WS = m_fMountYaw_WS;
//		}
	}

	fUnitMovement = m_fMountPitchClampedNormDelta + m_fMountYawClampedNormDelta + m_fClampedNormSpeedXZ_WS;
	FMATH_CLAMPMAX( fUnitMovement, 1.0f );
	fTimeStep = FMATH_FPOT( fUnitMovement, 0.25f*FLoop_fPreviousLoopSecs, FLoop_fPreviousLoopSecs );

	// Compute aim pitch...
	if( fmath_Abs( fDesiredPitch_WS - m_fAimPitch_WS ) > FMATH_DEG2RAD(0.1f) ) {
		if( fDesiredPitch_WS > m_fAimPitch_WS ) {
			m_fAimPitch_WS += 2.0f * fTimeStep;
			FMATH_CLAMPMAX( m_fAimPitch_WS, fDesiredPitch_WS );
		} else {
			m_fAimPitch_WS -= 2.0f * fTimeStep;
			FMATH_CLAMPMIN( m_fAimPitch_WS, fDesiredPitch_WS );
		}
	}

	// Compute aim delta yaw...
	f32 fCurrentYaw = m_fMountYaw_WS + m_fAimDeltaYaw_MS;
	if( fCurrentYaw > FMATH_PI ) {
		do {
			fCurrentYaw -= FMATH_2PI;
		} while( fCurrentYaw > FMATH_PI );
	} else if( fCurrentYaw < -FMATH_PI ) {
		do {
			fCurrentYaw += FMATH_2PI;
		} while( fCurrentYaw < -FMATH_PI );
	}

	f32 fDesiredMinusCurrentYaw = fDesiredYaw_WS - fCurrentYaw;
	f32 fAdjustedYaw_WS;

	if( fDesiredMinusCurrentYaw > 0.0f ) {
		if( fDesiredMinusCurrentYaw >= FMATH_PI ) {
			// Step is counter-clockwise...

			fDesiredYaw_WS -= FMATH_2PI;
			fAdjustedYaw_WS = fCurrentYaw;
			fAdjustedYaw_WS -= 2.0f * fTimeStep;
			FMATH_CLAMPMIN( fAdjustedYaw_WS, fDesiredYaw_WS );
			m_fAimDeltaYaw_MS += (fAdjustedYaw_WS - fCurrentYaw);
		} else {
			// Step is clockwise...

			fAdjustedYaw_WS = fCurrentYaw;
			fAdjustedYaw_WS += 2.0f * fTimeStep;
			FMATH_CLAMPMAX( fAdjustedYaw_WS, fDesiredYaw_WS );
			m_fAimDeltaYaw_MS += (fAdjustedYaw_WS - fCurrentYaw);
		}
	} else if( fDesiredMinusCurrentYaw < 0.0f ){
		if( fDesiredMinusCurrentYaw <= -FMATH_PI ) {
			// Step is clockwise...

			fDesiredYaw_WS += FMATH_2PI;
			fAdjustedYaw_WS = fCurrentYaw;
			fAdjustedYaw_WS += 2.0f * fTimeStep;
			FMATH_CLAMPMAX( fAdjustedYaw_WS, fDesiredYaw_WS );
			m_fAimDeltaYaw_MS += (fAdjustedYaw_WS - fCurrentYaw);
		} else {
			// Step is counter-clockwise...

			fAdjustedYaw_WS = fCurrentYaw;
			fAdjustedYaw_WS -= 2.0f * fTimeStep;
			FMATH_CLAMPMIN( fAdjustedYaw_WS, fDesiredYaw_WS );
			m_fAimDeltaYaw_MS += (fAdjustedYaw_WS - fCurrentYaw);
		}
	}
}


void CBot::_AdjustAimVectorToCompensateForCamLookatDeviation( const CFVec3A *pCamFrontUnitVec_WS, CFVec3A *pUnitAimVec_WS ) {
	f32 fMag2, fSin, fCos, fCamToAimYaw, fCamPitch, fAimPitch, fCamToAimPitch;

	if( m_nPossessionPlayerIndex < 0 ) {
		// Not a player bot...
		return;
	}

	// Compute the yaw from the camera front vector to the bot mount front vector...
	CFVec3A CamUnitFrontXZ_WS;
	CamUnitFrontXZ_WS.x = pCamFrontUnitVec_WS->x;
	CamUnitFrontXZ_WS.y = 0.0f;
	CamUnitFrontXZ_WS.z = pCamFrontUnitVec_WS->z;
	fMag2 = CamUnitFrontXZ_WS.MagSq();
	if( fMag2 < 0.00001f ) {
		return;
	}
	CamUnitFrontXZ_WS.Mul( fmath_InvSqrt( fMag2 ) );

	CFVec3A CamFrontCrossBotFrontXZ_WS;
	CamFrontCrossBotFrontXZ_WS.Cross( CamUnitFrontXZ_WS, m_MountUnitFrontXZ_WS );
	fSin = CamFrontCrossBotFrontXZ_WS.Mag();
	FMATH_CLAMP( fSin, -1.0f, 1.0f );
	fCos = fmath_Sqrt( 1.0f - fSin*fSin );

	fCamToAimYaw = fmath_Atan( fSin, fCos );

	// Compute the pitch from the camera front vector to the bot mount front vector...
	fCos = pCamFrontUnitVec_WS->y;
	FMATH_CLAMP( fCos, -1.0f, 1.0f );
	fSin = fmath_Sqrt( 1.0f - fCos*fCos );
	fCamPitch = fmath_Atan( fSin, fCos );

	fCos = m_MtxToWorld.m_vFront.y;
	FMATH_CLAMP( fCos, -1.0f, 1.0f );
	fSin = fmath_Sqrt( 1.0f - fCos*fCos );
	fAimPitch = fmath_Atan( fSin, fCos );

	fCamToAimPitch = fAimPitch - fCamPitch;

	// Rotate vector by the delta yaw...
	pUnitAimVec_WS->RotateY( fCamToAimYaw );

	// Rotate vector by the delta pitch...
	CFQuatA PitchQuat;
	CFVec3A PitchRotUnitAxis_WS;

	PitchRotUnitAxis_WS.UnitCrossYWithVec( *pUnitAimVec_WS );
	PitchQuat.BuildQuat( PitchRotUnitAxis_WS, fCamToAimPitch );
	PitchQuat.MulPoint( *pUnitAimVec_WS );
}


void CBot::HandleSleeping( void ) {
	if( m_nSleepState != BOTSLEEPSTATE_NONE) {
		FASSERT(CanSleep());		//shouldn't be in sleep state if this bot doesn't have these anims
		switch (m_nSleepState) {
			case BOTSLEEPSTATE_INIT:
				UpdateTime(ASI_DOZE_LOOP, 0.0f);
				UpdateTime(ASI_WAKE, 0.0f);
				SetControlValue( ASI_DOZE_LOOP, 1.0f);
				SetControlValue( ASI_WAKE, 0.0f );
				m_fUnitWake = 0.0f;
				m_fUnitNapJerk = 0.0f;
				m_nSleepState = BOTSLEEPSTATE_DOZE_LOOP;
				break;
			case BOTSLEEPSTATE_DOZE_LOOP:
				//nothing to do, except advance the doze anim
				// something will tell us to wake up by advancing the sleepstate via CBot::WakeUp() 
				DeltaTime( ASI_DOZE_LOOP, FLoop_fPreviousLoopSecs, FALSE );
				break;
			case BOTSLEEPSTATE_NAPJERK:
				{
					//blend In and Out the Nap Jerk anim (higher priority than doze)
					f32 fGetTime = GetTime(ASI_NAPJERK);
					if (fGetTime*GetOOTotalTime(ASI_NAPJERK) >= 0.75f)
					{
						m_fUnitNapJerk -= 4.1f*FLoop_fPreviousLoopSecs;
					}
					else if (fGetTime*GetOOTotalTime(ASI_NAPJERK) <= 0.25f)
					{
						m_fUnitNapJerk += 4.1f*FLoop_fPreviousLoopSecs;
					}

					if (!_bPlayedSoundThisJerk && fGetTime > 0.25f)
					{
						_bPlayedSoundThisJerk = TRUE;
						FSndFx_FxHandle_t JerkSound = fsndfx_GetFxHandle( "B_ShutUp" );
						if (JerkSound)
						{
							fsndfx_Play3D(
								JerkSound,
								&MtxToWorld()->m_vPos,
								35.0f,
								1.0f,
								1.0f);

						}
					}
					SetControlValue( ASI_NAPJERK, m_fUnitNapJerk );
					FMATH_CLAMP(m_fUnitNapJerk, 0.0f, 1.0f);

					//play the Nap Jerk anim
					if (DeltaTime( ASI_NAPJERK, FLoop_fPreviousLoopSecs, TRUE ))
					{
						m_nSleepState = BOTSLEEPSTATE_DOZE_LOOP;
						SetControlValue( ASI_NAPJERK, 0.0f );
					}
				}

				break;
			case BOTSLEEPSTATE_WAKE:
				//blend in the wak anim (higher priority than doze)
				m_fUnitWake += _WAKE_DELTA_PER_SEC*FLoop_fPreviousLoopSecs;
				FMATH_CLAMPMAX(m_fUnitWake, 1.0f);
				SetControlValue( ASI_WAKE, m_fUnitWake );

				//play the wake anim, e
				if (DeltaTime( ASI_WAKE, FLoop_fPreviousLoopSecs, TRUE )) {
					m_nSleepState = BOTSLEEPSTATE_NONE;
					SetControlValue( ASI_DOZE_LOOP, 0.0f);
					SetControlValue( ASI_WAKE, 0.0f );
					SetControlValue( ASI_NAPJERK, 0.0f );
				}

				break;
		}
	}
}

void CBot::UpdateAimToTargetSamePoint( BOOL bCamAimsDownMountZ ) {
	if( m_nPossessionPlayerIndex < 0 ) {
		return;
	}

	if( bCamAimsDownMountZ ) {
		f32 fDot, fDistFromNewCamToTarget2;
		CFVec3A UnitVecFromNewCamToTarget_WS;
		const CFCamera *pCamera;
		const CFXfm *pCamXfm;
		const CFVec3A *pCamFront_WS;

		// Grab camera Xfm and front vector...
		pCamera = fcamera_GetCameraByIndex( m_nPossessionPlayerIndex );
		pCamXfm = pCamera->GetXfmWithoutShake();
		pCamFront_WS = &pCamXfm->m_MtxR.m_vFront;

		// Compute vector from camera position to targeted point...
		UnitVecFromNewCamToTarget_WS.Sub( m_TargetedPoint_WS, pCamXfm->m_MtxR.m_vPos );

		// Compute distance from camera position to targeted point...
		fDistFromNewCamToTarget2 = UnitVecFromNewCamToTarget_WS.MagSq();
		if( fDistFromNewCamToTarget2 < 0.01f ) {
			// Target is too close to new camera position...
			return;
		}

		// Unitize the vector...
		UnitVecFromNewCamToTarget_WS.Mul( fmath_InvSqrt( fDistFromNewCamToTarget2 ) );

		// Make sure the vector isn't pointing in the opposite XZ direction...
		fDot = UnitVecFromNewCamToTarget_WS.Dot( m_MountUnitFrontXZ_WS );
		if( fDot <= 0.01f ) {
			// Vector is pointing in the opposite XZ direction...
			return;
		}

		SetNewMountAimVec( &UnitVecFromNewCamToTarget_WS );

		return;
	}

	const CReticle *pReticle;
	const FViewport_t *pViewport;
	const CFXfm *pCamXfm;
	CFCameraMan *pCamMan;
	f32 fReticleX_SS, fReticleY_SS, fPointDepthZ_VS, fOrigYaw, fOrigPitch, fYaw, fPitch, fDeltaYaw, fDeltaPitch, fDistFromTargetToReticle2_SS;
	f32 fReticleMinusTargetX_SS, fReticleMinusTargetY_SS, fPrevReticleMinusTargetX_SS, fPrevReticleMinusTargetY_SS;
	u32 nTryCount;
	CFVec3A TargetedPoint_SS;
	CFMtx43A OrigMtxToWorld, FinalMtxToWorld;

	#define __MAX_TRIES								100
	#define __TARGET_TO_RETICLE_DESIRED_DIST_SS		0.001f

	// Save original mount aim values...
	OrigMtxToWorld = m_MtxToWorld;
	fOrigYaw = m_fMountYaw_WS;
	fOrigPitch = m_fMountPitch_WS;

	// Get current camera, viewport, and camera xfm...
	pCamMan = gamecam_GetCameraManByIndex( (GameCamPlayer_e)m_nPossessionPlayerIndex );
	pViewport = gamecam_GetActiveCamera()->GetViewport();
	pCamXfm = fcamera_GetCameraByIndex( m_nPossessionPlayerIndex )->GetXfmWithoutShake();

	// Get reticle screen point...
	pReticle = &Player_aPlayer[ m_nPossessionPlayerIndex ].m_Reticle;
	fReticleX_SS = pReticle->GetNormOriginX();
	fReticleY_SS = pReticle->GetNormOriginY();

	// Get original yaw and pitch...
	fYaw = m_fMountYaw_WS;
	fPitch = m_fMountPitch_WS;

	// Compute target point's screen space...
	fPointDepthZ_VS = fviewport_ComputeUnitOrtho3DScreenPoint_WS( pViewport, &pCamXfm->m_MtxF, &m_TargetedPoint_WS, &TargetedPoint_SS );
	if( fPointDepthZ_VS <= 0.1f ) {
		// Point is behind camera...
		return;
	}

	// Determine distance between the reticle and the targeted point in screen space...
	fPrevReticleMinusTargetX_SS = fReticleX_SS - TargetedPoint_SS.x;
	fPrevReticleMinusTargetY_SS = fReticleY_SS - TargetedPoint_SS.y;

	// Set original delta yaw and pitch...
	fDeltaYaw = (FMATH_2PI / 100.0f);
	fDeltaPitch = (FMATH_2PI / 100.0f);

	if( fPrevReticleMinusTargetX_SS > 0.0f ) {
		fDeltaYaw = -fDeltaYaw;
	}
	if( fPrevReticleMinusTargetY_SS < 0.0f ) {
		fDeltaPitch = -fDeltaPitch;
	}

	for( nTryCount=0; nTryCount<__MAX_TRIES; ++nTryCount ) {
		// See if the reticle is close enough to the targeted point...
		fDistFromTargetToReticle2_SS = fPrevReticleMinusTargetX_SS*fPrevReticleMinusTargetX_SS + fPrevReticleMinusTargetY_SS*fPrevReticleMinusTargetY_SS;
		if( fDistFromTargetToReticle2_SS <= (__TARGET_TO_RETICLE_DESIRED_DIST_SS * __TARGET_TO_RETICLE_DESIRED_DIST_SS) ) {
			// We're close enough...

			// Save solution...
			FinalMtxToWorld = m_MtxToWorld;

			// Restore original mount aim values...
			m_MtxToWorld = OrigMtxToWorld;
			ChangeMountYaw( fOrigYaw );

			// Now, officially relocate entity to our solution...
			Relocate_RotXlatFromUnitMtx_WS( &FinalMtxToWorld, FALSE );

//			DEVPRINTF( "Solution found in %u iterations.\n", nTryCount );

			return;
		}

		// Step our yaw and pitch...
		fYaw += fDeltaYaw;
		fPitch += fDeltaPitch;

		// Make sure the pitch isn't outside the bot's capable range...
		if( fPitch > m_pBotInfo_MountAim->fMountPitchDownLimit ) {
			if( fDeltaPitch > 0.0f ) {
				fDeltaPitch = m_pBotInfo_MountAim->fMountPitchDownLimit - fPitch;
			}

			fPitch = m_pBotInfo_MountAim->fMountPitchDownLimit;
		}
		if( fPitch < m_pBotInfo_MountAim->fMountPitchUpLimit ) {
			if( fDeltaPitch < 0.0f ) {
				fDeltaPitch = m_pBotInfo_MountAim->fMountPitchUpLimit - fPitch;
			}

			fPitch = m_pBotInfo_MountAim->fMountPitchUpLimit;
		}

		// Compute bot aim data for camera to use...
		m_MtxToWorld.m_vFront = CFVec3A::m_UnitAxisZ;
		m_MtxToWorld.m_vFront.RotateX( fPitch );
		m_MtxToWorld.m_vFront.RotateY( fYaw );
		m_MtxToWorld.m_vRight.UnitCrossYWithVec( m_MtxToWorld.m_vFront );
		m_MtxToWorld.m_vUp.Cross( m_MtxToWorld.m_vFront, m_MtxToWorld.m_vRight );

		m_fMountPitch_WS = fPitch;
		ChangeMountYaw( fYaw );

		// Update camera...
		pCamMan->Work();

		// Compute target point's in screen space...
		fPointDepthZ_VS = fviewport_ComputeUnitOrtho3DScreenPoint_WS( pViewport, &pCamXfm->m_MtxF, &m_TargetedPoint_WS, &TargetedPoint_SS );
		if( fPointDepthZ_VS <= 0.1f ) {
			// Point is behind camera...

			fYaw -= fDeltaYaw;
			fPitch -= fDeltaPitch;

			fDeltaYaw *= 0.5f;
			fDeltaPitch *= 0.5f;

			continue;
		}

		// Determine distance between the reticle and the targeted point in screen space...
		fReticleMinusTargetX_SS = fReticleX_SS - TargetedPoint_SS.x;
		fReticleMinusTargetY_SS = fReticleY_SS - TargetedPoint_SS.y;

		if( fReticleMinusTargetX_SS * fPrevReticleMinusTargetX_SS < 0.0f ) {
			// Yaw movement passed up the reticle...

			fDeltaYaw *= -0.5f;
		}

		if( fReticleMinusTargetY_SS * fPrevReticleMinusTargetY_SS < 0.0f ) {
			// Pitch movement passed up the reticle...

			fDeltaPitch *= -0.5f;
		}

		// Remember target point relative to reticle for next iteration...
		fPrevReticleMinusTargetX_SS = fReticleMinusTargetX_SS;
		fPrevReticleMinusTargetY_SS = fReticleMinusTargetY_SS;
	}

	// A solution could not be found...
	m_MtxToWorld = OrigMtxToWorld;
	m_fMountPitch_WS = fOrigPitch;
	ChangeMountYaw( fOrigYaw );


	#undef __TARGET_TO_RETICLE_DESIRED_DIST_SS
	#undef __MAX_TRIES
}


void CBot::ShakeCamera( f32 fUnitIntensity, f32 fDurationInSecs, BOOL bControllerRumbleOnly ) {
	FASSERT( IsCreated() );

	if( fDurationInSecs <= 0.0f ) {
		return;
	}

	if( m_nPossessionPlayerIndex >= 0 ) {
		// Player has control of this bot. Make his camera rock and roll...

		if( !bControllerRumbleOnly ) {
			CFCamera *pCamera = fcamera_GetCameraByIndex( m_nPossessionPlayerIndex );
			pCamera->ShakeCamera( fUnitIntensity, fDurationInSecs );
		}

		#if FANG_PLATFORM_XB
			f32 fRumbleUnitIntensity;

			fRumbleUnitIntensity = fUnitIntensity * 4.0f;
			FMATH_CLAMPMAX( fRumbleUnitIntensity, 0.8f );

			fxbforce_DecayCoarse( Player_aPlayer[m_nPossessionPlayerIndex].m_nControllerIndex, (fDurationInSecs + 0.3f), fRumbleUnitIntensity );
			fxbforce_DecayFine( Player_aPlayer[m_nPossessionPlayerIndex].m_nControllerIndex, (fDurationInSecs + 0.3f), fRumbleUnitIntensity );
		#endif
	}
}


void CBot::HandleWeaponReloadingAnimations( void ) {
	if( m_apWeapon[0]->m_pInfo->nInfoFlags & CWeapon::INFOFLAG_NO_AMMO ) {
		// Weapon has no notion of ammo...

		m_abReloadRequest[0] = FALSE;

		return;
	}

	switch( m_apWeapon[0]->m_pInfo->nReloadType ) {
	case CWeapon::RELOAD_TYPE_FULLYAUTOMATIC:
		HandleWeaponReloadingAnimations_FullyAutomatic();
		break;

	case CWeapon::RELOAD_TYPE_SHELL:
		HandleWeaponReloadingAnimations_Shell();
		break;

	case CWeapon::RELOAD_TYPE_CLIP:
		HandleWeaponReloadingAnimations_Clip();
		break;

	case CWeapon::RELOAD_TYPE_ROCKET1:
		HandleWeaponReloadingAnimations_Rocket1();
		break;

	case CWeapon::RELOAD_TYPE_TETHER:
		HandleWeaponReloadingAnimations_Tether();
		break;

	}

	// Absorb reload requests under certain conditions...
	if( !m_apWeapon[0]->CanClipBeReloaded() || (m_nJumpState == BOTJUMPSTATE_CABLE) || ( (m_nWRState != WR_STATE_NONE) && (m_nWRState != WR_STATE_ABORT) ) ) {
		m_abReloadRequest[0] = FALSE;
	}
	if( (m_apWeapon[0]->m_pInfo->nInfoFlags & CWeapon::INFOFLAG_LEFT_HAND_RELOADS) && (m_nJumpState == BOTJUMPSTATE_CABLE) ) {
		m_abReloadRequest[0] = FALSE;
	}
	if( !IsReverseFacingForward() ) {
		m_abReloadRequest[0] = FALSE;
	}

	// Handle weapon state changing between "deployed" and "reload opened"...
	if( (m_nWRState == WR_STATE_NONE) && m_apWeapon[0]->GetClipAmmo() ) {
		// We're not reloading and we have ammo...

		if( m_apWeapon[0]->CurrentState() == CWeapon::STATE_RELOAD_OPENED ) {
			m_apWeapon[0]->SetDesiredState( CWeapon::STATE_DEPLOYED );
		}
	} else {
		// We're either reloading or we have no ammo...

		if( m_apWeapon[0]->CurrentState() == CWeapon::STATE_DEPLOYED ) {
			if( !(m_nBotFlags & BOTFLAG_PLAY_FIRE1_ANIM) ) {
				m_apWeapon[0]->SetDesiredState( CWeapon::STATE_RELOAD_OPENED );
			}
		}
	}
}


void CBot::HandleWeaponReloadingAnimations_FullyAutomatic( void ) {
	switch( m_nWRState ) {
	case WR_STATE_NONE:
		if( m_nWSState == WS_STATE_NONE ) {
			if( !(m_nBotFlags & BOTFLAG_PLAY_FIRE1_ANIM) ) {
				// Not firing primary...

				if( IsReverseFacingForward() ) {
					// reverse facing mode is not engaged

					if( !(m_apWeapon[0]->m_pInfo->nInfoFlags & CWeapon::INFOFLAG_LEFT_HAND_RELOADS) || (m_nJumpState != BOTJUMPSTATE_CABLE) ) {
						// Either we're not on a cable, or we are and this weapon allows us to reload while on the cable...

						if( m_apWeapon[0]->IsClipEmptyAndCanItBeReloaded() || (m_abReloadRequest[0] && m_apWeapon[0]->CanClipBeReloaded()) ) {
							m_nWRState = WR_STATE_FULLYAUTO_RELOAD_OPENING;
						}
					}
				}
			}
		}

		break;

	case WR_STATE_FULLYAUTO_RELOAD_OPENING:
		if( m_apWeapon[0]->CurrentState() == CWeapon::STATE_RELOAD_OPENED ) {
			// Weapon is ready to be reloaded. Do it...

			m_apWeapon[0]->Reload();
			m_nWRState = WR_STATE_FULLYAUTO_RELOADING;
		}

		break;

	case WR_STATE_FULLYAUTO_RELOADING:
		if( m_apWeapon[0]->CurrentState() == CWeapon::STATE_RELOAD_OPENED ) {
			// Weapon is done reloading and is ready for another one...

			if( !m_apWeapon[0]->CanClipBeReloaded() || (m_fControls_Fire1 > 0.0f ) ) {
				// Don't start another reload...
				m_nWRState = WR_STATE_NONE;
			} else {
				// Start another reload...
				m_apWeapon[0]->Reload();
			}
		}

		break;

	case WR_STATE_ABORT:
		if( m_apWeapon[0]->CurrentState() == CWeapon::STATE_RELOAD_OPENED ) {
			m_nWRState = WR_STATE_NONE;
		}

		break;

	default:
		FASSERT_NOW;
	}
}


void CBot::HandleWeaponReloadingAnimations_Rocket1( void ) {
	f32 fUnitTime;

	switch( m_nWRState ) {
	case WR_STATE_NONE:
		if( m_nWSState == WS_STATE_NONE ) {
			if( !(m_apWeapon[0]->m_pInfo->nInfoFlags & CWeapon::INFOFLAG_LEFT_HAND_RELOADS) || (m_nJumpState != BOTJUMPSTATE_CABLE) ) {
				// Either we're not on a cable, or we are and this weapon allows us to reload while on the cable...

				if( IsReverseFacingForward() ) {
					// reverse facing mode is not engaged

					if( m_apWeapon[0]->IsClipEmptyAndCanItBeReloaded() || (m_abReloadRequest[0] && m_apWeapon[0]->CanClipBeReloaded()) ) {
						StartedUsingLeftArmToReload();

						SetHeadLookInterrupt();
						m_nWRState = WR_STATE_ROCKET1_GRAB_AMMO;
						SetControlValue( ASI_RELOAD_WITH_LEFT_GRAB_AMMO, 0.0f );
						ZeroTime( ASI_RELOAD_WITH_LEFT_GRAB_AMMO );
					}
				}
			}
		}

		break;

	case WR_STATE_ROCKET1_GRAB_AMMO:
		if( !DeltaTime( ASI_RELOAD_WITH_LEFT_GRAB_AMMO, FLoop_fPreviousLoopSecs * m_pBotInfo_Weapon->fRocket1ReloadAnimSpeedMult, TRUE ) ) {
			// We're in the process of grabbing ammo...

			fUnitTime = fmath_UnitLinearToSCurve( GetUnitTime( ASI_RELOAD_WITH_LEFT_GRAB_AMMO ) );
			SetControlValue( ASI_RELOAD_WITH_LEFT_GRAB_AMMO, fUnitTime );
		} else {
			// We're done grabbing...

			// Wait for weapon to be ready for reloading...
			if( m_apWeapon[0]->CurrentState() == CWeapon::STATE_RELOAD_OPENED ) {
				// Weapon is ready for reloading...

				m_nWRState = WR_STATE_ROCKET1_DROP_IN;
				SetControlValue( ASI_RELOAD_WITH_LEFT_GRAB_AMMO, 0.0f );
				SetControlValue( ASI_RELOAD_WITH_LEFT_DROP_IN, 1.0f );
				ZeroTime( ASI_RELOAD_WITH_LEFT_DROP_IN );
			} else if( (m_fControls_Fire1 > 0.0f) && !m_apWeapon[0]->IsClipEmpty() ) {
				// Reload abort requested...

				m_nWRAbortASI = ASI_RELOAD_WITH_LEFT_GRAB_AMMO;
				m_fWRAbortUnitControl = 1.0f;
				m_nWRState = WR_STATE_ABORT;
			}
		}

		break;

	case WR_STATE_ROCKET1_DROP_IN:
		if( DeltaTime( ASI_RELOAD_WITH_LEFT_DROP_IN, FLoop_fPreviousLoopSecs * m_pBotInfo_Weapon->fRocket1ReloadAnimSpeedMult, TRUE ) ) {
			// We're done dropping the ammo in...

			m_apWeapon[0]->Reload();

			if( m_fControls_Fire1 == 0.0f ) {
				if( m_apWeapon[0]->CanClipBeReloaded() ) {
					// Clip has room for more. Reload another...

					m_nWRState = WR_STATE_ROCKET1_GRAB_ANOTHER;
				} else {
					// Clip cannot be reloaded...

					m_nWRState = WR_STATE_ROCKET1_FINISH;
					SetControlValue( ASI_RELOAD_WITH_LEFT_DROP_IN, 0.0f );
					SetControlValue( ASI_RELOAD_WITH_LEFT_FINISH, 1.0f );
					ZeroTime( ASI_RELOAD_WITH_LEFT_FINISH );
				}
			} else {
				// Abort reload...
				m_nWRAbortASI = ASI_RELOAD_WITH_LEFT_DROP_IN;
				m_fWRAbortUnitControl = 1.0f;
				m_nWRState = WR_STATE_ABORT;
			}
		}

		break;

	case WR_STATE_ROCKET1_GRAB_ANOTHER:
		if( DeltaTime( ASI_RELOAD_WITH_LEFT_DROP_IN, -FLoop_fPreviousLoopSecs * m_pBotInfo_Weapon->fRocket1ReloadAnimSpeedMult, TRUE ) ) {
			// We're done grabbing another...

			if( m_fControls_Fire1 == 0.0f ) {
				if( m_apWeapon[0]->CurrentState() == CWeapon::STATE_RELOAD_OPENED ) {
					// Weapon is ready for another round...
					m_nWRState = WR_STATE_ROCKET1_DROP_IN;
				}
			} else {
				// Abort reload...
				m_nWRAbortASI = ASI_RELOAD_WITH_LEFT_DROP_IN;
				m_fWRAbortUnitControl = 1.0f;
				m_nWRState = WR_STATE_ABORT;
			}
		}

		break;

	case WR_STATE_ROCKET1_FINISH:
		if( !DeltaTime( ASI_RELOAD_WITH_LEFT_FINISH, FLoop_fPreviousLoopSecs * m_pBotInfo_Weapon->fRocket1ReloadAnimSpeedMult, TRUE ) ) {
			// We're in the process of returning to the finish position...

			fUnitTime = 1.01f - fmath_UnitLinearToSCurve( GetUnitTime( ASI_RELOAD_WITH_LEFT_FINISH ) );
			SetControlValue( ASI_RELOAD_WITH_LEFT_FINISH, fUnitTime );
		} else {
			// We're done returning to the finish position...

			ClearHeadLookInterrupt();
			m_nWRState = WR_STATE_NONE;
			SetControlValue( ASI_RELOAD_WITH_LEFT_FINISH, 0.0f );
		}

		break;

	case WR_STATE_ABORT:
		m_fWRAbortUnitControl -= 4.0f * FLoop_fPreviousLoopSecs;
		FMATH_CLAMPMIN( m_fWRAbortUnitControl, 0.0f );
		SetControlValue( m_nWRAbortASI, fmath_UnitLinearToSCurve( m_fWRAbortUnitControl ) );

		if( m_fWRAbortUnitControl == 0.0f ) {
			// Abortion done...

			SetHeadLookInterrupt();
			m_nWRState = WR_STATE_NONE;
		}

		break;

	default:
		FASSERT_NOW;
	}
}


void CBot::HandleWeaponReloadingAnimations_Tether( void ) {
	f32 fUnitTime;

	FASSERT( m_apWeapon[0]->Type() == CWeapon::WEAPON_TYPE_TETHER );

	switch( m_nWRState ) {
	case WR_STATE_NONE:
		if( m_nWSState == WS_STATE_NONE ) {
			CWeaponTether *pTether = (CWeaponTether *)m_apWeapon[0];

			if( pTether->IsTetherDestroyed() || pTether->IsTetherDetachedFromSource() ) {
				// Tether is destroyed, so we can reload...

				if( m_nJumpState != BOTJUMPSTATE_CABLE ) {
					// We're not on a cable...

					if( !(m_nBotFlags & BOTFLAG_PLAY_FIRE1_ANIM) ) {
						// We're not playing the fire1 animation...

						if( IsReverseFacingForward() ) {
						// reverse facing mode is not engaged

							if( m_apWeapon[0]->IsClipEmptyAndCanItBeReloaded() ||
								(m_abReloadRequest[0] && m_apWeapon[0]->CanClipBeReloaded()) ) {
								SetHeadLookInterrupt();

								if( pTether->GetNumTetherPumpsNeeded() ) {
									// the tether need pumps to reload
									m_nWRState = WR_STATE_TETHER_PREPARE_TO_PUMP;
									SetControlValue( ASI_RELOAD_TETHER_PREPARE_TO_PUMP, 0.0f );
									ZeroTime( ASI_RELOAD_TETHER_PREPARE_TO_PUMP );
								} else {
									// the tether doesn't need pumped to reload, skip to the grabbing of the ammo state
									m_nWRState = WR_STATE_TETHER_GRAB_AMMO;
									SetControlValue( ASI_RELOAD_TETHER_GRAB_AMMO, 0.0f );
									ZeroTime( ASI_RELOAD_TETHER_GRAB_AMMO );									
								}
							}
						}
					}
				}
			}
		}

		break;

	case WR_STATE_TETHER_PREPARE_TO_PUMP:
		if( !DeltaTime( ASI_RELOAD_TETHER_PREPARE_TO_PUMP, FLoop_fPreviousLoopSecs * m_pBotInfo_Weapon->fTetherReloadAnimSpeedMult, TRUE ) ) {
			// We're preparing to pump:

			fUnitTime = fmath_UnitLinearToSCurve( GetUnitTime( ASI_RELOAD_TETHER_PREPARE_TO_PUMP ) );
			SetControlValue( ASI_RELOAD_TETHER_PREPARE_TO_PUMP, fUnitTime );
		} else {
			// Preparing-to-pump anim is done playing...

			// Wait for weapon to be ready for reloading...
			if( m_apWeapon[0]->CurrentState() == CWeapon::STATE_RELOAD_OPENED ) {
				// Weapon is ready for pumping...

				CWeaponTether *pTether = (CWeaponTether *)m_apWeapon[0];

				if( !pTether->Pump( m_pBotInfo_Weapon->fTetherPumpAnimSpeedMult ) ) {
					// We need to pump...

					m_nWRState = WR_STATE_TETHER_PUMP;
					SetControlValue( ASI_RELOAD_TETHER_PREPARE_TO_PUMP, 0.0f );
					SetControlValue( ASI_RELOAD_TETHER_PUMP, 1.0f );
					ZeroTime( ASI_RELOAD_TETHER_PUMP );
				} else {
					// No need to pump...

					m_nWRState = WR_STATE_TETHER_GRAB_AMMO;
					SetControlValue( ASI_RELOAD_TETHER_PREPARE_TO_PUMP, 0.0f );
					SetControlValue( ASI_RELOAD_TETHER_GRAB_AMMO, 1.0f );
					ZeroTime( ASI_RELOAD_TETHER_GRAB_AMMO );
				}
			}
		}

		break;

	case WR_STATE_TETHER_PUMP:
		if( DeltaTime( ASI_RELOAD_TETHER_PUMP, FLoop_fPreviousLoopSecs * m_pBotInfo_Weapon->fTetherPumpAnimSpeedMult, TRUE ) ) {
			// We're done with one pump...

			CWeaponTether *pTether = (CWeaponTether *)m_apWeapon[0];

			if( !pTether->Pump( m_pBotInfo_Weapon->fTetherPumpAnimSpeedMult ) ) {
				// We need to pump again...
				ZeroTime( ASI_RELOAD_TETHER_PUMP );
			} else {
				// No more pumping needed...

				m_nWRState = WR_STATE_TETHER_GRAB_AMMO;
				SetControlValue( ASI_RELOAD_TETHER_PUMP, 0.0f );
				SetControlValue( ASI_RELOAD_TETHER_GRAB_AMMO, 1.0f );
				ZeroTime( ASI_RELOAD_TETHER_GRAB_AMMO );
			}
		}

		break;

	case WR_STATE_TETHER_GRAB_AMMO:
		if( DeltaTime( ASI_RELOAD_TETHER_GRAB_AMMO, FLoop_fPreviousLoopSecs * m_pBotInfo_Weapon->fTetherReloadAnimSpeedMult, TRUE ) ) {
			// We're done grabbing...

			m_apWeapon[0]->Trigger_AttachRoundToOwnerBotBone( m_pBotInfo_Weapon->pszSecondaryAttachBoneName );

			m_nWRState = WR_STATE_TETHER_DROP_IN;
			SetControlValue( ASI_RELOAD_TETHER_GRAB_AMMO, 0.0f );
			SetControlValue( ASI_RELOAD_TETHER_DROP_IN, 1.0f );
			ZeroTime( ASI_RELOAD_TETHER_DROP_IN );
		}

		break;

	case WR_STATE_TETHER_DROP_IN:
		if( DeltaTime( ASI_RELOAD_TETHER_DROP_IN, FLoop_fPreviousLoopSecs * m_pBotInfo_Weapon->fTetherReloadAnimSpeedMult, TRUE ) ) {
			// We're done dropping the ammo in...

			m_apWeapon[0]->Reload();

			m_nWRState = WR_STATE_TETHER_FINISH;
			SetControlValue( ASI_RELOAD_TETHER_DROP_IN, 0.0f );
			SetControlValue( ASI_RELOAD_TETHER_FINISH, 1.0f );
			ZeroTime( ASI_RELOAD_TETHER_FINISH );
		}

		break;

	case WR_STATE_TETHER_FINISH:
		if( !DeltaTime( ASI_RELOAD_TETHER_FINISH, FLoop_fPreviousLoopSecs * m_pBotInfo_Weapon->fTetherReloadAnimSpeedMult, TRUE ) ) {
			// We're in the process of returning to the finish position...

			fUnitTime = 1.01f - fmath_UnitLinearToSCurve( GetUnitTime( ASI_RELOAD_TETHER_FINISH ) );
			SetControlValue( ASI_RELOAD_TETHER_FINISH, fUnitTime );
		} else {
			// We're done returning to the finish position...

			ClearHeadLookInterrupt();
			m_nWRState = WR_STATE_NONE;
			SetControlValue( ASI_RELOAD_TETHER_FINISH, 0.0f );
		}

		break;

	case WR_STATE_ABORT:
		m_fWRAbortUnitControl -= 4.0f * FLoop_fPreviousLoopSecs;
		FMATH_CLAMPMIN( m_fWRAbortUnitControl, 0.0f );
		SetControlValue( m_nWRAbortASI, fmath_UnitLinearToSCurve( m_fWRAbortUnitControl ) );

		if( m_fWRAbortUnitControl == 0.0f ) {
			// Abortion done...

			ClearHeadLookInterrupt();
			m_nWRState = WR_STATE_NONE;
		}

		break;

	default:
		FASSERT_NOW;
	}
}


void CBot::HandleWeaponReloadingAnimations_Shell( void ) {
	switch( m_nWRState ) {
	case WR_STATE_NONE:
		if( m_nWSState == WS_STATE_NONE ) {
			if( !(m_nBotFlags & BOTFLAG_PLAY_FIRE1_ANIM) ) {
				// Not firing primary...

				if( !(m_apWeapon[0]->m_pInfo->nInfoFlags & CWeapon::INFOFLAG_LEFT_HAND_RELOADS) || (m_nJumpState != BOTJUMPSTATE_CABLE) ) {
					// Either we're not on a cable, or we are and this weapon allows us to reload while on the cable...

					if( IsReverseFacingForward() ) {
					// reverse facing mode is not engaged

						if( m_apWeapon[0]->IsClipEmpty() || (m_abReloadRequest[0] && m_apWeapon[0]->CanClipBeReloaded()) ) {
							m_nWRState = WR_STATE_RAISING_WEAPON;

							SetControlValue( ASI_RELOAD_SINGLE_ARM, 0.0f );
							SetControlValue( ASI_RELOAD_SINGLE_BODY, 0.0f );
							ZeroTime( ASI_RELOAD_SINGLE_ARM );
							m_fUnitReloadSingleBlend = 0.0f;
						}
					}
				}
			}
		}

		break;

	case WR_STATE_RAISING_WEAPON:
		if( m_fUnitReloadSingleBlend < 1.0f ) {
			m_fUnitReloadSingleBlend += 4.0f * FLoop_fPreviousLoopSecs;
			FMATH_CLAMPMAX( m_fUnitReloadSingleBlend, 1.0f );
			SetControlValue( ASI_RELOAD_SINGLE_ARM, m_fUnitReloadSingleBlend * m_pBotInfo_Weapon->fMaxSingleReloadUnitBlend );
			SetControlValue( ASI_RELOAD_SINGLE_BODY, m_fUnitReloadSingleBlend * m_pBotInfo_Weapon->fMaxSingleReloadUnitBlend );
		} else {
			// We're done raising our weapon...

			// Wait for weapon to be ready for reloading...
			if( m_apWeapon[0]->CanClipBeReloaded() ) {
				if( m_apWeapon[0]->CurrentState() == CWeapon::STATE_RELOAD_OPENED ) {
					// Weapon is ready for reloading...

					SetHeadLookInterrupt();
					m_nWRState = WR_STATE_RELOADING;
				} else if( m_apWeapon[0]->IsClipEmpty() || (m_fControls_Fire1 == 0.0f) ) {
					// Reload abort requested...

					m_fWRAbortUnitControl =  GetControlValue( ASI_RELOAD_SINGLE_ARM );
					m_nWRState = WR_STATE_ABORT;
				}
			}

		}

		break;

	case WR_STATE_RELOADING:
		if( m_apWeapon[0]->CurrentState() == CWeapon::STATE_RELOAD_OPENED ) {
			if( m_apWeapon[0]->IsClipEmpty() || (m_fControls_Fire1 == 0.0f) ) {
				if( m_apWeapon[0]->CanClipBeReloaded() ) {
					m_apWeapon[0]->Reload();
				} else {
					// Clip cannot be reloaded...

					m_nWRState = WR_STATE_COCKING;
					m_fUnitReloadSingleBlend = 1.0f;

					SetControlValue( ASI_RELOAD_SINGLE_ARM, m_pBotInfo_Weapon->fMaxSingleReloadUnitBlend );
					SetControlValue( ASI_RELOAD_SINGLE_BODY, m_pBotInfo_Weapon->fMaxSingleReloadUnitBlend );
					ZeroTime( ASI_RELOAD_SINGLE_ARM );
				}
			} else {
				// Abort reload...
				m_fWRAbortUnitControl =  GetControlValue( ASI_RELOAD_SINGLE_ARM );
				m_nWRState = WR_STATE_ABORT;
			}
		}

		break;

	case WR_STATE_COCKING:
		if( !DeltaTime( ASI_RELOAD_SINGLE_ARM, 1.5f * FLoop_fPreviousLoopSecs, TRUE ) ) {
			// We're cocking...

			if( GetUnitTime( ASI_RELOAD_SINGLE_ARM ) > 0.75f ) {
				if( m_fUnitReloadSingleBlend > 0.0f ) {
					m_fUnitReloadSingleBlend -= 3.0f * FLoop_fPreviousLoopSecs;
					FMATH_CLAMPMIN( m_fUnitReloadSingleBlend, 0.0f );
					SetControlValue( ASI_RELOAD_SINGLE_ARM, m_fUnitReloadSingleBlend * m_pBotInfo_Weapon->fMaxSingleReloadUnitBlend );
					SetControlValue( ASI_RELOAD_SINGLE_BODY, m_fUnitReloadSingleBlend * m_pBotInfo_Weapon->fMaxSingleReloadUnitBlend );
				}
			}

			if( m_fControls_Fire1 > 0.0f ) {
				// Abort cocking...
				m_fWRAbortUnitControl = GetControlValue( ASI_RELOAD_SINGLE_ARM );
				m_nWRState = WR_STATE_ABORT;
			}
		} else {
			// We're done cocking...
			m_nWRState = WR_STATE_LOWERING_WEAPON;
		}

		break;

	case WR_STATE_LOWERING_WEAPON:
		m_fUnitReloadSingleBlend -= 3.0f * FLoop_fPreviousLoopSecs;
		if( m_fUnitReloadSingleBlend > 0.0f ) {
			SetControlValue( ASI_RELOAD_SINGLE_ARM, m_fUnitReloadSingleBlend * m_pBotInfo_Weapon->fMaxSingleReloadUnitBlend );
			SetControlValue( ASI_RELOAD_SINGLE_BODY, m_fUnitReloadSingleBlend * m_pBotInfo_Weapon->fMaxSingleReloadUnitBlend );
		} else {
			SetControlValue( ASI_RELOAD_SINGLE_ARM, 0.0f );
			SetControlValue( ASI_RELOAD_SINGLE_BODY, 0.0f );
			ClearHeadLookInterrupt();
			m_nWRState = WR_STATE_NONE;
		}

		break;

	case WR_STATE_ABORT:
		m_fWRAbortUnitControl -= 4.0f * FLoop_fPreviousLoopSecs;
		FMATH_CLAMPMIN( m_fWRAbortUnitControl, 0.0f );
		SetControlValue( ASI_RELOAD_SINGLE_ARM, fmath_UnitLinearToSCurve( m_fWRAbortUnitControl ) );
		SetControlValue( ASI_RELOAD_SINGLE_BODY, fmath_UnitLinearToSCurve( m_fWRAbortUnitControl ) );

		if( m_fWRAbortUnitControl == 0.0f ) {
			// Abortion done...

			ClearHeadLookInterrupt();
			m_nWRState = WR_STATE_NONE;
		}

		break;

	default:
		FASSERT_NOW;
	}
}


void CBot::HandleWeaponReloadingAnimations_Clip( void ) {
	f32 fUnitTime;

	switch( m_nWRState ) {
	case WR_STATE_NONE:
		if( m_nWSState == WS_STATE_NONE ) {
			if( !(m_nBotFlags & BOTFLAG_PLAY_FIRE1_ANIM) ) {
				// We're not playing the fire1 animation...

				if( !(m_apWeapon[0]->m_pInfo->nInfoFlags & CWeapon::INFOFLAG_LEFT_HAND_RELOADS) || (m_nJumpState != BOTJUMPSTATE_CABLE) ) {
					// Either we're not on a cable, or we are and this weapon allows us to reload while on the cable...

					if( m_nThrowState == THROWSTATE_NONE ) {
						// We're not throwing a secondary...

						if( IsReverseFacingForward() ) {
							// Reverse facing mode is not engaged...

							if( !m_apWeapon[0]->Clip_IsInstalled() ) {
								// The clip's not installed. Install it...

								StartedUsingLeftArmToReload();

								SetHeadLookInterrupt();
								m_nWRState = WR_STATE_CLIP_GRAB_NEW_BLEND_IN;
								SetControlValue( ASI_RELOAD_CLIP_GRAB_NEW, 0.0f );
								ZeroTime( ASI_RELOAD_CLIP_GRAB_NEW );
							} else {
								// The clip's installed...

								if( m_apWeapon[0]->IsClipEmptyAndCanItBeReloaded() || (m_abReloadRequest[0] && m_apWeapon[0]->CanClipBeReloaded()) ) {
									StartedUsingLeftArmToReload();

									SetHeadLookInterrupt();
									m_nWRState = WR_STATE_CLIP_EJECT_OLD;
									SetControlValue( ASI_RELOAD_CLIP_EJECT_OLD, 0.0f );
									ZeroTime( ASI_RELOAD_CLIP_EJECT_OLD );
								}
							}
						}
					}
				}
			}
		}

		break;

	case WR_STATE_CLIP_EJECT_OLD:
		if( !DeltaTime( ASI_RELOAD_CLIP_EJECT_OLD, FLoop_fPreviousLoopSecs * m_pBotInfo_Weapon->fClipReloadAnimSpeedMult, TRUE ) ) {
			// We're ejecting old clip:

			fUnitTime = fmath_UnitLinearToSCurve( GetUnitTime( ASI_RELOAD_CLIP_EJECT_OLD ) );
			SetControlValue( ASI_RELOAD_CLIP_EJECT_OLD, fUnitTime );
		} else {
			// Clip ejection anim is done playing...

			m_apWeapon[0]->Clip_Eject();

			m_nWRState = WR_STATE_CLIP_GRAB_NEW;
			SetControlValue( ASI_RELOAD_CLIP_EJECT_OLD, 0.0f );
			SetControlValue( ASI_RELOAD_CLIP_GRAB_NEW, 1.0f );
			ZeroTime( ASI_RELOAD_CLIP_GRAB_NEW );
		}

		break;

	case WR_STATE_CLIP_GRAB_NEW:
		if( DeltaTime( ASI_RELOAD_CLIP_GRAB_NEW, FLoop_fPreviousLoopSecs * m_pBotInfo_Weapon->fClipReloadAnimSpeedMult, TRUE ) ) {
			// Grab new anim is done playing...

			// Wait for weapon to be ready for reloading...
			if( m_apWeapon[0]->CurrentState() == CWeapon::STATE_RELOAD_OPENED ) {
				// Weapon is ready to be reloaded...

				m_apWeapon[0]->Clip_AttachToOwnerBotBone( m_pBotInfo_Weapon->pszSecondaryAttachBoneName );

				m_nWRState = WR_STATE_CLIP_INSERT_NEW;
				SetControlValue( ASI_RELOAD_CLIP_GRAB_NEW, 0.0f );
				SetControlValue( ASI_RELOAD_CLIP_INSERT_NEW, 1.0f );
				ZeroTime( ASI_RELOAD_CLIP_INSERT_NEW );
			}
		}

		break;

	case WR_STATE_CLIP_GRAB_NEW_BLEND_IN:
		if( !DeltaTime( ASI_RELOAD_CLIP_GRAB_NEW, FLoop_fPreviousLoopSecs * m_pBotInfo_Weapon->fClipReloadAnimSpeedMult, TRUE ) ) {
			// We're grabbing new clip:

			fUnitTime = fmath_UnitLinearToSCurve( GetUnitTime( ASI_RELOAD_CLIP_GRAB_NEW ) );
			SetControlValue( ASI_RELOAD_CLIP_GRAB_NEW, fUnitTime );
		} else {
			// Grab new anim is done playing...

			// Wait for weapon to be ready for reloading...
			if( m_apWeapon[0]->CurrentState() == CWeapon::STATE_RELOAD_OPENED ) {
				// Weapon is ready to be reloaded...

				m_apWeapon[0]->Clip_AttachToOwnerBotBone( m_pBotInfo_Weapon->pszSecondaryAttachBoneName );

				m_nWRState = WR_STATE_CLIP_INSERT_NEW;
				SetControlValue( ASI_RELOAD_CLIP_GRAB_NEW, 0.0f );
				SetControlValue( ASI_RELOAD_CLIP_INSERT_NEW, 1.0f );
				ZeroTime( ASI_RELOAD_CLIP_INSERT_NEW );
			}
		}

		break;

	case WR_STATE_CLIP_INSERT_NEW:
		if( DeltaTime( ASI_RELOAD_CLIP_INSERT_NEW, FLoop_fPreviousLoopSecs * m_pBotInfo_Weapon->fClipReloadAnimSpeedMult, TRUE ) ) {
			// We're done inserting new clip...

			m_apWeapon[0]->Clip_AttachToWeapon();

			FMATH_CLEARBITMASK( m_nBotFlags, BOTFLAG_CLIP_IS_SLAPPED_IN );

			m_nWRState = WR_STATE_CLIP_SLAPIN_NEW;
			SetControlValue( ASI_RELOAD_CLIP_INSERT_NEW, 0.0f );
			SetControlValue( ASI_RELOAD_CLIP_SLAPIN_NEW, 1.0f );
			ZeroTime( ASI_RELOAD_CLIP_SLAPIN_NEW );
		}

		break;

	case WR_STATE_CLIP_SLAPIN_NEW:
		if( !DeltaTime( ASI_RELOAD_CLIP_SLAPIN_NEW, FLoop_fPreviousLoopSecs * m_pBotInfo_Weapon->fClipReloadAnimSpeedMult, TRUE ) ) {
			// We're in the process of slapping in the new clip...

			fUnitTime = GetUnitTime( ASI_RELOAD_CLIP_SLAPIN_NEW );

			if( !(m_nBotFlags & BOTFLAG_CLIP_IS_SLAPPED_IN) ) {
				if( fUnitTime > 0.4f ) {
					m_apWeapon[0]->Clip_SlapIn();
					FMATH_SETBITMASK( m_nBotFlags, BOTFLAG_CLIP_IS_SLAPPED_IN );
				}
			}

			if( fUnitTime >= 0.5f ) {
				fUnitTime = 1.01f - fmath_UnitLinearToSCurve( (fUnitTime - 0.5f) * 2.0f );
				SetControlValue( ASI_RELOAD_CLIP_SLAPIN_NEW, fUnitTime );
			}
		} else {
			// We're done slapping in new clip...

			ClearHeadLookInterrupt();
			m_nWRState = WR_STATE_NONE;
			SetControlValue( ASI_RELOAD_CLIP_SLAPIN_NEW, 0.0f );
		}

		break;

	case WR_STATE_ABORT:
		m_fWRAbortUnitControl -= 4.0f * FLoop_fPreviousLoopSecs;
		FMATH_CLAMPMIN( m_fWRAbortUnitControl, 0.0f );
		SetControlValue( m_nWRAbortASI, fmath_UnitLinearToSCurve( m_fWRAbortUnitControl ) );

		if( m_fWRAbortUnitControl == 0.0f ) {
			// Abortion done...

			ClearHeadLookInterrupt();
			m_nWRState = WR_STATE_NONE;
		}

		break;

	default:
		FASSERT_NOW;
	}
}


void CBot::AbortReloadImmediately( void ) {
	switch( m_nWRState ) {
	case WR_STATE_NONE:
		// Not doing anything. Nothing to abort...
		break;

	case WR_STATE_FULLYAUTO_RELOAD_OPENING:
	case WR_STATE_FULLYAUTO_RELOADING:
		m_nWRState = WR_STATE_ABORT;
		break;

	case WR_STATE_ROCKET1_GRAB_AMMO:
		m_nWRAbortASI = ASI_RELOAD_WITH_LEFT_GRAB_AMMO;
		m_fWRAbortUnitControl = GetControlValue( m_nWRAbortASI );
		m_nWRState = WR_STATE_ABORT;

		break;

	case WR_STATE_ROCKET1_DROP_IN:
	case WR_STATE_ROCKET1_GRAB_ANOTHER:
		m_nWRAbortASI = ASI_RELOAD_WITH_LEFT_DROP_IN;
		m_fWRAbortUnitControl = GetControlValue( m_nWRAbortASI );
		m_nWRState = WR_STATE_ABORT;

		break;

	case WR_STATE_ROCKET1_FINISH:
		m_nWRAbortASI = ASI_RELOAD_WITH_LEFT_FINISH;
		m_fWRAbortUnitControl = GetControlValue( m_nWRAbortASI );
		m_nWRState = WR_STATE_ABORT;

		break;

	case WR_STATE_TETHER_PREPARE_TO_PUMP:
		m_apWeapon[0]->Trigger_RemoveRoundFromWorld();
		m_nWRAbortASI = ASI_RELOAD_TETHER_PREPARE_TO_PUMP;
		m_fWRAbortUnitControl = GetControlValue( m_nWRAbortASI );
		m_nWRState = WR_STATE_ABORT;

		break;

	case WR_STATE_TETHER_PUMP:
		m_apWeapon[0]->Trigger_RemoveRoundFromWorld();
		m_nWRAbortASI = ASI_RELOAD_TETHER_PUMP;
		m_fWRAbortUnitControl = GetControlValue( m_nWRAbortASI );
		m_nWRState = WR_STATE_ABORT;

		break;

	case WR_STATE_TETHER_GRAB_AMMO:
		m_apWeapon[0]->Trigger_RemoveRoundFromWorld();
		m_nWRAbortASI = ASI_RELOAD_TETHER_GRAB_AMMO;
		m_fWRAbortUnitControl = GetControlValue( m_nWRAbortASI );
		m_nWRState = WR_STATE_ABORT;

		break;

	case WR_STATE_TETHER_DROP_IN:
		m_apWeapon[0]->Trigger_RemoveRoundFromWorld();
		m_nWRAbortASI = ASI_RELOAD_TETHER_DROP_IN;
		m_fWRAbortUnitControl = GetControlValue( m_nWRAbortASI );
		m_nWRState = WR_STATE_ABORT;

		break;

	case WR_STATE_TETHER_FINISH:
		m_nWRAbortASI = ASI_RELOAD_TETHER_FINISH;
		m_fWRAbortUnitControl = GetControlValue( m_nWRAbortASI );
		m_nWRState = WR_STATE_ABORT;

		break;

	case WR_STATE_RAISING_WEAPON:
	case WR_STATE_RELOADING:
	case WR_STATE_COCKING:
	case WR_STATE_LOWERING_WEAPON:
		m_nWRAbortASI = ASI_RELOAD_SINGLE_ARM;
		m_fWRAbortUnitControl = GetControlValue( m_nWRAbortASI );
		m_nWRState = WR_STATE_ABORT;

		break;

	case WR_STATE_CLIP_EJECT_OLD:
		m_nWRAbortASI = ASI_RELOAD_CLIP_EJECT_OLD;
		m_fWRAbortUnitControl = GetControlValue( m_nWRAbortASI );
		m_nWRState = WR_STATE_ABORT;

		break;

	case WR_STATE_CLIP_GRAB_NEW:
	case WR_STATE_CLIP_GRAB_NEW_BLEND_IN:
		m_nWRAbortASI = ASI_RELOAD_CLIP_GRAB_NEW;
		m_fWRAbortUnitControl = GetControlValue( m_nWRAbortASI );
		m_nWRState = WR_STATE_ABORT;

		break;

	case WR_STATE_CLIP_INSERT_NEW:
		m_apWeapon[0]->Clip_DiscardAttachedToOwnerBotBone();
		m_nWRAbortASI = ASI_RELOAD_CLIP_INSERT_NEW;
		m_fWRAbortUnitControl = GetControlValue( m_nWRAbortASI );
		m_nWRState = WR_STATE_ABORT;

		break;

	case WR_STATE_CLIP_SLAPIN_NEW:
		m_nWRAbortASI = ASI_RELOAD_CLIP_SLAPIN_NEW;
		m_fWRAbortUnitControl = GetControlValue( m_nWRAbortASI );
		m_nWRState = WR_STATE_ABORT;

		break;

	case WR_STATE_ABORT:
		// Already aborting. Nothing more to do...
		break;

	default:
		FASSERT_NOW;
	}
}

// m_fPowerOffOnAnimSpeedMult
void CBot::Power( BOOL bPowerUp, f32 fPowerOffTime, f32 fPowerOffOnSpeedMult ) {
	FASSERT( IsCreated() );

	if( bPowerUp ) {
		// Caller wishes to power-up the bot...

		if( (m_nPowerState == POWERSTATE_WAITING_FOR_IMMOBILIZED) || (m_nPowerState == POWERSTATE_POWERED_DOWN) ) {
			// Start power-up immediately...

			m_nPowerState = POWERSTATE_POWERING_UP;
			m_bOppositePowerStatePending = FALSE;

			if( m_anAnimStackIndex[ASI_RC_POWER_UP] != -1 ) {
				ZeroTime( ASI_RC_POWER_UP );
				SetControlValue( ASI_RC_POWER_UP, 0.0f );
			}
			m_fPowerAnimUnitBlend = 0.0f;
			m_fPowerOffOnAnimSpeedMult = fPowerOffOnSpeedMult;

		} else if( m_nPowerState == POWERSTATE_POWERING_DOWN ) {
			// Bot is in the process of powering down, so we need to queue the power-up request...
			m_bOppositePowerStatePending = TRUE;
			m_fPendingPowerOffOnAnimSpeedMult = fPowerOffOnSpeedMult;

		} else {
			// Bot is already powered up...
			m_bOppositePowerStatePending = FALSE;
		}
	} else {
		// Caller wishes to power-down the bot...

		if( m_nPowerState == POWERSTATE_POWERED_UP ) {
			// Start power-down immediately...

			m_nPowerState = POWERSTATE_WAITING_FOR_IMMOBILIZED;
			m_bOppositePowerStatePending = FALSE;
			m_fPowerOffOnTime = fPowerOffTime;
			m_fPowerOffOnAnimSpeedMult = fPowerOffOnSpeedMult;

			ImmobilizeBot();

		} else if( (m_nPowerState == POWERSTATE_POWERING_UP) || (m_nPowerState == POWERSTATE_FINISHING_UP) ) {
			// Bot is in the process of powering up, so we need to queue the power-down request...
			m_bOppositePowerStatePending = TRUE;
			m_fPendingOffOnTime = fPowerOffTime;
			m_fPendingPowerOffOnAnimSpeedMult = fPowerOffOnSpeedMult;

		} else {
			// Bot is already powered down...
			m_bOppositePowerStatePending = FALSE;
			// if this request wants a longer wait period, then update our timer value
			if( fPowerOffTime <= 0.0f ) {
				// wait forever
				m_fPowerOffOnTime = 0.0f;
			} else {
				// wait for the longer of the current wait time or the new wait time
				m_fPowerOffOnTime = FMATH_MAX( m_fPowerOffOnTime, fPowerOffTime );			
			}
		}
	}
}


void CBot::Power_SetState( BOOL bPowerUp, f32 fPowerOffTime/*=0.0f*/ ) {

	if( bPowerUp ) {
        m_fPowerOffOnTime				= 0.0f;
		m_fPowerAnimUnitBlend			= 0.0f;
		m_bOppositePowerStatePending	= FALSE;
		m_nPowerState					= POWERSTATE_POWERED_UP;
		if( m_anAnimStackIndex[ASI_RC_POWER_DOWN] != -1) {
			SetControlValue( ASI_RC_POWER_DOWN, 0.0f );
		}
		if( m_anAnimStackIndex[ASI_RC_POWER_UP] != -1) {
			SetControlValue( ASI_RC_POWER_UP, 0.0f );
		}

	} else {
		m_fPowerOffOnTime				= fPowerOffTime;
		m_fPowerAnimUnitBlend			= 1.0f;
		m_bOppositePowerStatePending	= FALSE;
		m_nPowerState					= POWERSTATE_POWERED_DOWN;

		if( m_anAnimStackIndex[ASI_RC_POWER_DOWN] != -1) {
			SetControlValue( ASI_RC_POWER_DOWN, 1.0f );
			UpdateUnitTime( ASI_RC_POWER_DOWN, 1.0f );
		}
		if( m_anAnimStackIndex[ASI_RC_POWER_UP] != -1) {
			SetControlValue( ASI_RC_POWER_UP, 0.0f );
		}
	}
}


BOOL CBot::Power_IsPoweringUp( void ) const {
	FASSERT( IsCreated() );

	if( (m_nPowerState == POWERSTATE_POWERING_UP) || (m_nPowerState == POWERSTATE_FINISHING_UP) ) {
		return !m_bOppositePowerStatePending;
	}

	if( (m_nPowerState == POWERSTATE_WAITING_FOR_IMMOBILIZED) || (m_nPowerState == POWERSTATE_POWERING_DOWN) ) {
		return m_bOppositePowerStatePending;
	}

	return FALSE;
}


BOOL CBot::Power_IsPoweringDown( void ) const {
	FASSERT( IsCreated() );

	if( (m_nPowerState == POWERSTATE_WAITING_FOR_IMMOBILIZED) || (m_nPowerState == POWERSTATE_POWERING_DOWN) ) {
		return !m_bOppositePowerStatePending;
	}

	if( (m_nPowerState == POWERSTATE_POWERING_UP) || (m_nPowerState == POWERSTATE_FINISHING_UP) ) {
		return m_bOppositePowerStatePending;
	}

	return FALSE;
}


void CBot::Power_Work( void ) {
	FASSERT( IsCreated() );

	switch( m_nPowerState ) {
	case POWERSTATE_POWERED_UP:
		// Nothing to do, so bail!
		break;

	case POWERSTATE_POWERED_DOWN:
		// If we were turned off on a timer...
		if( m_fPowerOffOnTime > 0.0f ) {
			m_fPowerOffOnTime -= FLoop_fPreviousLoopSecs;
			FMATH_CLAMPMIN( m_fPowerOffOnTime, 0.0f );

			// Time is done, turn the bot back on
			if( (m_fPowerOffOnTime == 0.0f) && !IsDead() && 
				(!(m_nBotFlags2 & BOTFLAG2_POWERDOWN_UNTIL_POSESSED) || (m_nRecruitID >= 0)) ) {
				Power( TRUE );
			}
		}
		break;

	case POWERSTATE_WAITING_FOR_IMMOBILIZED:
		if( IsImmobile() ) {
			// Bot is finally immobilized...
			m_nPowerState = POWERSTATE_POWERING_DOWN;
			ZeroTime( ASI_RC_POWER_DOWN );
			m_fPowerAnimUnitBlend = 0.0f;
		} else {
			// keep trying to immobilize him.  fixes bug where bot is 
			// re-mobilized in same frame after being EMPed.
			ImmobilizeBot();
		}


		break;

	case POWERSTATE_POWERING_DOWN:
		if( !DeltaTime( ASI_RC_POWER_DOWN, FLoop_fPreviousLoopSecs * m_pBotInfo_Gen->fPowerDownSpeedMult * m_fPowerOffOnAnimSpeedMult, TRUE ) ) {
			// We're powering down...

			if( m_fPowerAnimUnitBlend < 1.0f ) {
				m_fPowerAnimUnitBlend += FLoop_fPreviousLoopSecs;
				FMATH_CLAMPMAX( m_fPowerAnimUnitBlend, 1.0f );
				SetControlValue( ASI_RC_POWER_DOWN, m_fPowerAnimUnitBlend );
			}
		} else {
			// Done powering down...

			m_nPowerState = POWERSTATE_POWERED_DOWN;
			SetControlValue( ASI_RC_POWER_DOWN, 1.0f );

			if( m_bOppositePowerStatePending ) {
				// Start power-up...
				Power( TRUE, 0.0f, m_fPendingPowerOffOnAnimSpeedMult );
			}
		}

		break;

	case POWERSTATE_POWERING_UP:
		if( !DeltaTime( ASI_RC_POWER_UP, FLoop_fPreviousLoopSecs * m_pBotInfo_Gen->fPowerUpSpeedMult * m_fPowerOffOnAnimSpeedMult, TRUE ) ) {
			// We're powering up...

			if( m_fPowerAnimUnitBlend < 1.0f ) {
				m_fPowerAnimUnitBlend += FLoop_fPreviousLoopSecs * 1.5f;
				FMATH_CLAMPMAX( m_fPowerAnimUnitBlend, 1.0f );
				SetControlValue( ASI_RC_POWER_UP, m_fPowerAnimUnitBlend );
			}
		} else {
			// Done powering up...
			m_nPowerState = POWERSTATE_FINISHING_UP;
			SetControlValue( ASI_RC_POWER_UP, 1.0f );
			SetControlValue( ASI_RC_POWER_DOWN, 0.0f );
			m_fPowerAnimUnitBlend = 1.0f;
		}

		break;

	case POWERSTATE_FINISHING_UP:
		if( m_fPowerAnimUnitBlend > 0.0f ) {
			m_fPowerAnimUnitBlend -= FLoop_fPreviousLoopSecs * 6.0f;
			FMATH_CLAMPMIN( m_fPowerAnimUnitBlend, 0.0f );
			SetControlValue( ASI_RC_POWER_UP, m_fPowerAnimUnitBlend );
		} else {
			// Finished up...

			m_nPowerState = POWERSTATE_POWERED_UP;
			SetControlValue( ASI_RC_POWER_UP, 0.0f );

			if( m_bOppositePowerStatePending ) {
				// Start powering down...
				Power( FALSE, m_fPendingOffOnTime, m_fPendingPowerOffOnAnimSpeedMult );
			} else {
				MobilizeBot();
			}
		}

		break;

	default:
		FASSERT_NOW;
	}
}


// m_pnEnableBoneNameIndexTableForSummer_TetherShock must be initialized already.
void CBot::DataPort_SetupTetherShockInfo( void ) {
	cchar *pszBoneName;
	s32 nBoneIndex;
	u32 i;

	nBoneIndex = m_pWorldMesh->FindBone( m_pBotInfo_Gen->pszDataPortBoneName );
	if( nBoneIndex >= 0 ) {
		m_pDataPortBoneMtx = m_pWorldMesh->GetBoneMtxPalette()[nBoneIndex];
	}

	m_nTetherShockBoneCount = 0;

	for( i=0; m_pnEnableBoneNameIndexTableForSummer_TetherShock[i]<255; ++i ) {
		pszBoneName = m_Anim.m_pBotAnimStackDef->m_apszBoneNameTable[ m_pnEnableBoneNameIndexTableForSummer_TetherShock[i] ];
		nBoneIndex = m_Anim.m_pAnimManMtx->FindBoneIndex( pszBoneName );
		if( nBoneIndex >= 0 ) {
			FASSERT( m_nTetherShockBoneCount < FDATA_MAX_BONE_COUNT );
			m_anDriveManMtxIndexForSummer_TetherShock[m_nTetherShockBoneCount++] = (u8)nBoneIndex;
		}
	}
}


// Set fShockSecs to -1 for the shock to remain enabled until it is explicitely disabled.
// fShockSecs is ignored when bEnable is FALSE.
void CBot::DataPort_Shock( BOOL bEnable, f32 fShockSecs ) {
	if( bEnable ) {
		if( !DataPort_IsBeingShocked() ) {
			// Turn on tether shock...

			FMATH_SETBITMASK( m_nBotFlags, BOTFLAG_IS_BEING_SHOCKED );

			ImmobilizeBot();

			if( m_anAnimStackIndex[ASI_RC_TETHERED] != -1 ) {
				m_fTetherShockAnimUnitBlend = 1.0f;
				SetControlValue( ASI_RC_TETHERED, 1.0f );

				m_Anim.m_pAnimCombiner->Mask_UpdateTapBoneMask(
					m_Anim.m_pnAnimTapID[ m_Anim.m_pBotAnimStackDef->m_NameSetTaps.nSummerIndex ],
					m_pnEnableBoneNameIndexTableForSummer_TetherShock,
					m_Anim.m_pBotAnimStackDef->m_apszBoneNameTable,
					TRUE
				);

				m_Anim.ManMtxSummer_Identity();
			}

			if( fShockSecs >= 0.0f ) {
				m_fDataPortShockSecsRemaining = fShockSecs;
			}
		}
	} else {
		if( DataPort_IsBeingShocked() ) {
			// Turn off tether shock...
			FMATH_CLEARBITMASK( m_nBotFlags, BOTFLAG_IS_BEING_SHOCKED );

			if( m_anAnimStackIndex[ASI_RC_TETHERED] != -1 ) {

				m_Anim.ManMtxSummer_Identity();

				m_Anim.m_pAnimCombiner->Mask_UpdateTapBoneMask(
					m_Anim.m_pnAnimTapID[ m_Anim.m_pBotAnimStackDef->m_NameSetTaps.nSummerIndex ],
					m_pnEnableBoneNameIndexTableForSummer_Normal,
					m_Anim.m_pBotAnimStackDef->m_apszBoneNameTable,
					TRUE
				);

				SetControlValue( ASI_RC_TETHERED, 0.0f );
			}
			m_fDataPortShockSecsRemaining = 0.0f;
		}
	}
}


void CBot::DataPort_Work( void ) {
	u32 i;
	f32 fHudUnitDestruct;

	if( !DataPort_IsBeingShocked() ) {
		if( m_fTetherShockAnimUnitBlend > 0.0f ) {
			m_fTetherShockAnimUnitBlend -= FLoop_fPreviousLoopSecs * 4.0f;
			FMATH_CLAMPMIN( m_fTetherShockAnimUnitBlend, 0.0f );

			SetControlValue( ASI_RC_TETHERED, fmath_UnitLinearToSCurve( m_fTetherShockAnimUnitBlend ) );
		}
	} else {
		for( i=0; i<m_nTetherShockBoneCount; ++i ) {
			*m_Anim.m_pAnimManMtx->GetFrameAndFlagAsChanged( m_anDriveManMtxIndexForSummer_TetherShock[i] ) = m_aTetherMtx43A[ fmath_RandomChoice(TETHER_RANDOM_MTX_COUNT) ];
		}
	}

	if( (m_nPossessionPlayerIndex < 0) || (m_nOwnerPlayerIndex >= 0) ) {
		// We're not possessed...
		return;
	}

	if( m_nUnplugState > UNPLUG_STATE_NONE ) {
		// We're unplugging...
		_Work_UnplugTether();
		return;
	}

	// We're possessed...
	f32 fPossessionDist, fSurgeAmp, fUnitFluxStatic;

	// Compute the unit distance from the possession point...
	fPossessionDist = m_MtxToWorld.m_vPos.Dist( m_PossessedPos_WS );

	// Compute self-destruct static...
	fHudUnitDestruct = CHud2::GetHudForPlayer(m_nPossessionPlayerIndex)->GetUnitSelfDestruct();

	if( fHudUnitDestruct == 1.0f ) {
		// Pull the plug...

		PullThePlug();

		return;
	}

	if( fHudUnitDestruct > m_fUnitHudStatic ) {
		m_fUnitHudStatic += FLoop_fPreviousLoopSecs * 8.0f;
		FMATH_CLAMPMAX( m_fUnitHudStatic, fHudUnitDestruct );
	} else if( fHudUnitDestruct < m_fUnitHudStatic ) {
		m_fUnitHudStatic -= FLoop_fPreviousLoopSecs * 2.0f;
		FMATH_CLAMPMIN( m_fUnitHudStatic, fHudUnitDestruct );
	}

	// Compute the static on the screen...
	if( m_bHysterisisLargerThreshold ) {
		// We're not in the static region.
		// We're looking for the bot to exceed our larger distance...

		if( m_fUnitBaseStatic > 0.0f ) {
			m_fUnitBaseStatic -= FLoop_fPreviousLoopSecs * _STATIC_BASE_DECAY_RATE;
			FMATH_CLAMPMIN( m_fUnitBaseStatic, 0.0f );
		}

		m_fUnitStatic = m_fUnitBaseStatic;

		if( fPossessionDist >= m_fStaticOnDistThreshold ) {
			// The bot has crossed into the static region...

			m_bHysterisisLargerThreshold = FALSE;
			m_fStaticOnDistThreshold = m_fPossessionDist - _START_STATIC_DIST_FROM_LIMIT - _STATIC_HYSTERISIS_DIST;
			m_fUnitStaticSurge = 0.0f;
			m_fUnitBaseStatic = _STATIC_UNIT_BASE;

			m_fUnitStaticFluxSliderRate = 100000.0f;
			m_fStartUnitStaticFlux = 0.0f;
			m_fEndUnitStaticFlux = 0.0f;
			m_fUnitOutOfRangeWarningMsg = 0.0f;
		}
	} else {
		// We're in the static region.
		// We're looking for the bot to fall closer than our smaller distance...

		m_fUnitStatic = (fPossessionDist - m_fStaticOnDistThreshold) * (1.0f / (_START_STATIC_DIST_FROM_LIMIT + _STATIC_HYSTERISIS_DIST));
		m_fUnitStatic *= m_fUnitStatic * m_fUnitStatic * m_fUnitStatic;
		m_fUnitStatic *= m_fUnitStatic;
		m_fUnitStatic += m_fUnitBaseStatic;
		FMATH_CLAMP( m_fUnitStatic, 0.0f, 1.0f );

		if( m_fUnitStatic >= 0.85f ) {
			// Pull the plug...

			PullThePlug();

			return;
		}

		if( fPossessionDist <= m_fStaticOnDistThreshold ) {
			// The bot has crossed out of the static region...

			m_bHysterisisLargerThreshold = TRUE;
			m_fStaticOnDistThreshold = m_fPossessionDist - _START_STATIC_DIST_FROM_LIMIT;
		}
	}

	m_fUnitOutOfRangeWarningMsg += FLoop_fPreviousLoopSecs * _POSSESSION_OUTOFRANGE_WARNING_FREQ;
	if( m_fUnitOutOfRangeWarningMsg >= 1.0f ) {
		m_fUnitOutOfRangeWarningMsg = 0.0f;
	}

	m_fUnitDrawStatic = m_fUnitStatic;

	// Apply HUD self-destruct static...
	if( m_fUnitHudStatic > 0.0f ) {
		m_fUnitDrawStatic += m_fUnitHudStatic;
		FMATH_CLAMPMAX( m_fUnitDrawStatic, 1.0f );
	}

	// Update fluctuation...
	if( (m_fUnitStatic > 0.0f) && (m_fUnitStaticSurge == 1.0f) ) {
		m_fUnitStaticFluxSlider += FLoop_fPreviousLoopSecs * m_fUnitStaticFluxSliderRate;
		if( m_fUnitStaticFluxSlider >= 1.0f ) {
			m_fUnitStaticFluxSlider = 0.0f;

			// Set up static flux data...
			m_fUnitStaticFluxSliderRate = fmath_RandomFloatRange( 4.0f, 8.0f );
			m_fStartUnitStaticFlux = m_fEndUnitStaticFlux;
			m_fEndUnitStaticFlux = fmath_RandomFloat();
		}

		// Apply fluctuation...
		fUnitFluxStatic = FMATH_FPOT( m_fUnitStaticFluxSlider, m_fStartUnitStaticFlux, m_fEndUnitStaticFlux );
		fUnitFluxStatic = fmath_UnitLinearToSCurve( fUnitFluxStatic );
		m_fUnitDrawStatic += 0.4f * m_fUnitStatic * (fUnitFluxStatic - 0.5f);
		FMATH_CLAMP( m_fUnitDrawStatic, 0.0f, 1.0f );
	}

	// Apply static surge...
	if( m_fUnitStaticSurge < 1.0f ) {
		m_fUnitStaticSurge += FLoop_fPreviousLoopSecs * (1.0f / _STATIC_SURGE_TIME);
		FMATH_CLAMPMAX( m_fUnitStaticSurge, 1.0f );

		fSurgeAmp = _STATIC_SURGE_UNIT_AMP * fmath_Sin( FMATH_PI * m_fUnitStaticSurge );
		m_fUnitDrawStatic = FMATH_FPOT( m_fUnitStaticSurge, fSurgeAmp, m_fUnitDrawStatic );
	}

}


void CBot::PullThePlug( BOOL bMakeSoundEffects ) {
	// Restore original armor modifier...
	SetArmorModifier( m_fOrigArmorModifier );

	// Let the barter bots know they can come out of hiding now...
	bartersystem_Hide( FALSE );

	if( bMakeSoundEffects ) {
		CFSoundGroup::PlaySound( m_pSoundGroupPullThePlug );
	}

	m_nUnplugState = UNPLUG_STATE_STATIC_SHRINK_Y;
	m_fUnitUnplug = 0.0f;

	m_fUnitStatic = 1.0f;
	m_fUnitDrawStatic = 1.0f;

	_InitTransmissionLostTextBox();

	// Enable the data port effect...
	if( m_bDataPortOpen && m_pDataPortEffectMeshEntity ) {
		m_pDataPortEffectMeshEntity->Attach_UnitMtxToParent_PS( this, m_pBotInfo_Gen->pszDataPortBoneName );
		m_pDataPortEffectMeshEntity->AddToWorld();
	}

	ImmobilizeBot();

	// Power down the bot for a pre-determined amount of time...
	Power( FALSE, m_fPossessionTerminatedPowerDownTime );

	CPauseScreen::SetEnabled( FALSE );

	if( m_nPossessionPlayerIndex >= 0 ) {
		CHud2::GetHudForPlayer(m_nPossessionPlayerIndex)->SetWSEnable( FALSE );
		CHud2::GetHudForPlayer(m_nPossessionPlayerIndex)->SetDrawEnabled( FALSE );
	}

	if( !AIBrain() ) {
		// No-brains case needs autowork on...
		EnableAutoWork( TRUE );
	}

	// Set back the primary weapon EUK level during possession...
	if( m_apWeapon[0] ) {
		m_apWeapon[0]->SetItemInst(NULL);
		if( m_apWeapon[0]->GetUpgradeLevel() != m_nPrePossessionWeaponEUKLevel[ 0 ] ) {
			m_apWeapon[0]->SetUpgradeLevel( m_nPrePossessionWeaponEUKLevel[ 0 ] );
		}
	}

	if( m_apWeapon[1] ) {
		m_apWeapon[1]->SetItemInst(NULL);
		if( m_apWeapon[1]->GetUpgradeLevel() != m_nPrePossessionWeaponEUKLevel[ 1 ] ) {
			m_apWeapon[1]->SetUpgradeLevel( m_nPrePossessionWeaponEUKLevel[ 1 ] );
		}
	}
	m_pInventory = NULL;

	if( m_pCanOnlyBeDispensedBy && IsPossessedByConsole() ) {
		// This bot was specifically allocated by a console dispenser for the player... 
		// If this bot is still alive and being exited, then kill this bot off so
		// it will return to the autobot pool and be available again for possession.
		if( !IsDeadOrDying() || (m_uBotDeathFlags & CBot::BOTDEATHFLAG_WALKING_DEAD ) ) {
			Die( FALSE, FALSE );
		}
	}

	if( m_nPossessionPlayerIndex >= 0 ) {
		CFScriptSystem::TriggerEvent(CFScriptSystem::GetEventNumFromName("possess"), m_nPossessionPlayerIndex, (u32) this, FALSE);

		CPlayer* pPlayer = &Player_aPlayer[ m_nPossessionPlayerIndex ];
		CBot* pPlayerBot = (CBot *)pPlayer->m_pEntityOrig;

		// If this is multiplayer, give the player back his proper speed, and me as well
		if (MultiplayerMgr.IsMultiplayer()) {
			pPlayerBot->SetRunSpeed( MultiplayerMgr.GetSpeedMultiplier( m_nPossessionPlayerIndex ) );
			SetRunSpeed(1.0f);

			// If we are not recruited, then reset our team back to its original team
			if ( m_nRecruitID < 0 ) {
				ResetTeam();
			}
		}

		// LET THE POSSESSING PLAYER KNOW THAT THEY ARE NO LONGER POSSESSING THIS BOT
		pPlayerBot->HaveUnPossessedBot( this );
	}
}


void CBot::SetHudMode( CHud2 *pHud, const CBotDef *pBotDef ) {
	if( pBotDef->m_nRace == BOTRACE_DROID ) {
		if( pBotDef->m_nClass == BOTCLASS_MINER ) {
            pHud->SetHudMode( HUDMODE_GLITCH );
		} else if( pBotDef->m_nClass == BOTCLASS_CHEMBOT) {
			pHud->SetHudMode( HUDMODE_SLOSH );
		} else if( pBotDef->m_nClass == BOTCLASS_KRUNK) {
			pHud->SetHudMode( HUDMODE_KRUNK );
		}else if( pBotDef->m_nClass == BOTCLASS_COLOSSUS) {
			pHud->SetHudMode( HUDMODE_MOZER );
		}
	} else {
		pHud->SetHudMode( HUDMODE_MIL );

		if( pBotDef->m_nClass == BOTCLASS_AAGUN ) {
			pHud->SetRedTint( FALSE );
			pHud->SetDrawFlags( CHud2::DRAW_BATTERIES |
								CHud2::DRAW_RADAR |
								CHud2::DRAW_COLALLOY_MSG |
								CHud2::DRAW_BATTERY_MESHES |
								CHud2::DRAW_ITEMS |
								CHud2::DRAW_WASHERS );
		}
	}
}


void CBot::ForceQuickDataPortUnPlug( void ) {
	if( m_nPossessionPlayerIndex < 0 ) {
		return;
	}

	PullThePlug( FALSE );

	_EnablePossessionTextBox( FALSE );
	CPlayer* pPlayer = &Player_aPlayer[m_nPossessionPlayerIndex];
	gamecam_SwitchPlayerTo3rdPersonCamera( PLAYER_CAM(m_nPossessionPlayerIndex), (CBot *)pPlayer->m_pEntityOrig );

	// Deal with our bot...
	aibrainman_ConfigurePostPossessionBotBrain(AIBrain());
	MobilizeBot();

	// Switch to original bot...
	pPlayer->m_pEntityCurrent = pPlayer->m_pEntityOrig;

	// Deal with original bot...
	CBot* pOrigBot = (CBot *)pPlayer->m_pEntityOrig;
	pOrigBot->m_nPossessionPlayerIndex = m_nPossessionPlayerIndex;
	pPlayer->EnableEntityControl();

	if( (pOrigBot->TypeBits() & (ENTITY_BIT_BOTGLITCH | ENTITY_BIT_KRUNK) ) && !IsPossessedByConsole() ) {
		// put glitch back together
		pOrigBot->BreakIntoPieces( -1.0f );
	} else {
		pOrigBot->MobilizeBot();
		SetPossessedByConsole( FALSE );
	}

	CPauseScreen::SetEnabled( TRUE );
	CHud2* pHud = CHud2::GetHudForPlayer( m_nPossessionPlayerIndex );
	SetHudMode( pHud, pOrigBot->m_pBotDef );	
	pHud->SetWSEnable( TRUE );
	pHud->SetDrawEnabled( TRUE );
	m_nPossessionPlayerIndex = -1;

	m_fStaticOnDistThreshold = 0.0f;
	m_bHysterisisLargerThreshold = TRUE;
	m_fUnitStaticSurge = 1.0f;
	m_fUnitBaseStatic = 0.0f;
	m_fUnitHudStatic = 0.0f;
	m_fUnitStatic = 0.0f;
	m_fUnitDrawStatic = 0.0f;

	m_fUnitStaticFluxSlider = 0.0f;
	m_fUnitStaticFluxSliderRate = 1000.0f;
	m_fStartUnitStaticFlux = 0.0f;
	m_fEndUnitStaticFlux = 0.0f;

	if( pOrigBot->m_apWeapon[0] ) {
		// Restore this bot's reticle to it's drawn...
		pOrigBot->m_apWeapon[0]->UpdateReticle();

		// Kill the tether...
		if( pOrigBot->m_apWeapon[0]->TypeBits() & ENTITY_BIT_WEAPONTETHER ) {
			((CWeaponTether *)pOrigBot->m_apWeapon[0])->DestroyTether( TRUE );
		}
	}
}


void CBot::_Work_UnplugTether( void ) {
	CPlayer *pPlayer;
	CBot *pOrigBot;

	switch( m_nUnplugState ) {
	case UNPLUG_STATE_STATIC_SHRINK_Y:
		m_fUnitUnplug += FLoop_fPreviousLoopSecs * 4.0f;

		if( m_fUnitUnplug >= 1.0f ) {
			m_fUnitUnplug = 0.0f;
			m_nUnplugState = UNPLUG_STATE_STATIC_SHRINK_X;
		}

		break;

	case UNPLUG_STATE_STATIC_SHRINK_X:
		m_fUnitUnplug += FLoop_fPreviousLoopSecs * 2.0f;

		if( m_fUnitUnplug >= 1.0f ) {
			m_fUnitUnplug = 0.0f;
			m_nUnplugState = UNPLUG_STATE_DOT_FADE;
		}

		break;

	case UNPLUG_STATE_DOT_FADE:
		m_fUnitUnplug += FLoop_fPreviousLoopSecs * 2.0f;

		if( m_fUnitUnplug >= 1.0f ) {
			m_fUnitUnplug = 0.0f;
			fvis_BlockOnStreamingData();
			m_nUnplugState = UNPLUG_STATE_FADE_IN;

			_EnablePossessionTextBox( FALSE );

			pPlayer = &Player_aPlayer[ m_nPossessionPlayerIndex ];
			gamecam_SwitchPlayerTo3rdPersonCamera( PLAYER_CAM(m_nPossessionPlayerIndex), (CBot *)pPlayer->m_pEntityOrig );
		}

		break;

	case UNPLUG_STATE_FADE_IN:
		m_fUnitUnplug += FLoop_fPreviousLoopSecs * 2.0f;

		if( m_fUnitUnplug >= 1.0f ) {
			m_fUnitUnplug = 0.0f;
			m_nUnplugState = UNPLUG_STATE_POWER_UP;
		}

		break;

	case UNPLUG_STATE_POWER_UP:
		{
			m_nUnplugState = UNPLUG_STATE_NONE;
			m_fUnitUnplug = 0.0f;

			pPlayer = &Player_aPlayer[ m_nPossessionPlayerIndex ];

			// Deal with our bot...
			aibrainman_ConfigurePostPossessionBotBrain(AIBrain());
			//RAFHACK -- Replacing MobilizeBot with a powerdown command...
			//MobilizeBot();

			// Switch to original bot...
			pPlayer->m_pEntityCurrent = pPlayer->m_pEntityOrig;

			// Deal with original bot...
			pOrigBot = (CBot *)pPlayer->m_pEntityOrig;
			pOrigBot->m_nPossessionPlayerIndex = m_nPossessionPlayerIndex;
			pPlayer->EnableEntityControl();
			
			if( (pOrigBot->TypeBits() & (ENTITY_BIT_BOTGLITCH | ENTITY_BIT_KRUNK)) && !IsPossessedByConsole() ) {
				// put glitch back together
				pOrigBot->BreakIntoPieces( -1.0f );
			} else {
				pOrigBot->MobilizeBot();
				SetPossessedByConsole( FALSE );
			}

			CPauseScreen::SetEnabled( TRUE );
			CHud2* pHud = CHud2::GetHudForPlayer(m_nPossessionPlayerIndex);
			SetHudMode( pHud, pOrigBot->m_pBotDef );
			pHud->SetWSEnable( TRUE );
			pHud->SetDrawEnabled( TRUE );
			m_nPossessionPlayerIndex = -1;

			m_fStaticOnDistThreshold = 0.0f;
			m_bHysterisisLargerThreshold = TRUE;
			m_fUnitStaticSurge = 1.0f;
			m_fUnitBaseStatic = 0.0f;
			m_fUnitHudStatic = 0.0f;
			m_fUnitStatic = 0.0f;
			m_fUnitDrawStatic = 0.0f;

			m_fUnitStaticFluxSlider = 0.0f;
			m_fUnitStaticFluxSliderRate = 1000.0f;
			m_fStartUnitStaticFlux = 0.0f;
			m_fEndUnitStaticFlux = 0.0f;

			if( pOrigBot->m_apWeapon[0] ) {
				// Restore this bot's reticle to it's drawn...
				pOrigBot->m_apWeapon[0]->UpdateReticle();
			}
		}

		break;

	default:
		FASSERT_NOW;
	}
}


void CBot::Possess( s32 nPlayerIndex, f32 fPossessionArmorModifier ) {
	cchar *pszWeaponName;
	CVehicle *pDrivingVehicle = NULL;
	CVehicle::Station_e eDriverStation = CVehicle::STATION_NONE;

	FASSERT( IsCreated() && IsInWorld());
	FASSERT( nPlayerIndex >= 0 );
	FASSERT( nPlayerIndex < CPlayer::m_nPlayerCount );

	if( m_pDrivingVehicle ) {
		// temporarily remove driver from vehicle.  We will restore him to vehicle at end of this function.
		pDrivingVehicle = m_pDrivingVehicle;
		eDriverStation = m_pDrivingVehicle->WhichStationOccupied( this );
		FASSERT( eDriverStation != CVehicle::STATION_NONE );
		if( eDriverStation != CVehicle::STATION_NONE ) {
			m_pDrivingVehicle->ExitStation( this, TRUE /*bImmediately*/ );
		}
	}

	// Set our working pointers
	CPlayer *pPlayer = &Player_aPlayer[ nPlayerIndex ];
	FASSERT( pPlayer->m_pEntityCurrent->TypeBits() & ENTITY_BIT_BOT );
	CBot *pBotThatIsDoingThePossession = (CBot *)pPlayer->m_pEntityCurrent;

	// Kill any active powerup effects on the possessor
	if( pBotThatIsDoingThePossession->GetPowerupFx() ) {
		pBotThatIsDoingThePossession->GetPowerupFx()->KillAll();
	}

	// BEFORE DOING ANYTHING, CALL THE BOT DOING THE POSSESSING TO NOTIFY THEM (ALLOWING THEM TO SAVE THEIR INVENTORY)
	pBotThatIsDoingThePossession->AboutToPossessBot( this ); 

	// If someone is already possessing this bot (multiplayer), then jerk them out of it...
	if( m_nPossessionPlayerIndex >= 0 ) {
		ForceQuickDataPortUnPlug();
	}

	// Kill the data port effect...
	if( m_pDataPortEffectMeshEntity ) {
		m_pDataPortEffectMeshEntity->DetachFromParent();
		m_pDataPortEffectMeshEntity->RemoveFromWorld();
	}

	// Set possession armor modifier...
	m_fOrigArmorModifier = GetArmorModifier();
	SetArmorModifier( fPossessionArmorModifier );

	// Hide the barter bots!
	bartersystem_Hide( TRUE );

	pPlayer->m_pEntityCurrent = this;

	if( MultiplayerMgr.IsMultiplayer() ) {
		// If this bot has been recruited, and we are being possessed by someone
		// on the other team, then unrecruit the bot
		if( (m_nRecruitID >= 0) && !IsSameTeam( pBotThatIsDoingThePossession ) ) {
			Unrecruit();
		}
	}

	// Copy the player's team into this bot (only affects multiplayer)
	SetTeam( pBotThatIsDoingThePossession->GetTeam() );

	if( AIBrain() ) {
		// Turns this bot's brain into a player brain...
		aibrainman_ConfigurePlayerBotBrain( AIBrain(), nPlayerIndex );
	} else {
		// No brains case needs autowork off...
		EnableAutoWork( FALSE );
	}

	gamecam_SwitchPlayerTo3rdPersonCamera( PLAYER_CAM(nPlayerIndex), this );

	// Get the possession inventory for this player...
	m_pInventory = pPlayer->GetInventory( nPlayerIndex, TRUE );

	m_pInventory->m_pfcnCallback = NULL;
	m_pInventory->m_auCurWeapon[0] = 0;
	m_pInventory->m_auCurWeapon[1] = 0;
	m_pInventory->m_auNumWeapons[0] = 1;
	m_pInventory->m_auNumWeapons[1] = (m_apWeapon[1] != NULL);

	u32 uPossessionEUKLevel;
	// Set the primary weapon EUK level during possession...
	if( m_apWeapon[0] ) {
		m_nPrePossessionWeaponEUKLevel[ 0 ] = m_apWeapon[0]->GetUpgradeLevel();
		uPossessionEUKLevel = m_nPrePossessionWeaponEUKLevel[ 0 ] + 1;
		FMATH_CLAMPMAX( uPossessionEUKLevel, m_apWeapon[0]->GetMaxUpgradeLevel() );
		m_apWeapon[0]->SetUpgradeLevel( uPossessionEUKLevel );

		// Set up primary weapon inventory...
		pszWeaponName = CItem::GetWeaponName( m_apWeapon[0] );
		m_pInventory->m_aoWeapons[0][0].m_pItemData = CItemRepository::RetrieveEntry( pszWeaponName, m_apWeapon[0]->GetUpgradeLevel(), NULL );
		m_pInventory->m_aoWeapons[0][0].m_nClipAmmo = m_apWeapon[0]->GetClipAmmo();
		m_pInventory->m_aoWeapons[0][0].m_nReserveAmmo = m_apWeapon[0]->GetReserveAmmo();
		m_apWeapon[0]->SetItemInst( &m_pInventory->m_aoWeapons[0][0], TRUE );
		m_pInventory->m_aoWeapons[0][0].m_pOwnerInventory = m_pInventory;
	}

	// Set up secondary weapon inventory...
	if( m_apWeapon[1] ) {

		// set the secondary weapon euk level during possession
		m_nPrePossessionWeaponEUKLevel[ 1 ] = m_apWeapon[1]->GetUpgradeLevel();
		uPossessionEUKLevel = m_nPrePossessionWeaponEUKLevel[ 1 ] + 1;
		FMATH_CLAMPMAX( uPossessionEUKLevel, m_apWeapon[1]->GetMaxUpgradeLevel() );
		m_apWeapon[1]->SetUpgradeLevel( uPossessionEUKLevel );
		
		pszWeaponName = CItem::GetWeaponName( m_apWeapon[1] );
		m_pInventory->m_aoWeapons[1][0].m_pItemData = CItemRepository::RetrieveEntry( pszWeaponName, m_apWeapon[1]->GetUpgradeLevel(), NULL );
		m_pInventory->m_aoWeapons[1][0].m_nClipAmmo = m_apWeapon[1]->GetClipAmmo();
		m_pInventory->m_aoWeapons[1][0].m_nReserveAmmo = m_apWeapon[1]->GetReserveAmmo();
		m_apWeapon[1]->SetItemInst( &m_pInventory->m_aoWeapons[1][0], TRUE );
		m_pInventory->m_aoWeapons[1][0].m_pOwnerInventory = m_pInventory;
	} else {
		m_pInventory->m_aoWeapons[1][0].m_pItemData = CItemRepository::RetrieveEntry( "Empty Secondary", NULL );
	}

	m_nPossessionPlayerIndex = nPlayerIndex;
	pBotThatIsDoingThePossession->m_nPossessionPlayerIndex = -1;

	SetHudMode( CHud2::GetHudForPlayer(nPlayerIndex), m_pBotDef );
	
	// If this is a multiplayer game, bring over any speed modifications from
	// the possessing bot
	SetRunSpeed( MultiplayerMgr.GetSpeedMultiplier( nPlayerIndex ) );

	// Initialize possession data...
	m_PossessedPos_WS = m_MtxToWorld.m_vPos;

	m_fStaticOnDistThreshold = m_fPossessionDist - _START_STATIC_DIST_FROM_LIMIT;
	m_bHysterisisLargerThreshold = TRUE;
	m_fUnitStaticSurge = 1.0f;
	m_fUnitBaseStatic = 0.0f;
	m_fUnitStatic = 0.0f;
	m_fUnitDrawStatic = 0.0f;

	m_fUnitStaticFluxSlider = 0.0f;
	m_fUnitStaticFluxSliderRate = 1000.0f;
	m_fStartUnitStaticFlux = 0.0f;
	m_fEndUnitStaticFlux = 0.0f;

	_InitPossessionOutOfRangeWarningTextBox();
	_EnablePossessionTextBox( FALSE );

	m_nUnplugState = UNPLUG_STATE_NONE;
	m_fUnitUnplug = 0.0f;

	if( m_apWeapon[0] ) {
		// Show possessed bot's reticle...
		m_apWeapon[0]->UpdateReticle();
	}

	if( pDrivingVehicle && eDriverStation != CVehicle::STATION_NONE ) {
		// restore bot into vehicle he was previously driving.
		CVehicle::StationStatus_e eStatus;
		eStatus = pDrivingVehicle->EnterStation( this, eDriverStation, FALSE /*bProximityCheck*/, TRUE /*bImmediately*/ );
		FASSERT( eStatus == CVehicle::STATION_STATUS_ENTERING );
	}

    CFScriptSystem::TriggerEvent(CFScriptSystem::GetEventNumFromName("possess"), nPlayerIndex, (u32) this, TRUE);

	// AS THE VERY LAST THING, CALL THE BOT DOING THE POSSESSING TO NOTIFY THEM THAT THEY HAVE POSSESSED SOMEONE
	pBotThatIsDoingThePossession->HavePossessedBot( this ); 

}


void CBot::SetNoLegsMode( BOOL bNoLegs ) {
	FASSERT( IsCreated() );

	if( bNoLegs ) {
		if( !HasNoLegs() ) {
			FMATH_SETBITMASK( m_nBotFlags, BOTFLAG_NO_LEGS );

			ApplyVelocityImpulse_WS( CFVec3A( 0.0f, 20.0f, 0.0f ) );

			// Cancel any bot talks...
			if( m_pActiveTalkInst ) { 
				CTalkSystem2::TerminateActiveTalk( m_pActiveTalkInst );
			}
		}
	} else {
		if( HasNoLegs() ) {
			FMATH_CLEARBITMASK( m_nBotFlags, BOTFLAG_NO_LEGS );

			if( m_pNoLegsDebrisPileWorldMesh ) {
				m_pNoLegsDebrisPileWorldMesh->RemoveFromWorld();
			}
		}
	}
}


// Assumes the FDraw renderer is selected.
// Assumes the current viewport is selected.
// Assumes the Xfm stack has been reset.
//
// Doesn't assume the FDraw state is set up.
// Will not preserve the FDraw state.
// Will preserve the Xfm stack, the active viewport, and
// the current FDraw renderer.
void CBot::DrawOrthoEffects( s32 nPlayerIndex ) {
	CPlayer *pPlayer = &Player_aPlayer[nPlayerIndex];

	if( pPlayer->m_pEntityCurrent == pPlayer->m_pEntityOrig ) {
		// Player is not in possession...
	}

	if( !(pPlayer->m_pEntityCurrent->TypeBits() & ENTITY_BIT_BOT) ) {
		// Not a bot...
		return;
	}

	// Player is in possession of a bot!
	CBot *pBot = (CBot *)pPlayer->m_pEntityCurrent;

	ftext_GetAttributes( Player_aPlayer[ nPlayerIndex ].m_hTextBoxOutOfRange )->bVisible = FALSE;

	if( pBot->m_fUnitDrawStatic > 0.0f ) {
		pBot->_DrawStatic();
	}
}


void CBot::_DrawStatic( void ) {
	if( (m_nUnplugState <= UNPLUG_STATE_FADE_IN) && (m_fUnitDrawStatic > 0.0f) ) {
		frenderer_SetDefaultState();
		fdraw_Depth_SetTest( FDRAW_DEPTHTEST_ALWAYS );
		fdraw_Depth_EnableWriting( FALSE );

		FViewport_t *pViewport = fviewport_GetActive();

		CFColorRGBA ColorRGBA;
		FDrawVtx_t aVtx[4];
		f32 fZ, fTexOffsetX, fTexOffsetY, fScaleX, fScaleY, fUnit;
		f32 fLeftX, fRightX, fBottomY, fTopY;
		u32 i;

		fZ = 0.5f * (pViewport->fNearZ + pViewport->fFarZ);

		if( (m_nUnplugState >= UNPLUG_STATE_STATIC_SHRINK_Y) && (m_nUnplugState <= UNPLUG_STATE_FADE_IN) ) {
			// Draw a black background first...

			if( m_nUnplugState == UNPLUG_STATE_FADE_IN ) {
				fdraw_Alpha_SetBlendOp( FDRAW_BLENDOP_LERP_WITH_ALPHA_OPAQUE );
				ColorRGBA.Set( 0.0f, 0.0f, 0.0f, 1.0f - m_fUnitUnplug );
			} else {
				fdraw_Alpha_SetBlendOp( FDRAW_BLENDOP_SRC );
				ColorRGBA.OpaqueBlack();
			}

			fdraw_Color_SetFunc( FDRAW_COLORFUNC_DECAL_AI );
			fdraw_SetTexture( NULL );

			for( i=0; i<4; ++i ) {
				aVtx[i].ColorRGBA = ColorRGBA;
				aVtx[i].ST.Zero();
			}

			aVtx[0].Pos_MS.Set( -pViewport->HalfRes.x,  pViewport->HalfRes.y, fZ );
			aVtx[1].Pos_MS.Set(  pViewport->HalfRes.x,  pViewport->HalfRes.y, fZ );
			aVtx[2].Pos_MS.Set( -pViewport->HalfRes.x, -pViewport->HalfRes.y, fZ );
			aVtx[3].Pos_MS.Set(  pViewport->HalfRes.x, -pViewport->HalfRes.y, fZ );

			fdraw_PrimList( FDRAW_PRIMTYPE_TRISTRIP, aVtx, 4 );
		}

		if( m_nUnplugState <= UNPLUG_STATE_DOT_FADE ) {
			// Draw the static...

			fScaleX = fScaleY = 1.0f;

			switch( m_nUnplugState ) {
			case UNPLUG_STATE_STATIC_SHRINK_Y:
				fUnit = m_fUnitUnplug * m_fUnitUnplug;
				fScaleY = FMATH_FPOT( fUnit, 1.0f, 0.002f );
				break;

			case UNPLUG_STATE_STATIC_SHRINK_X:
				fUnit = 1.0f - m_fUnitUnplug;
				fUnit *= fUnit * fUnit;
				fScaleX = FMATH_FPOT( fUnit, 0.002f, 1.0f );
				fScaleY = 0.002f;
				break;

			case UNPLUG_STATE_DOT_FADE:
				fScaleX = fScaleY = 0.002f;
				break;
			}

			fLeftX = -pViewport->HalfRes.x * fScaleX;
			fRightX = pViewport->HalfRes.x * fScaleX;
			fBottomY = -pViewport->HalfRes.y * fScaleY;
			fTopY = pViewport->HalfRes.y * fScaleY;

			aVtx[0].Pos_MS.Set( fLeftX, fTopY, fZ );
			aVtx[1].Pos_MS.Set( fRightX,fTopY, fZ );
			aVtx[2].Pos_MS.Set( fLeftX, fBottomY, fZ );
			aVtx[3].Pos_MS.Set( fRightX, fBottomY, fZ );

			if( m_StaticTexInst.GetTexDef() ) {
				fTexOffsetX = fmath_RandomFloat() * _STATIC_TEXTURE_SCALE;
				fTexOffsetY = fmath_RandomFloat() * _STATIC_TEXTURE_SCALE;

				aVtx[0].ST.Set( fTexOffsetX, fTexOffsetY );
				aVtx[1].ST.Set( fTexOffsetX + _STATIC_TEXTURE_SCALE, fTexOffsetY );
				aVtx[2].ST.Set( fTexOffsetX, fTexOffsetY + _STATIC_TEXTURE_SCALE );
				aVtx[3].ST.Set( fTexOffsetX + _STATIC_TEXTURE_SCALE, fTexOffsetY + _STATIC_TEXTURE_SCALE );

				switch( m_nUnplugState ) {
				case UNPLUG_STATE_NONE:
					fdraw_Alpha_SetBlendOp( FDRAW_BLENDOP_LERP_WITH_ALPHA_OPAQUE );
					fdraw_Color_SetFunc( FDRAW_COLORFUNC_DIFFUSETEX_AI );
					fdraw_SetTexture( &m_StaticTexInst );

					ColorRGBA.Set( 0.6f, 0.6f, 0.6f, m_fUnitDrawStatic );

					break;

				case UNPLUG_STATE_STATIC_SHRINK_Y:
					fdraw_Alpha_SetBlendOp( FDRAW_BLENDOP_LERP_WITH_ALPHA_OPAQUE );
					fdraw_Color_SetFunc( FDRAW_COLORFUNC_DIFFUSETEX_AI );
					fdraw_SetTexture( &m_StaticTexInst );

					fUnit = FMATH_FPOT( m_fUnitUnplug, 0.6f, 1.0f );
					ColorRGBA.Set( fUnit, fUnit, fUnit, 1.0f );

					break;

				case UNPLUG_STATE_STATIC_SHRINK_X:
					fdraw_Alpha_SetBlendOp( FDRAW_BLENDOP_SRC );
					fdraw_Color_SetFunc( FDRAW_COLORFUNC_SPECULARTEX_AT );
					fdraw_SetTexture( &m_StaticTexInst );

					ColorRGBA.Set( 1.0f, 1.0f, 1.0f, m_fUnitUnplug );

					break;

				case UNPLUG_STATE_DOT_FADE:
					fdraw_Alpha_SetBlendOp( FDRAW_BLENDOP_LERP_WITH_ALPHA_OPAQUE );
					fdraw_Color_SetFunc( FDRAW_COLORFUNC_DECAL_AI );
					fdraw_SetTexture( NULL );

					ColorRGBA.Set( 1.0f, 1.0f, 1.0f, 1.0f - m_fUnitUnplug );

					break;
				}

				for( i=0; i<4; ++i ) {
					aVtx[i].ColorRGBA = ColorRGBA;
				}

				fdraw_PrimList( FDRAW_PRIMTYPE_TRISTRIP, aVtx, 4 );
			}

			// Draw the glowcard...

			if( (m_nUnplugState >= UNPLUG_STATE_STATIC_SHRINK_Y) && (m_nUnplugState <= UNPLUG_STATE_DOT_FADE) ) {
				if( m_GlowTexInst.GetTexDef() ) {
					fdraw_Alpha_SetBlendOp( FDRAW_BLENDOP_LERP_WITH_ALPHA_OPAQUE );
					fdraw_Color_SetFunc( FDRAW_COLORFUNC_DIFFUSETEX_AIAT );
					fdraw_SetTexture( &m_GlowTexInst );

					switch( m_nUnplugState ) {
					case UNPLUG_STATE_STATIC_SHRINK_Y:
						ColorRGBA.Set( 1.0f, 1.0f, 1.0f, m_fUnitUnplug * 0.2f );
						break;

					case UNPLUG_STATE_STATIC_SHRINK_X:
						ColorRGBA.Set( 1.0f, 1.0f, 1.0f, 0.2f );
						break;

					case UNPLUG_STATE_DOT_FADE:
						ColorRGBA.Set( 1.0f, 1.0f, 1.0f, (1.0f - m_fUnitUnplug) * 0.2f );
						break;

					}

					for( i=0; i<4; ++i ) {
						aVtx[i].ColorRGBA = ColorRGBA;
					}

					aVtx[0].ST.Set( 0.0, 0.0f );
					aVtx[1].ST.Set( 1.0, 0.0f );
					aVtx[2].ST.Set( 0.0, 1.0f );
					aVtx[3].ST.Set( 1.0, 1.0f );

					fScaleX = 0.1f;
					fScaleY = 0.1f;

					fLeftX = -pViewport->HalfRes.x * fScaleX;
					fRightX = pViewport->HalfRes.x * fScaleX;
					fBottomY = -pViewport->HalfRes.y * fScaleY;
					fTopY = pViewport->HalfRes.y * fScaleY;

					aVtx[0].Pos_MS.x += fLeftX;
					aVtx[0].Pos_MS.y += fTopY;
					aVtx[1].Pos_MS.x += fRightX;
					aVtx[1].Pos_MS.y += fTopY;
					aVtx[2].Pos_MS.x += fLeftX;
					aVtx[2].Pos_MS.y += fBottomY;
					aVtx[3].Pos_MS.x += fRightX;
					aVtx[3].Pos_MS.y += fBottomY;

					fdraw_PrimList( FDRAW_PRIMTYPE_TRISTRIP, aVtx, 4 );
				}
			}
		}
	}

	// Draw the warning text...
	if( m_nUnplugState == UNPLUG_STATE_NONE ) {
		FTextArea_t *pTextArea = ftext_GetAttributes( Player_aPlayer[ m_nPossessionPlayerIndex ].m_hTextBoxOutOfRange );

		f32 fUnit = fmath_PositiveCos( m_fUnitOutOfRangeWarningMsg * FMATH_2PI + FMATH_HALF_PI );
		fUnit = 1.0f - fUnit * fUnit;
		pTextArea->oColorForeground.Set( 0.8f, 0.8f, 0.8f, FMATH_FPOT( fUnit, 0.2f, 1.0f ) );

		// Set the active viewport to the safe-area-truncated viewport (used for the hud)
		FViewport_t *pViewport = fviewport_GetActive();
		fviewport_SetActive(Player_aPlayer[m_nPossessionPlayerIndex].m_pViewportSafeOrtho3D);
		ftext_PrintString( Player_aPlayer[ m_nPossessionPlayerIndex ].m_hTextBoxOutOfRange, Game_apwszPhrases[ GAMEPHRASE_LOSING_SIGNAL ] );
		fviewport_SetActive(pViewport);
		pTextArea->oColorBackground.Set( 0.5f, 0.1f, 0.1f, 0.3f );
		pTextArea->oColorBorder.Set( 0.5f, 0.1f, 0.1f, 0.5f );

		ftext_GetAttributes( Player_aPlayer[ m_nPossessionPlayerIndex ].m_hTextBoxOutOfRange )->bVisible = TRUE;
	} else if( (m_nUnplugState >= UNPLUG_STATE_STATIC_SHRINK_Y) && (m_nUnplugState <= UNPLUG_STATE_DOT_FADE) ) {
		// Set the active viewport to the safe-area-truncated viewport (used for the hud)
		FViewport_t *pViewport = fviewport_GetActive();
		fviewport_SetActive(Player_aPlayer[m_nPossessionPlayerIndex].m_pViewportSafeOrtho3D);
		ftext_PrintString( Player_aPlayer[ m_nPossessionPlayerIndex ].m_hTextBoxOutOfRange, Game_apwszPhrases[ GAMEPHRASE_SIGNAL_LOST ] );
		fviewport_SetActive(pViewport);
		ftext_GetAttributes( Player_aPlayer[ m_nPossessionPlayerIndex ].m_hTextBoxOutOfRange )->bVisible = TRUE;
	}
}


void CBot::_EnablePossessionTextBox( BOOL bEnable ) {
	if( m_nPossessionPlayerIndex >= 0 ) {
		ftext_GetAttributes( Player_aPlayer[ m_nPossessionPlayerIndex ].m_hTextBoxOutOfRange )->bVisible = bEnable;
	}
}


void CBot::_InitPossessionOutOfRangeWarningTextBox( void ) {
	FTextArea_t *pTextArea;

	pTextArea = ftext_GetAttributes( Player_aPlayer[ m_nPossessionPlayerIndex ].m_hTextBoxOutOfRange );

	pTextArea->bVisible = FALSE;
	pTextArea->fUpperLeftX = 0.5f - 0.5f*_POSSESSION_OUTOFRANGE_TBOX_WIDTH2;
	pTextArea->fLowerRightX = 0.5f + 0.5f*_POSSESSION_OUTOFRANGE_TBOX_WIDTH2;
	pTextArea->fUpperLeftY = 0.5f - 0.5f*_POSSESSION_OUTOFRANGE_TBOX_HEIGHT;
	pTextArea->fLowerRightY = 0.5f + 0.5f*_POSSESSION_OUTOFRANGE_TBOX_HEIGHT;
	pTextArea->fNumberOfLines = 1;
	pTextArea->fBorderThicknessX = _POSSESSION_OUTOFRANGE_BORDER_THICKNESS;
	pTextArea->fBorderThicknessY = _POSSESSION_OUTOFRANGE_BORDER_THICKNESS;
	pTextArea->oColorForeground.OpaqueWhite();
	pTextArea->oColorBackground.Set( 0.0f, 0.0f, 0.0f, 0.2f );
	pTextArea->oColorBorder.Set( 0.0f, 0.0f, 0.0f, 0.4f );
	pTextArea->ohFont = _POSSESSION_OUTOFRANGE_FONT_CODE;
	pTextArea->oHorzAlign = FTEXT_HORZ_ALIGN_CENTER;
}


void CBot::_InitTransmissionLostTextBox( void ) {
	FTextArea_t *pTextArea;

	pTextArea = ftext_GetAttributes( Player_aPlayer[ m_nPossessionPlayerIndex ].m_hTextBoxOutOfRange );

	pTextArea->bVisible = FALSE;
	pTextArea->fUpperLeftX = 0.5f - 0.5f*_POSSESSION_SIGNALLOST_TBOX_WIDTH;
	pTextArea->fLowerRightX = 0.5f + 0.5f*_POSSESSION_SIGNALLOST_TBOX_WIDTH;
	pTextArea->fUpperLeftY = 0.5f - 0.5f*_POSSESSION_SIGNALLOST_TBOX_HEIGHT;
	pTextArea->fLowerRightY = 0.5f + 0.5f*_POSSESSION_SIGNALLOST_TBOX_HEIGHT;
	pTextArea->fNumberOfLines = 1;
	pTextArea->fBorderThicknessX = _POSSESSION_SIGNALLOST_BORDER_THICKNESS;
	pTextArea->fBorderThicknessY = _POSSESSION_SIGNALLOST_BORDER_THICKNESS;
	pTextArea->oColorForeground.OpaqueWhite();
	pTextArea->oColorBackground.Set( 0.0f, 0.0f, 0.0f, 0.2f );
	pTextArea->oColorBorder.Set( 0.0f, 0.0f, 0.0f, 0.4f );
	pTextArea->ohFont = _POSSESSION_SIGNALLOST_FONT_CODE;
	pTextArea->oHorzAlign = FTEXT_HORZ_ALIGN_CENTER;
}


// Move bot head in response to head look requests.
void CBot::HeadLookUpdate( CFMtx43A &rOutputMtx, const CFMtx43A &rParentMtx, const CFMtx43A &rHeadMtx, BOOL bNegateLook ) {
	CFMtx43A TempMtx;
	CFQuatA TargetQuat;

	// construct standard head matrix
	TempMtx.Mul( rParentMtx, rHeadMtx );

	// attempt to swivel head to requested world space location
	if( HeadShouldLook() ) {
		CFVec3A vUp, vTorsoFront, vHeadFront, vLook;
		f32 fPitchCos, fYawCos;

		// first, gather data needed to determine if it is
		// currently possible for head to look at the
		// requested location

		// construct normalized up vector
		vUp = rParentMtx.m_vUp;
		vUp.Unitize();

		// construct normalized torso front vector
		vTorsoFront = rParentMtx.m_vFront;
		vTorsoFront.Unitize();

		// construct normalized head front vector
		vHeadFront = TempMtx.m_vFront;
		vHeadFront.Unitize();

		// construct normalized look at vector
		vLook = m_vHeadLookPoint_WS;
		vLook.Sub( TempMtx.m_vPos );
		vLook.Unitize();
		if( bNegateLook ) {
			// negation option because Glitch anims are rotated 180 degrees...
			vLook.Negate();
		}

		// add hysteresis to reduce chance of head popping in and out of
		// look mode when at extremes of constraint.
		if( HeadIsLooking() ) {
			fPitchCos = m_fHeadLookHystPitchCos;
			fYawCos = m_fHeadLookHystYawCos;
		} else {
			fPitchCos = m_fHeadLookMaxPitchCos;
			fYawCos = m_fHeadLookMaxYawCos;
		}

		// Next, test to see if requested look location is within head swiveling range
		// (and that the look mode is not being interrupted)
		if( vLook.InYawPitchRange( vTorsoFront, vUp, fYawCos, fPitchCos ) && !HeadLookInterrupt() ) {
			// indicate that head is looking and
			// reset slerp-to-target amount

			if( !HeadIsLooking() ) {
				HeadSetLooking();
				m_fHeadLookSlerp = 0.0f;
				m_HeadLookOriginQuat = m_HeadLookCurrentQuat;
			}

			// make slerp target quat for difference
			// between current head look dirction and 
			// target head look direction.
			TargetQuat.BuildQuat( vHeadFront, vLook );
		} else {
			// head is not in swiveling range.
			// indicate that head is not looking
			// and reset slerp-to-target amount.
			if( HeadIsLooking() ) {
				HeadClearLooking();
				m_fHeadLookSlerp = 0.0f;
				m_HeadLookOriginQuat = m_HeadLookCurrentQuat;
			}

			// make slerp target quat to return
			// head to default look direction
			TargetQuat.Identity();
		}

	} else {
		// if we are here then head look is not 
		// currently being requested

		// indicate that head is not looking
		// and reset slerp-to-target amount
		if( HeadIsLooking() ) {
			HeadClearLooking();
			m_fHeadLookSlerp = 0.0f;
			m_HeadLookOriginQuat = m_HeadLookCurrentQuat;
		}

		// make slerp target quat to return
		// head to default look direction
		TargetQuat.Identity();
	}

	// perform head look slerp if head look is engaged, or
	// if returning to non-head-look orientation
	if( HeadIsLooking() || m_fHeadLookSlerp < 1.0f ) {
		CFQuatA StepQuat;

		// slerp to target look direction if necessary
		if( m_fHeadLookSlerp < 1.0f ) {
			// transition to new target orientation
			m_fHeadLookSlerp += BOT_HEAD_LOOK_INV_TIME * FLoop_fPreviousLoopSecs;
			FMATH_CLAMPMAX( m_fHeadLookSlerp, 1.0f );
			StepQuat.ReceiveSlerpOf( fmath_UnitLinearToSCurve( m_fHeadLookSlerp ), m_HeadLookOriginQuat, TargetQuat );
		} else {
			// transition finished, continually track target point
//			StepQuat = targetQuat;
			StepQuat.ReceiveSlerpedStep( FMATH_PI * FLoop_fPreviousLoopSecs, m_HeadLookCurrentQuat, TargetQuat );
		}

		// remember current quat.  It will become the origin
		// quat when a new target is requested.
		m_HeadLookCurrentQuat = StepQuat;

		// mult bone matrix by quat
		StepQuat.MulPoint( rOutputMtx.m_vRight, TempMtx.m_vRight );
		StepQuat.MulPoint( rOutputMtx.m_vUp, TempMtx.m_vUp );
		StepQuat.MulPoint( rOutputMtx.m_vFront, TempMtx.m_vFront );

		// add in bone position
		rOutputMtx.m_vPos = TempMtx.m_vPos;
	
	} else {
		// copy standard head matrix to output
		rOutputMtx = TempMtx;
	}

}


// Process head lock requests from entity control.
void CBot::HandleHeadLook( void ) {
	if( m_nControlsBot_Flags & CBotControl::FLAG_USE_HEADLOOK ) {
		HeadLook();
	} else {
		HeadStopLook();
	}

	if( m_nControlsBot_Flags & CBotControl::FLAG_NEW_HEADLOOK_LOCATION ) {
		m_nControlsBot_Flags &= ~CBotControl::FLAG_NEW_HEADLOOK_LOCATION;
		SetHeadLookLocation( m_vControlBot_HeadLookPoint_WS );
	}
}


void CBot::StartReverseFacingMode( void ) {
	FASSERT( IsCreated() );

	if( !(m_nBotFlags & BOTFLAG_REVERSE_FACING) ) {
		FMATH_SETBITMASK( m_nBotFlags, BOTFLAG_REVERSE_FACING );

		AbortScopeMode();
	}
}


// handles bot turning around to face backwards relative to mount
void CBot::HandleReverseFacing( void ) {
	if( m_nBotFlags & BOTFLAG_REVERSE_FACING ) {
		// if reverse facing mode is engaged

		// hide reticle if displayed
		ReticleEnable(FALSE);

		if( m_fReverseFacingYaw_MS < FMATH_DEG2RAD( 180.0f ) ) {
			// if turn to reverse facing is not yet completed,
			// increment turn angle and clamp at maximum
			m_fReverseFacingYaw_MS += BOT_REVERSE_FACING_RATE * FLoop_fPreviousLoopSecs;
			FMATH_CLAMPMAX( m_fReverseFacingYaw_MS, FMATH_DEG2RAD( 180.0f ) );
		}

		if( m_fReverseFacingYaw_MS > FMATH_DEG2RAD( 90.0f ) ) {
			CFCamera *pCurCam = gamecam_GetActiveCamera();
			CFVec3A vCamPos;
			vCamPos.Set( *(pCurCam->GetPos()) );

			// if more than halfway to full turn

			if( HeadShouldLook() ) {
				// if head look mode already engaged (via the else statement below)
				// then update look location so bot head tracks active camera
				UpdateHeadLookLocation( vCamPos );
			} else {
				// turn on head look mode and track active camera
				SetHeadLookLocation( vCamPos );
				HeadLook();
			}
		}
	} else {
		// reverse facing mode is disengaged

		if( m_fReverseFacingYaw_MS > 0.0f ) {
			// if turn back to front facing is not yet completed
			// decrement turn angle and clamp at minimum
			m_fReverseFacingYaw_MS -= BOT_REVERSE_FACING_RATE * FLoop_fPreviousLoopSecs;
			FMATH_CLAMPMIN( m_fReverseFacingYaw_MS, 0.0f );

			if( m_fReverseFacingYaw_MS == 0.0f ) {
				// show reticle if hidden
				if( !IsReticleEnabled() ) {
					ReticleEnable( TRUE );
				}

			}

			if( HeadShouldLook() ) {
				// turn off head look
				HeadStopLook();
			}
		}
	}
}


BOOL CBot::UserAnim_BeginAbortLock( AbortCompleteCB *pAbortReqCompleteCB ) {
	if( UserAnim_IsLocked() && UserAnim_IsLockAbortable() && !UserAnim_IsAborting() ) {
		m_pAbortReqCompleteCB = pAbortReqCompleteCB;
		m_nUASlotFlags |= UASLOTFLAG_ABORTING;
		m_fUnitAbortUserAnimLock = 1.0f;

		if( m_pUserAnimAbortCB ) {
			if( m_pUserAnimAbortCB(this) ) {
				//  CB Said they were done, so we're done...
			   UserAnim_UnLock();
			}
		}

		return TRUE;
	}

	return FALSE;
}


void CBot::UserAnim_LockWork( void ){
	if( m_nUASlotFlags & UASLOTFLAG_ABORTING ) {
		if( m_nUASlotFlags & UASLOTFLAG_ABORTWITHAUTOBLENDOUT ) {
			m_fUnitAbortUserAnimLock -= _UASLOTABORTLOCK_DELTA_PER_SEC*FLoop_fPreviousLoopSecs;
			FMATH_CLAMP(m_fUnitAbortUserAnimLock, 0.0f, 1.0f);
		} else if( m_pUserAnimAbortCB ) {
			if( m_pUserAnimAbortCB(this) ) {
				// CB Said they were done, so we're done...
				m_fUnitAbortUserAnimLock = 0.0f;
			}
		} else {
			m_fUnitAbortUserAnimLock = 0.0f;
		}


		if( m_fUnitAbortUserAnimLock == 0.0f ) {
			if( !(m_nUASlotFlags & UASLOTFLAG_ABORTWITHAUTOBLENDOUT) && m_pUserAnimAbortCB ) {
				// Call CB once if autoblendout was on...
				m_pUserAnimAbortCB(this);
			}

			// Now, free the lock...
			UserAnim_UnLock();
			if( m_pAbortReqCompleteCB ) {
				m_pAbortReqCompleteCB(this);
			}
		}
	}
}


BOOL CBot::UserAnim_Lock( void* pUserAnimSlotLockId, BOOL bAbortable /*= FALSE*/, BOOL bAutoBlendOut, UserAnimAbortCB *pUserAnimAbortCB ) {
	if( !UserAnim_IsLocked() ) {
		m_pUserAnimSlotLockId = pUserAnimSlotLockId;
		m_nUASlotFlags |= UASLOTFLAG_LOCKED;
		m_nUASlotFlags |= bAbortable*UASLOTFLAG_ABORTABLE;
		m_nUASlotFlags |= bAutoBlendOut*UASLOTFLAG_ABORTWITHAUTOBLENDOUT;
		m_pUserAnimAbortCB = pUserAnimAbortCB;

		AbortScopeMode();
		//EnableAiming( FALSE );

		return TRUE;
	}

	return FALSE;
}


void CBot::UserAnim_UnLock( void ) {
	m_nUASlotFlags &= ~UASLOTFLAG_LOCKED; 
	m_nUASlotFlags &= ~UASLOTFLAG_ABORTABLE;
	m_nUASlotFlags &= ~UASLOTFLAG_ABORTING;
	m_nUASlotFlags &= ~UASLOTFLAG_ABORTWITHAUTOBLENDOUT;
	m_pUserAnimAbortCB = NULL;
	m_pAbortReqCompleteCB = NULL;
	m_fUnitAbortUserAnimLock = 0.0f;
//	EnableAiming( TRUE );
}


BOOL CBot::Sleep( void ) {
	if( CanSleep()) {
		m_nSleepState = BOTSLEEPSTATE_INIT;
		return TRUE;
	}

	return FALSE;
}


BOOL CBot::WakeUp( void ) {
	if( (m_nSleepState == BOTSLEEPSTATE_DOZE_LOOP) || (m_nSleepState == BOTSLEEPSTATE_NAPJERK) ) {
		m_nSleepState = BOTSLEEPSTATE_WAKE;
		return TRUE;
	}

	return FALSE;
}


BOOL CBot::NapJerk( BOOL bAllowAutoWakeup ) {
	BOOL bDidIt = FALSE;
	f32 fGameTime = FLoop_nTotalLoopTicks*FLoop_fSecsPerTick;

	if( m_nSleepState == BOTSLEEPSTATE_DOZE_LOOP &&	bAllowAutoWakeup &&	(fGameTime < m_fNapJerkTimeOut)) {
		m_nSleepState = BOTSLEEPSTATE_WAKE;
		bDidIt = TRUE;
	} else if( m_nSleepState == BOTSLEEPSTATE_DOZE_LOOP ) {
		m_nSleepState = BOTSLEEPSTATE_NAPJERK;
		SetControlValue( ASI_NAPJERK, 0.0f );
		UpdateTime(ASI_NAPJERK, 0.0f);
		_bPlayedSoundThisJerk = FALSE;

		m_fUnitNapJerk = 0.0f;
		m_fNapJerkTimeOut = fGameTime+2.0f;
		bDidIt = TRUE;
	}

	return bDidIt;
}


// saves state for checkpoint
BOOL CBot::CheckpointSave( void )
{
	// save base class data
	CEntity::CheckpointSave();
 
	// save bot class data.
	// order must match load order below
	CFCheckPoint::SaveData( m_MountPos_WS );
	CFCheckPoint::SaveData( m_fMountYaw_WS );
	CFCheckPoint::SaveData( m_fMountPitch_WS );
	CFCheckPoint::SaveData( m_fMountYawSin_WS );
	CFCheckPoint::SaveData( m_fMountYawCos_WS );
	CFCheckPoint::SaveData( m_MountUnitFrontXZ_WS );
	CFCheckPoint::SaveData( m_fRunMultiplier );
	CFCheckPoint::SaveData( m_fOORunMultiplier );
	CFCheckPoint::SaveData( m_fJumpMultiplier );
	CFCheckPoint::SaveData( m_fGravityMultiplier );
	CFCheckPoint::SaveData( (m_nBotFlags&~BOTFLAG_GLUED_TO_PARENT) );
	CFCheckPoint::SaveData( m_nBotFlags2);
	CFCheckPoint::SaveData( (u32&) m_uLifeCycleState );
	CFCheckPoint::SaveData( m_uBotDeathFlags );
	CFCheckPoint::SaveData( (u32&) m_pInventory );
	CFCheckPoint::SaveData( (u32&) m_eSpotLightState );
	CFCheckPoint::SaveData( (u32&) m_nSleepState );

	if( m_pInventory ) {
		CFCheckPoint::SaveData( (void*) m_pInventory, sizeof( CInventory ) );
	}

	f32 fSecsTillPowerUp;
	if( m_nPowerState == POWERSTATE_POWERED_UP ) {
		fSecsTillPowerUp = 0.0f;
	} else if( m_nPowerState == POWERSTATE_POWERED_DOWN ) {
		fSecsTillPowerUp = m_fPowerOffOnTime;
	} else {
		// in some halfway power state, set us to power up in a second...
		fSecsTillPowerUp = 1.0f;
	}
	CFCheckPoint::SaveData( fSecsTillPowerUp );

	// in pieces stuff
	CFCheckPoint::SaveData( m_bInPieces );
	CFCheckPoint::SaveData( m_fTimeInPieces );
	CFCheckPoint::SaveData( m_bColWithPieces );

	CWeaponRecruiter::BotCheckpointSaved( this );

	FASSERT( m_hSaveData[CFCheckPoint::GetCheckPoint()] != CFCheckPoint::NULL_HANDLE );
	return TRUE;
}


// loads state for checkpoint
void CBot::CheckpointRestore( void )
{
	CInventory *pInventory;

	BOOL bPreInWorld = IsInWorld();

	BotLifeCycle_e uPreRestoreLifeCycleState = m_uLifeCycleState;

	// load base class data
	FASSERT( m_hSaveData[CFCheckPoint::GetCheckPoint()] != CFCheckPoint::NULL_HANDLE );
	CEntity::CheckpointRestore();

	// load bot class data.
	// order must match save order above
	CFCheckPoint::LoadData( m_MountPos_WS );
	CFCheckPoint::LoadData( m_fMountYaw_WS );
	CFCheckPoint::LoadData( m_fMountPitch_WS );
	CFCheckPoint::LoadData( m_fMountYawSin_WS );
	CFCheckPoint::LoadData( m_fMountYawCos_WS );
	CFCheckPoint::LoadData( m_MountUnitFrontXZ_WS );
	CFCheckPoint::LoadData( m_fRunMultiplier );
	CFCheckPoint::LoadData( m_fOORunMultiplier );
	CFCheckPoint::LoadData( m_fJumpMultiplier );
	CFCheckPoint::LoadData( m_fGravityMultiplier );
	CFCheckPoint::LoadData( m_nBotFlags );
	CFCheckPoint::LoadData( m_nBotFlags2);
	CFCheckPoint::LoadData( (u32&) m_uLifeCycleState );
	CFCheckPoint::LoadData( m_uBotDeathFlags );
	CFCheckPoint::LoadData( (u32&) pInventory );
	CFCheckPoint::LoadData( (u32&) m_eSpotLightState );
	CFCheckPoint::LoadData( (u32&) m_nSleepState );

	FMATH_CLEARBITMASK( m_nBotFlags, BOTFLAG_PLAY_FIRE1_ANIM );
	FMATH_CLEARBITMASK( m_nBotFlags2, BOTFLAG2_CAN_DOUBLE_JUMP );

	if( m_pInventory ) {
		CFCheckPoint::LoadData( (void*) m_pInventory, sizeof( CInventory ) );
	}

	f32 fSecsTillPowerUp;
	CFCheckPoint::LoadData( fSecsTillPowerUp );
	if( fSecsTillPowerUp > 0.0f ) {
		Power_SetState( FALSE, fSecsTillPowerUp );
	} else {
		Power_SetState( TRUE );
	}

	// restore in pieces state
	m_bFallDown = FALSE;
	m_bFallAnimDone = TRUE;
	m_bOnGround = FALSE;
	CFCheckPoint::LoadData( m_bInPieces );
	CFCheckPoint::LoadData( m_fTimeInPieces );
	CFCheckPoint::LoadData( m_bColWithPieces );

	if( IsSleeping() ) {
		m_nSleepState = BOTSLEEPSTATE_INIT;
	}
 
	// update position of world mesh
	m_pWorldMesh->m_Xfm.BuildFromMtx( m_MtxToWorld );
	m_pWorldMesh->UpdateTracker();
	if( !IsInWorld() || !IsDrawEnabled() ) {
		// update tracker adds mesh to world automatically.
		// Here we re-remove the mesh if appropriate.
		m_pWorldMesh->RemoveFromWorld();
	}

	if (m_pActiveTalkInst) {
		// stop any active bot talk
		CTalkSystem2::TerminateActiveTalk(m_pActiveTalkInst);
		m_pActiveTalkInst = NULL;
	}

	if( uPreRestoreLifeCycleState != BOTLIFECYCLE_LIVING && m_uLifeCycleState == BOTLIFECYCLE_LIVING ) {
		// bot now dead was alive at last checkpoint, so resurrect him.
		UnDie();
	} else if( m_uLifeCycleState != BOTLIFECYCLE_LIVING ) {
		// bot is dead upon restore

		if( m_uBotDeathFlags & BOTDEATHFLAG_PLAYDEATHANIM ) {
			// bot was not blown to bits, but instead persists in world as a dead body.

			// Force restored bots that are dead to look dead.
			// Needs more work, re-triggers the full death anim.
			m_uLifeCycleState = BOTLIFECYCLE_LIVING;
			Die( FALSE, FALSE );
		}		
	}

	// set a bunch of vars back to the neutral settings
	ResetToNeutral();	

	// restore spotlight state
	switch( m_eSpotLightState )
	{
		case LIGHT_STATE_OFF:
		case LIGHT_STATE_TURNING_OFF:
			SetSpotLightOff( TRUE );
			break;

		case LIGHT_STATE_ON:
		case LIGHT_STATE_TURNING_ON:
			SetSpotLightOn( TRUE );
			break;
	}

	// Swarmers are no longer attached
	m_uNumAttachedSwarmers = 0;

	if( m_nPossessionPlayerIndex >= 0 ) {
		Player_aPlayer[ m_nPossessionPlayerIndex ].ZeroControls();
	}

	CWeaponRecruiter::BotCheckpointRestored( this );

	DataPort_Reserve( FALSE );

	if( m_pCableHook ) {
		m_pCableHook->m_pCable->DetachHook( m_pCableHook, this, FALSE );
		m_pCableHook = NULL;
	}

	//if bot was in pieces at save time, set bot in pieces immediately upon restore.
	if( m_bInPieces ) {
		f32 fTime;

		// BreakIntoPieces() uses current value of m_fTimeInPieces to decide 
		// if bot should be assembled or disassembled. We want the bot to be
		// disassembled, so we must make sure m_fTimeInPieces is not set to -1.0 
		// before calling BreakIntoPieces().
		fTime = m_fTimeInPieces;
		m_fTimeInPieces = 0.0f;
		BreakIntoPieces( fTime, TRUE /*bImmediately*/ );
	}


	m_pCablePrev = NULL;
	m_fCableGrabTime = BOT_ZIPLINE_GRAB_TIME;
}

void CBot::PreUserRender( void )
{
	// If this bot is recruited, modify the recruiter icon accordingly
	// Note: user bot cannot be recruited in single player, hence no check
	if ( Recruit_IsRecruited() ) {
		CWeaponRecruiter::SetUserViewMode( this, TRUE );
	}
}

void CBot::PostUserRender( void )
{
	// If this bot is recruited, modify the recruiter icon accordingly
	// Note: user bot cannot be recruited in single player, hence no check
	if ( Recruit_IsRecruited() ) {
		CWeaponRecruiter::SetUserViewMode( this, FALSE );
	}
}

void CBot::DebugRender( f32 fScreenTextX, f32 fScreenTextY ) {
	f32 fLineHeight = 0.0f;
	f32 fLine = 0.0f;

	CFVec3A CollSpherePos;
	if( m_pBotInfo_Gen ) {
		CollSpherePos.x = m_pBotInfo_Gen->fCollSphere1X_MS + m_MountPos_WS.x;
		CollSpherePos.y = ComputeCollSphereY() + m_MountPos_WS.y - 1.0f;
		CollSpherePos.z = m_pBotInfo_Gen->fCollSphere1Z_MS + m_MountPos_WS.z;

		fdraw_FacetedWireSphere( &(CollSpherePos.v3), m_pBotInfo_Gen->fCollSphere1Radius_MS, 2, 2, &FColor_MotifBlue);
		fdraw_FacetedCylinder(&(m_MountPos_WS.v3), &(CFVec3A::m_UnitAxisY.v3), m_fCollCylinderRadius_WS, m_fCollCylinderHeight_WS, &FColor_MotifRed, 4);
	}

	if( m_pDisguisedBot && m_pDisguisedBot->IsInWorld() ) {
		ftext_DebugPrintf( fScreenTextX, fScreenTextY+fLine, "~w1 ~c00990099 Suspicious of %s, %0.3f", m_pDisguisedBot->Name(), 1.0f-m_fUnitDisguised);
		fLine += fLineHeight;
	}
}


void CBot::_LoadSoundResource( u32 uBankToLoad ) {
	FASSERT( uBankToLoad == BOTSOUND_LIGHT_WEIGHT    || 
			 uBankToLoad == BOTSOUND_MEDIUM_WEIGHT   || 
			 uBankToLoad == BOTSOUND_HEAVY_WEIGHT    ||
			 uBankToLoad == BOTSOUND_CORROSIVE_WEIGHT||
			 uBankToLoad == BOTSOUND_ZOMBIEBOSS_WEIGHT ||
			 uBankToLoad == BOTSOUND_VEHICLE_WEIGHT);

	if( uBankToLoad == BOTSOUND_LIGHT_WEIGHT ) {
		m_pBotInfo_Sound = &m_BotInfo_SoundLight;
	} else if( uBankToLoad == BOTSOUND_MEDIUM_WEIGHT ) {
		m_pBotInfo_Sound = &m_BotInfo_SoundMedium;
	} else if( uBankToLoad == BOTSOUND_HEAVY_WEIGHT ) {
		m_pBotInfo_Sound = &m_BotInfo_SoundHeavy;
	} else if( uBankToLoad == BOTSOUND_CORROSIVE_WEIGHT ) {
		m_pBotInfo_Sound = &m_BotInfo_SoundCorrosive;
	} else if( uBankToLoad == BOTSOUND_ZOMBIEBOSS_WEIGHT) {
		m_pBotInfo_Sound = &m_BotInfo_SoundZombieBoss;
	} else if( uBankToLoad == BOTSOUND_VEHICLE_WEIGHT) {
		m_pBotInfo_Sound = &m_BotInfo_SoundVehicle;
	}

	// Bank has already been loaded
	if( m_uBotSoundFlags & uBankToLoad ) {
		return;
	}

	m_uBotSoundFlags |= uBankToLoad;

	// Third entry of the table is always the same for all sounds
	// !!!!!!!!!!!!!!!!!!
	// Order must match the order of the SurfaceType_e enum.
	// !!!!!!!!!!!!!!!!!!
	FGameDataMap_t GameDataMapSounds[] = {
		"Concrete",		m_aBotInfoVocab_Sounds, sizeof( m_BotInfo_SoundMedium.ahStepSound[0] ), 0,
		"Metal",   		m_aBotInfoVocab_Sounds,	sizeof( m_BotInfo_SoundMedium.ahStepSound[0] ), 0,
		"Metal_Grate",	m_aBotInfoVocab_Sounds,	sizeof( m_BotInfo_SoundMedium.ahStepSound[0] ), 0,
		"Dirt",			m_aBotInfoVocab_Sounds,	sizeof( m_BotInfo_SoundMedium.ahStepSound[0] ), 0,
		"Rock",			m_aBotInfoVocab_Sounds,	sizeof( m_BotInfo_SoundMedium.ahStepSound[0] ), 0,
		"Glass",		m_aBotInfoVocab_Sounds,	sizeof( m_BotInfo_SoundMedium.ahStepSound[0] ), 0,
		"Composite",	m_aBotInfoVocab_Sounds,	sizeof( m_BotInfo_SoundMedium.ahStepSound[0] ), 0,
		"Electronics",	m_aBotInfoVocab_Sounds,	sizeof( m_BotInfo_SoundMedium.ahStepSound[0] ), 0,
		"Junk",			m_aBotInfoVocab_Sounds,	sizeof( m_BotInfo_SoundMedium.ahStepSound[0] ), 0,
		"Water",		m_aBotInfoVocab_Sounds,	sizeof( m_BotInfo_SoundMedium.ahStepSound[0] ), 0,
		"Goop",			m_aBotInfoVocab_Sounds,	sizeof( m_BotInfo_SoundMedium.ahStepSound[0] ), 0,
		"Acid",			m_aBotInfoVocab_Sounds,	sizeof( m_BotInfo_SoundMedium.ahStepSound[0] ), 0,
		"Force_Field",	m_aBotInfoVocab_Sounds,	sizeof( m_BotInfo_SoundMedium.ahStepSound[0] ), 0,
		NULL
	};
	u32 i;

	if( uBankToLoad == BOTSOUND_LIGHT_WEIGHT ) {
		for( i = 0; i < SURFACE_TYPE_COUNT; ++i ) {
			GameDataMapSounds[i].pDestTableData = (void *)&m_BotInfo_SoundLight.ahStepSound[i][0];
		}

		if( !fresload_Load( FSNDFX_RESTYPE, _SOUND_BANK_LIGHT_WEIGHT ) ) {
			DEVPRINTF( "CBot::_LoadSoundResources(): Could not load sound effect bank '%s'\n", _SOUND_BANK_LIGHT_WEIGHT );
		}

		if( !fgamedata_ReadFileUsingMap( GameDataMapSounds, _SOUND_NAMES_LIGHT_CSV ) ) {
			DEVPRINTF( "CBot::_LoadSoundResources(): Could not load '%s'\n", _SOUND_NAMES_LIGHT_CSV);
		}
	} 

	if( uBankToLoad == BOTSOUND_MEDIUM_WEIGHT ) {
		for( i = 0; i < SURFACE_TYPE_COUNT; ++i ) {
			GameDataMapSounds[i].pDestTableData = (void *)&m_BotInfo_SoundMedium.ahStepSound[i][0];
		}

		if( !fresload_Load( FSNDFX_RESTYPE, _SOUND_BANK_MEDIUM_WEIGHT ) ) {
			DEVPRINTF( "CBot::_LoadSoundResources(): Could not load sound effect bank '%s'\n", _SOUND_BANK_MEDIUM_WEIGHT );
		}

		if( !fgamedata_ReadFileUsingMap( GameDataMapSounds, _SOUND_NAMES_MEDIUM_CSV ) ) {
			DEVPRINTF( "CBot::_LoadSoundResources(): Could not load '%s'\n", _SOUND_NAMES_MEDIUM_CSV);
		}
	}

	if( uBankToLoad == BOTSOUND_HEAVY_WEIGHT ) {
		for( i = 0; i < SURFACE_TYPE_COUNT; ++i ) {
			GameDataMapSounds[i].pDestTableData = (void *)&m_BotInfo_SoundHeavy.ahStepSound[i][0];
		}

		if( !fresload_Load( FSNDFX_RESTYPE, _SOUND_BANK_HEAVY_WEIGHT ) ) {
			DEVPRINTF( "CBot::_LoadSoundResources(): Could not load sound effect bank '%s'\n", _SOUND_BANK_HEAVY_WEIGHT );
		}

		if( !fgamedata_ReadFileUsingMap( GameDataMapSounds, _SOUND_NAMES_HEAVY_CSV ) ) {
			DEVPRINTF( "CBot::_LoadSoundResources(): Could not load '%s'\n", _SOUND_NAMES_HEAVY_CSV);
		}
	}

	if( uBankToLoad == BOTSOUND_CORROSIVE_WEIGHT ) {
		for( i = 0; i < SURFACE_TYPE_COUNT; ++i ) {
			GameDataMapSounds[i].pDestTableData = (void *)&m_BotInfo_SoundCorrosive.ahStepSound[i][0];
		}

		if( !fresload_Load( FSNDFX_RESTYPE, _SOUND_BANK_CORROSIVE_WEIGHT ) ) {
			DEVPRINTF( "CBot::_LoadSoundResources(): Could not load sound effect bank '%s'\n", _SOUND_BANK_CORROSIVE_WEIGHT );
		}

		if( !fgamedata_ReadFileUsingMap( GameDataMapSounds, _SOUND_NAMES_CORROSIVE_CSV ) ) {
			DEVPRINTF( "CBot::_LoadSoundResources(): Could not load '%s'\n", _SOUND_NAMES_CORROSIVE_CSV);
		}
	}
	
	if( uBankToLoad == BOTSOUND_ZOMBIEBOSS_WEIGHT ) {
		for( i = 0; i < SURFACE_TYPE_COUNT; ++i ) {
			GameDataMapSounds[i].pDestTableData = (void *)&m_BotInfo_SoundZombieBoss.ahStepSound[i][0];
		}

		if( !fresload_Load( FSNDFX_RESTYPE, _SOUND_BANK_ZOMBIEBOSS_WEIGHT) ) {
			DEVPRINTF( "CBot::_LoadSoundResources(): Could not load sound effect bank '%s'\n", _SOUND_BANK_ZOMBIEBOSS_WEIGHT );
		}

		if( !fgamedata_ReadFileUsingMap( GameDataMapSounds, _SOUND_NAMES_ZOMBIEBOSS_CSV ) ) {
			DEVPRINTF( "CBot::_LoadSoundResources(): Could not load '%s'\n", _SOUND_NAMES_ZOMBIEBOSS_CSV);
		}
	}

	if( uBankToLoad == BOTSOUND_VEHICLE_WEIGHT ) {
		for( i = 0; i < SURFACE_TYPE_COUNT; ++i ) {
			GameDataMapSounds[i].pDestTableData = (void *)&m_BotInfo_SoundVehicle.ahStepSound[i][0];
		}

		if( !fresload_Load( FSNDFX_RESTYPE, _SOUND_BANK_VEHICLE_WEIGHT) ) {
			DEVPRINTF( "CBot::_LoadSoundResources(): Could not load sound effect bank '%s'\n", _SOUND_BANK_VEHICLE_WEIGHT );
		}

		if( !fgamedata_ReadFileUsingMap( GameDataMapSounds, _SOUND_NAMES_VEHICLE_CSV ) ) {
			DEVPRINTF( "CBot::_LoadSoundResources(): Could not load '%s'\n", _SOUND_NAMES_VEHICLE_CSV);
		}
	}
}


void CBot::UninitLevel( void ) {
	m_uBotSoundFlags = 0;
	fang_MemZero(&m_BotInfo_SoundLight, sizeof(m_BotInfo_SoundLight));
	fang_MemZero(&m_BotInfo_SoundMedium, sizeof(m_BotInfo_SoundMedium));
	fang_MemZero(&m_BotInfo_SoundHeavy, sizeof(m_BotInfo_SoundHeavy));
	fang_MemZero(&m_BotInfo_SoundCorrosive, sizeof(m_BotInfo_SoundCorrosive));
}


BOOL CBot::WeaponsCreate( u32 nSlot, u32 nWpnType, u32 nDuplicates, u32 nUpgradeLevel ) {
	FASSERT( nSlot < 2 );
	FASSERT( m_apWeapon[nSlot] == NULL );
	FASSERT( nDuplicates <= BOT_MAX_DUPLICATE_WPNS ); 

	u32 i;
	BOOL bCreatedOK = TRUE;

	m_anNumDupWeapons[nSlot] = nDuplicates;

	switch( nWpnType ) {
		case CWeapon::WEAPON_TYPE_CHAINGUN:
			m_apWeapon[nSlot]		= fnew CWeaponChaingun();
			bCreatedOK = ((CWeaponChaingun*)m_apWeapon[nSlot])->Create();
			if( nDuplicates > 0 ) {
				for( i=0; i<nDuplicates; i++ ) {
					m_aapDupWeapons[nSlot][i] = fnew CWeaponChaingun();
					bCreatedOK |= ((CWeaponChaingun*)m_aapDupWeapons[nSlot][i])->Create();
				}
			}
			break;

		default:
			FASSERT_NOW;
	}

	if( !bCreatedOK ) {
		return FALSE;
	}

	m_apWeapon[nSlot]->SetUpgradeLevel(nUpgradeLevel);
	m_apWeapon[nSlot]->EnableAutoWork( FALSE );
	m_apWeapon[nSlot]->RemoveFromWorld();
	m_apWeapon[nSlot]->SetOwner(this);
	m_apWeapon[nSlot]->SetDesiredState( CWeapon::STATE_DEPLOYED );

	FASSERT( m_apWeapon[nSlot] );
	for( i=0; i<nDuplicates; i++ ) {
		m_aapDupWeapons[nSlot][i]->SetUpgradeLevel(nUpgradeLevel);
		m_aapDupWeapons[nSlot][i]->EnableAutoWork( FALSE );
		m_aapDupWeapons[nSlot][i]->RemoveFromWorld();
		m_aapDupWeapons[nSlot][i]->SetOwner(this);
		m_aapDupWeapons[nSlot][i]->SetDesiredState( CWeapon::STATE_DEPLOYED );
	}

	return bCreatedOK;
}

void CBot::WeaponsDestroy( u32 nSlot ) {
	u32 i;

	if( nSlot != 0 ) {					//destroy weapon 1
		fdelete( m_apWeapon[1] );
		m_apWeapon[1] = NULL;

		for( i=0; i<m_anNumDupWeapons[1]; i++ ) {
			fdelete( m_aapDupWeapons[1][i] );
			m_aapDupWeapons[1][i] = NULL;
		}
	}

	if( nSlot != 1 ) {					//destroy weapon 0
		fdelete( m_apWeapon[0] );
		m_apWeapon[0] = NULL;

		for( i=0; i<m_anNumDupWeapons[0]; i++ ) {
			fdelete( m_aapDupWeapons[0][i] );
			m_aapDupWeapons[0][i] = NULL;
		}
	}
}


void CBot::WeaponsWork( u32 nSlot ) {
	u32 i;

	if( nSlot != 0 ) {		// do weapon 1
		m_apWeapon[1]->Work();

		for( i=0; i<m_anNumDupWeapons[1]; i++ ) {
			m_aapDupWeapons[1][i]->Work();
		}
	}
	
	if( nSlot != 1 ) {		// do weapon 1
		m_apWeapon[0]->Work();

		for( i=0; i<m_anNumDupWeapons[0]; i++ ) {
			m_aapDupWeapons[0][i]->Work();
		}
	}
}

void CBot::WeaponsAddToWorld( u32 nSlot ) {
	u32 i;

	if( nSlot != 0 ) {		// do weapon 1
		m_apWeapon[1]->AddToWorld();
		m_apWeapon[1]->ResetToState( CWeapon::STATE_DEPLOYED );

		for( i=0; i<m_anNumDupWeapons[1]; i++ ) {
			m_aapDupWeapons[1][i]->AddToWorld();
			m_aapDupWeapons[1][i]->ResetToState( CWeapon::STATE_DEPLOYED );
		}
	}
	
	if( nSlot != 1 ) {		// do weapon 0
		m_apWeapon[0]->AddToWorld();
		m_apWeapon[0]->ResetToState( CWeapon::STATE_DEPLOYED );

		for( i=0; i<m_anNumDupWeapons[0]; i++ ) {
			m_aapDupWeapons[0][i]->AddToWorld();
			m_aapDupWeapons[0][i]->ResetToState( CWeapon::STATE_DEPLOYED );
		}
	}
}

void CBot::WeaponsRemoveFromWorld( u32 nSlot ) {
	u32 i;

	if( nSlot != 0 ) {		// do weapon 1
		m_apWeapon[1]->RemoveFromWorld();

		for( i=0; i<m_anNumDupWeapons[1]; i++ ) {
			m_aapDupWeapons[1][i]->RemoveFromWorld();
		}
	}
	
	if( nSlot != 1 ) {		// do weapon 1
		m_apWeapon[0]->RemoveFromWorld();

		for( i=0; i<m_anNumDupWeapons[0]; i++ ) {
			m_aapDupWeapons[0][i]->RemoveFromWorld();
		}
	}
}

void CBot::WeaponsDrawEnable( BOOL bDrawingHasBeenEnabled, u32 nSlot ) {

	u32 i;

	if( nSlot != 0 ) {			// do weapon 1
		if( m_apWeapon[1] ) {
			m_apWeapon[1]->DrawEnable( bDrawingHasBeenEnabled );
		}
		for( i=0; i<m_anNumDupWeapons[1]; i++ ) {
			m_aapDupWeapons[1][i]->DrawEnable( bDrawingHasBeenEnabled );
		}
	}

	if( nSlot != 1 ) {			// do weapon 0
		if( m_apWeapon[0] ) {
			m_apWeapon[0]->DrawEnable( bDrawingHasBeenEnabled );
		}
		for( i=0; i<m_anNumDupWeapons[0]; i++ ) {
			m_aapDupWeapons[0][i]->DrawEnable( bDrawingHasBeenEnabled );
		}
	}
}


BOOL CBot::IsStoopable( f32 *pfMinHeight, f32 *pfMaxHeight ) const {

	if (m_fStoopMinHeight == 0.0f && m_fStoopMaxHeight)
	{
		if (pfMinHeight)
		{
			*pfMinHeight = m_fCollCylinderHeight_WS;
		}
		if( pfMaxHeight ) {
			*pfMaxHeight = m_fCollCylinderHeight_WS;
		}
		return FALSE;
	}

	if( pfMinHeight ) {
		*pfMinHeight = m_fStoopMinHeight;
	}

	if( pfMaxHeight ) {
		*pfMaxHeight = m_fStoopMaxHeight;
	}

	return m_fStoopMinHeight != m_fStoopMaxHeight;
}


void CBot::HandleStooping( void ) {
	if( m_anAnimStackIndex[ASI_STOOP] == -1 ) {
		return;
	}
	if( (!m_bStooping && (m_fCollCylinderHeight_WS == m_fStoopMaxHeight)) ||
		(m_bStooping && (m_fCollCylinderHeight_WS == m_fStoopMinHeight)) ) {
		return;
	}
		

	f32 fCtlValue = GetControlValue( ASI_STOOP );
	m_fCollCylinderHeight_WS = FMATH_FPOT( fCtlValue, m_fStoopMaxHeight, m_fStoopMinHeight );

	if( m_bStooping ) {
		DataPort_Open( FALSE );			// all necessary checks happen inside the dataport_open fn
		if( fCtlValue < 1.0f ) {
			fCtlValue += FLoop_fPreviousLoopSecs * 0.5f;
			FMATH_CLAMP_MAX1( fCtlValue );
			SetControlValue( ASI_STOOP, fCtlValue );
		}
	} else {
		DataPort_Open( TRUE );			// all necessary checks happen inside the dataport_open fn
		if( fCtlValue > 0.0f ) {
			fCtlValue -= FLoop_fPreviousLoopSecs;
			FMATH_CLAMP_MIN0( fCtlValue );
			SetControlValue( ASI_STOOP, fCtlValue );
		}
	}
}

BOOL CBot::CanAttachToZipLine( void ) {
	if( m_nThrowState != THROWSTATE_NONE ) {
		return FALSE;
	}

	if( m_nWSState != WS_STATE_NONE ) {
		return FALSE;
	}

	if( m_nWRState != WR_STATE_NONE ) {
		return FALSE;
	}

	if( m_nScopeState != SCOPESTATE_NONE ) {
		return FALSE;
	}

	if( m_pCableHook ) {
		return FALSE;
	}

	if( m_Velocity_WS.y >= 0.0f ) {
		return FALSE;
	}

	return TRUE;
}

void CBot::ZipLineVerletSetup( void ) { 
	FASSERT( m_pCableHook );

	// m_ZipVerletCurr, m_ZipVerletPrev, m_ZipHandPos_MS and m_fZipVerletRestDistance will be setup in ZipLineVerletWork()
	// This is done since we need to delay the setup of these values by one frame.  If they are setup here, the bot swings
	// wildly on the zipline at the start.
	m_fZipVerletRestDistance = -3.0f;
}

// pHandPos_WS will be our anchor point
void CBot::ZipLineVerletWork( const CFVec3A *pHandPos_WS, CFMtx43A *OffsetMatrix ) {
	FASSERT( pHandPos_WS );
	FASSERT( m_pCableHook );

	CFVec3A Gravity;
	CFVec3A Pos_WS_Diff;

	Gravity.Set( 0.0f, -128.0f, 0.0f );

	// Make the bot follow the zipline
	Pos_WS_Diff.Sub( m_pCableHook->m_Pos_WS, *pHandPos_WS );
	m_MountPos_WS.Add( Pos_WS_Diff );

	if( m_fZipVerletRestDistance == -3.0f ) {
		// In this case we need to skip the work
		m_fZipVerletRestDistance += 1.0f;
		OffsetMatrix->Identity();

		return;
	} else if( m_fZipVerletRestDistance < 0.0f ) {
		m_fZipVerletRestDistance += 1.0f;

		if( m_fZipVerletRestDistance == 0.0f ) {
			// Now set everything up since we are on the zipline
			m_ZipVerletCurr = m_MountPos_WS;
			m_ZipVerletPrev = m_MountPos_WS;
			
			m_ZipHandPos_MS.Sub( *pHandPos_WS, m_MountPos_WS );
			m_fZipVerletRestDistance = m_ZipHandPos_MS.Mag();
			WS2MS( m_ZipHandPos_MS );

			// Give a bit of swing when we first get on
			f32 fAmnt = fmath_RandomFloatRange( 164.0f, 200.0f );
			CFVec3A ZipLineRight;

			fAmnt *= fmath_RandomChance( 0.5f ) ? 1.0f : -1.0f;
			
			ZipLineRight.Cross( m_pCableHook->m_UnitVecToNext, CFVec3A::m_UnitAxisY );
			ZipLineRight.Mul( fAmnt );
			Gravity.Add( ZipLineRight );
		} else {
			OffsetMatrix->Identity();
			return;
		}
	}

	// If we just started to brake, swing a bit to the zipline's right
	if( m_pCableHook->m_bBrakeStartThisFrame ) {
		f32 fAmnt = fmath_RandomFloatRange( 164.0f, 200.0f );
		CFVec3A ZipLineRight;

		fAmnt *= fmath_RandomChance( 0.5f ) ? 1.0f : -1.0f;

		ZipLineRight.Cross( m_pCableHook->m_UnitVecToNext, CFVec3A::m_UnitAxisY );
		ZipLineRight.Mul( fAmnt );
		Gravity.Add( ZipLineRight );
	}

	// Do the verlet integration
	CFVec3A NewVerletToHand, TempVec;
	CFQuatA RotQuat;

	SimpleVerlet( &m_ZipVerletCurr, &m_ZipVerletPrev, &Gravity , 0.3f );
	CFVerlet::Constraint_AnchorDistance( &m_ZipVerletCurr, pHandPos_WS, m_fZipVerletRestDistance );

	NewVerletToHand.Sub( *pHandPos_WS, m_ZipVerletCurr );

	if( NewVerletToHand.MagSq() > FMATH_POS_EPSILON ) {
		NewVerletToHand.Unitize();
	} else {
		NewVerletToHand = CFVec3A::m_UnitAxisY;
	}

	// Don't let the swing go too much
	if( NewVerletToHand.y < 0.2f ) {
		CFVec3A Down = CFVec3A::m_UnitAxisY;

		// Bump the verlet point down a little
		Down.Mul( -0.05f );
		m_ZipVerletCurr.Add( Down );

		// Re-apply constraint
		CFVerlet::Constraint_AnchorDistance( &m_ZipVerletCurr, pHandPos_WS, m_fZipVerletRestDistance );

		NewVerletToHand.Sub( *pHandPos_WS, m_ZipVerletCurr );
		NewVerletToHand.Unitize();
	}

	// Get our offset matrix from the world Y-axis
	RotQuat.BuildQuat( CFVec3A::m_UnitAxisY, NewVerletToHand );
	RotQuat.BuildMtx( *OffsetMatrix );
}

// pCurrent		 - Current verlet point
// pPrevious	 - Previous verlet point
// pForce		 - Force to apply
// fDampenFactor - Amount to dampen in a second
void CBot::SimpleVerlet( CFVec3A *pCurrent, CFVec3A *pPrevious, const CFVec3A *pForce, const f32 &fDampenFactor /*= 0.0f*/ ) {
	FASSERT( pCurrent );
	FASSERT( pPrevious );
	FASSERT( pForce );
	FASSERT( fDampenFactor >= 0.0f );
	
	CFVec3A CurrTemp = *pCurrent;
	CFVec3A Force = *pForce;
	CFVec3A OldToNew;
	f32 fDelta = FMATH_MIN( FLoop_fPreviousLoopSecs, 0.033f );

	Force.Mul( ( fDelta * fDelta ) );

	// Dampening
	OldToNew.Sub( *pCurrent, *pPrevious ).Mul( ( fDampenFactor * fDelta ) );
    pPrevious->Add( OldToNew );

	CurrTemp.Mul( 2.0f );
	CurrTemp.Sub( *pPrevious );
	CurrTemp.Add( Force );

	*pPrevious = *pCurrent;
	*pCurrent = CurrTemp;
}


void CBot::StunBot( const f32 &fStunTime ) {
	FASSERT( IsCreated() ); 
	
	if( m_nPossessionPlayerIndex < 0 ) {
		FMATH_CLEARBITMASK( m_nBotFlags, BOTFLAG_IS_STUNNED );
		return;
	}

	// If we get a negative number, don't enable
	if( fStunTime > 0.0f ) {
		FMATH_SETBITMASK( m_nBotFlags, BOTFLAG_IS_STUNNED );
		m_fStunTime = fStunTime;
	}

	CFCamera *pCamera = fcamera_GetCameraByIndex( m_nPossessionPlayerIndex );
	pCamera->StunCamera( fStunTime );
}

BOOL CBot::SlowBot(f32 fControlMult, f32 fTimeToSlowDown, f32 fTimeToStartReturningToNormal){
	FASSERT( IsCreated() ); 
	BOOL bRet = m_nBotFlags & BOTFLAG_IS_SLOWED;
	// If we get a negative number or zero, reset status
	if( fControlMult == 0.0f ) {
		FMATH_CLEARBITMASK( m_nBotFlags, BOTFLAG_IS_SLOWED );
	}
	else if (fTimeToSlowDown > 0.f){
		if ( (fTimeToStartReturningToNormal > fTimeToSlowDown) || (fTimeToStartReturningToNormal < 0.f) )
			fTimeToStartReturningToNormal =fTimeToSlowDown;
		FMATH_SETBITMASK( m_nBotFlags, BOTFLAG_IS_SLOWED );
		m_fSlowTimeSinceHit=0.0f;
		m_fSlowTimeSpeedUp=fTimeToStartReturningToNormal;
		m_fSlowTimeOver=fTimeToSlowDown;
		m_fSlowEffectMult=fControlMult;
		m_fSlowUnitParalysis=1.0f;
	}
	return bRet;
}

void CBot::Squish( const CBot* pSquisher ) {
	if( IsPlayerOutOfBody() || IsInvincible() ) {
		// Ignore this...
		return;
	}

	// If I am already dead, dont' credit a kill!
	if ( !IsDeadOrDying() ) {
		m_uBotDeathFlags = BOTDEATHFLAG_DISAPPEAR | BOTDEATHFLAG_COMEAPART_FAST;
		//SpawnDeathEffects();
		Die();

		if ( pSquisher ) {
			s32 nPlayerIndex = pSquisher->m_nPossessionPlayerIndex;
			if (nPlayerIndex >= 0)
				Player_aPlayer[nPlayerIndex].CreditKill( m_nPossessionPlayerIndex, this);

			// If the squisher was recruited, give the recruiter credit also
			if (pSquisher->Recruit_IsRecruited()) {
				nPlayerIndex = pSquisher->Recruit_GetRecruiter();
				if (nPlayerIndex >= 0)
					Player_aPlayer[nPlayerIndex].CreditKill( m_nPossessionPlayerIndex, this );
			}
		}
	}
}

void CBot::Attach_ToParent_WithGlue_WS( CEntity *pParentEntity, cchar *pszAttachBoneName, BOOL bInheritParentScale)
{
	CEntity::Attach_ToParent_WS(pParentEntity,pszAttachBoneName,bInheritParentScale);
	m_Velocity_WS.Zero();
	VelocityHasChanged( );
	
	m_nBotFlags |= BOTFLAG_GLUED_TO_PARENT;
}

void CBot::DetachFromParent( void )
{
	m_nBotFlags &= (~BOTFLAG_GLUED_TO_PARENT);
	CEntity::DetachFromParent();
	m_pStickyEntity = NULL;
}

void CBot::StartVelocityJump( const CFVec3A *pvJumpVelocity_WS ) {
	FASSERT( IsCreated() ); 
	_StartVelocityJump( pvJumpVelocity_WS );
}



#define _LIGHT_ON_RATE					( 1.5f )	// rate of increase in light intensity. In units per second.
#define _LIGHT_OFF_RATE					( 1.2f )	// rate of decrease in light intensity. In units per second.
#define _MAX_SPOTLIGHT_INTENSITY		( 2.0f )
#define _MAX_SPOTLIGHT_MESH_INTENSITY	( 1.0f )
//-----------------------------------------------------------------------------
// Bot light state machine. This function should be called in ClassHierarchyWork() of derived bot class.
void CBot::UpdateSpotLight( void )
{
	f32 fIntensity;

	if( m_pSpotLight )
	{

		_OverrideSpotLight();

		switch( m_eSpotLightState )
		{
			case LIGHT_STATE_TURNING_ON:
				fIntensity = m_pSpotLight->m_Light.GetIntensity() + FLoop_fPreviousLoopSecs * _LIGHT_ON_RATE;

				if( fIntensity >= _MAX_SPOTLIGHT_INTENSITY )
				{
					fIntensity = _MAX_SPOTLIGHT_INTENSITY;
					m_eSpotLightState = LIGHT_STATE_ON;
				}
				m_pSpotLight->m_Light.SetIntensity( fIntensity );
			break;

			case LIGHT_STATE_TURNING_OFF:
				fIntensity = m_pSpotLight->m_Light.GetIntensity() - FLoop_fPreviousLoopSecs * _LIGHT_OFF_RATE;
				if( fIntensity <= 0.0f )
				{
					fIntensity = 0.0f;
					m_eSpotLightState = LIGHT_STATE_OFF;
					m_pSpotLight->m_Light.Enable( FALSE );
				}
				m_pSpotLight->m_Light.SetIntensity( fIntensity );
			break;
		}
	}

	if( m_pSpotLightMesh ) {
		if( m_eSpotLightState != LIGHT_STATE_OFF ) {
			m_pSpotLightMesh->m_Xfm.BuildFromMtx( *m_pSpotLightMeshMtx );
				m_pSpotLightMesh->UpdateTracker();
		}

		switch( m_eSpotLightState ) {
			case LIGHT_STATE_TURNING_ON:
				fIntensity = m_pSpotLightMesh->GetMeshTintAndAlpha()->fAlpha + FLoop_fPreviousLoopSecs * _LIGHT_ON_RATE;

				if( fIntensity >= _MAX_SPOTLIGHT_MESH_INTENSITY ) {
					fIntensity = _MAX_SPOTLIGHT_MESH_INTENSITY;
					m_eSpotLightState = LIGHT_STATE_ON;
				}
				m_pSpotLightMesh->SetMeshAlpha( fIntensity );
				break;

			case LIGHT_STATE_TURNING_OFF:
				fIntensity = m_pSpotLightMesh->GetMeshTintAndAlpha()->fAlpha - FLoop_fPreviousLoopSecs * _LIGHT_OFF_RATE;

				if( fIntensity <= 0.0f ) {
					fIntensity = 0.0f;

					m_eSpotLightState = LIGHT_STATE_OFF;
					FMATH_SETBITMASK( m_pSpotLightMesh->m_nFlags, FMESHINST_FLAG_DONT_DRAW );
				}

				m_pSpotLightMesh->SetMeshAlpha( fIntensity);
				break;
		}
	}
}

//-----------------------------------------------------------------------------
// turn on bot's spotlight(s) if any.
// if bImmediately is TRUE, light snaps on rather than fading in.
// Note that UpdateLights() must be called in ClassHierarchyWork() of derived bot class.
void CBot::SetSpotLightOn( BOOL bImmediately )
{
	if( bImmediately ) {
		m_eSpotLightState = LIGHT_STATE_ON;

		if( m_pSpotLight ) {
			m_pSpotLight->m_Light.Enable( TRUE );
			m_pSpotLight->m_Light.SetIntensity( 1.0f );
		}

		if( m_pSpotLightMesh ) {
			FMATH_CLEARBITMASK( m_pSpotLightMesh->m_nFlags, FMESHINST_FLAG_DONT_DRAW );
			m_pSpotLightMesh->SetMeshAlpha( 1.0f );
		}

	} else if( m_eSpotLightState == LIGHT_STATE_TURNING_OFF || m_eSpotLightState == LIGHT_STATE_OFF ) {
		m_eSpotLightState = LIGHT_STATE_TURNING_ON;

		if( m_pSpotLight ) {
			m_pSpotLight->m_Light.Enable( TRUE );
		}

		if( m_pSpotLightMesh ) {
			FMATH_CLEARBITMASK( m_pSpotLightMesh->m_nFlags, FMESHINST_FLAG_DONT_DRAW );
		}
	}
}

//-----------------------------------------------------------------------------
// turn on bot's spotlight(s) if any.
// if bImmediately is TRUE, light snaps off rather than fading out.
// Note that UpdateLights() must be called in ClassHierarchyWork() of derived bot class.
void CBot::SetSpotLightOff( BOOL bImmediately )
{
	if( bImmediately ) {
		m_eSpotLightState = LIGHT_STATE_OFF;

		if( m_pSpotLight ) {
			m_pSpotLight->m_Light.Enable( FALSE );
			m_pSpotLight->m_Light.SetIntensity( 0.0f );
		}

		if( m_pSpotLightMesh ) {
			FMATH_SETBITMASK( m_pSpotLightMesh->m_nFlags, FMESHINST_FLAG_DONT_DRAW );
			m_pSpotLightMesh->SetMeshAlpha( 0.0f );
		}

	} else if( m_eSpotLightState == LIGHT_STATE_TURNING_ON || m_eSpotLightState == LIGHT_STATE_ON ) {
		m_eSpotLightState = LIGHT_STATE_TURNING_OFF;
	}
}


///////////////////////////////////
//LIMP FNs

// Set bLeftSide to TRUE to hop on his left leg only.
void CBot::GiveBotLimp( BOOL bLeftSide ) {
	if( bLeftSide ) {
		if( m_nLimpState == BOTLIMPSTATE_RIGHT ) {
			// kill the bot
			Die();
		} else {
			SetLimpState( BOTLIMPSTATE_LEFT );
		}
	} else if( !bLeftSide ) {
		if( m_nLimpState == BOTLIMPSTATE_LEFT ) {
            Die();
		} else {
			SetLimpState( BOTLIMPSTATE_RIGHT );
		}
	}
}

void CBot::SetLimpState( u32 uLimpState ) {
	FASSERT( uLimpState < BOTLIMPSTATE_COUNT );
	m_nLimpState = (LimpState_e)uLimpState;

	if( m_nLimpState == BOTLIMPSTATE_NONE ) {
		m_fUnitLimpBlend			= 0.0f;
		m_fUnitLimpSpeedModifier	= 1.0f;

		if( m_anAnimStackIndex[ASI_STAND_LIMP_LEFT] != -1 ) {
			SetControlValue( ASI_STAND_LIMP_LEFT, 0.0f );
		}

		if( m_anAnimStackIndex[ASI_STAND_LIMP_RIGHT] != -1 ) {
			SetControlValue( ASI_STAND_LIMP_RIGHT, 0.0f );
		}

		if( m_anAnimStackIndex[ASI_LIMP_RIGHT] != -1 ) {
			SetControlValue( ASI_LIMP_RIGHT, 0.0f );
		}

		if( m_anAnimStackIndex[ASI_LIMP_LEFT] != -1 ) {
			SetControlValue( ASI_LIMP_LEFT, 0.0f );
		}

	} else {
		// make sure this bot can actually limp...
		if( m_nLimpState == BOTLIMPSTATE_LEFT ) {
			if( (m_anAnimStackIndex[ASI_LIMP_LEFT] == -1) ||
				(m_anAnimStackIndex[ASI_STAND_LIMP_LEFT] == -1) ) {
				m_nLimpState = BOTLIMPSTATE_NONE;
				return;
			}
		} else if( m_nLimpState == BOTLIMPSTATE_RIGHT ) {
			if( (m_anAnimStackIndex[ASI_LIMP_RIGHT] == -1) ||
				(m_anAnimStackIndex[ASI_STAND_LIMP_RIGHT] == -1) ) {
				m_nLimpState = BOTLIMPSTATE_NONE;
				return;
			}
		}

		// make sure some appropriate limp values are set
		m_fUnitLimpSpeedModifier	= m_pBotInfo_Walk->fLimpMaxUnitRun;
	}
}

void CBot::EnteringMechWork(CBot* pMech)//must be called when this bot gets into a vehicle or site-weapon
{ 
	m_fLegsYaw_MS = 0.0f;
	ChangeMountYaw(0.0f);
	ComputeMtxPalette( FALSE );
	m_pCurMech = pMech;
}	  

void CBot::ExitingMechWork(void)//must be called when this bot gets leaves a vehicle or site-weapon
{ 
	m_pCurMech = NULL;
}		  


BOOL CBot::EyesDamaged( void ) const {
	FASSERT( IsCreated() );
	FASSERT( m_pPartMgr != NULL );
	
	if( !m_pPartMgr->IsCreated() ) {
		return FALSE;
	}

	return (m_pPartMgr->GetComponentStatus( CBotPartMgr::COMPONENT_TYPE_EYES ) == CBotPartMgr::COMPONENT_STATUS_SOME_PARTIALLY_OPERATIONAL);
}


BOOL CBot::EyesDestroyed( void ) const {
	FASSERT( IsCreated() );
	FASSERT( m_pPartMgr != NULL );

	if( !m_pPartMgr->IsCreated() ) {
		return FALSE;
	}

	return (m_pPartMgr->GetComponentStatus( CBotPartMgr::COMPONENT_TYPE_EYES ) == CBotPartMgr::COMPONENT_STATUS_NONE_OPERATIONAL);
}


void CBot::PlayFootSound( BOOL bLeft, BOOL bRight, SurfaceType_e nSurfaceType ) {
	FASSERT( nSurfaceType >= SURFACE_TYPE_NONE && nSurfaceType < SURFACE_TYPE_COUNT );

	m_fNextScuffSound -= FLoop_fPreviousLoopSecs;
	FMATH_CLAMPMIN( m_fNextScuffSound, 0.0f );

	if( !bLeft && !bRight ) {
		return;
	}

	if( m_nPossessionPlayerIndex >= 0 ) {
		if( m_pBotInfo_Sound && (nSurfaceType != SURFACE_TYPE_NONE) ) {
			u32 uSndIndex;

			if( m_fNextScuffSound == 0.0f ) {
				uSndIndex = fmath_RandomRange( BOTSOUND_SCUFF_SOUND_START, BOTSOUND_NUM_SOUNDS_PER_SURFACE - 1 );
				m_fNextScuffSound = fmath_RandomFloatRange( BOT_SOUND_SCUFF_MIN_TIME, BOT_SOUND_SCUFF_MAX_TIME );
			} else {
				uSndIndex = fmath_RandomChoice( BOTSOUND_SCUFF_SOUND_START );
			}

			fsndfx_Play2D(
				m_pBotInfo_Sound->ahStepSound[nSurfaceType][ uSndIndex ],
				m_fClampedNormSpeedAlongSurface_WS * m_fStepVolumeScale * m_pBotInfo_Walk->fFootstepSoundUnitVolume_2D 
			);

			AIEnviro_BoostPlayerSoundTo(m_nPossessionPlayerIndex, 19.0f*m_fClampedNormSpeedAlongSurface_WS*m_fClampedNormSpeedAlongSurface_WS);
		}
	} else {
		if( m_pBotInfo_Sound && (nSurfaceType != SURFACE_TYPE_NONE) ) {
			u32 uSndIndex;

			if( m_fNextScuffSound == 0.0f ) {
				uSndIndex = fmath_RandomRange( BOTSOUND_SCUFF_SOUND_START, BOTSOUND_NUM_SOUNDS_PER_SURFACE - 1);
				m_fNextScuffSound = fmath_RandomFloatRange( BOT_SOUND_SCUFF_MIN_TIME, BOT_SOUND_SCUFF_MAX_TIME );
			} else {
				uSndIndex = fmath_RandomChoice( BOTSOUND_SCUFF_SOUND_START );
			}

			fsndfx_Play3D(
				m_pBotInfo_Sound->ahStepSound[nSurfaceType][ uSndIndex ],
				&MtxToWorld()->m_vPos,
				m_pBotInfo_Walk->fFootstepSoundRadius_3D,
				1.0f,
				m_fClampedNormSpeedAlongSurface_WS * m_pBotInfo_Walk->fFootstepSoundUnitVolume_3D
			);
		}
	}
}

void CBot::ComputeWeaponMuzzlePoint_WS( CFVec3A *pMuzzlePt ) { 
	FASSERT( IsCreated() ); 
	if( m_apWeapon[0] ) {
		m_apWeapon[0]->ComputeMuzzlePoint_WS( pMuzzlePt ); 
	} else {
        *pMuzzlePt = m_MountPos_WS; 
	}
}


void CBot::ComputeWeaponMuzzleVec_WS( CFVec3A *pMuzzleVec ) { 
	FASSERT( IsCreated() ); 
	if( m_apWeapon[0] ) {
		*pMuzzleVec = m_apWeapon[0]->MtxToWorld()->m_vFront; 
	} else {
		*pMuzzleVec = m_MtxToWorld.m_vFront;
	}
}

FINLINE BOOL BotIsTargetable( CBot *pBot, CBot *pPotentialTarget ) {
	// Check the submitted bot for targetability
	if ( pPotentialTarget->IsDeadOrDying() ) {
		// Bot is dying:
		return FALSE;
	}

    if ( aiutils_IsFriendly( pBot, pPotentialTarget, NULL ) ) {
		// Bot is friendly
		return FALSE;
	}

	// Is this a vehicle?
	if ( pPotentialTarget->TypeBits() & ENTITY_BIT_VEHICLE ) {
		// Is there a driver
		if ( ((CVehicle *)pPotentialTarget)->GetDriverBot() ) {
			// Check the driver's affiliation
			if ( aiutils_IsFriendly( pBot, ((CVehicle *)pPotentialTarget)->GetDriverBot(), NULL ) ) {
				// The driver is a friendly, so don't target it
				return FALSE;
			}
		} else {
			// There is no driver, so nothing to target
			return FALSE;
		}
	}

	// All outs failed, so we have a valid target
	return TRUE;
}

// will copy the target point into pRawUnAssistedTargetPt before any target assistance is done to the final pTargetPt
// Desktop: the target assistance below is tuned for sticks. It applies while the player aims
// with a controller and is off while they aim with the mouse, unless -aim-assist forces it.
static FINLINE BOOL _AimAssistAllowed( const CPlayer *pPlayer ) {
#if defined(MA_PC_INPUT)
	return pcinput_AimAssistAllowed( pPlayer->m_nControllerIndex );
#else
	return TRUE;
#endif
}

static FINLINE f32 _TargetingAssistance( const CPlayer *pPlayer ) {
	return _AimAssistAllowed( pPlayer ) ? pPlayer->m_fUnitTargetingAssistance : 0.0f;
}


void CBot::ComputeHumanTargetPoint_WS( CFVec3A *pTargetPt, CWeapon *pWeapon, f32 fWeaponRangeOverride/*=-1.f*/, CFVec3A *pRawUnAssistedTargetPt/*=NULL*/ ) { 
	FASSERT( IsCreated() ); 
	FASSERT( m_nPossessionPlayerIndex >= 0 );

	// Calculate target point
	u32 nThisBotTrackerCount;
	f32 fReticleX, fReticleY, fMaxLiveRange, fEndPointReticleAdjust, fCameraZAdjust;
	CFVec3A RayStartPoint, RayEndPoint, ReticleAdjustX, ReticleAdjustY;

	CFCamera*				pCamera;
	const FViewport_t*		pViewport;
	GameCamType_e			nCamType;
	FCollImpact_t			CollImpact;
	CFTrackerCollideRayInfo CollRayInfo;
	const CFWorldTracker*	pHitTracker;
	CEntity*				pHitEntity;
	CReticle*				pReticle;
	CPlayer*				pPlayer;
	//GCollSurfType_e			eImpactedSurfaceType;

	// A RAT gun must exclude the entire occupied vehicle while targeting.
	FWorld_nTrackerSkipListCount = 0;
	if( (TypeBits() & ENTITY_BIT_SITEWEAPON) && GetParent() && (GetParent()->TypeBits() & ENTITY_BIT_VEHICLERAT) ) GetParent()->AppendTrackerSkipList();
	else AppendTrackerSkipList();
	nThisBotTrackerCount = FWorld_nTrackerSkipListCount;

	pPlayer = &Player_aPlayer[ m_nPossessionPlayerIndex ];
	pReticle = &pPlayer->m_Reticle;

	// Get camera and viewport info...
	pCamera = fcamera_GetCameraByIndex(m_nPossessionPlayerIndex);
	pViewport = pCamera->GetViewport();
	CFCameraMan *pCameraMan = gamecam_GetCameraManByIndex( (GameCamPlayer_e)m_nPossessionPlayerIndex, &nCamType );

	// Initially, ray start point is the camera position, though this might need to be adjusted along the camera Z-axis
	RayStartPoint.Set( *(pCamera->GetPos()) );
	if ( nCamType == GAME_CAM_TYPE_ROBOT_3RD ) {
		// Camera is offset from the player bot
		fCameraZAdjust = ((CCamBot *)pCameraMan)->GetDesiredToLookAtPoint_MS()->z * ((CCamBot *)pCameraMan)->GetLastUnitDist();
	} else {
		// Camera is not offset
		fCameraZAdjust = 0.f;
	}

	// Compute the ray end point which lies in the center of the reticle...
	if ( fWeaponRangeOverride != -1.f ) {
		fMaxLiveRange = fWeaponRangeOverride;
	}
	else if ( pWeapon ) {
		fMaxLiveRange = pWeapon->m_pInfo->fMaxTargetAssistDist;
	} else {
		FASSERT( m_pBotInfo_MountAim != NULL );
		fMaxLiveRange = m_pBotInfo_MountAim->fMaxTargetRange;
	}

	// Build a ray from the start point of fMaxLiveRange through the reticle

	fReticleX = pReticle->GetNormOriginX();
	fReticleY = pReticle->GetNormOriginY();

	fEndPointReticleAdjust = (fMaxLiveRange * fReticleX) * pViewport->fTanHalfFOVX;
	ReticleAdjustX.Mul( pCamera->GetFinalXfm()->m_MtxR.m_vRight, fEndPointReticleAdjust );

	fEndPointReticleAdjust = (fMaxLiveRange * fReticleY) * pViewport->fTanHalfFOVY;
	ReticleAdjustY.Mul( pCamera->GetFinalXfm()->m_MtxR.m_vUp, fEndPointReticleAdjust );

	RayEndPoint.Mul( pCamera->GetFinalXfm()->m_MtxR.m_vFront, fMaxLiveRange ).Add( RayStartPoint ).Add( ReticleAdjustX ).Add( ReticleAdjustY );

	if ( fCameraZAdjust ) {
		// The camera is in third-person mode, behind the player.  In this case, we
		// need to push the start point forward so that we don't collide with objects
		// behind the player but in front of the camera

		// Use this currently unused var
		ReticleAdjustX.Sub( RayEndPoint, RayStartPoint );
		f32 fDistSq = ReticleAdjustX.MagSq();
		if ( fDistSq ) {
			ReticleAdjustX.Mul( fmath_InvSqrt( fDistSq ) );
			RayStartPoint.Add( ReticleAdjustX.Mul( -fCameraZAdjust ) );
		}
	}

	// Set up the screen space info
	CFVec2 vScreenXY( pReticle->GetNormOriginX(), pReticle->GetNormOriginY() );
	f32 fReticleActiveRadius = pReticle->GetScreenSpaceRadius() * PLAYER_TARGET_ASSIST_RETICLE_MULTIPLIER;

	// These vars will get filled out if the relevant meshes are detected
	CFWorldMesh *pTargetedMesh = NULL;
	FVis_ScreenMesh_t *pBiasToMesh = NULL;

	// Determine the coll mask
	u32 nCollMask;
	if( !pWeapon || !(pWeapon->m_pInfo->nInfoFlags & CWeapon::INFOFLAG_THICK_TARGETING) ) {
		nCollMask = FCOLL_MASK_COLLIDE_WITH_THIN_PROJECTILES;
	} else {
		nCollMask = FCOLL_MASK_COLLIDE_WITH_THICK_PROJECTILES;
	}

	// We'll start out with the reticle center flag cleared
	pPlayer->m_bReticleCenterOverTarget = FALSE;

	// Cast a ray straight down the camera line to see if there is an object along that ray
	FVis_ScreenMesh_t MeshUnderReticle;
	if( fworld_FindClosestImpactPointToRayStart( &CollImpact, &RayStartPoint, &RayEndPoint, FWorld_nTrackerSkipListCount, (const CFWorldTracker **)FWorld_apTrackerSkipList, TRUE, m_pWorldMesh, -1, nCollMask ) ) {
		// The ray hit something
		pTargetPt->Set( CollImpact.ImpactPoint );

		pHitEntity = CGColl::ExtractEntity( &CollImpact );

		if ( pHitEntity ) {
			if ( pHitEntity->IsTargetable() ) {
				if ( pHitEntity->TypeBits() & ENTITY_BIT_BOT ) {
					// This is a bot, so check to see if it is something to target
					if (  BotIsTargetable( this, (CBot *)pHitEntity ) ) {
						// This is a bot we should target
						pPlayer->m_bReticleCenterOverTarget = TRUE;
						pTargetedMesh = (CFWorldMesh *)CollImpact.pTag;

						// Check to see if this bot is Corrosive.  If it is, check to see if the impact point paints a VSPOT
						if( pHitEntity->TypeBits() & ENTITY_BIT_BOTCORROSIVE ) {

							// Check Corrosives vspot array to see if the impact point is within a Vspot
							if( !( (CBotCorrosive *)pHitEntity)->ImpactPointInVSpot( &CollImpact.ImpactPoint ) ) {
							// Check the surface type of the impact.  If it's of VULNERABLE TYPE, then it's all good!
//							eImpactedSurfaceType = CGColl::GetSurfaceType( &CollImpact );
//							if( eImpactedSurfaceType != GCOLL_SURF_TYPE_VULNERABLE ) {
								// The surface type impacted is NOT vulnerable, so don't paint the reticle...
								pPlayer->m_bReticleCenterOverTarget = FALSE;
								pTargetedMesh = NULL;
							}
						}

						// Perform aim biasing, if this user has it enabled and the bot is not a vehicle
						if ( !(pHitEntity->TypeBits() & (ENTITY_BIT_VEHICLE|ENTITY_BIT_BOTSWARMER|ENTITY_BIT_BOTCORROSIVE)) && _TargetingAssistance( pPlayer ) ) {
							// We are going to be biasing to this mesh, so get its position relative to the reticle 
							pBiasToMesh = &MeshUnderReticle;
							fvis_GetMeshPosInScreenRegion( pViewport, 
															&pCamera->GetFinalXfm()->m_MtxF, 
															(CFWorldMesh *)CollImpact.pTag, 
															&vScreenXY, 
															fReticleActiveRadius * PLAYER_AIM_BIASING_RETICLE_MULTIPLIER, 
															PLAYER_TARGET_ASSIST_TRACKER_MULTIPLIER,
															pBiasToMesh );

							if ( pBiasToMesh->spScreen.m_Pos.z + fCameraZAdjust < 0.f ) {
								// This mesh is behind the camera
								pBiasToMesh = NULL;
							}
						}
					}
				} else {
					// If this is a weapon, make sure it doesn't belong to a friendly

					if ( pHitEntity->TypeBits() & ENTITY_BIT_WEAPON ) {
						CBot *pOwner = ((CWeapon *)pHitEntity)->GetOwner();
						if ( pOwner && !aiutils_IsFriendly( this, pOwner, NULL ) ) {
							// This is an enemy weapon
							pPlayer->m_bReticleCenterOverTarget = TRUE;
							pTargetedMesh = (CFWorldMesh *)CollImpact.pTag;
						}
					} else {
						// This is not a bot, but is a targetable entity, so target it
						pPlayer->m_bReticleCenterOverTarget = TRUE;
						pTargetedMesh = (CFWorldMesh *)CollImpact.pTag;
					}
				}
			}
		}
	} else {
		pTargetPt->Set( RayEndPoint );
	}

	if( pRawUnAssistedTargetPt ) {
		// save off the target point before focusing the pt
		pRawUnAssistedTargetPt->Set( *pTargetPt );
	}

	// If we don't have a targeted mesh from the simple ray case, and this
	// player has targeting assistance enabled, then we need to check the
	// proximity of the reticle to determine what is nearby
	if ( !pTargetedMesh /*&& pPlayer->m_fUnitTargetingAssistance*/ && _AimAssistAllowed( pPlayer ) && (!pWeapon || !(pWeapon->m_pInfo->nInfoFlags & CWeapon::INFOFLAG_NO_AUTO_TARGETING)) ) {
		u64 nUnwantedBits = (ENTITY_BIT_RESERVED_FOR_FANG|ENTITY_BIT_BOT|ENTITY_BIT_BOTSWARMER|ENTITY_BIT_DOOR|ENTITY_BIT_GOODIE|ENTITY_BIT_CONSOLE
							|ENTITY_BIT_SHIELD|ENTITY_BIT_VEHICLELOADER|ENTITY_BIT_VEHICLESENTINEL|ENTITY_BIT_VEHICLERAT|ENTITY_BIT_VEHICLE|ENTITY_BIT_BOTCORROSIVE);

		u32 nMeshCountUnderReticle = fvis_GetMeshesInScreenRegion(  pViewport,
																	&pCamera->GetFinalXfm()->m_MtxF, 
																	m_nPossessionPlayerIndex,					// Check the screen rendered for this player
																	&vScreenXY, 
																	fReticleActiveRadius * PLAYER_AIM_BIASING_RETICLE_MULTIPLIER,
																	fMaxLiveRange,								// Do not include any meshes beyond the range of the weapon
																	PLAYER_TARGET_ASSIST_TRACKER_MULTIPLIER,	// Use this percentage of the tracker radius
																	~nUnwantedBits,						
																	TRUE );										// Use tracker skip list

		if( nMeshCountUnderReticle ) {
			// Check the meshes under the reticle for a target
			CFVec3A vFocalPoint;

			// Sort the results - we will consider closer objects more relevant
			fvis_SortRegionMeshResults();

			// First we want to check the bots that are on screen and within the reticle to see if the line of sight is clear to them
			u32 nMeshIdx;
			for ( nMeshIdx = 0; nMeshIdx < nMeshCountUnderReticle; nMeshIdx++ ) {
				FASSERT( FVis_apSortedMeshesInRegion[nMeshIdx]->pWorldMesh );

				// Get a pointer to the next mesh
				pHitTracker = FVis_apSortedMeshesInRegion[nMeshIdx]->pWorldMesh;

				// We should only get back entities
				FASSERT( pHitTracker->m_nUser == MESHTYPES_ENTITY );

				pHitEntity = (CEntity *)pHitTracker->m_pUser;

				if( !pHitEntity->IsTargetable() ) {
					// This mesh is not targetable, so remove it from the list
					FVis_apSortedMeshesInRegion[nMeshIdx] = NULL;
					continue;
				}

				if ( FVis_apSortedMeshesInRegion[nMeshIdx]->spScreen.m_Pos.z + fCameraZAdjust < 0.f ) {
					// This mesh is behind the camera
					FVis_apSortedMeshesInRegion[nMeshIdx] = NULL;
					continue;
				}


				if ( !(pHitEntity->TypeBits() & ENTITY_BIT_BOT) ) {
					continue;
				}

				if ( !BotIsTargetable( this, (CBot *)pHitEntity ) ) {
					// This bot is not targetable, so let's ignore him.
					FVis_apSortedMeshesInRegion[nMeshIdx] = NULL;
					continue;
				}

				// This mesh is a bot, so let's test it for line of sight
				if ( FVis_apSortedMeshesInRegion[nMeshIdx]->fDistanceFromRegion <= 0.0001f ) {

					// The center of the target reticle is over the bounding sphere
					f32 fDist = FVis_apSortedMeshesInRegion[nMeshIdx]->spScreen.m_Pos.z + pHitTracker->GetBoundingSphere().m_fRadius + fCameraZAdjust;

					fEndPointReticleAdjust = fDist * fReticleX * pViewport->fTanHalfFOVX;
					ReticleAdjustX.Mul( pCamera->GetFinalXfm()->m_MtxR.m_vRight, fEndPointReticleAdjust );

					fEndPointReticleAdjust = fDist * fReticleY * pViewport->fTanHalfFOVY;
					ReticleAdjustY.Mul( pCamera->GetFinalXfm()->m_MtxR.m_vUp, fEndPointReticleAdjust );

					vFocalPoint.Mul( pCamera->GetFinalXfm()->m_MtxR.m_vFront, fDist ).Add( RayStartPoint ).Add( ReticleAdjustX ).Add( ReticleAdjustY );
				} else {
					// The center of the reticle does not cover the tracker, so choose the point on the tracker closest to it
					CFVec3A *pDir = &FVis_apSortedMeshesInRegion[nMeshIdx]->vNonUnitToRegion;
					CFSphere spTracker = pHitTracker->GetBoundingSphere();
					pDir->Unitize().Mul( spTracker.m_fRadius * PLAYER_TARGET_ASSIST_TRACKER_MULTIPLIER );
					pCamera->GetFinalXfm()->m_MtxR.MulDir( *pDir );
					vFocalPoint.Set( spTracker.m_Pos + pDir->v3 );

					if ( FVis_apSortedMeshesInRegion[nMeshIdx]->fDistanceFromRegion > fReticleActiveRadius ) {
						// The mesh is outside of the targeting reticle, but we might use it for biasing
						if ( !pBiasToMesh && _TargetingAssistance( pPlayer ) ) {
							// Build the tracker skip list starting where this bot ends
							FWorld_nTrackerSkipListCount = nThisBotTrackerCount;
							pHitEntity->AppendTrackerSkipList();

							// This is a fast test, but does not calculate actual surface impact
							if ( !fworld_IsLineOfSightObstructed( &RayStartPoint, &vFocalPoint, FWorld_nTrackerSkipListCount, (const CFWorldTracker **)FWorld_apTrackerSkipList, m_pWorldMesh ) ) {
								// Line of sight is not blocked to this Bot
								pBiasToMesh = FVis_apSortedMeshesInRegion[nMeshIdx];
							}
						}

						FVis_apSortedMeshesInRegion[nMeshIdx] = NULL;
						continue;
					}
				}

				// Build the tracker skip list starting where this bot ends
				FWorld_nTrackerSkipListCount = nThisBotTrackerCount;
				pHitEntity->AppendTrackerSkipList();

				// This is a fast test, but does not calculate actual surface impact
				if ( fworld_IsLineOfSightObstructed( &RayStartPoint, &vFocalPoint, FWorld_nTrackerSkipListCount, (const CFWorldTracker **)FWorld_apTrackerSkipList, m_pWorldMesh ) ) {
					// Line of sight is blocked to this Bot
					FVis_apSortedMeshesInRegion[nMeshIdx] = NULL;
					continue;
				}

				pTargetPt->Set( vFocalPoint );

				// We've found a bot that collides, so let's use it
				if( _TargetingAssistance( pPlayer ) ) {
					pBiasToMesh = FVis_apSortedMeshesInRegion[nMeshIdx];
				}

				pTargetedMesh = FVis_apSortedMeshesInRegion[nMeshIdx]->pWorldMesh;

				break;
			}

			if ( !pTargetedMesh ) {
				// We didn't find a bot, so check the non-bot objects
				for ( nMeshIdx = 0; nMeshIdx < nMeshCountUnderReticle; nMeshIdx++ ) {
					if ( FVis_apSortedMeshesInRegion[nMeshIdx] == NULL ) {
						continue;
					}

					// Get a pointer to the next mesh
					pHitTracker = FVis_apSortedMeshesInRegion[nMeshIdx]->pWorldMesh;
					pHitEntity = (CEntity *)pHitTracker->m_pUser;

					// Make sure we don't target the weapons of friendly units
					if ( pHitEntity->TypeBits() & ENTITY_BIT_WEAPON ) {
						CBot *pOwner = ((CWeapon *)pHitEntity)->GetOwner();
						if ( pOwner && aiutils_IsFriendly( this, pOwner, NULL ) ) {
							// This is a friendly unit's weapon
							continue;
						}
					}

					// Let's test it for line of sight
					if ( FVis_apSortedMeshesInRegion[nMeshIdx]->fDistanceFromRegion <= 0.0001f ) {

						f32 fDist = FVis_apSortedMeshesInRegion[nMeshIdx]->spScreen.m_Pos.z + pHitTracker->GetBoundingSphere().m_fRadius + fCameraZAdjust;

						// The center of the target reticle is over the bounding sphere
						fEndPointReticleAdjust = fDist * fReticleX * pViewport->fTanHalfFOVX;
						ReticleAdjustX.Mul( pCamera->GetFinalXfm()->m_MtxR.m_vRight, fEndPointReticleAdjust );

						fEndPointReticleAdjust = fDist * fReticleY * pViewport->fTanHalfFOVY;
						ReticleAdjustY.Mul( pCamera->GetFinalXfm()->m_MtxR.m_vUp, fEndPointReticleAdjust );

						vFocalPoint.Mul( pCamera->GetFinalXfm()->m_MtxR.m_vFront, fDist ).Add( RayStartPoint ).Add( ReticleAdjustX ).Add( ReticleAdjustY );
					} else if ( FVis_apSortedMeshesInRegion[nMeshIdx]->fDistanceFromRegion <= fReticleActiveRadius ) {
						// The center of the reticle does not cover the center of the tracker, so choose the point on the tracker closest to it
						CFVec3A *pDir = &FVis_apSortedMeshesInRegion[nMeshIdx]->vNonUnitToRegion;
						CFSphere spTracker = pHitTracker->GetBoundingSphere();
						pDir->Unitize().Mul( spTracker.m_fRadius * PLAYER_TARGET_ASSIST_TRACKER_MULTIPLIER );
						pCamera->GetFinalXfm()->m_MtxR.MulDir( *pDir );
						vFocalPoint.Set( spTracker.m_Pos + pDir->v3 );
					} else {
						continue;
					}

					// This is a fast test, but does not calculate actual surface impact
					if ( fworld_IsLineOfSightObstructed( &RayStartPoint, &vFocalPoint, FWorld_nTrackerSkipListCount, (const CFWorldTracker **)FWorld_apTrackerSkipList, m_pWorldMesh ) ) {
						// Line of sight is blocked to this Bot
						continue;
					}

					pTargetPt->Set( vFocalPoint );

					// We've found an object that collides, so let's use it
					pTargetedMesh = (CFWorldMesh *)pHitTracker;

					break;
				}
			}
		}
	}

	// The ripper can sever wires, so handle wire targeting here...
	if( pWeapon && (pWeapon->Type() == CWeapon::WEAPON_TYPE_RIPPER) ) {
		CFVec3A TargetedPointOnWire_WS;
		CFWorldMesh *pTargetedWireWorldMesh;

		if( CFWire::Targeting( &RayStartPoint, &RayEndPoint, -fCameraZAdjust, &TargetedPointOnWire_WS, &pTargetedWireWorldMesh ) ) {
			if( pTargetedMesh ) {
				// We're targeting both a wire and something else. Pick the closest one...

				if( TargetedPointOnWire_WS.DistSq( RayStartPoint ) < pTargetPt->DistSq( RayStartPoint ) ) {
					// Wire is closer...
					*pTargetPt = TargetedPointOnWire_WS;
					pTargetedMesh = pTargetedWireWorldMesh;
				}
			} else {
				// The wire is the only targetable thing...
				*pTargetPt = TargetedPointOnWire_WS;
				pTargetedMesh = pTargetedWireWorldMesh;
			}
		}
	}

	// OK, the ColiseumMiniGame has special targets that I can hit
	if( CBot::m_bColiseumMiniGame ) {
		CFVec3A TargetedPointOnTarget_WS;
		CFWorldMesh *pTargetedTargetWorldMesh;

		if ( CColiseumMiniGame::Targeting( &RayStartPoint, &RayEndPoint, &TargetedPointOnTarget_WS, &pTargetedTargetWorldMesh ) ) {
			if( pTargetedMesh ) {
				// We're targeting both a target and something else. Pick the closest one...

				if( TargetedPointOnTarget_WS.DistSq( RayStartPoint ) < pTargetPt->DistSq( RayStartPoint ) ) {
					// Target is closer...
					*pTargetPt = TargetedPointOnTarget_WS;
					pTargetedMesh = pTargetedTargetWorldMesh;
				}
			} else {
				// The target is the only targetable thing...
				*pTargetPt = TargetedPointOnTarget_WS;
				pTargetedMesh = pTargetedTargetWorldMesh;
			}
		}
	}

	// If this player has targeting assistance, compute aim biasing here
	if ( _TargetingAssistance( pPlayer ) > 0.f ) {
		// Compute pitch and yaw biasing (note that this currently only works for bots - by setting the pBiasToMesh
		// pointer in the search of non-bot entities, it could apply to other objects as well)
		if ( pBiasToMesh ) {
			/////////////
			// Take a sample of the mesh's movement in screen space

			// Back out the sample that we are about to expire
			pPlayer->m_fYawVelAvg -= pPlayer->m_afFrameYawVel[pPlayer->m_nCurrentSample] * (1.f/PLAYER_AIM_BIASING_MESH_SAMPLES);
			pPlayer->m_fPitchVelAvg -= pPlayer->m_afFramePitchVel[pPlayer->m_nCurrentSample] * (1.f/PLAYER_AIM_BIASING_MESH_SAMPLES);

			if ( pPlayer->m_pBiasingMesh == pBiasToMesh->pWorldMesh ) {
				// If we have the same mesh targeted as last frame, we can calculate an inter frame sample.
				pPlayer->m_afFrameYawVel[pPlayer->m_nCurrentSample] = (((pBiasToMesh->spScreen.m_Pos.x - pPlayer->m_vBiasMeshPosLastFrame.x) * pViewport->fHalfFOVX) + pPlayer->m_fYawAdjust) * FLoop_fPreviousLoopOOSecs;
				pPlayer->m_afFramePitchVel[pPlayer->m_nCurrentSample] = (((pBiasToMesh->spScreen.m_Pos.y - pPlayer->m_vBiasMeshPosLastFrame.y) * -pViewport->fHalfFOVY) + pPlayer->m_fPitchAdjust) * FLoop_fPreviousLoopOOSecs;
			} else {
				// Otherwise we need to wait a frame because we have a new mesh.
				pPlayer->m_afFrameYawVel[pPlayer->m_nCurrentSample] *= 0.5f;
				pPlayer->m_afFramePitchVel[pPlayer->m_nCurrentSample] *= 0.5f;
				// We've detected a new mesh, so start the calculations over
				pPlayer->m_pBiasingMesh = pBiasToMesh->pWorldMesh;
			}

			// Add this sample into the average
			pPlayer->m_fYawVelAvg += pPlayer->m_afFrameYawVel[pPlayer->m_nCurrentSample] * (1.f/PLAYER_AIM_BIASING_MESH_SAMPLES);
			pPlayer->m_fPitchVelAvg += pPlayer->m_afFramePitchVel[pPlayer->m_nCurrentSample] * (1.f/PLAYER_AIM_BIASING_MESH_SAMPLES);

			// On to the next sample
			pPlayer->m_nCurrentSample++;
			if ( pPlayer->m_nCurrentSample == PLAYER_AIM_BIASING_MESH_SAMPLES ) {
				pPlayer->m_nCurrentSample = 0;
			}

			/////////////
			// Calculate the yaw and pitch adjustments to be applied to the player movement:

			// Modify effect based on the proximity of the mesh to the reticle region.
			f32 fReticleProximityAdjust = fmath_Cos( fmath_Div( pBiasToMesh->fDistanceFromRegion + pBiasToMesh->spScreen.m_fRadius, 
													(fReticleActiveRadius * PLAYER_AIM_BIASING_RETICLE_MULTIPLIER) + pBiasToMesh->spScreen.m_fRadius ) * FMATH_HALF_PI );
			fReticleProximityAdjust *= 1.3f;
			FMATH_CLAMP( fReticleProximityAdjust, 0.f, 1.f );
			FASSERT( fReticleProximityAdjust >= 0.f );

			// Modify effect based on the current movement of the player
			f32 fMostMovement = fmath_Abs( m_fControls_AimDown ) - 0.3f;
			f32 fTest = fmath_Abs( m_fControls_RotateCW ) - 0.3f;
			if ( fTest > fMostMovement ) {
				fMostMovement = fTest;
			}
			fTest = fmath_Abs( m_fControlsHuman_StrafeRight * m_fClampedNormSpeedAlongSurface_WS ) - 0.2f;
			if ( fTest > fMostMovement ) {
				fMostMovement = fTest;
			}

			// Set the adjustments to be applied
			f32 fPlayerMovementAdjust = FMATH_MAX( fMostMovement, PLAYER_MINIMUM_YAW_BIAS_WEIGHT );
			pPlayer->m_fYawAdjust = pPlayer->m_fYawVelAvg * fReticleProximityAdjust * fPlayerMovementAdjust * FLoop_fPreviousLoopSecs * pPlayer->m_fUnitTargetingAssistance;
			fPlayerMovementAdjust = FMATH_MAX( fMostMovement - 0.1f, PLAYER_MINIMUM_PITCH_BIAS_WEIGHT );
			pPlayer->m_fPitchAdjust = pPlayer->m_fPitchVelAvg * fReticleProximityAdjust * fPlayerMovementAdjust * FLoop_fPreviousLoopSecs * pPlayer->m_fUnitTargetingAssistance;

			// Record the current position for calculations next frame
			pPlayer->m_vBiasMeshPosLastFrame = pBiasToMesh->spScreen.m_Pos;

		} else {
			pPlayer->m_pBiasingMesh = NULL;
			pPlayer->m_fYawAdjust = 0.f;
			pPlayer->m_fPitchAdjust = 0.f;
			pPlayer->m_vBiasMeshPosLastFrame.Set( 0.f, 0.f, 0.f );
		}
	} else {
		// Assistance is off (or suspended while aiming with the mouse): drop any earlier bias.
		pPlayer->m_pBiasingMesh = NULL;
		pPlayer->m_fYawAdjust = 0.f;
		pPlayer->m_fPitchAdjust = 0.f;
	}

	if ( pTargetedMesh ) {
		// We found a targetable mesh or wire, so set the appropriate data

		pReticle->ColorScheme_SelectActive();

		if( pTargetedMesh->m_nUser == MESHTYPES_ENTITY ) {
			// Do this only for targeted entities...

			m_pTargetedMesh = pTargetedMesh;

			if( pWeapon ) { 
				pWeapon->SetTargetedEntity( (CEntity *)pTargetedMesh->m_pUser );
			}
		} else {
			// Not targeting an entity...
			m_pTargetedMesh = NULL;
		}
	} else {
		// We didn't find a targetable mesh
		m_pTargetedMesh = NULL;
		pReticle->ColorScheme_SelectInactive();
	}
}


//
// ComputeHumanTargetPoint_WS only calculates a general target point.  This function will take the current targeted 
// mesh and make sure that the targeted point is the closest damageable point to the center of the screen reticle
//
BOOL CBot::FocusHumanTargetPoint_WS( CFVec3A *pFocusedTargetPt, CWeapon *pWeapon ) { 
	FASSERT( IsCreated() ); 
	FASSERT( pFocusedTargetPt );

	// It seems that some confused code occasionally calls this with a non-human...
	if (m_nPossessionPlayerIndex < 0)
		return FALSE;

	// Next we check the results of the reticle proximity code (the results are placed in m_pTargetMesh).

	CPlayer *pPlayer = &Player_aPlayer[ m_nPossessionPlayerIndex ];

	if ( _TargetingAssistance( pPlayer ) == 0.f ) {
		// This player does not have targeting assistance enabled, so don't do anything
		return FALSE;
	}

	if ( !m_pTargetedMesh ) {
		// The Bot is currently not targeting a mesh, so there is nothing to fine tune
		return FALSE;
	}

	// If there is a targetable entity under the center of the reticle, then we will use that entity...
	if ( pPlayer->m_bReticleCenterOverTarget ) {
		return FALSE;
	}
/*
	CFWorldMesh *pWorldMesh;
	ComputeHumanReticleCenterCollision( pWeapon, pFocusedTargetPt, &pWorldMesh );
	if ( pWorldMesh && pWorldMesh->m_pUser && ((CEntity *)pWorldMesh->m_pUser)->IsTargetable() ) {
		m_pTargetedMesh = pWorldMesh;        
	}
*/
	if ( pWeapon && (pWeapon->m_pInfo->nInfoFlags & CWeapon::INFOFLAG_NO_AUTO_TARGETING) ) {
		// This weapon does not allow auto-targeting, so the mesh must be under the reticle center
		return FALSE;
	}

	CEntity *pHitEntity = (CEntity *)m_pTargetedMesh->m_pUser;

	if ( !(pHitEntity->TypeBits() & ENTITY_BIT_BOT) || (pHitEntity->TypeBits() & (ENTITY_BIT_VEHICLE|ENTITY_BIT_SITEWEAPON)) ) {
		// We're only going to focus targeting on certain bots
		return FALSE;
	}

	CBot *pBot = (CBot *)pHitEntity;

	if( pBot->m_pShield && (pBot->m_pShield->NormHealth() > 0.0f) ) {
		// Shield is up on this bot. Don't target limbs
		return FALSE;
	}

	// Get camera and viewport info...
	CFCamera *pCamera = gamecam_GetActiveCamera();
	const FViewport_t *pViewport = pCamera->GetViewport();

	// Get the reticle
	CReticle *pReticle = &pPlayer->m_Reticle;

	f32 fDistSq;
	f32 fClosest = FMATH_MAX_FLOAT;
	CFVec2 vDiff, vScreenXY( pReticle->GetNormOriginX(), pReticle->GetNormOriginY() );
	f32 fReticleActiveRadius = pReticle->GetScreenSpaceRadius() * 0.75f;  // Applying a little fudge factor to make it more accurate

	// Adjust y to be 1 to 1 ratio with X
	vScreenXY.y *= pViewport->fAspectHOW;

	CFSphere spBone, spResult;
	FASSERT( ((CBot *)pHitEntity)->m_pWorldMesh->m_pMesh );
	CFWorldMesh *pWorldMesh = ((CBot *)pHitEntity)->m_pWorldMesh;

	// We need to see if there are targetable parts in the reticle
	CBotPartMgr *pPartMgr = ((CBot *)pHitEntity)->GetPartMgr();
	if ( !pPartMgr || !pPartMgr->IsCreated() || !pPartMgr->GetBotPartPool() ) {
		// This bot does not have targetable parts, so let's find the closest bone

		u32 i;
		for ( i = 0; i < pWorldMesh->m_pMesh->nUsedBoneCount; i++ )
		{
			pWorldMesh->GetBoneSphere_WS( i, &spBone );

			fviewport_ComputeScreenPointAndSize_WS( pViewport, &pCamera->GetFinalXfm()->m_MtxF, &spBone, &spResult );

			// Convert y to be the same scale as x
			spResult.m_Pos.y *= pViewport->fAspectHOW;

			vDiff.x = (vScreenXY.x - spResult.m_Pos.x);
			vDiff.y = (vScreenXY.y - spResult.m_Pos.y);
			fDistSq = (vDiff.x * vDiff.x) + (vDiff.y * vDiff.y);
			if ( fDistSq < fClosest ) {
				fClosest = fDistSq;
				pFocusedTargetPt->Set( spBone.m_Pos );
			}
			// It's possible (and likely) that this code will result in some bending of the
			// fire vector slightly outside of the reticle (when the tracker sphere and the
			// reticle just barely intersect).  But to correct for this would require a
			// sqrt and some additional tests.  Since the error might not be noticeable, we
			// should start out with this faster method.
		}

		if ( fClosest == FMATH_MAX_FLOAT ) {
			return FALSE;
		}
	} else {
		// This bot has a part list, so let's check it for targeting

		u32 nBoneDefCount = 0;
		u32 nBoneDefIdx; 

		const CBotPartPool::CBoneDef *paBoneDef = pPartMgr->GetBotPartPool()->GetBoneInfoArray( &nBoneDefCount );
		for ( nBoneDefIdx = 0; nBoneDefIdx < nBoneDefCount; nBoneDefIdx++ )	{
			FASSERT( paBoneDef[nBoneDefIdx].m_nBoneIndex < pWorldMesh->m_pMesh->nBoneCount );

			// Test each bone that is targetable to see if it is in the reticle
			u8 nBoneIdx = paBoneDef[nBoneDefIdx].m_nBoneIndex;

			pWorldMesh->GetBoneSphere_WS( nBoneIdx, &spBone );

			fviewport_ComputeScreenPointAndSize_WS( pViewport, &pCamera->GetFinalXfm()->m_MtxF, &spBone, &spResult );

			// Convert y to be the same scale as x
			spResult.m_Pos.y *= pViewport->fAspectHOW;

			vDiff.x = (vScreenXY.x - spResult.m_Pos.x);
			vDiff.y = (vScreenXY.y - spResult.m_Pos.y);
			fDistSq = (vDiff.x * vDiff.x) + (vDiff.y * vDiff.y);
			if ( fDistSq < fClosest ) {
				fClosest = fDistSq;
				pFocusedTargetPt->Set( spBone.m_Pos );
			}
			// It's possible (and likely) that this code will result in some bending of the
			// fire vector slightly outside of the reticle (when the tracker sphere and the
			// reticle just barely intersect).  But to correct for this would require a
			// sqrt and some additional tests.  Since the error might not be noticeable, we
			// should start out with this faster method.
		}

		if ( fClosest == FMATH_MAX_FLOAT ) {
			return FALSE;
		}
	}

	return TRUE;
}


//this routine will direct the spotlight for a player controlled bot in the direction of the mount.
//if the player does not touch the controls for a certain amount of time, then the
//light will assume the inherited bone position of the light.
void CBot::_OverrideSpotLight( void ) {

	//first, only do this logic if the spotlight is not OFF.
	if( m_eSpotLightState == LIGHT_STATE_OFF ) {
		return;
	}

	////////////////////////////////////////////////////////
	//figure out here what our new override state should be
	//and manage the OverrideTimer
	////////////////////////////////////////////////////////
	LightOverrideState_e eNewOverrideState = _DetermineNewSpotLightOverrideState();
	if( m_eSpotLightOverrideState != eNewOverrideState ) {
		m_fSpotLightOverrideTimer = 0.0f;
	} else {
		m_fSpotLightOverrideTimer += FLoop_fPreviousLoopSecs;
	}

	/////////////////////////////////////////////////////////////////
	// calculate the new position state of the spotlight...
	/////////////////////////////////////////////////////////////////
	CFMtx43 FinalMtx;
	switch( eNewOverrideState )
	{
		case LIGHTOVERRIDE_STATE_BLENDING_TO: 
		{
			//we are blending to an overridden position
			if( m_eSpotLightOverrideState == LIGHTOVERRIDE_STATE_NOT_OVERRIDDEN ) {
				m_pSpotLight->SetOverrideFlags( FWORLDATTACHEDLIGHT_OVERRIDE_ORIENTATION );
			}
			CFQuatA qMountQuat, qBoneQuat, qFinalQuat;
			CFMtx43 TempMtx;
			CFMtx43A TempMtx2;

			qMountQuat.BuildQuat( m_MtxToWorld );

			if( m_fFlipPitch ) {
				CFQuatA qFlipQuat;
				qFlipQuat.BuildQuat( m_pWorldMesh->m_Xfm.m_MtxF.m_vRight, m_fFlipPitch );
				qFlipQuat.Mul( qMountQuat );
				qMountQuat = qFlipQuat;
			}

			m_pSpotLight->GetWSAttachedOrientation( &TempMtx );
			qBoneQuat.BuildQuat( TempMtx );

			//calculate our unit slerp value over a quarter of a second...
			f32 fUnitBlendVal = m_fSpotLightOverrideTimer * 4.0f;
			if( fUnitBlendVal > 1.0f ) {
				fUnitBlendVal = 1.0f;
			}

			//now, slerp between the two quaternians
			qFinalQuat.ReceiveSlerpOf( fUnitBlendVal, qBoneQuat, qMountQuat );
			qFinalQuat.BuildMtx( TempMtx2 );

			//now set the overridden position
			FinalMtx.m_vFront = TempMtx2.m_vFront.v3;
			FinalMtx.m_vRight = TempMtx2.m_vRight.v3;
			FinalMtx.m_vUp = TempMtx2.m_vUp.v3;
		}
		break;

		case LIGHTOVERRIDE_STATE_OVERRIDDEN: 
		{
			//the light is being controlled by the player
			if( m_eSpotLightOverrideState == LIGHTOVERRIDE_STATE_NOT_OVERRIDDEN ) {
				m_pSpotLight->SetOverrideFlags( FWORLDATTACHEDLIGHT_OVERRIDE_ORIENTATION );
			}
			if( m_fFlipPitch ) {
				CFQuatA FlipQuat, OrientQuat;
				CFMtx43A TempMtx;

				FlipQuat.BuildQuat( m_pWorldMesh->m_Xfm.m_MtxF.m_vRight, m_fFlipPitch );
				OrientQuat.BuildQuat( m_MtxToWorld );

				FlipQuat.Mul( OrientQuat );
				FlipQuat.BuildMtx( TempMtx );

				FinalMtx.m_vFront = TempMtx.m_vFront.v3;
				FinalMtx.m_vRight = TempMtx.m_vRight.v3;
				FinalMtx.m_vUp = TempMtx.m_vUp.v3;
			} else {
				FinalMtx.m_vFront = m_MtxToWorld.m_vFront.v3;
				FinalMtx.m_vRight = m_MtxToWorld.m_vRight.v3;
				FinalMtx.m_vUp = m_MtxToWorld.m_vUp.v3;
			}
		}
		break;

		case LIGHTOVERRIDE_STATE_BLENDING_FROM: 
		{
			//we are blending from the override position to the Light's attached position
			if( m_eSpotLightOverrideState == LIGHTOVERRIDE_STATE_NOT_OVERRIDDEN ) {
				m_pSpotLight->SetOverrideFlags( FWORLDATTACHEDLIGHT_OVERRIDE_ORIENTATION );
			}

			CFQuatA qMountQuat, qBoneQuat, qFinalQuat;
			CFMtx43 TempMtx;
			CFMtx43A TempMtx2;

			qMountQuat.BuildQuat( m_MtxToWorld );

			if( m_fFlipPitch ) {
				CFQuatA qFlipQuat;
				qFlipQuat.BuildQuat( m_pWorldMesh->m_Xfm.m_MtxF.m_vRight, m_fFlipPitch );
				qFlipQuat.Mul( qMountQuat );
				qMountQuat = qFlipQuat;
			}

			m_pSpotLight->GetWSAttachedOrientation( &TempMtx );
			qBoneQuat.BuildQuat( TempMtx );

			//calculate our unit slerp value over a quarter of a second...
			f32 fUnitBlendVal = m_fSpotLightOverrideTimer * 4.0f;
			if( fUnitBlendVal > 1.0f ) {
				fUnitBlendVal = 1.0f;
			}

			//now, slerp between the two quaternians
			qFinalQuat.ReceiveSlerpOf( fUnitBlendVal, qMountQuat, qBoneQuat );
			qFinalQuat.BuildMtx( TempMtx2 );

			//now set the overridden position
			FinalMtx.m_vFront = TempMtx2.m_vFront.v3;
			FinalMtx.m_vRight = TempMtx2.m_vRight.v3;
			FinalMtx.m_vUp = TempMtx2.m_vUp.v3;
		}
		break;

		case LIGHTOVERRIDE_STATE_NOT_OVERRIDDEN: 
			//the light is being controlled by the bot
			if( m_eSpotLightOverrideState != LIGHTOVERRIDE_STATE_NOT_OVERRIDDEN ) {
				m_pSpotLight->ClearOverrideFlags( FWORLDATTACHEDLIGHT_OVERRIDE_ORIENTATION );
			}
		break;
	}

	if( eNewOverrideState != LIGHTOVERRIDE_STATE_NOT_OVERRIDDEN ) {
		//set the new orientation of the light
		m_pSpotLight->SetWSOverrideVars( NULL, &FinalMtx );
	}

	m_eSpotLightOverrideState = eNewOverrideState;
}


CBot::LightOverrideState_e CBot::_DetermineNewSpotLightOverrideState( void ) {

	LightOverrideState_e eCommandedState = m_eSpotLightOverrideState; //default to our current state

	////////////////////////////////////////////////////////
	// First, look at all the situations which would dictate
	// switching back to bot spotlight control.
	////////////////////////////////////////////////////////

	// if human is not in control, then no override.
	if ( m_nPossessionPlayerIndex == -1 ) {
		eCommandedState = LIGHTOVERRIDE_STATE_BLENDING_FROM;
		goto _COMMANDEDSTATE_DETERMINED;
	}

	// don't override if a vehicle
	if ( TypeBits() & ENTITY_BIT_VEHICLE ) {
		eCommandedState = LIGHTOVERRIDE_STATE_BLENDING_FROM;
		goto _COMMANDEDSTATE_DETERMINED;
	}

	//human is in control...
	// DONT override when flying
	if (m_fUnitFlyBlend != 0.0f) {
		eCommandedState = LIGHTOVERRIDE_STATE_BLENDING_FROM;
		goto _COMMANDEDSTATE_DETERMINED;
	}
	// DONT override when cabling
	if (m_fUnitCableBlend != 0.0f) {
		eCommandedState = LIGHTOVERRIDE_STATE_BLENDING_FROM;
		goto _COMMANDEDSTATE_DETERMINED;
	}
	// DONT override when driving
	if (m_pDrivingVehicle != NULL) {
		eCommandedState = LIGHTOVERRIDE_STATE_BLENDING_FROM;
		goto _COMMANDEDSTATE_DETERMINED;
	}
	// DONT override if a user animation is playing
	if( UserAnim_IsLocked() ) {
		eCommandedState = LIGHTOVERRIDE_STATE_BLENDING_FROM;
		goto _COMMANDEDSTATE_DETERMINED;
	}
	// DONT override if a cutscene is being played...
	if( m_bCutscenePlaying ) {
		eCommandedState = LIGHTOVERRIDE_STATE_BLENDING_FROM;
		goto _COMMANDEDSTATE_DETERMINED;
	}
	// DONT override if bot is facing player...
	if( IsReverseFacingMode() ) {
		eCommandedState = LIGHTOVERRIDE_STATE_BLENDING_FROM;
		goto _COMMANDEDSTATE_DETERMINED;
	}

	if( ( m_eSpotLightOverrideState == LIGHTOVERRIDE_STATE_OVERRIDDEN ) &&
		( m_fSpotLightOverrideTimer > 15.0f ) ) {
		//there have been 15 seconds of non-activity...
		//transition from an overridden state
		eCommandedState = LIGHTOVERRIDE_STATE_BLENDING_FROM;
		goto _COMMANDEDSTATE_DETERMINED;
	}

	////////////////////////////////////////////////////////
	// now, look at all the situations which would dictate
	// control spotlight control being overridden
	////////////////////////////////////////////////////////

	// override if fire buttons depressed AND ammo's available
	if ( (m_apWeapon[0] && (m_fControls_Fire1 > 0.0f) && m_apWeapon[0]->GetClipAmmo()) || 
		(m_apWeapon[1] && (m_fControls_Fire2 > 0.0f) && m_apWeapon[1]->GetClipAmmo()) ) {
		eCommandedState = LIGHTOVERRIDE_STATE_BLENDING_TO;
		goto _COMMANDEDSTATE_DETERMINED;
	}

	// override if main buttons depressed
	if (m_bControls_Jump || m_bControls_Melee) {
		eCommandedState = LIGHTOVERRIDE_STATE_BLENDING_TO;
		goto _COMMANDEDSTATE_DETERMINED;
	}

	// override when walking
	if (m_fUnitWalkBlend != 0.0f) {
		eCommandedState = LIGHTOVERRIDE_STATE_BLENDING_TO;
		goto _COMMANDEDSTATE_DETERMINED;
	}
	// override when running
	if (m_fUnitRunBlend != 0.0f) {
		eCommandedState = LIGHTOVERRIDE_STATE_BLENDING_TO;
		goto _COMMANDEDSTATE_DETERMINED;
	}

_COMMANDEDSTATE_DETERMINED:

	if( ( eCommandedState == LIGHTOVERRIDE_STATE_BLENDING_TO ) &&
		( m_eSpotLightOverrideState == LIGHTOVERRIDE_STATE_OVERRIDDEN ) ) {
		//we are already in an overridden state, and we have been
		//commanded to re-enter an overridden state, so
		//just stay in an overridden state and reset the timer, which at this
		//point represents an INACTIVITY timer.
		eCommandedState = m_eSpotLightOverrideState;
		m_fSpotLightOverrideTimer = 0.0f; 
	}

	if( ( eCommandedState == LIGHTOVERRIDE_STATE_BLENDING_FROM ) &&
		( m_eSpotLightOverrideState == LIGHTOVERRIDE_STATE_NOT_OVERRIDDEN ) ) {
		//we are already in a non-overridden state, and we have been
		//commanded to re-enter a non-overridden state, so
		//just stay in an non-overridden state.
		eCommandedState = m_eSpotLightOverrideState;
	}

	if( ( eCommandedState == LIGHTOVERRIDE_STATE_BLENDING_TO ) &&
		( m_eSpotLightOverrideState == eCommandedState  ) && 
		( m_fSpotLightOverrideTimer > 0.25f ) ) {
		//we have been in the blend to state for the blend duration
		//it's time to transition into full override state.
		eCommandedState = LIGHTOVERRIDE_STATE_OVERRIDDEN;
	}

	if( ( eCommandedState == LIGHTOVERRIDE_STATE_BLENDING_FROM ) &&
		( m_eSpotLightOverrideState == eCommandedState  ) && 
		( m_fSpotLightOverrideTimer > 0.25f ) ) {
		//we have been in the blend to state for the blend duration
		//it's time to transition into full override state.
		eCommandedState = LIGHTOVERRIDE_STATE_NOT_OVERRIDDEN;
	}

	return eCommandedState;
}


void CBot::PlaySound( CFSoundGroup *pSoundGroup, f32 fVolMult, f32 fPitchMult, f32 fRadiusOverride, CFAudioEmitter **ppUserAudioEmitter, BOOL bForce2D ) const {
	if( m_nPossessionPlayerIndex >= 0 ) {
		// Bot is being controlled by a player. Play a 2D sound and inform AI about it...
		CFSoundGroup::PlaySound( pSoundGroup, TRUE, &m_MtxToWorld.m_vPos, m_nPossessionPlayerIndex, TRUE, fVolMult, fPitchMult, fRadiusOverride, ppUserAudioEmitter );
	} else {
		// Bot is not being controlled by a player. Play a 3D sound and don't tell AI about it...
		CFSoundGroup::PlaySound( pSoundGroup, (bForce2D)?TRUE:FALSE, &m_MtxToWorld.m_vPos, -1, FALSE, fVolMult, fPitchMult, fRadiusOverride, ppUserAudioEmitter );
	}
}


CFAudioEmitter *CBot::AllocAndPlaySound( CFSoundGroup *pSoundGroup, f32 fVolMult, f32 fPitchMult, f32 fRadiusOverride ) const {
	if( m_nPossessionPlayerIndex >= 0 ) {
		// Bot is being controlled by a player. Play a 2D sound and inform AI about it...
		return CFSoundGroup::AllocAndPlaySound( pSoundGroup, TRUE, &m_MtxToWorld.m_vPos, fVolMult, fPitchMult, fRadiusOverride );
	} else {
		// Bot is not being controlled by a player. Play a 3D sound and don't tell AI about it...
		return CFSoundGroup::AllocAndPlaySound( pSoundGroup, FALSE, &m_MtxToWorld.m_vPos, fVolMult, fPitchMult, fRadiusOverride );
	}
}


// NKM - Removes all attached Swarmers from the bot
void CBot::DetachAllSwarmers( void ) {
	CEntity *pChildEntity = GetFirstChild();
	CEntity *pNext = NULL;

	while( pChildEntity ) {
		pNext = GetNextChild( pChildEntity );

		if( pChildEntity->TypeBits() & ENTITY_BIT_BOTSWARMER ) {
			pChildEntity->DetachFromParent();
		}

		pChildEntity = pNext;
	}

	m_uNumAttachedSwarmers = 0;
}


// Constructs the bot's damager data and stores it in pDestDamager.
// If pDestDamager is NULL, the data is stored in CDamageForm::m_TempDamager.
void CBot::ConstructDamagerData( CDamageForm::Damager_t *pDestDamager ) {
	if( pDestDamager == NULL ) {
		pDestDamager = &CDamageForm::m_TempDamager;
	}

	pDestDamager->nDamagerPlayerIndex = m_nPossessionPlayerIndex;
	pDestDamager->pBot = this;
	pDestDamager->pEntity = this;
	pDestDamager->pWeapon = NULL;
}


void CBot::StartBotTalkByName( cchar *pszBTAName ) {
	CBotTalkInst *pInst = CTalkSystem2::BTIPool_Get(pszBTAName);
	if (pInst) {
		if( CTalkSystem2::SubmitTalkRequest( (CBotTalkInst *)(pInst), this ) ) {
			pInst->SetTalkDoneCB( _StopBotTalkCB );
		}
	}
}


void CBot::_StopBotTalkCB( CBotTalkInst *pTalkInst, CBot *pBot ) {
	CTalkSystem2::BTIPool_Recycle( pTalkInst );
}





// Public method of enabling/disabling drawing of reticle.
// if bEnable is TRUE, then current and subsequent settings of
// the protected function Reticle_EnableDraw() will be obeyed.
// If bEnable is FALSE, then reticle drawing is disabled regardless
// of current or subsequent Reticle_EnableDraw() settings.
void CBot::ReticleEnable( BOOL bEnable ) {
	FASSERT( IsCreated());
	if (m_nPossessionPlayerIndex < 0) {
		// I'm a drone, I have no reticle, goodbye
		return;
	}

	CReticle *pReticle = &Player_aPlayer[ m_nPossessionPlayerIndex ].m_Reticle;
	if( bEnable ) {
		pReticle->ClearReticleOverride();
		pReticle->EnableDraw(TRUE);
		// make sure it's the right type of reticle
		if (m_apWeapon[0])
			m_apWeapon[0]->UpdateReticle();
	} else {
		pReticle->EnableDraw(FALSE);
		pReticle->SetReticleOverride();
	}
}


BOOL CBot::IsReticleEnabled( void ) {
	FASSERT( IsCreated());
	if( m_nPossessionPlayerIndex < 0 ) {
		// I'm a drone, I have no reticle, goodbye
		return TRUE;
	}

	CReticle *pReticle = &Player_aPlayer[ m_nPossessionPlayerIndex ].m_Reticle;
	if( pReticle->IsReticleOverriden() ) {
		// I'm in forced mode...

		if( pReticle->IsDrawEnabled() == FALSE ) {
			return FALSE;
		}
	}
	return TRUE;
}


// Returns the distance-squared from this bot to the nearest human
// player in the game.
//
// If no players are in the game, or if all players don't have a non-NULL
// m_pEntityCurrent, FMATH_MAX_FLOAT is returned.
f32 CBot::ComputeDistSquaredToNearestHumanPlayer( void ) {
	s32 i;
	f32 fSmallestDist2, fDist2;

	fSmallestDist2 = FMATH_MAX_FLOAT;

	for( i=0; i<CPlayer::m_nPlayerCount; ++i ) {
		if( Player_aPlayer[i].m_pEntityCurrent ) {
			fDist2 = MtxToWorld()->m_vPos.DistSq( Player_aPlayer[i].m_pEntityCurrent->MtxToWorld()->m_vPos );

			if( fDist2 < fSmallestDist2 ) {
				fSmallestDist2 = fDist2;
			}
		}
	}

	return fSmallestDist2;
}


BOOL CBot::TryToBreakLimb( const CFVec3A *pvPos_WS/*=NULL*/ ) {
	FASSERT( IsCreated() );

	if( !m_pPartMgr || !m_pPartMgr->IsCreated() ) {
		return FALSE;
	}

	s32 nBoneIdx = -1;

	if( m_uLimbBreakCounter == 1 ) {
		BreakLimb( pvPos_WS );
		m_uLimbBreakCounter = m_pBotInfo_Gen->uNumHitsToBreakLimb;
		return TRUE;
	} else {
		if( m_uLimbBreakCounter > 1 ) {
			m_uLimbBreakCounter--;
		}
		return FALSE;
	}
}


void CBot::BreakLimb( const CFVec3A *pvPos_WS/*=NULL*/ ) {
	FASSERT( IsCreated() );
	
	s32 nLimb = -1;
	u32 uBoneDefCount = 0;
	f32 fVal;
	u32 i;
	CFSphere spBone;
	f32 fClosestBoneDist2 = FMATH_MAX_FLOAT;
	u32 uTmpLimb;


	if( !m_pPartMgr || !m_pPartMgr->IsCreated() ) {
		return;
	}

	// try to find a limb close to the given point
	if( pvPos_WS ) {
		const CBotPartPool::CBoneDef *paBoneDef = m_pPartMgr->GetBotPartPool()->GetBoneInfoArray( &uBoneDefCount );
		for( i=0; i<uBoneDefCount; i++ ) {
			FASSERT( paBoneDef[i].m_nBoneIndex < m_pWorldMesh->m_pMesh->nBoneCount );
			m_pWorldMesh->GetBoneSphere_WS( i, &spBone );
			fVal = pvPos_WS->DistSq( spBone.m_Pos );
			if( (fVal < fClosestBoneDist2) ) {
				uTmpLimb = m_pPartMgr->GetLimbTypeFromBoneIndex( i );
				if( (uTmpLimb < m_pPartMgr->GetLimbTypeCount()) && 
					!m_pPartMgr->IsLimbInvincible( uTmpLimb ) && 
					m_pPartMgr->GetLimbState( uTmpLimb ) == CBotPartMgr::LIMB_STATE_INTACT ) {
					fClosestBoneDist2 = fVal;
					nLimb = m_pPartMgr->GetLimbTypeFromBoneIndex( i );
				}
			}
		}
	}

	if( nLimb < 0 ) {
		m_pPartMgr->SetRandomLimb_Dangle();
	} else {
		m_pPartMgr->SetState_Dangle( (u32)nLimb );
	}
}

void CBot::BreakAllLimbs( void ) {
	FASSERT( IsCreated() );

	if( m_pPartMgr && m_pPartMgr->IsCreated() ) {
		m_pPartMgr->SetAllLimbs_Dangle();
	}
}


#if 0
	// This bot has a part list, so let's check it for targeting
	u32 nBoneDefCount = 0;
	u32 nBoneDefIdx; 
	f32 fDistSq;
	CFSphere spBone, spResult;
	f32 fClosest = FMATH_MAX_FLOAT;

	FASSERT( ((CBot *)pHitEntity)->m_pWorldMesh->m_pMesh );

	const CBotPartPool::CBoneDef *paBoneDef = pPartMgr->GetBotPartPool()->GetBoneInfoArray( &nBoneDefCount );
	for ( nBoneDefIdx = 0; nBoneDefIdx < nBoneDefCount; nBoneDefIdx++ )	{
		FASSERT( paBoneDef[nBoneDefIdx].m_nBoneIndex < ((CBot *)pHitEntity)->m_pWorldMesh->m_pMesh->nBoneCount );

		// Test each bone that is targetable to see if it is in the reticle
		u8 nBoneIdx = paBoneDef[nBoneDefIdx].m_nBoneIndex;

		((CBot *)pHitEntity)->m_pWorldMesh->GetBoneSphere_WS( nBoneIdx, &spBone );

		fviewport_ComputeScreenPointAndSize_WS( pViewport, &pCamera->GetFinalXfm()->m_MtxF, &spBone, &spResult );

		// Convert y to be the same scale as x
		spResult.m_Pos.y *= pViewport->fAspectHOW;

		vDiff.x = (vScreenXY.x - spResult.m_Pos.x);
		vDiff.y = (vScreenXY.y - spResult.m_Pos.y);
		fDistSq = (vDiff.x * vDiff.x) + (vDiff.y * vDiff.y);
		if ( fDistSq < fClosest ) {
			fClosest = fDistSq;
			pFocusedTargetPt->Set( spBone.m_Pos );
		}
		// It's possible (and likely) that this code will result in some bending of the
		// fire vector slightly outside of the reticle (when the tracker sphere and the
		// reticle just barely intersect).  But to correct for this would require a
		// sqrt and some additional tests.  Since the error might not be noticeable, we
		// should start out with this faster method.
	}

	if ( fClosest == FMATH_MAX_FLOAT ) {
		return FALSE;
	}

#endif


BOOL CBot::RespawnEffect_Init( f32 fParticleIntensity, s32 nMeshAttachBone, const CFVec3A &vMeshAttachPos, f32 fMeshStartScale, f32 fMeshFinishScale ) {
	FASSERT( !m_bRespawnEffectInitialized );
	FASSERT( m_anAnimStackIndex[ASI_RESPAWN] != -1 );

	FMeshInit_t meshinit;
	FResFrame_t frame = fres_GetFrame();
	FMesh_t *pMesh;
    
	m_bRespawnEffectInitialized = TRUE;
	m_fRespawnEffectPartIntensity = fParticleIntensity;
	m_nRespawnEffectMeshBoneIdx = nMeshAttachBone;
	m_vRespawnEffectMeshPos = vMeshAttachPos;
	m_fRespawnEffectMeshStartScale = fMeshStartScale;
	m_fRespawnEffectMeshFinishScale = fMeshFinishScale;

	// load the mesh
	pMesh = (FMesh_t*)fresload_Load( FMESH_RESTYPE, _RESPAWN_MESH );
	if( pMesh == NULL ) {
		DEVPRINTF( "CBot::RespawnEffect_Init():  Could not load mesh %s\n", _RESPAWN_MESH );
		goto _ExitWithError;
	}

	// Create world mesh...
	m_pRespawnEffectMesh = fnew CFWorldMesh;
	if( m_pRespawnEffectMesh == NULL ) {
		DEVPRINTF( "CBot::RespawnEffect_Init(): Not enough memory to create CFWorldMesh.\n" );
		goto _ExitWithError;
	}

	// Init the world mesh...
	meshinit.pMesh = pMesh;
	meshinit.nFlags = 0;
	meshinit.fCullDist = FMATH_MAX_FLOAT;
	meshinit.Mtx.Identity();
    
	m_pRespawnEffectMesh->SetMeshAlpha( 1.0f );
	m_pRespawnEffectMesh->Init( &meshinit );
	m_pRespawnEffectMesh->RemoveFromWorld();


	// Init the particle...
	m_hRespawnEffectPartDef = (FParticle_DefHandle_t)fresload_Load( FPARTICLE_RESTYPE, _RESPAWN_PARTICLE );
	if( m_hRespawnEffectPartDef == FPARTICLE_INVALID_HANDLE ) {
		DEVPRINTF( "CBot::RespawnEffect_Init():  Could not load particle %s\n", _RESPAWN_PARTICLE );
	}


	SetControlValue( ASI_RESPAWN, 0.0f );
	ZeroTime( ASI_RESPAWN );

	return TRUE;

_ExitWithError:
	FASSERT_NOW;
	RespawnEffect_Destroy();
	fres_ReleaseFrame( frame );
	return FALSE;
}


void CBot::RespawnEffect_Start( void ) {
	FASSERT( m_bRespawnEffectInitialized );
	FASSERT( m_pRespawnEffectMesh );

	m_fRespawnEffectTimer = _RESPAWN_TIME;

	// position, initialize, and add respawn mesh to world
	CFVec3A vTmp;
	MS2WS( vTmp, m_vRespawnEffectMeshPos );

	CFMtx43A::m_XlatRotY.m_vFront.z = 1.0f;
	CFMtx43A::m_XlatRotY.m_vFront.x = 0.0f;
	CFMtx43A::m_XlatRotY.m_vRight.z = 0.0f;
	CFMtx43A::m_XlatRotY.m_vRight.x = 1.0f;	
	CFMtx43A::m_XlatRotY.m_vPos.Add( m_pWorldMesh->GetBoneMtxPalette()[m_nRespawnEffectMeshBoneIdx]->m_vPos, vTmp );
	m_pRespawnEffectMesh->m_Xfm.BuildFromMtx( CFMtx43A::m_XlatRotY );
	m_pRespawnEffectMesh->UpdateTracker();
	m_pRespawnEffectMesh->AddToWorld();
	m_pRespawnEffectMesh->SetMeshAlpha( 0.0f );

	// spawn the respawn particle effect
	if( m_hRespawnEffectParticle != FPARTICLE_INVALID_HANDLE ) {
		fparticle_KillEmitter( m_hRespawnEffectParticle );
	}

	SetControlValue( ASI_RESPAWN, 1.0f );
	ZeroTime( ASI_RESPAWN );

	m_hRespawnEffectParticle = fparticle_SpawnEmitter( m_hRespawnEffectPartDef, &(m_pWorldMesh->GetBoneMtxPalette()[m_nRespawnEffectMeshBoneIdx]->m_vPos.v3), &(CFVec3A::m_UnitAxisY.v3), &(CFVec3A::m_Null.v3), m_fRespawnEffectPartIntensity );
}


void CBot::RespawnEffect_Kill( void ) {
	FASSERT( m_bRespawnEffectInitialized );
	FASSERT( m_anAnimStackIndex[ASI_RESPAWN] != -1 );


	if( m_hRespawnEffectParticle != FPARTICLE_INVALID_HANDLE ) {
		fparticle_StopEmitter( m_hRespawnEffectParticle );
		m_hRespawnEffectParticle = FPARTICLE_INVALID_HANDLE;
	}

	SetControlValue( ASI_RESPAWN, 0.0f );

	m_fRespawnEffectTimer = 0.0f;

	m_pRespawnEffectMesh->RemoveFromWorld();
}


void CBot::RespawnEffect_Work( void ) {
	FASSERT( m_anAnimStackIndex[ASI_RESPAWN] != -1 );

	if( m_fRespawnEffectTimer == 0.0f ) {
		return;
	}

	m_fRespawnEffectTimer -= FLoop_fPreviousLoopSecs;
	if( m_fRespawnEffectTimer <= 0.0f ) {
		RespawnEffect_Kill();
		return;
	}

	// relocate the mesh
	f32 fVal;
	CFVec3A vTmp;
	MS2WS( vTmp, m_vRespawnEffectMeshPos );

	CFMtx43A::m_XlatRotY.m_vFront.z = 1.0f;
	CFMtx43A::m_XlatRotY.m_vFront.x = 0.0f;
	CFMtx43A::m_XlatRotY.m_vRight.z = 0.0f;
	CFMtx43A::m_XlatRotY.m_vRight.x = 1.0f;
	CFMtx43A::m_XlatRotY.m_vPos.Add( m_pWorldMesh->GetBoneMtxPalette()[m_nRespawnEffectMeshBoneIdx]->m_vPos, vTmp );
	
	// calculate scale
//	fVal = fmath_Div( m_fRespawnEffectTimer, _RESPAWN_TIME );
	fVal = m_fRespawnEffectTimer * (1.0f / _RESPAWN_TIME );
	fVal = FMATH_FPOT( fVal, m_fRespawnEffectMeshStartScale, m_fRespawnEffectMeshFinishScale );

	m_pRespawnEffectMesh->m_Xfm.BuildFromMtx( CFMtx43A::m_XlatRotY, fVal );
	m_pRespawnEffectMesh->UpdateTracker();

	// update the alpha if necessary
//	fVal = fmath_Div( m_fRespawnEffectTimer, _BOT_RESPAWN_FADEOUT_TIME );
	fVal = m_fRespawnEffectTimer * (1.0f / _BOT_RESPAWN_FADEOUT_TIME);
	FMATH_CLAMP_UNIT_FLOAT( fVal );
	m_pRespawnEffectMesh->SetMeshAlpha( fVal );

	// run the animation
	DeltaTime( ASI_RESPAWN, FLoop_fPreviousLoopSecs * 2.0f, TRUE );

	// if we're very close to the end, blend it out
	if( m_fRespawnEffectTimer < _BOT_RESPAWN_BLENDOUT_TIME ) {
		fVal = fmath_Div( m_fRespawnEffectTimer, _BOT_RESPAWN_BLENDOUT_TIME );
		fVal = fmath_UnitLinearToSCurve( fVal );
		SetControlValue( ASI_RESPAWN, fVal );
	}
}


void CBot::RespawnEffect_Destroy( void ) {
    if( m_hRespawnEffectParticle != FPARTICLE_INVALID_HANDLE ) {
		fparticle_KillEmitter( m_hRespawnEffectParticle );
		m_hRespawnEffectParticle = FPARTICLE_INVALID_HANDLE;
	}

	if( m_pRespawnEffectMesh ) {
		fdelete( m_pRespawnEffectMesh );
		m_pRespawnEffectMesh = NULL;
	}
}
	
void CBot::RepairAllLimbs( BOOL bRepairDestroyed/*=FALSE*/ ) {
	if( !m_pPartMgr || !(m_pPartMgr->IsCreated() ) ) {
		return;
	}

	for( u32 i=0; i<m_pPartMgr->GetLimbTypeCount(); i++ ) {
		if( bRepairDestroyed || (m_pPartMgr->GetLimbState( i ) == CBotPartMgr::LIMB_STATE_DANGLING) ) {
			m_pPartMgr->RepairLimb( i );
		}
	}
}

//any bot class that implements a HOP, ROLL or Startle with an ASI_ slot animation can use this function
void CBot::HandleHopRollStartleGeneric( void ) {

	if (!m_bControls_Human &&
		m_nMoveState == BOTMOVESTATE_NONE &&
		m_nJumpState == BOTJUMPSTATE_NONE &&
		((m_anAnimStackIndex[ASI_HOP_LEFT]!=-1 && (m_nControlsBot_Flags & CBotControl::FLAG_HOP_LEFT)) ||
		 (m_anAnimStackIndex[ASI_HOP_RIGHT]!=-1 && (m_nControlsBot_Flags & CBotControl::FLAG_HOP_RIGHT )) ||
		 (m_anAnimStackIndex[ASI_ROLL_LEFT]!=-1 && (m_nControlsBot_Flags & CBotControl::FLAG_ROLL_LEFT)) ||
		 (m_anAnimStackIndex[ASI_ROLL_RIGHT]!=-1 && (m_nControlsBot_Flags & CBotControl::FLAG_ROLL_RIGHT)) ||
		 (m_anAnimStackIndex[ASI_STARTLE]!=-1 && (m_nControlsBot_Flags & CBotControl::FLAG_HOP_VERT))))
	{
		m_nMoveState = BOTMOVESTATE_BLENDIN;
		
		if (m_nControlsBot_Flags & CBotControl::FLAG_HOP_VERT)
		{
			m_nMoveType = BOTMOVETYPE_VERT_STARTLE;
		}
		else if (m_nControlsBot_Flags & CBotControl::FLAG_HOP_LEFT)
		{
			m_nMoveType = BOTMOVETYPE_HOP_LEFT;
		}
		else if (m_nControlsBot_Flags & CBotControl::FLAG_HOP_RIGHT)
		{
			m_nMoveType = BOTMOVETYPE_HOP_RIGHT;
		}
		else if (m_nControlsBot_Flags & CBotControl::FLAG_ROLL_LEFT)
		{
			m_nMoveType = BOTMOVETYPE_ROLL_LEFT;
		}
		else if (m_nControlsBot_Flags & CBotControl::FLAG_ROLL_RIGHT)
		{
			m_nMoveType = BOTMOVETYPE_ROLL_RIGHT;
		}
		m_fUnitHop = 0.0f;
	}
}






void CBot::StartDoubleJumpGeneric( BOOL bAddToVelocityY,
								  u32 uTapJumpTuck,			//ANIMTAP_JUMP_TUCK
								  u32 uTapJumpUnTuck,		//ANIMTAP_JUMP_UNTUCK
								  u32 uControlJumpLaunch,	//ANIMCONTROL_JUMP_LAUNCH
								  u32 uControlJumpFly,		//ANIMCONTROL_JUMP_FLY
								  u32 uControlJumpTuck) {	//ANIMCONTROL_JUMP_TUCK
	f32 fOOTuckUntuckAnimSpeedMult, fTuckSecs, fUntuckSecs, fJumpSecs, fAnimSecs, fVelY;

	if( bAddToVelocityY ) {
		m_Velocity_WS.y = m_pBotInfo_Jump->fVerticalVelocityJump2 * m_fJumpMultiplier;
		m_Velocity_MS.y = m_pBotInfo_Jump->fVerticalVelocityJump2 * m_fJumpMultiplier;
	}

	FASSERT( m_pBotInfo_Jump->fTuckUntuckAnimSpeedMult != 0.f );
	fOOTuckUntuckAnimSpeedMult = fmath_Inv( m_pBotInfo_Jump->fTuckUntuckAnimSpeedMult );

	fTuckSecs = GetTotalTime( uTapJumpTuck ) * fOOTuckUntuckAnimSpeedMult;
	fUntuckSecs = GetTotalTime( uTapJumpUnTuck ) * fOOTuckUntuckAnimSpeedMult;
	fAnimSecs = fTuckSecs + fUntuckSecs;

	fVelY = m_pBotInfo_Jump->fVerticalVelocityJump2;

	fJumpSecs = fmath_Div( -2.0f * fVelY, m_pBotInfo_Gen->fGravity );

	if( fAnimSecs <= fJumpSecs ) {
		m_fTuckUntuckAnimTimeMult = 1.0f;
		m_fUntuckStartCountdownSecs = fJumpSecs - (fTuckSecs + fUntuckSecs);
	} else {
		m_fTuckUntuckAnimTimeMult = fmath_Div( fAnimSecs, fJumpSecs );
		m_fUntuckStartCountdownSecs = 0.0f;
	}

	SetControlValue( uControlJumpLaunch, 0.0f );
	SetControlValue( uControlJumpFly, 0.0f );
	SetControlValue( uControlJumpTuck, 1.0f );
	ZeroTime( uTapJumpTuck );

	m_fFlipPitchVelocity = fmath_Div( FMATH_2PI, fJumpSecs );

	if( IsReversePastHalfway() ) {
		// flip rotation is backwards when reverse facing
		if( m_Velocity_MS.z > 0.01f ) {
			m_fFlipPitchVelocity = -m_fFlipPitchVelocity;
		}
	} else {
		if( m_Velocity_MS.z < -0.01f ) {
			m_fFlipPitchVelocity = -m_fFlipPitchVelocity;
		}
	}

	m_fFlipPitch = 0.0f;
	m_nJumpState = BOTJUMPSTATE_TUCK;
	SetJumping();

	FMATH_CLEARBITMASK( m_nBotFlags2, BOTFLAG2_CAN_DOUBLE_JUMP );
}


BOOL CBot::IsUseless( void ) const {
	FASSERT( IsCreated() );

	//findfix: would be nice to get some higher level functions for checking whether arms are dangling, or torso is broken or worse.
	//for now, just use these component status things.
	return (IsDeadOrDying() ||
		!IsInWorld() ||
		(m_pPartMgr && m_pPartMgr->IsCreated() && m_pPartMgr->GetComponentStatus(CBotPartMgr::COMPONENT_TYPE_EYES) == CBotPartMgr::COMPONENT_STATUS_NONE_OPERATIONAL)	||
		(m_pPartMgr && m_pPartMgr->IsCreated() && m_pPartMgr->GetComponentStatus(CBotPartMgr::COMPONENT_TYPE_PRIMARY) == CBotPartMgr::COMPONENT_STATUS_NONE_OPERATIONAL)  ||
		(m_pPartMgr && m_pPartMgr->IsCreated() && m_pPartMgr->GetComponentStatus(CBotPartMgr::COMPONENT_TYPE_SECONDARY) == CBotPartMgr::COMPONENT_STATUS_NONE_OPERATIONAL));
}


void CBot::MeleeInit( MeleeData_t *pMeleeData, f32 fRadius, const CDamageProfile *pDamageProfile, const CFVec3A *pvAttackPos_WS/*=NULL*/, MeleeData_t *pMasterData/*=NULL*/, u32 uMaxHitPerSwing/* = MELEE_MAX_ENTITIES_PER_SWING*/ ) {
	for( u32 i=0; i<MELEE_MAX_ENTITIES_PER_SWING; i++ ) {
		pMeleeData->apMeleeHitEntityBuffer[i] = NULL;
	}

	pMeleeData->uMeleeNumHitEntities = 0;
	pMeleeData->vMeleeLastPos = CFVec3A::m_Null;
	pMeleeData->pvMeleePos = pvAttackPos_WS;
	pMeleeData->uMaxHitEntities = uMaxHitPerSwing;
	pMeleeData->fMeleeSphereRadius = fRadius;
	pMeleeData->pDamageProfile = (CDamageProfile*)pDamageProfile;
	pMeleeData->pMasterData = pMasterData ? pMasterData : pMeleeData;
	pMeleeData->fMinNotifyInterval = 0.10f;
	pMeleeData->uLastNotification = 0;
}


void CBot::MeleeStartAttack( MeleeData_t *pMeleeData, const CFVec3A *pvAttackPos_WS/*=NULL*/ ) {
	FASSERT( pMeleeData->uMaxHitEntities <= MELEE_MAX_ENTITIES_PER_SWING);
	FASSERT( pvAttackPos_WS || pMeleeData->pvMeleePos );

	pMeleeData->uMeleeNumHitEntities = 0;
	pMeleeData->vMeleeLastPos = pvAttackPos_WS ? *pvAttackPos_WS : *(pMeleeData->pvMeleePos);
	pMeleeData->uLastNotification = 0;
}


void CBot::MeleeSwing( MeleeData_t *pMeleeData, const CFVec3A *pvAttackPos_WS/*=NULL*/ ) {
	FASSERT( pvAttackPos_WS || pMeleeData->pvMeleePos );
	FASSERT( m_uMeleeCollisionThisFrameCount == 0 );
	FASSERT( !m_pMeleeDataCB );
	
	CFVec3A vMove;
	u32 i, j;

	// set up some initial stuff, updating & checking pMasterData's lists
	MeleeData_t *pMasterData = pMeleeData->pMasterData;
	if( !pvAttackPos_WS ) {
		pvAttackPos_WS = pMeleeData->pvMeleePos;
	}

	vMove.Sub( *(pvAttackPos_WS), pMeleeData->vMeleeLastPos );

#if SAS_ACTIVE_USER == SAS_USER_ELLIOTT
	fdraw_DevSphere( &(pvAttackPos_WS->v3), pMeleeData->fMeleeSphereRadius, &FColor_MotifBlue );
	fdraw_DevSphere( &(pMeleeData->vMeleeLastPos.v3), pMeleeData->fMeleeSphereRadius, &FColor_MotifRed );
#endif
	
	CFCollData colldata;
	CFSphere sphere;

	if( IsPlayerBot() ) {
		colldata.nCollMask				= FCOLL_MASK_COLLIDE_WITH_PLAYER | FCOLL_MASK_COLLIDE_WITH_THIN_PROJECTILES | FCOLL_MASK_COLLIDE_WITH_THICK_PROJECTILES;
	} else {
		colldata.nCollMask				= FCOLL_MASK_COLLIDE_WITH_NPCS | FCOLL_MASK_COLLIDE_WITH_THIN_PROJECTILES | FCOLL_MASK_COLLIDE_WITH_THICK_PROJECTILES;
	}
	
	colldata.nFlags						= FCOLL_DATA_FLAGS_NONE;
	colldata.nStopOnFirstOfCollMask		= FCOLL_MASK_NONE;
	colldata.nTrackerUserTypeBitsMask	= FCOLL_USER_TYPE_BITS_ALL;
	colldata.pCallback					= _MeleeHitTrackerCB;
	colldata.pLocationHint				= m_pWorldMesh;
	colldata.nTrackerSkipCount			= FWorld_nTrackerSkipListCount;
	colldata.ppTrackerSkipList			= FWorld_apTrackerSkipList;
	colldata.pMovement					= &vMove;

	
	sphere.Set( pvAttackPos_WS->v3, pMeleeData->fMeleeSphereRadius );
	FWorld_nTrackerSkipListCount = 0;
	AppendTrackerSkipList();
	fcoll_Clear();

	m_uMeleeCollisionThisFrameCount = 0;
	m_pMeleeDataCB = pMasterData;

	fcoll_Check( &colldata, &sphere );

	// spin through the collisions, keeping each entity & one world collision 
	m_uMeleeCollisionThisFrameCount = 0;
	BOOL bAdd;
	f32 fClosestWorldDist2;
	f32 fVal;

	for( i=0; i<FColl_nImpactCount; i++ ) {
		bAdd = TRUE;
		
		for( j=0; j<m_uMeleeCollisionThisFrameCount; j++ ) {
			if( FColl_aImpactBuf[i].pTag == m_aMeleeCollisionThisFrameBuffer[j].pTag ) {
				// already have this guy

				if( !FColl_aImpactBuf[i].pTag ) {
					fVal = FColl_aImpactBuf[i].ImpactPoint.DistSq( *(pvAttackPos_WS) );
					if( fVal < fClosestWorldDist2 ) {
                        // replace this slot w/ the close one
						m_aMeleeCollisionThisFrameBuffer[j] = FColl_aImpactBuf[i];
						fClosestWorldDist2 = fVal;
					}
				} else {
					FASSERT_NOW;
					bAdd = FALSE;
				}
			}
		}

		if( bAdd ) {
			m_aMeleeCollisionThisFrameBuffer[m_uMeleeCollisionThisFrameCount++] = FColl_aImpactBuf[i];
			if( !FColl_aImpactBuf[i].pTag ) {
				fClosestWorldDist2 = FColl_aImpactBuf[i].ImpactPoint.DistSq( *(pvAttackPos_WS) );
			}
			if( m_uMeleeCollisionThisFrameCount == pMeleeData->uMaxHitEntities ) {
				break;
			}
		}
	}

	for( u32 i=0; i<m_uMeleeCollisionThisFrameCount; i++ ) {
		CEntity *pHitEntity = CGColl::ExtractEntity( &(m_aMeleeCollisionThisFrameBuffer[i]) );

		if( pHitEntity ) {
			
			// it's an entity
			if( pMasterData->uMeleeNumHitEntities < pMasterData->uMaxHitEntities ) {

				// increment our counter
				pMasterData->apMeleeHitEntityBuffer[pMasterData->uMeleeNumHitEntities] = pHitEntity;
				pMasterData->uMeleeNumHitEntities++;
			} else {

				// ignore, we're at max
				continue;
			}
		}

		// notify the bot, if we don't have a last notification, this is the first
		if( pHitEntity || ((FLoop_nRealTotalLoopTicks - pMasterData->uLastNotification) > (u32)(pMasterData->fMinNotifyInterval * FLoop_nTicksPerSec)) ) {
			MeleeCollision( pHitEntity, &(m_aMeleeCollisionThisFrameBuffer[i]), !(pMasterData->uLastNotification) );
			pMasterData->uLastNotification = FLoop_nRealTotalLoopTicks;
		}
	}

	m_uMeleeCollisionThisFrameCount = 0;
	m_pMeleeDataCB = NULL;
	pMeleeData->vMeleeLastPos = *pvAttackPos_WS;
}


void CBot::MeleeCollision( CEntity *pEntity, FCollImpact_t *pImpact, BOOL bFirstHit ) {
	FASSERT( m_pMeleeDataCB );

	if( pEntity ) {
		// if it's an entity and we hit it, it must have a mesh, right?
		FASSERT( pImpact->pTag );

		// Inflict damage to the entity...

		// Get an empty damage form...
		CDamageForm *pDamageForm = CDamage::GetEmptyDamageFormFromPool();

		CFVec3A vToMesh;
		vToMesh.Sub( pImpact->ImpactPoint, m_MountPos_WS );
		f32 fMag2 = vToMesh.MagSq();
		if( fMag2 > 0.01f ) {
			vToMesh.Mul( fmath_InvSqrt( fMag2 ) );
		} else {
			return;
		}

		if( pDamageForm ) 
		{
			// Fill out the form...
			pDamageForm->m_nDamageLocale = CDamageForm::DAMAGE_LOCALE_IMPACT;
			pDamageForm->m_nDamageDelivery = CDamageForm::DAMAGE_DELIVERY_ONE_SPECIFIC_ENTITY;
			pDamageForm->m_pDamageProfile = m_pMeleeDataCB->pDamageProfile;
			pDamageForm->m_Damager.pWeapon = NULL;
			pDamageForm->m_Damager.pBot = this;
			pDamageForm->m_Damager.nDamagerPlayerIndex = m_nPossessionPlayerIndex;
			pDamageForm->m_pDamageeEntity = pEntity;

			pDamageForm->InitTriDataFromCollImpact( (CFWorldMesh*)pImpact->pTag, pImpact, &vToMesh );
			CDamage::SubmitDamageForm( pDamageForm );
		}
	}


	//MEDEVPRINTF( "CBot::MeleeCollision():  Impact at %0.2f, entity = %x, first?  %d\n", FLoop_nRealTotalLoopTicks/(f32)FLoop_nTicksPerSec, pEntity, bFirstHit );

	return;
}


u32 CBot::_MeleeHitTrackerCB( CFWorldTracker *pTracker ) {
	u32 i;

	if( m_pMeleeDataCB->uMeleeNumHitEntities >= m_pMeleeDataCB->uMaxHitEntities ) {
		// already hit all the entities we can
		return FCOLL_CHECK_CB_EXIT_IMMEDIATELY;
	}

	// if it's an entity...
	if( pTracker->m_nUser == MESHTYPES_ENTITY ) {

		// see if we've already hit it
		for( i=0; i<m_pMeleeDataCB->uMeleeNumHitEntities; i++ ) {
			if( (CEntity*)pTracker->m_pUser == m_pMeleeDataCB->apMeleeHitEntityBuffer[i] ) {
				return FCOLL_CHECK_CB_EXIT_IMMEDIATELY;
			}
		}
	}


	return FCOLL_CHECK_CB_FIRST_IMPACT_ONLY;
}


BOOL CBot::IsBackBroken( void ) const {
	FASSERT( IsCreated() ); 
	return (m_pPartMgr && m_pPartMgr->IsCreated() && m_pPartMgr->IsBackBroken() );
}


////////////////////////////////////////////////////////////////////////////////////////////////////////
// Bot in pieces

// Causes Bot to break into pieces, or restores him if he's already in pieces.
// Passing a positive number in fTimeInPieces will cause Glitch to remain in pieces
// for that amount of time and then re-assemble himself automatically (used with wrench).
// Otherwise -1.0 should be passed in, which causes Glitch to remain in pieces until
// another call to the function is made.
// The parameter bImmediately can be set to TRUE to have Glitch go straight to in-pieces state
// without animating.  bImmediately does not currently function in the case of re-assembling.

void CBot::BreakIntoPieces( const f32 &fTimeInPieces, BOOL bImmediately /*=FALSE*/ ) {
	if( fTimeInPieces < 0.0f && fTimeInPieces != -1.0f ) {
		FASSERT_NOW;
	}

	// We are already in pieces, put ourself back together
	if( m_fTimeInPieces == -1.0f ) {
		FASSERT_MSG( !bImmediately, "immediate re-assembly of Glitch not supported" );
		m_bFallDown = FALSE;
		m_bOnGround = FALSE;

		m_fTimeInPieces = 0.0f;
		m_fUnitPiecesBlend = 1.0f;

		ZeroTime( ASI_ASSEMBLE );
		SetControlValue( ASI_ASSEMBLE, m_fUnitPiecesBlend );
		SetControlValue( ASI_DISASSEMBLE, 0.0f );

		return;
	}

	// we are not in pieces, initialize the break-into-pieces effect
	if( bImmediately ) {
		// immediately snap to final in-pieces state
		m_fUnitPiecesBlend = 1.0f;
		UpdateUnitTime( ASI_DISASSEMBLE, 1.0f );
		ZeroTime( ASI_ASSEMBLE );
		SetControlValue( ASI_DISASSEMBLE, m_fUnitPiecesBlend );
		SetControlValue( ASI_ASSEMBLE, 0.0f );
		ImmobilizeBot();
		m_bInPieces = TRUE;
		m_bFallDown = TRUE;
		m_bFallAnimDone = TRUE;
		m_bOnGround = TRUE;
		m_fTimeInPieces = fTimeInPieces;
		m_pWorldMesh->SetCollisionFlag( m_bColWithPieces );
		SetTargetable( FALSE );
		aibrainman_SetLODOverrideForOneWork( AIBrain() );	// ensure Glitch work runs for one frame to update animation
		CHud2::GetHudForPlayer(m_nOwnerPlayerIndex)->SetWSEnable( FALSE );
	} else {
		// init animated effect
		m_fUnitPiecesBlend = 0.0f;
		ZeroTime( ASI_DISASSEMBLE );
		ZeroTime( ASI_ASSEMBLE );
		SetControlValue( ASI_DISASSEMBLE, m_fUnitPiecesBlend );
		SetControlValue( ASI_ASSEMBLE, 0.0f );
		ImmobilizeBot();
		m_bInPieces = TRUE;
		m_bFallDown = TRUE;
		m_bFallAnimDone = FALSE;
		m_fTimeInPieces = fTimeInPieces;
		m_pWorldMesh->SetCollisionFlag( m_bColWithPieces );
		SetTargetable( FALSE );
		CHud2::GetHudForPlayer(m_nOwnerPlayerIndex)->SetWSEnable( FALSE );
	}
}


void CBot::BotInPiecesWork( void ) {
	if( !m_bInPieces ) {
		return;
	}

	if( m_fTimeInPieces != 0.0f && m_fTimeInPieces != -1.0f ) {
		m_fTimeInPieces -= FLoop_fPreviousLoopSecs;
		FMATH_CLAMPMIN( m_fTimeInPieces, 0.0f );

		if( m_fTimeInPieces == 0.0f ) {
			m_bFallDown = FALSE;
			m_bOnGround = FALSE;

			m_fUnitPiecesBlend = 1.0f;
			ZeroTime( ASI_ASSEMBLE );
			SetControlValue( ASI_ASSEMBLE, m_fUnitPiecesBlend );
			SetControlValue( ASI_DISASSEMBLE, 0.0f );
		}
	}

	if( m_bFallDown ) {
		DeltaTime( ASI_DISASSEMBLE, FLoop_fPreviousLoopSecs, TRUE );
		SetControlValue( ASI_DISASSEMBLE, m_fUnitPiecesBlend );

		m_fUnitPiecesBlend += FLoop_fPreviousLoopSecs;
		FMATH_CLAMPMAX( m_fUnitPiecesBlend, 1.0f );

		if( GetUnitTime( ASI_DISASSEMBLE ) >= 0.9999f ) {
			m_bFallAnimDone = TRUE;
			m_bOnGround = TRUE;
		} else {
			aibrainman_SetLODOverrideForOneWork( AIBrain() );
		}
	} else {
		DeltaTime( ASI_ASSEMBLE, FLoop_fPreviousLoopSecs, TRUE );
		m_pWorldMesh->SetCollisionFlag( TRUE );
		SetTargetable( TRUE );
		FMATH_SETBITMASK( m_nBotFlags2, BOTFLAG2_UNDER_CONSTRUCTION );
		_PushAwayVehicles();

		if( GetUnitTime( ASI_ASSEMBLE ) >= 0.9999f ) {
			m_fUnitPiecesBlend -= ( 4.0f * FLoop_fPreviousLoopSecs );
			FMATH_CLAMPMIN( m_fUnitPiecesBlend, 0.0f );
			SetControlValue( ASI_ASSEMBLE, m_fUnitPiecesBlend );

			if( m_fUnitPiecesBlend == 0.0f ) {
				m_bInPieces = FALSE;
				m_fTimeInPieces = 0.0f;
				

				SetControlValue( ASI_DISASSEMBLE, 0.0f );
				SetControlValue( ASI_ASSEMBLE, 0.0f );

				// We're done animating, so we can move
				FMATH_CLEARBITMASK( m_nBotFlags2, BOTFLAG2_UNDER_CONSTRUCTION );
				MobilizeBot();
				CHud2::GetHudForPlayer(m_nPossessionPlayerIndex)->SetWSEnable( TRUE );
			}
		}
	}
}

#define _START_BLEND_TIME		( 0.33f )
#define _OO_START_BLEND_TIME	( 1.0f / _START_BLEND_TIME )

BOOL CBot::BotInPieces_InPiecesAnimBoneCB( CFMtx43A &rNewMtx, const CFMtx43A &rParentMtx, const CFMtx43A &rBoneMtx ) {
	if( m_bInPieces ) {
		// If we are on the ground, falling down or we are in the early stages of building up
		if( m_bOnGround ||
			m_bFallDown ||
			m_fUnitPiecesBlend > _START_BLEND_TIME ) {
			rNewMtx.Mul( rParentMtx, rBoneMtx );
			return TRUE;
		}
	}

	return FALSE;
}


BOOL CBot::BotInPieces_RecoverAnimBoneCB( CFMtx43A &rNewMtx, const CFMtx43A &rParentMtx, const CFMtx43A &rBoneMtx ) {
	// Blend between where the animation wants to put our pieces and where the callback
	// wants to put our pieces
	if( m_bInPieces && m_fUnitPiecesBlend <= _START_BLEND_TIME ) {
		CFVec3A DestPos, SrcPos;
		CFQuatA SrcQuat, DestQuat, Slerp;

		DestPos = rNewMtx.m_vPos;
		DestQuat.BuildQuat( rNewMtx );

		rNewMtx.Mul( rParentMtx, rBoneMtx );
		SrcPos = rNewMtx.m_vPos;
		SrcQuat.BuildQuat( rNewMtx );

		Slerp.ReceiveSlerpOf( m_fUnitPiecesBlend * _OO_START_BLEND_TIME, DestQuat, SrcQuat );
		Slerp.BuildMtx( rNewMtx );
		rNewMtx.m_vPos.Lerp( m_fUnitPiecesBlend * _OO_START_BLEND_TIME, DestPos, SrcPos );

		return TRUE;
	}

	return FALSE;
}


// this function is used to get a vehicle off of glitch if he's reassembling
void CBot::_PushAwayVehicles( void ) {
	CFSphere sphere;
	sphere.m_Pos.Set( m_pBotInfo_Gen->fCollSphere1X_MS, m_pBotInfo_Gen->fCollSphere1Y_MS, m_pBotInfo_Gen->fCollSphere1Z_MS );
	sphere.m_fRadius = m_pBotInfo_Gen->fCollSphere1Radius_MS;
	sphere.m_Pos.Add( m_MountPos_WS.v3 );

	m_pCollBot = this;
	fworld_FindTrackersIntersectingSphere( &sphere, FWORLD_TRACKERTYPE_MESH, _PushAwayVehiclesCB, 0, NULL, m_pWorldMesh, MESHTYPES_ENTITY, ENTITY_BIT_VEHICLE );
	m_pCollBot = NULL;
}

BOOL CBot::_PushAwayVehiclesCB( CFWorldTracker *pTracker, FVisVolume_t *pVolume ) {
	FASSERT( m_pCollBot && (m_pCollBot->TypeBits() & ENTITY_BIT_BOTGLITCH) );

	// make sure we got what we asked for out of the collision system
	FASSERT( pTracker->GetType() == FWORLD_TRACKERTYPE_MESH );
	FASSERT( pTracker->m_nUser == MESHTYPES_ENTITY );
	FASSERT( pTracker->m_pUser && ((CEntity*)pTracker->m_pUser)->TypeBits() & ENTITY_BIT_VEHICLE );

	CFVec3A vToVehicle;
	f32 fDist;
	f32 fSpd;
	CVehicle *pVehicle = (CVehicle*)pTracker->m_pUser;


	vToVehicle.Sub( pVehicle->MtxToWorld()->m_vPos, m_pCollBot->MtxToWorld()->m_vPos );
	fDist = vToVehicle.Mag();
	if( fDist > 0.01f ) {
		vToVehicle.Mul( fmath_Inv( fDist ) );		
	} else {
		vToVehicle = CFVec3A::m_UnitAxisZ;
	}

	fDist =  pTracker->GetBoundingSphere().m_fRadius + m_pCollBot->m_pBotInfo_Gen->fCollSphere1Radius_MS - fDist;

	fSpd =  5.0f - pVehicle->m_fSpeed_WS;
	FMATH_CLAMP_MIN0( fSpd );
	vToVehicle.Mul( fSpd );

	pVehicle->ApplyVelocityImpulse_WS( vToVehicle );

	//#if SAS_ACTIVE_USER == SAS_USER_ELLIOTT
	//	DEVPRINTF( "Vehicle on top of glitch.  Push distance is %0.2f\n", fDist );
	//#endif

	return FALSE;
}



#define _DEATH_TIME						( 1.0f )
#define _DEATH_IMPULSE_X 				( 2.0f )
#define _DEATH_IMPULSE_Y 				( 10.0f )
#define _DEATH_IMPULSE_Z 				( 2.0f )
#define _DEATH_CAMSHAKE_INTENSITY		( 0.25f )
#define _DEATH_CAMSHAKE_DURATION		( 0.25f )
#define _DAMAGE_CAMSHAKE_INTENSITY		( 0.125f )
#define _DAMAGE_CAMSHAKE_DURATION		( 0.25f )
#define _DEATH_DELAY_RESPAWN_TIME		( 1.5f )
#define _DEATH_DELAY_RESPAWN_TIME_MP	( 2.0f )


void CBot::PlayerDamage_StartDeath( void ) {
	FASSERT( m_pPlayerDeathData );
	FASSERT( IsPlayerBot() );

	m_pPlayerDeathData->fDeathTimer		= _DEATH_TIME;
	m_pPlayerDeathData->fNextDeathEvent = _DEATH_TIME; 

	CFVec3A vLookAt, vPos;
	vPos.Set( 0.0f, 2.0f, -10.0f );
	SetCameraOverride( TRUE, _DEATH_TIME, TRUE, CFVec3A::m_Null, vPos );
	ClearBotFlag_IgnoreControls();
}


BOOL CBot::PlayerDamage_DeathWork( void ) {
	if( !m_pPlayerDeathData || !IsPlayerBot() ) {
		return FALSE;
	}

	u32 i;

	f32 fLastTimer = m_pPlayerDeathData->fDeathTimer;
	m_pPlayerDeathData->fDeathTimer -= FLoop_fPreviousLoopSecs;

	if( m_uLifeCycleState == BOTLIFECYCLE_BURIED ) {
		return FALSE;		// all done
	
	} else if( m_uLifeCycleState == BOTLIFECYCLE_DEAD ) {
		
		if ( m_pPlayerDeathData->fDeathTimer < 0.0f ) {
			m_uLifeCycleState = BOTLIFECYCLE_BURIED;
		}

		return TRUE;
	}

	
	if( (fLastTimer >= m_pPlayerDeathData->fNextDeathEvent) && 
		(m_pPlayerDeathData->fDeathTimer < m_pPlayerDeathData->fNextDeathEvent) ) {

		m_pPartMgr->SetRandomLimb_Dangle();

		// give bot a push
		CFVec3A vImpulse;
		vImpulse.Set( fmath_RandomBipolarUnitFloat() * _DEATH_IMPULSE_X,
			fmath_RandomFloatRange( 0.0f/*0.5f * _DEATH_IMPULSE_Y*/, _DEATH_IMPULSE_Y ),
			fmath_RandomBipolarUnitFloat() * _DEATH_IMPULSE_Z );
		ApplyVelocityImpulse_MS( vImpulse );

		ShakeCamera( _DAMAGE_CAMSHAKE_INTENSITY, _DAMAGE_CAMSHAKE_DURATION );

		// pick a time for the next effect
		f32 fVal = 0.0f;
		for( i=0; i<m_pPlayerDeathData->uNumDeathLimbs; i++ ) {
			if( m_pPartMgr->GetLimbState( i ) == CBotPartMgr::LIMB_STATE_INTACT ) {
				fVal += 1.0f;
			}
		}

		if( fVal > 0.0f ) {
			fVal = fmath_Inv( fVal ) * m_pPlayerDeathData->fDeathTimer;
			m_pPlayerDeathData->fNextDeathEvent = m_pPlayerDeathData->fDeathTimer - fmath_RandomFloatRange( fVal * 0.5f, fVal );
		}
	}

	if( m_pPlayerDeathData->fDeathTimer < 0.0f ) {
		ShakeCamera( _DEATH_CAMSHAKE_INTENSITY, _DEATH_CAMSHAKE_DURATION );
		m_pPartMgr->DestroyBot( NULL );
		m_uLifeCycleState = BOTLIFECYCLE_DEAD;
		SetBotFlag_IgnoreControls();

		// Reset the death timer to delay a transition to our "buried" state.
		// The buried state basically just prevents respawn or restart for
		// a period of time after death.
		m_pPlayerDeathData->fDeathTimer = MultiplayerMgr.IsMultiplayer() ? _DEATH_DELAY_RESPAWN_TIME_MP : _DEATH_DELAY_RESPAWN_TIME;
	}

	return TRUE;
}




























