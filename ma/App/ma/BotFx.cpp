//////////////////////////////////////////////////////////////////////////////////////
// BotFx.cpp - 
//
// Author: Michael Starich   
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
// 02/26/02 Starich     Created.
//////////////////////////////////////////////////////////////////////////////////////
#include "fang.h"
#include "BotFx.h"
#include "floop.h"
#include "bot.h"
#include "fdatapool.h"
#include "fdraw.h"
#include "fresload.h"
#include "fclib.h"
#include "fvtxpool.h"
#include "FCheckPoint.h"
#include "FXStreamer.h"
#include "eshield.h"
#include "BlinkSpeed.h"
#include "BlinkGlow.h"
#include "botglitch.h"
#include "MultiplayerMgr.h"

//====================
// private definitions

static const f32 _Speedup_fMaxCamZoom = 0.8f;
static const f32 _Speedup_fCamRate = 0.8f;

// streamer defs:
#define _ENABLE_STREAMER_FX		TRUE
#define _INITIAL_STREAMER_ALPHA	0.25f
#define _STREAMER_WIDTH_WS		0.75f
#define _SECS_AT_END_TO_FADE	0.75f

//=================
// public variables

//==================
// private variables

static BOOL _bSystemInited = FALSE;
static CFTexInst _StreamerTexInst;

static cchar *_apszMinerStreamerBoneNames[] = {
	"R_HEEL",
	"L_HEEL",
	NULL
};

CFAudioEmitter *CBotPowerupFx::s_pActiveSound;
u32 CBotPowerupFx::s_uActivePickupCount;
FSndFx_FxHandle_t CBotPowerupFx::s_hActiveSound;
u32 CBotPowerupFx::s_nBotSFXType;

//===================
// private prototypes

//=================
// public functions

//==================
// private functions

//-----------------------------------------------------------------------------
void CBotPowerupFx::_ClearDataMembers( void )
{
	m_pBot = NULL;
	m_uBotFXMask = BOT_FX_MASK_NONE;
	m_fHiJumpSecs = 0.0f;
	m_fSpeedUpSecs = 0.0f;
	m_fArmorSecs = 0.0f;
	m_fWeaponSecs = 0.0f;
	m_hStreamerEmitter = FXSTREAMER_INVALID_HANDLE;
	//m_pShell = NULL;
	m_pShield = NULL;
	m_pSpeed = NULL;
	m_pGlow = NULL;
}

//-----------------------------------------------------------------------------
// increase count of active BotFX's and start new powerup sound.
// m_uBotFXMask must not include new FX bit.
void CBotPowerupFx::_IncrementActiveCountAndUpdateSFX( BotFxTypes_e nType, FSndFx_FxHandle_t hSndHandle )
{
	if( s_pActiveSound != NULL)
	{
		s_pActiveSound->Destroy();
		s_pActiveSound = NULL;
	}

	s_hActiveSound = hSndHandle;
	s_pActiveSound = FSNDFX_ALLOCNPLAY2D(s_hActiveSound, 1.0f, 1.0f, FAudio_EmitterDefaultPriorityLevel, 0.f, TRUE);
	s_nBotSFXType = nType;

	if( s_uActivePickupCount == 0 || (m_uBotFXMask & (1 << nType)) == 0 )
	{
		// They're not picking up the same powerup again, so let's increase the powerup count.
		++s_uActivePickupCount;
	}
}

//-----------------------------------------------------------------------------
// reduce count of active BotFX's, and turn off sound
// effect if count reaches zero.
void CBotPowerupFx::_DecrementActiveCountAndUpdateSFX( void )
{
	FASSERT(s_uActivePickupCount > 0);
	if( s_uActivePickupCount <= 0 )
	{
		return;
	}

	--s_uActivePickupCount;
	if(s_uActivePickupCount == 0)
	{
		if(s_pActiveSound != NULL)
		{
			s_pActiveSound->Destroy();
			s_pActiveSound = NULL;
		}
	}
}

