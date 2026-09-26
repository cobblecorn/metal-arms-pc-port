//////////////////////////////////////////////////////////////////////////////////////
// Tripwire.cpp - 
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
// 07/09/02 MacKellar   Created.
//////////////////////////////////////////////////////////////////////////////////////
#include "fang.h"
#include "fscriptsystem.h"
#include "Tripwire.h"
#include "fclib.h"
#include "entity.h"
#include "AI\AiPatrolPath.h"
#include "gstring.h"
#include "spawnsys.h"




//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CTripwireBuilder
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

cchar *CTripwireBuilder::m_pszEC_TripwireFilterEntityName;		// NULL=allow entities with any name to trip tripwire. Otherwise, this is the
																// name of the specific entity that can trip the tripwire.

u32 CTripwireBuilder::m_nEC_TripwireEntityCount;				// The max number of entities that can be inside the tripwire (0=not a tripwire)
u32 CTripwireBuilder::m_nEC_TripwireEventFlags;					// See CEntity::TRIPWIRE_EVENTFLAG_* for info
u32 CTripwireBuilder::m_nEC_TripperFilterFlags;					// See CEntity::TRIPWIRE_FILTER_INCLUDE_* for info
u32 CTripwireBuilder::m_bEC_AIPatrolZoneType;					// This tripwire is a CPatrolZone type of tripwire
u32 CTripwireBuilder::m_nEC_TripwireTriggerMode;				// mode of tripwire triggering (single, multiple)

// Data for an AIZone tripwire:
cchar* CTripwireBuilder::m_pszLookAtObjName;
u8 CTripwireBuilder::m_uZoneBehavior;
u8 CTripwireBuilder::m_uZoneTimeParam;
u8 CTripwireBuilder::m_auZoneNonRandBehaviorParam[NUM_AIZONE_BEHAVIORS];
u8 CTripwireBuilder::m_auZoneRandBehaviorParamMin[NUM_AIZONE_BEHAVIORS];
u8 CTripwireBuilder::m_auZoneRandBehaviorParamMax[NUM_AIZONE_BEHAVIORS];
u8 CTripwireBuilder::m_auZoneToBehaviorSlotMap[NUM_AIZONE_BEHAVIORS];

// Name of door attached to this tripwire (if any):
cchar *CTripwireBuilder::m_pszDoorToOpen;

// Damage info (if any):
BOOL CTripwireBuilder::m_bEC_DamageBuild;
BOOL CTripwireBuilder::m_bEC_DamageEnable;
f32 CTripwireBuilder::m_fEC_DamageNormIntensity;

// Killplane variables
CTripwire::KillMode_e CTripwireBuilder::m_eEC_KillMode;
BOOL CTripwireBuilder::m_bEC_SpawnDeathEffects;




void CTripwireBuilder::SetDefaults( CEntityBuilder* pBuilder ) {
	m_pszEC_TripwireFilterEntityName = NULL;
	m_nEC_TripwireEntityCount = 0;
	m_nEC_TripwireEventFlags = CEntity::TRIPWIRE_EVENTFLAG_ENTER_EVENT;
	m_nEC_TripperFilterFlags = CEntity::TRIPWIRE_FILTER_INCLUDE_ALL;
	m_bEC_AIPatrolZoneType = FALSE;
	m_nEC_TripwireTriggerMode = CEntity::TRIPWIRE_TRIGGER_MODE_MULTIPLE;

	//  Data for an AIZone tripwire...
	m_pszLookAtObjName = NULL;
	m_uZoneBehavior = 0;
	m_uZoneTimeParam = 0;
	fang_MemZero( m_auZoneRandBehaviorParamMin, sizeof(m_auZoneRandBehaviorParamMin) );
	fang_MemZero( m_auZoneRandBehaviorParamMax, sizeof(m_auZoneRandBehaviorParamMax) );
	fang_MemZero( m_auZoneNonRandBehaviorParam, sizeof(m_auZoneNonRandBehaviorParam) );
	fang_MemZero( m_auZoneToBehaviorSlotMap, sizeof(m_auZoneToBehaviorSlotMap) );

	m_pszDoorToOpen = NULL;

	// Damage info...
	m_bEC_DamageBuild = FALSE;
	m_bEC_DamageEnable = FALSE;
	m_fEC_DamageNormIntensity = 0.0f;

	// KillMode info...
	m_eEC_KillMode = CTripwire::TRIPWIRE_KILLMODE_NONE;
	m_bEC_SpawnDeathEffects = FALSE;
}


