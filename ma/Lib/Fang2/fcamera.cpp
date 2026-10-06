//////////////////////////////////////////////////////////////////////////////////////
// fcamera.cpp - 
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
// 01/14/02 Starich     Created.
//////////////////////////////////////////////////////////////////////////////////////
#include "fang.h"
#include "fcamera.h"
#include "frenderer.h"
#include "fvid.h"
#include "fviewport.h"
#include "fres.h"
#include "fclib.h"

#if !FANG_PRODUCTION_BUILD
#include "fdraw.h"
#endif

//====================
// private definitions

#if FANG_RELEASE_BUILD
	#pragma message( "FCAMERA.CPP - DEBUG CAMERA NON-FUNCTIONAL IN RELEASE MODE ON GC.  CAMERA GEO STRUCTURES CAUSE COMPILER TO CRASH.\n" )
#endif

#if FANG_PRODUCTION_BUILD
// never enable the camera icon on a production release, it wastes memory
	#define _ENABLE_DEBUG_CAMERA_ICON		FALSE // set to FALSE to disable the drawing of the camera icon.  This will save some memory!!!
#else
	#if FANG_PLATFORM_GC && !FANG_DEBUG_BUILD
		#define _ENABLE_DEBUG_CAMERA_ICON	FALSE // set to FALSE to disable the drawing of the camera icon.  This will save some memory!!!
	#else
		#define _ENABLE_DEBUG_CAMERA_ICON	TRUE // set to FALSE to disable the drawing of the camera icon.  This will save some memory!!!
	#endif
#endif


#define _NUM_TRIS						106
#define _NUM_CAM_VERTS					( _NUM_TRIS * 3 )
#define _CAM_RED						( 175.0f / 255.0f )
#define _CAM_GREEN						( 180.0f / 255.0f )
#define _CAM_BLUE						( 185.0f / 255.0f ) 
#define _CAM_ALPHA						0.65f
#define _NUM_LINE_VERTS					156
#define _LINE_RED						( 0.0f / 255.0f )
#define _LINE_GREEN						( 0.0f / 255.0f )
#define _LINE_BLUE						( 0.0f / 255.0f )
#define _LINE_ALPHA						1.0f
#define _S								0.0f
#define _T								0.0f

#define _DEFAULT_CAM_X_FOV				FMATH_QUARTER_PI
#define _DEFAULT_CAM_Y_FOV				0.0f
#define _DEFAULT_CAM_NEAR_Z				0.1f
#define _DEFAULT_CAM_FAR_Z				1000.0f

//=================
// public variables

//==================
// private variables

#if _ENABLE_DEBUG_CAMERA_ICON

