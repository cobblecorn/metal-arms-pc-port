//////////////////////////////////////////////////////////////////////////////////////
// DestructEntity.cpp - Destructable entity object.
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
// Date		Who			Description
// --------	----------	--------------------------------------------------------------
// 04/26/02	Link		Created.
//////////////////////////////////////////////////////////////////////////////////////

#include "DestructEntity.h"
#include "fresload.h"
#include "explosion.h"
#include "floop.h"
#include "FScriptSystem.h"
#include "fclib.h"
#include "FCheckPoint.h"
#include "debris.h"
#include <stdio.h>

// =============================================================================================================

//#define DISABLE_ALL_DESTRUCT

// Constants.

// =============================================================================================================

// Static members.
BOOL CDestructEntity::m_bSystemInitialized = FALSE;

static CEDestructBuilder _EDestructBuilder;
s32 CDestructEntity::m_nDestructEvent;

static cchar *_apszDefault[] = { "GF_1chunk01", "GF_1chunk02", "GF_1chunk01", "GF_1chunk02" };
static cchar *_apszCrystalBlue[] = { "GODMshrd2_1", "GODMshrd2_1", "GODMshrd2_1", "GODMshrd2_1", "GODMshrd2_1", "GODMshrd2_1" };
static cchar *_apszCrystalPink[] = { "GODMshrd3_1", "GODMshrd3_1", "GODMshrd3_1", "GODMshrd3_1", "GODMshrd3_1", "GODMshrd3_1" };
static cchar *_apszCrystalGreen[] = { "GODMshrd4_1", "GODMshrd4_1", "GODMshrd4_1", "GODMshrd4_1", "GODMshrd4_1", "GODMshrd4_1" };
static cchar *_apszCrystalTeal[] = { "GODMshrd5_1", "GODMshrd5_1", "GODMshrd5_1", "GODMshrd5_1", "GODMshrd5_1", "GODMshrd5_1" };

cchar **CDestructEntity::m_apszDebrisMeshNames[DEBRISTYPE_COUNT] =
{
	NULL, _apszDefault, _apszCrystalBlue, _apszCrystalPink, _apszCrystalGreen, _apszCrystalTeal
};


FMesh_t **CDestructEntity::m_papDebrisMeshes[DEBRISTYPE_COUNT];
const u32 CDestructEntity::m_auDebrisMeshCounts[DEBRISTYPE_COUNT] = { 0, 4, 6, 6, 6, 6 };

// =============================================================================================================

BOOL CDestructEntity::InitSystem()
{
#ifndef DISABLE_ALL_DESTRUCT
	FASSERT(!m_bSystemInitialized);

	u32 uDebrisIdx, uMeshIdx;
	for(uDebrisIdx = 0; uDebrisIdx < DEBRISTYPE_COUNT; ++uDebrisIdx)
	{
		m_papDebrisMeshes[uDebrisIdx] = (FMesh_t **)(fres_Alloc(sizeof(FMesh_t *) * m_auDebrisMeshCounts[uDebrisIdx]));
//		m_papDebrisMeshes[uDebrisIdx] = new (FMesh_t *)[2];
		for(uMeshIdx = 0; uMeshIdx < m_auDebrisMeshCounts[uDebrisIdx]; ++uMeshIdx)
		{
			m_papDebrisMeshes[uDebrisIdx][uMeshIdx] = (FMesh_t *)(fresload_Load(FMESH_RESTYPE, m_apszDebrisMeshNames[uDebrisIdx][uMeshIdx]));
			if(m_papDebrisMeshes[uDebrisIdx][uMeshIdx] == NULL)
			{
				DEVPRINTF("CDestructEntity::InitSystem() : Could not load mesh '%s' for debris.\n", m_apszDebrisMeshNames[uDebrisIdx][uMeshIdx]);
			}
		}
	}

	m_bSystemInitialized = TRUE;
#endif
	return(TRUE);
}

// =============================================================================================================

void CDestructEntity::UninitSystem()
{
#ifndef DISABLE_ALL_DESTRUCT
	if(m_bSystemInitialized)
	{
		m_bSystemInitialized = FALSE;
	}
#endif
}

// =============================================================================================================

