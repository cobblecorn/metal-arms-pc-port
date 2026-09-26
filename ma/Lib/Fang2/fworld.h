//////////////////////////////////////////////////////////////////////////////////////
// fworld.h - Fang world module.
//
// Author: Steve Ranck     
//////////////////////////////////////////////////////////////////////////////////////
// THIS CODE IS PROPRIETARY PROPERTY OF SWINGIN' APE STUDIOS, INC.
// Copyright (c) 2000
//
// The contents of this file may not be disclosed to third
// parties, copied or duplicated in any form, in whole or in part,
// without the prior written permission of Swingin' Ape Studios, Inc.
//////////////////////////////////////////////////////////////////////////////////////
// Modification History:
//
// Date     Who         Description
// -------- ----------  --------------------------------------------------------------
// 09/12/00 Ranck       Created.
//////////////////////////////////////////////////////////////////////////////////////

#ifndef _FWORLD_H_
#define _FWORLD_H_ 1

#include "fang.h"
#include "fmesh.h"
#include "flinklist.h"
#include "fpsprite.h"

#include "fsh.h"

// if you are looking for FWORLD_RESTYPE, see fres.h - Mike



//-------------------------------------------------------------------------------------------------------------------
// General Module Definitions:
//-------------------------------------------------------------------------------------------------------------------

typedef enum 
{
	FWORLD_EVENT_WORLD_PRELOAD,			// The world is about to be loaded
	FWORLD_EVENT_WORLD_POSTLOAD,		// The world has been loaded
	FWORLD_EVENT_WORLD_PREDESTROY,		// The world is about to be destroyed
	FWORLD_EVENT_WORLD_POSTDESTROY,		// The world has been destroyed

	FWORLD_EVENT_COUNT
} FWorldEvent_e;


typedef BOOL FWorldCallback_t( FWorldEvent_e nEvent );


#define FWORLD_MAX_MESH_SHAPE_LIGHTMAPS			4			// We support up to 4 lightmaps for mesh shapes
#define FWORLD_DLBITSTREAM_COUNT_FIELD_BITS		3
#define FWORLD_DLBITSTREAM_LEVEL_FIELD_BITS		3
#define FWORLD_DLBITSTREAM_LEVEL_TERM_VALUE		((1<<FWORLD_DLBITSTREAM_LEVEL_FIELD_BITS)-1)
#define FWORLD_KEYCOUNT							3
#define FWORLD_MAX_SKIPLIST_ENTRIES				100


enum 
{
	FWORLD_SHADOWFLAG_ENABLE		= 0x00000001,	// This shadow is enabled
	FWORLD_SHADOWFLAG_NO_TERRAIN	= 0x00000002,	// Don't project onto terrain
	FWORLD_SHADOWFLAG_NO_OBJECTS	= 0x00000004,	// Don't project onto objects

	FWORLD_SHADOWFLAG_NONE
};


typedef enum 
{
	FWORLD_TRACKERTYPE_MESH,		// Mesh category (the tracker bounds the graphical object)
	FWORLD_TRACKERTYPE_LIGHT,		// Light category (the tracker bounds the light)
	FWORLD_TRACKERTYPE_PSGROUP,		// Point sprite group (the tracker bounds the group)
	FWORLD_TRACKERTYPE_USER,		// User category (the tracker bounds a user-defined object)

	FWORLD_TRACKERTYPE_COUNT
} FWorldTrackerType_e;


enum 
{
	FWORLD_TRACKERTYPEBIT_MESH		= (1<<FWORLD_TRACKERTYPE_MESH),
	FWORLD_TRACKERTYPEBIT_LIGHT		= (1<<FWORLD_TRACKERTYPE_LIGHT),
	FWORLD_TRACKERTYPEBIT_USER		= (1<<FWORLD_TRACKERTYPE_USER),

	FWORLD_TRACKERTYPEBIT_NONE		= 0
};


enum 
{
	FWORLD_TRACKER_FLAG_COLLIDE					= 0x01,	// When set, fworld_CollideRayWithTrackers() and fworld_CollideSpheresWithTrackers() will include this object
	FWORLD_TRACKER_FLAG_ADDED					= 0x02,	// This tracker is added to the world
	FWORLD_TRACKER_FLAG_LINE_OF_SIGHT			= 0x04,	// When set, this object will be included in line-of-site collision queries
	FWORLD_TRACKER_FLAG_AUTO_DELETE				= 0x08,	// When set, this tracker has been flagged for auto-deletion. When the world is destroyed, this object will also be destroyed
	FWORLD_TRACKER_FLAG_TRAVERSE_CLOSED_PORTALS	= 0x10,	// When set, this tracker is allowed to traverse closed portals

	FWORLD_TRACKER_FLAG_NONE				= 0x00
};


typedef enum 
{
	FWORLD_USERTYPE_NONE					= 0x00000000,

	FWORLD_USERTYPE_APPMIN					= 0x00000001,
	FWORLD_USERTYPE_APPMAX					= 0x7fffffff,

	FOWRLD_USERTYPE_RESERVEDNONE			= 0x80000000,
	FWORLD_USERTYPE_RESERVEDMIN				= 0x80000001,
	FWORLD_USERTYPE_RESERVEDMAX				= 0xfffffffe,
	FWORLD_USERTYPE_DONT_USE				= 0xffffffff,

	FWORLD_USERTYPE_AUDIO_EMITTER_INACTIVE	= FWORLD_USERTYPE_RESERVEDMIN,
	FWORLD_USERTYPE_AUDIO_EMITTER_2D,
	FWORLD_USERTYPE_AUDIO_EMITTER_3D,
	FWORLD_USERTYPE_AUDIO_LISTENER,

	FWORLD_USERTYPE_WIRE,
} FWorldUserType_e;


class CFWorldIntersect;
class CFWorldTracker;
class CFWorldMesh;
class CFWorldPSGroup;
struct FVisVolume_t;

typedef void FWorldCollPrepCallback_t( CFWorldMesh *pWorldMesh );
typedef BOOL FWorldPSGroupPreDrawCallback_t( CFWorldPSGroup *pPSGroup, f32 fDistToCameraSq );


// World Collision system Callback typedefs:
typedef BOOL FWorldCollideRayWithTrackersCallback_t( CFWorldTracker *pTracker, FVisVolume_t *pVolume, const CFVec3 *pIntersectionPoint_WS, f32 fUnitDistToIntersection );
typedef BOOL FWorldCollideSpheresWithTrackersCallback_t( CFWorldTracker *pTracker, FVisVolume_t *pVolume, u32 nSphereIndex );
typedef BOOL FWorldIntersectingTrackerCallback_t( CFWorldTracker *pTracker, FVisVolume_t *pVolume );
typedef BOOL FWorldVolCallbackFcn_t( FVisVolume_t *pVolume, BOOL bUsingVisData );


