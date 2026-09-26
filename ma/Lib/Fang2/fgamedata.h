//////////////////////////////////////////////////////////////////////////////////////
// fgamedata.h - 
//
// Author: Michael Starich   
//////////////////////////////////////////////////////////////////////////////////////
// THIS CODE IS PROPRIETARY PROPERTY OF SWINGIN' APE STUDIOS, INC.
// Copyright (c) 2001
//
// The contents of this file may not be disclosed to third
// parties, copied or duplicated in any form, in whole or in part,
// without the prior written permission of Swingin' Ape Studios, Inc.
//////////////////////////////////////////////////////////////////////////////////////
// Modification History:
//
// Date     Who         Description
// -------- ----------  --------------------------------------------------------------
// 07/30/01 Starich     Created.
//////////////////////////////////////////////////////////////////////////////////////
#ifndef _FGAME_DATA_H_
#define _FGAME_DATA_H_ 1

#include "fang.h"
#include "fmath.h"


typedef enum {
	FGAMEDATA_VAR_TYPE_STRING = 0,
	FGAMEDATA_VAR_TYPE_FLOAT,
	FGAMEDATA_VAR_TYPE_WIDESTRING,
	
	// This enum is stored in a u8, not to exceed 254 entries
	FGAMEDATA_VAR_TYPE_COUNT
} FGameData_VarType_e;


	enum {
		//////////////////
		// STRING DEFINES:
		//////////////////
		
		FGAMEDATA_FLAGS_STRING_RESERVED/*nDataType*/= 0x000000FF,
		FGAMEDATA_FLAGS_STRING_PTR_ONLY				= 0x00000100,	// fetch a pointer to the string value (will not copy the string)
		FGAMEDATA_FLAGS_STRING_FAIL_ON_OVERFLOW		= 0x00000200,	// when copying a string, if it is too long for the dest, fail
		FGAMEDATA_FLAGS_STRING_PTR_TO_MAIN_STR_TBL	= 0x00000400,	// copy string into "Main" string table and store the pointer
		FGAMEDATA_FLAGS_STRING_PTR_TO_SFX_HANDLE	= 0x00000800,	// convert the string ptr into a Sound Fx Handle (FSndFx_FxHandle_t) and store the handle
		FGAMEDATA_FLAGS_STRING_NONE_TO_NULL			= 0x00001000,	// if the string is "none", NULL is stored (case insensitive)
		FGAMEDATA_FLAGS_STRING_TO_TEXDEF			= 0x00002000,	// load the FTexDef_t and store a pointer to it
		FGAMEDATA_FLAGS_STRING_PTR_TO_FPART_HANDLE	= 0x00004000,	// convert the string ptr into a particle handle (FParticle_DefHandle_t) and store the handle
		FGAMEDATA_FLAGS_STRING_TO_MESH				= 0x00008000,	// load the FMesh_t and store a pointer to it
		FGAMEDATA_FLAGS_STRING_TO_DEBRIS_GROUP		= 0x00010000,	// stores a pointer to the debris group (CFDebrisGroup) whose name is in the data field
		FGAMEDATA_FLAGS_STRING_TO_DEBRIS_MESH_SET	= 0x00020000,	// stores a pointer to the debris mesh set (CFDebrisMeshSet) whose name is in the data field
		FGAMEDATA_FLAGS_STRING_TO_EXPLODE_GROUP		= 0x00040000,	// stores an explosion group handle (FExplosion_GroupHandle_t) in the data field
		FGAMEDATA_FLAGS_STRING_TO_SOUND_GROUP		= 0x00080000,	// stores a pointer to the sound group (CFSoundGroup) whose name is in the data field
		FGAMEDATA_FLAGS_STRING_TO_DAMAGE_PROFILE	= 0x00100000,	// stores a pointer to the game-specific damage profile whose name is in the data field
		FGAMEDATA_FLAGS_STRING_TO_ARMOR_PROFILE		= 0x00200000,	// stores a pointer to the game-specific armor profile whose name is in the data field
		FGAMEDATA_FLAGS_STRING_TO_MOTIF_BASE		= 0x00400000,	// stores the base motif index corresponding to the given motif name
		FGAMEDATA_FLAGS_STRING_TO_DECAL_DEF			= 0x00800000,	// stores a handle to the decal whose name is in the data field


		/////////////////
		// FLOAT DEFINES:
		/////////////////
		FGAMEDATA_FLAGS_FLOAT_RESERVED/*nDataType*/ = 0x000000FF,
		FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO			= 0x00000100,	// clamp x to a supplied min/max value before doing any conversions
		FGAMEDATA_FLAGS_FLOAT_CHECK_RANGE_AND_FAIL	= 0x00000200,	// check x to be in the supplied min/max range before doing any conversions, if outside of range fail
		///////////////////////////////////////////////////////////////////////////////////////////////////////
		// NOTE: WHEN MULTIPLE FLOAT VALUES ARE REQUESTED, THEY WILL BE 
		// SAVED OUT IN LOW TO HI BIT ORDER, EACH REQUIRING A f32.
		// FOR EXAMPLE, THE FOLLOWING MASK:
		//		( FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_UNIT_FLOAT | FGAMEDATA_FLAGS_FLOAT_SQRT_X )
		// WILL REQUIRE 3 f32s AND
		// f32[0] WILL BE SET TO X
		// f32[1] WILL BE SET TO A UNIT FLOAT OF X
		// f32[2] WILL BE SET TO SQRT( X )
		///////////////////////////////////////////////////////////////////////////////////////////////////////
		FGAMEDATA_FLAGS_FLOAT_X						= 0x00000400,	// fetch the original float value ( x )
		FGAMEDATA_FLAGS_FLOAT_OO_X					= 0x00000800,	// fetch ( 1.0f / x )
		FGAMEDATA_FLAGS_FLOAT_RADS_TO_DEGS			= 0x00001000,	// fetch ( FMATH_RAD2DEG( x )
		FGAMEDATA_FLAGS_FLOAT_DEGS_TO_RADS			= 0x00002000,	// fetch ( FMATH_DEG2RAD( x )
		FGAMEDATA_FLAGS_FLOAT_PERCENT_TO_UNIT_FLOAT = 0x00004000,	// fetch ( FMATH_CLAMP_UNIT_FLOAT( x / 100.0f ) ) 
		FGAMEDATA_FLAGS_FLOAT_UNIT_FLOAT			= 0x00008000,	// fetch ( (x - min) / (max - min) )
		FGAMEDATA_FLAGS_FLOAT_BI_POLAR_UNIT_FLOAT	= 0x00010000,	// fetch ( (((x - min)/(max - min)) - 0.5f) * 2.0f )
		FGAMEDATA_FLAGS_FLOAT_SQRT_X				= 0x00020000,	// fetch ( fmath_AcuSqrt( x ) )
		FGAMEDATA_FLAGS_FLOAT_OO_SQRT_X				= 0x00040000,	// fetch ( fmath_AcuInvSqrt( x ) )
		FGAMEDATA_FLAGS_FLOAT_SIN_DEGS_TO_RADS_X	= 0x00080000,	// fetch ( fmath_Sin( FMATH_DEG2RAD( x ) ) )
		FGAMEDATA_FLAGS_FLOAT_COS_DEGS_TO_RADS_X	= 0x00100000,	// fetch ( fmath_Cos( FMATH_DEG2RAD( x ) ) )
		FGAMEDATA_FLAGS_FLOAT_X_SQUARED				= 0x00200000,	// fetch ( FMATH_SQUARE( x ) )
		FGAMEDATA_FLAGS_FLOAT_X_CUBED				= 0x00400000,	// fetch ( FMATH_CUBE( x ) )
		FGAMEDATA_FLAGS_FLOAT_ABS_X					= 0x00800000,	// fetch ( FMATH_FABS( x ) )
		FGAMEDATA_FLAGS_FLOAT_OO_X_SQUARED			= 0x01000000,	// fetch ( 1.0f / FMATH_SQUARE( x ) )
		FGAMEDATA_FLAGS_CONVERT_TO_U32				= 0x02000000,	// fetch (u32)x
		FGAMEDATA_FLAGS_CONVERT_TO_S32				= 0x04000000,	// fetch (s32)x
		FGAMEDATA_FLAGS_FLOAT_UNCLAMPED_PERCENT_TO_UNIT_FLOAT = 0x08000000,	// fetch ( x / 100.0f ) 

		FGAMEDATA_FLAGS_FLOAT_REQUIRE_MIN_MAX		= ( FGAMEDATA_FLAGS_FLOAT_UNIT_FLOAT | 
														FGAMEDATA_FLAGS_FLOAT_BI_POLAR_UNIT_FLOAT |
														FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO |
														FGAMEDATA_FLAGS_FLOAT_CHECK_RANGE_AND_FAIL ),

		FGAMEDATA_FLAGS_FLOAT_ALL_POSSIBLE			= (	FGAMEDATA_FLAGS_FLOAT_X	|
														FGAMEDATA_FLAGS_FLOAT_OO_X |
														FGAMEDATA_FLAGS_FLOAT_RADS_TO_DEGS |
														FGAMEDATA_FLAGS_FLOAT_DEGS_TO_RADS |
														FGAMEDATA_FLAGS_FLOAT_PERCENT_TO_UNIT_FLOAT |
														FGAMEDATA_FLAGS_FLOAT_UNIT_FLOAT |
														FGAMEDATA_FLAGS_FLOAT_BI_POLAR_UNIT_FLOAT |
														FGAMEDATA_FLAGS_FLOAT_SQRT_X |
														FGAMEDATA_FLAGS_FLOAT_OO_SQRT_X |
														FGAMEDATA_FLAGS_FLOAT_SIN_DEGS_TO_RADS_X |
														FGAMEDATA_FLAGS_FLOAT_COS_DEGS_TO_RADS_X |
														FGAMEDATA_FLAGS_FLOAT_X_SQUARED |
														FGAMEDATA_FLAGS_FLOAT_X_CUBED |
														FGAMEDATA_FLAGS_FLOAT_ABS_X |
														FGAMEDATA_FLAGS_FLOAT_OO_X_SQUARED |
														FGAMEDATA_FLAGS_CONVERT_TO_U32 |
														FGAMEDATA_FLAGS_CONVERT_TO_S32 |
														FGAMEDATA_FLAGS_FLOAT_UNCLAMPED_PERCENT_TO_UNIT_FLOAT ),	
	};

