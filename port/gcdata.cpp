#include "gcdata.h"

#include "fdata.h"
#include "fparticle.h"
#include "fres.h"

#include <math.h>
#include <stddef.h>
#include <string.h>

static u16 _ReadBE16( const u8 *pData )
{
	return (u16)(((u16)pData[0] << 8) | pData[1]);
}

static u32 _ReadBE32( const u8 *pData )
{
	return ((u32)pData[0] << 24) | ((u32)pData[1] << 16) | ((u32)pData[2] << 8) | (u32)pData[3];
}

static BOOL _IsRangeValid( u32 nOffset, u32 nLength, u32 nTotalBytes )
{
	return nOffset <= nTotalBytes && nLength <= nTotalBytes - nOffset;
}

static BOOL _ConvertCsv( void *pData, u32 nBytes )
{
	u8 *pBytes = (u8 *)pData;
	if( !pBytes || nBytes < sizeof(FDataGamFile_Header_t) ) return FALSE;

	const u32 nSerializedBytes = _ReadBE32( pBytes + offsetof(FDataGamFile_Header_t, nBytesInFile) );
	const u32 nNumTables = _ReadBE32( pBytes + offsetof(FDataGamFile_Header_t, nNumTables) );
	const u32 nTablesOffset = _ReadBE32( pBytes + offsetof(FDataGamFile_Header_t, paTables) );
	if( nSerializedBytes != nBytes || nNumTables == 0 || nNumTables > FDATA_GAMFILE_MAX_TABLES ||
		!_IsRangeValid( nTablesOffset, nNumTables * sizeof(FDataGamFile_Table_t), nBytes ) )
	{
		DEVPRINTF( "gcdata: invalid GameCube CSV header (bytes=%u/%u tables=%u offset=0x%08x).\n",
			nSerializedBytes, nBytes, nNumTables, nTablesOffset );
		return FALSE;
	}

	FDataGamFile_Header_t *pHeader = (FDataGamFile_Header_t *)pData;
	pHeader->ChangeEndian();
	pHeader->nFlags = FDATA_GAMFILE_FLAGS_NONE;

	FDataGamFile_Table_t *pTables = (FDataGamFile_Table_t *)(pBytes + nTablesOffset);
	for( u32 i=0; i<nNumTables; i++ )
	{
		const u16 nFields = _ReadBE16( pBytes + nTablesOffset + i * sizeof(FDataGamFile_Table_t) + offsetof(FDataGamFile_Table_t, nNumFields) );
		const u32 nFieldsOffset = _ReadBE32( pBytes + nTablesOffset + i * sizeof(FDataGamFile_Table_t) + offsetof(FDataGamFile_Table_t, paFields) );
		const u32 nKeyOffset = _ReadBE32( pBytes + nTablesOffset + i * sizeof(FDataGamFile_Table_t) + offsetof(FDataGamFile_Table_t, pszKeyString) );
		if( nFields > FDATA_GAMFILE_MAX_FIELDS_PER_TABLE ||
			!_IsRangeValid( nFieldsOffset, (u32)nFields * sizeof(FDataGamFile_Field_t), nBytes ) ||
			(nKeyOffset && nKeyOffset >= nBytes) )
		{
			DEVPRINTF( "gcdata: invalid GameCube CSV table %u.\n", i );
			return FALSE;
		}

		pTables[i].ChangeEndian();
		FDataGamFile_Field_t *pFields = (FDataGamFile_Field_t *)(pBytes + nFieldsOffset);
		for( u32 j=0; j<nFields; j++ )
		{
			const u32 nFieldOffset = nFieldsOffset + j * sizeof(FDataGamFile_Field_t);
			const u32 nDataType = _ReadBE32( pBytes + nFieldOffset + offsetof(FDataGamFile_Field_t, nDataType) );
			if( nDataType >= FDATA_GAMEFILE_DATA_TYPE_COUNT )
			{
				DEVPRINTF( "gcdata: invalid GameCube CSV field type %u at table %u field %u.\n", nDataType, i, j );
				return FALSE;
			}

			if( nDataType == FDATA_GAMEFILE_DATA_TYPE_WIDESTRING )
			{
				const u32 nStringOffset = _ReadBE32( pBytes + nFieldOffset + offsetof(FDataGamFile_Field_t, pwszValue) );
				const u32 nStringLength = _ReadBE32( pBytes + nFieldOffset + offsetof(FDataGamFile_Field_t, nStringLen) );
				if( nStringLength > 0x7fffffffu || ! _IsRangeValid( nStringOffset, nStringLength * 2, nBytes ) )
				{
					DEVPRINTF( "gcdata: invalid GameCube CSV wide string at table %u field %u.\n", i, j );
					return FALSE;
				}
				for( u32 k=0; k<nStringLength; k++ )
				{
					u8 *pChar = pBytes + nStringOffset + k * 2;
					u8 nTemp = pChar[0];
					pChar[0] = pChar[1];
					pChar[1] = nTemp;
				}
			}

			pFields[j].ChangeEndian( (u32)pData );
		}
	}

	return TRUE;
}

