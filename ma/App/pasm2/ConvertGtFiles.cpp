//////////////////////////////////////////////////////////////////////////////////////
// ConvertGtFiles.cpp - 
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
// 02/25/02 Starich     Created.
//////////////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "fang.h"
#include "ConvertGtFiles.h"
#include "ErrorLog.h"
#include "flinklist.h"
#include "../../lib/AIGraphConvert/AIGraphConvert.h"


//====================
// private definitions

#define _ERROR_HEADING				".GT FILE COMPILER "

//=================
// public variables


//==================
// private variables

//===================
// private prototypes

//=================
// public functions

CConvertGtFile::CConvertGtFile() 
	: m_uGraphHandle( AIGRAPHCONVERT_INVALIDHANDLE ) {
}

CConvertGtFile::~CConvertGtFile() {
	FreeData();
}

BOOL CConvertGtFile::ConvertFile( const CFileInfo *pFileInfo, BOOL bConvertToBigEndian/*=FALSE*/ ) {
	
	FreeData();

	if( !pFileInfo ) {
		return FALSE;
	}

	CErrorLog &rErrorLog = CErrorLog::GetCurrent();

	m_uGraphHandle = aigraphconvert_fnAIGraphConvertAsciiToBinary(pFileInfo->GetFilePath());

	if( m_uGraphHandle == AIGRAPHCONVERT_INVALIDHANDLE ) {
		FreeData();
		rErrorLog.WriteErrorHeader( _ERROR_HEADING + pFileInfo->GetFilePath() );
		rErrorLog.WriteErrorLine( "Trouble loading ascii graph file" );
		return FALSE;
	}	
	
	if( bConvertToBigEndian ) {
		aigraphconvert_fnAIGraphEndianFlip( m_uGraphHandle );
	}

	return TRUE;
}

u32 CConvertGtFile::GetSizeOfConvertedFile() {

	if( m_uGraphHandle == AIGRAPHCONVERT_INVALIDHANDLE) {
		return 0;
	}

	return aigraphconvert_fnAIGraphCalcConvertedFileSizeBytes( m_uGraphHandle );
}

BOOL CConvertGtFile::WriteConvertedFile( cchar *pszFilename, FILE *pFileStream/*=NULL*/ ) {

	if( m_uGraphHandle == AIGRAPHCONVERT_INVALIDHANDLE) {
		return FALSE;
	}
	return aigraphconvert_fnAIGraphSaveToBuffer( m_uGraphHandle, pszFilename, pFileStream );
}

void CConvertGtFile::FreeData() {

	if( m_uGraphHandle != AIGRAPHCONVERT_INVALIDHANDLE ) {
		aigraphconvert_fvAIGraphFreeData( m_uGraphHandle );
		m_uGraphHandle = AIGRAPHCONVERT_INVALIDHANDLE;
	}

}

//==================
// private functions



