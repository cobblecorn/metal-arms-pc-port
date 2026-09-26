#include "MagnetDevice.h"
#include "fang.h"
#include "fworld.h"
#include "floop.h"
#include "fresload.h"
#include "game.h"
#include "gamecam.h"
#include "entity.h"
#include "meshentity.h"
#include "bot.h"
#include "cambot.h"
#include "cammanual.h"
#include "camsimple.h"

#define MAGNET_SHOW_DEBUG 0

#define _SPEED_DELTA 1.0f
#define _MAX_ANIM_SPD 4.0f
#define  _PULL_FORCE_MAG 5.0f
//50*50 = 2500
#define _EFFECT_RADIUS 50.0f
#define _EFFECT_RADIUS2 2500.0f
//25*25 = 500, damage radius2
#define _DAMAGE_RADIUS2 500.0f
//Spark particle when objects are being chewed by the device.
#define _SPARK_PARTICLE_NAME "PEBCspark01"
#define _MAGNET_PARTICLE_NAME "Magnetic"

static BOOL _bFoundBone=FALSE;
static u8 _nTargetBoneIdx=0, _nBladeBoneIdx=1;
static CFSphere _Sphere;
static BOOL _bPlayerSetup=FALSE;

u32 CMagnetDevice::m_nNumCollisions=0;
u32 CMagnetDevice::m_nNumCollisionEntity=0;
CEntity *CMagnetDevice::m_pCollisionList[CMagnetDevice::MAX_COLLISION_SIZE];
u32 CMagnetDevice::m_nColTest=CMagnetDevice::MCOL_LIST;

CMagnetDevice::CMagnetDevice()
{
	m_pMeshEntity = NULL;
	m_pWorldMesh = NULL;
	m_pCallbackFunc = NULL;
	m_State = STATE_IDLE;
	m_fAnimSpd = 0;
	m_hEmitter = 0;
	m_hMagnetEmitter = 0;
	m_nNumCollisionEntity = 0;
	m_bParticleSpawn = FALSE;

	m_fUnitTargetRot = 0.0f;
	m_bDisable = FALSE;

	m_pDamageProfile = CDamage::FindDamageProfile( "Water" );
	m_hSparkParticleDef = (FParticle_DefHandle_t)fresload_Load( FPARTICLE_RESTYPE, _SPARK_PARTICLE_NAME );
	m_hMagnetParticleDef = (FParticle_DefHandle_t)fresload_Load( FPARTICLE_RESTYPE, _MAGNET_PARTICLE_NAME );
}

void CMagnetDevice::SetMeshEntity(CMeshEntity *pEntity)
{
	m_pMeshEntity = pEntity;
	if (m_pMeshEntity)
	{
		m_pWorldMesh = m_pMeshEntity->GetMesh();
		m_pMeshEntity->UserAnim_Pause(TRUE);

		if (!_bFoundBone)
		{
			_bFoundBone = TRUE;

			const FMeshBone_t *pMeshBoneArray = m_pWorldMesh->m_pMesh->pBoneArray;
			u32 i;

			for (i=0; i<m_pWorldMesh->m_pMesh->nBoneCount; i++)
			{
				if ( fclib_stricmp(pMeshBoneArray[i].szName, "target")==0 )
				{
					_nTargetBoneIdx = i;
				}
				else if ( fclib_stricmp(pMeshBoneArray[i].szName, "blade")==0 )
				{
					_nBladeBoneIdx = i;
				}
			}
		}

		//Cone data
		m_fSinReciprocal = 1.0f/fmath_Sin( FMATH_QUARTER_PI );
		m_fCosSqr = fmath_Cos( FMATH_QUARTER_PI ); m_fCosSqr *= m_fCosSqr;
		m_fSinSqr = fmath_Sin( FMATH_QUARTER_PI ); m_fSinSqr *= m_fSinSqr;

		CFVec3 vFront;
		vFront.Set( m_pWorldMesh->m_Xfm.m_MtxF.m_vFront.x, m_pWorldMesh->m_Xfm.m_MtxF.m_vFront.y, m_pWorldMesh->m_Xfm.m_MtxF.m_vFront.z );
		vFront.Unitize();

		m_vAxis = -vFront;
		//

		CFSphere sphere;
		m_pWorldMesh->GetBoneSphere_WS(_nBladeBoneIdx, &sphere);

		sphere.m_Pos.x -= vFront.x*6.0f;
		sphere.m_Pos.y -= 4.0f;
		sphere.m_Pos.z -= vFront.z*6.0f;

		m_vMagnetOrigin = sphere.m_Pos;
	}

	m_fUnitTargetRot = 0.0f;
	m_bDisable = FALSE;
}

