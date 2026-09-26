//////////////////////////////////////////////////////////////////////////////////////
// ConvertMtxFiles.h - 
//
// Author: Michael Starich   
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
// 04/03/02 Starich     Created.
//////////////////////////////////////////////////////////////////////////////////////
#ifndef _CONVERT_MTX_FILES_H_
#define _CONVERT_MTX_FILES_H_ 1

#include "fang.h"
#include "ConvertFile.h"
#include "fdata.h"

// john: I use this as the default threshold value (the one used if there is no aid file)
#define CONVERT_MTX_FILES_ANIM_QUAT_COMPRESSION_THRESHHOLD	0.9995f
//#define CONVERT_MTX_FILES_ANIM_QUAT_COMPRESSION_THRESHHOLD	1.f

class CConvertMtxFile : public CConvertFile
{
public:
	CConvertMtxFile();
	~CConvertMtxFile();

	virtual BOOL ConvertFile( const CFileInfo *pFileInfo, BOOL bConvertToBigEndian=FALSE );
	virtual u32  GetDataCRC( void );
	virtual u32  GetSizeOfConvertedFile( void );
	virtual BOOL WriteConvertedFile( cchar *pszFilename, FILE *pFileStream=NULL );
	virtual void FreeData( void );

	void SetCompressionVars( BOOL bCompressTime, BOOL bCompressAnim, f32 fThreshold );

	u32 m_nBoneCount;
	u32 m_nTotalFrameCount;

private:
	u32 m_nBytesAllocated;
	u8 *m_pMemAllocated;
	u8 *m_pCompiledFile;
	u32 m_nCompiledBytes;

	// compression vars
	BOOL m_bCompressTime;
	BOOL m_bCompressAnim;
	f32 m_fThreshold;

	void *CompressAnimationData( const CString &sFilepath, const FData_MtxFileHeader_t *pHeader,
								 u32 nNumHeaderBytes, BOOL bConvertToBigEndian, u32 &rnNumCompressedBytes );
	
protected:
	
};

#endif

