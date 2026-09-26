//////////////////////////////////////////////////////////////////////////////////////
// AIEnviro.cpp - 
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
// 07/03/02 MacKellar   Created.
//////////////////////////////////////////////////////////////////////////////////////
#include "fang.h"
#include "fres.h"
#include "AIEnviro.h"
#include "ftl.h"
#include "AIGameUtils.h"
#include "AIBrain.h"
#include "AIMover.h"
#include "../entity.h"
#include "../player.h"
#include "../bot.h"
#include "../Weapon_Laser.h"

static CNiList<CAISound*>* _pActiveSounds = NULL;
static CNiList<CAISound*>* _pVisibleEntities = NULL;
static CNiBank<CAISound>* _pSoundBank = NULL;
CAISound* CNiIterator<CAISound *>::s_ReturnError;
AISoundHandle _aPlayerSoundHandles[MAX_PLAYERS];
f32 _afPlayerSoundVols[MAX_PLAYERS];

u16 _uHandleGUID = 1;
f32 AIEnviro_fExplosionSoundRadToAIAudibleRadScale = 1.0f;
f32 AIEnviro_fGrenadeAudibleRange = 50.0f;
f32 AIEnviro_fLaserImpactAudibleRange = 25.0f;
f32 AIEnviro_fProjectileVisualRadius = 4.0f;
f32 AIEnviro_fVehicleBotSoundRadius = 50.0f;
f32 AIEnviro_fFlamerImpactSoundRadius = 5.0f;
f32 AIEnviro_fMagmaImpactSoundRadius = 15.0f;

BOOL AIEnviro_InitSystem(	struct FLinkRoot_s* pGlobalPtrNodePool,
							u32 nSoundBankSize /*= AIENVIRO_DEFAULT_SOUND_GLOBAL_BANK_SIZE*/)
{
	u32 i = 0;
	FASSERT(_pSoundBank == NULL && _pActiveSounds == NULL && _pVisibleEntities == NULL);

	FResFrame_t ResFrame = fres_GetFrame();

	_pSoundBank = APE_NEW CNiBank<CAISound>(pGlobalPtrNodePool, nSoundBankSize, APE_NEW CAISound[nSoundBankSize]);
	if (!_pSoundBank)
	{
		goto _ExitInitEnviroWithError;
	}
	_pActiveSounds = APE_NEW CNiList<CAISound*>(pGlobalPtrNodePool);
	if (!_pActiveSounds)
	{
		goto _ExitInitEnviroWithError;
	}

	_pVisibleEntities = APE_NEW CNiList<CAISound*>(pGlobalPtrNodePool);
	if (!_pVisibleEntities)
	{
		goto _ExitInitEnviroWithError;
	}

	for (i = 0; i < MAX_PLAYERS; i++)
	{
		_afPlayerSoundVols[i] = 0.0f;
		_aPlayerSoundHandles[i] = 0;
	}

	return TRUE;
_ExitInitEnviroWithError:
	DEVPRINTF( "aiEnviro_InitSystem(): Out of memory for enviro.\n" );
	fres_ReleaseFrame( ResFrame );
	_pSoundBank = NULL;
	_pActiveSounds = NULL;
	_pVisibleEntities = NULL;

	return FALSE;
}


void AIEnviro_UninitSystem(void)
{
	AIEnviro_KillAllSounds();
	APE_DELETE(_pActiveSounds); _pActiveSounds = NULL;
	APE_DELETE(_pSoundBank); _pSoundBank = NULL;
	APE_DELETE(_pVisibleEntities); _pVisibleEntities = NULL;
}

f32 _fNowTime = 0.0f;

static void _WorkPerSound(CAISound* pSound)
{
	FASSERT(pSound);
	if (pSound->m_uSoundCtrlFlags & AISOUNDCTRL_NEW_CLEAR)
	{
		pSound->m_uSoundCtrlFlags &= ~AISOUNDCTRL_NEW_SOUND;
		pSound->m_uSoundCtrlFlags &= ~AISOUNDCTRL_NEW_CLEAR;
	}

	if (pSound->m_uSoundCtrlFlags & AISOUNDCTRL_MODIFIED_CLEAR)
	{
		pSound->m_uSoundCtrlFlags &= ~AISOUNDCTRL_MODIFIED_SOUND;
		pSound->m_uSoundCtrlFlags &= ~AISOUNDCTRL_MODIFIED_CLEAR;
	}

	if (pSound->m_uSoundCtrlFlags & AISOUNDCTRL_MODIFIED_SOUND)
	{
		pSound->m_uSoundCtrlFlags |= AISOUNDCTRL_MODIFIED_CLEAR;
	}

	if (pSound->m_uSoundCtrlFlags & AISOUNDCTRL_NEW_SOUND)
	{
		pSound->m_uSoundCtrlFlags |= AISOUNDCTRL_NEW_CLEAR;
	}
}


