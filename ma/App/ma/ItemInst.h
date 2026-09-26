//////////////////////////////////////////////////////////////////////////////////////
// ItemInst.h - Item Instance Class for Mettle Arms.
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
#ifndef _ITEMINST_H_
#define _ITEMINST_H_ 1

#include "fang.h"
#include "ftext.h"

///////////////////////
// forward declaration
class CItem;
class CWeapon;
struct FDrawVtx_t;
class CInventory;

//////////////////////
// defines
const u32 ItemInst_uMaxInventoryWeapons = 16;
const u32 ItemInst_uMaxInventoryItems = 16;
const u32 ItemInst_uMaxPossibleEUKS = 10;

// Use these constants in the m_nMaxAmmo field for special 
const s16 CItemInst_nNoAmmo = -2;			// Display no ammo information whatsoever for this CItemInst.
const s16 CItemInst_nNoMaxAmmo = -1;		// Display only current ammo information for this CItemInst.

enum InventoryPos_e {
	INVPOS_WASHER = 0,
	INVPOS_CHIP,
	INVPOS_SECRETCHIP,
	INVPOS_ARM_SERVO,
	//INVPOS_MILTRANS,
	//INVPOS_ANTENNA,
	INVPOS_DETPACK,
//	INVPOS_EUK,
//	INVPOS_MISBRIEFING,
	INVPOS_COUNT,
};

enum InventoryIndex_e {
	INV_INDEX_PRIMARY = 0,
	INV_INDEX_SECONDARY,

	INV_INDEX_COUNT
};

enum CInventoryCallbackReason_e {
	IREASON_WEAPONCHANGE,			// bIsPrimary will contain whether the change occurred on the primary or secondary side,
									// uWpnIdx will contain the index of the newly selected weapon.
	IREASON_RELOAD,					// bIsPrimary will contain whether the reload occurred on the primary or secondary side,
									// uWpnIdx will contain the index of the weapon that is getting reloaded.
	IREASON_WEAPONUPGRADED,			// bIsPrimary will contain whether the upgrade occurred on the primary or secondary side,
									// uWpnIdx will contain the index of the weapon that got upgraded.
};

typedef BOOL CInventoryCallback_t(CInventoryCallbackReason_e eReason, CInventory *pInventory, u32 uHandIndex, u32 uWpnIdx);

////////////////////
// public variables
extern cchar *ItemInst_apszItemNames[INVPOS_COUNT];



///////////////////////////////
// a single instance of a CItem
class CItemInst  
{
public:
	CItemInst();
	~CItemInst();

	// These will print or return the text to the specified areas.
	// Logic about what should be written according to the ammo flags (see above) is handled internally.
	// If you want to force one or the other to not draw, send it 0 as the handle or NULL as the pointer.
	void PrintItemText( FTextAreaHandle_t hAreaCur, FTextAreaHandle_t hAreaMax,
						FDrawVtx_t *pvtxArray=NULL, u32 *puStartVtx=NULL,
						BOOL bScaleCurText=TRUE, BOOL bScaleMaxText=TRUE);
	void GetItemText( wchar *pszClipAmmo, wchar *pszReserveAmmo, u32 nStringLength );
	CWeapon *MakeWeapon();
	void NotifyAmmoMightHaveChanged( u16 nNewClipAmmo, u16 nNewReserveAmmo );
	void NotifyWeaponUpgraded( u32 nNewUpgradeLevel );

	enum AmmoDisplayType_e {
		AMMODISPLAY_NONE =	0x00,// 8 bit
		AMMODISPLAY_NUMERIC,
		AMMODISPLAY_METER,

		AMMODISPLAY_COUNT
	};
	void SetAmmoDisplayType( AmmoDisplayType_e eClipType, AmmoDisplayType_e eReserveType );

	u32 HighestUpgradeAvailable();

	//////////////////////////////////////////////////////////////////
	// DATA: KEEP FOOTPRINT SMALL, THIS DATA IS SAVED TO MEMORY CARDS
	//////////////////////////////////////////////////////////////////

	// If this CItemInst is in an inventory, this variable needs to point to it.
	CInventory *m_pOwnerInventory;

	// pointer to item data object for current euk level of this item instance (if a weapon).
	CItem *m_pItemData;

	CWeapon *m_pWeapon;	// NULL=none

	// upgrade level of this item instance.	Needed so mem card restore can reattach correct
	// item to m_pItemData.  One-based index.
	s16 m_nUpgradeLevel;

	// In the case of items, these can also be current and maximum counts respectively.
	s16 m_nClipAmmo;
	s16 m_nReserveAmmo;

	u8 m_aeDisplayType[2];  // AmmoDisplayType_e, pgm believes that there are two display types because one is for clip and one is for reserve

	FCLASS_STACKMEM_NOALIGN( CItemInst );
};

//////////////////////////////////////////////////////////////
// a collection of CItemInsts that make up a players inventory
class CInventory
{
public:
	typedef struct {
		BOOL bUsed;
		CItem *pItem;
		CWeapon *pWeapon;
	} WeaponPickupInfo_t;

	CInventory() { m_pfcnCallback = NULL; m_pUser = NULL; }

	BOOL InitItemInst( CItemInst *pItemInst, cchar *pszItemTagName, s32 nClipAmmo, s32 nReserveAmmo );
	BOOL InitFromCSVTable( FGameDataTableHandle_t hTable, BOOL bLoadPrimary = TRUE, BOOL bLoadSecondary = TRUE );
	void InitForMultiplayer( FGameDataTableHandle_t hTable, u32 ePrimary, u32 eSecondary );

