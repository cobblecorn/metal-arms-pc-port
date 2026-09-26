//////////////////////////////////////////////////////////////////////////////////////
// BarterSound.h - 
//
// Author: Justin Link      
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
// 09/15/02 Link        Created.
// 05/09/03 Ranck		Rewrote.
//////////////////////////////////////////////////////////////////////////////////////
#ifndef _BARTERSOUND_H_
#define _BARTERSOUND_H_ 1

#include "fang.h"
#include "fsndfx.h"




FCLASS_NOALIGN_PREFIX class CBarterSound {
public: 
	enum Mode_e {
		MODE_NOT_PLAYING,	
		MODE_ATTRACT,
		MODE_BARTER,
		MODE_FADING_OUT,
		
		MODE_COUNT
	};


	CBarterSound();
	~CBarterSound();

	BOOL Create();

	void UpdateAttractMusic( const CFVec3A *pBarterPos_WS, f32 fRadius, const CFMtx43A *pPlayerMtx );
	void StartBarteringTune( void );
	void Stop( BOOL bFadeOut=FALSE );
	void Work( void );

private:
	Mode_e m_eCurMode;
	f32    m_fUnitVolume;
	BOOL   m_bMusicPaused;
	BOOL   m_bBarterTunePlaying;



	FCLASS_STACKMEM_NOALIGN( CBarterSound );
} FCLASS_NOALIGN_SUFFIX;



#endif

