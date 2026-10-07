//////////////////////////////////////////////////////////////////////////////////////
// entity.cpp - Main game entity class.
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

#include "fang.h"
#include "entity.h"
#include "fmath.h"
#include "flinklist.h"
#include "fworld.h"
#include "fworld_coll.h"
#include "fclib.h"
#include "AlarmSys.h"
#include "meshentity.h"
#include "meshtypes.h"
#include "fresload.h"
#include "fgamedata.h"
#include "fstringtable.h"
#include "level.h"
#include "gstring.h"
#include "botgrunt.h"
#include "bottitan.h"
#include "botprobe.h"
#include "botswarmer.h"
#include "site_BotWeapon.h"
#include "epoint.h"
#include "espline.h"
#include "esphere.h"
#include "ebox.h"
#include "edebris.h"
#include "eline.h"
#include "eswitch.h"
#include "door.h"
#include "eboomer.h"
#include "AI\AIBrainMan.h"
#include "ai/aibuilder.h"
#include "fscriptsystem.h"
#include "GoodieProps.h"
#include "BotPred.h"
#include "econsole.h"
#include "eparticle.h"
#include "ZipLine.h"
#include "gamecam.h"
#include "gamepad.h"
#include "player.h"
#if FANG_WINGC
#include "MultiplayerMgr.h"
#include "game.h"
#endif
#include "FCheckPoint.h"
#include "spawnsys.h"
#include "fperf.h"
#include "vehiclerat.h"
#include "vehiclesentinel.h"
#include "protrack.h"
#include "botslosh.h"
#include "eliquidvolume.h"
#include "eliquidmesh.h"
#include "loadingscreen.h"
#include "vehicleloader.h"
#include "boteliteguard.h"
#include "botkrunk.h"
#include "botzom.h"
#include "botjumper.h"
#include "EDetPackDrop.h"
#include "botmozer.h"
#include "botmortar.h"
#include "gamesave.h"
#include "collectable.h"
#include "botminer.h"
#include "botscout.h"
#include "botglitch.h"
#include "botcorrosive.h"
#include "fsound.h"
#include "ejumppad.h"
#include "fwire.h"
#include "BotAAGun.h"
#include "botscientist.h"
#include "botsnarq.h"
#include "botzombieboss.h"
#include "difficulty.h"

#define _WIRE_MESH_NAME				"GO_1cable01"
#define _WIRE_DEBRIS_MESH_NAME		"GO_1cable02"
#define _WIRE_SLICE_SOUND_GROUP		"WireSlice"
#define _WIRE_COLORS_PER_TEXTURE	8
#define _WIRE_TEX_LAYER_ID			1




//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CEntityBuilder
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

static CEntityBuilder _EntityBuilder;
static CAlarmSysBuilder _AlarmSys_Builder;  //entits may cause some structurs to be allocated in the alarm system.
static CSpawnSysBuilder _SpawnSys_Builder;//entits may cause some structurs to be allocated in the spawn system.

void CEntityBuilder::SetDefaults( u64 nEntityTypeBits, u64 nEntityLeafTypeBit, cchar *pszEntityType ) {
	FASSERT( (fmath_CountBits( (u32)(nEntityLeafTypeBit & 0xffffffff) ) + fmath_CountBits( (u32)(nEntityLeafTypeBit >> 32) )) <= 1 );

	m_pszEC_EntityName = NULL;
	m_pszEC_AttachParent = NULL;
	m_pszEC_AttachBone = NULL;
	m_pszEC_ParentEntity = NULL;
	m_nEC_ParentShapeIndex = -1;
	m_nEC_TypeBits = nEntityTypeBits;
	m_nEC_LeafTypeBit = nEntityLeafTypeBit;
	m_nEC_Flags = CEntity::ENTITY_FLAG_INWORLD | CEntity::ENTITY_FLAG_ENABLE_WORK | CEntity::ENTITY_FLAG_ENABLE_AUTOWORK | CEntity::ENTITY_FLAG_CREATED | CEntity::ENTITY_FLAG_TRIPWIRE_ARMED;
	m_EC_Mtx_WS.Identity();
	m_fEC_Scale_WS = 1.0f;
	m_nEC_HealthContainerCount = 1;
	m_fEC_NormHealth = 1.0f;
	m_fEC_ArmorModifier = 0.0f;
	m_bEC_ClassIsTripwireCapable = FALSE;

	m_TripwireBuilder.SetDefaults(this);
	_AlarmSys_Builder.SetDefaults(this);
	_SpawnSys_Builder.SetDefaults(this);
	m_pszEC_DamageProfile = NULL;
	m_pszEC_ArmorProfile = NULL;
	m_pszAIBuilderName = NULL;
	m_pAIBuilder = NULL;

	m_oGoodieBag.Reset();

	m_pGoodieProps = NULL;

	m_pszAmbientTag = NULL;
	m_bAmbientPlay = TRUE;
	m_fAmbientVolume = 1.0f;
	m_fAmbientRadius = 20.0f;

	m_nProjectileReaction = CEntity::PROJECTILE_REACTION_DEFAULT;

	m_bIsCollectable = FALSE;

	m_bAllowVerlet = FALSE;
	m_bVerletEnabled = FALSE;
	m_bVerletFreeze = FALSE;
	m_bVerletFreezeWhenMotionless = FALSE;
	m_bDieWhenCollide = FALSE;
	m_VerletInit.fMass = 500.0f;
	m_VerletInit.fDimX = -1.0f;
	m_VerletInit.fDimY = -1.0f;
	m_VerletInit.fDimZ = -1.0f;
	m_fVerletUnitDampening = 0.02f;
	m_fVerletPrimeSecs = 0.0f;
	m_fVerletGravity = -64.0f;
	m_nVerletSurfaceCount = 0;
	m_ppszVerletSurfaceNames = NULL;

	m_nTackDefCount = 0;
}


// Returns FALSE upon catastrophic error.
// Note that this function may allocate an AIBuilder object!
BOOL CEntityBuilder::InterpretFile( void ) {
	// Set defaults...
	SetDefaults( 0, 0, CEntityParser::m_pszEntityType );

	m_EC_Mtx_WS.m_vRight.Set( CEntityParser::m_pWorldShapeInit->m_Mtx43.m_vRight );
	m_EC_Mtx_WS.m_vUp.Set( CEntityParser::m_pWorldShapeInit->m_Mtx43.m_vUp );
	m_EC_Mtx_WS.m_vFront.Set( CEntityParser::m_pWorldShapeInit->m_Mtx43.m_vFront );
	m_EC_Mtx_WS.m_vPos.Set( CEntityParser::m_pWorldShapeInit->m_Mtx43.m_vPos );

	m_fEC_Scale_WS = m_EC_Mtx_WS.m_vRight.MagSq();
	if( m_fEC_Scale_WS != 1.0f ) {
		CFVec3A ScaleVec;

		m_fEC_Scale_WS = fmath_Sqrt( m_fEC_Scale_WS );

		ScaleVec.Set( fmath_Inv( m_fEC_Scale_WS ) );
		m_EC_Mtx_WS.m_vRight.Mul( ScaleVec );
		m_EC_Mtx_WS.m_vUp.Mul( ScaleVec );
		m_EC_Mtx_WS.m_vFront.Mul( ScaleVec );
	}

	m_pszAIBuilderName = CEntityParser::m_pszAIBuilderName;

	if( (m_pszAIBuilderName == NULL) && (m_nEC_TypeBits & ENTITY_BIT_BOT) ) {
		// Bots without an "AI=" builder name get a default builder...
		m_pszAIBuilderName = CEntityParser::m_pszAIBuilderName = "Default";
	}

	if( m_pszAIBuilderName ) {
		// An AI builder has been specified...
		m_pAIBuilder = CAIBuilder::Request();

		if( m_pAIBuilder ) {
			m_pAIBuilder->SetDefaults( this );
		} else {
			m_pszAIBuilderName = CEntityParser::m_pszAIBuilderName = NULL;
		}
	}

	// Find first table...
	if( CEntityParser::SetFirstTable() ) {
		// Found a table...

		do {
			// Interpret this table...

			if( !InterpretTable() ) {
				// Catastrophic error...
				goto _ExitWithError;
			}

			// Find the next table...
		} while( CEntityParser::SetNextTable() );
	}

	// Fixup any loose ends and exit...
	if( !PostInterpretFixup() ) {
		goto _ExitWithError;
	}

	// Success...

	return TRUE;

	// Error...
_ExitWithError:
	if( m_pAIBuilder ) {
		CAIBuilder::Recycle( m_pAIBuilder );
		m_pAIBuilder = NULL;
	}

	return FALSE;
}


// Returns FALSE upon catastrophic error.
BOOL CEntityBuilder::PostInterpretFixup( void ) {
	FGameData_VarType_e nVarType;
	cchar *pszString;
	u32 i;

	loadingscreen_Update();
	if( m_pszEC_AttachBone && !(m_pszEC_AttachBone = gstring_Main.AddString( m_pszEC_AttachBone )) ) {
		goto _OutOfMemory;
	}

	m_nEC_ParentShapeIndex = CEntityParser::m_pWorldShapeInit->m_nParentShapeIndex;

	if( !_AlarmSys_Builder.PostInterpretFixup(this) ) {
		goto _OutOfMemory;
	}

	if( !_SpawnSys_Builder.PostInterpretFixup(this) ) {
		goto _OutOfMemory;
	}

	if( !CTripwireBuilder::PostInterpretFixup(this) ) {
		goto _OutOfMemory;
	}

	if( m_pAIBuilder ) {
		if( !m_pAIBuilder->PostInterpretFixup(this) ) {
			goto _OutOfMemory;
		}
	}

	if( m_ppszVerletSurfaceNames ) {
		// One or more Verlet Collision Surfaces have been specified...

		CEntityParser::SetTable( (FGameDataTableHandle_t)m_ppszVerletSurfaceNames );
		FASSERT( CEntityParser::m_nFieldCount >= 1 );

		pszString = (cchar *)fgamedata_GetPtrToFieldData( CEntityParser::m_hTable, 0, nVarType );
		FASSERT( nVarType == FGAMEDATA_VAR_TYPE_STRING );

		if( !fclib_stricmp( pszString, "None" ) ) {
			if( CEntityParser::m_nFieldCount != 1 ) {
				CEntityParser::Error_InvalidParameterCount();
			}

			m_ppszVerletSurfaceNames = NULL;
			m_nVerletSurfaceCount = 0;
		} else {
			if( !fclib_stricmp( CEntityParser::m_pszTableName, "PhysCollSurf" ) ) {
				// List of animation names provided...
				if( !CEntityParser::Interpret_StringArray_BuildInFMem( &m_ppszVerletSurfaceNames ) ) {
					// Out of memory...
					goto _OutOfMemory;
				}
				m_nVerletSurfaceCount = CEntityParser::m_nFieldCount;

				//now, insert these strings into the main string table for later retrival...
				for( i=0; i<CEntityParser::m_nFieldCount; i++ ) {
					m_ppszVerletSurfaceNames[ i ] = gstring_Main.AddString( m_ppszVerletSurfaceNames[ i ] );
					if( !m_ppszVerletSurfaceNames[ i ] ) {
						// Out of memory...
						goto _OutOfMemory;
					}
				}
			} else {
				FASSERT_NOW;
			}
		}
	}

	loadingscreen_Update();
	return TRUE;

_OutOfMemory:
	CEntityParser::Error_OutOfMemory();
	return FALSE;
}


// Returns FALSE upon catastrophic error.
BOOL CEntityBuilder::InterpretTable( void ) {
	FGameData_VarType_e nDataType;
	cchar *pszString, *apszPhysString[3];
	BOOL bValue;

	if( !fclib_stricmp( CEntityParser::m_pszTableName, "Name" ) ) {

		m_pszEC_EntityName = CEntityParser::m_pszEntityName;
		return TRUE;

	} else if( !fclib_stricmp( CEntityParser::m_pszTableName, "Type" ) ) {

		// We've already handled this command...
		return TRUE;

	} else if( !fclib_stricmp( CEntityParser::m_pszTableName, "AttachBone" ) ) {

		CEntityParser::Interpret_String( &m_pszEC_AttachBone );
		return TRUE;

	} else if( !fclib_stricmp( CEntityParser::m_pszTableName, "InWorld" ) ) {

		CEntityParser::Interpret_Flag( &m_nEC_Flags, CEntity::ENTITY_FLAG_INWORLD );
		return TRUE;

	} else if( !fclib_stricmp( CEntityParser::m_pszTableName, "SetDamageable" ) ) {

		CEntityParser::Interpret_Flag( &m_nEC_Flags, CEntity::ENTITY_FLAG_INVINCIBLE, TRUE );
		return TRUE;

	} else if( !fclib_stricmp( CEntityParser::m_pszTableName, "HealthContainer" ) ) {

		CEntityParser::Interpret_U32( &m_nEC_HealthContainerCount, 1, 0xffffffff, TRUE );
		return TRUE;

	} else if( !fclib_stricmp( CEntityParser::m_pszTableName, "Health" ) ) {

		pszString = (cchar *)fgamedata_GetPtrToFieldData( CEntityParser::m_hTable, 0, nDataType );

		switch( nDataType ) {
		case FGAMEDATA_VAR_TYPE_STRING:
			if( CEntityParser::m_nFieldCount != 1 ) {
				CEntityParser::Error_InvalidParameterCount();
			} else {
				if( !fclib_stricmp( pszString, "Min" ) ) {
					m_fEC_NormHealth = 0.0000000001f;
				} else if( !fclib_stricmp( pszString, "Max" ) ) {
					m_fEC_NormHealth = 100000000000000000000.0f;
				} else {
					CEntityParser::Error_InvalidParameterValue();
				}
			}

			break;

		case FGAMEDATA_VAR_TYPE_FLOAT:
			CEntityParser::Interpret_F32( &m_fEC_NormHealth, 0.0f );
			break;

		default:
			CEntityParser::Error_InvalidParameterType();
		}

		return TRUE;

	} else if( !fclib_stricmp( CEntityParser::m_pszTableName, "DamageProfile" ) ) {

		CEntityParser::Interpret_String( &m_pszEC_DamageProfile );
		return TRUE;

	} else if( !fclib_stricmp( CEntityParser::m_pszTableName, "ArmorModifier" ) ) {

		CEntityParser::Interpret_F32( &m_fEC_ArmorModifier, -1.0f, 1.0f, TRUE );
		return TRUE;

	} else if( !fclib_stricmp( CEntityParser::m_pszTableName, "ArmorProfile" ) ) {

		CEntityParser::Interpret_String( &m_pszEC_ArmorProfile );
		return TRUE;

	} else if( !fclib_stricmp( CEntityParser::m_pszTableName, "Attach" ) ) {

		CEntityParser::Interpret_String( &m_pszEC_ParentEntity );
		return TRUE;

	} else if( !fclib_stricmp( CEntityParser::m_pszTableName, "Goodiebag" ) ) {

		// They are specifying properties of the goodiebag.
		// Get the probability of spawning anything.

		FGameData_VarType_e eType;

		f32 fProb = *(const f32 *)fgamedata_GetPtrToFieldData( CEntityParser::m_hTable, 0, eType );
		if( eType != FGAMEDATA_VAR_TYPE_FLOAT ) {
			CEntityParser::Error_Prefix();
			DEVPRINTF( "Error in probability field of 'goodiebag' command.  It's a string!.\n" );
			CEntityParser::Error_Dashes();
			return TRUE;
		}
		m_oGoodieBag.SetProb( fProb );

		if( fgamedata_GetNumFields( CEntityParser::m_hTable ) >= 2 ) {
			cchar *pszBrainType;
			pszBrainType = (cchar *)fgamedata_GetPtrToFieldData( CEntityParser::m_hTable, 1, eType );
			if( eType != FGAMEDATA_VAR_TYPE_STRING ) {
				CEntityParser::Error_Prefix();
				DEVPRINTF( "Error in brain field of 'goodiebag' command.  It's not a string!.\n" );
				CEntityParser::Error_Dashes();
				return TRUE;
			}
			if( fclib_stricmp(pszBrainType, "any") == 0 ) {
				m_oGoodieBag.m_eBrain = GOODIEBAGBRAIN_ANY;
			} else if( fclib_stricmp(pszBrainType, "one") == 0 ) {
				m_oGoodieBag.m_eBrain = GOODIEBAGBRAIN_ONE;
			} else {
				CEntityParser::Error_Prefix();
				DEVPRINTF( "Error in brain field of 'goodiebag' command.  Unknown brain type '%s'.\n", pszBrainType );
				CEntityParser::Error_Dashes();
				return TRUE;
			}
		}

		return TRUE;

	} else if( !fclib_strnicmp( CEntityParser::m_pszTableName, "Goodie", fclib_strlen( "Goodie")) ) {

		// They are specifying a specific goodie in the bag.
		FGameData_VarType_e eType;

		const u32 uGoodieFieldCount = fgamedata_GetNumFields( CEntityParser::m_hTable );
#if FANG_WINGC
		const u32 uMaxGoodieFields = 6;
#else
		const u32 uMaxGoodieFields = 5;
#endif
		if( uGoodieFieldCount < 4 || uGoodieFieldCount > uMaxGoodieFields ) {
			CEntityParser::Error_Prefix();
			DEVPRINTF( "Error in 'goodie' table. Need 4 to %u params.\n", uMaxGoodieFields );
			CEntityParser::Error_Dashes();
			return TRUE;
		}

		/////////////////////////////////////////////////////////////
		// Get the goodie set up properly.
		cchar *pszGoodieType;
		pszGoodieType = (cchar *)fgamedata_GetPtrToFieldData( CEntityParser::m_hTable, 0, eType );
		if( eType != FGAMEDATA_VAR_TYPE_STRING ) {
			CEntityParser::Error_Prefix();
			DEVPRINTF( "Error in type field of 'goodie' command. It's not a string!.\n" );
			CEntityParser::Error_Dashes();
			return TRUE;
		}

		// Check what type of goodie they requested.
		GoodieType_e eGoodieType = CGoodieInst::GetGoodieTypeFromString( pszGoodieType );
		if( eGoodieType == COLLECTABLE_UNKNOWN ) {
			CEntityParser::Error_Prefix();
			DEVPRINTF( "Error in type field of 'goodie' command, unrecognized type '%s'.\n", pszGoodieType );
			CEntityParser::Error_Dashes();
			return TRUE;
		}
		//
		/////////////////////////////////////////////////////////////

		/////////////////////////////////////////////////////////////
		// Get the quantities.
		u32 uQuantity1 = (u32)(*(const f32 *)(fgamedata_GetPtrToFieldData( CEntityParser::m_hTable, 1, eType )));
		if( eType != FGAMEDATA_VAR_TYPE_FLOAT ) {
			CEntityParser::Error_Prefix();
			DEVPRINTF( "Error in quantity1 field of 'goodie' command. It's a string!.\n" );
			CEntityParser::Error_Dashes();
			return TRUE;
		}

		u32 uQuantity2 = (u32)(*(const f32 *)(fgamedata_GetPtrToFieldData( CEntityParser::m_hTable, 2, eType )));
		if( eType != FGAMEDATA_VAR_TYPE_FLOAT ) {
			CEntityParser::Error_Prefix();
			DEVPRINTF( "Error in quantity2 field of 'goodie' command. It's a string!.\n" );
			CEntityParser::Error_Dashes();
			return TRUE;
		}
		//
		/////////////////////////////////////////////////////////////

		/////////////////////////////////////////////////////////////
		// Get the probability of spawning anything.
		f32 fProb = *(const f32 *)fgamedata_GetPtrToFieldData( CEntityParser::m_hTable, 3, eType );
		if( eType  != FGAMEDATA_VAR_TYPE_FLOAT ) {
			CEntityParser::Error_Prefix();
			DEVPRINTF( "Error in probability field of 'goodie' command. It's a string!.\n" );
			CEntityParser::Error_Dashes();
			return TRUE;
		}
		//
		/////////////////////////////////////////////////////////////

		/////////////////////////////////////////////////////////////
		// Get the angular velocity type set up.
		GoodieBagAngVelType_e eAngVelType = GOODIEBAGANGVELTYPE_NONE;
		if( uGoodieFieldCount >= 5 ) {
			cchar *pszAngVelType;
			pszAngVelType = (cchar *)fgamedata_GetPtrToFieldData( CEntityParser::m_hTable, 4, eType );
			if( eType != FGAMEDATA_VAR_TYPE_STRING ) {
				CEntityParser::Error_Prefix();
				DEVPRINTF( "Error in angular velocity field of 'goodie' command. It's not a string!.\n" );
				CEntityParser::Error_Dashes();
				return TRUE;
			}
			if( fclib_stricmp( pszAngVelType, "none" ) == 0 ) {
				eAngVelType = GOODIEBAGANGVELTYPE_NONE;
			} else if( fclib_stricmp( pszAngVelType, "random" ) == 0 ) {
				eAngVelType = GOODIEBAGANGVELTYPE_RANDOM;
#if FANG_WINGC
			} else if( uGoodieFieldCount == 6 && !fclib_stricmp( pszAngVelType, "X" ) ) {
				// Retail's six-field loot entry uses X as the unused spin placeholder.
				eAngVelType = GOODIEBAGANGVELTYPE_NONE;
#endif
			} else {
				CEntityParser::Error_Prefix();
				DEVPRINTF( "Error in angular velocity field of 'goodie' command. Unknown type '%s'.\n", pszAngVelType );
				CEntityParser::Error_Dashes();
				return TRUE;
			}
		}
		//
		/////////////////////////////////////////////////////////////

#if FANG_WINGC
		if( uGoodieFieldCount == 6 ) {
			// Retail prison2's required chip is authored as: chip 1 1 1 X safe.
			// The current collectable system already keeps chips out of ordinary loot reuse.
			// Accept this known retail suffix without discarding the entire chip entry.
			cchar *pszSafety = (cchar *)fgamedata_GetPtrToFieldData( CEntityParser::m_hTable, 5, eType );
			if( eType != FGAMEDATA_VAR_TYPE_STRING || fclib_stricmp( pszSafety, "safe" ) ) {
				CEntityParser::Error_Prefix();
				DEVPRINTF( "Error in safety field of 'goodie' command. Expected 'safe'.\n" );
				CEntityParser::Error_Dashes();
				return TRUE;
			}
		}
#endif

		// Let's finally add it to the bag.
		if( !m_oGoodieBag.AddGoodie( eGoodieType, uQuantity1, uQuantity2, fProb, eAngVelType ) ) {
			CEntityParser::Error_Prefix();
			DEVPRINTF( "Error adding '%s' goodie.  Probably the goodie bag was already full.\n", pszGoodieType );
			CEntityParser::Error_Dashes();
			return TRUE;
		}

		CCollectable::NotifyCollectableUsedInWorld( pszGoodieType );

		return TRUE;

	} else if( !fclib_stricmp( CEntityParser::m_pszTableName, "Collectable" ) ) {

		if( CEntityParser::m_nFieldCount != 1 ) {
			CEntityParser::Error_InvalidParameterCount();
		} else {
			pszString = (cchar *)fgamedata_GetPtrToFieldData( CEntityParser::m_hTable, 0, nDataType );

			switch( nDataType ) {
			case FGAMEDATA_VAR_TYPE_STRING:
				if( !fclib_stricmp( pszString, "true" ) ) {
					m_bIsCollectable = TRUE;
					m_TripwireBuilder.m_nEC_TripwireEntityCount = CTripwireBuilder::DEFAULT_TRIPWIRE_ENTITY_COUNT;	//findfix: what is going on here?
					FMATH_SETBITMASK( m_nEC_Flags, CEntity::ENTITY_FLAG_TRIPWIRE_ARMED );
					FMATH_SETBITMASK( m_nEC_Flags, CEntity::ENTITY_FLAG_COLLECTABLE );
				} else if( !fclib_stricmp( pszString, "false" ) ) {
					m_bIsCollectable = FALSE;
					m_TripwireBuilder.m_nEC_TripwireEntityCount = 0;							    //findfix: what is going on here?
					FMATH_CLEARBITMASK( m_nEC_Flags, CEntity::ENTITY_FLAG_TRIPWIRE_ARMED );
					FMATH_CLEARBITMASK( m_nEC_Flags, CEntity::ENTITY_FLAG_COLLECTABLE );
				} else {
					CEntityParser::Error_InvalidParameterValue();
				}

				break;
			}
		}

		return TRUE;

	} else if( !fclib_stricmp( CEntityParser::m_pszTableName, "Tripper" ) ) {
		CEntityParser::Interpret_Flag( &m_nEC_Flags, CEntity::ENTITY_FLAG_TRIPPER );

		return TRUE;
	} else if( !fclib_stricmp( CEntityParser::m_pszTableName, "TripwireArm" ) ) {
		CEntityParser::Interpret_Flag( &m_nEC_Flags, CEntity::ENTITY_FLAG_TRIPWIRE_ARMED );
		
		return TRUE;
	} else if( !fclib_stricmp( CEntityParser::m_pszTableName, "Targetable" ) ) {
		CEntityParser::Interpret_Flag( &m_nEC_Flags, CEntity::ENTITY_FLAG_TARGETABLE );

		return TRUE;
	} else if( !fclib_stricmp( CEntityParser::m_pszTableName, "sound_ambient_tag" ) ) {
		CEntityParser::Interpret_String( &m_pszAmbientTag );

		return TRUE;
	} else if( !fclib_stricmp( CEntityParser::m_pszTableName, "sound_ambient_volume" ) ) {
		CEntityParser::Interpret_F32( &m_fAmbientVolume, 0.0f, 1.0f, TRUE );

		return TRUE;
	} else if( !fclib_stricmp( CEntityParser::m_pszTableName, "sound_ambient_radius" ) ) {
		CEntityParser::Interpret_F32( &m_fAmbientRadius );

		return TRUE;
	} else if( !fclib_stricmp( CEntityParser::m_pszTableName, "sound_ambient_off" ) ) {
		CEntityParser::Interpret_BOOL( &bValue );
		m_bAmbientPlay = !bValue;
		return TRUE;

	} else if( !fclib_stricmp( CEntityParser::m_pszTableName, "ProjReact" ) ) {
		// ProjReact Default/Ricochet/DeathToAll/HurtMe

		if( CEntityParser::m_nFieldCount != 1 ) {
			CEntityParser::Error_InvalidParameterCount();
		} else {
			cchar *pszDefaultString;
			if( CEntityParser::Interpret_String( &pszDefaultString ) ) {
				if( !fclib_stricmp( pszDefaultString, "Default" ) ) {
					m_nProjectileReaction = CEntity::PROJECTILE_REACTION_DEFAULT;
				} else if( !fclib_stricmp( pszDefaultString, "Ricochet" ) ) {
					m_nProjectileReaction = CEntity::PROJECTILE_REACTION_RICOCHET;
				} else if( !fclib_stricmp( pszDefaultString, "DeathToAll" ) ) {
					m_nProjectileReaction = CEntity::PROJECTILE_REACTION_DEATH_TO_ALL;
				} else if( !fclib_stricmp( pszDefaultString, "Sticky" ) ) {
					m_nProjectileReaction = CEntity::PROJECTILE_REACTION_STICKY;
				} else if( !fclib_stricmp( pszDefaultString, "HurtMe" ) ) {
					m_nProjectileReaction = CEntity::PROJECTILE_REACTION_HURT_ME;
				} else if( !fclib_stricmp( pszDefaultString, "Shield" ) ) {
					m_nProjectileReaction = CEntity::PROJECTILE_REACTION_SHIELD;
				} else {
					CEntityParser::Error_InvalidParameterValue();
				}
			} else {
				CEntityParser::Error_InvalidParameterType();
			}
		}

		return TRUE;

	} else if( !fclib_stricmp( CEntityParser::m_pszTableName, "Physics" ) ) {
		// Physics on/off

		if( !m_bAllowVerlet ) {
			CEntityParser::Error_Prefix();
			DEVPRINTF( "Physics cannot be used on this type of object.\n" );
			CEntityParser::Error_Dashes();
		} else {
			CEntityParser::Interpret_BOOL( &bValue );
			m_bVerletEnabled = bValue;
		}

		return TRUE;

	} else if( !fclib_stricmp( CEntityParser::m_pszTableName, "PhysFreeze" ) ) {
		// PhysFreeze on/off

		if( !m_bAllowVerlet ) {
			CEntityParser::Error_Prefix();
			DEVPRINTF( "Physics cannot be used on this type of object.\n" );
			CEntityParser::Error_Dashes();
		} else {
			CEntityParser::Interpret_BOOL( &bValue );
			m_bVerletFreeze = bValue;
		}

		return TRUE;

	} else if( !fclib_stricmp( CEntityParser::m_pszTableName, "PhysFreezeWhenStill" ) ) {
		// PhysFreezeWhenStill on/off

		if( !m_bAllowVerlet ) {
			CEntityParser::Error_Prefix();
			DEVPRINTF( "Physics cannot be used on this type of object.\n" );
			CEntityParser::Error_Dashes();
		} else {
			CEntityParser::Interpret_BOOL( &bValue );
			m_bVerletFreezeWhenMotionless = bValue;
		}

		return TRUE;

	} else if( !fclib_stricmp( CEntityParser::m_pszTableName, "PhysDieWhenCollide" ) ) {
		// PhysDieWhenCollide on/off

		if( !m_bAllowVerlet ) {
			CEntityParser::Error_Prefix();
			DEVPRINTF( "Physics cannot be used on this type of object.\n" );
			CEntityParser::Error_Dashes();
		} else {
			CEntityParser::Interpret_BOOL( &bValue );
			m_bDieWhenCollide = bValue;
		}

		return TRUE;

	} else if( !fclib_stricmp( CEntityParser::m_pszTableName, "PhysMass" ) ) {
		// PhysMass mass

		if( !m_bAllowVerlet ) {
			CEntityParser::Error_Prefix();
			DEVPRINTF( "Physics cannot be used on this type of object.\n" );
			CEntityParser::Error_Dashes();
		} else {
			CEntityParser::Interpret_F32( &m_VerletInit.fMass, 0.1f, 100000.0f, TRUE );
		}

		return TRUE;

	} else if( !fclib_stricmp( CEntityParser::m_pszTableName, "PhysDampening" ) ) {
		// PhysDampening dampen

		if( !m_bAllowVerlet ) {
			CEntityParser::Error_Prefix();
			DEVPRINTF( "Physics cannot be used on this type of object.\n" );
			CEntityParser::Error_Dashes();
		} else {
			CEntityParser::Interpret_F32( &m_fVerletUnitDampening, 0.0f, 1.0f, TRUE );
		}

		return TRUE;

	} else if( !fclib_stricmp( CEntityParser::m_pszTableName, "PhysPrimeSecs" ) ) {
		// PhysPrimeSecs secs

		if( !m_bAllowVerlet ) {
			CEntityParser::Error_Prefix();
			DEVPRINTF( "Physics cannot be used on this type of object.\n" );
			CEntityParser::Error_Dashes();
		} else {
			CEntityParser::Interpret_F32( &m_fVerletPrimeSecs, 0.0f, 60.0f, TRUE );
		}

		return TRUE;

	} else if( !fclib_stricmp( CEntityParser::m_pszTableName, "PhysGravity" ) ) {
		// PhysGravity gravity

		if( !m_bAllowVerlet ) {
			CEntityParser::Error_Prefix();
			DEVPRINTF( "Physics cannot be used on this type of object.\n" );
			CEntityParser::Error_Dashes();
		} else {
			CEntityParser::Interpret_F32( &m_fVerletGravity, -1000.0f, 1000.0f, TRUE );
		}

		return TRUE;

	} else if( !fclib_stricmp( CEntityParser::m_pszTableName, "PhysDim" ) ) {
		// PhysDim dimx, dimy, dimz
		// PhysDim FromMesh

		if( !m_bAllowVerlet ) {
			CEntityParser::Error_Prefix();
			DEVPRINTF( "Physics cannot be used on this type of object.\n" );
			CEntityParser::Error_Dashes();
		} else {
			if( (CEntityParser::m_nFieldCount != 3) && (CEntityParser::m_nFieldCount != 1) ) {
				CEntityParser::Error_InvalidParameterCount();
			} else {
				if( CEntityParser::m_nFieldCount == 1 ) {
					cchar *pszDefaultString;
					if( CEntityParser::Interpret_String( &pszDefaultString ) ) {
						if( fclib_stricmp( pszDefaultString, "FromMesh" ) ) {
							CEntityParser::Error_InvalidParameterValue();
						} else {
							// Flag to use default from mesh...
							m_VerletInit.fDimX = m_VerletInit.fDimY = m_VerletInit.fDimZ = -1.0f;
						}
					}
				} else {
					f32 afDim[3];

					if( CEntityParser::Interpret_F32Array( afDim, 3, 1.0f, 100000.0f, TRUE ) ) {
						m_VerletInit.fDimX = afDim[0];
						m_VerletInit.fDimY = afDim[1];
						m_VerletInit.fDimZ = afDim[2];
					}
				}
			}
		}

		return TRUE;

	} else if( !fclib_stricmp( CEntityParser::m_pszTableName, "PhysAnchorEnable" ) ) {
		// PhysAnchorEnable tackname, on/off

		if( !m_bAllowVerlet ) {
			CEntityParser::Error_Prefix();
			DEVPRINTF( "Physics cannot be used on this type of object.\n" );
			CEntityParser::Error_Dashes();
		} else {
			if( CEntityParser::m_nFieldCount != 2 ) {
				CEntityParser::Error_InvalidParameterCount();
			} else {
				if( CEntityParser::Interpret_String( &apszPhysString[0], FALSE ) ) {
					BOOL bAnchorEnabled;
					CEntityParser::Interpret_BOOL( &bAnchorEnabled, FALSE, 1 );

					VerletTackDef_t *pTackDef = FindTackDef( apszPhysString[0], bAnchorEnabled );
					if( pTackDef ) {
						pTackDef->bAnchorEnabled = bAnchorEnabled;
					}
				}
			}
		}

		return TRUE;

	} else if( !fclib_stricmp( CEntityParser::m_pszTableName, "PhysAnchorNone" ) ) {
		// PhysAnchorNone tackname

		if( !m_bAllowVerlet ) {
			CEntityParser::Error_Prefix();
			DEVPRINTF( "Physics cannot be used on this type of object.\n" );
			CEntityParser::Error_Dashes();
		} else {
			if( CEntityParser::m_nFieldCount != 1 ) {
				CEntityParser::Error_InvalidParameterCount();
			} else {
				if( CEntityParser::Interpret_String( &apszPhysString[0], FALSE ) ) {
					VerletTackDef_t *pTackDef = FindTackDef( apszPhysString[0], FALSE );
					if( pTackDef ) {
						pTackDef->nAnchor = VERLET_ANCHOR_NONE;
					}
				}
			}
		}

		return TRUE;

	} else if( !fclib_stricmp( CEntityParser::m_pszTableName, "PhysAnchorToInitialPos" ) ) {
		// PhysAnchorToInitialPos tackname

		if( !m_bAllowVerlet ) {
			CEntityParser::Error_Prefix();
			DEVPRINTF( "Physics cannot be used on this type of object.\n" );
			CEntityParser::Error_Dashes();
		} else {
			if( CEntityParser::m_nFieldCount != 1 ) {
				CEntityParser::Error_InvalidParameterCount();
			} else {
				if( CEntityParser::Interpret_String( &apszPhysString[0], FALSE ) ) {
					VerletTackDef_t *pTackDef = FindTackDef( apszPhysString[0], TRUE );
					if( pTackDef ) {
						pTackDef->nAnchor = VERLET_ANCHOR_ORIG_TACK_POS;
					}
				}
			}
		}

		return TRUE;

	} else if( !fclib_stricmp( CEntityParser::m_pszTableName, "PhysAnchorToAnotherTack" ) ) {
		// PhysAnchorToAnotherTack tackname, anchorentityname, anchortackname

		if( !m_bAllowVerlet ) {
			CEntityParser::Error_Prefix();
			DEVPRINTF( "Physics cannot be used on this type of object.\n" );
			CEntityParser::Error_Dashes();
		} else {
			if( CEntityParser::m_nFieldCount != 3 ) {
				CEntityParser::Error_InvalidParameterCount();
			} else {
				if( CEntityParser::Interpret_StringArray( apszPhysString, 3 ) ) {
					VerletTackDef_t *pTackDef = FindTackDef( apszPhysString[0], TRUE );
					if( pTackDef ) {
						pTackDef->nAnchor = VERLET_ANCHOR_SPECIFIED_TACK;
						pTackDef->pszAnchorAttachToEntity = apszPhysString[1];
						pTackDef->pszAnchorAttachToTack = apszPhysString[2];
					}
				}
			}
		}

		return TRUE;

	} else if( !fclib_stricmp( CEntityParser::m_pszTableName, "PhysAnchorToStaticEntity" ) ) {
		// PhysAnchorToStaticEntity tackname, anchorentityname

		if( !m_bAllowVerlet ) {
			CEntityParser::Error_Prefix();
			DEVPRINTF( "Physics cannot be used on this type of object.\n" );
			CEntityParser::Error_Dashes();
		} else {
			if( CEntityParser::m_nFieldCount != 2 ) {
				CEntityParser::Error_InvalidParameterCount();
			} else {
				if( CEntityParser::Interpret_StringArray( apszPhysString, 2 ) ) {
					VerletTackDef_t *pTackDef = FindTackDef( apszPhysString[0], TRUE );
					if( pTackDef ) {
						pTackDef->nAnchor = VERLET_ANCHOR_ENTITY_ORIGIN_STATIC;
						pTackDef->pszAnchorAttachToEntity = apszPhysString[1];
					}
				}
			}
		}

		return TRUE;

	} else if( !fclib_stricmp( CEntityParser::m_pszTableName, "PhysAnchorToDynamicEntity" ) ) {
		// PhysAnchorToDynamicEntity tackname, anchorentityname

		if( !m_bAllowVerlet ) {
			CEntityParser::Error_Prefix();
			DEVPRINTF( "Physics cannot be used on this type of object.\n" );
			CEntityParser::Error_Dashes();
		} else {
			if( CEntityParser::m_nFieldCount != 2 ) {
				CEntityParser::Error_InvalidParameterCount();
			} else {
				if( CEntityParser::Interpret_StringArray( apszPhysString, 2 ) ) {
					VerletTackDef_t *pTackDef = FindTackDef( apszPhysString[0], TRUE );
					if( pTackDef ) {
						pTackDef->nAnchor = VERLET_ANCHOR_ENTITY_ORIGIN_DYNAMIC;
						pTackDef->pszAnchorAttachToEntity = apszPhysString[1];
					}
				}
			}
		}

		return TRUE;

	} else if( !fclib_stricmp( CEntityParser::m_pszTableName, "PhysAnchorRange" ) ) {
		// PhysAnchorRange tackname, mindist, maxdist

		if( !m_bAllowVerlet ) {
			CEntityParser::Error_Prefix();
			DEVPRINTF( "Physics cannot be used on this type of object.\n" );
			CEntityParser::Error_Dashes();
		} else {
			if( CEntityParser::m_nFieldCount != 3 ) {
				CEntityParser::Error_InvalidParameterCount();
			} else {
				if( CEntityParser::Interpret_String( &apszPhysString[0], FALSE ) ) {
					VerletTackDef_t *pTackDef = FindTackDef( apszPhysString[0], TRUE );
					if( pTackDef ) {
						f32 afRange[2];
						if( CEntityParser::Interpret_F32Array( afRange, 2, 0.0f, 100000.0f, TRUE, 1 ) ) {
							pTackDef->fMinConstraintDist = afRange[0];
							pTackDef->fMaxConstraintDist = afRange[1];
						}
					}
				}
			}
		}

		return TRUE;

	} else if( !fclib_stricmp( CEntityParser::m_pszTableName, "PhysAnchorDraw" ) ) {
		// PhysAnchorDraw tackname, wire_style_code

		if( !m_bAllowVerlet ) {
			CEntityParser::Error_Prefix();
			DEVPRINTF( "Physics cannot be used on this type of object.\n" );
			CEntityParser::Error_Dashes();
		} else {
			if( CEntityParser::m_nFieldCount != 2 ) {
				CEntityParser::Error_InvalidParameterCount();
			} else {
				if( CEntityParser::Interpret_String( &apszPhysString[0], FALSE ) ) {
					VerletTackDef_t *pTackDef = FindTackDef( apszPhysString[0], TRUE );
					if( pTackDef ) {
						f32 fWireStyleCode;
						if( CEntityParser::Interpret_F32Array( &fWireStyleCode, 1, 0.0f, 255.0f, TRUE, 1 ) ) {
							// 0 --> -1
							pTackDef->nWireStyleCode = (u8)fWireStyleCode - 1;
						}
					}
				}
			}
		}

		return TRUE;

	} else if( !fclib_stricmp( CEntityParser::m_pszTableName, "PhysAnchorSlice" ) ) {
		// PhysAnchorSlice tackname, on/off

		if( !m_bAllowVerlet ) {
			CEntityParser::Error_Prefix();
			DEVPRINTF( "Physics cannot be used on this type of object.\n" );
			CEntityParser::Error_Dashes();
		} else {
			if( CEntityParser::m_nFieldCount != 2 ) {
				CEntityParser::Error_InvalidParameterCount();
			} else {
				if( CEntityParser::Interpret_String( &apszPhysString[0], FALSE ) ) {
					VerletTackDef_t *pTackDef = FindTackDef( apszPhysString[0], FALSE );
					if( pTackDef ) {
						CEntityParser::Interpret_BOOL( &pTackDef->bAnchorSlice, FALSE, 1 );
					}
				}
			}
		}

		return TRUE;

	} else if( !fclib_stricmp( CEntityParser::m_pszTableName, "PhysAnchorHealth" ) ) {
		// PhysAnchorHealth tackname, health

		if( !m_bAllowVerlet ) {
			CEntityParser::Error_Prefix();
			DEVPRINTF( "Physics cannot be used on this type of object.\n" );
			CEntityParser::Error_Dashes();
		} else {
			if( CEntityParser::m_nFieldCount != 2 ) {
				CEntityParser::Error_InvalidParameterCount();
			} else {
				if( CEntityParser::Interpret_String( &apszPhysString[0], FALSE ) ) {
					VerletTackDef_t *pTackDef = FindTackDef( apszPhysString[0], TRUE );
					if( pTackDef ) {
						f32 fHealth;
						if( CEntityParser::Interpret_F32Array( &fHealth, 1, 0.0f, 100000.0f, TRUE, 1 ) ) {
							pTackDef->fAnchorHealth = fHealth;
						}
					}
				}
			}
		}

		return TRUE;

	} else if( !fclib_stricmp( CEntityParser::m_pszTableName, "PhysAnchorArmorProfile" ) ) {
		// PhysAnchorArmorProfile tackname, armorprofile

		if( !m_bAllowVerlet ) {
			CEntityParser::Error_Prefix();
			DEVPRINTF( "Physics cannot be used on this type of object.\n" );
			CEntityParser::Error_Dashes();
		} else {
			if( CEntityParser::m_nFieldCount != 2 ) {
				CEntityParser::Error_InvalidParameterCount();
			} else {
				if( CEntityParser::Interpret_StringArray( apszPhysString, 2 ) ) {
					VerletTackDef_t *pTackDef = FindTackDef( apszPhysString[0], TRUE );
					if( pTackDef ) {
						const CArmorProfile *pArmorProfile = CDamage::FindArmorProfile( apszPhysString[1], FALSE, FALSE );

						if( pArmorProfile == NULL ) {
							CEntityParser::Error_Prefix();
							DEVPRINTF( "Armor profile '%s' not found. Cannot set.\n", apszPhysString[1] );
							CEntityParser::Error_Dashes();
						} else {
							pTackDef->pArmorProfile = pArmorProfile;
						}
					}
				}
			}
		}

		return TRUE;
	} else if( !fclib_stricmp( CEntityParser::m_pszTableName, "PhysAnchorExplosionProfile" ) ) {
		// PhysAnchorExplosionProfile tackname, explosion profile

		if( !m_bAllowVerlet ) {
			CEntityParser::Error_Prefix();
			DEVPRINTF( "Physics cannot be used on this type of object.\n" );
			CEntityParser::Error_Dashes();
		} else {
			if( CEntityParser::m_nFieldCount != 2 ) {
				CEntityParser::Error_InvalidParameterCount();
			} else {
				if( CEntityParser::Interpret_StringArray( apszPhysString, 2 ) ) {
					VerletTackDef_t *pTackDef = FindTackDef( apszPhysString[0], TRUE );
					if( pTackDef ) {
						pTackDef->hExplosionGroup = CExplosion2::GetExplosionGroup( apszPhysString[1] );
					}
				}
			}
		}

		return TRUE;
	} else if( !fclib_stricmp( CEntityParser::m_pszTableName, "PhysCollSurf" ) ) {
		// List of collision entity names...

		CEntityParser::Interpret_StringArray_CheckSyntax( (FGameDataTableHandle_t *)&m_ppszVerletSurfaceNames );
		return TRUE;

	} else if( !fclib_stricmp( CEntityParser::m_pszTableName, "Actionable" ) ) {
		CEntityParser::Interpret_Flag( &m_nEC_Flags, CEntity::ENTITY_FLAG_ACTIONABLE );
		return TRUE;
	} else if (_AlarmSys_Builder.InterpretTable(this)) {
		return TRUE;
	} else if (_SpawnSys_Builder.InterpretTable(this)) {
		return TRUE;
	} else if (CTripwireBuilder::InterpretTable(this)) {
		return TRUE;
	}

	// Give AI a crack at interpreting the table...

	if( m_pAIBuilder ) {
		// AI builder present...
		if( m_pAIBuilder->InterpretTable( this ) ) {
			// AI interpreted...
			return TRUE;
		}
	} else {
		// No AI builder present. Absorb AI_* commands...

		if( fclib_strlen( CEntityParser::m_pszTableName ) >= 3 ) {
			if( !fclib_strnicmp( CEntityParser::m_pszTableName, "AI_", 3 ) ) {
				// Found "AI_*"...
				return TRUE;
			}
		}
	}

	// Give goodie bags a crack at interpreting the table...
	if( CGoodieProps::InterpretTable( CEntityParser::m_hTable, &m_pGoodieProps) ) {
		return(TRUE);
	}

	CEntityParser::Error_UnknownCommand();

	return TRUE;
}


