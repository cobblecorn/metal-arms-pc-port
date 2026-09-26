//////////////////////////////////////////////////////////////////////////////////////
// Gesture.h - 
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
// 08/15/02 Link        Created.
//////////////////////////////////////////////////////////////////////////////////////
#ifndef _GESTURE_H
#define _GESTURE_H 1

#include "fanim.h"
#include "BoneGroupList.h"

// This class represents a gesture type.  
class CGesture
{
public:
	CGesture();
	~CGesture();

	BOOL Init(CString strName, CString strSrcFile, u32 uFlags);
	void Copy(CGesture *pCopy);

	enum Flags_e
	{
		FLAG_UPPERBODY		=	0x00000001,
		FLAG_LOWERBODY		=	0x00000002,
		FLAG_NOBLENDOUT		=	0x00000004,
		FLAG_STICKATEND		=	0x00000008,
		FLAG_UPPER_TORSO	=	0x00000010,
		FLAG_LOWER_TORSO	=	0x00000020,
		FLAG_LEFT_ARM		=	0x00000040,
		FLAG_RIGHT_ARM		=	0x00000080,
		FLAG_HEAD			=	0x00000100,

		FLAG_NONE		=	0
	};

	FAnim_t *m_pAnim;
	CString m_strName;			// Name of the gesture as it appears in the animation file.
	CString m_strSrcFile;		// Name of the file from which the animation is taken (not necessary).
	u32 m_uFlags;				// It drives the upper body, lower body, etc.
};

// This class represents a specific instance of a gesture in a dialogue.
class CGestureInst
{
public:
	CGestureInst();

	void Copy(const CGestureInst *pCopy);
	BOOL Init(const CGesture *pGesture, f32 fStartTime);

	const CGesture *m_pGesture;
	f32 m_fStartTime;
	CFAnimInst *m_pAnimInst;
	u32 m_uAnimSlot;
};

#endif