//-------------------------------------------------------------------------------------------------------------------
// CFWorldKey:
//-------------------------------------------------------------------------------------------------------------------
class CFWorldKey 
{
	public:
		FINLINE CFWorldKey();

		static void InitSystem( void ) 
		{
			for( u32 i=0; i<FWORLD_KEYCOUNT; i++ ) 
			{
				m_abInUse[i] = FALSE;
				m_anGlobalVisitedKey[i] = 1;
			}
		};

		static FINLINE s32 OpenKey( void );
		static FINLINE void CloseKey( s32 nKeyIndex ) { m_abInUse[nKeyIndex] = FALSE; }

		static FINLINE void ResetAllKeys( void ) { for( s32 i=0; i<FWORLD_KEYCOUNT; i++ ) ResetKey( i ); }
		static FINLINE void ResetKey( s32 nKeyIndex ) { m_anGlobalVisitedKey[nKeyIndex] = 0; }
		static FINLINE void IncKey( s32 nKeyIndex ) { FASSERT( m_abInUse[nKeyIndex] ); m_anGlobalVisitedKey[nKeyIndex]++; }
		FINLINE void FlagAsVisited( s32 nKeyIndex ) { FASSERT( m_abInUse[nKeyIndex] ); m_anVisitedKey[nKeyIndex] = m_anGlobalVisitedKey[nKeyIndex]; }
		FINLINE void FlagAsNotVisited( s32 nKeyIndex ) { FASSERT( m_abInUse[nKeyIndex] ); m_anVisitedKey[nKeyIndex] = m_anGlobalVisitedKey[nKeyIndex] - 1; }
		FINLINE BOOL HaveVisited( s32 nKeyIndex ) const { FASSERT( m_abInUse[nKeyIndex] ); return (m_anVisitedKey[nKeyIndex] == m_anGlobalVisitedKey[nKeyIndex]); }

	protected:
		u32 m_anVisitedKey[FWORLD_KEYCOUNT];

		static BOOL m_abInUse[FWORLD_KEYCOUNT];
		static u32 m_anGlobalVisitedKey[FWORLD_KEYCOUNT];

	FCLASS_STACKMEM_NOALIGN( CFWorldKey );
};

//
//
FINLINE CFWorldKey::CFWorldKey() 
{
	for( u32 i=0; i<FWORLD_KEYCOUNT; i++ ) 
	{
		m_anVisitedKey[i] = m_anGlobalVisitedKey[i] - 1;
	}
}

//
//
FINLINE s32 CFWorldKey::OpenKey( void ) 
{
	u32 i;

	for( i=0; i<FWORLD_KEYCOUNT; i++ ) 
	{
		if( !m_abInUse[i] ) 
		{
			m_abInUse[i] = TRUE;
			IncKey( (s32)i );
			return (s32)i;
		}
	}

	DEVPRINTF( "CFWorldKey::OpenKey(): No more world keys available.\n" );
	return -1;
}



//-------------------------------------------------------------------------------------------------------------------
// CFWorldIntersect:
//-------------------------------------------------------------------------------------------------------------------
class CFWorldIntersect 
{
	public:
		// Construction / Destruction:
		FINLINE CFWorldIntersect() { m_pTracker = NULL; }

		// Interface:
		FINLINE static CFWorldIntersect *Create( void ) { return (CFWorldIntersect *)flinklist_RemoveTail( &m_FreeIntersectRoot ); }
		FINLINE static CFWorldIntersect *Create( CFWorldTracker *pTracker, FVisVolume_t *pVolume );
		FINLINE void Destroy( void ) { RemoveFromWorld(); ReturnToFreePool(); }

		void AddToWorld( CFWorldTracker *pTracker, FVisVolume_t *pVolume );
		void RemoveFromWorld( void );

		FINLINE FVisVolume_t *GetVolume( void ) { return m_pVolume; }
		FINLINE CFWorldTracker *GetTracker( void ) { return m_pTracker; }

		static FINLINE s32 GetOffsetOfTrackerInNodeLink( void ) { return (s32)FANG_OFFSETOF( CFWorldIntersect, m_TrackerInNodeLink ); }
		static void InitPool( CFWorldIntersect *pPoolBase, u32 nMaxIntersects );

	private:
		FINLINE void ReturnToFreePool( void ) { flinklist_AddTail( &m_FreeIntersectRoot, this ); }

		// Data:
		static CFWorldIntersect *m_pIntersectFreePool;	// Pointer to pool of free CFWorldIntersect objects
		static FLinkRoot_t m_FreeIntersectRoot;			// Linklist of free CFWorldIntersects in _pIntersectFreePool

		union {
			FLink_t m_PoolLink;					// When not attached to a tracker, this is the link to other CFWorldIntersects in the free list
			FLink_t m_TrackerInNodeLink;		// Link to previous and next tracker in pWorldNode
		};

		FLink_t m_NodeInTrackerLink;		// Link to previous and next node in pTracker
		FVisVolume_t *m_pVolume;			// Pointer to intersecting Volume
		CFWorldTracker *m_pTracker;			// Pointer to intersecting tracker

		// Friends:
		friend class CFWorldTracker;

	FCLASS_STACKMEM_NOALIGN( CFWorldIntersect );
};


FINLINE CFWorldIntersect *CFWorldIntersect::Create( CFWorldTracker *pTracker, FVisVolume_t *pVolume ) 
{
	CFWorldIntersect *pWorldIntersect;

	pWorldIntersect = Create();
	if( pWorldIntersect ) 
	{
		pWorldIntersect->AddToWorld( pTracker, pVolume );
	}

	return pWorldIntersect;
}




//-------------------------------------------------------------------------------------------------------------------
// CFWorldTracker:
//-------------------------------------------------------------------------------------------------------------------
class CFWorldTracker 
{
	public:
		// Construction / Destruction:
		CFWorldTracker( FWorldTrackerType_e nTrackerType );
		virtual ~CFWorldTracker();

		// Interface:
		FINLINE void MoveTracker( const CFSphere& rNewTightSphere_WS, BOOL bCalcNewVolume = TRUE );
		void AddToWorld( void );
		virtual void RemoveFromWorld( void );

