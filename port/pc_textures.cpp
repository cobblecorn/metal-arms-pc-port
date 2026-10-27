// Texture replacement folder (see pc_textures.h).
#include "pc_textures.h"
#include "fres.h"
#if FANG_WINGC
#include "pc_mods.h"
#endif

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <wincodec.h>
#include <stdio.h>
#include <string.h>
#include <string>
#include <unordered_map>
#include <vector>

#pragma comment( lib, "windowscodecs.lib" )
#pragma comment( lib, "ole32.lib" )

static const u32 _MAX_DIMENSION = 2048;

static BOOL _bScanned = FALSE;
static std::unordered_map<std::string, std::string> _Files;	// lower-case texture name -> image path
static u32 _nApplied = 0, _nRefused = 0;

static int _ExtensionRank( const char *pszExt ) {
	if( !_stricmp( pszExt, ".tga" ) ) return 0;
	if( !_stricmp( pszExt, ".png" ) ) return 1;
	if( !_stricmp( pszExt, ".bmp" ) ) return 2;
	if( !_stricmp( pszExt, ".jpg" ) || !_stricmp( pszExt, ".jpeg" ) ) return 3;
	return -1;
}

static void _ScanFolder( const std::string &sDir ) {
	WIN32_FIND_DATAA Find;
	HANDLE hFind = FindFirstFileA( ( sDir + "\\*" ).c_str(), &Find );
	if( hFind == INVALID_HANDLE_VALUE ) {
		return;
	}
	do {
		if( Find.cFileName[0] == '.' ) {
			continue;
		}
		const std::string sPath = sDir + "\\" + Find.cFileName;
		if( Find.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) {
			_ScanFolder( sPath );
			continue;
		}
		const char *pszDot = strrchr( Find.cFileName, '.' );
		const int nRank = pszDot ? _ExtensionRank( pszDot ) : -1;
		if( nRank < 0 ) {
			continue;
		}
		std::string sName( Find.cFileName, pszDot - Find.cFileName );
		for( char &c : sName ) c = (char)tolower( (unsigned char)c );
		auto It = _Files.find( sName );
		if( It == _Files.end() ) {
			_Files[sName] = sPath;
		} else {
			const char *pszOldDot = strrchr( It->second.c_str(), '.' );
			if( nRank < _ExtensionRank( pszOldDot ? pszOldDot : "" ) ) {
				It->second = sPath;
			}
		}
	} while( FindNextFileA( hFind, &Find ) );
	FindClose( hFind );
}

static void _Scan( void ) {
	_bScanned = TRUE;
	char szDir[MAX_PATH * 2];
	DWORD nLength = GetEnvironmentVariableA( "MA_PORT_TEXTURES", szDir, sizeof( szDir ) );
	if( nLength == 0 || nLength >= sizeof( szDir ) ) {
		nLength = GetModuleFileNameA( NULL, szDir, MAX_PATH );
		char *pszSlash = nLength ? strrchr( szDir, '\\' ) : NULL;
		if( !pszSlash ) {
			return;
		}
		strcpy( pszSlash, "\\textures" );
	}
	_ScanFolder( szDir );
	DEVPRINTF( "PORT-TEX: %u replacement images in '%s'.\n", (u32)_Files.size(), szDir );
}

// ---------------------------------------------------------------------------------------------------
// Decoding to 0xAARRGGBB pixels

static BOOL _ReadFile( const char *pszPath, std::vector<u8> &raData ) {
	FILE *pFile = fopen( pszPath, "rb" );
	if( !pFile ) {
		return FALSE;
	}
	fseek( pFile, 0, SEEK_END );
	const long nSize = ftell( pFile );
	fseek( pFile, 0, SEEK_SET );
	if( nSize <= 0 || nSize > 256 * 1024 * 1024 ) {
		fclose( pFile );
		return FALSE;
	}
	raData.resize( (size_t)nSize );
	const BOOL bOk = fread( raData.data(), 1, raData.size(), pFile ) == raData.size();
	fclose( pFile );
	return bOk;
}

