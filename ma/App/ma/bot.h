//////////////////////////////////////////////////////////////////////////////////////
// bot.h - Bot module.
//
// Author: Steve Ranck     
//////////////////////////////////////////////////////////////////////////////////////
// THIS CODE IS PROPRIETARY PROPERTY OF SWINGIN' APE STUDIOS, INC.
// Copyright (c) 2001
//
// The contents of this file may not be disclosed to third
// parties, copied or duplicated in any form, in whole or in part,
// without the prior written permission of Swingin' Ape Studios, Inc.
//////////////////////////////////////////////////////////////////////////////////////
// Modification History:
//
// Date     Who         Description
// -------- ----------  --------------------------------------------------------------
// 02/18/02 Ranck       Created.
//////////////////////////////////////////////////////////////////////////////////////
#ifndef _BOT_H_
#define _BOT_H_ 1

#include "fang.h"
#include "fmath.h"
#include "fworld.h"
#include "entity.h"
#include "botanim.h"
#include "fgamedata.h"
#include "fcoll.h"
#include "gcoll.h"

class CBotPart;
class CBotPartMgr;
class CWeapon;
class CMeshEntity;
class CZipLineHook;
class CEZipLine;
class CVehicle;
class CGCollMaterial;
class CEShield;
class CFDebrisGroup;
class CHud2;
class CBotPowerupFx;
class CFSoundGroup;
class CBotDispenser;


#define ACTION_BUTTON_TEST_RADIUS	( 4.0f )

#define BOT_MIN_TETHER_DIST			100.0f

#define BOT_HEAD_LOOK_HYSTERESIS_ANGLE	( FMATH_DEG2RAD( 15.0f ) )
#define BOT_HEAD_LOOK_TIME				( 0.3f )
#define BOT_HEAD_LOOK_INV_TIME			( 1.0f / BOT_HEAD_LOOK_TIME )
#define BOT_ZIPLINE_GRAB_TIME			( 3.0f ) // 3 seconds before you can re-attach to the same zip line
#define BOT_REVERSE_FACING_RATE			( FMATH_DEG2RAD( 360.0f ) )	// radians/sec
#define BOT_MAX_DUPLICATE_WPNS			3

#define BOT_SOUND_SCUFF_MIN_TIME		1.0f
#define BOT_SOUND_SCUFF_MAX_TIME		2.0f

#define NEUTRAL_TEAM					4	// This should be at least MAX_PLAYERS

#define _stringize( L )		#L
#define _makestring( M, L )	M(L)
#define _line				_makestring( _stringize, __LINE__ )
#define msg(str)			message( __FILE__ "(" _line ") : " str)

typedef enum {
	BOTRACE_MIL=0,	// ASSERT(BOTRACE_MIL     == AIRACE_MIL)
	BOTRACE_DROID,	// ASSERT(BOTRACE_DROID   == AIRACE_DROID)
	BOTRACE_ZOMBIE,	// ASSERT(BOTRACE_ZOMBIE  == AIRACE_ZOMBIE)
	BOTRACE_AMBIENT,// ASSERT(BOTRACE_AMBIENT == AIRACE_AMBIENT)
	
	BOTRACE_COUNT	// ASSERT(BOTRACE_COUNT  == NUM_AIRACES)
} BotRace_e;


typedef enum {
///////////////////////////////////////////
// NOTE: IF YOU ADD, DELETE, REARRANGE THIS
// TABLE, YOU MUST FIX ALL ARRAYS BASED ON
// BOTCLASS_COUNT AND B_CAM_DATA.CSV FILE
///////////////////////////////////////////
	// Droids:
	BOTCLASS_MINER,
	BOTCLASS_COLOSSUS,
	BOTCLASS_SCIENTIST,
	BOTCLASS_CHEMBOT,
	BOTCLASS_ALLOY,
	BOTCLASS_ZOBBY,
	BOTCLASS_BARTER,
	BOTCLASS_VERMIN,
	BOTCLASS_AMBIENT,
	BOTCLASS_KRUNK,

	// Mils:
	BOTCLASS_GRUNT,
	BOTCLASS_ELITE_GUARD,
	BOTCLASS_JUMP_TROOPER,
	BOTCLASS_TITAN,
	BOTCLASS_SNIPER,
	BOTCLASS_SWARMER,
	BOTCLASS_SWARMER_BOSS,
	BOTCLASS_PREDATOR,
	BOTCLASS_SITEWEAPON,
	BOTCLASS_MORTAR,
	BOTCLASS_CORROSIVE,
	BOTCLASS_LOADER,
	BOTCLASS_RAT,
	BOTCLASS_SENTINEL,
	BOTCLASS_PROBE,
	BOTCLASS_SCOUT,
	BOTCLASS_AAGUN,
	BOTCLASS_SNARQ,

	// Zombies:
	BOTCLASS_ZOMBIE,
	BOTCLASS_ZOMBIE_BOSS,

///////////////////////////////////////////
// NOTE: IF YOU ADD, DELETE, REARRANGE THIS
// TABLE, YOU MUST FIX ALL ARRAYS BASED ON
// BOTCLASS_COUNT AND B_CAM_DATA.CSV FILE
///////////////////////////////////////////
	BOTCLASS_COUNT
} BotClass_e;


typedef enum {
	BOTSUBCLASS_NONE,

	BOTSUBCLASS_MINER_BASE,
		BOTSUBCLASS_MINER_GENERIC = BOTSUBCLASS_MINER_BASE,
		BOTSUBCLASS_MINER_BLINK,
	BOTSUBCLASS_MINER_CAP,


	BOTSUBCLASS_COLOSSUS_BASE,
		BOTSUBCLASS_COLOSSUS_GENERIC = BOTSUBCLASS_COLOSSUS_BASE,
		BOTSUBCLASS_COLOSSUS_MOZER,
	BOTSUBCLASS_COLOSSUS_CAP,


	BOTSUBCLASS_SCIENTIST_BASE,
		BOTSUBCLASS_SCIENTIST_GENERIC = BOTSUBCLASS_SCIENTIST_BASE,
		BOTSUBCLASS_SCIENTIST_EXAVOLT,
	BOTSUBCLASS_SCIENTIST_CAP,
	

	BOTSUBCLASS_CHEMBOT_BASE,
		BOTSUBCLASS_CHEMBOT_GENERIC = BOTSUBCLASS_CHEMBOT_BASE,
		BOTSUBCLASS_CHEMBOT_SLOSH,
	BOTSUBCLASS_CHEMBOT_CAP,
	

	BOTSUBCLASS_BARTER_BASE,
		BOTSUBCLASS_BARTER_SLIM = BOTSUBCLASS_BARTER_BASE,
		BOTSUBCLASS_BARTER_SHADY,
	BOTSUBCLASS_BARTER_CAP,
	

	BOTSUBCLASS_SITEWEAPON_BASE,
		BOTSUBCLASS_SITEWEAPON_FLOORSENTRY = BOTSUBCLASS_SITEWEAPON_BASE,
		BOTSUBCLASS_SITEWEAPON_WALLSENTRY,
		BOTSUBCLASS_SITEWEAPON_PILLBOX,
		BOTSUBCLASS_SITEWEAPON_RATGUN,
	BOTSUBCLASS_SITEWEAPON_CAP,
	

	BOTSUBCLASS_GRUNT_BASE,
		BOTSUBCLASS_GRUNT_NORMAL = BOTSUBCLASS_GRUNT_BASE,
		BOTSUBCLASS_GRUNT_SCIENTIST,
		BOTSUBCLASS_GRUNT_VLAX,
	BOTSUBCLASS_GRUNT_CAP,
	

	BOTSUBCLASS_TITAN_BASE,
		BOTSUBCLASS_TITAN_NORMAL = BOTSUBCLASS_TITAN_BASE,
	BOTSUBCLASS_TITAN_CAP,
	

	BOTSUBCLASS_PREDATOR_BASE,
		BOTSUBCLASS_PREDATOR_NORMAL = BOTSUBCLASS_PREDATOR_BASE,
	BOTSUBCLASS_PREDATOR_CAP,
	

	BOTSUBCLASS_PROBE_BASE,
		BOTSUBCLASS_PROBE_NORMAL = BOTSUBCLASS_PROBE_BASE,
	BOTSUBCLASS_PROBE_CAP,


	BOTSUBCLASS_SWARMER_BASE,
		BOTSUBCLASS_SWARMER_NORMAL = BOTSUBCLASS_SWARMER_BASE,
		BOTSUBCLASS_SWARMER_VERMIN,
	BOTSUBCLASS_SWARMER_CAP,


	BOTSUBCLASS_ZOMBIE_BASE,
		BOTSUBCLASS_ZOMBIE_NORMAL = BOTSUBCLASS_ZOMBIE_BASE,
	BOTSUBCLASS_ZOMBIE_CAP,


	BOTSUBCLASS_ZOMBIEBOSS_BASE,
		BOTSUBCLASS_ZOMBIEBOSS_NORMAL = BOTSUBCLASS_ZOMBIEBOSS_BASE,
	BOTSUBCLASS_ZOMBIEBOSS_CAP,


	BOTSUBCLASS_KRUNK_BASE,
		BOTSUBCLASS_KRUNK_NORMAL = BOTSUBCLASS_KRUNK_BASE,
	BOTSUBCLASS_KRUNK_CAP,


	BOTSUBCLASS_JUMPTROOPER_BASE,
		BOTSUBCLASS_JUMPTROOPER_NORMAL = BOTSUBCLASS_JUMPTROOPER_BASE,
	BOTSUBCLASS_JUMPTROOPER_CAP,
	

	BOTSUBCLASS_MORTAR_BASE,
		BOTSUBCLASS_MORTAR_GENERIC = BOTSUBCLASS_MORTAR_BASE,
	BOTSUBCLASS_MORTAR_CAP,


	BOTSUBCLASS_SNIPER_BASE,
		BOTSUBCLASS_SNIPER_GENERIC = BOTSUBCLASS_SNIPER_BASE,
	BOTSUBCLASS_SNIPER_CAP,

	BOTSUBCLASS_AAGUN_BASE,
		BOTSUBCLASS_AAGUN_NORMAL = BOTSUBCLASS_AAGUN_BASE,
	BOTSUBCLASS_AAGUN_CAP,

	BOTSUBCLASS_BOTSNARQ_BASE,
		BOTSUBCLASS_BOTSNARQ_NORMAL = BOTSUBCLASS_BOTSNARQ_BASE,
	BOTSUBCLASS_BOTSNARQ_CAP,



	BOTSUBCLASS_COUNT,

	BOTSUBCLASS_MINER_COUNT			= BOTSUBCLASS_MINER_CAP - BOTSUBCLASS_MINER_BASE,
	BOTSUBCLASS_COLOSSUS_COUNT		= BOTSUBCLASS_COLOSSUS_CAP - BOTSUBCLASS_COLOSSUS_BASE,
	BOTSUBCLASS_SCIENTIST_COUNT 	= BOTSUBCLASS_SCIENTIST_CAP - BOTSUBCLASS_SCIENTIST_BASE,
	BOTSUBCLASS_CHEMBOT_COUNT		= BOTSUBCLASS_CHEMBOT_CAP - BOTSUBCLASS_CHEMBOT_BASE,
	BOTSUBCLASS_BARTER_COUNT		= BOTSUBCLASS_BARTER_CAP - BOTSUBCLASS_BARTER_BASE,
	BOTSUBCLASS_SITEWEAPON_COUNT	= BOTSUBCLASS_SITEWEAPON_CAP - BOTSUBCLASS_SITEWEAPON_BASE,
	BOTSUBCLASS_GRUNT_COUNT			= BOTSUBCLASS_GRUNT_CAP - BOTSUBCLASS_GRUNT_BASE,
	BOTSUBCLASS_TITAN_COUNT			= BOTSUBCLASS_TITAN_CAP - BOTSUBCLASS_TITAN_BASE,
	BOTSUBCLASS_PREDATOR_COUNT		= BOTSUBCLASS_PREDATOR_CAP - BOTSUBCLASS_PREDATOR_BASE,
	BOTSUBCLASS_PROBE_COUNT			= BOTSUBCLASS_PROBE_CAP - BOTSUBCLASS_PROBE_BASE,
	BOTSUBCLASS_SWARMER_COUNT		= BOTSUBCLASS_SWARMER_CAP - BOTSUBCLASS_SWARMER_BASE,
	BOTSUBCLASS_ZOMBIE_COUNT		= BOTSUBCLASS_ZOMBIE_CAP - BOTSUBCLASS_ZOMBIE_BASE,
	BOTSUBCLASS_ZOMBIEBOSS_COUNT	= BOTSUBCLASS_ZOMBIEBOSS_CAP - BOTSUBCLASS_ZOMBIEBOSS_BASE,
	BOTSUBCLASS_KRUNK_COUNT			= BOTSUBCLASS_KRUNK_CAP - BOTSUBCLASS_KRUNK_BASE,
	BOTSUBCLASS_JUMPTROOPER_COUNT	= BOTSUBCLASS_JUMPTROOPER_CAP - BOTSUBCLASS_JUMPTROOPER_BASE,
	BOTSUBCLASS_MORTAR_COUNT		= BOTSUBCLASS_MORTAR_CAP - BOTSUBCLASS_MORTAR_BASE,
	BOTSUBCLASS_AAGUN_COUNT			= BOTSUBCLASS_AAGUN_CAP - BOTSUBCLASS_AAGUN_BASE,
	BOTSUBCLASS_BOTSNARQ_COUNT		= BOTSUBCLASS_BOTSNARQ_CAP - BOTSUBCLASS_BOTSNARQ_BASE,

} BotSubClass_e;



FCLASS_NOALIGN_PREFIX class CBotDef {
public:

	BotRace_e m_nRace;						// This bot's race
	BotClass_e m_nClass;					// This bot's class
	BotSubClass_e m_nSubClass;				// This bot's subclass


	FCLASS_STACKMEM_ALIGN( CBotDef );
} FCLASS_NOALIGN_SUFFIX;




// Forward declared for the pointer in CBot.
class CInventory;
class CBotBuilder;
class CBotTalkData;
class CBotTalkInst;


//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CBot
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************
typedef BOOL UserAnimAbortCB(CBot* pBot);
typedef void AbortCompleteCB(CBot* pBot);

FCLASS_ALIGN_PREFIX class CBot : public CEntity {
//----------------------------------------------------------------------------------------------------------------------------------
// Public Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	enum {
		BONE_GEO_DEBRIS_GROUP_TIMED,
		BONE_GEO_DEBRIS_GROUP_IMMEDIATE,
		BONE_GEO_DEBRIS_GROUP_IMPACT,

		BONE_GEO_DEBRIS_GROUP_COUNT
	};


	typedef enum {
		LIMB_CODE_HEAD,
		LIMB_CODE_TORSO,
		LIMB_CODE_ARMS,
		LIMB_CODE_LEGS,
		LIMB_CODE_WINGS,
		LIMB_CODE_ROCKET_LAUNCHER,
		LIMB_CODE_SPOTLIGHT,
		LIMB_CODE_MISC,					// Ribs, etc.

		LIMB_CODE_COUNT
	} LimbCode_e;


	typedef struct {
		f32 fGravity;

		f32 fCollSphere1X_MS;
		f32 fCollSphere1Y_MS;
		f32 fCollSphere1Z_MS;
		f32 fCollSphere1Radius_MS;

		cchar *pszDataPortBoneName;						// NULL=Bot cannot have a data port
		cchar *apszDataPortMeshName[3];					// [0]=closed, [1]=opened, [2]=effect   (NULL=none)

		f32 fPowerDownSpeedMult;						// Anim speed multiplier applied to the power-down animation
		f32 fPowerUpSpeedMult;							// Anim speed multiplier applied to the power-up animation
		f32 fDisguiseTime;								// Amount of time a possessed bot will be disguised
		f32 fPossessionDist;							// Maximum distance a possessed bot can travel
		cchar *pszWeightClass;							// Weight class of the bot: 'light', 'medium' and 'heavy'

		u32 nBatteryCount;								// Number of batteries (health containers) for this bot
		cchar *pszArmorProfile;							// The default armor profile to use with this bot

		f32 fUnitDustKickup;							// How much dust the bot can kick up with each foot step (0=none)
		f32 fUnitShockwaveEffect;						// Unit amount this bot will be affected by shockwaves

		f32 fPhysForce_Land;							// Force applied when landing on a phys object
		f32 fPhysForce_Stand;							// Force applied when standing on a phys object
		f32 fPhysForce_Step;							// Force applied with each step taken on a phys object
		f32 fPhysForce_Lateral;							// Force applied laterally when translating on a phys object

		CFDebrisGroup *apBoneGeoDebrisGroup[BONE_GEO_DEBRIS_GROUP_COUNT];

		CFDebrisGroup *pDebrisGroup_Chunks;				// Debris group for this bot's chunks (NULL=none)
		CFDebrisGroup *pDebrisGroup_Guts;				// Debris group for this bot's guts (gears, bolts, etc.) (NULL=none)
		FSndFx_FxHandle_t hFlamingDebrisBurstSound;		// Sound to make when limb's pieces get blown into flaming debris

		union {
			cchar *pszDebrisBurstImpulseDamageProfile;			// Data file: Impulse damage profile to use when a limb bursts into flaming debris
			CDamageProfile *pDebrisBurstImpulseDamageProfile;	// In-game: Impulse damage profile to use when a limb bursts into flaming debris
		};

		f32 fMinBoneGeoDebrisSpeed;
		f32 fMaxBoneGeoDebrisSpeed;
		f32 fUnitBoneGeoSpread;

		f32 fMinChunksDebrisSpeed;
		f32 fMaxChunksDebrisSpeed;
		f32 fUnitChunksSpread;

		f32 fMinGutsDebrisSpeed;
		f32 fMaxGutsDebrisSpeed;
		f32 fUnitGutsSpread;

		cchar *pszRightLegLimbName;						// Both of these must be valid to blow off legs
		cchar *pszLeftLegLimbName;
		FMesh_t *pDebrisPileMesh;						// Optional debris pile mesh for bot to sit upon when he has no legs
		f32 fNoLegsDeltaCollSphereY_MS;					// When the bot has no legs, this is how much we adjust its collision sphere

		cchar *pszRootBoneName;							// Bone name of root bone used for total destruction of the bot (NULL=none)
		cchar *pszTorsoBoneName;						// Bone name of torso bone used for partial destruction of the bot (NULL=none)

		union {
			cchar *pszTorsoBlowOffArmorProfile;			// Data file: Armor profile to use to determine whether to blow off a bot's torso
			CArmorProfile *pTorsoBlowOffArmorProfile;	// In-game: Armor profile to use to determine whether to blow off a bot's torso
		};

		u32 uLoaderPickupEnabled;						// can the loader pick this bot up? 0 or 1
		f32	fBotCollisionMass;							// relative mass for bot-bot collisions
		u32 uNumHitsToBreakLimb;						// how many blows it takes to break one of the bot's limbs

		f32 fDoubleJumpTime;							// Time frame in which we are allowed to double jump
	} BotInfo_Gen_t;

	typedef struct {
		cchar*	pszIdleName;
		f32		fIdleToIdleDeltaSpeed;					// One over the blend time from default anim to full idle animation at start of idle state.. .(ie 10 means it takes 1/10 of a second)
		f32		fIdleMinimumDelay;						// how long it takes to enter the idle animation
		f32		fIdleProbability;						// the per-frame probability of starting THIS idle animation AFTER fIdleMinimumDelay time period has passed.
	} BotInfo_Idle_t;


	// Bone mask groups for user anims:
	typedef enum {
		UABONEMASK_ENABLE_UPPER_BODY,		// Enable driving of entire upper body (includes upper torso, both arms, and head)
		UABONEMASK_ENABLE_LOWER_BODY,		// Enable driving of entire lower body

		UABONEMASK_ENABLE_UPPER_TORSO,		// Enable driving of upper torso (excludes both arms and head)
		UABONEMASK_ENABLE_LOWER_TORSO,		// Enable driving of lower torso
		UABONEMASK_ENABLE_LEFT_ARM,			// Enable driving of left arm
		UABONEMASK_ENABLE_RIGHT_ARM,		// Enable driving of right arm
		UABONEMASK_ENABLE_HEAD,				// Enable driving of head

		UABONEMASK_COUNT
	} UserAnimBoneMask_e;


	// Body parts:
	typedef enum {
		BODYPART_TYPE_HEAD,
		BODYPART_TYPE_TORSO,
		BODYPART_TYPE_WAIST,
		BODYPART_TYPE_RIGHT_ARM_UPPER,
		BODYPART_TYPE_RIGHT_ARM_LOWER,
		BODYPART_TYPE_LEFT_ARM_UPPER,
		BODYPART_TYPE_LEFT_ARM_LOWER,
		BODYPART_TYPE_RIGHT_LEG_UPPER,
		BODYPART_TYPE_RIGHT_LEG_LOWER,
		BODYPART_TYPE_RIGHT_FOOT,
		BODYPART_TYPE_LEFT_LEG_UPPER,
		BODYPART_TYPE_LEFT_LEG_LOWER,
		BODYPART_TYPE_LEFT_FOOT,

		BODYPART_TYPE_COUNT
	} BodyPartType_e;


	// World surface types (Due to screwy access privaleges on GC, this needs to be in the public section)
	// The order of these matters.  If changed, change CBot::_LoadSoundResource()
	typedef enum { 
		SURFACE_TYPE_NONE     = -1,
		SURFACE_TYPE_CONCRETE = 0,
		SURFACE_TYPE_METAL,
		SURFACE_TYPE_METAL_GRATE,
		SURFACE_TYPE_DIRT,
		SURFACE_TYPE_ROCK,
		SURFACE_TYPE_GLASS,
		SURFACE_TYPE_COMPOSITE,
		SURFACE_TYPE_ELECTRONICS,
		SURFACE_TYPE_JUNK,
		SURFACE_TYPE_WATER,
		SURFACE_TYPE_GOOP,
		SURFACE_TYPE_ACID,
		SURFACE_TYPE_FORCE_FIELD,
		SURFACE_TYPE_COUNT,
	} SurfaceType_e;


	// Bot sound information
	enum { 
		BOTSOUND_LIGHT_WEIGHT			= 1,
		BOTSOUND_MEDIUM_WEIGHT			= 2,
		BOTSOUND_HEAVY_WEIGHT			= 4,
		BOTSOUND_CORROSIVE_WEIGHT		= 8,
		BOTSOUND_ZOMBIEBOSS_WEIGHT		= 16,
		BOTSOUND_VEHICLE_WEIGHT			= 32,
		BOTSOUND_NUM_SOUNDS_PER_SURFACE = 12,
		BOTSOUND_SCUFF_SOUND_START      = 9,
	};

	// bot weight class
	typedef enum {
		BOTWEIGHT_LIGHT,
		BOTWEIGHT_MEDIUM,
		BOTWEIGHT_HEAVY,
		BOTWEIGHT_CORROSIVE,
		BOTWEIGHT_ZOMBIEBOSS,
		BOTWEIGHT_VEHICLE,
		BOTWEIGHT_COUNT
	} BotWeight_e;


	typedef enum {
		MELEE_MAX_ENTITIES_PER_SWING			= 4,
	};