		FINLINE FWorldTrackerType_e GetType( void ) const { return (FWorldTrackerType_e)m_nTrackerType; }
		FINLINE const CFSphere& GetBoundingSphere( void ) const { return m_BoundSphere_WS; }
		FINLINE BOOL IsAddedToWorld( void ) const { return !!(m_nTrackerFlags & FWORLD_TRACKER_FLAG_ADDED); }
		FINLINE void SetTraverseClosedPortalsFlag( BOOL bSetFlag ) { if( bSetFlag ) { m_nTrackerFlags |= FWORLD_TRACKER_FLAG_TRAVERSE_CLOSED_PORTALS; } else { m_nTrackerFlags &= ~FWORLD_TRACKER_FLAG_TRAVERSE_CLOSED_PORTALS; } }
		FINLINE BOOL IsTraverseClosedPortalsSet( void ) const { return !!(m_nTrackerFlags & FWORLD_TRACKER_FLAG_TRAVERSE_CLOSED_PORTALS); }
		FINLINE void SetCollisionFlag( BOOL bSetFlag ) { if( bSetFlag ) { m_nTrackerFlags |= FWORLD_TRACKER_FLAG_COLLIDE; } else { m_nTrackerFlags &= ~FWORLD_TRACKER_FLAG_COLLIDE; } }
		FINLINE BOOL IsCollisionFlagSet( void ) const { return !!(m_nTrackerFlags & FWORLD_TRACKER_FLAG_COLLIDE); }
		FINLINE void SetLineOfSightFlag( BOOL bSetFlag ) { if( bSetFlag ) { m_nTrackerFlags |= FWORLD_TRACKER_FLAG_LINE_OF_SIGHT; } else { m_nTrackerFlags &= ~FWORLD_TRACKER_FLAG_LINE_OF_SIGHT; } }
		FINLINE BOOL IsLineOfSightFlagSet( void ) const { return !!(m_nTrackerFlags & FWORLD_TRACKER_FLAG_LINE_OF_SIGHT); }
		FINLINE void SetAutoDelFlag( BOOL bSetFlag ) { if( bSetFlag ) { m_nTrackerFlags |= FWORLD_TRACKER_FLAG_AUTO_DELETE; } else { m_nTrackerFlags &= ~FWORLD_TRACKER_FLAG_AUTO_DELETE; } }
		FINLINE BOOL IsAutoDelFlagSet( void ) const { return !!(m_nTrackerFlags & FWORLD_TRACKER_FLAG_AUTO_DELETE); }

		static FINLINE s32 GetOffsetOfTrackerLink( void ) { return (s32)FANG_OFFSETOF( CFWorldTracker, m_TrackerLink ); }
		static FINLINE s32 GetOffsetOfTrackerDelLink( void ) { return (s32)FANG_OFFSETOF( CFWorldTracker, m_TrackerDelLink ); }

		FINLINE CFWorldIntersect *GetFirstIntersect( void ) const { return (CFWorldIntersect *)flinklist_GetHead( &m_IntersectList ); }
		FINLINE CFWorldIntersect *GetLastIntersect( void ) const { return (CFWorldIntersect *)flinklist_GetTail( &m_IntersectList ); }
		FINLINE CFWorldIntersect *GetNextIntersect( CFWorldIntersect *pIntersect ) const { return (CFWorldIntersect *)flinklist_GetNext( &m_IntersectList, pIntersect ); }
		FINLINE CFWorldIntersect *GetPrevIntersect( CFWorldIntersect *pIntersect ) const { return (CFWorldIntersect *)flinklist_GetPrev( &m_IntersectList, pIntersect ); }
		FINLINE u32 GetIntersectCount( void ) const { return m_IntersectList.nCount; }

		BOOL FindIntersectingTrackers( FWorldIntersectingTrackerCallback_t *pCallback );
		BOOL FindIntersectingTrackers( FWorldIntersectingTrackerCallback_t *pCallback, FWorldTrackerType_e nTypeToFind );

		FINLINE void SetUserTypeBits( u64 nBits ) { FASSERT( !(nBits & 0x8000000000000000) ); m_nUserTypeBits = nBits; }
		FINLINE u64  GetUserTypeBits( void ) { return m_nUserTypeBits; }

		u32 GetVolumeFlags( void );
		FVisVolume_t* GetCenterpointVolume( void ) 
		{ 
			CFWorldIntersect *pIntersect = GetFirstIntersect();
			if ( !pIntersect )
			{
				return NULL;
			}
			return pIntersect->GetVolume(); 
		}
		BOOL VolumeBecameActive( void );
		BOOL VolumeBecameInactive( void );

		static FINLINE CFWorldTracker *GetFirstTracker( void ) { return (CFWorldTracker *)flinklist_GetHead( &m_TrackerRoot ); }
		static FINLINE CFWorldTracker *GetLastTracker( void ) { return (CFWorldTracker *)flinklist_GetTail( &m_TrackerRoot ); }
		static FINLINE CFWorldTracker *GetNextTracker( CFWorldTracker *pTracker ) { return (CFWorldTracker *)flinklist_GetNext( &m_TrackerRoot, pTracker ); }
		static FINLINE CFWorldTracker *GetPrevTracker( CFWorldTracker *pTracker ) { return (CFWorldTracker *)flinklist_GetPrev( &m_TrackerRoot, pTracker ); }
		static FINLINE u32 GetTrackerCount( void ) { return m_TrackerRoot.nCount; }

		static FINLINE CFWorldTracker *GetFirstTrackerDel( void ) { return (CFWorldTracker *)flinklist_GetHead( &m_TrackerDelRoot ); }
		static FINLINE CFWorldTracker *GetLastTrackerDel( void ) { return (CFWorldTracker *)flinklist_GetTail( &m_TrackerDelRoot ); }
		static FINLINE CFWorldTracker *GetNextTrackerDel( CFWorldTracker *pTracker ) { return (CFWorldTracker *)flinklist_GetNext( &m_TrackerDelRoot, pTracker ); }
		static FINLINE CFWorldTracker *GetPrevTrackerDel( CFWorldTracker *pTracker ) { return (CFWorldTracker *)flinklist_GetPrev( &m_TrackerDelRoot, pTracker ); }
		static FINLINE u32 GetTrackerDelCount( void ) { return m_TrackerDelRoot.nCount; }
		static void DeleteAllFlagedTrackers( void );

		static void InitTrackerLists( void );

	private:
		FINLINE CFWorldTracker() {}			// This is used only by the FCLASS_STACKMEM_NOALIGN macro

