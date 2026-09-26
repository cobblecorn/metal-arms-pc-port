// xgraphics.h - stand-in for the Xbox SDK's XGraphics helpers used by Fang.
//
// The Xbox stores textures in swizzled (Morton / Z-order) layout. Fang's DX code
// includes this header on Windows too and calls XGUnswizzleRect() for textures
// flagged FTEX_FLAG_LOAD_IN_PLACE. Only the 2D rect (un)swizzle routines are
// provided; they are what the Windows path needs.
#ifndef _PORT_XGRAPHICS_H_
#define _PORT_XGRAPHICS_H_ 1

#include "d3d8.h"
#include <string.h>

// Bit masks that place x/y bits in Xbox swizzle order: bits alternate x,y,x,y...
// until the smaller dimension runs out, then the remaining high bits of the
// larger dimension continue linearly.
inline void XGCompat_SwizzleMasks( DWORD nWidth, DWORD nHeight, DWORD *pMaskU, DWORD *pMaskV )
{
	DWORD nBitsU = 0, nBitsV = 0;
	while( (1u << nBitsU) < nWidth )  nBitsU++;
	while( (1u << nBitsV) < nHeight ) nBitsV++;

	DWORD nMaskU = 0, nMaskV = 0, nBit = 1, iu = 0, iv = 0;
	while( iu < nBitsU || iv < nBitsV )
	{
		if( iu < nBitsU ) { nMaskU |= nBit; nBit <<= 1; iu++; }
		if( iv < nBitsV ) { nMaskV |= nBit; nBit <<= 1; iv++; }
	}
	*pMaskU = nMaskU;
	*pMaskV = nMaskV;
}

// Copy a linear (pitched) image into swizzled layout (or back, when bUnswizzle).
inline void XGCompat_SwizzleCopy( const void *pSrc, DWORD nWidth, DWORD nHeight, void *pDst, DWORD nLinearPitch, DWORD nBytesPerPixel, BOOL bUnswizzle )
{
	DWORD nMaskU, nMaskV;
	XGCompat_SwizzleMasks( nWidth, nHeight, &nMaskU, &nMaskV );
	if( nLinearPitch == 0 ) nLinearPitch = nWidth * nBytesPerPixel;

	DWORD nV = 0;
	for( DWORD y = 0; y < nHeight; y++ )
	{
		DWORD nU = 0;
		for( DWORD x = 0; x < nWidth; x++ )
		{
			DWORD nSwizzled = (nU | nV) * nBytesPerPixel;
			DWORD nLinear = y * nLinearPitch + x * nBytesPerPixel;
			if( bUnswizzle )
				memcpy( (BYTE *)pDst + nLinear, (const BYTE *)pSrc + nSwizzled, nBytesPerPixel );
			else
				memcpy( (BYTE *)pDst + nSwizzled, (const BYTE *)pSrc + nLinear, nBytesPerPixel );
			nU = (nU - nMaskU) & nMaskU;		// step to the next x in swizzle order
		}
		nV = (nV - nMaskV) & nMaskV;			// step to the next y
	}
}

// Signatures match the Xbox SDK. Only full-image (NULL rect / NULL point) is supported.
inline void XGUnswizzleRect( const void *pSrcData, DWORD nWidth, DWORD nHeight, const RECT *, void *pDestData, DWORD nPitch, const POINT *, DWORD nBytesPerPixel )
{
	XGCompat_SwizzleCopy( pSrcData, nWidth, nHeight, pDestData, nPitch, nBytesPerPixel, TRUE );
}

inline void XGSwizzleRect( const void *pSrcData, DWORD nPitch, const RECT *, void *pDestData, DWORD nWidth, DWORD nHeight, const POINT *, DWORD nBytesPerPixel )
{
	XGCompat_SwizzleCopy( pSrcData, nWidth, nHeight, pDestData, nPitch, nBytesPerPixel, FALSE );
}

#endif
