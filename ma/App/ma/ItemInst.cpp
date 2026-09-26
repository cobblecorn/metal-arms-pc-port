//////////////////////////////////////////////////////////////////////////////////////
// ItemInst.cpp - Item Instance Class for Mettle Arms.
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
#include "ItemInst.h"
#include "Item.h"
#include "ItemRepository.h"
#include "Weapons.h"
#include "fclib.h"
#include "gamesave.h"
#include "MultiplayerMgr.h"
#include "Hud2.h"	// For Hud2_XFormPrintf

///////////////////
// public variables
cchar *ItemInst_apszItemNames[INVPOS_COUNT] = {
	"Washer",			// INVPOS_WASHER
	"Chip",				// INVPOS_CHIP
	"Secret Chip",		// INVPOS_SECRETCHIP
	"Arm Servo",		// INVOPOS_ARM_SERVO
//	"Mil Translator",	// INVPOS_MILTRANS
//	"Antenna",			// INVPOS_ANTENNA
	"Det Pack",			// INVPOS_DETPACK
//	"EUK",				// INVPOS_EUK
//	"Mission Briefing", // INVPOS_MISBRIEFING
};


///////////////////////
// Multiplayer item table
struct _ItemDesc_t {
	cchar*	m_szName;			// Item name
	s32		m_nClipAmmo;		// Initial Clip ammo
	s32		m_nReserveAmmo;		// Initial Reserve ammo
};

static const _ItemDesc_t _sPrimaryItemTable[] = {
	{ "Laser L1",			  100,		    0 },				//GAMESAVE_PRIMARY_WEAPON_LIMIT_NO_LIMIT
	{ "RLauncher L1",		 1000,		   10 },				//GAMESAVE_PRIMARY_WEAPON_LIMIT_ROCKETS_ONLY
	{ "Laser L1",			  100,		    0 },				//GAMESAVE_PRIMARY_WEAPON_LIMIT_LASER_ONLY
	{ "Rivet Gun L1",		10000,		10000 },				//GAMESAVE_PRIMARY_WEAPON_LIMIT_RIVET_ONLY
	{ "Flamer L1",			10000,		10000 },				//GAMESAVE_PRIMARY_WEAPON_LIMIT_TOASTER_ONLY
	{ "Ripper L1",			10000,		10000 },				//GAMESAVE_PRIMARY_WEAPON_LIMIT_RIPPER_ONLY
	{ "SPEW L1",			10000,		10000 },				//GAMESAVE_PRIMARY_WEAPON_LIMIT_SPEW_ONLY
	{ "Blaster L1",			10000,		10000 },				//GAMESAVE_PRIMARY_WEAPON_LIMIT_SCATTER_BLASTER_ONLY
	{ "Mortar L1",				0, CItemInst_nNoMaxAmmo },		//GAMESAVE_PRIMARY_WEAPON_LIMIT_SLINGSHOT_ONLY
	{ "Empty Primary",			1, CItemInst_nNoAmmo }			//GAMESAVE_PRIMARY_WEAPON_LIMIT_NO_WEAPONS
};

static const _ItemDesc_t _sSecondaryItemTable[] = {
	{ "Coring Charge",			3,			CItemInst_nNoMaxAmmo },		//GAMESAVE_SECONDARY_WEAPON_LIMIT_NO_LIMIT
	{ "Coring Charge",		    3,			CItemInst_nNoMaxAmmo },		//GAMESAVE_SECONDARY_WEAPON_LIMIT_CORING_CHARGE_ONLY
	{ "Magma Bomb",			    6,			CItemInst_nNoMaxAmmo },		//GAMESAVE_SECONDARY_WEAPON_LIMIT_MAGMA_BOMB_ONLY
	{ "EMP Grenade",		    3,			CItemInst_nNoMaxAmmo },		//GAMESAVE_SECONDARY_WEAPON_LIMIT_EMP_ONLY
	{ "Scope L2",		CItemInst_nNoAmmo,	CItemInst_nNoMaxAmmo },		//GAMESAVE_SECONDARY_WEAPON_LIMIT_SCOPE_ONLY
	{ "Cleaner",			   20,			CItemInst_nNoMaxAmmo },		//GAMESAVE_SECONDARY_WEAPON_LIMIT_CLEANER_ONLY
	{ "Recruiter Grenade",		3,			CItemInst_nNoMaxAmmo },		//GAMESAVE_SECONDARY_WEAPON_LIMIT_RECRUITER_ONLY
	{ "Empty Secondary",		1,			CItemInst_nNoAmmo }			//GAMESAVE_SECONDARY_WEAPON_LIMIT_NO_WEAPONS
};

// CItemInst functions
///////////////////////

CItemInst::CItemInst() {
	m_pItemData = NULL;
	m_pOwnerInventory = NULL;
	m_pWeapon = NULL;
	m_aeDisplayType[0] = AMMODISPLAY_NONE;
	m_aeDisplayType[1] = AMMODISPLAY_NONE;
}

CItemInst::~CItemInst() {
	m_pItemData = NULL;
}

