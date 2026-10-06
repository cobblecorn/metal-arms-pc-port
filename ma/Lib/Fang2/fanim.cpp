//////////////////////////////////////////////////////////////////////////////////////
// fanim.cpp - Fang animation module.
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
// 01/29/01 Ranck       Created.
//////////////////////////////////////////////////////////////////////////////////////

#include "fang.h"

//ARG - >>>>>
#if FANG_PLATFORM_PS2
	#pragma	implementation
#endif
//ARG - <<<<<

#include "fanim.h"
#include "fresload.h"
#include "fdata.h"
#include "fclib.h"
#include "floop.h"
#include "fperf.h"
#include "sas_user.h"

#if FANG_PLATFORM_GC
	#include "gc\fgcanim.inl"
#elif FANG_PLATFORM_DX
	#include "dx\fdx8anim.inl"
//ARG - >>>>>
#elif FANG_PLATFORM_PS2
	#include "ps2\fps2anim.inl"
//ARG - <<<<<
#else
	#pragma message ("FANIM.CPP - Error - No animation inlines defined for this platform.\n")
#endif


static BOOL _bModuleInitialized;
static FResLoadReg_t _ResLoadRegistration;
static CFAnimFrame *_pAnimFrameResultStack;
static CFMtx43A *_pAnimBoneMtx;

static CFAnimCombinerConfig _SimpleCombinerConfig;
static CFAnimCombinerConfig _SummerCombinerConfig;
static CFAnimCombinerConfig _BlenderCombinerConfig;



static BOOL _ResLoadCreate( FResHandle_t hRes, void *pLoadedBase, u32 nLoadedBytes, cchar *pszResName );
static void _ResLoadDestroy( void *pBase );



BOOL fanim_ModuleStartup( void ) {
	FASSERT( !_bModuleInitialized );

	_pAnimFrameResultStack = fnew CFAnimFrame [ CFAnimMixer::TYPE_MAX_TAP_COUNT ];
	if( _pAnimFrameResultStack == NULL ) {
		DEVPRINTF( "fanim_ModuleStartup(): Could not allocate memory for animation frame result stack.\n" );
		goto _ExitModuleStartup;
	}

	_pAnimBoneMtx = fnew CFMtx43A [ FDATA_MAX_BONE_COUNT ];
	if( _pAnimBoneMtx == NULL ) {
		DEVPRINTF( "fanim_ModuleStartup(): Could not allocate memory for animation bone matrix array.\n" );
		goto _ExitModuleStartup;
	}

	if( !_SimpleCombinerConfig.BeginCreation( FANIM_SIMPLE_COMBINER_CONFIG_TAPNAME, CFAnimMixer::TYPE_TAP ) ) {
		// Trouble creating config object...
		goto _ExitModuleStartup;
	}

	if( !_SimpleCombinerConfig.EndCreation() ) {
		// Trouble ending creation mode...
		goto _ExitModuleStartup;
	}

	if( !_SummerCombinerConfig.BeginCreation( FANIM_SUMMER_COMBINER_CONFIG_MIXERNAME, CFAnimMixer::TYPE_SUMMER) ) {
		// Trouble creating config object...
		goto _ExitModuleStartup;
	}

	_SummerCombinerConfig.AddTap( FANIM_SUMMER_COMBINER_CONFIG_TAPNAME1, FANIM_SUMMER_COMBINER_CONFIG_MIXERNAME, 0 );
	_SummerCombinerConfig.AddTap( FANIM_SUMMER_COMBINER_CONFIG_TAPNAME2, FANIM_SUMMER_COMBINER_CONFIG_MIXERNAME, 1 );

	if( !_SummerCombinerConfig.EndCreation() ) {
		// Trouble ending creation mode...
		goto _ExitModuleStartup;
	}

	if( !_BlenderCombinerConfig.BeginCreation( FANIM_BLENDER_COMBINER_CONFIG_MIXERNAME, CFAnimMixer::TYPE_BLENDER) ) {
		// Trouble creating config object...
		goto _ExitModuleStartup;
	}

	_BlenderCombinerConfig.AddTap( FANIM_BLENDER_COMBINER_CONFIG_TAPNAME1, FANIM_BLENDER_COMBINER_CONFIG_MIXERNAME, 0 );
	_BlenderCombinerConfig.AddTap( FANIM_BLENDER_COMBINER_CONFIG_TAPNAME2, FANIM_BLENDER_COMBINER_CONFIG_MIXERNAME, 1 );

	if( !_BlenderCombinerConfig.EndCreation() ) {
		// Trouble ending creation mode...
		goto _ExitModuleStartup;
	}

	fres_CopyType( _ResLoadRegistration.sResType, FANIM_RESNAME );
	_ResLoadRegistration.pszFileExtension = "mtx";
	_ResLoadRegistration.nMemType = FRESLOAD_MEMTYPE_PERM;
	_ResLoadRegistration.nAlignment = FCLASS_BYTE_ALIGN;
	_ResLoadRegistration.pFcnCreate = _ResLoadCreate;
	_ResLoadRegistration.pFcnDestroy = NULL;

	if( !fresload_RegisterHandler( &_ResLoadRegistration ) ) {
		// Registration failed...
		DEVPRINTF( "fanim_ModuleStartup(): Could not register resource.\n" );
		goto _ExitModuleStartup;
	}

	// Success...

	_bModuleInitialized = TRUE;

	return TRUE;

_ExitModuleStartup:
	// Failure...
	return FALSE;
}

void fanim_ModuleShutdown( void ) {
	if( _bModuleInitialized ) {
		_SimpleCombinerConfig.Destroy();
		_SummerCombinerConfig.Destroy();
		_BlenderCombinerConfig.Destroy();
		_bModuleInitialized = FALSE;
	}
}



//-------------------------------------------------------------------------------------------------------------------
// Animation Resource Object:
//-------------------------------------------------------------------------------------------------------------------
static BOOL _ResLoadCreate( FResHandle_t hRes, void *pLoadedBase, u32 nLoadedBytes, cchar *pszResName ) {
	nLoadedBytes;

	FASSERT( pLoadedBase );

	u32 i;
	#define __FIXUP_POINTER( pointer, type )	if (pointer) pointer = (type *)((u32)pLoadedBase + (u32)pointer)

	FAnim_t *pAnim = (FAnim_t *)pLoadedBase;
	__FIXUP_POINTER( pAnim->pBoneArray, FAnimBone_t );
	fclib_strncpy( pAnim->szName, pszResName, FANIM_ANIMNAME_LEN );

//	if ( pAnim->nFlags & FANIM_BONEFLAGS_8BIT_FRAMECOUNT )
//	{
//		pAnim->nFlags &= ~FANIM_BONEFLAGS_8BIT_FRAMECOUNT;
//	}

	FAnimBone_t *pBone = pAnim->pBoneArray;
	for ( i = 0; i < pAnim->nBoneCount; i++, pBone++ )
	{
		__FIXUP_POINTER( pBone->pszName, char );
		__FIXUP_POINTER( pBone->paSKeyUnitTime, void );
		__FIXUP_POINTER( pBone->paTKeyUnitTime, void );
		__FIXUP_POINTER( pBone->paOKeyUnitTime, void );
		__FIXUP_POINTER( pBone->paSKeyData, void );
		__FIXUP_POINTER( pBone->paTKeyData, void );
		__FIXUP_POINTER( pBone->paOKeyData, void );

		if ( pAnim->nFlags & FANIM_BONEFLAGS_8BIT_FRAMECOUNT )
		{
			FASSERT( pBone->nSKeyCount < 256 );
			FASSERT( pBone->nTKeyCount < 256 );
			FASSERT( pBone->nOKeyCount < 256 );
		}
	}

	fres_SetBase( hRes, pAnim );

	#undef __FIXUP_POINTER

	return TRUE;
}



//-------------------------------------------------------------------------------------------------------------------
// CFAnimSource:
//-------------------------------------------------------------------------------------------------------------------
s32 CFAnimSource::FindBoneIndex( cchar *pszBoneName ) const {
	u32 i;

	for( i=0; i<GetBoneCount(); i++ ) {
		if( !fclib_stricmp( pszBoneName, GetBoneName(i) ) ) {
			return (s32)i;
		}
	}

	return -1;
}



//-------------------------------------------------------------------------------------------------------------------
// CFAnimInst:
//-------------------------------------------------------------------------------------------------------------------
CFAnimInst *CFAnimInst::Load( cchar *pszAnimResName ) {
	FAnim_t *pAnimRes;
	CFAnimInst *pAnimInst = NULL;

	FResFrame_t ResFrame = fres_GetFrame();

	pAnimRes = (FAnim_t *)fresload_Load( FANIM_RESNAME, pszAnimResName );

	if( pAnimRes == NULL ) {
		DEVPRINTF( "demo::_CreateAnimInst(): Could not find animation '%s'\n", pszAnimResName );
		goto _ExitLoadWithError;
	}

	pAnimInst = fnew CFAnimInst;

	if( pAnimInst == NULL ) {
		DEVPRINTF( "CFAnimInst::Load(): Not enough memory to create CFAnimInst for animation '%s'\n", pszAnimResName );
		goto _ExitLoadWithError;
	}

	if( !pAnimInst->Create( pAnimRes ) ) {
		DEVPRINTF( "CFAnimInst::Load(): Trouble creating CFAnimInst for  '%s'\n", pszAnimResName );
		goto _ExitLoadWithError;
	}

	return pAnimInst;

	// Error:
_ExitLoadWithError:
	fdelete( pAnimInst );
	fres_ReleaseFrame( ResFrame );
	return NULL;
}

BOOL CFAnimInst::Create( const FAnim_t *pAnim ) 
{
	FASSERT( !m_pBoneStateArray );
	FASSERT( pAnim );

	m_pAnim = pAnim;

	if ( m_pAnim->nFlags & FANIM_BONEFLAGS_8BIT_FRAMECOUNT )
	{
		m_pBoneStateArray = fres_AllocAndZero( FMATH_BYTE_ALIGN_UP( sizeof( CFAnimInstState8 ) * pAnim->nBoneCount, 4 ) );
		if( m_pBoneStateArray == NULL ) 
		{
			return FALSE;
		}
	}
	else
	{
		m_pBoneStateArray = fres_AllocAndZero( FMATH_BYTE_ALIGN_UP( sizeof( CFAnimInstState16 ) * pAnim->nBoneCount, 4 ) );
		if( m_pBoneStateArray == NULL ) 
		{
			return FALSE;
		}
	}

	return TRUE;
}

//
//
//
static u16 _SetUnitKeySlider( u8 *paKeyTime, u16 nKeyCount, s32 nLowKey, f32 fAnimSecs, f32 fDeltaSecs )
{
	// Hold endpoint keys outside the authored range. Some retail constant
	// tracks have both timestamps at the final frame (factory builder root).
	if( fAnimSecs <= (f32)paKeyTime[0] ) return 0;
	if( fAnimSecs >= (f32)paKeyTime[nKeyCount-1] ) return nKeyCount-2;

	s32 nInitialLowKey = nLowKey;
	s32 nHighKey = nLowKey + 1;
	u8 n8BitAnimSecs = (u8)fAnimSecs;

	if( fDeltaSecs >= 0.0f )
	{
		// Search forward from our current location to find which keys we're between...
		for( nLowKey++, nHighKey++; nHighKey < nKeyCount; nLowKey++, nHighKey++ ) 
		{
			if( n8BitAnimSecs >= paKeyTime[nLowKey] && n8BitAnimSecs <= paKeyTime[nHighKey] ) 
			{
				return (u16)nLowKey;
			}
		}

		// Search from the beginning to find the keys we're between...
		for( nLowKey = 0, nHighKey = 1; nLowKey < nInitialLowKey; nLowKey++, nHighKey++ ) 
		{
			if( n8BitAnimSecs >= paKeyTime[nLowKey] && n8BitAnimSecs <= paKeyTime[nHighKey] ) 
			{
				return (u16)nLowKey;
			}
		}
	} 
	else 
	{
		// Search backward from our current location to find which keys we're between...
		for( nLowKey--, nHighKey--; nLowKey>=0; nLowKey--, nHighKey-- ) 
		{
			if( n8BitAnimSecs >= paKeyTime[nLowKey] && n8BitAnimSecs <= paKeyTime[nHighKey] ) 
			{
				return (u16)nLowKey;
			}
		}

		// Search from the end to find the keys we're between...
		for( nLowKey = nKeyCount - 2, nHighKey = nKeyCount - 1; nLowKey > nInitialLowKey; nLowKey--, nHighKey-- ) 
		{
			if( n8BitAnimSecs >= paKeyTime[nLowKey] && n8BitAnimSecs <= paKeyTime[nHighKey] ) 
			{
				return (u16)nLowKey;
			}
		}
	}

	FASSERT_NOW;
	return 0;
}

//
//
//
static u16 _SetUnitKeySlider( u16 *paKeyTime, u16 nKeyCount, s32 nLowKey, f32 fAnimSecs, f32 fDeltaSecs )
{
	// Hold endpoint keys outside the authored range. Some retail constant
	// tracks have both timestamps at the final frame (factory builder root).
	if( fAnimSecs <= (f32)paKeyTime[0] ) return 0;
	if( fAnimSecs >= (f32)paKeyTime[nKeyCount-1] ) return nKeyCount-2;

	s32 nInitialLowKey = nLowKey;
	s32 nHighKey = nLowKey + 1;
	u16 n16BitAnimSecs = (u16)fAnimSecs;

	if( fDeltaSecs >= 0.0f )
	{
		// Search forward from our current location to find which keys we're between...
		for( nLowKey++, nHighKey++; nHighKey < nKeyCount; nLowKey++, nHighKey++ ) 
		{
			if( n16BitAnimSecs >= paKeyTime[nLowKey] && n16BitAnimSecs <= paKeyTime[nHighKey] ) 
			{
				return (u16)nLowKey;
			}
		}

		// Search from the beginning to find the keys we're between...
		for( nLowKey = 0, nHighKey = 1; nLowKey < nInitialLowKey; nLowKey++, nHighKey++ ) 
		{
			if( n16BitAnimSecs >= paKeyTime[nLowKey] && n16BitAnimSecs <= paKeyTime[nHighKey] ) 
			{
				return (u16)nLowKey;
			}
		}
	} 
	else 
	{
		// Search backward from our current location to find which keys we're between...
		for( nLowKey--, nHighKey--; nLowKey>=0; nLowKey--, nHighKey-- ) 
		{
			if( n16BitAnimSecs >= paKeyTime[nLowKey] && n16BitAnimSecs <= paKeyTime[nHighKey] ) 
			{
				return (u16)nLowKey;
			}
		}

		// Search from the end to find the keys we're between...
		for( nLowKey = nKeyCount - 2, nHighKey = nKeyCount - 1; nLowKey > nInitialLowKey; nLowKey--, nHighKey-- ) 
		{
			if( n16BitAnimSecs >= paKeyTime[nLowKey] && n16BitAnimSecs <= paKeyTime[nHighKey] ) 
			{
				return (u16)nLowKey;
			}
		}
	}

	FASSERT_NOW;
	return 0;
}

