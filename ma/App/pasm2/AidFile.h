//////////////////////////////////////////////////////////////////////////////////////
// AidFile.h -
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
// 09/26/01 Starich     Created.
//////////////////////////////////////////////////////////////////////////////////////
#ifndef _AIDFILE_H_
#define _AIDFILE_H_ 1

#include "fang.h"
#include "fdata.h"
#include "fileinfo.h"

typedef struct {
	BOOL bValueWasFound;
	CString sValue;
} AidFileStringValue_t;

typedef struct {
	BOOL bValueWasFound;
	union {
		f32 fValue;
		BOOL bValue;
		u32 nValue;
	};
} AidFileNonStringValue_t;

typedef enum {
	AID_FILE_STRINGS_TEXTURE_USAGE = 0,
	AID_FILE_STRINGS_XB_TEX_FORMAT,
	AID_FILE_STRINGS_GC_TEX_FORMAT,
	AID_FILE_STRINGS_LOD_SWITCHES,
#ifdef _MMI_TARGET_PS2
    AID_FILE_STRINGS_PS2_TEX_FORMAT,
#endif
	AID_FILE_STRINGS_COUNT
} AidFileStrings_e;

typedef enum {
	AID_FILE_FLOATS_DITHER = 0,
	AID_FILE_FLOATS_GENERATE_MIPS,
	AID_FILE_FLOATS_MIP_TO,
	AID_FILE_FLOATS_GC_TEX_SCALE,
	AID_FILE_FLOATS_ANIM_COMPRESS,
	AID_FILE_FLOATS_TIME_COMPRESS,
	AID_FILE_FLOATS_THRESHOLD,
	AID_FILE_FLOATS_BUMP_SCALE,
	AID_FILE_FLOATS_BUMP_WRAP,
	AID_FILE_FLOATS_LIGHTMAP_DETAIL,
	AID_FILE_FLOATS_LIGHTMAP_MBS,
	AID_FILE_FLOATS_LIGHTMAP_SUBSAMPLE,
	AID_FILE_FLOATS_GENERATE_LMS,
	AID_FILE_FLOATS_COLLISION_LOD,
	AID_FILE_FLOATS_SHADOW_LOD_BIAS,
#ifdef _MMI_TARGET_PS2
	AID_FILE_FLOATS_LBIAS,
	AID_FILE_FLOATS_KBIAS,
	AID_FILE_FLOATS_MIPMAP_FILTER,
	AID_FILE_FLOATS_DOWNSIZE,
	AID_FILE_FLOATS_LAYER,
#endif
	AID_FILE_FLOATS_LIGHTMAP_SKYLIGHT_DETAIL,
	AID_FILE_FLOATS_LIGHTMAP_SKYLIGHT_HORIZ_RED,
	AID_FILE_FLOATS_LIGHTMAP_SKYLIGHT_HORIZ_GREEN,
	AID_FILE_FLOATS_LIGHTMAP_SKYLIGHT_HORIZ_BLUE,
	AID_FILE_FLOATS_LIGHTMAP_SKYLIGHT_ZENITH_RED,
	AID_FILE_FLOATS_LIGHTMAP_SKYLIGHT_ZENITH_GREEN,
	AID_FILE_FLOATS_LIGHTMAP_SKYLIGHT_ZENITH_BLUE,
	AID_FILE_FLOATS_LIGHTMAP_SKYLIGHT_INTENSITY,

	AID_FILE_FLOATS_COUNT
} AidFileFloats_e;


class CAidFile
{
public:
	CAidFile();
	~CAidFile();

	BOOL Load( const CFileInfo *pFileInfo );

	u32 m_nFileCRCValue;
	BOOL m_bDataValid;
	AidFileStringValue_t m_aPartStrings[FDATA_MAX_MESH_PARTS];
	AidFileStringValue_t m_aStrings[AID_FILE_STRINGS_COUNT];
	AidFileNonStringValue_t m_aNonStrings[AID_FILE_FLOATS_COUNT];

private:
	static BOOL IsCommentLine( const CString &rsString );
	static void ReplaceControlCharsWithNULL( u8 *pMem, u32 nLen );
	static BOOL SplitLineIntoLeftRight( const CString &rsString, CString &rsLeft, CString &rsRight );
	static BOOL DoesStringContainInvalidChars( const CString &rsString, BOOL bAllowSpaces );
	static char *GetNextLine( char *pszLine, char *pszEOF );

protected:

};


#endif