void CItemInst::PrintItemText( FTextAreaHandle_t hAreaCur,
							   FTextAreaHandle_t hAreaMax,
							   FDrawVtx_t *pvtxArray/*=NULL*/,
							   u32 *puStartVtx/*=NULL*/,
							   BOOL bScaleCurText/*=TRUE*/,
							   BOOL bScaleMaxText/*=TRUE*/ ) {
	FTextAreaHandle_t ahArea[2];
	ahArea[0] = hAreaCur;
	ahArea[1] = hAreaMax;

	s16 anAmmo[2];
	anAmmo[0] = m_nClipAmmo;
	anAmmo[1] = m_nReserveAmmo;

	u32 abScale[2];
	abScale[0] = !bScaleCurText;
	abScale[1] = !bScaleMaxText;

	u32 nDeciRed, nDeciGreen, nDeciBlue;

	CHud2::ComputeAmmoAlertTextColor( m_pWeapon, &nDeciRed, &nDeciGreen, &nDeciBlue );

	u32 uAmmoIdx;
	for( uAmmoIdx = 0; uAmmoIdx < 2; ++uAmmoIdx ) {
		switch(m_aeDisplayType[uAmmoIdx])
		{
			case AMMODISPLAY_NUMERIC:
			{
				if(ahArea[uAmmoIdx] != 0)
				{
//					Hud2_XFormPrintf(ahArea[uAmmoIdx], "%d", anAmmo[uAmmoIdx]);
					Hud2_XFormPrintf(ahArea[uAmmoIdx], "~C%02u%02u%02u99~o%d%d", nDeciRed, nDeciGreen, nDeciBlue, abScale[uAmmoIdx], anAmmo[uAmmoIdx]);
				}
				break;
			}
			case AMMODISPLAY_METER:
			{
				if((pvtxArray != NULL) && (puStartVtx != NULL) && (uAmmoIdx == 0))
				{
					static const f32 _fS1 = 0.0078125f;
					static const f32 _fT1 = 0.921875f;
					static const f32 _fS2 = 0.23632812f;
					static const f32 _fT2 = 0.9921875f;

					// DFS -- This is a kludge on a hack. These numbers should
					// not be hardwired, they depend on the location of the
					// surrounding border texture! 
					static const f32 _fX1 = 0.4555f; //0.318f;
					static const f32 _fY1 = -0.615625f; //-0.50f;
					static const f32 _fY2 = -0.720625f; //-0.605f;
					static const f32 _fWidth = 0.360f;

					f32 fUnitAmmo = (f32)(m_nClipAmmo) * (1.0f / 100.0f);
					f32 fX2 = _fX1 + (_fWidth * fUnitAmmo);
					f32 fS2 = _fS1 + (fUnitAmmo) * (_fS2 - _fS1);

					CFColorRGBA rgbaColor;
					rgbaColor.fRed = fUnitAmmo > 0.5f ? (2.0f - 2.0f * fUnitAmmo) : 1.0f;
					rgbaColor.fGreen = fUnitAmmo < 0.5f ? (2.0f * fUnitAmmo) : 1.0f;
					rgbaColor.fBlue = 0.0f;
					rgbaColor.fAlpha = 1.0f;

					// Upper left.
					pvtxArray[(*puStartVtx)].Pos_MS.Set(_fX1, _fY1, 0.0f);
					pvtxArray[(*puStartVtx)].ColorRGBA = rgbaColor;
					pvtxArray[(*puStartVtx)++].ST.Set(_fS1, _fT1);

					// Upper right.
					pvtxArray[(*puStartVtx)].Pos_MS.Set(fX2, _fY1, 0.0f);
					pvtxArray[(*puStartVtx)].ColorRGBA = rgbaColor;
					pvtxArray[(*puStartVtx)++].ST.Set(fS2, _fT1);

					// Lower right.
					pvtxArray[(*puStartVtx)].Pos_MS.Set(fX2, _fY2, 0.0f);
					pvtxArray[(*puStartVtx)].ColorRGBA = rgbaColor;
					pvtxArray[(*puStartVtx)++].ST.Set(fS2, _fT2);

					// Lower left.
					pvtxArray[(*puStartVtx)].Pos_MS.Set(_fX1, _fY2, 0.0f);
					pvtxArray[(*puStartVtx)].ColorRGBA = rgbaColor;
					pvtxArray[(*puStartVtx)++].ST.Set(_fS1, _fT2);
/*
					// Upper right.
					pvtxArray[(*puStartVtx)].Pos_MS.Set(fX2, _fY1, 0.0f);
					pvtxArray[(*puStartVtx)].ColorRGBA = rgbaColor;
					pvtxArray[(*puStartVtx)++].ST.Set(fS2, _fT1);

					// Lower left.
					pvtxArray[(*puStartVtx)].Pos_MS.Set(_fX1, _fY2, 0.0f);
					pvtxArray[(*puStartVtx)].ColorRGBA = rgbaColor;
					pvtxArray[(*puStartVtx)++].ST.Set(_fS1, _fT2);
*/
				}
				break;
			}
			case AMMODISPLAY_NONE:
			{
				break;
			}
			default:
			{
				FASSERT_NOW;
				break;
			}
		}
	}
}

void CItemInst::GetItemText(wchar *pwszClipAmmo, wchar *pwszReserveAmmo, u32 nStringLength) {
	if((pwszClipAmmo != NULL) && (m_aeDisplayType[0] == AMMODISPLAY_NUMERIC))
	{
		_snwprintf( pwszClipAmmo, nStringLength, L"%d", m_nClipAmmo);
	}

	if((pwszReserveAmmo != NULL) && (m_aeDisplayType[1] == AMMODISPLAY_NUMERIC))
	{
		_snwprintf( pwszReserveAmmo, nStringLength, L"%d", m_nReserveAmmo);
	}
}

// Create a weapon object corresponding to the ItemInst object parameters.
CWeapon *CItemInst::MakeWeapon()
{
	if(m_pItemData == NULL)
	{
		return(NULL);
	}

	// allocate and create() a weapon object corresponding to
	// the name string stored in the item
	CWeapon *pWeap = m_pItemData->MakeWeapon();

	if(pWeap != NULL)
	{
		// Set upgrade level of weapon.  It's not currently safe to upgrade
		// an empty weapon.
		if( fclib_stricmp(m_pItemData->m_pszCodeName, "Empty Primary") != 0 &&
			fclib_stricmp(m_pItemData->m_pszCodeName, "Empty Secondary") != 0 )
		{
			// weapon code uses zero-based index, Item code uses one-based index...
			if( m_pItemData->m_uCurLevel > 0 )
			{
				pWeap->SetUpgradeLevel( m_pItemData->m_uCurLevel - 1 );
			}
		}

		// tell weapon object how much ammo it has, based on quantities
		// stored in this ItemInst.  Weapon may alter quantites, which
		// are reflected back to the ItemInst.
		m_nClipAmmo = (s16)pWeap->SetClipAmmo( (u16)m_nClipAmmo );
		m_nReserveAmmo = (s16)pWeap->SetReserveAmmo( (u16)m_nReserveAmmo );
	}

	return(pWeap);
}

void CItemInst::NotifyAmmoMightHaveChanged( u16 nNewClipAmmo, u16 nNewReserveAmmo ) {
	m_nClipAmmo = nNewClipAmmo;
	m_nReserveAmmo = nNewReserveAmmo;
}

// FYI: this function accepts a zero-based upgrade value,
// but the item class uses a one-based value.
void CItemInst::NotifyWeaponUpgraded( u32 nNewUpgradeLevel )
{
	// 
	FASSERT(m_pItemData != NULL);
	u32 uCurrentLevel = m_pItemData->m_uCurLevel;

	// DFS -- Added this to prevent a crash on selecting upgrade for an empty
	// hand or perhaps another non-upgradable weapon
	if (uCurrentLevel == 0)
		return;

	CItem *pNewI;

	// It's an upgrade.
	while(nNewUpgradeLevel > (uCurrentLevel - 1))
	{
		pNewI = m_pItemData->m_pUpgraded;
		FASSERT(pNewI != NULL);
		m_pItemData = pNewI;
		uCurrentLevel = m_pItemData->m_uCurLevel;
	}

	// It's a downgrade.
	while(nNewUpgradeLevel < (uCurrentLevel - 1))
	{
		pNewI = m_pItemData->m_pDowngraded;
		FASSERT(pNewI != NULL);
		m_pItemData = pNewI;
		uCurrentLevel = m_pItemData->m_uCurLevel;
	}

	// needed so mem card restore can reattach correct
	// item to m_pItemData.  Store as one-based since
	// it's part of item system.
	m_nUpgradeLevel = (s16) nNewUpgradeLevel + 1;

//	m_pOwnerInventory->SetCurWeapon(0, this ,TRUE, TRUE);
//	m_pOwnerInventory->UpgradeWeapon(0, this);
//	m_pOwnerInventory->CheckForSecretDualChange();
//	m_pOwnerInventory->SafetyValve(0);
}

u32 CItemInst::HighestUpgradeAvailable() {
	// 
	FASSERT(m_pItemData != NULL);
	CItem *pI = m_pItemData;

	while( pI->m_pUpgraded != NULL ) {
		pI = pI->m_pUpgraded;
	}

	return(pI->m_uCurLevel - 1);
}

void CItemInst::SetAmmoDisplayType( AmmoDisplayType_e eClipType, AmmoDisplayType_e eReserveType ) {
	m_aeDisplayType[0] = eClipType;
	m_aeDisplayType[1] = eReserveType;
}



/////////////////////////
// CInventory functions
/////////////////////////