//
//
//
static u16 _SetUnitKeySlider( f32 *paKeyTime, u16 nKeyCount, s32 nLowKey, f32 fAnimSecs, f32 fDeltaSecs )
{
	// Hold endpoint keys outside the authored range. Some retail constant
	// tracks have both timestamps at the final frame (factory builder root).
	if( fAnimSecs <= (f32)paKeyTime[0] ) return 0;
	if( fAnimSecs >= (f32)paKeyTime[nKeyCount-1] ) return nKeyCount-2;

	s32 nInitialLowKey = nLowKey;
	s32 nHighKey = nLowKey + 1;

	if( fDeltaSecs >= 0.0f )
	{
		// Search forward from our current location to find which keys we're between...
		for( nLowKey++, nHighKey++; nHighKey < nKeyCount; nLowKey++, nHighKey++ ) 
		{
			if( fAnimSecs >= paKeyTime[nLowKey] && fAnimSecs <= paKeyTime[nHighKey] ) 
			{
				return (u16)nLowKey;
			}
		}

		// Search from the beginning to find the keys we're between...
		for( nLowKey = 0, nHighKey = 1; nLowKey < nInitialLowKey; nLowKey++, nHighKey++ ) 
		{
			if( fAnimSecs >= paKeyTime[nLowKey] && fAnimSecs <= paKeyTime[nHighKey] ) 
			{
				return (u16)nLowKey;
			}
		}
	} 
	else 
	{
		// Search backward from our current location to find which keys we're between...
		for( nLowKey--, nHighKey--; nLowKey>=0; nLowKey--, nHighKey-- ) 
		{
			if( fAnimSecs >= paKeyTime[nLowKey] && fAnimSecs <= paKeyTime[nHighKey] ) 
			{
				return (u16)nLowKey;
			}
		}

		// Search from the end to find the keys we're between...
		for( nLowKey = nKeyCount - 2, nHighKey = nKeyCount - 1; nLowKey > nInitialLowKey; nLowKey--, nHighKey-- ) 
		{
			if( fAnimSecs >= paKeyTime[nLowKey] && fAnimSecs <= paKeyTime[nHighKey] ) 
			{
				return (u16)nLowKey;
			}
		}
	}

	FASSERT_NOW;
	return 0;
}


void CFAnimInst::UpdateLowKeys8( f32 fDeltaTime ) 
{
	u32 nBone;
	FAnimBone_t *pBone;

	// Update the bone state array
	CFAnimInstState8 *pState = (CFAnimInstState8 *)m_pBoneStateArray;
	
	if ( m_pAnim->fTotalSeconds - m_fAnimSeconds < 0.01f )
	{
		// We're at the end of the animation, so loop through the bones setting their key position to the end
		for( nBone=0; nBone<m_pAnim->nBoneCount; nBone++, pState++ ) 
		{
			pState->m_nLowSKeyIndex = (u8)(m_pAnim->pBoneArray[nBone].nSKeyCount - 2);
			pState->m_nLowTKeyIndex = (u8)(m_pAnim->pBoneArray[nBone].nTKeyCount - 2);
			pState->m_nLowOKeyIndex = (u8)(m_pAnim->pBoneArray[nBone].nOKeyCount - 2);
		}
	}
	else if( m_pAnim->nFlags & FANIM_BONEFLAGS_8BIT_SECS )
	{
		// Calculate the appropriate anim secs
		m_fUnitAnimSecs = m_fAnimSeconds * 128.f * m_pAnim->fOOTotalSeconds;
		
		u8  nAnimSecs = (u8)m_fUnitAnimSecs;

		// Loop through the bones to find the bounding keys
		for( nBone=0; nBone < m_pAnim->nBoneCount; nBone++, pState++ ) 
		{
			pBone = &m_pAnim->pBoneArray[nBone];

			// Cast the key unit times appropriately
			u8 *paSKeyTime = (u8 *)pBone->paSKeyUnitTime;
			u8 *paTKeyTime = (u8 *)pBone->paTKeyUnitTime;
			u8 *paOKeyTime = (u8 *)pBone->paOKeyUnitTime;

			// First we need to find the bounding translation keys
			u16 nLowKey = pState->m_nLowTKeyIndex;
			u16 nHighKey = nLowKey + 1;

			// Set the appropriate values for translation
			if ( nAnimSecs < paTKeyTime[nLowKey] || nAnimSecs >= paTKeyTime[nHighKey] )
			{
				// We need to find out which new two key frames we're between...
				pState->m_nLowTKeyIndex = (u8)_SetUnitKeySlider( paTKeyTime, pBone->nTKeyCount, pState->m_nLowTKeyIndex, m_fUnitAnimSecs, fDeltaTime );
			}

			// Next we need to find the bounding orientation keys
			nLowKey = pState->m_nLowOKeyIndex;
			nHighKey = pState->m_nLowOKeyIndex + 1;
			
			if ( nAnimSecs < paOKeyTime[nLowKey] || nAnimSecs >= paOKeyTime[nHighKey] )
			{
				// We need to find out which new two key frames we're between...
				pState->m_nLowOKeyIndex = (u8)_SetUnitKeySlider( paOKeyTime, pBone->nOKeyCount, pState->m_nLowOKeyIndex, m_fUnitAnimSecs, fDeltaTime );
			}

			// Next we need to find the bounding scale keys
			nLowKey = pState->m_nLowSKeyIndex;
			nHighKey = pState->m_nLowSKeyIndex + 1;
			
			if ( nAnimSecs < paSKeyTime[nLowKey] || nAnimSecs >= paSKeyTime[nHighKey] )
			{
				// We need to find out which new two key frames we're between...
				pState->m_nLowSKeyIndex = (u8)_SetUnitKeySlider( paSKeyTime, pBone->nSKeyCount, pState->m_nLowSKeyIndex, m_fUnitAnimSecs, fDeltaTime );
			}
		}
	}
	else if( m_pAnim->nFlags & FANIM_BONEFLAGS_16BIT_SECS )
	{
		// Calculate the appropriate anim secs
		m_fUnitAnimSecs = m_fAnimSeconds * 32768.f * m_pAnim->fOOTotalSeconds;
		
		u16 nAnimSecs = (u16)m_fUnitAnimSecs;

		// Loop through the bones to find the bounding keys
		for( nBone=0; nBone < m_pAnim->nBoneCount; nBone++, pState++ ) 
		{
			pBone = &m_pAnim->pBoneArray[nBone];

			// Cast the key unit times appropriately
			u16 *paTKeyTime = (u16 *)pBone->paTKeyUnitTime;
			u16 *paOKeyTime = (u16 *)pBone->paOKeyUnitTime;
			u16 *paSKeyTime = (u16 *)pBone->paSKeyUnitTime;

			// First we need to find the bounding translation keys
			u16 nLowKey = pState->m_nLowTKeyIndex;
			u16 nHighKey = nLowKey + 1;

			if( nAnimSecs < paTKeyTime[nLowKey] || nAnimSecs >= paTKeyTime[nHighKey] )
			{
				// We need to find out which new two key frames we're between...
				pState->m_nLowTKeyIndex = (u8)_SetUnitKeySlider( paTKeyTime, pBone->nTKeyCount, pState->m_nLowTKeyIndex, m_fUnitAnimSecs, fDeltaTime );
			}

			// Next we need to find the bounding orientation keys
			nLowKey = pState->m_nLowOKeyIndex;
			nHighKey = pState->m_nLowOKeyIndex + 1;
			
			if ( nAnimSecs < paOKeyTime[nLowKey] || nAnimSecs >= paOKeyTime[nHighKey] )
			{
				// We need to find out which new two key frames we're between...
				pState->m_nLowOKeyIndex = (u8)_SetUnitKeySlider( paOKeyTime, pBone->nOKeyCount, pState->m_nLowOKeyIndex, m_fUnitAnimSecs, fDeltaTime );
			}
			
			// Next we need to find the bounding scale keys
			nLowKey = pState->m_nLowSKeyIndex;
			nHighKey = pState->m_nLowSKeyIndex + 1;
			
			if ( nAnimSecs < paSKeyTime[nLowKey] || nAnimSecs >= paSKeyTime[nHighKey] ) 
			{
				// We need to find out which new two key frames we're between...
				pState->m_nLowSKeyIndex = (u8)_SetUnitKeySlider( paSKeyTime, pBone->nSKeyCount, pState->m_nLowSKeyIndex, m_fUnitAnimSecs, fDeltaTime );
			}
		}
	}
	else
	{
		// Calculate the appropriate anim secs
		m_fUnitAnimSecs = m_fAnimSeconds * m_pAnim->fOOTotalSeconds;

		// Loop through the bones to find the bounding keys
		for( nBone=0; nBone < m_pAnim->nBoneCount; nBone++, pState++ ) 
		{
			pBone = &m_pAnim->pBoneArray[nBone];

			// Cast the key unit times appropriately
			f32 *paTKeyTime = (f32 *)pBone->paTKeyUnitTime;
			f32 *paOKeyTime = (f32 *)pBone->paOKeyUnitTime;
			f32 *paSKeyTime = (f32 *)pBone->paSKeyUnitTime;

			// First we need to find the bounding translation keys
			u16 nLowKey = pState->m_nLowTKeyIndex;
			u16 nHighKey = nLowKey + 1;

			if ( m_fUnitAnimSecs < paTKeyTime[nLowKey] || m_fUnitAnimSecs >= paTKeyTime[nHighKey] )
			{
				// We need to find out which new two key frames we're between...
				pState->m_nLowTKeyIndex = (u8)_SetUnitKeySlider( paTKeyTime, pBone->nTKeyCount, pState->m_nLowTKeyIndex, m_fUnitAnimSecs, fDeltaTime );
			}

			// Next we need to find the bounding orientation keys
			nLowKey = pState->m_nLowOKeyIndex;
			nHighKey = pState->m_nLowOKeyIndex + 1;
			
			if ( m_fUnitAnimSecs < paOKeyTime[nLowKey] || m_fUnitAnimSecs >= paOKeyTime[nHighKey] )
			{
				// We need to find out which new two key frames we're between...
				pState->m_nLowOKeyIndex = (u8)_SetUnitKeySlider( paOKeyTime, pBone->nOKeyCount, pState->m_nLowOKeyIndex, m_fUnitAnimSecs, fDeltaTime );
			}
			
			// Next we need to find the bounding scale keys
			nLowKey = pState->m_nLowSKeyIndex;
			nHighKey = pState->m_nLowSKeyIndex + 1;
			
			if ( m_fUnitAnimSecs < paSKeyTime[nLowKey] || m_fUnitAnimSecs >= paSKeyTime[nHighKey] )
			{
				// We need to find out which new two key frames we're between...
				pState->m_nLowSKeyIndex = (u8)_SetUnitKeySlider( paSKeyTime, pBone->nSKeyCount, pState->m_nLowSKeyIndex, m_fUnitAnimSecs, fDeltaTime );
			}
		}
	}
}

