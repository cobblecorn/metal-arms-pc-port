//////////////////////////////////////////////////////////////////////////////////////
// SeqBank.cpp - 
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
// 06/18/02 Starich     Created.
//////////////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "fang.h"
#include "SeqBank.h"
#include "fclib.h"

#define _NUM_CMDS_TO_ALLOCATE	8

CSeqBank::CSeqBank() {
	
	fang_MemZero( &m_Header, sizeof( SeqBank_Header_t ) );
	m_apFx.RemoveAll();

	// go ahead and create an empty file by default
	CreateEmpty();

	SeqBank_Cmd_t Cmd;
	Cmd.nCmdIndex = 0;
	FASSERT( sizeof( Cmd.TypeData ) == SEQ_BANK_LARGEST_CMD_BYTES );
}

CSeqBank::~CSeqBank() {
	FreeData();
}

BOOL CSeqBank::SetFileSettings( SeqBank_ToolSettings_t *pSettings ) {

	// simply copy the new settings over the old ones
	fang_MemCopy( &m_Header.Settings, pSettings, sizeof( SeqBank_ToolSettings_t ) );

	return TRUE;
}

SeqBank_ToolSettings_t *CSeqBank::GetFileSettings() {
	return &m_Header.Settings;
}

BOOL CSeqBank::CreateEmpty() {

	// free any data that may be contained
	FreeData();
	
	// a few fields need to be setup in the header
	m_Header.nSignature = FVERSION_FILE_SIGNATURE;
	m_Header.nFileVersion = SEQ_BANK_VERSION_NUM;
	m_Header.nNumBytes = sizeof( SeqBank_Header_t );
	m_Header.nSizeOfEachCmd = SEQ_BANK_LARGEST_CMD_BYTES;
	
	// record the name of who created this file
	DWORD nLen = SEQ_BANK_STRING_LEN;
	if( !GetComputerName( m_Header.szCreatedBy, &nLen ) ) {
		fclib_strncpy( m_Header.szCreatedBy, "unknown user", SEQ_BANK_STRING_LEN );
	}
	return TRUE;
}

BOOL CSeqBank::IsEmpty() {

	if( m_Header.nNumFxs ) {
		return FALSE;
	}
	return TRUE;
}

u32 CSeqBank::GetBankSize() {
	return m_Header.nNumBytes;
}

