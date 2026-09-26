//////////////////////////////////////////////////////////////////////////////////////
// MaiAddTool.h - Tool for adding pathfinding verts
// 
// Author: Pat MacKellar
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
// 02/19/02 patm       Created.
//////////////////////////////////////////////////////////////////////////////////////
#ifndef _MAIADDTOOL3D_H_
#define _MAIADDTOOL3D_H_ 1

#include "MaiSelectTool.h"

class AddTool3D: public SelectTool
{
public:
	AddTool3D(void);
	virtual void UserInput(const CMaiUserInput& rMaiUI);
	virtual void OnFocus(void);

	void SetJoinMode(BOOL bOnOff) { m_bJoinMode = bOnOff;};
	BOOL m_bJoinMode;
	f32 m_fLastAddDist;
	f32 m_fLastRad;
	f32 m_fLastHeight;
	BOOL m_bNoNewVertOnUp;
};


#endif