void CFAnimInst::UpdateLowKeys16( f32 fDeltaTime ) 
{
	u32 nBone;
	FAnimBone_t *pBone;
	
	// Update the bone state array
	CFAnimInstState16 *pState = (CFAnimInstState16 *)m_pBoneStateArray;
	
	if ( m_pAnim->fTotalSeconds - m_fAnimSeconds < 0.01f )
	{
		// We're at the end of the animation, so loop through the bones setting their key position to the end
		for( nBone=0; nBone<m_pAnim->nBoneCount; nBone++, pState++ ) 
		{
			pState->m_nLowSKeyIndex = (u16)(m_pAnim->pBoneArray[nBone].nSKeyCount - 2);
			pState->m_nLowTKeyIndex = (u16)(m_pAnim->pBoneArray[nBone].nTKeyCount - 2);
			pState->m_nLowOKeyIndex = (u16)(m_pAnim->pBoneArray[nBone].nOKeyCount - 2);
		}
	}
	else if( m_pAnim->nFlags & FANIM_BONEFLAGS_8BIT_SECS )
	{
		// Calculate the appropriate anim secs
		m_fUnitAnimSecs = m_fAnimSeconds * 128.f * m_pAnim->fOOTotalSeconds;
		u8  nAnimSecs = (u8)m_fUnitAnimSecs;

		// Loop through the bones to find the bounding keys
		for( nBone=0; nBone < m_pAnim->nBoneCount; nBone++, pState++ ) 
		{
			pBone = &m_pAnim->pBoneArray[nBone];

			// Cast the key unit times appropriately
			u8 *paSKeyTime = (u8 *)pBone->paSKeyUnitTime;
			u8 *paTKeyTime = (u8 *)pBone->paTKeyUnitTime;
			u8 *paOKeyTime = (u8 *)pBone->paOKeyUnitTime;

			// First we need to find the bounding translation keys
			u16 nLowKey = pState->m_nLowTKeyIndex;
			u16 nHighKey = nLowKey + 1;

			// Set the appropriate values for translation
			if ( nAnimSecs < paTKeyTime[nLowKey] || nAnimSecs >= paTKeyTime[nHighKey] )
			{
				// We need to find out which new two key frames we're between...
				pState->m_nLowTKeyIndex = _SetUnitKeySlider( paTKeyTime, pBone->nTKeyCount, pState->m_nLowTKeyIndex, m_fUnitAnimSecs, fDeltaTime );
			}

			// Next we need to find the bounding orientation keys
			nLowKey = pState->m_nLowOKeyIndex;
			nHighKey = pState->m_nLowOKeyIndex + 1;
			
			if ( nAnimSecs < paOKeyTime[nLowKey] || nAnimSecs >= paOKeyTime[nHighKey] )
			{
				// We need to find out which new two key frames we're between...
				pState->m_nLowOKeyIndex = _SetUnitKeySlider( paOKeyTime, pBone->nOKeyCount, pState->m_nLowOKeyIndex, m_fUnitAnimSecs, fDeltaTime );
			}

			// Next we need to find the bounding scale keys
			nLowKey = pState->m_nLowSKeyIndex;
			nHighKey = pState->m_nLowSKeyIndex + 1;
			
			if ( nAnimSecs < paSKeyTime[nLowKey] || nAnimSecs >= paSKeyTime[nHighKey] )
			{
				// We need to find out which new two key frames we're between...
				pState->m_nLowSKeyIndex = _SetUnitKeySlider( paSKeyTime, pBone->nSKeyCount, pState->m_nLowSKeyIndex, m_fUnitAnimSecs, fDeltaTime );
			}
		}
	}
	else if( m_pAnim->nFlags & FANIM_BONEFLAGS_16BIT_SECS )
	{
		// Calculate the appropriate anim secs
		m_fUnitAnimSecs = m_fAnimSeconds * 32768.f * m_pAnim->fOOTotalSeconds;
		u16 nAnimSecs = (u16)m_fUnitAnimSecs;

		// Loop through the bones to find the bounding keys
		for( nBone=0; nBone < m_pAnim->nBoneCount; nBone++, pState++ ) 
		{
			pBone = &m_pAnim->pBoneArray[nBone];

			// Cast the key unit times appropriately
			u16 *paTKeyTime = (u16 *)pBone->paTKeyUnitTime;
			u16 *paOKeyTime = (u16 *)pBone->paOKeyUnitTime;
			u16 *paSKeyTime = (u16 *)pBone->paSKeyUnitTime;

			// First we need to find the bounding translation keys
			u16 nLowKey = pState->m_nLowTKeyIndex;
			u16 nHighKey = nLowKey + 1;

			if( nAnimSecs < paTKeyTime[nLowKey] || nAnimSecs >= paTKeyTime[nHighKey] )
			{
				// We need to find out which new two key frames we're between...
				pState->m_nLowTKeyIndex = _SetUnitKeySlider( paTKeyTime, pBone->nTKeyCount, pState->m_nLowTKeyIndex, m_fUnitAnimSecs, fDeltaTime );
			}

			// Next we need to find the bounding orientation keys
			nLowKey = pState->m_nLowOKeyIndex;
			nHighKey = pState->m_nLowOKeyIndex + 1;
			
			if ( nAnimSecs < paOKeyTime[nLowKey] || nAnimSecs >= paOKeyTime[nHighKey] )
			{
				// We need to find out which new two key frames we're between...
				pState->m_nLowOKeyIndex = _SetUnitKeySlider( paOKeyTime, pBone->nOKeyCount, pState->m_nLowOKeyIndex, m_fUnitAnimSecs, fDeltaTime );
			}
			
			// Next we need to find the bounding scale keys
			nLowKey = pState->m_nLowSKeyIndex;
			nHighKey = pState->m_nLowSKeyIndex + 1;
			
			if ( nAnimSecs < paSKeyTime[nLowKey] || nAnimSecs >= paSKeyTime[nHighKey] ) 
			{
				// We need to find out which new two key frames we're between...
				pState->m_nLowSKeyIndex = _SetUnitKeySlider( paSKeyTime, pBone->nSKeyCount, pState->m_nLowSKeyIndex, m_fUnitAnimSecs, fDeltaTime );
			}
		}
	}
	else
	{
		// Calculate the appropriate anim secs
		m_fUnitAnimSecs = m_fAnimSeconds * m_pAnim->fOOTotalSeconds;

		// Loop through the bones to find the bounding keys
		for( nBone=0; nBone < m_pAnim->nBoneCount; nBone++, pState++ ) 
		{
			pBone = &m_pAnim->pBoneArray[nBone];

			// Cast the key unit times appropriately
			f32 *paTKeyTime = (f32 *)pBone->paTKeyUnitTime;
			f32 *paOKeyTime = (f32 *)pBone->paOKeyUnitTime;
			f32 *paSKeyTime = (f32 *)pBone->paSKeyUnitTime;

			// First we need to find the bounding translation keys
			u16 nLowKey = pState->m_nLowTKeyIndex;
			u16 nHighKey = nLowKey + 1;

			if ( m_fUnitAnimSecs < paTKeyTime[nLowKey] || m_fUnitAnimSecs >= paTKeyTime[nHighKey] )
			{
				// We need to find out which new two key frames we're between...
				pState->m_nLowTKeyIndex = _SetUnitKeySlider( paTKeyTime, pBone->nTKeyCount, pState->m_nLowTKeyIndex, m_fUnitAnimSecs, fDeltaTime );
			}

			// Next we need to find the bounding orientation keys
			nLowKey = pState->m_nLowOKeyIndex;
			nHighKey = pState->m_nLowOKeyIndex + 1;
			
			if ( m_fUnitAnimSecs < paOKeyTime[nLowKey] || m_fUnitAnimSecs >= paOKeyTime[nHighKey] )
			{
				// We need to find out which new two key frames we're between...
				pState->m_nLowOKeyIndex = _SetUnitKeySlider( paOKeyTime, pBone->nOKeyCount, pState->m_nLowOKeyIndex, m_fUnitAnimSecs, fDeltaTime );
			}
			
			// Next we need to find the bounding scale keys
			nLowKey = pState->m_nLowSKeyIndex;
			nHighKey = pState->m_nLowSKeyIndex + 1;
			
			if ( m_fUnitAnimSecs < paSKeyTime[nLowKey] || m_fUnitAnimSecs >= paSKeyTime[nHighKey] )
			{
				// We need to find out which new two key frames we're between...
				pState->m_nLowSKeyIndex = _SetUnitKeySlider( paSKeyTime, pBone->nSKeyCount, pState->m_nLowSKeyIndex, m_fUnitAnimSecs, fDeltaTime );
			}
		}
	}
}

void CFAnimInst::UpdateTime( f32 fNewTime ) 
{
	f32 fDeltaTime;

	FMATH_DEBUG_FCHECK( fNewTime );

	while( fNewTime > GetTotalTime() ) 
	{
		fNewTime -= GetTotalTime();
	}

	while( fNewTime < 0.0f ) {
		fNewTime += GetTotalTime();
	}

	if( fNewTime == m_fAnimSeconds ) {
		// Time hasn't changed...
		return;
	}

	fDeltaTime = fNewTime - m_fAnimSeconds;
	m_fAnimSeconds = fNewTime;

	// If we don't currently have bone state memory allocated, we just record where we are in the animation
	if ( !m_pBoneStateArray )
	{
		return;
	}
	
	if ( m_pAnim->nFlags & FANIM_BONEFLAGS_8BIT_FRAMECOUNT )
	{
		UpdateLowKeys8( fDeltaTime );
	}
	else
	{
		UpdateLowKeys16( fDeltaTime );
	}
}

BOOL CFAnimInst::DeltaTime( f32 fDeltaTime, BOOL bClamp ) {
	f32 fNewTime;
	BOOL bWrapped = FALSE;

//	FASSERT( m_pBoneStateArray );
	FMATH_DEBUG_FCHECK( fDeltaTime );

	fNewTime = GetTime() + fDeltaTime;

	if( !bClamp ) {
		while( fNewTime > GetTotalTime() ) {
			fNewTime -= GetTotalTime();
			bWrapped = TRUE;
		}

		while( fNewTime < 0.0f ) {
			fNewTime += GetTotalTime();
			bWrapped = TRUE;
		}
	} else {
		if( fNewTime > GetTotalTime() ) {
			fNewTime = GetTotalTime();
			bWrapped = TRUE;
		} else if( fNewTime < 0.0f ) {
			fNewTime = 0.0f;
			bWrapped = TRUE;
		}
	}

	UpdateTime( fNewTime );

	return bWrapped;
}



//-------------------------------------------------------------------------------------------------------------------
// CFAnimManFrame:
//-------------------------------------------------------------------------------------------------------------------
BOOL CFAnimManFrame::Create( u32 nBoneCount, const cchar **ppszBoneNameArray ) {
	u32 i;

	FASSERT( !m_pBoneFrameArray );

	m_nBoneCount = nBoneCount;
	m_ppszBoneNameArray = ppszBoneNameArray;

	if( nBoneCount ) {
		m_pBoneFrameArray = fnew CFAnimFrame[nBoneCount];
		if( m_pBoneFrameArray == NULL ) {
			return FALSE;
		}

		for( i=0; i<nBoneCount; i++ ) {
			m_pBoneFrameArray[i].Zero();
		}
	} else {
		m_pBoneFrameArray = NULL;
	}

	return TRUE;
}


BOOL CFAnimManFrame::Create( const u8 *pnBoneNameIndexTable, cchar **ppszBoneNameTable ) {
	u32 i, nBoneCount;
	cchar **ppszBoneNameArray;

	FASSERT( !m_pBoneFrameArray );

	FResFrame_t ResFrame = fres_GetFrame();

	// Count the number of bones...
	for( nBoneCount=0; pnBoneNameIndexTable[nBoneCount] != 255; ++nBoneCount )
	{
	}

	if( nBoneCount ) {
		ppszBoneNameArray = (cchar **)fres_AllocAndZero( nBoneCount * sizeof(cchar *) );
		if( ppszBoneNameArray == NULL ) {
			goto _ExitWithError;
		}

		for( i=0; i<nBoneCount; ++i ) {
			ppszBoneNameArray[i] = ppszBoneNameTable[ pnBoneNameIndexTable[i] ];
		}

		if( !Create( nBoneCount, ppszBoneNameArray ) ) {
			goto _ExitWithError;
		}
	} else {
		m_pBoneFrameArray = NULL;
	}

	return TRUE;

	// Error:
_ExitWithError:
	fres_ReleaseFrame( ResFrame );
	return FALSE;
}


void CFAnimManFrame::Reset( void ) {
	u32 i;

	FASSERT( m_pBoneFrameArray );

	for( i=0; i<m_nBoneCount; i++ ) {
		m_pBoneFrameArray[i].Identity();
	}
}




//-------------------------------------------------------------------------------------------------------------------
// CFAnimManMtx:
//-------------------------------------------------------------------------------------------------------------------
BOOL CFAnimManMtx::Create( u32 nBoneCount, cchar **ppszBoneNameArray ) {
	FResFrame_t ResFrame;

	FASSERT( !m_pBoneMtxArray );

	ResFrame = fres_GetFrame();

	m_ppszBoneNameArray = NULL;
	m_pBoneMtxArray = NULL;
	m_pFrameArray = NULL;
	m_abMtxChanged = NULL;

	m_nBoneCount = nBoneCount;
	m_ppszBoneNameArray = ppszBoneNameArray;

	if( m_ppszBoneNameArray && nBoneCount ) {
		m_pBoneMtxArray = fnew CFMtx43A[nBoneCount];
		if( m_pBoneMtxArray == NULL ) {
			goto _ExitCreateWithError;
		}

		m_pFrameArray = fnew CFAnimFrame[nBoneCount];
		if( m_pFrameArray == NULL ) {
			goto _ExitCreateWithError;
		}

		m_abMtxChanged = (u8 *)fres_Alloc( nBoneCount * sizeof(u8) );
		if( m_abMtxChanged == NULL ) {
			goto _ExitCreateWithError;
		}
	}

	Reset();

	// Success...

	return TRUE;

_ExitCreateWithError:
	// Failure...
	Destroy();
	fres_ReleaseFrame( ResFrame );
	return FALSE;
}

BOOL CFAnimManMtx::Create( cchar *pszBoneName ) {
	return Create( 1, &pszBoneName );
}

void CFAnimManMtx::Destroy( void ) {
	fdelete_array( m_pBoneMtxArray );
	fdelete_array( m_pFrameArray );

	m_pBoneMtxArray = NULL;
	m_pFrameArray = NULL;
	m_abMtxChanged = NULL;
	m_ppszBoneNameArray = NULL;
}

void CFAnimManMtx::Reset( void ) {
	u32 i;

	FASSERT( m_pBoneMtxArray );

	for( i=0; i<m_nBoneCount; i++ ) {
		m_pBoneMtxArray[i].Identity();
	}

	for( i=0; i<m_nBoneCount; i++ ) {
		m_pFrameArray[i].Identity();
	}

	for( i=0; i<m_nBoneCount; i++ ) {
		m_abMtxChanged[i] = FALSE;
	}
}



//-------------------------------------------------------------------------------------------------------------------
// CFAnimMeshRest
//-------------------------------------------------------------------------------------------------------------------
BOOL CFAnimMeshRest::Create(FMesh_t *pMesh) {
	FASSERT( !m_paTQuat );

	FResFrame_t hResFrame = fres_GetFrame();

	//////////////////////////////////////////////////////////////////////
	//
	m_pMesh = pMesh;
	m_nBoneCount = m_pMesh->nBoneCount;
	//
	//////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////
	//
	if (m_nBoneCount)
	{
		m_paTQuat = fnew CFAnimFrame[m_nBoneCount];
		if( !m_paTQuat ) {
			goto _ExitCreateWithError;
		}
	}
	else
	{
		m_paTQuat = NULL;
	}
	//
	//////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////
	// Initialize the bone TQuatA array.
	u32 uCurBoneIdx;
	for( uCurBoneIdx = 0; uCurBoneIdx < m_nBoneCount; ++uCurBoneIdx ) {
//		m_paTQuat[uCurBoneIdx].BuildQuat( m_pMesh->pBoneArray[uCurBoneIdx].AtRestModelToBoneMtx, TRUE );
		m_paTQuat[uCurBoneIdx].BuildQuat( m_pMesh->pBoneArray[uCurBoneIdx].AtRestBoneToParentMtx, TRUE );
//		m_paTQuat[uCurBoneIdx].BuildQuat( m_pMesh->pBoneArray[uCurBoneIdx].AtRestBoneToModelMtx, TRUE );
	}
	//
	//////////////////////////////////////////////////////////////////////

	// Success...

	return TRUE;

_ExitCreateWithError:
	// Failure...
	fres_ReleaseFrame( hResFrame );
	return FALSE;
}



void CFAnimMeshRest::Destroy( void ) {
	fdelete_array(m_paTQuat);
	m_paTQuat = NULL;
}




//-------------------------------------------------------------------------------------------------------------------
// CFAnimCombinerConfig:
//-------------------------------------------------------------------------------------------------------------------
CFAnimCombinerConfig::CFAnimCombinerConfig() {
	m_bCreationComplete = FALSE;
	m_RootMixer.nCount = 0;
	m_nMixerCount = 0;
	m_nTapCount = 0;
}

BOOL CFAnimCombinerConfig::BeginCreation( cchar *pszRootMixerName, CFAnimMixer::Type_e nRootMixerType ) {
	CFAnimMixer *pMixer;

	if( m_bCreationComplete ) {
		// Creation has already been completed...
		DEVPRINTF( "CFAnimCombinerConfig::BeginCreation(): Creation has already ended.\n" );
		return FALSE;
	}

	if( m_RootMixer.nCount ) {
		// BeginCreation() already called...
		DEVPRINTF( "CFAnimCombinerConfig::BeginCreation(): BeginCreation() has already been started.\n" );
		return FALSE;
	}

	if( pszRootMixerName==NULL || pszRootMixerName[0]==0 ) {
		DEVPRINTF( "CFAnimCombinerConfig::BeginCreation(): Must specify a root mixer name.\n" );
		return FALSE;
	}
	
	if( fclib_strlen(pszRootMixerName) > FANIM_MIXERNAME_LEN ) {
		// Invalid root mixer name...
		DEVPRINTF( "CFAnimCombinerConfig::BeginCreation(): Invalid root mixer name '%s'.\n", pszRootMixerName );
		return FALSE;
	}

	// Create our root mixer...
	pMixer = fnew CFAnimMixer;
	if( pMixer == NULL ) {
		DEVPRINTF( "CFAnimCombinerConfig::BeginCreation(): Could not allocate mixer memory.\n" );
		return FALSE;
	}
	fclib_strcpy( pMixer->m_szName, pszRootMixerName );
	pMixer->m_pDownstreamMixer = NULL;
	pMixer->m_nDownstreamMixerInputIndex = 0;
	pMixer->m_apUpstreamMixer[0] = NULL;
	pMixer->m_apUpstreamMixer[1] = NULL;
	pMixer->m_nType = nRootMixerType;

	// Add it to our list of mixers...
	flinklist_InitRoot( &m_RootMixer, (s32)FANG_OFFSETOF( CFAnimMixer, m_Link ) );
	flinklist_AddTail( &m_RootMixer, pMixer );

	FASSERT( CFAnimMixer::TYPE_MAX_TAP_COUNT >= 2 );

	if( CFAnimMixer::IsTap(nRootMixerType) ) {
		// Root is a tap...
		m_nMixerCount = 0;
		m_nTapCount = 1;
	} else {
		// Root is a mixer...
		m_nMixerCount = 1;
		m_nTapCount = 0;
	}

	return TRUE;
}

