//////////////////////////////////////////////////////////////////////////////////////
// EShield.cpp	- bot shields
//
// Author: Mike Elliott
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
// 11.19.02 Elliott		Created.
//////////////////////////////////////////////////////////////////////////////////////


#include "EShield.h"
#include "meshtypes.h"
#include "fworld.h"
#include "floop.h"
#include "fresload.h"
#include "ftext.h"
#include "AI/AIBrainman.h"
#include "Bot.h"
#include "gamecam.h"
#include "fworld_coll.h"
#include "sas_user.h"

//#define _DEFAULT_SCALE			1.0f
#define _DEFAULT_RECHARGETIME	1.0f
#define _DEFAULT_RECHARGEDELAY	5.0f

#define	_SHIELD_MESH		"gf_shield02"
#define _SPLASH_MESH		"gf_shield01"
#define	_SHIELD_CULLDIST	300.0f
#define _SPLASH_PUSH_MIN	1.1f
#define _SPLASH_PUSH_MAX	1.2f
#define _SHIELD_TEXLAYER	50
#define _SPLASH_TEXLAYER	50
#define _EFFECT_TIMESCALE	(1.0f/0.75f)
#define _EFFECT_ON_OO_TIME	(1.0f/2.0f)
#define _EFFECT_OFF_OO_TIME	(1.0f/0.5f)
#define _DEBUG_VISIBLE		1
#define _MAX_SPLASHES		20

#define _SPLASH_INITIAL_ALPHA	0.5f
#define _SPLASH_FINAL_ALPHA		0.02f


#define _ALPHA_ON			0.05f
#define _ALPHA_SHIELD_HIT	0.5f
#define _ALPHA_FADE_RATE	0.5f	// 1/2

#define _FLICKER_MIN_INTERVAL	0.05f
#define _FLICKER_MAX_INTERVAL	0.1f

#define _FLICKER_MIN_COUNT		10
#define _FLICKER_MAX_COUNT		15



#define _PUSHABLE_BOTS (ENTITY_BIT_BOTGLITCH | ENTITY_BIT_BOTGRUNT | ENTITY_BIT_BOTMINER)

#define _ORIENT_TOWARD_CAMERA	1

struct _ShieldSplash_t {
	f32			 fUnitTime;				//how long it's been active 1.0 = dead
	CFVec3A		 vDisplacement;			//displacement from center
	CEShield	*pShield;				//which shield we came from
	CFWorldMesh *pMesh;					//our splash worldmesh inst

	FMeshTexLayerHandle_t	hTexLayer;	//the tex layer we're animating

	FLink_t		 link;					//link to other _ShieldSplashes
};


//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CELineBuilder
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

static CEShieldBuilder _EShieldBuilder;


void CEShieldBuilder::SetDefaults( u64 nEntityTypeBits, u64 nEntityLeafTypeBit, cchar *pszEntityType ) {
	ENTITY_BUILDER_SET_PARENT_CLASS_DEFAULTS( CEntityBuilder, ENTITY_BIT_SHIELD, pszEntityType );

	// Shields detonate projectiles by default...
	m_nProjectileReaction = CEntity::PROJECTILE_REACTION_SHIELD;
}


BOOL CEShieldBuilder::InterpretTable( void ) {
	return CEntityBuilder::InterpretTable();
}


BOOL CEShieldBuilder::PostInterpretFixup( void ) {
	if( !CEntityBuilder::PostInterpretFixup() ) {
		goto _ExitWithError;
	}
	return TRUE;

_ExitWithError:
	return FALSE;
}


//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CEShield
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

_ShieldSplash_t *CEShield::m_aSplashes		= NULL;

BOOL		CEShield::m_bSystemInitialized	= FALSE;
BOOL		CEShield::m_bEffectsLoaded		= FALSE;
FLinkRoot_t CEShield::m_FreeSplashList;
CEShield*	CEShield::m_pCollThis			= NULL;
const CFVec3A*	CEShield::m_pPosToDrawSplash	= NULL;


#if SAS_ACTIVE_USER == SAS_USER_ELLIOTT
	u32  CEShield::uHighestSplashOverride = 0;
	u32  CEShield::uCurrentSplashOverride = 0;
#endif