BOOL CSeqBank::Read( cchar *pszFilename ) {

	CreateEmpty();

	if( !pszFilename || 
		fclib_strlen( pszFilename ) == 0 ) {
		return FALSE;
	}
	
	// open the file for reading
	FILE *pFileStream = fopen( pszFilename, "rb" );
	if( !pFileStream ) {
		return FALSE;
	}

	// read the header in
	if( fread( &m_Header, sizeof( SeqBank_Header_t ), 1, pFileStream ) != 1 ) {
		fclose( pFileStream );
		return FALSE;
	}

	// check the version and signature
	if( m_Header.nFileVersion != SEQ_BANK_VERSION_NUM ||
		m_Header.nSignature != FVERSION_FILE_SIGNATURE ||
		m_Header.nSizeOfEachCmd != SEQ_BANK_LARGEST_CMD_BYTES ) {
		fang_MemZero( &m_Header, sizeof( SeqBank_Header_t ) );
		return FALSE;
	}

	// check the filesize
	fseek( pFileStream, 0, SEEK_END );
	if( ftell( pFileStream ) != (int)m_Header.nNumBytes ) {
		// the filesize in the header does not match the actual filesize
		fclose( pFileStream );
		fang_MemZero( &m_Header, sizeof( SeqBank_Header_t ) );
		return FALSE;
	}
	fseek( pFileStream, sizeof( SeqBank_Header_t ), SEEK_SET );
	
	u32 i, nFileFxCount, nFileCmdCount, nNumCmds;
	SeqBank_Fx_t *pFx;

	if( m_Header.nNumFxs ) {
		nFileFxCount = m_Header.nNumFxs;
		nFileCmdCount = m_Header.nNumCmds;
		m_Header.nNumFxs = 0;
		m_Header.nNumCmds = 0;

		// read in each fx
		for( i=0; i < nFileFxCount; i++ ) {
			// allocate memory for an fx
			pFx = (SeqBank_Fx_t *)malloc( sizeof( SeqBank_Fx_t ) );
			if( !pFx ) {
				fclose( pFileStream );
				CreateEmpty();
				return FALSE;
			}

			// read in the fx
			if( fread( pFx, sizeof( SeqBank_Fx_t ), 1, pFileStream ) != 1 ) {
				free( pFx );
				fclose( pFileStream );
				CreateEmpty();
				return FALSE;
			}

			// go ahead and allocate memory to store the cmds (which we will read in just a bit)
			nNumCmds = pFx->nNumCmds + _NUM_CMDS_TO_ALLOCATE;
			pFx->paCmds = (SeqBank_Cmd_t *)malloc( nNumCmds * sizeof( SeqBank_Cmd_t ) );
			if( !pFx->paCmds ) {
				free( pFx );
				fclose( pFileStream );
				CreateEmpty();
				return FALSE;
			}

			pFx->nNumAllocatedCmds = nNumCmds;

			// add this entry to our entry list
			m_apFx.Add( pFx );
			m_Header.nNumFxs++;
		}

		FASSERT( m_Header.nNumFxs == nFileFxCount );

		// read in each cmd
		for( i=0; i < nFileCmdCount; i++ ) {
			pFx = (SeqBank_Fx_t *)m_apFx[i];

			if( pFx->nNumCmdBytes ) {
				if( fread( pFx->paCmds, pFx->nNumCmdBytes, 1, pFileStream ) != 1 ) {
					fclose( pFileStream );
					CreateEmpty();
					return FALSE;
				}				
			}
			m_Header.nNumCmds += pFx->nNumCmds;
		}

		FASSERT( m_Header.nNumCmds == nFileCmdCount );
	}

	// close the file
	fclose( pFileStream );

	return TRUE;
}

BOOL CSeqBank::Write( cchar *pszFilename ) {

	if( !pszFilename ) {
		return FALSE;
	}

	FILE *pFileStream = fopen( pszFilename, "wb" );
	if( !pFileStream ) {
		return FALSE;
	}
	
	// update our header
	CTime CurrentTime = CTime::GetCurrentTime();
	m_Header.nModifiedTime = (u32)CurrentTime.GetTime();
	DWORD nLen = SEQ_BANK_STRING_LEN;
	if( !GetComputerName( m_Header.szLastModifiedBy, &nLen ) ) {
		fclib_strncpy( m_Header.szLastModifiedBy, "unknown user", SEQ_BANK_STRING_LEN );
	}
	u32 nByteOffset = 0;

	// write out our header
	fwrite( &m_Header, sizeof( SeqBank_Header_t ), 1, pFileStream );
	nByteOffset = sizeof( SeqBank_Header_t );

	// write out each Fx
	nByteOffset += sizeof( SeqBank_Fx_t ) * m_Header.nNumFxs;
	u32 i, nNumAllocated;
	SeqBank_Cmd_t *pCmds;
	SeqBank_Fx_t *pFx;
	for( i=0; i < m_Header.nNumFxs; i++ ) {
		pFx = (SeqBank_Fx_t *)m_apFx[i];
		
		// fix pFx->nDataOffset
		pFx->nDataOffset = nByteOffset;
		nByteOffset += pFx->nNumCmdBytes;
		
		// cache the cmd ptr & the nNumAllcoated
		pCmds = pFx->paCmds;
		pFx->paCmds = NULL;
		nNumAllocated = pFx->nNumAllocatedCmds;
		pFx->nNumAllocatedCmds = 0;

		// write the entry to disk
		fwrite( pFx, sizeof( SeqBank_Fx_t ), 1, pFileStream );

		// restore the cmd ptr
		pFx->paCmds = pCmds;
		pFx->nNumAllocatedCmds = nNumAllocated;
	}

	// sanity check
	FASSERT( nByteOffset == m_Header.nNumBytes );
	
	// write out each Fx
	for( i=0; i < m_Header.nNumFxs; i++ ) {
		pFx = (SeqBank_Fx_t *)m_apFx[i];

		if( pFx->nNumCmdBytes ) {
			fwrite( pFx->paCmds, pFx->nNumCmdBytes, 1, pFileStream );
		}
	}

	// close the file
	fclose( pFileStream );

	return TRUE;
}

