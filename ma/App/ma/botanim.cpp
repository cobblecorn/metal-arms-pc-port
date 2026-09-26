//////////////////////////////////////////////////////////////////////////////////////
// botanim.cpp - General bot animation functionality.
//
// Author: Steve Ranck     
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
// 03/26/02 Ranck       Created.
//////////////////////////////////////////////////////////////////////////////////////

#include "fang.h"
#include "botanim.h"
#include "fanim.h"
#include "fresload.h"
#include "fclib.h"


static CFAnimManMtx _DummyManMtx;
static cchar *_aszDummyBoneName[1] = {
	"DummyBone",
};




//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CBotAnimStackDef
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

BOOL CBotAnimStackDef::Create( const Init_t *pInit ) {
	cchar *pszNameToSnapTo;
	u32 i, nInputIndexToSnapTo;

	fang_MemZero( this, sizeof(CBotAnimStackDef) );

	FResFrame_t ResFrame = fres_GetFrame();

	FASSERT( !IsCreated() );

	// Make sure that they provided either a configuration net *or* or a configuration stack.
	FASSERT( (pInit->pBaseAnimConfigNet==NULL && pInit->pBaseAnimConfigStack!=NULL) || (pInit->pBaseAnimConfigNet!=NULL && pInit->pBaseAnimConfigStack==NULL) );

	m_pCombinerConfig = fnew CFAnimCombinerConfig;
	if( m_pCombinerConfig == NULL ) {
		DEVPRINTF( "CBotAnimStackDef::Create(): Not enough memory to allocate CFAnimCombinerConfig.\n" );
		goto _ExitCreateWithError;
	}

	// Build our control name set...
	if( !_BuildNameSet( &m_NameSetControls, pInit->nBaseControlCount, pInit->nUserCount, pInit->nIdleCount, pInit->apszBaseControlNameTable ) ) {
		goto _ExitCreateWithError;
	}

	// Build our tap name set...
	if( !_BuildNameSet( &m_NameSetTaps, pInit->nBaseTapCount, pInit->nUserCount, pInit->nIdleCount, pInit->apszBaseTapNameTable ) ) {
		goto _ExitCreateWithError;
	}

	// Set up remaining fields...
	m_apszBaseAnimNameTable = pInit->apszBaseAnimNameTable;
	m_apszIdleAnimNameTable = pInit->apszIdleAnimNameTable;
	m_nBaseAnimNameCount = pInit->nBaseAnimNameCount;
	m_nUserCount = pInit->nUserCount;
	m_nIdleCount = pInit->nIdleCount;
	m_pBaseAnimAttachList = pInit->pBaseAnimAttachList;
	m_nBoneCount = pInit->nBoneCount;
	m_apszBoneNameTable = pInit->apszBoneNameTable;
	m_ppnEnableBoneNameIndexTableForEachBaseTap = pInit->ppnEnableBoneNameIndexTableForEachBaseTap;
	m_ppnEnableBoneNameIndexTableForEachIdleTap = pInit->ppnEnableBoneNameIndexTableForEachIdleTap;
	m_pnEnableBoneNameIndexTableForSummer = pInit->pnEnableBoneNameIndexTableForSummer;

	// Initialize our combiner config...
	pszNameToSnapTo = m_NameSetControls.apszNameTable[m_NameSetControls.nSummerIndex];
	if( !m_pCombinerConfig->BeginCreation( pszNameToSnapTo, CFAnimMixer::TYPE_SUMMER ) ) {
		goto _ExitCreateWithError;
	}

	if( !m_pCombinerConfig->AddTap( pszNameToSnapTo, pszNameToSnapTo, 1 ) ) {
		goto _ExitCreateWithError;
	}

	// Add the user anim stack...
	for( i=0; i<m_nUserCount; i++ ) {
		if( !m_pCombinerConfig->AddBlender( m_NameSetControls.apszNameTable[m_NameSetControls.nFirstUserIndex + i], pszNameToSnapTo, 0 ) ) {
			goto _ExitCreateWithError;
		}

		pszNameToSnapTo = m_NameSetControls.apszNameTable[m_NameSetControls.nFirstUserIndex + i];
	}

	// Add the base anim stack/net...
	if( pInit->pBaseAnimConfigNet ) {
		// Caller provided an animation configuration network...
		if( !m_pCombinerConfig->AddNet( pInit->pBaseAnimConfigNet, FALSE, m_NameSetControls.apszNameTable, pszNameToSnapTo, 0 ) ) {
			goto _ExitCreateWithError;
		}
	} else {
		// Caller provided an animation configuration stack...
		if( !m_pCombinerConfig->AddStack( pInit->pBaseAnimConfigStack, FALSE, m_NameSetControls.apszNameTable, pszNameToSnapTo, 0 ) ) {
			goto _ExitCreateWithError;
		}
	}

	// Add the idle anim stack...
	pszNameToSnapTo = m_NameSetControls.apszNameTable[pInit->nIdleStackConnectionControlIndex];
	nInputIndexToSnapTo = pInit->nIdleStackConnectionControlInput;
	for( i=0; i<m_nIdleCount; i++ ) {
		if( !m_pCombinerConfig->AddBlender( m_NameSetControls.apszNameTable[m_NameSetControls.nFirstIdleIndex + i], pszNameToSnapTo, nInputIndexToSnapTo ) ) {
			goto _ExitCreateWithError;
		}

		pszNameToSnapTo = m_NameSetControls.apszNameTable[m_NameSetControls.nFirstIdleIndex + i];
		nInputIndexToSnapTo = 0;
	}

	// Add our user taps...
	for( i=0; i<m_nUserCount; i++ ) {
		pszNameToSnapTo = m_NameSetTaps.apszNameTable[m_NameSetTaps.nFirstUserIndex + i];
		if( !m_pCombinerConfig->AddTap( pszNameToSnapTo, pszNameToSnapTo, 1 ) ) {
			goto _ExitCreateWithError;
		}
	}

	// Add our base taps...
	if( !m_pCombinerConfig->AddTaps( pInit->pBaseAnimConfigTap, m_NameSetTaps.apszNameTable, m_NameSetControls.apszNameTable ) ) {
		goto _ExitCreateWithError;
	}

	// Add our idle taps...
	for( i=0; i<m_nIdleCount; i++ ) {
		pszNameToSnapTo = m_NameSetTaps.apszNameTable[m_NameSetTaps.nFirstIdleIndex + i];
		if( !m_pCombinerConfig->AddTap( pszNameToSnapTo, pszNameToSnapTo, 1 ) ) {
			goto _ExitCreateWithError;
		}
	}

	// Add our stack cap...
	if( m_nIdleCount ) {
		pszNameToSnapTo = m_NameSetTaps.apszNameTable[m_NameSetTaps.nFirstIdleIndex + m_nIdleCount - 1];
	} else if( m_NameSetTaps.nBaseCount ) {
		pszNameToSnapTo = m_NameSetTaps.apszNameTable[0];
	} else if( m_nUserCount ) {
		pszNameToSnapTo = m_NameSetTaps.apszNameTable[m_NameSetTaps.nFirstUserIndex + m_nUserCount - 1];
	} else {
		pszNameToSnapTo = BOTANIM_SUMMER_NAME;
	}

	if( !m_pCombinerConfig->AddTap( BOTANIM_DUMMY_TAP_NAME, pszNameToSnapTo, 0 ) ) {
		goto _ExitCreateWithError;
	}

	// Combiner config creation complete...
	if( !m_pCombinerConfig->EndCreation() ) {
		goto _ExitCreateWithError;
	}

	// Create a dummy _DummyManMtx to cap our animation stack with...
	if( !_DummyManMtx.IsCreated() ) {
		if( !_DummyManMtx.Create( 1, _aszDummyBoneName ) ) {
			DEVPRINTF( "CBotAnimStackDef::Create(): Trouble creating _DummyManMtx.\n" );
			goto _ExitCreateWithError;
		}
	}

	return TRUE;

	// Failure...
_ExitCreateWithError:
	Destroy();
	fres_ReleaseFrame( ResFrame );
	return FALSE;
}


