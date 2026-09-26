//////////////////////////////////////////////////////////////////////////////////////
// weapon_laser.cpp - Mining Laser weapon.
//
// Author: Justin Link
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
// 06/19/02 Link		Created.
//////////////////////////////////////////////////////////////////////////////////////

#include "fang.h"
#include "weapon_laser.h"
#include "fgamedata.h"
#include "fworld.h"
#include "fmesh.h"
#include "fanim.h"
#include "fresload.h"
#include "meshtypes.h"
#include "floop.h"
#include "bot.h"
#include "fforce.h"
#include "player.h"
#include "explosion.h"
#include "potmark.h"
#include "fcamera.h"
#include "ItemInst.h"
#include "LaserBeam.h"
#include "fsndfx.h"
#include "AI/AIEnviro.h"
#include "EShield.h"
#include "damage.h"
#include "gcoll.h"
#include "fdecal.h"

#include "sas_user.h"


#define _USER_PROP_FILENAME		"w_laser.csv"


static const u32 _uFizzleThreshhold = 25;


//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CWeaponLaserBuilder
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

static CWeaponLaserBuilder _WeaponLaserBuilder;


void CWeaponLaserBuilder::SetDefaults( u64 nEntityTypeBits, u64 nEntityLeafTypeBit, cchar *pszEntityType ) {
	ENTITY_BUILDER_SET_PARENT_CLASS_DEFAULTS( CWeaponBuilder, ENTITY_BIT_WEAPONLASER, pszEntityType );
}


BOOL CWeaponLaserBuilder::PostInterpretFixup( void ) {
	return CWeaponBuilder::PostInterpretFixup();
}


BOOL CWeaponLaserBuilder::InterpretTable( void ) {
	return CWeaponBuilder::InterpretTable();
}




//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CWeaponLaser
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

CWeaponLaser::_UserProps_t CWeaponLaser::m_aUserProps[EUK_COUNT_LASER];
CDamageProfile *CWeaponLaser::m_apDamageProfile[EUK_COUNT_LASER];
TracerDef_t CWeaponLaser::m_aoTracerDef[EUK_COUNT_LASER];
CFTexInst CWeaponLaser::m_aTracerTexInst[EUK_COUNT_LASER];


