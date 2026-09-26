//////////////////////////////////////////////////////////////////////////////////////
// fcamera.h - 
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
// 01/10/02 Starich     Created.
//////////////////////////////////////////////////////////////////////////////////////
#ifndef _CAMERA_H_
#define _CAMERA_H_ 1

#include "fang.h"
#include "fxfm.h"
#include "fcamshake.h"
#include "fcamstun.h"

#define FCAMERA_MAX_NAME_LEN		15

struct FViewport_t;

// Camera data is meant for quick access by camera men.
// This structure is needed because, inherited classes
// don't inherit the friend directive.  So to get around
// this, each CFCameraMan class contains a FCameraData_t 
// pointer.
FCLASS_ALIGN_PREFIX struct FCameraData_t {
	// general vars:
	BOOL m_bInited;
	u32 m_nID;
	char m_szName[FCAMERA_MAX_NAME_LEN+1];
	
	// fov vars:
	f32 m_fHalfFOV_X;
	f32 m_fHalfFOV_Y;
	f32 m_fNearZ;
	f32 m_fFarZ;
	BOOL m_bReInitViewport;
		
	FViewport_t *m_pViewport;// Here so that the camera man can change the FOV
	CFXfm m_Xfm;			 // Where are we located in the world? 	DOES NOT INCLUDE CAMERA SHAKE MOTION
	f32 m_fHeading;			 // only updated on calls to GetHeadingPitchRoll()		
	f32 m_fPitch;			 // only updated on calls to GetHeadingPitchRoll()
	f32 m_fRoll;			 // only updated on calls to GetHeadingPitchRoll()
	CFVec2 m_XZUnitLookAt;	 // only updated on calls to GetXZUnitDir()
	CFVec3 m_LastPos;		 // automatically handled by the post work function
		
	// camera shake vars:
	f32 m_fCamUnitVibration;
	CFCamShake m_CamShake;
	CFCamStun m_CamStun;

	FCLASS_STACKMEM_ALIGN( FCameraData_t );
} FCLASS_ALIGN_SUFFIX;

// The camera class contains everything that
// a camera man will need to capture a scene.
// This includes an Xfm and a Viewport as
// well as common data useful to every camera
// man.  A camera can be operated by many 
// camera men, but only 1 at a time.
FCLASS_ALIGN_PREFIX class CFCamera {
	friend class CFCameraMan;// the camera man will need total access to a camera's data
public:
	// general interface:
	CFCamera();
	virtual ~CFCamera();
	void InvalidateCacheKeys();// should only be called by cameramen when they have updated the camera xfm

	BOOL Init( u32 nID );
	BOOL IsInited();
	BOOL Reset( u32 nID );
	
	const FViewport_t *GetViewport();	
	// Returns the xfm that the camera man setup and also includes any camera shake motion too.
	const CFXfm *GetFinalXfm();
	// Returns the xfm that the camera man setup but doesn't include any camera shake.
	const CFXfm *GetXfmWithoutShake() const;
	// Called once per frame, before and after the camera man's work function.
	void PreWork();
	void PostWork();
	// Called to setup the camera and viewport (uses the final xfm to initialize the camera stack).
	void SetupCameraAndViewport();

	// Move the camera viewport to the new location
	void MoveViewport(u32 left, u32 top, u32 width, u32 height, BOOL bPreserveFOVX);
	void MoveViewport( const FViewport_t *pViewport );

	// Get data about the current camera position (all these function return non-camera shake values)
	const CFVec3 *GetPos();
	void GetHeadingPitchRoll( f32 *pfHeading, f32 *pfPitch, f32 *pfRoll );
	const CFVec3 *GetLastPos();
	CFVec2 *GetXZUnitDir();
	BOOL IsPointCloseToCamera( const CFVec3 &rWSPoint, f32 fDistance2 );

	void SetFOV( f32 fHalfX, f32 fHalfY=0.0f );
	void GetFOV( f32 *pfHalfX, f32 *pfHalfY=NULL );

	void SetClipPlanes( f32 fNearZ, f32 fFarZ );
	void GetClipPlanes( f32 *pfNearZ, f32 *pfFarZ );

	// Sets and gets the ID assigned to this camera.
	// The set routine returns the previous value.
	u32 SetID( u32 nID );
	u32 GetID();
	
	// Sets and gets the name assigned to this camera.
	void SetName( cchar *pszName );
	cchar *GetName();
	
	// camera shake interface:
	void SetCamVibrationForOneFrame( f32 fUnitVibration );
	void ShakeCamera( f32 fUnitIntensity, f32 fDurationInSecs );
	void StunCamera( f32 fDurationInSecs );

	void StopCamEffects( void );
	
private:
	FCameraData_t m_CameraData;	
	
	///////////////////////////////////////////////////////////////////////////////////
	// super private - these vars should only be used by CFCamera code and nothing else
	s32 m_nHPRCacheKey;
	s32 m_nXZLookAtCacheKey;
	CFXfm m_FinalXfm;		 // The final xfm that includes all effects like camera shake
	CFVec3 m_PreWorkCache;	 // used by prework() to cache the last camera pos
	/////////////////////////////////////////////////////////////////////////////////////

protected:

	FCLASS_STACKMEM_ALIGN( CFCamera );
} FCLASS_ALIGN_SUFFIX;

