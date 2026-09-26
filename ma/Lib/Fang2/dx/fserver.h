//////////////////////////////////////////////////////////////////////////////////////
// fserver.h - Master file update server for Fang
//
// Author: Nathan Miller
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
// 09/12/02 Miller		Created.
//////////////////////////////////////////////////////////////////////////////////////
#ifndef _FSERVER_H_ 
#define _FSERVER_H_ 1

#include "fang.h"

// !! IF YOU CHANGE ANY OF THIS MAKE SURE YOU REBUILD BOTH CLIENT AND SERVER
//    THIS INCLUDES ALL APPLICATIONS THAT MAKE USE OF THE SERVER

// Be careful with these values.  The larger FSERVER_MAX_COMMANDS is, the larger
// FSERVER_MAX_SEND_BUFFER must be.  Currently, ~800 bytes are used out of the 
// FSERVER_MAX_SEND_BUFFER size.  If you want to see how many bytes are being used,
// uncomment the DEVPRINTF() in _SendNotificationBatch().  _SendNotificationBatch() is
// used only for sending master file directory information over quickly.
#define FSERVER_MAX_SEND_BUFFER       1024 // Buffer for sending and receiving
#define FSERVER_MAX_COMMANDS          20   // Max number of commands we can put into a buffer in _SendNotificationBatch()
#define FSERVER_MAX_COMMAND_ARGUMENTS 10   // Max arguments in a given command

#define FSERVER_MAX_CLIENT_NAME       64
#define FSERVER_COMMAND_PREFIX		  "XSRV"
#define FSERVER_COMMAND_PREFIX_LEN    4
#define FSERVER_FILE_SEND_WORK_MSG    5

#define FSERVER_ROOT_DIR              "xe:\\sas\\"
#define FSERVER_CLIENT_TEMP_DIR		  "xe:\\sas\\temp\\"
#define FSERVER_FULL_MASTER_PATH      "xe:\\sas\\mettlearms_xb.mst"
#define FSERVER_SERVER_TEMP_DIR	      "d:\\temp\\"
#define FSERVER_MASTER_FILE_NAME      "d:\\mettlearms_xb.mst"

#define FSERVER_ITEM_TOKEN            '|'
#define FSERVER_COMMAND_TOKEN         ':'

#ifndef _FANGDEF_PRODUCTION_BUILD
#ifdef _FANGDEF_PLATFORM_XB

extern BOOL fserver_ModuleStartup( void );
extern void fserver_ModuleShutdown( void );
extern BOOL fserver_GetTakeover( void );
extern void fserver_SetTakeover( BOOL bTake );
extern void fserver_Reset( void );
extern BOOL fserver_Work( void );
extern BOOL fserver_ShutdownApplication( void );
extern BOOL fserver_GraphicsStartup( void );

#endif
#endif

#endif
