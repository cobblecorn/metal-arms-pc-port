//////////////////////////////////////////////////////////////////////////////////////
// botzom_part.cpp - Zombie bot part module.
//
// Author: Steve Ranck     
//////////////////////////////////////////////////////////////////////////////////////
// THIS CODE IS PROPRIETARY PROPERTY OF SWINGIN' APE STUDIOS, INC.
// Copyright (c) 2003
//
// The contents of this file may not be disclosed to third
// parties, copied or duplicated in any form, in whole or in part,
// without the prior written permission of Swingin' Ape Studios, Inc.
//////////////////////////////////////////////////////////////////////////////////////
// Modification History:
//
// Date     Who         Description
// -------- ----------  --------------------------------------------------------------
// 01/27/03 Ranck       Created.
//////////////////////////////////////////////////////////////////////////////////////

#include "fang.h"
#include "botzom_part.h"
#include "fres.h"
#include "fworld.h"
#include "botzom.h"
#include "fanim.h"
#include "floop.h"
#include "botpart.h"
#include "explosion.h"


#define _GRAVITY					-64.0f
#define _MAX_BOUNDING_SPHERE_SIZE	100.0f




//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CBotZomPartGroup
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

BOOL8 CBotZomPartGroup::m_bGroupsCreated;
u8 CBotZomPartGroup::m_nPartCount;
u8 CBotZomPartGroup::m_nGroupCount;
CBotZomPartGroup *CBotZomPartGroup::m_pGroupArray;
CFMtx43A **CBotZomPartGroup::m_apHiddenMtxPalette;
CFMtx43A *CBotZomPartGroup::m_aHiddenMtxPalette;


BOOL CBotZomPartGroup::CreateGroups( u32 nGroupCount, const FMesh_t *pMesh ) {
	FASSERT( !m_bGroupsCreated );
	FASSERT( nGroupCount );
	FASSERT( m_pGroupArray == NULL );

	CFVec3A Pos;
	u32 i, j;

	m_bGroupsCreated = TRUE;

	FResFrame_t ResFrame = fres_GetFrame();

	#if !FANG_PRODUCTION_BUILD
		u32 nFreeBytes = fres_GetFreeBytes();
	#endif

	m_nPartCount = 0;
	m_nGroupCount = 0;
	m_pGroupArray = NULL;

	m_nPartCount = pMesh->nUsedBoneCount;
	m_nGroupCount = nGroupCount;

	m_pGroupArray = fnew CBotZomPartGroup[nGroupCount];
	if( m_pGroupArray == NULL ) {
		DEVPRINTF( "CBotZomPartGroup::CreateGroups(): Not enough memory to create %u group arrays.\n", nGroupCount );
		goto _ExitWithError;
	}

	for( i=0; i<m_nGroupCount; ++i ) {
		m_pGroupArray[i].m_pOwnerBot = NULL;

		m_pGroupArray[i].m_pPartArray = fnew CBotZomPart[m_nPartCount];
		if( m_pGroupArray[i].m_pPartArray == NULL ) {
			DEVPRINTF( "CBotZomPartGroup::CreateGroups(): Not enough memory to create part array.\n" );
			goto _ExitWithError;
		}

		for( j=0; j<m_nPartCount; ++j ) {
			m_pGroupArray[i].m_pPartArray[j].m_nMode = CBotZomPart::MODE_DONE;
			m_pGroupArray[i].m_pPartArray[j].m_nFlags = CBotZomPart::FLAG_NONE;
		}
	}

	m_aHiddenMtxPalette = (CFMtx43A *)fres_AllocAndZero( sizeof(CFMtx43A) * pMesh->nBoneCount );
	if( m_aHiddenMtxPalette == NULL ) {
		DEVPRINTF( "CBotZomPartGroup::CreateGroups(): Not enough memory to create m_aHiddenMtxPalette.\n" );
		goto _ExitWithError;
	}

	m_apHiddenMtxPalette = (CFMtx43A **)fres_AllocAndZero( sizeof(CFMtx43A *) * pMesh->nBoneCount );
	if( m_apHiddenMtxPalette == NULL ) {
		DEVPRINTF( "CBotZomPartGroup::CreateGroups(): Not enough memory to create m_apHiddenMtxPalette.\n" );
		goto _ExitWithError;
	}

	for( i=0; i<pMesh->nBoneCount; ++i ) {
		m_apHiddenMtxPalette[i] = &m_aHiddenMtxPalette[i];
	}

	#if !FANG_PRODUCTION_BUILD
		DEVPRINTF( "Zombie Part System used %u bytes of memory for %u groups of parts.\n", nFreeBytes - fres_GetFreeBytes(), nGroupCount );
	#endif

	return TRUE;

_ExitWithError:
	DestroyGroups();
	fres_ReleaseFrame( ResFrame );

	return FALSE;
}


void CBotZomPartGroup::DestroyGroups( void ) {
	if( m_bGroupsCreated ) {
		u32 i;

		if( m_pGroupArray ) {
			for( i=0; i<m_nGroupCount; ++i ) {
				fdelete_array( m_pGroupArray[i].m_pPartArray );
				m_pGroupArray[i].m_pPartArray = NULL;
			}

			fdelete_array( m_pGroupArray );
			m_pGroupArray = NULL;
		}

		m_nPartCount = 0;
		m_nGroupCount = 0;

		m_bGroupsCreated = FALSE;
	}
}


void CBotZomPartGroup::FreeAllGroups( void ) {
	FASSERT( AreGroupsCreated() );

	u32 i;

	for( i=0; i<m_nGroupCount; ++i ) {
		m_pGroupArray[i].FreeGroup();
	}
}


void CBotZomPartGroup::FreeGroup( void ) {
	m_pOwnerBot = NULL;
}


BOOL CBotZomPartGroup::AssignFreeGroup( CBotZom *pBotZom ) {
	s32 nGroupIndex;

	for( nGroupIndex=0; nGroupIndex<m_nGroupCount; ++nGroupIndex ) {
		if( m_pGroupArray[nGroupIndex].m_pOwnerBot == NULL ) {
			// Found a free group...
			break;
		}
	}

	if( nGroupIndex == m_nGroupCount ) {
		// No free groups. Find the farthest bot that's no longer
		// in the active list...

		f32 fFarthestDist2, fDist2;
		s32 nFarthestGroupIndex;

		fFarthestDist2 = 0.0f;
		nFarthestGroupIndex = -1;

		for( nGroupIndex=0; nGroupIndex<m_nGroupCount; ++nGroupIndex ) {
			if( !(m_pGroupArray[nGroupIndex].m_pOwnerBot->m_pWorldMesh->GetVolumeFlags() & FVIS_VOLUME_IN_ACTIVE_LIST) ) {
				// This bot is no longer in the active list...

				fDist2 = pBotZom->MtxToWorld()->m_vPos.DistSq( m_pGroupArray[nGroupIndex].m_pOwnerBot->MtxToWorld()->m_vPos );

				if( fDist2 > fFarthestDist2 ) {
					// This bot is farther than any we've encountered so far...

					fFarthestDist2 = fDist2;
					nFarthestGroupIndex = nGroupIndex;
				}
			}
		}

		if( nFarthestGroupIndex < 0 ) {
			// Could not find a free group...
			return FALSE;
		}

		// Inform bot that he's losing his group...
		m_pGroupArray[ nFarthestGroupIndex ].m_pOwnerBot->LosingPartGroup();

		// Yank this group from the bot...
		m_pGroupArray[ nFarthestGroupIndex ].UnassignGroup();

		nGroupIndex = nFarthestGroupIndex;
	}

	// Assign this group to our bot...
	m_pGroupArray[ nGroupIndex ].m_pOwnerBot = pBotZom;
	m_pGroupArray[ nGroupIndex ].m_pOwnerBot->m_pPartGroup = &m_pGroupArray[ nGroupIndex ];

	m_pGroupArray[ nGroupIndex ].m_nRoutedPartsCount = 0;
	m_pGroupArray[ nGroupIndex ].m_nFlags = FLAG_NONE;

	return TRUE;
}


void CBotZomPartGroup::UnassignGroup( void ) {
	if( m_pOwnerBot ) {
		// This group is currently assigned to a bot...

		u32 i;

		// Make sure all parts are flagged as Done...
		for( i=0; i<m_nPartCount; ++i ) {
			m_pPartArray[i].m_nMode = CBotZomPart::MODE_DONE;
			m_pPartArray[i].m_nFlags = CBotZomPart::FLAG_NONE;
		}

		// Remove this group from use...
		m_nFlags = FLAG_NONE;
		m_nRoutedPartsCount = 0;
		m_pOwnerBot->m_pPartGroup = NULL;
		m_pOwnerBot = NULL;
	}
}


