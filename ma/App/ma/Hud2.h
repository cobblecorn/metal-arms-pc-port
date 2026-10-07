//////////////////////////////////////////////////////////////////////////////////////
// Hud2.h - (New) HUD Class for Mettle Arms.
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
// 02/05/02 Link		Created.
//////////////////////////////////////////////////////////////////////////////////////
#ifndef _HUD2_H_
#define _HUD2_H_ 1

#include "fang.h"
#include "fmath.h"
#include "fdraw.h"
#include "ftext.h"
#include "ftex.h"
#include "fcolor.h"
#include "HudWeaponProfile.h"


const u32 uMaxRadarEntities = 20;


class CWeapon;
class CFMeshInst;
class CInventory;
class CItemInst;
class CBot;
class CItem;
class CCollectableType;
class CFWorldMesh;
class CFWorldTracker;
class CFAudioEmitter;
class CMeshEntity;
struct FViewport_t;
struct FMesh_t;
struct FVisVolume_t;

enum ItemType_e
{
	ITEMTYPE_ENERGY,
	ITEMTYPE_BATTERY,
	ITEMTYPE_WASHER,
	ITEMTYPE_POWERUP,
	ITEMTYPE_CORINGCHARGE,
	ITEMTYPE_CHIP,
};

enum ItemState_e
{
	ITEMSTATE_EMPTY,
	ITEMSTATE_JUMPINGUP,
	ITEMSTATE_SCROLLINGOFF
};

enum WeaponSelectState_e
{
	WEAPONSELECTSTATE_IDLE,
	WEAPONSELECTSTATE_SCROLLINGON,
	WEAPONSELECTSTATE_PAUSED,
	WEAPONSELECTSTATE_SCROLLINGTO,
	WEAPONSELECTSTATE_SCROLLINGOFF,
};

enum BatteryPickupState_e
{
	BPSTATE_IDLE,
	BPSTATE_STARTING,
	BPSTATE_MOVING,
	BPSTATE_PAUSING,
	BPSTATE_ENDING
};

enum QuickSelectState_e
{
	QSSTATE_WAITINGFORRELEASE,
	QSSTATE_WAITINGFORCLEANSTART,
	QSSTATE_CLEANSTARTED,
	QSSTATE_FINISHINGARROWDISPLAY,
	QSSTATE_ARROWFLASHING
};

enum WasherState_e
{
	WASHERSTATE_OFF,
	WASHERSTATE_SLIDINGON,
	WASHERSTATE_ON,
	WASHERSTATE_TIMEDON,
	WASHERSTATE_CLINKING,
	WASHERSTATE_SLIDINGOFF
};

class CWasherDisplay
{
public:
	CWasherDisplay();
	~CWasherDisplay();

	BOOL Create();
	void Destroy();
	void Draw3D();
	void Draw2D();
	void SetUL(CFVec3 *vecUL, FViewport_t* pRefVP);

	f32 m_fThetaY;
	FViewport_t *m_pView;
	CFMeshInst *m_pMeshInst;
	FMesh_t *m_pMesh;
	FTextAreaHandle_t m_hText;
	f32 m_fTimer, m_fTimeTilOff;
	WasherState_e m_eState;
	u32 m_nWasherCountToDraw;// this could be different to what the player actually has so that we can animate the number down/up
	f32 m_fWasherCount;

	FCLASS_STACKMEM_NOALIGN(CWasherDisplay);
};

class CHudItemInst
{
public:
	CHudItemInst();
	~CHudItemInst();
	BOOL InitFromMI(CCollectableType *pCollectableType);
	void CalcMtx(f32 fThetaY, CFVec3 *vecPos_CS);
	void Draw();
	void Empty();									// Make no item in this slot.
	void Clear();									// Clear it as if it was never initted.

	static s32 m_nActiveCount;

