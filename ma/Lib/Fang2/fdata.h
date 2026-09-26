//////////////////////////////////////////////////////////////////////////////////////
// fdata.h - 
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

#ifndef _FDATA_H_
#define _FDATA_H_ 1

#include "fang.h"
#include "fres.h"
#include "fmath.h"
#include "flinklist.h"
#include "fversion.h"


#if !FANG_PLATFORM_GC && !FANG_WINGC
	// non-gamecube builds
	#define FDATA_WORLD_MESH_ALIGNMENT	16
#else
	// gamecube & gamecube tool builds
	#define FDATA_WORLD_MESH_ALIGNMENT	32
#endif

//---------------------------------------------------------------------------------------------------------------------------
// General Data Definitions:
//---------------------------------------------------------------------------------------------------------------------------
#define FDATA_TEXNAME_LEN				15	// Maximum number of characters in the texture name (excluding the NULL terminator)
#define FDATA_BONENAME_LEN				31	// Maximum number of characters in the bone name (excluding the NULL terminator)
#define FDATA_MESHNAME_LEN				15	// Maximum number of characters in the mesh name (excluding the NULL terminator)
#define FDATA_MAX_BONE_COUNT			127	// Maximum number of bones allowed in a single object

#define FDATA_TC_COUNT_PER_VTX			2	// Maximum number of texture coordinates per vertex
#define FDATA_VW_COUNT_PER_VTX			4	// Maximum number of matrix weights per vertex

#define FDATA_MAX_MESH_PARTS			32	// Mesh parts are detachable segments of meshes which may be consist of one or more bones

#define FDATA_MAX_LOD_MESH_COUNT		8	// There are a number of dependencies that rely on this number being 8 or less (so u8 storage is okay)

#define FDATA_MAX_SURFACE_SHADER_REGS	8
#define FDATA_MAX_LIGHT_SHADER_REGS		16


class CFTexInst;


// Kludgy temp hack for Mike:
typedef enum {
	FDXDATA_TEXFMT_A8R8G8B8 = 0,			// 32-bit, with 8-bit alpha
	FDXDATA_TEXFMT_X8R8G8B8,				// 32-bit
	FDXDATA_TEXFMT_A4R4G4B4,				// 16-bit, with 4-bit alpha
	FDXDATA_TEXFMT_A1R5G5B5,				// 16-bit, with 1-bit alpha
	FDXDATA_TEXFMT_X1R5G5B5,				// 16-bit
	FDXDATA_TEXFMT_R5G6B5,					// 16-bit
	FDXDATA_TEXFMT_DXT1,					// DXT1 compressed
	FDXDATA_TEXFMT_DXT3,					// DXT3 compressed
	FDXDATA_TEXFMT_DXT5,					// DXT5 compressed
	FDXDATA_TEXFMT_COUNT,

	FGCDATA_TEXFMT_R8G8B8A8 = 0,			// 32-bit, 8-bit alpha
	FGCDATA_TEXFMT_R8G8B8X8,				// 32-bit, opaque
	FGCDATA_TEXFMT_R4G4B4A3,				// 16-bit, 3-bit alpha
	FGCDATA_TEXFMT_R5G5B5X1,				// 15-bit, opaque  (ONLY FOR INTERNAL FANG USAGE)
	FGCDATA_TEXFMT_R5G6B5,					// 16-bit, opaque
	FGCDATA_TEXFMT_S3TC,					// 4-bit compressed RGB, opaque
	FGCDATA_TEXFMT_S3TCA1,					// 4-bit compressed RGB, 1-bit alpha
	FGCDATA_TEXFMT_S3TCx2,					// S3TC pair, 4-bit RGB representing color, 4-bit rep. Alpha in green channel
	FGCDATA_TEXFMT_I8,						// 8-bit intensity map
	FGCDATA_TEXFMT_IA8,						// 8-bit intensity + 8-bit alpha map (used for bumpmapping)
	FGCDATA_TEXFMT_IA4,						// 4-bit intensity + 4-bit alpha map (used for bumpmapping)
	FGCDATA_TEXFMT_COUNT
} FData_TexFmt_e;

typedef enum {
	FDXDATA_PALFMT_NONE = 0,
	FDXDATA_PALFMT_COUNT,
} FData_PalFmt_e;



//---------------------------------------------------------------------------------------------------------------------------
// Animation Matrix File Data (.mtx files):
//
// Data laid out as follows:
//    A) One FData_MtxFileHeader_t, immediately followed by:
//    B) Array of FData_MtxFileBone_t (number of elements is FData_MtxFileHeader_t::nNumBones.
//    C) Array of FData_MtxFileMatrix_t for each FData_MtxFileBone_t (number of elements is FData_MtxFileBone_t::nNumFrames)
//---------------------------------------------------------------------------------------------------------------------------

typedef enum {
	FDATA_MTX_FILE_DATA_TYPE_MATRIX = 0,// Animation data is raw matrices

	FDATA_MTX_FILE_DATA_TYPE_COUNT
} FData_MtxFileDataType_e;

typedef struct {						// Describes the file:
	u32 nNumBones;						// The number of elements in the FData_MtxFileBone_t array that immediately follows this structure
	FData_MtxFileDataType_e nDataType;	// The type of animation data contained in this file


	//////////////////////////////////////////////////////////////////////////////
	// THIS FUNCTION MUST BE PRESENT AND MUST BE UPDATED FOR PASM TO WORK PROPERLY
	//////////////////////////////////////////////////////////////////////////////
	// call to change from Big to Little Endian and visa versa
	void ChangeEndian() { 
		nNumBones = fang_ConvertEndian( nNumBones );
		nDataType = (FData_MtxFileDataType_e)fang_ConvertEndian( (u32)nDataType );
	}
} FData_MtxFileHeader_t;

