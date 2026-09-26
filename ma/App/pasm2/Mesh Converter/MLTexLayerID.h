//////////////////////////////////////////////////////////////////////////////////////
// MLTexLayerID.h - Classes used to convert generic mesh data into Fang platform specific data
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
// 08/15/02 Lafleur		Adapted from GCTexLayerID.h to be platform independent
//////////////////////////////////////////////////////////////////////////////////////

#ifndef __MLTEXLAYERID_H_
#define __MLTEXLAYERID_H_

#include "fang.h"
#include "gc\fGCmesh.h"


// Forward class declarations
class MLMesh;


//
// 
//
class MLTexLayerID
{
	public:
		FMeshTexLayerID_t	m_FMeshTexLayerID;

		MLTexLayerID		*m_pNext;

	public:
		MLTexLayerID( FMeshTexLayerID_t *pTexLayerID )
		{
			fang_MemCopy( &m_FMeshTexLayerID, pTexLayerID, sizeof( FMeshTexLayerID_t) );

			m_pNext = NULL;
		}

		BOOL IsTheSame( FMeshTexLayerID_t *pTexLayerID )
		{
			return fang_IsMemIdentical( &m_FMeshTexLayerID, pTexLayerID, sizeof( FMeshTexLayerID_t ) );
		}

};



#endif