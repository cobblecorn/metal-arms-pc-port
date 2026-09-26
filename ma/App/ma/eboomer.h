//////////////////////////////////////////////////////////////////////////////////////
// eboomer.h - Destructable entity class.
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
// 03/25/03 Ranck       Created.
//////////////////////////////////////////////////////////////////////////////////////

#ifndef _EBOOMER_H_
#define _EBOOMER_H_ 1

#include "fang.h"
#include "meshentity.h"




FCLASS_ALIGN_PREFIX class CEBoomer : public CMeshEntity {
//----------------------------------------------------------------------------------------------------------------------------------
// Public Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	enum {
		MAX_PARTICLE_COUNT = 4
	};




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
		BOOMER_FLAG_DETPACK_ONLY	= 0x00000001,		// This boomer will accept hitpoint damage only from a detpack
		BOOMER_FLAG_ZOMBIEBALL_ONLY	= 0x00000002,		// This boomer will accept hitpoint damage only from a zombie ball
		BOOMER_FLAG_VEHICLE_COLLIDE	= 0x00000004,		// This boomer will collide with vehicles and not be damaged by them.

		BOOMER_FLAG_NONE			= 0x00000000
	};


	typedef struct {
		FParticle_DefHandle_t hParticleDef;		// Particle definition (FPARTICLE_INVALID_HANDLE if none)
		s32 nDeadMeshBoneIndex;					// Bone index in the dead mesh to attach to (-1=spawn from center of bounding sphere)
		f32 fDuration;							// Duration of the particle
	} ParticleInfo_t;




//----------------------------------------------------------------------------------------------------------------------------------
// Private Data:
//----------------------------------------------------------------------------------------------------------------------------------
private:

	static s32 m_nDestructEvent;				// Destruct event (-1=none)

	FMeshTexLayerHandle_t m_hDeadMeshTexFlip;	// Texture layer handle that will be flipped for one cycle when the dead mesh is displayed (FMESH_TEXLAYERHANDLE_INVALID=none)

	u32 m_nBoomerFlags;							// See BOOMER_FLAG_* for info

	CDamageProfile *m_pDamageProfile;			// Damage profile to use when there is no explosion set (or to override the damage profile in the explosion) (NULL=none)
	CDamageProfile *m_pDamageOnlyProfile;		// if non-NULL, object will only respond to damage which has this damage profile

	CFSoundGroup *m_pSoundGroupBoom;			// Explosion sound to use if there is no explosion profile (NULL=none)
	CFSoundGroup *m_pSoundGroupAlive;			// Ambient sound this object makes when it's alive (NULL=none)
	CFSoundGroup *m_pSoundGroupDead;			// Ambient sound this object makes when it's dead (NULL=none)

	FExplosion_GroupHandle_t m_hExplosion;		// Explosion (FEXPLOSION_INVALID_HANDLE if none)
	s32 m_nExplosionAliveMeshBoneIndex;			// Bone index in the alive mesh to spawn the explosion from (-1=spawn from center of bounding sphere)

	ParticleInfo_t m_aParticleInfo[MAX_PARTICLE_COUNT];	// Particle information

	CFAudioEmitter *m_pAudioEmitter;			// Looping alive audio emitter (NULL=none)

	f32 m_fVehicleSlowdown;							// unit amount to reduce vehicle velocity when colliding with this object




//----------------------------------------------------------------------------------------------------------------------------------
// Public Functions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	// Construct/Destruct:
	CEBoomer();
	virtual ~CEBoomer();

	static BOOL InitLevel( void );
	static void UninitLevel( void );
	FINLINE BOOL IsDetPackOnly( void ) const { FASSERT( IsCreated() ); return( m_nBoomerFlags & BOOMER_FLAG_DETPACK_ONLY ); };
	FINLINE BOOL IsVehicleCollide( void ) const { FASSERT( IsCreated() ); return( m_nBoomerFlags & BOOMER_FLAG_VEHICLE_COLLIDE ); };
	FINLINE f32 GetVehicleSlowdown( void ) const { FASSERT( IsCreated() ); return m_fVehicleSlowdown; };