BOOL CEShield::InitSystem( void ) {
	FASSERT( !m_bSystemInitialized );
	m_bSystemInitialized = TRUE;

	FResFrame_t frame = fres_GetFrame();
	FMeshInit_t meshInit;

	m_aSplashes = (_ShieldSplash_t*)fres_AlignedAlloc( _MAX_SPLASHES * sizeof( _ShieldSplash_t ), 16 );
	if( m_aSplashes == NULL ) {
		DEVPRINTF( "CEShield::InitSystem(): Unable to allocate array of splashes\n" );
		goto _ExitInitSystemWithError;
	}

	flinklist_InitRoot( &m_FreeSplashList, FANG_OFFSETOF( _ShieldSplash_t, link ) );

	for( u32 i=0; i<_MAX_SPLASHES; i++ ) {
		m_aSplashes[i].fUnitTime	= 1.0f;
		m_aSplashes[i].pShield		= NULL;
		m_aSplashes[i].pMesh		= NULL;
		m_aSplashes[i].vDisplacement.Zero();
			
		flinklist_AddTail( &m_FreeSplashList, &m_aSplashes[i] );
	}

	return TRUE;

	// Error:
_ExitInitSystemWithError:
	UninitSystem();
	fres_ReleaseFrame( frame );
	return FALSE;

}


void CEShield::UninitSystem( void ) {
	if( !m_bSystemInitialized ) {
		return;
	}

	m_bSystemInitialized = FALSE;

	if( m_aSplashes == NULL ) {
		return;
	}
}


BOOL CEShield::_InitEffects( void ) {
	FASSERT( m_bSystemInitialized );
	FASSERT( !m_bEffectsLoaded );
	m_bEffectsLoaded = TRUE;

	FResFrame_t frame = fres_GetFrame();
	FMeshInit_t meshInit;
	FMesh_t		*pMesh;


	//init all the splashes
	pMesh = (FMesh_t*)fresload_Load( FMESH_RESTYPE, _SPLASH_MESH );
	if( pMesh == NULL ) {
		DEVPRINTF( "CEShield::InitSystem():  Unable to load mesh %s\n", _SPLASH_MESH );
		goto _ExitInitEffectsWithError;
	}

	
	meshInit.fCullDist	= _SHIELD_CULLDIST;
	meshInit.nFlags		= 0;
	meshInit.pMesh		= pMesh;
	meshInit.Mtx.Identity();

	for( u32 i=0; i<_MAX_SPLASHES; i++ ) {
		m_aSplashes[i].fUnitTime	= 1.0f;
		m_aSplashes[i].pShield		= NULL;
		m_aSplashes[i].vDisplacement.Zero();
		m_aSplashes[i].pMesh = fnew CFWorldMesh;
		m_aSplashes[i].pMesh->Init( &meshInit );
		m_aSplashes[i].pMesh->SetCollisionFlag( FALSE );
		m_aSplashes[i].pMesh->m_nUser = NULL;
		m_aSplashes[i].pMesh->m_pUser = NULL;
		m_aSplashes[i].pMesh->SetUserTypeBits( 0 );
		m_aSplashes[i].pMesh->UpdateTracker();

		m_aSplashes[i].hTexLayer = m_aSplashes[i].pMesh->GetTexLayerHandle( _SPLASH_TEXLAYER );
		if( m_aSplashes[i].hTexLayer < 0 ) {
			DEVPRINTF( "CEShield::_InitEffects():  Error getting tex layer handle\n" );
			goto _ExitInitEffectsWithError;
		}

		
		m_aSplashes[i].pMesh->SetCullDirection( FMESH_CULLDIR_NONE );
		m_aSplashes[i].pMesh->AnimTC_AnimateScroll( m_aSplashes[i].hTexLayer, FALSE );

		m_aSplashes[i].pMesh->RemoveFromWorld();
	}

	return TRUE;

	// Error:
_ExitInitEffectsWithError:
	fres_ReleaseFrame( frame );
	return FALSE;
}


void CEShield::_DestroyEffects( void ) {
	FASSERT( m_bSystemInitialized );
	if( !m_bEffectsLoaded ) {
		return;
	}

	m_bEffectsLoaded = FALSE;

	for( u32 i=0; i<_MAX_SPLASHES; i++ ) {
		fdelete( m_aSplashes[i].pMesh );
		m_aSplashes[i].pMesh = NULL;
	}
}


_ShieldSplash_t* CEShield::_GetNextSplash( void ) {
	FASSERT( m_bSystemInitialized );
	_ShieldSplash_t *pSplash;

	pSplash = (_ShieldSplash_t*)flinklist_RemoveHead( &m_FreeSplashList );
	if( pSplash != NULL ) {
		return pSplash;
	}

	f32 fHighestTime = 0.0f;
	u32 uHighestIdx	 = 0;

	for( u32 i=0; i<_MAX_SPLASHES; i++ ) {
		if( m_aSplashes[i].fUnitTime > fHighestTime ) {
			fHighestTime = m_aSplashes[i].fUnitTime;
			uHighestIdx  = i;
		}
	}

	m_aSplashes[uHighestIdx].pShield->_KillSplash( m_aSplashes[uHighestIdx].fUnitTime );		// will kill one and put it on the free list
	pSplash = (_ShieldSplash_t*)flinklist_RemoveHead( &m_FreeSplashList );						// get it and return it

	return pSplash;
}



