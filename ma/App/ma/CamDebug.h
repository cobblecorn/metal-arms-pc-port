//////////////////////////////////////////////////////////////////////////////////////
// CamDebug.h - a debug cameraman (looks at another camera) or just a free camera (no collision)
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
#ifndef _CAMDEBUG_H_
#define _CAMDEBUG_H_ 1

#include "fang.h"
#include "fcamera.h"

#if FANG_PRODUCTION_BUILD
	#define CAM_DEBUG_ENABLE	FALSE
#else
	#define CAM_DEBUG_ENABLE	TRUE
#endif

#if CAM_DEBUG_ENABLE

FCLASS_ALIGN_PREFIX class CCamDebug : public CFCameraMan {
	
public:
	CCamDebug();
	virtual ~CCamDebug();

	virtual BOOL Init( CFCamera *pTarget, u32 nControlPortIndex );
	virtual void Work();
	virtual void Reset();
	CFCamera *GetCameraBeingDebugged();
	void EnableModeChanges( BOOL bEnable );
	void EnableUserMovement( BOOL bEnable );
			
	enum {
		CAM_DEBUG_FREE = 0,
		CAM_DEBUG_LOOKAT,
		CAM_DEBUG_MATCHYAW,

		CAM_DEBUG_MODE_COUNT
	};

	void ForceModeChange( u32 nNewMode );
	u32 GetCurrentMode() { return m_nMode; }

private:
	
protected:
	u32 m_nMode;
	CFCamera *m_pTargetCam;
	u32 m_nControlPort;
	BOOL m_bSwitchModesButtonDown;
	BOOL m_bAllowModeChanges;
	BOOL m_bAllowMovement;

	BOOL m_bFreeModeValid;
	f32 m_fFreeHeading;
	f32 m_fFreePitch;
	CFVec3 m_FreePos_WS;
	
	BOOL m_bLookAtModeValid;
	f32 m_fLookAtLength;
	f32 m_fLookAtHeading;
	f32 m_fLookAtPitch;
	CFVec3 m_LookAtPos_WS;
		
	void SetFreeModeDefaults();
	void SetLookAtModeDefaults();
	void SwitchModes( u32 nNewMode );
	static f32 SafeAnalogSquare( f32 fVal );
	
	FCLASS_STACKMEM_ALIGN( CCamDebug );
} FCLASS_ALIGN_SUFFIX;

#endif

#endif