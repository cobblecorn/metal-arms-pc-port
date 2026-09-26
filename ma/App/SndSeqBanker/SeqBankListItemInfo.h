//////////////////////////////////////////////////////////////////////////////////////
// SeqBankListItemInfo.h - 
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
#ifndef _SEQ_BANK_LIST_ITEM_INFO_H_
#define _SEQ_BANK_LIST_ITEM_INFO_H_ 1


#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "fang.h"
#include "ItemInfo.h"
#include "SeqBank.h"

class CSeqBankListItemInfo : public CItemInfo  
{
public:
	CSeqBankListItemInfo( int nItem, SeqBank_Fx_t *pFx );
	virtual ~CSeqBankListItemInfo();

	CString &GetIdString()			{ return m_sId; }
	u32 GetId()						{ return m_nId; }

	CString &GetNameString()		{ return m_sName; }
	
	CString &GetNumCmdsString()		{ return m_sNumCmds; }
	u32 GetNumCmds()				{ return m_pFx->nNumCmds; }

	SeqBank_Fx_t *GetFx()			{ return m_pFx; }
			
private:

	CSeqBankListItemInfo();

	CString m_sId;
	CString m_sName;
	CString m_sNumCmds;

	u32 m_nId;
	SeqBank_Fx_t *m_pFx;	
};


#endif