//----------------------------------------------------------------------------------------------------------------------------------
// Protected Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
protected:

	ENTITY_CLASS_HIERARCHY_BITDEF


	// General definitions:
	enum {
		TETHER_RANDOM_MTX_COUNT = 10
	};


	// Bot flags:
	enum {
		BOTFLAG_ENEMY						= 0x00000001,	// TRUE if this bot is an enemy (will never change polarity for a particular bot)
		BOTFLAG_USE_MESH_FOR_COLLISION		= 0x00000002,	// TRUE when bot should ignore his bot collision sphere and use mesh rep instead
		BOTFLAG_XLAT_BACKWARDS				= 0x00000004,	// TRUE if this bot is moving backwards
		BOTFLAG_TARGET_LOCKED				= 0x00000008,	// TRUE when m_TargetLockUnitVec_WS is pointing to an aquired target
		BOTFLAG_SNEAKING					= 0x00000010,	// TRUE if the bot is in sneak mode
		BOTFLAG_VELOCITY_IMPULSE_VALID		= 0x00000020,	// TRUE when m_ImpulseVelocity_WS is non-zero
		BOTFLAG_PLAY_FIRE1_ANIM				= 0x00000040,	// TRUE when we're playing the primary fire anim
		BOTFLAG_YAW_CONTROL_OVERDRIVE		= 0x00000080,	// TRUE when player is rotating in overdrive mode
		BOTFLAG_IMMOBILIZE_PENDING			= 0x00000100,	// TRUE when bot is trying to become immobilized (might be airborne, not immobilized until on ground and stopped)
		BOTFLAG_IS_IMMOBILE					= 0x00000200,	// TRUE when the bot is immobile
		BOTFLAG_IS_BEING_SHOCKED			= 0x00000400,	// TRUE when the bot is being shocked by the tether
		BOTFLAG_PLAY_TETHER_VIBRATE			= 0x00000800,	// TRUE when the bot is tether-shocking another bot (used in conjunction with BOTFLAG_PLAY_FIRE1_ANIM)
		BOTFLAG_CLIP_IS_SLAPPED_IN			= 0x00001000,	// TRUE when the clip of a clip-based weapon has been slapped in
		BOTFLAG_USE_HEAD_LOOK				= 0x00002000,	// TRUE when bot should try to point head in direction of m_vHeadLookPoint_WS
		BOTFLAG_HEAD_IS_LOOKING				= 0x00004000,	// TRUE when bot's head is actually looking in direction of m_vHeadLookPoint_WS
		BOTFLAG_FORCED_PANIC				= 0x00008000,	// TRUE when Bot has been put in panic by some means other than through the CEntityControls interface
		BOTFLAG_FORCED_ALERT				= 0x00010000,	// TRUE when Bot has been put in alert mode by some means other than through the CEntityControls interface
		BOTFLAG_REVERSE_FACING				= 0x00020000,	// TRUE when reverse-facing mode is engaged (body facing 180 degrees to mount)
		BOTFLAG_HIPSLACK_OVERRIDE			= 0x00040000,	// To be set every frame by clients if the don't wan't any hip slack (hip slack allows bot's lower boddy to not point forward when he stopped)
		BOTFLAG_GLUED_TO_PARENT				= 0x00080000,	// TRUE when bot is attached to something well enough that the ground won't reparent
		BOTFLAG_IS_STUNNED					= 0x00100000,	// TRUE when the bot has been stunned and will have their view modified
		BOTFLAG_DONT_PICKUP_ITEMS			= 0x00200000,	// TRUE when the bot should not pick up items in the world (washers, etc)
		BOTFLAG_NO_GRAVITY					= 0x00400000,	// TRUE when we don't want to apply gravity to the bot (used for Probe attack when caught in air)
		BOTFLAG_NO_LEGS						= 0x00800000,	// TRUE when the bot has no legs
		BOTFLAG_NO_FEET_ANIM				= 0x01000000,	// TRUE when we want to force the feet to not animate
		BOTFLAG_SKIP_BOT_V_BOT_COLLISIONS	= 0x02000000,	// TRUEN when we want this bot to be impervious to bot-v-bot collisions
		BOTFLAG_IS_SLOWED					= 0x04000000,	// TRUE when the bot has been slowed 
		BOTFLAG_IS_INVERTED					= 0x08000000,	// TRUE when the bot is upside down
		// available bit					= 0x10000000,	// 
		BOTFLAG_IGNORE_CONTROLS				= 0x20000000,	// TRUE when bot is not interested in controls (when dying, for example)
		BOTFLAG_NO_COLLIDE_STATE_AIR		= 0x40000000,	// TRUE when bot should be in STATE_AIR when collision flag is not set
		BOTFLAG_TORSO_IS_BLOWN_OFF			= 0x80000000,	// TRUE when the bot's torso is blown off
		// ALL OUT OF FLAGS, START USING BOTFLAG2...

		BOTFLAG_NONE						= 0x00000000
	};


	enum {
		BOTFLAG2_UNDER_CONSTRUCTION			= 0x00000001,	// TRUE when the bot is under construction
		BOTFLAG2_TAG_POINT_IS_BOUND_CENTER	= 0x00000002,	// TRUE when we're to use the mesh's bounding sphere center as the one and only tag point
		BOTFLAG2_DISABLE_AIMING				= 0x00000004,	// if TRUE, bot will not aim weapon at target (blends quickly out of bone overrides)
		BOTFLAG2_DATAPORT_RESERVED			= 0x00000008,	// This dataport is temporarily out of service, please try again later
		BOTFLAG2_DROPPED_WEAPON				= 0x00000010,	// TRUE when the bot has already dropped its weapon
		BOTFLAG2_CAN_DOUBLE_JUMP			= 0x00000020,	// TRUE when the bot is allowed to perform a double-jump
		BOTFLAG2_POWERDOWN_UNTIL_POSESSED	= 0x00000040,	// The bot will powerdown if it isn't posessed by a player
		BOTFLAG2_INST_CANNOT_BE_RECRUITED	= 0x00000080,	// This bot instance cannot be recruited by the recruiter grenade
		BOTFLAG2_CLASS_CAN_BE_RECRUITED		= 0x00000100,	// This class of bots can be recruited
		BOTFLAG2_DPORT_NOT_NEEDED_FOR_RECRUITMENT	= 0x00000200,	// This class of bots can be recruited without a data port (BOTFLAG2_CLASS_CAN_BE_RECRUITED needs to be set as well)
		BOTFLAG2_DONT_USE_IDLE_ANIMATIONS	= 0x00000400,	// TRUE when we don't want the bot to use any of its idle animations.
		BOTFLAG2_DONT_ALLOW_FIRE			= 0x00000800,	// TRUE when we don't want to the bot to fire at all.
		BOTFLAG2_SHOWS_UP_ON_RADAR			= 0x00001000,	// TRUE if this bot is to show up on Glitch's radar
		BOTFLAG2_CONSOLE_POSSESSED			= 0x00002000,	// TRUE if this bot is being possessed via a console
		BOTFLAG2_NO_POSSESSION_EXIT			= 0x00004000,	// TRUE if player cannot exit this bot after possessing it
		BOTFLAG2_BABBLE						= 0x00008000,	// TRUE if the bot should babble (distorted voice) all the time

		BOTFLAG2_NONE						= 0x00000000
	};


	// Bot flags for camera:
	enum {
		BOTCAMERAFLAG_OVERRIDDEN			= 0x00000001,	// TRUE when the cambot's values should be overriden
		BOTCAMERAFLAG_NEW_SETTINGS			= 0x00000002,	// TRUE when the cambot's values have been updated
		BOTCAMERAFLAG_OVERRIDEPITCH_TRICK	= 0x00000004,	// TRUE when the cambot's values have been updated
		BOTCAMERAFLAG_NONE					= 0x00000000
	};


	// Bot flags for jumping:
	enum {
		JUMPFLAG_IS_JUMPING					= 0x00000001,	// TRUE when the bot is jumping
		JUMPFLAG_CANNOT_USE_JUMPPAD			= 0x00000002,	// TRUE when the bot doesn't boost off of jumppads
		JUMPFLAG_JUMPPAD_BOOST				= 0x00000004,	// TRUE when the bot was propelled by a jumppad
		JUMPFLAG_NONE						= 0x00000000
	};


	// Weapon reloading state:
	typedef enum {
		// Common:
		WR_STATE_NONE,
		WR_STATE_ABORT,

		// Fully automatic:
		WR_STATE_FULLYAUTO_RELOAD_OPENING,
		WR_STATE_FULLYAUTO_RELOADING,

		// Rocket launcher 1:
		WR_STATE_ROCKET1_GRAB_AMMO,
		WR_STATE_ROCKET1_DROP_IN,
		WR_STATE_ROCKET1_GRAB_ANOTHER,
		WR_STATE_ROCKET1_FINISH,

		// Tether:
		WR_STATE_TETHER_PREPARE_TO_PUMP,
		WR_STATE_TETHER_PUMP,
		WR_STATE_TETHER_GRAB_AMMO,
		WR_STATE_TETHER_DROP_IN,
		WR_STATE_TETHER_FINISH,

		// Primary weapon up:
		WR_STATE_RAISING_WEAPON,
		WR_STATE_RELOADING,
		WR_STATE_COCKING,
		WR_STATE_LOWERING_WEAPON,

		// Clip:
		WR_STATE_CLIP_EJECT_OLD,
		WR_STATE_CLIP_GRAB_NEW,
		WR_STATE_CLIP_GRAB_NEW_BLEND_IN,
		WR_STATE_CLIP_INSERT_NEW,
		WR_STATE_CLIP_SLAPIN_NEW,

		WR_STATE_COUNT
	} WRState_e;


	// Throwing state:
	typedef enum {
		THROWSTATE_NONE,						// Not in the process of throwing
		THROWSTATE_REACHING_FOR_AMMO,			// Reaching for ammo to throw
		THROWSTATE_WINDING_BACK,				// Arm going back to throw position
		THROWSTATE_HOLD_WIND_BACK,				// Holding at full wind up
		THROWSTATE_CANCEL_WIND_BACK,			// Not going to throw
		THROWSTATE_WINDING_UP,					// Winding up the throw
		THROWSTATE_RECOVERING,					// Returning to the non-throw state

		THROWSTATE_COUNT
	} ThrowState_e;


	// Scope state:
	typedef enum {
		SCOPESTATE_NONE,						// Not using the scope at all
		SCOPESTATE_REACHING_FOR_SCOPE,			// Reaching for the scope device
		SCOPESTATE_ATTACHING_SCOPE,				// Moving scope to eye
		SCOPESTATE_LOOKING_THROUGH_SCOPE,		// Looking through the scope (zoom mode)
		SCOPESTATE_RETURNING_TO_IDLE,			// Returning to the idle pose
		SCOPESTATE_REATTACHING_SCOPE,			// Reattaching scope to eye
		SCOPESTATE_ABORTING,					// Aborting

		SCOPESTATE_COUNT
	} ScopeState_e;


	// Bot state:
	typedef enum {
		STATE_GROUND,							// Bot is on the ground
		STATE_AIR,								// Bot is in the air

		STATE_COUNT
	} State_e;


	// Traction mode:
	typedef enum {
		TRACTIONMODE_NORMAL,					// Normal idle/sneak/walk/run
		TRACTIONMODE_LOW_TRACTION,				// Low traction area
		TRACTIONMODE_SLIP,						// Slipping (moving up a steep hill at normal speed)
		TRACTIONMODE_SLIDE,						// Sliding (moving down a steep hill, or moving above normal speed)

		TRACTIONMODE_COUNT
	} TractionMode_e;


	// Jump state:
	typedef enum {
		BOTJUMPSTATE_NONE,						// Not jumping
		BOTJUMPSTATE_LAUNCH,					// Launching off the ground
		BOTJUMPSTATE_AIR,						// Fully in air
		BOTJUMPSTATE_TUCK,						// Tucking for flip
		BOTJUMPSTATE_FLIPPING,					// Flipping
		BOTJUMPSTATE_UNTUCK,					// Untucking from flip
		BOTJUMPSTATE_CABLE,						// Hanging from a cable
		BOTJUMPSTATE_LAND,
		BOTJUMPSTATE_DIVE_ENTER,				// used by msch for diving manuever
		BOTJUMPSTATE_AIR2,						// used for bots that don't have a tuck/flip, but a different kind of secondary jump
		BOTJUMPSTATE_DIVE_FLY = BOTJUMPSTATE_AIR2,
		BOTJUMPSTATE_DIVE_TUCK,
		BOTJUMPSTATE_LAND2,
		BOTJUMPSTATE_DIVE_LAND = BOTJUMPSTATE_LAND2,
		BOTJUMPSTATE_DIVE_EXIT,

		BOTJUMPSTATE_COUNT
	} BotJumpState_e;


	// BotMove state
	typedef enum {
		BOTMOVESTATE_NONE,
		BOTMOVESTATE_BLENDIN,
		BOTMOVESTATE_BLENDOUT,
	} BotMoveState_e;


	// Special Animation Based Moves
	typedef enum {
		BOTMOVETYPE_HOP_LEFT,
		BOTMOVETYPE_HOP_RIGHT,
		BOTMOVETYPE_VERT_STARTLE,
		BOTMOVETYPE_ROLL_LEFT,
		BOTMOVETYPE_ROLL_RIGHT,
		NUM_BOTMOVETYPES,
	} BotMoveType_e;


	// Sleep state
	typedef enum {
		BOTSLEEPSTATE_NONE,
		BOTSLEEPSTATE_INIT,
		BOTSLEEPSTATE_DOZE_LOOP,
		BOTSLEEPSTATE_WAKE,
		BOTSLEEPSTATE_NAPJERK,
	} BotSleepState_e;


	// Power state:
	typedef enum {
		POWERSTATE_POWERED_UP,					// Bot is powered up
		POWERSTATE_WAITING_FOR_IMMOBILIZED,		// Waiting for bot to be immobilized
		POWERSTATE_POWERING_DOWN,				// Bot is powering down
		POWERSTATE_POWERED_DOWN,				// Bot is powered down
		POWERSTATE_POWERING_UP,					// Bot is powering up
		POWERSTATE_FINISHING_UP,				// Finishing up the power-up (blending to idle)

		POWERSTATE_COUNT
	} PowerState_e;


	// Weapon switching state:
	typedef enum {
		WS_STATE_NONE,
		WS_STATE_GRABBING,
		WS_STATE_STOWING,
		WS_STATE_RETRIEVING,
		WS_STATE_RELEASING,

		WS_STATE_COUNT
	} WSState_e;

	// Idle switching state:
	typedef enum {
		IDLESTATE_NO_IDLES_TO_PLAY,				// There are no idles animations for this bot to play
		IDLESTATE_NONE,							// This bot is not currently in an idle animation
		IDLESTATE_PLAYING,						// This bot is currently playing an idle animation
		IDLESTATE_TRANSITIONING_TO_NONE,		// This bots idle state was playing and has been aborted.  Blending back to default animation.

		IDLESTATE_COUNT,
	} IdleState_e;

	// Flags that control the useage of the user anim slots (see m_nUASlotFlags):
	enum {
		UASLOTFLAG_NONE						= 0x00,	
		UASLOTFLAG_LOCKED					= 0x01,	// TRUE when the user Anim slots are in use
		UASLOTFLAG_ABORTABLE				= 0x02,	// TRUE when the user Anim slots are in use and the user has specified that they are abortable
		UASLOTFLAG_ABORTING					= 0x04,	// TRUE when in the process of aborting slots
		UASLOTFLAG_ABORTWITHAUTOBLENDOUT	= 0x08,	// TRUE means that bot class should autoblendout the anims when aborting
	};

	enum {
		COLLSTATE_NO_COLLISION				= 0x00000000,
		COLLSTATE_FLOOR						= 0x00000001,	// Bot colided with the floor this frame
		COLLSTATE_WALL						= 0x00000002,	// Bot collided with a wall this frame
		COLLSTATE_OBJECT					= 0x00000004, // Bot collided with a static object this frame
		COLLSTATE_BOT						= 0x00000008	// Bot collided with another bot this frame
	};


	// Bot info:
	typedef struct {
		f32 fMountPitchUpDownRevsPerSec;
		f32 fMountPitchDownLimit;
		f32 fMountPitchUpLimit;

		f32 fMountYawBaseRevsPerSec;
		f32 fMountYawOverdriveRevsPerSec;
		f32 fMountYawDeltaUnitIntoOverdrivePerSec;
		f32 fMountYawDeltaUnitOutofOverdrivePerSec;
		f32 fMountYawUnitIntoControlOverdriveThresh;
		f32 fMountYawUnitOutofControlOverdriveThresh;
		f32 fMountYawAIQuickTurnRevsPerSec;
		f32 fMaxTargetRange;							// default max range if bot has no weapon
		f32 fAllowableLockAngleCos;						// handletargeting will fail if angle to target is greater than this
	} BotInfo_MountAim_t;


	typedef struct {
		f32 fSneakVelocity;
		f32 fSneakNormVelocity;
		
		f32 fMinWalkVelocity;
		f32 fMinWalkNormVelocity;
		
		f32 fMinRunBlendVelocity;
		f32 fMinRunBlendNormVelocity;
		f32 fMaxRunBlendVelocity;
		f32 fMaxRunBlendNormVelocity;
		
		f32 fMaxXlatVelocity;
		f32 fInvMaxXlatVelocity;
		f32 fNormVelocityStepSize;

		f32 fStepSizeSlopeFactor;
		f32 fInvStepSizeSlopeFactor;

		f32 fMaxSneakStickMag;
		f32 fTopSpeedMultiplierForSlope;

		f32 fFasterThanUsualNormSpeed;
		f32 fWithinUsualNormSpeed;
		f32 fSteepSlopeUnitNormY;
		f32 fSlipStickBiasSpeed;
		f32 fMaxSlipStickBias;
		f32 fSlideGravityMult;
		f32 fSlideStrafeSpeed;

		f32 fDeltaFeetAnimPerFootSneak;
		f32 fDeltaFeetAnimPerFootWalk;
		f32 fDeltaFeetAnimPerFootRun;
		f32 fDeltaFeetAnimPerRadian;
		f32 fDeltaFeetAnimSlideWalkMult;
		f32 fDeltaFeetAnimPerFootAlertWalk;
		f32 fDeltaFeetAnimPerFootPanicRun;

		f32 fAnimAtRestWalkRunUnitTime;
		f32 fIdleToWalkDeltaSpeed;
		f32 fIdleToSneakDeltaSpeed;
		f32 fAnimLeftFootDownUnitTime;
		f32 fAnimRightFootDownUnitTime;

		f32 fSwivelHipsReturnSpeedThreshold;
		f32 fSwivelHipsDeltaRadiansPerSec;
		f32 fHipYawSlackWhileStoppedThreshold;
		f32 fHipFlipHysteresisRads;

		f32 fAirControlNudgeSpeed;

		f32 fJumpLandedSoundUnitVolume_2D;
		f32 fFootstepSoundUnitVolume_2D;

		f32 fJumpLandedSoundRadius_3D;
		f32 fJumpLandedSoundUnitVolume_3D;
		f32 fFootstepSoundRadius_3D;
		f32 fFootstepSoundUnitVolume_3D;

		f32 fHopLRImpulseMag;
		f32 fHopAnimLandPct;
		f32 fApproxHopLRDist;

		f32 fRollLRImpulseMagXZ;
		f32 fRollLRImpulseMagY;
		f32 fRollStickForceWhileRolling;
		f32 fRollAnimLandPct;
		f32 fApproxRollLRDist;

		f32 fAirControlSpeedThreshold;		// if the bot is moving faster than this in the XZ direction, air control is disabled

		f32 fLimpMaxUnitRun;				// 0.0 will limit the bot to walk speed while limping.  1.0 will allow full speed while limping
		f32 fLimpAnimSpeedMult;				// multiplier for limp animations, they need to cycle faster to travel the same distance as walk anims
		f32 fLimpOOBlendinTime;				// How long it takes to blend into limp mode

		f32 fOOMaxWalkRunBlendTime;			// max speed the walk run animations can blend in or out
	} BotInfo_Walk_t;


	typedef struct {
		f32 fVerticalVelocityJump1;
		f32 fVerticalVelocityJump2;

		f32 fFlipOriginX;
		f32 fFlipOriginY;
		f32 fFlipOriginZ;

		f32 fTuckUntuckAnimSpeedMult;
		f32 fUntuckToFlyBlendSpeed;
		f32 fGroundToAirBlendSpeed;

		f32 fMovingLandAnimSpeedMult;
	} BotInfo_Jump_t;


	typedef struct {
		f32 fOriginToApproxMuzzleX;
		f32 fOriginToApproxMuzzleY;
		f32 fOriginToApproxMuzzleZ;

		f32 fFire1BlendInSpeed;
		f32 fFire1BlendOutSpeed;

		f32 fThrowBlendInUnitTime;
		f32 fOOThrowBlendInUnitTime;
		f32 fThrowBlendOutUnitTime;
		f32 fOOInvThrowBlendOutUnitTime;

		f32 fMaxSingleReloadUnitBlend;
		f32 fRocket1ReloadAnimSpeedMult;
		f32 fTetherReloadAnimSpeedMult;
		f32 fTetherPumpAnimSpeedMult;

		f32 fSwitchTimeScale;

		cchar *pszPrimaryAttachBoneName;
		cchar *pszSecondaryAttachBoneName;

		FSndFx_FxHandle_t hWSAttachSound;
		FSndFx_FxHandle_t hWSDetachSound;
		FSndFx_FxHandle_t hWSBackpackSound;

		f32 fClipReloadAnimSpeedMult;
	} BotInfo_Weapon_t;


	typedef struct {
		FSndFx_FxHandle_t ahStepSound[SURFACE_TYPE_COUNT][BOTSOUND_NUM_SOUNDS_PER_SURFACE];
		// Other surface types below
	} BotInfo_Sounds_t;


	typedef enum {
		ASI_STAND,
		ASI_STAND_ALERT,
		ASI_STAND_LIMP_LEFT,
		ASI_STAND_LIMP_RIGHT,

		ASI_TURN_IN_PLACE,
		ASI_SNEAK,
		ASI_WALK,
		ASI_WALK_ALERT,
		ASI_RUN,
		ASI_RUN_PANIC,

		ASI_LIMP_LEFT,
		ASI_LIMP_RIGHT,

		ASI_FALL,

		ASI_RELOAD_WITH_LEFT_GRAB_AMMO,
		ASI_RELOAD_WITH_LEFT_DROP_IN,
		ASI_RELOAD_WITH_LEFT_FINISH,

		ASI_RELOAD_TETHER_PREPARE_TO_PUMP,
		ASI_RELOAD_TETHER_PUMP,
		ASI_RELOAD_TETHER_GRAB_AMMO,
		ASI_RELOAD_TETHER_DROP_IN,
		ASI_RELOAD_TETHER_FINISH,

		ASI_RELOAD_SINGLE_ARM,
		ASI_RELOAD_SINGLE_BODY,

		ASI_RELOAD_CLIP_EJECT_OLD,
		ASI_RELOAD_CLIP_GRAB_NEW,
		ASI_RELOAD_CLIP_INSERT_NEW,
		ASI_RELOAD_CLIP_SLAPIN_NEW,

		ASI_RC_TETHERED,
		ASI_RC_POWER_DOWN,
		ASI_RC_POWER_UP,

		ASI_HOP_LEFT,
		ASI_HOP_RIGHT,
		ASI_STARTLE,
		ASI_ROLL_LEFT,
		ASI_ROLL_RIGHT,

		ASI_STOOP,

		ASI_DOZE_LOOP,
		ASI_NAPJERK,
		ASI_WAKE,

		ASI_AIM_PILLBOX,

		ASI_RESPAWN,

		ASI_ASSEMBLE,
		ASI_DISASSEMBLE,

		ASI_COUNT
	} AnimStackIndex_e;


	typedef enum {
		UNPLUG_STATE_NONE,
		UNPLUG_STATE_STATIC_SHRINK_Y,
		UNPLUG_STATE_STATIC_SHRINK_X,
		UNPLUG_STATE_DOT_FADE,
		UNPLUG_STATE_FADE_IN,
		UNPLUG_STATE_POWER_UP,

		UNPLUG_STATE_COUNT
	} UnplugState_e;


	//
	//	 Another important state of a bot
	//
	enum BotLifeCycle_e {
		BOTLIFECYCLE_LIVING = 0,	   // From birth to death. the lifecycle is living.  Note! Should bot be re-cycled after having died, then lifecycle must be reset to living.
		BOTLIFECYCLE_DYING,			   // Bot has begun dying.  CBot::Deathwork will get called every frame and contains logic for when and why to advance to BOTLIFECYCLE_DEAD
		BOTLIFECYCLE_DEAD,			   // Bot is dead.  and will be removed from world if   (BOTDEATHFLAG_PERSISTAFTERDEATH or BOTDEATHFLAG_AUTOPERSISTAFTERDEATH) is not set.
		BOTLIFECYCLE_BURIED			   // Bot is dead, and has been dead long enough to be resurrected.
	};


	//
	// These are flags pertaining to the behaviour of the bot it dies.
	//
	enum BotDeathParamFlags_e {
		BOTDEATHFLAG_DISAPPEAR				=	0x00000000, // Bot will disappear when destroyed.
		BOTDEATHFLAG_COMEAPART				=	0x00000001, // Bot will come apart into debris apon dying  (probably doesn't make sense to comine this with either type of persist)
		BOTDEATHFLAG_PLAYDEATHANIM			=	0x00000002, // Bot has a death animation
		BOTDEATHFLAG_PERSISTAFTERDEATH		=	0x00000004, // Bot will persist in the world after dying and never disappear  (this is good for things that die, but shouldn't disappear, like the main player if it has a death animation or something).
		BOTDEATHFLAG_AUTOPERSISTAFTERDEATH	=	0x00000008, // Bot will persist after dying until the game engine decides it is a good time to remove it from the world. (good for regular bots when they don't explode and have a death animation)
		BOTDEATHFLAG_PLAYDEATHANIM_STARTED	=	0x00000010, // Bot's death Animation is currently being played.
		BOTDEATHFLAG_COMEAPART_FAST			=   0x00000020, // A flag that BotGrunt used to use
		BOTDEATHFLAG_BOTPARTMGR_DEATHANIM	=   0x00000040, // Bot is dying ant the botpartmgr will decide when to advance the lifecycle state to BOTLIFECYCLE_DEAD
		BOTDEATHFLAG_WALKING_DEAD			=   0x00000080, // Bot Died, but was given a bit of health so that he could walk around, until he dies again.
		BOTDEATHFLAG_COLLIDES_AFTER_DEATH	=   0x00000100, // Bot Died, but you can still walk on 'im
	};


	typedef enum {
		LIGHT_STATE_OFF,
		LIGHT_STATE_TURNING_ON,
		LIGHT_STATE_TURNING_OFF,
		LIGHT_STATE_ON,
	} LightState_e;


	enum LightOverrideState_e {
		LIGHTOVERRIDE_STATE_BLENDING_TO,
		LIGHTOVERRIDE_STATE_OVERRIDDEN,
		LIGHTOVERRIDE_STATE_BLENDING_FROM,
		LIGHTOVERRIDE_STATE_NOT_OVERRIDDEN,
	};

	typedef enum {
		BOTLIMPSTATE_NONE,
		BOTLIMPSTATE_LEFT,
		BOTLIMPSTATE_RIGHT,

		BOTLIMPSTATE_COUNT
	} LimpState_e;

	struct MeleeData_t {
		const CEntity*	apMeleeHitEntityBuffer[MELEE_MAX_ENTITIES_PER_SWING];
		u32				uMeleeNumHitEntities;
		CFVec3A			vMeleeLastPos;
		const CFVec3A*	pvMeleePos;
		u32				uMaxHitEntities;
		f32				fMeleeSphereRadius;
		f32				fMinNotifyInterval;
		CDamageProfile*	pDamageProfile;
		MeleeData_t*	pMasterData;
		u64				uLastNotification;
	};

	struct PlayerDeathData_t {
		u32 uFirstDamageLimb;
		u32 uNumDamageLimbs;
		u32 uFirstDeathLimb;
		u32 uNumDeathLimbs;
		f32 fDeathTimer;
		f32 fNextDeathEvent;
	};


