/*********************************************************************NVMH2****
Path:  C:\Dev\devrel\Nv_sdk_4\Dx7\NormalMapGen
File:  NormalMapGen.cpp

Copyright (C) 1999, 2000 NVIDIA Corporation
This file is provided without support, instruction, or implied warranty of any
kind.  NVIDIA makes no guarantee of its fitness for a particular purpose and is
not liable under any circumstances for any damages or loss whatsoever arising
from the use or inability to use this file or items derived from it.

Comments:

     Command-line driven program for creating normal maps from input height
map images.

Usage:
Usage:  NormalMapGen <filename> <outfilename> [float scale] [+w,-w] [3x3,5x5]\n");

4/1/01
Updated with 3x3 convolution and 5x5 convolution for deriving slopes from
height values.  Uses image library convolution class which accepts a8r8g8b8
format data.


The program loads & operates on 24 or 32 bit targa files.
If 24 bit, the height is derived from color and written to a 32-bit format
with alpha.  
If 32-bit, height is derived from alpha.

It saves 32 bit targa files.


******************************************************************************/




//#include <windows.h>
#include <assert.h>

#include "NormalMapGen.h"

#include "NV_Common.h"

//#include "NV_Error.h"
#include "NVI_ImageLib.h"



NVI_TGA_File in;
NVI_TGA_File out;
  


//////////////////////////////////////////////////////////////////////
// Function prototypes

void MakeKernelElems( const float * pInWeightArray, int num_x, int num_y,
						ConvolutionKernelElement * pOutArray );
void RotateArrayCCW( const float * pInArray, int num_x, int num_y, float * pOutArray );


void ConvertAlphaToNormalMap_4x( float scale, BOOL wrap );
void ConvertAlphaToNormalMap_3x3( float scale, BOOL wrap );
void ConvertAlphaToNormalMap_5x5( float scale, BOOL wrap );
void ConvertAlphaToNormalMap_7x7( float scale, BOOL wrap );
void ConvertAlphaToNormalMap_9x9( float scale, BOOL wrap );


//////////////////////////////////////////////////////////


void 	CalculateNormalMap( ConvolutionKernel * pKernels, int num_kernels, float scale, BOOL wrap )
{
	// pKernels must be an array of at least two kernels
	// The first kernel extracts dh/dx  (change in height with respect to x )
	// the second extracts dh/dy  (change in height with respect to y )

	assert( pKernels != NULL );
	assert( num_kernels == 2 );

	float			     results[2];

	////////////////////////////////////////////////////////
	// Set up the convolver & prepare image data

	Convolver	conv;

	NVI_Image	** pSrc = in.GetImage();
	assert( *pSrc != NULL );


	conv.Initialize( pSrc, pKernels, num_kernels, wrap & 0x1 );

	int size_y, size_x;

	size_x = (int) (*pSrc)->GetWidth();
	size_y = (int) (*pSrc)->GetHeight();
	DWORD * pArray = (DWORD*) (*pSrc)->GetImageDataPointer();

	assert( pArray != NULL );

	/////////////////////////////////////////////////////
	// Now run the kernel over the source image area
	//  and write out the values.

	int				i,j;
	float			du, dv;
	unsigned long	nmap_color;
	char	height;

	// coordinates of source image (not padded)
	for( j=0; j < size_y; j++ )
	{
		for( i=0; i < size_x; i++ )
		{
			// apply kernels

			conv.Convolve_Alpha_At( i,j, results, 2 );

			du = results[0] * scale;
			dv = results[1] * scale;

			// det  | x  y  z |
			//      | 1  0 du |
			//      | 0  1 dv |
			//
			// cross product gives (-du, -dv, 1.0) as normal 

			float mag = du*du + dv*dv + 1.0f;
			mag = (float) sqrt( mag );

			// Get alpha as height
			height = (char) ( pArray[ j * size_x + i ] ) >> 24;

			AlphaAndVectorToARGB( height, -du/mag, -dv/mag, 1.0f / mag, nmap_color );


			pArray[ j * size_x + i ] = nmap_color;
		}
	}
}



