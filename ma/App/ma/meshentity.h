//////////////////////////////////////////////////////////////////////////////////////
// meshentity.h - Mesh entity object.
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
// 04/17/02 Ranck       Created.
//////////////////////////////////////////////////////////////////////////////////////

#ifndef _MESHENTITY_H_
#define _MESHENTITY_H_ 1

#include "fang.h"
#include "entity.h"

class CFAnimCombiner;
class CFAnimInst;
class CMeshEntity;

typedef void MeshEntityDieUserCallback_t( CMeshEntity *pEntity, void *pUserParam );

FCLASS_ALIGN_PREFIX class CMeshEntity : public CEntity {
//----------------------------------------------------------------------------------------------------------------------------------
// Public Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	typedef enum {
		DIR_NEXT,					// Go to the next in the sequence
		DIR_PREV,					// Go to the previous in the sequence
		DIR_RAND1,					// Pick a random item from the sequence
		DIR_RAND2,					// Same as DIR_RAND but will avoid picking the same item two times in a row

		DIR_COUNT
	} Dir_e;


	typedef enum {
		ANIMFLIP_OFF,				// Current animation will continue to be used
		ANIMFLIP_MATCHMESH,			// Choose animation index that matches the current mesh and reset to the beginning of the animation each time
		ANIMFLIP_NEXT,				// When current animation cycles, go to the next animation in the list
		ANIMFLIP_PREV,				// When current animation cycles, go to the previous animation in the list
		ANIMFLIP_RAND1,				// When current animation cycles, pick a random animation next
		ANIMFLIP_RAND2,				// When current animation cycles, pick a random animation next, but don't pick the same as last time

		ANIMFLIP_COUNT
	} AnimFlip_e;




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
		_ME_FLAG_ANIM_DRIVING_MESH		= 0x00000001,	// Animation is driving the mesh bones
		_ME_FLAG_USER_ANIM_PAUSED		= 0x00000002,	// User animation is paused
		_ME_FLAG_ALWAYS_WORK			= 0x00000004,	// Always call work function
		_ME_FLAG_NO_VERLET_INIT			= 0x00000008,	// Set when a mesh entity has been created via the Create() functions
		_ME_FLAG_VEHICLE_COLL_ONLY		= 0x00000010,	// Only collide with vehicles
		_ME_FLAG_AI_AVOID				= 0x00000020,	// AI objects should avoid colliding with this object
		_ME_FLAG_LOADER_PICKUP_ENABLED	= 0x00000040,	// Loader can pick this object up
		_ME_FLAG_USERANIM_CLAMP			= 0x00000080,	// The current animation will be run in clamp mode.
		_ME_FLAG_USEANIMBLENDING		= 0x00000100,	// Whether or not animation blending is an option for this
		_ME_FLAG_TRANSLATOR_BONE		= 0x00000200,	// This object will be relocated based on the animation of its translator bone

		_ME_FLAG_NONE					= 0x00000000
	};




