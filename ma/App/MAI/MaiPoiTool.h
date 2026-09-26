//////////////////////////////////////////////////////////////////////////////////////
// MaiPoiTool.h - 
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
// 08/16/02 MacKellar   Created.
//////////////////////////////////////////////////////////////////////////////////////
#ifndef _MAIPOITOOL_H_
#define _MAIPOITOOL_H_ 1

#include "MaiBaseTool.h"
#include "fmath.h"
#include "AI\ftl.h"

class CPoiTool : public BaseTool
{
public:
	CPoiTool(void);

	virtual void UserInput(const CMaiUserInput& rMaiUI);
	virtual void Draw(void);

	static s32 m_bMakingDragRect;
	static CFVec2 m_DragRectOrigin;
	static CFVec2 m_DragRectCur;
	BOOL m_bBeginMove;
	u16 m_nMovingPoi;
	u16 m_nUnselectPoiIdOnUp;

	enum
	{
		MOVE_STYLE_2D = 0,
		MOVE_STYLE_3D,
	};
	u32 m_uMoveStyle;
	
	BOOL m_bOffensivePoiMode;  //true if all poi's place should flagged as offensive ones
		
	static void InitSystem(void);
	static void UninitSystem(void);
	static void SelectPoi(CGraphPoi* pPoi);
	static void UnselectPoi(CGraphPoi* pPoi);
	static BOOL IsPoiSelected(CGraphPoi* pPoi);
	static void ClearSelectedPoi(void);
	static void DeleteSelectedPoi(void);

	static void RenderPoi(void);

	static CNiList<CGraphPoi*> m_SelectedPoi;
};



#endif

