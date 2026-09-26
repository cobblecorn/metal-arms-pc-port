//////////////////////////////////////////////////////////////////////////////////////
// SeqBank.h - 
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
// 06/17/02 Starich     Created.
//////////////////////////////////////////////////////////////////////////////////////
#ifndef _SEQBANK_H_
#define _SEQBANK_H_ 1

#include "fang.h"
#include "WavBank.h"
#include "fversion.h"

#define SEQ_BANK_STRING_LEN				256
#define SEQ_BANK_MAX_FX_NAME_LEN		16	// includes a NULL terminator
#define SEQ_BANK_VERSION_NUM			0x00000002
#define SEQ_BANK_LARGEST_CMD_BYTES		64

typedef struct {
	char szBankFileUsed[SEQ_BANK_STRING_LEN];
	
	u32 nSeqSortColumn;
	BOOL bSeqAscendingSort;
	
	u32 nBankSortColumn;
	BOOL bBankAscendingSort;
} SeqBank_ToolSettings_t;

typedef struct {
	u32 nSignature;		// should always == FVERSION_FILE_SIGNATURE
	u32 nFileVersion;	// should always == SEQ_BANK_VERSION_NUM
	u32 nNumBytes;
	u32 nNumFxs;		// how many SeqBank_Fx_t follow this header
	u32 nNumCmds;		// how many SeqBank_Cmd_t follow the last Fx
	u32 nModifiedTime;	// Really the file's time_t structure containing the file's last write time
	u32 nSizeOfEachCmd;	// should always == SEQ_BANK_LARGEST_CMD_BYTES

	// user names to keep track of who created and modified this bank
	char szCreatedBy[SEQ_BANK_STRING_LEN];
	char szLastModifiedBy[SEQ_BANK_STRING_LEN];
	
	// settings used solely by the SndSeqBanker Tool to restore the last settings used to create this file
	SeqBank_ToolSettings_t Settings;
} SeqBank_Header_t;

typedef enum {
	SEQ_BANK_CMD_TYPE_UNUSED = 0,
	SEQ_BANK_CMD_TYPE_PLAY,

	SEQ_BANK_CMD_TYPE_COUNT
} SeqBank_CmdType_e;

typedef struct {
	char szFilename[WAV_BANK_MAX_WAV_FILENAME_LEN];// the wav filename, without extension
	u32 nNumLoops;		// 0 is loop forever, any other number is a discreet number of loops
	u32 nVolPercent;	// how loud should we play the wav (100 is full volume, 0 if lowest volume)
	s32 nPitchPercent;	// how much faster or slower should the wav be played (0 is normal freq, 100 is 100% faster or 2 times the normal freq...)
} SeqBank_PlayCmd_t;

typedef struct {
	u32 nCmdIndex;		// what command is this in the Fx
	SeqBank_CmdType_e nCmdType;
	union {
		SeqBank_PlayCmd_t Play;

		u8 nPadding[SEQ_BANK_LARGEST_CMD_BYTES];// all cmd structs should be smaller than this var
	} TypeData;	
} SeqBank_Cmd_t;

typedef struct {
	u32 nNumCmds;			// how many cmds are there in the cmd array
	u32 nNumCmdBytes;		// the number of bytes pointed to by paCmds ( sizeof( SeqBank_Cmd_t ) * nNumCmds )	
	u32 nDataOffset;		// in bytes from the head of the bank file
	SeqBank_Cmd_t *paCmds;	// points to the 1st cmd in the cmd array
	char szFxName[SEQ_BANK_MAX_FX_NAME_LEN];
	
	// only used at tool time
	u32 nNumAllocatedCmds;	// how many Cmds were allocated (this number could be larger than nNumCmds so that we can add Cmds easier)		
} SeqBank_Fx_t;

class CSeqBank
{
public:
	CSeqBank();
	~CSeqBank();

	BOOL SetFileSettings( SeqBank_ToolSettings_t *pSettings );
	SeqBank_ToolSettings_t *GetFileSettings();

	BOOL CreateEmpty();
	BOOL IsEmpty();
	u32 GetBankSize();

	BOOL Read( cchar *pszFilename );
	BOOL Write( cchar *pszFilename );
	
	u32 GetNumFxs();
	u32 GetNumCmds();

	BOOL DeleteFxByIndex( u32 nIndex );
	BOOL DeleteFx( SeqBank_Fx_t *pFx );

	SeqBank_Fx_t *GetFxByIndex( u32 nIndex );
	SeqBank_Fx_t *GetFxByName( cchar *pszName );
	SeqBank_Fx_t *AddFx( cchar *pszFxName );
	
	SeqBank_Cmd_t *AddNewCmd( SeqBank_Fx_t *pFx, SeqBank_CmdType_e nType );
	BOOL DeleteCmdByIndex( SeqBank_Fx_t *pFx, u32 nIndex );
	BOOL DeleteCmd( SeqBank_Fx_t *pFx, SeqBank_Cmd_t *pCmd );	

private:
	void FreeData();
	void RemoveFx( SeqBank_Fx_t *pFx, u32 nIndex );

protected:
	SeqBank_Header_t m_Header;
	CPtrArray m_apFx;// each element is a SeqBank_Fx_t *	
};






#endif

