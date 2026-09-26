//////////////////////////////////////////////////////////////////////////////////////
// botswarmerboss.h - 
//
// Author: Mike Elliott     
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
// 03.31.03 Elliott     Created.
//////////////////////////////////////////////////////////////////////////////////////

#if 0

#ifndef _BOTSWARMERBOSS_H_
#define _BOTSWARMERBOSS_H_ 1


#include "fang.h"
#include "bot.h"

// Forward Declarations
class  CBotPartPool;



FCLASS_ALIGN_PREFIX class CBotSwarmerBoss : public CBot {
//----------------------------------------------------------------------------------------------------------------------------------
// Public Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
public:
	typedef enum {
        MOVESTATE_NORMAL,
		MOVESTATE_WALL,
		MOVESTATE_JUMPING_TO_FLOOR,
		MOVESTATE_JUMPING_TO_WALL,


		MOVESTATE_COUNT
	} MoveState_e;



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
		PART_INSTANCE_COUNT_PER_TYPE	= 3,
		ANIM_IDLE_COUNT					= 1,	// SER: Eventually this will be retrieved from a data file
		_MAX_HIT_ENTITIES				= 4,
		_FIREPOINT_COUNT				= 4
	};


	typedef enum {
		LIMB_TYPE_COUNT
	} LimbType_e;


	// Animation controls:
	typedef enum {
		ANIMCONTROL_STAND,
		ANIMCONTROL_WALK,
		
		ANIMCONTROL_JUMP_LAUNCH,
		ANIMCONTROL_JUMP_FLY,
		ANIMCONTROL_JUMP_LAND,

		ANIMCONTROL_LOWER,
		ANIMCONTROL_SLASH,
		ANIMCONTROL_FIRE,
        
		ANIMCONTROL_BASE_COUNT
	} AnimControl_e;


	// Animation taps:
	typedef enum {
		ANIMTAP_STAND,
		ANIMTAP_WALK,
		
		ANIMTAP_JUMP_LAUNCH,
		ANIMTAP_JUMP_FLY,
		ANIMTAP_JUMP_LAND,

		ANIMTAP_LOWER,
		ANIMTAP_SLASH,
		ANIMTAP_FIRE,

		ANIMTAP_BASE_COUNT
	} AnimTap_e;


	// Base animations:
	typedef enum {
		ANIM_STAND,				// "ARMAidle001"
		ANIM_WALK,				// "ARMAwalkF01"
		
		ANIM_JUMP_LAUNCH,		// "ARMAjump_01"
		ANIM_JUMP_FLY,			// "ARMAjump_02"
		ANIM_JUMP_LAND,			// "ARMAjump_03"

		ANIM_LOWER,				// "ARMAlower01"
		ANIM_SLASH,				// "ARMAslash01"
		ANIM_FIRE,				// "ARMAaimF_01"

		ANIM_BASE_COUNT
	} Anim_e;


	// Bones:
	typedef enum {
		BONE_DUMMY,			// SwarmerBoss_Dummy
		BONE_TORSO,			// Torso
		BONE_LEG_LF_A,		// L_Front_Leg_A
		BONE_LEG_LF_B,		// L_Front_Leg_B
		BONE_LEG_LF_C,		// L_Front_Leg_C
		BONE_LEG_LF_D,		// L_Front_Leg_D
		BONE_LEG_LF_E,		// L_Front_Leg_E
		BONE_LEG_LF_DUMMY,	// L_Front_Foot_Dummy
				
		BONE_LEG_RF_A,		// R_Front_Leg_A
		BONE_LEG_RF_B,		// R_Front_Leg_B
		BONE_LEG_RF_C,		// R_Front_Leg_C
		BONE_LEG_RF_D,		// R_Front_Leg_D
		BONE_LEG_RF_E,		// R_Front_Leg_E
		BONE_LEG_RF_DUMMY,	// R_Front_Foot_Dummy

		BONE_DIODE_L,		// L_Diode
		BONE_DIODE_R,		// R_Diode
		BONE_DIODE_FRONT,	// Diode_Front
		BONE_ASS,			// Ass	

		BONE_LEG_LR_A,		// L_Rear_Leg_A
		BONE_LEG_LR_B,		// L_Rear_Leg_B
		BONE_LEG_LR_C,		// L_Rear_Leg_C
		BONE_LEG_LR_D,		// L_Rear_Leg_D
		BONE_LEG_LR_E,		// L_Rear_Leg_E
		BONE_LEG_LR_DUMMY,	// L_Rear_Foot_Dummy

		BONE_LEG_RR_A,		// R_Rear_Leg_A
		BONE_LEG_RR_B,		// R_Rear_Leg_B
		BONE_LEG_RR_C,		// R_Rear_Leg_C
		BONE_LEG_RR_D,		// R_Rear_Leg_D
		BONE_LEG_RR_E,		// R_Rear_Leg_E
		BONE_LEG_RR_DUMMY,	// R_Rear_Foot_Dummy

		BONE_THORAX_A,		// Thorax_A
		BONE_THORAX_B,		// Thorax_B
		BONE_THORAX_C,		// Thorax_C
		BONE_THORAX_D,		// Thorax_D

		BONE_PRIFIRE,		// Primary_Fire
		BONE_HEAD,			// Head
		BONE_MANDIBLE_R,	// R_Mandible
		BONE_MANDIBLE_L,	// L_Mandible
			
		BONE_FEELER_R3A,	// R_Feeler_3_A
		BONE_FEELER_R3B,	// R_Feeler_3_B
		BONE_FEELER_R3C,	// R_Feeler_3_C
		BONE_FEELER_R3D,	// R_Feeler_3_D
		BONE_FEELER_R1A,	// R_Feeler_1_A
		BONE_FEELER_R1B,	// R_Feeler_1_B
		BONE_FEELER_R1C,	// R_Feeler_1_C
		BONE_FEELER_R1D,	// R_Feeler_1_D
		BONE_FEELER_R2A,	// R_Feeler_2_A
		BONE_FEELER_R2B,	// R_Feeler_2_B
		BONE_FEELER_R2C,	// R_Feeler_2_C
		BONE_FEELER_R2D,	// R_Feeler_2_D
		BONE_FEELER_L3A,	// L_Feeler_3_A
		BONE_FEELER_L3B,	// L_Feeler_3_B
		BONE_FEELER_L3C,	// L_Feeler_3_C
		BONE_FEELER_L3D,	// L_Feeler_3_D
		BONE_FEELER_L1A,	// L_Feeler_2_A
		BONE_FEELER_L1B,	// L_Feeler_2_B
		BONE_FEELER_L1C,	// L_Feeler_2_C
		BONE_FEELER_L1D,	// L_Feeler_2_D
		BONE_FEELER_L2A,	// L_Feeler_1_A
		BONE_FEELER_L2B,	// L_Feeler_1_B
		BONE_FEELER_L2C,	// L_Feeler_1_C
		BONE_FEELER_L2D,	// L_Feeler_1_D

		BONE_COUNT
	} Bone_e;


	typedef struct {
	} BotInfo_SwarmerBoss_t;

