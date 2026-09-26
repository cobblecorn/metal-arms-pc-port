//////////////////////////////////////////////////////////////////////////////////////
// arcball.h - Arcball UI class.
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

#ifndef _ARCBALL_H_
#define _ARCBALL_H_ 1

#include "fang.h"
#include "fmath.h"

#include <d3dx8.h>



FCLASS_ALIGN_PREFIX class CArcBall {
	float m_fRadius;					// ArcBall's radius in screen coords
	float m_fRadius2;					// ArcBall's radius in screen coords

	u32 m_nLastMouseX;
	u32 m_nLastMouseY;
	D3DXVECTOR3 m_vDown;
	D3DXVECTOR3 m_vCur;

	D3DXQUATERNION m_qDown;				// Quaternion before button down
	D3DXQUATERNION m_qNow;				// Composite quaternion for current drag
	D3DXMATRIX m_matRotation;			// Matrix for arcball's orientation
	D3DXMATRIX m_matRotationDelta;		// Matrix for arcball's orientation
	D3DXMATRIX m_matTranslation;		// Matrix for arcball's position
	D3DXMATRIX m_matTranslationDelta;	// Matrix for arcball's position
	BOOL m_bDrag;						// Whether user is dragging arcball

	D3DXVECTOR3 ScreenToVector( int sx, int sy );

public:
	CFMtx43A m_Mtx;

	void BeginRotate( u32 nMouseX, u32 nMouseY );
	void EndRotate( u32 nMouseX, u32 nMouseY );
	void Rotate( u32 nMouseX, u32 nMouseY );
	void BeginTranslate( u32 nMouseX, u32 nMouseY );
	void EndTranslate( u32 nMouseX, u32 nMouseY );
	void Translate( u32 nMouseX, u32 nMouseY );
	void BeginTranslateZ( u32 nMouseX, u32 nMouseY );
	void EndTranslateZ( u32 nMouseX, u32 nMouseY );
	void TranslateZ( u32 nMouseX, u32 nMouseY );

	BOOL IsBeingDragged( void ) { return m_bDrag; }

	void SetRadius( f32 fRadius1, f32 fRadius2 );

	CArcBall();
	FCLASS_STACKMEM_ALIGN( CArcBall );
} FCLASS_ALIGN_SUFFIX;



#endif