CEntityBuilder::VerletTackDef_t *CEntityBuilder::FindTackDef( cchar *pszTackName, BOOL bAppendIfNotFound ) {
	u32 i;

	for( i=0; i<m_nTackDefCount; ++i ) {
		if( !fclib_stricmp( pszTackName, m_aTackDef[i].pszTackName ) ) {
			return &m_aTackDef[i];
		}
	}

	if( bAppendIfNotFound ) {
		if( m_nTackDefCount < ENTITY_MAX_VERLET_TACKS_PER_OBJECT ) {
			m_aTackDef[m_nTackDefCount].pszTackName = pszTackName;
			m_aTackDef[m_nTackDefCount].pszAnchorAttachToEntity = NULL;
			m_aTackDef[m_nTackDefCount].pszAnchorAttachToTack = NULL;
			m_aTackDef[m_nTackDefCount].pArmorProfile = &CDamage::m_ArmorProfileInfinite;
			m_aTackDef[m_nTackDefCount].hExplosionGroup = FEXPLOSION_INVALID_HANDLE;
			m_aTackDef[m_nTackDefCount].bAnchorEnabled = TRUE;
			m_aTackDef[m_nTackDefCount].nAnchor = VERLET_ANCHOR_NONE;
			m_aTackDef[m_nTackDefCount].fMinConstraintDist = 0.0f;
			m_aTackDef[m_nTackDefCount].fMaxConstraintDist = 0.0f;
			m_aTackDef[m_nTackDefCount].fAnchorHealth = 1.0f;
			m_aTackDef[m_nTackDefCount].nWireStyleCode = -1;
			m_aTackDef[m_nTackDefCount].bAnchorSlice = FALSE;

			++m_nTackDefCount;

			return &m_aTackDef[m_nTackDefCount-1];
		}
	}

	return NULL;
}


BOOL CEntityBuilder::InitVerlet( CFVerlet *pVerlet ) {
	const VerletTackDef_t *pTackDef;
	CFVerletTack *pTack;
	u32 i, j;

	pVerlet->SetDampening( m_fVerletUnitDampening );
	pVerlet->SetPrimeSecs( m_fVerletPrimeSecs );
	pVerlet->SetGravity( m_fVerletGravity );

	for( j=0; j<pVerlet->Tack_GetCount(); ++j ) {
		pVerlet->Tack_GetFromIndex(j)->m_nUser = VERLET_ANCHOR_NONE;
	}

	// Set up preliminary tack data...
	for( i=0; i<m_nTackDefCount; ++i ) {
		// Find tack in the Verlet object...

		pTackDef = &m_aTackDef[i];

		for( j=0; j<pVerlet->Tack_GetCount(); ++j ) {
			pTack = pVerlet->Tack_GetFromIndex(j);

			if( !fclib_stricmp( pTackDef->pszTackName, pTack->GetTackName() ) ) {
				// Found tack...

				pTack->m_nUser = pTackDef->nAnchor;

				if( pTackDef->nAnchor != VERLET_ANCHOR_NONE ) {
					pTack->Anchor_SetToImmovablePoint_Static();

					pTack->m_pUser1 = (void *)gstring_Main.AddString( pTackDef->pszAnchorAttachToEntity );
					pTack->m_pUser2 = (void *)gstring_Main.AddString( pTackDef->pszAnchorAttachToTack );

					if( (!pTack->m_pUser1 && pTackDef->pszAnchorAttachToEntity) || (!pTack->m_pUser2 && pTackDef->pszAnchorAttachToTack) ) {
						DEVPRINTF( "CEntityBuilder::InitVerlet(): Not enough memory to store tack init strings.\n" );
						return FALSE;
					}

					pTack->m_pUser3 = (void *)pTackDef->pArmorProfile;
					pTack->SetExplosionGroup( pTackDef->hExplosionGroup );
				}

				pTack->Anchor_SetRange( pTackDef->fMinConstraintDist, pTackDef->fMaxConstraintDist );
				pTack->Anchor_SetUnitHealth( pTackDef->fAnchorHealth );
				pTack->Anchor_DrawWire( pTackDef->nWireStyleCode );
				pTack->Anchor_EnableWireCollision( pTackDef->bAnchorSlice );
				pTack->Anchor_Enable( pTackDef->bAnchorEnabled );

				break;
			}
		}

		if( j == pVerlet->Tack_GetCount() ) {
			DEVPRINTF( "CEntityBuilder::InitVerlet(): Tack name '%s' specified in user props was not found.\n", pTackDef->pszTackName );
		}
	}

	if( m_nVerletSurfaceCount ) {
		// Create the CFVerletTackSurface array for use with this object.
		// Create the array for now, and fill with pointers to the entity string names.
		// Later, grab those string names and resolve them to actual surface pointers (done at ResolveEntityPointerFixups time).

		FASSERT( m_nVerletSurfaceCount > 0 );

		FResFrame_t ResFrame = fres_GetFrame();

		CFVerletTackSurface **papVerletTackSurfaces = (CFVerletTackSurface**) fres_AllocAndZero( sizeof( CFVerletTackSurface* ) * m_nVerletSurfaceCount );
		if( papVerletTackSurfaces == NULL ) {
			DEVPRINTF( "CEntityBuilder::InitVerlet(): Not enough memory to create papVerletTachSurfaces.\n" );
			fres_ReleaseFrame( ResFrame );
			return FALSE;
		}

		for( i=0; i<m_nVerletSurfaceCount; i++ ) {
			papVerletTackSurfaces[i] = ( CFVerletTackSurface* )m_ppszVerletSurfaceNames[ i ];
		}

		pVerlet->SetTackSurfaceArray( papVerletTackSurfaces, m_nVerletSurfaceCount );
	}

	pVerlet->Net_Freeze( m_bVerletFreeze );
	pVerlet->Net_MakeOneShot( m_bVerletFreezeWhenMotionless );

	if( m_bDieWhenCollide ) {
		pVerlet->SetExplodeCallback( CEntity::_VerletExplodeCallback );
		pVerlet->EnableExplodeFunctionality( TRUE );
	}

	return TRUE;
}


void CEntityBuilder::FixupVerlet( CEntity *pEntity, CFVerlet *pVerlet ) {
	CFVec3A TackPos_WS;
	CFVerletTack *pTack, *pTackToAnchorTo;
	VerletAnchor_e nAnchor;
	CEntity *pEntityToAnchorTo, *pTackSurfaceEntity;
	cchar *pszEntityName;
	char szNewEntityName[100];
	BOOL bNextInSeq, bPrevInSeq, bAttachToSame, bFilterWhiteSpace;
	s32 nStringLen, nCharIndex, nFirstNumericCharIndex, nLastNumericCharIndex, nSeqNum, nPower;
	u32 i;
	char c;

	for( i=0; i<pVerlet->Tack_GetCount(); ++i ) {
		pTack = pVerlet->Tack_GetFromIndex(i);

		nAnchor = (VerletAnchor_e)pTack->m_nUser;

		switch( nAnchor ) {
		case VERLET_ANCHOR_NONE:
			pTack->Anchor_Remove();
			break;

		case VERLET_ANCHOR_ORIG_TACK_POS:
			break;

		case VERLET_ANCHOR_SPECIFIED_TACK:
			pszEntityName = (cchar *)pTack->m_pUser1;

			bNextInSeq = bPrevInSeq = bAttachToSame = FALSE;

			if( !fclib_stricmp( pszEntityName, "NextInSeq" ) ) {
				bNextInSeq = TRUE;
			} else if( !fclib_stricmp( pszEntityName, "PrevInSeq" ) ) {
				bPrevInSeq = TRUE;
			}

			if( bNextInSeq || bPrevInSeq ) {
				pszEntityName = pEntity->Name();
				nStringLen = fclib_strlen( pszEntityName );

				nPower = 1;
				nSeqNum = -1;
				bFilterWhiteSpace = TRUE;

				for( nCharIndex=nStringLen-1; nCharIndex>=0; --nCharIndex ) {
					c = pszEntityName[nCharIndex];

					if( c==' ' && c=='\t' ) {
						if( bFilterWhiteSpace ) {
							continue;
						}

						break;
					}

					if( bFilterWhiteSpace ) {
						nLastNumericCharIndex = nCharIndex;
						bFilterWhiteSpace = FALSE;
					}

					if( c<'0' || c>'9' ) {
						break;
					}

					if( nSeqNum < 0 ) {
						nSeqNum = 0;
					}

					nSeqNum += (c-'0') * nPower;

					if( nPower > 1000000000 ) {
						nSeqNum = -1;
						break;
					}

					nPower *= 10;
				}

				if( (nSeqNum < 0) || (bPrevInSeq && (nSeqNum == 0)) ) {
					DEVPRINTF( "CEntityBuilder::FixupVerlet(): In user props, could not find next/prev in sequence for entity '%s'.\n", pszEntityName );
					pTack->Anchor_Remove();
					break;
				}

				if( bNextInSeq ) {
					FASSERT( nSeqNum < 0xffffffff );
					++nSeqNum;
				} else {
					FASSERT( nSeqNum > 0 );
					--nSeqNum;
				}

				nFirstNumericCharIndex = nCharIndex + 1;

				for( nCharIndex=0; nCharIndex<nFirstNumericCharIndex; ++nCharIndex ) {
					FASSERT( nCharIndex < 100 );
					szNewEntityName[nCharIndex] = pszEntityName[nCharIndex];
				}

				nStringLen = sprintf( &szNewEntityName[nCharIndex], "%u", nSeqNum );
				FASSERT( nStringLen < 99 );

				pszEntityName = szNewEntityName;

				pEntityToAnchorTo = CEntity::Find( pszEntityName );
				if( pEntityToAnchorTo == NULL ) {
					DEVPRINTF( "CEntityBuilder::FixupVerlet(): In user props, tack '%s' specifies entity '%s' which was not found.\n", pTack->GetTackName(), pszEntityName );
					pTack->Anchor_Remove();
					break;
				}
			} else if( !fclib_stricmp( pszEntityName, "Same" ) ) {
				pEntityToAnchorTo = (CEntity *)pVerlet->m_pUser;
			} else {
				pEntityToAnchorTo = CEntity::Find( pszEntityName );
				if( pEntityToAnchorTo == NULL ) {
					DEVPRINTF( "CEntityBuilder::FixupVerlet(): In user props, tack '%s' specifies entity '%s' which was not found.\n", pTack->GetTackName(), pszEntityName );
					pTack->Anchor_Remove();
					break;
				}
			}

			if( pEntityToAnchorTo == (CEntity *)pVerlet->m_pUser ) {
				bAttachToSame = TRUE;
			}

			if( pEntityToAnchorTo->GetVerlet() == NULL ) {
				DEVPRINTF( "CEntityBuilder::FixupVerlet(): In user props, tack '%s' specifies entity '%s' which isn't a physics object.\n", pTack->GetTackName(), pszEntityName );
				pTack->Anchor_Remove();
				break;
			}

			pTackToAnchorTo = pEntityToAnchorTo->GetVerlet()->Tack_FindByName( (cchar *)pTack->m_pUser2 );
			if( pTackToAnchorTo == NULL ) {
				DEVPRINTF( "CEntityBuilder::FixupVerlet(): In user props, tack '%s' specifies tack '%s' of entity '%s' which was not found.\n", pTack->GetTackName(), (cchar *)pTack->m_pUser2, pszEntityName );
				pTack->Anchor_Remove();
				break;
			}

			if( bAttachToSame ) {
				pTackToAnchorTo->ComputePos_WS( &TackPos_WS );
				pTack->Anchor_SetToImmovablePoint_Static( &TackPos_WS );
			} else {
				pTack->Anchor_SetToTack( pTackToAnchorTo );
			}

			break;

		case VERLET_ANCHOR_ENTITY_ORIGIN_STATIC:
			pszEntityName = (cchar *)pTack->m_pUser1;
			pEntityToAnchorTo = CEntity::Find( pszEntityName );

			if( pEntityToAnchorTo == NULL ) {
				DEVPRINTF( "CEntityBuilder::FixupVerlet(): In user props, tack '%s' specifies entity '%s' which was not found.\n", pTack->GetTackName(), pszEntityName );
				pTack->Anchor_Remove();
				break;
			}

			if( pEntityToAnchorTo == (CEntity *)pVerlet->m_pUser ) {
				DEVPRINTF( "CEntityBuilder::FixupVerlet(): In user props, tack '%s' specifies its own entity '%s' which is not allowed.\n", pTack->GetTackName(), pszEntityName );
				pTack->Anchor_Remove();
				break;
			}

			pTack->Anchor_SetToImmovablePoint_Static( &pEntityToAnchorTo->MtxToWorld()->m_vPos );

			break;

		case VERLET_ANCHOR_ENTITY_ORIGIN_DYNAMIC:
			pszEntityName = (cchar *)pTack->m_pUser1;
			pEntityToAnchorTo = CEntity::Find( pszEntityName );

			if( pEntityToAnchorTo == NULL ) {
				DEVPRINTF( "CEntityBuilder::FixupVerlet(): In user props, tack '%s' specifies entity '%s' which was not found.\n", pTack->GetTackName(), pszEntityName );
				pTack->Anchor_Remove();
				break;
			}

			if( pEntityToAnchorTo == (CEntity *)pVerlet->m_pUser ) {
				DEVPRINTF( "CEntityBuilder::FixupVerlet(): In user props, tack '%s' specifies its own entity '%s' which is not allowed.\n", pTack->GetTackName(), pszEntityName );
				pTack->Anchor_Remove();
				break;
			}

			pTack->Anchor_SetToImmovablePoint_Dynamic( &pEntityToAnchorTo->MtxToWorld()->m_vPos );

			break;

		default:
			FASSERT_NOW;
		};
	}

	// Fixup the Verlet CollisionSurfaceArray....

	u32 nVerletSurfaceCount = pVerlet->GetNumTackSurfaces();
	if( nVerletSurfaceCount ) {
		//get the surface array from the Verlet Object.
		//The array is currently stuffed with pointers to the CEBox string names that contain the collision object we are interested in.
		//Take those strings, find the CEBox object, grab their VerletTackSurface pointer, and stuff that pointer back into the array. 
		FASSERT( nVerletSurfaceCount > 0 );
		CFVerletTackSurface **papVerletTackSurfaces = pVerlet->GetTackSurfaceArray();
		FASSERT( papVerletTackSurfaces );
		u32 nValidTackSurfaces = 0;
		for( i=0; i< nVerletSurfaceCount; i++ ) {
			pszEntityName = ( char* ) papVerletTackSurfaces[i];
			pTackSurfaceEntity = CEntity::Find( pszEntityName );
			if( pTackSurfaceEntity == NULL ) {
				DEVPRINTF( "CEntityBuilder::FixupVerlet(): In user props, TackSurface entity '%s' which was not found.\n", pszEntityName );
				//set our pointer to null and move on to the next collsion surface name
				papVerletTackSurfaces[ i ] = NULL;
				continue;
			}

			//we have found the entity... ensure that it's a CEBox
			if( !( pTackSurfaceEntity->TypeBits() & ENTITY_BIT_BOX  ) ) {
				DEVPRINTF( "CEntityBuilder::FixupVerlet(): In user props, TackSurface entity '%s' is not a CEBox.\n", pszEntityName );
				//set our pointer to null and move on to the next collision surface name
				papVerletTackSurfaces[ i ] = NULL;
				continue;
			}

			//we have a valid CEBox object...  make sure this CEBox has a valid collision surface pointer
			CEBox* pEBox = ( CEBox* ) pTackSurfaceEntity;
			papVerletTackSurfaces[ nValidTackSurfaces ] = pEBox->GetVerletCollisionSurface();
			if( !papVerletTackSurfaces[ nValidTackSurfaces ] ) {
				//no collision surface pointer, just move on.
				DEVPRINTF( "CEntityBuilder::FixupVerlet(): In user props, TackSurface entity '%s' does not have a VerletTackSurface associated with it.\n", pszEntityName );
				continue;
			}

			//We have found a valid collision surface pointer!
			//Enable this collision surface (since it's being used)
			//and then increment our nValidTackSurfaceCount
			papVerletTackSurfaces[ nValidTackSurfaces ]->Enable( TRUE );
			nValidTackSurfaces++; //it's all good.
		} // for each surface in the array

		//now, reset the surface array pointer, with the updated values....
		pVerlet->SetTackSurfaceArray( papVerletTackSurfaces, nValidTackSurfaces );
	} //if there are VerletTackSurfaces
}


void CEntity::_VerletExplodeCallback( CFVerlet *pVerletToExplode ) {
	CEntity *pEntity = (CEntity *)pVerletToExplode->m_pUser;

	if( pEntity ) {
		pEntity->Die();
	}
}




//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CEntity
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

BOOL CEntity::m_bSystemInitialized;
u32 CEntity::m_nWirePoolCount = ENTITY_DEFAULT_WIRE_POOL_COUNT;
CNoControl CEntity::m_NoControl;
FLinkRoot_t CEntity::m_InWorldList_Root;
FLinkRoot_t CEntity::m_OutOfWorldList_Root;
FLinkRoot_t CEntity::m_AutoWorkList_Root;
FLinkRoot_t CEntity::m_MarkedList_Root;
FLinkRoot_t CEntity::m_CheckpointSaveList_Root[FCHECKPOINT_MAX_CHECKPOINTS];
FMesh_t *CEntity::m_pErrorMesh;

cchar *CEntity::m_apszEntityType[] = {
	ENTITY_TYPE_BOT,
	ENTITY_TYPE_WEAPON,
	ENTITY_TYPE_MESH,
	ENTITY_TYPE_BOTGLITCH,
	ENTITY_TYPE_WEAPONHAND,
	ENTITY_TYPE_WEAPONLASER,
	ENTITY_TYPE_WEAPONRIVET,
	ENTITY_TYPE_POINT,
	ENTITY_TYPE_DOOR,
	ENTITY_TYPE_SWITCH,
	ENTITY_TYPE_BOOMER,
	ENTITY_TYPE_SPLINE,
	ENTITY_TYPE_SPHERE,
	ENTITY_TYPE_LINE,
	ENTITY_TYPE_BOX,
	ENTITY_TYPE_BOTPRED,
	ENTITY_TYPE_WEAPONROCKET,
	ENTITY_TYPE_WEAPONBLASTER,
	ENTITY_TYPE_WEAPONGREN,
	ENTITY_TYPE_GOODIE,
	ENTITY_TYPE_PROJECTILE,
	ENTITY_TYPE_BOTGRUNT,
	ENTITY_TYPE_WEAPONTETHER,
	ENTITY_TYPE_CONSOLE,
	ENTITY_TYPE_WEAPONSPEW,
	ENTITY_TYPE_WEAPONMORTAR,
	ENTITY_TYPE_WEAPONFLAMER,
	ENTITY_TYPE_WEAPONRIPPER,
	ENTITY_TYPE_WEAPONCLEANER,
	ENTITY_TYPE_PARTICLE,
	ENTITY_TYPE_ZIPLINE,
	ENTITY_TYPE_WEAPONSCOPE,
	ENTITY_TYPE_BOTTITAN,
	ENTITY_TYPE_WEAPONCHAINGUN,
	ENTITY_TYPE_BOTSITEWEAPON,
	ENTITY_TYPE_VEHICLE,
	ENTITY_TYPE_VEHICLERAT,
	ENTITY_TYPE_SHIELD,
	ENTITY_TYPE_WEAPONEMP,
	ENTITY_TYPE_BOTCHEMBOT,
	ENTITY_TYPE_MAGMABOMB,
	ENTITY_TYPE_BOTPROBE,
	ENTITY_TYPE_VEHICLESENTINEL,
	ENTITY_TYPE_WEAPONQUADLASER,
	ENTITY_TYPE_LIQUIDVOLUME,
	ENTITY_TYPE_LIQUIDMESH,
	ENTITY_TYPE_VEHICLELOADER,
	ENTITY_TYPE_BOTSWARMER,
	ENTITY_TYPE_BOTELITEGUARD,
	ENTITY_TYPE_BOTAAGUN,
	ENTITY_TYPE_KRUNK,
	ENTITY_TYPE_BOTZOM,
	ENTITY_TYPE_BOTJUMPER,
	ENTITY_TYPE_BOTMINER,
	ENTITY_TYPE_BOTMORTAR,
	ENTITY_TYPE_BOTMOZER,
	ENTITY_TYPE_BOTSCOUT,
	ENTITY_TYPE_BOTCORROSIVE,
	ENTITY_TYPE_WEAPONWRENCH,
	ENTITY_TYPE_BOTSCIENTIST,
	ENTITY_TYPE_RECRUITER,
};



BOOL CEntity::InitSystem( void ) {
	FASSERT( !IsSystemInitialized() );

	m_bSystemInitialized = TRUE;

	m_nWirePoolCount = ENTITY_DEFAULT_WIRE_POOL_COUNT;

	_OutOfWorldList_Init();
	_InWorldList_Init();
	_AutoWorkList_Init();
	_MarkedList_Init();

	for( s32 nIndex = 0; nIndex < FCHECKPOINT_MAX_CHECKPOINTS; nIndex++ ) {
		_CheckpointSaveList_Init(nIndex);
	}

	CTripwire::TripwireList_Init();

	fworld_SetShapeCreateCallback( _WorldShapeCreateCallback );
	fworld_RegisterWorldCallbackFunction( _WorldCallback );

	m_pErrorMesh = (FMesh_t *)fresload_Load( FMESH_RESTYPE, ENTITY_ERROR_MESH_NAME );

	return TRUE;
}


void CEntity::UninitSystem( void ) {
	if( IsSystemInitialized() ) {
		fworld_UnregisterWorldCallbackFunction( _WorldCallback );
		fworld_SetShapeCreateCallback( NULL );

		m_pErrorMesh = NULL;

		m_bSystemInitialized = FALSE;
	}
}



// Only entities 
CEntity *CEntity::FindInWorld( cchar *pszName ) {
	CEntity *pEntity;

	FASSERT( IsSystemInitialized() );

	for( pEntity=InWorldList_GetHead(); pEntity; pEntity=pEntity->InWorldList_GetNext() ) {
		if( pEntity->Name() ) {
			if( !fclib_stricmp( pszName, pEntity->Name() ) ) {
				return pEntity;
			}
		}
	}

	return NULL;
}

// Only entities 
CEntity *CEntity::FindInWorld_Next( const CEntity *pCur, cchar *pszName ) {
	CEntity *pEntity;

	FASSERT( IsSystemInitialized() );

	for( pEntity=pCur->InWorldList_GetNext(); pEntity; pEntity=pEntity->InWorldList_GetNext() ) {
		if (pEntity) {
			if( pEntity->Name() ) {
				if( !fclib_stricmp( pszName, pEntity->Name() ) ) {
					return pEntity;
				}
			}
		}
	}

	return NULL;
}


// Only entities 
CEntity *CEntity::FindInWorld( u32 nGUID ) {
	CEntity *pEntity;

	FASSERT( IsSystemInitialized() );

	for( pEntity=InWorldList_GetHead(); pEntity; pEntity=pEntity->InWorldList_GetNext() ) {
		if( pEntity->Guid() == nGUID ) {
			return pEntity;
		}
	}

	return NULL;
}


CEntity *CEntity::FindOutOfWorld( cchar *pszName ) {
	CEntity *pEntity;

	FASSERT( IsSystemInitialized() );
	FASSERT( pszName );

	for( pEntity=OutOfWorldList_GetHead(); pEntity; pEntity=pEntity->OutOfWorldList_GetNext() ) {
		if( pEntity->Name() ) {
			if( !fclib_stricmp( pszName, pEntity->Name() ) ) {
				return pEntity;
			}
		}
	}

	return NULL;
}


CEntity *CEntity::FindOutOfWorld( u32 nGUID ) {
	CEntity *pEntity;

	FASSERT( IsSystemInitialized() );

	for( pEntity=OutOfWorldList_GetHead(); pEntity; pEntity=pEntity->OutOfWorldList_GetNext() ) {
		if( pEntity->Guid() == nGUID ) {
			return pEntity;
		}
	}

	return NULL;
}


CEntity *CEntity::Find( cchar *pszName ) {
	CEntity *pEntity;

	FASSERT( IsSystemInitialized() );
	FASSERT( pszName );
	if( pEntity = FindInWorld( pszName ) ) {
		return pEntity;
	}

	if( pEntity = FindOutOfWorld( pszName ) ) {
		return pEntity;
	}

	return NULL;
}


CEntity *CEntity::Find( u32 nGUID ) {
	CEntity *pEntity;

	FASSERT( IsSystemInitialized() );

	if( pEntity = FindInWorld( nGUID ) ) {
		return pEntity;
	}

	return NULL;
}


CEntity::CEntity() {
	flinklist_InitRoot( &m_ChildEntityRoot, FANG_OFFSETOF( CEntity, m_SiblingLink ) );
	_SetDefaults();
}


CEntity::~CEntity() {
	if( IsSystemInitialized() && IsCreated() ) {
		DetachFromParent();
		DetachAllChildren();
		RemoveFromWorld( TRUE );
		ClassHierarchyDestroy();
	}
}


// Set pszName to NULL for no name.
// Set pMtx to NULL for identity.
BOOL CEntity::Create( cchar *pszEntityName, const CFMtx43A *pMtx, cchar *pszAIBuilderName, u32 nTripwireEntityCount ) {
	FASSERT( IsSystemInitialized() );
	FASSERT( !IsCreated() );
	FASSERT( FWorld_pWorld );

	// Get a frame...
	FResFrame_t ResFrame = fres_GetFrame();

	// Get pointer to the leaf class's builder object...
	CEntityBuilder *pBuilder = GetLeafClassBuilder();

	// If we're the leaf class, set the builder defaults...
	if( pBuilder == &_EntityBuilder ) {
		pBuilder->SetDefaults( 0, 0, ENTITY_TYPE_ENTITY );
	}

	// Set our builder parameters...
	pBuilder->m_pszEC_EntityName = pszEntityName;

	pBuilder->m_TripwireBuilder.m_nEC_TripwireEntityCount = nTripwireEntityCount;  //is this some kind of override?

	if( pMtx == NULL ) {
		pMtx = &CFMtx43A::m_IdentityMtx;
	}

	pBuilder->m_EC_Mtx_WS = *pMtx;

	pBuilder->m_fEC_Scale_WS = pBuilder->m_EC_Mtx_WS.m_vRight.MagSq();
	if( pBuilder->m_fEC_Scale_WS != 1.0f ) {
		CFVec3A ScaleVec;

		pBuilder->m_fEC_Scale_WS = fmath_Sqrt( pBuilder->m_fEC_Scale_WS );

		ScaleVec.Set( fmath_Inv( pBuilder->m_fEC_Scale_WS ) );
		pBuilder->m_EC_Mtx_WS.m_vRight.Mul( ScaleVec );
		pBuilder->m_EC_Mtx_WS.m_vUp.Mul( ScaleVec );
		pBuilder->m_EC_Mtx_WS.m_vFront.Mul( ScaleVec );
	}

	pBuilder->m_pszAIBuilderName = pszAIBuilderName;
	if( pBuilder->m_pszAIBuilderName ) {
		// An AI builder has been specified...
		pBuilder->m_pAIBuilder = CAIBuilder::Request();

		if( pBuilder->m_pAIBuilder ) {
			pBuilder->m_pAIBuilder->SetDefaults( pBuilder );
		} else {
			pBuilder->m_pszAIBuilderName = NULL;
		}
	}

	// Build an entity...
	if( !Build() ) {
		// Could not build the entity...
		goto _ExitWithError;
	}

	// Success...

	return TRUE;

	// Error:
_ExitWithError:
	_SetDefaults();
	fres_ReleaseFrame( ResFrame );
	return FALSE;
}


