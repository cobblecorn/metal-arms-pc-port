//////////////////////////////////////////////////////////////////////////////////////
// fmesh.h - Fang mesh module.
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
// 09/07/00 Ranck       Created.
//////////////////////////////////////////////////////////////////////////////////////

#ifndef _FMESH_H_
#define _FMESH_H_ 1

#include "fang.h"
#include "fdata.h"
#include "fxfm.h"
#include "flight.h"
#include "fviewport.h"
#include "fshaders.h"

//
// Forward declarations
class CFCollInfo;
class CFMeshInst;
struct FShTexInst_t;
struct ColorStream_t;

extern BOOL FMesh_bRenderShadows;
extern CFLight *FMesh_CurrentLight;
extern CFVec3 FMesh_vShadowCam;

typedef BOOL FMeshCFMeshInstDrawCallback_t( CFMeshInst *pMeshInst, void *pUserData );

//const int FMESH_BONENAME_LEN		= 31;	// Maximum number of characters in the bone name (excluding the NULL terminator)
//const int FMESH_MESHNAME_LEN		= 15;	// Maximum number of characters in the mesh name (excluding the NULL terminator)
//const int FMESH_TC_COUNT_PER_VTX	= 2;	// Maximum number of texture coordinates per vertex
//const int FMESH_VW_COUNT_PER_VTX	= 4;	// Maximum number of matrix weights per vertex


#define FMESH_MAX_LAYER_SAVE_SLOTS		20		// Number of layer slots for our save state info

#define FMESH_ALLOW_POSTER_BONES		TRUE	// Set to TRUE to allow postering of a mesh to extend to its bones

// if you are looking for FMESH_RESTYPE, see fres.h - Mike


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	The "Normal Sphere" is a magical device.  It was created to conserve memory
//	on some platforms (initially GC).  The normal sphere consists of around 32k normals
//	compressed to 8-bit fixed point that are used to represent all of the normals 
//	in the geometry data (for lighting purposes).  The effect of this is two-fold:
//	normals are compressed to 8-bit, which affects their accuracy; and normals are
//	"rounded" off to one of the 32k existing normals in order that a u16 index can
//	be used to represent the normal.  Where more accuracy is desired (collision
//	data, for instance), the structures can create their own normals (compressed or
//	not) in order to gain more accuracy.
//

#define	FMESH_NORMAL_SPHERE_MAX_INDEX		32800
#define FMESH_NORMAL_SPHERE_NORMAL_NULL		0
#define FMESH_NORMAL_SPHERE_OCT_OFFSET		4096
#define FMESH_NORMAL_SPHERE_NORMAL_UP		32768
#define FMESH_NORMAL_SPHERE_NORMAL_DOWN		32769
#define FMESH_NORMAL_SPHERE_NORMAL_RIGHT	32770
#define FMESH_NORMAL_SPHERE_NORMAL_LEFT		32771
#define FMESH_NORMAL_SPHERE_NORMAL_FORWARD	32772
#define FMESH_NORMAL_SPHERE_NORMAL_BACKWARD	32773
#define FMESH_NORMAL_SPHERE_FIXED_POINT		64

//
// Typical normals *hopefully* can be represented effectively with 8 bits
struct FMesh_CNorm8_t
{
	s8	nx, ny, nz;
	
};

extern FMesh_CNorm8_t *FMesh_avCNormalSphere;
extern void fmesh_InitNormalSphere( void );
extern u16  fmesh_GetNormalSphereIndex( f32 fx, f32 fy, f32 fz );
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


#if !FANG_PRODUCTION_BUILD
	enum
	{
		FMESH_DRAW_COLL_GEO_DISABLED  = 0x00000000,
		FMESH_DRAW_COLL_GEO_ROOT_DOPS = 0x00000001,
		FMESH_DRAW_COLL_GEO_LEAF_DOPS = 0x00000002,
		FMESH_DRAW_COLL_GEO_ALL_DOPS  = 0x00000004,
		FMESH_DRAW_COLL_GEO_LEAF_TRIS = 0x00000008,
	};
	extern u32 FMesh_nDrawCollisionGeoFlags;
#endif
	
// Forward declarations
class CFMeshInst;
struct FkDOP_Tree_t;
struct FkDOP_Geo_t;


enum 
{
	FMESHINST_FLAG_WORLDSPACE			= 0x00000001,		// Mesh is already in world space (For optimization purposes. Not required to be set).
	FMESHINST_FLAG_DONT_DRAW			= 0x00000002,		// Don't draw this mesh (Engine flag setting - Max flag is different)
	FMESHINST_FLAG_POSTER_Y				= 0x00000004,		// Poster around mesh's Y axis (neg-Z axis faces camera)
	FMESHINST_FLAG_STATIC				= 0x00000008,		// Instance bounding sphere has a constant footprint in worldspace (even if postered)
	FMESHINST_FLAG_NOLIGHT_AMBIENT		= 0x00000010,		// Don't include ambient lighting on this mesh
	FMESHINST_FLAG_NOLIGHT_DYNAMIC		= 0x00000020,		// Don't include dynamic lighting on this mesh
	FMESHINST_FLAG_LM					= 0x00000040,		// This mesh inst has a lightmap associated with it
	FMESHINST_FLAG_VERT_RADIOSITY		= 0x00000080,		// This mesh inst has a vertex radiosity lit colorstream associated with it
	FMESHINST_FLAG_LIGHT_PER_PIXEL		= 0x00000100,		// This mesh inst should have per pixel lighting applied
	FMESHINST_FLAG_NOBONES				= 0x00000200,		// Bone matrix palette will not be valid for this mesh inst
	FMESHINST_FLAG_BONEMTX_IN_WS		= 0x00000400,		// Matrices in bone palette are in world space instead of model space
	FMESHINST_FLAG_NOCOLLIDE			= 0x00000800,		// Don't collide with this mesh

	FMESHINST_FLAG_NOANIMLOD			= 0x00001000,		// Don't apply animation lod'ing to this mesh
	FMESHINST_FLAG_NOCLIP				= 0x00004000,		// The triangles in this object are guaranteed to never cross a frustum plane
	FMESHINST_FLAG_POSTER_X				= 0x00008000,		// Poster around mesh's X axis
	FMESHINST_FLAG_POSTER_Z				= 0x00010000,		// Poster around mesh's Z axis
	FMESHINST_FLAG_ANIM_WHEN_PAUSED		= 0x00020000,		// Continue to animate texture flipping & texture coordinate scrolling while floop is paused
	FMESHINST_FLAG_SAMPLE_ANIMLAYER_TICKS = 0x00040000,	// Used internally by fmesh to determine whether CFMeshInst::m_nLastAnimateLayerTicks has been sampled yet
	FMESHINST_FLAG_WORLD_GEO			= 0x00080000,		// Used to indicate that this mesh should be considered as though it is a world mesh

	FMESHINST_FLAG_NOSHADOWLOD			= 0x00100000,		// A shadow cast by this mesh will not have any LOD, it will always be displayed at full detail.
	FMESHINST_FLAG_CAST_SHADOWS			= 0x00200000,		// This mesh inst will cast shadows
	FMESHINST_FLAG_ACCEPT_SHADOWS		= 0x00400000,		// This mesh inst will receive shadows
	FMESHINST_FLAG_TINT					= 0x00800000,		// PASM applied tint to this mesh (as requested by the tools)
	
	FMESHINST_FLAG_FORCE_CLIP			= 0x01000000,		// Force clipping of this geometry.
	FMESHINST_FLAG_SURFACEONLY			= 0x02000000,		// Surface Pass only (no lighting).
	
	#if FANG_PLATFORM_GC
	FMESHINST_FLAG_ENABLECOLORKEY		= 0x04000000,		// Color Key (used for rendertargets since no alpha is available).
	#endif

	FMESHINST_FLAG_NOSHADOW				= 0x08000000,		// Object is too far away to recieve or cast a shadow.

	FMESHINST_FLAG_LMS_READY			= 0x10000000,		// If the object has lightmaps, this flag indicates that the lightmaps are ready to be used (in the cache if streaming is enabled).
	
	FMESHINST_FLAG_POSTER_BONES			= 0x20000000,		// If the object has bones, this flag will poster the bones if the postering flags, X, Y or Z, are set.

	FMESHINST_FLAG_NOLIGHT				= FMESHINST_FLAG_NOLIGHT_AMBIENT | FMESHINST_FLAG_NOLIGHT_DYNAMIC,

	FMESHINST_FLAG_NONE					= 0x00000000
};


