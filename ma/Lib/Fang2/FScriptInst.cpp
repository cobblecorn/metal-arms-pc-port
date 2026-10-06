//////////////////////////////////////////////////////////////////////////////////////
// FScriptInst.cpp - Script Instance Class for Mettle Arms.
//
// Author: Justin Link
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
// 10/24/01 Link		Created.
//////////////////////////////////////////////////////////////////////////////////////


//#include <stdlib.h>			// For malloc()/free().
//#include <string.h>			// For memset().
#include "FScriptInst.h"
#include "FScriptSystem.h"
#if FANG_PLATFORM_WIN
#include "pc_script_goals.h"
#endif

// =============================================================================================================
// Private definitions:
// =============================================================================================================

// =============================================================================================================
// Public variables:
// =============================================================================================================

extern "C" AMX_NATIVE_INFO core_Natives[];
extern "C" AMX_NATIVE_INFO console_Natives[];
//extern AMX_NATIVE_INFO CFScriptSystem::m_aEventNatives[];

// =============================================================================================================
// Public functions:
// =============================================================================================================

BOOL CFScriptInst::Init( CFScript *pScript, void *pObject )
{
	// init the script object to defaults for early outs
	m_bIsInitialized = FALSE;
	m_pObject = NULL;
	m_uFlags = 0;
	m_pDataArea = NULL;
	m_fDisabledCtdn = 0.0f;
	fang_MemZero( &m_oAMX, sizeof(AMX) );

	int nAMXError = amx_Init( &m_oAMX, pScript->m_pProgram );
	if( nAMXError != AMX_ERR_NONE ) {
		DEVPRINTF( "CFScriptInst::Init: amx_Init failed for '%s' (error %d).\n", pScript->m_szScriptFileName, nAMXError );
		return FALSE;
	}

#if FANG_PLATFORM_WIN
	PortPatchScriptGoals( pScript->m_szScriptFileName, &m_oAMX );
#endif
	amx_Register(&m_oAMX, core_Natives, -1);
	amx_Register(&m_oAMX, console_Natives, -1);
	if( amx_Register(&m_oAMX, CFScriptSystem::m_paNativeFunc, CFScriptSystem::m_uNumNatives) != AMX_ERR_NONE ) {
		// Retail scripts may import natives this source snapshot lacks. Keep the
		// script (as the original did) and name them; amx_Callback fails such a
		// call with AMX_ERR_NOTFOUND instead of calling through a NULL pointer.
		const AMX_HEADER *pHdr = (const AMX_HEADER *)m_oAMX.base;
		const AMX_FUNCSTUB *pFunc = (const AMX_FUNCSTUB *)(m_oAMX.base + pHdr->natives);
		for( int i=0; i<pHdr->num_natives; ++i, ++pFunc ) {
			if( pFunc->address == 0 ) {
				DEVPRINTF( "CFScriptInst::Init: '%s' imports unimplemented native '%s'.\n", pScript->m_szScriptFileName, pFunc->name );
			}
		}
	}

	// An OnInit entry point is mandatory.
	if( amx_FindPublic(&m_oAMX, "OnInit", (int *)&m_nOnInitIdx) != AMX_ERR_NONE ) {
		DEVPRINTF( "CFScriptInst::Init: mandatory OnInit missing in '%s'.\n", pScript->m_szScriptFileName );
		return FALSE;
	}
	amx_FindPublic( &m_oAMX, "Work", (int *)&m_nWorkIdx );
	amx_FindPublic( &m_oAMX, "OnEvent", (int *)&m_nOnEventIdx );
	amx_FindPublic( &m_oAMX, "OnEnd", (int *)&m_nOnEndIdx );

	m_uEventFlags = 0;
	m_pObject = pObject;
	m_uFlags = (CFSCRIPTINST_FLAG_DOWORK);

	m_pDataArea = fres_Alloc(pScript->m_uDataAreaSize);
	if( !m_pDataArea ) {
        FASSERT_NOW;
		return FALSE;
	}

	pScript->GetDataArea(m_pDataArea);
	m_pScript = pScript;

	// if we got here, we are good to go
	m_bIsInitialized = TRUE;

	return TRUE;
}