		// Data:
	public:
		CFWorldKey m_VisitedKey;			// Used to track whether this tracker has been visited by a function before
		void *m_pUser;						// General purpose field available to application (set to NULL by CFWorldTracker::CFWorldTracker)
		u32 m_nUser;						// General purpose field available to application (set to 0 by CFWorldTracker::CFWorldTracker)
		u32 m_nLastFrameMoved;				// Holds the frame count (FVid_nFrameCounter) for the last frame this tracker was moved

	private:
		u64 m_nUserTypeBits;				// Bit field available to allow user to specify type definition (Used by collision system for fast object culling - set to 0x8000000000000000 by default, 0 results in no collision)
		u8 m_nTrackerType;					// Tracker type (this field is of type FWorldTrackerType_e)
		u8 m_nTrackerFlags;					// Tracker flags (see FWORLD_TRACKER_FLAG_* for info)

		FLinkRoot_t m_IntersectList;		// Linklist of FWorldIntersect_t structures
		CFSphere m_BoundSphere_WS;			// Current bounding sphere in world space
		FLink_t m_TrackerLink;				// Linklist of CFWorldTracker objects that are currently in the world
		FLink_t m_TrackerDelLink;			// Linklist of CFWorldTracker objects that have their auto-delete flag set

		static FLinkRoot_t m_TrackerRoot;	// Linklist of all trackers that are currently in the world
		static FLinkRoot_t m_TrackerDelRoot;// Linklist of all trackers that have been created after the world has been loaded

		// Friends:
		friend class CFWorldIntersect;

	FCLASS_STACKMEM_NOALIGN( CFWorldTracker );
};


FINLINE void CFWorldTracker::MoveTracker( const CFSphere& rNewBoundSphere_WS, BOOL bCalcNewVolume ) 
{
	FASSERT( rNewBoundSphere_WS.m_fRadius >= 0.0f );

	if( (rNewBoundSphere_WS != m_BoundSphere_WS) || !IsAddedToWorld() ) 
	{
		// Set our new sphere...
		m_BoundSphere_WS = rNewBoundSphere_WS;

		// Record this frame as the last frame updated
		m_nLastFrameMoved = FVid_nFrameCounter;

		if ( bCalcNewVolume || !IsAddedToWorld() )
		{
			AddToWorld();
		}
	}
}



//-------------------------------------------------------------------------------------------------------------------
// CFWorldMeshDirLight:
//-------------------------------------------------------------------------------------------------------------------
class CFWorldMeshDirLight 
{
	public:
		// Construction / Destruction:
		CFWorldMeshDirLight( void );

		// Data:
		CFColorMotif m_Motif;			// Light color motif (RGBA components range from 0.0f to 1.0f). Alpha is not used.
		f32 m_fIntensity;				// Light intensity to be multiplied by each component (0.0f to 1.0f) (Set to 0.0f to indicate no dir light)
		CFVec3 m_Dir_WS;				// Light direction (away from source) in world space (dir and spot). Need not be unit.

		FCLASS_STACKMEM_NOALIGN( CFWorldMeshDirLight );
};


class CFWorldAttachedLight;

//-------------------------------------------------------------------------------------------------------------------
// CFWorldMesh:
//-------------------------------------------------------------------------------------------------------------------
FCLASS_ALIGN_PREFIX class CFWorldMesh : public CFWorldTracker, public CFMeshInst 
{
	public:
		// Construction / Destruction:
		CFWorldMesh();
		~CFWorldMesh() { DestroyAllAttachedLights(); }

		// Interface:
		void Init( FMesh_t *pMesh, BOOL bAddAttachedLights=TRUE, BOOL bAnimWhenPaused = FALSE, u32 nDrawnPartsMask = 0xffffffff, u32 nCollisionPartsMask = 0xffffffff, BOOL bNoMemAlloc = FALSE );
		void Init( FMeshInit_t *pMeshInit, BOOL bAddAttachedLights=TRUE, u32 nDrawnPartsMask = 0xffffffff, u32 nCollisionPartsMask = 0xffffffff, BOOL bNoMemAlloc = FALSE );
		BOOL FindIntersectingWorldLights( FWorldIntersectingTrackerCallback_t *pCallback );
		BOOL FindIntersectingWorldMeshes( FWorldIntersectingTrackerCallback_t *pCallback );
		static CFWorldMesh *FindByName( cchar *pszMeshName );
		FINLINE u32 GetID( void ) const { return m_nID; }
		FINLINE void SetID( u32 nID ) { m_nID = nID; }

		void UpdateTracker( BOOL bCalcNewVolume = TRUE );
		void RemoveFromWorld( void );

		FINLINE CFWorldAttachedLight *GetFirstAttachedLight( void ) { return (CFWorldAttachedLight *)flinklist_GetHead( &m_LightsRoot ); }
		FINLINE CFWorldAttachedLight *GetLastAttachedLight( void ) { return (CFWorldAttachedLight *)flinklist_GetTail( &m_LightsRoot ); }
		FINLINE CFWorldAttachedLight *GetNextAttachedLight( CFWorldAttachedLight *pWorldAttachedLight ) { return (CFWorldAttachedLight *)flinklist_GetNext( &m_LightsRoot, pWorldAttachedLight ); }
		FINLINE CFWorldAttachedLight *GetPrevAttachedLight( CFWorldAttachedLight *pWorldAttachedLight ) { return (CFWorldAttachedLight *)flinklist_GetPrev( &m_LightsRoot, pWorldAttachedLight ); }
		FINLINE u32 GetAttachedLightCount( void ) const { return m_LightsRoot.nCount; }
		FINLINE void AddAttachedLight( CFWorldAttachedLight *pWorldAttachedLight ) { flinklist_AddTail( &m_LightsRoot, pWorldAttachedLight ); }
		FINLINE void RemoveAttachedLight( CFWorldAttachedLight *pWorldAttachedLight ) { flinklist_Remove( &m_LightsRoot, pWorldAttachedLight ); }

		CFWorldAttachedLight* GetAttachedLightByID( u16 nLightID );

		FINLINE void SetCollPrepCallback( FWorldCollPrepCallback_t *pFcnCallback ) { m_pFcnCollPrepCallback = pFcnCallback; }
		FINLINE FWorldCollPrepCallback_t *GetCollPrepCallback( void ) const { return m_pFcnCollPrepCallback; }
		FINLINE void CallPreCollCallback( void );

		FINLINE BOOL CollideWithMeshTris( CFCollInfo *pCollInfo );

		static FINLINE void ResetDrawnKey( void ) { m_nGlobalDrawnKey = 1; }
		static FINLINE void IncDrawnKey( void ) { m_nGlobalDrawnKey++; }
		FINLINE void FlagAsDrawn( void ) { m_nDrawnKey = m_nGlobalDrawnKey; }
		FINLINE void FlagAsNotDrawn( void ) { m_nDrawnKey = m_nGlobalDrawnKey-1; }
		FINLINE BOOL HaveDrawn( void ) const { return m_nDrawnKey == m_nGlobalDrawnKey; }

	private:
		void AddAttachedMeshLightsToWorld( void );
		void DestroyAttachedLight( CFWorldAttachedLight *pWorldAttachedLight );
		void DestroyAllAttachedLights( void );

	// DATA:
	public:
		CFColorRGB m_AmbientRGB;		// Ambient light to use when drawing this instance
		CFWorldMeshDirLight m_DirLight;	// Directional light to use when drawing this instance (if fIntensity is 0.0f, there is no dir light on this object)

	private:
		static u32 m_nGlobalDrawnKey;	// Used to track whether CFWorldMeshInst objects have been drawn or not

		FLinkRoot_t m_LightsRoot;					// Linklist of CFWorldAttachedLight's attached to this mesh
		u32 m_nID;									// Parsed from FWorldMeshInit_t::pszUserData (set to 0 if a valid ID= command was not found)
		FWorldCollPrepCallback_t *m_pFcnCollPrepCallback;	// If not NULL, called before collision testing is performed on this world mesh

		u32 m_nDrawnKey;				// Used to track whether this CFWorldMeshInst object has been drawn before

	FCLASS_STACKMEM_ALIGN( CFWorldMesh );
} FCLASS_ALIGN_SUFFIX;


