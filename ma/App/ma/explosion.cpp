//////////////////////////////////////////////////////////////////////////////////////
// explosion.cpp - 
//
// Author: Michael Starich   
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
// 08/05/01 Starich     Created.
//////////////////////////////////////////////////////////////////////////////////////
#include "fang.h"
#include "explosion.h"
#include "gcoll.h"
#include "damage.h"

BOOL CExplosion2::m_bSystemInitialized = FALSE;

BOOL CExplosion2::InitSystem( void ) {
	FASSERT( !IsSystemInitialized() );

	m_bSystemInitialized = TRUE;

	fexplosion_SetSurfaceDebrisCallback( _ExplosionDebrisCallback );
	fexplosion_SetSurfaceDustTextureCallback( _ExplosionGetDustTexInstCallback );
	fexplosion_SetDamageCallback( _ExplosionDamageCallback );

	return TRUE;
}


void CExplosion2::UninitSystem( void ) {
	if( IsSystemInitialized() ) {
		m_bSystemInitialized = FALSE;
		fexplosion_SetSurfaceDebrisCallback( NULL );
		fexplosion_SetSurfaceDustTextureCallback( NULL );
	}
}

// We MUST set m_pDebrisGroup here!!
void CExplosion2::_ExplosionDebrisCallback( CFDebrisSpawner *pSpawner, FExplosionSurfceDebrisSize_e eDebrisSize, u16 uSurfaceType ) {
	const CGCollMaterial *pMat = CGColl::GetMaterial( uSurfaceType );

	switch( eDebrisSize ) {
		case FEXPLOSION_SIZE_SMALL:
			pSpawner->m_pDebrisGroup = pMat->m_apDebrisGroup[ CGCollMaterial::DEBRIS_GROUP_SMALL ];
		break;
		case FEXPLOSION_SIZE_MEDIUM:
			pSpawner->m_pDebrisGroup = pMat->m_apDebrisGroup[ CGCollMaterial::DEBRIS_GROUP_MEDIUM ];
		break;
		case FEXPLOSION_SIZE_LARGE:
			pSpawner->m_pDebrisGroup = pMat->m_apDebrisGroup[ CGCollMaterial::DEBRIS_GROUP_LARGE ];
		break;
	}

	CGColl::SpawnDebris( pSpawner );
}

const CFTexInst *CExplosion2::_ExplosionGetDustTexInstCallback( u16 uSurfaceType ) {
	const CGCollMaterial *pMat = CGColl::GetMaterial( uSurfaceType );

	return &pMat->m_ExplosionDustTexInst;
}

void CExplosion2::_ExplosionDamageCallback( const void *pDamage, const void *pDamager, const CFVec3A *pPos, const void *pDamageImpactTriData/* = NULL*/ ) {
	CDamageProfile *pDamageProfile = (CDamageProfile *) pDamage;
	
	if( pDamageProfile == NULL ) {
		return;
	}
	
	CDamageForm *pDamageForm = CDamage::GetEmptyDamageFormFromPool();
	
	if( pDamageForm ) {
		pDamageForm->m_nDamageLocale = CDamageForm::DAMAGE_LOCALE_BLAST;
		pDamageForm->m_nDamageDelivery = CDamageForm::DAMAGE_DELIVERY_ALL_ENTITIES_WITHIN_PROFILE_RADIUS;
		pDamageForm->m_pDamageProfile = pDamageProfile;
		pDamageForm->m_Epicenter_WS = *pPos;
		
		if( pDamager ) {
			pDamageForm->m_Damager = *( (CDamageForm::Damager_t *) pDamager );
		}

		if( pDamageImpactTriData ) {
			fang_MemCopy( (void *) &pDamageForm->m_TriData, pDamageImpactTriData, sizeof( CDamageForm::TriData_t ) );
		}

		CDamage::SubmitDamageForm( pDamageForm );
	}
}