//----------------------------------------------------------------------------------------------------------------------------------
// Private Data:
//----------------------------------------------------------------------------------------------------------------------------------
private:

	static BOOL m_bSystemInitialized;			// TRUE = this system has been initialized

	u32 m_nMeshEntityFlags;						// See _ME_FLAG_* for info

	// Mesh stuff:
	u32 m_nWorldMeshCount;						// The number of CFWorldMesh objects in 
	CFWorldMesh *m_pWorldMeshArray;				// Array of world meshes m_pWorldMeshArray
	CFAnimCombiner **m_ppExtAnimCombinerArray;	// Array of external animation combiners (one per mesh. Each entry: NULL=use user anim if available)
	s32 m_nTranslatorBoneIndex;					// Index of the translator bone (-1=none)
	CFMtx43A *m_pTranslatorReferenceFrame;		// If a translator bone is used, this is the frame of reference

	u32 m_nCurrentWorldMeshIndex;				// The current world mesh's index
	CFWorldMesh *m_pCurrentWorldMesh;			// The current world mesh (never NULL)
	CFAnimCombiner *m_pCurrentExtAnimCombiner;	// Current external animation combiner (NULL=use user anim if available)

	// User animation stuff:
	CFAnimCombiner *m_pUserAnim_CombinerArray;	// Array of simple user animation combiners (one per mesh, NULL=none)
	CFAnimCombiner *m_pUserAnim_CurrentCombiner;// Current user animation combiner
	s32 *m_pnUserAnim_CombinerTapIDArray;		// Array of user combiner tap IDs for the user CFAnimInst objects (one per mesh, NULL=none)
	s32 *m_pnUserAnim_CombinerTapIDArray2;		// Second array of tap IDs for use when anim blending is enabled.
	s32 *m_pnUserAnim_CombinerControlIDArray;	// Array of user combiner control IDs for use when anim blending is enabled.
	u32 m_nUserAnim_InstCount;					// The number of user CFAnimInst objects in m_ppUserAnim_InstArray
	CFAnimInst **m_ppUserAnim_InstArray;		// Array of user animation instances (if entry is NULL, the animation could not be loaded)

	s32 m_nUserAnim_CurrentInstIndex;			// The current user animation instance index (-1=none)
	CFAnimInst *m_pUserAnim_CurrentInst;		// The current user animation instance (NULL=none)

	u64 m_nLastAnimTicks;						// Snapshot of FLoop_nTotalLoopTicks when user animation was last updated
	u64 m_nLastFlipTicks;						// Snapshot of FLoop_nTotalLoopTicks when mesh flip was last performed

	f32 m_fMeshFlipsPerSec;						// Mesh flips per second (0=off, <0=animloop)
	Dir_e m_nMeshFlipDir;						// Mesh flip direction
	u32 m_nMeshFlipIndex;						// Current mesh flip index
	f32 m_fMeshFlipIndex;						// Floating point version of m_nMeshFlipIndex

	AnimFlip_e m_nUserAnim_Flip;				// User animation flipping instructions
	f32 m_fUserAnim_SpeedMult;					// User animation speed multiplier
	f32 m_fUnitAnimBlend;						// Used only if animation blending is enabled, 0.0f otherwise.
	f32 m_fOOBlendTimeSecs;						// Used only if animation blending is enabled, 0.0f otherwise.
	CFAnimInst *m_pBlendOutAnimInst;			// The animation that we are blending from.  Used only if animation blending is enabled.

	MeshEntityDieUserCallback_t *m_pDieCallback;// a user supplied callback function that will be called when the entity dies
	void *m_pDieUserParam;						// a user supplied parameter that will be passed into the die user callback	