typedef struct {
	u32 nFlags;		 // see FGAMEDATA_FLAGS_ // [31..24] are used/usable, rest reserved as Data Type:
	
	u16 nBytesForData;   // this number will vary, depending on how many f32s are requsted by the flags field or
						 // if a copy of a string is requested
	u8  uMinValueLookup; // the smallest valid number that the float can be equal to (ONLY USED IF (nFlags & FGAMEDATA_FLAGS_FLOAT_REQUIRE_MIN_MAX))
	u8  uMaxValueLookup; // the largest valid number that the float can be equal to (ONLY USED IF (nFlags & FGAMEDATA_FLAGS_FLOAT_REQUIRE_MIN_MAX))	
	
	u8 GetDataType( void ) const { return nFlags & FGAMEDATA_FLAGS_FLOAT_RESERVED; }
} FGameData_TableEntry_t;


typedef enum {
	F32_DATATABLE_0, 
	F32_DATATABLE_Pt0000001,
	F32_DATATABLE_POS_EPSILON = F32_DATATABLE_Pt0000001,
	F32_DATATABLE_Pt00001,
	F32_DATATABLE_Pt0001,
	F32_DATATABLE_Pt001,
	F32_DATATABLE_Pt01,
	F32_DATATABLE_Pt1,
	F32_DATATABLE_Pt95,
	F32_DATATABLE_Neg1,
	F32_DATATABLE_1,
	F32_DATATABLE_2,
	F32_DATATABLE_3,
	F32_DATATABLE_5,
	F32_DATATABLE_6,
	F32_DATATABLE_10,
	F32_DATATABLE_15,
	F32_DATATABLE_20,
	F32_DATATABLE_30,
	F32_DATATABLE_50,
	F32_DATATABLE_60,
	F32_DATATABLE_Neg100,
	F32_DATATABLE_100,
	F32_DATATABLE_255,
	F32_DATATABLE_500,
	F32_DATATABLE_Neg1000,
	F32_DATATABLE_1000,
	F32_DATATABLE_5000,
	F32_DATATABLE_Neg10000,
	F32_DATATABLE_10000,
	F32_DATATABLE_65534,
	F32_DATATABLE_65535,
	F32_DATATABLE_100000,
	F32_DATATABLE_10000000,
	F32_DATATABLE_Neg1000000000,
	F32_DATATABLE_1000000000,
	F32_DATATABLE_4294967295,
	F32_DATATABLE_MAX_UINT = F32_DATATABLE_4294967295,
	F32_DATATABLE_100000000000,
	F32_DATATABLE_COUNT
} FGameData_F32DataTable_e;

