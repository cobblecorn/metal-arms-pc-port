//////////////////////////////////////////////////////////////////////////////////////
// tether.h - Tether cable.
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
// 07/25/02 Ranck       Created.
//////////////////////////////////////////////////////////////////////////////////////

#ifndef _TETHER_H_
#define _TETHER_H_ 1

#include "fang.h"
#include "flinklist.h"
#include "fmath.h"
#include "fcoll.h"
#include "fdraw.h"
#include "ftex.h"
#include "fpsprite.h"
#include "smoketrail.h"
#include "fworld.h"
#include "fmotion.h"
#include "fforce.h"
#include "camsimple.h"
#include "fparticle.h"
#include "fexplosion.h"


class CEntity;
class CBot;
class CWeaponTether;
class CFSoundGroup;


#define TETHER_GAMEDATA_VOCAB \
	FGAMEDATA_VOCAB_SOUND_GROUP, \
	FGAMEDATA_VOCAB_SOUND_GROUP, \
	FGAMEDATA_VOCAB_SOUND_GROUP, \
	FGAMEDATA_VOCAB_SOUND_GROUP, \
	FGAMEDATA_VOCAB_SOUND_GROUP, \
	FGAMEDATA_VOCAB_SOUND_GROUP, \
	FGAMEDATA_VOCAB_SOUND_GROUP, \
	FGAMEDATA_VOCAB_PARTICLE, \
	FGAMEDATA_VOCAB_PARTICLE, \
	FGAMEDATA_VOCAB_PARTICLE, \
	FGAMEDATA_VOCAB_PARTICLE, \
	FGAMEDATA_VOCAB_DEBRIS_GROUP, \
							  \
	FGAMEDATA_VAR_TYPE_FLOAT| \
	FGAMEDATA_FLAGS_FLOAT_PERCENT_TO_UNIT_FLOAT | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO, \
	sizeof( f32 ), \
	F32_DATATABLE_0, \
	F32_DATATABLE_100




FCLASS_ALIGN_PREFIX class CTether {
//----------------------------------------------------------------------------------------------------------------------------------
// Public Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	typedef struct {
		CFSoundGroup *pSoundGroupFlyDownCable;			// Sound for flying down the cable
		CFSoundGroup *pSoundGroupDeRes;					// Sound for when player is materializing behind new bot
		CFSoundGroup *pSoundGroupZapSurge;				// Zap surge being sent down the cable
		CFSoundGroup *pSoundGroupDetach;				// Cable detaching from the weapon
		CFSoundGroup *pSoundGroupShatter;				// Cable shattering sound
		CFSoundGroup *pSoundGroupDataPort;				// Sound made when cable attaches to a data port
		CFSoundGroup *pSoundGroupChunkImpact;			// Sound made when the cable chunks blow into particles

		FParticle_DefHandle_t hParticleDataPortBurst;	// Burst made when cable attaches to a data port
		FParticle_DefHandle_t hParticleDataPortEffect;	// Effect made while cable is attached to a data port

		FParticle_DefHandle_t hParticleSeverSparks;		// Sparks made when the cable severs from the tether gun
		FParticle_DefHandle_t hParticleChunkPuff;		// Puff made when a cable chunk hits the ground
		CFDebrisGroup *pDebrisGroupChunk;				// Debris spawned when a cable chunk hits the ground

		f32 fPossessionArmorModifierDelta;				// Armor modifier delta to apply to the bot being possessed
	} UserProps_t;




//----------------------------------------------------------------------------------------------------------------------------------
// Protected Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
protected:




//----------------------------------------------------------------------------------------------------------------------------------
// Private Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
private:

	typedef enum {
		STATE_DESTROYED,			// The tether is destroyed (no work or draw, but dust & debris chunks might still be going)
		STATE_FLYING,				// The tether is flying through the air toward its target
		STATE_ZAPPING,				// The tether is tightening & zapping

		STATE_COUNT
	} State_e;


	typedef enum {
		POSSESSMODE_NORMAL,				// Normal mode
		POSSESSMODE_FLY_ALONG_CABLE,	// Camera is flying along the tether cable
		POSSESSMODE_FADE_IN_BEHIND_BOT,	// Camera is fading in behind the possessed bot
		POSSESSMODE_POWERING_UP_BOT,	// We're powering up the bot

		POSSESSMODE_COUNT
	} PossessMode_e;


	enum {
		FLAG_CREATED			= 0x00000001,		// This CTether has been created
		FLAG_DETACHED			= 0x00000002,		// Tether has detached from source
		FLAG_DEBRIS_DUST		= 0x00000004,		// Move & draw debris dust
		FLAG_DEBRIS_CHUNKS		= 0x00000008,		// Move debris chunks
		FLAG_WHITESAT_VIEW		= 0x00000010,		// TRUE when we're white-saturating the view
		FLAG_DRAWING_MIL_LOGO	= 0x00000020,		// TRUE when we're drawing the Mil logo
		FLAG_CONTACT			= 0x00000040,		// TRUE if the tether connected with the port and we've started the possession sequence

		FLAG_NONE				= 0x00000000
	};