//----------------------------------------------------------------------------------------------------------------------------------
// Public Functions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	// System:
	static BOOL InitSystem( void );
	static void UninitSystem( void );
	static BOOL IsSystemInitialized( void ) { return m_bSystemInitialized && CEntity::IsSystemInitialized(); }


	// Construct/Destruct:
	CMeshEntity();
	virtual ~CMeshEntity();


	// Create/Destroy:
	BOOL Create( cchar *pszMeshName, cchar *pszAnimName=NULL, cchar *pszEntityName=NULL, const CFMtx43A *pMtx=NULL, cchar *pszAIBuilderName=NULL, BOOL bUseAnimBlending=FALSE, const CFVerlet::Init_t *pVerletInit=NULL );
	BOOL Create( cchar *pszMeshName, u32 nAnimCount, cchar **ppszAnimNames, cchar *pszEntityName=NULL, const CFMtx43A *pMtx=NULL, cchar *pszAIBuilderName=NULL, BOOL bUseAnimBlending=FALSE, const CFVerlet::Init_t *pVerletInit=NULL );
	BOOL Create( u32 nMeshCount, cchar **ppszMeshNames, u32 nAnimCount, cchar **ppszAnimNames, cchar *pszEntityName=NULL, const CFMtx43A *pMtx=NULL, cchar *pszAIBuilderName=NULL, BOOL bUseAnimBlending=FALSE, const CFVerlet::Init_t *pVerletInit=NULL );

	virtual void AppendTrackerSkipList(u32& FWorld_nTrackerSkipListCount=FWorld_nTrackerSkipListCount, CFWorldTracker ** FWorld_apTrackerSkipList=&FWorld_apTrackerSkipList[0]);

	// Mesh:
	FINLINE u32 GetMeshCount( void ) const { return m_nWorldMeshCount; }
	void SelectMesh( u32 nMeshIndex, BOOL bResetIntraFlipAmount=FALSE, BOOL bForceSelectMesh = FALSE );
	FINLINE u32 GetCurrentMeshIndex( void ) const { return m_nCurrentWorldMeshIndex; }

	FINLINE u32 GetID( void ) const { FASSERT( IsCreated() ); return m_pCurrentWorldMesh->GetID(); }
	FINLINE void SetID( u32 nID ) { FASSERT( IsCreated() ); m_pCurrentWorldMesh->SetID( nID ); }

	FINLINE BOOL IsCollisionFlagSet( void ) const { FASSERT( IsCreated() ); return m_pCurrentWorldMesh->IsCollisionFlagSet(); }
	FINLINE void SetCollisionFlag( BOOL bSetFlag ) { FASSERT( IsCreated() ); m_pCurrentWorldMesh->SetCollisionFlag( bSetFlag ); }

	FINLINE BOOL IsLineOfSightFlagSet( void ) const { FASSERT( IsCreated() ); return m_pCurrentWorldMesh->IsLineOfSightFlagSet(); }
	FINLINE void SetLineOfSightFlag( BOOL bSetFlag ) { FASSERT( IsCreated() ); m_pCurrentWorldMesh->SetLineOfSightFlag( bSetFlag ); }

	FINLINE f32 GetCullDist( void ) const { FASSERT( IsCreated() ); return m_pCurrentWorldMesh->m_fCullDist; }
	FINLINE void SetCullDist( f32 fCullDist ) { FASSERT( IsCreated() ); m_pCurrentWorldMesh->m_fCullDist = fCullDist; }

