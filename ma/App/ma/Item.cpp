//////////////////////////////////////////////////////////////////////////////////////
// Item.cpp - Item Data Class for Mettle Arms.
//
// Author: Justin Link
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
// 02/11/02 Link		Created.
//////////////////////////////////////////////////////////////////////////////////////

// =============================================================================================================

#include "Item.h"
#include "fclib.h"
#include "fresload.h"
#include "ftex.h"
#include "fmesh.h"
#include "gstring.h"
#include "weapons.h"
#include "hud2.h"


// =============================================================================================================

CItem::CItem()
{
	m_pszTag = NULL;
	m_pszCodeName = NULL;
	m_pwszDisplayName = NULL;
	m_pwszLongDesc = NULL;
	m_pwszUpgradeDesc = NULL;
	m_pMI = NULL;
	m_pMesh = NULL;
	m_pUpgraded = NULL;
	m_pDowngraded = NULL;
}

CItem::~CItem()
{
	fdelete(m_pMI); m_pMI = NULL;
}

// =============================================================================================================

BOOL CItem::Init(ItemInit_s *pItemInit)
{
	FASSERT(pItemInit->pszTag != NULL);
	m_pszTag = gstring_Main.AddString(pItemInit->pszTag);

//	m_ItemLight.InitDirLight(1.0f, -1.0f, 1.0f);
//	m_ItemLight.SetColor(1.0f, 1.0f, 1.0f);
	
//	FASSERT(fclib_strlen(pItemInit->pszName) <= uMaxItemNameLength);
//	fclib_strcpy(m_szName, pItemInit->pszName);
	m_pszCodeName = gstring_Main.AddString(pItemInit->pszCodeName);
	m_pwszDisplayName = gstring_Main.AddString(pItemInit->pwszDisplayName);
	
//	fclib_strcpy(m_szLongDesc, pItemInit->pszLongDesc);
	m_pwszLongDesc = gstring_Main.AddString(pItemInit->pwszLongDesc);

//	fclib_strcpy(m_szUpgradeDesc, pItemInit->pszUpgradeDesc);
	m_pwszUpgradeDesc = gstring_Main.AddString(pItemInit->pwszUpgradeDesc);

	if( pItemInit->pszIconTGAFile != NULL ) {
		m_texIcon.SetTexDef( (FTexDef_t *)(fresload_Load(FTEX_RESNAME, pItemInit->pszIconTGAFile)) );
		if(m_texIcon.GetTexDef() == NULL)
			DEVPRINTF("CItem::Init() : Could not load icon texture '%s' for weapon '%s'.\n", pItemInit->pszIconTGAFile, pItemInit->pszCodeName);
	} else {
		m_texIcon.SetTexDef( NULL );
	}

	if( pItemInit->pszMeshFile != NULL ) {
		m_pMesh = (FMesh_t *)(fresload_Load(FMESH_RESTYPE, pItemInit->pszMeshFile));
		if(m_pMesh == NULL)
			DEVPRINTF("CItem::Init() : Could not load weapon mesh '%s' for weapon '%s'.\n", pItemInit->pszMeshFile, pItemInit->pszCodeName);
	} else {
		m_pMesh = NULL;
	}

	/////////////////////////////////////////////////////////////
	// Create and init our CFMeshInst.
	m_pMI = fnew CFMeshInst;
	if(m_pMI == NULL)
	{
		DEVPRINTF("CItem::Init() : Could create CFMeshInst for weapon '%s'.\n", pItemInit->pszCodeName);
		return(FALSE);
	}
	FMeshInit_t oMeshInit;
	oMeshInit.fCullDist = FMATH_MAX_FLOAT;
	oMeshInit.Mtx.Identity();
	oMeshInit.nFlags = FMESHINST_FLAG_ANIM_WHEN_PAUSED;
	oMeshInit.pMesh = m_pMesh;

	if(pItemInit->bPoster)
	{
		oMeshInit.nFlags |= ( FMESHINST_FLAG_POSTER_X | FMESHINST_FLAG_POSTER_Y | FMESHINST_FLAG_POSTER_BONES ); 
	}

	m_pMI->Init(&oMeshInit);
	//
	/////////////////////////////////////////////////////////////

	m_vecST1 = pItemInit->vecST1;
	m_vecST2 = pItemInit->vecST2;

	m_uInventoryPos_x = pItemInit->uInventoryPos_x;
	m_uInventoryPos_y = pItemInit->uInventoryPos_y;

	m_uFlags = pItemInit->uFlags;

	m_uCurLevel = pItemInit->uCurLevel;
	m_uMaxLevel = pItemInit->uMaxLevel;

	m_uItemType = pItemInit->uItemType;

	m_fScale = pItemInit->fScale;
	m_fYOffset = pItemInit->fYOffset;
	m_fZOffset = pItemInit->fZOffset;
	m_fPitch = FMATH_DEG2RAD(pItemInit->fPitch_DEG);

	return(TRUE);
}