	CCollectableType *m_pCollectableType;
	CFWorldMesh *m_poItem;
	CFVec3 m_vecStartPos_CS, m_vecEndPos_CS;
	f32 m_fScale;
	f32 m_fConstant;
	f32 m_fTimeCounter, m_fTotalTime;
	ItemState_e m_eItemState;
	ItemType_e m_eItemType;

	FCLASS_STACKMEM_NOALIGN(CHudItemInst);
};

class CHudWSItem
{
public:
	CHudWSItem();
	~CHudWSItem();

	void Init();
	void UpdateTextArea(u32 uSide);		// 0 = left, 1 = right
//	void Draw(BOOL bIsSelected, u32 uSide, BOOL bDrawHighlight);
	void Draw(BOOL bIsSelected, u32 uSide, BOOL bDrawHighlight, BOOL bOverride=FALSE, CItem *pItem=NULL, BOOL bReverse=FALSE, CHudWSItem *pOppositeItem=NULL );

	static BOOL ShouldWeDisplayScopeUnusableInfo( CHudWSItem *pWSItem, CHudWSItem *pOppositeWSItem );


	CItemInst *m_pItemInst;
	CFVec3 m_vecCurUL, m_vecStartUL, m_vecEndUL;
	FTextAreaHandle_t m_ahAmmoText[2];
	static CFTexInst m_tex;

	FCLASS_STACKMEM_NOALIGN(CHudWSItem);
};

FCLASS_ALIGN_PREFIX class CHudRadarEntity
{
public:
	CHudRadarEntity();
	~CHudRadarEntity();

	void CalcBSPos(CBot *pPlayer);

	CFVec3A m_vecPos_WS;	// This means in world space.
	CFVec3A m_vecPos_BS;	// This means in Blink space.
	CFWorldMesh *m_pWM;
	f32 m_fStationaryTime;
	BOOL8 m_bNewThisFrame;
	BOOL8 m_bIsActive;
	BOOL8 m_bFriend;		// is this bot my friend?

	FCLASS_STACKMEM_ALIGN(CHudRadarEntity);
} FCLASS_ALIGN_SUFFIX;

class CHudRadarBlip
{
public:
	CHudRadarBlip();
	~CHudRadarBlip();

	void Set(CFVec3 *vecPos, CFColorRGB *oColor, f32 fLifetime, f32 fSpeed, f32 fInitSize);
	void Work();
	BOOL IsActive(void) const {return m_bIsActive;}
	void AddRBToVtxList(FDrawVtx_t *avtxList, u32 &uCurVtx);
private:
	BOOL m_bIsActive;
	CFVec3 m_vecPos;
	CFColorRGB m_rgbColor;
	f32 m_fTimer, m_fLifetime, m_fOOLifetime, m_fSpeed, m_fInitSize;

	friend class CHud2;

	FCLASS_STACKMEM_NOALIGN(CHudRadarBlip);
};
/*
enum
{
	HUDDRAWFLAG_HEALTHBAR		=	1 << 0,
	HUDDRAWFLAG_ITEMPICKUP		=	1 << 1
};*/

enum HudMode_e
{
	HUDMODE_GLITCH,
	HUDMODE_SLOSH,
	HUDMODE_KRUNK,
	HUDMODE_MOZER,
	HUDMODE_MIL
};

enum BatteryPickupStateN_e
{
	BPSTATEN_IDLE,
	BPSTATEN_PAUSING,
	BPSTATEN_FLYING,
	BPSTATEN_STABILIZING
};

enum EnergyBarState_e
{
	EBSTATE_IDLE,
	EBSTATE_GAINING
};

class CFScalarObj;

class CHudBatteryBarSystem
{
public:
	// Start/Stop for static elements once per execution
	static BOOL InitSystem();
	static void Shutdown();

	// Start/Stop for static elements, once per level
	static void InitLevel();
	static void EndLevel();

	// Start/Stop for each instance, once per level
	BOOL		Create();
	void		Destroy();

