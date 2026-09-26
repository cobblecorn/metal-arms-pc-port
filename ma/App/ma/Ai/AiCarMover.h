#ifndef _AICARMOVER_H_
#define _AICARMOVER_H_ 1

//////////////////////////////////////////////////////////////////////////////////////
// AICarMover.h 
//  classes:
//                        
//		CCarMover - Special CAIBotMover that adds logic for bots that can not turn in place
//
//                
// Author: MacKellar & MacDonald
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
// 03/18/03 MacDonald   Created.
//////////////////////////////////////////////////////////////////////////////////////

#include "fmath.h"
#include "AIMath.h"
#include "AIMover.h"
#include "AIBotMover.h"

class CBot;
class CAIBrain;


FCLASS_ALIGN_PREFIX class CAICarMover : public CAIBotMover
{
public:
	CAICarMover(void);
	virtual ~CAICarMover(void);

	virtual void AddToWorld(void);
	virtual void RemoveFromWorld(void);

	virtual void BeginFrame(void);	

	//Assign the mover to a path
	virtual void AssignPath(CAIPath* pPath,	u32 uPathInfoFlags = CAIMover::ASSIGNPATHFLAG_NONE);


	virtual BOOL FollowPath(void);  //return of FALSE means not done, TRUE means path complete

	virtual BOOL MoveToward(const CFVec3A& GotoPoint_WS,		
								f32* pfVelAlgnmnt = NULL);		

	virtual BOOL FaceToward(const CFVec3A& Location, f32* pfTorsoAlgnmnt = NULL);
	
	virtual void EndFrame(void);		  //call when no more orders will be given this frame

	virtual void _DoStuckWork(void);

	enum
	{
		AICARMOVERFLAG_NONE						= 0x0000,
		AICARMOVERFLAG_BACKING_UP				= 0x0001,
		AICARMOVERFLAG_STUCK_EXTRACTION			= 0x0002,	// car is attempting to get itself un-stuck
	};

	typedef enum
	{
		VEHICLE_LOCATION_AHEAD,
		VEHICLE_LOCATION_BESIDE,
		VEHICLE_LOCATION_BEHIND,
	} VehicleLocation_t;

	typedef enum
	{
		TACTIC_STAY_AHEAD,
		TACTIC_STAY_BESIDE,
		TACTIC_STAY_BEHIND,
	} Tactic_t;

	CAIPathWaypoint* CarPath_DoWayPtProgress(void);
	BOOL CarPath_FollowNormalEdge(void);


	u16 m_uAICarMoverFlags;

	CFVec3A m_vEdgeLocation;	// nearest edge location to vehicle
	f32 m_fSpeedXZ;				// current desired speed
	f32 m_fStuckCount;			// timer used when car might be stuck
	f32 m_fAtDestTime;			// time elapsed since vehicle position = goto position
	f32 m_fDesiredEdgeOffset;	// units to the left or right of edge at which vehicle should be located, + = to the right of edge
	VehicleLocation_t m_ePlayerLocation;	// basic location of player relative to this vehicle
	Tactic_t m_eTactic;			// tactic vehicle should employ against player
	f32 m_fTacticTimer;			// count down to tactic change

private:
	void _UpdateEstimatedLinkTraverseTime( void );

	FCLASS_STACKMEM_ALIGN( CAICarMover );
} FCLASS_ALIGN_SUFFIX;

#endif // _AICARMOVER_H_
