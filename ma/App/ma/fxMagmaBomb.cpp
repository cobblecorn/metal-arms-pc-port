 //////////////////////////////////////////////////////////////////////////////////////
// FXMagmaBomb.cpp - Draw calls for ye ole magmabomb
//
// Author: Michael Scholz
//////////////////////////////////////////////////////////////////////////////////////
// THIS CODE IS PROPRIETARY PROPERTY OF SWINGIN' APE STUDIOS, INC.

// Copyright (c) 2003
//
// The contents of this file may not be disclosed to third
// parties, copied or duplicated in any form, in whole or in part,
// without the prior written permission of Swingin' Ape Studios, Inc.
//////////////////////////////////////////////////////////////////////////////////////
// Modification History:
//
// Date     Who         Description
// -------- ----------  --------------------------------------------------------------
// 01.08.03 Scholz		Created.
//////////////////////////////////////////////////////////////////////////////////////


#include "fxmagmabomb.h"
#include "fresload.h"
#include "fdraw.h"
#include "frenderer.h"
#include "fcoll.h"
#include "fworld_coll.h"
#include "floop.h"
#include "player.h"
#include "fvtxpool.h"
#include "damage.h"
#include "Bot.h"
#include "EBotFire.h"
#include "MeshTypes.h"
#include "Ai\aienviro.h"
#include "fsound.h"

BOOL		 CFXMagmaBomb::m_bSystemInitialized	= FALSE;				// TRUE if the system has been initialized
BOOL		 CFXMagmaBomb::m_bSystemActive		= FALSE;

static CFXMagmaBomb* s_aMagmaBombs;

cchar* CFXMagmaBomb::m_apszTexNames[_MAGMABOMB_NUM_TEXTURES] =
{
	"TRDSmagma01",
};

cchar* CFXMagmaBomb::m_apszPartNames[_MAGMABOMB_NUM_PARTICLE_FX] =
{
	"PWDMwpnS801",
	"PWDMwpnS802",
	"PWDMwpnS803",
};

CFTexInst	CFXMagmaBomb::m_aTextures[_MAGMABOMB_NUM_TEXTURES];
FParticle_DefHandle_t	CFXMagmaBomb::m_ahParticleDefs[_MAGMABOMB_NUM_PARTICLE_FX] = {FPARTICLE_INVALID_HANDLE};


static BOOL _DamageNotify(CDamageForm* pTheDamage,CEntity* pToWho, void* pUser)
{
	for(CEntity* pChildEntity=pToWho->GetFirstChild(); pChildEntity; pChildEntity=pToWho->GetNextChild(pChildEntity) )
	{
		if( pChildEntity->TypeBits() & ENTITY_BIT_PARTICLE ) // it's a particle, likely an attached fire fx
		{
			return TRUE;
		}
	}
	
	if (pToWho->TypeBits() & ENTITY_BIT_BOT)
	{
		CBot* pBot = (CBot*)pToWho;

		CEBotFire* pBotFire = CEBotFire::GetFreeEBotFireFromPool();
		if (!pBotFire)
		{
			DEVPRINTF("EBotFire Pool ran out of flames");
			return TRUE;
		}
		
		FCollImpact_t CollImpact;
		CFVec3A vTestStartPt,vTestEndPt;
		vTestStartPt.Set(pTheDamage->m_Epicenter_WS);
		vTestEndPt.Set(*pBot->GetTagPoint(0));
		
		cchar* pszStickyParentBoneName = NULL;
		
		CFVec3A vFront  = CFVec3A::m_UnitAxisZ;
		CFVec3A vNormal = CFVec3A::m_UnitAxisY;
		CFVec3A vFirePos = vTestEndPt;

		if	( fworld_FindClosestImpactPointToRayStart( &CollImpact, &vTestStartPt, &vTestEndPt) ) 
		{
			CEntity* pEntity = CGColl::ExtractEntity( &CollImpact );
			if( pEntity ) 
			{
				CFWorldMesh *pWorldMesh = (CFWorldMesh *)CollImpact.pTag;

				if (CollImpact.nBoneIndex != 0)
				{
					pszStickyParentBoneName = pWorldMesh->m_pMesh->pBoneArray[CollImpact.nBoneIndex].szName;
					s32 nBoneFound = pWorldMesh->FindBone(pszStickyParentBoneName);
					FASSERT(nBoneFound > 0);
					vFirePos.Set(pWorldMesh->GetBoneMtxPalette()[nBoneFound]->m_vPos);
				}
				vNormal.Set(CollImpact.UnitFaceNormal);
				vFront.Sub(CollImpact.aTriVtx[1],CollImpact.aTriVtx[0]);
				vFront.Unitize();
			}
		}

		CFMtx43A mtxAttach;
		mtxAttach.m_vFront.Set(vNormal);
		mtxAttach.m_vPos.Set(vFirePos);
		mtxAttach.m_vUp.Set(vFront);
		mtxAttach.m_vRight.Cross(mtxAttach.m_vUp,mtxAttach.m_vFront);
		
		pBotFire->AddToWorld();
		pBotFire->Relocate_RotXlatFromUnitMtx_WS(&mtxAttach);
		pBotFire->Attach_ToParent_WS(pBot,pszStickyParentBoneName);
		MagmaBombProps_t* pProps =  (MagmaBombProps_t*)pUser;
		pBotFire->Start(pProps->pCatchFireDamageProfile,pTheDamage->m_Damager.pWeapon,pTheDamage->m_Damager.pBot,fmath_RandomFloatRange(pProps->fMinAttachedFlameDuration,pProps->fMaxAttachedFlameDuration));
	}
	
	return TRUE;
}