void AIEnviro_Work(void)
{
	_fNowTime = aiutils_GetCurTimeSecs();
	
	//remove stale sounds
	CNiIterator<CAISound*> it = _pActiveSounds->Begin();
	while (it.IsValid())
	{
		CAISound* pSound = it.Get();
		if (!(pSound->m_uSoundCtrlFlags & AISOUNDCTRL_PERSISTENT) && pSound->m_fWhen+pSound->m_fHowLong < _fNowTime)
		{
			it.RemoveThenNext();
			_pSoundBank->Recycle(pSound);
		}
		else
		{
			_WorkPerSound(pSound);
		}
		it.Next();
	}

	//remove stale visible entities
	it = _pVisibleEntities->Begin();
	while (it.IsValid())
	{
		CAISound* pSound = it.Get();
		if (!(pSound->m_uSoundCtrlFlags & AISOUNDCTRL_PERSISTENT) && pSound->m_fWhen+pSound->m_fHowLong < _fNowTime)
		{
			it.RemoveThenNext();
			_pSoundBank->Recycle(pSound);
		}
		else
		{
			_WorkPerSound(pSound);
		}
		it.Next();
	}
	
	//update/add some special, (hard-coded) sounds into the list
	for (u32 i = 0; i < MAX_PLAYERS; i++)
	{
		CPlayer* pPlayer = Player_aPlayer+i;
		if (!pPlayer || !pPlayer->m_pEntityCurrent)
		{
			continue;
		}

		f32 fPlayerSoundVolumeRad = 0.0f;
		if ((pPlayer->m_pEntityCurrent->TypeBits() & ENTITY_BIT_BOT))
		{
			CBot* pBot =  (CBot*) pPlayer->m_pEntityCurrent;
			if (pBot->m_fSpeed_WS > 0.5f)
			{
//				if (pBot->IsSneaking())
				{
					fPlayerSoundVolumeRad = 1.0f;
				}
			}

			if (_afPlayerSoundVols[i] > fPlayerSoundVolumeRad)
			{
				fPlayerSoundVolumeRad = _afPlayerSoundVols[i];
			}
		}

		CFVec3A PlrSoundOrigin = pPlayer->m_pEntityCurrent->MtxToWorld()->m_vPos;
		if (pPlayer->m_pEntityCurrent->AIBrain())
		{
			PlrSoundOrigin.y+=pPlayer->m_pEntityCurrent->AIBrain()->GetAIMover()->GetHeight();
		}
		if (!AIEnviro_ModifySound(_aPlayerSoundHandles[i], PlrSoundOrigin, fPlayerSoundVolumeRad, 1.0f, AISOUNDTYPE_BOT, pPlayer->m_pEntityCurrent))
		{
			_aPlayerSoundHandles[i] = AIEnviro_AddSound(PlrSoundOrigin, fPlayerSoundVolumeRad, 1.0f, AISOUNDTYPE_BOT|AISOUNDCTRL_PERSISTENT, 0, pPlayer->m_pEntityCurrent);
		}

		if (_afPlayerSoundVols[i] > 0.0f)
		{
			_afPlayerSoundVols[i] -= FLoop_fPreviousLoopSecs*25.0f;
			FMATH_CLAMPMIN(_afPlayerSoundVols[i], 0.0f);
		}
	}
}

void AIEnviro_BoostPlayerSoundTo(u32 uPlayerId, f32 fRad)
{
	if (fRad > _afPlayerSoundVols[uPlayerId])
	{
		_afPlayerSoundVols[uPlayerId] = fRad;
	}
}


void AIEnviro_KillAllSounds(void)
{
	CNiIterator<CAISound*> it;
	if (_pActiveSounds)
	{
		it = _pActiveSounds->Begin();
		while (it.IsValid())
		{
			it.Get()->m_uHandle = 0;
			_pSoundBank->Recycle(it.Get());
			it.Next();
		}
		_pActiveSounds->RemoveAll();
	}

	if (_pVisibleEntities)
	{
		it = _pVisibleEntities->Begin();
		while (it.IsValid())
		{
			it.Get()->m_uHandle = 0;
			_pSoundBank->Recycle(it.Get());
			it.Next();
		}
		_pVisibleEntities->RemoveAll();
	}


	for (u32 i = 0; i < MAX_PLAYERS; i++)
	{
		_afPlayerSoundVols[i] = 0.0f;
		_aPlayerSoundHandles[i] = 0;
	}
}