static const FDrawVtx_t _aCameraIconVtxs[_NUM_CAM_VERTS] = {
	FDrawVtx_t( 0.361f,		0.246f,		-0.648f,	_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),
	FDrawVtx_t( 0.437f,		0.436f,		0.972f,		_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),
	FDrawVtx_t( -0.361f,	0.246f,		-0.648f,	_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.437f,	0.436f,		0.972f,		_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.361f,	0.246f,		-0.648f,	_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.437f,		0.436f,		0.972f,		_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.437f,	-0.859f,	0.972f,		_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.437f,		-0.859f,	0.972f,		_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.361f,	-0.669f,	-0.648f,	_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.361f,		-0.669f,	-0.648f,	_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.361f,	-0.669f,	-0.648f,	_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.437f,		-0.859f,	0.972f,		_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.437f,	-0.859f,	0.972f,		_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.437f,	0.436f,		0.972f,		_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.437f,		-0.859f,	0.972f,		_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.437f,	0.436f,		0.972f,		_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.437f,		-0.859f,	0.972f,		_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.437f,		0.436f,		0.972f,		_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.361f,	0.246f,		-0.648f,	_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.437f,	0.436f,		0.972f,		_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.361f,	-0.669f,	-0.648f,	_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.437f,	-0.859f,	0.972f,		_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.361f,	-0.669f,	-0.648f,	_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.437f,	0.436f,		0.972f,		_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.361f,		0.246f,		-0.648f,	_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.361f,	0.246f,		-0.648f,	_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.361f,		-0.669f,	-0.648f,	_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.361f,	-0.669f,	-0.648f,	_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.361f,		-0.669f,	-0.648f,	_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.361f,	0.246f,		-0.648f,	_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.437f,		0.436f,		0.972f,		_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.361f,		0.246f,		-0.648f,	_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.437f,		-0.859f,	0.972f,		_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.361f,		-0.669f,	-0.648f,	_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.437f,		-0.859f,	0.972f,		_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.361f,		0.246f,		-0.648f,	_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.247f,	-0.105f,	0.962f,		_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.319f,	0.122f,		1.543f,		_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.393f,	-0.105f,	1.543f,		_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.247f,	-0.105f,	0.962f,		_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.201f,	0.037f,		0.962f,		_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.319f,	0.122f,		1.543f,		_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.201f,	0.037f,		0.962f,		_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.126f,	0.263f,		1.543f,		_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.319f,	0.122f,		1.543f,		_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.201f,	0.037f,		0.962f,		_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.081f,	0.124f,		0.962f,		_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.126f,	0.263f,		1.543f,		_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.081f,	0.124f,		0.962f,		_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.113f,		0.263f,		1.543f,		_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.126f,	0.263f,		1.543f,		_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.081f,	0.124f,		0.962f,		_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.068f,		0.124f,		0.962f,		_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.113f,		0.263f,		1.543f,		_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.068f,		0.124f,		0.962f,		_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.306f,		0.122f,		1.543f,		_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.113f,		0.263f,		1.543f,		_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.068f,		0.124f,		0.962f,		_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.188f,		0.037f,		0.962f,		_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.306f,		0.122f,		1.543f,		_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.188f,		0.037f,		0.962f,		_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.380f,		-0.105f,	1.543f,		_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.306f,		0.122f,		1.543f,		_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.188f,		0.037f,		0.962f,		_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.234f,		-0.105f,	0.962f,		_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.380f,		-0.105f,	1.543f,		_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.234f,		-0.105f,	0.962f,		_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.306f,		-0.332f,	1.543f,		_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.380f,		-0.105f,	1.543f,		_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.234f,		-0.105f,	0.962f,		_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.188f,		-0.246f,	0.962f,		_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.306f,		-0.332f,	1.543f,		_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.188f,		-0.246f,	0.962f,		_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.113f,		-0.472f,	1.543f,		_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.306f,		-0.332f,	1.543f,		_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.188f,		-0.246f,	0.962f,		_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.068f,		-0.334f,	0.962f,		_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.113f,		-0.472f,	1.543f,		_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.068f,		-0.334f,	0.962f,		_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.126f,	-0.472f,	1.543f,		_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.113f,		-0.472f,	1.543f,		_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.068f,		-0.334f,	0.962f,		_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.081f,	-0.334f,	0.962f,		_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.126f,	-0.472f,	1.543f,		_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.081f,	-0.334f,	0.962f,		_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.319f,	-0.332f,	1.543f,		_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.126f,	-0.472f,	1.543f,		_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.081f,	-0.334f,	0.962f,		_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.201f,	-0.246f,	0.962f,		_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.319f,	-0.332f,	1.543f,		_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.201f,	-0.246f,	0.962f,		_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.393f,	-0.105f,	1.543f,		_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.319f,	-0.332f,	1.543f,		_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.201f,	-0.246f,	0.962f,		_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.247f,	-0.105f,	0.962f,		_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.393f,	-0.105f,	1.543f,		_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.087f,	0.551f,		-1.281f,	_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.087f,	0.152f,		-0.679f,	_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.087f,	-0.126f,	-1.028f,	_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.087f,	0.551f,		-1.281f,	_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.087f,	0.582f,		-0.560f,	_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.087f,	0.152f,		-0.679f,	_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.087f,	0.551f,		-1.281f,	_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.087f,	1.000f,		-0.716f,	_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.087f,	0.582f,		-0.560f,	_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.087f,	0.551f,		-1.281f,	_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.087f,	1.246f,		-1.088f,	_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.087f,	1.000f,		-0.716f,	_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.087f,	0.551f,		-1.281f,	_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.087f,	1.227f,		-1.534f,	_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.087f,	1.246f,		-1.088f,	_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.087f,	0.551f,		-1.281f,	_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.087f,	0.949f,		-1.883f,	_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.087f,	1.227f,		-1.534f,	_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.087f,	0.551f,		-1.281f,	_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.087f,	0.519f,		-2.002f,	_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.087f,	0.949f,		-1.883f,	_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.087f,	0.551f,		-1.281f,	_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.087f,	0.101f,		-1.846f,	_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.087f,	0.519f,		-2.002f,	_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.087f,	0.551f,		-1.281f,	_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.087f,	-0.145f,	-1.474f,	_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.087f,	0.101f,		-1.846f,	_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.087f,	0.551f,		-1.281f,	_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.087f,	-0.126f,	-1.028f,	_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.087f,	-0.145f,	-1.474f,	_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.087f,	-0.126f,	-1.028f,	_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.089f,		0.152f,		-0.679f,	_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.089f,		-0.126f,	-1.028f,	_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.087f,	-0.126f,	-1.028f,	_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.087f,	0.152f,		-0.679f,	_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.089f,		0.152f,		-0.679f,	_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.087f,	0.152f,		-0.679f,	_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.089f,		0.582f,		-0.560f,	_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.089f,		0.152f,		-0.679f,	_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.087f,	0.152f,		-0.679f,	_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.087f,	0.582f,		-0.560f,	_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.089f,		0.582f,		-0.560f,	_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.087f,	1.000f,		-0.716f,	_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.089f,		1.246f,		-1.088f,	_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.089f,		1.000f,		-0.716f,	_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.087f,	1.000f,		-0.716f,	_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.087f,	1.246f,		-1.088f,	_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.089f,		1.246f,		-1.088f,	_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.087f,	1.246f,		-1.088f,	_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.089f,		1.227f,		-1.534f,	_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.089f,		1.246f,		-1.088f,	_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.087f,	1.246f,		-1.088f,	_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.087f,	1.227f,		-1.534f,	_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.089f,		1.227f,		-1.534f,	_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.087f,	1.227f,		-1.534f,	_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.089f,		0.949f,		-1.883f,	_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.089f,		1.227f,		-1.534f,	_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.087f,	1.227f,		-1.534f,	_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.087f,	0.949f,		-1.883f,	_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.089f,		0.949f,		-1.883f,	_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.087f,	0.949f,		-1.883f,	_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.089f,		0.519f,		-2.002f,	_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.089f,		0.949f,		-1.883f,	_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.087f,	0.949f,		-1.883f,	_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.087f,	0.519f,		-2.002f,	_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.089f,		0.519f,		-2.002f,	_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.087f,	0.519f,		-2.002f,	_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.089f,		0.101f,		-1.846f,	_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.089f,		0.519f,		-2.002f,	_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.087f,	0.519f,		-2.002f,	_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.087f,	0.101f,		-1.846f,	_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.089f,		0.101f,		-1.846f,	_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.087f,	0.101f,		-1.846f,	_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.089f,		-0.145f,	-1.474f,	_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.089f,		0.101f,		-1.846f,	_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.087f,	0.101f,		-1.846f,	_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.087f,	-0.145f,	-1.474f,	_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.089f,		-0.145f,	-1.474f,	_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.087f,	-0.145f,	-1.474f,	_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.089f,		-0.126f,	-1.028f,	_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.089f,		-0.145f,	-1.474f,	_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.087f,	-0.145f,	-1.474f,	_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.087f,	-0.126f,	-1.028f,	_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.089f,		-0.126f,	-1.028f,	_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.089f,		0.551f,		-1.281f,	_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.089f,		-0.126f,	-1.028f,	_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.089f,		0.152f,		-0.679f,	_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.089f,		0.551f,		-1.281f,	_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.089f,		0.152f,		-0.679f,	_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.089f,		0.582f,		-0.560f,	_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.089f,		0.551f,		-1.281f,	_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.089f,		0.582f,		-0.560f,	_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.089f,		1.000f,		-0.716f,	_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.089f,		0.551f,		-1.281f,	_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.089f,		1.000f,		-0.716f,	_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.089f,		1.246f,		-1.088f,	_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.089f,		0.551f,		-1.281f,	_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.089f,		1.246f,		-1.088f,	_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.089f,		1.227f,		-1.534f,	_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.089f,		0.551f,		-1.281f,	_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.089f,		1.227f,		-1.534f,	_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.089f,		0.949f,		-1.883f,	_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.089f,		0.551f,		-1.281f,	_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.089f,		0.949f,		-1.883f,	_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.089f,		0.519f,		-2.002f,	_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.089f,		0.551f,		-1.281f,	_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.089f,		0.519f,		-2.002f,	_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.089f,		0.101f,		-1.846f,	_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.089f,		0.551f,		-1.281f,	_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.089f,		0.101f,		-1.846f,	_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.089f,		-0.145f,	-1.474f,	_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.089f,		0.551f,		-1.281f,	_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.089f,		-0.145f,	-1.474f,	_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.089f,		-0.126f,	-1.028f,	_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.087f,	1.032f,		0.006f,		_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.087f,	0.633f,		0.608f,		_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.087f,	0.355f,		0.258f,		_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.087f,	1.032f,		0.006f,		_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.087f,	1.063f,		0.727f,		_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.087f,	0.633f,		0.608f,		_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.087f,	1.032f,		0.006f,		_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.087f,	1.481f,		0.571f,		_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.087f,	1.063f,		0.727f,		_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.087f,	1.032f,		0.006f,		_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.087f,	1.727f,		0.199f,		_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.087f,	1.481f,		0.571f,		_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.087f,	1.032f,		0.006f,		_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.087f,	1.708f,		-0.247f,	_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.087f,	1.727f,		0.199f,		_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.087f,	1.032f,		0.006f,		_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.087f,	1.430f,		-0.597f,	_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.087f,	1.708f,		-0.247f,	_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.087f,	1.032f,		0.006f,		_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.087f,	1.000f,		-0.716f,	_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.087f,	1.430f,		-0.597f,	_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.087f,	1.032f,		0.006f,		_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.087f,	0.582f,		-0.560f,	_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.087f,	1.000f,		-0.716f,	_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.087f,	1.032f,		0.006f,		_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.087f,	0.336f,		-0.187f,	_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.087f,	0.582f,		-0.560f,	_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.087f,	1.032f,		0.006f,		_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.087f,	0.355f,		0.258f,		_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.087f,	0.336f,		-0.187f,	_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.087f,	0.355f,		0.258f,		_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.089f,		0.633f,		0.608f,		_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.089f,		0.355f,		0.258f,		_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.087f,	0.355f,		0.258f,		_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.087f,	0.633f,		0.608f,		_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.089f,		0.633f,		0.608f,		_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.087f,	0.633f,		0.608f,		_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.089f,		1.063f,		0.727f,		_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.089f,		0.633f,		0.608f,		_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.087f,	0.633f,		0.608f,		_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.087f,	1.063f,		0.727f,		_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.089f,		1.063f,		0.727f,		_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.087f,	1.063f,		0.727f,		_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.089f,		1.481f,		0.571f,		_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.089f,		1.063f,		0.727f,		_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.087f,	1.063f,		0.727f,		_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.087f,	1.481f,		0.571f,		_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.089f,		1.481f,		0.571f,		_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.087f,	1.481f,		0.571f,		_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.089f,		1.727f,		0.199f,		_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.089f,		1.481f,		0.571f,		_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.087f,	1.481f,		0.571f,		_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.087f,	1.727f,		0.199f,		_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.089f,		1.727f,		0.199f,		_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.087f,	1.727f,		0.199f,		_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.089f,		1.708f,		-0.247f,	_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.089f,		1.727f,		0.199f,		_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.087f,	1.727f,		0.199f,		_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.087f,	1.708f,		-0.247f,	_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.089f,		1.708f,		-0.247f,	_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.087f,	1.708f,		-0.247f,	_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.089f,		1.430f,		-0.597f,	_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.089f,		1.708f,		-0.247f,	_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.087f,	1.708f,		-0.247f,	_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.087f,	1.430f,		-0.597f,	_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.089f,		1.430f,		-0.597f,	_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.087f,	1.430f,		-0.597f,	_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.089f,		1.000f,		-0.716f,	_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.089f,		1.430f,		-0.597f,	_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.087f,	1.430f,		-0.597f,	_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.087f,	1.000f,		-0.716f,	_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.089f,		1.000f,		-0.716f,	_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.087f,	0.582f,		-0.560f,	_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.089f,		0.336f,		-0.187f,	_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.089f,		0.582f,		-0.560f,	_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.087f,	0.582f,		-0.560f,	_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.087f,	0.336f,		-0.187f,	_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.089f,		0.336f,		-0.187f,	_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.089f,		1.032f,		0.006f,		_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.089f,		0.355f,		0.258f,		_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.089f,		0.633f,		0.608f,		_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.089f,		1.032f,		0.006f,		_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.089f,		0.633f,		0.608f,		_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.089f,		1.063f,		0.727f,		_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.089f,		1.032f,		0.006f,		_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.089f,		1.063f,		0.727f,		_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.089f,		1.481f,		0.571f,		_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.089f,		1.032f,		0.006f,		_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.089f,		1.481f,		0.571f,		_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.089f,		1.727f,		0.199f,		_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.089f,		1.032f,		0.006f,		_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.089f,		1.727f,		0.199f,		_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.089f,		1.708f,		-0.247f,	_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.089f,		1.032f,		0.006f,		_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.089f,		1.708f,		-0.247f,	_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.089f,		1.430f,		-0.597f,	_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.089f,		1.032f,		0.006f,		_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.089f,		1.430f,		-0.597f,	_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.089f,		1.000f,		-0.716f,	_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.089f,		1.032f,		0.006f,		_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.089f,		1.000f,		-0.716f,	_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.089f,		0.582f,		-0.560f,	_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.089f,		1.032f,		0.006f,		_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.089f,		0.582f,		-0.560f,	_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.089f,		0.336f,		-0.187f,	_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.089f,		1.032f,		0.006f,		_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.089f,		0.336f,		-0.187f,	_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.089f,		0.355f,		0.258f,		_CAM_RED,  _CAM_GREEN,  _CAM_BLUE,  _CAM_ALPHA,  _S,  _T ),  
};

