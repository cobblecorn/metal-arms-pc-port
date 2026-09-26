//////////////////////////////////////////////////////////////////////////////////////
// fverlet.cpp - Verlet physics module.
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
// 11/21/02 Ranck       Created.
//////////////////////////////////////////////////////////////////////////////////////

#include "fang.h"
#include "fverlet.h"
#include "fmesh.h"
#include "floop.h"
#include "fclib.h"
#include "fdraw.h"
#include "frenderer.h"
#include "fwire.h"


#define _DRAW_DEBUG_INFO		FALSE


//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CFVerlet
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

#define _PHYSICS_WORK_DELTA_SECS	(1.0f / 60.0f)
#define _MOTIONLESS_COUNTDOWN_SECS	1.0f
#define _MOTIONLESS_SPEED			0.3f
#define _IMPULSE_DURATION			(1.0f / 60.0f)
#define _PRIME_DELTA_SECS			(1.0f / 60.0f)


BOOL CFVerlet::m_bSystemInitialized;
f32 CFVerlet::m_fSecsUntilNextWork;


const CFVec3A CFVerlet::m_aTetraPos_MS[4] = {
	CFVec3A( -1.0f/3.0f, -0.25f, -0.5f ),
	CFVec3A(  2.0f/3.0f, -0.25f,  0.0f ),
	CFVec3A( -1.0f/3.0f, -0.25f,  0.5f ),
	CFVec3A(       0.0f,  0.75f,  0.0f ),
};


const CFVec3A CFVerlet::m_aRegularTetraPos_MS[4] = {
	CFVec3A( -0.5f,	-0.20412414523193150818310700622549f, -0.28867513459481288225457439025098f	),
	CFVec3A(  0.5f,	-0.20412414523193150818310700622549f, -0.28867513459481288225457439025098f	),
	CFVec3A(  0.0f,	-0.20412414523193150818310700622549f,  0.57735026918962576450914878050196f	),
	CFVec3A(  0.0f,	 0.61237243569579452454932101867647f,  0.0f									),
};



const CFVec3A CFVerlet::m_QuarterVec( 0.25f, 0.25f, 0.25f );

FLinkRoot_t CFVerlet::m_InactiveNetsRoot;
FLinkRoot_t CFVerlet::m_ActiveNetsRoot;

CFVerlet **CFVerlet::m_ppVerletWorkArray;
CFVerlet **CFVerlet::m_ppNetWorkArray;
u32 CFVerlet::m_nWorkArrayCount;
u32 CFVerlet::m_nVerletWorkCount;
u32 CFVerlet::m_nNetWorkCount;

cchar **CFVerlet::m_apszTackNameArray;
cchar **CFVerlet::m_apszCTackNameArray;

CFVerlet* CFVerlet::m_pCBVerlet	= NULL;


BOOL CFVerlet::ModuleStartup( void ) {
	FASSERT( !IsModuleInitialized() );

	CFVerletTack::m_nBoneTackPrefixLength = fclib_strlen( FVERLET_MESH_BONE_TACK_PREFIX );
	CFVerletTack::m_nBoneCTackPrefixLength = fclib_strlen( FVERLET_MESH_BONE_CTACK_PREFIX );

	flinklist_InitRoot( &m_InactiveNetsRoot, FANG_OFFSETOF( CFVerlet, m_NetMasterLink ) );
	flinklist_InitRoot( &m_ActiveNetsRoot, FANG_OFFSETOF( CFVerlet, m_NetMasterLink ) );

	m_nWorkArrayCount = Fang_ConfigDefs.nVerlet_MaxCount;
	FMATH_CLAMPMIN( m_nWorkArrayCount, 1 );

	m_ppVerletWorkArray = (CFVerlet **)fres_Alloc( sizeof(CFVerlet *) * m_nWorkArrayCount );
	if( m_ppVerletWorkArray == NULL ) {
		DEVPRINTF( "CFVerlet::ModuleStartup(): Not enough memory to allocate m_ppVerletWorkArray.\n" );
		return FALSE;
	}

	m_ppNetWorkArray = (CFVerlet **)fres_Alloc( sizeof(CFVerlet *) * m_nWorkArrayCount );
	if( m_ppNetWorkArray == NULL ) {
		DEVPRINTF( "CFVerlet::ModuleStartup(): Not enough memory to allocate m_ppNetWorkArray.\n" );
		return FALSE;
	}

	m_bSystemInitialized = TRUE;

	SetBoneNameTackTable( NULL, NULL );

	Reset();

	return TRUE;
}


void CFVerlet::ModuleShutdown( void ) {
	if( IsModuleInitialized() ) {
		SetBoneNameTackTable( NULL, NULL );

		m_ppVerletWorkArray = NULL;
		m_ppNetWorkArray = NULL;
		m_nWorkArrayCount = 0;

		m_bSystemInitialized = FALSE;
	}
}


void CFVerlet::Reset( void ) {
	FASSERT( m_bSystemInitialized );

	m_fSecsUntilNextWork = 0.0f;
}


void CFVerlet::WorkAll( void ) {
	FASSERT( IsModuleInitialized() );

	if( FLoop_bGamePaused ) {
		return;
	}

	CFVerlet *pNetMasterVerlet;
	u32 nNetWorkIndex, nPass, nPassCount;
	f32 fDeltaSecs;
	BOOL bFirstPass, bLastPass;

	fDeltaSecs = FLoop_fPreviousLoopSecs;
	FMATH_CLAMPMAX( fDeltaSecs, Fang_ConfigDefs.fVerlet_MaxDeltaSecs );

	m_fSecsUntilNextWork -= fDeltaSecs;
	if( m_fSecsUntilNextWork > 0.0f ) {
		// Not time to do work yet...
		return;
	}

	// Let's do some work...

	nPassCount = 0;
	while( m_fSecsUntilNextWork <= 0.0f ) {
		m_fSecsUntilNextWork += _PHYSICS_WORK_DELTA_SECS;
		++nPassCount;
	}

	for( nPass=0; nPass<nPassCount; ++nPass ) {
		// Add our active net masters to the work array...
		_FillNetArray( &m_ActiveNetsRoot );

		// Clear forces on last pass...
		bFirstPass = (nPass == 0);
		bLastPass = (nPass == (nPassCount-1));

		for( nNetWorkIndex=0; nNetWorkIndex < m_nNetWorkCount; ++nNetWorkIndex ) {
			pNetMasterVerlet = m_ppNetWorkArray[nNetWorkIndex];

			if( pNetMasterVerlet->m_pNetMaster != pNetMasterVerlet ) {
				// We're no longer a master...
				continue;
			}

			// Update only the nets that don't have any manual work member objects
			// and work has not already been performed on this master this frame...
			if( ( pNetMasterVerlet->m_nNetManualWorkCount == 0 ) && ( pNetMasterVerlet->m_nNetLastWorkedFrame != FVid_nFrameCounter ) ) {
				pNetMasterVerlet->_NetWork( _PHYSICS_WORK_DELTA_SECS, (1.0f / _PHYSICS_WORK_DELTA_SECS), FALSE, bFirstPass, bLastPass );
				if( bLastPass ) {
					pNetMasterVerlet->m_nNetLastWorkedFrame = FVid_nFrameCounter;
				}
			}
		}
	}

	// Add our inactive net masters to the work array...
	_FillNetArray( &m_InactiveNetsRoot );

	for( nNetWorkIndex=0; nNetWorkIndex < m_nNetWorkCount; ++nNetWorkIndex ) {
		pNetMasterVerlet = m_ppNetWorkArray[nNetWorkIndex];

		if( pNetMasterVerlet->m_nFlags & FLAG_NET_MASTER_FREEZE ) {
			// Entire net is frozen...
			continue;
		}

		if( ( pNetMasterVerlet->m_nNetManualWorkCount == 0 ) && ( pNetMasterVerlet->m_nNetLastWorkedFrame != FVid_nFrameCounter ) ) {
			// This is NOT a manual work verlet Network.  Deal with it now....
			pNetMasterVerlet->_InactiveNetWork();				
			pNetMasterVerlet->m_nNetLastWorkedFrame = FVid_nFrameCounter;
		}
	}
}



void CFVerlet::ManualWork( void ) {
	FASSERT( IsModuleInitialized() );
	FASSERT( m_nFlags & FLAG_MANUAL_WORK );

	if( FLoop_bGamePaused ) {
		return;
	}

	CFVerlet *pNetMasterVerlet;
	BOOL bFirstPass, bLastPass;
	f32 fDeltaSecs;
	
	// Make sure we use the master object to call the work function for this guy...
	pNetMasterVerlet = m_pNetMaster;

	if( pNetMasterVerlet->m_nNetLastWorkedFrame == FVid_nFrameCounter ) {
		// We have already worked on this network, so just return...
		return;
	}

	if( pNetMasterVerlet->m_nNetListMember == NET_LIST_MEMBER_ACTIVE ) {
		//RAFHACK -- We should check the time to make sure we don't call this net
		//           multiple times...
		// Clear forces on last pass...
		bFirstPass = TRUE;
		bLastPass = TRUE;
		fDeltaSecs = FLoop_fPreviousLoopSecs;
		if( fDeltaSecs > 0.0f ) {
			pNetMasterVerlet->_NetWork( fDeltaSecs, (1.0f / fDeltaSecs), FALSE, bFirstPass, bLastPass );
		}
	} else if ( pNetMasterVerlet->m_nNetListMember == NET_LIST_MEMBER_INACTIVE ) {
		//RAFHACK -- We should check the time to make sure we don't call this net
		//           multiple times...
		if( pNetMasterVerlet->m_nFlags & FLAG_NET_MASTER_FREEZE ) {
			// Entire net is frozen...
			pNetMasterVerlet->m_nNetLastWorkedFrame = FVid_nFrameCounter;
			return;
		}
		pNetMasterVerlet->_InactiveNetWork();				
	}
	pNetMasterVerlet->m_nNetLastWorkedFrame = FVid_nFrameCounter;
}



BOOL CFVerlet::CheckPointSave( void ) {
	FASSERT( IsModuleInitialized() );

	//NOTE: For safety, I am saving everything that could be externally modified
	//through the Verlet API.  A lot of these api's may only be called once though.
	//We may be able to save some memory by saving less 
	//and making assumptions/rules that certain values will never change after
	//creation.  At this point, I will NOT make that assumption, and see what
	//our memory usage ends up to be.

	CFCheckPoint::SaveData( (u32&) m_pNetMaster );
	CFCheckPoint::SaveData( m_nFlags ); // this is a u16, but it's going to eat up a u32 worth of memory

	if( m_pNetMaster == this ) {
		//only save off these values if we are a Net Master
		//Pack them into 32 bit values to save memory
		//(Internally, the checkpoint system takes u8's and
		// u16s and saves them as u32's )
		u32 uPackedVal;
		uPackedVal  = ( m_nNetInScopeCount    << 24 );
		uPackedVal |= ( m_nNetMovingCount     << 16 );
		uPackedVal |= ( m_nNetManualWorkCount << 8  );
		uPackedVal |= ( m_nNetListMember            );
		CFCheckPoint::SaveData( uPackedVal );
	}

	CFCheckPoint::SaveData( (u32&) m_pWorldMesh );
	CFCheckPoint::SaveData( m_fGravity );
	CFCheckPoint::SaveData( m_fMass );
	CFCheckPoint::SaveData( m_fPrimeSecs );
	CFCheckPoint::SaveData( m_fUnitDampen );
	CFCheckPoint::SaveData( m_fMotionlessSecsRemaining );
	CFCheckPoint::SaveData( m_TetraDim );
	CFCheckPoint::SaveData( m_CenterOfMass_MS );
	CFCheckPoint::SaveData( m_UnitMtx );
	CFCheckPoint::SaveData( m_nUnanchoredCollType ); //This is a u8, but it's going to eat up a u32 worth of memory

	//next, save off all the tacks associated with this Object
	for( u32 i=0; i<m_nTackCount; i++ ) {
		m_pTackArray[ i ]._CheckPointSave();
	}

	return TRUE;
}


BOOL CFVerlet::CheckPointLoad( void ) {
	FASSERT( IsModuleInitialized() );

	// First, detatch ourself from whatever slave array we are currently connected to.
	// Then get our new master and attach ourself to that masters root.
	flinklist_Remove( &m_pNetMaster->m_NetSlaveRoot, this );

	// Next, check if we are a master, and if so, detach ourself from whatever Master
	// list we currently belong to.
	if( m_pNetMaster == this ) {
		FASSERT( m_nNetListMember != NET_LIST_MEMBER_COUNT );
		//we are a master...
		if( m_nNetListMember == NET_LIST_MEMBER_ACTIVE ) {
			flinklist_Remove( &m_ActiveNetsRoot, this );
		} else {
			flinklist_Remove( &m_InactiveNetsRoot, this );
		}
		m_nNetListMember = NET_LIST_MEMBER_COUNT;
	}

	CFCheckPoint::LoadData( (u32&) m_pNetMaster );
	flinklist_AddTail( &m_pNetMaster->m_NetSlaveRoot, this );

	CFCheckPoint::LoadData( m_nFlags );

	if( m_pNetMaster == this ) { //we are a master!
		u32 uPackedVal;
		CFCheckPoint::LoadData( uPackedVal );
		m_nNetInScopeCount =    ( ( uPackedVal >> 24 ) & 0xFF );
		m_nNetMovingCount =     ( ( uPackedVal >> 16 ) & 0xFF );
		m_nNetManualWorkCount = ( ( uPackedVal >>  8 ) & 0xFF );
		m_nNetListMember =      ( ( uPackedVal       ) & 0xFF );
		FASSERT( m_nNetListMember != NET_LIST_MEMBER_COUNT );
		if( m_nNetListMember == NET_LIST_MEMBER_ACTIVE ) {
			flinklist_AddTail( &m_ActiveNetsRoot, this );
		} else {
			flinklist_AddTail( &m_InactiveNetsRoot, this );
		}
	}
	CFCheckPoint::LoadData( (u32&) m_pWorldMesh );
	CFCheckPoint::LoadData( m_fGravity );
	CFCheckPoint::LoadData( m_fMass );
	CFCheckPoint::LoadData( m_fPrimeSecs );
	CFCheckPoint::LoadData( m_fUnitDampen );
	CFCheckPoint::LoadData( m_fMotionlessSecsRemaining );
	CFCheckPoint::LoadData( m_TetraDim );
	CFCheckPoint::LoadData( m_CenterOfMass_MS );
	CFCheckPoint::LoadData( m_UnitMtx );
	CFCheckPoint::LoadData( m_nUnanchoredCollType ); // This is a u8, but it's going to eat up a u32 worth of memory

	// Now call the necessary functions to recompute internal variables...
	SetMass( m_fMass );
	// Don't re-kickstart the net because it could possibly activate inactive objects
	// that are inscope that should NOT be activated.
	InitUnitMtx( &m_UnitMtx, FALSE); 
	SetObjectDim( m_TetraDim );

	// Now, reload the tack data associated with this verlet object...
	for( u32 i=0; i<m_nTackCount; i++ ) {
		m_pTackArray[ i ]._CheckPointLoad();
	}

	return TRUE;
}


void CFVerlet::EnableExplodeFunctionality( BOOL bCanExplode ) {
	FASSERT( IsCreated() );

	if( bCanExplode ) {
		FMATH_SETBITMASK( m_nFlags, FLAG_CAN_EXPLODE );
	} else {
		FMATH_CLEARBITMASK( m_nFlags, FLAG_CAN_EXPLODE | FLAG_EXPLODE_WHEN_TACK_HITS_SURFACE );
	}
}


void CFVerlet::_UpdateExplosionDetectionFlag( void ) {
	if( !(m_nFlags & FLAG_CAN_EXPLODE) ) {
		// This object can't explode...
		return;
	}

	u32 i;
	CFVerletTack *pTack;

	for( i=0; i<m_nTackCount; ++i ) {
		pTack = m_pTackArray + i;

		if( !pTack->Anchor_IsEnabled() ) {
			// Anchor not enabled...
			continue;
		}

		if( pTack->Anchor_GetType() == CFVerletTack::ANCHOR_TYPE_NONE ) {
			// No anchor...
			continue;
		}

		if( pTack->Anchor_GetType() == CFVerletTack::ANCHOR_TYPE_TACK ) {
			// Tacks anchored to other tacks don't count...
			continue;
		}

		// This tack is still remaining...

		FMATH_CLEARBITMASK( m_nFlags, FLAG_EXPLODE_WHEN_TACK_HITS_SURFACE );

		return;
	}

	// All tack candidates have been destroyed...

	FMATH_SETBITMASK( m_nFlags, FLAG_EXPLODE_WHEN_TACK_HITS_SURFACE );
}


void CFVerlet::_FillNetArray( FLinkRoot_t *pLinkRoot ) {
	CFVerlet *pNetMasterVerlet;

	m_nNetWorkCount = 0;

	for( pNetMasterVerlet=(CFVerlet *)flinklist_GetHead( pLinkRoot ); pNetMasterVerlet; pNetMasterVerlet=(CFVerlet *)flinklist_GetNext( pLinkRoot, pNetMasterVerlet ) ) {
		if( m_nNetWorkCount >= m_nWorkArrayCount ) {
			DEVPRINTF( "CFVerlet::_FillNetArray(): Too many active CFVerlet master physics objects. Try increasing Fang_ConfigDefs.nVerlet_MaxCount.\n" );
			break;
		}

		m_ppNetWorkArray[m_nNetWorkCount++] = pNetMasterVerlet;
	}
}


void CFVerlet::_FillSlaveArray( void ) {
	CFVerlet *pSlaveVerlet;

	m_nVerletWorkCount = 0;

	for( pSlaveVerlet=(CFVerlet *)flinklist_GetHead( &m_NetSlaveRoot ); pSlaveVerlet; pSlaveVerlet=(CFVerlet *)flinklist_GetNext( &m_NetSlaveRoot, pSlaveVerlet ) ) {
		if( !(pSlaveVerlet->m_nFlags & FLAG_ENABLE_WORK) ) {
			// Work not enabled...
			continue;
		}

		if( m_nVerletWorkCount >= m_nWorkArrayCount ) {
			DEVPRINTF( "CFVerlet::_FillSlaveArray(): Too many active CFVerlet slave physics objects. Try increasing Fang_ConfigDefs.nVerlet_MaxCount.\n" );
			break;
		}

		m_ppVerletWorkArray[m_nVerletWorkCount++] = pSlaveVerlet;
	}
}


void CFVerlet::_NetWork( f32 fDeltaSecs, f32 fInvDeltaSecs, BOOL bPrimeMode, BOOL bFirstPass, BOOL bLastPass ) {
	CFVerlet *pSlaveVerlet;
	u32 i, nSlaveWorkIndex, nConstraintPassesCount;

	// Add our slaves to the work array...
	m_nVerletWorkCount = 0;
	for( pSlaveVerlet=(CFVerlet *)flinklist_GetHead( &m_NetSlaveRoot ); pSlaveVerlet; pSlaveVerlet=(CFVerlet *)flinklist_GetNext( &m_NetSlaveRoot, pSlaveVerlet ) ) {
		// Update boned tacks...
		if( pSlaveVerlet->m_pBoneArray ) {
			if( bFirstPass ) {
				pSlaveVerlet->_UpdateBonedTacks();
			}
		}

		if( !(pSlaveVerlet->m_nFlags & FLAG_ENABLE_WORK) ) {
			// Work not enabled...
			continue;
		}

		if( m_nVerletWorkCount >= m_nWorkArrayCount ) {
			DEVPRINTF( "CFVerlet::_NetWork(): Too many active CFVerlet slave physics objects. Try increasing Fang_ConfigDefs.nVerlet_MaxCount.\n" );
			break;
		}

		if( !bPrimeMode ) {
			if( pSlaveVerlet->m_pWorldMesh ) {
				if( !(pSlaveVerlet->m_pWorldMesh->GetVolumeFlags() & FVIS_VOLUME_IN_ACTIVE_LIST) ) {
					// This slave is no longer in the fvis active list...

					pSlaveVerlet->SetInScope( FALSE );

					if( m_nNetListMember != NET_LIST_MEMBER_ACTIVE ) {
						// By making the slave out-of-scope, the entire net has been
						// placed out-of-scope, and so the net is no longer in the active list.
						// Skip all slaves in this net...

						m_nVerletWorkCount = 0;

						break;
					}
				}
			}
		}

		m_ppVerletWorkArray[m_nVerletWorkCount++] = pSlaveVerlet;
	}

	if( m_nVerletWorkCount == 0 ) {
		return;
	}

	// Handle pre-collision...
	if( m_nVerletWorkCount == 1 ) {
		// This is an unanchored object...

		pSlaveVerlet = m_ppVerletWorkArray[0];

		if( pSlaveVerlet->m_nUnanchoredCollType >= UNANCHORED_COLL_TYPE_SPHERE ) {
			pSlaveVerlet->_ResolveCollisionState( fDeltaSecs, fInvDeltaSecs );
		}
	}

	// Integrate all slaves in the net...
	for( nSlaveWorkIndex=0; nSlaveWorkIndex < m_nVerletWorkCount; ++nSlaveWorkIndex ) {
		pSlaveVerlet = m_ppVerletWorkArray[nSlaveWorkIndex];

		pSlaveVerlet->ComputeWSOriginPointFromMS( &pSlaveVerlet->m_PreWorkPos_WS );

		pSlaveVerlet->Integrate( fDeltaSecs, bLastPass );
	}

	// Satisfy constraints of all slaves in the net...
	nConstraintPassesCount = 3 + m_nVerletWorkCount;
	for( i=0; i<nConstraintPassesCount; ++i ) {
		for( nSlaveWorkIndex=0; nSlaveWorkIndex < m_nVerletWorkCount; ++nSlaveWorkIndex ) {
			m_ppVerletWorkArray[nSlaveWorkIndex]->SatisfyConstraints();
		}
	}

	// Handle collisions...
	if( m_nVerletWorkCount == 1 ) {
		// This is an unanchored object...

		pSlaveVerlet = m_ppVerletWorkArray[0];

		if( pSlaveVerlet->m_nUnanchoredCollType >= UNANCHORED_COLL_TYPE_SPHERE ) {
			pSlaveVerlet->_HandleCollision( fDeltaSecs, fInvDeltaSecs );
		}
	}

	// Compute matrices and let user know that this Verlet object has moved...
	for( nSlaveWorkIndex=0; nSlaveWorkIndex < m_nVerletWorkCount; ++nSlaveWorkIndex ) {
		pSlaveVerlet = m_ppVerletWorkArray[nSlaveWorkIndex];

		// Update matrix...
		pSlaveVerlet->ComputeMtx();

		if( !bPrimeMode ) {
			if( pSlaveVerlet->m_pFcnMovedCallback ) {
				pSlaveVerlet->m_pFcnMovedCallback( pSlaveVerlet, fDeltaSecs );
			}
		}
	}

	// Handle motionless detection...
	for( nSlaveWorkIndex=0; nSlaveWorkIndex < m_nVerletWorkCount; ++nSlaveWorkIndex ) {
		m_ppVerletWorkArray[nSlaveWorkIndex]->_HandleMotionlessDetection( fDeltaSecs, fInvDeltaSecs );
	}

	// Handle exploded slaves...
	for( nSlaveWorkIndex=0; nSlaveWorkIndex < m_nVerletWorkCount; ++nSlaveWorkIndex ) {
		pSlaveVerlet = m_ppVerletWorkArray[nSlaveWorkIndex];

		if( pSlaveVerlet->m_nFlags & FLAG_CALL_EXPLODE_CALLBACK ) {
			if( pSlaveVerlet->m_pFcnExplodeCallback ) {
				pSlaveVerlet->m_pFcnExplodeCallback( pSlaveVerlet );
			}

			FMATH_CLEARBITMASK( pSlaveVerlet->m_nFlags, FLAG_CALL_EXPLODE_CALLBACK );
		}
	}

	if( bLastPass ) {
		CFVerletTack *pTack;
		u32 nTackIndex;

		// Update wires...
		for( nSlaveWorkIndex=0; nSlaveWorkIndex < m_nVerletWorkCount; ++nSlaveWorkIndex ) {
			pSlaveVerlet = m_ppVerletWorkArray[nSlaveWorkIndex];

			for( nTackIndex=0; nTackIndex<pSlaveVerlet->m_nTackCount; ++nTackIndex ) {
				pTack = pSlaveVerlet->m_pTackArray + nTackIndex;

				pTack->_UpdateTackWire();
			}
		}
	}
}