typedef struct {						// Describes one bone:
	char szName[FDATA_BONENAME_LEN+1];	// The name of the bone
	u32 nNumFrames;						// The number of elements in the FData_MtxFileMatrix_t array, referenced by nFrameArrayOffset
	u32 nFrameArrayOffset;				// The offset of this bone's FData_MtxFileMatrix_t array (in bytes from the head of this file)


	//////////////////////////////////////////////////////////////////////////////
	// THIS FUNCTION MUST BE PRESENT AND MUST BE UPDATED FOR PASM TO WORK PROPERLY
	//////////////////////////////////////////////////////////////////////////////
	// call to change from Big to Little Endian and visa versa
	void ChangeEndian() { 
		for( u32 i=0; i < sizeof( szName ); i++ ) {
			szName[i] = fang_ConvertEndian( szName[i] );
		}
		nNumFrames = fang_ConvertEndian( nNumFrames );
		nFrameArrayOffset = fang_ConvertEndian( nFrameArrayOffset );
	}
} FData_MtxFileBone_t;

typedef struct {						// Describes one frame of animation:
	f32 fStartingSecs;					// The time of this frame's matrix (in seconds from the beginning of the animation)
	CFMtx43 Mtx;						// This frame's matrix. Can hold rotation, translation, and uniform scale


	//////////////////////////////////////////////////////////////////////////////
	// THIS FUNCTION MUST BE PRESENT AND MUST BE UPDATED FOR PASM TO WORK PROPERLY
	//////////////////////////////////////////////////////////////////////////////
	// call to change from Big to Little Endian and visa versa
	void ChangeEndian() { 
		fStartingSecs = fang_ConvertEndian( fStartingSecs );
		Mtx.ChangeEndian();
	}
} FData_MtxFileMatrix_t;

// END OF MTX STRUCTS
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



//---------------------------------------------------------------------------------------------------------------------------
// World File Data (.wld files):
//
// Data laid out as follows:
//    A) One FData_WorldFileHeader_t	( load into memory 16 byte aligned and each header & data will be 16 byte aligned )
//    B) One FData_MeshDataHeader_t		
//	  C) One Mesh file					( Mesh file + Fixups[] )
//	  D) One FData_WorldDataHeader_t	
//    E) One World file					( World file + Fixups[] )
//	  F) One FData_WorldInitHeader_t	
//	  G) One Init file					( Init file + String[] )
//
//	*** FOR PORTAL SYSTEM ***
// Data should be laid out as follows:
//    A) One FData_WorldFileHeader_t	( load into memory 16 byte aligned and each header & data will be 16 byte aligned )
//    B) Array of mesh offsets (each are a u32)				
//    C) Array of mesh sizes (each are a u32)			
//	  C) World Mesh files				( FMesh_t files )
//    E) One FVisData file				( FVisData_t )
//	  F) One FData_WorldInitHeader_t	
//	  G) One Init file					( Init file + String[] )
//
//---------------------------------------------------------------------------------------------------------------------------

typedef struct {			// World file header:

	u32 nNumBytes;			// The number of bytes contained in this world file

	u32 nNumMeshes;			// The number of mesh offsets contained at nMeshInitOffset
	u32 nOffsetToMeshInits;	// The offset to the first mesh offset (in bytes from the head of this file)
	u32 nOffsetToMeshSizes;	// The offset to the first mesh size (bytes of the entire mesh, mesh header, and fixups)
	u32 nMeshBytes;			// The size in bytes of the entire mesh block.

	u32 nWorldOffset;		// The offset to the world object (in bytes from the head of this file)
	u32 nWorldBytes;		// The number of bytes in the entire world, world header, and world fixups
	
	u32 nStreamingDataOffset;// The offset to the streaming data for this world (in bytes from the head of this file)
	u32 nStreamingDataBytes; // The number of bytes in the streaming data
	
	u32 nInitOffsets;		// The offset to the init data (in bytes from the head of this file)
	u32 nInitBytes;			// The number of bytes in the init block plus string table


	//////////////////////////////////////////////////////////////////////////////
	// THIS FUNCTION MUST BE PRESENT AND MUST BE UPDATED FOR PASM TO WORK PROPERLY
	//////////////////////////////////////////////////////////////////////////////
	// call to change from Big to Little Endian and visa versa
	void ChangeEndian() { 
		nNumBytes = fang_ConvertEndian( nNumBytes );
		nNumMeshes = fang_ConvertEndian( nNumMeshes );
		nOffsetToMeshInits = fang_ConvertEndian( nOffsetToMeshInits );
		nOffsetToMeshSizes = fang_ConvertEndian( nOffsetToMeshSizes );
		nMeshBytes = fang_ConvertEndian( nMeshBytes );
		nWorldOffset = fang_ConvertEndian( nWorldOffset );
		nWorldBytes = fang_ConvertEndian( nWorldBytes );
		nStreamingDataOffset = fang_ConvertEndian( nStreamingDataOffset );
		nStreamingDataBytes = fang_ConvertEndian( nStreamingDataBytes );
		nInitOffsets = fang_ConvertEndian( nInitOffsets );
		nInitBytes = fang_ConvertEndian( nInitBytes );
	}

} FData_WorldFileHeader_t;

typedef struct {
	u32 nOffsetToFixupTable;	// The offset to the mesh fixup table (in bytes from the start of the mesh data)
	u32 nNumFixups;				// The number of fixups in the fixup table	

	u8 nPad[8];					// To ensure sizof is multiple of 16


	//////////////////////////////////////////////////////////////////////////////
	// THIS FUNCTION MUST BE PRESENT AND MUST BE UPDATED FOR PASM TO WORK PROPERLY
	//////////////////////////////////////////////////////////////////////////////
	// call to change from Big to Little Endian and visa versa
	void ChangeEndian() { 
		nOffsetToFixupTable = fang_ConvertEndian( nOffsetToFixupTable );
		nNumFixups = fang_ConvertEndian( nNumFixups );
		u32 i;
		for( i=0; i < sizeof( nPad ); i++ ) {
			nPad[i] = fang_ConvertEndian( nPad[i] );
		}
	}
} FData_MeshDataHeader_t;