void CMagnetDevice::Reset()
{
	_bPlayerSetup = FALSE;
	m_State = STATE_IDLE;
	m_bDisable = FALSE;

	if ( m_bParticleSpawn && m_hEmitter != FPARTICLE_INVALID_HANDLE )
	{
		m_bParticleSpawn = FALSE;
		fparticle_StopEmitter( m_hEmitter );
        m_hEmitter = 0;
	}

	if ( m_hMagnetParticleDef != FPARTICLE_INVALID_HANDLE && m_hMagnetEmitter )
	{
		fparticle_StopEmitter( m_hMagnetEmitter );
		m_hMagnetEmitter = 0;
	}

	m_fUnitTargetRot = 0.0f;
}

void CMagnetDevice::AddCollisionEntity(CEntity *pEntity)
{
	if (m_nNumCollisionEntity < MAX_COLLISION_SIZE)
	{
		m_pCollisionList[ m_nNumCollisionEntity++ ] = pEntity;
	}
}

void CMagnetDevice::GenerateCollisionList()
{
	m_nNumCollisionEntity = 0;

	u32 i, nNumEntity = CEntity::GetInWorldCount();
	if (nNumEntity)
	{
		CEntity *pEntity = CEntity::InWorldList_GetHead();
		for (i=0; i<nNumEntity; i++)
		{
			if (pEntity)
			{
				if (pEntity->TypeBits()&ENTITY_BIT_BOT)
				{
					AddCollisionEntity(pEntity);
				}
			}
			pEntity = pEntity->InWorldList_GetNext();
		}
	}
}

void CMagnetDevice::ClearCollisionEntity()
{
	m_nNumCollisionEntity = 0;
}

BOOL CMagnetDevice::MagnetDeviceCollisionCallback( CFWorldTracker *pTracker, FVisVolume_t *pVolume )
{
	if (pTracker)
	{
		FWorldTrackerType_e nType = pTracker->GetType();
		if ( nType != FWORLD_TRACKERTYPE_LIGHT && nType != FWORLD_TRACKERTYPE_USER && pTracker->IsAddedToWorld() )
		{
			CFSphere Sphere = pTracker->GetBoundingSphere();
			CFVec3 vOffs = Sphere.m_Pos - _Sphere.m_Pos;
			f32 fMag = vOffs.Mag();
			fMag = fMag - _Sphere.m_fRadius;

			//I hit the target with something
			if (fMag <= 0.0f)
			{
				m_nNumCollisions++;
			}
		}
	}
	return TRUE;
}

BOOL CMagnetDevice::RayCollidesWithTarget(const CFVec3A *vRayStart, const CFVec3A *vRayEnd, CFVec3A *vCenter)
{
	BOOL bRet = FALSE;
	CFSphere sphere;
	m_pWorldMesh->GetBoneSphere_WS(_nTargetBoneIdx, &sphere);
	sphere.m_Pos.y += 1.0f;
	sphere.m_fRadius = 3.5f;

	if ( m_State == STATE_IDLE && m_fUnitTargetRot <= 0.0f )
	{
	
		if ( sphere.IsIntersecting(vRayStart->v3, vRayEnd->v3) )
		{
			vCenter->v3 = sphere.m_Pos;
			bRet = TRUE;
		}

	}

	return bRet;
}

