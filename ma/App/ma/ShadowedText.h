//////////////////////////////////////////////////////////////////////////////////////
// ShadowedText.h - A class to simplify the managment and drawing of text
// with a programmatic shadow. Note: The text drawn with a CShadowedText
// object is drawn twice, so should not be used where performance is paramount.
//
// Author: Dan Stanfill, Pinniped Software     
//////////////////////////////////////////////////////////////////////////////////////
// THIS CODE IS PROPRIETARY PROPERTY OF SWINGIN' APE STUDIOS, INC.
// Copyright (c) 2003
//
// The contents of this file may not be disclosed to third
// parties, copied or duplicated in any form, in whole or in part,
// without the prior written permission of Swingin' Ape Studios, Inc.
//////////////////////////////////////////////////////////////////////////////////////
// Modification History:
//
// Date     Who         Description
// -------- ----------  --------------------------------------------------------------
// 06/09/03 Stanfill    Created.
//////////////////////////////////////////////////////////////////////////////////////

#ifndef _ShadowedText_H_
#define _ShadowedText_H_ 1

#include "fang.h"
#include "ftext.h"

class CShadowedText {
private:
	FTextAreaHandle_t	m_hMainArea;
	FTextAreaHandle_t	m_hShadowArea;

public:
	// Creation. Mirrors ftext_Create
	void	Create( const FTextArea_t* pArea );

	// Get information
	f32		GetTop( void );
	const CFColorRGBA* GetColor( void );

	// Set various text attributes
	void	SetAlignment( FTextHorzAlign_e eAlign );
	void	SetColor( const CFColorRGBA& rFgColor );

	// Printing
	void	PrintString( cwchar *pwszString );
	void	Printf( cwchar *pwszFormat, ... );
	void	Xform_PrintString( cwchar *pwszString );
	void	Xform_Printf( cwchar *pwszFormat, ... );
};

#endif	// _ShadowedText_H_