//-----------------------------------------------------------------------------
void CBotPowerupFx::_StartHiJumpFx( f32 fDuration, f32 fJumpVelMult, f32 fGravityMult )
{
	FASSERT( m_pBot );
	if( !m_pBot )
	{
		return;
	}

	FASSERT(m_pBot->TypeBits() & ENTITY_BIT_BOTGLITCH);
	if( !(m_pBot->TypeBits() & ENTITY_BIT_BOTGLITCH) )
	{
		return;
	}

	if( fDuration <= 0.0f )
	{
		return;
	}

#if _ENABLE_STREAMER_FX
	if( CFXStreamerEmitter::IsValidHandle( m_hStreamerEmitter ) )
	{
		// we already have a streamer emitter, keep it emitting
		CFXStreamerEmitter::EnableStreamerEmission( m_hStreamerEmitter, TRUE );
	} 
	else if( _bSystemInited ) 
	{
		// grab a streamer emitter
		m_hStreamerEmitter = CFXStreamerEmitter::SpawnFromMeshBones( 50, 
			m_pBot->m_pWorldMesh,
			_apszMinerStreamerBoneNames,
			_INITIAL_STREAMER_ALPHA,
			&_StreamerTexInst,
			_STREAMER_WIDTH_WS,
			20.0f );
		if( m_hStreamerEmitter == FXSTREAMER_INVALID_HANDLE )
		{
			DEVPRINTF( "BotFx::PickupPowerup(): trouble starting streamer emitter.\n" );
		}
	}
#endif

	if(m_pGlow == NULL)
	{
		m_pGlow = CBlinkGlow::GetGlow();
	}

	if( m_pGlow )
	{
		m_pGlow->SetBotBlink((CBotGlitch *)(m_pBot));
		m_pGlow->Activate();
		m_pGlow->SetColor(1.0f, 1.0f, 0.80f, 1.0f, 1.0f);
	}

	m_uBotFXMask |= BOT_FX_MASK_HIJUMP;						
	m_pBot->m_fJumpMultiplier = fJumpVelMult;//_HI_JUMP_MULTIPLIER;
	m_pBot->m_fGravityMultiplier = fGravityMult;//_HiJump_fGravMult;

	m_fHiJumpSecs = fDuration;
}

//-----------------------------------------------------------------------------
void CBotPowerupFx::_StartSpeedFx( f32 fDuration, f32 fSpeedMult )
{
	FASSERT( m_pBot );
	if( !m_pBot )
	{
		return;
	}

	FASSERT(m_pBot->TypeBits() & ENTITY_BIT_BOTGLITCH);
	if( !(m_pBot->TypeBits() & ENTITY_BIT_BOTGLITCH) )
	{
		return;
	}

	if( fDuration <= 0.0f )
	{
		return;
	}

	m_uBotFXMask |= BOT_FX_MASK_SPEEDUP;						
	m_pBot->SetRunSpeed( fSpeedMult * MultiplayerMgr.GetSpeedMultiplier( m_pBot->m_nPossessionPlayerIndex ) );

	if(m_pSpeed == NULL)
	{
		m_pSpeed = CBlinkSpeed::GetSpeed();
		FASSERT(m_pSpeed != NULL);
	}

	if( m_pSpeed )
	{
		m_pSpeed->SetBotBlink((CBotGlitch *)(m_pBot));
		m_pSpeed->Activate();
		m_pSpeed->SetUnitEffectEnd( 0.0f );
	}

	if( m_pGlow )
	{
		m_pGlow->SetUnitEffectEnd( 0.0f );
		m_pGlow->SetBotBlink((CBotGlitch *)(m_pBot));
		m_pGlow->SetColor(1.0f, 1.0f, 1.0f, 1.0f, 1.0f);
		m_pGlow->Activate();
	}

	m_fSpeedUpSecs = fDuration;
}

