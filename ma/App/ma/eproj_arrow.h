//////////////////////////////////////////////////////////////////////////////////////
// eproj_arrow.h - Arrow projectiles.
//
// Author: John Lafleur   
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
// 06/19/02 Lafleur		Created.
// 03/19/03 Ranck		Took ownership and modified to support the new Rivet weapon.
//////////////////////////////////////////////////////////////////////////////////////

#ifndef _EPROJ_ARROW_H_
#define _EPROJ_ARROW_H_ 1

#include "fang.h"
#include "eproj.h"


class CEProj;
class CEParticle;




//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CEProj_Arrow
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

FCLASS_ALIGN_PREFIX class CEProj_Arrow : public CEProjExt {
//----------------------------------------------------------------------------------------------------------------------------------
// Private Data:
//----------------------------------------------------------------------------------------------------------------------------------
private:

	CFVec3A m_VibeRotUnitAxis_WS;			// When vibrating, this is the unit axis to vibrate around

	f32 m_fVibeUnitAmplitude;
	f32 m_fVibePolarity;
	f32 m_fLifeSecs;
	f32 m_fInitialScale;
	f32 m_fUnitSteamIntensity;

	BOOL8 m_bTerminateMode;					// TRUE if we're shrinking or blowing steam
	CEParticle *m_pParticleSteam;			// Steam particle (NULL=none)
	CFAudioEmitter *m_pAudioEmitterSteam;	// Steam sound

	CEProj_Arrow_Params_t m_SetupParams;	// pCollImpact must persist through the call to CEProj::Launch()




//----------------------------------------------------------------------------------------------------------------------------------
// Public Functions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	// Construct/Destruct:
	CEProj_Arrow() {}
	~CEProj_Arrow() {}

	// Extension interface:
	virtual BOOL Create( CEProj *pProj );

	virtual void Init( CEProj *pProj );
	virtual void Launched( CEProj *pProj );
	virtual BOOL Detonated( CEProj *pProj, BOOL bMakeEffect, u32 nEvent, FCollImpact_t *pCollImpact );

	virtual BOOL Work( CEProj *pProj );

	virtual void SetArrowParams( CEProj *pProj, const CEProj_Arrow_Params_t *pParams );

	FINLINE BOOL IsInTerminateMode( void ) const { return m_bTerminateMode; }




//----------------------------------------------------------------------------------------------------------------------------------
// Private Functions:
//----------------------------------------------------------------------------------------------------------------------------------
private:

	void _SetBotPanicSecs( f32 fPanicSecs );
	void _StartSteamSound( CEProj *pProj );
	void _KillSteamSound( CEProj *pProj );
	void _UpdateSteamSound( CEProj *pProj );
	void _UpdateSteamPitchMult( CEProj *pProj );


	FCLASS_STACKMEM_ALIGN( CEProj_Arrow );
} FCLASS_ALIGN_SUFFIX;




#endif