CEShield::CEShield() : CEntity() {
	m_pWorldMesh		= NULL;
	m_hTexLayer			= FMESH_TEXLAYERHANDLE_INVALID;
	m_pOwnerBot			= NULL;
	m_bEnabled			= FALSE;
	m_bShieldActive		= FALSE;
	m_bPushOutBots		= TRUE;
	m_nFlickerCount 	= 0;
	m_fFlickerTimer		= 0.0f;
	m_bShieldSuspended	= FALSE;

	m_fShieldRechargeTime	= _DEFAULT_RECHARGETIME;	
	m_fOOShieldRechargeTime	= fmath_Inv(_DEFAULT_RECHARGETIME);
	m_fShieldRechargeDelay	= _DEFAULT_RECHARGEDELAY;
	m_fShieldRechargeTimer	= 0.0f;

	flinklist_InitRoot( &m_SplashList, FANG_OFFSETOF( _ShieldSplash_t, link ) );
}



CEShield::~CEShield() {
	if( IsSystemInitialized() && IsCreated() ) {
		DetachFromParent();
		DetachAllChildren();
		RemoveFromWorld( TRUE );
		ClassHierarchyDestroy();
	}
}


BOOL CEShield::Create( cchar *pszEntityName/*=NULL*/, const CFMtx43A *pMtx/*=NULL*/, cchar *pszAIBuilderName/*=NULL*/ ) {
	FASSERT( IsSystemInitialized() );
	FASSERT( !IsCreated() );
	
	// Get pointer to the leaf class's builder object...
	CEShieldBuilder *pBuilder = (CEShieldBuilder*)GetLeafClassBuilder();

	// If we're the leaf class, set the builder defaults...
	if( pBuilder == &_EShieldBuilder ) {
		pBuilder->SetDefaults( 0, 0, ENTITY_TYPE_SHIELD );
	}

	// Set our builder parameters...

	// Create an entity...
	return CEntity::Create( pszEntityName, pMtx, pszAIBuilderName );
}



BOOL CEShield::ClassHierarchyBuild( void ) {
	FASSERT( IsSystemInitialized() );
	FASSERT( !IsCreated() );
	//FASSERT( FWorld_pWorld );

	FResFrame_t  ResFrame;
	FMeshInit_t	 meshInit;
	FMesh_t		*pMesh;

	//get frame
	ResFrame = fres_GetFrame();

	// Get pointer to the leaf class's builder object...
	CEShieldBuilder *pBuilder = (CEShieldBuilder *)GetLeafClassBuilder();

	// Build parent class...
	if( !CEntity::ClassHierarchyBuild() ) {
		// Parent class could not be built...
		goto _ExitWithError;
	}

	if( !m_bEffectsLoaded ) {
		_InitEffects();
	}

	m_pWorldMesh = fnew CFWorldMesh;
	if( m_pWorldMesh == NULL ) {
		DEVPRINTF( "CEShield::ClassHierarchyBuild():  Unable to create worldmesh\n" );
		goto _ExitWithError;
	}
	
	pMesh = (FMesh_t*)fresload_Load( FMESH_RESTYPE, _SHIELD_MESH );
	if( pMesh == NULL ) {
		DEVPRINTF( "CEShield::ClassHierarchyBuild():  Unable to load mesh %s\n", _SHIELD_MESH );
		goto _ExitWithError;
	}
	
	meshInit.pMesh		= pMesh;
	meshInit.fCullDist	= _SHIELD_CULLDIST;
	meshInit.Mtx		= *MtxToWorld();
	meshInit.nFlags		= FMESHINST_FLAG_NOBONES;

	m_pWorldMesh->Init( &meshInit );
	m_pWorldMesh->RemoveFromWorld();
	m_pWorldMesh->SetCollisionFlag( FALSE );
	m_pWorldMesh->m_nUser = MESHTYPES_ENTITY;
	m_pWorldMesh->m_pUser = this;
	m_pWorldMesh->SetUserTypeBits( TypeBits() );

	//grab our tex layer handle
	m_hTexLayer = m_pWorldMesh->GetTexLayerHandle( _SHIELD_TEXLAYER );
	if( m_hTexLayer == FMESH_TEXLAYERHANDLE_INVALID ) {
		DEVPRINTF( "CEShield::ClassHierarchyBuild():  Unable to get shield tex layer handle, ID=%d\n", _SHIELD_TEXLAYER );
		goto _ExitWithError;
	}
//	m_pWorldMesh->AnimTC_AnimateScroll( m_hTexLayer, FALSE );	//don't auto animate our tex layer

	return TRUE;

	// Error:
_ExitWithError:
	Destroy();
	fres_ReleaseFrame( ResFrame );
	return FALSE;
}