// Truecolour or greyscale TGA, raw or run-length encoded.
static BOOL _DecodeTga( const char *pszPath, std::vector<u32> &raPixels, u32 *pnWidth, u32 *pnHeight ) {
	std::vector<u8> aData;
	if( !_ReadFile( pszPath, aData ) || aData.size() < 18 ) {
		return FALSE;
	}
	const u8 *p = aData.data();
	const u32 nIdLength = p[0], nColourMapType = p[1], nType = p[2];
	const u32 nWidth = p[12] | (p[13] << 8), nHeight = p[14] | (p[15] << 8), nBpp = p[16], nDescriptor = p[17];
	const BOOL bRle = nType == 10 || nType == 11;
	const BOOL bGrey = nType == 3 || nType == 11;
	if( nColourMapType != 0 || !( nType == 2 || nType == 3 || nType == 10 || nType == 11 ) || !nWidth || !nHeight ||
		nWidth > 8192 || nHeight > 8192 || !( bGrey ? nBpp == 8 : ( nBpp == 24 || nBpp == 32 ) ) ) {
		return FALSE;
	}
	const u32 nPixelBytes = nBpp / 8;
	size_t nAt = 18 + nIdLength;
	const u32 nCount = nWidth * nHeight;
	std::vector<u32> aRaw( nCount );
	auto ReadPixel = [&]( u32 *pOut ) -> BOOL {
		if( nAt + nPixelBytes > aData.size() ) return FALSE;
		const u8 *q = &aData[nAt];
		nAt += nPixelBytes;
		if( bGrey ) *pOut = 0xff000000u | ( q[0] << 16 ) | ( q[0] << 8 ) | q[0];
		else *pOut = ( nPixelBytes == 4 ? ( (u32)q[3] << 24 ) : 0xff000000u ) | ( q[2] << 16 ) | ( q[1] << 8 ) | q[0];
		return TRUE;
	};
	for( u32 i = 0; i < nCount; ) {
		if( !bRle ) {
			if( !ReadPixel( &aRaw[i++] ) ) return FALSE;
			continue;
		}
		if( nAt >= aData.size() ) return FALSE;
		const u8 nPacket = aData[nAt++];
		const u32 nRun = ( nPacket & 0x7f ) + 1;
		if( i + nRun > nCount ) return FALSE;
		if( nPacket & 0x80 ) {
			u32 nPixel;
			if( !ReadPixel( &nPixel ) ) return FALSE;
			for( u32 k = 0; k < nRun; k++ ) aRaw[i++] = nPixel;
		} else {
			for( u32 k = 0; k < nRun; k++ ) if( !ReadPixel( &aRaw[i++] ) ) return FALSE;
		}
	}
	// Rows are stored bottom-up unless descriptor bit 5 says top-down; right-to-left if bit 4.
	raPixels.resize( nCount );
	const BOOL bTopDown = ( nDescriptor & 0x20 ) != 0, bRightToLeft = ( nDescriptor & 0x10 ) != 0;
	for( u32 y = 0; y < nHeight; y++ ) {
		const u32 nSrcY = bTopDown ? y : nHeight - 1 - y;
		for( u32 x = 0; x < nWidth; x++ ) {
			raPixels[y * nWidth + x] = aRaw[nSrcY * nWidth + ( bRightToLeft ? nWidth - 1 - x : x )];
		}
	}
	*pnWidth = nWidth;
	*pnHeight = nHeight;
	return TRUE;
}

