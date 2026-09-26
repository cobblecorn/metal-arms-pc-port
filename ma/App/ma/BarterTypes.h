#ifndef _BARTERTYPES_H
#define _BARTERTYPES_H 1

#include "fang.h"
#include "fdata.h"
#include "BotTalkInst.h"
#include "fgamedata.h"
#include "Collectable.h"
#include "bartersystem.h"

class CBot;
class CMeshEntity;
class CBotGlitch;

static const s32 BARTER_DEFAULT_PRICE	= 0;
static const s32 BARTER_INFINITE		=-1;
static const u32 BARTER_TYPES_NUM_SLOTS = 3;

FCLASS_NOALIGN_PREFIX class CBarterResponse
{
public:
	CBarterResponse();
	~CBarterResponse();
	BOOL Init( cchar *pszFilename, f32 fPercent, cchar *pszKeyword, cchar *pszEventName, f32 fEventChance );
	BOOL Load();

	CTalkInst *m_pInst;// only valid during a level that actually uses this item

	cchar *m_pszBarterFilename;

	u8 m_nShadyState;
	u8 m_nPercentChance;// 0 - 100
	
	u8 m_nEventID;
	u8 m_nPercentCompleteToFireOffEvent;// 0 - 100
	u8 m_bEventFired;
	
	FCLASS_STACKMEM_NOALIGN( CBarterResponse );
} FCLASS_NOALIGN_SUFFIX;




// This contains all of the data that pertains to a particular item, in general.
FCLASS_NOALIGN_PREFIX class CBarterItem
{
public:
	CBarterItem();
	~CBarterItem();

	// INTERFACE:
	static BOOL InitFromFile( cchar *pszCSVFile );
	static void Shutdown();
	static CBarterItem *FindBarterItem( u32 nSlot, cchar *pszItemName );

	static void UninitLevel();

	// Should only be called by InitFromFile
	BOOL Init( cchar *pszItemName,const u32 uWasherPrice,f32 fScale=1.0f,CFVec3A *pOffset=NULL,u32 nNumResponses=0,CBarterResponse *paResponses=NULL );
	
	BOOL LoadSharedResources(void);

	// Asserts that the player has enough washers to buy the item
	void Purchase( CBot *pPlayerBot, u32 nFinalPrice ) const;

	// DATA:
	CFVec3 m_vTableOffset;

	CollectableType_e m_eCollectableType;

	f32 m_fScale;
	u32 m_nBasePrice;
	
	// these are all of the items that can be offered during the game
	static u32 m_aNumItems[BARTER_TYPES_NUM_SLOTS];
	static CBarterItem *m_apaItems[BARTER_TYPES_NUM_SLOTS];

	static u32 m_nNumGenericResponses;
	static CBarterResponse *m_paGenericResponses;
	static u8 *m_pauGenericPlayFlags;
	static u8 *m_pauGenericSortOrder;
	
	u32 m_nNumResponses;
	CBarterResponse *m_paResponses;
	u8 *m_pauPlayFlags;
	u8 *m_pauSortOrder;

private:
	static CBarterResponse *CreateResponseArrayFromTable( FGameDataTableHandle_t hBotTalkTable, u32 &rnNumItems );

	FCLASS_STACKMEM_NOALIGN( CBarterItem );
} FCLASS_NOALIGN_SUFFIX;