void CMagnetDevice::ApplyDamage(CEntity *pEDamagee, CFVec3& vPos, CFVec3& vDir)
{
	// Get an empty damage form...
	CDamageForm *pDamageForm = CDamage::GetEmptyDamageFormFromPool();

	if( pDamageForm )
	{
		// Fill out the form...
		pDamageForm->m_nDamageLocale = CDamageForm::DAMAGE_LOCALE_AMBIENT;
		pDamageForm->m_nDamageDelivery = CDamageForm::DAMAGE_DELIVERY_ONE_SPECIFIC_ENTITY;
		pDamageForm->m_pDamageProfile = m_pDamageProfile;
		pDamageForm->m_Damager.pWeapon = NULL;
		pDamageForm->m_Damager.pBot = NULL;
		pDamageForm->m_pDamageeEntity = pEDamagee;
		
		CDamage::SubmitDamageForm( pDamageForm );
	}
	//Play particle effect at bot location.
	if ( m_hSparkParticleDef != FPARTICLE_INVALID_HANDLE && !m_bParticleSpawn)
	{
		m_bParticleSpawn = TRUE;

		if (m_hEmitter)
		{
			fparticle_StopEmitter( m_hEmitter );
			m_hEmitter = 0;
		}

		CFVec3 vSparkDir = m_vAxis;
		m_hEmitter = fparticle_SpawnEmitter( m_hSparkParticleDef, vPos, &vSparkDir, 1.0f ); 
	}
}

BOOL CMagnetDevice::InsideCone(const CFSphere *vSphere)
{
	return TRUE;
	CFVec3 vU, vD;
	f32 fDsqr, fE;

	vU = m_vMagnetOrigin - (m_vAxis*(vSphere->m_fRadius*m_fSinReciprocal));
	vD = vSphere->m_Pos - vU;
	fDsqr = vD.Dot(vD);
	fE = m_vAxis.Dot(vD);
	if ( fE > 0.0f && fE*fE >= fDsqr*m_fCosSqr )
	{
		vD = vSphere->m_Pos - m_vMagnetOrigin;
		fDsqr = vD.Dot(vD);
		fE = -m_vAxis.Dot(vD);
		if ( fE > 0.0f && fE*fE >= fDsqr*m_fSinSqr )
		{
			return (fDsqr <= vSphere->m_fRadius*vSphere->m_fRadius);
		}
		else
		{
			return TRUE;
		}
	}
	return FALSE;
}

void CMagnetDevice::RotateTarget(f32 fRadians)
{
	if (fRadians <= 0.0f) return;

	CFMtx43A **pMtxPal = m_pWorldMesh->GetBoneMtxPalette();
	CFMtx43A rotMtx;

	rotMtx.SetRotationYXZ(0, fRadians, 0);
	*pMtxPal[ _nTargetBoneIdx ] = pMtxPal[ _nTargetBoneIdx ]->Mul33(rotMtx);

	m_pWorldMesh->SetBoneMtxPalette(pMtxPal);
}

#define _ROT_RAD 1.047197551196598f