BOOL CInventory::InitItemInst( CItemInst *pItemInst, cchar *pszItemTagName, s32 nClipAmmo, s32 nReserveAmmo ) {

	pItemInst->m_pItemData = CItemRepository::RetrieveEntry( pszItemTagName, NULL );
	if( !pItemInst->m_pItemData ) {
		return FALSE;
	}
	pItemInst->m_nClipAmmo = nClipAmmo;
	pItemInst->m_nReserveAmmo = nReserveAmmo;
	pItemInst->m_pOwnerInventory = this;
	pItemInst->m_pWeapon = NULL;
	pItemInst->m_nUpgradeLevel = pItemInst->m_pItemData->m_uCurLevel;

	return TRUE;
}

// This sets up the inventory structure to have all items and weapons.
void CInventory::SetToDefaults() {
	CItemInst *pItemInst;

	fang_MemZero( this, sizeof( CInventory ) );

	m_uNumBatteries = 3;
	m_auNumWeapons[0] = 10;
	m_auNumWeapons[1] = 8;
	m_auCurWeapon[0] = 1;
	m_auCurWeapon[1] = 0;
	m_auSavedWeapon[0] = m_auCurWeapon[0];
	m_auSavedWeapon[1] = m_auCurWeapon[1];
	m_auEUKFlags[0] = 0;

	//// Load up the primary weapons.
	pItemInst = &m_aoWeapons[0][0];
	InitItemInst( pItemInst, "Empty Primary", 1, CItemInst_nNoAmmo );

	pItemInst = &m_aoWeapons[0][1];
	InitItemInst( pItemInst, "Laser L1", 100, 0 );

	pItemInst = &m_aoWeapons[0][2];
	InitItemInst( pItemInst, "Rivet Gun L1", 10000, 10000 );

	pItemInst = &m_aoWeapons[0][3];
	InitItemInst( pItemInst, "RLauncher L1", 10000, 10000 );

	pItemInst = &m_aoWeapons[0][4];
	InitItemInst( pItemInst, "Blaster L1", 10000, 10000 );

	pItemInst = &m_aoWeapons[0][5];
	InitItemInst( pItemInst, "Tether L1", 10000, 10000 );

	pItemInst = &m_aoWeapons[0][6];
	InitItemInst( pItemInst, "SPEW L1", 10000, 10000 );

	pItemInst = &m_aoWeapons[0][7];
	InitItemInst( pItemInst, "Mortar L1", 0, CItemInst_nNoMaxAmmo );

	pItemInst = &m_aoWeapons[0][8];
	InitItemInst( pItemInst, "Flamer L1", 10000, 10000 );

	pItemInst = &m_aoWeapons[0][9];
	InitItemInst( pItemInst, "Ripper L1", 10000, 10000 );

	//// Load up the secondary weapons.
	//
	// NOTE : "Empty Secondary" *must* be the first item loaded here.  If you absolutely have to change this, you must let Justin
	//   know as it has ramifications in other places.
	pItemInst = &m_aoWeapons[1][0];
	InitItemInst( pItemInst, "Empty Secondary", 1, CItemInst_nNoAmmo );

	pItemInst = &m_aoWeapons[1][1];
	InitItemInst( pItemInst, "Coring Charge", 3, CItemInst_nNoMaxAmmo );

	pItemInst = &m_aoWeapons[1][2];
	InitItemInst( pItemInst, "Cleaner", 20, CItemInst_nNoMaxAmmo );

	pItemInst = &m_aoWeapons[1][3];
	InitItemInst( pItemInst, "Scope L1", CItemInst_nNoAmmo, CItemInst_nNoMaxAmmo );

	pItemInst = &m_aoWeapons[1][4];
	InitItemInst( pItemInst, "EMP Grenade", 3, CItemInst_nNoMaxAmmo );

	pItemInst = &m_aoWeapons[1][5];
	InitItemInst( pItemInst, "Magma Bomb", 6, CItemInst_nNoMaxAmmo );

	pItemInst = &m_aoWeapons[1][6];
	InitItemInst( pItemInst, "Wrench", 1, 0 );

	pItemInst = &m_aoWeapons[1][7];
	InitItemInst( pItemInst, "Recruiter Grenade", 6, CItemInst_nNoMaxAmmo );

	// Load up the items.
	SetupDefaultItems();
}

// This sets up the inventory structure to have the min number of weapons.	
void CInventory::SetToInitial() {
	CItemInst *pItemInst;

	fang_MemZero( this, sizeof( CInventory ) );

	m_uNumBatteries = 3;
	m_auNumWeapons[0] = 2;
	m_auNumWeapons[1] = 1;
	m_auCurWeapon[0] = 1;
	m_auCurWeapon[1] = 0;
	m_auSavedWeapon[0] = m_auCurWeapon[0];
	m_auSavedWeapon[1] = m_auCurWeapon[1];
	m_auEUKFlags[0] = 0;

	//// Load up the primary weapons.
	pItemInst = &m_aoWeapons[0][0];
	InitItemInst( pItemInst, "Empty Primary", 1, CItemInst_nNoAmmo );

	pItemInst = &m_aoWeapons[0][1];
	InitItemInst( pItemInst, "Laser L1", 100, 0 );
	
	//// Load up the secondary weapons.
	//
	// NOTE : "Empty Secondary" *must* be the first item loaded here.  If you absolutely have to change this, you must let Justin
	//   know as it has ramifications in other places.
	pItemInst = &m_aoWeapons[1][0];
	InitItemInst( pItemInst, "Empty Secondary", 1, CItemInst_nNoAmmo );
	
	// Load up the items.
	SetupDefaultItems();
}

void CInventory::SetToUnarmed(BOOL bNoItems) {
	CItemInst *pItemInst;

	fang_MemZero( this, sizeof( CInventory ) );

	m_uNumBatteries = 3;
	m_auNumWeapons[0] = 1;
	m_auNumWeapons[1] = 1;
	m_auCurWeapon[0] = 0;
	m_auCurWeapon[1] = 0;
	m_auSavedWeapon[0] = m_auCurWeapon[0];
	m_auSavedWeapon[1] = m_auCurWeapon[1];
	m_auEUKFlags[0] = 0;

	//// Load up the primary weapons.
	pItemInst = &m_aoWeapons[0][0];
	InitItemInst( pItemInst, "Empty Primary", 1, CItemInst_nNoAmmo );
	
	//// Load up the secondary weapons.
	//
	// NOTE : "Empty Secondary" *must* be the first item loaded here.  If you absolutely have to change this, you must let Justin
	//   know as it has ramifications in other places.
	pItemInst = &m_aoWeapons[1][0];
	InitItemInst( pItemInst, "Empty Secondary", 1, CItemInst_nNoAmmo );
	
	// Load up the items.
	if (!bNoItems)
		SetupDefaultItems();
}