//----------------------------------------------------------------------------------------------------------------------------------
// Public Data:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	// Public Data Members:
	const CBotDef  *m_pBotDef;				// Type information about this bot (remains constant for the entire level) [except for siteweapons]
	BotInfo_Gen_t  *m_pBotInfo_Gen;			// General bot info
	BotInfo_Idle_t *m_pBotInfo_Idle;		// Idle info array
	u32				m_nCountIdles;			// Size of above array

	CFWorldMesh *m_pWorldMesh;				// The main world mesh for this bot (note that the bot may be made up of several meshes)
	CFWorldMesh *m_pNoLegsDebrisPileWorldMesh;	// If the bot requires a debris pile to stand on when it has no legs, this is it (NULL=none)

	s32 m_nOwnerPlayerIndex;				// Index of player who owns this bot permanently (-1=none)
	s32 m_nPossessionPlayerIndex;			// Index of player who is currently possessing this bot (-1=none)

	f32 m_fRunMultiplier;
	f32 m_fOORunMultiplier;
	
	// CameraOverride Used to influence CamBot:
	f32		m_fCamTransTime;				// amount of time to take to transition to new lookat point
	u32		m_nBotCameraFlags;				// 
	CFVec3A m_vecCamLookAdj;				// amount to adjust the camera's lookat point
	CFVec3A	m_vecCamPosAdj;					// amount to adjust the camera's position

	f32 m_fJumpMultiplier;
	f32 m_fGravityMultiplier;
	CBotPowerupFx* m_pPowerupFx;

	u32 m_uNumAttachedSwarmers;

	// Sensors:
	CFVec3A m_GazeUnitVec_WS;				// Points in the direction the bot is gazing in world space
	const CFVec3A *m_pApproxEyePoint_WS;	// Approximate position of bot's eyes in world space (never NULL)
	const CFMtx43A* m_pAISteerMtx;			// Ai needs read access to this since it pitches and yaws torsos.

	f32 m_fCamZoomFactor;

	// Targeting:
	CFWorldMesh *m_pTargetedMesh;			// This is the mesh that the m_TargetedPoint_WS is on
	CFVec3A m_TargetLockUnitVec_WS;			// Points toward target (if target is not obtainable, this is equal to m_MtxToWorld.m_vFront)
	CFVec3A m_TargetedPoint_WS;				// This is the targeted point (for AI, this is just provided by the AI controls)

	f32 m_fBestCaseSurfaceUnitNormY_WS;
	f32 m_fXlatStickNormMag;				// The magnitude of m_XlatStickNormVecXZ_MS (0.0f to 1.0f)

	// Mount:								// Note: CEntity::m_MtxToWorld is the mount matrix
	f32 m_fMountPitch_WS;					// The mount pitch angle in world space
	f32 m_fMountPitchMin_WS;				// The minimum allowed value for m_fMountPitch_WS
	f32 m_fMountPitchMax_WS;				// The maximum allowed value for m_fMountPitch_WS
	f32 m_fMountYaw_WS;						// The mount yaw angle in world space (from world +Z axis, in the range from -PI to +PI)
	f32 m_fMountYawSin_WS;					// Sin of m_fMountYaw_WS
	f32 m_fMountYawCos_WS;					// Cos of m_fMountYaw_WS
	CFVec3A m_MountUnitFrontXZ_WS;			// Unit version of CEntity::m_MtxToWorld.m_vFront projected onto the XZ plane
	CFVec3A m_MountPrevPos_WS;				// Copy of CEntity::m_MtxToWorld.m_vPos from the previous frame

	// Xlat control stick vectors:
	CFVec3A m_XlatStickNormVecXZ_MS;		// The vector represented by the control stick (maximum magnitude is about 1.0f)
	CFVec3A m_XlatStickNormVecXZ_WS;		// World space version of m_XlatStickNormVecXZ_MS
	CFVec3A m_XlatStickUnitVecXZ_MS;		// Unit version of m_XlatStickNormVecXZ_MS, or NULL if m_fXlatStickNormMag is 0.0f
	CFVec3A m_XlatStickUnitVecXZ_WS;		// World space version of m_XlatStickUnitVecXZ_MS

	// Surface vectors:
	CFVec3A m_SurfaceUnitNorm_WS;			// If standing on a surface, this is its unit normal in world space. Otherwise this is NULL
	CFVec3A m_MovingSurfaceUnitVec_WS;		// Z lies on the surface and points toward the bot's XZ velocity vector (NULL if the bot is not moving or if it's moving 100% vertically)

	// Velocity vectors:
	CFVec3A m_Velocity_MS;					// Velocity in model space
	CFVec3A m_Velocity_WS;					// Velocity in world space
	CFVec3A m_UnitVelocity_WS;				// Unit version of m_Velocity_WS, or NULL if m_fSpeed_WS is 0.0f
	CFVec3A m_VelocityXZ_WS;				// Velocity XZ in world space
	CFVec3A m_UnitVelocityXZ_WS;			// Unit version of m_UnitVelocityXZ_WS, or NULL if m_fSpeedXZ_WS is 0.0f
	CFVec3A m_NormVelocityXZ_WS;			// Normalized XZ velocity in world space
	CFVec3A m_NormVelRotatedToXZ_WS;		// Normalized world space velocity rotated (not projected) onto the world space XZ plane

	// Speed values:
	f32 m_fSpeed_WS;						// Magnitude of m_Velocity_WS
	f32 m_fSpeedXZ_WS;						// Magnitude of m_VelocityXZ_WS
	f32 m_fNormSpeedXZ_WS;					// Magnituded of m_NormVelocityXZ_WS
	f32 m_fClampedNormSpeedXZ_WS;			// m_fNormSpeedXZ_WS clamped to 1.0f max
	f32 m_fSpeedAlongSurface_WS;			// Speed relative to the surface (when in air, this remains what it was when we left the ground)
	f32 m_fNormSpeedAlongSurface_WS;		// Normalized version of m_fSpeedAlongSurface_WS
	f32 m_fClampedNormSpeedAlongSurface_WS;	// m_fNormSpeedAlongSurface_WS clamped to 1.0f max
	
	f32 m_fUnitLimpSpeedModifier;			// used to slow the bot down while limping.


	// Info:
	f32 m_fMaxFlatSurfaceSpeed_WS;			// Maximum speed on a normal flat surface
	f32 m_fMaxVerticalSpeed_WS;				// Maximum vertical speed
	f32 m_fMaxDeltaJumpVelY_WS;				// This is what the Y component of the jump velocity of CBotControl is clamped to
	f32 m_fGravity;							// The gravity currently influencing this bot

	f32 m_fCollCylinderRadius_WS;			// Collision cylinder radius
	f32 m_fCollCylinderHeight_WS;			// Collision cylinder height

	CWeapon *m_apWeapon[2];					// Points to the currently-selected weapon (NULL=none): [0]=primary, [1]=secondary
	u32		 m_anNumDupWeapons[2];			// amount of duplicate weapons

	CWeapon *m_aapDupWeapons[2][BOT_MAX_DUPLICATE_WPNS];			// duplicate weapons

	CInventory *m_pInventory;				// points to inventory storage for this bot (for glitch, storage is in player object)

	CBotTalkInst *m_pTalkInst;				// Each bot can pre-allocate exactly one talkInst. This is it.
	CBotTalkInst *m_pActiveTalkInst;
	CBotTalkInst *m_pDeathTalkInst;			//hate to do it, but I had to remember the inst used to play the death anim, so that it can be returned to the inst pool when stopped
	CVehicle *m_pDrivingVehicle;			// pointer to vehicle that bot is currently driving, or NULL if none.
	CEntity	*m_pActionableEntityNearby;		// pointer to an entity that CBot::HandleCOllision has determined is nearby the bot.  Note!  Cleared Every frame

	u32 m_uActionTalkedCnt;

	BotWeight_e	m_eWeightClass;


	// reverse facing mode
	f32 m_fReverseFacingYaw_MS;				// current angle of bot in reverse facing mode (pi = fully reversed)

	u64 m_uShockwaveResetTicks;				// time of last shockwave effect in ticks.

	// dispenser info
	CBotDispenser *m_pCanOnlyBeDispensedBy;	// when NULL, this bot wasn't manufactured by a dispenser, if non-NULL this bot was manufactored by this dispenser

	static BOOL m_bColiseumMiniGame;