void CFVerlet::_InactiveNetWork( void ) {
	CFVerlet *pSlaveVerlet;
	CFVerletTack *pTack;
	CFVec3A TackPos_WS;
	u32 i;

	for( pSlaveVerlet=(CFVerlet *)flinklist_GetHead( &m_NetSlaveRoot ); pSlaveVerlet; pSlaveVerlet=(CFVerlet *)flinklist_GetNext( &m_NetSlaveRoot, pSlaveVerlet ) ) {
		if( pSlaveVerlet->m_pWorldMesh ) {
			if( !pSlaveVerlet->IsInScope() ) {
				// Slave is currently not in scope. See if it is in scope now...

				if( pSlaveVerlet->m_pWorldMesh->GetVolumeFlags() & FVIS_VOLUME_IN_ACTIVE_LIST ) {
					// This slave is now in the fvis active list...

					pSlaveVerlet->SetInScope( TRUE );

					if( m_nNetListMember != NET_LIST_MEMBER_INACTIVE ) {
						// By making the slave in-scope, the entire net has been placed in-scope,
						// and so the net is now in the active list...

						break;
					}
				}
			} else {
				// Slave is currently in scope. See if it's no longer...

				if( !(pSlaveVerlet->m_pWorldMesh->GetVolumeFlags() & FVIS_VOLUME_IN_ACTIVE_LIST) ) {
					// This slave is no longer in the fvis active list...

					pSlaveVerlet->SetInScope( FALSE );
				}
			}
		}

		if( pSlaveVerlet->m_nFlags & FLAG_IN_SCOPE ) {
			// Slave is in scope, but is not awake...

			if( pSlaveVerlet->m_nFlags & FLAG_HAS_DYNAMIC_ANCHOR ) {
				// Slave has a dynamic anchor...

				for( i=0; i<pSlaveVerlet->m_nTackCount; ++i ) {
					pTack = &pSlaveVerlet->m_pTackArray[i];

					if( pTack->Anchor_GetType() == CFVerletTack::ANCHOR_TYPE_POINT_DYNAMIC ) {
						// Found the dynamic anchor...

						if( *pTack->m_pAnchorPos_WS != pTack->m_AnchorPos_WS ) {
							// Dynamic anchor has moved. Awaken this slave for next frame...

							pSlaveVerlet->Net_Kickstart();
						}
					}
				}
			}

			if( pSlaveVerlet->m_nFlags & FLAG_HAS_BONED_ANCHOR ) {
				// Slave has a boned anchor...

				for( i=0; i<pSlaveVerlet->m_nTackCount; ++i ) {
					pTack = &pSlaveVerlet->m_pTackArray[i];

					if( pTack->Anchor_GetType() == CFVerletTack::ANCHOR_TYPE_POINT_BONED ) {
						// Found the boned anchor...

						pTack->m_pBonedAnchorMtx->MulPoint( TackPos_WS, pTack->m_BonedAnchorPos_BS );

						if( TackPos_WS != pTack->m_AnchorPos_WS ) {
							// Boned anchor has moved. Awaken this slave for next frame...

							pTack->m_AnchorPos_WS = TackPos_WS;
							pTack->m_nBonedAnchorComputedFrame = FVid_nFrameCounter;

							pSlaveVerlet->Net_Kickstart();
						}
					}
				}
			}
		}
	}
}

void CFVerlet::_UpdateBonedTacks( void ) {
	CFVec3A CenterOfMass_WS, TempVec, TackPos_BS;
	CFMtx43A BoneToBoneMtx;
	const Bone_t *pVerletBone;
	FMeshBone_t *pBone;
	CFVerletTack *pTack;
	u32 i, j;

	if( m_nBoneCount > 1 ) {
		// Update center-of-mass...
		CenterOfMass_WS.Zero();

		for( i=0, pVerletBone=m_pBoneArray; i<m_nBoneCount; ++i, ++pVerletBone ) {
			pBone = &m_pWorldMesh->m_pMesh->pBoneArray[ pVerletBone->nBoneIndex ];

			TempVec.Set( pBone->SegmentedBoundSphere_BS.m_Pos );
			m_pWorldMesh->GetBoneMtxPalette()[ pVerletBone->nBoneIndex ]->MulPoint( TempVec );
			CenterOfMass_WS.Add( TempVec );
		}

		TempVec.Set( fmath_Inv( (f32)m_nBoneCount ) );
		CenterOfMass_WS.Mul( TempVec );
		SetCenterOfMass_WS( &CenterOfMass_WS, FALSE );
	}

	// Update boned tacks...
	for( i=0, pVerletBone=m_pBoneArray; i<m_nBoneCount; ++i, ++pVerletBone ) {
		pBone = &m_pWorldMesh->m_pMesh->pBoneArray[ pVerletBone->nBoneIndex ];

		if( pVerletBone->nTackCount ) {
			_ComputeInvMtx();
			BoneToBoneMtx.Mul( m_InvUnitMtx, *m_pWorldMesh->GetBoneMtxPalette()[ pVerletBone->nBoneIndex ] );

			for( j=0; j<pVerletBone->nTackCount; ++j ) {
				pTack = &m_pTackArray[ pVerletBone->nTackStartIndex + j ];
				pTack->_UpdateBonedTack( &BoneToBoneMtx );
			}
		}
	}
}


void CFVerlet::PrimeAll( void ) {
	FASSERT( IsModuleInitialized() );

	_FillNetArray( &m_ActiveNetsRoot );
	_NetListPrime();

	_FillNetArray( &m_InactiveNetsRoot );
	_NetListPrime();
}


void CFVerlet::_NetListPrime( void ) {
	FASSERT( IsModuleInitialized() );

	CFVerlet *pSlaveVerlet;
	u32 i, nPrimeLoops, nNetWorkIndex, nSlaveWorkIndex;
	f32 fPrimeSecs;

	for( nNetWorkIndex=0; nNetWorkIndex < m_nNetWorkCount; ++nNetWorkIndex ) {
		m_ppNetWorkArray[nNetWorkIndex]->_FillSlaveArray();

		fPrimeSecs = 0.0f;
		for( nSlaveWorkIndex=0; nSlaveWorkIndex < m_nVerletWorkCount; ++nSlaveWorkIndex ) {
			if( m_ppVerletWorkArray[nSlaveWorkIndex]->m_fPrimeSecs > fPrimeSecs ) {
				fPrimeSecs = m_ppVerletWorkArray[nSlaveWorkIndex]->m_fPrimeSecs;
			}
		}

		if( fPrimeSecs == 0.0f ) {
			continue;
		}

		nPrimeLoops = (u32)( fPrimeSecs * (1.0f / _PRIME_DELTA_SECS) ) + 1;

		for( i=0; i<nPrimeLoops; ++i ) {
			m_ppNetWorkArray[nNetWorkIndex]->_NetWork( _PRIME_DELTA_SECS, (1.0f / _PRIME_DELTA_SECS), TRUE, TRUE, FALSE );
		}

		for( nSlaveWorkIndex=0; nSlaveWorkIndex < m_nVerletWorkCount; ++nSlaveWorkIndex ) {
			pSlaveVerlet = m_ppVerletWorkArray[nSlaveWorkIndex];

			pSlaveVerlet->ComputeMtx();

			if( pSlaveVerlet->m_pFcnMovedCallback ) {
				pSlaveVerlet->m_pFcnMovedCallback( pSlaveVerlet, _PRIME_DELTA_SECS );
			}
		}
	}
}


BOOL CFVerlet::_ResolveCollisionState( f32 fDeltaSecs, f32 fInvDeltaSecs ) {
	#define __ITERATIONS	4
	CFCollData CollData;
	CFVec3A PushVec_WS, ImpactPointOnObject_WS, MoveVec_WS, aDeltaPos_WS[4];
	u32 i, j;

	if( m_nFlags & FLAG_COLLIDE_WITH_DYN_OBJ ) {
		CollData.nFlags = 0; 
	} else {
		CollData.nFlags = FCOLL_DATA_IGNORE_DYNAMIC_OBJECTS;
	}

	switch( m_nUnanchoredCollType ) {
	case UNANCHORED_COLL_TYPE_SPHERE:
		CollData.nFlags |= FCOLL_DATA_USE_OBJECT_SPHERE;
		break;

	case UNANCHORED_COLL_TYPE_CAPSULE:
		CollData.nFlags |= FCOLL_DATA_USE_OBJECT_CAPSULE;
		break;

	case UNANCHORED_COLL_TYPE_KDOP:
		break;

	default:
		FASSERT_NOW;
	};

	MoveVec_WS.Zero();

	// We're going to attempt to resolve the move by performing up to three collision detections
	for( j = 0; j < __ITERATIONS; j++ ) {
		// Setup the collision system data
		CollData.nCollMask = FCOLL_MASK_COLLIDE_WITH_OBJECTS;
		CollData.nTrackerUserTypeBitsMask = FCOLL_USER_TYPE_BITS_ALL;
		CollData.pCallback = _TrackerCollisionCB;
		CollData.pLocationHint = NULL;
		CollData.pMovement = &MoveVec_WS;
		CollData.nTrackerSkipCount = 0;

		// Clear the collision buffer and check the collision
		fcoll_Clear();
		FASSERT( m_pWorldMesh != NULL );
		m_pCBVerlet = this;
		fcoll_Check( &CollData, m_pWorldMesh );
		m_pCBVerlet = NULL;

		// If there is no collision, we can just apply the move and exit the loop
		if( FColl_nImpactCount == 0 ) {
			break;
		}

		// Sort the impacts:
		FCollImpact_t *pImpact = &FColl_aImpactBuf[0];
		for( i = 1; i<FColl_nImpactCount; ++i ) {
			FASSERT( FColl_aImpactBuf[i].fUnitImpactTime == -1.f );
			if ( pImpact->fImpactDistInfo < FColl_aImpactBuf[i].fImpactDistInfo ) {
				pImpact = &FColl_aImpactBuf[i];
			}
		}

		PushVec_WS.Mul( pImpact->PushUnitVec, pImpact->fImpactDistInfo + 0.001f );

		if ( j == __ITERATIONS-1 ) {
			for( i=0; i<4; ++i ) {
				m_aTetraPos_WS[i].Add( PushVec_WS );
				m_aTetraOldPos_WS[i].Add( PushVec_WS );
			}
		} else	{
			for( i=0; i<4; ++i ) {
				aDeltaPos_WS[i] = m_aTetraPos_WS[i];
			}

			ImpactPointOnObject_WS.Sub( pImpact->ImpactPoint, PushVec_WS );
			ApplyDisplacementImpulse_Pos( &ImpactPointOnObject_WS, &pImpact->ImpactPoint );

			for( i=0; i<4; ++i ) {
				SatisfyConstraints();
			}

			for( i=0; i<4; ++i ) {
				aDeltaPos_WS[i].RevSub( m_aTetraPos_WS[i] );
				m_aTetraOldPos_WS[i].Add( aDeltaPos_WS[i] );
			}
		}

		ComputeMtx();

		if( m_pFcnMovedCallback ) {
			m_pFcnMovedCallback( this, fDeltaSecs );
		}
	}

	return FALSE;

	#undef __ITERATIONS
}


BOOL CFVerlet::_HandleCollision( f32 fDeltaSecs, f32 fInvDeltaSecs ) {
	CFCollData CollData;
	CFVec3A vAdjust, CurrentOrigin_WS, ObjectMoveStepVec_WS, PushVec_WS, SurfaceVel_WS, ImpactPointOnObject_WS, PlaneVelocity_WS;
	CFVec3A DestPointOnSurface_WS, ImpactPoint_MS;
	CFVec4 Weights;
	u32 i, j;
	f32 fDot;

	if( m_nFlags & FLAG_COLLIDE_WITH_DYN_OBJ ) {
		CollData.nFlags = 0; 
	} else {
		CollData.nFlags = FCOLL_DATA_IGNORE_DYNAMIC_OBJECTS;
	}

	switch( m_nUnanchoredCollType ) {
	case UNANCHORED_COLL_TYPE_SPHERE:
		CollData.nFlags |= FCOLL_DATA_USE_OBJECT_SPHERE;
		break;

	case UNANCHORED_COLL_TYPE_CAPSULE:
		CollData.nFlags |= FCOLL_DATA_USE_OBJECT_CAPSULE;
		break;

	case UNANCHORED_COLL_TYPE_KDOP:
		break;

	default:
		FASSERT_NOW;
	};

	ComputeWSOriginPointFromMS( &CurrentOrigin_WS );
	ObjectMoveStepVec_WS.Sub( CurrentOrigin_WS, m_PreWorkPos_WS );

	if( ObjectMoveStepVec_WS.MagSq() < 0.0000001f ) {
		return FALSE;
	}

	// We're going to attempt to resolve the move by performing up to 4 collision detections
	for( j = 0; j < 4; j++ ) {
		// Setup the collision system data
		CollData.nCollMask = FCOLL_MASK_COLLIDE_WITH_OBJECTS;
		CollData.nTrackerUserTypeBitsMask = FCOLL_USER_TYPE_BITS_ALL;
		CollData.pCallback = _TrackerCollisionCB;
		CollData.pLocationHint = NULL;
		CollData.pMovement = &ObjectMoveStepVec_WS;
		CollData.nTrackerSkipCount = 0;

		// Clear the collision buffer and check the collision
		fcoll_Clear();

		m_pCBVerlet = this;
		fcoll_Check( &CollData, m_pWorldMesh );
		m_pCBVerlet = NULL;

		// If there is no collision, we can just apply the move and exit the loop
		if( FColl_nImpactCount == 0 ) {
			break;
		}

		// Sort the impacts:
		//   First impacts are those that have the fUnitImpactTime -1.f, sorted by greatest fImpactDistInfo.  If there
		//   are no impacts that have the fUnitImpactTime -1.f, then we sort from soonest impact time to latest.
		FCollImpact_t *pImpact = &FColl_aImpactBuf[0];
		for( i = 1; i<FColl_nImpactCount; ++i ) {
			if ( pImpact->fUnitImpactTime == -1.f ) {
				if ( FColl_aImpactBuf[i].fUnitImpactTime == -1.f && pImpact->fImpactDistInfo < FColl_aImpactBuf[i].fImpactDistInfo ) {
					pImpact = &FColl_aImpactBuf[i];
				}
			} else if ( FColl_aImpactBuf[i].fUnitImpactTime == -1.f || pImpact->fUnitImpactTime > FColl_aImpactBuf[i].fUnitImpactTime )	{
				pImpact = &FColl_aImpactBuf[i];
			}
		}

		// Push object out of collision...

		if( pImpact->fUnitImpactTime == -1.0f ) {
			// Oh, poop.  The object is already in collision, so we need to back out the move and hope it resolves the collision
			// at the beginning of the next frame.

			// The tetrahedron had been moved, previously, so we need to move it back and get it out of collision
			for( j=0; j<4; ++j ) {
				m_aTetraPos_WS[j].Set( m_aTetraOldPos_WS[j] );
			}

			break;
		} else {
			PushVec_WS.Mul( pImpact->PushUnitVec, pImpact->fImpactDistInfo + 0.01f );

			ImpactPointOnObject_WS.Mul( ObjectMoveStepVec_WS, 1.0f - pImpact->fUnitImpactTime ).Add( pImpact->ImpactPoint );

			ComputeSurfaceVelocity_WS( &SurfaceVel_WS, &ImpactPointOnObject_WS, fInvDeltaSecs );
			fDot = SurfaceVel_WS.Dot( pImpact->UnitFaceNormal );
			PlaneVelocity_WS.Mul( pImpact->UnitFaceNormal, fDot ).Sub( SurfaceVel_WS );
			PlaneVelocity_WS.Mul( 1.0f );
			ApplyForce( &ImpactPointOnObject_WS, &PlaneVelocity_WS );

			DestPointOnSurface_WS.Add( PushVec_WS, ImpactPointOnObject_WS );

			ApplyDisplacementImpulse_Pos( &ImpactPointOnObject_WS, &DestPointOnSurface_WS );

			for( i=0; i<4; ++i ) {
				SatisfyConstraints();
			}

			ComputeWSOriginPointFromMS( &CurrentOrigin_WS );
			ObjectMoveStepVec_WS.Sub( CurrentOrigin_WS, m_PreWorkPos_WS );
		}
	}

	return FALSE;
}


u32 CFVerlet::_TrackerCollisionCB( CFWorldTracker *pTracker ) {
	FASSERT( m_pCBVerlet != NULL );

	if( m_pCBVerlet->m_pFcnTrackerCollCallback != NULL ) {
		if ( m_pCBVerlet->m_pFcnTrackerCollCallback( m_pCBVerlet, pTracker ) )
		{
			return FCOLL_CHECK_CB_ALL_IMPACTS;
		}

		return FCOLL_CHECK_CB_DO_NOT_CHECK_TRACKER;
	}
	
	return FCOLL_CHECK_CB_ALL_IMPACTS;
}


CFVerlet::CFVerlet() {
	m_nTackCount = 0;
	m_nTackSurfaceCount = 0;
	m_nFlags = FLAG_NONE;
	m_pTackArray = NULL;
}


CFVerlet::~CFVerlet() {
	Destroy();
}


void CFVerlet::SetBoneNameTackTable( cchar **apszTackNameArray, cchar **apszCTackNameArray ) {
	FASSERT( m_bSystemInitialized );

	m_apszTackNameArray = apszTackNameArray;
	m_apszCTackNameArray = apszCTackNameArray;
}


// Creates a verlet object with the specified number of tacks.
BOOL CFVerlet::Create( f32 fMass, const CFVec3A *pObjectDim, u32 nTackCount, const CFMtx43A *pUnitMtx, const CFVec3A *pCenterOfMass_MS ) {
	FASSERT( m_bSystemInitialized );
	FASSERT( !IsCreated() );

	CFVec3A ScaledTetraVtx0_MS, ScaledTetraVtx1_MS, ScaledTetraVtx3_MS;
	u32 i;

	// Get a frame...
	FResFrame_t ResFrame = fres_GetFrame();

	// Set the CREATED flag...
	m_nFlags = FLAG_CREATED | FLAG_ENABLE_WORK;

	m_nNetListMember = NET_LIST_MEMBER_INACTIVE;
	flinklist_AddTail( &m_InactiveNetsRoot, this );

	SetMass( fMass );

	// Set the length of the tetrahedron side...
	m_TetraDim = *pObjectDim;

	FMATH_CLAMPMIN( m_TetraDim.x, 0.5f );
	FMATH_CLAMPMIN( m_TetraDim.y, 0.5f );
	FMATH_CLAMPMIN( m_TetraDim.z, 0.5f );

	// Compute tetrahedron edge lengths...
	ScaledTetraVtx0_MS.Set( (-1.0f/3.0f)*m_TetraDim.x, -0.25f*m_TetraDim.y, -0.5f*m_TetraDim.z );
	ScaledTetraVtx1_MS.Set( (2.0f/3.0f)*m_TetraDim.x, -0.25f*m_TetraDim.y, 0.0f );
	ScaledTetraVtx3_MS.Set( 0.0f,  0.75f*m_TetraDim.y, 0.0f );

	m_fDim_v10_v12 = fmath_Sqrt( m_TetraDim.x*m_TetraDim.x + 0.25f*m_TetraDim.z*m_TetraDim.z );
	m_fDim_v30_v32 = ScaledTetraVtx0_MS.Dist( ScaledTetraVtx3_MS );
	m_fDim_v31 = ScaledTetraVtx1_MS.Dist( ScaledTetraVtx3_MS );

	// Compute magic constants (thank you MathCAD)...
	m_K1 = 1.0f / (-12.0f * m_TetraDim.x * m_TetraDim.y * m_TetraDim.z);
	m_K2 = 1.0f / (12.0f * m_TetraDim.x * m_TetraDim.y);
	m_K3 = 1.0f / m_TetraDim.y;
	m_K4 = 12.0f * m_TetraDim.x * m_TetraDim.y;
	m_K5 = 6.0f * m_TetraDim.y * m_TetraDim.z;
	m_K6 = 4.0f * m_TetraDim.x * m_TetraDim.z;
	m_K7 = -3.0f * m_TetraDim.x * m_TetraDim.y * m_TetraDim.z;
	m_K8 = -4.0f * m_TetraDim.x;
	m_K9 = 12.0f * m_TetraDim.y;
	m_K10 = 3.0f * m_TetraDim.x * m_TetraDim.y;

	// Set our matrix...
	m_UnitMtx = *pUnitMtx;
	_ComputeInvMtx();

	// Init current and old tetrahedron vertices from the initial matrix...
	_ComputeTetraVerticesFromMtx();

	m_nUnanchoredCollType = UNANCHORED_COLL_TYPE_NONE;

	m_nCollTackCount = 0;

	// Set up our tacks...
	m_nTackCount = nTackCount;
	if( nTackCount ) {
		// Allocate tack array...

		m_pTackArray = fnew CFVerletTack[nTackCount];

		if( m_pTackArray == NULL ) {
			DEVPRINTF( "CFVerlet::Create(): Not enough memory to allocate tack array.\n" );
			goto _ExitWithError;
		}

		// Init tack array...

		for( i=0; i<nTackCount; ++i ) {
			m_pTackArray[i].Create_MS( this, &CFVec3A::m_Null );
		}
	}

	// Compute regular tetrahedron weights...
	for( i=0; i<4; ++i ) {
		ComputeWeights_MS( &m_aRegularTetraVtxWeights[i], &m_aRegularTetraPos_MS[i] );
	}

	// Set up default gravity...
	m_fGravity = -64.0f;

	// Clear force vectors...
	ClearForces();
	ClearImpulses();

	// We're our own master...
	m_pNetMaster = this;
	flinklist_InitRoot( &m_NetSlaveRoot, FANG_OFFSETOF( CFVerlet, m_NetSlaveLink ) );
	flinklist_AddTail( &m_NetSlaveRoot, this );

	m_nNetInScopeCount = 0;
	m_nNetMovingCount = 0;
	m_nNetManualWorkCount = 0;
	m_nNetLastWorkedFrame = 0;

	m_fPrimeSecs = 0.0f;
	m_fUnitDampen = 0.02f;
	m_fMotionlessSecsRemaining = _MOTIONLESS_COUNTDOWN_SECS;

	m_pWorldMesh = NULL;

	m_ppTackSurfaceArray = NULL;

	m_nBoneCount = 0;
	m_nBaseBoneIndex = 0;
	m_pBoneArray = NULL;

	m_pFcnMovedCallback = NULL;
	m_pFcnConstraintCallback = NULL;
	m_pFcnDeactivateCallback = NULL;
	m_pFcnExplodeCallback = NULL;
	m_pFcnTrackerCollCallback = NULL;

	SetCenterOfMass_MS( pCenterOfMass_MS );

	// Success...

	return TRUE;

	// Error...
_ExitWithError:
	Destroy();
	fres_ReleaseFrame( ResFrame );
	return FALSE;
}


// Creates a verlet object with tacks as discovered in the specified mesh.
// If pObjectDim is NULL, the dimensions of the object are taken from the mesh.
BOOL CFVerlet::Create( f32 fMass, const CFVec3A *pObjectDim, const FMesh_t *pMesh, const CFMtx43A *pUnitMtx, const CFVec3A *pCenterOfMass_MS ) {
	FASSERT( m_bSystemInitialized );
	FASSERT( !IsCreated() );

	CFVec3A ObjectDim;

	u32 nTackCount = CFVerletTack::DetermineTackCountFromMesh( pMesh );

	if( pObjectDim == NULL ) {
		ObjectDim.x = pMesh->vBoundBoxMax_MS.x - pMesh->vBoundBoxMin_MS.x;
		ObjectDim.y = pMesh->vBoundBoxMax_MS.y - pMesh->vBoundBoxMin_MS.y;
		ObjectDim.z = pMesh->vBoundBoxMax_MS.z - pMesh->vBoundBoxMin_MS.z;

		pObjectDim = &ObjectDim;
	}

	if( !Create( fMass, pObjectDim, nTackCount, pUnitMtx, pCenterOfMass_MS ) ) {
		// Trouble creating the CFVerlet object...
		return FALSE;
	}

	// Initialize the tack array properly...

	Tack_InitAllTacksFromMesh( pMesh );

	return TRUE;
}


