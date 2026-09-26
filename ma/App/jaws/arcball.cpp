//////////////////////////////////////////////////////////////////////////////////////
// arcball.cpp - Arcball UI class.
//
// Author: Steve Ranck     
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
// 02/13/01 Ranck       Created.
//////////////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "fang.h"
#include "arcball.h"
#include "fvid.h"
#include "fmath.h"


#if 0
class CTestClass {
public:
	FINLINE CTestClass() {}

	CFMtx43A m;
	CFVec3A V3, V4;

	FANG_CLASS_REQUIRES_ALIGNMENT;
};

f32 TestFunc( CFVec3A *pV1, CFVec3A *pV2 ) {
#if 0
	CFVec3A V3, V4;

	V3.Mul( *pV1, *pV2 );
	V4.Sub( *pV1, *pV2 );
	return V3.Dot( V4 );
#else
	CTestClass TestClass;
	TestClass.V3.Set( pV1->x );
	TestClass.V4.Set( pV2->y );
	return TestClass.V3.Dot( TestClass.V4 ) * TestClass.V3.w;
#endif
}
#endif

CArcBall::CArcBall() {
	D3DXQuaternionIdentity( &m_qDown );
	D3DXQuaternionIdentity( &m_qNow );
	D3DXMatrixIdentity( &m_matRotation );
	D3DXMatrixIdentity( &m_matRotationDelta );
	D3DXMatrixIdentity( &m_matTranslation );
	D3DXMatrixIdentity( &m_matTranslationDelta );

	m_Mtx.Identity();

	m_bDrag = FALSE;
	m_fRadius = 0.9f;
	m_fRadius2 = 2.0f;
}

D3DXVECTOR3 CArcBall::ScreenToVector( int sx, int sy ) {
	// Scale to screen
	float x   = -(sx - FVid_Mode.nPixelsAcross/2)  / (m_fRadius*FVid_Mode.nPixelsAcross/2);
	float y   =  (sy - FVid_Mode.nPixelsDown/2) / (m_fRadius*FVid_Mode.nPixelsDown/2);
	float z   = 0.0f;
	float mag = x*x + y*y;

	if( mag > 1.0f ) {
		float scale = 1.0f/sqrtf(mag);
		x *= scale;
		y *= scale;
	} else {
		z = sqrtf( 1.0f - mag );
	}

	// Return vector
	return D3DXVECTOR3( x, y, z );
}

void CArcBall::SetRadius( f32 fRadius1, f32 fRadius2 ) {
    m_fRadius = fRadius1;
    m_fRadius2 = fRadius2;
}

void CArcBall::BeginRotate( u32 nMouseX, u32 nMouseY ) {
	m_matRotation.m[0][0] = m_Mtx.aa[0][0];
	m_matRotation.m[0][1] = m_Mtx.aa[0][1];
	m_matRotation.m[0][2] = m_Mtx.aa[0][2];
	m_matRotation.m[1][0] = m_Mtx.aa[1][0];
	m_matRotation.m[1][1] = m_Mtx.aa[1][1];
	m_matRotation.m[1][2] = m_Mtx.aa[1][2];
	m_matRotation.m[2][0] = m_Mtx.aa[2][0];
	m_matRotation.m[2][1] = m_Mtx.aa[2][1];
	m_matRotation.m[2][2] = m_Mtx.aa[2][2];
	m_matTranslation.m[3][0] = m_Mtx.aa[3][0];
	m_matTranslation.m[3][1] = m_Mtx.aa[3][1];
	m_matTranslation.m[3][2] = m_Mtx.aa[3][2];

	m_bDrag = TRUE;
	m_vDown = ScreenToVector( nMouseX, nMouseY );
}

void CArcBall::EndRotate( u32 nMouseX, u32 nMouseY ) {
	m_bDrag = FALSE;
	m_qDown = m_qNow;
}

void CArcBall::Rotate( u32 nMouseX, u32 nMouseY ) {
	if( m_bDrag ) {
		D3DXVECTOR3 vPart;
		D3DXVECTOR3 vCur = ScreenToVector( nMouseX, nMouseY );
		D3DXVec3Cross( &vPart, &m_vDown, &vCur );
		m_qNow = m_qDown * D3DXQUATERNION( vPart.x, vPart.y, vPart.z, D3DXVec3Dot( &m_vDown, &vCur ) );
	}

	D3DXQUATERNION qConj;
	D3DXQuaternionConjugate( &qConj, &m_qNow );

	D3DXMatrixRotationQuaternion( &m_matRotationDelta, &qConj );
	D3DXMatrixTranspose( &m_matRotationDelta, &m_matRotationDelta );
	D3DXMatrixMultiply( &m_matRotation, &m_matRotation, &m_matRotationDelta );

	m_Mtx.aa[0][0] = m_matRotation.m[0][0];
	m_Mtx.aa[0][1] = m_matRotation.m[0][1];
	m_Mtx.aa[0][2] = m_matRotation.m[0][2];
	m_Mtx.aa[1][0] = m_matRotation.m[1][0];
	m_Mtx.aa[1][1] = m_matRotation.m[1][1];
	m_Mtx.aa[1][2] = m_matRotation.m[1][2];
	m_Mtx.aa[2][0] = m_matRotation.m[2][0];
	m_Mtx.aa[2][1] = m_matRotation.m[2][1];
	m_Mtx.aa[2][2] = m_matRotation.m[2][2];

	D3DXQuaternionIdentity( &m_qDown );
	D3DXQuaternionIdentity( &m_qNow );
	m_vDown = ScreenToVector( nMouseX, nMouseY );
	m_bDrag = TRUE;
}