typedef struct {
	u32 nOffsetToFixupTable;	// The ofset to the world fixup table (in bytes from the start of the world data)
	u32 nNumFixups;				// The number of fixups in the fixup table

	u8 nPad[8];					// To ensure sizof is multiple of 16


	//////////////////////////////////////////////////////////////////////////////
	// THIS FUNCTION MUST BE PRESENT AND MUST BE UPDATED FOR PASM TO WORK PROPERLY
	//////////////////////////////////////////////////////////////////////////////
	// call to change from Big to Little Endian and visa versa
	void ChangeEndian() { 
		nOffsetToFixupTable = fang_ConvertEndian( nOffsetToFixupTable );
		nNumFixups = fang_ConvertEndian( nNumFixups );
		u32 i;
		for( i=0; i < sizeof( nPad ); i++ ) {
			nPad[i] = fang_ConvertEndian( nPad[i] );
		}
	}
} FData_WorldDataHeader_t;

typedef struct {
	u32 nNumInitStructs;		// how many CFWorldShapeInit are there following this struct
	
	u8 nPad[12];				// To ensure sizof is multiple of 16


	//////////////////////////////////////////////////////////////////////////////
	// THIS FUNCTION MUST BE PRESENT AND MUST BE UPDATED FOR PASM TO WORK PROPERLY
	//////////////////////////////////////////////////////////////////////////////
	// call to change from Big to Little Endian and visa versa
	void ChangeEndian() { 
		nNumInitStructs = fang_ConvertEndian( nNumInitStructs );
		u32 i;
		for( i=0; i < sizeof( nPad ); i++ ) {
			nPad[i] = fang_ConvertEndian( nPad[i] );
		}
	}
} FData_WorldInitHeader_t;

// END OF WLD STRUCTS
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



//---------------------------------------------------------------------------------------------------------------------------
// Master File Data (.mst files):
//
// Data laid out as follows:
//    A) One FDataPrjFile_Header_t
//    B) Array of FDataPrjFile_Entry_t (The number of used elements is FDataPrjFile_Header_t::nNumEntries, the total 
//										array size is FDataPrjFile_Header_t::nNumEntries + FDataPrjFile_Header_t::nNumFreeEntries)
//	  D) Array of FDataPrjFile_SupportEntry_t (The number of used elements is FDataPrjFile_Header_t::nNumSupportEntries, the total
//											   array size is FDataPrjFile_Header_t::nNumSupportEntries + FDataPrjFile_Header_t::nNumFreeSupportEntries)
//    C) Data Bytes
//---------------------------------------------------------------------------------------------------------------------------

#define FDATA_PRJFILE_FILENAME_LEN				16
//ARG - >>>>>
#ifdef _MMI_TARGET_PS2
#define FDATA_PRJFILE_FILE_ALIGNMENT			16
#else
//ARG - <<<<<
#define FDATA_PRJFILE_FILE_ALIGNMENT			2048	// Each file's data starts on a file offset divisible by this number (must be power of 2)
#endif	//ARG
#define FDATA_PRJFILE_MIN_ENTRY_COUNT			16390	// Allocate enough elements for this many entries

#define FDATA_PRJFILE_MAJOR						0x01	// 1 Byte
#define FDATA_PRJFILE_MINOR						0x08	// 1 Byte
#define FDATA_PRJFILE_SUB						0x00	// 1 Byte
#define FDATA_PRJFILE_XB_VERSION				( FVERSION_CREATE_VERSION( FVERSION_PLATFORM_FLAG_XBOX | FVERSION_PLATFORM_FLAG_TOOLS, FDATA_PRJFILE_MAJOR, FDATA_PRJFILE_MINOR, FDATA_PRJFILE_SUB ) )
#define FDATA_PRJFILE_GC_VERSION				( FVERSION_CREATE_VERSION( FVERSION_PLATFORM_FLAG_GC | FVERSION_PLATFORM_FLAG_TOOLS, FDATA_PRJFILE_MAJOR, FDATA_PRJFILE_MINOR, FDATA_PRJFILE_SUB ) )
#define FDATA_PRJFILE_PS_VERSION				( FVERSION_CREATE_VERSION( FVERSION_PLATFORM_FLAG_PS2 | FVERSION_PLATFORM_FLAG_TOOLS, FDATA_PRJFILE_MAJOR, FDATA_PRJFILE_MINOR, FDATA_PRJFILE_SUB ) )	//ARG

#define FDATA_MESH_MAX_TEXLAYERIDS_PER_MTL		4		// Max allowed texture layer IDs per material

// Compiler versions of each compiled type, so that we can auto compile types:
enum {
	// xb compiler versions
	FDATA_PRJFILE_XB_TGA_VERSION	= 0x00000010,
	FDATA_PRJFILE_XB_APE_VERSION	= 0x00000044,// covers both .ape & .wld files

	// gc compiler versions
	FDATA_PRJFILE_GC_TGA_VERSION	= 0x00000006,
	FDATA_PRJFILE_GC_APE_VERSION	= 0x00000037,// covers both .ape & .wld files
	
//ARG - >>>>>
	// ps2 compiler versions
	FDATA_PRJFILE_PS_TGA_VERSION	= 0x00000001,
	FDATA_PRJFILE_PS_APE_VERSION	= 0x00000001,// covers both .ape & .wld files
//ARG - <<<<<

	// common compiler versions
	FDATA_PRJFILE_MTX_VERSION		= 0x00000014,
	FDATA_PRJFILE_CSV_VERSION		= 0x0000000B,
	FDATA_PRJFILE_FNT_VERSION		= 0x0000000A,
	FDATA_PRJFILE_SMA_VERSION		= 0x00000003,
	FDATA_PRJFILE_GT_VERSION		= 0x0000000a,
	FDATA_PRJFILE_WVB_VERSION		= 0x00000003,
	FDATA_PRJFILE_FPR_VERSION		= 0x00000003,
	FDATA_PRJFILE_CAM_VERSION		= 0x00000000,

