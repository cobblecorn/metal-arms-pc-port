//////////////////////////////////////////////////////////////////////////////////////
// entity.h - Main game entity class.
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
// 04/10/02 Ranck       Created.
//////////////////////////////////////////////////////////////////////////////////////

#ifndef _ENTITY_H_
#define _ENTITY_H_ 1

#include "fang.h"
#include "fworld.h"
#include "fmath.h"
#include "flinklist.h"
#include "guid.h"
#include "entitycontrol.h"
#include "fgamedata.h"
#include "GoodieBag.h"
#include "faudio.h"
#include "tripwire.h"
#include "fverlet.h"
#include "fexplosion.h"
#include "fcheckpoint.h"

#define ENTITY_TRIPWIRE_EVENT_NAME				"Tripwire"
#define ENTITY_ERROR_MESH_NAME					"gpdmwpnunkn"
#define ENTITY_LEVEL_START_CHECKPOINT_INDEX		0
#define ENTITY_MAX_VERLET_TACKS_PER_OBJECT		64
#define ENTITY_DEFAULT_WIRE_POOL_COUNT			30



#define ENTITY_BUILDER_SET_PARENT_CLASS_DEFAULTS( szParentClassName, nOurEntityClassBit, pszEntityType ) \
	szParentClassName::SetDefaults( (u64)( ((u64)(nEntityTypeBits)) | ((u64)(nOurEntityClassBit)) ), ((u64)(nEntityLeafTypeBit)) ? ((u64)(nEntityLeafTypeBit)) : ((u64)(nOurEntityClassBit)), pszEntityType )


// Put this macro in the "protected definitions" section at the top of every class that derives from CEntity:
#define ENTITY_CLASS_HIERARCHY_BITDEF \
	typedef enum { ENTITY_CLASS_HIERARCHY_BIT = (ENTITY_CLASS_HIERARCHY_BIT << 1) }; \
	FINLINE u32 ClassHierarchyBit( void ) const { return ENTITY_CLASS_HIERARCHY_BIT; } \
	FINLINE BOOL IsOurWorkBitSet( void ) const { return GetClassHierarchyWorkMask() & ClassHierarchyBit(); } \
	FINLINE void EnableOurWorkBit( void ) { EnableClassHierarchyWorkBit( ClassHierarchyBit() ); } \
	FINLINE void DisableOurWorkBit( void ) { DisableClassHierarchyWorkBit( ClassHierarchyBit() ); }




const u64 ENTITY_BIT_BOT				= 0x0000000000000001;	// CBot
const u64 ENTITY_BIT_WEAPON				= 0x0000000000000002;	// CWeapon
const u64 ENTITY_BIT_MESHENTITY			= 0x0000000000000004;	// CMeshEntity
const u64 ENTITY_BIT_BOTGLITCH			= 0x0000000000000008;	// CBotGlitch
const u64 ENTITY_BIT_WEAPONHAND			= 0x0000000000000010;	// CWeaponHand
const u64 ENTITY_BIT_WEAPONLASER		= 0x0000000000000020;	// CWeaponLaser
const u64 ENTITY_BIT_WEAPONRIVET		= 0x0000000000000040;	// CWeaponRivet
const u64 ENTITY_BIT_POINT				= 0x0000000000000080;	// CEPoint
const u64 ENTITY_BIT_DOOR				= 0x0000000000000100;	// CEDoor
const u64 ENTITY_BIT_SWITCH				= 0x0000000000000200;	// CESwitch
const u64 ENTITY_BIT_BOOMER				= 0x0000000000000400;	// CEBoomer
const u64 ENTITY_BIT_SPLINE				= 0x0000000000000800;	// CESpline
const u64 ENTITY_BIT_SPHERE				= 0x0000000000001000;	// CESphere
const u64 ENTITY_BIT_LINE				= 0x0000000000002000;	// CELine
const u64 ENTITY_BIT_BOX				= 0x0000000000004000;	// CEBox
const u64 ENTITY_BIT_BOTPRED			= 0x0000000000008000;	// CBotPred
const u64 ENTITY_BIT_WEAPONROCKET		= 0x0000000000010000;	// CWeaponRocket
const u64 ENTITY_BIT_WEAPONBLASTER		= 0x0000000000020000;	// CWeaponBlaster
const u64 ENTITY_BIT_WEAPONGREN			= 0x0000000000040000;	// CWeaponGren
const u64 ENTITY_BIT_GOODIE				= 0x0000000000080000;	// CEGoodie
const u64 ENTITY_BIT_PROJECTILE			= 0x0000000000100000;	// CEProj
const u64 ENTITY_BIT_BOTGRUNT			= 0x0000000000200000;	// CBotGrunt
const u64 ENTITY_BIT_WEAPONTETHER		= 0x0000000000400000;	// CWeaponTether
const u64 ENTITY_BIT_CONSOLE			= 0x0000000000800000;	// CEConsole
const u64 ENTITY_BIT_WEAPONSPEW			= 0x0000000001000000;	// CWeaponSpew
const u64 ENTITY_BIT_WEAPONMORTAR		= 0x0000000002000000;	// CWeaponMortar
const u64 ENTITY_BIT_WEAPONFLAMER		= 0x0000000004000000;	// CWeaponFlamer
const u64 ENTITY_BIT_WEAPONRIPPER		= 0x0000000008000000;	// CWeaponRipper
const u64 ENTITY_BIT_WEAPONCLEANER		= 0x0000000010000000;	// CWeaponCleaner
const u64 ENTITY_BIT_PARTICLE			= 0x0000000020000000;	// CEParticle
const u64 ENTITY_BIT_ZIPLINE			= 0x0000000040000000;	// CEZipLine
const u64 ENTITY_BIT_WEAPONSCOPE		= 0x0000000080000000;	// CWeaponScope
const u64 ENTITY_BIT_BOTTITAN			= 0x0000000100000000;	// CBotTitan
const u64 ENTITY_BIT_BOTAAGUN			= 0x0000000200000000;	// CBotAAGun
const u64 ENTITY_BIT_SITEWEAPON			= 0x0000000400000000;	// CBotSiteWeapon
const u64 ENTITY_BIT_VEHICLE			= 0x0000000800000000;	// CVehicle
const u64 ENTITY_BIT_VEHICLERAT			= 0x0000001000000000;	// CVehicleRat
const u64 ENTITY_BIT_SHIELD				= 0x0000002000000000;	// CEShield
const u64 ENTITY_BIT_WEAPONEMP			= 0x0000004000000000;   // CWeaponEMP
const u64 ENTITY_BIT_BOTCHEMBOT			= 0x0000008000000000;   // CBotSlosh
const u64 ENTITY_BIT_MAGMABOMB			= 0x0000010000000000;   // CWeaponMagmaBomb
const u64 ENTITY_BIT_BOTPROBE			= 0x0000020000000000;   // CBotProbe
const u64 ENTITY_BIT_VEHICLESENTINEL	= 0x0000040000000000;	// CVehicleSentinel
//const u64 ENTITY_BIT_WEAPONQUADLASER	= 0x0000080000000000;	// CWeaponQuadLaser
const u64 ENTITY_BIT_BOTSNARQ			= 0x0000080000000000;	// CBotSnarq
const u64 ENTITY_BIT_LIQUIDVOLUME	    = 0x0000100000000000;	// CELiquidVolume
const u64 ENTITY_BIT_LIQUIDMESH		    = 0x0000200000000000;	// CELiquidVolume
const u64 ENTITY_BIT_VEHICLELOADER		= 0x0000400000000000;	// CVehicleLoader
const u64 ENTITY_BIT_BOTELITEGUARD		= 0x0000800000000000;	// CBotEliteGuard
const u64 ENTITY_BIT_BOTSWARMER			= 0x0001000000000000;	// CBotSwarmer
const u64 ENTITY_BIT_BOTZOMBIEBOSS		= 0x0002000000000000;	// CBotZombieBoss
const u64 ENTITY_BIT_KRUNK				= 0x0004000000000000;	// CBotKrunk
const u64 ENTITY_BIT_BOTZOM				= 0x0008000000000000;	// CBotZom
const u64 ENTITY_BIT_BOTMORTAR			= 0x0010000000000000;	// CBotMortar 
const u64 ENTITY_BIT_DETPACKDROP		= 0x0020000000000000;	// CEDetPackDrop
const u64 ENTITY_BIT_BOTJUMPER			= 0x0040000000000000;	// CBotJumper
const u64 ENTITY_BIT_BOTMOZER			= 0x0080000000000000;	// CBotMozer
const u64 ENTITY_BIT_BOTSCOUT			= 0x0100000000000000;	// CBotScout
const u64 ENTITY_BIT_BOTMINER			= 0x0200000000000000;	// CBotMiner
const u64 ENTITY_BIT_BOTSCIENTIST		= 0x0400000000000000;	// CBotScientist
const u64 ENTITY_BIT_JUMPPAD			= 0x0800000000000000;	// CEJumpPad
const u64 ENTITY_BIT_BOTCORROSIVE		= 0x1000000000000000;	// CBotCorrosive
const u64 ENTITY_BIT_DEBRIS				= 0x2000000000000000;	// CEDebris
const u64 ENTITY_BIT_UNSPECIFIED		= 0x4000000000000000;	// Unspecified entity
// The entity bit namespace is full. Sniper bots share this generic leaf bit;
// ENTITY_BIT_BOT remains present so bot-only filters still classify them as bots.
const u64 ENTITY_BIT_BOTSNIPER			= ENTITY_BIT_UNSPECIFIED;	// CBotSniper


const u64 ENTITY_BIT_RESERVED_FOR_FANG	= 0x8000000000000000;	// Fang reserves this bit for its internal tracker filter
const u64 ENTITY_BIT_NONE				= 0x0000000000000000;
const u64 ENTITY_BITS_ALLBOTBITS		= (	ENTITY_BIT_BOT		  |
											ENTITY_BIT_SITEWEAPON |
											ENTITY_BIT_VEHICLE	  | 
											ENTITY_BIT_VEHICLERAT | 
											ENTITY_BIT_VEHICLESENTINEL | 
											ENTITY_BIT_VEHICLELOADER|
											ENTITY_BIT_KRUNK        |
											ENTITY_BIT_BOTGLITCH	| 
											ENTITY_BIT_BOTPRED	| 
											ENTITY_BIT_BOTGRUNT	|
											ENTITY_BIT_BOTTITAN	|
											ENTITY_BIT_BOTPROBE	   | 
											ENTITY_BIT_BOTSWARMER   |
											ENTITY_BIT_BOTCHEMBOT   |
											ENTITY_BIT_BOTELITEGUARD   |
											ENTITY_BIT_BOTZOM       |
											ENTITY_BIT_BOTJUMPER    |
											ENTITY_BIT_BOTMINER		|
											ENTITY_BIT_BOTMORTAR    |
											ENTITY_BIT_BOTMOZER |
											ENTITY_BIT_BOTSCOUT |
											ENTITY_BIT_BOTSCIENTIST |
											ENTITY_BIT_BOTCORROSIVE | 
											ENTITY_BIT_BOTZOMBIEBOSS | 
											ENTITY_BIT_BOTAAGUN | 
											ENTITY_BIT_BOTSNARQ);