void AIEnviro_DebugRender(void)
{
	CNiIterator<CAISound*> it = _pActiveSounds->Begin();
	while (it.IsValid())
	{
		CAISound* pSound = it.Get();
		fdraw_FacetedWireSphere( &(pSound->m_Origin.v3), pSound->m_fAudibleRadius, 2, 2);
		it.Next();
	}

	it = _pVisibleEntities->Begin();
	while (it.IsValid())
	{
		CAISound* pSound = it.Get();
		fdraw_FacetedWireSphere( &(pSound->m_Origin.v3), pSound->m_fAudibleRadius, 2, 2, &FColor_MotifBlue);
		it.Next();
	}
}


CAISound* AIEnviro_SoundHandleToPtr(AISoundHandle uSoundHandle)
{
	CNiIterator<CAISound*> it = _pActiveSounds->Begin();
	while (it.IsValid())
	{
		if ((u32) it.Get()->m_uHandle == uSoundHandle)
		{
			return it.Get();
		}
		it.Next();
	}

	it = _pVisibleEntities->Begin();
	while (it.IsValid())
	{
		if ((u32) it.Get()->m_uHandle == uSoundHandle)
		{
			return it.Get();
		}
		it.Next();
	}
	return NULL;
}


AISoundHandle AIEnviro_AddSound(const CFVec3A& Origin, f32 fAudibleRadius, f32 fHowLongSecs, AISoundType uSoundType, AISoundCtrlFlag uSoundCtrlFlag, CEntity* pEntity )
{
	f32 fNowTime = aiutils_GetCurTimeSecs();
	CAISound* pSound = _pSoundBank->Get();
	if (pSound)
	{
		BOOL bPushed = FALSE;
		if (uSoundCtrlFlag & AISOUNDCTRL_VISIBLE_ENTITY)
		{
			bPushed = _pVisibleEntities->PushHead(pSound);
		}
		else
		{
			bPushed = _pActiveSounds->PushHead(pSound);
		}

		if (bPushed)
		{
			pSound->m_uHandle = _uHandleGUID;
			_uHandleGUID++;
			pSound->m_fWhen = fNowTime;
			pSound->m_Origin = Origin;
			pSound->m_fAudibleRadius = fAudibleRadius;
			pSound->m_fHowLong = fHowLongSecs;
			pSound->m_uSoundType = uSoundType;
			pSound->m_pEntity = pEntity;
			pSound->m_uSoundCtrlFlags = AISOUNDCTRL_NEW_SOUND | uSoundCtrlFlag;
			pSound->m_uSoundCtrlFlags &= ~AISOUNDCTRL_NEW_CLEAR;
			return pSound->m_uHandle;
		}
		else
		{
			_pSoundBank->Recycle(pSound);
			pSound = NULL;
		}
	}
	return 0;
}


//Modify a Sound that is currently playing
BOOL AIEnviro_ModifySound(	AISoundHandle uSoundHandle,
							const CFVec3A& Origin,
							f32 fAudibleRadius,
							f32 fHowLongSecs,
							AISoundType uSoundType,
							CEntity* pEntity)
{
	f32 fNowTime = aiutils_GetCurTimeSecs();
	CAISound* pSound = AIEnviro_SoundHandleToPtr(uSoundHandle);
	if (pSound)
	{
		pSound->m_Origin = Origin;
		pSound->m_fAudibleRadius = fAudibleRadius;
		pSound->m_fHowLong = fHowLongSecs;
		pSound->m_uSoundType = uSoundType;
		pSound->m_fWhen = fNowTime;
		pSound->m_pEntity = pEntity;
		pSound->m_uSoundCtrlFlags |= AISOUNDCTRL_MODIFIED_SOUND;
		pSound->m_uSoundCtrlFlags &= ~AISOUNDCTRL_MODIFIED_CLEAR;
		return TRUE;
	}
	return FALSE;
}


void AIEnviro_DetectSoundCollisions(const CFVec3A &EarLoc, CEntity* pListener, f32 fHearingMagnification, SoundCollisionCallback pFunc, void* pData)
{
	CFSphereA SoundBounds;
	CNiIterator<CAISound*> it = _pActiveSounds->Begin();
	while (it.IsValid() )
	{
		if (pListener != it.Get()->m_pEntity)
		{
			SoundBounds.Set(it.Get()->m_Origin, it.Get()->m_fAudibleRadius*fHearingMagnification);
			if (SoundBounds.IsIntersecting(EarLoc))
			{
				(*pFunc)(it.Get(), pData);
			}
		}
		it.Next();
	}
}


void AIEnviro_IterateAllAIVisibleEntities(SoundCollisionCallback pFunc, void* pData)
{
	CNiIterator<CAISound*> it = _pVisibleEntities->Begin();
	while (it.IsValid() )
	{
		(*pFunc)(it.Get(), pData);
		it.Next();
	}
}