	void SetHudMode(HudMode_e eNewHudMode);
	void StartBatteryPickup();
	void Work(CInventory *pInventory, f32 fBotHealth);
//	void Draw(CInventory *pInventory);
	void Draw( u32 uNumBatteries, f32 *pfVal=NULL, const CFColorRGBA *pStartColor=NULL, const CFColorRGBA *pEndColor=NULL );
	void DrawMeshes(CInventory *pInventory);
private:
	static BOOL				ms_bSystemInitted;
	static CFTexInst		ms_atexParticle[2];
	static CFTexInst		ms_texBatteryNormal;
	static FDrawVtx_t		ms_aavtxBatteryBar[2][24];
	static CFMeshInst		*ms_pmeshBP;

	f32						m_fXOffsetPos, m_fXOffsetVel;
	f32						m_fLastHealth;
	f32						m_fEnergyGainTimer;
	f32						m_afBPTimer[2];				// by stage.
	f32						m_fBPThetaY, m_fBPThetaZ;
	f32						m_fBPFlyDurFactor;
	CFVec3					m_vecLastBPPos;
	CFColorRGBA				m_rgbaEnergyBarCur, m_rgbaEnergyBarNormal;
	HudMode_e				m_eCurHudMode;
	EnergyBarState_e		m_eEBState;
	BatteryPickupStateN_e	m_eBPState;
	BOOL					m_bFirstFrame;
};



class CHud2
{
public:
	enum DrawFlags_e {
		DRAW_SELFDESTRUCT		= 0x00000001,
		DRAW_BATTERIES			= 0x00000002,
		DRAW_RADAR				= 0x00000004,
		DRAW_QUICKSELECT		= 0x00000008,
		DRAW_WEAPONSELECT		= 0x00000010,
		DRAW_ENABLEOVERRIDE		= 0x00000020,
		DRAW_COLALLOY_MSG		= 0x00000040,
		DRAW_BATTERY_MESHES		= 0x00000080,
		DRAW_WASHERS			= 0x00000100,
		DRAW_ITEMS				= 0x00000200,
		DRAW_LEFT_AMMOBOX		= 0x00000400,
		DRAW_RIGHT_AMMOBOX		= 0x00000800,
		DRAW_LEFT_WEAPONBOX		= 0x00001000,
		DRAW_RIGHT_WEAPONBOX	= 0x00002000,
		DRAW_ICON_TIMER			= 0x00004000,


		DRAW_AMMOBOXES			= DRAW_LEFT_AMMOBOX |
								  DRAW_RIGHT_AMMOBOX,

		DRAW_WEAPONBOXES		= DRAW_LEFT_WEAPONBOX |
								  DRAW_RIGHT_WEAPONBOX,

		DRAW_WEAPONSYSTEM		= DRAW_WEAPONBOXES |
								  DRAW_WEAPONSELECT,

		DRAW_GLITCH_HUD			= DRAW_BATTERIES | DRAW_RADAR | DRAW_QUICKSELECT | 
								  DRAW_WEAPONSYSTEM | DRAW_AMMOBOXES | DRAW_COLALLOY_MSG | 
								  DRAW_BATTERY_MESHES | DRAW_ITEMS | DRAW_WASHERS,

		DRAW_SLOSH_HUD			= DRAW_BATTERIES | DRAW_RADAR | DRAW_QUICKSELECT | 
								  DRAW_WEAPONSYSTEM | DRAW_AMMOBOXES | DRAW_COLALLOY_MSG | 
								  DRAW_BATTERY_MESHES | DRAW_ITEMS | DRAW_WASHERS,
	  
		DRAW_KRUNK_HUD			= DRAW_BATTERIES | DRAW_RADAR | DRAW_QUICKSELECT | 
								  DRAW_WEAPONSYSTEM | DRAW_AMMOBOXES | DRAW_COLALLOY_MSG | 
								  DRAW_BATTERY_MESHES | DRAW_ITEMS | DRAW_WASHERS,

