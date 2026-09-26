//////////////////////////////////////////////////////////////////////////////////////
// esphere.h - Generic entity sphere class.
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

#ifndef _ESPHERE_H_
#define _ESPHERE_H_ 1

#include "fang.h"
#include "entity.h"
#include "fmath.h"




FCLASS_ALIGN_PREFIX class CESphere : public CEntity {
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

	CFSphereA m_SphereA_WS;				// The sphere in world space
	f32 m_fRadius_MS;					// Radius in model space




//----------------------------------------------------------------------------------------------------------------------------------
// Public Functions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	// Construct/Destruct:
	CESphere();
	virtual ~CESphere();


	// Create/Destroy:
	BOOL Create( f32 fRadius, cchar *pszEntityName=NULL, const CFMtx43A *pMtx=NULL, cchar *pszAIBuilderName=NULL, u32 nTripwireEntityCount=0 );

	FINLINE const CFSphereA *Sphere_WS( void ) const { return &m_SphereA_WS; }
	FINLINE f32 Radius_MS( void ) const { return m_fRadius_MS; }


	// Goodie properties functions.
	virtual void GetGoodieDistributionOrigin(CFVec3A *pPt_WS);
	virtual void GetGoodieDistributionDir(CFVec3A *pVec_WS);



//----------------------------------------------------------------------------------------------------------------------------------
// Protected Functions:
//----------------------------------------------------------------------------------------------------------------------------------
protected:

	virtual BOOL ClassHierarchyBuild( void );				// Must be implemented by all classes in the derivation chain
	virtual CEntityBuilder *GetLeafClassBuilder( void );	// Must be implemented by all classes in the derivation chain
	void ClassHierarchyRelocated( void *pIdentifier );

	virtual u32 TripwireCollisionTest( const CFVec3A *pPrevPos_WS, const CFVec3A *pNewPos_WS );




//----------------------------------------------------------------------------------------------------------------------------------
// Private Functions:
//----------------------------------------------------------------------------------------------------------------------------------
private:

	void _SetDefaults( void );
	u32 _PerformTripwireCollisionTest( const CFVec3A *pPrevPos_WS, const CFVec3A *pNewPos_WS );
	BOOL _PassedThroughOriginZPlane( const CFVec3A *pvecPosA, const CFVec3A *pvecPosB );


	FCLASS_STACKMEM_ALIGN( CESphere );
} FCLASS_ALIGN_SUFFIX;




//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CESphereBuilder
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

FCLASS_ALIGN_PREFIX class CESphereBuilder : public CEntityBuilder {
//----------------------------------------------------------------------------------------------------------------------------------
// Public Data:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	f32 m_fRadius;




//----------------------------------------------------------------------------------------------------------------------------------
// Public Functions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	FINLINE CESphereBuilder() {}
	virtual void SetDefaults( u64 nEntityTypeBits, u64 nEntityLeafTypeBit, cchar *pszEntityType );




//----------------------------------------------------------------------------------------------------------------------------------
// Protected Functions:
//----------------------------------------------------------------------------------------------------------------------------------
protected:

	virtual BOOL InterpretTable( void );
	virtual BOOL PostInterpretFixup( void );


	FCLASS_STACKMEM_ALIGN( CESphereBuilder );
} FCLASS_ALIGN_SUFFIX;





#endif

