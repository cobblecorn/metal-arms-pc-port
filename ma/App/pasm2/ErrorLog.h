//////////////////////////////////////////////////////////////////////////////////////
// ErrorLog.h - "manages writting data to an error log file (text mode)
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
// 01/23/01 Starich     Created.
//////////////////////////////////////////////////////////////////////////////////////
#ifndef _ERROR_LOG_H_
#define _ERROR_LOG_H_ 1

#include "fang.h"

class CErrorLog
{
public:
	CErrorLog();// don't call this, use GetCurrent(), we only 1 instance of this 
				// class in the application, and is declared locally to this .cpp file
	~CErrorLog();
	
	static void SetFilename( cchar *pszFullFilename );
    static CErrorLog& GetCurrent();
	static cchar *GetErrorFilename();
		
	void WriteErrorHeader( cchar *pszTitle );
	void WriteErrorLine( cchar *pszLine );
	void ClearErrorLog();
private:	
	
};

#endif

