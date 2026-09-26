//////////////////////////////////////////////////////////////////////////////////////
// fpadio.h - 
//
// Author: Albert Yale
//////////////////////////////////////////////////////////////////////////////////////
// THIS CODE IS PROPRIETARY PROPERTY OF SWINGIN' APE STUDIOS, INC.
// Copyright (c) 2001
//
// The contents of this file may not be disclosed to third
// parties, copied or duplicated in any form, in whole or in part,
// without the prior written permission of Swingin' Ape Studios, Inc.
//////////////////////////////////////////////////////////////////////////////////////
// Modification History:
//
// Date     Who         Description
// -------- ----------  --------------------------------------------------------------
// 11/16/01 ayale       Created.
//////////////////////////////////////////////////////////////////////////////////////

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#ifndef _FPADIO_H_
#define _FPADIO_H_ 1

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#include "fang.h"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#define FPADIO_MAX_DEVICES						4		// Must be 4 or less.
#define FPADIO_MAX_INPUTS						19		// Can't be less than 6.
#define FPADIO_MAX_DEVICE_NAME_LENGTH			259		// Must be 259 or more.

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

typedef enum
{
	FPADIO_NO_ERROR	=	0,
	FPADIO_ERROR	=	-1

} FPadio_Error_e;

typedef enum
{
	FPADIO_INPUT_EMULATION_PLATFORM_NONE = 0,
	FPADIO_INPUT_EMULATION_PLATFORM_XB,
	FPADIO_INPUT_EMULATION_PLATFORM_GC

} FPadio_InputEmulationPlatform_e;

typedef struct
{
	u32 DX8ONLY_ohWnd;						// [DX8] Handle of window                 [else] ignore.
	u32 DX8ONLY_ohInstance;					// [DX8] HInstance of app                 [else] ignore.
	FPadio_InputEmulationPlatform_e DX8ONLY_uInputEmulationPlatform;
											// [DX8] 0: Off, 1: Xbox, 2: GC           [else] ignore.
	u32 *DX8ONLY_pauInputEmulationMap;		// [DX8] NULL: Off, else: ptr to map      [else] ignore.
	u8 *DX8ONLY_pszInputEmulationDevName;	// [DX8] NULL: Off, else: ptr to dev name [else] ignore.

} FPadio_Init_t;

typedef enum
{
	FPADIO_INPUT_NONE						=  0,

	//// Common.
	//
	FPADIO_INPUT_RESET						=  1,
	FPADIO_INPUT_START						=  2,
	FPADIO_INPUT_STICK_LEFT_X				=  3,
	FPADIO_INPUT_STICK_LEFT_Y				=  4,
	FPADIO_INPUT_STICK_RIGHT_X				=  5,
	FPADIO_INPUT_STICK_RIGHT_Y				=  6,
	FPADIO_INPUT_TRIGGER_LEFT				=  7,
	FPADIO_INPUT_TRIGGER_RIGHT				=  8,
	FPADIO_INPUT_DPAD_X						=  9,
	FPADIO_INPUT_DPAD_Y						= 10,
	FPADIO_INPUT_CROSS_TOP					= 11,
	FPADIO_INPUT_CROSS_BOTTOM				= 12,
	FPADIO_INPUT_CROSS_LEFT					= 13,
	FPADIO_INPUT_CROSS_RIGHT				= 14,
	//
	////

	//// DX specific.
	//
	FPADIO_INPUT_DX_AXIS					= 15,
	FPADIO_INPUT_DX_BUTTON					= 16,
	//
	////

	//// XB specific.
	//
	FPADIO_INPUT_XB_DBUTTON_BACK			= 15,
	FPADIO_INPUT_XB_DBUTTON_STICK_LEFT		= 16,
	FPADIO_INPUT_XB_DBUTTON_STICK_RIGHT		= 17,
	FPADIO_INPUT_XB_ABUTTON_BLACK			= 18,
	FPADIO_INPUT_XB_ABUTTON_WHITE			= 19,
	//
	////

	//// GC specific.
	//
	FPADIO_INPUT_GC_DBUTTON_TRIGGER_LEFT	= 15,
	FPADIO_INPUT_GC_DBUTTON_TRIGGER_RIGHT	= 16,
	FPADIO_INPUT_GC_DBUTTON_TRIGGER_Z		= 17,
// Not supported by standard controller:
	FPADIO_INPUT_GC_ABUTTON_A				= 18,
	FPADIO_INPUT_GC_ABUTTON_B				= 19,
	//
	////

//ARG - >>>>>
	//// PS2 specific.
	//
	FPADIO_INPUT_PS2_SELECT					= 15,
	FPADIO_INPUT_PS2_LEFT2					= 16,
	FPADIO_INPUT_PS2_RIGHT2					= 17
	//
	////
//ARG - <<<<<

} FPadio_InputID_e;