	/////////////////////////////////////////////////////////////////
	// IF YOU ADD NEW COMPILER TYPES, YOU NEED TO UPDATE FFILE.CPP SO 
	// THAT APPS CHECK EACH TYPE'S COMPILER VERSION FOR MISMATCH AT 
	// MASTER FILE LOAD TIME.
	//////////////////////////////////////////////////////////////////
};

typedef struct {
	FVersionHeader_t Version;					// Version & signature information
	u32 nBytesInFile;							// Total number of bytes in the file	
	u32 nNumEntries;							// How many valid entries are used in this .mst file
	u32 nNumFreeEntries;						// How many entries are free? (these start after nNumEntries)
	u32 nNumSupportEntries;						// How many valid support entries are used in this .mst file (these start after the last free entry)
	u32 nNumFreeSupportEntries;					// How many support entries are free? (these start after nNumSupportEntries)

	u32 nDataOffset;							// Byte offset to the 1st data entry's data
	
	u32 nTgaCompilerVersion;					// What TGA compiler version was used to compile the TGAs in this file
	u32 nApeCompilerVersion;					// What APE compiler version was used to compile the APEs in this file
	u32 nMtxCompilerVersion;					// What MTX compiler version was used to compile the MTXs in this file
	u32 nCsvCompilerVersion;					// What CSV compiler version was used to compile the CSVs in this file
	u32 nFntCompilerVersion;					// What FNT compiler version was used to compile the FNTs in this file
	u32 nSmaCompilerVersion;					// What SMA compiler version was used to compile the SMAs in this file
	u32 nGtCompilerVersion;						// What GT compiler version was used to compile the GTs in this file
	u32 nWvbCompilerVersion;					// What WVB compiler version was used to compile the WVBs in this file
	u32 nFprCompilerVersion;					// What FPR compiler version was used to compile the FPRs in this file
	u32 nCamCompilerVersion;					// What CAM compiler version was used to compile the CAMs in this file

	u8 nReserved[36];							// Reserved for future expansion
	

	//////////////////////////////////////////////////////////////////////////////
	// THIS FUNCTION MUST BE PRESENT AND MUST BE UPDATED FOR PASM TO WORK PROPERLY
	//////////////////////////////////////////////////////////////////////////////
	// call to change from Big to Little Endian and visa versa
	void ChangeEndian() { 
		Version.ChangeEndian();
		nBytesInFile = fang_ConvertEndian( nBytesInFile );
		nNumEntries = fang_ConvertEndian( nNumEntries );
		nNumFreeEntries = fang_ConvertEndian( nNumFreeEntries );
		nNumSupportEntries = fang_ConvertEndian( nNumSupportEntries );
		nNumFreeSupportEntries = fang_ConvertEndian( nNumFreeSupportEntries );
		nDataOffset = fang_ConvertEndian( nDataOffset );
		nTgaCompilerVersion = fang_ConvertEndian( nTgaCompilerVersion );
		nApeCompilerVersion = fang_ConvertEndian( nApeCompilerVersion );
		nMtxCompilerVersion = fang_ConvertEndian( nMtxCompilerVersion );
		nCsvCompilerVersion = fang_ConvertEndian( nCsvCompilerVersion );
		nFntCompilerVersion = fang_ConvertEndian( nFntCompilerVersion );
		nSmaCompilerVersion = fang_ConvertEndian( nSmaCompilerVersion );
		nGtCompilerVersion = fang_ConvertEndian( nGtCompilerVersion );
		nWvbCompilerVersion = fang_ConvertEndian( nWvbCompilerVersion );
		nFprCompilerVersion = fang_ConvertEndian( nFprCompilerVersion );
		nCamCompilerVersion = fang_ConvertEndian( nCamCompilerVersion );
		for( u32 i=0; i < sizeof( nReserved ); i++ ) {
			nReserved[i] = fang_ConvertEndian( nReserved[i] );
		}
	}
} FDataPrjFile_Header_t;


typedef struct {
	char szFilename[FDATA_PRJFILE_FILENAME_LEN];// The filename (no path info) in lowercase, with extension
	u16 nFlags;									// Flags that are specific to each entry type and include PASM compilation information
	u16 __PAD;
	u32 nStartOffset;							// Offset to the first byte of data for this entry (from the beginning of the file)
	u32 nNumBytes;								// Total number of bytes in this entry

	u32 nModifiedTime;							// Really the file's time_t structure containing the file's last write time
	u32 nCRC;									// The CRC value for the file (0 indicates that no CRC exists)


	//////////////////////////////////////////////////////////////////////////////
	// THIS FUNCTION MUST BE PRESENT AND MUST BE UPDATED FOR PASM TO WORK PROPERLY
	//////////////////////////////////////////////////////////////////////////////
	// call to change from Big to Little Endian and visa versa
	void ChangeEndian() {
		for( u32 i=0; i < FDATA_PRJFILE_FILENAME_LEN; i++ ) {
			szFilename[i] = fang_ConvertEndian( szFilename[i] );
		}
		nFlags = fang_ConvertEndian( nFlags );
		nStartOffset = fang_ConvertEndian( nStartOffset );
		nNumBytes = fang_ConvertEndian( nNumBytes );
		nModifiedTime = fang_ConvertEndian( nModifiedTime );
		nCRC = fang_ConvertEndian( nCRC );
	}
} FDataPrjFile_Entry_t;

typedef struct {
	char szFilename[FDATA_PRJFILE_FILENAME_LEN];// The filename (no path info) in lowercase, with extension
	u32 nModifiedTime;							// Really the file's time_t structure containing the file's last write time


	//////////////////////////////////////////////////////////////////////////////
	// THIS FUNCTION MUST BE PRESENT AND MUST BE UPDATED FOR PASM TO WORK PROPERLY
	//////////////////////////////////////////////////////////////////////////////
	// call to change from Big to Little Endian and visa versa
	void ChangeEndian() {
		for( u32 i=0; i < FDATA_PRJFILE_FILENAME_LEN; i++ ) {
			szFilename[i] = fang_ConvertEndian( szFilename[i] );
		}
		nModifiedTime = fang_ConvertEndian( nModifiedTime );
	}
} FDataPrjFile_SupportEntry_t;			