BOOL CDestructEntity::InitLevel()
{
#ifndef DISABLE_ALL_DESTRUCT
	m_nDestructEvent = CFScriptSystem::GetEventNumFromName("destruct");
	if(m_nDestructEvent == -1)
	{
		DEVPRINTF("CDestructEntity::InitLevel() : Could not find 'destruct' event.\n");
	}
#endif
	return(TRUE);
}

// =============================================================================================================

void CDestructEntity::UninitLevel()
{
#ifndef DISABLE_ALL_DESTRUCT
#endif
}

// =============================================================================================================

CDestructEntity::CDestructEntity()
{
#ifndef DISABLE_ALL_DESTRUCT
	// Clear all pointers to NULL...
	Clear();
#endif
}

// =============================================================================================================

CDestructEntity::~CDestructEntity()
{
#ifndef DISABLE_ALL_DESTRUCT
	if(IsSystemInitialized() && IsCreated())
	{
		DetachFromParent();
		DetachAllChildren();
		RemoveFromWorld();
		ClassHierarchyDestroy();
	}
#endif
}

// =============================================================================================================

BOOL CDestructEntity::Create()
{
	FASSERT(m_bSystemInitialized);
	FASSERT(!IsCreated());

	// Get pointer to the leaf class's builder object...
	CEDestructBuilder *pBuilder = (CEDestructBuilder *)(GetLeafClassBuilder());

	// If we're the leaf class, set the builder object' defaults...
	if(pBuilder == &_EDestructBuilder)
	{
		pBuilder->SetDefaults(0, 0, "destruct");
	}
#ifndef DISABLE_ALL_DESTRUCT
	Clear();
/*
	/////////////////////////////////////////////////////////////
	// Set up the destructable object parameters.
	// TODO: This should be dealing with the builder object.
		const char *pszMeshNames[] = { "GODMcrst1_1", "GODMstmp1_1" };

		m_uNumMeshes = 2;
		m_fMeshSwitchInc = 1.0f / ((f32)(m_uNumMeshes) - 1.0f);
		m_fNextMeshSwitch = 1.0f - m_fMeshSwitchInc;

		m_bPulsate = TRUE;
		m_bLastMeshIsDeadMesh = TRUE;

		m_fPulstate = 1.0f;
	//
	/////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////
	// Set up the mesh entity.
	if(!CMeshEntity::Create(2, pszMeshNames, 0, NULL, "JustinCrystal", NULL))
	{
		return(FALSE);
	}
	//
	/////////////////////////////////////////////////////////////
*/
	SetCollisionFlag(TRUE);
	ClearMeshInstFlags(FMESHINST_FLAG_NOBONES);
	SetMaxHealth();

	// TEMP:
	Relocate_RotXlatFromUnitMtx_WS(&CFMtx43A::m_IdentityMtx);

	EnableAutoWork(TRUE);
#endif
	return(TRUE);
}

// =============================================================================================================

void CDestructEntity::ClassHierarchyDestroy()
{
#ifndef DISABLE_ALL_DESTRUCT
	// TODO: Fill this in.
	// fdelete stuff here.

	Clear();
#endif
	CMeshEntity::ClassHierarchyDestroy();
}

// =============================================================================================================

