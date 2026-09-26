//////////////////////////////////////////////////////////////////////////////////////
// swarmerbossgame.h - 
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
// 04.02.03 Elliott     Created.
//////////////////////////////////////////////////////////////////////////////////////

#if 0

#ifndef _SWARMERBOSS_GAME_H_
#define _SWARMERBOSS_GAME_H_ 1

// =============================================================================================================

#include "fang.h"
#include "level.h"

#include "sas_user.h"


class CBotSwarmerBoss;
class CBotControl;
class CESphere;

//temp, should be in class
typedef struct {
	CFQuatA qPos;
	CFQuatA qDesiredPos;

	CFVec3A vPos;
	CFVec3A vTgtPos;

	CFVec3A vJumpPos;

	BOOL	bActive;
} SwarmerBossBrain_t;



FCLASS_ALIGN_PREFIX class CSwarmerBossGame {
//----------------------------------------------------------------------------------------------------------------------------------
// Public Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
public:


//----------------------------------------------------------------------------------------------------------------------------------
// Protected Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
protected:


//----------------------------------------------------------------------------------------------------------------------------------
// Private Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
private:

	enum {
		_MAX_BOSSES			= 1,
		_MAX_AVOID_SPHERES	= 9,
	};

	
	enum {
		BANDFLAG_ALLOW_FLOOR_JUMP	= 0x00000001,
		BANDFLAG_ALLOW_WALL_JUMP	= 0x00000002,
	};

	
	typedef enum {
		BEHAVIOR_NONE,
		BEHAVIOR_NOT_WALKABLE,
		BEHAVIOR_MATCH_LEVEL,
		BEHAVIOR_MATCH_PROJECTION,

		BEHAVIOR_COUNT
	} BandBehavior_e;


    typedef struct {
		u32 uID;
		f32 fYTop;
		f32 fYBottom;
		u32 uBandFlags;
		f32 fFloorJumpY;
		f32 fFloorJumpMinXZ;
		f32 fFloorJumpMaxXZ;

		f32 fWallJumpMinY;
		f32 fWallJumpMaxY;
		
		BandBehavior_e behavior;
	} Band_t;



//----------------------------------------------------------------------------------------------------------------------------------
// Public Data:
//----------------------------------------------------------------------------------------------------------------------------------
public:


//----------------------------------------------------------------------------------------------------------------------------------
// Private Data:
//----------------------------------------------------------------------------------------------------------------------------------
private:
	static CSwarmerBossGame*	m_pSwarmerBossGame;				// the single instance of this class
	static cchar*				m_apszBossNames[_MAX_BOSSES];	
	static CFVec3A				m_SphereCtr;
	

	CBotSwarmerBoss*	m_apSBosses[_MAX_BOSSES];
	SwarmerBossBrain_t	m_aSBBrains[_MAX_BOSSES];
	CBotControl*		m_apSBControls[_MAX_BOSSES];


	BOOL m_bCreated;

	//CFSphere	m_aAvoidSpheres[_MAX_AVOID_SPHERES];
	CESphere *m_apAvoidSpheres[_MAX_AVOID_SPHERES];

//	f32 m_fSBSphereSpeed;				// speed in radians around the outside of the sphere
	Band_t *m_paBands;
	u32 m_uNoBands;
	const Band_t *m_pCurrentBand;

	


//----------------------------------------------------------------------------------------------------------------------------------
// Public Functions:
//----------------------------------------------------------------------------------------------------------------------------------
public:
	static BOOL LoadLevel( LevelEvent_e eEvent );
	static void UnloadLevel( void );
	static void Restore( void );

	static FINLINE void Work( void )	{ FASSERT( m_pSwarmerBossGame->IsCreated() ); m_pSwarmerBossGame->_Work(); };

	static FINLINE BOOL IsCreated( void ) { return (m_pSwarmerBossGame && m_pSwarmerBossGame->m_bCreated ); }
	

//----------------------------------------------------------------------------------------------------------------------------------
// Protected Functions:
//----------------------------------------------------------------------------------------------------------------------------------
protected:


//----------------------------------------------------------------------------------------------------------------------------------
// Private Functions:
//----------------------------------------------------------------------------------------------------------------------------------
private:
	CSwarmerBossGame();

	void _ClearDataMembers( void );
	BOOL _Create( void );
	void _Destroy( void );

	void _Work( void );

	void _Think( u32 uID );
	void _Move( u32 uID );
	void _OrientBossOnWall( u32 uID );

	void _MoveAlongSphereToPoint( const CFVec3A &vFrom, const CFVec3A &vTo, CFVec3A *pvResult );
	void _CalcJumpTarget( const CFVec3A &vPos, CFVec3A *pvTgt );

	const Band_t* _GetBand( f32 fYPos );

	void _FaceToward( u32 uID, const CFVec3A &vTgtPos );
	void _MoveToward( u32 uID, const CFVec3A &vTgtPos );

	void _WallCalcDest( const CFVec3A &vPos, const CFVec3A &vTgt, CFVec3A *pvDest );			// if boss is on wall, calculate a destination
	BOOL _CheckForPlatformJump( const CFVec3A &vPos, const CFVec3A &vTgt, CFVec3A *pvDest );	// test whether boss should jump to a platform, will return jump tgt point

	void _PlatformCalcDest( const CFVec3A &vPos, const CFVec3A &vTgt, CFVec3A *pvDest );		// we're on a platform, calculate our next destination


	//void _CalcPath( u32 uID, const CFVec3A &vTgt );											// calculates a path for this boss
	//BOOL _CheckForTunnelShortcut( const CFVec3A &vStart, CFVec3A *pvEnd );
	//BOOL _CheckForFloorShortcut( const CFVec3A &vStart, CFVec3A *pvEnd );
	//BOOL _CheckForLevelJump( const CFVec3A &vStart, const CFVec3A &vEnd, CFVec3A *pvNextWayPt, CFVec3A *pvJumpPos, CFVec3A *pvJumpLandPos );
	//
	//void _CalcNextWallWayPt( const CFVec3A &vStart, const CFVec3A &vEnd, CFVec3A *pvNextWayPt );
	//void _CalcJumpPoints( const CFVec3A &vStart, const CFVec3A &vEnd, CFVec3A *pvJumpPos, CFVec3A *pvJumpLandPos );
	//void _CalcNextFloorWaypt( const CFVec3A &vStart, const CFVec3A &vEnd, CFVec3A *pvNextWayPt, CFVec3A *pvJumpPos, CFVec3A *pvJumpLandPos );
	const CFSphereA *_FindAvoidSphere( const CFVec3A &vPt );

	
	
	
#if SAS_ACTIVE_USER == SAS_USER_ELLIOTT
	void _DebugFn( void );
#endif

	
	FCLASS_STACKMEM_ALIGN(CSwarmerBossGame);
} FCLASS_ALIGN_SUFFIX;

// =============================================================================================================

#endif


#endif