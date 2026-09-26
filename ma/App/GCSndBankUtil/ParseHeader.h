//////////////////////////////////////////////////////////////////////////////////////
// ParseHeader.h - 
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
// 06/13/02 Starich     Created.
//////////////////////////////////////////////////////////////////////////////////////
#ifndef _PARSE_HEADER_H_
#define _PARSE_HEADER_H_ 1

#include "fang.h"

class CParseHeader {
public:
	CParseHeader();
	~CParseHeader();

	BOOL Parse( const CString &rsFilename );
	s32 FindGroupIndex( cchar *pszGroupName );
	s32 FindSfxIndex( cchar *pszSfxName );

	u32 m_nNumGRPs;
	CStringList m_asGRPs;	// stores the group names
	CUIntArray m_anGRPs;	// stores the group values	

	u32 m_nNumSFXs;
	CStringList m_asSFXs;	// stores the SFX names
	CUIntArray m_anSFXs;	// stores the SFX values

private:
	void Reset();
};



#endif

