//////////////////////////////////////////////////////////////////////////////////////
// EBotFire.cpp - Catch me botty on flame-style
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
// 01.12.03 Scholz		Created.
//////////////////////////////////////////////////////////////////////////////////////


#include "ebotfire.h"
#include "fsndfx.h"
#include "fresload.h"
#include "bot.h"

BOOL		CEBotFire::m_bSystemInitialized;
CEBotFire*	CEBotFire::m_pPoolFreeEBotFire;
FLinkRoot_t CEBotFire::m_RootFreeEBotFire;
CEBotFire*	CEBotFire::m_pPoolUsedEBotFire;
FLinkRoot_t CEBotFire::m_RootUsedEBotFire;

cchar* CEBotFire::m_apszPartNames[_EBOTFIRE_NUM_PARTICLE_FX] =
{
	"flame1",
	"smoke1"
};

FParticle_DefHandle_t CEBotFire::m_ahParticleDefs[_EBOTFIRE_NUM_PARTICLE_FX];

BOOL CEBotFire::_InitParticles( void )
{
	for( u32 i=0; i<_EBOTFIRE_NUM_PARTICLE_FX; i++ )
	{
		// Particle effects
		m_ahParticleDefs[i] = (FParticle_DefHandle_t) fresload_Load( FPARTICLE_RESTYPE, m_apszPartNames[i] );
		if( m_ahParticleDefs[i] == FPARTICLE_INVALID_HANDLE ) 
		{
			DEVPRINTF( "CEBotFire::ClassHierarchyBuild(): Could not find particle definition '%s'.\n", m_apszPartNames[i]);
			return FALSE;
		}
	}
	return TRUE;
}

void CEBotFire::_InitPools( void ) 
{
	FASSERT( m_bSystemInitialized );

	flinklist_InitRoot( &m_RootFreeEBotFire, FANG_OFFSETOF( CEBotFire, m_Link ) );
	flinklist_InitRoot( &m_RootUsedEBotFire, FANG_OFFSETOF( CEBotFire, m_Link ) );

	flinklist_InitPool( &m_RootFreeEBotFire, m_pPoolFreeEBotFire, sizeof(CEBotFire), _EBOTFIRE_POOL_SIZE );
	flinklist_InitPool( &m_RootFreeEBotFire, m_pPoolUsedEBotFire, sizeof(CEBotFire), 0);

	for( u32 i=0; i<_EBOTFIRE_POOL_SIZE; ++i ) 
	{
		m_pPoolFreeEBotFire[i].Create();
		m_pPoolFreeEBotFire[i].m_uListMember = LIST_MEMBER_FREE_POOL;
	}
}

void CEBotFire::_UninitPools( void ) 
{
	FASSERT( m_bSystemInitialized );

	for( u32 i=0; i<_EBOTFIRE_POOL_SIZE; ++i ) 
	{
		m_pPoolFreeEBotFire[i].Destroy();
	}
}


BOOL CEBotFire::InitSystem( void ) 
{
	FASSERT( !m_bSystemInitialized );

	m_bSystemInitialized = TRUE;

	FResFrame_t ResFrame = fres_GetFrame();

	// Build our pools...
	m_pPoolFreeEBotFire = fnew CEBotFire[_EBOTFIRE_POOL_SIZE];
	if( m_pPoolFreeEBotFire == NULL ) 
	{
		DEVPRINTF( "CEBotFire::InitSystem(): Not enough memory to allocate pool of CEBotFire objects.\n" );
		goto _ExitWithError;
	}
	
	fworld_RegisterWorldCallbackFunction( _WorldCallback );

	// Init our particles...
	_InitParticles();

	return TRUE;

	// Error:
_ExitWithError:
	UninitSystem();
	fres_ReleaseFrame( ResFrame );
	return TRUE;
}
void CEBotFire::UninitSystem( void ) 
{
	if( m_bSystemInitialized ) 
	{
		fdelete_array( m_pPoolFreeEBotFire );

		fworld_UnregisterWorldCallbackFunction( _WorldCallback );
		
		m_pPoolFreeEBotFire = NULL;
		m_pPoolUsedEBotFire = NULL;
		m_bSystemInitialized = FALSE;
	}
}
// Returns a pointer to an free initialized CEBotFire instance.
// Returns NULL if it could not be retrieved.
CEBotFire* CEBotFire::GetFreeEBotFireFromPool( void )
{
	FASSERT( m_bSystemInitialized );

	// Get a CEBotFire from the free pool...
	CEBotFire *pBotFire = (CEBotFire *)flinklist_RemoveTail( &m_RootFreeEBotFire );
	if( pBotFire == NULL ) 
	{
		// No more in the free pool...
		return NULL;
	}
	FASSERT(pBotFire->m_uListMember == LIST_MEMBER_FREE_POOL);
	pBotFire->m_uListMember = LIST_MEMBER_USED_POOL;
	flinklist_AddTail( &m_RootUsedEBotFire, pBotFire );

	// Initialize the CEBotFire to defaults...
	pBotFire->Reset();

	// Return it to caller...
	return pBotFire;
}