BOOL CFAnimCombinerConfig::EndCreation( void ) {
	if( m_bCreationComplete ) {
		// Creation is already complete...
		DEVPRINTF( "CFAnimCombinerConfig::EndCreation(): Creation has already ended.\n" );
		return TRUE;
	}

	if( m_RootMixer.nCount == 0 ) {
		// Creation hasn't even begun yet...
		DEVPRINTF( "CFAnimCombinerConfig::EndCreation(): BeginCreation() has not been called.\n" );
		return FALSE;
	}

	if( GetTapCount() == 0 ) {
		// No taps defined...
		DEVPRINTF( "CFAnimCombinerConfig::EndCreation(): Must define at least one tap.\n" );
		return FALSE;
	}

	if( !ConfirmInputsAreHookedUp( (CFAnimMixer *)flinklist_GetHead(&m_RootMixer) ) ) {
		// All inputs have not been hooked up...
		return FALSE;
	}

	m_bCreationComplete = TRUE;

	return TRUE;
}

void CFAnimCombinerConfig::Destroy( void ) {
	m_bCreationComplete = FALSE;
	m_RootMixer.nCount = 0;
	m_nMixerCount = 0;
	m_nTapCount = 0;
}

BOOL CFAnimCombinerConfig::AddMixer( cchar *pszMixerName, CFAnimMixer::Type_e nMixerType, cchar *pszDownstreamMixerName, u32 nDownstreamMixerInputIndex ) {
	CFAnimMixer *pMixerToAdd, *pDownstreamMixer;

	if( m_bCreationComplete ) {
		// Creation has already been completed...
		DEVPRINTF( "CFAnimCombinerConfig::AddMixer(): Creation has already ended.\n" );
		return FALSE;
	}

	if( m_RootMixer.nCount == 0 ) {
		// BeginCreation() hasn't been called...
		DEVPRINTF( "CFAnimCombinerConfig::AddMixer(): BeginCreation() has not been called.\n" );
		return FALSE;
	}

	if( pszMixerName==NULL || pszMixerName[0]==0 ) {
		DEVPRINTF( "CFAnimCombinerConfig::AddMixer(): Must specify a mixer name.\n" );
		return FALSE;
	}

	if( fclib_strlen(pszMixerName) > FANIM_MIXERNAME_LEN ) {
		// Invalid mixer name...
		DEVPRINTF( "CFAnimCombinerConfig::AddMixer(): Invalid mixer name '%s'.\n", pszMixerName );
		return FALSE;
	}

	if( FindName( pszMixerName, nMixerType ) ) {
		// Name already exists...
		DEVPRINTF( "CFAnimCombinerConfig::AddMixer(): Mixer/Tap name '%s' already exists.\n", pszMixerName );
		return FALSE;
	}

	if( pszDownstreamMixerName==NULL || pszDownstreamMixerName[0]==0 ) {
		DEVPRINTF( "CFAnimCombinerConfig::AddMixer(): Must specify a downstream mixer name.\n" );
		return FALSE;
	}
	
	if( fclib_strlen(pszDownstreamMixerName) > FANIM_MIXERNAME_LEN ) {
		// Invalid downstream mixer name...
		DEVPRINTF( "CFAnimCombinerConfig::AddMixer(): Invalid downstream mixer name '%s'.\n", pszDownstreamMixerName );
		return FALSE;
	}

	pDownstreamMixer = FindMixer( pszDownstreamMixerName );
	if( pDownstreamMixer == NULL ) {
		// Couldn't find the mixer we're supposed to drive...
		DEVPRINTF( "CFAnimCombinerConfig::AddMixer(): downstream mixer name '%s' not found.\n", pszDownstreamMixerName );
		return FALSE;
	}

	if( pDownstreamMixer->IsTap() ) {
		// Cannot drive a tap...
		DEVPRINTF( "CFAnimCombinerConfig::AddMixer(): Cannot specify a tap to drive.\n" );
		return FALSE;
	}

	if( nDownstreamMixerInputIndex >= 2 ) {
		// Invalid input...
		DEVPRINTF( "CFAnimCombinerConfig::AddMixer(): Invalid input for downstream mixer '%s' specified: %u.\n", pszDownstreamMixerName, nDownstreamMixerInputIndex );
		return FALSE;
	}

	if( pDownstreamMixer->m_apUpstreamMixer[nDownstreamMixerInputIndex] ) {
		// The input of the mixer we're driving is already being driven by someone else...
		DEVPRINTF( "CFAnimCombinerConfig::AddMixer(): Input %u of mixer '%s' is already being driven.\n", nDownstreamMixerInputIndex, pszDownstreamMixerName );
		return FALSE;
	}

	if( CFAnimMixer::IsTap(nMixerType) && m_nTapCount>=CFAnimMixer::TYPE_MAX_TAP_COUNT ) {
		// Too many taps...
		DEVPRINTF( "CFAnimCombinerConfig::AddMixer(): Too many taps (max is %u).\n", CFAnimMixer::TYPE_MAX_TAP_COUNT );
		return FALSE;
	}

	// Create our new mixer...
	pMixerToAdd = fnew CFAnimMixer;
	if( pMixerToAdd == NULL ) {
		DEVPRINTF( "CFAnimCombinerConfig::AddMixer(): Could not allocate mixer memory.\n" );
		return FALSE;
	}
	pMixerToAdd->m_nType = nMixerType;
	fclib_strcpy( pMixerToAdd->m_szName, pszMixerName );
	pMixerToAdd->m_pDownstreamMixer = pDownstreamMixer;
	pMixerToAdd->m_nDownstreamMixerInputIndex = nDownstreamMixerInputIndex;
	pMixerToAdd->m_apUpstreamMixer[0] = NULL;
	pMixerToAdd->m_apUpstreamMixer[1] = NULL;

	// Add it to our list of mixers...
	flinklist_AddTail( &m_RootMixer, pMixerToAdd );

	pDownstreamMixer->m_apUpstreamMixer[nDownstreamMixerInputIndex] = pMixerToAdd;

	if( CFAnimMixer::IsTap(nMixerType) ) {
		m_nTapCount++;
	} else {
		m_nMixerCount++;
	}

	return TRUE;
}

BOOL CFAnimCombinerConfig::AddNet( const ConfigNet_t *pConfigNet, BOOL bCreateRoot, cchar **apszMixerNameTable, cchar *pszDownstreamMixerName, u32 nDownstreamMixerInputIndex ) {
	if( pConfigNet->nMixerType == CFAnimMixer::TYPE_COUNT ) {
		return TRUE;
	}

	if( bCreateRoot ) {
		if( !BeginCreation( apszMixerNameTable[ pConfigNet->nMixerNameIndex ], (CFAnimMixer::Type_e)pConfigNet->nMixerType ) ) {
			return FALSE;
		}

		pConfigNet++;
	}

	if( pszDownstreamMixerName == NULL ) {
		// Snap onto mixer specified in net table...

		for( ; pConfigNet->nMixerType != CFAnimMixer::TYPE_COUNT; pConfigNet++ ) {
			if( !AddMixer( apszMixerNameTable[ pConfigNet->nMixerNameIndex ], (CFAnimMixer::Type_e)pConfigNet->nMixerType, apszMixerNameTable[ pConfigNet->nDownstreamMixerNameIndex ], (u32)pConfigNet->nDownstreamMixerInputIndex ) ) {
				return FALSE;
			}
		}
	} else {
		// Snap onto mixer provided in parameters...

		if( pConfigNet->nMixerType != CFAnimMixer::TYPE_COUNT ) {
			if( !AddMixer( apszMixerNameTable[ pConfigNet->nMixerNameIndex ], (CFAnimMixer::Type_e)pConfigNet->nMixerType, pszDownstreamMixerName, nDownstreamMixerInputIndex ) ) {
				return FALSE;
			}

			for( pConfigNet++; pConfigNet->nMixerType != CFAnimMixer::TYPE_COUNT; pConfigNet++ ) {
				if( !AddMixer( apszMixerNameTable[ pConfigNet->nMixerNameIndex ], (CFAnimMixer::Type_e)pConfigNet->nMixerType, apszMixerNameTable[ pConfigNet->nDownstreamMixerNameIndex ], (u32)pConfigNet->nDownstreamMixerInputIndex ) ) {
					return FALSE;
				}
			}
		}
	}

	return TRUE;
}


cchar *CFAnimCombinerConfig::AddStack( const ConfigStack_t *pConfigStack, BOOL bCreateRoot, cchar **apszMixerNameTable, cchar *pszDownstreamMixerName, u32 nDownstreamMixerInputIndex ) {
	if( pConfigStack->nMixerType == CFAnimMixer::TYPE_COUNT ) {
		return pszDownstreamMixerName;
	}

	if( bCreateRoot ) {
		if( !BeginCreation( apszMixerNameTable[ pConfigStack->nMixerNameIndex ], (CFAnimMixer::Type_e)pConfigStack->nMixerType ) ) {
			return NULL;
		}

		pConfigStack++;
	}

	for( ; pConfigStack->nMixerType != CFAnimMixer::TYPE_COUNT; pConfigStack++ ) {
		if( !AddMixer( apszMixerNameTable[ pConfigStack->nMixerNameIndex ], (CFAnimMixer::Type_e)pConfigStack->nMixerType, pszDownstreamMixerName, nDownstreamMixerInputIndex ) ) {
			return NULL;
		}

		pszDownstreamMixerName = apszMixerNameTable[ pConfigStack->nMixerNameIndex ];
		nDownstreamMixerInputIndex = 0;
	}

	return pszDownstreamMixerName;
}


BOOL CFAnimCombinerConfig::AddTaps( const ConfigTap_t *pConfigTap, cchar **apszTapNameTable, cchar **apszMixerNameTable ) {
	for( ; pConfigTap->nMixerInputIndex != 255; pConfigTap++ ) {
		if( !AddTap( apszTapNameTable[ pConfigTap->nTapNameIndex ], apszMixerNameTable[ pConfigTap->nMixerNameIndex ], pConfigTap->nMixerInputIndex ) ) {
			return FALSE;
		}
	}

	return TRUE;
}



CFAnimMixer *CFAnimCombinerConfig::FindName( cchar *pszName, BOOL bIsTap ) const {
	if( bIsTap ) {
		return FindTap( pszName );
	} else {
		return FindMixer( pszName );
	}
}

CFAnimMixer *CFAnimCombinerConfig::FindName( cchar *pszName, CFAnimMixer::Type_e nMixerType ) const {
	FASSERT( nMixerType>=0 && nMixerType<CFAnimMixer::TYPE_COUNT );

	if( CFAnimMixer::IsTap(nMixerType) ) {
		return FindTap( pszName );
	} else {
		return FindMixer( pszName );
	}
}

CFAnimMixer *CFAnimCombinerConfig::FindMixer( cchar *pszMixerName ) const {
	CFAnimMixer *pMixer;

	for( pMixer=(CFAnimMixer *)flinklist_GetHead(&m_RootMixer); pMixer; pMixer=(CFAnimMixer *)flinklist_GetNext(&m_RootMixer,pMixer) ) {
		if( !fclib_stricmp( pMixer->m_szName, pszMixerName ) ) {
			// Found mixer name...

			if( pMixer->IsMixer() ) {
				// This is a mixer (not a tap)...
				return pMixer;
			}
		}
	}

	return NULL;
}

CFAnimMixer *CFAnimCombinerConfig::FindTap( cchar *pszTapName ) const {
	CFAnimMixer *pMixer;

	for( pMixer=(CFAnimMixer *)flinklist_GetHead(&m_RootMixer); pMixer; pMixer=(CFAnimMixer *)flinklist_GetNext(&m_RootMixer,pMixer) ) {
		if( !fclib_stricmp( pMixer->m_szName, pszTapName ) ) {
			// Found mixer name...

			if( pMixer->IsTap() ) {
				// This is a mixer (not a tap)...
				return pMixer;
			}
		}
	}

	return NULL;
}

CFAnimMixer *CFAnimCombinerConfig::FindMixerHead( void ) const {
	CFAnimMixer *pMixer;

	for( pMixer=(CFAnimMixer *)flinklist_GetHead(&m_RootMixer); pMixer; pMixer=(CFAnimMixer *)flinklist_GetNext(&m_RootMixer,pMixer) ) {
		if( pMixer->IsMixer() ) {
			// This is the first mixer...
			return pMixer;
		}
	}

	return NULL;
}

CFAnimMixer *CFAnimCombinerConfig::FindMixerTail( void ) const {
	CFAnimMixer *pMixer;

	for( pMixer=(CFAnimMixer *)flinklist_GetTail(&m_RootMixer); pMixer; pMixer=(CFAnimMixer *)flinklist_GetPrev(&m_RootMixer,pMixer) ) {
		if( pMixer->IsMixer() ) {
			// This is the last mixer...
			return pMixer;
		}
	}

	return NULL;
}

CFAnimMixer *CFAnimCombinerConfig::FindMixerNext( CFAnimMixer *pMixer ) const {
	FASSERT( pMixer==NULL || pMixer->IsMixer() );

	while( (pMixer = (CFAnimMixer *)flinklist_GetNext( &m_RootMixer, pMixer )) ) {
		if( pMixer->IsMixer() ) {
			// This is the next mixer...
			return pMixer;
		}
	}

	return NULL;
}

CFAnimMixer *CFAnimCombinerConfig::FindMixerPrev( CFAnimMixer *pMixer ) const {
	FASSERT( pMixer==NULL || pMixer->IsMixer() );

	while( (pMixer = (CFAnimMixer *)flinklist_GetPrev( &m_RootMixer, pMixer )) ) {
		if( pMixer->IsMixer() ) {
			// This is the previous mixer...
			return pMixer;
		}
	}

	return NULL;
}

CFAnimMixer *CFAnimCombinerConfig::FindTapHead( void ) const {
	CFAnimMixer *pMixer;

	for( pMixer=(CFAnimMixer *)flinklist_GetHead(&m_RootMixer); pMixer; pMixer=(CFAnimMixer *)flinklist_GetNext(&m_RootMixer,pMixer) ) {
		if( pMixer->IsTap() ) {
			// This is the first tap...
			return pMixer;
		}
	}

	return NULL;
}

CFAnimMixer *CFAnimCombinerConfig::FindTapTail( void ) const {
	CFAnimMixer *pMixer;

	for( pMixer=(CFAnimMixer *)flinklist_GetTail(&m_RootMixer); pMixer; pMixer=(CFAnimMixer *)flinklist_GetPrev(&m_RootMixer,pMixer) ) {
		if( pMixer->IsTap() ) {
			// This is the last tap...
			return pMixer;
		}
	}

	return NULL;
}

