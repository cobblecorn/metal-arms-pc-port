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
#ifndef _OLD_APE_FILE_DEF_H_
#define _OLD_APE_FILE_DEF_H_ 1

#include "fang.h"
#include "fmath.h"
#include "fcolor.h"
#include "fversion.h"

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

#define OLD_TEXTURE_NAME_LEN			16
#define OLD_LIGHT_NAME_LEN				16
#define OLD_SEGMENT_NAME_LEN			16
#define OLD_MESH_NAME_LEN				16
#define OLD_BONE_NAME_LEN				32
#define OLD_OBJECT_NAME_LEN				16
#define OLD_MAX_LAYERS_PER_MAT			4
#define OLD_MAX_WEIGHTS_PER_VERT		4
#define OLD_MAX_WEIGHTS_PER_FACE		4
#define OLD_MAX_CHILDREN_PER_BONE		64
#define OLD_MAX_TOTAL_BONE_COUNT		200

#define OLD_MAX_VIS_CELLS_PER_VOL		16
#define OLD_MAX_VIS_FACES_PER_CELL		26
#define OLD_MAX_VIS_DEGREES_PER_FACE	6
#define OLD_MAX_VIS_PTS_PER_CELL		( OLD_MAX_VIS_FACES_PER_CELL * OLD_MAX_VIS_DEGREES_PER_FACE )
#define OLD_MAX_VIS_EDGES_PER_CELL		( (OLD_MAX_VIS_PTS_PER_CELL >> 1) + 1 )
#define OLD_VIS_NAME_LEN				32

#define OLD_ON_PLANE_EPSILON			( 0.01f )
#define OLD_ON_PLANE_EPSILON_2			( OLD_ON_PLANE_EPSILON * OLD_ON_PLANE_EPSILON )

typedef enum {
	OLD_APE_LIGHT_TYPE_SPOT = 0,
	OLD_APE_LIGHT_TYPE_OMNI,
	OLD_APE_LIGHT_TYPE_DIR,
	OLD_APE_LIGHT_TYPE_AMBIENT,

	OLD_APE_LIGHT_TYPE_COUNT
} OldApeLightType_e;

enum {
	OLD_APE_LIGHT_FLAG_DONT_USE_RGB				= 0x00000001,	// Disregard the light's rgb and only use the motif's color (default = off)
	OLD_APE_LIGHT_FLAG_LIGHT_SELF				= 0x00000002,	// Light the object that the light is attached to (default = off)
	OLD_APE_LIGHT_FLAG_OBJ_DONT_LIGHT_TERRAIN	= 0x00000004,	// Lights attached to this object don't light the terrain (default = off)

	OLD_APE_LIGHT_FLAG_PER_PIXEL				= 0x00000008,		// This light casts a projection on the environment (may or may not have a texture)

	OLD_APE_LIGHT_FLAG_LIGHTMAP_ONLY_LIGHT		= 0x00000010,		// This light will only be used in the lightmap portion of PASM and will not be exported to the engine.
	OLD_APE_LIGHT_FLAG_LIGHTMAP_LIGHT			= 0x00000020,		// This light is to be used for generating lightmaps (If it is not dynamic, it can be discarded prior to the engine)
	OLD_APE_LIGHT_FLAG_UNIQUE_LIGHTMAP			= 0x00000040,		// This light will generate its own unique lightmap in the lightmapping phase (it must also have a unique m_nLightID)

	OLD_APE_LIGHT_FLAG_CORONA					= 0x00000080,		// This light has a corona
	OLD_APE_LIGHT_FLAG_CORONA_PROXFADE			= 0x00000100,		// Fade the corona as the camera gets closer.

	OLD_APE_LIGHT_FLAG_NONE						= 0x00000000
};

