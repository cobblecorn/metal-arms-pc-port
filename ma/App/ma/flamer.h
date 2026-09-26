//////////////////////////////////////////////////////////////////////////////////////
// flamer.h - Flaming stream.
//
// Author: Steve Ranck     
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
// 09/11/02 Ranck       Created.
//////////////////////////////////////////////////////////////////////////////////////

#ifndef _FLAMER_H_
#define _FLAMER_H_ 1

#include "fang.h"

#include "flinklist.h"
#include "fmath.h"
#include "fcoll.h"
#include "fdraw.h"
#include "ftex.h"
#include "smoketrail.h"
#include "fpsprite.h"
#include "fparticle.h"
#include "eparticle.h"




class CFlamer;
class CFXfm;
class CWeapon;
class CDamageProfile;


// Layer 0 is innermost.
#define FLAMER_LAYER_COUNT			6



FCLASS_ALIGN_PREFIX class CFlamerNode {
private:

	CFVec3A m_Pos_WS;								// Position in world space
	CFVec3A m_InitialVel_WS;						// Initial velocity in world space
	CFVec3A m_CurrentVel_WS;						// Current velocity in world space

	f32 m_fUnitLife_WS;								// Unit life of node (0=born, 1=dead)
	f32 m_afHalfWidth_WS[FLAMER_LAYER_COUNT];		// Half width of the node
	f32 m_afWidthSpeed_WS[FLAMER_LAYER_COUNT];		// Change in width speed
	f32 m_afTC[FLAMER_LAYER_COUNT];					// Texture coordinate for each layer
	f32 m_afAlpha[FLAMER_LAYER_COUNT];				// Alpha value for the node

	BOOL8 m_bTerminalNode;							// TRUE = this is a terminal node
	BOOL8 m_bEmitterNode;							// TRUE = this is an emitter node


	friend class CFlamer;

	FCLASS_STACKMEM_ALIGN( CFlamerNode );
} FCLASS_ALIGN_SUFFIX;



FCLASS_ALIGN_PREFIX class CFlamerPart {
private:

	CFVec3A m_Pos_WS;								// Position in world space
	CFVec3A m_Vel_WS;								// Velocity of particle in world space

	f32 m_fScale_WS;								// Scale in world space (-1 = free slot)
	f32 m_fAlpha_WS;								// Alpha in world space


	friend class CFlamer;

	FCLASS_STACKMEM_ALIGN( CFlamerPart );
} FCLASS_ALIGN_SUFFIX;






FCLASS_ALIGN_PREFIX class CFlamer {
//----------------------------------------------------------------------------------------------------------------------------------
// Public Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	enum {
		FLAMER_DEFAULT_STREAM_NODE_COUNT	= 50,
	};




//----------------------------------------------------------------------------------------------------------------------------------
// Protected Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
protected:




//----------------------------------------------------------------------------------------------------------------------------------
// Private Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
private:

	enum {
		_FLAME_PARTICLE_TYPE_COUNT	= 3,
		_CROSS_SECTION_CORNER_COUNT = 6,
		_PARTICLE_COUNT				= 200,
	};


	enum {
		FLAG_CREATED			= 0x00000001,		// This CFlamer has been created
		FLAG_STREAM_ON			= 0x00000002,		// The flamer stream is on

		FLAG_NONE				= 0x00000000
	};




