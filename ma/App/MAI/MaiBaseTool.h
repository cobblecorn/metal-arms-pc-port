//////////////////////////////////////////////////////////////////////////////////////
// MAIBaseTool.h - Base Class for all tools in the Pathfinding graph editor
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
#ifndef _MAIBASETOOL_H_
#define _MAIBASETOOL_H_ 1

#include "maimain.h"

class BaseTool
{
public:
	virtual void UserInput(const CMaiUserInput& rMaiUI) = 0;
	virtual void OnFocus(void) {};
	virtual void Draw(void) {};
};

#endif //_MAIBASETOOL_H_