enum 
{
	FMESH_BONEFLAG_VOIDBONE			= 0x01,		// This is a bone that has no geometry associated with it (no vertices or triangles)
	FMESH_BONEFLAG_SKINNEDBONE		= 0x02,		// This bone contributes to more than one segment (skinning)

	FMESH_BONEFLAG_COUNT
};


enum 
{
	FMESH_MTLFLAG_NOCOLLIDE				= 0x0001,		// Don't collide with any triangles in this material
	FMESH_MTLFLAG_RENDER_LAST			= 0x0002,		// Render this material last(automatic if materials aren't rendered out of order)
	FMESH_MTLFLAG_ZWRITE_ON				= 0x0004,		// Only relevant for translucent materials.  If set, zbuffer writes will be turned on
	FMESH_MTLFLAG_DO_NOT_TINT			= 0x0008,		// Do not tint this material if mesh tinting is attempted
	FMESH_MTLFLAG_DO_NOT_CAST_SHADOWS	= 0x0010,		// Polys using this material will not cast shadows
	FMESH_MTLFLAG_INVERT_EMISSIVE_MASK	= 0x0020,		// This material will use the invert of the emissive map alpha to mask emissiveness
	FMESH_MTLFLAG_ANGULAR_EMISSIVE		= 0x0040,		// The emissiveness of this material is determined by angle to camera
	FMESH_MTLFLAG_ANGULAR_TRANSLUCENCY	= 0x0080,		// The translucency of this material is determined by angle to camera
	FMESH_MTLFLAG_NO_ALPHA_SCROLL		= 0x0100,		// The alpha layer will not scroll with the color layer

	FMESH_MTLFLAG_NONE					= 0x0000
};



#if FANG_PLATFORM_DX
	typedef struct FDX8Mesh_s FDX8Mesh_t;
#elif FANG_PLATFORM_GC
	typedef struct FGCMesh_s FGCMesh_t;
//ARG - >>>>>
#elif FANG_PLATFORM_PS2
	typedef struct FPS2Mesh_s FPS2Mesh_t;
//ARG - <<<<<
#else
	#error <Fang Error: Undefined mesh type for this platform.>
#endif


#define FMESH_TEXLAYERHANDLE_INVALID	-1		// Invalid texture layer handle

typedef s32 FMeshTexLayerHandle_t;				// Texture layer handle


//
//
struct FMeshMaterial_t
{
	////////////////////////////////////////////////////////////////////////////
	// WARNING:  FMeshMaterial_t is embedded in data exported by PASM
	////////////////////////////////////////////////////////////////////////////

	u32 *pnShLightRegisters;			// Pointer to shader's lighting register array
	u32 *pnShSurfaceRegisters;			// Pointer to shader's surface register array
	u8  nLightShaderIdx;				// Light Shader index for this material
	u8  nSpecularShaderIdx;				// Specular Shader index for this material
	u16 nSurfaceShaderIdx;				// Surface Shader index for this material

	u32 nPartIDMask;					// A mask that has bits set for each mesh part ID that uses it
	
	void *pPlatformData;				// Pointer to the platform specific data for this material
	
	u8 nLODMask;						// A bit mask that identifies all of the LOD that use this material
	u8 nDepthBiasLevel;					// 0=normal, 1=appear in front of 0, 2=appear in front of 1, etc. (negative values not allowed)
	u8 nBaseSTSets;
	u8 nLightMapSTSets;
	u8 anTexLayerIDIndex[4];			// Array of texture layer indices used by this material (255=empty slot) (fill lower elements first)
										//   Indices are into FMesh_t::pTexLayerIDArray[]

	f32 fAffectAngle;					// cos of angle of affect for angular emissive or angular translucency
	s8  anCompAffectNormal[3];			// Compressed affect normal used for determining material angle to camera (mult by 1/64)
	s8  nAffectBoneID;					// Bone ID used to transform the affect angle

	u8 nCompressedRadius;				// The radius of the material verts from the vAverageVertPos in model space represented as a percentage of
										// the mesh's bounding sphere through a unit float compressed to a u8 (multiply by (1/255) * mesh BoundSphere_MS.m_fRadius)
	u8 __PAD;

	u16 nMtlFlags;						// Material flags (see FMESH_MTLFLAG_* for info)

	u32 nDrawKey;						// Key used by the engine to indicate that this material has already been submitted for drawing during the current viewport render

	CFColorRGB MaterialTint;			// Tint to be applied to the material
	CFVec3 vAverageVertPos;				// Average of the position of all verts using this material
	u32 nDLHashKey;						// Hash key used in display list rendering (only valid in game)

	void ChangeEndian( void )
	{
		pnShLightRegisters = (u32 *)fang_ConvertEndian( pnShLightRegisters );
		pnShSurfaceRegisters = (u32 *)fang_ConvertEndian( pnShSurfaceRegisters );
		nLightShaderIdx = fang_ConvertEndian( nLightShaderIdx );
		nSpecularShaderIdx = fang_ConvertEndian( nSpecularShaderIdx );
		nSurfaceShaderIdx = fang_ConvertEndian( nSurfaceShaderIdx );
		
		nPartIDMask = fang_ConvertEndian( nPartIDMask );
		
		pPlatformData = (void *)fang_ConvertEndian( pPlatformData );

		fAffectAngle = fang_ConvertEndian( fAffectAngle );

		nMtlFlags = fang_ConvertEndian( nMtlFlags );
		nDepthBiasLevel = fang_ConvertEndian( nDepthBiasLevel );
		
		nBaseSTSets = fang_ConvertEndian( nBaseSTSets );
		nLightMapSTSets = fang_ConvertEndian( nLightMapSTSets );

		nLODMask = fang_ConvertEndian( nLODMask );

		nDrawKey = fang_ConvertEndian( nDrawKey );
		
		vAverageVertPos.ChangeEndian();
		nDLHashKey = fang_ConvertEndian( nDLHashKey );
		MaterialTint.ChangeEndian();
	}

};


//
//
//
struct FMeshSeg_t
{
	////////////////////////////////////////////////////////////////////////////
	// WARNING:  FMeshSeg_t is embedded in data exported by PASM
	////////////////////////////////////////////////////////////////////////////

	CFSphere BoundSphere_MS;					// Bounds the segment in model space
	u8 nBoneMtxCount;							// Number of simultaneous bone matrices used for vertices within this segment (1=segmented, but not skinned)
	u8 anBoneMtxIndex[FDATA_VW_COUNT_PER_VTX];	// Index into object instance's bone matrix palette (255=none)

	void ChangeEndian( void ) 
	{
		BoundSphere_MS.ChangeEndian();
	}
	
};


//
//
//
struct FMeshSkeleton_t
{
	////////////////////////////////////////////////////////////////////////////
	// WARNING:  FMeshSkeleton_t is embedded in data exported by PASM
	////////////////////////////////////////////////////////////////////////////

	u8 nParentBoneIndex;					// Bone index of this bone's parent (255 = no parent)
	u8 nChildBoneCount;						// Number of children attached to this bone (0 = no children)
	u8 nChildArrayStartIndex;				// Index into the array of bone indices (FMesh_t::pnSkeletonIndexArray) of where this bone's child index list begins
	
};


//
//
// Dynamic Lights Attached To This Mesh:
struct FMeshLight_t
{
	////////////////////////////////////////////////////////////////////////////
	// WARNING:  FMeshLight_t is embedded in data exported by PASM
	////////////////////////////////////////////////////////////////////////////

	FLightInit_t LightInit;					// Note that Sphere_WS and Dir_WS are really in mesh-xfm space
	
};


//
//
//
struct FMeshBone_t
{
	////////////////////////////////////////////////////////////////////////////
	// WARNING:  FMeshBone_t is embedded in data exported by PASM
	////////////////////////////////////////////////////////////////////////////

	char szName[FDATA_BONENAME_LEN+1];	// ASCII name of this bone
	CFMtx43A AtRestBoneToModelMtx;		// When an at-rest bone-space vertex is transformed by this, it moves the vertex into model space
	CFMtx43A AtRestModelToBoneMtx;		// When a model-space vertex is transformed by this, it moves the vertex into this bone's at-rest space
	CFMtx43A AtRestParentToBoneMtx;		// Describes this bone's frame of reference relative to its parent's frame of reference
	CFMtx43A AtRestBoneToParentMtx;		// Inverse of AtRestParentToBoneMtx
	CFSphere SegmentedBoundSphere_BS;	// Segmented bones only: The bounding sphere of the segment in bone space (skinned and void bones: x=y=z=r=0)
	FMeshSkeleton_t Skeleton;//=3 bytes // Describes the skeleton hierarchy for this bone
	u8 nFlags; 							// Bone flags (see FMESH_BONEFLAG_* for info)
	u8 nPartID;							// Part ID for this bone.  Used to detach parts. Default = 0.
	u8 __nPad[3];