void CEShield::ClassHierarchyDestroy( void ) {
	fdelete( m_pWorldMesh );
	m_pWorldMesh = NULL;

	_DestroyEffects();

	CEntity::ClassHierarchyDestroy();
}


void CEShield::ClassHierarchyRelocated( void *pIdentifier ) {
	CEntity::ClassHierarchyRelocated( pIdentifier );

	if( IsInWorld() ) {
		m_pWorldMesh->m_Xfm.BuildFromMtx( m_MtxToWorld, m_fScaleToWorld );
		m_pWorldMesh->m_Xfm.m_fScaleF = m_fScaleToWorld;
		m_pWorldMesh->UpdateTracker();
	}
}


void CEShield::ClassHierarchyAddToWorld( void ) {
    FASSERT( IsCreated() );
	FASSERT( !IsInWorld() );

	CEntity::ClassHierarchyAddToWorld();

	m_pWorldMesh->m_Xfm.BuildFromMtx( m_MtxToWorld, m_fScaleToWorld );
	m_pWorldMesh->m_Xfm.m_fScaleF = m_fScaleToWorld;
	m_pWorldMesh->UpdateTracker();
}

void CEShield::ClassHierarchyRemoveFromWorld( void ) {
	CEntity::ClassHierarchyRemoveFromWorld();
	m_pWorldMesh->RemoveFromWorld();
	_KillSplash( 0.0f, TRUE );
}


CEntityBuilder *CEShield::GetLeafClassBuilder( void ) {
	return &_EShieldBuilder;
}

void CEShield::AppendTrackerSkipList(u32& nTrackerSkipListCount, CFWorldTracker ** apTrackerSkipList) {
	FASSERT( IsCreated() );
	FASSERT( (nTrackerSkipListCount + 1) <= FWORLD_MAX_SKIPLIST_ENTRIES );
	apTrackerSkipList[nTrackerSkipListCount++] = m_pWorldMesh;
}


void CEShield::Init( CBot *pOwnerBot, const ShieldInit_t *pShieldInit/*=NULL*/, BOOL bStartOn/*=TRUE*/ ) {
	FASSERT( m_bSystemInitialized );
	FASSERT( IsCreated() );

	m_pOwnerBot = pOwnerBot;

	m_uFlags = 0;

	// if shieldinit is filled out, use that data, otherwise, use the defaults that have already been set
	if( pShieldInit ) {
		FASSERT( pShieldInit->fShieldRechargeTime != 0.0f );

		m_fScaleToWorld			= pShieldInit->fShieldScale;
		m_fShieldRechargeTime	= pShieldInit->fShieldRechargeTime;		
		m_fOOShieldRechargeTime	= fmath_Inv( pShieldInit->fShieldRechargeTime );
		m_fShieldRechargeDelay	= pShieldInit->fShieldRechargeDelay;
		if ( pShieldInit->pArmorProfile )
		{
			SetArmorProfile( pShieldInit->pArmorProfile );
		}

		if( pShieldInit->uShieldFlags & SHIELD_ENABLE_TINT ) {
			m_rgbTintColor = pShieldInit->rgbTint;
		} else {
			m_rgbTintColor.White();
		}
		m_pWorldMesh->SetMeshTint( m_rgbTintColor.fRed, m_rgbTintColor.fGreen, m_rgbTintColor.fBlue );

		m_uFlags = pShieldInit->uShieldFlags;
	}

	if( bStartOn ) {
		SetNormHealth( 1.0f );
		m_fShieldRechargeTimer	= 0.0f;
		//m_fEffectTimer	= 1.0f;	
		//m_fEffectTarget	= 1.0f;
		m_fShieldAlpha		= _ALPHA_ON;

	} else {
		SetNormHealth( 0.0f );
		m_fShieldRechargeTimer	= 0.0f;  
		//m_fEffectTimer	= 0.0f;	
		//m_fEffectTarget	= 0.0f;
		m_fShieldAlpha		= 0.0f;
	}



	RemoveFromWorld();
	AddToWorld();
}

void CEShield::SetTint(CFColorRGB& rTint)
{ 
	if( m_uFlags & SHIELD_ENABLE_TINT ) {
		m_rgbTintColor = rTint;
		m_pWorldMesh->SetMeshTint( m_rgbTintColor.fRed, m_rgbTintColor.fGreen, m_rgbTintColor.fBlue );
	}
}