void CInventory::CopyInv(CInventory *pSrc, BOOL bChangeOwner) {
	//CItemInst *pItemInst;

	fang_MemZero( this, sizeof( CInventory ) );

	*this = *pSrc;

	m_uNumBatteries = pSrc->m_uNumBatteries;
	m_auNumWeapons[0] = pSrc->m_auNumWeapons[0];
	m_auNumWeapons[1] = pSrc->m_auNumWeapons[1];
	m_auCurWeapon[0] = pSrc->m_auCurWeapon[0];
	m_auCurWeapon[1] = pSrc->m_auCurWeapon[1];
	m_auSavedWeapon[0] = pSrc->m_auSavedWeapon[0];
	m_auSavedWeapon[1] = pSrc->m_auSavedWeapon[1];
	m_auEUKFlags[0] = pSrc->m_auEUKFlags[0];

	s32 i;

	for (i=0; i<m_auNumWeapons[0]; i++)	{
		m_aoWeapons[0][i] = pSrc->m_aoWeapons[0][i];

		if (bChangeOwner)
		{
			m_aoWeapons[0][i].m_pOwnerInventory = this;
			m_aoWeapons[0][i].m_pWeapon = NULL;
		}
	}
	for (i=0; i<m_auNumWeapons[1]; i++) { 
		m_aoWeapons[1][i] = pSrc->m_aoWeapons[1][i];

		if (bChangeOwner)
		{
			m_aoWeapons[1][i].m_pOwnerInventory = this;
			m_aoWeapons[1][i].m_pWeapon = NULL;
		}
	}

	m_uNumItems = pSrc->m_uNumItems;
	m_uNumPickupWeapons = pSrc->m_uNumPickupWeapons;

	for (i=0; i<m_uNumItems; i++) {
		m_aoItems[i] = pSrc->m_aoItems[i];

		if (bChangeOwner)
		{
			m_aoItems[i].m_pOwnerInventory = this;
			m_aoItems[i].m_pWeapon = NULL;
		}
	}
}

void CInventory::SetToSlosh() {
	CItemInst *pItemInst;

	fang_MemZero( this, sizeof( CInventory ) );

	m_uNumBatteries = 3;
	m_auNumWeapons[0] = 1;
	m_auNumWeapons[1] = 2;
	m_auCurWeapon[0] = 0;
	m_auCurWeapon[1] = 1;
	m_auSavedWeapon[0] = m_auCurWeapon[0];
	m_auSavedWeapon[1] = m_auCurWeapon[1];
	m_auEUKFlags[0] = 0;

	//// Load up the primary weapons.
	pItemInst = &m_aoWeapons[0][0];
	InitItemInst( pItemInst, "Flamer L1", 100, (s16)CWeapon::INFINITE_AMMO );

	//// Load up the secondary weapons.
	//
	// NOTE : "Empty Secondary" *must* be the first item loaded here.  If you absolutely have to change this, you must let Justin
	//   know as it has ramifications in other places.
	pItemInst = &m_aoWeapons[1][0];
	InitItemInst( pItemInst, "Empty Secondary", 1, CItemInst_nNoAmmo );

	pItemInst = &m_aoWeapons[1][1];
	InitItemInst( pItemInst, "Magma Bomb", 5, CItemInst_nNoMaxAmmo );

	// Load up the items.
	SetupDefaultItems();
}

void CInventory::SetToKrunk() {
	CItemInst *pItemInst;

	fang_MemZero( this, sizeof( CInventory ) );

	m_uNumBatteries = 3;
	m_auNumWeapons[0] = 1;
	m_auNumWeapons[1] = 2;
	m_auCurWeapon[0] = 0;
	m_auCurWeapon[1] = 1;
	m_auSavedWeapon[0] = m_auCurWeapon[0];
	m_auSavedWeapon[1] = m_auCurWeapon[1];
	m_auEUKFlags[0] = 0;

	//// Load up the primary weapons.
	pItemInst = &m_aoWeapons[0][0];
	InitItemInst( pItemInst, "Tether Krunk", 1, (s16)CWeapon::INFINITE_AMMO );

	//// Load up the secondary weapons.
	//
	// NOTE : "Empty Secondary" *must* be the first item loaded here.  If you absolutely have to change this, you must let Justin
	//   know as it has ramifications in other places.
	pItemInst = &m_aoWeapons[1][0];
	InitItemInst( pItemInst, "Empty Secondary", 1, CItemInst_nNoAmmo );

	pItemInst = &m_aoWeapons[1][1];
	InitItemInst( pItemInst, "EMP Grenade", 7, CItemInst_nNoMaxAmmo );

	//// Load up the items.
	SetupDefaultItems();
}

void CInventory::SetToMozer() {
	CItemInst *pItemInst;

	fang_MemZero( this, sizeof( CInventory ) );

	m_uNumBatteries = 3;
	m_auNumWeapons[0] = 1;
	m_auNumWeapons[1] = 1;
	m_auCurWeapon[0] = 0;
	m_auCurWeapon[1] = 1;
	m_auSavedWeapon[0] = m_auCurWeapon[0];
	m_auSavedWeapon[1] = m_auCurWeapon[1];
	m_auEUKFlags[0] = 0;

	//// Load up the primary weapons.
	pItemInst = &m_aoWeapons[0][0];
	InitItemInst( pItemInst, "Rivet Gun L1", 100, (s16)CWeapon::INFINITE_AMMO );

	//// Load up the secondary weapons.
	//
	// NOTE : "Empty Secondary" *must* be the first item loaded here.  If you absolutely have to change this, you must let Justin
	//   know as it has ramifications in other places.
	pItemInst = &m_aoWeapons[1][0];
	InitItemInst( pItemInst, "Empty Secondary", 1, CItemInst_nNoAmmo );

	// Load up the items.
	SetupDefaultItems();
}

BOOL CInventory::SetupItem( u32 nItemIndex, cchar *pszItemTagName, s32 nCount ) {
	
	FASSERT( nItemIndex < INVPOS_COUNT );

	CItemInst *pItemInst = &m_aoItems[nItemIndex];
	pItemInst->SetAmmoDisplayType( CItemInst::AMMODISPLAY_NUMERIC, CItemInst::AMMODISPLAY_NONE );
	if( !InitItemInst( pItemInst, pszItemTagName, nCount, CItemInst_nNoMaxAmmo ) ) {
		return FALSE;
	}
	return TRUE;
}

void CInventory::SetupDefaultItems() {
	
	// washers
	SetupItem( INVPOS_WASHER, ItemInst_apszItemNames[INVPOS_WASHER], 99 );

	// chips
	SetupItem( INVPOS_CHIP, ItemInst_apszItemNames[INVPOS_CHIP], 0 );

	// secret chips
	SetupItem( INVPOS_SECRETCHIP, ItemInst_apszItemNames[INVPOS_SECRETCHIP], 0 );

	// mil translator
	//SetupItem( INVPOS_MILTRANS, ItemInst_apszItemNames[INVPOS_MILTRANS], 1 );

	// antenna
	//SetupItem( INVPOS_ANTENNA, ItemInst_apszItemNames[INVPOS_ANTENNA], 1 );

#if 0
	// mission briefing
	SetupItem( INVPOS_MISBRIEFING, ItemInst_apszItemNames[INVPOS_MISBRIEFING], 1 );
#endif

	// arm servo
	SetupItem( INVPOS_ARM_SERVO, ItemInst_apszItemNames[INVPOS_ARM_SERVO], 1 );

	// det pack
	SetupItem( INVPOS_DETPACK, ItemInst_apszItemNames[INVPOS_DETPACK], 4 );

#if 0
	// EUK
	SetupItem( INVPOS_EUK, ItemInst_apszItemNames[INVPOS_EUK], 0 );
#endif

	// Item count
	m_uNumItems = INVPOS_COUNT;

	m_uNumPickupWeapons = 0;
}

