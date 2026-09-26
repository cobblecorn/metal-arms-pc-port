//////////////////////////////////////////////////////////////////////////////////////
// ape_file_def.h - 
//
// Author: Michael Starich   
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
// 10/11/00 Starich     Created.
//////////////////////////////////////////////////////////////////////////////////////
#ifndef _APE_FILE_DEF_H_
#define _APE_FILE_DEF_H_ 1

#include "fang.h"
#include "fmath.h"
#include "fcolor.h"
#include "fversion.h"
#include "fmesh.h"

//////////////////////////////////////////////////////////////////////////////////////
// NOTE: WHEN ADDING TO A STRUCT TAKE ELEMENTS FROM nUnused[] THIS WAY PASM WILL STILL
// BE ABLE TO READ THE OLD APE FILE.
// IF YOU NEED TO ADD NEW STRUCTURES OR DELETE FIELDS, THEN YOU WILL NEED TO CHANGE
// THE VERSION NUMBER INSIDE FVERSION.H.
// BE WARNED:
// IF YOU CHANGE THE APE VERSION NUMBER, YOU WILL NEED TO RE-EXPORT ALL MAX/MAYA FILES
// AS THEY WILL NO LONGER BE ABLE TO BE COMPILED BY PASM.
//////////////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////////////////////
// FILE FORMAT: 
// ApeMesh_t
//		ApeBone_t[] - the number of elements is specified in the mesh
//		ApeLight_t[] - the number of elements is specified in the mesh
//		ApeObject_t[] - the number of elements is specifed in the mesh
//		ApeFog_t[] - the number of elements is specifed in the mesh
//		ApeShape_t[] - the number of elements is specified in the mesh
//		ApeVisVolume_t[] - the number of elements is specified in the mesh
//		ApeVisPortal_t[] - the number of elements is specifed in the mesh
//		ApeSegment_t[] - each segment is variable length, each contains a:
//			ApeMaterial_t[] - the number of materials is specified in the segment
//			ApeVert_t[] - the number of verts is specified in the segment (this is all of the verts in the entire segment)		
//			ApeVertIndex_t[] - the number of indices is specified in the segment (this is all the tris in the entire segment)
//				
/////////////////////////////////////////////////////////////////////////////////////////////

#define TEXTURE_NAME_LEN			16
#define LIGHT_NAME_LEN				16
#define SEGMENT_NAME_LEN			16
#define MESH_NAME_LEN				16
#define BONE_NAME_LEN				32
#define OBJECT_NAME_LEN				16
#define MAX_LAYERS_PER_MAT			4
#define MAX_WEIGHTS_PER_VERT		4
#define MAX_WEIGHTS_PER_FACE		4
#define MAX_CHILDREN_PER_BONE		64
#define MAX_TOTAL_BONE_COUNT		200

#define MAX_VIS_CELLS_PER_VOL		16
#define MAX_VIS_FACES_PER_CELL		26
#define MAX_VIS_DEGREES_PER_FACE	6
#define MAX_VIS_PTS_PER_CELL		( MAX_VIS_FACES_PER_CELL * MAX_VIS_DEGREES_PER_FACE )
#define MAX_VIS_EDGES_PER_CELL		( (MAX_VIS_PTS_PER_CELL >> 1) + 1 )
#define VIS_NAME_LEN				32

#define ON_PLANE_EPSILON			( 0.01f )
#define ON_PLANE_EPSILON_2			( ON_PLANE_EPSILON * ON_PLANE_EPSILON )

typedef enum {
	APE_LIGHT_TYPE_SPOT = 0,
	APE_LIGHT_TYPE_OMNI,
	APE_LIGHT_TYPE_DIR,
	APE_LIGHT_TYPE_AMBIENT,

	APE_LIGHT_TYPE_COUNT
} ApeLightType_e;

enum {
	APE_LIGHT_FLAG_DONT_USE_RGB				= 0x00000001,	// Disregard the light's rgb and only use the motif's color (default = off)
	APE_LIGHT_FLAG_LIGHT_SELF				= 0x00000002,	// Light the object that the light is attached to (default = off)
	APE_LIGHT_FLAG_OBJ_DONT_LIGHT_TERRAIN	= 0x00000004,	// Lights attached to this object don't light the terrain (default = off)

	APE_LIGHT_FLAG_PER_PIXEL				= 0x00000008,	// This light casts a projection on the environment (may or may not have a texture)