extern f32 afDataTable[F32_DATATABLE_COUNT];

typedef u32 FGameDataFileHandle_t;
#define FGAMEDATA_INVALID_FILE_HANDLE		0

typedef u32 FGameDataTableHandle_t;
#define FGAMEDATA_INVALID_TABLE_HANDLE		0

#define FGAMEDATA_MAX_RECURSION_DEPTH		8

typedef struct {
	FGameDataTableHandle_t hTable;
	s32 nFieldIndex;// positive numbers are indices into keyword tables, negatives are regular tables
} FGameDataStackEntry_t;

typedef struct {
	FGameDataStackEntry_t aStack[FGAMEDATA_MAX_RECURSION_DEPTH];
	u32 nNextStackIndex;	
} FGameDataWalker_t;


typedef struct {
	cchar *pszTableName;						// The table name (NULL=end of FGameDataMap_t array)
	const FGameData_TableEntry_t *pVocabTable;	// The vocabulary table (tells us how to interpret the table data)
	u32 nDestTableBytes;						// Number of bytes in the pDestTableData buffer
	void *pDestTableData;						// Points to the buffer that will hold the interpreted data
} FGameDataMap_t;



#define FGAMEDATA_VOCAB_U32_BOUND( uMin, uMax ) \
	FGAMEDATA_VAR_TYPE_FLOAT| \
	FGAMEDATA_FLAGS_CONVERT_TO_U32 | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO, \
	sizeof( u32 ), \
	uMin, \
	uMax