BOOL CFVerlet::Create( const Init2_t *pInit2, const FMesh_t *pMesh ) {
	FASSERT( m_bSystemInitialized );
	FASSERT( !IsCreated() );
	FASSERT( pMesh );

	FMeshBone_t *pBone, *pChildBone;
	CFVerletTack *pTack;
	CFMtx43A UnitMtx, InvUnitMtx;
	CFVec3A ObjectDim;
	u32 i, j, nBoneCount, nBonedTackCountForOneBone, nTotalTackCount, nChildBoneIndex;
	s32 nBoneIndexInArray;
	BOOL bIsTack, bIsCTack;

	FResFrame_t ResFrame = fres_GetFrame();

	// Count the number of bones and boned tacks...
	nBoneCount = 0;
	nTotalTackCount = 0;

	for( i=0; i<pMesh->nBoneCount; ++i ) {
		pBone = pMesh->pBoneArray + i;

		nBoneIndexInArray = fclib_FindStringInArray( pBone->szName, pInit2->apszBoneNames );

		if( nBoneIndexInArray < 0 ) {
			// This bone is not a member of our object...
			continue;
		}

		if( nBoneIndexInArray == 0 ) {
			// This is the base bone...

			if( (fclib_FindStringInArray( pBone->szName, pInit2->apszBoneNameTacks ) >= 0) ||
				(fclib_FindStringInArray( pBone->szName, pInit2->apszBoneNameCTacks ) >= 0) )
			{
				++nTotalTackCount;
			}
		}

		for( j=0; j<pBone->Skeleton.nChildBoneCount; ++j ) {
			pChildBone = &pMesh->pBoneArray[ pMesh->pnSkeletonIndexArray[ pBone->Skeleton.nChildArrayStartIndex + j ] ];

			if( (fclib_FindStringInArray( pChildBone->szName, pInit2->apszBoneNameTacks ) >= 0) ||
				(fclib_FindStringInArray( pChildBone->szName, pInit2->apszBoneNameCTacks ) >= 0) )
			{
				++nTotalTackCount;
			}
		}

		++nBoneCount;
	}

	if( nBoneCount == 0 ) {
		DEVPRINTF( "CFVerlet::Create(): No bones specified. Cannot create.\n" );
		goto _ExitWithError;
	}

	ObjectDim.Set( pInit2->fDimX, pInit2->fDimY, pInit2->fDimZ );

	if( !Create( pInit2->fMass, &ObjectDim, nTotalTackCount ) ) {
		// Trouble creating the CFVerlet object...
		goto _ExitWithError;
	}

	m_nBoneCount = nBoneCount;

	m_pBoneArray = (Bone_t *)fres_Alloc( m_nBoneCount * sizeof(Bone_t) );
	if( m_pBoneArray == NULL ) {
		DEVPRINTF( "CFVerlet::Create(): Not enough memory to allocate boned tack array.\n" );
		goto _ExitWithError;
	}

	nBoneCount = 0;
	nTotalTackCount = 0;
	m_nBaseBoneIndex = -1;

	for( i=0; i<pMesh->nBoneCount; ++i ) {
		pBone = pMesh->pBoneArray + i;

		nBoneIndexInArray = fclib_FindStringInArray( pBone->szName, pInit2->apszBoneNames );

		if( nBoneIndexInArray < 0 ) {
			// This bone is not a member of our object...
			continue;
		}

		m_pBoneArray[nBoneCount].nBoneIndex = i;
		m_pBoneArray[nBoneCount].nTackStartIndex = nTotalTackCount;

		nBonedTackCountForOneBone = 0;

		if( nBoneIndexInArray == 0 ) {
			// This is the base bone...

			m_nBaseBoneIndex = i;

			bIsTack = bIsCTack = FALSE;

			if( fclib_FindStringInArray( pBone->szName, pInit2->apszBoneNameTacks ) >= 0 ) {
				bIsTack = TRUE;
			} else if( fclib_FindStringInArray( pBone->szName, pInit2->apszBoneNameCTacks ) >= 0 ) {
				bIsCTack = TRUE;
			}

			if( bIsTack || bIsCTack ) {
				pTack = m_pTackArray + nTotalTackCount;

				pTack->SetTackName( pBone->szName );
				pTack->EnableCollisionTack( bIsCTack );
				pTack->m_nBonedTackBoneIndex = -1;

				++nTotalTackCount;
			}
		}

		for( j=0; j<pBone->Skeleton.nChildBoneCount; ++j ) {
			nChildBoneIndex = pMesh->pnSkeletonIndexArray[ pBone->Skeleton.nChildArrayStartIndex + j ];
			pChildBone = &pMesh->pBoneArray[ nChildBoneIndex ];

			bIsTack = bIsCTack = FALSE;

			if( fclib_FindStringInArray( pChildBone->szName, pInit2->apszBoneNameTacks ) >= 0 ) {
				bIsTack = TRUE;
			} else if( fclib_FindStringInArray( pChildBone->szName, pInit2->apszBoneNameCTacks ) >= 0 ) {
				bIsCTack = TRUE;
			}

			if( bIsTack || bIsCTack ) {
				pTack = m_pTackArray + nTotalTackCount;

				pTack->SetTackName( pChildBone->szName );
				pTack->EnableCollisionTack( bIsCTack );

				pTack->m_TackPos_MS = pChildBone->AtRestBoneToParentMtx.m_vPos;

				++nTotalTackCount;

				if( nBoneIndexInArray ) {
					// Only non-base boned tacks need dynamic updating...
					pTack->m_nBonedTackBoneIndex = i;
					++nBonedTackCountForOneBone;
				} else {
					pTack->m_nBonedTackBoneIndex = -1;
				}
			}
		}

		m_pBoneArray[nBoneCount].nTackCount = nBonedTackCountForOneBone;

		++nBoneCount;
	}

	if( m_nBaseBoneIndex == -1 ) {
		DEVPRINTF( "CFVerlet::Create(): Base bone '%s' not found in mesh '%s'.\n", pInit2->apszBoneNames[0], pMesh->szName );
		goto _ExitWithError;
	}

	return TRUE;

_ExitWithError:
	Destroy();
	fres_ReleaseFrame( ResFrame );
	return FALSE;
}


void CFVerlet::AttachedToWorldMesh( CFWorldMesh *pWorldMesh ) {
	FASSERT( IsCreated() );

	CFMtx43A UnitMtx;

	SetWorldMesh( pWorldMesh );

	if( pWorldMesh == NULL ) {
		return;
	}

	if( m_pBoneArray ) {
		// This is a boned object...

		const CFMtx43A *pBoneMtx = pWorldMesh->GetBoneMtxPalette()[ m_nBaseBoneIndex ];

		UnitMtx.Mul33( *pBoneMtx, pBoneMtx->m_vX.InvMag() );
		UnitMtx.m_vPos = pBoneMtx->m_vPos;
	} else {
		// This is not a boned object...

		UnitMtx.Mul33( pWorldMesh->m_Xfm.m_MtxF, pWorldMesh->m_Xfm.m_fScaleR );
		UnitMtx.m_vPos = pWorldMesh->m_Xfm.m_MtxF.m_vPos;
	}

	InitUnitMtx( &UnitMtx );

	// Compute center-of-mass if there is only one bone...
	if( m_nBoneCount == 1 ) {
		CFVec3A CenterOfMass_WS;
		FMeshBone_t *pBone;

		pBone = &m_pWorldMesh->m_pMesh->pBoneArray[ m_nBaseBoneIndex ];

		CenterOfMass_WS.Set( pBone->SegmentedBoundSphere_BS.m_Pos );
		m_pWorldMesh->GetBoneMtxPalette()[ m_nBaseBoneIndex ]->MulPoint( CenterOfMass_WS );

		SetCenterOfMass_WS( &CenterOfMass_WS, FALSE );
	}

	_UpdateBonedTacks();
}


void CFVerlet::Destroy( void ) {
	if( IsCreated() ) {
		u32 i;

		if( m_nNetListMember == NET_LIST_MEMBER_ACTIVE ) {
			flinklist_Remove( &m_ActiveNetsRoot, this );
		} else if( m_nNetListMember == NET_LIST_MEMBER_INACTIVE ) {
			flinklist_Remove( &m_InactiveNetsRoot, this );
		}
		m_nNetListMember = NET_LIST_MEMBER_COUNT;

		m_pBoneArray = NULL;

		for( i=0; i<m_nTackCount; ++i ) {
			CFWire::ReturnToFreePool( m_pTackArray[i].m_pWire );
		}

		fdelete_array( m_pTackArray );
		m_pTackArray = NULL;
		m_nTackCount = 0;

		m_ppTackSurfaceArray = NULL;
		m_nTackSurfaceCount = 0;

		m_nFlags = FLAG_NONE;
	}

	SetBoneNameTackTable( NULL, NULL );
}


void CFVerlet::SetCenterOfMass_WS( const CFVec3A *pCenterOfMass_WS, BOOL bUpdateBonedTacks ) {
	FASSERT( IsCreated() );

	CFVec3A CenterOfMass_MS;

	WS2MS_Point( &CenterOfMass_MS, pCenterOfMass_WS );

	SetCenterOfMass_MS( &CenterOfMass_MS, bUpdateBonedTacks );
}


void CFVerlet::SetCenterOfMass_MS( const CFVec3A *pCenterOfMass_MS, BOOL bUpdateBonedTacks ) {
	FASSERT( IsCreated() );

	if( ((pCenterOfMass_MS == NULL) || (*pCenterOfMass_MS == CFVec3A::m_Null)) && !(m_nFlags & FLAG_HAS_NON_ZERO_COM) ) {
		return;
	}

	if( pCenterOfMass_MS && (*pCenterOfMass_MS == m_CenterOfMass_MS) ) {
		return;
	}

	CFVec3A CenterOfMass_WS;
	u32 i;

	if( m_nFlags & FLAG_HAS_NON_ZERO_COM ) {
		// Remove current origin offset...

		m_UnitMtx.MulDir( CenterOfMass_WS, m_CenterOfMass_MS );

		for( i=0; i<4; ++i ) {
			m_aTetraPos_WS[i].Sub( CenterOfMass_WS );
			m_aTetraOldPos_WS[i].Sub( CenterOfMass_WS );
		}
	}

	if( pCenterOfMass_MS ) {
		m_CenterOfMass_MS = *pCenterOfMass_MS;

		if( m_CenterOfMass_MS != CFVec3A::m_Null ) {
			FMATH_SETBITMASK( m_nFlags, FLAG_HAS_NON_ZERO_COM );

			// Add in new origin offset...
			m_UnitMtx.MulDir( CenterOfMass_WS, m_CenterOfMass_MS );

			for( i=0; i<4; ++i ) {
				m_aTetraPos_WS[i].Add( CenterOfMass_WS );
				m_aTetraOldPos_WS[i].Add( CenterOfMass_WS );
			}

			for( i=0; i<m_nTackCount; ++i ) {
				if( bUpdateBonedTacks || (m_pTackArray[i].m_nBonedTackBoneIndex < 0) ) {
					m_pTackArray[i]._RepositionTack( NULL );
				}
			}

			return;
		}
	} else {
		m_CenterOfMass_MS.Zero();
	}

	FMATH_CLEARBITMASK( m_nFlags, FLAG_HAS_NON_ZERO_COM );

	for( i=0; i<m_nTackCount; ++i ) {
		if( bUpdateBonedTacks || (m_pTackArray[i].m_nBonedTackBoneIndex < 0) ) {
			m_pTackArray[i]._RepositionTack( NULL );
		}
	}
}


// ppTackSurfaceArray must be terminated with a NULL.
void CFVerlet::SetTackSurfaceArray( CFVerletTackSurface **ppTackSurfaceArray ) {
	FASSERT( IsCreated() );

	m_ppTackSurfaceArray = ppTackSurfaceArray;

	for( m_nTackSurfaceCount=0; m_ppTackSurfaceArray[m_nTackSurfaceCount]; ++m_nTackSurfaceCount ) {}

	if( m_nTackSurfaceCount == 0 ) {
		m_ppTackSurfaceArray = NULL;
	}
}


// ppTackSurfaceArray doesn't need to be terminated.
void CFVerlet::SetTackSurfaceArray( CFVerletTackSurface **ppTackSurfaceArray, u32 nTackSurfaceCount ) {
	FASSERT( IsCreated() );

	m_ppTackSurfaceArray = ppTackSurfaceArray;
	m_nTackSurfaceCount = nTackSurfaceCount;

	if( m_nTackSurfaceCount == 0 ) {
		m_ppTackSurfaceArray = NULL;
	}
}


f32 CFVerlet::_ApproxCubeRoot( f32 fVal ) {
	f32 fI, fValCubed, fGuess;
	u32 i;

	if( fVal == 0.0f ) {
		return 0.0f;
	}

	fValCubed = fVal * fVal * fVal;

	for( i=0; i<200; ++i ) {
		fI = (f32)i;
		fGuess = fI * fI * fI;

		if( fGuess > fValCubed ) {
			break;
		}
	}
	if( i == 200 ) {
		return 200.0f;
	}

	FASSERT( i );

	fGuess = (f32)( i - 1 );
	FMATH_CLAMPMIN( fGuess, 0.5f );
	for( i=0; i<50; ++i ) {
		fGuess = fGuess - fmath_Div( fGuess*fGuess*fGuess - fVal, 3.0f*fGuess*fGuess );
	}

	return fGuess;
}


void CFVerlet::DrawAll( void ) {
	FASSERT( IsModuleInitialized() );

	#if !_DRAW_DEBUG_INFO
		return;
	#endif

	u32 nNetWorkIndex, nSlaveWorkIndex;

	frenderer_SetDefaultState();
	fdraw_Alpha_SetBlendOp( FDRAW_BLENDOP_LERP_WITH_ALPHA_OPAQUE );

	_FillNetArray( &m_ActiveNetsRoot );

	for( nNetWorkIndex=0; nNetWorkIndex < m_nNetWorkCount; ++nNetWorkIndex ) {
		m_ppNetWorkArray[nNetWorkIndex]->_FillSlaveArray();

		for( nSlaveWorkIndex=0; nSlaveWorkIndex < m_nVerletWorkCount; ++nSlaveWorkIndex ) {
			m_ppVerletWorkArray[nSlaveWorkIndex]->_Draw();
		}
	}

	_FillNetArray( &m_InactiveNetsRoot );

	for( nNetWorkIndex=0; nNetWorkIndex < m_nNetWorkCount; ++nNetWorkIndex ) {
		if( !m_ppNetWorkArray[nNetWorkIndex]->Net_IsFrozen() ) {
			m_ppNetWorkArray[nNetWorkIndex]->_FillSlaveArray();

			for( nSlaveWorkIndex=0; nSlaveWorkIndex < m_nVerletWorkCount; ++nSlaveWorkIndex ) {
				if( m_ppVerletWorkArray[nSlaveWorkIndex]->m_nFlags & FLAG_IN_SCOPE ) {
					m_ppVerletWorkArray[nSlaveWorkIndex]->_Draw();
				}
			}
		}
	}
}


void CFVerlet::_Draw( void ) {
	_DrawTetra( m_aTetraPos_WS );
	_DrawTacks();

	CFXfm Xfm;

	Xfm.BuildFromMtx( m_UnitMtx );
	Xfm.PushModel();

	fdraw_ModelSpaceAxis();

	Xfm.PopModel();
}


void CFVerlet::_DrawTetra( const CFVec3A *pTetraPosArray_WS ) {
	fdraw_FacetedWireSphere( &pTetraPosArray_WS[0].v3, 0.5f, 2, 2, &FColor_MotifRed, 8 );
	fdraw_FacetedWireSphere( &pTetraPosArray_WS[1].v3, 0.5f, 2, 2, &FColor_MotifGreen, 8 );
	fdraw_FacetedWireSphere( &pTetraPosArray_WS[2].v3, 0.5f, 2, 2, &FColor_MotifBlue, 8 );
	fdraw_FacetedWireSphere( &pTetraPosArray_WS[3].v3, 0.5f, 2, 2, &FColor_MotifWhite, 8 );

	fdraw_SolidLine( &pTetraPosArray_WS[0].v3, &pTetraPosArray_WS[3].v3, &FColor_MotifRed, &FColor_MotifWhite );
	fdraw_SolidLine( &pTetraPosArray_WS[1].v3, &pTetraPosArray_WS[3].v3, &FColor_MotifGreen, &FColor_MotifWhite );
	fdraw_SolidLine( &pTetraPosArray_WS[2].v3, &pTetraPosArray_WS[3].v3, &FColor_MotifBlue, &FColor_MotifWhite );

	fdraw_SolidLine( &pTetraPosArray_WS[0].v3, &pTetraPosArray_WS[1].v3, &FColor_MotifRed, &FColor_MotifGreen );
	fdraw_SolidLine( &pTetraPosArray_WS[0].v3, &pTetraPosArray_WS[2].v3, &FColor_MotifRed, &FColor_MotifBlue );
	fdraw_SolidLine( &pTetraPosArray_WS[1].v3, &pTetraPosArray_WS[2].v3, &FColor_MotifGreen, &FColor_MotifBlue );
}


void CFVerlet::_DrawTacks( void ) {
	const CFVerletTack *pTack;
	CFVec3A TackPos_WS, AnchorPos_WS;
	BOOL bDrawAnchorWire = FALSE;
	f32 fAlpha;
	u32 i;

	for( i=0; i<m_nTackCount; ++i ) {
		pTack = &m_pTackArray[i];

		pTack->ComputePos_WS( &TackPos_WS );

		fdraw_FacetedWireSphere( &TackPos_WS.v3, 0.25f, 2, 2, &CFColorMotif( 1.0f, 1.0f, 0.0f ), 8 );

		if( pTack->Anchor_IsEnabled() ) {
			fAlpha = 1.0f;
		} else {
			fAlpha = 0.25f;
		}

		if( pTack->Anchor_IsEnabled() ) {
			if( pTack->Anchor_GetType()==CFVerletTack::ANCHOR_TYPE_POINT_STATIC || pTack->Anchor_GetType()==CFVerletTack::ANCHOR_TYPE_POINT_DYNAMIC || pTack->Anchor_GetType()==CFVerletTack::ANCHOR_TYPE_POINT_BONED ) {
				AnchorPos_WS = pTack->Anchor_GetAnchorPoint()->v3;

				fdraw_FacetedWireSphere( &AnchorPos_WS.v3, 0.28f, 2, 2, &CFColorMotif( 0.0f, 1.0f, 1.0f, fAlpha ), 8 );

				bDrawAnchorWire = TRUE;
			} else if( pTack->Anchor_GetType() == CFVerletTack::ANCHOR_TYPE_TACK ) {
				CFVerletTack *pAnchorTack = pTack->Anchor_GetAnchorTack();
				if( pAnchorTack ) {
					pAnchorTack->ComputePos_WS( &AnchorPos_WS );

					fdraw_FacetedWireSphere( &AnchorPos_WS.v3, 0.28f, 2, 2, &CFColorMotif( 0.0f, 1.0f, 1.0f, fAlpha ), 8 );

					bDrawAnchorWire = TRUE;
				}
			}

			if( bDrawAnchorWire ) {
				fdraw_SolidLine( &TackPos_WS.v3, &AnchorPos_WS.v3, &CFColorMotif( 1.0f, 1.0f, 0.0f ), &CFColorMotif( 0.0f, 1.0f, 1.0f, fAlpha ) );
			}
		}
	}
}


// Establishes a new orientation matrix for the object.
// Snaps the tetrahedron points (current and old) accordingly.
void CFVerlet::InitUnitMtx( const CFMtx43A *pUnitMtx, BOOL bKickstartNet ) {
	FASSERT( IsCreated() );

	// Set new matrix...
	m_UnitMtx = *pUnitMtx;

	FMATH_CLEARBITMASK( m_nFlags, FLAG_INVMTX_BUILT );

	// Snap tetrahedron and tack points to the new matrix...
	_ComputeTetraVerticesFromMtx();

	if( bKickstartNet ) {
		Net_Kickstart();
	}
}


// Attempts to find the tack with the specified tack name.
CFVerletTack *CFVerlet::Tack_FindByName( cchar *pszTackName ) {
	FASSERT( IsCreated() );

	u32 i;

	for( i=0; i<m_nTackCount; ++i ) {
		if( !fclib_stricmp( pszTackName, m_pTackArray[i].m_pszName ) ) {
			// Found tack...
			return &m_pTackArray[i];
		}
	}

	// Tack not found...

	return NULL;
}


// Attempts to find the tack with the specified tack name.
// Returns its index, or -1 if the tack was not found.
s32 CFVerlet::Tack_FindIndexByName( cchar *pszTackName ) {
	FASSERT( IsCreated() );

	s32 i;

	for( i=0; i<m_nTackCount; ++i ) {
		if( !fclib_stricmp( pszTackName, m_pTackArray[i].m_pszName ) ) {
			// Found tack...
			return i;
		}
	}

	// Tack not found...

	return -1;
}


// Initializes the tack array from the tack definitions in the mesh.
void CFVerlet::Tack_InitAllTacksFromMesh( const FMesh_t *pMesh ) {
	FASSERT( IsCreated() );

	u32 nBoneIndex, nTackIndex;
	FMeshBone_t *pBone;
	CFVerletTack *pTack;
	cchar *pszTackName;
	BOOL bCTack;

	nTackIndex = 0;

	for( nBoneIndex=0, pBone=pMesh->pBoneArray; (nTackIndex < m_nTackCount) && (nBoneIndex < pMesh->nBoneCount); ++nBoneIndex, ++pBone ) {
		if( CFVerletTack::IsTackBone( pBone, &pszTackName, &bCTack ) ) {
			pTack = Tack_GetFromIndex( nTackIndex );

			pTack->Reposition_MS( &pBone->AtRestBoneToModelMtx.m_vPos );
			pTack->SetTackName( pszTackName );
			pTack->EnableCollisionTack( bCTack );

			++nTackIndex;
		}
	}
}


void CFVerlet::Net_MoveToInactiveList( void ) {
	FASSERT( IsCreated() );
	FASSERT( m_pNetMaster->m_nNetListMember==NET_LIST_MEMBER_ACTIVE || m_pNetMaster->m_nNetListMember==NET_LIST_MEMBER_INACTIVE );

	if( m_pNetMaster->m_nNetListMember == NET_LIST_MEMBER_ACTIVE ) {
		CFVerlet *pSlaveVerlet;
		CFVerletTack *pTack;
		u32 i;

		// Notify if requested...
		if( m_pFcnDeactivateCallback != NULL) {
			m_pFcnDeactivateCallback( this );
		}

		flinklist_Remove( &m_ActiveNetsRoot, m_pNetMaster );
		flinklist_AddTail( &m_InactiveNetsRoot, m_pNetMaster );

		// We've moved this net to the inactive list. Let's take a snapshot of any dynamic and boned
		// anchor positions so that we can kick the net back into the active list if the anchor moves...
		for( pSlaveVerlet=(CFVerlet *)flinklist_GetHead( &m_pNetMaster->m_NetSlaveRoot ); pSlaveVerlet; pSlaveVerlet=(CFVerlet *)flinklist_GetNext( &m_pNetMaster->m_NetSlaveRoot, pSlaveVerlet ) ) {
			pSlaveVerlet->StopMotion();
			pSlaveVerlet->ClearForces();
			pSlaveVerlet->ClearImpulses();

			if( pSlaveVerlet->m_nFlags & FLAG_HAS_DYNAMIC_ANCHOR ) {
				// This slave has a dynamic anchor...

				for( i=0; i<pSlaveVerlet->m_nTackCount; ++i ) {
					pTack = &pSlaveVerlet->m_pTackArray[i];

					if( pTack->Anchor_GetType() == CFVerletTack::ANCHOR_TYPE_POINT_DYNAMIC ) {
						// Found a dynamic anchor...

						pTack->m_AnchorPos_WS = *pTack->m_pAnchorPos_WS;
					}
				}
			}
		}

		m_pNetMaster->m_nNetListMember = NET_LIST_MEMBER_INACTIVE;
	}
}


void CFVerlet::Net_MoveToActiveList( void ) {
	FASSERT( IsCreated() );
	FASSERT( m_pNetMaster->m_nNetListMember==NET_LIST_MEMBER_ACTIVE || m_pNetMaster->m_nNetListMember==NET_LIST_MEMBER_INACTIVE );

	if( m_pNetMaster->m_nFlags & FLAG_NET_MASTER_FREEZE ) {
		return;
	}

	if( m_pNetMaster->m_nFlags & FLAG_NET_MASTER_ONE_SHOT ) {
		FMATH_SETBITMASK( m_pNetMaster->m_nFlags, FLAG_NET_MASTER_FREEZE_WHEN_MOTIONLESS );
	}

	if( m_pNetMaster->m_nNetListMember == NET_LIST_MEMBER_INACTIVE ) {
		flinklist_Remove( &m_InactiveNetsRoot, m_pNetMaster );
		flinklist_AddTail( &m_ActiveNetsRoot, m_pNetMaster );

		m_pNetMaster->m_nNetListMember = NET_LIST_MEMBER_ACTIVE;

		CFVerlet *pSlaveVerlet;

		for( pSlaveVerlet=(CFVerlet *)flinklist_GetHead( &m_pNetMaster->m_NetSlaveRoot ); pSlaveVerlet; pSlaveVerlet=(CFVerlet *)flinklist_GetNext( &m_pNetMaster->m_NetSlaveRoot, pSlaveVerlet ) ) {
			pSlaveVerlet->m_fMotionlessSecsRemaining = _MOTIONLESS_COUNTDOWN_SECS;

			if( !(pSlaveVerlet->m_nFlags & FLAG_MOVING) ) {
				FMATH_SETBITMASK( pSlaveVerlet->m_nFlags, FLAG_MOVING );

				FASSERT( m_pNetMaster->m_nNetMovingCount < 255 );
				++m_pNetMaster->m_nNetMovingCount;
			}
		}
	}
}