BOOL CInventory::SetCurWeapon(u32 uWhichSide, u32 uNewWeaponIdx, BOOL bRestoreSaved, BOOL bNoReloadCallback)
{
	FASSERT((s32)uNewWeaponIdx < m_auNumWeapons[uWhichSide]);

	u32 uOldItemIdx = m_auCurWeapon[uWhichSide];
	CItemInst *pIIOld = &(m_aoWeapons[uWhichSide][uOldItemIdx]);
	CItem *pIOld = pIIOld->m_pItemData;
	CItemInst *pIINew = &(m_aoWeapons[uWhichSide][uNewWeaponIdx]);
	CItem *pINew = pIINew->m_pItemData;

	u32 auOldItemIdx[2];
	auOldItemIdx[0] = m_auCurWeapon[0];
	auOldItemIdx[1] = m_auCurWeapon[1];

#if 0
	if(uWhichSide == 0)
	{
		//ME:  can equip left handed wpns with a two handed wpn selected
		//if((!pIOld->IsDual()) && (pINew->IsDual()))
		//{
		//	DEVPRINTF("Non-dual to dual transition.\n");

		//	m_auSavedWeapon[1] = m_auCurWeapon[1];
		//	m_auCurWeapon[1] = 0;
		//}
		//else if(bRestoreSaved)
		//{
		//	if((pIOld->IsDual()) && (!pINew->IsDual()))
		//	{
		//		DEVPRINTF("Dual to non-dual transition.\n");

		//		u32 uTemp = m_auCurWeapon[1];
		//		m_auCurWeapon[1] = m_auSavedWeapon[1];
		//		m_auSavedWeapon[1] = uTemp;
		//	}
		//}
	}
	else
	{
		FASSERT(uWhichSide == 1);
		if((uOldItemIdx == 0) && (uNewWeaponIdx != 0))
		{
			DEVPRINTF("Empty to non-empty transition.\n");

			//u32 uTemp = m_auCurWeapon[0];
			//ME:  grenades & RL are ok now
			//if(m_aoWeapons[0][uTemp].m_pItemData->IsDual())
			//{
			//	FASSERT(!m_aoWeapons[0][m_auSavedWeapon[0]].m_pItemData->IsDual());
			//	m_auCurWeapon[0] = m_auSavedWeapon[0];
			//	m_auSavedWeapon[0] = uTemp;
			//}
		}
/*		else if(bRestoreSaved)
		{
			if((uOldItemIdx != 0) && (uNewWeaponIdx == 0))
			{
				DEVPRINTF("Non-empty to empty transition.\n");

				u32 uTemp = m_auCurWeapon[0];
				m_auCurWeapon[0] = m_auSavedWeapon[0];
				m_auSavedWeapon[0] = uTemp;
			}
		}*/
	}
#endif

#if 0
	if((!pIOld->IsDual()) || !pINew->IsDual())
	{
		m_auSavedWeapon[uWhichSide] = uOldItemIdx;
	}
	m_auCurWeapon[uWhichSide] = uNewWeaponIdx;

#if FANG_DEBUG_BUILD
	u32 uCurPrimary = m_auCurWeapon[0];
	u32 uCurSecondary = m_auCurWeapon[1];

	if(m_aoWeapons[0][uCurPrimary].m_pItemData->IsDual())
		FASSERT(uCurSecondary == 0);
	else if(uCurSecondary != 0)
		FASSERT(!m_aoWeapons[0][uCurPrimary].m_pItemData->IsDual());
#endif
#endif //0

	m_auCurWeapon[uWhichSide] = uNewWeaponIdx;
	BOOL bRetVal = TRUE;

	//SafetyValve(uWhichSide);

	if(m_pfcnCallback != NULL)
	{
		// Check to see if the primary was changed.
		if(m_auCurWeapon[0] != auOldItemIdx[0])
		{
			bRetVal &= m_pfcnCallback(IREASON_WEAPONCHANGE, this, 0, m_auCurWeapon[0]);
		}
		// Check to see if the secondary was changed.
		if(m_auCurWeapon[1] != auOldItemIdx[1])
		{
			bRetVal &= m_pfcnCallback(IREASON_WEAPONCHANGE, this, 1, m_auCurWeapon[1]);
		}

		// If nothing changed, possibly issue a reload message.
		if((uOldItemIdx == uNewWeaponIdx) && (!bNoReloadCallback))
		{
			FASSERT(bRetVal);
			FASSERT(m_auCurWeapon[0] == auOldItemIdx[0]);
			FASSERT(m_auCurWeapon[1] == auOldItemIdx[1]);
			bRetVal &= m_pfcnCallback(IREASON_RELOAD, this, uWhichSide, uNewWeaponIdx);
		}
	}

	return(bRetVal);
}

BOOL CInventory::SetCurWeapon(u32 uWhichSide, CItemInst *pII, BOOL bRestoreSaved, BOOL bNoReloadCallback) {
	// Search through our items for a matching CItem.
	u32 uCurWeapIdx, uInventorySize = m_auNumWeapons[uWhichSide];
	for(uCurWeapIdx = 0; uCurWeapIdx < uInventorySize; ++uCurWeapIdx)
	{
		if(&(m_aoWeapons[uWhichSide][uCurWeapIdx]) == pII)
			break;
	}
	FASSERT(uCurWeapIdx < uInventorySize);

	return(SetCurWeapon(uWhichSide, uCurWeapIdx, bRestoreSaved, bNoReloadCallback));
}

BOOL CInventory::UpgradeWeapon(u32 uWhichSide, u32 uWhichWeapon) {
/*	CItemInst *pII = &(m_aoWeapons[uWhichSide][uWhichWeapon]);
	FASSERT(pII != NULL);

	// 
	CItem *pI = pII->m_pItemData;
	FASSERT(pI != NULL);

	CItem *pNewI = pII->m_pItemData->m_pUpgraded;
	FASSERT(pNewI != NULL);

	pII->m_pItemData = pNewI;
*/
#if 0
	u32 uNumEUKS = m_aoItems[INVPOS_EUK].m_nClipAmmo;
	FASSERT(uNumEUKS > 0);

	//////////////////////////////////////////////////////////////////////
	// Shift the EUK flags array downward one and remove it.
	u32 uCurItemIdx;
	for(uCurItemIdx = 1; uCurItemIdx < uNumEUKS; ++uCurItemIdx)
		m_auEUKFlags[uCurItemIdx - 1] = m_auEUKFlags[uCurItemIdx];

	--m_aoItems[INVPOS_EUK].m_nClipAmmo;
	//
	//////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////
	// Call the callback and tell it that we upgraded a weapon.
	if(m_pfcnCallback != NULL)
	{
		m_pfcnCallback(IREASON_WEAPONUPGRADED, this, uWhichSide, uWhichWeapon);
	}
#endif
	//
	//////////////////////////////////////////////////////////////////////

	// TRUE just means that the weapon was able to be upgraded.
	return(TRUE);
}

BOOL CInventory::UpgradeWeapon(u32 uWhichSide, CItemInst *pII) {
	// Search through our items for a matching CItem.
	u32 uCurWeapIdx, uInventorySize = m_auNumWeapons[uWhichSide];
	for(uCurWeapIdx = 0; uCurWeapIdx < uInventorySize; ++uCurWeapIdx)
	{
		if(&(m_aoWeapons[uWhichSide][uCurWeapIdx]) == pII)
			break;
	}
	FASSERT(uCurWeapIdx < uInventorySize);

	return(UpgradeWeapon(uWhichSide, uCurWeapIdx));
}

