//////////////////////////////////////////////////////////////////////////////////////
// ConvertCamFiles.cpp - 
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
#include "stdafx.h"
#include "fang.h"
#include "ConvertCamFiles.h"
#include "ErrorLog.h"
#include "fmath.h"
#include "fdata.h"
#include "fclib.h"
#include "fcamanim.h"

#define _ERROR_HEADING				".CAM FILE COMPILER " 



CConvertCamFile::CConvertCamFile() {
	m_nUncompressedBytesAllocated = 0;
	m_pUncompressedMemAllocated = NULL;
	m_pCompiledFile = NULL;
	m_nCompiledBytes = 0;

	m_pWorkingMemory = NULL;
	m_nWorkingBytesAllocated = NULL;

	m_nFKeyCount = 0;
	m_nTKeyCount = 0;
	m_nOKeyCount = 0;

	m_nTotalKeyTimes = 0;
	m_paKeyTimes = NULL;		// Array of times at which particular keyframes occur
	m_paFKeyTimeIdx = NULL;
	m_paTKeyTimeIdx = NULL;
	m_paOKeyTimeIdx = NULL;
	
	m_paFKeyData = NULL;
	m_paTKeyData = NULL;
	m_paOKeyData = NULL;

	m_bCompressAnim = TRUE;
}

CConvertCamFile::~CConvertCamFile() {
	FreeData();
}

void CConvertCamFile::SetCompressionVars( BOOL bCompressAnim ) {
	m_bCompressAnim = bCompressAnim;
}