//----------------------------------------------------------------------------------------------------------------------------------
// Public Data:
//----------------------------------------------------------------------------------------------------------------------------------
public:


//----------------------------------------------------------------------------------------------------------------------------------
// Private Data:
//----------------------------------------------------------------------------------------------------------------------------------
private:
	static BOOL	 m_bSystemInitialized;									// TRUE: InitSystem() has been called
	static u32 m_nBotClassClientCount;									// Number of bots of this class using the class-wide resources
	static const CBotDef m_BotDef;										// Bot definition data


	// Bot parts:
	static CBotPartPool *m_pPartPool;									// One part pool per bot class

	// BotInfo data:
	static BotInfo_Gen_t			m_BotInfo_Gen;						// General bot info
	static BotInfo_MountAim_t		m_BotInfo_MountAim;					// Mount aim bot info
	static BotInfo_Walk_t			m_BotInfo_Walk;						// Walk bot info
	static BotInfo_Jump_t			m_BotInfo_Jump;						// Jump bot info
	static BotInfo_Weapon_t			m_BotInfo_Weapon;					// Weapon bot info
	static BotInfo_SwarmerBoss_t	m_BotInfo_SwarmerBoss;				// this bot's data

	static CBotSwarmerBoss*			m_pCBSBoss;							// ptr to this used for callbacks

	// Bot animation stack data:
	static CBotAnimStackDef m_AnimStackDef;								// Animation stack definition data

	static const FGameDataMap_t m_aGameDataMap[];
	static const FGameData_TableEntry_t m_aBotInfoVocab_SwarmerBoss[];
	


	static cchar *m_apszBoneNameTable[BONE_COUNT];						// Array of bone names
	static cchar *m_apszBaseControlNameTable[ANIMCONTROL_BASE_COUNT];	// Array of base animation control names
	static cchar *m_apszBaseTapNameTable[ANIMTAP_BASE_COUNT];			// Array of base animation tap names
	static cchar *m_apszIdleAnimNameTable[ANIM_IDLE_COUNT];				// Array of idle animation names
	static cchar *m_apszBaseAnimNameTable[ANIM_BASE_COUNT];				// Array of base animation names

	static const u8 *m_apnEnableBoneNameIndexTableForEachBaseTap[ANIMTAP_BASE_COUNT];
	static const u8 *m_apnEnableBoneNameIndexTableForEachIdleTap[ANIM_IDLE_COUNT];
	static const u8 m_anEnableBoneNameIndexTableForSummer_Normal[];
	static const u8 m_anEnableBoneNameIndexTableForSummer_TetherShock[];
	static const u8 m_aBoneEnableIndices_FullBody[];
	static const u8 m_aBoneEnableIndices_MainBody[];		// everything except the little feeler things
	//static const u8 m_anEnableBonesUpperBody[];
	//static const u8 m_aBoneEnableIndices_Attack[];
	//static const u8 m_aBoneEnableIndices_AimSummer[];
	//static const u8 m_anEnableBonesLand_Upper[];
	//static const u8 m_anEnableBonesLand_Lower[];
	static const u8 m_anTagPointBoneNameIndexArray[];
	//static const u8 m_nApproxEyePointBoneNameIndex;

	//user anim bone masks
	//static const u8 m_aBoneEnableIndices_UserAnim_UpperBody[];
	//static const u8 m_aBoneEnableIndices_UserAnim_LowerBody[];
	//static const u8 m_aBoneEnableIndices_UserAnim_UpperTorso[];
	//static const u8 m_aBoneEnableIndices_UserAnim_LowerTorso[];
	//static const u8 m_aBoneEnableIndices_UserAnim_LeftArm[];
	//static const u8 m_aBoneEnableIndices_UserAnim_RightArm[];
	//static const u8 m_aBoneEnableIndices_UserAnim_Head[];

	//bone groups
	static CFAnimCombinerConfig::ConfigStack_t	m_aAnimCombinerConfigStack[];
	static CFAnimCombinerConfig::ConfigTap_t	m_aAnimCombinerConfigTaps[];
	static CFAnimCombiner::AttachList_t			m_aAnimAttach[];

	static	CFVec3A m_GroinVecY_WS;								// Recorded groin matrice's Y vector (used in the animation callback)
	const	CFVec3A *m_pGazeDir_WS;								// Points to the gaze direction vector (might not be unit in length)

	MoveState_e m_eMoveState;
	CFVec3A m_vLastWallMountPos;

	const CFVec3A *m_pvTgtPos;
	f32 m_fOOGravity;

	// used to transition from wall to floor
	f32		m_fJumpTimer;
	f32		m_fOOJumpTime;
	CFQuatA m_qTakeoff;
	CFQuatA m_qLanding;
	CFVec3A m_vTakeoff;
	CFVec3A m_vLanding;
	

	//----------------------------------------------------------------------------------------------------------------------------------
	// Public Functions:
	//----------------------------------------------------------------------------------------------------------------------------------