//----------------------------------------------------------------------------------------------------------------------------------
// Protected Data:
//----------------------------------------------------------------------------------------------------------------------------------
protected:

	static CFCollInfo m_CollInfo;				// Used for collision detection
	static CFSphere m_CollSphere;				// Used for collision detection
	static CFVec3A m_vMoveNormal;				// Used for collision detection
	static f32 m_fMoveLength;					// Used for collision detection
	static CBot *m_pCollBot;					// Used for collision detection

	static const FGameData_TableEntry_t m_aBotInfoVocab_Gen[];
	static const FGameData_TableEntry_t m_aBotInfoVocab_MountAim[];
	static const FGameData_TableEntry_t m_aBotInfoVocab_Walk[];
	static const FGameData_TableEntry_t m_aBotInfoVocab_Jump[];
	static const FGameData_TableEntry_t m_aBotInfoVocab_Weapon[];
	static const FGameData_TableEntry_t m_aBotInfoVocab_Sounds[];

	static CFMtx43A m_aTetherMtx43A[TETHER_RANDOM_MTX_COUNT];

	// General info:
	CBotAnimStack m_Anim;						// Animation stack
	CBotPartMgr *m_pPartMgr;					// One part manager per bot instance

	f32 m_fPanicSecs;							// How long the bot should panic (0=bot should not panic)

	u32 m_nBotFlags;							// See BOTFLAG_* for info
	u32 m_nBotFlags2;							// See BOTFLAG2_* for info
	u16 m_uJumpFlags;							// See JUMPFLAG_* for info
	u16 m_uLandingVelocityOverride;				// Intended for percentage usage 0-100%, multiply XZ velocity during jumppad landing
	void *m_pMoveIdentifier;					// Used to determine whether the bot was responsible for the relocation


	// Bot Lifecycle and death controls
	BotLifeCycle_e m_uLifeCycleState;			// BotLifeCycle_e
	u32 m_uBotDeathFlags;						// BotDeathParamFlags_e

	const CFVec3A *m_pLeftFootDustPos_WS;		// When spawning dust from under the left foot, this is the position from where to spawn the dust (NULL=none)
	const CFVec3A *m_pRightFootDustPos_WS;		// When spawning dust from under the right foot, this is the position from where to spawn the dust (NULL=none)


	// User Anim Slot Useage
	UserAnimAbortCB *m_pUserAnimAbortCB;		// A CB func that the user of the user anim slots can specify to be called when his useage is aborted
	AbortCompleteCB *m_pAbortReqCompleteCB;		// A CB func that will be called when an abort is actually completed.
	f32 m_fUnitAbortUserAnimLock;				// If aborting, ramp this down toward zero. When at zero, then consider the slots free!!!!!
	void* m_pUserAnimSlotLockId;				// When you lock, give an ID, then you check it later if you want
	

	// Remote control:
	const u8 *m_pnEnableBoneNameIndexTableForSummer_Normal;
	const u8 *m_pnEnableBoneNameIndexTableForSummer_TetherShock;
	u32 m_nTetherShockBoneCount;
	
	u8 m_anDriveManMtxIndexForSummer_TetherShock[FDATA_MAX_BONE_COUNT];
	u8  m_nUASlotFlags;							// Flags that control useage of the User Anim Slots
    BOOL8 m_bDataPortInstalled;					// This bot has a data port
	BOOL8 m_bDataPortOpen;						// This bot's data port is open
	
	f32 m_fOrigArmorModifier;					// Armor modifier at the time possession took place
	CMeshEntity *m_pDataPortMeshEntity;			// The mesh entity for this bot's data port (NULL=none)
	CMeshEntity *m_pDataPortEffectMeshEntity;	// The mesh entity for this bot's data port effect (NULL=none)
	const CFMtx43A *m_pDataPortBoneMtx;			// Data port bone matrix (NULL if data port is not installed)
	f32 m_fTetherShockAnimUnitBlend;			// Controls blending of the tether shock pose
	CFVec3A m_PossessedPos_WS;					// Position where player last took control of us
	CBot* m_pDisguisedBot;						// A different bot that this bot believes is disguised
	f32 m_fDisguiseTime;						// Amount of time remaining that m_pDisguisedBot will be disguised (only decreases when other bot is in View)
	f32 m_fPossessionDist;						// Maximum distance a possessed bot can travel
	f32 m_fUnitDisguised;						// How much we believe the disguise of m_pDisguisedBot;
	f32 m_fUnitDisguisedDecreaseRate;			// Rate at which m_fUnitDisguised is decreased
	u32 m_nPrePossessionWeaponEUKLevel[2];		// The EUK Levels of the weapons prior to Possession ( will be restored after possession is terminated )

	f32 m_fStaticOnDistThreshold;				// This is the distance at which the static will be turned on
	BOOL m_bHysterisisLargerThreshold;			// We're using the larger threshold for our hysterisis
	f32 m_fUnitStaticSurge;						// Amount of static surge (0=surge starting, 1=no surge)
	f32 m_fUnitBaseStatic;						// Amount of base static
	f32 m_fUnitHudStatic;						// Static from the self-destruct button
	f32 m_fUnitStatic;							// Amount of distance-based static
	f32 m_fUnitDrawStatic;						// The amount of static to draw

	f32 m_fUnitStaticFluxSlider;				// Used to fluctuate the static
	f32 m_fUnitStaticFluxSliderRate;
	f32 m_fStartUnitStaticFlux;
	f32 m_fEndUnitStaticFlux;

	f32 m_fUnitOutOfRangeWarningMsg;			// Controls the flashing of the out-of-range warning message

	UnplugState_e m_nUnplugState;				// When we're unplugging from a possessed bot, this is the state
	f32 m_fUnitUnplug;							// Used to animate the unplug animations

	CBot* m_pCurMech;							// this points to the mech that this bot is currently operating.  (operating a mech is not the same as possession.  (Mechs can be rats, or siteweapons or loaders, etc)

	s32 m_nRecruitID;							// Used by the recruitment system to ID a bot (-1 = not recruited)
	s32 m_nRecruitPlayerIndex;					// Player index of recruiting player if recruited

	// Power:
	PowerState_e m_nPowerState;					// Power state
	BOOL m_bOppositePowerStatePending;			// If TRUE, a power request for the opposite of the current state is pending
	f32 m_fPowerAnimUnitBlend;					// Used for blending power up/down animations
	f32 m_fPowerOffOnTime;						// How long the bot should remain off before turning back on again.
	f32 m_fPowerOffOnAnimSpeedMult;				// Normally 1, but can be set otherwise to speed up or slow down the power up/down animation
	f32 m_fPendingOffOnTime;					// Pending version of m_fPowerOffOnTime
	f32 m_fPendingPowerOffOnAnimSpeedMult;		// Pending version of m_fPowerOffOnAnimSpeedMult
	f32 m_fDataPortShockSecsRemaining;			// Number of seconds remaining to continue the data port shock effect


	// Bot info:
	BotInfo_MountAim_t *m_pBotInfo_MountAim;	// Mount aim bot info
	BotInfo_Walk_t *m_pBotInfo_Walk;			// Walk bot info
	BotInfo_Jump_t *m_pBotInfo_Jump;			// Jump bot info
	BotInfo_Weapon_t *m_pBotInfo_Weapon;		// Weapon bot info
	
	CFVec3A m_ImpulseVelocity_WS;				// Next frame's impulse velocity in world space

	// Bot sound info:
	f32 m_fNextScuffSound;								// Time until the next scuff sound in seconds.  (0,0f = play sound now)
	f32 m_fStepVolumeScale;								// Used to scale the volume of footsteps
	f32 m_fStepVolumeScaleDir;							// Direction to scale, 1.0f means up, -1.0f means down.  Only 1 or -1
	BotInfo_Sounds_t *m_pBotInfo_Sound;					// Pointer to m_BotInfo_SoundLight, m_BotInfo_SoundMedium or m_BotInfo_SoundHeavy
	static u8 m_uBotSoundFlags;							// Flag that holds which sound banks have been loaded
	static BotInfo_Sounds_t m_BotInfo_SoundLight;		// Handle to light sounds
	static BotInfo_Sounds_t m_BotInfo_SoundMedium;		// Handle to medium sounds
	static BotInfo_Sounds_t m_BotInfo_SoundHeavy;		// Handle to heavy sounds
	static BotInfo_Sounds_t m_BotInfo_SoundCorrosive;	// Handle to Corrosive sounds
	static BotInfo_Sounds_t m_BotInfo_SoundZombieBoss;	// Handle to Zombie Boss sounds
	static BotInfo_Sounds_t m_BotInfo_SoundVehicle;		// Handle to Vehicle sounds

	// Controller input:
	BOOL m_bControls_Human;						// TRUE=human controls (or none), FALSE=bot controls

	f32 m_fControls_AimDown;					// +1=aim down, -1=aim up
	f32 m_fControls_RotateCW;					// +1=rotate CW, -1=rotate CCW
	f32 m_fControls_Fire1;						// >0: primary fire trigger pressed
	f32 m_fControls_Fire2;						// >0: secondary fire trigger pressed
	BOOL m_bControls_Jump;						// Jump button pressed
	BOOL m_bControls_Action;					// Action button pressed.
	f32	m_fControls_FlyUp;						// +1=fly up, -1=fly down
	BOOL m_bControls_Melee;						// Melee button pressed

	f32 m_fControlsHuman_Forward;				// Human controls: +1=forward, -1=backward
	f32 m_fControlsHuman_StrafeRight;			// Human controls: +1=strafe right, -1=strafe left

	BOOL m_bControlsBot_Jump2;					// Jump2 command
	CFVec3A m_ControlBot_TargetPoint_WS;		// Desired target point in world space (used when m_nFlags::FLAG_AIM_AT_TARGET_POINT is TRUE)
	CFVec3A m_ControlsBot_XlatNormSpeedXZ_WS;	// Bot controls: Copy of CBotControl::m_XlatNormSpeedXZ_WS
	CFVec3A m_ControlsBot_JumpVelocity_WS;		// Jump velocity in world space (used when m_bControlsBot_JumpVec is used to jump)
	CFVec3A m_vControlBot_HeadLookPoint_WS;		// Desired WS head look target point as received from entity control
	BOOL m_bControlsBot_JumpVec;				// Jump using m_ControlsBot_JumpVelocity_WS
	u32 m_nControlsBot_Flags;					// See CBotControl::FLAG_* for info
	u32 m_nControlsBot_Buttons;					// See CBotControl::BUTTONFLAG_* for info
	f32 m_fControlsQuickRotateRads;				// Number of radians to rotate if QuickRotate flag was set
	f32 m_fUnitTopSpeedLimiter;					// 0.0f-1.0f limits top speed (for example titan when firing guns)
	f32 m_fControls_PrevFrameFire2;				// Copy of m_fControls_Fire2 from last frame
	f32 m_fInputIdleTime;

	// Traction:
	TractionMode_e m_nTractionMode;				// Traction mode
	f32 m_fSlipGravityStickBias;				// When in TRACTIONMODE_SLIP mode, this is the amount to bias the control stick toward downhill
	f32 m_fUnitTraction;						// Current terrain traction (0=no traction, 1=max traction)


	// Jump:
	BotJumpState_e m_nJumpState;			// Jump state
	f32 m_fTuckUntuckAnimTimeMult;			// Multiplier for tuck/untuck animation delta time
	f32 m_fUntuckStartCountdownSecs;		// Number of seconds until untuck animation should start
	f32 m_fFlipPitchVelocity;				// Flip pitch velocity (radians/sec)
	f32 m_fFlipPitch;						// Flip pitch radians
	f32 m_fDoubleJumpTimer;					// Time in which the bot has to execute a double jump.  If 0.0f, no double jump allowed.

	BOOL m_bPlayLandAnim;					// TRUE=play land animation
	BOOL m_bPlayLandAnimLower;				// TRUE=play land animation on lower body
	f32 m_fMaxLandUnitBlend;				// When landing from a jump, this indicates the maximum amount of land-animation in the blend


	// Hop, Roll, Startle
	BotMoveState_e m_nMoveState;				
	BotMoveType_e m_nMoveType;


	// Sleep
	BotSleepState_e m_nSleepState;
	f32 m_fNapJerkTimeOut;					// If a napjerk happens before this time, bot will wake up.


	// Collision:
	State_e m_nState;						// Bot state
	State_e m_nPrevState;					// Copy of m_nState from the previous frame
	FCollImpact_t m_FeetToGroundCollImpact;	// If we're on the ground, this is the last collision impact info from the foot ray-to-ground collision test
	f32 m_fSecsSinceLastFootCollision;		// Seconds since the last foot collision was detected
	const CGCollMaterial *m_pSurfaceMtl;	// The material we're standing on
	GCollSurfType_e m_nSurfaceType;			// Tye surface type we're standing on
	CFVec3 m_vWallImpactUnitVec;			// Unit vector that indicates the push direction of a wall collision, if any
	SurfaceType_e m_nSurfaceTypeOn;			// SER: Remove this!!!      (The surface type that the bot is standing on)
	u32	m_uCollisionStatus;					// collision status, cleared each frame and set in HandleCollision

	// Animation:
	s8 m_anAnimStackIndex[ASI_COUNT];		// Anim stack index for controls and taps
	f32 m_fMaxSneakStickMag;				// If the norm xlat control stick mag is less than this, the bot is in sneak mode
	f32 m_fUnitRunBlend;					// Amount of run animation
	f32 m_fUnitWalkBlend;					// Amount of walk animation
	f32 m_fUnitSneakBlend;					// Amount of sneak animation
	f32 m_fUnitCableBlend;					// Amount of cable grasping animation
	f32 m_fUnitFlyBlend;					// Amount of fly animation
	f32 m_fUnitJump1Jump2Blend;				// 0=jump1, 1=jump2
	f32 m_fUnitFeetAnim;					// Unit percentage into the Sneak/Walk/Run animation cycle
	f32 m_fUnitPanic;						// Amount of Panic Run animationthat will be blended in, instead of regular run/walk/stand anim
	f32 m_fUnitAlert;						// Amount of the Alert Animations (Walk/Stand) that will be blended in
	f32 m_fUnitHop;							// Amount that the hop animation is on
	f32 m_fUnitWake;						// Amount that the Wake Animation is Blended in
	f32 m_fUnitScope;						// Used for the scope animations
	f32 m_fUnitNapJerk;						// Amount that the Nap Jerk ANimation is blenDed in
	s16 m_eIdleState;						// what's the idle status, kenneth?
	s16 m_nCurrentIdle;						// which idle is blending and delta timing
	f32 m_fIdleTransitionTimer;				// Timer for transition blending into and out of idle states.

	// Weapons:
	ThrowState_e m_nThrowState;				// Secondary weapon throw state (for grenades and other throwable secondary weapons)
	ScopeState_e m_nScopeState;				// Secondary weapon scope state (for the sniper scope)
	BOOL m_abReloadRequest[2];				// TRUE=reload this weapon: [0]=primary, [1]=secondary
	f32 m_fLastFireTime;					// world time in seconds of the last fire.

	f32 m_fScopeUnzoomHitpointAccumulator;	// Accumulates recent hitpoints. When the value exceeds a certain threshold, scope zoom mode is cancelled
	f32 m_fScopeUnzoomHitpointTimer;		// Measures time since last hitpoint damage. When this reaches a certain time, m_fScopeUnzoomHitpointAccumulator is cleared


	// Weapon switching:
	WSState_e m_nWSState;					// Weapon switching state
	u32 m_nWSASI;							// Current weapon switch anim stack index


	// Weapon reloading:
	WRState_e m_nWRState;					// Weapon reloading state
	AnimStackIndex_e m_nWRAbortASI;			// Abortion weapon reload anim stack index
	f32 m_fWRAbortUnitControl;				// Abortion weapon reload animation control amount


	// Mount/Aim:
	f32 m_fYawUnitOverdrive;
	f32 m_fAimPitch_WS;						// Aim pitch angle
	f32 m_fAimDeltaYaw_MS;					// Aim yaw angle
	f32 m_fMountPitchClampedNormDelta;		// This is the normalized speed at which the mount's pitch is changing (0 to 1)
	f32 m_fMountYawClampedNormDelta;		// This is the normalized speed at which the mount's yaw is changing (0 to 1)

	CFVec3A m_MountPos_WS;					// This will eventually get stored in the mount position
	CFVec3A m_LastStickyEntityPos;			// This is the last position of the entity I'm sticking to (meaningless if m_pStickyEntity=NULL)
	CEntity *m_pStickyEntity;				// This is a thing that I stick to and move with automatically
	cchar *m_pszStickyEntityBoneName;		// This the name on the thing that I stick to and move with automatically


	// Animation:
	CFAnimManFrame m_AnimManFrameAim;		// Anim manual-driven frame used for aiming
	f32 m_fUnitTorsoTwistBlend;				// Amount of rocket launcher L1 torso twist
	f32 m_fUnitReloadSingleBlend;			// Amount of reload-single blending
	f32 m_fLegsYaw_MS;						// Yaw of where legs are facing in model space
	f32 m_fHipFlipHysteresisTimeOut;		// If > current Time, the time at which the hip-flip historesis will disable			


	// Head Look
	f32 m_fHeadLookMaxPitchCos;				// cosine of max angle to pitch head up or down
	f32 m_fHeadLookMaxYawCos;				// cosine of max angle to yaw head left or right
	f32 m_fHeadLookHystPitchCos;			// cosine of hysteresis angle to pitch head up or down
	f32 m_fHeadLookHystYawCos;				// cosine of hysteresis angle to yaw head left or right
	f32 m_fHeadLookSlerp;					// head look slerp amount
	CFVec3A m_vHeadLookPoint_WS;			// Desired point for bot's head to look at. Only use SetHeadLookLocation() to set!
	CFQuatA m_HeadLookCurrentQuat;			// current head look quat
	CFQuatA m_HeadLookOriginQuat;			// head look quat for start of current slerp
	s32 m_nHeadLookInterruptCount;			// counts head look interrupt requests


	// Zipline:
	CZipLineHook *m_pCableHook;				// The cable hook we're attached to (NULL=none)
	CEZipLine *m_pCablePrev;				// Previous cable we were attached to (NULL=none)
	f32 m_fCableGrabTime;					// Time in seconds until we can grab the previous zip line again (0.0=can grab now)
	CFVec3A m_ZipVerletCurr;				// Current verlet point
	CFVec3A m_ZipVerletPrev;				// Previous verlet point
	CFVec3A m_ZipHandPos_MS;				// Hand position in model space
	f32 m_fZipVerletRestDistance;			// Rest distance between the mount position and the hand

	LimpState_e m_nLimpState;				// Our current limp state
	f32			m_fUnitLimpBlend;			// Max amo


	// Damage:
	f32 m_fRotationalImpactDeltaYaw;		// Amount to add to rotational yaw when bot gets hit
	f32 m_fRotationalImpactSpeedMult;		// Times the duration of the rotational impact

	BOOL m_bStooping;						// are we stooping/trying to stoop
	f32  m_fStoopMaxHeight;					// max height (unstooped)
	f32	 m_fStoopMinHeight;					// min height (stoopid)

	f32 m_fStunTime;						// how much longer will we be stunned
	
	f32 m_fSlowEffectMult;					// how slowed down do we get?
	f32 m_fSlowTimeSinceHit;				// how long have we been paralyzed
	f32 m_fSlowTimeSpeedUp;					// when do we start moving again?
	f32 m_fSlowTimeOver;					// when do we attain full speed
	f32 m_fSlowUnitParalysis;				// 0 means fine, 1 means paralzyed


	// Spotlight Variables:
	CFWorldAttachedLight* m_pSpotLight;					// pointer to bot spotlight
	CFWorldMesh*		  m_pSpotLightMesh;				// pointer to bot spotlight mesh
	LightState_e		  m_eSpotLightState;			// state of bot light
	const CFMtx43A*		  m_pSpotLightMeshMtx;			// used to orient bot spotlight
	LightOverrideState_e  m_eSpotLightOverrideState;	// what is the status of our spotlight override?
	f32					  m_fSpotLightOverrideTimer;	// timer used for the spot light override code

	const CFVec3A *m_pStickModifier_MS;

	CEShield *m_pShield;					// Bot shield (NULL=none)

	u32 m_uLimbBreakCounter;				// used to track how many potential limb breaking attacks we've taken.

	u8 m_nTeamNum;							// Team number of this bot for multiplayer
	u8 m_nOrigTeam;							// Team number this bot started with (unrecruited team)
	u8 m_nTagPointCount;					// Number of tag points on this bot (never zero)
	u8 m_nPad;								// This space available for rent
	CFVec3A *m_pTagPointArray_WS;			// Points to an array of world space tag points (never NULL)
	CFVec3A *m_pTagPointArray_MS;			// Points to an array of model space tag points (NULL=one tag point in the bone matrix palette and it's pointed to by m_pTagPointArray_WS)

	// Data for respawn effects:
	FParticle_DefHandle_t m_hRespawnEffectPartDef;
	FParticle_EmitterHandle_t m_hRespawnEffectParticle;
	CFWorldMesh *m_pRespawnEffectMesh;

	BOOL	m_bRespawnEffectInitialized;
	f32		m_fRespawnEffectTimer;
	s32		m_nRespawnEffectMeshBoneIdx;
	CFVec3A m_vRespawnEffectMeshPos;
	f32		m_fRespawnEffectPartIntensity;
	f32		m_fRespawnEffectMeshAlpha;
	f32		m_fRespawnEffectMeshStartScale;
	f32		m_fRespawnEffectMeshFinishScale;

	// Melee
	static FCollImpact_t	m_aMeleeCollisionThisFrameBuffer[MELEE_MAX_ENTITIES_PER_SWING];
    static u32				m_uMeleeCollisionThisFrameCount;
	static MeleeData_t*		m_pMeleeDataCB;

	
	// bot in pieces
	BOOL8 m_bFallAnimDone;
	BOOL8 m_bOnGround;
	BOOL8 m_bInPieces;
	BOOL8 m_bFallDown;
	BOOL m_bColWithPieces;
	f32 m_fTimeInPieces;
	f32	m_fUnitPiecesBlend;

	// player death
	PlayerDeathData_t *m_pPlayerDeathData;


//----------------------------------------------------------------------------------------------------------------------------------
// Private Data:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	static BOOL m_bBotSystemInitialized;
	static CFTexInst m_StaticTexInst;					// Texture used for drawing the possession static
	static CFTexInst m_GlowTexInst;						// Texture used for drawing the static glow
	static s32 m_nBotDieEvent;
	static BOOL m_bAllowPlayerDeath;					// If this is TRUE, the player can die.
	static BOOL m_bDisableBotDamageGlobally;			// If this si TRUE, no bot can be damaged.
	static BOOL m_bCutscenePlaying;						// If this is TRUE, then a game cutscene is playing
	static f32 m_fPossessionTerminatedPowerDownTime;	// The amount of time a bot will be powered down after possesion is over.
	static CFSoundGroup *m_pSoundGroupPullThePlug;		// Sound played when the "TV turn-off" effect begins when the possession plug is pulled