BOOL CConvertCamFile::ConvertFile( const CFileInfo *pFileInfo, BOOL bConvertToBigEndian/*=FALSE*/ ) {
	CString sFilepath, sError;
	u32 i;

	FreeData();

	if( !pFileInfo ) {
		return FALSE;
	}

	CErrorLog &rErrorLog = CErrorLog::GetCurrent();

	// gather some info about the file
	sFilepath = pFileInfo->GetFilePath();


	////////////////////////
	// read our MAX exported cam file
	if( !LoadMaxCamFile( pFileInfo ) ) {
		return FALSE;
	}


	// now compress the Cam file
	if( !CompressCamFile( sFilepath ) ){
		return FALSE;
	}

	////////////////////////////////////////////////////////////////
	// Determine the size and addresses of data to allocate
	////////////////////////////////////////////////////////////////
	u32 nDataBytes = 0;

	u32 ADDR_FCamAnim =		FMATH_BYTE_ALIGN_UP( nDataBytes, 4 );
	nDataBytes =			ADDR_FCamAnim + sizeof( FCamAnim_t );

	u32 ADDR_FKeyTimes =	FMATH_BYTE_ALIGN_UP( nDataBytes, 4 );
	nDataBytes =			ADDR_FKeyTimes + ( sizeof( FCamAnimKeyTime_t ) * m_nTotalKeyTimes );

	u32 ADDR_FFKeyIdxs =	FMATH_BYTE_ALIGN_UP( nDataBytes, 4 );
	nDataBytes =			ADDR_FFKeyIdxs + ( sizeof( FCamAnimKeyTimeIdx_t ) * m_nFKeyCount );

	u32 ADDR_FTKeyIdxs =	FMATH_BYTE_ALIGN_UP( nDataBytes, 4 );
	nDataBytes =			ADDR_FTKeyIdxs + ( sizeof( FCamAnimKeyTimeIdx_t ) * m_nTKeyCount );

	u32 ADDR_FOKeyIdxs =	FMATH_BYTE_ALIGN_UP( nDataBytes, 4 );
	nDataBytes =			ADDR_FOKeyIdxs + ( sizeof( FCamAnimKeyTimeIdx_t ) * m_nOKeyCount );

	u32 ADDR_FFKeys =		FMATH_BYTE_ALIGN_UP( nDataBytes, 4 );
	nDataBytes =			ADDR_FFKeys + ( sizeof( FCamAnimFKey_t ) * m_nFKeyCount );

	u32 ADDR_FTKeys =		FMATH_BYTE_ALIGN_UP( nDataBytes, 4 );
	nDataBytes =			ADDR_FTKeys + ( sizeof( FCamAnimTKey_t ) * m_nTKeyCount );

	u32 ADDR_FOKeys =		FMATH_BYTE_ALIGN_UP( nDataBytes, FCLASS_BYTE_ALIGN );
	nDataBytes =			ADDR_FOKeys + ( sizeof( FCamAnimOKey_t ) * m_nOKeyCount );

	// Round it up
	nDataBytes = FMATH_BYTE_ALIGN_UP( nDataBytes, 4 );

	// Allocate memory to contain export data
	u8 *pExportData = (u8 *)fang_MallocAndZero( nDataBytes, FCLASS_BYTE_ALIGN );
	if ( pExportData == NULL ) 
	{
		DEVPRINTF( "CConvertCamFile::ConvertFile(): Could not allocate %u bytes.\n", nDataBytes );
		return FALSE;
	}

	u8 *pDataEnd = pExportData + nDataBytes;

	////////////////////////////////////////////////////////////////
	// Fill in export data memory
	////////////////////////////////////////////////////////////////

	//////////////////////////
	// PACK THE FCamAnim_t
	FCamAnim_t *pCamAnim = (FCamAnim_t *)(pExportData + ADDR_FCamAnim);
	strcpy(pCamAnim->szName, m_pHeader->szCameraName);
	pCamAnim->fTotalSeconds =	m_paKeyTimes[ m_nTotalKeyTimes - 1 ];
	pCamAnim->fOOTotalSeconds = 1.0f / pCamAnim->fTotalSeconds;
	pCamAnim->nFlags = 0;
	pCamAnim->nFKeyCount =		m_nFKeyCount;
	pCamAnim->nTKeyCount =		m_nTKeyCount;
	pCamAnim->nOKeyCount =		m_nOKeyCount;
	pCamAnim->nTotalKeyTimes =	m_nTotalKeyTimes;
	pCamAnim->paKeyTimes =		( FCamAnimKeyTime_t *)ADDR_FKeyTimes;
	pCamAnim->paFKeyTimeIdx =	( FCamAnimKeyTimeIdx_t *)ADDR_FFKeyIdxs;
	pCamAnim->paTKeyTimeIdx =	( FCamAnimKeyTimeIdx_t *)ADDR_FTKeyIdxs;
	pCamAnim->paOKeyTimeIdx =	( FCamAnimKeyTimeIdx_t *)ADDR_FOKeyIdxs;
	pCamAnim->paFKeyData =		( FCamAnimFKey_t *) ADDR_FFKeys;
	pCamAnim->paTKeyData =		( FCamAnimTKey_t *) ADDR_FTKeys;
	pCamAnim->paOKeyData =		( FCamAnimOKey_t *) ADDR_FOKeys;

	////////////////////////////////////////
	// PACK THE FCamAnim_t::paKeyTimes array
	FCamAnimKeyTime_t *paKeyTimes = (FCamAnimKeyTime_t *)(pExportData + ADDR_FKeyTimes);
	for( i=0; i<pCamAnim->nTotalKeyTimes; i++ ){
		paKeyTimes[ i ].fTime = m_paKeyTimes[ i ];
		if( bConvertToBigEndian ) {
			paKeyTimes[ i ].ChangeEndian();
		}
	}

	///////////////////////////////////////////
	// PACK THE FCamAnim_t::paFKeyTimeIdx array
	FCamAnimKeyTimeIdx_t *paFKeyTimeIdx = (FCamAnimKeyTimeIdx_t *)(pExportData + ADDR_FFKeyIdxs);
	for( i=0; i<pCamAnim->nFKeyCount; i++ ){
		paFKeyTimeIdx[ i ].uIdx = m_paFKeyTimeIdx[ i ];
		if( bConvertToBigEndian ) {
			paFKeyTimeIdx[ i ].ChangeEndian();
		}
	}

	///////////////////////////////////////////
	// PACK THE FCamAnim_t::paTKeyTimeIdx array
	FCamAnimKeyTimeIdx_t *paTKeyTimeIdx = (FCamAnimKeyTimeIdx_t *)(pExportData + ADDR_FTKeyIdxs);
	for( i=0; i<pCamAnim->nTKeyCount; i++ ){
		paTKeyTimeIdx[ i ].uIdx = m_paTKeyTimeIdx[ i ];
		if( bConvertToBigEndian ) {
			paTKeyTimeIdx[ i ].ChangeEndian();
		}
	}

	///////////////////////////////////////////
	// PACK THE FCamAnim_t::paOKeyTimeIdx array
	FCamAnimKeyTimeIdx_t *paOKeyTimeIdx = (FCamAnimKeyTimeIdx_t *)(pExportData + ADDR_FOKeyIdxs);
	for( i=0; i<pCamAnim->nOKeyCount; i++ ){
		paOKeyTimeIdx[ i ].uIdx = m_paOKeyTimeIdx[ i ];
		if( bConvertToBigEndian ) {
			paOKeyTimeIdx[ i ].ChangeEndian();
		}
	}

	///////////////////////////////////////////
	// PACK THE FCamAnim_t::paFKeyData array
	FCamAnimFKey_t *paFKeyData = (FCamAnimFKey_t *)(pExportData + ADDR_FFKeys);
	for( i=0; i<pCamAnim->nFKeyCount; i++ ){
		paFKeyData[ i ].fFOV = m_paFKeyData[ i ];
		if( bConvertToBigEndian ) {
			paFKeyData[ i ].ChangeEndian();
		}
	}

	///////////////////////////////////////////
	// PACK THE FCamAnim_t::paTKeyData array
	FCamAnimTKey_t *paTKeyData = (FCamAnimTKey_t *)(pExportData + ADDR_FTKeys);
	for( i=0; i<pCamAnim->nTKeyCount; i++ ){
		paTKeyData[ i ].vPosition = m_paTKeyData[ i ];
		if( bConvertToBigEndian ) {
			paTKeyData[ i ].ChangeEndian();
		}
	}

	///////////////////////////////////////////
	// PACK THE FCamAnim_t::paOKeyData array
	FCamAnimOKey_t *paOKeyData = (FCamAnimOKey_t *)(pExportData + ADDR_FOKeys);
	for( i=0; i<pCamAnim->nOKeyCount; i++ ){
		//copy over from a CFQuat to a CFQuatA
		paOKeyData[ i ].qOrientation.x = m_paOKeyData[ i ].x;
		paOKeyData[ i ].qOrientation.y = m_paOKeyData[ i ].y;
		paOKeyData[ i ].qOrientation.z = m_paOKeyData[ i ].z;
		paOKeyData[ i ].qOrientation.w = m_paOKeyData[ i ].w;
		if( bConvertToBigEndian ) {
			paOKeyData[ i ].ChangeEndian();
		}
	}

	if( bConvertToBigEndian ) {
		pCamAnim->ChangeEndian();
	}

	//update our statistics variables
	m_pCompiledFile = pExportData;
	m_nCompiledBytes = nDataBytes;

	return TRUE;
}

