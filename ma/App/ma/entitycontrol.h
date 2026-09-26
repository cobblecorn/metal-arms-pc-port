//////////////////////////////////////////////////////////////////////////////////////
// entitycontrol.h - Entity control class.
//
// Author: Steve Ranck     
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
// 04/23/02 Ranck       Created.
//////////////////////////////////////////////////////////////////////////////////////

#ifndef _ENTITYCONTROL_H_
#define _ENTITYCONTROL_H_ 1

#include "fang.h"
#include "fmath.h"




//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CEntityControl
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

FCLASS_NOALIGN_PREFIX class CEntityControl {
//----------------------------------------------------------------------------------------------------------------------------------
// Public Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
public:
	typedef enum {
		TYPE_NONE,
		TYPE_HUMAN,
		TYPE_BOT,
		TYPE_VEHICLE,

		TYPE_COUNT
	} Type_e;




//----------------------------------------------------------------------------------------------------------------------------------
// Private Data:
//----------------------------------------------------------------------------------------------------------------------------------
private:
	Type_e m_nType;



	
//----------------------------------------------------------------------------------------------------------------------------------
// Public Functions:
//----------------------------------------------------------------------------------------------------------------------------------
public:
	FINLINE ~CEntityControl() { m_nType = TYPE_NONE; }

	FINLINE Type_e Type( void ) { return m_nType; }
	FINLINE virtual void Zero( void ) {}




//----------------------------------------------------------------------------------------------------------------------------------
// Protected Functions:
//----------------------------------------------------------------------------------------------------------------------------------
protected:
	CEntityControl( Type_e nType ) { FASSERT( nType>=0 && nType<TYPE_COUNT ); m_nType = nType; }




//----------------------------------------------------------------------------------------------------------------------------------
// Private Functions:
//----------------------------------------------------------------------------------------------------------------------------------
private:
	CEntityControl() { m_nType = TYPE_NONE; }


	FCLASS_STACKMEM_NOALIGN( CEntityControl );
} FCLASS_NOALIGN_SUFFIX;




//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CNoControl
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

FCLASS_NOALIGN_PREFIX class CNoControl : public CEntityControl {
//----------------------------------------------------------------------------------------------------------------------------------
// Public Functions:
//----------------------------------------------------------------------------------------------------------------------------------
public:
	FINLINE CNoControl() : CEntityControl( CEntityControl::TYPE_NONE ) {}

	FCLASS_STACKMEM_NOALIGN( CNoControl );
} FCLASS_NOALIGN_SUFFIX;




//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CHumanControl
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

FCLASS_NOALIGN_PREFIX class CHumanControl : public CEntityControl {
//----------------------------------------------------------------------------------------------------------------------------------
// Public Data:
//----------------------------------------------------------------------------------------------------------------------------------
public:
	// Analog inputs:
	f32 m_fForward;				// +1=forward, -1=backward
	f32 m_fStrafeRight;			// +1=strafe right, -1=strafe left

	f32 m_fAimDown;				// +1=aim down, -1=aim up
	f32 m_fRotateCW;			// +1=rotate CW, -1=rotate CCW

	f32 m_fFire1;				// >0: primary fire trigger pressed
	f32 m_fFire2;				// >0: secondary fire trigger pressed
	f32 m_fCrossUp;				// Cross up button
	f32 m_fCrossDown;			// Cross down button

	// Button inputs (see FPAD_LATCH_* for info on what the flags are):
	u32 m_nPadFlagsAction;		// Action button
	u32 m_nPadFlagsJump;		// Jump button
	u32 m_nPadFlagsMelee;		// Melee button
	u32 m_nPadFlagsPause;		// Pause button
	u32 m_nPadFlagsSelect1;		// Primary weapon select
	u32 m_nPadFlagsSelect2;		// Secondary weapon select




//----------------------------------------------------------------------------------------------------------------------------------
// Public Functions:
//----------------------------------------------------------------------------------------------------------------------------------
public:
	CHumanControl();

	virtual void Zero( void );


	FCLASS_STACKMEM_NOALIGN( CHumanControl );
} FCLASS_NOALIGN_SUFFIX;




//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CBotControl
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