//----------------------------------------------------------------------------------------------------------------------------------
// Public Functions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	virtual ~CBot();


	// System:
	static BOOL InitSystem( void );
	static void UninitSystem( void );
	static FINLINE BOOL IsSystemInitialized( void ) { return m_bBotSystemInitialized; }
	static void DrawOrthoEffects( s32 nPlayerIndex );
	static void DisableBotDamageGlobally( void ) { m_bDisableBotDamageGlobally = TRUE; }
	static void EnableBotDamageGlobally( void ) { m_bDisableBotDamageGlobally = FALSE; }
	static BOOL IsBotDamageDisabledGlobally( void ) { return m_bDisableBotDamageGlobally; }
	static void SetHudMode( CHud2 *pHud, const CBotDef *pBotDef );
	static void SetCutscenePlaying( BOOL bCutscenePlaying ) { m_bCutscenePlaying = bCutscenePlaying; }


	// Getters:
	FINLINE BOOL IsEnemy( void ) const { FASSERT( IsCreated() ); return (BOOL)(m_nBotFlags & BOTFLAG_ENEMY); }
	FINLINE BOOL IsSameTeam(CBot* pBot) {FASSERT( IsCreated() ); return (m_nTeamNum == pBot->m_nTeamNum); }
	FINLINE BOOL IsMeshCollideOnly( void ) const { FASSERT( IsCreated() ); return (BOOL)(m_nBotFlags & BOTFLAG_USE_MESH_FOR_COLLISION); }
	FINLINE BOOL IsMovingBackwards( void ) const { FASSERT( IsCreated() ); return (BOOL)(m_nBotFlags & BOTFLAG_XLAT_BACKWARDS); }
	FINLINE BOOL HasTargetLock( void) const { FASSERT( IsCreated() ); return (BOOL)(m_nBotFlags & BOTFLAG_TARGET_LOCKED); }
	FINLINE BOOL IsSneaking( void ) const { FASSERT( IsCreated() ); return (BOOL)(m_nBotFlags & BOTFLAG_SNEAKING); }
	FINLINE BOOL CanPickupItems( void ) const { FASSERT( IsCreated() ); return (BOOL)!(m_nBotFlags & BOTFLAG_DONT_PICKUP_ITEMS); }
	FINLINE BOOL IsThrowing( void ) const { FASSERT( IsCreated() ); return !!(m_nThrowState != THROWSTATE_NONE); }
	FINLINE BOOL IsInAir( void ) const { FASSERT( IsCreated() ); return m_nState == STATE_AIR; }
	FINLINE BOOL HasNoLegs( void ) const { FASSERT( IsCreated() ); return (BOOL)(m_nBotFlags & BOTFLAG_NO_LEGS); }
	FINLINE BOOL IsTorsoBlownOff( void ) const { FASSERT( IsCreated() ); return (BOOL)(m_nBotFlags & BOTFLAG_TORSO_IS_BLOWN_OFF); }
	FINLINE CEShield* GetShield( void ) { FASSERT( IsCreated() ); return m_pShield; }
	FINLINE f32 ComputeCollSphereY( void ) const;
	FINLINE BOOL IsInVisibleVolumeList( void) const { return !!(m_pWorldMesh && m_pWorldMesh->GetVolumeFlags() & FVIS_VOLUME_IN_VISIBLE_LIST);}
	FINLINE BOOL IsInActiveVolumeList( void) const { return !!(m_pWorldMesh && m_pWorldMesh->GetVolumeFlags() & FVIS_VOLUME_IN_ACTIVE_LIST);}
	FINLINE BOOL IsNotInAnyVolume( void) const { return !!(m_pWorldMesh && m_pWorldMesh->GetVolumeFlags() == FVIS_VOLUME_NOT_IN_LIST);}
	FINLINE BOOL IsInVisibleOrActiveVolumeList( void ) const { return !!(m_pWorldMesh && m_pWorldMesh->GetVolumeFlags() & (FVIS_VOLUME_IN_VISIBLE_LIST | FVIS_VOLUME_IN_ACTIVE_LIST));}
	FINLINE BOOL IsFloorSentry( void ) const {FASSERT( IsCreated() ); return (TypeBits() & ENTITY_BIT_SITEWEAPON) && m_pBotDef && (m_pBotDef->m_nSubClass == BOTSUBCLASS_SITEWEAPON_FLOORSENTRY);}
	FINLINE BOOL IsPillBox( void ) const { FASSERT( IsCreated() ); return (TypeBits() & ENTITY_BIT_SITEWEAPON) && m_pBotDef && m_pBotDef->m_nSubClass == BOTSUBCLASS_SITEWEAPON_PILLBOX;}
	FINLINE BOOL IsRatGun( void ) const { FASSERT( IsCreated() ); return (TypeBits() & ENTITY_BIT_SITEWEAPON) && m_pBotDef && m_pBotDef->m_nSubClass == BOTSUBCLASS_SITEWEAPON_RATGUN;}
	FINLINE BOOL IsJumping( void ) const { FASSERT( IsCreated() ); return (BOOL)(m_uJumpFlags & JUMPFLAG_IS_JUMPING);}
	FINLINE BOOL CanUseJumpPad( void ) const { FASSERT( IsCreated() ); return !(m_uJumpFlags & JUMPFLAG_CANNOT_USE_JUMPPAD);}
	FINLINE BOOL IsJumpPadBoosted( void ) const { FASSERT( IsCreated() ); return (BOOL)(m_uJumpFlags & JUMPFLAG_JUMPPAD_BOOST);}
	FINLINE BOOL IgnoreControls( void ) const { FASSERT( IsCreated() ); return !!(m_nBotFlags & BOTFLAG_IGNORE_CONTROLS); }
	FINLINE BOOL IsUnderConstruction( void ) const { FASSERT( IsCreated() ); return !!(m_nBotFlags2 & BOTFLAG2_UNDER_CONSTRUCTION ); }
	FINLINE BOOL DroppedWeapon( void ) const { FASSERT( IsCreated() ); return ( m_nBotFlags2 & BOTFLAG2_DROPPED_WEAPON ); }
	FINLINE void SetBotFlag_PowerDownUntilPosessed( void )		{ FMATH_SETBITMASK( m_nBotFlags2, BOTFLAG2_POWERDOWN_UNTIL_POSESSED ); }
	FINLINE BOOL GetBotFlag_PowerDownUntilPosessed( void )		{ return !!(m_nBotFlags2 & BOTFLAG2_POWERDOWN_UNTIL_POSESSED); }
	FINLINE void ClearBotFlag_PowerDownUntilPosessed( void )	{ FMATH_CLEARBITMASK( m_nBotFlags2, BOTFLAG2_POWERDOWN_UNTIL_POSESSED ); }
	FINLINE BOOL GetBotFlag_CanDoubleJump( void ) const { FASSERT( IsCreated() ); return ( m_nBotFlags2 & BOTFLAG2_CAN_DOUBLE_JUMP ); }
	FINLINE BOOL GetBotFlag_UseIdleAnimations( void ) const { FASSERT( IsCreated() ); return !( m_nBotFlags2 & BOTFLAG2_DONT_USE_IDLE_ANIMATIONS ); }
	FINLINE void SetBotFlag_DontUseIdleAnimations( void ) { FASSERT( IsCreated() ); FMATH_SETBITMASK( m_nBotFlags2, BOTFLAG2_DONT_USE_IDLE_ANIMATIONS ); }
	FINLINE void ClearBotFlag_DontUseIdleAnimations( void ) { FASSERT( IsCreated() ); FMATH_CLEARBITMASK( m_nBotFlags2, BOTFLAG2_DONT_USE_IDLE_ANIMATIONS ); }
	FINLINE BOOL GetBotFlag_AllowFire( void ) const { FASSERT( IsCreated() ); return !( m_nBotFlags2 & BOTFLAG2_DONT_ALLOW_FIRE ); }
	FINLINE void SetBotFlag_DontAllowFire( void ) { FASSERT( IsCreated() ); FMATH_SETBITMASK( m_nBotFlags2, BOTFLAG2_DONT_ALLOW_FIRE ); }
	FINLINE void ClearBotFlag_DontAllowFire( void ) { FASSERT( IsCreated() ); FMATH_CLEARBITMASK( m_nBotFlags2, BOTFLAG2_DONT_ALLOW_FIRE ); }
	FINLINE void SetBotFlag_NoGravity( void ) { FASSERT( IsCreated() ); FMATH_SETBITMASK( m_nBotFlags, BOTFLAG_NO_GRAVITY ); }
	FINLINE void ClearBotFlag_NoGravity( void ) { FASSERT( IsCreated() ); FMATH_CLEARBITMASK( m_nBotFlags, BOTFLAG_NO_GRAVITY ); }
	
	CBot*	GetTargetedBot( void ) const;

	virtual FINLINE BotRace_e GetRace( void ) const { return m_pBotDef->m_nRace; }
	virtual FINLINE BotRace_e SetRace( BotRace_e eNewRace) {FASSERT( IsCreated() ); /*m_pBotDef->m_nRace = eNewRace*/ FASSERT(!"error C2166: l-value specifies const object"); return m_pBotDef->m_nRace;}
	
	// Set the team. Teams determine who is friendly for multiplayer
	FINLINE u32  GetTeam( void ) const { return (u32)m_nTeamNum; }
	FINLINE void SetTeam( u32 nTeam ) {FASSERT( IsCreated() ); m_nTeamNum = (u8)nTeam; }
	FINLINE void InitialTeam( u32 nTeam ) {FASSERT( IsCreated() ); m_nTeamNum = m_nOrigTeam = (u8)nTeam; }
	FINLINE u32  GetOrigTeam( void ) const {return (u32)m_nOrigTeam; }
	FINLINE void ResetTeam( void ) {FASSERT( IsCreated() ); m_nTeamNum = m_nOrigTeam; }
	FINLINE f32  GetRunSpeed( void ) const {return m_fRunMultiplier;}
	FINLINE f32  SetRunSpeed( f32 fNewSpeed ) { FASSERT(fNewSpeed > 0.001f); f32 f = m_fRunMultiplier; m_fRunMultiplier = fNewSpeed; m_fOORunMultiplier = 1.0f / m_fRunMultiplier; return f; }

	FINLINE void SetBotFlag_UnderConstruction( void ) { FMATH_SETBITMASK( m_nBotFlags2, BOTFLAG2_UNDER_CONSTRUCTION ); }
	FINLINE void ClearBotFlag_UnderConstruction( void ) { FMATH_CLEARBITMASK( m_nBotFlags2, BOTFLAG2_UNDER_CONSTRUCTION ); }
	
	FINLINE BotWeight_e GetWeightClass( void ) { FASSERT( IsCreated() ); return m_eWeightClass; }
			// Public method of enabling/disabling drawing of reticle.
			//   If TRUE, then setting of Reticle_EnableDraw is obeyed.
			//   If FALSE, then reticle drawing is disabled and Reticle_EnableDraw() settings ignored.
			void ReticleEnable ( BOOL bIsEnabled );
			BOOL IsReticleEnabled(void);

	virtual FINLINE u32 GetTagPointCount( void ) const { return m_nTagPointCount; }
	virtual FINLINE const CFVec3A *GetTagPoint( u32 nTagPointIndex ) const { FASSERT( nTagPointIndex < m_nTagPointCount ); return &m_pTagPointArray_WS[nTagPointIndex]; }


	// Panic:
	FINLINE f32 GetPanicSecs( void ) const { FASSERT( IsCreated() ); return m_fPanicSecs; }
	FINLINE void SetPanicSecs( f32 fNewPanicSecs ) { FASSERT( IsCreated() ); m_fPanicSecs = fNewPanicSecs; }


	// Control:
	FINLINE void ImmobilizeBot( void ) { FASSERT( IsCreated() ); FMATH_SETBITMASK( m_nBotFlags, BOTFLAG_IMMOBILIZE_PENDING ); FMATH_CLEARBITMASK( m_nBotFlags, BOTFLAG_IS_IMMOBILE ); ZeroControls(); }
	FINLINE void MobilizeBot( void ) { FASSERT( IsCreated() ); FMATH_CLEARBITMASK( m_nBotFlags, BOTFLAG_IMMOBILIZE_PENDING ); FMATH_CLEARBITMASK( m_nBotFlags, BOTFLAG_IS_IMMOBILE ); }
	FINLINE BOOL IsImmobilizePending( void ) const { FASSERT( IsCreated() ); return (BOOL)(m_nBotFlags & BOTFLAG_IMMOBILIZE_PENDING); }
	FINLINE BOOL IsImmobile( void ) const { FASSERT( IsCreated() ); return (BOOL)(m_nBotFlags & BOTFLAG_IS_IMMOBILE); }
	FINLINE BOOL IsImmobileOrPending( void ) const { FASSERT( IsCreated() ); return (BOOL)(m_nBotFlags & (BOTFLAG_IS_IMMOBILE | BOTFLAG_IMMOBILIZE_PENDING) ); }
	FINLINE BOOL IsStunned( void ) const { FASSERT( IsCreated() ); return (BOOL) (m_nBotFlags & BOTFLAG_IS_STUNNED); }
	FINLINE BOOL IsSlowed( void ) const { FASSERT( IsCreated() ); return (BOOL) (m_nBotFlags & BOTFLAG_IS_SLOWED); }
	virtual void Possess( s32 nPlayerIndex, f32 fPossessionArmorModifier );	// called on the bot who is being possessed by nPlayerIndex
	FINLINE BOOL IsPossessed( void ) const { FASSERT( IsCreated() ); return (BOOL)(m_nPossessionPlayerIndex>=0 && m_nOwnerPlayerIndex<=-1 ); }
	FINLINE void SetPossessedByConsole( BOOL bConsole ) { FASSERT( IsCreated() ); if( bConsole ) m_nBotFlags2 |= BOTFLAG2_CONSOLE_POSSESSED; else m_nBotFlags2 &= ~BOTFLAG2_CONSOLE_POSSESSED; };
	FINLINE BOOL IsPossessedByConsole( void ) const { FASSERT( IsCreated() ); return !!(m_nBotFlags2 & BOTFLAG2_CONSOLE_POSSESSED);	};
	FINLINE void SetPossessionExitable( BOOL bExitable ) { FASSERT( IsCreated() ); if( bExitable ) m_nBotFlags2 &= ~BOTFLAG2_NO_POSSESSION_EXIT; else m_nBotFlags2 |= BOTFLAG2_NO_POSSESSION_EXIT; };
	FINLINE BOOL IsPossessionExitable( void ) const { FASSERT( IsCreated() ); return !(m_nBotFlags2 & BOTFLAG2_NO_POSSESSION_EXIT);	};

		
	void SetNoLegsMode( BOOL bNoLegs );
	void SetBlownOffTorsoMode( BOOL bTorsoIsBlownOff ) { FASSERT( IsCreated() ); FMATH_WRITEBIT( bTorsoIsBlownOff, m_nBotFlags, BOTFLAG_TORSO_IS_BLOWN_OFF ); }

	// Power:
	virtual void Power( BOOL bPowerUp, f32 fPowerOffTime=0.0f, f32 fPowerOffOnSpeedMult=1.0f );
	virtual void Power_SetState( BOOL bPowerUp, f32 fPowerOffTime=0.0f );		// Immediately sets the bot to this state (no animations)
	FINLINE BOOL Power_IsPoweredUp( void ) const { FASSERT( IsCreated() ); return (BOOL)(m_nPowerState == POWERSTATE_POWERED_UP); }
	FINLINE BOOL Power_IsPoweredDown( void ) const { FASSERT( IsCreated() ); return (BOOL)(m_nPowerState == POWERSTATE_POWERED_DOWN); }
	BOOL Power_IsPoweringUp( void ) const;
	BOOL Power_IsPoweringDown( void ) const;

	FINLINE BOOL IsPlayerOutOfBody( void ) const { FASSERT( IsCreated() ); return (BOOL)(m_nPossessionPlayerIndex<=-1 && m_nOwnerPlayerIndex>=0 ); }
	FINLINE BOOL IsPlayerBot( void ) const { FASSERT( IsCreated() ); return m_nPossessionPlayerIndex >= 0; } //is a player in possession of this bot


	// Data port:
	FINLINE BOOL DataPort_IsInstalled( void ) const { FASSERT( IsCreated() ); return m_bDataPortInstalled; }
	FINLINE BOOL DataPort_IsOpen( void ) const { FASSERT( IsCreated() ); return m_bDataPortOpen; }
	FINLINE void DataPort_Reserve( BOOL bReserve ) {FASSERT( IsCreated() ); if (bReserve) FMATH_SETBITMASK( m_nBotFlags2, BOTFLAG2_DATAPORT_RESERVED ); else FMATH_CLEARBITMASK( m_nBotFlags2, BOTFLAG2_DATAPORT_RESERVED ); }
	FINLINE BOOL DataPort_IsReserved( void ) const {FASSERT( IsCreated() ); return (BOOL)(m_nBotFlags2 & BOTFLAG2_DATAPORT_RESERVED); }
	virtual void DataPort_Open( BOOL bOpen );

	void DataPort_Shock( BOOL bEnable, f32 fShockSecs=-1.0f );
	FINLINE BOOL DataPort_IsBeingShocked( void ) const { FASSERT( IsCreated() ); return (BOOL)(m_nBotFlags & BOTFLAG_IS_BEING_SHOCKED); }

	FINLINE const CFMtx43A *DataPort_GetNonUnitMatrix( void ) const { FASSERT( IsCreated() ); return m_pDataPortBoneMtx; }
	FINLINE const CFVec3A *DataPort_GetNonUnitNormal( void ) const { FASSERT( IsCreated() ); return &m_pDataPortBoneMtx->m_vY; }
	FINLINE const CFVec3A *DataPort_GetPos( void ) const { FASSERT( IsCreated() ); return &m_pDataPortBoneMtx->m_vPos; }
	void DataPort_ComputeUnitNormal( CFVec3A *pUnitNormal ) const;

	FINLINE void DataPort_AttachEntity_WS( CEntity *pEntityToAttach );
	FINLINE void DataPort_AttachEntity_PS( CEntity *pEntityToAttach );
	FINLINE void DataPort_AttachEntityUsingUnitMtx_PS( CEntity *pEntityToAttach, const CFMtx43A *pUnitMtx_PS=&CFMtx43A::m_IdentityMtx );
	FINLINE void DataPort_AttachEntityUsingUnitMtx_PS_AndScale_WS( CEntity *pEntityToAttach, const CFMtx43A *pUnitMtx_PS=&CFMtx43A::m_IdentityMtx, f32 fNewEntityScale_WS=1.0f );
	FINLINE void DataPort_AttachEntityUsingUnitMtx_PS_AndScale_PS( CEntity *pEntityToAttach, const CFMtx43A *pUnitMtx_PS=&CFMtx43A::m_IdentityMtx, f32 fNewEntityScale_PS=1.0f );

	FINLINE f32 DataPort_GetDisguiseTime( void ) const { FASSERT( IsCreated() ); return m_fDisguiseTime; }
	FINLINE f32 DataPort_GetPossessionDist( void ) const { FASSERT( IsCreated() ); return m_fPossessionDist; }
	void DataPort_SetDisguiseTime( f32 fDisguiseTime );
	FINLINE void DataPort_SetPossessionDist( f32 fPossessionDist ) { FASSERT( IsCreated() ); m_fPossessionDist = fPossessionDist; }


	// Recruitment:
	BOOL Recruit_CanBeRecruited( void ) const;
	BOOL Recruit_IsRecruited( void ) const { FASSERT( IsCreated() ); return (m_nRecruitID >= 0); }
	s32  Recruit_GetRecruiter( void ) const { FASSERT( IsCreated() ); return (m_nRecruitID >= 0) ? m_nRecruitPlayerIndex : -1; }
	BOOL Recruit( CBot *pRecruiterBot, const CFVec3A *pEpicenter_WS=NULL );
	void Unrecruit( void );


	// Radar:
	BOOL Radar_CanShowUp( void ) const { FASSERT( IsCreated() ); return m_nBotFlags2 & BOTFLAG2_SHOWS_UP_ON_RADAR; }


	// Bots can operate Mechs (a mech is another bot, might be a vehicle, might be a siteweapon, might be a rat gun....
	virtual void EnteringMechWork(CBot* pMech);	// must be called when this bot gets into a vehicle or site-weapon
	virtual void ExitingMechWork(void);				// must be called when this bot gets leaves a vehicle or site-weapon
	FINLINE CBot* GetCurMech(void) const { FASSERT( IsCreated() ); return m_pCurMech;}


	// Weapons:
	void UpdateAimToTargetSamePoint( BOOL bCamAimsDownMountZ );
	virtual void ComputeHumanTargetPoint_WS( CFVec3A *pTargetPt, CWeapon *pWeapon, f32 fWeaponRangeOverride = -1.f, CFVec3A *pRawUnAssistedTargetPt=NULL );
	virtual BOOL FocusHumanTargetPoint_WS( CFVec3A *pFocusedTargetPt, CWeapon *pWeapon );
	FINLINE void JustFired( void ) { FASSERT( IsCreated() ); m_fLastFireTime = FLoop_nTotalLoopTicks*FLoop_fSecsPerTick; }
	FINLINE f32 GetLastFireTime( void ) const { FASSERT( IsCreated() ); return m_fLastFireTime; }
	virtual FINLINE BOOL SwitchingWeapons( void ) const { FASSERT( IsCreated() ); return (!(m_nWSState == WS_STATE_NONE)); };
	CWeapon* GetPrimaryWeapon( void ) { FASSERT( IsCreated() ); return m_apWeapon[0]; }
	CWeapon* GetSecondaryWeapon( void ) { FASSERT( IsCreated() ); return m_apWeapon[1]; }

	virtual void ShakeCamera( f32 fUnitIntensity, f32 fDurationInSecs, BOOL bControllerRumbleOnly=FALSE );


	// User Animations:  Please use these functions to control useage of the user anim slots
	FINLINE BOOL UserAnim_IsLocked( void ) const		{ FASSERT( IsCreated() ); return m_nUASlotFlags & UASLOTFLAG_LOCKED; }		//Are the slots locked (meaning in use)?
	FINLINE BOOL UserAnim_IsLockAbortable( void ) const	{ FASSERT( IsCreated() ); return m_nUASlotFlags & UASLOTFLAG_ABORTABLE; }	//Did the user specify that the lock is abortable?
	FINLINE BOOL UserAnim_IsAborting( void ) const		{ FASSERT( IsCreated() ); return m_nUASlotFlags & UASLOTFLAG_ABORTING; }
	FINLINE BOOL UserAnim_BeginAbortLock( AbortCompleteCB *pAbortReqCompleteCB );							//Begin aborting an abortable lock


	// Lock the User Anim Slots (only works when slots are not in use):
	BOOL UserAnim_Lock( void* pLockId,																//When you lock, please provide an ID
						BOOL bAbortable = FALSE,													//This lock is abortable
						BOOL bAutoBlendOut = TRUE,													//Abortable, but will be blended out automatically
						UserAnimAbortCB *pUserAnimAbortCB = NULL);									//If bAutoBlendOut, called once when done, else called every frame for custom aborting
	void UserAnim_UnLock( void );
	FINLINE void *UserAnim_GetLockId( void ) const { FASSERT( IsCreated() ); return m_pUserAnimSlotLockId; }

	FINLINE BOOL UserAnim_IsCreated( void ) const { FASSERT( IsCreated() ); return m_Anim.IsCreated(); }
	FINLINE u32 UserAnim_GetCount( void ) const { FASSERT( IsCreated() ); return m_Anim.UserAnim_GetCount(); }

	FINLINE void UserAnim_Attach( u32 nUserIndex, CFAnimSource *pAnimSource ) { FASSERT( IsCreated() ); m_Anim.UserAnim_Attach( nUserIndex, pAnimSource ); }
	FINLINE CFAnimSource *UserAnim_GetAttached( u32 nUserIndex ) const { FASSERT( IsCreated() ); return m_Anim.UserAnim_GetAttached( nUserIndex ); }

	FINLINE void UserAnim_SetControlValue( u32 nUserIndex, f32 fUnitControlValue ) { FASSERT( IsCreated() ); m_Anim.UserAnim_SetControlValue( nUserIndex, fUnitControlValue ); }
	FINLINE f32 UserAnim_GetControlValue( u32 nUserIndex ) const { FASSERT( IsCreated() ); return m_Anim.UserAnim_GetControlValue( nUserIndex ); }

	FINLINE void UserAnim_EnableAllBones( u32 nUserIndex ) { FASSERT( IsCreated() ); m_Anim.UserAnim_EnableAllBones( nUserIndex ); }
	FINLINE void UserAnim_DisableAllBones( u32 nUserIndex ) { FASSERT( IsCreated() ); m_Anim.UserAnim_DisableAllBones( nUserIndex ); }
	FINLINE void UserAnim_UpdateBoneMask( u32 nUserIndex, cchar *pszBoneName, BOOL bDriveBone ) { FASSERT( IsCreated() ); m_Anim.UserAnim_UpdateBoneMask( nUserIndex, pszBoneName, bDriveBone ); }
	FINLINE BOOL UserAnim_IsDrivingBone( u32 nUserIndex, cchar *pszBoneName ) const { FASSERT( IsCreated() ); return m_Anim.UserAnim_IsDrivingBone( nUserIndex, pszBoneName ); }

	FINLINE void UserAnim_BatchUpdateTapBoneMask_Open( u32 nUserIndex, BOOL bDriveBonesInList ) { FASSERT( IsCreated() ); m_Anim.UserAnim_BatchUpdateTapBoneMask_Open( nUserIndex, bDriveBonesInList ); }
	FINLINE void UserAnim_BatchUpdateTapBoneMask_Close( void ) { FASSERT( IsCreated() ); m_Anim.UserAnim_BatchUpdateTapBoneMask_Close(); }
	FINLINE void UserAnim_BatchUpdateTapBoneMask( const u8 *pnBoneNameIndexTable, cchar **ppszBoneNameTable ) { FASSERT( IsCreated() ); m_Anim.UserAnim_BatchUpdateTapBoneMask( pnBoneNameIndexTable, ppszBoneNameTable ); }
	virtual void UserAnim_BatchUpdateTapBoneMask( UserAnimBoneMask_e nBoneMaskGroup ) { FASSERT( IsCreated() ); }

	FINLINE void UserAnim_DetachAll( void ) { m_Anim.UserAnim_DetachAll(); }
	FINLINE void UserAnim_ResetAllControlValues( void ) { m_Anim.UserAnim_ResetAllControlValues(); }

	FINLINE CBotPartMgr *GetPartMgr( void ) { return m_pPartMgr; }


	// Other functions:
	FINLINE CFVec3A &WS2MS( CFVec3A &rVec_MS, const CFVec3A &rVec_WS );
	FINLINE CFVec3A &MS2WS( CFVec3A &rVec_WS, const CFVec3A &rVec_MS );
	FINLINE CFVec3A &WS2MS( CFVec3A &rVec_WS );
	FINLINE CFVec3A &MS2WS( CFVec3A &rVec_MS );

	virtual void ZeroVelocity( void );
	virtual void ApplyVelocityImpulse_MS( const CFVec3A &rVelocityImpulseVec_MS );
	virtual void ApplyVelocityImpulse_WS( const CFVec3A &rVelocityImpulseVec_WS );

	virtual void SetControls( CEntityControl *pControl );

	virtual FINLINE void NotifyWeaponUpgraded( CWeapon *pUpgradedWeapon ) { FASSERT( IsCreated() ); }
	virtual FINLINE void NotifyAmmoMightHaveChanged( CWeapon *pWeapon ) { FASSERT( IsCreated() ); }

	virtual void ComputeApproxMuzzlePoint_WS( CFVec3A *pApproxMuzzlePoint_WS );

	virtual f32 ComputeEstimatedControlledStopTimeXZ( void );
	virtual FINLINE f32 ComputeEstimatedControlledStopTimeY( void ) { FASSERT( IsCreated() ); return -1.0f; }
	virtual FINLINE f32 ComputeEstimatedMinimumTurnRadius( f32 fVelocity ) { FASSERT( IsCreated() ); return 0.0f; }

	void ConstructDamagerData( CDamageForm::Damager_t *pDestDamager=NULL );
	virtual void InflictDamageResult( const CDamageResult *pDamageResult );

	virtual void Die( BOOL bSpawnDeathEffects=TRUE, BOOL bSpawnGoodies=TRUE );	//called by TakeDamage to start the death sequence for a bot could also be thought of as StartDeath for bots
	virtual void DeathWork(void);							//should be called every frame by CBot::ClassHierarchyWork() if the bot has reached lifecycle_dying state
	virtual void SpawnDeathEffects(void);					//called once inside of CBot::Die. provides opportunity for derived classes to kick of effects or whatever. 

	virtual void ResetToNeutral( void );// called by CheckpointRestore and UnDie to reset the bot to have no twist, impulses, 

	// This method informs the Entity that it should consider itself to have pressed the action button.
	virtual void NotifyActionButton();
	// This method informs the Entity that another Entity (the one passed via parameter) pressed the action
	//   button near it.
	virtual BOOL ActionNearby( CEntity *pEntity );

	virtual BOOL DoesBoneExist( cchar *pszBoneName );

	void StartBotTalkByName( cchar *pszBTAName );
	virtual BOOL IsUseless( void ) const;
	BOOL IsBackBroken( void ) const;


	// Collision 
	FINLINE void IgnoreBotVBotCollision(void)			{ FASSERT( IsCreated() ); m_nBotFlags |= BOTFLAG_SKIP_BOT_V_BOT_COLLISIONS;}
	FINLINE void DontIgnoreBotVBotCollision(void)		{ FASSERT( IsCreated() ); m_nBotFlags &= ~BOTFLAG_SKIP_BOT_V_BOT_COLLISIONS;}
	FINLINE BOOL ShouldIgnoreBotVBotCollisions(void)	{ FASSERT( IsCreated() ); return m_nBotFlags & BOTFLAG_SKIP_BOT_V_BOT_COLLISIONS;}
	FINLINE void SetNoCollideStateAir( BOOL bAir ) { FASSERT( IsCreated() ); if( bAir ) m_nBotFlags |= BOTFLAG_NO_COLLIDE_STATE_AIR; else m_nBotFlags &= ~BOTFLAG_NO_COLLIDE_STATE_AIR; };
	FINLINE BOOL IsNoCollideStateAir( void ) const { FASSERT( IsCreated() ); return ( m_nBotFlags & BOTFLAG_NO_COLLIDE_STATE_AIR ); }


	// Head Look:
	FINLINE void HeadLook( void ) { FASSERT( IsCreated() ); FMATH_SETBITMASK( m_nBotFlags, BOTFLAG_USE_HEAD_LOOK ); }
	FINLINE void HeadStopLook( void ) { FASSERT( IsCreated() ); FMATH_CLEARBITMASK( m_nBotFlags, BOTFLAG_USE_HEAD_LOOK ); }
	FINLINE BOOL HeadShouldLook( void ) const { FASSERT( IsCreated() ); return (BOOL)(m_nBotFlags & BOTFLAG_USE_HEAD_LOOK); }
	FINLINE BOOL HeadIsLooking( void ) const { FASSERT( IsCreated() ); return (BOOL)(m_nBotFlags & BOTFLAG_HEAD_IS_LOOKING); }
	FINLINE void SetHeadLookLocation( const CFVec3A &vLoc );
	FINLINE void UpdateHeadLookLocation( const CFVec3A &vLoc );
	FINLINE void SetHeadLookRanges( const f32 fYaw, const f32 fPitch );
	FINLINE void SetHeadLookInterrupt( void ) { FASSERT( IsCreated() ); m_nHeadLookInterruptCount++; }
	FINLINE void ClearHeadLookInterrupt( void ) { FASSERT( IsCreated() ); m_nHeadLookInterruptCount--; FMATH_CLAMPMIN( m_nHeadLookInterruptCount, 0 ); }
	const CFVec3A& GetHeadLookLocation( void ) { FASSERT( IsCreated() ); return m_vHeadLookPoint_WS;}
	FINLINE BOOL HeadLookInterrupt( void ) { FASSERT( IsCreated() ); return m_nHeadLookInterruptCount; }
	void HeadLookUpdate( CFMtx43A &rOutputMtx, const CFMtx43A &rParentMtx, const CFMtx43A &rHeadMtx, BOOL bNegateLook );


	// Special animation modes:
	// (forcedPanic Interface is for making a bot be in panic mode without going throught the EntityControls interface)
	FINLINE void StartForcedPanic( void ) { FASSERT( IsCreated() ); m_fUnitPanic -= m_fUnitPanic*(m_nBotFlags & BOTFLAG_FORCED_PANIC); m_nBotFlags |= BOTFLAG_FORCED_PANIC;}
	FINLINE void EndForcedPanic( void )	{ FASSERT( IsCreated() ); m_fUnitPanic = 0.0f;	m_nBotFlags &= ~BOTFLAG_FORCED_PANIC;}
	FINLINE BOOL IsPanicOn( void ) const { FASSERT( IsCreated() ); return ((m_nBotFlags & BOTFLAG_FORCED_PANIC) || (!m_bControls_Human && m_nControlsBot_Flags & CBotControl::FLAG_PANIC));}
	FINLINE void StartForcedAlert( void ) { FASSERT( IsCreated() ); m_fUnitAlert -= m_fUnitAlert*(m_nBotFlags & BOTFLAG_FORCED_ALERT); m_nBotFlags |= BOTFLAG_FORCED_ALERT;}
	FINLINE void EndForcedAlert( void )	{ FASSERT( IsCreated() ); m_fUnitAlert = 0.0f;	m_nBotFlags &= ~BOTFLAG_FORCED_ALERT;}
	FINLINE BOOL IsAlertOn( void ) const { FASSERT( IsCreated() ); return (!IsPanicOn() && ((m_nBotFlags & BOTFLAG_FORCED_ALERT) || (!m_bControls_Human && m_nControlsBot_Flags & CBotControl::FLAG_ALERT)));}
	FINLINE BOOL IsHopping( void ) const { FASSERT( IsCreated() ); return m_nMoveState != BOTMOVESTATE_NONE; }
	FINLINE BOOL IsCapableofAlert(void) const { FASSERT( IsCreated() ); return m_anAnimStackIndex[ASI_WALK_ALERT] != -1;}
	virtual FINLINE BOOL IsCapableOfHopping( void ) const { FASSERT( IsCreated() ); return (m_anAnimStackIndex[ASI_HOP_LEFT] != -1 && m_anAnimStackIndex[ASI_HOP_RIGHT] != -1); }
	FINLINE BOOL IsCapableOfAimingPB( void ) const { FASSERT( IsCreated() ); return (m_anAnimStackIndex[ASI_AIM_PILLBOX] != -1);}
	FINLINE BOOL IsRolling( void ) const { FASSERT( IsCreated() ); return m_nMoveState != BOTMOVESTATE_NONE; }
	FINLINE BOOL IsCapableOfRolling( void ) const { FASSERT( IsCreated() ); return (m_anAnimStackIndex[ASI_ROLL_LEFT] != -1 && m_anAnimStackIndex[ASI_ROLL_RIGHT] != -1); }
	FINLINE f32 GetApproxHopDist( void ) const { FASSERT( IsCreated() ); return m_pBotInfo_Walk ? (m_pBotInfo_Walk->fApproxHopLRDist) : 0.0f; }	 //findfix: make this data come from the .csv next time I can check all b_*.csv's out.
	FINLINE f32 GetApproxRollDist( void) const { FASSERT( IsCreated() ); return m_pBotInfo_Walk ? (m_pBotInfo_Walk->fApproxRollLRDist) : 0.0f; }	 //findfix: make this data come from the .csv next time I can check all b_*.csv's out.
	virtual FINLINE BOOL CanSleep( void ) const { FASSERT( IsCreated() ); return (m_anAnimStackIndex[ASI_DOZE_LOOP]!=-1 && m_anAnimStackIndex[ASI_WAKE]!=-1 && m_anAnimStackIndex[ASI_NAPJERK]!=-1);}
	FINLINE BOOL IsSleeping( void ) const { FASSERT( IsCreated() ); return m_nSleepState != BOTSLEEPSTATE_NONE;}
	FINLINE BOOL IsWaking( void ) const { FASSERT( IsCreated() ); return m_nSleepState == BOTSLEEPSTATE_WAKE;}
	FINLINE BOOL IsLimping( void ) const { FASSERT( IsCreated() ); return !(m_nLimpState == BOTLIMPSTATE_NONE ); }
	FINLINE BOOL IsCapableOfStooping( void ) const { FASSERT( IsCreated() ); return (m_anAnimStackIndex[ASI_STOOP] != -1); }
	
	virtual BOOL WakeUp( void );							
	virtual BOOL NapJerk( BOOL bAllowAutoWakeup = TRUE );	//bAllowAutoWakeup means that this call to NapJerk is allowed to wake up the bot if bot isn't sleeping very well
	virtual BOOL Sleep( void );								//Note! This bot won't sleep if it doesn't have the necessary animations (ASI_SLEEP, ASI_WAKE, ASI_NAPJERK)
	FINLINE void OverrideHipSlackThisFrame( void ) { FASSERT( IsCreated() ); m_nBotFlags |= BOTFLAG_HIPSLACK_OVERRIDE;}
	FINLINE BOOL IsDying( void ) const { FASSERT( IsCreated() ); return m_uLifeCycleState == BOTLIFECYCLE_DYING;}
	FINLINE BOOL IsDeadOrDying( void ) const { FASSERT( IsCreated() ); return m_uLifeCycleState > BOTLIFECYCLE_LIVING;}
	FINLINE BOOL IsWalkingDead( void ) const { FASSERT( IsCreated() ); return m_uBotDeathFlags & BOTDEATHFLAG_WALKING_DEAD;}
	FINLINE BOOL IsDead( void ) const { FASSERT( IsCreated() ); return m_uLifeCycleState >= BOTLIFECYCLE_DEAD;}
	FINLINE BOOL IsBuried( void ) const { FASSERT( IsCreated() ); return m_uLifeCycleState == BOTLIFECYCLE_BURIED; }
	FINLINE BOOL IsDeathAnimStarted( void ) const { FASSERT( IsCreated() ); return m_uBotDeathFlags & BOTDEATHFLAG_PLAYDEATHANIM_STARTED;}
	virtual void UnDie(void);
	void DeathBTAEndCB(CBotTalkInst *pTalkInst);
	void NotifyPossessedBotSeen(CBot* pDisguisedBot, f32 fAddDisguiseDecrease = 0.0f);			//This is to be called whenever the bot sees a possessed bot. (bot's don't check this automatically)
	CBot* GetDisguisedBot(f32* pfDisguiseTimeLeft = NULL);		//Retrieve info about the most recently disguised bot that was seen. (returns NULL ands sets pfDisguiseTimeLeft = 0.0f if there is none.
	FINLINE const CFMtx43A* GetAISteerMtx(void) { FASSERT( IsCreated() ); return m_pAISteerMtx;}

	FINLINE	CBotPowerupFx* GetPowerupFx(void)	{ FASSERT(IsCreated());  return m_pPowerupFx;}

	// Reverse facing mode:
	void StartReverseFacingMode( void );
	FINLINE void EndReverseFacingMode( void ) { FASSERT( IsCreated() ); FMATH_CLEARBITMASK( m_nBotFlags, BOTFLAG_REVERSE_FACING ); }
	FINLINE BOOL IsReverseFacingForward( void ) const { FASSERT( IsCreated() ); return (BOOL)(m_fReverseFacingYaw_MS == 0.0f);}
	FINLINE BOOL IsReverseFacingMode( void ) const { FASSERT( IsCreated() ); return (BOOL)(m_nBotFlags & BOTFLAG_REVERSE_FACING);}
	FINLINE BOOL IsReverseTransition( void ) const { FASSERT( IsCreated() ); return (BOOL)( m_fReverseFacingYaw_MS > 0.0f && m_fReverseFacingYaw_MS < FMATH_DEG2RAD( 180.0f ) );}
	FINLINE BOOL IsReversePastHalfway( void ) const { FASSERT( IsCreated() ); return (BOOL)( m_fReverseFacingYaw_MS > FMATH_DEG2RAD( 90.0f ) );}
	void HandleReverseFacing( void );

	FINLINE BOOL IsReloading( void ) const { FASSERT( IsCreated() ); return (BOOL)( m_nWRState != WR_STATE_NONE ); }

	virtual BOOL CheckpointSave( void );
	virtual void CheckpointRestore( void );

	// Pre- and Post- user render. These are used to change what gets rendered
	// for the user's camera as opposed to another camera. That is, in multiplayer,
	// these will be called before and after the render for the player associated
	// with this bot.
	virtual void PreUserRender( void );
	virtual void PostUserRender( void );
	virtual void DebugRender( f32 fScreenTextX, f32 fScreenTextY );

	virtual FINLINE void AbortScopeMode( BOOL bImmediate=FALSE ) { FASSERT( IsCreated() ); }
	virtual FINLINE void GetScopeViewPos( CFVec3A *pScopeViewPos_WS ) { FASSERT( IsCreated() ); *pScopeViewPos_WS = m_MtxToWorld.m_vPos; }

	static void UninitLevel( void );

	virtual FINLINE void ComputeMtxPalette( BOOL bApplyOffscreenOptimizations ) { FASSERT( IsCreated() ); FASSERT( m_Anim.m_pAnimCombiner != NULL ); m_pCollBot = this; m_Anim.m_pAnimCombiner->ComputeMtxPalette( bApplyOffscreenOptimizations ); m_pCollBot = NULL; };
	FINLINE void AtRestMatrixPalette( void )		 { FASSERT( IsCreated() ); if( m_Anim.m_pAnimCombiner != NULL )  m_Anim.m_pAnimCombiner->AtRestMtxPalette(); }

	f32 GetMaxFlatSurfaceSpeed( void ) const { FASSERT( IsCreated() ); return m_pBotInfo_Walk ? ((m_fMaxFlatSurfaceSpeed_WS * GetLimitedTopSpeed()) + ((1.0f-m_pBotInfo_Walk->fMinWalkNormVelocity)*!IsAlertOn())) : m_fMaxFlatSurfaceSpeed_WS;}; 


	// Access to stooping:
	BOOL IsStoopable( f32 *pMinHeight, f32 *pMaxHeight ) const;
	void Stoop( BOOL bStoop ) { m_bStooping = bStoop; };

	void ForceQuickDataPortUnPlug( void );


	FINLINE BOOL IsSquishy( void ) const { FASSERT( IsCreated() ); return ((m_pBotDef->m_nClass == BOTCLASS_ZOMBIE) || (m_pBotDef->m_nClass == BOTCLASS_MINER) || (m_pBotDef->m_nClass == BOTCLASS_GRUNT)) && !IsInvincible(); };			// can this bot be squished by a big bot?
	virtual void Squish( const CBot* pSquisher );
	
	virtual void NotifyShieldStatus( u32 uStatus, const CFVec3A &vPos )	{};

	void Attach_ToParent_WithGlue_WS( CEntity *pParentEntity, cchar *pszAttachBoneName=NULL, BOOL bInheritParentScale=FALSE );
	virtual void DetachFromParent( void );

	void StunBot( const f32 &fStunTime );
	BOOL SlowBot(f32 fControlMult=.0f, f32 fTimeToSlowDown=.0f, f32 fTimeToStartReturningToNormal=.0f);

	void SetLimpState( u32 uNewLimpState );	// will just override whatever state is there now
	void GiveBotLimp( BOOL bLeftSide );		// give this bot a limp, if supported.  Note:  If he's already limping on one side, giving a limp on the other will kill him


	// Bot Spotlight
	virtual void SetSpotLightOn( BOOL bImmediately=FALSE );
	virtual void SetSpotLightOff( BOOL bImmediately=FALSE );
	virtual BOOL IsSpotLightOn( void ) { FASSERT( IsCreated() ); return ( m_eSpotLightState == LIGHT_STATE_TURNING_ON || m_eSpotLightState == LIGHT_STATE_ON ); }

	FINLINE void SetStickModifier_MS( const CFVec3A *pStick_MS ) { FASSERT( IsCreated() ); m_pStickModifier_MS = pStick_MS; }

	FINLINE CEntity *GetStickyEntity( void ) const { FASSERT( IsCreated() ); return m_pStickyEntity; }

	BOOL EyesDamaged( void ) const;
	BOOL EyesDestroyed( void ) const;

	FINLINE BOOL IsVoiceDamaged( void ) const { FASSERT( IsCreated() );  return EyesDamaged() || (m_nBotFlags2 & BOTFLAG2_BABBLE); }
	FINLINE BOOL IsVoiceDestroyed( void ) const { FASSERT( IsCreated() );  return EyesDestroyed(); }
	

	FINLINE u32	GetCollisionState( void ) const { FASSERT( IsCreated() ); return m_uCollisionStatus; };
	FINLINE BOOL CollidedThisFrame( void ) const { FASSERT( IsCreated() ); return !!m_uCollisionStatus; };


	// CameraOverride
	FINLINE BOOL IsCameraOverridenWithPitch( void ) const { FASSERT( IsCreated() ); return (BOOL) (IsCameraOverriden() && (m_nBotCameraFlags & BOTCAMERAFLAG_OVERRIDEPITCH_TRICK)); }
	FINLINE BOOL IsCameraOverriden( void ) const { FASSERT( IsCreated() ); return (BOOL) (m_nBotCameraFlags & BOTCAMERAFLAG_OVERRIDDEN ); }
	FINLINE BOOL IsCameraOverrideChanged( void ) const { FASSERT( IsCreated() ); return (BOOL) (m_nBotCameraFlags & BOTCAMERAFLAG_NEW_SETTINGS); }
	FINLINE void SetCameraOverrideChanged(BOOL bChanged) { FASSERT( IsCreated() ); if (bChanged) FMATH_SETBITMASK(m_nBotCameraFlags,BOTCAMERAFLAG_NEW_SETTINGS); else FMATH_CLEARBITMASK(m_nBotCameraFlags,BOTCAMERAFLAG_NEW_SETTINGS);}

	FINLINE virtual CFWorldMesh *GetMesh( void ) const { FASSERT( IsCreated() ); return m_pWorldMesh; }


	// Sounds:
	void PlaySound( CFSoundGroup *pSoundGroup, f32 fVolMult=1.0f, f32 fPitchMult=1.0f, f32 fRadiusOverride=-1.0f, CFAudioEmitter **ppUserAudioEmitter=NULL, BOOL bForce2D=FALSE ) const;
	CFAudioEmitter *AllocAndPlaySound( CFSoundGroup *pSoundGroup, f32 fVolMult=1.0f, f32 fPitchMult=1.0f, f32 fRadiusOverride=-1.0f ) const;


	// Misc:
	BOOL LoadAuxMeshEntity( CMeshEntity **ppMeshEntity, cchar **pszMeshName, u32 uMeshCount, cchar *pszEntityName );

	FINLINE virtual void StartVelocityJump( const CFVec3A *pvJumpVelocity_WS );

	f32 ComputeDistSquaredToNearestHumanPlayer( void );

	FINLINE void EnableAiming( BOOL bEnable )	{ FASSERT( IsCreated() ); bEnable ? FMATH_CLEARBITMASK( m_nBotFlags2, BOTFLAG2_DISABLE_AIMING ) : FMATH_SETBITMASK( m_nBotFlags2, BOTFLAG2_DISABLE_AIMING ); }
	BOOL TryToBreakLimb( const CFVec3A *pvPos_WS=NULL );	// will try to break a limb on the bot.  If a position is given, tries to break the bone nearest that point
	void BreakLimb( const CFVec3A *pvPos_WS=NULL );
	void BreakAllLimbs( void );

	void AddRotationalImpact( f32 fUnitIntensity );

	f32  RespawnEffects_GetRemainingTime( void ) const { FASSERT( IsCreated() ); return m_bRespawnEffectInitialized ?  m_fRespawnEffectTimer : 0.0f; };

	void RepairAllLimbs( BOOL bRepairDestroyed=FALSE );

	void RespawnEffect_Kill( void );						// will immediately kill all effects

	FINLINE void SetDoubleJumpTimer( f32 fTime )	{ m_fDoubleJumpTimer = fTime; }
	FINLINE BOOL HaveTimeToDoubleJump( void )		{ return m_fDoubleJumpTimer > 0.0f; }

	// bot in pieces
	// Bot in pieces

	void BreakIntoPieces( const f32 &fTimeInPieces, BOOL bImmediately = FALSE );

	//// Pass a time to keep Glitch in pieces.  -1.0f puts him into pieces until BreakIntoPieces( -1.0f ) is called again.
	FINLINE BOOL IsFallAnimationDone( void ) { return m_bFallAnimDone; }

	//Is Glitch being taken apart, put together or on the ground?
	FINLINE BOOL IsOnGroundInPieces( void )  { return m_bOnGround; } 
	FINLINE BOOL IsInPieces( void ) { return m_bInPieces && (m_bFallDown || m_bOnGround); }

	//For the Coliseum Minigame.
	FINLINE void OverrideTimeInPieces(f32 fNewTime) { m_fTimeInPieces = fNewTime; }

	//Does glitch still recieve collisions while in pieces? Default = FALSE.
	void EnableCollisionWithPieces(BOOL bEnable=FALSE) { m_bColWithPieces = bEnable; }

    
