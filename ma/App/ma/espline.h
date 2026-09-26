//////////////////////////////////////////////////////////////////////////////////////
// espline.h - Generic entity spline class.
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

#ifndef _ESPLINE_H_
#define _ESPLINE_H_ 1

#include "fang.h"
#include "entity.h"




//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CESplineBuilder
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

FCLASS_ALIGN_PREFIX class CESplineBuilder : public CEntityBuilder {
//----------------------------------------------------------------------------------------------------------------------------------
// Public Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	typedef enum {
		POINTTYPE_CFVEC3,				// Source buffer is an array of CFVec3 objects, which will be copied to permanent memory
		POINTTYPE_CFVEC3A,				// Source buffer is an array of CFVec3A objects, which will be copied to permanent memory
		POINTTYPE_CFVEC3A_INPLACE,		// Source buffer is an array of CFVec3A objects, which will be used in place

		POINTTYPE_COUNT
	} PointType_e;




//----------------------------------------------------------------------------------------------------------------------------------
// Public Data:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	PointType_e m_nPointType;	// Source point type
	u32 m_nPointCount;			// Number of points
	const void *m_pPointArray;	// Pointer to array of points (either CFVec3 or CFVec3A)

	BOOL m_bAppendFirstPoint;	// TRUE: Append a copy of the first point to the end of the array (may be TRUE only if PointType_e is not POINTTYPE_CFVEC3A_INPLACE)




//----------------------------------------------------------------------------------------------------------------------------------
// Public Functions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	FINLINE CESplineBuilder() {}
	virtual void SetDefaults( u64 nEntityTypeBits, u64 nEntityLeafTypeBit, cchar *pszEntityType );




//----------------------------------------------------------------------------------------------------------------------------------
// Protected Functions:
//----------------------------------------------------------------------------------------------------------------------------------
protected:

	virtual BOOL InterpretTable( void );
	virtual BOOL PostInterpretFixup( void );


	FCLASS_STACKMEM_ALIGN( CESplineBuilder );
} FCLASS_ALIGN_SUFFIX;




//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CESpline
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

FCLASS_ALIGN_PREFIX class CESpline : public CEntity {
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

	BOOL m_bClosedSpline;				// TRUE means the last point is identical to the first point
	u32 m_nPointCount;					// Number of points in m_pPointArray
	const CFVec3A *m_pPointArray;		// Spline points




//----------------------------------------------------------------------------------------------------------------------------------
// Public Functions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	// Construct/Destruct:
	CESpline();
	virtual ~CESpline();
	virtual void ClassHierarchyDestroy( void );


	// Create/Destroy:
	BOOL CreateFromCopy( u32 nPointCount, const CFVec3 *pTempPointArray, BOOL bAppendFirstPoint, cchar *pszEntityName=NULL );
	BOOL CreateFromCopy( u32 nPointCount, const CFVec3A *pTempPointArray, BOOL bAppendFirstPoint, cchar *pszEntityName=NULL );
	BOOL CreateInPlace( u32 nPointCount, const CFVec3A *pPermPointArray, BOOL bClosedSpline, cchar *pszEntityName=NULL );

	u32 PointCount( void ) const { return m_nPointCount; }
	BOOL IsClosed( void ) const { return m_bClosedSpline; }
	const CFVec3A *PointArray( void ) const { return m_pPointArray; }




//----------------------------------------------------------------------------------------------------------------------------------
// Protected Functions:
//----------------------------------------------------------------------------------------------------------------------------------
protected:

	virtual BOOL ClassHierarchyBuild( void );				// Must be implemented by all classes in the derivation chain
	virtual CEntityBuilder *GetLeafClassBuilder( void );	// Must be implemented by all classes in the derivation chain




//----------------------------------------------------------------------------------------------------------------------------------
// Private Functions:
//----------------------------------------------------------------------------------------------------------------------------------
private:

	void _SetDefaults( void );
	BOOL _Create( CESplineBuilder::PointType_e nPointType, u32 nPointCount, const void *pPointArray, BOOL bAppendFirstPoint, cchar *pszEntityName );


	FCLASS_STACKMEM_ALIGN( CESpline );
} FCLASS_ALIGN_SUFFIX;





#endif