// This table describes to fgamedata how our user property table is to be interpreted:
const FGameData_TableEntry_t CWeaponLaser::m_aUserPropVocab[] = {
	// apszMeshName[_MESH_WEAPON]:
	FGAMEDATA_VAR_TYPE_STRING|
	FGAMEDATA_FLAGS_STRING_PTR_TO_MAIN_STR_TBL,
	sizeof( char * ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	// fClipAmmoMax:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_Neg1,
	F32_DATATABLE_65534,

	// fReserveAmmoMax:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_Neg1,
	F32_DATATABLE_65534,

	// fRoundsPerSec:
	// fOORoundsPerSec:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO | FGAMEDATA_FLAGS_FLOAT_OO_X,
	sizeof( f32 ) * 2,
	F32_DATATABLE_Pt001,
	F32_DATATABLE_1000,

	// fWeaponCullDist:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_100000,

	// fRocketsInPoolCount:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_1,
	F32_DATATABLE_1000,

	// fMaxLiveRange:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_1,
	F32_DATATABLE_100000,

	// fUnitRecoil:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_1,

	// fRocketCullDist:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_100000,

	// fRechargeRate:
	// fOORechargeRate:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO | FGAMEDATA_FLAGS_FLOAT_OO_X,
	sizeof( f32 ) * 2,
	F32_DATATABLE_0,
	F32_DATATABLE_100000,

	// fRocketSpeed:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_1,
	F32_DATATABLE_100000,

	// fTracerLength
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_500,

	// fTracerWidth
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_10,

	// fDistFromWeaponOrigToMuzzle:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_1,
	F32_DATATABLE_100000,

	// fMinTargetAssistDist:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_100000,

	// fMaxTargetAssistDist:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_100000,

	// pszL12_TexName
	FGAMEDATA_VAR_TYPE_STRING|
	FGAMEDATA_FLAGS_STRING_PTR_TO_MAIN_STR_TBL,
	sizeof( char * ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	// pszMuzzleTex
	FGAMEDATA_VAR_TYPE_STRING|
	FGAMEDATA_FLAGS_STRING_PTR_TO_MAIN_STR_TBL,
	sizeof( char * ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	// fMuzzleWidth
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_100000,

	// fMuzzleHeight
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_100000,

	// pszDamageProfile:
	FGAMEDATA_VAR_TYPE_STRING|
	FGAMEDATA_FLAGS_STRING_PTR_TO_MAIN_STR_TBL,
	sizeof( char * ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	// hFiringSound:
	FGAMEDATA_VAR_TYPE_STRING|
	FGAMEDATA_FLAGS_STRING_PTR_TO_SFX_HANDLE,
	sizeof( u32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	FGAMEDATA_VOCAB_DECAL_DEF,

	// End of table:
	FGAMEDATA_VAR_TYPE_COUNT| 0, 0, F32_DATATABLE_0, F32_DATATABLE_0
};


const FGameDataMap_t CWeaponLaser::m_aUserPropMapTable[] = {
	"LaserL1",
	m_aUserPropVocab,
	sizeof(m_aUserProps),
	(void *)&m_aUserProps[0],

	"LaserL2",
	m_aUserPropVocab,
	sizeof(m_aUserProps),
	(void *)&m_aUserProps[1],

	"LaserL3",
	m_aUserPropVocab,
	sizeof(m_aUserProps),
	(void *)&m_aUserProps[2],

	"LaserMil",
	m_aUserPropVocab,
	sizeof(m_aUserProps),
	(void *)&m_aUserProps[3],

	"LaserMil_Possessed",
	m_aUserPropVocab,
	sizeof(m_aUserProps),
	(void *)&m_aUserProps[4],

	NULL
};



BOOL CWeaponLaser::InitSystem( void ) {
	Info_t *pInfo;
	u32 i;
	FTexDef_t *pTexDef;

	FResFrame_t ResFrame = fres_GetFrame();

	fang_MemZero( m_aUserProps, sizeof(m_aUserProps) );

	// Read the user properties for all EUK levels of this weapon...
	if( !fgamedata_ReadFileUsingMap( m_aUserPropMapTable, _USER_PROP_FILENAME ) ) {
		DEVPRINTF( "CWeaponLaser::InitSystem(): Could not read user properties from file '%s'.\n", _USER_PROP_FILENAME );
		goto _ExitWithError;
		return FALSE;
	}

	// Do this for each EUK level...
	for( i=0; i<EUK_COUNT_LASER; i++ ) {
		if(i != 2)
		{
			m_aoTracerDef[i].pUser = NULL;				// This will get set at the point of spawning the tracers.
			m_aoTracerDef[i].pFcnKillCallback = _TracerKilledCallback;
			m_aoTracerDef[i].fWidth_WS = m_aUserProps[i].fTracerWidth;
			m_aoTracerDef[i].fLength_WS = m_aUserProps[i].fTracerLength;
			m_aoTracerDef[i].fSpeed_WS = m_aUserProps[i].fRocketSpeed;
			m_aoTracerDef[i].fMaxTailDist_WS = m_aUserProps[i].fMaxLiveRange;
			m_aoTracerDef[i].fBeginDeathUnitFade_WS = 0.25f;
			m_aoTracerDef[i].uFlags = TRACERFLAG_NONE;

			pTexDef = (FTexDef_t *)(fresload_Load(FTEX_RESNAME, m_aUserProps[i].pszL12_TexName));
			if(pTexDef == NULL)
			{
				DEVPRINTF("CWeaponLaser::InitSystem() : Could not load tracer texture '%s'.\n", m_aUserProps[i].pszL12_TexName);
			}

			m_aTracerTexInst[i].SetTexDef( pTexDef );
		}

		// Fill out our global info data...
		pInfo = &m_aaInfo[WEAPON_TYPE_LASER][i];

		// Do any special limitations on the values we got from the CSV file.

		pInfo->nGrip = GRIP_RIGHT_ARM;
		pInfo->nReloadType = RELOAD_TYPE_FULLYAUTOMATIC;
		pInfo->nStanceType = STANCE_TYPE_STANDARD;
		pInfo->fMinTargetAssistDist = m_aUserProps[i].fMinTargetAssistDist;
		pInfo->fMaxTargetAssistDist = m_aUserProps[i].fMaxTargetAssistDist;
		pInfo->fMaxLiveRange = m_aUserProps[i].fMaxLiveRange;
		pInfo->fUnitRecoil = m_aUserProps[i].fUnitRecoil;
		pInfo->nClipAmmoMax = (m_aUserProps[i].fClipAmmoMax) >= 0.0f ? (u16)m_aUserProps[i].fClipAmmoMax : INFINITE_AMMO;
		pInfo->nReserveAmmoMax = (m_aUserProps[i].fReserveAmmoMax) >= 0.0f ? (u16)m_aUserProps[i].fReserveAmmoMax : INFINITE_AMMO;
		pInfo->nInfoFlags = INFOFLAG_NONE;
		pInfo->nReticleType = CReticle::TYPE_DROID_STANDARD;

		m_apDamageProfile[i] = CDamage::FindDamageProfile( m_aUserProps[i].pszDamageProfile );
	}

	// Success...

	return TRUE;

	// Error...
_ExitWithError:
	fres_ReleaseFrame( ResFrame );
	return FALSE;
}


void CWeaponLaser::UninitSystem( void ) {
}



CWeaponLaser::CWeaponLaser() {
	_ClearDataMembers();
}



CWeaponLaser::~CWeaponLaser() {
	if( IsSystemInitialized() && IsCreated() ) {
		DetachFromParent();
		DetachAllChildren();
		RemoveFromWorld( TRUE );
		ClassHierarchyDestroy();
	}
}



BOOL CWeaponLaser::Create( cchar *pszEntityName, const CFMtx43A *pMtx, cchar *pszAIBuilderName ) {
	// Get pointer to the leaf class's builder object...
	CWeaponLaserBuilder *pBuilder = (CWeaponLaserBuilder *)GetLeafClassBuilder();

	// If we're the leaf class, set the builder defaults...
	if( pBuilder == &_WeaponLaserBuilder ) {
		pBuilder->SetDefaults( 0, 0, ENTITY_TYPE_WEAPONLASER );
	}

	m_pSoundEmitter = NULL;

	// Set our builder parameters...

	// Create an entity...
	return CWeapon::Create( WEAPON_TYPE_LASER, pszEntityName, pMtx, pszAIBuilderName );
}



BOOL CWeaponLaser::ClassHierarchyBuild( void ) {
	u32 i;//, j;
	FMesh_t *pMesh;
	FMeshInit_t MeshInit;
	_ResourceData_t *pResourceData;
	Info_t *pInfo;
	u32 uMeshEUK;
	u32 uResEUK;

	FASSERT( IsSystemInitialized() );
	FASSERT( !IsCreated() );
	FASSERT( FWorld_pWorld );

	// Get a frame...
	FResFrame_t ResFrame = fres_GetFrame();

	// Get pointer to the leaf class's builder object...
	CWeaponLaserBuilder *pBuilder = (CWeaponLaserBuilder *)GetLeafClassBuilder();

	// Set input parameters for CWeapon creation...

	// Build parent class...
	if( !CWeapon::ClassHierarchyBuild() ) {
		// Parent class could not be built...
		goto _ExitWithError;
	}

	// Set defaults...
	_ClearDataMembers();

	// Initialize from builder object...

	for( i=0, pResourceData=m_aResourceData; i<EUK_COUNT_LASER; ++i, ++pResourceData ) {
		// Each EUK level...
		if( m_nSingleMeshForEUKs >= 0 ) {
			FASSERT( m_nSingleMeshForEUKs < EUK_COUNT_LASER );
			uMeshEUK = m_nSingleMeshForEUKs;
		} else {
			uMeshEUK = i;
		}

		pInfo = &m_aaInfo[WEAPON_TYPE_LASER][i];

		// Create the world mesh...
		pResourceData->m_pWorldMesh = fnew CFWorldMesh;
		if( pResourceData->m_pWorldMesh == NULL ) {
			DEVPRINTF( "CWeaponLaser::Create(): Not enough memory to create m_pWorldMesh.\n" );
			goto _ExitWithError;
		}

		// Load the mesh resource...
		pMesh = (FMesh_t *)fresload_Load( FMESH_RESTYPE, m_aUserProps[uMeshEUK].apszMeshName[_MESH_WEAPON] );


		if( pMesh == NULL ) {
			DEVPRINTF( "CWeaponLaser::Create(): Could not find mesh '%s'.\n", m_aUserProps[i].apszMeshName[_MESH_WEAPON] );

			pMesh = m_pErrorMesh;
			if( pMesh == NULL ) {
				goto _ExitWithError;
			}
		}

		// Init the world mesh...
		MeshInit.pMesh = pMesh;
		MeshInit.nFlags = FMESHINST_FLAG_NOBONES;
		MeshInit.fCullDist = m_aUserProps[i].fWeaponCullDist;
		MeshInit.Mtx = *MtxToWorld();

		uResEUK = i;
		if( !CreateSharedEUKData( &MeshInit, &uResEUK, &(pResourceData->m_pWorldMesh) ) ) {
			DEVPRINTF( "CWeaponLaser::ClassHierarchyBuild():  Error creating EUK shared data\n" );
			goto _ExitWithError;
		}

		if( uResEUK == i ) {
			pResourceData->m_pWorldMesh->Init( &MeshInit );
			pResourceData->m_pWorldMesh->RemoveFromWorld();
			pResourceData->m_pWorldMesh->SetCollisionFlag( TRUE );
			pResourceData->m_pWorldMesh->m_nUser = MESHTYPES_ENTITY;
			pResourceData->m_pWorldMesh->m_pUser = this;
			pResourceData->m_pWorldMesh->SetUserTypeBits( TypeBits() );
			pResourceData->m_pWorldMesh->SetLineOfSightFlag(FALSE); //pgm added this so that weapons never block los tests
		}

		if(uMeshEUK == 2)
		{
			m_apL3_LaserBeam[0] = CLaserBeam::GetLaserBeam();
			m_apL3_LaserBeam[1] = CLaserBeam::GetLaserBeam();
			if((m_apL3_LaserBeam[0] == NULL) || (m_apL3_LaserBeam[1] == NULL))
			{
				DEVPRINTF("CWeaponLaser::ClassHierarchyBuild(): Could not get laser beam object.\n");
				goto _ExitWithError;
			}
		}
	}

	m_hTracerGroup = tracer_CreateGroup(&m_aTracerTexInst[m_nUpgradeLevel], 18);
	if(m_hTracerGroup == TRACER_NULLGROUPHANDLE)
	{
		DEVPRINTF( "CWeaponLaser::ClassHierarchyBuild(): Could not create tracer group.\n" );
		goto _ExitWithError;
	}

	m_pResourceData = &m_aResourceData[ m_nUpgradeLevel ];

	fforce_NullHandle( &m_hForce );
	m_bFiredLastFrame = FALSE;
	return TRUE;

	// Error:
_ExitWithError:
	Destroy();
	fres_ReleaseFrame( ResFrame );
	return FALSE;
}


BOOL CWeaponLaser::ClassHierarchyBuilt( void ) {
	FASSERT( IsSystemInitialized() );
	FASSERT( IsCreated() );

	FResFrame_t ResFrame = fres_GetFrame();

	if( !CWeapon::ClassHierarchyBuilt() ) {
		goto _ExitWithError;
	}

	EnableOurWorkBit();

	return TRUE;

_ExitWithError:
	Destroy();
	fres_ReleaseFrame( ResFrame );
	return FALSE;
}


void CWeaponLaser::ClassHierarchyDestroy( void ) {
	// Destroy ourselves first...

	fdelete( m_apL3_LaserBeam[0] );
	fdelete( m_apL3_LaserBeam[1] );

	fforce_Kill( &m_hForce );

	tracer_DestroyGroup(m_hTracerGroup);
	m_hTracerGroup = TRACER_NULLGROUPHANDLE;


	for( u32 i=0; i<EUK_COUNT_LASER; i++ ) {
		DestroySharedEUKData( i );
	}

	_ClearDataMembers();

	// Now destroy our parent...
	CWeapon::ClassHierarchyDestroy();
}


CEntityBuilder *CWeaponLaser::GetLeafClassBuilder( void ) {
	return &_WeaponLaserBuilder;
}


void CWeaponLaser::_ClearDataMembers( void ) {
	u32 i;//, j;

	for( i=0; i<EUK_COUNT_LASER; ++i )
	{
		m_aResourceData[i].m_pWorldMesh = NULL;
	}

	m_pResourceData = NULL;
	m_fSecondsCountdownTimer = 0.0f;
	m_fRechargeCntDn = 0.0f;
	m_L3_fDrainCntDn = 0.0f;
	m_bFireThisFrame = FALSE;
	m_bRechargeThisFrame = FALSE;
	m_bFiredLastFrame = FALSE;
	m_hTracerGroup = TRACER_NULLGROUPHANDLE;
	m_pvecBuddyFirePos_WS = NULL;

	m_vTargetPos_WS.Zero();
	m_vecL3_EndPoint.Zero();
	m_apL3_LaserBeam[0] = NULL;
	m_apL3_LaserBeam[1] = NULL;
}


void CWeaponLaser::_TracerKilledCallback( TracerDef_t *pTracerDef, TracerKillReason_e nKillReason, const FCollImpact_t *pImpact ) {
	if( nKillReason == TRACER_KILLREASON_HIT_GEO ) {
		FASSERT(pTracerDef->pUser != NULL);
		CWeaponLaser *pLaser = (CWeaponLaser *)(pTracerDef->pUser);

		const CGCollMaterial *pCollMaterial = CGColl::GetMaterial( pImpact->nUserType );

		pCollMaterial->DrawAllDrawableParticles(
			&pImpact->ImpactPoint,
			&pImpact->UnitFaceNormal,
			TRUE,
			fmath_RandomFloatRange( 0.0f, 0.0f ),
			fmath_RandomFloatRange( 0.0f, 0.1f ),
			fmath_RandomFloatRange( 0.0f, 0.1f )
		);

//#if SAS_ACTIVE_USER == SAS_USER_ELLIOTT
//	fforce_Kill( &pLaser->m_hForce );
//
//	FDecalDefHandle_t hDecalDef;
//	hDecalDef = CFDecal::GetDefByName( "explosion" );
//	CFDecalDef decaldef = *(CFDecalDef*)hDecalDef;
//	decaldef.fMinRadius = decaldef.fMaxRadius = 20.0f;
//	CFDecal::Create( &decaldef, pImpact, &pTracerDef->UnitDir_WS );
//
//#endif

		//FDecalDefHandle_t hDecal = CFDecal::GetDefByName( "rocket" );

		CFDecal::Create( m_aUserProps[pLaser->m_nUpgradeLevel].hDecalDef, pImpact, &pTracerDef->UnitDir_WS );
		//CFDecal::Create( hDecal, pImpact );

		if(pImpact->pTag == NULL)
		{
//#if !SAS_ACTIVE_USER == SAS_USER_ELLIOTT
//			if( pCollMaterial->IsPockMarkEnabled( pImpact ) )
//			{
//				potmark_NewPotmark(&pImpact->ImpactPoint, &pImpact->UnitFaceNormal);
//			}
//#endif
		
		}


		else
		{
			CFWorldMesh *pHitWorldMesh = (CFWorldMesh *)(pImpact->pTag);

			// See if the world mesh that we found is an entity...
			if(pHitWorldMesh->m_nUser == MESHTYPES_ENTITY)
			{
				// It's an Entity!
				CEntity *pEDamagee = (CEntity *)(pHitWorldMesh->m_pUser);

				//if( pCollMaterial->IsPockMarkEnabled( pImpact ) )
				//{
				//	if( !(pEDamagee->TypeBits() & ENTITY_BIT_BOT) && !(pEDamagee->TypeBits() & ENTITY_BIT_SHIELD) && !(pEDamagee->TypeBits() & ENTITY_BIT_PROJECTILE)) {
				//		potmark_NewPotmark( &pImpact->ImpactPoint, &pImpact->UnitFaceNormal );
				//	}
				//}

				// Get an empty damage form...
				CDamageForm *pDamageForm = CDamage::GetEmptyDamageFormFromPool();

				if( pDamageForm )
				{
					// Fill out the form...

					pDamageForm->m_nDamageLocale = CDamageForm::DAMAGE_LOCALE_IMPACT;
					pDamageForm->m_nDamageDelivery = CDamageForm::DAMAGE_DELIVERY_ONE_SPECIFIC_ENTITY;
					pDamageForm->m_pDamageProfile = pLaser->m_apDamageProfile[pLaser->m_nUpgradeLevel];
					pDamageForm->m_Damager.pWeapon = pLaser;
					pDamageForm->m_Damager.pBot = pLaser->GetOwner();
					pDamageForm->m_Damager.nDamagerPlayerIndex = pLaser->GetOwner() ? pLaser->GetOwner()->m_nPossessionPlayerIndex : -1;
					pDamageForm->m_pDamageeEntity = pEDamagee;
					pDamageForm->InitTriDataFromCollImpact( pHitWorldMesh, pImpact, &pTracerDef->UnitDir_WS );

					CDamage::SubmitDamageForm( pDamageForm );
				}
			}
		}

		//make an AI impact sound in case any AI are listening
		if( pLaser->m_pOwnerBot->m_nPossessionPlayerIndex > -1 && pLaser->m_fSecsUntilNextSound <= 0.0f)
		{
			AIEnviro_AddSound( pImpact->ImpactPoint, AIEnviro_fLaserImpactAudibleRange, 0.3f, AISOUNDTYPE_WEAPON, 0, pLaser );
			pLaser->m_fSecsUntilNextSound = 0.5f;
		}
	}
}



void CWeaponLaser::KillRumble( void ) {
	FASSERT( IsCreated() );

	fforce_Kill( &m_hForce );
}


void CWeaponLaser::SetItemInst( CItemInst *pItemInst, BOOL bUpdateItemInstAmmoFromWeaponAmmo ) {
	CWeapon::SetItemInst(pItemInst, bUpdateItemInstAmmoFromWeaponAmmo);
	if( pItemInst ) {
		pItemInst->SetAmmoDisplayType(CItemInst::AMMODISPLAY_METER, CItemInst::AMMODISPLAY_METER);
	}
}



void CWeaponLaser::ClassHierarchyAddToWorld( void ) {
	FASSERT( IsCreated() );
	FASSERT( !IsInWorld() );

	CWeapon::ClassHierarchyAddToWorld();

	m_pResourceData->m_pWorldMesh->m_Xfm.BuildFromMtx( m_MtxToWorld, m_fScaleToWorld );
	m_pResourceData->m_pWorldMesh->UpdateTracker();
}


void CWeaponLaser::ClassHierarchyRemoveFromWorld( void ) {
	FASSERT( IsCreated() );
	FASSERT( IsInWorld() );

	CWeapon::ClassHierarchyRemoveFromWorld();

	// Remove the weapon mesh from the world...
	m_pResourceData->m_pWorldMesh->RemoveFromWorld();
}


void CWeaponLaser::ClassHierarchyDrawEnable( BOOL bDrawingHasBeenEnabled ) {
	CWeapon::ClassHierarchyDrawEnable( bDrawingHasBeenEnabled );

//	if( IsInWorld() ) 
	{
		u32 i;

		if( bDrawingHasBeenEnabled ) {
			// Enable drawing of this weapon...

			for( i=0; i<EUK_COUNT_LASER; ++i ) {
				FMATH_CLEARBITMASK( m_aResourceData[i].m_pWorldMesh->m_nFlags, FMESHINST_FLAG_DONT_DRAW );
			}
		} else {
			// Disable drawing of this weapon...

			for( i=0; i<EUK_COUNT_LASER; ++i ) {
				FMATH_SETBITMASK( m_aResourceData[i].m_pWorldMesh->m_nFlags, FMESHINST_FLAG_DONT_DRAW );
			}
		}
	}
}


void CWeaponLaser::ClassHierarchyRelocated( void *pIdentifier ) {
	CWeapon::ClassHierarchyRelocated( pIdentifier );

	if( IsInWorld() ) {
		m_pResourceData->m_pWorldMesh->m_Xfm.BuildFromMtx( m_MtxToWorld, m_fScaleToWorld );
		m_pResourceData->m_pWorldMesh->UpdateTracker();
	}
}


void CWeaponLaser::AppendTrackerSkipList(u32& nTrackerSkipListCount, CFWorldTracker ** apTrackerSkipList) {
	// Add our weapon mesh...
	FASSERT( (nTrackerSkipListCount + 1) <= FWORLD_MAX_SKIPLIST_ENTRIES );
	apTrackerSkipList[nTrackerSkipListCount++] = m_pResourceData->m_pWorldMesh;
}

// Note that ClassHierarchyResetToState() is always called prior to this call.
void CWeaponLaser::ClassHierarchySetUpgradeLevel( u32 nPreviousUpgradeLevel ) {
	FASSERT( !IsTransitionState( CurrentState() ) );

	_ResourceData_t *pOldResourceData = &m_aResourceData[nPreviousUpgradeLevel];
	Info_t *pOldInfo = &m_aaInfo[WEAPON_TYPE_LASER][nPreviousUpgradeLevel];

	// Remove old mesh from world...
	pOldResourceData->m_pWorldMesh->RemoveFromWorld();

	// Set up new level...
	m_pResourceData = &m_aResourceData[ m_nUpgradeLevel ];

	if( IsInWorld() ) {
		m_pResourceData->m_pWorldMesh->m_Xfm = pOldResourceData->m_pWorldMesh->m_Xfm;
		m_pResourceData->m_pWorldMesh->UpdateTracker();
	}

	tracer_SetNewTexture( m_hTracerGroup, &m_aTracerTexInst[m_nUpgradeLevel] );
}



void CWeaponLaser::ComputeMuzzlePoint_WS( CFVec3A *pMuzzlePoint_WS ) const {
	FASSERT( IsCreated() );

	pMuzzlePoint_WS->Mul( m_MtxToWorld.m_vFront, m_aUserProps[m_nUpgradeLevel].fDistFromWeaponOrigToMuzzle ).Add( m_MtxToWorld.m_vPos );
}



// This function is called to let the weapon know what the trigger values are.
// It should be called whether or not the triggers are pressed because this function
// is responsible for firing and reloading of the weapon.
//
// pProjUnitDir_WS must point in the direction that the weapon will fire.
//
// Return value:
//   Bit 0: Trigger #1 fired
//   Bit 1: Trigger #2 fired
u32 CWeaponLaser::TriggerWork( f32 fUnitTriggerVal1, f32 fUnitTriggerVal2, const CFVec3A *pvTargetPos_WS, const CFVec3A *pBuddyFirePos_WS/* = NULL*/ ) {
	FASSERT( IsCreated() );

	m_bFireThisFrame = FALSE;

	if(fUnitTriggerVal1 < 0.25f)
	{
		// Trigger not down...
		m_bRechargeThisFrame = TRUE;
		return(0);
	}

	m_bRechargeThisFrame = FALSE;

	if(CurrentState() != STATE_DEPLOYED)
	{
		// Not in a state to fire...
		return(0);
	}

	u32 uLowerThreshold;
	if((m_nUpgradeLevel == 2) && (!m_bFiredLastFrame))
	{
		uLowerThreshold = 10;
	}
	else
	{
		uLowerThreshold = 0;
	}

	if(m_nClipAmmo <= uLowerThreshold)
	{
		// No rounds in clip...
		return(0);
	}

	if((m_nUpgradeLevel != 2) && (m_fSecondsCountdownTimer > 0.0f))
	{
		// Can't fire another round yet...
		return(0);
	}

	m_vTargetPos_WS.Set( *pvTargetPos_WS );
	if ( m_pOwnerBot && m_pOwnerBot->m_nPossessionPlayerIndex >= 0 )
	{
		m_pOwnerBot->FocusHumanTargetPoint_WS( &m_vTargetPos_WS, this );
	}

	// We passed all the tests, let's go ahead and fire off a round.
	m_bFireThisFrame = TRUE;
	m_pvecBuddyFirePos_WS = pBuddyFirePos_WS;

	return(1);
}


f32 CWeaponLaser::GetProjectileSpeed(void) const
{
	return m_aoTracerDef[m_nUpgradeLevel].fSpeed_WS;
}



static void _TrackPlayerLaser(void* pUserData, const CFVec3A& NewPos_WS, BOOL bDied)
{
	CWeaponLaser* pLaser = (CWeaponLaser*) pUserData;

	if (bDied)
	{
//		pLaser->m_uAISoundHandle = 0;
	}
	else
	{
		if( pLaser->m_uAISoundHandle &&
			!AIEnviro_ModifySound( pLaser->m_uAISoundHandle,  NewPos_WS, AIEnviro_fProjectileVisualRadius, 0.1f, AISOUNDTYPE_PROJECTILE, pLaser->GetOwner()) )
		{
			pLaser->m_uAISoundHandle = 0;
		}
	}

}
// Called once per frame to update animations, timers, etc.
void CWeaponLaser::ClassHierarchyWork( void ) {
	// Call our parent. It might need to do some work...
	CWeapon::ClassHierarchyWork();

	// Exit if we don't have any work to do...
	if( !IsOurWorkBitSet() ) {
		return;
	}

	static const f32 _fPerturbFactor = 0.01f;

	if(m_bFireThisFrame)
	{
		//////////////////////////////////////////////////////////////////////
		// Do the effects for the firing.
		BOOL bPlayForce = FALSE;
		CFVec3A vecPerturb;
		CFVec3A vFireUnitDir;
		CFVec3A vecMuzzlePoint;

		
		//vFireUnitDir.Sub( m_vTargetPos_WS, vecMuzzlePoint );
		//f32 fMag2 = vFireUnitDir.MagSq();
		//if( fMag2 < 0.001f ) {
		//	// too close
		//	vFireUnitDir = m_MtxToWorld.m_vFront;
		//}
		//
		//vFireUnitDir.Mul( fmath_InvSqrt( fMag2 ) );

		ComputeMuzzlePoint_WS(&vecMuzzlePoint);
		ComputeFireUnitDir( &vFireUnitDir, &vecMuzzlePoint, &m_MtxToWorld.m_vFront, &m_vTargetPos_WS, FALSE );

		vecPerturb.Set(fmath_RandomFloatRange(-_fPerturbFactor, _fPerturbFactor), fmath_RandomFloatRange(-_fPerturbFactor, _fPerturbFactor), fmath_RandomFloatRange(-_fPerturbFactor, _fPerturbFactor));
		vFireUnitDir.Add(vecPerturb);
		vFireUnitDir.Unitize();


		switch(m_nUpgradeLevel)
		{
			case 2:
			{
				if((m_nClipAmmo >= _uFizzleThreshhold) || (fmath_RandomChance(1.0f - (f32)(m_nClipAmmo) * (1.0f / (f32)(_uFizzleThreshhold)))))
				{
					m_apL3_LaserBeam[0]->SetStartPosAndDir(vecMuzzlePoint, vFireUnitDir, this, m_apDamageProfile[2]);

					if( IsDrawEnabled() ) {
						CMuzzleFlash::AddFlash_Card3D(m_ahMuzzleFlashGroup[MUZZLEFLASH_TYPE_LASER_RED], vecMuzzlePoint, m_aUserProps[2].fMuzzleWidth, m_aUserProps[2].fMuzzleHeight, 0.5f * (f32)(FLoop_nTotalLoopTicks) * FLoop_fSecsPerTick);
						CMuzzleFlash::AddFlash_Card3D(m_ahMuzzleFlashGroup[MUZZLEFLASH_TYPE_LASER_RED], vecMuzzlePoint, m_aUserProps[2].fMuzzleWidth, m_aUserProps[2].fMuzzleHeight, -0.4f * (f32)(FLoop_nTotalLoopTicks) * FLoop_fSecsPerTick);
					}

					if(m_pvecBuddyFirePos_WS != NULL)
					{
						m_apL3_LaserBeam[1]->SetStartPosAndDir(*m_pvecBuddyFirePos_WS, vFireUnitDir, this, m_apDamageProfile[2]);

						if( IsDrawEnabled() ) {
							CMuzzleFlash::AddFlash_Card3D(m_ahMuzzleFlashGroup[MUZZLEFLASH_TYPE_LASER_RED], *m_pvecBuddyFirePos_WS, m_aUserProps[2].fMuzzleWidth, m_aUserProps[2].fMuzzleHeight, 0.5f * (f32)(FLoop_nTotalLoopTicks) * FLoop_fSecsPerTick);
							CMuzzleFlash::AddFlash_Card3D(m_ahMuzzleFlashGroup[MUZZLEFLASH_TYPE_LASER_RED], *m_pvecBuddyFirePos_WS, m_aUserProps[2].fMuzzleWidth, m_aUserProps[2].fMuzzleHeight, -0.4f * (f32)(FLoop_nTotalLoopTicks) * FLoop_fSecsPerTick);
						}
					}

					if ( IsOwnedByPlayer() )
					{
						if ( !m_pSoundEmitter )
						{
							m_pSoundEmitter = FSNDFX_ALLOCNPLAY2D( m_aUserProps[m_nUpgradeLevel].hFiringSound, 1.0f, 1.0f, FAudio_EmitterDefaultPriorityLevel, 0.f, TRUE );
						}
					}
					else
					{
						if ( !m_pSoundEmitter )
						{
							m_pSoundEmitter = FSNDFX_ALLOCNPLAY3D( m_aUserProps[m_nUpgradeLevel].hFiringSound, &vecMuzzlePoint, m_aUserProps[m_nUpgradeLevel].fMaxLiveRange, 1.0f, 1.0f, FAudio_EmitterDefaultPriorityLevel, TRUE );
						}
					}

					if (GetOwner() && GetOwner()->m_nPossessionPlayerIndex > -1)
					{
						AIEnviro_BoostPlayerSoundTo(GetOwner()->m_nPossessionPlayerIndex, 30.0f);
					}

					bPlayForce = TRUE;
				}
				else if ( m_pSoundEmitter )
				{
					m_pSoundEmitter->Destroy();
					m_pSoundEmitter = NULL;
				}
				break;

			}
			case 0:
			case 1:
			case 3:
			default:
			{
				m_aoTracerDef[m_nUpgradeLevel].pUser = this;
				m_aoTracerDef[m_nUpgradeLevel].ColorRGBA.OpaqueWhite();
				m_aoTracerDef[m_nUpgradeLevel].UnitDir_WS = vFireUnitDir;
				m_aoTracerDef[m_nUpgradeLevel].TailPos_WS = vecMuzzlePoint;
//				m_aoTracerDef[m_nUpgradeLevel].ppCollSkipTrackers = &m_pOwnerBot->m_pWorldMesh;

				CFCamera *pCamera = fcamera_GetCameraByIndex(CPlayer::m_pCurrent->m_nPlayerIndex);

				if(m_nUpgradeLevel == 1)
				{
					if( IsDrawEnabled() ) {
						CMuzzleFlash::AddFlash_Card3D(m_ahMuzzleFlashGroup[MUZZLEFLASH_TYPE_LASER_YELLOW], vecMuzzlePoint, m_aUserProps[1].fMuzzleWidth, m_aUserProps[1].fMuzzleHeight, 0.5f * (f32)(FLoop_nTotalLoopTicks) * FLoop_fSecsPerTick);
					}
				}

				ConstructDamagerData();
				if (tracer_NewTracer(m_hTracerGroup, &m_aoTracerDef[m_nUpgradeLevel], 0.2f, FALSE, FALSE, &CDamageForm::m_TempDamager))
				{

					if (m_pOwnerBot->m_nPossessionPlayerIndex > -1)
					{
						if( !m_uAISoundHandle || !AIEnviro_SoundHandleToPtr(m_uAISoundHandle)) {
							m_uAISoundHandle = AIEnviro_AddSound( vecMuzzlePoint, AIEnviro_fProjectileVisualRadius, 0.1f, AISOUNDTYPE_PROJECTILE, AISOUNDCTRL_VISIBLE_ENTITY, GetOwner());
						}
						tracer_AddExtraParamsToLastTracer(_TrackPlayerLaser, (void*) this);
					}
				
				}
				if(m_pvecBuddyFirePos_WS != NULL)
				{
					m_aoTracerDef[m_nUpgradeLevel].UnitDir_WS = vFireUnitDir;
					m_aoTracerDef[m_nUpgradeLevel].TailPos_WS = *m_pvecBuddyFirePos_WS;

					ConstructDamagerData();
					tracer_NewTracer(m_hTracerGroup, &m_aoTracerDef[m_nUpgradeLevel], 0.2f, FALSE, FALSE, &CDamageForm::m_TempDamager);
				}

				if ( IsOwnedByPlayer() )
				{
					fsndfx_Play2D( m_aUserProps[m_nUpgradeLevel].hFiringSound,
									0.75f);	//fVolumeMultiplier  =
				}
				else
				{
					fsndfx_Play3D( m_aUserProps[m_nUpgradeLevel].hFiringSound,
									&vecMuzzlePoint,
									450.0f,
									-1.f,
									0.6f);//fVolumeMultiplier);
				}

				if (GetOwner() && GetOwner()->m_nPossessionPlayerIndex > -1)
				{
					AIEnviro_BoostPlayerSoundTo(GetOwner()->m_nPossessionPlayerIndex, 30.0f);
				}
	
				bPlayForce = TRUE;

				break;
			}
		}

		//
		//////////////////////////////////////////////////////////////////////

		//////////////////////////////////////////////////////////////////////
		// Force feedback effects.
		if(bPlayForce && IsOwnedByPlayer())
		{
			// Human player...
			fforce_Kill(&m_hForce);
			fforce_Play(Player_aPlayer[GetOwner()->m_nPossessionPlayerIndex].m_nControllerIndex, FFORCE_EFFECT_LIGHT_PULSE, &m_hForce);
		}
		//
		//////////////////////////////////////////////////////////////////////

		//////////////////////////////////////////////////////////////////////
		// Update timer variables and do any appropriate energy depletion.
		switch(m_nUpgradeLevel)
		{
			case 2:
			{
				// See if 
				m_L3_fDrainCntDn -= FLoop_fPreviousLoopSecs;
				while(m_L3_fDrainCntDn <= 0.0f)
				{
					// JUSTIN: Have this come from the CSV file.
					m_L3_fDrainCntDn += m_aUserProps[m_nUpgradeLevel].fOORoundsPerSec;
					RemoveFromClip(1);
				}
				break;
			}
			case 0:
			case 1:
			case 3:
			default:
			{
				// Set timer until the next round is ready...
				m_fSecondsCountdownTimer += m_aUserProps[m_nUpgradeLevel].fOORoundsPerSec;
				RemoveFromClip(1);
				break;
			}
		}
		//
		//////////////////////////////////////////////////////////////////////

		m_bFiredLastFrame = TRUE;
	}
	else
	{
		m_bFiredLastFrame = FALSE;

		if ( m_pSoundEmitter )
		{
			m_pSoundEmitter->Destroy();
			m_pSoundEmitter = NULL;
		}
	}

	if(m_bRechargeThisFrame)
	{
		// Replenish the energy.
		m_fRechargeCntDn -= FLoop_fPreviousLoopSecs;

		while(m_fRechargeCntDn <= 0.0f)
		{
			m_fRechargeCntDn += m_aUserProps[m_nUpgradeLevel].fOORechargeRate;			// JUSTIN: Get this from a CSV.
			SetClipAmmo(GetClipAmmo() + 1);
		}
	}

	if(m_fSecondsCountdownTimer > 0.0f)
	{
		m_fSecondsCountdownTimer -= FLoop_fPreviousLoopSecs;
	}

	// Do EUK-level-specific work...
	switch( m_nUpgradeLevel )
	{
	case 0:
		break;
	case 1:
		break;
	case 2:
		break;
	case 3:
		break;
	default:
		break;
	}
}


// Switches immediately to state CWeapon::CurrentState().
void CWeaponLaser::ClassHierarchyResetToState( void ) {
	CWeapon::ClassHierarchyResetToState();

	m_fSecondsCountdownTimer = 0.0f;
	m_L3_fDrainCntDn = 0.0f;
	m_fRechargeCntDn = 0.0f;
	m_bFiredLastFrame = FALSE;
}



void CWeaponLaser::NotifyAmmoMightHaveChanged( void ) {

}


