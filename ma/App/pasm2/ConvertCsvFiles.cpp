//////////////////////////////////////////////////////////////////////////////////////
// ConvertCsvFiles.cpp - 
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
// 04/15/02 Starich     Created.
//////////////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "fang.h"
#include "ConvertCsvFiles.h"
#include "ErrorLog.h"
#include "fmath.h"
#include "fdata.h"
#include "fclib.h"
#include "CSVFile.h"

#define _ERROR_HEADING				".CSV FILE COMPILER " 



CConvertCsvFile::CConvertCsvFile() {
	m_nBytesAllocated = 0;
	m_pMemAllocated = NULL;
	m_pCompiledFile = NULL;
	m_pWorkingMem = NULL;
	m_nCompiledBytes = 0;
	m_nNumTables = 0;
	m_nNumFields = 0;
	m_nNumStringFields = 0;
	m_nNumWideStringFields = 0;
	m_nNumFloatFields = 0;
	m_nNumRows = 0;
	m_nNumCols = 0;
	m_paRowData = NULL;
	m_anFieldsPerTable.RemoveAll();
}

CConvertCsvFile::~CConvertCsvFile() {
	FreeData();
}

BOOL CConvertCsvFile::ConvertFile( const CFileInfo *pFileInfo, BOOL bConvertToBigEndian/*=FALSE*/ ) {
	CString sFilepath, sError;
	u32 i, nStringBytesNeeded, nWideStringBytesNeeded, nFieldsInCurrentTable, nMaxFieldsPerTable;
	s32 j, nNumFields, nFirstFieldToRemove, nNumToRemove;
	BOOL bNeedValue;
	
	FreeData();

	if( !pFileInfo ) {
		return FALSE;
	}

	CErrorLog &rErrorLog = CErrorLog::GetCurrent();

	// gather some info about the file
	sFilepath = pFileInfo->GetFilePath();

	////////////////////
	// read our csv file
	CCSVFile CsvFile( sFilepath, FALSE );
	m_nNumRows = CsvFile.GetTotalRows();
	m_nNumCols = CsvFile.GetTotalColumns();

	if( m_nNumRows <= 0 || m_nNumCols < 2 ) {
		rErrorLog.WriteErrorHeader( _ERROR_HEADING + sFilepath );
		rErrorLog.WriteErrorLine( "There must be at least 2 rows of data, at least 1 of them having 2 or more columns" );
		return FALSE;
	}

	//////////////////////////////////////////////////
	// allocate memory to hold all of the rows of data
	m_paRowData = new CArray<CStringW, CStringW &>[m_nNumRows];
	if( !m_paRowData ) {
		rErrorLog.WriteErrorHeader( _ERROR_HEADING + sFilepath );
		rErrorLog.WriteErrorLine( "Could not allocated memory to hold all of the rows of data" );
		return FALSE;
	}

	/////////////////////////////////////////////////////////////
	// read all of the rows of data (removing any comment fields)
	for( i=0; i < m_nNumRows; i++ ) {
		nFirstFieldToRemove = 0;
		nNumToRemove = 0;

		if( CsvFile.ReadRow( m_paRowData[i], i+1 ) ) {
			nNumFields = m_paRowData[i].GetSize();
			FASSERT( nNumFields > 0 );
			
			// search for comment fields, if we find one, empty all strings after it
			for( j=0; j < nNumFields; j++ ) {
				if( IsCommentFieldW( m_paRowData[i][j] ) ) {
					// the j'th field is a comment, empty it and all strings after it on this row
					do {
						m_paRowData[i][j].Empty();
						if( j > 0 ) {
							// flag this field for removal
							if( !nFirstFieldToRemove ) {
								nFirstFieldToRemove = j;
							}
							nNumToRemove++;
						}
						j++;
					} while( j < nNumFields );

					// now that we emptied the strings, remove all un-needed fields
					m_paRowData[i].RemoveAt( nFirstFieldToRemove, nNumToRemove );	
					break;
				} else {
					m_paRowData[i][j].TrimLeft();
					m_paRowData[i][j].TrimRight();
				}
			}

			// now that we've removed comments, see if we have a valid keystring name
			if( DoesKeyStringContainInvalidCharsW( m_paRowData[i][0], TRUE ) ) {
				rErrorLog.WriteErrorHeader( _ERROR_HEADING + sFilepath );
				sError.Format( "The keystring on row %d contains invalid characters, check the file and re-pasm.", i );
				rErrorLog.WriteErrorLine( sError );
				return FALSE;
			}
		} else {
			rErrorLog.WriteErrorHeader( _ERROR_HEADING + sFilepath );
			sError.Format( "Error reading row %d, check the file and re-pasm.", i );
			rErrorLog.WriteErrorLine( sError );
			return FALSE;
		}
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////
	// check for table validity and count the number of fields (also turn all string fields into lowercase)
	m_nNumTables = 0;
	m_nNumFields = 0;
	m_nNumStringFields = 0;
	m_nNumWideStringFields = 0;
	m_nNumFloatFields = 0;
	bNeedValue = FALSE;
	nStringBytesNeeded = 0;
	nWideStringBytesNeeded = 0;
	m_anFieldsPerTable.RemoveAll();
	nFieldsInCurrentTable = 0;
	nMaxFieldsPerTable = 0;
	for( i=0; i < m_nNumRows; i++ ) {
		if( !m_paRowData[i][0].IsEmpty() ) {
			// make sure that we don't currently need a value for the last table
			if( bNeedValue ) {
				rErrorLog.WriteErrorHeader( _ERROR_HEADING + sFilepath );
				sError.Format( "Every table must have at least 1 value, table %d does not contain any valid values, check the file and re-pasm.", m_nNumTables );
				rErrorLog.WriteErrorLine( sError );
				return FALSE;
			}
			// record how many fields were contained in that last table
			if( m_nNumFields ) {
				m_anFieldsPerTable.Add( nFieldsInCurrentTable );

				if( nFieldsInCurrentTable > nMaxFieldsPerTable ) {
					nMaxFieldsPerTable = nFieldsInCurrentTable;
				}
			}
			// start a new table
			m_nNumTables++;
			nStringBytesNeeded += m_paRowData[i][0].GetLength() + 1;
			m_paRowData[i][0].MakeLower();
			bNeedValue = TRUE;
			nFieldsInCurrentTable = 0;
		}
		// find all of the value strings for the current table
		for( j=1; j < m_paRowData[i].GetSize(); j++ ) {
			if( !m_paRowData[i][j].IsEmpty() ) {
				if( m_nNumTables == 0 ) {
					// we found a value string before we got a table keystring
					rErrorLog.WriteErrorHeader( _ERROR_HEADING + sFilepath );
					sError.Format( "Row %d contains values without first finding a table keystring name, check the file and re-pasm.", i );
					rErrorLog.WriteErrorLine( sError );
					return FALSE;
				}
				bNeedValue = FALSE;
				m_nNumFields++;
				nFieldsInCurrentTable++;
				// determine if we have a string or numeric field
				if( IsStringNumericW( m_paRowData[i][j] ) ) {
					m_nNumFloatFields++;
				} else if( IsStringWide( m_paRowData[i][j] ) ) { //subtract the '|' front the string length
					m_nNumWideStringFields++;
					nWideStringBytesNeeded += ( ( m_paRowData[i][j].GetLength() ) * sizeof( wchar_t ) );
				} else {
					m_nNumStringFields++;
					nStringBytesNeeded += m_paRowData[i][j].GetLength() + 1;
					//m_paRowData[i][j].MakeLower();
				}
			}
		}
	}

	if( m_nNumTables == 0 || m_nNumFields == 0 ) {
		rErrorLog.WriteErrorHeader( _ERROR_HEADING + sFilepath );
		rErrorLog.WriteErrorLine( "Could not find any tables or fields, nothing to convert." );
		return FALSE;
	}

	if( bNeedValue ) {
		rErrorLog.WriteErrorHeader( _ERROR_HEADING + sFilepath );
		rErrorLog.WriteErrorLine( "The last table did not contain any valid fields, tables must have at least 1 field." );
		return FALSE;
	}

	///////////////////////////////////////////////////////////
	// record how many fields were contained in that last table
	if( m_nNumFields ) {
		m_anFieldsPerTable.Add( nFieldsInCurrentTable );

		if( nFieldsInCurrentTable > nMaxFieldsPerTable ) {
			nMaxFieldsPerTable = nFieldsInCurrentTable;
		}
	}
	FASSERT( (int)m_nNumTables == m_anFieldsPerTable.GetSize() );

#if 0
	/////////////////////////////////////////////////////
	// make sure that we don't have duplicate table names
	for( i=0; i < m_nNumRows; i++ ) {
		if( !m_paRowData[i][0].IsEmpty() ) {
			for( j=i+1; j < (int)m_nNumRows; j++ ) {
				if( !m_paRowData[j][0].IsEmpty() ) {
					if( m_paRowData[i][0].CompareNoCase( m_paRowData[j][0] ) == 0 ) {
						// there are 2 identical table names
						rErrorLog.WriteErrorHeader( _ERROR_HEADING + sFilepath );
						sError.Format( "The table name '%s' is used more than once, all table names must be unique, check the file and re-pasm.", m_paRowData[j][0] );
						rErrorLog.WriteErrorLine( sError );
						return FALSE;
					}
				}
			}
		}
	}
#endif

	if( m_nNumTables > FDATA_GAMFILE_MAX_TABLES ) {
		rErrorLog.WriteErrorHeader( _ERROR_HEADING + sFilepath );
		sError.Format( "There are too many tables (%d) in one file, max = %d.\n", m_nNumTables, FDATA_GAMFILE_MAX_TABLES );
		rErrorLog.WriteErrorLine( sError );
		return FALSE;
	}
	if( nMaxFieldsPerTable > FDATA_GAMFILE_MAX_FIELDS_PER_TABLE ) {
		rErrorLog.WriteErrorHeader( _ERROR_HEADING + sFilepath );
		sError.Format( "There are too many fields (%d) in one table, max = %d.\n", nMaxFieldsPerTable, FDATA_GAMFILE_MAX_FIELDS_PER_TABLE );
		rErrorLog.WriteErrorLine( sError );
		return FALSE;
	}

	/////////////////////////////////
	// allocate memory for disk image
	m_nBytesAllocated = sizeof( FDataGamFile_Header_t ) + 
						( m_nNumTables * sizeof( FDataGamFile_Table_t ) ) + 
						( m_nNumFields * sizeof( FDataGamFile_Field_t ) ) +
						( nWideStringBytesNeeded  ) +  
						( nStringBytesNeeded << 2 );// this gives us a bit of fudge factor
	m_nBytesAllocated = FMATH_BYTE_ALIGN_UP( m_nBytesAllocated, 4096 );// this gives us a bit of fudge factor
	m_pMemAllocated = (u8 *)fang_MallocAndZero( m_nBytesAllocated, 4 );
	if( !m_pMemAllocated ) {
		rErrorLog.WriteErrorHeader( _ERROR_HEADING + sFilepath );
		rErrorLog.WriteErrorLine( "Could not allocate memory for file image" );
		return FALSE;
	}

	//////////////////////
	// fixup our pointers
	FDataGamFile_Header_t *pHeader;
	FDataGamFile_Table_t *pTable;
	FDataGamFile_Field_t *pField;
	m_pCompiledFile = m_pMemAllocated;
	m_nCompiledBytes = 0;
	pHeader = (FDataGamFile_Header_t *)m_pCompiledFile;
	pTable = (FDataGamFile_Table_t *)&pHeader[1];
	m_pWorkingMem = (u8 *)&pTable[m_nNumTables];

	pHeader->nFlags = FDATA_GAMFILE_FLAGS_NONE;
	pHeader->paTables = pTable;// the table array starts right after the header
	
	////////////////////////
	// create our file image
	for( i=0; i < m_nNumRows; i++ ) {
		if( !m_paRowData[i][0].IsEmpty() ) {
			// start a new table
			pTable = &pHeader->paTables[ pHeader->nNumTables ];
			nFieldsInCurrentTable = m_anFieldsPerTable.GetAt( pHeader->nNumTables );
			pTable->nTableIndex = (u16)pHeader->nNumTables;
			pHeader->nNumTables++;

			// set the table keystring and its length
			pTable->pszKeyString = (cchar *)m_pWorkingMem;
			CString sTempString = m_paRowData[i][0];
			fclib_strcpy( (char *)pTable->pszKeyString, sTempString );
			pTable->nKeyStringLen = fclib_strlen( pTable->pszKeyString );
			
			// advance our working memory past the string and NULL 
			m_pWorkingMem += (pTable->nKeyStringLen + 1);

			// set the table field array (make sure it is aligned by 4)
			m_pWorkingMem = (u8 *)FMATH_BYTE_ALIGN_UP( (u32)m_pWorkingMem, 4 );
			pTable->paFields = (FDataGamFile_Field_t *)m_pWorkingMem;
			m_pWorkingMem += nFieldsInCurrentTable * sizeof( FDataGamFile_Field_t );
		}
			
		// find all of the value strings that belong to the current table
		for( j=1; j < m_paRowData[i].GetSize(); j++ ) {
			if( !m_paRowData[i][j].IsEmpty() ) {
				pField = &pTable->paFields[ pTable->nNumFields ];
				pTable->nNumFields++;
				FASSERT( pTable->nNumFields <= nFieldsInCurrentTable );

				// determine if we have a string or numeric field
				if( IsStringNumericW( m_paRowData[i][j] ) ) {
					pField->nDataType = FDATA_GAMEFILE_DATA_TYPE_FLOAT;
					pField->fValue = ExtractFloatValueW( m_paRowData[i][j] );
				} else if( IsStringWide( m_paRowData[i][j] ) ) {
					pField->nDataType = FDATA_GAMEFILE_DATA_TYPE_WIDESTRING;
//ARG - >>>>>
#ifdef _MMI_TARGET_PS2
					m_pWorkingMem = (u8 *)FMATH_BYTE_ALIGN_UP( (u32)m_pWorkingMem, 2 );
#endif
//ARG - <<<<<
					pField->pwszValue = ( wchar_t *) m_pWorkingMem;
					CStringW sTempWString = m_paRowData[i][j];
					sTempWString.Remove( L'|' );
					sTempWString.Replace( L"\\n", L"\n" ); // (Try and replace newlines string combos with newline char)
					fclib_wcscpy( (u16*)pField->pwszValue, sTempWString );
					pField->nStringLen = fclib_wcslen( pField->pwszValue );
					m_pWorkingMem += ( ( pField->nStringLen + 1 ) * sizeof( wchar_t ) );						
				} else {
					pField->nDataType = FDATA_GAMEFILE_DATA_TYPE_STRING;
					pField->pszValue = (cchar *)m_pWorkingMem;
					CString sTempString = m_paRowData[i][j];
					fclib_strcpy( (char *)pField->pszValue, (cchar *)sTempString );
					pField->nStringLen = fclib_strlen( pField->pszValue );
					m_pWorkingMem += (pField->nStringLen + 1);						
				}
			}
		}		
	}

	m_nCompiledBytes = m_pWorkingMem - m_pCompiledFile;
	FASSERT( m_nCompiledBytes <= m_nBytesAllocated );
	pHeader->nBytesInFile = m_nCompiledBytes;
	FASSERT( pHeader->nNumTables == m_nNumTables );
	
	ConvertPtrsToOffsets( pHeader );

	if( bConvertToBigEndian ) {
		ConvertEndian( pHeader );	
	}

	return TRUE;
}

u32 CConvertCsvFile::GetDataCRC() {

	if( !m_pCompiledFile ) {
		return FALSE;
	}

	u32 nReturnCRC = fmath_Crc32( 0, (u8 *)m_pCompiledFile, m_nCompiledBytes );

	return nReturnCRC;
}

u32 CConvertCsvFile::GetSizeOfConvertedFile() {
	
	if( !m_nCompiledBytes ) {
		return 0;
	}
	return m_nCompiledBytes;
}

BOOL CConvertCsvFile::WriteConvertedFile( cchar *pszFilename, FILE *pFileStream/*=NULL*/ ) {

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

void CConvertCsvFile::FreeData() {
	if( m_pMemAllocated ) {
		fang_Free( m_pMemAllocated );
		m_pMemAllocated = NULL;
	}
	m_nBytesAllocated = 0;
	m_pCompiledFile = NULL;
	m_pWorkingMem = NULL;
	m_nCompiledBytes = 0;
	m_nNumTables = 0;
	m_nNumFields = 0;
	m_nNumStringFields = 0;
	m_nNumWideStringFields = 0;
	m_nNumFloatFields = 0;
	m_nNumRows = 0;
	m_nNumCols = 0;
	if( m_paRowData ) {
		delete [] m_paRowData;
		m_paRowData = NULL;
	}
	m_anFieldsPerTable.RemoveAll();
}

BOOL CConvertCsvFile::IsCommentField( const CString &rsField ) {
	CStringW sWString = rsField;
	return CConvertCsvFile::IsCommentFieldW( sWString );
}


BOOL CConvertCsvFile::IsCommentFieldW( const CStringW &rsField ) {
	
	if( rsField.IsEmpty() ) {
		return FALSE;
	}
	if( rsField.Find( L'#' ) >= 0 ) {
		return TRUE;
	}
	return FALSE;
}


BOOL CConvertCsvFile::DoesKeyStringContainInvalidChars( const CString &rsString, BOOL bAllowSpaces ) {
	CStringW sWString = rsString;
	return CConvertCsvFile::DoesKeyStringContainInvalidCharsW( sWString, bAllowSpaces );
}

BOOL CConvertCsvFile::DoesKeyStringContainInvalidCharsW( const CStringW &rsString, BOOL bAllowSpaces ) {

	if( !bAllowSpaces ) {
		return (rsString.FindOneOf( L"\" ()<>?+=*&^%$@!~`';:/.,{}[]\\#" ) >= 0);
	}
	return (rsString.FindOneOf( L"\"()<>?+=*&^%$@!~`';:/.,{}[]\\#" ) >= 0);
}

//This function checks to see if the first character in the string is a bar '|'.
//If it is, then this is a flag to store this string as a widechar string!
BOOL CConvertCsvFile::IsStringWide( const CStringW &rsString ) {
	return ( ( rsString[0] == L'|' ) ? TRUE : FALSE ); 
}

BOOL CConvertCsvFile::IsStringNumeric( const CString &rsString ) {
	CStringW sWString = rsString;
	return CConvertCsvFile::IsStringNumericW( sWString );
}

// see if there are any chars beside +-.0123456789, if so TRUE is return
BOOL CConvertCsvFile::IsStringNumericW( const CStringW &rsString ) {
	u32 i, nLen;
	wchar_t c;
	BOOL bDigitFound = FALSE;

	nLen = rsString.GetLength();
	if( !nLen ) {
		return FALSE;
	}
	for( i=0; i < nLen; i++ ) {
		c = rsString.GetAt( i );
		if( c != L'-' && c != L'+' && c != L'.' && (c < L'0' || c > L'9') ) {
			return FALSE;
		}
		if( c >= L'0' && c <= L'9' ) {
			// we found a digit
			bDigitFound = TRUE;
		}
	}
	return bDigitFound;
}

f32 CConvertCsvFile::ExtractFloatValue( const CString &rsString ) {
	CStringW sWString = rsString;
	return CConvertCsvFile::ExtractFloatValueW( sWString );
}


f32 CConvertCsvFile::ExtractFloatValueW( const CStringW &rsString ) {
	f32 fValue;
	wchar_t *pszEndString;

	fValue = (f32)wcstod( rsString, &pszEndString );

	return fValue;
}

void CConvertCsvFile::ConvertPtrsToOffsets( FDataGamFile_Header_t *pHeader ) {
	FDataGamFile_Table_t *pTable;
	FDataGamFile_Field_t *pField;
	u32 i, j, nBaseAddress;
	
	if( !pHeader ) {
		return;
	}

	nBaseAddress = (u32)pHeader;
	pTable = (FDataGamFile_Table_t *)pHeader->paTables;
	
	for( i=0; i < pHeader->nNumTables; i++ ) {
		for( j=0; j < pTable[i].nNumFields; j++ ) {
			pField = &pTable[i].paFields[j];
			if( pField->nDataType == FDATA_GAMEFILE_DATA_TYPE_STRING ) {
				// fix up this field's char *
				pField->pszValue -= nBaseAddress;
			}
			else if( pField->nDataType == FDATA_GAMEFILE_DATA_TYPE_WIDESTRING ) {
				// fix up this field's wchar_t *
				pField->pwszValue = (cwchar*) ( (u32)pField->pwszValue - nBaseAddress );
			}
		}
		// fix up this table's ptrs
		pTable[i].pszKeyString -= nBaseAddress;
		pTable[i].paFields = (FDataGamFile_Field_t *)( (u32)pTable[i].paFields - nBaseAddress );
	}
	// fix up the header's ptrs
	pHeader->paTables = (FDataGamFile_Table_t *)( (u32)pHeader->paTables - nBaseAddress );
}

void CConvertCsvFile::ConvertEndian( FDataGamFile_Header_t *pHeader ) {
	FDataGamFile_Table_t *pTable;
	FDataGamFile_Field_t *pField;
	u32 i, j, nBaseAddress;

	if( !pHeader ) {
		return;
	}
	
	nBaseAddress = (u32)pHeader;
	pTable = (FDataGamFile_Table_t *)( (u32)pHeader->paTables + nBaseAddress );

	for( i=0; i < pHeader->nNumTables; i++ ) {
		pField = (FDataGamFile_Field_t *)( (u32)pTable[i].paFields + nBaseAddress );
		for( j=0; j < pTable[i].nNumFields; j++ ) {
			pField[j].ChangeEndian( (u32)pHeader );
		}
		pTable[i].ChangeEndian();
	}

	// change the header
	pHeader->ChangeEndian();
}