CFAnimMixer *CFAnimCombinerConfig::FindTapNext( CFAnimMixer *pTap ) const {
	FASSERT( pTap==NULL || pTap->IsTap() );

	while( (pTap = (CFAnimMixer *)flinklist_GetNext( &m_RootMixer, pTap )) ) {
		if( pTap->IsTap() ) {
			// This is the next tap...
			return pTap;
		}
	}

	return NULL;
}

CFAnimMixer *CFAnimCombinerConfig::FindTapPrev( CFAnimMixer *pTap ) const {
	FASSERT( pTap==NULL || pTap->IsTap() );

	while( (pTap = (CFAnimMixer *)flinklist_GetPrev( &m_RootMixer, pTap )) ) {
		if( pTap->IsTap() ) {
			// This is the previous tap...
			return pTap;
		}
	}

	return NULL;
}

BOOL CFAnimCombinerConfig::ConfirmInputsAreHookedUp( const CFAnimMixer *pMixer ) const {
	CFAnimMixer *pUpstreamMixer;
	u32 i;

	if( pMixer->IsTap() ) {
		// This is a tap...
		return TRUE;
	}

	for( i=0; i<2; i++ ) {
		pUpstreamMixer = pMixer->GetUpstreamMixer( i );

		if( pUpstreamMixer == NULL ) {
			DEVPRINTF( "CFAnimCombinerConfig::ConfirmInputsAreHookedUp(): Mixer '%s' input %u is not connected.\n", pMixer->m_szName, i );
			return FALSE;
		}

		if( !ConfirmInputsAreHookedUp( pUpstreamMixer ) ) {
			return FALSE;
		}
	}

	// All inputs are hooked up...
	return TRUE;
}



//-------------------------------------------------------------------------------------------------------------------
// CFAnimCombiner:
//-------------------------------------------------------------------------------------------------------------------
u32 CFAnimCombiner::m_nBoneIndex;
u32 CFAnimCombiner::m_nNextCmdIndex;
u8 *CFAnimCombiner::m_pnExecBuf;


CFAnimCombiner::CFAnimCombiner() {
	m_pCombinerConfig = NULL;
	m_pMeshInst = NULL;
	m_nCombinerTapCount = 0;
	m_nCombinerMixCount = 0;
	m_nMaskBatchTapID = -1;
	m_bMaskBatchDriveBonesInList = FALSE;
	m_nBoneCount = 0;
	m_nBoneBitArrayElementCount = 0;
	m_nKeyInterpolationMode = CFAnimSource::LERP_KEYS;
	m_pRootCombinerNode = NULL;
	m_pCombinerTapArray = NULL;
	m_pCombinerMixArray = NULL;
	m_ppnBoneMixExecBuf = NULL;
}

BOOL CFAnimCombiner::Create( const CFAnimCombinerConfig *pCombinerConfig, CFMeshInst *pMesh ) {
	FResFrame_t ResFrame;
	CFAnimMixer *pMixer;
	CFAnimCombinerNode *pCombinerNode;
	u32 i, j, nByteCount, nExecBufBytes;
	u8 *pnExecBuf;

	ResFrame = fres_GetFrame();

	if( m_pCombinerConfig ) {
		// Already created...
		DEVPRINTF( "CFAnimCombiner::Create(): Object already created.\n" );
		goto _ExitCreateWithError;
	}

	if( !pCombinerConfig->IsCreationComplete() ) {
		// Creation of Combiner Config object hasn't been completed yet...
		DEVPRINTF( "CFAnimCombiner::Create(): Creation of CFAnimCombinerConfig object is incomplete.\n" );
		goto _ExitCreateWithError;
	}

	// Store object pointers...
	m_pCombinerConfig = pCombinerConfig;
	m_pMeshInst = pMesh;
	m_pFcnBoneCallback = NULL;
	m_nBoneCount = m_pMeshInst->m_pMesh->nBoneCount;
	m_nBoneBitArrayElementCount = (m_nBoneCount + 31) >> 5;
	m_nFlags = EXECUTE_BUFFER_REBUILD_NEEDED;

	for( i=0; i<FANIM_BONE_BITARRAY_ELEMENT_COUNT; i++ ) {
		m_anBoneCallbackMask[i] = 0;
	}

	// Allocate and initialize our combiner tap array...
	m_nCombinerTapCount = pCombinerConfig->GetTapCount();
	m_pCombinerTapArray = fnew CFAnimCombinerTap[m_nCombinerTapCount];
	if( m_pCombinerTapArray == NULL ) {
		DEVPRINTF( "CFAnimCombiner::Create(): Could not allocate combiner tap array memory.\n" );
		goto _ExitCreateWithError;
	}

	for( i=0, pMixer=pCombinerConfig->FindTapHead(); pMixer; i++, pMixer=pCombinerConfig->FindTapNext(pMixer) ) {
		FASSERT( i < m_nCombinerTapCount );
		FASSERT( pMixer->IsTap() );

		m_pCombinerTapArray[i].m_nCombinerArrayIndex = (u8)i;
		m_pCombinerTapArray[i].m_pMixer = pMixer;
		m_pCombinerTapArray[i].m_pDownstreamNode = NULL;
		m_pCombinerTapArray[i].m_apUpstreamNode[0] = NULL;
		m_pCombinerTapArray[i].m_apUpstreamNode[1] = NULL;
		m_pCombinerTapArray[i].m_nDownstreamNodeInputIndex = 0;
		m_pCombinerTapArray[i].m_pAnimSource = NULL;

		m_pCombinerTapArray[i].m_pBoneIndexMeshToAnimTable = (u8 *)fres_Alloc( m_nBoneCount * sizeof(u8) );
		if( m_pCombinerTapArray[i].m_pBoneIndexMeshToAnimTable == NULL ) {
			DEVPRINTF( "CFAnimCombiner::Create(): Could not allocate memory for bone index conversion table.\n" );
			goto _ExitCreateWithError;
		}

		for( j=0; j<m_nBoneCount; j++ ) {
			m_pCombinerTapArray[i].m_pBoneIndexMeshToAnimTable[j] = 0;
		}

		for( j=0; j<m_nBoneBitArrayElementCount; j++ ) {
			m_pCombinerTapArray[i].m_anBoneMask[j] = 0;
		}
	}

	// Allocate and initialize our combiner mix array...
	m_nCombinerMixCount = pCombinerConfig->GetMixerCount();
	m_pCombinerMixArray = fnew CFAnimCombinerMix[m_nCombinerMixCount];
	if( m_pCombinerMixArray == NULL ) {
		DEVPRINTF( "CFAnimCombiner::Create(): Could not allocate combiner mix array memory.\n" );
		goto _ExitCreateWithError;
	}

	for( i=0, pMixer=pCombinerConfig->FindMixerHead(); pMixer; i++, pMixer=pCombinerConfig->FindMixerNext(pMixer) ) {
		FASSERT( i < m_nCombinerMixCount );
		FASSERT( pMixer->IsMixer() );

		m_pCombinerMixArray[i].m_nCombinerArrayIndex = (u8)i;
		m_pCombinerMixArray[i].m_pMixer = pMixer;
		m_pCombinerMixArray[i].m_pDownstreamNode = NULL;
		m_pCombinerMixArray[i].m_apUpstreamNode[0] = NULL;
		m_pCombinerMixArray[i].m_apUpstreamNode[1] = NULL;
		m_pCombinerMixArray[i].m_nDownstreamNodeInputIndex = 0;
		m_pCombinerMixArray[i].m_nFlags = CFAnimCombinerMix::FLAG_NONE;

		if( pMixer->IsBlender() ) {
			// Blender...
			m_pCombinerMixArray[i].m_fValue = 0.0f;
		} else {
			// Summer...
			m_pCombinerMixArray[i].m_fValue = 1.0f;
		}
//		m_pCombinerMixArray[i].m_fPrevValue = m_pCombinerMixArray[i].m_fValue;

		for( j=0; j<m_nBoneBitArrayElementCount; j++ ) {
			m_pCombinerMixArray[i].m_anBoneMask[j] = 0;
		}
	}

	// Find our root combiner node...
	for( i=0; i<m_nCombinerTapCount; i++ ) {
		if( m_pCombinerTapArray[i].m_pMixer == m_pCombinerConfig->GetRoot() ) {
			m_pRootCombinerNode = &m_pCombinerTapArray[i];
			break;
		}
	}
	if( i == m_nCombinerTapCount ) {
		for( i=0; i<m_nCombinerMixCount; i++ ) {
			if( m_pCombinerMixArray[i].m_pMixer == m_pCombinerConfig->GetRoot() ) {
				m_pRootCombinerNode = &m_pCombinerMixArray[i];
				break;
			}
		}

		FASSERT( i < m_nCombinerMixCount );
	}

	// Wire up node interconnections...
	for( i=0; i<m_nCombinerMixCount; i++ ) {
		for( j=0; j<2; j++ ) {
			FASSERT( m_pCombinerMixArray[i].m_apUpstreamNode[j] == NULL );

			pCombinerNode = _FindCombinerNodeDrivingInput( &m_pCombinerMixArray[i], j );
			FASSERT( pCombinerNode );
			FASSERT( pCombinerNode->m_pDownstreamNode == NULL );

			m_pCombinerMixArray[i].m_apUpstreamNode[j] = pCombinerNode;
			pCombinerNode->m_pDownstreamNode = &m_pCombinerMixArray[i];
			pCombinerNode->m_nDownstreamNodeInputIndex = (u8)j;
		}
	}

	#if FANG_DEBUG_BUILD || FANG_TEST_BUILD
		// Verify interconnections...
		for( i=0; i<m_nCombinerTapCount; i++ ) {
			FASSERT( m_pCombinerTapArray[i].m_apUpstreamNode[0] == NULL );
			FASSERT( m_pCombinerTapArray[i].m_apUpstreamNode[1] == NULL );

			if( &m_pCombinerTapArray[i] == m_pRootCombinerNode ) {
				FASSERT( m_pCombinerTapArray[i].m_pDownstreamNode == NULL );
			} else {
				FASSERT( m_pCombinerTapArray[i].m_pDownstreamNode != NULL );
			}
		}
		for( i=0; i<m_nCombinerMixCount; i++ ) {
			FASSERT( m_pCombinerMixArray[i].m_apUpstreamNode[0] != NULL );
			FASSERT( m_pCombinerMixArray[i].m_apUpstreamNode[1] != NULL );

			if( &m_pCombinerMixArray[i] == m_pRootCombinerNode ) {
				FASSERT( m_pCombinerMixArray[i].m_pDownstreamNode == NULL );
			} else {
				FASSERT( m_pCombinerMixArray[i].m_pDownstreamNode != NULL );
			}
		}
	#endif

	// Allocate our bone mixer execute buffers.
	// Each entry in the execute buffer is one byte and either contains an index into m_pMixControlArray
	// or a reserved index value to indicate whether to blend or sum the values. We need a maximum of
	// 2*TapCount entries for each bone. In addition, we need a table of pointers to these execute buffers,
	// one entry for each bone. We'll allocate the memory in one block...

	nExecBufBytes = pCombinerConfig->GetTapCount() * 3 * sizeof(u8) - sizeof(u8);
	nByteCount = m_nBoneCount * nExecBufBytes;
	nByteCount += m_nBoneCount * sizeof(u8 *);

	m_ppnBoneMixExecBuf = (u8 **)fres_Alloc( nByteCount );
	if( m_ppnBoneMixExecBuf == NULL ) {
		DEVPRINTF( "CFAnimCombiner::Create(): Could not allocate bone mixer execute buffer memory.\n" );
		goto _ExitCreateWithError;
	}

	pnExecBuf = (u8 *)( (u32)m_ppnBoneMixExecBuf + m_nBoneCount*sizeof(u8 *) );

	for( i=0; i<m_nBoneCount; i++, pnExecBuf+=nExecBufBytes ) {
		m_ppnBoneMixExecBuf[i] = pnExecBuf;
		m_ppnBoneMixExecBuf[i][0] = EXECCMD_END;
	}

	// Success...

	return TRUE;

_ExitCreateWithError:
	fres_ReleaseFrame( ResFrame );
	return FALSE;
}

BOOL CFAnimCombiner::CreateSimple( CFAnimSource *pAnimSource, CFMeshInst *pMesh  ) {
	s32 nOnlyTapID;

	if( !Create( &_SimpleCombinerConfig, pMesh ) ) {
		// Trouble creating our combiner...
		return FALSE;
	}

	nOnlyTapID = GetTapID( FANIM_SIMPLE_COMBINER_CONFIG_TAPNAME );
	FASSERT( nOnlyTapID != -1 );

	AttachToTap( nOnlyTapID, pAnimSource );

	// Success...

	return TRUE;
}


BOOL CFAnimCombiner::CreateSummer( CFMeshInst *pMesh  ) {
//	s32 nOnlyTapID;

	if( !Create( &_SummerCombinerConfig, pMesh ) ) {
		// Trouble creating our combiner...
		return FALSE;
	}

//	nOnlyTapID = GetTapID( FANIM_SIMPLE_COMBINER_CONFIG_TAPNAME );
//	FASSERT( nOnlyTapID != -1 );

//	AttachToTap( nOnlyTapID, pAnimSource );

	// Success...

	return TRUE;
}



BOOL CFAnimCombiner::CreateBlender( CFMeshInst *pMesh  ) {
	if( !Create( &_BlenderCombinerConfig, pMesh ) ) {
		// Trouble creating our combiner...
		return FALSE;
	}

	return TRUE;
}


s32 CFAnimCombiner::GetTapID( cchar *pszTapName ) {
	u32 i;

	FASSERT( m_pCombinerConfig );

	for( i=0; i<m_nCombinerTapCount; i++ ) {
		if( m_pCombinerTapArray[i].m_pMixer->IsTap() ) {
			if( !fclib_stricmp( m_pCombinerTapArray[i].m_pMixer->GetName(), pszTapName ) ) {
				return (s32)i;
			}
		}
	}

	return -1;
}

s32 CFAnimCombiner::GetControlID( cchar *pszMixerName ) {
	u32 i;

	FASSERT( m_pCombinerConfig );

	for( i=0; i<m_nCombinerMixCount; i++ ) {
		if( m_pCombinerMixArray[i].m_pMixer->IsMixer() ) {
			if( !fclib_stricmp( m_pCombinerMixArray[i].m_pMixer->GetName(), pszMixerName ) ) {
				return (s32)i;
			}
		}
	}

	return -1;
}

