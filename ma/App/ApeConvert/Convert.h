//////////////////////////////////////////////////////////////////////////////////////
// Convert.h - 
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
// 04/20/01 Starich     Created.
//////////////////////////////////////////////////////////////////////////////////////
#ifndef _CONVERT_H_
#define _CONVERT_H_ 1

#include "fang.h"
#include "fversion.h"
#include "ape_file_def.h"
#include "old_ape_file_def.h"

#define CONVERT_OLD_APE_MAJOR		0x01 // 1 Byte
#define CONVERT_OLD_APE_MINOR		0x04 // 1 Byte
#define CONVERT_OLD_APE_SUB			0x01 // 1 Byte
//#define CONVERT_OLD_APE_MAJOR		0x01 // 1 Byte
//#define CONVERT_OLD_APE_MINOR		0x03 // 1 Byte
//#define CONVERT_OLD_APE_SUB		0x00 // 1 Byte
#define CONVERT_OLD_APE_VERSION		( FVERSION_CREATE_VERSION( FVERSION_APE_PLATFORM, CONVERT_OLD_APE_MAJOR, CONVERT_OLD_APE_MINOR, CONVERT_OLD_APE_SUB ) )

class COldApeFileLoader;

class CConvertApeFile 
{
public:
	CConvertApeFile();
	~CConvertApeFile();
	BOOL Convert( COldApeFileLoader &rOldApeFile, cchar *pszFilename );

private:
	
	void ConvertStruct( ApeMesh_t *pNew, OldApeMesh_t *pOld );
	void ConvertStruct( ApeBone_t *pNew, OldApeBone_t *pOld );
	void ConvertStruct( ApeLight_t *pNew, OldApeLight_t *pOld );
	void ConvertStruct( ApeObject_t *pNew, OldApeObject_t *pOld );
	void ConvertStruct( ApeFog_t *pNew, OldApeFog_t *pOld );
	void ConvertStruct( ApeShape_t *pNew, OldApeShape_t *pOld );
	void ConvertStruct( ApeVisVolume_t *pNew, OldApeVisVolume_t *pOld );
	void ConvertStruct( ApeVisPortal_t *pNew, OldApeVisPortal_t *pOld );
	void ConvertStruct( ApeSegment_t *pNew, OldApeSegment_t *pOld );
	void ConvertStruct( ApeMaterial_t *pNew, OldApeMaterial_t *pOld );
	void ConvertStruct( ApeVert_t *pNew, OldApeVert_t *pOld );
	void ConvertStruct( ApeVertIndex_t *pNew, OldApeVertIndex_t *pOld );
	void ConvertStruct( ApeCommands_t *pNew, OldApeCommands_t *pOld );
};

#endif