	void SetToDefaults(void);	// this function will fill the inventory up with all of Glitch's weapons
	void SetToInitial(void);	// this function will fill the inventory up with only the weapons that Glitch starts the game with
	void SetToUnarmed(BOOL bNoItems=FALSE);    // this function will fill the inventory up with only Glitch's hands.
	void SetToSlosh(void);	// setup inventory representing Slosh
	void SetToKrunk(void);	// setup inventory representing Krunk
	void SetToMozer(void);	// setup inventory representing Mozer

	void CopyInv(CInventory *pSrc, BOOL bChangeOwner=FALSE);

		
	BOOL SetCurWeapon(u32 uWhichSide, u32 uWhichWeapon, BOOL bRestoreSaved, BOOL bNoReloadCallback);
	BOOL SetCurWeapon(u32 uWhichSide, CItemInst *pII, BOOL bRestoreSaved, BOOL bNoReloadCallback);

	BOOL UpgradeWeapon(u32 uWhichSide, u32 uWhichWeapon);
	BOOL UpgradeWeapon(u32 uWhichSide, CItemInst *pII);

	BOOL CycleToNextWeapon(u32 uWhichSide, BOOL bNoReloadCallback);
	BOOL CycleToPrevWeapon(u32 uWhichSide, BOOL bNoReloadCallback);

	// This is here because *somebody* might have changed the EUK of the currently selected weapon behind
	//   our back (you know who you are) and we might need to do some stuff in the inventory to compenstate
	//   for that *if* the 'dualness' changed.
	BOOL SafetyValve(u32 uWhichSideIsCorrect);

	// NKM - So the collectable system can query an inventory to see what is in it.
	CItemInst *IsWeaponInInventory( cchar *pszWeaponName, BOOL bUseTagName = FALSE );
	CItemInst *IsItemInInventory( cchar *pszItemName );

	u32 m_uNumPickupWeapons;
	WeaponPickupInfo_t m_PickupWeaponInfo[ItemInst_uMaxInventoryWeapons];

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// DATA: KEEP FOOTPRINT SMALL, THIS DATA IS SAVED TO MEMORY CARDS - CMemCardInventory MUST BE UPDATED IF DATA IS ADDED
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	s8 m_auNumWeapons[INV_INDEX_COUNT];
	u8 m_uNumItems;

	u8 m_uNumBatteries;

	CItemInst m_aoWeapons[INV_INDEX_COUNT][ItemInst_uMaxInventoryWeapons];	// [prim=0, sec=1][]
	CItemInst m_aoItems[ItemInst_uMaxInventoryItems];

	u16 m_auEUKFlags[ItemInst_uMaxPossibleEUKS];		// If the player currently has N EUKs, then the first N of these are valid.

	u8 m_auCurWeapon[INV_INDEX_COUNT];
	u8 m_auSavedWeapon[INV_INDEX_COUNT];			// Used when a dual weapon is unselected.

	CInventoryCallback_t *m_pfcnCallback;
	void *m_pUser;

private:
	void SetupDefaultItems();
	BOOL SetupItem( u32 nItemIndex, cchar *pszItemTagName, s32 nCount );
	
	FCLASS_STACKMEM_NOALIGN(CInventory);
};

////////////////////////////////////////////////////
// THE FOLLOWING CLASSES MIRROR THE ABOVE CLASSES
// BUT ARE MEM CARD SAFE - MEANING THAT THERE
// ARE NO POINTERS AND A METHOD TO FIGURE OUT
// WHAT ITEM WAS IN WHICH SLOT
////////////////////////////////////////////////////

/////////////////////////////////////////////////
// a small mem card friendly version of CItemInst
class CMemCardItemInst
{
public:
	static u32 GenerateNameCRC( const CItem *pItem );
	static void SaveInventory( CMemCardItemInst *pSave, const CItemInst *pItem );
	static BOOL RestoreInventory( const CMemCardItemInst *pSave, CItemInst *pItem, const CInventory *pInv );
	
	u32 m_nItemNameCRC;// a CRC of the lower case item tag name

	// upgrade level of this item instance.	Needed so mem card restore can reattach correct
	// item to m_pItemData.  One-based index.
	s16 m_nUpgradeLevel;

	// In the case of items, these can also be current and maximum counts respectively.
	s16 m_nClipAmmo;
	s16 m_nReserveAmmo;

	u8 m_aDisplayType[2];
};

//////////////////////////////////////////////////
// a small mem card friendly version of CInventory
class CMemCardInventory
{
public:
	static void SaveInventory( CMemCardInventory *pSave, const CInventory *pInv );
	static BOOL RestoreInventory( const CMemCardInventory *pSave, CInventory *pInv );

	s8 m_anNumWeapons[INV_INDEX_COUNT];
	u8 m_nNumItems;
	u8 m_nNumBatteries;
	CMemCardItemInst m_aWeapons[INV_INDEX_COUNT][ItemInst_uMaxInventoryWeapons];	// [prim=0, sec=1][]
	CMemCardItemInst m_aItems[ItemInst_uMaxInventoryItems];
	u16 m_anEUKFlags[ItemInst_uMaxPossibleEUKS];		// If the player currently has N EUKs, then the first N of these are valid.
	u8 m_anCurWeapon[INV_INDEX_COUNT];
	u8 m_anSavedWeapon[INV_INDEX_COUNT];			// Used when a dual weapon is unselected.	
};


#endif