// END OF MST STRUCTS
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////




//---------------------------------------------------------------------------------------------------------------------------
// Game File Data (.csv files):
//
// Data laid out as follows:
//    A) One FDataGamFile_Header_t
//	  B) Array of FDataGamFile_Table_t (The size of this array is FDataGamFile_Header_t::nNumTables)
//	  C) Array of [ szKeyStrings followed by FDataGamFile_Field_t[] ]
//---------------------------------------------------------------------------------------------------------------------------

typedef enum {
	FDATA_GAMEFILE_DATA_TYPE_STRING = 0,	// The data type is a lower-case string 
	FDATA_GAMEFILE_DATA_TYPE_FLOAT,			// The data type is a floating point number		
	FDATA_GAMEFILE_DATA_TYPE_WIDESTRING,    // The data type is a wide (unicode) string

	FDATA_GAMEFILE_DATA_TYPE_COUNT
} FDataGameFile_DataType_e;

typedef struct {
	FDataGameFile_DataType_e nDataType;	// What type of data is this entry?
	union {
		cchar  *pszValue;	// points to a NULL terminated value string
		cwchar *pwszValue;  // points to a NULL terminated wide value string
		f32    fValue;		// a floating point value
	};
	u32 nStringLen;			// fclib_strlen on pszValue (always 0 if nDataType != _FILE_DATA_TYPE_STRING)


	//////////////////////////////////////////////////////////////////////////////
	// THIS FUNCTION MUST BE PRESENT AND MUST BE UPDATED FOR PASM TO WORK PROPERLY
	//////////////////////////////////////////////////////////////////////////////
	// call to change from Big to Little Endian and visa versa
	void ChangeEndian( u32 nOffsetToPtr ) {
		if( nDataType == FDATA_GAMEFILE_DATA_TYPE_WIDESTRING ) {
			//convert the widestring that this pointer points to...
			u16* pwszString = (u16*) ( (u32)pszValue + nOffsetToPtr );
			for(u32 i=0;i<nStringLen;i++){
				pwszString[ i ] = fang_ConvertEndian( pwszString[ i ] );
			}
		}
		nDataType = (FDataGameFile_DataType_e)fang_ConvertEndian( (u32)nDataType );
		// only change 1 of the union vars (otherwise it will change it back and forth)
		pszValue = (cchar *)fang_ConvertEndian( (void *)pszValue );
		nStringLen = fang_ConvertEndian( nStringLen );		
	}
} FDataGamFile_Field_t;

#define FDATA_GAMFILE_MAX_TABLES			0xFFFF
#define FDATA_GAMFILE_MAX_FIELDS_PER_TABLE	0xFFFF

typedef struct {
	cchar *pszKeyString;		// points to a NULL terminated lowercase keystring
	u32 nKeyStringLen;			// fclib_strlen on pszKeyString
	u16 nNumFields;				// how many fields are contained in this table? (Max = FDATA_GAMFILE_MAX_FIELDS_PER_TABLE)
	u16 nTableIndex;			// what table index is this in the file? (Max = FDATA_GAMFILE_MAX_TABLES-1)
	FDataGamFile_Field_t *paFields;// points to nNumFields FDataGamFile_Field_t structs
	

	//////////////////////////////////////////////////////////////////////////////
	// THIS FUNCTION MUST BE PRESENT AND MUST BE UPDATED FOR PASM TO WORK PROPERLY
	//////////////////////////////////////////////////////////////////////////////
	// call to change from Big to Little Endian and visa versa
	void ChangeEndian() { 
		pszKeyString = (cchar *)fang_ConvertEndian( (void *)pszKeyString );
		nKeyStringLen = fang_ConvertEndian( nKeyStringLen );
		nNumFields = fang_ConvertEndian( nNumFields );
		nTableIndex = fang_ConvertEndian( nTableIndex );
		paFields = (FDataGamFile_Field_t *)fang_ConvertEndian( paFields );
	}
} FDataGamFile_Table_t;

enum {
	FDATA_GAMFILE_FLAGS_OFFSETS_FIXEDUP		= 0x00000001,// The pointers in this file have been fixed up already
	FDATA_GAMFILE_FLAGS_FMEM_LOADED			= 0x00000002,// This file has been loaded into fmem memory (if not set fres was used)
	FDATA_GAMFILE_FLAGS_UNKNOWN_ORIGIN		= 0x00000004,// This file was neither fres or fmem loaded (probably loaded with the world)
	
	FDATA_GAMFILE_FLAGS_NONE				= 0x00000000,
};

typedef struct {
	u32 nBytesInFile;				// Total number of bytes in the file	
	u32 nNumTables;					// How many tables are there? (MAX = FDATA_GAMFILE_MAX_TABLES)
	FDataGamFile_Table_t *paTables;	// points to nNumTables FDataGamFile_Table_t structs

	u32 nFlags;						// (FDATA_GAMFILE_FLAGS_...) used internally by the fgamedata.h system


	//////////////////////////////////////////////////////////////////////////////
	// THIS FUNCTION MUST BE PRESENT AND MUST BE UPDATED FOR PASM TO WORK PROPERLY
	//////////////////////////////////////////////////////////////////////////////
	// call to change from Big to Little Endian and visa versa
	void ChangeEndian() { 
		nBytesInFile = fang_ConvertEndian( nBytesInFile );
		nNumTables = fang_ConvertEndian( nNumTables );
		paTables = (FDataGamFile_Table_t *)fang_ConvertEndian( paTables );	
	}
} FDataGamFile_Header_t;

// END OF CSV STRUCTS
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////




