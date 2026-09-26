//////////////////////////////////////////////////////////////////////////////////////
// fGCdata.h - 
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
// 02/18/02 Lafleur		Created from stubbed DX version.
//////////////////////////////////////////////////////////////////////////////////////

#ifndef _FGCDATA_H_
#define _FGCDATA_H_ 1

#include "fang.h"

#define FGCDATA_FIXED_VERTEX_FORMATS		7

#if FANG_PLATFORM_GC
	#include "Dolphin\GX\GXEnum.h"
	#define FGCDATA_VARIABLE_VERTEX_FORMAT		GX_VTXFMT7
#else
	#define FGCDATA_VARIABLE_VERTEX_FORMAT		7
#endif

//
//
//
struct FGCData_VtxFmt_t
{
	u8		nPosBitDepth;
	u8		nPosFracBits;
	u8		nNrmBitDepth;
	u8		nNrmFracBits;
	u8		nSTsBitDepth;
	u8		nSTsFracBits;
};

extern FGCData_VtxFmt_t FGCData_VertexFormatDesc[FGCDATA_FIXED_VERTEX_FORMATS];



//
//
typedef enum 
{
	// The type describes what the corresponding shader register contains:

	FGCDATA_MESHSHREGTYPE_IMMEDIATE,	// The register value is a 32-bit constant
	FGCDATA_MESHSHREGTYPE_MOTIF,		// The register value is an index into the array of MeshMotif_t's
	FGCDATA_MESHSHREGTYPE_SHTEXINST,	// The register value is an index into the array of FShTexInst_t's

	FGCDATA_MESHSHREGTYPE_COUNT
} FGCData_MeshShRegType_e;



///////////////////////////////////////
// Streaming Wav File Data (.wvs files):
//
// Data laid out as follows:
//    A) One FGCData_WvsFile_Header_t
//	  B) Chunks of data (The chunk size is specified in the header.
//						 The chunks are interleaved so Channel1 - Chunk1, Channel2 - Chunk1, Channel1 - Chunk2...)
//
#define FGCDATA_WVS_MAX_CHUNK_BYTES		4096
#define FGCDATA_WVS_MAX_NUM_CHANNELS	2

typedef struct {
	s16 anCoef[16];		// the coeffients computed by the ADPCM dll

	//////////////////////////////////////////////////////////////////////////////
	// THIS FUNCTION MUST BE PRESENT AND MUST BE UPDATED FOR PASM TO WORK PROPERLY
	//////////////////////////////////////////////////////////////////////////////
	// call to change from Big to Little Endian and visa versa
	void ChangeEndian() { 
		u32 i;
		for( i=0; i < 16; i++ ) {
			anCoef[i] = fang_ConvertEndian( anCoef[i] );
		}
	}
} FGCData_WvsFile_ChannelInfo_t;
typedef struct {
	u32 nNumChannels;	// how many FGCData_WvsFile_ChannelInfo_t's follow this struct
	f32 fSecs;			// how long (in Secs) is the audio data
	f32 fFreqHz;		// what is the samples per second
	u32 nChunkBytes;	// how large is each chunk of data (the last one may be smaller)
	u32 nNumChunksPerChannel;// how many chunks of data are there per channel
	u32 nNumBytesPerChannel;// how many total bytes of data are there per channel

	FGCData_WvsFile_ChannelInfo_t aChannelInfo[FGCDATA_WVS_MAX_NUM_CHANNELS];// only the first nNumChannels are used

	/////////////////////////////////////////////////////////////////////////////////////////////////////
	u8 nPad[8];			// FFILE LIMITATION: ALWAYS PAD THE SIZE OF THIS STRUCT TO A MULTIPLE OF 32 BYTES
	/////////////////////////////////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////////
	// THIS FUNCTION MUST BE PRESENT AND MUST BE UPDATED FOR PASM TO WORK PROPERLY
	//////////////////////////////////////////////////////////////////////////////
	// call to change from Big to Little Endian and visa versa
	void ChangeEndian() { 
		nNumChannels = fang_ConvertEndian( nNumChannels );
		fSecs = fang_ConvertEndian( fSecs );
		fFreqHz = fang_ConvertEndian( fFreqHz );
		nChunkBytes = fang_ConvertEndian( nChunkBytes );
		nNumChunksPerChannel = fang_ConvertEndian( nNumChunksPerChannel );
		nNumBytesPerChannel = fang_ConvertEndian( nNumBytesPerChannel );
		u32 i;
		for( i=0; i < FGCDATA_WVS_MAX_NUM_CHANNELS; i++ ) {
			aChannelInfo[i].ChangeEndian();
		}			
	}
} FGCData_WvsFile_Header_t;

//////////////////////////////////////////////////
// MusyX Sound Data Files (has the .rdg extension)
//
// Data laid out as follows:
//    A) One FGCData_MusyXDataHeader_t
//	  B) One .proj file (the size is specified in the header)
//	  C) One .pool file (the size is specifed in the header)
//	  D) One .sdir file (the size is specified in the header)
//
typedef struct {
	u32 nProjFileBytes;
	u32 nProjFileOffset;

	u32 nPoolFileBytes;
	u32 nPoolFileOffset;

	u32 nSDirFileBytes;
	u32 nSDirFileOffset;

	//////////////////////////////////////////////////////////////////////////////
	// THIS FUNCTION MUST BE PRESENT AND MUST BE UPDATED FOR PASM TO WORK PROPERLY
	//////////////////////////////////////////////////////////////////////////////
	// call to change from Big to Little Endian and visa versa
	void ChangeEndian() { 
		nProjFileBytes = fang_ConvertEndian( nProjFileBytes );
		nProjFileOffset = fang_ConvertEndian( nProjFileOffset );
		nPoolFileBytes = fang_ConvertEndian( nPoolFileBytes );
		nPoolFileOffset = fang_ConvertEndian( nPoolFileOffset );
		nSDirFileBytes = fang_ConvertEndian( nSDirFileBytes );
		nSDirFileOffset = fang_ConvertEndian( nSDirFileOffset );
	}
} FGCData_MusyXDataHeader_t;

#endif