#define ENTITY_TYPE_ENTITY			"Entity"				// Maps to CEntity
#define ENTITY_TYPE_BOT				"Bot"					// Maps to CBot
#define ENTITY_TYPE_WEAPON			"Weapon"				// Maps to CWeapon
#define ENTITY_TYPE_MESH			"Mesh"					// Maps to CMeshEntity
#define ENTITY_TYPE_BOTGLITCH		"BotGlitch"				// Maps to CBotGlitch
#define ENTITY_TYPE_BOTSNIPER		"BotSniper"				// Maps to CBotSniper
#define ENTITY_TYPE_WEAPONHAND		"WeaponHand"			// Maps to CWeaponHand
#define ENTITY_TYPE_WEAPONLASER		"WeaponLaser"			// Maps to CWeaponLaser
#define ENTITY_TYPE_WEAPONRIVET		"WeaponRivet"			// Maps to CWeaponRivet
#define ENTITY_TYPE_POINT			"Point"					// Maps to CEPoint
#define ENTITY_TYPE_DOOR			"Door"					// Maps to CDoorEntity
#define ENTITY_TYPE_LIFT			"Lift"					// Maps to CDoorEntity
#define ENTITY_TYPE_SWITCH			"Switch"				// Maps to CESwitch
#define ENTITY_TYPE_BOOMER			"Destruct"				// Maps to CEBoomer (Using "destruct" to be backward compatible with old CDestructEntity)
#define ENTITY_TYPE_SPLINE			"Spline"				// Maps to CESpline
#define ENTITY_TYPE_SPHERE			"Sphere"				// Maps to CESphere
#define ENTITY_TYPE_LINE			"Line"					// Maps to CELine
#define ENTITY_TYPE_BOX				"Box"					// Maps to CEBox
#define ENTITY_TYPE_BOTPRED			"BotPred"				// Maps to CBotPred
#define ENTITY_TYPE_WEAPONROCKET	"WeaponRocket"			// Maps to CWeaponRocket
#define ENTITY_TYPE_WEAPONBLASTER	"WeaponBlaster"			// Maps to CWeaponBlaster
#define ENTITY_TYPE_WEAPONGREN		"WeaponGren"			// Maps to CWeaponGren
#define ENTITY_TYPE_GOODIE			"Goodie"				// Maps to CEGoodie
#define ENTITY_TYPE_PROJECTILE		"Projectile"			// Maps to CEProj
#define ENTITY_TYPE_BOTGRUNT		"BotGrunt"				// Maps to CBotGrunt
#define ENTITY_TYPE_WEAPONTETHER	"WeaponTether"			// Maps to CWeaponTether
#define ENTITY_TYPE_CONSOLE			"Console"				// Maps to CEConsole
#define ENTITY_TYPE_WEAPONSPEW		"WeaponSpew"			// Maps to CWeaponSpew
#define ENTITY_TYPE_WEAPONMORTAR	"WeaponMortar"			// Maps to CWeaponMortar
#define ENTITY_TYPE_WEAPONFLAMER	"WeaponFlamer"			// Maps to CWeaponFlamer
#define ENTITY_TYPE_WEAPONRIPPER	"WeaponRipper"			// Maps to CWeaponRipper
#define ENTITY_TYPE_WEAPONCLEANER	"WeaponCleaner"			// Maps to CWeaponCleaner
#define ENTITY_TYPE_PARTICLE		"Particle"				// Maps to CEParticle
#define ENTITY_TYPE_ZIPLINE			"Zipline"				// Maps to CEZipLine
#define ENTITY_TYPE_WEAPONSCOPE		"WeaponScope"			// Maps to CWeaponScope
#define ENTITY_TYPE_BOTTITAN		"BotTitan"				// Maps to CBotTitan
#define ENTITY_TYPE_WEAPONCHAINGUN	"WeaponChaingun"		// Maps to CWeaponChaingun
#define ENTITY_TYPE_BOTSITEWEAPON	"SiteWeapon"			// Maps to CBotSentry
#define ENTITY_TYPE_VEHICLE			"Vehicle"				// Maps to CVehicle
#define ENTITY_TYPE_VEHICLERAT		"VehicleRat"			// Maps to CVehicleRat
#define ENTITY_TYPE_VEHICLESENTINEL	"VehicleTank"			// Maps to CVehicleSentinel
#define ENTITY_TYPE_SHIELD			"Shield"				// Maps to CEShield
#define ENTITY_TYPE_WEAPONEMP		"WeaponEMP"				// Maps to CWeaponEMP
#define ENTITY_TYPE_BOTCHEMBOT		"BotSlosh"				// Maps to CBotSlosh
#define ENTITY_TYPE_BOTPROBE		"BotProbe"				// Maps to CBotProbe
#define ENTITY_TYPE_MAGMABOMB		"MagmaBomb"				// Maps to CWeaponMagmaBomb
#define ENTITY_TYPE_WEAPONQUADLASER	"WeaponQuadLaser"		// Maps to CWeaponQuadLaser
#define ENTITY_TYPE_LIQUIDVOLUME	"LiquidVolume"			// Maps to CELiquidVolume
#define ENTITY_TYPE_LIQUIDMESH		"LiquidMesh"			// Maps to CELiquidMesh
#define ENTITY_TYPE_VEHICLELOADER	"VehicleLoader"			// Maps to CVehicleLoader
#define ENTITY_TYPE_BOTSWARMER		"BotSwarmer"			// Maps to CBotSwarmer
#define ENTITY_TYPE_BOTELITEGUARD	"BotEliteGuard"			// Maps to CBotEliteGuard
#define ENTITY_TYPE_WEAPONWRENCH	"WeaponWrench"			// Maps to CWeaponWrench
#define ENTITY_TYPE_BOTAAGUN		"BotAAGun"				// Maps to CBotAAGun
#define ENTITY_TYPE_KRUNK			"BotKrunk"				// Maps to CBotKrunk
#define ENTITY_TYPE_BOTZOM			"BotZombie"				// Maps to CBotZom
#define ENTITY_TYPE_DETPACKDROP		"DetPackDrop"			// Maps to CEDetPackDrop
#define ENTITY_TYPE_BOTJUMPER		"BotJumper"				// Maps to CBotJumper
#define ENTITY_TYPE_BOTMINER		"BotBlink"				// Maps to CBotMiner
#define ENTITY_TYPE_BOTMORTAR		"BotMortar"				// Maps to CBotMortar
#define ENTITY_TYPE_BOTMOZER		"BotMozer"				// Maps to CBotMozer
#define ENTITY_TYPE_BOTSCOUT		"BotScout"				// Maps to CBotScout
#define ENTITY_TYPE_JUMPPAD			"JumpPad"				// Maps to CEJumpPad
#define ENTITY_TYPE_BOTCORROSIVE	"BotCorrosive"			// Maps to CBotCorrosive
#define ENTITY_TYPE_BOTZOMBIEBOSS	"BotZombieBoss"			// Maps to CBotZombieBoss
#define ENTITY_TYPE_BOTSCIENTIST	"BotScientist"			// Maps to CBotScientist
#define ENTITY_TYPE_RECRUITER		"Recruiter"				// Maps to CWeaponRecruiter
#define ENTITY_TYPE_BOTSNARQ		"BotSnarq"				// Maps to CBotSnarq
#define ENTITY_TYPE_DEBRIS			"Debris"				// Maps to CEDebris

// Forward declarations.
class CEntity;
class CAIBrain;
class CEntityBuilder;
class CAIBuilder;
class CTripwire;
class CGoodieProps;
class CDamageData;
class CArmorProfile;
class CFSoundGroup;




// Typedef for a function that will be given first chance when the action button is pressed.
typedef BOOL ActionFunction_t( CEntity *pActioningEntity, CEntity *pActionedEntity );




//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CEntity
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

