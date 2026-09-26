#include "fang.h"
#include "Apenew.h"
#include "AIRooms.h"
#include "AIGraph.h"
#include "fres.h"


u32 *_auVertToRoomId = NULL;

BOOL airooms_InitSystem(CAIGraph* pGraph)
{
	FASSERT(!_auVertToRoomId);  //better not be initialized

	_auVertToRoomId = (u32*) fres_AlignedAllocAndZero(4*pGraph->GetNumVerts(), 16);

	return _auVertToRoomId != NULL;
}


extern void airooms_CleanupSystem(void)
{
	//fres_Free(_auVertToRoomId);	
	_auVertToRoomId = NULL;
}


u32 airooms_FindRoomID( const CFVec3A &Pos_WS )
{
	return 0;
}


u32 airooms_IsVertInRoom(u16 uVertId)
{
	if (!_auVertToRoomId)
	{
		return 0;
	}

	return _auVertToRoomId[uVertId];

}