// =============================================================================================================

BOOL CFScriptInst::InitClone( CFScriptInst *pScriptInst, void *pObject ) 
{
	FASSERT( pScriptInst->m_bIsInitialized );

	// init the script object to defaults for early outs
	m_bIsInitialized = FALSE;
	m_pObject = NULL;
	m_uFlags = 0;
	m_pDataArea = NULL;
	m_fDisabledCtdn = 0.0f;
	fang_MemZero( &m_oAMX, sizeof(AMX) );

	// grab the script from our source script inst	
	CFScript *pScript = pScriptInst->m_pScript;
	FASSERT( pScript );

	fang_MemCopy( &m_oAMX, &pScriptInst->m_oAMX, sizeof(AMX) );

	m_nOnInitIdx = pScriptInst->m_nOnInitIdx;
	m_nWorkIdx = pScriptInst->m_nWorkIdx;
	m_nOnEventIdx = pScriptInst->m_nOnEventIdx;
	m_nOnEndIdx = pScriptInst->m_nOnEndIdx;

	m_uEventFlags = 0;
	m_pObject = pObject;
	m_uFlags = (CFSCRIPTINST_FLAG_DOWORK);

	m_pDataArea = fres_Alloc( pScript->m_uDataAreaSize );
	if( !m_pDataArea ) {
		FASSERT_NOW;
		return FALSE;
	}
	
	pScript->GetDataArea( m_pDataArea );
	m_pScript = pScript;

	// if we got here, we are good to go
	m_bIsInitialized = TRUE; 

	return TRUE;
}

// =============================================================================================================

BOOL CFScriptInst::OnInit()
{
	FASSERT( m_bIsInitialized );

	cell oRetVal;

	m_pScript->SetDataArea(m_pDataArea);
	if(amx_Exec(&m_oAMX, &oRetVal, m_nOnInitIdx, 1, (cell)(m_pObject)) != AMX_ERR_NONE)
		return(FALSE);
	m_pScript->GetDataArea(m_pDataArea);
	return(TRUE);
}

// =============================================================================================================

BOOL CFScriptInst::Work()
{
	FASSERT( m_bIsInitialized );

	cell oRetVal;

	m_pScript->SetDataArea(m_pDataArea);
	if(amx_Exec(&m_oAMX, &oRetVal, m_nWorkIdx, 1, (cell)(m_pObject)) != AMX_ERR_NONE)
		return(FALSE);
	m_pScript->GetDataArea(m_pDataArea);
	return(TRUE);
}

// =============================================================================================================

BOOL CFScriptInst::OnEvent(u32 uWhichEvent, u32 uEventData1, u32 uEventData2, u32 uEventData3)
{
	FASSERT( m_bIsInitialized );

	cell oRetVal;

	m_pScript->SetDataArea(m_pDataArea);
	if(amx_Exec(&m_oAMX, &oRetVal, m_nOnEventIdx, 5, (cell)(m_pObject), uWhichEvent, uEventData1, uEventData2, uEventData3) != AMX_ERR_NONE)
		return(FALSE);
	m_pScript->GetDataArea(m_pDataArea);
	return(TRUE);
}

// =============================================================================================================

BOOL CFScriptInst::OnEnd()
{
	FASSERT( m_bIsInitialized );

	cell oRetVal;

	// TODO: Are both of these SetDataAreas necessary??
	m_pScript->SetDataArea(m_pDataArea);
	if(amx_Exec(&m_oAMX, &oRetVal, m_nOnEndIdx, 1, (cell)(m_pObject)) != AMX_ERR_NONE)
	{
		return(FALSE);
	}
	m_pScript->SetDataArea(m_pDataArea);
	return(TRUE);
}

// =============================================================================================================

BOOL CFScriptInst::Execute(/*stuff*/)
{
	FASSERT( m_bIsInitialized );

	// This is not yet implemented.
	return(TRUE);
}

// =============================================================================================================

// =============================================================================================================