void ConvertAlphaToNormalMap_4x( float scale, BOOL wrap )
{
	// Do the conversion using a 4 sample nearest neighbor
	//  pattern

	// d height / du kernel:
	//  0		0		0
	//	-1/2	0		1/2
	//	0		0		0

	// d height / dv kernel:
	//	0	1/2		0
	//	0	0		0
	//	0	-1/2	0

	int numelem;				// num elements in each kernel


	numelem = 2;
	ConvolutionKernelElement	du_elem[] =	{
		{-1, 0, -1.0f/2.0f}, { 1, 0, 1.0f/2.0f}			};


	ConvolutionKernelElement	dv_elem[] = {
			{ 0,  1,  1.0f/2.0f},
			{ 0, -1, -1.0f/2.0f}			 };

	
	int num_kernels = 2;
	ConvolutionKernel	 kernels[2];

	kernels[0].SetElements( numelem, du_elem );
	kernels[1].SetElements( numelem, dv_elem );


	///////////////////////////////////////////////////////
	//  Calc ARGB normal map & write to the "in." file

	CalculateNormalMap( kernels, num_kernels, scale, wrap );


}


void ConvertAlphaToNormalMap_3x3( float scale, BOOL wrap )
{
	// Uses Anders' 3x3 kernels for transforming height 
	//  into a normal map vector.
	// Either wraps or does not wrap.
	// The convolver class memcopies to larger surface (width + kernel width-1, 
	//		height + kernel heigh - 1 ) to make wrap code easy for arbitrary
	//		kernels.  Edge texels are duplicated into the border regions or
	//      copied from the other side of the source image if wrapping is on.


	int numelem;				// num elements in each kernel

	float wt = 1.0f / 6.0f;

	// Kernel for change of height in u axis:
	//  -1/6	0	1/6
	//  -1/6	0	1/6
	//  -1/6	0	1/6

	numelem = 6;
	ConvolutionKernelElement	du_elem[] =	{
		{-1, 1, -wt}, { 1, 1, wt},
		{-1, 0, -wt}, { 1, 0, wt},
		{-1,-1, -wt}, { 1,-1, wt}	};

	// Kernel for change of height in v axis:
	//						 1,1
	//   1/6	 1/6	 1/6
	//     0	   0	   0
	//  -1/6	-1/6	-1/6
	// 0,0

	ConvolutionKernelElement	dv_elem[] = {
		{-1, 1,  wt}, { 0, 1, wt}, { 1, 1,  wt },
		{-1,-1, -wt}, { 0,-1,-wt}, { 1,-1, -wt } };

	
	int num_kernels = 2;
	ConvolutionKernel	 kernels[2];

	kernels[0].SetElements( numelem, du_elem );
	kernels[1].SetElements( numelem, dv_elem );


	///////////////////////////////////////////////////////
	//  Calc ARGB normal map & write to the "in." file

	CalculateNormalMap( kernels, num_kernels, scale, wrap );

}



