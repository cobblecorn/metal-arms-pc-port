//////////////////////////////////////////////////////////////////////////////////////
// eboomer.cpp - Destructable entity class.
//
// Author: Steve Ranck     
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
// 03/25/03 Ranck       Created.
//////////////////////////////////////////////////////////////////////////////////////

#include "fang.h"
#include "eboomer.h"
#include "entity.h"
#include "fsound.h"
#include "fresload.h"
#include "explosion.h"
#include "eparticlepool.h"
#include "FScriptSystem.h"




//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CEBoomerBuilder
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

static CEBoomerBuilder _EBoomerBuilder;


void CEBoomerBuilder::SetDefaults( u64 nEntityTypeBits, u64 nEntityLeafTypeBit, cchar *pszEntityType ) {
	u32 i;

	ENTITY_BUILDER_SET_PARENT_CLASS_DEFAULTS( CMeshEntityBuilder, ENTITY_BIT_BOOMER, pszEntityType );

	m_nDeadMeshTexFlipID = 0;

	m_bDetPackOnly = FALSE;
	m_bZombieBossBallOnly = FALSE;
	m_bVehicleCollide = FALSE;

	m_pszDamageProfile = NULL;
	m_pszDamageOnlyProfile = NULL;
	m_pszSoundGroupBoom = NULL;
	m_pszSoundGroupAlive = NULL;
	m_pszSoundGroupDead = NULL;

	m_ExplosionBuilder.apszName[0] = NULL;
	m_ExplosionBuilder.apszName[1] = NULL;

	for( i=0; i<CEBoomer::MAX_PARTICLE_COUNT; ++i ) {
		m_aParticleBuilder[i].apszName[0] = NULL;
		m_aParticleBuilder[i].apszName[1] = NULL;
		m_aParticleBuilder[i].fDuration = 1.0f;
	}

	// The default armor profile for boomers is "None"...
	m_pszEC_ArmorProfile = "None";

	m_nVehicleSlowdown = 0;
}