// The camera man defines an abstract base class.
// A camera man is responsible for moving the camera
// through the scene and setting up all of the camera's
// variables.  A Camera Man can only operate one camera
// at a time.
class CFCameraMan {

public:
	CFCameraMan();
	virtual ~CFCameraMan();

	virtual void Work() = 0;
	virtual void Draw();
	CFCamera *AssignCamera( CFCamera *pCamera );
	CFCamera *GetAssignedCamera();	
	const FCameraData_t *GetAssignedCameraData();
	virtual void Reset() = 0;
	virtual BOOL FreezeShot( BOOL bFreeze );
	
private:
	
protected:
	CFCamera *m_pCamera;			// the camera being operated by this cameraman
	FCameraData_t *m_pCameraData;// this pointer, points to m_pCamera->m_CameraData (this is done so that derived 
								// classes can gain access to friend private data)	
	BOOL m_bFreezeShot;			// should this cameraman freeze the last shot, or do processing to create a new one
};

// Simply resets the camera module, the real magic comes from calling fcamera_InitSystem()
extern BOOL fcamera_ModuleStartup( void );
extern void fcamera_ModuleShutdown( void );

// The camera system manages have multiple cameras at one time.
// The system also manages assigning camera men to cameras, although this
// system knows nothing about the camera man implementation (a game specific
// module would have that knowledge).  The camera system also has no idea
// which camera is currently being used for rendering (maintaining the ACTIVE
// camera is another game specific issue), but can call the work functions for
// each camera man, calling the CFCamera & CFCameraMan functions in the correct
// order.  The fcamera_Draw() requires that the camera stack be setup before 
// calling (remember this system doesn't know which camera is ACTIVE).
extern BOOL fcamera_InitSystem( u32 nNumCameras );
extern void fcamera_UninitSystem( void );
extern u32 fcamera_GetCameraCount( void );

extern BOOL fcamera_SetCameraName( u32 nIndex, cchar *pszName );
extern CFCamera *fcamera_GetCameraByName( cchar *pszName );
extern CFCamera *fcamera_GetCameraByIndex( u32 nIndex );

extern CFCameraMan *fcamera_GetCameraManByIndex( u32 nCameraIndex );
extern CFCameraMan *fcamera_SetCameraManByIndex( u32 nCameraIndex, CFCameraMan *pCameraMan );
extern u32 fcamera_GetNumWorkingCameraMen( void );

extern void fcamera_ToggleCameraIconDraw( u32 nIndex, BOOL bDrawIcon );

extern void fcamera_StopEffectsOnAllCameras( void );

extern void fcamera_Work( void );
extern void fcamera_Draw( void );

extern s32 fcamera_GetLastCameraIndex();

#endif

