//////////////////////////////////////////////////////////////////////////////////////
// botscout.h - Scout Bot
//
// Author: Chris MacDonald
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
// 02/25/03 MacDonald   Created.
//////////////////////////////////////////////////////////////////////////////////////

#ifndef _BOTSCOUT_H_
#define _BOTSCOUT_H_ 1


#include "fang.h"
#include "bot.h"
#include "fparticle.h"
#include "fforce.h"
#include "fAntenna.h"
#include "FXStreamer.h"
#include "tracer.h"
#include "explosion.h"
#include "eparticle.h"

// Forward Declarations
struct FAnim_t;
class CBotAIBrain;
class CMeshEntity;
class CBotPartPool;
class CFTextInst;
class CDamageResult;


FCLASS_ALIGN_PREFIX class CBotScout : public CBot
{
//----------------------------------------------------------------------------------------------------------------------------------
// Public Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	// Misc definitions:
	enum
	{
		NUM_TRANS_TEX = 3						// number of transmission effect textures
	};

	enum
	{
		TAG_POINT_TORSO,
		TAG_POINT_HEAD,
		TAG_POINT_ANTENNA3,
		TAG_POINT_COUNT
	};

	typedef struct
	{
		CFVec3A vStart;
		CFVec3A vEnd;
		CFColorRGBA vColor;
		f32 fTexFrame;
		f32 fTexScroll;
	} TransBeam_t;

//----------------------------------------------------------------------------------------------------------------------------------
// Protected Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
protected:

	ENTITY_CLASS_HIERARCHY_BITDEF




//----------------------------------------------------------------------------------------------------------------------------------
// Private Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
private:

	enum
	{
		ANIM_IDLE_COUNT					= 1,
	};

	// Animation controls:
	typedef enum
	{
		ANIMCONTROL_SCAN,
		ANIMCONTROL_HEADUP,
		ANIMCONTROL_BASE_COUNT
	} AnimControl_e;


	// Animation taps:
	typedef enum
	{
		ANIMTAP_SCAN,
		ANIMTAP_HEADUP,
		ANIMTAP_BASE_COUNT
	} AnimTap_e;


	// Base animations:
	typedef enum
	{
		ANIM_SCAN,
		ANIM_HEADUP,
		ANIM_BASE_COUNT
	} Anim_e;


	// Bones:
	typedef enum
	{
		BONE_HEAD,				// "Groin",						
		BONE_TORSO,				// "Torso",
		BONE_NECK1,				// "Neck1"
		BONE_NECK2,				// "Neck2"
		BONE_NECK3,				// "Neck3"
		BONE_NECK4,				// "Neck4"
		BONE_ANTENNA1,			// "Antenna1"
		BONE_ANTENNA2,			// "Antenna2"
		BONE_ANTENNA3,			// "Antenna3"
		BONE_ANTENNA4,			// "Antenna4"
		BONE_ATTACHPOINT_BEAM,	// "AttachPoint_Beam"
		BONE_DUMMY,				// "ScoutDummy"

		BONE_COUNT
	} Bone_e;


	enum
	{
		SCOUT_STATE_IDLE,
		SCOUT_STATE_RAISING_SCANNER,
		SCOUT_STATE_SCANNING,
		SCOUT_STATE_START_LOWERING_SCANNER,
		SCOUT_STATE_LOWERING_SCANNER,
		SCOUT_STATE_START_SURPRISE,
		SCOUT_STATE_SURPRISE,
		SCOUT_STATE_SURPRISE_JUMP,
		SCOUT_STATE_SURPRISE_LAND,
		SCOUT_STATE_SURPRISE_FLASH,
	};

#define SCOUT_FLAG_SHOULD_SCAN		( 0x00000001 )
#define SCOUT_FLAG_DRAW_BEAM		( 0x00000002 )
#define SCOUT_FLAG_TUMBLE			( 0x00000004 )
#define SCOUT_FLAG_TARGET_SIGHTED	( 0x00000008 )
#define SCOUT_FLAG_DAMAGE_TAKEN		( 0x00000010 )
#define SCOUT_FLAG_TICK_SOUND		( 0x00000020 )
#define SCOUT_FLAG_AT_SWITCH		( 0x00000040 )

	typedef struct
	{
		FParticle_DefHandle_t hSmokeParticle;			// damaged smoke particle
		f32 fDamageSlowdownAmount;						// max amount to slow down scout as damage increases, 0.0 = none, 1.0 = stopped
		cchar *apszScanConeMeshName[1];					// name of scanning cone effect mesh
		f32 fScanConeMinScale;							// minimum scale of scan cone mesh
		f32 fScanConeScaleRange;						// max scale added to fScanConeMinScale when animating
		f32 fScanConeScaleFrequency;					// frequency at which scan cone should scale 
		f32 fScanConeLightFlicker;						// amount to flicker spotlight in conjunction with scaling anim. 0.0 = none, 1.0 = full off
		CFSoundGroup *pSoundGroupAlertChirp;
		CFSoundGroup *pSoundGroupScanning;
		CFSoundGroup *pSoundGroupServo1;
		CFSoundGroup *pSoundGroupServo2;
		CFSoundGroup *pSoundGroupTreads;
		CFSoundGroup *pSoundGroupAlarmZap;
		CFSoundGroup *pSoundGroupTick;					//ticking sound for when scout is in blow up state
#if FANG_WINGC
		CFSoundGroup *pSoundGroupSiren;					// Retail addition (ScoutSiren); not played yet
#endif
		CDamageProfile* pScoutDetDamageProfile;
#if FANG_WINGC
		CFDebrisGroup *pDebrisGroupGuts;				// Retail addition (ScoutGuts); not spawned yet
#endif
	} BotInfo_Scout_t;


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

	// BotInfo data:
	static BotInfo_Gen_t			m_BotInfo_Gen;						// General bot info
	static BotInfo_Walk_t			m_BotInfo_Walk;						// Walk bot info
	static BotInfo_Weapon_t			m_BotInfo_Weapon;					// Weapon bot info
	static BotInfo_Scout_t			m_BotInfo_Scout;					// scout bot info

	static const FGameData_TableEntry_t m_aBotInfoVocab_Scout[];

	static CBotScout*				m_pCBScout;							// ptr to this used for callbacks

	// Bot animation stack data:
	static CBotAnimStackDef m_AnimStackDef;								// Animation stack definition data

	static const FGameDataMap_t m_aGameDataMap[];

	static cchar *m_apszBoneNameTable[BONE_COUNT];						// Array of bone names
	static cchar *m_apszBaseControlNameTable[ANIMCONTROL_BASE_COUNT];	// Array of base animation control names
	static cchar *m_apszBaseTapNameTable[ANIMTAP_BASE_COUNT];			// Array of base animation tap names
	static cchar *m_apszBaseAnimNameTable[ANIM_BASE_COUNT];				// Array of base animation names

	static const u8 *m_apnEnableBoneNameIndexTableForEachBaseTap[ANIMTAP_BASE_COUNT];
	static const u8 m_aBoneEnableIndices_FullBody[];
	static const u8 m_anTagPointBoneNameIndexArray[TAG_POINT_COUNT];
	static const u8 m_nApproxEyePointBoneNameIndex;

	//bone groups
	static CFAnimCombinerConfig::ConfigStack_t	m_aAnimCombinerConfigStack[];
	static CFAnimCombinerConfig::ConfigTap_t	m_aAnimCombinerConfigTaps[];
	static CFAnimCombiner::AttachList_t			m_aAnimAttach[];

	static FExplosion_GroupHandle_t m_hExplosionGroup;				// scout destruction explosion group handle

	static u32 m_nBoneIndexHead;									// head bone index

	BotInfo_Scout_t *m_pBotInfo_Scout;								// pointer to scout bot info

	u32 m_nTagPointCount;											// Number of tag points
	const CFVec3A *m_apTagPoint_WS[TAG_POINT_COUNT];				// Tag points in world space
	const CFVec3A *m_pGazeDir_WS;									// Points to the gaze direction vector (might not be unit in length)
	CFQuatA m_qOrientation;											// quaternion representing scout orientation
	CFMtx43A m_mAirRot;												// rotation matrix while in air
	f32 m_fAirRot;
	u32 m_uScoutState;												// overall state of scout, see SCOUT_STATE_* for states
	u32 m_uScoutFlags;												// miscellaneous flags, see SCOUT_FLAG_* for details

	static CFTexInst m_TransmissionTexInst[NUM_TRANS_TEX];			// array of textures for transmission effect
	f32 m_fTransTexFrame;											// current frame of transmission texture animation to display
	f32 m_fTransTexScroll;											// tex coord scrolling for transmission texture
	CFVec3A m_vTransBeamEnd;										// world-space end point for transmission beam drawing
	f32 m_fPctSwitchPressComplete;									// unit value for how complete the switch press is.

  	FMeshTexLayerHandle_t m_hTreadTexLayerR;						// handle to right tread texture
	FMeshTexLayerHandle_t m_hTreadTexLayerL;						// handle to left tread texture
	f32 m_fTreadUnitAnimPos;										// position of tread textures

	f32 m_fSurpriseTimer;											// time remaining to act surprised
	f32 m_fHeadScanSpeed;											// velocity of head scanning
	f32 m_fHeadScanAngle;											// y angle of head
	f32 m_fDistToTarget;

	f32 m_fDamageRot;												// y rotation caused this frame by inflicted damage
	f32 m_fTickTimer;
	f32 m_fTickRate;

	CEParticle *m_pEParticleSmoke;									// damaged smoke emitter

	CMeshEntity *m_pScanConeMeshEntity;								// pointer to scanning cone effect mesh
	f32 m_fScanConeScaleAngle;										// angle of sinusoid used to scale scan cone mesh

	CFAudioEmitter *m_pAudioEmitterScanning;						// Scout scanning audio emitter
	CFAudioEmitter *m_pAudioEmitterTreads;							// Scout movement audio emitter 
	CFAudioEmitter *m_pAudioEmitterBeam;							// Scout transmission beam audio emitter 

	CFAntenna m_Antenna;											// antenna animator
	//----------------------------------------------------------------------------------------------------------------------------------
	// Public Functions:
	//----------------------------------------------------------------------------------------------------------------------------------
public:
	CBotScout();
	~CBotScout();

	static BOOL InitSystem( void );
	static void UninitSystem( void );
	static BOOL InitLevel( void );
	static void UninitLevel( void );

	static void DrawBeams( void );

	BOOL	Create( s32 nPlayerIndex=-1, BOOL bInstallDataPort=FALSE, cchar *pszEntityName=NULL, const CFMtx43A *pMtx=NULL, cchar *pszAIBuilderName=NULL );

	virtual u32 GetTagPointCount( void ) const { return m_nTagPointCount; }
	virtual const CFVec3A *GetTagPoint( u32 nTagPointIndex ) const;
	virtual const CFVec3A *GetApproxEyePoint( void ) const;

	virtual void AppendTrackerSkipList(u32& FWorld_nTrackerSkipListCount=FWorld_nTrackerSkipListCount, CFWorldTracker ** FWorld_apTrackerSkipList=&FWorld_apTrackerSkipList[0]);			

	virtual void Die( BOOL bSpawnDeathEffects=TRUE, BOOL bSpawnGoodies=TRUE );

	virtual BOOL CheckpointSave( void );
	virtual void CheckpointRestore( void );
	virtual void CheckpointSaveSelect( s32 nCheckpoint );

	virtual FINLINE void ComputeMtxPalette( BOOL bApplyOffscreenOptimizations ) { FASSERT( IsCreated() ); FASSERT( m_Anim.m_pAnimCombiner != NULL ); m_pCBScout = this; m_Anim.m_pAnimCombiner->ComputeMtxPalette(bApplyOffscreenOptimizations); m_pCBScout = NULL; };
	f32 ComputeEstimatedControlledStopTimeXZ( void );
	f32 ComputeEstimatedControlledStopTimeY( void );
	void SetScanning( BOOL bScan );
	BOOL IsScanning( void ) const { FASSERT( IsCreated() ); return m_uScoutState == SCOUT_STATE_SCANNING; }

	void InflictDamageResult( const CDamageResult *pDamageResult );

	void TargetSighted( f32 fDistToTarget );		// call once when scout first sees target which will make him go for the alarm
	BOOL IsSurprised( void );	// TRUE if scout is acting surprised after sighting a target.
	void ActivateBeam( CFVec3A &rvBeamEnd, BOOL bScoutAtSwitch, f32 fPctSwitchPressComplete);		// must be called every frame that transmission beam should be drawn


	void SetTickRate(f32 fTicksPerSec);
	// bone callback
	void AnimateHead( u32 uBoneIdx, CFMtx43A &rNewMtx, const CFMtx43A &rParentMtx, const CFMtx43A &rBoneMtx );

	static void DrawTransmissionBeam( CFVec3A &rvStart, CFVec3A &rvEnd, CFColorRGBA &rvColor, f32 fTexFrame, f32 fTexScroll , CFTexInst* pTexInst);//pgm made this public so that EG could utilize it's greatness

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

	virtual BOOL NotifyBotCollision( CBot *pBot );		// notifies the bot it has collided with another bot.  if FALSE is returned, normal collision response is skipped.
//----------------------------------------------------------------------------------------------------------------------------------
// Private Functions:
//----------------------------------------------------------------------------------------------------------------------------------
private:
	static BOOL _BuildAnimStackDef( void );

	void _ClearDataMembers( void );
	void _UpdateMatrices( void );

	static void _AnimBoneCallback( u32 uBoneidx, CFMtx43A &rNewMtx, const CFMtx43A &rParentMtx, const CFMtx43A &rBoneMtx );
	void _UpdateBeam();

	friend class CBotScoutBuilder;

FCLASS_STACKMEM_ALIGN( CBotScout );
} FCLASS_ALIGN_SUFFIX;


//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CBotScoutBuilder
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

FCLASS_ALIGN_PREFIX class CBotScoutBuilder : public CBotBuilder
{
public:

//----------------------------------------------------------------------------------------------------------------------------------
// Public Functions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	FINLINE CBotScoutBuilder() {}
	virtual void SetDefaults( u64 nEntityTypeBits, u64 nEntityLeafTypeBit, cchar *pszEntityType );




//----------------------------------------------------------------------------------------------------------------------------------
// Protected Functions:
//----------------------------------------------------------------------------------------------------------------------------------
protected:

	virtual BOOL InterpretTable( void );
	virtual BOOL PostInterpretFixup( void );


	FCLASS_STACKMEM_ALIGN( CBotScoutBuilder );
} FCLASS_ALIGN_SUFFIX;

#endif