BOOL CTripwireBuilder::InterpretTable( CEntityBuilder* pBuilder ) {
	FGameDataTableHandle_t hCurTable = CEntityParser::m_hTable;
	FGameData_VarType_e nDataType;
	FGameData_VarType_e nVarType;
	cchar *pszString;
	u32 i;
	const void *pData;

	if( !fclib_stricmp( CEntityParser::m_pszTableName, "TripwireEvent" ) ) {

		if( !pBuilder->m_bEC_ClassIsTripwireCapable ) {
			// This class isn't capable of being a tripwire...

			CEntityParser::Error_Prefix();
			DEVPRINTF( "Tripwire command specified on an entity that cannot be a tripwire.\n" );
			CEntityParser::Error_Dashes();

		} else {
			// This class is capable of being a tripwire...

			if( CEntityParser::m_nFieldCount == 1 ) {
				pszString = (cchar *)fgamedata_GetPtrToFieldData( CEntityParser::m_hTable, 0, nDataType );

				if( nDataType == FGAMEDATA_VAR_TYPE_STRING ) {
					if( !fclib_stricmp( pszString, "None" ) ) {
						m_nEC_TripwireEventFlags = CEntity::TRIPWIRE_EVENTFLAG_NONE;
						return TRUE;
					} else if( !fclib_stricmp( pszString, "All" ) ) {
						m_nEC_TripwireEventFlags = CEntity::TRIPWIRE_EVENTFLAG_ALL;
					}
				}
			}

			u32 nTripwireFlags = 0;

			for( i=0; i<CEntityParser::m_nFieldCount; ++i ) {
				pszString = (cchar *)fgamedata_GetPtrToFieldData( CEntityParser::m_hTable, i, nDataType );

				if( nDataType != FGAMEDATA_VAR_TYPE_STRING ) {
					CEntityParser::Error_InvalidParameterType();
					break;
				} else {
					if( !fclib_stricmp( pszString, "Enter" ) ) {
						nTripwireFlags |= CEntity::TRIPWIRE_EVENTFLAG_ENTER_EVENT;

					} else if( !fclib_stricmp( pszString, "Exit" ) ) {
						nTripwireFlags |= CEntity::TRIPWIRE_EVENTFLAG_EXIT_EVENT;

					} else if( !fclib_stricmp( pszString, "Inside" ) ) {
						nTripwireFlags |= CEntity::TRIPWIRE_EVENTFLAG_INSIDE_EVENT;

					} else {
						CEntityParser::Error_InvalidParameterValue();
						break;
					}
				}
			}

			if( i == CEntityParser::m_nFieldCount ) {
				// No error while parsing...

				m_nEC_TripwireEventFlags = nTripwireFlags;
			}
		}

		return TRUE;

	} else if( !fclib_stricmp( CEntityParser::m_pszTableName, "TripwireFilterEntityName" ) ) {

		if( !pBuilder->m_bEC_ClassIsTripwireCapable ) {
			// This class isn't capable of being a tripwire...

			CEntityParser::Error_Prefix();
			DEVPRINTF( "Tripwire command specified on an entity that cannot be a tripwire.\n" );
			CEntityParser::Error_Dashes();

		} else {
			// This class is capable of being a tripwire...

			if( CEntityParser::m_nFieldCount != 1 ) {
				CEntityParser::Error_InvalidParameterCount();
				return TRUE;
			}

			pData = fgamedata_GetPtrToFieldData( CEntityParser::m_hTable, 0, nVarType );

			switch( nVarType ) {
			case FGAMEDATA_VAR_TYPE_STRING:
				pszString = (cchar *)pData;

				if( !fclib_stricmp( pszString, "All" ) ) {
					m_pszEC_TripwireFilterEntityName = NULL;
				} else {
					CEntityParser::Interpret_String( &m_pszEC_TripwireFilterEntityName );
				}

				return TRUE;

			case FGAMEDATA_VAR_TYPE_FLOAT:
				CEntityParser::Error_InvalidParameterType();
				return TRUE;

			default:
				FASSERT_NOW;
				return TRUE;
			}

			return TRUE;
		}

	} else if( !fclib_stricmp( CEntityParser::m_pszTableName, "TripwireWho" ) ) {

		if( !pBuilder->m_bEC_ClassIsTripwireCapable ) {
			// This class isn't capable of being a tripwire...

			CEntityParser::Error_Prefix();
			DEVPRINTF( "Tripwire command specified on an entity that cannot be a tripwire.\n" );
			CEntityParser::Error_Dashes();

		} else {
			// This class is capable of being a tripwire...

			if( CEntityParser::m_nFieldCount == 1 ) {
				pszString = (cchar *)fgamedata_GetPtrToFieldData( CEntityParser::m_hTable, 0, nDataType );

				if( nDataType == FGAMEDATA_VAR_TYPE_STRING ) {
					if( !fclib_stricmp( pszString, "None" ) ) {
						m_nEC_TripperFilterFlags = CEntity::TRIPWIRE_FILTER_INCLUDE_NONE;
						return TRUE;
					} else if( !fclib_stricmp( pszString, "All" ) ) {
						m_nEC_TripperFilterFlags = CEntity::TRIPWIRE_FILTER_INCLUDE_ALL;
						return TRUE;
					}
				}
			}

			u32 nTripwireFlags = 0;

			for( i=0; i<CEntityParser::m_nFieldCount; ++i ) {
				pszString = (cchar *)fgamedata_GetPtrToFieldData( CEntityParser::m_hTable, i, nDataType );

				if( nDataType != FGAMEDATA_VAR_TYPE_STRING ) {
					CEntityParser::Error_InvalidParameterType();
					break;
				} else {
					if( !fclib_stricmp( pszString, "Player" ) ) {
						nTripwireFlags |= CEntity::TRIPWIRE_FILTER_INCLUDE_PLAYER;

					} else if( !fclib_stricmp( pszString, "Enemy" ) ) {
						nTripwireFlags |= CEntity::TRIPWIRE_FILTER_INCLUDE_ENEMY;

					} else if( !fclib_stricmp( pszString, "Friendly" ) ) {
						nTripwireFlags |= CEntity::TRIPWIRE_FILTER_INCLUDE_FRIENDLY;

					} else if( !fclib_stricmp( pszString, "Other" ) ) {
						nTripwireFlags |= CEntity::TRIPWIRE_FILTER_INCLUDE_OTHER;

					} else {
						CEntityParser::Error_InvalidParameterValue();
						break;
					}
				}
			}

			if( i == CEntityParser::m_nFieldCount ) {
				// No error while parsing...

				m_nEC_TripperFilterFlags = nTripwireFlags;
			}
		}

		return TRUE;

	} else if( !fclib_stricmp( CEntityParser::m_pszTableName, "TripwireBuild" ) ) {

		if( CEntityParser::m_nFieldCount != 1 ) {
			CEntityParser::Error_InvalidParameterCount();
		} else {
			pszString = (cchar *)fgamedata_GetPtrToFieldData( CEntityParser::m_hTable, 0, nDataType );

			switch( nDataType ) {
			case FGAMEDATA_VAR_TYPE_STRING:
				if( !fclib_stricmp( pszString, "On" ) ) {
					m_nEC_TripwireEntityCount = DEFAULT_TRIPWIRE_ENTITY_COUNT;
				} else if( !fclib_stricmp( pszString, "Off" ) ) {
					m_nEC_TripwireEntityCount = 0;
				} else {
					CEntityParser::Error_InvalidParameterValue();
				}

				break;

			case FGAMEDATA_VAR_TYPE_FLOAT:
				CEntityParser::Interpret_U32( &m_nEC_TripwireEntityCount );
				break;

			default:
				CEntityParser::Error_InvalidParameterType();
				break;
			}
		}

		return TRUE;

	} else if( !fclib_stricmp( CEntityParser::m_pszTableName, "TripwireIsAIZone" ) ) {

		if( CEntityParser::m_nFieldCount != 1 ) {
			CEntityParser::Error_InvalidParameterCount();
		} else {
			pszString = (cchar *)fgamedata_GetPtrToFieldData( CEntityParser::m_hTable, 0, nDataType );

			switch( nDataType ) {
			case FGAMEDATA_VAR_TYPE_STRING:
				if( !fclib_stricmp( pszString, "true" ) ) {
					m_bEC_AIPatrolZoneType = TRUE;
					return TRUE;
				} else if( !fclib_stricmp( pszString, "false" ) ) {
					m_bEC_AIPatrolZoneType = FALSE;
					return TRUE;
				} else {
					CEntityParser::Error_InvalidParameterValue();
				}

				break;

			default:
				CEntityParser::Error_InvalidParameterType();
				break;
			}
		}

	} else if( !fclib_stricmp( CEntityParser::m_pszTableName, "aizone_behavior" ) ) {

		cchar *pszValue;
		u32 uCurEntry, uNumEntries = CEntityParser::m_nFieldCount;
		if( uNumEntries > NUM_AIZONE_BEHAVIORS ) {
			DEVPRINTF( "CTripwirebuilder::InterpretTable() : Too many entries in 'ZONE_BEHAVIOR' table (max = %d).\n", NUM_AIZONE_BEHAVIORS );	
			uNumEntries = NUM_AIZONE_BEHAVIORS;
		}

		for( uCurEntry = 0; uCurEntry < uNumEntries; ++uCurEntry ) {
			pszValue = (cchar*) fgamedata_GetPtrToFieldData(hCurTable, uCurEntry, nDataType);

			if( !fclib_stricmp(pszValue, "speed") ) {
				m_uZoneBehavior |= CPatrolZone::ZONESPEC_SPEED;
				m_auZoneToBehaviorSlotMap[CPatrolZone::ZONESPEC_SPEED_LOG2] = (u8)uCurEntry+1;

			} else if( !fclib_stricmp(pszValue, "wait") ) {
				m_uZoneBehavior |= CPatrolZone::ZONESPEC_WAIT;
				m_auZoneToBehaviorSlotMap[CPatrolZone::ZONESPEC_WAIT_LOG2] = (u8)uCurEntry+1;

			} else if( !fclib_stricmp(pszValue, "anim") ) {
				m_uZoneBehavior |= CPatrolZone::ZONESPEC_ANIM;
				m_auZoneToBehaviorSlotMap[CPatrolZone::ZONESPEC_ANIM_LOG2] = (u8)uCurEntry+1;

			} else if( !fclib_stricmp(pszValue, "look") ) {
				m_uZoneBehavior |= CPatrolZone::ZONESPEC_LOOK;
				m_auZoneToBehaviorSlotMap[CPatrolZone::ZONESPEC_LOOK_LOG2] = (u8)uCurEntry+1;

			} else {
				DEVPRINTF( "CTripwirebuilder::InterpretTable() : Invalid value '%s' for 'ZONE_BEHAVIOR' table. Use (SPEED, WAIT, BOTTALK, LOOKAT).\n", pszValue );
				return TRUE;
			}
		}

		return TRUE;

	} else if( !fclib_stricmp( CEntityParser::m_pszTableName, "aizone_param1min" ) ) {

		u32 uCurEntry, uNumEntries = CEntityParser::m_nFieldCount;
		if( uNumEntries > NUM_AIZONE_BEHAVIORS ) {
			DEVPRINTF( "CTripwirebuilder::InterpretTable() : Too many entries in 'aizone_param1min' table (max = %d).\n", NUM_AIZONE_BEHAVIORS );	
			uNumEntries = NUM_AIZONE_BEHAVIORS;
		}

		for( uCurEntry = 0; uCurEntry < uNumEntries; ++uCurEntry ) {
			f32* pf32 = (f32*)fgamedata_GetPtrToFieldData( hCurTable, uCurEntry, nDataType );
			if( pf32 ) {
				m_auZoneRandBehaviorParamMin[uCurEntry] = (u8)(*pf32);
			}
		}

		return TRUE;

	} else if( !fclib_stricmp( CEntityParser::m_pszTableName, "aizone_param1max" ) ) {

		u32 uCurEntry, uNumEntries = CEntityParser::m_nFieldCount;
		if( uNumEntries > NUM_AIZONE_BEHAVIORS ) {
			DEVPRINTF( "CTripwirebuilder::InterpretTable() : Too many entries in 'aizone_param1max' table (max = %d).\n", NUM_AIZONE_BEHAVIORS );	
			uNumEntries = NUM_AIZONE_BEHAVIORS;
		}

		for( uCurEntry = 0; uCurEntry < uNumEntries; ++uCurEntry ) {
			f32* pf32 = (f32*)fgamedata_GetPtrToFieldData(hCurTable, uCurEntry, nDataType);
			if( pf32 ) {
				m_auZoneRandBehaviorParamMax[uCurEntry] = (u8)(*pf32);
			}
		}

		return TRUE;

	} else if( !fclib_stricmp( CEntityParser::m_pszTableName, "aizone_param2" ) ) {

		u32 uCurEntry, uNumEntries = CEntityParser::m_nFieldCount;
		if( uNumEntries > NUM_AIZONE_BEHAVIORS ) {
			DEVPRINTF( "CTripwirebuilder::InterpretTable() : Too many entries in 'aizone_param2' table (max = %d).\n", NUM_AIZONE_BEHAVIORS );	
			uNumEntries = NUM_AIZONE_BEHAVIORS;
		}

		for( uCurEntry = 0; uCurEntry < uNumEntries; ++uCurEntry ) {
			f32* pf32 = (f32*)fgamedata_GetPtrToFieldData( hCurTable, uCurEntry, nDataType );
			if( pf32 ) {
				m_auZoneNonRandBehaviorParam[uCurEntry] = (u8)(*pf32);
			}
		}

		return TRUE;

	} else if( !fclib_stricmp( CEntityParser::m_pszTableName, "aizone_paramname" ) ) {

		CEntityParser::Interpret_String( &m_pszLookAtObjName );
		return TRUE;

	} else if( !fclib_stricmp( CEntityParser::m_pszTableName, "aizone_paramtime" ) ) {

		u32 uTmp;
		CEntityParser::Interpret_U32( &uTmp );
		m_uZoneTimeParam = (u8)uTmp;
		return TRUE;

	} else if( !fclib_stricmp( CEntityParser::m_pszTableName, "DoorToOpen" ) ) {

		CEntityParser::Interpret_String( &m_pszDoorToOpen );
		return TRUE;

	} else if( !fclib_stricmp( CEntityParser::m_pszTableName, "TriggerMode" ) ) {

		cchar *pszValue;
		pszValue = (cchar*) fgamedata_GetPtrToFieldData(hCurTable, 0, nDataType);
		if( nDataType == FGAMEDATA_VAR_TYPE_STRING ) {
			if( !fclib_stricmp( pszValue, "once" ) ) {
				m_nEC_TripwireTriggerMode = CEntity::TRIPWIRE_TRIGGER_MODE_ONCE;
			}
		}

		return TRUE;

	} else if( !fclib_stricmp( CEntityParser::m_pszTableName, "TripwireDamageBuild" ) ) {

		CEntityParser::Interpret_Flag( (u32 *)&m_bEC_DamageBuild, 1 );
		return TRUE;

	} else if( !fclib_stricmp( CEntityParser::m_pszTableName, "TripwireDamageEnable" ) ) {

		CEntityParser::Interpret_Flag( (u32 *)&m_bEC_DamageEnable, 1 );
		return TRUE;

	} else if( !fclib_stricmp( CEntityParser::m_pszTableName, "TripwireDamageNormIntensity" ) ) {

		CEntityParser::Interpret_F32( &m_fEC_DamageNormIntensity, 0.0f, 1000000000000.0f, TRUE );
		return TRUE;

	} else if( !fclib_stricmp( CEntityParser::m_pszTableName, "TripwireKillMode" ) ) {
		cchar *pszValue;
		CEntityParser::Interpret_String(&pszValue);

		if(fclib_stricmp(pszValue, "None") == 0)
		{
			m_eEC_KillMode = CTripwire::TRIPWIRE_KILLMODE_NONE;
		}
		else if(fclib_stricmp(pszValue, "Plane") == 0)
		{
			m_eEC_KillMode = CTripwire::TRIPWIRE_KILLMODE_PLANE;
		}
		else if(fclib_stricmp(pszValue, "Volume") == 0)
		{
			m_eEC_KillMode = CTripwire::TRIPWIRE_KILLMODE_VOLUME;
		}
		else
		{
			DEVPRINTF("CTripwireBuilder::InterpretTable() : Invalid value '%s' for 'TripwireKillMode' table.\n", pszValue);
			FASSERT_NOW;
			return(TRUE);
		}
		return(TRUE);

	} else if( !fclib_stricmp( CEntityParser::m_pszTableName, "TripwireSpawnDeathEffects" ) ) {

		CEntityParser::Interpret_Flag( (u32 *)&m_bEC_SpawnDeathEffects, 1 );
		return TRUE;

	}

	return FALSE;
}


