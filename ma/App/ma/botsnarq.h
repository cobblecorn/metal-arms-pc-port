//////////////////////////////////////////////////////////////////////////////////////
// botSnarq.h - Snarq (mini predator)
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
// 05/01/03 Elliott     Created.
//////////////////////////////////////////////////////////////////////////////////////

#ifndef _BOTSNARQ_H_
#define _BOTSNARQ_H_ 1


#include "fang.h"
#include "bot.h"
#include "fparticle.h"
#include "fforce.h"
#include "FXStreamer.h"
#include "tracer.h"
#include "explosion.h"

// Forward Declarations
class  CBotAIBrain;



FCLASS_ALIGN_PREFIX class CBotSnarq : public CBot {
//----------------------------------------------------------------------------------------------------------------------------------
// Public Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	typedef enum {
		FLIP_NONE,
		FLIP_ROLL,
		FLIP_PITCH,
		FLIP_QUICKTURN,
		FLIP_CRAZY,
	} FlipMode_e;


//----------------------------------------------------------------------------------------------------------------------------------
// Protected Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
protected:

	ENTITY_CLASS_HIERARCHY_BITDEF




//----------------------------------------------------------------------------------------------------------------------------------
// Private Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
private:

	// Bones:
	typedef enum {
		BONE_DUMMY,				// SnarqDummy

		BONE_JETBACK1,			// AttachPoint_JetBack1
		BONE_JETBACK2,			// AttachPoint_JetBack2
		BONE_JETDOWN_L,			// AttachPoint_L_JetDown
		BONE_JETDOWN_R,			// AttachPoint_R_JetDown

		BONE_STREAMER_L,		// AttachPoint_L_Streamer
		BONE_STREAMER_R,		// AttachPoint_R_Streamer
		
		BONE_PRIFIRE_L,			// Primary_Fire_L
		BONE_PRIFIRE_R,			// Primary_Fire_R

		BONE_COUNT
	} Bone_e;

	typedef struct {
		f32 fMaxXZVel;
		f32 fMaxXZAccel;
		f32 fMaxYVel;
		f32 fMaxYAccel;

		u32 uMaxTracers;
		f32 fTracerWidth;		
		f32 fTracerLength;	
		f32 fTracerSpeed;	
		f32 fTracerTailDist;

		u32	uStreamerDepth;
		f32 fStreamerAlpha; 
		f32 fStreamerSize;
		f32 fStreamerSamples;
		FTexDef_t *pStreamerTex;

		f32 fOOFireRate;
		f32 fOOFireSoundRate;

		f32 fEngineResponseRate;				// how fast the particles/sounds respond to changes in accel

		CDamageProfile *pGunDamageProfile;
		CFSoundGroup *pHoverSound;
		CFSoundGroup *pGunSound;
		FParticle_DefHandle_t hExhaustParticle;
		FExplosion_GroupHandle_t hDeathExplosion1;
		FExplosion_GroupHandle_t hDeathExplosion2;
	} BotInfo_Snarq_t;

	typedef enum {
		BONEFLAG_NONE		= 0x0,
		BONEFLAG_ARMOR		= 0x1,
		BONEFLAG_DESTROYED	= 0x2		
	} BoneFlag_e;





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
	static BotInfo_MountAim_t		m_BotInfo_MountAim;					// Mount aim bot info
	static BotInfo_Walk_t			m_BotInfo_Walk;						// Walk bot info
	static BotInfo_Jump_t			m_BotInfo_Jump;						// Jump bot info
	static BotInfo_Weapon_t			m_BotInfo_Weapon;					// Weapon bot info
	static BotInfo_Snarq_t			m_BotInfo_Snarq;					// Snarq bot info

	static CBotSnarq*				m_pCBSnarq;							// ptr to this used for callbacks

	static const FGameDataMap_t m_aGameDataMap[];
	static const FGameDataMap_t m_aSoundDataMap[];
	static const FGameData_TableEntry_t m_aBotInfoVocab_Snarq[];

	static cchar *m_apszBoneNameTable[BONE_COUNT];						// Array of bone names

	static const u8 m_anTagPointBoneNameIndexArray[];
	static const u8 m_nApproxEyePointBoneNameIndex;

	const CFVec3A *m_pGazeDir_WS;										// Points to the gaze direction vector (might not be unit in length)

	static TracerGroupHandle_t	m_hTracerGroup;							// tracer group
	static TracerDef_t			m_TracerDef;							// Tracer definition for guns
	static CFTexInst			m_GunTracerTex;							// tracer tex
	
	f32 m_fGunTimer;
	f32	m_fGunSoundTimer;
	BOOL m_bFireLeftSide;
    
	// bones
	s32 m_nBoneIdxPriFireLeft;
	s32 m_nBoneIdxPriFireRight;
	s32 m_nBoneIdxJetTop;
	s32 m_nBoneIdxJetBottom;

	// streamers
	static cchar* m_apszStreamerBones[];
	FXStreamerHandle_t m_hStreamer;
	static CFTexInst m_StreamerTex;

	// sound
	CFAudioEmitter *m_pHoverAudioEmitter;
	
	// particles
	FParticle_EmitterHandle_t	m_hJetPartTop;
	FParticle_EmitterHandle_t	m_hJetPartBottom;

	f32 m_fEngineLevel;

	CFAnimCombiner *m_pAnimCombiner;
	CFAnimMeshRest *m_pAnimMeshRest;
	
	CFVec3A m_vUnitPYR;			// pitch/yaw/roll
	f32 m_fUnitYawTimer;
	f32 m_fUnitRollTimer;

	// array of bones in this model.  Used to determine which are armor bones and which are alive/dead
	u8	*m_pauArmorBones;


	// flipping
	FlipMode_e m_FlipMode;
	f32 m_fOOFlipTime;
	f32 m_fUnitFlipTimer;
	f32 m_fFlipRevolutions;
	CFVec3A m_vFlipAxis_MS;
	f32		m_fFlipRads;
	CFQuatA m_qQTStart;
	CFQuatA m_qQTFinal;
	CFQuatA m_qQTHalfway;
	
		
	//----------------------------------------------------------------------------------------------------------------------------------
	// Public Functions:
	//----------------------------------------------------------------------------------------------------------------------------------
