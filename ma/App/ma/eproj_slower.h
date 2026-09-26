//////////////////////////////////////////////////////////////////////////////////////
// eproj_slower.h - slowdown projectiles.
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
// 02.07.03 Scholz		Created.
//////////////////////////////////////////////////////////////////////////////////////

#ifndef _EPROJ_SLOWDOWN_H_
#define _EPROJ_SLOWDOWN_H_ 1

#include "fang.h"
#include "eproj.h"
#include "fparticle.h"

struct _ShieldSplash_t;		//rename later


//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CEProj_Slower
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

FCLASS_ALIGN_PREFIX class CEProj_Slower : public CEProjExt
{
	//----------------------------------------------------------------------------------------------------------------------------------
	// Private Data:
	//----------------------------------------------------------------------------------------------------------------------------------
	private:
		static BOOL m_bSystemInitialized;
		static BOOL m_bEffectsLoaded;

		// particle definition
		static FParticle_DefHandle_t m_hTrailParticleDef;
		static CFTexInst			_StreamerTexInst;
		static _ShieldSplash_t		*m_aSplashes;
		static FLinkRoot_t		m_FreeSplashList;
		static f32				m_fMaxDevLengthSquared;
		FLinkRoot_t				m_SplashList;			// each proj has a list of its active splashes



		
		CFWorldAttachedLight		m_WorldLight;
		CFVec3A						m_vWorldCollideOffset;
        FParticle_EmitterHandle_t	m_hTrailParticle;
		f32							m_fSpeedBoost;
		f32							m_fStationaryTimer;
		BOOL8						m_bAbortCollisionTests;
		BOOL8						m_bGoneStationary;
		
	//----------------------------------------------------------------------------------------------------------------------------------
	// Public Functions:
	//----------------------------------------------------------------------------------------------------------------------------------
	public:

		// Construct/Destruct:
		CEProj_Slower() {}
		~CEProj_Slower() {}

		static BOOL InitSystem( f32 fMaxDeviationSquared );
		static void UninitSystem( void );
		static BOOL InitEffects( void );
		static void DestroyEffects( void );

		static void DamageCallback( CEProj *pProj, CDamageData *pDamageData );
		static void DeathCallback( CEProj* pProj );
		
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

		static _ShieldSplash_t* _GetNextSplash( void );	// returns the next available splash, off the list of free splashes if possible		
		
		void _AddSplashEffect(CEProj *pProj, const CFVec3A &rvPos, f32 fMagnitude=1.0f );
		void _KillSplash( f32 fTime, BOOL bReleaseAll=FALSE );			// Release the (first if !bReleaseAll) splash in the list that is at least as old as fTime
		void _UpdateSplashPosition( const CFVec3A &vShieldPos, _ShieldSplash_t *pSplash );
		
		_ShieldSplash_t* _ReleaseSplash( _ShieldSplash_t *pSplash );				// turns it off, removes from our list and adds back to free list.  Returns next in our work list or NULL

		
		

		FCLASS_STACKMEM_ALIGN( CEProj_Slower );
} FCLASS_ALIGN_SUFFIX;




#endif

