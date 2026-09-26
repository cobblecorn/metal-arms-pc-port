//////////////////////////////////////////////////////////////////////////////////////
// MAISelectTool.h - Tool for selecting verts (part of the graph editor)
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
#ifndef _MAISELECTTOOL_H_
#define _MAISELECTTOOL_H_ 1

#include "MaiBaseTool.h"
#include "fmath.h"


class SelectTool : public BaseTool
{
public:
	SelectTool(void);

	virtual void UserInput(const CMaiUserInput& rMaiUI);
	virtual void Draw(void);
	virtual void OnFocus(void);

	s32 m_bMakingDragRect;
	CFVec2 m_DragRectOrigin;
	CFVec2 m_DragRectCur;
	BOOL m_bBeginMove;
	u16 m_uMovingVerts;
	u16 m_uLastVertClicked;
	enum
	{
		MOVE_STYLE_2D = 0,
		MOVE_STYLE_3D,
	};
	u32 m_uMoveStyle;
};


extern void maiselectTool_DoRightClickCustomizeUI(const CMaiUserInput& rMaiUI, BOOL b3DOnly = FALSE);


#endif  //_MAISELECTTOOL_H_ 