static BOOL _ConvertFpr( void *pData, u32 nBytes, cchar *pszResName )
{
	u8 *pBytes = (u8 *)pData;
	if( !pBytes || nBytes < sizeof(u32) ) return FALSE;

	const u32 nVersion = _ReadBE32( pBytes );
	if( nVersion == FPARTICLE_FILE_VERSION && nBytes == sizeof(FParticleDef_t) )
	{
		((FParticleDef_t *)pData)->ChangeEndian();
		return TRUE;
	}

	// Retail GC particles are version 8. The serialized structure adds one
	// 8-byte field to each keyframe; the old fields after each keyframe retain
	// their order. Remove those additions so the v7 runtime layout can consume
	// the file. Validate the shifted texture name and invariant tail first.
	const u32 nV7TextureOffset = (u32)offsetof(FParticleDef_t, szTextureName);
	const u32 nV7TailOffset = (u32)offsetof(FParticleDef_t, fSecsBetweenLightSamples);
	const u32 nKeyFrameBytes = sizeof(FParticleKeyFrame_t);
	const u32 nHousekeepingBytes = sizeof(u32) + sizeof(FLink_t) + sizeof(FLinkRoot_t);
	const u32 nLegacySuffixBytes = sizeof(f32) + sizeof(CFVec3) + nHousekeepingBytes;
	if( nVersion != FPARTICLE_FILE_VERSION + 1 ||
		nBytes != sizeof(FParticleDef_t) + 16 ||
		nV7TextureOffset != 2 * sizeof(u32) + 2 * nKeyFrameBytes ||
		nV7TailOffset > sizeof(FParticleDef_t) ||
		nLegacySuffixBytes != sizeof(FParticleDef_t) - nV7TailOffset )
	{
		DEVPRINTF( "gcdata: unsupported GameCube particle format in '%s' (version %u, %u bytes).\n",
			pszResName ? pszResName : "(unnamed)", nVersion, nBytes );
		return FALSE;
	}

	const u32 nV8TextureOffset = nV7TextureOffset + 16;
	const u32 nTextureNameBytes = FDATA_TEXNAME_LEN + 1;
	if( !_IsRangeValid( nV8TextureOffset, nTextureNameBytes, nBytes ) ||
		!memchr( pBytes + nV8TextureOffset, 0, nTextureNameBytes ) )
	{
		DEVPRINTF( "gcdata: invalid shifted texture name in GameCube particle '%s'.\n",
			pszResName ? pszResName : "(unnamed)" );
		return FALSE;
	}

	const u32 nV8TailOffset = nV7TailOffset + 16;
	const f32 fSamplePeriod = fang_ConvertEndian( *(const f32 *)(pBytes + nV8TailOffset) );
	if( !isfinite( fSamplePeriod ) || fSamplePeriod < (1.0f / 20.0f) || fSamplePeriod > 1.0f )
	{
		DEVPRINTF( "gcdata: invalid GameCube particle sampling period in '%s'.\n",
			pszResName ? pszResName : "(unnamed)" );
		return FALSE;
	}

	const u32 nHousekeepingOffset = nV8TailOffset + sizeof(f32) + sizeof(CFVec3);
	for( u32 i = 0; i < nHousekeepingBytes; i++ )
	{
		if( pBytes[nHousekeepingOffset + i] != 0 )
		{
			DEVPRINTF( "gcdata: unsupported nonzero GameCube particle housekeeping data in '%s'.\n",
				pszResName ? pszResName : "(unnamed)" );
			return FALSE;
		}
	}

	const u32 nMinExtensionOffset = 2 * sizeof(u32) + nKeyFrameBytes;
	const u32 nMaxExtensionOffsetV8 = nMinExtensionOffset + 8 + nKeyFrameBytes;
	for( u32 i = 0; i < 8; i++ )
	{
		if( pBytes[nMinExtensionOffset + i] != 0 || pBytes[nMaxExtensionOffsetV8 + i] != 0 )
		{
			DEVPRINTF( "gcdata: unsupported nonzero GameCube particle keyframe extension in '%s'.\n",
				pszResName ? pszResName : "(unnamed)" );
			return FALSE;
		}
	}

	memmove( pBytes + nMinExtensionOffset, pBytes + nMinExtensionOffset + 8,
		nBytes - (nMinExtensionOffset + 8) );
	const u32 nMaxExtensionOffset = 2 * sizeof(u32) + 2 * nKeyFrameBytes;
	const u32 nBytesAfterMinCompaction = nBytes - 8;
	memmove( pBytes + nMaxExtensionOffset, pBytes + nMaxExtensionOffset + 8,
		nBytesAfterMinCompaction - (nMaxExtensionOffset + 8) );
	pBytes[0] = (u8)(FPARTICLE_FILE_VERSION >> 24);
	pBytes[1] = (u8)(FPARTICLE_FILE_VERSION >> 16);
	pBytes[2] = (u8)(FPARTICLE_FILE_VERSION >> 8);
	pBytes[3] = (u8)FPARTICLE_FILE_VERSION;
	((FParticleDef_t *)pData)->ChangeEndian();
	return TRUE;
}

