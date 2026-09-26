//////////////////////////////////////////////////////////////////////////////////////
// ShadowedText.cpp - A class to simplify the managment and drawing of text
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

#include "fang.h"
#include "ShadowedText.h"
#include "Hud2.h"
#include <wchar.h> //for _vsnwprintf


//-----------------------------------------------------------------------------
void CShadowedText::Create( const FTextArea_t* pArea ) {

	// Just use the given area to create our main area
    m_hMainArea = ftext_Create( pArea );

	// Make a copy of the text area so we can change it
	FTextArea_t areaTemp = *pArea;
	areaTemp.fUpperLeftX += 0.005f;
	areaTemp.fUpperLeftY += 0.005f;
	areaTemp.fLowerRightX += 0.005f;
	areaTemp.fLowerRightY += 0.005f;
	areaTemp.oColorForeground.Set(0.0f, 0.0f, 0.0f, 1.0f);
	m_hShadowArea = ftext_Create(&areaTemp);
}

//-----------------------------------------------------------------------------
f32 CShadowedText::GetTop( void ) {
	return ftext_GetAttributes(m_hMainArea)->fUpperLeftY;
}

//-----------------------------------------------------------------------------
const CFColorRGBA* CShadowedText::GetColor( void ) {
	return &ftext_GetAttributes(m_hMainArea)->oColorForeground;
}

//-----------------------------------------------------------------------------
void CShadowedText::SetAlignment( FTextHorzAlign_e eAlign ) {
	ftext_GetAttributes(m_hMainArea)->oHorzAlign = eAlign;
	ftext_GetAttributes(m_hShadowArea)->oHorzAlign = eAlign;
}

//-----------------------------------------------------------------------------
void CShadowedText::SetColor( const CFColorRGBA& rFgColor ) {
	ftext_GetAttributes(m_hMainArea)->oColorForeground.Set(rFgColor);
	ftext_GetAttributes(m_hShadowArea)->oColorForeground.fAlpha = rFgColor.fAlpha;
}

//-----------------------------------------------------------------------------
void CShadowedText::PrintString( cwchar *pwszString ) {
	ftext_PrintString( m_hShadowArea, pwszString );
	ftext_PrintString( m_hMainArea, pwszString );
}

//-----------------------------------------------------------------------------
void CShadowedText::Printf( cwchar *pwszFormat, ... ) {
	va_list	pArgs;

	va_start( pArgs, pwszFormat );
	wchar tmp[160];
	_vsnwprintf( tmp, 160, pwszFormat, pArgs );

	ftext_Printf( m_hShadowArea, tmp );
	ftext_Printf( m_hMainArea, tmp );

	va_end( pArgs );
}

//-----------------------------------------------------------------------------
void CShadowedText::Xform_PrintString( cwchar *pwszString ) {
	Hud2_PrintString( m_hShadowArea, pwszString );
	Hud2_PrintString( m_hMainArea, pwszString );
}

//-----------------------------------------------------------------------------
void CShadowedText::Xform_Printf( cwchar *pwszFormat, ... ) {
	va_list	pArgs;

	va_start( pArgs, pwszFormat );
	wchar tmp[160];
	_vsnwprintf( tmp, 160, pwszFormat, pArgs );

	Hud2_PrintString( m_hShadowArea, tmp );
	Hud2_PrintString( m_hMainArea, tmp );

	va_end( pArgs );
}
