//////////////////////////////////////////////////////////////////////////////////////
// ItemRepository.h - Repository of item data for Mettle Arms.
//
// Author: Justin Link
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
// 02/11/02 Link		Created.
//////////////////////////////////////////////////////////////////////////////////////
#ifndef _ITEMREPOSITORY_H_
#define _ITEMREPOSITORY_H_ 1

#include "fang.h"


class CItem;

class CItemRepository
{
public:
	static BOOL InitSystem();
	static void UninitSystem();
	
	static CItem *RetrieveEntry( cchar *pszItemTag, u32 *puIndex);
	static CItem *RetrieveEntry( cchar *pszItemName, u32 uEUKLevel, u32 *puIndex );
	static u32 GetNumItems();
	static CItem *RetrieveEntry( u32 nIndex );

private:
	static BOOL m_bIsInitialized;

	static u32 m_nItemCount;// how many items are there
	static CItem *m_paItems;// points to m_nItemCount CItems

	static BOOL InitFromCSV( cchar *pszFileName );	
};

#endif