FCLASS_ALIGN_PREFIX class CEntity {
//----------------------------------------------------------------------------------------------------------------------------------
// Public Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	enum {
		ENTITY_FLAG_CREATED						= 0x00000001,	// This CEntity has been created
		ENTITY_FLAG_INWORLD						= 0x00000002,	// This CEntity is in the world
		ENTITY_FLAG_ENABLE_WORK					= 0x00000004,	// This CEntity's work function is allowed to be called
		ENTITY_FLAG_ENABLE_AUTOWORK				= 0x00000008,	// This CEntity's work function will be automatically called each loop
		ENTITY_FLAG_ALLOW_WORK_IF_PAUSED		= 0x00000010,	// Don't prevent work function from being called when game is paused
		ENTITY_FLAG_TARGETABLE					= 0x00000020,	// This CEntity is targetable
		ENTITY_FLAG_INVINCIBLE					= 0x00000040,	// This CEntity is invincible from all forms of damage (hit points, impulse, and rumble)
		ENTITY_FLAG_AUTODELETE					= 0x00000080,	// Entity will be fdeleted when world is destroyed
		ENTITY_FLAG_BUILDING					= 0x00000100,	// Entity is being built
		ENTITY_FLAG_ACTIONABLE					= 0x00000200,	// This CEntity would potentially be interesting in having its ActionNearby() called
		ENTITY_FLAG_BUILDPARENT_IS_INDEX		= 0x00000400,	// FALSE: m_pszParentEntityName is valid, TRUE: m_nPendingAttachShapeIndex is valid
		ENTITY_FLAG_TRIPPER						= 0x00000800,	// This CEntity can interact with tripwires
		ENTITY_FLAG_TRIPWIRE_ARMED				= 0x00001000,	// Tripwire functionality is enabled (if this CEntity is a tripwire)
		ENTITY_FLAG_COLLECTABLE					= 0x00002000,   // This item is a collectable (ENTITY_FLAG_TRIPWIRE_ARMED must be set also)
		ENTITY_FLAG_INHERIT_PARENT_SCALE		= 0x00004000,	// This entity is to inherit its parent's scale
		ENTITY_FLAG_PARENTMTXTOWORLD_POINTS_TO_MTXTOWORLD	= 0x00008000,	// m_pParentMtxToWorld points to the parent's m_MtxToWorld (as opposed to the bone palette, etc.)
		ENTITY_FLAG_SAVE_STATE					= 0x00010000,	// this entity saves state information
		ENTITY_FLAG_MARKED_FOR_WORLD_REMOVE		= 0x00020000,	// This entity will be removed at the end of this frame
		ENTITY_FLAG_DISABLE_DRAWING				= 0x00040000,	// This entity should not be drawn
		ENTITY_FLAG_RESTORE_ATTACH				= 0x00080000,	// Re-attach this entity to its parent when restoring a checkpoint
		ENTITY_FLAG_INFINITE_ARMOR				= 0x00100000,	// This entity's armor profile provides infinite protection from all types of damage (hit points, impulse, and rumble)
		ENTITY_FLAG_NOT_REMOVED_ON_DETACH		= 0x00200000,	// This entity remains in world after detached from parent
		ENTITY_FLAG_DISABLE_CAMERA_COLLISION	= 0x00400000,	// This entity will not obstruct the camera 
		ENTITY_FLAG_ALARMSYS_USE				= 0x00800000,	// This entity is in some way associated with the alarm system. So, Alarm system should get notification of its destruction
		ENTITY_FLAG_SPAWNSYS_USE				= 0x01000000,	// This entity is in some way associated with the spawn system. So, CSpawnSys should get notification of major events related to it.
		ENTITY_FLAG_USE_SPHERE_FOR_BLAST_DAMAGE	= 0x02000000,	// When issuing blast damage to this entity, use the tracker sphere with no blast protection checks
		ENTITY_FLAG_RESTART_AMBIENT_SOUND_ON_RESTORE = 0x04000000, // Used only for Checkpoint saves and restore, this tells the restore code to restart the ambient sound if the sound was playing at checkpoint save time
		ENTITY_FLAG_NOT_WALKABLE					 = 0x08000000, 
		ENTITY_FLAG_NONE						= 0x00000000
	};


	enum { MAX_INTERSECTING_TRIPWIRES = 4 };


	typedef enum {
		PROJECTILE_REACTION_DEFAULT,						// It's up to the projectile that hits this entity to determine how to behave
		PROJECTILE_REACTION_RICOCHET,						// Projectiles that hit this entity should ricochet (glass)
		PROJECTILE_REACTION_DEATH_TO_ALL,					// Projectiles that hit this entity should detonate (hot surface)
		PROJECTILE_REACTION_STICKY,							// Saw blades should stick immediately (catwalks)
		PROJECTILE_REACTION_HURT_ME,						// This is an object that projectiles were made for (bots)
		PROJECTILE_REACTION_SHIELD,							// Grenades will detonate, arrows and saws will ricochet (shield)

		PROJECTILE_REACTION_COUNT
	} ProjectileReaction_e;


	// For events triggered by a tripwire:
	//   Parameter 1 is this enum
	//   Parameter 2 is a pointer to the tripwire's CEntity (never NULL)
	//   Parameter 3 is a pointer to the tripper's CEntity (never NULL)
	typedef enum {
		TRIPWIRE_EVENTTYPE_ENTER = 0,						// Tripwire "enter" event
		TRIPWIRE_EVENTTYPE_EXIT,							// Tripwire "exit" event
		TRIPWIRE_EVENTTYPE_INSIDE,							// Tripwire "inside" event

		TRIPWIRE_EVENTTYPE_COUNT
	} EventCode_e;


	enum {
		TRIPWIRE_EVENTFLAG_ENTER_EVENT		= 0x00000001,	// Tripwire "enter" events are enabled
		TRIPWIRE_EVENTFLAG_EXIT_EVENT		= 0x00000002,	// Tripwire "exit" events are enabled
		TRIPWIRE_EVENTFLAG_INSIDE_EVENT		= 0x00000004,	// Tripwire "inside" events are enabled

		TRIPWIRE_EVENTFLAG_ALL				= (TRIPWIRE_EVENTFLAG_ENTER_EVENT | TRIPWIRE_EVENTFLAG_EXIT_EVENT | TRIPWIRE_EVENTFLAG_INSIDE_EVENT),
		TRIPWIRE_EVENTFLAG_NONE				= 0x00000000
	};


	enum {
		TRIPWIRE_FILTER_INCLUDE_PLAYER		= 0x00000001,	// Tripwire can be tripped by any entity that the player is in control of
		TRIPWIRE_FILTER_INCLUDE_ENEMY		= 0x00000002,	// Tripwire can be tripped by any enemy entity (aside from other players)
		TRIPWIRE_FILTER_INCLUDE_FRIENDLY	= 0x00000004,	// Tripwire can be tripped by any friendly entity (aside from other players)
		TRIPWIRE_FILTER_INCLUDE_OTHER		= 0x00000008,	// Tripwire can be tripped by any tripper not included in TRIPWIRE_FILTER_INCLUDE_PLAYER and TRIPWIRE_FILTER_INCLUDE_ENEMY

		TRIPWIRE_FILTER_INCLUDE_ALL			= (TRIPWIRE_FILTER_INCLUDE_PLAYER | TRIPWIRE_FILTER_INCLUDE_ENEMY | TRIPWIRE_FILTER_INCLUDE_FRIENDLY | TRIPWIRE_FILTER_INCLUDE_OTHER),
		TRIPWIRE_FILTER_INCLUDE_NONE		= 0x00000000
	};


	enum {
		TRIPWIRE_TRIGGER_MODE_MULTIPLE = 0,					// Tripwire triggers normally (default)
		TRIPWIRE_TRIGGER_MODE_ONCE,							// Tripwire triggers once, then removes itself from world
	};




//----------------------------------------------------------------------------------------------------------------------------------
// Protected Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
protected:

	typedef enum {
		ENTITY_CLASS_HIERARCHY_BIT = 1
	};


	enum {													// Note that it's possible to have any number of these bits set:
		TRIPWIRE_COLLFLAG_ENTER_EVENT			= 0x00000001,	// The point passed from the tripwire's exterior to its interior
		TRIPWIRE_COLLFLAG_EXIT_EVENT			= 0x00000002,	// The point passed from the tripwire's interior to its exterior
		TRIPWIRE_COLLFLAG_NEWPOS_INSIDE			= 0x00000004,	// The new position is inside the tripwire
		TRIPWIRE_COLLFLAG_KILL_NOW				= 0x00000008,	// This tripwire is a KillMode tripwire, and GAME OVER for this guy
		TRIPWIRE_COLLFLAG_SPAWN_DEATH_EFFECTS	= 0x00000010,	// (Used in conjunction junction with TRIPWIRE_COLLFLAG_KILL_NOW) -- This flag specifies that the entity should spawn its death effects.  

		TRIPWIRE_COLLFLAG_NONE					= 0x00000000
	};




//----------------------------------------------------------------------------------------------------------------------------------
// Public Data:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	CFMtx43A *m_pParentMtxToWorld;					// Pointer to parent's matrix that places it in world space (NULL if no parent) (may have uniform scale)
	CGoodieBag *m_pGoodieBag;						// Pointer to this entity's goodie bag

	static FMesh_t *m_pErrorMesh;					// Error mesh (the infamous question mark)  NOTE: This could be NULL !!!
	static cchar *m_apszEntityType[];




//----------------------------------------------------------------------------------------------------------------------------------
// Protected Data:
//----------------------------------------------------------------------------------------------------------------------------------
protected:

	cchar *m_pszAttachBone;							// Bone name of parent we're attached to (NULL=none)
	CTripwire *m_pTripwire;							// Pointer to tripwire data (NULL=we're not a tripwire)
	CFMtx43A m_MtxToWorld;							// Matrix that places us in world space (always unit in scale)
	CFMtx43A m_MtxToParent;							// Matrix that places us in parent space (or world space if m_pParentEntity is NULL) (always unit in scale)
	CFMtx43A m_PreviousParentMtxToWorld;			// Copy of *m_pParentMtxToWorld from when RelocateAllChildren was last called
	CFVerlet *m_pVerlet;							// Pointer to Verlet object (NULL=none)

	f32 m_fScaleToWorld;							// The scale of this entity in world space
	f32 m_fScaleToParent;							// The scale of this entity relative to its parent's scale (or world space if m_pParentEntity is NULL)

	union {											// (ENTITY_FLAG_BUILDPARENT_IS_INDEX indicates which field in this union is valid):
		cchar *m_pszParentEntityName;				// When the entity is being built, this points to the name of the entity's parent (NULL=none)
		s32 m_nPendingAttachShapeIndex;				// During initialization from world data, this is set to the index into the world shape array
													//  of the entity that is this entity's parent (-1=no parent)
	};

	CGoodieProps *m_pGoodieProps;					// If this entity can be used as a goodie distributor, this will point to some properties for 
													//   such distribution.

	s32 m_hSaveData[FCHECKPOINT_MAX_CHECKPOINTS];	// handle to "save point" data storage




//----------------------------------------------------------------------------------------------------------------------------------
// Private Data:
//----------------------------------------------------------------------------------------------------------------------------------
private:

	static BOOL m_bSystemInitialized;				// TRUE: The CEntity system has been initialized
	static u32 m_nWirePoolCount;					// Number of wires in the wire pool

	static CNoControl m_NoControl;					// No input controls

	static FLinkRoot_t m_InWorldList_Root;			// Linklist of all entities currently in the world
	static FLinkRoot_t m_OutOfWorldList_Root;		// Linklist of all entities currently out of the world
	static FLinkRoot_t m_AutoWorkList_Root;			// Linklist of all entities currently in the world with their auto-work flag set
	static FLinkRoot_t m_MarkedList_Root;			// Linklist of all entities currently marked for world removal
	static FLinkRoot_t m_CheckpointSaveList_Root[FCHECKPOINT_MAX_CHECKPOINTS];	// Linklist of all entities currently being saved

	CEntity *m_pParentEntity;						// Pointer to parent CEntity
	FLinkRoot_t m_ChildEntityRoot;					// Link list of CEntity children
	FLink_t m_SiblingLink;							// Link to sibling CEntity objects

	FLink_t m_EntityLink;							// Link to other CEntity objects in either Enabled or Disabled link lists
	FLink_t m_AutoWorkLink;							// Link to other in-world CEntity objects that have their auto-work flag set
	FLink_t m_MarkedLink;							// Link to other Centity objects that have been marked for world removal at the end of the given frame
	FLink_t m_CheckpointSaveLink[FCHECKPOINT_MAX_CHECKPOINTS];	// Link to other CEntity objects being saved.

	cchar *m_pszName;								// ASCIIZ name of this entity (NULL=no name)
	u64 m_nEntityTypeBits;							// Describes derivation hierarchy of instantiated class. See ENTITY_BIT_* for info.
	u64 m_nEntityLeafTypeBit;						// One bit set for the instantiated (leaf) class. See ENTITY_BIT_* for info.
	u32 m_nEntityFlags;								// See CEntity::ENTITY_FLAG_* for info
	u32 m_nEnableWorkMask;							// Class hierarchy bits set for each class in the class hierarchy that needs its ClassHierarchyWork() called
	u32 m_nGuid;									// This entity's GUID (0=none)

	CEntityControl *m_pControl;						// Controller input data (never NULL)
	CAIBrain *m_pAIBrain;							// Pointer to AI interface (NULL=none)
	ActionFunction_t *m_pFcnAction;					// Pointer to action handler

	f32 m_fHealthContainerCount;					// Number of health containers (must be an integer and a minimum of 1)
	f32 m_fOOHealthContainerCount;					// 1/m_fHealthContainerCount
	f32 m_fNormHealth;								// 0.0=dead, 1.0=one health container filled, 2.0=two health containers filled, etc.
	f32 m_fArmorModifier;							// -1=fully attenuate profile armor, 0=don't modify profile armor, +1=fully boost profile armor
	CArmorProfile *m_pArmorProfile;					// Armor profile
	CDamageProfile *m_pDamageProfile;				// Damage profile (used by script system to generate damage from this entity)

	u32 m_nIntersectingTripwireCount;								// Number of elements in m_apIntersectingTripwires
	CEntity *m_apIntersectingTripwires[MAX_INTERSECTING_TRIPWIRES];	// Array of tripwires that we're currently intersecting

	FSndFx_FxHandle_t m_hAmbientSFX;				// handle to entity's ambient sound effect
	CFAudioEmitter *m_pAmbientEmitter;				// pointer to audio emitter for ambient sound
	f32 m_fAmbientRadius;							// Ambient sound radius
	f32 m_fAmbientVolume;							// Ambient sound volume

	u8 m_nProjectileReaction;						// See ProjectileReaction_e for info

	u8 m_abCheckpointSave[FCHECKPOINT_MAX_CHECKPOINTS];	// TRUE if this entity is saving its state, indexed by checkpoint




