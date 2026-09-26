//////////////////////////////////////////////////////////////////////////////////////
// econsole.h - Remote control console entity object.
//
// Author: Steve Ranck     
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
// 04/17/02 Ranck       Created.
//////////////////////////////////////////////////////////////////////////////////////

#ifndef _ECONSOLE_H_
#define _ECONSOLE_H_ 1

#include "fang.h"
#include "entity.h"
#include "meshentity.h"
#include "ebox.h"


class CEConsole;
class CBot;
class CBotDispenser;
class CAlarmNet;
class CEShield;
class CBotGlitch;

//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CConsoleSlot
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

FCLASS_ALIGN_PREFIX class CConsoleSlot {
public:

	CMeshEntity m_MEChip;					// Chip mesh entity
	CFMtx43A m_SocketMtx;					// The matrix of the socket
	u32 m_nUserPropIndex;					// Chip index as set by the Max user property field (suffix on name)


//	friend CEConsole;


	FCLASS_STACKMEM_ALIGN( CConsoleSlot );
} FCLASS_ALIGN_SUFFIX;





//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CEConsole
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

FCLASS_ALIGN_PREFIX class CEConsole : public CEntity {
//----------------------------------------------------------------------------------------------------------------------------------
// Public Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	enum {
		MAX_CHIPS = 5,
	};


//----------------------------------------------------------------------------------------------------------------------------------
// Protected Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
protected:

	ENTITY_CLASS_HIERARCHY_BITDEF




//----------------------------------------------------------------------------------------------------------------------------------
// Private Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
private:

	enum {
		_CONSOLE_FLAG_WHITESAT_VIEW		= 0x00000001,		// TRUE when we're white-saturating the view
		_CONSOLE_FLAG_DRAWING_MIL_LOGO	= 0x00000002,		// TRUE when we're drawing the Mil logo
		_CONSOLE_FLAG_MONITOR_ON		= 0x00000004,		// TRUE when the console monitor is lit up
		_CONSOLE_FLAG_PLAYED_SHIELD		= 0x00000008,		// TRUE if shield sound was played during activation
		_CONSOLE_FLAG_USE_ONCE			= 0x00000010,		// TRUE if console should only be used once
		_CONSOLE_FLAG_USED_IT_ONCE		= 0x00000020,		// TRUE if console was used once and is now unusable

		_CONSOLE_FLAG_NONE				= 0x00000000
	};


	typedef enum {
		CONSOLE_STATE_IDLE,
		CONSOLE_STATE_CHIP_FLYING,
		CONSOLE_STATE_CHIP_INSERTING,
		CONSOLE_STATE_START_ACTIVATION,
		CONSOLE_STATE_ACTIVATING,
		CONSOLE_STATE_START_POSSESSION,
		CONSOLE_STATE_FADE_OUT_WITH_LOGO,
		CONSOLE_STATE_FADE_IN_BEHIND_BOT,
		CONSOLE_STATE_CONTROLLING_BOT,
		CONSOLE_STATE_CHECKPOINT_RESTORE,

		CONSOLE_STATE_COUNT
	} ConsoleState_e;

	typedef enum {
		LIGHT_COLOR_RED,		
		LIGHT_COLOR_GREEN,		
		LIGHT_COLOR_BLACK,		
	} LightColor_e;

	typedef struct {
		CFSoundGroup *pSoundGroupAttractLoop;	// loop of console machinery
		CFSoundGroup *pSoundGroupShieldUp;		// loop of console shield powering up
		CFSoundGroup *pSoundGroupFlyingChip;	// chip flying to insertion point
		CFSoundGroup *pSoundGroupInsertingChip;	// chip inserting into socket
		CFSoundGroup *pSoundGroupActivating;	// console activating
		CFSoundGroup *pSoundGroupDeactivating;	// console deactivating
		CFSoundGroup *pSoundGroupPossessing;	// taking control of console bot

	} BotInfo_Console_t;

//----------------------------------------------------------------------------------------------------------------------------------
// Public Data:
//----------------------------------------------------------------------------------------------------------------------------------
public:
	static FLinkRoot_t m_ConsoleRoot;			// Linked list of consoles
	FLink_t m_ConsoleLink;						// Link to consoles in m_ConsoleRoot

