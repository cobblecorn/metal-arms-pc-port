//////////////////////////////////////////////////////////////////////////////////////
// ELiquidMesh.h - 
//
// Author: Jeremy Chernobieff
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
// 01/20/03 Chernobieff Created.
//////////////////////////////////////////////////////////////////////////////////////
#ifndef _ELIQUIDMESH_H_
#define _ELIQUIDMESH_H_ 1

#include "fang.h"
#include "fliquid.h"
#include "entity.h"

FCLASS_ALIGN_PREFIX class CELiquidMesh : public CEntity {
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

	f32 m_afRawValues[3];						// the values from the max scene, unscaled
	f32 m_afShapeValues[3];						// the scaled values that are used by the particle system 
	
	CFVec3A m_vExt;
	CFMtx43A m_vMtx;

	BOOL m_bStopFlow, m_bStartFlow;
	f32 m_fDeltaTime, m_fTotalTime;

	CFLiquidMesh *m_pLiquid;
	CFTexInst *m_pTexInst[2];
	
//----------------------------------------------------------------------------------------------------------------------------------
// Public Functions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	// Construct/Destruct:
	CELiquidMesh();
	virtual ~CELiquidMesh();

	// Create/Destroy:
	BOOL Create( cchar *pszEntityName=NULL, const CFMtx43A *pMtx=NULL, cchar *pszAIBuilderName=NULL );

	void StopFlow(f32 fTime);
	void StartFlow(f32 fTime);
	
	// These are optionally implemented. Most entity classes don't need these 3 static functions.
	static BOOL InitSystem( void );
	static void UninitSystem( void );
	static BOOL IsSystemInitialized( void ) { return m_bSystemInitialized && CEntity::IsSystemInitialized(); }
	

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

	//static CDamageProfile *m_pDamageProfile[3];

//----------------------------------------------------------------------------------------------------------------------------------
// Private Functions:
//----------------------------------------------------------------------------------------------------------------------------------
private:

	void _ClearMemberData( void );


	FCLASS_STACKMEM_ALIGN( CELiquidMesh );
} FCLASS_ALIGN_SUFFIX;




//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CELiquidMeshBuilder
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

FCLASS_ALIGN_PREFIX class CELiquidMeshBuilder : public CEntityBuilder {
//----------------------------------------------------------------------------------------------------------------------------------
// Public Data:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	f32 m_fDimX;
	f32 m_fDimY;
	f32 m_fDimZ;
	FWorldShapeType_e m_nShapeType;
	CFColorRGB m_Color;
	f32 m_fOpacity; //for texture type.
	f32 m_fTile;
	f32 m_fSpeed;
	f32 m_fCurvature;
	cchar *m_pszType;
	cchar *m_pszTexLayer0;
	cchar *m_pszTexLayer1;

	BOOL m_bEmpty;		// TRUE when created through the ::Create Methods, FALSE when created through the ::InterpreteTable Methods


//----------------------------------------------------------------------------------------------------------------------------------
// Public Functions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	FINLINE CELiquidMeshBuilder() {}

	virtual void SetDefaults( u64 nEntityTypeBits, u64 nEntityLeafTypeBit, cchar *pszEntityType );


//----------------------------------------------------------------------------------------------------------------------------------
// Protected Functions:
//----------------------------------------------------------------------------------------------------------------------------------
protected:

	virtual BOOL InterpretTable( void );
	virtual BOOL PostInterpretFixup( void );


	FCLASS_STACKMEM_ALIGN( CELiquidMeshBuilder );
} FCLASS_ALIGN_SUFFIX;


#endif

