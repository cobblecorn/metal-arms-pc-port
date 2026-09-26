//////////////////////////////////////////////////////////////////////////////////////
// CamCutscene.h - A cameraman driven by a CFCamAnimInst object.
//
// Author: Russell Foushee
//////////////////////////////////////////////////////////////////////////////////////
// THIS CODE IS PROPRIETARY PROPERTY OF SWINGIN' APE STUDIOS, INC.
// Copyright (c) 2003
//
// The contents of this file may not be disclosed to third
// parties, copied or duplicated in any form, in whole or in part,
// without the prior written permission of Swingin' Ape Studios, Inc.
//////////////////////////////////////////////////////////////////////////////////////
// Modification History:
//
// Date     Who         Description
// -------- ----------  --------------------------------------------------------------
// 01/24/03 Foushee		Created.
//////////////////////////////////////////////////////////////////////////////////////
#ifndef _CAMCUTSCENE_H_
#define _CAMCUTSCENE_H_ 1

// =============================================================================================================

#include "fang.h"
#include "fcamera.h"
#include "fcamanim.h"

// =============================================================================================================

class CCamCutsceneInfo
{
public:
	// Constructors:	
	CCamCutsceneInfo() : m_pCamAnimInst(NULL), m_bDriveAnimInst(TRUE), m_bDone(FALSE) {}

	void SetCamAnimInst( CFCamAnimInst* pCamAnimInst ) { m_pCamAnimInst = pCamAnimInst; m_bDone = FALSE; }
	void DriveAnimInst( BOOL bDriveAnimInst ) { m_bDriveAnimInst = bDriveAnimInst; }
	BOOL IsDone( void ) { return m_bDone; }
	CFCamAnimInst* GetCamAnimInst( void ) { return m_pCamAnimInst; }
private:
	CFCamAnimInst *m_pCamAnimInst;
	BOOL m_bDriveAnimInst;
	BOOL m_bDone;

	friend class CCamCutscene;

	FCLASS_STACKMEM_NOALIGN(CCamCutsceneInfo);
};

// =============================================================================================================

FCLASS_ALIGN_PREFIX class CCamCutscene : public CFCameraMan
{
public:
	CCamCutscene();
//	~CCamCutscene();

	static BOOL InitSystem();
	static void UninitSystem();

	BOOL Init(CCamCutsceneInfo *pCamInfo);
	void Reset();
	void Work();

private:
	f32 m_fHalfFOV;
	CFQuatA m_qOrient;
	CFVec3 m_vPos;
	CCamCutsceneInfo *m_pCamInfo;
	

	FCLASS_STACKMEM_ALIGN(CCamCutscene);
} FCLASS_ALIGN_SUFFIX;

#endif