	void ChangeEndian( void ) 
	{
		nFlags = fang_ConvertEndian( nFlags );
		AtRestBoneToModelMtx.ChangeEndian();
		AtRestModelToBoneMtx.ChangeEndian();
		AtRestParentToBoneMtx.ChangeEndian();
		AtRestBoneToParentMtx.ChangeEndian();
		SegmentedBoundSphere_BS.ChangeEndian();
		nPartID = fang_ConvertEndian( nPartID );
	}
	
};


enum 
{
	FMESH_TEXLAYERIDFLAG_USE_ST_INFO		= 0x01,	// Create auto-scroll data from FMeshTexLayerID_t
	FMESH_TEXLAYERIDFLAG_USE_FLIP_INFO		= 0x02,	// Create auto-flip data from FMeshTexLayerID_t

	FMESH_TEXLAYERIDFLAG_BEGIN_SCROLLING	= 0x04,	// Immediately begin scrolling TCs when the mesh resource is instantiated
	FMESH_TEXLAYERIDFLAG_BEGIN_FLIPPING		= 0x08,	// Immediately begin flipping texture pages when the mesh resource is instantiated
	FMESH_TEXLAYERIDFLAG_BEGIN_ROTATING		= 0x10,	// Immediately begin rotating STs when the mesh resource is instantiated

	FMESH_TEXLAYERIDFLAG_NONE				= 0x00
};


//
//
//
struct FMeshTexLayerID_t
{
	////////////////////////////////////////////////////////////////////////////
	// WARNING:  FMeshTexLayerID_t is embedded in data exported by PASM
	////////////////////////////////////////////////////////////////////////////

	u8 nTexLayerID;						// Texture layer ID associated with this entry
	u8 nFlags;							// See FMESH_TEXLAYERIDFLAG_* for info
	
	u8 nFlipPageCount;					// Number of texture flip pages in the palette
	u8 nFramesPerFlip;					// Number of swap-buffer frames per flip
	CFTexInst **apFlipPalette;			// Pointer to flip palette (array of CFTexInst pointers). Number of palette entries is nFlipPageCount.

	CFVec2 m_ScrollSTPerSec;			// Describes the direction and rate at which the texture coordinates are scrolled
	f32 m_fUVDegreeRotationPerSec;		// Describes the rotation degrees and direction (negative is counter-clockwise) per second
	u8  m_nCompressedUVRotAnchor[2];	// UV of the coord around which the UV's rotate (unit values compressed to 0 to 255, no negative);
	u8  __PAD[2];

	void ChangeEndian( void )
	{
		nTexLayerID = fang_ConvertEndian( nTexLayerID );
		nFlags = fang_ConvertEndian( nFlags );
		nFlipPageCount = fang_ConvertEndian( nFlipPageCount );
		nFramesPerFlip = fang_ConvertEndian( nFramesPerFlip );
		m_fUVDegreeRotationPerSec = fang_ConvertEndian( m_fUVDegreeRotationPerSec );
		apFlipPalette = (CFTexInst **)fang_ConvertEndian( apFlipPalette );
		m_ScrollSTPerSec.ChangeEndian();
	}
	
};


enum 
{
	FMESH_FLAGS_VOLUME_MESH		= 0x0001,	// Mesh will not move or re-orient after loading
	
	FMESH_FLAGS_NONE			= 0x0000
};


//
// FMesh_t - This is the base struct that holds the mesh geometry
//
struct FMesh_t
{
	////////////////////////////////////////////////////////////////////////////
	// WARNING:  FMesh_t is embedded in data exported by PASM
	////////////////////////////////////////////////////////////////////////////

	char szName[FDATA_MESHNAME_LEN+1];	// ASCIIZ name of this mesh
	CFSphere BoundSphere_MS;			// Bounds the mesh in model space (might not be valid for skinned models)
	CFVec3 vBoundBoxMin_MS;
	CFVec3 vBoundBoxMax_MS;

	u16 nFlags;
	u16 nMeshCollMask;					// Or'ing of all MasterCollMasks in the collision trees
	u8 nUsedBoneCount;					// Number of non-void bones which are located at the beginning of pBoneArray
	u8 nRootBoneIndex;					// The index into the FMeshBone_t array of the root bone (255 if this mesh has no bones)
	u8 nBoneCount;						// Number of bones in this model (0 if none)
	u8 nSegCount;						// Number of segments in this object
	u8 nTexLayerIDCount;				// Number of entries in pTexLayerIDArray
	u8 nTexLayerIDCount_ST;				// Number of entries in pTexLayerIDArray that have their FMESH_TEXLAYERIDFLAG_USE_ST_INFO flag set
	u8 nTexLayerIDCount_Flip;			// Number of entries in pTexLayerIDArray that have their FMESH_TEXLAYERIDFLAG_USE_FLIP_INFO flag set
	u8 nLightCount;						// Number of lights attached to this mesh
	u8 nMaterialCount;					// Number of materials in the material array (aMtl)
	u8 nCollTreeCount;					// Number of elements in the collision tree array
	u8 nLODCount;						// Number of LOD meshes for this object
	u8 nShadowLODBias;					// Bias added to the current LOD for generating shadows

	f32 afLODDistance[FDATA_MAX_LOD_MESH_COUNT];

	FMeshSeg_t		*aSeg;				// Base of segment array with public information
	FMeshBone_t		*pBoneArray;		// Pointer to bone array (number of elements is nBoneCount) (NULL if nBoneCount is 0)
	FMeshLight_t	*pLightArray;		// Pointer to light array (number of elements is nLightCount) (NULL if nLightCount is 0)
	u8 				*pnSkeletonIndexArray;// Pointer to the skeleton index array used by FMeshBone_t::Skelton.nChildArrayStartIndex
	FMeshMaterial_t	*aMtl;				// Pointer to the array of materials

	FkDOP_Tree_t 	*paCollTree;		// Pointer to an array of the mesh collision data structures (1 per segment)
	
	FMeshTexLayerID_t *pTexLayerIDArray;// Texture layer ID array. Each slot matches up with a corresponding slot in each instance of this mesh.

	#if FANG_PLATFORM_PS2
		FPS2Mesh_t *pMeshIS;			// Pointer to implementation-specific object data
	#elif FANG_PLATFORM_DX
		FDX8Mesh_t *pMeshIS;			// Pointer to implementation-specific object data
	#elif FANG_PLATFORM_GC
		FGCMesh_t *pMeshIS;				// Pointer to implementation-specific object data
	#else
		#error <Fang Error: Undefined mesh type for this platform.>
	#endif

	void ChangeEndian( void )
	{
		nFlags = fang_ConvertEndian( nFlags );

		nUsedBoneCount = fang_ConvertEndian( nUsedBoneCount );
		nRootBoneIndex = fang_ConvertEndian( nRootBoneIndex );
		nBoneCount = fang_ConvertEndian( nBoneCount );
		nSegCount = fang_ConvertEndian( nSegCount );
		nTexLayerIDCount = fang_ConvertEndian( nTexLayerIDCount );
		nTexLayerIDCount_ST = fang_ConvertEndian( nTexLayerIDCount_ST );
		nTexLayerIDCount_Flip = fang_ConvertEndian( nTexLayerIDCount_Flip );
		nLightCount = fang_ConvertEndian( nLightCount );
		nMaterialCount = fang_ConvertEndian( nMaterialCount );
		nCollTreeCount = fang_ConvertEndian( nCollTreeCount );
		nLODCount = fang_ConvertEndian( nLODCount );
		nShadowLODBias = fang_ConvertEndian( nShadowLODBias );

		nMeshCollMask = fang_ConvertEndian( nMeshCollMask );

		BoundSphere_MS.ChangeEndian();
		vBoundBoxMin_MS.ChangeEndian();
		vBoundBoxMax_MS.ChangeEndian();
		nLightCount = fang_ConvertEndian( nLightCount );
		aSeg = (FMeshSeg_t *)fang_ConvertEndian( aSeg );
		pBoneArray = (FMeshBone_t *)fang_ConvertEndian( pBoneArray );
		pLightArray = (FMeshLight_t *)fang_ConvertEndian( pLightArray );
		pnSkeletonIndexArray = (u8 *)fang_ConvertEndian( pnSkeletonIndexArray );
		aMtl = (FMeshMaterial_t *)fang_ConvertEndian( aMtl );
		pTexLayerIDArray = (FMeshTexLayerID_t *)fang_ConvertEndian( pTexLayerIDArray );
		
		paCollTree = (FkDOP_Tree_t *)fang_ConvertEndian( paCollTree );

		u32 i;
		for ( i = 0; i < FDATA_MAX_LOD_MESH_COUNT; i++ )
		{
			afLODDistance[i] = fang_ConvertEndian( afLODDistance[i] );
		}

		#if FANG_PLATFORM_PS2
			pMeshIS = (FPS2Mesh_t *)fang_ConvertEndian( pMeshIS );
		#elif FANG_PLATFORM_DX
			pMeshIS = (FDX8Mesh_t *)fang_ConvertEndian( pMeshIS );
		#elif FANG_PLATFORM_GC
			pMeshIS = (FGCMesh_t *)fang_ConvertEndian( pMeshIS );
		#endif
	}
};






