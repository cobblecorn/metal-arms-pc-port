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

