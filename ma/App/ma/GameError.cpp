//////////////////////////////////////////////////////////////////////////////////////
// GameError.cpp - 
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
#if 0

#include "fang.h"
#include "GameError.h"
#include "floop.h"

//====================
// private definitions

//=================
// public variables

//==================
// private variables

u32 CGameError::s_uFlags = 0;
cwchar *CGameError::s_pwszCurErrorMsg = NULL;
FTextAreaHandle_t CGameError::s_hTextAreaHandle;

//===================
// private prototypes

//=================
// public functions


BOOL CGameError::InitSystem()
{
	s_uFlags = 0;
	s_pwszCurErrorMsg = NULL;

	FTextArea_t oTextArea;
	ftext_SetToDefaults(&oTextArea);
	oTextArea.bVisible = TRUE;
	oTextArea.fNumberOfLines = 5.0f;
	oTextArea.ohFont = '4';
	oTextArea.oColorBackground.Blue();
	oTextArea.oColorBackground.fAlpha = 0.5f;
	oTextArea.fBorderThicknessX = 0.002f;
	oTextArea.fBorderThicknessY = 0.002f;
	oTextArea.oColorBorder.OpaqueBlue();
	oTextArea.oHorzAlign = FTEXT_HORZ_ALIGN_CENTER;
	oTextArea.bVisible = FALSE;

	s_hTextAreaHandle = ftext_Create(&oTextArea);


	return(TRUE);
}



void CGameError::UninitSystem()
{
	s_pwszCurErrorMsg = NULL;
	s_uFlags = 0;
}



void CGameError::Work()
{
	if(s_uFlags == FLAGS_NONE)
	{
		return;
	}

	if(s_uFlags & FLAGS_ERRORMSGSTART)
	{
		floop_PauseGame(TRUE);
		ftext_GetAttributes(s_hTextAreaHandle)->bVisible = TRUE;
		s_uFlags &= ~(FLAGS_ERRORMSGSTART);
	}

	if(s_uFlags & FLAGS_ERRORMSGON)
	{
		ftext_PrintString(s_hTextAreaHandle, s_pwszCurErrorMsg);
	}

	if(s_uFlags & FLAGS_ERRORMSGEND)
	{
		floop_PauseGame(FALSE);
		ftext_GetAttributes(s_hTextAreaHandle)->bVisible = FALSE;
		s_uFlags &= ~(FLAGS_ERRORMSGEND | FLAGS_ERRORMSGON);
	}
}



void CGameError::Draw()
{
}



BOOL CGameError::SetErrorMsg(cwchar *pwszErrorMsg, BOOL bSetOn)
{
	if(bSetOn)
	{
		if(s_uFlags & FLAGS_ERRORMSGON)
		{
			// We're already displaying an error message, can't do another.
			return(FALSE);
		}

		s_uFlags |= (FLAGS_ERRORMSGON | FLAGS_ERRORMSGSTART);
		s_pwszCurErrorMsg = pwszErrorMsg;
	}
	else
	{
		if(!(s_uFlags & FLAGS_ERRORMSGON))
		{
			// We're not even displaying an error message.
			return(FALSE);
		}

		s_uFlags |= (FLAGS_ERRORMSGEND);
		s_pwszCurErrorMsg = NULL;
	}

	return(TRUE);
}

//==================
// private functions

#endif