	APE_LIGHT_FLAG_LIGHTMAP_ONLY_LIGHT		= 0x00000010,	// This light will only be used in the lightmap portion of PASM and will not be exported to the engine.
	APE_LIGHT_FLAG_LIGHTMAP_LIGHT			= 0x00000020,	// This light is to be used for generating lightmaps (If it is not dynamic, it can be discarded prior to the engine)
	APE_LIGHT_FLAG_UNIQUE_LIGHTMAP			= 0x00000040,	// This light will generate its own unique lightmap in the lightmapping phase (it must also have a unique m_nLightID)

	APE_LIGHT_FLAG_CORONA					= 0x00000080,	// This light has a corona
	APE_LIGHT_FLAG_CORONA_PROXFADE			= 0x00000100,	// Fade the corona as the camera gets closer.

	APE_LIGHT_FLAG_CAST_SHADOWS				= 0x00000200,	// This light will cast shadows (only relevant for engine lights)

	APE_LIGHT_FLAG_DYNAMIC_ONLY				= 0x00000400,	// This light will not affect static objects

	APE_LIGHT_FLAG_MESH_MUST_BE_PER_PIXEL	= 0x00000800,	// For per-pixel lights that have a projected texture.  If this flag is set, only objects that are flagged
															// as per pixel lit will have the texture projected on them.  Others will just apply as a dynamic vertex light

	APE_LIGHT_FLAG_NONE						= 0x00000000
};

enum {
	APE_OB_FLAG_STATIC				= 0x00000001,	// Object's bounding sphere has a static footprint in 3D space
	APE_OB_FLAG_POSTER_Y			= 0x00000002,	// Poster object around it's Y axis to always face the camera (neg-Z axis of object toward viewer)
	APE_OB_FLAG_NO_COLL				= 0x00000004,	// Don't collide with this object

	APE_OB_FLAG_NO_LIGHT			= 0x00000010,	// Don't light this object

	APE_OB_FLAG_POSTER_X			= 0x00000040,	// Poster object around it's X axis
	APE_OB_FLAG_POSTER_Z			= 0x00000080,	// Poster object around it's Z axis
	APE_OB_FLAG_NO_DRAW				= 0x00000100,
	APE_OB_FLAG_LM					= 0x00000200,	// This (static) object will receive light maps
	APE_OB_FLAG_PER_PIXEL			= 0x00000400,
	APE_OB_FLAG_VERT_RADIOSITY		= 0x00000800,	// This (static) object will receive vertex radiosity
	APE_OB_FLAG_ACCEPT_SHADOWS		= 0x00001000,	// This object will receive shadows
	APE_OB_FLAG_CAST_SHADOWS		= 0x00002000,	// This object will cast shadows
	APE_OB_FLAG_TINT				= 0x00004000,	// This object will be tinted
	APE_OB_FLAG_NO_LM_USE			= 0x00008000,	// This object will not be considered when generating lightmaps (even if static)
	
	APE_OB_FLAG_NONE				= 0x00000000
};

enum {
	APE_MAT_FLAGS_NO_DRAW				= 0x01,	// Polys with this material will not be drawn
	APE_MAT_FLAGS_APPLY_TINT			= 0x02,	// This tint is modulated into the texture color in the surface pass
	APE_MAT_FLAGS_DO_NOT_LM				= 0x04,	// These polys should not be light mapped, though they can obscure light
	APE_MAT_FLAGS_ZWRITE_ON				= 0x08,	// Relevant for translucent materials, only. When rendering this material, write to the zbuffer (default for translucent materials is not to)
	APE_MAT_FLAGS_DO_NOT_TINT			= 0x10,	// If tint is applied to the mesh, this material will not receive tint
	APE_MAT_FLAGS_NO_LM_USE				= 0x20,	// These polys should not be used in the lightmap phase (to receive or obscure light)
	APE_MAT_FLAGS_DO_NOT_BLOCK_LM		= 0x40,	// These polys will not block light during lightmap application
	APE_MAT_FLAGS_VERT_RADIOSITY		= 0x80,	// These polys will receive vertex radiosity

	APE_MAT_FLAGS_NONE					= 0x00
};