u32 CSeqBank::GetNumFxs() {
	return m_Header.nNumFxs;
}

u32 CSeqBank::GetNumCmds() {
	return m_Header.nNumCmds;
}

BOOL CSeqBank::DeleteFxByIndex( u32 nIndex ) {

	SeqBank_Fx_t *pFx = GetFxByIndex( nIndex );
	if( !pFx ) {
		return FALSE;
	}
	
	RemoveFx( pFx, nIndex );	

	return TRUE;
}

BOOL CSeqBank::DeleteFx( SeqBank_Fx_t *pFx ) {

	if( !pFx ) {
		return FALSE;
	}
	// find the index of pEntry
	u32 i;
	for( i=0; i < m_Header.nNumFxs; i++ ) {
		if( (u32)pFx == (u32)m_apFx[i] ) {
			break;
		}
	}
	if( i == m_Header.nNumFxs ) {
		return FALSE;
	}
	
	RemoveFx( pFx, i );

	return TRUE;
}

SeqBank_Fx_t *CSeqBank::GetFxByIndex( u32 nIndex ) {

	if( nIndex >= m_Header.nNumFxs ) {
		return NULL;
	}
	return (SeqBank_Fx_t *)m_apFx[nIndex];
}

// retrieves an entry by name
SeqBank_Fx_t *CSeqBank::GetFxByName( cchar *pszName ) {

	if( !pszName ) {
		return NULL;
	}
	// search our entry array for a match
	u32 i;
	SeqBank_Fx_t *pFx;
	for( i=0; i < m_Header.nNumFxs; i++ ) {
		pFx = (SeqBank_Fx_t *)m_apFx[i];
		if( fclib_stricmp( pFx->szFxName, pszName ) == 0 ) {
			// we match, return this entry
			return pFx;
		}
	}
	return NULL;
}

// will create a new Fx if one does not already exist,
// if one already exists with the same name, it will be returned
SeqBank_Fx_t *CSeqBank::AddFx( cchar *pszFxName ) {
	SeqBank_Fx_t *pFx;
	
	// see if an Fx named pszFxName already exists
	pFx = GetFxByName( pszFxName );
	if( pFx ) {
		return pFx;
	}
	
	if( !pszFxName ) {
		return NULL;
	}
	
	// allocate the Fx
	pFx = (SeqBank_Fx_t *)malloc( sizeof( SeqBank_Fx_t ) );
	if( !pFx ) {
		return NULL;
	}
	fang_MemZero( pFx, sizeof( SeqBank_Fx_t ) );

	// allocate some memory to hold some commands
	pFx->paCmds = (SeqBank_Cmd_t *)malloc( sizeof( SeqBank_Cmd_t ) * _NUM_CMDS_TO_ALLOCATE );
	if( !pFx->paCmds ) {
		free( pFx );
		return NULL;
	}
	fang_MemZero( pFx->paCmds, sizeof( SeqBank_Cmd_t ) * _NUM_CMDS_TO_ALLOCATE );

	// fill in some of the Fx fields
	fclib_strncpy( pFx->szFxName, pszFxName, SEQ_BANK_MAX_FX_NAME_LEN );
	pFx->nNumAllocatedCmds = _NUM_CMDS_TO_ALLOCATE;

	// finally add pFx to our fx list
	m_apFx.Add( pFx );

	// update the header
	m_Header.nNumBytes += sizeof( SeqBank_Fx_t );
	m_Header.nNumFxs++;

	return pFx;
}

