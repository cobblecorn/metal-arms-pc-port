//////////////////////////////////////////////////////////////////////////////////////
// fdx8vb.h - Fang DX vertex buffer module.
//
// Author: Steve Ranck     
//////////////////////////////////////////////////////////////////////////////////////
// THIS CODE IS PROPRIETARY PROPERTY OF SWINGIN' APE STUDIOS, INC.
// Copyright (c) 2000
//
// The contents of this file may not be disclosed to third
// parties, copied or duplicated in any form, in whole or in part,
// without the prior written permission of Swingin' Ape Studios, Inc.
//////////////////////////////////////////////////////////////////////////////////////
// Modification History:
//
// Date     Who         Description
// -------- ----------  --------------------------------------------------------------
// 11/09/00 Ranck       Created.
//////////////////////////////////////////////////////////////////////////////////////

#ifndef _FDX8VB_H_
#define _FDX8VB_H_ 1

#include "fang.h"
#include "fdx8.h"
#include "flinklist.h"



typedef enum 
{
	FDX8VB_TYPE_N1C1T1,			// 1 normal, 1 color, 1 TC
	FDX8VB_TYPE_N1C1T2,			// 1 normal, 1 color, 2 TCs

	FDX8VB_TYPE_N1W3C1T1,		// 1 normal, 3 weights, 1 color, 1 TC
	FDX8VB_TYPE_N1W3C1T2,		// 1 normal, 3 weights, 1 color, 2 TCs

	FDX8VB_TYPE_TL_C2T2,		// Post-transformed-and-lit vertex: 2 color, 2 TCs

	FDX8VB_TYPE_C1,				// 0 normals, 1 color, 0 TCs
	FDX8VB_TYPE_C1T1,			// 0 normals, 1 color, 1 TC

	FDX8VB_TYPE_COUNT
} FDX8VB_Type_e;


struct FDX8VB_Info_t
{
	u32 nFVF;			// D3D Flexible Vertex Format code (0=end of table)
	u8 nVtxBytes;		// Number of bytes per vertex
	BOOL8 bPostTL;		// TRUE: vertex contains already transformed and lit data

	u8 nNormalCount;	// Number of f32,f32,f32 normal fields
	u8 nWeightCount;	// Number of f32 weights
	u8 nColorCount;		// Number of D3DCOLOR color fields
	u8 nTCCount;		// Number of f32,f32 (S,T) texture coordinate pairs

	u8 nOffsetPos;		// Offset from beginning of vertex structure of the 3D coordinate
	u8 nOffsetWeight;	// Offset from beginning of vertex structure of the weight array
	u8 nOffsetNorm;		// Offset from beginning of vertex structure of the normal
	u8 nOffsetColor;	// Offset from beginning of vertex structure of the color array
	u8 nOffsetTC;		// Offset from beginning of vertex structure of the texture coordinate pair array
};


// Type 0:
typedef struct 
{
	f32 fPosX, fPosY, fPosZ;
	f32 fNormX, fNormY, fNormZ;
	DWORD nDiffuseRGBA;
	f32 fS0, fT0;
} FDX8VB_N1C1T1_t;


// Type 1:
typedef struct 
{
	f32 fPosX, fPosY, fPosZ;
	f32 fNormX, fNormY, fNormZ;
	DWORD nDiffuseRGBA;
	f32 fS0, fT0;
	f32 fS1, fT1;
} FDX8VB_N1C1T2_t;


// Type 2:
typedef struct 
{
	f32 fPosX, fPosY, fPosZ;
	f32 fW0, fW1, fW2;
	f32 fNormX, fNormY, fNormZ;
	DWORD nDiffuseRGBA;
	f32 fS0, fT0;
} FDX8VB_N1W3C1T1_t;


// Type 3:
typedef struct 
{
	f32 fPosX, fPosY, fPosZ;
	f32 fW0, fW1, fW2;
	f32 fNormX, fNormY, fNormZ;
	DWORD nDiffuseRGBA;
	f32 fS0, fT0;
	f32 fS1, fT1;
} FDX8VB_N1W3C1T2_t;


// Type 4:
typedef struct 
{
	f32 fPosX, fPosY, fPosZ, fRHW;
	DWORD nDiffuseRGBA;
	DWORD nSpecularRGBA;
	f32 fS0, fT0;
	f32 fS1, fT1;
} FDX8VB_TL_C2T2_t;