// =============================================================================================================

void CItem::DrawMesh(f32 fThetaY)
{
	if((m_pMI == NULL) || (m_pMesh == NULL))
	{
		return;
	}

	f32 fScale = m_fScale * 5.0f;
	f32 fYOffset = m_fYOffset;
	f32 fZOffset = m_fZOffset;
	f32 fPitch = m_fPitch;
	f32 fOffsetX;

	if( m_uFlags & ITEMFLAG_PRIMARY ) {
		fOffsetX = -1.75f;
	} else {
		fOffsetX = 0.5f;
	}

	CFVec3A vecOfsToBS;
	vecOfsToBS.Set(m_pMesh->BoundSphere_MS.m_Pos);
	vecOfsToBS.Mul(fScale);
//	vecOfsToBS.z += fZOffset;
	CFMtx43A mtx1, mtx2, mtxMiddle, mtxFinal;

	// Adjust the object so that it's origin is the center of it's BS.
	mtx1.Identity();
	mtx1.m_vPos = vecOfsToBS;
	mtx1.m_vPos.Mul(-1.0f);

	// Move the object back the way it was.
	mtx2.Identity();
	mtx2.m_vPos = vecOfsToBS;

	mtxMiddle.Identity();
	
	if( !( m_pMI->m_nFlags & ( FMESHINST_FLAG_POSTER_X | FMESHINST_FLAG_POSTER_Y | FMESHINST_FLAG_POSTER_Z ) ) ) {
		mtxMiddle.RotateY(fThetaY);
	}

	mtxMiddle.RotateX(fPitch);
	mtxMiddle.m_vPos.Set(fOffsetX, fYOffset, 10.0f);
//	mtxMiddle.m_vX.Mul(fScale);
//	mtxMiddle.m_vY.Mul(fScale);
//	mtxMiddle.m_vZ.Mul(fScale);

	mtxFinal = mtx2;
	mtxFinal.Mul(mtxMiddle);
	mtxFinal.Mul(mtx1);
	m_pMI->m_Xfm.BuildFromMtx(mtxFinal, fScale);

	CFColorRGB oColorRGB;
	f32 fIntensity;

	m_pMI->ResetLightList();

	fmesh_Ambient_Get(&oColorRGB, &fIntensity);
	fmesh_Ambient_Set(1.0f, 1.0f, 1.0f, 0.2f);

	m_pMI->ConsiderLightForRender( &CHud2::m_HUDDirLight );

	m_pMI->Draw(FVIEWPORT_PLANESMASK_NONE, FALSE);

	fmesh_Ambient_Set(&oColorRGB, fIntensity);
}

// =============================================================================================================