// Returns TRUE if the part group is no longer needed and has been unassigned.
BOOL CBotZomPartGroup::Work( void ) {
	const FMeshBone_t *pBoneArray;
	CFVec3A Pos;
	BOOL bKeepWorking;
	u32 i;

	pBoneArray = m_pOwnerBot->m_pWorldMesh->m_pMesh->pBoneArray;

	for( i=0; i<m_nPartCount; ++i ) {
		if( m_pPartArray[i].m_nMode != CBotZomPart::MODE_DONE ) {
			if( m_pPartArray[i].m_nFlags & CBotZomPart::FLAG_ROUTE_ANIM_TO_HIDDEN_PAL ) {
				m_pPartArray[i].m_QuatPosEnd.m_Quat.BuildQuat( m_aHiddenMtxPalette[i] );

				Pos.Set( pBoneArray[i].SegmentedBoundSphere_BS.m_Pos );
				m_aHiddenMtxPalette[i].MulPoint( m_pPartArray[i].m_QuatPosEnd.m_Pos, Pos );
			}
		}
	}

	bKeepWorking = FALSE;

	for( i=0; i<m_nPartCount; ++i ) {
		bKeepWorking |= m_pPartArray[i].Work( m_pOwnerBot, i );
	}

	if( !bKeepWorking ) {
		// All done with this part group...

		UnassignGroup();

		return TRUE;
	}

	m_nRoutedPartsCount = 0;
	for( i=0; i<m_nPartCount; ++i ) {
		if( m_pPartArray[i].m_nFlags & CBotZomPart::FLAG_ROUTE_ANIM_TO_HIDDEN_PAL ) {
			++m_nRoutedPartsCount;
		}
	}

	// Not done with this part group yet...
	return FALSE;
}


void CBotZomPartGroup::UpdateBotBoundingSphere( CBotZom *pBotZom ) {
	CFVec3A Min, Max, Origin_WS, Origin_MS, OriginToCorner_WS, *pBonePos;
	CFSphere BoundSphere_WS;
	CFMtx43A **pBonePalette;
	u32 i;
	BOOL bBoundSphereValid;

	pBonePalette = pBotZom->m_pWorldMesh->GetBoneMtxPalette();

	// Compute bounding sphere radius...
	Origin_WS.Zero();
	for( i=0; i<m_nPartCount; ++i ) {
		Origin_WS.Add( pBonePalette[i]->m_vPos );
	}
	if ( m_nPartCount)
	{
		Origin_WS.Mul(fmath_Inv(m_nPartCount));
	}

	// Compute min/max cube bounds...
	Min.Set( FMATH_MAX_FLOAT, FMATH_MAX_FLOAT, FMATH_MAX_FLOAT );
	Max.Set( -FMATH_MAX_FLOAT, -FMATH_MAX_FLOAT, -FMATH_MAX_FLOAT );
	bBoundSphereValid = FALSE;

	for( i=0; i<m_nPartCount; ++i ) {
		pBonePos = &pBonePalette[i]->m_vPos;

		if( pBonePos->DistSq( Origin_WS ) >= _MAX_BOUNDING_SPHERE_SIZE * _MAX_BOUNDING_SPHERE_SIZE) {
			// Throw this part out because it's too far away...
			continue;
		}

		Min.ClampMax( *pBonePos );
		Max.ClampMin( *pBonePos );

		bBoundSphereValid = TRUE;
	}

	if( !bBoundSphereValid ) {
		Min.Set( 0.0f, 0.0f, 0.0f );
		Max.Set( 0.001f, 0.001f, 0.001f );
	}

	// Compute origin in model space...
	pBotZom->m_pWorldMesh->m_Xfm.m_MtxR.MulPoint( Origin_MS, Origin_WS );

	// Compute vector from origin to maxima...
	OriginToCorner_WS.Sub( Max, Origin_WS );

	pBotZom->m_pWorldMesh->m_BoundSphere_MS.m_Pos = Origin_MS.v3;
	pBotZom->m_pWorldMesh->m_BoundSphere_MS.m_fRadius = OriginToCorner_WS.Mag() * pBotZom->m_pWorldMesh->m_Xfm.m_fScaleR;

	pBotZom->m_pWorldMesh->UpdateTracker();
}


// Returns FALSE if there are no free part groups.
BOOL CBotZomPartGroup::InitState_FinalDeathExplosion( CBotZom *pBot ) {
	CBotZomPartGroup *pPartGroup;
	CBotZomPart *pPart;
	u32 i;

	#define __JITTER 0.5f

	if( pBot->m_pPartGroup == NULL ) {
		// Get a free part group...
		if( !AssignFreeGroup( pBot ) ) {
			// No free part groups...
			return FALSE;
		}
	}

	pPartGroup = pBot->m_pPartGroup;
	FASSERT( pPartGroup );

	if( pPartGroup->m_nFlags & FLAG_LOCKED ) {
		// Already exploding...
		return TRUE;
	}

	// Part group obtained and assigned to bot!

	FMATH_SETBITMASK( pPartGroup->m_nFlags, FLAG_LOCKED );

	CFVec3A PerturbVec;

	pPartGroup->_InitPartQuatsFromCurrentBoneMtxPalette();

	for( i=0; i<m_nPartCount; ++i ) {
		pPart = &pPartGroup->m_pPartArray[i];

		pPart->m_nMode = CBotZomPart::MODE_DEBRIS;
		pPart->m_nConvertDebris = CBotZomPart::CONVERT_DEBRIS_TO_EXPLOSION;
		pPart->m_nFlags = CBotZomPart::FLAG_ROUTE_ANIM_TO_HIDDEN_PAL;
		pPart->m_fSpeedMult = 0.0f;

		pPart->m_fTimer = fmath_RandomFloatRange( 0.01f, 1.5f );

		pPart->m_fRotSpeed = fmath_RandomFloatRange( 4.0f, 8.0f );
		pPart->m_UnitRotAxis_WS.SetUnitRandom();

		PerturbVec.Set(
			fmath_RandomFloatRange( -__JITTER, __JITTER ),
			fmath_RandomFloatRange( -__JITTER, __JITTER ),
			fmath_RandomFloatRange( -__JITTER, __JITTER )
		);

		pPart->m_LinVel_WS.Add( CFVec3A::m_UnitAxisY, PerturbVec ).Unitize().Mul( fmath_RandomFloatRange( 20.0f, 50.0f ) );
	}

	return TRUE;
}


// Returns FALSE if there are no free part groups.
BOOL CBotZomPartGroup::InitState_Forming( CBotZom *pBot ) {
	CBotZomPartGroup *pPartGroup;
	CBotZomPart *pPart;
	u32 i;

	if( pBot->m_pPartGroup == NULL ) {
		// Get a free part group...
		if( !AssignFreeGroup( pBot ) ) {
			// No free part groups...
			return FALSE;
		}
	}

	pPartGroup = pBot->m_pPartGroup;
	FASSERT( pPartGroup );

	if( pPartGroup->m_nFlags & FLAG_LOCKED ) {
		return FALSE;
	}

	// Part group obtained and assigned to bot!

	const FMeshBone_t *pBoneArray;
	CFMtx43A **ppMtxPalette;

	ppMtxPalette = pBot->m_pWorldMesh->GetBoneMtxPalette();
	pBoneArray = pBot->m_pWorldMesh->m_pMesh->pBoneArray;

	for( i=0; i<m_nPartCount; ++i ) {
		pPart = &pPartGroup->m_pPartArray[i];

		if( pPart->m_nMode == CBotZomPart::MODE_FORMING ) {
			// Already forming...
			continue;
		}

		// Delay for a bit before beginning to move part...
		pPart->m_fTimer = fmath_RandomFloatRange( 0.001f, 0.5f );
		pPart->m_nConvertDebris = CBotZomPart::CONVERT_DEBRIS_TO_FORMING;

		// If the part isn't debris, switch to forming mode now...
		if( (pPart->m_nMode != CBotZomPart::MODE_DEBRIS) || (pPart->m_nFlags & CBotZomPart::FLAG_EMP_DEBRIS_MODE) ) {
			pPart->_ConvertToFormingMode( ppMtxPalette[i], &pBoneArray[i], pPart->m_fTimer );
		}
	}

	return TRUE;
}


// Returns FALSE if there are no free part groups.
BOOL CBotZomPartGroup::InitState_Hopping( CBotZom *pBot ) {
	CBotZomPartGroup *pPartGroup;
	CBotZomPart *pPart;
	u32 i;

	if( pBot->m_pPartGroup == NULL ) {
		// Get a free part group...
		if( !AssignFreeGroup( pBot ) ) {
			// No free part groups...
			return FALSE;
		}
	}

	pPartGroup = pBot->m_pPartGroup;
	FASSERT( pPartGroup );

	if( pPartGroup->m_nFlags & FLAG_LOCKED ) {
		return FALSE;
	}

	// Part group obtained and assigned to bot!

	const FMeshBone_t *pBoneArray;
	CFMtx43A **ppMtxPalette;
	CFVec3A Pos;

	ppMtxPalette = pBot->m_pWorldMesh->GetBoneMtxPalette();
	pBoneArray = pBot->m_pWorldMesh->m_pMesh->pBoneArray;

	pPartGroup->_InitPartQuatsFromCurrentBoneMtxPalette();

	for( i=0; i<m_nPartCount; ++i ) {
		pPart = &pPartGroup->m_pPartArray[i];

		// Delay for a bit before beginning to move part...
		pPart->m_nMode = CBotZomPart::MODE_FORMING;
		pPart->m_nFlags = CBotZomPart::FLAG_ROUTE_ANIM_TO_HIDDEN_PAL | CBotZomPart::FLAG_NO_FORMING_SCURVE;
		pPart->m_nConvertDebris = CBotZomPart::CONVERT_DEBRIS_NONE;

		pPart->m_fTimer = 0.0f;
		pPart->m_fSpeedMult = fmath_RandomFloatRange( 0.75f, 1.75f );
		pPart->m_nScaledHeight = (u8)fmath_RandomChoice( 256 );
//		pPart->m_nScaledHeight = 0;

#if 0
		pPart->m_QuatPosStart.m_Quat.BuildQuat( *ppMtxPalette[i] );
		Pos.Set( pBoneArray[i].SegmentedBoundSphere_BS.m_Pos );
		ppMtxPalette[i]->MulPoint( pPart->m_QuatPosStart.m_Pos, Pos );
#endif
	}

	return TRUE;
}