//-----------------------------------------------------------------------------
void CBotPowerupFx::_StartWeaponFx( f32 fDuration )
{
	FASSERT( m_pBot );
	if( !m_pBot )
	{
		return;
	}

	FASSERT(m_pBot->TypeBits() & ENTITY_BIT_BOTGLITCH);
	if( !(m_pBot->TypeBits() & ENTITY_BIT_BOTGLITCH) )
	{
		return;
	}

	if( fDuration <= 0.0f )
	{
		return;
	}

	if( !(m_uBotFXMask & BOT_FX_MASK_WEAPON) )
	{
		m_uBotFXMask |= BOT_FX_MASK_WEAPON;
		CBotGlitch *pBotBlink = (CBotGlitch *)(m_pBot);
		pBotBlink->DeployBuddy();
	} else {
		CBotGlitch *pBotBlink = (CBotGlitch *)(m_pBot);
		pBotBlink->SetBuddyUnitEffectEnd( 0.0f );
	}

	m_fWeaponSecs = fDuration;
}

//-----------------------------------------------------------------------------
void CBotPowerupFx::_StartArmorFx( f32 fDuration )
{
	FASSERT( m_pBot );
	if( !m_pBot )
	{
		return;
	}

	FASSERT(m_pBot->TypeBits() & ENTITY_BIT_BOTGLITCH);
	if( !(m_pBot->TypeBits() & ENTITY_BIT_BOTGLITCH) )
	{
		return;
	}

	if( fDuration <= 0.0f )
	{
		return;
	}

	if( !(m_uBotFXMask & BOT_FX_MASK_ARMOR) )
	{
		// armored shell not currently active, set it up

		if (m_bNewShield && m_pBot)
		{
			ShieldInit_t shieldInit;

			shieldInit.fShieldScale = 0.33f;
			shieldInit.fShieldRechargeDelay = 1;
			shieldInit.fShieldRechargeTime = 1;
			shieldInit.rgbTint.fRed = 1.0f;
			shieldInit.rgbTint.fBlue = 0.25f;
			shieldInit.rgbTint.fGreen = 1.0f;
			shieldInit.pArmorProfile = NULL;
			shieldInit.uShieldFlags = CEShield::SHIELD_ENABLE_TINT | CEShield::SHIELD_NOPROTECTION;

			m_pShield->Init( m_pBot, &shieldInit );
			if( m_pShield ) {
				m_pShield->AddToWorld();
				m_pShield->Attach_UnitMtxToParent_PS( m_pBot, "Waist" ); 
				m_pShield->EnableShield( FALSE );
			}
		}

		m_bNewShield = FALSE;

		m_pShield->EnableShield(TRUE);
		m_uBotFXMask |= BOT_FX_MASK_ARMOR;
	}

	m_fArmorSecs = fDuration;
}

//-----------------------------------------------------------------------------
BOOL CBotPowerupFx::InitSystem() {

	FResFrame_t Frame;

	FASSERT( !_bSystemInited );

	Frame = fres_GetFrame();

	_StreamerTexInst.SetTexDef( (FTexDef_t *)fresload_Load( FTEX_RESNAME, "trdmstrmry1" ) );
	if( !_StreamerTexInst.GetTexDef() ) {
		goto _ExitSetupWithError;
	}

	_bSystemInited = TRUE;

	return TRUE;

_ExitSetupWithError:
	UninitSystem();
	fres_ReleaseFrame( Frame );
	return FALSE;	
}

//-----------------------------------------------------------------------------
void CBotPowerupFx::UninitSystem( void ) {

	_StreamerTexInst.SetTexDef( NULL );

	_bSystemInited = FALSE;
}


//-----------------------------------------------------------------------------
BOOL CBotPowerupFx::InitLevel( void ) {
	s_pActiveSound = NULL;
	s_uActivePickupCount = 0;
	s_hActiveSound = FSNDFX_INVALID_FX_HANDLE;
	s_nBotSFXType = 0;	// uses BOT_FX_MASK, but only one type is set at any time

	return(TRUE);
}


//-----------------------------------------------------------------------------
void CBotPowerupFx::UninitLevel( void ) {
	if( s_pActiveSound )
	{
		s_pActiveSound->Destroy();
	}
	s_pActiveSound = NULL;
	s_uActivePickupCount = 0;
}

//-----------------------------------------------------------------------------
CBotPowerupFx::CBotPowerupFx() {
	_ClearDataMembers();

	m_pShield = fnew CEShield;
	m_pShield->Create( "GlitchShield" );
	m_bNewShield = TRUE;
}

