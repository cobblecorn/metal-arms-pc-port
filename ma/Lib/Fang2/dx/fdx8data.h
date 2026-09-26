//////////////////////////////////////////////////////////////////////////////////////
// fdx8data.h - 
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
// 10/24/00 Ranck       Created.
//////////////////////////////////////////////////////////////////////////////////////

#ifndef _FDX8DATA_H_
#define _FDX8DATA_H_ 1

#include "fang.h"

#if FANG_PLATFORM_XB
	#include <dsound.h>
#else
	#include "windows.h"
	#include <mmreg.h>
#endif

#define FDX8DATA_VB_SET_BITS		3							// See comment below:
#define FDX8DATA_VB_SET_COUNT		(1<<FDX8DATA_VB_SET_BITS)	// Number of possible vertex buffer sets (there can be this many sets per VB type)

#define FDX8DATA_MESH_MAX_TEXLAYERIDS_PER_MTL	3		// Max allowed texture layer IDs per material
/*
typedef enum {
	// The type describes what the corresponding shader register contains:

	FDX8DATA_MESHSHREGTYPE_IMMEDIATE,	// The register value is a 32-bit constant
	FDX8DATA_MESHSHREGTYPE_MOTIF,		// The register value is an index into the array of MeshMotif_t's
	FDX8DATA_MESHSHREGTYPE_SHTEXINST,	// The register value is an index into the array of FShTexInst_t's

	FDX8DATA_MESHSHREGTYPE_COUNT
} FDX8Data_MeshShRegType_e;


typedef struct {
	CFColorMotif Motif;					// Motif
} FDX8Data_MeshMotif_t;


typedef struct {
	char szName[FSH_NAME_LEN+1];		// Shader name
} FDX8Data_MeshShaderName_t;


typedef struct {						// Single Vertex Normal:
	CFVec3 UnitNorm;					// Unit normal (x, y, z)
} FDX8Data_MeshVn_t;


typedef struct {						// Single Vertex Lighting Definition:
	u8 nA, nR, nG, nB;					// Baked-in color (0-255 for each component)
	u32 nVnIndex;						// Index into vertex normal array (MeshVn_t)
	u32 nPosIndex;						// Index into vertex position array (MeshPos_t)
} FDX8Data_MeshVl_t;


typedef struct {						// Single Pos Coordinate:
	CFVec3 Pos;							// 3D coordinate of vertex (x, y, z)
} FDX8Data_MeshPos_t;


typedef struct {						// Single Texture Coordinate:
	CFVec2 ST;							// ST[0]=s,   ST[1]=t
} FDX8Data_MeshTc_t;


// Note: nVtxComponents must be set properly. By doing so, we insure that vertices
//       will not be compressed across differing vertex buffer types.
typedef struct {							// Single Vertex:
	u32 nVlIndex;							// Index into the vertex lighting array (MeshVl_t)
	u32 nPosIndex;							// Index into the Pos coordinate array (MeshPos_t)
	u32 anTcIndex[FDATA_TC_COUNT_PER_VTX];	// Indices into the texture coordinate array (MeshTc_t), or 0xffffffff if not used
	u8 anWeight[FDATA_VW_COUNT_PER_VTX-1];	// Vertex weights (0=>0.0f, 255->1.0f) of all but last matrix. Set to 0 if not used.

	u8 nVtxComponents;						// Bitfield (this is mucho importanto, Mike!):
											//  Bit 0: Number of vertex colors (0 or 1)
											//  Bits 2-1: Number of vertex weights (0 to 3)
											//  Bits 4-3: Number of vertex texture coordinates (0 to 3)
											//  Bit 7-5: Used to separate materials into unique vertex buffers of the same type
} FDX8Data_MeshVtx_t;


typedef struct {						// Vertex Index:
	u32 nVtxIndex;						// Index into the vertex array (MeshVtx_t)
} FDX8Data_MeshVindex_t;


typedef struct {						// Single Triangle Definition:
	CFSphere BoundSphere;				// Bounding sphere for this triangle (valid only if not skinning)
	CFVec3 FaceUnitNorm;				// Triangle face's unit normal (valid only if not skinning)
	u8 nCollType;						// User-defined collision type for this triangle
} FDX8Data_MeshTri_t;


typedef struct {						// List of Triangles:
	u32 nStartVindex;					// Index into the vindex array (MeshVindex_t) of the list of vertices in this tri-list
	u32 nTriCount;						// Number of consecutive triangles in this tri-list
	u32 nTriIndex;						// Index into triangle array (MeshTri_t) of the first consecutive triangle
} FDX8Data_MeshTriList_t;


typedef struct {						// Single Strip:
	CFSphere BoundSphere;				// Bounding sphere for this strip (valid only if not skinning)
	u32 nStartVindex;					// Index into the vindex array (MeshVindex_t) of the list of vertices in this strip
	u32 nTriCount;						// Number of consecutive triangles in this strip
	u32 nTriIndex;						// Index into triangle array (MeshTri_t) of the first consecutive triangle
} FDX8Data_MeshStrip_t;


typedef struct {						// List of Strips:
	u32 nStripCount;					// Number of consecutive strips in this strip-list
	u32 nStripIndex;					// Index into the strip array (MeshStrip_t) of the first consecutive strip
} FDX8Data_MeshStripList_t;


typedef struct {
	FDX8Data_MeshStripList_t StripList;	// The list of strips for this cluster
	FDX8Data_MeshTriList_t TriList;		// The list of triangles for this cluster
} FDX8Data_MeshCluster_t;


typedef struct {
	FDX8Data_MeshShRegType_e nShRegType;// Defines what's in the shader register with the corresponding index
} FDX8Data_MeshShRegInfo_t;


typedef struct {
	u16 nShNameIndex;					// Index into the shader name table (MeshShaderName_t)
	u16 nShRegCount;					// Number of registers pointed to by nShRegValArrayOffset and nShRegInfoArrayOffset
	u32 nShRegValArrayOffset;			// Offset from the base of Mesh_t to the array of shader registers (see fshader.h)
	u32 nShRegInfoArrayOffset;			// Offset from the base of Mesh_t to an array of MeshShRegInfo_t's
} FDX8Data_MeshShader_t;


typedef struct {
	u8 anTexLayerIDIndex[FDX8DATA_MESH_MAX_TEXLAYERIDS_PER_MTL];// Array of texture layer indices used by this material (255=empty slot) (fill lower elements first)
																// Indices index into the array pointed to by FDX8Data_Mesh_t::nTexLayerIDArrayOffset
	u8 nDepthBiasLevel;					// 0=normal, 1=appear in front of 0, 2=appear in front of 1, etc. (negative values not allowed)
	u8 nVBSetNum;						// The vertex set number
	u8 nVBType;							// The vertex type (FDX8VB_Type_e)
	u8 nMtlFlags;						// Material flags (see FMESH_MTLFLAG_* for info)
	u8 nReserved;						// Padding
	FDX8Data_MeshShader_t Shader;		// Shader info
	FDX8Data_MeshCluster_t Cluster;		// Cluster of primitives
} FDX8Data_MeshMtl_t;


typedef struct {
	u32 nCount;							// Number of consecutive materials in this segment
	u32 nStartIndex;					// Index into the material array (MeshMtl_t) of the first consecutive material
} FDX8Data_MeshMtlList_t;


typedef struct {						// Single Segment:
	CFSphere BoundSphere;				// This segment's bounding sphere
	FDX8Data_MeshMtlList_t MtlList;		// Material list

	u8 nBoneMtxCount;							// Number of simultaneous bone matrices used for vertices within this segment (1=segmented, but not skinned)
	u8 anBoneMtxIndex[FDATA_VW_COUNT_PER_VTX];	// Index into object instance's bone matrix palette (255=none)
} FDX8Data_MeshSeg_t;


typedef struct {						// World Node Info:
	FDX8Data_MeshMtlList_t MtlList;		// Material list
} FDX8Data_MeshNode_t;


typedef struct {
	u16 nMeshNodeIndex;					// Index into the mesh node array (MeshNode_t) for this leaf
} FDX8Data_MeshLeaf_t;


typedef struct {						// Mesh Object:
	char szName[FDATA_MESHNAME_LEN+1];
	CFSphere AtRestBoundSphere;			// Model space bound sphere computed using identity matrices for all bone matrices (if any)

	u32 nWorldID;						// Must match FWorld_t's nMeshID to be attached together (if not a world mesh, set to 0)
	u32 nRootNodeIndex;					// The index into the MeshNode_t array of the root node (0xffffffff if this mesh isn't a world)
	u8 nRootBoneIndex;					// The index into the FMeshBone_t array of the root bone (255 if this mesh has no bones)
	u8 nUsedBoneCount;					// Number of non-void bones which are located at the beginning of nBoneArrayOffset

										// (* = common entries are compressed):
	u32 nTexLayerIDArrayCount;			//   Number of elements in the FMeshTexLayerID_t array
	u32 nBoneArrayCount;				//   Number of elements in the FMeshBone_t array (this is also the number of bone matrices used for skinning)
	u32 nSkeletonIndexArrayCount;		//   Number of elements in the skeleton index array (each element is a u8)
	u32 nLightArrayCount;				//   Number of elements in the FMeshLight_t array
	u32 nNodeArrayCount;				//   Number of elements in the MeshNode_t array (0 for non-world meshes)
	u32 nLeafArrayCount;				//   Number of elements in the MeshLeaf_t array (0 for non-world meshes)
	u32 nSegArrayCount;					//   Number of elements in the MeshSeg_t array
	u32 nMtlArrayCount;					//   Number of elements in the MeshMtl_t array
	u32 nStripArrayCount;				//   Number of elements in the MeshStrip_t array
	u32 nTriArrayCount;					//   Number of elements in the MeshTri_t array
	u32 nVindexArrayCount;				//   Number of elements in the MeshVindex_t array
	u32 nVtxArrayCount;					// * Number of elements in the MeshVtx_t array
	u32 nTcArrayCount;					// * Number of elements in the MeshTc_t array
	u32 nPosArrayCount;					// * Number of elements in the MeshPos_t array
	u32 nVlArrayCount;					// * Number of elements in the MeshVl_t array
	u32 nVnArrayCount;					// * Number of elements in the MeshVn_t array
	u32 nShTexInstArrayCount;			// * Number of elements in the FShTexInst_t array
	u32 nShaderNameArrayCount;			// * Number of elements in the MeshShaderName_t array
	u32 nMotifArrayCount;				// * Number of elements in the MeshMotif_t array

	u32 nTexLayerIDArrayOffset;			// Memory offset in bytes from Mesh_t to the FMeshTexLayerID_t array
	u32 nBoneArrayOffset;				// Memory offset in bytes from Mesh_t to the FMeshBone_t array
	u32 nSkeletonIndexArrayOffset;		// Memory offset in bytes from Mesh_t to the skeleton index array (each element is a u8)
	u32 nLightArrayOffset;				// Memory offset in bytes from Mesh_t to the FMeshLight_t array
	u32 nNodeArrayOffset;				// Memory offset in bytes from Mesh_t to the MeshNode_t array
	u32 nLeafArrayOffset;				// Memory offset in bytes from Mesh_t to the MeshLeaf_t array
	u32 nSegArrayOffset;				// Memory offset in bytes from Mesh_t to the MeshSeg_t array
	u32 nMtlArrayOffset;				// Memory offset in bytes from Mesh_t to the MeshMtl_t array
	u32 nStripArrayOffset;				// Memory offset in bytes from Mesh_t to the MeshStrip_t array
	u32 nTriArrayOffset;				// Memory offset in bytes from Mesh_t to the MeshTri_t array
	u32 nVindexArrayOffset;				// Memory offset in bytes from Mesh_t to the MeshVindex_t array
	u32 nVtxArrayOffset;				// Memory offset in bytes from Mesh_t to the MeshVtx_t array
	u32 nTcArrayOffset;					// Memory offset in bytes from Mesh_t to the MeshTc_t array
	u32 nPosArrayOffset;				// Memory offset in bytes from Mesh_t to the MeshPos_t array
	u32 nVlArrayOffset;					// Memory offset in bytes from Mesh_t to the MeshVl_t array
	u32 nVnArrayOffset;					// Memory offset in bytes from Mesh_t to the MeshVn_t array
	u32 nShTexInstArrayOffset;			// Memory offset in bytes from Mesh_t to the FShTexInst_t array
	u32 nShaderNameArrayOffset;			// Memory offset in bytes from Mesh_t to the MeshShaderName_t array
	u32 nMotifArrayOffset;				// Memory offset in bytes from Mesh_t to the MeshMotif_t array
} FDX8Data_Mesh_t;
*/


typedef struct {
	WAVEFORMATEX WaveFormatEx;
	u16	nSamplesPerBlock;
} FDX8Data_WaveFormatEx_t; // Must be identical to XBOXADPCMWAVEFORMAT.

#define FDX8DATA_WAVEFORMATEX_ID_PCM			0x0001 // Must be identical to WAVE_FORMAT_PCM.
#define FDX8DATA_WAVEFORMATEX_ID_XBOX_ADPCM		0x0069 // Must be identical to WAVE_FORMAT_XBOX_ADPCM.

///////////////////////////////////////
// Streaming Wav File Data (.wvs files):
//
// Data laid out as follows:
//    A) One FDX8Data_WvsFile_Header_t
//	  B) Audio Data (The number of bytes is contained in the header.)
//
#define FDX8DATA_WVS_MAX_CHUNK_BYTES		41040

typedef struct {
	u32 nNumBytes;	// how many bytes follow this structure (not including header)
	f32 fSecs;		// how long (in Secs) is the audio data
	f32 fFreqHz;	// what is the samples per second
	FDX8Data_WaveFormatEx_t WavFormat;
} FDX8Data_WvsFile_Header_t;
#endif

