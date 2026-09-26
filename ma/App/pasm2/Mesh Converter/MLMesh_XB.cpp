//////////////////////////////////////////////////////////////////////////////////////
// MLMesh_XB.cpp - Classes used to convert generic mesh data into Fang XB specific data
//
// Author: John Lafleur
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
// 03/17/02 Lafleur		Created.
// 08/15/02 Lafleur		Adapted from platform specific GCMeshData.cpp
//////////////////////////////////////////////////////////////////////////////////////

#include <math.h>
#include "stdafx.h"
#include <mmsystem.h>

// Fang Includes
#include "FkDOP.h"

// MeshLib Includes
#include "MLMesh_XB.h"



//////////////////////////////////////////////////////////////////////////////////////
// Implementation:
//////////////////////////////////////////////////////////////////////////////////////

//
//
//
MLResults* MLMesh_XB::GenerateExportData( BOOL bGenerateCollisionData )
{
	return &m_Results;
}


