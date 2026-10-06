#ifndef MA_PC_SCRIPT_GOALS_H
#define MA_PC_SCRIPT_GOALS_H

#include "amx.h"
#include "fclib.h"

// During a native callback, cip points immediately after SYSREQ_C. Only accept
// the compiler's direct zero-argument return-to-global sequence; local variables
// and arbitrary entity references must not become live story-player bindings.
static int PortScriptPlayerStoreOffset( const AMX *pAMX, unsigned nDataSize )
{
	if( !pAMX || !pAMX->base ) return -1;
	const AMX_HEADER *pHdr = (const AMX_HEADER *)pAMX->base;
	if( pHdr->magic != AMX_MAGIC || pHdr->cod < (int)sizeof(AMX_HEADER) ||
		pHdr->dat < pHdr->cod || pAMX->cip < 0 || pAMX->cip % sizeof(cell) ||
		pAMX->cip > pHdr->dat - pHdr->cod - 4 * (int)sizeof(cell) ) return -1;
	const cell *pReturn = (const cell *)(pAMX->base + pHdr->cod + pAMX->cip);
	// SmallAMX: STACK=44, STOR_PRI=15. Bot_GetPlayer has zero arguments.
	if( pReturn[0] != 44 || pReturn[1] != 4 || pReturn[2] != 15 ||
		pReturn[3] < 0 || pReturn[3] % sizeof(cell) || nDataSize < sizeof(cell) ||
		(unsigned)pReturn[3] > nDataSize - sizeof(cell) ) return -1;
	return (int)pReturn[3];
}

// Apply the user's asylum exit requirement to the expanded, initialized retail
// program. Change only the kill comparison; the fourteen-bot roster, spawns,
// death counter, camera sequence, and door commands remain authored behavior.
static BOOL PortPatchScriptGoals( const char *pszScriptName, AMX *pAMX )
{
	if( !pszScriptName || fclib_stricmp( pszScriptName, "xewrasycam1.sma" ) ||
		!pAMX || !pAMX->base ) {
		return FALSE;
	}
	const AMX_HEADER *pHdr = (const AMX_HEADER *)pAMX->base;
	const int nCompareOffset = 0x4b0;
	// SmallAMX's private opcode enum: LOAD_PRI=1, EQ_C_PRI=105, JZER=53.
	// Check the exact retail instruction window before touching its operand;
	// do not reinterpret another revision or accidentally change a roster bound.
	if( pHdr->magic != AMX_MAGIC || pHdr->cod < (int)sizeof(AMX_HEADER) ||
		pHdr->cod % sizeof(cell) || pHdr->dat < pHdr->cod ||
		pHdr->dat - pHdr->cod < nCompareOffset + 3 * (int)sizeof(cell) ) {
		return FALSE;
	}
	cell *pCode = (cell *)(pAMX->base + pHdr->cod);
	cell *pCompare = pCode + nCompareOffset / sizeof(cell);
	if( pCompare[-2] != 1 || pCompare[-1] != 560 ||
		pCompare[0] != 105 || pCompare[2] != 53 ||
		(pCompare[1] != 14 && pCompare[1] != 10) ) {
		DEVPRINTF( "Port: asylum exit kill goal not changed: unexpected script layout.\n" );
		return FALSE;
	}
	if( pCompare[1] == 14 ) {
		pCompare[1] = 10;
		DEVPRINTF( "Port: asylum exit requires 10 kills; all 14 bots retained.\n" );
	}
	return TRUE;
}

#endif