//----------------------------------------------------------------------------------------------------------

//
//
//
struct FMeshShadow_t
{
	CFVec3 LightUnitDir;			// The unit direction away from the light that's casting the shadow
	f32 fUnitStrength;				// Shadow's strength: 0.0f=no shadow, 1.0f=maximum darkening
	f32 fNearPlaneZ;				// Nearest Z value that can be projected onto the shadowmap
	f32 fFarPlaneZ;					// Farthest Z value that can be projected onto the shadowmap
	f32 fFullStrengthZ;				// Z value that specifies the depth at which the shadow is darkest

	FTexDef_t *pTexDef;				// The shadow texture (must be square)

	CFMtx43A WorldToLightMtx;		// Transforms world space points to light space
	CFMtx43A BlockerProjMtx;		// Projection matrix used to map blocker's 3D light space coordinates onto the render target
	CFMtx43A ReceiverTCMtx;			// Texture coordinate matrix used to map receiver's 3D light space coordinates onto the render target
	
};





//---------------------------------------------------------------------------------------------------------------------
// fmesh_ModuleStartup()
// fmesh_ModuleShutdown()
//
// Starts up and shuts down this module, respectively.
// Called by the Fang initialization system.
// Returns TRUE if successful, or FALSE if the module could not successfully initialized.
//
// pVtxCacheBuf describes a memory region allocated by the application and given to the Mesh module
// to manage and use for software-transformed vertices. The more memory given to this buffer, the
// faster rendering will be when software transformations are required. pVtxCacheBuf must be
// specified or Startup will fail.
//
// pDecalBuf describes a memory region allocated by the application and given to the Mesh module
// to manage and use for dynamically-applied decals. The more memory allocated, the more decals
// can coexist on a global basis. The number of decals can also be throttled by fmesh_SetMaxDecals().
// If the app doesn't want decals, it may set pDecalBuf to NULL.
extern BOOL fmesh_ModuleStartup( void );
extern void fmesh_ModuleShutdown( void );




//---------------------------------------------------------------------------------------------------------------------
// fmesh_Draw()
//
// Draws the mesh using the current rendering state and viewport. The mesh's vertices are
// in modelspace and are transformed by the current camera and model transformation
// matrices (see fxfm module).
//
// TBD: This stuff has changed:
// nCrossesPlanesMask indicates which frustum planes the application suspects the mesh to
// intersect, as follows:
//   bit 0: If set, the mesh potentially crosses the right frustum plane
//   bit 1: If set, the mesh potentially crosses the left frustum plane
//   bit 2: If set, the mesh potentially crosses the bottom frustum plane
//   bit 3: If set, the mesh potentially crosses the top frustum plane
//   bit 4: If set, the mesh potentially crosses the back frustum plane
//   bit 5: If set, the mesh potentially crosses the front frustum plane
//
// If nCrossesPlanesMask is 0x3f, all planes will be checked for clipping.
// If nCrossesPlanesMask is 0, no planes will be checked for clipping.
// If nCrossesPlanesMask is -1, this is a special value indicating that the
// mesh is completely outside the frustum.
//
// Upon return, fmesh_Draw() returns a value representing a new nCrossesPlanesMask.
// This value is modified from the input nCrossesPlanesMask and represents new
// plane-crossing information that this function determined based on its own frustum
// intersection tests. This is useful for hierarchical testing.



extern void fmesh_Ambient_Set( float fRed, float fGreen, float fBlue, float fIntensity=1.0f );
extern void fmesh_Ambient_Set( const CFColorRGB *pColorRGB, float fIntensity=1.0f );
extern void fmesh_Ambient_Get( float *pfRed, float *pfGreen, float *pfBlue, float *pfIntensity );
extern void fmesh_Ambient_Get( float *pfRed, float *pfGreen, float *pfBlue );
extern void fmesh_Ambient_Get( CFColorRGB *pColorRGB, float *pfIntensity );
extern void fmesh_Ambient_Get( CFColorRGB *pColorRGB );

extern u32 fmesh_GetLightCount( void );
extern void fmesh_ResetLightList( void );




//-------------------------------------------------------------------------------------------------------------------
// These variables are private to the fmesh system, but need to be shared with the
// platform-specific components of the fmesh system:

extern CFColorMotif FMesh_AmbientMotif;
extern CFColorRGB FMesh_AmbientRGB;
extern f32 FMesh_fAmbientIntensity;

extern u32 FMesh_nMaxMeshLights;

//-------------------------------------------------------------------------------------------------------------------



//-------------------------------------------------------------------------------------------------------------------
// FMeshInit_t:
//-------------------------------------------------------------------------------------------------------------------
struct FMeshInit_t 
{
	FMesh_t *pMesh;						// Pointer to mesh associated with this instance
	u32 nFlags;							// Instance flags (see FMESHINST_FLAG_* for info)
	f32 fCullDist;						// Don't draw this mesh if its distance to the camera squared is greater than this value (FMATH_MAX_FLOAT=don't distance cull)
	CFMtx43A Mtx;						// Orientation matrix (uniform scale may be embedded)
};




//-------------------------------------------------------------------------------------------------------------------
// CFMeshAnimTC:
//-------------------------------------------------------------------------------------------------------------------
FCLASS_NOALIGN_PREFIX class CFMeshAnimTC 
{
	public:
		enum
		{
			SCROLL_APPLIED	= 0x01,
			ROTATE_APPLIED	= 0x02,
		};
		u8 m_nFlags;						// TRUE=apply the delta texture coordinates, FALSE=don't apply
		u8 m_nFMeshLayerIndex;				// Index into the FMesh_t's array that represents this layer
		u8 __PAD;
		BOOL8 m_bApplyToShader;				// TRUE=override the shader's TexDef with this one, FALSE=use shader's TexDef
		f32 m_fRotationRad;					// Current position of the UV rotation in compressed radians
		f32 m_fUVRotationSpeed;				// UV Rotation speed in rads per second
		CFVec2 m_ScrollSTPerSec;			// Describes the direction and rate at which OffsetST changes
		CFVec2 m_ScrollPos;					// Describes the current UV scroll pos

	FCLASS_STACKMEM_NOALIGN( CFMeshAnimTC );
} FCLASS_NOALIGN_SUFFIX;




//-------------------------------------------------------------------------------------------------------------------
// CFMeshTexFlip:
//-------------------------------------------------------------------------------------------------------------------
FCLASS_NOALIGN_PREFIX class CFMeshTexFlip 
{
	public:
		enum {
			FLAG_APPLY_TO_SHADER	= 0x01,	// TRUE=override the shader's TexDef with this one, FALSE=use shader's TexDef
			FLAG_ANIMATE_FLIP		= 0x02,	// TRUE=turn on the texture flipping, FALSE=turn off the texture flipping
			FLAG_CLAMP_AND_STOP		= 0x04,	// TRUE=when the first (backward) or last (forward) page is reached, stop animating

			FLAG_NONE				= 0x00
		};

		u8 m_nUnused;						// Padding
		u8 m_nFlags;						// See FLAG_* for info
		u8 m_nFlipPageCount;				// Number of texture flip pages in the palette
		u8 m_nCurrentPageIndex;				// Current page index

		f32 m_fCurrentPageIndex;			// Current page index in floating point
		f32 m_fPagesPerSecond;				// Flip rate: pages per second
		CFTexInst **m_apFlipPalette;		// Pointer to flip palette (array of CFTexInst pointers). Number of palette entries is m_nFlipPageCount.
		CFTexInst *m_pTexInst;				// Current TexDef

	FCLASS_STACKMEM_NOALIGN( CFMeshTexFlip );
} FCLASS_NOALIGN_SUFFIX;


//-------------------------------------------------------------------------------------------------------------------
// FMeshInstTexLayerID_t:
//-------------------------------------------------------------------------------------------------------------------
struct FMeshInstTexLayerID_t 
{
	u8 nLayerAlpha;						// Layer alpha can only be set on non-base layers.  The base layer assumes the overall alpha of the object
	u8 __PAD[3];

