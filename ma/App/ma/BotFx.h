//////////////////////////////////////////////////////////////////////////////////////
// BotFx.h - 
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
#ifndef _BOTFX_H_
#define _BOTFX_H_ 1

#include "fang.h"
#include "FXStreamer.h"
#include "fsndfx.h"

// forward declarations
class CBot;
//class CBlinkShell;
class CEShield;
class CBlinkSpeed;
class CBlinkGlow;

class CBotPowerupFx 
{ 
public:
	CBotPowerupFx();
	~CBotPowerupFx();

	static BOOL InitSystem();
	static void UninitSystem( void );
	static BOOL InitLevel();
	static void UninitLevel( void );

	typedef enum {
		BOT_FX_TYPES_HIJUMP = 0,
		BOT_FX_TYPES_SPEEDUP,
		BOT_FX_TYPES_ARMOR,
		BOT_FX_TYPES_WEAPON,

		BOT_FX_TYPES_COUNT
	} BotFxTypes_e;

	void SetBot( CBot *pBot );
	// NOTE: Any parameters that are not used for the particular kind of powerup will be ignored.
	void PickupPowerup( BotFxTypes_e nType, FSndFx_FxHandle_t hSndHandle, f32 fDuration, f32 fSpeedMult, f32 fJumpVelMult, f32 fGravityMult );
	void KillAll();
	void Work();
	void Draw();
	void CheckpointSave( void );
	void CheckpointRestore( void );
	FINLINE BOOL IsArmorActive( void ) const { return( m_fArmorSecs > 0.0f ); };

	void SetDamagePoint(const CFVec3A& rvDamagePoint);

private:
	enum {
		BOT_FX_MASK_HIJUMP	= 0x1,
		BOT_FX_MASK_SPEEDUP = 0x2,
		BOT_FX_MASK_ARMOR	= 0x4,
		BOT_FX_MASK_WEAPON	= 0x8,
		BOT_FX_MASK_NONE = 0
	};

	void _ClearDataMembers( void );
	void _DecrementActiveCountAndUpdateSFX( void );
	void _IncrementActiveCountAndUpdateSFX( BotFxTypes_e nType, FSndFx_FxHandle_t hSndHandle );

	void _StartHiJumpFx( f32 fDuration, f32 fJumpVelMult, f32 fGravityMult );
	void _StartSpeedFx( f32 fDuration, f32 fSpeedMult );
	void _StartWeaponFx( f32 fDuration );
	void _StartArmorFx( f32 fDuration );

	static CFAudioEmitter *s_pActiveSound;
	static u32 s_uActivePickupCount;
	static FSndFx_FxHandle_t s_hActiveSound;
	static u32 s_nBotSFXType;	// uses BOT_FX_MASK, but only one type is set at any time

	CBot *m_pBot;
	FXStreamerHandle_t m_hStreamerEmitter;

	// HiJump/Speedup vars
	CBlinkGlow *m_pGlow;

	// Hi jump vars
	f32 m_fHiJumpSecs;
	
	// Speedup vars
	f32 m_fSpeedUpSecs;
	CBlinkSpeed *m_pSpeed;

	// Armor vars.
	f32 m_fArmorSecs;
	//CBlinkShell *m_pShell;
	CEShield *m_pShield;

	// Weapon vars.
	f32 m_fWeaponSecs;

	u16 m_uBotFXMask;// see BOT_FX_MASK_...

	BOOL m_bNewShield;
	
protected:
	friend class CEJumpPad;
	FCLASS_STACKMEM_NOALIGN( CBotPowerupFx );
};





#endif