BOOL CDestructEntity::ClassHierarchyBuild()
{
#ifndef DISABLE_ALL_DESTRUCT
	FASSERT(m_bSystemInitialized);
	FASSERT(!IsCreated());
#endif
	FASSERT(FWorld_pWorld);

	// Get a frame.
	FResFrame_t hResFrame = fres_GetFrame();

	// Get pointer to the leaf class's builder object.
	CEDestructBuilder *pBuilder = (CEDestructBuilder *)(GetLeafClassBuilder());

	// Build our parent entity class.
	if(!CMeshEntity::ClassHierarchyBuild())
	{
		// Parent class could not be built.
		goto _ExitWithError;
	}

#ifndef DISABLE_ALL_DESTRUCT
	// Set any defaults in our class.
	_SetDefaults();

	/////////////////////////////////////////////////////////////
	// Initialize our entity from our builder object.
	m_eDEConfig.m_uTotMeshCnt = GetMeshCount();
	if(pBuilder->m_bLastMeshIsDeadMesh)
	{
		FASSERT( m_eDEConfig.m_uTotMeshCnt >= 1 );

		if( m_eDEConfig.m_uTotMeshCnt > 1 )
		{
			m_eDEConfig.m_uNormMeshCnt = m_eDEConfig.m_uTotMeshCnt - 1;
		}
		else
		{
			m_eDEConfig.m_uNormMeshCnt = 1;
		}
	}
	else
	{
		m_eDEConfig.m_uNormMeshCnt = m_eDEConfig.m_uTotMeshCnt;
	}

	if(m_eDEConfig.m_uNormMeshCnt == 1)
	{
		m_fMeshSwitchInc = 0.0f;
		m_fNextMeshSwitch = 0.0f;		// This will ensure that we never switch meshes.
	}
	else
	{
		m_fMeshSwitchInc = NormHealth() / ((f32)(m_eDEConfig.m_uNormMeshCnt));
		m_fNextMeshSwitch = NormHealth() - m_fMeshSwitchInc;
	}


	m_eDEConfig.m_uDamageFlags = pBuilder->m_uDamageFlags;
	m_eDEConfig.m_uDieFlags = pBuilder->m_uDieFlags;

	m_eDieDebris = pBuilder->m_eDieDebris;
	m_fDieDebrisVel = pBuilder->m_fDieDebrisVel;

	if((pBuilder->m_pszDestructBank != NULL) && (pBuilder->m_pszDestructWave != NULL))
	{
		cchar *apszBanks[2];
		apszBanks[0] = pBuilder->m_pszDestructBank;
		apszBanks[1] = NULL;

		FAudio_BankHandle_t hBank;

		faudio_LoadBanks(apszBanks, &hBank);
		FAudio_WaveHandle_t	hWave = faudio_GetWaveHandle(hBank, pBuilder->m_pszDestructWave);

		//////////////////////////////////////////////////////////////////////
		// Create and set up the CFAudioEmitter for when it gets destroyed.
		f32 fCurRadius = (pBuilder->m_fDestructRadius == -1.0f) ? 100.0f : pBuilder->m_fDestructRadius;

		m_pAEDestruct = CFAudioEmitter::Create3D(hWave, FAudio_EmitterDefaultPriorityLevel, &MtxToWorld()->m_vPos, fCurRadius);

		m_fDestructVolume = (pBuilder->m_fDestructVolume == -1.0f) ? 1.0f : pBuilder->m_fDestructVolume;
		//
		//////////////////////////////////////////////////////////////////////
	}
	//
	/////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////
	// Disable any animation from playing.
	UserAnim_Select(-1);
	//
	/////////////////////////////////////////////////////////////
#endif
	return(TRUE);

_ExitWithError:
	Destroy();
	fres_ReleaseFrame(hResFrame);
	return(FALSE);
}

// =============================================================================================================

BOOL CDestructEntity::ClassHierarchyBuilt( void ) {
#ifndef DISABLE_ALL_DESTRUCT
	FASSERT( IsSystemInitialized() );
	FASSERT( IsCreated() );
#endif
	FResFrame_t ResFrame = fres_GetFrame();

	if( !CMeshEntity::ClassHierarchyBuilt() ) {
		goto _ExitWithError;
	}
#ifndef DISABLE_ALL_DESTRUCT
	EnableOurWorkBit();
//	SetTargetable(TRUE);
#endif
	return TRUE;

_ExitWithError:
	Destroy();
	fres_ReleaseFrame( ResFrame );
	return FALSE;
}

// =============================================================================================================

CEntityBuilder *CDestructEntity::GetLeafClassBuilder()
{
	return(&_EDestructBuilder);
}

// =============================================================================================================

void CDestructEntity::ClassHierarchyResolveEntityPointerFixups()
{
	CMeshEntity::ClassHierarchyResolveEntityPointerFixups();
#ifndef DISABLE_ALL_DESTRUCT
	FASSERT(IsCreated());
#endif
}

// =============================================================================================================

