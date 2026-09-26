//////////////////////////////////////////////////////////////////////////////////////
// edebris.h - Debris spawner class.
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
// 05/10/03 Ranck       Created.
//////////////////////////////////////////////////////////////////////////////////////

#ifndef _EDEBRIS_H_
#define _EDEBRIS_H_ 1

#include "fang.h"
#include "fdebris.h"
#include "ebox.h"


#define EDEBRIS_SPAWNER_COUNT	3		// 0=small, 1=medium, 2=large



FCLASS_ALIGN_PREFIX class CEDebris : public CEBox {
//----------------------------------------------------------------------------------------------------------------------------------
// Protected Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
protected:

	ENTITY_CLASS_HIERARCHY_BITDEF




//----------------------------------------------------------------------------------------------------------------------------------
// Private Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
private:

	enum {
		FLAG_POINT_EMITTER		= 0x00000001,	// This is a point debris emitter
		FLAG_ATTENUATE_SOUND	= 0x00000002,	// When TRUE, the sound effect volume will be attenuated along with the debris intensity

		FLAG_NONE				= 0
	};




//----------------------------------------------------------------------------------------------------------------------------------
// Private Data:
//----------------------------------------------------------------------------------------------------------------------------------
private:

	static BOOL m_bDebrisSystemInitialized;		// TRUE when InitSystem() has been called
	static CArmorProfile m_ArmorProfileRumble;	// Allows 100% rumble damage only

	u32 m_nFlags;								// See FLAG_* for info

	f32 m_fGravityMult;							// Gravity multiplier
	f32 m_fMinDebrisCount;						// Minimum number of debris chunks per spawn
	f32 m_fMaxDebrisCount;						// Maximum number of debris chunks per spawn
	f32 m_fMinDuration;							// Minimum duration at full intensity
	f32 m_fMaxDuration;							// Maximum duration at full intensity

	f32 m_fAutoShakeSecsMin;					// Minimum time between shakes (0=off, >0=time between shakes)
	f32 m_fAutoShakeSecsMax;					// Maximum time between shakes (0=off, >0=time between shakes)
	f32 m_fAutoShakeTimer;						// Counts down when auto-shaking

	CFDebrisSpawner m_aDebrisSpawner[EDEBRIS_SPAWNER_COUNT];	// Our very own debris spawners ([0]=small, [1]=medium, [2]=large)

	FSndFx_FxHandle_t m_hSound;					// Sound handle (FSNDFX_INVALID_FX_HANDLE for none)
	f32 m_fSoundUnitVolume;						// Sound unit volume
	f32 m_fSoundRadius;							// Sound radius
	f32 m_fSoundPitchMult;						// Sound pitch multiplier

	CFAudioEmitter *m_pAudioEmitter;			// NULL if none




//----------------------------------------------------------------------------------------------------------------------------------
// Public Functions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	// System:
	static BOOL InitSystem( void );
	static void UninitSystem( void );
	FINLINE static BOOL IsDebrisSystemInitialized( void ) { return m_bDebrisSystemInitialized; }


	// Construct/Destruct:
	CEDebris();
	virtual ~CEDebris();


	// Interface:
	void EnableRumbleShake( BOOL bEnable );
	FINLINE BOOL IsRumbleShakeEnabled( void ) const { FASSERT( IsCreated() ); return GetArmorProfile()->m_fUnitProtectionFromRumble < 1.0f; }

	void SetAutoShake( f32 fMinTime, f32 fMaxTime );
	FINLINE f32 GetAutoShakeMin( void ) const { FASSERT( IsCreated() ); return m_fAutoShakeSecsMin; }
	FINLINE f32 GetAutoShakeMax( void ) const { FASSERT( IsCreated() ); return m_fAutoShakeSecsMax; }

	void Shake( f32 fUnitRumbleIntensity );
	void Kill( BOOL bKillSound );




//----------------------------------------------------------------------------------------------------------------------------------
// Protected Functions:
//----------------------------------------------------------------------------------------------------------------------------------
protected:

	virtual BOOL ClassHierarchyBuild( void );
	virtual BOOL ClassHierarchyBuilt( void );
	virtual CEntityBuilder *GetLeafClassBuilder( void );
	virtual void ClassHierarchyRelocated( void *pIdentifier );
	virtual void ClassHierarchyAddToWorld( void );
	virtual void ClassHierarchyRemoveFromWorld( void );
	virtual void ClassHierarchyWork( void );
	virtual void ClassHierarchyBecameActive( void );
	virtual void ClassHierarchyBecameInactive( void );
	virtual void InflictDamageResult( const CDamageResult *pDamageResult );
	virtual void CheckpointSaveSelect( s32 nCheckpoint );
	virtual BOOL CheckpointSave( void );
	virtual void CheckpointRestore( void );




//----------------------------------------------------------------------------------------------------------------------------------
// Private Functions:
//----------------------------------------------------------------------------------------------------------------------------------
private:

	void _SetDefaults( void );
	void _UpdateOurWorkBit( void );


	FCLASS_STACKMEM_ALIGN( CEDebris );
} FCLASS_ALIGN_SUFFIX;