static const FDrawVtx_t _aCameraOutlineVtxs[_NUM_LINE_VERTS] = {
	FDrawVtx_t( -0.437f,	-0.859f,	0.972f,		_LINE_RED,  _LINE_GREEN,  _LINE_BLUE,  _LINE_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.437f,	0.436f,		0.972f,		_LINE_RED,  _LINE_GREEN,  _LINE_BLUE,  _LINE_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.437f,	0.436f,		0.972f,		_LINE_RED,  _LINE_GREEN,  _LINE_BLUE,  _LINE_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.437f,		0.436f,		0.972f,		_LINE_RED,  _LINE_GREEN,  _LINE_BLUE,  _LINE_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.437f,		0.436f,		0.972f,		_LINE_RED,  _LINE_GREEN,  _LINE_BLUE,  _LINE_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.437f,		-0.859f,	0.972f,		_LINE_RED,  _LINE_GREEN,  _LINE_BLUE,  _LINE_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.437f,		-0.859f,	0.972f,		_LINE_RED,  _LINE_GREEN,  _LINE_BLUE,  _LINE_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.437f,	-0.859f,	0.972f,		_LINE_RED,  _LINE_GREEN,  _LINE_BLUE,  _LINE_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.437f,	-0.859f,	0.972f,		_LINE_RED,  _LINE_GREEN,  _LINE_BLUE,  _LINE_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.361f,	-0.669f,	-0.648f,	_LINE_RED,  _LINE_GREEN,  _LINE_BLUE,  _LINE_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.361f,	-0.669f,	-0.648f,	_LINE_RED,  _LINE_GREEN,  _LINE_BLUE,  _LINE_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.361f,	0.246f,		-0.648f,	_LINE_RED,  _LINE_GREEN,  _LINE_BLUE,  _LINE_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.361f,	0.246f,		-0.648f,	_LINE_RED,  _LINE_GREEN,  _LINE_BLUE,  _LINE_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.437f,	0.436f,		0.972f,		_LINE_RED,  _LINE_GREEN,  _LINE_BLUE,  _LINE_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.437f,		-0.859f,	0.972f,		_LINE_RED,  _LINE_GREEN,  _LINE_BLUE,  _LINE_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.361f,		-0.669f,	-0.648f,	_LINE_RED,  _LINE_GREEN,  _LINE_BLUE,  _LINE_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.361f,		-0.669f,	-0.648f,	_LINE_RED,  _LINE_GREEN,  _LINE_BLUE,  _LINE_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.361f,		0.246f,		-0.648f,	_LINE_RED,  _LINE_GREEN,  _LINE_BLUE,  _LINE_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.361f,		0.246f,		-0.648f,	_LINE_RED,  _LINE_GREEN,  _LINE_BLUE,  _LINE_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.437f,		0.436f,		0.972f,		_LINE_RED,  _LINE_GREEN,  _LINE_BLUE,  _LINE_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.361f,		-0.669f,	-0.648f,	_LINE_RED,  _LINE_GREEN,  _LINE_BLUE,  _LINE_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.361f,	-0.669f,	-0.648f,	_LINE_RED,  _LINE_GREEN,  _LINE_BLUE,  _LINE_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.361f,		0.246f,		-0.648f,	_LINE_RED,  _LINE_GREEN,  _LINE_BLUE,  _LINE_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.361f,	0.246f,		-0.648f,	_LINE_RED,  _LINE_GREEN,  _LINE_BLUE,  _LINE_ALPHA,  _S,  _T ),
	FDrawVtx_t( -0.393f,	-0.105f,	1.543f,		_LINE_RED,  _LINE_GREEN,  _LINE_BLUE,  _LINE_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.319f,	-0.332f,	1.543f,		_LINE_RED,  _LINE_GREEN,  _LINE_BLUE,  _LINE_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.319f,	-0.332f,	1.543f,		_LINE_RED,  _LINE_GREEN,  _LINE_BLUE,  _LINE_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.126f,	-0.472f,	1.543f,		_LINE_RED,  _LINE_GREEN,  _LINE_BLUE,  _LINE_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.126f,	-0.472f,	1.543f,		_LINE_RED,  _LINE_GREEN,  _LINE_BLUE,  _LINE_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.113f,		-0.472f,	1.543f,		_LINE_RED,  _LINE_GREEN,  _LINE_BLUE,  _LINE_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.113f,		-0.472f,	1.543f,		_LINE_RED,  _LINE_GREEN,  _LINE_BLUE,  _LINE_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.306f,		-0.332f,	1.543f,		_LINE_RED,  _LINE_GREEN,  _LINE_BLUE,  _LINE_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.306f,		-0.332f,	1.543f,		_LINE_RED,  _LINE_GREEN,  _LINE_BLUE,  _LINE_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.38f,		-0.105f,	1.543f,		_LINE_RED,  _LINE_GREEN,  _LINE_BLUE,  _LINE_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.38f,		-0.105f,	1.543f,		_LINE_RED,  _LINE_GREEN,  _LINE_BLUE,  _LINE_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.306f,		0.122f,		1.543f,		_LINE_RED,  _LINE_GREEN,  _LINE_BLUE,  _LINE_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.306f,		0.122f,		1.543f,		_LINE_RED,  _LINE_GREEN,  _LINE_BLUE,  _LINE_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.113f,		0.263f,		1.543f,		_LINE_RED,  _LINE_GREEN,  _LINE_BLUE,  _LINE_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.113f,		0.263f,		1.543f,		_LINE_RED,  _LINE_GREEN,  _LINE_BLUE,  _LINE_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.126f,	0.263f,		1.543f,		_LINE_RED,  _LINE_GREEN,  _LINE_BLUE,  _LINE_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.126f,	0.263f,		1.543f,		_LINE_RED,  _LINE_GREEN,  _LINE_BLUE,  _LINE_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.319f,	0.122f,		1.543f,		_LINE_RED,  _LINE_GREEN,  _LINE_BLUE,  _LINE_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.319f,	0.122f,		1.543f,		_LINE_RED,  _LINE_GREEN,  _LINE_BLUE,  _LINE_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.393f,	-0.105f,	1.543f,		_LINE_RED,  _LINE_GREEN,  _LINE_BLUE,  _LINE_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.247f,	-0.105f,	0.962f,		_LINE_RED,  _LINE_GREEN,  _LINE_BLUE,  _LINE_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.201f,	-0.246f,	0.962f,		_LINE_RED,  _LINE_GREEN,  _LINE_BLUE,  _LINE_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.201f,	-0.246f,	0.962f,		_LINE_RED,  _LINE_GREEN,  _LINE_BLUE,  _LINE_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.081f,	-0.334f,	0.962f,		_LINE_RED,  _LINE_GREEN,  _LINE_BLUE,  _LINE_ALPHA,  _S,  _T ),
	FDrawVtx_t( -0.081f,	-0.334f,	0.962f,		_LINE_RED,  _LINE_GREEN,  _LINE_BLUE,  _LINE_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.068f,		-0.334f,	0.962f,		_LINE_RED,  _LINE_GREEN,  _LINE_BLUE,  _LINE_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.068f,		-0.334f,	0.962f,		_LINE_RED,  _LINE_GREEN,  _LINE_BLUE,  _LINE_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.188f,		-0.246f,	0.962f,		_LINE_RED,  _LINE_GREEN,  _LINE_BLUE,  _LINE_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.188f,		-0.246f,	0.962f,		_LINE_RED,  _LINE_GREEN,  _LINE_BLUE,  _LINE_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.234f,		-0.105f,	0.962f,		_LINE_RED,  _LINE_GREEN,  _LINE_BLUE,  _LINE_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.234f,		-0.105f,	0.962f,		_LINE_RED,  _LINE_GREEN,  _LINE_BLUE,  _LINE_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.188f,		0.037f,		0.962f,		_LINE_RED,  _LINE_GREEN,  _LINE_BLUE,  _LINE_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.188f,		0.037f,		0.962f,		_LINE_RED,  _LINE_GREEN,  _LINE_BLUE,  _LINE_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.068f,		0.124f,		0.962f,		_LINE_RED,  _LINE_GREEN,  _LINE_BLUE,  _LINE_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.068f,		0.124f,		0.962f,		_LINE_RED,  _LINE_GREEN,  _LINE_BLUE,  _LINE_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.081f,	0.124f,		0.962f,		_LINE_RED,  _LINE_GREEN,  _LINE_BLUE,  _LINE_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.081f,	0.124f,		0.962f,		_LINE_RED,  _LINE_GREEN,  _LINE_BLUE,  _LINE_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.201f,	0.037f,		0.962f,		_LINE_RED,  _LINE_GREEN,  _LINE_BLUE,  _LINE_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.201f,	0.037f,		0.962f,		_LINE_RED,  _LINE_GREEN,  _LINE_BLUE,  _LINE_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.247f,	-0.105f,	0.962f,		_LINE_RED,  _LINE_GREEN,  _LINE_BLUE,  _LINE_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.247f,	-0.105f,	0.962f,		_LINE_RED,  _LINE_GREEN,  _LINE_BLUE,  _LINE_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.393f,	-0.105f,	1.543f,		_LINE_RED,  _LINE_GREEN,  _LINE_BLUE,  _LINE_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.201f,	-0.246f,	0.962f,		_LINE_RED,  _LINE_GREEN,  _LINE_BLUE,  _LINE_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.319f,	-0.332f,	1.543f,		_LINE_RED,  _LINE_GREEN,  _LINE_BLUE,  _LINE_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.201f,	0.037f,		0.962f,		_LINE_RED,  _LINE_GREEN,  _LINE_BLUE,  _LINE_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.319f,	0.122f,		1.543f,		_LINE_RED,  _LINE_GREEN,  _LINE_BLUE,  _LINE_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.081f,	-0.334f,	0.962f,		_LINE_RED,  _LINE_GREEN,  _LINE_BLUE,  _LINE_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.126f,	-0.472f,	1.543f,		_LINE_RED,  _LINE_GREEN,  _LINE_BLUE,  _LINE_ALPHA,  _S,  _T ),
	FDrawVtx_t( -0.081f,	0.124f,		0.962f,		_LINE_RED,  _LINE_GREEN,  _LINE_BLUE,  _LINE_ALPHA,  _S,  _T ),  
	FDrawVtx_t( -0.126f,	0.263f,		1.543f,		_LINE_RED,  _LINE_GREEN,  _LINE_BLUE,  _LINE_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.068f,		-0.334f,	0.962f,		_LINE_RED,  _LINE_GREEN,  _LINE_BLUE,  _LINE_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.113f,		-0.472f,	1.543f,		_LINE_RED,  _LINE_GREEN,  _LINE_BLUE,  _LINE_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.068f,		0.124f,		0.962f,		_LINE_RED,  _LINE_GREEN,  _LINE_BLUE,  _LINE_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.113f,		0.263f,		1.543f,		_LINE_RED,  _LINE_GREEN,  _LINE_BLUE,  _LINE_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.188f,		-0.246f,	0.962f,		_LINE_RED,  _LINE_GREEN,  _LINE_BLUE,  _LINE_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.306f,		-0.332f,	1.543f,		_LINE_RED,  _LINE_GREEN,  _LINE_BLUE,  _LINE_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.188f,		0.037f,		0.962f,		_LINE_RED,  _LINE_GREEN,  _LINE_BLUE,  _LINE_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.306f,		0.122f,		1.543f,		_LINE_RED,  _LINE_GREEN,  _LINE_BLUE,  _LINE_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.234f,		-0.105f,	0.962f,		_LINE_RED,  _LINE_GREEN,  _LINE_BLUE,  _LINE_ALPHA,  _S,  _T ),  
	FDrawVtx_t( 0.38f,		-0.105f,	1.543f,		_LINE_RED,  _LINE_GREEN,  _LINE_BLUE,  _LINE_ALPHA,  _S,  _T ),
	FDrawVtx_t( -0.087f,	1.063f,		0.727f,		_LINE_RED,  _LINE_GREEN,  _LINE_BLUE,  _LINE_ALPHA,  _S,  _T ),
	FDrawVtx_t( -0.087f,	0.633f,		0.608f,		_LINE_RED,  _LINE_GREEN,  _LINE_BLUE,  _LINE_ALPHA,  _S,  _T ),
	FDrawVtx_t( -0.087f,	0.633f,		0.608f,		_LINE_RED,  _LINE_GREEN,  _LINE_BLUE,  _LINE_ALPHA,  _S,  _T ),	
	FDrawVtx_t( -0.087f,	0.355f,		0.258f,		_LINE_RED,  _LINE_GREEN,  _LINE_BLUE,  _LINE_ALPHA,  _S,  _T ),
	FDrawVtx_t( -0.087f,	0.355f,		0.258f,		_LINE_RED,  _LINE_GREEN,  _LINE_BLUE,  _LINE_ALPHA,  _S,  _T ),
	FDrawVtx_t( -0.087f,	0.336f,		-0.187f,	_LINE_RED,  _LINE_GREEN,  _LINE_BLUE,  _LINE_ALPHA,  _S,  _T ),
	FDrawVtx_t( -0.087f,	0.336f,		-0.187f,	_LINE_RED,  _LINE_GREEN,  _LINE_BLUE,  _LINE_ALPHA,  _S,  _T ),
	FDrawVtx_t( -0.087f,	0.582f,		-0.56f,		_LINE_RED,  _LINE_GREEN,  _LINE_BLUE,  _LINE_ALPHA,  _S,  _T ),
	FDrawVtx_t( -0.087f,	0.582f,		-0.56f,		_LINE_RED,  _LINE_GREEN,  _LINE_BLUE,  _LINE_ALPHA,  _S,  _T ),
	FDrawVtx_t( -0.087f,	0.152f,		-0.679f,	_LINE_RED,  _LINE_GREEN,  _LINE_BLUE,  _LINE_ALPHA,  _S,  _T ),
	FDrawVtx_t( -0.087f,	0.152f,		-0.679f,	_LINE_RED,  _LINE_GREEN,  _LINE_BLUE,  _LINE_ALPHA,  _S,  _T ),
	FDrawVtx_t( -0.087f,	-0.126f,	-1.028f,	_LINE_RED,  _LINE_GREEN,  _LINE_BLUE,  _LINE_ALPHA,  _S,  _T ),
	FDrawVtx_t( -0.087f,	-0.126f,	-1.028f,	_LINE_RED,  _LINE_GREEN,  _LINE_BLUE,  _LINE_ALPHA,  _S,  _T ),
	FDrawVtx_t( -0.087f,	-0.145f,	-1.474f,	_LINE_RED,  _LINE_GREEN,  _LINE_BLUE,  _LINE_ALPHA,  _S,  _T ),
	FDrawVtx_t( -0.087f,	-0.145f,	-1.474f,	_LINE_RED,  _LINE_GREEN,  _LINE_BLUE,  _LINE_ALPHA,  _S,  _T ),
	FDrawVtx_t( -0.087f,	0.101f,		-1.846f,	_LINE_RED,  _LINE_GREEN,  _LINE_BLUE,  _LINE_ALPHA,  _S,  _T ),
	FDrawVtx_t( -0.087f,	0.101f,		-1.846f,	_LINE_RED,  _LINE_GREEN,  _LINE_BLUE,  _LINE_ALPHA,  _S,  _T ),
	FDrawVtx_t( -0.087f,	0.519f,		-2.002f,	_LINE_RED,  _LINE_GREEN,  _LINE_BLUE,  _LINE_ALPHA,  _S,  _T ),
	FDrawVtx_t( -0.087f,	0.519f,		-2.002f,	_LINE_RED,  _LINE_GREEN,  _LINE_BLUE,  _LINE_ALPHA,  _S,  _T ),
	FDrawVtx_t( -0.087f,	0.949f,		-1.883f,	_LINE_RED,  _LINE_GREEN,  _LINE_BLUE,  _LINE_ALPHA,  _S,  _T ),
	FDrawVtx_t( -0.087f,	0.949f,		-1.883f,	_LINE_RED,  _LINE_GREEN,  _LINE_BLUE,  _LINE_ALPHA,  _S,  _T ),
	FDrawVtx_t( -0.087f,	1.227f,		-1.534f,	_LINE_RED,  _LINE_GREEN,  _LINE_BLUE,  _LINE_ALPHA,  _S,  _T ),
	FDrawVtx_t( -0.087f,	1.227f,		-1.534f,	_LINE_RED,  _LINE_GREEN,  _LINE_BLUE,  _LINE_ALPHA,  _S,  _T ),
	FDrawVtx_t( -0.087f,	1.246f,		-1.088f,	_LINE_RED,  _LINE_GREEN,  _LINE_BLUE,  _LINE_ALPHA,  _S,  _T ),
	FDrawVtx_t( -0.087f,	1.246f,		-1.088f,	_LINE_RED,  _LINE_GREEN,  _LINE_BLUE,  _LINE_ALPHA,  _S,  _T ),
	FDrawVtx_t( -0.087f,	1.0f,		-0.716f,	_LINE_RED,  _LINE_GREEN,  _LINE_BLUE,  _LINE_ALPHA,  _S,  _T ),
	FDrawVtx_t( -0.087f,	1.0f,		-0.716f,	_LINE_RED,  _LINE_GREEN,  _LINE_BLUE,  _LINE_ALPHA,  _S,  _T ),
	FDrawVtx_t( -0.087f,	1.43f,		-0.597f,	_LINE_RED,  _LINE_GREEN,  _LINE_BLUE,  _LINE_ALPHA,  _S,  _T ),
	FDrawVtx_t( -0.087f,	1.43f,		-0.597f,	_LINE_RED,  _LINE_GREEN,  _LINE_BLUE,  _LINE_ALPHA,  _S,  _T ),
	FDrawVtx_t( -0.087f,	1.708f,		-0.247f,	_LINE_RED,  _LINE_GREEN,  _LINE_BLUE,  _LINE_ALPHA,  _S,  _T ),
	FDrawVtx_t( -0.087f,	1.708f,		-0.247f,	_LINE_RED,  _LINE_GREEN,  _LINE_BLUE,  _LINE_ALPHA,  _S,  _T ),
	FDrawVtx_t( -0.087f,	1.727f,		0.199f,		_LINE_RED,  _LINE_GREEN,  _LINE_BLUE,  _LINE_ALPHA,  _S,  _T ),
	FDrawVtx_t( -0.087f,	1.727f,		0.199f,		_LINE_RED,  _LINE_GREEN,  _LINE_BLUE,  _LINE_ALPHA,  _S,  _T ),
	FDrawVtx_t( -0.087f,	1.481f,		0.571f,		_LINE_RED,  _LINE_GREEN,  _LINE_BLUE,  _LINE_ALPHA,  _S,  _T ),
	FDrawVtx_t( -0.087f,	1.481f,		0.571f,		_LINE_RED,  _LINE_GREEN,  _LINE_BLUE,  _LINE_ALPHA,  _S,  _T ),
	FDrawVtx_t( -0.087f,	1.063f,		0.727f,		_LINE_RED,  _LINE_GREEN,  _LINE_BLUE,  _LINE_ALPHA,  _S,  _T ),
	FDrawVtx_t( 0.089f,		1.063f,		0.727f,		_LINE_RED,  _LINE_GREEN,  _LINE_BLUE,  _LINE_ALPHA,  _S,  _T ),
	FDrawVtx_t( 0.089f,		0.633f,		0.608f,		_LINE_RED,  _LINE_GREEN,  _LINE_BLUE,  _LINE_ALPHA,  _S,  _T ),
	FDrawVtx_t( 0.089f,		0.633f,		0.608f,		_LINE_RED,  _LINE_GREEN,  _LINE_BLUE,  _LINE_ALPHA,  _S,  _T ),
	FDrawVtx_t( 0.089f,		0.355f,		0.258f,		_LINE_RED,  _LINE_GREEN,  _LINE_BLUE,  _LINE_ALPHA,  _S,  _T ),
	FDrawVtx_t( 0.089f,		0.355f,		0.258f,		_LINE_RED,  _LINE_GREEN,  _LINE_BLUE,  _LINE_ALPHA,  _S,  _T ),
	FDrawVtx_t( 0.089f,		0.336f,		-0.187f,	_LINE_RED,  _LINE_GREEN,  _LINE_BLUE,  _LINE_ALPHA,  _S,  _T ),
	FDrawVtx_t( 0.089f,		0.336f,		-0.187f,	_LINE_RED,  _LINE_GREEN,  _LINE_BLUE,  _LINE_ALPHA,  _S,  _T ),
	FDrawVtx_t( 0.089f,		0.582f,		-0.56f,		_LINE_RED,  _LINE_GREEN,  _LINE_BLUE,  _LINE_ALPHA,  _S,  _T ),
	FDrawVtx_t( 0.089f,		0.582f,		-0.56f,		_LINE_RED,  _LINE_GREEN,  _LINE_BLUE,  _LINE_ALPHA,  _S,  _T ),
	FDrawVtx_t( 0.089f,		0.152f,		-0.679f,	_LINE_RED,  _LINE_GREEN,  _LINE_BLUE,  _LINE_ALPHA,  _S,  _T ),
	FDrawVtx_t( 0.089f,		0.152f,		-0.679f,	_LINE_RED,  _LINE_GREEN,  _LINE_BLUE,  _LINE_ALPHA,  _S,  _T ),
	FDrawVtx_t( 0.089f,		-0.126f,	-1.028f,	_LINE_RED,  _LINE_GREEN,  _LINE_BLUE,  _LINE_ALPHA,  _S,  _T ),
	FDrawVtx_t( 0.089f,		-0.126f,	-1.028f,	_LINE_RED,  _LINE_GREEN,  _LINE_BLUE,  _LINE_ALPHA,  _S,  _T ),
	FDrawVtx_t( 0.089f,		-0.145f,	-1.474f,	_LINE_RED,  _LINE_GREEN,  _LINE_BLUE,  _LINE_ALPHA,  _S,  _T ),
	FDrawVtx_t( 0.089f,		-0.145f,	-1.474f,	_LINE_RED,  _LINE_GREEN,  _LINE_BLUE,  _LINE_ALPHA,  _S,  _T ),
	FDrawVtx_t( 0.089f,		0.101f,		-1.846f,	_LINE_RED,  _LINE_GREEN,  _LINE_BLUE,  _LINE_ALPHA,  _S,  _T ),
	FDrawVtx_t( 0.089f,		0.101f,		-1.846f,	_LINE_RED,  _LINE_GREEN,  _LINE_BLUE,  _LINE_ALPHA,  _S,  _T ),
	FDrawVtx_t( 0.089f,		0.519f,		-2.002f,	_LINE_RED,  _LINE_GREEN,  _LINE_BLUE,  _LINE_ALPHA,  _S,  _T ),
	FDrawVtx_t( 0.089f,		0.519f,		-2.002f,	_LINE_RED,  _LINE_GREEN,  _LINE_BLUE,  _LINE_ALPHA,  _S,  _T ),
	FDrawVtx_t( 0.089f,		0.949f,		-1.883f,	_LINE_RED,  _LINE_GREEN,  _LINE_BLUE,  _LINE_ALPHA,  _S,  _T ),
	FDrawVtx_t( 0.089f,		0.949f,		-1.883f,	_LINE_RED,  _LINE_GREEN,  _LINE_BLUE,  _LINE_ALPHA,  _S,  _T ),
	FDrawVtx_t( 0.089f,		1.227f,		-1.534f,	_LINE_RED,  _LINE_GREEN,  _LINE_BLUE,  _LINE_ALPHA,  _S,  _T ),
	FDrawVtx_t( 0.089f,		1.227f,		-1.534f,	_LINE_RED,  _LINE_GREEN,  _LINE_BLUE,  _LINE_ALPHA,  _S,  _T ),
	FDrawVtx_t( 0.089f,		1.246f,		-1.088f,	_LINE_RED,  _LINE_GREEN,  _LINE_BLUE,  _LINE_ALPHA,  _S,  _T ),
	FDrawVtx_t( 0.089f,		1.246f,		-1.088f,	_LINE_RED,  _LINE_GREEN,  _LINE_BLUE,  _LINE_ALPHA,  _S,  _T ),
	FDrawVtx_t( 0.089f,		1.0f,		-0.716f,	_LINE_RED,  _LINE_GREEN,  _LINE_BLUE,  _LINE_ALPHA,  _S,  _T ),
	FDrawVtx_t( 0.089f,		1.0f,		-0.716f,	_LINE_RED,  _LINE_GREEN,  _LINE_BLUE,  _LINE_ALPHA,  _S,  _T ),
	FDrawVtx_t( 0.089f,		1.43f,		-0.597f,	_LINE_RED,  _LINE_GREEN,  _LINE_BLUE,  _LINE_ALPHA,  _S,  _T ),
	FDrawVtx_t( 0.089f,		1.43f,		-0.597f,	_LINE_RED,  _LINE_GREEN,  _LINE_BLUE,  _LINE_ALPHA,  _S,  _T ),
	FDrawVtx_t( 0.089f,		1.708f,		-0.247f,	_LINE_RED,  _LINE_GREEN,  _LINE_BLUE,  _LINE_ALPHA,  _S,  _T ),
	FDrawVtx_t( 0.089f,		1.708f,		-0.247f,	_LINE_RED,  _LINE_GREEN,  _LINE_BLUE,  _LINE_ALPHA,  _S,  _T ),
	FDrawVtx_t( 0.089f,		1.727f,		0.199f,		_LINE_RED,  _LINE_GREEN,  _LINE_BLUE,  _LINE_ALPHA,  _S,  _T ),
	FDrawVtx_t( 0.089f,		1.727f,		0.199f,		_LINE_RED,  _LINE_GREEN,  _LINE_BLUE,  _LINE_ALPHA,  _S,  _T ),
	FDrawVtx_t( 0.089f,		1.481f,		0.571f,		_LINE_RED,  _LINE_GREEN,  _LINE_BLUE,  _LINE_ALPHA,  _S,  _T ),
	FDrawVtx_t( 0.089f,		1.481f,		0.571f,		_LINE_RED,  _LINE_GREEN,  _LINE_BLUE,  _LINE_ALPHA,  _S,  _T ),
	FDrawVtx_t( 0.089f,		1.063f,		0.727f,		_LINE_RED,  _LINE_GREEN,  _LINE_BLUE,  _LINE_ALPHA,  _S,  _T ),
};
#endif