enum {
	APE_MAT_COLL_FLAGS_COLL_WITH_PLAYER				= 0x0001,
	APE_MAT_COLL_FLAGS_COLL_WITH_NPCS				= 0x0002,
	APE_MAT_COLL_FLAGS_OBSTRUCT_LINE_OF_SIGHT		= 0x0004,
	APE_MAT_COLL_FLAGS_COLL_WITH_THIN_PROJECTILES	= 0x0008,
	APE_MAT_COLL_FLAGS_COLL_WITH_THICK_PROJECTILTES = 0x0010,
	APE_MAT_COLL_FLAGS_COLL_WITH_CAMERA				= 0x0020,
	APE_MAT_COLL_FLAGS_COLL_WITH_OBJECTS			= 0x0040,
	APE_MAT_COLL_FLAGS_WALKABLE						= 0x0080,
	APE_MAT_COLL_FLAGS_OBSTRUCT_SPLASH_DAMAGE		= 0x0100,
	APE_MAT_COLL_FLAGS_COLLIDE_WITH_DEBRIS			= 0x0200,
	APE_MAT_COLL_FLAGS_COLLIDE_WITH_VEHICLES		= 0x0400,
	APE_MAT_COLL_FLAGS_HOVER_COLLIDABLE				= 0x0800,

	APE_MAT_COLL_FLAGS_COLL_WITH_NOTHING			= 0x0000,
	APE_MAT_COLL_FLAGS_COLL_WITH_EVERYTHING			= 0xFFFF
};

enum {
													// 0x00000000 - 0x000000ff are reserved for or'ing of APE_MAT_FLAGS_*
	APE_LAYER_FLAGS_DO_NOT_CAST_SHADOWS				= 0x00000100,
	APE_LAYER_FLAGS_INVERT_EMISSIVE_MASK			= 0x00000200,
	APE_LAYER_FLAGS_ANGULAR_EMISSIVE				= 0x00000400,
	APE_LAYER_FLAGS_ANGULAR_TRANSLUCENCY			= 0x00000800,
	APE_LAYER_FLAGS_NO_ALPHA_SCROLL					= 0x00001000,

	APE_LAYER_FLAGS_NONE							= 0x00000000,
};

enum
{
	APE_LAYER_TEXTURE_DIFFUSE = 0,		// From the "Diffuse" slot in max material editor
	APE_LAYER_TEXTURE_SPECULAR_MASK,	// From the "Specular Level" slot in max material editor
	APE_LAYER_TEXTURE_EMISSIVE_MASK,	// From the "Self-Illumination" slot in max material editor
	APE_LAYER_TEXTURE_ALPHA_MASK,		// From the "Opacity" slot in max material editor
	APE_LAYER_TEXTURE_BUMP,				// From the "Bump" slot in max material editor
	APE_LAYER_TEXTURE_DETAIL,			// From the "Displacement" slot in max material editor
	APE_LAYER_TEXTURE_ENVIRONMENT,		// From the "Reflection" slot in max material editor
	APE_LAYER_TEXTURE_UNUSED_3,			// For expansion
	APE_LAYER_TEXTURE_UNUSED_2,			// For expansion
	APE_LAYER_TEXTURE_UNUSED_1,			// For expansion

	APE_LAYER_TEXTURE_MAX,
};


enum {
	APE_BONE_FLAG_NOT_USED			= 0x00000001,	// this bone is not referenced by any face in the mesh
	
	APE_BONE_FLAG_NONE				= 0x00000000
};

enum {
	APE_PORTAL_FLAG_MIRROR			= 0x00000001,	// the portal is a mirror
	APE_PORTAL_FLAG_SOUND_ONLY		= 0x00000002,	// the portal is for sound only
	APE_PORTAL_FLAG_ONE_WAY			= 0x00000004,	// the portal is 1 way
	APE_PORTAL_FLAG_ANTI			= 0x00000008,	// the portal is an anti portal

	APE_PORTAL_FLAG_NONE			= 0x00000000
};

typedef enum {
	APE_SHAPE_TYPE_SPHERE = 0,
	APE_SHAPE_TYPE_CYLINDER,
	APE_SHAPE_TYPE_BOX,
	APE_SHAPE_TYPE_CAMERA,
	APE_SHAPE_TYPE_SPEAKER,
	APE_SHAPE_TYPE_SPAWN_POINT,	// Used for AI
	APE_SHAPE_TYPE_START_POINT,	// Used for placing the human player
	APE_SHAPE_TYPE_ROOM,		// Used to define rooms
	APE_SHAPE_TYPE_ARENA,		// Used to define arena's within rooms	
	APE_SHAPE_TYPE_PARTICLE_BOX,
	APE_SHAPE_TYPE_PARTICLE_SPHERE,
	APE_SHAPE_TYPE_PARTICLE_CYLINDER,
	APE_SHAPE_TYPE_SPLINE,

	APE_SHAPE_TYPE_COUNT
} ApeShapeType_e;