BOOL CTripwireBuilder::PostInterpretFixup( CEntityBuilder* pBuilder ) {
	if( m_pszLookAtObjName && !(m_pszLookAtObjName = gstring_Main.AddString( m_pszLookAtObjName)) ) {
		// Couldn't fit that string into the global table...
		goto _XOutOfMemory;
	}

	if( m_pszEC_TripwireFilterEntityName && !(m_pszEC_TripwireFilterEntityName = gstring_Main.AddString( m_pszEC_TripwireFilterEntityName )) ) {
		goto _XOutOfMemory;
	}

	return TRUE;

_XOutOfMemory:
	CEntityParser::Error_OutOfMemory();
	return FALSE;
}


CTripwire* CTripwireBuilder::AllocNewTripWireFromBuilder( CEntityBuilder* pBuilder, CEntity* pOwnerEntity ) {
	CTripwire *pTripwire = NULL;

	if( m_bEC_AIPatrolZoneType )	{
		pTripwire = fnew CPatrolZone;
	} else {
		pTripwire = fnew CTripwire;
	}

	if( pTripwire == NULL ) {
		DEVPRINTF( "CTripwireBuilder::AllocNewTripWireFromBuilder(): Not enough memory to allocate a tripwire.\n" );
		return NULL;
	}

	FResFrame_t ResFrame = fres_GetFrame();

	pTripwire->m_ppContainedTrippersArray = NULL;
	pTripwire->m_pDamageForm = NULL;

	pTripwire->m_pOwnerEntity = pOwnerEntity;
	pTripwire->m_nEventFlags = m_nEC_TripwireEventFlags;
	pTripwire->m_nTripperFilterFlags = m_nEC_TripperFilterFlags;
	pTripwire->m_pszTripwireFilterEntityName = m_pszEC_TripwireFilterEntityName;

	pTripwire->m_BoundingSphere_MS.m_Pos.Zero();
	pTripwire->m_BoundingSphere_MS.m_fRadius = 0.0f;
	pTripwire->m_BoundingSphere_WS.m_Pos = pOwnerEntity->MtxToWorld()->m_vPos;
	pTripwire->m_BoundingSphere_WS.m_fRadius = 0.0f;

	pTripwire->m_nContainedEntityCount = 0;
	pTripwire->m_nMaxContainedEntityCount = m_nEC_TripwireEntityCount;

	pTripwire->m_ppContainedTrippersArray = (CEntity **)fres_AllocAndZero( sizeof(CEntity *) * pTripwire->m_nMaxContainedEntityCount );
	if( pTripwire->m_ppContainedTrippersArray == NULL ) {
		DEVPRINTF( "CTripwireBuilder::AllocNewTripWireFromBuilder(): Not enough memory to allocate tripper array.\n" );
		goto _ExitWithError;
	}

	if( m_bEC_AIPatrolZoneType )	{
		((CPatrolZone*) pTripwire)->InitFromTripwireBuilder();
	}

	// Attach door to tripwire, if any...
	pTripwire->m_pDoorToOpen = NULL;
	if( m_pszDoorToOpen ) {
		CEntity *pEntity = CEntity::Find( m_pszDoorToOpen );

		if( pEntity ) {
			if( pEntity->TypeBits() & ENTITY_BIT_DOOR ) {
				pTripwire->m_pDoorToOpen = (CDoorEntity*) pEntity;
			}
		}
	}

	// Set tripwire trigger mode...
	pTripwire->m_nTripwireTriggerMode = m_nEC_TripwireTriggerMode;

	// Setup damage from tripwire builder...
	if( m_bEC_DamageBuild ) {
		// Build damage info for this tripwire...

		pTripwire->m_pDamageForm = CDamage::BuildNewEmptyDamageForm();
		if( pTripwire->m_pDamageForm == NULL ) {
			DEVPRINTF( "CTripwireBuilder::AllocNewTripWireFromBuilder(): Not enough memory to allocate a CDamageForm.\n" );
			goto _ExitWithError;
		}

		pTripwire->m_bDamageEnabled = m_bEC_DamageEnable;
		pTripwire->m_fDamageNormIntensity = m_fEC_DamageNormIntensity;

		pTripwire->m_pDamageForm->m_nDamageLocale = CDamageForm::DAMAGE_LOCALE_AMBIENT;
		pTripwire->m_pDamageForm->m_nDamageDelivery = CDamageForm::DAMAGE_DELIVERY_ALL_ENTITIES_WITHIN_SPECIFIC_TRIPWIRE;
		pTripwire->m_pDamageForm->m_fNormIntensity = pTripwire->m_fDamageNormIntensity;
		pTripwire->m_pDamageForm->m_Damager.pEntity = pTripwire->m_pOwnerEntity;
		pTripwire->m_pDamageForm->m_Damager.pWeapon = NULL;
		pTripwire->m_pDamageForm->m_Damager.pBot = NULL;
		pTripwire->m_pDamageForm->m_pTripwireEntity = pTripwire->m_pOwnerEntity;
	}

	// Set the KillMode parameters
	pTripwire->m_eKillMode = m_eEC_KillMode;
	pTripwire->m_bKillModeSpawnDeathEffects = m_bEC_SpawnDeathEffects;

	// Success...
	return pTripwire;

	// Error...
_ExitWithError:
	fdelete( pTripwire->m_pDamageForm );
	fdelete( pTripwire->m_ppContainedTrippersArray );
	fdelete( pTripwire );

	fres_ReleaseFrame( ResFrame );

	return NULL;
}