//----------------------------------------------------------------------------------------------------------------------------------
// Public Functions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	// System:
	static BOOL InitSystem( void );
	static void UninitSystem( void );
	static void CallAllAutoWorks( void );
	static void CallAllMarkedRemoveFromWorld( void );
	static void ResolveEntityPointerFixups( void );		//   After all entities have been created, that function gets called so entities that are to be attached to other entities can be done.
	static void SetWirePoolCount( u32 nWirePoolCount = ENTITY_DEFAULT_WIRE_POOL_COUNT ) { m_nWirePoolCount = nWirePoolCount; }

	static void CallActiveListCallbacks( void );	// queries all in-world entities and calls callbacks if the entity's mesh just entered or left the active list
	static void RemoveAndDestroyAll( void );		// will remove and destroy all entities

	// Find:
	static CEntity *FindInWorld( cchar *pszName );
	static CEntity *FindInWorld_Next( const CEntity *pCur, cchar *pszName );
	static CEntity *FindOutOfWorld( cchar *pszName );
	static CEntity *Find( cchar *pszName );

	static CEntity *FindInWorld( u32 nGUID );
	static CEntity *FindOutOfWorld( u32 nGUID );
	static CEntity *Find( u32 nGUID );


	// Creation:
	CEntity();
	BOOL Create( cchar *pszEntityName=NULL, const CFMtx43A *pMtx=NULL, cchar *pszAIBuilderName=NULL, u32 nTripwireEntityCount=0 );
	BOOL BuildFromWorldShape( void );
	BOOL Build( void );


	// Destruction:
	virtual ~CEntity();
	void Destroy( void );


	// Information:
	static FINLINE BOOL IsSystemInitialized( void ) { return m_bSystemInitialized; }
	FINLINE BOOL IsCreated( void ) const { return (BOOL)(m_nEntityFlags & ENTITY_FLAG_CREATED); }
	FINLINE cchar *Name( void ) const { return m_pszName; }
	FINLINE u64 TypeBits( void ) const { return m_nEntityTypeBits; }
	FINLINE u64 LeafTypeBit( void ) const { return m_nEntityLeafTypeBit; }
	u64 TypeBitsRecurseParents( void ) const;		 //bitwise or the type bits of this entity and all its parents


	// Goodie Distribution functions.
	//   apparently these are used if goodies are ever emitted from this object. (some derived classes override them)
	virtual void GetGoodieDistributionOrigin(CFVec3A *pPt_WS);
	virtual void GetGoodieDistributionDir(CFVec3A *pVec_WS);


	// World:
	FINLINE BOOL IsInWorld( void ) const { return (BOOL)(m_nEntityFlags & ENTITY_FLAG_INWORLD); }
	void AddToWorld( void );
	void RemoveFromWorld( BOOL bDoItNow=FALSE );
	void DrawEnable( BOOL bEnableDrawing, BOOL bForce = FALSE );
	FINLINE BOOL IsDrawEnabled( void ) const { FASSERT( IsCreated() ); return !(m_nEntityFlags & ENTITY_FLAG_DISABLE_DRAWING); }

	FINLINE BOOL IsMarkedForWorldRemove( void ) const { return (BOOL)(m_nEntityFlags & ENTITY_FLAG_MARKED_FOR_WORLD_REMOVE); }

	void Relocate_Scale_WS( f32 fNewEntityScale_WS, BOOL bEnableTripping=TRUE, void *pIdentifier=NULL );
	void Relocate_Xlat_WS( const CFVec3A *pNewXlat_WS, BOOL bEnableTripping=TRUE, void *pIdentifier=NULL );
	void Relocate_RotXlatFromUnitMtx_WS( const CFMtx43A *pUnitMtx_WS, BOOL bEnableTripping=TRUE, void *pIdentifier=NULL );
	void Relocate_RotXlatFromUnitMtx_WS_NewScale_WS( const CFMtx43A *pUnitMtx_WS, f32 fNewEntityScale_WS, BOOL bEnableTripping=TRUE, void *pIdentifier=NULL );
	void Relocate_RotXlatScaleFromScaledMtx_WS( const CFMtx43A *pMtx_WS, f32 fScaleOfMtx, BOOL bEnableTripping=TRUE, void *pIdentifier=NULL );
	void Relocate_RotXlatFromScaledMtx_WS_NewScale_WS( const CFMtx43A *pMtx_WS, f32 fScaleOfMtx, f32 fNewEntityScale_WS, BOOL bEnableTripping=TRUE, void *pIdentifier=NULL );

	void Relocate_Scale_PS( f32 fNewEntityScale_PS, BOOL bEnableTripping=TRUE, void *pIdentifier=NULL );
	void Relocate_Xlat_PS( const CFVec3A *pNewXlat_PS, BOOL bEnableTripping=TRUE, void *pIdentifier=NULL );
	void Relocate_RotXlatFromUnitMtx_PS( const CFMtx43A *pUnitMtx_PS, BOOL bEnableTripping=TRUE, void *pIdentifier=NULL );
	void Relocate_RotXlatFromUnitMtx_PS_NewScale_PS( const CFMtx43A *pUnitMtx_PS, f32 fNewEntityScale_PS, BOOL bEnableTripping=TRUE, void *pIdentifier=NULL );
	void Relocate_RotXlatFromUnitMtx_PS_NewScale_WS( const CFMtx43A *pUnitMtx_PS, f32 fNewEntityScale_WS, BOOL bEnableTripping=TRUE, void *pIdentifier=NULL );
	void Relocate_RotXlatFromScaledMtx_PS_ScaleFromScaledMtx_PS( const CFMtx43A *pMtx_PS, f32 fScaleOfMtx, BOOL bEnableTripping=TRUE, void *pIdentifier=NULL );
	void Relocate_RotXlatFromScaledMtx_PS_ScaleFromScaledMtx_WS( const CFMtx43A *pMtx_PS, f32 fScaleOfMtx, BOOL bEnableTripping=TRUE, void *pIdentifier=NULL );
	void Relocate_RotXlatFromScaledMtx_PS_NewScale_PS( const CFMtx43A *pMtx_WS, f32 fScaleOfMtx, f32 fNewEntityScale_PS, BOOL bEnableTripping=TRUE, void *pIdentifier=NULL );
	void Relocate_RotXlatFromScaledMtx_PS_NewScale_WS( const CFMtx43A *pMtx_WS, f32 fScaleOfMtx, f32 fNewEntityScale_WS, BOOL bEnableTripping=TRUE, void *pIdentifier=NULL );

	FINLINE CFMtx43A *MtxToWorld( void ) const { return (CFMtx43A *)&m_MtxToWorld; }
	FINLINE CFMtx43A *MtxToParent( void ) const { return (CFMtx43A *)&m_MtxToParent; }
	FINLINE f32 ScaleToWorld( void ) const { return m_fScaleToWorld; }
	FINLINE f32 ScaleToParent( void ) const { return m_fScaleToParent; }


	// Flags:
	FINLINE BOOL IsTargetable( void ) const { return (BOOL)(m_nEntityFlags & ENTITY_FLAG_TARGETABLE); }
	void SetTargetable( BOOL bEnable );

	FINLINE BOOL IsInvincible( void ) const { return (BOOL)(m_nEntityFlags & (ENTITY_FLAG_INVINCIBLE | ENTITY_FLAG_INFINITE_ARMOR)); }
	void SetInvincible( BOOL bEnable );

	FINLINE BOOL IsActionable( void ) const { return (BOOL)(m_nEntityFlags & ENTITY_FLAG_ACTIONABLE); }
	void SetActionable( BOOL bEnable );

	FINLINE BOOL RemovedOnParentsDetach( void ) const { return (BOOL)(!(m_nEntityFlags & ENTITY_FLAG_NOT_REMOVED_ON_DETACH)); }
	void SetRemovedOnParentsDetach( BOOL bEnable );

	FINLINE BOOL HasAlarmSysUse( void ) const { return !!(m_nEntityFlags & ENTITY_FLAG_ALARMSYS_USE);}
	FINLINE void SetAlarmSysUse( BOOL bEnable ) { m_nEntityFlags &= ~ENTITY_FLAG_ALARMSYS_USE; m_nEntityFlags |= ENTITY_FLAG_ALARMSYS_USE*bEnable; }

	FINLINE BOOL HasSpawnSysUse( void ) const { return !!(m_nEntityFlags & ENTITY_FLAG_SPAWNSYS_USE);}
	FINLINE void SetSpawnSysUse( BOOL bEnable ) { m_nEntityFlags &= ~ENTITY_FLAG_SPAWNSYS_USE; m_nEntityFlags |= ENTITY_FLAG_SPAWNSYS_USE*bEnable; }

	FINLINE BOOL IsAutoDelete( void )			{ return m_nEntityFlags & ENTITY_FLAG_AUTODELETE; }
	FINLINE void SetAutoDelete( BOOL bEnable )	{ m_nEntityFlags &= ~ENTITY_FLAG_AUTODELETE; m_nEntityFlags |= ENTITY_FLAG_AUTODELETE * bEnable; }

	FINLINE BOOL IsWalkable( void )    const	{ FASSERT( IsCreated() ); return !(m_nEntityFlags & ENTITY_FLAG_NOT_WALKABLE); }
	FINLINE void SetWalkable( BOOL bEnable )	{ (bEnable ? (m_nEntityFlags &= ~ENTITY_FLAG_NOT_WALKABLE) : (m_nEntityFlags |= ENTITY_FLAG_NOT_WALKABLE)); }

	// Tripwire:
	FINLINE BOOL IsTripwire( void ) const { return (BOOL)m_pTripwire; }
	FINLINE BOOL IsTripwireArmed( void ) const { return m_pTripwire ? m_nEntityFlags & ENTITY_FLAG_TRIPWIRE_ARMED : FALSE; }
	void ArmTripwire( BOOL bArm );