typedef enum {
	// 1 layer:
	APE_SHADER_TYPE_oBASE = 0,	//0 specular, & emissive versions too	(4 versions)
	APE_SHADER_TYPE_cBASE,		//1	specular, & emissive versions too	(4 versions)	
	APE_SHADER_TYPE_tBASE,		//2	specular, & emissive versions too	(4 versions)
	APE_SHADER_TYPE_obsBASE,	//3 emissive version too				(2 versions)
	APE_SHADER_TYPE_beoBASE,	//4 specular version too				(2 versions)
	APE_SHADER_TYPE_etbsBASE,	//5										(1 version)
	// 2 layers:
	APE_SHADER_TYPE_oBASE_LERP_tLAYER,//6 specular version too			(2 versions)
	APE_SHADER_TYPE_cBASE_LERP_tLAYER,//7 specular version too			(2 versions)
	APE_SHADER_TYPE_tBASE_LERP_tLAYER,//8 specular version too			(2 versions)
	
	APE_SHADER_TYPE_oBASE_LERP_vLAYER,//9								(1 version)
	APE_SHADER_TYPE_cBASE_LERP_vLAYER,//10								(1 version)
	APE_SHADER_TYPE_oBASE_LERP_pLAYER,//11								(1 version)	
	APE_SHADER_TYPE_cBASE_LERP_pLAYER,//12								(1 version)
	
	APE_SHADER_TYPE_oBASE_MOD_SHADOWMAP,//13 specular version too		(2 versions)
	APE_SHADER_TYPE_cBASE_MOD_SHADOWMAP,//14 specular version too		(2 versions)

	APE_SHADER_TYPE_oBASE_ADD_rbENV,//15								(1 version)
	APE_SHADER_TYPE_etBASE_ADD_rbENV,//16								(1 version)

	APE_SHADER_TYPE_oBASE_ADD_rbENV_MOD_SHADOWMAP,//17					(1 version)
	
	APE_SHADER_TYPE_ADD_BASE,//18										(1 version)

	APE_SHADER_TYPE_oBASE_ADD_rbSREFLECT,//19

	APE_SHADER_TYPE_tBASE_vALPHA,		  //20
	APE_SHADER_TYPE_LIQUID_ENV,			  //21
	APE_SHADER_TYPE_LIQUID_LAYER_ENV,	  //22
	APE_SHADER_TYPE_LIQUID_TEXTURE,		  //23
	APE_SHADER_TYPE_LIQUID_MOLTEN_1LAYER, //24
	APE_SHADER_TYPE_LIQUID_MOLTEN_2LAYER, //25

	APE_SHADER_TYPE_oBASE_LERP_tLAYER_ADD_rbENV,	//26
	APE_SHADER_TYPE_oBASE_LERP_vLAYER_ADD_rbENV,    //27
	APE_SHADER_TYPE_oBASE_LERP_pLAYER_ADD_rbENV,	//28

	APE_SHADER_TYPE_pBASE,		//2	-> 29
	APE_SHADER_TYPE_epbsBASE,	//5	-> 30								(1 version)
	APE_SHADER_TYPE_ADD_vBASE,	//18 -> 31
	
} ApeShaderType_e;

//////////////
// MESH HEADER
typedef struct {
	FVersionHeader_t Header;// has a signature and version info
	u32 nBytesInFile;	// what is the total file size of the entire .ape file
	char szMeshName[MESH_NAME_LEN];
	BOOL bWorldFile;	// does this ape file represent a world, if FALSE then this is an object
	u16 nNumBoneNames;	// how many bone names follow this structure
	u16 nNumVisVolumes;	// how many vis volumes follow the last shape
	u16 nNumLights;		// how many lights follow the last bone name
	u16 nNumVisPortals;	// how many vis portals follow the last vis volume
	u16 nNumObjects;	// how many objects follow the last light 
	u16 nNumFogs;		// how many fog structs follow the last object
	u16 nNumSegments;	// how many segments follow the last vis portal struct	
	u16 nNumShapes;		// how many shapes follow the last fog struct
	u16 nSizeOfBoneStruct;		// how many bytes is the ApeBone_t
	u16 nSizeOfLightStruct;		// how many bytes is the ApeLight_t
	u16 nSizeOfObjectStruct;	// how many bytes is the ApeObject_t
	u16 nSizeOfFogStruct;		// how many bytes is the ApeFog_t
	u16 nSizeOfSegmentStruct;	// how many bytes is the ApeSegment_t
	u16 nSizeOfMaterialStruct;	// how many bytes is the ApeMaterial_t
	u16 nSizeOfVertStruct;		// how many bytes is the ApeVert_t
	u16 nSizeOfVertIndexStruct;	// how many bytes is the ApeVertIndex_t
	u16 nSizeOfShapeStruct;		// how many bytes is the ApeShape_t
	
	u8 nUnused[64];				// USE THESE BYTES TO ADD NEW FIELDS
} ApeMesh_t;

