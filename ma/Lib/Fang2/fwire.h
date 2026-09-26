//////////////////////////////////////////////////////////////////////////////////////
// FWire.h - Wire rendering module.
//
// Author: Steve Ranck   
//////////////////////////////////////////////////////////////////////////////////////
// THIS CODE IS PROPRIETARY PROPERTY OF SWINGIN' APE STUDIOS, INC.
// Copyright (c) 2001
//
// The contents of this file may not be disclosed to third
// parties, copied or duplicated in any form, in whole or in part,
// without the prior written permission of Swingin' Ape Studios, Inc.
//////////////////////////////////////////////////////////////////////////////////////
// Modification History:
//
// Date     Who         Description
// -------- ----------  --------------------------------------------------------------
// 03/30/03 Ranck       Created.
//////////////////////////////////////////////////////////////////////////////////////
#ifndef _FWIRE_H_
#define _FWIRE_H_ 1

#include "fang.h"
#include "fworld.h"
#include "flinklist.h"
#include "fdebris.h"
#include "fworld_coll.h"


class CFWire;
class CFMeshPool;
class CFWorldMeshItem;
class CFSoundGroup;


#define FWIRE_MAX_WIRE_TARGETS	4




//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CFWire
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

FCLASS_ALIGN_PREFIX class CFWire {
//----------------------------------------------------------------------------------------------------------------------------------
// Public Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	class CCreateInfo {
	public:
		cchar *m_pszWireMeshName;			// The name of the one and only wire mesh
		cchar *m_pszDebrisMeshName;			// The name of the one and only debris mesh

		cchar *m_pszSoundGroupSlice;		// Slice wire sound group name

		u32 m_nWiresPerTexture;				// Number of wires in the wire mesh's texture
		u32 m_nWirePoolCount;				// Number of wires in the pool
		u32 m_nTexLayerID;					// The scrollable texture layer ID


		FINLINE CCreateInfo() { SetToDefaults(); }
		void SetToDefaults( void );
	};




//----------------------------------------------------------------------------------------------------------------------------------
// Private Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
private:

	FCLASS_ALIGN_PREFIX class CWireTargetInfo {
	public:

		CFVec3A m_PointOnWire_WS;
		CFWorldMesh *m_pWorldMesh;
		f32 m_fDistToCamera2;


		FCLASS_STACKMEM_ALIGN( CFWire );
	} FCLASS_ALIGN_SUFFIX;




//----------------------------------------------------------------------------------------------------------------------------------
// Public Data:
//----------------------------------------------------------------------------------------------------------------------------------
public:




//----------------------------------------------------------------------------------------------------------------------------------
// Private Data:
//----------------------------------------------------------------------------------------------------------------------------------
private:

	static BOOL m_bSystemInitialized;				// TRUE when ModuleStartup() has been successfully called
	static BOOL m_bCreated;							// Wire pool created

	static u32 m_nWirePoolCount;					// Number of wires in our pool
	static CFWire *m_pWireArray;					// Array of wire objects (matches one-to-one with the world mesh pool's array)

	static CFMeshPool *m_pMeshPool;					// Our pool of wire world meshes

	static u32 m_nWiresPerTexture;					// Number of wire versions in the texture
	static f32 m_fWireVersionDeltaT;				// Amount to offset the texture T coordinate for a wire version step in the texture

	static f32 m_fInvGeoLength;						// Inverse length of the wire geometry
	static FMeshTexLayerHandle_t m_hTexLayer;		// Handle to the texture layer we're going to scroll

	static CFSoundGroup *m_pSoundGroupSlice;		// Slice sound group (NULL=none)

	static CFDebrisDef m_DebrisDef;					// Debris definition for our wire chunks
	static SmokeTrailAttrib_t m_SmokeTrailAttrib;	// Smoke trail attributes

	static u32 m_nWireTargetCount;
	static CWireTargetInfo m_aWireTargetInfo[FWIRE_MAX_WIRE_TARGETS];

	static CFTrackerCollideRayInfo m_TrackerCollideRayInfo;	// Used for targeting collision:
	static const CFVec3A *m_pStartPoint_WS;
	static const CFVec3A *m_pEndPoint_WS;
	static CFVec3A m_StartToEndUnitVec_WS;


	u16 m_nMeshPoolIndex;							// Wire index into the mesh pool
	u8 m_nWireGeoIndex;								// Tells us which wire version to use
	void *m_pOwner;									// User-provided owner pointer




//----------------------------------------------------------------------------------------------------------------------------------
// Public Functions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	// System:
	static BOOL ModuleStartup( void );
	static void ModuleShutdown( void );
	static FINLINE BOOL IsModuleInitialized( void ) { return m_bSystemInitialized; }

	static FINLINE BOOL IsCreated( void ) { return (BOOL)m_pMeshPool; }
	static BOOL Create( const CCreateInfo *pCreateInfo );
	static void Destroy( void );

	static CFWire *GetFromFreePool( BOOL bMakeCollidable=FALSE, u8 nWireGeoIndex=0, void *pOwner=NULL );
	static void ReturnToFreePool( CFWire *pWire );

	void *GetOwner( void ) const { FASSERT( IsCreated() ); return m_pOwner; }

	BOOL UpdateEndPoints( const CFVec3A *pStart, const CFVec3A *pEnd, BOOL bReturnToPoolIfTooSmall );
	void EnableCollision( BOOL bEnableCollision );

	static void Shatter( CFWire *pWire );

	static BOOL Targeting( const CFVec3A *pRayStart_WS, const CFVec3A *pRayEnd_WS, f32 fCamAdjustZ, CFVec3A *pTargetedPointOnWire_WS, CFWorldMesh **ppTargetedWireWorldMesh );




//----------------------------------------------------------------------------------------------------------------------------------
// Private Functions:
//----------------------------------------------------------------------------------------------------------------------------------
private:

	static void _ClearStaticMembers( void );
	static void _SetSmokeTrailAttributes( void );
	static CFWorldMeshItem *_StealWire( void );
	static void _CollPrepCallback( CFWorldMesh *pWorldMesh );
	static BOOL _FindTrackersCallback( CFWorldTracker *pTracker, FVisVolume_t *pVolume, const CFVec3 *pIntersectionPoint_WS, f32 fUnitDistToIntersection );
	static f32 _ComputeDistBetweenLines( const CFVec3A *pPosA_WS, const CFVec3A *pUnitDirA_WS, const CFVec3A *pPosB_WS, const CFVec3A *pUnitDirB_WS );


	FCLASS_STACKMEM_ALIGN( CFWire );
} FCLASS_ALIGN_SUFFIX;



#endif