void CMagnetDevice::HandleInsideCone(CFVec3& vOffs, BOOL& bTurnOffSparks, f32 fMagXZ2, u32 i)
{
	CFVec3 vDir, vMDir;
	CFVec3A vNewLocWS;
	f32 fMag;

	if ( FMATH_FABS(vOffs.x) > 0.01f || FMATH_FABS(vOffs.y) > 0.01f || FMATH_FABS(vOffs.z) > 0.01f )
	{
		fMag = vOffs.ExtractUnitAndInvMag(vOffs);
		vDir = vOffs;
	
		vOffs *= _PULL_FORCE_MAG;
		vOffs *= FMATH_MIN(m_fAnimSpd, 1.0f);
	}
	else
	{
		vNewLocWS.v3 = m_vMagnetOrigin;
		vNewLocWS.y -= ((CBot *)m_pCollisionList[i])->GetMesh()->m_pMesh->BoundSphere_MS.m_Pos.y;
		f32 fD = -m_vAxis.Dot(vNewLocWS.v3);

        ((CBot *)m_pCollisionList[i])->ImmobilizeBot();
		((CBot *)m_pCollisionList[i])->GetMesh()->SetCollisionFlag(FALSE);
		((CBot *)m_pCollisionList[i])->SetBotFlag_NoGravity();

		f32 fMove = FLoop_fPreviousLoopSecs*15.0f;
		vOffs = vNewLocWS.v3 - ((CBot *)m_pCollisionList[i])->GetMesh()->m_Xfm.m_MtxF.m_vPos.v3;
		f32 fMag2 = vOffs.Mag2();
		if ( fMag2 > 4.0f*225.0f*FLoop_fPreviousLoopSecs )
		{
			CFVec3 vOffs = vNewLocWS.v3 - ((CBot *)m_pCollisionList[i])->GetMesh()->m_Xfm.m_MtxF.m_vPos.v3;
			fMag = vOffs.ExtractUnitAndMag(vMDir);

			vMDir *= (fMove<fMag)?(fMove):(fMag);
			vNewLocWS.v3 = ((CBot *)m_pCollisionList[i])->GetMesh()->m_Xfm.m_MtxF.m_vPos.v3 + vMDir;

			f32 fDist = m_vAxis.Dot(vNewLocWS.v3) + fD;
			if ( fDist < 0.0f )
			{
				vNewLocWS.v3 = m_vMagnetOrigin;	
				vNewLocWS.y -= ((CBot *)m_pCollisionList[i])->GetMesh()->m_pMesh->BoundSphere_MS.m_Pos.y;	
			}
		}

		((CBot *)m_pCollisionList[i])->Relocate_Xlat_WS(&vNewLocWS);
		vOffs.Set(0,0,0);
	}

 	if ( fMagXZ2 - _DAMAGE_RADIUS2 <= 0.0f )
	{
		if (m_pCallbackFunc)
		{
			m_pCallbackFunc( m_pUserData, (CBot *)m_pCollisionList[i], m_vMagnetOrigin );
		}

		vNewLocWS.v3 = m_vMagnetOrigin;
		vNewLocWS.y -= ((CBot *)m_pCollisionList[i])->GetMesh()->m_pMesh->BoundSphere_MS.m_Pos.y;

		f32 fD = -m_vAxis.Dot(vNewLocWS.v3);

		ApplyDamage( m_pCollisionList[i], m_vMagnetOrigin, vDir );
		m_pCurDamageBot = (CBot *)m_pCollisionList[i];

		((CBot *)m_pCollisionList[i])->ImmobilizeBot();
		((CBot *)m_pCollisionList[i])->GetMesh()->SetCollisionFlag(FALSE);
		((CBot *)m_pCollisionList[i])->SetBotFlag_NoGravity();

		f32 fMove = FLoop_fPreviousLoopSecs*15.0f;

		vOffs = vNewLocWS.v3 - ((CBot *)m_pCollisionList[i])->GetMesh()->m_Xfm.m_MtxF.m_vPos.v3;
		f32 fMag2 = vOffs.Mag2();
		if ( fMag2 > 4.0f*225.0f*FLoop_fPreviousLoopSecs )
		{
			CFVec3 vOffs = vNewLocWS.v3 - ((CBot *)m_pCollisionList[i])->GetMesh()->m_Xfm.m_MtxF.m_vPos.v3;
			fMag = vOffs.ExtractUnitAndMag(vMDir);

			vMDir *= (fMove<fMag)?(fMove):(fMag);
			vNewLocWS.v3 = ((CBot *)m_pCollisionList[i])->GetMesh()->m_Xfm.m_MtxF.m_vPos.v3 + vMDir;

			f32 fDist = m_vAxis.Dot(vNewLocWS.v3) + fD;
			if ( fDist < 0.0f )
			{
				vNewLocWS.v3 = m_vMagnetOrigin;	
				vNewLocWS.y -= ((CBot *)m_pCollisionList[i])->GetMesh()->m_pMesh->BoundSphere_MS.m_Pos.y;	
			}
		}

		((CBot *)m_pCollisionList[i])->Relocate_Xlat_WS(&vNewLocWS);

		bTurnOffSparks = FALSE;
		vOffs.Set(0,0,0);	
	}
}