/////////////////
// BONE STRUCTURE
typedef struct {
	char szBoneName[BONE_NAME_LEN];	// what is the name of this bone, THIS MUST MATCH THE NAME IN THE ANIMATION FILE!!!
	u32 nFlags;						// see APE_BONE_FLAG_* for info
	u32 nBoneIndex;					// what bone index is this
	s32 nParentIndex;				// -1 = no parent
	CFMtx43 AtRestModelToBoneMtx;	// LEFT HANDED COORDINATE SYSTEM!!!
	u32 nNumChildren;				// how many children does this bone have
	u8 auChildIndices[MAX_CHILDREN_PER_BONE];// only the first nNumChildren are used
		
	u8 nUnused[16];					// USE THESE BYTES TO ADD NEW FIELDS
} ApeBone_t;

//////////////////
// LIGHT STRUCTURE
typedef struct {
	ApeLightType_e nType;// see APE_LIGHT_TYPE_* for info	
	char szLightName[LIGHT_NAME_LEN];
	CFSphere Sphere;
	CFVec3 Dir;
	CFColorRGB Color;	// unit floats
	f32 fIntensity;
	f32 fSpotInnerAngle;// in radians, the full angle
	f32 fSpotOuterAngle;// in radians, the full angle
	u32 nFlags;			// see APE_LIGHT_FLAG_* for info
	u32 nMotifID;		// what is the light's motif ID (default = 0)
	f32 fCoronaScale;
	CFMtx43 mtxOrientation; // Orientation of the light relative to it's parent or the world/model, as applicable
	char szCoronaTexture[TEXTURE_NAME_LEN];
	char szPerPixelTexture[TEXTURE_NAME_LEN];
	u16 nLightID;
    
	char szParentBoneName[BONE_NAME_LEN];	// The name of the parent bone in the object hiearchy (or 0 if there is no parent

	u8 nUnused[30];
} ApeLight_t;

///////////////////
// OBJECT STRUCTURE
typedef struct {
	char szName[OBJECT_NAME_LEN];
	u32 nFlags;				// See APE_OB_FLAG_* for info
	CFMtx43 Orientation;	// a left handed orientation, Rot, Pos, & Scale
	u32 nBytesOfUserData;	// how many bytes follow this struct
	f32 fCullDistance;		// how many meters way from the camera should this object no longer be drawn
	u32 nParentIndex;		// are we attached to a parent? (either object or shape) (0 = none, nParentIndex-1 = index into combined obj/shape array)
	CFColorRGB TintRGB;		// The tint to apply to this mesh instance

	u8 nUnused[20];			// USE THESE BYTES TO ADD NEW FIELDS
} ApeObject_t;

////////////////
// FOG STRUCTURE
typedef struct {
	f32 fStartDist;		// at what distance should the fog start? (meters)
	f32 fEndDist;		// at what distance should the fog end?	(meters)
	f32 fUnitDensity;	// what is the max density
	CFColorRGB Color;	// what color is the fog
	u32 nMotifID;		// what is the fog's motif ID

	u8 nUnused[16];		// USE THESE BYTES TO ADD NEW FIELDS
} ApeFog_t;

//////////////////
// SHAPE STRUCTURE


// the following structs are used by the different shape types
// IF ANY STRUCT IS MORE THAN 16 BYTES, YOU MUST INCREASE THE APE FILE VERSION!!!
typedef struct {
	u8 anUnused[4];
	f32 fLength;	// our z, if no rotation
	f32 fWidth;		// our x, if no rotation
	f32 fHeight;	// our y, if no rotation	
} ApeBoxShape_t;	// nType == APE_SHAPE_TYPE_BOX

typedef struct {
	f32 fRadius;
	u8 anUnused[12];
} ApeSphereShape_t;	// nType == APE_SHAPE_TYPE_SPHERE