//----------------------------------------------------------------------------------------------------------------------------------
// Private Data:
//----------------------------------------------------------------------------------------------------------------------------------
private:
	static const FGameData_TableEntry_t m_aBotInfoVocab_Console[];
	static BotInfo_Console_t m_BotInfo_Console;
	static u32 m_nConsoleClassClientCount;		// Number of consoles of this class using the class-wide resources
	static const FGameDataMap_t m_aGameDataMap[];// Vocabulary map used to parse through user props

	static BOOL m_bSystemInitialized;			// TRUE = this system has been initialized
	static FMesh_t *m_pErrorMesh;				// Mesh displayed if a specified mesh could not be found (NULL = no error mesh)
	static CFTexInst m_MilLogoTexInst;			// The texture for the Mil logo

	CBotGlitch *m_pGlitchBot;					// The bot who is inserting the chips
	CBot *m_pControlBot;						// The bot this console controls
	CBotDispenser *m_pBotDispenser;				// The dispenser this bot may be dispensed from upon possession
	CAlarmNet *m_pBotDispenserNet;				// The alarm net this dispenser belongs to

	u32 m_nConsoleFlags;						// See _CONSOLE_FLAG_* for info
	f32 m_fChipScale;							// The scale to use for placing the chip into the socket
	cchar *m_pszBaseSocketName;					// Base socket name prefix (NULL=none)

	CMeshEntity *m_pMEConsole;					// Remote control console entity (NULL=none)
	CMeshEntity *m_pMEJoystick;					// Joystick mesh entity (NULL=none)
	CEBox *m_pOperatorBox;						// Operator box entity

	u32 m_nSocketCount;							// Number of sockets (0=none)
	CConsoleSlot *m_pSlotArray;					// Array of slot data

	u32 m_nDrawChipCount;						// The number of chips that are being drawn
	u32 m_nInsertedChipCount;					// The number of chips that are 100% inserted
	f32 m_fUnitInserting;						// The unit distance above the socket for the chip currently being inserted

	ConsoleState_e m_nConsoleState;				// current state of Console state machine
	ConsoleState_e m_nRestoreState;				// stores console state during checkpoint restore

	CFQuatA m_EndQuat_WS;
	CFVec3A m_EndPos_WS;
	f32 m_fFlyinAngle;

	f32 m_fUnitCamAnim;							// Used to animate the camera
	f32 m_fStartHalfFOV;						// The half-FOV of the camera when we take control
	f32 m_fUnitDrawMilLogo;						// How prevelent the Mil logo is
	f32 m_fPossessSpeed;						// Speed multiplier to make possession faster

	f32 m_fUnitActivateAnim;					// unit progress through console activation animation

	CEShield *m_pShield;						// shield that is enabled when console is operating bot

	f32 m_fUnitLightAnim;						// unit anim time for das blinken lighten
	f32 m_fMonitorScroll;						// scroll position of monitor tex
	f32 m_fMonitorEmissive;						// current emissive value for monitor
	FMeshTexLayerHandle_t m_hChipLightTex[MAX_CHIPS];	// handles to chip light textures
	FMeshTexLayerHandle_t m_hButtonLightTex[MAX_CHIPS];	// handles to side button light textures
	FMeshTexLayerHandle_t m_hMonitorTex;		// handle to monitor texture
	FMeshTexLayerHandle_t m_hMonitorStripTex;	// handle to monitor strip texture
	FMeshTexLayerHandle_t m_hHeaderTex;			// handle to cabinet header texture
	FMeshTexLayerHandle_t m_hJoystickTex;		// handle to joystick light texture

	CFAudioEmitter *m_pAttractEmitter;			// emitter for attract sound


//----------------------------------------------------------------------------------------------------------------------------------
// Public Functions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	// System:
	static BOOL InitSystem( void );
	static void UninitSystem( void );
	static BOOL IsSystemInitialized( void ) { return m_bSystemInitialized && CEntity::IsSystemInitialized(); }


	// Construct/Destruct:
	CEConsole();
	virtual ~CEConsole();

	virtual BOOL ClassHierarchyLoadSharedResources( void );
	virtual void ClassHierarchyUnloadSharedResources( void );

	// Other:
	virtual void AppendTrackerSkipList(u32& FWorld_nTrackerSkipListCount=FWorld_nTrackerSkipListCount, CFWorldTracker ** FWorld_apTrackerSkipList=&FWorld_apTrackerSkipList[0]);

	// Checkpoint routines
	virtual BOOL CheckpointSave( void );
	virtual void CheckpointRestore( void );
	virtual void CheckpointSaveSelect( s32 nCheckpoint );

	static void DrawText( CBotGlitch *pBotGlitch );

	BOOL InOperationArea( CBotGlitch *pBotGlitch );
	FINLINE BOOL IsIdle( void ) const { FASSERT( IsCreated() ); return m_nConsoleState == CONSOLE_STATE_IDLE; };
	FINLINE u32 NumInsertedChips( void ) const { FASSERT( IsCreated() ); return m_nInsertedChipCount; };
	FINLINE u32 NumSockets( void ) const { FASSERT( IsCreated() ); return m_nSocketCount; };
	FINLINE BOOL WasUsedOnce( void ) const { FASSERT( IsCreated() ); return !!(m_nConsoleFlags & _CONSOLE_FLAG_USED_IT_ONCE); };



