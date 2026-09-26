//////////////////////////////////////////////////////////////////////////////////////
// fguid.h - Globally unique identifier module.
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
// 04/11/02 Ranck       Created.
//////////////////////////////////////////////////////////////////////////////////////

#ifndef _FGUID_H_
#define _FGUID_H_ 1

#include "fang.h"


class CFGuid {
//----------------------------------------------------------------------------------------------------------------------------------
// Public Functions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	CFGuid();

	void Reset( void );
	FINLINE u32 GetNew( void );
	FINLINE u32 GetLastIssued( void ) { return m_nGuid; }




//----------------------------------------------------------------------------------------------------------------------------------
// Private Data:
//----------------------------------------------------------------------------------------------------------------------------------
private:

	u32 m_nGuid;					// The last issued GUID (0=none)


	FCLASS_STACKMEM_NOALIGN( CFGuid );
};


FINLINE u32 CFGuid::GetNew( void ) {
	if( ++m_nGuid == 0 ) {
		++m_nGuid;
	}

	return m_nGuid;
}







#endif