//---------------------------------------------------------------------------------------------------------------------------
// Font File Data (.fnt files):
//
// The font file is 2-byte char compliant.  
// Characters are grouped into "letter buckets" based on the lower 8 bits of their character code, thus requiring
// a maximum of 256 buckets for an entire font, and restricing the maximum # of elements needing to be searched 
// within a bucket to 256.
// Each letter bucket contains an index into a number of BucketLetter Structures.  A BucketLetter structure maps a character
// code to a particular FData_FntFile_Letter_t structure.  There can be a many to one relationship between BucketLetter structs
// and font letter structs.  For example, the 'A' letter could be exported in a fontLetter, but not an 'a'.  Two BucketLetter
// structs will be created, mapping both the 'A' and the 'a' character codes to the single 'A' FontLetter struct
//
// Data laid out as follows:
//    A) One FDataFntFile_Font_t
//    B) Array of FDataFntFile_LetterBucket_t (The length of this array is FDataFntFile_Font_t::uBuckets)
//    C) Array of FDataFntFile_BucketLetter_t (The length of this array is FDataFntFile_Font_t::uBucketLetters)
//    D) Array of FDataFntFile_Letter_t (The length of this array is FDataFntFile_Font_t::uFntLetters)
//---------------------------------------------------------------------------------------------------------------------------

#define FDATA_FNT_MAXBUCKETS_IN_CHAR_SET	256
#define FDATA_FNT_MAXLETTERS_IN_BUCKET		256
#define FDATA_FNT_LETTERBUCKET_EMPTY		0xFFFF

typedef u8 FDataFntFile_Handle_t;

typedef struct {
	u32 uTexurePage;
	f32 fVertAlign; // From top.
	f32 fHorzAlign; // From left.
	f32 fUVUpperLeftX;  // UV in texture page.
	f32 fUVUpperLeftY;  // UV in texture page.
	f32 fUVLowerRightX; // UV in texture page.
	f32 fUVLowerRightY; // UV in texture page.
	f32 fWidth;
	f32 fHeight;
	f32 fItalicPositiveSlantTop;    // Optimization used for uniform italicizing.
	f32 fItalicPositiveSlantBottom; // Optimization used for uniform italicizing.
	f32 fItalicNegativeSlantTop;    // Optimization used for uniform italicizing.
	f32 fItalicNegativeSlantBottom; // Optimization used for uniform italicizing.


	//////////////////////////////////////////////////////////////////////////////
	// THIS FUNCTION MUST BE PRESENT AND MUST BE UPDATED FOR PASM TO WORK PROPERLY
	//////////////////////////////////////////////////////////////////////////////
	// call to change from Big to Little Endian and visa versa.
	void ChangeEndian( void ) {
		uTexurePage = fang_ConvertEndian( uTexurePage );
		fVertAlign = fang_ConvertEndian( fVertAlign );
		fHorzAlign = fang_ConvertEndian( fHorzAlign );
		fUVUpperLeftX = fang_ConvertEndian( fUVUpperLeftX );
		fUVUpperLeftY = fang_ConvertEndian( fUVUpperLeftY );
		fUVLowerRightX = fang_ConvertEndian( fUVLowerRightX );
		fUVLowerRightY = fang_ConvertEndian( fUVLowerRightY );
		fWidth = fang_ConvertEndian( fWidth );
		fHeight = fang_ConvertEndian( fHeight );
		fItalicPositiveSlantTop = fang_ConvertEndian( fItalicPositiveSlantTop );
		fItalicPositiveSlantBottom = fang_ConvertEndian( fItalicPositiveSlantBottom );
		fItalicNegativeSlantTop = fang_ConvertEndian( fItalicNegativeSlantTop );
		fItalicNegativeSlantBottom = fang_ConvertEndian( fItalicNegativeSlantBottom );
	}
} FDataFntFile_Letter_t;


typedef struct {
	u16 uCharCode;
	u16 uFntLetterIndex;

	//////////////////////////////////////////////////////////////////////////////
	// THIS FUNCTION MUST BE PRESENT AND MUST BE UPDATED FOR PASM TO WORK PROPERLY
	//////////////////////////////////////////////////////////////////////////////
	// call to change from Big to Little Endian and visa versa.
	void ChangeEndian( void ) {
		uCharCode = fang_ConvertEndian( uCharCode );
		uFntLetterIndex = fang_ConvertEndian( uFntLetterIndex );
	}
} FDataFntFile_BucketLetter_t;


typedef struct {
	u16 nNumLettersInBucket;
	u16 nBucketLetterBaseIndex; //base index into the array of bucket letters in FDataFntFile_Font_t  

	//////////////////////////////////////////////////////////////////////////////
	// THIS FUNCTION MUST BE PRESENT AND MUST BE UPDATED FOR PASM TO WORK PROPERLY
	//////////////////////////////////////////////////////////////////////////////
	// call to change from Big to Little Endian and visa versa.
	void ChangeEndian( void ) {
		nNumLettersInBucket = fang_ConvertEndian( nNumLettersInBucket );
		nBucketLetterBaseIndex = fang_ConvertEndian( nBucketLetterBaseIndex );
	}
} FDataFntFile_LetterBucket_t;



typedef struct _FntFile_Font_t FDataFntFile_Font_t;

struct _FntFile_Font_t {
	FDataFntFile_Font_t *poFontNext;

	FDataFntFile_Handle_t oHandle;

	u8 szName[ FRES_NAMELEN + 1 ];

	f32 fLetterSpacing;
	f32 fWordSpacing;
	f32 fItalicSlant;
	f32 fVerticalRuler;
	f32 fWidth;
	f32 fHeight;

	//This array is used to take a letter and get an index to the
	//appropriate bucket.  The lower 8 bits of a (possibly) 16 bit
	//character is used.  An Index of FDATA_FNT_LETTERBUCKET_EMPTY 
	//means that a bucket does not exist for that 8 bit key value.
	u16 auLetterToBucketIndex[ FDATA_FNT_MAXBUCKETS_IN_CHAR_SET ];

