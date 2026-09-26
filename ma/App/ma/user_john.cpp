//////////////////////////////////////////////////////////////////////////////////////
// user_john.cpp - 
//
// Author: Michael Starich   
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
// 01/23/02 Starich     Created.
//////////////////////////////////////////////////////////////////////////////////////
#include "fang.h"
#include "user_john.h"
#include "floop.h"
#include "fresload.H"
#include "bot.h"
#include "player.h"

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

BOOL _bInit = FALSE;
s32 _nAnimating = -120;
CFAnimInst _PoopyHead;

BOOL user_john_PreworldInit( void ) 
{
	if ( !_bInit )
	{
		FAnim_t *pAnim = (FAnim_t *)fresload_Load( FANIM_RESNAME, "ardmwave_01" );
		_PoopyHead.Create( pAnim );
		_bInit = TRUE;
	}
	return TRUE;
}

BOOL user_john_PrelevelInit( void ) 
{
	return TRUE;
}

BOOL user_john_Main( void ) 
{
	if ( !CPlayer::m_pCurrent )
	{
		return TRUE;
	}
	
	CBot *pPlayerIHope = (CBot *)CPlayer::m_pCurrent->m_pEntityCurrent;
	if ( !pPlayerIHope )
	{
		return TRUE;
	}
	
	if ( _nAnimating > 0 )
	{
		_PoopyHead.DeltaTime( FLoop_fPreviousLoopSecs, TRUE );
		f32 fTime = _PoopyHead.GetUnitTime();
		if ( fTime <= 0.1f )
		{
			pPlayerIHope->UserAnim_SetControlValue( 0, fTime * 10.f );
		}
		else if ( fTime < 0.9f )
		{
			pPlayerIHope->UserAnim_SetControlValue( 0, 1.f );
		}
		else if ( fTime >= 0.9f && fTime < 0.99f )
		{
			pPlayerIHope->UserAnim_SetControlValue( 0, (1.f - fTime) * 10.f );
		}
		else if ( fTime > 0.99f )
		{
			_nAnimating = -600;
			pPlayerIHope->UserAnim_DisableAllBones( 0 );
			pPlayerIHope->UserAnim_SetControlValue( 0, 0.0f );
			pPlayerIHope->UserAnim_Attach( 0, NULL );
		}
	}
	else
	{
		CFVec3A vBotPos, vPos;
		static CFVec3A _vMirrorPos( -137.f, 5.29f, 120.f );
		vPos.Set( pPlayerIHope->MtxToWorld()->m_vPos );
		vPos.Sub( _vMirrorPos );
		
		if ( vPos.Mag() < 15.f && pPlayerIHope->MtxToWorld()->m_vFront.z > 0.966f )
		{
			if ( _nAnimating == 0 )
			{
				pPlayerIHope->UserAnim_Attach( 0, &_PoopyHead );
				pPlayerIHope->UserAnim_SetControlValue( 0, 0.f );
				pPlayerIHope->UserAnim_DisableAllBones( 0 );
				pPlayerIHope->UserAnim_UpdateBoneMask( 0, "Head", TRUE );
				pPlayerIHope->UserAnim_UpdateBoneMask( 0, "L_Shoulder", TRUE );
				pPlayerIHope->UserAnim_UpdateBoneMask( 0, "L_Arm_Upper", TRUE );
				pPlayerIHope->UserAnim_UpdateBoneMask( 0, "L_Arm_Elbow", TRUE );
				pPlayerIHope->UserAnim_UpdateBoneMask( 0, "L_Arm_Lower", TRUE );
				pPlayerIHope->UserAnim_UpdateBoneMask( 0, "L_ThumbB", TRUE );
				pPlayerIHope->UserAnim_UpdateBoneMask( 0, "L_ThumbA", TRUE );
				pPlayerIHope->UserAnim_UpdateBoneMask( 0, "L_Hand", TRUE );
				pPlayerIHope->UserAnim_UpdateBoneMask( 0, "L_IndexFingerA", TRUE );
				pPlayerIHope->UserAnim_UpdateBoneMask( 0, "L_IndexFingerB", TRUE );
				pPlayerIHope->UserAnim_UpdateBoneMask( 0, "L_MiddleFingerA", TRUE );
				pPlayerIHope->UserAnim_UpdateBoneMask( 0, "L_MiddleFingerB", TRUE );
				pPlayerIHope->UserAnim_UpdateBoneMask( 0, "L_PinkyFingerA", TRUE );
				pPlayerIHope->UserAnim_UpdateBoneMask( 0, "L_PinkyFingerB", TRUE );
				pPlayerIHope->UserAnim_UpdateBoneMask( 0, "Secondary_Fire", TRUE );
				pPlayerIHope->UserAnim_UpdateBoneMask( 0, "Torso", TRUE );
				pPlayerIHope->UserAnim_UpdateBoneMask( 0, "Backpack", TRUE );
				pPlayerIHope->UserAnim_UpdateBoneMask( 0, "Beltpack1", TRUE );
				pPlayerIHope->UserAnim_UpdateBoneMask( 0, "Beltpack2", TRUE );
				pPlayerIHope->UserAnim_UpdateBoneMask( 0, "Beltpack3", TRUE );
				pPlayerIHope->UserAnim_UpdateBoneMask( 0, "Beltpack4", TRUE );
				pPlayerIHope->UserAnim_UpdateBoneMask( 0, "Beltpack5", TRUE );
				pPlayerIHope->UserAnim_UpdateBoneMask( 0, "BackpackDoor", TRUE );
				pPlayerIHope->UserAnim_UpdateBoneMask( 0, "AntennaA", TRUE );
				pPlayerIHope->UserAnim_UpdateBoneMask( 0, "AntennaB", TRUE );
				pPlayerIHope->UserAnim_UpdateBoneMask( 0, "AntennaC", TRUE );
				pPlayerIHope->UserAnim_UpdateBoneMask( 0, "AntennaD", TRUE );
				pPlayerIHope->UserAnim_UpdateBoneMask( 0, "AntennaE", TRUE );
				_PoopyHead.UpdateTime( 0.0f );
				_nAnimating = 1;
			}
			else
			{
				_nAnimating++;
			}
		}
		else
		{
			_nAnimating = -120;
		}
	}

	return TRUE;
}

void user_john_Terminate( void ) 
{
}

//==================
// private functions



