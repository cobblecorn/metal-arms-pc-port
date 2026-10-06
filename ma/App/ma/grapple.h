//////////////////////////////////////////////////////////////////////////////////////
// grapple.h - Sniper's grapple line.
//
// Author: Mike Elliott   
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
// 03/18/02 Elliott       Created.
//////////////////////////////////////////////////////////////////////////////////////


#ifndef _GRAPPLE_H_
#define _GRAPPLE_H_

#include "fang.h"
#include "fmath.h"
#include "flinklist.h"
#include "ftex.h"

struct FMesh_t;

class CMeshEntity;
class CBot;
class CFWorldTracker;


FCLASS_ALIGN_PREFIX class CGrapple {
//----------------------------------------------------------------------------------------------------------------------------------
// Public Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
public:
	typedef enum {
		STATE_INACTIVE,								// not doing anything, just attached
		STATE_LAUNCHING,							// firing / initial launch
		STATE_TIGHTENING,							// tightening up after connected
		STATE_TIGHT,								// line is taut, bot can ascend
		STATE_RELOADING,
		STATE_DETACHED,
		STATE_BREAKING,

		STATE_COUNT
	} GrappleState_e;


//----------------------------------------------------------------------------------------------------------------------------------
// Protected Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
protected:


//----------------------------------------------------------------------------------------------------------------------------------
// Private Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
private:
	enum {
		_NO_GRAPPLE_LINE_PTS = 25
	};


//----------------------------------------------------------------------------------------------------------------------------------
// Private Data:
//----------------------------------------------------------------------------------------------------------------------------------
private:

	static BOOL m_bSystemInitialized;
	static FLinkRoot_t m_LinkRoot;
	static FMesh_t *m_pMesh;				// mesh resource
	static CFTexInst m_CableTex;

	BOOL m_bIsCreated;
	FLink_t m_Link;							// Link to other tethers
	CMeshEntity *m_pHookMesh;				// the actual grappling hook mesh

	CBot *m_pOwnerBot;
	cchar *m_pszAttachBoneName;
	CFMtx43A *m_pMtxAttachBone;

	CFVec3A	m_vDetachedBasePos_WS;									
	CFVec3A *m_pvBasePos_WS;										// where the base of the grapple is
	CFVec3A m_vTgtPos_WS;
	CFVec3A m_vLastPos_WS;											// where the grapple was last frame
	CFVec3A m_vPos_WS;												// position of the grapple
	CFVec3A	m_vVel_WS;												// velocity of the grapple
	CFVec3A m_vXZVel_WS;											// velocity of the grapple in the XZ plane
	CFVec3A	m_avLinePts[_NO_GRAPPLE_LINE_PTS];						// points on the grapple line
	CFVec3A m_avTopWavePts[_NO_GRAPPLE_LINE_PTS];					// points on the upper wave part of the line
	CFVec3A m_avStraightLinePts[_NO_GRAPPLE_LINE_PTS];				// points on a straight line
	//f32	m_afUnitLineTimer[_NO_GRAPPLE_LINE_PTS];
	f32 m_afUnitDistOnLine[_NO_GRAPPLE_LINE_PTS];
	u32 m_uNumPtsInLine;											// number of points in the grapple line
	f32 m_fDistTraveled;											// how far the grapple has traveled
	f32 m_fXZDistTraveled;											// how far the grapple has traveled in the XZ plane
	f32 m_fUnitLineSlack;
	f32 m_fXZSpeed;													// speed of the grapple in the XZ plane

	f32 m_fFinalDist;
	f32 m_fFinalInterval;

	f32 m_fUnitTightenStart;
	f32 m_fUnitTightenFinish;

	f32 m_fReloadTimer;
	GrappleState_e m_eState;

	f32 m_fXZSpacing;
	f32 m_fUnitDeathTimer;
	f32 m_fUnitRadius;

	u32 m_uBreakPointClose;
	u32 m_uBreakPointFar;



//----------------------------------------------------------------------------------------------------------------------------------
// Public Functions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	// System:
	CGrapple();
	virtual ~CGrapple();

	static BOOL InitSystem( void );
	static void UninitSystem( void );

	static void DrawAll( void );
	static void WorkAll( void );

	//static CGrapple* CreateNewGrapple( CBot* pOwnerBot, cchar* pszAttachBoneName );
	BOOL Create( CBot* pOwnerBot, cchar* pszAttachBoneName );
	void Destroy( void );
	FINLINE BOOL IsCreated( void ) { return m_bIsCreated; }

	void AddToWorld( void );
	void RemoveFromWorld( void );

	void Fire( const CFVec3A &vTgtPt );
	void Kill( void );
	void Work( void );
	void Break( void )								{ _BreakLine(); }

	FINLINE BOOL IsReloading( void )				{ return m_eState == STATE_RELOADING; };
	FINLINE GrappleState_e GetState( void )			{ return m_eState; }
	void GetDistToGrapple( CFVec3A *pVec );



//----------------------------------------------------------------------------------------------------------------------------------
// Protected Functions:
//----------------------------------------------------------------------------------------------------------------------------------
protected:



//----------------------------------------------------------------------------------------------------------------------------------
// Private Functions:
//----------------------------------------------------------------------------------------------------------------------------------
private:
	static BOOL _InitSharedData( void );
	static void _DestroySharedData( void );
	static BOOL _IntersectingTrackerCallback( CFWorldTracker *pTracker );

	void _ClearDataMembers( void );

	void _HandleLaunchWork( void );
	void _HandleTightenWork( void );
	void _DroopCable( void );

	BOOL _CheckForHookCollision( void );
	void _CheckForLineCollision( void );

	void _StartTightening( void );
	void _KeepCableTight( void );
	void _MoveCableBase( void );

	void _CalcUpperWavePts( void );
	void _CalcStraightPts( void );

	void _Detach( void );
	void _DetachedWork( void );

	void _Draw( void );
	void _DrawCable( void );

	void _BreakLine( const CFVec3A *pvBreakPos_WS=NULL );
	void _BreakWork( void );

	static CFVec3A m_avPentagonVtx[6];
	static f32 m_afPentagonVtxIntensity[6];


	FCLASS_STACKMEM_ALIGN( CGrapple );
} FCLASS_ALIGN_SUFFIX;


#endif