//-----------------------------------------------------------------------------
CBotPowerupFx::~CBotPowerupFx() {
	KillAll();

	if (m_pShield)
	{
		m_pShield->Destroy();
		fdelete (m_pShield);
		m_pShield = NULL;
	}
}

//-----------------------------------------------------------------------------
void CBotPowerupFx::SetBot( CBot *pBot ) {

	m_pBot = pBot;

	if (m_bNewShield && m_pBot)
	{
	
		ShieldInit_t shieldInit;

		shieldInit.fShieldScale = 0.33f;
		shieldInit.fShieldRechargeDelay = 1;
		shieldInit.fShieldRechargeTime = 1;
		shieldInit.rgbTint.fRed = 1.0f;
		shieldInit.rgbTint.fBlue = 0.25f;
		shieldInit.rgbTint.fGreen = 1.0f;
		shieldInit.pArmorProfile = NULL;
		shieldInit.uShieldFlags = CEShield::SHIELD_ENABLE_TINT | CEShield::SHIELD_NOPROTECTION;

		m_pShield->Init( m_pBot, &shieldInit );
		if( m_pShield ) {
			m_pShield->AddToWorld();
			m_pShield->Attach_UnitMtxToParent_PS( m_pBot, "Waist" ); 
			m_pShield->EnableShield( FALSE );
		}

		m_bNewShield = FALSE;
	}

	FASSERT(m_pShield != NULL);
}

//-----------------------------------------------------------------------------
void CBotPowerupFx::SetDamagePoint(const CFVec3A& rvDamagePoint) {
	if (m_pShield) {
		if ( m_pShield->IsEnabled() ) {
			m_pShield->AddSplashEffect(rvDamagePoint);
		}
	}
}

//-----------------------------------------------------------------------------
void CBotPowerupFx::PickupPowerup( BotFxTypes_e nType, FSndFx_FxHandle_t hSndHandle, f32 fDuration, f32 fSpeedMult, f32 fJumpVelMult, f32 fGravityMult ) {
	FASSERT( nType < BOT_FX_TYPES_COUNT );

	if( !m_pBot ) {
		DEVPRINTF( "BotFx::PickupPowerup(): don't know which bot to whom to apply powerup.\n" );
		return;
	}

	_IncrementActiveCountAndUpdateSFX( nType, hSndHandle );

	switch( nType ) {

	case BOT_FX_TYPES_HIJUMP:
			_StartHiJumpFx( fDuration, fJumpVelMult, fGravityMult );
		break;

	case BOT_FX_TYPES_SPEEDUP:
		_StartSpeedFx( fDuration, fSpeedMult );
		break;

	case BOT_FX_TYPES_WEAPON:
		_StartWeaponFx( fDuration );
		break;
		
	case BOT_FX_TYPES_ARMOR:
		_StartArmorFx( fDuration );
		break;

	default:
		FASSERT( FALSE );
		return;
	}
}

//-----------------------------------------------------------------------------
void CBotPowerupFx::KillAll() {
	
	if( !m_pBot ) {
		return;
	}

	m_uBotFXMask = BOT_FX_MASK_NONE;
	m_pBot->m_fGravityMultiplier = 1.0f;
	m_pBot->m_fJumpMultiplier = 1.0f;
	m_fHiJumpSecs = 0.0f;
	if( CFXStreamerEmitter::IsValidHandle( m_hStreamerEmitter ) ) {
		CFXStreamerEmitter::ReturnEmitterHandle( m_hStreamerEmitter );
		m_hStreamerEmitter = FXSTREAMER_INVALID_HANDLE;
	}

	m_pBot->SetRunSpeed( MultiplayerMgr.GetSpeedMultiplier( m_pBot->m_nPossessionPlayerIndex ) );
	m_fSpeedUpSecs = 0.0f;
	if( m_pSpeed ) {
		m_pSpeed->Deactivate();
		m_pSpeed->ReturnSpeed();
		m_pSpeed = NULL;
	}

	m_fArmorSecs = 0.0f;
	if( m_pShield ) {
		m_pShield->EnableShield(FALSE);
	}

	if( m_pGlow ) {
		m_pGlow->Deactivate();
		m_pGlow->ReturnGlow();
		m_pGlow = NULL;
	}

	if( s_pActiveSound ) {
		s_pActiveSound->Destroy();
		s_pActiveSound = NULL;
	}

	if( m_pBot->TypeBits() & ENTITY_BIT_BOTGLITCH ) {
		CBotGlitch *pBotBlink = (CBotGlitch *)m_pBot;
		pBotBlink->StowBuddy();
	}
	
	m_fWeaponSecs = 0.0f;
	
	s_uActivePickupCount = 0;
}

