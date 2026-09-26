//////////////////////////////////////////////////////////////////////////////////////
// skybox.h - Sky box module.
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
// 08/21/02 Ranck       Created.
//////////////////////////////////////////////////////////////////////////////////////

#ifndef _SKYBOX_H_
#define _SKYBOX_H_ 1

#include "fang.h"
#include "fmath.h"
#include "fgamedata.h"
#include "fres.h"


class CFMeshInst;



FCLASS_ALIGN_PREFIX class CSkyBox {
//----------------------------------------------------------------------------------------------------------------------------------
// Private Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
private:

	typedef struct {
		cchar *pszMeshName;
		f32 fScale;
		f32 fDispX;
		f32 fDispY;
		f32 fDispZ;
		f32 fSpinAxisX;
		f32 fSpinAxisY;
		f32 fSpinAxisZ;
		f32 fSpinSpeed;
	} UserPropData_t;




//----------------------------------------------------------------------------------------------------------------------------------
// Private Data:
//----------------------------------------------------------------------------------------------------------------------------------
private:

	CFMeshInst *m_pMeshInst;			// The mesh instance of the sky box

	CFVec3A m_Displacement;				// Displacement of sky box origin from camera
	CFVec3A m_RotUnitAxis;				// The unit axis of rotation
	f32 m_fRotSpeed;					// Rotation speed
	f32 m_fRotAngle;					// Rotation angle
	f32 m_fScale;						// Scale of sky box

	static const FGameData_TableEntry_t m_aUserPropVocab[];




//----------------------------------------------------------------------------------------------------------------------------------
// Public Functions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	CSkyBox();

	void Work( void );
	void Draw( const CFVec3A *pPos_WS );

	BOOL Create( cchar *pszMeshName );
	BOOL CreateFromCSV( FGameDataFileHandle_t hGameDataFile, cchar *pszTableName = NULL );
	void Destroy( void );




//----------------------------------------------------------------------------------------------------------------------------------
// Private Functions:
//----------------------------------------------------------------------------------------------------------------------------------
private:


	FCLASS_STACKMEM_ALIGN( CSkyBox );
} FCLASS_ALIGN_SUFFIX;




#endif

