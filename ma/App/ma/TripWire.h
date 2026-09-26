//////////////////////////////////////////////////////////////////////////////////////
// TripWire.h - 
//
// Author: Pat MacKellar 
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
// 07/09/02 MacKellar   Created (moved here from entity.h).
//////////////////////////////////////////////////////////////////////////////////////
#ifndef _TRIPWIRE_H_
#define _TRIPWIRE_H_ 1

#include "fang.h"
#include "fmath.h"
#include "flinklist.h"
#include "damage.h"

class CEntity;
class CEntityBuilder;
class CDoorEntity;




//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CTripwire
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

FCLASS_ALIGN_PREFIX class CTripwire {
//----------------------------------------------------------------------------------------------------------------------------------
// Public Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	typedef enum {
		TRIPWIRE_KILLMODE_NONE = 0,						// Tripwire does no instant death
		TRIPWIRE_KILLMODE_PLANE,						// Tripwire does instant death when passing through plane of origin
		TRIPWIRE_KILLMODE_VOLUME,						// Tripwire does instant death when entering the tripwire volume

		TRIPWIRE_KILLMODE_COUNT
	} KillMode_e;

	static s32 m_nTripwireEventNumber;		// Given to us by CFScriptSystem. This is the event number for tripwires (-1=none).




//----------------------------------------------------------------------------------------------------------------------------------
// Public Variables:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	CFSphereA m_BoundingSphere_MS;			// Tripwire's bounding sphere in model space
	CFSphereA m_BoundingSphere_WS;			// Tripwire's bounding sphere in world space

	FLink_t m_Link;							// Link to other tripwires
	CEntity *m_pOwnerEntity;				// This tripwire structure is an extension of this entity
	cchar *m_pszTripwireFilterEntityName;	// Describes the name of the entity tripper that may trip this tripwire (NULL=allow all)
	u32 m_nEventFlags;						// See CEntity::TRIPWIRE_EVENTFLAG_* for info

	u32 m_nTripperFilterFlags;				// Describes which types of trippers may trip this tripwire. See CEntity::TRIPWIRE_FILTER_INCLUDE_* for info

	u32 m_nMaxContainedEntityCount;			// Maximum number of entity pointers m_ppContainedTrippersArray can hold
	u32 m_nContainedEntityCount;			// Number of entities in m_ppContainedTrippersArray
	CEntity **m_ppContainedTrippersArray;	// Array of entity pointers that are contained in this tripwire
	CDoorEntity *m_pDoorToOpen;				// Pointer to entity of door to open (if any) (Note: before building will contain pointer to string name of entity)
	u32 m_nTripwireTriggerMode;				// Mode of tripwire triggering (single, multiple)

	BOOL m_bDamageEnabled;
	const CDamageProfile *m_pDamageProfile;
	f32 m_fDamageNormIntensity;
	CDamageForm *m_pDamageForm;

	KillMode_e m_eKillMode;					// What type of killmode params does this tripwire have? Default == NONE
	BOOL m_bKillModeSpawnDeathEffects;		// When this tripwire kills an entity, does it trigger the entity death special effects? DEFAULT == FALSE


//----------------------------------------------------------------------------------------------------------------------------------
// Private Variables:
//----------------------------------------------------------------------------------------------------------------------------------
private:

	static FLinkRoot_t m_LinkRoot;			// Linklist of all tripwires