FCLASS_NOALIGN_PREFIX struct _CameraManInfo_t {
	CFCameraMan *pCamMan;
	BOOL bDrawIcon;

	FCLASS_STACKMEM_NOALIGN( _CameraManInfo_t );
} FCLASS_NOALIGN_SUFFIX;

static BOOL _bSystemInited = FALSE;
static u32 _nNumCameras = 0;
static CFCamera *_paCameras = NULL;// _nNumCameras of these
static _CameraManInfo_t *_paCameraMenInfo = NULL;// 1-1 relationship with the cameras (_nNumCameras of these)
static u32 _nNumCameraMenInUse = 0;
static u32 _nNumCamsToDraw = 0;
static s32 _nLastCamIdx = 0;
static s32 _nLastCamSetup = -1;

//===================
// private prototypes

//=================
// public functions

BOOL fcamera_ModuleStartup( void ) {
	FASSERT( !_bSystemInited );

	// just reset our vars, the magic happens later
	_bSystemInited = FALSE;
	_nNumCameras = 0;
	_paCameras = NULL;
	_paCameraMenInfo = NULL;
	_nNumCameraMenInUse = 0;
	_nNumCamsToDraw = 0;

	return TRUE;
}

void fcamera_ModuleShutdown( void ) {
	fcamera_UninitSystem();
}