//	FINLINE s32 GetDrawSeg( void ) const { FASSERT( IsCreated() ); return m_pCurrentWorldMesh->m_nDrawSeg; }
//	FINLINE void SetDrawSeg( s32 nDrawSeg ) { FASSERT( IsCreated() ); m_pCurrentWorldMesh->m_nDrawSeg = nDrawSeg; }

	FINLINE u32 GetMeshInstFlags( void ) const { FASSERT( IsCreated() ); return m_pCurrentWorldMesh->m_nFlags; }
	FINLINE void SetMeshInstFlags( u32 nBitsToSet ) { FASSERT( IsCreated() ); FMATH_SETBITMASK( m_pCurrentWorldMesh->m_nFlags, nBitsToSet ); }
	FINLINE void ClearMeshInstFlags( u32 nBitsToClear ) { FASSERT( IsCreated() ); FMATH_CLEARBITMASK( m_pCurrentWorldMesh->m_nFlags, nBitsToClear ); }
	FINLINE void ToggleMeshInstFlags( u32 nBitsToToggle ) { FASSERT( IsCreated() ); m_pCurrentWorldMesh->m_nFlags ^= nBitsToToggle; }
	FINLINE void WriteMeshInstFlags( u32 nNewFlags ) { FASSERT( IsCreated() ); m_pCurrentWorldMesh->m_nFlags = nNewFlags; }

	FINLINE const CFSphere& GetBoundingSphere_WS( void ) const { FASSERT( IsCreated() ); return m_pCurrentWorldMesh->GetBoundingSphere(); }

	FINLINE CFWorldMesh *GetMeshInst( void ) const { FASSERT( IsCreated() ); return m_pCurrentWorldMesh; }
	FINLINE CFWorldMesh *GetMeshInst( u32 nMeshIndex ) const { FASSERT( IsCreated() ); FASSERT( nMeshIndex < m_nWorldMeshCount ); return &m_pWorldMeshArray[nMeshIndex]; }

	FINLINE f32 MeshFlip_GetFlipsPerSec( void ) const { FASSERT( IsCreated() ); return m_fMeshFlipsPerSec; }
	void MeshFlip_SetFlipsPerSec( f32 fFlipsPerSec );
	FINLINE Dir_e MeshFlip_GetFlipDirection( void ) const { FASSERT( IsCreated() ); return m_nMeshFlipDir; }
	void MeshFlip_SetFlipDirection( Dir_e nDirection );


	// Anim:
	void ComputeMtxPalette( BOOL bApplyOffscreenOptimizations );

	void DriveMeshWithAnim( BOOL bDrive );
	FINLINE BOOL IsAnimDrivingMesh( void ) const { FASSERT( IsCreated() ); return (BOOL)(m_nMeshEntityFlags & _ME_FLAG_ANIM_DRIVING_MESH); }

	FINLINE CFAnimCombiner *GetExternalCombiner( u32 nMeshIndex ) const { FASSERT( IsCreated() ); FASSERT( nMeshIndex < m_nWorldMeshCount ); return m_ppExtAnimCombinerArray[nMeshIndex]; }
	void SetExternalCombiner( u32 nMeshIndex, CFAnimCombiner *pCombiner );

	FINLINE u32 UserAnim_GetCount( void ) const { FASSERT( IsCreated() ); return m_nUserAnim_InstCount; }
	void UserAnim_Select( s32 nUserAnimIndex );
	void UserAnim_SelectWithBlend(	s32 nUserAnimIndex,
									f32 fBlendTimeSecs );
	FINLINE s32 UserAnim_GetCurrentIndex( void ) const { FASSERT( IsCreated() ); return m_nUserAnim_CurrentInstIndex; }
	FINLINE CFAnimInst *UserAnim_GetCurrentInst( void ) const { FASSERT( IsCreated() ); return m_pUserAnim_CurrentInst; }
	FINLINE CFAnimInst *UserAnim_GetInstByIndex( u32 nUserAnimIndex ) const { FASSERT( IsCreated() ); return m_ppUserAnim_InstArray[nUserAnimIndex]; }

	void UserAnim_UpdateTime( f32 fNewTime );
	void UserAnim_UpdateUnitTime( f32 fNewUnitTime );

	void UserAnim_Pause( BOOL bPause );
	FINLINE BOOL UserAnim_IsPaused( void ) const { FASSERT( IsCreated() ); return (BOOL)(m_nMeshEntityFlags & _ME_FLAG_USER_ANIM_PAUSED); }

	FINLINE void UserAnim_SetClampMode( BOOL bClampMode ) { FASSERT( IsCreated() ); m_nMeshEntityFlags &= ~_ME_FLAG_USERANIM_CLAMP; m_nMeshEntityFlags |= _ME_FLAG_USERANIM_CLAMP*bClampMode; }
	FINLINE BOOL UserAnim_GetClampMode( void ) const { FASSERT( IsCreated() ); return !!(m_nMeshEntityFlags & _ME_FLAG_USERANIM_CLAMP); }
	FINLINE BOOL UsesAnimBlending( void ) const { FASSERT( IsCreated() ); return !!(m_nMeshEntityFlags & _ME_FLAG_USEANIMBLENDING); }

	FINLINE void UserAnim_SetSpeedMult( f32 fSpeedMult ) { FASSERT( IsCreated() ); m_fUserAnim_SpeedMult = fSpeedMult; }
	FINLINE f32 UserAnim_GetAnimSpeedMult( void ) const { FASSERT( IsCreated() ); return m_fUserAnim_SpeedMult; }

	FINLINE void UserAnim_SetFlip( AnimFlip_e nAnimFlip ) { FASSERT( IsCreated() ); m_nUserAnim_Flip = nAnimFlip; }
	FINLINE AnimFlip_e UserAnim_GetFlip( void ) const { FASSERT( IsCreated() ); return m_nUserAnim_Flip; }

	virtual void CheckpointSaveSelect( s32 nCheckpoint );
	virtual BOOL CheckpointSave( void );
	virtual void CheckpointRestore( void );

	void AlwaysWork( BOOL bAlways );

	FINLINE BOOL IsVehicleCollOnly( void ) const { FASSERT( IsCreated() ); return (BOOL)(m_nMeshEntityFlags & _ME_FLAG_VEHICLE_COLL_ONLY); }
	
	FINLINE BOOL IsLoaderPickupEnabled( void ) const {FASSERT( IsCreated() ); return (BOOL)(m_nMeshEntityFlags & _ME_FLAG_LOADER_PICKUP_ENABLED); };

	FINLINE BOOL AIShouldAvoid( void ) const { FASSERT( IsCreated() ); return (BOOL)(m_nMeshEntityFlags & _ME_FLAG_AI_AVOID ); }

	FINLINE virtual CFWorldMesh *GetMesh( void ) const { FASSERT( IsCreated() ); return GetMeshInst(); }

	FINLINE void SetDieCallback( MeshEntityDieUserCallback_t *pDieCallback, void *pUserParam ) { FASSERT( IsCreated() ); m_pDieCallback = pDieCallback; m_pDieUserParam = pUserParam; }
	virtual void Die( BOOL bSpawnDeathEffects=TRUE, BOOL bSpawnGoodies=TRUE );