void CFVerlet::Net_Freeze( BOOL bFreeze ) {
	FASSERT( IsCreated() );
	FASSERT( m_pNetMaster->m_nNetListMember==NET_LIST_MEMBER_ACTIVE || m_pNetMaster->m_nNetListMember==NET_LIST_MEMBER_INACTIVE );

	if( bFreeze ) {
		if( !(m_pNetMaster->m_nFlags & FLAG_NET_MASTER_FREEZE) ) {
			// Freeze the net...
			FMATH_SETBITMASK( m_pNetMaster->m_nFlags, FLAG_NET_MASTER_FREEZE );
			Net_MoveToInactiveList();
		}
	} else {
		if( m_pNetMaster->m_nFlags & FLAG_NET_MASTER_FREEZE ) {
			// Unfreeze the net...
			FMATH_CLEARBITMASK( m_pNetMaster->m_nFlags, FLAG_NET_MASTER_FREEZE );

			// Check to see if we have a mesh associated with ourself and if that mesh is visible.
			// If we have a mesh and it's visible, then make ourself IN_SCOPE so the Net_Kickstart will work
			// (Net_Kickstart works only for objects that are IN_SCOPE).

			if( m_pWorldMesh && ( m_pWorldMesh->GetVolumeFlags() & FVIS_VOLUME_IN_ACTIVE_LIST ) ) {
				SetInScope( TRUE );
			}

			Net_Kickstart();
		}
	}
}


void CFVerlet::Net_MakeOneShot( BOOL bOneShot ) {
	FASSERT( IsCreated() );
	FASSERT( m_pNetMaster->m_nNetListMember==NET_LIST_MEMBER_ACTIVE || m_pNetMaster->m_nNetListMember==NET_LIST_MEMBER_INACTIVE );

	if( bOneShot ) {
		FMATH_SETBITMASK( m_nFlags, FLAG_NET_MASTER_ONE_SHOT );
	} else {
		FMATH_CLEARBITMASK( m_nFlags, FLAG_NET_MASTER_ONE_SHOT | FLAG_NET_MASTER_FREEZE_WHEN_MOTIONLESS | FLAG_NET_MASTER_CTACK_HIT_CPLANE );
	}
}


// Merges the two specified nets. If the nets are the same, this function
// simply returns. Otherwise, net 2 is merged into net 1. If net 2 is
// currently in the active list, then net 1 will be promoted to the active
// list if it's not already.
void CFVerlet::_Net_Merge( CFVerlet *pNetMaster1, CFVerlet *pNetMaster2 ) {
	FASSERT( pNetMaster1->m_pNetMaster == pNetMaster1 );
	FASSERT( pNetMaster2->m_pNetMaster == pNetMaster2 );
	FASSERT( pNetMaster1->m_nNetListMember==NET_LIST_MEMBER_ACTIVE || pNetMaster1->m_nNetListMember==NET_LIST_MEMBER_INACTIVE );
	FASSERT( pNetMaster2->m_nNetListMember==NET_LIST_MEMBER_ACTIVE || pNetMaster2->m_nNetListMember==NET_LIST_MEMBER_INACTIVE );

	if( pNetMaster1 == pNetMaster2 ) {
		// Already in the same net...
		return;
	}

	if( !(pNetMaster2->m_nFlags & FLAG_NET_MASTER_FREEZE) ) {
		// Net 2 is not frozen, so unfreeze net 1...
		FMATH_CLEARBITMASK( pNetMaster1->m_nFlags, FLAG_NET_MASTER_FREEZE );
	}

	if( (pNetMaster1->m_nFlags & FLAG_NET_MASTER_ONE_SHOT) || (pNetMaster2->m_nFlags & FLAG_NET_MASTER_ONE_SHOT) ) {
		// If either net is a one shot, the merged net will also be...
		FMATH_SETBITMASK( pNetMaster1->m_nFlags, FLAG_NET_MASTER_ONE_SHOT );
		FMATH_SETBITMASK( pNetMaster2->m_nFlags, FLAG_NET_MASTER_ONE_SHOT );
	}

	if( (pNetMaster1->m_nFlags & FLAG_NET_MASTER_FREEZE_WHEN_MOTIONLESS) || (pNetMaster2->m_nFlags & FLAG_NET_MASTER_FREEZE_WHEN_MOTIONLESS) ) {
		// If either net is flagged to freeze upon no motion, the merged net will also be...
		FMATH_SETBITMASK( pNetMaster1->m_nFlags, FLAG_NET_MASTER_FREEZE_WHEN_MOTIONLESS );
		FMATH_SETBITMASK( pNetMaster2->m_nFlags, FLAG_NET_MASTER_FREEZE_WHEN_MOTIONLESS );
	}

	if( (pNetMaster1->m_nFlags & FLAG_NET_MASTER_CTACK_HIT_CPLANE) || (pNetMaster2->m_nFlags & FLAG_NET_MASTER_CTACK_HIT_CPLANE) ) {
		// If either net is flagged as hitting a collision plane, the merged net will also be...
		FMATH_SETBITMASK( pNetMaster1->m_nFlags, FLAG_NET_MASTER_CTACK_HIT_CPLANE );
		FMATH_SETBITMASK( pNetMaster2->m_nFlags, FLAG_NET_MASTER_CTACK_HIT_CPLANE );
	}

	if( pNetMaster2->m_nNetListMember == NET_LIST_MEMBER_ACTIVE ) {
		// Since net 2 is in the active list, let's promote net 1 to the active list...
		pNetMaster1->Net_MoveToActiveList();

		// Remove net 2 from the active list...
		flinklist_Remove( &m_ActiveNetsRoot, pNetMaster2 );
	} else {
		// Remove net 2 from the inactive list...
		flinklist_Remove( &m_InactiveNetsRoot, pNetMaster2 );
	}

	CFVerlet *pVerlet;

	// Move members from net 2 to net 1...
	while( pVerlet = (CFVerlet *)flinklist_RemoveHead( &pNetMaster2->m_NetSlaveRoot ) ) {
		flinklist_AddTail( &pNetMaster1->m_NetSlaveRoot, pVerlet );
		pVerlet->m_pNetMaster = pNetMaster1;

		if( pVerlet->m_nFlags & FLAG_IN_SCOPE ) {
			FASSERT( pNetMaster1->m_nNetInScopeCount < 255 );
			++pNetMaster1->m_nNetInScopeCount;
		}

		if( pVerlet->m_nFlags & FLAG_MOVING ) {
			FASSERT( pNetMaster1->m_nNetMovingCount < 255 );
			++pNetMaster1->m_nNetMovingCount;
		}

		if( pVerlet->m_nFlags & FLAG_MANUAL_WORK ) {
			FASSERT( pNetMaster1->m_nNetManualWorkCount < 255 );
			++pNetMaster1->m_nNetManualWorkCount;
		}

		pVerlet->m_nNetInScopeCount = -1;
		pVerlet->m_nNetMovingCount = -1;
		pVerlet->m_nNetManualWorkCount = -1;
		pVerlet->m_nNetListMember = NET_LIST_MEMBER_COUNT;
	}

	// Kickstart the net...
	pNetMaster1->Net_Kickstart();
}


// Attempts to sever the net between the two specified Verlet objects.
// It is assumed that an anchor between these two Verlet objects has
// already been destroyed. If the two Verlet objects are no longer
// connected via the tack pointers, this function will split the net
// into two nets and create a new net master.
void CFVerlet::_Net_Sever( CFVerlet *pVerlet1, CFVerlet *pVerlet2 ) {
	if( pVerlet1->m_pNetMaster != pVerlet2->m_pNetMaster ) {
		// Already severed...
		return;
	}

	CFVerlet *pNetMaster, *pNewNetMaster;

	pNetMaster = pVerlet1->m_pNetMaster;

	FASSERT( pNetMaster->m_nNetListMember==NET_LIST_MEMBER_ACTIVE || pNetMaster->m_nNetListMember==NET_LIST_MEMBER_INACTIVE );

	_Net_ResetVisitedFlag( pNetMaster );
	if( _Net_Find( pVerlet1, pVerlet2 ) ) {
		// Verlet2 is still connected to Verlet1, so we can't sever...
		return;
	}

	// Verlet1 and Verlet2 are no longer connected, so we can sever them!

	// Find out which half of the split net the master is on...
	_Net_ResetVisitedFlag( pNetMaster );
	if( _Net_Find( pVerlet1, pNetMaster ) ) {
		// The net master is part of the Verlet1 half...

		pNewNetMaster = pVerlet2;
	} else {
		// The net master is part if the Verlet2 half...

		#if FANG_DEBUG_BUILD
			_Net_ResetVisitedFlag( pNetMaster );
			FASSERT( _Net_Find( pVerlet2, pNetMaster ) );
		#endif

		pNewNetMaster = pVerlet1;
	}
	//make sure the new net master assumes the LastWorked value of the original net master
	pNewNetMaster->m_nNetLastWorkedFrame = pNetMaster->m_nNetLastWorkedFrame;

	// Build our new master...
	flinklist_AddTail( &m_InactiveNetsRoot, pNewNetMaster );
	pNewNetMaster->m_nNetListMember = NET_LIST_MEMBER_INACTIVE;

	// Copy the freeze flag from the original net to the new net...
	if( pNetMaster->m_nFlags & FLAG_NET_MASTER_FREEZE ) {
		FMATH_SETBITMASK( pNewNetMaster->m_nFlags, FLAG_NET_MASTER_FREEZE );
	} else {
		FMATH_CLEARBITMASK( pNewNetMaster->m_nFlags, FLAG_NET_MASTER_FREEZE );
	}

	// Copy the one-shot flag from the original net to the new net...
	if( pNetMaster->m_nFlags & FLAG_NET_MASTER_ONE_SHOT ) {
		FMATH_SETBITMASK( pNewNetMaster->m_nFlags, FLAG_NET_MASTER_ONE_SHOT );
	} else {
		FMATH_CLEARBITMASK( pNewNetMaster->m_nFlags, FLAG_NET_MASTER_ONE_SHOT );
	}

	// Copy the freeze-when-motionless flag from the original net to the new net...
	if( pNetMaster->m_nFlags & FLAG_NET_MASTER_FREEZE_WHEN_MOTIONLESS ) {
		FMATH_SETBITMASK( pNewNetMaster->m_nFlags, FLAG_NET_MASTER_FREEZE_WHEN_MOTIONLESS );
	} else {
		FMATH_CLEARBITMASK( pNewNetMaster->m_nFlags, FLAG_NET_MASTER_FREEZE_WHEN_MOTIONLESS );
	}

	// Copy the collided-with-plane flag from the original net to the new net...
	if( pNetMaster->m_nFlags & FLAG_NET_MASTER_CTACK_HIT_CPLANE ) {
		FMATH_SETBITMASK( pNewNetMaster->m_nFlags, FLAG_NET_MASTER_CTACK_HIT_CPLANE );
	} else {
		FMATH_CLEARBITMASK( pNewNetMaster->m_nFlags, FLAG_NET_MASTER_CTACK_HIT_CPLANE );
	}

	//RAFNOTE: I don't think the following InitRoot call is necessary, but I'll leave it in anyway
	flinklist_InitRoot( &pNewNetMaster->m_NetSlaveRoot, FANG_OFFSETOF( CFVerlet, m_NetSlaveLink ) );

	pNewNetMaster->m_nNetInScopeCount = 0;
	pNewNetMaster->m_nNetMovingCount = 0;
	pNewNetMaster->m_nNetManualWorkCount = 0;

	// Now transfer members from current net master to new net master...
	_Net_ResetVisitedFlag( pNetMaster );
	_Net_TransferMembersToNewMaster( pNewNetMaster, pNewNetMaster );

	// Kickstart both nets...
	pNetMaster->Net_MoveToInactiveList();
	pNetMaster->Net_Kickstart();
	pNewNetMaster->Net_Kickstart();
}


// Resets the FLAG_VISITED flag in every member of the specified net master.
void CFVerlet::_Net_ResetVisitedFlag( CFVerlet *pNetMaster ) {
	FASSERT( pNetMaster->m_pNetMaster == pNetMaster );

	CFVerlet *pVerlet;

	for( pVerlet=(CFVerlet *)flinklist_GetHead( &pNetMaster->m_NetSlaveRoot ); pVerlet; pVerlet=(CFVerlet *)flinklist_GetNext( &pNetMaster->m_NetSlaveRoot, pVerlet ) ) {
		FMATH_CLEARBITMASK( pVerlet->m_nFlags, FLAG_VISITED );
	}
}


// Attempts to find pVerletToFind in the net of which pVerlet is a member.
// This function is recursive.
//
// IMPORTANT: Call _Net_ResetVisitedFlag() prior to calling this function!
BOOL CFVerlet::_Net_Find( CFVerlet *pVerlet, const CFVerlet *pVerletToFind ) {
	if( pVerlet == pVerletToFind ) {
		// Found it!
		return TRUE;
	}

	FMATH_SETBITMASK( pVerlet->m_nFlags, FLAG_VISITED );

	u32 i;
	const CFVerletTack *pTack;
	CFVerlet *pNextVerlet;

	for( i=0, pTack=pVerlet->m_pTackArray; i<pVerlet->m_nTackCount; ++i, ++pTack ) {
		if( pTack->m_nFlags & CFVerletTack::FLAG_ANCHOR_ENABLED ) {
			if( pTack->m_nAnchorType == CFVerletTack::ANCHOR_TYPE_TACK ) {
				pNextVerlet = pTack->m_pAnchorTack->m_pTackOwner;

				if( !(pNextVerlet->m_nFlags & FLAG_VISITED) ) {
					// Haven't already visited this verlet object...

					if( _Net_Find( pNextVerlet, pVerletToFind ) ) {
						// Found it!
						return TRUE;
					}
				}
			}
		}

		if( pTack->m_pTackAnchoredToMe ) {
			FASSERT( pTack->m_pTackAnchoredToMe->m_nAnchorType == CFVerletTack::ANCHOR_TYPE_TACK );

			if( pTack->m_pTackAnchoredToMe->m_nFlags & CFVerletTack::FLAG_ANCHOR_ENABLED ) {
				pNextVerlet = pTack->m_pTackAnchoredToMe->m_pTackOwner;

				if( !(pNextVerlet->m_nFlags & FLAG_VISITED) ) {
					// Haven't already visited this verlet object...

					if( _Net_Find( pNextVerlet, pVerletToFind ) ) {
						// Found it!
						return TRUE;
					}
				}
			}
		}
	}

	return FALSE;
}


// Transfers all Verlet objects connected to pVerlet (including pVerlet) from its
// current net master to the new specified net master.
//
// IMPORTANT: Call _Net_ResetVisitedFlag() prior to calling this function!
void CFVerlet::_Net_TransferMembersToNewMaster( CFVerlet *pVerlet, CFVerlet *pNewNetMaster ) {
	FMATH_SETBITMASK( pVerlet->m_nFlags, FLAG_VISITED );

	if( pVerlet->m_nFlags & FLAG_IN_SCOPE ) {
		FASSERT( pVerlet->m_pNetMaster->m_nNetInScopeCount > 0 );
		FASSERT( pNewNetMaster->m_nNetInScopeCount < 255 );

		--pVerlet->m_pNetMaster->m_nNetInScopeCount;
		++pNewNetMaster->m_nNetInScopeCount;
	}

	if( pVerlet->m_nFlags & FLAG_MOVING ) {
		FASSERT( pVerlet->m_pNetMaster->m_nNetMovingCount > 0 );
		FASSERT( pNewNetMaster->m_nNetMovingCount < 255 );

		--pVerlet->m_pNetMaster->m_nNetMovingCount;
		++pNewNetMaster->m_nNetMovingCount;
	}

	if( pVerlet->m_nFlags & FLAG_MANUAL_WORK ) {
		FASSERT( pVerlet->m_pNetMaster->m_nNetManualWorkCount > 0 );
		FASSERT( pNewNetMaster->m_nNetManualWorkCount < 255 );

		--pVerlet->m_pNetMaster->m_nNetManualWorkCount;
		++pNewNetMaster->m_nNetManualWorkCount;
	}

	// Remove from current master...
	flinklist_Remove( &pVerlet->m_pNetMaster->m_NetSlaveRoot, pVerlet );

	// Add to new master...
	flinklist_AddTail( &pNewNetMaster->m_NetSlaveRoot, pVerlet );
	pVerlet->m_pNetMaster = pNewNetMaster;

	u32 i;
	const CFVerletTack *pTack;
	CFVerlet *pNextVerlet;

	for( i=0, pTack=pVerlet->m_pTackArray; i<pVerlet->m_nTackCount; ++i, ++pTack ) {
		if( pTack->m_nFlags & CFVerletTack::FLAG_ANCHOR_ENABLED ) {
			if( pTack->m_nAnchorType == CFVerletTack::ANCHOR_TYPE_TACK ) {
				pNextVerlet = pTack->m_pAnchorTack->m_pTackOwner;

				if( !(pNextVerlet->m_nFlags & FLAG_VISITED) ) {
					// Haven't already visited this verlet object...

					_Net_TransferMembersToNewMaster( pNextVerlet, pNewNetMaster );
				}
			}
		}

		if( pTack->m_pTackAnchoredToMe ) {
			FASSERT( pTack->m_pTackAnchoredToMe->m_nAnchorType == CFVerletTack::ANCHOR_TYPE_TACK );

			if( pTack->m_pTackAnchoredToMe->m_nFlags & CFVerletTack::FLAG_ANCHOR_ENABLED ) {
				pNextVerlet = pTack->m_pTackAnchoredToMe->m_pTackOwner;

				if( !(pNextVerlet->m_nFlags & FLAG_VISITED) ) {
					// Haven't already visited this verlet object...

					_Net_TransferMembersToNewMaster( pNextVerlet, pNewNetMaster );
				}
			}
		}
	}
}


// Hints to the specified net master that something might have introduced motion into the net.
// This function adds the net to the active list and resets the motion detection timers of
// all its members.
void CFVerlet::_Net_Kickstart( CFVerlet *pNetMaster ) {
	FASSERT( pNetMaster->m_pNetMaster == pNetMaster );
	FASSERT( pNetMaster->m_nNetInScopeCount );
	FASSERT( pNetMaster->m_nNetListMember == NET_LIST_MEMBER_INACTIVE );

	if( pNetMaster->m_nFlags & FLAG_NET_MASTER_FREEZE ) {
		return;
	}

	pNetMaster->Net_MoveToActiveList();

	CFVerlet *pVerlet;

	for( pVerlet=(CFVerlet *)flinklist_GetHead( &pNetMaster->m_NetSlaveRoot ); pVerlet; pVerlet=(CFVerlet *)flinklist_GetNext( &pNetMaster->m_NetSlaveRoot, pVerlet ) ) {
		pVerlet->m_fMotionlessSecsRemaining = _MOTIONLESS_COUNTDOWN_SECS;
	}
}


// Computes the tetrahedron vertices using the current matrix.
void CFVerlet::_ComputeTetraVerticesFromMtx( void ) {
	CFVec3A ScaledVec;
	u32 i;

	if( !(m_nFlags & FLAG_HAS_NON_ZERO_COM) ) {
		for( i=0; i<4; ++i ) {
			ScaledVec.Mul( m_aTetraPos_MS[i], m_TetraDim );
			m_UnitMtx.MulPoint( m_aTetraPos_WS[i], ScaledVec );
			m_aTetraOldPos_WS[i] = m_aTetraPos_WS[i];
		}
	} else {
		CFVec3A CenterOfMass_WS;

		m_UnitMtx.MulDir( CenterOfMass_WS, m_CenterOfMass_MS );

		for( i=0; i<4; ++i ) {
			ScaledVec.Mul( m_aTetraPos_MS[i], m_TetraDim );
			m_UnitMtx.MulPoint( m_aTetraPos_WS[i], ScaledVec );
			m_aTetraPos_WS[i].Add( CenterOfMass_WS );
			m_aTetraOldPos_WS[i] = m_aTetraPos_WS[i];
		}
	}
}


void CFVerlet::SetInScope( BOOL bInScope ) {
	FASSERT( IsCreated() );
	FASSERT( m_pNetMaster->m_nNetListMember==NET_LIST_MEMBER_ACTIVE || m_pNetMaster->m_nNetListMember==NET_LIST_MEMBER_INACTIVE );

	if( bInScope ) {
		if( !(m_nFlags & FLAG_IN_SCOPE) ) {
			FMATH_SETBITMASK( m_nFlags, FLAG_IN_SCOPE );

			FASSERT( m_pNetMaster->m_nNetInScopeCount < 255 );
			++m_pNetMaster->m_nNetInScopeCount;

			if( !(m_pNetMaster->m_nFlags & FLAG_NET_MASTER_FREEZE) ) {
				if( m_pNetMaster->m_nNetInScopeCount == 1 ) {
					if( m_pNetMaster->m_nNetMovingCount ) {
						Net_MoveToActiveList();
					}
				}
			}

			// Since we're in scope, get anchor wires...

			CFVerlet *pVerlet;
			CFVerletTack *pTack;
			u32 nTackIndex;

			for( pVerlet=(CFVerlet *)flinklist_GetHead( &m_pNetMaster->m_NetSlaveRoot ); pVerlet; pVerlet=(CFVerlet *)flinklist_GetNext( &m_pNetMaster->m_NetSlaveRoot, pVerlet ) ) {
				for( nTackIndex=0; nTackIndex<pVerlet->m_nTackCount; ++nTackIndex ) {
					pTack = pVerlet->m_pTackArray + nTackIndex;

					pTack->_UpdateTackWire();
				}
			}
		}
	} else {
		if( m_nFlags & FLAG_IN_SCOPE ) {
			FMATH_CLEARBITMASK( m_nFlags, FLAG_IN_SCOPE );

			FASSERT( m_pNetMaster->m_nNetInScopeCount > 0 );
			--m_pNetMaster->m_nNetInScopeCount;

			if( m_pNetMaster->m_nNetInScopeCount == 0 ) {
				Net_MoveToInactiveList();
			}

			// Since we're out of scope, remove all wires...

			CFVerlet *pVerlet;
			CFVerletTack *pTack;
			u32 nTackIndex;

			for( pVerlet=(CFVerlet *)flinklist_GetHead( &m_pNetMaster->m_NetSlaveRoot ); pVerlet; pVerlet=(CFVerlet *)flinklist_GetNext( &m_pNetMaster->m_NetSlaveRoot, pVerlet ) ) {
				for( nTackIndex=0; nTackIndex<pVerlet->m_nTackCount; ++nTackIndex ) {
					pTack = pVerlet->m_pTackArray + nTackIndex;

					if( pTack->m_pWire ) {
						CFWire::ReturnToFreePool( pTack->m_pWire );
						pTack->m_pWire = NULL;
					}
				}
			}
		}
	}
}


// Performs verlet integration on the object.
void CFVerlet::Integrate( f32 fDeltaSecs, BOOL bClearForces ) {
	FASSERT( IsCreated() );

	CFVec3A OldPos_WS, Force_WS, GravityForce_WS, Vel_WS, DeltaSecs2Vec;
	f32 fDeltaSecs2;
	u32 i;

	// Process time step...
	fDeltaSecs2 = fDeltaSecs * fDeltaSecs;
	DeltaSecs2Vec.Set( fDeltaSecs2 );

	// Compute gravity force...
	GravityForce_WS.Set( 0.0f, m_fGravity * fDeltaSecs2, 0.0f );

	// Apply dampening...
	ApplyDampening( m_fUnitDampen, fDeltaSecs );

	for( i=0; i<4; ++i ) {
		// Store what will become our old position...
		OldPos_WS = m_aTetraPos_WS[i];

		// Compute velocity...
		Vel_WS.Sub( m_aTetraPos_WS[i], m_aTetraOldPos_WS[i] );

		// Integrate and add gravity...
		m_aTetraPos_WS[i].Add( Vel_WS );
		m_aTetraPos_WS[i].Add( GravityForce_WS );

		// Compute the force acting on this tetrahedron vertex...
		Force_WS.Mul( m_aTetraForce_WS[i], DeltaSecs2Vec );
		m_aTetraPos_WS[i].Add( Force_WS );

		// Compute the impulse forces acting on this tetrahedron vertex...
		Force_WS.Mul( m_aTetraImpulse_WS[i], (_IMPULSE_DURATION * _IMPULSE_DURATION) );
		m_aTetraPos_WS[i].Add( Force_WS );

		// Update old position...
		m_aTetraOldPos_WS[i] = OldPos_WS;
	}

	// Zero forces...
	if( bClearForces ) {
		ClearForces();
	}

	// Zero impulses...
	ClearImpulses();
}


