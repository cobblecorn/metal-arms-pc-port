//////////////////////////////////////////////////////////////////////////////////////
// SeqBankListItemInfo.cpp - 
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
#include "SeqBankListItemInfo.h"


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

CSeqBankListItemInfo::CSeqBankListItemInfo( int nItem, SeqBank_Fx_t *pFx ) : CItemInfo( nItem ) {
	
	m_nId = (u32)nItem;
	m_sId.Format( "%d", m_nId );

	m_sName = pFx->szFxName;

	m_sNumCmds.Format( "%d", pFx->nNumCmds );

	m_pFx = pFx;	
}

CSeqBankListItemInfo::~CSeqBankListItemInfo() {

}