enum {
	OLD_APE_OB_FLAG_STATIC				= 0x00000001,	// Object's bounding sphere has a static footprint in 3D space
	OLD_APE_OB_FLAG_POSTER_Y			= 0x00000002,	// Poster object around it's Y axis to always face the camera (neg-Z axis of object toward viewer)
	OLD_APE_OB_FLAG_NO_COLL				= 0x00000004,	// Don't collide with this object
	OLD_APE_OB_FLAG_NO_FOG				= 0x00000008,	// Don't fog this object
	OLD_APE_OB_FLAG_NO_LIGHT			= 0x00000010,	// Don't light this object
	OLD_APE_OB_FLAG_SORT_AFTER_TERRAIN	= 0x00000020,	// Draw sorted after the terrain
	OLD_APE_OB_FLAG_POSTER_X			= 0x00000040,	// Poster object around it's X axis
	OLD_APE_OB_FLAG_POSTER_Z			= 0x00000080,	// Poster object around it's Z axis
	OLD_APE_OB_FLAG_NO_DRAW				= 0x00000100,
	OLD_APE_OB_FLAG_NO_LM				= 0x00000200,	// This (static) object will not generate light maps
	OLD_APE_OB_FLAG_PER_PIXEL			= 0x00000400,
	
	OLD_APE_OB_FLAG_NONE				= 0x00000000
};

enum {
	OLD_APE_MAT_FLAGS_NO_DRAW			= 0x00000001,

	OLD_APE_MAT_FLAGS_NONE				= 0x00000000
};

enum {
	OLD_APE_MAT_COLL_FLAGS_COLL_WITH_PLAYER				= 0x0001,
	OLD_APE_MAT_COLL_FLAGS_COLL_WITH_NPCS				= 0x0002,
	OLD_APE_MAT_COLL_FLAGS_OBSTRUCT_LINE_OF_SIGHT		= 0x0004,
	OLD_APE_MAT_COLL_FLAGS_COLL_WITH_THIN_PROJECTILES	= 0x0008,
	OLD_APE_MAT_COLL_FLAGS_COLL_WITH_THICK_PROJECTILTES = 0x0010,
	OLD_APE_MAT_COLL_FLAGS_COLL_WITH_CAMERA				= 0x0020,
	OLD_APE_MAT_COLL_FLAGS_COLL_WITH_OBJECTS			= 0x0040,
	OLD_APE_MAT_COLL_FLAGS_WALKABLE						= 0x0080,
	OLD_APE_MAT_COLL_FLAGS_OBSTRUCT_SPLASH_DAMAGE		= 0x0100,

	OLD_APE_MAT_COLL_FLAGS_COLL_WITH_NOTHING			= 0x0000,
	OLD_APE_MAT_COLL_FLAGS_COLL_WITH_EVERYTHING			= 0xFFFF
};

enum {
	OLD_APE_LAYER_FLAGS_USE_SPECULAR_TEXMAP				= ( 1 << 0 ),
	OLD_APE_LAYER_FLAGS_USE_SPECULAR_RED_CHANNEL		= ( 1 << 1 ),
	OLD_APE_LAYER_FLAGS_USE_SPECULAR_GREEN_CHANNEL		= ( 1 << 2 ),
	OLD_APE_LAYER_FLAGS_USE_SPECULAR_BLUE_CHANNEL		= ( 1 << 3 ),
	OLD_APE_LAYER_FLAGS_USE_SPECULAR_ALPHA_CHANNEL		= ( 1 << 4 ),

	OLD_APE_LAYER_FLAGS_USE_SELF_ILLUM_TEXMAP			= ( 1 << 5 ),
	OLD_APE_LAYER_FLAGS_USE_SELF_ILLUM_RED_CHANNEL		= ( 1 << 6 ),
	OLD_APE_LAYER_FLAGS_USE_SELF_ILLUM_GREEN_CHANNEL	= ( 1 << 7 ),
	OLD_APE_LAYER_FLAGS_USE_SELF_ILLUM_BLUE_CHANNEL		= ( 1 << 8 ),
	OLD_APE_LAYER_FLAGS_USE_SELF_ILLUM_ALPHA_CHANNEL	= ( 1 << 9 ),

	OLD_APE_LAYER_FLAGS_USE_OPACITY_TEXMAP				= ( 1 << 10 ),
	OLD_APE_LAYER_FLAGS_USE_OPACITY_RED_CHANNEL			= ( 1 << 11 ),
	OLD_APE_LAYER_FLAGS_USE_OPACITY_GREEN_CHANNEL		= ( 1 << 12 ),
	OLD_APE_LAYER_FLAGS_USE_OPACITY_BLUE_CHANNEL		= ( 1 << 13 ),
	OLD_APE_LAYER_FLAGS_USE_OPACITY_ALPHA_CHANNEL		= ( 1 << 14 ),