void ConvertAlphaToNormalMap_5x5( float scale, BOOL wrap )
{
	// Either wraps or does not wrap.
	// The convolver class memcopies to larger surface (width + kernel width-1, 
	//		height + kernel heigh - 1 ) to make wrap code easy for arbitrary
	//		kernels.  Edge texels are duplicated into the border regions or
	//      copied from the other side of the source image if wrapping is on.

	int numelem;				// num elements in each kernel


	float wt1 = 1.0f / 6.0f;
	float wt2 = 1.0f / 48.0f;

/*
	// Kernel for change of height in u axis:
	// The are cubic coefs for interpolation with sample 
	//  points at -2, -1, 1, and 2, hence the 0 coefs along
	//  center axis.  Resulting pattern is undesirable for 
	//  outstanding points in the height field.  These become
	//  a region of 4 mounds, when they should instead become a 
	//  smooth blob.

	//	1/48	-1/6	0	1/6		-1/48
	//	1/48	-1/6	0	1/6		-1/48
	//	0		0		0	0		0
	//	1/48	-1/6	0	1/6		-1/48
	//	1/48	-1/6	0	1/6		-1/48


	numelem = 16;
	ConvolutionKernelElement	du_elem[] =	{
		{-2, 2, wt2}, {-1,2,-wt1},    {1,2,wt1}, {2,2,-wt2},
		{-2, 1, wt2}, {-1,1,-wt1},    {1,1,wt1}, {2,1,-wt2},

		{-2,-1, wt2}, {-1,-1,-wt1},   {1,-1,wt1}, {2,-1,-wt2},
		{-2,-2, wt2}, {-1,-2,-wt1},   {1,-2,wt1}, {2,-2,-wt2}   };

	// Kernel for change of height in v axis:
	// This is same as u kernel above - just rotated 90 degrees

	ConvolutionKernelElement	dv_elem[] = {
		{-2, 2,-wt2}, {-1,2,-wt2},    {1,2,-wt2}, {2,2,-wt2},
		{-2, 1, wt1}, {-1,1, wt1},    {1,1, wt1}, {2,1, wt1},

		{-2,-1,-wt1}, {-1,-1,-wt1},   {1,-1,-wt1}, {2,-1,-wt1},
		{-2,-2, wt2}, {-1,-2, wt2},   {1,-2, wt2}, {2,-2, wt2}   };
*/


	

	numelem = 20;
	float wt22 = 1.0f/16.0f;
	float wt12 = 1.0f/10.0f;
	float wt02 = 1.0f/8.0f;
	float wt11 = 1.0f/2.8f;

	// Kernels using slope based on distance of that point from the 0,0
	// This is not from math derivation, but makes nice result

	ConvolutionKernelElement	du_elem[] =	{
		{-2, 2,-wt22 }, {-1, 2,-wt12 },   {1, 2, wt12 }, {2, 2, wt22 },
		{-2, 1,-wt12 }, {-1, 1,-wt11 },   {1, 1, wt11 }, {2, 1, wt12 },

		{-2, 0,-wt02 }, {-1, 0,-1.0f/2.0f },   {1, 0, 1.0f/2.0f }, {2, 0, wt02 },

		{-2,-1,-wt12 }, {-1,-1,-wt11 },   {1,-1, wt11 }, {2,-1, wt12 },
		{-2,-2,-wt22 }, {-1,-2,-wt12 },   {1,-2, wt12 }, {2,-2, wt22 }   };

	ConvolutionKernelElement	dv_elem[] = {
		{-2, 2, wt22 }, {-1, 2, wt12 }, {0, 2, 1.0f/4.0f },  {1, 2, wt12 }, {2, 2, wt22 },
		{-2, 1, wt12 }, {-1, 1, wt11 }, {0, 1, 1.0f/2.0f },  {1, 1, wt11 }, {2, 1, wt12 },

		{-2,-1,-wt12 }, {-1,-1,-wt11 }, {0,-1,-1.0f/2.0f },  {1,-1,-wt11 }, {2,-1,-wt12 },
		{-2,-2,-wt22 }, {-1,-2,-wt12 }, {0,-2,-1.0f/4.0f },  {1,-2,-wt12 }, {2,-2,-wt22 }   };


	// normalize the kernel so abs of all 
	// weights add to one
		
	int i;
	float usum, vsum;
	usum = vsum = 0.0f;
	for( i=0; i < numelem; i++ )
	{
		usum += (float) fabs( du_elem[i].weight );
		vsum += (float) fabs( dv_elem[i].weight );
	}
	for( i=0; i < numelem; i++ )
	{
		du_elem[i].weight /= usum;
		dv_elem[i].weight /= vsum;
	}



	int num_kernels = 2;
	ConvolutionKernel	 kernels[2];

	kernels[0].SetElements( numelem, du_elem );
	kernels[1].SetElements( numelem, dv_elem );


	///////////////////////////////////////////////////////
	//  Calc ARGB normal map & write to the "in." file

	CalculateNormalMap( kernels, num_kernels, scale, wrap );

}


void MakeKernelElems( const float * pInWeightArray, int num_x, int num_y,
						ConvolutionKernelElement * pOutArray )
{
	// This makes coordinates for an array of weights, assumed to 
	//  be a rectangle.
	//
	// You must allocate pOutArray outside the function!
	// num_x and num_y should be odd
	//
	// Specify elems from  upper left corner (-num_x/2, num_y/2) to
	//   lower right corner. 
	// This generates the coordinates of the samples for you
	// For example:
	//  elem_array[] = { 00, 11, 22, 33, 44, 55, 66, 77, 88 }
	//  MakeKernelsElems( elem_array, 3, 3 )
	//  would make:
	//
	//   { -1, 1, 00 }  { 0, 1, 11 }  { 1, 1, 22 }
	//   { -1, 0, 33 }  { 0, 0, 44 }  { 1, 0, 55 }
	//   { -1, -1, 66}  ..etc 
	//
	//  As the array of kernel elements written to pOutArray
	//
	
	assert( pOutArray != NULL );
	assert( pInWeightArray != NULL );

	int i,j;
	int ind;

	for( j=0; j < num_y; j++ )
	{
		for( i=0; i < num_x; i++ )
		{
			ind = i + j * num_x;
			
			assert( ind < num_x * num_y );

			pOutArray[ ind ].x_offset =  (int)( i - num_x / 2 );
			pOutArray[ ind ].y_offset =  (int)( num_y / 2 - j );
			pOutArray[ ind ].weight   = pInWeightArray[ind];
		}
	}
}