BOOL CEBoomerBuilder::InterpretTable( void ) {
	if( !fclib_stricmp( CEntityParser::m_pszTableName, "Explosion" ) ) {
		// Explosion = <None | name> [, ptack]
		//
		// Sets the name of the explosion.

		if( CEntityParser::m_nFieldCount == 1 ) {
			// Explosion = <None | name>:

			CEntityParser::Interpret_String( &m_ExplosionBuilder.apszName[0], TRUE, 0, TRUE );
			m_ExplosionBuilder.apszName[1] = NULL;

		} else if( CEntityParser::m_nFieldCount == 2 ) {
			// Explosion = <None | name>, ptack:

			if( !CEntityParser::Interpret_StringArray( m_ExplosionBuilder.apszName, 2, 0, TRUE ) ) {
				m_ExplosionBuilder.apszName[0] = NULL;
				m_ExplosionBuilder.apszName[1] = NULL;
			}

		} else {
			CEntityParser::Error_InvalidParameterCount();
		}

		return TRUE;

	} else if( !fclib_stricmp( CEntityParser::m_pszTableName, "Damage" ) ) {
		// Damage = <None | name>
		//
		// Sets the name of the damage profile used to override the damage profile in the explosion set.

		CEntityParser::Interpret_String( &m_pszDamageProfile, TRUE, 0, TRUE );

		return TRUE;

	} else if( !fclib_stricmp( CEntityParser::m_pszTableName, "DamageOnly" ) ) {
		// if set, object can only be hurt by a damage profile of the specifed name

		CEntityParser::Interpret_String( &m_pszDamageOnlyProfile, TRUE, 0, TRUE );

		return TRUE;

	} else if( !fclib_stricmp( CEntityParser::m_pszTableName, "SoundBoom" ) ) {
		// BoomSound = <None | name>
		//
		// Sets the name of the sound group used if there is no explosion specified.

		CEntityParser::Interpret_String( &m_pszSoundGroupBoom, TRUE, 0, TRUE );

		return TRUE;

	} else if( !fclib_stricmp( CEntityParser::m_pszTableName, "SoundAlive" ) ) {
		// BoomSound = <None | name>
		//
		// Sets the name of the sound group used if there is no explosion specified.

		CEntityParser::Interpret_String( &m_pszSoundGroupAlive, TRUE, 0, TRUE );

		return TRUE;

	} else if( !fclib_stricmp( CEntityParser::m_pszTableName, "SoundDead" ) ) {
		// BoomSound = <None | name>
		//
		// Sets the name of the sound group used if there is no explosion specified.

		CEntityParser::Interpret_String( &m_pszSoundGroupDead, TRUE, 0, TRUE );

		return TRUE;

	} else if( !fclib_stricmp( CEntityParser::m_pszTableName, "Particle1" ) ) {
		// Particle1 = Duration, <None | name> [, ptack]
		//
		// Sets the name of a particle.

		_InterpretParticle( 0 );
		return TRUE;

	} else if( !fclib_stricmp( CEntityParser::m_pszTableName, "Particle2" ) ) {
		// Particle2 = Duration, <None | name> [, ptack]
		//
		// Sets the name of a particle.

		_InterpretParticle( 1 );
		return TRUE;

	} else if( !fclib_stricmp( CEntityParser::m_pszTableName, "Particle3" ) ) {
		// Particle3 = Duration, <None | name> [, ptack]
		//
		// Sets the name of a particle.

		_InterpretParticle( 2 );
		return TRUE;

	} else if( !fclib_stricmp( CEntityParser::m_pszTableName, "Particle4" ) ) {
		// Particle4 = Duration, <None | name> [, ptack]
		//
		// Sets the name of a particle.

		_InterpretParticle( 3 );
		return TRUE;

	} else if( !fclib_stricmp( CEntityParser::m_pszTableName, "DetPackOnly" ) ) {
		// DetPackOnly = <True | False>
		//
		// When TRUE, only hitpoint damage from det packs will be recognized.

		CEntityParser::Interpret_BOOL( &m_bDetPackOnly );

		return TRUE;

	} else if( !fclib_stricmp( CEntityParser::m_pszTableName, "VehicleCollide" ) ) {
		// DetPackOnly = <True | False>
		//
		// When TRUE, vehicles will collide with object and will not do damage to it.

		CEntityParser::Interpret_BOOL( &m_bVehicleCollide );

		return TRUE;

	} else if( !fclib_stricmp( CEntityParser::m_pszTableName, "VehicleSlowdown" ) ) {
		// percentage amount to reduce vehicle velocity when colliding with this object
		CEntityParser::Interpret_S32( &m_nVehicleSlowdown, 0, 100, TRUE );

		return TRUE;

	} else if( !fclib_stricmp( CEntityParser::m_pszTableName, "ZombieBossBallOnly" ) ) {
		// ZombieBossBallOnly = <True | False>
		//
		// When TRUE, only hitpoint damage from zombie boss ball will be recognized.

		CEntityParser::Interpret_BOOL( &m_bZombieBossBallOnly);

		return TRUE;

	} else if( !fclib_stricmp( CEntityParser::m_pszTableName, "DeadMeshTexFlipID" ) ) {
		// DeadMeshTexFlipID = <# from 1 to 127>
		// DeadMeshTexFlipID = 0 to disable.
		//
		// When the dead mesh is displayed, one complete cycle through the texture flip will be made.

		CEntityParser::Interpret_U32( &m_nDeadMeshTexFlipID, 0, 127 );

		return TRUE;
	}

	// We don't understand this command. Pass it on...

	return CMeshEntityBuilder::InterpretTable();
}


void CEBoomerBuilder::_InterpretParticle( u32 nParticleIndex ) {
	ParticleBuilder_t *pParticleBuilder = &m_aParticleBuilder[nParticleIndex];
	BOOL bSuccess = FALSE;

	FASSERT( nParticleIndex < CEBoomer::MAX_PARTICLE_COUNT );

	if( CEntityParser::Interpret_F32Array( &pParticleBuilder->fDuration, 1, 0.001f, 60.0f, TRUE, 0 ) ) {
		if( CEntityParser::Interpret_StringArray( pParticleBuilder->apszName, 2, 1, TRUE ) ) {
			bSuccess = TRUE;
		}
	}

	if( !bSuccess ) {
		pParticleBuilder->apszName[0] = NULL;
		pParticleBuilder->apszName[1] = NULL;
		pParticleBuilder->fDuration = 1.0f;
	}
}