	OLD_APE_LAYER_FLAGS_USE_BUMP_TEXMAP					= ( 1 << 15 ),
	
	OLD_APE_LAYER_FLAGS_USE_REFLECTIVITY_TEXMAP			= ( 1 << 16 ),
	OLD_APE_LAYER_FLAGS_USE_REFLECTIVITY_RED_CHANNEL	= ( 1 << 17 ),
	OLD_APE_LAYER_FLAGS_USE_REFLECTIVITY_GREEN_CHANNEL	= ( 1 << 18 ),
	OLD_APE_LAYER_FLAGS_USE_REFLECTIVITY_BLUE_CHANNEL	= ( 1 << 19 ),
	OLD_APE_LAYER_FLAGS_USE_REFLECTIVITY_ALPHA_CHANNEL	= ( 1 << 20 ),

	OLD_APE_LAYER_FLAGS_USE_DISPLACEMENT_TEXMAP			= ( 1 << 21 ),

	OLD_APE_LAYER_FLAGS_USE_NO_ADDITIONAL_TEXMAPS		= 0x00000000,
};

enum {
	OLD_APE_BONE_FLAG_NOT_USED			= 0x00000001,	// this bone is not referenced by any face in the mesh
	
	OLD_APE_BONE_FLAG_NONE				= 0x00000000
};

enum {
	OLD_APE_PORTAL_FLAG_MIRROR			= 0x00000001,	// the portal is a mirror
	OLD_APE_PORTAL_FLAG_SOUND_ONLY		= 0x00000002,	// the portal is for sound only
	OLD_APE_PORTAL_FLAG_ONE_WAY			= 0x00000004,	// the portal is 1 way
	OLD_APE_PORTAL_FLAG_ANTI			= 0x00000008,	// the portal is an anti portal

	OLD_APE_PORTAL_FLAG_NONE			= 0x00000000
};

typedef enum {
	OLD_APE_SHAPE_TYPE_SPHERE = 0,
	OLD_APE_SHAPE_TYPE_CYLINDER,
	OLD_APE_SHAPE_TYPE_BOX,
	OLD_APE_SHAPE_TYPE_CAMERA,
	OLD_APE_SHAPE_TYPE_SPEAKER,
	OLD_APE_SHAPE_TYPE_SPAWN_POINT,	// Used for AI
	OLD_APE_SHAPE_TYPE_START_POINT,	// Used for placing the human player
	OLD_APE_SHAPE_TYPE_ROOM,		// Used to define rooms
	OLD_APE_SHAPE_TYPE_ARENA,		// Used to define arena's within rooms	
	OLD_APE_SHAPE_TYPE_PARTICLE_BOX,
	OLD_APE_SHAPE_TYPE_PARTICLE_SPHERE,
	OLD_APE_SHAPE_TYPE_PARTICLE_CYLINDER,
	OLD_APE_SHAPE_TYPE_SPLINE,

	OLD_APE_SHAPE_TYPE_COUNT
} OldApeShapeType_e;

typedef enum {
	// 1 layer:
	OLD_APE_SHADER_TYPE_oBASE = 0,	//0 specular, & emissive versions too	(4 versions)
	OLD_APE_SHADER_TYPE_cBASE,		//1	specular, & emissive versions too	(4 versions)	
	OLD_APE_SHADER_TYPE_tBASE,		//2	specular, & emissive versions too	(4 versions)
	OLD_APE_SHADER_TYPE_obsBASE,	//3 emissive version too				(2 versions)
	OLD_APE_SHADER_TYPE_beoBASE,	//4 specular version too				(2 versions)
	OLD_APE_SHADER_TYPE_etbsBASE,	//5										(1 version)
	// 2 layers:
	OLD_APE_SHADER_TYPE_oBASE_LERP_tLAYER,//6 specular version too			(2 versions)
	OLD_APE_SHADER_TYPE_cBASE_LERP_tLAYER,//7 specular version too			(2 versions)
	OLD_APE_SHADER_TYPE_tBASE_LERP_tLAYER,//8 specular version too			(2 versions)
	
	OLD_APE_SHADER_TYPE_oBASE_LERP_vLAYER,//9								(1 version)
	OLD_APE_SHADER_TYPE_cBASE_LERP_vLAYER,//10								(1 version)
	OLD_APE_SHADER_TYPE_oBASE_LERP_pLAYER,//11								(1 version)	
	OLD_APE_SHADER_TYPE_cBASE_LERP_pLAYER,//12								(1 version)
	
	OLD_APE_SHADER_TYPE_oBASE_MOD_SHADOWMAP,//13 specular version too		(2 versions)
	OLD_APE_SHADER_TYPE_cBASE_MOD_SHADOWMAP,//14 specular version too		(2 versions)

	OLD_APE_SHADER_TYPE_oBASE_ADD_rbENV,//15								(1 version)
	OLD_APE_SHADER_TYPE_etBASE_ADD_rbENV,//16								(1 version)

	OLD_APE_SHADER_TYPE_oBASE_ADD_rbENV_MOD_SHADOWMAP,//17					(1 version)
	
	OLD_APE_SHADER_TYPE_ADD_BASE,//18										(1 version)

} OldApeShaderType_e;