void CEShield::EnableShield( BOOL bEnable ) {
	m_bEnabled		= bEnable;
	m_bShieldActive = bEnable;
	m_pWorldMesh->SetCollisionFlag( !!(bEnable && !(m_uFlags&SHIELD_NOPROTECTION)) );
	m_pWorldMesh->SetLineOfSightFlag(FALSE); //FALSE means it will not be included in LOS queries
	if( bEnable ) {
		FMATH_CLEARBITMASK( m_pWorldMesh->m_nFlags, FMESHINST_FLAG_DONT_DRAW );
	} else {
		FMATH_SETBITMASK( m_pWorldMesh->m_nFlags, FMESHINST_FLAG_DONT_DRAW );
	}

	if (!bEnable && (m_uFlags&SHIELD_NOPROTECTION)) {
		_KillSplash( 0.0f, TRUE );		
	}

}


void CEShield::AddSplashEffect( const CFVec3A &rvPos, f32 fMagnitude/*=1.0f*/ ) {
	FASSERT( m_bSystemInitialized );

	_ShieldSplash_t *pSplash = _GetNextSplash();
	
	pSplash->fUnitTime		= 0.0f;
	pSplash->pShield		= this;
	pSplash->pMesh->SetMeshTint( m_rgbTintColor.fRed, m_rgbTintColor.fGreen, m_rgbTintColor.fBlue );
	pSplash->vDisplacement.Sub( rvPos, m_pWorldMesh->GetBoundingSphere().m_Pos );
	//let's try this instead:
	if( pSplash->vDisplacement.MagSq() < 0.01f ) {
		// not a lot we can do, just bail, put the splash back first
		flinklist_AddTail( &m_FreeSplashList, pSplash );
		return;
	}

	pSplash->vDisplacement.Unitize();
	pSplash->vDisplacement.Mul( m_pWorldMesh->GetBoundingSphere().m_fRadius );
//	pSplash->vDisplacement.Mul( fmath_RandomFloatRange( _SPLASH_PUSH_MIN, _SPLASH_PUSH_MAX ) );
	
	_UpdateSplashPosition( m_pWorldMesh->GetBoundingSphere().m_Pos, pSplash );

	pSplash->pMesh->m_Xfm.BuildScale( m_fScaleToWorld );
	pSplash->pMesh->m_Xfm.m_fScaleF = m_fScaleToWorld;
	pSplash->pMesh->m_nFlags &= ~FMESHINST_FLAG_DONT_DRAW;
	pSplash->pMesh->AddToWorld();
	pSplash->pMesh->SetMeshAlpha( _SPLASH_INITIAL_ALPHA );

	m_fShieldAlpha = _ALPHA_SHIELD_HIT;

	// tint the splash a little bit to give an idea of shield health
//	pSplash->pMesh->SetMeshTint( 0.75f, 0.75f, FMATH_FPOT( NormHealth(), 0.75f, 1.0f ) ); 
	
	flinklist_AddTail( &m_SplashList, pSplash );

	// notify the bot that the shield is doing the splash thing
	if( m_pOwnerBot ) {
		m_pOwnerBot->NotifyShieldStatus( SHIELD_NOTIFY_IMPACT, rvPos );
	}
}


_ShieldSplash_t* CEShield::_ReleaseSplash( _ShieldSplash_t *pSplash ) {
	FASSERT( pSplash != NULL );

	_ShieldSplash_t *pRetSplash = (_ShieldSplash_t*)flinklist_GetNext( &m_SplashList, pSplash );

	if( pSplash->pMesh ) {
		pSplash->pMesh->m_nFlags |= FMESHINST_FLAG_DONT_DRAW;
		pSplash->pMesh->RemoveFromWorld();
	}

	flinklist_Remove( &m_SplashList, pSplash );
	flinklist_AddTail( &m_FreeSplashList, pSplash );

	return pRetSplash;
}


void CEShield::_KillSplash( f32 fTime, BOOL bReleaseAll/*=FALSE*/ ) {
	FASSERT( m_bSystemInitialized );

	_ShieldSplash_t *pSplash = (_ShieldSplash_t*)flinklist_GetHead( &m_SplashList );

	while( pSplash != NULL ) {
		if( pSplash->fUnitTime >= fTime ) {
			pSplash = _ReleaseSplash( pSplash );
			if( !bReleaseAll ) {
				return;
			}
		} else {
			pSplash = (_ShieldSplash_t*)flinklist_GetNext( &m_SplashList, pSplash );
		}
	}
}


