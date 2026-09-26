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

#ifndef _FXMAGMABOMB_H_
#define _FXMAGMABOMB_H_ 1

#include "fang.h"
#include "fdraw.h" 
#include "fcoll.h"
#include "FParticle.h"
#include "EParticle.h"
#include "Damage.h"

class EBotFire;
class CAreaFlame;
class CFXMagmaBomb;
class CFSoundGroup;

#define FMATH_SQRT3	(1.73205080756888f)

#define _MAGMABOMB_NUM_TEXTURES		1
#define _MAGMABOMB_NUM_PARTICLE_FX	3
#define _MAX_NUM_MAGMAEFFECTS		6

#define _NUM_FLAMES					6
#define _NUM_SMOKES					1

typedef struct
{
	CFSoundGroup *pSoundGroupOnFire;

	f32 fMinBombDuration;
	f32 fMaxBombDuration;

	f32 fMinSpreadRadius;
	f32 fMaxSpreadRadius;

	f32 fMinSpreadSpeed;
	f32 fMaxSpreadSpeed;

	CDamageProfile* pAmbientDamageProfile;
	CDamageProfile* pCatchFireDamageProfile;

	f32 fMinAttachedFlameDuration;
	f32 fMaxAttachedFlameDuration;
}MagmaBombProps_t;

FCLASS_ALIGN_PREFIX class CAreaFlame
{
public:
	void Create(const FParticle_DefHandle_t& rPart, BOOL bDoesDamage);
	void Init(CBot* pBot,CWeapon* pWeapon,const CFVec3A& vPos,const CFVec3A& vUp,const CFVec3A& vFront);
	void Draw(void);
	// returns if particle is active or not;
	BOOL Work(BOOL bComputeCollisions);
	void EndFlame(void);

public:
	// travels pts
	CFVec3A m_vEndPos;
	CFVec3A m_vTravelDirection;

	// up points
	CFVec3A m_vStartUp;
	CFVec3A m_vEndUp;

	FParticle_DefHandle_t m_hFireParticleDef;
	CEParticle m_FireParticle;
		
	const CFSphere* m_pBoundsWS;
	
	CDamageProfile m_DamageProfile;
	CDamageForm* m_pDamageForm;
	MagmaBombProps_t* m_pBombProps;
	BOOL	   m_bDealsDamage;
	BOOL	   m_bIsCreeping;
	BOOL	   m_bTryToCorner;
	f32 m_fVelocity;
	f32 m_fDuration;
	f32 m_fOODuration;
	f32 m_fTimePassed;
	u8 m_bActive;
private:	
}FCLASS_ALIGN_SUFFIX; 

FCLASS_ALIGN_PREFIX class CFXMagmaBomb
{

//----------------------------------------------------------------------------------------------------------------------------------
// Public Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
public:
	
//----------------------------------------------------------------------------------------------------------------------------------
// Protected Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
protected:

//----------------------------------------------------------------------------------------------------------------------------------
// Private Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
private:

//----------------------------------------------------------------------------------------------------------------------------------
// Public Functions:
//----------------------------------------------------------------------------------------------------------------------------------
public:
	CFXMagmaBomb();
	static BOOL			 InitSystem( void );
		   BOOL			 _InitSystem( void );
	static void			 UninitSystem( void );
	static FINLINE	BOOL IsSystemInitialized( void ) { return m_bSystemInitialized; };
	static void EndAllEffects(void);

	static void Work( void );
	static void DrawAll( void );	
	static CFXMagmaBomb* SpawnBombFX(CBot* pBot,CWeapon* pWeapon,const FCollImpact_t *pImpact, MagmaBombProps_t& MagmaProps);
	void EndEffect(void);
//----------------------------------------------------------------------------------------------------------------------------------
// Protected Functions:
//----------------------------------------------------------------------------------------------------------------------------------
protected:
	

//----------------------------------------------------------------------------------------------------------------------------------
// Private Functions:
//----------------------------------------------------------------------------------------------------------------------------------
private:
	static  BOOL _InitTextures( void );
	static  BOOL _InitParticles( void );
			
	void _Work( void );
	void _Draw( void );
	BOOL _IsOffscreen( void );
	
	friend class CWeaponMagmaBomb; // we actually belong to him, he can do anything with us
		
	void _Init(const FCollImpact_t*);
	
//----------------------------------------------------------------------------------------------------------------------------------
// Private Data:
//----------------------------------------------------------------------------------------------------------------------------------
private:
	static BOOL					m_bSystemInitialized;				// TRUE if the system has been initialized
	static BOOL					m_bSystemActive;
	static cchar*				m_apszTexNames[_MAGMABOMB_NUM_TEXTURES];
	static CFTexInst			m_aTextures[_MAGMABOMB_NUM_TEXTURES];
	static cchar*				m_apszPartNames[_MAGMABOMB_NUM_PARTICLE_FX];
	static FParticle_DefHandle_t m_ahParticleDefs[_MAGMABOMB_NUM_PARTICLE_FX];

	CAreaFlame m_aFlames[_NUM_FLAMES];
	CEParticle m_aSmokeEmitters[_NUM_SMOKES];
	MagmaBombProps_t* m_pBombProps;
	s32		   m_nWhichFlameToComputeCollision;
	CFVec3A	   m_vImpactPos;
	BOOL	   m_bActive;
	CBot*	   m_pSpawnerBot;
	CWeapon*   m_pSpawnerWeapon;
	f32		   m_fDuration;
	f32        m_fOODuration;
	f32        m_fTimePassed;
	u32 m_uAISoundHandle;					//handle to an AI sound that is used to notify AI that a fire is happening here.

	FCLASS_STACKMEM_ALIGN( CFXMagmaBomb );
} FCLASS_ALIGN_SUFFIX;

#endif