//-----------------------------------------------------------------------------
void CBotPowerupFx::Work() {

	if( !m_pBot ) {
		return;
	}

	//-------------------------------------------------------------------------
	if( m_uBotFXMask & BOT_FX_MASK_HIJUMP ) {
		m_fHiJumpSecs -= FLoop_fPreviousLoopSecs;
		if( m_fHiJumpSecs <= 0.0f ) {
			m_fHiJumpSecs = 0.0f;
			m_uBotFXMask &= ~BOT_FX_MASK_HIJUMP;
			m_pBot->m_fGravityMultiplier = 1.0f;
			m_pBot->m_fJumpMultiplier = 1.0f;

			CFXStreamerEmitter::EnableStreamerEmission( m_hStreamerEmitter, FALSE );

			if( m_pGlow != NULL )
			{
				if((m_uBotFXMask & BOT_FX_MASK_SPEEDUP) == 0)
				{
					// They don't also have the speedup still, so we can kill the glow.
					m_pGlow->Deactivate();
				}
				else
				{
					// continue drawing glow
					m_pGlow->SetUnitEffectEnd(0.0f);
				}
			}

			_DecrementActiveCountAndUpdateSFX();
		} 
		else if( m_fHiJumpSecs < 2.0f )
		{
			if( m_fHiJumpSecs <= _SECS_AT_END_TO_FADE ) {
				CFXStreamerEmitter::NearingEnd( m_hStreamerEmitter, 
											1.0f - (m_fHiJumpSecs * (1.0f/_SECS_AT_END_TO_FADE)) );	
			}
			if( m_pGlow != NULL )
			{
				m_pGlow->SetUnitEffectEnd( 1.0f - 0.5f * m_fHiJumpSecs );
			}

			if(m_pBot->m_fCamZoomFactor > 0.0f)
			{
				m_pBot->m_fCamZoomFactor -= (_Speedup_fCamRate * FLoop_fPreviousLoopSecs);
				FMATH_CLAMP_MIN0(m_pBot->m_fCamZoomFactor);
			}

			if(s_uActivePickupCount == 1)
			{
				if(s_pActiveSound != NULL && s_nBotSFXType == BOT_FX_MASK_HIJUMP )
				{
					s_pActiveSound->SetVolume(0.5f * m_fHiJumpSecs);
				}
			}
		}
		else
		{
			if(m_pBot->m_fCamZoomFactor < _Speedup_fMaxCamZoom)
			{
				m_pBot->m_fCamZoomFactor += (_Speedup_fCamRate * FLoop_fPreviousLoopSecs);
				FMATH_CLAMPMAX(m_pBot->m_fCamZoomFactor, _Speedup_fMaxCamZoom);
			}
		}
	} else {
		// see if our streamer handle is still valid (note: the work will be called automatically)
		if( m_hStreamerEmitter != FXSTREAMER_INVALID_HANDLE ) {
			if( !CFXStreamerEmitter::IsValidHandle( m_hStreamerEmitter ) ) {
				m_hStreamerEmitter = FXSTREAMER_INVALID_HANDLE;
			}
		}		
	}

	//-------------------------------------------------------------------------
	if( m_uBotFXMask & BOT_FX_MASK_SPEEDUP ) {
		m_fSpeedUpSecs -= FLoop_fPreviousLoopSecs;
		if( m_fSpeedUpSecs <= 0.0f ) {
			m_fSpeedUpSecs = 0.0f;
			m_uBotFXMask &= ~BOT_FX_MASK_SPEEDUP;
			m_pBot->SetRunSpeed( MultiplayerMgr.GetSpeedMultiplier( m_pBot->m_nPossessionPlayerIndex ) );

			if(m_pSpeed != NULL)
			{
				m_pSpeed->Deactivate();
			}

			if( m_pGlow )
			{
				if(m_uBotFXMask & BOT_FX_MASK_HIJUMP)
				{
					m_pGlow->SetUnitEffectEnd(0.0f);
					m_pGlow->SetColor(1.0f, 1.0f, 0.80f, 1.0f, 1.0f);
				}
				else
				{
					m_pGlow->Deactivate();
				}
			}

			_DecrementActiveCountAndUpdateSFX();
		}
		else if( m_fSpeedUpSecs < 2.0f )
		{
			if(m_pSpeed != NULL)
			{
				m_pSpeed->SetUnitEffectEnd( 1.0f - 0.5f * m_fSpeedUpSecs );
			}
			if( m_pGlow != NULL )
			{
				m_pGlow->SetUnitEffectEnd( 1.0f - 0.5f * m_fSpeedUpSecs );
			}

			if(m_pBot->m_fCamZoomFactor > 0.0f)
			{
				m_pBot->m_fCamZoomFactor -= (_Speedup_fCamRate * FLoop_fPreviousLoopSecs);
				FMATH_CLAMP_MIN0(m_pBot->m_fCamZoomFactor);
			}

			if(s_uActivePickupCount == 1)
			{
				if(s_pActiveSound != NULL && s_nBotSFXType == BOT_FX_MASK_SPEEDUP )
				{
					s_pActiveSound->SetVolume(0.5f * m_fSpeedUpSecs);
				}
			}
		}
		else
		{
			if(m_pBot->m_fCamZoomFactor < _Speedup_fMaxCamZoom)
			{
				m_pBot->m_fCamZoomFactor += (_Speedup_fCamRate * FLoop_fPreviousLoopSecs);
				FMATH_CLAMPMAX(m_pBot->m_fCamZoomFactor, _Speedup_fMaxCamZoom);
			}
		}
	}

	//-------------------------------------------------------------------------
	if( m_uBotFXMask & BOT_FX_MASK_ARMOR ) {
		m_fArmorSecs -= FLoop_fPreviousLoopSecs;

		m_pShield->Work();

		if( m_fArmorSecs <= 0.0f ) {
			m_fArmorSecs = 0.0f;
			m_uBotFXMask &= ~(BOT_FX_MASK_ARMOR);

			if(m_pShield != NULL)
			{
				m_pShield->EnableShield(FALSE);
			}

			_DecrementActiveCountAndUpdateSFX();
		}
		else if( m_fArmorSecs < 2.0f )
		{
			if(m_pShield != NULL)
			{
				//m_pShield->SetUnitEffectEnd( 1.0f - 0.5f * m_fArmorSecs );
				CFColorRGB currentTint;
				
				f32 fClr = fmath_Fmod( m_fArmorSecs, 0.25f ) * 4.0f;

				currentTint.fRed = 1.0f;
				currentTint.fBlue = 0.25f;
				currentTint.fGreen = 1.0f * fClr;
				
				m_pShield->SetTint(currentTint);
			}

			if(s_uActivePickupCount == 1 && s_nBotSFXType == BOT_FX_MASK_ARMOR )
			{
				if(s_pActiveSound != NULL)
				{
					s_pActiveSound->SetVolume(0.5f * m_fArmorSecs);
				}
			}
		}
		else
		{
			CFColorRGB currentTint;
				
			currentTint.fRed = 1.0f;
			currentTint.fBlue = 0.25f;
			currentTint.fGreen = 1.0f;
			
			m_pShield->SetTint(currentTint);
		}
	}

	//-------------------------------------------------------------------------
	if( m_uBotFXMask & BOT_FX_MASK_WEAPON ) {
		m_fWeaponSecs -= FLoop_fPreviousLoopSecs;
		if( m_fWeaponSecs <= 0.0f ) {
			m_fWeaponSecs = 0.0f;
			m_uBotFXMask &= ~(BOT_FX_MASK_WEAPON);

			if(m_pBot != NULL)
			{
				FASSERT(m_pBot->TypeBits() & ENTITY_BIT_BOTGLITCH);
				CBotGlitch *pBotBlink = (CBotGlitch *)(m_pBot);
				pBotBlink->StowBuddy();
			}

			_DecrementActiveCountAndUpdateSFX();
		}
		else if( m_fWeaponSecs < 2.0f )
		{
			if(m_pBot != NULL)
			{
				FASSERT(m_pBot->TypeBits() & ENTITY_BIT_BOTGLITCH);
				CBotGlitch *pBotBlink = (CBotGlitch *)(m_pBot);
				pBotBlink->SetBuddyUnitEffectEnd( 1.0f - 0.5f * m_fWeaponSecs );
			}

			if(s_uActivePickupCount == 1)
			{
				if(s_pActiveSound != NULL && s_nBotSFXType == BOT_FX_MASK_WEAPON )
				{
					s_pActiveSound->SetVolume(0.5f * m_fWeaponSecs);
				}
			}
		}
	}

}

