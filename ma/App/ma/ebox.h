//////////////////////////////////////////////////////////////////////////////////////
// ebox.h - Generic entity box class.
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
// 04/30/02 Ranck       Created.
//////////////////////////////////////////////////////////////////////////////////////

#ifndef _EBOX_H_
#define _EBOX_H_ 1

#include "fang.h"
#include "entity.h"
#include "fmath.h"
#include "fworld.h"
#include "fverlet.h"




FCLASS_ALIGN_PREFIX class CEBox : public CEntity {
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
// Protected Data:
//----------------------------------------------------------------------------------------------------------------------------------
protected:

	CFVec3A m_HalfDim_MS;		// Box half dimensions in model space (x=width, y=height, z=depth)




//----------------------------------------------------------------------------------------------------------------------------------
// Private Data:
//----------------------------------------------------------------------------------------------------------------------------------
private:

	CFVec3A m_HalfDim_WS;		// Box dimensions in world space (x=width, y=height, z=depth)
	CFVec3A m_aCorner_WS[2];	// Two box corners in world space: [0]=minimum MS XYZ transormed to WS, [1]=maximum MS XYZ transformed to WS

	CFWorldUser m_WorldUser;	// So that we get tracker callbacks

	CFVerletTackSurface *m_pVerletTackSurface;	// This box entity could represent a collision surface




//----------------------------------------------------------------------------------------------------------------------------------
// Public Functions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	// Construct/Destruct:
	CEBox();
	virtual ~CEBox();


	// Create/Destroy:
	BOOL Create( f32 fDimX, f32 fDimY, f32 fDimZ, cchar *pszEntityName=NULL, const CFMtx43A *pMtx=NULL, cchar *pszAIBuilderName=NULL, u32 nTripwireEntityCount=0 );


	// Goodie properties functions.
	virtual void GetGoodieDistributionOrigin(CFVec3A *pPt_WS);
	virtual void GetGoodieDistributionDir(CFVec3A *pVec_WS);


	// Verlet Access Functions
	FINLINE CFVerletTackSurface *GetVerletCollisionSurface( void ) { FASSERT( IsCreated() ); return m_pVerletTackSurface; }

	BOOL IsPointWithinBox(const CFVec3A& rTestPointWS);
	BOOL IsPointWithinBoxXZ(const CFVec3A& rTestPointWS);
	void Draw( void );


	// Model Space:
	FINLINE const CFVec3A *HalfDim_MS( void ) const { FASSERT( IsCreated() ); return &m_HalfDim_MS; }
	FINLINE f32 HalfDimX_MS( void ) const { FASSERT( IsCreated() ); return m_HalfDim_MS.x; }
	FINLINE f32 HalfDimY_MS( void ) const { FASSERT( IsCreated() ); return m_HalfDim_MS.y; }
	FINLINE f32 HalfDimZ_MS( void ) const { FASSERT( IsCreated() ); return m_HalfDim_MS.z; }


	// World Space:
	FINLINE const CFVec3A *HalfDim_WS( void ) const { FASSERT( IsCreated() ); return &m_HalfDim_WS; }
	FINLINE f32 HalfDimX_WS( void ) const { FASSERT( IsCreated() ); return m_HalfDim_WS.x; }
	FINLINE f32 HalfDimY_WS( void ) const { FASSERT( IsCreated() ); return m_HalfDim_WS.y; }
	FINLINE f32 HalfDimZ_WS( void ) const { FASSERT( IsCreated() ); return m_HalfDim_WS.z; }

	FINLINE const CFVec3A *Corner_WS( void ) const { FASSERT( IsCreated() ); return m_aCorner_WS; }
	FINLINE const CFVec3A *CornerMin_WS( void ) const { FASSERT( IsCreated() ); return &m_aCorner_WS[0]; }
	FINLINE const CFVec3A *CornerMax_WS( void ) const { FASSERT( IsCreated() ); return &m_aCorner_WS[1]; }

	virtual void AppendTrackerSkipList(u32& FWorld_nTrackerSkipListCount=FWorld_nTrackerSkipListCount, CFWorldTracker ** FWorld_apTrackerSkipList=&FWorld_apTrackerSkipList[0]);




//----------------------------------------------------------------------------------------------------------------------------------
// Protected Functions:
//----------------------------------------------------------------------------------------------------------------------------------
protected:

	virtual BOOL ClassHierarchyBuild( void );
	virtual CEntityBuilder *GetLeafClassBuilder( void );
	virtual void ClassHierarchyRelocated( void *pIdentifier );
	virtual void ClassHierarchyAddToWorld( void );
	virtual void ClassHierarchyRemoveFromWorld( void );

	virtual u32 TripwireCollisionTest( const CFVec3A *pPrevPos_WS, const CFVec3A *pNewPos_WS );




//----------------------------------------------------------------------------------------------------------------------------------
// Private Functions:
//----------------------------------------------------------------------------------------------------------------------------------
private:

	void _SetDefaults( void );
	void _Move( void );
	BOOL _PosInside( const CFVec3A &vecPos );
	BOOL _PassedThroughOriginZPlane( const CFVec3A *pvecPosA, const CFVec3A *pvecPosB );


	FCLASS_STACKMEM_ALIGN( CEBox );
} FCLASS_ALIGN_SUFFIX;




//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CEBoxBuilder
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

FCLASS_ALIGN_PREFIX class CEBoxBuilder : public CEntityBuilder {
//----------------------------------------------------------------------------------------------------------------------------------
// Public Data:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	CFVec3A m_HalfDim_MS;		// Box half dimensions in model space (x=width, y=height, z=depth)

	//Verlet Surface Characteristics
	CFVerletTackSurface::Type_e m_eVerletSurfaceType; //If this value is TYPE_COUNT, then This entity is NOT a collision surface
	f32 m_fVerletSurfaceFriction;
	f32 m_fVerletSurfacePlaneDist;
	BOOL m_bVerletSurfacePreserveVelocity;
	BOOL m_bVerletSurfaceDynamic; //Determines if the surface is static or dynamic.  Static is default


//----------------------------------------------------------------------------------------------------------------------------------
// Public Functions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	FINLINE CEBoxBuilder() {}
	virtual void SetDefaults( u64 nEntityTypeBits, u64 nEntityLeafTypeBit, cchar *pszEntityType );



//----------------------------------------------------------------------------------------------------------------------------------
// Protected Functions:
//----------------------------------------------------------------------------------------------------------------------------------
protected:

	virtual BOOL InterpretTable( void );
	virtual BOOL PostInterpretFixup( void );


	FCLASS_STACKMEM_ALIGN( CEBoxBuilder );
} FCLASS_ALIGN_SUFFIX;





#endif