void CDestructEntity::_StartDie()
{
#ifndef DISABLE_ALL_DESTRUCT
	SelectMesh(m_eDEConfig.m_uTotMeshCnt - 1);

	// The state has to be changed up here or else a cluster of crystals could result in infinitely
	//   recursive _StartDie() calls.
	m_fDyingTimer = 0.5f;
	m_eDEState.m_eState = STATE_DYING;
	m_fNextExplode = 0.0f;

	CFMeshInst *pMI = GetMeshInst();
	CFVec3A vecBoneOrigin, vecSpawnPos;
	CFVec3A vecDir;

	if(UserAnim_GetCount() != 0)
	{
		UserAnim_Select(0);
		UserAnim_SetClampMode(TRUE);
		UserAnim_SetSpeedMult(1.0f);
		UserAnim_Pause(FALSE);
	}

	if(m_pAEDestruct != NULL)
	{
		m_pAEDestruct->SetVolume(m_fDestructVolume);
		m_pAEDestruct->Play(1);
	}

	// Look for debris bones.
	u32 uDebrisBone;
	s32 nBoneIdx;
	char szBoneName[9] = "debrisxx";
	FMeshBone_t *pBoneMtxArray = pMI->m_pMesh->pBoneArray;

	for(uDebrisBone = 1; uDebrisBone < 100; ++uDebrisBone)
	{
		sprintf(szBoneName, "debris%02d", uDebrisBone);
		nBoneIdx = pMI->FindBone(szBoneName);
		if(nBoneIdx == -1)
		{
			break;
		}

		vecBoneOrigin = pBoneMtxArray[nBoneIdx].AtRestBoneToParentMtx.m_vPos;
		vecDir = pBoneMtxArray[nBoneIdx].AtRestBoneToParentMtx.m_vY;
		m_MtxToWorld.MulPoint(vecBoneOrigin);
		m_MtxToWorld.MulDir(vecDir);
		vecSpawnPos = vecBoneOrigin;
		vecSpawnPos.Add(vecDir);
		vecSpawnPos.Add(vecDir);
		vecSpawnPos.Add(vecDir);
		vecSpawnPos.Add(vecDir);
		vecSpawnPos.Add(vecDir);
		vecSpawnPos.Add(vecDir);

#if 0
		if(m_eDEConfig.m_uDieFlags & DIEFLAG_EXPLODE)
		{
			explosion_Spawn(EXPLOSION_TYPE_GRENADE, vecSpawnPos, 0.5f, &vecDir, TRUE, 20.0f);
			CDamageInfo oDamageInfo;
			oDamageInfo.m_eDamagerClass = DAMAGERCLASS_EXPLOSION_GENERIC;
			oDamageInfo.m_fUnitBulletDamage = 0.0f;
			oDamageInfo.m_fUnitExplosionDamage = 0.08f;
			oDamageInfo.m_fUnitFlameDamage = 0.0f;
			oDamageInfo.m_nBoneIdx = -1;
			oDamageInfo.m_pEDamagee = NULL;
			oDamageInfo.m_pEDamager = this;
			oDamageInfo.m_pEDamagerOwner = NULL;
			vecDir.Mul(m_fDieDebrisVel);
			oDamageInfo.m_vecDir_WS = vecDir;
//			oDamageInfo.m_vecNormal_WS.Set(0.0f, 0.0f, 0.0f);
//			oDamageInfo.m_vecPos_WS
			CDamageSystem::SubmitDamageRequest(&oDamageInfo, vecSpawnPos, 25.0f);
//			explosion_Spawn2(EXPLOSION_TYPE_GRENADE, vecSpawnPos.v3, 0.5f, &vecDir.v3, TRUE, EXPLOSION_DEBRISTYPE_SHARDS);
		}
#endif

		if(m_eDieDebris != DEBRISTYPE_NONE)
		{
			vecDir.Mul(2.0f);
			CDebris::CreateDebris(&vecSpawnPos, &vecDir, m_papDebrisMeshes[m_eDieDebris], m_auDebrisMeshCounts[m_eDieDebris], 2.0f);
		}
//		CDebris::CreateDebris(&vecSpawnPos, &vecDir, m_papDebrisMeshes[m_eDieDebris], m_auDebrisMeshCounts[m_eDieDebris], 1.5f);
//		CDebris::CreateDebris(&vecSpawnPos, &vecDir, m_papDebrisMeshes[m_eDieDebris], m_auDebrisMeshCounts[m_eDieDebris], 1.0f);
	}

	if(m_eDEConfig.m_uDieFlags & DIEFLAG_DISAPPEAR)
	{
		CMeshEntity::Die();
		RemoveFromWorld();
	}

	if(m_nDestructEvent != -1)
	{
		CFScriptSystem::TriggerEvent(m_nDestructEvent, 0, (u32)(this), 0);
	}
#endif
}

