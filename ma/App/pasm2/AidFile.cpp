//////////////////////////////////////////////////////////////////////////////////////
// AidFile.cpp -
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
// 09/26/01 Starich     Created.
//////////////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "fang.h"
#include "AidFile.h"
#include "errorlog.h"
#include "fmath.h"
#include "ConvertCsvFiles.h"

#define _ERROR_HEADING				".AID FILE PARSER "


CAidFile::CAidFile() {
	m_nFileCRCValue = 0;
	m_bDataValid = FALSE;
}

CAidFile::~CAidFile() {
	m_bDataValid = FALSE;
}

// returns TRUE if there were at least 1 valid command and there were no errors,
// returns FALSE if no valid commands could be found or any errors exist
BOOL CAidFile::Load( const CFileInfo *pFileInfo ) {
	CString sFilepath, sLine, sLeft, sRight, sValue, sError;
	u32 nFileLen, nBytesAllocated, i, nNumValidLines, nErrorLines;
	u8 *pMemAllocated;
	FILE *pFile;
	char *pszTextFile, *pEOF, *pEndString;

	m_bDataValid = FALSE;

	if( !pFileInfo ) {
		return FALSE;
	}

	CErrorLog &rErrorLog = CErrorLog::GetCurrent();

	// gather some info about the file
	sFilepath = pFileInfo->GetFilePath();
	nFileLen = pFileInfo->GetLength();

	// allocate memory
	nBytesAllocated = nFileLen + 4;
	nBytesAllocated = FMATH_BYTE_ALIGN_UP( nBytesAllocated, 16 );
	pMemAllocated = new u8[nBytesAllocated];
	if( !pMemAllocated ) {
		rErrorLog.WriteErrorHeader( _ERROR_HEADING + sFilepath );
		rErrorLog.WriteErrorLine( "Could not allocate memory to hold aid file" );
		return FALSE;
	}

	// zero out our memory
	ZeroMemory( pMemAllocated, nBytesAllocated );

	// read the entire file into memory
	pFile = fopen( sFilepath, "rb" );
	if( !pFile ) {
		delete [] pMemAllocated;
		rErrorLog.WriteErrorHeader( _ERROR_HEADING + sFilepath );
		rErrorLog.WriteErrorLine( "Could not open file for reading" );
		return FALSE;
	}
	if( fread( pMemAllocated, 1, nFileLen, pFile ) != nFileLen ) {
		fclose( pFile );
		delete [] pMemAllocated;
		rErrorLog.WriteErrorHeader( _ERROR_HEADING + sFilepath );
		rErrorLog.WriteErrorLine( "Trouble reading the file" );
		return FALSE;
	}
	fclose( pFile );

	m_nFileCRCValue = fmath_Crc32( 0, (u8 *)pMemAllocated, nFileLen );

	// run through file and replace control characters with NULL (easier parsing)
	ReplaceControlCharsWithNULL( pMemAllocated, nFileLen );

	// reset all of our string and float values
	for( i=0; i < FDATA_MAX_MESH_PARTS; i++ ) {
		m_aPartStrings[i].bValueWasFound = FALSE;
		m_aPartStrings[i].sValue.Empty();
	}
	for( i=0; i < AID_FILE_STRINGS_COUNT; i++ ) {
		m_aStrings[i].bValueWasFound = FALSE;
		m_aStrings[i].sValue.Empty();
	}
	for( i=0; i < AID_FILE_FLOATS_COUNT; i++ ) {
		m_aNonStrings[i].bValueWasFound = FALSE;
		m_aNonStrings[i].nValue = 0;
	}

	// scan for errors and setup the various variables
	nNumValidLines = 0;
	nErrorLines = 0;
	pszTextFile = (char *)pMemAllocated;
	pEOF = &pszTextFile[nFileLen];
	while( pszTextFile ) {
		sLine = pszTextFile;
		// trim sLine of leading/trailing spaces
		sLine.TrimLeft();
		sLine.TrimRight();

		// see if the current line is a comment line
		if( !sLine.IsEmpty() && !IsCommentLine( sLine ) ) {

			// make the line lowercase
			sLine.MakeLower();

			// break the line into the left and right parts
			if( !SplitLineIntoLeftRight( sLine, sLeft, sRight ) ) {
				// could not break the line up into left and right parts
				if( !nErrorLines ) {
					rErrorLog.WriteErrorHeader( _ERROR_HEADING + sFilepath );
				}
				sError.Format( "The following line has no '=' sign: '%s'", sLine );
				rErrorLog.WriteErrorLine( sError );
				nErrorLines++;
				goto _ADVANCE_TO_NEXT_LINE;
			}

			// make sure that neither the left or right string is empty
			if( sLeft.IsEmpty() || sRight.IsEmpty() ) {
				// one of the 2 sides is empty
				if( !nErrorLines ) {
					rErrorLog.WriteErrorHeader( _ERROR_HEADING + sFilepath );
				}
				sError.Format( "The following line is not valid: '%s'", sLine );
				rErrorLog.WriteErrorLine( sError );
				nErrorLines++;
				goto _ADVANCE_TO_NEXT_LINE;
			}

			// make sure that the left string doesn't contain invalid characters
			if( DoesStringContainInvalidChars( sLeft, FALSE ) ) {
				// the keystring contains invalid characters
				if( !nErrorLines ) {
					rErrorLog.WriteErrorHeader( _ERROR_HEADING + sFilepath );
				}
				sError.Format( "The following line contains invalid characters in the keystring: '%s'", sLine );
				rErrorLog.WriteErrorLine( sError );
				nErrorLines++;
				goto _ADVANCE_TO_NEXT_LINE;
			}

			// see if the right string is a non-string value
			if( CConvertCsvFile::IsStringNumeric( sRight ) ) {
				// determine which command string we have
				if( sLeft.CompareNoCase( "dither" ) == 0 ) {
					m_aNonStrings[AID_FILE_FLOATS_DITHER].bValueWasFound = TRUE;
					m_aNonStrings[AID_FILE_FLOATS_DITHER].fValue = (f32)strtod( sRight, &pEndString );
					nNumValidLines++;

					// this value is a BOOL, do the logic here
					m_aNonStrings[AID_FILE_FLOATS_DITHER].bValue = ( m_aNonStrings[AID_FILE_FLOATS_DITHER].fValue == 0.0f ) ? FALSE : TRUE;

				} else if( sLeft.CompareNoCase( "generate_mips" ) == 0 ) {
					m_aNonStrings[AID_FILE_FLOATS_GENERATE_MIPS].bValueWasFound = TRUE;
					m_aNonStrings[AID_FILE_FLOATS_GENERATE_MIPS].fValue = (f32)strtod( sRight, &pEndString );
					nNumValidLines++;

					// this value is a BOOL, do the logic here
					m_aNonStrings[AID_FILE_FLOATS_GENERATE_MIPS].bValue = ( m_aNonStrings[AID_FILE_FLOATS_GENERATE_MIPS].fValue == 0.0f ) ? FALSE : TRUE;

				} else if( sLeft.CompareNoCase( "generate_lightmap_uvs" ) == 0 ) {
					m_aNonStrings[AID_FILE_FLOATS_GENERATE_LMS].bValueWasFound = TRUE;
					m_aNonStrings[AID_FILE_FLOATS_GENERATE_LMS].fValue = (f32)strtod( sRight, &pEndString );
					nNumValidLines++;

					// this value is a BOOL, do the logic here
					m_aNonStrings[AID_FILE_FLOATS_GENERATE_LMS].bValue = ( m_aNonStrings[AID_FILE_FLOATS_GENERATE_LMS].fValue == 0.0f ) ? FALSE : TRUE;

				} else if( sLeft.CompareNoCase( "collision_LOD" ) == 0 ) {
					m_aNonStrings[AID_FILE_FLOATS_COLLISION_LOD].bValueWasFound = TRUE;
					m_aNonStrings[AID_FILE_FLOATS_COLLISION_LOD].nValue = (u32)strtod( sRight, &pEndString );
					nNumValidLines++;

				} else if( sLeft.CompareNoCase( "shadow_LOD_bias" ) == 0 ) {
					m_aNonStrings[AID_FILE_FLOATS_SHADOW_LOD_BIAS].bValueWasFound = TRUE;
					m_aNonStrings[AID_FILE_FLOATS_SHADOW_LOD_BIAS].nValue = (u32)strtod( sRight, &pEndString );
					nNumValidLines++;

				} else if( sLeft.CompareNoCase( "light_map_detail" ) == 0 ) {
					m_aNonStrings[AID_FILE_FLOATS_LIGHTMAP_DETAIL].bValueWasFound = TRUE;
					m_aNonStrings[AID_FILE_FLOATS_LIGHTMAP_DETAIL].nValue = (u32)strtod( sRight, &pEndString );
					nNumValidLines++;

					// clamp this value to the valid range
					FMATH_CLAMP( m_aNonStrings[AID_FILE_FLOATS_LIGHTMAP_DETAIL].fValue, 0.0f, 10.0f );

				} else if( sLeft.CompareNoCase( "light_map_mbs" ) == 0 ) {
					m_aNonStrings[AID_FILE_FLOATS_LIGHTMAP_MBS].bValueWasFound = TRUE;
					m_aNonStrings[AID_FILE_FLOATS_LIGHTMAP_MBS].fValue = (f32)strtod( sRight, &pEndString );
					nNumValidLines++;

				} else if( sLeft.CompareNoCase( "light_map_subsample" ) == 0 ) {
					m_aNonStrings[AID_FILE_FLOATS_LIGHTMAP_SUBSAMPLE].bValueWasFound = TRUE;
					m_aNonStrings[AID_FILE_FLOATS_LIGHTMAP_SUBSAMPLE].fValue = (f32)strtod( sRight, &pEndString );
					nNumValidLines++;

				} else if( sLeft.CompareNoCase( "light_map_skylight_detail" ) == 0) {
					m_aNonStrings[AID_FILE_FLOATS_LIGHTMAP_SKYLIGHT_DETAIL].bValueWasFound = TRUE;
					m_aNonStrings[AID_FILE_FLOATS_LIGHTMAP_SKYLIGHT_DETAIL].nValue = (u32)strtod( sRight, &pEndString );
					nNumValidLines++;
				} else if( sLeft.CompareNoCase( "light_map_skylight_horiz_red" ) == 0) {
					m_aNonStrings[AID_FILE_FLOATS_LIGHTMAP_SKYLIGHT_HORIZ_RED].bValueWasFound = TRUE;
					m_aNonStrings[AID_FILE_FLOATS_LIGHTMAP_SKYLIGHT_HORIZ_RED].nValue = (u32)strtod( sRight, &pEndString );
					nNumValidLines++;
				} else if( sLeft.CompareNoCase( "light_map_skylight_horiz_green" ) == 0) {
					m_aNonStrings[AID_FILE_FLOATS_LIGHTMAP_SKYLIGHT_HORIZ_GREEN].bValueWasFound = TRUE;
					m_aNonStrings[AID_FILE_FLOATS_LIGHTMAP_SKYLIGHT_HORIZ_GREEN].nValue = (u32)strtod( sRight, &pEndString );
					nNumValidLines++;
				} else if( sLeft.CompareNoCase( "light_map_skylight_horiz_blue" ) == 0) {
					m_aNonStrings[AID_FILE_FLOATS_LIGHTMAP_SKYLIGHT_HORIZ_BLUE].bValueWasFound = TRUE;
					m_aNonStrings[AID_FILE_FLOATS_LIGHTMAP_SKYLIGHT_HORIZ_BLUE].nValue = (u32)strtod( sRight, &pEndString );
					nNumValidLines++;
				} else if( sLeft.CompareNoCase( "light_map_skylight_zenith_red" ) == 0) {
					m_aNonStrings[AID_FILE_FLOATS_LIGHTMAP_SKYLIGHT_ZENITH_RED].bValueWasFound = TRUE;
					m_aNonStrings[AID_FILE_FLOATS_LIGHTMAP_SKYLIGHT_ZENITH_RED].nValue = (u32)strtod( sRight, &pEndString );
					nNumValidLines++;
				} else if( sLeft.CompareNoCase( "light_map_skylight_zenith_green" ) == 0) {
					m_aNonStrings[AID_FILE_FLOATS_LIGHTMAP_SKYLIGHT_ZENITH_GREEN].bValueWasFound = TRUE;
					m_aNonStrings[AID_FILE_FLOATS_LIGHTMAP_SKYLIGHT_ZENITH_GREEN].nValue = (u32)strtod( sRight, &pEndString );
					nNumValidLines++;
				} else if( sLeft.CompareNoCase( "light_map_skylight_zenith_blue" ) == 0) {
					m_aNonStrings[AID_FILE_FLOATS_LIGHTMAP_SKYLIGHT_ZENITH_BLUE].bValueWasFound = TRUE;
					m_aNonStrings[AID_FILE_FLOATS_LIGHTMAP_SKYLIGHT_ZENITH_BLUE].nValue = (u32)strtod( sRight, &pEndString );
					nNumValidLines++;
				} else if( sLeft.CompareNoCase( "light_map_skylight_intens" ) == 0) {
					m_aNonStrings[AID_FILE_FLOATS_LIGHTMAP_SKYLIGHT_INTENSITY].bValueWasFound = TRUE;
					m_aNonStrings[AID_FILE_FLOATS_LIGHTMAP_SKYLIGHT_INTENSITY].fValue = (f32)strtod( sRight, &pEndString );;
					nNumValidLines++;
				} else if( sLeft.CompareNoCase( "mip_to" ) == 0 ) {
					m_aNonStrings[AID_FILE_FLOATS_MIP_TO].bValueWasFound = TRUE;
					m_aNonStrings[AID_FILE_FLOATS_MIP_TO].fValue = (f32)strtod( sRight, &pEndString );
					nNumValidLines++;

					// this value is a BOOL, do the logic here
					m_aNonStrings[AID_FILE_FLOATS_MIP_TO].bValue = ( m_aNonStrings[AID_FILE_FLOATS_MIP_TO].fValue == 0.0f ) ? FALSE : TRUE;

				} else if( sLeft.CompareNoCase( "gc_tex_scale" ) == 0 ) {
					m_aNonStrings[AID_FILE_FLOATS_GC_TEX_SCALE].bValueWasFound = TRUE;
					m_aNonStrings[AID_FILE_FLOATS_GC_TEX_SCALE].nValue = (u32)strtod( sRight, &pEndString );
					nNumValidLines++;

					// this value is a BOOL, do the logic here
//					m_aNonStrings[AID_FILE_FLOATS_GC_TEX_SCALE].bValue = ( m_aNonStrings[AID_FILE_FLOATS_GC_TEX_SCALE].fValue == 0.0f ) ? FALSE : TRUE;

				} else if( sLeft.CompareNoCase( "anim_compress" ) == 0 ) {
					m_aNonStrings[AID_FILE_FLOATS_ANIM_COMPRESS].bValueWasFound = TRUE;
					m_aNonStrings[AID_FILE_FLOATS_ANIM_COMPRESS].fValue = (f32)strtod( sRight, &pEndString );
					nNumValidLines++;

					// this value is a BOOL, do the logic here
					m_aNonStrings[AID_FILE_FLOATS_ANIM_COMPRESS].bValue = ( m_aNonStrings[AID_FILE_FLOATS_ANIM_COMPRESS].fValue == 0.0f ) ? FALSE : TRUE;

				} else if( sLeft.CompareNoCase( "time_compress" ) == 0 ) {
					m_aNonStrings[AID_FILE_FLOATS_TIME_COMPRESS].bValueWasFound = TRUE;
					m_aNonStrings[AID_FILE_FLOATS_TIME_COMPRESS].fValue = (f32)strtod( sRight, &pEndString );
					nNumValidLines++;

					// this value is a BOOL, do the logic here
					m_aNonStrings[AID_FILE_FLOATS_TIME_COMPRESS].bValue = ( m_aNonStrings[AID_FILE_FLOATS_TIME_COMPRESS].fValue == 0.0f ) ? FALSE : TRUE;

				} else if( sLeft.CompareNoCase( "threshold" ) == 0 ) {
					m_aNonStrings[AID_FILE_FLOATS_THRESHOLD].bValueWasFound = TRUE;
					m_aNonStrings[AID_FILE_FLOATS_THRESHOLD].fValue = (f32)strtod( sRight, &pEndString );
					nNumValidLines++;

					// clamp this value to the valid range
					FMATH_CLAMP( m_aNonStrings[AID_FILE_FLOATS_THRESHOLD].fValue, 0.0f, 45.0f );

				} else if( sLeft.CompareNoCase( "bump_scale" ) == 0 ) {
					m_aNonStrings[AID_FILE_FLOATS_BUMP_SCALE].bValueWasFound = TRUE;
					m_aNonStrings[AID_FILE_FLOATS_BUMP_SCALE].fValue = (f32)strtod( sRight, &pEndString );

					nNumValidLines++;

					// clamp this value to the valid range
					FMATH_CLAMP( m_aNonStrings[AID_FILE_FLOATS_BUMP_SCALE].fValue, 0.0f, 10.0f );

				} else if( sLeft.CompareNoCase( "bump_wrap" ) == 0 ) {
					m_aNonStrings[AID_FILE_FLOATS_BUMP_WRAP].bValueWasFound = TRUE;
					m_aNonStrings[AID_FILE_FLOATS_BUMP_WRAP].fValue = (f32)strtod( sRight, &pEndString );
					nNumValidLines++;

					// this value is a BOOL, do the logic here
					m_aNonStrings[AID_FILE_FLOATS_BUMP_WRAP].bValue = ( m_aNonStrings[AID_FILE_FLOATS_BUMP_WRAP].fValue == 0.0f ) ? FALSE : TRUE;

// NS PS2 specific aid file settings
#ifdef _MMI_TARGET_PS2
				} else if( sLeft.CompareNoCase( "layer" ) == 0 ) {
					m_aNonStrings[AID_FILE_FLOATS_LAYER].bValueWasFound = TRUE;
                    m_aNonStrings[AID_FILE_FLOATS_LAYER].nValue = (int)strtod( sRight, &pEndString );
					nNumValidLines++;
				} else if( sLeft.CompareNoCase( "l_bias" ) == 0 ) {
					m_aNonStrings[AID_FILE_FLOATS_LBIAS].bValueWasFound = TRUE;
                    m_aNonStrings[AID_FILE_FLOATS_LBIAS].nValue = (int)strtod( sRight, &pEndString );
                    nNumValidLines++;

				} else if( sLeft.CompareNoCase( "k_bias" ) == 0 ) {
					m_aNonStrings[AID_FILE_FLOATS_KBIAS].bValueWasFound = TRUE;
                    m_aNonStrings[AID_FILE_FLOATS_KBIAS].nValue = (int)strtod( sRight, &pEndString );
					nNumValidLines++;

				} else if( sLeft.CompareNoCase( "mipmap_filter" ) == 0 ) {
					m_aNonStrings[AID_FILE_FLOATS_MIPMAP_FILTER].bValueWasFound = TRUE;
                    m_aNonStrings[AID_FILE_FLOATS_MIPMAP_FILTER].nValue = (u32)strtod( sRight, &pEndString );
					nNumValidLines++;
				} else if( sLeft.CompareNoCase( "downsize" ) == 0 ) {
					m_aNonStrings[AID_FILE_FLOATS_DOWNSIZE].bValueWasFound = TRUE;
                    m_aNonStrings[AID_FILE_FLOATS_DOWNSIZE].nValue = (u32)strtod( sRight, &pEndString );
					nNumValidLines++;
#endif
// NS
				} else {
					// unknown command
					if( !nErrorLines ) {
						rErrorLog.WriteErrorHeader( _ERROR_HEADING + sFilepath );
					}
					sError.Format( "The following line contains an unknown command string: '%s'", sLine );
					rErrorLog.WriteErrorLine( sError );
					nErrorLines++;
					goto _ADVANCE_TO_NEXT_LINE;
				}
				// the right side is a number, don't treat it as a string
				goto _ADVANCE_TO_NEXT_LINE;
			}

			// the right side contains a string, test for errors
			if( DoesStringContainInvalidChars( sRight, FALSE ) ) {
				if( !nErrorLines ) {
					rErrorLog.WriteErrorHeader( _ERROR_HEADING + sFilepath );
				}
				sError.Format( "The following line contains invalid characters in the value string: '%s'", sLine );
				rErrorLog.WriteErrorLine( sError );
				nErrorLines++;
				goto _ADVANCE_TO_NEXT_LINE;
			}

			// determine which command string we have
			if( sLeft.CompareNoCase( "texture_usage" ) == 0 ) {
				m_aStrings[AID_FILE_STRINGS_TEXTURE_USAGE].bValueWasFound = TRUE;
				m_aStrings[AID_FILE_STRINGS_TEXTURE_USAGE].sValue = sRight;
				nNumValidLines++;

			} else if( sLeft.CompareNoCase( "xb_tex_format" ) == 0 ) {
				m_aStrings[AID_FILE_STRINGS_XB_TEX_FORMAT].bValueWasFound = TRUE;
				m_aStrings[AID_FILE_STRINGS_XB_TEX_FORMAT].sValue = sRight;
				nNumValidLines++;

			} else if( sLeft.CompareNoCase( "gc_tex_format" ) == 0 ) {
				m_aStrings[AID_FILE_STRINGS_GC_TEX_FORMAT].bValueWasFound = TRUE;
				m_aStrings[AID_FILE_STRINGS_GC_TEX_FORMAT].sValue = sRight;
				nNumValidLines++;

#ifdef _MMI_TARGET_PS2
            } else if( sLeft.CompareNoCase( "ps2_tex_format" ) == 0 ) {
                m_aStrings[AID_FILE_STRINGS_PS2_TEX_FORMAT].bValueWasFound = TRUE;
                m_aStrings[AID_FILE_STRINGS_PS2_TEX_FORMAT].sValue = sRight;
				nNumValidLines++;
#endif
			} else if( sLeft.Left(8).CompareNoCase( "partlist" ) == 0 ) {
				u32 nIndex = atol( sLeft.Right( sLeft.GetLength() - 8 ).GetBuffer(0) );
				m_aPartStrings[nIndex].bValueWasFound = TRUE;
				m_aPartStrings[nIndex].sValue = sRight;
				nNumValidLines++;

			} else if( sLeft.CompareNoCase( "switch_LOD" ) == 0 ) {
				m_aStrings[AID_FILE_STRINGS_LOD_SWITCHES].bValueWasFound = TRUE;
				m_aStrings[AID_FILE_STRINGS_LOD_SWITCHES].sValue = sRight;
				nNumValidLines++;

			} else {
				// unknown command
				if( !nErrorLines ) {
					rErrorLog.WriteErrorHeader( _ERROR_HEADING + sFilepath );
				}
				sError.Format( "The following line contains an unknown command string: '%s'", sLine );
				rErrorLog.WriteErrorLine( sError );
				nErrorLines++;
				goto _ADVANCE_TO_NEXT_LINE;
			}
		}
_ADVANCE_TO_NEXT_LINE:
		// move to the next line
		pszTextFile = GetNextLine( pszTextFile, pEOF );
	}

	if( !nNumValidLines || nErrorLines ) {
		delete [] pMemAllocated;
		if( !nNumValidLines && !nErrorLines ) {
			// the only error is that are no valid lines in the file
			rErrorLog.WriteErrorHeader( _ERROR_HEADING + sFilepath );
			rErrorLog.WriteErrorLine( "There are no valid lines in the aid file, delete it." );
		}
		return FALSE;
	}

	delete [] pMemAllocated;

	m_bDataValid = TRUE;
	return TRUE;
}

