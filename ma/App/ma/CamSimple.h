//////////////////////////////////////////////////////////////////////////////////////
// CamSimple.h - Simple camera.
//
// Author: Steve Ranck     
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
// 08/01/02 Ranck       Created.
//////////////////////////////////////////////////////////////////////////////////////

#ifndef _CAMSIMPLE_H_
#define _CAMSIMPLE_H_ 1

#include "fang.h"
#include "fcamera.h"
#include "fmath.h"


class CCamSimpleInfo {
public:

	CFVec3A m_Pos_WS;					// Camera position
	CFQuatA m_Quat_WS;					// Camera orientation

	CCamSimpleInfo() { m_Pos_WS.Zero(); m_Quat_WS.Identity(); }


	FCLASS_STACKMEM_NOALIGN( CCamSimpleInfo );
};



FCLASS_ALIGN_PREFIX class CCamSimple : public CFCameraMan {
private:
	CCamSimpleInfo *m_pCamInfo;


public:
	CCamSimple();

	static BOOL InitSystem();
	static void UninitSystem();

	BOOL Init( CCamSimpleInfo *pCamInfo );
	void Reset();
	void Work();


	FCLASS_STACKMEM_ALIGN( CCamSimple );
} FCLASS_ALIGN_SUFFIX;


#endif