BOOL CEBoomerBuilder::PostInterpretFixup( void ) {
	if( !CMeshEntityBuilder::PostInterpretFixup() ) {
		goto _ExitWithError;
	}

	return TRUE;

_ExitWithError:
	return FALSE;
}




//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CEBoomer
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

s32 CEBoomer::m_nDestructEvent;


CEBoomer::CEBoomer() {
	_ClearDataMembers();
}


CEBoomer::~CEBoomer() {
	if( IsSystemInitialized() && IsCreated() ) {
		DetachFromParent();
		DetachAllChildren();
		RemoveFromWorld( TRUE );
		ClassHierarchyDestroy();
	}
}

BOOL CEBoomer::InitLevel( void ) {
	m_nDestructEvent = CFScriptSystem::GetEventNumFromName( "destruct" );

	if( m_nDestructEvent < 0 ) {
		DEVPRINTF( "CEBoomer::InitLevel() : Could not find 'destruct' event.\n" );
	}

	return TRUE;
}


void CEBoomer::UninitLevel( void ) {
}


CEntityBuilder *CEBoomer::GetLeafClassBuilder( void ) {
	return &_EBoomerBuilder;
}


BOOL CEBoomer::ClassHierarchyBuild( void ) {
	FASSERT( IsSystemInitialized() );
	FASSERT( !IsCreated() );
	FASSERT( FWorld_pWorld );

	u32 i;

	// Get a frame...
	FResFrame_t ResFrame = fres_GetFrame();

	// Get pointer to the leaf class's builder object...
	CEBoomerBuilder *pBuilder = (CEBoomerBuilder *)GetLeafClassBuilder();

	// Build parent class...
	if( !CMeshEntity::ClassHierarchyBuild() ) {
		// Parent class could not be built...
		goto _ExitWithError;
	}

	// Set defaults...
	_ClearDataMembers();

	// Initialize from builder object...

	if( pBuilder->m_nDeadMeshTexFlipID ) {
		// Tex-flip ID provided...

		if( GetMeshCount() > 1 ) {
			// A dedicated dead mesh is provided...

			m_hDeadMeshTexFlip = GetMeshInst( 1 )->GetTexLayerHandle( pBuilder->m_nDeadMeshTexFlipID );

			_SetupTexLayerState();
		}
	}

	if( pBuilder->m_bDetPackOnly ) {
		FMATH_SETBITMASK( m_nBoomerFlags, BOOMER_FLAG_DETPACK_ONLY );
	}

	if( pBuilder->m_bZombieBossBallOnly ) {
		FMATH_SETBITMASK( m_nBoomerFlags, BOOMER_FLAG_ZOMBIEBALL_ONLY);
	}

	if( pBuilder->m_bVehicleCollide ) {
		FMATH_SETBITMASK( m_nBoomerFlags, BOOMER_FLAG_VEHICLE_COLLIDE );
	}

	if( pBuilder->m_pszDamageProfile ) {
		m_pDamageProfile = CDamage::FindDamageProfile( pBuilder->m_pszDamageProfile );
	}

	if( pBuilder->m_pszDamageOnlyProfile ) {
		m_pDamageOnlyProfile = CDamage::FindDamageProfile( pBuilder->m_pszDamageOnlyProfile );
	}

	// unit slowdown to apply to vehicle when it hits this object, (converted from builder's 0 to 100 integer value)
	m_fVehicleSlowdown = (f32)pBuilder->m_nVehicleSlowdown * 0.01f;

	m_pSoundGroupBoom = CFSoundGroup::RegisterGroup( pBuilder->m_pszSoundGroupBoom );
	m_pSoundGroupAlive = CFSoundGroup::RegisterGroup( pBuilder->m_pszSoundGroupAlive );
	m_pSoundGroupDead = CFSoundGroup::RegisterGroup( pBuilder->m_pszSoundGroupDead );

	m_hExplosion = fexplosion_GetExplosionGroup( pBuilder->m_ExplosionBuilder.apszName[0] );
	m_nExplosionAliveMeshBoneIndex = GetMeshInst( 0 )->FindBone( pBuilder->m_ExplosionBuilder.apszName[1] );

	if( (GetMeshCount() >= 2) && GetMeshInst( 1 ) ) {
		// A dead mesh was specified...

		for( i=0; i<MAX_PARTICLE_COUNT; ++i ) {
			m_aParticleInfo[i].hParticleDef = (FParticle_DefHandle_t *)fresload_Load( FPARTICLE_RESTYPE, pBuilder->m_aParticleBuilder[i].apszName[0] );
			m_aParticleInfo[i].nDeadMeshBoneIndex = GetMeshInst( 1 )->FindBone( pBuilder->m_aParticleBuilder[i].apszName[1] );
			m_aParticleInfo[i].fDuration = pBuilder->m_aParticleBuilder[i].fDuration;
		}
	}

	// Success...

	return TRUE;

	// Error:
_ExitWithError:
	Destroy();
	fres_ReleaseFrame( ResFrame );
	return FALSE;
}