//----------------------------------------------------------------------------------------------------------------------------------
// Protected Functions:
//----------------------------------------------------------------------------------------------------------------------------------
protected:

	virtual BOOL ClassHierarchyBuild( void );
	virtual BOOL ClassHierarchyBuilt( void );
	virtual void ClassHierarchyDestroy( void );
	virtual CEntityBuilder *GetLeafClassBuilder( void );
	virtual void ClassHierarchyRelocated( void *pIdentifier );

	virtual void CheckpointRestore( void );

	virtual void Die( BOOL bSpawnDeathEffects=TRUE, BOOL bSpawnGoodies=TRUE );
	virtual void InflictDamage( CDamageData *pDamageData );




//----------------------------------------------------------------------------------------------------------------------------------
// Private Functions:
//----------------------------------------------------------------------------------------------------------------------------------
private:

	void _ClearDataMembers( void );
	void _SetupTexLayerState( void );


	FCLASS_STACKMEM_ALIGN( CEBoomer );
} FCLASS_ALIGN_SUFFIX;




//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CEBoomerBuilder
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

FCLASS_ALIGN_PREFIX class CEBoomerBuilder : public CMeshEntityBuilder {
//----------------------------------------------------------------------------------------------------------------------------------
// Public Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	typedef struct {
		cchar *apszName[2];		// [0]=explosion group name, [1]=attach bone name
	} ExplosionBuilder_t;


	typedef struct {
		cchar *apszName[2];		// [0]=particle name, [1]=attach bone name
		f32 fDuration;			// Duration of the particle effect
	} ParticleBuilder_t;




//----------------------------------------------------------------------------------------------------------------------------------
// Public Data:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	u32 m_nDeadMeshTexFlipID;		// Texture layer ID that will be flipped for one cycle when the dead mesh is displayed (0=none)

	BOOL m_bDetPackOnly;			// Can be destroyed only by damage issued from a det pack
	BOOL m_bZombieBossBallOnly;		// Can be destroyed only by damage issued from a Zombie Ball
	BOOL m_bVehicleCollide;			// When TRUE, vehicles will collide with object and will not do damage to it.

	cchar *m_pszDamageProfile;		// Damage profile to use when there is no explosion set (or to override the damage profile in the explosion) (NULL=none)
	cchar *m_pszDamageOnlyProfile;	// if set, object can only be affected by damage which has this damage profile.

	cchar *m_pszSoundGroupBoom;		// Explosion sound to use if there is no explosion profile (NULL=none)
	cchar *m_pszSoundGroupAlive;	// Ambient sound this object makes when it's alive (NULL=none)
	cchar *m_pszSoundGroupDead;		// Ambient sound this object makes when it's dead (NULL=none)

	s32 m_nVehicleSlowdown;			// percentage to reduce vehicle velocity when colliding with this object (0 to 100, 0 = no slowdown)

	ExplosionBuilder_t m_ExplosionBuilder;								// Explosion group name and the dead-mesh bone to attach it to
	ParticleBuilder_t m_aParticleBuilder[CEBoomer::MAX_PARTICLE_COUNT];	// Particle names and the dead-mesh bones to attach them to




//----------------------------------------------------------------------------------------------------------------------------------
// Public Functions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	FINLINE CEBoomerBuilder() {}
	virtual void SetDefaults( u64 nEntityTypeBits, u64 nEntityLeafTypeBit, cchar *pszEntityType );




//----------------------------------------------------------------------------------------------------------------------------------
// Protected Functions:
//----------------------------------------------------------------------------------------------------------------------------------
protected:

	virtual BOOL InterpretTable( void );
	virtual BOOL PostInterpretFixup( void );




//----------------------------------------------------------------------------------------------------------------------------------
// Private Functions:
//----------------------------------------------------------------------------------------------------------------------------------
protected:

	void _InterpretParticle( u32 nParticleIndex );


	FCLASS_STACKMEM_ALIGN( CEBoomerBuilder );
} FCLASS_ALIGN_SUFFIX;





#endif