//----------------------------------------------------------------------------------------------------------------------------------
// Private Data:
//----------------------------------------------------------------------------------------------------------------------------------
private:

	static BOOL m_bSystemInitialized;
	static SmokeTrailAttrib_t m_SmokeTrailAttribChunk;
	static SmokeTrailAttrib_t m_SmokeTrailAttribSever;
	static FLinkRoot_t m_LinkRoot;
	static CFVec3A m_aPentagonVtx[6];		// The 5 corners that define a pentagon (origin is centroid) ([5] == [0])
	static f32 m_afPentagonVtxIntensity[6]; // Vertex light intensity on the pentagon vertices
	static CFTexInst m_GlowTexInst;			// The texture for the cable glow card
	static CFTexInst m_CableTexInst;		// The texture for the inside of the cable
	static CFTexInst m_DustTexInst;			// The texture for the dust particle
	static CFTexInst m_MilLogoTexInst;		// The texture for the Mil logo
	static FMesh_t *m_pDebrisMesh;			// Debris mesh (NULL=none)

	static CTether *m_pCallback_Tether;		// This tether
	static CBot *m_pCallback_ClosestBot;	// Closest bot found (NULL=none)
	static f32 m_fCallback_ClosestDist2;	// Closest distance squared
	static CFVec3A m_Callback_CollPoint;	// Collision point

	FLink_t m_Link;							// Link to other tethers

	u32 m_nMaxVtxCount;						// Maximum number of vertices in the tether
	u32 m_nVtxCount;						// Number of vertices currently being drawn
	u32 m_nVtxPoolCount;					// The number of vertices we need in our draw-vertex pool
	CFVec3A *m_pPosArray_MS;				// Position array ([0] is the vertex at the projectile)
	CFVec3A *m_pPSPosArray_MS;				// Point sprite position array (for dust effect)
	CFVec3A *m_pPSVelArray_MS;				// Point sprite velocity array (for dust effect)

	u32 m_nFlags;							// See FLAG_* for info
	State_e m_nState;						// Tether state
	FForceHandle_t m_hForce;				// For force feedback of zap

	CEntity *m_pProjEntity;					// Projectile entity
	CEntity *m_pSourceDummyEntity;			// Dummy entity attached to the source entity
	CEntity *m_pTargetDummyEntity;			// Dummy entity attached to the targeted entity (NULL=none)
	CWeaponTether *m_pWeaponTether;			// The tether weapon that fired this tether
	const UserProps_t *m_pUserProps;		// User props for this tether

	PossessMode_e m_nPossessMode;			// Possession mode
	CCamSimpleInfo m_CamInfo;				// Camera info
	CFMtx43A m_DeltaCamMtx;					// Matrix that converts the owner bot's m_MtxToWorld to the camera matrix
	f32 m_fUnitCamAnim;						// Used to animate the camera
	f32 m_fStartHalfFOV;					// The half-FOV of the camera when we take control
	f32 m_fUnitDrawMilLogo;					// How prevelent the Mil logo is

	CBot *m_pTargetBot;						// The bot we're targeting (NULL=none)
	CBot *m_pSourceBot;						// The bot that fired the tether (NULL=none)

	f32 m_fPossessSpeed;					// Speed multiplier to make possession faster

	f32 m_fSpawnSparksSecs;					// Seconds remaining until we spawn more sparks

	f32 m_fUnitGoal;						// Unit goal from source to target
	f32 m_fUnitGoalSpeed;					// Speed at which m_fUnitGoal changes
	f32 m_fLaunchUnitRandomFactor1;			// Set to a random value from 0 to 1 when Launch() is called
	f32 m_fLaunchUnitRandomFactor2;			// Set to a random value from 0 to 1 when Launch() is called

	f32 m_fMaxTetherStretchDist;			// Max distance an attached tether can be stretched before destroying itself
	f32 m_fMaxFlyDist;						// max distance a flying tether can be from its source before destroying itself
	f32 m_fMaxTetherLength;					// Maximum length of tether cable before it detaches from its source
	f32 m_fCurrentTetherLength;				// Current tether length
	f32 m_fDistanceBetweenVtx;				// Distance between vertices

	f32 m_fUnitSourcePinchDisable;			// 1=don't do source pinching, 0=do full source pinching
	f32 m_fVtxUngulateAngle;				// Used to ungulate the flying vertices

	f32 m_fUnitTighten;						// Amount cable is tightened
	f32 m_fTightenOOVtxCountMinusOne;		// When Tighten() is called this is set to 1.0f/(m_nVtxCount-1)

	f32 m_fTimeUntilZapOn;					// Time remaining until the zap effect gets turned on
	f32 m_fZapPulseDist;					// Distance the zap pulse has traveled
	f32 m_fZappingTimeRemaining;			// While fully zapping, this counts down

	CFPSpriteGroup m_PSGroup;				// Our point sprite group for the dust
	f32 m_fUnitDustRemaining;				// While dust is in the air, this decreases from 1 to 0 over the lifetime of the particles
	u32 m_nPSDrawCount;						// Number of point sprites to be drawn

	CFWorldMesh *m_pDebrisWorldMeshArray;	// Array of cable debris pieces
	CFMotionSimple *m_pDebrisMotionArray;	// Array of debris motion data
	u32 m_nMaxDebrisChunckCount;			// Number of elements in m_pDebrisArray
	u32 m_nDebrisChunkDrawCount;			// Number of debris chunks to draw

	CFMtx43A m_ModelMtx;					// The matrix used to draw the tether
	CFVec3A m_Velocity_WS;					// Current velocity