		DRAW_MOZER_HUD			= DRAW_BATTERIES | DRAW_RADAR | DRAW_QUICKSELECT | 
								  DRAW_RIGHT_WEAPONBOX | DRAW_RIGHT_AMMOBOX | DRAW_COLALLOY_MSG | 
								  DRAW_BATTERY_MESHES | DRAW_ITEMS | DRAW_WASHERS,

		DRAW_POSESSED_MIL		= DRAW_BATTERIES | DRAW_RADAR | DRAW_SELFDESTRUCT | DRAW_AMMOBOXES | DRAW_WEAPONBOXES
	};


	typedef enum {
		TRANSMISSION_AUTHOR_COLONEL_ALLOY,
		TRANSMISSION_AUTHOR_AGENT_SHHH,
		TRANSMISSION_AUTHOR_KRUNK,
		TRANSMISSION_AUTHOR_AGENT_GOFF,
		TRANSMISSION_AUTHOR_DR_EXAVOLT,

		TRANSMISSION_AUTHOR_COUNT
	} TransmissionAuthor_e;


	typedef enum AmmoBox_Override_e {
		OVERRIDE_NONE,					// just use inventory/health
		OVERRIDE_METER,
		OVERRIDE_COUNTER,
		OVERRIDE_METER_MIL,
		OVERRIDE_COUNTER_MIL,

		OVERRIDE_MODE_COUNT
	};


	typedef enum {
		RIGHT	= 0,
		LEFT	= 1
	};

	// types of icon timers
	typedef enum {
		ICON_TIMER_TYPE_DETPACK = 0,
		ICON_TIMER_TYPE_RACE,
		NUM_ICON_TIMERS,
	}IconTimerType_t;

	// data for each type of icon timer
	typedef struct {
		FDrawVtx_t		m_aVertices[4];
		CFVec2			m_vTextPos;
		f32*			m_pfDrawFloat;
	} IconTimerData_t;

	//-------------------------------------------------------------------------
	// Constructor/Destructor
	CHud2();
	~CHud2();

	// Initialize this HUD instance. This function should be called after the
	// owning player has initialized, once per level.
	BOOL			Create(s32 nPlayerIndex);

	// Destroy any instance data in this HUD. Called once per level when the
	// level is exiting.
	void			Destroy(s32 nPlayerIndex);

	// These static functions are hooks for the generic hud system; they are
	// not used for individual hud instances.
	static BOOL		InitSystem();
	static void		UninitSystem();
	static BOOL		InitLevel();
	static void		EndLevel();
	static CHud2*	GetCurrentHud(void);
	static CHud2*	GetHudForPlayer(s32 nPlayerIndex);
	FINLINE static void EnableAudioPause(BOOL bEnable) { m_bAudioPause = bEnable; }

	FINLINE void EnableRadar( BOOL bEnable ) { m_bNoRadar = !bEnable; }


	// Use this to turn on/off individual Hud elements.  This value is looked at *only if* draw for the Hud
	//   is turned on (see SetDrawEnabled()).  See HUDDRAWFLAG_* for details on the flags.
	FINLINE u32	SetDrawFlags( u32 uNewDrawFlags )	{ u32 uTmp = m_uDrawFlags; m_uDrawFlags = uNewDrawFlags; return uTmp; };
	FINLINE u32	GetDrawFlags( void )				{ return m_uDrawFlags; };
	FINLINE void EnableDrawFlags( u32 uFlags ) { m_uDrawEnableFlags |= uFlags; }
	FINLINE void DisableDrawFlags( u32 uFlags ) {m_uDrawEnableFlags &= ~uFlags; }
	FINLINE u32 DrawFlagsEnabled( u32 uFlags ) const {return m_uDrawFlags & m_uDrawEnableFlags & uFlags;}

	// Like above, but used to set and clear just one flag without affecting the others
	FINLINE void AddDrawFlags( u32 uFlags ) {m_uDrawFlags |= uFlags;}
	FINLINE void ClearDrawFlags( u32 uFlags ) {m_uDrawFlags &= ~uFlags;}