// This is an instance of an item appearing in the world.
FCLASS_NOALIGN_PREFIX class CBarterItemInst
{
public:

// BarterItem flags:
	enum 
	{
		BARTERFLAG_ITEM_DEFAULT				= 0x00,	// LSB is 0, means this item is not specifically cased in level.csv
		BARTERFLAG_ITEM_IS_FROM_CSV			= 0x01,	// TRUE if this item was specifically cased in the level.csv
		BARTERFLAG_ITEM_IS_AUTO_OFFERED		= 0x02,	// TRUE if this item should be offered automatically
		BARTERFLAG_ITEM_IS_IN_WORLD			= 0x04,	// TRUE if this item is in the world
		BARTERFLAG_ITEM_IS_SELECTED			= 0x08,	// TRUE if this item is selected
		BARTERFLAG_ITEM_IS_NEEDED			= 0x10,	// TRUE based on need determined by CCollectible.h
		BARTERFLAG_ITEM_IS_AMMO				= 0x20,	// 
		BARTERFLAG_ITEM_USE_BOX_SCALE		= 0x40,	// 
	};

	CBarterItemInst();
	~CBarterItemInst();

	// INTERFACE:
	FINLINE BOOL IsLevelSpecified(void) {return (!!(m_uBarterItemFlags & BARTERFLAG_ITEM_IS_FROM_CSV));}
	FINLINE BOOL IsAutoOffered(void)	{return (!!(m_uBarterItemFlags & BARTERFLAG_ITEM_IS_AUTO_OFFERED));}
	FINLINE BOOL IsInWorld(void)		{return (!!(m_uBarterItemFlags & BARTERFLAG_ITEM_IS_IN_WORLD));}
	FINLINE BOOL IsSelected(void)		{return (!!(m_uBarterItemFlags & BARTERFLAG_ITEM_IS_SELECTED));}
	FINLINE BOOL IsNeeded(void)			{return (!!(m_uBarterItemFlags & BARTERFLAG_ITEM_IS_NEEDED));}
	FINLINE BOOL IsAmmo(void)			{return (!!(m_uBarterItemFlags & BARTERFLAG_ITEM_IS_AMMO));}
	FINLINE BOOL IsABox(void)			{return (!!(m_uBarterItemFlags & BARTERFLAG_ITEM_USE_BOX_SCALE));}
	

	FINLINE BOOL IsAvailable(void)		{return ( (m_nBarterItemQuantity == BARTER_INFINITE)/*infinite supply*/ || (m_nBarterItemQuantity > 0)/*finite supply*/);}


	FINLINE void SetLevelSpecified(BOOL bIsIt)  { ( bIsIt ? (m_uBarterItemFlags |= BARTERFLAG_ITEM_IS_FROM_CSV)  : (m_uBarterItemFlags &= (~BARTERFLAG_ITEM_IS_FROM_CSV)) );}
	FINLINE void SetAutoOffered(BOOL bIsIt)		{ ( bIsIt ? (m_uBarterItemFlags |= BARTERFLAG_ITEM_IS_AUTO_OFFERED)  : (m_uBarterItemFlags &= (~BARTERFLAG_ITEM_IS_AUTO_OFFERED)) );}
	FINLINE void SetInWorld(BOOL bIsIt)			{ ( bIsIt ? (m_uBarterItemFlags |= BARTERFLAG_ITEM_IS_IN_WORLD)  : (m_uBarterItemFlags &= (~BARTERFLAG_ITEM_IS_IN_WORLD)) );}
	FINLINE void SetSelected(BOOL bIsIt)		{ ( bIsIt ? (m_uBarterItemFlags |= BARTERFLAG_ITEM_IS_SELECTED)  :  (m_uBarterItemFlags &= (~BARTERFLAG_ITEM_IS_SELECTED)) );}
	FINLINE void SetNeeded(BOOL bIsIt)			{ ( bIsIt ? (m_uBarterItemFlags |= BARTERFLAG_ITEM_IS_NEEDED)    : (m_uBarterItemFlags &= (~BARTERFLAG_ITEM_IS_NEEDED)) );}
	FINLINE void SetAmmo(BOOL bIsIt)			{ ( bIsIt ? (m_uBarterItemFlags |= BARTERFLAG_ITEM_IS_AMMO)      : (m_uBarterItemFlags &= (~BARTERFLAG_ITEM_IS_AMMO)) );}
	FINLINE void SetABox(BOOL bIsIt)		    { ( bIsIt ? (m_uBarterItemFlags |= BARTERFLAG_ITEM_USE_BOX_SCALE): (m_uBarterItemFlags &= (~BARTERFLAG_ITEM_USE_BOX_SCALE)) );}

	FINLINE s8	 GetQuantity(void)				{ return m_nBarterItemQuantity;}
	FINLINE void SetQuantity(s32 nHowMany)		{ FASSERT( ((s32)(s8)nHowMany) == nHowMany); // size check
												  m_nBarterItemQuantity = (s8)nHowMany;}

	FINLINE BOOL ShouldBeOffered(void)	
	{ 
		if (IsNeeded())
		{
			if (IsAutoOffered())
				return TRUE;

			if (IsLevelSpecified() && IsAvailable() )
				return TRUE;
		}
		return FALSE;
	}


	FINLINE void SetPrice (u16 uNewPrice)		{ m_nWasherPrice = uNewPrice; }
	FINLINE u16  GetPrice (void)				{ return m_nWasherPrice;}

	BOOL Init( u32 nSlot, cchar *pszItemName, BOOL bLevelSpecified, s32 nHowMany, s32 nPrice);
	BOOL Init( CBarterItem* pBarterItem, BOOL bLevelSpecified, s32 nHowMany, s32 nPrice);

	void SetMtx( CFMtx43A &mtxNew );
	void AddToWorld();
	void RemoveFromWorld();
	
	void Work();

	BOOL CanPurchase( CBot *pPlayerBot ) const;
	void Purchase( CBot *pPlayerBot );
	

// DATA:
	CFMtx43 m_mtxToWorld;			// This is saved off so that, during the work function, we can start from this
									//   xfm and rotate it, rather than doing a cumulative rotation (which suffers precision errors).
	CBarterItem *m_poItem;

	CCollectableType* m_pCollectableType;
	CFWorldMesh* m_pWorldMesh;

	f32 m_fAngle;
	f32 m_fTimer;

private:	
	u16 m_nWasherPrice;		// How much is this item instance, which may be more or less than the base item price
	s8 m_nBarterItemQuantity;
	u8 m_uBarterItemFlags;

	FCLASS_STACKMEM_NOALIGN( CBarterItemInst );
} FCLASS_NOALIGN_SUFFIX;




