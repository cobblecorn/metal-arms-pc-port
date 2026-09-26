//////////////////////////////////////////////////////////////////////////////////////
// TriPacketList.h - 
//
// Author: Michael Starich   
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
// 05/03/01 Starich     Created.
//////////////////////////////////////////////////////////////////////////////////////
#ifndef _TRI_PACKET_LIST_H_
#define _TRI_PACKET_LIST_H_ 1

#include "fang.h"
#include "TriCollection.h"
#include "TriPacket.h"

class CTriPacketList
{
public:
	CTriPacketList();
	~CTriPacketList();
	
	u32 Create( const CPtrArray &rKongTris, u32 uKongTris, u32 uStartingKTriIndex, BOOL bGenerateStrips, 
				TriCollection_SearchType_e nSearchType=TRI_COLLECTION_SEARCH_TYPE_NEAREST,
				u32 nMaxVertsPerPacket=0 );
	u32 GetNumPackets() { return m_nNumPackets; }
	CTriPacket *GetTriPacket( u32 nIndex );

	void Reset();

protected:
	
private:
	u32 m_nNumPackets;
	CPtrArray m_aPackets;// CTriPacket *

};



#endif