//
//
FINLINE BOOL CFWorldMesh::CollideWithMeshTris( CFCollInfo *pCollInfo )
{
	CallPreCollCallback();

	return CFMeshInst::CollideWithMeshTris( pCollInfo );
}

//
//
FINLINE void CFWorldMesh::CallPreCollCallback( void )
{
	if( m_pFcnCollPrepCallback )
	{
		m_pFcnCollPrepCallback( this );
	}
}


//-------------------------------------------------------------------------------------------------------------------
// CFWorldLight:
//-------------------------------------------------------------------------------------------------------------------
#ifndef _MMI_TARGET_PS2	//ARG
class CFWorldLight : public CFWorldTracker 
//ARG - >>>>>
#else
FCLASS_ALIGN_PREFIX class CFWorldLight : public CFWorldTracker
#endif
//ARG - <<<<<
{
	public:
		// Construction / Destruction:
		FINLINE CFWorldLight() : CFWorldTracker( FWORLD_TRACKERTYPE_LIGHT ) {}

		// Interface:
		FINLINE void Init( const FLightInit_t *pLightInit );
		FINLINE void UpdateTracker( BOOL bCalcNewVolume = TRUE ) { MoveTracker( m_Light.m_spInfluence_WS, bCalcNewVolume ); }

		// Fills out the parameter structure with the information necessary to clone this light...
				void GetInit(FLightInit_t* pLightInit );

		virtual void UpdateOrientationAndPosition( void ) {};

		static CFWorldLight *FindByName( cchar *pszLightName );

		// Data:
		CFLight		m_Light;				// Light data
		CFWorldKey	m_VisitedKey;			// Used to track whether this light has been visited by a function before
		u32			m_nVisibileLightIdx;	// Filled out by fvis when the light is added to the visible light list (Only valid is visited key is set)

#ifndef _MMI_TARGET_PS2	//ARG
	FCLASS_STACKMEM_NOALIGN( CFWorldLight );
};
//ARG - >>>>>
#else
	FCLASS_STACKMEM_ALIGN( CFWorldLight );
} FCLASS_ALIGN_SUFFIX;
#endif
//ARG - <<<<<


//
//
FINLINE void CFWorldLight::Init( const FLightInit_t *pLightInit ) 
{
	FASSERT( pLightInit->nType==FLIGHT_TYPE_OMNI || pLightInit->nType==FLIGHT_TYPE_SPOT );
	m_Light.Init( pLightInit );
	UpdateTracker();
}


//-------------------------------------------------------------------------------------------------------------------
// CFWorldAttachedLight:
//-------------------------------------------------------------------------------------------------------------------

// NOTE: ANY OVERRIDES MUST BE DONE AFTER THE LIGHT HAS ATTACHED MESH HAS BEEN MOVED!
// IF OVERRIDES ARE DONE BEFORE IT'S MOVED, THEN THE LIGHT COULD POSSIBLY BE IN THE WRONG
// POSITION/ORIENTATION!

enum 
{
	FWORLDATTACHEDLIGHT_OVERRIDE_POSITION = 0x01,
	FWORLDATTACHEDLIGHT_OVERRIDE_ORIENTATION = 0x02,
	FWORLDATTACHEDLIGHT_MESH_ALLOCATED_LIGHT_IMPLICITELY = 0x04		// Set when the world mesh fnew'd this light (it will fdelete it when the mesh is fdeleted)
};

#ifndef _MMI_TARGET_PS2	//ARG
class CFWorldAttachedLight : public CFWorldLight 
//ARG - >>>>>
#else
FCLASS_ALIGN_PREFIX class CFWorldAttachedLight : public CFWorldLight
#endif
//ARG - <<<<<
{
	public:
		// Construction / Destruction:
		FINLINE CFWorldAttachedLight() : CFWorldLight() {}

		// Interface:
		void Init( const FLightInit_t *pLightInit, const CFWorldMesh *pParentMesh );

		FINLINE void UpdateTracker( void );
		FINLINE void ResetLightToIdentity( void );  // This is used to reset the light position and orientation to identity, PERMANENTLY

		FINLINE u8 GetOverrideFlags( void ) const { return m_nFlags; }
		FINLINE void SetOverrideFlags( u8 nOverrideFlags ) { FMATH_SETBITMASK( m_nFlags, nOverrideFlags ); }
		FINLINE void ClearOverrideFlags( u8 nClearFlags ) { FMATH_CLEARBITMASK( m_nFlags, nClearFlags ); }
		FINLINE void SetWSOverrideVars( CFVec3 *pOverridePos, CFMtx43 *pOverrideOrientation ); //overrides the light position and orientation in WS

		//these functions return the world space light values IGNORING THE OVERRIDE FLAGS
		void GetWSAttachedOrientation( CFMtx43 *pWSMtx );
		void GetWSAttachedPosition( CFVec3 *pWSPosVec );

		static FINLINE s32 GetOffsetOfLink( void ) { return (s32)FANG_OFFSETOF( CFWorldAttachedLight, m_Link ); }
		virtual void UpdateOrientationAndPosition( void );

		// Data:
	private:
		const CFWorldMesh *m_pParentMesh;		// The mesh this light is attached to
		s8			m_nParentBoneIdx;	// The parent mesh bone index, if any, this light is attached to
		u8			m_nFlags;			// Flags for the light
		u8			__PAD[2];
		CFSphere	m_spInfluence_MS;	// Light center and radius in the model space of the CFWorldMesh it's attached to
		CFMtx43		m_mtxOrientation_MS;// Light orientation relative to the parent bone mtx or the model space of the CFWorldMesh it's attached to (direction is m_vFront)

		FLink_t		m_Link;				// For maintaining linklists of attached lights in the CFWorldMesh

#ifndef _MMI_TARGET_PS2	//ARG
	FCLASS_STACKMEM_NOALIGN( CFWorldAttachedLight );
};
//ARG - >>>>>
#else
	FCLASS_STACKMEM_ALIGN( CFWorldAttachedLight );
} FCLASS_ALIGN_SUFFIX;
#endif
//ARG - <<<<<