//////////////
// MESH HEADER
typedef struct {
	FVersionHeader_t Header;// has a signature and version info
	u32 nBytesInFile;	// what is the total file size of the entire .ape file
	char szMeshName[OLD_MESH_NAME_LEN];
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
} OldApeMesh_t;

/////////////////
// BONE STRUCTURE
typedef struct {
	char szBoneName[OLD_BONE_NAME_LEN];	// what is the name of this bone, THIS MUST MATCH THE NAME IN THE ANIMATION FILE!!!
	u32 nFlags;						// see OLD_APE_BONE_FLAG_* for info
	u32 nBoneIndex;					// what bone index is this
	s32 nParentIndex;				// -1 = no parent
	CFMtx43 AtRestModelToBoneMtx;	// LEFT HANDED COORDINATE SYSTEM!!!
	u32 nNumChildren;				// how many children does this bone have
	u8 auChildIndices[OLD_MAX_CHILDREN_PER_BONE];// only the first nNumChildren are used
		
	u8 nUnused[16];					// USE THESE BYTES TO ADD NEW FIELDS
} OldApeBone_t;

//////////////////
// LIGHT STRUCTURE
typedef struct {
	OldApeLightType_e nType;// see OLD_APE_LIGHT_TYPE_* for info	
	char szLightName[OLD_LIGHT_NAME_LEN];
	CFSphere Sphere;
	CFVec3 Dir;
	CFColorRGB Color;	// unit floats
	f32 fIntensity;
	f32 fSpotInnerAngle;// in radians, the full angle
	f32 fSpotOuterAngle;// in radians, the full angle
	u32 nFlags;			// see OLD_APE_LIGHT_FLAG_* for info
	u32 nMotifID;		// what is the light's motif ID (default = 0)
	u16 nLightID;

	u8 nUnused[62];
} OldApeLight_t;

///////////////////
// OBJECT STRUCTURE
typedef struct {
	char szName[OLD_OBJECT_NAME_LEN];
	u32 nFlags;			// See OLD_APE_OB_FLAG_* for info
	CFMtx43 Orientation;// a left handed orientation, Rot, Pos, & Scale
	u32 nBytesOfUserData;// how many bytes follow this struct
	f32 fCullDistance;	// how many meters way from the camera should this object no longer be drawn
	u32 nParentIndex;	// are we attached to a parent? (either object or shape) (0 = none, nParentIndex-1 = index into combined obj/shape array)

	u8 nUnused[32];		// USE THESE BYTES TO ADD NEW FIELDS
} OldApeObject_t;

////////////////
// FOG STRUCTURE
typedef struct {
	f32 fStartDist;		// at what distance should the fog start? (meters)
	f32 fEndDist;		// at what distance should the fog end?	(meters)
	f32 fUnitDensity;	// what is the max density
	CFColorRGB Color;	// what color is the fog
	u32 nMotifID;		// what is the fog's motif ID

	u8 nUnused[16];		// USE THESE BYTES TO ADD NEW FIELDS
} OldApeFog_t;

//////////////////
// SHAPE STRUCTURE


// the following structs are used by the different shape types
// IF ANY STRUCT IS MORE THAN 16 BYTES, YOU MUST INCREASE THE APE FILE VERSION!!!
typedef struct {
	u8 anUnused[4];
	f32 fLength;	// our z, if no rotation
	f32 fWidth;		// our x, if no rotation
	f32 fHeight;	// our y, if no rotation	
} OldApeBoxShape_t;	// nType == OLD_APE_SHAPE_TYPE_BOX