// =============================================================================================================

BOOL CDestructEntity::StartDestruction( void )
{
	if( m_eDEState.m_eState == STATE_ALIVE )
	{
		SetNormHealth( 0.0f );
		_StartDie();
		return TRUE;
	}

	return FALSE;
}

// =============================================================================================================

void CDestructEntity::InflictDamage(CDamageData *pDamageData)
{
	switch(m_eDEState.m_eState)
	{
		case STATE_ALIVE:
		{
			CMeshEntity::InflictDamage(pDamageData);

			f32 fCurHealth = NormHealth();
			if(fCurHealth <= 0.0f)
			{
				_StartDie();
				return;
			}

			//////////////////////////////////////////////////////////////////////
			// Check to see if we have crossed a mesh threshold.
			if(fCurHealth <= m_fNextMeshSwitch)
			{
				SelectMesh(GetCurrentMeshIndex() + 1);
				m_fNextMeshSwitch -= m_fMeshSwitchInc;
			}
			//
			//////////////////////////////////////////////////////////////////////

			//////////////////////////////////////////////////////////////////////
			//

			//
			//////////////////////////////////////////////////////////////////////

			//////////////////////////////////////////////////////////////////////
			// Check to see if we have crossed any other types of thresholds.
			// TODO: Add anything new here.
			//
			//////////////////////////////////////////////////////////////////////
			return;
		}
		case STATE_DYING:
		case STATE_DEAD:
		{
			CMeshEntity::InflictDamage(pDamageData);
			return;
		}
		default:
		{
			FASSERT_NOW;
			return;
		}
	}
}

// =============================================================================================================

void CDestructEntity::ClassHierarchyWork()
{
#ifndef DISABLE_ALL_DESTRUCT
	CMeshEntity::ClassHierarchyWork();

	if( !IsOurWorkBitSet() ) {
		return;
	}

	StateTransitions();
	StateWork();
#endif
}

// =============================================================================================================

void CDestructEntity::Clear()
{
}

// =============================================================================================================

void CDestructEntity::_SetDefaults()
{
#ifndef DISABLE_ALL_DESTRUCT
	m_pAEDestruct = NULL;
	m_eDieDebris = DEBRISTYPE_NONE;
	m_fDieDebrisVel = 20.0f;
	m_eDEState.m_eState = STATE_ALIVE;
	m_eDEConfig.m_fOOShakeDur = 1.f;
	m_eDEConfig.m_fShakeDur = 1.f;
	m_eDEConfig.m_fShakeIntens = 0.f;
	m_eDEConfig.m_uDamageFlags = 0;
	m_eDEConfig.m_uDieFlags = 0;
	m_eDEConfig.m_uNormMeshCnt = 0;
	m_eDEConfig.m_uTotMeshCnt = 0;
#endif
}

// =============================================================================================================

void CDestructEntity::StateTransitions()
{
#ifndef DISABLE_ALL_DESTRUCT
	switch(m_eDEState.m_eState)
	{
		case STATE_ALIVE:
		{
//			if(m_fShakeCtdown >= 0.0f)
			if(m_eDEState.m_fShakeCtdown >= 0.0f)
			{
//				m_fShakeCtdown -= FLoop_fPreviousLoopSecs;
				m_eDEState.m_fShakeCtdown -= FLoop_fPreviousLoopSecs;
			}
			break;
		}
		case STATE_DYING:
		{
			m_fDyingTimer -= FLoop_fPreviousLoopSecs;
			if(m_fDyingTimer <= 0.0f)
			{
				m_eDEState.m_eState = STATE_DEAD;
			}
			break;
		}
		case STATE_DEAD:
		{
			break;
		}
		default:
		{
		}
	}
#endif
}

// =============================================================================================================