void CEntity::Destroy( void ) {
	if( IsSystemInitialized() && IsCreated() ) {
		DetachFromParent();
		DetachAllChildren();
		RemoveFromWorld( TRUE );
		ClassHierarchyDestroy();
	}
}


void CEntity::ClassHierarchyDestroy( void ) {
	FASSERT( IsCreated() );
	FASSERT_MSG( !IsInWorld(), "CEntity::ClassHierarchyDestroy(): Leaf ClassHierarchyDestroy() functions must remove themselves from the world." );
	FASSERT_MSG( m_pParentEntity==NULL, "CEntity::ClassHierarchyDestroy(): Leaf ClassHierarchyDestroy() functions must detach themselves from their parent." );
	FASSERT_MSG( m_ChildEntityRoot.nCount==0, "CEntity::ClassHierarchyDestroy(): Leaf ClassHierarchyDestroy() functions must detach all children." );

	if( AIBrain() ) {
		aibrainman_Destroy( this );
		FASSERT( m_pAIBrain == NULL );
	}

	_OutOfWorldList_Remove();

	fdelete( m_pGoodieBag );
	fdelete( m_pTripwire );

	ClassHierarchyUnloadSharedResources();

	_SetDefaults();
}


// CEntityParser::Begin() must have already been called.
// Returns FALSE if the object could not be built.
BOOL CEntity::BuildFromWorldShape( void ) {
	FASSERT( IsSystemInitialized() );
	FASSERT( !IsCreated() );
	FASSERT( FWorld_pWorld );

	// Get a frame...
	FResFrame_t ResFrame = fres_GetFrame();

	// Get pointer to the leaf class's builder object...
	CEntityBuilder *pBuilder = GetLeafClassBuilder();

	// Interpret user-data file...
	if( !pBuilder->InterpretFile() ) {
		// Catastrophic failure...
		goto _ExitWithError;
	}

	// Build object...
	if( !Build() ) {
		// Object could not be built...
		goto _ExitWithError;
	}

	// Success...

	return TRUE;

	// Error:
_ExitWithError:
	_SetDefaults();
	fres_ReleaseFrame( ResFrame );
	return FALSE;
}


// The Builder object for the *this entity must have been initialized.
// Returns FALSE if the object could not be built.
// CEntityParser is not used.

BOOL CEntity::Build( void ) {
	CEntityBuilder *pBuilder;

	FASSERT( IsSystemInitialized() );
	FASSERT( !IsCreated() );
	FASSERT( FWorld_pWorld );

	pBuilder = GetLeafClassBuilder();

	FMATH_SETBITMASK( m_nEntityFlags, ENTITY_FLAG_BUILDING );

	// Build from the leaf of the class hierarchy down to CEntity...
	if( !ClassHierarchyBuild() ) {
		// Could not build object...
		goto _ExitWithError;
	}

	if( pBuilder->m_pAIBuilder ) {
		FASSERT(!m_pAIBrain);
		m_pAIBrain = aibrainman_Create( this, pBuilder );
		//  note from pgm.  it isn't really a bad thing to get NULL back.  It might just mean that your bot's "default" is to not have a brain.
	}

	if (!_AlarmSys_Builder.BuildAlarmEntityIfNeeded(this))
	{   //must be out of memory or something
		goto _ExitWithError;
	}

	if (!_SpawnSys_Builder.BuildSpawnSysObjIfNeeded(this))
	{   //must be out of memory or something
		goto _ExitWithError;
	}

	FMATH_CLEARBITMASK( m_nEntityFlags, ENTITY_FLAG_BUILDING );

	// Now that the entire object has been built,
	// add it to the world if so desired...
	if( pBuilder->m_nEC_Flags & ENTITY_FLAG_INWORLD ) {
		AddToWorld();
	}

	if( !ClassHierarchyBuilt() ) {
		// Could not build object...
		goto _ExitWithError;
	}

	if( pBuilder->m_pAIBuilder ) {
		CAIBuilder::Recycle( pBuilder->m_pAIBuilder );
		pBuilder->m_pAIBuilder = NULL;
	}

	// Object successfully built...	

	return TRUE;

	// Error:
_ExitWithError:
	if( pBuilder->m_pAIBuilder ) {
		CAIBuilder::Recycle( pBuilder->m_pAIBuilder );
		pBuilder->m_pAIBuilder = NULL;
	}

	FMATH_CLEARBITMASK( m_nEntityFlags, (ENTITY_FLAG_BUILDING | ENTITY_FLAG_CREATED) );

	return FALSE;
}

// Called from the leaf down to CEntity.
// Returns FALSE if the object could not be built.
BOOL CEntity::ClassHierarchyBuild( void ) {
	u32 nPrevFlags;

	FASSERT( IsSystemInitialized() );
	FASSERT( !IsCreated() );
	FASSERT( FWorld_pWorld );

	loadingscreen_Update();

	// Get a frame...
	FResFrame_t ResFrame = fres_GetFrame();

	// Get pointer to the leaf class's builder object...
	CEntityBuilder *pBuilder = GetLeafClassBuilder();

	// Add the entity name to the string table in case it hasn't already...
	if( pBuilder->m_pszEC_EntityName && !(pBuilder->m_pszEC_EntityName = gstring_Main.AddString( pBuilder->m_pszEC_EntityName )) ) {
		// Out of memory...
		goto _ExitWithError;
	}

	// Add the parent name to the string table in case it hasn't already...
	if( pBuilder->m_pszEC_ParentEntity && !(pBuilder->m_pszEC_ParentEntity = gstring_Main.AddString( pBuilder->m_pszEC_ParentEntity )) ) {
		// Out of memory...
		goto _ExitWithError;
	}

	// Set defaults...
	nPrevFlags = m_nEntityFlags;
	_SetDefaults();

	// Initialize from builder object...
	m_nEntityFlags = pBuilder->m_nEC_Flags & ~(ENTITY_FLAG_INWORLD | ENTITY_FLAG_ACTIONABLE | ENTITY_FLAG_TRIPWIRE_ARMED | ENTITY_FLAG_TRIPPER);
	m_nEntityFlags |= (nPrevFlags & ENTITY_FLAG_BUILDING);

	m_pszName = pBuilder->m_pszEC_EntityName;
	m_pszAttachBone = pBuilder->m_pszEC_AttachBone;
	m_nEntityTypeBits = pBuilder->m_nEC_TypeBits;
	m_nEntityLeafTypeBit = pBuilder->m_nEC_LeafTypeBit;
	m_MtxToWorld = pBuilder->m_EC_Mtx_WS;
	m_MtxToParent = pBuilder->m_EC_Mtx_WS;
	m_fScaleToWorld = pBuilder->m_fEC_Scale_WS;
	m_fScaleToParent = pBuilder->m_fEC_Scale_WS;
	m_fHealthContainerCount = (f32)( (pBuilder->m_nEC_HealthContainerCount >= 1) ? pBuilder->m_nEC_HealthContainerCount : 1 );
	m_fOOHealthContainerCount = 1.0f / m_fHealthContainerCount;
	m_fNormHealth = pBuilder->m_fEC_NormHealth;
	FMATH_CLAMP( m_fNormHealth, 0.0f, m_fHealthContainerCount );

	m_fArmorModifier = pBuilder->m_fEC_ArmorModifier;
	SetArmorProfile( CDamage::FindArmorProfile( pBuilder->m_pszEC_ArmorProfile ) );
	SetDamageProfile( CDamage::FindDamageProfile( pBuilder->m_pszEC_DamageProfile ) );

	if( pBuilder->m_bEC_ClassIsTripwireCapable && pBuilder->m_TripwireBuilder.m_nEC_TripwireEntityCount ) {
		// Entity is a tripwire...

		if( CTripwire::m_nTripwireEventNumber < 0 ) {
			DEVPRINTF( "CEntity::ClassHierarchyBuild(): Tripwire support is DISABLED.\n" );
			DEVPRINTF( "                                But an entity has been flagged to be a tripwire:\n" );

			if( pBuilder->m_pszEC_EntityName ) {
				DEVPRINTF( "                                Entity name = '%s'\n", pBuilder->m_pszEC_EntityName );
			} else {
				DEVPRINTF( "                                Entity name is unknown.\n" );
			}

			DEVPRINTF( "                                Object XYZ (Max space) = ( %.2f, %.2f, %.2f )\n", pBuilder->m_EC_Mtx_WS.m_vPos.x, pBuilder->m_EC_Mtx_WS.m_vPos.z, pBuilder->m_EC_Mtx_WS.m_vPos.y );
		} else {
			m_pTripwire = CTripwireBuilder::AllocNewTripWireFromBuilder(pBuilder, this);
			if( m_pTripwire == NULL ) {
				DEVPRINTF( "CEntity::ClassHierarchyBuild(): Not enough memory to allocate m_pTripwire.\n" );
				goto _ExitWithError;
			}

			if( pBuilder->m_nEC_Flags & ENTITY_FLAG_TRIPWIRE_ARMED ) {
				ArmTripwire( TRUE );
			}

			if( pBuilder->m_bIsCollectable ) {
				m_pTripwire->m_nEventFlags |= CEntity::TRIPWIRE_EVENTFLAG_ENTER_EVENT;
				ArmTripwire( TRUE );
			}
		}
	} else if( pBuilder->m_nEC_Flags & ENTITY_FLAG_TRIPPER ) {
		// Entity is a tripper...

		EnableTripper( TRUE );
	}

	if( pBuilder->m_nEC_Flags & ENTITY_FLAG_ACTIONABLE ) {
		SetActionable( TRUE );
	}

	if( pBuilder->m_pszEC_ParentEntity ) {
		// Parent name provided...
		m_pszParentEntityName = pBuilder->m_pszEC_ParentEntity;
	} else {
		// No parent name provided. Use index...
		m_nPendingAttachShapeIndex = pBuilder->m_nEC_ParentShapeIndex;
		FMATH_SETBITMASK( m_nEntityFlags, ENTITY_FLAG_BUILDPARENT_IS_INDEX );
	}

	if( pBuilder->m_oGoodieBag.NumGoodies() != 0 ) {
		m_pGoodieBag = fnew CGoodieBag;
		if( m_pGoodieBag == NULL ) {
			DEVPRINTF( "CEntity::ClassHierarchyBuild(): Not enough memory to allocate m_pGoodieBag.\n" );
			goto _ExitWithError;
		}
		m_pGoodieBag->CloneOther( &pBuilder->m_oGoodieBag );
		m_pGoodieBag->CreateEntities();
	}

	m_pGoodieProps = pBuilder->m_pGoodieProps;

	if( pBuilder->m_pszAmbientTag ) {
		SetAmbientRadius( pBuilder->m_fAmbientRadius );
		SetAmbientVolume( pBuilder->m_fAmbientVolume );
		SetAmbientSFXHandle( fsndfx_GetFxHandle( pBuilder->m_pszAmbientTag ) );

		if( m_hAmbientSFX == FSNDFX_INVALID_FX_HANDLE ) {
			DEVPRINTF( "CEntity::ClassHierarchyBuild(): Unable to get sFX handle for %s.\n", pBuilder->m_pszAmbientTag );
		}

		if( pBuilder->m_bAmbientPlay && (m_hAmbientSFX != FSNDFX_INVALID_FX_HANDLE ) ) {
			//Russ -- Removed SFX configuration,Creation, and play code from here 
			//since it also exists in the CEntity::PlayAmbientSFX member function.  
			//Why have the same code in two places??? 
			PlayAmbientSFX();
		}
	}

	m_nProjectileReaction = pBuilder->m_nProjectileReaction;

	AssignNewGuid();

	// Initially remove from world...
	_OutOfWorldList_AddTail();

	loadingscreen_Update();

	// Success...
	return TRUE;

	// Error:
_ExitWithError:
	fres_ReleaseFrame( ResFrame );
	return FALSE;
}


// Called after the class is 100% built.
// Returns FALSE if the object could not be built.
BOOL CEntity::ClassHierarchyBuilt( void ) {
	FASSERT( IsSystemInitialized() );
	FASSERT( IsCreated() );
	FASSERT( FWorld_pWorld );

	loadingscreen_Update();

	if( m_pTripwire ) {
		// We're a tripwire...

		m_MtxToWorld.MulPoint( m_pTripwire->m_BoundingSphere_WS.m_Pos, m_pTripwire->m_BoundingSphere_MS.m_Pos );
		m_pTripwire->m_BoundingSphere_WS.m_fRadius = m_fScaleToWorld * m_pTripwire->m_BoundingSphere_MS.m_fRadius;
	}

	DisableClassHierarchyWorkBit( ENTITY_CLASS_HIERARCHY_BIT );

	return TRUE;
}


void CEntity::GetGoodieDistributionOrigin(CFVec3A *pPt_WS)
{
	FASSERT(pPt_WS);
	pPt_WS->Set(m_MtxToWorld.m_vPos);
}


void CEntity::GetGoodieDistributionDir(CFVec3A *pVec_WS)
{
	FASSERT(pVec_WS); 
	pVec_WS->Set(m_MtxToWorld.m_vY);
}

CEntityBuilder *CEntity::GetLeafClassBuilder( void ) {
	return &_EntityBuilder;
}


// Sets the number of health containers. fHealthContainerCount must be a minimum of 1.
// If fHealthContainerCount is less than 1, it is clamped to 1.
void CEntity::SetHealthContainerCount( f32 fHealthContainerCount ) {
	FASSERT( IsCreated() );

	FMATH_CLAMPMIN( fHealthContainerCount, 1.0f );

	fHealthContainerCount = (f32)( (u32)fHealthContainerCount );

	if( m_fHealthContainerCount != fHealthContainerCount ) {
		m_fHealthContainerCount = fHealthContainerCount; // NKM added this since it wasn't being updated
		m_fOOHealthContainerCount = 1.0f / m_fHealthContainerCount;
		FMATH_CLAMPMAX( m_fNormHealth, m_fHealthContainerCount );
	}
}


// Sets health based on a normalized value: 0=no health, 1=fill one health container, 2=fill two health containers, etc.
void CEntity::SetNormHealth( f32 fNormHealth ) {
	FASSERT( IsCreated() );

	FMATH_CLAMP( fNormHealth, 0.0f, m_fHealthContainerCount );
	m_fNormHealth = fNormHealth;
}


// Sets health based on a unit value: 0=no health, 1=max health (fills all health containers).
void CEntity::SetUnitHealth( f32 fUnitHealth ) {
	FASSERT( IsCreated() );

	FMATH_CLAMP_UNIT_FLOAT( fUnitHealth );
	m_fNormHealth = fUnitHealth * m_fHealthContainerCount;
}


// Assigns the specified damage profile to this entity.
void CEntity::SetDamageProfile( CDamageProfile *pDamageProfile ) {
	FASSERT( IsCreated() );

	m_pDamageProfile = pDamageProfile;

	if( m_pTripwire ) {
		m_pTripwire->m_pDamageForm->m_pDamageProfile = m_pDamageProfile;
		CDamage::ResubmitDamageForm( m_pTripwire->m_pDamageForm );
	}
}


// Searches for the specified damage profile by name and sets the current profile.
// If the profile is not found, the default (infinite armor) is used.
void CEntity::FindAndSetDamageProfile( cchar *pszDamageProfileName, BOOL bDisplayErrorIfNotFound ) {
	FASSERT( IsCreated() );

	SetDamageProfile( CDamage::FindDamageProfile( pszDamageProfileName, TRUE, bDisplayErrorIfNotFound ) );
}


// Assigns the specified armor profile to this entity.
void CEntity::SetArmorProfile( CArmorProfile *pArmorProfile ) {
	FASSERT( IsCreated() );

	if( m_pArmorProfile == pArmorProfile ) {
		return;
	}
	
	m_pArmorProfile = pArmorProfile;

	u32 i;

	FMATH_CLEARBITMASK( m_nEntityFlags, ENTITY_FLAG_INFINITE_ARMOR );

	for( i=0; i<DAMAGE_HITPOINT_TYPE_COUNT; ++i ) {
		if( pArmorProfile->m_afUnitProtection[i] < 1.0f ) {
			break;
		}
	}

	if( i == DAMAGE_HITPOINT_TYPE_COUNT ) {
		if( pArmorProfile->m_fUnitProtectionFromImpulse >= 1.0f ) {
			if( pArmorProfile->m_fUnitProtectionFromRumble >= 1.0f ) {
				FMATH_SETBITMASK( m_nEntityFlags, ENTITY_FLAG_INFINITE_ARMOR );
			}
		}
	}
}


// Searches for the specified armor profile by name and sets the current profile.
// If the profile is not found, the default (infinite armor) is used.
void CEntity::FindAndSetArmorProfile( cchar *pszArmorProfileName, BOOL bDisplayErrorIfNotFound ) {
	FASSERT( IsCreated() );

	SetArmorProfile( CDamage::FindArmorProfile( pszArmorProfileName, TRUE, bDisplayErrorIfNotFound ) );
}


// -1 to 0: Attenuate affect of armor (-1=no armor, 0=no modification of armor)
// 0 to +1: Amplify affect of armor (0=no modification of armor, +1=infinite armor)
void CEntity::SetArmorModifier( f32 fArmorModifier ) {
	FASSERT( IsCreated() );

	FMATH_CLAMP( fArmorModifier, -1.0f, 1.0f );
	m_fArmorModifier = fArmorModifier;
}


// 0 to 1: Amplify affect of armor (0=no modification of armor, 1=infinite armor)
void CEntity::SetArmorModifier_UnitBoost( f32 fUnitBoost ) {
	FASSERT( IsCreated() );

	FMATH_CLAMP_UNIT_FLOAT( fUnitBoost );
	m_fArmorModifier = fUnitBoost;
}


// 0 to 1: Attenuate affect of armor (0=no modification of armor, 1=no armor)
void CEntity::SetArmorModifier_UnitAttenuate( f32 fUnitBoost ) {
	FASSERT( IsCreated() );

	FMATH_CLAMP_UNIT_FLOAT( fUnitBoost );
	m_fArmorModifier = -fUnitBoost;
}


// Called (normally by the damage module) to inflict damage upon this entity.
void CEntity::InflictDamage( CDamageData *pDamageData ) {
	FASSERT( IsCreated() );

	CDamageResult DamageResult;
	pDamageData->ComputeDamageResult( m_pArmorProfile, &DamageResult, m_fArmorModifier );

	InflictDamageResult( &DamageResult );
}


// Note that this function may not be called if the derived class has
// implemented its own handler.
void CEntity::InflictDamageResult( const CDamageResult *pDamageResult ) {
	FASSERT( IsCreated() );

	f32 fNormHealth = NormHealth();

	if( fNormHealth > 0.0f ) {
		fNormHealth -= pDamageResult->m_fDeltaHitpoints;
		SetNormHealth( fNormHealth );

		fNormHealth = NormHealth();
		if( fNormHealth > 0.0f ) {
			InformAIOfDamageResult( pDamageResult );
		} else {
			Die();
		}
	}

	ShakeCamera( pDamageResult );
}


void CEntity::ShakeCamera( const CDamageResult *pDamageResult ) {
	FASSERT( IsCreated() );

	BOOL bControllerRumbleOnly = TRUE;

	if( pDamageResult->m_pDamageData->m_nDamageLocale == CDamageForm::DAMAGE_LOCALE_BLAST ) {
		bControllerRumbleOnly = FALSE;
	}

	ShakeCamera( pDamageResult->m_fRumbleUnitIntensity, pDamageResult->m_pDamageData->m_pDamageProfile->m_fRumbleSecs, bControllerRumbleOnly );
}


void CEntity::InformAIOfDamageResult( const CDamageResult *pDamageResult ) {
	if( pDamageResult->m_fDeltaHitpoints != 0.0f ) {
		const CDamageData *pDamageData = pDamageResult->m_pDamageData;

		if( AIBrain() ) {
			// Damage was done to this entity's AI brain...
			aibrainman_DamagedNotify( AIBrain(), pDamageResult );
		}

		if( pDamageData->m_Damager.pBot && pDamageData->m_Damager.pBot->AIBrain() ) {
			// Damage was done by the damager entity's AI brain...
			aibrainman_DamageDealtNotify( pDamageData->m_Damager.pBot->AIBrain(), pDamageResult );
		}
	}
}


// Called to kill the entity ?
void CEntity::Die( BOOL bSpawnDeathEffects/*=TRUE*/, BOOL bSpawnGoodies ) {
	if( bSpawnGoodies && (m_pGoodieBag != NULL) ) {
		m_pGoodieBag->ReleaseIntoWorld( this );
	}
}


void CEntity::ShatterWire( CFWire *pWire, BOOL bSilent ) {
	CFVerletTack *pTack = (CFVerletTack *)pWire->GetOwner();

	if( pTack == NULL ) {
		return;
	}

	CEntity *pEntity = (CEntity *)pTack->GetTackOwner()->m_pUser;

	if( pEntity ) {
		pEntity->ClassHierarchyShatterWire( pWire, bSilent );
	}
}


// attempts to play entity's ambient sound.
// returns TRUE if successful, FALSE otherwise.
BOOL CEntity::PlayAmbientSFX( void ) {
	// Nate - So we don't eat up a virtual emitter
	/*if( m_pAmbientEmitter == NULL )
	{
		return FALSE;
	}

	// Nate - Don't use 0 for play since it ignores the internal loop count of the sound and
	//		  will cause non-looping sounds to loop forever.
	m_pAmbientEmitter->Play(0);*/

	if( m_hAmbientSFX == FSNDFX_INVALID_FX_HANDLE ) {
		return FALSE;
	}

	s32 nNumLoops = fsndfx_GetNumLoops( m_hAmbientSFX );

	if( nNumLoops <= -1 ) {
		// Bad handle...
		return FALSE;
	}

	if( nNumLoops > 0 ) {
		// Fire and forget...
		fsndfx_Play3D( m_hAmbientSFX, &m_MtxToWorld.m_vPos, m_fAmbientRadius, -1.0f, m_fAmbientVolume );
	} else {
		// Loop forever...

		if( m_pAmbientEmitter ) {
			return FALSE;
		}
		
		m_pAmbientEmitter = FSNDFX_ALLOCNPLAY3D( m_hAmbientSFX, &m_MtxToWorld.m_vPos, m_fAmbientRadius, m_fAmbientVolume, 1.0f, FAudio_EmitterDefaultPriorityLevel, TRUE );
				
		if( m_pAmbientEmitter == NULL ) {
			return FALSE;
		}
		//set the pause level for this emitter to 2, meaning that it won't pause during weapon select screens.
		//we only do this if this is a looping ambient emitter
		m_pAmbientEmitter->SetPauseLevel( FAUDIO_PAUSE_LEVEL_2 );
	}

	return TRUE;
}


// attempts to stop entity's ambient sound from playing.
// returns TRUE if successful, FALSE otherwise.
void CEntity::StopAmbientSFX( void ) {
	if( m_pAmbientEmitter == NULL ) {
		return;
	}

	// Nate - Just destroy it so we can release the virtual emitter
	//m_pAmbientEmitter->Stop(FALSE);
	m_pAmbientEmitter->Destroy();
	m_pAmbientEmitter = NULL;
}


// attempts to set the volume of the entity's ambient sound.
// returns TRUE if successful, FALSE otherwise.
void CEntity::SetAmbientVolume( f32 fVolume ) {
	FASSERT( IsCreated() );

	if( fVolume != m_fAmbientVolume ) {
		m_fAmbientVolume = fVolume;

		if( m_pAmbientEmitter ) {
			m_pAmbientEmitter->SetVolume( m_fAmbientVolume );
		}
	}
}


void CEntity::SetAmbientRadius( f32 fRadius ) {
	FASSERT( IsCreated() );

	if( fRadius != m_fAmbientRadius ) {
		m_fAmbientRadius = fRadius;

//right now, it's not possible to set the radius
//of an Emitter at play time... Only at creation time
//		if( m_pAmbientEmitter ) {
//			m_pAmbientEmitter->SetRadius( m_fAmbientRadius );
//		}
	}
}


void CEntity::SetAmbientPropertiesFromSoundGroup( CFSoundGroup *pSoundGroup, BOOL bPlay ) {
	FASSERT( IsCreated() );

	CFSoundInfo::DecompressedSoundInfo_t DecompressedSoundInfo;

	CFSoundGroup::GetRandomSoundInfo( pSoundGroup, &DecompressedSoundInfo );

	SetAmbientSFXHandle( DecompressedSoundInfo.m_hSound );
	SetAmbientVolume( DecompressedSoundInfo.m_fUnitVol3D );
	SetAmbientRadius( DecompressedSoundInfo.m_fRadius3D );

	if( bPlay ) {
		PlayAmbientSFX();
	}
}


void CEntity::_SetDefaults( void ) {
	u32 i;

	m_nEntityTypeBits = ENTITY_BIT_NONE;
	m_nEntityLeafTypeBit = ENTITY_BIT_NONE;
	m_nEntityFlags &= ENTITY_FLAG_AUTODELETE;
	m_nEnableWorkMask = 0;
	m_pParentEntity = NULL;
	m_pszName = NULL;
	m_nGuid = 0;

	m_pControl = &m_NoControl;
	m_pAIBrain = NULL;
	m_fNormHealth = 0.0f;

	m_pVerlet = NULL;
	m_pTripwire = NULL;
	m_nIntersectingTripwireCount = 0;
	for( i=0; i<MAX_INTERSECTING_TRIPWIRES; i++ ) {
		m_apIntersectingTripwires[i] = NULL;
	}

	m_pFcnAction = NULL;
	m_pGoodieBag = NULL;
	m_pGoodieProps = NULL;

	m_MtxToWorld.Identity();
	m_MtxToParent.Identity();
	m_PreviousParentMtxToWorld.Identity();
	m_pParentMtxToWorld = NULL;
	m_pszAttachBone = NULL;
	m_nPendingAttachShapeIndex = -1;

	m_fScaleToWorld = 1.0f;
	m_fScaleToParent = 1.0f;

	m_hAmbientSFX = FSNDFX_INVALID_FX_HANDLE;
	m_pAmbientEmitter = NULL;
	m_fAmbientVolume = 1.0f;
	m_fAmbientRadius = 20.0f;

	m_nProjectileReaction = CEntity::PROJECTILE_REACTION_DEFAULT;

	m_EntityLink.pNextLink = NULL;
	m_EntityLink.pPrevLink = NULL;
	m_AutoWorkLink.pNextLink = NULL;
	m_AutoWorkLink.pPrevLink = NULL;
	m_MarkedLink.pNextLink = NULL;
	m_MarkedLink.pPrevLink = NULL;
	m_SiblingLink.pNextLink = NULL;
	m_SiblingLink.pPrevLink = NULL;

	for( i=0; i<FCHECKPOINT_MAX_CHECKPOINTS; i++ ) {
		// Handle to save data for entity, indexed by checkpoint number...
		m_hSaveData[i] = CFCheckPoint::NULL_HANDLE;
	}
}




//-----------------------------------------------------------------------------
// Saves checkpoint save data for all entities that have state-saving enabled
BOOL CEntity::CheckpointSaveAll( s32 nCheckpoint ) {
	CEntity *pEntity;

	FASSERT( IsSystemInitialized() );

	// clear list and flags
	_CheckpointSaveList_Init( nCheckpoint );
	for( pEntity=OutOfWorldList_GetHead(); pEntity; pEntity=pEntity->OutOfWorldList_GetNext() )
	{
		pEntity->SetStateSaving( nCheckpoint, FALSE );
	}
	for( pEntity=InWorldList_GetHead(); pEntity; pEntity=pEntity->InWorldList_GetNext() )
	{
		pEntity->SetStateSaving( nCheckpoint, FALSE );
	}


	// select which entities will be saved
	for( pEntity=OutOfWorldList_GetHead(); pEntity; pEntity=pEntity->OutOfWorldList_GetNext() )
	{
		if( !pEntity->IsStateSaving( nCheckpoint ) )
		{
			// pEntity hasn't been saved yet
			pEntity->CheckpointSaveSelect( nCheckpoint );
		}
	}
	for( pEntity=InWorldList_GetHead(); pEntity; pEntity=pEntity->InWorldList_GetNext() )
	{
		if( !pEntity->IsStateSaving( nCheckpoint ) )
		{
			// pEntity hasn't been saved yet
			pEntity->CheckpointSaveSelect( nCheckpoint );
		}
	}

	// save selected entities
	for( pEntity=CheckpointSaveList_GetHead( nCheckpoint ); pEntity; pEntity=pEntity->CheckpointSaveList_GetNext( nCheckpoint ) )
	{
		if( !pEntity->CheckpointSave() )
		{
			return FALSE;
		}
	}

	return TRUE;
}


//-----------------------------------------------------------------------------
// loads the checkpoint save data for all entities that have state-saving enabled
void CEntity::CheckpointRestoreAll( s32 nCheckpoint ) {
	CEntity *pEntity;

	FASSERT( IsSystemInitialized() );

	// run preamble
//	for( pEntity=CheckpointSaveList_GetHead( nCheckpoint ); pEntity; pEntity=pEntity->CheckpointSaveList_GetNext( nCheckpoint ) )
//	{
//		pEntity->CheckpointRestorePreamble();
//	}

	// loop through saved entities and restore them
	for( pEntity=CheckpointSaveList_GetHead( nCheckpoint ); pEntity; pEntity=pEntity->CheckpointSaveList_GetNext( nCheckpoint ) )
	{
		pEntity->CheckpointRestore();

	}

	// run postamble
	for( pEntity=CheckpointSaveList_GetHead( nCheckpoint ); pEntity; pEntity=pEntity->CheckpointSaveList_GetNext( nCheckpoint ) )
	{
		pEntity->CheckpointRestorePostamble();
	}
}


//-----------------------------------------------------------------------------
// if TRUE, this entity will save and load its state
void CEntity::SetStateSaving( s32 nCheckpoint, BOOL bEnabled ) {
	FASSERT( IsCreated() );

	m_abCheckpointSave[ nCheckpoint ] = bEnabled;
}


//-----------------------------------------------------------------------------
void CEntity::CheckpointSaveSelect( s32 nCheckpoint ) {
	CEntity *pChildEntity;

	// Insure that children flagged as "Restore Attach" are saved/restored before their parent...
	for( pChildEntity=GetFirstChild(); pChildEntity; pChildEntity=GetNextChild(pChildEntity) ) {
		if( pChildEntity->IsRestoreAttach() ) {
			pChildEntity->CheckpointSaveList_AddTailAndMark( nCheckpoint );
		}
	}

	if ( IsTripwire() || m_pAmbientEmitter )
	{
		CheckpointSaveList_AddTailAndMark( nCheckpoint );
	}
}


//-----------------------------------------------------------------------------
// Save state information for entity.
// CheckpointSave() is virtual.  Derived class function will be called first. 
// Derived function should immediately call CheckpointSave() for its parent class before
// saving any data for the derived class.  This should continue down to the base 
// class function located here.
BOOL CEntity::CheckpointSave( void ) {

	// get and remember a data handle for this entity's checkpoint save data.
	// all subsequent data saved with SaveData() will be assigned to this handle,
	// until the next entity calls CreateObjectDataHandle() for itself.
	m_hSaveData[CFCheckPoint::GetCheckPoint()] = CFCheckPoint::CreateObjectDataHandle();

	if( m_pszName )
	{
		int a;
		a = 0;
	}

	// order of saving must match loading order in CheckpointRestore() below.

	// Before we save off the entity flags, check to see if we should restart our ambient sound
	if( m_pAmbientEmitter != NULL ) {
		FMATH_SETBITMASK( m_nEntityFlags, ENTITY_FLAG_RESTART_AMBIENT_SOUND_ON_RESTORE );
	}
 
	CFCheckPoint::SaveData( m_nEntityFlags );
	CFCheckPoint::SaveData( m_nEnableWorkMask );
	CFCheckPoint::SaveData( m_nGuid );
	CFCheckPoint::SaveData( m_MtxToWorld );
	CFCheckPoint::SaveData( m_MtxToParent );
	CFCheckPoint::SaveData( m_nProjectileReaction );

	CFCheckPoint::SaveData( m_hAmbientSFX );
	CFCheckPoint::SaveData( m_fAmbientRadius );
	CFCheckPoint::SaveData( m_fAmbientVolume );

	CFCheckPoint::SaveData( m_fArmorModifier );
	CFCheckPoint::SaveData( (u32 &)m_pArmorProfile );
	CFCheckPoint::SaveData( (u32 &)m_pDamageProfile );

	CFCheckPoint::SaveData( m_fNormHealth );
	CFCheckPoint::SaveData( m_fHealthContainerCount );
	CFCheckPoint::SaveData( m_fOOHealthContainerCount );

	//save off the verlet object (if it exists)
	if( m_pVerlet ) {
		m_pVerlet->CheckPointSave();
	}


	if( IsRestoreAttach() )
	{
		CFCheckPoint::SaveData( (u32&) m_pParentEntity );
		CFCheckPoint::SaveData( (u32&) m_pParentMtxToWorld );
		CFCheckPoint::SaveData( (u32&) m_pszAttachBone );
		CFCheckPoint::SaveData( m_PreviousParentMtxToWorld );
	}

	aibrainman_CheckpointSave( this );

	return TRUE;
}


//-----------------------------------------------------------------------------
void CEntity::CheckpointRestorePreamble( void ) {
}


//-----------------------------------------------------------------------------
// load state information for entity.
// CheckpointRestore() is virtual.  Derived class function will be called first. 
// Derived function should immediately call CheckpointRestore() for its parent class before
// loading any data for the derived class.  This should continue down to the base 
// class function located here.
void CEntity::CheckpointRestore( void ) {

	DrawEnable( FALSE, TRUE );

	// This will also remove the entity from the auto-work list...
	RemoveFromWorld( TRUE );

	// tell the checkpoint system what data handle to load data from.
	// all subsequent data loaded via LoadData() will be retrieved from
	// this handle's data.
	CFCheckPoint::SetObjectDataHandle( m_hSaveData[CFCheckPoint::GetCheckPoint()] );

	// order of loading must match loading order in CheckpointSave() above.
	CFCheckPoint::LoadData( m_nEntityFlags );
	CFCheckPoint::LoadData( m_nEnableWorkMask );
	CFCheckPoint::LoadData( m_nGuid );
	CFCheckPoint::LoadData( m_MtxToWorld );
	CFCheckPoint::LoadData( m_MtxToParent );
	CFCheckPoint::LoadData( m_nProjectileReaction );

	CFCheckPoint::LoadData( m_hAmbientSFX );
	CFCheckPoint::LoadData( m_fAmbientRadius );
	CFCheckPoint::LoadData( m_fAmbientVolume );

	CFCheckPoint::LoadData( m_fArmorModifier );
	CFCheckPoint::LoadData( (u32 &)m_pArmorProfile );
	CFCheckPoint::LoadData( (u32 &)m_pDamageProfile );

	CFCheckPoint::LoadData( m_fNormHealth );
	CFCheckPoint::LoadData( m_fHealthContainerCount );
	CFCheckPoint::LoadData( m_fOOHealthContainerCount );

	//restore the state of the verlet object (if it exists)
	if( m_pVerlet ) {
		m_pVerlet->CheckPointLoad();
	}
	aibrainman_CheckpointRestore( this );

	if( IsInWorld() )
	{
		// clear in-world bit so AddToWorld() will operate
		FMATH_CLEARBITMASK( m_nEntityFlags, ENTITY_FLAG_INWORLD );
		AddToWorld();

		if ( m_nEntityTypeBits & ENTITY_BIT_BOT ) 
		{
			CBot* pBot = (CBot*) this;

			if (pBot->UserAnim_IsCreated())
			{
				pBot->ComputeMtxPalette( FALSE );
			}

			if( pBot->m_nPossessionPlayerIndex >= 0 )
			{
				// special fixup for player AI brain
				aibrainman_ConfigurePlayerBotBrain( m_pAIBrain, pBot->m_nPossessionPlayerIndex);
				Player_aPlayer[pBot->m_nPossessionPlayerIndex].m_pEntityOrig->EnableAutoWork( FALSE );
				Player_aPlayer[pBot->m_nPossessionPlayerIndex].m_pEntityOrig->SetControls( &Player_aPlayer[pBot->m_nPossessionPlayerIndex].m_HumanControl );

			}
		}
	}

	if( IsRestoreAttach() )
	{
		CFCheckPoint::LoadData( (u32&) m_pParentEntity );
		CFCheckPoint::LoadData( (u32&) m_pParentMtxToWorld );
		CFCheckPoint::LoadData( (u32&) m_pszAttachBone );
		CFCheckPoint::LoadData( m_PreviousParentMtxToWorld );
		if( m_pParentEntity )
		{
			if( !flinklist_IsLinkInList( &m_pParentEntity->m_ChildEntityRoot, flinklist_GetLinkPointer( &m_pParentEntity->m_ChildEntityRoot, this ) ) ) 
			{
				flinklist_AddTail( &m_pParentEntity->m_ChildEntityRoot, this );
			}
		}
	}

	//restart this ambient sound effect (if it was running)
	if( m_nEntityFlags & ENTITY_FLAG_RESTART_AMBIENT_SOUND_ON_RESTORE ) {
		StopAmbientSFX(); //this will destroy it
		PlayAmbientSFX(); //this will restart it
		FMATH_CLEARBITMASK( m_nEntityFlags, ENTITY_FLAG_RESTART_AMBIENT_SOUND_ON_RESTORE );
	}

	DrawEnable( IsDrawEnabled(), TRUE );
}