void CFVerlet::_HandleMotionlessDetection( f32 fDeltaSecs, f32 fInvDeltaSecs ) {
	CFVec3A RegularTetraVtxVel_WS;
	f32 fSpeed2_WS;
	u32 i, nMotionlessVertexCount;

	nMotionlessVertexCount = 0;

	for( i=0; i<4; ++i ) {
		// Compute speed at regular tetrahedron vertex...
		ComputeSurfaceVelocity_WS( &RegularTetraVtxVel_WS, &m_aRegularTetraVtxWeights[i], fInvDeltaSecs );
		fSpeed2_WS = RegularTetraVtxVel_WS.MagSq();
		if( fSpeed2_WS < (_MOTIONLESS_SPEED*_MOTIONLESS_SPEED) ) {
			++nMotionlessVertexCount;
		}
	}

	if( nMotionlessVertexCount < 4 ) {
		// Not all vertices are motionless...

		m_fMotionlessSecsRemaining = _MOTIONLESS_COUNTDOWN_SECS;

		if( !(m_nFlags & FLAG_MOVING) ) {
			FMATH_SETBITMASK( m_nFlags, FLAG_MOVING );

			FASSERT( m_pNetMaster->m_nNetMovingCount < 255 );
			++m_pNetMaster->m_nNetMovingCount;

			if( m_pNetMaster->m_nNetMovingCount == 1 ) {
				Net_Kickstart();
			}
		}
	} else {
		// All vertices are motionless...

		m_fMotionlessSecsRemaining -= fDeltaSecs;

		if( m_fMotionlessSecsRemaining <= 0.0f ) {
			m_fMotionlessSecsRemaining = 0.0f;

			if( m_nFlags & FLAG_MOVING ) {
				FMATH_CLEARBITMASK( m_nFlags, FLAG_MOVING );

				FASSERT( m_pNetMaster->m_nNetMovingCount > 0 );
				--m_pNetMaster->m_nNetMovingCount;

				if( m_pNetMaster->m_nNetMovingCount == 0 ) {
					// No more moving objects in this net!

					// Notify that we've stopped if requested...
					if( m_pFcnDeactivateCallback ) {
						m_pFcnDeactivateCallback( this );
					}

					Net_MoveToInactiveList();

					if( m_pNetMaster->m_nFlags & FLAG_NET_MASTER_FREEZE_WHEN_MOTIONLESS ) {
						if( m_pNetMaster->m_nFlags & FLAG_NET_MASTER_CTACK_HIT_CPLANE ) {
							Net_Freeze( TRUE );
						}

						FMATH_CLEARBITMASK( m_pNetMaster->m_nFlags, FLAG_NET_MASTER_FREEZE_WHEN_MOTIONLESS | FLAG_NET_MASTER_CTACK_HIT_CPLANE );
					}
				}
			}
		}
	}
}


void CFVerlet::ApplyDampening( f32 fUnitDampen, f32 fDeltaSecs ) {
	CFVec3A UnitVelDir_WS;
	f32 fUnitDeltaTime;
	u32 i;

	fUnitDeltaTime = fDeltaSecs * 30.0f;
	FMATH_CLAMPMAX( fUnitDeltaTime, 1.0f );

	fUnitDampen = 1.0f - fUnitDampen*fUnitDeltaTime;

	for( i=0; i<4; ++i ) {
		UnitVelDir_WS.Sub( m_aTetraPos_WS[i], m_aTetraOldPos_WS[i] );
		UnitVelDir_WS.Mul( fUnitDampen );
		m_aTetraOldPos_WS[i].Sub( m_aTetraPos_WS[i], UnitVelDir_WS );
	}
}


// Performs one iteration of contraint application for the tetrahedron
// and tack points.
void CFVerlet::SatisfyConstraints( void ) {
	FASSERT( IsCreated() );

	// Apply any user-inforced constraints...
	if( m_pFcnConstraintCallback ) {
		m_pFcnConstraintCallback( this );
	}

	// Apply tack surface collisions...
	if( m_ppTackSurfaceArray ) {
		if( m_nCollTackCount ) {
			u32 i;

			for( i=0; i<m_nTackSurfaceCount; ++i ) {
				if( m_ppTackSurfaceArray[i]->IsEnabled() ) {
					_CollideTacksWithTackSurface( m_ppTackSurfaceArray[i] );
				}
			}
		}
	}

	// Apply tack and tetra constraints...
	_ApplyTackConstraints();
	_ApplyTetraConstraints();
}


void CFVerlet::CollideTacksWithTackSurface( CFVerletTackSurface *pTackSurface ) {
	if( m_nCollTackCount ) {
		if( pTackSurface->IsEnabled() ) {
			_CollideTacksWithTackSurface( pTackSurface );
		}
	}
}


void CFVerlet::_CollideTacksWithTackSurface( CFVerletTackSurface *pTackSurface ) {
	pTackSurface->ComputeUnitMtx();

	switch( pTackSurface->m_nType ) {
	case CFVerletTackSurface::TYPE_PLANE:
		{
			const CFVec3A *pUnitNormal_WS;
			CFVec3A UnitNormal_WS;

			pUnitNormal_WS = &(&pTackSurface->m_pUnitMtx->m_vX)[ pTackSurface->m_nPlaneAxisIndex ];

			if( pTackSurface->m_nFlags & CFVerletTackSurface::FLAG_PLANE_NEGATE_NORMAL ) {
				UnitNormal_WS.ReceiveNegative( *pUnitNormal_WS );
				pUnitNormal_WS = &UnitNormal_WS;
			}

			CollideTacksWithPlane( &pTackSurface->m_pUnitMtx->m_vPos, pUnitNormal_WS, pTackSurface->m_fFriction, pTackSurface->m_fPlaneDist );
		}

		break;

	case CFVerletTackSurface::TYPE_BOX:
		CollideTacksWithBox( pTackSurface->m_pUnitMtx, &pTackSurface->m_BoxMinCorner_WS, &pTackSurface->m_BoxMaxCorner_WS, pTackSurface->m_fFriction );

		break;

	default:
		FASSERT_NOW;
	};
}


void CFVerlet::_ApplyPlaneConstraint( const CFVec3A *pPos_WS, const CFVec3A *pUnitAxis, const CFVec4 *pScaledWeights, f32 fPenetrationDist, f32 fFriction, BOOL bPreserveVelocity ) {
	CFVec3A PointOnPlane_WS, SurfaceVel_WS, DeltaVel_WS, aDeltaPos_WS[4];
	u32 i;

	FMATH_SETBITMASK( m_pNetMaster->m_nFlags, FLAG_NET_MASTER_CTACK_HIT_CPLANE );

	if( m_nFlags & FLAG_EXPLODE_WHEN_TACK_HITS_SURFACE ) {
		if( m_fGravity <= 0.0f ) {
			if( pUnitAxis->y >= 0.707f ) {
				FMATH_SETBITMASK( m_nFlags, FLAG_CALL_EXPLODE_CALLBACK );
			}
		} else {
			if( pUnitAxis->y <= -0.707f ) {
				FMATH_SETBITMASK( m_nFlags, FLAG_CALL_EXPLODE_CALLBACK );
			}
		}
	}

	if( bPreserveVelocity ) {
		for( i=0; i<4; ++i ) {
			aDeltaPos_WS[i] = m_aTetraPos_WS[i];
		}
	}

	PointOnPlane_WS.Mul( *pUnitAxis, fPenetrationDist ).Add( *pPos_WS );
	ApplyDisplacementImpulse_Pos( pScaledWeights, pPos_WS, &PointOnPlane_WS );

	if( fFriction != 0.0f ) {
		f32 fDot;

		ComputeSurfaceStepDist_WS( &SurfaceVel_WS, pScaledWeights );
		fDot = SurfaceVel_WS.Dot( *pUnitAxis );
		DeltaVel_WS.Mul( *pUnitAxis, fDot );
		SurfaceVel_WS.Sub( DeltaVel_WS );

		// Save off the projection of the old tack position onto
		// the surface plane.  We can use this as a max clamp
		// pos if the magnitude of our friction calculations exceed
		// the position of the old point on the plane.
		CFVec3A OldPosPointOnPlane_WS;
		OldPosPointOnPlane_WS.Sub( PointOnPlane_WS, SurfaceVel_WS );

		f32 fPreFrictionModMag = SurfaceVel_WS.MagSq();
		SurfaceVel_WS.Mul( fFriction * (-0.03f - 0.12f*fPenetrationDist) );
		f32 fPostFrictionModMag = SurfaceVel_WS.MagSq();

		if( fPostFrictionModMag <= fPreFrictionModMag ) {
			// The friction calculation has not exceeded the old position
			// of the Tack.  Use this calculation.
			ApplyDisplacementImpulse_Vec( pScaledWeights, &SurfaceVel_WS );
		} else {
			// The friction calculation has exceeded the old position.  This
			// is probably due to a huge "warp-like" positional change over
			// a single frame.  Just clamp the calculation to the old position.
			//DEVPRINTF( "Clamping friction calculations!\n" );
			ApplyDisplacementImpulse_Pos( pScaledWeights, &PointOnPlane_WS, &OldPosPointOnPlane_WS );
		}
	}

	if( bPreserveVelocity ) {
		for( i=0; i<4; ++i ) {
			aDeltaPos_WS[i].RevSub( m_aTetraPos_WS[i] );
			m_aTetraOldPos_WS[i].Add( aDeltaPos_WS[i] );
		}
	}
}


void CFVerlet::CollideTacksWithPlane( const CFVec3A *pPointOnPlane_WS, const CFVec3A *pPlaneUnitNormal_WS, f32 fFriction, f32 fMinDistFromPlane, BOOL bPreserveVelocity ) {
	CFVerletTack *pTack;
	CFVec3A TackPos_WS, TackToPointOnPlane_WS;
	f32 fDot;
	u32 i;

	for( i=0, pTack=m_pTackArray; i<m_nTackCount; ++i, ++pTack ) {
		if( pTack->m_nFlags & CFVerletTack::FLAG_COLLISION_TACK ) {
			ComputeWSSurfacePointFromMS( &TackPos_WS, &pTack->m_Weights );

			TackToPointOnPlane_WS.Sub( *pPointOnPlane_WS, TackPos_WS );

			fDot = TackToPointOnPlane_WS.Dot( *pPlaneUnitNormal_WS ) + fMinDistFromPlane;
			if( fDot <= 0.0f ) {
				continue;
			}

			// Tack penetrates plane by a distance of fDot...

			_ApplyPlaneConstraint( &TackPos_WS, pPlaneUnitNormal_WS, &pTack->m_ScaledWeights, fDot, fFriction, bPreserveVelocity );
		}
	}
}


void CFVerlet::CollideTacksWithBox( const CFMtx43A *pUnitMtx, const CFVec3A *pMinCorner_WS, const CFVec3A *pMaxCorner_WS, f32 fFriction, BOOL bPreserveVelocity ) {
	CFVerletTack *pTack;
	CFVec3A TackPos_WS, TackToPointOnPlane_WS, UnitAxis_WS;
	f32 fDot;
	u32 i;

	for( i=0, pTack=m_pTackArray; i<m_nTackCount; ++i, ++pTack ) {
		if( pTack->m_nFlags & CFVerletTack::FLAG_COLLISION_TACK ) {
			ComputeWSSurfacePointFromMS( &TackPos_WS, &pTack->m_Weights );

			TackToPointOnPlane_WS.Sub( *pMinCorner_WS, TackPos_WS );

			fDot = TackToPointOnPlane_WS.Dot( pUnitMtx->m_vX );
			if( fDot > 0.0f ) {
				_ApplyPlaneConstraint( &TackPos_WS, &pUnitMtx->m_vX, &pTack->m_ScaledWeights, fDot, fFriction, bPreserveVelocity );
			}

			fDot = TackToPointOnPlane_WS.Dot( pUnitMtx->m_vY );
			if( fDot > 0.0f ) {
				_ApplyPlaneConstraint( &TackPos_WS, &pUnitMtx->m_vY, &pTack->m_ScaledWeights, fDot, fFriction, bPreserveVelocity );
			}

			fDot = TackToPointOnPlane_WS.Dot( pUnitMtx->m_vZ );
			if( fDot > 0.0f ) {
				_ApplyPlaneConstraint( &TackPos_WS, &pUnitMtx->m_vZ, &pTack->m_ScaledWeights, fDot, fFriction, bPreserveVelocity );
			}

			TackToPointOnPlane_WS.Sub( *pMaxCorner_WS, TackPos_WS );

			UnitAxis_WS.ReceiveNegative( pUnitMtx->m_vX );
			fDot = TackToPointOnPlane_WS.Dot( UnitAxis_WS );
			if( fDot > 0.0f ) {
				_ApplyPlaneConstraint( &TackPos_WS, &UnitAxis_WS, &pTack->m_ScaledWeights, fDot, fFriction, bPreserveVelocity );
			}

			UnitAxis_WS.ReceiveNegative( pUnitMtx->m_vY );
			fDot = TackToPointOnPlane_WS.Dot( UnitAxis_WS );
			if( fDot > 0.0f ) {
				_ApplyPlaneConstraint( &TackPos_WS, &UnitAxis_WS, &pTack->m_ScaledWeights, fDot, fFriction, bPreserveVelocity );
			}

			UnitAxis_WS.ReceiveNegative( pUnitMtx->m_vZ );
			fDot = TackToPointOnPlane_WS.Dot( UnitAxis_WS );
			if( fDot > 0.0f ) {
				_ApplyPlaneConstraint( &TackPos_WS, &UnitAxis_WS, &pTack->m_ScaledWeights, fDot, fFriction, bPreserveVelocity );
			}
		}
	}
}


// Computes the matrix based on the current positions of the tetrahedron points.
// The matrix is always unit in length.
void CFVerlet::ComputeMtx( void ) {
	m_UnitMtx.m_vPos.Sub( m_aTetraPos_WS[1], m_aTetraPos_WS[0] );

	m_UnitMtx.m_vZ.Sub( m_aTetraPos_WS[2], m_aTetraPos_WS[0] ).Unitize();
	m_UnitMtx.m_vY.UnitCross( m_UnitMtx.m_vZ, m_UnitMtx.m_vPos );
	m_UnitMtx.m_vX.Cross( m_UnitMtx.m_vY, m_UnitMtx.m_vZ );

	// Compute the real m_vPos...
	m_UnitMtx.m_vPos.Add( m_aTetraPos_WS[0], m_aTetraPos_WS[1] ).Add( m_aTetraPos_WS[2] ).Add( m_aTetraPos_WS[3] ).Mul( m_QuarterVec );

	if( m_nFlags & FLAG_HAS_NON_ZERO_COM ) {
		CFVec3A CenterOfMass_WS;

		m_UnitMtx.MulDir( CenterOfMass_WS, m_CenterOfMass_MS );
		m_UnitMtx.m_vPos.Sub( CenterOfMass_WS );
	}

	FMATH_CLEARBITMASK( m_nFlags, FLAG_INVMTX_BUILT );

	#if FANG_DEBUG_BUILD
	{
		CFVec3A TetraAxisX, TetraAxisY, TetraAxisZ, ZCrossX;
		f32 fDot;

		TetraAxisX.Sub( m_aTetraPos_WS[0], m_aTetraPos_WS[3] );
		TetraAxisY.Sub( m_aTetraPos_WS[1], m_aTetraPos_WS[3] );
		TetraAxisZ.Sub( m_aTetraPos_WS[2], m_aTetraPos_WS[3] );

		ZCrossX.Cross( TetraAxisZ, TetraAxisX );

		fDot = TetraAxisY.Dot( ZCrossX );
//		FASSERT( fDot > 0.0f );
	}
	#endif
}


// Stops linear and angular velocity.
void CFVerlet::StopMotion( void ) {
	FASSERT( IsCreated() );

	u32 i;

	for( i=0; i<4; ++i ) {
		m_aTetraOldPos_WS[i] = m_aTetraPos_WS[i];
	}
}


// Zeros the force vectors for the tetrahedron's vertices.
void CFVerlet::ClearForces( void ) {
	FASSERT( IsCreated() );

	m_aTetraForce_WS[0].Zero();
	m_aTetraForce_WS[1].Zero();
	m_aTetraForce_WS[2].Zero();
	m_aTetraForce_WS[3].Zero();
}


// Zeros the force vectors for the tetrahedron's vertices.
void CFVerlet::ClearImpulses( void ) {
	FASSERT( IsCreated() );

	m_aTetraImpulse_WS[0].Zero();
	m_aTetraImpulse_WS[1].Zero();
	m_aTetraImpulse_WS[2].Zero();
	m_aTetraImpulse_WS[3].Zero();
}


void CFVerlet::ApplyForce( const CFVec3A *pPos_WS, const CFVec3A *pForceVec_WS ) {
	FASSERT( IsCreated() );

	if( Net_IsFrozen() ) {
		// Frozen...
		return;
	}

	CFVec3A aTetraVertexDisplacementVec_WS[4], ScaledImpulseVec_WS;
	CFVec4 Weights;

	// Compute the weighted world space displacement vectors for the four tetrahedron vertices...
	ScaledImpulseVec_WS.Mul( *pForceVec_WS, m_fHundredOverMass );

	// Compute the weighted displacement vectors for each of the tetrahedron vertices...
	ComputeWeights_WS( &Weights, pPos_WS );
	_ComputeWeightedDisplacementVectors( aTetraVertexDisplacementVec_WS, &Weights, &ScaledImpulseVec_WS );

	// Apply the displacement vectors to the 4 tetrahedron vertex forces...
	m_aTetraForce_WS[0].Add( aTetraVertexDisplacementVec_WS[0] );
	m_aTetraForce_WS[1].Add( aTetraVertexDisplacementVec_WS[1] );
	m_aTetraForce_WS[2].Add( aTetraVertexDisplacementVec_WS[2] );
	m_aTetraForce_WS[3].Add( aTetraVertexDisplacementVec_WS[3] );

	Net_Kickstart();
}


void CFVerlet::ApplyForceToCOM( const CFVec3A *pImpulseVec_WS ) {
	FASSERT( IsCreated() );

	if( Net_IsFrozen() ) {
		// Frozen...
		return;
	}

	CFVec3A ScaledImpulseVec_WS;

	// Compute the weighted world space displacement vectors for the four tetrahedron vertices...
	ScaledImpulseVec_WS.Mul( *pImpulseVec_WS, m_fHundredOverMass ).Mul( m_QuarterVec );

	// Apply the displacement vectors to the 4 tetrahedron vertex forces...
	m_aTetraForce_WS[0].Add( ScaledImpulseVec_WS );
	m_aTetraForce_WS[1].Add( ScaledImpulseVec_WS );
	m_aTetraForce_WS[2].Add( ScaledImpulseVec_WS );
	m_aTetraForce_WS[3].Add( ScaledImpulseVec_WS );

	Net_Kickstart();
}


void CFVerlet::ApplyImpulse( const CFVec3A *pPos_WS, const CFVec3A *pImpulseVec_WS ) {
	FASSERT( IsCreated() );

	if( Net_IsFrozen() ) {
		// Frozen...
		return;
	}

	CFVec3A aTetraVertexDisplacementVec_WS[4], ScaledImpulseVec_WS;
	CFVec4 Weights;

	// Compute the weighted world space displacement vectors for the four tetrahedron vertices...
	ScaledImpulseVec_WS.Mul( *pImpulseVec_WS, m_fHundredOverMass );

	// Compute the weighted displacement vectors for each of the tetrahedron vertices...
	ComputeWeights_WS( &Weights, pPos_WS );
	_ComputeWeightedDisplacementVectors( aTetraVertexDisplacementVec_WS, &Weights, &ScaledImpulseVec_WS );

	// Apply the displacement vectors to the 4 tetrahedron vertex forces...
	m_aTetraImpulse_WS[0].Add( aTetraVertexDisplacementVec_WS[0] );
	m_aTetraImpulse_WS[1].Add( aTetraVertexDisplacementVec_WS[1] );
	m_aTetraImpulse_WS[2].Add( aTetraVertexDisplacementVec_WS[2] );
	m_aTetraImpulse_WS[3].Add( aTetraVertexDisplacementVec_WS[3] );

	Net_Kickstart();
}


void CFVerlet::ApplyImpulseToCOM( const CFVec3A *pImpulseVec_WS ) {
	FASSERT( IsCreated() );

	if( Net_IsFrozen() ) {
		// Frozen...
		return;
	}

	CFVec3A ScaledImpulseVec_WS;

	// Compute the weighted world space displacement vectors for the four tetrahedron vertices...
	ScaledImpulseVec_WS.Mul( *pImpulseVec_WS, m_fHundredOverMass ).Mul( m_QuarterVec );

	// Apply the displacement vectors to the 4 tetrahedron vertex forces...
	m_aTetraImpulse_WS[0].Add( ScaledImpulseVec_WS );
	m_aTetraImpulse_WS[1].Add( ScaledImpulseVec_WS );
	m_aTetraImpulse_WS[2].Add( ScaledImpulseVec_WS );
	m_aTetraImpulse_WS[3].Add( ScaledImpulseVec_WS );

	Net_Kickstart();
}


// Applies a displacement impulse at the application point pPos_WS, such that
// after the impulse the point will have moved to the position given by pTargetPos_WS.
void CFVerlet::ApplyDisplacementImpulse_Pos( const CFVec3A *pPos_WS, const CFVec3A *pTargetPos_WS ) {
	FASSERT( IsCreated() );

	if( Net_IsFrozen() ) {
		// Frozen...
		return;
	}

	CFVec3A aTetraVertexDisplacementVec_WS[4], DisplacementVec_WS;

	// Compute the displacement vector...
	DisplacementVec_WS.Sub( *pTargetPos_WS, *pPos_WS );

	// Compute the weighted world space displacement vectors for the four tetrahedron vertices...
	_ComputeWeightedDisplacementVectors( aTetraVertexDisplacementVec_WS, pPos_WS, &DisplacementVec_WS );

	// Apply the displacement vectors to the 4 tetrahedron vertices...
	m_aTetraPos_WS[0].Add( aTetraVertexDisplacementVec_WS[0] );
	m_aTetraPos_WS[1].Add( aTetraVertexDisplacementVec_WS[1] );
	m_aTetraPos_WS[2].Add( aTetraVertexDisplacementVec_WS[2] );
	m_aTetraPos_WS[3].Add( aTetraVertexDisplacementVec_WS[3] );

	Net_Kickstart();
}


// Same as ApplyDisplacementImpulse_Pos(), but accepts pre-computed scaled weights
// instead of computing them internally.
void CFVerlet::ApplyDisplacementImpulse_Pos( const CFVec4 *pScaledWeights, const CFVec3A *pPos_WS, const CFVec3A *pTargetPos_WS ) {
	FASSERT( IsCreated() );

	if( Net_IsFrozen() ) {
		// Frozen...
		return;
	}

	CFVec3A aTetraVertexDisplacementVec_WS[4], DisplacementVec_WS;

	// Compute the displacement vector...
	DisplacementVec_WS.Sub( *pTargetPos_WS, *pPos_WS );

	// Compute the weighted world space displacement vectors for the four tetrahedron vertices...
	_ComputeWeightedDisplacementVectors( aTetraVertexDisplacementVec_WS, pScaledWeights, &DisplacementVec_WS );

	// Apply the displacement vectors to the 4 tetrahedron vertices...
	m_aTetraPos_WS[0].Add( aTetraVertexDisplacementVec_WS[0] );
	m_aTetraPos_WS[1].Add( aTetraVertexDisplacementVec_WS[1] );
	m_aTetraPos_WS[2].Add( aTetraVertexDisplacementVec_WS[2] );
	m_aTetraPos_WS[3].Add( aTetraVertexDisplacementVec_WS[3] );

	Net_Kickstart();
}


// Applies a displacement impulse at the application point pPos_WS, such that
// after the impulse the point will have moved to the position given by
// pPos_WS + pDisplacementVec_WS.
void CFVerlet::ApplyDisplacementImpulse_Vec( const CFVec3A *pPos_WS, const CFVec3A *pDisplacementVec_WS ) {
	FASSERT( IsCreated() );

	if( Net_IsFrozen() ) {
		// Frozen...
		return;
	}

	CFVec3A aTetraVertexDisplacementVec_WS[4];

	// Compute the weighted model space displacement vectors for the four tetrahedron vertices...
	_ComputeWeightedDisplacementVectors( aTetraVertexDisplacementVec_WS, pPos_WS, pDisplacementVec_WS );

	// Apply the displacement vectors to the 4 tetrahedron vertices...
	m_aTetraPos_WS[0].Add( aTetraVertexDisplacementVec_WS[0] );
	m_aTetraPos_WS[1].Add( aTetraVertexDisplacementVec_WS[1] );
	m_aTetraPos_WS[2].Add( aTetraVertexDisplacementVec_WS[2] );
	m_aTetraPos_WS[3].Add( aTetraVertexDisplacementVec_WS[3] );

	Net_Kickstart();
}