void CFAnimCombiner::AttachToTap( s32 nTapID, CFAnimSource *pAnimSource, cchar **ppszUnmaskedBoneNames ) {
	CFAnimCombinerTap *pCombinerTap;
	cchar *pszMeshBoneName;
	u32 i, j, nAnimBoneCount;
	s32 nBoneIndex;

	FASSERT( m_pCombinerConfig );
	FASSERT( nTapID>=0 && nTapID<(s32)m_nCombinerTapCount );

	pCombinerTap = &m_pCombinerTapArray[nTapID];

	if( pCombinerTap->m_pAnimSource == pAnimSource ) {
		return;
	}

	pCombinerTap->m_pAnimSource = pAnimSource;

	// Reset bone mask...
	for( i=0; i<m_nBoneBitArrayElementCount; i++ ) {
		pCombinerTap->m_anBoneMask[i] = 0;
	}

	if( pAnimSource == NULL ) {
		// Unattach...
		_UpdateMixerBoneMasksFromTapToRoot( pCombinerTap );
		return;
	}

	nAnimBoneCount = pAnimSource->GetBoneCount();
	FASSERT( nAnimBoneCount <= FDATA_MAX_BONE_COUNT );

	// Populate our bone index translation table...
	for( i=0; i<m_nBoneCount; i++ ) {
		pszMeshBoneName = m_pMeshInst->m_pMesh->pBoneArray[i].szName;

		for( j=0; j<nAnimBoneCount; j++ ) {
			if( !fclib_stricmp( pszMeshBoneName, pAnimSource->GetBoneName(j) ) ) {
				// Found matching bone name...
				pCombinerTap->m_pBoneIndexMeshToAnimTable[i] = (u8)j;
				break;
			}
		}
		if( j == nAnimBoneCount ) {
			// Matching bone name not found...
			pCombinerTap->m_pBoneIndexMeshToAnimTable[i] = 255;
		}
	}

	// Init bone mask...
	if( ppszUnmaskedBoneNames ) {
		// Bone mask provided...

		for( i=0; ppszUnmaskedBoneNames[i]; i++ ) {
			if( pAnimSource->FindBoneIndex( ppszUnmaskedBoneNames[i] ) != -1 ) {
				// The specified name is in the anim source...

				nBoneIndex = m_pMeshInst->FindBone( ppszUnmaskedBoneNames[i] );
				if( nBoneIndex != -1 ) {
					_SetBoneBitMask( pCombinerTap, (u32)nBoneIndex );
				}
			}
		}
	} else {
		// Bone mask not provided.
		// Unmask all anim bones that are also in the mesh...

		for( i=0; i<nAnimBoneCount; i++ ) {
			nBoneIndex = m_pMeshInst->FindBone( pAnimSource->GetBoneName(i) );
			if( nBoneIndex != -1 ) {
				_SetBoneBitMask( pCombinerTap, (u32)nBoneIndex );
			}
		}
	}

	_UpdateMixerBoneMasksFromTapToRoot( pCombinerTap );
}

void CFAnimCombiner::AttachToTaps( const AttachList_t *pAttachList, CFAnimSource *pAnimSourceTable, const u8 *pnAnimTapTable ) {
	FASSERT( m_pCombinerConfig );

	for( ; pAttachList->nTapIDIndex != 255; pAttachList++ ) {
		AttachToTap( pnAnimTapTable[ pAttachList->nTapIDIndex ], &pAnimSourceTable[ pAttachList->nAnimSourceIndex ] );
	}
}

void CFAnimCombiner::Mask_EnableAllBones( s32 nTapID ) {
	FASSERT( m_pCombinerConfig );
	FASSERT( nTapID>=0 && nTapID<(s32)m_nCombinerTapCount );

	_Mask_EnableAllBones( &m_pCombinerTapArray[nTapID] );
	_UpdateMixerBoneMasksFromTapToRoot( &m_pCombinerTapArray[nTapID] );
}

void CFAnimCombiner::Mask_DisableAllBones( s32 nTapID ) {
	FASSERT( m_pCombinerConfig );
	FASSERT( nTapID>=0 && nTapID<(s32)m_nCombinerTapCount );

	_Mask_DisableAllBones( &m_pCombinerTapArray[nTapID] );
	_UpdateMixerBoneMasksFromTapToRoot( &m_pCombinerTapArray[nTapID] );
}

void CFAnimCombiner::Mask_BatchUpdateTapBoneMask_Open( s32 nTapID, BOOL bDriveBonesInList ) {
	FASSERT( m_pCombinerConfig );
	FASSERT( nTapID>=0 && nTapID<(s32)m_nCombinerTapCount );

#if FANG_DEBUG_BUILD || FANG_TEST_BUILD
	if( m_nMaskBatchTapID >= 0 ) {
		DEVPRINTF( "Mask_BatchUpdateTapBoneMask_Open(): WARNING: Mask_BatchUpdateTapBoneMask_Close() has not been called!\n" );
		m_nMaskBatchTapID = -1;
	}
#endif

	m_nMaskBatchTapID = nTapID;
	m_bMaskBatchDriveBonesInList = bDriveBonesInList;

	CFAnimCombinerTap *pCombinerTap = &m_pCombinerTapArray[m_nMaskBatchTapID];

	if( bDriveBonesInList ) {
		_Mask_DisableAllBones( pCombinerTap );
	} else {
		_Mask_EnableAllBones( pCombinerTap );
	}
}

void CFAnimCombiner::Mask_BatchUpdateTapBoneMask_Close( void ) {
	FASSERT( m_pCombinerConfig );

	if( m_nMaskBatchTapID >= 0 ) {
		CFAnimCombinerTap *pCombinerTap = &m_pCombinerTapArray[m_nMaskBatchTapID];

		_UpdateMixerBoneMasksFromTapToRoot( pCombinerTap );

		m_nMaskBatchTapID = -1;
	}
}

void CFAnimCombiner::Mask_BatchUpdateTapBoneMask( const u8 *pnBoneNameIndexTable, cchar **ppszBoneNameTable ) {
	CFAnimCombinerTap *pCombinerTap;
	const CFAnimSource *pAnimSource;
	cchar *pszBoneName;
	s32 nBoneIndex;

	FASSERT( m_pCombinerConfig );
	FASSERT_MSG( m_nMaskBatchTapID >= 0, "Mask_BatchUpdateTapBoneMask_Open() has not been called." );

	pCombinerTap = &m_pCombinerTapArray[m_nMaskBatchTapID];

	pAnimSource = pCombinerTap->m_pAnimSource;

	if( pAnimSource == NULL ) {
		// No source...
		return;
	}

	if( m_bMaskBatchDriveBonesInList ) {
		// Flag to drive the specified bones...

		if( pnBoneNameIndexTable ) {
			// Bone mask provided...

			for( ; *pnBoneNameIndexTable != 255; pnBoneNameIndexTable++ ) {
				pszBoneName = ppszBoneNameTable[ *pnBoneNameIndexTable ];

				if( pAnimSource->FindBoneIndex( pszBoneName ) != -1 ) {
					// The specified name is in the anim source...

					nBoneIndex = m_pMeshInst->FindBone( pszBoneName );
					if( nBoneIndex != -1 ) {
						_SetBoneBitMask( pCombinerTap, (u32)nBoneIndex );
					}
				}
			}
		}
	} else {
		// Flag to drive all but the specified bones...

		if( pnBoneNameIndexTable ) {
			// Bone mask provided...

			for( ; *pnBoneNameIndexTable != 255; pnBoneNameIndexTable++ ) {
				pszBoneName = ppszBoneNameTable[ *pnBoneNameIndexTable ];

				if( pAnimSource->FindBoneIndex( pszBoneName ) != -1 ) {
					// The specified name is in the anim source...

					nBoneIndex = m_pMeshInst->FindBone( pszBoneName );
					if( nBoneIndex != -1 ) {
						_ClearBoneBitMask( pCombinerTap, (u32)nBoneIndex );
					}
				}
			}
		}
	}
}

void CFAnimCombiner::_Mask_EnableAllBones( CFAnimCombinerTap *pCombinerTap ) {
	const CFAnimSource *pAnimSource;
	u32 i, nAnimBoneCount;
	s32 nBoneIndex;

	pAnimSource = pCombinerTap->m_pAnimSource;
	nAnimBoneCount = pAnimSource->GetBoneCount();

	for( i=0; i<nAnimBoneCount; i++ ) {
		nBoneIndex = m_pMeshInst->FindBone( pAnimSource->GetBoneName(i) );
		if( nBoneIndex != -1 ) {
			_SetBoneBitMask( pCombinerTap, (u32)nBoneIndex );
		}
	}
}

void CFAnimCombiner::_Mask_DisableAllBones( CFAnimCombinerTap *pCombinerTap ) {
	u32 i;

	for( i=0; i<m_nBoneBitArrayElementCount; i++ ) {
		pCombinerTap->m_anBoneMask[i] = 0;
	}
}

// Passing NULL for ppszBoneNameTable means "apply to all bones".
void CFAnimCombiner::Mask_UpdateTapBoneMask( s32 nTapID, cchar **ppszBoneNameTable, BOOL bDriveBonesInList ) {
	CFAnimCombinerTap *pCombinerTap;
	const CFAnimSource *pAnimSource;
	u32 i;
	s32 nBoneIndex;

	FASSERT( m_pCombinerConfig );
	FASSERT( nTapID>=0 && nTapID<(s32)m_nCombinerTapCount );

	pCombinerTap = &m_pCombinerTapArray[nTapID];

	pAnimSource = pCombinerTap->m_pAnimSource;

	if( pAnimSource == NULL ) {
		// No source...
		return;
	}

	if( bDriveBonesInList ) {
		// Flag to drive the specified bones...

		if( ppszBoneNameTable ) {
			// Bone mask provided...

			_Mask_DisableAllBones( pCombinerTap );

			for( i=0; ppszBoneNameTable[i]; i++ ) {
				if( pAnimSource->FindBoneIndex( ppszBoneNameTable[i] ) != -1 ) {
					// The specified name is in the anim source...

					nBoneIndex = m_pMeshInst->FindBone( ppszBoneNameTable[i] );
					if( nBoneIndex != -1 ) {
						_SetBoneBitMask( pCombinerTap, (u32)nBoneIndex );
					}
				}
			}
		} else {
			// No bone mask provided. Drive all bones...
			_Mask_EnableAllBones( pCombinerTap );
		}
	} else {
		// Flag to drive all but the specified bones...

		if( ppszBoneNameTable ) {
			// Bone mask provided...

			_Mask_EnableAllBones( pCombinerTap );

			for( i=0; ppszBoneNameTable[i]; i++ ) {
				if( pAnimSource->FindBoneIndex( ppszBoneNameTable[i] ) != -1 ) {
					// The specified name is in the anim source...

					nBoneIndex = m_pMeshInst->FindBone( ppszBoneNameTable[i] );
					if( nBoneIndex != -1 ) {
						_ClearBoneBitMask( pCombinerTap, (u32)nBoneIndex );
					}
				}
			}
		} else {
			// No bone mask provided. Disable all bones...
			_Mask_DisableAllBones( pCombinerTap );
		}
	}

	_UpdateMixerBoneMasksFromTapToRoot( pCombinerTap );
}

void CFAnimCombiner::Mask_UpdateTapBoneMask( s32 nTapID, const u8 *pnBoneNameIndexTable, cchar **ppszBoneNameTable, BOOL bDriveBonesInList ) {
	CFAnimCombinerTap *pCombinerTap;
	const CFAnimSource *pAnimSource;
	cchar *pszBoneName;
	s32 nBoneIndex;

	FASSERT( m_pCombinerConfig );
	FASSERT( nTapID>=0 && nTapID<(s32)m_nCombinerTapCount );

	pCombinerTap = &m_pCombinerTapArray[nTapID];

	pAnimSource = pCombinerTap->m_pAnimSource;

	if( pAnimSource == NULL ) {
		// No source...
		return;
	}

	if( bDriveBonesInList ) {
		// Flag to drive the specified bones...

		if( pnBoneNameIndexTable ) {
			// Bone mask provided...

			_Mask_DisableAllBones( pCombinerTap );

			for( ; *pnBoneNameIndexTable != 255; pnBoneNameIndexTable++ ) {
				pszBoneName = ppszBoneNameTable[ *pnBoneNameIndexTable ];

				if( pAnimSource->FindBoneIndex( pszBoneName ) != -1 ) {
					// The specified name is in the anim source...

					nBoneIndex = m_pMeshInst->FindBone( pszBoneName );
					if( nBoneIndex != -1 ) {
						_SetBoneBitMask( pCombinerTap, (u32)nBoneIndex );
					}
				}
			}
		} else {
			// No bone mask provided. Drive all bones...
			_Mask_EnableAllBones( pCombinerTap );
		}
	} else {
		// Flag to drive all but the specified bones...

		if( pnBoneNameIndexTable ) {
			// Bone mask provided...

			_Mask_EnableAllBones( pCombinerTap );

			for( ; *pnBoneNameIndexTable != 255; pnBoneNameIndexTable++ ) {
				pszBoneName = ppszBoneNameTable[ *pnBoneNameIndexTable ];

				if( pAnimSource->FindBoneIndex( pszBoneName ) != -1 ) {
					// The specified name is in the anim source...

					nBoneIndex = m_pMeshInst->FindBone( pszBoneName );
					if( nBoneIndex != -1 ) {
						_ClearBoneBitMask( pCombinerTap, (u32)nBoneIndex );
					}
				}
			}
		} else {
			// No bone mask provided. Disable all bones...
			_Mask_DisableAllBones( pCombinerTap );
		}
	}

	_UpdateMixerBoneMasksFromTapToRoot( pCombinerTap );
}

void CFAnimCombiner::Mask_UpdateTapBoneMask( s32 nTapID, cchar *pszBoneName, BOOL bDriveBone ) {
	CFAnimCombinerTap *pCombinerTap;
	const CFAnimSource *pAnimSource;
	s32 nBoneIndex;

	FASSERT( m_pCombinerConfig );
	FASSERT( nTapID>=0 && nTapID<(s32)m_nCombinerTapCount );

	pCombinerTap = &m_pCombinerTapArray[nTapID];

	pAnimSource = pCombinerTap->m_pAnimSource;

	if( pAnimSource == NULL ) {
		// No source...
		return;
	}

	if( pszBoneName == NULL ) {
		// No bone name provided...
		return;
	}

	// Init bone mask...
	if( pAnimSource->FindBoneIndex( pszBoneName ) != -1 ) {
		// The specified name is in the anim source...

		nBoneIndex = m_pMeshInst->FindBone( pszBoneName );
		if( nBoneIndex != -1 ) {
			if( _GetBoneBitMask( pCombinerTap, (u32)nBoneIndex ) != !!bDriveBone ) {
				_UpdateBoneBitMask( pCombinerTap, nBoneIndex, bDriveBone );
				_UpdateMixerBoneMasksFromTapToRoot( pCombinerTap );
			}
		}
	}
}

BOOL CFAnimCombiner::Mask_IsDrivingBone( s32 nTapID, cchar *pszBoneName ) {
	CFAnimCombinerTap *pCombinerTap;
	const CFAnimSource *pAnimSource;
	s32 nBoneIndex;

	FASSERT( m_pCombinerConfig );
	FASSERT( nTapID>=0 && nTapID<(s32)m_nCombinerTapCount );

	pCombinerTap = &m_pCombinerTapArray[nTapID];

	pAnimSource = pCombinerTap->m_pAnimSource;

	if( pAnimSource == NULL ) {
		// No source...
		return FALSE;
	}

	if( pszBoneName == NULL ) {
		// No bone name provided...
		return FALSE;
	}

	// Get bone mask...
	if( pAnimSource->FindBoneIndex( pszBoneName ) != -1 ) {
		// The specified name is in the anim source...

		nBoneIndex = m_pMeshInst->FindBone( pszBoneName );
		if( nBoneIndex != -1 ) {
			return _GetBoneBitMask( pCombinerTap, (u32)nBoneIndex );
		}
	}

	return FALSE;
}


