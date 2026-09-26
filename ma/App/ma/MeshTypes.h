//////////////////////////////////////////////////////////////////////////////////////
// MeshTypes.h - 
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
// 03/27/02 Starich     Created.
//////////////////////////////////////////////////////////////////////////////////////
#ifndef _MESH_TYPES_H_
#define _MESH_TYPES_H_ 1

#include "fang.h"


typedef enum {
	MESHTYPES_UNKNOWN = 0,		// m_pUser is undefined
	MESHTYPES_ENTITY,			// m_pUser points to a CEntity game object
	
	MESHTYPES_COUNT
} MeshTypes_e;




#endif