typedef struct {
	f32 fRadius;
	f32 fHeight;	// our y, if no rotation
	u8 anUnused[8];
} ApeCylinderShape_t;// nType == APE_SHAPE_TYPE_CYLINDER

typedef struct {
	f32 fFOV;		// in radians, the full angle
	u32 nFrames;	// how many animation frames are there?
	u32 nOffsetToFrames;// offset into the user data buffer to the first frame
	u32 nOffsetToString;// offset into the user data buffer to the string data
} ApeCameraShape_t;	// nType == APE_SHAPE_TYPE_CAMERA

typedef struct {
	f32 fRadius;
	f32 fUnitVolume;
	u8 anUnused[8];
} ApeSpeakerShape_t;// nType == APE_SHAPE_TYPE_SPEAKER

typedef struct {
	u8 anUnused[16];
} ApeSpawnPointShape_t;// nType == APE_SHAPE_TYPE_SPAWN_POINT

typedef struct {
	u8 anUnused[16];
} ApeStartPointShape_t;	// nType == APE_SHAPE_TYPE_START_POINT

typedef struct {
	f32 fLength;	// our z, if no rotation
	f32 fWidth;		// our x, if no rotation
	f32 fHeight;	// our y, if no rotation	
	u32 nRoomID;
} ApeRoomShape_t;	// nType == APE_SHAPE_TYPE_ROOM

typedef struct {
	f32 fLength;	// our z, if no rotation
	f32 fWidth;		// our x, if no rotation
	f32 fHeight;	// our y, if no rotation	
	u8 anUnused[4];	
} ApeArenaShape_t;	// nType == APE_SHAPE_TYPE_ARENA

typedef struct {
	f32 fLength;	// our z, if no rotation
	f32 fWidth;		// our x, if no rotation
	f32 fHeight;	// our y, if no rotation	
	u8 anUnused[4];
} ApeParticleBoxShape_t;// nType == APE_SHAPE_TYPE_PARTICLE_BOX

typedef struct {
	f32 fRadius;
	u8 anUnused[12];
} ApeParticleSphereShape_t;// nType == APE_SHAPE_TYPE_PARTICLE_SPHERE

typedef struct {
	f32 fRadius;
	f32 fHeight;	// our y, if no rotation
	u8 anUnused[8];
} ApeParticleCylinderShape_t;// nType == APE_SHAPE_TYPE_PARTICLE_CYLINDER

typedef struct {
	u32 nNumPts;	// how many ApeSplinePt_t structs are contained in the user data (this array is the 1st data in the user props)
	BOOL bClosed;	// connect the last pt and the first pt?
	u32 nNumSegments;// depending on bClosed, will either be (nNumPts - 1) or (nNumPts)
	u8 anUnused[4];
} ApeSplineShape_t;	// nType == APE_SHAPE_TYPE_SPLINE

typedef struct {
	ApeShapeType_e nType;// see APE_SHAPE_TYPE_* for info

	// you can add structs to this union,
	// IF ANY STRUCT IS MORE THAN 16 BYTES, YOU MUST INCREASE THE APE FILE VERSION!!!
	union {
		ApeBoxShape_t				Box;
		ApeSphereShape_t			Sphere;
		ApeCylinderShape_t			Cylinder;
		ApeCameraShape_t			Camera;
		ApeSpeakerShape_t			Speaker;
		ApeSpawnPointShape_t		SpawnPt;
		ApeStartPointShape_t		StartPt;	
		ApeRoomShape_t				Room;
		ApeArenaShape_t				Arena;
		ApeParticleBoxShape_t		ParticleBox;
		ApeParticleSphereShape_t	ParticleSphere;
		ApeParticleCylinderShape_t	ParticleCylinder;
		ApeSplineShape_t			Spline;
	} TypeData;
	CFMtx43 Orientation;	// a left handed orientation, Rot, Pos, & Scale
	u32 nBytesOfUserData;	// how many bytes follow this struct
	u32 nParentIndex;		// are we attached to a parent? (either object or shape) (0 = none, nParentIndex-1 = index into combined obj/shape array)

	u8 nUnused[12];			// USE THESE BYTES TO ADD NEW FIELDS
} ApeShape_t;

// used by an ApeShape_t (nType == APE_SHAPE_TYPE_CAMERA)
typedef struct {
	f32 fSecsFromStart;
	f32 fFOV;
} ApeCamFrame_t;

// used by an ApeSplineShape_t (nType == APE_SHAPE_TYPE_SPLINE)
typedef struct {
	CFVec3 Pos;// in left handed world space
} ApeSplinePt_t;

