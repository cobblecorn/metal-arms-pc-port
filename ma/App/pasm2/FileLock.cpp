//////////////////////////////////////////////////////////////////////////////////////
// FileLock.cpp - 
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
// 11/21/01 Starich     Created.
//////////////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "fang.h"
#include "FileLock.h"
#include "progresswnd.h"


#define _BUFFER_SIZE	( 4 * (MAX_COMPUTERNAME_LENGTH + 1) )
#define _UNKNOWN_USER	"??? Unknown ???"

static char _szBuffer[_BUFFER_SIZE];
 

CFileLock::CFileLock( cchar *pszFilename/*=NULL*/ ) {
	m_bFileLocked = FALSE;
	SetFilename( pszFilename );
	
	DWORD nLen = _BUFFER_SIZE;
	if( GetComputerName( _szBuffer, &nLen ) ) {
		m_sComputerName = _szBuffer;
	} else {
		m_sComputerName = _UNKNOWN_USER;
	}		
}

CFileLock::~CFileLock() {
	UnlockFile();
}

BOOL CFileLock::SetFilename( cchar *pszFilename ) {
	if( m_bFileLocked ) {
		// can't change the filename while we are locked
		return FALSE;
	}

	if( !pszFilename ) {
		m_sFilename.Empty();
	} else {
		m_sFilename = pszFilename;
	}
	return TRUE;
}

// will try to lock the file, will will not wait
// returns TRUE if the file is locked, 
// FALSE if it is not
BOOL CFileLock::LockFile() {
	if( m_bFileLocked ) {
		return TRUE;
	}
	if( m_sFilename.IsEmpty() ) {
		return FALSE;
	}
	if( !m_File.Open( m_sFilename, CFile::modeCreate | 
								   CFile::modeWrite |
								   CFile::shareDenyWrite |
								   CFile::typeBinary ) ) {
		return FALSE;
	}
	m_File.Write( m_sComputerName, m_sComputerName.GetLength() );
	char szNull[4];
	fang_MemZero( szNull, 4 );
	m_File.Write( (void *)szNull, 4 );
	m_File.Flush();

	m_bFileLocked = TRUE;

	return TRUE;
}

#define _PROGRESS_RANGE	10000

// will attempt to lock the file, if not available this 
// function will display a progress bar and will wait till 
// the file can be locked or until the user cancels the wait.
// returns TRUE if the file is locked, 
// FALSE if it is not lock
BOOL CFileLock::WaitToLockFile( CWnd* pParent ) {

	if( LockFile() ) {
		return TRUE;
	}
	// wait for the file to be available for locking
	BOOL bGetNameOfLockedUser = TRUE;
	CString sWhoHasFileLocked;
	sWhoHasFileLocked = _UNKNOWN_USER;
	CProgressWnd WndProgress( pParent, "PASM: Waiting To Lock Library", TRUE, FALSE );
	WndProgress.SetRange( 0,_PROGRESS_RANGE );
	WndProgress.SetText( "Trying to lock the library...\n"
						 "'%s' currently has the library locked.\n\n"
						 "Click Cancel to quit waiting.", sWhoHasFileLocked );
	u32 nTotalTicksWaited = 0, nTicksBefore, nTicksAfter, nLoops = 0, nMins = 0, nSecs = 0, nCSecs = 0;

	while( TRUE ) {
		// grab the time
		nTicksBefore = GetTickCount();
		// increment our loop counter, so that we know if we should reacquire the person with the lock
		nLoops++;
		if( (nLoops % 5) == 1 ) {
			bGetNameOfLockedUser = TRUE;
		}
		// get the name of the person who currently has the library locked
		if( bGetNameOfLockedUser ) {
			if( WhoHasFileLocked( sWhoHasFileLocked ) ) {
				// we got the name of the person who has the file locked				
				bGetNameOfLockedUser = FALSE;
				
				// go ahead and update the user name in the progress dialog's text
				WndProgress.SetText( "Trying to lock the library...\n"
									 "'%s' currently has the library locked.\n"
									 "Wait time (Min:Sec) = %02d:%02d.%02d.\n"
									 "Click Cancel to quit waiting.", sWhoHasFileLocked, nMins, nSecs, nCSecs );
			}
		}
		// waste some time...
		for( int i = 0; i < _PROGRESS_RANGE; i++ ) {
			if( WndProgress.Cancelled() ) {
				// the user go tired of waiting
				return FALSE;
			}

			WndProgress.SetPos( i );
			
			WndProgress.PeekAndPump( FALSE );			
		}
		// try to lock the file
		if( LockFile() ) {
			return TRUE;
		}
		// grab the time
		nTicksAfter = GetTickCount();
		nTotalTicksWaited += (nTicksAfter - nTicksBefore);

		nMins = nTotalTicksWaited / (60 * 1000);
		nSecs = (nTotalTicksWaited / 1000) - (nMins * 60);
		nCSecs = (nTotalTicksWaited / 10) - (nMins * 60 * 100) - (nSecs * 100);

		// update the progress dialog's text
		WndProgress.SetText( "Trying to lock the library...\n"
							 "'%s' currently has the library locked.\n"
							 "Wait time (Min:Sec) = %02d:%02d.%02d.\n"	
							 "Click Cancel to quit waiting.", sWhoHasFileLocked, nMins, nSecs, nCSecs );
	}	
	
	return TRUE;
}

BOOL CFileLock::IsFileLocked() {
	if( m_bFileLocked ) {
		// the file is locked by us
		return TRUE;
	}
	// attempt to lock the file
	if( LockFile() ) {
		// nobody has the file locked, we were able to lock it
		UnlockFile();
		return FALSE;
	}
	return TRUE;
}

// returns TRUE rsName was filled in to the person who has the file locked
// returns FALSE if the file is not locked
BOOL CFileLock::WhoHasFileLocked( CString &rsName ) {

	if( m_bFileLocked ) {
		// we have the file locked
		rsName = m_sComputerName;
		return TRUE;
	}
	if( !IsFileLocked() ) {
		// the file is not currently locked
		return FALSE;		
	}
	// see who has the file locked
	if( !m_File.Open( m_sFilename, CFile::modeRead |
								   CFile::typeBinary |
								   CFile::modeNoTruncate |
								   CFile::shareDenyNone ) ) {
		// could not open the file for reading
		return FALSE;
	}
	u32 nFileLen = (u32)m_File.GetLength();
	if( nFileLen > _BUFFER_SIZE ) {
		nFileLen = _BUFFER_SIZE;
	}
	if( m_File.Read( _szBuffer, nFileLen ) != nFileLen ) {
		m_File.Close();
		return FALSE;
	}
	m_File.Close();

	rsName = _szBuffer;
	
	return TRUE;
}

void CFileLock::UnlockFile() {
	if( m_bFileLocked ) {
		m_File.Close();
		m_bFileLocked = FALSE;
	}
}