//-----------------------------------------------------------------------------
BOOL CInventory::CycleToNextWeapon(u32 uWhichSide, BOOL bNoReloadCallback)
{
	u32 uOldItemIdx = m_auCurWeapon[uWhichSide];

	u32 uNewWeaponIdx = uOldItemIdx + 1;
	if ((s32)uNewWeaponIdx >= m_auNumWeapons[uWhichSide])
		uNewWeaponIdx = 0;

	return SetCurWeapon( uWhichSide, uNewWeaponIdx, TRUE, bNoReloadCallback);
}

//-----------------------------------------------------------------------------
BOOL CInventory::CycleToPrevWeapon(u32 uWhichSide, BOOL bNoReloadCallback)
{
	s32 nOldItemIdx = m_auCurWeapon[uWhichSide];

	s32 nNewWeaponIdx = nOldItemIdx - 1;
	if (nNewWeaponIdx < 0)
		nNewWeaponIdx = m_auNumWeapons[uWhichSide] - 1;

	return SetCurWeapon( uWhichSide, nNewWeaponIdx, TRUE, bNoReloadCallback);
}

//-----------------------------------------------------------------------------
BOOL CInventory::SafetyValve(u32 uWhichSideIsCorrect) {
	FASSERT_NOW;
	return FALSE;
#if 0
	CItemInst *pCurPrim = &m_aoWeapons[0][ m_auCurWeapon[0] ];

	if((pCurPrim->m_pItemData->IsDual()) && (m_auCurWeapon[1] != 0)) {
		// Yup, we're in an inconsistent state, let's correct it.
		switch(uWhichSideIsCorrect)
		{
			case 0:
			{
				m_auCurWeapon[1] = 0;
				break;
			}
			case 1:
			{
				m_auCurWeapon[0] = 1;
				break;
			}
		}
		return(TRUE);
	}

	return(FALSE);
#endif
}

// NKM
// Is the weapon in the inventory.  
// If bUseTagName is FALSE, will use the code name, for example: "SPEW", "Ripper", etc
// If bUseTagName is TRUE, will use names like, "Rocket L1", "Rivet L3" and not "Rivet Gun L1".
CItemInst *CInventory::IsWeaponInInventory( cchar *pszWeaponName, BOOL bUseTagName/* = FALSE*/ ) {
	CItem *pItem = CItemRepository::RetrieveEntry( pszWeaponName, NULL );

	if( !pItem ) {
		// We shouldn't get here.  If we did, we got a bad weapon name or one that isn't in the itemlist.csv
		FASSERT_NOW;
	}

	// Compare code names since we don't want to compare the tag names, that would give duplicate weapons
	for( s8 i = 0; i < m_auNumWeapons[ INV_INDEX_PRIMARY ]; ++i ) {
		if( !bUseTagName ) {
			if( !fclib_stricmp( pItem->m_pszCodeName, m_aoWeapons[ INV_INDEX_PRIMARY ][i].m_pItemData->m_pszCodeName ) ) {
				return &m_aoWeapons[ INV_INDEX_PRIMARY ][i];
			}
		} else {
			if( !fclib_stricmp( pItem->m_pszTag, m_aoWeapons[ INV_INDEX_PRIMARY ][i].m_pItemData->m_pszTag ) ) {
				return &m_aoWeapons[ INV_INDEX_PRIMARY ][i];
			}
		}
	}

	for( s8 i = 0; i < m_auNumWeapons[ INV_INDEX_SECONDARY ]; ++i ) {
		if( !bUseTagName ) {
			if( !fclib_stricmp( pItem->m_pszCodeName, m_aoWeapons[ INV_INDEX_SECONDARY ][i].m_pItemData->m_pszCodeName ) ) {
				return &m_aoWeapons[ INV_INDEX_SECONDARY ][i];
			}
		} else {
			if( !fclib_stricmp( pItem->m_pszTag, m_aoWeapons[ INV_INDEX_SECONDARY ][i].m_pItemData->m_pszTag ) ) {
				return &m_aoWeapons[ INV_INDEX_SECONDARY ][i];
			}
		}
	}

	return NULL;
}

CItemInst *CInventory::IsItemInInventory( cchar *pszItemName ) {
	for( u32 i = 0; i < m_uNumItems; ++i ) {
		if( m_aoItems[i].m_pItemData == NULL ) {
			continue;
		}

		if( !fclib_stricmp( pszItemName, m_aoItems[i].m_pItemData->m_pszTag ) ) {
			return &m_aoItems[i];
		}
	}

	return NULL;
}

void CInventory::InitForMultiplayer( FGameDataTableHandle_t hTable, u32 ePrimary, u32 eSecondary ) {

	BOOL bPrimaryFromInv = (ePrimary == GAMESAVE_PRIMARY_WEAPON_LIMIT_NO_LIMIT);
	BOOL bSecondaryFromInv = (eSecondary == GAMESAVE_SECONDARY_WEAPON_LIMIT_NO_LIMIT);

	// Try to load from our csv file if possible
	BOOL bLoaded = InitFromCSVTable( hTable, bPrimaryFromInv, bSecondaryFromInv );

	// If that failed and the weapons are not restricted, then just load up everything and quit
	if( !bLoaded && bPrimaryFromInv && bSecondaryFromInv ) {
		SetToDefaults();
		return;
	}

	// If the load from CSV failed and we are still here, then try and set up
	// the inventory manually.
	if( !bLoaded ) {
		fang_MemZero( this, sizeof( CInventory ) );

		m_uNumBatteries = 3;
		m_auNumWeapons[0] = 0;
		m_auNumWeapons[1] = 0;
		m_auCurWeapon[0] = 0;
		m_auCurWeapon[1] = 0;
		m_auSavedWeapon[0] = m_auCurWeapon[0];
		m_auSavedWeapon[1] = m_auCurWeapon[1];
		m_auEUKFlags[0] = 0;
	}

	// Set up the primary weapons
	if ( !bLoaded || !bPrimaryFromInv ) {
		FASSERT( m_auNumWeapons[0] == 0 );

		// If weapons are not restricted, ePrimary/eSecondary still point to a default weapon
		InitItemInst( &m_aoWeapons[0][0], _sPrimaryItemTable[ePrimary].m_szName, _sPrimaryItemTable[ePrimary].m_nClipAmmo, _sPrimaryItemTable[ePrimary].m_nReserveAmmo );
		m_auNumWeapons[0]++;

		// If there are AI bots in the world, give him a tether
		if( MultiplayerMgr.PossessionEnabled() ) {
			InitItemInst( &m_aoWeapons[0][1], "Tether L3", 10000, 10000 );
			m_auNumWeapons[0]++;
		}
	}

	if( !bLoaded || !bSecondaryFromInv ) {
		FASSERT( m_auNumWeapons[1] == 0 );

		InitItemInst( &m_aoWeapons[1][0], _sSecondaryItemTable[eSecondary].m_szName, _sSecondaryItemTable[eSecondary].m_nClipAmmo, _sSecondaryItemTable[eSecondary].m_nReserveAmmo );
		m_auNumWeapons[1]++;
	}

	// We are done
	SetupDefaultItems();
	return;
}

