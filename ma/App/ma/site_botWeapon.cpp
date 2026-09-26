//////////////////////////////////////////////////////////////////////////////////////
// botsentry.cpp -
//
// Author: Michael Scholz
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
// 11/05/02 MScholz     Created.
//////////////////////////////////////////////////////////////////////////////////////

#include "player.h"
#include "site_BotWeapon.h"
#include "site_FloorSentry.h"
#include "site_WallSentry.h"
#include "site_Pillbox.h"
#include "site_RatGun.h"
#include "fworld_coll.h"
#include "fclib.h"
#include "fresload.h"
#include "FCheckpoint.h"
#include "weapon.h" 
#include "meshtypes.h"
#include "gamepad.h"
#include "fsound.h"
#include "MultiplayerMgr.h"
#include "vehicle.h"
#include "vehiclerat.h"
#include "AI\AIGroup.h"
#include "ai\aigameutils.h"
#include "AI\AIBrainman.h"
#include "MiscDraw.h"

#define _SOUND_BANK_SENTRY_GUN			"Sentry"
#define _SOUND_BANK_MEDIUM_WEIGHT		"Bots_Med"
#define _RELOCATE_IDENTIFIER			(&m_pGun)

cchar _BOTINFO_FILENAME[] = 		"b_sentry";
static BOOL _bDisableAudio = FALSE;

//**********************************************************************************************************************************
//--------------------------------------------------------------------------------------------------------
// Game Data Map:
//--------------------------------------------------------------------------------------------------------

#define UNCLAMPED_FLOAT_SINOFDEG2RAD_X \
	FGAMEDATA_VAR_TYPE_FLOAT| \
	FGAMEDATA_FLAGS_FLOAT_SIN_DEGS_TO_RADS_X, \
	sizeof( f32 ), \
	F32_DATATABLE_0, \
	F32_DATATABLE_0