void CDestructEntity::StateWork()
{
#ifndef DISABLE_ALL_DESTRUCT
	switch(m_eDEState.m_eState)
	{
		case STATE_ALIVE:
		{
//			if(m_fShakeCtdown >= 0.0f)
			if(m_eDEState.m_fShakeCtdown >= 0.0f)
			{
//				f32 fIntensity = m_fShakeCtdown * m_fOOShakeDur;

				f32 fIntensity = m_eDEState.m_fShakeCtdown * m_eDEConfig.m_fOOShakeDur;
				CFVec3A vecShakeOfs;
				fmath_SinCos(fmath_RandomFloat() * FMATH_2PI, &(vecShakeOfs.x), &(vecShakeOfs.z));
				vecShakeOfs.y = 0.f;
				vecShakeOfs.Mul(fIntensity * m_eDEConfig.m_fShakeIntens);

				CFMtx43A mtxNew = m_MtxToWorld;
				mtxNew.m_vPos.Add(vecShakeOfs);

				Relocate_RotXlatFromUnitMtx_WS(&mtxNew);
			}
			break;
		}
		case STATE_DYING:
		{
			m_fNextExplode -= FLoop_fPreviousLoopSecs;
			if(m_fNextExplode <= 0.0f)
			{
				CFMeshInst *pMI = GetMeshInst();
				CFVec3A vecCenter, vecPoint, vecDir;
				vecCenter = pMI->m_Xfm.m_MtxF.m_vPos;
	
				//////////////////////////////////////////////////////////////////////
				// Get a point from which to spawn an explosion.
				vecPoint = vecCenter;
				vecPoint.x += fmath_RandomBipolarUnitFloat() * 4.0f;
				vecPoint.y += fmath_RandomFloat() * 3.5f + 0.5f;
				vecPoint.z += fmath_RandomBipolarUnitFloat() * 4.0f;
				//
				//////////////////////////////////////////////////////////////////////

				//////////////////////////////////////////////////////////////////////
				// Get a direction in which to spawn an explosion.
				vecDir.Set(0.0f, 1.0f, 0.0f);
				vecDir.x += fmath_RandomBipolarUnitFloat() * 0.3f;
				vecDir.z += fmath_RandomBipolarUnitFloat() * 0.3f;
				vecDir.Unitize();
				//
				//////////////////////////////////////////////////////////////////////

//				explosion_Spawn2(EXPLOSION_TYPE_ROCKET, vecPoint.v3, 0.5f/*0.4f*/, &(vecDir.v3)/*NULL*/, TRUE, EXPLOSION_DEBRISTYPE_SHARDS);

				m_fNextExplode = fmath_RandomFloat() * 0.2f + 0.07f;
			}

			break;
		}
		case STATE_DEAD:
		{
			break;
		}
		default:
		{
		}
	}
#endif
}

// saves state for checkpoint
BOOL CDestructEntity::CheckpointSave( void )
{
	// save parent class data
	CMeshEntity::CheckpointSave();

	// save destruct class data.
	// order must match load order below
	CFCheckPoint::SaveData( m_fDyingTimer );
	CFCheckPoint::SaveData( (u32&) m_eDEState.m_eState );
	CFCheckPoint::SaveData( m_fNextExplode );

	return TRUE;
}

// loads state for checkpoint
void CDestructEntity::CheckpointRestore( void )
{
	// load parent class data
	CMeshEntity::CheckpointRestore();

	// load destruct class data.
	// order must match save order above
	CFCheckPoint::LoadData( m_fDyingTimer );
	CFCheckPoint::LoadData( (u32&) m_eDEState.m_eState );
	CFCheckPoint::LoadData( m_fNextExplode );
}


// =============================================================================================================
// =============================================================================================================
// =============================================================================================================

void CEDestructBuilder::SetDefaults(u64 nEntityTypeBits, u64 nEntityLeafTypeBit, cchar *pszEntityType)
{
	// TODO: Should I be passing the entity type like this?
	ENTITY_BUILDER_SET_PARENT_CLASS_DEFAULTS(CMeshEntityBuilder, ENTITY_BIT_DESTRUCT, pszEntityType);

	// Clear out the CEDestructBuilder object.
	// JUSTIN: Maybe this shouldn't be the default.
	m_bLastMeshIsDeadMesh = TRUE;

	m_uDamageFlags = 0;
	m_uDieFlags = CDestructEntity::DIEFLAG_EXPLODE;

	m_eDieDebris = CDestructEntity::DEBRISTYPE_NONE;
	m_fDieDebrisVel = 20.0f;

	m_pszDestructBank = NULL;
	m_pszDestructWave = NULL;
	m_fDestructRadius = 100.0f;
	m_fDestructVolume = 1.0f;

	m_pszEC_ArmorProfile = "DefaultDestruct";
}