BOOL CFXMagmaBomb::_InitTextures( void )
{
	FTexDef_t *pTexDef;

	for( u32 i=0; i<_MAGMABOMB_NUM_TEXTURES; i++ )
	{
		pTexDef = (FTexDef_t *)fresload_Load( FTEX_RESNAME, m_apszTexNames[i] );
		if( pTexDef == NULL )
		{
			DEVPRINTF( "CFXMagmaBomb::_InitTextures():  Error loading texture:  %s\n", m_apszTexNames[i] );
			return FALSE;
		}
		m_aTextures[i].SetTexDef( pTexDef );
		m_aTextures[i].SetFlags( CFTexInst::FLAG_WRAP_S | CFTexInst::FLAG_WRAP_T );
	}
	return TRUE;
}

BOOL CFXMagmaBomb::_InitParticles( void )
{
	BOOL bDidAll = TRUE;
	for( u32 i=0; i<_MAGMABOMB_NUM_PARTICLE_FX; i++ )
	{
		// Particle effects
		m_ahParticleDefs[i] = (FParticle_DefHandle_t) fresload_Load( FPARTICLE_RESTYPE, m_apszPartNames[i] );
		if( m_ahParticleDefs[i] == FPARTICLE_INVALID_HANDLE ) 
		{
			DEVPRINTF( "CBotSlosh::ClassHierarchyBuild(): Could not find particle definition '%s'.\n", m_apszPartNames[i]);
			bDidAll = FALSE;
		}
	}
	return bDidAll;
}

BOOL CFXMagmaBomb::_InitSystem( void )
{
	for (s32 nIndex= 0;nIndex < _NUM_FLAMES ; nIndex++)
	{
		m_aFlames[nIndex].Create(m_ahParticleDefs[2],TRUE);
	}
	
	return TRUE;
}