	//Array Element counts
	u16 uBucketLetters;
	u16 uFntLetters;
	u8 uBuckets;
	u8 uTexPages;

	//Arrays
	FDataFntFile_LetterBucket_t *paoLetterBuckets; //# of elements = uBuckets;
	FDataFntFile_BucketLetter_t *paoBucketLetters; //# of elements = uBucketLetters;
	FDataFntFile_Letter_t *paoFntLetters;          //# of elements = uFntLetters

	// runtime variables
	CFTexInst *paoTexInsts; // count represented by uTexPages
	u32 *pauTextLengthPerTexturePages; // (in chars) per texture pages. reset at every draw.Count Represented by uTexPages

	//////////////////////////////////////////////////////////////////////////////
	// THIS FUNCTION MUST BE PRESENT AND MUST BE UPDATED FOR PASM TO WORK PROPERLY
	//////////////////////////////////////////////////////////////////////////////
	// call to change from Big to Little Endian and visa versa.
	void ChangeEndian( void ) {
		//// Useless at PASM-time. Only here for the sake of completeness.
		poFontNext = (FDataFntFile_Font_t *)fang_ConvertEndian( poFontNext );
		oHandle = (FDataFntFile_Handle_t)fang_ConvertEndian( (u8)oHandle );
		u32 i;
		for( i=0; i < sizeof( szName ); i++ )
		{
			szName[ i ] = fang_ConvertEndian( szName[ i ] );
		}

		fLetterSpacing = fang_ConvertEndian( fLetterSpacing );
		fWordSpacing = fang_ConvertEndian( fWordSpacing );
		fItalicSlant = fang_ConvertEndian( fItalicSlant );
		fVerticalRuler = fang_ConvertEndian( fVerticalRuler );
		fWidth = fang_ConvertEndian( fWidth );
		fHeight = fang_ConvertEndian( fHeight );
		
		for( i=0; i < FDATA_FNT_MAXBUCKETS_IN_CHAR_SET; i++ )
		{
			auLetterToBucketIndex[ i ] = fang_ConvertEndian( auLetterToBucketIndex[ i ] );
		}

		uBucketLetters = fang_ConvertEndian( uBucketLetters );
		uFntLetters = fang_ConvertEndian( uFntLetters );
		uBuckets = fang_ConvertEndian( uBuckets );
		uTexPages = fang_ConvertEndian( uTexPages );

		paoLetterBuckets = ( FDataFntFile_LetterBucket_t *) fang_ConvertEndian( paoLetterBuckets );
		paoBucketLetters = ( FDataFntFile_BucketLetter_t *) fang_ConvertEndian( paoBucketLetters );
		paoFntLetters = ( FDataFntFile_Letter_t *) fang_ConvertEndian( paoFntLetters );

		paoTexInsts = (CFTexInst *)fang_ConvertEndian( paoTexInsts );
		pauTextLengthPerTexturePages = (u32 *)fang_ConvertEndian( pauTextLengthPerTexturePages );
	}
};

// END OF FNT STRUCTS
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////




//---------------------------------------------------------------------------------------------------------------------------
// Wav Bank Files (.wvb files):
//
// Data laid out as follows:
//    A) One FDataWvbFile_Bank_t
//	  B) Array of FDataWvbFile_Wave_t (The length of this array is FDataWvbFile_Bank_t::uWaves)
//	  C) Platform-specific data.
//
// Platform-specific data is laid out as follow on XB:
//    A) Array of FDataWvbFile_Bank_t::uWaves of XBOXADPCMWAVEFORMAT.
//    B) Array of FDataWvbFile_Bank_t::uWaves of wave sample data.
//
// Platform-specific data is laid out as follow on GC:
//    A) One endianized Musyx Bank ID in a u32.
//    B) Array of FDataWvbFile_Bank_t::uWaves of endianized Musyx Sound IDs in u32.
//
//---------------------------------------------------------------------------------------------------------------------------

#define FDATA_AUDIO_FILENAME_LEN		( 11 + 1 )

typedef u32 FAudio_BankHandle_t;	// ( FDataWvbFile_Bank_t *).
typedef u32 FAudio_WaveHandle_t;	// ( FDataWvbFile_Wave_t *).

typedef struct {
	char szName[FDATA_AUDIO_FILENAME_LEN];
	u32 uWaves;
	FLink_t oLink;
	void *pData;		// Points to data immediately following the last FDataWvbFile_Wave_t of this bank.
	u32 uDataLength;	// Length of data pointed to by pData (see above).


	//////////////////////////////////////////////////////////////////////////////
	// THIS FUNCTION MUST BE PRESENT AND MUST BE UPDATED FOR PASM TO WORK PROPERLY
	//////////////////////////////////////////////////////////////////////////////
	// call to change from Big to Little Endian and visa versa.
	void ChangeEndian( void ) {
		uWaves      = fang_ConvertEndian( uWaves );
		uDataLength = fang_ConvertEndian( uDataLength );
		
		//// Useless at PASM-time. Only here for the sake of completeness.
		oLink.pPrevLink = (FLink_t *)fang_ConvertEndian( oLink.pPrevLink );
		oLink.pNextLink = (FLink_t *)fang_ConvertEndian( oLink.pNextLink );
		pData           = fang_ConvertEndian( pData );
	}
} FDataWvbFile_Bank_t;