// Same as ApplyDisplacementImpulse_Vec(), but accepts pre-computed scaled weights
// instead of computing them internally.
void CFVerlet::ApplyDisplacementImpulse_Vec( const CFVec4 *pScaledWeights, const CFVec3A *pDisplacementVec_WS ) {
	FASSERT( IsCreated() );

	if( Net_IsFrozen() ) {
		// Frozen...
		return;
	}

	CFVec3A aTetraVertexDisplacementVec_WS[4];

	// Compute the weighted model space displacement vectors for the four tetrahedron vertices...
	_ComputeWeightedDisplacementVectors( aTetraVertexDisplacementVec_WS, pScaledWeights, pDisplacementVec_WS );

	// Apply the displacement vectors to the 4 tetrahedron vertices...
	m_aTetraPos_WS[0].Add( aTetraVertexDisplacementVec_WS[0] );
	m_aTetraPos_WS[1].Add( aTetraVertexDisplacementVec_WS[1] );
	m_aTetraPos_WS[2].Add( aTetraVertexDisplacementVec_WS[2] );
	m_aTetraPos_WS[3].Add( aTetraVertexDisplacementVec_WS[3] );

	Net_Kickstart();
}


// Given the specified application point and displacement vector,
// this function fills in the 4-element array pTetraVertexDisplacementVec
// with the weighted displacements for the 4 tetrahedron vertices.
void CFVerlet::_ComputeWeightedDisplacementVectors( CFVec3A *pTetraVertexDisplacementVec, const CFVec3A *pPos_WS, const CFVec3A *pDisplacementVec ) {
	CFVec4 ScaledWeights;

	// Compute the weighted displacement vectors for each of the tetrahedron vertices...
	ComputeScaledWeights_WS( &ScaledWeights, pPos_WS );
	_ComputeWeightedDisplacementVectors( pTetraVertexDisplacementVec, &ScaledWeights, pDisplacementVec );
}


// Given the specified scale weights and displacement vector,
// this function fills in the 4-element array pTetraVertexDisplacementVec
// with the weighted displacements for the 4 tetrahedron vertices.
void CFVerlet::_ComputeWeightedDisplacementVectors( CFVec3A *pTetraVertexDisplacementVec, const CFVec4 *pScaledWeights, const CFVec3A *pDisplacementVec ) {
	pTetraVertexDisplacementVec[0].Mul( *pDisplacementVec, pScaledWeights->x );
	pTetraVertexDisplacementVec[1].Mul( *pDisplacementVec, pScaledWeights->y );
	pTetraVertexDisplacementVec[2].Mul( *pDisplacementVec, pScaledWeights->z );
	pTetraVertexDisplacementVec[3].Mul( *pDisplacementVec, pScaledWeights->w );
}


// Based on the given model space point, computes C1-C4.
void CFVerlet::ComputeWeights_MS( CFVec4 *pWeights, const CFVec3A *pPos_MS ) {
	FASSERT( IsCreated() );

	CFVec3A AdjustedPos_MS;

	if( m_nFlags & FLAG_HAS_NON_ZERO_COM ) {
		AdjustedPos_MS.Sub( *pPos_MS, m_CenterOfMass_MS );
		pPos_MS = &AdjustedPos_MS;
	}

	f32 fTerm1 = m_K5*pPos_MS->x;
	f32 fTerm2 = m_K6*pPos_MS->y;
	f32 fTerm3 = m_K4*pPos_MS->z;

	pWeights->x = (fTerm1 + fTerm2 + fTerm3 + m_K7) * m_K1;
	pWeights->y = (m_K8*pPos_MS->y + m_K9*pPos_MS->x + m_K10) * m_K2;
	pWeights->z = (fTerm1 + fTerm2 - fTerm3 + m_K7) * m_K1;
	pWeights->w = 0.25f + m_K3*pPos_MS->y;
}


// Based on the given world space point, computes C1-C4.
void CFVerlet::ComputeWeights_WS( CFVec4 *pWeights, const CFVec3A *pPos_WS ) {
	FASSERT( IsCreated() );

	CFVec3A Pos_MS;

	// Compute C1-C4...
	WS2MS_Point( &Pos_MS, pPos_WS );
	ComputeWeights_MS( pWeights, &Pos_MS );
}


// Based on the given model space point, computes C1-C4, all scaled by 1/lamda.
void CFVerlet::ComputeScaledWeights_MS( CFVec4 *pScaledWeights, const CFVec3A *pPos_MS ) {
	FASSERT( IsCreated() );

	// Compute C1-C4 and scale by 1/lamda...
	ComputeWeights_MS( pScaledWeights, pPos_MS );
	ScaleWeightsByInvLamda( pScaledWeights );
}


// Based on the given world space point, computes C1-C4, all scaled by 1/lamda.
void CFVerlet::ComputeScaledWeights_WS( CFVec4 *pScaledWeights, const CFVec3A *pPos_WS ) {
	FASSERT( IsCreated() );

	CFVec3A Pos_MS;

	// Compute C1-C4 and scale by 1/lamda...
	WS2MS_Point( &Pos_MS, pPos_WS );
	ComputeWeights_MS( pScaledWeights, &Pos_MS );
	ScaleWeightsByInvLamda( pScaledWeights );
}


void CFVerlet::ComputeWSOriginPointFromMS( CFVec3A *pPos_WS ) {
	FASSERT( IsCreated() );

	CFVec3A WeightedPos_WS;

	pPos_WS->Mul( m_aTetraPos_WS[0], m_QuarterVec );

	WeightedPos_WS.Mul( m_aTetraPos_WS[1], m_QuarterVec );
	pPos_WS->Add( WeightedPos_WS );

	WeightedPos_WS.Mul( m_aTetraPos_WS[2], m_QuarterVec );
	pPos_WS->Add( WeightedPos_WS );

	WeightedPos_WS.Mul( m_aTetraPos_WS[3], m_QuarterVec );
	pPos_WS->Add( WeightedPos_WS );
}


void CFVerlet::ComputeWSSurfacePointFromMS( CFVec3A *pPos_WS, const CFVec3A *pPos_MS ) {
	FASSERT( IsCreated() );

	CFVec3A WeightedPos_WS;
	CFVec4 Weights;

	ComputeWeights_MS( &Weights, pPos_MS );

	pPos_WS->Mul( m_aTetraPos_WS[0], Weights.x );

	WeightedPos_WS.Mul( m_aTetraPos_WS[1], Weights.y );
	pPos_WS->Add( WeightedPos_WS );

	WeightedPos_WS.Mul( m_aTetraPos_WS[2], Weights.z );
	pPos_WS->Add( WeightedPos_WS );

	WeightedPos_WS.Mul( m_aTetraPos_WS[3], Weights.w );
	pPos_WS->Add( WeightedPos_WS );
}


void CFVerlet::ComputeWSSurfacePointFromMS( CFVec3A *pPos_WS, const CFVec4 *pWeights ) {
	FASSERT( IsCreated() );

	CFVec3A WeightedPos_WS;

	pPos_WS->Mul( m_aTetraPos_WS[0], pWeights->x );

	WeightedPos_WS.Mul( m_aTetraPos_WS[1], pWeights->y );
	pPos_WS->Add( WeightedPos_WS );

	WeightedPos_WS.Mul( m_aTetraPos_WS[2], pWeights->z );
	pPos_WS->Add( WeightedPos_WS );

	WeightedPos_WS.Mul( m_aTetraPos_WS[3], pWeights->w );
	pPos_WS->Add( WeightedPos_WS );
}


// Computes the linear velocity at the origin of the object.
void CFVerlet::ComputeOriginVelocity_WS( CFVec3A *pLinearVelocity_WS, f32 fInvDeltaSecs ) {
	FASSERT( IsCreated() );

	ComputeOriginStepDist_WS( pLinearVelocity_WS );
	pLinearVelocity_WS->Mul( fInvDeltaSecs );
}


void CFVerlet::ComputeSurfaceVelocity_WS( CFVec3A *pSurfaceVelocity_WS, const CFVec3A *pPos_WS, f32 fInvDeltaSecs ) {
	FASSERT( IsCreated() );

	ComputeSurfaceStepDist_WS( pSurfaceVelocity_WS, pPos_WS );
	pSurfaceVelocity_WS->Mul( fInvDeltaSecs );
}


void CFVerlet::ComputeSurfaceVelocity_WS( CFVec3A *pSurfaceVelocity_WS, const CFVec4 *pWeights, f32 fInvDeltaSecs ) {
	FASSERT( IsCreated() );

	ComputeSurfaceStepDist_WS( pSurfaceVelocity_WS, pWeights );
	pSurfaceVelocity_WS->Mul( fInvDeltaSecs );
}


// Computes the step distance at the origin of the object.
void CFVerlet::ComputeOriginStepDist_WS( CFVec3A *pLinearStepDist_WS ) {
	FASSERT( IsCreated() );

	CFVec3A TetraVertexStep_WS;
	u32 i;

	pLinearStepDist_WS->Zero();

	for( i=0; i<4; ++i ) {
		TetraVertexStep_WS.Sub( m_aTetraPos_WS[i], m_aTetraOldPos_WS[i] ).Mul( m_QuarterVec );
		pLinearStepDist_WS->Add( TetraVertexStep_WS );
	}
}


void CFVerlet::ComputeSurfaceStepDist_WS( CFVec3A *pSurfaceStepDist_WS, const CFVec3A *pPos_WS ) {
	FASSERT( IsCreated() );

	CFVec3A TetraVertexStep_WS;
	CFVec4 Weights;
	u32 i;

	ComputeWeights_WS( &Weights, pPos_WS );

	pSurfaceStepDist_WS->Zero();

	for( i=0; i<4; ++i ) {
		TetraVertexStep_WS.Sub( m_aTetraPos_WS[i], m_aTetraOldPos_WS[i] ).Mul( Weights.a[i] );
		pSurfaceStepDist_WS->Add( TetraVertexStep_WS );
	}
}


void CFVerlet::ComputeSurfaceStepDist_WS( CFVec3A *pSurfaceStepDist_WS, const CFVec4 *pWeights ) {
	FASSERT( IsCreated() );

	CFVec3A TetraVertexStep_WS;
	u32 i;

	pSurfaceStepDist_WS->Zero();

	for( i=0; i<4; ++i ) {
		TetraVertexStep_WS.Sub( m_aTetraPos_WS[i], m_aTetraOldPos_WS[i] ).Mul( pWeights->a[i] );
		pSurfaceStepDist_WS->Add( TetraVertexStep_WS );
	}
}


// Applies a simple equality distance constraint.
void CFVerlet::Constraint_Distance( CFVec3A *pPos1_WS, CFVec3A *pPos2_WS, f32 fRestLength ) {
	CFVec3A DeltaVec_WS, CorrectionVec_WS;
	f32 fDeltaLength2, fOODeltaLength, fErrorLength;

	DeltaVec_WS.Sub( *pPos1_WS, *pPos2_WS );
	fDeltaLength2 = DeltaVec_WS.MagSq();
	if( fDeltaLength2 < 0.00001f ) {
		return;
	}

	fOODeltaLength = fmath_InvSqrt( fDeltaLength2 );

	fErrorLength = (fmath_Inv(fOODeltaLength) - fRestLength) * fOODeltaLength;

	CorrectionVec_WS.Mul( DeltaVec_WS, 0.5f * fErrorLength );

	pPos1_WS->Sub( CorrectionVec_WS );
	pPos2_WS->Add( CorrectionVec_WS );
}


// Applies a simple equality distance constraint with an immovable anchor.
void CFVerlet::Constraint_AnchorDistance( CFVec3A *pPos_WS, const CFVec3A *pAnchorPos_WS, f32 fRestLength ) {
	CFVec3A DeltaVec_WS, CorrectionVec_WS;
	f32 fDeltaLength2, fOODeltaLength, fErrorLength;

	DeltaVec_WS.Sub( *pPos_WS, *pAnchorPos_WS );
	fDeltaLength2 = DeltaVec_WS.MagSq();
	if( fDeltaLength2 < 0.00001f ) {
		return;
	}

	fOODeltaLength = fmath_InvSqrt( fDeltaLength2 );

	fErrorLength = (fmath_Inv(fOODeltaLength) - fRestLength) * fOODeltaLength;

	CorrectionVec_WS.Mul( DeltaVec_WS, fErrorLength );

	pPos_WS->Sub( CorrectionVec_WS );
}


// Applies a min/max distance range constraint.
void CFVerlet::Constraint_Range( CFVec3A *pPos1_WS, CFVec3A *pPos2_WS, f32 fMinRestLength, f32 fMaxRestLength ) {
	CFVec3A DeltaVec_WS, CorrectionVec_WS;
	f32 fDeltaLength2, fOODeltaLength, fErrorLength;

	DeltaVec_WS.Sub( *pPos1_WS, *pPos2_WS );
	fDeltaLength2 = DeltaVec_WS.MagSq();

	if( fDeltaLength2 < 0.00001f ) {
		return;
	}

	if( fDeltaLength2 > fMaxRestLength*fMaxRestLength ) {
		// Too long...

		fOODeltaLength = fmath_InvSqrt( fDeltaLength2 );
		fErrorLength = (fmath_Inv(fOODeltaLength) - fMaxRestLength) * fOODeltaLength;

		CorrectionVec_WS.Mul( DeltaVec_WS, 0.5f * fErrorLength );

		pPos1_WS->Sub( CorrectionVec_WS );
		pPos2_WS->Add( CorrectionVec_WS );

		return;
	}

	if( fDeltaLength2 < fMinRestLength*fMinRestLength ) {
		// Too short...

		fOODeltaLength = fmath_InvSqrt( fDeltaLength2 );
		fErrorLength = (fmath_Inv(fOODeltaLength) - fMinRestLength) * fOODeltaLength;

		CorrectionVec_WS.Mul( DeltaVec_WS, 0.5f * fErrorLength );

		pPos1_WS->Sub( CorrectionVec_WS );
		pPos2_WS->Add( CorrectionVec_WS );

		return;
	}
}


// Applies a min/max distance range constraint with an immovable anchor.
void CFVerlet::Constraint_AnchorRange( CFVec3A *pPos_WS, const CFVec3A *pAnchor, f32 fMinRestLength, f32 fMaxRestLength ) {
	CFVec3A DeltaVec_WS, CorrectionVec_WS;
	f32 fDeltaLength2, fOODeltaLength, fErrorLength;

	DeltaVec_WS.Sub( *pPos_WS, *pAnchor );
	fDeltaLength2 = DeltaVec_WS.MagSq();

	if( fDeltaLength2 < 0.00001f ) {
		return;
	}

	if( fDeltaLength2 > fMaxRestLength*fMaxRestLength ) {
		// Too long...

		fOODeltaLength = fmath_InvSqrt( fDeltaLength2 );
		fErrorLength = (fmath_Inv(fOODeltaLength) - fMaxRestLength) * fOODeltaLength;

		CorrectionVec_WS.Mul( DeltaVec_WS, fErrorLength );

		pPos_WS->Sub( CorrectionVec_WS );

		return;
	}

	if( fDeltaLength2 < fMinRestLength*fMinRestLength ) {
		// Too short...

		fOODeltaLength = fmath_InvSqrt( fDeltaLength2 );
		fErrorLength = (fmath_Inv(fOODeltaLength) - fMinRestLength) * fOODeltaLength;

		CorrectionVec_WS.Mul( DeltaVec_WS, fErrorLength );

		pPos_WS->Sub( CorrectionVec_WS );

		return;
	}
}


// Applies the 6 tetrahedron constraints one time.
void CFVerlet::_ApplyTetraConstraints( void ) {
	Constraint_Distance( &m_aTetraPos_WS[0], &m_aTetraPos_WS[1], m_fDim_v10_v12 );
	Constraint_Distance( &m_aTetraPos_WS[0], &m_aTetraPos_WS[2], m_TetraDim.z );
	Constraint_Distance( &m_aTetraPos_WS[0], &m_aTetraPos_WS[3], m_fDim_v30_v32 );
	Constraint_Distance( &m_aTetraPos_WS[1], &m_aTetraPos_WS[2], m_fDim_v10_v12 );
	Constraint_Distance( &m_aTetraPos_WS[1], &m_aTetraPos_WS[3], m_fDim_v31 );
	Constraint_Distance( &m_aTetraPos_WS[2], &m_aTetraPos_WS[3], m_fDim_v30_v32 );
}


void CFVerlet::TackConstraint_AnchorPoint( CFVerletTack *pTack, const CFVec3A *pAnchorPos_WS, f32 fMinConstraintDist, f32 fMaxConstraintDist ) {
	CFVec3A TackPos_WS;

	// Compute the location of the tack in world space...
	ComputeWSSurfacePointFromMS( &TackPos_WS, &pTack->m_Weights );

	if( fMinConstraintDist==0.0f && fMaxConstraintDist==0.0f ) {
		// Constrain tack directly to anchor point...

		// Snap the point to the anchor...
		ApplyDisplacementImpulse_Pos( &pTack->m_ScaledWeights, &TackPos_WS, pAnchorPos_WS );

		return;
	}

	if( fMinConstraintDist >= fMaxConstraintDist ) {
		// Constrain tack to a distance of fMinConstraintDist from anchor point...

		CFVec3A DeltaVec_WS, CorrectionVec_WS;
		f32 fDist, fErrorLength;

		DeltaVec_WS.Sub( TackPos_WS, *pAnchorPos_WS );
		fDist = DeltaVec_WS.Mag();
		if( fDist < 0.00001f ) {
			DeltaVec_WS.Set( 0.0f, 0.01f, 0.0f );
			fDist = 0.01f;
		}

		fErrorLength = (fMinConstraintDist - fDist) * fmath_Inv(fDist);

		CorrectionVec_WS.Mul( DeltaVec_WS, fErrorLength );
		ApplyDisplacementImpulse_Vec( &pTack->m_ScaledWeights, &CorrectionVec_WS );

		return;
	}

	// Constrain tack to a distance between fMinConstraintDist and fMaxConstraintDist from anchor point...

	CFVec3A DeltaVec_WS;
	f32 fDeltaLength2;

	DeltaVec_WS.Sub( TackPos_WS, *pAnchorPos_WS );
	fDeltaLength2 = DeltaVec_WS.MagSq();

	if( fDeltaLength2 < 0.00001f ) {
		DeltaVec_WS.Set( 0.0f, 0.01f, 0.0f );
		fDeltaLength2 = (0.01f * 0.01f);
	}

	if( fDeltaLength2 > fMaxConstraintDist*fMaxConstraintDist ) {
		// Too long...

		CFVec3A CorrectionVec_WS;
		f32 fOODeltaLength, fErrorLength;

		fOODeltaLength = fmath_InvSqrt( fDeltaLength2 );
		fErrorLength = (fMaxConstraintDist - fmath_Inv(fOODeltaLength)) * fOODeltaLength;

		CorrectionVec_WS.Mul( DeltaVec_WS, fErrorLength );
		ApplyDisplacementImpulse_Vec( &pTack->m_ScaledWeights, &CorrectionVec_WS );

		return;
	}

	if( fDeltaLength2 < fMinConstraintDist*fMinConstraintDist ) {
		// Too short...

		CFVec3A CorrectionVec_WS;
		f32 fOODeltaLength, fErrorLength;

		fOODeltaLength = fmath_InvSqrt( fDeltaLength2 );
		fErrorLength = (fMinConstraintDist - fmath_Inv(fOODeltaLength)) * fOODeltaLength;

		CorrectionVec_WS.Mul( DeltaVec_WS, fErrorLength );
		ApplyDisplacementImpulse_Vec( &pTack->m_ScaledWeights, &CorrectionVec_WS );

		return;
	}
}


void CFVerlet::TackConstraint_AnchorTack( CFVerletTack *pTack, CFVerletTack *pAnchorTack, f32 fMinConstraintDist, f32 fMaxConstraintDist ) {
	CFVec3A TackPos_WS, AnchorTackPos_WS;

	// Compute the location of the tacks in world space...
	ComputeWSSurfacePointFromMS( &TackPos_WS, &pTack->m_Weights );
	pAnchorTack->m_pTackOwner->ComputeWSSurfacePointFromMS( &AnchorTackPos_WS, &pAnchorTack->m_Weights );

	if( fMinConstraintDist >= fMaxConstraintDist ) {
		// Constrain tack to a distance of fMinConstraintDist from anchor tack...

		CFVec3A AnchorPos_WS, DeltaVec_WS, CorrectionVec_WS;
		f32 fDist, fErrorLength;

		// Compute a vector from the anchor tack to our tack...
		DeltaVec_WS.Sub( TackPos_WS, AnchorTackPos_WS );
		fDist = DeltaVec_WS.Mag();
		if( fDist < 0.00001f ) {
			DeltaVec_WS.Set( 0.0f, 0.01f, 0.0f );
			fDist = 0.01f;
		}

		// Compute error length...
		fErrorLength = (fDist - fMinConstraintDist) * fmath_Inv(fDist);

		// Compute correction vector and apply to our tack...
		CorrectionVec_WS.Mul( DeltaVec_WS, -0.5f * fErrorLength );
		ApplyDisplacementImpulse_Vec( &pTack->m_ScaledWeights, &CorrectionVec_WS );

		// Compute correction vector and apply to anchor tack...
		CorrectionVec_WS.Negate();
		pAnchorTack->m_pTackOwner->ApplyDisplacementImpulse_Vec( &pAnchorTack->m_ScaledWeights, &CorrectionVec_WS );

		return;
	}

	// Constrain tack to a distance between fMinConstraintDist and fMaxConstraintDist from anchor point...

	CFVec3A DeltaVec_WS;
	f32 fDeltaLength2;

	DeltaVec_WS.Sub( TackPos_WS, AnchorTackPos_WS );
	fDeltaLength2 = DeltaVec_WS.MagSq();

	if( fDeltaLength2 < 0.00001f ) {
		DeltaVec_WS.Set( 0.0f, 0.01f, 0.0f );
		fDeltaLength2 = (0.01f * 0.01f);
	}

	if( fDeltaLength2 > fMaxConstraintDist*fMaxConstraintDist ) {
		// Too long...

		CFVec3A CorrectionVec_WS;
		f32 fOODeltaLength, fErrorLength;

		fOODeltaLength = fmath_InvSqrt( fDeltaLength2 );
		fErrorLength = (fmath_Inv(fOODeltaLength) - fMaxConstraintDist) * fOODeltaLength;

		// Compute correction vector and apply to our tack...
		CorrectionVec_WS.Mul( DeltaVec_WS, -0.5f * fErrorLength );
		ApplyDisplacementImpulse_Vec( &pTack->m_ScaledWeights, &CorrectionVec_WS );

		// Compute correction vector and apply to anchor tack...
		CorrectionVec_WS.Negate();
		pAnchorTack->m_pTackOwner->ApplyDisplacementImpulse_Vec( &pAnchorTack->m_ScaledWeights, &CorrectionVec_WS );

		return;
	}

	if( fDeltaLength2 < fMinConstraintDist*fMinConstraintDist ) {
		// Too short...

		CFVec3A CorrectionVec_WS;
		f32 fOODeltaLength, fErrorLength;

		fOODeltaLength = fmath_InvSqrt( fDeltaLength2 );
		fErrorLength = (fmath_Inv(fOODeltaLength) - fMinConstraintDist) * fOODeltaLength;

		// Compute correction vector and apply to our tack...
		CorrectionVec_WS.Mul( DeltaVec_WS, -0.5f * fErrorLength );
		ApplyDisplacementImpulse_Vec( &pTack->m_ScaledWeights, &CorrectionVec_WS );

		// Compute correction vector and apply to anchor tack...
		CorrectionVec_WS.Negate();
		pAnchorTack->m_pTackOwner->ApplyDisplacementImpulse_Vec( &pAnchorTack->m_ScaledWeights, &CorrectionVec_WS );

		return;
	}
}