#define FGAMEDATA_VOCAB_F32_UNBOUND \
	FGAMEDATA_VAR_TYPE_FLOAT| \
	FGAMEDATA_FLAGS_FLOAT_X, \
	sizeof( f32 ), \
	F32_DATATABLE_0, \
	F32_DATATABLE_0


#define FGAMEDATA_VOCAB_F32_BOUND( uMin, uMax ) \
	FGAMEDATA_VAR_TYPE_FLOAT| \
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO, \
	sizeof( f32 ), \
	uMin, \
	uMax

#define FGAMEDATA_VOCAB_F32_BOUND_XOOX( uMin, uMax ) \
	FGAMEDATA_VAR_TYPE_FLOAT| \
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO|FGAMEDATA_FLAGS_FLOAT_OO_X, \
	sizeof( f32 )*2, \
	uMin, \
	uMax

#define FGAMEDATA_VOCAB_F32_BOUNDMIN( uMin ) \
	FGAMEDATA_VAR_TYPE_FLOAT| \
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO, \
	sizeof( f32 ), \
	uMin, \
	F32_DATATABLE_1000000000


#define FGAMEDATA_VOCAB_F32_BOUNDMAX( uMax ) \
	FGAMEDATA_VAR_TYPE_FLOAT| \
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO, \
	sizeof( f32 ), \
	F32_DATATABLE_Neg1000000000, \
	uMax


#define FGAMEDATA_VOCAB_F32_MIN0 \
	FGAMEDATA_VAR_TYPE_FLOAT| \
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO, \
	sizeof( f32 ), \
	F32_DATATABLE_0, \
	F32_DATATABLE_1000000000


#define FGAMEDATA_VOCAB_F32_MINEPSILON \
	FGAMEDATA_VAR_TYPE_FLOAT| \
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO, \
	sizeof( f32 ), \
	F32_DATATABLE_Pt001, \
	F32_DATATABLE_1000000000


