//////////////////////////////////////////////////////////////////////////////////////
// SplineActor.h - Wrapper class meant to simplify MeshEntity spline based scripting
//
// Author: Russell Foushee
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
// 04/18/03 Miller		You can now pass in a NULL mesh entity, but still get a position
//						and orientation.
// 01/17/03 Foushee		Created.
//////////////////////////////////////////////////////////////////////////////////////
#ifndef _SPLINE_ACTOR_H_
#define _SPLINE_ACTOR_H_ 1

#include "fang.h"
#include "fpointpath2.h"
#include "actor.h"
#include "FQOtang1.h"
#include "FQuatTang2.h"
#include "FQuatTang3.h"
#include "fQuatComp.h"
#include "espline.h"

class CSplineActor 
{
public:
	virtual BOOL Init( cchar *pszSplineName, f32 fTotalTime, CMeshEntity* pME = NULL, BOOL bAutoStart = TRUE ) = 0;
	virtual void Reset( BOOL bAutoStart ) = 0;
	virtual void Start( void ) = 0;
	virtual void Stop( void ) = 0;
	virtual void GetMatrix( CFMtx43A *pMtx ) = 0; // Fills pMtx with the position and orientation of the spline actor

	void SetTimePos( f32 fNewTimePos );
	FINLINE f32 GetTimePos( void ) { return m_poPointPath2->GetTimePos(); }
	FINLINE BOOL IsDone( void ) { return m_poPointPath2->IsDone(); }

protected:
	CFV3OPointPath2 *m_poPointPath2;
	CMeshActor *m_poMeshActor;

	virtual BOOL AcquireMemberObjects( void );
	CESpline* FindCESplineFromString( cchar *pszSplineName );
};



FCLASS_ALIGN_PREFIX class CStaticOrientationSplineActor : public CSplineActor
{
public:

	static CStaticOrientationSplineActor *Acquire( void );
	BOOL Init( cchar *pszSplineName, f32 fTotalTime, CMeshEntity* pME = NULL, BOOL bAutoStart = TRUE ); 

	void Reset( BOOL bAutoStart );
	void Start( void );
	void Stop( void );
	void GetMatrix( CFMtx43A *pMtx );

protected:

	BOOL AcquireMemberObjects( void );

	FCLASS_STACKMEM_ALIGN( CStaticOrientationSplineActor );
} FCLASS_ALIGN_SUFFIX;



FCLASS_ALIGN_PREFIX class CGeneralSplineActor : public CSplineActor
{
public:

	static CGeneralSplineActor *Acquire( void );
	BOOL Init( cchar *pszSplineName, f32 fTotalTime, CMeshEntity* pME = NULL, BOOL bAutoStart = TRUE ); 

	void Reset( BOOL bAutoStart );
	void Start( void );
	void Stop( void );
	void GetMatrix( CFMtx43A *pMtx );

protected:
	CFQOTang1 *m_poQuatTang1;

	BOOL AcquireMemberObjects( void );

	FCLASS_STACKMEM_ALIGN( CGeneralSplineActor );
} FCLASS_ALIGN_SUFFIX;



FCLASS_ALIGN_PREFIX class CHorizontalSplineActor : public CSplineActor
{
public:

	static CHorizontalSplineActor *Acquire( void );
	BOOL Init( cchar *pszSplineName, f32 fTotalTime, CMeshEntity* pME = NULL, BOOL bAutoStart = TRUE ); 

	void Reset( BOOL bAutoStart );
	void Start( void );
	void Stop( void );
	void GetMatrix( CFMtx43A *pMtx );

protected:
	CFQuatTang2 *m_poQuatTang2;

	BOOL AcquireMemberObjects( void );

	FCLASS_STACKMEM_ALIGN( CHorizontalSplineActor );
} FCLASS_ALIGN_SUFFIX;



FCLASS_ALIGN_PREFIX class CSwingingSplineActor : public CSplineActor
{
public:

	static CSwingingSplineActor *Acquire( void );
	BOOL Init( cchar *pszSplineName, f32 fTotalTime, CMeshEntity* pME = NULL, BOOL bAutoStart = TRUE ); 

	void Reset( BOOL bAutoStart );
	void Start( void );
	void Stop( void );
	void GetMatrix( CFMtx43A *pMtx );

protected:
	CFQuatTang2 *m_poQuatTang2;
	CFQuatTang3 *m_poQuatTang3;
	CFQuatComp  *m_poQuatComp;

	BOOL AcquireMemberObjects( void );

	FCLASS_STACKMEM_ALIGN( CSwingingSplineActor );
} FCLASS_ALIGN_SUFFIX;

#endif
