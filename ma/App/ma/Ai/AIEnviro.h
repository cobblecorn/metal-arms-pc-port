//////////////////////////////////////////////////////////////////////////////////////
// AIEnviro.h - 
//
//	  Global Record of Sounds and potentially other things that happen in the environement.
//
//    CAIBrain objects can reference the AIEnviro as part of their perception/reaction module
//
//	  AISounds:  
//		  Totally separate from "real" sounds like the ones that might play through speakers.
//		  Use AISounds to notify AI of "real" sounds, so that they can react if desired.
//
//	  Visible representation for Entities that don't have brains
//		  Use  AISOUNDCTRL_VISIBLE_ENTITY
//
// Author: Pat MacKellar 
//////////////////////////////////////////////////////////////////////////////////////
// THIS CODE IS PROPRIETARY PROPERTY OF SWINGIN' APE STUDIOS, INC.
// Copyright (c) 2002
//
// The contents of this file may not be disclosed to third or fourth
// parties, copied or duplicated in any form, in whole or in part,
// without the prior written permission of Swingin' Ape Studios, Inc.
//////////////////////////////////////////////////////////////////////////////////////
// Modification History:
//
// Date     Who         Description
// -------- ----------  --------------------------------------------------------------
// 07/03/02 MacKellar   Created.
// 12/04/02 MacKelalr	Implemented VISIBLE_ENTITY_ Control flag
//////////////////////////////////////////////////////////////////////////////////////
#ifndef _AIENVIRO_H_
#define _AIENVIRO_H_ 1

#include "fmath.h"

class CEntity;
class CAIBrain;
class CAISound;

enum
{
	AIENVIRO_DEFAULT_SOUND_GLOBAL_BANK_SIZE = 50
};

//
// note from pgm:
//   sound types have become more or less like desire reactions, except that certain bots may react differently, or not at all, or prioritize reactions differently
//
enum
{
	AISOUNDTYPE_BOT = 0,				// some sound that is very obviously from a bot (includes the sound a weapon makes when it fires)
	AISOUNDTYPE_GRENADE,				// a grenade bouncing sound
	AISOUNDTYPE_EXPLOSION,				// sound of an explosion
	AISOUNDTYPE_WEAPON,					// some sound that is very obviously from a weapon (impact, projectile flying by, etc)
	AISOUNDTYPE_RAGE,					// this is a sound a bot makes when it is enraged
	AISOUNDTYPE_PROJECTILE,				// this is a a projectile that is in the air. normally elicits a two stage look at reaction.
	AISOUNDTYPE_DODGEME,				// this sound s coming from something big.  ai might want to dodge it.
	AISOUNDTYPE_BOTFOOD,				// this is the sound of something that bots aren't scared of, but probably are will investigate
	AISOUNDTYPE_ALARM,					// alarm going off
	AISOUNDTYPE_SWARMERBARRIER,			// This soun's radius forms a swarmer barrier
	AISOUNDTYPE_NONE,					// this is for visible entities that aren't making any sound

	AISOUNDTYPE_COUNT
};

enum
{
	AISOUNDCTRL_NONE				= 0x0000,
	AISOUNDCTRL_PERSISTENT			= 0x0001,			// this is a sound that never dies out
	AISOUNDCTRL_VISIBLE_ENTITY		= 0x0002,			// AI can see this sound object (used for entities that we want AI to see. important since AIBrains only see other AIBrains)
	AISOUNDCTRL_NEW_SOUND			= 0x0004,			// will be set on a sound that is in it's first frame of life. Can be used to make existing sounds to be reevaluated as new ones
	AISOUNDCTRL_SWARMERBARRIER		= 0x0008,			//  This sound's sphere forms a swarmer barrier
	AISOUNDCTRL_MODIFIED_SOUND		= 0x0010,			//  Internal flag
	AISOUNDCTRL_NEW_CLEAR			= 0x0020,			//  Internal flag
	AISOUNDCTRL_MODIFIED_CLEAR		= 0x0040,			//  Internal flag
};


typedef u16 AISoundHandle;
typedef u8 AISoundType;			// See AISOUNDTYPE_* for more info
typedef u8 AISoundCtrlFlag;		// See AISOUNDCTRL_* for more info

BOOL AIEnviro_InitSystem(struct FLinkRoot_s* pGlobalPtrNodePool, u32 uSoundBankSize = AIENVIRO_DEFAULT_SOUND_GLOBAL_BANK_SIZE);
void AIEnviro_UninitSystem(void);
void AIEnviro_KillAllSounds(void);
void AIEnviro_Work(void);
void AIEnviro_DebugRender(void);

//Add a Sound into the AI Environment   returns a handle to a sound
AISoundHandle AIEnviro_AddSound(const CFVec3A& Origin,
								f32 fRadius,
								f32 uHowLongSecs,
								AISoundType uSoundType,
								AISoundCtrlFlag uSoundCtrlFlag,
								CEntity* pEntityOfOrigin = NULL);

//Modify a Sound that is currently playing
BOOL AIEnviro_ModifySound(	AISoundHandle uSoundHandle,
							const CFVec3A& Origin,
							f32 fRadius,
							f32 fHowLongSecs,
							AISoundType uSoundType,
							CEntity* pEntityOfOrigin);

//Determine if this sound is intersecting a sphere in the world
//Callback will be called for each sound
typedef BOOL SoundCollisionCallback(CAISound* pSound, void* pData);		  //CB should return TRUE to stop iteration
void AIEnviro_DetectSoundCollisions(const CFVec3A &Loc,
									CEntity* pListener,
									f32 fHearingMagnification,	  //the radius of all sounds will be magnified by this (represents ear quality)
									SoundCollisionCallback *pFunc,
									void* pData);
//got a handle, need the sound?
CAISound* AIEnviro_SoundHandleToPtr(AISoundHandle uSoundHandle);

//call the cb func for each living AI-Visible-Projectile
void AIEnviro_IterateAllAIVisibleEntities(SoundCollisionCallback pFunc, void* pData);

//each player Bot has a sound associated with him. Call this to boost it for a brief period
void AIEnviro_BoostPlayerSoundTo(u32 uPlayerId, f32 fRad);

//
//  Class used to represent a Sound in the AI Environment
//	  - Passed to SoundCollisionCallback function by DetectSoundCollisions
//
FCLASS_ALIGN_PREFIX class CAISound
{
public:
	CAISound(void)	: m_uHandle(0) {}
	CFVec3A m_Origin;							//where the sound is coming from
	f32 m_fAudibleRadius;						//the audible radius		
	CEntity* m_pEntity;							//the thing making this sound
	f32 m_fHowLong;								//How long in seconds it should exist
	f32 m_fWhen;								//When it originated
	u16 m_uHandle;
	AISoundType m_uSoundType;					//Use an AISOUNDTYPE....
	AISoundCtrlFlag m_uSoundCtrlFlags;			//Use one or more AISOUNDCTRL
	FCLASS_STACKMEM_ALIGN(CAISound); 
} FCLASS_ALIGN_SUFFIX;




extern f32 AIEnviro_fGrenadeAudibleRange;
extern f32 AIEnviro_fExplosionSoundRadToAIAudibleRadScale;
extern f32 AIEnviro_fLaserImpactAudibleRange;
extern f32 AIEnviro_fProjectileVisualRadius;
extern f32 AIEnviro_fVehicleBotSoundRadius;
extern f32 AIEnviro_fFlamerImpactSoundRadius;
extern f32 AIEnviro_fMagmaImpactSoundRadius;




#endif