void CBotAnimStackDef::Destroy( void ) {
	if( IsCreated() ) {
		fdelete( m_pCombinerConfig );
		m_pCombinerConfig = NULL;
	}
}


BOOL CBotAnimStackDef::_BuildNameSet( NameSet_t *pNameSet, u32 nBaseCount, u32 nUserCount, u32 nIdleCount, cchar **apszBaseAnimNameTable ) {
	FResFrame_t ResFrame;
	FMemFrame_t MemFrame;
	u32 i, nIndex, nMaxUserIndex, nMaxIdleIndex, nUserStringLen, nIdleStringLen;
	char *pszUserName, *pszIdleName;

	ResFrame = fres_GetFrame();
	MemFrame = fmem_GetFrame();

	// Set up our anim user string prefix...
	nUserStringLen = fclib_strlen( BOTANIM_USER_NAME_PREFIX ) + USER_NAME_INDEX_CHAR_COUNT;
	pszUserName = (char *)fmem_Alloc( nUserStringLen + 1 );
	if( pszUserName == NULL ) {
		DEVPRINTF( "CBotAnimStackDef::_BuildNameSet(): Not enough memory to allocate pszUserName.\n" );
		goto _ExitBuildNameSet;
	}
	nMaxUserIndex = _InitAnimStringPrefix( pszUserName, nUserStringLen, BOTANIM_USER_NAME_PREFIX, USER_NAME_INDEX_CHAR_COUNT );
	if( nUserCount > nMaxUserIndex ) {
		DEVPRINTF( "CBotAnimStackDef::_BuildNameSet(): Too many user animations requested (requested=%u, max=%u).\n", nUserCount, nMaxUserIndex );
		goto _ExitBuildNameSet;
	}

	// Set up our anim idle string prefix...
	nIdleStringLen = fclib_strlen( BOTANIM_IDLE_NAME_PREFIX ) + IDLE_NAME_INDEX_CHAR_COUNT;
	pszIdleName = (char *)fmem_Alloc( nIdleStringLen + 1 );
	if( pszIdleName == NULL ) {
		DEVPRINTF( "CBotAnimStackDef::_BuildNameSet(): Not enough memory to allocate pszIdleName.\n" );
		goto _ExitBuildNameSet;
	}
	nMaxIdleIndex = _InitAnimStringPrefix( pszIdleName, nIdleStringLen, BOTANIM_IDLE_NAME_PREFIX, IDLE_NAME_INDEX_CHAR_COUNT );
	if( nIdleCount > nMaxIdleIndex ) {
		DEVPRINTF( "CBotAnimStackDef::_BuildNameSet(): Too many user animations requested (requested=%u, max=%u).\n", nIdleCount, nMaxIdleIndex );
		goto _ExitBuildNameSet;
	}

	pNameSet->nBaseCount = nBaseCount;
	pNameSet->nTotalCount = nBaseCount + 1 + nUserCount + nIdleCount;
	pNameSet->nSummerIndex = nBaseCount;
	pNameSet->nFirstUserIndex = nBaseCount + 1;
	pNameSet->nFirstIdleIndex = nBaseCount + 1 + nUserCount;
	pNameSet->nIdleCount = nIdleCount;
	pNameSet->apszNameTable = (cchar **)fres_AllocAndZero( pNameSet->nTotalCount * sizeof( cchar * ) );
	if( pNameSet->apszNameTable == NULL ) {
		DEVPRINTF( "CBotAnimStackDef::_BuildNameSet(): Not enough memory to allocate apszNameTable.\n" );
		goto _ExitBuildNameSet;
	}

	// Fill base  names...
	for( i=0; i<nBaseCount; i++ ) {
		pNameSet->apszNameTable[i] = apszBaseAnimNameTable[i];
	}

	// Fill summer name...
	pNameSet->apszNameTable[pNameSet->nSummerIndex] = (cchar *)fres_Alloc( fclib_strlen(BOTANIM_SUMMER_NAME) + 1 );
	if( pNameSet->apszNameTable[pNameSet->nSummerIndex] == NULL ) {
		DEVPRINTF( "CBotAnimStackDef::_BuildNameSet(): Not enough memory to allocate apszNameTable[%u].\n", pNameSet->nSummerIndex );
		goto _ExitBuildNameSet;
	}
	fclib_strcpy( (char *)pNameSet->apszNameTable[pNameSet->nSummerIndex], BOTANIM_SUMMER_NAME );

	// Fill user  names...
	for( nIndex=0, i=pNameSet->nFirstUserIndex; nIndex<nUserCount; nIndex++, i++ ) {
		pNameSet->apszNameTable[i] = (cchar *)fres_Alloc( nUserStringLen + 1 );
		if( pNameSet->apszNameTable[i] == NULL ) {
			DEVPRINTF( "CBotAnimStackDef::_BuildNameSet(): Not enough memory to allocate apszNameTable[%u].\n", i );
			goto _ExitBuildNameSet;
		}

		_StoreAnimStringPrefix( (char *)pNameSet->apszNameTable[i], nUserStringLen, pszUserName, USER_NAME_INDEX_CHAR_COUNT, nIndex );
	}

	// Fill idle  names...
	for( nIndex=0, i=pNameSet->nFirstIdleIndex; nIndex<nIdleCount; nIndex++, i++ ) {
		pNameSet->apszNameTable[i] = (cchar *)fres_Alloc( nIdleStringLen + 1 );
		if( pNameSet->apszNameTable[i] == NULL ) {
			DEVPRINTF( "CBotAnimStackDef::_BuildNameSet(): Not enough memory to allocate apszNameTable[%u].\n", i );
			goto _ExitBuildNameSet;
		}

		_StoreAnimStringPrefix( (char *)pNameSet->apszNameTable[i], nIdleStringLen, pszIdleName, IDLE_NAME_INDEX_CHAR_COUNT, nIndex );
	}

	return TRUE;

	// Error:
_ExitBuildNameSet:
	fmem_ReleaseFrame( MemFrame );
	fres_ReleaseFrame( ResFrame );
	return FALSE;
}