public:
	CBotSwarmerBoss();
	~CBotSwarmerBoss();

	static BOOL InitSystem( void );
	static void UninitSystem( void );

	BOOL	Create( s32 nPlayerIndex=-1, BOOL bInstallDataPort=FALSE, cchar *pszEntityName=NULL, const CFMtx43A *pMtx=NULL, cchar *pszAIBuilderName=NULL );

	virtual void AppendTrackerSkipList(u32& FWorld_nTrackerSkipListCount=FWorld_nTrackerSkipListCount, CFWorldTracker ** FWorld_apTrackerSkipList=&FWorld_apTrackerSkipList[0]);			

	//virtual BOOL CheckpointSave( void );
	//virtual void CheckpointRestore( void );
	//virtual void CheckpointSaveSelect( s32 nCheckpoint );

	virtual void UserAnim_BatchUpdateTapBoneMask( UserAnimBoneMask_e nBoneMaskGroup );

	static void DebugDraw(  CBotSwarmerBoss *pSBoss  );

	virtual FINLINE void ComputeMtxPalette( BOOL bApplyOffscreenOptimizations ) { FASSERT( IsCreated() ); FASSERT( m_Anim.m_pAnimCombiner ); m_pCBSBoss = this; m_Anim.m_pAnimCombiner->ComputeMtxPalette(bApplyOffscreenOptimizations); m_pCBSBoss = NULL; };

	virtual FINLINE const CFVec3A* GetApproxEyePoint( void ) const { FASSERT( IsCreated() ); return m_pApproxEyePoint_WS; };

	FINLINE BOOL IsOnWall( void ) const { FASSERT( IsCreated() ); return m_eMoveState == MOVESTATE_WALL; }
	FINLINE void SetTargetPos( const CFVec3A *pvPos ) { FASSERT( IsCreated() ); m_pvTgtPos = pvPos; }
	

	//used by the minigame to adjust the bot's states
	void EnterMoveStateWall( void );
	void EnterMoveStateNormal( void );
	void JumpFromWallToPoint( const CFVec3A &vPt );
	FINLINE MoveState_e GetMoveState( void ) { FASSERT( IsCreated() ); return m_eMoveState; }

	

