//////////////////////////////////////////////////////////////////////////////////////
// MAIUndo.h - quick store/restore of the an ai graph and ai edit graph
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
// 03/05/02 patm       Created.
//////////////////////////////////////////////////////////////////////////////////////
#ifndef _MAIUNDO_H_
#define _MAIUNDO_H_ 1

void maiundo_Init(void);
void maiundo_Uninit(void);

void maiundo_Store(void);
void maiundo_Restore(void);

#endif