typedef struct {
	char szName[FDATA_AUDIO_FILENAME_LEN];
	u32 uLength; // In bytes, of actual wave data in bank.
	u32 uOffset; // In bytes, from beginning of first wav data in the bank.
	FAudio_BankHandle_t oBankHandle;
	u32 uIndex; // Zero-based sequence number in the bank.
	f32 fLengthInSeconds;
	u16 nNumChannels;
	u16 nFlags;
	f32 fFreqHz;


	//////////////////////////////////////////////////////////////////////////////
	// THIS FUNCTION MUST BE PRESENT AND MUST BE UPDATED FOR PASM TO WORK PROPERLY
	//////////////////////////////////////////////////////////////////////////////
	// call to change from Big to Little Endian and visa versa.
	void ChangeEndian( void ) {
		uLength          = fang_ConvertEndian( uLength );
		uOffset          = fang_ConvertEndian( uOffset );
		uIndex           = fang_ConvertEndian( uIndex );
		fLengthInSeconds = fang_ConvertEndian( fLengthInSeconds );
		nNumChannels	 = fang_ConvertEndian( nNumChannels );
		nFlags			 = fang_ConvertEndian( nFlags );
		fFreqHz			 = fang_ConvertEndian( fFreqHz );
		
		//// Useless at PASM-time. Only here for the sake of completeness.
		oBankHandle = fang_ConvertEndian( oBankHandle );
	}
} FDataWvbFile_Wave_t;

// END OF WVB STRUCTS
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////




//---------------------------------------------------------------------------------------------------------------------------
// Sound Fx Bank Files (.sfb files):
//
// Data laid out as follows:
//    A) One FData_SFxBank_Header_t
//	  B) Array of FData_SFxBank_Seq_t (The length of this array is FData_SFxBank_Header_t::nNumSFx)
//	  C) Array of FData_SFxBank_Cmd_t (These are pointed to by each FData_SFxBank_Seq_t)
//    D) Various structs such as specific cmd structs and NULL terminated strings (These are referenced by the various arrays) 
//
//---------------------------------------------------------------------------------------------------------------------------

typedef u32 FSndFx_BankHandle_t;
typedef u32 FSndFx_FxHandle_t;

typedef enum {
	FDATA_SFXBANK_CMD_TYPE_PLAY = 0,

	FDATA_SFXBANK_CMD_TYPE_COUNT
} FData_SFxBank_CmdType_e;

typedef struct {
	cchar *pszWavName;
	FAudio_WaveHandle_t WavHandle;
	u32 nLoops;
	f32 fUnitVolume;
	f32 fFrequencyFactor;


	//////////////////////////////////////////////////////////////////////////////
	// THIS FUNCTION MUST BE PRESENT AND MUST BE UPDATED FOR PASM TO WORK PROPERLY
	//////////////////////////////////////////////////////////////////////////////
	// call to change from Big to Little Endian and visa versa.
	void ChangeEndian( void ) {
		pszWavName = (cchar *)fang_ConvertEndian( (char *)pszWavName );
		WavHandle = fang_ConvertEndian( WavHandle );
		nLoops = fang_ConvertEndian( nLoops );
		fUnitVolume = fang_ConvertEndian( fUnitVolume );
		fFrequencyFactor = fang_ConvertEndian( fFrequencyFactor );
	}
} FData_SFxBank_PlayCmd_t;

typedef struct {
	FData_SFxBank_CmdType_e nCmdType;
	void *pCmdData;


	//////////////////////////////////////////////////////////////////////////////
	// THIS FUNCTION MUST BE PRESENT AND MUST BE UPDATED FOR PASM TO WORK PROPERLY
	//////////////////////////////////////////////////////////////////////////////
	// call to change from Big to Little Endian and visa versa.
	void ChangeEndian( void ) {
		nCmdType = (FData_SFxBank_CmdType_e)fang_ConvertEndian( (u32)nCmdType );
		pCmdData = fang_ConvertEndian( pCmdData );
	}
} FData_SFxBank_Cmd_t;

typedef struct {
	cchar *pszName;
	u32 nNumCmds;
	FData_SFxBank_Cmd_t *paCmds;
	FSndFx_FxHandle_t hHandleToMyself;


	//////////////////////////////////////////////////////////////////////////////
	// THIS FUNCTION MUST BE PRESENT AND MUST BE UPDATED FOR PASM TO WORK PROPERLY
	//////////////////////////////////////////////////////////////////////////////
	// call to change from Big to Little Endian and visa versa.
	void ChangeEndian( void ) {
		pszName = (cchar *)fang_ConvertEndian( (char *)pszName );
		nNumCmds = fang_ConvertEndian( nNumCmds );
		paCmds = (FData_SFxBank_Cmd_t *)fang_ConvertEndian( paCmds );
	}
} FData_SFxBank_Seq_t;

typedef struct {
	cchar *pszBankName;// no extension	
	FAudio_BankHandle_t BankHandle;
	u32 nNumSFx;
	FData_SFxBank_Seq_t *paSeqs;
	u32 nKey;	// to ensure that a handles are still valid


	//////////////////////////////////////////////////////////////////////////////
	// THIS FUNCTION MUST BE PRESENT AND MUST BE UPDATED FOR PASM TO WORK PROPERLY
	//////////////////////////////////////////////////////////////////////////////
	// call to change from Big to Little Endian and visa versa.
	void ChangeEndian( void ) {
		pszBankName = (cchar *)fang_ConvertEndian( (char *)pszBankName );
		BankHandle = fang_ConvertEndian( BankHandle );
		nNumSFx = fang_ConvertEndian( nNumSFx );
		paSeqs = (FData_SFxBank_Seq_t *)fang_ConvertEndian( paSeqs );
	}
} FData_SFxBank_Header_t;

// END OF SFB STRUCTS
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



//---------------------------------------------------------------------------------------------------------------------------
// Platform-Specific Data:
//---------------------------------------------------------------------------------------------------------------------------
#include "./gc/fgcdata.h"// MASS MEDIA, THIS NEEDS TO BE HERE FOR OUR WINDOWS TOOLS, IT SHOULD NOT HURT YOU CAUSE THERE IS 
						 // NOTHING GC SPECIFIC ABOUT THIS HEADER - Mike Starich
//ARG#if !FANG_PLATFORM_GC
//ARG	#include "./dx/fdx8data.h"	
//ARG - >>>>>
#if FANG_PLATFORM_DX
	#include "./dx/fdx8data.h"
#elif FANG_PLATFORM_GC
	#include "./gc/fgcdata.h"
//ARG - <<<<<
#endif

#endif
