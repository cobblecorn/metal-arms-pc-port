/*********************************************************************NVMH2****
Path:  D:\Dev\devrel\Nv_sdk_4\CommonSrc\nvImageLib
File:  NVI_ColorUtils.h

Copyright (C) 1999, 2000 NVIDIA Corporation
This file is provided without support, instruction, or implied warranty of any
kind.  NVIDIA makes no guarantee of its fitness for a particular purpose and is
not liable under any circumstances for any damages or loss whatsoever arising
from the use or inability to use this file or items derived from it.

Comments:
Utilty functions for working with colors



******************************************************************************/


#ifndef __NVICOLORUTILS_H__
#define __NVICOLORUTILS_H__


#include <math.h>
#include <assert.h>
//#include <windows.h>



// BYTEs are unsigned




__forceinline
void AlphaAndVectorToARGB( BYTE alpha, float red, float green, float blue, DWORD & outColor )
{
	// Map the range [-1,1] to [0,255]
	// Does not clamp for you

	BYTE r = (BYTE) (( red   + 1.0f ) * 127.5f);
	BYTE g = (BYTE) (( green + 1.0f ) * 127.5f);
	BYTE b = (BYTE) (( blue  + 1.0f ) * 127.5f);

	//a fix for incorrect alpha mucking everything up.
	alpha = 0xff;
	//

	outColor = ( ( alpha << 24 ) | ( r << 16 ) | ( g << 8 ) | ( b << 0 ) );
}


__forceinline
void RGBToFloat( DWORD inColor, float * outAlpha, float * outRed, float * outGreen, float * outBlue )
{
	// Converts RGB [0,255] to float [0,1]

	*outAlpha	= ((float)(  inColor >> 24				  )) / 255.0f;
	*outRed		= ((float)( (inColor >> 16 ) & 0x000000FF )) / 255.0f;
	*outGreen	= ((float)( (inColor >> 24 ) & 0x000000FF )) / 255.0f;
	*outBlue	= ((float)( (inColor >> 24 ) & 0x000000FF )) / 255.0f;
}


__forceinline
void RGBToFloatVector( DWORD inColor, float * outAlpha, float * outRed, float * outGreen, float * outBlue )
{
	// Converts RGB [0,255] to float [-1,1]
	// Use if RGB is a vector from a normal map

	RGBToFloat( inColor, outAlpha, outRed, outGreen, outBlue );
	*outAlpha	= ( *outAlpha	* 2.0f ) - 1.0f;
	*outRed		= ( *outRed		* 2.0f ) - 1.0f;
	*outGreen	= ( *outGreen	* 2.0f ) - 1.0f;
	*outBlue	= ( *outBlue	* 2.0f ) - 1.0f;

}







#endif