// Returns FALSE if there are no free part groups.
BOOL CBotZomPartGroup::InitState_EMP( CBotZom *pBot ) {
	CBotZomPartGroup *pPartGroup;
	CBotZomPart *pPart;
	u32 i;

	if( pBot->m_pPartGroup == NULL ) {
		// Get a free part group...
		if( !AssignFreeGroup( pBot ) ) {
			// No free part groups...
			return FALSE;
		}
	}

	pPartGroup = pBot->m_pPartGroup;
	FASSERT( pPartGroup );

	if( pPartGroup->m_nFlags & FLAG_LOCKED ) {
		return FALSE;
	}

	// Part group obtained and assigned to bot!

	CFVec3A BotCenter_WS;
	f32 fMag2;

	pPartGroup->_InitPartQuatsFromCurrentBoneMtxPalette();

	BotCenter_WS.Set( pBot->m_pWorldMesh->GetBoundingSphere().m_Pos );
	BotCenter_WS.Add( pBot->MtxToWorld()->m_vPos );
	BotCenter_WS.Mul( CFVec3A::m_HalfVec );

	for( i=0; i<m_nPartCount; ++i ) {
		pPart = &pPartGroup->m_pPartArray[i];

		if( pPart->m_nMode == CBotZomPart::MODE_DEBRIS ) {
			if( pPart->m_nFlags & CBotZomPart::FLAG_EMP_DEBRIS_MODE ) {
				// Already in EMP mode...
				continue;
			}
		}

		pPart->m_nMode = CBotZomPart::MODE_DEBRIS;
		pPart->m_nConvertDebris = CBotZomPart::CONVERT_DEBRIS_NONE;
		pPart->m_nFlags = CBotZomPart::FLAG_ROUTE_ANIM_TO_HIDDEN_PAL | CBotZomPart::FLAG_EMP_DEBRIS_MODE;

		pPart->m_fRotSpeed = fmath_RandomFloatRange( 1.0f, 1.5f );
		pPart->m_UnitRotAxis_WS.SetUnitRandom();

		pPart->m_LinVel_WS.Sub( pPart->m_QuatPosStart.m_Pos, BotCenter_WS );
		fMag2 = pPart->m_LinVel_WS.MagSq();
		if( fMag2 > 0.0001f ) {
			pPart->m_LinVel_WS.Mul( fmath_InvSqrt(fMag2) );
		} else {
			pPart->m_LinVel_WS = CFVec3A::m_UnitAxisY;
		}

		pPart->m_LinVel_WS.Mul( fmath_RandomFloatRange( 5.0f, 10.0f ) );
	}

	return TRUE;
}


// Returns FALSE if there are no free part groups.
BOOL CBotZomPartGroup::InitState_Collapse( CBotZom *pBot ) {
	CBotZomPartGroup *pPartGroup;
	CBotZomPart *pPart;
	u32 i;

	if( pBot->m_pPartGroup == NULL ) {
		// Get a free part group...
		if( !AssignFreeGroup( pBot ) ) {
			// No free part groups...
			return FALSE;
		}
	}

	pPartGroup = pBot->m_pPartGroup;
	FASSERT( pPartGroup );

	if( pPartGroup->m_nFlags & FLAG_LOCKED ) {
		return FALSE;
	}

	// Part group obtained and assigned to bot!

	pPartGroup->_InitPartQuatsFromCurrentBoneMtxPalette();

	for( i=0; i<m_nPartCount; ++i ) {
		pPart = &pPartGroup->m_pPartArray[i];

		pPart->m_nMode = CBotZomPart::MODE_DEBRIS;
		pPart->m_nConvertDebris = CBotZomPart::CONVERT_DEBRIS_NONE;
		pPart->m_nFlags = CBotZomPart::FLAG_ROUTE_ANIM_TO_HIDDEN_PAL;

		pPart->m_fRotSpeed = fmath_RandomFloatRange( 4.0f, 8.0f );
		pPart->m_fSpeedMult = 0.0f;
		pPart->m_LinVel_WS.Set( 0.0f, fmath_RandomFloatRange( 5.0f, 10.0f ), 0.0f );
		pPart->m_UnitRotAxis_WS.SetUnitRandom();
	}

	return TRUE;
}


// Returns FALSE if there are no free part groups.
BOOL CBotZomPartGroup::InitState_StagedCollapse( CBotZom *pBot ) {
	CBotZomPartGroup *pPartGroup;
	CBotZomPart *pPart;
	CFVec3A PartialBotVel_WS;
	u32 i;

	if( pBot->m_pPartGroup == NULL ) {
		// Get a free part group...
		if( !AssignFreeGroup( pBot ) ) {
			// No free part groups...
			return FALSE;
		}
	}

	pPartGroup = pBot->m_pPartGroup;
	FASSERT( pPartGroup );

	if( pPartGroup->m_nFlags & FLAG_LOCKED ) {
		return FALSE;
	}

	// Part group obtained and assigned to bot!

	PartialBotVel_WS.Mul( pBot->m_Velocity_WS, 0.5f );

	for( i=0; i<m_nPartCount; ++i ) {
		pPart = &pPartGroup->m_pPartArray[i];

		pPart->m_nMode = CBotZomPart::MODE_STAGED_DEBRIS;
		pPart->m_nFlags = CBotZomPart::FLAG_NONE;

		pPart->m_fRotSpeed = fmath_RandomFloatRange( 4.0f, 8.0f );
		pPart->m_UnitRotAxis_WS.SetUnitRandom();

		pPart->m_LinVel_WS.Set( 0.0f, fmath_RandomFloatRange( 15.0f, 20.0f ), 0.0f );
		pPart->m_LinVel_WS.Add( PartialBotVel_WS );

		pPart->m_fTimer = 8.0f;
		pPart->m_fSpeedMult = pPart->m_fTimer;
	}

	return TRUE;
}


// Returns FALSE if the affect was not started (either there were no
// available part groups or the blast didn't affect any of the parts).
BOOL CBotZomPartGroup::InitState_DebrisFromBlast( CBotZom *pBot, const CDamageResult *pDamageResult ) {
	FASSERT( pDamageResult->m_pDamageData->m_nDamageLocale == CDamageForm::DAMAGE_LOCALE_BLAST );

	CFVec3A EpicenterToPart_WS;
	const CFVec3A *pEpicenter_WS;
	CFMtx43A **ppMtxPalette;
	f32 fBlastRadius2;
	u32 i;

	pEpicenter_WS = pDamageResult->m_pDamageData->m_pEpicenter_WS;
	ppMtxPalette = pBot->m_pWorldMesh->GetBoneMtxPalette();

	fBlastRadius2 = pDamageResult->m_pDamageData->m_pDamageProfile->m_ImpulseRange.m_fOuterRadius;
	fBlastRadius2 *= fBlastRadius2;

	// Check to see if any part is within range of the blast sphere...
	for( i=0; i<m_nPartCount; ++i ) {
		EpicenterToPart_WS.Sub( ppMtxPalette[i]->m_vPos, *pEpicenter_WS );

		if( EpicenterToPart_WS.MagSq() < fBlastRadius2 ) {
			// We found a part that was within range of the blast sphere...
			break;
		}
	}

	if( i == m_nPartCount ) {
		// No parts were affected...
		return FALSE;
	}

	if( pBot->m_pPartGroup == NULL ) {
		// Get a free part group...
		if( !AssignFreeGroup( pBot ) ) {
			// No free part groups...
			return FALSE;
		}
	}

	CBotZomPartGroup *pPartGroup = pBot->m_pPartGroup;
	FASSERT( pPartGroup );

	if( pPartGroup->m_nFlags & FLAG_LOCKED ) {
		return FALSE;
	}

	// Part group obtained and assigned to bot!

	const FMeshBone_t *pBoneArray;
	CFVec3A *pPartPos_WS, Pos, UpVelocity_WS, ScaledUpVelocity_WS;
	f32 fDistFromEpicenterToPart2, fUnitImpulse;
	CBotZomPart *pPart;

	pBoneArray = pBot->m_pWorldMesh->m_pMesh->pBoneArray;

	UpVelocity_WS.Set( 0.0f, 40.0f, 0.0f );

	for( i=0; i<m_nPartCount; ++i ) {
		pPart = &pPartGroup->m_pPartArray[i];

		pPart->m_nMode = CBotZomPart::MODE_DEBRIS;
		pPart->m_nConvertDebris = CBotZomPart::CONVERT_DEBRIS_NONE;
		pPart->m_nFlags = CBotZomPart::FLAG_ROUTE_ANIM_TO_HIDDEN_PAL;
		pPart->m_fSpeedMult = 0.0f;

		pPart->m_QuatPosStart.m_Quat.BuildQuat( *ppMtxPalette[i] );
		Pos.Set( pBoneArray[i].SegmentedBoundSphere_BS.m_Pos );
		ppMtxPalette[i]->MulPoint( pPart->m_QuatPosStart.m_Pos, Pos );

		pPartPos_WS = &ppMtxPalette[i]->m_vPos;
		EpicenterToPart_WS.Sub( *pPartPos_WS, *pEpicenter_WS );

		fDistFromEpicenterToPart2 = EpicenterToPart_WS.MagSq();

		fUnitImpulse = 0.0f;
		if( fDistFromEpicenterToPart2 > 0.00001f ) {
			if( fDistFromEpicenterToPart2 < fBlastRadius2 ) {
				fUnitImpulse = CDamage::ComputeRadialIntensity_Impulse( fmath_Sqrt( fDistFromEpicenterToPart2 ), pDamageResult->m_pDamageData->m_pDamageProfile );
			}
		}

		if( fUnitImpulse > 0.0f ) {
			// Part is affected by blast...

			fUnitImpulse *= (1.0f / 12000.0f);
			FMATH_CLAMP_UNIT_FLOAT( fUnitImpulse );

			ScaledUpVelocity_WS.Mul( UpVelocity_WS, fUnitImpulse * fmath_RandomFloatRange( 0.5f, 1.5f ) );

			pPart->m_LinVel_WS.Mul( EpicenterToPart_WS, fmath_InvSqrt(fDistFromEpicenterToPart2) );
			pPart->m_LinVel_WS.Mul( fUnitImpulse * fmath_RandomFloatRange( 10.0f, 20.0f ) );
			pPart->m_LinVel_WS.Add( ScaledUpVelocity_WS );

			pPart->m_fRotSpeed = fmath_RandomFloatRange( 4.0f, 8.0f );
		} else {
			// Part is not affected by blast...

			pPart->m_LinVel_WS.Zero();

			pPart->m_fRotSpeed = fmath_RandomFloatRange( 1.0f, 2.0f );
		}

		pPart->m_UnitRotAxis_WS.SetUnitRandom();
	}

	return TRUE;
}