void CArcBall::BeginTranslate( u32 nMouseX, u32 nMouseY ) {
	m_matRotation.m[0][0] = m_Mtx.aa[0][0];
	m_matRotation.m[0][1] = m_Mtx.aa[0][1];
	m_matRotation.m[0][2] = m_Mtx.aa[0][2];
	m_matRotation.m[1][0] = m_Mtx.aa[1][0];
	m_matRotation.m[1][1] = m_Mtx.aa[1][1];
	m_matRotation.m[1][2] = m_Mtx.aa[1][2];
	m_matRotation.m[2][0] = m_Mtx.aa[2][0];
	m_matRotation.m[2][1] = m_Mtx.aa[2][1];
	m_matRotation.m[2][2] = m_Mtx.aa[2][2];
	m_matTranslation.m[3][0] = m_Mtx.aa[3][0];
	m_matTranslation.m[3][1] = m_Mtx.aa[3][1];
	m_matTranslation.m[3][2] = m_Mtx.aa[3][2];

	m_nLastMouseX = nMouseX;
	m_nLastMouseY = nMouseY;
}

void CArcBall::EndTranslate( u32 nMouseX, u32 nMouseY ) {
}

void CArcBall::Translate( u32 nMouseX, u32 nMouseY ) {
	// Normalize based on size of window and bounding sphere radius
	f32 fDeltaX = (f32)( (s32)m_nLastMouseX-(s32)nMouseX ) * m_fRadius2 / FVid_Mode.nPixelsAcross;
	f32 fDeltaY = (f32)( (s32)m_nLastMouseY-(s32)nMouseY ) * m_fRadius2 / FVid_Mode.nPixelsDown;

	D3DXMatrixTranslation( &m_matTranslationDelta, -2*fDeltaX, 2*fDeltaY, 0.0f );
	D3DXMatrixMultiply( &m_matTranslation, &m_matTranslation, &m_matTranslationDelta );
	m_Mtx.aa[3][0] = m_matTranslation.m[3][0];
	m_Mtx.aa[3][1] = m_matTranslation.m[3][1];
	m_Mtx.aa[3][2] = m_matTranslation.m[3][2];

	// Store mouse coordinate
	m_nLastMouseX = nMouseX;
	m_nLastMouseY = nMouseY;
}

void CArcBall::BeginTranslateZ( u32 nMouseX, u32 nMouseY ) {
	m_matRotation.m[0][0] = m_Mtx.aa[0][0];
	m_matRotation.m[0][1] = m_Mtx.aa[0][1];
	m_matRotation.m[0][2] = m_Mtx.aa[0][2];
	m_matRotation.m[1][0] = m_Mtx.aa[1][0];
	m_matRotation.m[1][1] = m_Mtx.aa[1][1];
	m_matRotation.m[1][2] = m_Mtx.aa[1][2];
	m_matRotation.m[2][0] = m_Mtx.aa[2][0];
	m_matRotation.m[2][1] = m_Mtx.aa[2][1];
	m_matRotation.m[2][2] = m_Mtx.aa[2][2];
	m_matTranslation.m[3][0] = m_Mtx.aa[3][0];
	m_matTranslation.m[3][1] = m_Mtx.aa[3][1];
	m_matTranslation.m[3][2] = m_Mtx.aa[3][2];

	m_nLastMouseX = nMouseX;
	m_nLastMouseY = nMouseY;
}

void CArcBall::EndTranslateZ( u32 nMouseX, u32 nMouseY ) {
}

void CArcBall::TranslateZ( u32 nMouseX, u32 nMouseY ) {
	// Normalize based on size of window and bounding sphere radius
	f32 fDeltaZ = (f32)( (s32)m_nLastMouseX-(s32)nMouseX ) * m_fRadius2 / FVid_Mode.nPixelsAcross;

	D3DXMatrixTranslation( &m_matTranslationDelta, 0.0f, 0.0f, -2*fDeltaZ );
	D3DXMatrixMultiply( &m_matTranslation, &m_matTranslation, &m_matTranslationDelta );
	m_Mtx.aa[3][0] = m_matTranslation.m[3][0];
	m_Mtx.aa[3][1] = m_matTranslation.m[3][1];
	m_Mtx.aa[3][2] = m_matTranslation.m[3][2];

	// Store mouse coordinate
	m_nLastMouseX = nMouseX;
	m_nLastMouseY = nMouseY;
}