// =============================================================================================================

BOOL CEDestructBuilder::InterpretTable()
{
	FGameDataTableHandle_t hCurTable = CEntityParser::m_hTable;

	if(!fclib_stricmp(CEntityParser::m_pszTableName, "sound_destruct_bank"))
	{
		CEntityParser::Interpret_String(&m_pszDestructBank);
		return TRUE;
	}
	else if(!fclib_stricmp(CEntityParser::m_pszTableName, "sound_destruct_wave"))
	{
		CEntityParser::Interpret_String(&m_pszDestructWave);
		return TRUE;
	}
	else if(!fclib_stricmp(CEntityParser::m_pszTableName, "sound_destruct_volume"))
	{
		CEntityParser::Interpret_F32(&m_fDestructVolume, 0.0f, 1.0f, TRUE);
		return TRUE;
	}
	else if(!fclib_stricmp(CEntityParser::m_pszTableName, "sound_destruct_radius"))
	{
		CEntityParser::Interpret_F32(&m_fDestructRadius);
		return TRUE;
	}
	else if(!fclib_stricmp(CEntityParser::m_pszTableName, "disappear"))
	{
		CEntityParser::Interpret_Flag(&m_uDieFlags, CDestructEntity::DIEFLAG_DISAPPEAR);
		return TRUE;
	}
	else if(!fclib_stricmp(CEntityParser::m_pszTableName, "dieexplode"))
	{
		CEntityParser::Interpret_Flag(&m_uDieFlags, CDestructEntity::DIEFLAG_EXPLODE);
		return TRUE;
	}
	else if(!fclib_stricmp(CEntityParser::m_pszTableName, "diedebris"))
	{
		cchar *pszValue;
		CEntityParser::Interpret_String(&pszValue);
		if(!fclib_stricmp(pszValue, "none"))
		{
			m_eDieDebris = CDestructEntity::DEBRISTYPE_NONE;
		}
		else if(!fclib_stricmp(pszValue, "crystalblue"))
		{
			m_eDieDebris = CDestructEntity::DEBRISTYPE_CRYSTALBLUE;
		}
		else if(!fclib_stricmp(pszValue, "crystalpink"))
		{
			m_eDieDebris = CDestructEntity::DEBRISTYPE_CRYSTALPINK;
		}
		else if(!fclib_stricmp(pszValue, "crystalgreen"))
		{
			m_eDieDebris = CDestructEntity::DEBRISTYPE_CRYSTALGREEN;
		}
		else if(!fclib_stricmp(pszValue, "crystalteal"))
		{
			m_eDieDebris = CDestructEntity::DEBRISTYPE_CRYSTALTEAL;
		}
		else if(!fclib_stricmp(pszValue, "default"))
		{
			m_eDieDebris = CDestructEntity::DEBRISTYPE_DEFAULT;
		}
		else
		{
			DEVPRINTF("CDestructEntity::InterpretTable() : Unrecognized value '%s' for 'diedebris' table.\n", pszValue);
			return(TRUE);
		}
		return(TRUE);
	}
	else if(!fclib_stricmp(CEntityParser::m_pszTableName, "diedebrisvel"))
	{
		CEntityParser::Interpret_F32(&m_fDieDebrisVel);
		return TRUE;
	}


	return(CMeshEntityBuilder::InterpretTable());
}

// =============================================================================================================

BOOL CEDestructBuilder::PostInterpretFixup()
{
	if(!CMeshEntityBuilder::PostInterpretFixup())
	{
		goto _ExitWithError;
	}

	// TODO: I'm not exactly sure what this should be doing.
	return(TRUE);

_ExitWithError:
	return(FALSE);
}

// =============================================================================================================

// =============================================================================================================