// Returns FALSE if there are no free part groups.
BOOL CBotZomPartGroup::InitState_DebrisFromImpact( CBotZom *pBot, const CDamageResult *pDamageResult ) {
	FASSERT( pDamageResult->m_pDamageData->m_nDamageLocale == CDamageForm::DAMAGE_LOCALE_IMPACT );

	CBotZomPartGroup *pPartGroup;
	CBotZomPart *pPart;

	if( pBot->m_pPartGroup == NULL ) {
		// Get a free part group...
		if( !AssignFreeGroup( pBot ) ) {
			// No free part groups...
			return FALSE;
		}
	}

	pPartGroup = pBot->m_pPartGroup;
	FASSERT( pPartGroup );

	if( pPartGroup->m_nFlags & FLAG_LOCKED ) {
		return FALSE;
	}

	// Part group obtained and assigned to bot!

	const FMeshBone_t *pBoneArray;
	CFMtx43A *pMtxPalette;
	CFVec3A Pos, ScaledUpVelocity_WS;
	f32 fUnitImpulse;
	u32 nBoneIndex;

	nBoneIndex = pDamageResult->m_pDamageData->m_pTriData->nDamageeBoneIndex;

	pMtxPalette = pBot->m_pWorldMesh->GetBoneMtxPalette()[ nBoneIndex ];
	pBoneArray = &pBot->m_pWorldMesh->m_pMesh->pBoneArray[ nBoneIndex ];

	pPart = &pPartGroup->m_pPartArray[ nBoneIndex ];

	if( pPart->m_nMode == CBotZomPart::MODE_FORMING ) {
		return TRUE;
	}

	fUnitImpulse = pDamageResult->m_fImpulseMag;

	if( fUnitImpulse <= 0.0f ) {
		return TRUE;
	}

	fUnitImpulse *= (1.0f / 1200.0f);
	FMATH_CLAMP_UNIT_FLOAT( fUnitImpulse );

	pPart->m_nMode = CBotZomPart::MODE_DEBRIS;
	pPart->m_nConvertDebris = CBotZomPart::CONVERT_DEBRIS_NONE;
	pPart->m_nFlags = CBotZomPart::FLAG_ROUTE_ANIM_TO_HIDDEN_PAL;
	pPart->m_fSpeedMult = 0.0f;

	ScaledUpVelocity_WS.Set( 0.0f, 20.0f, 0.0f );
	ScaledUpVelocity_WS.Mul( fUnitImpulse * fmath_RandomFloatRange( 0.5f, 1.5f ) );

	pPart->m_LinVel_WS.Mul( pDamageResult->m_pDamageData->m_AttackUnitDir_WS, fUnitImpulse * fmath_RandomFloatRange( 5.0f, 10.0f ) );
	pPart->m_LinVel_WS.Add( ScaledUpVelocity_WS );

	pPart->m_fRotSpeed = fmath_RandomFloatRange( 4.0f, 8.0f );
	pPart->m_UnitRotAxis_WS.SetUnitRandom();

	pPart->m_QuatPosStart.m_Quat.BuildQuat( *pMtxPalette );
	Pos.Set( pBoneArray->SegmentedBoundSphere_BS.m_Pos );
	pMtxPalette->MulPoint( pPart->m_QuatPosStart.m_Pos, Pos );

	return TRUE;
}


// Returns FALSE if there are no free part groups.
BOOL CBotZomPartGroup::InitState_RecoilFromBlast( CBotZom *pBot, const CDamageResult *pDamageResult ) {
	FASSERT( pDamageResult->m_pDamageData->m_nDamageLocale == CDamageForm::DAMAGE_LOCALE_BLAST );

	CBotZomPartGroup *pPartGroup;
	CBotZomPart *pPart;
	u32 i;

	if( pBot->m_pPartGroup == NULL ) {
		// Get a free part group...
		if( !AssignFreeGroup( pBot ) ) {
			// No free part groups...
			return FALSE;
		}
	}

	pPartGroup = pBot->m_pPartGroup;
	FASSERT( pPartGroup );

	if( pPartGroup->m_nFlags & FLAG_LOCKED ) {
		return FALSE;
	}

	// Part group obtained and assigned to bot!

	const FMeshBone_t *pBoneArray;
	CFMtx43A **ppMtxPalette;
	CFVec3A *pPartPos_WS, EpicenterToPart_WS, Pos, ScaledUpVelocity_WS, PartialBotVel_WS;
	f32 fBlastRadius2, fDistFromEpicenterToPart2, fUnitImpulse, fImpulse;

	ppMtxPalette = pBot->m_pWorldMesh->GetBoneMtxPalette();
	pBoneArray = pBot->m_pWorldMesh->m_pMesh->pBoneArray;

	fBlastRadius2 = pDamageResult->m_pDamageData->m_pDamageProfile->m_ImpulseRange.m_fOuterRadius;
	fBlastRadius2 *= fBlastRadius2;

	PartialBotVel_WS.Mul( pBot->m_Velocity_WS, 0.5f );

	for( i=0; i<m_nPartCount; ++i ) {
		pPart = &pPartGroup->m_pPartArray[i];

		if( pPart->m_nMode == CBotZomPart::MODE_RECOIL ) {
			continue;
		}

		pPartPos_WS = &ppMtxPalette[i]->m_vPos;

		EpicenterToPart_WS.Sub( *pPartPos_WS, *pDamageResult->m_pDamageData->m_pEpicenter_WS );

		fDistFromEpicenterToPart2 = EpicenterToPart_WS.MagSq();

		if( fDistFromEpicenterToPart2 <= 0.00001f ) {
			continue;
		}

		if( fDistFromEpicenterToPart2 >= fBlastRadius2 ) {
			continue;
		}

		// Part is in range of blast impulse...

		fUnitImpulse = CDamage::ComputeRadialIntensity_Impulse( fmath_Sqrt( fDistFromEpicenterToPart2 ), pDamageResult->m_pDamageData->m_pDamageProfile );

		if( fUnitImpulse <= 0.0f ) {
			continue;
		}

		fUnitImpulse = pDamageResult->m_fImpulseMag;

		if( fUnitImpulse <= 0.0f ) {
			return TRUE;
		}

		fUnitImpulse *= (1.0f / 12000.0f);
		FMATH_CLAMP_UNIT_FLOAT( fUnitImpulse );
		fImpulse = FMATH_FPOT( fUnitImpulse, 1.0f, 2.0f );

		pPart->m_nMode = CBotZomPart::MODE_RECOIL;
		pPart->m_nFlags = CBotZomPart::FLAG_ROUTE_ANIM_TO_HIDDEN_PAL | CBotZomPart::FLAG_PLAY_SPARSED_CLATTER_SOUND;
		pPart->m_fTimer = 0.0f;

		ScaledUpVelocity_WS.Mul( CFVec3A::m_UnitAxisY, fImpulse * fmath_RandomFloatRange( 10.0f, 15.0f ) );

		pPart->m_LinVel_WS.Mul( pDamageResult->m_pDamageData->m_AttackUnitDir_WS, fUnitImpulse * fmath_RandomFloatRange( 2.0f, 4.0f ) );
		pPart->m_LinVel_WS.Add( ScaledUpVelocity_WS );
		pPart->m_LinVel_WS.Add( PartialBotVel_WS );

		pPart->m_fRotSpeed = fmath_RandomFloatRange( 4.0f, 8.0f );
		pPart->m_UnitRotAxis_WS.SetUnitRandom();

		pPart->m_fSpeedMult = pPart->m_LinVel_WS.y * (1.75f / -_GRAVITY);

		pPart->m_QuatPosStart.m_Quat.BuildQuat( *ppMtxPalette[i] );
		Pos.Set( pBoneArray[i].SegmentedBoundSphere_BS.m_Pos );
		ppMtxPalette[i]->MulPoint( pPart->m_QuatPosStart.m_Pos, Pos );
	}

	return TRUE;
}