	// Use this to override the data that the ammo slider / counter displays.  Note that Drawflags must include DRAW_ENABLEOVERRIDE
	void	OverrideAmmoData( u32 uSide, u32 uOverrideMode, f32 *pfData, BOOL bOverrideColor, CFColorRGBA *pStartColor=NULL,
																								  CFColorRGBA *pEndColor=NULL    );
	
	// Use this to override the health bar.  *pfData should range from 0.0 to 1.0 to fill the first battery, 2.0 for the second, etc,
	// ranges from startcolor at 0, endcolor at the end of the last battery
	void OverrideHealthBar( BOOL bOverride, f32 *pfData, u32 uNumBatteries, const CFColorRGBA &startColor, const CFColorRGBA &endColor );

	//Use this to override the weapons displayed in the boxes.  Either look it up by name, or just feed in the ST values
	//can get those by grabbing an item pointer from CItemRepository
	BOOL OverrideWeaponBox( BOOL bOverride, u32 uSide, cchar *pName, BOOL bReverse=FALSE );
	BOOL OverrideWeaponBox( BOOL bOverride, u32 uSide, CItem *pItem, BOOL bReverse=FALSE );

	BOOL SetIconTimerDraw( IconTimerType_t nTimer, BOOL bDraw, f32* pTimer=NULL );
	void ClearIconTimerDrawIfOwned( IconTimerType_t nTimer, const f32 *pTimer );

	//void SetPlayerBot(CBot *poPlayerBot);
	void SetDrawEnabled(BOOL bDrawEnabled);			// Note that 
	BOOL IsDrawEnabled();
	void SetWSEnable(BOOL bEnabled) { m_bWSEnabled = bEnabled; }
	FINLINE BOOL IsWSEnabled( void ) const { return m_bWSEnabled; }

	void SetHudMode(HudMode_e eHudMode);
	void SetRedTint(BOOL bRedTint);

	f32 GetUnitSelfDestruct();
	
	static CFTexInst* GetTexHud(void){ return &_texHud;}
	static CFTexInst* GetTexControls(void){ return &_texControls;}

	BOOL TransmissionMsg_Start( TransmissionAuthor_e nAuthorIndex, FSndFx_FxHandle_t hSndFx, f32 fUnitVolume, BOOL bAbortWithCutScene );
	BOOL TransmissionMsg_Start( TransmissionAuthor_e nAuthorIndex, cchar *pszStreamingFileName, f32 fUnitVolume, BOOL bAbortWithCutScene );
	static BOOL TransmissionShared_Start( TransmissionAuthor_e nAuthorIndex, FSndFx_FxHandle_t hSndFx, f32 fUnitVolume, BOOL bAbortWithCutScene );
	static BOOL TransmissionShared_Start( TransmissionAuthor_e nAuthorIndex, cchar *pszFile, f32 fUnitVolume, BOOL bAbortWithCutScene );
	static void TransmissionShared_Stop( BOOL bFadeOut );
	void TransmissionMsg_Stop( BOOL bFadeOut=TRUE );
	BOOL TransmissionMsg_IsDonePlaying( void ) const;
	BOOL Transmission_GetAbortWithCutSceneFlag( void ) const;

	BOOL StartWeaponSelect(u32 uWhichSide, CInventory *pInventory, BOOL bDelay);		// 0 = primary, 1 = secondary.
#if defined(MA_PC_INPUT)
	BOOL PcWeaponSelectTap(u32 uWhichSide, CInventory *pInventory, BOOL bCycle);
#endif
	FINLINE BOOL IsWSActive()
	{
		return((m_eWeaponSelectState == WEAPONSELECTSTATE_SCROLLINGON)
			|| (m_eWeaponSelectState == WEAPONSELECTSTATE_SCROLLINGOFF)
			|| (m_eWeaponSelectState == WEAPONSELECTSTATE_SCROLLINGTO) 
			|| (m_eWeaponSelectState == WEAPONSELECTSTATE_PAUSED));
	}