u32 CConvertCamFile::GetDataCRC() {

	if( !m_pCompiledFile ) {
		return FALSE;
	}

	u32 nReturnCRC = fmath_Crc32( 0, (u8 *)m_pCompiledFile, m_nCompiledBytes );

	return nReturnCRC;
}

u32 CConvertCamFile::GetSizeOfConvertedFile() {
	
	if( !m_nCompiledBytes ) {
		return 0;
	}
	return m_nCompiledBytes;
}

BOOL CConvertCamFile::WriteConvertedFile( cchar *pszFilename, FILE *pFileStream/*=NULL*/ ) {

	if( !m_nCompiledBytes ) {
		return FALSE;
	}
	BOOL bCloseFile = FALSE;
	if( !pFileStream ) {
		if( !pszFilename ) {
			// invalid filename
			return FALSE;
		}
		pFileStream = _tfopen( pszFilename, _T("wb") );
		if( !pFileStream ) {
			return FALSE;
		}
		bCloseFile = TRUE;
	}
	// write out our file
	fwrite( m_pCompiledFile, m_nCompiledBytes, 1, pFileStream );
	// close our file
	if( bCloseFile ) {
		fclose( pFileStream );
	}

	return TRUE;
}

void CConvertCamFile::FreeData() {
	if( m_pUncompressedMemAllocated ) {
		fang_Free( m_pUncompressedMemAllocated );
		m_pUncompressedMemAllocated = NULL;
	}
	m_nUncompressedBytesAllocated = 0;

	if( m_pWorkingMemory ) {
		fang_Free( m_pWorkingMemory );
		m_pWorkingMemory = NULL;
	}
	m_nWorkingBytesAllocated = NULL;

	m_nFKeyCount = 0;
	m_nTKeyCount = 0;
	m_nOKeyCount = 0;

	m_nTotalKeyTimes = 0;
	m_paKeyTimes = NULL;		// Array of times at which particular keyframes occur
	m_paFKeyTimeIdx = NULL;
	m_paTKeyTimeIdx = NULL;
	m_paOKeyTimeIdx = NULL;
	
	m_paFKeyData = NULL;
	m_paTKeyData = NULL;
	m_paOKeyData = NULL;

	if( m_pCompiledFile ) {
		fang_Free( m_pCompiledFile );
		m_pCompiledFile = NULL;
	}
	m_nCompiledBytes = 0;
}


