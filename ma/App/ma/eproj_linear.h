//////////////////////////////////////////////////////////////////////////////////////
// eproj_linear.h - Linear projectiles.
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
// 06/19/02 Ranck       Created.
//////////////////////////////////////////////////////////////////////////////////////

#ifndef _EPROJ_LINEAR_H_
#define _EPROJ_LINEAR_H_ 1

#include "fang.h"
#include "eproj.h"
#include "fcoll.h"


class CEProj;



//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CEProj_Linear
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

FCLASS_ALIGN_PREFIX class CEProj_Linear : public CEProjExt {
//----------------------------------------------------------------------------------------------------------------------------------
// Private Data:
//----------------------------------------------------------------------------------------------------------------------------------
private:

	
	
	
//----------------------------------------------------------------------------------------------------------------------------------
// Public Functions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	// Construct/Destruct:
	CEProj_Linear() {}
	~CEProj_Linear() {}


	// Extension interface:
	virtual void Detonated( CEProj *pProj, BOOL bMakeEffect, u32 nEvent, FCollImpact_t *pCollImpact );
	virtual BOOL Work( CEProj *pProj );


	FCLASS_STACKMEM_ALIGN( CEProj_Linear );
} FCLASS_ALIGN_SUFFIX;



#endif

