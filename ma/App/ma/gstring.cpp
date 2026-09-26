//////////////////////////////////////////////////////////////////////////////////////
// gstring.cpp - Sexy, sexy game string module.
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

#include "fang.h"
#include "gstring.h"



CGString gstring_Main;
CGString gstring_Event;


BOOL CGString::m_bSystemInitialized;


BOOL CGString::InitSystem( void ) {
	FASSERT( !IsSystemInitialized() );

	m_bSystemInitialized = TRUE;

	if( !gstring_Main.Create( GSTRING_MAIN ) ) {
		goto _ExitWithError;
	}

	if( !gstring_Event.Create( GSTRING_EVENT ) ) {
		goto _ExitWithError;
	}

	return TRUE;

_ExitWithError:
	DEVPRINTF( "CGString::InitSystem(): Not enough memory to create string tables.\n" );
	m_bSystemInitialized = FALSE;
	return FALSE;
}


void CGString::UninitSystem( void ) {
	if( IsSystemInitialized() ) {
		gstring_Main.Destroy();
		gstring_Event.Destroy();

		m_bSystemInitialized = FALSE;
	}
}


BOOL CGString::Create( cchar *pszStringTableName ) {
	FASSERT( IsSystemInitialized() );
	FASSERT( m_pStringTable == NULL );

	m_pStringTable = CFStringTable::CreateTable( pszStringTableName );

	if( m_pStringTable ) {
		return TRUE;
	}

	return FALSE;
}


void CGString::Destroy( void ) {
	if( IsSystemInitialized() && m_pStringTable ) {
		m_pStringTable = NULL;
	}
}