#if FANG_WINGC
	// Local co-op (PC): a tripwire's enter event from a player waits until every standing player has
	// arrived in it (entity.cpp). Work runs once a frame; Reset at level load; Waiting tells a
	// player's HUD they are holding an event for the others.
	static void CoopTripwireWork( void );
	static void CoopTripwireReset( void );
	static BOOL CoopTripwireWaiting( const CEntity *pPlayerEntity );
#endif

	FINLINE CFSphereA *TripwireBoundingSphere_WS( void ) const;
	void SetTripwireBoundingSphere_MS( const CFSphereA *pSphere_MS );

	FINLINE u32 TripwireEventFlags( void ) const;
	void EnableTripwireEvents( u32 nEventFlagsToEnable );
	void DisableTripwireEvents( u32 nEventFlagsToDisable );
	void SetTripwireEventMask( u32 nEventMask );

	FINLINE u32 TripwireFilterFlags( void ) const;
	void EnableTripwireFilterFlags( u32 nFilterFlagsToEnable );
	void DisableTripwireFilterFlags( u32 nFilterFlagsToDisable );
	void SetTripwireFilterMask( u32 nFilterFlags );

	FINLINE u32 GetContainedTripperCount( void ) const;
	FINLINE CEntity **GetContainedTripperArray( void ) const;

	FINLINE BOOL TripwireDamage_IsBuiltForDamage( void ) const;

	FINLINE BOOL TripwireDamage_IsEnabled( void ) const;
	FINLINE void TripwireDamage_Enable( BOOL bEnable );

	FINLINE f32 TripwireDamage_GetNormIntensity( void ) const;
	FINLINE void TripwireDamage_SetNormIntensity( f32 fNormIntensity );


	// Tripper:
	FINLINE BOOL IsTripper( void ) const { return (BOOL)(m_nEntityFlags & ENTITY_FLAG_TRIPPER); }
	void EnableTripper( BOOL bEnable );

	FINLINE u32 GetIntersectingTripwireCount( void ) const { return m_nIntersectingTripwireCount; }
	FINLINE CEntity **GetIntersectingTripwire( void ) const { return (CEntity **)m_apIntersectingTripwires; }


	// Physics:
	FINLINE CFVerlet *GetVerlet( void ) const { return m_pVerlet; }


	// GUID:
	FINLINE u32 Guid( void ) const { return m_nGuid; }
	FINLINE void AssignNewGuid( void ) { FASSERT( IsCreated() ); m_nGuid = GUID_NEW; }
	FINLINE void ResetGuid( void ) { FASSERT( IsCreated() ); m_nGuid = 0; }


	// Work:
	FINLINE BOOL IsWorkEnabled( void ) const { return (BOOL)(m_nEntityFlags & ENTITY_FLAG_ENABLE_WORK); }
	void EnableWork( BOOL bEnable );
	FINLINE u32 GetClassHierarchyWorkMask( void ) const { return m_nEnableWorkMask; }

	FINLINE BOOL IsAutoWorkEnabled( void ) const { return (BOOL)(m_nEntityFlags & ENTITY_FLAG_ENABLE_AUTOWORK); }
	void EnableAutoWork( BOOL bEnable );

	FINLINE BOOL IsWorkWhilePausedEnabled( void ) const { return (BOOL)(m_nEntityFlags & ENTITY_FLAG_ALLOW_WORK_IF_PAUSED); }
	FINLINE void EnableWorkWhilePaused( BOOL bEnable );

	void Work( void );


	// Parent/Child:
	void Attach_ToParent_WS( CEntity *pParentEntity, cchar *pszAttachBoneName=NULL, BOOL bInheritParentScale=FALSE );
	void Attach_ToParent_PS( CEntity *pParentEntity, cchar *pszAttachBoneName=NULL, BOOL bInheritParentScale=FALSE );

	void Attach_UnitMtxToParent_PS( CEntity *pParentEntity, cchar *pszAttachBoneName=NULL, const CFMtx43A *pUnitMtx_PS=&CFMtx43A::m_IdentityMtx, BOOL bInheritParentScale=FALSE  );
	void Attach_UnitMtxToParent_PS_NewScale_WS( CEntity *pParentEntity, cchar *pszAttachBoneName=NULL, const CFMtx43A *pUnitMtx_PS=&CFMtx43A::m_IdentityMtx, f32 fNewEntityScale_WS=1.0f, BOOL bInheritParentScale=FALSE  );
	void Attach_UnitMtxToParent_PS_NewScale_PS( CEntity *pParentEntity, cchar *pszAttachBoneName=NULL, const CFMtx43A *pUnitMtx_PS=&CFMtx43A::m_IdentityMtx, f32 fNewEntityScale_PS=1.0f, BOOL bInheritParentScale=FALSE  );

	virtual void DetachFromParent( void );
	void DetachAllChildren( void );
	void RelocateAllChildren( BOOL bEnableTripping=TRUE );

	CEntity *GetParentOfType( u64 uTypeBits ) ;	   //recurse up through link list of parents looking for one that has type matching any Bit in uTypeBits
	FINLINE CEntity *GetParent( void ) const { return m_pParentEntity; }
	FINLINE cchar *GetAttachBoneName( void ) const { return m_pszAttachBone; }

	FINLINE u32 GetChildCount( void ) const { FASSERT( IsSystemInitialized() ); return m_ChildEntityRoot.nCount; }
	FINLINE CEntity *GetFirstChild( void ) const { FASSERT( IsSystemInitialized() ); return (CEntity *)flinklist_GetHead( &m_ChildEntityRoot ); }
	FINLINE CEntity *GetLastChild( void ) const { FASSERT( IsSystemInitialized() ); return (CEntity *)flinklist_GetTail( &m_ChildEntityRoot ); }
	FINLINE CEntity *GetNextChild( const CEntity *pChildEntity ) const { FASSERT( IsSystemInitialized() ); return (CEntity *)flinklist_GetNext( &m_ChildEntityRoot, pChildEntity ); }
	FINLINE CEntity *GetPrevChild( const CEntity *pChildEntity ) const { FASSERT( IsSystemInitialized() ); return (CEntity *)flinklist_GetPrev( &m_ChildEntityRoot, pChildEntity ); }


	// Collision:
	virtual FINLINE void AppendTrackerSkipList(u32& FWorld_nTrackerSkipListCount=FWorld_nTrackerSkipListCount, CFWorldTracker ** FWorld_apTrackerSkipList=&FWorld_apTrackerSkipList[0]) { FASSERT( IsCreated() ); }

	FINLINE void SetProjectileReaction( ProjectileReaction_e nProjReact ) { FASSERT( IsCreated() ); FASSERT( nProjReact>=0 && nProjReact<PROJECTILE_REACTION_COUNT ); m_nProjectileReaction = nProjReact; }
	FINLINE ProjectileReaction_e GetProjectileReaction( void ) const { FASSERT( IsCreated() ); return (ProjectileReaction_e)m_nProjectileReaction; }
	
	FINLINE void EnableCameraCollision( BOOL bEnable );
	FINLINE BOOL IsCameraCollisionEnabled( void ) const { FASSERT( IsCreated() ); return !(m_nEntityFlags & ENTITY_FLAG_DISABLE_CAMERA_COLLISION); };

	virtual FINLINE const CFVec3A *GetApproxEyePoint( void ) const { return &m_MtxToWorld.m_vPos; }

	virtual FINLINE u32 GetTagPointCount( void ) const { return 1; }
	virtual FINLINE const CFVec3A *GetTagPoint( u32 nTagPointIndex ) const { return &m_MtxToWorld.m_vPos; }

	// Checks LOS. Specify a list of trackers to ignore, and/or an entity type mask.
	static BOOL IsLOSObstructed( const CFVec3A *pRayStart_WS, const CFVec3A *pRayEnd_WS, u64 uSkipEntityTypeMask,
								 u32 nTrackerSkipCount, const CFWorldTracker * const * ppTrackerSkipList );



	// Health Containers:
	FINLINE f32 HealthContainerCount( void ) const { FASSERT( IsCreated() ); return m_fHealthContainerCount; }
	FINLINE f32 InvHealthContainerCount( void ) const { FASSERT( IsCreated() ); return m_fOOHealthContainerCount; }
	void SetHealthContainerCount( f32 fHealthContainerCount );


	// Health:
	FINLINE f32 NormHealth( void ) const { FASSERT( IsCreated() ); return m_fNormHealth; }
	virtual void SetNormHealth( f32 fNormHealth );

	FINLINE f32 ComputeUnitHealth( void ) const { FASSERT( IsCreated() ); f32 fUnitHealth = m_fNormHealth * m_fOOHealthContainerCount; FMATH_CLAMPMAX( fUnitHealth, 1.0f ); return fUnitHealth; }
	void SetUnitHealth( f32 fUnitHealth );

	FINLINE f32 ConvertUnitToNormHealth( f32 fUnitHealth ) const { FASSERT( IsCreated() ); return fUnitHealth * m_fHealthContainerCount; }
	FINLINE f32 ConvertNormToUnitHealth( f32 fNormHealth ) const { FASSERT( IsCreated() ); f32 fUnitHealth = fNormHealth * m_fOOHealthContainerCount; FMATH_CLAMPMAX( fUnitHealth, 1.0f ); return fUnitHealth; }

	FINLINE void SetMaxHealth( void ) { FASSERT( IsCreated() ); SetUnitHealth( 1.0f ); }


	// Damage and Armor:
	FINLINE CDamageProfile *GetDamageProfile( void ) { FASSERT( IsCreated() ); return m_pDamageProfile; }
	void SetDamageProfile( CDamageProfile *pDamageProfile );
	void FindAndSetDamageProfile( cchar *pszDamageProfileName, BOOL bDisplayErrorIfNotFound=TRUE );

	FINLINE CArmorProfile *GetArmorProfile( void ) const { FASSERT( IsCreated() ); return m_pArmorProfile; }
	void SetArmorProfile( CArmorProfile *pArmorProfile );
	void FindAndSetArmorProfile( cchar *pszArmorProfileName, BOOL bDisplayErrorIfNotFound=TRUE );

	FINLINE f32 GetArmorModifier( void ) const { FASSERT( IsCreated() ); return m_fArmorModifier; }
	void SetArmorModifier( f32 fArmorModifier );
	void SetArmorModifier_UnitBoost( f32 fUnitBoost );
	void SetArmorModifier_UnitAttenuate( f32 fUnitBoost );

	FINLINE BOOL DoWeUseSphereTestForBlastDamage( void ) const { FASSERT( IsCreated() ); return (m_nEntityFlags & ENTITY_FLAG_USE_SPHERE_FOR_BLAST_DAMAGE); }
	FINLINE void UseSphereTestForBlastDamage( void ) { FASSERT( IsCreated() ); FMATH_SETBITMASK( m_nEntityFlags, ENTITY_FLAG_USE_SPHERE_FOR_BLAST_DAMAGE ); }
	FINLINE void UseNormalTestForBlastDamage( void ) { FASSERT( IsCreated() ); FMATH_CLEARBITMASK( m_nEntityFlags, ENTITY_FLAG_USE_SPHERE_FOR_BLAST_DAMAGE ); }


	// Damage:
	virtual void InflictDamage( CDamageData *pDamageData );
	virtual void InflictDamageResult( const CDamageResult *pDamageResult );
	virtual FINLINE void ShakeCamera( f32 fUnitIntensity, f32 fDurationInSecs, BOOL bControllerRumbleOnly=FALSE ) { FASSERT( IsCreated() ); }
	void ShakeCamera( const CDamageResult *pDamageResult );
	void InformAIOfDamageResult( const CDamageResult *pDamageResult );
	virtual void Die( BOOL bSpawnDeathEffects=TRUE, BOOL bSpawnGoodies=TRUE );

	static void ShatterWire( CFWire *pWire, BOOL bSilent=FALSE );
	virtual void ClassHierarchyShatterWire( CFWire *pWire, BOOL bSilent ) { FASSERT( IsCreated() ); }


	// AI:
	FINLINE void SetAIBrain( CAIBrain *pAIBrain ) { FASSERT( IsCreated() ); m_pAIBrain = pAIBrain; }
	FINLINE CAIBrain *AIBrain( void ) const { FASSERT( IsCreated() ); return m_pAIBrain; }


	// Controls:
	virtual void SetControls( CEntityControl *pControl );
	FINLINE CEntityControl *Controls( void ) const { FASSERT( IsCreated() ); return m_pControl; }
	FINLINE BOOL HasHumanOrBotControls( void ) const { FASSERT( IsCreated() ); return (!m_pControl || (m_pControl != &m_NoControl)); }


	// Action Button:
	FINLINE void SetActionFunction( ActionFunction_t *pFcnAction ) { FASSERT( IsCreated() ); m_pFcnAction = pFcnAction; }
	FINLINE ActionFunction_t* GetActionFunction( void ) const { FASSERT( IsCreated() ); return m_pFcnAction;}

	// Informs the Entity that it should consider itself to have pressed the action button.
	virtual FINLINE void NotifyActionButton() { FASSERT( IsCreated() ); }

	// Informs the Entity that another Entity (the one passed via parameter) pressed the action button near it.
	virtual BOOL ActionNearby( CEntity *pEntity );


	// Collectable:
	FINLINE BOOL IsCollectable( void ) const { return (BOOL)(m_nEntityFlags & ENTITY_FLAG_COLLECTABLE); }
	// Called when a CEntity gets collected.
	virtual BOOL GetCollected( CEntity *pCollector ) { return(FALSE); }

	// State Save & Restore:
	static BOOL CheckpointSaveAll( s32 nCheckpoint );
	static void CheckpointRestoreAll( s32 nCheckpoint );
	virtual void CheckpointSaveSelect( s32 nCheckpoint );
	virtual void CheckpointRestorePreamble( void );
	virtual BOOL CheckpointSave( void );
	virtual void CheckpointRestore( void );
	virtual void CheckpointRestorePostamble( void );
	void SetStateSaving( s32 nCheckpoint, BOOL bEnabled );
	FINLINE BOOL IsStateSaving( s32 nCheckpoint ) const { FASSERT(nCheckpoint<FCHECKPOINT_MAX_CHECKPOINTS); return (BOOL)m_abCheckpointSave[ nCheckpoint ]; }

	// Functionality to allow a child to re-attach itself to its parent after a checkpoint restore. 
	// (Don't use if parent entity already has code to re-attach its children after restoring, as with bot weapons.)
	FINLINE BOOL IsRestoreAttach( void ) const { return (BOOL)(m_nEntityFlags & ENTITY_FLAG_RESTORE_ATTACH); }
	void SetRestoreAttach( BOOL bAttach );


	// List Walking:
	static FINLINE u32 GetOutOfWorldCount( void ) { FASSERT( IsSystemInitialized() ); return m_OutOfWorldList_Root.nCount; }
	static FINLINE CEntity *OutOfWorldList_GetHead( void ) { FASSERT( IsSystemInitialized() ); return (CEntity *)flinklist_GetHead( &m_OutOfWorldList_Root ); }
	static FINLINE CEntity *OutOfWorldList_GetTail( void ) { FASSERT( IsSystemInitialized() ); return (CEntity *)flinklist_GetTail( &m_OutOfWorldList_Root ); }
	FINLINE CEntity *OutOfWorldList_GetNext( void ) const { FASSERT( IsSystemInitialized() ); return (CEntity *)flinklist_GetNext( &m_OutOfWorldList_Root, this ); }
	FINLINE CEntity *OutOfWorldList_GetPrev( void ) const { FASSERT( IsSystemInitialized() ); return (CEntity *)flinklist_GetPrev( &m_OutOfWorldList_Root, this ); }

	static FINLINE u32 GetInWorldCount( void ) { FASSERT( IsSystemInitialized() ); return m_InWorldList_Root.nCount; }
	static FINLINE CEntity *InWorldList_GetHead( void ) { FASSERT( IsSystemInitialized() ); return (CEntity *)flinklist_GetHead( &m_InWorldList_Root ); }
	static FINLINE CEntity *InWorldList_GetTail( void ) { FASSERT( IsSystemInitialized() ); return (CEntity *)flinklist_GetTail( &m_InWorldList_Root ); }
	FINLINE CEntity *InWorldList_GetNext( void ) const { FASSERT( IsSystemInitialized() ); return (CEntity *)flinklist_GetNext( &m_InWorldList_Root, this ); }
	FINLINE CEntity *InWorldList_GetPrev( void ) const { FASSERT( IsSystemInitialized() ); return (CEntity *)flinklist_GetPrev( &m_InWorldList_Root, this ); }

	static FINLINE u32 GetAutoWorkCount( void ) { FASSERT( IsSystemInitialized() ); return m_AutoWorkList_Root.nCount; }
	static FINLINE CEntity *AutoWorkList_GetHead( void ) { FASSERT( IsSystemInitialized() ); return (CEntity *)flinklist_GetHead( &m_AutoWorkList_Root ); }
	static FINLINE CEntity *AutoWorkList_GetTail( void ) { FASSERT( IsSystemInitialized() ); return (CEntity *)flinklist_GetTail( &m_AutoWorkList_Root ); }
	FINLINE CEntity *AutoWorkList_GetNext( void ) const { FASSERT( IsSystemInitialized() ); return (CEntity *)flinklist_GetNext( &m_AutoWorkList_Root, this ); }
	FINLINE CEntity *AutoWorkList_GetPrev( void ) const { FASSERT( IsSystemInitialized() ); return (CEntity *)flinklist_GetPrev( &m_AutoWorkList_Root, this ); }

	static FINLINE u32 GetMarkedCount( void ) { FASSERT( IsSystemInitialized() ); return m_MarkedList_Root.nCount; }
	static FINLINE CEntity *MarkedList_GetHead( void ) { FASSERT( IsSystemInitialized() ); return (CEntity *)flinklist_GetHead( &m_MarkedList_Root ); }
	static FINLINE CEntity *MarkedList_GetTail( void ) { FASSERT( IsSystemInitialized() ); return (CEntity *)flinklist_GetTail( &m_MarkedList_Root ); }
	FINLINE CEntity *MarkedList_GetNext( void ) const { FASSERT( IsSystemInitialized() ); return (CEntity *)flinklist_GetNext( &m_MarkedList_Root, this ); }
	FINLINE CEntity *MarkedList_GetPrev( void ) const { FASSERT( IsSystemInitialized() ); return (CEntity *)flinklist_GetPrev( &m_MarkedList_Root, this ); }

	static FINLINE u32 GetCheckpointSaveCount( s32 nCheckpoint ) { FASSERT( IsSystemInitialized() ); return m_CheckpointSaveList_Root[nCheckpoint].nCount; }
	static FINLINE CEntity *CheckpointSaveList_GetHead( s32 nCheckpoint ) { FASSERT( IsSystemInitialized() ); return (CEntity *)flinklist_GetHead( &m_CheckpointSaveList_Root[nCheckpoint] ); }
	static FINLINE CEntity *CheckpointSaveList_GetTail( s32 nCheckpoint ) { FASSERT( IsSystemInitialized() ); return (CEntity *)flinklist_GetTail( &m_CheckpointSaveList_Root[nCheckpoint] ); }
	FINLINE CEntity *CheckpointSaveList_GetNext( s32 nCheckpoint ) const { FASSERT( IsSystemInitialized() ); return (CEntity *)flinklist_GetNext( &m_CheckpointSaveList_Root[nCheckpoint], this ); }
	FINLINE CEntity *CheckpointSaveList_GetPrev( s32 nCheckpoint ) const { FASSERT( IsSystemInitialized() ); return (CEntity *)flinklist_GetPrev( &m_CheckpointSaveList_Root[nCheckpoint], this ); }
	void CheckpointSaveList_Remove( s32 nCheckpoint );
	void CheckpointSaveList_AddTailAndMark( s32 nCheckpoint );

	// ambient audio
	BOOL SetAmbientSFXHandle( FSndFx_FxHandle_t hSFX ) { FASSERT( IsCreated() ); m_hAmbientSFX = hSFX; return TRUE; }
	FSndFx_FxHandle_t GetAmbientSFXHandle( void ) { FASSERT( IsCreated() ); return m_hAmbientSFX; }
	BOOL PlayAmbientSFX( void );
	void StopAmbientSFX( void );
	void SetAmbientVolume( f32 fVolume );
	f32 GetAmbientVolume( void ) { FASSERT( IsCreated() ); return m_fAmbientVolume; }
	void SetAmbientRadius( f32 fRadius );
	f32 GetAmbientRadius( void ) { FASSERT( IsCreated() ); return m_fAmbientRadius; };
	void SetAmbientPropertiesFromSoundGroup( CFSoundGroup *pSoundGroup, BOOL bPlay=FALSE );

	// Debug:
	// Prints debug text about every entity on the screen.
	static void DrawAllDebugText( void );

	FINLINE virtual CFWorldMesh *GetMesh( void ) const { return NULL; }