// Returns FALSE if there are no free part groups.
BOOL CBotZomPartGroup::InitState_RecoilFromImpact( CBotZom *pBot, const CDamageResult *pDamageResult ) {
	FASSERT( pDamageResult->m_pDamageData->m_nDamageLocale == CDamageForm::DAMAGE_LOCALE_IMPACT );

	CBotZomPartGroup *pPartGroup;
	CBotZomPart *pPart;

	if( pBot->m_pPartGroup == NULL ) {
		// Get a free part group...
		if( !AssignFreeGroup( pBot ) ) {
			// No free part groups...
			return FALSE;
		}
	}

	pPartGroup = pBot->m_pPartGroup;
	FASSERT( pPartGroup );

	if( pPartGroup->m_nFlags & FLAG_LOCKED ) {
		return FALSE;
	}

	// Part group obtained and assigned to bot!

	const FMeshBone_t *pBoneArray;
	CFMtx43A *pMtxPalette;
	CFVec3A Pos, ScaledUpVelocity_WS, PartialBotVel_WS;
	f32 fUnitImpulse, fImpulse;
	u32 nBoneIndex;

	nBoneIndex = pDamageResult->m_pDamageData->m_pTriData->nDamageeBoneIndex;

	pPart = &pPartGroup->m_pPartArray[ nBoneIndex ];

	if( pPart->m_nMode == CBotZomPart::MODE_RECOIL ) {
		return TRUE;
	}

	pMtxPalette = pBot->m_pWorldMesh->GetBoneMtxPalette()[ nBoneIndex ];
	pBoneArray = &pBot->m_pWorldMesh->m_pMesh->pBoneArray[ nBoneIndex ];

	fUnitImpulse = pDamageResult->m_fImpulseMag;

	if( fUnitImpulse <= 0.0f ) {
		return TRUE;
	}

	fUnitImpulse *= (1.0f / 1200.0f);
	FMATH_CLAMP_UNIT_FLOAT( fUnitImpulse );
	fImpulse = FMATH_FPOT( fUnitImpulse, 1.0f, 2.0f );

	pPart->m_nMode = CBotZomPart::MODE_RECOIL;
	pPart->m_nFlags = CBotZomPart::FLAG_ROUTE_ANIM_TO_HIDDEN_PAL;
	pPart->m_fTimer = 0.0f;

	PartialBotVel_WS.Mul( pBot->m_Velocity_WS, 0.5f );
	ScaledUpVelocity_WS.Mul( CFVec3A::m_UnitAxisY, fImpulse * fmath_RandomFloatRange( 10.0f, 15.0f ) );

	pPart->m_LinVel_WS.Mul( pDamageResult->m_pDamageData->m_AttackUnitDir_WS, fUnitImpulse * fmath_RandomFloatRange( 5.0f, 10.0f ) );
	pPart->m_LinVel_WS.Add( ScaledUpVelocity_WS );
	pPart->m_LinVel_WS.Add( PartialBotVel_WS );

	pPart->m_fRotSpeed = fmath_RandomFloatRange( 2.0f, 4.0f );
	pPart->m_UnitRotAxis_WS.SetUnitRandom();

	pPart->m_fSpeedMult = pPart->m_LinVel_WS.y * (1.75f / -_GRAVITY);

	pPart->m_QuatPosStart.m_Quat.BuildQuat( *pMtxPalette );
	Pos.Set( pBoneArray->SegmentedBoundSphere_BS.m_Pos );
	pMtxPalette->MulPoint( pPart->m_QuatPosStart.m_Pos, Pos );

	pPartGroup->m_pOwnerBot->_PlayClatterSound( &pPart->m_QuatPosStart.m_Pos );

	return TRUE;
}


void CBotZomPartGroup::_InitPartQuatsFromCurrentBoneMtxPalette( void ) {
	CFMtx43A **ppMtxPalette;
	const FMeshBone_t *pBoneArray;
	CFVec3A Pos;
	u32 i;

	ppMtxPalette = m_pOwnerBot->m_pWorldMesh->GetBoneMtxPalette();
	pBoneArray = m_pOwnerBot->m_pWorldMesh->m_pMesh->pBoneArray;

	for( i=0; i<m_nPartCount; ++i ) {
		m_pPartArray[i].m_QuatPosStart.m_Quat.BuildQuat( *ppMtxPalette[i] );

		Pos.Set( pBoneArray[i].SegmentedBoundSphere_BS.m_Pos );
		ppMtxPalette[i]->MulPoint( m_pPartArray[i].m_QuatPosStart.m_Pos, Pos );
	}
}


void CBotZomPartGroup::ComputeBotMtxPalette( CBotZom *pBotZom ) {
	pBotZom->m_pCollBot = pBotZom;

	if( pBotZom->m_pPartGroup == NULL ) {
		// Bot has no part group...

		pBotZom->m_Anim.m_pAnimCombiner->ComputeMtxPalette( TRUE );

		return;
	}

	// Bot has a part group...

	CBotZomPartGroup *pPartGroup = pBotZom->m_pPartGroup;
	u32 i;

	// Save mtx palette redirection array and reroute the appropriate entries...
	for( i=0; i<m_nPartCount; ++i ) {
		m_apHiddenMtxPalette[i] = pBotZom->m_pWorldMesh->GetBoneMtxPalette()[i];

		if( pPartGroup->m_pPartArray[i].m_nFlags & CBotZomPart::FLAG_ROUTE_ANIM_TO_HIDDEN_PAL ) {
			pBotZom->m_pWorldMesh->GetBoneMtxPalette()[i] = &m_aHiddenMtxPalette[i];
		}
	}

	// Compute bot's matrix palette...
	pBotZom->m_Anim.m_pAnimCombiner->ComputeMtxPalette( TRUE );

	// Restore mtx palette redirection array...
	for( i=0; i<m_nPartCount; ++i ) {
		pBotZom->m_pWorldMesh->GetBoneMtxPalette()[i] = m_apHiddenMtxPalette[i];
	}
}




//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CBotZomPart
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************


// Returns TRUE if the work function needs to be called again.
// Returns FALSE if no more work is required.
BOOL CBotZomPart::Work( CBotZom *pBot, u32 nBoneIndex ) {
	switch( m_nMode ) {
	case MODE_FORMING:
		_Work_Forming( pBot, nBoneIndex );
		break;

	case MODE_DEBRIS:
		_Work_Debris( pBot, nBoneIndex );
		break;

	case MODE_STAGED_DEBRIS:
		_Work_StagedDebris( pBot, nBoneIndex );
		break;

	case MODE_RECOIL:
		_Work_Recoiling( pBot, nBoneIndex );
		break;

	};

	return (m_nMode != MODE_DONE);
}