static u8 _Expand5( u32 nValue )
{
	return (u8)((nValue << 3) | (nValue >> 2));
}

static u8 _Expand6( u32 nValue )
{
	return (u8)((nValue << 2) | (nValue >> 4));
}

static u8 _Expand4( u32 nValue )
{
	return (u8)(nValue * 17);
}

static u32 _PackArgb( u8 nR, u8 nG, u8 nB, u8 nA )
{
	return ((u32)nA << 24) | ((u32)nR << 16) | ((u32)nG << 8) | nB;
}

static u32 _GCTileBytes( u32 nFormat, u32 nWidth, u32 nHeight )
{
	u32 nAcross, nDown, nBytesPerTile;

	switch( nFormat )
	{
	case 0: case 1: nAcross = 4; nDown = 4; nBytesPerTile = 64; break;
	case 2: case 3: case 4: case 9: nAcross = 4; nDown = 4; nBytesPerTile = 32; break;
	case 5: case 6: case 7: case 11: nAcross = 8; nDown = 8; nBytesPerTile = 32; break;
	case 8: case 10: nAcross = 8; nDown = 4; nBytesPerTile = 32; break;
	default: return 0;
	}
	return ((nWidth + nAcross - 1) / nAcross) * ((nHeight + nDown - 1) / nDown) * nBytesPerTile;
}