void CEShield::Work( void ) {
	if( !m_bEnabled ) {
		return;
	}

	if( m_pWorldMesh ) {
		m_pWorldMesh->SetLineOfSightFlag(FALSE); //FALSE means it will not be included in LOS queries
	}

	if( _ORIENT_TOWARD_CAMERA ) {
		_OrientMesh();
	}

	_HandleShieldLevelandState();

	// handle shield alpha...
	if( NormHealth() > 0.0f ) {
		// shield on...
		if( m_fShieldAlpha > _ALPHA_ON ) {
			m_fShieldAlpha -= FLoop_fPreviousLoopSecs * _ALPHA_FADE_RATE;
			FMATH_CLAMPMIN( m_fShieldAlpha, _ALPHA_ON );
		}
	}

	// handle the splashes
	_ShieldSplash_t *pSplash = (_ShieldSplash_t*)flinklist_GetHead( &m_SplashList );

	while( pSplash != NULL ) {
		if( pSplash->fUnitTime >= 1.0f ) {
			pSplash = _ReleaseSplash( pSplash );
		} else {
			_UpdateSplashPosition( m_pWorldMesh->GetBoundingSphere().m_Pos, pSplash );
			pSplash->fUnitTime += FLoop_fPreviousLoopSecs * _EFFECT_TIMESCALE;
			pSplash = (_ShieldSplash_t*)flinklist_GetNext( &m_SplashList, pSplash );
		}
	}
	
	// handle flickering...
	if( m_nFlickerCount > 0 ) {
		if( m_fFlickerTimer < 0.0f ) {
			if( (m_nFlickerCount & 0x01) == 1 ) {
				FMATH_SETBITMASK( m_pWorldMesh->m_nFlags, FMESHINST_FLAG_DONT_DRAW );
				m_pWorldMesh->SetMeshTint( 1.0f, 1.0f, 1.0f );
				m_fShieldAlpha = 1.0f;
			} else {
				FMATH_CLEARBITMASK( m_pWorldMesh->m_nFlags, FMESHINST_FLAG_DONT_DRAW );
				m_pWorldMesh->SetMeshTint( 1.0f, 0.0f, 0.0f );
				m_fShieldAlpha = 0.66f;
			}
			if( --m_nFlickerCount > 0 ) {
				m_fFlickerTimer += fmath_RandomFloatRange( _FLICKER_MIN_INTERVAL, _FLICKER_MAX_INTERVAL );
			}
		}
		m_fFlickerTimer -= FLoop_fPreviousLoopSecs;
	}

	//push out any bots that have entered the shield's area
	if( m_bShieldActive ) {
		_PushOutBots();
	}

	m_pWorldMesh->SetMeshAlpha( m_fShieldAlpha );
}


void CEShield::_HandleShieldLevelandState( void ) {
	f32 fHealth;

	fHealth = NormHealth();

	if( fHealth < 1.0f ) {
		if( m_fShieldRechargeTimer < 0.0f ) {
			if( !IsShieldOn() ) {
				_ShieldOn();
			}
	
			fHealth += FLoop_fPreviousLoopSecs * m_fOOShieldRechargeTime;
			FMATH_CLAMP_MAX1( fHealth );
			SetNormHealth( fHealth );

			// while shield is charging, set our alpha to shield's health
			m_fShieldAlpha = fHealth;
		} else {
			m_fShieldRechargeTimer -= FLoop_fPreviousLoopSecs;
		}
	}
}


void CEShield::_UpdateSplashPosition( const CFVec3A &vShieldPos, _ShieldSplash_t *pSplash ) {
	CFMtx43A::m_Temp.Identity();
	CFVec3A vTmpPos;
	vTmpPos.Add( vShieldPos, pSplash->vDisplacement );
	
	CFMtx43A::m_Temp.m_vFront.Sub( vShieldPos, vTmpPos );
	CFMtx43A::m_Temp.m_vFront.Unitize();
	CFMtx43A::m_Temp.m_vRight.CrossYWithVec( CFMtx43A::m_Temp.m_vFront );

	if( CFMtx43A::m_Temp.m_vRight.MagSq() > 0.0001f ) {
		CFMtx43A::m_Temp.m_vRight.Unitize();
	} else {
		CFMtx43A::m_Temp.m_vRight = CFVec3A::m_UnitAxisX;
	}

	CFMtx43A::m_Temp.m_vUp.Cross( CFMtx43A::m_Temp.m_vFront, CFMtx43A::m_Temp.m_vRight );

	CFMtx43A::m_RotZ.SetRotationZ( fmath_RandomFloatRange( 0.0f, FMATH_PI ) );
	CFMtx43A::m_Temp.Mul( CFMtx43A::m_RotZ );

	CFMtx43A::m_Temp.m_vPos = vTmpPos;

	pSplash->pMesh->m_Xfm.BuildFromMtx( CFMtx43A::m_Temp, m_fScaleToWorld );
	pSplash->pMesh->UpdateTracker();

	//Update alpha
	CFVec2	vTmp;
	f32		fUnitTime;

	fUnitTime = pSplash->fUnitTime;
	FMATH_CLAMPMAX( fUnitTime, 1.0f );
	pSplash->pMesh->SetMeshAlpha( FMATH_FPOT( fUnitTime, 0.5f, 0.0f ) );
	
	fUnitTime = fmath_Sqrt( fUnitTime );
    vTmp.Set( FMATH_FPOT( fUnitTime, 1.0f, 0.75f ), 0.0f );
	pSplash->pMesh->AnimTC_SetScrollST( pSplash->hTexLayer, vTmp );
	
	
}