	FINLINE BOOL IsWSPaused( void ) { return (m_eWeaponSelectState == WEAPONSELECTSTATE_PAUSED); }

	void SetWasher(BOOL bVisible);
	void SetWasherTimed(f32 fTimeOn);

	void PickupItemGeneric( CCollectableType *pCollectableType, ItemType_e eItemType, f32 fScale);
	void PickupItemBattery();

	void Work(CInventory *pInventory);
	void Draw(CInventory *pInventory);
	void NoDraw( void );

	HudMode_e m_eCurHudMode;
	WeaponSelectState_e m_eWeaponSelectState;

	s32 m_nWhichWeaponSelectIsActive;			// -1 = none, 0 = primary, 1 = secondary.
	FINLINE f32 GetWSBoxUnitPos() { return(m_fWSBoxBPUnitPos); }

	void CheckpointRestore( void );
	void ResetRadar( void );

	static FINLINE f32 GetAmmoAlertUnitCountdownTimer( void ) { return m_fAmmoAlertUnitCountdownTimer; }
	static cwchar *ComputeAmmoAlertTextColor( CWeapon *pWeapon, u32 *pnDeciRed, u32 *pnDeciGreen, u32 *pnDeciBlue );

	static void GlobalWork( void );

	BOOL TextDisplay_Start( f32 fDisplayTime, CFColorRGBA *pColor, BOOL bBlink, BOOL bPulse, cwchar *pszWFormat, ... );

public:
	// Directional light used for the items rendered in the HUD
	static CFLight 				m_HUDDirLight;

private:
	// Statics
	static BOOL					_bSystemInitialized;
	static BOOL					_bLevelInitialized;
	static FSndFx_FxHandle_t	_hScrollClick;
	static FSndFx_FxHandle_t	_hSelect;
	static CFTexInst			_texInfoBox;
	static CFTexInst			_texHud;
	static CFTexInst			_texControls;
	static f32					m_fAmmoAlertUnitCountdownTimer;		// Counts down to animate the ammo alert text
	static BOOL					m_bAudioPause;

	CHudBatteryBarSystem	m_Battery;
	s32	 m_nPlayerIdx;				// Player index who owns this hud
	u32  m_uDrawFlags;				// used to set what is or isn't drawn
	u32	 m_uDrawEnableFlags;		// A mask for uDrawFlags to keep an item disabled even when turned on
	u32  m_auAmmoOverrideMode[2];		// used to override what's displayed in the ammo boxes
	f32* m_apfAmmoOverrideData[2];		// pointers to override data
	u32	m_auAmmoOverrideStyle[2];		// 0 for glitchy 1 for millish
	BOOL m_abAmmoOverrideColor[2];		// should we set our own color?
	BOOL m_bOverrideHealth;				// are we overriding the health bar?
	f32* m_pfHealthOverrideData;			// where we're getting the overriden health data
	u32	m_uHealthOverrideBatteries;		// How many batteries we want the hud system to draw
	CFColorRGBA m_HealthOverrideStartColor;	//What color to use for 0
	CFColorRGBA m_HealthOverrideEndColor;	//What color to use for 0

	CItem*	m_apOverrideWeaponBoxItem[2];		// the items we should draw there
	BOOL8 m_abOverrideWeaponBox[2];			// should we override the weapon display box(es)?
	BOOL8 m_abOverrideWeaponBoxReverse[2];	// should we reverse (mirror) the item (so we can draw weapons on the left side

	BOOL8 m_bNoRadar;

	CFColorRGBA m_aAmmoOverrideStartColorRGB[2];
	CFColorRGBA m_aAmmoOverrideEndColorRGB[2];

	enum SelfDestructState_e
	{
		SDSTATE_IDLE,
		SDSTATE_MESSAGE,
		SDSTATE_COUNTINGDOWN
	};