//----------------------------------------------------------------------------------------------------------------------------------
// Protected Functions:
//----------------------------------------------------------------------------------------------------------------------------------
protected:

	virtual BOOL ClassHierarchyLoadSharedResources( void );
	virtual void ClassHierarchyUnloadSharedResources( void );

	virtual BOOL ClassHierarchyBuild( void );				// Must be implemented by all classes in the derivation chain
	virtual void ClassHierarchyDestroy( void );

	virtual BOOL ClassHierarchyBuilt( void );
	virtual CEntityBuilder *GetLeafClassBuilder( void );	// Must be implemented by all classes in the derivation chain

	virtual void ClassHierarchyWork( void );
	virtual void ClassHierarchyAddToWorld( void );
	virtual void ClassHierarchyRemoveFromWorld( void );

//	virtual void FINLINE ComputeWeaponMuzzlePoint_WS( CFVec3A *pMuzzlePt )		{ FASSERT( IsCreated() ); *pMuzzlePt = m_pWorldMesh->GetBoneMtxPalette()[m_anBoneIdxPriFire[0]]->m_vPos; };
//	virtual void FINLINE ComputeWeaponMuzzleVec_WS( CFVec3A *pMuzzleVec )		{ FASSERT( IsCreated() ); *pMuzzleVec = m_pWorldMesh->GetBoneMtxPalette()[m_anBoneIdxPriFire[0]]->m_vFront; };

//----------------------------------------------------------------------------------------------------------------------------------
// Private Functions:
//----------------------------------------------------------------------------------------------------------------------------------
private:
	static BOOL _BuildAnimStackDef( void );

	void _ClearDataMembers( void );
	void _UpdateMatrices( void );

	static void _AnimBoneCallback( u32 uBoneidx, CFMtx43A &rNewMtx, const CFMtx43A &rParentMtx, const CFMtx43A &rBoneMtx );
	


	// jumping
	void _StartSingleJump( const CFVec3A *pvJumpVec=NULL );
	void _EnterFlyMode( void );
	void _HandleJumping( void );
	void _HandleJumpAnimations( void );
	void _JumpLanded( void );
	void _HandleGroundTranslation( void );

	void _NormalMoveWork( void );
	void _WallWork( void );
	void _JumpToFloorWork( void );


//METMP
#if SAS_ACTIVE_USER == SAS_USER_ELLIOTT
	void _DebugFn( void );
#endif
	
	

	friend class CBotSwarmerBossBuilder;

FCLASS_STACKMEM_ALIGN( CBotSwarmerBoss );
} FCLASS_ALIGN_SUFFIX;


//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CBotSwarmerBossBuilder
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

FCLASS_ALIGN_PREFIX class CBotSwarmerBossBuilder : public CBotBuilder {
public:

//----------------------------------------------------------------------------------------------------------------------------------
// Public Functions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	FINLINE CBotSwarmerBossBuilder() {}
	virtual void SetDefaults( u64 nEntityTypeBits, u64 nEntityLeafTypeBit, cchar *pszEntityType );




//----------------------------------------------------------------------------------------------------------------------------------
// Protected Functions:
//----------------------------------------------------------------------------------------------------------------------------------
protected:

	virtual BOOL InterpretTable( void );
	virtual BOOL PostInterpretFixup( void );


	FCLASS_STACKMEM_ALIGN( CBotSwarmerBossBuilder );
} FCLASS_ALIGN_SUFFIX;

#endif

#endif //0