public:
	CBotSnarq();
	~CBotSnarq();

	static BOOL InitSystem( void );
	static void UninitSystem( void );

	BOOL	Create( s32 nPlayerIndex=-1, BOOL bInstallDataPort=FALSE, cchar *pszEntityName=NULL, const CFMtx43A *pMtx=NULL, cchar *pszAIBuilderName=NULL );

	virtual const CFVec3A *GetApproxEyePoint( void ) const;

	virtual void AppendTrackerSkipList(u32& FWorld_nTrackerSkipListCount=FWorld_nTrackerSkipListCount, CFWorldTracker ** FWorld_apTrackerSkipList=&FWorld_apTrackerSkipList[0]);			

	virtual BOOL CheckpointSave( void );
	virtual void CheckpointRestore( void );
	virtual void CheckpointSaveSelect( s32 nCheckpoint );

	virtual void ComputeMtxPalette( BOOL bApplyOffscreenOptimizations )	{ FASSERT( IsCreated() ); }

	virtual f32 ComputeEstimatedControlledStopTimeXZ( void )		{ FASSERT( IsCreated() ); return fmath_Div( m_fSpeedXZ_WS, m_BotInfo_Snarq.fMaxXZAccel ); }
	virtual FINLINE f32 ComputeEstimatedControlledStopTimeY( void ) { FASSERT( IsCreated() ); return fmath_Div( m_Velocity_WS.y, m_BotInfo_Snarq.fMaxYAccel ); }

	virtual void InflictDamageResult( const CDamageResult* pDamageResult );
	virtual void Die( BOOL bSpawnDeathEffects=TRUE, BOOL bSpawnGoodies=TRUE );

	void ClearSpin( void );
	void StartRoll( f32 fNumRevs=1.0f, u32 uDirection=0 );		// Set direction to 1 CW, 0 (random), or -1 CCW
	void StartFlip( FlipMode_e flipTYpe, f32 fNumRevs=1.0f );
	void StartQuickturn( f32 fNewMountYaw_WS );


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

	virtual void DeathWork( void );


//----------------------------------------------------------------------------------------------------------------------------------
// Private Functions:
//----------------------------------------------------------------------------------------------------------------------------------
private:
	void _ClearDataMembers( void );
	void _UpdateMatrices( void );

	static void _AnimBoneCallback( u32 uBoneidx, CFMtx43A &rNewMtx, const CFMtx43A &rParentMtx, const CFMtx43A &rBoneMtx );

	void _HandleTranslation( void );

	void _SoundWork( void );
	void _ParticleWork( void );

	void _GunWork( void );
	void _FireRound( void );

	void _StopAllEffects( void );


	static void _TracerKilledCallback( TracerDef_t *pTracerDef, TracerKillReason_e nKillReason, const FCollImpact_t *pImpact );
	static void _TracerBuildTrackerSkipList( void *pUser );


	BOOL _InitArmorBoneArray( void );
	void _CheckForArmorBoneDestroy( const CDamageResult* pDamageResult );
	void _MoveDestroyedBones( void );

	void _CalcPitchYawRoll( void );

	void _FlipWork( void );

	void _BuildFlipMtx( CFMtx43A *pmtx );

	void _Power_Work( void );

	friend class CBotSnarqBuilder;

FCLASS_STACKMEM_ALIGN( CBotSnarq );
} FCLASS_ALIGN_SUFFIX;


//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CBotSnarqBuilder
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

FCLASS_ALIGN_PREFIX class CBotSnarqBuilder : public CBotBuilder {
public:

//----------------------------------------------------------------------------------------------------------------------------------
// Public Functions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	FINLINE CBotSnarqBuilder() {}
	virtual void SetDefaults( u64 nEntityTypeBits, u64 nEntityLeafTypeBit, cchar *pszEntityType );




//----------------------------------------------------------------------------------------------------------------------------------
// Protected Functions:
//----------------------------------------------------------------------------------------------------------------------------------
protected:

	virtual BOOL InterpretTable( void );
	virtual BOOL PostInterpretFixup( void );


	FCLASS_STACKMEM_ALIGN( CBotSnarqBuilder );
} FCLASS_ALIGN_SUFFIX;

#endif


