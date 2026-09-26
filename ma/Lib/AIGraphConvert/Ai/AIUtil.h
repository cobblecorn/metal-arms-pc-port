//////////////////////////////////////////////////////////////////////////////////////
// AIUtil.h - A collection of utility functions used by the AI
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
// 02/06/02 patm       Created.
// 03/27/02 patm       Added EdgeLock System
// 05/**/02 patm		moved all game code to aigameutils  this file is now just for the tools
//////////////////////////////////////////////////////////////////////////////////////

#ifndef _AIUTIL_H_
#define _AIUTIL_H_ 1

extern const char *aiutil_NextAsciiToken(const char* psData, const char* pDataEnd);
extern BOOL        aiutil_MatchAsciiToken(const char* pszLabel, const char* sTokenPos, const char* pDataEnd);
extern s32         aiutil_AsciiIntToken(const char* psTokenPos, const char* pDataEnd);
extern u32         aiutil_AsciiUnsignedIntToken(const char* psTokenPos, const char* pDataEnd);
extern f32         aiutil_AsciiFloatToken(const char* psTokenPos, const char* pDataEnd);
extern char		  *aiutil_EmitAsciiLabel(const char* pszLabel, char* psTokenPos, const char* pDataEnd);
extern char		  *aiutil_EmitAsciiInt(const char* pszLabel, s32 nValue, char* psTokenPos, const char* pDataEnd);
extern char		  *aiutil_EmitAsciiUnsignedInt(const char* pszLabel, u32 uValue, char* psTokenPos, const char* pDataEnd);
extern char		  *aiutil_EmitAsciiFloat(const char* pszLabel, f32 fValue, char* psTokenPos, const char* pDataEnd);
extern char		  *aiutil_EmitAsciiString(const char* pszLabel, const char* pszString, char* psTokenPos, const char* pDataEnd);
extern void		   aiutil_IncAsciiIndent(void);
extern void		   aiutil_DecAsciiIndent(void);
extern void		   aiutil_SetAsciiIndent(s32 nLevel);
extern void		   aiutil_SetTokenDelimiter(const char cDelimeter);

#endif //_AIUTIL_H_