FCLASS_ALIGN_PREFIX class CBotControl : public CEntityControl {
//----------------------------------------------------------------------------------------------------------------------------------
// Public Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
public:
	enum {
		BUTTONFLAG_JUMP1				= 0x00000001,	// Normal jump
		BUTTONFLAG_JUMP2				= 0x00000002,	// Special jump
		BUTTONFLAG_JUMPVEC				= 0x00000004,	// Jump using m_JumpVelocity_WS
		BUTTONFLAG_ACTION				= 0x00000008,	// Action
		BUTTONFLAG_SELECT1				= 0x00000010,	// Primary weapon select
		BUTTONFLAG_SELECT2				= 0x00000020,	// Secondary weapon select
		BUTTONFLAG_SPECIAL1				= 0x00000040,	// Special bot-specific function 1
		BUTTONFLAG_SPECIAL2				= 0x00000080,	// Special bot-specific function 2
		BUTTONFLAG_MELEE				= 0x00000100,
		BUTTONFLAG_NONE					= 0
	};


	enum {
		FLAG_AIM_AT_TARGET_POINT		= 0x00000001,	// TRUE=auto-aim at m_TargetPoint_WS if within range. FALSE=just aim down CBot::m_MtxToWorld.m_vFront
		FLAG_FORCE_SNEAK				= 0x00000002,	// TRUE=force sneaking on
		FLAG_NO_AUTO_SNEAK				= 0x00000004,	// TRUE=don't allow sneaking to be automatically enabled based on the xlat speed
		FLAG_PANIC						= 0x00000008,	// Some bot's may have special animations while in panic mode
		FLAG_ALERT						= 0x00000010,	// Some bot's may have special animations while in alert mode
		FLAG_QUICKROTATE_XZ				= 0x00000020,	// Some bot's may have a way to turn quickly
		FLAG_USE_HEADLOOK				= 0x00000040,	// This means the interpereter of these controls should use m_HeadLookPt_WS as an "Ideal" thing to look at (with the head only)
		FLAG_NEW_HEADLOOK_LOCATION		= 0x00000080,	// Set this to true when a new headlook location should be read from m_HeadLookPt_WS
		FLAG_HOP_LEFT					= 0x00000100,	//	Some bot's may be capable of special moves
		FLAG_HOP_RIGHT					= 0x00000200,	//	   Note! ye who flips these bits need 
		FLAG_HOP_VERT					= 0x00000400,	//			 make sure that the 
		FLAG_ROLL_LEFT					= 0x00000800,	//			 surroundings are conducive to success 
		FLAG_ROLL_RIGHT					= 0x00001000,	//			 of the move if success is desired
		FLAG_FORCE_JUMP1				= 0x00002000,	// TRUE=force a jump1 this frame.  Cleared after the frame
		FLAG_FORCE_JUMP2				= 0x00004000,	// TRUE=force a jump2 this frame.  Cleared after the frame
		FLAG_NONE						= 0
	};




//----------------------------------------------------------------------------------------------------------------------------------
// Public Data:
//----------------------------------------------------------------------------------------------------------------------------------
public:
	CFVec3A m_TargetPoint_WS;		// Desired target point in world space (used when m_nFlags::FLAG_AIM_AT_TARGET_POINT is TRUE)
	CFVec3A m_XlatNormSpeedXZ_WS;	// Translation normalized speed in XZ world space
	CFVec3A m_JumpVelocity_WS;		// Jump velocity in world space
	CFVec3A m_HeadLookPt_WS;		// Specify something to look at (with the head)

	f32 m_fAimDown;					// +1=aim down, -1=aim up
	f32 m_fRotateCW;				// +1=rotate CW, -1=rotate CCW
	f32 m_fQuickRotateXZRads;		// +-PI rads to rotate if the FLAG_QUICKROTATE_XZ is specified

	f32 m_fFire1;					// >0: primary fire trigger pressed
	f32 m_fFire2;					// >0: secondary fire trigger pressed
	f32 m_fFlyUp;					// +1: fly up, -1: fly down

	u32 m_nButtons;					// See BUTTONFLAG_* for info
	u32 m_nFlags;					// See FLAG_* for info




//----------------------------------------------------------------------------------------------------------------------------------
// Public Functions:
//----------------------------------------------------------------------------------------------------------------------------------
public:
	CBotControl();

	virtual void Zero( void );


	FCLASS_STACKMEM_ALIGN( CBotControl );
} FCLASS_ALIGN_SUFFIX;




#endif

