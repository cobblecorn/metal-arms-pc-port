//////////////////////////////////////////////////////////////////////////////////////
// BankListItemInfo.cpp - 
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
// 06/04/02 Starich     Created.
//////////////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "fang.h"
#include "BankListItemInfo.h"


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

CBankListItemInfo::CBankListItemInfo( int nItem, WavBank_Entry_t *pEntry ) : CItemInfo( nItem ) {
	
	m_nId = (u32)nItem;
	m_sId.Format( "%d", m_nId );

	m_sFilename = pEntry->szFilename;

	ConvertSecsToString( pEntry->fSecsOfAudio, m_sPlayTime );
	
	m_sChannels.Format( "%d", pEntry->nNumChannels );

	m_sFreq.Format( "%d", pEntry->nSamplesPerSec );

	m_pEntry = pEntry;	
}

CBankListItemInfo::~CBankListItemInfo() {

}

void CBankListItemInfo::ConvertSecsToString( f32 fSecs, CString &rsString ) {
	u32 nHours, nMins;
	BOOL bNeg;

	if( fSecs < 0.0f ) {
		bNeg = TRUE;
		fSecs *= -1.0f;
	} else {
		bNeg = FALSE;
	}

	nHours = (u32)( fSecs * (1.0f/3600.0f) );
	if( nHours ) {
		fSecs -= (f32)(nHours * 3600.0f);
	}

	nMins = (u32)( fSecs * (1.0f/60.0f) );
	if( nMins ) {
		fSecs -= (f32)(nMins * 60.0f);
	}

	if( !bNeg ) {
		rsString.Format( "%02d:%02d:%02.2f", nHours, nMins, fSecs );
	} else {
		rsString.Format( "-%02d:%02d:%02.2f", nHours, nMins, fSecs );	
	}
}

