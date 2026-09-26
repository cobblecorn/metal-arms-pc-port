//////////////////////////////////////////////////////////////////////////////////////
// ConvertApeFile.h - 
//
// Author: Michael Starich   
//////////////////////////////////////////////////////////////////////////////////////
// THIS CODE IS PROPRIETARY PROPERTY OF SWINGIN' APE STUDIOS, INC.
// Copyright (c) 2000
//
// The contents of this file may not be disclosed to third
// parties, copied or duplicated in any form, in whole or in part,
// without the prior written permission of Swingin' Ape Studios, Inc.
//////////////////////////////////////////////////////////////////////////////////////
// Modification History:
//
// Date     Who         Description
// -------- ----------  --------------------------------------------------------------
// 10/25/00 Starich     Created.
//////////////////////////////////////////////////////////////////////////////////////
#ifndef _CONVERT_APE_FILE_H_
#define _CONVERT_APE_FILE_H_ 1

#include "fang.h"
#include "ApeFileLoader.h"
#include "flight.h"
#include "ApeToKongFormat.h"
#include "shaders.h"
#include "aidfile.h"
#include "PasmDlg.h"


//
// APE master file entry flags
enum
{
	APE_ENTRY_FLAGS_VERTRAD		= 0x0001,
	APE_ENTRY_FLAGS_LIGHTMAPUV	= 0x0002,
};


// abstract base class for platform specific converters
class CConvertApeFile
{
public:
	CConvertApeFile();
	~CConvertApeFile();

//	virtual BOOL Convert( CApeToKongFormat &Kong, cchar *pszFilename, CAidFile *pAidFile, TargetPlatform_e nPlatform, BOOL bWorld=FALSE, BOOL bVertexRadiosity=FALSE ) = 0;
	virtual u32 GetSizeOfConvertedFile() = 0;
	virtual BOOL WriteConvertedFile( cchar *pszFilename, FILE *pFileStream=NULL ) = 0;
	virtual void FreeData() = 0;
	static FLightType_e ConvertLightType( ApeLightType_e nApeLightType );
	static u32 CreateTexInfoFlags( const ApeLayer_t *pLayer );
	static BOOL IsEmissiveNonBlack( const ApeLayer_t *pLayer );
//	static ShaderTypes_e MapApeMatToShaderType( const ApeMaterial_t *pApeMat );
	static u32 GetNumTextureCoords( const ApeMaterial_t *pApeMat );
	
	u32 m_nNumUVsBefore;
	u32 m_nNumUVsAfter;
	u32 m_nNumXYZsBefore;
	u32 m_nNumXYZsAfter;
	u32 m_nNumRGBsBefore;
	u32 m_nNumRGBsAfter;
	u32 m_nNumNormsBefore;
	u32 m_nNumNormsAfter;
	u32 m_nNumTanBefore;
	u32 m_nNumTanAfter;
	u32 m_nNumBiNBefore;
	u32 m_nNumBiNAfter;

private:
	
protected:
	
};

#endif

