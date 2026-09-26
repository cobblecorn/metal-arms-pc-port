//////////////////////////////////////////////////////////////////////////////////////
// AIBTATable.h - 
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
// 09/10/02 MacKellar   Created.
//////////////////////////////////////////////////////////////////////////////////////
#ifndef _AIBTATABLE_H_
#define _AIBTATABLE_H_ 1

#include "fang.h"

class CAIBrain;

void AIBTA_InitSystem(void);
void AIBTA_UninitSystem(void);
cchar* AIBTA_EventToBTAName(cchar* pszEvent, CAIBrain* pBrain);
void AIBTA_ResetGlobalTimers(void);
BOOL AIBTA_IsBTATaunt(cchar* pszBTAName);
BOOL AIBTA_GroupLaughTime(CAIBrain* pBrain);
BOOL AIBTA_LoadBTAGroup(cchar *pszBTAName);

#endif