	CFMeshAnimTC  *pAnimTC;				// Points to this ID's TC animation state info (NULL=none)
	CFMeshTexFlip *pTexFlip;			// Points to this ID's texture flip state info (NULL=none)
	CFColorMotif  *pEmissiveMotif;		// Points to this ID's emissive motive state (NULL=none)
};


//-------------------------------------------------------------------------------------------------------------------
// FMesh_ListLight_t:
//-------------------------------------------------------------------------------------------------------------------
struct FMesh_RenderLight_t
{
	CFLight *pLight;
	f32		fLightWeight;	
};


//-------------------------------------------------------------------------------------------------------------------
// Private Collision Stuff:
//-------------------------------------------------------------------------------------------------------------------
typedef struct FMeshCollPacket_s FMeshCollPacket_t;
typedef struct FMeshCollSubset_s FMeshCollSubset_t;



//--------------------------------------------------------------------------------------------------------------------
// Cull Direction:

typedef enum 
{
	FMESH_CULLDIR_CW = 0,		// Cull triangles with a clockwise screenspace vertex specification
	FMESH_CULLDIR_CCW,			// Cull triangles with a counter-clockwise screenspace vertex specification
	FMESH_CULLDIR_NONE,			// Don't cull any triangles

	FMESH_CULLDIR_COUNT
} FMeshCullDir_e;