//-----------------------------------------------------------------------------
void CEntity::CheckpointRestorePostamble( void ) {

	ClassHierarchyRelocated( NULL );

	aibrainman_CheckpointRestorePostamble( this );
}


//-----------------------------------------------------------------------------
void CEntity::CheckpointSaveList_Remove( s32 nCheckpoint ) { 
	FASSERT( IsSystemInitialized() ); 
	flinklist_Remove( &m_CheckpointSaveList_Root[nCheckpoint], this ); 
}


//-----------------------------------------------------------------------------
void CEntity::CheckpointSaveList_AddTailAndMark( s32 nCheckpoint ) { 
	FASSERT( IsSystemInitialized() ); 

	if( !IsStateSaving( nCheckpoint ) )
	{
		flinklist_AddTail( &m_CheckpointSaveList_Root[nCheckpoint], this ); 
		SetStateSaving( nCheckpoint, TRUE );
	}
}


//-----------------------------------------------------------------------------
void CEntity::SetRestoreAttach( BOOL bEnable ) {
	if( bEnable ) {
		FMATH_SETBITMASK( m_nEntityFlags, ENTITY_FLAG_RESTORE_ATTACH );
	} else {
		FMATH_CLEARBITMASK( m_nEntityFlags, ENTITY_FLAG_RESTORE_ATTACH );
	}
}


void CEntity::AddToWorld( void ) {
	FASSERT( IsCreated() );
	FASSERT_MSG( !_IsBuilding(), "CEntity::AddToWorld(): Use CEntityBuilder::m_nEC_Flags & ENTITY_FLAG_INWORLD to control initial in-world state." );

	if( !IsInWorld() ) {
		if( Guid() == 0 ) {
			AssignNewGuid();
		}

		ClassHierarchyAddToWorld();

		if( m_pParentEntity ) {
			m_pParentEntity->ChildHasBeenAddedToWorld( this );
		}
	} else {
		if( IsMarkedForWorldRemove() ) {
			_MarkedList_Remove();
		}
	}

	FMATH_CLEARBITMASK( m_nEntityFlags, ENTITY_FLAG_MARKED_FOR_WORLD_REMOVE );
}


void CEntity::RemoveFromWorld( BOOL bDoItNow ) {
	FASSERT( IsCreated() );

	if( IsInWorld() && !_IsBuilding() ) {
		if( !bDoItNow ) {
			// Flag for removal...

			if( !IsMarkedForWorldRemove() ) {
				_MarkedList_AddTail();
				FMATH_SETBITMASK( m_nEntityFlags, ENTITY_FLAG_MARKED_FOR_WORLD_REMOVE );
			}
		} else {
			// Remove now...

			if( IsMarkedForWorldRemove() ) {
				_MarkedList_Remove();
				FMATH_CLEARBITMASK( m_nEntityFlags, ENTITY_FLAG_MARKED_FOR_WORLD_REMOVE );
			}

			if( m_pParentEntity ) {
				m_pParentEntity->ChildHasBeenRemovedFromWorld( this );
			}
			StopAmbientSFX();

			ClassHierarchyRemoveFromWorld();
		}
	}
}


void CEntity::DrawEnable( BOOL bEnableDrawing, BOOL bForce /* == FALSE */ ) {
	FASSERT( IsCreated() );

	if( bEnableDrawing ) {
		if( !IsDrawEnabled() || bForce ) {
			FMATH_CLEARBITMASK( m_nEntityFlags, ENTITY_FLAG_DISABLE_DRAWING );

			ClassHierarchyDrawEnable( TRUE );
		}
	} else {
		if( IsDrawEnabled() || bForce ) {
			FMATH_SETBITMASK( m_nEntityFlags, ENTITY_FLAG_DISABLE_DRAWING );

			ClassHierarchyDrawEnable( FALSE );
		}
	}

#if 0
	if( bRecurseToChildren ) {
		CEntity *pChildEntity;

		for( pChildEntity=GetFirstChild(); pChildEntity; pChildEntity=GetNextChild(pChildEntity) ) {
			pChildEntity->DrawEnable( bEnableDrawing, TRUE );
		}
	}
#endif
}

void CEntity::ClassHierarchyAddToWorld( void ) {
	FASSERT( IsCreated() );
	FASSERT( !IsInWorld() );
	FASSERT( !_IsBuilding() );

	if( IsWorkEnabled() && m_nEnableWorkMask && IsAutoWorkEnabled() ) {
		_AutoWorkList_AddTail();
	}

	if( IsTripwire() && m_pTripwire->m_nEventFlags && m_pTripwire->m_nTripperFilterFlags && IsTripwireArmed() ) {
		m_pTripwire->TripwireList_AddTail();
	}

	_OutOfWorldList_Remove();
	_InWorldList_AddTail();

	FMATH_SETBITMASK( m_nEntityFlags, ENTITY_FLAG_INWORLD );

	if( AIBrain() ) {
		aibrainman_AddToWorld( this );
	}
}

void CEntity::_DetachAndRemoveAllChildrenAndRecurse( void ) {
	CEntity *pChildEntity, *pNextEntity;

	pNextEntity = GetFirstChild();
	while( pNextEntity ) {
		pChildEntity = pNextEntity;

		pNextEntity = GetNextChild( pNextEntity );

		if( !pChildEntity->IsRestoreAttach() ) {
			if( pChildEntity->RemovedOnParentsDetach() ) {
				// no flag set, same ops as before

				pChildEntity->_DetachAndRemoveAllChildrenAndRecurse(); 
				pChildEntity->DetachFromParent();
				pChildEntity->RemoveFromWorld();
			} else {
				// flag is set, don't recurse (further) and detach without remove
				pChildEntity->DetachFromParent();
			}
		}
	}
}


void CEntity::ClassHierarchyRemoveFromWorld( void ) {
	FASSERT( IsCreated() );
	FASSERT( IsInWorld() );
	FASSERT( !_IsBuilding() );

	_DetachAndRemoveAllChildrenAndRecurse();

	if( AIBrain() ) {
		aibrainman_RemoveFromWorld( this );
		//note! AIBrain is not cleared, incase this entity is re-added to the world, it should still have a brain
	}

	_InWorldList_Remove();
	_OutOfWorldList_AddTail();

	_ClearTripwireArray();
	if( IsTripwire() && m_pTripwire->m_nEventFlags && m_pTripwire->m_nTripperFilterFlags && IsTripwireArmed() ) {
		m_pTripwire->TripwireList_Remove();
	}

	if( IsWorkEnabled() && m_nEnableWorkMask && IsAutoWorkEnabled() ) {
		_AutoWorkList_Remove();
	}

	FMATH_CLEARBITMASK( m_nEntityFlags, ENTITY_FLAG_INWORLD );
}


// Given that the parent's m_MtxToWorld or m_fScaleToWorld has changed, this
// function recomputes the child's m_MtxToWorld, m_fScaleToWorld, m_MtxToParent,
// and m_fScaleToParent.
void CEntity::_Compute_ChildMtxAndScale( void ) {
	if( m_nEntityFlags & ENTITY_FLAG_PARENTMTXTOWORLD_POINTS_TO_MTXTOWORLD ) {
		// We know *m_pParentMtxToWorld is a unit matrix since it points to the parent's m_MtxToWorld...

		if( fmath_Abs( 1.0f - m_pParentEntity->ScaleToWorld() ) < 0.001f ) {
			// Parent's scale is very close to 1...

			m_MtxToWorld.Mul( *m_pParentMtxToWorld, m_MtxToParent );

			if( m_nEntityFlags & ENTITY_FLAG_INHERIT_PARENT_SCALE ) {
				// Inherit the parent's scale...
				m_fScaleToWorld = m_fScaleToParent;
			} else {
				// Keep our world scale constant, ignoring the parent's...
				m_fScaleToParent = m_fScaleToWorld;
			}
		} else {
			// Parent's scale isn't close to 1...

			m_pParentMtxToWorld->MulDir( m_MtxToWorld.m_vPos, m_MtxToParent.m_vPos );
			m_MtxToWorld.m_vPos.Mul( m_pParentEntity->ScaleToWorld() ).Add( m_pParentMtxToWorld->m_vPos );
			m_MtxToWorld.Mul33( *m_pParentMtxToWorld, m_MtxToParent );

			if( m_nEntityFlags & ENTITY_FLAG_INHERIT_PARENT_SCALE ) {
				// Inherit the parent's scale...
				m_fScaleToWorld = m_pParentEntity->ScaleToWorld() * m_fScaleToParent;
			} else {
				// Keep our world scale constant, ignoring the parent's...
				m_fScaleToParent = m_fScaleToWorld * fmath_Inv( m_pParentEntity->ScaleToWorld() );
			}
		}
	} else {
		// We know *m_pParentMtxToWorld is a non-unit matrix...

		f32 fOOParentMtxScaleToWorld = m_pParentMtxToWorld->m_vRight.InvMag();
		CFVec3A ScaleVec( fOOParentMtxScaleToWorld );

		m_MtxToWorld.Mul( *m_pParentMtxToWorld, m_MtxToParent );
		m_MtxToWorld.m_vRight.Mul( ScaleVec );
		m_MtxToWorld.m_vUp.Mul( ScaleVec );
		m_MtxToWorld.m_vFront.Mul( ScaleVec );

		if( m_nEntityFlags & ENTITY_FLAG_INHERIT_PARENT_SCALE ) {
			// Inherit the parent's scale...
			m_fScaleToWorld = fmath_Inv( fOOParentMtxScaleToWorld ) * m_fScaleToParent;
		} else {
			// Keep our world scale constant, ignoring the parent's...
			m_fScaleToParent = m_fScaleToWorld * fOOParentMtxScaleToWorld;
		}
	}
}


// Given that m_MtxToWorld and m_fScaleToWorld have changed, this function computes
// m_MtxToParent and m_fScaleToParent.
void CEntity::_Compute_MtxToParent_And_ScaleToParent_From_Modified_MtxToWorld_And_ScaleToWorld( void ) {
	if( m_pParentEntity == NULL ) {
		// No parent...

		m_MtxToParent = m_MtxToWorld;
		m_fScaleToParent = m_fScaleToWorld;
	} else {
		// This entity has a parent...

		if( m_nEntityFlags & ENTITY_FLAG_PARENTMTXTOWORLD_POINTS_TO_MTXTOWORLD ) {
			// We know *m_pParentMtxToWorld is a unit matrix since it points to the parent's m_MtxToWorld...

			if( fmath_Abs( 1.0f - m_pParentEntity->ScaleToWorld() ) < 0.001f ) {
				// Parent's scale is very close to 1...

				m_MtxToParent.ReceiveAffineInverse( *m_pParentMtxToWorld, FALSE ).Mul( m_MtxToWorld );
				m_fScaleToParent = m_fScaleToWorld;
			} else {
				// Parent's scale isn't close to 1...

				CFVec3A InvPos;
				f32 fOOParentMtxScaleToWorld = fmath_Inv( m_pParentEntity->ScaleToWorld() );

				m_MtxToParent.ReceiveAffineInverse( *m_pParentMtxToWorld, FALSE );
				InvPos = m_MtxToParent.m_vPos;

				m_MtxToParent.MulDir( m_MtxToParent.m_vPos, m_MtxToWorld.m_vPos );
				m_MtxToParent.m_vPos.Add( InvPos ).Mul( fOOParentMtxScaleToWorld );
				m_MtxToParent.Mul33( m_MtxToWorld );

				m_fScaleToParent = m_fScaleToWorld * fOOParentMtxScaleToWorld;
			}
		} else {
			// We know *m_pParentMtxToWorld is a non-unit matrix...

			f32 fOOParentMtxScaleToWorld = fmath_AcuSqrt( m_pParentMtxToWorld->m_vRight.MagSq() );
			CFVec3A ScaleVec( fOOParentMtxScaleToWorld );
			fOOParentMtxScaleToWorld = 1.0f / fOOParentMtxScaleToWorld;

//			f32 fOOParentMtxScaleToWorld = m_pParentMtxToWorld->m_vRight.InvMag();
//			CFVec3A ScaleVec( fmath_Inv( fOOParentMtxScaleToWorld ) );

			m_MtxToParent.ReceiveAffineInverse_KnowOOScale2( *m_pParentMtxToWorld, fOOParentMtxScaleToWorld * fOOParentMtxScaleToWorld );
			m_MtxToParent.Mul( m_MtxToWorld );
			m_MtxToParent.m_vRight.Mul( ScaleVec );
			m_MtxToParent.m_vUp.Mul( ScaleVec );
			m_MtxToParent.m_vFront.Mul( ScaleVec );

			m_fScaleToParent = m_fScaleToWorld * fOOParentMtxScaleToWorld;
		}
	}
}


// Given that m_MtxToParent and m_fScaleToParent have changed, this function computes
// m_MtxToWorld and m_fScaleToWorld.
void CEntity::_Compute_MtxToWorld_And_ScaleToWorld_From_Modified_MtxToParent_And_ScaleToParent( void ) {
	if( m_pParentEntity == NULL ) {
		// No parent...

		m_MtxToWorld = m_MtxToParent;
		m_fScaleToWorld = m_fScaleToParent;
	} else {
		// This entity has a parent...

		if( m_nEntityFlags & ENTITY_FLAG_PARENTMTXTOWORLD_POINTS_TO_MTXTOWORLD ) {
			// We know *m_pParentMtxToWorld is a unit matrix since it points to the parent's m_MtxToWorld...

			if( fmath_Abs( 1.0f - m_pParentEntity->ScaleToWorld() ) < 0.001f ) {
				// Parent's scale is very close to 1...

				m_MtxToWorld.Mul( *m_pParentMtxToWorld, m_MtxToParent );
				m_fScaleToWorld = m_fScaleToParent;
			} else {
				// Parent's scale isn't close to 1...

				m_pParentMtxToWorld->MulDir( m_MtxToWorld.m_vPos, m_MtxToParent.m_vPos );
				m_MtxToWorld.m_vPos.Mul( m_pParentEntity->ScaleToWorld() ).Add( m_pParentMtxToWorld->m_vPos );
				m_MtxToWorld.Mul33( *m_pParentMtxToWorld, m_MtxToParent );

				m_fScaleToWorld = m_pParentEntity->ScaleToWorld() * m_fScaleToParent;
			}
		} else {
			// We know *m_pParentMtxToWorld is a non-unit matrix...

			f32 fOOParentMtxScaleToWorld = m_pParentMtxToWorld->m_vRight.InvMag();
			CFVec3A ScaleVec( fOOParentMtxScaleToWorld );

			m_MtxToWorld.Mul( *m_pParentMtxToWorld, m_MtxToParent );
			m_MtxToWorld.m_vRight.Mul( ScaleVec );
			m_MtxToWorld.m_vUp.Mul( ScaleVec );
			m_MtxToWorld.m_vFront.Mul( ScaleVec );

			m_fScaleToWorld = fmath_Inv( fOOParentMtxScaleToWorld ) * m_fScaleToParent;
		}
	}
}


// Given that m_MtxToWorld has changed, this function computes m_MtxToParent.
void CEntity::_Compute_MtxToParent_From_Modified_MtxToWorld( void ) {
	if( m_pParentEntity == NULL ) {
		// No parent...

		m_MtxToParent = m_MtxToWorld;
	} else {
		// This entity has a parent...

		if( m_nEntityFlags & ENTITY_FLAG_PARENTMTXTOWORLD_POINTS_TO_MTXTOWORLD ) {
			// We know *m_pParentMtxToWorld is a unit matrix since it points to the parent's m_MtxToWorld...

			if( fmath_Abs( 1.0f - m_pParentEntity->ScaleToWorld() ) < 0.001f ) {
				// Parent's scale is very close to 1...

				m_MtxToParent.ReceiveAffineInverse( *m_pParentMtxToWorld, FALSE ).Mul( m_MtxToWorld );
			} else {
				// Parent's scale isn't close to 1...

				CFVec3A InvPos;
				f32 fOOParentMtxScaleToWorld = fmath_Inv( m_pParentEntity->ScaleToWorld() );

				m_MtxToParent.ReceiveAffineInverse( *m_pParentMtxToWorld, FALSE );
				InvPos = m_MtxToParent.m_vPos;

				m_MtxToParent.MulDir( m_MtxToParent.m_vPos, m_MtxToWorld.m_vPos );
				m_MtxToParent.m_vPos.Add( InvPos ).Mul( fOOParentMtxScaleToWorld );
				m_MtxToParent.Mul33( m_MtxToWorld );
			}
		} else {
			// We know *m_pParentMtxToWorld is a non-unit matrix...

			f32 fOOParentMtxScaleToWorld = fmath_AcuSqrt( m_pParentMtxToWorld->m_vRight.MagSq() );
			CFVec3A ScaleVec( fOOParentMtxScaleToWorld );
			fOOParentMtxScaleToWorld = 1.0f / fOOParentMtxScaleToWorld;

			//f32 fOOParentMtxScaleToWorld = m_pParentMtxToWorld->m_vRight.InvMag();
			//CFVec3A ScaleVec( fmath_Inv( fOOParentMtxScaleToWorld ) );

			m_MtxToParent.ReceiveAffineInverse_KnowOOScale2( *m_pParentMtxToWorld, fOOParentMtxScaleToWorld * fOOParentMtxScaleToWorld );
			m_MtxToParent.Mul( m_MtxToWorld );

			m_MtxToParent.m_vRight.Mul( ScaleVec );
			m_MtxToParent.m_vUp.Mul( ScaleVec );
			m_MtxToParent.m_vFront.Mul( ScaleVec );
		}
	}
}


// Given that m_MtxToParent has changed, this function computes m_MtxToWorld.
void CEntity::_Compute_MtxToWorld_From_Modified_MtxToParent( void ) {
	if( m_pParentEntity == NULL ) {
		// No parent...

		m_MtxToWorld = m_MtxToParent;
	} else {
		// This entity has a parent...

		if( m_nEntityFlags & ENTITY_FLAG_PARENTMTXTOWORLD_POINTS_TO_MTXTOWORLD ) {
			// We know *m_pParentMtxToWorld is a unit matrix since it points to the parent's m_MtxToWorld...

			if( fmath_Abs( 1.0f - m_pParentEntity->ScaleToWorld() ) < 0.001f ) {
				// Parent's scale is very close to 1...

				m_MtxToWorld.Mul( *m_pParentMtxToWorld, m_MtxToParent );
			} else {
				// Parent's scale isn't close to 1...

				m_pParentMtxToWorld->MulDir( m_MtxToWorld.m_vPos, m_MtxToParent.m_vPos );
				m_MtxToWorld.m_vPos.Mul( m_pParentEntity->ScaleToWorld() ).Add( m_pParentMtxToWorld->m_vPos );
				m_MtxToWorld.Mul33( *m_pParentMtxToWorld, m_MtxToParent );
			}
		} else {
			// We know *m_pParentMtxToWorld is a non-unit matrix...

			f32 fOOParentMtxScaleToWorld = m_pParentMtxToWorld->m_vRight.InvMag();
			CFVec3A ScaleVec( fOOParentMtxScaleToWorld );

			m_MtxToWorld.Mul( *m_pParentMtxToWorld, m_MtxToParent );
			m_MtxToWorld.m_vRight.Mul( ScaleVec );
			m_MtxToWorld.m_vUp.Mul( ScaleVec );
			m_MtxToWorld.m_vFront.Mul( ScaleVec );
		}
	}
}


// Given that m_fScaleToWorld has changed, this function computes m_fScaleToParent.
void CEntity::_Compute_ScaleToParent_From_Modified_ScaleToWorld( void ) {
	if( m_pParentEntity == NULL ) {
		// No parent...

		m_fScaleToParent = m_fScaleToWorld;
	} else {
		// This entity has a parent...

		if( m_nEntityFlags & ENTITY_FLAG_PARENTMTXTOWORLD_POINTS_TO_MTXTOWORLD ) {
			// We know *m_pParentMtxToWorld is a unit matrix since it points to the parent's m_MtxToWorld...

			if( fmath_Abs( 1.0f - m_pParentEntity->ScaleToWorld() ) < 0.001f ) {
				// Parent's scale is very close to 1...

				m_fScaleToParent = m_fScaleToWorld;
			} else {
				// Parent's scale isn't close to 1...

				m_fScaleToParent = m_fScaleToWorld * fmath_Inv( m_pParentEntity->ScaleToWorld() );
			}
		} else {
			// We know *m_pParentMtxToWorld is a non-unit matrix...

			m_fScaleToParent = m_fScaleToWorld * m_pParentMtxToWorld->m_vRight.InvMag();
		}
	}
}


// Given that m_fScaleToParent haas changed, this function computes m_fScaleToWorld.
void CEntity::_Compute_ScaleToWorld_From_Modified_ScaleToParent( void ) {
	if( m_pParentEntity == NULL ) {
		// No parent...

		m_fScaleToWorld = m_fScaleToParent;
	} else {
		// This entity has a parent...

		if( m_nEntityFlags & ENTITY_FLAG_PARENTMTXTOWORLD_POINTS_TO_MTXTOWORLD ) {
			// We know *m_pParentMtxToWorld is a unit matrix since it points to the parent's m_MtxToWorld...

			if( fmath_Abs( 1.0f - m_pParentEntity->ScaleToWorld() ) < 0.001f ) {
				// Parent's scale is very close to 1...

				m_fScaleToWorld = m_fScaleToParent;
			} else {
				// Parent's scale isn't close to 1...

				m_fScaleToWorld = m_pParentEntity->ScaleToWorld() * m_fScaleToParent;
			}
		} else {
			// We know *m_pParentMtxToWorld is a non-unit matrix...

			m_fScaleToWorld = m_pParentMtxToWorld->m_vRight.Mag() * m_fScaleToParent;
		}
	}
}


// Given that m_MtxToParent and m_fScaleToWorld have changed, this function computes
// m_MtxToWorld and m_fScaleToParent.
void CEntity::_Compute_MtxToWorld_And_ScaleToParent_From_Modified_MtxToParent_And_ScaleToWorld( void ) {
	if( m_pParentEntity == NULL ) {
		// No parent...

		m_MtxToWorld = m_MtxToParent;
		m_fScaleToParent = m_fScaleToWorld;
	} else {
		// This entity has a parent...

		if( m_nEntityFlags & ENTITY_FLAG_PARENTMTXTOWORLD_POINTS_TO_MTXTOWORLD ) {
			// We know *m_pParentMtxToWorld is a unit matrix since it points to the parent's m_MtxToWorld...

			if( fmath_Abs( 1.0f - m_pParentEntity->ScaleToWorld() ) < 0.001f ) {
				// Parent's scale is very close to 1...

				m_MtxToWorld.Mul( *m_pParentMtxToWorld, m_MtxToParent );
				m_fScaleToParent = m_fScaleToWorld;
			} else {
				// Parent's scale isn't close to 1...

				m_pParentMtxToWorld->MulDir( m_MtxToWorld.m_vPos, m_MtxToParent.m_vPos );
				m_MtxToWorld.m_vPos.Mul( m_pParentEntity->ScaleToWorld() ).Add( m_pParentMtxToWorld->m_vPos );
				m_MtxToWorld.Mul33( *m_pParentMtxToWorld, m_MtxToParent );

				m_fScaleToParent = m_fScaleToWorld * fmath_Inv( m_pParentEntity->ScaleToWorld() );
			}
		} else {
			// We know *m_pParentMtxToWorld is a non-unit matrix...

			f32 fOOParentMtxScaleToWorld = m_pParentMtxToWorld->m_vRight.InvMag();
			CFVec3A ScaleVec( fOOParentMtxScaleToWorld );

			m_MtxToWorld.Mul( *m_pParentMtxToWorld, m_MtxToParent );
			m_MtxToWorld.m_vRight.Mul( ScaleVec );
			m_MtxToWorld.m_vUp.Mul( ScaleVec );
			m_MtxToWorld.m_vFront.Mul( ScaleVec );

			m_fScaleToParent = m_fScaleToWorld * fOOParentMtxScaleToWorld;
		}
	}
}


void CEntity::_RelocatePostamble( const CFVec3A *pPrevPos_WS, BOOL bEnableTripping, void *pIdentifier ) {
	if( m_pTripwire ) {
		// We're a tripwire. Update our tripwire bounding sphere...

		m_MtxToWorld.MulPoint( m_pTripwire->m_BoundingSphere_WS.m_Pos, m_pTripwire->m_BoundingSphere_MS.m_Pos );
		m_pTripwire->m_BoundingSphere_WS.m_fRadius = m_fScaleToWorld * m_pTripwire->m_BoundingSphere_MS.m_fRadius;
	}

	// Let the class know this entity has been relocated...
	ClassHierarchyRelocated( pIdentifier );

	// Trigger any tripwires...
	if( (m_nEntityFlags & ENTITY_FLAG_TRIPPER) && bEnableTripping ) {
		// We can trigger tripwires...

		if( CTripwire::GetTripwireCount() ) {
			// There's at least one tripwire...

			_CheckMovedEntityAgainstTripwires( pPrevPos_WS, &m_MtxToWorld.m_vPos );
		}
	}

	// Let the class's children know their parent has been relocated...
	RelocateAllChildren( bEnableTripping );
}



// For all Relocate_*() functions:
//
// If bEnableTripping is TRUE, the movement by this function will be eligible for tripping tripwires.
//
// pIdentifier is normally NULL. If an entity's work function wishes to move itself,
// it calls this function to do so. In turn, this function calls ClassHierarchyRelocated()
// to inform the class hierarchy that the entity has moved. ClassHierarchyRelocated() is
// passed pIdentifier. The class whose work function called this function may check
// the value of pIdentifier passed to ClassHierarchyRelocated() and compare it to the value
// it passed to Relocate_WS(). If they match, ClassHierarchyRelocated() does not need to
// perform the relocation. A good identifier might be just the address of a local non-static
// variable in the class.



// Modified:
//   - Scale from fNewEntityScale_WS
//
// Not Modified:
//   - Translation
//   - Rotation
void CEntity::Relocate_Scale_WS( f32 fNewEntityScale_WS, BOOL bEnableTripping, void *pIdentifier ) {
	FASSERT( IsCreated() );
	FASSERT_MSG( !_IsBuilding(), "CEntity::Relocate_Scale_WS(): Cannot be called while building." );

	m_fScaleToWorld = fNewEntityScale_WS;

	_Compute_ScaleToParent_From_Modified_ScaleToWorld();

	_RelocatePostamble( &m_MtxToWorld.m_vPos, bEnableTripping, pIdentifier );
}


// Modified:
//   - Translation from *pNewXlat_WS
//
// Not Modified:
//   - Rotation
//   - Scale
void CEntity::Relocate_Xlat_WS( const CFVec3A *pNewXlat_WS, BOOL bEnableTripping, void *pIdentifier ) {
	CFVec3A PrevPos_WS;

	FASSERT( IsCreated() );
	FASSERT_MSG( !_IsBuilding(), "CEntity::Relocate_Xlat_WS(): Cannot be called while building." );

	PrevPos_WS = m_MtxToWorld.m_vPos;

	m_MtxToWorld.m_vPos = *pNewXlat_WS;

	_Compute_MtxToParent_From_Modified_MtxToWorld();

	_RelocatePostamble( &PrevPos_WS, bEnableTripping, pIdentifier );
}


// Modified:
//   - Rotation from *pUnitMtx_WS
//   - Translation from *pUnitMtx_WS
//
// Not Modified:
//   - Scale
void CEntity::Relocate_RotXlatFromUnitMtx_WS( const CFMtx43A *pUnitMtx_WS, BOOL bEnableTripping, void *pIdentifier ) {
	CFVec3A PrevPos_WS;

	FASSERT( IsCreated() );
	FASSERT_MSG( !_IsBuilding(), "CEntity::Relocate_RotXlatFromUnitMtx_WS(): Cannot be called while building." );

	PrevPos_WS = m_MtxToWorld.m_vPos;

	m_MtxToWorld = *pUnitMtx_WS;

	_Compute_MtxToParent_From_Modified_MtxToWorld();

	_RelocatePostamble( &PrevPos_WS, bEnableTripping, pIdentifier );
}


// Modified:
//   - Rotation from *pUnitMtx_WS
//   - Translation from *pUnitMtx_WS
//   - Scale from fNewEntityScale_WS
void CEntity::Relocate_RotXlatFromUnitMtx_WS_NewScale_WS( const CFMtx43A *pUnitMtx_WS, f32 fNewEntityScale_WS, BOOL bEnableTripping, void *pIdentifier ) {
	CFVec3A PrevPos_WS;

	FASSERT( IsCreated() );
	FASSERT_MSG( !_IsBuilding(), "CEntity::Relocate_RotXlatFromUnitMtx_WS_NewScale_WS(): Cannot be called while building." );

	PrevPos_WS = m_MtxToWorld.m_vPos;

	m_MtxToWorld = *pUnitMtx_WS;
	m_fScaleToWorld = fNewEntityScale_WS;

	_Compute_MtxToParent_And_ScaleToParent_From_Modified_MtxToWorld_And_ScaleToWorld();

	_RelocatePostamble( &PrevPos_WS, bEnableTripping, pIdentifier );
}


// Modified:
//   - Rotation from *pMtx_WS
//   - Translation from *pMtx_WS
//   - Scale from fScaleOfMtx
//
// Pass 0 for fScaleOfMtx to have this function compute the scale of *pMtx_WS.
// Otherwise, pMtx_WS have have a scale equal to fScaleOfMtx.
void CEntity::Relocate_RotXlatScaleFromScaledMtx_WS( const CFMtx43A *pMtx_WS, f32 fScaleOfMtx, BOOL bEnableTripping, void *pIdentifier ) {
	CFVec3A PrevPos_WS;

	FASSERT( IsCreated() );
	FASSERT_MSG( !_IsBuilding(), "CEntity::Relocate_RotXlatScaleFromScaledMtx_WS(): Cannot be called while building." );

	PrevPos_WS = m_MtxToWorld.m_vPos;

	if( fmath_Abs( 1.0f - fScaleOfMtx ) < 0.001f ) {
		// Provided scale is pretty close to 1.0f...

		m_MtxToWorld = *pMtx_WS;
		m_fScaleToWorld = 1.0f;
	} else {
		// Provided scale isn't close to 1.0f...

		f32 fOOScaleOfMtx;
		CFVec3A ScaleVec;

		if( fScaleOfMtx == 0.0f ) {
			// Caller wishes us to compute the matrice's scale...
			fOOScaleOfMtx = pMtx_WS->m_vRight.InvMag();
			m_fScaleToWorld = fmath_Inv( fOOScaleOfMtx );
		} else {
			fOOScaleOfMtx = fmath_Inv( fScaleOfMtx );
			m_fScaleToWorld = fScaleOfMtx;
		}

		ScaleVec.Set( fOOScaleOfMtx );

		m_MtxToWorld.m_vRight.Mul( pMtx_WS->m_vRight, ScaleVec );
		m_MtxToWorld.m_vUp.Mul( pMtx_WS->m_vUp, ScaleVec );
		m_MtxToWorld.m_vFront.Mul( pMtx_WS->m_vFront, ScaleVec );
		m_MtxToWorld.m_vPos = pMtx_WS->m_vPos;
	}

	_Compute_MtxToParent_And_ScaleToParent_From_Modified_MtxToWorld_And_ScaleToWorld();

	_RelocatePostamble( &PrevPos_WS, bEnableTripping, pIdentifier );
}


// Modified:
//   - Rotation from *pMtx_WS
//   - Translation from *pMtx_WS
//   - Scale from fNewEntityScale_WS
//
// Pass 0 for fScaleOfMtx to have this function compute the scale of *pMtx_WS.
// Otherwise, pMtx_WS have have a scale equal to fScaleOfMtx.
// fScaleOfMtx is not used when computing the entity's new scale.
void CEntity::Relocate_RotXlatFromScaledMtx_WS_NewScale_WS( const CFMtx43A *pMtx_WS, f32 fScaleOfMtx, f32 fNewEntityScale_WS, BOOL bEnableTripping, void *pIdentifier ) {
	CFVec3A PrevPos_WS;

	FASSERT( IsCreated() );
	FASSERT_MSG( !_IsBuilding(), "CEntity::Relocate_RotXlatFromScaledMtx_WS_NewScale_WS(): Cannot be called while building." );

	PrevPos_WS = m_MtxToWorld.m_vPos;

	if( fmath_Abs( 1.0f - fScaleOfMtx ) < 0.001f ) {
		// Provided scale is pretty close to 1.0f...

		m_MtxToWorld = *pMtx_WS;
	} else {
		// Provided scale isn't close to 1.0f...

		f32 fOOScaleOfMtx;
		CFVec3A ScaleVec;

		if( fScaleOfMtx == 0.0f ) {
			// Caller wishes us to compute the matrice's scale...
			fOOScaleOfMtx = pMtx_WS->m_vRight.InvMag();
		} else {
			fOOScaleOfMtx = fmath_Inv( fScaleOfMtx );
		}

		ScaleVec.Set( fOOScaleOfMtx );

		m_MtxToWorld.m_vRight.Mul( pMtx_WS->m_vRight, ScaleVec );
		m_MtxToWorld.m_vUp.Mul( pMtx_WS->m_vUp, ScaleVec );
		m_MtxToWorld.m_vFront.Mul( pMtx_WS->m_vFront, ScaleVec );
		m_MtxToWorld.m_vPos = pMtx_WS->m_vPos;
	}

	m_fScaleToWorld = fNewEntityScale_WS;

	_Compute_MtxToParent_And_ScaleToParent_From_Modified_MtxToWorld_And_ScaleToWorld();

	_RelocatePostamble( &PrevPos_WS, bEnableTripping, pIdentifier );
}


// Modified:
//   - Scale
//
// Not Modified:
//   - Translation:
//   - Rotation
void CEntity::Relocate_Scale_PS( f32 fNewEntityScale_PS, BOOL bEnableTripping, void *pIdentifier ) {
	FASSERT( IsCreated() );
	FASSERT_MSG( !_IsBuilding(), "CEntity::Relocate_Scale_PS(): Cannot be called while building." );

	m_fScaleToParent = fNewEntityScale_PS;

	_Compute_ScaleToWorld_From_Modified_ScaleToParent();

	_RelocatePostamble( &m_MtxToWorld.m_vPos, bEnableTripping, pIdentifier );
}


// Modified:
//   - Translation from *pNewXlat_PS
//
// Not Modified:
//   - Rotation
//   - Scale
void CEntity::Relocate_Xlat_PS( const CFVec3A *pNewXlat_PS, BOOL bEnableTripping, void *pIdentifier ) {
	CFVec3A PrevPos_WS;

	FASSERT( IsCreated() );
	FASSERT_MSG( !_IsBuilding(), "CEntity::Relocate_Xlat_PS(): Cannot be called while building." );

	PrevPos_WS = m_MtxToWorld.m_vPos;

	m_MtxToParent.m_vPos = *pNewXlat_PS;

	_Compute_MtxToWorld_From_Modified_MtxToParent();

	_RelocatePostamble( &PrevPos_WS, bEnableTripping, pIdentifier );
}


// Modified:
//   - Rotation from *pUnitMtx_PS
//   - Translation from *pUnitMtx_PS
//
// Not Modified:
//   - Scale
void CEntity::Relocate_RotXlatFromUnitMtx_PS( const CFMtx43A *pUnitMtx_PS, BOOL bEnableTripping, void *pIdentifier ) {
	CFVec3A PrevPos_WS;

	FASSERT( IsCreated() );
	FASSERT_MSG( !_IsBuilding(), "CEntity::Relocate_RotXlatFromUnitMtx_PS(): Cannot be called while building." );

	PrevPos_WS = m_MtxToWorld.m_vPos;

	m_MtxToParent = *pUnitMtx_PS;

	_Compute_MtxToWorld_From_Modified_MtxToParent();

	_RelocatePostamble( &PrevPos_WS, bEnableTripping, pIdentifier );
}


