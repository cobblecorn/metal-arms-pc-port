//////////////////////////////////////////////////////////////////////////////////////
// AIPatrolPath.h - 
//
// Author: Pat MacKellar 
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
// 05/21/02 MacKellar   Created.
//////////////////////////////////////////////////////////////////////////////////////
#ifndef _AIPATROLPATH_H_
#define _AIPATROLPATH_H_ 1

#include "fang.h"
#include "ftl.h"
#include "AIPath.h"
#include "AIGraphSearcher.h"
#include "../tripwire.h"

class CESpline;

class CPatrolPath
{
public:
	CPatrolPath(void): m_uStage(FORWARD_INIT), 
						m_pPtArray(NULL),
						m_uMinXZRadius(0),
						m_uMinHeight(0),
						m_b3DPath(FALSE)
						{
						}

	void Init(	CESpline* pPtArray,
				u8 uMinXZRadius, 
				u8 uMinHeight, 
				BOOL b3DPatrolPath)	{	m_uStage = FORWARD_INIT;
										m_pPtArray = pPtArray;
										m_uMinXZRadius = uMinXZRadius;
										m_uMinHeight = uMinHeight;
										m_b3DPath = (u8) b3DPatrolPath;
									}

	CESpline* GetPtArray(void)										{ return m_pPtArray;}
	BOOL Is3DPath(void)												{ return m_b3DPath!=0;}
	u8	GetWidth(void)												{ return m_uMinXZRadius;}
	u8	GetHeight(void)												{ return m_uMinHeight;}
	CAIPath* GetAIPath(void)										{ return &m_AIPath;}
	BOOL Failed(void)												{ return m_uStage == FAILED;}

public:	
	static BOOL InitSystem(void);
	static void Work(void);
	static void UninitSystem(void);
	static BOOL Register(	CESpline* pPtArrayEntity,
							u8 uMinXZRadius, 
							u8 uMinHeight, 
							BOOL b3DPatrolPath);
	static CPatrolPath* GetRegistered(cchar* pszName);
	static void DebugRender(void);

protected:
	CESpline *m_pPtArray;
	CAIPath m_AIPath;
	enum
	{
		FORWARD_INIT =0,
		FORWARD_EDGE_INIT,
		FORWARD_EDGE_WAITING,
		FORWARD_COOKED,
		REVERSE_INIT,
		REVERSE_EDGE_INIT,
		REVERSE_EDGE_WAITING,
		REVERSE_COOKED,
		FAILED,
	};
	u32 m_uStage;
	u8 m_uMinXZRadius;
	u8 m_uMinHeight;
	u8 m_b3DPath;

	static CSearchQuery	*s_pSearchQuery;
	static CSearchResults *s_pSearchResults;
	static u16 s_uNumPatrolLinksFound;
	static BOOL IsCooking(cchar* pszName);

	static CNiList<CPatrolPath*> s_CookingList;
	static CNiList<CPatrolPath*> s_RegisterList;
	static CNiBank<CPatrolPath>* s_pAllocBank;
	FCLASS_STACKMEM_NOALIGN(CPatrolPath);
};


//
// CPatrolZone
// 	 --used to control very precisely the behavior of a patrolling unit
//
FCLASS_ALIGN_PREFIX class CPatrolZone  : public CTripwire
{
public:

	virtual void OnEnter(CEntity* pEntity);
	virtual void OnExit(CEntity* pEntity);

	void InitFromTripwireBuilder(void);
	//Zone Behaviors
	enum
	{
		ZONESPEC_SPEED			= 0x01,		// Change Speed while in the zone.						CPatrolZoneSpec::m_uZoneData = (0-100)
		ZONESPEC_WAIT			= 0x02,		// Stop apon entering the zone.							CPatrolZoneSpec::m_uZoneData = (0-255) seconds to wait
		ZONESPEC_ANIM			= 0x04,		// Play an anim apon entering the zone.					CPatrolZoneSpec::m_uZoneData = AnimID?
		ZONESPEC_LOOK			= 0x08,		// Look at something while in through the zone.			CPatrolZoneSpec::m_pLookAtObj = a thing
	};

	enum
	{
		ZONESPEC_SPEED_LOG2			= 0x00,		// Change Speed while in the zone.						CPatrolZoneSpec::m_uZoneData = (0-100)
		ZONESPEC_WAIT_LOG2			= 0x01,		// Stop apon entering the zone.							CPatrolZoneSpec::m_uZoneData = (0-255) seconds to wait
		ZONESPEC_ANIM_LOG2			= 0x02,		// Play an anim apon entering the zone.					CPatrolZoneSpec::m_uZoneData = AnimID?
		ZONESPEC_LOOK_LOG2			= 0x03,		// Look at something while in through the zone.			CPatrolZoneSpec::m_pLookAtObj = a thing
	};
	
	u8 GetSpeed(void);
	u8 GetWaitTime(void);
	u8 GetWaitLookAroundChance(void);
	u8 GetLookZoneBehaviorValue(void);
	u8 GetAnimZoneBehaviorValue(void);
	CEntity* GetLookAtObj(void);
	cchar* GetAnimZoneAnimName(void) { return m_pszLookAtObjName;};
	u8 GetTimeParam(void);

	inline BOOL IsSpeedZone(void)	{ return (m_uZoneBehavior & ZONESPEC_SPEED)!=0;}
	inline BOOL IsWaitZone(void)	{ return (m_uZoneBehavior & ZONESPEC_WAIT)!=0;}
	inline BOOL IsLookZone(void)	{ return (m_uZoneBehavior & ZONESPEC_LOOK)!=0;}
	inline BOOL IsAnimZone(void)	{ return (m_uZoneBehavior & ZONESPEC_ANIM)!=0;}

protected:	
	cchar* m_pszLookAtObjName;
	u8 m_uZoneBehavior;
	u8 m_uZoneTimeParam;

	enum
	{
		NUM_ZONE_BEHAVIORS = 4
	};
	u8 m_auZoneNonRandBehaviorParam[NUM_ZONE_BEHAVIORS];
	u8 m_auZoneRandBehaviorParamMin[NUM_ZONE_BEHAVIORS];
	u8 m_auZoneRandBehaviorParamMax[NUM_ZONE_BEHAVIORS];



	//working data
	CEntity* m_pLookAtObj;	// entity to lookat while in the zone
	
	FCLASS_STACKMEM_ALIGN(CPatrolZone);

} FCLASS_ALIGN_SUFFIX;


#endif

