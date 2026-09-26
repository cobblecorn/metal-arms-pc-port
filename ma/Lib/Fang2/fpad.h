//////////////////////////////////////////////////////////////////////////////////////
// fpad.h - 
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
// 01/28/01 ayale       Created.
//////////////////////////////////////////////////////////////////////////////////////

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#ifndef _FPAD_H_
#define _FPAD_H_ 1

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#include "fang.h"
#include "fpadio.h"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

typedef enum
{
	FPAD_NO_ERROR	=	0,
	FPAD_ERROR		=	-1

} FPad_Error_e;

typedef enum
{
	FPAD_PORT0_CONNECTED	=	0x00000001,	// ( 1 << 0 )
	FPAD_PORT1_CONNECTED	=	0x00000002,	// ( 1 << 1 )
	FPAD_PORT2_CONNECTED	=	0x00000004,	// ( 1 << 2 )
	FPAD_PORT3_CONNECTED	=	0x00000008,	// ( 1 << 3 )
	FPAD_ANY_CONNECTED		=	( FPAD_PORT0_CONNECTED | FPAD_PORT1_CONNECTED | FPAD_PORT2_CONNECTED | FPAD_PORT3_CONNECTED )	// ( 1111 )

} FPad_Port_e;

typedef enum
{
	FPAD_LATCH_ON												=	0x00000001,	// ( 1 << 0 ), current state.
	FPAD_LATCH_CHANGED											=	0x00000002,	// ( 1 << 1 ), FPAD_LATCH_ON is diffrent from last sample.
	FPAD_LATCH_SPIKED											=	0x00000004,	// ( 1 << 2 ), FPAD_LATCH_ON is same as last sample, but it flipped on and back in one frame. (In the case of a button, this can be caused by a very quick tap.)
	FPAD_LATCH_TURNED_ON_WITH_NO_REPEAT							=	0x00000008, // ( 1 << 3 ), FPAD_LATCH_ON and FPAD_LATCH_CHANGED are true.
	FPAD_LATCH_TURNED_ON_WITH_REPEAT_AND_NO_INITIAL_DELAY		=	0x00000010, // ( 1 << 3 ), FPAD_LATCH_ON and FPAD_LATCH_CHANGED are true, or every fRepeatDelay seconds FPAD_LATCH_ON has been true. (See fpad_Install().)
	FPAD_LATCH_TURNED_ON_WITH_REPEAT_AND_WITH_INITIAL_DELAY		=	0x00000020  // ( 1 << 4 ), Like FPAD_LATCH_TURNED_ON_WITH_REPEAT_AND_NO_INITIAL_DELAY, but the initial delay is fRepeatInitialDelay. (See fpad_Install().)

} FPad_Latch_e;

typedef struct
{
	u32 uLatches; // See FPad_Latch_e.
	f32 fCurrentState;
	f32 fVelocity;

} FPad_Sample_t;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

extern BOOL fpad_ModuleStartup( void );
extern void fpad_ModuleShutdown( void );
extern FPad_Error_e fpad_Install( f32 fRepeatInitialDelay = 1.0f, f32 fRepeatDelay = 1.0f, f32 fHysteresisThresholdLow = 0.1f, f32 fHysteresisThresholdHigh = 0.2f ); // fRepeatInitialDelay and fRepeatDelay are in seconds. fHysteresisThresholdLow and fHysteresisThresholdHigh only affects the latches.
extern void fpad_Uninstall( void );
extern BOOL fpad_IsInstalled( void );

extern void fpad_Map( u32 uPadIndex, u32 uNumIDs, const FPadio_InputID_e *paeInputIDs, FPad_Sample_t **papoSamples );
extern u32 fpad_UpdateSamples( void ); // returns mask of FPad_Port_e. MUST be called once per loop, and ONLY once per loop.
extern void fpad_ZeroAllControllerSamples( void );

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#endif // _FPAD_H_

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
