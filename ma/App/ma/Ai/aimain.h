//////////////////////////////////////////////////////////////////////////////////////
// AIMain.h - AI Module interface
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
// 02/13/02 patm       Created.
//////////////////////////////////////////////////////////////////////////////////////
#ifndef _AIMAIN_H_
#define _AIMAIN_H_ 1

extern BOOL aimain_InitSystem(const char* pszLevelFilename);
extern BOOL aimain_InitSystemPostWorldLoad(void);
extern void aimain_Work(void);
extern void aimain_Draw(void);
extern BOOL aimain_CreateBots(void);
extern void aimain_DestroyBots(void);
extern void aimain_SetDebugString(cchar* pszString, f32 fHowLong = 3.0f);

class CGraphSearcher;
class CAIGraph;
class CAIGraphDataAccess;
extern CAIGraph	*aimain_pAIGraph;				//THE Graph
extern CGraphSearcher *aimain_pGraphSearcher;	//THE Path Finder
extern CAIGraphDataAccess *aimain_pGraphAccess;	 //some lookup tables to help access various components of the graph
#endif //_AIMAIN_H_