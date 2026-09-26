//////////////////////////////////////////////////////////////////////////////////////
// fontscript.h - "Writes out a hcomp compiliable font script file"
//
// Author: Michael Starich   
//////////////////////////////////////////////////////////////////////////////////////
// THIS CODE IS PROPRIETARY PROPERTY OF SWINGIN' APE STUDIOS, INC.
// Copyright (c) 2001
//
// The contents of this file may not be disclosed to third
// parties, copied or duplicated in any form, in whole or in part,
// without the prior written permission of Swingin' Ape Studios, Inc.
//////////////////////////////////////////////////////////////////////////////////////
// Modification History:
//
// Date     Who         Description
// -------- ----------  --------------------------------------------------------------
// 08/02/99 Starich     Created.
//////////////////////////////////////////////////////////////////////////////////////
#ifndef _FONTSCRIPT_H_
#define _FONTSCRIPT_H_ 1

#include "fang.h"
#include "fontpack.h"
#include "fontcut.h"
#include "tga.h"

extern BOOL fontscript_ModuleInit( void );
extern BOOL fontscript_WriteScriptFile( LPCTSTR pszFileName, LPCTSTR pszSourceFileName, PackData_t *pPack,
									    LPCTSTR pszComputerName, LPCTSTR pszUserName, LPCTSTR pszLetterString,
										LPCTSTR pszTGAPath, const f32 fLetterSpacing, const f32 fWordSpacing, const f32 fItalicSlant );

extern LPCTSTR fontscript_GetLastError( void );

#endif