// returns TRUE if the camera system could init ok.
// returns FALSE if there was a problem.
BOOL fcamera_InitSystem( u32 nNumCameras ) {
	u32 i;
	FResFrame_t ResFrame;

	FASSERT( !_bSystemInited );

	ResFrame = fres_GetFrame();
	
	if( !nNumCameras ) {
		DEVPRINTF( "fcamera_InitSystem(): No desired camera count specified, there must be at lease 1.\n" );
		goto _ExitSetupWithError;
	}

	// allocate the desired camera count	
	_paCameras = fnew CFCamera[nNumCameras];
	if( !_paCameras ) {
		DEVPRINTF( "fcamera_InitSystem(): Could not allocated %d CFCamera objects.\n", nNumCameras );
		goto _ExitSetupWithError;
	}
	// allocate the camera man info array
	_paCameraMenInfo = fnew _CameraManInfo_t[nNumCameras];
	if( !_paCameraMenInfo ) {
		DEVPRINTF( "fcamera_InitSystem(): Could not allocated %d _CameraManInfo_t objects.\n", nNumCameras );
		goto _ExitSetupWithError;
	}
	// init each camera
	for( i=0; i < nNumCameras; i++ ) {
		if( !_paCameras[i].Init( i ) ) {
			DEVPRINTF( "fcamera_InitSystem(): Could not init all of the cameras.\n" );
			goto _ExitSetupWithError;
		}
		_paCameraMenInfo[i].bDrawIcon = FALSE;
		_paCameraMenInfo[i].pCamMan = NULL;
	}

	// mark all slots, except the first slot, as unused
	_nNumCameraMenInUse = 0;
	_nNumCameras = 0;
	_nNumCameras = nNumCameras;
	_nLastCamIdx = 0;
	_nLastCamSetup = -1;
	_bSystemInited = TRUE;

	return TRUE;

_ExitSetupWithError:
	fcamera_UninitSystem();
	fres_ReleaseFrame( ResFrame );
	return FALSE;
}

