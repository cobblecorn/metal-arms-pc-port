//////////////////////////////////////////////////////////////////////////////////////
// MaiJoinTool.h - Tool for joining verts in the pathfinding graph
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
// 02/20/02 patm       Created
//////////////////////////////////////////////////////////////////////////////////////
#ifndef _MAIJOINTOOL_H_
#define _MAIJOINTOOL_H_ 1

#include "MaiBaseTool.h"

class JoinTool: public BaseTool
{
public:
	JoinTool();
	virtual void UserInput(const CMaiUserInput& rMaiUI);
	virtual void OnFocus(void);

	s32 m_nVertAId;	//the id of one of the verts that is selected
	BOOL m_bUnLinkMode;

	enum
	{
		JOIN_STYLE_2D = 0,
		JOIN_STYLE_3D,
	};
	u32 m_uJoinStyle;
};


#endif // _MAIJOINTOOL_H_