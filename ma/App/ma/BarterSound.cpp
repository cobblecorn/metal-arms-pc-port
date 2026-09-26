//////////////////////////////////////////////////////////////////////////////////////
// BarterSound.cpp - 
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
#include "fang.h"
#include "floop.h"

#include "BarterSound.h"
#include "BarterSystem.h"
#include "level.h"


#define _BARTER_ATTRACT_STREAMING_FILENAME	"Barter_Attr"
#define _BARTER_SHOP_STREAMING_FILENAME		"Barter_Shop"

#define _BARTER_ATTRACT_VOLUME				(1.0f)
#define _BARTER_SHOP_VOLUME					(1.0f)
#define _VOLUME_FADEOUT_TIME				(1.0f)

CBarterSound::CBarterSound() {
	m_eCurMode = MODE_NOT_PLAYING;
	m_bMusicPaused = FALSE;
	m_fUnitVolume = 1.0f;
	m_bBarterTunePlaying = FALSE;
}


CBarterSound::~CBarterSound() {
}


BOOL CBarterSound::Create() {
	m_eCurMode = MODE_NOT_PLAYING;

	return TRUE;
}


void CBarterSound::StartBarteringTune( void ) {
	if( m_eCurMode != MODE_BARTER ) {
		Stop();
		level_StopAllStreams();
		level_StartStream( _BARTER_SHOP_STREAMING_FILENAME, 0, _BARTER_SHOP_VOLUME, 0, TRUE);
		m_bBarterTunePlaying = TRUE;
		m_eCurMode = MODE_BARTER;
		m_fUnitVolume = _BARTER_SHOP_VOLUME;
	}
}


void CBarterSound::Stop( BOOL bFadeOut ) {
	if( bFadeOut ) {
		// Fade out...

		if( m_eCurMode!=MODE_NOT_PLAYING && m_eCurMode!=MODE_FADING_OUT ) {
			m_eCurMode = MODE_FADING_OUT;
		}
	} else {
		// Stop immediately...
		if( m_eCurMode != MODE_NOT_PLAYING ) {
			level_StopStreamingSpeech();
            
			if (m_bBarterTunePlaying)
			{
				level_StopStream( _BARTER_SHOP_STREAMING_FILENAME ); 
				m_bBarterTunePlaying = FALSE;
			}
			
			m_eCurMode = MODE_NOT_PLAYING;
			m_fUnitVolume = 0.0f;
		}
	}
}


void CBarterSound::UpdateAttractMusic( const CFVec3A *pBarterPos_WS, f32 fRadius,const CFMtx43A *pPlayerMtx ) {
	if( m_eCurMode == MODE_BARTER ) {
		Stop();
		return;
	}

	m_fUnitVolume = _BARTER_ATTRACT_VOLUME;

	f32 fDist = pBarterPos_WS->Dist( pPlayerMtx->m_vPos );

	if( m_eCurMode == MODE_ATTRACT ) 
	{
		if ( (fDist - (fRadius * 0.5f)) < -BARTER_SYSTEM_LEVELMUSIC_PAUSE_HYSTERISIS )
		{
			if (!m_bMusicPaused )
			{
				level_PauseMusic(TRUE);
				m_bMusicPaused = TRUE;
			}
		}
		else if ( (fDist - (fRadius * 0.5f)) > BARTER_SYSTEM_LEVELMUSIC_PAUSE_HYSTERISIS )
		{
			if (m_bMusicPaused)
			{
				level_PauseMusic(FALSE);
				m_bMusicPaused = FALSE;
			}
		}


		if( fDist >= (fRadius + BARTER_SYSTEM_ATTRACT_RADIUS_HYSTERISIS) ) 
		{
			Stop();
			return;
		}
	
		// Update volume and pan...
		CFAudioStream *pAudioStream = level_GetStreamingSpeechAudioStream();
		if( pAudioStream ) {
			f32 fPan;
			CFVec3A UnitPlayerToBarterXZ_WS;

			m_fUnitVolume = _BARTER_ATTRACT_VOLUME * (1.0f - fDist * fmath_Inv(fRadius));
			FMATH_CLAMP_UNIT_FLOAT( m_fUnitVolume );

			pAudioStream->SetVolume( m_fUnitVolume );

			if( fDist > 0.01f ) {
				UnitPlayerToBarterXZ_WS.Sub( *pBarterPos_WS, pPlayerMtx->m_vPos ).Unitize();
				fPan = UnitPlayerToBarterXZ_WS.Dot( pPlayerMtx->m_vRight );
				FMATH_CLAMP( fPan, -1.0f, 1.0f );
			} else {
				fPan = 0.0f;
			}

			pAudioStream->SetPan( fPan );
		}	
	} 
	else 
	{
		// Not yet playing attract music...

		if( fDist < (fRadius - BARTER_SYSTEM_ATTRACT_RADIUS_HYSTERISIS) ) {
			level_PlayStreamingSpeech( _BARTER_ATTRACT_STREAMING_FILENAME, _BARTER_ATTRACT_VOLUME, 0 );
			m_eCurMode = MODE_ATTRACT;
		}

	}
}


void CBarterSound::Work( void ) {
	if( m_eCurMode == MODE_FADING_OUT ) {
		m_fUnitVolume -= (1.0f / _VOLUME_FADEOUT_TIME) * FLoop_fPreviousLoopSecs;
		if( m_fUnitVolume <= 0.0f ) {
			Stop();
			return;
		}

		CFAudioStream *pAudioStream = level_GetStreamingSpeechAudioStream();
		if( pAudioStream ) {
			pAudioStream->SetVolume( m_fUnitVolume );
		}
	}
}