void CFAnimCombiner::EnableBoneCallback( cchar *pszBoneName ) {
	s32 nBoneIndex;

	FASSERT( m_pCombinerConfig );

	nBoneIndex = m_pMeshInst->FindBone( pszBoneName );
	if( nBoneIndex >= 0 ) {
		m_anBoneCallbackMask[ nBoneIndex >> 5 ] |= (1 << (nBoneIndex & 31));
	}
}


void CFAnimCombiner::DisableBoneCallback( cchar *pszBoneName ) {
	s32 nBoneIndex;

	FASSERT( m_pCombinerConfig );

	nBoneIndex = m_pMeshInst->FindBone( pszBoneName );
	if( nBoneIndex >= 0 ) {
		m_anBoneCallbackMask[ nBoneIndex >> 5 ] &= ~(1 << (nBoneIndex & 31));
	}
}


BOOL CFAnimCombiner::IsBoneCallbackEnabled( cchar *pszBoneName ) {
	s32 nBoneIndex;

	FASSERT( m_pCombinerConfig );

	nBoneIndex = m_pMeshInst->FindBone( pszBoneName );
	if( nBoneIndex >= 0 ) {
		return (BOOL)( (m_anBoneCallbackMask[ nBoneIndex >> 5 ] >> (nBoneIndex & 31)) & 1 );
	}

	return FALSE;
}


void CFAnimCombiner::EnableBoneCallback( u32 nBoneIndex ) {
	FASSERT( m_pCombinerConfig );

	m_anBoneCallbackMask[ nBoneIndex >> 5 ] |= (1 << (nBoneIndex & 31));
}


void CFAnimCombiner::DisableBoneCallback( u32 nBoneIndex ) {
	m_anBoneCallbackMask[ nBoneIndex >> 5 ] &= ~(1 << (nBoneIndex & 31));
}


BOOL CFAnimCombiner::IsBoneCallbackEnabled( u32 nBoneIndex ) {
	FASSERT( m_pCombinerConfig );

	return (BOOL)( (m_anBoneCallbackMask[ nBoneIndex >> 5 ] >> (nBoneIndex & 31)) & 1 );
}


void CFAnimCombiner::DisableAllBoneCallbacks( void ) {
	u32 i;

	FASSERT( m_pCombinerConfig );

	for( i=0; i<FANIM_BONE_BITARRAY_ELEMENT_COUNT; i++ ) {
		m_anBoneCallbackMask[i] = 0;
	}
}


void CFAnimCombiner::ComputeMtxPalette( BOOL bAllowOffscreenOptimizations/*=FALSE*/, u32 nTranslatorBoneIndexToSkip/*0xffffffff*/, CFMtx43A *pDestTranslatorBoneIndex/*=NULL*/ ) {
	FASSERT( m_pCombinerConfig );

	if( m_pMeshInst->m_pMesh->nBoneCount == 0 ) {
		// This mesh has no bones...
		return;
	}

	if( m_pMeshInst->GetBoneMtxPalette() == NULL ) {
		// This mesh has no bone matrix palette...
		return;
	}

#if FANG_DEBUG_BUILD || FANG_TEST_BUILD
	if( m_nMaskBatchTapID >= 0 ) {
		DEVPRINTF( "ComputeMtxPalette(): WARNING: Mask_BatchUpdateTapBoneMask_Close() has not been called!\n" );
		m_nMaskBatchTapID = -1;
	}
#endif

	FMATH_CLEARBITMASK( m_pMeshInst->m_nFlags, FMESHINST_FLAG_NOBONES );

#if !FANG_PRODUCTION_BUILD
#if SAS_ACTIVE_USER == SAS_USER_ELLIOTT
	if( uLastComputeMtxPaletteTime == FLoop_nTotalLoopTicks ) {
//		DEVPRINTF( "ComputeMtxPalette(): WARNING: ComputeMtxPalette has already been called this frame for this combiner (mesh name:  %s)\n", m_pMeshInst->m_pMesh->szName );
	}
	uLastComputeMtxPaletteTime = FLoop_nTotalLoopTicks; 
#endif
#endif

	if ( !(m_pMeshInst->m_nFlags & FMESHINST_FLAG_NOANIMLOD) && bAllowOffscreenOptimizations ) {
		// This mesh will accept optimizations, if possible
		if( !m_pMeshInst->WasDrawnThisFrame() ) {
			// Mesh was not drawn so we can just render the mesh with its at rest matrices
			_ComputeDefaultMtxPalette( m_pMeshInst->m_pMesh->nRootBoneIndex, m_pMeshInst->m_Xfm.m_MtxF, nTranslatorBoneIndexToSkip, pDestTranslatorBoneIndex );
		} else {
			// Mesh was rendered, but maybe we can turn off interpolation if it is far enough away
			if( (m_pMeshInst->GetPixelRadiusLastDrawn() > 15) && FVid_nFrameCounter - m_pMeshInst->GetFrameLastDrawn() < 10 ) {
				m_nKeyInterpolationMode = CFAnimSource::LERP_KEYS;
			} else {
				m_nKeyInterpolationMode = CFAnimSource::USE_LOW_KEY;
			}
			_ComputeMtxPalette( m_pMeshInst->m_pMesh->nRootBoneIndex, m_pMeshInst->m_Xfm.m_MtxF, nTranslatorBoneIndexToSkip, pDestTranslatorBoneIndex );
			m_nFlags &= ~EXECUTE_BUFFER_REBUILD_NEEDED;
		}
	} else {
		// This mesh does not accept optimizations
		m_nKeyInterpolationMode = CFAnimSource::LERP_KEYS;
		_ComputeMtxPalette( m_pMeshInst->m_pMesh->nRootBoneIndex, m_pMeshInst->m_Xfm.m_MtxF, nTranslatorBoneIndexToSkip, pDestTranslatorBoneIndex );
		m_nFlags &= ~EXECUTE_BUFFER_REBUILD_NEEDED;
	}

#if FANG_WINGC
	{
		// Port diagnostic: does the palette end up in world space for the player mesh?
		static u32 _nLogged = 0;
		if ( Fang_bPortDiag && _nLogged < 4 && fclib_stricmp( m_pMeshInst->m_pMesh->szName, "grdggltch00" ) == 0 )
		{
			_nLogged++;
			FMesh_t *pMesh = m_pMeshInst->m_pMesh;
			const u32 nRoot = pMesh->nRootBoneIndex;
			const CFMtx43A *pRootMtx = m_pMeshInst->GetBoneMtxPalette()[nRoot];
			const CFMtx43A *pBone2 = pMesh->nBoneCount > 2 ? m_pMeshInst->GetBoneMtxPalette()[2] : NULL;
			DEVPRINTF( "PORT-ANIM root=%u bones=%u rootChildren=%u rootChildStart=%u xfm=(%.1f,%.1f,%.1f) rootPal=(%.1f,%.1f,%.1f) bone2Pal=(%.1f,%.1f,%.1f) boneCount(comb)=%u\n",
				nRoot, (u32)pMesh->nBoneCount,
				nRoot < pMesh->nBoneCount ? (u32)pMesh->pBoneArray[nRoot].Skeleton.nChildBoneCount : 0xffff,
				nRoot < pMesh->nBoneCount ? (u32)pMesh->pBoneArray[nRoot].Skeleton.nChildArrayStartIndex : 0xffff,
				m_pMeshInst->m_Xfm.m_MtxF.m_vPos.x, m_pMeshInst->m_Xfm.m_MtxF.m_vPos.y, m_pMeshInst->m_Xfm.m_MtxF.m_vPos.z,
				pRootMtx ? pRootMtx->m_vPos.x : 0.0f, pRootMtx ? pRootMtx->m_vPos.y : 0.0f, pRootMtx ? pRootMtx->m_vPos.z : 0.0f,
				pBone2 ? pBone2->m_vPos.x : 0.0f, pBone2 ? pBone2->m_vPos.y : 0.0f, pBone2 ? pBone2->m_vPos.z : 0.0f,
				(u32)m_nBoneCount );
		}
	}
#endif
}

void CFAnimCombiner::_ComputeDefaultMtxPalette( u32 nBoneIndex, const CFMtx43A &rParentMtx, u32 nTranslatorBoneIndexToSkip/*=0xffffffff*/, CFMtx43A *pDestTranslatorBoneIndex/*=NULL*/ ) {
	FMeshBone_t *pBone;
	CFMtx43A FrameMtx, CallbackMtx, NextParentMtx;
	CFMtx43A *pFrameMtx, *pMeshMtxPalette, *pNextParentMtx;
	u8 *pnChildBoneIndex, *pnChildBoneLastIndex;

	FASSERT( m_pCombinerConfig );
	FASSERT( nBoneIndex < m_nBoneCount );
	FASSERT( nBoneIndex < m_pMeshInst->m_pMesh->nBoneCount );

	#if FPERF_ENABLE
		FPerf_nTotalAnimSysBones++;
		FPerf_nTotalBonesStatic++;
	#endif

	pMeshMtxPalette = m_pMeshInst->GetBoneMtxPalette()[ nBoneIndex ];

	if( pMeshMtxPalette == NULL ) {
		// This bone matrix palette entry points nowhere...
		return;
	}

	pBone = m_pMeshInst->m_pMesh->pBoneArray + nBoneIndex;

	pNextParentMtx = pMeshMtxPalette;
	if( pBone->nFlags & FMESH_BONEFLAG_SKINNEDBONE ) {
		// Skinned bone...
		pNextParentMtx = &NextParentMtx;
	}

	if( nBoneIndex != nTranslatorBoneIndexToSkip ) {
		// We are not going to drive the bone. Use at-rest bone matrix...
		pFrameMtx = &pBone->AtRestBoneToParentMtx;

		if( m_pFcnBoneCallback==NULL || ((m_anBoneCallbackMask[ nBoneIndex >> 5 ] >> (nBoneIndex & 31)) & 1)==0 ) {
			// No bone callback is provided...
			pNextParentMtx->Mul( rParentMtx, *pFrameMtx );
		} else {
			// This bone is flagged for callback...
			m_pFcnBoneCallback( nBoneIndex, *pNextParentMtx, rParentMtx, *pFrameMtx );
		}
	} else {
		// This is the specified skip bone.
		// The matrix palette will recieve the at-rest bone orientation, and the bone hierarchy below will use that.
		// If pDestTranslatorBoneIndex is not NULL, it will be filled with the matrix computed from the animation frame.

		pFrameMtx = &pBone->AtRestBoneToParentMtx;
		pNextParentMtx->Mul( rParentMtx, *pFrameMtx );

		if( pDestTranslatorBoneIndex ) {
			// We are not going to drive the bone. Use at-rest bone matrix...
			*pDestTranslatorBoneIndex = pBone->AtRestBoneToParentMtx;
		}
	}

	if( pBone->nFlags & FMESH_BONEFLAG_SKINNEDBONE ) {
		// Skinned bone...
		pMeshMtxPalette->Mul( NextParentMtx, pBone->AtRestModelToBoneMtx );
	}

	pnChildBoneIndex = &m_pMeshInst->m_pMesh->pnSkeletonIndexArray[ pBone->Skeleton.nChildArrayStartIndex ];
	pnChildBoneLastIndex = pnChildBoneIndex + pBone->Skeleton.nChildBoneCount;

	for( ; pnChildBoneIndex<pnChildBoneLastIndex; pnChildBoneIndex++ ) {
		_ComputeDefaultMtxPalette( *pnChildBoneIndex, *pNextParentMtx, nTranslatorBoneIndexToSkip, pDestTranslatorBoneIndex );
	}
}

void CFAnimCombiner::_ComputeMtxPalette( u32 nBoneIndex, const CFMtx43A &rParentMtx, u32 nTranslatorBoneIndexToSkip/*=0xffffffff*/, CFMtx43A *pDestTranslatorBoneIndex/*=NULL*/ ) {
	FMeshBone_t *pBone;
	CFMtx43A FrameMtx, CallbackMtx, NextParentMtx;
	CFMtx43A *pFrameMtx, *pMeshMtxPalette, *pNextParentMtx;
	u8 *pnChildBoneIndex, *pnChildBoneLastIndex;

	FASSERT( m_pCombinerConfig );
	FASSERT( nBoneIndex < m_nBoneCount );
	FASSERT( nBoneIndex < m_pMeshInst->m_pMesh->nBoneCount );

	#if FPERF_ENABLE
		FPerf_nTotalAnimSysBones++;
	#endif

	pMeshMtxPalette = m_pMeshInst->GetBoneMtxPalette()[ nBoneIndex ];

	if( pMeshMtxPalette == NULL ) {
		// This bone matrix palette entry points nowhere...
		return;
	}

	if ( m_nFlags & EXECUTE_BUFFER_REBUILD_NEEDED ) {
		#if FPERF_ENABLE
			FPerf_nTotalExecBuffersBuilt++;
		#endif
		_BuildBoneExecBuffer( nBoneIndex );
	}

	pBone = m_pMeshInst->m_pMesh->pBoneArray + nBoneIndex;

	pNextParentMtx = pMeshMtxPalette;
	if( pBone->nFlags & FMESH_BONEFLAG_SKINNEDBONE ) {
		// Skinned bone...
		pNextParentMtx = &NextParentMtx;
	}

	if( nBoneIndex != nTranslatorBoneIndexToSkip ) {
		if( _GetBoneBitMask( m_pRootCombinerNode, nBoneIndex ) ) {
			// Bone is being driven...
			_ComputeFrame( FrameMtx, nBoneIndex );
			pFrameMtx = &FrameMtx;
		} else {
			// This bone isn't being driven by anything. Use at-rest bone matrix...
			pFrameMtx = &pBone->AtRestBoneToParentMtx;
		}

		if( m_pFcnBoneCallback==NULL || ((m_anBoneCallbackMask[ nBoneIndex >> 5 ] >> (nBoneIndex & 31)) & 1)==0 ) {
			// No bone callback is provided...
			pNextParentMtx->Mul( rParentMtx, *pFrameMtx );
		} else {
			// This bone is flagged for callback...
			m_pFcnBoneCallback( nBoneIndex, *pNextParentMtx, rParentMtx, *pFrameMtx );
		}
	} else {
		// This is the specified skip bone.
		// The matrix palette will recieve the at-rest bone orientation, and the bone hierarchy below will use that.
		// If pDestTranslatorBoneIndex is not NULL, it will be filled with the matrix computed from the animation frame.

		pFrameMtx = &pBone->AtRestBoneToParentMtx;
		pNextParentMtx->Mul( rParentMtx, *pFrameMtx );

		if( pDestTranslatorBoneIndex ) {
			if( _GetBoneBitMask( m_pRootCombinerNode, nBoneIndex ) ) {
				// Bone is being driven...
				_ComputeFrame( *pDestTranslatorBoneIndex, nBoneIndex );
			} else {
				// This bone isn't being driven by anything. Use at-rest bone matrix...
				*pDestTranslatorBoneIndex = pBone->AtRestBoneToParentMtx;
			}
		}
	}

	if( pBone->nFlags & FMESH_BONEFLAG_SKINNEDBONE ) {
		// Skinned bone...
		pMeshMtxPalette->Mul( NextParentMtx, pBone->AtRestModelToBoneMtx );
	}

	pnChildBoneIndex = &m_pMeshInst->m_pMesh->pnSkeletonIndexArray[ pBone->Skeleton.nChildArrayStartIndex ];
	pnChildBoneLastIndex = pnChildBoneIndex + pBone->Skeleton.nChildBoneCount;

	for( ; pnChildBoneIndex<pnChildBoneLastIndex; pnChildBoneIndex++ ) {
		_ComputeMtxPalette( *pnChildBoneIndex, *pNextParentMtx, nTranslatorBoneIndexToSkip, pDestTranslatorBoneIndex );
	}
}