//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CEDebrisBuilder
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

FCLASS_ALIGN_PREFIX class CEDebrisBuilder : public CEBoxBuilder {
//----------------------------------------------------------------------------------------------------------------------------------
// Public Data:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	const CFDebrisGroup *m_apDebrisGroup[EDEBRIS_SPAWNER_COUNT];	// The debris groups ([0]=small, [1]=medium, [2]=large)
	u32 m_nMinDebrisCount;						// Minimum number of debris chunks per spawn
	u32 m_nMaxDebrisCount;						// Maximum number of debris chunks per spawn
	f32 m_fMinDuration;							// Minimum duration at full intensity
	f32 m_fMaxDuration;							// Maximum duration at full intensity
	f32 m_fFadeOutSecs;							// Fade-out duration
	f32 m_fSpawnsPerSec;						// Number of spawns per second
	f32 m_fMinSpeed;							// Minimum debris chunk speed when spawned
	f32 m_fMaxSpeed;							// Maximum debris chunk speed when spawned
	f32 m_fUnitDirSpread;						// 0=spawned along Z axis, 1=maximum direction spread
	f32 m_fScaleMult;							// Multiplied by the scale in the debris group
	f32 m_fGravityMult;							// Multiplied by the gravity in the debris group
	f32 m_fRotSpeedMult;						// Multiplied by the rotational speed in the debris group
	f32 m_fAutoShakeSecsMin;					// Minimum time between shakes (0=off, >0=time between shakes)
	f32 m_fAutoShakeSecsMax;					// Maximum time between shakes (0=off, >0=time between shakes)
	BOOL m_bRumbleShake;						// FALSE=infinite armor profile, TRUE=rumble armor profile

	cchar *m_pszSound;							// Sound name
	f32 m_fSoundUnitVolume;						// Sound unit volume
	f32 m_fSoundRadius;							// Sound radius
	f32 m_fSoundPitchMult;						// Sound pitch multiplier
	BOOL m_bSoundAttenuate;						// TRUE=attenuate volume along with debris intensity

	BOOL m_bPlayImpactSound;					// TRUE=play impact sound specified in the debris group
	BOOL m_bPlayFlamingSound;					// TRUE=play flaming sound specified in the debris group




//----------------------------------------------------------------------------------------------------------------------------------
// Public Functions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	FINLINE CEDebrisBuilder() {}
	virtual void SetDefaults( u64 nEntityTypeBits, u64 nEntityLeafTypeBit, cchar *pszEntityType );




//----------------------------------------------------------------------------------------------------------------------------------
// Protected Functions:
//----------------------------------------------------------------------------------------------------------------------------------
protected:

	virtual BOOL InterpretTable( void );
	virtual BOOL PostInterpretFixup( void );


	FCLASS_STACKMEM_ALIGN( CEDebrisBuilder );
} FCLASS_ALIGN_SUFFIX;



#endif