void CEBoomer::_SetupTexLayerState( void ) {
	
	//ARG - GOTO problem
	u32 nPageCount;

	if( m_hDeadMeshTexFlip == FMESH_TEXLAYERHANDLE_INVALID ) {
		// No texture flip handle provided...
		goto _ExitWithNoFlippingSupport;
	}

	if( GetMeshCount() == 1 ) {
		// Only 1 mesh. No flipping support...
		goto _ExitWithNoFlippingSupport;
	}

	nPageCount = GetMeshInst( 1 )->TexFlip_GetFlipPageCount( m_hDeadMeshTexFlip );

	if( nPageCount <= 1 ) {
		// Must be at least two pages to flip...
		goto _ExitWithNoFlippingSupport;
	}

	u32 nFlipPage;

	if( NormHealth() > 0.0f ) {
		nFlipPage = 0;
	} else {
		nFlipPage = nPageCount - 1;
	}

	// Reset flipping on dead mesh...
	GetMeshInst( 1 )->TexFlip_AnimateFlip( m_hDeadMeshTexFlip, FALSE );
	GetMeshInst( 1 )->TexFlip_SetFlipPage( m_hDeadMeshTexFlip, nFlipPage );
	GetMeshInst( 1 )->TexFlip_ClampAndStop( m_hDeadMeshTexFlip, TRUE );
	FMATH_SETBITMASK( GetMeshInst( 1 )->m_nFlags, FMESHINST_FLAG_SAMPLE_ANIMLAYER_TICKS );

	if( GetMeshInst( 0 )->m_pMesh == GetMeshInst( 1 )->m_pMesh ) {
		// Alive mesh is the same as the dead mesh. Don't texture flip on alive mesh...

		GetMeshInst( 0 )->TexFlip_AnimateFlip( m_hDeadMeshTexFlip, FALSE );
		GetMeshInst( 0 )->TexFlip_SetFlipPage( m_hDeadMeshTexFlip, nFlipPage );
		FMATH_SETBITMASK( GetMeshInst( 0 )->m_nFlags, FMESHINST_FLAG_SAMPLE_ANIMLAYER_TICKS );
	}

	// Flipping supported...

	return;

_ExitWithNoFlippingSupport:
	m_hDeadMeshTexFlip = FMESH_TEXLAYERHANDLE_INVALID;
	return;
}


BOOL CEBoomer::ClassHierarchyBuilt( void ) {
	FASSERT( IsSystemInitialized() );
	FASSERT( IsCreated() );

	FResFrame_t ResFrame = fres_GetFrame();

	if( !CEntity::ClassHierarchyBuilt() ) {
		goto _ExitWithError;
	}

	SelectMesh( 0, TRUE );
	MeshFlip_SetFlipsPerSec( 0.0f );

	m_pAudioEmitter = CFSoundGroup::AllocAndPlaySound( m_pSoundGroupAlive, FALSE, &MtxToWorld()->m_vPos );

	DisableOurWorkBit();

	return TRUE;

_ExitWithError:
	Destroy();
	fres_ReleaseFrame( ResFrame );
	return FALSE;
}


void CEBoomer::ClassHierarchyDestroy( void ) {
	_ClearDataMembers();

	CMeshEntity::ClassHierarchyDestroy();
}