#define FGAMEDATA_VOCAB_F32_UNIT \
	FGAMEDATA_VAR_TYPE_FLOAT| \
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO, \
	sizeof( f32 ), \
	F32_DATATABLE_0, \
	F32_DATATABLE_1


#define FGAMEDATA_VOCAB_F32_DEG2RAD \
	FGAMEDATA_VAR_TYPE_FLOAT| \
	FGAMEDATA_FLAGS_FLOAT_DEGS_TO_RADS, \
	sizeof( f32 ), \
	F32_DATATABLE_0, \
	F32_DATATABLE_0


#define FGAMEDATA_VOCAB_BOOL \
	FGAMEDATA_VAR_TYPE_FLOAT| \
	FGAMEDATA_FLAGS_CONVERT_TO_U32 | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO, \
	sizeof( BOOL ), \
	F32_DATATABLE_0, \
	F32_DATATABLE_1


#define FGAMEDATA_VOCAB_SOUND_GROUP \
	FGAMEDATA_VAR_TYPE_STRING| \
	FGAMEDATA_FLAGS_STRING_TO_SOUND_GROUP | FGAMEDATA_FLAGS_STRING_NONE_TO_NULL, \
	sizeof( CFSoundGroup * ), \
	F32_DATATABLE_0, \
	F32_DATATABLE_0


#define FGAMEDATA_VOCAB_EXPLODE_GROUP \
	FGAMEDATA_VAR_TYPE_STRING| \
	FGAMEDATA_FLAGS_STRING_TO_EXPLODE_GROUP | FGAMEDATA_FLAGS_STRING_NONE_TO_NULL, \
	sizeof( FExplosion_GroupHandle_t ), \
	F32_DATATABLE_0, \
	F32_DATATABLE_0


#define FGAMEDATA_VOCAB_SNDFX_HANDLE \
	FGAMEDATA_VAR_TYPE_STRING| \
	FGAMEDATA_FLAGS_STRING_PTR_TO_SFX_HANDLE | FGAMEDATA_FLAGS_STRING_NONE_TO_NULL, \
	sizeof( FSndFx_FxHandle_t ), \
	F32_DATATABLE_0, \
	F32_DATATABLE_0


#define FGAMEDATA_VOCAB_PARTICLE \
	FGAMEDATA_VAR_TYPE_STRING| \
	FGAMEDATA_FLAGS_STRING_PTR_TO_FPART_HANDLE | FGAMEDATA_FLAGS_STRING_NONE_TO_NULL, \
	sizeof( FParticle_DefHandle_t ), \
	F32_DATATABLE_0, \
	F32_DATATABLE_0


#define FGAMEDATA_VOCAB_DEBRIS_GROUP \
	FGAMEDATA_VAR_TYPE_STRING| \
	FGAMEDATA_FLAGS_STRING_TO_DEBRIS_GROUP | FGAMEDATA_FLAGS_STRING_NONE_TO_NULL, \
	sizeof( CFDebrisGroup * ), \
	F32_DATATABLE_0, \
	F32_DATATABLE_0


#define FGAMEDATA_VOCAB_DEBRIS_MESH_SET \
	FGAMEDATA_VAR_TYPE_STRING| \
	FGAMEDATA_FLAGS_STRING_TO_DEBRIS_MESH_SET | FGAMEDATA_FLAGS_STRING_NONE_TO_NULL, \
	sizeof( CFDebrisMeshSet * ), \
	F32_DATATABLE_0, \
	F32_DATATABLE_0


#define FGAMEDATA_VOCAB_MESH \
	FGAMEDATA_VAR_TYPE_STRING| \
	FGAMEDATA_FLAGS_STRING_TO_MESH | FGAMEDATA_FLAGS_STRING_NONE_TO_NULL, \
	sizeof( FMesh_t * ), \
	F32_DATATABLE_0, \
	F32_DATATABLE_0


