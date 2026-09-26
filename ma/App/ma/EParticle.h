//////////////////////////////////////////////////////////////////////////////////////
// EParticle.h - 
//
// Author: Justin Link      
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
// 09/14/02 Link        Created.
// 09/15/02 Ranck		Took ownership.
//////////////////////////////////////////////////////////////////////////////////////
#ifndef _EPARTICLE_H_
#define _EPARTICLE_H_ 1

#include "fang.h"
#include "fparticle.h"
#include "entity.h"



FCLASS_ALIGN_PREFIX class CEParticle : public CEntity {
//----------------------------------------------------------------------------------------------------------------------------------
// Protected Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
protected:

	ENTITY_CLASS_HIERARCHY_BITDEF


//----------------------------------------------------------------------------------------------------------------------------------
// Private Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
private:
	


//----------------------------------------------------------------------------------------------------------------------------------
// Private Data:
//----------------------------------------------------------------------------------------------------------------------------------
private:

	static BOOL m_bSystemInitialized;			// TRUE = this system has been initialized

	FParticleEmitCfg_t *m_pPartConf;			// a filled in particle config to be used for delayed spawning, 
												// if NULL then the spawn occured at ClassHierarchyBuild time
	FParticle_DefHandle_t m_hPartDef;			// The particle def to emit
	FParticle_EmitterHandle_t m_hEmitter;		// Emitter handle (FPARTICLE_INVALID_HANDLE if not valid)
	f32 m_fUnitIntensity;						// The current emission intensity
	f32 m_fInitialUnitIntensity;				// The initial emission intensity

	// time reducing
	f32 m_fSecsRemaining;						// Seconds remaining (-1=don't time emission)
	f32 m_fOOMaxTime;							// 1.0f / (max time)
	BOOL8 m_bReduceIntensityWithTime;			// TRUE=as m_fSecondsRemaining ticks away, the intensity is reduced

	BOOL8 m_bNeedsRestored;						// this particle group was placed in max and must be restored on checkpoint restore
	BOOL8 m_bEmissionEnabled;					// emission is currently enabled
	BOOL8 m_bBurstSoundEnabled;					// Enables the burst sound effect associated with the particle


	// used for describing a particle shape
	FParticleShape_t m_ParticleShape;
	f32 m_afRawValues[3];						// the values from the max scene, unscaled
	f32 m_afShapeValues[3];						// the scaled values that are used by the particle system 
	

//----------------------------------------------------------------------------------------------------------------------------------
// Public Functions:
//----------------------------------------------------------------------------------------------------------------------------------
public:
	// system functions:
	static BOOL InitSystem( void );
	static void UninitSystem( void );
	static BOOL IsSystemInitialized( void ) { return m_bSystemInitialized && CEntity::IsSystemInitialized(); }

	// Construct/Destruct:
	CEParticle();
	virtual ~CEParticle();

	// Create/Destroy:
	BOOL Create( cchar *pszEntityName=NULL, const CFMtx43A *pMtx=NULL, cchar *pszAIBuilderName=NULL );

	// Info:
	FINLINE const CFSphere *GetBoundingSphere(void) { FASSERT( IsCreated() ); return ((m_hEmitter != FPARTICLE_INVALID_HANDLE) ? fparticle_GetBoundingSpherePtr(m_hEmitter) : NULL); }
	FINLINE const CFWorldTracker *GetTracker(void)	{ FASSERT( IsCreated() ); return ((m_hEmitter != FPARTICLE_INVALID_HANDLE) ? fparticle_GetTrackerPtr(m_hEmitter) : NULL); }
	FINLINE BOOL IsEmitting( void ) const { FASSERT( IsCreated() ); return (m_hEmitter != FPARTICLE_INVALID_HANDLE); }

	// Particle Control functions:
	BOOL StartEmission( FParticle_DefHandle_t hParticleDef, f32 fUnitIntensity=1.0f, f32 fDuration=-1.0f, BOOL bReduceIntensityWithTime=FALSE );
	void StopEmission( BOOL bImmediately=FALSE );
	void EnableEmission( BOOL bEnable );
	void SetUnitIntensity( f32 fUnitIntensity );
	FINLINE f32  GetUnitIntensity( void ) { FASSERT( IsCreated() ); return m_fUnitIntensity; }

	// checkpoint load/save functions
	virtual void CheckpointSaveSelect( s32 nCheckpoint );
	virtual BOOL CheckpointSave( void );
	virtual void CheckpointRestore( void );


//----------------------------------------------------------------------------------------------------------------------------------
// Protected Functions:
//----------------------------------------------------------------------------------------------------------------------------------
protected:

	virtual BOOL ClassHierarchyBuild( void );
	virtual CEntityBuilder *GetLeafClassBuilder( void );
	virtual void ClassHierarchyRelocated( void *pIdentifier );
	virtual void ClassHierarchyAddToWorld( void );
	virtual void ClassHierarchyRemoveFromWorld( void );
	virtual void ClassHierarchyDestroy( void );
	virtual BOOL ClassHierarchyBuilt( void );
	virtual void ClassHierarchyResolveEntityPointerFixups( void );	
	virtual void ClassHierarchyWork( void );

//----------------------------------------------------------------------------------------------------------------------------------
// Private Functions:
//----------------------------------------------------------------------------------------------------------------------------------
private:

	void _ClearMemberData( void );

	FCLASS_STACKMEM_ALIGN( CEParticle );
} FCLASS_ALIGN_SUFFIX;




//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CEParticleBuilder
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

FCLASS_ALIGN_PREFIX class CEParticleBuilder : public CEntityBuilder {
//----------------------------------------------------------------------------------------------------------------------------------
// Public Data:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	f32 m_fDimX;
	f32 m_fDimY;
	f32 m_fDimZ;
	f32 m_fRadius;
	FWorldShapeType_e m_nShapeType;
	cchar *m_pszPartDefFile;
	f32 m_fUnitIntensity;
	BOOL8 m_bDelaySpawn;
	BOOL8 m_bBurstSoundEnabled;
	BOOL8 m_bEmitterCouldChangeVolumes;

	BOOL8 m_bEmpty;		// TRUE when created through the ::Create Methods, FALSE when created through the ::InterpreteTable Methods


//----------------------------------------------------------------------------------------------------------------------------------
// Public Functions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	FINLINE CEParticleBuilder() {}

	virtual void SetDefaults( u64 nEntityTypeBits, u64 nEntityLeafTypeBit, cchar *pszEntityType );


//----------------------------------------------------------------------------------------------------------------------------------
// Protected Functions:
//----------------------------------------------------------------------------------------------------------------------------------
protected:

	virtual BOOL InterpretTable( void );
	virtual BOOL PostInterpretFixup( void );


	FCLASS_STACKMEM_ALIGN( CEParticleBuilder );
} FCLASS_ALIGN_SUFFIX;


#endif