BOOL CInventory::InitFromCSVTable( FGameDataTableHandle_t hTable, BOOL bLoadPrimary, BOOL bLoadSecondary ) {
	
	fang_MemZero( this, sizeof( CInventory ) );

	if( hTable == FGAMEDATA_INVALID_TABLE_HANDLE ) {
		return FALSE;
	}
	u32 nNumFields = fgamedata_GetNumFields( hTable );
	if( nNumFields < (6 + 3 + 2) ) {
		DEVPRINTF( "CInventory::InitFromCSVTable() - not enough fields in the table.\n" );
		return FALSE;
	}
	
	// fill in a gamedata table entry profile to read each field in
	FGameData_TableEntry_t FloatEntry, StringEntry;	
	FloatEntry.nFlags = (FGAMEDATA_VAR_TYPE_FLOAT | FGAMEDATA_FLAGS_FLOAT_X);
	FloatEntry.nBytesForData = sizeof( f32 );

	StringEntry.nFlags = (FGAMEDATA_VAR_TYPE_STRING | FGAMEDATA_FLAGS_STRING_PTR_ONLY);
	StringEntry.nBytesForData = sizeof( char * );

	// read in the header data
	f32 fNumBatteries, fNumPrimary, fNumSecondary, fNumItems, fStartingPrimary, fStartingSecondary;
	u32 nIndex = 0;
	if( !fgamedata_GetFieldFromTable( hTable, nIndex++, &FloatEntry, &fNumBatteries ) ||
		!fgamedata_GetFieldFromTable( hTable, nIndex++, &FloatEntry, &fNumPrimary ) ||
		!fgamedata_GetFieldFromTable( hTable, nIndex++, &FloatEntry, &fNumSecondary ) ||
		!fgamedata_GetFieldFromTable( hTable, nIndex++, &FloatEntry, &fNumItems ) ||
		!fgamedata_GetFieldFromTable( hTable, nIndex++, &FloatEntry, &fStartingPrimary ) ||
		!fgamedata_GetFieldFromTable( hTable, nIndex++, &FloatEntry, &fStartingSecondary ) ) {
		DEVPRINTF( "CInventory::InitFromCSVTable() - trouble reading header fields.\n" );
		return FALSE;
	}

	// make sure that there are enough fields in the table
	u32 nNumRequiredFields = (u32)( 6.0f + ((fNumPrimary + fNumSecondary) * 3.0f) + (fNumItems * 2.0f) );
	if( nNumFields != nNumRequiredFields ) {
		DEVPRINTF( "CInventory::InitFromCSVTable() - the passed in table has %d fields, not the required %d.\n", nNumFields, nNumRequiredFields );
		return FALSE;
	}

	// do some error checking
	if( fNumBatteries < 1.0f || 
		fNumPrimary < 1.0f ||
		fNumSecondary < 1.0f || 
		fNumItems < 0.0f ||
		fStartingPrimary >= fNumPrimary ||
		fStartingSecondary >= fNumSecondary ) {
		DEVPRINTF( "CInventory::InitFromCSVTable() - invalid data in the header fields.\n" );
		return FALSE;
	}

	// init to having 0 weapons, will set to actual values if we successfully setup the all weapons
	m_auNumWeapons[0] = 0;
	m_auNumWeapons[1] = 0;
	m_uNumBatteries = (u32)fNumBatteries;
	m_auEUKFlags[0] = 0;

	// read in the primary weapons
	u32 i, nCount, j, nNumValid;
	cchar *pszString;
	f32 fClipSize, fReserveAmmo;
	CItemInst *pItemInst;
	nCount = (u32)fNumPrimary;
	nNumValid = 0;
	for( i=0; i < nCount; i++ ) {
		// read the primary weapon info (name, clip size, reserve ammo)
		if( !fgamedata_GetFieldFromTable( hTable, nIndex++, &StringEntry, &pszString ) ||
			!fgamedata_GetFieldFromTable( hTable, nIndex++, &FloatEntry, &fClipSize ) ||
			!fgamedata_GetFieldFromTable( hTable, nIndex++, &FloatEntry, &fReserveAmmo ) ) {
			DEVPRINTF( "CInventory::InitFromCSVTable() - trouble reading primary weapon info %d.\n", i );
			return FALSE;
		}

		// Don't actually load the weapon if so requested (limited ammo case)
		if( bLoadPrimary ) {
			// make sure that the item doesn't already exist
			for( j=0; j < i; j++ ) {
				if( m_aoWeapons[0][j].m_pItemData ) {
					if( fclib_stricmp( m_aoWeapons[0][j].m_pItemData->m_pszCodeName, pszString ) == 0 ) {
						// oophs, each weapon can only occupy 1 weapon slot
						DEVPRINTF( "CInventory::InitFromCSVTable() - the primary weapon %s is not valid, an early weapon was the same type.\n\tOnly 1 EUK may be put into the inventory.\n", pszString );
						return FALSE;
					}
				}
			}

			pItemInst = &m_aoWeapons[0][nNumValid];
			if( !InitItemInst( pItemInst, pszString, (s32)fClipSize, (s32)fReserveAmmo ) ) {
				DEVPRINTF( "CInventory::InitFromCSVTable() - trouble initing the primary item named %s.\n", pszString );
			} else {
				nNumValid++;
			}
		}
	}

	if( bLoadPrimary ) {
		m_auNumWeapons[0] = nNumValid;
		m_auCurWeapon[0] = (u32)fStartingPrimary;
		FMATH_CLAMP( m_auCurWeapon[0], 0, nNumValid-1 );
	}
	m_auSavedWeapon[0] = m_auCurWeapon[0];

	// read in the secondary weapons
	nCount = (u32)fNumSecondary;
	nNumValid = 0;
	for( i=0; i < nCount; i++ ) {
		// read the secondary weapon info (name, clip size, reserve ammo)
		if( !fgamedata_GetFieldFromTable( hTable, nIndex++, &StringEntry, &pszString ) ||
			!fgamedata_GetFieldFromTable( hTable, nIndex++, &FloatEntry, &fClipSize ) ||
			!fgamedata_GetFieldFromTable( hTable, nIndex++, &FloatEntry, &fReserveAmmo ) ) {
			DEVPRINTF( "CInventory::InitFromCSVTable() - trouble reading secondary weapon info %d.\n", i );
			return FALSE;
		}

		if( bLoadSecondary ) {
			// make sure that the item doesn't already exist
			for( j=0; j < i; j++ ) {
				if( m_aoWeapons[1][j].m_pItemData ) {
					if( fclib_stricmp( m_aoWeapons[1][j].m_pItemData->m_pszCodeName, pszString ) == 0 ) {
						// oophs, each weapon can only occupy 1 weapon slot
						DEVPRINTF( "CInventory::InitFromCSVTable() - the secondary weapon %s is not valid, an early weapon was the same type.\n\tOnly 1 EUK may be put into the inventory.\n", pszString );
						return FALSE;
					}
				}
			}

			pItemInst = &m_aoWeapons[1][nNumValid];
			if( !InitItemInst( pItemInst, pszString, (s32)fClipSize, (s32)fReserveAmmo ) ) {
				DEVPRINTF( "CInventory::InitFromCSVTable() - trouble initing the secondary item named %s.\n", pszString );
			} else {
				nNumValid++;
			}
		}
	}

	if( bLoadSecondary ) {
		m_auNumWeapons[1] = nNumValid;
		m_auCurWeapon[1] = (u32)fStartingSecondary;
		FMATH_CLAMP( m_auCurWeapon[1], 0, nNumValid-1 );
	}
	m_auSavedWeapon[1] = m_auCurWeapon[1];
	
	// setup the items
	nCount = (u32)fNumItems;
	if( nCount > INVPOS_COUNT ) {
		DEVPRINTF( "CInventory::InitFromCSVTable() - there are too many items (%d) listed, the max is %d.\n", nCount, INVPOS_COUNT );
	}
	
	// start with the default items and over write
	SetupDefaultItems();

	for( i=0; i < nCount; i++ ) {
		// read the item info (name, count)
		if( !fgamedata_GetFieldFromTable( hTable, nIndex++, &StringEntry, &pszString ) ||
			!fgamedata_GetFieldFromTable( hTable, nIndex++, &FloatEntry, &fClipSize ) ) {
			DEVPRINTF( "CInventory::InitFromCSVTable() - trouble reading item info %d.\n", i );
			return FALSE;
		}
		// compare the string to our know time names
		for( j=0; j < INVPOS_COUNT; j++ ) {
			if( fclib_stricmp( pszString, ItemInst_apszItemNames[j] ) == 0 ) {
				// match
				if( SetupItem( j, ItemInst_apszItemNames[j], (s32)fClipSize ) ) {
					break;
				} else {
					DEVPRINTF( "CInventory::InitFromCSVTable() - trouble initing the item named %s.\n", pszString );
					return FALSE;
				}
			}
		}
		if( j == INVPOS_COUNT ) {
			// didn't find a match
			DEVPRINTF( "CInventory::InitFromCSVTable() - could not find an item named %s, check the spelling.\n", pszString );
		}
	}
	
	return TRUE;	
}