/////////////////
// VIS STRUCTURES

typedef struct {
	CFVec3 Pos;			// in left handed world space
} ApeVisPt_t;

typedef struct {
	u16 anPtIndices[2];		// indices into the pt array
	
	u32 nNumFaces;			// can only be 0, 1, or 2
	u16 anFaceIndices[2];	// indices into the face array (nNumFaces of are used)

	u8 nUnused[4];		// USE THESE BYTES TO ADD NEW FIELDS
} ApeVisEdges_t;

typedef struct {
	u32 nDegree;		// the degree is the number of points and edges
	u16 aPtIndices[MAX_VIS_DEGREES_PER_FACE];// nDegree are used
	u16 aEdgeIndices[MAX_VIS_DEGREES_PER_FACE];// nDegree are used
	
	CFVec3 Normal;		// face outward of the cell
	CFVec3 Centroid;	// a central point on the face

	u8 nUnused[4];		// USE THESE BYTES TO ADD NEW FIELDS	
} ApeVisFaces_t;

typedef struct {
	char szName[VIS_NAME_LEN];// a name so that this particular portal can be idenitifed

	u32 nNumPts;
	ApeVisPt_t aPts[MAX_VIS_PTS_PER_CELL];// nNumPts are used

	u32 nNumEdges;
	ApeVisEdges_t aEdges[MAX_VIS_EDGES_PER_CELL];// nNumEdges are used

	u32 nNumFaces;
	ApeVisFaces_t aFaces[MAX_VIS_FACES_PER_CELL];// nNumFaces are used

	CFSphere Sphere;

	u32 nFlags;

	u8 nUnused[16];		// USE THESE BYTES TO ADD NEW FIELDS
} ApeVisCell_t;

typedef struct {
	u32 nNumCells;
	ApeVisCell_t aCells[MAX_VIS_CELLS_PER_VOL];// nNumCells are used

	CFSphere Sphere;

	u32 nFlags;

	u8 nUnused[16];		// USE THESE BYTES TO ADD NEW FIELDS
} ApeVisVolume_t;

typedef struct {
	char szName[VIS_NAME_LEN];// a name so that this particular portal can be idenitifed

	CFVec3 aCorners[4];
	CFVec3 Normal;
	CFVec3 Centroid;

	u32 nFlags;			// see APE_PORTAL_FLAG_* for details

	u8 nUnused[16];		// USE THESE BYTES TO ADD NEW FIELDS	
} ApeVisPortal_t;

////////////////////
// SEGMENT STRUCTURE
typedef struct {
	char szNodename[SEGMENT_NAME_LEN];
	BOOL bSkinned;		// all of the tris in this segment are skinned
	u32 nNumMaterials;	// how many ApeMaterial_t structures follow this structure	
	u32 nNumVerts;		// how many ApeVert_t structures follow the last material structure
	u32 nNumIndices;	// how many ApeVertIndex_t structure follow the last vert structure	

	u8 nUnused[16];		// USE THESE BYTES TO ADD NEW FIELDS
} ApeSegment_t;

/////////////////////////
// STAR COMMAND STRUCTURE
typedef struct {
	// vars related to *sort
	BOOL bSort;				// should we do a dynamic sort
	// vars related to *order
	u32 nOrderNum;			// should we rearrange this material's order in the mesh, 0 means no rearranging
	// vars related to *shader
	s32 nShaderNum;			// what is the shader number, -1 means none was set
	// vars related to *motif
	u32 nEmissiveMotifID;	// what is the emissive motif ID
	u32 nSpecularMotifID;	// what is the specular motif ID
	u32 nDiffuseMotifID;	// what is the diffuse motif ID
	BOOL bUseEmissiveColor;	// should we use the emissive color
	BOOL bUseSpecularColor;	// should we use the specular color
	BOOL bUseDiffuseColor;	// should we use the diffuse color
	// vars related to *anim
	u32 nNumTexFrames;		// how many texture frames are there
	f32 fFramesPerSecs;		// how many frames should be played per second
	// vars related to *scroll
	f32 fDeltaUPerSec;		// how much should the u tc move per second
	f32 fDeltaVPerSec;		// how much should the v tc move per second
	// vars related to *Z
	u32 nZTugValue;			// 0 is no tug at all
	// vars related to *id
	u8 nID;					// default = 255, manual assigned = 0-127, auto assigned = 128-254
	// vars related to *nocoll
	BOOL8 bNoColl;			// default = FALSE, don't collide with this material
	// vars related to *collid
	u8 nCollID;				// default = 0, manual assigned = 1-63

	u8 nFlags;			// see APE_MAT_FLAGS_
	u16 nCollMask;		// see APE_MAT_COLL_FLAGS_, default APE_MAT_COLL_FLAGS_COLL_WITH_EVERYTHING
	u16 nReactType;		// 0 - 7, default 0
	u16 nSurfaceType;	// 0 - 15, default 0

	CFColorRGB  TintRGB;	// Color to tint the material
	CFColorRGBA LightRGBI;	// Light value for this surface (RGB, intensity), if used as a light surface that emits into the light map scene

	f32 fBumpMapTileFactor;
	f32 fDetailMapTileFactor;

	f32 fDeltaUVRotationPerSec;	// how many degrees the UV's should rotate around 0.5, 0.5 per second
	CFVec2 vRotateUVAround;		// center point of uv rotation

	//////////////////////////////////////////////////////////////////////////////
	// MAKE SURE THAT IF YOU ADD FIELDS YOU UPDATE PASM'S MAT COMPRESSION ROUTINES
	//////////////////////////////////////////////////////////////////////////////

	u8 nUnused[12];			// USE THESE BYTES TO ADD NEW FIELDS
} ApeCommands_t;