// Modified:
//   - Rotation from *pUnitMtx_PS
//   - Translation from *pUnitMtx_PS
//   - Scale from fNewEntityScale_PS
void CEntity::Relocate_RotXlatFromUnitMtx_PS_NewScale_PS( const CFMtx43A *pUnitMtx_PS, f32 fNewEntityScale_PS, BOOL bEnableTripping, void *pIdentifier ) {
	CFVec3A PrevPos_WS;

	FASSERT( IsCreated() );
	FASSERT_MSG( !_IsBuilding(), "CEntity::Relocate_RotXlatFromUnitMtx_PS_NewScale_PS(): Cannot be called while building." );

	PrevPos_WS = m_MtxToWorld.m_vPos;

	m_MtxToParent = *pUnitMtx_PS;
	m_fScaleToParent = fNewEntityScale_PS;

	_Compute_MtxToWorld_And_ScaleToWorld_From_Modified_MtxToParent_And_ScaleToParent();

	_RelocatePostamble( &PrevPos_WS, bEnableTripping, pIdentifier );
}


// Modified:
//   - Rotation from *pUnitMtx_PS
//   - Translation from *pUnitMtx_PS
//   - Scale from fNewEntityScale_WS
void CEntity::Relocate_RotXlatFromUnitMtx_PS_NewScale_WS( const CFMtx43A *pUnitMtx_PS, f32 fNewEntityScale_WS, BOOL bEnableTripping, void *pIdentifier ) {
	CFVec3A PrevPos_WS;

	FASSERT( IsCreated() );
	FASSERT_MSG( !_IsBuilding(), "CEntity::Relocate_RotXlatFromUnitMtx_PS_NewScale_WS(): Cannot be called while building." );

	PrevPos_WS = m_MtxToWorld.m_vPos;

	m_MtxToParent = *pUnitMtx_PS;
	m_fScaleToWorld = fNewEntityScale_WS;

	_Compute_MtxToWorld_And_ScaleToParent_From_Modified_MtxToParent_And_ScaleToWorld();

	_RelocatePostamble( &PrevPos_WS, bEnableTripping, pIdentifier );
}


// Modified:
//   - Rotation from *pMtx_PS
//   - Translation from *pMtx_PS
//   - Scale from fScaleOfMtx (fScaleOfMtx is considered in parent space when computing new entity scale)
//
// Pass 0 for fScaleOfMtx to have this function compute the scale of *pMtx_PS.
// Otherwise, *pMtx_PS have have a scale equal to fScaleOfMtx.
void CEntity::Relocate_RotXlatFromScaledMtx_PS_ScaleFromScaledMtx_PS( const CFMtx43A *pMtx_PS, f32 fScaleOfMtx, BOOL bEnableTripping, void *pIdentifier ) {
	CFVec3A PrevPos_WS;

	FASSERT( IsCreated() );
	FASSERT_MSG( !_IsBuilding(), "CEntity::Relocate_RotXlatFromScaledMtx_PS_ScaleFromScaledMtx_PS(): Cannot be called while building." );

	PrevPos_WS = m_MtxToWorld.m_vPos;

	if( fmath_Abs( 1.0f - fScaleOfMtx ) < 0.001f ) {
		// Provided scale is pretty close to 1.0f...

		m_MtxToParent = *pMtx_PS;
		m_fScaleToParent = 1.0f;
	} else {
		// Provided scale isn't close to 1.0f...

		f32 fOOScaleOfMtx;
		CFVec3A ScaleVec;

		if( fScaleOfMtx == 0.0f ) {
			// Caller wishes us to compute the matrice's scale...
			fOOScaleOfMtx = pMtx_PS->m_vRight.InvMag();
			m_fScaleToParent = fmath_Inv( fOOScaleOfMtx );
		} else {
			fOOScaleOfMtx = fmath_Inv( fScaleOfMtx );
			m_fScaleToParent = fScaleOfMtx;
		}

		ScaleVec.Set( fOOScaleOfMtx );

		m_MtxToParent.m_vRight.Mul( pMtx_PS->m_vRight, ScaleVec );
		m_MtxToParent.m_vUp.Mul( pMtx_PS->m_vUp, ScaleVec );
		m_MtxToParent.m_vFront.Mul( pMtx_PS->m_vFront, ScaleVec );
		m_MtxToParent.m_vPos = pMtx_PS->m_vPos;
	}

	_Compute_MtxToWorld_And_ScaleToWorld_From_Modified_MtxToParent_And_ScaleToParent();

	_RelocatePostamble( &PrevPos_WS, bEnableTripping, pIdentifier );
}


// Modified:
//   - Rotation from *pMtx_PS
//   - Translation from *pMtx_PS
//   - Scale from fScaleOfMtx (fScaleOfMtx is considered in world space when computing new entity scale)
//
// Pass 0 for fScaleOfMtx to have this function compute the scale of *pMtx_PS.
// Otherwise, *pMtx_PS have have a scale equal to fScaleOfMtx.
void CEntity::Relocate_RotXlatFromScaledMtx_PS_ScaleFromScaledMtx_WS( const CFMtx43A *pMtx_PS, f32 fScaleOfMtx, BOOL bEnableTripping, void *pIdentifier ) {
	CFVec3A PrevPos_WS;

	FASSERT( IsCreated() );
	FASSERT_MSG( !_IsBuilding(), "CEntity::Relocate_RotXlatFromScaledMtx_PS_ScaleFromScaledMtx_WS(): Cannot be called while building." );

	PrevPos_WS = m_MtxToWorld.m_vPos;

	if( fmath_Abs( 1.0f - fScaleOfMtx ) < 0.001f ) {
		// Provided scale is pretty close to 1.0f...

		m_MtxToParent = *pMtx_PS;
		m_fScaleToWorld = 1.0f;
	} else {
		// Provided scale isn't close to 1.0f...

		f32 fOOScaleOfMtx;
		CFVec3A ScaleVec;

		if( fScaleOfMtx == 0.0f ) {
			// Caller wishes us to compute the matrice's scale...
			fOOScaleOfMtx = pMtx_PS->m_vRight.InvMag();
			m_fScaleToWorld = fmath_Inv( fOOScaleOfMtx );
		} else {
			fOOScaleOfMtx = fmath_Inv( fScaleOfMtx );
			m_fScaleToWorld = fScaleOfMtx;
		}

		ScaleVec.Set( fOOScaleOfMtx );

		m_MtxToParent.m_vRight.Mul( pMtx_PS->m_vRight, ScaleVec );
		m_MtxToParent.m_vUp.Mul( pMtx_PS->m_vUp, ScaleVec );
		m_MtxToParent.m_vFront.Mul( pMtx_PS->m_vFront, ScaleVec );
		m_MtxToParent.m_vPos = pMtx_PS->m_vPos;
	}

	_Compute_MtxToWorld_And_ScaleToParent_From_Modified_MtxToParent_And_ScaleToWorld();

	_RelocatePostamble( &PrevPos_WS, bEnableTripping, pIdentifier );
}


// Modified:
//   - Rotation from *pMtx_PS
//   - Translation from *pMtx_PS
//   - Scale from fNewEntityScale_PS
//
// Pass 0 for fScaleOfMtx to have this function compute the scale of *pMtx_PS.
// Otherwise, *pMtx_PS have have a scale equal to fScaleOfMtx.
// fScaleOfMtx is not used when computing the entity's new scale.
void CEntity::Relocate_RotXlatFromScaledMtx_PS_NewScale_PS( const CFMtx43A *pMtx_PS, f32 fScaleOfMtx, f32 fNewEntityScale_PS, BOOL bEnableTripping, void *pIdentifier ) {
	CFVec3A PrevPos_WS;

	FASSERT( IsCreated() );
	FASSERT_MSG( !_IsBuilding(), "CEntity::Relocate_RotXlatFromScaledMtx_PS_NewScale_PS(): Cannot be called while building." );

	PrevPos_WS = m_MtxToWorld.m_vPos;

	if( fmath_Abs( 1.0f - fScaleOfMtx ) < 0.001f ) {
		// Provided scale is pretty close to 1.0f...

		m_MtxToParent = *pMtx_PS;
	} else {
		// Provided scale isn't close to 1.0f...

		f32 fOOScaleOfMtx;
		CFVec3A ScaleVec;

		if( fScaleOfMtx == 0.0f ) {
			// Caller wishes us to compute the matrice's scale...
			fOOScaleOfMtx = pMtx_PS->m_vRight.InvMag();
		} else {
			fOOScaleOfMtx = fmath_Inv( fScaleOfMtx );
		}

		ScaleVec.Set( fOOScaleOfMtx );

		m_MtxToParent.m_vRight.Mul( pMtx_PS->m_vRight, ScaleVec );
		m_MtxToParent.m_vUp.Mul( pMtx_PS->m_vUp, ScaleVec );
		m_MtxToParent.m_vFront.Mul( pMtx_PS->m_vFront, ScaleVec );
		m_MtxToParent.m_vPos = pMtx_PS->m_vPos;
	}

	m_fScaleToParent = fNewEntityScale_PS;

	_Compute_MtxToWorld_And_ScaleToWorld_From_Modified_MtxToParent_And_ScaleToParent();

	_RelocatePostamble( &PrevPos_WS, bEnableTripping, pIdentifier );
}


// Modified:
//   - Rotation from *pMtx_PS
//   - Translation from *pMtx_PS
//   - Scale from fNewEntityScale_WS
//
// Pass 0 for fScaleOfMtx to have this function compute the scale of *pMtx_PS.
// Otherwise, *pMtx_PS have have a scale equal to fScaleOfMtx.
// fScaleOfMtx is not used when computing the entity's new scale.
void CEntity::Relocate_RotXlatFromScaledMtx_PS_NewScale_WS( const CFMtx43A *pMtx_PS, f32 fScaleOfMtx, f32 fNewEntityScale_WS, BOOL bEnableTripping, void *pIdentifier ) {
	CFVec3A PrevPos_WS;

	FASSERT( IsCreated() );
	FASSERT_MSG( !_IsBuilding(), "CEntity::Relocate_RotXlatFromScaledMtx_PS_NewScale_WS(): Cannot be called while building." );

	PrevPos_WS = m_MtxToWorld.m_vPos;

	if( fmath_Abs( 1.0f - fScaleOfMtx ) < 0.001f ) {
		// Provided scale is pretty close to 1.0f...

		m_MtxToParent = *pMtx_PS;
	} else {
		// Provided scale isn't close to 1.0f...

		f32 fOOScaleOfMtx;
		CFVec3A ScaleVec;

		if( fScaleOfMtx == 0.0f ) {
			// Caller wishes us to compute the matrice's scale...
			fOOScaleOfMtx = pMtx_PS->m_vRight.InvMag();
		} else {
			fOOScaleOfMtx = fmath_Inv( fScaleOfMtx );
		}

		ScaleVec.Set( fOOScaleOfMtx );

		m_MtxToParent.m_vRight.Mul( pMtx_PS->m_vRight, ScaleVec );
		m_MtxToParent.m_vUp.Mul( pMtx_PS->m_vUp, ScaleVec );
		m_MtxToParent.m_vFront.Mul( pMtx_PS->m_vFront, ScaleVec );
		m_MtxToParent.m_vPos = pMtx_PS->m_vPos;
	}

	m_fScaleToWorld = fNewEntityScale_WS;

	_Compute_MtxToWorld_And_ScaleToParent_From_Modified_MtxToParent_And_ScaleToWorld();

	_RelocatePostamble( &PrevPos_WS, bEnableTripping, pIdentifier );
}


// Called when the parent's m_MtxToWorld or m_fScaleToWorld has changed.
// Also called when the parent has changed an internal matrix that may be
// driving a child.
void CEntity::RelocateAllChildren( BOOL bEnableTripping ) {
	CEntity *pChildEntity;
	CFVec3A PrevPos_WS;

	for( pChildEntity=GetFirstChild(); pChildEntity; pChildEntity=GetNextChild(pChildEntity) ) {
		PrevPos_WS = pChildEntity->m_MtxToWorld.m_vPos;

		if( *pChildEntity->m_pParentMtxToWorld != pChildEntity->m_PreviousParentMtxToWorld ) {
			pChildEntity->m_PreviousParentMtxToWorld = *pChildEntity->m_pParentMtxToWorld;

			pChildEntity->_Compute_ChildMtxAndScale();

			// Let the class know this entity has been relocated...
			pChildEntity->ClassHierarchyRelocated( NULL );

			// Trigger any tripwires...
 			if( pChildEntity->m_nEntityFlags & ENTITY_FLAG_TRIPPER ) {
				// We can trigger tripwires...

				if( CTripwire::GetTripwireCount() ) {
					// There's at least one tripwire...

					pChildEntity->_CheckMovedEntityAgainstTripwires( &PrevPos_WS, &pChildEntity->m_MtxToWorld.m_vPos );
				}
			}

			// Let the class's children know their parent has been relocated...
			pChildEntity->RelocateAllChildren( bEnableTripping );
		}
	}
}



// For all Attach_*() functions:
//
// If bInheritParentScale is TRUE, the child entity's m_fScaleToParent
// will remain constant as its parent's world space scale changes. The
// net result is that the child's world space scale will change as its
// parent's world space scale changes.
//
// If bInheritParentScale is FALSE, the child entity's m_fScaleToWorld
// will remain constant as its parent's world space scale changes.



// Attaches the entity to the specified parent entity and its optional bone.
// Neither the world space position or world space scale of the child
// entity is changed. Tripwires will not be tripped.
void CEntity::Attach_ToParent_WS( CEntity *pParentEntity, cchar *pszAttachBoneName, BOOL bInheritParentScale ) {
	FASSERT( IsCreated() );
	FASSERT( pParentEntity==NULL || pParentEntity->IsCreated() );

	if( m_pParentEntity ) {
		// Detach ourselves from our current parent...
		DetachFromParent();
	}

	// Attach ourselves to our new parent...
	if( pParentEntity ) {
		if( bInheritParentScale ) {
			FMATH_SETBITMASK( m_nEntityFlags, ENTITY_FLAG_INHERIT_PARENT_SCALE );
		}

		flinklist_AddTail( &pParentEntity->m_ChildEntityRoot, this );
		m_pParentEntity = pParentEntity;

		m_pParentMtxToWorld = pParentEntity->ClassHierarchyAttachChild( this, pszAttachBoneName );

		if( m_pParentMtxToWorld == NULL ) {
			// Attach to parent's m_MtxToWorld...

			m_pParentMtxToWorld = &pParentEntity->m_MtxToWorld;
			FMATH_SETBITMASK( m_nEntityFlags, ENTITY_FLAG_PARENTMTXTOWORLD_POINTS_TO_MTXTOWORLD );
		} else {
			// Attach to parent's custom matrix (which must contain full scale information within it)...

			m_pszAttachBone = pszAttachBoneName;
		}

		m_PreviousParentMtxToWorld = *m_pParentMtxToWorld;

		_Compute_MtxToParent_And_ScaleToParent_From_Modified_MtxToWorld_And_ScaleToWorld();
	}
}


// Attaches the entity to the specified parent entity and its optional bone.
// The function uses the child's world space orientation as its new parent-space
// orientation. It also uses the child's world space scale as its new parent-space
// scale. The child's world space orientation and scale are modified accordingly.
// Tripwires will not be tripped.
void CEntity::Attach_ToParent_PS( CEntity *pParentEntity, cchar *pszAttachBoneName, BOOL bInheritParentScale  ) {
	FASSERT( IsCreated() );
	FASSERT( pParentEntity==NULL || pParentEntity->IsCreated() );

	if( m_pParentEntity ) {
		// Detach ourselves from our current parent...
		DetachFromParent();
	}

	// Attach ourselves to our new parent...
	if( pParentEntity ) {
		if( bInheritParentScale ) {
			FMATH_SETBITMASK( m_nEntityFlags, ENTITY_FLAG_INHERIT_PARENT_SCALE );
		}

		flinklist_AddTail( &pParentEntity->m_ChildEntityRoot, this );
		m_pParentEntity = pParentEntity;

		m_pParentMtxToWorld = pParentEntity->ClassHierarchyAttachChild( this, pszAttachBoneName );

		if( m_pParentMtxToWorld == NULL ) {
			// Attach to parent's m_MtxToWorld...

			m_pParentMtxToWorld = &pParentEntity->m_MtxToWorld;
			FMATH_SETBITMASK( m_nEntityFlags, ENTITY_FLAG_PARENTMTXTOWORLD_POINTS_TO_MTXTOWORLD );
		} else {
			// Attach to parent's custom matrix (which must contain full scale information within it)...

			m_pszAttachBone = pszAttachBoneName;
		}

		m_PreviousParentMtxToWorld = *m_pParentMtxToWorld;
		m_MtxToParent = m_MtxToWorld;
		m_fScaleToParent = m_fScaleToWorld;

		_Compute_MtxToWorld_And_ScaleToWorld_From_Modified_MtxToParent_And_ScaleToParent();

		// Let the class know this entity has been relocated...
		ClassHierarchyRelocated( NULL );

		// Let the class's children know their parent has been relocated...
		RelocateAllChildren( FALSE );
	}
}


// Attaches the entity to the specified parent entity and its optional bone.
// pUnitMtx_PS specifies the orientation of the child relative to its parent's
// frame of reference. The child's world space orientation is modified accordingly.
// Tripwires will not be tripped.
void CEntity::Attach_UnitMtxToParent_PS( CEntity *pParentEntity, cchar *pszAttachBoneName, const CFMtx43A *pUnitMtx_PS, BOOL bInheritParentScale  ) {
	FASSERT( IsCreated() );
	FASSERT( pParentEntity==NULL || pParentEntity->IsCreated() );

	if( m_pParentEntity ) {
		// Detach ourselves from our current parent...
		DetachFromParent();
	}

	// Attach ourselves to our new parent...
	if( pParentEntity ) {
		if( bInheritParentScale ) {
			FMATH_SETBITMASK( m_nEntityFlags, ENTITY_FLAG_INHERIT_PARENT_SCALE );
		}

		flinklist_AddTail( &pParentEntity->m_ChildEntityRoot, this );
		m_pParentEntity = pParentEntity;

		m_pParentMtxToWorld = pParentEntity->ClassHierarchyAttachChild( this, pszAttachBoneName );

		if( m_pParentMtxToWorld == NULL ) {
			// Attach to parent's m_MtxToWorld...

			m_pParentMtxToWorld = &pParentEntity->m_MtxToWorld;
			FMATH_SETBITMASK( m_nEntityFlags, ENTITY_FLAG_PARENTMTXTOWORLD_POINTS_TO_MTXTOWORLD );
		} else {
			// Attach to parent's custom matrix (which must contain full scale information within it)...

			m_pszAttachBone = pszAttachBoneName;
		}

		m_PreviousParentMtxToWorld = *m_pParentMtxToWorld;
		m_MtxToParent = *pUnitMtx_PS;

		_Compute_MtxToWorld_And_ScaleToParent_From_Modified_MtxToParent_And_ScaleToWorld();

		// Let the class know this entity has been relocated...
		ClassHierarchyRelocated( NULL );

		// Let the class's children know their parent has been relocated...
		RelocateAllChildren( FALSE );
	}
}


// Attaches the entity to the specified parent entity and its optional bone.
// pUnitMtx_PS specifies the orientation of the child relative to its parent's
// frame of reference. The child's world space orientation is modified accordingly.
// The child's world space scale is set to fNewEntityScale_WS.
// Tripwires will not be tripped.
void CEntity::Attach_UnitMtxToParent_PS_NewScale_WS( CEntity *pParentEntity, cchar *pszAttachBoneName, const CFMtx43A *pUnitMtx_PS, f32 fNewEntityScale_WS, BOOL bInheritParentScale  ) {
	FASSERT( IsCreated() );
	FASSERT( pParentEntity==NULL || pParentEntity->IsCreated() );

	if( m_pParentEntity ) {
		// Detach ourselves from our current parent...
		DetachFromParent();
	}

	// Attach ourselves to our new parent...
	if( pParentEntity ) {
		if( bInheritParentScale ) {
			FMATH_SETBITMASK( m_nEntityFlags, ENTITY_FLAG_INHERIT_PARENT_SCALE );
		}

		flinklist_AddTail( &pParentEntity->m_ChildEntityRoot, this );
		m_pParentEntity = pParentEntity;

		m_pParentMtxToWorld = pParentEntity->ClassHierarchyAttachChild( this, pszAttachBoneName );

		if( m_pParentMtxToWorld == NULL ) {
			// Attach to parent's m_MtxToWorld...

			m_pParentMtxToWorld = &pParentEntity->m_MtxToWorld;
			FMATH_SETBITMASK( m_nEntityFlags, ENTITY_FLAG_PARENTMTXTOWORLD_POINTS_TO_MTXTOWORLD );
		} else {
			// Attach to parent's custom matrix (which must contain full scale information within it)...

			m_pszAttachBone = pszAttachBoneName;
		}

		m_PreviousParentMtxToWorld = *m_pParentMtxToWorld;
		m_MtxToParent = *pUnitMtx_PS;
		m_fScaleToWorld = fNewEntityScale_WS;

		_Compute_MtxToWorld_And_ScaleToParent_From_Modified_MtxToParent_And_ScaleToWorld();

		// Let the class know this entity has been relocated...
		ClassHierarchyRelocated( NULL );

		// Let the class's children know their parent has been relocated...
		RelocateAllChildren( FALSE );
	}
}


// Attaches the entity to the specified parent entity and its optional bone.
// pUnitMtx_PS specifies the orientation of the child relative to its parent's
// frame of reference. The child's world space orientation is modified accordingly.
// The child's parent space scale is set to fNewEntityScale_PS and its world space
// scale is updated accordingly. Tripwires will not be tripped.
void CEntity::Attach_UnitMtxToParent_PS_NewScale_PS( CEntity *pParentEntity, cchar *pszAttachBoneName, const CFMtx43A *pUnitMtx_PS, f32 fNewEntityScale_PS, BOOL bInheritParentScale  ) {
	FASSERT( IsCreated() );
	FASSERT( pParentEntity==NULL || pParentEntity->IsCreated() );

	if( m_pParentEntity ) {
		// Detach ourselves from our current parent...
		DetachFromParent();
	}

	// Attach ourselves to our new parent...
	if( pParentEntity ) {
		if( bInheritParentScale ) {
			FMATH_SETBITMASK( m_nEntityFlags, ENTITY_FLAG_INHERIT_PARENT_SCALE );
		}

		flinklist_AddTail( &pParentEntity->m_ChildEntityRoot, this );
		m_pParentEntity = pParentEntity;

		m_pParentMtxToWorld = pParentEntity->ClassHierarchyAttachChild( this, pszAttachBoneName );

		if( m_pParentMtxToWorld == NULL ) {
			// Attach to parent's m_MtxToWorld...

			m_pParentMtxToWorld = &pParentEntity->m_MtxToWorld;
			FMATH_SETBITMASK( m_nEntityFlags, ENTITY_FLAG_PARENTMTXTOWORLD_POINTS_TO_MTXTOWORLD );
		} else {
			// Attach to parent's custom matrix (which must contain full scale information within it)...

			m_pszAttachBone = pszAttachBoneName;
		}

		m_PreviousParentMtxToWorld = *m_pParentMtxToWorld;
		m_MtxToParent = *pUnitMtx_PS;
		m_fScaleToParent = fNewEntityScale_PS;

		_Compute_MtxToWorld_And_ScaleToWorld_From_Modified_MtxToParent_And_ScaleToParent();

		// Let the class know this entity has been relocated...
		ClassHierarchyRelocated( NULL );

		// Let the class's children know their parent has been relocated...
		RelocateAllChildren( FALSE );
	}
}


void CEntity::DetachFromParent( void ) {
	FASSERT( IsCreated() );

	if( m_pParentEntity ) {
		m_pParentEntity->ClassHierarchyDetachChild( this );

		flinklist_Remove( &m_pParentEntity->m_ChildEntityRoot, this );
		m_pParentEntity = NULL;
		m_pParentMtxToWorld = NULL;
		m_pszAttachBone = NULL;

		// Compute m_MtxToParent...
		m_MtxToParent = m_MtxToWorld;
		m_fScaleToParent = m_fScaleToWorld;

		FMATH_CLEARBITMASK( m_nEntityFlags, ENTITY_FLAG_PARENTMTXTOWORLD_POINTS_TO_MTXTOWORLD | ENTITY_FLAG_INHERIT_PARENT_SCALE );
	}
}


void CEntity::DetachAllChildren( void ) {
	CEntity *pChildEntity;

	FASSERT( IsCreated() );

	while( pChildEntity = GetLastChild() ) {
		pChildEntity->DetachFromParent();
	}
}


void CEntity::SetTripwireBoundingSphere_MS( const CFSphereA *pSphere_MS ) {
	FASSERT( IsCreated() );

	if( m_pTripwire ) {
		m_pTripwire->m_BoundingSphere_MS = *pSphere_MS;

		m_MtxToWorld.MulPoint( m_pTripwire->m_BoundingSphere_WS.m_Pos, m_pTripwire->m_BoundingSphere_MS.m_Pos );
		m_pTripwire->m_BoundingSphere_WS.m_fRadius = m_fScaleToWorld * m_pTripwire->m_BoundingSphere_MS.m_fRadius;
	}
}


void CEntity::_CheckMovedEntityAgainstTripwires( const CFVec3A *pPrevPos_WS, const CFVec3A *pNewPos_WS ) {
	CEntity *pTripwireEntity;
	CTripwire *pTripwire;
	CFSphereA MovementSphere;
	f32 fDist2, fRadiusSum;
	u32 nTripwireCollResultFlags;

	MovementSphere.m_Pos.Add( *pPrevPos_WS, *pNewPos_WS );
	MovementSphere.m_Pos.Mul( CFVec3A::m_HalfVec );
	MovementSphere.m_fRadius = 0.5f * pPrevPos_WS->Dist( *pNewPos_WS );

	for( pTripwire=CTripwire::TripwireList_GetHead(); pTripwire; pTripwire=pTripwire->TripwireList_GetNext() ) {
		pTripwireEntity = pTripwire->m_pOwnerEntity;

		if( pTripwire->m_nEventFlags & CEntity::TRIPWIRE_EVENTFLAG_INSIDE_EVENT ) {
			_EntityIsOutsideTripwire( pTripwireEntity );
		}

		if( !pTripwireEntity->_DoesTripperPassTripwireFilterTest( this ) ) {
			// The tripper (*this) doesn't pass the tripwire filter test...
			continue;
		}

		fDist2 = MovementSphere.m_Pos.DistSq( pTripwire->m_BoundingSphere_WS.m_Pos );
		fRadiusSum = MovementSphere.m_fRadius + pTripwire->m_BoundingSphere_WS.m_fRadius;

		if( fDist2 < (fRadiusSum * fRadiusSum) ) {
			nTripwireCollResultFlags = pTripwireEntity->TripwireCollisionTest( pPrevPos_WS, pNewPos_WS );

			//Check to see if we were killed by this tripwire
			if( nTripwireCollResultFlags & TRIPWIRE_COLLFLAG_KILL_NOW ) {
				Die( !!( nTripwireCollResultFlags & TRIPWIRE_COLLFLAG_SPAWN_DEATH_EFFECTS ), FALSE ); 
				break;
			}

			if( nTripwireCollResultFlags & TRIPWIRE_COLLFLAG_NEWPOS_INSIDE ) {
				// Entity is inside the tripwire...

				if( pTripwire->m_nEventFlags & CEntity::TRIPWIRE_EVENTFLAG_INSIDE_EVENT ) {
					if( _EntityIsInsideTripwire( pTripwireEntity ) ) {
						//added tripper to tripwires list
					}
				}
			}

			if( nTripwireCollResultFlags & TRIPWIRE_COLLFLAG_ENTER_EVENT ) {
				// Trigger a tripwire enter event...

				if( pTripwire->m_nEventFlags & CEntity::TRIPWIRE_EVENTFLAG_ENTER_EVENT ) {
					SCRIPT_MESSAGE( "TRIPWIRE ENTER EVENT tripwire='%s' tripper='%s'", pTripwireEntity->Name() ? pTripwireEntity->Name() : "unknown", Name() ? Name() : "unknown" );
//					DEVPRINTF( ">>>>>>> TRIPWIRE ENTER EVENT (tripwire='%s', tripper='%s') <<<<<<<\n", pTripwireEntity->Name() ? pTripwireEntity->Name() : "unknown", Name() ? Name() : "unknown" );

					if( pTripwire->m_pOwnerEntity->IsCollectable() ) {
						// It's a collectable...
						pTripwire->m_pOwnerEntity->GetCollected( this );
#if FANG_WINGC
					} else if( pTripwireEntity->_CoopHoldTripwireEnter( this ) ) {
						// Local co-op: held (or already fired) until every standing player has arrived.
#endif
					} else {
						// It's not a collectable...
						pTripwire->OnEnter( this );

						// Open attached door, if any...
						if( pTripwire->m_pDoorToOpen ) {
							pTripwire->m_pDoorToOpen->GotoPos( 1, CDoorEntity::GOTOREASON_DESTINATION );
						}

						if( pTripwire->m_nTripwireTriggerMode == TRIPWIRE_TRIGGER_MODE_ONCE ) {
							pTripwire->m_pOwnerEntity->RemoveFromWorld();
						}
					}
				}
			}

			if( nTripwireCollResultFlags & TRIPWIRE_COLLFLAG_EXIT_EVENT ) {
				// Trigger a tripwire exit event...

				if( pTripwire->m_nEventFlags & CEntity::TRIPWIRE_EVENTFLAG_EXIT_EVENT ) {
					DEVPRINTF( ">>>>>>> TRIPWIRE EXIT EVENT (tripwire='%s', tripper='%s') <<<<<<<\n", pTripwireEntity->Name() ? pTripwireEntity->Name() : "unknown", Name() ? Name() : "unknown" );
					pTripwire->OnExit( this );
				}
			}
		}
	}
}


#if FANG_WINGC
// Local co-op: checkpoint enters run for the first standing player. Other shared events (an
// elevator start, a buddy's "follow me", a script beat) wait for every standing player, in any order; a
// player who is down is not waited for. Arrival is sticky, so a thin trigger the players cross one
// at a time still releases. Collectables and kill volumes are never held; an attached door still
// opens for whoever arrives first. Audited one-shot encounters also accept a
// nearby group after one real crossing; transport and unlisted events stay strict.
#define _COOP_MAX_HELD_TRIPWIRES	16

typedef struct {
	CEntity *pTripwireEntity;
	u32 nArrivedMask;		// players (bit per player index) who have been inside
	CEntity *pEncounterAnchor;	// most recent real crossing, never inferred from proximity
	CFVec3A vEncounterAnchorPos;
} _CoopHeldTripwire_t;

static _CoopHeldTripwire_t _aCoopHeldTripwires[_COOP_MAX_HELD_TRIPWIRES];
static u32 _nCoopHeldTripwires;
static CEntity *_apCoopPreviousPlayers[MAX_PLAYERS];
static CFVec3A _avCoopPreviousPositions[MAX_PLAYERS];
static const CEntity *_apCoopWaitMessagePlayers[MAX_PLAYERS];
static u64 _anCoopWaitMessageStartTicks[MAX_PLAYERS];

// Checkpoint trigger families present in the retail campaign scripts. Require
// the whole numeric suffix so objectives such as "saveus" are not checkpoints.
static BOOL _CoopCheckpointTripwire( cchar *pszName ) {
	if( !pszName ) {
		return FALSE;
	}
	static cchar *apszPrefixes[] = {
		"save", "check", "checkpoint", "checkpoint_trigger",
		"trigger_checkpoint", "trig_checkpoint", "hall_checkpoint", "savepoint"
	};
	for( u32 i=0; i < sizeof(apszPrefixes)/sizeof(apszPrefixes[0]); i++ ) {
		const s32 nPrefix = fclib_strlen( apszPrefixes[i] );
		if( fclib_strnicmp( pszName, apszPrefixes[i], nPrefix ) ) {
			continue;
		}
		cchar *pSuffix = pszName + nPrefix;
		if( *pSuffix < '0' || *pSuffix > '9' ) {
			continue;
		}
		while( *pSuffix >= '0' && *pSuffix <= '9' ) {
			++pSuffix;
		}
		if( !*pSuffix ) {
			return TRUE;
		}
	}
	// Additional checkpoint enters in the Mines and research campaign scripts.
	return !fclib_stricmp( pszName, "checksnaptrigger" ) ||
		!fclib_stricmp( pszName, "bonusbuddysave" ) ||
		!fclib_stricmp( pszName, "save_hall1" ) ||
		!fclib_stricmp( pszName, "m3check01trigger" ) ||
		!fclib_stricmp( pszName, "m3check02trigger" ) ||
		!fclib_stricmp( pszName, "m3check03trigger" ) ||
		!fclib_stricmp( pszName, "m3check04trigger" ) ||
		!fclib_stricmp( pszName, "m3check05trigger" );
}

// Retail Morbot floor pads operate local structures. Their tiny footprints are
// individual interactions, rather than team rendezvous or mission exits.
static BOOL _CoopFloorSwitchTripwire( cchar *pszName ) {
	if( !pszName || Level_nLoadedIndex < 0 || Level_nLoadedIndex >= LEVEL_SINGLE_PLAYER_COUNT ) return FALSE;
	cchar *pszWorld = Level_aInfo[Level_nLoadedIndex].pszWorldResName;
	if( !fclib_stricmp(pszWorld,"WERMmorbot1") ) {
		return !fclib_stricmp(pszName,"circtrig1") || !fclib_stricmp(pszName,"circtrig2") ||
			!fclib_stricmp(pszName,"fliptrig");
	}
	// I, Predator's beam opens the local bridge and starts its ambush. It
	// neither carries the team nor advances to another mission.
	if( !fclib_stricmp(pszWorld,"WERMmorbot2") ) {
		return !fclib_stricmp(pszName,"bridge1trig");
	}
	// Reactor pressure pads must work from either side of a timed door.
	// Keep exact world/name lists: hallway, lift and scene triggers also use
	// "trig" names but still need the team to gather.
	if( !fclib_stricmp(pszWorld,"WERRreactr1") ) {
		static cchar *apszPads[] = {"door1_triga","door1_trigb",
			"door2_triga","door2_trigb","door3_triga","door3_trigb",
			"door4_triga","bridge1_triga","orb_trig1","orb_trig2"};
		for( u32 i=0; i<sizeof(apszPads)/sizeof(apszPads[0]); ++i ) {
			if( !fclib_stricmp(pszName,apszPads[i]) ) return TRUE;
		}
	}
	if( !fclib_stricmp(pszWorld,"WERRreactr2") ) {
		static cchar *apszPads[] = {"door1_triga","door1_trigb",
			"door2_triga","door2_trigb","bridge1_triga","orb_trig1"};
		for( u32 i=0; i<sizeof(apszPads)/sizeof(apszPads[0]); ++i ) {
			if( !fclib_stricmp(pszName,apszPads[i]) ) return TRUE;
		}
	}
	return FALSE;
}

static BOOL _CoopTripwireGateActive( void ) {
	// Test aid: MA_PORT_COOP_HOLD=0 (-coop-hold off) lets player 1 trip events alone, so a test
	// window whose partners can't move still reaches scripted scenes.
	static int _nHold = -1;
	if( _nHold < 0 ) {
		char szHold[8];
		_nHold = (GetEnvironmentVariableA( "MA_PORT_COOP_HOLD", szHold, sizeof(szHold) ) && szHold[0] == '0') ? 0 : 1;
	}
	return _nHold && MultiplayerMgr.IsSinglePlayer() && (CPlayer::m_nPlayerCount >= 2);
}

// Players whose current entity is up: in the world and not dead or dying.
static u32 _CoopStandingPlayerMask( void ) {
	u32 nMask = 0;
	for( s32 i=0; i < CPlayer::m_nPlayerCount; i++ ) {
		CEntity *pEntity = Player_aPlayer[i].m_pEntityCurrent;
		if( !pEntity || !pEntity->IsInWorld() ) {
			continue;
		}
		if( (pEntity->TypeBits() & ENTITY_BIT_BOT) && ((CBot *)pEntity)->IsDeadOrDying() ) {
			continue;
		}
		nMask |= (1 << i);
	}
	return nMask;
}

static s32 _CoopPlayerOfEntity( const CEntity *pEntity ) {
	for( s32 i=0; i < CPlayer::m_nPlayerCount; i++ ) {
		if( Player_aPlayer[i].m_pEntityCurrent == pEntity ) {
			return i;
		}
	}
	return -1;
}