static void _DecodeCmprPlane( const u8 *pSource, u32 nWidth, u32 nHeight, u32 *pPixels, BOOL bAlphaPlane, BOOL bOneBitAlpha )
{
	const u32 nTilesAcross = (nWidth + 7) >> 3;
	const u32 nTilesDown = (nHeight + 7) >> 3;

	for( u32 nTileY = 0; nTileY < nTilesDown; nTileY++ )
	{
		for( u32 nTileX = 0; nTileX < nTilesAcross; nTileX++ )
		{
			const u8 *pTile = pSource + ((nTileY * nTilesAcross + nTileX) * 32);
			for( u32 nBlockY = 0; nBlockY < 2; nBlockY++ )
			{
				for( u32 nBlockX = 0; nBlockX < 2; nBlockX++ )
				{
					const u8 *pBlock = pTile + ((nBlockY * 2 + nBlockX) * 8);
					const u16 nColor0 = _ReadBE16( pBlock );
					const u16 nColor1 = _ReadBE16( pBlock + 2 );
					u8 anColor[4][4];
					anColor[0][0] = _Expand5( (nColor0 >> 11) & 31 );
					anColor[0][1] = _Expand6( (nColor0 >> 5) & 63 );
					anColor[0][2] = _Expand5( nColor0 & 31 );
					anColor[1][0] = _Expand5( (nColor1 >> 11) & 31 );
					anColor[1][1] = _Expand6( (nColor1 >> 5) & 63 );
					anColor[1][2] = _Expand5( nColor1 & 31 );
					if( nColor0 > nColor1 || !bOneBitAlpha )
					{
						for( u32 nChannel = 0; nChannel < 3; nChannel++ )
						{
							anColor[2][nChannel] = (u8)((2 * anColor[0][nChannel] + anColor[1][nChannel]) / 3);
							anColor[3][nChannel] = (u8)((anColor[0][nChannel] + 2 * anColor[1][nChannel]) / 3);
						}
					}
					else
					{
						for( u32 nChannel = 0; nChannel < 3; nChannel++ )
							anColor[2][nChannel] = (u8)((anColor[0][nChannel] + anColor[1][nChannel]) / 2);
						anColor[3][0] = anColor[3][1] = anColor[3][2] = 0;
					}

					for( u32 nY = 0; nY < 4; nY++ )
					{
						const u8 nIndices = pBlock[4 + nY];
						for( u32 nX = 0; nX < 4; nX++ )
						{
							const u32 nColorIndex = (nIndices >> (6 - nX * 2)) & 3;
							const u32 nXOut = nTileX * 8 + nBlockX * 4 + nX;
							const u32 nYOut = nTileY * 8 + nBlockY * 4 + nY;
							if( nXOut >= nWidth || nYOut >= nHeight ) continue;

							const u8 nR = anColor[nColorIndex][0];
							const u8 nG = anColor[nColorIndex][1];
							const u8 nB = anColor[nColorIndex][2];
							const u32 nIndex = nYOut * nWidth + nXOut;
							if( bAlphaPlane )
							{
								pPixels[nIndex] = (pPixels[nIndex] & 0x00ffffff) | ((u32)nG << 24);
							}
							else
							{
								const u8 nAlpha = (bOneBitAlpha && nColor0 <= nColor1 && nColorIndex == 3) ? 0 : 255;
								pPixels[nIndex] = _PackArgb( nR, nG, nB, nAlpha );
							}
						}
					}
				}
			}
		}
	}
}

static BOOL _DecodeUncompressedGCTile( u32 nFormat, const u8 *pTile, u32 nTileX, u32 nTileY, u32 nWidth, u32 nHeight, u32 *pPixels )
{
	u32 nTileAcross = 4, nTileDown = 4;
	if( nFormat == 8 || nFormat == 10 ) { nTileAcross = 8; nTileDown = 4; }
	if( nFormat == 11 ) { nTileAcross = 8; nTileDown = 8; }

	for( u32 y = 0; y < nTileDown; y++ )
	{
		for( u32 x = 0; x < nTileAcross; x++ )
		{
			const u32 nX = nTileX * nTileAcross + x;
			const u32 nY = nTileY * nTileDown + y;
			if( nX >= nWidth || nY >= nHeight ) continue;
			u8 nR = 0, nG = 0, nB = 0, nA = 255;
			switch( nFormat )
			{
			case 0: case 1:
			{
				const u32 nOffset = y * 8 + x * 2;
				nA = pTile[nOffset]; nR = pTile[nOffset + 1];
				nG = pTile[32 + nOffset]; nB = pTile[33 + nOffset];
				if( nFormat == 1 ) nA = 255;
				break;
			}
			case 2: case 3:
			{
				const u16 nColor = _ReadBE16( pTile + (y * 4 + x) * 2 );
				if( nColor & 0x8000 )
				{
					nR = _Expand5( (nColor >> 10) & 31 );
					nG = _Expand5( (nColor >> 5) & 31 );
					nB = _Expand5( nColor & 31 );
				}
				else
				{
					nA = _Expand4( (nColor >> 12) & 7 );
					nR = _Expand4( (nColor >> 8) & 15 );
					nG = _Expand4( (nColor >> 4) & 15 );
					nB = _Expand4( nColor & 15 );
				}
				if( nFormat == 3 ) nA = 255;
				break;
			}
			case 4:
			{
				const u16 nColor = _ReadBE16( pTile + (y * 4 + x) * 2 );
				nR = _Expand5( (nColor >> 11) & 31 );
				nG = _Expand6( (nColor >> 5) & 63 );
				nB = _Expand5( nColor & 31 );
				break;
			}
			case 8:
			{
				const u8 nIntensity = pTile[y * 8 + x];
				nR = nG = nB = nIntensity;
				break;
			}
			case 9:
			{
				const u32 nOffset = (y * 4 + x) * 2;
				nA = pTile[nOffset];
				nR = nG = nB = pTile[nOffset + 1];
				break;
			}
			case 10:
			{
				const u8 nIA = pTile[y * 8 + x];
				nA = _Expand4( (nIA >> 4) & 15 );
				nR = nG = nB = _Expand4( nIA & 15 );
				break;
			}
			case 11:
			{
				const u8 nPair = pTile[y * 4 + (x >> 1)];
				const u8 nIntensity = (x & 1) ? (nPair & 15) : (nPair >> 4);
				nR = nG = nB = _Expand4( nIntensity );
				break;
			}
			default:
				return FALSE;
			}
			pPixels[nY * nWidth + nX] = _PackArgb( nR, nG, nB, nA );
		}
	}
	return TRUE;
}