BOOL CFXMagmaBomb::InitSystem( void )
{
	FASSERT( !m_bSystemInitialized );

	FResFrame_t Frame;

	Frame = fres_GetFrame();
	
	s_aMagmaBombs = fnew CFXMagmaBomb[_MAX_NUM_MAGMAEFFECTS];
	if(s_aMagmaBombs == NULL )
	{
		DEVPRINTF( "CFXMagmaBomb::InitSystem(): Could not allocate CFXMagmaBomb array\n" );
		goto _ExitInitSystemWithError;
	}
	
	//initialize the textures
	if( !_InitTextures() )
	{
		DEVPRINTF( "CFXMagmaBomb::InitSystem(): Could not initialize textures\n" );
		goto _ExitInitSystemWithError;
	}
	
	//initialize the particles
	if( !_InitParticles() )
	{
		DEVPRINTF( "CFXMagmaBomb::InitSystem(): Could not initialize particles\n" );
//		goto _ExitInitSystemWithError;
	}

	for( u32 i=0; i<_MAX_NUM_MAGMAEFFECTS; i++ )
	{
		if (!s_aMagmaBombs[i]._InitSystem())
		{
			goto _ExitInitSystemWithError;
		}
	}


	m_bSystemInitialized = TRUE;
	return TRUE;

	// Error:
_ExitInitSystemWithError:
	UninitSystem();
	fres_ReleaseFrame( Frame );
	return FALSE;
}

void CFXMagmaBomb::UninitSystem( void )
{
	fdelete_array(s_aMagmaBombs);
	s_aMagmaBombs = NULL;

	if( !m_bSystemInitialized )
	{
		return;
	}
	m_bSystemInitialized = FALSE;
}

void CFXMagmaBomb::Work( void )
{
	if( m_bSystemActive )
	{
		m_bSystemActive = FALSE;

		for( u32 i=0; i<_MAX_NUM_MAGMAEFFECTS; i++ )
		{
			if( s_aMagmaBombs[i].m_bActive )
			{
				m_bSystemActive = TRUE;
				s_aMagmaBombs[i]._Work();
			}
		}
	}
}

void CFXMagmaBomb::_Init(const FCollImpact_t* pImpact)
{
	CFVec3  vFront;
	CFVec3  vUp;
	CFVec3A vTemp;
	
	vUp = pImpact->UnitFaceNormal.v3;
	vFront.Sub(pImpact->aTriVtx[1].v3,pImpact->aTriVtx[0].v3);
	vFront.Unitize();
	m_vImpactPos = pImpact->ImpactPoint;

	f32 fRadRotate = fmath_Div(FMATH_2PI,_NUM_FLAMES);
	for (s32 nIndex= 0;nIndex < _NUM_FLAMES ; nIndex++)
	{
		if (m_aFlames[nIndex].m_FireParticle.IsCreated()==FALSE)
			m_aFlames[nIndex].m_FireParticle.Create();

		CFQuatA qTemp;
		qTemp.BuildQuat(pImpact->UnitFaceNormal,fRadRotate*nIndex);
		qTemp.MulPoint(vTemp,vFront);
		
		m_aFlames[nIndex].m_pBombProps = m_pBombProps;
		m_aFlames[nIndex].m_fVelocity = fmath_RandomFloatRange(m_pBombProps->fMinSpreadSpeed,m_pBombProps->fMaxSpreadSpeed);
		m_aFlames[nIndex].m_fDuration = fmath_RandomFloatRange(m_pBombProps->fMinBombDuration,m_pBombProps->fMaxBombDuration);
		m_aFlames[nIndex].m_fOODuration = 1.0f/m_aFlames[nIndex].m_fDuration;

		m_aFlames[nIndex].Init(m_pSpawnerBot,m_pSpawnerWeapon,pImpact->ImpactPoint,vUp,vTemp);
	}

	for (s32 nIndex= 0;nIndex < _NUM_SMOKES; nIndex++)
	{
		CFMtx43A mtxSmoke;
		mtxSmoke.m_vPos = pImpact->ImpactPoint;
		mtxSmoke.m_vUp = vFront;
		mtxSmoke.m_vFront = pImpact->UnitFaceNormal;
		mtxSmoke.m_vRight.Cross(mtxSmoke.m_vFront,mtxSmoke.m_vUp);
		if (m_aSmokeEmitters[nIndex].IsCreated()==FALSE)
			m_aSmokeEmitters[nIndex].Create();

		m_aSmokeEmitters[nIndex].AddToWorld();
		m_aSmokeEmitters[nIndex].SetUnitIntensity( 0.0 );
		m_aSmokeEmitters[nIndex].Relocate_RotXlatFromUnitMtx_WS( &mtxSmoke );
		m_aSmokeEmitters[nIndex].StartEmission( m_ahParticleDefs[1]);
		m_aSmokeEmitters[nIndex].EnableEmission( TRUE );
		m_aSmokeEmitters[nIndex].SetAmbientPropertiesFromSoundGroup( m_pBombProps->pSoundGroupOnFire, TRUE );
		
		CEntity* pEntity = CGColl::ExtractEntity( pImpact );
		if( pEntity ) 
		{
			// It's an Entity!
			CEntity *pHitEntity = (CEntity *)pEntity;
			m_aSmokeEmitters[nIndex].Attach_ToParent_WS(pHitEntity);
		}
	}
	m_uAISoundHandle = 0;
}