BOOL CConvertCamFile::LoadMaxCamFile ( const CFileInfo *pFileInfo ) {
	FILE *pFile;
	CString sFilepath, sError;
	CErrorLog &rErrorLog = CErrorLog::GetCurrent();

	// gather some info about the file
	sFilepath = pFileInfo->GetFilePath();
	m_nUncompressedBytesAllocated = pFileInfo->GetLength();	


	// allocate memory
	m_pUncompressedMemAllocated = (u8 *)fang_MallocAndZero( m_nUncompressedBytesAllocated, 4 );
	if( !m_pUncompressedMemAllocated ) {
		FreeData();
		rErrorLog.WriteErrorHeader( _ERROR_HEADING + sFilepath );
		rErrorLog.WriteErrorLine( "Could not allocate memory for file image" );
		return FALSE;
	}

	// read the entire file into memory
	pFile = fopen( sFilepath, "rb" );
	if( !pFile ) {
		FreeData();
		rErrorLog.WriteErrorHeader( _ERROR_HEADING + sFilepath );
		rErrorLog.WriteErrorLine( "Could not open file for reading" );
		return FALSE;
	}
	if( fread( m_pUncompressedMemAllocated, 1, m_nUncompressedBytesAllocated, pFile ) != m_nUncompressedBytesAllocated ) {
		fclose( pFile );
		FreeData();
		rErrorLog.WriteErrorHeader( _ERROR_HEADING + sFilepath );
		rErrorLog.WriteErrorLine( "Trouble reading the file" );
		return FALSE;
	}
	fclose( pFile );

	//now, fixup our pointers
	m_pHeader = (CamInfo_t *) m_pUncompressedMemAllocated;
	m_paFrames = (CamFrame_t *) &m_pHeader[1];

	return TRUE;
}