//-------------------------------------------------------------------------------------------------------------------
// CFMeshInst:
//-------------------------------------------------------------------------------------------------------------------
FCLASS_ALIGN_PREFIX class CFMeshInst 
{
	public:
		// Construction / Destruction:
		CFMeshInst( void );
		~CFMeshInst( void );

		// Interface:
		void Init( FMesh_t *pMesh, BOOL bAnimWhenPaused = FALSE, u32 nDrawnPartsMask = 0xffffffff, u32 nCollisionPartsMask = 0xffffffff, BOOL bNoMemAlloc = FALSE );
		void Init( FMeshInit_t *pMeshInit, u32 nDrawnPartsMask = 0xffffffff, u32 nCollisionPartsMask = 0xffffffff, BOOL bNoMemAlloc = FALSE );

		// Lighting data interface
		void  SetLightmaps( cchar **pLightMapNameArray, const u16 *anLightMapMotif, u32 nArrayCount );
		void  SetColorStreams( u32 nStreamCount, ColorStream_t *paStreams );
		u8    GetColorStreamCount( void ) { return m_nColorStreamCount; }
		void* GetColorStreams( void ) { return m_papColorStreams; }

		// Shadow Intensity Adjustment interface (Note than shadows have a natural max intensity of 50% intensity, so
		// if you pass in 2.0, the shadow will appear at full black.  1.0 will use the usual shadow intensity.)
		void SetShadowIntensityAdjust( f32 fIntensity ) { FASSERT( fIntensity >= 0.f && fIntensity <= 2.f ); m_fShadowIntensity = fIntensity; }
		f32  GetShadowIntensityAdjust( void ) { return m_fShadowIntensity; }

		// Mesh Part interface
		FINLINE u32 Parts_GetCreatedPartsMask( void ) const { return m_nCreatedPartsMask; }
		FINLINE u32 Parts_GetCreatedBoneCount( void ) const { return m_nCreatedBonesCount; }
		FINLINE u32 Parts_ResetDrawnPartsMask( void ) { m_nDrawnPartsMask = m_nCreatedPartsMask; return m_nDrawnPartsMask; }
		FINLINE u32 Parts_GetDrawnPartsMask( void ) const { return m_nDrawnPartsMask; }
		FINLINE u32 Parts_GetDrawnBoneCount( void ) const { return m_nDrawnBonesCount; }
		FINLINE u32 Parts_SetDrawnPartsMask( u32 nNewPartsMask ) { return m_nDrawnPartsMask = nNewPartsMask & m_nCreatedPartsMask; }
		FINLINE u32 Parts_GetCollPartsMask( void ) const { return m_nCollPartsMask; }
		FINLINE u32 Parts_SetCollPartsMask( u32 nNewPartsMask ) { return m_nCollPartsMask = nNewPartsMask & m_nCreatedPartsMask; }
		
		const void *GetSaveData( u32 *pnSizeInBytes );
		void SetSaveData( const void *pData );

		FINLINE u32  GetFrameLastDrawn( void ) const { return m_nFrameLastDrawn; }
		FINLINE void SetFrameLastDrawn( u32 nNewValue ) { m_nFrameLastDrawn = nNewValue; }
		FINLINE void InvalidateFrameLastDrawn( void ) { m_nFrameLastDrawn = FVid_nFrameCounter-10; }
		FINLINE void FlagAsDrawnThisFrame( void ) { m_nFrameLastDrawn = FVid_nFrameCounter; }
		FINLINE BOOL WasDrawnThisFrame( void ) const { return (m_nFrameLastDrawn == FVid_nFrameCounter); }
		FINLINE BOOL WasDrawnLastFrame( void ) const { return (m_nFrameLastDrawn == (FVid_nFrameCounter-1)); }

		FINLINE u32  GetPixelRadiusLastDrawn( void ) const { return m_nPixelRadiusLastRender; }
		FINLINE void SetPixelRadiusLastDrawn( u32 nNewSize ) { m_nPixelRadiusLastRender = nNewSize; }

		FINLINE void SetCullDirection( FMeshCullDir_e nCullDirection ) { m_nCullDirection = nCullDirection; }
		
		// NOTE:  SetMeshAlpha will only affect materials whose shader is oBASE or cBASE.  It will have no effect on other shader types.
		FINLINE void SetMeshAlpha( f32 fUnitAlpha ) { FASSERT( fUnitAlpha >= 0.f && fUnitAlpha <= 1.f); m_MeshRGBA.fAlpha = fUnitAlpha; }
		FINLINE void SetMeshTint( f32 fRed, f32 fGreen, f32 fBlue ) 
		{ 
			FASSERT( fRed >= 0.f && fRed <= 1.f && fGreen >= 0.f && fGreen <= 1.f && fBlue >= 0.f && fBlue <= 1.f ); 
			m_MeshRGBA.fRed = fRed; 
			m_MeshRGBA.fGreen = fGreen; 
			m_MeshRGBA.fBlue = fBlue; 
		}
		FINLINE CFColorRGBA* GetMeshTintAndAlpha( void ) { return &m_MeshRGBA; }

		FINLINE CFMtx43A **GetBoneMtxPalette( void ) const { return m_apBoneMtxPalette; }
		FINLINE void SetBoneMtxPalette( CFMtx43A **ppBoneMtxPalette ) { m_apBoneMtxPalette = ppBoneMtxPalette; }
		s32 FindBone( cchar *pszBoneName ) const;
		void AtRestMtxPalette( void );

		FMeshTexLayerHandle_t GetTexLayerHandle( u32 nTexLayerID ) const;
		FINLINE FMeshInstTexLayerID_t *GetTexLayerID( FMeshTexLayerHandle_t hTexLayer ) const;
		FINLINE CFMeshAnimTC *GetAnimTC( FMeshTexLayerHandle_t hTexLayer ) const;
		FINLINE CFMeshTexFlip *GetTexFlip( FMeshTexLayerHandle_t hTexLayer ) const;

		FINLINE void AnimTC_ApplyToShader( FMeshTexLayerHandle_t hTexLayer, BOOL bEnable ) { GetAnimTC(hTexLayer)->m_bApplyToShader = !!bEnable; }
		FINLINE BOOL AnimTC_IsAppliedToShader( FMeshTexLayerHandle_t hTexLayer ) const { return GetAnimTC(hTexLayer)->m_bApplyToShader; }
		FINLINE void AnimTC_AnimateScroll( FMeshTexLayerHandle_t hTexLayer, BOOL bEnable ) { FMATH_WRITEBIT( bEnable, GetAnimTC(hTexLayer)->m_nFlags, CFMeshAnimTC::SCROLL_APPLIED ); }
		FINLINE BOOL AnimTC_IsScrollAnimating( FMeshTexLayerHandle_t hTexLayer ) const { return (GetAnimTC(hTexLayer)->m_nFlags & CFMeshAnimTC::SCROLL_APPLIED); }
		FINLINE void AnimTC_SetScrollRate( FMeshTexLayerHandle_t hTexLayer, const CFVec2 &rDeltaSTPerSec ) { GetAnimTC(hTexLayer)->m_ScrollSTPerSec = rDeltaSTPerSec; }
		FINLINE CFVec2 AnimTC_GetScrollRate( FMeshTexLayerHandle_t hTexLayer ) const { return GetAnimTC(hTexLayer)->m_ScrollSTPerSec; }
		FINLINE void AnimTC_SetScrollST( FMeshTexLayerHandle_t hTexLayer, const CFVec2 &rNewST ) { GetAnimTC(hTexLayer)->m_ScrollPos = rNewST; NormalizeScrollST( GetAnimTC(hTexLayer) ); }
		FINLINE CFVec2 AnimTC_GetScrollST( FMeshTexLayerHandle_t hTexLayer ) const {return GetAnimTC(hTexLayer)->m_ScrollPos; }
		FINLINE void AnimTC_AnimateRotation( FMeshTexLayerHandle_t hTexLayer, BOOL bEnable ) { GetAnimTC(hTexLayer)->m_nFlags |= CFMeshAnimTC::ROTATE_APPLIED; }
		FINLINE BOOL AnimTC_IsRotationAnimating( FMeshTexLayerHandle_t hTexLayer ) const { return (GetAnimTC(hTexLayer)->m_nFlags & CFMeshAnimTC::ROTATE_APPLIED); }
		FINLINE void AnimTC_SetRotationRate( FMeshTexLayerHandle_t hTexLayer, const f32 &fRadiansPerSec ) { GetAnimTC(hTexLayer)->m_fUVRotationSpeed = fRadiansPerSec; };
		FINLINE f32  AnimTC_GetRotationRate( FMeshTexLayerHandle_t hTexLayer ) const { return GetAnimTC(hTexLayer)->m_fUVRotationSpeed; };

		FINLINE void TexFlip_ApplyToShader( FMeshTexLayerHandle_t hTexLayer, BOOL bEnable ) { FMATH_WRITEBIT( bEnable, GetTexFlip(hTexLayer)->m_nFlags, CFMeshTexFlip::FLAG_APPLY_TO_SHADER ); }
		FINLINE BOOL TexFlip_IsAppliedToShader( FMeshTexLayerHandle_t hTexLayer ) const { return GetTexFlip(hTexLayer)->m_nFlags & CFMeshTexFlip::FLAG_APPLY_TO_SHADER; }
		FINLINE void TexFlip_AnimateFlip( FMeshTexLayerHandle_t hTexLayer, BOOL bEnable ) { FMATH_WRITEBIT( bEnable, GetTexFlip(hTexLayer)->m_nFlags, CFMeshTexFlip::FLAG_ANIMATE_FLIP ); }
		FINLINE BOOL TexFlip_IsFlipAnimating( FMeshTexLayerHandle_t hTexLayer ) const { return GetTexFlip(hTexLayer)->m_nFlags & CFMeshTexFlip::FLAG_ANIMATE_FLIP; }
		FINLINE void TexFlip_ClampAndStop( FMeshTexLayerHandle_t hTexLayer, BOOL bClampAndStop ) { FMATH_WRITEBIT( bClampAndStop, GetTexFlip(hTexLayer)->m_nFlags, CFMeshTexFlip::FLAG_CLAMP_AND_STOP ); }
		FINLINE BOOL TexFlip_IsClampAndStop( FMeshTexLayerHandle_t hTexLayer ) const { return GetTexFlip(hTexLayer)->m_nFlags & CFMeshTexFlip::FLAG_CLAMP_AND_STOP; }
		FINLINE void TexFlip_SetFlipRate( FMeshTexLayerHandle_t hTexLayer, f32 fPagesPerSecond ) { FASSERT( fPagesPerSecond > 0.0f ); GetTexFlip(hTexLayer)->m_fPagesPerSecond = fPagesPerSecond; }
		FINLINE f32  TexFlip_GetFlipRate( FMeshTexLayerHandle_t hTexLayer ) const { return GetTexFlip(hTexLayer)->m_fPagesPerSecond; }
		FINLINE void TexFlip_SetFlipPage( FMeshTexLayerHandle_t hTexLayer, u32 nFlipPageIndex );
		FINLINE u32  TexFlip_GetFlipPage( FMeshTexLayerHandle_t hTexLayer ) const;
		FINLINE u32  TexFlip_GetFlipPageCount( FMeshTexLayerHandle_t hTexLayer ) const;
		FINLINE void TexFlip_SetFlipPalette( FMeshTexLayerHandle_t hTexLayer, CFTexInst **apFlipPalette, u32 nPaletteEntryCount );
		FINLINE void TexFlip_GetFlipPalette( FMeshTexLayerHandle_t hTexLayer, CFTexInst ***papFlipPalette, u32 *pnPaletteEntryCount ) const;
		FINLINE void TexFlip_SetSingleTexture( FMeshTexLayerHandle_t hTexLayer, CFTexInst *pTexInst );
		FINLINE CFTexInst *TexFlip_GetSingleTexture( FMeshTexLayerHandle_t hTexLayer ) const;

		FINLINE CFColorRGB* LayerEmissive_Get( FMeshTexLayerHandle_t hTexLayer ) const;
		FINLINE void LayerEmissive_Set( FMeshTexLayerHandle_t hTexLayer, f32 fRed, f32 fGreen, f32 fBlue );

		// This call can be used to set the translucency of a secondary layer for
		// any two texture material.  It can also be used to set the alpha or intensity
		// of a translucent material.
		FINLINE f32  LayerAlpha_Get( FMeshTexLayerHandle_t hTexLayer ) const;
		FINLINE void LayerAlpha_Set( FMeshTexLayerHandle_t hTexLayer, f32 fAlpha );

		FINLINE void OverrideZMask(CFTexInst *pTex);

		FINLINE void GetBoneSphere_WS( u32 nBoneIdx, CFSphere *pResult );

		#if FANG_PLATFORM_GC
			FINLINE u16 SkinVB_GetStartingIndex( void ) { return m_nStartingSkinVertexBufferIdx; }
		#endif

		void ResetLightList( void );
		BOOL ConsiderLightForRender( CFLight *pLight, CFSphere *pBoundingSphere_WS = NULL );

		// Given the segment index, this function returns
		// a FkDOP_Geo_t that represents the volume contained by the kDOP.
		// This call is costly so it should be done as seldom as possible.
		//
		// This geo comes from an internal pool managed by the kDOP code.  The
		// caller should not attempt to free or modify the memory pointer to by
		// the FkDOP_Geo_t*. After the caller is done using the geo, it must be 
		// freed using the FreeGeo call, below.
		const FkDOP_Geo_t* GetkDOPGeo( u32 nBoneIdx );
		void ReleasekDOPGeo( const FkDOP_Geo_t *pGeo );
		BOOL ConvertMeshCollTokDOP( CFVec3A *pCollision_WS, CFVec3A *pNormalToOrigin_WS, s32 *pnBoneIdx, CFVec3A *pvResult_WS, s32 *pnkDOPNormalIdx );
		void kDOP_Draw( CFColorRGBA *pColor = NULL, s32 nBoneIdx = -1, BOOL bKeepAxisAligned = TRUE );
		BOOL kDOP_TransformToWS( s32 nSegmentIdx = -1 );

		// Public Draw related functions		
		void AnimateLayers( void );
//ARG - >>>>>
#if	FANG_PLATFORM_PS2
		void PS2_AddShaderLights( FMeshMaterial_t *pMaterial );
#else
//ARG - <<<<<
		void AddShaderLights( void );
#endif	//ARG
		void CacheMeshData( u32 nPriority );
		FViewportPlanesMask_t DrawPrep( FViewportPlanesMask_t nCrossesPlanesMask, BOOL bUnconditional = FALSE, BOOL bDrawFull = TRUE );
		BOOL DrawMaterialLight_P( FViewportPlanesMask_t nCrossesPlanesMask, FMeshMaterial_t *pMaterial, u32 nLODIndex );
		void DrawMaterialSurface_P( FViewportPlanesMask_t nCrossesPlanesMask, FMeshMaterial_t *pMaterial, u32 nLODIndex );
		void DrawMaterialSpecular_P( FViewportPlanesMask_t nCrossesPlanesMask, FMeshMaterial_t *pMaterial, u32 nLODIndex );
		void DrawMaterialTranslucency_P( FViewportPlanesMask_t nCrossesPlanesMask, FMeshMaterial_t *pMaterial, u32 nLODIndex );
		void DrawShadow_P( FViewportPlanesMask_t nCrossesPlanesMask, FMeshMaterial_t *pMaterial, u32 nLODIndex );
		FViewportPlanesMask_t Draw( FViewportPlanesMask_t nCrossesPlanesMask, BOOL bUnconditional = FALSE )
		{
			return DrawPrep( nCrossesPlanesMask, bUnconditional, TRUE );
		}

		#if !FANG_PRODUCTION_BUILD
			void DrawBoneInfo( void );
		#endif

		u32 PushXfm( void ) const;
		FINLINE void PopXfm( u32 nNumPops ) { CFXfm::PopModel( nNumPops ); }

		BOOL CollideWithMeshTris( CFCollInfo *pCollInfo );

		// IsPointInMesh() tests a point and an optional radius for containment in the mesh.
		// Returns -1 if it was unable to perform the test, 0 if the test failed and 1 if the test succeeded.
		// Keep in mind that this test is only accurate if the mesh is convex and there is
		// no pretest in this function to determine this.
		s32 IsPointInMesh( CFVec3A *pPoint, f32 fRadius = 0.f );

		FINLINE void SetPreDrawCallback( FMeshCFMeshInstDrawCallback_t *pFcnCallback, void *pUserData ) { m_pPreDrawCallback = pFcnCallback; m_pPreDrawData = pUserData; }
		FINLINE void SetPostDrawCallback( FMeshCFMeshInstDrawCallback_t *pFcnCallback, void *pUserData ) { m_pPostDrawCallback = pFcnCallback; m_pPostDrawData = pUserData; }

	protected:
		BOOL AllocInstArrays( BOOL bNoMemAlloc );
		static FINLINE void NormalizeScrollST( CFMeshAnimTC *pAnimTC );
		static void NormalizeScrollValueUp( f32 *pfValue );
		static void NormalizeScrollValueDown( f32 *pfValue );

		// Protected platform specific draw functions
		BOOL DrawPrep_P( BOOL bFlushImmediate );
		void DrawAllMaterials_P( FViewportPlanesMask_t nCrossesPlanesMask );
		BOOL SetupShaderParameters( FMeshMaterial_t *pMaterial, u32 nPasses ) const;
		
		#if FANG_PLATFORM_GC		
			void SetGPMatrix( FMeshMaterial_t *pMaterial, u16 nCurrentDL ); 		
		#endif
		
	public:
		// Data:
		FMesh_t *m_pMesh;					// Pointer to mesh associated with this instance
		u32 m_nFlags;						// Instance flags (see FMESHINST_FLAG_* for info)
		f32 m_fCullDist;					// Don't draw this mesh if its distance to the camera squared is greater than this value (FMATH_MAX_FLOAT=don't distance cull)

		u16 m_nkDOPBufferStartIdx;			// Index into transformed collision kDOP pool
		u8  m_nCullDirection;
		u8	m_nCurrentLOD;					// The LOD at which this mesh was last rendered

		CFXfm m_Xfm;						// Transformation matrix for this instance (modelspace to worldspace)
		CFSphere m_BoundSphere_MS;			// Bounds the entire object in model space (must account for animation)
											//  (if the STATIC and POSTER_Y flags are both set, the bounding sphere's origin must be 0,0,0)
		// Ambient light:
		CFColorRGB m_FinalAmbientRGB;		// Ambient light for this mesh instance

		#if FSH_DYNAMIC_SREFLECT
		BOOL m_bReflect;			// Texture used for dynamic reflections.
		CFTexInst *m_pReflectTex;
		#endif

		//For procedural textures
		void *m_pProcedural;

	protected:
		u32 m_nFrameLastDrawn;				// Snapshot of FVid_nFrameCounter when this instance was last rendered
		
		CFMtx43A **m_apBoneMtxPalette;		// Skinning 3D orientation & translation (transforms a MS vertex into WS)

		u32 m_nCreatedPartsMask;			// This mask indicates which "parts" of the mesh were activated at creation time (mesh can use no more than these parts)
		u32 m_nDrawnPartsMask;				// This mask indicates which "parts" of the mesh should be drawn.  "Parts" are defined in the mesh aide files
		u32 m_nCollPartsMask;				// This mask indicates which "parts" of the mesh are collidable
		u8	m_nCreatedBonesCount;			// Number of bones at creation time involved in rendering the created parts
		u8	m_nDrawnBonesCount;				// Number of active bones involved in rendering the active parts
		u8	m_nColorStreamCount;			// Number of platform specific vertex color streams in m_paColorStreams
	
		u8	m_nRenderLightCount;			// Number of lights in light list pointed to be m_pRenderLights
		s8	m_nLowestWeightedLightIdx;		// The index of the current lowest weighted light in the light list
		u8	__PAD;

		u16 m_nPixelRadiusLastRender;		// The on screen pixel radius for the tracker last time it was rendered

		FMesh_RenderLight_t *m_pRenderLights;// Pointer to array of CFLight pointers to be used for rendering

		void *m_papColorStreams;			// An array of platform specific color streams that represent the vertex colors for this instance
		
		FMeshInstTexLayerID_t *m_aTexLayerID;// Each slot corresponds to the same slot in FMesh_t::pTexLayerIDArray
		CFMeshAnimTC *m_aAnimTC;			// Describes the current TC animation state
		CFMeshTexFlip *m_aTexFlip;			// Describes the current texture flip page state

		CFColorRGBA m_MeshRGBA;				// The RGB is used to tint the mesh.  The alpha is used for translucency.

		f32 m_fShadowIntensity;				// 0 to 1 value indicating the intensity adjuster for the mesh's shadow

		u32 m_nFrameOfLastAnimateLayer;		// Snapshot of FVid_nFrameCounter of when AnimateLayers() was last called
		u64 m_nLastAnimateLayerTicks;		// The tick count of when AnimateLayers() was last called

		FMeshCFMeshInstDrawCallback_t *m_pPreDrawCallback;	// Callback that is called just prior to drawing the instance - could be called more than once per frame
		FMeshCFMeshInstDrawCallback_t *m_pPostDrawCallback;	// Callback that is called just after drawing the instance - could be called more than once per frame
		void *m_pPreDrawData;
		void *m_pPostDrawData;

		// Light register override for instance lightmaps
		u32 *m_panLightRegisterLMOverride;

		#if FANG_PLATFORM_GC
			u16 m_nStartingMatrixBufferIdx;
			u16 m_nStartingSkinVertexBufferIdx;
		#elif FANG_PLATFORM_DX
			CFMtx43A	m_PosMtx;
			f32			m_fMatrixScale;
//ARG - >>>>>
		#elif FANG_PLATFORM_PS2
			CFMtx43A	m_PosMtx;
			f32			m_fMatrixScale;
//ARG - <<<<<
		#endif
		
		static u8 m_anSaveData[ FMESH_MAX_LAYER_SAVE_SLOTS * (sizeof(CFMeshAnimTC) + sizeof(CFMeshTexFlip)) ];

	private:

		f32 GetMatAngularIntensity( FMeshMaterial_t *pMat ) const;

FCLASS_STACKMEM_ALIGN( CFMeshInst );
} FCLASS_ALIGN_SUFFIX;