typedef enum
{
	FPADIO_DEVICE_NONE				= 0,

	//// DX specific.
	//
	FPADIO_INPUT_DX_GAMEPAD			= 1,
	FPADIO_INPUT_DX_NON_GAMEPAD		= 2,
	//
	////

	//// XB specific.
	//
	FPADIO_INPUT_XB_GAMEPAD			= 1,
	FPADIO_INPUT_XB_NON_GAMEPAD		= 2,
	//
	////

	//// GC specific.
	//
	FPADIO_INPUT_GC_GAMEPAD			= 1,
	FPADIO_INPUT_GC_NON_GAMEPAD		= 2
	//
	////

} FPadio_DeviceID_e;

typedef struct
{
	BOOL bValid;

	f32 afInputValues[ FPADIO_MAX_INPUTS ];

} FPadio_Sample_t;

typedef struct
{
	char szName[ ( FPADIO_MAX_DEVICE_NAME_LENGTH + 1 ) ]; // Can be larger, but not any smaller, than window's MAX_PATH (260). Not Unicode. Do not display to user. For internal use only.
	FPadio_DeviceID_e oeID;
	u32 uInputs;
	FPadio_InputID_e aeInputIDs[ FPADIO_MAX_INPUTS ];

} FPadio_DeviceInfo_t;

#if FANG_PLATFORM_GC
	typedef enum
	{
		FPADIO_GCFFMODE_SOFT_OFF,
		FPADIO_GCFFMODE_HARD_OFF,
		FPADIO_GCFFMODE_ON,

		FPADIO_GCFFMODE_COUNT

	} FPadio_GCFFMode_e;
#endif

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

extern BOOL fpadio_ModuleStartup( void );
extern void fpadio_ModuleShutdown( void );
extern FPadio_Error_e fpadio_Install( const FPadio_Init_t *poInit = NULL ); // if DX8: poInit can't be NULL.
extern void fpadio_Uninstall( void );
extern BOOL fpadio_IsInstalled( void );

#if FANG_PLATFORM_XB
	extern void fpadio_SetForcefeedback_XB( u32 uDeviceIndex, f32 fUnitVibeCoarse, f32 fUnitVibeFine );
#elif FANG_PLATFORM_GC
	extern void fpadio_SetForcefeedback_GC( u32 uDeviceIndex, FPadio_GCFFMode_e oeMode );
#endif

extern void fpadio_ApplyAllForcefeedbacks( void );
// GC Warning: Do not call fpadio_GetSamples() more than once per frame.
extern void fpadio_GetSamples( u32 *puValidSamples, FPadio_Sample_t ***ppapaoSamples ); // ptr to array of ( FPADIO_MAX_DEVICES * FPADIO_NUMBER_SAMPLES_PER_BUFFER ) samples.
extern void fpadio_GetDeviceInfo( u32 uDeviceIndex, FPadio_DeviceInfo_t *poDeviceInfo ); // Provide a pointer to an FPadio_DeviceInfo_t.

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#endif // _FPADIO_H_

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