u32 CBotAnimStackDef::_InitAnimStringPrefix( char *pszString, u32 nStringLen, cchar *pszPrefix, u32 nIndexCharCount ) {
	u32 i, nMaxIndex;

	FASSERT( nStringLen <= FANIM_ANIMNAME_LEN );

	fclib_strcpy( pszString, pszPrefix );
	fang_MemSet( &pszString[ nStringLen - USER_NAME_INDEX_CHAR_COUNT ], '_', nIndexCharCount );
	pszString[nStringLen] = 0;

	for( i=0, nMaxIndex=1; i<nIndexCharCount; i++ ) {
		nMaxIndex *= 10;
	}

	return nMaxIndex;
}


void CBotAnimStackDef::_StoreAnimStringPrefix( char *pszString, u32 nStringLen, cchar *pszPrefix, u32 nIndexCharCount, u32 nIndex ) {
	u32 i, nCharIndex;

	fclib_strcpy( pszString, pszPrefix );

	nCharIndex = nStringLen - 1;

	for( i=0; i<nIndexCharCount; i++, nCharIndex--, nIndex/=10 ) {
		pszString[nCharIndex] = (char)( (nIndex % 10) + '0' );
	}
}




//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CBotAnimStack
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

BOOL CBotAnimStack::Create( const CBotAnimStackDef *pAnimStackDef, CFWorldMesh *pMesh ) {
	u32 i;
	s32 nControlID, nTapID, nBonePaletteIndex;

	FASSERT( !IsCreated() );

	fang_MemZero( this, sizeof(CBotAnimStack) );

	FResFrame_t ResFrame = fres_GetFrame();

	m_pBotAnimStackDef = pAnimStackDef;

	if( !pAnimStackDef->IsCreated() ) {
		DEVPRINTF( "CBotAnimStack::Create(): CBotAnimStackDef hasn't been created yet.\n" );
		goto _ExitCreateWithError;
	}

	// Create our animation combiner...
	m_pAnimCombiner = fnew CFAnimCombiner;
	if( m_pAnimCombiner == NULL ) {
		DEVPRINTF( "CBotAnimStack::Create(): Not enough memory to allocate CFAnimCombiner.\n" );
		goto _ExitCreateWithError;
	}

	if( !m_pAnimCombiner->Create( pAnimStackDef->m_pCombinerConfig, pMesh ) ) {
		DEVPRINTF( "CBotAnimStack::Create(): Trouble creating CFAnimCombiner.\n" );
		goto _ExitCreateWithError;
	}

	// Load our base animations...
	if( !_LoadAnims( &m_pLoadedBaseAnimInst, pAnimStackDef->m_nBaseAnimNameCount, pAnimStackDef->m_apszBaseAnimNameTable ) ) {
		DEVPRINTF( "CBotAnimStack::Create(): Trouble loading base animations.\n" );
		goto _ExitCreateWithError;
	}

	// Load our idle animations...
	if( !_LoadAnims( &m_pLoadedIdleAnimInst, pAnimStackDef->m_nIdleCount, pAnimStackDef->m_apszIdleAnimNameTable ) ) {
		DEVPRINTF( "CBotAnimStack::Create(): Trouble loading idle animations.\n" );
		goto _ExitCreateWithError;
	}

	// Record our animation control IDs...
	m_pnAnimControlID = (u8 *)fres_Alloc( pAnimStackDef->m_NameSetControls.nTotalCount * sizeof(u8) );
	if( m_pnAnimControlID == NULL ) {
		DEVPRINTF( "CBotAnimStack::Create(): Not enough memory to allocate m_pnAnimControlID.\n" );
		goto _ExitCreateWithError;
	}

	// Create our bone palette index table...
	m_pnBonePaletteIndex = (u8 *)fres_Alloc( pAnimStackDef->m_nBoneCount * sizeof(u8) );
	if( m_pnBonePaletteIndex == NULL ) {
		DEVPRINTF( "CBotAnimStack::Create(): Not enough memory to allocate m_pnBonePaletteIndex.\n" );
		goto _ExitCreateWithError;
	}

	for( i=0; i<pAnimStackDef->m_nBoneCount; i++ ) {
		nBonePaletteIndex = pMesh->FindBone( pAnimStackDef->m_apszBoneNameTable[i] );

		if( nBonePaletteIndex < 0 ) {
			// Bone not found...
			DEVPRINTF( "CBotAnimStack::Create(): WARNING - Bone name '%s' not found in mesh.\n", pAnimStackDef->m_apszBoneNameTable[i] );
			m_pnBonePaletteIndex[i] = 0;
		} else {
			// Bone found...
			m_pnBonePaletteIndex[i] = (u8)nBonePaletteIndex;
		}
	}

	for( i=0; i<pAnimStackDef->m_NameSetControls.nTotalCount; i++ ) {
		nControlID = m_pAnimCombiner->GetControlID( pAnimStackDef->m_NameSetControls.apszNameTable[i] );

		if( nControlID < 0 ) {
			DEVPRINTF( "CBotAnimStack::Create(): Could not locate Control ID for control '%s'.\n", pAnimStackDef->m_NameSetControls.apszNameTable[i] );
			goto _ExitCreateWithError;
		}

		FASSERT( nControlID>=0 && nControlID<=255 );
		m_pnAnimControlID[i] = (u8)nControlID;
	}

	// Record our animation tap IDs...
	m_pnAnimTapID = (u8 *)fres_Alloc( pAnimStackDef->m_NameSetTaps.nTotalCount * sizeof(u8) );
	if( m_pnAnimTapID == NULL ) {
		DEVPRINTF( "CBotAnimStack::Create(): Not enough memory to allocate m_pnAnimTapID.\n" );
		goto _ExitCreateWithError;
	}

	for( i=0; i<pAnimStackDef->m_NameSetTaps.nTotalCount; i++ ) {
		nTapID = m_pAnimCombiner->GetTapID( pAnimStackDef->m_NameSetTaps.apszNameTable[i] );

		if( nTapID < 0 ) {
			DEVPRINTF( "CBotAnimStack::Create(): Could not locate Tap ID for tap '%s'.\n", pAnimStackDef->m_NameSetTaps.apszNameTable[i] );
			goto _ExitCreateWithError;
		}

		FASSERT( nTapID>=0 && nTapID<=255 );
		m_pnAnimTapID[i] = (u8)nTapID;
	}

	// Allocate m_ppBaseAnimInst...
	m_ppBaseAnimInst = (CFAnimInst **)fres_AllocAndZero( pAnimStackDef->m_NameSetTaps.nBaseCount * sizeof(CFAnimInst *) );
	if( m_ppBaseAnimInst == NULL ) {
		DEVPRINTF( "CBotAnimStack::Create(): Not enough memory to allocate m_ppBaseAnimInst.\n" );
		goto _ExitCreateWithError;
	}

	// Allocate m_ppIdleAnimInst...
	m_ppIdleAnimInst = (CFAnimInst **)fres_AllocAndZero( pAnimStackDef->m_nIdleCount * sizeof(CFAnimInst *) );
	if( m_ppIdleAnimInst == NULL ) {
		DEVPRINTF( "CBotAnimStack::Create(): Not enough memory to allocate m_ppIdleAnimInst.\n" );
		goto _ExitCreateWithError;
	}

	// Create CFAnimManMtx for summer and attach it to the summer tap...
	m_pAnimManMtx = fnew CFAnimManMtx;
	if( m_pAnimManMtx == NULL ) {
		DEVPRINTF( "CBotAnimStack::Create(): Not enough memory to allocate m_pAnimManMtx.\n" );
		goto _ExitCreateWithError;
	}

	if( !m_pAnimManMtx->Create( pAnimStackDef->m_nBoneCount, pAnimStackDef->m_apszBoneNameTable ) ) {
		DEVPRINTF( "CBotAnimStack::Create(): Trouble creating m_pAnimManMtx.\n" );
		goto _ExitCreateWithError;
	}

	// Reset our stack cap...
	nTapID = m_pAnimCombiner->GetTapID( BOTANIM_DUMMY_TAP_NAME );
	if( nTapID < 0 ) {
		DEVPRINTF( "CBotAnimStack::Create(): Could not locate Tap ID for tap '%s'.\n", BOTANIM_DUMMY_TAP_NAME );
		goto _ExitCreateWithError;
	}
	m_pAnimCombiner->AttachToTap( nTapID, &_DummyManMtx );
	m_pAnimCombiner->Mask_DisableAllBones( nTapID );

	Reset();

	return TRUE;

_ExitCreateWithError:
	Destroy();
	fres_ReleaseFrame( ResFrame );
	return FALSE;
}