#define FGAMEDATA_VOCAB_TEXDEF \
	FGAMEDATA_VAR_TYPE_STRING| \
	FGAMEDATA_FLAGS_STRING_TO_TEXDEF | FGAMEDATA_FLAGS_STRING_NONE_TO_NULL, \
	sizeof( FTexDef_t * ), \
	F32_DATATABLE_0, \
	F32_DATATABLE_0


#define FGAMEDATA_VOCAB_DAMAGE \
	FGAMEDATA_VAR_TYPE_STRING| \
	FGAMEDATA_FLAGS_STRING_TO_DAMAGE_PROFILE | FGAMEDATA_FLAGS_STRING_NONE_TO_NULL, \
	sizeof( void * ), \
	F32_DATATABLE_0, \
	F32_DATATABLE_0


#define FGAMEDATA_VOCAB_ARMOR \
	FGAMEDATA_VAR_TYPE_STRING| \
	FGAMEDATA_FLAGS_STRING_TO_ARMOR_PROFILE | FGAMEDATA_FLAGS_STRING_NONE_TO_NULL, \
	sizeof( void * ), \
	F32_DATATABLE_0, \
	F32_DATATABLE_0


#define FGAMEDATA_VOCAB_MOTIF \
	FGAMEDATA_VAR_TYPE_STRING| \
	FGAMEDATA_FLAGS_STRING_TO_MOTIF_BASE, \
	sizeof( u32 ), \
	F32_DATATABLE_0, \
	F32_DATATABLE_0

#define FGAMEDATA_VOCAB_DECAL_DEF \
	FGAMEDATA_VAR_TYPE_STRING| \
	FGAMEDATA_FLAGS_STRING_TO_DECAL_DEF, \
	sizeof( void* ), \
	F32_DATATABLE_0, \
	F32_DATATABLE_0



// Game-specific function that converts a string to a damage profile.
typedef void *FGameDataDamageConverterCallback_t( cchar *pszDamageProfile );
typedef void *FGameDataArmorConverterCallback_t( cchar *pszArmorProfile );


extern BOOL fgamedata_ModuleStartup( void );
extern void fgamedata_ModuleShutdown( void );

extern void fgamedata_SetDamageConverterCallback( FGameDataDamageConverterCallback_t *pFcnDamageConverter );
extern FGameDataDamageConverterCallback_t *fgamedata_GetDamageConverterCallback( void );

extern void fgamedata_SetArmorConverterCallback( FGameDataArmorConverterCallback_t *pFcnArmorConverter );
extern FGameDataArmorConverterCallback_t *fgamedata_GetArmorConverterCallback( void );


// called by the fres system when a fmem frame is released.
// DO NOT CALL DIRECTLY!!!
extern void fgamedata_MemFrameReleased( void *pReleasedMem );

// Sets and Gets the current keyword.  pszKeyword must point to a 
// persistant NULL terminated string.  When a keyword has been set
// tables whose name == pszkeyword will cause a recursive fgamedata_load
// of all string fields contained in that table.
extern void fgamedata_SetTableIncludeKeyword( cchar *pszKeyword );
extern cchar *fgamedata_GetTableIncludeKeyworld( void );

// Call this function to load the game data file into fmem memory.
// Returns a file handle, returns FGAMEDATA_INVALID_FILE_HANDLE
// if the file couldn't be loaded for some reason.
// It is up to the caller to get a fmem frame and release the frame to free the file
// Usage Note:
//		pszFilename can have an extension, for example filename.csv, or one will be added by the system
extern FGameDataFileHandle_t fgamedata_LoadFileToFMem( cchar *pszFileName );

// Call this function to obtain a handle from an already loaded data file.
// Returns a file handle, returns FGAMEDATA_INVALID_FILE_HANDLE
// if there was a problem fixing up the offsets for some reason.
extern FGameDataFileHandle_t fgamedata_GetHandleFromLoadedFile( void *pLoadedFile );

// Call this function to find out the filename of hFileHandle.
// Returns a pointer to the filename, NULL if there was a problem or 
// hFileHandle was the result of calling fgamedata_GetHandleFromLoadedFile().
extern cchar *fgamedata_GetFileNameFromFileHandle( FGameDataFileHandle_t hFileHandle );