//----------------------------------------------------------------------------------------------------------------------------------
// Protected Functions:
//----------------------------------------------------------------------------------------------------------------------------------
protected:

	virtual void ClassHierarchyDestroy( void );

	virtual BOOL ClassHierarchyBuild( void );
	virtual BOOL ClassHierarchyBuilt( void );
	virtual CEntityBuilder *GetLeafClassBuilder( void );
	virtual void ClassHierarchyResolveEntityPointerFixups( void );

	virtual void ClassHierarchyWork( void );
	virtual void ClassHierarchyAddToWorld( void );
	virtual void ClassHierarchyRemoveFromWorld( void );
	virtual void ClassHierarchyDrawEnable( BOOL bDrawingHasBeenEnabled );
	virtual void ClassHierarchyRelocated( void *pIdentifier );

	virtual u32 TripwireCollisionTest( const CFVec3A *pPrevPos_WS, const CFVec3A *pNewPos_WS );

	virtual BOOL GetCollected( CEntity *pCollector );

	CFMtx43A *ClassHierarchyAttachChild( CEntity *pChildEntity, cchar *pszAttachBoneName );

	virtual void InflictDamage( CDamageData *pDamageData );
	virtual void InflictDamageResult( const CDamageResult *pDamageResult );

	virtual void ClassHierarchyShatterWire( CFWire *pWire, BOOL bSilent );




//----------------------------------------------------------------------------------------------------------------------------------
// Private Functions:
//----------------------------------------------------------------------------------------------------------------------------------
private:

	static void _VerletMovedCallback( CFVerlet *pVerlet, f32 fDeltaSecs );

	BOOL _DetermineList( cchar *psResType, u32 *pnCount, cchar ***pppszNameArray );
	void _ClearDataMembers( void );
	void _UpdateWorkFcn( void );
	void _SelectMesh( u32 nMeshIndex, BOOL bForceSelectMesh = FALSE );
	void _FlipMesh_Rand1( void );
	void _FlipMesh_Rand2( void );
	void _MeshHasFlipped( void );
	void _DamageTack( CFVerletTack *pTack, CDamageData *pDamageData );
	void _SetTranslatorInfoForCurrentMesh( CFWorldMesh *pPrevWorldMesh );


	FCLASS_STACKMEM_ALIGN( CMeshEntity );
} FCLASS_ALIGN_SUFFIX;




//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CMeshEntityBuilder
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