BOOL CAidFile::IsCommentLine( const CString &rsString ) {

	if( rsString.Find( '#' ) >= 0 ) {
		return TRUE;
	}
	return FALSE;
}

// replaces CR, LF, and ^Z with NULL
void CAidFile::ReplaceControlCharsWithNULL( u8 *pMem, u32 nLen ) {
	u32 i;
	u8 c;

	for( i=0; i < nLen; i++ ) {
		c = pMem[i];

		switch( c ) {

		case 0x1A:
		case 0xA:
		case 0xD:
			pMem[i] = 0;
			break;
		}
	}
}

BOOL CAidFile::SplitLineIntoLeftRight( const CString &rsString, CString &rsLeft, CString &rsRight ) {
	int nIndex;

	nIndex = rsString.Find( '=', 0 );
	if( nIndex < 0 ) {
		// there is no = sign
		return FALSE;
	}

	// extract the left and right parts of the string
	rsLeft = rsString.Mid( 0, nIndex );
	rsRight = rsString.Mid( nIndex+1 );

	// trim the left string
	rsLeft.TrimLeft();
	rsLeft.TrimRight();

	// trim the right string
	rsRight.TrimLeft();
	rsRight.TrimRight();

	return TRUE;
}

BOOL CAidFile::DoesStringContainInvalidChars( const CString &rsString, BOOL bAllowSpaces ) {

	if( !bAllowSpaces ) {
		return (rsString.FindOneOf( "\" ()<>?|+=*&^%$@!~`';:/.{}[]\\#" ) >= 0);
	}
	return (rsString.FindOneOf( "\"()<>?|+=*&^%$@!~`';:/.{}[]\\#" ) >= 0);
}

char *CAidFile::GetNextLine( char *pszLine, char *pszEOF ) {

	if( !pszLine || !pszEOF || (pszLine >= pszEOF) ) {
		return NULL;
	}
	if( pszLine[0] != NULL ) {
		// move to the next NULL char
		while( pszLine < pszEOF ) {
			pszLine++;
			if( pszLine[0] == NULL ) {
				break;
			}
		}
		// make sure that we haven't gone too far
		if( pszLine >= pszEOF ) {
			return NULL;
		}
	}

	// move to the 1st non NULL char
	while( pszLine < pszEOF ) {
		pszLine++;
		if( pszLine[0] != NULL ) {
			break;
		}
	}
	// make sure that we haven't gone too far
	if( pszLine >= pszEOF ) {
		return NULL;
	}
	return pszLine;
}
