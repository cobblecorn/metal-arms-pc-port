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
#ifndef _MAIADDTOOL_H_
#define _MAIADDTOOL_H_ 1

#include "MaiSelectTool.h"

class AddTool: public SelectTool
{
public:
	AddTool(void);
	virtual void UserInput(const CMaiUserInput& rMaiUI);
	virtual void OnFocus(void);

	void SetJoinMode(BOOL bOnOff) { m_bJoinMode = bOnOff;};
	BOOL m_bJoinMode;
	BOOL m_bNoNewVertOnUp;
};


#endif