void CBotZomPart::_Work_Forming( CBotZom *pBot, u32 nBoneIndex ) {
	if( m_nFlags & FLAG_WAITING_BEFORE_FORMING ) {
		// Wait mode...

		m_fTimer -= FLoop_fPreviousLoopSecs;

		if( m_fTimer > 0.0f ) {
			// Still in wait mode...
			return;
		}

		// We're done waiting...
		FMATH_CLEARBITMASK( m_nFlags, FLAG_WAITING_BEFORE_FORMING );
		m_fTimer = 0.0f;
	}

	// Let's move the part...

	CFVec3A BoundSphereDeltaPos_BS, BoundSphereDeltaPos_WS, *pDestPos;
	CFQuatA Quat;
	f32 fHeight, fAnim, fMinX, fMaxX, fDeltaX, fMaxY, fDeltaY, fY;

	// Advance unit time...
	m_fTimer += m_fSpeedMult * FLoop_fPreviousLoopSecs;
	if( m_fTimer >= 1.0f ) {
		// We're done moving the part...

		m_fTimer = 1.0f;

		m_nMode = MODE_DONE;
		FMATH_CLEARBITMASK( m_nFlags, FLAG_ROUTE_ANIM_TO_HIDDEN_PAL );
	}

	if( !(m_nFlags & FLAG_NO_FORMING_SCURVE) ) {
		fAnim = fmath_UnitLinearToSCurve( m_fTimer );
		fHeight = ((f32)m_nScaledHeight * (6.0f/255.0f)) + 4.0f;
	} else {
		fAnim = m_fTimer;
		fHeight = ((f32)m_nScaledHeight * (3.0f/255.0f)) + 0.1f;
	}

	if( m_QuatPosStart.m_Pos.y >= m_QuatPosEnd.m_Pos.y ) {
		fDeltaY = m_QuatPosStart.m_Pos.y - m_QuatPosEnd.m_Pos.y;
		fMaxX = fmath_Sqrt( 1.0f + fDeltaY * fmath_Inv(fHeight) );

		fMinX = -1.0f;
		fDeltaX = fMaxX + 1.0f;
		fMaxY = m_QuatPosStart.m_Pos.y;
	} else {
		fDeltaY = m_QuatPosEnd.m_Pos.y - m_QuatPosStart.m_Pos.y;
		fMaxX = fmath_Sqrt( 1.0f + fDeltaY * fmath_Inv(fHeight) );

		fMinX = fMaxX;
		fDeltaX = -1.0f - fMaxX;
		fMaxY = m_QuatPosEnd.m_Pos.y;
	}

	// Compute Y value...
	fY = fMinX + fAnim * fDeltaX;
	fY = fMaxY + fHeight*( 1.0f - fY*fY );

	// Compute new orientation...
	Quat.ReceiveSlerpOf( fAnim, m_QuatPosStart.m_Quat, m_QuatPosEnd.m_Quat );
	Quat.BuildMtx( *pBot->m_pWorldMesh->GetBoneMtxPalette()[ nBoneIndex ] );

	// Compute world space vector from bound sphere center to bone geo origin...
	BoundSphereDeltaPos_BS.Set( pBot->m_pWorldMesh->m_pMesh->pBoneArray[ nBoneIndex ].SegmentedBoundSphere_BS.m_Pos );
	BoundSphereDeltaPos_BS.Negate();
	Quat.MulPoint( BoundSphereDeltaPos_WS, BoundSphereDeltaPos_BS );

	pDestPos = &pBot->m_pWorldMesh->GetBoneMtxPalette()[ nBoneIndex ]->m_vPos;

	pDestPos->x = FMATH_FPOT( fAnim, m_QuatPosStart.m_Pos.x, m_QuatPosEnd.m_Pos.x ) + BoundSphereDeltaPos_WS.x;
	pDestPos->z = FMATH_FPOT( fAnim, m_QuatPosStart.m_Pos.z, m_QuatPosEnd.m_Pos.z ) + BoundSphereDeltaPos_WS.z;
	pDestPos->y = fY + BoundSphereDeltaPos_WS.y;
}


// Returns TRUE if the work function needs to be called again.
// Returns FALSE if no more work is required.
void CBotZomPart::_Work_Debris( CBotZom *pBot, u32 nBoneIndex ) {
	CFMtx43A *pBoneMtx;
	const FMeshBone_t *pMeshBone;
	f32 fTimeStepSecs;

	pBoneMtx = pBot->m_pWorldMesh->GetBoneMtxPalette()[ nBoneIndex ];
	pMeshBone = &pBot->m_pWorldMesh->m_pMesh->pBoneArray[ nBoneIndex ];

	// Compute time step...
	fTimeStepSecs = FLoop_fPreviousLoopSecs;
	FMATH_CLAMPMAX( fTimeStepSecs, (1.0f / 30.0f) );

	if( !(m_nFlags & FLAG_EMP_DEBRIS_MODE) ) {
		_Work_Debris_Move( pBot, pBoneMtx, fTimeStepSecs );
	} else {
		_Work_Debris_MoveEMP( pBot, pBoneMtx, fTimeStepSecs );
	}

	// Update rotational motion...
	if( !(m_nFlags & FLAG_FLATTEN_MODE) || (m_nFlags & FLAG_EMP_DEBRIS_MODE) ) {
		if( m_fRotSpeed != 0.0f ) {
			CFQuatA OrigQuat, RotateQuat;
			f32 fHalfAngle, fSin, fCos;

			OrigQuat = m_QuatPosStart.m_Quat;

			fHalfAngle = m_fRotSpeed * fTimeStepSecs;
			fmath_SinCos( fHalfAngle, &fSin, &fCos );

			RotateQuat.x = m_UnitRotAxis_WS.x * fSin;
			RotateQuat.y = m_UnitRotAxis_WS.y * fSin;
			RotateQuat.z = m_UnitRotAxis_WS.z * fSin;
			RotateQuat.w = fCos;

			m_QuatPosStart.m_Quat.Mul( RotateQuat, OrigQuat );
		}
	} else {
		CFQuatA OrigQuat, RotateQuat;
		CFVec3A NewRotAxis, NewMtxUnitAxisX;
		f32 fDot;

		RotateQuat.BuildQuat( m_FlattenModeUnitRotAxis, m_fRotSpeed * FLoop_fPreviousLoopSecs );

		OrigQuat = m_QuatPosStart.m_Quat;
		m_QuatPosStart.m_Quat.Mul( RotateQuat, OrigQuat );

		m_QuatPosStart.m_Quat.BuildAxisX( NewMtxUnitAxisX );
		NewRotAxis.Cross( NewMtxUnitAxisX, m_FlattenModeUnitFaceNorm );

		fDot = NewRotAxis.Dot( m_FlattenModeUnitRotAxis );

		if( fDot < 0.0f ) {
			// We stepped too far...

			f32 fMagSq = pBoneMtx->m_vX.MagSq();
			if( fMagSq > 0.00001f ) {
				CFVec3A CurrentUnitAxisY;
				CurrentUnitAxisY.Mul( pBoneMtx->m_vX, fmath_InvSqrt(fMagSq) );

				RotateQuat.BuildQuat( CurrentUnitAxisY, m_FlattenModeUnitFaceNorm );
				m_QuatPosStart.m_Quat.Mul( RotateQuat, OrigQuat );
			}

			FMATH_CLEARBITMASK( m_nFlags, FLAG_FLATTEN_MODE );
			m_fRotSpeed = 0.0f;
		}
	}

	CFVec3A BoundSphereDeltaPos_BS, BoundSphereDeltaPos_WS;

	m_QuatPosStart.m_Quat.BuildMtx( *pBoneMtx );

	// Compute world space vector from bound sphere center to bone geo origin...
	BoundSphereDeltaPos_BS.Set( pMeshBone->SegmentedBoundSphere_BS.m_Pos );
	BoundSphereDeltaPos_BS.Negate();
	m_QuatPosStart.m_Quat.MulPoint( BoundSphereDeltaPos_WS, BoundSphereDeltaPos_BS );

	pBoneMtx->m_vPos.Add( m_QuatPosStart.m_Pos, BoundSphereDeltaPos_WS );

	pBoneMtx->m_vPos.y += 0.2f * pMeshBone->SegmentedBoundSphere_BS.m_fRadius;

	if( !(m_nFlags & FLAG_EMP_DEBRIS_MODE) ) {
		if( m_nMode != MODE_DONE ) {
			if( m_nConvertDebris != CONVERT_DEBRIS_NONE ) {
				// Conversion mode...

				if( m_fTimer >= 0.0f ) {
					m_fTimer -= FLoop_fPreviousLoopSecs;

					if( m_fTimer <= 0.0f ) {
						if( m_nConvertDebris == CONVERT_DEBRIS_TO_FORMING ) {
							_ConvertToFormingMode( pBoneMtx, pMeshBone, 0.0f );
						} else {
							FASSERT( m_nConvertDebris == CONVERT_DEBRIS_TO_EXPLOSION );
							_ExplodePart( pBot, pBoneMtx, pMeshBone, nBoneIndex );
						}
					}
				}
			}
		} else {
			// Debris mode done for this part...

			if( m_nConvertDebris != CONVERT_DEBRIS_NONE ) {
				// Conversion mode...

				if( m_nConvertDebris == CONVERT_DEBRIS_TO_FORMING ) {
					_ConvertToFormingMode( pBoneMtx, pMeshBone, m_fTimer );
				} else {
					FASSERT( m_nConvertDebris == CONVERT_DEBRIS_TO_EXPLOSION );
					_ExplodePart( pBot, pBoneMtx, pMeshBone, nBoneIndex );
				}
			}
		}
	}
}


