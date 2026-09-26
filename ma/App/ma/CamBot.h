//////////////////////////////////////////////////////////////////////////////////////
// CamBot.h - the cameraman used when playing a robot
//
// Author: Michael Starich   
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
// 01/31/02 Starich     Created.
//////////////////////////////////////////////////////////////////////////////////////
#ifndef _CAMBOT_H_
#define _CAMBOT_H_ 1

#include "fang.h"
#include "fcamera.h"
//#include "bot.h"

class CBot;

typedef struct CamBotInfo_s {
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// WARNING: IF YOU ADD, REMOVE, OR REARRANGE THE ORDER OF THESE VARS YOU WILL NEED TO UPDATE THE BOT CAM CSV FILE
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	f32 fHalfFOV;				// in Radians	
	f32 fHalfFOV_MP;			// Multiplayer version of fHalfFOV
	CFVec3 TargetPos_MS;		// Desired look-at point of camera in model space
	CFVec3 Pos_MS;				// Desired position of camera in model space
	f32 fPosScale_MP;			// Scale to apply to Pos_MS for multiplayer
	f32 fLookupYOffset;			// when looking upward, how much should the target point be raised
	f32 fClosestDistToTarget;	// what it is the closest the camera should come to the target pos (in feet)
								// (negative is behind the target toward the camera position)
} CamBotInfo_t;


FCLASS_ALIGN_PREFIX class CCamBot : public CFCameraMan {
public:
	CCamBot();
	virtual ~CCamBot();

	static BOOL InitSystem();
	static void UninitSystem();

	virtual BOOL Init( CBot *pBot, const CamBotInfo_t *pInfo );
	virtual void Work();
	virtual void Draw();
	virtual void Reset();

	typedef enum {
		CAM_BOT_3RD_PERSON = 0,
		CAM_BOT_1ST_PERSON,

		CAM_BOT_MODE_COUNT
	} CamBotMode_e;

	void ChangeCameraMode( CamBotMode_e nNewMode );

	FINLINE CamBotMode_e GetCamBotMode( void ) { return m_nMode; }
	FINLINE const CFVec3A* GetMinCamPos( void ) { return &m_MinCamPos_WS; }
	FINLINE const CFVec3A* GetLookAtPoint( void ) { return &m_LookAtPoint_WS; }
	FINLINE const CFVec3A* GetDesiredToLookAtPoint_MS( void ) { return &m_DesiredLookAtToPos_MS; }
	FINLINE const f32 GetLastUnitDist( void ) { return m_fLastUnitDist; }
	FINLINE BOOL GetAlwaysOverride( void ) const { return m_bAlwaysOverride; }
	FINLINE void SetAlwaysOverride( BOOL bOver ) { m_bAlwaysOverride = bOver; }

private:
	f32		m_fOverrideUnitTime;		// how far we are into the transition to the bot's override
	BOOL	m_bOverrideEnabled;			// whether or not to use the bot's override
	f32		m_fOOOverrideTime;			// how fast to transition to the bot's suggested location
	BOOL	m_bAlwaysOverride;			// TRUE when we always want to override

protected:
	CamBotMode_e m_nMode;
	CBot *m_pBot;
	const CamBotInfo_t *m_pCamInfo;	
	f32 m_fPitch;
	CFVec3A m_DesiredLookAtToPos_MS;

	CFVec3A m_LookAtPoint_WS;	// point being looked at by this camera
	CFVec3A m_UnitLookDir_WS;	// what direction is the camera looking
	CFVec3A m_MinCamPos_WS;		// the closest pos the camera can zoom into
	CFVec3A m_DesiredPos_WS;		// if there where no collisions, where would have the camera been
	f32 m_fLastUnitDist;		// how far from m_MinCamPos_WS did we get last frame
	f32 m_fLastCenterRayUnitDist;// where did the center ray collide last frame?
	f32 m_fLastDeltaZoom;		// how much did the zoom factor change last frame?
	
	
#define _MAX_COLLISION_SPHERES		12
	static CFSphere m_aSphereList[_MAX_COLLISION_SPHERES];
	static CFSphere m_MasterBoundingSphere;
	static u32 m_nNumSpheresUsed;

	// used during debugging
#define _HISTORY_BUFFER_SIZE		30
	static f32 m_afHistoryBuffer[_HISTORY_BUFFER_SIZE];
	static u32 m_anHistoryIndex;

	void Work_3rdPerson( BOOL bCollide );
	static void SetupCollisionSpheres( const CFVec3A &rStartPos_WS, const CFVec3A &rEndPos_WS, const CFVec3A &rUnitStartToEnd_WS );
	f32 AnimateZoom( BOOL bThereAreCollisions, f32 fCurRayZoom, f32 fCurCenterZoom );

	FCLASS_STACKMEM_ALIGN( CCamBot );
} FCLASS_ALIGN_SUFFIX;




#endif