// JUSTIN: Move these to CSV !!!
#define _WEAPON_NAME_RIVET_GUN			"Rivet Gun"
#define _WEAPON_NAME_MINING_LASER		"Mining Laser"
#define _WEAPON_NAME_ROCKET_LAUNCHER	"Rocket Launcher"
#define _WEAPON_NAME_CONTROL_TETHER		"Control Tether"
#define _WEAPON_NAME_SCATTER_BLASTER	"Scatter Blaster"
#define _WEAPON_NAME_EMPTY_PRIMARY		"Empty Primary"
#define _WEAPON_NAME_EMPTY_SECONDARY	"Empty Secondary"
#define _WEAPON_NAME_CORING_CHARGE		"Coring Charge"
#define _WEAPON_NAME_SPEW				"SPEW"
#define _WEAPON_NAME_MORTAR				"Mortar"
#define _WEAPON_NAME_FLAMER				"Flamer"
#define _WEAPON_NAME_RIPPER				"Ripper"
#define _WEAPON_NAME_CLEANER			"Cleaner"
#define _WEAPON_NAME_SCOPE				"Scope"
#define _WEAPON_NAME_CHAINGUN			"Chaingun"
#define _WEAPON_NAME_EMP_GRENADE		"EMP Grenade"
#define _WEAPON_NAME_RECRUITER_GRENADE	"Recruiter Grenade"
#define _WEAPON_NAME_QUADLASER			"Quad Laser"
#define _WEAPON_NAME_MAGMA_BOMB			"Magma Bomb"
#define _WEAPON_NAME_STAFF				"Staff"
#define _WEAPON_NAME_WRENCH				"Wrench"

