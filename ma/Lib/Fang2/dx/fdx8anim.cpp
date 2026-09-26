//////////////////////////////////////////////////////////////////////////////////////
// fDX8anim.cpp - Fang platform specific animation functions
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
// 07/16/02	Lafleur		Created.
//////////////////////////////////////////////////////////////////////////////////////

#include "fang.h"
#include "fanim.h"
#include "fdx8anim.inl"

#define _COMP_LERP_FLOAT( result, x, f1, f2 )	result = (((f32)f1 * (1.f - x)) + ((f32)f2 * x)) * FANIM_COMP_TRANSLATION_CONVERTER


//
//
//
void CFAnimInst::ComputeFrame( CFAnimFrame &rDestFrame, u32 nBoneIndex, u32 nLerpType ) const 
{
	FAnimSKey_t *paSKeyData;

	FASSERT( nBoneIndex < GetBoneCount() );

	// Determine lower key frames
	u16 nSLowKey, nTLowKey, nOLowKey;
	if ( m_pAnim->nFlags & FANIM_BONEFLAGS_8BIT_FRAMECOUNT )
	{
		// Use the bone state array key indices
		CFAnimInstState8 *pState = &((CFAnimInstState8 *)m_pBoneStateArray)[nBoneIndex];
		nTLowKey = pState->m_nLowTKeyIndex;
		nOLowKey = pState->m_nLowOKeyIndex;
		nSLowKey = pState->m_nLowSKeyIndex;
	}
	else
	{
		// Use the bone state array key indices
		CFAnimInstState16 *pState = &((CFAnimInstState16 *)m_pBoneStateArray)[nBoneIndex];
		nTLowKey = pState->m_nLowTKeyIndex;
		nOLowKey = pState->m_nLowOKeyIndex;
		nSLowKey = pState->m_nLowSKeyIndex;
	}
	
	FASSERT( (nTLowKey + 1) < m_pAnim->pBoneArray[nBoneIndex].nTKeyCount );
	FASSERT( (nOLowKey + 1) < m_pAnim->pBoneArray[nBoneIndex].nOKeyCount );
	FASSERT( (nSLowKey + 1) < m_pAnim->pBoneArray[nBoneIndex].nSKeyCount );

	if ( nLerpType != LERP_KEYS )
	{
		// Generate Translation component of frame
		if ( m_pAnim->nFlags & FANIM_BONEFLAGS_COMP_TRANSLATION )
		{
			FAnimCTKey_t *paTKeyData = (FAnimCTKey_t *)m_pAnim->pBoneArray[nBoneIndex].paTKeyData;
			rDestFrame.m_Pos.x = paTKeyData[nTLowKey].nCX * FANIM_COMP_TRANSLATION_CONVERTER;
			rDestFrame.m_Pos.y = paTKeyData[nTLowKey].nCY * FANIM_COMP_TRANSLATION_CONVERTER;
			rDestFrame.m_Pos.z = paTKeyData[nTLowKey].nCZ * FANIM_COMP_TRANSLATION_CONVERTER;
		}
		else
		{
			FAnimTKey_t *paTKeyData = (FAnimTKey_t *)m_pAnim->pBoneArray[nBoneIndex].paTKeyData;
			rDestFrame.m_Pos = paTKeyData[nTLowKey].vPosition;
		}

		// Generate Orientation component of frame
		if ( m_pAnim->nFlags & FANIM_BONEFLAGS_COMP_ORIENTATION )
		{
			FAnimCOKey_t *paOKeyData = (FAnimCOKey_t *)m_pAnim->pBoneArray[nBoneIndex].paOKeyData;

			rDestFrame.v.x = paOKeyData[nOLowKey].nCX * FANIM_COMP_ORIENTATION_CONVERTER;
			rDestFrame.v.y = paOKeyData[nOLowKey].nCY * FANIM_COMP_ORIENTATION_CONVERTER;
			rDestFrame.v.z = paOKeyData[nOLowKey].nCZ * FANIM_COMP_ORIENTATION_CONVERTER;
			rDestFrame.v.w = paOKeyData[nOLowKey].nCW * FANIM_COMP_ORIENTATION_CONVERTER;
		}
		else
		{
			FAnimOKey_t *paOKeyData = (FAnimOKey_t *)m_pAnim->pBoneArray[nBoneIndex].paOKeyData;
			CFQuatA *pQuat = &rDestFrame;
			pQuat->Set( paOKeyData[nOLowKey].qOrientation );
		}

		// Generate Scale component of frame
		paSKeyData = (FAnimSKey_t *)m_pAnim->pBoneArray[nBoneIndex].paSKeyData;
		rDestFrame.m_fScale = paSKeyData[nSLowKey].fScale;
	}
	else
	{
		f32 fUnitSSlider, fUnitTSlider, fUnitOSlider;
		
		// Determine the unit time slider between the prior key frame and the next key frame
		if ( m_pAnim->fTotalSeconds - m_fAnimSeconds < 0.01f )
		{
			// We're at the end of the animation, so set the slider to the end
			fUnitSSlider = 1.f;
			fUnitTSlider = 1.f;
			fUnitOSlider = 1.f;
		}
		else
		{
			FAnimBone_t *pBone = &m_pAnim->pBoneArray[nBoneIndex];
			
			if( m_pAnim->nFlags & FANIM_BONEFLAGS_8BIT_SECS )
			{
				u8 *paKeyTime;

				// Get intra-frame time		
				paKeyTime = (u8 *)pBone->paTKeyUnitTime;
				fUnitTSlider = fanim_GenerateRatio_8bit( m_fUnitAnimSecs, paKeyTime[nTLowKey], paKeyTime[nTLowKey+1] );
				paKeyTime = (u8 *)pBone->paOKeyUnitTime;
				fUnitOSlider = fanim_GenerateRatio_8bit( m_fUnitAnimSecs, paKeyTime[nOLowKey], paKeyTime[nOLowKey+1] );
				paKeyTime = (u8 *)pBone->paSKeyUnitTime;
				fUnitSSlider = fanim_GenerateRatio_8bit( m_fUnitAnimSecs, paKeyTime[nSLowKey], paKeyTime[nSLowKey+1] );
			}
			else if( m_pAnim->nFlags & FANIM_BONEFLAGS_16BIT_SECS )
			{
				u16 *paKeyTime;

				// Get intra-frame time		
				paKeyTime = (u16 *)pBone->paTKeyUnitTime;
				fUnitTSlider = fanim_GenerateRatio_16bit( m_fUnitAnimSecs, paKeyTime[nTLowKey], paKeyTime[nTLowKey+1] );
				paKeyTime = (u16 *)pBone->paOKeyUnitTime;
				fUnitOSlider = fanim_GenerateRatio_16bit( m_fUnitAnimSecs, paKeyTime[nOLowKey], paKeyTime[nOLowKey+1] );
				paKeyTime = (u16 *)pBone->paSKeyUnitTime;
				fUnitSSlider = fanim_GenerateRatio_16bit( m_fUnitAnimSecs, paKeyTime[nSLowKey], paKeyTime[nSLowKey+1] );
			}
			else
			{
				f32 *paKeyTime;

				// Get intra-frame time		
				paKeyTime = (f32 *)pBone->paTKeyUnitTime;
				fUnitTSlider = fanim_GenerateRatio_32bit( m_fUnitAnimSecs, paKeyTime[nTLowKey], paKeyTime[nTLowKey+1] );
				paKeyTime = (f32 *)pBone->paOKeyUnitTime;
				fUnitOSlider = fanim_GenerateRatio_32bit( m_fUnitAnimSecs, paKeyTime[nOLowKey], paKeyTime[nOLowKey+1] );
				paKeyTime = (f32 *)pBone->paSKeyUnitTime;
				fUnitSSlider = fanim_GenerateRatio_32bit( m_fUnitAnimSecs, paKeyTime[nSLowKey], paKeyTime[nSLowKey+1] );
			}
		}

		FMATH_CLAMP( fUnitOSlider, 0.f, 1.f );
		FMATH_CLAMP( fUnitTSlider, 0.f, 1.f );
		FMATH_CLAMP( fUnitSSlider, 0.f, 1.f );

		// Generate Translation component of frame
		if ( m_pAnim->nFlags & FANIM_BONEFLAGS_COMP_TRANSLATION )
		{
			FAnimCTKey_t *paTKeyData = (FAnimCTKey_t *)m_pAnim->pBoneArray[nBoneIndex].paTKeyData;

			// Optimize me!
			_COMP_LERP_FLOAT( rDestFrame.m_Pos.x, fUnitTSlider, paTKeyData[nTLowKey].nCX, paTKeyData[nTLowKey + 1].nCX );
			_COMP_LERP_FLOAT( rDestFrame.m_Pos.y, fUnitTSlider, paTKeyData[nTLowKey].nCY, paTKeyData[nTLowKey + 1].nCY );
			_COMP_LERP_FLOAT( rDestFrame.m_Pos.z, fUnitTSlider, paTKeyData[nTLowKey].nCZ, paTKeyData[nTLowKey + 1].nCZ );
		}
		else
		{
			FAnimTKey_t *paTKeyData = (FAnimTKey_t *)m_pAnim->pBoneArray[nBoneIndex].paTKeyData;

			rDestFrame.m_Pos.ReceiveLerpOf( fUnitTSlider, paTKeyData[nTLowKey].vPosition, paTKeyData[nTLowKey + 1].vPosition );
		}

		// Generate Orientation component of frame
		if ( m_pAnim->nFlags & FANIM_BONEFLAGS_COMP_ORIENTATION )
		{
			FAnimCOKey_t *paOKeyData = (FAnimCOKey_t *)m_pAnim->pBoneArray[nBoneIndex].paOKeyData;

			// Optimize me!
			CFQuat qLowQuat, qHighQuat, qResult;
			qLowQuat.x = paOKeyData[nOLowKey].nCX * FANIM_COMP_ORIENTATION_CONVERTER;
			qLowQuat.y = paOKeyData[nOLowKey].nCY * FANIM_COMP_ORIENTATION_CONVERTER;
			qLowQuat.z = paOKeyData[nOLowKey].nCZ * FANIM_COMP_ORIENTATION_CONVERTER;
			qLowQuat.w = paOKeyData[nOLowKey].nCW * FANIM_COMP_ORIENTATION_CONVERTER;
			qHighQuat.x = paOKeyData[nOLowKey + 1].nCX * FANIM_COMP_ORIENTATION_CONVERTER;
			qHighQuat.y = paOKeyData[nOLowKey + 1].nCY * FANIM_COMP_ORIENTATION_CONVERTER;
			qHighQuat.z = paOKeyData[nOLowKey + 1].nCZ * FANIM_COMP_ORIENTATION_CONVERTER;
			qHighQuat.w = paOKeyData[nOLowKey + 1].nCW * FANIM_COMP_ORIENTATION_CONVERTER;
			qResult.ReceiveSlerpOf( fUnitOSlider, qLowQuat, qHighQuat );
			rDestFrame.v.Set( qResult );
		}
		else
		{
			FAnimOKey_t *paOKeyData = (FAnimOKey_t *)m_pAnim->pBoneArray[nBoneIndex].paOKeyData;
			CFQuatA *pQuat = &rDestFrame;
			pQuat->ReceiveSlerpOf( fUnitOSlider, paOKeyData[nOLowKey].qOrientation, paOKeyData[nOLowKey + 1].qOrientation );
		}

		// Generate Scale component of frame
		paSKeyData = (FAnimSKey_t *)m_pAnim->pBoneArray[nBoneIndex].paSKeyData;
		rDestFrame.m_fScale = (paSKeyData[nSLowKey].fScale * (1 - fUnitSSlider)) + (paSKeyData[nSLowKey + 1].fScale * fUnitSSlider);
	}
/*
	if ( fmath_Fcheck( rDestFrame.m_fScale ) != FMATH_FCHECK_RESULT_OK 	)
	{
		OutputDebugString( "Bad scale in Anim System!!" );
	}

	u32 i;
	for ( i = 0; i < 4; i++ )
	{
		if ( fmath_Fcheck( rDestFrame.m_Pos.a[0] ) != FMATH_FCHECK_RESULT_OK 	)
		{
			OutputDebugString( "Bad position in Anim System!!" );
		}
	}
	for ( i = 0; i < 4; i++ )
	{
		if ( fmath_Fcheck( rDestFrame.a[0] ) != FMATH_FCHECK_RESULT_OK 	)
		{
			OutputDebugString( "Bad orientation in Anim System!!" );
		}
	}
*/
}