typedef struct {
	f32 fRadius;
	u8 anUnused[12];
} OldApeSphereShape_t;	// nType == OLD_APE_SHAPE_TYPE_SPHERE

typedef struct {
	f32 fRadius;
	f32 fHeight;	// our y, if no rotation
	u8 anUnused[8];
} OldApeCylinderShape_t;// nType == OLD_APE_SHAPE_TYPE_CYLINDER

typedef struct {
	f32 fFOV;		// in radians, the full angle
	u32 nFrames;	// how many animation frames are there?
	u32 nOffsetToFrames;// offset into the user data buffer to the first frame
	u32 nOffsetToString;// offset into the user data buffer to the string data
} OldApeCameraShape_t;	// nType == OLD_APE_SHAPE_TYPE_CAMERA

typedef struct {
	f32 fRadius;
	f32 fUnitVolume;
	u8 anUnused[8];
} OldApeSpeakerShape_t;// nType == OLD_APE_SHAPE_TYPE_SPEAKER

typedef struct {
	u8 anUnused[16];
} OldApeSpawnPointShape_t;// nType == OLD_APE_SHAPE_TYPE_SPAWN_POINT

typedef struct {
	u8 anUnused[16];
} OldApeStartPointShape_t;	// nType == OLD_APE_SHAPE_TYPE_START_POINT

typedef struct {
	f32 fLength;	// our z, if no rotation
	f32 fWidth;		// our x, if no rotation
	f32 fHeight;	// our y, if no rotation	
	u32 nRoomID;
} OldApeRoomShape_t;	// nType == OLD_APE_SHAPE_TYPE_ROOM

typedef struct {
	f32 fLength;	// our z, if no rotation
	f32 fWidth;		// our x, if no rotation
	f32 fHeight;	// our y, if no rotation	
	u8 anUnused[4];	
} OldApeArenaShape_t;	// nType == OLD_APE_SHAPE_TYPE_ARENA

typedef struct {
	f32 fLength;	// our z, if no rotation
	f32 fWidth;		// our x, if no rotation
	f32 fHeight;	// our y, if no rotation	
	u8 anUnused[4];
} OldApeParticleBoxShape_t;// nType == OLD_APE_SHAPE_TYPE_PARTICLE_BOX

typedef struct {
	f32 fRadius;
	u8 anUnused[12];
} OldApeParticleSphereShape_t;// nType == OLD_APE_SHAPE_TYPE_PARTICLE_SPHERE

typedef struct {
	f32 fRadius;
	f32 fHeight;	// our y, if no rotation
	u8 anUnused[8];
} OldApeParticleCylinderShape_t;// nType == OLD_APE_SHAPE_TYPE_PARTICLE_CYLINDER

typedef struct {
	u32 nNumPts;	// how many ApeSplinePt_t structs are contained in the user data (this array is the 1st data in the user props)
	BOOL bClosed;	// connect the last pt and the first pt?
	u32 nNumSegments;// depending on bClosed, will either be (nNumPts - 1) or (nNumPts)
	u8 anUnused[4];
} OldApeSplineShape_t;	// nType == OLD_APE_SHAPE_TYPE_SPLINE

typedef struct {
	OldApeShapeType_e nType;// see OLD_APE_SHAPE_TYPE_* for info

	// you can add structs to this union,
	// IF ANY STRUCT IS MORE THAN 16 BYTES, YOU MUST INCREASE THE APE FILE VERSION!!!
	union {
		OldApeBoxShape_t				Box;
		OldApeSphereShape_t			Sphere;
		OldApeCylinderShape_t			Cylinder;
		OldApeCameraShape_t			Camera;
		OldApeSpeakerShape_t			Speaker;
		OldApeSpawnPointShape_t		SpawnPt;
		OldApeStartPointShape_t		StartPt;	
		OldApeRoomShape_t				Room;
		OldApeArenaShape_t				Arena;
		OldApeParticleBoxShape_t		ParticleBox;
		OldApeParticleSphereShape_t	ParticleSphere;
		OldApeParticleCylinderShape_t	ParticleCylinder;
		OldApeSplineShape_t			Spline;
	} TypeData;
	CFMtx43 Orientation;	// a left handed orientation, Rot, Pos, & Scale
	u32 nBytesOfUserData;	// how many bytes follow this struct
	u32 nParentIndex;		// are we attached to a parent? (either object or shape) (0 = none, nParentIndex-1 = index into combined obj/shape array)

	u8 nUnused[12];			// USE THESE BYTES TO ADD NEW FIELDS
} OldApeShape_t;