//----------------------------------------------------------------------------------------------------------------------------------
// Protected Functions:
//----------------------------------------------------------------------------------------------------------------------------------
protected:

	virtual FINLINE BOOL ClassHierarchyLoadSharedResources( void ) { return TRUE; }
	virtual FINLINE void ClassHierarchyUnloadSharedResources( void ) {}

	virtual void ClassHierarchyDestroy( void );
	FINLINE u32 ClassHierarchyBit( void ) const { return ENTITY_CLASS_HIERARCHY_BIT; }
	FINLINE BOOL IsOurWorkBitSet( void ) const { return (BOOL)(GetClassHierarchyWorkMask() & ClassHierarchyBit()); }

	virtual FINLINE void ClassHierarchyWork( void ) {}
	virtual BOOL ClassHierarchyBuild( void );				// Must be implemented by all classes in the derivation chain
	virtual BOOL ClassHierarchyBuilt( void );
	virtual CEntityBuilder *GetLeafClassBuilder( void );	// Must be implemented by all classes in the derivation chain
	virtual void ClassHierarchyResolveEntityPointerFixups( void );

	virtual void ClassHierarchyAddToWorld( void );
	virtual void ClassHierarchyRemoveFromWorld( void );
	virtual FINLINE void ChildHasBeenAddedToWorld( CEntity *pChildEntity ) {}
	virtual FINLINE void ChildHasBeenRemovedFromWorld( CEntity *pChildEntity ) {}

	virtual FINLINE void ClassHierarchyDrawEnable( BOOL bDrawingHasBeenEnabled ) { FASSERT( IsCreated() ); }

	virtual FINLINE CFMtx43A *ClassHierarchyAttachChild( CEntity *pChildEntity, cchar *pszAttachBoneName=NULL ) { return NULL; }
	virtual FINLINE void ClassHierarchyDetachChild( CEntity *pChildEntity ) {}