// Returns TRUE if the particle has been killed.
void CBotZomPart::_Work_Debris_Move( CBotZom *pBot, CFMtx43A *pBoneMtx, f32 fTimeStepSecs ) {
	CFVec3A TempVec, PrevPos_WS;
	FCollImpact_t CollImpact;
	CFWorldTracker *pTrackerToSkip;
	u32 nLoopCount;

	// Compute new linear velocity...
	m_LinVel_WS.y += _GRAVITY * fTimeStepSecs;

	for( nLoopCount=0; nLoopCount<3; nLoopCount++ ) {
		if( fTimeStepSecs <= 0.0f ) {
			break;
		}

		// Save old position...
		PrevPos_WS = m_QuatPosStart.m_Pos;

		// Compute new position...
		TempVec.Mul( m_LinVel_WS, fTimeStepSecs );
		m_QuatPosStart.m_Pos.Add( TempVec );

		pTrackerToSkip = pBot->m_pWorldMesh;

		// xxxxxxxxx
//	if( fworld_FindClosestImpactPointToRayStart( &CollImpact, &PrevPos_WS, &m_QuatPosStart.m_Pos, 1, NULL, TRUE, NULL, -1, FCOLL_MASK_COLLIDE_WITH_DEBRIS, FCOLL_LOD_HIGHEST ) ) {
		if( fworld_FindClosestImpactPointToRayStart( &CollImpact, &PrevPos_WS, &m_QuatPosStart.m_Pos, 1, &pTrackerToSkip ) ) {
			// We hit something!

			f32 fSurfaceNormComponentOfSpeed_WS, fSpeedOnSurfacePlane2_WS, fOOSpeedOnSurfacePlane_WS, fFrictionMag;
			CFVec3A FrictionVec_WS, SurfaceNormComponentOfVel_WS, UnitDirOnSurfacePlane_WS;

			// Compute new position...
			TempVec.Set( CollImpact.UnitFaceNormal );
			TempVec.Mul( 0.001f );
			m_QuatPosStart.m_Pos.Set( CollImpact.ImpactPoint );
			m_QuatPosStart.m_Pos.Add( TempVec );

			// Flatten linear velocity to surface plane...
			fSurfaceNormComponentOfSpeed_WS = -m_LinVel_WS.Dot( CollImpact.UnitFaceNormal );
			SurfaceNormComponentOfVel_WS.Mul( CollImpact.UnitFaceNormal, fSurfaceNormComponentOfSpeed_WS );
			m_LinVel_WS.Add( SurfaceNormComponentOfVel_WS );

			// Apply friction to linear velocity...
			fSpeedOnSurfacePlane2_WS = m_LinVel_WS.MagSq();

			if( fSpeedOnSurfacePlane2_WS > 0.001f ) {
				fOOSpeedOnSurfacePlane_WS = fmath_InvSqrt( fSpeedOnSurfacePlane2_WS );
				fFrictionMag = (-2.5f*fmath_Inv( fOOSpeedOnSurfacePlane_WS ) - 60.0f) * fTimeStepSecs;

				UnitDirOnSurfacePlane_WS.Mul( m_LinVel_WS, fOOSpeedOnSurfacePlane_WS );
				FrictionVec_WS.Mul( UnitDirOnSurfacePlane_WS, fFrictionMag );

				if( fFrictionMag*fFrictionMag < fSpeedOnSurfacePlane2_WS ) {
					m_LinVel_WS.Add( FrictionVec_WS );
				} else {
					m_LinVel_WS.Zero();
				}
			} else {
				// Our linear velocity has come to rest...

				m_LinVel_WS.Zero();
			}

			FMATH_SETBITMASK( m_nFlags, FLAG_FLATTEN_MODE );

			// Add a little bounce to linear velocity...
			if( fSurfaceNormComponentOfSpeed_WS > 15.0f ) {
				FMATH_CLEARBITMASK( m_nFlags, FLAG_FLATTEN_MODE );

				SurfaceNormComponentOfVel_WS.Mul( 0.25f + fmath_RandomFloatRange( -0.08f, 0.08f ) );
				m_LinVel_WS.Add( SurfaceNormComponentOfVel_WS );

				if( m_fRotSpeed < 2.0f ) {
					m_fRotSpeed += m_fRotSpeed * fmath_RandomFloatRange( 0.5f, 1.0f );
				}

				if( !(m_nFlags & FLAG_PLAYED_DEBRIS_IMPACT_SOUND) ) {
					if( CollImpact.UnitFaceNormal.y > 0.5f ) {
						FMATH_SETBITMASK( m_nFlags, FLAG_PLAYED_DEBRIS_IMPACT_SOUND );
						pBot->_PlayClatterSound( &m_QuatPosStart.m_Pos, 0.5f );
					}
				}
			}

			if( fSurfaceNormComponentOfSpeed_WS > 15.0f ) {
				m_UnitRotAxis_WS.SetUnitRandom();
			}

			// Apply friction to rotational velocity...

			FASSERT( m_fRotSpeed >= 0 );

			m_fRotSpeed -= 40.0f * fTimeStepSecs;
			FMATH_CLAMPMIN( m_fRotSpeed, 0.0f );

			if( m_nFlags & FLAG_FLATTEN_MODE ) {
				if( pBoneMtx->m_vX.Dot( CollImpact.UnitFaceNormal ) >= 0.0f ) {
					m_FlattenModeUnitFaceNorm = CollImpact.UnitFaceNormal;
				} else {
					m_FlattenModeUnitFaceNorm.ReceiveNegative( CollImpact.UnitFaceNormal );
				}

				m_FlattenModeUnitRotAxis.Cross( pBoneMtx->m_vX, m_FlattenModeUnitFaceNorm );

				f32 fMag2 = m_FlattenModeUnitRotAxis.MagSq();

				if( fMag2 > 0.00001f ) {
					m_FlattenModeUnitRotAxis.Mul( fmath_InvSqrt(fMag2) );
					m_fRotSpeed = FMATH_2PI * 2.0f;
				} else {
					FMATH_CLEARBITMASK( m_nFlags, FLAG_FLATTEN_MODE );
				}
			}

			fTimeStepSecs -= fTimeStepSecs * CollImpact.fImpactDistInfo;
		} else {
			fTimeStepSecs = 0.0f;
		}
	}

	f32 fLinVelMag2 = m_LinVel_WS.MagSq();

	if( (fLinVelMag2 <= 2.0f) || (fLinVelMag2 >= 10000.0f) ) {
		// We're nearly stopped or we're going too fast...

		m_fSpeedMult += FLoop_fPreviousLoopSecs;

		if( m_fSpeedMult >= 2.0f ) {
			// Consider ourselves stopped...
			m_nMode = MODE_DONE;
			FMATH_CLEARBITMASK( m_nFlags, FLAG_ROUTE_ANIM_TO_HIDDEN_PAL );
		}
	} else {
		m_fSpeedMult = 0.0f;
	}
}


// Returns TRUE if the particle has been killed.
void CBotZomPart::_Work_Debris_MoveEMP( CBotZom *pBot, CFMtx43A *pBoneMtx, f32 fTimeStepSecs ) {
	CFVec3A TempVec, PrevPos_WS;
	FCollImpact_t CollImpact;
	CFWorldTracker *pTrackerToSkip;
	f32 fSpeed2, fSpeed;

	// Save old position...
	PrevPos_WS = m_QuatPosStart.m_Pos;

	// Reduce velocity...
	fSpeed2 = m_LinVel_WS.MagSq();
	if( fSpeed2 > 0.001f ) {
		// Compute speed and unit velocity vector...
		fSpeed = fmath_InvSqrt(fSpeed2);
		m_LinVel_WS.Mul( fSpeed );
		fSpeed = fmath_Inv(fSpeed);

		fSpeed -= 5.0f * FLoop_fPreviousLoopSecs;

		if( fSpeed > 0.001f ) {
			m_LinVel_WS.Mul( fSpeed );
		} else {
			m_LinVel_WS.Zero();
		}
	} else {
		m_LinVel_WS.Zero();
	}

	// Compute new position...
	TempVec.Mul( m_LinVel_WS, fTimeStepSecs );
	m_QuatPosStart.m_Pos.Add( TempVec );

	pTrackerToSkip = pBot->m_pWorldMesh;

	// xxxxxxxxx
//	if( fworld_FindClosestImpactPointToRayStart( &CollImpact, &PrevPos_WS, &m_QuatPosStart.m_Pos, 1, NULL, TRUE, NULL, -1, FCOLL_MASK_COLLIDE_WITH_DEBRIS, FCOLL_LOD_HIGHEST ) ) {
	if( fworld_FindClosestImpactPointToRayStart( &CollImpact, &PrevPos_WS, &m_QuatPosStart.m_Pos, 1, &pTrackerToSkip ) ) {
		// We hit something!

		// Compute new position...
		TempVec.Set( CollImpact.UnitFaceNormal );
		TempVec.Mul( 0.001f );
		m_QuatPosStart.m_Pos.Set( CollImpact.ImpactPoint );
		m_QuatPosStart.m_Pos.Add( TempVec );

		// Reflect velocity around face normal...
		m_LinVel_WS.Reflect( CollImpact.UnitFaceNormal );
	}
}


void CBotZomPart::_ConvertToFormingMode( CFMtx43A *pBoneMtxPalette, const FMeshBone_t *pMeshBone, f32 fSecsToWaitBeforeMoving ) {
	CFVec3A Pos;

	m_nMode = MODE_FORMING;
	m_nFlags = FLAG_ROUTE_ANIM_TO_HIDDEN_PAL;

	if( fSecsToWaitBeforeMoving > 0.0f ) {
		// Wait before moving...

		m_fTimer = fSecsToWaitBeforeMoving;
		FMATH_SETBITMASK( m_nFlags, FLAG_WAITING_BEFORE_FORMING );
	} else {
		// Start moving immediately...

		m_fTimer = 0.0f;
	}

	m_fSpeedMult = fmath_RandomFloatRange( 0.5f, 1.0f );
	m_nScaledHeight = (u8)fmath_RandomChoice( 256 );

	m_QuatPosStart.m_Quat.BuildQuat( *pBoneMtxPalette );
	Pos.Set( pMeshBone->SegmentedBoundSphere_BS.m_Pos );
	pBoneMtxPalette->MulPoint( m_QuatPosStart.m_Pos, Pos );
}


