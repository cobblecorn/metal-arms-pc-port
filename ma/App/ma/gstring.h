//////////////////////////////////////////////////////////////////////////////////////
// gstring.h - Sexy, sexy game string module.
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
// 04/24/02 Ranck       Created.
//////////////////////////////////////////////////////////////////////////////////////

#ifndef _GSTRING_H_
#define _GSTRING_H_ 1

#include "fang.h"
#include "fstringtable.h"


#define GSTRING_MAIN		"Main"
#define GSTRING_EVENT		"Event"



FCLASS_NOALIGN_PREFIX class CGString {
//----------------------------------------------------------------------------------------------------------------------------------
// Private Data:
//----------------------------------------------------------------------------------------------------------------------------------
private:

	static BOOL m_bSystemInitialized;		// TRUE if the gstring system has been initialized
	CFStringTable *m_pStringTable;			// Pointer to the string table




//----------------------------------------------------------------------------------------------------------------------------------
// Public Functions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	static BOOL InitSystem( void );
	static void UninitSystem( void );
	static FINLINE BOOL IsSystemInitialized( void ) { return m_bSystemInitialized; }

	FINLINE CGString() { m_pStringTable = NULL; }

	FINLINE u32 GetStringCount( void ) const { FASSERT(m_pStringTable); return m_pStringTable->GetStringCount(); }

	//ANSI Interfaces
	FINLINE s32 ComputeStringIndex( cchar *pszString ) const { FASSERT(m_pStringTable); return m_pStringTable->ComputeStringIndex(pszString); }
	FINLINE cchar *FindString( cchar *pszString ) const { FASSERT(m_pStringTable); return m_pStringTable->FindString(pszString); }
	FINLINE cchar *AddString( cchar *pszString, BOOL bErrorIfAlreadyExists=FALSE ) { FASSERT(m_pStringTable); return m_pStringTable->AddString( pszString, bErrorIfAlreadyExists ); }

	//UNICODE Interfaces
	FINLINE s32 ComputeStringIndex( cwchar *pwszString ) const { FASSERT(m_pStringTable); return m_pStringTable->ComputeStringIndex(pwszString); }
	FINLINE cwchar *FindString( cwchar *pwszString ) const { FASSERT(m_pStringTable); return m_pStringTable->FindString(pwszString); }
	FINLINE cwchar *AddString( cwchar *pwszString, BOOL bErrorIfAlreadyExists=FALSE ) { FASSERT(m_pStringTable); return m_pStringTable->AddString( pwszString, bErrorIfAlreadyExists ); }



//----------------------------------------------------------------------------------------------------------------------------------
// Private Functions:
//----------------------------------------------------------------------------------------------------------------------------------
private:

	BOOL Create( cchar *pszStringTableName );
	void Destroy( void );


	FCLASS_STACKMEM_NOALIGN( CGString );
} FCLASS_NOALIGN_SUFFIX;



extern CGString gstring_Main;
extern CGString gstring_Event;


#endif