SeqBank_Cmd_t *CSeqBank::AddNewCmd( SeqBank_Fx_t *pFx, SeqBank_CmdType_e nType ) {
	SeqBank_Cmd_t *pCmd;

	if( nType >= SEQ_BANK_CMD_TYPE_COUNT ) {
		return NULL;
	}

	if( pFx->nNumAllocatedCmds >= pFx->nNumCmds ) {
		// there are no extra cmds, allocate a new cmds array
		u32 nNumCmds;
		nNumCmds = pFx->nNumCmds + SEQ_BANK_LARGEST_CMD_BYTES;
		pCmd = (SeqBank_Cmd_t *)malloc( sizeof( SeqBank_Cmd_t ) * nNumCmds );
		if( !pCmd ) {
			return NULL;
		}
		fang_MemZero( pCmd, sizeof( SeqBank_Cmd_t ) * nNumCmds );

		// copy the old cmds into the new buffer
		if( pFx->nNumCmdBytes ) {
			fang_MemCopy( pCmd, pFx->paCmds, pFx->nNumCmdBytes );
		}

		// free the old cmd buffer
		free( pFx->paCmds );
		
		// update the fx
		pFx->paCmds = pCmd;
		pFx->nNumAllocatedCmds = nNumCmds;
	}

	// use one of our allocated cmd buffers
	pCmd = &pFx->paCmds[pFx->nNumCmds];
	
	// fill in some fields from the cmd
	pCmd->nCmdType = nType;
	pCmd->nCmdIndex = pFx->nNumCmds;
	fang_MemZero( &pCmd->TypeData.nPadding[0], SEQ_BANK_LARGEST_CMD_BYTES );

	// update the Fx
	pFx->nNumCmds++;
	pFx->nNumCmdBytes += sizeof( SeqBank_Cmd_t );
	
	// update the header
	m_Header.nNumBytes += sizeof( SeqBank_Cmd_t );
	m_Header.nNumCmds++;
	
	return pCmd;
}

BOOL CSeqBank::DeleteCmdByIndex( SeqBank_Fx_t *pFx, u32 nIndex ) {

	if( nIndex >= pFx->nNumCmds ) {
		return FALSE;
	}
	SeqBank_Cmd_t *pDestCmd = (SeqBank_Cmd_t *)&pFx->paCmds[nIndex];
	SeqBank_Cmd_t *pSrcCmd = (SeqBank_Cmd_t *)&pFx->paCmds[nIndex+1];

	// move all of the cmds up
	u32 i;
	for( i = (pDestCmd->nCmdIndex + 1); i < pFx->nNumCmds; i++ ) {
		// copy the cmd up a spot
		fang_MemCopy( pDestCmd, pSrcCmd, sizeof( SeqBank_Cmd_t ) );

		pDestCmd->nCmdIndex--;

		pDestCmd++;
		pSrcCmd++;
	}

	// remove the last command from the Fx
	pFx->nNumCmdBytes -= sizeof( SeqBank_Cmd_t );
	pFx->nNumCmds--;

	// update the header
	m_Header.nNumBytes -= sizeof( SeqBank_Cmd_t );
	m_Header.nNumCmds--;

	return TRUE;
}

BOOL CSeqBank::DeleteCmd( SeqBank_Fx_t *pFx, SeqBank_Cmd_t *pCmd ) {

	return DeleteCmdByIndex( pFx, pCmd->nCmdIndex );
}

void CSeqBank::FreeData() {

	while( m_Header.nNumFxs ) {
		DeleteFxByIndex( 0 );
	}
	m_apFx.RemoveAll();
	fang_MemZero( &m_Header, sizeof( SeqBank_Header_t ) );
}

void CSeqBank::RemoveFx( SeqBank_Fx_t *pFx, u32 nIndex ) {

	// free the cmd memory
	if( pFx->paCmds ) {
		free( pFx->paCmds );
		pFx->paCmds = NULL;
	}

	// update our header vars
	m_Header.nNumCmds -= pFx->nNumCmds;
	m_Header.nNumFxs--;
	m_Header.nNumBytes -= pFx->nNumCmdBytes;
	m_Header.nNumBytes -= sizeof( SeqBank_Fx_t );

	// free the fx memory
	free( pFx );
	m_apFx.RemoveAt( nIndex, 1 );
}