//
//
FINLINE FMeshInstTexLayerID_t *CFMeshInst::GetTexLayerID( FMeshTexLayerHandle_t hTexLayer ) const 
{
	FASSERT( (s32)hTexLayer>=0 && (s32)hTexLayer<m_pMesh->nTexLayerIDCount );
	return m_aTexLayerID + (s32)hTexLayer;
}

//
//
FINLINE CFMeshAnimTC *CFMeshInst::GetAnimTC( FMeshTexLayerHandle_t hTexLayer ) const 
{
	FASSERT( (s32)hTexLayer>=0 && (s32)hTexLayer<m_pMesh->nTexLayerIDCount );
	return m_aTexLayerID[ (s32)hTexLayer ].pAnimTC;
}

//
//
FINLINE CFMeshTexFlip *CFMeshInst::GetTexFlip( FMeshTexLayerHandle_t hTexLayer ) const 
{
	FASSERT( (s32)hTexLayer>=0 && (s32)hTexLayer<m_pMesh->nTexLayerIDCount );
	return m_aTexLayerID[ (s32)hTexLayer ].pTexFlip;
}

//
//
FINLINE CFColorRGB* CFMeshInst::LayerEmissive_Get( FMeshTexLayerHandle_t hTexLayer ) const
{
	FMeshInstTexLayerID_t *pLayerID = GetTexLayerID( hTexLayer );
	if ( !pLayerID || !pLayerID->pEmissiveMotif )
	{
		return NULL;
	}
	
	return &pLayerID->pEmissiveMotif->ColorRGB;
}

