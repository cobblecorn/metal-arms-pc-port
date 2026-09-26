//////////////////////////////////////////////////////////////////////////////////////
// AIUtil.cpp - 
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
// 02/06/02 MacKellar   Created.
//////////////////////////////////////////////////////////////////////////////////////
#include "fang.h"
#include "fres.h"
#include "fclib.h"
#include "AIUtil.h"
#include "fmath.h"
#include "fcoll.h"
#include "fworld.h"
#include "floop.h"
#include <stdio.h>	//sscanf, _sprintf


const char* aiutil_NextAsciiToken(const char* psData, const char* pDataEnd)
{
	while (psData && psData < pDataEnd && !(*psData==' ' || *psData==13 || *psData=='\n' || *psData=='\t' || *psData==',' || *psData== '=' ))
	{
		psData++;
	}

	while (psData && psData < pDataEnd && (*psData==' ' || *psData==13 || *psData=='\n' || *psData=='\t' || *psData==',' || *psData== '='))
	{
		psData++;
	}
	
	if (psData < pDataEnd)
		return psData;

	return NULL;
}


BOOL aiutil_MatchAsciiToken(const char* pszLabel, const char* psTokenPos, const char* pDataEnd)
{
	s32 len = fclib_strlen(pszLabel);
	if (pDataEnd - psTokenPos < len)	  //don't check past data end
		return FALSE;

	return (fclib_strnicmp(pszLabel, psTokenPos, len) == 0);
}


s32 aiutil_AsciiIntToken(const char* psTokenPos, const char* pDataEnd)
{
	s32 i = 0;
	s32 count = sscanf(psTokenPos, "%d", &i);
	FASSERT(count == 1);

	return i;
}


u32 aiutil_AsciiUnsignedIntToken(const char* psTokenPos, const char* pDataEnd)
{
	u32 i = 0;
	s32 count = sscanf(psTokenPos, "%u", &i);
	FASSERT(count == 1);

	return i;

}


float aiutil_AsciiFloatToken(const char* psTokenPos, const char* pDataEnd)
{
	float i = 0;
	s32 count = sscanf(psTokenPos, "%f", &i);
	FASSERT(count == 1);
	return i;
}

static char _cTokenDelimiter = '\n';
static s32 _nIndentLevel = 0;
static char _cIndentChar = ' ';
static s32 _nIndentSize = 4;


char *aiutil_EmitAsciiLabel(const char* pszLabel, char* psTokenPos, const char* pDataEnd)
{
	s32 i = 0;
	while (psTokenPos < pDataEnd && i++ < _nIndentSize*_nIndentLevel)
		*(psTokenPos++) = _cIndentChar;

	while (pszLabel && *pszLabel !='\0' && psTokenPos < pDataEnd)
		*(psTokenPos++) = *(pszLabel++);

	if (psTokenPos < pDataEnd) 
		*(psTokenPos++) = _cTokenDelimiter;

	if (psTokenPos >= pDataEnd)
		return NULL;

	return psTokenPos;

}


char *aiutil_EmitAsciiInt(const char* pszLabel, s32 nValue, char* psTokenPos, const char* pDataEnd)
{
	s32 i = 0;
	while (psTokenPos < pDataEnd && i++ < _nIndentSize*_nIndentLevel)
		*(psTokenPos++) = _cIndentChar;

	while (pszLabel && *pszLabel !='\0' && psTokenPos < pDataEnd)
		*(psTokenPos++) = *(pszLabel++);

	if (psTokenPos < pDataEnd) 
		*(psTokenPos++) = _cIndentChar;

	s32 count = _snprintf(psTokenPos, pDataEnd-psTokenPos, "%d", nValue);
	psTokenPos+=count;

	if (psTokenPos < pDataEnd) 
		*(psTokenPos++) = _cTokenDelimiter;

	if (psTokenPos >= pDataEnd)
		return NULL;

	return psTokenPos;
}