// Exact retail branches audited in docs/coop-encounter-trigger-list-20261006.json.
// Names such as "bridge" can mean either an ambush or transport; never use
// a name prefix or expand a volume to guess which policy is safe.
static BOOL _CoopNearbyEncounterTripwire( const CEntity *pTripwire, const CTripwire *pData ) {
	if( !pTripwire || !pTripwire->IsTripwire() || !pTripwire->Name() ||
		Level_nLoadedIndex < 0 || Level_nLoadedIndex >= Level_nCount ||
		Level_aInfo[Level_nLoadedIndex].nLevel < 0 ||
		Level_aInfo[Level_nLoadedIndex].nLevel >= LEVEL_SINGLE_PLAYER_COUNT ||
		pData->m_nTripwireTriggerMode != CEntity::TRIPWIRE_TRIGGER_MODE_ONCE ||
		pData->m_pszTripwireFilterEntityName ||
		pData->m_eKillMode != CTripwire::TRIPWIRE_KILLMODE_NONE ) return FALSE;
	static const struct { cchar *pszWorld; cchar *pszTrigger; } aEvents[] = {
		{ "WEDMmines01", "seetrig1" },
		{ "WEDMmines02", "trigger_stopmusic1" },
		{ "WEDMmines03", "bridgetrigger" },
		{ "WEDMmines03", "snarqtrigger" },
		{ "WEWJjourn01", "trig_pipe" },
		{ "WEWJjourn02", "trig_plank" },
		{ "WEWJjourn02", "trig_snipe" },
		{ "WEWJjourn02", "trig_zip" },
		{ "WEWCcomm_01", "trig_titan" },
		{ "WEWCcomm_01", "trig_hole" },
		{ "WEWRresrch4", "trigger_spews" },
		{ "WERMmorbot1", "btrip01" },
		{ "WERMmorbot1", "trig_leech" },
		{ "WERMmorbot1", "trig_flames" },
		{ "WERMmorbot1", "trig_pred" },
		{ "WEMCcity_01", "startgrunttrigger" },
		{ "WEMCcity_03", "gruntgate_trigger" },
		{ "WEMCcity_03", "backalley_trigger" },
		{ "WEMCcity_03", "gentitan_trigger02" },
		{ "WEMCcity_05", "musicstarttrigger01" },
		{ "WEMCcity_05", "titan1trigger" },
		{ "WEMCcity_05", "titan2trigger" },
		{ "WEMCcity_05", "hutgrunt1trigger" },
		{ "WEMCcity_05", "hutgrunt2trigger" },
		{ "WEMCcity_05", "bgrunt1trigger" },
		{ "WEMCcity_05", "powertrigger" },
		{ "WEMCcity_05", "MainStreetTrigger" },
		{ "WEMCcity_05", "tunneltrigger" },
		{ "WECRruins01", "addsomegrunts_trigger" },
		{ "WECRruins01", "trippie" },
		{ "WECRruins01", "jump" },
		{ "WECRruins01", "bridge_trig" },
		{ "WECRruins01", "pill_backup" },
		{ "WESSstatn01", "trig_jumper" },
		{ "WESSstatn01", "trig_jumper2" },
		{ "WESSstatn01", "trig_sting" },
		{ "WESSstatn02", "trig_jump2" },
	};
	cchar *pszWorld = Level_aInfo[Level_nLoadedIndex].pszWorldResName;
	for( u32 i=0; i < sizeof(aEvents)/sizeof(aEvents[0]); ++i ) {
		if( !fclib_stricmp(pszWorld,aEvents[i].pszWorld) &&
			!fclib_stricmp(pTripwire->Name(),aEvents[i].pszTrigger) ) return TRUE;
	}
	return FALSE;
}

// A teleport that intersects an earlier gate must not anchor that gate at
// its distant endpoint. Inside endpoints and short tracked crossings are real
// anchors; the caller has already confirmed this tripwire's enter/crossing.
static BOOL _CoopEncounterCrossingAnchor( CEntity *pTripwire, CEntity *pPlayer ) {
	const s32 nPlayer = _CoopPlayerOfEntity(pPlayer);
	if( nPlayer < 0 || Player_aPlayer[nPlayer].m_pEntityOrig != pPlayer ) return FALSE;
	const CFVec3A &vPos = pPlayer->MtxToWorld()->m_vPos;
	return pTripwire->TripwireContainsPoint(vPos) ||
		(_apCoopPreviousPlayers[nPlayer] == pPlayer &&
		 _avCoopPreviousPositions[nPlayer].DistSq(vPos) <= 100.0f);
}

// Whole-group diameter, rather than a chain of nearby partners. Twelve world
// units allows a few bot lengths; four vertically avoids adjacent storeys.
// This supplies readiness only: no fabricated crossings or partner teleports.
static BOOL _CoopEncounterGroupNearby( CEntity *pTripwire, u32 nStanding,
	const _CoopHeldTripwire_t *pHeld, CEntity *pCrossingPlayer=NULL ) {
	if( !nStanding || CBot::m_bCutscenePlaying ) return FALSE;
	BOOL bAtTrigger = FALSE;
	for( s32 i=0; i < CPlayer::m_nPlayerCount; ++i ) {
		if( !(nStanding & (1 << i)) ) continue;
		CEntity *pPlayer = Player_aPlayer[i].m_pEntityCurrent;
		// Remote possession must not make the pilot's group look assembled.
		if( pPlayer != Player_aPlayer[i].m_pEntityOrig ) return FALSE;
		const CFVec3A &vPos = pPlayer->MtxToWorld()->m_vPos;
		if( (pPlayer == pCrossingPlayer && _CoopEncounterCrossingAnchor(pTripwire,pPlayer)) ||
			pTripwire->TripwireContainsPoint(vPos) ||
			(pHeld && pHeld->pEncounterAnchor == pPlayer && vPos.DistSq(pHeld->vEncounterAnchorPos) <= 16.0f) ) {
			bAtTrigger = TRUE;
		}
		for( s32 j=i+1; j < CPlayer::m_nPlayerCount; ++j ) {
			if( !(nStanding & (1 << j)) ) continue;
			const CFVec3A &vOther = Player_aPlayer[j].m_pEntityCurrent->MtxToWorld()->m_vPos;
			if( vPos.DistSq(vOther) > 144.0f || fmath_Abs(vPos.y-vOther.y) > 4.0f ) return FALSE;
		}
	}
	return bAtTrigger;
}

// The retail scripts were written for player 1 as the tripper; use them when they are up.
static CEntity *_CoopReleaseTripper( CEntity *pFallback ) {
	if( _CoopStandingPlayerMask() & 1 ) {
		return Player_aPlayer[0].m_pEntityCurrent;
	}
	return pFallback;
}

u32 CEntity::_CoopPlayersInsideMask( BOOL bCheckCrossings ) {
	u32 nMask = 0;
	for( s32 i=0; i < CPlayer::m_nPlayerCount; i++ ) {
		CEntity *pEntity = Player_aPlayer[i].m_pEntityCurrent;
		if( !pEntity || !pEntity->IsInWorld() ||
			((pEntity->TypeBits() & ENTITY_BIT_BOT) && ((CBot *)pEntity)->IsDeadOrDying()) ) {
			continue;
		}
		const CFVec3A *pPos_WS = &pEntity->MtxToWorld()->m_vPos;
		const CFVec3A *pPrev_WS = pPos_WS;
		// Recheck actual movement if the normal enter notification was missed.
		// Only track the same living original body; possession, revival and large
		// teleports must not claim all the gates along an unrelated path.
		if( bCheckCrossings && _apCoopPreviousPlayers[i] == pEntity &&
			Player_aPlayer[i].m_pEntityOrig == pEntity &&
			_avCoopPreviousPositions[i].DistSq( *pPos_WS ) <= 100.0f ) {
			pPrev_WS = &_avCoopPreviousPositions[i];
		}
		if( TripwireCollisionTest( pPrev_WS, pPos_WS ) &
			(TRIPWIRE_COLLFLAG_NEWPOS_INSIDE | TRIPWIRE_COLLFLAG_ENTER_EVENT | TRIPWIRE_COLLFLAG_EXIT_EVENT) ) {
			nMask |= (1 << i);
		}
	}
	return nMask;
}

void CEntity::_CoopFireTripwireEnter( CEntity *pTripper ) {
	SCRIPT_MESSAGE( "TRIPWIRE ENTER EVENT tripwire='%s' tripper='%s' (co-op: released)", Name() ? Name() : "unknown", pTripper->Name() ? pTripper->Name() : "unknown" );
	DEVPRINTF( "Co-op: tripwire '%s' released.\n", Name() ? Name() : "unknown" );
	m_pTripwire->OnEnter( pTripper );
	if( m_pTripwire->m_pDoorToOpen ) {
		m_pTripwire->m_pDoorToOpen->GotoPos( 1, CDoorEntity::GOTOREASON_DESTINATION );
	}
	if( m_pTripwire->m_nTripwireTriggerMode == TRIPWIRE_TRIGGER_MODE_ONCE ) {
		RemoveFromWorld();
	}
}

// Called for this tripwire's enter event from pTripper. Returns TRUE when the retail handling must
// not run: the event is held for the others, or this arrival completed the team and it was fired.
BOOL CEntity::_CoopHoldTripwireEnter( CEntity *pTripper ) {
	if( !_CoopTripwireGateActive() || (m_pTripwire->m_eKillMode != CTripwire::TRIPWIRE_KILLMODE_NONE) ) {
		return FALSE;
	}
	const s32 nPlayer = _CoopPlayerOfEntity( pTripper );
	if( nPlayer < 0 || Player_aPlayer[nPlayer].m_pEntityOrig != pTripper || m_pTripwire->m_pszTripwireFilterEntityName ) {
		// Possession and entity-specific objectives retain the actual actor and retail timing.
		return FALSE;
	}

	_CoopHeldTripwire_t *pHeld = NULL;
	for( u32 i=0; i < _nCoopHeldTripwires; i++ ) {
		if( _aCoopHeldTripwires[i].pTripwireEntity == this ) {
			pHeld = &_aCoopHeldTripwires[i];
			break;
		}
	}
	if( CBot::m_bCutscenePlaying && nPlayer == game_GetStoryPlayerIndex() ) {
		// The team already gathered for the scene. Its scripted actor must be
		// able to cross later triggers while spectators' controls are disabled.
		if( pHeld ) {
			*pHeld = _aCoopHeldTripwires[--_nCoopHeldTripwires];
		}
		return FALSE;
	}
	const u32 nStanding = _CoopStandingPlayerMask();
	if( !(nStanding & (1 << nPlayer)) ) {
		// A dying body's final movement cannot advance the campaign.
		return TRUE;
	}
	const BOOL bFloorSwitch = _CoopFloorSwitchTripwire(Name());
	if( bFloorSwitch || _CoopCheckpointTripwire( Name() ) ) {
		if( pHeld ) {
			*pHeld = _aCoopHeldTripwires[--_nCoopHeldTripwires];
		}
		DEVPRINTF( "Co-op: %s '%s' reached by player %d; no partner wait.\n",
			bFloorSwitch ? "floor switch" : "checkpoint trigger", Name(), nPlayer + 1 );
		_CoopFireTripwireEnter( bFloorSwitch ? pTripper : _CoopReleaseTripper(pTripper) );
		return TRUE;
	}
	u32 nArrived = (1 << nPlayer) | _CoopPlayersInsideMask() | (pHeld ? pHeld->nArrivedMask : 0);
	const BOOL bNearbyEncounter = _CoopNearbyEncounterTripwire(this,m_pTripwire) &&
		_CoopEncounterGroupNearby(this,nStanding,pHeld,pTripper);
	if( (nArrived & nStanding) == nStanding || bNearbyEncounter ) {
		// Everyone crossed, or this audited encounter has a nearby living group.
		if( bNearbyEncounter ) DEVPRINTF("Co-op: encounter '%s' released for nearby group.\n",Name());
		if( pHeld ) {
			*pHeld = _aCoopHeldTripwires[--_nCoopHeldTripwires];
		}
		_CoopFireTripwireEnter( _CoopReleaseTripper( pTripper ) );
		return TRUE;
	}
	if( !pHeld ) {
		if( _nCoopHeldTripwires >= _COOP_MAX_HELD_TRIPWIRES ) {
			return FALSE;
		}
		pHeld = &_aCoopHeldTripwires[_nCoopHeldTripwires++];
		pHeld->pTripwireEntity = this;
		pHeld->pEncounterAnchor = NULL;
		DEVPRINTF( "Co-op: tripwire '%s' waits for every player (player %d arrived first).\n", Name() ? Name() : "unknown", nPlayer + 1 );
		if( m_pTripwire->m_pDoorToOpen ) {
			m_pTripwire->m_pDoorToOpen->GotoPos( 1, CDoorEntity::GOTOREASON_DESTINATION );
		}
	}
	pHeld->nArrivedMask = nArrived;
	pHeld->pEncounterAnchor = _CoopEncounterCrossingAnchor(this,pTripper) ? pTripper : NULL;
	pHeld->vEncounterAnchorPos = pTripper->MtxToWorld()->m_vPos;
	return TRUE;
}

void CEntity::CoopTripwireWork( void ) {
	if( !_CoopTripwireGateActive() ) {
		CoopTripwireReset();
		return;
	}
	const u32 nStanding = _CoopStandingPlayerMask();
	for( s32 i = (s32)_nCoopHeldTripwires - 1; i >= 0; --i ) {
		_CoopHeldTripwire_t *pHeld = &_aCoopHeldTripwires[i];
		CEntity *pTripwireEntity = pHeld->pTripwireEntity;
		if( !pTripwireEntity->IsTripwire() || !pTripwireEntity->IsInWorld() || !pTripwireEntity->IsTripwireArmed() ) {
			*pHeld = _aCoopHeldTripwires[--_nCoopHeldTripwires];
			continue;
		}
		const u32 nReached = pTripwireEntity->_CoopPlayersInsideMask( TRUE );
		const u32 nRecovered = nReached & ~pHeld->nArrivedMask;
		if( nRecovered ) {
			DEVPRINTF( "Co-op: tripwire '%s' recovered player arrival mask 0x%x from position/crossing.\n", pTripwireEntity->Name() ? pTripwireEntity->Name() : "unknown", nRecovered );
		}
		pHeld->nArrivedMask |= nReached;
		if( nReached && _CoopNearbyEncounterTripwire(pTripwireEntity,pTripwireEntity->m_pTripwire) ) {
			for( s32 nPlayer=0; nPlayer < CPlayer::m_nPlayerCount; ++nPlayer ) {
				CEntity *pPlayer = Player_aPlayer[nPlayer].m_pEntityCurrent;
				if( (nReached & (1 << nPlayer)) && _CoopEncounterCrossingAnchor(pTripwireEntity,pPlayer) ) {
					pHeld->pEncounterAnchor = pPlayer;
					pHeld->vEncounterAnchorPos = pPlayer->MtxToWorld()->m_vPos;
					break;
				}
			}
		}
		const BOOL bNearbyEncounter = _CoopNearbyEncounterTripwire(pTripwireEntity,pTripwireEntity->m_pTripwire) &&
			_CoopEncounterGroupNearby(pTripwireEntity,nStanding,pHeld);
		if( nStanding && (((pHeld->nArrivedMask & nStanding) == nStanding) || bNearbyEncounter) ) {
			if( bNearbyEncounter ) DEVPRINTF("Co-op: held encounter '%s' released for nearby group.\n",pTripwireEntity->Name());
			CEntity *pTripper = NULL;
			for( s32 nPlayer=0; nPlayer < CPlayer::m_nPlayerCount && !pTripper; nPlayer++ ) {
				if( nStanding & (1 << nPlayer) ) {
					pTripper = Player_aPlayer[nPlayer].m_pEntityCurrent;
				}
			}
			*pHeld = _aCoopHeldTripwires[--_nCoopHeldTripwires];
			pTripwireEntity->_CoopFireTripwireEnter( _CoopReleaseTripper( pTripper ) );
		}
	}
	for( s32 nPlayer=0; nPlayer < MAX_PLAYERS; nPlayer++ ) {
		_apCoopPreviousPlayers[nPlayer] = (nStanding & (1 << nPlayer)) ? Player_aPlayer[nPlayer].m_pEntityCurrent : NULL;
		if( _apCoopPreviousPlayers[nPlayer] ) {
			_avCoopPreviousPositions[nPlayer] = _apCoopPreviousPlayers[nPlayer]->MtxToWorld()->m_vPos;
		}
	}
}

void CEntity::CoopTripwireReset( void ) {
	_nCoopHeldTripwires = 0;
	for( s32 nPlayer=0; nPlayer < MAX_PLAYERS; nPlayer++ ) {
		_apCoopPreviousPlayers[nPlayer] = NULL;
		_apCoopWaitMessagePlayers[nPlayer] = NULL;
	}
}

BOOL CEntity::CoopTripwireWaitMessage( const CEntity *pPlayerEntity ) {
	const s32 nPlayer = _CoopPlayerOfEntity( pPlayerEntity );
	if( nPlayer < 0 ) {
		return FALSE;
	}
	if( CBot::m_bCutscenePlaying || !CoopTripwireWaiting( pPlayerEntity ) ) {
		_apCoopWaitMessagePlayers[nPlayer] = NULL;
		return FALSE;
	}
	// Crossings that resolve in a few frames need no flashing HUD prompt.
	// This delay changes only the message, never trigger release or exit safety.
	if( _apCoopWaitMessagePlayers[nPlayer] != pPlayerEntity ||
		FLoop_nTotalLoopTicks < _anCoopWaitMessageStartTicks[nPlayer] ) {
		_apCoopWaitMessagePlayers[nPlayer] = pPlayerEntity;
		_anCoopWaitMessageStartTicks[nPlayer] = FLoop_nTotalLoopTicks;
		return FALSE;
	}
	return FLoop_nTotalLoopTicks - _anCoopWaitMessageStartTicks[nPlayer] >= (u64)(FLoop_nTicksPerSec * 0.85f);
}

BOOL CEntity::CoopTripwireWaiting( const CEntity *pPlayerEntity ) {
	const s32 nPlayer = _CoopPlayerOfEntity( pPlayerEntity );
	if( nPlayer < 0 || !_CoopTripwireGateActive() ) {
		return FALSE;
	}
	// Terminal arrivals are parked deliberately, even if their entry momentum
	// carried them just beyond the volume before the wait began.
	if( CoopTripwireExitWaiting( pPlayerEntity ) ) {
		return TRUE;
	}
	const u32 nStanding = _CoopStandingPlayerMask();
	if( !(nStanding & (1 << nPlayer)) ) {
		return FALSE;
	}
	for( u32 i=0; i < _nCoopHeldTripwires; i++ ) {
		const _CoopHeldTripwire_t *pHeld = &_aCoopHeldTripwires[i];
		CEntity *pTripwire = pHeld->pTripwireEntity;
		if( !(pHeld->nArrivedMask & (1 << nPlayer)) || !pTripwire->IsTripwire() ||
			!pTripwire->IsInWorld() || !pTripwire->IsTripwireArmed() ||
			(pHeld->nArrivedMask & nStanding) == nStanding ) {
			continue;
		}
		// Arrival history stays sticky for thin gates crossed one at a time.
		// The HUD only describes a gate the player is still standing inside;
		// an unused side-route trigger must not leave a wait message over later play.
		const CFVec3A *pPos_WS = &pPlayerEntity->MtxToWorld()->m_vPos;
		if( pTripwire->TripwireCollisionTest( pPos_WS, pPos_WS ) & TRIPWIRE_COLLFLAG_NEWPOS_INSIDE ) {
			return TRUE;
		}
	}
	return FALSE;
}

BOOL CEntity::CoopTripwireExitWaiting( const CEntity *pPlayerEntity ) {
	const s32 nPlayer = _CoopPlayerOfEntity( pPlayerEntity );
	if( nPlayer < 0 || !_CoopTripwireGateActive() || !(_CoopStandingPlayerMask() & (1 << nPlayer)) ) {
		return FALSE;
	}
	if( CBot::m_bCutscenePlaying && nPlayer == game_GetStoryPlayerIndex() ) {
		return FALSE;
	}
	for( u32 i=0; i < _nCoopHeldTripwires; i++ ) {
		const _CoopHeldTripwire_t *pHeld = &_aCoopHeldTripwires[i];
		CEntity *pTripwire = pHeld->pTripwireEntity;
		if( !(pHeld->nArrivedMask & (1 << nPlayer)) || !pTripwire->IsInWorld() || !pTripwire->IsTripwireArmed() ) {
			continue;
		}
		// Common terminal tripwires in the retail WinLevel scripts. Ordinary
		// checkpoint, combat and possession triggers must leave players free to move.
		cchar *pszName = pTripwire->Name();
		if( pszName && (!fclib_stricmp( pszName, "levelend" ) ||
			!fclib_stricmp( pszName, "levelend1" ) || !fclib_stricmp( pszName, "levelend2" ) ||
			!fclib_stricmp( pszName, "endlevel" )) ) {
			return TRUE;
		}
	}
	return FALSE;
}
#endif


// Called when the entity is removed from the world.
// Depending on the type of entity, this function clears out
// either the entity's tripwire array or the tripwire's
// tripper array.
void CEntity::_ClearTripwireArray( void ) {
	FASSERT( !IsTripwire() || !IsTripper() );	// Cannot be both a tripwire and a tripper!

	if( m_pTripwire ) {
		// We're a tripwire entity...

		while( m_pTripwire->m_nContainedEntityCount ) {
			u32 nContainedEntityCount = m_pTripwire->m_nContainedEntityCount;
			m_pTripwire->m_ppContainedTrippersArray[ m_pTripwire->m_nContainedEntityCount - 1 ]->_EntityIsOutsideTripwire( this );
			if (m_pTripwire->m_nContainedEntityCount != nContainedEntityCount -1)
			{
				m_pTripwire->m_nContainedEntityCount--;//break infinite loop since this tripwire has guys inside it that don't know they are inside
			}
		}
	} else if( IsTripper() ) {
		// We're a tripper...

		while( m_nIntersectingTripwireCount ) {
			u32 nIntersectingTripwireCount = m_nIntersectingTripwireCount;
			_EntityIsOutsideTripwire( m_apIntersectingTripwires[ m_nIntersectingTripwireCount - 1 ] );
			if (m_nIntersectingTripwireCount != nIntersectingTripwireCount- 1)
			{
				m_nIntersectingTripwireCount--;//break infinite loop since this tripwire has guys inside it that don't know they are inside
			}
		}
	}
}


// Called when the tripwire's tripper include mask has changed.
// If the tripwire's container list includes a tripper that's
// not recognized by the tripwire, the tripper is removed from
// the tripwire's list.
void CEntity::_UpdateTripwireArrayBasedOnFilterChange( void ) {
	s32 i;

	FASSERT( IsTripwire() );
	FASSERT( !IsTripper() );

	for( i=0; i<(s32)m_pTripwire->m_nContainedEntityCount; i++ ) {
		if( !_DoesTripperPassTripwireFilterTest( m_pTripwire->m_ppContainedTrippersArray[i] ) ) {
			// This tripper doesn't belong in this tripwire's list...
			m_pTripwire->m_ppContainedTrippersArray[i]->_EntityIsOutsideTripwire( this );
			--i;
		}
	}
}


// Returns TRUE if the specified tripper passes the tripwire (*this) filter test.
BOOL CEntity::_DoesTripperPassTripwireFilterTest( CEntity *pTripperEntity ) {
	CBot *pBot;

	FASSERT( IsTripwire() );
	FASSERT( pTripperEntity->IsTripper() );

	if( m_pTripwire->m_pszTripwireFilterEntityName && pTripperEntity->Name()) {
		// Filter entity name specified...

		if( fclib_stricmp( pTripperEntity->Name(), m_pTripwire->m_pszTripwireFilterEntityName ) ) {
			// Tripper's name doesn't match filter...
			return FALSE;
		}
	}

	if( pTripperEntity->m_nEntityTypeBits & ENTITY_BIT_BOT ) {
		pBot = (CBot *)pTripperEntity;

		if( pBot->m_nPossessionPlayerIndex >= 0 ) {
			// This is a player tripper...

			if( m_pTripwire->m_nTripperFilterFlags & TRIPWIRE_FILTER_INCLUDE_PLAYER ) {
				return TRUE;
			}

		} else if( pBot->IsEnemy() ) {
			// This is an enemy tripper...

			if( m_pTripwire->m_nTripperFilterFlags & TRIPWIRE_FILTER_INCLUDE_ENEMY ) {
				return TRUE;
			}

		} else if( !pBot->IsEnemy() ) {
			// This is a friendly tripper...

			if( m_pTripwire->m_nTripperFilterFlags & TRIPWIRE_FILTER_INCLUDE_FRIENDLY ) {
				return TRUE;
			}
		}
	} else {
		// Not a bot...

		if( m_pTripwire->m_nTripperFilterFlags & TRIPWIRE_FILTER_INCLUDE_OTHER ) {
			return TRUE;
		}
	}

	return FALSE;
}


// Returns FALSE if there is no more space in either the entity's tripwire buffer, or the tripwire's entity buffer.
BOOL CEntity::_EntityIsInsideTripwire( CEntity *pTripwireEntity ) {
	CTripwire *pTripwire = pTripwireEntity->m_pTripwire;
	FASSERT( pTripwire );

	// Make sure both the entity and tripwire arrays have enough space...

	if( m_nIntersectingTripwireCount >= MAX_INTERSECTING_TRIPWIRES ) {
		// No more room in the entity's array...
		DEVPRINTF( "CEntity::_EntityIsInsideTripwire(): Entity is inside more than %u tripwires! Tell Steve!\n", MAX_INTERSECTING_TRIPWIRES );
		return FALSE;
	}

	if( pTripwire->m_nContainedEntityCount >= pTripwire->m_nMaxContainedEntityCount ) {
		// No more room in the tripwire's array...
		DEVPRINTF( "CEntity::_EntityIsInsideTripwire(): Tripwire contains more than %u entities! Tell Steve!\n", pTripwire->m_nMaxContainedEntityCount );
		return FALSE;
	}

	// Add the tripwire to the entity's array...
	m_apIntersectingTripwires[ m_nIntersectingTripwireCount++ ] = pTripwireEntity;

	// Add the entity to the tripwire's array...
	pTripwire->m_ppContainedTrippersArray[ pTripwire->m_nContainedEntityCount++ ] = this;

	// Success...

	return TRUE;
}


void CEntity::_EntityIsOutsideTripwire( CEntity *pTripwireEntity ) {
	u32 i;

	for( i=0; i<m_nIntersectingTripwireCount; ++i ) {
		if( m_apIntersectingTripwires[i] == pTripwireEntity ) {
			// The tripwire is in the entity's array...

			if( i < (m_nIntersectingTripwireCount - 1) ) {
				// This is not the last element in the array.
				// Replace our deleted element with the last element in the array...
				m_apIntersectingTripwires[i] = m_apIntersectingTripwires[ m_nIntersectingTripwireCount - 1 ];
			}

			// One less element in the entity's array...
			--m_nIntersectingTripwireCount;

			// Now, find the entity in the tripwire's array...
			CTripwire *pTripwire = pTripwireEntity->m_pTripwire;
			if( pTripwire )
			{

				for( i=0; i<pTripwire->m_nContainedEntityCount; ++i ) {
					if( pTripwire->m_ppContainedTrippersArray[i] == this ) {
						// Found it...

						if( i < (pTripwire->m_nContainedEntityCount - 1) ) {
							// This is not the last element in the array.
							// Replace our deleted element with the last element in the array...
							pTripwire->m_ppContainedTrippersArray[i] = pTripwire->m_ppContainedTrippersArray[ pTripwire->m_nContainedEntityCount - 1 ];
						}

						// One less element in the tripwire's array...
						--pTripwire->m_nContainedEntityCount;

						return;
					}
				}
			}

		}
	}
}


void CEntity::EnableClassHierarchyWorkBit( u32 nClassHierarchyBit ) {
	FASSERT( IsCreated() );

	u32 nPrevAutoWorkMask = m_nEnableWorkMask;

	FMATH_SETBITMASK( m_nEnableWorkMask, nClassHierarchyBit );

	if( !nPrevAutoWorkMask && m_nEnableWorkMask ) {
		if( IsWorkEnabled() && IsInWorld() && IsAutoWorkEnabled() ) {
			_AutoWorkList_AddTail();
		}
	}
}


void CEntity::DisableClassHierarchyWorkBit( u32 nClassHierarchyBit ) {
	FASSERT( IsCreated() );

	u32 nPrevAutoWorkMask = m_nEnableWorkMask;

	FMATH_CLEARBITMASK( m_nEnableWorkMask, nClassHierarchyBit );

	if( nPrevAutoWorkMask && !m_nEnableWorkMask ) {
		if( IsWorkEnabled() && IsInWorld() && IsAutoWorkEnabled() ) {
			_AutoWorkList_Remove();
		}
	}
}


void CEntity::EnableWork( BOOL bEnable ) {
	FASSERT( IsCreated() );

	if( bEnable ) {
		if( !IsWorkEnabled() ) {
			FMATH_SETBITMASK( m_nEntityFlags, ENTITY_FLAG_ENABLE_WORK );

			if( m_nEnableWorkMask && IsInWorld() && IsAutoWorkEnabled() ) {
				_AutoWorkList_AddTail();
			}
		}
	} else {
		if( IsWorkEnabled() ) {
			FMATH_CLEARBITMASK( m_nEntityFlags, ENTITY_FLAG_ENABLE_WORK );

			if( m_nEnableWorkMask && IsInWorld() && IsAutoWorkEnabled() ) {
				_AutoWorkList_Remove();
			}
		}
	}
}


void CEntity::EnableAutoWork( BOOL bEnable ) {
	FASSERT( IsCreated() );

	if( bEnable ) {
		if( !IsAutoWorkEnabled() ) {
			FMATH_SETBITMASK( m_nEntityFlags, ENTITY_FLAG_ENABLE_AUTOWORK );

			if( m_nEnableWorkMask && IsInWorld() && IsWorkEnabled() ) {
				_AutoWorkList_AddTail();
			}
		}
	} else {
		if( IsAutoWorkEnabled() ) {
			FMATH_CLEARBITMASK( m_nEntityFlags, ENTITY_FLAG_ENABLE_AUTOWORK );

			if( m_nEnableWorkMask && IsInWorld() && IsWorkEnabled() ) {
				_AutoWorkList_Remove();
			}
		}
	}
}


void CEntity::CallAllAutoWorks( void ) {
	CEntity *pEntity;

	if( !IsSystemInitialized() ) {
		return;
	}

	for( pEntity=AutoWorkList_GetHead(); pEntity; pEntity=pEntity->AutoWorkList_GetNext() ) {
		if( !FLoop_bGamePaused || pEntity->IsWorkWhilePausedEnabled() ) {
			pEntity->ClassHierarchyWork();
		}
	}
}


void CEntity::CallAllMarkedRemoveFromWorld( void ) {
	CEntity *pEntity;

	if( !IsSystemInitialized() ) {
		return;
	}

	while( pEntity = MarkedList_GetHead() ) {
		pEntity->RemoveFromWorld( TRUE );
		pEntity->ResetGuid();		  //findfix: pgm thinks the GUID reset should be in RemoveFromWorld, but that can't happen until we change things so that people can't remove from world and expect to re-add with no harm done
	}
}


void CEntity::Work( void ) {
	FASSERT_MSG( !_IsBuilding(), "CEntity::Work(): Cannot be called while building."  );

	if( m_nEnableWorkMask == 0 ) {
		return;
	}

	if( !IsSystemInitialized() || !IsWorkEnabled() || !IsInWorld() ) {
		return;
	}

	if( FLoop_bGamePaused && !IsWorkWhilePausedEnabled() ) {
		return;
	}

	ClassHierarchyWork();
}


void CEntity::ResolveEntityPointerFixups( void ) {
	CEntity *pEntity;

	FASSERT( IsSystemInitialized() );

	loadingscreen_Update();

	for( pEntity=OutOfWorldList_GetHead(); pEntity; pEntity=pEntity->OutOfWorldList_GetNext() ) {
		pEntity->ClassHierarchyResolveEntityPointerFixups();
	}

	for( pEntity=InWorldList_GetHead(); pEntity; pEntity=pEntity->InWorldList_GetNext() ) {
		pEntity->ClassHierarchyResolveEntityPointerFixups();
	}

	AlarmSys_ResolveEntityPointerFixups();
	CSpawnSys::ResolveEntityPointerFixups();

	loadingscreen_Update();	
}


void CEntity::ClassHierarchyResolveEntityPointerFixups( void ) {
	CEntity *pParentEntity;

	if( !(m_nEntityFlags & ENTITY_FLAG_BUILDPARENT_IS_INDEX) ) {
		if( m_pszParentEntityName ) {
			pParentEntity = Find( m_pszParentEntityName );

			if( pParentEntity ) {
				Attach_ToParent_WS( pParentEntity, m_pszAttachBone );

				// Allow child to re-attach to parent after checkpoint restore.
				// (Don't use if parent entity already has code to re-attach its 
				// children after restore, as with bot weapons.)
				SetRestoreAttach( TRUE );
			} else {
				DEVPRINTF( "CEntity::ClassHierarchyResolveEntityPointerFixups(): Entity '%s' at Max ( %.2f, %.2f, %.2f )\n", Name(), m_MtxToWorld.m_vPos.x, m_MtxToWorld.m_vPos.z, m_MtxToWorld.m_vPos.y );
				DEVPRINTF( "                                                     Cannot find parent entity '%s'.\n", m_pszParentEntityName );
			}
		}
	}
}


void CEntity::SetTargetable( BOOL bEnabled ) {
	FASSERT( IsCreated() );

	if( bEnabled ) {
		FMATH_SETBITMASK( m_nEntityFlags, ENTITY_FLAG_TARGETABLE );
	} else {
		FMATH_CLEARBITMASK( m_nEntityFlags, ENTITY_FLAG_TARGETABLE );
	}
}


void CEntity::SetInvincible( BOOL bEnabled ) {
	FASSERT( IsCreated() );

	if( bEnabled ) {
		FMATH_SETBITMASK( m_nEntityFlags, ENTITY_FLAG_INVINCIBLE );
	} else {
		FMATH_CLEARBITMASK( m_nEntityFlags, ENTITY_FLAG_INVINCIBLE );
	}
}


void CEntity::SetActionable( BOOL bEnabled ) {
	FASSERT( IsCreated() );

	if( bEnabled ) {
		FMATH_SETBITMASK( m_nEntityFlags, ENTITY_FLAG_ACTIONABLE );
	} else {
		FMATH_CLEARBITMASK( m_nEntityFlags, ENTITY_FLAG_ACTIONABLE );
	}
}

void CEntity::SetRemovedOnParentsDetach( BOOL bEnabled ) 
{
	FASSERT( IsCreated() );
	if( bEnabled ) 
	{
		FMATH_CLEARBITMASK( m_nEntityFlags, ENTITY_FLAG_NOT_REMOVED_ON_DETACH );
	}
	else 
	{
		FMATH_SETBITMASK( m_nEntityFlags, ENTITY_FLAG_NOT_REMOVED_ON_DETACH );
	}
}

BOOL CEntity::ActionNearby( CEntity *pFromWho )
{
	FASSERT( IsCreated() );

	CEntity *pScriptActor = pFromWho;
#if FANG_WINGC
	// xemc_crabt's escape conversation accepts only its cached Bot_GetPlayer
	// handle. Let a co-op Glitch recruit the captain through that same story
	// actor; keep proximity, buddy AI and other action callbacks on the operator.
	if( MultiplayerMgr.IsLocalCoop() && Level_nLoadedIndex >= 0 &&
		!fclib_stricmp(Level_aInfo[Level_nLoadedIndex].pszWorldResName,"WEMCcity_01") &&
		Name() && !fclib_stricmp(Name(),"CapnPeanuts") ) {
		for( s32 n=0; n<CPlayer::m_nPlayerCount; ++n ) {
			if( Player_aPlayer[n].m_pEntityOrig == pFromWho &&
				Player_aPlayer[n].m_pEntityCurrent == pFromWho ) {
				pScriptActor = Player_aPlayer[game_GetStoryPlayerIndex()].m_pEntityCurrent;
				break;
			}
		}
	}
#endif
	CFScriptSystem::TriggerEvent(CFScriptSystem::GetEventNumFromName("action"), (u32)(this), (u32)(pScriptActor), 0);

	if (HasAlarmSysUse())
	{
		AlarmSys_ActionNearby(pFromWho, this);
	}
	if( m_pFcnAction != NULL )
	{ 
		return m_pFcnAction( pFromWho, this );
	}
	return FALSE;
}