//-----------------------------------------------------------------------------
void CBotPowerupFx::Draw() {

	if( !m_pBot ) {
		return;
	}

	// the streamer draw will be called automatically 
}

//-----------------------------------------------------------------------------
void CBotPowerupFx::CheckpointSave( void )
{
	CFCheckPoint::SaveData( m_uBotFXMask );
	CFCheckPoint::SaveData( m_fHiJumpSecs );
	CFCheckPoint::SaveData( m_fSpeedUpSecs );
	CFCheckPoint::SaveData( m_fWeaponSecs );
	CFCheckPoint::SaveData( m_fArmorSecs );
	CFCheckPoint::SaveData( s_uActivePickupCount );
}

//-----------------------------------------------------------------------------
void CBotPowerupFx::CheckpointRestore( void )
{
	f32 fJumpMult, fGravityMult, fRunMult, fOORunMult;

	if( m_pBot )
	{
		// these values set to default by KillAll()
		fJumpMult = m_pBot->m_fJumpMultiplier;
		fGravityMult = m_pBot->m_fGravityMultiplier;
		fRunMult = m_pBot->m_fRunMultiplier;
		fOORunMult = m_pBot->m_fOORunMultiplier;
	}

	KillAll();

	if (m_pShield)
	{
		m_bNewShield = TRUE;
	}

	CFCheckPoint::LoadData( m_uBotFXMask );
	CFCheckPoint::LoadData( m_fHiJumpSecs );
	CFCheckPoint::LoadData( m_fSpeedUpSecs );
	CFCheckPoint::LoadData( m_fWeaponSecs );
	CFCheckPoint::LoadData( m_fArmorSecs );
	CFCheckPoint::LoadData( s_uActivePickupCount );

	FASSERT( m_pBot );
	if( !m_pBot )
	{
		return;
	}

	if( m_uBotFXMask & BOT_FX_MASK_HIJUMP )
	{
		m_uBotFXMask &= ~BOT_FX_MASK_HIJUMP;
		_StartHiJumpFx( m_fHiJumpSecs, fJumpMult, fGravityMult );
	}

	if( m_uBotFXMask & BOT_FX_MASK_SPEEDUP )
	{
		m_uBotFXMask &= ~BOT_FX_MASK_SPEEDUP;
		_StartSpeedFx( m_fSpeedUpSecs, fRunMult );
	}

	if( m_uBotFXMask & BOT_FX_MASK_WEAPON )
	{
		m_uBotFXMask &= ~BOT_FX_MASK_WEAPON;
		_StartWeaponFx( m_fWeaponSecs );
	}

	if( m_uBotFXMask & BOT_FX_MASK_ARMOR )
	{
		m_uBotFXMask &= ~BOT_FX_MASK_ARMOR;
		_StartArmorFx( m_fArmorSecs );
	}
}