// allocate and create() a weapon object corresponding to
// the name string stored in the item
CWeapon *CItem::MakeWeapon()
{
	CWeapon *pWeapon = NULL;

	if (m_pszCodeName == NULL)
	{
		//DEVPRINTF("CItem::MakeWeapon() : Do not know how to create weapon '%s'.\n", m_pszName);
		FASSERT( m_pszCodeName == NULL );
	}
	else if(fclib_stricmp(m_pszCodeName, _WEAPON_NAME_RIVET_GUN) == 0)
	{
		pWeapon = fnew CWeaponRivet;
		if( pWeapon == NULL )
		{
			return NULL;
		}

		if( !((CWeaponRivet *)(pWeapon))->Create() )
		{
			return NULL;
		}

		pWeapon->EnableAutoWork(FALSE);
	}
	else if(fclib_stricmp(m_pszCodeName, _WEAPON_NAME_MINING_LASER) == 0)
	{
		pWeapon = fnew CWeaponLaser;
		if( pWeapon == NULL )
		{
			return NULL;
		}

		if( !((CWeaponLaser *)(pWeapon))->Create() )
		{
			return NULL;
		}

		pWeapon->EnableAutoWork(FALSE);
	}
	else if(fclib_stricmp(m_pszCodeName, _WEAPON_NAME_ROCKET_LAUNCHER) == 0)
	{
		pWeapon = fnew CWeaponRocket;
		if( pWeapon == NULL )
		{
			return NULL;
		}

		if( !((CWeaponRocket *)(pWeapon))->Create() )
		{
			return NULL;
		}

		pWeapon->EnableAutoWork(FALSE);
	}
	else if(fclib_stricmp(m_pszCodeName, _WEAPON_NAME_CONTROL_TETHER) == 0)
	{
		pWeapon = fnew CWeaponTether;
		if( pWeapon == NULL )
		{
			return NULL;
		}

		if( !((CWeaponTether *)(pWeapon))->Create() )
		{
			return NULL;
		}

		pWeapon->EnableAutoWork(FALSE);
	}
	else if(fclib_stricmp(m_pszCodeName, _WEAPON_NAME_SPEW) == 0)
	{
		pWeapon = fnew CWeaponSpew;
		if( pWeapon == NULL )
		{
			return NULL;
		}

		if( !((CWeaponSpew *)(pWeapon))->Create() )
		{
			return NULL;
		}

		pWeapon->EnableAutoWork(FALSE);
	}
	else if(fclib_stricmp(m_pszCodeName, _WEAPON_NAME_MORTAR) == 0)
	{
		pWeapon = fnew CWeaponMortar;
		if( pWeapon == NULL )
		{
			return NULL;
		}

		if( !((CWeaponMortar *)(pWeapon))->Create() )
		{
			return NULL;
		}

		pWeapon->EnableAutoWork(FALSE);
	}
	else if(fclib_stricmp(m_pszCodeName, _WEAPON_NAME_FLAMER) == 0)
	{
		pWeapon = fnew CWeaponFlamer;
		if( pWeapon == NULL )
		{
			return NULL;
		}

		if( !((CWeaponFlamer *)(pWeapon))->Create() )
		{
			return NULL;
		}

		pWeapon->EnableAutoWork(FALSE);
	}
	else if(fclib_stricmp(m_pszCodeName, _WEAPON_NAME_RIPPER) == 0)
	{
		pWeapon = fnew CWeaponRipper;
		if( pWeapon == NULL )
		{
			return NULL;
		}

		if( !((CWeaponRipper *)(pWeapon))->Create() )
		{
			return NULL;
		}

		pWeapon->EnableAutoWork(FALSE);
	}
	else if(fclib_stricmp(m_pszCodeName, _WEAPON_NAME_SCATTER_BLASTER) == 0)
	{
		pWeapon = fnew CWeaponBlaster;
		if( pWeapon == NULL )
		{
			return NULL;
		}

		if( !((CWeaponBlaster *)(pWeapon))->Create() )
		{
			return NULL;
		}

		pWeapon->EnableAutoWork(FALSE);
	}
	else if(fclib_stricmp(m_pszCodeName, _WEAPON_NAME_EMPTY_PRIMARY) == 0)
	{
		pWeapon = fnew CWeaponHand;
		if( pWeapon == NULL )
		{
			return NULL;
		}

		if( !((CWeaponHand *)(pWeapon))->Create() )
		{
			return NULL;
		}

		pWeapon->EnableAutoWork(FALSE);
	}
	else if(fclib_stricmp(m_pszCodeName, _WEAPON_NAME_EMPTY_SECONDARY) == 0)
	{
		return(NULL);
	}
	else if(fclib_stricmp(m_pszCodeName, _WEAPON_NAME_CORING_CHARGE) == 0)
	{
		pWeapon = fnew CWeaponGren;
		if( pWeapon == NULL )
		{
			return NULL;
		}

		if( !((CWeaponGren *)(pWeapon))->Create() )
		{
			return NULL;
		}

		pWeapon->EnableAutoWork(FALSE);
	}
	else if(fclib_stricmp(m_pszCodeName, _WEAPON_NAME_CLEANER) == 0)
	{
		pWeapon = fnew CWeaponCleaner;
		if( pWeapon == NULL )
		{
			return NULL;
		}

		if( !((CWeaponCleaner *)(pWeapon))->Create() )
		{
			return NULL;
		}

		pWeapon->EnableAutoWork(FALSE);
	}
	else if(fclib_stricmp(m_pszCodeName, _WEAPON_NAME_SCOPE) == 0)
	{
		pWeapon = fnew CWeaponScope;
		if( pWeapon == NULL )
		{
			return NULL;
		}

		if( !((CWeaponScope *)(pWeapon))->Create() )
		{
			return NULL;
		}

		pWeapon->EnableAutoWork(FALSE);
	}
	else if(fclib_stricmp(m_pszCodeName, _WEAPON_NAME_EMP_GRENADE) == 0)
	{
		pWeapon = fnew CWeaponEMP;
		if( pWeapon == NULL )
		{
			return NULL;
		}

		if( !((CWeaponEMP *)(pWeapon))->Create() )
		{
			return NULL;
		}

		pWeapon->EnableAutoWork(FALSE);
	}
	else if(fclib_stricmp(m_pszCodeName, _WEAPON_NAME_RECRUITER_GRENADE) == 0)
	{
		pWeapon = fnew CWeaponRecruiter;
		if( pWeapon == NULL )
		{
			return NULL;
		}

		if( !((CWeaponRecruiter *)(pWeapon))->Create() )
		{
			return NULL;
		}

		pWeapon->EnableAutoWork(FALSE);
	}
	else if(fclib_stricmp(m_pszCodeName, _WEAPON_NAME_MAGMA_BOMB) == 0)
	{
		pWeapon = fnew CWeaponMagmaBomb;
		if( pWeapon == NULL )
		{
			return NULL;
		}

		if( !((CWeaponMagmaBomb *)(pWeapon))->Create() )
		{
			return NULL;
		}

		pWeapon->EnableAutoWork(FALSE);
	}
	else if(fclib_stricmp(m_pszCodeName, _WEAPON_NAME_WRENCH) == 0)
	{
		pWeapon = fnew CWeaponWrench;
		if( pWeapon == NULL )
		{
			return NULL;
		}

		if( !((CWeaponWrench *)(pWeapon))->Create() )
		{
			return NULL;
		}

		pWeapon->EnableAutoWork(FALSE);
	}
	else
	{
		DEVPRINTF("CItem::MakeWeapon() : Do not know how to create weapon '%s'.\n", m_pszCodeName);
	}

	return(pWeapon);
}