//
//
FINLINE void CFMeshInst::LayerEmissive_Set( FMeshTexLayerHandle_t hTexLayer, f32 fRed, f32 fGreen, f32 fBlue )
{
	FMeshInstTexLayerID_t *pLayerID = GetTexLayerID( hTexLayer );
	if ( !pLayerID )
	{
		return;
	}

	if ( !pLayerID->pEmissiveMotif )
	{
		pLayerID->pEmissiveMotif = (CFColorMotif *)fres_Alloc( sizeof( CFColorMotif ) );
		if ( !pLayerID->pEmissiveMotif )
		{
			return;
		}
	}
	
	pLayerID->pEmissiveMotif->ColorRGB.Set( fRed, fGreen, fBlue );
	pLayerID->pEmissiveMotif->nMotifIndex = 0;
}

//
//
FINLINE f32 CFMeshInst::LayerAlpha_Get( FMeshTexLayerHandle_t hTexLayer ) const
{
	FMeshInstTexLayerID_t *pLayerID = GetTexLayerID( hTexLayer );
	if ( !pLayerID )
	{
		return -1.f;
	}
	
	return (pLayerID->nLayerAlpha * (1.f / 255.f));
}

//
//
FINLINE void CFMeshInst::LayerAlpha_Set( FMeshTexLayerHandle_t hTexLayer, f32 fAlpha )
{
	FMeshInstTexLayerID_t *pLayerID = GetTexLayerID( hTexLayer );
	if ( !pLayerID )
	{
		return;
	}
	
	pLayerID->nLayerAlpha = (u8)(fAlpha * 255.f);
}

//
//
FINLINE void CFMeshInst::OverrideZMask(CFTexInst *pTex)
{
	u32 i;
	for (i=0; i<m_pMesh->nMaterialCount; i++)
	{
		m_pMesh->aMtl[i].pnShLightRegisters[FSHADERS_LIGHT_REG_ZMASK] = (u32)pTex;
	}
}

//
//
FINLINE void CFMeshInst::TexFlip_SetFlipPage( FMeshTexLayerHandle_t hTexLayer, u32 nFlipPageIndex ) 
{
	CFMeshTexFlip *pTexFlip;

	pTexFlip = GetTexFlip(hTexLayer);

	FASSERT( nFlipPageIndex < pTexFlip->m_nFlipPageCount );

	pTexFlip->m_nCurrentPageIndex = (u8)nFlipPageIndex;
	pTexFlip->m_fCurrentPageIndex = (f32)nFlipPageIndex + (pTexFlip->m_fCurrentPageIndex - (f32)((u32)pTexFlip->m_fCurrentPageIndex) );

	if( pTexFlip->m_apFlipPalette ) 
	{
		pTexFlip->m_pTexInst = pTexFlip->m_apFlipPalette[nFlipPageIndex];
	}
}

//
//
FINLINE u32 CFMeshInst::TexFlip_GetFlipPage( FMeshTexLayerHandle_t hTexLayer ) const 
{
	return GetTexFlip(hTexLayer)->m_nCurrentPageIndex;
}

FINLINE u32 CFMeshInst::TexFlip_GetFlipPageCount( FMeshTexLayerHandle_t hTexLayer ) const 
{
	return GetTexFlip(hTexLayer)->m_nFlipPageCount;
}
//
//
FINLINE void CFMeshInst::TexFlip_SetFlipPalette( FMeshTexLayerHandle_t hTexLayer, CFTexInst **apFlipPalette, u32 nPaletteEntryCount ) 
{
	CFMeshTexFlip *pTexFlip;

	FASSERT( nPaletteEntryCount<=255 );

	pTexFlip = GetTexFlip(hTexLayer);

	pTexFlip->m_nFlipPageCount = (u8)nPaletteEntryCount;
	pTexFlip->m_apFlipPalette = apFlipPalette;
	pTexFlip->m_nCurrentPageIndex = 0;
	pTexFlip->m_fCurrentPageIndex = 0.0f;

	if( nPaletteEntryCount && apFlipPalette ) 
	{
		pTexFlip->m_pTexInst = apFlipPalette[pTexFlip->m_nCurrentPageIndex];
	} 
	else 
	{
		pTexFlip->m_pTexInst = NULL;
	}
}

//
//
FINLINE void CFMeshInst::TexFlip_GetFlipPalette( FMeshTexLayerHandle_t hTexLayer, CFTexInst ***papFlipPalette, u32 *pnPaletteEntryCount ) const 
{
	CFMeshTexFlip *pTexFlip;

	pTexFlip = GetTexFlip(hTexLayer);

	if( papFlipPalette ) {
		*papFlipPalette = pTexFlip->m_apFlipPalette;
	}

	if( pnPaletteEntryCount ) {
		*pnPaletteEntryCount = pTexFlip->m_nFlipPageCount;
	}
}

//
//
FINLINE void CFMeshInst::TexFlip_SetSingleTexture( FMeshTexLayerHandle_t hTexLayer, CFTexInst *pTexInst ) 
{
	CFMeshTexFlip *pTexFlip;

	pTexFlip = GetTexFlip(hTexLayer);

	pTexFlip->m_apFlipPalette = &pTexFlip->m_pTexInst;
	pTexFlip->m_nFlipPageCount = 1;
	pTexFlip->m_pTexInst = pTexInst;
	pTexFlip->m_nCurrentPageIndex = 0;
	pTexFlip->m_fCurrentPageIndex = 0.0f;
}

//
//
FINLINE CFTexInst *CFMeshInst::TexFlip_GetSingleTexture( FMeshTexLayerHandle_t hTexLayer ) const 
{
	return GetTexFlip(hTexLayer)->m_pTexInst;
}

//
//
FINLINE void CFMeshInst::NormalizeScrollST( CFMeshAnimTC *pAnimTC ) 
{
	// Normalize the scroll S coordinate...
	if( pAnimTC->m_ScrollPos.x >= 1.0f ) 
	{
		NormalizeScrollValueDown( &pAnimTC->m_ScrollPos.x );
	} 
	else if( pAnimTC->m_ScrollPos.x <= -1.0f ) 
	{
		NormalizeScrollValueUp( &pAnimTC->m_ScrollPos.x );
	}

	// Normalize the scroll T coordinate...
	if( pAnimTC->m_ScrollPos.y >= 1.0f ) 
	{
		NormalizeScrollValueDown( &pAnimTC->m_ScrollPos.y );
	} 
	else if( pAnimTC->m_ScrollPos.y <= -1.0f ) 
	{
		NormalizeScrollValueUp( &pAnimTC->m_ScrollPos.y );
	}
}

//
//
FINLINE void CFMeshInst::GetBoneSphere_WS( u32 nBoneIdx, CFSphere *pResult )
{
	FASSERT( pResult );
    FASSERT( m_pMesh );
	FASSERT( m_pMesh->nBoneCount > 0 );
	FASSERT( nBoneIdx < m_pMesh->nBoneCount );

	if ( !(m_nFlags & FMESHINST_FLAG_NOBONES) ) 
	{
		CFMtx43A *pBoneMtx = GetBoneMtxPalette()[nBoneIdx];
		pResult->m_Pos = pBoneMtx->m44.MultPoint( m_pMesh->pBoneArray[nBoneIdx].SegmentedBoundSphere_BS.m_Pos );
		pResult->m_fRadius *= pBoneMtx->m_vFront.Mag();
	}
	else
	{
		CFMtx43A mtxTransform;
		mtxTransform.Mul( m_Xfm.m_MtxF, m_pMesh->pBoneArray[nBoneIdx].AtRestBoneToModelMtx );
		pResult->m_Pos = mtxTransform.m44.MultPoint( m_pMesh->pBoneArray[nBoneIdx].SegmentedBoundSphere_BS.m_Pos );
		pResult->m_fRadius *=mtxTransform.m_vFront.Mag();
	}
}

extern s32  FMesh_nForceLOD;
extern BOOL FMesh_bForceMinLOD;


extern void fmesh_Renderer_Open( void );
extern void fmesh_Renderer_Close( void );
extern u32 fmesh_Renderer_GetStateSize( void );
extern void fmesh_Renderer_GetState( void *pDestState );
extern void fmesh_Renderer_SetState( const void *pState );
extern void fmesh_Renderer_SetDefaultState( void );

extern s32 fmesh_FindBone( const FMesh_t *pMesh, cchar *pszBoneName );

//
// Call this to force flushing of all relevant mesh states prior to render.
// Note that if the CFMeshInst::DrawPrep_P() call was called with bFlushImmediate
// set, then there is no need to call this function.
extern BOOL fmesh_FlushDrawPrep( void );

extern void fmesh_SetCullDir( FMeshCullDir_e nCullDir );

//this is here so I can track the player mesh to display lighting info.
extern CFMeshInst *FMesh_PlayerMesh;
FINLINE void fmesh_RegisterPlayerMesh(CFMeshInst *pMesh)
{
	FMesh_PlayerMesh = pMesh;
}

#endif