void CEShield::InflictDamage( CDamageData *pDamageData ) {

	// reset our recharge timer

	if( m_pOwnerBot ) {
		if ( ((m_pOwnerBot->m_nPossessionPlayerIndex == -1) && (pDamageData->m_Damager.pBot == m_pOwnerBot))) {	//findfix: pgm says not sure if this should go here, but was trying to prevent NPC titans from inadvertantly damaging themselves
			return;
		}
	}

	if (m_uFlags & SHIELD_NOPROTECTION) {
		return;
	}

	m_fShieldRechargeTimer = m_fShieldRechargeDelay;
	CEntity::InflictDamage( pDamageData );
}


void CEShield::InflictDamageResult( const CDamageResult *pDamageResult ) {
	CEntity::InflictDamageResult( pDamageResult );

	if( pDamageResult->m_fDeltaHitpoints == 0.0f ) {
		// If no damage was done, no need to do damage or let AI know...
		return;
	}

	AddSplashEffect( pDamageResult->m_pDamageData->m_ImpactPoint_WS );

	if( m_pOwnerBot ) {
		aibrainman_DamagedNotify( m_pOwnerBot->AIBrain(), pDamageResult );
	}

	if( IsShieldInvulnerable() ) {
		SetNormHealth( 1.0f );
	}

	if( NormHealth() == 0.0f ) {
		_ShieldOff();
	}
}


void CEShield::DropShield( f32 fRechargeTime/*=-1.0f*/ ) { 
	SetNormHealth( 0.0f );

	if( fRechargeTime < 0.0f ) {
		m_fShieldRechargeTimer = m_fShieldRechargeDelay;
	} else {
		m_fShieldRechargeTimer = fRechargeTime;
	}

	_ShieldOff( FALSE ); 
}


void CEShield::SetShieldHealth( f32 fUnitHealth ) {
	FASSERT( m_bSystemInitialized );

	SetNormHealth( fUnitHealth );

	if( fUnitHealth == 0.0f ) {
		m_bShieldActive = FALSE;
		m_pWorldMesh->SetCollisionFlag( FALSE );
		FMATH_SETBITMASK( m_pWorldMesh->m_nFlags, FMESHINST_FLAG_DONT_DRAW );
	} else {
		m_bShieldActive = TRUE;
		//m_pWorldMesh->SetCollisionFlag( TRUE );
		m_pWorldMesh->SetCollisionFlag( !(m_uFlags&SHIELD_NOPROTECTION) );
		FMATH_CLEARBITMASK( m_pWorldMesh->m_nFlags, FMESHINST_FLAG_DONT_DRAW );
	}
	

}


void CEShield::_ShieldOff( BOOL bShowEffects/*=TRUE*/ ) {
	if( m_bShieldActive && bShowEffects ) {
		m_fShieldAlpha	= 1.0f;
		m_fFlickerTimer = fmath_RandomFloatRange( _FLICKER_MIN_INTERVAL, _FLICKER_MAX_INTERVAL );
		m_nFlickerCount	= fmath_RandomRange( _FLICKER_MIN_COUNT, _FLICKER_MAX_COUNT );

		if( m_pOwnerBot ) {
			m_pOwnerBot->NotifyShieldStatus( SHIELD_NOTIFY_DOWN, m_MtxToWorld.m_vPos );
		}
	}
	
	m_bShieldActive	= FALSE;
	_KillSplash( 0.0f, TRUE );

	m_pWorldMesh->SetCollisionFlag( FALSE );
}