void CBotAnimStack::Destroy( void ) {
	if( IsCreated() ) {
		fdelete_array( m_pLoadedBaseAnimInst );
		m_pLoadedBaseAnimInst = NULL;
		fdelete_array( m_pLoadedIdleAnimInst );
		m_pLoadedIdleAnimInst = NULL;
		fdelete( m_pAnimManMtx );
		m_pAnimManMtx = NULL;
		fdelete( m_pAnimCombiner );
		m_pAnimCombiner = NULL;

		m_pBotAnimStackDef = NULL;
	}
}

void CBotAnimStack::Reset( void ) {
	const CFAnimMixer *pMixer;
	const CFAnimCombiner::AttachList_t *pAttachList;
	u32 i;

	FASSERT( IsCreated() );

	// Reset our summer stuff...
	m_pAnimCombiner->AttachToTap( m_pnAnimTapID[m_pBotAnimStackDef->m_NameSetTaps.nSummerIndex], m_pAnimManMtx );
	m_pAnimManMtx->Reset();

	// Reset our set of base animations...
	for( pAttachList=m_pBotAnimStackDef->m_pBaseAnimAttachList; pAttachList->nTapIDIndex != 255; pAttachList++ ) {
		if( pAttachList->nAnimSourceIndex != BOTANIM_NULL_ANIMSOURCE_ATTACH ) {
			m_pAnimCombiner->AttachToTap( m_pnAnimTapID[ pAttachList->nTapIDIndex ], &m_pLoadedBaseAnimInst[ pAttachList->nAnimSourceIndex ] );
		} else {
			m_pAnimCombiner->AttachToTap( m_pnAnimTapID[ pAttachList->nTapIDIndex ], NULL );
		}
	}

	for( i=0; i<m_pBotAnimStackDef->m_nBaseAnimNameCount; i++ ) {
		m_pLoadedBaseAnimInst[i].UpdateTime( 0.0f );
	}

	// Reset our set of idle animations...
	for( i=0; i<m_pBotAnimStackDef->m_nIdleCount; i++ ) {
		m_pAnimCombiner->AttachToTap( m_pnAnimTapID[m_pBotAnimStackDef->m_NameSetTaps.nFirstIdleIndex + i], &m_pLoadedIdleAnimInst[i] );
		m_pLoadedIdleAnimInst[i].UpdateTime( 0.0f );
	}

	// Reset our set of user animations...
	for( i=0; i<m_pBotAnimStackDef->m_nUserCount; i++ ) {
		m_pAnimCombiner->AttachToTap( m_pnAnimTapID[m_pBotAnimStackDef->m_NameSetTaps.nFirstUserIndex + i], NULL );
	}

	// Reset m_ppBaseAnimInst...
	for( i=0; i<m_pBotAnimStackDef->m_NameSetTaps.nBaseCount; i++ ) {
		m_ppBaseAnimInst[i] = (CFAnimInst *)m_pAnimCombiner->GetSourceDrivingTap( m_pnAnimTapID[i] );
	}

	// Reset m_ppIdleAnimInst...
	for( i=0; i<m_pBotAnimStackDef->m_nIdleCount; i++ ) {
		m_ppIdleAnimInst[i] = (CFAnimInst *)m_pAnimCombiner->GetSourceDrivingTap( m_pnAnimTapID[m_pBotAnimStackDef->m_NameSetTaps.nFirstIdleIndex + i] );
	}

	// Reset all controls...
	for( i=0; i<m_pBotAnimStackDef->m_NameSetControls.nTotalCount; i++ ) {
		pMixer = m_pAnimCombiner->GetControlMixer( m_pnAnimControlID[i] );
		FASSERT( pMixer );

		if( pMixer->IsSummer() ) {
			m_pAnimCombiner->SetControlValue( m_pnAnimControlID[i], 1.0f );
		} else {
			m_pAnimCombiner->SetControlValue( m_pnAnimControlID[i], 0.0f );
		}
	}

	// Reset base tap masks...
	for( i=0; i<m_pBotAnimStackDef->m_NameSetTaps.nBaseCount; i++ ) {
		m_pAnimCombiner->Mask_UpdateTapBoneMask(
			m_pnAnimTapID[i],
			m_pBotAnimStackDef->m_ppnEnableBoneNameIndexTableForEachBaseTap[i],
			m_pBotAnimStackDef->m_apszBoneNameTable,
			TRUE
		);
	}

	// Reset idle tap masks...
	for( i=0; i<m_pBotAnimStackDef->m_nIdleCount; i++ ) {
		m_pAnimCombiner->Mask_UpdateTapBoneMask(
			m_pnAnimTapID[m_pBotAnimStackDef->m_NameSetTaps.nFirstIdleIndex + i],
			m_pBotAnimStackDef->m_ppnEnableBoneNameIndexTableForEachIdleTap[i],
			m_pBotAnimStackDef->m_apszBoneNameTable,
			TRUE
		);
	}

	// Reset summer mask...
	m_pAnimCombiner->Mask_UpdateTapBoneMask(
		m_pnAnimTapID[m_pBotAnimStackDef->m_NameSetTaps.nSummerIndex],
		m_pBotAnimStackDef->m_pnEnableBoneNameIndexTableForSummer,
		m_pBotAnimStackDef->m_apszBoneNameTable,
		TRUE
	);
}