void CEntity::EnableTripper( BOOL bEnable ) {
	FASSERT( IsCreated() );

	if( CTripwire::m_nTripwireEventNumber >= 0 ) {
		if( bEnable ) {
			FMATH_SETBITMASK( m_nEntityFlags, ENTITY_FLAG_TRIPPER );
		} else {
			FMATH_CLEARBITMASK( m_nEntityFlags, ENTITY_FLAG_TRIPPER );
		}
	}
}


void CEntity::EnableTripwireEvents( u32 nEventFlagsToEnable ) {
	FASSERT( IsCreated() );

	if( m_pTripwire ) {
		u32 nPrevFlags = m_pTripwire->m_nEventFlags;

		FMATH_SETBITMASK( m_pTripwire->m_nEventFlags, nEventFlagsToEnable );

		if( !nPrevFlags && m_pTripwire->m_nEventFlags ) {
			if( IsTripwireArmed() && m_pTripwire->m_nTripperFilterFlags && IsInWorld() ) {
				m_pTripwire->TripwireList_AddTail();
			}
		}
	}
}


void CEntity::DisableTripwireEvents( u32 nEventFlagsToDisable ) {
	FASSERT( IsCreated() );

	if( m_pTripwire ) {
		u32 nPrevFlags = m_pTripwire->m_nEventFlags;

		FMATH_CLEARBITMASK( m_pTripwire->m_nEventFlags, nEventFlagsToDisable );

		if( nPrevFlags && !m_pTripwire->m_nEventFlags ) {
			if( IsTripwireArmed() && m_pTripwire->m_nTripperFilterFlags && IsInWorld() ) {
				_ClearTripwireArray();
				m_pTripwire->TripwireList_Remove();
			}
		}
	}
}


void CEntity::SetTripwireEventMask( u32 nEventMask ) {
	FASSERT( IsCreated() );

	if( m_pTripwire ) {
		u32 nPrevFlags = m_pTripwire->m_nEventFlags;

		m_pTripwire->m_nEventFlags = nEventMask;

		if( nPrevFlags && !m_pTripwire->m_nEventFlags ) {
			if( IsTripwireArmed() && m_pTripwire->m_nTripperFilterFlags && IsInWorld() ) {
				_ClearTripwireArray();
				m_pTripwire->TripwireList_Remove();
			}
		}
	}
}


void CEntity::EnableTripwireFilterFlags( u32 nFilterFlagsToEnable ) {
	FASSERT( IsCreated() );

	if( m_pTripwire ) {
		u32 nPrevFlags = m_pTripwire->m_nTripperFilterFlags;

		FMATH_SETBITMASK( m_pTripwire->m_nTripperFilterFlags, nFilterFlagsToEnable );

		if( !nPrevFlags && m_pTripwire->m_nTripperFilterFlags ) {
			if( IsTripwireArmed() && m_pTripwire->m_nEventFlags && IsInWorld() ) {
				m_pTripwire->TripwireList_AddTail();
			}
		}

		if( nPrevFlags != m_pTripwire->m_nTripperFilterFlags ) {
			_UpdateTripwireArrayBasedOnFilterChange();
		}
	}
}


void CEntity::DisableTripwireFilterFlags( u32 nFilterFlagsToDisable ) {
	FASSERT( IsCreated() );

	if( m_pTripwire ) {
		u32 nPrevFlags = m_pTripwire->m_nTripperFilterFlags;

		FMATH_CLEARBITMASK( m_pTripwire->m_nTripperFilterFlags, nFilterFlagsToDisable );

		if( nPrevFlags && !m_pTripwire->m_nTripperFilterFlags ) {
			if( IsTripwireArmed() && m_pTripwire->m_nEventFlags && IsInWorld() ) {
				m_pTripwire->TripwireList_Remove();
			}
		}

		if( nPrevFlags != m_pTripwire->m_nTripperFilterFlags ) {
			_UpdateTripwireArrayBasedOnFilterChange();
		}
	}
}


void CEntity::SetTripwireFilterMask( u32 nFilterFlags ) {
	FASSERT( IsCreated() );

	if( m_pTripwire ) {
		u32 nPrevFlags = m_pTripwire->m_nTripperFilterFlags;

		m_pTripwire->m_nTripperFilterFlags = nFilterFlags;

		if( nPrevFlags && !m_pTripwire->m_nTripperFilterFlags ) {
			if( IsTripwireArmed() && m_pTripwire->m_nEventFlags && IsInWorld() ) {
				m_pTripwire->TripwireList_Remove();
			}
		}

		if( nPrevFlags != m_pTripwire->m_nTripperFilterFlags ) {
			_UpdateTripwireArrayBasedOnFilterChange();
		}
	}
}


void CEntity::ArmTripwire( BOOL bArm ) {
	FASSERT( IsCreated() );

	if( m_pTripwire ) {
		if( bArm ) {
			if( !IsTripwireArmed() ) {
				FMATH_SETBITMASK( m_nEntityFlags, ENTITY_FLAG_TRIPWIRE_ARMED );

				if( m_pTripwire->m_nEventFlags && m_pTripwire->m_nTripperFilterFlags && IsInWorld() ) {
					m_pTripwire->TripwireList_AddTail();
				}
			}
		} else {
			if( IsTripwireArmed() ) {
				FMATH_CLEARBITMASK( m_nEntityFlags, ENTITY_FLAG_TRIPWIRE_ARMED );

				if( m_pTripwire->m_nEventFlags && m_pTripwire->m_nTripperFilterFlags && IsInWorld() ) {
					_ClearTripwireArray();
					m_pTripwire->TripwireList_Remove();
				}
			}
		}
	}
}


BOOL CEntity::_WorldCallback( FWorldEvent_e nEvent ) {
	switch( nEvent ) {
	case FWORLD_EVENT_WORLD_POSTLOAD:
		CTripwire::m_nTripwireEventNumber = CFScriptSystem::GetEventNumFromName( ENTITY_TRIPWIRE_EVENT_NAME );

		{
			CFWire::CCreateInfo WireCreateInfo;

			WireCreateInfo.m_pszWireMeshName = _WIRE_MESH_NAME;
			WireCreateInfo.m_pszDebrisMeshName = _WIRE_DEBRIS_MESH_NAME;
			WireCreateInfo.m_pszSoundGroupSlice = _WIRE_SLICE_SOUND_GROUP;
			WireCreateInfo.m_nWiresPerTexture = _WIRE_COLORS_PER_TEXTURE;
			WireCreateInfo.m_nWirePoolCount = m_nWirePoolCount;
			WireCreateInfo.m_nTexLayerID = _WIRE_TEX_LAYER_ID;

			if( !CFWire::Create( &WireCreateInfo ) ) {
				DEVPRINTF( "gameloop::_GameInit(): Unable to create physics wire system. No wires will be drawn.\n" );
			}
		}

		break;

	case FWORLD_EVENT_WORLD_PREDESTROY:
		CFWire::Destroy();
		_DestroyAll();
		break;
	}

	return TRUE;
}


void CEntity::RemoveAndDestroyAll( void ) {
	FASSERT( FWorld_pWorld );

	CFWire::Destroy();
	_DestroyAll();
}


void CEntity::_DestroyAll( void ) {
	CEntity *pEntity;

	do {
		pEntity = InWorldList_GetTail();

		for( ; pEntity; pEntity = pEntity->InWorldList_GetPrev() ) {
			if( pEntity->m_nEntityFlags & ENTITY_FLAG_AUTODELETE ) {
				pEntity->Destroy();
				fdelete( pEntity );
				break;
			}
		}
	} while( pEntity );

	do {
		pEntity = OutOfWorldList_GetTail();

		for( ; pEntity; pEntity = pEntity->OutOfWorldList_GetPrev() ) {
			if( pEntity->m_nEntityFlags & ENTITY_FLAG_AUTODELETE ) {
				pEntity->Destroy();
				fdelete( pEntity );
				break;
			}
		}
	} while( pEntity );

/*
	while( pEntity = InWorldList_GetTail() ) {
		pEntity->Destroy();

		if( pEntity->m_nEntityFlags & ENTITY_FLAG_AUTODELETE ) {
			fdelete( pEntity );
		}
	}

	while( pEntity = OutOfWorldList_GetTail() ) {
		pEntity->Destroy();

		if( pEntity->m_nEntityFlags & ENTITY_FLAG_AUTODELETE ) {
			fdelete( pEntity );
		}
	}
*/
}

#if	( 0 )	// turn on to check entity sizes, but don't leave on, doubles entity allocations and is not stable!!
static CEntity *_pSE;
static u32 _ssize, _dsize, _tsize; 
static u32 _bsize = 0; 
static u32 _nObj = 0; 
static u32 _totalMem = 0; 
static FResFrame_t _mB, _mA; 
static char _szBE[128];
static char _szET[128];
#define _SET_ENTITY_TYPE( t,s )	{ _pSE = fnew (t); _ssize = sizeof(t); fclib_strcpy( _szET, (s) ); }
#define _DSIZE_BEFORE()			{ _mB = fres_GetFrame(); fres_ReleaseFrame( _mB ); }
#define _DSIZE_AFTER()			{ _mA = fres_GetFrame(); fres_ReleaseFrame( _mA ); _dsize = (u32)_mB - (u32)_mA; _tsize = _ssize + _dsize; ++_nObj; _totalMem += _tsize; }
#define _COMPUTE_SIZES()		{ _DSIZE_BEFORE(); _pSE->BuildFromWorldShape(); _DSIZE_AFTER(); }
#define _REPORT_SIZE_HEADER()	{ DEVPRINTF( "World Shape Entity Memory Usage Report:\n" ); }
#define _REPORT_SIZE( pE )		{ if(_tsize>_bsize){_bsize=_tsize;if(pE->m_pszName)fclib_strcpy(_szBE,pE->m_pszName);}  DEVPRINTF( "%d) %s named %s used: %d bytes, %d static, %d dynamic\n", _nObj, _szET, (pE)->m_pszName, _ssize + _dsize, _ssize, _dsize ); }
#define _REPORT_SIZE_SUMMARY()	{ DEVPRINTF( "Total World Shape Entity mem usage: %d bytes.\nAverage mem usage: %d bytes-per-entity\nAnd the Memory Hog award goes to... %s at %d bytes.\n\n", _totalMem, _totalMem / _nObj, _szBE, _bsize ); }
#else
#define _SET_ENTITY_TYPE( t,s )
#define _DSIZE_BEFORE()
#define _DSIZE_AFTER()
#define _COMPUTE_SIZES()
#define _REPORT_SIZE_HEADER()
#define _REPORT_SIZE( pE )
#define _REPORT_SIZE_SUMMARY()
#endif



BOOL CEntity::_WorldShapeCreateCallback( cchar *pszWorldResName, const void *pFixupOffsetBase, const CFWorldShapeInit *pShapeInitArray, u32 nShapeInitCount ) {
	const CFWorldShapeInit *pShapeInit;
	u32 i;

	FMemFrame_t MemFrame = fmem_GetFrame();

	_REPORT_SIZE_HEADER();

	for( i=0, pShapeInit=pShapeInitArray; i<nShapeInitCount; i++, pShapeInit++ ) {

		loadingscreen_Update();

		if( (pShapeInit->m_pGameData==NULL && pShapeInit->m_nParentShapeIndex<0) || !_CreateWorldShape( pszWorldResName, (CFWorldShapeInit *)pShapeInit, pFixupOffsetBase ) ) {
			// Either no game data was provided, or there was a catastrophic error parsing the game data.
			// Just construct a normal world mesh...

			if( pShapeInit->m_nShapeType==FWORLD_SHAPETYPE_MESH && pShapeInit->m_pMesh->m_pszMeshName ) {
				const CFWorldShapeMesh *pShapeMesh = pShapeInit->m_pMesh;

				FMeshInit_t MeshInit;
				CFWorldMesh *pWorldMesh;

				MeshInit.Mtx.Set( pShapeInit->m_Mtx43 );
				MeshInit.fCullDist = fmath_Sqrt( pShapeMesh->m_fCullDist2 );
				MeshInit.nFlags = pShapeMesh->m_nMeshInstFlags;

				MeshInit.pMesh = (FMesh_t *)fresload_Load( FMESH_RESTYPE, pShapeMesh->m_pszMeshName );
				if( MeshInit.pMesh == NULL ) {
					// Mesh resource not found...
					DEVPRINTF( "CEntity::_WorldShapeCreateCallback(): Could not load mesh '%s'.\n", pShapeMesh->m_pszMeshName );
					continue;
				}

				pWorldMesh = fnew CFWorldMesh;
				if( pWorldMesh == NULL ) {
					DEVPRINTF( "CEntity::_WorldShapeCreateCallback(): Could not allocate memory for all CFWorldMesh objects.\n" );
					break;
				}

				pWorldMesh->Init( &MeshInit );
				pWorldMesh->SetCollisionFlag( !(pWorldMesh->m_nFlags & FMESHINST_FLAG_NOCOLLIDE) );
				if ( pShapeMesh->m_papszLightMapName[0] )
				{
					// This object has instance lightmaps
					pWorldMesh->SetLightmaps( (cchar **)pShapeMesh->m_papszLightMapName, pShapeMesh->m_anLightMapMotif, FWORLD_MAX_MESH_SHAPE_LIGHTMAPS );
				}
				if ( pShapeMesh->m_nColorStreamCount )
				{
					// This object has instance color streams
					pWorldMesh->SetColorStreams( pShapeMesh->m_nColorStreamCount, pShapeMesh->m_paColorStreams );
				}
				if ( pShapeMesh->m_nMeshInstFlags & FMESHINST_FLAG_TINT )
				{
					// This object has been tinted in the tools
					pWorldMesh->SetMeshTint( pShapeMesh->m_TintRGB.fRed, pShapeMesh->m_TintRGB.fGreen, pShapeMesh->m_TintRGB.fBlue );
				}
			}
		}
	}

	_REPORT_SIZE_SUMMARY();

	// Set up parent/child relationships...

	CEntity *pChildEntity;

	for( pChildEntity=OutOfWorldList_GetHead(); pChildEntity; pChildEntity=pChildEntity->OutOfWorldList_GetNext() ) {
		loadingscreen_Update();
		_AttachWorldShape( pChildEntity, (CFWorldShapeInit *)pShapeInitArray );
	}

	for( pChildEntity=InWorldList_GetHead(); pChildEntity; pChildEntity=pChildEntity->InWorldList_GetNext() ) {
		loadingscreen_Update();
		_AttachWorldShape( pChildEntity, (CFWorldShapeInit *)pShapeInitArray );
	}

	fmem_ReleaseFrame( MemFrame );

	return TRUE;
}


// Establishes parent/child relationships for art-path-generated entities.
void CEntity::_AttachWorldShape( CEntity *pChildEntity, CFWorldShapeInit *pShapeInitArray ) {
	CFWorldShapeInit *pShapeInit;
	CEntity *pParentEntity;

	if( pChildEntity->m_nEntityFlags & ENTITY_FLAG_BUILDPARENT_IS_INDEX ) {
		if( pChildEntity->m_nPendingAttachShapeIndex < 0 ) {
			return;
		}

		pShapeInit = &pShapeInitArray[pChildEntity->m_nPendingAttachShapeIndex];
		pChildEntity->m_nPendingAttachShapeIndex = -1;

		if( pShapeInit->m_nShapeType != FWORLD_SHAPETYPE_COUNT ) {
			DEVPRINTF( "CEntity::_WorldShapeCreateCallback(): Entity '%s' cannot find parent at Max ( %.2f, %.2f, %.2f ).\n", pChildEntity->Name(), pShapeInit->m_Mtx43.m_vPos.x, pShapeInit->m_Mtx43.m_vPos.z, pShapeInit->m_Mtx43.m_vPos.y );
			return;
		}

		pParentEntity = (CEntity *)pShapeInit->m_pGameData;

		pChildEntity->Attach_ToParent_WS( pParentEntity, pChildEntity->m_pszAttachBone );

		// Allow child to re-attach to parent after checkpoint restore.
		// (Don't use if parent entity already has code to re-attach its 
		// children after restore, as with bot weapons.)
		pChildEntity->SetRestoreAttach( TRUE );
	}
}


CEntity *CEntity::_CreateWorldShape( cchar *pszWorldResName, CFWorldShapeInit *pShapeInit, const void *pFixupOffsetBase ) {
	const CFWorldShapeMesh *pShapeMesh = pShapeInit->m_pMesh;
	CEntity *pEntity;
	BOOL bBuildEntity = TRUE;

	FResFrame_t ResFrame = fres_GetFrame();

	pEntity = NULL;

	CEntityParser::Begin( pShapeInit );

	if( CEntityParser::m_pszEntityName && !gstring_Main.AddString( CEntityParser::m_pszEntityName ) ) {
		CEntityParser::Error_OutOfMemory();
		goto _ExitWithError;
	}

	if( CEntityParser::m_pszEntityType==NULL || !fclib_stricmp( CEntityParser::m_pszEntityType, "Object" ) ) {
		// No type specified...

		switch( pShapeInit->m_nShapeType ) {
		case FWORLD_SHAPETYPE_POINT:
			// CEPoint object...

			pEntity = fnew CEPoint;
			_SET_ENTITY_TYPE( CEPoint, "CEPoint" );
			break;

		case FWORLD_SHAPETYPE_SPLINE:
			// CESpline object...

			pEntity = fnew CESpline;
			_SET_ENTITY_TYPE( CESpline, "CESpline" );
			break;

		case FWORLD_SHAPETYPE_SPHERE:
			// CESphere object...

			pEntity = fnew CESphere;
			_SET_ENTITY_TYPE( CESphere, "CESphere" );
			break;

		case FWORLD_SHAPETYPE_LINE:
			// CELine object...

			pEntity = fnew CELine;
			_SET_ENTITY_TYPE( CELine, "CELine" );
			break;

		case FWORLD_SHAPETYPE_BOX:
			// CEBox object...

			pEntity = fnew CEBox;
			_SET_ENTITY_TYPE( CEBox, "CEBox" );
			break;

		case FWORLD_SHAPETYPE_CYLINDER:
			bBuildEntity = FALSE;
			break;

		case FWORLD_SHAPETYPE_MESH:
			// Generic CMeshEntity...

			pEntity = fnew CMeshEntity;
			_SET_ENTITY_TYPE( CMeshEntity, "CMeshEntity" );
			break;

		default:
			bBuildEntity = FALSE;
			DEVPRINTF( "CEntity::_CreateWorldShapeMesh(): Unknown shape type %u.\n", pShapeInit->m_nShapeType );
			break;
		}
	} else {
		// Entity type specified...

		if( !fclib_stricmp( CEntityParser::m_pszEntityType, ENTITY_TYPE_BOTGLITCH ) ) {
			pEntity = fnew CBotGlitch;
			_SET_ENTITY_TYPE( CBotGlitch, "CBotGlitch" );

		} else if( !fclib_stricmp( CEntityParser::m_pszEntityType, ENTITY_TYPE_BOTGRUNT ) ) {
			pEntity = fnew CBotGrunt;
			_SET_ENTITY_TYPE( CBotGrunt, "CBotGrunt" );

		} else if( !fclib_stricmp( CEntityParser::m_pszEntityType, ENTITY_TYPE_DOOR ) || !fclib_stricmp( CEntityParser::m_pszEntityType, ENTITY_TYPE_LIFT ) ) {
			pEntity = fnew CDoorEntity;
			_SET_ENTITY_TYPE( CDoorEntity, "CDoorEntity" );

		} else if( !fclib_stricmp( CEntityParser::m_pszEntityType, ENTITY_TYPE_BOOMER ) ) {
			if( pShapeInit->m_nShapeType != FWORLD_SHAPETYPE_MESH ) {
				CEntityParser::Error_Prefix();
				DEVPRINTF( "Destruct objects must be placed with an obj_.\n" );
				CEntityParser::Error_Dashes();

				bBuildEntity = FALSE;
			} else {
				pEntity = fnew CEBoomer;
				_SET_ENTITY_TYPE( CEBoomer, "CEBoomer" );
			}

		} else if( !fclib_stricmp( CEntityParser::m_pszEntityType, ENTITY_TYPE_SWITCH ) ) {
			pEntity = fnew CESwitch;
			_SET_ENTITY_TYPE( CESwitch, "CESwitch" );

		} else if( !fclib_stricmp( CEntityParser::m_pszEntityType, ENTITY_TYPE_GOODIE ) ) {
			pEntity = CCollectable::GetCollectable( TRUE );

			if( pEntity && pEntity->IsCreated() ) {
				((CCollectable *) pEntity )->Destroy();
				FMATH_CLEARBITMASK( pEntity->m_nEntityFlags, (ENTITY_FLAG_BUILDING | ENTITY_FLAG_CREATED) );
			}

			_SET_ENTITY_TYPE( CEGoodie, "CEGoodie" );

		} else if( !fclib_stricmp( CEntityParser::m_pszEntityType, ENTITY_TYPE_BOTPRED ) ) {
			pEntity = fnew CBotPred;
			_SET_ENTITY_TYPE( CBotPred, "CEBotPred" );

		} else if( !fclib_stricmp( CEntityParser::m_pszEntityType, ENTITY_TYPE_CONSOLE ) ) {
			pEntity = fnew CEConsole;
			_SET_ENTITY_TYPE( CEConsole, "CEConsole"  );

		} else if( !fclib_stricmp( CEntityParser::m_pszEntityType, ENTITY_TYPE_PARTICLE ) ) {
			pEntity = fnew CEParticle;
			_SET_ENTITY_TYPE( CEParticle, "CEParticle" );

		} else if( !fclib_stricmp( CEntityParser::m_pszEntityType, ENTITY_TYPE_ZIPLINE ) ) {
			pEntity = fnew CEZipLine;
			_SET_ENTITY_TYPE( CEZipLine, "CEZipLine" );

		} else if( !fclib_stricmp( CEntityParser::m_pszEntityType, ENTITY_TYPE_BOTTITAN ) ) {
			pEntity = fnew CBotTitan;
			_SET_ENTITY_TYPE( CBotTitan, "CBotTitan" );

		} else if( !fclib_stricmp( CEntityParser::m_pszEntityType, ENTITY_TYPE_BOTSITEWEAPON ) ) {
			pEntity = fnew CBotSiteWeapon;

		} else if( !fclib_stricmp( CEntityParser::m_pszEntityType, ENTITY_TYPE_VEHICLERAT ) ) {
			pEntity = fnew CVehicleRat;
			_SET_ENTITY_TYPE( CVehicleRat, "CVehicleRat" );

		} else if( !fclib_stricmp( CEntityParser::m_pszEntityType, ENTITY_TYPE_BOTPROBE ) ) {
			pEntity = fnew CBotProbe;
			_SET_ENTITY_TYPE( CBotProbe, "CBotProbe" );

		} else if( !fclib_stricmp( CEntityParser::m_pszEntityType, ENTITY_TYPE_VEHICLESENTINEL ) ) {
			pEntity = fnew CVehicleSentinel;
			_SET_ENTITY_TYPE( CVehicleSentinel, "CVehicleSentinel" );

		} else if( !fclib_stricmp( CEntityParser::m_pszEntityType, ENTITY_TYPE_BOTCHEMBOT ) ) {
			pEntity = fnew CBotSlosh;
			_SET_ENTITY_TYPE( CBotSlosh, ENTITY_TYPE_CHEMBOT );

		} else if( !fclib_stricmp( CEntityParser::m_pszEntityType, ENTITY_TYPE_LIQUIDVOLUME ) ) {
			pEntity = fnew CELiquidVolume;
			_SET_ENTITY_TYPE( CELiquidVolume, "CELiquidVolume" );

		} else if( !fclib_stricmp( CEntityParser::m_pszEntityType, ENTITY_TYPE_LIQUIDMESH ) ) {
			pEntity = fnew CELiquidMesh;
			_SET_ENTITY_TYPE( CELiquidMesh, "CELiquidMesh" );

		} else if( !fclib_stricmp( CEntityParser::m_pszEntityType, ENTITY_TYPE_VEHICLELOADER ) ) {
			pEntity = fnew CVehicleLoader;
			_SET_ENTITY_TYPE( CVehicleLoader, "CVehicleLoader" );

		} else if( !fclib_stricmp( CEntityParser::m_pszEntityType, ENTITY_TYPE_BOTSWARMER ) ) {
			pEntity = fnew CBotSwarmer;
			_SET_ENTITY_TYPE( CVehicleLoader, "CBotSwarmer" );

		} else if( !fclib_stricmp( CEntityParser::m_pszEntityType, ENTITY_TYPE_BOTELITEGUARD ) ) {
			pEntity = fnew CBotEliteGuard;

		} else if( !fclib_stricmp( CEntityParser::m_pszEntityType, ENTITY_TYPE_BOTAAGUN ) ) {
			pEntity = fnew CBotAAGun;
			_SET_ENTITY_TYPE( CBotAAGun, "CBotAAGun" );

		} else if( !fclib_stricmp( CEntityParser::m_pszEntityType, ENTITY_TYPE_KRUNK ) ) {
			pEntity = fnew CBotKrunk;
			_SET_ENTITY_TYPE( CBotKrunk, "CBotKrunk" );

		} else if( !fclib_stricmp( CEntityParser::m_pszEntityType, ENTITY_TYPE_BOTZOM ) ) {
			pEntity = fnew CBotZom;
			_SET_ENTITY_TYPE( CBotZom, "CBotZom" );
		
		} else if( !fclib_stricmp( CEntityParser::m_pszEntityType, ENTITY_TYPE_BOTJUMPER ) ) {
			pEntity = fnew CBotJumper;
			_SET_ENTITY_TYPE( CBotJumper, "CBotJumper" );

		} else if( !fclib_stricmp( CEntityParser::m_pszEntityType, ENTITY_TYPE_DETPACKDROP ) ) {
			pEntity = fnew CEDetPackDrop;
			_SET_ENTITY_TYPE( CEDetPackDrop, "CEDetPackDrop" );

		} else if( !fclib_stricmp( CEntityParser::m_pszEntityType, ENTITY_TYPE_BOTMINER ) ||
					!fclib_stricmp( CEntityParser::m_pszEntityType, "BotMiner")) {
			pEntity = fnew CBotMiner;
			_SET_ENTITY_TYPE( CBotMiner, "CBotMiner" );

		} else if( !fclib_stricmp( CEntityParser::m_pszEntityType, ENTITY_TYPE_BOTMORTAR ) ) {
			pEntity = fnew CBotMortar;
		
		} else if( !fclib_stricmp( CEntityParser::m_pszEntityType, ENTITY_TYPE_BOTMOZER ) ) {
			pEntity = fnew CBotMozer;

		} else if( !fclib_stricmp( CEntityParser::m_pszEntityType, ENTITY_TYPE_BOTSCOUT ) ) {
			pEntity = fnew CBotScout;

		} else if( !fclib_stricmp( CEntityParser::m_pszEntityType, ENTITY_TYPE_JUMPPAD ) ) {
			pEntity = fnew CEJumpPad;

		} else if( !fclib_stricmp( CEntityParser::m_pszEntityType, ENTITY_TYPE_BOTCORROSIVE ) ) {
			pEntity = fnew CBotCorrosive;
			_SET_ENTITY_TYPE( CBotCorrosive, "CBotCorrosive" );

		} else if( !fclib_stricmp( CEntityParser::m_pszEntityType, ENTITY_TYPE_BOTSCIENTIST ) ) {
			pEntity = fnew CBotScientist;
			_SET_ENTITY_TYPE( CBotScientist, "CBotScientist" );

		} else if( !fclib_stricmp( CEntityParser::m_pszEntityType, ENTITY_TYPE_BOTSNARQ ) ) {
			pEntity = fnew CBotSnarq;
			_SET_ENTITY_TYPE( CBotSnarq, "CBotSnarq" );
		
		} else if( !fclib_stricmp( CEntityParser::m_pszEntityType, ENTITY_TYPE_BOTZOMBIEBOSS) ) {
			pEntity = fnew CBotZombieBoss;

		} else if( !fclib_stricmp( CEntityParser::m_pszEntityType, ENTITY_TYPE_DEBRIS ) ) {
			pEntity = fnew CEDebris;
			_SET_ENTITY_TYPE( CEDebris, "CEDebris" );

		} else {
			CEntityParser::Error_InvalidObjectType();
			bBuildEntity = FALSE;
		}
	}

	if( bBuildEntity ) {
		if( pEntity == NULL ) {
			CEntityParser::Error_OutOfMemory();
			goto _ExitWithError;
		}
static BOOL dump = FALSE;

//if( dump ) fheap_ShowMemoryTracking();
		if( !pEntity->ClassHierarchyLoadSharedResources() ) {
			goto _ExitWithError;
		}

		if( !pEntity->BuildFromWorldShape() ) {
			goto _ExitWithError;
		}
//if( dump ) fheap_ShowMemoryTracking();

		_COMPUTE_SIZES();
		_REPORT_SIZE( pEntity );

		// Entity build successfully...

		// NKM
		if( CEntityParser::m_pszEntityType && 
			!fclib_stricmp( CEntityParser::m_pszEntityType, ENTITY_TYPE_GOODIE ) ) {
			pEntity->SetAutoDelete( FALSE );
		} else {
			FMATH_SETBITMASK( pEntity->m_nEntityFlags, ENTITY_FLAG_AUTODELETE );
		}

		// Store CEntity pointer into shape and flag as stored...
		pShapeInit->m_nShapeType = FWORLD_SHAPETYPE_COUNT;
		pShapeInit->m_pGameData = pEntity;
	}

	return pEntity;

	// Error:
_ExitWithError:
#if FANG_WINGC
	// Scripts and level logic look entities up by name, so name the ones that failed.
	DEVPRINTF( "CEntity::_CreateWorldShape(): Could not build %s entity '%s'.\n",
		CEntityParser::m_pszEntityType ? CEntityParser::m_pszEntityType : "(untyped)",
		CEntityParser::m_pszEntityName ? CEntityParser::m_pszEntityName : "(unnamed)" );
#endif
	// NKM
	if( pEntity && !fclib_stricmp( CEntityParser::m_pszEntityType, ENTITY_TYPE_GOODIE ) ) {
		((CCollectable *) pEntity )->Destroy();
		FMATH_CLEARBITMASK( pEntity->m_nEntityFlags, (ENTITY_FLAG_BUILDING | ENTITY_FLAG_CREATED) );
		CCollectable::ReturnCollectable( (CCollectable *) pEntity );
	} else {
		fdelete( pEntity );
		fres_ReleaseFrame( ResFrame );
	}

	return NULL;
}


// Tests the ray for collision against the terrain and CFWorldMesh objects that have their collision flag enabled and their line-of-site flag set.
//
// Returns TRUE if the ray is obstructed.
// Returns FALSE if the ray is not obstructed.
//
// ppTrackerSkipList and nTrackerSkipCount describe a buffer of CFWorldTracker's that will
// not be collided with.
//
// The collision impact buffer is cleared during this call.
BOOL CEntity::IsLOSObstructed(	const CFVec3A *pRayStart_WS, const CFVec3A *pRayEnd_WS,
								u64 uSkipEntityTypeMask,
								u32 nTrackerSkipCount, const CFWorldTracker * const * ppTrackerSkipList ) {

	return fworld_IsLineOfSightObstructed( pRayStart_WS, pRayEnd_WS, nTrackerSkipCount, ppTrackerSkipList, NULL, ~uSkipEntityTypeMask );
}

#if !FANG_PRODUCTION_BUILD
//-----------------------------------------------------------------------------
// convert an entity's WS position to screen-space.
// filter out entities not visible from player camera.
// return TRUE if visible from player camera.
static BOOL _GetScreenPos( CEntity *pEntity, CFVec3A &rScreenPos )
{
	const FViewport_t * pView = gamecam_GetActiveCamera()->GetViewport();
	BOOL bInView = FALSE;
	CFVec3A vPoint_VS = pEntity->MtxToWorld()->m_vPos;
	const CFXfm *pCamXfm = fcamera_GetCameraByIndex( 0 )->GetXfmWithoutShake();

	if( pView == NULL || pCamXfm == NULL )
	{
		return FALSE;
	}

	pCamXfm->m_MtxF.MulPoint(vPoint_VS);
	if (vPoint_VS.z > 0.0f)
	{
		// entity is located in positive Z view space

		// calc screen coordinate
		CFVec3A vLoc = pEntity->MtxToWorld()->m_vPos;
		rScreenPos.z = -1.0f;
		fviewport_ComputeUnitOrtho3DScreenPoint_WS(pView, &pCamXfm->m_MtxF, &vLoc, &rScreenPos);
		if (rScreenPos.z >0.0f && rScreenPos.x < 1.0f && rScreenPos.x > -1.0f && rScreenPos.y < 1.0f && rScreenPos.y > -1.0f)
		{
			// point is on-screen

			// convert screen coord to format used by ftext_DebugPrintf
			rScreenPos.x = 0.5f + 0.5f * rScreenPos.x;
			rScreenPos.y = 0.375f - 0.375f * rScreenPos.y;
			bInView = TRUE;
		}
	}

	if( bInView )
	{
		CFVec3A vRayStart, vRayEnd, vCamPosA;

		// cast a ray to test visibility
		vCamPosA.Set( *(gamecam_GetActiveCamera()->GetPos()) );
		vRayStart.Set( vCamPosA );
		vRayEnd.Set( pEntity->MtxToWorld()->m_vPos );
		vRayEnd.y += 0.5f;	// lift ray end up a bit
		FWorld_nTrackerSkipListCount = 0;
		pEntity->AppendTrackerSkipList();

		bInView = !CEntity::IsLOSObstructed( &vRayStart, &vRayEnd, 0, FWorld_nTrackerSkipListCount, FWorld_apTrackerSkipList );
  
//		bInView = !fworld_IsLineOfSightObstructed( &vRayStart, &vRayEnd, FWorld_nTrackerSkipListCount, FWorld_apTrackerSkipList );
	}

	return bInView;
}

// entity debug text modes
enum
{
	_DEBUG_TEXT_NONE,
	_DEBUG_TEXT_ALL,				// print all debug info for all entities
	_DEBUG_TEXT_ALL_MINIMAL,		// just print name of all entities
	_DEBUG_TEXT_FILTERED,			// print all info for a selected list of entity types
	_DEBUG_TEXT_FILTERED_MINIMAL,	// print just the name of a selected list of entity types
	_DEBUG_TEXT_MAX,
};

//-----------------------------------------------------------------------------
// print debug text info for pEntity at ScreenPos.
// nMode specifies what info to print
static void _PrintDebugInfo( CEntity *pEntity, CFVec3A &ScreenPos, u32 nMode )
{
	int nBit = 0;
	u64 nVal = pEntity->LeafTypeBit();
	cchar *pszInOut;
	cchar *pszDraw;
	CFVec3A *pPos;

	if( nMode == _DEBUG_TEXT_FILTERED || nMode == _DEBUG_TEXT_FILTERED_MINIMAL )
	{
		switch( pEntity->LeafTypeBit() )
		{
			// in filtered modes, debug info will be displayed for these types
			case ENTITY_BIT_BOT:
			case ENTITY_BIT_BOTGLITCH:
			case ENTITY_BIT_BOTGRUNT:
			case ENTITY_BIT_BOTTITAN:
			case ENTITY_BIT_BOTPRED:
			case ENTITY_BIT_SITEWEAPON:
			case ENTITY_BIT_MESHENTITY:
			case ENTITY_BIT_DOOR:
			case ENTITY_BIT_SWITCH:
			case ENTITY_BIT_SPLINE:
			case ENTITY_BIT_SPHERE:
			case ENTITY_BIT_LINE:
			case ENTITY_BIT_BOX:
			case ENTITY_BIT_GOODIE:
			case ENTITY_BIT_CONSOLE:
			case ENTITY_BIT_ZIPLINE:
			case ENTITY_BIT_BOTPROBE:
			case ENTITY_BIT_BOTJUMPER:
			case ENTITY_BIT_BOTSCOUT:
			case ENTITY_BIT_BOTMINER:
			case ENTITY_BIT_BOTCORROSIVE:
				break;

			default:
				return;
		}
	}

	if( nMode == _DEBUG_TEXT_ALL_MINIMAL || nMode == _DEBUG_TEXT_FILTERED_MINIMAL )
	{
		pszInOut = "";
	}
	else if( pEntity->IsInWorld() )
	{
		pszInOut = " In";
	}
	else
	{
		pszInOut = " Out";
	}

	if( nMode == _DEBUG_TEXT_ALL_MINIMAL || nMode == _DEBUG_TEXT_FILTERED_MINIMAL )
	{
		pszDraw = "";
	}
	else if( pEntity->IsDrawEnabled() )
	{
		pszDraw = " Draw";
	}
	else
	{
		pszDraw = "";
	}

	if( pEntity->Name() )
	{
		ftext_DebugPrintf( ScreenPos.x, ScreenPos.y, "~w0%s%s%s", pEntity->Name(), pszInOut, pszDraw );
	}
	else
	{
		ftext_DebugPrintf( ScreenPos.x, ScreenPos.y, "~w0no name %s%s", pszInOut, pszDraw );
	}

	if( nMode == _DEBUG_TEXT_ALL_MINIMAL || nMode == _DEBUG_TEXT_FILTERED_MINIMAL )
	{
		return;
	}

	ScreenPos.y += 0.02f;
	while( nVal>>=1 ) nBit++;
	ftext_DebugPrintf( ScreenPos.x, ScreenPos.y, "~w0%s", pEntity->m_apszEntityType[ nBit ] );

	ScreenPos.y += 0.02f;	
	pPos = &pEntity->MtxToWorld()->m_vPos;
	ftext_DebugPrintf( ScreenPos.x, ScreenPos.y, "~s0.80~w1%6.1f,%6.1f,%6.1f", pPos->x, pPos->y, pPos->z );
}
#endif // FANG_PRODUCTION_BUILD