void CBotZomPart::_Work_StagedDebris( CBotZom *pBot, u32 nBoneIndex ) {
	f32 fLimboBarY;

	m_fTimer -= m_fSpeedMult * FLoop_fPreviousLoopSecs;

	fLimboBarY = pBot->m_pWorldMesh->m_Xfm.m_MtxF.m_vPos.y + m_fTimer;

	if( (m_fTimer <= 0.0f) || (fLimboBarY <= pBot->m_pWorldMesh->GetBoneMtxPalette()[ nBoneIndex ]->m_vPos.y) ) {
		CFMtx43A *pMtxPalette;
		CFVec3A Pos;

		m_nMode = MODE_DEBRIS;
		m_nConvertDebris = CBotZomPart::CONVERT_DEBRIS_NONE;
		m_nFlags = FLAG_ROUTE_ANIM_TO_HIDDEN_PAL;
		m_fSpeedMult = 0.0f;

		pMtxPalette = pBot->m_pWorldMesh->GetBoneMtxPalette()[ nBoneIndex ];

		m_QuatPosStart.m_Quat.BuildQuat( *pMtxPalette );

		Pos.Set( pBot->m_pWorldMesh->m_pMesh->pBoneArray[ nBoneIndex ].SegmentedBoundSphere_BS.m_Pos );
		pMtxPalette->MulPoint( m_QuatPosStart.m_Pos, Pos );
	}
}


void CBotZomPart::_Work_Recoiling( CBotZom *pBot, u32 nBoneIndex ) {
	CFQuatA OrigQuat, RotateQuat;
	CFVec3A BoundSphereDeltaPos_BS, BoundSphereDeltaPos_WS, TempVec;
	CFMtx43A *pBoneMtx;
	f32 fHalfAngle, fSin, fCos;

	// Advance timer...
	if( m_fSpeedMult > 0.0f ) {
		m_fSpeedMult -= FLoop_fPreviousLoopSecs;
		if( m_fSpeedMult <= 0.0f ) {
			// Time to start blending out the debris animation...

			m_fSpeedMult = 0.0f;
		}
	} else {
		// Advance blend-out animation...

		m_fTimer += 2.5f * FLoop_fPreviousLoopSecs;
		if( m_fTimer >= 1.0f ) {
			// We're done moving the part...

			m_fTimer = 1.0f;

			m_nMode = MODE_DONE;
			FMATH_CLEARBITMASK( m_nFlags, FLAG_ROUTE_ANIM_TO_HIDDEN_PAL );

			if( !(m_nFlags & FLAG_PLAY_SPARSED_CLATTER_SOUND) || ((nBoneIndex & 7) == 0) ) {
				pBot->_PlayClatterSound( &m_QuatPosStart.m_Pos, 0.5f );
			}
		}
	}

	pBoneMtx = pBot->m_pWorldMesh->GetBoneMtxPalette()[ nBoneIndex ];

	// Compute new linear velocity...
	m_LinVel_WS.y += _GRAVITY * FLoop_fPreviousLoopSecs;

	// Compute new position...
	TempVec.Mul( m_LinVel_WS, FLoop_fPreviousLoopSecs );
	m_QuatPosStart.m_Pos.Add( TempVec );

	// Update rotational motion...
	OrigQuat = m_QuatPosStart.m_Quat;

	fHalfAngle = m_fRotSpeed * FLoop_fPreviousLoopSecs;
	fmath_SinCos( fHalfAngle, &fSin, &fCos );

	RotateQuat.x = m_UnitRotAxis_WS.x * fSin;
	RotateQuat.y = m_UnitRotAxis_WS.y * fSin;
	RotateQuat.z = m_UnitRotAxis_WS.z * fSin;
	RotateQuat.w = fCos;

	if( m_fTimer == 0.0f ) {
		// No blending yet...

		m_QuatPosStart.m_Quat.Mul( RotateQuat, OrigQuat );
	} else {
		// Blend between current quaternion and the end quaternion...

		CFQuatA NewQuat;

		NewQuat.Mul( RotateQuat, OrigQuat );

		m_QuatPosStart.m_Quat.ReceiveSlerpOf( m_fTimer, RotateQuat, m_QuatPosEnd.m_Quat );
		m_QuatPosStart.m_Pos.Lerp( m_fTimer, m_QuatPosStart.m_Pos, m_QuatPosEnd.m_Pos );
	}

	m_QuatPosStart.m_Quat.BuildMtx( *pBoneMtx );

	// Compute world space vector from bound sphere center to bone geo origin...
	BoundSphereDeltaPos_BS.Set( pBot->m_pWorldMesh->m_pMesh->pBoneArray[ nBoneIndex ].SegmentedBoundSphere_BS.m_Pos );
	BoundSphereDeltaPos_BS.Negate();
	m_QuatPosStart.m_Quat.MulPoint( BoundSphereDeltaPos_WS, BoundSphereDeltaPos_BS );

	pBoneMtx->m_vPos.Add( m_QuatPosStart.m_Pos, BoundSphereDeltaPos_WS );
}


void CBotZomPart::_ExplodePart( CBotZom *pBot, CFMtx43A *pBoneMtx, const FMeshBone_t *pMeshBone, u32 nBoneIndex ) {
	CFVec3A BoneSphereCenter_WS, SpawnPos_WS;

	m_nMode = MODE_DONE;
	m_nFlags = FLAG_NONE;

	BoneSphereCenter_WS.Set( pMeshBone->SegmentedBoundSphere_BS.m_Pos );
	pBoneMtx->MulPoint( SpawnPos_WS, BoneSphereCenter_WS );

	if( (nBoneIndex & 3) == 0 ) {
		if( pBot->m_BotInfo_Zom.hPartExplosion != FEXPLOSION_INVALID_HANDLE ) {
			FExplosion_SpawnerHandle_t hSpawner = CExplosion2::GetExplosionSpawner();

			if( hSpawner != FEXPLOSION_INVALID_HANDLE ) {
				FExplosionSpawnParams_t SpawnParams;

				SpawnParams.InitToDefaults();

				SpawnParams.uFlags = FEXPLOSION_SPAWN_NONE;
				SpawnParams.Pos_WS = SpawnPos_WS;
				SpawnParams.UnitDir = CFVec3A::m_UnitAxisY;
				SpawnParams.uSurfaceType = 0;
				SpawnParams.pDamageProfile = NULL;
				SpawnParams.pDamager = NULL;

				CExplosion2::SpawnExplosion( hSpawner, pBot->m_BotInfo_Zom.hPartExplosion, &SpawnParams );
			}
		}
	} else {
		CFDebrisSpawner DebrisSpawner;

		DebrisSpawner.InitToDefaults();
		DebrisSpawner.m_Mtx.m_vPos = SpawnPos_WS;
		DebrisSpawner.m_Mtx.m_vZ = CFVec3A::m_UnitAxisY;

		// Generate debris chunks...
		if( pBot->m_pBotInfo_Gen->pDebrisGroup_Chunks ) {
			DebrisSpawner.m_pDebrisGroup = pBot->m_pBotInfo_Gen->pDebrisGroup_Chunks;
			DebrisSpawner.m_fMinSpeed = pBot->m_pBotInfo_Gen->fMinChunksDebrisSpeed;
			DebrisSpawner.m_fMaxSpeed = pBot->m_pBotInfo_Gen->fMaxChunksDebrisSpeed;
			DebrisSpawner.m_fUnitDirSpread = pBot->m_pBotInfo_Gen->fUnitChunksSpread;
			DebrisSpawner.m_fScaleMul = 1.0f;
			DebrisSpawner.m_nMinDebrisCount = 2;
			DebrisSpawner.m_nMaxDebrisCount = 2;

			DebrisSpawner.Spawn();
		}

		// Generate debris guts...
		if( pBot->m_pBotInfo_Gen->pDebrisGroup_Guts ) {
			DebrisSpawner.m_pDebrisGroup = pBot->m_pBotInfo_Gen->pDebrisGroup_Guts;
			DebrisSpawner.m_fMinSpeed = pBot->m_pBotInfo_Gen->fMinGutsDebrisSpeed;
			DebrisSpawner.m_fMaxSpeed = pBot->m_pBotInfo_Gen->fMaxGutsDebrisSpeed;
			DebrisSpawner.m_fUnitDirSpread = pBot->m_pBotInfo_Gen->fUnitGutsSpread;
			DebrisSpawner.m_fScaleMul = 1.0f;
			DebrisSpawner.m_nMinDebrisCount = 3;
			DebrisSpawner.m_nMaxDebrisCount = 3;

			DebrisSpawner.Spawn();
		}
	}

	pBoneMtx->Identity33();
	pBoneMtx->m_vPos.Set( 2000000.0f, 2000000.0f, 2000000.0f );
}