BOOL CConvertCamFile::CompressCamFile( CString & sFilepath ) {

	CErrorLog &rErrorLog = CErrorLog::GetCurrent();

	//first, allocate all the memory we will need
	u32 nBytesToAllocate = ( sizeof( f32 ) * 2 + sizeof ( u16 ) * 3 + sizeof( CFVec3 ) + sizeof( CFQuat ) ) * m_pHeader->nFrames;

	m_nWorkingBytesAllocated = nBytesToAllocate;
	m_nWorkingBytesAllocated = FMATH_BYTE_ALIGN_UP( m_nWorkingBytesAllocated, 4 );
	m_pWorkingMemory = (u8 *)fang_MallocAndZero( m_nWorkingBytesAllocated, 4 );
	if( !m_pWorkingMemory ) {
		rErrorLog.WriteErrorHeader( _ERROR_HEADING + sFilepath );
		rErrorLog.WriteErrorLine( "Could not allocate memory for file image" );
		return FALSE;
	}

	//now, fixup our working pointers
    m_paKeyTimes = ( f32 *) m_pWorkingMemory;
	m_paFKeyTimeIdx = ( u16 *) & m_paKeyTimes[ m_pHeader->nFrames ];
	m_paTKeyTimeIdx = & m_paFKeyTimeIdx[ m_pHeader->nFrames ];
	m_paOKeyTimeIdx = & m_paTKeyTimeIdx[ m_pHeader->nFrames ];

	m_paFKeyData = ( f32 *) & m_paOKeyTimeIdx[ m_pHeader->nFrames ];
	m_paTKeyData = ( CFVec3 *) & m_paFKeyData[ m_pHeader->nFrames ];
	m_paOKeyData = ( CFQuat *) & m_paTKeyData[ m_pHeader->nFrames ];

	//now, we have the internal memory variables all set up, lets
	//start running through the animation and discarding unneeded keyframes
	//on a per component variable basis

	u32 i;
	f32 fPreviousFOV, fCurrentFOV, fNextFOV;
	CFVec3 vPreviousCamPos, vCurrentCamPos, vNextCamPos;
	CFQuat qPreviousCamOrient, qCurrentCamOrient, qNextCamOrient;

	f32 fPreviousFTime;
	f32 fPreviousTTime;
	f32 fPreviousOTime;

	for( i=0; i<m_pHeader->nFrames; i++ ) {
		BOOL bExportFKey = !m_bCompressAnim;
		BOOL bExportTKey = !m_bCompressAnim;
		BOOL bExportOKey = !m_bCompressAnim;
		f32 fCurrentTime = m_paFrames[ i ].fSecsFromStart;
		
		fCurrentFOV = m_paFrames[ i ].fFOV;
		vCurrentCamPos = m_paFrames[ i ].Orientation.m_vPos;
		qCurrentCamOrient.BuildQuat( m_paFrames[ i ].Orientation );

		if( ( i == 0 ) || ( i == ( m_pHeader->nFrames - 1 ) ) ) {
			//we always export the first and last frames of an animation
			bExportFKey = TRUE;
			bExportTKey = TRUE;
			bExportOKey = TRUE;
		}
		else{
			f32 fNextKeyframeTime = m_paFrames[ i + 1 ].fSecsFromStart;  

			//we need to check if we want to record this current keyframe...
			//check FOV
			fNextFOV = m_paFrames[ i + 1 ].fFOV;
			f32 fFOVInc = ( fNextFOV - fPreviousFOV ) / ( fNextKeyframeTime - fPreviousFTime );

			f32 fPreviousToCurrentFTime = fCurrentTime - fPreviousFTime;
			f32 fInterpolatedFOV = fPreviousFOV + fFOVInc * fPreviousToCurrentFTime;
			if( FMATH_FABS( fInterpolatedFOV - fCurrentFOV ) > 0.001 ) {
				//too significant of a difference
				bExportFKey = TRUE;
			}

			//check Translation
			vNextCamPos = m_paFrames[ i + 1 ].Orientation.m_vPos;

			f32 fSlider = ( fCurrentTime - fPreviousTTime ) / ( fNextKeyframeTime - fPreviousTTime );
			CFVec3 vInterpolatedCamPos;
			vInterpolatedCamPos.ReceiveLerpOf( fSlider, vPreviousCamPos, vNextCamPos );

			CFVec3 vDiff;
			vDiff = vInterpolatedCamPos - vCurrentCamPos;
			if ( vDiff.Mag() > 0.001f )
			{
				bExportTKey = TRUE;
			}

			//check orientation
			qNextCamOrient.BuildQuat( m_paFrames[ i + 1 ].Orientation );
			fSlider = ( fCurrentTime - fPreviousOTime ) / ( fNextKeyframeTime - fPreviousOTime );

			//now, build a quaternian interpolated between the Previous and next orientations
			//and compare to the current quaternian...
			CFQuat qInterpolatedOrient;
			qInterpolatedOrient.ReceiveSlerpOf(fSlider, qPreviousCamOrient, qNextCamOrient);

			//now, compare the quaternians and see if they are common...
			f32 fDot = (qInterpolatedOrient.x * qCurrentCamOrient.x) + (qInterpolatedOrient.y * qCurrentCamOrient.y)  
						+ (qInterpolatedOrient.z * qCurrentCamOrient.z) + (qInterpolatedOrient.w * qCurrentCamOrient.w);
			if ( fDot < 0.9999f )
			{
				bExportOKey = TRUE;
			}
		}

		if( bExportFKey ) {
			m_paFKeyTimeIdx[ m_nFKeyCount ] = m_nTotalKeyTimes;
			m_paFKeyData[ m_nFKeyCount ] = fCurrentFOV;
			m_nFKeyCount++;
			fPreviousFTime = fCurrentTime;
			fPreviousFOV = fCurrentFOV;
		}
		if( bExportTKey ) {
			m_paTKeyTimeIdx[ m_nTKeyCount ] = m_nTotalKeyTimes;
			m_paTKeyData[ m_nTKeyCount ] = vCurrentCamPos;
			m_nTKeyCount++;
			fPreviousTTime = fCurrentTime;
			vPreviousCamPos = vCurrentCamPos;
		}
		if( bExportOKey ) {
			m_paOKeyTimeIdx[ m_nOKeyCount ] = m_nTotalKeyTimes;
			m_paOKeyData[ m_nOKeyCount ] = qCurrentCamOrient;
			m_nOKeyCount++;
			fPreviousOTime = fCurrentTime;
			qPreviousCamOrient = qCurrentCamOrient;
		}

		if( bExportFKey || bExportTKey || bExportOKey ) {
			m_paKeyTimes[ m_nTotalKeyTimes ] = fCurrentTime;
			m_nTotalKeyTimes++;
		}
	}

	return TRUE;
}