//
//
FINLINE void CFWorldAttachedLight::UpdateTracker( void ) 
{
	UpdateOrientationAndPosition();
	CFWorldLight::UpdateTracker();
}

//
//
// Note that this will get rid of position and orientation of any light supplied by PASM
// and detach the light from it's parent bone
FINLINE void CFWorldAttachedLight::ResetLightToIdentity( void )
{
	m_nParentBoneIdx = -1;
	m_spInfluence_MS.m_Pos.Set( 0.f, 0.f, 0.f );
	m_mtxOrientation_MS.Identity();
	CFWorldLight::UpdateTracker();
}

//
//
FINLINE void CFWorldAttachedLight::SetWSOverrideVars( CFVec3 *pOverridePosVec, CFMtx43 *pOverrideOrientationMtx )
{
	if( pOverridePosVec ) 
	{
		FASSERT ( m_nFlags & FWORLDATTACHEDLIGHT_OVERRIDE_POSITION );
		m_Light.m_spInfluence_WS.m_Pos = *pOverridePosVec;
	}

	if( pOverrideOrientationMtx ) 
	{
		FASSERT ( m_nFlags & FWORLDATTACHEDLIGHT_OVERRIDE_ORIENTATION );
		m_Light.m_mtxOrientation_WS = *pOverrideOrientationMtx;
	}
}



//-------------------------------------------------------------------------------------------------------------------
// CFWorldPSGroup:
//-------------------------------------------------------------------------------------------------------------------
FCLASS_ALIGN_PREFIX class CFWorldPSGroup : public CFWorldTracker, public CFPSpriteGroup 
{
	public:
		// Construction / Destruction:
		FINLINE CFWorldPSGroup() : CFWorldTracker( FWORLD_TRACKERTYPE_PSGROUP ), CFPSpriteGroup() { FlagAsNotDrawn(); m_pPreDrawCallback=NULL; }

		// Interface:
		FINLINE void UpdateTracker( BOOL bCalcNewVolume = TRUE ) 
		{ 
			CFSphere BoundSphere_WS; 
			m_Xfm.TransformSphereF( BoundSphere_WS, m_BoundSphere_MS ); 
			MoveTracker( BoundSphere_WS, bCalcNewVolume ); 
		}

		static FINLINE void ResetDrawnKey( void ) { m_nGlobalDrawnKey = 1; }
		static FINLINE void IncDrawnKey( void ) { m_nGlobalDrawnKey++; }
		FINLINE void FlagAsDrawn( void ) { m_nDrawnKey = m_nGlobalDrawnKey; }
		FINLINE void FlagAsNotDrawn( void ) { m_nDrawnKey = m_nGlobalDrawnKey-1; }
		FINLINE BOOL HaveDrawn( void ) const { return m_nDrawnKey == m_nGlobalDrawnKey; }

		FINLINE void SetPreDrawCallback( FWorldPSGroupPreDrawCallback_t *pFcnCallback ) { m_pPreDrawCallback = pFcnCallback; }
		FINLINE FWorldPSGroupPreDrawCallback_t *GetPreDrawCallback( void ) const { return m_pPreDrawCallback; }

	protected:
		FWorldPSGroupPreDrawCallback_t *m_pPreDrawCallback;

	private:
		u32 m_nDrawnKey;				// Used to track whether this CFWorldPSGroup object has been drawn before
		static u32 m_nGlobalDrawnKey;	// Used to track whether CFWorldPSGroup objects have been drawn or not

	FCLASS_STACKMEM_ALIGN( CFWorldPSGroup );
} FCLASS_ALIGN_SUFFIX;



//-------------------------------------------------------------------------------------------------------------------
// CFWorldUser:
//-------------------------------------------------------------------------------------------------------------------
class CFWorldUser : public CFWorldTracker 
{
	public:
		// Construction / Destruction:
		FINLINE CFWorldUser() : CFWorldTracker( FWORLD_TRACKERTYPE_USER ) {}

	FCLASS_STACKMEM_NOALIGN( CFWorldUser );
};

//-------------------------------------------------------------------------------------------------------------------
// CFWorldShapeInit:
//-------------------------------------------------------------------------------------------------------------------

typedef enum 
{
	FWORLD_SHAPETYPE_POINT,
	FWORLD_SHAPETYPE_LINE,
	FWORLD_SHAPETYPE_SPLINE,
	FWORLD_SHAPETYPE_BOX,
	FWORLD_SHAPETYPE_SPHERE,
	FWORLD_SHAPETYPE_CYLINDER,
	FWORLD_SHAPETYPE_MESH,

	FWORLD_SHAPETYPE_COUNT
} FWorldShapeType_e;



// Point Shape:
//
// CFWorldShapeInit::m_Mtx43 defines the point and orientation.
// There is no class defined for point objects because the the point
// information is contained in CFWorldShapeInit::m_Mtx43 alone.



// Line Shape:
//
// CFWorldShapeInit::m_Mtx43.m_vPos is the first point.
// CFWorldShapeInit::m_Mtx43.m_vFront is the line's unit direction away from the first point.
// CFWorldShapeLine::m_fLength is the line's length along m_Mtx43.m_vFront.
//
// CFWorldShapeInit::m_Mtx43.m_vUp and CFWorldShapeInit::m_Mtx43.Right are undefined.
FCLASS_NOALIGN_PREFIX class CFWorldShapeLine 
{
	public:
		f32 m_fLength;

		//////////////////////////////////////////////////////////////////////////////
		// THIS FUNCTION MUST BE PRESENT AND MUST BE UPDATED FOR PASM TO WORK PROPERLY
		//////////////////////////////////////////////////////////////////////////////
		// call to change from Big to Little Endian and visa versa
		void ChangeEndian() 
		{
			m_fLength = fang_ConvertEndian( m_fLength );
		}

	FCLASS_STACKMEM_NOALIGN( CFWorldShapeLine );
} FCLASS_NOALIGN_SUFFIX;



