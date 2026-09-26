//////////////////////////////////////////////////////////////////////////////////////
// GoodieProps.h - Goodie properties class for Mettle Arms.
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
// 05/23/02	Link		Created.
//////////////////////////////////////////////////////////////////////////////////////

#ifndef _GOODIEPROPS_H_
#define _GOODIEPROPS_H_ 1

#include "fgamedata.h"

// Entity shapes that have special properties as goodie spawn points
//   will have one of these.
FCLASS_ALIGN_PREFIX class CGoodieProps
{
public:

	f32 m_afSpeed[2];
	f32 m_fDispersion;

	// Pass this function a table and it will check if it is something that it is interested in.
	// If so, it might instantiate a CGoodieProps for you, point *pGoodieProps to it, and then
	//   will set the appropriate field.
	// Returns true if the table was a table that goodie properties should handle (even if there
	//   was an error in the table).
	static BOOL InterpretTable(FGameDataTableHandle_t hTableHandle, CGoodieProps **ppGoodieProps);

private:

	CGoodieProps();		// Right now, there is no need for anyone else to actually create one of these.

	static void CreateGoodieProps(CGoodieProps **pGoodieProps);

	FCLASS_STACKMEM_NOALIGN(CGoodieProps);
} FCLASS_ALIGN_SUFFIX;

#endif