//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CTripwire
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

s32 CTripwire::m_nTripwireEventNumber = -1;
FLinkRoot_t CTripwire::m_LinkRoot;




void CTripwire::OnEnter( CEntity* pTripperEntity ) {
	CFScriptSystem::TriggerEvent( m_nTripwireEventNumber, (u32)CEntity::TRIPWIRE_EVENTTYPE_ENTER, (u32)m_pOwnerEntity, (u32) pTripperEntity );
}


void CTripwire::OnExit( CEntity* pTripperEntity ) {
	CFScriptSystem::TriggerEvent( m_nTripwireEventNumber, (u32)CEntity::TRIPWIRE_EVENTTYPE_EXIT, (u32)m_pOwnerEntity, (u32) pTripperEntity );
}


void CTripwire::OnInside( CEntity* pTripperEntity ) {
	CFScriptSystem::TriggerEvent( m_nTripwireEventNumber, (u32)CEntity::TRIPWIRE_EVENTTYPE_INSIDE, (u32)m_pOwnerEntity, (u32) pTripperEntity );

	if (m_pOwnerEntity && m_pOwnerEntity->HasSpawnSysUse() && pTripperEntity)	{
		CSpawnSys::TripwireInside(m_pOwnerEntity, pTripperEntity);
	}
}


void CTripwire::TripwireList_Work( void )
{
	CTripwire *pTripwire;

	//send out the inside events for all those tripwires that have things inside of them
	for( pTripwire=CTripwire::TripwireList_GetHead(); pTripwire; pTripwire=pTripwire->TripwireList_GetNext() )
	{
		if (pTripwire->m_nEventFlags & CEntity::TRIPWIRE_EVENTFLAG_INSIDE_EVENT )
		{
			for (u32 i = 0; i < pTripwire->m_nContainedEntityCount; i++)
			{
				pTripwire->OnInside(pTripwire->m_ppContainedTrippersArray[i]);
			}
		}
	}
}