// Spline Shape:
//
// CFWorldShapeInit::m_Mtx43 is a copy of the first spline point.
// CFWorldShapeSpline::m_nPointCount is the number of points on the spline.
// CFWorldShapeSpline::m_pPtArray is a pointer to the spline point array.
FCLASS_NOALIGN_PREFIX class CFWorldShapeSpline 
{
	public:
		u32 m_nPointCount;
		BOOL m_bClosedSpline;// if TRUE, connect the last and first points yielding m_nPointCount segments, 
							// if FALSE, don't connect the last and first points yielding m_nPointCount-1 segments
		CFVec3 *m_pPtArray;

		//////////////////////////////////////////////////////////////////////////////
		// THIS FUNCTION MUST BE PRESENT AND MUST BE UPDATED FOR PASM TO WORK PROPERLY
		//////////////////////////////////////////////////////////////////////////////
		// call to change from Big to Little Endian and visa versa
		void ChangeEndian() 
		{
			m_nPointCount = fang_ConvertEndian( m_nPointCount );
			m_bClosedSpline = (BOOL)fang_ConvertEndian( (u32)m_bClosedSpline );
			m_pPtArray = (CFVec3 *)fang_ConvertEndian( m_pPtArray );
		}

	FCLASS_STACKMEM_NOALIGN( CFWorldShapeSpline );
} FCLASS_NOALIGN_SUFFIX;



// Box Shape:
//
// CFWorldShapeInit::m_Mtx43 defines the box's center and orientation.
// CFWorldShapeBox::m_fDimX is the dimension of the box along CFWorldShapeInit::m_Mtx43.m_vX.
// CFWorldShapeBox::m_fDimY is the dimension of the box along CFWorldShapeInit::m_Mtx43.m_vY.
// CFWorldShapeBox::m_fDimZ is the dimension of the box along CFWorldShapeInit::m_Mtx43.m_vZ.
FCLASS_NOALIGN_PREFIX class CFWorldShapeBox 
{
	public:
		f32 m_fDimX;
		f32 m_fDimY;
		f32 m_fDimZ;

		//////////////////////////////////////////////////////////////////////////////
		// THIS FUNCTION MUST BE PRESENT AND MUST BE UPDATED FOR PASM TO WORK PROPERLY
		//////////////////////////////////////////////////////////////////////////////
		// call to change from Big to Little Endian and visa versa
		void ChangeEndian() 
		{ 
			m_fDimX = fang_ConvertEndian( m_fDimX );
			m_fDimY = fang_ConvertEndian( m_fDimY );
			m_fDimZ = fang_ConvertEndian( m_fDimZ );
		}

	FCLASS_STACKMEM_NOALIGN( CFWorldShapeBox );
} FCLASS_NOALIGN_SUFFIX;



// Sphere Shape:
//
// CFWorldShapeInit::m_Mtx43 defines the sphere's center and orientation.
// CFWorldShapeSphere::m_fRadius is the sphere's radius.
FCLASS_NOALIGN_PREFIX class CFWorldShapeSphere 
{
	public:
		f32 m_fRadius;

		//////////////////////////////////////////////////////////////////////////////
		// THIS FUNCTION MUST BE PRESENT AND MUST BE UPDATED FOR PASM TO WORK PROPERLY
		//////////////////////////////////////////////////////////////////////////////
		// call to change from Big to Little Endian and visa versa
		void ChangeEndian() 
		{ 
			m_fRadius = fang_ConvertEndian( m_fRadius );		
		}

	FCLASS_STACKMEM_NOALIGN( CFWorldShapeSphere );
} FCLASS_NOALIGN_SUFFIX;



// Cylinder Shape:
//
// CFWorldShapeInit::m_Mtx43.m_vPos is the center of one endcap (circle) of the cylinder.
// CFWorldShapeInit::m_Mtx43.m_vY points from CFWorldShapeInit::m_Mtx43.m_vPos toward the other end of the cylinder (the cylinder's Y axis).
// CFWorldShapeInit::m_Mtx43.m_vX and CFWorldShapeInit::m_Mtx43.m_vZ indicate the spin around the cylinder's Y axis.
// CFWorldShapeCylinder::m_fRadius is the cylinder's radius.
// CFWorldShapeCylinder::m_fDimY is the dimension of the cylinder along its Y axis.
FCLASS_NOALIGN_PREFIX class CFWorldShapeCylinder 
{
	public:
		f32 m_fRadius;
		f32 m_fDimY;

		//////////////////////////////////////////////////////////////////////////////
		// THIS FUNCTION MUST BE PRESENT AND MUST BE UPDATED FOR PASM TO WORK PROPERLY
		//////////////////////////////////////////////////////////////////////////////
		// call to change from Big to Little Endian and visa versa
		void ChangeEndian() 
		{ 
			m_fRadius = fang_ConvertEndian( m_fRadius );
			m_fDimY = fang_ConvertEndian( m_fDimY );
		}

	FCLASS_STACKMEM_NOALIGN( CFWorldShapeCylinder );
} FCLASS_NOALIGN_SUFFIX;


struct ColorStream_t
{
	u16  nVBIndex; // vertex buffer index
	u16  nColorCount;
	void *paVertexColors; // pointer to color data (one color for each vertex in a platform specific format)
};