//	virtual FINLINE void ClassHierarchyRelocated( void *pIdentifier ) {}
	virtual FINLINE void ClassHierarchyRelocated( void *pIdentifier );
	virtual void ClassHierarchyBecameActive( void );	// entity mesh just entered active list
	virtual void ClassHierarchyBecameInactive( void );	// entity mesh just left active list

	void EnableClassHierarchyWorkBit( u32 nClassHierarchyBit );
	void DisableClassHierarchyWorkBit( u32 nClassHierarchyBit );

	virtual u32 TripwireCollisionTest( const CFVec3A *pPrevPos_WS, const CFVec3A *pNewPos_WS ) { FASSERT( IsCreated() ); return TRIPWIRE_COLLFLAG_NONE; }




//----------------------------------------------------------------------------------------------------------------------------------
// Private Functions:
//----------------------------------------------------------------------------------------------------------------------------------
private:

	FINLINE BOOL _IsBuilding( void ) const { return (BOOL)(m_nEntityFlags & ENTITY_FLAG_BUILDING); }

	void _SetDefaults( void );
	static void _DestroyAll( void );
	static void _VerletExplodeCallback( CFVerlet *pVerletToExplode );


	static BOOL _WorldCallback( FWorldEvent_e nEvent );
	static BOOL _WorldShapeCreateCallback( cchar *pszWorldResName, const void *pFixupOffsetBase, const CFWorldShapeInit *pShapeInitArray, u32 nShapeInitCount );
	static void _AttachWorldShape( CEntity *pChildEntity, CFWorldShapeInit *pShapeInitArray );
	static CEntity *_CreateWorldShape( cchar *pszWorldResName, CFWorldShapeInit *pShapeInit, const void *pFixupOffsetBase );

	static void _OutOfWorldList_Init( void ) { FASSERT( IsSystemInitialized() ); flinklist_InitRoot( &m_OutOfWorldList_Root, FANG_OFFSETOF( CEntity, m_EntityLink ) ); }
	FINLINE void _OutOfWorldList_AddTail( void ) { FASSERT( IsSystemInitialized() ); flinklist_AddTail( &m_OutOfWorldList_Root, this ); }
	FINLINE void _OutOfWorldList_Remove( void ) { FASSERT( IsSystemInitialized() ); flinklist_Remove( &m_OutOfWorldList_Root, this ); }

	static void _InWorldList_Init( void ) { FASSERT( IsSystemInitialized() ); flinklist_InitRoot( &m_InWorldList_Root, FANG_OFFSETOF( CEntity, m_EntityLink ) ); }
	FINLINE void _InWorldList_AddTail( void ) { FASSERT( IsSystemInitialized() ); flinklist_AddTail( &m_InWorldList_Root, this ); }
	FINLINE void _InWorldList_Remove( void ) { FASSERT( IsSystemInitialized() ); flinklist_Remove( &m_InWorldList_Root, this ); }

	static void _AutoWorkList_Init( void ) { FASSERT( IsSystemInitialized() ); flinklist_InitRoot( &m_AutoWorkList_Root, FANG_OFFSETOF( CEntity, m_AutoWorkLink ) ); }
	FINLINE void _AutoWorkList_AddTail( void ) { FASSERT( IsSystemInitialized() ); flinklist_AddTail( &m_AutoWorkList_Root, this ); }
	FINLINE void _AutoWorkList_Remove( void ) { FASSERT( IsSystemInitialized() ); flinklist_Remove( &m_AutoWorkList_Root, this ); }

	static void _MarkedList_Init( void ) { FASSERT( IsSystemInitialized() ); flinklist_InitRoot( &m_MarkedList_Root, FANG_OFFSETOF( CEntity, m_MarkedLink ) ); }
	FINLINE void _MarkedList_AddTail( void ) { FASSERT( IsSystemInitialized() ); flinklist_AddTail( &m_MarkedList_Root, this ); }
	FINLINE void _MarkedList_Remove( void ) { FASSERT( IsSystemInitialized() ); flinklist_Remove( &m_MarkedList_Root, this ); }

	static void _CheckpointSaveList_Init( s32 nCheckpoint ) { FASSERT( IsSystemInitialized() ); flinklist_InitRoot( &m_CheckpointSaveList_Root[nCheckpoint], FANG_OFFSETOF( CEntity, m_CheckpointSaveLink[nCheckpoint] ) ); }


	void _CheckMovedEntityAgainstTripwires( const CFVec3A *pPrevPos_WS, const CFVec3A *pNewPos_WS );
#if FANG_WINGC
	BOOL _CoopHoldTripwireEnter( CEntity *pTripper );
	void _CoopFireTripwireEnter( CEntity *pTripper );
	u32 _CoopPlayersInsideMask( void );
#endif
	void _ClearTripwireArray( void );
	void _UpdateTripwireArrayBasedOnFilterChange( void );
	BOOL _DoesTripperPassTripwireFilterTest( CEntity *pTripperEntity );
	BOOL _EntityIsInsideTripwire( CEntity *pTripwireEntity );
	void _EntityIsOutsideTripwire( CEntity *pTripwireEntity );

	void _Compute_ChildMtxAndScale( void );
	void _Compute_MtxToParent_And_ScaleToParent_From_Modified_MtxToWorld_And_ScaleToWorld( void );
	void _Compute_MtxToWorld_And_ScaleToWorld_From_Modified_MtxToParent_And_ScaleToParent( void );
	void _Compute_MtxToParent_From_Modified_MtxToWorld( void );
	void _Compute_MtxToWorld_From_Modified_MtxToParent( void );
	void _Compute_ScaleToParent_From_Modified_ScaleToWorld( void );
	void _Compute_ScaleToWorld_From_Modified_ScaleToParent( void );
	void _Compute_MtxToWorld_And_ScaleToParent_From_Modified_MtxToParent_And_ScaleToWorld( void );
	void _RelocatePostamble( const CFVec3A *pPrevPos_WS, BOOL bEnableTripping, void *pIdentifier );

	void _DetachAndRemoveAllChildrenAndRecurse( void );


	friend class CEntityBuilder;


	FCLASS_STACKMEM_ALIGN( CEntity );
} FCLASS_ALIGN_SUFFIX;


FINLINE void CEntity::ClassHierarchyRelocated( void *pIdentifier ) {
	if( m_pAmbientEmitter ) {
		m_pAmbientEmitter->SetPosition( &m_MtxToWorld.m_vPos );
	}
}


FINLINE void CEntity::EnableWorkWhilePaused( BOOL bEnable ) {
	if( bEnable ) {
		FMATH_SETBITMASK( m_nEntityFlags, ENTITY_FLAG_ALLOW_WORK_IF_PAUSED );
	} else {
		FMATH_CLEARBITMASK( m_nEntityFlags, ENTITY_FLAG_ALLOW_WORK_IF_PAUSED );
	}
}


FINLINE void CEntity::SetControls( CEntityControl *pControl ) {
	FASSERT( IsCreated() );

	if( pControl ) {
		m_pControl = pControl;
	} else {
		m_pControl = &m_NoControl;
	}
}




//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CEntityBuilder
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

FCLASS_ALIGN_PREFIX class CEntityBuilder {
//----------------------------------------------------------------------------------------------------------------------------------
// Public Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	typedef enum {
		VERLET_ANCHOR_NONE,						// Don't anchor this tack
		VERLET_ANCHOR_ORIG_TACK_POS,			// Anchor to this tack's original position (static)
		VERLET_ANCHOR_SPECIFIED_TACK,			// Anchor to the specified tack (dynamic)
		VERLET_ANCHOR_ENTITY_ORIGIN_STATIC,		// Anchor to the specified entity at the point defined by the entity's original origin (static)
		VERLET_ANCHOR_ENTITY_ORIGIN_DYNAMIC,	// Anchor to the specified entity at the point defined by the entity's dynamic origin (dynamic)

		VERLET_ANCHOR_COUNT
	} VerletAnchor_e;


	typedef struct {
		cchar *pszTackName;
		cchar *pszAnchorAttachToEntity;				// The name of the entity to anchor to
		cchar *pszAnchorAttachToTack;				// The name of the tack to anchor to

		const CArmorProfile *pArmorProfile;			// The armor profile to use (never NULL)
		FExplosion_GroupHandle_t hExplosionGroup;	// The explosion group to assign to this tack (FEXPLOSION_INVALID_HANDLE for none)

		BOOL bAnchorEnabled;						// TRUE if this anchor is enabled
		VerletAnchor_e nAnchor;						// The type of anchor for this tack

		f32 fMinConstraintDist;						// Minimum distance the tack is allowed from its anchor point
		f32 fMaxConstraintDist;						// Maximum distance the tack is allowed from its anchor point

		f32 fAnchorHealth;							// Anchor health
		u8 nWireStyleCode;							// Wire style code (-1=none)
		BOOL bAnchorSlice;							// TRUE=sliceable
	} VerletTackDef_t;




