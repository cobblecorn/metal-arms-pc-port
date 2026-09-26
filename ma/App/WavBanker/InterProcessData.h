//////////////////////////////////////////////////////////////////////////////////////
// InterProcessData.h - 
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
// 01/03/02 Starich     Created.
//////////////////////////////////////////////////////////////////////////////////////
#ifndef _INTER_PROCESS_DATA_H_
#define _INTER_PROCESS_DATA_H_ 1

#include "fang.h"

typedef enum {
	APP_TYPE_PASM = 0,
	APP_TYPE_FONTOMATIC, 
	APP_TYPE_JAWS,
	APP_TYPE_K9,
	APP_TYPE_MAWIN,
	APP_TYPE_APP_LAUNCHER,
	APP_TYPE_STAR_COMMAND,
	APP_TYPE_WAV_BANKER,
	APP_TYPE_WAV_COMPILER,
	APP_TYPE_GC_BANK_CONVERTER,
	APP_TYPE_SEQUENCER,
	APP_TYPE_SPIT,
	APP_TYPE_BOT_TALK,
	APP_TYPE_LIGHTMAPTOOL,

	APP_TYPE_COUNT
} InterProcessData_AppType_e;

// use the guid generator to create a unique number and then append the app name on
#define APP_NAMES_PASM						"8AD1D504-9117-457e-A01A-4B0767CD0265 PASM"
#define APP_NAMES_FONTOMATIC				"CC243C50-0FF9-46f8-8FC5-4F39F05C2647 FONTOMATIC"
#define APP_NAMES_JAWS						"26226CCC-D311-4c5a-AA2A-7F50BEA9B5C7 JAWS"
#define APP_NAMES_K9						"6472A0FD-11DC-436f-905B-3DFBB4E4CC19 K9"
#define APP_NAMES_MAWIN						"0FCC0B2B-FF4D-43fb-A368-C9772529FEF5 MAWIN"
#define APP_NAMES_APE_APP_LAUNCHER			"D11761D2-DD3B-422a-A123-019B86995B97 APE APP LAUNCHER"
#define APP_NAMES_STAR_COMMAND				"990BE3C7-FA39-406a-B53A-C92BEB229DF1 STAR COMMAND"
#define APP_NAMES_WAV_BANKER				"7BEE8A98-CFFA-4e36-BEB2-9B3FEE9DFAD2 WAV BANKER"
#define APP_NAMES_WAV_COMPILER				"F345427F-1B68-47b9-8622-91094CE24F09 WAV COMPILER"
#define APP_NAMES_GC_BANK_CONVERTER			"44D1104D-37ED-4181-8164-9BEEC1024DF3 GC BANK CONVERTER"
#define APP_NAMES_SEQUENCER					"D10E0835-A6CD-4ac9-A4F5-911335661C9A SEQUENCER"
#define APP_NAMES_SPIT						"B98B90A1-C614-41ad-9BFA-E94DE33D4210 SPIT"
#define APP_NAMES_BOT_TALK					"8893F798-C592-4a33-96B0-BDA0C714E633 BOTTALK"
#define APP_NAMES_LIGHTMAPTOOL				"F2FEE370-688E-4a42-A3A2-9DE67EE1B564 LIGHTMAPTOOL"

#define INTER_PROCESS_SHARED_FILE_NAME		"35BEA1C0-37FA-4646-A17A-81F03E7D1BC4"

// Ape Ape Launch Pad Message Codes
#define APP_MSG_CODES_LAUNCHER_2ND_APP_RUN		( WM_USER + 300 )
#define APP_MSG_CODES_LAUNCHER_ALREADY_BUSY		( WM_USER + 301 )
#define APP_MSG_CODES_LAUNCHER_PASM_DONE		( WM_USER + 302 )

// PASM Message Codes
#define APP_MSG_CODES_PASM_CHECK_MASTER_FILE_VERSION	( WM_USER + 300 )// USED INTERNALLY IN PASM, DON'T SEND THIS MESSAGE FROM OUTSIDE PASM
#define APP_MSG_CODES_PASM_RESCAN_AND_COMPILE_LOCAL		( WM_USER + 301 )

// K9 Message Codes
#define APP_MSG_CODES_K9_LAUNCH					( WM_USER + 300 )

// Jaws Message Codes
#define APP_MSG_CODES_JAWS_LAUNCH				( WM_USER + 300 )

#define APP_MSG_CODES_LIGHTMAPTOOL_LAUNCH		( WM_USER + 300 )


#define APP_STRING_LEN							256

typedef struct {
	BOOL bDataInUse;
	u32 nNumStrings;// how many strings are seperated by ':', not at the start or end
	char szString[APP_STRING_LEN];
} AppData_t;

// shared data between different applications
typedef struct {
	AppData_t aAppData[APP_TYPE_COUNT];
} InterProcessData_t;

#endif