CFXMagmaBomb* CFXMagmaBomb::SpawnBombFX(CBot* pBot,CWeapon* pWeapon,const FCollImpact_t *pImpact,MagmaBombProps_t& MagmaProps)
{
	m_bSystemActive = TRUE;
	for( u32 i=0; i<_MAX_NUM_MAGMAEFFECTS; i++ ) 
	{
		if (!s_aMagmaBombs[i].m_bActive)
		{
			CFXMagmaBomb* pBomb = &s_aMagmaBombs[i];
			pBomb->m_pSpawnerBot    = pBot;
			pBomb->m_pSpawnerWeapon = pWeapon;
			pBomb->m_fDuration = MagmaProps.fMaxBombDuration;
			pBomb->m_fOODuration = 1.0f/pBomb->m_fDuration;
			pBomb->m_pBombProps = &MagmaProps;
			pBomb->m_fTimePassed=0.0f;

			pBomb->m_nWhichFlameToComputeCollision = 0;
			
			fparticle_SpawnEmitter( m_ahParticleDefs[0], pImpact->ImpactPoint.v3,  &pImpact->UnitFaceNormal.v3); 
			pBomb->_Init(pImpact);
			pBomb->m_bActive = TRUE;
			return pBomb;
		}
	}
	FASSERT(!"Maximum number of magma bomb fx in world exceeded, Tell Scholz to revise maximum ");
	return 0;
}

void CFXMagmaBomb::EndAllEffects(void)
{
	for (s32 nIndex= 0;nIndex < _MAX_NUM_MAGMAEFFECTS; nIndex++)
	{
		if (s_aMagmaBombs[nIndex].m_bActive)
			s_aMagmaBombs[nIndex].EndEffect();
	}
}

void CFXMagmaBomb::EndEffect(void)
{
	m_bActive = FALSE;
	for (s32 nIndex= 0;nIndex < _NUM_FLAMES ; nIndex++)
	{
		m_aFlames[nIndex].EndFlame();
	}
	for( u32 i=0; i<_NUM_SMOKES; i++ ) 
	{
		if (m_aSmokeEmitters[i].IsCreated())
		{
			m_aSmokeEmitters[i].StopAmbientSFX();
			if (m_aSmokeEmitters[i].IsEmitting())
				m_aSmokeEmitters[i].StopEmission();
			if (m_aSmokeEmitters[i].IsInWorld())
				m_aSmokeEmitters[i].RemoveFromWorld();
		}
	}
}

// Assumes the FDraw renderer is currently active.
// Assumes the game's main perspective camera is
// set up. Assumes there are no model Xfms on the
// stack.
//
// Does not preserve the current FDraw state.
// Does (not) preserve the current viewport.
// Will preserve the Xfm stack.
// Will preserve the frenderer fog state.