FCLASS_NOALIGN_PREFIX class CBarterSaleChains 
{
public:
	CBarterSaleChains()		{ m_nNumItems = 0; m_paItemInsts = NULL; m_nItemIndex = 0; }
	u32 m_nNumItems;		// how many items does this Sale Chain have
	CBarterItemInst *m_paItemInsts;
	u32 m_nItemIndex;		// what is the current Item Index

	FCLASS_STACKMEM_NOALIGN( CBarterSaleChains );
} FCLASS_NOALIGN_SUFFIX;



#define BARTER_SLOT_UNUSED_SLOT		-1

FCLASS_NOALIGN_PREFIX class CBarterSlot 
{
public:
	CBarterSlot()			{ m_nSaleChains = 0; m_nSpecSaleChains = 0; m_paSaleChains = NULL; m_nChainIndex = BARTER_SLOT_UNUSED_SLOT; }

	void CheckPointSave(void);
	void CheckPointRestore(void);

	u32 m_nSaleChains;			// how many Sale Chains are allocated
	u32 m_nSpecSaleChains;		// how many Sale Chains are predetermined 
	CBarterSaleChains *m_paSaleChains;
	
	void SetChainIndex( s32 nChainIndex,BOOL bSearchUp=TRUE );
	FINLINE s32 GetChainIndex( void )
	{
		return m_nChainIndex;
	}
private:
	s32 m_nChainIndex;		// what is the current SaleChain Index (BARTER_SLOT_UNUSED_SLOT means that this slot is either perm empty or temp empty)
	
	FCLASS_STACKMEM_NOALIGN( CBarterSlot );
} FCLASS_NOALIGN_SUFFIX;