//----------------------------------------------------------------------------------------------------------------------------------
// Protected Functions:
//----------------------------------------------------------------------------------------------------------------------------------
protected:

	// Creation:
	CBot( void );
	BOOL Create( const CBotDef *pBotDef, s32 nPlayerIndex=-1, BOOL bInstallDataPort=FALSE, cchar *pszEntityName=NULL, const CFMtx43A *pMtx=NULL, cchar *pszAIBuilderName=NULL );

	void DataPort_SetupTetherShockInfo( void );

	void MakeLimbInvincible( LimbCode_e nLimbCode, u32 nLimbType, CBotBuilder *pBuilder );

	virtual BOOL ClassHierarchyBuild( void );
	virtual BOOL ClassHierarchyBuilt( void );
	virtual void ClassHierarchyDestroy( void );
	virtual void ClassHierarchyDrawEnable( BOOL bDrawingHasBeenEnabled );
	CEntityBuilder *GetLeafClassBuilder( void );

	virtual CFMtx43A* ClassHierarchyAttachChild( CEntity *pChildEntity, cchar *pszAttachBoneName );
	virtual void	  ClassHierarchyAddToWorld( void );
	virtual void	  ClassHierarchyRemoveFromWorld( void );
	virtual void      ClassHierarchyRelocated( void *pIdentifier );
	virtual void      ClassHierarchyWork( void );
	virtual BOOL SetNewMountAimVec( const CFVec3A *pUnitMountAimVec );
	
	static BOOL ReadBotInfoFile( const FGameDataMap_t *pMapTable, cchar *pszFileName );
	static BOOL ReadBotIdleFile( BotInfo_Idle_t** ppBotIdleArray, u32* pnBotIdleCount, cchar *pszFileName );


	// Flag-setting functions:
	FINLINE void SetBotFlag_Enemy( void ) { FMATH_SETBITMASK( m_nBotFlags, BOTFLAG_ENEMY ); }
	FINLINE void SetBotFlag_MeshCollideOnly( void ) { FMATH_SETBITMASK( m_nBotFlags, BOTFLAG_USE_MESH_FOR_COLLISION ); }
	FINLINE void SetBotFlag_MovingBackwards( void ) { FMATH_SETBITMASK( m_nBotFlags, BOTFLAG_XLAT_BACKWARDS ); }
	FINLINE void SetBotFlag_IgnoreControls( void ) { FMATH_SETBITMASK( m_nBotFlags, BOTFLAG_IGNORE_CONTROLS ); }
	
	FINLINE void SetJumping(void) { FMATH_SETBITMASK( m_uJumpFlags, JUMPFLAG_IS_JUMPING); }
	FINLINE void SetJumpPadBoost(void) { FMATH_SETBITMASK( m_uJumpFlags, JUMPFLAG_JUMPPAD_BOOST); }
	FINLINE void SetJumpPadDenialFlag(void) { FMATH_SETBITMASK( m_uJumpFlags, JUMPFLAG_CANNOT_USE_JUMPPAD); }

    FINLINE void SetDroppedWeapon( void ) { FASSERT( IsCreated() ); FMATH_SETBITMASK( m_nBotFlags2, BOTFLAG2_DROPPED_WEAPON ); }

	// Flag-clearing functions:
	FINLINE void ClearBotFlag_Enemy( void ) { FMATH_CLEARBITMASK( m_nBotFlags, BOTFLAG_ENEMY ); }
	FINLINE void ClearBotFlag_MeshCollideOnly( void ) { FMATH_CLEARBITMASK( m_nBotFlags, BOTFLAG_USE_MESH_FOR_COLLISION ); }
	FINLINE void ClearBotFlag_MovingBackwards( void ) { FMATH_CLEARBITMASK( m_nBotFlags, BOTFLAG_XLAT_BACKWARDS ); }
	FINLINE void ClearBotFlag_IgnoreControls( void ) { FMATH_CLEARBITMASK( m_nBotFlags, BOTFLAG_IGNORE_CONTROLS ); }
	FINLINE void ClearJumping(void) { FMATH_CLEARBITMASK( m_uJumpFlags, JUMPFLAG_IS_JUMPING); }
	FINLINE void ClearJumpPadBoost(void) { FMATH_CLEARBITMASK( m_uJumpFlags, JUMPFLAG_JUMPPAD_BOOST); }
	FINLINE void ClearJumpPadDenialFlag(void) { FMATH_CLEARBITMASK( m_uJumpFlags, JUMPFLAG_CANNOT_USE_JUMPPAD); }


	// Tag points:
	void TagPoint_CreateFromMountPos( void );
	BOOL TagPoint_CreateFromBoundingSphereCenter( void );
	void TagPoint_CreateFromBuiltInVector( const CFVec3A *pTagPoint_WS );
	BOOL TagPoint_CreateFromBoneArray( const u8 *pnTagBoneIndexArray, cchar **apszBoneNameTable );
	void TagPoint_Update( void );
	

	// Animation control:
	FINLINE void AttachAnim( u32 nTapIndex, CFAnimSource *pAnimSource ) { m_Anim.BaseAnim_Attach( nTapIndex, pAnimSource ); }
	FINLINE CFAnimSource *GetAttachedAnim( u32 nTapIndex ) const { return m_Anim.BaseAnim_GetAttached( nTapIndex ); }

	FINLINE void EnableAllBones( u32 nTapIndex ) { m_Anim.BaseAnim_EnableAllBones( nTapIndex ); }
	FINLINE void DisableAllBones( u32 nTapIndex ) { m_Anim.BaseAnim_DisableAllBones( nTapIndex ); }
	FINLINE void UpdateBoneMask( u32 nTapIndex, cchar *pszBoneName, BOOL bDriveBone ) { m_Anim.BaseAnim_UpdateBoneMask( nTapIndex, pszBoneName, bDriveBone ); }
	FINLINE void UpdateBoneMask( u32 nTapIndex, const u8 *pnBoneNameIndexTable, BOOL bDriveBonesInList ) { m_Anim.BaseAnim_UpdateBoneMask( nTapIndex, pnBoneNameIndexTable, bDriveBonesInList ); }
	FINLINE BOOL IsDrivingBone( u32 nTapIndex, cchar *pszBoneName ) const { return m_Anim.BaseAnim_IsDrivingBone( nTapIndex, pszBoneName ); }

	FINLINE void SetControlValue( u32 nControlIndex, f32 fVal ) { m_Anim.BaseAnim_SetControlValue( nControlIndex, fVal ); }
	FINLINE f32 GetControlValue( u32 nControlIndex ) const { return m_Anim.BaseAnim_GetControlValue( nControlIndex ); }
	FINLINE BOOL IsControlSmoothingEnabled( u32 nControlIndex ) const { return m_Anim.BaseAnim_IsControlSmoothingEnabled( nControlIndex ); }
	FINLINE void EnableControlSmoothing( u32 nControlIndex ) { m_Anim.BaseAnim_EnableControlSmoothing( nControlIndex ); }
	FINLINE void DisableControlSmoothing( u32 nControlIndex ) { m_Anim.BaseAnim_DisableControlSmoothing( nControlIndex ); }

	FINLINE f32 GetTotalTime( u32 nTapIndex ) const { return m_Anim.BaseAnim_GetTotalTime( nTapIndex ); }
	FINLINE f32 GetOOTotalTime( u32 nTapIndex ) const { return m_Anim.BaseAnim_GetOOTotalTime( nTapIndex ); }

	FINLINE void ZeroTime( u32 nTapIndex ) { m_Anim.BaseAnim_ZeroTime( nTapIndex ); }

	FINLINE f32 GetTime( u32 nTapIndex ) const { return m_Anim.BaseAnim_GetTime( nTapIndex ); }
	FINLINE BOOL DeltaTime( u32 nTapIndex, f32 fDeltaTime=FLoop_fPreviousLoopSecs, BOOL bClamp=FALSE ) { return m_Anim.BaseAnim_DeltaTime( nTapIndex, fDeltaTime, bClamp ); }
	FINLINE void UpdateTime( u32 nTapIndex, f32 fNewTime ) { m_Anim.BaseAnim_UpdateTime( nTapIndex, fNewTime ); }

	FINLINE f32 GetUnitTime( u32 nTapIndex ) const { return m_Anim.BaseAnim_GetUnitTime( nTapIndex ); }
	FINLINE BOOL DeltaUnitTime( u32 nTapIndex, f32 fDeltaTime=FLoop_fPreviousLoopSecs, BOOL bClamp=FALSE ) { return m_Anim.BaseAnim_DeltaUnitTime( nTapIndex, fDeltaTime, bClamp ); }
	FINLINE void UpdateUnitTime( u32 nTapIndex, f32 fNewTime ) { m_Anim.BaseAnim_UpdateUnitTime( nTapIndex, fNewTime ); }


	// Animation control:
	FINLINE void AttachAnim( AnimStackIndex_e nIndex, CFAnimSource *pAnimSource ) { FASSERT(nIndex>=0 && nIndex<ASI_COUNT ); m_Anim.BaseAnim_Attach( m_anAnimStackIndex[nIndex], pAnimSource ); }
	FINLINE CFAnimSource *GetAttachedAnim( AnimStackIndex_e nIndex ) const { FASSERT(nIndex>=0 && nIndex<ASI_COUNT ); return m_Anim.BaseAnim_GetAttached( m_anAnimStackIndex[nIndex] ); }

	FINLINE void EnableAllBones( AnimStackIndex_e nIndex ) { FASSERT(nIndex>=0 && nIndex<ASI_COUNT ); m_Anim.BaseAnim_EnableAllBones( m_anAnimStackIndex[nIndex] ); }
	FINLINE void DisableAllBones( AnimStackIndex_e nIndex ) { FASSERT(nIndex>=0 && nIndex<ASI_COUNT ); m_Anim.BaseAnim_DisableAllBones( m_anAnimStackIndex[nIndex] ); }
	FINLINE void UpdateBoneMask( AnimStackIndex_e nIndex, cchar *pszBoneName, BOOL bDriveBone ) { FASSERT(nIndex>=0 && nIndex<ASI_COUNT ); m_Anim.BaseAnim_UpdateBoneMask( m_anAnimStackIndex[nIndex], pszBoneName, bDriveBone ); }
	FINLINE void UpdateBoneMask( AnimStackIndex_e nIndex, const u8 *pnBoneNameIndexTable, BOOL bDriveBonesInList ) { FASSERT(nIndex>=0 && nIndex<ASI_COUNT ); m_Anim.BaseAnim_UpdateBoneMask( m_anAnimStackIndex[nIndex], pnBoneNameIndexTable, bDriveBonesInList ); }
	FINLINE BOOL IsDrivingBone( AnimStackIndex_e nIndex, cchar *pszBoneName ) const { FASSERT(nIndex>=0 && nIndex<ASI_COUNT ); return m_Anim.BaseAnim_IsDrivingBone( m_anAnimStackIndex[nIndex], pszBoneName ); }

	FINLINE void SetControlValue( AnimStackIndex_e nIndex, f32 fVal ) { FASSERT(nIndex>=0 && nIndex<ASI_COUNT ); m_Anim.BaseAnim_SetControlValue( m_anAnimStackIndex[nIndex], fVal ); }
	FINLINE f32 GetControlValue( AnimStackIndex_e nIndex ) const { FASSERT(nIndex>=0 && nIndex<ASI_COUNT ); return m_Anim.BaseAnim_GetControlValue( m_anAnimStackIndex[nIndex] ); }
	FINLINE BOOL IsControlSmoothingEnabled( AnimStackIndex_e nIndex ) const { FASSERT(nIndex>=0 && nIndex<ASI_COUNT ); return m_Anim.BaseAnim_IsControlSmoothingEnabled( m_anAnimStackIndex[nIndex] ); }
	FINLINE void EnableControlSmoothing( AnimStackIndex_e nIndex ) { FASSERT(nIndex>=0 && nIndex<ASI_COUNT ); m_Anim.BaseAnim_EnableControlSmoothing( m_anAnimStackIndex[nIndex] ); }
	FINLINE void DisableControlSmoothing( AnimStackIndex_e nIndex ) { FASSERT(nIndex>=0 && nIndex<ASI_COUNT ); m_Anim.BaseAnim_DisableControlSmoothing( m_anAnimStackIndex[nIndex] ); }

	FINLINE f32 GetTotalTime( AnimStackIndex_e nIndex ) const { FASSERT(nIndex>=0 && nIndex<ASI_COUNT ); return m_Anim.BaseAnim_GetTotalTime( m_anAnimStackIndex[nIndex] ); }
	FINLINE f32 GetOOTotalTime( AnimStackIndex_e nIndex ) const { FASSERT(nIndex>=0 && nIndex<ASI_COUNT ); return m_Anim.BaseAnim_GetOOTotalTime( m_anAnimStackIndex[nIndex] ); }

	FINLINE void ZeroTime( AnimStackIndex_e nIndex ) { FASSERT(nIndex>=0 && nIndex<ASI_COUNT ); m_Anim.BaseAnim_ZeroTime( m_anAnimStackIndex[nIndex] ); }

	FINLINE f32 GetTime( AnimStackIndex_e nIndex ) const { FASSERT(nIndex>=0 && nIndex<ASI_COUNT ); return m_Anim.BaseAnim_GetTime( m_anAnimStackIndex[nIndex] ); }
	FINLINE BOOL DeltaTime( AnimStackIndex_e nIndex, f32 fDeltaTime=FLoop_fPreviousLoopSecs, BOOL bClamp=FALSE ) { FASSERT(nIndex>=0 && nIndex<ASI_COUNT ); return m_Anim.BaseAnim_DeltaTime( m_anAnimStackIndex[nIndex], fDeltaTime, bClamp ); }
	FINLINE void UpdateTime( AnimStackIndex_e nIndex, f32 fNewTime ) { FASSERT(nIndex>=0 && nIndex<ASI_COUNT ); m_Anim.BaseAnim_UpdateTime( m_anAnimStackIndex[nIndex], fNewTime ); }

	FINLINE f32 GetUnitTime( AnimStackIndex_e nIndex ) const { FASSERT(nIndex>=0 && nIndex<ASI_COUNT ); return m_Anim.BaseAnim_GetUnitTime( m_anAnimStackIndex[nIndex] ); }
	FINLINE BOOL DeltaUnitTime( AnimStackIndex_e nIndex, f32 fDeltaTime=FLoop_fPreviousLoopSecs, BOOL bClamp=FALSE ) { FASSERT(nIndex>=0 && nIndex<ASI_COUNT ); return m_Anim.BaseAnim_DeltaUnitTime( m_anAnimStackIndex[nIndex], fDeltaTime, bClamp ); }
	FINLINE void UpdateUnitTime( AnimStackIndex_e nIndex, f32 fNewTime ) { FASSERT(nIndex>=0 && nIndex<ASI_COUNT ); m_Anim.BaseAnim_UpdateUnitTime( m_anAnimStackIndex[nIndex], fNewTime ); }


	// Built-in summer (the summer that's automatically built by m_Anim):
	FINLINE void Summer_EnableAllBones( void ) { m_Anim.Summer_EnableAllBones(); }
	FINLINE void Summer_DisableAllBones( void ) { m_Anim.Summer_DisableAllBones(); }
	FINLINE void Summer_UpdateBoneMask( cchar *pszBoneName, BOOL bDriveBone ) { m_Anim.Summer_UpdateBoneMask( pszBoneName, bDriveBone ); }
	FINLINE void Summer_UpdateBoneMask( const u8 *pnBoneNameIndexTable, BOOL bDriveBonesInList ) { m_Anim.Summer_UpdateBoneMask( pnBoneNameIndexTable, bDriveBonesInList ); }
	FINLINE BOOL Summer_IsDrivingBone( cchar *pszBoneName ) const { return m_Anim.Summer_IsDrivingBone( pszBoneName ); }

	FINLINE void Summer_SetControlValue( f32 fVal ) { m_Anim.Summer_SetControlValue( fVal ); }
	FINLINE f32 Summer_GetControlValue( void ) const { return m_Anim.Summer_GetControlValue(); }
	FINLINE BOOL Summer_IsControlSmoothingEnabled( void ) const { return m_Anim.Summer_IsControlSmoothingEnabled(); }
	FINLINE void Summer_EnableControlSmoothing( void ) { m_Anim.Summer_EnableControlSmoothing(); }
	FINLINE void Summer_DisableControlSmoothing( void ) { m_Anim.Summer_DisableControlSmoothing(); }

	FINLINE f32	GetLimitedTopSpeed( void ) const { return FMATH_MIN(m_fUnitTopSpeedLimiter, m_fUnitLimpSpeedModifier ); };

	// Velocity:
	void VelocityHasChanged( void );
	BOOL HandleVelocityImpulses( void );


	// Rotation:
	f32 TakeMouseLookDelta( BOOL bPitch );
	void HandlePitchMovement( void );
	void HandleYawMovement( void );

	//some of the functions that might be usefull to base classes 
	void HandleHopRollStartleGeneric( void );	//derived classes can call this from their work if they want there bot to hop,roll,or startle
	void StartDoubleJumpGeneric(BOOL bAddToVelocity,
								u32 uTapJumpTuck,		//ANIMTAP_JUMP_TUCK
								u32 uTapJumpUnTuck,		//ANIMTAP_JUMP_UNTUCK
								u32 uControlJumpLaunch,	//ANIMCONTROL_JUMP_LAUNCH
								u32 uControlJumpFly,	//ANIMCONTROL_JUMP_FLY
								u32 uControlJumpTuck);	//ANIMCONTROL_JUMP_TUCK



	void ChangeMountYaw( f32 fNewMountYaw_WS );


	// Controls:
	void ZeroControls( void );
	void ParseControls( void );
	void ComputeXlatStickInfo( void );
	void UpdateIdle();


	// Ground movement:
	void HandleCollision( void );
	virtual void MakeFootImpactDust( BOOL bLeftFoot, BOOL bRightFoot, f32 fUnitIntensity, const CFVec3A *pEmissionUnitDir_WS );

	void HandleGroundTranslation( void );
	void HandleGroundAnimations( void );
	void HandleAirTranslation( void );
	BOOL HandleAirAnimations( void );
	void HandleHipsAnimation( void );

	void UpdateTractionMode( void );
	void ComputeNewGroundVelocity( void );
	BOOL ComputeMovingSurfaceVector( void );