void CFXMagmaBomb::DrawAll( void )
{
	if( !m_bSystemActive )
	{
		return;
	}

	FASSERT( m_bSystemInitialized );
	if( !m_bSystemActive )
	{
		return;
	}

	frenderer_SetDefaultState();

	fdraw_Color_SetFunc( FDRAW_COLORFUNC_DIFFUSETEX_AIAT );
	fdraw_Alpha_SetBlendOp( FDRAW_BLENDOP_LERP_WITH_ALPHA_OPAQUE );
	fdraw_SetCullDir( FDRAW_CULLDIR_CW );

	// fdraw_SetCullDir( FDRAW_CULLDIR_NONE );
	// fdraw_Color_SetFunc( FDRAW_COLORFUNC_DIFFUSETEX_AIAT );
	// fdraw_Depth_SetTest( FDRAW_DEPTHTEST_CLOSER );

	// fdraw_Depth_EnableWriting( FALSE );


	for( u32 i=0; i<_MAX_NUM_MAGMAEFFECTS; i++ )
	{
		if( s_aMagmaBombs[i].m_bActive )
		{
			s_aMagmaBombs[i]._Draw();
		}
	}

}



CFXMagmaBomb::CFXMagmaBomb()
{
	m_bActive	= FALSE;
}

void CFXMagmaBomb::_Work( void )
{
	m_fTimePassed += FLoop_fPreviousLoopSecs;
	f32 fPercentComplete = m_fTimePassed * m_fOODuration;
	// update intensity
	f32 fCurve = fmath_PositiveCos( FMATH_2PI * (0.5f + fPercentComplete) );

	m_nWhichFlameToComputeCollision = (m_nWhichFlameToComputeCollision + 1)%_NUM_FLAMES;

	BOOL bActive=FALSE;
	for (s32 nIndex= 0;nIndex < _NUM_FLAMES ; nIndex++)
	{
		bActive |= m_aFlames[nIndex].Work(nIndex==m_nWhichFlameToComputeCollision);
	}
	for (s32 nIndex= 0;nIndex < _NUM_SMOKES ; nIndex++)
	{
		m_aSmokeEmitters[nIndex].SetAmbientVolume(fCurve);
		m_aSmokeEmitters[nIndex].SetUnitIntensity(fCurve);
	}

	if (bActive)
	{
		if (!m_uAISoundHandle || !AIEnviro_ModifySound(m_uAISoundHandle, m_vImpactPos, AIEnviro_fMagmaImpactSoundRadius, 1.0f, AISOUNDTYPE_SWARMERBARRIER, NULL))
		{
			m_uAISoundHandle = AIEnviro_AddSound(m_vImpactPos, AIEnviro_fMagmaImpactSoundRadius, 1.0, AISOUNDTYPE_SWARMERBARRIER, AISOUNDCTRL_NONE, NULL);
		}
	}
	else
	{
		EndEffect();
	}
}

void CAreaFlame::Create(const FParticle_DefHandle_t& rPart, BOOL bDoesDamage)
{
	m_bActive = FALSE;
	m_pBoundsWS = NULL;
	
	m_hFireParticleDef = rPart;
	
	m_pBoundsWS = NULL;
	
	m_bDealsDamage=bDoesDamage;
	
	m_pDamageForm = NULL;
}