void CEShield::_ShieldOn( void ) {
	if( !m_bShieldActive ) {
		m_fShieldAlpha	= 0.0f;
		m_bShieldActive = TRUE;

		//m_pWorldMesh->SetCollisionFlag( TRUE );
		m_pWorldMesh->SetCollisionFlag( !(m_uFlags&SHIELD_NOPROTECTION) );
		if( m_pOwnerBot ) {
			m_pOwnerBot->NotifyShieldStatus( SHIELD_NOTIFY_UP, m_pWorldMesh->GetBoundingSphere().m_Pos );
		}
	}
	FMATH_CLEARBITMASK( m_pWorldMesh->m_nFlags, FMESHINST_FLAG_DONT_DRAW );
}



void CEShield::_PushOutBots( void ) {
	// if pushout is disabled, don't bother
	if( !m_bPushOutBots || m_bShieldSuspended ) {
		return;
	}

	m_pCollThis = this;
	u64 uCollBots = _PUSHABLE_BOTS;

	m_pPosToDrawSplash = NULL;
	fworld_FindTrackersIntersectingSphere( &m_pWorldMesh->GetBoundingSphere(), FWORLD_TRACKERTYPE_MESH, _CollisionCallback, 0, NULL, m_pWorldMesh, MESHTYPES_ENTITY, uCollBots );
	if( m_pPosToDrawSplash ) {
		AddSplashEffect( *m_pPosToDrawSplash );
	}
}

void CEShield::_OrientMesh( void ) {
	CFCamera *pCamera = gamecam_GetActiveCamera();
	if( pCamera ) {
		CFMtx43A::m_Temp.m_vUp.Sub( m_MtxToWorld.m_vPos, *(pCamera->GetPos()) );
		CFMtx43A::m_Temp.m_vUp.Unitize();
		CFMtx43A::m_Temp.m_vUp.Negate();

		CFMtx43A::m_Temp.m_vRight.CrossVecWithY( CFMtx43A::m_Temp.m_vUp );
		if( CFMtx43A::m_Temp.m_vRight.MagSq() < 0.01f ) {
			CFMtx43A::m_Temp.m_vRight = CFVec3A::m_UnitAxisX;
			CFMtx43A::m_Temp.m_vFront = CFVec3A::m_UnitAxisZ;
		} else {
			CFMtx43A::m_Temp.m_vRight.Unitize();
			CFMtx43A::m_Temp.m_vFront.Cross( CFMtx43A::m_Temp.m_vRight, CFMtx43A::m_Temp.m_vUp );
		}

		CFMtx43A::m_Temp.m_vPos		= m_MtxToWorld.m_vPos;

		m_pWorldMesh->m_Xfm.BuildFromMtx( CFMtx43A::m_Temp, m_fScaleToWorld );
		m_pWorldMesh->UpdateTracker();
	}
}


BOOL CEShield::_CollisionCallback( CFWorldTracker *pTracker, FVisVolume_t *pVolume ) {
	CFVec3A vPush;
	CBot *pBot = (CBot*)pTracker->m_pUser;

	// do not attempt to push immobile bots.  (wouldn't work anyway)
	if( (pBot == m_pCollThis->m_pOwnerBot) || (pBot->IsImmobileOrPending()) ) {
		return TRUE;
	}

	vPush = pTracker->GetBoundingSphere().m_Pos;
	vPush.Sub( m_pCollThis->m_pWorldMesh->GetBoundingSphere().m_Pos );
	FMATH_CLAMPMIN( vPush.y, 0.0f );
	FMATH_BIPOLAR_CLAMPMIN(vPush.x, 0.01f );
	FMATH_BIPOLAR_CLAMPMIN( vPush.z, 0.01f );

	f32 fPushStr = 10.0f * FLoop_fPreviousLoopSecs;

	vPush.Mul( fPushStr );
	pBot->ApplyVelocityImpulse_WS( vPush );

	if ( !(m_pCollThis->m_uFlags&SHIELD_NOPROTECTION) ) {
	//	m_pCollThis->AddSplashEffect( pTracker->GetBoundingSphere().m_Pos );
		m_pPosToDrawSplash = &(pBot->MtxToWorld()->m_vPos);
	}

	return TRUE;
}


void CEShield::Suspend( BOOL bSuspend ) {
	FASSERT( IsCreated() );
    
	if( bSuspend ) {
		m_bShieldSuspended = TRUE;
		m_pWorldMesh->SetCollisionFlag( FALSE );
	} else {
		m_bShieldSuspended = FALSE;
		if( m_bShieldActive && m_bEnabled ) {
			//m_pWorldMesh->SetCollisionFlag( TRUE );
			m_pWorldMesh->SetCollisionFlag( !(m_uFlags&SHIELD_NOPROTECTION) );
		}
	}
}




