//----------------------------------------------------------------------------------------------------------------------------------
// Public Functions:
//----------------------------------------------------------------------------------------------------------------------------------
public:
	virtual void OnEnter( CEntity* pEntity );
	virtual void OnExit( CEntity* pEntity );
	virtual void OnInside( CEntity* pEntity );

	static FINLINE u32 GetTripwireCount( void ) { return m_LinkRoot.nCount; }
	static FINLINE CTripwire *TripwireList_GetHead( void ) { return (CTripwire *)flinklist_GetHead( &m_LinkRoot ); }
	static FINLINE CTripwire *TripwireList_GetTail( void ) { return (CTripwire *)flinklist_GetTail( &m_LinkRoot ); }
	FINLINE CTripwire *TripwireList_GetNext( void ) const { return (CTripwire *)flinklist_GetNext( &m_LinkRoot, this ); }
	FINLINE CTripwire *TripwireList_GetPrev( void ) const { return (CTripwire *)flinklist_GetPrev( &m_LinkRoot, this ); }

	static FINLINE void TripwireList_Init( void ) { flinklist_InitRoot( &m_LinkRoot, FANG_OFFSETOF( CTripwire, m_Link ) ); }
	static void TripwireList_Work( void );
	FINLINE void TripwireList_AddTail( void ) { flinklist_AddTail( &m_LinkRoot, this ); }
	FINLINE void TripwireList_Remove( void ) { flinklist_Remove( &m_LinkRoot, this ); }
	FINLINE BOOL TripwireList_IsMember( void ) { return flinklist_IsLinkInList( &m_LinkRoot, &m_Link ); }


	FCLASS_STACKMEM_ALIGN( CTripwire );
} FCLASS_ALIGN_SUFFIX;



//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CTripwireBuilder
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************
//class CTripwire;


class CTripwireBuilder {
public:

	enum {
		DEFAULT_TRIPWIRE_ENTITY_COUNT = 50,
		NUM_AIZONE_BEHAVIORS = 4
	};


	static void SetDefaults( CEntityBuilder* pBuilder );		// Just in case I have defaults to be set based on entity_type (definitely will)
	static BOOL InterpretTable( CEntityBuilder* pBuilder );
	static BOOL PostInterpretFixup( CEntityBuilder* pBuilder );
	static CTripwire* AllocNewTripWireFromBuilder( CEntityBuilder* pBuilder, CEntity* pOwnerEntity );

	static cchar *m_pszEC_TripwireFilterEntityName;				// NULL=allow entities with any name to trip tripwire. Otherwise, this is the
																// Name of the specific entity that can trip the tripwire.
	static u32 m_nEC_TripwireEntityCount;						// The max number of entities that can be inside the tripwire (0=not a tripwire)
	static u32 m_nEC_TripwireEventFlags;						// See CEntity::TRIPWIRE_EVENTFLAG_* for info
	static u32 m_nEC_TripperFilterFlags;						// See CEntity::TRIPWIRE_FILTER_INCLUDE_* for info
	static u32 m_bEC_AIPatrolZoneType;							// This tripwire is a CPatrolZone type of tripwire
	static u32 m_nEC_TripwireTriggerMode;						// Mode of tripwire triggering (single, multiple)

	//  Data for an AIZone tripwire:
	static cchar* m_pszLookAtObjName;
	static u8 m_uZoneBehavior;
	static u8 m_uZoneTimeParam;

	static cchar *m_pszDoorToOpen;								// Name of door attached to this tripwire (if any)

	static u8 m_auZoneRandBehaviorParamMin[NUM_AIZONE_BEHAVIORS];
	static u8 m_auZoneRandBehaviorParamMax[NUM_AIZONE_BEHAVIORS];
	static u8 m_auZoneNonRandBehaviorParam[NUM_AIZONE_BEHAVIORS];
	static u8 m_auZoneToBehaviorSlotMap[NUM_AIZONE_BEHAVIORS];

	static BOOL m_bEC_DamageBuild;								// TRUE=build damage info for this tripwire
	static BOOL m_bEC_DamageEnable;								// TRUE=tripwire damage is enabled
	static f32 m_fEC_DamageNormIntensity;						// Normalized damage intensity to use for this tripwire

	// Kill variables
	static CTripwire::KillMode_e m_eEC_KillMode;				// TRUE=This tripwire is a kill plane
	static BOOL m_bEC_SpawnDeathEffects;						// TRUE=If an entity gets killed by this killplane/zone, spawn the entites death effects
};





#endif