	SelfDestructState_e m_eSDState;
	s32 m_nSDCountDown;
	f32 m_fSDTimer;
	FTextAreaHandle_t m_hSDText;
	FTextAreaHandle_t m_hSDTextShadow;
	FTextAreaHandle_t m_ahSDCounter[2];

	FTextAreaHandle_t m_hMessage;

	void SetDestinationsToOffScreen();
	void SetCursToDests();
	void DoWSItemMovement(CHudWSItem *pItem, f32 fWeight1, f32 fWeight2);
	u32 GetDPadDirN();
	s32 GetSingleDir(u32 uInput);
	void LoadProfile(u32 uFirstChance, CInventory *pInventory);
	void SaveProfile(u32 uFirstChance, CInventory *pInventory);
	void CheckControls();
	void AddREToVtxList(FDrawVtx_t *avtxList, u32 &uCurVtx, f32 fSize, CHudRadarEntity *pRE);
	void DrawAmmoBox(u32 uSide, CInventory *pInventory);
	BOOL ProcessPendingScrolls();
	void _AddPendingScroll(s32 nDelPendingScroll);

	//////////////////////////////////////////////////////////////////////
	//
	typedef enum {
		TRANSMISSION_STATE_IDLE,
		TRANSMISSION_STATE_START_ON1,
		TRANSMISSION_STATE_START_OFF1,
		TRANSMISSION_STATE_START_ON2,
		TRANSMISSION_STATE_START_OFF2,
		TRANSMISSION_STATE_ON,
		TRANSMISSION_STATE_FADING_OUT,

		TRANSMISSION_STATE_COUNT
	} TransmissionState_e;

	TransmissionState_e m_nTransmissionState;
	f32 m_fTransmissionsTimer;
	CFAudioEmitter *m_pTransmissionAudioEmitter;
	cwchar *m_pwszTransmissionAuthor;
	CMeshEntity *m_pTransmissionAntennaMeshEntity;
	BOOL m_bTransmissionAbortWithCutScene;
	BOOL m_bTransmissionMirror;

	void _TransmissionMsg_Start( TransmissionAuthor_e nAuthorIndex, BOOL bAbortWithCutScene );
	void _TransmissionWork();
	//
	//////////////////////////////////////////////////////////////////////

	////
	//
	void RadarWork();
	static BOOL RadarCallback(CFWorldTracker *pTracker, FVisVolume_t *pVolume );
	BOOL DoRadarCallback(CFWorldTracker *pTracker, FVisVolume_t *pVolume );
	void RadarBlipWork();
	void RadarBlipDraw();
	void RadarBlipAdd(CFVec3 *vecPos, CFColorRGB *oColor, f32 fLifetime, f32 fSpeed, f32 fInitSize);
	void RadarDraw1();
	void RadarDraw2();
	void IconTimerDraw(void);
	void DrawWSInfoBoxText(CHudWSItem *pItem);
	void DrawWSInfoBoxMesh(CHudWSItem *pItem);
	void WSItemDraw(u32 uSide);
	void WSFlashDraw(u32 uSide);
	void WasherWork(CInventory *pInventory);
	void QSInit2();
	void QSWork2(CInventory *pInventory);
	void QSDraw2();
	void QuickCycleWork(CInventory *pInventory);

	void _PrepareAmmoDisplay( CInventory *pInventory, u32 uSide, FDrawVtx_t *pVtx, u32 *puCurVtx );

	//
	////

	BOOL m_bDrawEnabled, m_bRedTint;
	s32 m_nDrawCounter;

	FDrawVtx_t m_avtxButton[4];

	FViewport_t *m_pviewPerspective[2], *m_pviewOrtho3d;

	//// Related to items 
	//
	CHudItemInst m_aoHudItemInst[3];
	FViewport_t *m_pviewItemPickup;
	//
	////