//////////////////
// LAYER STRUCTURE
typedef struct {
	BOOL bTextured;// does this layer have a valid texture map assigned to it
	char szTexnames[APE_LAYER_TEXTURE_MAX][TEXTURE_NAME_LEN];// no path or extension is included
	f32 fUnitAlphaMultiplier;// unit float, amount to scale vertex alpha when using some shader types
	BOOL8 bDrawAsWire;
	BOOL8 bTwoSided;		// tells PASM to generate 2 tris
	BOOL8 abTile[2];		// [0] is U, [1] is V
	CFColorRGB SpecularRGB;	// each element is a unit float
	CFColorRGB SelfIllumRGB;// aka emmissiveness, each element is a unit float
	CFColorRGB DiffuseRGB;	// each element is a unit float
	f32 fShininess;			// not a unit f32, represents an exponent, range(0.0f - 127.0f)
	f32 fShinStr;			// unit f32 (used to scale the specualar color)
	ApeCommands_t StarCommands;

	u8 nUnused[36];			// USE THESE BYTES TO ADD NEW FIELDS
} ApeLayer_t;

/////////////////////
// MATERIAL STRUCTURE
typedef struct {
	u32 nLayersUsed;// how many aLayer[] are used
	ApeLayer_t aLayer[MAX_LAYERS_PER_MAT];	// [0] represents the base and each index works upward 
											// to the last index which is the topmost layer
	u32 nFirstIndex;// what is this material's first ApeVertIndex_t index into the segment's index array
	u32 nNumIndices;// how many ApeVertIndex_t structures does this material use
	ApeCommands_t StarCommands;
	u16 nLODIndex;
	u16 nAffectAngle; // Used for angular emissive or angular translucency
	u32 nFlags;		  // see APE_MAT_FLAGS_USE_...

	u8 nUnused[24];	  // USE THESE BYTES TO ADD NEW FIELDS
} ApeMaterial_t;

///////////////////
// WEIGHT STRUCTURE
typedef struct {
	f32 fBoneIndex;	// what bone index does fWeight represent
	f32 fWeight;	// unit float

	u8 nUnused[16];	// USE THESE BYTES TO ADD NEW FIELDS
} ApeWeight_t;

/////////////////
// VERT STRUCTURE

// KEEP THIS STRUCTURE'S ELEMENTS ALL FLOATING POINT SO THAT OUR HASH FUNCTION WORKS PERFECTLY!!!
typedef struct {
	CFVec3 Pos;
	CFVec3 Norm;
	CFColorRGBA Color;	// each element is a unit float
	CFVec2 aUV[MAX_LAYERS_PER_MAT];
	f32 fNumWeights;	// will be > 0.0f if the segment bSkinned == TRUE
	ApeWeight_t aWeights[MAX_WEIGHTS_PER_VERT];// the first nNumWeights elements will be used

	u8 nUnused[16];		// USE THESE BYTES TO ADD NEW FIELDS
} ApeVert_t;

//////////////////
// INDEX STRUCTURE
typedef struct {
	u32 nVertIndex;		// index into this segment's vertex array

	u8 nUnused[16];		// USE THESE BYTES TO ADD NEW FIELDS
} ApeVertIndex_t;

#endif