void CEBoomer::_ClearDataMembers( void ) {
	u32 i;

	m_hDeadMeshTexFlip = FMESH_TEXLAYERHANDLE_INVALID;

	m_nBoomerFlags = BOOMER_FLAG_NONE;

	m_pDamageProfile = NULL;
	m_pDamageOnlyProfile = NULL;

	m_pSoundGroupBoom = NULL;
	m_pSoundGroupAlive = NULL;
	m_pSoundGroupDead = NULL;

	m_hExplosion = FEXPLOSION_INVALID_HANDLE;
	m_nExplosionAliveMeshBoneIndex = -1;

	for( i=0; i<MAX_PARTICLE_COUNT; ++i ) {
		m_aParticleInfo[i].hParticleDef = FPARTICLE_INVALID_HANDLE;
		m_aParticleInfo[i].nDeadMeshBoneIndex = -1;
		m_aParticleInfo[i].fDuration = 1.0f;
	}

	m_pAudioEmitter = NULL;
}


void CEBoomer::ClassHierarchyRelocated( void *pIdentifier ) {
	FASSERT( IsCreated() );

	CMeshEntity::ClassHierarchyRelocated( pIdentifier );

	if( m_pAudioEmitter ) {
		m_pAudioEmitter->SetPosition( &MtxToWorld()->m_vPos );
	}
}


void CEBoomer::CheckpointRestore( void ) {
	CMeshEntity::CheckpointRestore();

	if( m_pAudioEmitter ) {
		m_pAudioEmitter->Destroy();
		m_pAudioEmitter = NULL;
	}

	if( GetCurrentMeshIndex() ) {
		m_pAudioEmitter = CFSoundGroup::AllocAndPlaySound( m_pSoundGroupDead, FALSE, &MtxToWorld()->m_vPos );
	} else {
		m_pAudioEmitter = CFSoundGroup::AllocAndPlaySound( m_pSoundGroupAlive, FALSE, &MtxToWorld()->m_vPos );
	}

	_SetupTexLayerState();
}


void CEBoomer::InflictDamage( CDamageData *pDamageData ) {
	if( IsDetPackOnly() ) {
		// Apply hitpoints only if from detpack...

		if( (pDamageData->m_Damager.pEntity == NULL) || !(pDamageData->m_Damager.pEntity->TypeBits() & ENTITY_BIT_DETPACKDROP) ) {
			// This is not a detpack. Issue zero-hitpoint damage...

			CDamageProfile NewDamageProfile;
			u32 i;

			// Init new damage profile with no hitpoints...
			fang_MemCopy( &NewDamageProfile, pDamageData->m_pDamageProfile, sizeof(CDamageProfile) );

			for( i=0; i<DAMAGE_HITPOINT_TYPE_COUNT; ++i ) {
				NewDamageProfile.m_afUnitHitpoints[i] = 0.0f;
				pDamageData->m_afDeltaHitpoints[i] = 0.0f;
			}

			NewDamageProfile.m_HitpointRange.m_fInnerValue = 0.0f;
			NewDamageProfile.m_HitpointRange.m_fOuterValue = 0.0f;
			NewDamageProfile.m_abZeroInnerAndOuterValues[0] = TRUE;

			pDamageData->m_pDamageProfile = &NewDamageProfile;

			// Inflict damage, sans hitpoints...
			CMeshEntity::InflictDamage( pDamageData );

			return;
		}
	} else if( m_nBoomerFlags & BOOMER_FLAG_ZOMBIEBALL_ONLY) {
		// Apply hitpoints only if from ZOMBIEBALL...

		if( (pDamageData->m_Damager.pEntity == NULL) || !(pDamageData->m_Damager.pEntity->TypeBits() & ENTITY_BIT_BOTZOMBIEBOSS) ) {
			// This is not a zombieball. Issue zero-hitpoint damage...
			CDamageProfile NewDamageProfile;
			u32 i;

			// Init new damage profile with no hitpoints...
			fang_MemCopy( &NewDamageProfile, pDamageData->m_pDamageProfile, sizeof(CDamageProfile) );

			for( i=0; i<DAMAGE_HITPOINT_TYPE_COUNT; ++i ) {
				NewDamageProfile.m_afUnitHitpoints[i] = 0.0f;
				pDamageData->m_afDeltaHitpoints[i] = 0.0f;
			}

			NewDamageProfile.m_HitpointRange.m_fInnerValue = 0.0f;
			NewDamageProfile.m_HitpointRange.m_fOuterValue = 0.0f;
			NewDamageProfile.m_abZeroInnerAndOuterValues[0] = TRUE;

			pDamageData->m_pDamageProfile = &NewDamageProfile;

			// Inflict damage, sans hitpoints...
			CMeshEntity::InflictDamage( pDamageData );

			return;
		}
	} else if( m_pDamageOnlyProfile ) {
		// if damage-only profile is set, only allow object to be damaged if profiles match.
		if( pDamageData->m_pDamageProfile != m_pDamageOnlyProfile ) {
			return;
		}
	}

	// Just pass along the damage as-is...
	CMeshEntity::InflictDamage( pDamageData );
}