// PNG, JPG, BMP and anything else Windows can decode, as 32-bit BGRA (= 0xAARRGGBB).
static BOOL _DecodeWic( const char *pszPath, std::vector<u32> &raPixels, u32 *pnWidth, u32 *pnHeight ) {
	const HRESULT hrCom = CoInitializeEx( NULL, COINIT_MULTITHREADED );
	IWICImagingFactory *pFactory = NULL;
	IWICBitmapDecoder *pDecoder = NULL;
	IWICBitmapFrameDecode *pFrame = NULL;
	IWICBitmapSource *pBgra = NULL;
	BOOL bOk = FALSE;
	wchar_t wszPath[MAX_PATH * 2];
	if( MultiByteToWideChar( CP_ACP, 0, pszPath, -1, wszPath, MAX_PATH * 2 ) &&
		SUCCEEDED( CoCreateInstance( CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS( &pFactory ) ) ) &&
		SUCCEEDED( pFactory->CreateDecoderFromFilename( wszPath, NULL, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &pDecoder ) ) &&
		SUCCEEDED( pDecoder->GetFrame( 0, &pFrame ) ) &&
		SUCCEEDED( WICConvertBitmapSource( GUID_WICPixelFormat32bppBGRA, pFrame, &pBgra ) ) ) {
		UINT nWidth = 0, nHeight = 0;
		pBgra->GetSize( &nWidth, &nHeight );
		if( nWidth && nHeight && nWidth <= 8192 && nHeight <= 8192 ) {
			raPixels.resize( (size_t)nWidth * nHeight );
			if( SUCCEEDED( pBgra->CopyPixels( NULL, nWidth * 4, nWidth * nHeight * 4, (BYTE *)raPixels.data() ) ) ) {
				*pnWidth = nWidth;
				*pnHeight = nHeight;
				bOk = TRUE;
			}
		}
	}
	if( pBgra ) pBgra->Release();
	if( pFrame ) pFrame->Release();
	if( pDecoder ) pDecoder->Release();
	if( pFactory ) pFactory->Release();
	if( SUCCEEDED( hrCom ) ) CoUninitialize();
	return bOk;
}

// ---------------------------------------------------------------------------------------------------
// Scaling and mips

static u32 _NearestPow2( u32 n ) {
	u32 nPow = 1;
	while( nPow < n ) nPow <<= 1;
	// nearer of the powers either side
	return ( nPow > 1 && nPow - n > n - ( nPow >> 1 ) ) ? nPow >> 1 : nPow;
}

static u8 _Channel( u32 nPixel, u32 nShift ) { return (u8)( nPixel >> nShift ); }

// Bilinear resample to nW x nH.
static void _Resample( const std::vector<u32> &aSrc, u32 nSrcW, u32 nSrcH, std::vector<u32> &aDst, u32 nW, u32 nH ) {
	aDst.resize( (size_t)nW * nH );
	for( u32 y = 0; y < nH; y++ ) {
		const f32 fY = ( ( (f32)y + 0.5f ) * nSrcH / nH ) - 0.5f;
		const s32 nY0 = fY < 0.0f ? 0 : (s32)fY;
		const s32 nY1 = nY0 + 1 < (s32)nSrcH ? nY0 + 1 : nY0;
		const f32 fTy = fY < 0.0f ? 0.0f : fY - (f32)nY0;
		for( u32 x = 0; x < nW; x++ ) {
			const f32 fX = ( ( (f32)x + 0.5f ) * nSrcW / nW ) - 0.5f;
			const s32 nX0 = fX < 0.0f ? 0 : (s32)fX;
			const s32 nX1 = nX0 + 1 < (s32)nSrcW ? nX0 + 1 : nX0;
			const f32 fTx = fX < 0.0f ? 0.0f : fX - (f32)nX0;
			const u32 a = aSrc[nY0 * nSrcW + nX0], b = aSrc[nY0 * nSrcW + nX1], c = aSrc[nY1 * nSrcW + nX0], d = aSrc[nY1 * nSrcW + nX1];
			u32 nOut = 0;
			for( u32 nShift = 0; nShift < 32; nShift += 8 ) {
				const f32 fTop = _Channel( a, nShift ) + ( _Channel( b, nShift ) - _Channel( a, nShift ) ) * fTx;
				const f32 fBottom = _Channel( c, nShift ) + ( _Channel( d, nShift ) - _Channel( c, nShift ) ) * fTx;
				const f32 fValue = fTop + ( fBottom - fTop ) * fTy + 0.5f;
				nOut |= (u32)( fValue < 0.0f ? 0 : ( fValue > 255.0f ? 255 : (u32)fValue ) ) << nShift;
			}
			aDst[(size_t)y * nW + x] = nOut;
		}
	}
}