//----------------------------------------------------------------------------------------------------------------------------------
// Public Functions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	// System:
	CTether();
	~CTether();

	static BOOL InitSystem( void );
	static void UninitSystem( void );

	static void DrawAll( void );
	static void WorkAll( void );

	BOOL Create( u32 nMaxVtxCount );
	void Destroy( void );
	FINLINE BOOL IsCreated( void ) const { return m_nFlags & FLAG_CREATED; }

	FINLINE BOOL IsDestroyed( void ) const { FASSERT( IsCreated() ); return m_nState == STATE_DESTROYED; }
	FINLINE BOOL IsAttachedToTarget( void ) const { FASSERT( IsCreated() ); return m_nState == STATE_ZAPPING; }
	FINLINE BOOL IsFiringZapPulse( void ) const { FASSERT( IsCreated() ); return IsAttachedToTarget() && (m_fTimeUntilZapOn == 0.0f) && (m_fZapPulseDist > 0.0f); }
	FINLINE BOOL IsZapping( void ) const { FASSERT( IsCreated() ); return IsAttachedToTarget() && (m_fTimeUntilZapOn == 0.0f) && (m_fZappingTimeRemaining > 0.0f); }
	FINLINE BOOL IsDoneZapping( void ) const { FASSERT( IsCreated() ); return IsAttachedToTarget() && (m_fTimeUntilZapOn == 0.0f) && (m_fZappingTimeRemaining == 0.0f); }
	FINLINE BOOL IsDetachedFromSource( void ) const { FASSERT( IsCreated() ); return m_nFlags & FLAG_DETACHED; }

	FINLINE void SetProjEntity( CEntity *pEntity ) { FASSERT( IsCreated() ); m_pProjEntity = pEntity; }
	FINLINE CEntity *GetProjEntity( void ) const { FASSERT( IsCreated() ); return m_pProjEntity; }

	void Launch( CWeaponTether *pWeaponTether, cchar *pszFireEmitterBoneName, CBot *pTargetBot, const CFVec3A *pTargetPos_WS, f32 fMaxTetherLength, f32 fMaxTetherStretchDist, f32 fMaxFlyDist, f32 fPossessSpeed, const UserProps_t *pUserProps );
	void Sever( void );
	void Kill( BOOL bSupressEffects=FALSE );




//----------------------------------------------------------------------------------------------------------------------------------
// Protected Functions:
//----------------------------------------------------------------------------------------------------------------------------------
protected:




//----------------------------------------------------------------------------------------------------------------------------------
// Private Functions:
//----------------------------------------------------------------------------------------------------------------------------------
private:

	static void _SetSmokeTrailAttributes( void );

	void _ClearDataMembers( void );

	void _Draw( void );
	void _DrawCableGlow( FDrawVtx_t *pVtxArray );
	void _DrawInside( FDrawVtx_t *pVtxArray );
	void _DrawOutside( FDrawVtx_t *pVtxArray );
	void _DrawFibers( FDrawVtx_t *pVtxArray );
	void _DrawDust( void );
	static void _DrawMilLogoOverlay( u32 nPlayerIndex, void *pUser );
	void _SpawnChunkImpactEffects( const CFVec3A *pPos_WS, const CFVec3A *pUnitDir_WS );

	void _Zap( void );
	void _Work_Flying( void );
	void _Work_Zapping( void );
	void _Work_DebrisDust( void );
	void _Work_DebrisChunks( void );
	void _Work_PossessMode_FlyAlongCable( void );
	void _Work_PossessMode_FadeInBehindBot( void );
	void _Work_PossessMode_PoweringUpBot( void );

	void _EmergencyDisconnect( void );

	BOOL _HandleTetherProjectileCollision( FCollImpact_t *pCollImpact );
	static BOOL _FindTrackersInRangeCallback( CFWorldTracker *pTracker, FVisVolume_t *pVolume );
	void _BuildTrackerSkipList( void );

	FCLASS_STACKMEM_ALIGN( CTether );
} FCLASS_ALIGN_SUFFIX;




#endif

