//////////////////////////////////////////////////////////////////////////////////////
// KongToChimp.h - "Chimps are just broken down kong files (only single segment ones)
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
// 05/21/02 Starich     Created.
//////////////////////////////////////////////////////////////////////////////////////
#ifndef _KONG_TO_CHIMP_H_
#define _KONG_TO_CHIMP_H_ 1

#include "fang.h"
#include "ApeToKongFormat.h"


class CKongToChimp : public CApeToKongFormat
{
public:
	CKongToChimp();
	~CKongToChimp();

	BOOL CreateEmptyMeshFromTemplate( const CApeToKongFormat &rTemplate, u32 nMaxTris, BOOL bPreserveEdgeData=FALSE );
	BOOL AddTriToMesh( KongTri_t &rTri, u32 nMatNum );
	BOOL CloseMesh( BOOL bRemoveUnusedMats=TRUE );
	
protected:
	virtual void FreeData();

private:
	u32 m_nMaxTris;
	KongSeg_t *m_pSeg;// since there is only 1 segment, just cache the ptr
	BOOL m_bPreserveEdgeData;// the tris that are going to be added have valid edge data
	u16 m_nChimpID;// each chimp will get a unique ID
	static u16 m_nChimpCount;

	u32 ComputeNeededMemory( u32 nNumMaterials, u32 nMaxTris );
};

#endif