void CAreaFlame::Init(CBot* pBot,CWeapon* pWeapon,const CFVec3A& vPos,const CFVec3A& vUp,const CFVec3A& vFront)
{
	m_bActive = TRUE;
	m_bIsCreeping = TRUE;
	m_bTryToCorner = FALSE;
	m_fTimePassed = 0.f;

	m_vStartUp.Set(vUp);
	m_vTravelDirection.Set(vFront);
	
	CFVec3A vTemp;
	vTemp.Mul(m_vTravelDirection,fmath_RandomFloatRange(m_pBombProps->fMinSpreadRadius,m_pBombProps->fMaxSpreadRadius));
	m_vEndPos.Add(vPos,vTemp);
	
	CFMtx43A mtxFire;
	mtxFire.m_vPos.Set(vPos);
	mtxFire.m_vUp.Set(vFront);
	mtxFire.m_vFront.Set(vUp);
	mtxFire.m_vRight.Cross(mtxFire.m_vFront,mtxFire.m_vUp);
	m_FireParticle.AddToWorld();
	m_FireParticle.SetUnitIntensity( 0.0 );
	m_FireParticle.Relocate_RotXlatFromUnitMtx_WS( &mtxFire );

	m_FireParticle.StartEmission(m_hFireParticleDef, 0.0f, -1.0f, FALSE);

	if (m_bDealsDamage)
	{
		m_pDamageForm = CDamage::GetEmptyDamageFormFromPool();
		if (!m_pDamageForm)
			DEVPRINTF("Failed to grab damage form");
	}
	if (m_bDealsDamage && m_pDamageForm) // this is a damaging area flame
	{
		CDamageProfile* pDamage =NULL;
		if (m_pBombProps->pAmbientDamageProfile)
			pDamage = m_pBombProps->pAmbientDamageProfile;
		else
			pDamage = CDamage::FindDamageProfile("MagmaBomb1",TRUE,TRUE);

		fang_MemCopy(&m_DamageProfile,pDamage,sizeof(CDamageProfile));
		m_DamageProfile.m_pszName = "MagmaBomb";
		m_DamageProfile.m_abZeroInnerAndOuterValues[0] = FALSE; // yes, hitpoints are affected
		m_DamageProfile.m_abZeroInnerAndOuterValues[1] = TRUE; // no impulse effect
		m_DamageProfile.m_abZeroInnerAndOuterValues[2] = TRUE; // no rumble effect
		m_DamageProfile.m_nDamageCause = DAMAGE_CAUSE_HEAT;
		m_DamageProfile.m_afUnitHitpoints[DAMAGE_HITPOINT_TYPE_PROJECTILE]=0.0f;
		m_DamageProfile.m_afUnitHitpoints[DAMAGE_HITPOINT_TYPE_BLAST]=0.0f;
		m_DamageProfile.m_afUnitHitpoints[DAMAGE_HITPOINT_TYPE_FLAME]=1.0f;

		m_pDamageForm->m_pFcnDamageBeingInflicted = _DamageNotify;
		m_pDamageForm->m_pUserArgument = m_pBombProps;
		m_pDamageForm->m_nDamageLocale = CDamageForm::DAMAGE_LOCALE_AMBIENT;
		m_pDamageForm->m_nDamageDelivery = CDamageForm::DAMAGE_DELIVERY_ALL_ENTITIES_WITHIN_PROFILE_RADIUS;
		m_pDamageForm->m_pDamageProfile = &this->m_DamageProfile;
		m_pDamageForm->m_fNormIntensity = 1.0f;
		m_pDamageForm->m_Damager.pBot = pBot;
		m_pDamageForm->m_Damager.pWeapon = pWeapon;
		m_pDamageForm->m_Damager.pEntity = NULL;
		FASSERT(pBot);
		m_pDamageForm->m_Damager.nDamagerPlayerIndex = pBot->m_nPossessionPlayerIndex; 
		// Tri Data:
		// not needed for LOCALE damage, presumed.
		m_pDamageForm->m_Epicenter_WS.x = vPos.x;
		m_pDamageForm->m_Epicenter_WS.y = vPos.y;
		m_pDamageForm->m_Epicenter_WS.z = vPos.z;
		FVisVolume_t* pVolume;
		fworld_GetVolumeContainingPoint( &m_pDamageForm->m_Epicenter_WS, &pVolume, m_FireParticle.GetTracker());
		m_pDamageForm->m_pVisVolume = pVolume;

		m_DamageProfile.m_HitpointRange.m_fOuterRadius = 0.f;
		m_DamageProfile.m_HitpointRange.m_fInnerRadius = 0.f;
		m_pBoundsWS = m_FireParticle.GetBoundingSphere();
		if (!m_pBoundsWS)
		{
			DEVPRINTF("no bounds, check the emitter handle\n");
		}
		CDamage::SubmitDamageForm(m_pDamageForm, -1.0f); // lasts forever or until canceled;
	}
}

void CAreaFlame::EndFlame(void)
{
	if (m_FireParticle.IsEmitting())
	{
		m_FireParticle.StopEmission();
	}

	if (m_pDamageForm)
	{
		CDamage::KillDamageForm(m_pDamageForm); // returns the form to the pool
		m_pDamageForm = NULL;
	}

	m_bActive= FALSE;
	m_pBoundsWS = NULL;
}