void  CEBotFire::Reset( void )
{
	SetAmbientRadius( 20.f );
	SetAmbientVolume( 1.0f );
	SetAmbientSFXHandle( fsndfx_GetFxHandle( "SWDMl1mhit" ) );

	if( GetAmbientSFXHandle() == FSNDFX_INVALID_FX_HANDLE )
	{
		DEVPRINTF( "CEBotFire::Reset(): Unable to get sFX handle for %s.\n", "SWDMl1mhit" );
	}
}

void CEBotFire::ReturnToFreePool(void)
{
	StopAmbientSFX();
	if (m_pDamageForm)
	{
		CDamage::KillDamageForm(m_pDamageForm);
	}
	m_pDamageForm = NULL;

	CEBotFire *pBotFire = (CEBotFire *)flinklist_Remove(&m_RootUsedEBotFire,this);
	FASSERT(pBotFire == this);
	FASSERT(m_uListMember == LIST_MEMBER_USED_POOL);
	
	flinklist_AddTail( &m_RootFreeEBotFire, this );
	m_uListMember = LIST_MEMBER_FREE_POOL;
	
	if (IsCreated())
	{
		if (IsEmitting())
			StopEmission();
		if (IsInWorld())
			RemoveFromWorld();
	}
}

void CEBotFire::EndAll( void )
{
	CEBotFire *pBotFire = NULL;
	while ( pBotFire = (CEBotFire *)flinklist_GetHead( &m_RootUsedEBotFire) )
	{
		pBotFire->ReturnToFreePool();
	}
}

BOOL CEBotFire::ClassHierarchyBuild( void )
{
	return CEParticle::ClassHierarchyBuild();
}

void CEBotFire::ClassHierarchyAddToWorld( void )
{
	CEParticle::ClassHierarchyAddToWorld();
}

void CEBotFire::ClassHierarchyRemoveFromWorld( void )
{
	CEParticle::ClassHierarchyRemoveFromWorld();
	if (m_uListMember == LIST_MEMBER_USED_POOL)
		ReturnToFreePool();
}

void CEBotFire::ClassHierarchyDestroy( void )
{
	CEParticle::ClassHierarchyDestroy();
}
void CEBotFire::ClassHierarchyWork( void )
{
	CEParticle::ClassHierarchyWork();

	SetAmbientVolume( GetUnitIntensity() );

	if (!IsEmitting())
		ReturnToFreePool();
}

BOOL CEBotFire::_WorldCallback( FWorldEvent_e nEvent ) 
{
	switch( nEvent ) 
	{
	case FWORLD_EVENT_WORLD_POSTLOAD:
		// The world has been loaded. Finish creating all projectile pools
		// that have been built prior to now...

		// Init our pools...
		_InitPools();

		break;

	case FWORLD_EVENT_WORLD_PREDESTROY:
		EndAll();
		// UnInit our pools...
		_UninitPools();
		break;
	}

	// Success...

	return TRUE;

	// Error...
//_ExitWithError:
//	return FALSE;
}

void CEBotFire::Start(CDamageProfile* pDamageProfile, CWeapon* pWeaponImFrom,CBot* pBotToBlame,f32 fDuration)
{
	PlayAmbientSFX();
	StartEmission(GetFlameDef(),1.0,fDuration,TRUE);
	
	if (IsEmitting() == FALSE)
	{
		DEVPRINTF("CEBotFire::Start() No visible flames, failure to emit particles\n");
	}
	if (!pDamageProfile)
        pDamageProfile = CDamage::FindDamageProfile("MagmaFireAttached1",TRUE,TRUE);

	if (!pDamageProfile)
	{
		DEVPRINTF("Attached Fire damage profile not found");
		return;
	}
	
	m_pDamageForm = CDamage::GetEmptyDamageFormFromPool();
	if (!m_pDamageForm)
	{
		DEVPRINTF("FAILEd to get damage form");
		return;
	}

	m_pDamageForm->m_nDamageLocale = CDamageForm::DAMAGE_LOCALE_AMBIENT;
	m_pDamageForm->m_nDamageDelivery = CDamageForm::DAMAGE_DELIVERY_ONE_SPECIFIC_ENTITY;
	m_pDamageForm->m_pDamageeEntity = this->GetParent();
	m_pDamageForm->m_pDamageProfile = pDamageProfile;
	m_pDamageForm->m_fNormIntensity = 1.0f;
	m_pDamageForm->m_Damager.pBot = pBotToBlame;
	m_pDamageForm->m_Damager.pWeapon = pWeaponImFrom;
	m_pDamageForm->m_Damager.pEntity = this;
	FASSERT(pBotToBlame); // I don't believe this function gets called w/ NULLs
	m_pDamageForm->m_Damager.nDamagerPlayerIndex = pBotToBlame->m_nPossessionPlayerIndex; // the possessor of blamebot gets the kill.

	m_pDamageForm->m_Epicenter_WS.Set(this->m_MtxToWorld.m_vPos);

	CDamage::SubmitDamageForm(m_pDamageForm, -1.0f); // damage's until cancelled
}