// removes the camera system.
void fcamera_UninitSystem( void ) {

	_bSystemInited = FALSE;
	_nNumCameras = 0;
	if( _paCameras ) {
		fdelete_array( _paCameras );
		_paCameras = NULL;
	}
	if( _paCameraMenInfo ) {
		fdelete_array( _paCameraMenInfo );
		_paCameraMenInfo = NULL;
	}	
	_nNumCameraMenInUse = 0;
	_nNumCamsToDraw = 0;
}

// gets the number of cameras that the system manages.
u32 fcamera_GetCameraCount( void ) {
	
	// Cleanup can query before camera initialization has completed.
	return _bSystemInited ? _nNumCameras : 0;
}

// gets the last camera index for use in render target callbacks.
s32 fcamera_GetLastCameraIndex()
{
	return (_nLastCamSetup);
}


// returns TRUE if pszName could be assigned to the nIndex'th camera.
// FALSE if nIndex is an invalid index or there was a problem.
BOOL fcamera_SetCameraName( u32 nIndex, cchar *pszName ) {
	
	FASSERT( _bSystemInited );

	if( nIndex >= _nNumCameras ) {
		DEVPRINTF( "fcamera_SetCameraName(): Index '%d' is invalid.\n", nIndex );
		return FALSE;	
	}
	_paCameras[nIndex].SetName( pszName );

	return TRUE;
}

// returns a pointer to the first camera named pszName.
// NULL if pszName could not be found.
CFCamera *fcamera_GetCameraByName( cchar *pszName ) {
	u32 i;

	FASSERT( _bSystemInited );

	for( i=0; i < _nNumCameras; i++ ) {
		if( fclib_strncmp( _paCameras[i].GetName(), pszName, FCAMERA_MAX_NAME_LEN ) == 0 ) {
			// the names match, this is the desired camera
			_nLastCamIdx = i;
			return &_paCameras[i];
		}
	}
	return FALSE;
}

// returns a pointer to the nIndex'th camera.
// NULL if nIndex is an invalid index.
CFCamera *fcamera_GetCameraByIndex( u32 nIndex ) {
	
	FASSERT( _bSystemInited );

	if( nIndex >= _nNumCameras ) {
		DEVPRINTF( "fcamera_GetCameraByIndex(): Index '%d' is invalid.\n", nIndex );
		return NULL;	
	}
	_nLastCamIdx = nIndex;
	
	return &_paCameras[nIndex];
}

// returns a pointer to the camera man assigned to the nCameraIndex'th camera.
// NULL if nCameraIndex is an invalid index or no camera man has been assigned to that slot.
CFCameraMan *fcamera_GetCameraManByIndex( u32 nCameraIndex ) {
	
	FASSERT( _bSystemInited );

	if( nCameraIndex >= _nNumCameras ) {
		DEVPRINTF( "fcamera_GetCameraManByIndex(): Index '%d' is invalid.\n", nCameraIndex );
		return NULL;	
	}
	return _paCameraMenInfo[nCameraIndex].pCamMan;
}

// returns a pointer to the previous camera man assigned to the nCameraIndex'th camera 
// and then assigns pCameraMan (could be NULL) to the camera.
CFCameraMan *fcamera_SetCameraManByIndex( u32 nCameraIndex, CFCameraMan *pCameraMan ) {
	
	FASSERT( _bSystemInited );

	if( nCameraIndex >= _nNumCameras ) {
		DEVPRINTF( "fcamera_SetCameraManByIndex(): Index '%d' is invalid.\n", nCameraIndex );
		return NULL;	
	}
	CFCameraMan *pPrev = _paCameraMenInfo[nCameraIndex].pCamMan;
	if( pPrev ) {
		// unassign the past camera man
		pPrev->AssignCamera( NULL );
		_paCameraMenInfo[nCameraIndex].pCamMan = NULL;
		_nNumCameraMenInUse--;
	}
	if( pCameraMan ) {
		if( pCameraMan->GetAssignedCamera() ) {
			// warn the user that this camera man already is assigned to a camera
			DEVPRINTF( "fcamera_SetCameraManByIndex(): Warning, trying to re-assign a working camera man.\n", nCameraIndex );
		}
		_paCameraMenInfo[nCameraIndex].pCamMan = pCameraMan;
		pCameraMan->AssignCamera( &_paCameras[nCameraIndex] );
		_nNumCameraMenInUse++;
	} else {
		_paCameraMenInfo[nCameraIndex].pCamMan = NULL;
	}

	return pPrev;
}

u32 fcamera_GetNumWorkingCameraMen( void ) {
	
	FASSERT( _bSystemInited );

	return _nNumCameraMenInUse;
}

void fcamera_ToggleCameraIconDraw( u32 nIndex, BOOL bDrawIcon ) {

	FASSERT( _bSystemInited );

	if( nIndex >= _nNumCameras ) {
		DEVPRINTF( "fcamera_ToggleCameraIconDraw(): Index %d is invalid.\n", nIndex );	
		return;
	}
	
	if( bDrawIcon ) {
		if( !_paCameraMenInfo[nIndex].bDrawIcon ) {
			// start drawing the camera icon
			_paCameraMenInfo[nIndex].bDrawIcon = TRUE;
			_nNumCamsToDraw++;
		}
	} else {
		if( _paCameraMenInfo[nIndex].bDrawIcon ) {
			// stop drawing the camera icon
			_paCameraMenInfo[nIndex].bDrawIcon = FALSE;
			_nNumCamsToDraw--;
		}
	}
}

void fcamera_StopEffectsOnAllCameras( void ) {
	u32 i;

	FASSERT( _bSystemInited );

	for( i=0; i < _nNumCameras; i++ ) {
		if( _paCameraMenInfo[i].pCamMan ) {
			// there is a camera man assigned to this camera
			_paCameras[i].StopCamEffects();
		}		
	}
}

void fcamera_Work( void ) {
	u32 i;

	FASSERT( _bSystemInited );

	if( !_nNumCameraMenInUse ) {
		// no cameramen are in use
		return;
	}

	// call the work functions for each camera in use
	for( i=0; i < _nNumCameras; i++ ) {
		if( _paCameraMenInfo[i].pCamMan ) {
			// there is a camera man assigned to this camera, call the CFCamera & CFCameraMan functions
			_paCameras[i].PreWork();
			_paCameraMenInfo[i].pCamMan->Work();
			_paCameras[i].PostWork();
		}		
	}
}

