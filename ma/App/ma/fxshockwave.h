//////////////////////////////////////////////////////////////////////////////////////
// fxshockwave.h
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
// 11.18.02 Elliott		Created.
//////////////////////////////////////////////////////////////////////////////////////

#ifndef _FXSHOCKWAVE_H_
#define _FXSHOCKWAVE_H_		1

#include "fang.h"
#include "fdraw.h"
#include "fworld.h"
#include "fpsprite.h"
#include "ftext.h"


struct _DustSprite_s; //defined in cpp file

FCLASS_ALIGN_PREFIX class CFXShockwave {
//----------------------------------------------------------------------------------------------------------------------------------
// Public Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
public:
	static CFXShockwave *m_pCollisionShockwave;

//----------------------------------------------------------------------------------------------------------------------------------
// Protected Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
protected:

//----------------------------------------------------------------------------------------------------------------------------------
// Private Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
private:


//----------------------------------------------------------------------------------------------------------------------------------
// Private Data:
//----------------------------------------------------------------------------------------------------------------------------------
private:
	static BOOL			 m_bSystemInitialized;		// TRUE if the system has been initialized
	static CFXShockwave *m_aFXShockwaves;
	
	static FDrawVtx_t  *m_aRingVtx;			
	static f32		   *m_afRingVtxAlpha;
	static CFTexInst	m_RingTexInst;
	static CFMtx43A		m_MtxRingTex;				//matrix for animating our stomp ring texture
	static BOOL			m_bActive;
	


	f32	m_fMaxRadius;				
	f32 m_fRingWidth;				
	f32 m_fAmpDecayRate;
	f32 m_fRadIncreaseRate;
	f32 m_fInitialAmp;
	f32 m_fOOInitialAmp;
	f32 m_fInitialScale;
	f32 m_fClearanceHeight;
	f32 m_fOOClearanceHeight;
	f32 m_fMaxPushMag;
	f32 m_fMinPushMag;
	u64 m_uEntityCollisionFlags;
		
	f32			m_fAmpY;
	f32			m_fScaleXZ;
	CFSphere	m_Sphere;
	

	FWorldIntersectingTrackerCallback_t *m_pCallbackFn;

	//point sprite fun
	static CFPSpriteGroup	m_PSGroup;			//point sprites for dust settle effect
	_DustSprite_s		   *m_aDustSpriteData;
	u32						m_nNumDustSprites;	// how many dust sprites are in use
	BOOL					m_bDrawingDust;
	f32						m_fDustCreateTimer;	// used to determine when to create a new puff of dust




//----------------------------------------------------------------------------------------------------------------------------------
// Public Functions:
//----------------------------------------------------------------------------------------------------------------------------------
public:
	static BOOL InitSystem( void );
	static void UninitSystem( void );
	static FINLINE BOOL IsSystemInitialized( void ) { return m_bSystemInitialized; };

	static void Work( void );
	static void Draw( void );

	static void KillAll( void );

	static void AddShockwave( const CFVec3A &vPos,											// center of the ring
							  f32 fInitialAmplitude,										// amplitude (Y direction)
							  f32 fSpeed,													// speed the ring expands
							  u64 uEntityFlags,												// what kind of entities to check collisions against
							  f32 fMaxPushEffect,											// how far we want to push entities 
							  f32 fMinPushEffect,											// amount to push right at the end
							  FWorldIntersectingTrackerCallback_t *pCollCallback=NULL );	// callback for when we hit an entity

//----------------------------------------------------------------------------------------------------------------------------------
// Protected Functions:
//----------------------------------------------------------------------------------------------------------------------------------
protected:
	CFXShockwave();

//----------------------------------------------------------------------------------------------------------------------------------
// Private Functions:
//----------------------------------------------------------------------------------------------------------------------------------
private:
//	static BOOL _StompRingCollisionCallback( CFWorldTracker *pTracker, FVisVolume_t *pVolume );
	static BOOL _InitMesh( void );			//initialize the mesh
	static BOOL _InitTex( void );			//initialize the texture(s)
	static BOOL _InitPointSprites( void );	//initialize ... the pointsprites.
	
	static CFXShockwave *_GetNewShockwave( void );		//gets a shockwave to be initialized (may result in an active one going away)
	static BOOL _RingCollisionCallback( CFWorldTracker *pTracker, FVisVolume_t *pVolume );
	
	
	void _Draw( void );
	void _Work( void );
	void _DrawDust( void );


FCLASS_STACKMEM_ALIGN( CFXShockwave );
} FCLASS_ALIGN_SUFFIX;

#endif