void CEBoomer::Die( BOOL bSpawnDeathEffects, BOOL bSpawnGoodies ) {
	BOOL bFinalDeath = FALSE;

	CMeshEntity::Die( bSpawnDeathEffects, bSpawnGoodies );

	if( GetCurrentMeshIndex() ) {
		// We've already switched to destroyed mesh...

		if( m_pVerlet ) {
			if( m_pVerlet->IsExplodeCallbackBeingCalled() ) {
				// We're being called because the Verlet object's ctack has passed through a collision surface.
				// Let's explode the object again and remove it from the world once and for all...
				bFinalDeath = TRUE;
			}
		}

		if( !bFinalDeath ) {
			return;
		}
	}

	SetNormHealth( 0.0f );
	SetTargetable( FALSE );

	ParticleInfo_t *pParticleInfo;
	CEParticle *pEParticle;
	CFMtx43A *pBoneMtx;
	u32 i;

	if( bSpawnDeathEffects ) {
		// Spawn explosion...
		if( m_hExplosion != FEXPLOSION_INVALID_HANDLE ) {
			CDamageForm::Damager_t Damager;

			FExplosion_SpawnerHandle_t hSpawner = CExplosion2::GetExplosionSpawner();

			if( hSpawner != FEXPLOSION_INVALID_HANDLE ) {
				FExplosionSpawnParams_t SpawnParams;

				SpawnParams.InitToDefaults();

				SpawnParams.uFlags = FEXPLOSION_SPAWN_NONE;
				SpawnParams.pDamageProfile = m_pDamageProfile;

				Damager.nDamagerPlayerIndex = -1;
				Damager.pBot = NULL;
				Damager.pEntity = this;
				Damager.pWeapon = NULL;

				SpawnParams.pDamager = &Damager;

				if( m_nExplosionAliveMeshBoneIndex < 0 ) {
					// Use bounding sphere center as epicenter...

					SpawnParams.Pos_WS.Set( GetMeshInst()->GetBoundingSphere().m_Pos );
					SpawnParams.UnitDir = CFVec3A::m_UnitAxisY;
					SpawnParams.uSurfaceType = 0;
				} else {
					// Use specified bone as epicenter and direction...

					pBoneMtx = GetMeshInst()->GetBoneMtxPalette()[ m_nExplosionAliveMeshBoneIndex ];

					SpawnParams.Pos_WS = pBoneMtx->m_vPos;
					SpawnParams.UnitDir.ReceiveUnit( pBoneMtx->m_vZ );
					SpawnParams.uSurfaceType = 0;
				}

				CExplosion2::SpawnExplosion( hSpawner, m_hExplosion, &SpawnParams );
			}
		} else if( m_pDamageProfile ) {
			CDamageForm *pDamageForm = CDamage::GetEmptyDamageFormFromPool();

			if( pDamageForm ) {
				// Fill out the form...

				pDamageForm->m_nDamageLocale = CDamageForm::DAMAGE_LOCALE_BLAST;
				pDamageForm->m_nDamageDelivery = CDamageForm::DAMAGE_DELIVERY_ALL_ENTITIES_WITHIN_PROFILE_RADIUS;
				pDamageForm->m_pDamageProfile = m_pDamageProfile;
				pDamageForm->m_Damager.pEntity = this;

				if( m_nExplosionAliveMeshBoneIndex < 0 ) {
					// Use bounding sphere center as epicenter...

					pDamageForm->m_Epicenter_WS.Set( GetMeshInst()->GetBoundingSphere().m_Pos );
				} else {
					// Use specified bone as epicenter and direction...

					pBoneMtx = GetMeshInst()->GetBoneMtxPalette()[ m_nExplosionAliveMeshBoneIndex ];

					pDamageForm->m_Epicenter_WS = pBoneMtx->m_vPos;
				}

				CDamage::SubmitDamageForm( pDamageForm );
			}
		}

		if( !bFinalDeath ) {
			// Spawn sounds...
			CFSoundGroup::PlaySound( m_pSoundGroupBoom, FALSE, &MtxToWorld()->m_vPos, -1, TRUE );

			if( m_pAudioEmitter ) {
				m_pAudioEmitter->Destroy();
			}

			m_pAudioEmitter = CFSoundGroup::AllocAndPlaySound( m_pSoundGroupDead, FALSE, &MtxToWorld()->m_vPos );
		}
	}

	// Swap meshes...
	if( bFinalDeath ) {
		RemoveFromWorld();
	} else {
		if( GetMeshCount() >= 2 ) {
			SelectMesh( 1 );
			AddToWorld();
		} else {
			// Destroy all verlet wires...
			if( bSpawnDeathEffects ) {
				if( m_pVerlet ) {
					CFVerletTack *pTack;

					for( i=0; i<m_pVerlet->Tack_GetCount(); ++i ) {
						pTack = m_pVerlet->Tack_GetFromIndex(i);

						pTack->Anchor_SetUnitHealth( 0.0f, FALSE );
					}
				}
			}

			RemoveFromWorld();
		}

		// Swap animations...
		if( UserAnim_GetCount() >= 2 ) {
			UserAnim_Select( 1 );
		} else {
			DriveMeshWithAnim( FALSE );
		}

		if( m_hDeadMeshTexFlip != FMESH_TEXLAYERHANDLE_INVALID ) {
			// A dead-mesh texture flip is requested...

			GetMeshInst()->TexFlip_AnimateFlip( m_hDeadMeshTexFlip, TRUE );
			FMATH_SETBITMASK( GetMeshInst()->m_nFlags, FMESHINST_FLAG_SAMPLE_ANIMLAYER_TICKS );
		}

		if( bSpawnDeathEffects && (GetMeshCount() >= 2) ) {
			// Spawn particles...

			CFMtx43A Mtx;

			Mtx.m_vX = CFVec3A::m_UnitAxisX;
			Mtx.m_vY.Set( 0.0f, 0.0f, -1.0f );
			Mtx.m_vZ = CFVec3A::m_UnitAxisY;

			for( i=0; i<MAX_PARTICLE_COUNT; ++i ) {
				pParticleInfo = m_aParticleInfo + i;

				if( pParticleInfo->hParticleDef == FPARTICLE_INVALID_HANDLE ) {
					continue;
				}

				pEParticle = eparticlepool_GetEmitter();

				if( pEParticle ) {
					pEParticle->StartEmission( pParticleInfo->hParticleDef, 1.0f, pParticleInfo->fDuration, TRUE );

					if( pParticleInfo->nDeadMeshBoneIndex < 0 ) {
						// Use bounding sphere center as epicenter...

						Mtx.m_vPos.Set( GetMeshInst()->GetBoundingSphere().m_Pos );

						pEParticle->Relocate_RotXlatFromUnitMtx_WS_NewScale_WS( &Mtx, 1.0f, FALSE );
						pEParticle->Attach_ToParent_WS( this );
					} else {
						// Use specified bone as epicenter and direction...

						pEParticle->Relocate_RotXlatFromUnitMtx_PS( &CFMtx43A::m_IdentityMtx, FALSE );
						pEParticle->Attach_ToParent_PS( this, GetMeshInst()->m_pMesh->pBoneArray[ pParticleInfo->nDeadMeshBoneIndex ].szName, FALSE );
					}
				}
			}
		}
	}

	if( m_nDestructEvent >= 0 ) {
		CFScriptSystem::TriggerEvent( m_nDestructEvent, 0, (u32)(this), 0 );
	}
}


