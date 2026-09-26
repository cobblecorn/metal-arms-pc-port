//////////////////////////////////////////////////////////////////////////////////////
// MaiQueryDetails.cpp - 
//
// Author: Pat MacKellar 
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
// 04/09/02 MacKellar   Created.
//////////////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "fang.h"
#include "MaiQueryDetails.h"
#include <string.h>

//====================
// private definitions

//=================
// public variables

//==================
// private variables

//===================
// private prototypes

//=================
// public functions
CQueryDetails::~CQueryDetails(void)
{
	delete [] m_pszName;
	m_pszName = NULL;
};

void CQueryDetails::Init(const char* pszName, float fWidth, float fHeight, BOOL bCanJump, BOOL bSurfaceMover, BOOL b3DMover)
{
	CopyName(pszName);
	m_fWidth = fWidth;
	m_fHeight = fHeight;
	m_bCanJump = bCanJump;
	m_bSurfaceMover = bSurfaceMover;
	m_b3DMover = b3DMover;
}


void CQueryDetails::CopyName(const char* pszName)
{
	delete [] m_pszName; m_pszName = NULL;
	if (pszName)
	{
		int nLen = strlen(pszName)+1;
		m_pszName = new char[nLen];
		strcpy(m_pszName, pszName);
	}
}


//==================
// private functions