// Type 5:
typedef struct 
{
	f32 fPosX, fPosY, fPosZ;
	DWORD nDiffuseRGBA;
} FDX8VB_C1_t;


// Type 6:
typedef struct 
{
	f32 fPosX, fPosY, fPosZ;
	DWORD nDiffuseRGBA;
	f32 fS0, fT0;
} FDX8VB_C1T1_t;

// Type 7:
typedef struct 
{
	f32 fPosX, fPosY, fPosZ;
	f32 fNormX, fNormY, fNormZ;
	DWORD nDiffuseRGBA;
	f32 fS0, fT0;
	f32 fS1, fT1;
	f32 fS2, fT2;
} FDX8VB_N1C1T3_t;


struct FDX8LightMapST_t 
{
	f32 fS, fT;
};


struct FDX8BasisVectors_t 
{
	f32 fTx, fTy, fTz;
	f32 fBx, fBy, fBz;
};

struct FDX8VB_t 
{
	FLink_t Link;					// Link to other VBs

	u32 nVtxCount;					// Number of vertices in this DX vertex buffer
	u16 nBytesPerVertex;			// Number of bytes per vertex

	u16 nLMTCCount;					// Number of f32,f32 (S,T) texture coordinate pairs used for lightmaps, per vertex
	void *pLMUVStream;				// Pointer to the stream of lightmap UV's
	void *pBasisStream;				// Pointer to the stream of basis vectors.

	s8 nInfoIndex;					// Index into FDX8VB_InfoTable[] of the entry that describes this VB format (-1=shader)
	BOOL8 bDynamic;					// TRUE=this VB is dynamic
	BOOL8 bSoftwareVP;				// TRUE=software vertex processing

	BOOL8 bLocked;					// TRUE=this VB is locked
	void *pLockBuf;					// Set when Lock() is called to the memory address that can be filled with data

	u32 nLockOffset;				// Used to restore lock state if we lose the device
	u32 nLockBytes;					// Used to restore lock state if we lose the device

	DWORD hVertexShader;			// Handle to the vertex shader this VB is currently attached to (or FVF code if nInfoIndex is not -1)

	IDirect3DVertexBuffer8 *pDXVB;	// Pointer to the actual DX vertex buffers
};


extern const FDX8VB_Info_t FDX8VB_InfoTable[FDX8VB_TYPE_COUNT];



extern BOOL fdx8vb_ModuleStartup( void );
extern void fdx8vb_ModuleShutdown( void );

extern s32 fdx8vb_FindFormat( BOOL bPostTL, u32 nNormalCount, u32 nWeightCount, u32 nColor1Count, u32 nTCCount );
extern u32 fdx8vb_GetVertexByteCount( FDX8VB_Type_e nType );

extern BOOL fdx8vb_CreateFVF( FDX8VB_t *pVB, u32 nVtxCount, s32 nInfoIndex, BOOL bDynamic, BOOL bSoftwareVertexProcessing, BOOL bPointSprites=FALSE );
extern BOOL fdx8vb_CreateVSH( FDX8VB_t *pVB, u32 nVtxCount, u32 nBytesPerVertex, BOOL bDynamic, BOOL bPointSprites=FALSE );
extern void fdx8vb_Destroy( FDX8VB_t *pVB );

extern void *fdx8vb_Lock( FDX8VB_t *pVB, u32 nStartVtxIndex, u32 nVtxCount );
extern void *fdx8vb_LockEntireVB( FDX8VB_t *pVB );
extern void fdx8vb_Unlock( FDX8VB_t *pVB );

extern BOOL fdx8vb_IsBusy( FDX8VB_t *pVB );
extern void fdx8vb_BlockUntilNotBusy( FDX8VB_t *pVB );

extern FDX8VB_t *fdx8vb_Select( FDX8VB_t *pVB, BOOL bSetVShader = TRUE, BOOL bUseLightMapSTs = FALSE, BOOL bUseBasisVectors = FALSE );
extern FDX8VB_t *fdx8vb_GetSelected( void );
extern void fdx8vb_UncacheSelected( void );

extern void fdx8vb_AttachToVertexShader( FDX8VB_t *pVB, DWORD hVertexShader );



#endif