//----------------------------------------------------------------------------------------------------------------------------------
// Protected Functions:
//----------------------------------------------------------------------------------------------------------------------------------
protected:

	virtual void ClassHierarchyDestroy( void );

	virtual BOOL ClassHierarchyBuild( void );
	virtual BOOL ClassHierarchyBuilt( void );
	virtual CEntityBuilder *GetLeafClassBuilder( void );
	virtual void ClassHierarchyResolveEntityPointerFixups( void );

	virtual void ClassHierarchyWork( void );
	virtual void ClassHierarchyAddToWorld( void );
	virtual void ClassHierarchyRemoveFromWorld( void );
	virtual void ClassHierarchyRelocated( void *pIdentifier );




//----------------------------------------------------------------------------------------------------------------------------------
// Private Functions:
//----------------------------------------------------------------------------------------------------------------------------------
private:

	static BOOL _OperatorBoxAction( CEntity *pActionerEntity, CEntity *pOperatorBoxEntity );
	void _ClearDataMembers( void );
	BOOL _LocateSlotPositions( cchar *pszNamePrefix );

	// Possession effect routines
	void _TriggerPossession( void );
	void _StartPossessionEffect( void );
	void _Work_ChipFlying( void );
	void _Work_ChipInserting( void );
	void _Work_StartActivation( void );
	void _Work_ActivateAnimation( void );
	void _Work_FadeOutWithLogo( void );
	void _Work_FadeInBehindBot( void );
	void _Work_ControllingBot( void );
	void _Work_CheckpointRestore( void );
	static void _DrawMilLogoOverlay( u32 nPlayerIndex, void *pUser );
	void _UpdateLights( void );
	void _SetChipLights( u32 uStart, u32 uEnd, LightColor_e eColor );
	void _SetButtonLights( u32 uStart, u32 uEnd, LightColor_e eColor );
	void _SetMonitor( BOOL bOn );
	void _SetMonitorStrip( BOOL bOn );
	void _SetHeader( BOOL bOn );
	void _SetJoystickLight( BOOL bOn );
	void _UpdateMonitor( void );
	BOOL _AttachJoystickToConsole( void );
	void _AnimateJoystick( void );
	void _ComputeOperationPosition( CFVec3A &rvPos );
	void _PlaceGlitchAtConsole( void );


	FCLASS_STACKMEM_ALIGN( CEConsole );
} FCLASS_ALIGN_SUFFIX;




//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CEConsoleBuilder
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

FCLASS_ALIGN_PREFIX class CEConsoleBuilder : public CEntityBuilder {
//----------------------------------------------------------------------------------------------------------------------------------
// Public Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
public:




//----------------------------------------------------------------------------------------------------------------------------------
// Public Data:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	cchar *m_pszMilName;								// The name of the Mil this console will control
	cchar *m_pszStickMesh;								// Joystick mesh (NULL=use default)
	cchar *m_pszConsoleMesh;							// Console mesh (NULL=baked into world)
	cchar *m_pszBaseSocketName;							// Base socket name prefix (NULL=none)
	cchar *m_pszDispenserName;							// Name of the dispenser this console should use to create a bot to possess
	u32 m_nSocketCount;									// Number of sockets (0=none)
	u32 m_nInsertedChipCount;							// The number of sockets that start out with chips in them (0=none)
	f32 m_fChipScale;									// The scale to use for placing the chip into the socket
	BOOL m_bUseOnce;									// this console can only be used once.




//----------------------------------------------------------------------------------------------------------------------------------
// Public Functions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	FINLINE CEConsoleBuilder() {}
	virtual void SetDefaults( u64 nEntityTypeBits, u64 nEntityLeafTypeBit, cchar *pszEntityType );




//----------------------------------------------------------------------------------------------------------------------------------
// Protected Functions:
//----------------------------------------------------------------------------------------------------------------------------------
protected:

	virtual BOOL InterpretTable( void );
	virtual BOOL PostInterpretFixup( void );


	FCLASS_STACKMEM_ALIGN( CEConsoleBuilder );
} FCLASS_ALIGN_SUFFIX;



#endif