static BOOL _AnyTransparency( const u32 *pPixels, u32 nCount ) {
	for( u32 i = 0; i < nCount; i++ ) {
		if( ( pPixels[i] >> 24 ) != 0xff ) return TRUE;
	}
	return FALSE;
}

static BOOL _AllTransparent( const u32 *pPixels, u32 nCount ) {
	for( u32 i = 0; i < nCount; i++ ) {
		if( pPixels[i] >> 24 ) return FALSE;
	}
	return TRUE;
}

BOOL pctextures_Apply( FTexInfo_t *pTexInfo, void **ppPixels, u32 *pnBytes ) {
#if FANG_WINGC
	// Explicit raw-texture mods take precedence over a general HD pack.
	// All other pack replacements retain their existing priority and behavior.
	if (pTexInfo) {
		char name[16], path[1200];
		const int length = snprintf(name, sizeof(name), "%s.tga", pTexInfo->szName);
		if (length > 0 && length < sizeof(name) && pcmods_ResolveFile(name, path, sizeof(path))) return FALSE;
	}
#endif
	if( !_bScanned ) {
		_Scan();
	}
	if( _Files.empty() || !pTexInfo || !ppPixels || !*ppPixels ) {
		return FALSE;
	}
	std::string sName( pTexInfo->szName );
	for( char &c : sName ) c = (char)tolower( (unsigned char)c );
	const auto It = _Files.find( sName );
	if( It == _Files.end() ) {
		return FALSE;
	}
	const char *pszPath = It->second.c_str();

	std::vector<u32> aImage;
	u32 nW = 0, nH = 0;
	const char *pszDot = strrchr( pszPath, '.' );
	const BOOL bDecoded = ( pszDot && !_stricmp( pszDot, ".tga" ) ) ? _DecodeTga( pszPath, aImage, &nW, &nH ) : _DecodeWic( pszPath, aImage, &nW, &nH );
	if( !bDecoded ) {
		DEVPRINTF( "PORT-TEX: could not read '%s' for '%s'; using the original.\n", pszPath, pTexInfo->szName );
		++_nRefused;
		return FALSE;
	}

	const u32 nOrigW = pTexInfo->nTexelsAcross, nOrigH = pTexInfo->nTexelsDown;
	// Same shape as the original (texture coordinates are fractions of it), to within a pixel.
	if( (u64)nW * nOrigH > (u64)( nH + 1 ) * nOrigW || (u64)( nW + 1 ) * nOrigH < (u64)nH * nOrigW ) {
		DEVPRINTF( "PORT-TEX: '%s' is %ux%u, the original %ux%u: a different shape, so the original is used.\n",
				   pszPath, nW, nH, nOrigW, nOrigH );
		++_nRefused;
		return FALSE;
	}

	// A power of two no larger than _MAX_DIMENSION, keeping the original's proportions.
	u32 nNewW = _NearestPow2( nW ), nNewH = _NearestPow2( nH );
	while( nNewW > _MAX_DIMENSION || nNewH > _MAX_DIMENSION ) {
		nNewW = nNewW > 1 ? nNewW >> 1 : 1;
		nNewH = nNewH > 1 ? nNewH >> 1 : 1;
	}
	if( nNewW * nOrigH != nNewH * nOrigW ) {
		// rounding broke the ratio: follow the original's
		nNewH = (u32)( ( (u64)nNewW * nOrigH ) / nOrigW );
		if( !nNewH ) nNewH = 1;
		nNewH = _NearestPow2( nNewH );
	}
	std::vector<u32> aTop;
	if( nNewW != nW || nNewH != nH ) {
		_Resample( aImage, nW, nH, aTop, nNewW, nNewH );
	} else {
		aTop.swap( aImage );
	}

	// Alpha follows the original: none where it had none; its own where the replacement has none.
	// A replacement with no visible pixel at all (a dump of a colour plane whose alpha the game keeps
	// separately, such as the menu pointer's) counts as having none.
	const u32 *pOrig = (const u32 *)*ppPixels;
	const BOOL bOrigAlpha = _AnyTransparency( pOrig, nOrigW * nOrigH );
	const BOOL bNewAlpha = _AnyTransparency( aTop.data(), nNewW * nNewH )
		&& !( _AllTransparent( aTop.data(), nNewW * nNewH ) && !_AllTransparent( pOrig, nOrigW * nOrigH ) );
	if( !bOrigAlpha ) {
		for( u32 &n : aTop ) n |= 0xff000000u;
	} else if( !bNewAlpha ) {
		std::vector<u32> aAlpha;
		_Resample( std::vector<u32>( pOrig, pOrig + nOrigW * nOrigH ), nOrigW, nOrigH, aAlpha, nNewW, nNewH );
		for( size_t i = 0; i < aTop.size(); i++ ) aTop[i] = ( aTop[i] & 0x00ffffffu ) | ( aAlpha[i] & 0xff000000u );
	}

	// Full mip chain, each level a 2x2 box filter of the one above.
	u32 nLevels = 1, nBytes = nNewW * nNewH * 4;
	for( u32 w = nNewW, h = nNewH; ( w > 1 || h > 1 ) && nLevels < 16; nLevels++ ) {
		w = w > 1 ? w >> 1 : 1;
		h = h > 1 ? h >> 1 : 1;
		nBytes += w * h * 4;
	}
	u32 *pOut = (u32 *)fmem_Alloc( nBytes, 16 );
	if( !pOut ) {
		DEVPRINTF( "PORT-TEX: out of memory for '%s'; using the original.\n", pszPath );
		++_nRefused;
		return FALSE;
	}
	memcpy( pOut, aTop.data(), nNewW * nNewH * 4 );
	u32 *pPrev = pOut, nPrevW = nNewW, nPrevH = nNewH;
	for( u32 nLevel = 1; nLevel < nLevels; nLevel++ ) {
		const u32 w = nPrevW > 1 ? nPrevW >> 1 : 1, h = nPrevH > 1 ? nPrevH >> 1 : 1;
		u32 *pLevel = pPrev + nPrevW * nPrevH;
		for( u32 y = 0; y < h; y++ ) {
			for( u32 x = 0; x < w; x++ ) {
				const u32 x0 = x * 2 < nPrevW ? x * 2 : nPrevW - 1, x1 = x * 2 + 1 < nPrevW ? x * 2 + 1 : x0;
				const u32 y0 = y * 2 < nPrevH ? y * 2 : nPrevH - 1, y1 = y * 2 + 1 < nPrevH ? y * 2 + 1 : y0;
				const u32 a = pPrev[y0 * nPrevW + x0], b = pPrev[y0 * nPrevW + x1], c = pPrev[y1 * nPrevW + x0], d = pPrev[y1 * nPrevW + x1];
				u32 nOut = 0;
				for( u32 nShift = 0; nShift < 32; nShift += 8 ) {
					nOut |= ( ( (u32)_Channel( a, nShift ) + _Channel( b, nShift ) + _Channel( c, nShift ) + _Channel( d, nShift ) + 2 ) / 4 ) << nShift;
				}
				pLevel[y * w + x] = nOut;
			}
		}
		pPrev = pLevel;
		nPrevW = w;
		nPrevH = h;
	}

	pTexInfo->nTexelsAcross = (u16)nNewW;
	pTexInfo->nTexelsDown = (u16)nNewH;
	pTexInfo->nLodCount = (u8)nLevels;
	*ppPixels = pOut;
	*pnBytes = nBytes;
	++_nApplied;
	DEVPRINTF( "PORT-TEX: '%s' %ux%u (original %ux%u) from '%s'.\n", pTexInfo->szName, nNewW, nNewH, nOrigW, nOrigH, pszPath );
	return TRUE;
}