//----------------------------------------------------------------------------------------------------------------------------------
// Private Data:
//----------------------------------------------------------------------------------------------------------------------------------
private:

	static BOOL m_bSystemInitialized;
	static FLinkRoot_t m_LinkRoot;
	static CFTexInst m_MainTexDef;
	static CFTexInst m_ParticleTexInst;
	static CFTexInst m_aParticleTexDef[_FLAME_PARTICLE_TYPE_COUNT];
	static cchar *m_apszParticleTexName[_FLAME_PARTICLE_TYPE_COUNT];
	static BOOL m_bCanDraw;
	static FDrawVtx_t *m_pVtxArray;
	static FDrawVtx_t *m_pNextVtx;			// Next available vertex
	static const CFXfm *m_pCamXfm;			// Camera referse Xfm in world space
	static CFVec3A m_aCornerUnitVec[_CROSS_SECTION_CORNER_COUNT+1];
	static f32 m_afCornerTC[_CROSS_SECTION_CORNER_COUNT+1];
	static CFVec3A *m_pNextCornerUnitVec;
	static SmokeTrailAttrib_t m_SmokeTrailAttrib;
	static FPSprite_t *m_pPSpriteArray;		// Array of point sprites
	static FParticle_DefHandle_t m_hParticleDef;
	static FParticle_DefHandle_t m_hParticleSmokeDef;

	CWeapon *m_pOwnerWeapon;				// The weapon that spawned us
	const CDamageProfile *m_pDamageProfileHit;	// Damage profile for when the flame first hits him (triangle-level)
	const CDamageProfile *m_pDamageProfileBurn;	// Damage profile for when the flame is burning (ambient-level)

	CFPSpriteGroup *m_pPSGroup;				// Our point sprite group for our particles
	u32 m_nActivePartCount;					// Number of active particles in m_aPartArray
	CFlamerPart m_aPartArray[_PARTICLE_COUNT];

	FLink_t m_Link;							// Link to other flamers

	u32 m_nFlags;							// See FLAG_* for info

	u32 m_nMaxFlamerNodeCount;				// Number of elements in m_pStreamNode
	CFlamerNode *m_pFlamerNodeArray;		// Array of flamer nodes

	f32 m_afCurrentTC[FLAMER_LAYER_COUNT];	// Current texture coordinate
	u32 m_nAliveNodeCount;					// Number of alive nodes
	u32 m_nNextAvailNodeIndex;				// Index into m_pFlamerNodeArray of the next available node
	u32 m_nOldestAliveNodeIndex;			// Index into m_pFlamerNodeArray of the oldest alive node
	u32 m_nEmitterNodeIndex;				// Node index of our emitter (valid only when the FLAG_STREAM_ON flag is set)

	f32 m_fSecsUntilNextSmokeSpawn;			// Seconds remaining until we spawn more smoke
	f32 m_fSecsUntilNextNodeSpawn;			// Seconds remaining until we create a new node
	CFlamerNode m_EmitterNode_WS;			// Emitter node

	f32 m_fSecsUntilNextBurner;				// Seconds remaining until we spawn another burner
	
	FSndFx_FxHandle_t m_hSoundFire;
	FSndFx_FxHandle_t m_hSoundOff;
	CFAudioEmitter *m_pSoundFireEmitter;
	u32 m_uAISoundHandle;					//handle to an AI sound that is used to notify AI that a fire is happening here.




//----------------------------------------------------------------------------------------------------------------------------------
// Public Functions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	// System:
	CFlamer();
	~CFlamer();

	static BOOL InitSystem( void );
	static void UninitSystem( void );
	static void _SetSmokeTrailAttributes( void );

	static void DrawAll( void );
	static void WorkAll( void );

	BOOL Create( CWeapon *pOwnerWeapon, const CDamageProfile *pDamageProfileHit, const CDamageProfile *pDamageProfileBurn, u32 nStreamNodeCount = FLAMER_DEFAULT_STREAM_NODE_COUNT, FSndFx_FxHandle_t hSoundFire=NULL, FSndFx_FxHandle_t hSoundOff=NULL );
	void Destroy( void );
	FINLINE BOOL IsCreated( void ) const { return m_nFlags & FLAG_CREATED; }

	BOOL TriggerWork( f32 fTriggerValue, const CFVec3A *pUnitEmitterDir_WS, const CFVec3A *pEmitterPos_WS, const CFVec3A *pUserVel_WS );

	void StopFlamer( void );


//----------------------------------------------------------------------------------------------------------------------------------
// Protected Functions:
//----------------------------------------------------------------------------------------------------------------------------------
protected:




//----------------------------------------------------------------------------------------------------------------------------------
// Private Functions:
//----------------------------------------------------------------------------------------------------------------------------------
private:

	void _ClearDataMembers( void );

	void _CreateInitialCornerUnitDirs( const CFVec3A *pUnitVecFromPrevToCurrentNode );
	void _ReplaceNode( CFlamerNode *pNode, const CFVec3A *pUnitEmitterDir_WS, const CFVec3A *pEmitterPos_WS, const CFVec3A *pUserVel_WS, BOOL bTerminalNode, BOOL bEmitterNode, BOOL bCreateNewColor=FALSE );
	void _CreateNewNode( const CFVec3A *pUnitEmitterDir_WS, const CFVec3A *pEmitterPos_WS, const CFVec3A *pUserVel_WS, BOOL bTerminalNode, BOOL bEmitterNode );
	void _Work( void );
	void _Draw( void );
	void _DrawParticles( void );
	void _SpawnSmokeTrail( void );
	void _ParticleWork( void );
	void _SpawnBurners( void );


	FCLASS_STACKMEM_ALIGN( CFlamer );
} FCLASS_ALIGN_SUFFIX;


#endif