	//// Related to the weapon selection screen.
	//
	f32 m_fWSTimeCountdown, m_fWSAlphaTimer;
	u32 m_uWSNumVisible, m_auWSCurSelected[2], m_uWSInventorySize;
	s32 m_nWSPendingScroll;
	CHudWSItem m_aWSItem[2][20];
	BOOL m_bFirstFrame, m_bWSEnabled;

	f32 m_fCurFIAlpha, m_fFIAlpha1, m_fFIAlpha2;

	BOOL m_bWSScrollCleared;

	FTextAreaHandle_t m_ahWSName[2], m_ahWSDesc[2], m_ahWSEUK[2], m_ahWSWeapType[2], m_ahWSAmmoAlert[2], m_ahWSUpgradeable[2], m_hScopeUnusableTextArea;
	f32 m_fWSItemTheta;
	BOOL m_bDualWasSelected;
	u32 m_auFlashing[2];
	f32 m_afFlashAlpha[2];
	//
	////

	IconTimerData_t m_aIconTimerData[ NUM_ICON_TIMERS ];	// data describing each type of icon timer
	s32 m_nCurrentIconTimer;								// array index of currently displayed timer

	//// Related to the radar.
	//
	FDrawVtx_t		m_avtxRadar[4];
	CHudRadarBlip	m_aoRadarBlipPool[uMaxRadarEntities];
	CHudRadarEntity	m_aRadarEntity[uMaxRadarEntities];
	//
	////

	//// Related to the WS box.
	//
	f32 m_fWSBoxBPUnitPos;
	f32 m_fWSBoxDestBPUnitPos;
	f32 m_fWSBoxDir;				// E {-1.0f, 0.0f, 1.0f}
	f32 m_fWSBoxCurThetaY;
	void WSBoxSetDest(f32 fDest);
	void WSBoxDrawMesh(CHudWSItem *pItem);
	void WSBoxDrawFDraw(CHudWSItem *pItem, CHudWSItem *pOppositeItem=NULL);
	void WSBoxWork();
	//
	////

	//// Related to quick select/save.
	//
	QuickSelectState_e m_eQSState;
	s32 m_anDir[2];
	f32 m_fQSTimer;
	CHudWeaponProfile m_aQSProfile[4];
	FDrawVtx_t m_aavtxArrow[4][4];

	s32 m_nLastDir, m_nDrawDir;
	u32 m_uLastDState;
	u32 m_uWhichArrow;
	//
	////

	//// 
	//
	CWasherDisplay m_oWasherDisplay;
	//
	////

	u32 m_uButtons, m_uButtonsLatched;

	// This should ultimately get replaced by getting read from the bot structure.
	u32 m_uNumBatteries;

	FTextAreaHandle_t m_ahCurAmmo[2], m_ahMaxAmmo[2];

	//////////////////////////////////////////////////////////////////////
	//
	typedef enum {
		TEXTDISPLAY_STATE_IDLE,
		TEXTDISPLAY_STATE_ON,
		TEXTDISPLAY_STATE_FADING_OUT,

		TEXTDISPLAY_STATE_COUNT
	} TextDisplayState_e;

	TextDisplayState_e m_eTextDisplayState;

	f32 m_fTextDisplayTime;
	f32 m_fTextFlashTime;
	f32 m_fTextPulseAngle;
	wchar *m_pTextBuffer;
	FTextAreaHandle_t m_hTextDisplay;
	CFColorRGBA m_TextColor;

	void _TextDisplayRelease( void );
	void _TextDisplayWork( void );
	//
	//////////////////////////////////////////////////////////////////////
};

// Text printing wrappers. Transform coordinates by current model transform
// before sending to ftext.
extern void Hud2_PrintString( const FTextAreaHandle_t ohArea, cwchar *pszWString );
extern void Hud2_XFormPrintf( const FTextAreaHandle_t ohArea, cwchar *pszWFormat, ... );
extern void Hud2_XFormPrintf( const FTextAreaHandle_t ohArea, cchar *pszFormat, ... );



#endif
