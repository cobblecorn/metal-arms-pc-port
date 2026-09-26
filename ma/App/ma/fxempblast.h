//////////////////////////////////////////////////////////////////////////////////////
// fxempblast.h
//
// Author: Nathan Miller
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
// 11/20/02 Miller		Created.
//////////////////////////////////////////////////////////////////////////////////////
#ifndef _FXEMPBLAST_H_
#define _FXEMPBLAST_H_		1

#include "fang.h"
#include "fworld.h"
#include "fparticle.h"


#define FXEMPBLAST_INCLUDE_SPARKLERS		FALSE

class CEntity;
class CFMeshInst;
class CEMPLightningBoltGroup;
class CFSoundGroup;


FCLASS_ALIGN_PREFIX class CFXEMPBlast {

private:

#if FXEMPBLAST_INCLUDE_SPARKLERS
	typedef struct {
		BOOL bAlive;
		f32 fLife;
		f32 fCurrentRadius;
		f32 fMaxRadius;
		FParticle_EmitterHandle_t hTrailEmitter;

		CFVec3A Rotate; // in radians
		CFVec3A RotateVel;
		CFVec3A Position;
	} BlastSparkler_t;
#endif

	typedef struct {
		BOOL bActive;			// Is the sphere active
		BOOL bWobble;			// Should we wobble the sphere after hitting the max radius
		f32 fGrowRate;			// How fast should the sphere grow
		f32 fMaxRadius;			// Max radius
		f32 fOOMaxRadius;		// fmath_Inv( m_fMaxRadius )
		f32 fCurrentRadius;		// Current sphere radius
		f32 fMinWobbleRadius;	// Min wobble radius
		f32 fMaxWobbleRadius;	// Max wobble radius
		f32 fWobbleDir;			// Direction to wobble: 1.0f = grow, -1.0f = shrink, even if m_bWobble = FALSE, must be 1.0f
		f32 fRotateZ;			// How much to rotate about Z
		f32 fAlpha;				// Alpha of this sphere
	} BlastSphere_t;

	enum {
		_MAX_SPARKLERS = 13,
		_MAX_BLAST_SPHERES = 3,
	};

	BOOL8 m_bAlive;										// Is the blast alive
	BOOL8 m_bRecruiterEffect;							// Recruiter version of the effect
	f32 m_fLife;										// Time lived
	f32 m_fLifeMax;										// Max life to live
	f32 m_fMaxRadius;									// Max blast radius
	f32 m_fMaxWobbleRadius;								// Max sphere wobble radius
	CFVec3A m_PosWS;									// WS position
	CEMPLightningBoltGroup *m_pBoltGroup;				// Lightning bolts

#if FXEMPBLAST_INCLUDE_SPARKLERS
	BlastSparkler_t m_Sparklers[_MAX_SPARKLERS];		// Sparklers
#endif

	BlastSphere_t m_BlastSpheres[_MAX_BLAST_SPHERES];	// Blast spheres
	CFWorldLight m_WorldLight;

	f32 m_fOrigSoundUnitVolume;							// Original volume of EMP sound effect
	CFAudioEmitter *m_pAudioEmitterEffect;				// Audio emitter for the EMP sound effect

	static BOOL m_bSystemInitialized;					// TRUE if the system has been initialized
	static BOOL m_bActive;								// TRUE if there are blasts that are alive
	static CFXEMPBlast *m_pBlasts;				

#if FXEMPBLAST_INCLUDE_SPARKLERS
	static FParticle_DefHandle_t m_hTrailParticleDef;
	static FParticle_DefHandle_t m_hBurstParticleDef;
#endif

	static CFMeshInst *m_pMeshInst;
	static f32 m_fOOMeshRadiusOuter;

public:
	static BOOL InitSystem( void );
	static void UninitSystem( void );

	static FINLINE BOOL IsSystemInitialized( void ) { return m_bSystemInitialized; };

	static void KillAll( void );
	static void Work( void );
	static void Draw( void );
	static void SpawnBlast( const f32 &fLife, const f32 &fMaxRadius, const CFVec3A &PosWS, CFSoundGroup *pSoundGroupEffect=NULL, BOOL bRecruiterEffect=FALSE );

protected:
	CFXEMPBlast();

	void _Work( void );
	void _Kill( void );
	
	void _SpawnBlastSphere( const u32 &uIndex, BOOL bWobble = FALSE );

#if FXEMPBLAST_INCLUDE_SPARKLERS
	void _WorkSparklers( void );
	void _KillParticles( void );
	void _SpawnSparkler( u32 uNdx );
#endif

	static CFXEMPBlast *_GetNewBlast( void );
	static BOOL _WorldLoadAndUnloadCallback( FWorldEvent_e eEvent );

FCLASS_STACKMEM_ALIGN( CFXEMPBlast );
} FCLASS_ALIGN_SUFFIX;

#endif