BOOL CBotAnimStack::_LoadAnims( CFAnimInst **ppAnimInstArray, u32 nAnimNameCount, cchar **pszAnimNameTable ) {
	FAnim_t *pAnimRes;
	u32 i;

	FResFrame_t ResFrame = fres_GetFrame();

	*ppAnimInstArray = NULL;

	*ppAnimInstArray = fnew CFAnimInst [nAnimNameCount];
	if( *ppAnimInstArray == NULL ) {
		DEVPRINTF( "CBotAnimStack::_LoadAnims(): Not enough memory to allocate CFAnimInst array.\n" );
		goto _ExitLoadAnimWithError;
	}

	for( i=0; i<nAnimNameCount; i++ ) {
		pAnimRes = (FAnim_t *)fresload_Load( FANIM_RESNAME, pszAnimNameTable[i] );

		if( pAnimRes == NULL ) {
			DEVPRINTF( "CBotAnimStack::_LoadAnims(): Could not find animation '%s'\n", pszAnimNameTable[i] );
			goto _ExitLoadAnimWithError;
		}

		if( !(*ppAnimInstArray)[i].Create( pAnimRes ) ) {
			DEVPRINTF( "CBotAnimStack::_LoadAnims(): Trouble creating CFAnimInst for '%s'\n", pszAnimNameTable[i] );
			goto _ExitLoadAnimWithError;
		}
	}

	return TRUE;

	// Error...
_ExitLoadAnimWithError:
	fdelete_array( *ppAnimInstArray );
	(*ppAnimInstArray) = NULL;
	fres_ReleaseFrame( ResFrame );
	return FALSE;
}


void CBotAnimStack::UserAnim_DetachAll( void ) {
	u32 i;

	FASSERT( IsCreated() );

	for( i=0; i<m_pBotAnimStackDef->m_nUserCount; i++ ) {
		UserAnim_Attach( i, NULL );
	}
}


void CBotAnimStack::UserAnim_ResetAllControlValues( void ) {
	u32 i;

	FASSERT( IsCreated() );

	for( i=0; i<m_pBotAnimStackDef->m_nUserCount; i++ ) {
		UserAnim_SetControlValue( i, 0.0f );
	}
}

void CBotAnimStack::IdleAnim_DetachAll( void ) {
	u32 i;

	FASSERT( IsCreated() );

	for( i=0; i<m_pBotAnimStackDef->m_nIdleCount; i++ ) {
		IdleAnim_Attach( i, NULL );
	}
}


void CBotAnimStack::IdleAnim_ResetAllControlValues( void ) {
	u32 i;

	FASSERT( IsCreated() );

	for( i=0; i<m_pBotAnimStackDef->m_nIdleCount; i++ ) {
		IdleAnim_SetControlValue( i, 0.0f );
	}
}


