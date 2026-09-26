
//////////////////////////////////////////////////////////////////////////////////////
// MaiQueryDetails.h - 
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
#ifndef _MAIQUERYDETAILS_H_
#define _MAIQUERYDETAILS_H_ 1

#include "fang.h"
#include "AI\ftl.h"

#if defined(__cplusplus)
extern "C" {
#endif


class CQueryDetails
{
public:
	CQueryDetails(void) : m_pszName(NULL), m_fWidth(0.0f), m_fHeight(0.0f), m_bCanJump(FALSE), m_bSurfaceMover(TRUE), m_b3DMover(TRUE) { }
	~CQueryDetails(void);

	void Init(const char* pszName, float fWidth, float fHeight, BOOL bCanJump, BOOL bSurfaceMover, BOOL b3DMover);
	void CopyName(const char* pszName);

	const char* GetName(void) {return m_pszName;};

	enum
	{
		IS_SURFACE_MOVER	= TRUE,
		IS_3D_MOVER			= TRUE,
		CAN_JUMP			= TRUE,
	};
	char *m_pszName;
	float m_fWidth;
	float m_fHeight;
	float m_fSlopeCost;
	BOOL m_bCanJump;
	BOOL m_bSurfaceMover;
	BOOL m_b3DMover;
};


#if defined(__cplusplus)
}
#endif



#endif