//////////////////////////////
// CMemCardItemInst functions
//////////////////////////////

u32 CMemCardItemInst::GenerateNameCRC( const CItem *pItem ) {
	u8 szLowerCaseItemName[64];

	if( pItem ) {
		fclib_strncpy( (char *)&szLowerCaseItemName[0], pItem->m_pszTag, 63 );
		szLowerCaseItemName[63] = 0;
		fclib_strlwr( (char *)&szLowerCaseItemName[0] );

		return fmath_Crc32( 0, szLowerCaseItemName, 64 );
	}
	return 0;
}

void CMemCardItemInst::SaveInventory( CMemCardItemInst *pSave, const CItemInst *pItem ) {

	// first generate a CRC of the pItem
	pSave->m_nItemNameCRC = GenerateNameCRC( pItem->m_pItemData );

	pSave->m_nUpgradeLevel = pItem->m_nUpgradeLevel;
	pSave->m_nClipAmmo = pItem->m_nClipAmmo;
	pSave->m_nReserveAmmo = pItem->m_nReserveAmmo;
	pSave->m_aDisplayType[0] = pItem->m_aeDisplayType[0];
	pSave->m_aDisplayType[1] = pItem->m_aeDisplayType[1];
}

BOOL CMemCardItemInst::RestoreInventory( const CMemCardItemInst *pSave, CItemInst *pItem, const CInventory *pInv ) {
	u32 i, nNumItems, nCRC;
	CItem *pPtr;

	pItem->m_pOwnerInventory = (CInventory *)pInv;
	
	if( pSave->m_nItemNameCRC ) {
		nNumItems = CItemRepository::GetNumItems();
		for( i=0; i < nNumItems; i++ ) {
			pPtr = CItemRepository::RetrieveEntry( i );
			nCRC = GenerateNameCRC( pPtr );
			if( nCRC == pSave->m_nItemNameCRC ) {
				// match
				pItem->m_pItemData = pPtr;
				break;
			}
		}
		if( i == nNumItems ) {
			// didn't find a match
			pItem->m_pItemData = NULL;
			DEVPRINTF( "CMemCardItemInst::RestoreInventory(): could not find a matching item for Item CRC %d.\n", pSave->m_nItemNameCRC );
			return FALSE;
		}
	} else {
		pItem->m_pItemData = NULL;
	}

	pItem->m_nUpgradeLevel = pSave->m_nUpgradeLevel;
	pItem->m_nClipAmmo = pSave->m_nClipAmmo;
	pItem->m_nReserveAmmo = pSave->m_nReserveAmmo;
	pItem->m_aeDisplayType[0] = pSave->m_aDisplayType[0];
	pItem->m_aeDisplayType[1] = pSave->m_aDisplayType[1];
	
	return TRUE;
}

///////////////////////////////
// CMemCardInventory functions
///////////////////////////////

void CMemCardInventory::SaveInventory( CMemCardInventory *pSave, const CInventory *pInv ) {
	u32 i, j;

	pSave->m_nNumItems = pInv->m_uNumItems;
	pSave->m_nNumBatteries = pInv->m_uNumBatteries;

	for( i=0; i < INV_INDEX_COUNT; i++ ) {
		pSave->m_anNumWeapons[i] = pInv->m_auNumWeapons[i];
		pSave->m_anCurWeapon[i] = pInv->m_auCurWeapon[i];
		pSave->m_anSavedWeapon[i] = pInv->m_auSavedWeapon[i];

		for( j=0; j < ItemInst_uMaxInventoryWeapons; j++ ) {
			CMemCardItemInst::SaveInventory( &pSave->m_aWeapons[i][j], &pInv->m_aoWeapons[i][j] );
		}	
	}

	for( i=0; i < ItemInst_uMaxInventoryItems; i++ ) {
		CMemCardItemInst::SaveInventory( &pSave->m_aItems[i], &pInv->m_aoItems[i] );
	}

	for( i=0; i < ItemInst_uMaxPossibleEUKS; i++ ) {
		pSave->m_anEUKFlags[i] = pInv->m_auEUKFlags[i];
	}	
}

BOOL CMemCardInventory::RestoreInventory( const CMemCardInventory *pSave, CInventory *pInv ) {
	u32 i, j;

	fang_MemZero( pInv, sizeof( CInventory ) );

	pInv->m_uNumItems = pSave->m_nNumItems;
	pInv->m_uNumBatteries = pSave->m_nNumBatteries;
	pInv->m_pfcnCallback = NULL;
	pInv->m_pUser = NULL;

	for( i=0; i < INV_INDEX_COUNT; i++ ) {
		pInv->m_auNumWeapons[i] = pSave->m_anNumWeapons[i];
		pInv->m_auCurWeapon[i] = pSave->m_anCurWeapon[i];
		pInv->m_auSavedWeapon[i] = pSave->m_anSavedWeapon[i];

		for( j=0; j < ItemInst_uMaxInventoryWeapons; j++ ) {
			if( !CMemCardItemInst::RestoreInventory( &pSave->m_aWeapons[i][j], &pInv->m_aoWeapons[i][j], pInv ) ) {
				return FALSE;
			}
			if( pInv->m_aoWeapons[i][j].m_pItemData && 
				pInv->m_aoWeapons[i][j].m_pItemData->m_uCurLevel != pInv->m_aoWeapons[i][j].m_nUpgradeLevel &&
				pInv->m_aoWeapons[i][j].m_pItemData->m_uMaxLevel != 0 ) {
				u32 nFoo;

				nFoo = 99;
				//pInv->m_aoWeapons[i][j].NotifyWeaponUpgraded( pInv->m_aoWeapons[i][j].m_nUpgradeLevel );
			}
		}		
	}

	for( i=0; i < ItemInst_uMaxInventoryItems; i++ ) {
		if( !CMemCardItemInst::RestoreInventory( &pSave->m_aItems[i], &pInv->m_aoItems[i], pInv ) ) {
			return FALSE;
		}
	}

	for( i=0; i < ItemInst_uMaxPossibleEUKS; i++ ) {
		pInv->m_auEUKFlags[i] = pSave->m_anEUKFlags[i];
	}	
	
	return TRUE;	
}