void CMagnetDevice::Disable()
{
	m_bDisable = TRUE;
	m_State = STATE_IDLE;
	
	if ( m_nColTest == MCOL_ALLBOTS )
	{
		GenerateCollisionList();
	}

	u32 i;	
	for (i=0; i<m_nNumCollisionEntity; i++)
	{
		((CBot *)m_pCollisionList[i])->MobilizeBot();
		((CBot *)m_pCollisionList[i])->GetMesh()->SetCollisionFlag(TRUE);
		((CBot *)m_pCollisionList[i])->ClearBotFlag_NoGravity();
	}
}

void CMagnetDevice::Work()
{
	if (!m_pWorldMesh) return;

	u32 i;
	CFSphere sphere;
	CFWorldMesh *pMesh;
	CFVec3 vOffs;
	CFVec3A vNewLocWS;
	BOOL bTurnOffSparks;
	BOOL bClearDamageBot;

#if !FANG_PRODUCTION_BUILD && MAGNET_SHOW_DEBUG
	if (m_pWorldMesh)
	{
		CFColorRGBA clr(0.5f, 1.0f, 0.5f, 1.0f);
		m_pWorldMesh->GetBoneSphere_WS(_nTargetBoneIdx, &sphere);
		sphere.m_Pos.y += 1.0f;
		fdraw_DevSphere(&sphere.m_Pos, 3.5f, &clr, 2, 2, 2);

		clr.fRed = 1.0f; clr.fGreen = 0.5f; clr.fBlue = 0.5f; clr.fAlpha = 1.0f;
		fdraw_DevSphere(&m_vMagnetOrigin, 1.0f, &clr, 2, 2, 2);
	}
#endif

    switch (m_State)
	{
		case STATE_IDLE:
			m_fAnimSpd = 0.0f;
			
			m_nNumCollisions = 0;
			
			if (!m_bDisable)
			{
				if (m_fUnitTargetRot <= 0.0f)
				{
					m_fUnitTargetRot = 0.0f;
					m_pWorldMesh->GetBoneSphere_WS(_nTargetBoneIdx, &sphere);

					_Sphere.m_Pos = sphere.m_Pos;
					_Sphere.m_Pos.y += 1.0f;
					_Sphere.m_fRadius = 3.5f;

					m_pWorldMesh->FindIntersectingTrackers( MagnetDeviceCollisionCallback );

					if (m_nNumCollisions)
					{
						m_State = STATE_PULL;
					}
				}
				else
				{
					m_fUnitTargetRot -= FLoop_fPreviousLoopSecs;
					FMATH_CLAMP(m_fUnitTargetRot, 0, _ROT_RAD);
					RotateTarget(m_fUnitTargetRot);
				}
			}

			m_pCurDamageBot = NULL;

			break;
		case STATE_PULL:
		{
			m_pWorldMesh->GetBoneSphere_WS(_nBladeBoneIdx, &sphere);

			if (m_fAnimSpd == 0.0f)
			{
				m_pMeshEntity->UserAnim_Pause(FALSE);
				m_pMeshEntity->UserAnim_Select(0);

				CFVec3 vFront;
				vFront.Set( m_pWorldMesh->m_Xfm.m_MtxF.m_vFront.x, m_pWorldMesh->m_Xfm.m_MtxF.m_vFront.y, m_pWorldMesh->m_Xfm.m_MtxF.m_vFront.z );
				vFront.Unitize();
				m_vMagnetPos = m_vMagnetOrigin - (vFront*_EFFECT_RADIUS);
				m_vMagnetDir = vFront;

				m_vNewMagnetPos = m_vMagnetPos;
				m_vNewMagnetDir = m_vMagnetDir;

				m_hMagnetEmitter = fparticle_SpawnEmitter( m_hMagnetParticleDef, &m_vNewMagnetPos, &m_vNewMagnetDir );
				if (m_hMagnetEmitter)
				{
					fparticle_SetDirection( m_hMagnetEmitter, &m_vNewMagnetDir );
				}
			}
			m_fAnimSpd += ( FLoop_fPreviousLoopSecs * _SPEED_DELTA );
			//Update particle system
			FMATH_CLAMP(m_fAnimSpd, 0.0f, _MAX_ANIM_SPD);
			m_pMeshEntity->UserAnim_SetSpeedMult(m_fAnimSpd);

			f32 fCos, fSin;
			fCos = fmath_Cos( fmath_Fmod(m_fAnimSpd*4.0f, FMATH_2PI) ) * _EFFECT_RADIUS * 0.1f;
			fSin = fmath_Sin( fmath_Fmod(m_fAnimSpd*4.0f, FMATH_2PI) ) * _EFFECT_RADIUS * 0.1f;
			m_vNewMagnetPos.x = fCos*m_pWorldMesh->m_Xfm.m_MtxF.m_vRight.x + fSin*m_pWorldMesh->m_Xfm.m_MtxF.m_vRight.y;
			m_vNewMagnetPos.y = fCos*m_pWorldMesh->m_Xfm.m_MtxF.m_vUp.x + fSin*m_pWorldMesh->m_Xfm.m_MtxF.m_vUp.y;
			m_vNewMagnetPos.z = fCos*m_pWorldMesh->m_Xfm.m_MtxF.m_vFront.x + fSin*m_pWorldMesh->m_Xfm.m_MtxF.m_vFront.y;

			m_vNewMagnetPos += m_vMagnetPos;
			m_vNewMagnetDir = m_vMagnetOrigin - m_vNewMagnetPos;
			m_vNewMagnetDir.Unitize();
			//
	
			//Create a force toward the blade:
			sphere = m_pWorldMesh->GetBoundingSphere();

			bTurnOffSparks = TRUE;

			if (m_pCurDamageBot)
			{
				if ( m_pCurDamageBot->IsDeadOrDying() )
				{
					m_pCurDamageBot = NULL;
				}
			}

			bClearDamageBot=TRUE;

			if ( m_nColTest == MCOL_ALLBOTS )
			{
				GenerateCollisionList();
			}

			for (i=0; i<m_nNumCollisionEntity; i++)
			{
				pMesh = m_pCollisionList[i]->GetMesh();
				//if (pMesh && !((CBot *)m_pCollisionList[i])->IsDeadOrDying() && (CBot *)m_pCollisionList[i] != m_pCurDamageBot)
				if ( pMesh && (CBot *)m_pCollisionList[i] != m_pCurDamageBot )
				{
					vNewLocWS.v3 = m_vMagnetOrigin;
					vNewLocWS.y -= ((CBot *)m_pCollisionList[i])->GetMesh()->m_pMesh->BoundSphere_MS.m_Pos.y;

					f32 fMagXZ2;
					vOffs = vNewLocWS.v3 - pMesh->GetBoundingSphere().m_Pos;
					fMagXZ2 = vOffs.MagXZ2();

                    if ( fMagXZ2 - _EFFECT_RADIUS2 <= 0.0f )
					{
						if ( InsideCone( &pMesh->GetBoundingSphere() ) )
						{
							if ( ((CBot *)m_pCollisionList[i])->IsDeadOrDying() )
							{
								((CBot *)m_pCollisionList[i])->ZeroVelocity();
							}
							else
							{
								if ( ((CBot *)m_pCollisionList[i]) == m_pCurDamageBot )
								{	
									bClearDamageBot=FALSE;
								}

								HandleInsideCone(vOffs, bTurnOffSparks, fMagXZ2, i);
								
								if ( FMATH_FABS(vOffs.x) > 0.1f || FMATH_FABS(vOffs.y) > 0.1f || FMATH_FABS(vOffs.z) > 0.1f )
								{
									((CBot *)m_pCollisionList[i])->ApplyVelocityImpulse_WS( CFVec3A(vOffs) );
								}
							}
						}
					}
				}
			}

			if (bClearDamageBot)
			{
				m_pCurDamageBot = NULL;
			}

			if (m_pFXCallbackFunc)
			{
				m_pFXCallbackFunc(m_pUserData, (void *)&m_fAnimSpd, m_vMagnetOrigin);
			}

			if (m_fAnimSpd == _MAX_ANIM_SPD)
			{
				m_State = STATE_PUSH;
				m_fUnitLerp = 0.0f;
			}

			m_fUnitTargetRot += FLoop_fPreviousLoopSecs*6.0f;
			FMATH_CLAMP(m_fUnitTargetRot, 0, _ROT_RAD);
			RotateTarget(m_fUnitTargetRot);

			if (bTurnOffSparks)
			{
				if (m_bParticleSpawn)
				{
					m_bParticleSpawn = FALSE;
					fparticle_StopEmitter( m_hEmitter );
					m_hEmitter = 0;
				}
			}
		}
		break;
		case STATE_PUSH:

			m_fAnimSpd -= ( FLoop_fPreviousLoopSecs * _SPEED_DELTA );
			FMATH_CLAMP(m_fAnimSpd, 0.0f, _MAX_ANIM_SPD);
			m_pMeshEntity->UserAnim_SetSpeedMult(m_fAnimSpd);

			//Update particle system
			FMATH_CLAMP(m_fAnimSpd, 0.0f, _MAX_ANIM_SPD);
			m_pMeshEntity->UserAnim_SetSpeedMult(m_fAnimSpd);

			f32 fCos, fSin;
			fCos = fmath_Cos( fmath_Fmod(m_fAnimSpd*4.0f, FMATH_2PI) ) * _EFFECT_RADIUS * 0.1f;
			fSin = fmath_Sin( fmath_Fmod(m_fAnimSpd*4.0f, FMATH_2PI) ) * _EFFECT_RADIUS * 0.1f;
			m_vNewMagnetPos.x = fCos*m_pWorldMesh->m_Xfm.m_MtxF.m_vRight.x + fSin*m_pWorldMesh->m_Xfm.m_MtxF.m_vRight.y;
			m_vNewMagnetPos.y = fCos*m_pWorldMesh->m_Xfm.m_MtxF.m_vUp.x + fSin*m_pWorldMesh->m_Xfm.m_MtxF.m_vUp.y;
			m_vNewMagnetPos.z = fCos*m_pWorldMesh->m_Xfm.m_MtxF.m_vFront.x + fSin*m_pWorldMesh->m_Xfm.m_MtxF.m_vFront.y;

			m_vNewMagnetPos += m_vMagnetPos;
			m_vNewMagnetDir = m_vMagnetOrigin - m_vNewMagnetPos;
			m_vNewMagnetDir.Unitize();
			//

			//Create a force toward the blade:
			sphere = m_pWorldMesh->GetBoundingSphere();

			bTurnOffSparks = TRUE;

			if (m_fAnimSpd > 0.0f)
			{
				if (m_pCurDamageBot)
				{
					if ( m_pCurDamageBot->IsDeadOrDying() )
					{
						m_pCurDamageBot = NULL;
					}
				}

				bClearDamageBot=TRUE;

				if ( m_nColTest == MCOL_ALLBOTS )
				{
					GenerateCollisionList();
				}

				for (i=0; i<m_nNumCollisionEntity; i++)
				{
                    pMesh = m_pCollisionList[i]->GetMesh();
					//if (pMesh && !((CBot *)m_pCollisionList[i])->IsDeadOrDying() && (CBot *)m_pCollisionList[i] != m_pCurDamageBot )
					if ( pMesh && (CBot *)m_pCollisionList[i] != m_pCurDamageBot )
					{
						vNewLocWS.v3 = m_vMagnetOrigin;
						vNewLocWS.y -= ((CBot *)m_pCollisionList[i])->GetMesh()->m_pMesh->BoundSphere_MS.m_Pos.y;

						f32 fMagXZ2;
						vOffs = vNewLocWS.v3 - pMesh->GetBoundingSphere().m_Pos;
						fMagXZ2 = vOffs.MagXZ2();

						if ( fMagXZ2 - _EFFECT_RADIUS2 <= 0.0f )
						{
							if ( InsideCone( &pMesh->GetBoundingSphere() ) )
							{
								if ( ((CBot *)m_pCollisionList[i])->IsDeadOrDying() )
								{
									((CBot *)m_pCollisionList[i])->ZeroVelocity();
								}
								else
								{
									if ( ((CBot *)m_pCollisionList[i]) == m_pCurDamageBot )
									{	
										bClearDamageBot=FALSE;
									}

									HandleInsideCone(vOffs, bTurnOffSparks, fMagXZ2, i);
																	
									if ( FMATH_FABS(vOffs.x) > 0.1f || FMATH_FABS(vOffs.y) > 0.1f || FMATH_FABS(vOffs.z) > 0.1f )
									{
										((CBot *)m_pCollisionList[i])->ApplyVelocityImpulse_WS( CFVec3A(vOffs) );
									}
								}
							}
						}
					}
				}

				if (bClearDamageBot)
				{
					m_pCurDamageBot = NULL;
				}
			}
			
			if (m_pFXCallbackFunc)
			{
				m_pFXCallbackFunc(m_pUserData, (void *)&m_fAnimSpd, m_vMagnetOrigin);
			}

			if (m_fAnimSpd == 0.0f)
			{
				m_State = STATE_IDLE;
				if (_bPlayerSetup)
				{
					m_pMeshEntity->UserAnim_Pause(TRUE);

					gamecam_SwitchPlayerTo3rdPersonCamera( GAME_CAM_PLAYER_1, (CBot *)m_pCollisionList[0] );
					_bPlayerSetup = FALSE;
				}

				for (i=0; i<m_nNumCollisionEntity; i++)
				{
					((CBot *)m_pCollisionList[i])->MobilizeBot();
					((CBot *)m_pCollisionList[i])->GetMesh()->SetCollisionFlag(TRUE);
					((CBot *)m_pCollisionList[i])->ClearBotFlag_NoGravity();
				}

				if (m_hMagnetEmitter)
				{
					fparticle_StopEmitter(m_hMagnetEmitter);
					m_hMagnetEmitter = 0;
				}

				if (m_bParticleSpawn)
				{
					m_bParticleSpawn = FALSE;
					fparticle_StopEmitter( m_hEmitter );
					m_hEmitter = 0;
				}
			}
			else if (bTurnOffSparks)
			{
				if (m_bParticleSpawn)
				{
					m_bParticleSpawn = FALSE;
					fparticle_StopEmitter( m_hEmitter );
					m_hEmitter = 0;
				}
			}

			RotateTarget(m_fUnitTargetRot);

			break;
		default:
			break;
	}
}