// used by an ApeShape_t (nType == OLD_APE_SHAPE_TYPE_CAMERA)
typedef struct {
	f32 fSecsFromStart;
	f32 fFOV;
} OldApeCamFrame_t;

// used by an ApeSplineShape_t (nType == OLD_APE_SHAPE_TYPE_SPLINE)
typedef struct {
	CFVec3 Pos;// in left handed world space
} OldApeSplinePt_t;

/////////////////
// VIS STRUCTURES

typedef struct {
	CFVec3 Pos;			// in left handed world space
} OldApeVisPt_t;

typedef struct {
	u16 anPtIndices[2];		// indices into the pt array
	
	u32 nNumFaces;			// can only be 0, 1, or 2
	u16 anFaceIndices[2];	// indices into the face array (nNumFaces of are used)

	u8 nUnused[4];		// USE THESE BYTES TO ADD NEW FIELDS
} OldApeVisEdges_t;

typedef struct {
	u32 nDegree;		// the degree is the number of points and edges
	u16 aPtIndices[OLD_MAX_VIS_DEGREES_PER_FACE];// nDegree are used
	u16 aEdgeIndices[OLD_MAX_VIS_DEGREES_PER_FACE];// nDegree are used
	
	CFVec3 Normal;		// face outward of the cell
	CFVec3 Centroid;	// a central point on the face

	u8 nUnused[4];		// USE THESE BYTES TO ADD NEW FIELDS	
} OldApeVisFaces_t;

typedef struct {
	char szName[OLD_VIS_NAME_LEN];// a name so that this particular portal can be idenitifed

	u32 nNumPts;
	OldApeVisPt_t aPts[OLD_MAX_VIS_PTS_PER_CELL];// nNumPts are used

	u32 nNumEdges;
	OldApeVisEdges_t aEdges[OLD_MAX_VIS_EDGES_PER_CELL];// nNumEdges are used

	u32 nNumFaces;
	OldApeVisFaces_t aFaces[OLD_MAX_VIS_FACES_PER_CELL];// nNumFaces are used

	CFSphere Sphere;

	u32 nFlags;

	u8 nUnused[16];		// USE THESE BYTES TO ADD NEW FIELDS
} OldApeVisCell_t;

typedef struct {
	u32 nNumCells;
	OldApeVisCell_t aCells[OLD_MAX_VIS_CELLS_PER_VOL];// nNumCells are used

	CFSphere Sphere;

	u32 nFlags;

	u8 nUnused[16];		// USE THESE BYTES TO ADD NEW FIELDS
} OldApeVisVolume_t;

typedef struct {
	char szName[OLD_VIS_NAME_LEN];// a name so that this particular portal can be idenitifed

	CFVec3 aCorners[4];
	CFVec3 Normal;
	CFVec3 Centroid;

	u32 nFlags;			// see OLD_APE_PORTAL_FLAG_* for details

	u8 nUnused[16];		// USE THESE BYTES TO ADD NEW FIELDS	
} OldApeVisPortal_t;

////////////////////
// SEGMENT STRUCTURE
typedef struct {
	char szNodename[OLD_SEGMENT_NAME_LEN];
	BOOL bSkinned;		// all of the tris in this segment are skinned
	u32 nNumMaterials;	// how many ApeMaterial_t structures follow this structure	
	u32 nNumVerts;		// how many ApeVert_t structures follow the last material structure
	u32 nNumIndices;	// how many ApeVertIndex_t structure follow the last vert structure	

	u8 nUnused[16];		// USE THESE BYTES TO ADD NEW FIELDS
} OldApeSegment_t;

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

	u8 nFlags;			// see OLD_APE_MAT_FLAGS_
	u16 nCollMask;		// see OLD_APE_MAT_COLL_FLAGS_, default OLD_APE_MAT_COLL_FLAGS_COLL_WITH_EVERYTHING
	u16 nReactType;		// 0 - 7, default 0
	u16 nSurfaceType;	// 0 - 15, default 0

	//////////////////////////////////////////////////////////////////////////////
	// MAKE SURE THAT IF YOU ADD FIELDS YOU UPDATE PASM'S MAT COMPRESSION ROUTINES
	//////////////////////////////////////////////////////////////////////////////

	u8 nUnused[32];			// USE THESE BYTES TO ADD NEW FIELDS
} OldApeCommands_t;