FCLASS_NOALIGN_PREFIX class CBarterTable 
{
public:
	CBarterTable()			{ m_pszName = NULL; m_nCurrentSlot = 0;m_fMusicRadius = BARTER_SYSTEM_ATTRACT_RADIUS; m_fSpeechRadius = BARTER_SYSTEM_SPEAK_RADIUS;}
	void CheckPointSave(void);
	void CheckPointRestore(void);

	cchar *m_pszName;		// the name of the barter droid point entity
	CBarterSlot m_aSlots[BARTER_TYPES_NUM_SLOTS];	// each slot of the table will offer differ Sale Chains
	u32 m_nCurrentSlot;		// what is the current Slot Index
	f32 m_fMusicRadius;
	f32 m_fSpeechRadius;

	FCLASS_STACKMEM_NOALIGN( CBarterTable );
} FCLASS_NOALIGN_SUFFIX;




// This contains all of the data for all of the possible barter tables in the current level
//	The data is organized as follows:
//		Table[]
//			Each Table has a Slot[]
//				Each Slot has a SaleChain[]
//					Each SaleChain has a ItemInst[]
//
// NOTE: SLOT INDICES ARE NEVER REMAPPED AT THIS LEVEL, REGARDLESS OF UPSALE MODE OR NOT!!!!
FCLASS_NOALIGN_PREFIX class CBarterLevel
{
public:
	// INTERFACE:
	static BOOL InitLevel( cchar *pszCSVFile );
	static void UninitLevel();

	static void Work();
	
	static u32 GetNumTables();
	static CBarterTable *GetTable( u32 nIndex );
	static CBarterTable *GetTable( cchar *pszName );

	static void SetActiveTable( CBarterTable *pActive );
	static CBarterTable *GetActiveTable(void);

	static void SelectStartingItems( CBot *pPlayerBot );
	static void SetCurrentSlot( u32 nSlotIndex );
	static s32  SetStartingSlotIndex(void); 
	static u32  GetCurrentSlot(void);

	static void SetCurrentSalesChain( s32 nSalesChainIndex );
	static u32  GetCurrentSalesChainIndex(void);
	
	static BOOL PurchaseCurrentItem( CBot *pPlayerBot );
	static BOOL SelectNewItems( CBot *pPlayerBot, BOOL bPlayerJustBoughtCurrentItem );
	static BOOL IsInUpSaleMode(void);
	static u32 HowDeepIsCurrentItemInSaleChain(void);
	
	static CBarterItemInst *GetCurrentItemInst(void);
	static CBarterItemInst *GetSlotItemInst( u32 nSlotIndex );
	static u32			    GetSlotItemCount( u32 nSlotIndex );

	static FTextArea_t		 m_oTextArea;
	static FTextAreaHandle_t m_hTextDisplay;
	static CFWorldMesh*		 m_pAmmoBoxMesh;

private:
	static void _UpdateItemsForSaleBasedOn (CBotGlitch* pPlayerBot);
	// DATA:
	static CBarterTable *m_pCurrentTable;
	static u32 m_nNumTables;
	static CBarterTable *m_paTables;
	FCLASS_STACKMEM_NOALIGN( CBarterLevel );
} FCLASS_NOALIGN_SUFFIX;




// used to pass data to slim, shady, and anyone else who needs
// to know about the current state of the table
typedef struct {
	CBarterItemInst *apSlots[BARTER_TYPES_NUM_SLOTS];
	u32 nCurrentSlot;
	BOOL bTableIsAnUpsale;
} BarterTypes_TableData_t;


extern void BarterTypes_ScrambleSortList(u8* paSortArray, CBarterResponse* paResponses, u32 nNumResponses, u32 uShadyState, CBarterResponse *pPleaseNotFirst);

#endif
