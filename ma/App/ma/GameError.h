//////////////////////////////////////////////////////////////////////////////////////
// GameError.h - System for displaying system error messages to the user.
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
// 08/03/02 Link        Created.
//////////////////////////////////////////////////////////////////////////////////////
#ifndef _GAMEERROR_H_
#define _GAMEERROR_H_ 1

#include "fang.h"
#include "ftext.h"



FCLASS_NOALIGN_PREFIX class CGameError
{
public:
	static BOOL InitSystem();
	static void UninitSystem();
	static void Work();
	static void Draw();

	// Pass NULL for pszErrorMsg if you are turning off the error message.
	static BOOL SetErrorMsg(cwchar *pwszErrorMsg, BOOL bSetOn);

private:
	enum Flags_e
	{
		FLAGS_ERRORMSGON		=	1 << 0,
		FLAGS_ERRORMSGSTART		=	1 << 1,			// Our next Work() will be the first one with the error message.
		FLAGS_ERRORMSGEND		=	1 << 2,			// Our next Work() will be the last one with the error message.

		FLAGS_NONE				=	0
	};

	static u32 s_uFlags;

	static cwchar *s_pwszCurErrorMsg;

	static FTextAreaHandle_t s_hTextAreaHandle;

} FCLASS_NOALIGN_SUFFIX;


#endif