//----------------------------------------------------------------------------------------------------------------------------------
// Public Data:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	cchar *m_pszEC_EntityName;							// Entity name
	cchar *m_pszEC_AttachParent;						// Parent entity name to attach to (NULL=none)
	cchar *m_pszEC_AttachBone;							// Bone name to attach to (NULL=none)
	cchar *m_pszEC_ParentEntity;						// Parent entity name (NULL=none... use m_nEC_ParentShapeIndex instead)
	s32 m_nEC_ParentShapeIndex;							// Index into shape array of this entity's parent entity (-1=none)
	u64 m_nEC_TypeBits;									// Entity type bits
	u64 m_nEC_LeafTypeBit;								// Entity leaf type bit
	u32 m_nEC_Flags;									// See EC_FLAG_* for info
	f32 m_fEC_Scale_WS;									// Initial scale in world space
	u32 m_nEC_HealthContainerCount;						// Number of health containers
	CFMtx43A m_EC_Mtx_WS;								// Initial orientation in world space
	f32 m_fEC_NormHealth;								// Normalized health
	f32 m_fEC_ArmorModifier;							// Used to modify the profile armor settings (-1 to +1)
	cchar *m_pszEC_DamageProfile;						// The name of the damage profile to use (NULL=no damage)
	cchar *m_pszEC_ArmorProfile;						// The name of the armor profile to use (NULL=infinite armor)
	cchar *m_pszAIBuilderName;							// The parameter of the "AI=" command
	CGoodieBag m_oGoodieBag;
	CGoodieProps *m_pGoodieProps;						// If this entity can be used as a goodie distributor, this will point to some properties for such distribution.
	BOOL8 m_bEC_ClassIsTripwireCapable;					// The derived class is capable of being a tripwire
	BOOL8 m_bIsCollectable;
	BOOL8 m_bDieWhenCollide;											// Auto-die when the Verlet object passes through a surface

	BOOL8 m_bAmbientPlay;								// TRUE if ambient sound should play upon entity creation
	cchar *m_pszAmbientTag;								// string representing fsndfx tag of entity's ambient sound
	f32 m_fAmbientVolume;								// volume of ambient sound
	f32 m_fAmbientRadius;								// radius of ambient sound

	CAIBuilder *m_pAIBuilder;							// Points to AI builder object (NULL=none)
	CTripwireBuilder m_TripwireBuilder;					// Collects tripwire specific info

	BOOL8 m_bAllowVerlet;											// Set by the derived class to indicate that it wishes to have Verlet user props parsed
	BOOL8 m_bVerletEnabled;											// Enable Verlet physics for this entity
	BOOL8 m_bVerletFreeze;											// Freeze the physics of this object
	BOOL8 m_bVerletFreezeWhenMotionless;							// Freeze the physics object when it goes motionless
	CFVerlet::Init_t m_VerletInit;									// Several init parameters
	f32 m_fVerletMass;												// Verlet object mass
	f32 m_fVerletDimX;												// Verlet object X dimension
	f32 m_fVerletDimY;												// Verlet object Y dimension
	f32 m_fVerletDimZ;												// Verlet object Z dimension
	f32 m_fVerletUnitDampening;										// Verlet object unit dampening factor
	f32 m_fVerletPrimeSecs;											// Verlet object prime seconds
	f32 m_fVerletGravity;											// Verlet object gravity
	u32 m_nVerletSurfaceCount;										// Number of meshes
	cchar **m_ppszVerletSurfaceNames;								// Pointer to array of mesh names (will be added to string table automatically)
	
	VerletTackDef_t m_aTackDef[ENTITY_MAX_VERLET_TACKS_PER_OBJECT];	// Verlet tack definitions
	u32 m_nTackDefCount;											// Number of valid definitions in m_aTackDef[]

	u8 m_nProjectileReaction;							// See CEntity::ProjectileReaction_e for info




//----------------------------------------------------------------------------------------------------------------------------------
// Public Functions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	FINLINE CEntityBuilder() { m_pAIBuilder = NULL; }
	BOOL InterpretFile( void );
	virtual void SetDefaults( u64 nEntityTypeBits, u64 nEntityLeafTypeBit, cchar *pszEntityType );
	VerletTackDef_t *FindTackDef( cchar *pszTackName, BOOL bAppendIfNotFound );
	BOOL InitVerlet( CFVerlet *pVerlet );
	static void FixupVerlet( CEntity *pEntity, CFVerlet *pVerlet );



//----------------------------------------------------------------------------------------------------------------------------------
// Protected Functions:
//----------------------------------------------------------------------------------------------------------------------------------
protected:

	virtual BOOL InterpretTable( void );
	virtual BOOL PostInterpretFixup( void );


	FCLASS_STACKMEM_ALIGN( CEntityBuilder );
} FCLASS_ALIGN_SUFFIX;




//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CEntityParser
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

FCLASS_NOALIGN_PREFIX class CEntityParser {
public:

	static FGameDataFileHandle_t m_hRootFile;
	static cchar *m_pszEntityType;
	static cchar *m_pszEntityName;
	static cchar *m_pszAIBuilderName;
	static const CFWorldShapeInit *m_pWorldShapeInit;
	static u64 m_nEntityTypeBits;
	static u64 m_nEntityLeafTypeBit;

	static FGameDataTableHandle_t m_hTable;
	static u32 m_nFieldCount;
	static cchar *m_pszTableName;
	static cchar *m_pszFileName;

	static FGameDataWalker_t m_Walker;


	FINLINE CEntityParser() { _Zero(); }

	static BOOL Begin( const CFWorldShapeInit *pWorldShapeInit );

	static BOOL SetTable( FGameDataTableHandle_t hTable );
	static BOOL SetFirstTable( void );
	static BOOL SetNextTable( void );

	static BOOL Interpret_CopyStringToFMem( cchar *pszSrcName, char **ppszFMemName, BOOL bRemoveExtension );
	static BOOL Interpret_CopyWildcardStringToFMem( cchar *pszSrcName, char **ppszWildcardFMemName, BOOL bRemoveExtension );

	static BOOL CompareBoolString( cchar *pszBoolString, BOOL *pbBool );
	static BOOL Interpret_String( cchar **ppszString, BOOL bCheckFieldCountOne=TRUE, u32 nFieldIndex=0, BOOL bNoneToNull=FALSE );
	static BOOL Interpret_StringArray( cchar **ppszStringArray, u32 nMaxArrayElements, u32 nFieldIndex=0, BOOL bNoneToNull=FALSE );
	static BOOL Interpret_BOOL( BOOL *pbData, BOOL bInvertValue=FALSE, s32 nTableFieldIndex=-1 );
	static BOOL Interpret_Flag( u32 *pnData, u32 nBitMask, BOOL bInvertValue=FALSE, s32 nTableFieldIndex=-1 );
	static BOOL Interpret_U32( u32 *pnData, u32 nMinValue=0, u32 nMaxValue=0xffffffff, BOOL bClamp=FALSE );
	static BOOL Interpret_S32( s32 *pnData, s32 nMinValue=0x80000000, s32 nMaxValue=0x7fffffff, BOOL bClamp=FALSE );
	static BOOL Interpret_F32( f32 *pfData, f32 fMinValue=-1e+20f, f32 fMaxValue=1e+20f, BOOL bClamp=FALSE );
	static BOOL Interpret_F32Array( f32 *pfDestArray, u32 nMaxArrayElements, f32 fMinValue=-1e+20f, f32 fMaxValue=1e+20f, BOOL bClamp=FALSE, u32 nStartFieldIndex=0 );

	static BOOL Interpret_StringArray_CheckSyntax( FGameDataTableHandle_t *phTable );
	static BOOL Interpret_StringArray_BuildInFMem( cchar ***pppszStringArray );

	static BOOL Interpret_F32Array_CheckSyntax( FGameDataTableHandle_t *phTable );

	static void Error_OutOfMemory( void );
	static void Error_UnknownCommand( void );
	static void Error_InvalidParameterCount( void );
	static void Error_InvalidParameterRange( void );
	static void Error_InvalidParameterValue( void );
	static void Error_InvalidParameterType( void );
	static void Error_CommandAlreadySpecified( void );
	static void Error_InvalidObjectType( void );
	static void Error_CouldNotCreateAIBrain( void );
	static void Error_Prefix( void );
	static void Error_Dashes( void );

private:
	static void _Zero( void );
	static void _SetEntityName( void );
	static void _SetEntityType( void );
	static void _SetEntityAIBuilder( void );


	FCLASS_STACKMEM_NOALIGN( CEntityParser );
} FCLASS_NOALIGN_SUFFIX;



FINLINE CFSphereA *CEntity::TripwireBoundingSphere_WS( void ) const {
	FASSERT( IsCreated() );

	if( m_pTripwire ) {
		return &m_pTripwire->m_BoundingSphere_WS;
	} else {
		return NULL;
	}
}


FINLINE u32 CEntity::TripwireEventFlags( void ) const {
	FASSERT( IsCreated() );

	if( m_pTripwire ) {
		return m_pTripwire->m_nEventFlags;
	} else {
		return TRIPWIRE_EVENTFLAG_NONE;
	}
}


FINLINE u32 CEntity::TripwireFilterFlags( void ) const {
	FASSERT( IsCreated() );

	if( m_pTripwire ) {
		return m_pTripwire->m_nTripperFilterFlags;
	} else {
		return TRIPWIRE_FILTER_INCLUDE_NONE;
	}
}


FINLINE u32 CEntity::GetContainedTripperCount( void ) const {
	FASSERT( IsCreated() );

	if( m_pTripwire ) {
		return m_pTripwire->m_nContainedEntityCount;
	} else {
		return 0;
	}
}


FINLINE CEntity **CEntity::GetContainedTripperArray( void ) const {
	FASSERT( IsCreated() );

	if( m_pTripwire ) {
		return m_pTripwire->m_ppContainedTrippersArray;
	} else {
		return NULL;
	}
}


FINLINE BOOL CEntity::TripwireDamage_IsBuiltForDamage( void ) const {
	FASSERT( IsCreated() );

	if( m_pTripwire ) {
		return (m_pTripwire->m_pDamageForm != NULL);
	} else {
		return NULL;
	}
}


FINLINE BOOL CEntity::TripwireDamage_IsEnabled( void ) const {
	FASSERT( IsCreated() );

	if( m_pTripwire && m_pTripwire->m_pDamageForm ) {
		return m_pTripwire->m_bDamageEnabled;
	} else {
		return NULL;
	}
}


FINLINE void CEntity::TripwireDamage_Enable( BOOL bEnable ) {
	FASSERT( IsCreated() );

	if( m_pTripwire && m_pTripwire->m_pDamageForm ) {
		m_pTripwire->m_bDamageEnabled = bEnable;
	}
}


FINLINE f32 CEntity::TripwireDamage_GetNormIntensity( void ) const {
	FASSERT( IsCreated() );

	if( m_pTripwire && m_pTripwire->m_pDamageForm ) {
		return m_pTripwire->m_fDamageNormIntensity;
	} else {
		return NULL;
	}
}


FINLINE void CEntity::TripwireDamage_SetNormIntensity( f32 fNormIntensity ) {
	FASSERT( IsCreated() );

	if( m_pTripwire && m_pTripwire->m_pDamageForm ) {
		FMATH_CLAMPMIN( fNormIntensity, 0.0f );

		if( m_pTripwire->m_fDamageNormIntensity != fNormIntensity ) {
			m_pTripwire->m_fDamageNormIntensity = fNormIntensity;

			m_pTripwire->m_pDamageForm->m_fNormIntensity = fNormIntensity;
			CDamage::ResubmitDamageForm( m_pTripwire->m_pDamageForm );
		}
	}
}

FINLINE void CEntity::EnableCameraCollision( BOOL bEnable ) {
	FASSERT( IsCreated() );
	if( bEnable ) {
		FMATH_CLEARBITMASK( m_nEntityFlags, ENTITY_FLAG_DISABLE_CAMERA_COLLISION );
	} else {
		FMATH_SETBITMASK( m_nEntityFlags, ENTITY_FLAG_DISABLE_CAMERA_COLLISION );
	}
}


#endif