// position of debug mode display
#define _DEBUG_MODE_TEXT_X	( 0.08f )
#define _DEBUG_MODE_TEXT_Y	( 0.16f )

//-----------------------------------------------------------------------------
// Prints debug text info for all entities.
// Also implements debug checkpoint load/save
// and debug player death enable/disable.
f32 fDisplayDeathEnabledTimer = 0.0f;
void CEntity::DrawAllDebugText( void )
{
//MEFIX
#if 0 //FANG_PRODUCTION_BUILD && FANG_ENABLE_DEV_FEATURES
	// use the player controller
	s32 nControllerId = Player_aPlayer[0].m_nControllerIndex;

	CBot *pBot = (CBot*) Player_aPlayer[0].m_pEntityOrig;
	if( Gamepad_aapSample[nControllerId][GAMEPAD_MAIN_HUD_SWITCH]->uLatches & GAMEPAD_BUTTON_1ST_PRESS_MASK )
	{
		pBot->m_bAllowPlayerDeath = !pBot->m_bAllowPlayerDeath;
		fDisplayDeathEnabledTimer = 4.0f;
		if (pBot->m_bAllowPlayerDeath)
		{
			pBot->SetNormHealth(1.0f);
		}
	}
	if (fDisplayDeathEnabledTimer >= 0.0f)
	{
		fDisplayDeathEnabledTimer -= FLoop_fPreviousLoopSecs;
	}

	if (fDisplayDeathEnabledTimer > 0.0f || ((f32) (FLoop_nTotalLoopTicks*FLoop_fSecsPerTick) < 4.0f))
	{
		if( pBot->m_bAllowPlayerDeath )
		{
			ftext_DebugPrintf( _DEBUG_MODE_TEXT_X, _DEBUG_MODE_TEXT_Y + 0.02f, "~w1Player Death Enabled" );
		}
		else
		{
			ftext_DebugPrintf( _DEBUG_MODE_TEXT_X, _DEBUG_MODE_TEXT_Y + 0.02f, "~w1Player Death Disabled" );
		}
	}

#elif !FANG_PRODUCTION_BUILD
	CEntity *pEntity;
	CFVec3A ScreenPos;
	BOOL bInView;
	s32 nControllerId;
	s32 saveButton, restoreButton;
	static u32 __nMode = _DEBUG_TEXT_NONE;

	// use the debug controller index
	nControllerId = Gamepad_nDebugPortIndex;

	#if( FANG_PLATFORM_WIN )		
		saveButton = GAMEPAD_MAIN_MELEE;
		restoreButton = GAMEPAD_MAIN_MELEE2;
	#elif( FANG_PLATFORM_GC )
		saveButton = GAMEPAD_MAIN_SELECT_SECONDARY;
		restoreButton = GAMEPAD_MAIN_SELECT_PRIMARY;
	#elif( FANG_PLATFORM_XB )
		saveButton = GAMEPAD_MAIN_SELECT_SECONDARY;
		restoreButton = GAMEPAD_MAIN_SELECT_PRIMARY;
	#endif

	//-----------------------------------------------------------------------------
	// save/restore checkpoints with button press
	//-----------------------------------------------------------------------------
	#if !FANG_PRODUCTION_BUILD
		#if( FANG_PLATFORM_XB || FANG_PLATFORM_GC )
		// checkpoint load/save TEST ONLY -cjm
		if ( !PROTRACK_ISMENUON())
		{
			if( Gamepad_aapSample[nControllerId][saveButton]->uLatches & GAMEPAD_BUTTON_1ST_PRESS_MASK )
			{
				checkpoint_Save(1, TRUE);
			}
			if( Gamepad_aapSample[nControllerId][restoreButton]->uLatches & GAMEPAD_BUTTON_1ST_PRESS_MASK )
			{
				if( checkpoint_Saved( 1 ) )
				{
					checkpoint_Restore( 1, TRUE, "dev:checkpoint-button" );
				}
				else
				{
					checkpoint_Restore( 0, TRUE, "dev:checkpoint-button" );
				}
			}
		}
		#endif	//( FANG_PLATFORM_XB || FANG_PLATFORM_GC )
	#endif	//!FANG_PRODUCTION_BUILD


	//-----------------------------------------------------------------------------
	// toggle player invincibility with button press
	//-----------------------------------------------------------------------------
	#if( FANG_PLATFORM_XB || FANG_PLATFORM_GC )
		// toggle player death with white button of second controller
		CBot *pBot = (CBot*) Player_aPlayer[0].m_pEntityOrig;
		if( Gamepad_aapSample[nControllerId][GAMEPAD_MAIN_UP_EUK]->uLatches & GAMEPAD_BUTTON_1ST_PRESS_MASK )
		{
			pBot->m_bAllowPlayerDeath = !pBot->m_bAllowPlayerDeath;
			if (pBot->m_bAllowPlayerDeath)
			{
				pBot->SetNormHealth(1.0f);
			}
		}
		if( pBot->m_bAllowPlayerDeath )
		{
			#if !FANG_PRODUCTION_BUILD
				fperf_Render_AddPerfInt( FPERF_TYPE_ALL, "Death", 1, 1.0f );
			#else	
				ftext_DebugPrintf( _DEBUG_MODE_TEXT_X, _DEBUG_MODE_TEXT_Y + 0.02f, "~w1Player Death Enabled" );
			#endif
		}
	#endif


	//-----------------------------------------------------------------------------
	// print on-screen difficulty setting
	//-----------------------------------------------------------------------------
	#if !FANG_PRODUCTION_BUILD
		fperf_Render_AddPerfInt( FPERF_TYPE_ALL, "Difficulty", CDifficulty::GetLevel(), 0.0f );
	#endif


	//-----------------------------------------------------------------------------
	// print on-screen entity info text
	//-----------------------------------------------------------------------------
	#if( FANG_PLATFORM_WIN )
		// in windows, DrawAllDebugText() only runs if check-box is selected.
		// In this case, we always run _DEBUG_TEXT_ALL.
			__nMode = _DEBUG_TEXT_ALL;
	#else

	#endif
	// change modes via joystick button
	if( Gamepad_aapSample[nControllerId][GAMEPAD_MAIN_PAUSE]->uLatches & GAMEPAD_BUTTON_1ST_PRESS_MASK )
	{
		__nMode++;
		if( __nMode >= _DEBUG_TEXT_MAX )
		{
			__nMode = _DEBUG_TEXT_NONE;
		}
	}

	// bail if in "off" mode
	if( __nMode == _DEBUG_TEXT_NONE )
	{
		return;
	}

	// print debug mode display
	switch( __nMode )
	{
		case _DEBUG_TEXT_ALL:				// print all debug info for all entities
	#if !FANG_PRODUCTION_BUILD
			fperf_Render_AddPerfString( FPERF_TYPE_ALL, "Ents", "Info All", 1.0f );
	#else
			ftext_DebugPrintf( _DEBUG_MODE_TEXT_X, _DEBUG_MODE_TEXT_Y, "~w1Info All Ents" );
	#endif
		break;

		case _DEBUG_TEXT_ALL_MINIMAL:		// just print name of all entities
	#if !FANG_PRODUCTION_BUILD
			fperf_Render_AddPerfString( FPERF_TYPE_ALL, "Ents", "Name All", 1.0f );
	#else
			ftext_DebugPrintf( _DEBUG_MODE_TEXT_X, _DEBUG_MODE_TEXT_Y, "~w1Name All Ents" );
	#endif
		break;															    

		case _DEBUG_TEXT_FILTERED:			// print all info for a selected list of entity types
	#if !FANG_PRODUCTION_BUILD
			fperf_Render_AddPerfString( FPERF_TYPE_ALL, "Ents", "Info Some", 1.0f );
	#else
			ftext_DebugPrintf( _DEBUG_MODE_TEXT_X, _DEBUG_MODE_TEXT_Y, "~w1Info Select Ents" );
	#endif
		break;

		case _DEBUG_TEXT_FILTERED_MINIMAL:	// print just the name of a selected list of entity types
	#if !FANG_PRODUCTION_BUILD
			fperf_Render_AddPerfString( FPERF_TYPE_ALL, "Ents", "Name Some", 1.0f );
	#else
			ftext_DebugPrintf( _DEBUG_MODE_TEXT_X, _DEBUG_MODE_TEXT_Y, "~w1Name Select Ents" );
	#endif
		break;
	}

	// step through out-of-world objects and print debug text if visible
	for( pEntity=OutOfWorldList_GetHead(); pEntity; pEntity=pEntity->OutOfWorldList_GetNext() )
	{
		bInView = _GetScreenPos( pEntity, ScreenPos );
		if( bInView )
		{
			_PrintDebugInfo( pEntity, ScreenPos, __nMode );
		}
	}

	// step through in-world objects and print debug text if visible
	for( pEntity=InWorldList_GetHead(); pEntity; pEntity=pEntity->InWorldList_GetNext() )
	{
		bInView = _GetScreenPos( pEntity, ScreenPos );
		if( bInView )
		{
			_PrintDebugInfo( pEntity, ScreenPos, __nMode );
		}
	}
#endif   //	FANG_PRODUCTION_BUILD
}


#if 0
void CEntity::SaveWorldMeshState( CFWorldMesh *pWorldMesh ) {
	if( pWorldMesh ) {
		void *pSaveData;
		u32 nSaveDataBytes;

		pSaveData = pWorldMesh->GetSaveData( &nSaveDataBytes );
		if( nSaveDataBytes ) {
			CFCheckPoint::SaveData( pSaveData, nSaveDataBytes );
		}
	}
}


void CEntity::RestoreWorldMeshState( CFWorldMesh *pWorldMesh ) {
	if( pWorldMesh ) {
		void *pSaveData;

		pSaveData = pWorldMesh->GetSaveData( &nSaveDataBytes );
		if( nSaveDataBytes ) {
			CFCheckPoint::SaveData( pSaveData, nSaveDataBytes );
		}
	}
}
#endif



//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CEntityParser
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

FGameDataFileHandle_t CEntityParser::m_hRootFile;
cchar *CEntityParser::m_pszEntityType;
cchar *CEntityParser::m_pszEntityName;
cchar *CEntityParser::m_pszAIBuilderName;
const CFWorldShapeInit *CEntityParser::m_pWorldShapeInit;
u64 CEntityParser::m_nEntityTypeBits;
u64 CEntityParser::m_nEntityLeafTypeBit;

FGameDataTableHandle_t CEntityParser::m_hTable;
u32 CEntityParser::m_nFieldCount;
cchar *CEntityParser::m_pszTableName;
cchar *CEntityParser::m_pszFileName;

FGameDataWalker_t CEntityParser::m_Walker;



void CEntityParser::_Zero( void ) {
	m_hRootFile = FGAMEDATA_INVALID_FILE_HANDLE;
	m_pszEntityType = NULL;
	m_pszEntityName = NULL;
	m_pszAIBuilderName = NULL;
	m_pWorldShapeInit = NULL;
	m_nEntityTypeBits = 0;
	m_nEntityLeafTypeBit = 0;

	m_hTable = FGAMEDATA_INVALID_TABLE_HANDLE;
	m_nFieldCount = 0;
	m_pszTableName = NULL;
	m_pszFileName = NULL;
}


BOOL CEntityParser::Begin( const CFWorldShapeInit *pWorldShapeInit ) {
	_Zero();

	m_pWorldShapeInit = pWorldShapeInit;

	m_hRootFile = fgamedata_GetHandleFromLoadedFile( pWorldShapeInit->m_pGameData );
	if( m_hRootFile == FGAMEDATA_INVALID_FILE_HANDLE ) {
		return FALSE;
	}

	_SetEntityName();
	_SetEntityType();
	_SetEntityAIBuilder();

	return TRUE;
}


void CEntityParser::_SetEntityName( void ) {
	m_pszEntityName = NULL;

	if( m_hRootFile == FGAMEDATA_INVALID_FILE_HANDLE ) {
		// No file...
		return;
	}

	if( !SetFirstTable() ) {
		// No tables...
		return;
	}

	do {
		if( !fclib_stricmp( m_pszTableName, "name" ) ) {
			Interpret_String( &m_pszEntityName );
		}
	} while( SetNextTable() );
}


void CEntityParser::_SetEntityType( void ) {
	m_pszEntityType = NULL;

	if( m_hRootFile == FGAMEDATA_INVALID_FILE_HANDLE ) {
		// No file...
		return;
	}

	if( !SetFirstTable() ) {
		// No tables...
		return;
	}

	do {
		if( !fclib_stricmp( m_pszTableName, "type" ) ) {
			Interpret_String( &m_pszEntityType );
		}
	} while( SetNextTable() );
}


void CEntityParser::_SetEntityAIBuilder( void ) {
	m_pszAIBuilderName = NULL;

	if( m_hRootFile == FGAMEDATA_INVALID_FILE_HANDLE ) {
		// No file...
		return;
	}

	if( !SetFirstTable() ) {
		// No tables...
		return;
	}

	do {
		if( !fclib_stricmp( m_pszTableName, "AI" ) ) {
			Interpret_String( &m_pszAIBuilderName );
		}
	} while( SetNextTable() );
}


// Returns FALSE if the specified table does not exist.
BOOL CEntityParser::SetTable( FGameDataTableHandle_t hTable ) {
	if( m_hRootFile == FGAMEDATA_INVALID_FILE_HANDLE ) {
		// No file...
		return FALSE;
	}

	if( hTable != FGAMEDATA_INVALID_TABLE_HANDLE ) {
		m_hTable = hTable;
		m_pszFileName = fgamedata_GetFileNameFromTableHandle( m_hTable );
		m_pszTableName = fgamedata_GetTableName( m_hTable );
		m_nFieldCount = fgamedata_GetNumFields( m_hTable );

		return TRUE;
	} else {
		m_hTable = FGAMEDATA_INVALID_TABLE_HANDLE;
		m_pszFileName = NULL;
		m_pszTableName = NULL;
		m_nFieldCount = 0;

		return FALSE;
	}
}


// Returns FALSE if there are no tables.
BOOL CEntityParser::SetFirstTable( void ) {
	if( m_hRootFile == FGAMEDATA_INVALID_FILE_HANDLE ) {
		// No file...
		return FALSE;
	}

	if( m_hRootFile != FGAMEDATA_INVALID_FILE_HANDLE ) {
		return SetTable( fgamedata_GetFirstTable( m_hRootFile, m_Walker ) );
	} else {
		return SetTable( FGAMEDATA_INVALID_TABLE_HANDLE );
	}
}


// Returns FALSE if there are no more tables.
BOOL CEntityParser::SetNextTable( void ) {
	if( m_hRootFile == FGAMEDATA_INVALID_FILE_HANDLE ) {
		// No file...
		return FALSE;
	}

	return SetTable( fgamedata_GetNextTable( m_Walker ) );
}


// Returns FALSE if syntax error. In this case, *ppszString is not modified.
BOOL CEntityParser::Interpret_String( cchar **ppszString, BOOL bCheckFieldCountOne, u32 nFieldIndex, BOOL bNoneToNull ) {
	FGameData_VarType_e nVarType;
	cchar *pszString;

	if( bCheckFieldCountOne ) {
		if( m_nFieldCount != 1 ) {
			Error_InvalidParameterCount();
			goto _ExitWithError;
		}
	}

	if( nFieldIndex >= fgamedata_GetNumFields( m_hTable ) ) {
		Error_InvalidParameterCount();
		goto _ExitWithError;
	}

	pszString = (cchar *)fgamedata_GetPtrToFieldData( m_hTable, nFieldIndex, nVarType );

	if( nVarType != FGAMEDATA_VAR_TYPE_STRING ) {
		Error_InvalidParameterType();
		goto _ExitWithError;
	}

	if( ppszString ) {
		if( bNoneToNull && !fclib_stricmp( pszString, "None" ) ) {
			*ppszString = NULL;
		} else {
			*ppszString = pszString;
		}
	}

	return TRUE;

_ExitWithError:
	return FALSE;
}


// Fills ppszStringArray with pointers to strings from the current table.
// No more than nMaxArrayElements will be filled.
// No more than CEntityParser::m_nFieldCount elements will be filled.
//
// If any of the fields is not a string, those array elements will be set to NULL,
// and the parsing continues.
//
// If any of the fields is not a string, FALSE is returned.
BOOL CEntityParser::Interpret_StringArray( cchar **ppszStringArray, u32 nMaxArrayElements, u32 nFieldIndex, BOOL bNoneToNull ) {
	FGameData_VarType_e nVarType;
	cchar *pszString;
	u32 i, nLoopCount;
	BOOL bSuccess = TRUE;

	for( i=0; i<nMaxArrayElements; i++ ) {
		ppszStringArray[i] = NULL;
	}

	nLoopCount = FMATH_MIN( nMaxArrayElements, m_nFieldCount );

	for( i=0; i<nLoopCount; i++ ) {
		if( (nFieldIndex + i) >= m_nFieldCount ) {
			break;
		}

		pszString = (cchar *)fgamedata_GetPtrToFieldData( m_hTable, nFieldIndex + i, nVarType );

		if( nVarType != FGAMEDATA_VAR_TYPE_STRING ) {
			Error_InvalidParameterType();
			bSuccess = FALSE;
		} else {
			if( bNoneToNull && !fclib_stricmp( pszString, "None" ) ) {
				ppszStringArray[i] = NULL;
			} else {
				ppszStringArray[i] = pszString;
			}
		}
	}

	return bSuccess;
}


// Copies pszSrcName to FMem, but removes any file extension (including the '.') if so desired.
// Returns TRUE if successful, or FALSE if out of memory.
BOOL CEntityParser::Interpret_CopyStringToFMem( cchar *pszSrcName, char **ppszFMemName, BOOL bRemoveExtension ) {
	char *pszFMemName;
	u32 nStringLength;

	FMemFrame_t MemFrame = fmem_GetFrame();

	if( bRemoveExtension ) {
		// Find first occurance of '.'...
		for( nStringLength=0; pszSrcName[nStringLength] && pszSrcName[nStringLength]!='.'; ++nStringLength );
	} else {
		// Copy entire string...
		nStringLength = fclib_strlen( pszSrcName );
	}

	pszFMemName = (char *)fmem_Alloc( nStringLength + 1 );
	if( pszFMemName == NULL ) {
		goto _OutOfMemory;
	}

	fclib_strncpy( pszFMemName, pszSrcName, nStringLength );
	pszFMemName[nStringLength] = 0;

	*ppszFMemName = pszFMemName;

	return TRUE;

_OutOfMemory:
	Error_OutOfMemory();
	fmem_ReleaseFrame( MemFrame );
	return FALSE;
}


// Same as Interpret_CopyStringToFMem(), but also replaces the last two characters with "**".
BOOL CEntityParser::Interpret_CopyWildcardStringToFMem( cchar *pszSrcName, char **ppszWildcardFMemName, BOOL bRemoveExtension ) {
	u32 nStringLength;

	if( !Interpret_CopyStringToFMem( pszSrcName, ppszWildcardFMemName, bRemoveExtension ) ) {
		return FALSE;
	}

	nStringLength = fclib_strlen( *ppszWildcardFMemName );

	if( nStringLength >= 2 ) {
		(*ppszWildcardFMemName)[nStringLength-2] = '*';
		(*ppszWildcardFMemName)[nStringLength-1] = '*';
	}

	return TRUE;
}


// Checks the syntax of the current table for a valid array of strings.
// If the syntax check passes, the current table handle is stored in *phTable and TRUE is returned.
// If the syntax check fails, *phTable is unmodified, and FALSE is returned.
//
// This function is usually called by the entity builder's InterpretTable() function.
BOOL CEntityParser::Interpret_StringArray_CheckSyntax( FGameDataTableHandle_t *phTable ) {
	FGameData_VarType_e nVarType;
	u32 i;

	for( i=0; i<m_nFieldCount; i++ ) {
		fgamedata_GetPtrToFieldData( m_hTable, i, nVarType );

		if( nVarType != FGAMEDATA_VAR_TYPE_STRING ) {
			Error_InvalidParameterType();
			goto _SytaxError;
		}
	}

	// Sytax checks out ok...

	if( phTable ) {
		*phTable = m_hTable;
	}

	return TRUE;

_SytaxError:
	return FALSE;
}


// This function allocates temporary (fmem) memory to hold an array of string pointers
// and sets the pointers to point into the game data.
//
// If successful, *pppszStringArray points to the array of string pointers (in fmem),
// *pnArrayCount is set to the number of elements in the array, and TRUE is returned.
// If not successful, *pppszStringArray and *pnArrayCount are unmodified and FALSE is returned.
//
// This function is usually called by the entity builder's PostInterpretFixup() function.
BOOL CEntityParser::Interpret_StringArray_BuildInFMem( cchar ***pppszStringArray ) {
	FGameData_VarType_e nVarType;
	cchar **ppszStringArray;
	u32 i;

	FMemFrame_t MemFrame = fmem_GetFrame();

	ppszStringArray = (cchar **)fmem_Alloc( sizeof(cchar *) * m_nFieldCount );
	if( ppszStringArray == NULL ) {
		goto _OutOfMemory;
	}

	for( i=0; i<m_nFieldCount; i++ ) {
		ppszStringArray[i] = (cchar *)fgamedata_GetPtrToFieldData( m_hTable, i, nVarType );
		FASSERT( nVarType == FGAMEDATA_VAR_TYPE_STRING );
	}

	if( pppszStringArray ) {
		*pppszStringArray = ppszStringArray;
	}

	return TRUE;

_OutOfMemory:
	Error_OutOfMemory();
	fmem_ReleaseFrame( MemFrame );
	return FALSE;
}


// Checks the syntax of the current table for a valid array of floats.
// If the syntax check passes, the current table handle is stored in *phTable and TRUE is returned.
// If the syntax check fails, *phTable is unmodified, and FALSE is returned.
//
// This function is usually called by the entity builder's InterpretTable() function.
BOOL CEntityParser::Interpret_F32Array_CheckSyntax( FGameDataTableHandle_t *phTable ) {
	FGameData_VarType_e nVarType;
	u32 i;

	for( i=0; i<m_nFieldCount; i++ ) {
		fgamedata_GetPtrToFieldData( m_hTable, i, nVarType );

		if( nVarType != FGAMEDATA_VAR_TYPE_FLOAT ) {
			Error_InvalidParameterType();
			goto _SyntaxError;
		}
	}

	// Syntax checks out ok...

	if( phTable ) {
		*phTable = m_hTable;
	}

	return TRUE;

_SyntaxError:
	return FALSE;
}


BOOL CEntityParser::CompareBoolString( cchar *pszBoolString, BOOL *pbBool ) {
	if( !fclib_stricmp( pszBoolString, "True" ) || !fclib_stricmp( pszBoolString, "Yes" ) || !fclib_stricmp( pszBoolString, "On" ) || !fclib_stricmp( pszBoolString, "1" ) ) {
		if( pbBool ) {
			*pbBool = TRUE;
		}

		return TRUE;
	} else if( !fclib_stricmp( pszBoolString, "False" ) || !fclib_stricmp( pszBoolString, "No" ) || !fclib_stricmp( pszBoolString, "Off" ) || !fclib_stricmp( pszBoolString, "0" ) ) {
		if( pbBool ) {
			*pbBool = FALSE;
		}

		return TRUE;
	} else {
		return FALSE;
	}
}


// nTableFieldIndex is the table field index to interpret.
// If nTableFieldIndex is -1 then this function expects there to be only
// a single field in the table, and will report an error if not.
//
// Returns FALSE if syntax error. In this case, *pnData is not modified.
BOOL CEntityParser::Interpret_BOOL( BOOL *pbData, BOOL bInvertValue, s32 nTableFieldIndex ) {
	if( pbData ) {
		*pbData = FALSE;
		return Interpret_Flag( (u32 *)pbData, 1, bInvertValue, nTableFieldIndex );
	}

	return TRUE;
}


// nTableFieldIndex is the table field index to interpret.
// If nTableFieldIndex is -1 then this function expects there to be only
// a single field in the table, and will report an error if not.
//
// Returns FALSE if syntax error. In this case, *pnData is not modified.
BOOL CEntityParser::Interpret_Flag( u32 *pnData, u32 nBitMask, BOOL bInvertValue, s32 nTableFieldIndex ) {
	FGameData_VarType_e nVarType;
	const void *pData;
	cchar *pszString;
	f32 fData;
	BOOL bData;

	if( nTableFieldIndex < 0 ) {
		nTableFieldIndex = 0;

		if( m_nFieldCount != 1 ) {
			Error_InvalidParameterCount();
			goto _ExitWithError;
		}
	}

	pData = fgamedata_GetPtrToFieldData( m_hTable, nTableFieldIndex, nVarType );

	switch( nVarType ) {
	case FGAMEDATA_VAR_TYPE_FLOAT:
		fData = *(const f32 *)pData;

		if( fData == 1.0f ) {
			bData = TRUE;
		} else if( fData == 0.0f ) {
			bData = FALSE;
		} else {
			Error_InvalidParameterValue();
			goto _ExitWithError;
		}

		break;

	case FGAMEDATA_VAR_TYPE_STRING:
		pszString = (cchar *)pData;

		if( !CompareBoolString( pszString, &bData ) ) {
			goto _ExitWithError;
		}

		break;

	default:
		Error_InvalidParameterType();
		goto _ExitWithError;
	}

	if( pnData ) {
		if( bInvertValue ) {
			bData = !bData;
		}

		if( bData ) {
			*pnData |= nBitMask;
		} else {
			*pnData &= ~nBitMask;
		}
	}

	return TRUE;

_ExitWithError:
	return FALSE;
}


// Returns FALSE if syntax error. In this case, *pnData is not modified.
BOOL CEntityParser::Interpret_U32( u32 *pnData, u32 nMinValue, u32 nMaxValue, BOOL bClamp ) {
	FGameData_VarType_e nVarType;
	f32 fData;
	u32 nData;

	if( m_nFieldCount != 1 ) {
		Error_InvalidParameterCount();
		goto _ExitWithError;
	}

	fData = *(const f32 *)fgamedata_GetPtrToFieldData( m_hTable, 0, nVarType );

	if( nVarType != FGAMEDATA_VAR_TYPE_FLOAT ) {
		Error_InvalidParameterType();
		goto _ExitWithError;
	}

	if( bClamp ) {
		FMATH_CLAMP( fData, 0.0f, 4294967295.0f );
		nData = (u32)fData;
		FMATH_CLAMP( nData, nMinValue, nMaxValue );
	} else {
		if( fData<0.0f || fData>4294967295.0f ) {
			Error_InvalidParameterRange();
			goto _ExitWithError;
		}

		nData = (u32)fData;

		if( nData<nMinValue || nData>nMaxValue ) {
			Error_InvalidParameterRange();
			goto _ExitWithError;
		}
	}

	if( pnData ) {
		*pnData = nData;
	}

	return TRUE;

_ExitWithError:
	return FALSE;
}


// Returns FALSE if syntax error. In this case, *pnData is not modified.
BOOL CEntityParser::Interpret_S32( s32 *pnData, s32 nMinValue, s32 nMaxValue, BOOL bClamp ) {
	FGameData_VarType_e nVarType;
	f32 fData;
	s32 nData;

	if( m_nFieldCount != 1 ) {
		Error_InvalidParameterCount();
		goto _ExitWithError;
	}

	fData = *(const f32 *)fgamedata_GetPtrToFieldData( m_hTable, 0, nVarType );

	if( nVarType != FGAMEDATA_VAR_TYPE_FLOAT ) {
		Error_InvalidParameterType();
		goto _ExitWithError;
	}

	if( bClamp ) {
		FMATH_CLAMP( fData, -2147483648.0f, 2147483647.0f );
		nData = (s32)fData;
		FMATH_CLAMP( nData, nMinValue, nMaxValue );
	} else {
		if( fData<-2147483648.0f || fData>2147483647.0f ) {
			Error_InvalidParameterRange();
			goto _ExitWithError;
		}

		nData = (s32)fData;

		if( nData<nMinValue || nData>nMaxValue ) {
			Error_InvalidParameterRange();
			goto _ExitWithError;
		}
	}

	if( pnData ) {
		*pnData = nData;
	}

	return TRUE;

_ExitWithError:
	return FALSE;
}


// Returns FALSE if syntax error. In this case, *pnData is not modified.
BOOL CEntityParser::Interpret_F32( f32 *pfData, f32 fMinValue, f32 fMaxValue, BOOL bClamp ) {
	FGameData_VarType_e nVarType;
	f32 fData;

	if( m_nFieldCount != 1 ) {
		Error_InvalidParameterCount();
		goto _ExitWithError;
	}

	fData = *(const f32 *)fgamedata_GetPtrToFieldData( m_hTable, 0, nVarType );

	if( nVarType != FGAMEDATA_VAR_TYPE_FLOAT ) {
		Error_InvalidParameterType();
		goto _ExitWithError;
	}

	if( bClamp ) {
		FMATH_CLAMP( fData, fMinValue, fMaxValue );
	} else {
		if( fData<fMinValue || fData>fMaxValue ) {
			Error_InvalidParameterRange();
			goto _ExitWithError;
		}
	}

	if( pfData ) {
		*pfData = fData;
	}

	return TRUE;

_ExitWithError:
	return FALSE;
}


// Fills pfDestArray with float values from the current table.
// No more than nMaxArrayElements will be filled.
// No more than CEntityParser::m_nFieldCount elements will be filled.
//
// If any of the fields is not a f32, those array elements will be set to 0.0f,
// and the parsing continues.
//
// If any of the fields is not a f32, FALSE is returned.
BOOL CEntityParser::Interpret_F32Array( f32 *pfDestArray, u32 nMaxArrayElements, f32 fMinValue, f32 fMaxValue, BOOL bClamp, u32 nStartFieldIndex ) {
	FGameData_VarType_e nVarType;
	f32 *pfVal, fVal;
	u32 i, nLoopCount;
	BOOL bSuccess = TRUE;

	for( i=0; i<nMaxArrayElements; i++ ) {
		pfDestArray[i] = 0.0f;
	}

	FASSERT( nStartFieldIndex < m_nFieldCount );

	nLoopCount = FMATH_MIN( nMaxArrayElements, (m_nFieldCount - nStartFieldIndex) );

	for( i=0; i<nLoopCount; i++ ) {
		pfVal = (f32 *)fgamedata_GetPtrToFieldData( m_hTable, nStartFieldIndex + i, nVarType );

		if( nVarType != FGAMEDATA_VAR_TYPE_FLOAT ) {
			Error_InvalidParameterType();
			bSuccess = FALSE;
			continue;
		}

		fVal = *pfVal;

		if( bClamp ) {
			FMATH_CLAMP( fVal, fMinValue, fMaxValue );
		} else {
			if( fVal<fMinValue || fVal>fMaxValue ) {
				Error_InvalidParameterRange();
				bSuccess = FALSE;
				continue;
			}
		}

		pfDestArray[i] = fVal;
	}

	return bSuccess;
}


void CEntityParser::Error_OutOfMemory( void ) {
	DEVPRINTF( "CEntityParser: Out of memory during entity parsing work.\n" );
}


void CEntityParser::Error_UnknownCommand( void ) {
	if( fclib_stricmp(m_pszTableName, "myLayer") ) {
		Error_Prefix();
		DEVPRINTF( "Unknown command '%s'.\n", m_pszTableName );
		Error_Dashes();
	}
}


void CEntityParser::Error_InvalidParameterCount( void ) {
	Error_Prefix();
	DEVPRINTF( "Invalid number of parameters for command '%s'.\n", m_pszTableName );
	Error_Dashes();
}


void CEntityParser::Error_InvalidParameterRange( void ) {
	Error_Prefix();
	DEVPRINTF( "Invalid parameter range for command '%s'.\n", m_pszTableName );
	Error_Dashes();
}


void CEntityParser::Error_InvalidParameterValue( void ) {
	Error_Prefix();
	DEVPRINTF( "Invalid parameter value for command '%s'.\n", m_pszTableName );
	Error_Dashes();
}


void CEntityParser::Error_InvalidParameterType( void ) {
	Error_Prefix();
	DEVPRINTF( "Invalid parameter type for command '%s'.\n", m_pszTableName );
	Error_Dashes();
}


void CEntityParser::Error_CommandAlreadySpecified( void ) {
	Error_Prefix();
	DEVPRINTF( "Command '%s' can be specified only once.\n", m_pszTableName );
	Error_Dashes();
}


void CEntityParser::Error_InvalidObjectType( void ) {
	Error_Prefix();
	DEVPRINTF( "Unrecognized object type '%s'.\n", m_pszEntityType );
	Error_Dashes();
}


void CEntityParser::Error_CouldNotCreateAIBrain( void ) {
	Error_Prefix();
	DEVPRINTF( "Could not create AI Brain.\n", m_pszEntityType );
	Error_Dashes();
}


void CEntityParser::Error_Prefix( void ) {
	Error_Dashes();
	DEVPRINTF( "ATTENTION LEVEL DESIGNERS AND ART TEAM:  ERROR!!!\n" );
	DEVPRINTF( "\n" );

	if( m_pszFileName ) {
		DEVPRINTF( "Error interpreting CSV file '%s'\n", m_pszFileName );
	} else {
		if( Level_nLoadedIndex>=0 && Level_aInfo[Level_nLoadedIndex].pszWorldResName ) {
			DEVPRINTF( "Error interpreting Max User Properties in world file '%s'.\n", Level_aInfo[Level_nLoadedIndex].pszWorldResName );
		} else {
			DEVPRINTF( "Error interpreting Max User Properties.\n" );
		}
	}

	if( m_pszEntityName ) {
		DEVPRINTF( "Entity name = '%s'\n", m_pszEntityName );
	} else {
		DEVPRINTF( "Entity name is unknown.\n" );
	}

	if( m_pWorldShapeInit ) {
		DEVPRINTF( "Object XYZ (Max space) = ( %.2f, %.2f, %.2f )\n", m_pWorldShapeInit->m_Mtx43.m_vPos.x, m_pWorldShapeInit->m_Mtx43.m_vPos.z, m_pWorldShapeInit->m_Mtx43.m_vPos.y );
	} else {
		DEVPRINTF( "Object position is unknown.\n" );
	}

	DEVPRINTF( "\n" );
}


void CEntityParser::Error_Dashes( void ) {
	DEVPRINTF( "-------------------------------------------------\n" );
}

// queries all in-world entities and calls callbacks if the entity's mesh just entered or left the active list
void CEntity::CallActiveListCallbacks( void )
{
	CEntity *pEntity;
	CFWorldMesh *pMesh;

	for( pEntity=InWorldList_GetHead(); pEntity; pEntity=pEntity->InWorldList_GetNext() )
	{
		pMesh = pEntity->GetMesh();
		if( pMesh )
		{
			if( pMesh->VolumeBecameActive() )
			{
				pEntity->ClassHierarchyBecameActive();
			}
			else if( pMesh->VolumeBecameInactive() )
			{
				pEntity->ClassHierarchyBecameInactive();
			}
		}
	}
}

// called if entity's mesh just entered active list
void CEntity::ClassHierarchyBecameActive( void )
{
}

// called if entity's mesh just entered active list
void CEntity::ClassHierarchyBecameInactive( void )
{
}


u64 CEntity::TypeBitsRecurseParents( void )		   const
{
	u64 uBits = 0;

	if (GetParent())
	{
		uBits |= GetParent()->TypeBitsRecurseParents();
	}

	return TypeBits() | uBits;
}


CEntity* CEntity::GetParentOfType( u64 uTypeBits ) 
{
	if (TypeBits() & uTypeBits)
	{
		return this;
	}
	else if (GetParent())
	{
		return GetParent()->GetParentOfType(uTypeBits);
	}

	return NULL;
}
