//////////////////////////////////////////////////////////////////////////////////////
// GCNormalSphere.cpp - Code for generation of the normal sphere used for object lighting/backface culling
//
// Author: John Lafleur
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
// 03/17/02 Lafleur		Created.
//////////////////////////////////////////////////////////////////////////////////////


#include "stdafx.h"
#include <math.h>
#include <stdio.h>

#include "fang.h"
#include "fmath.h"
#include "gc/fGCData.h"
#include "GCNormalSphere.h"


//
//
//
void WriteNormalSphere( void )
{
//	FGCNorm8_t vGCs8NormalSphere[ FGCDATA_NORMAL_SPHERE_MAX_INDEX ];
	FGCNorm8_t *vGCs8NormalSphere;

	vGCs8NormalSphere = (FGCNorm8_t *)malloc( FGCDATA_NORMAL_SPHERE_MAX_INDEX * sizeof( FGCNorm8_t ) );
	memset( vGCs8NormalSphere, 0, FGCDATA_NORMAL_SPHERE_MAX_INDEX * sizeof( FGCNorm8_t ) );

	vGCs8NormalSphere[FGCDATA_NORMAL_SPHERE_NORMAL_NULL].nx = 0;
	vGCs8NormalSphere[FGCDATA_NORMAL_SPHERE_NORMAL_NULL].ny = 0;
	vGCs8NormalSphere[FGCDATA_NORMAL_SPHERE_NORMAL_NULL].nz = 0;
	
	vGCs8NormalSphere[FGCDATA_NORMAL_SPHERE_NORMAL_UP].nx = 0;
	vGCs8NormalSphere[FGCDATA_NORMAL_SPHERE_NORMAL_UP].ny = 64;
	vGCs8NormalSphere[FGCDATA_NORMAL_SPHERE_NORMAL_UP].nz = 0;
	
	vGCs8NormalSphere[FGCDATA_NORMAL_SPHERE_NORMAL_DOWN].nx = 0;
	vGCs8NormalSphere[FGCDATA_NORMAL_SPHERE_NORMAL_DOWN].ny = -64;
	vGCs8NormalSphere[FGCDATA_NORMAL_SPHERE_NORMAL_DOWN].nz = 0;

	vGCs8NormalSphere[FGCDATA_NORMAL_SPHERE_NORMAL_RIGHT].nx = 64;
	vGCs8NormalSphere[FGCDATA_NORMAL_SPHERE_NORMAL_RIGHT].ny = 0;
	vGCs8NormalSphere[FGCDATA_NORMAL_SPHERE_NORMAL_RIGHT].nz = 0;

	vGCs8NormalSphere[FGCDATA_NORMAL_SPHERE_NORMAL_LEFT].nx = -64;
	vGCs8NormalSphere[FGCDATA_NORMAL_SPHERE_NORMAL_LEFT].ny = 0;
	vGCs8NormalSphere[FGCDATA_NORMAL_SPHERE_NORMAL_LEFT].nz = 0;

	vGCs8NormalSphere[FGCDATA_NORMAL_SPHERE_NORMAL_FORWARD].nx = 0;
	vGCs8NormalSphere[FGCDATA_NORMAL_SPHERE_NORMAL_FORWARD].ny = 0;
	vGCs8NormalSphere[FGCDATA_NORMAL_SPHERE_NORMAL_FORWARD].nz = 64;

	vGCs8NormalSphere[FGCDATA_NORMAL_SPHERE_NORMAL_BACKWARD].nx = 0;
	vGCs8NormalSphere[FGCDATA_NORMAL_SPHERE_NORMAL_BACKWARD].ny = 0;
	vGCs8NormalSphere[FGCDATA_NORMAL_SPHERE_NORMAL_BACKWARD].nz = -64;

	f32 fAngle;
	u32 i, iIndex = 1;
	for ( fAngle = 0; fAngle <= 90; fAngle += 1.f )
	{
		// Calculate Y
		u8 y = (u8)(cos( FMATH_DEG2RAD(fAngle) ) * FGCDATA_NORMAL_SPHERE_FIXED_POINT);
		f32 fSliceRadius = (f32)sin( FMATH_DEG2RAD(fAngle) );
		f32 fIncrement = 90.f / fAngle;
		for ( i = (u32)fAngle; i > 0; i-- )
		{
			u8 x = (u8)(fSliceRadius * sin( FMATH_DEG2RAD( fIncrement * (f32)i ) ) * FGCDATA_NORMAL_SPHERE_FIXED_POINT);
			u8 z = (u8)(fSliceRadius * cos( FMATH_DEG2RAD( fIncrement * (f32)i ) ) * FGCDATA_NORMAL_SPHERE_FIXED_POINT);
			
			// Upper Hemisphere
			vGCs8NormalSphere[iIndex].nx = x;
			vGCs8NormalSphere[iIndex].ny = y;
			vGCs8NormalSphere[iIndex].nz = z;
			
			vGCs8NormalSphere[iIndex + FGCDATA_NORMAL_SPHERE_OCT_OFFSET].nx = -x;
			vGCs8NormalSphere[iIndex + FGCDATA_NORMAL_SPHERE_OCT_OFFSET].ny = y;
			vGCs8NormalSphere[iIndex + FGCDATA_NORMAL_SPHERE_OCT_OFFSET].nz = z;

			vGCs8NormalSphere[iIndex + (FGCDATA_NORMAL_SPHERE_OCT_OFFSET * 2)].nx = -x;
			vGCs8NormalSphere[iIndex + (FGCDATA_NORMAL_SPHERE_OCT_OFFSET * 2)].ny = y;
			vGCs8NormalSphere[iIndex + (FGCDATA_NORMAL_SPHERE_OCT_OFFSET * 2)].nz = -z;
			
			vGCs8NormalSphere[iIndex + (FGCDATA_NORMAL_SPHERE_OCT_OFFSET * 3)].nx = x;
			vGCs8NormalSphere[iIndex + (FGCDATA_NORMAL_SPHERE_OCT_OFFSET * 3)].ny = y;
			vGCs8NormalSphere[iIndex + (FGCDATA_NORMAL_SPHERE_OCT_OFFSET * 3)].nz = -z;
			
			// Lower Hemisphere
			vGCs8NormalSphere[iIndex + (FGCDATA_NORMAL_SPHERE_OCT_OFFSET * 4)].nx = x;
			vGCs8NormalSphere[iIndex + (FGCDATA_NORMAL_SPHERE_OCT_OFFSET * 4)].ny = -y;
			vGCs8NormalSphere[iIndex + (FGCDATA_NORMAL_SPHERE_OCT_OFFSET * 4)].nz = z;
			
			vGCs8NormalSphere[iIndex + (FGCDATA_NORMAL_SPHERE_OCT_OFFSET * 5)].nx = -x;
			vGCs8NormalSphere[iIndex + (FGCDATA_NORMAL_SPHERE_OCT_OFFSET * 5)].ny = -y;
			vGCs8NormalSphere[iIndex + (FGCDATA_NORMAL_SPHERE_OCT_OFFSET * 5)].nz = z;
			
			vGCs8NormalSphere[iIndex + (FGCDATA_NORMAL_SPHERE_OCT_OFFSET * 6)].nx = -x;
			vGCs8NormalSphere[iIndex + (FGCDATA_NORMAL_SPHERE_OCT_OFFSET * 6)].ny = -y;
			vGCs8NormalSphere[iIndex + (FGCDATA_NORMAL_SPHERE_OCT_OFFSET * 6)].nz = -z;
			
			vGCs8NormalSphere[iIndex + (FGCDATA_NORMAL_SPHERE_OCT_OFFSET * 7)].nx = x;
			vGCs8NormalSphere[iIndex + (FGCDATA_NORMAL_SPHERE_OCT_OFFSET * 7)].ny = -y;
			vGCs8NormalSphere[iIndex + (FGCDATA_NORMAL_SPHERE_OCT_OFFSET * 7)].nz = -z;

			iIndex++;
			FASSERT( iIndex + (FGCDATA_NORMAL_SPHERE_OCT_OFFSET * 7) < FGCDATA_NORMAL_SPHERE_MAX_INDEX );
		}
	}
	
	fAngle = 0;

	// Write out the normal sphere
	char szLine[128];
	FILE *pFile = fopen( "NormalSphere.h", "w" );
	strcpy( szLine, "FGCNorm8_t vGCs8NormalSphere[] = {\n" );
	fwrite( szLine, strlen( szLine ), 1, pFile );
	for ( i = 0; i < 32780; i++ )
	{
		sprintf( szLine, "\t%d,\t%d,\t%d,\n", vGCs8NormalSphere[i].nx,vGCs8NormalSphere[i].ny,vGCs8NormalSphere[i].nz );
		fwrite( szLine, strlen( szLine ), 1, pFile );
	}
	strcpy( szLine, "};" );
	fwrite( szLine, strlen( szLine ), 1, pFile );
	fclose( pFile );

	printf( "\n" );
}