void 	RotateArrayCCW( float * pInArray, int num_x, int num_y, float * pOutArray )
{
	// rotate an array of floats 90 deg CCW, so
	//    1, 0
	//    2, 3
	//  becomes
	//    0, 3
	//    1, 2

	assert( pOutArray != NULL );
	assert( pInArray != NULL );

	int i,j;
	int newi, newj;

	float * pSrc;

	////////////////////////////////////////////////////
	// If arrays are same, copy source to new temp array

	if( pInArray == pOutArray )
	{
		pSrc = new float[ num_x * num_y ];
		assert( pSrc != NULL );

		for( i=0; i < num_x * num_y; i++ )
		{
			pSrc[i] = pInArray[ i ];
		}
	}
	else
	{
		pSrc = pInArray;
	}
	////////////////////////////////////////////////////

	for( j=0; j < num_y; j++ )
	{
		for( i=0; i < num_x; i++ )
		{
			newj = num_x - i - 1;
			newi = j;

			// rotate dims of array too ==>  j * num_y

			pOutArray[ newi + newj * num_y ] = pSrc[ i + j * num_x ];
		}
	}


	if( pInArray == pOutArray )
	{
		SAFE_ARRAY_DELETE( pSrc );
	}	
}


void ConvertAlphaToNormalMap_7x7( float scale, BOOL wrap )
{
	// Either wraps or does not wrap.
	// The convolver class memcopies to larger surface (width + kernel width-1, 
	//		height + kernel heigh - 1 ) to make wrap code easy for arbitrary
	//		kernels.  Edge texels are duplicated into the border regions or
	//      copied from the other side of the source image if wrapping is on.

	int numelem;				// num elements in each kernel


	/////////////////////////////////////////
	// Kernel for change of height in u axis:
	// A Sobel filter kernel

	numelem = 49;


	float du_f[] = {
			-1, -2, -3,  0,  3, 2, 1,
			-2, -3, -4,  0,  4, 3, 2,
			-3, -4, -5,  0,  5, 4, 3,
			-4, -5, -6,  0,  6, 5, 4,
			-3, -4, -5,  0,  5, 4, 3,
			-2, -3, -4,  0,  4, 3, 2,
			-1, -2, -3,  0,  3, 2,	1   };



	ConvolutionKernelElement	du_elem[49];
	
	MakeKernelElems( du_f, 7, 7, & (du_elem[0] ) );

	/////////////////////////////////////////////
	// Kernel for change of height in v axis:

	float dv_f[49];
	
	RotateArrayCCW( &( du_f[0] ), 7, 7,  &( dv_f[0] ) );

	ConvolutionKernelElement	dv_elem[49];
	
	MakeKernelElems( dv_f, 7, 7, & ( dv_elem[0] ) );

	/////////////////////////////////////////////

	///////////////////////////////////////////
	// normalize the kernels so abs of all 
	// weights add to one
		
	int i;
	float usum, vsum;
	usum = vsum = 0.0f;

	for( i=0; i < numelem; i++ )
	{
		usum += (float) fabs( du_elem[i].weight );
		vsum += (float) fabs( dv_elem[i].weight );
	}

	for( i=0; i < numelem; i++ )
	{
		du_elem[i].weight /= usum;
		dv_elem[i].weight /= vsum;
	}

	///////////////////////////////////////////
	int num_kernels = 2;
	ConvolutionKernel	 kernels[2];

	kernels[0].SetElements( numelem, du_elem );
	kernels[1].SetElements( numelem, dv_elem );


	///////////////////////////////////////////////////////
	//  Calc ARGB normal map & write to the "in." file

	CalculateNormalMap( kernels, num_kernels, scale, wrap );

}


