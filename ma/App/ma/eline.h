//////////////////////////////////////////////////////////////////////////////////////
// eline.h - Generic entity Line class.
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

#ifndef _ELINE_H_
#define _ELINE_H_ 1

#include "fang.h"
#include "entity.h"
#include "fmath.h"




FCLASS_ALIGN_PREFIX class CELine : public CEntity {
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
	CFVec3A m_aPoint_WS[2];				// The start [0] and end [1] points of the line in world space
	f32 m_fLength_MS;					// The length of the line in model space
	f32 m_fLength_WS;					// The length of the line in world space




//----------------------------------------------------------------------------------------------------------------------------------
// Public Functions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	// Construct/Destruct:
	CELine();
	virtual ~CELine();


	// Create/Destroy:
	BOOL Create( f32 fLength_MS, cchar *pszEntityName=NULL, const CFMtx43A *pMtx=NULL, cchar *pszAIBuilderName=NULL, u32 nTripwireEntityCount=0 );


	// Goodie properties functions.
	virtual void GetGoodieDistributionOrigin(CFVec3A *pPt_WS);
	virtual void GetGoodieDistributionDir(CFVec3A *pVec_WS);


	// World Space:
	FINLINE f32 Length_WS( void ) const { FASSERT( IsCreated() ); return m_fLength_WS; }
	FINLINE const CFVec3A *Points_WS( void ) const { FASSERT( IsCreated() ); return m_aPoint_WS; }
	FINLINE const CFVec3A *StartPoint_WS( void ) const { FASSERT( IsCreated() ); return &m_aPoint_WS[0]; }
	FINLINE const CFVec3A *EndPoint_WS( void ) const { FASSERT( IsCreated() ); return &m_aPoint_WS[1]; }




//----------------------------------------------------------------------------------------------------------------------------------
// Protected Functions:
//----------------------------------------------------------------------------------------------------------------------------------
protected:

	virtual BOOL ClassHierarchyBuild( void );				// Must be implemented by all classes in the derivation chain
	virtual CEntityBuilder *GetLeafClassBuilder( void );	// Must be implemented by all classes in the derivation chain
	virtual void ClassHierarchyRelocated( void *pIdentifier );
	virtual void ClassHierarchyAddToWorld( void );




//----------------------------------------------------------------------------------------------------------------------------------
// Private Functions:
//----------------------------------------------------------------------------------------------------------------------------------
private:

	void _Move( void );


	FCLASS_STACKMEM_ALIGN( CELine );
} FCLASS_ALIGN_SUFFIX;




//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CELineBuilder
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

FCLASS_ALIGN_PREFIX class CELineBuilder : public CEntityBuilder {
//----------------------------------------------------------------------------------------------------------------------------------
// Public Data:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	f32 m_fLength_MS;			// The length of the line in model space




//----------------------------------------------------------------------------------------------------------------------------------
// Public Functions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	FINLINE CELineBuilder() {}
	virtual void SetDefaults( u64 nEntityTypeBits, u64 nEntityLeafTypeBit, cchar *pszEntityType );




//----------------------------------------------------------------------------------------------------------------------------------
// Protected Functions:
//----------------------------------------------------------------------------------------------------------------------------------
protected:

	virtual BOOL InterpretTable( void );
	virtual BOOL PostInterpretFixup( void );


	FCLASS_STACKMEM_ALIGN( CELineBuilder );
} FCLASS_ALIGN_SUFFIX;





#endif