// Assumes that the game has already setup the camera.
// This function simply draws a camera icon at each
// camera location requested.
void fcamera_Draw( void ) {

	FASSERT( _bSystemInited );

#if _ENABLE_DEBUG_CAMERA_ICON
	if( !_nNumCamsToDraw ) {
		return;
	}
	u32 nDepthBias, i;
	CFXfm Xfm;

	frenderer_Push( FRENDERER_DRAW, NULL );
	fdraw_Depth_EnableWriting( TRUE );
	fdraw_Depth_SetTest( FDRAW_DEPTHTEST_CLOSER_OR_EQUAL );
	fdraw_SetTexture( NULL );
//	fdraw_Color_SetFunc( FDRAW_COLORFUNC_DIFFUSETEX_AIAT );
	fdraw_Color_SetFunc( FDRAW_COLORFUNC_DECAL_AI );
	fdraw_Alpha_SetBlendOp( FDRAW_BLENDOP_LERP_WITH_ALPHA_OPAQUE );

	for( i=0; i < _nNumCameras; i++ ) {
		if( _paCameraMenInfo[i].bDrawIcon ) {
			Xfm.ReceiveInverseOf( *_paCameras[i].GetFinalXfm() );
			Xfm.PushModel();

			fdraw_ModelSpaceAxis( 2.0f );

			fdraw_PrimList( FDRAW_PRIMTYPE_TRILIST, _aCameraIconVtxs, _NUM_CAM_VERTS );
			
			nDepthBias = fdraw_Depth_GetBiasLevel();
			fdraw_Depth_DeltaBiasLevel( 1 );
			fdraw_PrimList( FDRAW_PRIMTYPE_LINELIST, _aCameraOutlineVtxs, _NUM_LINE_VERTS );
			fdraw_Depth_SetBiasLevel( nDepthBias );

			Xfm.PopModel();
		}
	}

	frenderer_Pop();
#endif
}

//==================
// private functions


///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Start of CFCamera functions 

CFCamera::CFCamera() {
#if FANG_WINGC
	m_pViewSource = NULL;
	m_pViewViewport = NULL;
#endif
	m_CameraData.m_bInited = FALSE;
	m_CameraData.m_nID = 0;
	fang_MemZero( m_CameraData.m_szName, FCAMERA_MAX_NAME_LEN+1 );
	InvalidateCacheKeys();
	m_CameraData.m_pViewport = NULL;
	m_CameraData.m_Xfm.Identity();
	m_FinalXfm.Identity();
	m_CameraData.m_fHeading = 0.0f;
	m_CameraData.m_fPitch = 0.0f;
	m_CameraData.m_fRoll = 0.0f;
	m_CameraData.m_XZUnitLookAt.Zero();
	m_CameraData.m_LastPos.Zero();
	m_PreWorkCache.Zero();
	m_CameraData.m_fCamUnitVibration = 0.0f;
}

CFCamera::~CFCamera() {

	m_CameraData.m_bInited = FALSE;
	m_CameraData.m_pViewport = NULL;
}

void CFCamera::InvalidateCacheKeys() {
	
	m_nHPRCacheKey = -1;
	m_nXZLookAtCacheKey = -1;
}

// must be called before class can be used
BOOL CFCamera::Init( u32 nID ) {
	
	FASSERT( !m_CameraData.m_bInited );

	m_CameraData.m_pViewport = fviewport_Create();
	if( !m_CameraData.m_pViewport ) {		
		return FALSE;
	}

#if FANG_WINGC
	m_pViewViewport = fviewport_Create();
	if( !m_pViewViewport ) return FALSE;
#endif
	m_CameraData.m_bInited = TRUE;

	Reset( nID );

	return TRUE;
}

BOOL CFCamera::IsInited() {

	FASSERT( m_CameraData.m_bInited );

	return m_CameraData.m_bInited;
}

BOOL CFCamera::Reset( u32 nID ) {
#if FANG_WINGC
	m_pViewSource = NULL;
#endif

	FASSERT( m_CameraData.m_bInited );

	m_CameraData.m_CamStun.Reset();
	m_CameraData.m_CamShake.Reset();
	m_CameraData.m_fCamUnitVibration = 0.0f;
	m_CameraData.m_fHalfFOV_X = _DEFAULT_CAM_X_FOV;
	m_CameraData.m_fHalfFOV_Y = _DEFAULT_CAM_Y_FOV;
	m_CameraData.m_fNearZ = _DEFAULT_CAM_NEAR_Z;
	m_CameraData.m_fFarZ = _DEFAULT_CAM_FAR_Z;
	m_CameraData.m_bReInitViewport = FALSE;							
	fviewport_InitPersp( m_CameraData.m_pViewport, 
						 m_CameraData.m_fHalfFOV_X, m_CameraData.m_fHalfFOV_Y,
						 m_CameraData.m_fNearZ, m_CameraData.m_fFarZ,
						 NULL );
	m_CameraData.m_nID = nID;
	InvalidateCacheKeys();
	m_CameraData.m_Xfm.Identity();
	m_FinalXfm.Identity();
	
	return TRUE;
}

#if FANG_WINGC
BOOL CFCamera::SetViewSource( CFCamera *pSource ) {
	// Reject self/cyclic links; a source change takes effect in the same render frame.
	for( CFCamera *p = pSource; p; p = p->m_pViewSource ) if( p == this ) return FALSE;
	if( pSource && (!m_CameraData.m_bInited || !pSource->m_CameraData.m_bInited || !m_pViewViewport) ) return FALSE;
	m_pViewSource = pSource;
	return TRUE;
}
#endif

const FViewport_t *CFCamera::GetViewport() {

	FASSERT( m_CameraData.m_bInited );

#if FANG_WINGC
	if( m_pViewSource ) {
		const FViewport_t *pSource = m_pViewSource->GetViewport();
		const FViewport_t *pOwn = m_CameraData.m_pViewport;
		// Preserve cinematic vertical framing and adapt horizontal coverage to this split's aspect.
		const f32 fHalfX = fmath_Atan( pSource->fTanHalfFOVY * (f32)pOwn->nWidth, (f32)pOwn->nHeight );
		fviewport_InitPersp( m_pViewViewport, fHalfX, pSource->fHalfFOVY, pSource->fNearZ, pSource->fFarZ,
			pOwn->nScreenLeftX, pOwn->nScreenTopY, pOwn->nWidth, pOwn->nHeight, pOwn->pTexDef );
		return m_pViewViewport;
	}
#endif
	return m_CameraData.m_pViewport;
}

const CFXfm *CFCamera::GetFinalXfm() {

	FASSERT( m_CameraData.m_bInited );

#if FANG_WINGC
	if( m_pViewSource ) return m_pViewSource->GetFinalXfm();
#endif
	return &m_FinalXfm;
}

const CFXfm *CFCamera::GetXfmWithoutShake() const {

	FASSERT( m_CameraData.m_bInited );

#if FANG_WINGC
	if( m_pViewSource ) return m_pViewSource->GetXfmWithoutShake();
#endif
	return &m_CameraData.m_Xfm;
}

// called once per frame before the camera man's work function
void CFCamera::PreWork() {
	
	FASSERT( m_CameraData.m_bInited );

	// cache the pos
	m_PreWorkCache = m_CameraData.m_Xfm.m_MtxR.m_vPos.v3;
}

// called once per frame after the camera man's work function
void CFCamera::PostWork() {
	
	FASSERT( m_CameraData.m_bInited );

	m_CameraData.m_CamShake.Work();
	m_CameraData.m_CamStun.Work();

	// do any camera specific work

	// init the viewport to the desired fov 
	if( m_CameraData.m_bReInitViewport ) {
		m_CameraData.m_bReInitViewport = FALSE;
		fviewport_InitPersp( m_CameraData.m_pViewport, 
							 m_CameraData.m_fHalfFOV_X, m_CameraData.m_fHalfFOV_Y,
							 m_CameraData.m_fNearZ, m_CameraData.m_fFarZ,
							 m_CameraData.m_pViewport->nScreenLeftX,
							 m_CameraData.m_pViewport->nScreenTopY,
							 m_CameraData.m_pViewport->nWidth,
							 m_CameraData.m_pViewport->nHeight,
							 NULL );
	}

	m_CameraData.m_LastPos = m_PreWorkCache;

	m_FinalXfm = m_CameraData.m_Xfm;
	m_CameraData.m_CamShake.AddShakeToXfm( &m_FinalXfm );
	m_CameraData.m_CamStun.AddStunToXfm( &m_FinalXfm );

	if( m_CameraData.m_fCamUnitVibration > 0.0f ) {
		f32 fVibeRange = m_CameraData.m_fCamUnitVibration * 0.005f;

		CFMtx43A Mtx;
		Mtx.SetRotationYXZ( fmath_RandomFloatRange( -fVibeRange, fVibeRange ), fmath_RandomFloatRange( -fVibeRange, fVibeRange ), fmath_RandomFloatRange( -fVibeRange, fVibeRange ) );
		Mtx.m_vPos.Zero();

		CFXfm ShakeXfm;
		ShakeXfm.BuildFromMtx( Mtx );

		m_FinalXfm.ReceiveProductOf( ShakeXfm, m_FinalXfm );

		m_CameraData.m_fCamUnitVibration = 0.0f;
	}
}

