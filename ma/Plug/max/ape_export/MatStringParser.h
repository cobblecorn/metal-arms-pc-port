//////////////////////////////////////////////////////////////////////////////////////
// MatStringParser.h - 
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
// 02/08/01 Starich     Created.
//////////////////////////////////////////////////////////////////////////////////////
#ifndef _MAT_STRING_PARSER_H_
#define _MAT_STRING_PARSER_H_ 1

#include "fang.h"
#include "max.h"
#include "ape_file_def.h"

// parses a material string deciphering the * commands.
// The following are valid material * commands
// *sort
// *order
// *shader
// *motif
// *anim
// *scroll
// *z
// *id
// *nocoll
// *collid
// *coll
// *nodraw
// *surf
// *react
class CMatStringParser {
public:
	CMatStringParser();
	~CMatStringParser();

	void Parse( cchar *pszMaterialString, BOOL bResetToDefaults=TRUE );
	void ResetToDefaults();
	u32 GetNumberOfErrors();
	BOOL GetErrorString( u32 nErrorIndex, CStr &rsErrorString );

	CStr m_sOrigMatString;
	ApeCommands_t m_ApeCommands;// see ape_file_def.h
	u32 m_nMatFlags;
	u8  m_nAffectAngle; // For angular emissive or translucency

private:
	u32 m_nErrorMask;
protected:

};

// parses an object name string deciphering the * commands
// The following are valid object * commands
// *postery
// *posterx
// *posterz
// *nocoll
// *nofog
// *nolight
// *culldist
// *sort
// *nodraw
class CObjStringParser {
public:
	CObjStringParser();
	~CObjStringParser();

	void Parse( cchar *pszObjString );
	void ResetToDefaults();

	CStr m_sOrigString;
	u32 m_nCommands;// see APE_OB_FLAG_... in ape_file_def.h
	f32 m_fCullDist;
	CFColorRGB m_TintRGB;

private:

protected:

};

// parses a light name string deciphering the * commands
// The following are valid light * commands
// *self
// *noterrain
// *motif
class CLightStringParser {
public:
	CLightStringParser( BOOL bWorldFile );
	~CLightStringParser();

	void Parse( cchar *pszObjString );
	void ResetToDefaults();

	CStr m_sOrigString;
	u32 m_nCommands;// see APE_LIGHT_FLAG_... in ape_file_def.h
	u32 m_nMotifID;
	u32 m_nLightID;
	f32 m_fCoronaScale;
	char m_szCoronaTexture[TEXTURE_NAME_LEN];
	char m_szPerPixelTexture[TEXTURE_NAME_LEN];

private:
	BOOL m_bWorldFile;

protected:

};

// parses a portal name string deciphering the * commands
// The following are valid portal * commands
// *mirror
// *sound
// *1way or *oneway
// *anti
class CPortalStringParser {
public:
	CPortalStringParser();
	~CPortalStringParser();

	void Parse( cchar *pszPortalString );
	void ResetToDefaults();
	
	CStr m_sOrigString;
	u32 m_nCommands;// see APE_PORTAL_FLAG_... in ape_file_def.h
	
private:

protected:

};


#endif