//////////////////
// LAYER STRUCTURE
typedef struct {
	BOOL bTextured;// does this layer have a valid texture map assigned to it
	char szTexname[OLD_TEXTURE_NAME_LEN];// no path or extension is included
	f32 fUnitAlphaMultiplier;// unit float, amount to scale vertex alpha when using some shader types
	BOOL8 bDrawAsWire;
	BOOL8 bTwoSided;		// tells PASM to generate 2 tris
	BOOL8 abTile[2];		// [0] is U, [1] is V
	CFColorRGB SpecularRGB;	// each element is a unit float
	CFColorRGB SelfIllumRGB;// aka emmissiveness, each element is a unit float
	CFColorRGB DiffuseRGB;	// each element is a unit float
	f32 fShininess;			// not a unit f32, represents an exponent, range(0.0f - 127.0f)
	f32 fShinStr;			// unit f32 (used to scale the specualar color)
	OldApeCommands_t StarCommands;
	u32 nFlags;				// see OLD_APE_LAYER_FLAGS_USE_...

	// no path or extension is included for any of the texnames
	char szSpecularTexname[OLD_TEXTURE_NAME_LEN];		// check the flags for what channel to use (default A)
	char szSelfIllumTexname[OLD_TEXTURE_NAME_LEN];		// check the flags for what channel to use (default A)
	char szOpacityTexname[OLD_TEXTURE_NAME_LEN];		// check the flags for what channel to use (default A)
	char szBumpTexname[OLD_TEXTURE_NAME_LEN];			// uses RGB
	char szReflectivityTexname[OLD_TEXTURE_NAME_LEN];	// check the flags for what channel to use (default A)
	char szDisplacementTexname[OLD_TEXTURE_NAME_LEN];	// uses RGB
	
	u8 nUnused[32];			// USE THESE BYTES TO ADD NEW FIELDS
} OldApeLayer_t;

/////////////////////
// MATERIAL STRUCTURE
typedef struct {
	u32 nLayersUsed;// how many aLayer[] are used
	OldApeLayer_t aLayer[OLD_MAX_LAYERS_PER_MAT];	// [0] represents the base and each index works upward 
											// to the last index which is the topmost layer
	u32 nFirstIndex;// what is this material's first ApeVertIndex_t index into the segment's index array
	u32 nNumIndices;// how many ApeVertIndex_t structures does this material use
	OldApeCommands_t StarCommands;

	u8 nUnused[32];	// USE THESE BYTES TO ADD NEW FIELDS
} OldApeMaterial_t;

///////////////////
// WEIGHT STRUCTURE
typedef struct {
	f32 fBoneIndex;	// what bone index does fWeight represent
	f32 fWeight;	// unit float

	u8 nUnused[16];	// USE THESE BYTES TO ADD NEW FIELDS
} OldApeWeight_t;

/////////////////
// VERT STRUCTURE

// KEEP THIS STRUCTURE'S ELEMENTS ALL FLOATING POINT SO THAT OUR HASH FUNCTION WORKS PERFECTLY!!!
typedef struct {
	CFVec3 Pos;
	CFVec3 Norm;
	CFColorRGBA Color;	// each element is a unit float
	CFVec2 aUV[OLD_MAX_LAYERS_PER_MAT];
	f32 fNumWeights;	// will be > 0.0f if the segment bSkinned == TRUE
	OldApeWeight_t aWeights[OLD_MAX_WEIGHTS_PER_VERT];// the first nNumWeights elements will be used

	u8 nUnused[16];		// USE THESE BYTES TO ADD NEW FIELDS
} OldApeVert_t;

//////////////////
// INDEX STRUCTURE
typedef struct {
	u32 nVertIndex;		// index into this segment's vertex array

	u8 nUnused[16];		// USE THESE BYTES TO ADD NEW FIELDS
} OldApeVertIndex_t;

#endif

