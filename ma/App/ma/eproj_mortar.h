//////////////////////////////////////////////////////////////////////////////////////
// eproj_mortar.h - cannonballs for mortar projectiles
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
// 02/25/03 Scholz		Created.
//////////////////////////////////////////////////////////////////////////////////////

#ifndef _EPROJ_MORTAR_H_
#define _EPROJ_MORTAR_H_ 1

#include "fang.h"
#include "eproj.h"
#include "fparticle.h"
#include "fmotion.h"


//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CEProj_Mortar
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

FCLASS_ALIGN_PREFIX class CEProj_Mortar : public CEProjExt
{
	//----------------------------------------------------------------------------------------------------------------------------------
	// Private Data:
	//----------------------------------------------------------------------------------------------------------------------------------
	private:
		static BOOL m_bSystemLoaded;
		static u32  m_nClassClientCount;
		static BOOL m_bEffectsLoaded;

		// particle definition
		static FParticle_DefHandle_t m_hTrailParticleDef;
		static CFTexInst			_StreamerTexInst;
		static FLinkRoot_t			m_FreeSplashList;
		FLinkRoot_t					m_SplashList;			// each proj has a list of its active splashes

		CFMotion m_Motion;

		CFWorldAttachedLight		m_WorldLight;
		CFVec3A						m_vWorldCollideOffset;
        FParticle_EmitterHandle_t	m_hTrailParticle;
		BOOL						m_bAbortCollisionTests;

	//----------------------------------------------------------------------------------------------------------------------------------
	// Public Functions:
	//----------------------------------------------------------------------------------------------------------------------------------
	public:

		// Construct/Destruct:
		CEProj_Mortar() {}
		~CEProj_Mortar() {}

		static BOOL LoadSharedResources( void );
		static void UnloadSharedResources( void );
	
		static BOOL _SlowerTrackerCallback( CFWorldTracker *pTracker, FVisVolume_t *pVolume );

		// Extension interface:
		virtual BOOL Create( CEProj *pProj );
		virtual void Destroy( CEProj *pProj );

		virtual void Init( CEProj *pProj );
		virtual void Launched( CEProj *pProj );
		virtual BOOL Detonated( CEProj *pProj, BOOL bMakeEffect, u32 nEvent, FCollImpact_t *pCollImpact );

		virtual BOOL Work( CEProj *pProj );

		// function that gets called when projectile goes out of the world
		virtual void Removed( CEProj *pProj );

	private:
		//----------------------------------------------------------------------------------------------------------------------------------
		// Private Functions:
		//----------------------------------------------------------------------------------------------------------------------------------

		static BOOL				_CollideSphereCB( CFWorldTracker *pTracker, FVisVolume_t *pVolume, u32 nSphereIndex );
		FCLASS_STACKMEM_ALIGN( CEProj_Mortar );
} FCLASS_ALIGN_SUFFIX;




#endif