char *aiutil_EmitAsciiUnsignedInt(const char* pszLabel, u32 uValue, char* psTokenPos, const char* pDataEnd)
{
	s32 i = 0;
	while (psTokenPos < pDataEnd && i++ < _nIndentSize*_nIndentLevel)
		*(psTokenPos++) = _cIndentChar;

	while (pszLabel && *pszLabel !='\0' && psTokenPos < pDataEnd)
		*(psTokenPos++) = *(pszLabel++);

	if (psTokenPos < pDataEnd) 
		*(psTokenPos++) = _cIndentChar;

	s32 count = _snprintf(psTokenPos, pDataEnd-psTokenPos, "%u", uValue);
	psTokenPos+=count;

	if (psTokenPos < pDataEnd) 
		*(psTokenPos++) = _cTokenDelimiter;

	if (psTokenPos >= pDataEnd)
		return NULL;

	return psTokenPos;
}

char *aiutil_EmitAsciiFloat(const char* pszLabel, float value, char* psTokenPos, const char* pDataEnd)
{
	s32 i = 0;
	while (psTokenPos < pDataEnd && i++ < _nIndentSize*_nIndentLevel)
		*(psTokenPos++) = _cIndentChar;

	while (pszLabel && *pszLabel !='\0' && psTokenPos < pDataEnd)
		*(psTokenPos++) = *(pszLabel++);

	if (psTokenPos < pDataEnd) 
		*(psTokenPos++) = _cIndentChar;

	s32 count = _snprintf(psTokenPos, pDataEnd-psTokenPos, "%f", value);
	psTokenPos+=count;

	if (psTokenPos < pDataEnd) 
		*(psTokenPos++) = _cTokenDelimiter;

	if (psTokenPos >= pDataEnd)
		return NULL;

	return psTokenPos;
}

char *aiutil_EmitAsciiString(const char* pszLabel, const char* pszString, char* psTokenPos, const char* pDataEnd)
{
	s32 i = 0;
	while (psTokenPos < pDataEnd && i++ < _nIndentSize*_nIndentLevel)
		*(psTokenPos++) = _cIndentChar;

	while (pszLabel && *pszLabel !='\0' && psTokenPos < pDataEnd)
		*(psTokenPos++) = *(pszLabel++);

	if (psTokenPos < pDataEnd) 
		*(psTokenPos++) = _cIndentChar;

	s32 count = _snprintf(psTokenPos, pDataEnd-psTokenPos, "%s", pszString);
	psTokenPos+=count;

	if (psTokenPos < pDataEnd) 
		*(psTokenPos++) = _cTokenDelimiter;

	if (psTokenPos >= pDataEnd)
		return NULL;

	return psTokenPos;
}

void aiutil_IncAsciiIndent(void)
{
	_nIndentLevel++;
}


void aiutil_DecAsciiIndent(void)
{
	_nIndentLevel--;
}


void aiutil_SetAsciiIndent(s32 nLevel)
{
	_nIndentLevel = nLevel;
}


void aiutil_SetTokenDelimiter(const char cDelimeter)
{
	_cTokenDelimiter = cDelimeter;
}


/*
void aiutil_3DText(const CFVec3& pos_WS, const char* pszText)
{

	FViewport_t * pView = fviewport_GetActive( );
	
	CFVec3 ScreenPos;
	fviewport_ComputeUnitOrtho3DScreenPoint_WS(pView, &(FXfm_pView->m_MtxF), &pAI->pBot->Pos_WS, &ScreenPos);
	if (ScreenPos.z >0.0f && ScreenPos.x < 1.0f && ScreenPos.x > -1.0f && ScreenPos.y < 1.0f && ScreenPos.y > -1.0f)
	{
		ftext_DebugPrintf( 0.5f+0.5f*ScreenPos.x, (0.75f*0.5f-ScreenPos.y*0.5f), "~arDX: ~w1%02d", pAI->nAIGUID);
	}
}
*/