void CFAnimCombiner::_ComputeFrame( CFMtx43A &rDestFrameMtx43A, u32 nBoneIndex ) {
	CFAnimCombinerMix *pCombinerMix;
	u8 *pnExecPointer;
	u32 nAnimFrameSrcStackNextFree, nAnimBoneIndex;
	CFAnimFrame AnimFrame, AnimFrame2;

	FASSERT( m_pCombinerConfig );
	FASSERT( nBoneIndex < m_nBoneCount );

	nAnimFrameSrcStackNextFree = 0;

	for( pnExecPointer=m_ppnBoneMixExecBuf[nBoneIndex]; *pnExecPointer!=EXECCMD_END; pnExecPointer++ ) {
		switch( *pnExecPointer ) {
		case EXECCMD_BLEND_LINEAR:
			// Blend the last two things on the stack...

			pnExecPointer++;
			pCombinerMix = &m_pCombinerMixArray[ *pnExecPointer ];

			AnimFrame.ReceiveSlerpOf(
				pCombinerMix->m_fValue,
				_pAnimFrameResultStack[nAnimFrameSrcStackNextFree-2],
				_pAnimFrameResultStack[nAnimFrameSrcStackNextFree-1]
			);

			_pAnimFrameResultStack[nAnimFrameSrcStackNextFree-2] = AnimFrame;
			nAnimFrameSrcStackNextFree--;

			break;

		case EXECCMD_BLEND_SMOOTH:
			// Blend the last two things on the stack...

			pnExecPointer++;
			pCombinerMix = &m_pCombinerMixArray[ *pnExecPointer ];

			AnimFrame.ReceiveSlerpOf(
				fmath_UnitLinearToSCurve( pCombinerMix->m_fValue ),
				_pAnimFrameResultStack[nAnimFrameSrcStackNextFree-2],
				_pAnimFrameResultStack[nAnimFrameSrcStackNextFree-1]
			);

			_pAnimFrameResultStack[nAnimFrameSrcStackNextFree-2] = AnimFrame;
			nAnimFrameSrcStackNextFree--;

			break;

		case EXECCMD_SUM:
			// Sum the last two things on the stack...

			AnimFrame.Mul(
				_pAnimFrameResultStack[nAnimFrameSrcStackNextFree-2],
				_pAnimFrameResultStack[nAnimFrameSrcStackNextFree-1]
			);

			_pAnimFrameResultStack[nAnimFrameSrcStackNextFree-2] = AnimFrame;
			nAnimFrameSrcStackNextFree--;

			break;

		case EXECCMD_SUM_BLEND:
			// Perform LERP( fControl, Input0, Input0 + Input1 )...

			pnExecPointer++;
			pCombinerMix = &m_pCombinerMixArray[ *pnExecPointer ];

			AnimFrame2.Mul(
				_pAnimFrameResultStack[nAnimFrameSrcStackNextFree-2],
				_pAnimFrameResultStack[nAnimFrameSrcStackNextFree-1]
			);

			AnimFrame.ReceiveSlerpOf(
				pCombinerMix->m_fValue,
				_pAnimFrameResultStack[nAnimFrameSrcStackNextFree-2],
				AnimFrame2
			);

			_pAnimFrameResultStack[nAnimFrameSrcStackNextFree-2] = AnimFrame;
			nAnimFrameSrcStackNextFree--;

			break;

		case EXECCMD_SUM_BLEND_SMOOTH:
			// Perform LERP( Smooth(fControl), Input0, Input0 + Input1 )...

			pnExecPointer++;
			pCombinerMix = &m_pCombinerMixArray[ *pnExecPointer ];

			AnimFrame2.Mul(
				_pAnimFrameResultStack[nAnimFrameSrcStackNextFree-2],
				_pAnimFrameResultStack[nAnimFrameSrcStackNextFree-1]
			);

			AnimFrame.ReceiveSlerpOf(
				fmath_UnitLinearToSCurve( pCombinerMix->m_fValue ),
				_pAnimFrameResultStack[nAnimFrameSrcStackNextFree-2],
				AnimFrame2
			);

			_pAnimFrameResultStack[nAnimFrameSrcStackNextFree-2] = AnimFrame;
			nAnimFrameSrcStackNextFree--;

			break;

		default:
			// Tap index...

			nAnimBoneIndex = m_pCombinerTapArray[ *pnExecPointer ].m_pBoneIndexMeshToAnimTable[nBoneIndex];
			m_pCombinerTapArray[ *pnExecPointer ].m_pAnimSource->ComputeFrame( _pAnimFrameResultStack[nAnimFrameSrcStackNextFree++], nAnimBoneIndex, m_nKeyInterpolationMode );

			#if FPERF_ENABLE
				if ( m_nKeyInterpolationMode == CFAnimSource::LERP_KEYS )
				{
					FPerf_nTotalKeysInterpolated++;
				}
				else
				{
					FPerf_nTotalKeysNotInterpolated++;
				}
			#endif

			break;
		}
	}

	FASSERT( nAnimFrameSrcStackNextFree==0 || nAnimFrameSrcStackNextFree==1 );

	if( nAnimFrameSrcStackNextFree == 0 ) {
		rDestFrameMtx43A = m_pMeshInst->m_pMesh->pBoneArray[nBoneIndex].AtRestBoneToParentMtx;
	} else {

		_pAnimFrameResultStack->BuildMtx( rDestFrameMtx43A );

#if 0
		f32 fOOMag;
		rDestFrameMtx43A.m_vUp.Cross( rDestFrameMtx43A.m_vFront, rDestFrameMtx43A.m_vRight );
		rDestFrameMtx43A.m_vRight.Cross( rDestFrameMtx43A.m_vUp, rDestFrameMtx43A.m_vFront );

		fOOMag = 1.0f / fmath_AcuSqrt( rDestFrameMtx43A.m_vRight.x*rDestFrameMtx43A.m_vRight.x + rDestFrameMtx43A.m_vRight.y*rDestFrameMtx43A.m_vRight.y + rDestFrameMtx43A.m_vRight.z*rDestFrameMtx43A.m_vRight.z );
		rDestFrameMtx43A.m_vRight.Mul( fOOMag );

		fOOMag = 1.0f / fmath_AcuSqrt( rDestFrameMtx43A.m_vUp.x*rDestFrameMtx43A.m_vUp.x + rDestFrameMtx43A.m_vUp.y*rDestFrameMtx43A.m_vUp.y + rDestFrameMtx43A.m_vUp.z*rDestFrameMtx43A.m_vUp.z );
		rDestFrameMtx43A.m_vUp.Mul( fOOMag );

		fOOMag = 1.0f / fmath_AcuSqrt( rDestFrameMtx43A.m_vFront.x*rDestFrameMtx43A.m_vFront.x + rDestFrameMtx43A.m_vFront.y*rDestFrameMtx43A.m_vFront.y + rDestFrameMtx43A.m_vFront.z*rDestFrameMtx43A.m_vFront.z );
		rDestFrameMtx43A.m_vFront.Mul( fOOMag );

//		rDestFrameMtx43A.m_vRight.Unitize();
//		rDestFrameMtx43A.m_vUp.Unitize();
//		rDestFrameMtx43A.m_vFront.Unitize();
#endif

/*	 
//	pgm was testing this out because he thinks that the mtx's that come out of here
//		aren't quite orthogonal, which isn't that bad, except for when blink is sticking to one of them
//		and really needs them to be correct.  Anyway this seemed to fix the problem if the bone was
//		only translating. But, if the bone was rotating as well, it didn't even fix it.

		f32 fMag = (*_pAnimFrameResultStack).v.MagSq();
		FASSERT(fMag > 0.0f);
		fMag = 1.0f/fmath_AcuSqrt(fMag);
		(*_pAnimFrameResultStack).x *= fMag;
		(*_pAnimFrameResultStack).y *= fMag;
		(*_pAnimFrameResultStack).z *= fMag;
		(*_pAnimFrameResultStack).w *= fMag;
		f32 fMagSecond = (*_pAnimFrameResultStack).v.MagSq();
		(*_pAnimFrameResultStack).BuildMtx( rDestFrameMtx43A );
*/
	}
}

CFAnimCombinerNode *CFAnimCombiner::_FindCombinerNodeDrivingInput( const CFAnimCombinerNode *pCombinerNode, u32 nInputIndex ) {
	const CFAnimMixer *pUpstreamMixer;
	u32 i;

	FASSERT( nInputIndex < 2 );

	pUpstreamMixer = pCombinerNode->m_pMixer->GetUpstreamMixer(nInputIndex);

	// See if a tap is driving the specified input...
	for( i=0; i<m_nCombinerTapCount; i++ ) {
		if( m_pCombinerTapArray[i].m_pMixer == pUpstreamMixer ) {
			return &m_pCombinerTapArray[i];
		}
	}

	// See if a mixer is driving the specified input...
	for( i=0; i<m_nCombinerMixCount; i++ ) {
		if( m_pCombinerMixArray[i].m_pMixer == pUpstreamMixer ) {
			return &m_pCombinerMixArray[i];
		}
	}

	return NULL;
}

void CFAnimCombiner::_UpdateMixerBoneMasksFromTapToRoot( const CFAnimCombinerTap *pCombinerTap ) {
	CFAnimCombinerNode *pCombinerNode;
	u32 i;

	FASSERT( pCombinerTap->m_pMixer->IsTap() );

	for( pCombinerNode=pCombinerTap->m_pDownstreamNode; pCombinerNode; pCombinerNode=pCombinerNode->m_pDownstreamNode ) {
		FASSERT( pCombinerNode->m_pMixer->IsMixer() );

		for( i=0; i<m_nBoneBitArrayElementCount; i++ ) {
			pCombinerNode->m_anBoneMask[i] = pCombinerNode->m_apUpstreamNode[0]->m_anBoneMask[i] | pCombinerNode->m_apUpstreamNode[1]->m_anBoneMask[i];
		}
	}
}

void CFAnimCombiner::_BuildBoneExecBuffer( u32 nBoneIndex ) {
	FASSERT( nBoneIndex < m_nBoneCount );

	m_nBoneIndex = nBoneIndex;
	m_nNextCmdIndex = 0;
	m_pnExecBuf = m_ppnBoneMixExecBuf[nBoneIndex];

	_BuildBoneExecBufferAndRecurse( m_pRootCombinerNode );

	_AddTokenToExecBuf( EXECCMD_END );
}

void CFAnimCombiner::_BuildBoneExecBufferAndRecurse( CFAnimCombinerNode *pCombinerNode ) {
	CFAnimCombinerMix *pCombinerMix;
	BOOL abUpstreamNodeIsDrivingBone[2];
	f32 fMixerControlUnitValue;

	if( pCombinerNode->m_pMixer->IsTap() ) {
		// Node is a tap...
		_AddTokenToExecBuf( ((CFAnimCombinerTap *)pCombinerNode)->m_nCombinerArrayIndex );
		return;
	}

	// Node is a mixer...
	abUpstreamNodeIsDrivingBone[0] = _GetBoneBitMask( pCombinerNode->m_apUpstreamNode[0], m_nBoneIndex );
	abUpstreamNodeIsDrivingBone[1] = _GetBoneBitMask( pCombinerNode->m_apUpstreamNode[1], m_nBoneIndex );

	if( !(abUpstreamNodeIsDrivingBone[0] | abUpstreamNodeIsDrivingBone[1]) ) {
		// Neither input to this mixer is driving our bone...
		return;
	}

	if( !abUpstreamNodeIsDrivingBone[1] ) {
		// Only input 0 is driving our bone...
		_BuildBoneExecBufferAndRecurse( pCombinerNode->m_apUpstreamNode[0] );
		return;
	}

	if( !abUpstreamNodeIsDrivingBone[0] ) {
		// Only input 1 is driving our bone...
		_BuildBoneExecBufferAndRecurse( pCombinerNode->m_apUpstreamNode[1] );
		return;
	}

	// Both inputs are driving our bone...

	pCombinerMix = (CFAnimCombinerMix *)pCombinerNode;
	fMixerControlUnitValue = pCombinerMix->m_fValue;

	if( pCombinerMix->m_pMixer->IsBlender() ) {
		// Our mixer is a blender...

		if( fMixerControlUnitValue == 0.0f ) {
			// Drive bone from 100% input 0...
			_BuildBoneExecBufferAndRecurse( pCombinerMix->m_apUpstreamNode[0] );
		} else if( fMixerControlUnitValue == 1.0f ) {
			// Drive bone from 100% input 1...
			_BuildBoneExecBufferAndRecurse( pCombinerMix->m_apUpstreamNode[1] );
		} else {
			// Drive bone from a blend of input 0 and 1...
			_BuildBoneExecBufferAndRecurse( pCombinerMix->m_apUpstreamNode[0] );
			_BuildBoneExecBufferAndRecurse( pCombinerMix->m_apUpstreamNode[1] );
			_AddTokenToExecBuf( !(pCombinerMix->m_nFlags & CFAnimCombinerMix::FLAG_SMOOTH) ? EXECCMD_BLEND_LINEAR : EXECCMD_BLEND_SMOOTH );
			_AddTokenToExecBuf( pCombinerMix->m_nCombinerArrayIndex );
		}
	} else {
		// Our mixer is a summer...

		if( fMixerControlUnitValue == 0.0f ) {
			// Drive bone from 100% input 0...
			_BuildBoneExecBufferAndRecurse( pCombinerMix->m_apUpstreamNode[0] );
		} else if( fMixerControlUnitValue == 1.0f ) {
			// Drive bone from 100% sum of inputs 0 and 1...
			_BuildBoneExecBufferAndRecurse( pCombinerMix->m_apUpstreamNode[0] );
			_BuildBoneExecBufferAndRecurse( pCombinerMix->m_apUpstreamNode[1] );
			_AddTokenToExecBuf( EXECCMD_SUM );
		} else {
			// Drive bone from a blend of input 0 and the sum of inputs 0 and 1...
			_BuildBoneExecBufferAndRecurse( pCombinerMix->m_apUpstreamNode[0] );
			_BuildBoneExecBufferAndRecurse( pCombinerMix->m_apUpstreamNode[1] );
			_AddTokenToExecBuf( !(pCombinerMix->m_nFlags & CFAnimCombinerMix::FLAG_SMOOTH) ? EXECCMD_SUM_BLEND : EXECCMD_SUM_BLEND_SMOOTH );
			_AddTokenToExecBuf( pCombinerMix->m_nCombinerArrayIndex );
		}
	}
}


