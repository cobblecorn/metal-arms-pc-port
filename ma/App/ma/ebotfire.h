//////////////////////////////////////////////////////////////////////////////////////
// EBotFire.h - Catch me botty on flame-style
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

#ifndef _BOTFIRE_H_
#define _BOTFIRE_H_ 1

#include "fang.h"
#include "fdraw.h"
#include "eparticle.h"

#define _EBOTFIRE_NUM_PARTICLE_FX 2
#define _EBOTFIRE_POOL_SIZE	32

FCLASS_ALIGN_PREFIX class CEBotFire : public CEParticle 
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

	static BOOL InitSystem( void );
	static void UninitSystem( void );
	static CEBotFire *GetFreeEBotFireFromPool( void );
	static void EndAll( void );
	static BOOL _WorldCallback( FWorldEvent_e nEvent );

FINLINE FParticle_DefHandle_t GetFlameDef(void) {FASSERT(m_ahParticleDefs[0]);return m_ahParticleDefs[0];}
	void Reset(void);
	void ReturnToFreePool();
	void Start(CDamageProfile* pDamageProfile, CWeapon* pWeaponImFrom,CBot* pBotToBlame,f32 fDuration);
	

//----------------------------------------------------------------------------------------------------------------------------------
// Protected Functions:
//----------------------------------------------------------------------------------------------------------------------------------
protected:

	virtual void ClassHierarchyWork( void );
	virtual BOOL ClassHierarchyBuild( void );
	virtual void ClassHierarchyAddToWorld( void );
	virtual void ClassHierarchyRemoveFromWorld( void );
	virtual void ClassHierarchyDestroy( void );
	
	
//----------------------------------------------------------------------------------------------------------------------------------
// Private Functions:
//----------------------------------------------------------------------------------------------------------------------------------
private:
	static void _InitPools( void );
	static void _UninitPools( void );
	static BOOL _InitParticles( void );
	enum ListMember_t
	{
		LIST_MEMBER_UNUSED,
		LIST_MEMBER_FREE_POOL,
		LIST_MEMBER_USED_POOL,
	};
//----------------------------------------------------------------------------------------------------------------------------------
// Private Data:
//----------------------------------------------------------------------------------------------------------------------------------
private:
	static BOOL m_bSystemInitialized;
	static CEBotFire *m_pPoolFreeEBotFire;
	static FLinkRoot_t m_RootFreeEBotFire;
	static CEBotFire *m_pPoolUsedEBotFire;
	static FLinkRoot_t m_RootUsedEBotFire;

	static cchar*				 m_apszPartNames[_EBOTFIRE_NUM_PARTICLE_FX];
	static FParticle_DefHandle_t m_ahParticleDefs[_EBOTFIRE_NUM_PARTICLE_FX];

	CDamageForm*	m_pDamageForm;
	FLink_t			m_Link;							// Link to other CEBotFire objects in pool
	u8				m_uListMember;
FCLASS_STACKMEM_ALIGN( CEBotFire );
} FCLASS_ALIGN_SUFFIX;

#endif