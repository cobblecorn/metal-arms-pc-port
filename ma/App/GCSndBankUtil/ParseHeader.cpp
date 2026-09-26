//////////////////////////////////////////////////////////////////////////////////////
// ParseHeader.cpp - 
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
// 06/13/02 Starich     Created.
//////////////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "fang.h"
#include "ParseHeader.h"
#include "fclib.h"

CParseHeader::CParseHeader() {
	Reset();	
}

CParseHeader::~CParseHeader() {
}

void CParseHeader::Reset() {
	m_nNumGRPs = 0;
	m_asGRPs.RemoveAll();
	m_anGRPs.RemoveAll();

	m_nNumSFXs = 0;
	m_asSFXs.RemoveAll();
	m_anSFXs.RemoveAll();
}

BOOL CParseHeader::Parse( const CString &rsFilename ) {
	CString sLine, sLeft, sValue;
	int nDefineIndex, nLen, nMinLen, nTabIndex, nTypeIndex;
	u32 nValue;
	char *pEnd;

	Reset();

	nMinLen = fclib_strlen( "#define" );

	try {
		CFile *pFile = new CFile( rsFilename, CFile::modeRead | CFile::shareDenyNone );
		if( pFile ) {
			CArchive *pArchive = new CArchive( pFile, CArchive::load );
			if( pArchive ) {
				// Read and store all rows in memory
				while( pArchive->ReadString( sLine ) ) {
					if( !sLine.IsEmpty() ) {
						// sLine is not empty, see if it has "#define"
						nDefineIndex = sLine.Find( "#define" );
						nLen = sLine.GetLength();
						if( nDefineIndex >= 0 && (nDefineIndex + nMinLen) < nLen ) {
							// determine what type of line we have
							sLeft = sLine.Right( nLen - (nDefineIndex + nMinLen) );
							sLeft.TrimLeft();
							sLeft.TrimRight();
							nLen = sLeft.GetLength();
							// find the value part of the string
							nTabIndex = sLeft.ReverseFind( '\t' );
							if( nTabIndex <= 0 ) {
								// search for a space instead
								nTabIndex = sLeft.ReverseFind( ' ' );
								if( nTabIndex <= 0 ) {
									continue;
								}
							}
							sValue = sLeft.Mid( nTabIndex );
							sLeft.Delete( nTabIndex, nLen - nTabIndex );
							sValue.TrimLeft();
							sLeft.TrimRight();
							sValue.MakeLower();
							sLeft.MakeLower();

							nValue = strtol( sValue, &pEnd, 10 );

							// see if we have a group or sfx
							nTypeIndex = sLeft.Find( "grp" );
							if( nTypeIndex >= 0 ) {
								sLeft.Delete( 0, nTypeIndex + 3 );

								m_nNumGRPs++;
								m_anGRPs.Add( nValue );
								m_asGRPs.AddTail( sLeft );
							} else {
								nTypeIndex = sLeft.Find( "sfx" );
								if( nTypeIndex >= 0 ) {
									sLeft.Delete( 0, nTypeIndex + 3 );

									m_nNumSFXs++;
									m_anSFXs.Add( nValue );
									m_asSFXs.AddTail( sLeft );
								}
							}							
						}
					}
				}
				delete pArchive;
				delete pFile;
#if 0
				DEVPRINTF( "Found %d GRPs\n", m_nNumGRPs );
				POSITION Pos;
				u32 i;
				for( i=0; i < m_nNumGRPs; i++ ) {
					Pos = m_asGRPs.FindIndex( i );
					sLeft = m_asGRPs.GetAt( Pos );
					DEVPRINTF( "GRP[%d] = '%s' = %d\n", i, sLeft, m_anGRPs[i] );
				}
				
				DEVPRINTF( "Found %d SFXs\n", m_nNumSFXs );
				for( i=0; i < m_nNumSFXs; i++ ) {
					Pos = m_asSFXs.FindIndex( i );
					sLeft = m_asSFXs.GetAt( Pos );
					DEVPRINTF( "SFX[%d] = '%s' = %d\n", i, sLeft, m_anSFXs[i] );
				}
#endif			
				return TRUE;
			}
			delete pFile;
		}
	}
	catch(...) {
	}

	return FALSE;
}

s32 CParseHeader::FindGroupIndex( cchar *pszGroupName ) {

	if( !m_nNumGRPs ) {
		return -1;
	}
	
	u32 i;
	CString sName;
	POSITION Pos;
				
	for( i=0; i < m_nNumGRPs; i++ ) {
		Pos = m_asGRPs.FindIndex( i );
		sName = m_asGRPs.GetAt( Pos );

		if( sName.CompareNoCase( pszGroupName ) == 0 ) {
			return m_anGRPs[i];
		}
	}
	return -1;
}

s32 CParseHeader::FindSfxIndex( cchar *pszSfxName ) {
	
	if( !m_nNumSFXs ) {
		return -1;
	}
	
	u32 i;
	CString sName;
	POSITION Pos;
				
	for( i=0; i < m_nNumSFXs; i++ ) {
		Pos = m_asSFXs.FindIndex( i );
		sName = m_asSFXs.GetAt( Pos );

		if( sName.CompareNoCase( pszSfxName ) == 0 ) {
			return m_anSFXs[i];
		}
	}
	return -1;
}