BOOL CAreaFlame::Work( BOOL bComputeCollisions )
{
	if (!this->m_bActive) // when we haven't been lit, we do nothing until then.
		return FALSE;

	CEntity* pParent = m_FireParticle.GetParent();

	if (pParent && (pParent->TypeBits() & ENTITY_BIT_BOT))
	{
		CBot* pParentBot = (CBot*) pParent;
		if (pParentBot->IsDeadOrDying()) // parents a goner, goodbye!
		{
			m_FireParticle.DetachFromParent(); // fall down or somethin?
		}
	}

	// update position
	CFVec3A vToEndPt;	
	CFVec3A vDeltaThisFrame;
	vToEndPt.Sub(m_vEndPos,m_FireParticle.MtxToWorld()->m_vPos);
	f32 fDistToEnd = vToEndPt.Mag();
	f32 fVelocityTimesTime = m_fVelocity*FLoop_fPreviousLoopSecs;
	if (m_bIsCreeping && (fDistToEnd > fVelocityTimesTime)) // not there yet
	{
		vToEndPt.Unitize();
		vDeltaThisFrame.Mul(vToEndPt,fVelocityTimesTime);
	}
	else
	{
		vDeltaThisFrame.Zero();
		m_bIsCreeping = FALSE;
	}

	if ( (bComputeCollisions||pParent) && m_bIsCreeping)
	{
		CFVec3A vTestStartPt,vTestEndPt,vWork;
		vTestStartPt.Add(m_FireParticle.MtxToWorld()->m_vPos, vDeltaThisFrame);
		vWork.Mul(m_FireParticle.MtxToWorld()->m_vFront,2.0f);
		vTestStartPt.Add(vTestStartPt,vWork); // sets start test point 2.0 above location
		vWork.Mul(m_FireParticle.MtxToWorld()->m_vFront,-4.0f);
		vTestEndPt.Add(vTestStartPt,vWork); // sets end test point 2.0 below location
		if (m_bTryToCorner)
		{
			vTestStartPt.Add(m_vTravelDirection);
			vTestEndPt.Sub(m_vTravelDirection);
		}
		FCollImpact_t CollImpact;
		if( fworld_FindClosestImpactPointToRayStart( &CollImpact, &vTestStartPt, &vTestEndPt) ) 
		{
			// We hit something!
			m_bTryToCorner = FALSE;
			m_FireParticle.MtxToWorld()->m_vPos.Set(CollImpact.ImpactPoint);
			m_FireParticle.MtxToWorld()->m_vFront.Set(CollImpact.UnitFaceNormal);
			if (m_bIsCreeping)
			{
				m_vTravelDirection.PlanarProjection(m_FireParticle.MtxToWorld()->m_vFront);
				if (m_vTravelDirection.MagSq() < 0.05f) // if the projection indicates directions nears perpendicular to normal
				{										// choose a different direction for safety
					
					m_vTravelDirection.Sub(CollImpact.aTriVtx[1],CollImpact.aTriVtx[0]);
				}
				m_vTravelDirection.Unitize();
				vWork.Mul(m_vTravelDirection,fDistToEnd-fVelocityTimesTime);
				m_vEndPos.Add(m_FireParticle.MtxToWorld()->m_vPos,vWork);
			}
			
			CEntity* pEntity = CGColl::ExtractEntity( &CollImpact );
			if( pEntity ) 
			{
				// It's an Entity!
				CEntity *pHitEntity = (CEntity *)pEntity;
				CFWorldMesh *pWorldMesh = (CFWorldMesh *)CollImpact.pTag;
				if (CollImpact.nBoneIndex!=-1)
				{
					m_FireParticle.Attach_ToParent_WS(pHitEntity,pWorldMesh->m_pMesh->pBoneArray[CollImpact.nBoneIndex].szName);
				}
				else
				{
					m_FireParticle.Attach_ToParent_WS(pHitEntity);
				}
			}
			else
			{
				m_FireParticle.DetachFromParent();
			}
		}
		else
		{
//			CFColorRGBA magenta;magenta.OpaqueMagenta();
//			fdraw_DevSphere(&vTestStartPt,.5f);
//			fdraw_DevSphere(&vTestEndPt,.5f,&magenta,5,6,7);
			
			if (m_bTryToCorner==FALSE)
			{
				m_bTryToCorner = TRUE;
			}
			else
			{
				m_bTryToCorner = FALSE;
				m_vEndPos.Set(m_FireParticle.MtxToWorld()->m_vPos);
			}
		}
	}
	else if (m_bIsCreeping) // no collisions computed, add in the delta
	{
		CFVec3A vNewPosition;
		vNewPosition.Add(m_FireParticle.MtxToWorld()->m_vPos,vDeltaThisFrame);
		m_FireParticle.Relocate_Xlat_WS(&vNewPosition);
	}

	m_fTimePassed += FLoop_fPreviousLoopSecs;
	f32 fPercentComplete = m_fTimePassed * m_fOODuration;
	// update emitting
	if (m_FireParticle.IsEmitting() == FALSE) // failed to fparticle from pool?
	{
		m_FireParticle.StartEmission(m_hFireParticleDef, 0.0f, -1.0f, FALSE);
	}
	if (m_FireParticle.IsEmitting() == FALSE)
	{
		DEVPRINTF("couldn't continue AreaFlame::Work, fparticle fails to emit\n");
		return TRUE;
	}
	// update intensity
	f32 fCurve = fmath_PositiveCos( FMATH_2PI * (0.5f + fPercentComplete) );
	this->m_FireParticle.SetUnitIntensity(fCurve);

	// update damage 
	if (m_pDamageForm)
	{
		m_pBoundsWS = m_FireParticle.GetBoundingSphere();
		if (m_pBoundsWS)
		{
			m_pDamageForm->m_Epicenter_WS.x = m_pBoundsWS->m_Pos.x;
			m_pDamageForm->m_Epicenter_WS.y = m_pBoundsWS->m_Pos.y;
			m_pDamageForm->m_Epicenter_WS.z = m_pBoundsWS->m_Pos.z;
			FVisVolume_t* pVolume;
			fworld_GetVolumeContainingPoint( &m_pDamageForm->m_Epicenter_WS, &pVolume, m_FireParticle.GetTracker());
			m_pDamageForm->m_pVisVolume = pVolume;
			f32 fOuterInnerRadiusDifference = m_DamageProfile.m_HitpointRange.m_fOuterRadius - m_DamageProfile.m_HitpointRange.m_fInnerRadius;
			m_DamageProfile.m_HitpointRange.m_fOuterRadius = m_pBoundsWS->m_fRadius;
			FASSERT(m_pBoundsWS->m_fRadius < 10000.f);
			m_DamageProfile.m_HitpointRange.m_fInnerRadius = m_pBoundsWS->m_fRadius - fOuterInnerRadiusDifference;
	//		fdraw_DevSphere(&pBoundsWS->m_Pos,pBoundsWS->m_fRadius);
		}
		else
		{
			DEVPRINTF("CAreaFlame::Work() [magmabomb] couldn't get a particle system bounds sphere\n");
		}
	}
	// update duration
	if (m_fTimePassed>m_fDuration)
	{
		EndFlame();
		return FALSE;
	}
	return TRUE;
}
void CAreaFlame::Draw(void)
{
//	CFColorRGBA white;white.OpaqueWhite();
//	CFColorRGBA red;red.OpaqueRed();
///	CFColorRGBA green;green.OpaqueGreen();
//	CFColorRGBA cyan;cyan.OpaqueCyan();
	
//	fdraw_DevSphere(&m_FireParticle.MtxToWorld()->m_vPos,0.75f,&red,4,4,2);
}

void CFXMagmaBomb::_Draw( void )
{
	for (u32 uIndex= 0;uIndex < _NUM_FLAMES; uIndex++)
	{
		if (m_aFlames[uIndex].m_bActive)
		{
			m_aFlames[uIndex].Draw();
		}
	}
}