// Applies the tack constraints one time.
void CFVerlet::_ApplyTackConstraints( void ) {
	CFVerletTack *pTack;
	CFVec3A TackPos_WS;
	u32 i;

	for( i=0; i<m_nTackCount; ++i ) {
		pTack = &m_pTackArray[i];

		if( pTack->m_nFlags & CFVerletTack::FLAG_ANCHOR_ENABLED ) {
			switch( pTack->m_nAnchorType ) {
			case CFVerletTack::ANCHOR_TYPE_POINT_STATIC:
			case CFVerletTack::ANCHOR_TYPE_POINT_DYNAMIC:
				TackConstraint_AnchorPoint( pTack, pTack->m_pAnchorPos_WS, pTack->m_fMinDistFromTackToAnchor, pTack->m_fMaxDistFromTackToAnchor );
				break;

			case CFVerletTack::ANCHOR_TYPE_TACK:
				if( pTack->m_pAnchorTack->m_pTackOwner->m_nFlags & FLAG_ENABLE_WORK ) {
					TackConstraint_AnchorTack( pTack, pTack->m_pAnchorTack, pTack->m_fMinDistFromTackToAnchor, pTack->m_fMaxDistFromTackToAnchor );
				} else {
					// We're anchored to a tack with an owner whose work function is disabled...

					pTack->m_pAnchorTack->ComputePos_WS( &TackPos_WS );
					TackConstraint_AnchorPoint( pTack, &TackPos_WS, pTack->m_pAnchorTack->m_fMinDistFromTackToAnchor, pTack->m_pAnchorTack->m_fMaxDistFromTackToAnchor );
				}

				break;

			case CFVerletTack::ANCHOR_TYPE_POINT_BONED:
				if( pTack->m_nBonedAnchorComputedFrame != FVid_nFrameCounter ) {
					pTack->m_pBonedAnchorMtx->MulPoint( pTack->m_AnchorPos_WS, pTack->m_BonedAnchorPos_BS );
					pTack->m_nBonedAnchorComputedFrame = FVid_nFrameCounter;
				}

				TackConstraint_AnchorPoint( pTack, pTack->m_pAnchorPos_WS, pTack->m_fMinDistFromTackToAnchor, pTack->m_fMaxDistFromTackToAnchor );

				break;
			};
		}

		if( pTack->m_pTackAnchoredToMe ) {
			// We've got a tack anchored to us...

			if( !(pTack->m_pTackAnchoredToMe->m_pTackOwner->m_nFlags & FLAG_ENABLE_WORK) ) {
				// The tack that's anchored to us has its work disabled...

				pTack->m_pTackAnchoredToMe->ComputePos_WS( &TackPos_WS );
				TackConstraint_AnchorPoint( pTack, &TackPos_WS, pTack->m_pTackAnchoredToMe->m_fMinDistFromTackToAnchor, pTack->m_pTackAnchoredToMe->m_fMaxDistFromTackToAnchor );
			}
		}
	}
}


void CFVerlet::_UpdateHasDynamicAnchorFlag( void ) {
	u32 i;

	FMATH_CLEARBITMASK( m_nFlags, FLAG_HAS_DYNAMIC_ANCHOR );

	for( i=0; i<m_nTackCount; ++i ) {
		if( m_pTackArray[i].Anchor_GetType() == CFVerletTack::ANCHOR_TYPE_POINT_DYNAMIC ) {
			FMATH_SETBITMASK( m_nFlags, FLAG_HAS_DYNAMIC_ANCHOR );
			break;
		}
	}
}


void CFVerlet::_UpdateHasBonedAnchorFlag( void ) {
	u32 i;

	FMATH_CLEARBITMASK( m_nFlags, FLAG_HAS_BONED_ANCHOR );

	for( i=0; i<m_nTackCount; ++i ) {
		if( m_pTackArray[i].Anchor_GetType() == CFVerletTack::ANCHOR_TYPE_POINT_BONED ) {
			FMATH_SETBITMASK( m_nFlags, FLAG_HAS_BONED_ANCHOR );
			break;
		}
	}
}



void CFVerlet::SetObjectDim( const CFVec3A &vDim ) {
	CFVec3A ScaledTetraVtx0_MS, ScaledTetraVtx1_MS, ScaledTetraVtx3_MS;

	// Set the length of the tetrahedron side...
	m_TetraDim = vDim;

	FMATH_CLAMPMIN( m_TetraDim.x, 0.5f );
	FMATH_CLAMPMIN( m_TetraDim.y, 0.5f );
	FMATH_CLAMPMIN( m_TetraDim.z, 0.5f );

	// Compute tetrahedron edge lengths...
	ScaledTetraVtx0_MS.Set( (-1.0f/3.0f)*m_TetraDim.x, -0.25f*m_TetraDim.y, -0.5f*m_TetraDim.z );
	ScaledTetraVtx1_MS.Set( (2.0f/3.0f)*m_TetraDim.x, -0.25f*m_TetraDim.y, 0.0f );
	ScaledTetraVtx3_MS.Set( 0.0f,  0.75f*m_TetraDim.y, 0.0f );

	m_fDim_v10_v12 = fmath_Sqrt( m_TetraDim.x*m_TetraDim.x + 0.25f*m_TetraDim.z*m_TetraDim.z );
	m_fDim_v30_v32 = ScaledTetraVtx0_MS.Dist( ScaledTetraVtx3_MS );
	m_fDim_v31 = ScaledTetraVtx1_MS.Dist( ScaledTetraVtx3_MS );

	// Compute magic constants (thank you MathCAD)...
	m_K1 = 1.0f / (-12.0f * m_TetraDim.x * m_TetraDim.y * m_TetraDim.z);
	m_K2 = 1.0f / (12.0f * m_TetraDim.x * m_TetraDim.y);
	m_K3 = 1.0f / m_TetraDim.y;
	m_K4 = 12.0f * m_TetraDim.x * m_TetraDim.y;
	m_K5 = 6.0f * m_TetraDim.y * m_TetraDim.z;
	m_K6 = 4.0f * m_TetraDim.x * m_TetraDim.z;
	m_K7 = -3.0f * m_TetraDim.x * m_TetraDim.y * m_TetraDim.z;
	m_K8 = -4.0f * m_TetraDim.x;
	m_K9 = 12.0f * m_TetraDim.y;
	m_K10 = 3.0f * m_TetraDim.x * m_TetraDim.y;

	// Init current and old tetrahedron vertices from the initial matrix...
	_ComputeTetraVerticesFromMtx();
}



//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CFVerletTack
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

u32 CFVerletTack::m_nBoneTackPrefixLength;
u32 CFVerletTack::m_nBoneCTackPrefixLength;


// Creates the specified tack whose position is given in model space.
// The anchor is set to the tack position and disabled.
// The min and max anchor distances are set to 0.
// The magic constants to the tetrahedron are recomputed.
void CFVerletTack::Create_MS( CFVerlet *pTackOwner, const CFVec3A *pPos_MS, cchar *pszTackName, cchar *pszExplosionGroupName ) {
	FASSERT( !IsCreated() );

	m_pWire = NULL;

	_InitTack( pTackOwner, pPos_MS, pszTackName, pszExplosionGroupName );
}


// Creates the specified tack whose position is given in world space.
// The anchor is set to the tack position and disabled.
// The min and max anchor distances are set to 0.
// The magic constants to the tetrahedron are recomputed.
void CFVerletTack::Create_WS( CFVerlet *pTackOwner, const CFVec3A *pPos_WS, cchar *pszTackName, cchar *pszExplosionGroupName ) {
	FASSERT( !IsCreated() );

	CFVec3A TackPos_MS;

	m_pWire = NULL;

	pTackOwner->WS2MS_Point( &TackPos_MS, pPos_WS );
	_InitTack( pTackOwner, &TackPos_MS, pszTackName, pszExplosionGroupName );
}


// Initializes the specified tack whose position is given in model space.
// The anchor is set to the tack position and disabled.
// The min and max anchor distances are set to 0.
// The magic constants to the tetrahedron are recomputed.
void CFVerletTack::Init_MS( const CFVec3A *pPos_MS, cchar *pszTackName, cchar *pszExplosionGroupName ) {
	FASSERT( IsCreated() );

	Anchor_Remove();

	_InitTack( m_pTackOwner, pPos_MS, pszTackName, pszExplosionGroupName );
}


// Initializes the specified tack whose position is given in world space.
// The anchor is set to the tack position and disabled.
// The min and max anchor distances are set to 0.
// The magic constants to the tetrahedron are recomputed.
void CFVerletTack::Init_WS( const CFVec3A *pPos_WS, cchar *pszTackName, cchar *pszExplosionGroupName ) {
	FASSERT( IsCreated() );

	Anchor_Remove();

	CFVec3A TackPos_MS;

	m_pTackOwner->WS2MS_Point( &TackPos_MS, pPos_WS );
	_InitTack( m_pTackOwner, &TackPos_MS, pszTackName, pszExplosionGroupName );
}


// The string pointed to by pszTackName must persist.
void CFVerletTack::SetTackName( cchar *pszTackName ) {
	if( pszTackName == NULL ) {
		pszTackName = FVERLET_DUMMY_TACK_NAME;
	}

	m_pszName = pszTackName;
}


// Given the specified tack position in model space, this function
// repositions the tack and recomputes the magic constants to the tetrahedron.
//
// The anchor parameters are not modified.
void CFVerletTack::Reposition_MS( const CFVec3A *pPos_MS ) {
	FASSERT( IsCreated() );

	_RepositionTack( pPos_MS );

	m_pTackOwner->Net_Kickstart();
}


// Given the specified tack position in world space, this function
// repositions the tack and recomputes the magic constants to the tetrahedron.
//
// The anchor parameters are not modified.
void CFVerletTack::Reposition_WS( const CFVec3A *pPos_WS ) {
	FASSERT( IsCreated() );

	CFVec3A TackPos_MS;

	if( m_nBonedTackBoneIndex < 0 ) {
		m_pTackOwner->WS2MS_Point( &TackPos_MS, pPos_WS );
	} else {
		CFMtx43A::m_Temp.ReceiveAffineInverse( *m_pTackOwner->m_pWorldMesh->GetBoneMtxPalette()[ m_nBonedTackBoneIndex ], TRUE );
		CFMtx43A::m_Temp.MulPoint( TackPos_MS, *pPos_WS );
	}

	_RepositionTack( &TackPos_MS );

	m_pTackOwner->Net_Kickstart();
}


// Given the specified tack position in model space,
// this function initializes the specified tack.
// The anchor is set to the tack position and disabled.
// The anchor's owner is set to NULL.
// The min and max anchor distances are set to 0.
// The magic constants to the tetrahedron are recomputed.
void CFVerletTack::_InitTack( CFVerlet *pTackOwner, const CFVec3A *pPos_MS, cchar *pszTackName, cchar *pszExplosionGroupName ) {
	CFVec3A Pos_WS;

	if( m_pWire ) {
		CFWire::ReturnToFreePool( m_pWire );
		m_pWire = NULL;
	}

	pTackOwner->m_UnitMtx.MulPoint( Pos_WS, *pPos_MS );

	if( pszTackName == NULL ) {
		pszTackName = FVERLET_DUMMY_TACK_NAME;
	}

	m_pszName = pszTackName;
	m_pTackOwner = pTackOwner;
	m_pTackAnchoredToMe = NULL;
	m_nFlags = FLAG_POINT_CONSTRAINT;
	m_nAnchorType = ANCHOR_TYPE_POINT_STATIC;
	m_nWireGeoIndex = 255;

	m_AnchorPos_WS = Pos_WS;
	m_BonedAnchorPos_BS.Zero();
	m_nBonedAnchorComputedFrame = 0;
	m_pBonedAnchorMtx = NULL;
	m_pAnchorPos_WS = &m_AnchorPos_WS;
	m_fMinDistFromTackToAnchor = 0.0f;
	m_fMaxDistFromTackToAnchor = 0.0f;

	m_fAnchorUnitHealth = 1.0f;

	m_nBonedTackBoneIndex = -1;

	//set up the explosion group handle
	m_hExplosionGroup = FEXPLOSION_INVALID_HANDLE;
	if( pszExplosionGroupName ) {
		m_hExplosionGroup = fexplosion_GetExplosionGroup( pszExplosionGroupName );
	}

	_RepositionTack( pPos_MS );

	pTackOwner->_UpdateExplosionDetectionFlag();
}


// Given the specified tack position in model space,
// this function repositions the tack and recomputes
// the magic constants to the tetrahedron.
//
// The anchor parameters are not modified.
//
// If this is a boned tack, pPos_MS is in bone space.
//
// Set pPos_MS to NULL to recompute the weights while keeping
// the model or bone space position the same.
void CFVerletTack::_RepositionTack( const CFVec3A *pPos_MS ) {
	f32 fOOLamda;

	if( pPos_MS ) {
		m_TackPos_MS = *pPos_MS;
	}

	if( m_nBonedTackBoneIndex < 0 ) {
		// Non-boned tack...
		m_pTackOwner->ComputeWeights_MS( &m_Weights, &m_TackPos_MS );
	} else {
		// Boned tack...

		CFVec3A TackPos_MS;

		m_pTackOwner->_ComputeInvMtx();
		CFMtx43A::m_Temp.Mul( m_pTackOwner->m_InvUnitMtx, *m_pTackOwner->m_pWorldMesh->GetBoneMtxPalette()[ m_nBonedTackBoneIndex ] );
		CFMtx43A::m_Temp.MulPoint( TackPos_MS, m_TackPos_MS );

		m_pTackOwner->ComputeWeights_MS( &m_Weights, &TackPos_MS );
	}

	fOOLamda = m_pTackOwner->ComputeInvLamda( &m_Weights );
	m_ScaledWeights.x = m_Weights.x * fOOLamda;
	m_ScaledWeights.y = m_Weights.y * fOOLamda;
	m_ScaledWeights.z = m_Weights.z * fOOLamda;
	m_ScaledWeights.w = m_Weights.w * fOOLamda;
}


// Given the bone-to-bone transformation matrix,
// this function updates the bone space position
// of the tack and recomputes the magic constants
// to the tetrahedron.
//
// The anchor parameters are not modified.
void CFVerletTack::_UpdateBonedTack( const CFMtx43A *pBoneToBoneMtx ) {
	FASSERT( m_nBonedTackBoneIndex >= 0 );

	CFVec3A Pos_BS;
	f32 fOOLamda;

	pBoneToBoneMtx->MulPoint( Pos_BS, m_TackPos_MS );
	m_pTackOwner->ComputeWeights_MS( &m_Weights, &Pos_BS );

	fOOLamda = m_pTackOwner->ComputeInvLamda( &m_Weights );
	m_ScaledWeights.x = m_Weights.x * fOOLamda;
	m_ScaledWeights.y = m_Weights.y * fOOLamda;
	m_ScaledWeights.z = m_Weights.z * fOOLamda;
	m_ScaledWeights.w = m_Weights.w * fOOLamda;
}


// Returns TRUE if the specified string is prefixed with FVERLET_MESH_BONE_TACK_PREFIX.
BOOL CFVerletTack::IsStringPrefixedWithTackName( cchar *pszName ) {
	FASSERT( CFVerlet::IsModuleInitialized() );

	if( fclib_strlen( pszName ) <= (s32)m_nBoneTackPrefixLength ) {
		// Bone name is too small to have the tack prefix...
		return FALSE;
	}

	if( fclib_strnicmp( pszName, FVERLET_MESH_BONE_TACK_PREFIX, m_nBoneTackPrefixLength ) ) {
		// Bone name doesn't have the tack prefix...
		return FALSE;
	}

	return TRUE;
}


// Returns TRUE if the specified string is prefixed with FVERLET_MESH_BONE_CTACK_PREFIX.
BOOL CFVerletTack::IsStringPrefixedWithCTackName( cchar *pszName ) {
	FASSERT( CFVerlet::IsModuleInitialized() );

	if( fclib_strlen( pszName ) <= (s32)m_nBoneCTackPrefixLength ) {
		// Bone name is too small to have the tack prefix...
		return FALSE;
	}

	if( fclib_strnicmp( pszName, FVERLET_MESH_BONE_CTACK_PREFIX, m_nBoneCTackPrefixLength ) ) {
		// Bone name doesn't have the tack prefix...
		return FALSE;
	}

	return TRUE;
}


// Returns TRUE if the specified bone is a tack bone.
// Otherwise, returns FALSE.
//
// If ppszTackName is not NULL, it is filled with a pointer to the
// tack name.
//
// If pbCTack is not NULL, it is filled with TRUE if the bone is a CTack
// (collision tack), or FALSE if the bone is not a CTack.
BOOL CFVerletTack::IsTackBone( FMeshBone_t *pBone, cchar **ppszTackName, BOOL *pbCTack ) {
	if( CFVerlet::m_apszTackNameArray ) {
		u32 i;

		for( i=0; CFVerlet::m_apszTackNameArray[i]; ++i ) {
			if( !fclib_stricmp( pBone->szName, CFVerlet::m_apszTackNameArray[i] ) ) {
				if( pbCTack ) {
					*pbCTack = FALSE;
				}

				if( ppszTackName ) {
					*ppszTackName = pBone->szName;
				}

				return TRUE;
			}
		}
	}

	if( CFVerlet::m_apszCTackNameArray ) {
		u32 i;

		for( i=0; CFVerlet::m_apszCTackNameArray[i]; ++i ) {
			if( !fclib_stricmp( pBone->szName, CFVerlet::m_apszCTackNameArray[i] ) ) {
				if( pbCTack ) {
					*pbCTack = TRUE;
				}

				if( ppszTackName ) {
					*ppszTackName = pBone->szName;
				}

				return TRUE;
			}
		}
	}

	if( IsStringPrefixedWithTackName( pBone->szName ) ) {
		if( pbCTack ) {
			*pbCTack = FALSE;
		}

		if( ppszTackName ) {
			*ppszTackName = pBone->szName + m_nBoneTackPrefixLength;
		}

		return TRUE;
	}

	if( IsStringPrefixedWithCTackName( pBone->szName ) ) {
		if( pbCTack ) {
			*pbCTack = TRUE;
		}

		if( ppszTackName ) {
			*ppszTackName = pBone->szName + m_nBoneCTackPrefixLength;
		}

		return TRUE;
	}

	if( pbCTack ) {
		*pbCTack = FALSE;
	}

	if( ppszTackName ) {
		*ppszTackName = pBone->szName;
	}

	return FALSE;
}


// Returns the number of tacks defined in the mesh.
u32 CFVerletTack::DetermineTackCountFromMesh( const FMesh_t *pMesh ) {
	u32 i, nTackCount;
	FMeshBone_t *pBone;

	nTackCount = 0;

	for( i=0, pBone=pMesh->pBoneArray; i<pMesh->nBoneCount; ++i, ++pBone ) {
		if( IsTackBone( pBone ) ) {
			++nTackCount;
		}
	}

	return nTackCount;
}


void CFVerletTack::EnableCollisionTack( BOOL bCollisionTack ) {
	FASSERT( IsCreated() );

	if( bCollisionTack ) {
		if( !IsCollisionTack() ) {
			FMATH_SETBITMASK( m_nFlags, FLAG_COLLISION_TACK );

			FASSERT( m_pTackOwner->m_nCollTackCount < 255 );
			++m_pTackOwner->m_nCollTackCount;
		}
	} else {
		if( IsCollisionTack() ) {
			FMATH_CLEARBITMASK( m_nFlags, FLAG_COLLISION_TACK );

			FASSERT( m_pTackOwner->m_nCollTackCount > 0 );
			--m_pTackOwner->m_nCollTackCount;
		}
	}
}


void CFVerletTack::Anchor_Enable( BOOL bEnable ) {
	FASSERT( IsCreated() );

	if( m_nFlags & FLAG_ANCHOR_ENABLED ) {
		if( !bEnable ) {
			// Disable this anchor...

			FMATH_CLEARBITMASK( m_nFlags, FLAG_ANCHOR_ENABLED );

			if( m_nAnchorType == ANCHOR_TYPE_TACK ) {
				CFVerlet::_Net_Sever( m_pTackOwner, m_pAnchorTack->m_pTackOwner );
			} else {
				m_pTackOwner->Net_Kickstart();
			}

			m_pTackOwner->_UpdateExplosionDetectionFlag();
		}
	} else {
		if( bEnable ) {
			// Enable this anchor...

			FMATH_SETBITMASK( m_nFlags, FLAG_ANCHOR_ENABLED );

			if( m_nAnchorType == ANCHOR_TYPE_TACK ) {
				CFVerlet::_Net_Merge( m_pTackOwner->m_pNetMaster, m_pAnchorTack->m_pTackOwner->m_pNetMaster );
			} else {
				m_pTackOwner->Net_Kickstart();
			}

			m_pTackOwner->_UpdateExplosionDetectionFlag();
		}
	}
}


void CFVerletTack::Anchor_Remove( void ) {
	FASSERT( IsCreated() );

	CFVerlet *pAnchorTackOwner;

	switch( m_nAnchorType ) {
	case ANCHOR_TYPE_POINT_STATIC:
		// Kill our anchor...

		m_nAnchorType = ANCHOR_TYPE_NONE;
		m_pAnchorPos_WS = NULL;

		m_pTackOwner->Net_Kickstart();

		break;

	case ANCHOR_TYPE_POINT_DYNAMIC:
		// Kill our anchor...

		m_nAnchorType = ANCHOR_TYPE_NONE;
		m_pAnchorPos_WS = NULL;

		m_pTackOwner->_UpdateHasDynamicAnchorFlag();
		m_pTackOwner->Net_Kickstart();

		break;

	case ANCHOR_TYPE_POINT_BONED:
		// Kill our anchor...

		m_nAnchorType = ANCHOR_TYPE_NONE;
		m_pAnchorPos_WS = NULL;
		m_BonedAnchorPos_BS.Zero();
		m_nBonedAnchorComputedFrame = 0;
		m_pBonedAnchorMtx = NULL;

		m_pTackOwner->_UpdateHasBonedAnchorFlag();
		m_pTackOwner->Net_Kickstart();

		break;

	case ANCHOR_TYPE_TACK:
		// Tell the tack we're anchored to, that nobody is anchored to him...
		FASSERT( m_pAnchorTack );
		FASSERT( m_pAnchorTack->m_pTackAnchoredToMe  );
		FASSERT( m_pTackOwner->m_pNetMaster == m_pAnchorTack->m_pTackOwner->m_pNetMaster );

		pAnchorTackOwner = m_pAnchorTack->m_pTackOwner;

		// Kill our anchor...
		m_pAnchorTack->m_pTackAnchoredToMe = NULL;
		m_nAnchorType = ANCHOR_TYPE_NONE;
		m_pAnchorTack = NULL;

		CFVerlet::_Net_Sever( m_pTackOwner, pAnchorTackOwner );

		break;
	};

	m_pTackOwner->_UpdateExplosionDetectionFlag();
}


// If pPos_WS is NULL, the anchor is fixed to the tack's current position.
void CFVerletTack::Anchor_SetToImmovablePoint_Static( const CFVec3A *pPos_WS ) {
	FASSERT( IsCreated() );

	CFVec3A TackPos_WS;

	Anchor_Remove();

	if( pPos_WS == NULL ) {
		ComputePos_WS( &TackPos_WS );
		pPos_WS = &TackPos_WS;
	}

	m_nAnchorType = ANCHOR_TYPE_POINT_STATIC;
	m_AnchorPos_WS = *pPos_WS;
	m_pAnchorPos_WS = &m_AnchorPos_WS;

	m_pTackOwner->_UpdateExplosionDetectionFlag();
	m_pTackOwner->Net_Kickstart();
}


void CFVerletTack::Anchor_SetToImmovablePoint_Dynamic( const CFVec3A *pPos_WS ) {
	FASSERT( IsCreated() );

	Anchor_Remove();

	m_nAnchorType = ANCHOR_TYPE_POINT_DYNAMIC;
	m_pAnchorPos_WS = pPos_WS;

	FMATH_SETBITMASK( m_pTackOwner->m_nFlags, CFVerlet::FLAG_HAS_DYNAMIC_ANCHOR );

	m_pTackOwner->_UpdateExplosionDetectionFlag();
	m_pTackOwner->Net_Kickstart();
}


// The tack will be attached to the parent of the bone specified by nBoneIndex.
//
// The nBoneIndex bone defines where the tack is located in the at-rest model.
// However, as an optimization, that bone does not need to be animated and can be
// masked out in the animation system. As the parent bone is animated, the Verlet
// system updates the orientation of the tack automatically and only when
// necessary. In other words, the at-rest relationship between the nBoneIndex bone
// and its parent is maintained by the Verlet system, even as the parent moves.
//
// If the nBoneIndex bone has no parent, the world mesh's Xfm is used as the parent.
void CFVerletTack::Anchor_SetToImmovablePoint_Boned( const CFWorldMesh *pWorldMesh, u32 nBoneIndex ) {
	const CFMtx43A *pParentMtx;

	u8 nParentBoneIndex = pWorldMesh->m_pMesh->pBoneArray[nBoneIndex].Skeleton.nParentBoneIndex;

	if( nParentBoneIndex != 255 ) {
		// Bone has a parent...
		pParentMtx = pWorldMesh->GetBoneMtxPalette()[nParentBoneIndex];
	} else {
		// Bone has no parent. Use world mesh's Xfm...
		pParentMtx = &pWorldMesh->m_Xfm.m_MtxF;
	}

	Anchor_SetToImmovablePoint_Boned_BS( pParentMtx, &pWorldMesh->m_pMesh->pBoneArray[nBoneIndex].AtRestBoneToParentMtx.m_vPos );
}


