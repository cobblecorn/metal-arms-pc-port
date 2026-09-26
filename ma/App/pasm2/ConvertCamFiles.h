//////////////////////////////////////////////////////////////////////////////////////
// ConvertCamFiles.h - 
//
// Author: Russell Foushee   
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
// 01/22/03 Foushee     Created.
//////////////////////////////////////////////////////////////////////////////////////
#ifndef _CONVERT_CAM_FILES_H_
#define _CONVERT_CAM_FILES_H_ 1

#include "fang.h"
#include "ConvertFile.h"
#include "fdata.h"
#include "cam_file_def.h"

class CConvertCamFile : public CConvertFile
{
public:
	CConvertCamFile();
	~CConvertCamFile();

	virtual BOOL ConvertFile( const CFileInfo *pFileInfo, BOOL bConvertToBigEndian=FALSE );
	virtual u32  GetDataCRC();
	virtual u32  GetSizeOfConvertedFile();
	virtual BOOL WriteConvertedFile( cchar *pszFilename, FILE *pFileStream=NULL );
	virtual void FreeData();

	void SetCompressionVars( BOOL bCompressAnim );

private:

	//MAX .CAM variables
	u32 m_nUncompressedBytesAllocated;
	u8 *m_pUncompressedMemAllocated;
	CamInfo_t *m_pHeader; //derived from m_pUncompressedMemAllocated;
	CamFrame_t *m_paFrames; //derived from m_pUncompressedMemAllocated;

	// compression settings
	BOOL m_bCompressAnim;

	// Internal compression variables
	u8 *m_pWorkingMemory;		// memory allocated which the pointers below are derived from
	u32 m_nWorkingBytesAllocated; //the size of the m_pWorkingMemory chunk

	u16		m_nFKeyCount;		// Number of FOV key frames for this animation
	u16		m_nTKeyCount;		// Number of translation key frames for this animation
	u16		m_nOKeyCount;		// Number of orientation key frames for this animation

	u16		m_nTotalKeyTimes;	// The total number of Key times stored in m_paKeyTimes;
	f32		*m_paKeyTimes;		// Array of times at which particular keyframes occur
	u16		*m_paFKeyTimeIdx;	// Array of time indexes into m_paKeyTimes at which each FOV key frame occurs
	u16		*m_paTKeyTimeIdx;	// Array of time indexes into m_paKeyTimes at which each Translation key frame occurs
	u16		*m_paOKeyTimeIdx;	// Array of time indexes into m_paKeyTimes at which each Orientation key frame occurs
	
	f32		*m_paFKeyData;		// FOV key data array
	CFVec3	*m_paTKeyData;		// Translation key data array
	CFQuat	*m_paOKeyData;		// Orientation key data array

	//PASM .CAM variables
	u8 *m_pCompiledFile;
	u32 m_nCompiledBytes;

	BOOL LoadMaxCamFile( const CFileInfo *pFileInfo );
	BOOL CompressCamFile ( CString & sFilepath );
protected:
	
};





#endif

