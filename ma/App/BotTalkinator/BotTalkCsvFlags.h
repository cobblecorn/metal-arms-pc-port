//////////////////////////////////////////////////////////////////////////////////////
// BotTalkCsvFlags.h - 
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
// 10/01/02 Starich     Created.
//////////////////////////////////////////////////////////////////////////////////////
#ifndef _BOT_TALK_CSV_FLAGS_H_
#define _BOT_TALK_CSV_FLAGS_H_ 1

#include "fang.h"

// DON'T PUT AN CODE IN THIS FILE, THESE FLAGS ARE SHARED BETWEEN 
// THE GAME AND THE BOT TALK TOOL.
// 
// IF YOU ADD OR CHANGE THESE FLAGS, THE BOT TALK TOOL WILL NEED UPDATING.
enum BotTalkCsvFlags_e {
	BOT_TALK_CSV_FLAG_IS_INITTED			=	0x00000001,
	BOT_TALK_CSV_FLAG_DRIVES_UPPER_BODY		=	0x00000002,
	BOT_TALK_CSV_FLAG_DRIVES_LOWER_BODY		=	0x00000004,
	BOT_TALK_CSV_FLAG_HAS_TALK_LOOP			=	0x00000008,
	BOT_TALK_CSV_FLAG_NO_BLEND_OUT			=	0x00000010,
	BOT_TALK_CSV_FLAG_STICK_AT_END			=	0x00000020,
	// new bone mask flags, upper and lower are still supported too
	BOT_TALK_CSV_FLAG_DRIVES_UPPER_TORSO	=	0x00000040,
	BOT_TALK_CSV_FLAG_DRIVES_LOWER_TORSO	=	0x00000080,
	BOT_TALK_CSV_FLAG_DRIVES_LEFT_ARM		=	0x00000100,
	BOT_TALK_CSV_FLAG_DRIVES_RIGHT_ARM		=	0x00000200,
	BOT_TALK_CSV_FLAG_DRIVES_HEAD			=	0x00000400,

	// masks all of the bone driving flags
	BOT_TALK_CSV_FLAG_DRIVES_BONE_MASK		=	( BOT_TALK_CSV_FLAG_DRIVES_UPPER_BODY |
												  BOT_TALK_CSV_FLAG_DRIVES_LOWER_BODY |
												  BOT_TALK_CSV_FLAG_DRIVES_UPPER_TORSO |
												  BOT_TALK_CSV_FLAG_DRIVES_LOWER_TORSO |
												  BOT_TALK_CSV_FLAG_DRIVES_LEFT_ARM |
												  BOT_TALK_CSV_FLAG_DRIVES_RIGHT_ARM |
												  BOT_TALK_CSV_FLAG_DRIVES_HEAD ),

	BOT_TALK_CSV_FLAG_2DSOUNDS				=	0x00000800,
	BOT_TALK_CSV_FLAG_NONE					=	0x00000000,
};

// here is the format of the "BotTalk" table in BotTalk generated csv files
typedef struct {
	f32 fTotalTime;
	u32 nFlags;			// see BotTalkCsvFlags_e...
} BotTalkCsv_BotTalkTable_t;

// here is the format of the "AnimList" table in BotTalk generated csv files
typedef struct {
	u32 nAnimCount;
	cchar **ppszAnimFiles;	// there are nAnimCount animation filenames that follow the count
} BotTalkCsv_AnimListTable_t;


typedef enum {
	BOT_TALK_CSV_ACTION_TYPE_ANIMATION = 0,
	BOT_TALK_CSV_ACTION_TYPE_SOUND,
	BOT_TALK_CSV_ACTION_TYPE_STREAM,
	BOT_TALK_CSV_ACTION_TYPE_SFX,

	BOT_TALK_CSV_ACTION_TYPE_COUNT
} BotTalkCsv_ActionType_e;

typedef struct {
	u32 nAnimIndex;
	u32 nFlags;
	u32 nAnimSlot;
} BotTalkCsv_AnimAction_t;

typedef struct {
	cchar *pszSoundFilename;
	u32 nFlags;
	u32 nNotUsed;
} BotTalkCsv_SoundAction_t;

typedef struct {
	cchar *pszStreamFilename;
	u32 nFlags;
	u32 nNotUsed;
} BotTalkCsv_StreamAction_t;

typedef struct {
	cchar *pszSfxName;
	u32 nFlags;
	u32 nNotUsed;
} BotTalkCsv_SfxAction_t;

typedef struct {
	f32 fStartTime;
	f32 fLength;
	BotTalkCsv_ActionType_e nType;

	union {
		BotTalkCsv_AnimAction_t		AnimAction;
		BotTalkCsv_SoundAction_t	SoundAction;
		BotTalkCsv_StreamAction_t	StreamAction;
		BotTalkCsv_SfxAction_t		SfxAction;
	} TypeSpecificData;
} BotTalkCsv_Action_t;

// here is the format of the "ActionList" table in BotTalk generated csv files
typedef struct {
	u32 nActionCount;
	BotTalkCsv_Action_t **ppActions;	// there are nActionCount of these structs
} BotTalkCsv_ActionListTable_t;

#define BOT_TALK_CSV_NUM_ELEMENTS_PER_ACTION		( sizeof( BotTalkCsv_Action_t ) >> 2 )

#endif

