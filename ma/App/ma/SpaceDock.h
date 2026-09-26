//////////////////////////////////////////////////////////////////////////////////////
// SpaceDock.h - Special code for the Space Dock level of Metal Arms.
//
// Author: Justin Link      
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
// 08/01/02 Link        Created.
//////////////////////////////////////////////////////////////////////////////////////
#ifndef _SPACEDOCK_H_
#define _SPACEDOCK_H_ 1

#include "fang.h"
#include "fmath.h"

#include "level.h"
#include "CamManual.h"


class CMeshEntity;
class CESwitch;
class CFAnimManMtx;
class CFAnimMeshRest;
class CFAnimCombiner;
class CEntity;



FCLASS_NOALIGN_PREFIX class CSpaceDock
{
public:
	static BOOL InitLevel( LevelEvent_e eEvent );
	static void UninitLevel();
	static void Work();
	static void Draw();
	static void CheckpointSave( s32 nCheckpoint );
	static void CheckpointRestore( s32 nCheckpoint );

private:
	CSpaceDock() {};
	~CSpaceDock() {};

	static void _EventCallback(s32 nWhichEvent, u32 uUserData, u32 uEventData1, u32 uEventData2, u32 uEventData3);

	static void _InitCraneCam();
	static void _CalcCraneCam();
	static void _LeaveCraneCam();


	static void _UpdateCraneBones();

	//////////////////////////////////////////////////////////////////////
	//
	static BOOL s_bSystemInitted;
	//
	//////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////
	//
	static CMeshEntity *s_pEMagnet;
	static CESwitch *s_pESwitch;

	static CMeshEntity *s_apFence[5];

	static CEntity *s_pCamPos;
	//
	//////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////
	//
	static u32 s_hEventListener;
	//
	//////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////
	//
	static CCamManualInfo s_oCamInfo;
	static CFMtx43A s_mtxCam;
	//
	//////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////
	//
	enum State_e
	{
		STATE_PUZZLEUNSOLVED	=	0,
		STATE_OPERATINGCRANE,
		STATE_LOWERINGCRANE,
		STATE_RAISINGCRANE,
		STATE_PAUSEDATBOTTOM,
		STATE_RAISINGFENCE1,
		STATE_RAISINGFENCE2,
		STATE_PUZZLESOLVED,
		STATE_PAUSEDATEND,

		STATE_COUNT
	};
	static State_e s_eCurState;
	//
	//////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////
	// Used for animating the crane magnet.
	static CFAnimManMtx *s_pManMtx;
	static CFAnimMeshRest *s_pRestMtx;
	static CFAnimCombiner *s_pAnimCombiner;

	static CFVec2 s_vecCraneDisp;
	static CFVec2 s_vecCraneVel;
	static f32 s_fMagnetDispY;
	static f32 s_fMagnetVelY;
	//
	//////////////////////////////////////////////////////////////////////

	static f32 s_fStateTimer;
	static f32 s_fFenceTimer;
	static BOOL s_bFenceToggleState;

	//////////////////////////////////////////////////////////////////////
	//
	enum CraneBone_e
	{
		CRANEBONE_MAGNET	=	0,
		CRANEBONE_CROSSBEAM,
		CRANEBONE_SLIDER,

		CRANEBONE_COUNT
	};
	static cchar *s_apszCraneBoneNames[CRANEBONE_COUNT];
	static s32 s_anCraneBoneIndices[CRANEBONE_COUNT];
	//
	//////////////////////////////////////////////////////////////////////

	
	//////////////////////////////////////////////////////////////////////
	// Glitch turn around test
//	static void _TurnAroundCallback(s32 nWhichEvent, u32 uUserData, u32 uEventData1, u32 uEventData2, u32 uEventData3);
//	static CEntity *s_pETrigger;

} FCLASS_NOALIGN_SUFFIX;




#endif