// Call this function to find the first (or last) table with the name pszTableName.
// Returns FGAMEDATA_INVALID_TABLE_HANDLE if there was a problem or the table doesn't exist.
extern FGameDataTableHandle_t fgamedata_GetFirstTableHandle( FGameDataFileHandle_t hFileHandle, cchar *pszTableName );
extern FGameDataTableHandle_t fgamedata_GetLastTableHandle( FGameDataFileHandle_t hFileHandle, cchar *pszTableName );

// Call this function to find out the filename of the file containing
// hTableHandle.  Returns a pointer to the filename, NULL if there was 
// a problem or hTableHandle was the result of calling gamedata_GetHandleFromLoadedFile().
extern cchar *fgamedata_GetFileNameFromTableHandle( FGameDataTableHandle_t hTableHandle );

// Call this function to retrieve the table name for the hTableHandle table.
// Returns the table name or NULL if there was a problem.
extern cchar *fgamedata_GetTableName( FGameDataTableHandle_t hTableHandle );

// Call this function to retrieve the number of fields contained in the
// table pointed to by hTableHandle.
extern u32 fgamedata_GetNumFields( FGameDataTableHandle_t hTableHandle );

// Call this function if you have a template of data you would like to retrive from a data table.
// Simply fill in an array of FGameData_TableEntry_t will the desired dest format, allocate some
// memory to store the data, and call this function.
// Returns a non-NULL ptr if everything was ok, NULL if there was a problem (there are many DEVPRINTFs that report all errors)
extern const void *fgamedata_GetPtrToFieldData( FGameDataTableHandle_t hTableHandle, u32 nFieldIndex,
											    FGameData_VarType_e &rnDataType );

// Call this function to retrive a particular field from a table.
// Returns TRUE if pDest was updated ok, FALSE if there was a problem (there are many DEVPRINTFs that report all errors)
extern BOOL fgamedata_GetFieldFromTable( FGameDataTableHandle_t hTableHandle, u32 nFieldIndex,
										 const FGameData_TableEntry_t *pTableEntry, void *pDest );

// Fills pDest like fgamedata_GetTableData(), but reads vocabulary entry i from table field
// panFieldIndex[i]. Entries mapped to -1 are skipped and their destination bytes are left
// unchanged. For newer data tables whose fields were inserted or removed relative to the
// vocabulary. The map must have one index per entry and the entries must fill nNumDestBytes.
extern BOOL fgamedata_GetTableDataRemapped( FGameDataTableHandle_t hTableHandle, const FGameData_TableEntry_t *paTableEntries,
											const s8 *panFieldIndex, u32 nFieldIndexCount, void *pDest, u32 nNumDestBytes );

// Call this function to obtain a const ptr to a field's data.  rnDataType will be filled in 
// with the field's FGameData_VarType_e so the caller will know how to cast the returned ptr.
// Returns TRUE if table was created ok, FALSE if there was a problem (there are many DEVPRINTFs that report all errors)
extern BOOL fgamedata_GetTableData( FGameDataTableHandle_t hTableHandle, 
								    const FGameData_TableEntry_t *paTableEntries, u32 nNumTableEntries,
									void *pDest, u32 nNumDestBytes, u32 nFieldStartIndex=0 );

// A version of the previous function that's much easier to use.
extern BOOL fgamedata_GetTableData( FGameDataTableHandle_t hTableHandle, 
								    const FGameData_TableEntry_t *paTableEntries,
									void *pDest, u32 nNumDestBytes, u32 nFieldStartIndex=0 );

extern BOOL fgamedata_GetTableData_ArrayOfStructures( FGameDataTableHandle_t hTableHandle, 
								    const FGameData_TableEntry_t *paTableEntries,
									void *pDest, u32 nNumDestBytesPerElement, u32 nArrayElementCount );

extern FGameDataTableHandle_t fgamedata_GetFirstTable( FGameDataFileHandle_t hFileHandle, FGameDataWalker_t &rWalker );
extern FGameDataTableHandle_t fgamedata_GetNextTable( FGameDataWalker_t &rWalker );

extern BOOL fgamedata_ReadFileUsingMap( const FGameDataMap_t *pMapTable, cchar *pszFileName );



#endif