//	void AdjustForCollision( const CFVec3A *pPushVector_WS );
	virtual BOOL NotifyBotCollision( CBot *pBot )	{ return TRUE; };		// notifies the bot it has collided with another bot.  if FALSE is returned, normal collision response is skipped.

	void HandleStooping( void );
	virtual void NotifyFootDown( BOOL bLeftFoot, BOOL bRightFoot, f32 fUnitMag )	{};

	void HandleSleeping( void );

	// Weapons:
	virtual void HandleTargeting( void );
	void HandleWeaponReloadingAnimations( void );

	void HandleWeaponReloadingAnimations_FullyAutomatic( void );
	void HandleWeaponReloadingAnimations_Rocket1( void );
	void HandleWeaponReloadingAnimations_Tether( void );
	void HandleWeaponReloadingAnimations_Shell( void );
	void HandleWeaponReloadingAnimations_Clip( void );
	void AbortReloadImmediately( void );

	virtual void StartedUsingLeftArmToReload( void ) {}

	virtual void ComputeWeaponMuzzlePoint_WS( CFVec3A *pMuzzlePt );
	virtual void ComputeWeaponMuzzleVec_WS( CFVec3A *pMuzzleVec );

	
    // Still weapons, used to control dummy weapons if necessary:
	BOOL WeaponsCreate( u32 nSlot, u32 nWpnType, u32 nDuplicates, u32 nUpgradeLevel );						// will create the weapons and set owner to this bot, will not attach them
	void WeaponsDestroy( u32 nSlot=-1 );																	// will destroy all weapons if nSlot != 0 or 1
	void WeaponsWork( u32 nSlot=-1 );																		// will call work on all weapons if nSlot != 0 or 1
	void WeaponsAddToWorld( u32 nSlot=-1);																	// will call atw on all weapons if nSlot != 0 or 1
	void WeaponsRemoveFromWorld( u32 nSlot=-1);																// will call rfw on all weapons if nSlot != 0 or 1
	void WeaponsDrawEnable( BOOL bDrawingHasBeenEnabled, u32 nSlot=-1 );									// will call drawenable on all weapons if nSlot != 0 or 1


	// Power:
	void Power_Work( void );


	// UserAnim slot access:
	void UserAnim_LockWork();


	// Remote control:
	void DataPort_Work( void );
	virtual void PullThePlug( BOOL bMakeSoundEffects=TRUE );	// called as the bot being possessed is be not possessed anymore
	virtual void AboutToPossessBot( CBot *pBotToBePossessed )	{}// called from the bot about to be possessed to the bot about to be the possessor ( Called at the top of Possess() before anything has happened )
	virtual void HavePossessedBot( CBot *pBotBeingPossessed )	{ SetInvincible( TRUE ); }// called from the bot being possessed to the bot doing the possessing ( Called at the bottom of Possess() )
	virtual void HaveUnPossessedBot( CBot *pBotBeingLost )		{ SetInvincible( FALSE ); }// called from the bot being possessed to the bot doing the possessing ( Called at the bottom of PullThePlug() )


	// Detail Control Modes:
	FINLINE void HeadSetLooking( void ) { FASSERT( IsCreated() ); FMATH_SETBITMASK( m_nBotFlags, BOTFLAG_HEAD_IS_LOOKING ); }
	FINLINE void HeadClearLooking( void ) { FASSERT( IsCreated() ); FMATH_CLEARBITMASK( m_nBotFlags, BOTFLAG_HEAD_IS_LOOKING ); }
	void HandleHeadLook( void );
	void SetCameraOverride( BOOL bOverride, f32 fBlendTime, BOOL bPitched=FALSE, const CFVec3A &vLookAt=CFVec3A::m_Null, const CFVec3A &vCamPos=CFVec3A::m_Null );


	// Zipline:
	virtual BOOL CanAttachToZipLine( void );
	void ZipLineVerletSetup( void );
	void ZipLineVerletWork( const CFVec3A *pHandPos_WS, CFMtx43A *OffsetMatrix );

	void SimpleVerlet( CFVec3A *pCurrent, CFVec3A *pPrevious, const CFVec3A *pForce, const f32 &fDampenFactor = 0.0f );

	static BOOL TrackerCollisionCallback( CFWorldTracker *pTracker, FVisVolume_t *pVolume );
	static u32  NewTrackerCollisionCallback( CFWorldTracker *pTracker );

	// Lights:
	virtual void UpdateSpotLight( void );


	// Sounds:
	virtual void PlayFootSound( BOOL bLeft, BOOL bRight, SurfaceType_e nSurfaceType );


	// Misc:
	void DetachAllSwarmers( void );


	BOOL RespawnEffect_Init( f32 fParticleIntensity, s32 nMeshAttachBone, const CFVec3A &vMeshAttachPos, f32 fMeshStartScale, f32 fMeshFinishScale );
	void RespawnEffect_Start( void );
	void RespawnEffect_Work( void );
	void RespawnEffect_Destroy( void );						// cleanup


	// Melee
	// Note:  You either need to supply a pointer to a WS position to be used when swinging, or provide one on each call to MeleeStartAttack & MeleeSwing
	void MeleeInit( MeleeData_t *pMeleeData, f32 fRadius, const CDamageProfile *pDamageProfile, const CFVec3A *pvAttackPos_WS=NULL, MeleeData_t *pMasterData=NULL, u32 uMaxHitPerSwing=MELEE_MAX_ENTITIES_PER_SWING);
	void MeleeStartAttack( MeleeData_t *pMeleeData, const CFVec3A *pvAttackPos_WS=NULL );
	void MeleeSwing( MeleeData_t *pMeleeData, const CFVec3A *pvAttackPos_WS=NULL );
	virtual void MeleeCollision( CEntity *pEntity, FCollImpact_t *pImpact, BOOL bFirstHit );
	static u32 _MeleeHitTrackerCB( CFWorldTracker *pTracker );
	

	// Bot in pieces
	void BotInPiecesWork( void );
	BOOL BotInPieces_InPiecesAnimBoneCB( CFMtx43A &rNewMtx, const CFMtx43A &rParentMtx, const CFMtx43A &rBoneMtx );
	BOOL BotInPieces_RecoverAnimBoneCB( CFMtx43A &rNewMtx, const CFMtx43A &rParentMtx, const CFMtx43A &rBoneMtx );

	// Player damage
	void PlayerDamage_StartDamageEffect( s32 nBone );
	void PlayerDamage_ClearAllDamageEffects( void );
	void PlayerDamage_RepairAllDamageEffects( void );
	void PlayerDamage_StartDeathEffect( void );
	BOOL PlayerDamage_DeathWork( void );				// will return TRUE if handled, otherwise, normal bot deathwork should be called.
	void PlayerDamage_StartDeath( void );