// called to set the active viewport and init the camera stack
void CFCamera::SetupCameraAndViewport() {

	FASSERT( m_CameraData.m_bInited );
	_nLastCamSetup = _nLastCamIdx;

	fviewport_SetActive( (FViewport_t *)GetViewport() );

	GetFinalXfm()->InitStackWithView();
}

// Move the camera viewport to the new location
void CFCamera::MoveViewport(u32 left, u32 top, u32 width, u32 height, BOOL bPreserveFOVX) {

	FASSERT( m_CameraData.m_bInited );

	float fFOVX, fFOVY;
	if (bPreserveFOVX) {
		fFOVX = m_CameraData.m_fHalfFOV_X;
		fFOVY = 0.0f;
	}
	else {
		fFOVY = m_CameraData.m_pViewport->fHalfFOVY;
		fFOVX = fmath_Atan(m_CameraData.m_pViewport->fTanHalfFOVY * (f32)width, (f32)height);
		m_CameraData.m_fHalfFOV_X = fFOVX;
		m_CameraData.m_fHalfFOV_Y = fFOVY;
	}
	fviewport_InitPersp( m_CameraData.m_pViewport, fFOVX, fFOVY,
		m_CameraData.m_fNearZ, m_CameraData.m_fFarZ,
		left, top, width, height, NULL);

	// Just to be safe, copy the actual vertical fov back into the camera
	m_CameraData.m_fHalfFOV_Y = m_CameraData.m_pViewport->fHalfFOVY;
	m_CameraData.m_bReInitViewport = FALSE;
}

void CFCamera::MoveViewport( const FViewport_t *pViewport ) {

	FASSERT( m_CameraData.m_bInited );

	fviewport_InitPersp( m_CameraData.m_pViewport, 
		m_CameraData.m_fHalfFOV_X, m_CameraData.m_fHalfFOV_Y,
		m_CameraData.m_fNearZ, m_CameraData.m_fFarZ,
		pViewport->nScreenLeftX, pViewport->nScreenTopY, pViewport->nWidth, pViewport->nHeight, NULL);

	m_CameraData.m_bReInitViewport = FALSE;
}

const CFVec3 *CFCamera::GetPos() {

	FASSERT( m_CameraData.m_bInited );

#if FANG_WINGC
	if( m_pViewSource ) return m_pViewSource->GetPos();
#endif
	return &m_CameraData.m_Xfm.m_MtxR.m_vPos.v3;
}

void CFCamera::GetHeadingPitchRoll( f32 *pfHeading, f32 *pfPitch, f32 *pfRoll ) {

	FASSERT( m_CameraData.m_bInited );

	if( (s32)FVid_nFrameCounter != m_nHPRCacheKey ) {
		// we need to compute the heading pitch and roll, mark that we have done so this frame
		m_nHPRCacheKey = FVid_nFrameCounter;

		m_CameraData.m_fHeading = fmath_Atan( m_CameraData.m_Xfm.m_MtxR.m_vFront.x, m_CameraData.m_Xfm.m_MtxR.m_vFront.z );
		m_CameraData.m_fPitch = -fmath_Atan( m_CameraData.m_Xfm.m_MtxR.m_vUp.z, m_CameraData.m_Xfm.m_MtxR.m_vUp.y );
		m_CameraData.m_fRoll = fmath_Atan( m_CameraData.m_Xfm.m_MtxR.m_vRight.y, m_CameraData.m_Xfm.m_MtxR.m_vRight.x );		
	}
	if( pfHeading ) {
		*pfHeading = m_CameraData.m_fHeading;
	}
	if( pfPitch ) {
		*pfPitch = m_CameraData.m_fPitch;
	}
	if( pfRoll ) {
		*pfRoll = m_CameraData.m_fRoll;
	}
}

const CFVec3 *CFCamera::GetLastPos() {

	FASSERT( m_CameraData.m_bInited );

	return &m_CameraData.m_LastPos;
}

CFVec2 *CFCamera::GetXZUnitDir() {

	FASSERT( m_CameraData.m_bInited );

	if( (s32)FVid_nFrameCounter != m_nXZLookAtCacheKey ) {
		// we need to compute the xz unit dir, mark that we have done so this frame
		m_nXZLookAtCacheKey = FVid_nFrameCounter;

		m_CameraData.m_XZUnitLookAt.Set( m_CameraData.m_Xfm.m_MtxR.m_vFront.x, m_CameraData.m_Xfm.m_MtxR.m_vFront.z );
		m_CameraData.m_XZUnitLookAt.Unitize();
	}
	return &m_CameraData.m_XZUnitLookAt;
}
// returns TRUE if rWSPoint is fDistance2 or less from the camera pos
BOOL CFCamera::IsPointCloseToCamera( const CFVec3 &rWSPoint, f32 fDistance2 ) {

	FASSERT( m_CameraData.m_bInited );

	CFVec3A CamToPoint;
	CamToPoint.Set( rWSPoint );
	CamToPoint.Sub( m_CameraData.m_Xfm.m_MtxR.m_vPos );
	
	return ( CamToPoint.MagSq() <= fDistance2 );
}

void CFCamera::SetFOV( f32 fHalfX, f32 fHalfY/*=0.0f*/ ) {

	FASSERT( m_CameraData.m_bInited );

	m_CameraData.m_bReInitViewport = TRUE;

	m_CameraData.m_fHalfFOV_X = fHalfX;
	m_CameraData.m_fHalfFOV_Y = fHalfY;
}

void CFCamera::GetFOV( f32 *pfHalfX, f32 *pfHalfY/*=NULL*/ ) {
	
	FASSERT( m_CameraData.m_bInited );

	if( pfHalfX ) {
		*pfHalfX = m_CameraData.m_fHalfFOV_X;
	}
	if( pfHalfY ) {
		*pfHalfY = m_CameraData.m_fHalfFOV_Y;
	}
}	

void CFCamera::SetClipPlanes( f32 fNearZ, f32 fFarZ ) {
	
	FASSERT( m_CameraData.m_bInited );

	m_CameraData.m_bReInitViewport = TRUE;
	
	m_CameraData.m_fNearZ = fNearZ;
	m_CameraData.m_fFarZ = fFarZ;
}

void CFCamera::GetClipPlanes( f32 *pfNearZ, f32 *pfFarZ ) {

	FASSERT( m_CameraData.m_bInited );

	if( pfNearZ ) {
		*pfNearZ = m_CameraData.m_fNearZ;
	}
	if( pfFarZ ) {
		*pfFarZ = m_CameraData.m_fFarZ;
	}
}

u32 CFCamera::SetID( u32 nID ) { 

	FASSERT( m_CameraData.m_bInited );

	u32 nPrevID = m_CameraData.m_nID;
	m_CameraData.m_nID = nID;
	return nPrevID; 
}

u32 CFCamera::GetID() {

	FASSERT( m_CameraData.m_bInited );

	return m_CameraData.m_nID;
}

void CFCamera::SetName( cchar *pszName ) {

	FASSERT( m_CameraData.m_bInited );

	fclib_strncpyz( m_CameraData.m_szName, pszName, FCAMERA_MAX_NAME_LEN );
}

cchar *CFCamera::GetName() {

	FASSERT( m_CameraData.m_bInited );
	
	return m_CameraData.m_szName;
}

void CFCamera::SetCamVibrationForOneFrame( f32 fUnitVibration ) {
	FASSERT( m_CameraData.m_bInited );

	if( fUnitVibration > m_CameraData.m_fCamUnitVibration ) {
		m_CameraData.m_fCamUnitVibration = fUnitVibration;
	}
}

void CFCamera::ShakeCamera( f32 fUnitIntensity, f32 fDurationInSecs ) {
	FASSERT( m_CameraData.m_bInited );

	m_CameraData.m_CamShake.AddShaker( fUnitIntensity, fDurationInSecs );
}

void CFCamera::StunCamera( f32 fDurationInSecs ) {
	FASSERT( m_CameraData.m_bInited );

	m_CameraData.m_CamStun.EnableStun( fDurationInSecs );
}

void CFCamera::StopCamEffects( void ) {
	m_CameraData.m_CamShake.Reset();
	m_CameraData.m_CamStun.Reset();
}


// End of CFCamera functions
////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////
// Start of CFCameraMan Functions

CFCameraMan::CFCameraMan() {
	m_pCamera = NULL;
	m_pCameraData = NULL;
	m_bFreezeShot = FALSE;
}

CFCameraMan::~CFCameraMan() {
}

void CFCameraMan::Draw() {
	// the default implemenation does nothing...
}

CFCamera *CFCameraMan::AssignCamera( CFCamera *pCamera ) {
	CFCamera *pPrev = m_pCamera;
	
	m_pCamera = pCamera;
	m_pCameraData = (m_pCamera) ? &m_pCamera->m_CameraData : NULL;
	return pPrev;
}

CFCamera *CFCameraMan::GetAssignedCamera() {
	return m_pCamera;
}

const FCameraData_t *CFCameraMan::GetAssignedCameraData() {
	return m_pCameraData;
}

BOOL CFCameraMan::FreezeShot( BOOL bFreeze ) {
	BOOL bLast = m_bFreezeShot;
	m_bFreezeShot = bFreeze;
	return bLast;
}

// End of CFCameraMan functions
////////////////////////////////////////////////////////////////
