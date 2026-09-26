//////////////////////////////////////////////////////////////////////////////////////
// fGCdisplaylist.h - Fang GameCube Display List module.
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
// 03/19/02	Lafleur		Created.
//////////////////////////////////////////////////////////////////////////////////////

#ifndef _FGCDISPLAYLIST_H_
#define _FGCDISPLAYLIST_H_ 1

#if FANG_PLATFORM_GC
#include "fgc.h"
#endif
#include "fang.h"
#include "fgcvb.h"
#include "fgcsh.h"
//#include "fdatastreaming.h"

//#include "dolphin\GD.h"

class CFMeshInst;

extern u32	FGCDL_nFastDLCalls;
extern u32	FGCDL_nFlushDLCalls;
extern u8	FGCDL_ActiveTextures;

//
//
enum
{
	FGCDL_FLAGS_NONE					= 0x00,
	FGCDL_FLAGS_SKINNED					= 0x01,
	FGCDL_FLAGS_CONSTANT_COLOR			= 0x02,
	FGCDL_FLAGS_BUMPMAP					= 0x04,
	FGCDL_FLAGS_FACING_OPP_DIR_LIGHT	= 0x08,
	FGCDL_FLAGS_STREAMING				= 0x80
};

//
//
struct FGC_DLCont_t
{
	u8			nFlags;			// See display list flags, above
	u8			nMatrixIdx;		// Matrix index for this display list
	u8			nLODID;			// ID for the LOD this display list is part of (0 is closest)
	u8			nPartID;		// Matrix index for this display list
	u16			nStripTriCount; // Number of stripped triangles in this display list
	u16			nListTriCount;	// Number of list triangles in this display list
	u16			nStripCount;	// Number of tri strips
	u8			nListCount;		// Number of tri lists
	u8			nVBIndex;		// Index into the mesh's vertex buffers indicating which 
	
	u32 		nSize;			// size of pBuffer
	void		*pBuffer;
	
	FGCColor_t 	ConstantColor;

	void ChangeEndian( void )
	{
		nFlags = fang_ConvertEndian( nFlags );
		nMatrixIdx = fang_ConvertEndian( nMatrixIdx );
		nLODID = fang_ConvertEndian( nLODID );
		nPartID = fang_ConvertEndian( nPartID );
		nVBIndex = fang_ConvertEndian( nVBIndex );
		nSize = fang_ConvertEndian( nSize );
		pBuffer = fang_ConvertEndian( pBuffer );
		nStripTriCount = fang_ConvertEndian( nStripTriCount );
		nListTriCount = fang_ConvertEndian( nListTriCount );
		nStripCount = fang_ConvertEndian( nStripCount );
		nListCount = fang_ConvertEndian( nListCount );
	}

	void Submit( CFMeshInst *pMeshInst, u16 nSTSets, BOOL bActivateChannel2=FALSE );
};



#endif