void ConvertAlphaToNormalMap_9x9( float scale, BOOL wrap )
{
	// Either wraps or does not wrap.
	// The convolver class memcopies to larger surface (width + kernel width-1, 
	//		height + kernel heigh - 1 ) to make wrap code easy for arbitrary
	//		kernels.  Edge texels are duplicated into the border regions or
	//      copied from the other side of the source image if wrapping is on.
	//

	int numelem;				// num elements in each kernel


	/////////////////////////////////////////
	// Kernel for change of height in u axis:
	// A Sobel filter kernel

	numelem = 81;

	float du_f[] = {
			-1, -2, -3, -4,  0,  4, 3, 2, 1,
			-2, -3, -4, -5,  0,  5, 4, 3, 2,
			-3, -4, -5, -6,  0,  6, 5, 4, 3,
			-4, -5, -6, -7,  0,  7, 6, 5, 4,
			-5, -6, -7, -8,  0,  8, 7, 6, 5,
			-4, -5, -6, -7,  0,  7, 6, 5, 4,
			-3, -4, -5, -6,  0,  6, 5, 4, 3,
			-2, -3, -4, -5,  0,  5, 4, 3, 2,
			-1, -2, -3, -4,  0,  4, 3, 2, 1		};


	ConvolutionKernelElement	du_elem[81];
	
	MakeKernelElems( du_f, 9, 9, & (du_elem[0] ) );

	/////////////////////////////////////////////
	// Kernel for change of height in v axis:

	float dv_f[81];
	
	RotateArrayCCW( &( du_f[0] ), 9, 9,  &( dv_f[0] ) );

	ConvolutionKernelElement	dv_elem[81];
	
	MakeKernelElems( dv_f, 9, 9, & ( dv_elem[0] ) );

	/////////////////////////////////////////////

	///////////////////////////////////////////
	// normalize the kernels so abs of all 
	// weights add to one
		
	int i;
	float usum, vsum;
	usum = vsum = 0.0f;

	for( i=0; i < numelem; i++ )
	{
		usum += (float) fabs( du_elem[i].weight );
		vsum += (float) fabs( dv_elem[i].weight );
	}
	for( i=0; i < numelem; i++ )
	{
		du_elem[i].weight /= usum;
		dv_elem[i].weight /= vsum;
	}

	///////////////////////////////////////////
	int num_kernels = 2;
	ConvolutionKernel	 kernels[2];

	kernels[0].SetElements( numelem, du_elem );
	kernels[1].SetElements( numelem, dv_elem );


	///////////////////////////////////////////////////////
	//  Calc ARGB normal map & write to the "in." file

	CalculateNormalMap( kernels, num_kernels, scale, wrap );

}




void ConvertColorToAlphaHeight()
{

	// Take the source image pixels and write their average
	// r,g,b value to the alpha channel of the image.

	NVI_Image ** pImg;
	pImg = in.GetImage();

	assert( *pImg != NULL );

	(*pImg)->AverageRGBToAlpha();
	
}


// checks to see if there is an alpha in set any where
BOOL IsAlphaSet() {

    DWORD * pArray = in.GetDataPointer();
    
    DWORD dwWidth = in.GetWidth();
    DWORD dwHeight = in.GetHeight();
    
    DWORD i, color, first;

    if (pArray == 0)
        return false;

    first = *pArray & 0xFF000000;
    
    for ( i = 0; i < dwHeight * dwWidth; i++  )
    {
        color = *pArray++;
        if ((color & 0xFF000000) != first)
        {
            return true;
        }
    }

    return false;
}

BOOL ConvertTgaToNormMap( const char *pszInFile,
						 const char *pszOutFile,
						 KernelType	kerneltype/*=KERNEL_3x3*/,
						 BOOL wrap/*=TRUE*/,
						 float scale/*=0.0f*/ ) {
	int res2;
	unsigned long * result;
	UINT w, h;
	float width;

	if( !pszInFile || !pszOutFile ) {
		return false;
	}	

	result = in.ReadFile( (char *)pszInFile, UL_GFX_PAD );
	if( result == 0 ) {
		// File is not found or can't be read
		return false;
	}

	width = (float)in.GetWidth();	
	
	// scale can be less than one !
	if( scale == 0.0f ) {
		if( width > 0.0f ) 	{
			// Setting bump scale from image size
			scale = 600.0f / width;
		} else {
			// Scale not specified and image width == 0 !?  Setting scale = 5.0f\n");
			scale = 5.0f;
		}
	}
	
	if( IsAlphaSet() == false ) {
		ConvertColorToAlphaHeight();
	}

	switch( kerneltype )
	{
	case KERNEL_9x9:
		ConvertAlphaToNormalMap_9x9( scale, wrap );
		break;

	case KERNEL_7x7:
		ConvertAlphaToNormalMap_7x7( scale, wrap );
		break;

	case KERNEL_5x5:
		ConvertAlphaToNormalMap_5x5( scale, wrap );
		break;

	case KERNEL_4x:
		ConvertAlphaToNormalMap_4x( scale, wrap );
		break;

	case KERNEL_3x3:
	default:
		ConvertAlphaToNormalMap_3x3( scale, wrap );
		break;
	}
	
	w = in.GetWidth();
	h = in.GetHeight();

	res2 = out.WriteFile( pszOutFile, (unsigned char *)in.GetDataPointer(), w, h, 32, 32, 0 );
	if( res2 == 0 ) {
		// **** Error writing file
		return false;
	}

	return true;
}

