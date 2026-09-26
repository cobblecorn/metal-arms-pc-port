//////////////////////////////////////////////////////////////////////////////////////
// BankListItemInfo.h - 
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
#ifndef _BANK_LIST_ITEM_INFO_H_
#define _BANK_LIST_ITEM_INFO_H_ 1


#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "fang.h"
#include "ItemInfo.h"
#include "WavBank.h"

class CBankListItemInfo : public CItemInfo  
{
public:
	CBankListItemInfo( int nItem, WavBank_Entry_t *pEntry );
	virtual ~CBankListItemInfo();

	CString &GetIdString()			{ return m_sId; }
	u32 GetId()						{ return m_nId; }

	CString &GetFilenameString()	{ return m_sFilename; }
	
	CString &GetPlayTimeString()	{ return m_sPlayTime; }
	f32 GetPlaySecs()				{ return m_pEntry->fSecsOfAudio; }
	
	CString &GetChannelsString()	{ return m_sChannels; }
	u32 GetNumChannels()			{ return m_pEntry->nNumChannels; }

	CString &GetFreqString()		{ return m_sFreq; }
	u32 GetFrequency()				{ return m_pEntry->nSamplesPerSec; }

	WavBank_Entry_t *GetEntry()		{ return m_pEntry; }

	static void ConvertSecsToString( f32 fSecs, CString &rsString );
		
private:

	CBankListItemInfo();

	CString m_sId;
	CString m_sFilename;
	CString m_sPlayTime;
	CString m_sChannels;
	CString m_sFreq;

	u32 m_nId;
	WavBank_Entry_t *m_pEntry;	
};


#endif