void CFVerletTack::Anchor_SetToImmovablePoint_Boned_WS( const CFMtx43A *pBoneMtx, const CFVec3A *pPos_WS ) {
	FASSERT( IsCreated() );

	CFMtx43A InvBoneMtx;
	CFVec3A Pos_BS;

	InvBoneMtx.ReceiveAffineInverse( *pBoneMtx, TRUE );
	InvBoneMtx.MulPoint( Pos_BS, *pPos_WS );

	Anchor_SetToImmovablePoint_Boned_BS( pBoneMtx, &Pos_BS );
}


void CFVerletTack::Anchor_SetToImmovablePoint_Boned_BS( const CFMtx43A *pBoneMtx, const CFVec3A *pPos_BS ) {
	FASSERT( IsCreated() );

	Anchor_Remove();

	m_nAnchorType = ANCHOR_TYPE_POINT_BONED;
	m_nBonedAnchorComputedFrame = FVid_nFrameCounter;
	m_pAnchorPos_WS = &m_AnchorPos_WS;
	m_BonedAnchorPos_BS = *pPos_BS;
	m_pBonedAnchorMtx = pBoneMtx;

	pBoneMtx->MulPoint( m_AnchorPos_WS, m_BonedAnchorPos_BS );

	FMATH_SETBITMASK( m_pTackOwner->m_nFlags, CFVerlet::FLAG_HAS_BONED_ANCHOR );

	m_pTackOwner->_UpdateExplosionDetectionFlag();
	m_pTackOwner->Net_Kickstart();
}


void CFVerletTack::Anchor_SetToTack( CFVerletTack *pAnchorToThisTack ) {
	FASSERT( IsCreated() );
	FASSERT( pAnchorToThisTack != this );
	FASSERT( pAnchorToThisTack->m_pTackOwner );

	if( pAnchorToThisTack->m_pTackAnchoredToMe ) {
		// Another tack is already anchored to pAnchorToThisTack...
		pAnchorToThisTack->m_pTackAnchoredToMe->Anchor_Remove();
	}

	FASSERT( pAnchorToThisTack->m_pTackAnchoredToMe == NULL );

	// Remove current anchor, if any...
	Anchor_Remove();

	// Set our anchor to point to the other tack...
	m_nAnchorType = ANCHOR_TYPE_TACK;
	m_pAnchorTack = pAnchorToThisTack;

	// Tell the other tack that it has a tack anchored to it...
	pAnchorToThisTack->m_pTackAnchoredToMe = this;

	// Merge the two nets...
	m_pTackOwner->_UpdateExplosionDetectionFlag();
	CFVerlet::_Net_Merge( m_pTackOwner->m_pNetMaster, pAnchorToThisTack->m_pTackOwner->m_pNetMaster );
}


void CFVerletTack::Anchor_SetRange( f32 fMinConstraintDist, f32 fMaxConstraintDist ) {
	FASSERT( IsCreated() );

	FMATH_CLAMPMIN( fMaxConstraintDist, fMinConstraintDist );

	m_fMinDistFromTackToAnchor = fMinConstraintDist;
	m_fMaxDistFromTackToAnchor = fMaxConstraintDist;

	m_pTackOwner->Net_Kickstart();
}


// Set bSilent to TRUE if no effects are to be generated if the health falls to 0.
void CFVerletTack::Anchor_SetUnitHealth( f32 fNewUnitHealth, BOOL bSilent ) {
	FASSERT( IsCreated() );

	f32 fOldUnitHealth = m_fAnchorUnitHealth;

	FMATH_CLAMP_UNIT_FLOAT( fNewUnitHealth );
	m_fAnchorUnitHealth = fNewUnitHealth;

	if( m_fAnchorUnitHealth == 0.0f ) {
		if( fOldUnitHealth > 0.0f ) {
			// We just reached zero health...

			if( !bSilent ) {
				if( m_hExplosionGroup != FEXPLOSION_INVALID_HANDLE ) { 
					// We just reached zero health and we have a valid explosion group
					// associated with this tack, so spawn an explosion at the tack world
					// space position...

					FExplosion_SpawnerHandle_t hSpawner = fexplosion_GetExplosionSpawner();

					if( hSpawner != FEXPLOSION_INVALID_HANDLE ) {
						CFVec3A SpawnPos_WS;
						FExplosionSpawnParams_t SpawnParams;

						ComputePos_WS( &SpawnPos_WS );
						SpawnParams.InitToDefaults();

						SpawnParams.uFlags = FEXPLOSION_SPAWN_NONE;
						SpawnParams.Pos_WS = SpawnPos_WS;
						SpawnParams.uSurfaceType = 0;
						SpawnParams.pDamageProfile = NULL;
						SpawnParams.pDamager = NULL;

						fexplosion_SpawnExplosion( hSpawner, m_hExplosionGroup, &SpawnParams );
					}
				}

				CFWire::Shatter( m_pWire );
				m_pWire = NULL;
			}
		}

		Anchor_Enable( FALSE );

		CFWire::ReturnToFreePool( m_pWire );
		m_pWire = NULL;
	}
}


void CFVerletTack::Anchor_DrawWire( u8 nWireGeoIndex ) {
	FASSERT( IsCreated() );

	if( nWireGeoIndex == m_nWireGeoIndex ) {
		return;
	}

	m_nWireGeoIndex = nWireGeoIndex;

	CFWire::ReturnToFreePool( m_pWire );
	m_pWire = NULL;

	_UpdateTackWire();
}


void CFVerletTack::Anchor_EnableWireCollision( BOOL bEnableCollision ) {
	FASSERT( IsCreated() );

	FMATH_WRITEBIT( bEnableCollision, m_nFlags, FLAG_WIRE_GEO_COLLIDABLE );

	if( m_pWire ) {
		m_pWire->EnableCollision( bEnableCollision );
	}
}


BOOL CFVerletTack::_CheckPointSave( void ) {
	CFCheckPoint::SaveData( m_TackPos_MS );
	CFCheckPoint::SaveData( m_AnchorPos_WS );
	CFCheckPoint::SaveData( m_BonedAnchorPos_BS );
	CFCheckPoint::SaveData( m_nBonedAnchorComputedFrame );
	CFCheckPoint::SaveData( (u32&) m_pBonedAnchorMtx );
	CFCheckPoint::SaveData( (u32&) m_pAnchorPos_WS ); //this will also save off m_pAnchorTack as they are union'ed
	CFCheckPoint::SaveData( m_fMinDistFromTackToAnchor );
	CFCheckPoint::SaveData( m_fMaxDistFromTackToAnchor );
	CFCheckPoint::SaveData( (void *) &m_Weights, sizeof( CFVec4 ) ); //I may be able to skip saving this, as it's precomputed...
	CFCheckPoint::SaveData( (void *) &m_ScaledWeights, sizeof( CFVec4 ) ); //I may be able to skip this one too...
	CFCheckPoint::SaveData( (u32&)m_pTackAnchoredToMe );
	CFCheckPoint::SaveData( m_fAnchorUnitHealth ); //GOTTA SAVE THIS ONE... :)

	// These next four items will be packed into a u32 for memory savings...
	u32 uPackedData;
	uPackedData  = ( m_nFlags << 24 );
	uPackedData |= ( m_nAnchorType << 16 );
	uPackedData |= ( (u8)m_nBonedTackBoneIndex << 8 );
	uPackedData |= ( m_nWireGeoIndex );
	CFCheckPoint::SaveData( uPackedData );

	CFCheckPoint::SaveData( (u32&) m_hExplosionGroup );

	return TRUE;
}


BOOL CFVerletTack::_CheckPointLoad( void ) {
	CFCheckPoint::LoadData( m_TackPos_MS );
	CFCheckPoint::LoadData( m_AnchorPos_WS );
	CFCheckPoint::LoadData( m_BonedAnchorPos_BS );
	CFCheckPoint::LoadData( m_nBonedAnchorComputedFrame );
	CFCheckPoint::LoadData( (u32&) m_pBonedAnchorMtx );
	CFCheckPoint::LoadData( (u32&) m_pAnchorPos_WS ); //this will also save off m_pAnchorTack as they are union'ed
	CFCheckPoint::LoadData( m_fMinDistFromTackToAnchor );
	CFCheckPoint::LoadData( m_fMaxDistFromTackToAnchor );
	CFCheckPoint::LoadData( (void *) &m_Weights, sizeof( CFVec4 ) ); //I may be able to skip saving this, as it's precomputed...
	CFCheckPoint::LoadData( (void *) &m_ScaledWeights, sizeof( CFVec4 ) ); //I may be able to skip this one too...
	CFCheckPoint::LoadData( (u32&)m_pTackAnchoredToMe );
	CFCheckPoint::LoadData( m_fAnchorUnitHealth ); //GOTTA SAVE THIS ONE... :)

	// These next four items will be unpacked from a u32...
	u32 uPackedData;
	CFCheckPoint::LoadData( uPackedData );
	m_nFlags =              ( ( uPackedData >> 24 ) & 0xff );
	m_nAnchorType =         ( ( uPackedData >> 16 ) & 0xff );
	m_nBonedTackBoneIndex = (u8)( ( uPackedData >>  8 ) & 0xff );
	m_nWireGeoIndex = (u8)( uPackedData & 0xff );

	CFCheckPoint::LoadData( (u32&) m_hExplosionGroup );

	return TRUE;
}


void CFVerletTack::_UpdateTackWire( void ) {
	if( (m_nWireGeoIndex == 255)
		|| !Anchor_IsEnabled()
		|| (m_nAnchorType == ANCHOR_TYPE_NONE)
		|| (m_pTackOwner->m_pWorldMesh==NULL)
		|| !m_pTackOwner->m_pWorldMesh->IsAddedToWorld() )
	{
		if( m_pWire ) {
			CFWire::ReturnToFreePool( m_pWire );
			m_pWire = NULL;
		}

		return;
	}

	if( m_pWire == NULL ) {
		m_pWire = CFWire::GetFromFreePool( Anchor_IsWireCollidable(), Anchor_GetWireGeoIndex(), this );

		if( m_pWire == NULL ) {
			return;
		}
	}

	CFVec3A AnchorStartPos_WS;

	ComputePos_WS( &AnchorStartPos_WS );

	if( m_nAnchorType != CFVerletTack::ANCHOR_TYPE_TACK ) {
		if( m_pWire->UpdateEndPoints( &AnchorStartPos_WS, m_pAnchorPos_WS, TRUE ) ) {
			// Wire returned...
			m_pWire = NULL;
		}
	} else {
		CFVec3A AnchorEndPos_WS;

		if( m_pAnchorTack->m_pTackOwner->m_nFlags & CFVerlet::FLAG_ENABLE_WORK ) {
			m_pAnchorTack->m_pTackOwner->ComputeWSSurfacePointFromMS( &AnchorEndPos_WS, &m_pAnchorTack->m_Weights );
		} else {
			m_pAnchorTack->ComputePos_WS( &AnchorEndPos_WS );
		}

		if( m_pWire->UpdateEndPoints( &AnchorStartPos_WS, &AnchorEndPos_WS, TRUE ) ) {
			// Wire returned...
			m_pWire = NULL;
		}
	}
}





//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CFVerletTackSurface
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

void CFVerletTackSurface::Enable( BOOL bEnable ) {
	FASSERT( IsCreated() );

	if( bEnable ) {
		FMATH_SETBITMASK( m_nFlags, FLAG_ENABLED );
	} else {
		FMATH_CLEARBITMASK( m_nFlags, FLAG_ENABLED );
	}
}


// Inits a plane surface and disables it.
// Friction and distance are set to 0.
void CFVerletTackSurface::Init_Plane_Static( const CFVec3A *pPointOnPlane_WS, const CFVec3A *pUnitNormal_WS ) {
	m_fFriction = 0.0f;
	m_fPlaneDist = 0.0f;
	m_nType = TYPE_PLANE;
	m_nAnchorType = ANCHOR_TYPE_STATIC;
	m_nFlags = FLAG_NONE;
	m_nPlaneAxisIndex = PLANE_NORMAL_Z;

	m_pUnitMtx = &m_UnitMtx;

	m_UnitMtx.m_vX.Zero();
	m_UnitMtx.m_vY.Zero();
	m_UnitMtx.m_vZ = *pUnitNormal_WS;
	m_UnitMtx.m_vPos = *pPointOnPlane_WS;

	m_nUnitMtxComputedFrame = 0;
	m_pNonUnitMtx = NULL;
	m_Mtx_BS.Zero();
	m_BoxHalfDim_MS.Zero();
}


// Inits a plane surface and disables it.
// Friction and distance are set to 0.
//
// The surface will be attached directly to the provided matrix, pMtx.
void CFVerletTackSurface::Init_Plane_Dynamic( const CFMtx43A *pMtx, BOOL bMtxMayBeNonUnit, PlaneNormal_e nPlaneNormal ) {
	FASSERT( nPlaneNormal>=0 && nPlaneNormal<PLANE_NORMAL_COUNT );

	m_fFriction = 0.0f;
	m_fPlaneDist = 0.0f;
	m_nType = TYPE_PLANE;
	m_nAnchorType = ANCHOR_TYPE_DYNAMIC;
	m_nFlags = FLAG_NONE;

	if( nPlaneNormal < PLANE_NORMAL_NX ) {
		m_nPlaneAxisIndex = nPlaneNormal;
	} else {
		FMATH_SETBITMASK( m_nFlags, FLAG_PLANE_NEGATE_NORMAL );
		m_nPlaneAxisIndex = nPlaneNormal - PLANE_NORMAL_NX;
	}

	if( bMtxMayBeNonUnit ) {
		FMATH_SETBITMASK( m_nFlags, FLAG_MUST_UNITIZE_MTX );

		m_nUnitMtxComputedFrame = FVid_nFrameCounter-1;
		m_pUnitMtx = &m_UnitMtx;
		m_pNonUnitMtx = pMtx;
	} else {
		m_nUnitMtxComputedFrame = 0;
		m_pUnitMtx = pMtx;
		m_pNonUnitMtx = NULL;
	}

	m_Mtx_BS.Zero();
	m_BoxHalfDim_MS.Zero();
}


// Initializes a plane surface and disables it.
// Friction and distance are set to 0.
//
// The surface will be attached to the parent of the bone specified by nBoneIndex.
//
// The nBoneIndex bone defines where the surface is located in the at-rest model.
// However, as an optimization, that bone does not need to be animated and can be
// masked out in the animation system. As the parent bone is animated, the Verlet
// system updates the orientation of the surface automatically and only when
// necessary. In other words, the at-rest relationship between the nBoneIndex bone
// and its parent is maintained by the Verlet system, even as the parent moves.
//
// If the nBoneIndex bone has no parent, the world mesh's Xfm is used as the parent.
void CFVerletTackSurface::Init_Plane_Boned( const CFWorldMesh *pWorldMesh, u32 nBoneIndex, PlaneNormal_e nPlaneNormal ) {
	const CFMtx43A *pParentMtx;

	u8 nParentBoneIndex = pWorldMesh->m_pMesh->pBoneArray[nBoneIndex].Skeleton.nParentBoneIndex;

	if( nParentBoneIndex != 255 ) {
		// Bone has a parent...
		pParentMtx = pWorldMesh->GetBoneMtxPalette()[nParentBoneIndex];
	} else {
		// Bone has no parent. Use world mesh's Xfm...
		pParentMtx = &pWorldMesh->m_Xfm.m_MtxF;
	}

	Init_Plane_Boned_BS( pParentMtx, &pWorldMesh->m_pMesh->pBoneArray[nBoneIndex].AtRestBoneToParentMtx, nPlaneNormal );
}


// Inits a plane surface and disables it.
// Friction and distance are set to 0.
//
// The surface will be attached to the parent matrix pParentMtx,
// with an orientation relationship of Inv(pParentMtx) * pMtx_WS.
void CFVerletTackSurface::Init_Plane_Boned_WS( const CFMtx43A *pParentMtx, const CFMtx43A *pMtx_WS, PlaneNormal_e nPlaneNormal ) {
	CFMtx43A Mtx_BS;

	Mtx_BS.ReceiveAffineInverse( *pParentMtx, TRUE );
	Mtx_BS.Mul( *pMtx_WS );

	Init_Plane_Boned_BS( pParentMtx, &Mtx_BS, nPlaneNormal );
}


// Inits a plane surface and disables it.
// Friction and distance are set to 0.
//
// The surface will be attached to the parent matrix pParentMtx,
// with an orientation relationship of pMtx_BS.
void CFVerletTackSurface::Init_Plane_Boned_BS( const CFMtx43A *pParentMtx, const CFMtx43A *pMtx_BS, PlaneNormal_e nPlaneNormal ) {
	FASSERT( nPlaneNormal>=0 && nPlaneNormal<PLANE_NORMAL_COUNT );

	m_fFriction = 0.0f;
	m_fPlaneDist = 0.0f;
	m_nType = TYPE_PLANE;
	m_nAnchorType = ANCHOR_TYPE_BONED;
	m_nFlags = FLAG_MUST_UNITIZE_MTX;

	if( nPlaneNormal < PLANE_NORMAL_NX ) {
		m_nPlaneAxisIndex = nPlaneNormal;
	} else {
		FMATH_SETBITMASK( m_nFlags, FLAG_PLANE_NEGATE_NORMAL );
		m_nPlaneAxisIndex = nPlaneNormal - PLANE_NORMAL_NX;
	}

	m_nUnitMtxComputedFrame = FVid_nFrameCounter-1;

	m_pUnitMtx = &m_UnitMtx;
	m_pNonUnitMtx = pParentMtx;
	m_Mtx_BS = *pMtx_BS;

	m_UnitMtx.Mul( *m_pNonUnitMtx, m_Mtx_BS );
	m_UnitMtx.Mul33( m_UnitMtx.m_vX.InvMag() );

	m_BoxHalfDim_MS.Zero();
}


// Inits a box surface and disables it.
// Friction is set to 0.
void CFVerletTackSurface::Init_Box_Static( const CFMtx43A *pUnitMtx, const CFVec3A *pHalfDim_MS ) {
	m_fFriction = 0.0f;
	m_nType = TYPE_BOX;
	m_nAnchorType = ANCHOR_TYPE_STATIC;
	m_nPlaneAxisIndex = PLANE_NORMAL_Z;
	m_nFlags = FLAG_NONE;
	m_nUnitMtxComputedFrame = 0;

	m_pUnitMtx = &m_UnitMtx;
	m_UnitMtx = *pUnitMtx;
	m_BoxHalfDim_MS = *pHalfDim_MS;

	m_pNonUnitMtx = NULL;
	m_Mtx_BS.Zero();

	_ComputeBoxWorldSpaceCorners();
}


// Inits a box surface and disables it.
// Friction is set to 0.
//
// The surface will be attached directly to the provided matrix, pMtx.
void CFVerletTackSurface::Init_Box_Dynamic( const CFMtx43A *pMtx, const CFVec3A *pHalfDim_MS, BOOL bMtxMayBeNonUnit ) {
	m_fFriction = 0.0f;
	m_nType = TYPE_BOX;
	m_nAnchorType = ANCHOR_TYPE_DYNAMIC;
	m_nPlaneAxisIndex = PLANE_NORMAL_Z;
	m_nFlags = FLAG_NONE;
	m_nUnitMtxComputedFrame = FVid_nFrameCounter-1;

	if( bMtxMayBeNonUnit ) {
		FMATH_SETBITMASK( m_nFlags, FLAG_MUST_UNITIZE_MTX );

		m_pUnitMtx = &m_UnitMtx;
		m_pNonUnitMtx = pMtx;
	} else {
		m_pUnitMtx = pMtx;
		m_pNonUnitMtx = NULL;
	}

	m_BoxHalfDim_MS = *pHalfDim_MS;

	m_Mtx_BS.Zero();

	_ComputeBoxWorldSpaceCorners();
}


// Initializes a box surface and disables it.
// Friction and distance are set to 0.
//
// The surface will be attached to the parent of the bone specified by nBoneIndex.
//
// The nBoneIndex bone defines where the surface is located in the at-rest model.
// However, as an optimization, that bone does not need to be animated and can be
// masked out in the animation system. As the parent bone is animated, the Verlet
// system updates the orientation of the surface automatically and only when
// necessary. In other words, the at-rest relationship between the nBoneIndex bone
// and its parent is maintained by the Verlet system, even as the parent moves.
//
// If the nBoneIndex bone has no parent, the world mesh's Xfm is used as the parent.
void CFVerletTackSurface::Init_Box_Boned( CFWorldMesh *pWorldMesh, u32 nBoneIndex, const CFVec3A *pHalfDim_MS ) {
	CFMtx43A *pParentMtx;

	u8 nParentBoneIndex = pWorldMesh->m_pMesh->pBoneArray[nBoneIndex].Skeleton.nParentBoneIndex;

	if( nParentBoneIndex != 255 ) {
		// Bone has a parent...
		pParentMtx = pWorldMesh->GetBoneMtxPalette()[nParentBoneIndex];
	} else {
		// Bone has no parent. Use world mesh's Xfm...
		pParentMtx = &pWorldMesh->m_Xfm.m_MtxF;
	}

	Init_Box_Boned_BS( pParentMtx, &pWorldMesh->m_pMesh->pBoneArray[nBoneIndex].AtRestBoneToParentMtx, pHalfDim_MS );
}


// Inits a box surface and disables it.
// Friction is set to 0.
//
// The surface will be attached to the parent matrix pParentMtx,
// with an orientation relationship of Inv(pParentMtx) * pMtx_WS.
void CFVerletTackSurface::Init_Box_Boned_WS( const CFMtx43A *pParentMtx, const CFMtx43A *pMtx_WS, const CFVec3A *pHalfDim_MS ) {
	CFMtx43A Mtx_BS;

	Mtx_BS.ReceiveAffineInverse( *pParentMtx, TRUE );
	Mtx_BS.Mul( *pMtx_WS );

	Init_Box_Boned_BS( pParentMtx, &Mtx_BS, pHalfDim_MS );
}


// Inits a box surface and disables it.
// Friction is set to 0.
//
// The surface will be attached to the parent matrix pParentMtx,
// with an orientation relationship of pMtx_BS.
void CFVerletTackSurface::Init_Box_Boned_BS( const CFMtx43A *pParentMtx, const CFMtx43A *pMtx_BS, const CFVec3A *pHalfDim_MS ) {
	m_fFriction = 0.0f;
	m_nType = TYPE_BOX;
	m_nAnchorType = ANCHOR_TYPE_BONED;
	m_nPlaneAxisIndex = PLANE_NORMAL_Z;
	m_nFlags = FLAG_MUST_UNITIZE_MTX;
	m_nUnitMtxComputedFrame = FVid_nFrameCounter-1;

	m_pUnitMtx = &m_UnitMtx;
	m_pNonUnitMtx = pParentMtx;
	m_Mtx_BS = *pMtx_BS;

	m_UnitMtx.Mul( *m_pNonUnitMtx, m_Mtx_BS );
	m_UnitMtx.Mul33( m_UnitMtx.m_vX.InvMag() );

	m_BoxHalfDim_MS = *pHalfDim_MS;

	_ComputeBoxWorldSpaceCorners();
}


void CFVerletTackSurface::_ComputeUnitMtxForDynamicAnchor( void ) {
	m_nUnitMtxComputedFrame = FVid_nFrameCounter;

	m_UnitMtx.Mul33( *m_pNonUnitMtx, m_pNonUnitMtx->m_vX.InvMag() );
	m_UnitMtx.m_vPos = m_pNonUnitMtx->m_vPos;

	if( m_nType == TYPE_BOX ) {
		_ComputeBoxWorldSpaceCorners();
	}
}


void CFVerletTackSurface::_ComputeUnitMtxForBonedAnchor( void ) {
	m_nUnitMtxComputedFrame = FVid_nFrameCounter;

	m_UnitMtx.Mul( *m_pNonUnitMtx, m_Mtx_BS );
	m_UnitMtx.Mul33( m_UnitMtx.m_vX.InvMag() );

	if( m_nType == TYPE_BOX ) {
		_ComputeBoxWorldSpaceCorners();
	}
}


void CFVerletTackSurface::_ComputeBoxWorldSpaceCorners( void ) {
	CFVec3A MinCorner_MS;

	MinCorner_MS.ReceiveNegative( m_BoxHalfDim_MS );

	m_UnitMtx.MulPoint( m_BoxMinCorner_WS, MinCorner_MS );
	m_UnitMtx.MulPoint( m_BoxMaxCorner_WS, m_BoxHalfDim_MS );
}