//----------------------------------------------------------------------------------------------------------------------------------
// Private Functions:
//----------------------------------------------------------------------------------------------------------------------------------
private:

	BOOL _LoadDataPortResources( void );
	void _ClearDataMembers( void );
	void _DrawStatic( void );
	void _EnablePossessionTextBox( BOOL bEnable );
	void _InitPossessionOutOfRangeWarningTextBox( void );
	void _InitTransmissionLostTextBox( void );
	void _Work_UnplugTether( void );
	void _AdjustAimVectorToCompensateForCamLookatDeviation( const CFVec3A *pCamFrontUnitVec_WS, CFVec3A *pUnitAimVec_WS );
	void _LoadSoundResource( u32 uBankToLoad );
	void _OverrideSpotLight( void ); //called from within UpdateSpotLight()
	LightOverrideState_e _DetermineNewSpotLightOverrideState( void );

	static BOOL _AITalkStartCallback(u32 uControl, void *pvData1, void *pvData2);
	static void _AITalkEndCallback(CBotTalkInst *pTalkInst, CBot *pBot);

	static BOOL ActionCallback(CFWorldTracker *pTracker, FVisVolume_t *pVolume );

	virtual void _StartVelocityJump( const CFVec3A *pJumpVelocity_WS ) {FASSERT_NOW;};

	static void _StopBotTalkCB( CBotTalkInst *pTalkInst, CBot *pBot );

	void _PushAwayVehicles( void );
	static BOOL _PushAwayVehiclesCB( CFWorldTracker *pTracker, FVisVolume_t *pVolume );


	friend class CBotPart;
	friend class CBotPartPool;
	friend class CBotPartMgr;
	friend class CFloorSentry;
	friend class CWallSentry;
	friend class CPillbox;
	friend class CRatGun;
	friend class CEJumpPad;
	friend class CWeaponRecruiter;


	FCLASS_STACKMEM_ALIGN( CBot );
} FCLASS_ALIGN_SUFFIX;




//----------------------------------------------------------------------------------------------------------------------------------
// Inline Implementations:
//----------------------------------------------------------------------------------------------------------------------------------

// Converts a world space vector into model space.
FINLINE CFVec3A &CBot::WS2MS( CFVec3A &rVec_MS, const CFVec3A &rVec_WS ) {
	rVec_MS.ReceiveRotationY( rVec_WS, -m_fMountYawSin_WS, m_fMountYawCos_WS );
	return rVec_MS;
}


// Converts a model space vector into world space.
FINLINE CFVec3A &CBot::MS2WS( CFVec3A &rVec_WS, const CFVec3A &rVec_MS ) {
	rVec_WS.ReceiveRotationY( rVec_MS, m_fMountYawSin_WS, m_fMountYawCos_WS );
	return rVec_WS;
}


// Converts the world space vector into model space.
FINLINE CFVec3A &CBot::WS2MS( CFVec3A &rVec_WS ) {
	rVec_WS.RotateY( -m_fMountYawSin_WS, m_fMountYawCos_WS );
	return rVec_WS;
}


// Converts the model space vector into world space.
FINLINE CFVec3A &CBot::MS2WS( CFVec3A &rVec_MS ) {
	rVec_MS.RotateY( m_fMountYawSin_WS, m_fMountYawCos_WS );
	return rVec_MS;
}


FINLINE void CBot::DataPort_AttachEntity_WS( CEntity *pEntityToAttach ) {
	FASSERT( IsCreated() );
	pEntityToAttach->Attach_ToParent_WS( this, m_pBotInfo_Gen->pszDataPortBoneName );
}


FINLINE void CBot::DataPort_AttachEntity_PS( CEntity *pEntityToAttach ) {
	FASSERT( IsCreated() );
	pEntityToAttach->Attach_ToParent_PS( this, m_pBotInfo_Gen->pszDataPortBoneName );
}


FINLINE void CBot::DataPort_AttachEntityUsingUnitMtx_PS( CEntity *pEntityToAttach, const CFMtx43A *pUnitMtx_PS ) {
	FASSERT( IsCreated() );
	pEntityToAttach->Attach_UnitMtxToParent_PS( this, m_pBotInfo_Gen->pszDataPortBoneName, pUnitMtx_PS );
}


FINLINE void CBot::DataPort_AttachEntityUsingUnitMtx_PS_AndScale_WS( CEntity *pEntityToAttach, const CFMtx43A *pUnitMtx_PS, f32 fNewEntityScale_WS ) {
	FASSERT( IsCreated() );
	pEntityToAttach->Attach_UnitMtxToParent_PS_NewScale_WS( this, m_pBotInfo_Gen->pszDataPortBoneName, pUnitMtx_PS, fNewEntityScale_WS );
}


FINLINE void CBot::DataPort_AttachEntityUsingUnitMtx_PS_AndScale_PS( CEntity *pEntityToAttach, const CFMtx43A *pUnitMtx_PS, f32 fNewEntityScale_PS ) {
	FASSERT( IsCreated() );
	pEntityToAttach->Attach_UnitMtxToParent_PS_NewScale_PS( this, m_pBotInfo_Gen->pszDataPortBoneName, pUnitMtx_PS, fNewEntityScale_PS );
}


// Sets pitch and yaw head swiveling range for head look functionality.
// Note that head may be allowed to pitch or yaw to a greater angle
// for reasons of hysteresis (prevents head from snapping back and forth
// when at edges of range).
FINLINE void CBot::SetHeadLookRanges( const f32 fYaw, const f32 fPitch ) {
	m_fHeadLookMaxYawCos = fmath_Cos( fYaw );
	m_fHeadLookMaxPitchCos = fmath_Cos( fPitch );
	m_fHeadLookHystYawCos = fmath_Cos( fYaw + BOT_HEAD_LOOK_HYSTERESIS_ANGLE );
	m_fHeadLookHystPitchCos = fmath_Cos( fPitch + BOT_HEAD_LOOK_HYSTERESIS_ANGLE );
}


// sets world space location that bot should attempt to look at.
// should be used when setting a single WS location, not for
// continuous updating (use UpdateHeadLookLocation)
FINLINE void CBot::SetHeadLookLocation( const CFVec3A &vLoc ) {
	if( vLoc != m_vHeadLookPoint_WS )
	{
		m_fHeadLookSlerp = 0.0f; 
		m_vHeadLookPoint_WS.Set( vLoc ); 
		m_HeadLookOriginQuat = m_HeadLookCurrentQuat;
	}
}


// update the current head look location.  Use when head look mode
// is already turned on and initial location has been set with SetHeadLookLocation()
FINLINE void CBot::UpdateHeadLookLocation( const CFVec3A &vLoc ) {
	m_vHeadLookPoint_WS.Set( vLoc ); 
}


FINLINE f32 CBot::ComputeCollSphereY( void ) const {
	FASSERT( IsCreated() );

	if( !HasNoLegs() ) {
		return m_pBotInfo_Gen->fCollSphere1Y_MS;
	} else {
		return m_pBotInfo_Gen->fCollSphere1Y_MS + m_pBotInfo_Gen->fNoLegsDeltaCollSphereY_MS;
	}
}




//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CBotBuilder
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

FCLASS_ALIGN_PREFIX class CBotBuilder : public CEntityBuilder {


//----------------------------------------------------------------------------------------------------------------------------------
// Public Data:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	typedef enum {
		_NPC_WEAPON_HAND = 0,
		_NPC_WEAPON_LASER,
		_NPC_WEAPON_SPEW,
		_NPC_WEAPON_FLAMER,
		_NPC_WEAPON_GRENADE,
		_NPC_WEAPON_BLASTER,
		_NPC_WEAPON_RIVET,
		_NPC_WEAPON_MAGMABOMB,
		_NPC_WEAPON_TETHER,
		_NPC_WEAPON_EMP,
		_NPC_WEAPON_STAFF,
		_NPC_WEAPON_ROCKET,
		_NPC_WEAPON_RECRUITER,

		_NPC_WEAPON_COUNT,
	} NPCWeapon_e;


	enum {
		BOT_BUILDER_FLAG_SPOTLIGHT_ON					= 0x00000001,	// TRUE if spotlight should be on for this bot
		BOT_BUILDER_INST_CANNOT_BE_RECRUITED			= 0x00000002,	// This bot instance cannot be recruited by the recruiter grenade
		BOT_BUILDER_CLASS_CAN_BE_RECRUITED				= 0x00000004,	// This class of bots can be recruited
		BOT_BUILDER_DPORT_NOT_NEEDED_FOR_RECRUITMENT	= 0x00000008,	// If TRUE, this bot class does not require a data port to be recruited
		BOT_BUILDER_SHOWS_UP_ON_RADAR					= 0x00000010,	// TRUE if this bot is to show up on Glitch's radar

		BOT_BUILDER_FLAG_NONE							= 0x00000000
	};


	const CBotDef *m_pBotDef;				// Type information about this bot (remains constant for the entire level)
	s32 m_nOwnerPlayerIndex;				// Index of player who owns this bot permanently (-1=none)
	BOOL m_bDataPortInstalled;				// This bot has a data port
	BOOL m_bDataPortOpen;					// This bot's data port is open
	BOOL m_bSleepAtStart;					// This bot should be sleeping apon entering the world
	BOOL m_bSetBotFlag_PowerDownUntilPosessed;
	f32 m_fRunMultiplier;

	f32 m_fDisguiseTime;					// Amount of time a possessed bot will be disguised (-1=use class's default)
	f32 m_fPossessionDist;					// Maximum distance a possessed bot can travel (-1=use class's default)

	cchar *m_pszTalkDataFileName;			// Name of the BTA to load via CTalkSystem2.
	CBotTalkInst *m_pBuilderTalkInst;
	u32 m_uBuilderTalkInstFlags;
	u32	m_uMeshVersionOverride;				// Override the Mesh version (incase there are more than one version of the bot's mesh
	cchar *m_pszMeshReplacement;			// Replace the real mesh with this one. Bones and anims must match.  
	NPCWeapon_e m_anNPCWeaponType[2];		// what will weapon 0 be?
	u32 m_uFlags;							// See BOT_BUILDER_FLAG_* for info
	BOOL m_bBabble;							// TRUE if the bot should babble (voice distorted) all the time

	u32 m_nInvincibleLimbCodeBitMask;		// Bit mask of CBot::LimbCode_e bits, set to indicate which limbs are invincible





//----------------------------------------------------------------------------------------------------------------------------------
// Public Functions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	FINLINE CBotBuilder() {}
	virtual void SetDefaults( u64 nEntityTypeBits, u64 nEntityLeafTypeBit, cchar *pszEntityType );


	static CWeapon* AllocNPCWeaponOfType(NPCWeapon_e nNPCWeaponType);
	static BOOL CreateNPCWeaponOfType(CWeapon* pWeap, NPCWeapon_e nNPCWeaponType );




//----------------------------------------------------------------------------------------------------------------------------------
// Protected Functions:
//----------------------------------------------------------------------------------------------------------------------------------
protected:

	virtual BOOL InterpretTable( void );
	virtual BOOL PostInterpretFixup( void );


	FCLASS_STACKMEM_ALIGN( CBotBuilder );
} FCLASS_ALIGN_SUFFIX;

#endif



// Notes:
//
// CBotControl Input:
//   m_fRotateCW - Torso yaw
//   m_DesiredGazePoint_WS - Specifies desired gaze direction in world space
//   m_DesiredTargetPoint_WS - Specifies desired targeting direction in world space
//   m_bUseDesiredGaze - Indicates that m_DesiredGazePoint_WS should drive the bot's gaze
//
// Info:
//   m_MtxToWorld - Bot torso matrix
//   m_GazeUnitDir_WS - Points in the direction the bot is looking
//   m_TargetUnitDir_WS - Points in the direction the bot is targeting
//   m_bTargetLock - TRUE when m_DesiredTargetPoint_WS is within targeting range & field
//