// Mesh Shape:
//
// CFWorldShapeInit::m_Mtx43 defines the mesh's center and orientation.
// CFWorldShapeMesh::m_pszMeshName points to the mesh resource ASCIIZ name.
// CFWorldShapeMesh::m_pszLightMapName points to the mesh's lightmap resource ASCIIZ name.
// CFWorldShapeMesh::m_nMeshInstFlags is any number of FMESHINST_FLAG_* flag bits.
// CFWorldShapeMesh::m_fCullDist2 is the maximum distance squared that this object will be displayed.
FCLASS_NOALIGN_PREFIX class CFWorldShapeMesh 
{
	public:
		cchar *m_pszMeshName;
		cchar *m_papszLightMapName[FWORLD_MAX_MESH_SHAPE_LIGHTMAPS];
		u16 m_anLightMapMotif[FWORLD_MAX_MESH_SHAPE_LIGHTMAPS];
		u32 m_nMeshInstFlags;
		f32 m_fCullDist2;
		CFColorRGB m_TintRGB;  // Tint to be applied to the mesh
		u8  m_nColorStreamCount;
		u8  __PAD[3];
		ColorStream_t *m_paColorStreams;

		//////////////////////////////////////////////////////////////////////////////
		// THIS FUNCTION MUST BE PRESENT AND MUST BE UPDATED FOR PASM TO WORK PROPERLY
		//////////////////////////////////////////////////////////////////////////////
		// call to change from Big to Little Endian and visa versa
		void ChangeEndian( void ) 
		{ 
			m_pszMeshName = (cchar *)fang_ConvertEndian( (void *)m_pszMeshName );
			m_nMeshInstFlags = fang_ConvertEndian( m_nMeshInstFlags );
			u32 i;
			for ( i = 0; i < FWORLD_MAX_MESH_SHAPE_LIGHTMAPS; i++ )
			{
				m_papszLightMapName[i] = (cchar *)fang_ConvertEndian( (void *)m_papszLightMapName[i] );
				m_anLightMapMotif[i] = fang_ConvertEndian( m_anLightMapMotif[i] );
			}
			m_fCullDist2 = fang_ConvertEndian( m_fCullDist2 );
			m_nColorStreamCount = fang_ConvertEndian( m_nColorStreamCount );
			m_paColorStreams = (ColorStream_t *)fang_ConvertEndian( m_paColorStreams );
			m_TintRGB.ChangeEndian();
		}

	FCLASS_STACKMEM_NOALIGN( CFWorldShapeMesh );
} FCLASS_NOALIGN_SUFFIX;



FCLASS_NOALIGN_PREFIX class CFWorldShapeInit 
{
	public:

		FWorldShapeType_e m_nShapeType;	// The shape type code

		union 
		{							// Pointer to the type-specific shape object:
			void						*m_pShape;
			CFWorldShapeLine			*m_pLine;
			CFWorldShapeSpline			*m_pSpline;
			CFWorldShapeBox				*m_pBox;
			CFWorldShapeSphere			*m_pSphere;
			CFWorldShapeCylinder		*m_pCylinder;
			CFWorldShapeMesh			*m_pMesh;
		};

		CFMtx43 m_Mtx43;				// Orientation of the shape (Mesh shapes may have non-unit scale, but all other shapes will ALWAYS have m_Mtx43 unit scale)
		s32 m_nParentShapeIndex;		// The index of the CFWorldShapeInit in the init table that this shape is attached to (-1=not attached)
		void *m_pGameData;				// Pointer to game-specific data (NULL=none)

		//////////////////////////////////////////////////////////////////////////////
		// THIS FUNCTION MUST BE PRESENT AND MUST BE UPDATED FOR PASM TO WORK PROPERLY
		//////////////////////////////////////////////////////////////////////////////
		// call to change from Big to Little Endian and visa versa
		void ChangeEndian() 
		{ 
			m_nShapeType = (FWorldShapeType_e)fang_ConvertEndian( (u32)m_nShapeType );
			m_pShape = fang_ConvertEndian( m_pShape );
			m_Mtx43.ChangeEndian();
			m_nParentShapeIndex = fang_ConvertEndian( m_nParentShapeIndex );
			m_pGameData = fang_ConvertEndian( m_pGameData );
		}

	FCLASS_STACKMEM_NOALIGN( CFWorldShapeInit );
} FCLASS_NOALIGN_SUFFIX;


// Returns TRUE if successful, or FALSE if the game should be shut down (catastrophic failure).
typedef BOOL FWorldInitShapeArrayCallback_t( cchar *pszWorldResName, const void *pFixupOffsetBase, const CFWorldShapeInit *pShapeInitArray, u32 nShapeInitCount );


//-------------------------------------------------------------------------------------------------------------------
// Module Global Variables:
//-------------------------------------------------------------------------------------------------------------------
struct FVisData_t;
extern FVisData_t *FWorld_pWorld;		// Pointer to the one and only world object (NULL = world not created)

extern u32 FWorld_nTrackerSkipListCount;
extern CFWorldTracker *FWorld_apTrackerSkipList[FWORLD_MAX_SKIPLIST_ENTRIES];



//-------------------------------------------------------------------------------------------------------------------
// Module Interface:
//-------------------------------------------------------------------------------------------------------------------

// Initialization Interface:
extern BOOL fworld_ModuleStartup( void );
extern void fworld_ModuleShutdown( void );

void fworld_RegisterWorldCallbackFunction( FWorldCallback_t *pFcnWorldCallback );
void fworld_UnregisterWorldCallbackFunction( FWorldCallback_t *pFcnWorldCallback );
BOOL fworld_IsWorldCallbackFunctionRegistered( FWorldCallback_t *pFcnWorldCallback );

extern BOOL fworld_AnnouncePreLoad( void );
extern BOOL fworld_AnnouncePostLoad( void );
extern void fworld_AnnouncePreDestroy( void );
extern void fworld_AnnouncePostDestroy( void );


extern FWorldInitShapeArrayCallback_t *fworld_SetShapeCreateCallback( FWorldInitShapeArrayCallback_t *pInitShapeCallback );
extern FWorldInitShapeArrayCallback_t *fworld_GetShapeCreateCallback( void );
extern FWorldInitShapeArrayCallback_t *fworld_GetMeshCreateCallback( void );

extern BOOL fworld_CreateGameObjects( cchar *pszWorldResName, const void *pFixupOffsetBase, const CFWorldShapeInit *pShapeInitArray, u32 nShapeInitCount );

extern BOOL fworld_GetToolMode( void );
extern BOOL fworld_SetToolMode( BOOL bEnableToolMode );


// Ambient Light Interface:
extern CFColorMotif FWorld_AmbientMotif;
extern void fworld_Ambient_Set( float fRed, float fGreen, float fBlue, float fIntensity=1.0f );
extern void fworld_Ambient_Set( const CFColorRGB *pColorRGB, float fIntensity=1.0f );
extern void fworld_Ambient_Get( CFColorRGB *pColorRGB, float *pfIntensity );

extern BOOL fworld_EnableFog( BOOL bEnable );
extern BOOL fworld_IsFogEnabled( void );

//extern void fworld_Work( void );

extern BOOL fworld_LightPoint( const CFVec3A *pPoint_WS, CFColorRGB *pDestColor, BOOL bIncludeAmbient, const CFWorldTracker *pStartHint = NULL );
extern BOOL fworld_LightDirectionalPoint( const CFVec3A *pPoint_WS, const CFVec3A *pUnitNormal_WS, CFColorRGB *pDestColor, BOOL bIncludeAmbient, const CFWorldTracker *pStartHint = NULL );


#endif