BOOL gcdata_DecodeTga( const void *pFileData, u32 nFileBytes, FTexInfo_t *pTexInfo, void **ppImageData, u32 *pnImageBytes )
{
#if FANG_PLATFORM_WIN && FANG_WINGC
	if( !pFileData || !pTexInfo || !ppImageData || !pnImageBytes || nFileBytes < sizeof(FTexInfo_t) ) return FALSE;

	memcpy( pTexInfo, pFileData, sizeof(FTexInfo_t) );
	pTexInfo->ChangeEndian();
	pTexInfo->pUserData = NULL;
	const u32 nFormat = pTexInfo->nTexFmt;
	if( nFormat >= FGCDATA_TEXFMT_COUNT || !pTexInfo->nLodCount || pTexInfo->nLodCount > 16 ||
		!pTexInfo->nTexelsAcross || !pTexInfo->nTexelsDown )
	{
		DEVPRINTF( "gcdata: invalid GameCube TGA header for '%s'.\n", pTexInfo->szName );
		return FALSE;
	}

	u32 anWidth[16], anHeight[16], anSourceBytes[16], anPixelOffsets[16];
	u32 nTotalSourceBytes = 0, nTotalImageBytes = 0;
	u32 nWidth = pTexInfo->nTexelsAcross, nHeight = pTexInfo->nTexelsDown;
	const u32 nMinDim = (nFormat == 5 || nFormat == 6 || nFormat == 7) ? 8 :
		((nFormat == 0 || nFormat == 1 || nFormat == 2 || nFormat == 3 || nFormat == 4 || nFormat == 8 || nFormat == 9 || nFormat == 10 || nFormat == 11) ? 4 : 0);
	for( u32 i = 0; i < pTexInfo->nLodCount; i++ )
	{
		if( nWidth > 4096 || nHeight > 4096 )
		{
			DEVPRINTF( "gcdata: oversized GameCube TGA '%s' (%ux%u).\n", pTexInfo->szName, nWidth, nHeight );
			return FALSE;
		}
		const u32 nLevelBytes = _GCTileBytes( nFormat, nWidth, nHeight );
		const u32 nPixelBytes = nWidth * nHeight * 4;
		if( !nLevelBytes || (nWidth & (nWidth - 1)) || (nHeight & (nHeight - 1)) ||
			nTotalSourceBytes > 0xffffffffu - nLevelBytes || nTotalImageBytes > 0xffffffffu - nPixelBytes )
		{
			DEVPRINTF( "gcdata: unsupported or oversized GameCube TGA '%s'.\n", pTexInfo->szName );
			return FALSE;
		}
		anWidth[i] = nWidth; anHeight[i] = nHeight;
		anSourceBytes[i] = nLevelBytes;
		anPixelOffsets[i] = nTotalImageBytes;
		nTotalSourceBytes += nLevelBytes;
		nTotalImageBytes += nPixelBytes;
		nWidth >>= 1; nHeight >>= 1;
		if( nWidth < nMinDim ) nWidth = nMinDim;
		if( nHeight < nMinDim ) nHeight = nMinDim;
	}

	const u32 nPlaneCount = (nFormat == 7) ? 2 : 1;
	if( nTotalSourceBytes > (nFileBytes - sizeof(FTexInfo_t)) / nPlaneCount )
	{
		DEVPRINTF( "gcdata: truncated GameCube TGA '%s' (%u bytes of image data expected).\n", pTexInfo->szName, nTotalSourceBytes * nPlaneCount );
		return FALSE;
	}

	u32 *pPixels = (u32 *)fmem_Alloc( nTotalImageBytes, 16 );
	if( !pPixels ) return FALSE;
	const u8 *pImage = (const u8 *)pFileData + sizeof(FTexInfo_t);
	const u8 *pAlphaPlane = pImage + nTotalSourceBytes;
	for( u32 i = 0; i < pTexInfo->nLodCount; i++ )
	{
		u32 *pMipPixels = (u32 *)((u8 *)pPixels + anPixelOffsets[i]);
		if( nFormat <= 4 || nFormat >= 8 )
		{
			const u8 *pMip = pImage;
			for( u32 j = 0; j < i; j++ ) pMip += anSourceBytes[j];
			const u32 nTileAcross = (nFormat == 8 || nFormat == 10) ? 8 : ((nFormat == 11) ? 8 : 4);
			const u32 nTileDown = (nFormat == 8 || nFormat == 10) ? 4 : ((nFormat == 11) ? 8 : 4);
			const u32 nTileBytes = (nFormat == 0 || nFormat == 1) ? 64 : 32;
			const u32 nTilesAcross = (anWidth[i] + nTileAcross - 1) / nTileAcross;
			const u32 nTilesDown = (anHeight[i] + nTileDown - 1) / nTileDown;
			for( u32 y = 0; y < nTilesDown; y++ )
				for( u32 x = 0; x < nTilesAcross; x++ )
					if( !_DecodeUncompressedGCTile( nFormat, pMip + x * nTileBytes + y * nTilesAcross * nTileBytes, x, y, anWidth[i], anHeight[i], pMipPixels ) ) return FALSE;
		}
		else
		{
			const u8 *pMip = pImage;
			for( u32 j = 0; j < i; j++ ) pMip += anSourceBytes[j];
			_DecodeCmprPlane( pMip, anWidth[i], anHeight[i], pMipPixels, FALSE, nFormat == 6 );
			if( nFormat == 7 )
			{
				const u8 *pAlphaMip = pAlphaPlane;
				for( u32 j = 0; j < i; j++ ) pAlphaMip += anSourceBytes[j];
				_DecodeCmprPlane( pAlphaMip, anWidth[i], anHeight[i], pMipPixels, TRUE, FALSE );
			}
		}
	}

	pTexInfo->nTexFmt = FTEX_FMT_A8R8G8B8;
	pTexInfo->nPalFmt = FTEX_PALFMT_NONE;
	pTexInfo->nFlags = FTEX_FLAG_NONE;
	*ppImageData = pPixels;
	*pnImageBytes = nTotalImageBytes;
	return TRUE;
#else
	pFileData; nFileBytes; pTexInfo; ppImageData; pnImageBytes;
	return FALSE;
#endif
}

BOOL gcdata_Convert( cchar *pszExtension, cchar *pszResName, void *pData, u32 nBytes )
{
#if FANG_PLATFORM_WIN && FANG_WINGC
	static u32 nLoggedResources = 0;
	if( nLoggedResources < 16 )
	{
		DEVPRINTF( "gcdata: loaded GameCube resource '%s.%s' (%u bytes).\n",
			pszResName ? pszResName : "(unnamed)", pszExtension ? pszExtension : "(unknown)", nBytes );
		nLoggedResources++;
	}
	if( pszExtension && strcmp( pszExtension, "csv" ) == 0 )
	{
		static BOOL bLoggedFirstCsv = FALSE;
		if( !_ConvertCsv( pData, nBytes ) ) return FALSE;
		if( !bLoggedFirstCsv )
		{
			DEVPRINTF( "gcdata: converted GameCube CSV resource (%u bytes).\n", nBytes );
			bLoggedFirstCsv = TRUE;
		}
	}
	else if( pszExtension && strcmp( pszExtension, "fpr" ) == 0 )
	{
		if( !_ConvertFpr( pData, nBytes, pszResName ) ) return FALSE;
	}
#else
	pszExtension; pszResName; pData; nBytes;
#endif
	return TRUE;
}