// =============================================================================================================

cchar *CItem::GetWeaponName( const CWeapon *pWeapon )
{
	FASSERT( pWeapon );

	switch( pWeapon->Type() )
	{
	case CWeapon::WEAPON_TYPE_BLINK_RIGHT_HAND:
		return _WEAPON_NAME_EMPTY_PRIMARY;

	case CWeapon::WEAPON_TYPE_LASER:
		return _WEAPON_NAME_MINING_LASER;

	case CWeapon::WEAPON_TYPE_ROCKET_LAUNCHER:
		return _WEAPON_NAME_ROCKET_LAUNCHER;

	case CWeapon::WEAPON_TYPE_TETHER:
		return _WEAPON_NAME_CONTROL_TETHER;

	case CWeapon::WEAPON_TYPE_SPEW:
		return _WEAPON_NAME_SPEW;

	case CWeapon::WEAPON_TYPE_MORTAR:
		return _WEAPON_NAME_MORTAR;

	case CWeapon::WEAPON_TYPE_FLAMER:
		return _WEAPON_NAME_FLAMER;

	case CWeapon::WEAPON_TYPE_RIPPER:
		return _WEAPON_NAME_RIPPER;

	case CWeapon::WEAPON_TYPE_BLASTER:
		return _WEAPON_NAME_SCATTER_BLASTER;

	case CWeapon::WEAPON_TYPE_RIVET_GUN:
		return _WEAPON_NAME_RIVET_GUN;

	case CWeapon::WEAPON_TYPE_GRENADE:
		return _WEAPON_NAME_CORING_CHARGE;

	case CWeapon::WEAPON_TYPE_CLEANER:
		return _WEAPON_NAME_CLEANER;

	case CWeapon::WEAPON_TYPE_SCOPE:
		return _WEAPON_NAME_SCOPE;

	case CWeapon::WEAPON_TYPE_CHAINGUN:
		return _WEAPON_NAME_CHAINGUN;

	case CWeapon::WEAPON_TYPE_EMP:
		return _WEAPON_NAME_EMP_GRENADE;

	case CWeapon::WEAPON_TYPE_RECRUITER:
		return _WEAPON_NAME_RECRUITER_GRENADE;

	case CWeapon::WEAPON_TYPE_QUADLASER:
		return _WEAPON_NAME_QUADLASER;

	case CWeapon::WEAPON_TYPE_MAGMABOMB:
		return _WEAPON_NAME_MAGMA_BOMB;

	case CWeapon::WEAPON_TYPE_STAFF:
		return _WEAPON_NAME_STAFF;

	case CWeapon::WEAPON_TYPE_WRENCH:
		return _WEAPON_NAME_WRENCH;

	default:
		FASSERT_NOW;
	}

	// Should never get here!
	FASSERT_NOW;
	return NULL;
}

// =============================================================================================================

BOOL CItem::IsDual()
{
	return((m_uFlags & ITEMFLAG_DUAL) == ITEMFLAG_DUAL);
}

// =============================================================================================================

BOOL CItem::IsPrimary()
{
	return((m_uFlags & ITEMFLAG_PRIMARY) == ITEMFLAG_PRIMARY);
}

// =============================================================================================================

BOOL CItem::IsSecondary()
{
	return((m_uFlags & ITEMFLAG_SECONDARY) == ITEMFLAG_SECONDARY);
}

// =============================================================================================================

BOOL CItem::IsPrimaryOnly()
{
	if(IsDual())
		return(FALSE);
	return(IsPrimary());
}

// =============================================================================================================

BOOL CItem::IsSecondaryOnly()
{
	if(IsDual())
		return(FALSE);
	return(IsSecondary());
}

// =============================================================================================================