FCLASS_ALIGN_PREFIX class CMeshEntityBuilder : public CEntityBuilder {
//----------------------------------------------------------------------------------------------------------------------------------
// Public Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	enum {
		FLAG_DRIVE_MESH_WITH_ANIM			= 0x00000001,	// When set, the mesh will be driven by the animation system
		FLAG_NOANIMLOD_COMMAND_SPECIFIED	= 0x00000002,	// The "NoAnimLOD" command was specified in the user props
		FLAG_POSTERX_COMMAND_SPECIFIED		= 0x00000004,	// The "PosterX" command was specified in the user props
		FLAG_POSTERY_COMMAND_SPECIFIED		= 0x00000008,	// The "PosterY" command was specified in the user props
		FLAG_POSTERZ_COMMAND_SPECIFIED		= 0x00000010,	// The "PosterZ" command was specified in the user props
		FLAG_NOCOLL_COMMAND_SPECIFIED		= 0x00000020,	// The "NoColl" command was specified in the user props
		FLAG_NOLIGHT_COMMAND_SPECIFIED		= 0x00000080,	// The "NoLight" command was specified in the user props
		FLAG_ALWAYS_WORK					= 0x00000100,	// Always run work function
		FLAG_VEHICLE_COLL_ONLY				= 0x00000200,	// Collide with vehicles only
		FLAG_AI_AVOID						= 0x00000400,	// Collide with vehicles only
		FLAG_LOADER_PICKUP_ENABLED			= 0x00000800,	// Loader can pick this object up
		FLAG_TRANSLATOR_BONE				= 0x00001000,	// This object will be relocated based on the animation of its translator bone

		FLAG_NONE							= 0x00000000
	};




//----------------------------------------------------------------------------------------------------------------------------------
// Public Data:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	u32 m_nFlags;										// See FLAG_* for info

	u32 m_nMeshFlags;									// Mesh instance flags
	f32 m_fCullDist2;									// Mesh cull distance squared

	u32 m_nMeshCount;									// Number of meshes
	cchar **m_ppszMeshNames;							// Pointer to array of mesh names (will be added to string table automatically)
	u32 m_nUserAnimCount;								// Number of user animations
	cchar **m_ppszUserAnimNames;						// Pointer to array of user anim names (will be added to string table automatically)

	u32 m_nStartMeshIndex;								// Starting mesh index when flipping meshes (default=0)
	u32 m_nStartAnimIndex;								// Starting anim index when flipping animations (default=0)

	f32 m_fMeshFlipsPerSec;								// Mesh flips per second (0=off, <0=animloop, default=0)
	CMeshEntity::Dir_e m_nMeshFlipDir;					// Mesh flip direction (default = CMeshEntity::DIR_NEXT)

	CMeshEntity::AnimFlip_e m_nAnimFlip;				// Describes how to flip between animations in the list
	f32 m_fAnimSpeedMult;								// Animation speed multiplier (0=stopped, <0=backwards, 1=normal, etc.)

	BOOL m_bUseAnimBlending;							// Whether or not this should be built with the more complicated combiners
														//   and logic that provide blending.

	CFColorRGB m_ColorTint;								// The tint that will be applied to the mesh immediately after creation
	cchar **m_papszLightMapNames;						// array of lightmap names from shape mesh
	u16 *m_panLightMapMotifs;							// Pointer to the array of lightmap motifs from the shape mesh
	ColorStream_t *m_paColorStreams;					// Color streams from shape mesh
	u8 m_nColorStreamCount;								// Count of m_paColorStreams
	u8 __PAD[3];



//----------------------------------------------------------------------------------------------------------------------------------
// Public Functions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	FINLINE CMeshEntityBuilder() {}
	virtual void SetDefaults( u64 nEntityTypeBits, u64 nEntityLeafTypeBit, cchar *pszEntityType );




//----------------------------------------------------------------------------------------------------------------------------------
// Protected Functions:
//----------------------------------------------------------------------------------------------------------------------------------
protected:

	virtual BOOL InterpretTable( void );
	virtual BOOL PostInterpretFixup( void );


	FCLASS_STACKMEM_ALIGN( CMeshEntityBuilder );
} FCLASS_ALIGN_SUFFIX;



#endif