const FGameData_TableEntry_t CBotSiteWeapon::m_BotInfo_SiteWeaponVocab[] = 
{
//	cchar*	pszArmorProfile;
	FGAMEDATA_VAR_TYPE_STRING|
	FGAMEDATA_FLAGS_STRING_PTR_TO_MAIN_STR_TBL,
	sizeof( char * ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

//	f32		fMaxYawVelocity;
	FGAMEDATA_VOCAB_F32_DEG2RAD,
//	f32		fMaxYawVelocityPossess;
	FGAMEDATA_VOCAB_F32_DEG2RAD,
//	f32		fMaxYawVelocityPatrol;
	FGAMEDATA_VOCAB_F32_DEG2RAD,
//	f32		fMaxYawAcceleration;
	FGAMEDATA_VOCAB_F32_DEG2RAD,
//	f32		fMaxPitchAcceleration;
	FGAMEDATA_VOCAB_F32_DEG2RAD,
//	f32		fMaxPitchVelocity;
	FGAMEDATA_VOCAB_F32_DEG2RAD,
//	f32		fMaxPitchVelocityPossess;
	FGAMEDATA_VOCAB_F32_DEG2RAD,
//	f32		fMinimumPitch;
	FGAMEDATA_VOCAB_F32_UNBOUND,
//	f32		fMaximumPitch;
	FGAMEDATA_VOCAB_F32_UNBOUND,
//	f32		fMinimumYaw;
	FGAMEDATA_VOCAB_F32_UNBOUND,
//	f32		fMaximumYaw;
	FGAMEDATA_VOCAB_F32_UNBOUND,

	// End of table:
	FGAMEDATA_VAR_TYPE_COUNT| 0, 0, F32_DATATABLE_0, F32_DATATABLE_0
};

const FGameData_TableEntry_t CBotSiteWeapon::m_BotInfo_SiteAIWeaponVocab[] = 
{
//	f32		fPatrolPitch;
	FGAMEDATA_VOCAB_F32_DEG2RAD,
//	f32		fPatrolMaxYaw;
	FGAMEDATA_VOCAB_F32_DEG2RAD,
//	f32		fLookCosHalfFOV;
	FGAMEDATA_VOCAB_F32_UNBOUND,
//	f32		fVisionDistance;
	FGAMEDATA_VOCAB_F32_UNBOUND,
//	f32		fRadarCosHalfFOV;
	FGAMEDATA_VOCAB_F32_UNBOUND,
//	f32		fRadarDistance;
	FGAMEDATA_VOCAB_F32_UNBOUND,
//	f32		fRadarVelocity;
	FGAMEDATA_VOCAB_F32_UNBOUND,
//	f32		fTargetingYawSin;
	UNCLAMPED_FLOAT_SINOFDEG2RAD_X,
//	f32		fSpewFireYawSin;
	UNCLAMPED_FLOAT_SINOFDEG2RAD_X,
//	f32		fRocketFireYawSin;
	UNCLAMPED_FLOAT_SINOFDEG2RAD_X,
//	f32		fAttackDelayTime;
	FGAMEDATA_VOCAB_F32_UNBOUND,
//	f32		fStrafingTime;
	FGAMEDATA_VOCAB_F32_UNBOUND,
//	f32		fSearchingTime;
	FGAMEDATA_VOCAB_F32_UNBOUND,
//	f32		fDefendingTime;
	FGAMEDATA_VOCAB_F32_UNBOUND,
	// End of table:
	FGAMEDATA_VAR_TYPE_COUNT| 0, 0, F32_DATATABLE_0, F32_DATATABLE_0
};

const FGameData_TableEntry_t CBotSiteWeapon::m_BotInfo_RatGunVocab[] = 
{
//	f32 fMinCapBlendAngle;
	FGAMEDATA_VOCAB_F32_DEG2RAD,
//	f32 fMaxCapBlendAngle;
	FGAMEDATA_VOCAB_F32_DEG2RAD,
//	f32 fMaxPitchCapped;
	FGAMEDATA_VOCAB_F32_UNBOUND,
	// End of table:
	FGAMEDATA_VAR_TYPE_COUNT| 0, 0, F32_DATATABLE_0, F32_DATATABLE_0
};

const FGameData_TableEntry_t CBotSiteWeapon::m_BotInfo_DeathVocab[] = 
{
//	FExplosion_GroupHandle_t hExplosion;
	FGAMEDATA_VOCAB_EXPLODE_GROUP,
//	FExplosion_GroupHandle_t hExplosion2;
	FGAMEDATA_VOCAB_EXPLODE_GROUP,

	FGAMEDATA_VOCAB_PARTICLE,
//		s32	nExplosions;
	FGAMEDATA_VOCAB_U32_BOUND(F32_DATATABLE_0,F32_DATATABLE_10),
//		f32 fMinTimeBetweenExpl;
	FGAMEDATA_VOCAB_F32_UNBOUND,
//		f32 fMaxTimeBetweenExpl;
	FGAMEDATA_VOCAB_F32_UNBOUND,
//		f32 fSecSmoke;
	FGAMEDATA_VOCAB_F32_UNBOUND,

	// End of table:
	FGAMEDATA_VAR_TYPE_COUNT| 0, 0, F32_DATATABLE_0, F32_DATATABLE_0
};


const FGameDataMap_t CBotSiteWeapon::m_aGameDataMap[] = 
{
	"Gen",
	m_aBotInfoVocab_Gen,
	sizeof(m_BotInfo_Gen),
	(void *)&m_BotInfo_Gen,

	"PillBoxGen",
	m_aBotInfoVocab_Gen,
	sizeof(m_BotInfo_Gen),
	(void *)&m_BotInfo_PillBoxGen,

	"FloorSentryGen",
	m_aBotInfoVocab_Gen,
	sizeof(m_BotInfo_Gen),
	(void *)&m_BotInfo_FloorSentryGen,

	"MountAim",
	m_aBotInfoVocab_MountAim,
	sizeof(m_BotInfo_MountAim),
	(void *)&m_BotInfo_MountAim,

	"Weapon",
	m_aBotInfoVocab_Weapon,
	sizeof(m_BotInfo_Weapon),
	(void *)&m_BotInfo_Weapon,

	"FloorSentry",
	m_BotInfo_SiteWeaponVocab,
	sizeof(BotInfo_SiteWeapon_t),
	(void*)&m_BotInfo_FloorSentry,

	"CeilingSentry",
	m_BotInfo_SiteWeaponVocab,
	sizeof(BotInfo_SiteWeapon_t),
	(void*)&m_BotInfo_CeilingSentry,

	"PillBox",
	m_BotInfo_SiteWeaponVocab,
	sizeof(BotInfo_SiteWeapon_t),
	(void*)&m_BotInfo_PillBox,

	"RatGun",
	m_BotInfo_SiteWeaponVocab,
	sizeof(BotInfo_SiteWeapon_t),
	(void*)&m_BotInfo_RatGun,

	"RatGunXtra",
	m_BotInfo_RatGunVocab,
	sizeof(BotInfo_RatGun_t),
	(void*)&m_BotInfo_RatGunXtra,

	"FloorSentryAI",
	m_BotInfo_SiteAIWeaponVocab,
	sizeof(BotInfo_SiteWeaponAI_t),
	(void*)&m_BotInfo_FloorSentryAI,

	"CeilingSentryAI",
	m_BotInfo_SiteAIWeaponVocab,
	sizeof(BotInfo_SiteWeaponAI_t),
	(void*)&m_BotInfo_CeilingSentryAI,

	"FloorDeath",
	m_BotInfo_DeathVocab,
	sizeof(BotInfo_Death_t),
	(void*)&m_Death_Floor,

	"WallDeath",
	m_BotInfo_DeathVocab,
	sizeof(BotInfo_Death_t),
	(void*)&m_Death_Wall,

	"PillBoxDeath",
	m_BotInfo_DeathVocab,
	sizeof(BotInfo_Death_t),
	(void*)&m_Death_PillBox,

	NULL
};



//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CBotSiteWeaponBuilder
//
//**********************************************************************************************************************************
static CBotSiteWeaponBuilder _BotSiteWeaponBuilder;
void CBotSiteWeaponBuilder::SetDefaults( u64 nEntityTypeBits, u64 nEntityLeafTypeBit, cchar *pszEntityType ) 
{
	ENTITY_BUILDER_SET_PARENT_CLASS_DEFAULTS( CBotBuilder, ENTITY_BIT_SITEWEAPON, pszEntityType );
	m_bEnableLight = TRUE;
	m_fLightRadiusOverride = -1.0f;
}

BOOL CBotSiteWeaponBuilder::PostInterpretFixup( void ) 
{
	// comment:
	if( !CBotBuilder::PostInterpretFixup() ) 
	{
		goto _ExitWithError;
	}

	return TRUE;

	// Error:
_ExitWithError:

	return FALSE;
}


BOOL CBotSiteWeaponBuilder::InterpretTable( void ) 
{
	if (!fclib_stricmp( CEntityParser::m_pszTableName, "weapontype" ))
	{
		cchar* szWeaponTypeEquals;
		CEntityParser::Interpret_String(&szWeaponTypeEquals);

		if (!fclib_stricmp( szWeaponTypeEquals, "FloorSentry" ))
		{
			m_pBotDef = &m_MilFloorSentryBotDef;
			m_pszEC_ArmorProfile = (m_pszEC_ArmorProfile ? m_pszEC_ArmorProfile : CBotSiteWeapon::m_BotInfo_FloorSentry.pszArmorProfile);
			m_SiteWeaponInfo = CBotSiteWeapon::m_BotInfo_FloorSentry;
			m_SiteWeaponAI = CBotSiteWeapon::m_BotInfo_FloorSentryAI;
			FMATH_SETBITMASK( m_uFlags, BOT_BUILDER_CLASS_CAN_BE_RECRUITED | BOT_BUILDER_DPORT_NOT_NEEDED_FOR_RECRUITMENT );
		}
		else if (!fclib_stricmp( szWeaponTypeEquals, "WallSentry" ))
		{
			m_pBotDef = &m_WallSentryBotDef;
			m_pszEC_ArmorProfile = (m_pszEC_ArmorProfile ? m_pszEC_ArmorProfile : CBotSiteWeapon::m_BotInfo_CeilingSentry.pszArmorProfile);
			m_SiteWeaponInfo = CBotSiteWeapon::m_BotInfo_CeilingSentry;
			m_SiteWeaponAI = CBotSiteWeapon::m_BotInfo_CeilingSentryAI;
		}
		else if (!fclib_stricmp( szWeaponTypeEquals, "PillBox" ))
		{
			m_pBotDef = &m_AmbientPillBoxBotDef;
			m_pszEC_ArmorProfile = (m_pszEC_ArmorProfile ? m_pszEC_ArmorProfile : CBotSiteWeapon::m_BotInfo_PillBox.pszArmorProfile);
			m_SiteWeaponInfo = CBotSiteWeapon::m_BotInfo_PillBox;
			m_SiteWeaponAI = CBotSiteWeapon::m_BotInfo_FloorSentryAI;
		}
		else if (!fclib_stricmp( szWeaponTypeEquals, "RatGun" ))
		{
			m_pBotDef = &m_AmbientRatGunBotDef;
			m_pszEC_ArmorProfile = (m_pszEC_ArmorProfile ? m_pszEC_ArmorProfile : CBotSiteWeapon::m_BotInfo_RatGun.pszArmorProfile);
			m_SiteWeaponInfo = CBotSiteWeapon::m_BotInfo_RatGun;
			m_SiteWeaponAI = CBotSiteWeapon::m_BotInfo_FloorSentryAI;
		}
		return TRUE;
	}
	else if (!fclib_stricmp( CEntityParser::m_pszTableName, "SpotLight" ))
	{
		BOOL bVal=TRUE;
		BOOL bOK = CEntityParser::Interpret_BOOL(&bVal);
		m_bEnableLight = (bVal ? TRUE : FALSE);
		return TRUE;
	}
	else if (!fclib_stricmp( CEntityParser::m_pszTableName, "SpotLightRange" ))
	{
		f32 fVal=-1.0f;
		BOOL bOK = CEntityParser::Interpret_F32(&fVal);
		m_fLightRadiusOverride = fVal;
		return TRUE;
	}
	else if (!fclib_stricmp( CEntityParser::m_pszTableName, "MinimumPitch" ))
	{
		f32 fVal;
		BOOL bOK = CEntityParser::Interpret_F32(&fVal);
		m_SiteWeaponInfo.fMinimumPitch = FMATH_DEG2RAD(fVal);
		return bOK;
	}
	else if (!fclib_stricmp( CEntityParser::m_pszTableName, "MaximumPitch" ))
	{
		f32 fVal;
		BOOL bOK = CEntityParser::Interpret_F32(&fVal);
		m_SiteWeaponInfo.fMaximumPitch = FMATH_DEG2RAD(fVal);
		return bOK;		
	}
	else if (!fclib_stricmp( CEntityParser::m_pszTableName, "MaximumYaw" ))
	{
		f32 fVal;
		BOOL bOK = CEntityParser::Interpret_F32(&fVal);
		m_SiteWeaponInfo.fMaximumYaw = FMATH_DEG2RAD(fVal);
		return bOK;
	}		
	else if (!fclib_stricmp( CEntityParser::m_pszTableName, "MinimumYaw" ))
	{
		f32 fVal;
		BOOL bOK = CEntityParser::Interpret_F32(&fVal);
		m_SiteWeaponInfo.fMinimumYaw = FMATH_DEG2RAD(fVal);
		return bOK;
	}
	else if (!fclib_stricmp( CEntityParser::m_pszTableName, "PatrolPitch" ))
	{
		f32 fVal;
		BOOL bOK = CEntityParser::Interpret_F32(&fVal);
		m_SiteWeaponAI.fPatrolPitch = FMATH_DEG2RAD(fVal);
		return bOK;
	}
	else if (!fclib_stricmp( CEntityParser::m_pszTableName, "PatrolMinYaw" ))
	{
		f32 fVal;
		BOOL bOK = CEntityParser::Interpret_F32(&fVal);
		m_SiteWeaponAI.fPatrolMinYaw = FMATH_DEG2RAD(fVal);
		return bOK;
	}
	else if (!fclib_stricmp( CEntityParser::m_pszTableName, "PatrolMaxYaw" ))
	{
		f32 fVal;
		BOOL bOK = CEntityParser::Interpret_F32(&fVal);
		m_SiteWeaponAI.fPatrolMaxYaw = FMATH_DEG2RAD(fVal);
		return bOK;
	}
	else if (!fclib_stricmp( CEntityParser::m_pszTableName, "VisionCosHalfFOV" ))
	{
		f32 fVal;
		BOOL bOK = CEntityParser::Interpret_F32(&fVal);
		m_SiteWeaponAI.fVisionCosHalfFOV = fVal;
		return bOK;
	}
	else if (!fclib_stricmp( CEntityParser::m_pszTableName, "VisionDistance" ))
	{
		f32 fVal;
		BOOL bOK = CEntityParser::Interpret_F32(&fVal);
		m_SiteWeaponAI.fVisionDistance = fVal;
		return bOK;
	}
	else if (!fclib_stricmp( CEntityParser::m_pszTableName, "RadarCosHalfFOV" ))
	{
		f32 fVal;
		BOOL bOK = CEntityParser::Interpret_F32(&fVal);
		m_SiteWeaponAI.fRadarCosHalfFOV = fVal;
		return bOK;
	}
	else if (!fclib_stricmp( CEntityParser::m_pszTableName, "RadarDistance" ))
	{
		f32 fVal;
		BOOL bOK = CEntityParser::Interpret_F32(&fVal);
		m_SiteWeaponAI.fRadarDistance = fVal;
		return bOK;
	}
	else if (!fclib_stricmp( CEntityParser::m_pszTableName, "RadarVelocity" ))
	{
		f32 fVal;
		BOOL bOK = CEntityParser::Interpret_F32(&fVal);
		m_SiteWeaponAI.fRadarVelocity = fVal;
		return bOK;
	}
	else if (!fclib_stricmp( CEntityParser::m_pszTableName, "TargetingYawSin" ))
	{
		f32 fVal;
		BOOL bOK = CEntityParser::Interpret_F32(&fVal);
		m_SiteWeaponAI.fTargetingYawSin = fVal;
		return bOK;
	}
	else if (!fclib_stricmp( CEntityParser::m_pszTableName, "SpewFireYawSin" ))
	{
		f32 fVal;
		BOOL bOK = CEntityParser::Interpret_F32(&fVal);
		m_SiteWeaponAI.fSpewFireYawSin = fVal;
		return bOK;
	}
	else if (!fclib_stricmp( CEntityParser::m_pszTableName, "RocketFireYawSin" ))
	{
		f32 fVal;
		BOOL bOK = CEntityParser::Interpret_F32(&fVal);
		m_SiteWeaponAI.fRocketFireYawSin = fVal;
		return bOK;
	}
	else if (!fclib_stricmp( CEntityParser::m_pszTableName, "AttackDelayTime" ))
	{
		f32 fVal;
		BOOL bOK = CEntityParser::Interpret_F32(&fVal);
		m_SiteWeaponAI.fAttackDelayTime = fVal;
		return bOK;
	}
	else if (!fclib_stricmp( CEntityParser::m_pszTableName, "StrafingTime" ))
	{
		f32 fVal;
		BOOL bOK = CEntityParser::Interpret_F32(&fVal);
		m_SiteWeaponAI.fStrafingTime = fVal;
		return bOK;
	}
	else if (!fclib_stricmp( CEntityParser::m_pszTableName, "SearchingTime" ))
	{
		f32 fVal;
		BOOL bOK = CEntityParser::Interpret_F32(&fVal);
		m_SiteWeaponAI.fSearchingTime = fVal;
		return bOK;
	}
	else if (!fclib_stricmp( CEntityParser::m_pszTableName, "DefendingTime" ))
	{
		f32 fVal;
		BOOL bOK = CEntityParser::Interpret_F32(&fVal);
		m_SiteWeaponAI.fDefendingTime = fVal;
		return bOK;
	}
	// We don't understand this command. Pass it on...
	return CBotBuilder::InterpretTable();
}



//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CBotSiteWeapon
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************



BOOL				CBotSiteWeapon::m_bSystemInitialized = FALSE;
u32					CBotSiteWeapon::m_nBotClassClientCount	= 0;
CFSoundGroup*		CBotSiteWeapon::m_pSounds[CBotSiteWeapon::SITESND_COUNT];

CBotSiteWeapon::BotInfo_Walk_t		CBotSiteWeapon::m_BotInfo_Walk;			// Walk bot info
CBotSiteWeapon::BotInfo_Gen_t		CBotSiteWeapon::m_BotInfo_Gen;
CBotSiteWeapon::BotInfo_Gen_t		CBotSiteWeapon::m_BotInfo_PillBoxGen;
CBotSiteWeapon::BotInfo_Gen_t		CBotSiteWeapon::m_BotInfo_FloorSentryGen;
CBotSiteWeapon::BotInfo_MountAim_t  CBotSiteWeapon::m_BotInfo_MountAim;
CBotSiteWeapon::BotInfo_Weapon_t    CBotSiteWeapon::m_BotInfo_Weapon;

CBotSiteWeapon::BotInfo_SiteWeapon_t CBotSiteWeapon::m_BotInfo_FloorSentry;
CBotSiteWeapon::BotInfo_SiteWeapon_t CBotSiteWeapon::m_BotInfo_CeilingSentry;
CBotSiteWeapon::BotInfo_SiteWeapon_t CBotSiteWeapon::m_BotInfo_PillBox;
CBotSiteWeapon::BotInfo_SiteWeapon_t CBotSiteWeapon::m_BotInfo_RatGun;
CBotSiteWeapon::BotInfo_RatGun_t	 CBotSiteWeapon::m_BotInfo_RatGunXtra;

CBotSiteWeapon::BotInfo_SiteWeaponAI_t CBotSiteWeapon::m_BotInfo_FloorSentryAI;
CBotSiteWeapon::BotInfo_SiteWeaponAI_t CBotSiteWeapon::m_BotInfo_CeilingSentryAI;

CBotSiteWeapon::BotInfo_Death_t CBotSiteWeapon::m_Death_Floor;
CBotSiteWeapon::BotInfo_Death_t CBotSiteWeapon::m_Death_PillBox;
CBotSiteWeapon::BotInfo_Death_t CBotSiteWeapon::m_Death_Wall;

s32 CBotSiteWeapon::m_nBoneIndexPillboxAttachPt = -1;
s32 CBotSiteWeapon::m_nBoneIndexFloorSentryAttachPt = -1;

static cchar* _pszSoundGroupTable[CBotSiteWeapon::SITESND_COUNT] = 
{
	"SRGMpowerup",
	"SRGMpowerdn",
	"SRGMdeath",
	"SRMSMfire",
	"SRMSMrocket",
	"SRMSMlocked",
	"SRMSMrotate",
	"SRMSPtakcon",
	"SRMSPrelcon",
	"SRMSPlight",
};

BOOL CBotSiteWeapon::InitSystem( void )
{
	FASSERT( !m_bSystemInitialized );
	m_bSystemInitialized = TRUE;
	m_nBotClassClientCount = 0;
	return TRUE;
}

BOOL CBotSiteWeapon::ClassHierarchyLoadSharedResources( void ) 
{
	FASSERT( m_bSystemInitialized );
	FASSERT( m_nBotClassClientCount != 0xffffffff );

	++m_nBotClassClientCount;

	if( !CBot::ClassHierarchyLoadSharedResources() ) 
	{
		// Bail now since parent class has already called
		// ClassHierarchyUnloadSharedResources() and decremented
		// our client counter...
		return FALSE;
	}

	if( m_nBotClassClientCount > 1 ) 
	{
		// Resources already loaded...
		return TRUE;
	}

	FResFrame_t ResFrame;
	ResFrame = fres_GetFrame();

	if( !ReadBotInfoFile( m_aGameDataMap, _BOTINFO_FILENAME ) )
	{
		goto _ExitInitSystemWithError;
	}
	fang_MemSet( &m_BotInfo_Walk , 0, sizeof(m_BotInfo_Walk) );
	
	if (!CFloorSentry::InitSystem())
	{
		goto _ExitInitSystemWithError;
	}
	
	if (!CWallSentry::InitSystem())
	{
		goto _ExitInitSystemWithError;
	}
	if (!CPillbox::InitSystem())
	{
		goto _ExitInitSystemWithError;
	}
	
	if (!CRatGun::InitSystem())
	{
		goto _ExitInitSystemWithError;
	}

	if( !fresload_Load( FSNDFX_RESTYPE, _SOUND_BANK_SENTRY_GUN ) ) 
	{
		DEVPRINTF( "CBotSiteWeapon::_InitSoundHandles(): Could not load sound effect bank '%s'\n", _SOUND_BANK_SENTRY_GUN );
		_bDisableAudio = TRUE;
	}

	for (SiteSound_e iSndIdx = (SiteSound_e)0; iSndIdx < SITESND_COUNT; iSndIdx = (SiteSound_e)(int(iSndIdx)+1))
	{
		m_pSounds[iSndIdx] = CFSoundGroup::RegisterGroup( _pszSoundGroupTable[iSndIdx] );
		if (!m_pSounds[iSndIdx])
		{
			DEVPRINTF( "CBotSiteWeapon::_InitSoundHandles(): Could not load sound effect '%s'\n", _pszSoundGroupTable[iSndIdx] );
			_bDisableAudio = TRUE;
		}
	}
	return TRUE;

	// Error:
_ExitInitSystemWithError:
	ClassHierarchyUnloadSharedResources();
	fres_ReleaseFrame( ResFrame );
	return FALSE;
}


void CBotSiteWeapon::UninitSystem( void )
{
	m_bSystemInitialized = FALSE;
}

void CBotSiteWeapon::ClassHierarchyUnloadSharedResources( void ) 
{
	FASSERT( m_nBotClassClientCount > 0 );

	--m_nBotClassClientCount;

	if( m_nBotClassClientCount > 0 ) 
	{
		return;
	}

	CFloorSentry::UninitSystem();
	CWallSentry::UninitSystem();
	CPillbox::UninitSystem();
	CRatGun::UninitSystem();

	CBot::ClassHierarchyUnloadSharedResources();
}



CBotSiteWeapon::~CBotSiteWeapon()
{
	if( IsSystemInitialized() && IsCreated() )
	{
		DetachFromParent();
		DetachAllChildren();
		RemoveFromWorld( TRUE );
		ClassHierarchyDestroy();
	}
}


BOOL CBotSiteWeapon::Create( s32 eSClass, s32 nPlayerIndex, BOOL bInstallDataPort, cchar *pszEntityName, const CFMtx43A *pMtx, cchar *pszAIBuilderName )
{
	FASSERT( m_bSystemInitialized );
	FASSERT( !IsCreated() );
	FASSERT( FWorld_pWorld );
	
	if( !ClassHierarchyLoadSharedResources() ) 
	{
		// Failure! (resources have already been cleaned up, so we can bail now)
		return FALSE;
	}

	// Get pointer to the leaf class's builder object...
	CBotSiteWeaponBuilder *pBuilder = (CBotSiteWeaponBuilder *)GetLeafClassBuilder();

	// If we're the leaf class, set the builder defaults...
	if( pBuilder == &_BotSiteWeaponBuilder )
	{
		pBuilder->SetDefaults( 0, 0, ENTITY_TYPE_BOTSITEWEAPON );
	}

	switch (eSClass)
	{
	case BOTSUBCLASS_SITEWEAPON_WALLSENTRY:
		m_pBotDef = &(pBuilder->m_WallSentryBotDef);
		pBuilder->m_nEC_HealthContainerCount = 1; 
		pBuilder->m_pszEC_ArmorProfile = m_BotInfo_CeilingSentry.pszArmorProfile;
		pBuilder->m_SiteWeaponInfo = CBotSiteWeapon::m_BotInfo_CeilingSentry;
		pBuilder->m_SiteWeaponAI = CBotSiteWeapon::m_BotInfo_CeilingSentryAI;

		break;
	case BOTSUBCLASS_SITEWEAPON_FLOORSENTRY:
		m_pBotDef = &(pBuilder->m_MilFloorSentryBotDef);
		pBuilder->m_nEC_HealthContainerCount = 1; 
		pBuilder->m_pszEC_ArmorProfile = m_BotInfo_FloorSentry.pszArmorProfile;
		pBuilder->m_SiteWeaponInfo = CBotSiteWeapon::m_BotInfo_FloorSentry;
		pBuilder->m_SiteWeaponAI = CBotSiteWeapon::m_BotInfo_FloorSentryAI;

		break;
	case BOTSUBCLASS_SITEWEAPON_PILLBOX:
		m_pBotDef = &(pBuilder->m_AmbientPillBoxBotDef);
		pBuilder->m_nEC_HealthContainerCount = 1;
		pBuilder->m_pszEC_ArmorProfile = m_BotInfo_PillBox.pszArmorProfile;
		pBuilder->m_SiteWeaponInfo = CBotSiteWeapon::m_BotInfo_PillBox;
		pBuilder->m_SiteWeaponAI = CBotSiteWeapon::m_BotInfo_FloorSentryAI;

		break;
	case BOTSUBCLASS_SITEWEAPON_RATGUN:
		m_pBotDef = &(pBuilder->m_AmbientRatGunBotDef);
		pBuilder->m_nEC_HealthContainerCount = 1;
		pBuilder->m_pszEC_ArmorProfile = m_BotInfo_RatGun.pszArmorProfile;
		pBuilder->m_SiteWeaponInfo = CBotSiteWeapon::m_BotInfo_RatGun;
		pBuilder->m_SiteWeaponAI = CBotSiteWeapon::m_BotInfo_FloorSentryAI;

		break;
	}
	
	// Create an entity...
	return CBot::Create( NULL, nPlayerIndex, bInstallDataPort, pszEntityName, pMtx, pszAIBuilderName );
}

void CBotSiteWeapon::_SetDataMembersToDefaults( void )
{
	m_pData->m_MtxAimX_MS = CFMtx43A::m_IdentityMtx;
	m_pData->m_MtxAimY_MS = CFMtx43A::m_IdentityMtx;
	m_pData->m_fDistanceToTarget =0.0f;
	m_pData->m_fYawWS=0.0f;
	m_pData->m_fPitchWS=0.0f;
	m_pData->m_fYawVelocity=0.0f;
	m_fPowerOffOnTime = 0.0f;
	// Init data members...
	m_pBotInfo_MountAim		= &m_BotInfo_MountAim;
	m_pBotInfo_Weapon		= &m_BotInfo_Weapon;
	m_pBotInfo_Walk			= &m_BotInfo_Walk;
	m_pMoveIdentifier		= _RELOCATE_IDENTIFIER;

	m_pData->m_pEnemyTarget			= NULL;
	m_pData->m_pEnemyTargetDistance2	= NULL;

	m_pData->m_CamManInfo.m_fHalfFOV		= FMATH_DEG2RAD(45); // actually half FOV
	m_pData->m_CamManInfo.m_pmtxMtx		= &m_pData->m_MtxCam_WS;

	m_GazeUnitVec_WS		= CFVec3A::m_UnitAxisZ;
	m_vRadarDirectionWS		= CFVec3A::m_UnitAxisZ;

	m_nLastTagPointInspected = 0;
	m_bDamageTaken = FALSE;
	m_fLastBeepTime = 0.0f;
	m_fRotateVolume = .35f;
	m_RotateXAxis.Reset();
	m_RotateYAxis.Reset();

	m_uVehicleFlags = 0;

	m_fTimeToNextExplosion=0.0f;
	m_nExplosionsLeftInDeath=0;
}

void CBotSiteWeapon::_SetDataMembersToBuilder( void )
{
	// Get pointer to the leaf class's builder object...
	CBotSiteWeaponBuilder *pBuilder = (CBotSiteWeaponBuilder *)GetLeafClassBuilder();
	if (pBuilder->m_pBotDef)
	{
		m_pBotDef				= pBuilder->m_pBotDef;
	}
	m_pData->m_bEnableLight = pBuilder->m_bEnableLight;
	m_pData->m_fLightRadiusOverride = pBuilder->m_fLightRadiusOverride;

	m_pData->m_MtxPosition_WS	= pBuilder->m_EC_Mtx_WS;
	m_pData->m_fPitchAdjustWS2MS = 0.0f;
	if( m_pBotDef->m_nSubClass == BOTSUBCLASS_SITEWEAPON_RATGUN )
	{
		
		m_pData->m_fYawAdjustWS2MS	= fmath_Atan(m_pData->m_MtxPosition_WS.m_vFront.x,m_pData->m_MtxPosition_WS.m_vFront.z);
	} 
	else
	{
		m_pData->m_fYawAdjustWS2MS	= fmath_Atan(m_pData->m_MtxPosition_WS.m_vFront.x,m_pData->m_MtxPosition_WS.m_vFront.z);
	}

	m_SiteWeaponInfo = pBuilder->m_SiteWeaponInfo;
	m_SiteWeaponAI = pBuilder->m_SiteWeaponAI;
	
	// fixup to account for placement
	m_SiteWeaponInfo.fMinimumYaw = pBuilder->m_SiteWeaponInfo.fMinimumYaw+m_pData->m_fYawAdjustWS2MS;
	m_SiteWeaponInfo.fMaximumYaw = pBuilder->m_SiteWeaponInfo.fMaximumYaw+m_pData->m_fYawAdjustWS2MS;

	m_pData->m_fMinimumYawWS = m_SiteWeaponInfo.fMinimumYaw;
	m_pData->m_fMaximumYawWS = m_SiteWeaponInfo.fMaximumYaw;

	m_SiteWeaponAI.fPatrolPitch = pBuilder->m_SiteWeaponAI.fPatrolPitch + 0.0f;
	m_SiteWeaponAI.fPatrolMinYaw = pBuilder->m_SiteWeaponAI.fPatrolMinYaw+m_pData->m_fYawAdjustWS2MS;
	m_SiteWeaponAI.fPatrolMaxYaw = pBuilder->m_SiteWeaponAI.fPatrolMaxYaw+m_pData->m_fYawAdjustWS2MS;

	m_pData->m_fMaxYawVelocity = pBuilder->m_SiteWeaponInfo.fMaxYawVelocity;
	m_pData->m_fMaxYawVelocityPossess = pBuilder->m_SiteWeaponInfo.fMaxYawVelocityPossess;
	m_pData->m_fMaxYawVelocityPatrol = pBuilder->m_SiteWeaponInfo.fMaxYawVelocityPatrol;
	m_pData->m_fMaxYawAcceleration = pBuilder->m_SiteWeaponInfo.fMaxYawAcceleration;
	m_pData->m_fMaxPitchAcceleration = pBuilder->m_SiteWeaponInfo.fMaxPitchAcceleration;
	m_pData->m_fMaxPitchVelocity = pBuilder->m_SiteWeaponInfo.fMaxPitchVelocity;
	m_pData->m_fMaxPitchVelocityPossess = pBuilder->m_SiteWeaponInfo.fMaxPitchVelocityPossess;
	m_pData->m_fMinimumPitchWS = pBuilder->m_SiteWeaponInfo.fMinimumPitch;
	m_pData->m_fMaximumPitchWS = pBuilder->m_SiteWeaponInfo.fMaximumPitch;

	if (m_pBotDef == &pBuilder->m_MilFloorSentryBotDef)
	{
		m_pBotInfo_Gen = &m_BotInfo_FloorSentryGen;
		m_pDeathInfo = &CBotSiteWeapon::m_Death_Floor;
		m_pGun = fnew CFloorSentry();
		m_pGun->Create(m_pData,this,TypeBits());
		
		m_nBoneIndexFloorSentryAttachPt = m_pWorldMesh->FindBone("AttachPoint_Bot");
		if (m_nBoneIndexFloorSentryAttachPt < 0)
		{
			DEVPRINTF("Couldn't find attach point bone\n");
		}

		m_pData->m_fTimeToPowerUp = .75f;
		m_pData->m_fTimeToPowerDown = 2.00f;
		m_pData->m_fTimeToEnter = 0.0f;
		m_pData->m_fYawWS = 0.f +  m_pData->m_fYawAdjustWS2MS;
		m_bAimRockets = TRUE;
		m_pData->m_eSiteWeaponState		= MultiplayerMgr.IsSinglePlayer() ? SITE_PATROLLING : SITE_POWER_DOWN;

		m_fCollCylinderRadius_WS = 3.0f;
		m_fCollCylinderHeight_WS = 8.0f;

		SetBotFlag_MeshCollideOnly();
		EnableOurWorkBit();
//		DisableOurWorkBit();

	}
	else if (m_pBotDef == &pBuilder->m_WallSentryBotDef)
	{
		m_pBotInfo_Gen = &m_BotInfo_Gen;
		m_pDeathInfo = &CBotSiteWeapon::m_Death_Wall;
		m_pGun = fnew CWallSentry();
		m_pGun->Create(m_pData,this,TypeBits());
		m_pData->m_fTimeToPowerUp = 3.0f;
		m_pData->m_fTimeToPowerDown = 3.0f;
		m_pData->m_fTimeToEnter = 0.0f;
		m_pData->m_fYawWS = 0.f +  m_pData->m_fYawAdjustWS2MS;
		m_bAimRockets = FALSE;
		m_pData->m_eSiteWeaponState		= SITE_PATROLLING;

		m_fCollCylinderRadius_WS = 2.0f;
		m_fCollCylinderHeight_WS = 2.0f;

		EnableOurWorkBit();
//		DisableOurWorkBit();
	}
	else if (m_pBotDef == &pBuilder->m_AmbientPillBoxBotDef)
	{
		m_pBotInfo_Gen = &m_BotInfo_PillBoxGen;
		m_pDeathInfo = &CBotSiteWeapon::m_Death_PillBox;
		m_pGun = fnew CPillbox();
		m_pGun->Create(m_pData,this,TypeBits());
		
		m_nBoneIndexPillboxAttachPt = m_pWorldMesh->FindBone("Swivel");

		if (m_nBoneIndexFloorSentryAttachPt < 0)
		{
			DEVPRINTF("Couldn't find attach point bone\n");
		}

		m_pData->m_fTimeToPowerDown = 0.0f;
		m_pData->m_fTimeToPowerUp = 0.50f;
		m_pData->m_fTimeToEnter = .5f;
		m_pData->m_fYawWS = 0.f +  m_pData->m_fYawAdjustWS2MS;
		m_bAimRockets = FALSE;
		m_pData->m_eSiteWeaponState		= SITE_IDLING;
		EnableOurWorkBit(); 

		m_fCollCylinderRadius_WS = 10.0f;
		m_fCollCylinderHeight_WS = 8.0f;

		SetBotFlag_MeshCollideOnly();
	}
	else if (m_pBotDef == &pBuilder->m_AmbientRatGunBotDef)
	{
		m_pBotInfo_Gen = &m_BotInfo_Gen;
		m_pDeathInfo = NULL;
		m_pGun = fnew CRatGun();
		m_pGun->Create(m_pData,this,TypeBits());
		m_pData->m_fTimeToPowerDown = 0.0f;
		m_pData->m_fTimeToPowerUp = 0.50f;
		m_pData->m_fTimeToEnter = .5f;
		m_pData->m_fYawWS = 0.f +  m_pData->m_fYawAdjustWS2MS;
		m_bAimRockets = FALSE;
		 m_pData->m_eSiteWeaponState		= SITE_IDLING;
		//m_pData->m_eSiteWeaponState		= SITE_PATROLLING;
		EnableOurWorkBit();
		EnableAutoWork(FALSE);
		SetBotFlag_MeshCollideOnly();

		m_fCollCylinderRadius_WS = 6.0f;
		m_fCollCylinderHeight_WS = 6.0f;
	}
	else
	{
		DEVPRINTF("Create What?");
	}

	m_RotateXAxis.Create(	0.f,
							m_pData->m_fMaxPitchAcceleration,
							m_pData->m_fMaxPitchVelocity,
							pBuilder->m_SiteWeaponAI.fPatrolPitch);

	m_RotateYAxis.Create(	m_pData->m_fYawAdjustWS2MS,
							m_pData->m_fMaxYawAcceleration,
							m_pData->m_fMaxYawVelocityPatrol,
							pBuilder->m_SiteWeaponAI.fPatrolMinYaw);
}

void CBotSiteWeapon::ClassHierarchyDestroy( void )
{
	m_pGun->Destroy();
	fdelete ( m_pGun );

	for (SiteSound_e iSndIdx = (SiteSound_e)0; iSndIdx < SITESND_COUNT; iSndIdx = (SiteSound_e)(int(iSndIdx)+1))
	{
		if (m_pSoundEmitters[iSndIdx])
		{
			m_pSoundEmitters[iSndIdx]->Destroy();
			m_pSoundEmitters[iSndIdx] = NULL;
		}
	}

	_bDisableAudio = TRUE; // no more audio 

	// Delete the items that we had instantiated for us...
	CBot::ClassHierarchyDestroy();
}




BOOL CBotSiteWeapon::ClassHierarchyBuild( void )
{
	cchar * pszMeshFilename = NULL;

	FASSERT( IsSystemInitialized() );
	FASSERT( FWorld_pWorld );

	// Get a frame...
	FResFrame_t ResFrame = fres_GetFrame();

	// Set input parameters for CBot creation...
	// pBuilder->m_pBotDef = &m_BotDef;
	
	// save botDef sometimes;
	const CBotDef* pSavedBotDef=NULL; 
    if (m_pBotDef)
		pSavedBotDef = m_pBotDef;
	// Build parent class...
	if( !CBot::ClassHierarchyBuild() )
	{
		// Parent class could not be built...
		goto _ExitWithError;
	}
	m_pBotDef = pSavedBotDef;

	m_pGun = NULL;
	m_pData = &m_SiteWeaponData;
	fang_MemSet(m_pData,0,sizeof(SiteWeaponData_t));
    
	// Set defaults...
	_SetDataMembersToDefaults();
	_SetDataMembersToBuilder();
	
	AtRestMatrixPalette();
	
	SetMaxHealth();
	SetBotFlag_Enemy();

	fang_MemSet(m_pSoundEmitters,0,sizeof(m_pSoundEmitters[0])*SITESND_COUNT);

	ComputeApproxMuzzlePoint_WS(&m_pData->m_vAimingOrigin_WS);	// update gun's position
	m_fFixedEyeOffset = m_pData->m_vAimingOrigin_WS.y - m_MtxToWorld.m_vPos.y;
	return TRUE;

	// Error:
_ExitWithError:
	Destroy();
	fres_ReleaseFrame( ResFrame );
	return FALSE;
}



void CBotSiteWeapon::ClassHierarchyAddToWorld( void )
{
	FASSERT( IsCreated() );
	FASSERT( !IsInWorld() );


	CBot::ClassHierarchyAddToWorld();
	
	fang_MemSet(m_pSoundEmitters,0,sizeof(m_pSoundEmitters[0])*SITESND_COUNT);
	aigroup_RegisterAsMech(this);

	m_pGun->AddToWorld();

	if( m_pBotDef->m_nSubClass == BOTSUBCLASS_SITEWEAPON_RATGUN ) 
	{
		_SetSiteWeaponState( SITE_IDLING );
	}
}



void CBotSiteWeapon::ClassHierarchyRemoveFromWorld( void )
{
	FASSERT( IsCreated() );
	FASSERT( IsInWorld() );

	if (m_pData->m_pDriverBot && m_pData->m_pDriverBot->IsInWorld())
	{
		SetSiteWeaponDriver(NULL,FALSE);
	}
	_SetSiteWeaponState(SITE_NULL); // clears out state, re-add will have to restore the state

	m_pGun->RemoveFromWorld();

	CBot::ClassHierarchyRemoveFromWorld();
	aigroup_UnregisterAsMech(this);
}

void CBotSiteWeapon::SetControls( CEntityControl *pControl )
{
	CBot::SetControls(pControl);
}

void CBotSiteWeapon::ClassHierarchyRelocated( void *pIdentifier ) 
{
	m_pData->m_MtxPosition_WS.m_vPos.Set(m_MtxToWorld.m_vPos);
	CBot::ClassHierarchyRelocated( pIdentifier );
}


CEntityBuilder *CBotSiteWeapon::GetLeafClassBuilder( void )
{
	return &_BotSiteWeaponBuilder;
}

BOOL CBotSiteWeapon::ClassHierarchyBuilt( void )
{
	FASSERT( IsSystemInitialized() );
	FASSERT( IsCreated() );

	FResFrame_t ResFrame = fres_GetFrame();

	if( !CBot::ClassHierarchyBuilt() )
	{
		goto _ExitWithError;
	}
	
	switch (m_pBotDef->m_nSubClass)
	{
	case BOTSUBCLASS_SITEWEAPON_WALLSENTRY:
		m_pGun->m_pAliveMesh->m_nFlags &= (~FMESHINST_FLAG_CAST_SHADOWS);
		m_pGun->m_pDeadMesh->m_nFlags  &= (~FMESHINST_FLAG_CAST_SHADOWS);
		break;

	case BOTSUBCLASS_SITEWEAPON_FLOORSENTRY:
		m_pGun->m_pAliveMesh->m_nFlags |= (FMESHINST_FLAG_CAST_SHADOWS);
		m_pGun->m_pDeadMesh->m_nFlags  |= (FMESHINST_FLAG_CAST_SHADOWS);
		break;

	case BOTSUBCLASS_SITEWEAPON_PILLBOX:
		m_pGun->m_pAliveMesh->m_nFlags &= (~FMESHINST_FLAG_CAST_SHADOWS);
		m_pGun->m_pDeadMesh->m_nFlags  &= (~FMESHINST_FLAG_CAST_SHADOWS);
        break;

	case BOTSUBCLASS_SITEWEAPON_RATGUN:
		m_pGun->m_pAliveMesh->m_nFlags &= (~FMESHINST_FLAG_CAST_SHADOWS);
		break;
	}
	return TRUE;

_ExitWithError:
	Destroy();
	fres_ReleaseFrame( ResFrame );
	return FALSE;
}

void CBotSiteWeapon::ClassHierarchyWork()
{
	FASSERT( m_bSystemInitialized );

	CBot::ClassHierarchyWork();
	if( m_nRecruitID >= 0 ) 
	{
		if( IsDrawEnabled() ) 
		{
			CWeaponRecruiter::UpdateRecruiterIconPos( this );
		}
	}

	if( !(m_pWorldMesh->GetVolumeFlags() & FVIS_VOLUME_IN_ACTIVE_LIST))
	{
		// We're not part of any active volume, therefore no workie;
		return;
	}

	FASSERT(m_pGun);
	//timer to prevent too many AI sounds from being kicked off
	if( m_pGun->m_fSecsUntilNextSound > 0.0f ) 
	{
		m_pGun->m_fSecsUntilNextSound -= FLoop_fPreviousLoopSecs;
	} else 
	{
		m_pGun->m_fSecsUntilNextSound = 0.0f;
	}

	if( !IsOurWorkBitSet() )
	{
		return;
	}
	//CFSphere pSphere = m_pWorldMesh->GetBoundingSphere();
	//m_MountPos_WS = pSphere.m_Pos;

	// replace with some generic reset control vars call 
	m_fControls_Fire1=0.0f;
	m_fControls_Fire2=0.0f;	

	CBot::ParseControls(); 


	// Update position...
	//CFVec3A TempVec3A;
	//TempVec3A.Mul( m_Velocity_WS, FLoop_fPreviousLoopSecs );
	//m_MountPos_WS.Add( TempVec3A );

	_CallSiteStateFunction();

	BOOL bFired = FALSE;
	bFired = m_pGun->TriggerWorkPrimary( m_fControls_Fire1);
	bFired |= m_pGun->TriggerWorkSecondary( m_fControls_Fire2);
	
	if (bFired)
	{
		JustFired();  //Bot class keeps track of fire times
	}

	// update vars for A.I.
	m_fMountPitch_WS = m_pData->m_fPitchWS;
	m_fMountYaw_WS = m_pData->m_fYawWS;
	m_MtxToWorld.m_vFront.ReceiveUnit(m_pData->m_pMtxAimOrientation_WS->m_vFront);
	m_MtxToWorld.m_vUp.ReceiveUnit(m_pData->m_pMtxAimOrientation_WS->m_vUp);
	m_MtxToWorld.m_vRight.ReceiveUnit(m_pData->m_pMtxAimOrientation_WS->m_vRight);
	m_MountPos_WS = m_pData->m_MtxPosition_WS.m_vPos;
}

void CBotSiteWeapon::Die( BOOL bSpawnDeathEffects/*=TRUE*/, BOOL bSpawnGoodies )
{
	SetArmorModifier(1.0f); // max out the armour, so that no further damage occurs, but unset via checkpoint restore.

	if (m_uLifeCycleState != BOTLIFECYCLE_LIVING)
	{
		return;  //why die again if already dead or dying
	}
	CBot::Die( bSpawnDeathEffects, bSpawnGoodies );
	SetSiteWeaponDriver(NULL,FALSE);
}

static CBotSiteWeapon* pThis=NULL;

BOOL CBotSiteWeapon::_ProposedVisionTarget( CFWorldTracker *pTracker, FVisVolume_t *pVolume )
{
	CFWorldMesh* pMesh = (CFWorldMesh*)pTracker;
	CBot* pBot = (CBot*)pMesh->m_pUser;

	if ( (pBot->m_nPossessionPlayerIndex > -1) &&
		 (Player_aPlayer[pBot->m_nPossessionPlayerIndex].m_pEntityOrig != pBot))
	{
		pThis->NotifyPossessedBotSeen(pBot);
	}
	
	if (aiutils_IsFriendly(pThis,pBot) == FALSE)
	{
		if (pBot->IsDeadOrDying())
		{
			return TRUE;
		}
		if (pBot->IsPlayerOutOfBody()) // handle collapsed glitch case 
		{
			return TRUE;
		}
		if (pThis->_aihelper_can_see_you(pBot) == FALSE)
		{
			return TRUE;
		}

		CFVec3A DeltaToTarg;
		pThis->m_TargetedPoint_WS = *pBot->GetTagPoint(0);
		DeltaToTarg.Sub(pThis->m_TargetedPoint_WS,pThis->m_MtxToWorld.m_vPos);
		f32 fThisTargsDistance2 = DeltaToTarg.MagSq();
		if (fThisTargsDistance2 < pThis->m_pData->m_pEnemyTargetDistance2)
		{
			DeltaToTarg.Mul(fmath_InvSqrt(fThisTargsDistance2));
			CFVec3A vUnitMuzzleFront;
			vUnitMuzzleFront.ReceiveUnit(pThis->m_pData->m_pMtxAimOrientation_WS->m_vFront);
			f32 fDot = DeltaToTarg.Dot(vUnitMuzzleFront);
			if (fDot > pThis->m_SiteWeaponAI.fVisionCosHalfFOV)
			{
				pThis->m_pData->m_pEnemyTargetDistance2 = fThisTargsDistance2;
				pThis->m_pData->m_pEnemyTarget = pBot;
			}
		}
	}
	return TRUE; // keep searching
}

BOOL CBotSiteWeapon::_ProposedRadarTarget( CFWorldTracker *pTracker, FVisVolume_t *pVolume )
{
	CFWorldMesh* pMesh = (CFWorldMesh*)pTracker;
	CBot* pBot = (CBot*)pMesh->m_pUser;

	if (aiutils_IsFriendly(pThis,pBot) == FALSE)
	{
		if (pBot->IsDeadOrDying())
		{
			return TRUE;
		}
		if (pBot->IsPlayerOutOfBody()) // handle collapsed glitch case 
		{
			return TRUE;
		}
		if (pThis->_aihelper_can_see_you(pBot) == FALSE)
		{
			return TRUE;
		}

		CFVec3A DeltaToTarg;
		pThis->m_TargetedPoint_WS = *pBot->GetTagPoint(0);
		DeltaToTarg.Sub(pThis->m_TargetedPoint_WS,pThis->m_MtxToWorld.m_vPos);
		f32 fThisTargsDistance2 = DeltaToTarg.MagSq();
		if (fThisTargsDistance2 < pThis->m_pData->m_pEnemyTargetDistance2)
		{
			DeltaToTarg.Mul(fmath_InvSqrt(fThisTargsDistance2));
			f32 fDot = DeltaToTarg.Dot(pThis->m_vRadarDirectionWS);
			if (fDot > pThis->m_SiteWeaponAI.fRadarCosHalfFOV)
			{
				pThis->m_pData->m_pEnemyTargetDistance2 = fThisTargsDistance2;
				pThis->m_pData->m_pEnemyTarget = pBot;
			}
		}
	}
	return TRUE; // keep searching
}

void CBotSiteWeapon::_MoveXYAxes(void)
{
	f32 fXAim = m_RotateXAxis.Work(); // run our motors
	f32 fYAim = m_RotateYAxis.Work();

	m_pData->m_MtxAimY_MS.SetRotationY(fYAim - m_pData->m_fYawAdjustWS2MS);
	m_pData->m_MtxAimX_MS.SetRotationX(fXAim - m_pData->m_fPitchAdjustWS2MS);
	m_pData->m_fYawWS = fYAim;
	m_pData->m_fYawVelocity = m_RotateYAxis.GetVelocity();
	m_pData->m_fPitchWS = fXAim;

	if ( (fmath_Abs(m_RotateYAxis.GetVelocity()) > 0.01) || (fmath_Abs(m_RotateXAxis.GetVelocity()) > 0.01) ) 
	{
		_RotateBaseNoise(TRUE,1.0f);
	}
	else
	{
		_RotateBaseNoise(TRUE, 0.0f);
	}	
}

void CBotSiteWeapon::_aihelper_SearchForTarget(void)
{
	pThis = this;
	if ( (m_pData->m_pEnemyTarget) &&									// if I've had an enemy
		 (m_pData->m_pEnemyTarget->m_nPossessionPlayerIndex >= 0) &&	// who is a player
		 (_aihelper_can_see_you(m_pData->m_pEnemyTarget)) &&			// that I can still see
		 (aiutils_IsFriendly(this, m_pData->m_pEnemyTarget)==FALSE) &&  // whom I dislike
		  !m_pData->m_pEnemyTarget->IsDeadOrDying())					// who's still functional.
	{
		// pick this enemy again!
		return;
	}
	//	else
	m_pData->m_pEnemyTarget = NULL;
	m_pData->m_pEnemyTargetDistance2 = FMATH_MAX_FLOAT;

	CFSphere fSearchSphere;
	fSearchSphere.m_fRadius = m_SiteWeaponAI.fVisionDistance;
	fSearchSphere.m_Pos.x = m_MtxToWorld.m_vPos.x;
	fSearchSphere.m_Pos.y = m_MtxToWorld.m_vPos.y;
	fSearchSphere.m_Pos.z = m_MtxToWorld.m_vPos.z;

	FWorld_nTrackerSkipListCount = 0;
	this->AppendTrackerSkipList();
	fworld_FindTrackersIntersectingSphere(	&fSearchSphere,
		FWORLD_TRACKERTYPE_MESH,
		_ProposedVisionTarget,
		FWorld_nTrackerSkipListCount,
		FWorld_apTrackerSkipList,
		m_pWorldMesh,
		MESHTYPES_ENTITY,
		ENTITY_BIT_BOT);

	if (m_pData->m_pEnemyTarget==NULL) // check the radar
	{
		// DFS - temp hack. The list traversal above actually resets the list and
		// adds the targets we are interested in to it when checking line of site.
		FWorld_nTrackerSkipListCount = 0;
		this->AppendTrackerSkipList();

		fSearchSphere.m_fRadius = m_SiteWeaponAI.fRadarDistance;
		fworld_FindTrackersIntersectingSphere(	&fSearchSphere,
			FWORLD_TRACKERTYPE_MESH,
			_ProposedRadarTarget,
			FWorld_nTrackerSkipListCount,
			FWorld_apTrackerSkipList,
			m_pWorldMesh,
			MESHTYPES_ENTITY,
			ENTITY_BIT_BOT);
	}
}


f32 CBotSiteWeapon::_TurnToTargetAndFocusRadar2d(const CFVec3A& Location,f32* pDistToTarget)
{
	CFVec3A  vAdjustAimingPoint;
	// TODO: this is plainly not correct, ratgun is clear example, MS please fix -MS.
	vAdjustAimingPoint.Mul(m_pData->m_pMtxAimOrientation_WS->m_vFront,-4.0f * m_pData->m_pMtxAimOrientation_WS->m_vFront.InvMag()); 
	
	CFVec3A  BotPosXZ_WS;
	BotPosXZ_WS.Add(m_pData->m_vAimingOrigin_WS,vAdjustAimingPoint);

	// better to overshoot the target when turning / or target accurately?
//	BotPosXZ_WS.x = m_MtxToWorld.m_vPos.x;
//	BotPosXZ_WS.z = m_MtxToWorld.m_vPos.z;


	CFVec3A GotoPointXZ_WS;
	GotoPointXZ_WS.x = Location.x;
	GotoPointXZ_WS.z = Location.z;

	// rotate around y by m_fCurrentYawWS
	f32 fSin,fCos;
	CFVec3A BotUnitFrontXZ_WS;
	fmath_SinCos( m_pData->m_fYawWS, &fSin, &fCos );
	BotUnitFrontXZ_WS.x = fSin;
	BotUnitFrontXZ_WS.z = fCos;
	
	m_MountUnitFrontXZ_WS.x = BotUnitFrontXZ_WS.x;
	m_MountUnitFrontXZ_WS.z = BotUnitFrontXZ_WS.z;

	CFVec3A BotUnitRightXZ_WS;
	BotUnitRightXZ_WS.x = BotUnitFrontXZ_WS.z;
	BotUnitRightXZ_WS.z = -BotUnitFrontXZ_WS.x;

	CFVec3A  DeltaToDestXZ;
	DeltaToDestXZ.y = 0.0f; // advised against uninited vars into math funcs. 
	DeltaToDestXZ.x = GotoPointXZ_WS.x - BotPosXZ_WS.x;
	DeltaToDestXZ.z = GotoPointXZ_WS.z - BotPosXZ_WS.z;
	
	CFVec3A UnitDeltaToDestXZ=CFVec3A::m_Null;
	f32 fDistToDest = 0.0f;
	if (DeltaToDestXZ.x || DeltaToDestXZ.z)
	{
		fDistToDest = UnitDeltaToDestXZ.UnitAndMagXZ(DeltaToDestXZ);
	}
	
	// here, I'm aiming the radar in the direction we're looking:
	m_vRadarDirectionWS.x =UnitDeltaToDestXZ.x;
	m_vRadarDirectionWS.y =0.0f;
	m_vRadarDirectionWS.z =UnitDeltaToDestXZ.z;

	f32 fFrontDot;
	f32 fRightDot;

#define DOT_XZ(arg0,arg1) ((arg0.x * arg1.x) + (arg0.z * arg1.z))

	fFrontDot = DOT_XZ(BotUnitFrontXZ_WS, UnitDeltaToDestXZ);
	fRightDot = DOT_XZ(BotUnitRightXZ_WS, UnitDeltaToDestXZ);

	f32 fAbsRightDot = FMATH_FABS(fRightDot);

	if (fAbsRightDot < 0.0005f)		//kfEssentiallyStoppedSpeed)
	{
		fAbsRightDot = fRightDot = 0.0f;
	}
	if( fFrontDot < 0.0f )
	{
		if( fRightDot >= 0.0f )
		{
			fRightDot		= 1.0f;
			fAbsRightDot	= 1.0f;
		}
		else
		{
			fRightDot		= -1.0f;
			fAbsRightDot	= 1.0f;
		}
	}

	f32 fYRadToTarget = fmath_Atan(fRightDot,fFrontDot);
	f32 fYawTargetWS = m_pData->m_fYawWS+fYRadToTarget;

	if (fYawTargetWS < m_pData->m_fMinimumYawWS)
	{
		f32 fAngleDifferenceMin = m_pData->m_fMinimumYawWS - fYawTargetWS;
		f32 fAngleDifferenceMax = (fYawTargetWS+FMATH_2PI) - m_pData->m_fMaximumYawWS;
		if (fAngleDifferenceMin < fAngleDifferenceMax)
			fYawTargetWS = m_pData->m_fMinimumYawWS;
		else
			fYawTargetWS = m_pData->m_fMaximumYawWS;

	}
	if (fYawTargetWS > m_pData->m_fMaximumYawWS)
	{
		f32 fAngleDifferenceMax = fYawTargetWS - m_pData->m_fMaximumYawWS;
		f32 fAngleDifferenceMin = m_pData->m_fMinimumYawWS - (fYawTargetWS - FMATH_2PI);
		if (fAngleDifferenceMax < fAngleDifferenceMin)
			fYawTargetWS = m_pData->m_fMaximumYawWS;
		else
			fYawTargetWS = m_pData->m_fMinimumYawWS;
	}

	m_RotateYAxis.SetTarget(fYawTargetWS);
	*pDistToTarget = fDistToDest;
/*
#if !FANG_PRODUCTION_BUILD
	CFVec3A	vStart,vEnd;
	vStart = BotPosXZ_WS;
	vEnd.ReceiveRotationY(CFVec3A::m_UnitAxisZ,fYawTargetWS);
	vEnd.Mul(fDistToDest + 10.0f);
	vEnd.Add(vStart);
	CDebugDraw::Line(vStart,vEnd);
#endif
*/
	return fAbsRightDot;
}

BOOL CBotSiteWeapon::SetSiteWeaponDriver(CBot* pWhom, BOOL bDriversChoice)
{
	if (!pWhom)
	{
		if (m_pData->m_pDriverBot)
		{
			if (m_pBotDef->m_nSubClass == BOTSUBCLASS_SITEWEAPON_RATGUN)
			{
				if (m_pData->m_pDriverBot->IsCreated() && m_pData->m_pDriverBot->AIBrain() && m_pData->m_pDriverBot->m_nPossessionPlayerIndex < 0)
				{
					// AI gunner is exiting ratgun, RAT will no longer call gunner's work function.
					aibrainman_ClearDisableEntityWorkFlag(m_pData->m_pDriverBot->AIBrain());
				}
			}

			if (m_pData->m_pDriverBot->IsInWorld())
			{
				//
				//  Note, anytime a mech kicks its owner out, it is good to call notify it
				//
				if (m_pData->m_pDriverBot->AIBrain() && (bDriversChoice==FALSE))
				{
					aibrainman_NotifyMechEject(m_pData->m_pDriverBot->AIBrain());
				}

				m_pData->m_pDriverBot->ExitingMechWork();
			}
		}
		if (!IsDeadOrDying())
		{
			if ( (m_pBotDef->m_nSubClass == BOTSUBCLASS_SITEWEAPON_PILLBOX) ||
				(m_pBotDef->m_nSubClass == BOTSUBCLASS_SITEWEAPON_RATGUN) )
			{
				_SetSiteWeaponState(SITE_IDLING);
				FASSERT(!m_pData->m_pDriverBot);	   //pgm added this because it surely seems like now that this site has no driver, it should clear the pointer
				m_pData->m_pDriverBot = NULL;	   //pgm added this because it surely seems like now that this site has no driver, it should clear the pointer
				return TRUE;
			}
			else if (m_pBotDef->m_nSubClass == BOTSUBCLASS_SITEWEAPON_FLOORSENTRY)
			{
				if (MultiplayerMgr.IsSinglePlayer() || Recruit_IsRecruited())
					_SetSiteWeaponState(SITE_PATROLLING);
				else
					_SetSiteWeaponState(SITE_POWER_DOWN);
				FASSERT(!m_pData->m_pDriverBot);	   //pgm added this because it surely seems like now that this site has no driver, it should clear the pointer
				m_pData->m_pDriverBot = NULL;	   //pgm added this because it surely seems like now that this site has no driver, it should clear the pointer
				return TRUE;
			}
		}
		else
		{
			_SetSiteWeaponState(SITE_DEAD);
		}
		FASSERT(!m_pData->m_pDriverBot);	   //pgm added this because it surely seems like now that this site has no driver, it should clear the pointer
		m_pData->m_pDriverBot = NULL;	   //pgm added this because it surely seems like now that this site has no driver, it should clear the pointer
	}
	else //setting the site weapon driver
	{
		if (m_pBotDef->m_nSubClass == BOTSUBCLASS_SITEWEAPON_RATGUN)
		{
			if (pWhom->AIBrain() && pWhom->m_nPossessionPlayerIndex < 0)
			{
				// AI gunner is entering ratgun, RAT will call gunner's work function in order 
				// to prevent gunner's position from lagging behind that of gun.
				aibrainman_SetDisableEntityWorkFlag(pWhom->AIBrain());
			}
		}

		m_pData->m_pDriverBot = pWhom;
		m_pData->m_pDriverBot->EnteringMechWork(this);
		m_nPossessionPlayerIndex = m_pData->m_pDriverBot->m_nPossessionPlayerIndex;	 //need to do this so that CBot::ParseControls works. even though, we're not using the "possess" interface

		if (m_pData->m_eSiteWeaponState == SITE_POWER_DOWN)
		{
			m_nPowerState = POWERSTATE_POWERING_UP;
			_SetSiteWeaponState(SITE_ENTERING_POSSESSED);
			return TRUE;
		}
		else if (this->m_pBotDef->m_nRace == BOTRACE_DROID)  // friendly floor sentry case
		{
			_SetSiteWeaponState(SITE_POSSESSED);
			return TRUE;
		}
		else if (m_pBotDef->m_nSubClass != BOTSUBCLASS_SITEWEAPON_FLOORSENTRY) // new def, site weapons are controllable, except floor sentries, unless powered down;
		{
			_SetSiteWeaponState(SITE_ENTERING_POSSESSED);
			return TRUE;
		}
	}
	return FALSE;
}
BOOL CBotSiteWeapon::GetOpenOrClosed(void)
{
	if (m_pBotDef->m_nSubClass != BOTSUBCLASS_SITEWEAPON_RATGUN)
	{
		// only works on ratguns for now
		return FALSE;
	}
	FASSERT(m_pBotDef->m_nSubClass == BOTSUBCLASS_SITEWEAPON_RATGUN);
	CRatGun* pRatGun = (CRatGun*)m_pGun;
	return pRatGun->GetOpenOrClosed();
}

BOOL CBotSiteWeapon::SetOpenOrClosed(BOOL bOpen, f32 fHowLongItShouldTakeSecs )
{
	if (m_pBotDef->m_nSubClass != BOTSUBCLASS_SITEWEAPON_RATGUN)
	{
		// only works on ratguns for now
		return FALSE;
	}
	FASSERT(m_pBotDef->m_nSubClass == BOTSUBCLASS_SITEWEAPON_RATGUN);
	CRatGun* pRatGun = (CRatGun*)m_pGun;
	return pRatGun->SetOpenOrClosed(bOpen,fHowLongItShouldTakeSecs);
}

// We need to return true when we want to swallow the action event!
BOOL CBotSiteWeapon::ActionNearby( CEntity *pEntity )
{
	CBot::ActionNearby(pEntity);

	if (IsDeadOrDying())
		return FALSE;

	if (pEntity == this)
		pEntity = m_pData->m_pDriverBot;

	if( m_pBotDef->m_nSubClass == BOTSUBCLASS_SITEWEAPON_RATGUN )
	{
		// entry & exit handled by vehicle
		return FALSE;
	}
	
	if (m_pData->m_pDriverBot && 
		pEntity != m_pData->m_pDriverBot &&
		pEntity != this)
	{  //if somebody is in here, then tough.
		return FALSE;
	}

	CBot *pBot = (CBot*) pEntity;

	if (m_pGun->CanOccupyStation(pBot)==FALSE) // entry not allowed
	{
		return FALSE;
	}

	if (m_pData->m_eSiteWeaponState != SITE_POSSESSED &&			   // if we're not possessed (pgm says that this isn't really a good test since there are several states in which possession is true
		m_pData->m_eSiteWeaponState != SITE_ENTERING_POSSESSED &&
		m_pData->m_eSiteWeaponState != SITE_POWERING_UP_POSSESSED)
	{
		CBot* pObstructedBy = IsStationObstructed();		//pgm added this because site wasn't allowing the bot that was trying to get in to get in
		if ( (!pObstructedBy) || (pObstructedBy == pBot) )
		{
			if (pBot->m_nPossessionPlayerIndex < 0) // old path maintained fer pat
			{
				f32 fPillBoxGunnerLocYOffsetMin = 3.6f;
				f32 fPillBoxGunnerLocYOffsetMax = 3.75f;

				CFVec3A vFootPosition,vMyPosition;
				vFootPosition = pBot->MtxToWorld()->m_vPos;
				vMyPosition = MtxToWorld()->m_vPos;
				f32 fVerticalDelta = vFootPosition.y - vMyPosition.y;

				if ((vFootPosition.DistSqXZ(vMyPosition) < pBot->m_fCollCylinderRadius_WS*pBot->m_fCollCylinderRadius_WS) &&
					FMATH_FABS((fVerticalDelta-fPillBoxGunnerLocYOffsetMin)) < 5.0f) 
				{
					SetSiteWeaponDriver(pBot);
					FASSERT(m_pData->m_pDriverBot);
				}
			}
			else
			{
				SetSiteWeaponDriver(pBot);
				FASSERT(m_pData->m_pDriverBot);
				return TRUE;
			}
		}
	}
	else if	((pEntity == m_pData->m_pDriverBot) &&
			 m_pData->m_eSiteWeaponState == SITE_POSSESSED)
	{
		SetSiteWeaponDriver(NULL);
		FASSERT(!m_pData->m_pDriverBot);
		return TRUE;
	}
	return FALSE;
}

void CBotSiteWeapon::ComputeApproxMuzzlePoint_WS( CFVec3A *pApproxMuzzlePoint_WS )
{
	FASSERT(m_pGun);
	m_pGun->ComputeApproxMuzzlePoint_WS(pApproxMuzzlePoint_WS);
}

void CBotSiteWeapon::Power( BOOL bPowerUp, f32 fPowerOffTime, f32 fPowerOffOnSpeedMult )
{
	if ( (m_pBotDef->m_nSubClass == BOTSUBCLASS_SITEWEAPON_PILLBOX) || // no power downs for pillboxes;
		 (m_pBotDef->m_nSubClass == BOTSUBCLASS_SITEWEAPON_RATGUN) )   // no power downs for ratgun;
	{
		return; 
	}

	if ( (m_pData->m_eSiteWeaponState == SITE_NULL) ||		// no power down when state is null
		 (m_pData->m_eSiteWeaponState == SITE_IDLING) ||	// idle reserved for no power sites
		 (m_pData->m_eSiteWeaponState == SITE_ENTERING_POSSESSED) ||	// possession mean no power down
		 (m_pData->m_eSiteWeaponState == SITE_POWERING_UP_POSSESSED) || // possession mean no power down
		 (m_pData->m_eSiteWeaponState == SITE_POSSESSED) ||				// possession mean no power down
		 (m_pData->m_eSiteWeaponState == SITE_DEAD) )					// death == no power down
	{
		return;
	}

	if (bPowerUp)
	{
		_SetSiteWeaponState(SITE_POWER_UP);
	}
	else
	{
		_SetSiteWeaponState(SITE_POWER_DOWN);
	}

	if (bPowerUp == FALSE)
	{
		if (fPowerOffTime > 0.0f)
			m_fPowerOffOnTime = fPowerOffTime + m_pData->m_fTimeToPowerDown;
		else
			m_fPowerOffOnTime = 0.0f;
	}
}

void CBotSiteWeapon::Possess( s32 nPlayerIndex, f32 fPossessionArmorModifier )
{
	// didn't work to use possess call, break in AI?
	CBot::Possess(nPlayerIndex, fPossessionArmorModifier);
}

void CBotSiteWeapon::AppendTrackerSkipList(u32& nTrackerSkipListCount, CFWorldTracker ** apTrackerSkipList) 
{
	FASSERT( IsCreated() );
	m_pGun->AppendTrackerSkipList(nTrackerSkipListCount,apTrackerSkipList);
}

//-----------------------------------------------------------------------------
void CBotSiteWeapon::CheckpointSaveSelect( s32 nCheckpoint )
{
	CheckpointSaveList_AddTailAndMark( nCheckpoint );
}

//-----------------------------------------------------------------------------
// saves state for checkpoint
BOOL CBotSiteWeapon::CheckpointSave( void )
{
	m_MtxToWorld = m_pData->m_MtxPosition_WS;
	
	// save parent class data
	CBot::CheckpointSave();

	CFCheckPoint::SaveData( (u32&) m_pBotDef );
	CFCheckPoint::SaveData(m_pData, sizeof(SiteWeaponData_t));
	CFCheckPoint::SaveData( (u32&) m_fPowerOffOnTime );
	m_pGun->CheckpointSave();
	
	
	return TRUE;
}

//-----------------------------------------------------------------------------
// loads state for checkpoint
void CBotSiteWeapon::CheckpointRestore(void)
{
	// load parent class data
	CBot::CheckpointRestore();

	// since the checkpoint restore adds to the world
	m_pGun->m_pDeadMesh->RemoveFromWorld();
	m_pGun->m_pAliveMesh->RemoveFromWorld();

	SiteWeaponData_t SiteData;

	CFCheckPoint::LoadData( (u32&) m_pBotDef );
	CFCheckPoint::LoadData(&SiteData,sizeof(SiteWeaponData_t));
	CFCheckPoint::LoadData( (u32&) m_fPowerOffOnTime );
	m_pGun->CheckpointRestore(SiteData);
	
	_SetDataMembersToDefaults();
	*m_pData = SiteData;

	if (m_pData->m_eSiteWeaponState == SITE_DEAD)
		m_pWorldMesh = m_pGun->m_pDeadMesh;
	else
		m_pWorldMesh = m_pGun->m_pAliveMesh;

	m_pWorldMesh->UpdateTracker(TRUE);

	m_RotateYAxis.SetValue(m_pData->m_fYawWS);
	m_RotateXAxis.SetValue(m_pData->m_fPitchWS);
}

void CBotSiteWeapon::CheckpointRestorePostamble( void )
{
	CBot::CheckpointRestorePostamble();
	CBotSiteWeapon::State_e NewState = m_pData->m_eSiteWeaponState;
	m_pData->m_eSiteWeaponState = SITE_NULL; // since the previous assignment will reset the state var without transition effects;
	if (m_pData->m_pDriverBot)
	{
		SetSiteWeaponDriver(m_pData->m_pDriverBot);
		if ((m_pBotDef->m_nSubClass != BOTSUBCLASS_SITEWEAPON_RATGUN) && (m_pBotDef->m_nSubClass != BOTSUBCLASS_SITEWEAPON_PILLBOX))
		{
			// in the case of the ratgun, the SetSiteWeaponDriver() function above sets the state to
			// SITE_ENTERING_POSSESSED.  We can't immediately set the state to SITE_POSSESSED as is done below,
			// because then the work for the SITE_ENTERING_POSSESSED state never gets called, which messes up the
			// ratgun restore.

			// ME 04.27.03 - added the pillbox check above for what seems to be the same reason
			_SetSiteWeaponState(SITE_POSSESSED);
		}

	}
	else if (NewState == SITE_DEAD)
	{
		m_pData->m_eSiteWeaponState = SITE_DEAD; // you're dead, stay dead, no effects
		// _SetSiteWeaponState(SITE_DEAD);
	}
	else if ( (m_pBotDef->m_nSubClass == BOTSUBCLASS_SITEWEAPON_PILLBOX) ||
		(m_pBotDef->m_nSubClass == BOTSUBCLASS_SITEWEAPON_RATGUN) )
		_SetSiteWeaponState(SITE_IDLING);
	else
		_SetSiteWeaponState(NewState);
	
	m_pGun->Work();
}

u32 CBotSiteWeapon::GetTagPointCount( void ) const
{
	FASSERT(m_pGun);
	return m_pGun->GetTagPointCount();
}
const CFVec3A *CBotSiteWeapon::GetTagPoint( u32 nTagPointIndex ) const
{
	FASSERT(m_pGun);
	return m_pGun->GetTagPoint(nTagPointIndex);
}

void CBotSiteWeapon::_RotateBaseNoise(BOOL bOn, f32 fUnitVolume)
{
	if (_bDisableAudio)
		return;
	if (m_pSoundEmitters[SITESND_ROTATE]==NULL)
		return;
	// no rotate noise for pb 
	if (m_pBotDef->m_nSubClass == BOTSUBCLASS_SITEWEAPON_PILLBOX)
		return;
	// no rotate noise for rat
	if (m_pBotDef->m_nSubClass == BOTSUBCLASS_SITEWEAPON_RATGUN)
		return;

	if (bOn && !m_pSoundEmitters[SITESND_ROTATE])
	{
		m_pSoundEmitters[SITESND_ROTATE]=AllocAndPlaySound(m_pSounds[SITESND_ROTATE]);
	}
	if (!bOn && m_pSoundEmitters[SITESND_ROTATE])
	{
		m_pSoundEmitters[SITESND_ROTATE]->Destroy();
		m_pSoundEmitters[SITESND_ROTATE] = FALSE;
	}

	if (bOn)
	{
 		f32 fVolDelta = fUnitVolume - m_fRotateVolume;
		if (fVolDelta > 0.005f) // we want to increase our volume
			m_fRotateVolume += FLoop_fPreviousLoopSecs;
		if (fVolDelta < 0.005f) // we want to decrease our volume
			m_fRotateVolume -= FLoop_fPreviousLoopSecs;
		
		FMATH_CLAMP(m_fRotateVolume, .35f, 1.0f); 
		m_pSoundEmitters[SITESND_ROTATE]->SetVolume(m_fRotateVolume);
	}
}

CBotSiteWeaponBuilder::CBotSiteWeaponBuilder()
{
	m_MilFloorSentryBotDef.m_nRace		 =	BOTRACE_MIL;
	m_MilFloorSentryBotDef.m_nClass	     =	BOTCLASS_SITEWEAPON;
	m_MilFloorSentryBotDef.m_nSubClass   =	BOTSUBCLASS_SITEWEAPON_FLOORSENTRY;
	
    m_DroidFloorSentryBotDef.m_nRace	 =	BOTRACE_DROID;
	m_DroidFloorSentryBotDef.m_nClass	 =	BOTCLASS_SITEWEAPON;
	m_DroidFloorSentryBotDef.m_nSubClass =	BOTSUBCLASS_SITEWEAPON_FLOORSENTRY;

	m_WallSentryBotDef.m_nRace			 =	BOTRACE_MIL;
	m_WallSentryBotDef.m_nClass			 =	BOTCLASS_SITEWEAPON;
	m_WallSentryBotDef.m_nSubClass		 =	BOTSUBCLASS_SITEWEAPON_WALLSENTRY;
	
	m_AmbientPillBoxBotDef.m_nRace		=	BOTRACE_AMBIENT;
	m_AmbientPillBoxBotDef.m_nClass		=	BOTCLASS_SITEWEAPON;
	m_AmbientPillBoxBotDef.m_nSubClass	=	BOTSUBCLASS_SITEWEAPON_PILLBOX;

	m_DroidPillBoxBotDef.m_nRace		=	BOTRACE_DROID;
	m_DroidPillBoxBotDef.m_nClass		=	BOTCLASS_SITEWEAPON;
	m_DroidPillBoxBotDef.m_nSubClass	=	BOTSUBCLASS_SITEWEAPON_PILLBOX;

	m_AmbientRatGunBotDef.m_nRace		=	BOTRACE_AMBIENT;
	m_AmbientRatGunBotDef.m_nClass		=	BOTCLASS_SITEWEAPON;
	m_AmbientRatGunBotDef.m_nSubClass	=	BOTSUBCLASS_SITEWEAPON_RATGUN;

	m_DroidRatGunBotDef.m_nRace			=	BOTRACE_DROID;
	m_DroidRatGunBotDef.m_nClass		=	BOTCLASS_SITEWEAPON;
	m_DroidRatGunBotDef.m_nSubClass		=	BOTSUBCLASS_SITEWEAPON_RATGUN;
}

BOOL CBotSiteWeapon::GetRandomPointOnMeshSurface(CFVec3A& rPoint, CFVec3A& rNormal, const CFWorldMesh* pMesh)
{
	CFSphere rSphere = pMesh->GetBoundingSphere();
	CFVec3A vRandomPt;
	fmath_RandomPointInSphericalSector(vRandomPt, rSphere.m_fRadius,rSphere.m_fRadius,FMATH_HALF_PI);
	CFVec3A vRandomPtWS;
	vRandomPtWS.v3 = rSphere.m_Pos;
	vRandomPtWS.Add(vRandomPt);

	FCollImpact_t CollImpact;
	BOOL bImpact = fworld_FindClosestImpactPointToRayStart( &CollImpact, &vRandomPtWS, &pMesh->m_Xfm.m_MtxF.m_vPos);
	if (!bImpact) // failed to collide
		return FALSE;
	if (!CollImpact.pTag)
		return FALSE;
	
	CFWorldMesh *pWorldMesh = (CFWorldMesh *)CollImpact.pTag;
	if (pWorldMesh != pMesh)
		return FALSE;


	rPoint.Set(CollImpact.ImpactPoint);
	rNormal.Set(CollImpact.UnitFaceNormal);
	return TRUE;
}

#define SELF_DAMAGE_PERCENT 0.10f

void CBotSiteWeapon::InflictDamageResult( const CDamageResult *pDamageResult ) 
{

	if( m_pBotDef->m_nSubClass == BOTSUBCLASS_SITEWEAPON_RATGUN )
	{
		// rat guns don't take damage
		return;
	}

	CBot* pWhoHitMe = pDamageResult->m_pDamageData->m_Damager.pBot; // when hit by a possessed bot, 

	if (pWhoHitMe==this) // self-damage, 
	{
		if (m_nPossessionPlayerIndex < 0)
		{
			return; // either discard
		}
		else	// or apply a fraction of the damage;
		{
			for(s32 i=0; i<DAMAGE_HITPOINT_TYPE_COUNT; ++i ) 
			{
				((CDamageResult *)pDamageResult)->m_afDeltaHitpoints[i] *= SELF_DAMAGE_PERCENT;
			}
		}
	}
	
	if (pDamageResult->m_fDeltaHitpoints > 0.0f) // if I'm hit, do something
		m_bDamageTaken = TRUE;
	else										// if we're just shakey, s'okey. 
		return;

	m_vImpactPoint_WS = pDamageResult->m_pDamageData->m_ImpactPoint_WS;
	m_vUnitDirToDamageSource = pDamageResult->m_pDamageData->m_AttackUnitDir_WS;
	m_vUnitDirToDamageSource.Negate();

	if	(pWhoHitMe && 
		(pWhoHitMe != this) && 
		(pWhoHitMe != m_pData->m_pDriverBot) && 
		(pWhoHitMe->m_nPossessionPlayerIndex >= 0))  // set him as next enemy target
	{
		// For multiplayer, only do this if the hitter is not friendly
		if ( MultiplayerMgr.IsSinglePlayer() || !aiutils_IsFriendly(this, pWhoHitMe) )
		{
			m_pDisguisedBot = pWhoHitMe;

			m_pData->m_pEnemyTarget = pWhoHitMe;
			m_pData->m_pEnemyTargetDistance2 = FMATH_MAX_FLOAT; // distance not relevant, if I can line of sight you, you're my enemy

			// Not sure why we are unrecruiting when shot at, but at least don't do
			// it in multiplayer, where recruitment is serious business
			if ( (m_pBotDef->m_nSubClass==BOTSUBCLASS_SITEWEAPON_FLOORSENTRY) && MultiplayerMgr.IsSinglePlayer() )
			{
				CBotSiteWeaponBuilder* pBuilder =  (CBotSiteWeaponBuilder*)GetLeafClassBuilder();
				//m_pBotDef = ((pWhoHitMe->m_pBotDef->m_nRace == BOTRACE_MIL) ? &pBuilder->m_DroidFloorSentryBotDef : &pBuilder->m_MilFloorSentryBotDef);
				// always vote mil when responding to damage!
				if( m_nRecruitID >= 0 ) 
				{
					CWeaponRecruiter::UnrecruitBot(this, TRUE);
				}
				//m_pBotDef = &pBuilder->m_MilFloorSentryBotDef;
			}
		}
	}

	f32 fOldHealth = NormHealth();

	CBot::InflictDamageResult( pDamageResult );

	f32 fNormHealth = NormHealth();

    if ((fOldHealth > 0.0f) && (fNormHealth <= 0.0f))
	{
		if( m_pDeathInfo && m_pDeathInfo->hExplosion != FEXPLOSION_INVALID_HANDLE ) 
		{
			FExplosion_SpawnerHandle_t hSpawner = CExplosion2::GetExplosionSpawner();

			if( hSpawner != FEXPLOSION_INVALID_HANDLE ) 
			{
				FExplosionSpawnParams_t SpawnParams;

				SpawnParams.InitToDefaults();
				SpawnParams.pDamager = &pDamageResult->m_pDamageData->m_Damager;
				SpawnParams.Pos_WS.Set(pDamageResult->m_pDamageData->m_ImpactPoint_WS);
				SpawnParams.UnitDir.Set(pDamageResult->m_pDamageData->m_AttackUnitDir_WS);

				CExplosion2::SpawnExplosion( hSpawner, m_pDeathInfo->hExplosion, &SpawnParams );
			}
		}		
	}
}

BOOL CBotSiteWeapon::GetStationEntryPoint( CFVec3A *pPoint )
{
	s32 nBoneIdxAttachPt = -1;
	switch (m_pBotDef->m_nSubClass)
	{
	default:
	case BOTSUBCLASS_SITEWEAPON_WALLSENTRY:
	case BOTSUBCLASS_SITEWEAPON_RATGUN:
		return FALSE;

	case BOTSUBCLASS_SITEWEAPON_FLOORSENTRY:
		FASSERT(m_nBoneIndexFloorSentryAttachPt >= 0);
		nBoneIdxAttachPt = m_nBoneIndexFloorSentryAttachPt;
		break;

	case BOTSUBCLASS_SITEWEAPON_PILLBOX:
		FASSERT(m_nBoneIndexPillboxAttachPt >= 0);
		nBoneIdxAttachPt = m_nBoneIndexPillboxAttachPt;
		break;
	}
	
	FASSERT(nBoneIdxAttachPt >=0);
	*pPoint = m_pWorldMesh->GetBoneMtxPalette()[nBoneIdxAttachPt]->m_vPos;
	return TRUE;
}

CBot *CBotSiteWeapon::IsStationObstructed( void )
{
	if (m_pData->m_pDriverBot) 
		return m_pData->m_pDriverBot;


	s32 nBoneIdxAttachPt;
	switch (m_pBotDef->m_nSubClass)
	{
	default:
	case BOTSUBCLASS_SITEWEAPON_WALLSENTRY:
	case BOTSUBCLASS_SITEWEAPON_RATGUN:
		return NULL;
	
	case BOTSUBCLASS_SITEWEAPON_FLOORSENTRY:
		if (! ((this->m_pBotDef->m_nRace == BOTRACE_DROID)||(m_pData->m_eSiteWeaponState == SITE_POWER_DOWN)) )
			return this;
		
		FASSERT(m_nBoneIndexFloorSentryAttachPt >= 0);
		nBoneIdxAttachPt = m_nBoneIndexFloorSentryAttachPt;
		break;

	case BOTSUBCLASS_SITEWEAPON_PILLBOX:
		FASSERT(m_nBoneIndexPillboxAttachPt >= 0);
		nBoneIdxAttachPt = m_nBoneIndexPillboxAttachPt;
		break;
	}
	return CVehicle::StationObstructed( this, nBoneIdxAttachPt, 3.0f );		
}

BOOL CBotSiteWeapon::BotIsEligibleToOperatePillbox(CBot* pBot)
{
	if ( !((m_pBotDef->m_nSubClass==BOTSUBCLASS_SITEWEAPON_PILLBOX) ||
		 (m_pBotDef->m_nSubClass==BOTSUBCLASS_SITEWEAPON_RATGUN)) )
		return FALSE;

	if( pBot->m_apWeapon[0] && pBot->m_apWeapon[0]->Type() == CWeapon::WEAPON_TYPE_ROCKET_LAUNCHER ) 
		return FALSE;
	return TRUE;
}


void CBotSiteWeapon::AnimBoneCallback( u32 uBoneIndex, CFMtx43A &rNewMtx, const CFMtx43A &rParentMtx, const CFMtx43A &rBoneMtx ) 
{
	CRatGun *pGun;
	CBotSiteWeapon *pSite;

	if( m_pCollBot && (m_pCollBot->TypeBits() & ENTITY_BIT_SITEWEAPON) && m_pCollBot->m_pBotDef->m_nSubClass == BOTSUBCLASS_SITEWEAPON_RATGUN )
	{

		pSite = (CBotSiteWeapon*) m_pCollBot;
		pGun = (CRatGun*) pSite->m_pGun;

		if( pSite->GetParent() && pSite->GetParent()->TypeBits() & ENTITY_BIT_VEHICLERAT )
		{
			CVehicleRat *pRat;
			pRat = (CVehicleRat*) pSite->GetParent();

			if( pRat->IsFinalBoneUpdate() )
			{
				// prevent antenna strobing if compute matrix palette is called more than once per frame
				if( pGun->m_Antenna.AnimateBone( uBoneIndex, rNewMtx, rParentMtx, rBoneMtx ) )
				{
					return;
				}
			}
		}
	}
}

BotRace_e CBotSiteWeapon::SetRace( BotRace_e eNewRace)
{
	BotRace_e eOldRace = m_pBotDef->m_nRace;
	if (m_pBotDef->m_nSubClass==BOTSUBCLASS_SITEWEAPON_FLOORSENTRY)
	{
		CBotSiteWeaponBuilder *pBuilder = (CBotSiteWeaponBuilder *)GetLeafClassBuilder();
		m_pBotDef = ((eNewRace == BOTRACE_DROID) ? &pBuilder->m_DroidFloorSentryBotDef : &pBuilder->m_MilFloorSentryBotDef);
	}
	else // nothing happens
	{
	}
	return eOldRace;
}