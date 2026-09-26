//////////////////////////////////////////////////////////////////////////////////////
// botpart.cpp - Bot part module.
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
// 01/02/03 Ranck       Created.
//////////////////////////////////////////////////////////////////////////////////////
#include "fang.h"
#include "botpart.h"
#include "fverlet.h"
#include "bot.h"
#include "gstring.h"
#include "damage.h"
#include "fgamedata.h"
#include "fparticle.h"
#include "fdebris.h"
#include "fresload.h"
#include "fclib.h"
#include "eparticle.h"
#include "eshield.h"
#include "fworld_coll.h"
#include "eparticlepool.h"
#include "debristypes.h"
#include "weapon.h"
#include "MultiplayerMgr.h"
#include "fperf.h"


#define _LIMB_INFO_TABLE_NAME	"LimbInfo"
#define _BONE_INFO_TABLE_NAME	"BoneInfo"

#define _MIN_SECS_BETWEEN_LIMB_BLOW_SOUNDS		0.15f
#define _MIN_SECS_BETWEEN_DEBRIS_BLOW_SOUNDS	0.15f

#define _LIMB_REPAIR_TIME						( 0.25f )
#define _LIMB_OO_REPAIR_TIME					( 1.0f/_LIMB_REPAIR_TIME )


#if FANG_PLATFORM_DX
	#define _DEFAULT_DEBRIS_UNIT_MOD		1.f
#else
	#define _DEFAULT_DEBRIS_UNIT_MOD		0.5f
#endif

static f32 _fGlobalDebrisUnitMod = _DEFAULT_DEBRIS_UNIT_MOD;

//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CBotPart
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

CBotPart *CBotPart::CreateArrayFromGameData( cchar *pszTableName, FGameDataFileHandle_t hGameDataFile, const FMesh_t *pMesh, u32 nInstanceCount ) {
	CBotPartInit BotPartInit;

	if( !BotPartInit.LoadFromGameData( pszTableName, hGameDataFile ) ) {
		return NULL;
	}

	return CreateArray( pszTableName, &BotPartInit, pMesh, nInstanceCount );
}


CBotPart *CBotPart::CreateArrayFromGameData( FGameDataTableHandle_t hTable, cchar *pszFileName, const FMesh_t *pMesh, u32 nInstanceCount ) {
	CBotPartInit BotPartInit;

	if( !BotPartInit.LoadFromGameData( hTable, pszFileName ) ) {
		return NULL;
	}

	return CreateArray( fgamedata_GetTableName( hTable ), &BotPartInit, pMesh, nInstanceCount );
}


CBotPart *CBotPart::CreateArray( cchar *pszPartName, const CBotPartInit *pBotPartInit, const FMesh_t *pMesh, u32 nInstanceCount ) {
	CBotPart *pBotPartArray = NULL;
	u32 i;

	FResFrame_t ResFrame = fres_GetFrame();

	pBotPartArray = fnew CBotPart[nInstanceCount];
	if( pBotPartArray == NULL ) {
		DEVPRINTF( "CBotPart::CreateArray(): Could not create array of %u bot parts.\n", nInstanceCount );
		goto _ExitWithError;
	}

	for( i=0; i<nInstanceCount; ++i ) {
		if( !pBotPartArray[i].Create( pszPartName, pBotPartInit, pMesh ) ) {
			goto _ExitWithError;
		}
	}

	return pBotPartArray;

_ExitWithError:
	if( pBotPartArray ) {
		for( i=0; i<nInstanceCount; ++i ) {
			pBotPartArray[i].Destroy();
		}
	}

	fres_ReleaseFrame( ResFrame );

	return NULL;
}


BOOL CBotPart::Create( cchar *pszPartName, const CBotPartInit *pBotPartInit, const FMesh_t *pMesh ) {
	FASSERT( !IsCreated() );
	FASSERT( pszPartName );
	FASSERT( pBotPartInit );
	FASSERT( pMesh );
	FASSERT( pMesh->nBoneCount < 128 );

	//CPS 4.7.03 -->
	u32 i, nTackSurfaceIndex;
	s32 nTackIndex;
	s8 nBoneIndex;
	BOOL bTooManyTackSurfaces = FALSE;
	const CBotPartInit::CollPlane_t *pCollPlane;
	CFVerletTackSurface::PlaneNormal_e nPlaneNormal;
	CFVerletTackSurface::AnchorType_e nAnchorType;
	//<-- CPS 4.7.03
	
	CFVerlet::Init2_t Init2;

	cchar *apszBoneNameParts[BOTPART_MAX_BONES_PER_PART+1];
	cchar *apszBoneNameTacks[BOTPART_MAX_TACKS_PER_PART+1];
	cchar *apszBoneNameCTacks[BOTPART_MAX_CTACKS_PER_PART+1];
	cchar *apszBoneNameBTacks[BOTPART_MAX_BTACKS_PER_PART+1];

	FResFrame_t ResFrame = fres_GetFrame();

	m_nMode = MODE_DISABLED;
	m_fUnitRepairTimer = 0.0f;
	m_DanglingBoneAnimDrivenPos_WS.Zero();
	m_pDanglingFromBot = NULL;
	m_nMeshPartsMask = pBotPartInit->m_nMeshPartsMask;

	m_pTackSurfaceDefArray = NULL;
	m_pTackSurfaceArray = NULL;
	m_ppTackSurfaceArray = NULL;
	m_pAnchorDefArray = NULL;

	m_pszPartName = gstring_Main.AddString( pszPartName );
	if( m_pszPartName == NULL ) {
		DEVPRINTF( "CBotPart::Create(): Not enough memory to add part name '%s' to main string table.\n", pszPartName );
		goto _ExitWithError;
	}

	m_pszLimbName = gstring_Main.AddString( pBotPartInit->m_pszLimbName );
	if( m_pszLimbName == NULL ) {
		DEVPRINTF( "CBotPart::Create(): Not enough memory to add limb name '%s' to main string table.\n", m_pszLimbName );
		goto _ExitWithError;
	}

	u32 nCTackGameDataArrayCount;

	_FillStringTable( apszBoneNameParts, pBotPartInit->m_apszBoneNameParts, BOTPART_MAX_BONES_PER_PART, TRUE );
	_FillStringTable( apszBoneNameTacks, pBotPartInit->m_apszBoneNameTacks, BOTPART_MAX_TACKS_PER_PART, TRUE );
	_FillStringTable( apszBoneNameBTacks, pBotPartInit->m_apszBoneNameBTacks, BOTPART_MAX_BTACKS_PER_PART, TRUE );

	nCTackGameDataArrayCount = _FillStringTable( apszBoneNameCTacks, pBotPartInit->m_apszBoneNameCTacks, BOTPART_MAX_CTACKS_PER_PART, TRUE );

	Init2.fMass = pBotPartInit->m_fMass;
	Init2.fDimX = pBotPartInit->m_fDimX;
	Init2.fDimY = pBotPartInit->m_fDimY;
	Init2.fDimZ = pBotPartInit->m_fDimZ;
	Init2.apszBoneNames = apszBoneNameParts;
	Init2.apszBoneNameTacks = apszBoneNameTacks;
	Init2.apszBoneNameCTacks = apszBoneNameCTacks;

	if( !m_Verlet.Create( &Init2, pMesh ) ) {
		goto _ExitWithError;
	}

	m_Verlet.SetManualWork();
	m_Verlet.SetDampening( pBotPartInit->m_fDamping );
	m_Verlet.AttachedToWorldMesh( NULL );
	m_Verlet.SetTackSurfaceArray( NULL, NULL );

	if( pBotPartInit->m_bCTackRayTest && (nCTackGameDataArrayCount >= 2) ) {
		m_nRayTestStartVerletTackIndex = m_Verlet.Tack_FindIndexByName( apszBoneNameCTacks[0] );
		m_nRayTestEndVerletTackIndex = m_Verlet.Tack_FindIndexByName( apszBoneNameCTacks[1] );

		FASSERT( m_nRayTestStartVerletTackIndex >= 0 );
		FASSERT( m_nRayTestEndVerletTackIndex >= 0 );

		m_Verlet.SetConstraintCallback( _VerletConstraintCallback );
	} else {
		m_nRayTestStartVerletTackIndex = -1;
		m_nRayTestEndVerletTackIndex = -1;

		m_Verlet.SetConstraintCallback( NULL );
	}

	m_Verlet.m_pUser = this;

	/* CPS 4.7.03 -->
	u32 i, nTackSurfaceIndex;
	s32 nTackIndex;
	s8 nBoneIndex;
	BOOL bTooManyTackSurfaces = FALSE;
	const CBotPartInit::CollPlane_t *pCollPlane;
	CFVerletTackSurface::PlaneNormal_e nPlaneNormal;
	CFVerletTackSurface::AnchorType_e nAnchorType;
	<-- CPS 4.7.03 */

	// Set up blast tack bone index array...
	m_nBlastTackCount = 0;
	for( i=0; i<BOTPART_MAX_BTACKS_PER_PART; ++i ) {
		if( pBotPartInit->m_apszBoneNameBTacks[i] ) {
			nTackIndex = m_Verlet.Tack_FindIndexByName( pBotPartInit->m_apszBoneNameBTacks[i] );

			if( nTackIndex == -1 ) {
				DEVPRINTF( "CBotPart::Create(): Blast tack name '%s' not found in tack list or coll tack list.\n", pBotPartInit->m_apszBoneNameBTacks[i] );
				goto _ExitWithError;
			} else {
				++m_nBlastTackCount;
			}
		}
	}

	if( m_nBlastTackCount ) {
		m_pnBlastTackIndexArray = (u8 *)fres_Alloc( sizeof(u8 *) * m_nBlastTackCount );

		if( m_pnBlastTackIndexArray == NULL ) {
			DEVPRINTF( "CBotPart::Create(): Not enough memory to allocate m_pnBlastTackIndexArray.\n" );
			goto _ExitWithError;
		}

		for( i=0; i<m_nBlastTackCount; ++i ) {
			if( pBotPartInit->m_apszBoneNameBTacks[i] ) {
				nTackIndex = m_Verlet.Tack_FindIndexByName( pBotPartInit->m_apszBoneNameBTacks[i] );
				FASSERT( nTackIndex >= 0 );
				FASSERT( nTackIndex < 128 );

				m_pnBlastTackIndexArray[i] = nTackIndex;
			}
		}
	}

	// Count tack collision planes...
	m_nTackSurfaceCount = 0;
	for( i=0; i<BOTPART_MAX_CPLANES_PER_PART; ++i ) {
		pCollPlane = pBotPartInit->m_aCollPlanes + i;

		if( pCollPlane->pszBoneName ) {
			nBoneIndex = fmesh_FindBone( pMesh, pCollPlane->pszBoneName );

			if( nBoneIndex < 0 ) {
				DEVPRINTF( "CBotPart::Create(): Bone name '%s' not found in mesh.\n", pCollPlane->pszBoneName );
				goto _ExitWithError;
			} else {
				// Valid bone name...

				if( m_nTackSurfaceCount >= 255 ) {
					DEVPRINTF( "CBotPart::Create(): Too many tack surfaces. Limited to 255.\n" );
					bTooManyTackSurfaces = TRUE;
					break;
				}

				if( pCollPlane->nAxisCode > 5 ) {
					DEVPRINTF( "CBotPart::Create(): Invalid coll plane axis type '%u'.\n", pCollPlane->nAxisCode );
					goto _ExitWithError;
				}

				if( pCollPlane->nAnchorTypeCode > 1 ) {
					DEVPRINTF( "CBotPart::Create(): Invalid coll plane anchor type '%u'.\n", pCollPlane->nAnchorTypeCode );
					goto _ExitWithError;
				}

				++m_nTackSurfaceCount;
			}

			if( bTooManyTackSurfaces ) {
				break;
			}
		}
	}

	if( m_nTackSurfaceCount ) {
		// At least one collision plane was specified...

		// Allocate tack surface definition array...
		m_pTackSurfaceDefArray = (TackSurfaceDef_t *)fres_Alloc( sizeof(TackSurfaceDef_t) * m_nTackSurfaceCount );
		if( m_pTackSurfaceDefArray == NULL ) {
			DEVPRINTF( "CBotPart::Create(): Not enough memory to allocate m_pTackSurfaceDefArray.\n" );
			goto _ExitWithError;
		}

		// Allocate tack surface pointer array...
		m_ppTackSurfaceArray = (CFVerletTackSurface **)fres_Alloc( sizeof(CFVerletTackSurface *) * m_nTackSurfaceCount );
		if( m_ppTackSurfaceArray == NULL ) {
			DEVPRINTF( "CBotPart::Create(): Not enough memory to allocate m_ppTackSurfaceArray.\n" );
			goto _ExitWithError;
		}

		// Allocate tack surface array...
		m_pTackSurfaceArray = fnew CFVerletTackSurface[m_nTackSurfaceCount];
		if( m_pTackSurfaceArray == NULL ) {
			DEVPRINTF( "CBotPart::Create(): Not enough memory to allocate pTackSurfaceArray.\n" );
			goto _ExitWithError;
		}

		TackSurfaceDef_t *pTackSurfaceDef;
		CFVerletTackSurface *pTackSurface;

		nTackSurfaceIndex = 0;
		for( i=0; i<BOTPART_MAX_CPLANES_PER_PART; ++i ) {
			pCollPlane = pBotPartInit->m_aCollPlanes + i;

			if( pCollPlane->pszBoneName ) {
				nBoneIndex = fmesh_FindBone( pMesh, pCollPlane->pszBoneName );

				if( nBoneIndex >= 0 ) {
					FASSERT( nTackSurfaceIndex < m_nTackSurfaceCount );

					pTackSurfaceDef = m_pTackSurfaceDefArray + nTackSurfaceIndex;
					pTackSurface = m_pTackSurfaceArray + nTackSurfaceIndex;

					m_ppTackSurfaceArray[nTackSurfaceIndex] = pTackSurface;

					switch( pCollPlane->nAxisCode ) {
					case 0:
						nPlaneNormal = CFVerletTackSurface::PLANE_NORMAL_X;
						break;

					case 1:
						nPlaneNormal = CFVerletTackSurface::PLANE_NORMAL_Y;
						break;

					case 2:
						nPlaneNormal = CFVerletTackSurface::PLANE_NORMAL_Z;
						break;

					case 3:
						nPlaneNormal = CFVerletTackSurface::PLANE_NORMAL_NX;
						break;

					case 4:
						nPlaneNormal = CFVerletTackSurface::PLANE_NORMAL_NY;
						break;

					case 5:
						nPlaneNormal = CFVerletTackSurface::PLANE_NORMAL_NZ;
						break;

					default:
						FASSERT_NOW;
					};

					switch( pCollPlane->nAnchorTypeCode ) {
					case 0:
						nAnchorType = CFVerletTackSurface::ANCHOR_TYPE_DYNAMIC;
						break;

					case 1:
						nAnchorType = CFVerletTackSurface::ANCHOR_TYPE_BONED;
						break;

					default:
						FASSERT_NOW;
					};

					pTackSurfaceDef->nBoneIndex = nBoneIndex;
					pTackSurfaceDef->nAnchorType = nAnchorType;
					pTackSurfaceDef->nPlaneNormalAxis = nPlaneNormal;
					pTackSurfaceDef->fFriction = pCollPlane->fFriction;
					pTackSurfaceDef->fDistFromPlane = pCollPlane->fDistFromPlane;

					if( pCollPlane->bPreserveVelocity ) {
						FMATH_SETBITMASK( pTackSurfaceDef->nFlags, TACK_SURFACE_FLAG_PRESERVE_VELOCITY );
					} else {
						FMATH_CLEARBITMASK( pTackSurfaceDef->nFlags, TACK_SURFACE_FLAG_PRESERVE_VELOCITY );
					}

					++nTackSurfaceIndex;
				}
			}
		}

		FASSERT( nTackSurfaceIndex == m_nTackSurfaceCount );
	}

	//-----------------------------------------------------------------------------

	// Count the anchors...
	u32 nAnchorIndex;
	const CBotPartInit::Anchor_t *pAnchorInit;
	CFVerletTack *pTack;

	m_nAnchorCount = 0;
	for( i=0; i<BOTPART_MAX_ANCHORS; ++i ) {
		pAnchorInit = pBotPartInit->m_aAnchors + i;

		if( pAnchorInit->pszTackName ) {
			if( m_Verlet.Tack_FindByName( pAnchorInit->pszTackName ) ) {
				// Valid tack name...

				if( m_nAnchorCount >= 255 ) {
					DEVPRINTF( "CBotPart::Create(): Too many anchors. Limited to 255.\n" );
					goto _ExitWithError;
				}

				if( pAnchorInit->nAttachTypeCode > 2 ) {
					DEVPRINTF( "CBotPart::Create(): Invalid tack attach type '%u'.\n", pAnchorInit->nAttachTypeCode );
					goto _ExitWithError;
				}

				if( pAnchorInit->pszAnchorBoneName == NULL ) {
					DEVPRINTF( "CBotPart::Create(): Anchor bone name was not specified.\n" );
					goto _ExitWithError;
				}

				switch( pAnchorInit->nAttachTypeCode ) {
				case 0:
					// Anim-driven:
					if( pAnchorInit->pszAnchorParentBoneName ) {
						DEVPRINTF( "CBotPart::Create(): Do not specify a parent bone name ('%u') for attach code 0.\n", pAnchorInit->pszAnchorParentBoneName );
						goto _ExitWithError;
					}

					break;

				case 1:
					// Rest position:
					break;

				case 2:
					// Part:
					if( pAnchorInit->pszAnchorParentBoneName == NULL ) {
						DEVPRINTF( "CBotPart::Create(): Parent bone name required for attach code 2.\n" );
						goto _ExitWithError;
					}

					if( !fclib_stricmp( pAnchorInit->pszAnchorParentBoneName, pszPartName ) ) {
						DEVPRINTF( "CBotPart::Create(): Not allowed to anchor to same part '%s'.\n", pszPartName );
						goto _ExitWithError;
					}

					break;

				default:
					FASSERT_NOW;
				};

				++m_nAnchorCount;
			} else {
				DEVPRINTF( "CBotPart::Create(): Tack name '%s' not found.\n", pAnchorInit->pszTackName );
				goto _ExitWithError;
			}
		}
	}

	if( m_nAnchorCount ) {
		// At least one anchor was specified...

		// Allocate tack surface definition array...
		m_pAnchorDefArray = (AnchorDef_t *)fres_Alloc( sizeof(AnchorDef_t) * m_nAnchorCount );
		if( m_pAnchorDefArray == NULL ) {
			DEVPRINTF( "CBotPart::Create(): Not enough memory to allocate m_pAnchorDefArray.\n" );
			goto _ExitWithError;
		}

		AnchorDef_t *pAnchorDef;

		nAnchorIndex = 0;
		for( i=0; i<BOTPART_MAX_ANCHORS; ++i ) {
			pAnchorInit = pBotPartInit->m_aAnchors + i;

			if( pAnchorInit->pszTackName ) {
				pTack = m_Verlet.Tack_FindByName( pAnchorInit->pszTackName );
				if( pTack == NULL ) {
					continue;
				}

				FASSERT( nAnchorIndex < m_nAnchorCount );

				pAnchorDef = m_pAnchorDefArray + nAnchorIndex;

				pAnchorDef->pTack = pTack;
				pAnchorDef->fMinDist = pAnchorInit->fMinDist;
				pAnchorDef->fMaxDist = pAnchorInit->fMaxDist;

				switch( pAnchorInit->nAttachTypeCode ) {
				case 0:
					// Anim-driven:

					pAnchorDef->pszAnchorPartName = NULL;
					pAnchorDef->nParentBoneIndex = 0;

					nBoneIndex = fmesh_FindBone( pMesh, pAnchorInit->pszAnchorBoneName );
					if( nBoneIndex >= 0 ) {
						pAnchorDef->nAnchorBoneIndex = nBoneIndex;
					} else {
						DEVPRINTF( "CBotPart::Create(): Bone name '%s' not found in mesh.\n", pAnchorInit->pszAnchorBoneName );
						goto _ExitWithError;
					}

					if( fclib_stricmp( pAnchorInit->pszAnchorBoneName, apszBoneNameParts[0] ) ) {
						// Anchor bone name differs from our base bone name...
						pAnchorDef->nAttachType = CBotPart::ATTACH_TYPE_DYNAMIC_MTX_PALETTE;
					} else {
						// Anchor bone name is the same as our base bone name...
						pAnchorDef->nAttachType = CBotPart::ATTACH_TYPE_DYNAMIC_ANIM_OUTPUT;
					}

					break;

				case 1:
					// Rest position:

					pAnchorDef->pszAnchorPartName = NULL;

					nBoneIndex = fmesh_FindBone( pMesh, pAnchorInit->pszAnchorBoneName );
					if( nBoneIndex >= 0 ) {
						pAnchorDef->nAnchorBoneIndex = nBoneIndex;
					} else {
						DEVPRINTF( "CBotPart::Create(): Bone name '%s' not found in mesh.\n", pAnchorInit->pszAnchorBoneName );
						goto _ExitWithError;
					}

					if( pAnchorInit->pszAnchorParentBoneName ) {
						// Parent bone name provided...

						nBoneIndex = fmesh_FindBone( pMesh, pAnchorInit->pszAnchorParentBoneName );
						if( nBoneIndex >= 0 ) {
							pAnchorDef->nParentBoneIndex = nBoneIndex;
						} else {
							DEVPRINTF( "CBotPart::Create(): Bone name '%s' not found in mesh.\n", pAnchorInit->pszAnchorParentBoneName );
							goto _ExitWithError;
						}
					} else {
						// Parent bone name not provided...

						nBoneIndex = pMesh->pBoneArray[pAnchorDef->nAnchorBoneIndex].Skeleton.nParentBoneIndex;

						if( nBoneIndex != 255 ) {
							// Anchor bone has a parent...
							pAnchorDef->nParentBoneIndex = nBoneIndex;
						} else {
							// Anchor bone has no parent...
							pAnchorDef->nParentBoneIndex = 255;
						}
					}

					pAnchorDef->nAttachType = ATTACH_TYPE_BONED;

					break;

				case 2:
					// Part:

					nBoneIndex = fmesh_FindBone( pMesh, pAnchorInit->pszAnchorBoneName );
					if( nBoneIndex >= 0 ) {
						pAnchorDef->nAnchorBoneIndex = nBoneIndex;
					} else {
						DEVPRINTF( "CBotPart::Create(): Bone name '%s' not found in mesh.\n", pAnchorInit->pszAnchorBoneName );
						goto _ExitWithError;
					}

					pAnchorDef->pszAnchorPartName = gstring_Main.AddString( pAnchorInit->pszAnchorParentBoneName );
					if( pAnchorDef->pszAnchorPartName == NULL ) {
						DEVPRINTF( "CBotPart::Create(): Not enough memory to add '%s' to main string table.\n", pAnchorInit->pszAnchorParentBoneName );
						goto _ExitWithError;
					}

					pAnchorDef->nParentBoneIndex = 0;
					pAnchorDef->nAttachType = ATTACH_TYPE_PART;

					break;

				default:
					FASSERT_NOW;
				};

				++nAnchorIndex;
			}
		}

		FASSERT( nAnchorIndex == m_nAnchorCount );
	}

	m_Verlet.Net_Freeze( TRUE );

	return TRUE;

_ExitWithError:
	Destroy();
	fres_ReleaseFrame( ResFrame );

	return FALSE;
}


void CBotPart::Destroy( void ) {
	if( IsCreated() ) {
		fdelete_array( m_pTackSurfaceArray );
		m_pTackSurfaceArray = NULL;
		m_pTackSurfaceDefArray = NULL;

		m_Verlet.Destroy();
	}
}


void CBotPart::SetMode_Disabled( void ) {
	FASSERT( IsCreated() );

	switch( m_nMode ) {
	case MODE_DISABLED:
		break;

	case MODE_DANGLING:
		m_pDanglingFromBot = NULL;

		m_Verlet.AttachedToWorldMesh( NULL );
		m_Verlet.SetTackSurfaceArray( NULL, NULL );

		m_Verlet.Net_Freeze( TRUE );

		m_nMode = MODE_DISABLED;

		break;

	case MODE_DEBRIS:
		m_Verlet.Net_Freeze( TRUE );
		m_nMode = MODE_DISABLED;

		break;

	case MODE_REPAIR:
		m_Verlet.Net_Freeze( TRUE );
		m_nMode = MODE_DISABLED;

		break;

	default:
		FASSERT_NOW;
	};
}


void CBotPart::SetMode_Repair( void ) {
	FASSERT( IsCreated() );

	// (Could support repairing destroyed pieces too, but not today)
	if( m_nMode != MODE_DANGLING ) {
		return;
	}

	m_nMode = MODE_REPAIR;
	m_fUnitRepairTimer = 0.0f;
}


void CBotPart::SetMode_Dangling( CBot *pDanglingFromBot ) {
	FASSERT( IsCreated() );

	SetMode_Disabled();

	m_pDanglingFromBot = pDanglingFromBot;

	_InitTackSurfaces( pDanglingFromBot->m_pWorldMesh );

	m_Verlet.AttachedToWorldMesh( pDanglingFromBot->m_pWorldMesh );
	m_Verlet.SetTackSurfaceArray( m_ppTackSurfaceArray, m_nTackSurfaceCount );
	m_Verlet.StopMotion();
	m_Verlet.ClearForces();
	m_Verlet.ClearImpulses();

	m_nMode = MODE_DANGLING;
	m_Verlet.Net_Freeze( FALSE );
}


void CBotPart::_InitTackSurfaces( CFWorldMesh *pWorldMesh ) {
	const TackSurfaceDef_t *pTackSurfaceDef;
	CFVerletTackSurface *pTackSurface;
	u8 nTackSurfaceIndex;

	for( nTackSurfaceIndex=0; nTackSurfaceIndex<m_nTackSurfaceCount; ++nTackSurfaceIndex ) {
		pTackSurfaceDef = m_pTackSurfaceDefArray + nTackSurfaceIndex;
		pTackSurface = m_pTackSurfaceArray + nTackSurfaceIndex;

		switch( (CFVerletTackSurface::AnchorType_e)pTackSurfaceDef->nAnchorType ) {
			case CFVerletTackSurface::ANCHOR_TYPE_DYNAMIC:
				pTackSurface->Init_Plane_Dynamic( pWorldMesh->GetBoneMtxPalette()[ pTackSurfaceDef->nBoneIndex ], TRUE, (CFVerletTackSurface::PlaneNormal_e)pTackSurfaceDef->nPlaneNormalAxis );
				break;

			case CFVerletTackSurface::ANCHOR_TYPE_BONED:
				pTackSurface->Init_Plane_Boned( pWorldMesh, pTackSurfaceDef->nBoneIndex, (CFVerletTackSurface::PlaneNormal_e)pTackSurfaceDef->nPlaneNormalAxis );
				break;

			default:
				FASSERT_NOW;
		};

		pTackSurface->SetFriction( pTackSurfaceDef->fFriction );
		pTackSurface->SetPreserveVelocityFlag( !!(pTackSurfaceDef->nFlags & TACK_SURFACE_FLAG_PRESERVE_VELOCITY) );
		pTackSurface->SetPlaneDist( pTackSurfaceDef->fDistFromPlane );
		pTackSurface->Enable( TRUE );
	}
}


u32 CBotPart::_FillStringTable( cchar **apszDestNameTable, cchar * const *apszSourceNameTable, u32 nMaxCount, BOOL bAddNullTerminator ) {
	u32 i, nCount;

	nCount = 0;

	for( i=0; i<nMaxCount; ++i ) {
		if( apszSourceNameTable[i] ) {
			apszDestNameTable[nCount] = apszSourceNameTable[i];
			++nCount;
		}
	}

	if( bAddNullTerminator ) {
		apszDestNameTable[nCount] = NULL;
	}

	return nCount;
}


void CBotPart::_VerletConstraintCallback( CFVerlet *pVerlet ) {
	CBotPart *pBotPart = (CBotPart *)pVerlet->m_pUser;

	FWorld_nTrackerSkipListCount = 0;
	pBotPart->m_pDanglingFromBot->AppendTrackerSkipList();

	CFVec3A StartPos_WS, EndPos_WS;
	CFVerletTack *pStartTack, *pEndTack;
	FCollImpact_t CollImpact;

	pStartTack = pBotPart->m_Verlet.Tack_GetFromIndex( pBotPart->m_nRayTestStartVerletTackIndex );
	pEndTack = pBotPart->m_Verlet.Tack_GetFromIndex( pBotPart->m_nRayTestEndVerletTackIndex );

	pStartTack->ComputePos_WS( &StartPos_WS );
	pEndTack->ComputePos_WS( &EndPos_WS );

	if( fworld_FindClosestImpactPointToRayStart( &CollImpact, &StartPos_WS, &EndPos_WS, FWorld_nTrackerSkipListCount, FWorld_apTrackerSkipList ) ) {
		pBotPart->m_Verlet.CollideTacksWithPlane( &CollImpact.ImpactPoint, &CollImpact.UnitFaceNormal, 1.0f, 0.0f, TRUE );
	}
}




//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CBotPartInit
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

#define _VOCAB_BONE_NAME															\
	FGAMEDATA_VAR_TYPE_STRING|														\
	FGAMEDATA_FLAGS_STRING_PTR_ONLY | FGAMEDATA_FLAGS_STRING_NONE_TO_NULL,			\
	sizeof( cchar * ),																\
	F32_DATATABLE_0,																			\
	F32_DATATABLE_0


#define _VOCAB_ANCHOR																\
	FGAMEDATA_VAR_TYPE_FLOAT|														\
	FGAMEDATA_FLAGS_CONVERT_TO_U32 | FGAMEDATA_FLAGS_FLOAT_CHECK_RANGE_AND_FAIL,	\
	sizeof( u32 ),																	\
	F32_DATATABLE_0,																			\
	F32_DATATABLE_2,																			\
																					\
	FGAMEDATA_VAR_TYPE_STRING|														\
	FGAMEDATA_FLAGS_STRING_PTR_ONLY | FGAMEDATA_FLAGS_STRING_NONE_TO_NULL,			\
	sizeof( cchar * ),																\
	F32_DATATABLE_0,																			\
	F32_DATATABLE_0,																			\
																					\
	FGAMEDATA_VAR_TYPE_STRING|														\
	FGAMEDATA_FLAGS_STRING_PTR_ONLY | FGAMEDATA_FLAGS_STRING_NONE_TO_NULL,			\
	sizeof( cchar * ),																\
	F32_DATATABLE_0,																			\
	F32_DATATABLE_0,																			\
																					\
	FGAMEDATA_VAR_TYPE_STRING|														\
	FGAMEDATA_FLAGS_STRING_PTR_ONLY | FGAMEDATA_FLAGS_STRING_NONE_TO_NULL,			\
	sizeof( cchar * ),																\
	F32_DATATABLE_0,																			\
	F32_DATATABLE_0,																			\
																					\
	FGAMEDATA_VAR_TYPE_FLOAT|														\
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,					\
	sizeof( f32 ),																	\
	F32_DATATABLE_0,																			\
	F32_DATATABLE_1000000000,																\
																					\
	FGAMEDATA_VAR_TYPE_FLOAT|														\
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,					\
	sizeof( f32 ),																	\
	F32_DATATABLE_0,																			\
	F32_DATATABLE_1000000000


#define _VOCAB_COLL_PLANE															\
	FGAMEDATA_VAR_TYPE_STRING|														\
	FGAMEDATA_FLAGS_STRING_PTR_ONLY | FGAMEDATA_FLAGS_STRING_NONE_TO_NULL,			\
	sizeof( cchar * ),																\
	F32_DATATABLE_0,																			\
	F32_DATATABLE_0,																			\
																					\
	FGAMEDATA_VAR_TYPE_FLOAT|														\
	FGAMEDATA_FLAGS_CONVERT_TO_U32 | FGAMEDATA_FLAGS_FLOAT_CHECK_RANGE_AND_FAIL,	\
	sizeof( u32 ),																	\
	F32_DATATABLE_0,																			\
	F32_DATATABLE_5,																			\
																					\
	FGAMEDATA_VAR_TYPE_FLOAT|														\
	FGAMEDATA_FLAGS_CONVERT_TO_U32 | FGAMEDATA_FLAGS_FLOAT_CHECK_RANGE_AND_FAIL,	\
	sizeof( u32 ),																	\
	F32_DATATABLE_0,																			\
	F32_DATATABLE_1,																			\
																					\
	FGAMEDATA_VAR_TYPE_FLOAT|														\
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,					\
	sizeof( f32 ),																	\
	F32_DATATABLE_0,																			\
	F32_DATATABLE_1000000000,														\
																					\
	FGAMEDATA_VAR_TYPE_FLOAT|														\
	FGAMEDATA_FLAGS_FLOAT_X,														\
	sizeof( f32 ),																	\
	F32_DATATABLE_0,																			\
	F32_DATATABLE_0,																			\
																					\
	FGAMEDATA_VAR_TYPE_FLOAT|														\
	FGAMEDATA_FLAGS_CONVERT_TO_U32 | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,			\
	sizeof( u32 ),																	\
	F32_DATATABLE_0,																			\
	F32_DATATABLE_1



const FGameData_TableEntry_t CBotPartInit::m_aGameDataVocab[] = {
	FGAMEDATA_VAR_TYPE_STRING|
	FGAMEDATA_FLAGS_STRING_PTR_ONLY,
	sizeof( cchar * ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	_VOCAB_BONE_NAME,		// m_apszBoneNameParts[0]
	_VOCAB_BONE_NAME,		// m_apszBoneNameParts[1]
	_VOCAB_BONE_NAME,		// m_apszBoneNameParts[2]
	_VOCAB_BONE_NAME,		// m_apszBoneNameParts[3]
	_VOCAB_BONE_NAME,		// m_apszBoneNameParts[4]
	_VOCAB_BONE_NAME,		// m_apszBoneNameParts[5]
	_VOCAB_BONE_NAME,		// m_apszBoneNameParts[6]
	_VOCAB_BONE_NAME,		// m_apszBoneNameParts[7]
	_VOCAB_BONE_NAME,		// m_apszBoneNameParts[8]
	_VOCAB_BONE_NAME,		// m_apszBoneNameParts[9]
	_VOCAB_BONE_NAME,		// m_apszBoneNameParts[10]
	_VOCAB_BONE_NAME,		// m_apszBoneNameParts[11]
	_VOCAB_BONE_NAME,		// m_apszBoneNameParts[12]
	_VOCAB_BONE_NAME,		// m_apszBoneNameParts[13]
	_VOCAB_BONE_NAME,		// m_apszBoneNameParts[14]
	_VOCAB_BONE_NAME,		// m_apszBoneNameParts[15]

	_VOCAB_BONE_NAME,		// m_apszBoneNameTacks[0]
	_VOCAB_BONE_NAME,		// m_apszBoneNameTacks[1]
	_VOCAB_BONE_NAME,		// m_apszBoneNameTacks[2]
	_VOCAB_BONE_NAME,		// m_apszBoneNameTacks[3]
	_VOCAB_BONE_NAME,		// m_apszBoneNameTacks[4]
	_VOCAB_BONE_NAME,		// m_apszBoneNameTacks[5]
	_VOCAB_BONE_NAME,		// m_apszBoneNameTacks[6]
	_VOCAB_BONE_NAME,		// m_apszBoneNameTacks[7]
	_VOCAB_BONE_NAME,		// m_apszBoneNameTacks[8]
	_VOCAB_BONE_NAME,		// m_apszBoneNameTacks[9]
	_VOCAB_BONE_NAME,		// m_apszBoneNameTacks[10]
	_VOCAB_BONE_NAME,		// m_apszBoneNameTacks[11]
	_VOCAB_BONE_NAME,		// m_apszBoneNameTacks[12]
	_VOCAB_BONE_NAME,		// m_apszBoneNameTacks[13]
	_VOCAB_BONE_NAME,		// m_apszBoneNameTacks[14]
	_VOCAB_BONE_NAME,		// m_apszBoneNameTacks[15]

	_VOCAB_BONE_NAME,		// m_apszBoneNameCTacks[0]
	_VOCAB_BONE_NAME,		// m_apszBoneNameCTacks[1]
	_VOCAB_BONE_NAME,		// m_apszBoneNameCTacks[2]
	_VOCAB_BONE_NAME,		// m_apszBoneNameCTacks[3]
	_VOCAB_BONE_NAME,		// m_apszBoneNameCTacks[4]
	_VOCAB_BONE_NAME,		// m_apszBoneNameCTacks[5]
	_VOCAB_BONE_NAME,		// m_apszBoneNameCTacks[6]
	_VOCAB_BONE_NAME,		// m_apszBoneNameCTacks[7]
	_VOCAB_BONE_NAME,		// m_apszBoneNameCTacks[8]
	_VOCAB_BONE_NAME,		// m_apszBoneNameCTacks[9]
	_VOCAB_BONE_NAME,		// m_apszBoneNameCTacks[10]
	_VOCAB_BONE_NAME,		// m_apszBoneNameCTacks[11]
	_VOCAB_BONE_NAME,		// m_apszBoneNameCTacks[12]
	_VOCAB_BONE_NAME,		// m_apszBoneNameCTacks[13]
	_VOCAB_BONE_NAME,		// m_apszBoneNameCTacks[14]
	_VOCAB_BONE_NAME,		// m_apszBoneNameCTacks[15]

	_VOCAB_BONE_NAME,		// m_apszBoneNameBTacks[0]
	_VOCAB_BONE_NAME,		// m_apszBoneNameBTacks[1]
	_VOCAB_BONE_NAME,		// m_apszBoneNameBTacks[2]
	_VOCAB_BONE_NAME,		// m_apszBoneNameBTacks[3]

	_VOCAB_COLL_PLANE,		// m_aCollPlanes[0]
	_VOCAB_COLL_PLANE,		// m_aCollPlanes[1]
	_VOCAB_COLL_PLANE,		// m_aCollPlanes[2]
	_VOCAB_COLL_PLANE,		// m_aCollPlanes[3]

	_VOCAB_ANCHOR,			// m_aAnchors[0]
	_VOCAB_ANCHOR,			// m_aAnchors[1]
	_VOCAB_ANCHOR,			// m_aAnchors[2]
	_VOCAB_ANCHOR,			// m_aAnchors[3]

	// m_fDimX:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_100000000000,

	// m_fDimY:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_100000000000,

	// m_fDimZ:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_100000000000,

	// m_fMass:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_100000000000,

	// m_fDamping:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_100000000000,

	// m_nMeshPartsMask:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_CONVERT_TO_U32,
	sizeof( BOOL ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	// bCTackRayTest:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_CONVERT_TO_U32 | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( BOOL ),
	F32_DATATABLE_0,
	F32_DATATABLE_1,

	// End of table:
	FGAMEDATA_VAR_TYPE_COUNT| 0, 0, F32_DATATABLE_0, F32_DATATABLE_0
};



BOOL CBotPartInit::LoadFromGameData( cchar *pszTableName, FGameDataFileHandle_t hGameDataFile ) {
	FGameDataTableHandle_t hTable;		//CPS 4.7.03

	if( hGameDataFile == FGAMEDATA_INVALID_FILE_HANDLE ) {
		DEVPRINTF( "CBotPartInit::LoadFromGameData(): Invalid file handle passed in.\n" );
		goto _ExitWithError;
	}

//CPS 4.7.03	FGameDataTableHandle_t hTable = fgamedata_GetFirstTableHandle( hGameDataFile, pszTableName );
	hTable = fgamedata_GetFirstTableHandle( hGameDataFile, pszTableName );

	if( hTable == FGAMEDATA_INVALID_TABLE_HANDLE ) {
		DEVPRINTF( "CBotPartInit::LoadFromGameData(): Could not locate table '%s' in file '%s'.\n", pszTableName, fgamedata_GetFileNameFromFileHandle(hGameDataFile) );
		goto _ExitWithError;
	}

	return LoadFromGameData( hTable, fgamedata_GetFileNameFromFileHandle( hGameDataFile ) );

_ExitWithError:
	return FALSE;
}


BOOL CBotPartInit::LoadFromGameData( FGameDataTableHandle_t hTable, cchar *pszFileName ) {
	cchar *pszTableName = NULL;		//CPS 4.7.03
	
	if( hTable == FGAMEDATA_INVALID_TABLE_HANDLE ) {
		DEVPRINTF( "CBotPartInit::LoadFromGameData(): Invalid table handle for file '%s'.\n", pszFileName );
		goto _ExitWithError;
	}

//CPS 4.7.03	cchar *pszTableName = fgamedata_GetTableName( hTable );
	pszTableName = fgamedata_GetTableName( hTable );

	if( !fgamedata_GetTableData( hTable, m_aGameDataVocab, this, sizeof(CBotPartInit) ) ) {
		DEVPRINTF( "CBotPartInit::LoadFromGameData(): Trouble parsing bot part data for table '%s' in '%s.csv'.\n", pszTableName, pszFileName );
		goto _ExitWithError;
	}

	return TRUE;

_ExitWithError:
	return FALSE;
}




//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CBotPartPool
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

SmokeTrailAttrib_t CBotPartPool::m_SmokeTrailAttrib;
FParticle_DefHandle_t CBotPartPool::m_hDebrisFlame;
FParticle_DefHandle_t CBotPartPool::m_hDebrisSmoke;
BOOL CBotPartPool::m_bSystemInitialized;


const FGameData_TableEntry_t CBotPartPool::m_aGameDataVocab_LimbGameDataInfo[] = {
	// pszLimbName:
	FGAMEDATA_VAR_TYPE_STRING|
	FGAMEDATA_FLAGS_STRING_PTR_ONLY,
	sizeof( cchar * ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	// pszArmorProfileIntact:
	FGAMEDATA_VAR_TYPE_STRING|
	FGAMEDATA_FLAGS_STRING_PTR_ONLY,
	sizeof( char * ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	// pszArmorProfileDangle:
	FGAMEDATA_VAR_TYPE_STRING|
	FGAMEDATA_FLAGS_STRING_PTR_ONLY,
	sizeof( char * ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	// nComponentTypeFlags:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_CONVERT_TO_U32 | FGAMEDATA_FLAGS_FLOAT_CHECK_RANGE_AND_FAIL,
	sizeof( u32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_15,

	// fInitialHealthMultIntact:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	// fInitialHealthMultDangle:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	// nDangleRuleCode:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_CONVERT_TO_U32 | FGAMEDATA_FLAGS_FLOAT_CHECK_RANGE_AND_FAIL,
	sizeof( u32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_1,

	// pszProxyDamageBone:
	FGAMEDATA_VAR_TYPE_STRING|
	FGAMEDATA_FLAGS_STRING_PTR_ONLY | FGAMEDATA_FLAGS_STRING_NONE_TO_NULL,
	sizeof( cchar * ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	// fProxyDamageBox_NegX_BS:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	// fProxyDamageBox_PosX_BS:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	// fProxyDamageBox_NegY_BS:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	// fProxyDamageBox_PosY_BS:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	// fProxyDamageBox_NegZ_BS:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	// fProxyDamageBox_PosZ_BS:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	// pszParticleBone:
	FGAMEDATA_VAR_TYPE_STRING|
	FGAMEDATA_FLAGS_STRING_PTR_ONLY | FGAMEDATA_FLAGS_STRING_NONE_TO_NULL,
	sizeof( cchar * ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	// aParticleInfo[0].hParticleDef:
	FGAMEDATA_VAR_TYPE_STRING|
	FGAMEDATA_FLAGS_STRING_PTR_TO_FPART_HANDLE,
	sizeof( FParticle_DefHandle_t ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	// aParticleInfo[0].fDuration:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	// aParticleInfo[0].fUnitIntensity:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_1,

	// aParticleInfo[1].hParticleDef:
	FGAMEDATA_VAR_TYPE_STRING|
	FGAMEDATA_FLAGS_STRING_PTR_TO_FPART_HANDLE,
	sizeof( FParticle_DefHandle_t ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	// aParticleInfo[1].fDuration:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	// aParticleInfo[1].fUnitIntensity:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_1,

	// aParticleInfo[2].hParticleDef:
	FGAMEDATA_VAR_TYPE_STRING|
	FGAMEDATA_FLAGS_STRING_PTR_TO_FPART_HANDLE,
	sizeof( FParticle_DefHandle_t ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	// aParticleInfo[2].fDuration:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	// aParticleInfo[2].fUnitIntensity:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_1,

	// aParticleInfo[3].hParticleDef:
	FGAMEDATA_VAR_TYPE_STRING|
	FGAMEDATA_FLAGS_STRING_PTR_TO_FPART_HANDLE,
	sizeof( FParticle_DefHandle_t ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	// aParticleInfo[3].fDuration:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	// aParticleInfo[3].fUnitIntensity:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_1,

	// m_pDebrisGroup:
	FGAMEDATA_VAR_TYPE_STRING|
	FGAMEDATA_FLAGS_STRING_TO_DEBRIS_GROUP,
	sizeof( CFDebrisGroup * ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	// hDangleSound:
	FGAMEDATA_VAR_TYPE_STRING|
	FGAMEDATA_FLAGS_STRING_PTR_TO_SFX_HANDLE | FGAMEDATA_FLAGS_STRING_NONE_TO_NULL,
	sizeof( FSndFx_FxHandle_t ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	// hBlowOffSound:
	FGAMEDATA_VAR_TYPE_STRING|
	FGAMEDATA_FLAGS_STRING_PTR_TO_SFX_HANDLE | FGAMEDATA_FLAGS_STRING_NONE_TO_NULL,
	sizeof( FSndFx_FxHandle_t ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	// hBlowOffExplosion
	FGAMEDATA_VOCAB_EXPLODE_GROUP,

	// End of table:
	FGAMEDATA_VAR_TYPE_COUNT| 0, 0, F32_DATATABLE_0, F32_DATATABLE_0
};


const FGameData_TableEntry_t CBotPartPool::m_aGameDataVocab_BoneInfo[] = {
	// pszBoneName:
	FGAMEDATA_VAR_TYPE_STRING|
	FGAMEDATA_FLAGS_STRING_PTR_ONLY,
	sizeof( cchar * ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	// nBurstCode:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_CONVERT_TO_U32 | FGAMEDATA_FLAGS_FLOAT_CHECK_RANGE_AND_FAIL,
	sizeof( u32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_2,

	// nFlamingChunkCount:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_CONVERT_TO_U32 | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( u32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_10,

	// nChunkCount:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_CONVERT_TO_U32 | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( u32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_10,

	// nGutCount:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_CONVERT_TO_U32 | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( u32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_10,

	// bBoneGeoOnFire:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_CONVERT_TO_U32 | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( u32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_1,

	// fChunkScale:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	// fGutScale:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	// hExplosion
	FGAMEDATA_VOCAB_EXPLODE_GROUP,

	// End of table:
	FGAMEDATA_VAR_TYPE_COUNT| 0, 0, F32_DATATABLE_0, F32_DATATABLE_0
};


BOOL CBotPartPool::InitSystem( void ) {
	FASSERT( !IsSystemInitialized() );

	m_bSystemInitialized = TRUE;

	_InitEffectDefs();

	return TRUE;
}


void CBotPartPool::UninitSystem( void ) {
	if( IsSystemInitialized() ) {
		m_bSystemInitialized = FALSE;
	}
}


BOOL CBotPartPool::RegisterClient( CBotPartPool **ppPartPool, CBot *pBot, cchar *pszGameDataFileName, u32 nPartsPerPoolCount, const FMesh_t *pMesh ) {
	FASSERT( ppPartPool );
	FASSERT( pszGameDataFileName );
	FASSERT( nPartsPerPoolCount );
	FASSERT( pMesh );
	FASSERT( pMesh->nBoneCount < 128 );

	if( *ppPartPool ) {
		// We've already been created...

		++(*ppPartPool)->m_nClientCount;

		return TRUE;
	}

	// Create ourselves...

	u32 i, j, nPartType, nLimbType, nLimbTypeCount, nPartsInLimbCount, nBonesInPartCount, nTableFieldCount, nStructElementCount, nParticleIndex;
	u32 nLeftLegCount, nRightLegCount;
	s32 nBoneIndex;
	CBotPartInit BotPartInit;
	FGameDataFileHandle_t hFile;
	FGameDataTableHandle_t hLimbInfoTable, hBoneInfoTable;
	FGameDataTableHandle_t hTable;
	FGameDataWalker_t Walker;
	CBotPartPool *pPartPool;
	CLimbDef *pLimbDef;
	LimbGameDataInfo_t *pLimbGameDataInfo;
	BoneInfo_t *pBoneGameDataInfo;
	cchar *pszLimbName;

	FResFrame_t ResFrame = fres_GetFrame();
	FMemFrame_t MemFrame = fmem_GetFrame();

	pPartPool = fnew CBotPartPool;
	if( pPartPool == NULL ) {
		DEVPRINTF( "CBotPartPool::RegisterClient(): Not enough memory to create part pool object.\n" );
		goto _ExitWithError;
	}

	++pPartPool->m_nClientCount;

	*ppPartPool = pPartPool;
	pPartPool->m_ppPartPool = ppPartPool;

	// Load game data file...
	hFile = fgamedata_LoadFileToFMem( pszGameDataFileName );
	if( hFile == FGAMEDATA_INVALID_FILE_HANDLE ) {
		DEVPRINTF( "CBotPartPool::RegisterClient(): Could not load file '%s.csv'.\n", pszGameDataFileName );
		goto _ExitWithError;
	}

	//--------------------------------------------------
	// Alloc LimbInfo table:
	//--------------------------------------------------

	// Find LimbInfo table...
	hLimbInfoTable = fgamedata_GetFirstTableHandle( hFile, _LIMB_INFO_TABLE_NAME );
	if( hLimbInfoTable == FGAMEDATA_INVALID_TABLE_HANDLE ) {
		DEVPRINTF( "CBotPartPool::RegisterClient(): Could not locate limb info table '%s' in file '%s.csv'.\n", _LIMB_INFO_TABLE_NAME, pszGameDataFileName );
		goto _ExitWithError;
	}

	nStructElementCount = sizeof( m_aGameDataVocab_LimbGameDataInfo ) / sizeof( FGameData_TableEntry_t );
	FASSERT( nStructElementCount > 0 );
	--nStructElementCount;

	nTableFieldCount = fgamedata_GetNumFields( hLimbInfoTable );
	if( nTableFieldCount % nStructElementCount ) {
		DEVPRINTF( "CBotPartPool::RegisterClient(): Invalid number of fields in limb info table '%s' in file '%s.csv'.\n", _LIMB_INFO_TABLE_NAME, pszGameDataFileName );
		goto _ExitWithError;
	}

	nLimbTypeCount = nTableFieldCount / nStructElementCount;

	if( nLimbTypeCount > 255 ) {
		DEVPRINTF( "CBotPartPool::RegisterClient(): Limb count exceeds 255.\n" );
		goto _ExitWithError;
	}

	// Note that this is going in temp memory...
	pLimbGameDataInfo = (LimbGameDataInfo_t *)fmem_Alloc( sizeof(LimbGameDataInfo_t) * nLimbTypeCount );
	if( pLimbGameDataInfo == NULL ) {
		DEVPRINTF( "CBotPartPool::RegisterClient(): Not enough memory to create LimbGameDataInfo_t array.\n" );
		goto _ExitWithError;
	}

	if( !fgamedata_GetTableData_ArrayOfStructures( hLimbInfoTable, m_aGameDataVocab_LimbGameDataInfo, pLimbGameDataInfo, sizeof(LimbGameDataInfo_t), nLimbTypeCount ) ) {
		DEVPRINTF( "CBotPartPool::RegisterClient(): Problem encountered while parsing limb info table '%s' in file '%s.csv'.\n", _LIMB_INFO_TABLE_NAME, pszGameDataFileName );
		goto _ExitWithError;
	}

	// Make sure there are no duplicate limb names in the limb info table...
	for( i=0; i<nLimbTypeCount; ++i ) {
		for( j=i+1; j<nLimbTypeCount; ++j ) {
			if( !fclib_stricmp( pLimbGameDataInfo[i].pszLimbName, pLimbGameDataInfo[j].pszLimbName ) ) {
				DEVPRINTF( "CBotPartPool::RegisterClient(): Duplicate limb name '%' specified in string table.\n", pLimbGameDataInfo[i].pszLimbName );
				goto _ExitWithError;
			}
		}
	}

	pPartPool->m_pLimbDefArray = fnew CLimbDef[nLimbTypeCount];
	if( pPartPool->m_pLimbDefArray == NULL ) {
		DEVPRINTF( "CBotPartPool::RegisterClient(): Not enough memory to create m_pLimbDefArray.\n" );
		goto _ExitWithError;
	}

	pPartPool->m_nLimbTypeCount = nLimbTypeCount;

	for( i=0; i<nLimbTypeCount; ++i ) {
		pPartPool->m_pLimbDefArray[i].m_pParticleInfoArray = NULL;
		pPartPool->m_pLimbDefArray[i].m_pDebrisGroup = NULL;
	}

	//--------------------------------------------------
	// Init DebrisInfo table:
	//--------------------------------------------------

	pPartPool->m_nBoneInfoCount = 0;
	pPartPool->m_pBoneInfo = NULL;

	// Find BoneInfo table...
	hBoneInfoTable = fgamedata_GetFirstTableHandle( hFile, _BONE_INFO_TABLE_NAME );
	if( hBoneInfoTable != FGAMEDATA_INVALID_TABLE_HANDLE ) {
		nStructElementCount = sizeof( m_aGameDataVocab_BoneInfo ) / sizeof( FGameData_TableEntry_t );
		FASSERT( nStructElementCount > 0 );
		--nStructElementCount;

		nTableFieldCount = fgamedata_GetNumFields( hBoneInfoTable );
		if( nTableFieldCount % nStructElementCount ) {
			DEVPRINTF( "CBotPartPool::RegisterClient(): Invalid number of fields in bone info table '%s' in file '%s.csv'.\n", _BONE_INFO_TABLE_NAME, pszGameDataFileName );
			goto _ExitWithError;
		}

		pPartPool->m_nBoneInfoCount = nTableFieldCount / nStructElementCount;

		if( pPartPool->m_nBoneInfoCount > 127 ) {
			DEVPRINTF( "CBotPartPool::RegisterClient(): Bone count exceeds 127.\n" );
			goto _ExitWithError;
		}

		// Note that this is going in temp memory...
		pBoneGameDataInfo = (BoneInfo_t *)fmem_Alloc( sizeof(BoneInfo_t) * pPartPool->m_nBoneInfoCount );
		if( pBoneGameDataInfo == NULL ) {
			DEVPRINTF( "CBotPartPool::RegisterClient(): Not enough memory to create BoneInfo_t array.\n" );
			goto _ExitWithError;
		}

		if( !fgamedata_GetTableData_ArrayOfStructures( hBoneInfoTable, m_aGameDataVocab_BoneInfo, pBoneGameDataInfo, sizeof(BoneInfo_t), pPartPool->m_nBoneInfoCount ) ) {
			DEVPRINTF( "CBotPartPool::RegisterClient(): Problem encountered while parsing bone debris info table '%s' in file '%s.csv'.\n", _BONE_INFO_TABLE_NAME, pszGameDataFileName );
			goto _ExitWithError;
		}

		// Make sure there are no duplicate bone names in the limb info table, and that all the bone names exist...
		for( i=0; i<pPartPool->m_nBoneInfoCount; ++i ) {
			for( j=i+1; j<pPartPool->m_nBoneInfoCount; ++j ) {
				if( !fclib_stricmp( pBoneGameDataInfo[i].pszBoneName, pBoneGameDataInfo[j].pszBoneName ) ) {
					DEVPRINTF( "CBotPartPool::RegisterClient(): Duplicate bone name '%s' specified in string table.\n", pBoneGameDataInfo[i].pszBoneName );
					goto _ExitWithError;
				}
			}

			if( fmesh_FindBone( pMesh, pBoneGameDataInfo[i].pszBoneName ) < 0 ) {
				DEVPRINTF( "CBotPartPool::RegisterClient(): Bone name '%s' in bone debris table not found.\n", pBoneGameDataInfo[i].pszBoneName );
				goto _ExitWithError;
			}
		}

		pPartPool->m_pBoneInfo = fnew CBoneDef[pPartPool->m_nBoneInfoCount];
		if( pPartPool->m_pBoneInfo == NULL ) {
			DEVPRINTF( "CBotPartPool::RegisterClient(): Not enough memory to create m_pBoneInfo.\n" );
			goto _ExitWithError;
		}

		for( i=0; i<pPartPool->m_nBoneInfoCount; ++i ) {
			pPartPool->m_pBoneInfo[i].m_nBoneGeoDebrisGroupIndex = pBoneGameDataInfo[i].nBurstCode;
			pPartPool->m_pBoneInfo[i].m_nFlamingChunkCount = pBoneGameDataInfo[i].nFlamingChunkCount;
			pPartPool->m_pBoneInfo[i].m_nChunkCount = pBoneGameDataInfo[i].nChunkCount;
			pPartPool->m_pBoneInfo[i].m_nGutCount = pBoneGameDataInfo[i].nGutCount;
			pPartPool->m_pBoneInfo[i].m_bBoneGeoOnFire = pBoneGameDataInfo[i].bBoneGeoOnFire;
			pPartPool->m_pBoneInfo[i].m_nBoneIndex = fmesh_FindBone( pMesh, pBoneGameDataInfo[i].pszBoneName );
			pPartPool->m_pBoneInfo[i].m_fChunkScale = pBoneGameDataInfo[i].fChunkScale;
			pPartPool->m_pBoneInfo[i].m_fGutScale = pBoneGameDataInfo[i].fGutScale;
			pPartPool->m_pBoneInfo[i].m_hExplosion = pBoneGameDataInfo[i].hExplosion;
/*
			#if !FANG_PLATFORM_DX
				// Reduce the number of debris chunks on non-Xbox/PC builds...

				(pPartPool->m_pBoneInfo[i].m_nFlamingChunkCount >> 1) + 1;
				if( pBoneGameDataInfo[i].nFlamingChunkCount ) {
					FMATH_CLAMPMIN( pPartPool->m_pBoneInfo[i].m_nFlamingChunkCount, 1 );
				}

				(pPartPool->m_pBoneInfo[i].m_nChunkCount >> 1) + 1;
				if( pBoneGameDataInfo[i].nChunkCount ) {
					FMATH_CLAMPMIN( pPartPool->m_pBoneInfo[i].m_nChunkCount, 1 );
				}
			#endif
*/
		}
	}

	//--------------------------------------------------
	// Count parts and check validity:
	//--------------------------------------------------

	pPartPool->m_nPartsPerPool = nPartsPerPoolCount;
	pPartPool->m_nPartTypeCount = 0;
	pPartPool->m_pPartTypePoolArray = NULL;
	pPartPool->m_nBoneCount = pMesh->nBoneCount;
	pPartPool->m_nPackedBoneWordCount = (pPartPool->m_nBoneCount + 31) >> 5;

	pPartPool->m_pnOrigBoneCallbackEnabledBitmask = (u32 *)fres_AllocAndZero( sizeof(u32) * pPartPool->m_nPackedBoneWordCount );
	if( pPartPool->m_pnOrigBoneCallbackEnabledBitmask == NULL ) {
		DEVPRINTF( "CBotPartPool::RegisterClient(): Not enough memory to create m_pnOrigBoneCallbackEnabledBitmask.\n" );
		goto _ExitWithError;
	}

	for( i=0; i<pPartPool->m_nBoneCount; ++i ) {
		if( pBot->m_Anim.m_pAnimCombiner->IsBoneCallbackEnabled( i ) ) {
			pPartPool->_SetOrigBoneCallbackEnabledBitmask( i );
		}
	}

	pPartPool->m_pnBoneIsProxyBitmask = (u32 *)fres_AllocAndZero( sizeof(u32) * pPartPool->m_nPackedBoneWordCount );
	if( pPartPool->m_pnBoneIsProxyBitmask == NULL ) {
		DEVPRINTF( "CBotPartPool::RegisterClient(): Not enough memory to create m_pnBoneIsProxyBitmask.\n" );
		goto _ExitWithError;
	}

	pPartPool->m_anBoneToPartArray = (s8 *)fres_Alloc( sizeof(s8) * pMesh->nBoneCount );
	if( pPartPool->m_anBoneToPartArray == NULL ) {
		DEVPRINTF( "CBotPartPool::RegisterClient(): Not enough memory to create m_anBoneToPartArray.\n" );
		goto _ExitWithError;
	}

	for( i=0; i<pMesh->nBoneCount; ++i ) {
		pPartPool->m_anBoneToPartArray[i] = -1;
	}

	// Count the total number of parts...

	for( nLimbType=0; nLimbType<pPartPool->m_nLimbTypeCount; ++nLimbType ) {
		// Find all tables in file that have a matching limb name...

		pLimbDef = &pPartPool->m_pLimbDefArray[nLimbType];

		pszLimbName = pLimbGameDataInfo[nLimbType].pszLimbName;

		if( pPartPool->m_nPartTypeCount > 255 ) {
			DEVPRINTF( "CBotPartPool::RegisterClient(): Part count exceeds 255 in file '%s.csv'.\n", pszGameDataFileName );
			goto _ExitWithError;
		}

		if( pLimbGameDataInfo[nLimbType].pszProxyDamageBone ) {
			nBoneIndex = fmesh_FindBone( pMesh, pLimbGameDataInfo[nLimbType].pszProxyDamageBone );

			if( nBoneIndex < 0 ) {
				DEVPRINTF( "CBotPartPool::RegisterClient(): Proxy bone '%s' for limb '%s' of file '%s.csv' not found in mesh.\n", pLimbGameDataInfo[nLimbType].pszProxyDamageBone, pLimbGameDataInfo[nLimbType].pszLimbName, pszGameDataFileName );
				goto _ExitWithError;
			}
		}

		if( pLimbGameDataInfo[nLimbType].pszParticleBone ) {
			nBoneIndex = fmesh_FindBone( pMesh, pLimbGameDataInfo[nLimbType].pszParticleBone );

			if( nBoneIndex < 0 ) {
				DEVPRINTF( "CBotPartPool::RegisterClient(): Particle bone '%s' for limb '%s' of file '%s.csv' not found in mesh.\n", pLimbGameDataInfo[nLimbType].pszParticleBone, pLimbGameDataInfo[nLimbType].pszLimbName, pszGameDataFileName );
				goto _ExitWithError;
			}
		}

		pLimbDef->m_nStartPartType = pPartPool->m_nPartTypeCount;
		nPartsInLimbCount = 0;

		for( hTable=fgamedata_GetFirstTable( hFile, Walker ); hTable!=FGAMEDATA_INVALID_TABLE_HANDLE; hTable=fgamedata_GetNextTable(Walker) ) {
			if( !fclib_stricmp( fgamedata_GetTableName(hTable), _LIMB_INFO_TABLE_NAME ) ) {
				continue;
			}

			if( !fclib_stricmp( fgamedata_GetTableName(hTable), _BONE_INFO_TABLE_NAME ) ) {
				continue;
			}

			if( !BotPartInit.LoadFromGameData( hTable, pszGameDataFileName ) ) {
				goto _ExitWithError;
			}

			if( !fclib_stricmp( pszLimbName, BotPartInit.m_pszLimbName ) ) {
				// Found one...

				++pPartPool->m_nPartTypeCount;
				++nPartsInLimbCount;
			}
		}

		if( ((u32)pLimbDef->m_nStartPartType + nPartsInLimbCount) > 255 ) {
			DEVPRINTF( "CBotPartPool::RegisterClient(): Part count exceeds 255.\n" );
			goto _ExitWithError;
		}

		pLimbDef->m_nEndPartType = pLimbDef->m_nStartPartType + nPartsInLimbCount;

		if( nPartsInLimbCount == 0 ) {
			DEVPRINTF( "CBotPartPool::RegisterClient(): Warning! Limb '%s' specified in limb info table '%s' was not found. File '%s.csv'.\n", pszLimbName, _LIMB_INFO_TABLE_NAME, pszGameDataFileName );
		}

		if( nPartsInLimbCount > BOTPART_MAX_PARTS_PER_LIMB ) {
			DEVPRINTF( "CBotPartPool::RegisterClient(): Too many parts in limb '%s' (max is %u). Table '%s', File '%s'.\n", pszLimbName, BOTPART_MAX_PARTS_PER_LIMB, _LIMB_INFO_TABLE_NAME, pszGameDataFileName );
			goto _ExitWithError;
		}
	}

	//--------------------------------------------------
	// Alloc and init part tables:
	//--------------------------------------------------

	// Allocate part set array...
	pPartPool->m_pPartTypePoolArray = (PartTypePool_t *)fres_AllocAndZero( sizeof(PartTypePool_t) * pPartPool->m_nPartTypeCount );
	if( pPartPool->m_pPartTypePoolArray == NULL ) {
		DEVPRINTF( "CBotPartPool::RegisterClient(): Not enough memory to create m_pPartTypePoolArray.\n" );
		goto _ExitWithError;
	}

	if( pPartPool->m_nPartTypeCount == 0 ) {
		// No parts...
		goto _ExitSuccessful;
	}

	// Allocate part type pools...
	for( nPartType=0; nPartType<pPartPool->m_nPartTypeCount; ++nPartType ) {
		pPartPool->m_pPartTypePoolArray[nPartType].pPartArray = fnew CBotPart[pPartPool->m_nPartsPerPool];

		if( pPartPool->m_pPartTypePoolArray[nPartType].pPartArray == NULL ) {
			DEVPRINTF( "CBotPartPool::RegisterClient(): Not enough memory to create m_pPartTypePoolArray[%u].pPartArray.\n", nPartType );
			goto _ExitWithError;
		}
	}

	// Finally, load the data...

	nPartType = 0;
	nLeftLegCount = 0;
	nRightLegCount = 0;

	pPartPool->m_nLimbTypeRightLeg = -1;
	pPartPool->m_nLimbTypeLeftLeg = -1;
	pPartPool->m_nRootBoneIndex = -1;
	pPartPool->m_nTorsoBoneIndex = -1;

	if( pBot->m_pBotInfo_Gen->pszRootBoneName ) {
		pPartPool->m_nRootBoneIndex = fmesh_FindBone( pMesh, pBot->m_pBotInfo_Gen->pszRootBoneName );
	}
	if( pPartPool->m_nRootBoneIndex < 0 ) {
		DEVPRINTF( "CBotPartPool::RegisterClient(): The BotInfo_Gen structure must provide a valid pszRootBoneName if the bot part system is to be used.\n" );
		goto _ExitWithError;
	}

	if( pBot->m_pBotInfo_Gen->pTorsoBlowOffArmorProfile && pBot->m_pBotInfo_Gen->pszTorsoBoneName ) {
		pPartPool->m_nTorsoBoneIndex = fmesh_FindBone( pMesh, pBot->m_pBotInfo_Gen->pszTorsoBoneName );
	}

	for( nLimbType=0; nLimbType<pPartPool->m_nLimbTypeCount; ++nLimbType ) {
		// Find all tables in file that have a matching limb name...

		pLimbDef = &pPartPool->m_pLimbDefArray[nLimbType];

		pLimbDef->m_nParticleBoneIndex = -1;
		pLimbDef->m_nParticleDefCount = 0;
		pLimbDef->m_pParticleInfoArray = NULL;
		pLimbDef->m_pDebrisGroup = NULL;
		pLimbDef->m_hDangleSound = NULL;
		pLimbDef->m_hBlowOffSound = NULL;
		pLimbDef->m_hBlowOffExplosion = NULL;

		pLimbDef->m_fInitialHealthMultIntact = pLimbGameDataInfo[nLimbType].fInitialHealthMultIntact;
		pLimbDef->m_fInitialHealthMultDangle = pLimbGameDataInfo[nLimbType].fInitialHealthMultDangle;
		pLimbDef->m_nComponentTypeFlags = pLimbGameDataInfo[nLimbType].nComponentTypeFlags;
		pLimbDef->m_nDangleRule = (DangleRule_e)pLimbGameDataInfo[nLimbType].nDangleRuleCode;
		pLimbDef->m_pArmorProfileIntact = CDamage::FindArmorProfile( pLimbGameDataInfo[nLimbType].pszArmorProfileIntact );
		pLimbDef->m_pArmorProfileDangle = CDamage::FindArmorProfile( pLimbGameDataInfo[nLimbType].pszArmorProfileDangle );

		pszLimbName = pLimbGameDataInfo[nLimbType].pszLimbName;

		if( pBot->m_pBotInfo_Gen->pszRightLegLimbName && !fclib_stricmp( pszLimbName, pBot->m_pBotInfo_Gen->pszRightLegLimbName ) ) {
			// This is the right leg...

			pLimbDef->m_nLegType = LEG_TYPE_RIGHT;
			pPartPool->m_nLimbTypeRightLeg = nLimbType;

			++nRightLegCount;
		} else if( pBot->m_pBotInfo_Gen->pszLeftLegLimbName && !fclib_stricmp( pszLimbName, pBot->m_pBotInfo_Gen->pszLeftLegLimbName ) ) {
			// This is the left leg...

			pLimbDef->m_nLegType = LEG_TYPE_LEFT;
			pPartPool->m_nLimbTypeLeftLeg = nLimbType;

			++nLeftLegCount;
		} else {
			// This is not a leg...
			pLimbDef->m_nLegType = LEG_TYPE_NONE;
		}

		if( pLimbGameDataInfo[nLimbType].pszProxyDamageBone ) {
			pLimbDef->m_nProxyBoneIndex = fmesh_FindBone( pMesh, pLimbGameDataInfo[nLimbType].pszProxyDamageBone );
			FASSERT( pLimbDef->m_nProxyBoneIndex >= 0 );

			pLimbDef->m_ProxyBoxMinCorner_BS.Set( pLimbGameDataInfo[nLimbType].fProxyDamageBox_NegX_BS, pLimbGameDataInfo[nLimbType].fProxyDamageBox_NegY_BS, pLimbGameDataInfo[nLimbType].fProxyDamageBox_NegZ_BS );
			pLimbDef->m_ProxyBoxMaxCorner_BS.Set( pLimbGameDataInfo[nLimbType].fProxyDamageBox_PosX_BS, pLimbGameDataInfo[nLimbType].fProxyDamageBox_PosY_BS, pLimbGameDataInfo[nLimbType].fProxyDamageBox_PosZ_BS );

			// Flag as proxy bone...
			pPartPool->_SetProxyBoneBitmask( pLimbDef->m_nProxyBoneIndex );
		} else {
			pLimbDef->m_nProxyBoneIndex = -1;

			pLimbDef->m_ProxyBoxMinCorner_BS.Zero();
			pLimbDef->m_ProxyBoxMaxCorner_BS.Zero();
		}

		pLimbDef->m_pDebrisGroup = pLimbGameDataInfo[nLimbType].m_pDebrisGroup;
		pLimbDef->m_hDangleSound = pLimbGameDataInfo[nLimbType].hDangleSound;
		pLimbDef->m_hBlowOffSound = pLimbGameDataInfo[nLimbType].hBlowOffSound;
		pLimbDef->m_hBlowOffExplosion = pLimbGameDataInfo[nLimbType].hBlowOffExplosion;

		if( pLimbGameDataInfo[nLimbType].pszParticleBone ) {
			pLimbDef->m_nParticleBoneIndex = fmesh_FindBone( pMesh, pLimbGameDataInfo[nLimbType].pszParticleBone );
			FASSERT( pLimbDef->m_nParticleBoneIndex >= 0 );

			// Count the number of particle defs...
			pLimbDef->m_nParticleDefCount = 0;

			for( i=0; i<BOTPART_MAX_PARTICLES_PER_LIMB; ++i ) {
				if( pLimbGameDataInfo[nLimbType].aParticleInfo[i].hParticleDef ) {
					++pLimbDef->m_nParticleDefCount;
				}
			}

			// Allocate particle info array...
			if( pLimbDef->m_nParticleDefCount ) {
				pLimbDef->m_pParticleInfoArray = (ParticleInfo_t *)fres_Alloc( sizeof(ParticleInfo_t) * pLimbDef->m_nParticleDefCount );
				if( pLimbDef->m_pParticleInfoArray == NULL ) {
					DEVPRINTF( "CBotPartPool::RegisterClient(): Not enough memory to create m_pParticleInfoArray.\n" );
					goto _ExitWithError;
				}

				nParticleIndex = 0;
				for( i=0; i<BOTPART_MAX_PARTICLES_PER_LIMB; ++i ) {
					if( pLimbGameDataInfo[nLimbType].aParticleInfo[i].hParticleDef ) {
						pLimbDef->m_pParticleInfoArray[nParticleIndex].hParticleDef = pLimbGameDataInfo[nLimbType].aParticleInfo[i].hParticleDef;
						pLimbDef->m_pParticleInfoArray[nParticleIndex].fDuration = pLimbGameDataInfo[nLimbType].aParticleInfo[i].fDuration;
						pLimbDef->m_pParticleInfoArray[nParticleIndex].fUnitIntensity = pLimbGameDataInfo[nLimbType].aParticleInfo[i].fUnitIntensity;
						++nParticleIndex;
					}
				}
			}
		}

		for( hTable=fgamedata_GetFirstTable( hFile, Walker ); hTable!=FGAMEDATA_INVALID_TABLE_HANDLE; hTable=fgamedata_GetNextTable(Walker) ) {
			if( !fclib_stricmp( fgamedata_GetTableName(hTable), _LIMB_INFO_TABLE_NAME ) ) {
				continue;
			}

			if( !fclib_stricmp( fgamedata_GetTableName(hTable), _BONE_INFO_TABLE_NAME ) ) {
				continue;
			}

			if( !BotPartInit.LoadFromGameData( hTable, pszGameDataFileName ) ) {
				goto _ExitWithError;
			}

			if( !fclib_stricmp( pszLimbName, BotPartInit.m_pszLimbName ) ) {
				// Found one...

				// Check to see if this part name has already been added...
				for( i=0; i<nPartType; ++i ) {
					if( !fclib_stricmp( pPartPool->m_pPartTypePoolArray[i].pPartArray->m_pszPartName, fgamedata_GetTableName(hTable) ) ) {
						DEVPRINTF( "CBotPartPool::RegisterClient(): Part name '%s' appears more than once in file '%s.csv'.\n", fgamedata_GetTableName(hTable), pszGameDataFileName );
						goto _ExitWithError;
					}
				}

				FASSERT( nPartType < pPartPool->m_nPartTypeCount );

				// Create a pool of parts...
				for( i=0; i<pPartPool->m_nPartsPerPool; ++i ) {
					if( !pPartPool->m_pPartTypePoolArray[nPartType].pPartArray[i].Create( fgamedata_GetTableName(hTable), &BotPartInit, pMesh ) ) {
						goto _ExitWithError;
					}
				}

				FASSERT( nLimbType <= 255 );
				pPartPool->m_pPartTypePoolArray[nPartType].nLimbType = nLimbType;

				nBonesInPartCount = pPartPool->m_pPartTypePoolArray[nPartType].pPartArray->m_Verlet.GetContributingBoneCount();
				for( i=0; i<nBonesInPartCount; ++i ) {
					nBoneIndex = pPartPool->m_pPartTypePoolArray[nPartType].pPartArray->m_Verlet.GetBoneIndex(i);
					FASSERT( nBoneIndex < pMesh->nBoneCount );

					if( pLimbDef->m_nProxyBoneIndex == nBoneIndex ) {
						DEVPRINTF( "CBotPartPool::RegisterClient(): Proxy bone '%s' cannot also be a part bone. File '%s.csv'\n", pMesh->pBoneArray[pLimbDef->m_nProxyBoneIndex].szName, pszGameDataFileName );
						goto _ExitWithError;
					}

					if( pPartPool->m_anBoneToPartArray[nBoneIndex] != -1 ) {
						DEVPRINTF( "CBotPartPool::RegisterClient(): Warning! Bone '%s' referenced by more than one part in file '%s.csv'.\n", pMesh->pBoneArray[nBoneIndex].szName, pszGameDataFileName );
					} else {
						pPartPool->m_anBoneToPartArray[nBoneIndex] = nPartType;
					}
				}

				++nPartType;
			}
		}
	}

	if( ((nLeftLegCount + nRightLegCount) != 0) && ((nLeftLegCount + nRightLegCount) != 1) && ((nLeftLegCount + nRightLegCount) != 2) ) {
		// Weird leg count...
		DEVPRINTF( "CBotPartPool::RegisterClient(): Right/Left leg count is not 0, 1, or 2.\n" );
		goto _ExitWithError;
	}
	FASSERT( nPartType == pPartPool->m_nPartTypeCount );

_ExitSuccessful:
	fmem_ReleaseFrame( MemFrame );

	return TRUE;

_ExitWithError:
	if( *ppPartPool ) {
		UnregisterClient( *ppPartPool );
	}

	fmem_ReleaseFrame( MemFrame );
	fres_ReleaseFrame( ResFrame );

	return FALSE;
}


void CBotPartPool::_InitEffectDefs( void ) {
	FTexDef_t *pTexDef = (FTexDef_t *)fresload_Load( FTEX_RESNAME, "tfp1smoke01" );
	if( pTexDef == NULL ) {
		DEVPRINTF( "CBotPartPool::_InitEffectDefs(): Could not load smoke trail texture.\n" );
	}

	m_SmokeTrailAttrib.nFlags = SMOKETRAIL_FLAG_NONE;
	m_SmokeTrailAttrib.pTexDef = pTexDef;

	m_SmokeTrailAttrib.fScaleMin_WS = 1.0f;
	m_SmokeTrailAttrib.fScaleMax_WS = 1.3f;
	m_SmokeTrailAttrib.fScaleSpeedMin_WS = 1.2f;
	m_SmokeTrailAttrib.fScaleSpeedMax_WS = 1.5f;
	m_SmokeTrailAttrib.fScaleAccelMin_WS = -1.0f;
	m_SmokeTrailAttrib.fScaleAccelMax_WS = -1.5f;

	m_SmokeTrailAttrib.fXRandSpread_WS = 0.2f;
	m_SmokeTrailAttrib.fYRandSpread_WS = 0.2f;
	m_SmokeTrailAttrib.fDistBetweenPuffs_WS = 0.3f;
	m_SmokeTrailAttrib.fDistBetweenPuffsRandSpread_WS = 0.1f;

	m_SmokeTrailAttrib.fYSpeedMin_WS = 0.5f;
	m_SmokeTrailAttrib.fYSpeedMax_WS = 1.0f;
	m_SmokeTrailAttrib.fYAccelMin_WS = -0.2f;
	m_SmokeTrailAttrib.fYAccelMax_WS = -0.5f;

	m_SmokeTrailAttrib.fUnitOpaqueMin_WS = 0.6f;
	m_SmokeTrailAttrib.fUnitOpaqueMax_WS = 0.7f;
	m_SmokeTrailAttrib.fUnitOpaqueSpeedMin_WS = -0.5f;
	m_SmokeTrailAttrib.fUnitOpaqueSpeedMax_WS = -0.9f;
	m_SmokeTrailAttrib.fUnitOpaqueAccelMin_WS = 0.0f;
	m_SmokeTrailAttrib.fUnitOpaqueAccelMax_WS = 0.0f;

	m_SmokeTrailAttrib.StartColorRGB.Set( 1.0f, 0.5f, 0.25f );
	m_SmokeTrailAttrib.EndColorRGB.Set( 0.7f, 0.7f, 0.7f );
	m_SmokeTrailAttrib.fStartColorUnitIntensityMin = 1.0f;
	m_SmokeTrailAttrib.fStartColorUnitIntensityMax = 0.9f;
	m_SmokeTrailAttrib.fEndColorUnitIntensityMin = 0.0f;
	m_SmokeTrailAttrib.fEndColorUnitIntensityMax = 0.2f;

	m_SmokeTrailAttrib.fColorUnitSliderSpeedMin = 3.5f;
	m_SmokeTrailAttrib.fColorUnitSliderSpeedMax = 4.5f;
	m_SmokeTrailAttrib.fColorUnitSliderAccelMin = 0.0f;
	m_SmokeTrailAttrib.fColorUnitSliderAccelMax = 0.0f;

	m_hDebrisFlame = (FParticle_DefHandle_t)fresload_Load( FPARTICLE_RESTYPE, "DebrisFlam1" );
	m_hDebrisSmoke = (FParticle_DefHandle_t)fresload_Load( FPARTICLE_RESTYPE, "DebrisSmok1" );
}


CBotPart *CBotPartPool::FindFreePart( u32 nPartType ) {
	FASSERT( IsCreated() );

	if( nPartType >= m_nPartTypeCount ) {
		return NULL;
	}

	u32 i;
	CBotPart *pBotPart;

	for( i=0, pBotPart=m_pPartTypePoolArray[nPartType].pPartArray; i<m_nPartsPerPool; ++i, ++pBotPart ) {
		if( pBotPart->m_nMode == CBotPart::MODE_DISABLED ) {
			return pBotPart;
		}
	}

	return NULL;
}


// Looks in the part pool to determine if the parts for the specified limb are available.
// If so, TRUE is returned.
//
// If not, other bots with dangling limbs are searched and if a good candidate is found,
// his parts are detatched silently and placed back into the pool and TRUE is returned.
//
// If a good candidate is not found, this fuction returns FALSE.
//
// Set bRepairWhenStolen to FALSE to remove stolen limbs.
// Set bRepairWhenStolen to TRUE to make stolen limbs intact.
BOOL CBotPartPool::FindAvailablePartsForLimbDangling( u32 nLimbType, BOOL bRepairWhenStolen ) {
	FASSERT( IsCreated() );

	if( nLimbType >= m_nLimbTypeCount ) {
		return FALSE;
	}

	CBotPart *pBotPart;
	CBot *pBot, *pBestBot;
	s32 i, nBestPartIndex, nBestWeight, nWeight;
	u8 nPartType;
	f32 fDist2, fDistSquaredToNearestPlayer;

	pBestBot = NULL;
	nBestPartIndex = -1;
	nBestWeight = 0;
	fDistSquaredToNearestPlayer = FMATH_MAX_FLOAT;

	nPartType = m_pLimbDefArray[nLimbType].m_nStartPartType;

	for( i=0, pBotPart=m_pPartTypePoolArray[nPartType].pPartArray; i<(s32)m_nPartsPerPool; ++i, ++pBotPart ) {
		if( pBotPart->m_nMode == CBotPart::MODE_DISABLED ) {
			// This part is available for use...
			return TRUE;
		}

		if( pBotPart->m_nMode != CBotPart::MODE_DANGLING ) {
			// Not dangling, so skip this part...
			continue;
		}

		// Found a dangling part...

		pBot = pBotPart->m_pDanglingFromBot;

		nWeight = 0;

		if( !(pBot->m_pWorldMesh->GetVolumeFlags() & FVIS_VOLUME_IN_ACTIVE_LIST) ) {
			// This bot is no longer in the active list. Flag as super-high priority...

			nWeight = 2;
		} else {
			// This bot is still in the active list...

			if( !pBot->m_pWorldMesh->HaveDrawn() ) {
				// We did not draw this bot last frame...

				nWeight = 1;
			}
		}

		fDist2 = pBot->ComputeDistSquaredToNearestHumanPlayer();

		if( (nWeight > nBestWeight) || ( (nWeight == nBestWeight) && (fDist2 > fDistSquaredToNearestPlayer) ) ) {
			// This is the better candidate...

			pBestBot = pBot;
			nBestPartIndex = i;
			nBestWeight = nWeight;
			fDistSquaredToNearestPlayer = fDist2;
		}
	}

	if( nBestWeight == 0 ) {
		// No available parts to dangle this limb...
		return FALSE;
	}

	// We found a good candidate from which to steal his parts...

	FASSERT( nBestPartIndex >= 0 );
	FASSERT( pBestBot );

	if( bRepairWhenStolen ) {
		pBestBot->m_pPartMgr->SetState_Intact( nLimbType );
	} else {
		pBestBot->m_pPartMgr->SetState_Removed( nLimbType, TRUE, FALSE );
	}

	return TRUE;
}


void CBotPartPool::UnregisterClient( CBotPartPool *pPartPool ) {
	if( pPartPool ) {
		FASSERT( pPartPool->m_nClientCount );

		// One less clients...
		--pPartPool->m_nClientCount;

		if( pPartPool->m_nClientCount == 0 ) {
			// No more clients...

			fdelete( pPartPool );
		}
	}
}


CBotPartPool::CBotPartPool() {
	_ClearDataMembers();
}


CBotPartPool::~CBotPartPool() {
	u32 i;

	if( m_ppPartPool ) {
		*m_ppPartPool = NULL;
	}

	if( m_pPartTypePoolArray ) {
		for( i=0; i<m_nPartTypeCount; ++i ) {
			fdelete_array( m_pPartTypePoolArray[i].pPartArray );
		}
	}

	fdelete_array( m_pLimbDefArray );

	_ClearDataMembers();
}


void CBotPartPool::_ClearDataMembers( void ) {
	m_ppPartPool = NULL;
	m_nClientCount = 0;
	m_nPartsPerPool = 0;
	m_nPartTypeCount = 0;
	m_nLimbTypeCount = 0;
	m_nBoneCount= 0;
	m_pPartTypePoolArray = NULL;
	m_pLimbDefArray = NULL;
	m_anBoneToPartArray = NULL;
	m_pnBoneIsProxyBitmask = NULL;
	m_pnOrigBoneCallbackEnabledBitmask = NULL;
}




//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CBotPartMgr
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

CDamageResult CBotPartMgr::m_DamageResult;
CBotPartMgr *CBotPartMgr::m_pCallbackPartMgr;


BOOL CBotPartMgr::Create( CBot *pBot, CBotPartPool **ppPartPool, cchar *pszGameDataFileName, u32 nPartsPerPoolCount, u32 nLimbTypeCount ) {
	FASSERT( !IsCreated() );
	FASSERT( pBot );
	FASSERT( ppPartPool );
	FASSERT( pszGameDataFileName );
	FASSERT( nLimbTypeCount );

	u32 i;

	FResFrame_t ResFrame = fres_GetFrame();

	#if !FANG_PRODUCTION_BUILD
		u32 nFreeBytes = fres_GetFreeBytes();
	#endif

	if( !CBotPartPool::RegisterClient( ppPartPool, pBot, pszGameDataFileName, nPartsPerPoolCount, pBot->m_pWorldMesh->m_pMesh ) ) {
		goto _ExitWithError;
	}

	m_pBot = pBot;
	m_pPartPool = *ppPartPool;
	m_fBotInitialNormHealth = m_pBot->NormHealth();
	m_nPartTypeCount = m_pPartPool->m_nPartTypeCount;
	m_nLimbTypeCount = m_pPartPool->m_nLimbTypeCount;
	m_nActiveBoneMapCount = 0;
	m_nFlags = FLAG_NONE;
	m_nBoneGeoDebrisCount = 0;
	m_fSecsUntilNextLimbBlowSound = 0;
	m_fSecsUntilNextDebrisBlowSound = 0;
	m_nTintRed = 255;
	m_nTintGreen = 255;
	m_nTintBlue = 255;

	if( m_nLimbTypeCount != nLimbTypeCount ) {
		DEVPRINTF( "CBotPartMgr::Create(): Limb type counts of code (%u) and data file (%u) don't match. File '%s.csv'.\n", nLimbTypeCount, m_nLimbTypeCount, pszGameDataFileName );
		goto _ExitWithError;
	}

	m_pPartStateArray = (PartState_t *)fres_Alloc( sizeof(PartState_t) * m_nPartTypeCount );
	if( m_pPartStateArray == NULL ) {
		DEVPRINTF( "CBotPartMgr::Create(): Not enough memory to allocate m_pPartStateArray.\n" );
		goto _ExitWithError;
	}

	for( i=0; i<m_nPartTypeCount; ++i ) {
		m_pPartStateArray[i].pDanglingBotPart = NULL;
	}

	m_pLimbStateArray = (LimbState_t *)fres_Alloc( sizeof(LimbState_t) * m_nLimbTypeCount );
	if( m_pLimbStateArray == NULL ) {
		DEVPRINTF( "CBotPartMgr::Create(): Not enough memory to allocate m_pLimbStateArray.\n" );
		goto _ExitWithError;
	}

	for( i=0; i<m_nLimbTypeCount; ++i ) {
		m_pLimbStateArray[i].nState = LIMB_STATE_INTACT;
		m_pLimbStateArray[i].nFlags = LIMB_STATE_FLAG_NONE;
		m_pLimbStateArray[i].fNormHealth = m_fBotInitialNormHealth * m_pPartPool->m_pLimbDefArray[i].m_fInitialHealthMultIntact;
	}

	m_pBoneMapArray = (BoneMap_t *)fres_Alloc( sizeof(BoneMap_t) * m_nPartTypeCount );
	if( m_pBoneMapArray == NULL ) {
		DEVPRINTF( "CBotPartMgr::Create(): Not enough memory to allocate m_pBoneMapArray.\n" );
		goto _ExitWithError;
	}

	m_nSeveredBoneWordCount = (m_pBot->m_pWorldMesh->m_pMesh->nBoneCount + 31) >> 5;

	m_pnSeveredBoneBitmask = (u32 *)fres_AllocAndZero( sizeof(u32) * m_nSeveredBoneWordCount );
	if( m_pnSeveredBoneBitmask == NULL ) {
		DEVPRINTF( "CBotPartMgr::Create(): Not enough memory to allocate m_pnSeveredBoneBitmask.\n" );
		goto _ExitWithError;
	}

	_UpdateAllComponentStatus();

	m_nSeveredBoneCount = 0;

	#if !FANG_PRODUCTION_BUILD
//		DEVPRINTF( "Bot Part System used %u bytes of memory (data file '%s').\n", nFreeBytes - fres_GetFreeBytes(), pszGameDataFileName );
	#endif

	return TRUE;

_ExitWithError:
	Destroy();
	fres_ReleaseFrame( ResFrame );
	return FALSE;
}


void CBotPartMgr::Destroy( void ) {
	if( IsCreated() ) {
		m_pPartStateArray = NULL;
		m_pLimbStateArray = NULL;

		CBotPartPool::UnregisterClient( m_pPartPool );
		m_pPartPool = NULL;
	}
}


void CBotPartMgr::SetDebrisChunkTintColor( const CFColorRGB *pTintColorRGB ) {
	FASSERT( IsCreated() );

	m_nTintRed = (u8)(255.0f * pTintColorRGB->fRed);
	m_nTintGreen = (u8)(255.0f * pTintColorRGB->fGreen);
	m_nTintBlue = (u8)(255.0f * pTintColorRGB->fBlue);

	if( (m_nTintRed & m_nTintGreen & m_nTintBlue) < 255 ) {
		FMATH_SETBITMASK( m_nFlags, FLAG_TINT_DEBRIS_CHUNKS );
	} else {
		FMATH_CLEARBITMASK( m_nFlags, FLAG_TINT_DEBRIS_CHUNKS );
	}
}


void CBotPartMgr::Work( void ) {
	FASSERT( IsCreated() );

	if( m_fSecsUntilNextLimbBlowSound > 0.0f ) {
		m_fSecsUntilNextLimbBlowSound -= FLoop_fPreviousLoopSecs;
		FMATH_CLAMPMIN( m_fSecsUntilNextLimbBlowSound, 0.0f );
	}

	if( m_fSecsUntilNextDebrisBlowSound > 0.0f ) {
		m_fSecsUntilNextDebrisBlowSound -= FLoop_fPreviousLoopSecs;
		FMATH_CLAMPMIN( m_fSecsUntilNextDebrisBlowSound, 0.0f );
	}
}


void CBotPartMgr::_UpdateAllComponentStatus( void ) {
	u32 i;

	for( i=0; i<COMPONENT_TYPE_COUNT; ++i ) {
		_UpdateOneComponentStatus( (ComponentType_e)i );
	}

	m_nFlags &= ~FLAG_BACK_BROKEN;
	for( i=0; i<m_nLimbTypeCount; i++ ) {
		if( m_pPartPool->m_pLimbDefArray[i].m_nDangleRule == CBotPartPool::DANGLE_RULE_BACK_BREAKER ) {
			m_nFlags |= (m_pLimbStateArray[i].nState != LIMB_STATE_INTACT);
		}
	}
}


void CBotPartMgr::_UpdateOneComponentStatus( ComponentType_e nComponentType ) {
	u32 nLimbType, nComponentTypeBit, nLimbCount, nDanglingLimbCount, nIntactLimbCount;
	ComponentStatus_e nComponentStatus;

	nLimbCount = 0;
	nDanglingLimbCount = 0;
	nIntactLimbCount = 0;

	nComponentTypeBit = 1 << nComponentType;

	for( nLimbType=0; nLimbType<m_nLimbTypeCount; ++nLimbType ) {
		if( m_pPartPool->m_pLimbDefArray[nLimbType].m_nComponentTypeFlags & nComponentTypeBit ) {
			// We found a limb with a matching component type...

			++nLimbCount;

			if( m_pLimbStateArray[nLimbType].nState == LIMB_STATE_DANGLING ) {
				++nDanglingLimbCount;
			} else if( m_pLimbStateArray[nLimbType].nState == LIMB_STATE_INTACT ) {
				++nIntactLimbCount;
			}
		}
	}

	if( nLimbCount == 0 ) {
		// No limbs of this component type...

		nComponentStatus = COMPONENT_STATUS_ALL_FULLY_OPERATIONAL;
	} else {
		// There's at least one limb of this component type...

		if( nIntactLimbCount == nLimbCount ) {
			// All limbs of this component type are intact...

			nComponentStatus = COMPONENT_STATUS_ALL_FULLY_OPERATIONAL;
		} else {
			// Not all limbs of this component type are intact...

			if( nIntactLimbCount ) {
				// At least one limb of this component type is intact...

				nComponentStatus = COMPONENT_STATUS_SOME_FULLY_OPERATIONAL;
			} else {
				// No limbs of this component type are intact...

				if( nDanglingLimbCount ) {
					// At least one limb of this component type is dangling...

					nComponentStatus = COMPONENT_STATUS_SOME_PARTIALLY_OPERATIONAL;
				} else {
					// All limbs of this component type are removed...

					nComponentStatus = COMPONENT_STATUS_NONE_OPERATIONAL;
				}
			}
		}
	}

	m_nComponentStatus[nComponentType] = nComponentStatus;
}


void CBotPartMgr::SetState_BlowUp( u32 nLimbType, BOOL bDontBlowUpIfInvincible ) {
	FASSERT( IsCreated() );

	if( nLimbType >= m_nLimbTypeCount ) {
		return;
	}

	if( m_pLimbStateArray[nLimbType].nState == LIMB_STATE_REMOVED ) {
		// Limb is already removed...
		return;
	}

	if( bDontBlowUpIfInvincible && (m_pLimbStateArray[nLimbType].nFlags & LIMB_STATE_FLAG_INVINCIBLE) ) {
		// Invincible...
		return;
	}

	_BlowOffLimbIntoPieces( nLimbType );
}


void CBotPartMgr::SetState_Dangle( u32 nLimbType, BOOL bDontDangleIfInvincible, f32 fGravity ) {
	FASSERT( IsCreated() );

	if( nLimbType >= m_nLimbTypeCount ) {
		return;
	}

	if( m_pLimbStateArray[nLimbType].nState == LIMB_STATE_DANGLING ) {
		// Limb is already dangling...
		return;
	}

	if( m_pLimbStateArray[nLimbType].nState == LIMB_STATE_REMOVED ) {
		// Cannot dangle a removed limb...
		return;
	}

	if( bDontDangleIfInvincible && (m_pLimbStateArray[nLimbType].nFlags & LIMB_STATE_FLAG_INVINCIBLE) ) {
		// Invincible...
		return;
	}

	FASSERT( m_pLimbStateArray[nLimbType].nState == LIMB_STATE_INTACT );

	// Set dangle health...
	m_pLimbStateArray[nLimbType].fNormHealth = m_fBotInitialNormHealth * m_pPartPool->m_pLimbDefArray[nLimbType].m_fInitialHealthMultDangle;

	if( m_pLimbStateArray[nLimbType].fNormHealth == 0.0f ) {
		// If health is 0, this part doesn't dangle.  Just lose it...
		_BlowOffLimbIntoPieces( nLimbType );
		return;
	}

	if( m_pPartPool->m_pLimbDefArray[nLimbType].m_nLegType == CBotPartPool::LEG_TYPE_RIGHT ) {
		// This is the right leg...

		if( m_nFlags & FLAG_LEFT_LEG_NOT_INTACT ) {
			// Left leg is not intact. Destroy both legs...
			if( m_pBot->IsTorsoBlownOff() ) {
				DestroyBot( NULL );
			} else {
				_BlowOffLimbIntoPieces( m_pPartPool->m_nLimbTypeLeftLeg );
				_BlowOffLimbIntoPieces( m_pPartPool->m_nLimbTypeRightLeg );
			}
			return;
		}

		FMATH_SETBITMASK( m_nFlags, FLAG_RIGHT_LEG_NOT_INTACT );

		m_pBot->GiveBotLimp( TRUE );
	}

	if( m_pPartPool->m_pLimbDefArray[nLimbType].m_nLegType == CBotPartPool::LEG_TYPE_LEFT ) {
		// This is the left leg...

		if( m_nFlags & FLAG_RIGHT_LEG_NOT_INTACT ) {
			// Right leg is not intact. Destroy both legs...
			if( m_pBot->IsTorsoBlownOff() ) {
				DestroyBot( NULL );
			} else {
				_BlowOffLimbIntoPieces( m_pPartPool->m_nLimbTypeRightLeg );
				_BlowOffLimbIntoPieces( m_pPartPool->m_nLimbTypeLeftLeg );
			}

			return;
		}

		FMATH_SETBITMASK( m_nFlags, FLAG_LEFT_LEG_NOT_INTACT );

		m_pBot->GiveBotLimp( FALSE );
	}

	// Make sure all parts are available for this limb...
	if( !m_pPartPool->FindAvailablePartsForLimbDangling( nLimbType, m_pLimbStateArray[nLimbType].nFlags & LIMB_STATE_FLAG_REPAIR_WHEN_STOLEN ) ) {
		// Nope...
		return;
	}

	// All parts are available. Let's do it...

	m_pLimbStateArray[nLimbType].nState = LIMB_STATE_DANGLING;
	_UpdateAllComponentStatus();

	// Dangle all parts in this limb...
	u32 nPartType, nStartPartType, nEndPartType;

	nStartPartType = m_pPartPool->m_pLimbDefArray[nLimbType].m_nStartPartType;
	nEndPartType = m_pPartPool->m_pLimbDefArray[nLimbType].m_nEndPartType;

	for( nPartType=nStartPartType; nPartType<nEndPartType; ++nPartType ) {
		_Part_Dangle( nPartType );

		m_pPartStateArray[nPartType].pDanglingBotPart->m_Verlet.SetGravity( fGravity );
	}

	// Make sound effect...
	u32 nFirstPartBaseBoneIndex = m_pPartStateArray[nStartPartType].pDanglingBotPart->m_Verlet.GetBaseBoneIndex();

	fsndfx_Play3D(
		m_pPartPool->m_pLimbDefArray[nLimbType].m_hDangleSound,
		&m_pBot->m_pWorldMesh->GetBoneMtxPalette()[ nFirstPartBaseBoneIndex ]->m_vPos,
		200.0f,
		1.0f,
		1.0
	);
}


// Will return TRUE if a limb was found to dangle.
// Otherwise FALSE is returned if everything's already broken.
BOOL CBotPartMgr::SetRandomLimb_Dangle( BOOL bDontSelectInvincibleLimbs ) {
	u32 i, nEligibleLimbs, nLimbToBreak;
	s32 nBackBreaker;
	BOOL bLegBroken;

	nEligibleLimbs = 0;
	nBackBreaker = -1;
	bLegBroken = m_nFlags & (FLAG_LEFT_LEG_NOT_INTACT | FLAG_RIGHT_LEG_NOT_INTACT);

	// Loop once through the array, decide how many candidates there are...
	for( i=0; i<m_nLimbTypeCount; i++ ) {
		if( m_pLimbStateArray[i].nState == LIMB_STATE_INTACT ) {
			if( bDontSelectInvincibleLimbs && (m_pLimbStateArray[i].nFlags & LIMB_STATE_FLAG_INVINCIBLE) ) {
				// Invincible...
				continue;
			}

			if( m_pPartPool->m_pLimbDefArray[i].m_nDangleRule == CBotPartPool::DANGLE_RULE_BACK_BREAKER ) {
				nBackBreaker = i;
			} else if( (m_pPartPool->m_pLimbDefArray[i].m_nLegType != CBotPartPool::LEG_TYPE_NONE) && bLegBroken ) {
				// This is a leg and one is already broken, so skip it...
			} else {
				nEligibleLimbs++;
			}
		}
	}

	// If there weren't any eligible limbs, see if the backbreaker was found.
	// If so, break it, Otherwise return FALSE, nothing left to do.
	if( nEligibleLimbs == 0 ) {
		if( nBackBreaker >= 0 ) {
			if( !bDontSelectInvincibleLimbs || !(m_pLimbStateArray[nBackBreaker].nFlags & LIMB_STATE_FLAG_INVINCIBLE) ) {
				SetState_Dangle( nBackBreaker, bDontSelectInvincibleLimbs );
				return TRUE;
			}
		}

		// If we have any limbs, just play limb 0's sound
		// (might have to rethink this later)...

		if( m_nLimbTypeCount > 0 ) {
			// Play the sound...
			fsndfx_Play3D( m_pPartPool->m_pLimbDefArray[0].m_hDangleSound, &m_pBot->MtxToWorld()->m_vPos, 200.0f, 1.0f, 1.0 );
		}

		// All broken up...
		return FALSE;
	}

	nLimbToBreak = fmath_RandomChoice( nEligibleLimbs );

	for( i=0; i<m_nLimbTypeCount; i++ ) {
		if( (m_pLimbStateArray[i].nState == LIMB_STATE_INTACT) && 
			(m_pPartPool->m_pLimbDefArray[i].m_nDangleRule != CBotPartPool::DANGLE_RULE_BACK_BREAKER) &&
			((m_pPartPool->m_pLimbDefArray[i].m_nLegType == CBotPartPool::LEG_TYPE_NONE) || !bLegBroken) ) {

			if( bDontSelectInvincibleLimbs && (m_pLimbStateArray[i].nFlags & LIMB_STATE_FLAG_INVINCIBLE) ) {
				// Invincible...
				continue;
			}

			// This is eligible...
			if( nLimbToBreak == 0 ) {
				SetState_Dangle( i, bDontSelectInvincibleLimbs );
				return TRUE;
			} else {
				nLimbToBreak--;
			}
		}
	}

	// Should never get here...
	FASSERT_NOW;

	return FALSE;
}


void CBotPartMgr::SetAllLimbs_Dangle( BOOL bDontSelectInvincibleLimbs/*=TRUE */) {
	FASSERT( IsCreated() );

	// check each limb as we go through to see if one changed states.  If so, a sound should
	// have already been played, otherwise, play one.

	BOOL bSoundPlayed = FALSE;
	LimbState_e limbState;

	for( u32 i=0; i<m_nLimbTypeCount; i++ ) {
		limbState = GetLimbState( i );
		SetState_Dangle( i, bDontSelectInvincibleLimbs );
		if( limbState != GetLimbState( i ) ) {
			bSoundPlayed = TRUE;
		}
	}

	// If we have any limbs, just play limb 0's sound
	if( (m_nLimbTypeCount > 0) && !bSoundPlayed ) {
		// Play the sound...
		fsndfx_Play3D( m_pPartPool->m_pLimbDefArray[0].m_hDangleSound, &m_pBot->MtxToWorld()->m_vPos, 200.0f, 1.0f, 1.0 );
	}


}


// If limb is dangling, this function will quickly pull the limb back into position.
void CBotPartMgr::RepairLimb( u32 nLimbType ) {
	FASSERT( IsCreated() );
	FASSERT( nLimbType < m_nLimbTypeCount );

	if( m_pLimbStateArray[ nLimbType ].nState != LIMB_STATE_DANGLING ) {
		// We don't support the repairing of non-dangling limbs because
		// we might have to kill off some bone geo debris objects and
		// I'm not sure how to do that right now...

		return;
	}

	u32 nPartType;
	CBotPart *pBotPart;

	for( nPartType=m_pPartPool->m_pLimbDefArray[nLimbType].m_nStartPartType; nPartType<m_pPartPool->m_pLimbDefArray[nLimbType].m_nEndPartType; nPartType++ ) {
		pBotPart = m_pPartStateArray[nPartType].pDanglingBotPart;

		if( pBotPart ) {
			pBotPart->SetMode_Repair();
		}
	}

	m_pLimbStateArray[nLimbType].nState = LIMB_STATE_INTACT;
	m_pLimbStateArray[nLimbType].fNormHealth = m_fBotInitialNormHealth * m_pPartPool->m_pLimbDefArray[nLimbType].m_fInitialHealthMultIntact;

	_UpdateAllComponentStatus();    
}


void CBotPartMgr::_Part_Dangle( u32 nPartType ) {
	FASSERT( IsCreated() );

	// It's possible that there is a part connected here that needs to be dangled.
	// If so, disable it...
	_Part_Disable( nPartType );

	// Grab a part from the pool...
	CBotPart *pBotPart = m_pPartPool->FindFreePart( nPartType );
	FASSERT( pBotPart );

	// Set the part's mode to dangle...
	pBotPart->SetMode_Dangling( m_pBot );

	// Enable the anim bone callback for the base Verlet bone...
	u32 nBaseBoneIndex = pBotPart->m_Verlet.GetBaseBoneIndex();
	m_pBot->m_Anim.m_pAnimCombiner->EnableBoneCallback( nBaseBoneIndex );

	// It's possible for the limb to be intact but for the part to still be attached while it's being repaired.
	// If this is the case, just disable and clear the part...
	if( m_pPartStateArray[nPartType].pDanglingBotPart != NULL ) {
		m_pPartStateArray[nPartType].pDanglingBotPart->SetMode_Disabled();
		m_pPartStateArray[nPartType].pDanglingBotPart = NULL;
	}

	// Update our state info...
	m_pPartStateArray[nPartType].pDanglingBotPart = pBotPart;

	// Update our bone map...
	#if FANG_DEBUG_BUILD || FANG_TEST_BUILD
		// This part type shouldn't already exist in our bone map...
		for( u32 nBoneMapIndex=0; nBoneMapIndex<m_nActiveBoneMapCount; ++nBoneMapIndex ) {
			FASSERT( m_pBoneMapArray[nBoneMapIndex].nPartType != nPartType );
		}
	#endif

	FASSERT( m_nActiveBoneMapCount < m_nPartTypeCount );

	m_pBoneMapArray[m_nActiveBoneMapCount].nPartType = nPartType;
	m_pBoneMapArray[m_nActiveBoneMapCount].nBoneIndex = nBaseBoneIndex;

	++m_nActiveBoneMapCount;

	// Set up anchors for the dangling part...
	u32 i, j, nAnchorIndex;
	CBotPart::AnchorDef_t *pAnchorDef;
	CFVerletTack *pAnchorToThisTack;
	cchar *pszAnchorBoneName;

	pBotPart->m_DanglingBoneAnimDrivenPos_WS = m_pBot->m_pWorldMesh->GetBoneMtxPalette()[ nBaseBoneIndex ]->m_vPos;

	for( nAnchorIndex=0, pAnchorDef=pBotPart->m_pAnchorDefArray; nAnchorIndex<pBotPart->m_nAnchorCount; ++nAnchorIndex, ++pAnchorDef ) {
		switch( (CBotPart::AttachType_e)pAnchorDef->nAttachType ) {
		case CBotPart::ATTACH_TYPE_DYNAMIC_MTX_PALETTE:
			{
				const CFVec3A *pPos_WS = &m_pBot->m_pWorldMesh->GetBoneMtxPalette()[ pAnchorDef->nAnchorBoneIndex ]->m_vPos;
				pAnchorDef->pTack->Anchor_SetToImmovablePoint_Dynamic( pPos_WS );
			}

			break;

		case CBotPart::ATTACH_TYPE_DYNAMIC_ANIM_OUTPUT:
			pAnchorDef->pTack->Anchor_SetToImmovablePoint_Dynamic( &pBotPart->m_DanglingBoneAnimDrivenPos_WS );

			break;

		case CBotPart::ATTACH_TYPE_BONED:
			{
				const CFMtx43A *pParentMtx;
				const CFVec3A *pAnchorPos_MS;

				pAnchorPos_MS = &m_pBot->m_pWorldMesh->m_pMesh->pBoneArray[ pAnchorDef->nAnchorBoneIndex ].AtRestBoneToModelMtx.m_vPos;

				if( pAnchorDef->nParentBoneIndex != 255 ) {
					const CFVec3A *pParentPos_MS;
					CFVec3A Pos_BS;

					pParentMtx = m_pBot->m_pWorldMesh->GetBoneMtxPalette()[ pAnchorDef->nParentBoneIndex ];

					pParentPos_MS = &m_pBot->m_pWorldMesh->m_pMesh->pBoneArray[ pAnchorDef->nParentBoneIndex ].AtRestBoneToModelMtx.m_vPos;

					Pos_BS.Sub( *pAnchorPos_MS, *pParentPos_MS );
					m_pBot->m_pWorldMesh->m_pMesh->pBoneArray[ pAnchorDef->nParentBoneIndex ].AtRestModelToBoneMtx.MulDir( Pos_BS );

					pAnchorDef->pTack->Anchor_SetToImmovablePoint_Boned_BS( pParentMtx, &Pos_BS );
				} else {
					pParentMtx = &m_pBot->m_pWorldMesh->m_Xfm.m_MtxF;
					pAnchorDef->pTack->Anchor_SetToImmovablePoint_Boned_BS( pParentMtx, pAnchorPos_MS );
				}
			}

			break;

		case CBotPart::ATTACH_TYPE_PART:
			// See if the part we're attaching to is dangling...
			for( i=0; i<m_nPartTypeCount; ++i ) {
				if( i == nPartType ) {
					// This is our part, so skip it...
					continue;
				}

				if( m_pPartStateArray[i].pDanglingBotPart == NULL ) {
					// This part is still in the free pool...
					continue;
				}

				if( m_pPartStateArray[i].pDanglingBotPart->m_nMode != CBotPart::MODE_DANGLING ) {
					// This part isn't dangling...
					continue;
				}

				if( !fclib_stricmp( m_pPartStateArray[i].pDanglingBotPart->m_pszPartName, pAnchorDef->pszAnchorPartName ) ) {
					// This isn't the part we're attaching to...
					break;
				}
			}

			if( i < m_nPartTypeCount ) {
				// The part we're trying to anchor to is also dangling...

				pszAnchorBoneName = m_pBot->m_pWorldMesh->m_pMesh->pBoneArray[ pAnchorDef->nAnchorBoneIndex ].szName;

				pAnchorToThisTack = m_pPartStateArray[i].pDanglingBotPart->m_Verlet.Tack_FindByName( pszAnchorBoneName );
				if( pAnchorToThisTack ) {
					pAnchorDef->pTack->Anchor_SetToTack( pAnchorToThisTack );
				} else {
					DEVPRINTF( "CBotPartMgr::SetState_Dangle(): Could not find tack '%s' on part '%s'.\n", pszAnchorBoneName, pAnchorDef->pszAnchorPartName );
					pAnchorDef->pTack->Anchor_SetToImmovablePoint_Boned( m_pBot->m_pWorldMesh, pAnchorDef->nAnchorBoneIndex );
				}
			} else {
				// The part we're trying to anchor to is not dangling...

				const CFMtx43A *pParentMtx;
				const CFVec3A *pPos_BS;

				if( pAnchorDef->nParentBoneIndex != 255 ) {
					pParentMtx = m_pBot->m_pWorldMesh->GetBoneMtxPalette()[ pAnchorDef->nParentBoneIndex ];
				} else {
					pParentMtx = &m_pBot->m_pWorldMesh->m_Xfm.m_MtxF;
				}

				pPos_BS = &m_pBot->m_pWorldMesh->m_pMesh->pBoneArray[ pAnchorDef->nAnchorBoneIndex ].AtRestBoneToParentMtx.m_vPos;

				pAnchorDef->pTack->Anchor_SetToImmovablePoint_Boned_BS( pParentMtx, pPos_BS );
			}

			break;

		default:
			FASSERT_NOW;
		};

		pAnchorDef->pTack->Anchor_SetRange( pAnchorDef->fMinDist, pAnchorDef->fMaxDist );
		pAnchorDef->pTack->Anchor_Enable( TRUE );
	}

	// Now see if any other parts' anchors should be anchored to our part now that we're dangling...

	for( i=0; i<m_nPartTypeCount; ++i ) {
		if( i == nPartType ) {
			// This is our part, so skip it...
			continue;
		}

		if( m_pPartStateArray[i].pDanglingBotPart == NULL ) {
			// This part is still in the free pool...
			continue;
		}

		if( m_pPartStateArray[i].pDanglingBotPart->m_nMode != CBotPart::MODE_DANGLING ) {
			// This part isn't dangling...
			continue;
		}

		// Found another dangling part...

		for( j=0; j<m_pPartStateArray[i].pDanglingBotPart->m_nAnchorCount; ++j ) {
			pAnchorDef = &m_pPartStateArray[i].pDanglingBotPart->m_pAnchorDefArray[j];

			if( pAnchorDef->nAttachType != CBotPart::ATTACH_TYPE_PART ) {
				// This anchor isn't attached to a part...
				continue;
			}

			// This anchor is attached to a part...

			if( !fclib_stricmp( pAnchorDef->pszAnchorPartName, pBotPart->m_pszPartName ) ) {
				// This anchor is supposed to be attached to us...

				pszAnchorBoneName = m_pBot->m_pWorldMesh->m_pMesh->pBoneArray[ pAnchorDef->nAnchorBoneIndex ].szName;

				pAnchorToThisTack = pBotPart->m_Verlet.Tack_FindByName( pszAnchorBoneName );

				if( pAnchorToThisTack ) {
					pAnchorDef->pTack->Anchor_SetToTack( pAnchorToThisTack );
				} else {
					DEVPRINTF( "CBotPartMgr::SetState_Dangle(): Could not find tack '%s' on part '%s'.\n", pszAnchorBoneName, pAnchorDef->pszAnchorPartName );
				}
			}
		}
	}
}


// Will check to make sure that if this part is connected, it is disabled.
void CBotPartMgr::_Part_Disable( u32 nPartType ) {
	u8 i;

	// First clear the part and the m_pPartStateArray...
	CBotPart *pBotPart = m_pPartStateArray[nPartType].pDanglingBotPart;
	if( pBotPart ) {
		pBotPart->SetMode_Disabled();
		m_pPartStateArray[nPartType].pDanglingBotPart = NULL;
	}

	// Now clear the bone map...
	for( i=0; i<m_nActiveBoneMapCount; i++ ) {
		if( m_pBoneMapArray[i].nPartType == nPartType ) {
			if( i < (m_nActiveBoneMapCount - 1) ) {
				m_pBoneMapArray[i] = m_pBoneMapArray[ m_nActiveBoneMapCount - 1 ];
			}

			--m_nActiveBoneMapCount;
		}
	}
}


void CBotPartMgr::SetState_Removed( u32 nLimbType, BOOL bSilentlyVanish, BOOL bDontRemoveIfInvincible ) {
	FASSERT( IsCreated() );
	FASSERT( nLimbType>=0 && nLimbType<m_nLimbTypeCount );

	if( m_pLimbStateArray[nLimbType].nState == LIMB_STATE_REMOVED ) {
		// Already removed...
		return;
	}

	if( bDontRemoveIfInvincible && (m_pLimbStateArray[nLimbType].nFlags & LIMB_STATE_FLAG_INVINCIBLE) ) {
		// Invincible...
		return;
	}

	_BlowOffLimbIntoPieces( nLimbType, FALSE, bSilentlyVanish );
}


// Returns TRUE if the debris object should be killed.
BOOL CBotPartMgr::_KillDebrisCallback( const CFDebris *pDebris ) {
	if( (u32)pDebris->GetDebrisDef()->m_pUser != (u32)m_pCallbackPartMgr ) {
		// Not this bot's debris...
		return FALSE;
	}

	if( pDebris->GetDebrisDef()->m_nUser == 0 ) {
		// Don't kill this debris...
		return FALSE;
	}

	// Kill this debris...

	return TRUE;
}


void CBotPartMgr::ResetAllToIntact( void ) {
	FASSERT( IsCreated() );

	u32 i, nLimbType, nPartType, nStartPartType, nEndPartType;

	// Kill off any bone geo debris objects...
	m_pCallbackPartMgr = this;
	CFDebris::KillAllSelective( DEBRISTYPES_BOT_PART_SYSTEM, _KillDebrisCallback, FALSE );

	for( nLimbType=0; nLimbType<m_nLimbTypeCount; ++nLimbType ) {
		if( m_pLimbStateArray[nLimbType].nState == LIMB_STATE_DANGLING ) {
			nStartPartType = m_pPartPool->m_pLimbDefArray[nLimbType].m_nStartPartType;
			nEndPartType = m_pPartPool->m_pLimbDefArray[nLimbType].m_nEndPartType;

			for( nPartType=nStartPartType; nPartType<nEndPartType; ++nPartType ) {
				m_pPartStateArray[nPartType].pDanglingBotPart->SetMode_Disabled();
				m_pPartStateArray[nPartType].pDanglingBotPart = NULL;
			}
		}

		m_pLimbStateArray[nLimbType].nState = LIMB_STATE_INTACT;
		m_pLimbStateArray[nLimbType].fNormHealth = m_fBotInitialNormHealth * m_pPartPool->m_pLimbDefArray[nLimbType].m_fInitialHealthMultIntact;
	}

	m_nActiveBoneMapCount = 0;
	m_nSeveredBoneCount = 0;
	m_nBoneGeoDebrisCount = 0;

	// Clear all flags except debris chunk tinting flag...
	m_nFlags &= FLAG_TINT_DEBRIS_CHUNKS;

	for( i=0; i<m_nSeveredBoneWordCount; ++i ) {
		m_pnSeveredBoneBitmask[i] = 0;
	}

	for( i=0; i<m_pPartPool->m_nBoneCount; ++i ) {
		if( m_pPartPool->_GetOrigBoneCallbackEnabledBitmask( i ) ) {
			m_pBot->m_Anim.m_pAnimCombiner->EnableBoneCallback( i );
		} else {
			m_pBot->m_Anim.m_pAnimCombiner->DisableBoneCallback( i );
		}
	}

	_UpdateAllComponentStatus();

	m_fSecsUntilNextLimbBlowSound = 0;
	m_fSecsUntilNextDebrisBlowSound = 0;
}


// Snaps the limb back to an intact state. Works only with dangling limbs.
void CBotPartMgr::SetState_Intact( u32 nLimbType ) {
	FASSERT( IsCreated() );
	FASSERT( nLimbType < m_nLimbTypeCount );

	if( m_pLimbStateArray[ nLimbType ].nState != LIMB_STATE_DANGLING ) {
		// We don't support the repairing of non-dangling limbs because
		// we might have to kill off some bone geo debris objects and
		// I'm not sure how to do that right now...

		return;
	}

	u32 nPartType, nStartPartType, nEndPartType;
	s32 nBoneIndex;
	CBotPart *pBotPart;

	nStartPartType = m_pPartPool->m_pLimbDefArray[nLimbType].m_nStartPartType;
	nEndPartType = m_pPartPool->m_pLimbDefArray[nLimbType].m_nEndPartType;

	for( nPartType=nStartPartType; nPartType<nEndPartType; ++nPartType ) {
		pBotPart = m_pPartStateArray[nPartType].pDanglingBotPart;

		nBoneIndex = pBotPart->m_Verlet.GetBaseBoneIndex();

		if( !m_pPartPool->_GetOrigBoneCallbackEnabledBitmask( nBoneIndex ) ) {
			m_pBot->m_Anim.m_pAnimCombiner->DisableBoneCallback( nBoneIndex );
		}

		_Part_Disable( nPartType );
	}

	m_pLimbStateArray[nLimbType].nState = LIMB_STATE_INTACT;
	m_pLimbStateArray[nLimbType].fNormHealth = m_fBotInitialNormHealth * m_pPartPool->m_pLimbDefArray[nLimbType].m_fInitialHealthMultIntact;

	_UpdateAllComponentStatus();
}


void CBotPartMgr::InflictLimbDamage( const CDamageData *pDamageData, const CDamageResult *pBotDamageResult ) {
	FASSERT( IsCreated() );

	switch( pDamageData->m_nDamageLocale ) {
	case CDamageForm::DAMAGE_LOCALE_IMPACT:
		// Inflict impact damage on the limbs...
		_InflictLimbDamage_Impact( pDamageData );
		break;

	case CDamageForm::DAMAGE_LOCALE_BLAST:
		// Inflict blast damage on the limbs...
		_InflictLimbDamage_Blast( pDamageData, pBotDamageResult );
		break;

	case CDamageForm::DAMAGE_LOCALE_AMBIENT:
		// Limbs don't receive ambient damage...
		break;

	default:
		FASSERT_NOW;
	};
}


// Returns TRUE if the bot has been destroyed.
// Returns FALSE if the bot has not been destroyed.
BOOL CBotPartMgr::DestroyBot( const CDamageData *pDamageData ) {
	FASSERT( IsCreated() );

	if( m_pBot->m_nPossessionPlayerIndex != -1 ) {
		if( !CBot::m_bAllowPlayerDeath ) {
			// This makes it so that the player bot never dies!!!
			return FALSE;
		}
	}

	if( m_pBot->m_uBotDeathFlags & CBot::BOTDEATHFLAG_WALKING_DEAD ) {
		FASSERT( m_pBot->IsDying() );
		m_pBot->m_uBotDeathFlags &= ~CBot::BOTDEATHFLAG_WALKING_DEAD;
	}

	if( !m_pBot->IsDying() && MultiplayerMgr.IsSinglePlayer() ) {
		if( m_pPartPool->m_nTorsoBoneIndex >= 0 ) {
			// This bot supports blowing the torso off from its legs...

			if( (m_nFlags & (FLAG_RIGHT_LEG_NOT_INTACT | FLAG_LEFT_LEG_NOT_INTACT)) != (FLAG_RIGHT_LEG_NOT_INTACT | FLAG_LEFT_LEG_NOT_INTACT) ) {
				// Bot has at least one of his legs...

				if( pDamageData ) {
					// Damage data is valid...

					pDamageData->ComputeDamageResult( m_pBot->m_pBotInfo_Gen->pTorsoBlowOffArmorProfile, &m_DamageResult, m_pBot->GetArmorModifier() );

					if( m_DamageResult.m_fDeltaHitpoints > 0.0f ) {
						// Enough damage has been done to blow off the torso...

						if( pDamageData->m_nDamageLocale==CDamageForm::DAMAGE_LOCALE_BLAST || pDamageData->m_nDamageLocale==CDamageForm::DAMAGE_LOCALE_IMPACT ) {
							// Blast or impact damage...

							if( pDamageData->m_pTriData ) {
								// Blast directly hit a world mesh...

								if( pDamageData->m_pTriData->pDamageeWorldMesh == m_pBot->m_pWorldMesh ) {
									// Blast directly hit our bot (impacts will always get here)...

									if( pDamageData->m_pTriData->nDamageeBoneIndex == m_pPartPool->m_nTorsoBoneIndex ) {
										// Direct hit to the torso...

										// Amount of debris should be based on framerate
										#if !FANG_PLATFORM_DX
											f32 fFrameRateMod = FPerf_fAvgToTargetSPFRatio + 0.5f;
										#else
											f32 fFrameRateMod = FPerf_fAvgToTargetSPFRatio + 0.4f;
										#endif

										if ( fFrameRateMod <= 1.f )
										{
											_fGlobalDebrisUnitMod = _DEFAULT_DEBRIS_UNIT_MOD;
										}
										else
										{
											_fGlobalDebrisUnitMod = _DEFAULT_DEBRIS_UNIT_MOD * (fmath_Inv( fFrameRateMod ));
										}
										
										m_pBot->m_uBotDeathFlags |= CBot::BOTDEATHFLAG_WALKING_DEAD;	// must be set before _BlowOffBoneHierarchyIntoPieces, which calls Die(), which checks the flag
										_BlowOffBoneHierarchyIntoPieces( m_pPartPool->m_nTorsoBoneIndex );
										m_pBot->SetNormHealth( 0.5f );

										// Return the Mod to normal
										_fGlobalDebrisUnitMod = _DEFAULT_DEBRIS_UNIT_MOD;
										
										m_pBot->Die( FALSE );

										return FALSE;
									}
								}
							}
						}
					}
				}
			}
		}
	}

	if( m_pPartPool->IsCreated() ) {
		// Amount of debris should be based on framerate
		#if !FANG_PLATFORM_DX
			f32 fFrameRateMod = FPerf_fAvgToTargetSPFRatio + 0.6f;
		#else
			f32 fFrameRateMod = FPerf_fAvgToTargetSPFRatio + 0.4f;
		#endif

		if ( fFrameRateMod <= 1.f )
		{
			_fGlobalDebrisUnitMod = _DEFAULT_DEBRIS_UNIT_MOD;
		}
		else
		{
			_fGlobalDebrisUnitMod = _DEFAULT_DEBRIS_UNIT_MOD * (fmath_Inv( fFrameRateMod ));
		}

		_BlowOffBoneHierarchyIntoPieces( m_pPartPool->m_nRootBoneIndex, TRUE );
		
		// Return the Mod to normal
		_fGlobalDebrisUnitMod = _DEFAULT_DEBRIS_UNIT_MOD;
		
	}

	// if the bot's torso was destroyed above, the bot may think it's walking dead, but it's not.
	FMATH_CLEARBITMASK( m_pBot->m_uBotDeathFlags, CBot::BOTDEATHFLAG_WALKING_DEAD );

	m_pBot->Die( FALSE );
	m_pBot->m_uBotDeathFlags |= CBot::BOTDEATHFLAG_BOTPARTMGR_DEATHANIM;

	return TRUE;
}


void CBotPartMgr::_InflictLimbDamage_Impact( const CDamageData *pDamageData ) {
	FASSERT( pDamageData->m_nDamageLocale == CDamageForm::DAMAGE_LOCALE_IMPACT );

	u32 nLimbType, nPartType;
	s8 nBoneIndex;
	BOOL bHitLimb;

	nBoneIndex = pDamageData->m_pTriData->nDamageeBoneIndex;

	bHitLimb = m_pPartPool->GetLimbAndPartTypesFromBoneIndex( nBoneIndex, &nLimbType, &nPartType );

	if( bHitLimb ) {
		// Hit a limb...

		if( m_pLimbStateArray[nLimbType].nState != LIMB_STATE_REMOVED ) {
			if( !(m_pLimbStateArray[nLimbType].nFlags & LIMB_STATE_FLAG_INVINCIBLE) ) {
				_ComputeDamageResult( nLimbType, pDamageData );
				_InflictPartDamage( nLimbType, pDamageData );
				_ImpulseDanglingPart( nPartType );
			}
		}
	}

	if( m_pPartPool->_GetProxyBoneBitmask( nBoneIndex ) ) {
		// This is a proxy bone. Find out which limbs it belongs to...

		const CFMtx43A *pProxyBoneMtx;
		CFMtx43A ProxyBoneUnitMtx;
		CFVec3A BoxMinCorner_WS, BoxMaxCorner_WS;
		BOOL bComputedUnitMtx;
		const CBotPartPool::CLimbDef *pLimbDef;

		bComputedUnitMtx = FALSE;

		for( nLimbType=0; nLimbType<m_nLimbTypeCount; ++nLimbType ) {
			if( m_pLimbStateArray[nLimbType].nState == LIMB_STATE_REMOVED ) {
				continue;
			}

			if( m_pLimbStateArray[nLimbType].nFlags & LIMB_STATE_FLAG_INVINCIBLE ) {
				continue;
			}

			pLimbDef = &m_pPartPool->m_pLimbDefArray[nLimbType];

			if( pLimbDef->m_nProxyBoneIndex == nBoneIndex ) {
				// Found a proxy bone...

				pProxyBoneMtx = m_pBot->m_pWorldMesh->GetBoneMtxPalette()[ nBoneIndex ];

				if( !bComputedUnitMtx ) {
					// Compute proxy bone's unit matrix...

					ProxyBoneUnitMtx.Mul33( *pProxyBoneMtx, pProxyBoneMtx->m_vX.InvMag() );
					ProxyBoneUnitMtx.m_vPos = pProxyBoneMtx->m_vPos;

					bComputedUnitMtx = TRUE;
				}

				pProxyBoneMtx->MulPoint( BoxMinCorner_WS, pLimbDef->m_ProxyBoxMinCorner_BS );
				pProxyBoneMtx->MulPoint( BoxMaxCorner_WS, pLimbDef->m_ProxyBoxMaxCorner_BS );

				if( _TestPointInBox( &pDamageData->m_ImpactPoint_WS, &ProxyBoneUnitMtx, &BoxMinCorner_WS, &BoxMaxCorner_WS ) ) {
					// Impact is inside the box...
					_ComputeDamageResult( nLimbType, pDamageData );
					_InflictPartDamage( nLimbType, pDamageData );
				}
			}
		}
	}
}


// Uses m_DamageResult to apply an impulse to the dangling part.
void CBotPartMgr::_ImpulseDanglingPart( u32 nPartType ) {
	if( m_pLimbStateArray[ m_pPartPool->m_pPartTypePoolArray[nPartType].nLimbType ].nState == LIMB_STATE_DANGLING ) {
		if( m_DamageResult.m_fImpulseMag > 0.0f ) {
			m_pPartStateArray[nPartType].pDanglingBotPart->m_Verlet.ApplyImpulse( &m_DamageResult.m_pDamageData->m_ImpactPoint_WS, &m_DamageResult.m_Impulse_WS );
		}
	}
}


// Uses m_DamageResult to inflict hitpoint damage to a dangling or intact limb.
void CBotPartMgr::_InflictPartDamage( u32 nLimbType, const CDamageData *pDamageData ) {
	FASSERT( IsCreated() );

	LimbState_t *pLimbState;

	pLimbState = &m_pLimbStateArray[nLimbType];
	FASSERT( pLimbState->nState==LIMB_STATE_INTACT || pLimbState->nState==LIMB_STATE_DANGLING );

	if( m_DamageResult.m_fDeltaHitpoints != 0.0f ) {
		// There's at least some hitpoint damage...

		pLimbState->fNormHealth -= m_DamageResult.m_fDeltaHitpoints;

		if( pLimbState->fNormHealth <= 0.0f ) {
			pLimbState->fNormHealth = 0.0f;

			if( pLimbState->nState == LIMB_STATE_INTACT ) {
				SetState_Dangle( nLimbType );
				
				if( m_pLimbStateArray[nLimbType].nState == LIMB_STATE_DANGLING ) {
					// Spawn effects...
					_SpawnLimbEffects( nLimbType );
				}
			} else {
				_BlowOffLimbIntoPieces( nLimbType );
			}
		}
	}
}


// Computes the limb's damage result and stores it in m_DamageResult.
void CBotPartMgr::_ComputeDamageResult( u32 nLimbType, const CDamageData *pDamageData ) {
	FASSERT( IsCreated() );
	FASSERT( m_pLimbStateArray[nLimbType].nState==LIMB_STATE_INTACT || m_pLimbStateArray[nLimbType].nState==LIMB_STATE_DANGLING );

	f32 fDamageNormIntensity = 1.0f;

#if 1
	if( pDamageData->m_Damager.pWeapon && (pDamageData->m_Damager.pWeapon->TypeBits() & ENTITY_BIT_WEAPONRIPPER) ) {
		// Ripper saws do far more limb damage...

		fDamageNormIntensity = 4.0f;
	}
#endif

	if( m_pLimbStateArray[nLimbType].nState == LIMB_STATE_DANGLING ) {
		// This is a dangling limb...

		pDamageData->ComputeDamageResult( m_pPartPool->m_pLimbDefArray[nLimbType].m_pArmorProfileDangle, &m_DamageResult, m_pBot->GetArmorModifier(), FALSE, fDamageNormIntensity );
	} else {
		// This is an intact limb...

		pDamageData->ComputeDamageResult( m_pPartPool->m_pLimbDefArray[nLimbType].m_pArmorProfileIntact, &m_DamageResult, m_pBot->GetArmorModifier(), FALSE, fDamageNormIntensity );
	}
}


void CBotPartMgr::_ApplyPopOffTorqueToLimb( u32 nLimbType ) {
	u32 nPartType, nStartPartType, nEndPartType;

	nStartPartType = m_pPartPool->m_pLimbDefArray[nLimbType].m_nStartPartType;
	nEndPartType = m_pPartPool->m_pLimbDefArray[nLimbType].m_nEndPartType;

	// See if a part in this limb is affected by the blast...
	for( nPartType=nStartPartType; nPartType<nEndPartType; ++nPartType ) {
		_ApplyPopOffTorqueToPart( nLimbType, nPartType );
	}
}


void CBotPartMgr::_ApplyPopOffTorqueToPart( u32 nLimbType, u32 nPartType ) {
	FASSERT( m_pLimbStateArray[nLimbType].nState == LIMB_STATE_DANGLING );

	CFVec3A VerletCOM_WS, BotBoundSphereCenter_WS, BotCenterToPartCenterUnitVec_WS, ImpulseVec_WS;
	CFVerlet *pVerlet;
	f32 fMagSq;

	pVerlet = &m_pPartStateArray[nPartType].pDanglingBotPart->m_Verlet;

	// Compute the Verlet object's center of mass in world space...
	pVerlet->GetUnitMtx()->MulPoint( VerletCOM_WS, *pVerlet->GetCenterOfMass_MS() );

	// Compute the unit vector from the bot's bounding sphere center to the Verlet
	// object's center of mass...
	BotBoundSphereCenter_WS.Set( m_pBot->m_pWorldMesh->GetBoundingSphere().m_Pos );
	BotCenterToPartCenterUnitVec_WS.Sub( VerletCOM_WS, BotBoundSphereCenter_WS );

	fMagSq = BotCenterToPartCenterUnitVec_WS.MagSq();
	if( fMagSq > 0.0001f ) {
		BotCenterToPartCenterUnitVec_WS.Mul( fmath_InvSqrt( fMagSq ) );
	} else {
		BotCenterToPartCenterUnitVec_WS = CFVec3A::m_UnitAxisY;
	}

	ImpulseVec_WS.Mul( BotCenterToPartCenterUnitVec_WS, 5000.0f );

	pVerlet->ApplyImpulseToCOM( &ImpulseVec_WS );
}


void CBotPartMgr::_InflictLimbDamage_Blast( const CDamageData *pDamageData, const CDamageResult *pBotDamageResult ) {
	FASSERT( pDamageData->m_nDamageLocale == CDamageForm::DAMAGE_LOCALE_BLAST );

	u32 nLimbType;
	CDamageData PartDamageData;

	fang_MemCopy( &PartDamageData, pDamageData, sizeof(CDamageData) );

	// First inflict damage on dangling limbs...
	for( nLimbType=0; nLimbType<m_nLimbTypeCount; ++nLimbType ) {
		if( m_pLimbStateArray[nLimbType].nState == LIMB_STATE_DANGLING ) {
			if( !(m_pLimbStateArray[nLimbType].nFlags & LIMB_STATE_FLAG_INVINCIBLE) ) {
				_InflictLimbDamage_Blast_Dangling( nLimbType, &PartDamageData, pBotDamageResult );
			}
		}
	}

	if( pBotDamageResult->m_fDeltaHitpoints <= 0.0f ) {
		// Bot received no hitpoint damage, so neither will any limb...
		return;
	}

	s32 nClosestLimbType;
//	s32 nImpactedLimbType;
	f32 fDistToNearestBoneCenter2, fNearestBoneRadius, fClosestLimb_DistToNearestBoneCenter2, fClosestLimb_NearestBoneRadius;
	CFVec3A NearestBoneCenter_WS, ClosestLimb_NearestBoneCenter_WS;

	nClosestLimbType = -1;

#if 0
	if( pDamageData->m_pTriData ) {
		if( pDamageData->m_pTriData->pDamageeWorldMesh == m_pBot->m_pWorldMesh ) {
			if( pDamageData->m_pTriData->nDamageeBoneIndex >= 0 ) {
				nImpactedLimbType = GetLimbTypeFromBoneIndex( pDamageData->m_pTriData->nDamageeBoneIndex );

				if( nImpactedLimbType >= 0 ) {
					if( m_pLimbStateArray[nImpactedLimbType].nState == LIMB_STATE_INTACT ) {
						if( !(m_pLimbStateArray[nImpactedLimbType].nFlags & LIMB_STATE_FLAG_INVINCIBLE) ) {
							if( m_pPartPool->m_pLimbDefArray[nImpactedLimbType].m_nDangleRule != CBotPartPool::DANGLE_RULE_BACK_BREAKER ) {
								if( _FindBoneNearestToBlastCenter( nImpactedLimbType, pDamageData, &fDistToNearestBoneCenter2, &NearestBoneCenter_WS, &fNearestBoneRadius ) ) {
									// This limb was hit by the blast...

									_ComputeBlastDamageResult_Intact( nImpactedLimbType, &PartDamageData, 0.05f, &NearestBoneCenter_WS, fNearestBoneRadius );

									if( m_DamageResult.m_fDeltaHitpoints > 0.0f ) {
										nClosestLimbType = nImpactedLimbType;
										fClosestLimb_DistToNearestBoneCenter2 = 0.05f;
										ClosestLimb_NearestBoneCenter_WS = NearestBoneCenter_WS;
										fClosestLimb_NearestBoneRadius = fNearestBoneRadius;
									}
								}
							}
						}
					}
				}
			}
		}
	}
#endif

	// Now find the intact limb that's closest to the blast damage...
	for( nLimbType=0; nLimbType<m_nLimbTypeCount; ++nLimbType ) {
		if( m_pLimbStateArray[nLimbType].nState == LIMB_STATE_INTACT ) {
#if 0
			if( nLimbType == nClosestLimbType ) {
				continue;
			}
#endif

			if( m_pLimbStateArray[nLimbType].nFlags & LIMB_STATE_FLAG_INVINCIBLE ) {
				continue;
			}

			if( !_FindBoneNearestToBlastCenter( nLimbType, pDamageData, &fDistToNearestBoneCenter2, &NearestBoneCenter_WS, &fNearestBoneRadius ) ) {
				// This limb wasn't hit by the blast...
				continue;
			}

			// This bone was hit by the blast!

			if( m_pPartPool->m_pLimbDefArray[nLimbType].m_nDangleRule == CBotPartPool::DANGLE_RULE_BACK_BREAKER ) {
				// Always deal damage to back-breakers...

				_ComputeBlastDamageResult_Intact( nLimbType, &PartDamageData, fDistToNearestBoneCenter2, &NearestBoneCenter_WS, fNearestBoneRadius );
				_InflictPartDamage( nLimbType, &PartDamageData );

				continue;
			}

			// See if it's the closest so far...
			if( (nClosestLimbType >= 0) && (fDistToNearestBoneCenter2 >= fClosestLimb_DistToNearestBoneCenter2) ) {
				// Not the closest so far...
				continue;
			}

			// This is the closest limb so far...

			_ComputeBlastDamageResult_Intact( nLimbType, &PartDamageData, fDistToNearestBoneCenter2, &NearestBoneCenter_WS, fNearestBoneRadius );
			if( m_DamageResult.m_fDeltaHitpoints == 0.0f ) {
				// No damage done to this limb, so skip it...
				continue;
			}

			nClosestLimbType = nLimbType;
			fClosestLimb_DistToNearestBoneCenter2 = fDistToNearestBoneCenter2;
			ClosestLimb_NearestBoneCenter_WS = NearestBoneCenter_WS;
			fClosestLimb_NearestBoneRadius = fNearestBoneRadius;
		}
	}

	if( nClosestLimbType >= 0 ) {
		// We found the closest limb hit by the blast...

		_ComputeBlastDamageResult_Intact( nClosestLimbType, &PartDamageData, fClosestLimb_DistToNearestBoneCenter2, &ClosestLimb_NearestBoneCenter_WS, fClosestLimb_NearestBoneRadius );
		_InflictPartDamage( nClosestLimbType, &PartDamageData );
	}
}


void CBotPartMgr::_ComputeBlastDamageResult_Intact( u32 nLimbType, CDamageData *pDamageData, f32 fDistToNearestBoneCenter2, const CFVec3A *pNearestBoneCenter_WS, f32 fNearestBoneRadius ) {
	f32 fDistToNearestBoneCenter, fDistToClosestPointOnBoneSphere, fIntensity;
	u32 i;

	pDamageData->m_fImpulseMag = 0.0f;
	pDamageData->m_fRumbleUnitIntensity = 0.0f;

	if( fDistToNearestBoneCenter2 > 0.0001f ) {
		fDistToNearestBoneCenter = fmath_Sqrt( fDistToNearestBoneCenter2 );
		fDistToClosestPointOnBoneSphere = fDistToNearestBoneCenter - fNearestBoneRadius;

		pDamageData->m_AttackUnitDir_WS.Sub( *pNearestBoneCenter_WS, *pDamageData->m_pEpicenter_WS ).Unitize();

		if( fDistToClosestPointOnBoneSphere <= 0.0f ) {
			// Blast sphere center is inside bone sphere...

			fDistToClosestPointOnBoneSphere = 0.0f;

			pDamageData->m_ImpactPoint_WS = *pNearestBoneCenter_WS;
		} else {
			// Blast sphere center is outside bone sphere...

			pDamageData->m_ImpactPoint_WS.Mul( pDamageData->m_AttackUnitDir_WS, fDistToClosestPointOnBoneSphere );
			pDamageData->m_ImpactPoint_WS.Add( *pDamageData->m_pEpicenter_WS );
		}
	} else {
		fDistToClosestPointOnBoneSphere = 0.1f;

		pDamageData->m_AttackUnitDir_WS = CFVec3A::m_UnitAxisY;
		pDamageData->m_ImpactPoint_WS.Set( pDamageData->m_pEpicenter_WS->x, pDamageData->m_pEpicenter_WS->y + 0.1f, pDamageData->m_pEpicenter_WS->z );
	}

	// Compute damage data fields...
	fIntensity = CDamage::ComputeRadialIntensity_Hitpoint( fDistToClosestPointOnBoneSphere, pDamageData->m_pDamageProfile, pDamageData->m_fDamageFormOrigNormIntensity );
	for( i=0; i<DAMAGE_HITPOINT_TYPE_COUNT; ++i ) {
		pDamageData->m_afDeltaHitpoints[i] = fIntensity * pDamageData->m_pDamageProfile->m_afUnitHitpoints[i];
	}

	_ComputeDamageResult( nLimbType, pDamageData );
}


// Finds the nearest bone on the limb whose bounding sphere intersects the
// sphere defined by pDamageData.
//
// If found, returns TRUE and fills pfDistToNearestBoneCenter2, pNearestBoneCenter_WS, and pfNearestBoneRadius.
// If not found, returns FALSE;
BOOL CBotPartMgr::_FindBoneNearestToBlastCenter( u32 nLimbType, const CDamageData *pDamageData,
												 f32 *pfDistToNearestBoneCenter2, CFVec3A *pNearestBoneCenter_WS, f32 *pfNearestBoneRadius ) {
	FASSERT( pDamageData->m_nDamageLocale == CDamageForm::DAMAGE_LOCALE_BLAST );
	FASSERT( m_pLimbStateArray[nLimbType].nState == LIMB_STATE_INTACT );

	const CFVerlet *pVerlet;
	const FMeshBone_t *pBone, *pBoneArray;
	const CFMtx43A *pBoneMtx;
	CFMtx43A **ppBoneMtxPalette;
	CFVec3A BoneBoundSpherePos_WS, NearestBoneCenter_WS;
	const CFVec3A *pEpicenter_WS;
	u32 nPartType, nStartPartType, nEndPartType, nVerletBoneArrayIndex, nBoneIndex, nBoneCount;
	f32 fBlastRadius, fBoneBoundSphereRadius, fRadiusSum, fNearestBoneRadius;
	f32 fDistFromBoneSphereCenterToBlastSphereCenter2_WS, fSmallestDistFromBoneSphereCenterToBlastSphereCenter2_WS;
	BOOL bFoundIntersectingBone = FALSE;

	pBoneArray = m_pBot->m_pWorldMesh->m_pMesh->pBoneArray;
	ppBoneMtxPalette = m_pBot->m_pWorldMesh->GetBoneMtxPalette();

	pEpicenter_WS = pDamageData->m_pEpicenter_WS;
	fBlastRadius = pDamageData->m_pDamageProfile->m_HitpointRange.m_fOuterRadius;

	fSmallestDistFromBoneSphereCenterToBlastSphereCenter2_WS = 0.0f;

	nStartPartType = m_pPartPool->m_pLimbDefArray[nLimbType].m_nStartPartType;
	nEndPartType = m_pPartPool->m_pLimbDefArray[nLimbType].m_nEndPartType;

	// See if a part in this limb is affected by the blast...
	for( nPartType=nStartPartType; nPartType<nEndPartType; ++nPartType ) {
		// Any old Verlet object for this part type will do...
		pVerlet = &m_pPartPool->m_pPartTypePoolArray[nPartType].pPartArray->m_Verlet;

		// Get the contributing bone count for this part...
		nBoneCount = pVerlet->GetContributingBoneCount();

		// See if a bone in this part is affected by the blast...
		for( nVerletBoneArrayIndex=0; nVerletBoneArrayIndex<nBoneCount; ++nVerletBoneArrayIndex ) {
			nBoneIndex = pVerlet->GetBoneIndex(nVerletBoneArrayIndex);
			pBone = &pBoneArray[nBoneIndex];
			pBoneMtx = ppBoneMtxPalette[nBoneIndex];

			// Compute the bone's bounding sphere in world space...
			BoneBoundSpherePos_WS.Set( pBone->SegmentedBoundSphere_BS.m_Pos );
			pBoneMtx->MulPoint( BoneBoundSpherePos_WS );
			fBoneBoundSphereRadius = pBone->SegmentedBoundSphere_BS.m_fRadius * pBoneMtx->m_vX.Mag();

			// Get the distance squared between the bone sphere center and the blast sphere center...
			fDistFromBoneSphereCenterToBlastSphereCenter2_WS = BoneBoundSpherePos_WS.DistSq( *pEpicenter_WS );

			// See if the bone sphere intersects the blast sphere...
			fRadiusSum = fBlastRadius + fBoneBoundSphereRadius;
			if( fDistFromBoneSphereCenterToBlastSphereCenter2_WS >= fRadiusSum*fRadiusSum ) {
				// They don't intersect...
				continue;
			}

			// This bone is affected by the blast. See if it's the closest...
			if( bFoundIntersectingBone && (fDistFromBoneSphereCenterToBlastSphereCenter2_WS >= fSmallestDistFromBoneSphereCenterToBlastSphereCenter2_WS) ) {
				// Not the closest...
				continue;
			}

			// This is the closest so far...

			bFoundIntersectingBone = TRUE;
			fSmallestDistFromBoneSphereCenterToBlastSphereCenter2_WS = fDistFromBoneSphereCenterToBlastSphereCenter2_WS;
			NearestBoneCenter_WS = BoneBoundSpherePos_WS;
			fNearestBoneRadius = fBoneBoundSphereRadius;
		}
	}

	if( bFoundIntersectingBone ) {
		*pfDistToNearestBoneCenter2 = fSmallestDistFromBoneSphereCenterToBlastSphereCenter2_WS;
		*pNearestBoneCenter_WS = NearestBoneCenter_WS;
		*pfNearestBoneRadius = fNearestBoneRadius;
	}

	return bFoundIntersectingBone;
}


// Applies blast damage to a dangling limb.
void CBotPartMgr::_InflictLimbDamage_Blast_Dangling( u32 nLimbType, CDamageData *pDamageData, const CDamageResult *pBotDamageResult ) {
	FASSERT( pDamageData->m_nDamageLocale == CDamageForm::DAMAGE_LOCALE_BLAST );
	FASSERT( m_pLimbStateArray[nLimbType].nState == LIMB_STATE_DANGLING );

	if( (pBotDamageResult->m_fDeltaHitpoints <= 0.0f) && (pBotDamageResult->m_fImpulseMag <= 0.0f) ) {
		// Nothing to do...
		return;
	}

	const CFVerlet *pVerlet;
	const CFVerletTack *pTack;
	CFMtx43A **ppBoneMtxPalette;
	const CFVec3A *pBlastEpicenter_WS;
	CFVec3A VerletCOM_WS, BlastTackPos_WS, ClosestBlastPoint_WS;
	u32 i, nPartType, nStartPartType, nEndPartType, nBlastTackIndex, nBlastTackCount;
	f32 fDistFromEpicenterToBlastTackBone2, fIntensity;
	f32 fLargestDamageRadius2, fClosestDistToEpicenter2, fOOClosestDistToEpicenter, fClosestDistToEpicenter;
	const u8 *pnBlastTackIndexArray;
	CDamageResult DamageResultWithGreatestHitpoints;
	BOOL bFoundDamageResultWithGreatestHitpoints;

	ppBoneMtxPalette = m_pBot->m_pWorldMesh->GetBoneMtxPalette();
	pBlastEpicenter_WS = pDamageData->m_pEpicenter_WS;

	fLargestDamageRadius2 = pDamageData->m_fLargestOuterRadius * pDamageData->m_fLargestOuterRadius;
	DamageResultWithGreatestHitpoints.m_fDeltaHitpoints = 0.0f;
	bFoundDamageResultWithGreatestHitpoints = FALSE;

	nStartPartType = m_pPartPool->m_pLimbDefArray[nLimbType].m_nStartPartType;
	nEndPartType = m_pPartPool->m_pLimbDefArray[nLimbType].m_nEndPartType;

	// See if a part in this limb is affected by the blast...
	for( nPartType=nStartPartType; nPartType<nEndPartType; ++nPartType ) {
		if( m_pLimbStateArray[ m_pPartPool->m_pPartTypePoolArray[nPartType].nLimbType ].nState != LIMB_STATE_DANGLING ) {
			// This is possible if we got blown off...
			continue;
		}

		pVerlet = &m_pPartStateArray[nPartType].pDanglingBotPart->m_Verlet;

		// Compute the verlet object's center of mass in world space...
		pVerlet->GetUnitMtx()->MulPoint( VerletCOM_WS, *pVerlet->GetCenterOfMass_MS() );

		// Compute the distance-squared from the COM to the blast epicenter...
		fClosestDistToEpicenter2 = VerletCOM_WS.DistSq( *pBlastEpicenter_WS );
		ClosestBlastPoint_WS = VerletCOM_WS;

		// See if any blast points are closer to center of mass...
		nBlastTackCount = m_pPartPool->m_pPartTypePoolArray[nPartType].pPartArray->m_nBlastTackCount;
		pnBlastTackIndexArray = m_pPartPool->m_pPartTypePoolArray[nPartType].pPartArray->m_pnBlastTackIndexArray;

		for( nBlastTackIndex=0; nBlastTackIndex<nBlastTackCount; ++nBlastTackIndex ) {
			pTack = pVerlet->Tack_GetFromIndex( pnBlastTackIndexArray[nBlastTackIndex] );
			pTack->ComputePos_WS( &BlastTackPos_WS );

			fDistFromEpicenterToBlastTackBone2 = BlastTackPos_WS.DistSq( *pBlastEpicenter_WS );

			if( fDistFromEpicenterToBlastTackBone2 < fClosestDistToEpicenter2 ) {
				// Blast tack is closer to epicenter than any so far...

				fClosestDistToEpicenter2 = fDistFromEpicenterToBlastTackBone2;
				ClosestBlastPoint_WS = BlastTackPos_WS;
			}
		}

		// See if the closest blast point is within the blast radius...
		if( fClosestDistToEpicenter2 < fLargestDamageRadius2 ) {
			// It's within the blast radius!

			// Compute impact point and distance from epicenter to impact point...
			if( fClosestDistToEpicenter2 > 0.0001f ) {
				fOOClosestDistToEpicenter = fmath_InvSqrt( fClosestDistToEpicenter2 );
				fClosestDistToEpicenter = fmath_Inv( fOOClosestDistToEpicenter );

				pDamageData->m_ImpactPoint_WS = ClosestBlastPoint_WS;
				pDamageData->m_AttackUnitDir_WS.Sub( pDamageData->m_ImpactPoint_WS, *pBlastEpicenter_WS ).Mul( fOOClosestDistToEpicenter );
			} else {
				fClosestDistToEpicenter = 0.1f;
				pDamageData->m_ImpactPoint_WS.Set( pBlastEpicenter_WS->x, pBlastEpicenter_WS->y + 0.1f, pBlastEpicenter_WS->z );
				pDamageData->m_AttackUnitDir_WS = CFVec3A::m_UnitAxisY;
			}

			// Compute damage data fields...
			fIntensity = CDamage::ComputeRadialIntensity_Hitpoint( fClosestDistToEpicenter, pDamageData->m_pDamageProfile, pDamageData->m_fDamageFormOrigNormIntensity );
			for( i=0; i<DAMAGE_HITPOINT_TYPE_COUNT; ++i ) {
				pDamageData->m_afDeltaHitpoints[i] = fIntensity * pDamageData->m_pDamageProfile->m_afUnitHitpoints[i];
			}

			pDamageData->m_fImpulseMag = CDamage::ComputeRadialIntensity_Impulse( fClosestDistToEpicenter, pDamageData->m_pDamageProfile, pDamageData->m_fDamageFormOrigNormIntensity );

			pDamageData->m_fRumbleUnitIntensity = CDamage::ComputeRadialIntensity_Rumble( fClosestDistToEpicenter, pDamageData->m_pDamageProfile, pDamageData->m_fDamageFormOrigNormIntensity );
			FMATH_CLAMP_UNIT_FLOAT( pDamageData->m_fRumbleUnitIntensity );

			_ComputeDamageResult( nLimbType, pDamageData );

			if( m_DamageResult.m_fDeltaHitpoints > DamageResultWithGreatestHitpoints.m_fDeltaHitpoints ) {
				bFoundDamageResultWithGreatestHitpoints = TRUE;
				fang_MemCopy( &DamageResultWithGreatestHitpoints, &m_DamageResult, sizeof(CDamageResult) );
			}

			// Apply impulse to part...
			if( pBotDamageResult->m_fImpulseMag > 0.0f ) {
				_ImpulseDanglingPart( nPartType );
			}
		}
	}

	if( pBotDamageResult->m_fDeltaHitpoints > 0.0f ) {
		// Apply damage to limb...
		if( bFoundDamageResultWithGreatestHitpoints ) {
			fang_MemCopy( &m_DamageResult, &DamageResultWithGreatestHitpoints, sizeof(CDamageResult) );
			_InflictPartDamage( nLimbType, pDamageData );
		}
	}
}


BOOL CBotPartMgr::_TestPointInBox( const CFVec3A *pPos_WS, const CFMtx43A *pUnitMtx, const CFVec3A *pMinCorner_WS, const CFVec3A *pMaxCorner_WS ) {
	CFVec3A PosToPointOnPlane_WS;
	f32 fDot;

	PosToPointOnPlane_WS.Sub( *pMinCorner_WS, *pPos_WS );

	fDot = PosToPointOnPlane_WS.Dot( pUnitMtx->m_vX );
	if( fDot > 0.0f ) {
		return FALSE;
	}

	fDot = PosToPointOnPlane_WS.Dot( pUnitMtx->m_vY );
	if( fDot > 0.0f ) {
		return FALSE;
	}

	fDot = PosToPointOnPlane_WS.Dot( pUnitMtx->m_vZ );
	if( fDot > 0.0f ) {
		return FALSE;
	}

	PosToPointOnPlane_WS.Sub( *pMaxCorner_WS, *pPos_WS );

	fDot = PosToPointOnPlane_WS.Dot( pUnitMtx->m_vX );
	if( fDot < 0.0f ) {
		return FALSE;
	}

	fDot = PosToPointOnPlane_WS.Dot( pUnitMtx->m_vY );
	if( fDot < 0.0f ) {
		return FALSE;
	}

	fDot = PosToPointOnPlane_WS.Dot( pUnitMtx->m_vZ );
	if( fDot < 0.0f ) {
		return FALSE;
	}

	return TRUE;
}


BOOL CBotPartMgr::_AnimBoneCallbackFunctionHandler( u32 nBoneIndex, CFMtx43A &rNewMtx, const CFMtx43A &rParentMtx, const CFMtx43A &rBoneMtx ) {
	if( m_nSeveredBoneCount ) {
		if( _GetSeveredBoneBitMask( nBoneIndex ) ) {
			// This bone is severed...
			return TRUE;
		}
	}

	CBotPart *pBotPart;
	u32 i, nPartType;

	for( i=0; i<m_nActiveBoneMapCount; ++i ) {
		if( m_pBoneMapArray[i].nBoneIndex == nBoneIndex ) {
			break;
		}
	}

	if( i == m_nActiveBoneMapCount ) {
		// Not one of our bones...
		return FALSE;
	}

	// Found a bone we're driving...

	nPartType = m_pBoneMapArray[i].nPartType;

	pBotPart = m_pPartStateArray[nPartType].pDanglingBotPart;

	// See whether this part has been disabled...
	if( pBotPart->GetMode() == CBotPart::MODE_DISABLED ) {
		// This part is now disabled. Clear it...
		_Part_Disable( nPartType );

		if( !m_pPartPool->_GetOrigBoneCallbackEnabledBitmask( nBoneIndex ) ) {
			m_pBot->m_Anim.m_pAnimCombiner->DisableBoneCallback( nBoneIndex );
		}

		return FALSE;
	} else {
		rParentMtx.MulPoint( pBotPart->m_DanglingBoneAnimDrivenPos_WS, rBoneMtx.m_vPos );

		// We need to manually call the work function for this botpart verlet object.
		// Then take the results and stuff them into the new matrix.

		pBotPart->m_Verlet.ManualWork();

		rNewMtx = *pBotPart->m_Verlet.GetUnitMtx();
	}

	// If this part is in the process of being repaired, interpolate it back into position...
	if( pBotPart->GetMode() == CBotPart::MODE_REPAIR ) {
		// Update the repair unit timer...
		if( pBotPart->m_fUnitRepairTimer < 1.0f ) {
			pBotPart->m_fUnitRepairTimer += FLoop_fPreviousLoopSecs * _LIMB_OO_REPAIR_TIME;
			FMATH_CLAMP_MAX1( pBotPart->m_fUnitRepairTimer );
		} else {
			pBotPart->SetMode_Disabled();
		}

		// Figure out where the animation wants it to be...

		CFMtx43A::m_Temp.Mul( rParentMtx, rBoneMtx );
		CFQuatA qResult, qFrom, qTo;
		CFVec3A vXlat;

		qFrom.BuildQuat( rNewMtx );			// (returned from the verlet object)
		qTo.BuildQuat( CFMtx43A::m_Temp );	// (where the animation system thinks this bone belongs)
		qResult.ReceiveSlerpOf( pBotPart->m_fUnitRepairTimer, qFrom, qTo );

		qResult.BuildMtx33( rNewMtx );

		rNewMtx.m_vPos.x = FMATH_FPOT( pBotPart->m_fUnitRepairTimer, rNewMtx.m_vPos.x, CFMtx43A::m_Temp.m_vPos.x );
		rNewMtx.m_vPos.y = FMATH_FPOT( pBotPart->m_fUnitRepairTimer, rNewMtx.m_vPos.y, CFMtx43A::m_Temp.m_vPos.y );
		rNewMtx.m_vPos.z = FMATH_FPOT( pBotPart->m_fUnitRepairTimer, rNewMtx.m_vPos.z, CFMtx43A::m_Temp.m_vPos.z );
	}

	return TRUE;
}


void CBotPartMgr::_SpawnLimbEffects( u32 nLimbType ) {
	CBotPartPool::CLimbDef *pLimbDef;
	const FMeshBone_t *pBoneArray;
	const CFMtx43A *pInvParentMtx_MS, *pEmitterMtx_MS;
	CFMtx43A EmitterMtxInParentBoneSpace;
	CBotPartPool::ParticleInfo_t *pParticleInfo;
	u8 nParentBoneIndex;
	s8 nParticleBoneIndex;
	u32 i, nParticleTypeCount;
	f32 fOOMtxScale;
	cchar *pszAttachBoneName;

	pLimbDef = &m_pPartPool->m_pLimbDefArray[nLimbType];
	pBoneArray = m_pBot->m_pWorldMesh->m_pMesh->pBoneArray;

	nParticleBoneIndex = pLimbDef->m_nParticleBoneIndex;
	if( nParticleBoneIndex < 0 ) {
		return;
	}

	nParentBoneIndex = pBoneArray[ nParticleBoneIndex ].Skeleton.nParentBoneIndex;
	if( nParentBoneIndex == 255 ) {
		return;
	}

	pszAttachBoneName = pBoneArray[ nParentBoneIndex ].szName;

	pEmitterMtx_MS = &pBoneArray[ nParticleBoneIndex ].AtRestBoneToModelMtx;
	pInvParentMtx_MS = &pBoneArray[ nParentBoneIndex ].AtRestModelToBoneMtx;

	EmitterMtxInParentBoneSpace.Mul( *pInvParentMtx_MS, *pEmitterMtx_MS );

	fOOMtxScale = EmitterMtxInParentBoneSpace.m_vX.InvMag();
	EmitterMtxInParentBoneSpace.Mul33( fOOMtxScale );

	nParticleTypeCount = pLimbDef->m_nParticleDefCount;

	for( i=0; i<nParticleTypeCount; ++i ) {
		pParticleInfo = &pLimbDef->m_pParticleInfoArray[i];

		CEParticle *pEParticle = eparticlepool_GetEmitter();

		if( pEParticle ) {
			pEParticle->Attach_UnitMtxToParent_PS( m_pBot, pszAttachBoneName, &EmitterMtxInParentBoneSpace, FALSE );
			pEParticle->AddToWorld();

			pEParticle->StartEmission( pParticleInfo->hParticleDef, pParticleInfo->fUnitIntensity, pParticleInfo->fDuration, TRUE );
		}
	}
}


void CBotPartMgr::_BoneGeoDebrisCallback( CFDebris *pDebris, CFDebrisDef::CallbackReason_e nReason, const FCollImpact_t *pCollImpact ) {
	const CFDebrisDef *pDebrisDef = pDebris->GetDebrisDef();
	CBotPartMgr *pPartMgr = (CBotPartMgr *)pDebrisDef->m_pUser;

	if( nReason == CFDebrisDef::CALLBACK_REASON_BUILD_TRACKER_SKIP_LIST ) {
		pPartMgr->m_pBot->AppendTrackerSkipList();
		return;
	}

	if( nReason == CFDebrisDef::CALLBACK_REASON_COLLISION ) {
		pPartMgr->_SpawnBoneChunks( pDebrisDef->m_nBoneIndex, &pCollImpact->UnitFaceNormal );
	} else if( (nReason == CFDebrisDef::CALLBACK_REASON_DEAD) || (nReason == CFDebrisDef::CALLBACK_REASON_START_FADE) ) {
		pPartMgr->_SpawnBoneChunks( pDebrisDef->m_nBoneIndex, &CFVec3A::m_UnitAxisY );
	}

	pPartMgr->_KillBoneGeo( pDebrisDef->m_nBoneIndex );

	pDebris->Kill();

	// One less bot bone geo chunk flying through the air...
	FASSERT( pPartMgr->m_nBoneGeoDebrisCount > 0 );
	--pPartMgr->m_nBoneGeoDebrisCount;
}


void CBotPartMgr::_ChunkAndGutsDebrisCallback( CFDebris *pDebris, CFDebrisDef::CallbackReason_e nReason, const FCollImpact_t *pCollImpact ) {
	const CFDebrisDef *pDebrisDef = pDebris->GetDebrisDef();
	CBotPartMgr *pPartMgr = (CBotPartMgr *)pDebrisDef->m_pUser;

	if( nReason == CFDebrisDef::CALLBACK_REASON_BUILD_TRACKER_SKIP_LIST ) {
		pPartMgr->m_pBot->AppendTrackerSkipList();
	}
}


void CBotPartMgr::_KillBoneGeo( u32 nBoneIndex ) {
	FASSERT( nBoneIndex < m_pBot->m_pWorldMesh->m_pMesh->nBoneCount );

	// Relocate part someplace where it won't be seen...
	CFMtx43A *pBoneMtx = m_pBot->m_pWorldMesh->GetBoneMtxPalette()[ nBoneIndex ];

	pBoneMtx->Identity33();
	pBoneMtx->m_vPos.Set( 1000000.0f, 1000000.0f, 1000000.0f );
}


// would be nice if this was in debris
static f32 _CalcSpeedAdjForDebris( CFVec3A *pvNewUnitDir, const CFVec3A *pvBaseUnitDir, f32 fMinSpeed, f32 fMaxSpeed, const CFVec3A &vVel ) {
	// Account for bot's velocity...
	f32 fSpeedAdj;
	pvNewUnitDir->Mul( *pvBaseUnitDir, fMaxSpeed );
	pvNewUnitDir->Add( vVel );
	fSpeedAdj = pvNewUnitDir->Mag();
	if( fSpeedAdj > 0.01f ) {
		pvNewUnitDir->Mul( fmath_Inv( fSpeedAdj ) );
	} else {
		*pvNewUnitDir = *pvBaseUnitDir;
	}
	fSpeedAdj -= fMaxSpeed;
	FMATH_CLAMPMIN( fSpeedAdj, -fMinSpeed );

	return fSpeedAdj;
}


void CBotPartMgr::_SpawnBoneChunks( u32 nBoneIndex, const CFVec3A *pUnitDir_WS ) {
	FASSERT( nBoneIndex < m_pBot->m_pWorldMesh->m_pMesh->nBoneCount );
	FASSERT( pUnitDir_WS );

	if( m_pBot->m_pBotInfo_Gen->pDebrisGroup_Chunks == NULL ) {
		// Can't spawn any chunks...
		return;
	}

	u32 i;

	// Find bone in bone info array...
	for( i=0; i<m_pPartPool->m_nBoneInfoCount; ++i ) {
		if( m_pPartPool->m_pBoneInfo[i].m_nBoneIndex == nBoneIndex ) {
			break;
		}
	}

	if( i == m_pPartPool->m_nBoneInfoCount ) {
		// This bone is not in the info file. Nothing left to do...
		return;
	}

	// This bone is in the info file...

	const CBotPartPool::CBoneDef *pBoneDef = &m_pPartPool->m_pBoneInfo[i];

	if( pBoneDef->m_nChunkCount==0 && pBoneDef->m_nFlamingChunkCount==0 ) {
		// No chunks to spawn...
		return;
	}

	CFMtx43A *pBoneMtx = m_pBot->m_pWorldMesh->GetBoneMtxPalette()[ nBoneIndex ];

	if( m_fSecsUntilNextDebrisBlowSound == 0.0f ) {
		m_fSecsUntilNextDebrisBlowSound = _MIN_SECS_BETWEEN_DEBRIS_BLOW_SOUNDS;

		// Make sound effect...
		fsndfx_Play3D(
			m_pBot->m_pBotInfo_Gen->hFlamingDebrisBurstSound,
			&pBoneMtx->m_vPos,
			200.0f,
			1.0f,
			1.0
		);

		if( m_pBot->m_pBotInfo_Gen->pDebrisBurstImpulseDamageProfile ) {
			// Generate impulse wave...
			CDamageForm *pDamageForm;

			pDamageForm = CDamage::GetEmptyDamageFormFromPool();
			if( pDamageForm ) {
				pDamageForm->m_nDamageLocale = CDamageForm::DAMAGE_LOCALE_BLAST;
				pDamageForm->m_nDamageDelivery = CDamageForm::DAMAGE_DELIVERY_ALL_ENTITIES_WITHIN_PROFILE_RADIUS;
				pDamageForm->m_pDamageProfile = m_pBot->m_pBotInfo_Gen->pDebrisBurstImpulseDamageProfile;
				pDamageForm->m_Epicenter_WS = pBoneMtx->m_vPos;
				pDamageForm->m_Damager.pBot = m_pBot;
				pDamageForm->m_Damager.pEntity = m_pBot;
				pDamageForm->m_Damager.pWeapon = NULL;
				pDamageForm->m_Damager.nDamagerPlayerIndex = m_pBot ? m_pBot->m_nPossessionPlayerIndex : -1;

				CDamage::SubmitDamageForm( pDamageForm );
			}
		}
	}

	// Let's spawn some chunks...

	const CFSphere *pBoneSphere_BS;
	f32 fBoneRadius;
	CFVec3A BoneSphereCenter_WS;
	CFDebrisSpawner DebrisSpawner;

	DebrisSpawner.InitToDefaults();
	DebrisSpawner.m_pDebrisGroup = m_pBot->m_pBotInfo_Gen->pDebrisGroup_Chunks;

	pBoneSphere_BS = &m_pBot->m_pWorldMesh->m_pMesh->pBoneArray[ nBoneIndex ].SegmentedBoundSphere_BS;
	fBoneRadius = pBoneSphere_BS->m_fRadius * pBoneMtx->m_vX.Mag();

	BoneSphereCenter_WS.Set( pBoneSphere_BS->m_Pos );
	pBoneMtx->MulPoint( DebrisSpawner.m_Mtx.m_vPos, BoneSphereCenter_WS );

	//// Account for bot's velocity...
	//f32 fSpeedAdj;
	//CFVec3A vDebrisVel;
	//vDebrisVel.Mul( *pUnitDir_WS, m_pBot->m_pBotInfo_Gen->fMaxChunksDebrisSpeed );
	//vDebrisVel.Add( m_pBot->m_Velocity_WS );
	//fSpeedAdj = vDebrisVel.Mag();
	//if( fSpeedAdj > 0.01f ) {
	//	vDebrisVel.Mul( fmath_Inv( fSpeedAdj ) );
	//} else {
	//	vDebrisVel = *pUnitDir_WS;
	//}
	//fSpeedAdj -= m_pBot->m_pBotInfo_Gen->fMaxChunksDebrisSpeed;
	//FMATH_CLAMPMIN( fSpeedAdj, -m_pBot->m_pBotInfo_Gen->fMinChunksDebrisSpeed );

	f32 fSpeedAdj;
	CFVec3A vDebrisVel;   
	fSpeedAdj = _CalcSpeedAdjForDebris( &vDebrisVel, pUnitDir_WS, m_pBot->m_pBotInfo_Gen->fMinChunksDebrisSpeed,
										m_pBot->m_pBotInfo_Gen->fMaxChunksDebrisSpeed, m_pBot->m_Velocity_WS );

	DebrisSpawner.m_Mtx.UnitMtxFromUnitVec( &vDebrisVel );
	//DebrisSpawner.m_Mtx.UnitMtxFromUnitVec( pUnitDir_WS );

	DebrisSpawner.m_fPlaneDimX = DebrisSpawner.m_fPlaneDimY = fBoneRadius * 1.75f;
	DebrisSpawner.m_nEmitterType = CFDebrisSpawner::EMITTER_TYPE_BOUNDED_PLANE;
	DebrisSpawner.m_pFcnCallback = CBotPartMgr::_ChunkAndGutsDebrisCallback;
	DebrisSpawner.m_nOwnerID = DEBRISTYPES_BOT_PART_SYSTEM;
	DebrisSpawner.m_pUser = this;
	DebrisSpawner.m_nUser = FALSE;
	DebrisSpawner.m_fMinSpeed = m_pBot->m_pBotInfo_Gen->fMinChunksDebrisSpeed + fSpeedAdj;
	DebrisSpawner.m_fMaxSpeed = m_pBot->m_pBotInfo_Gen->fMaxChunksDebrisSpeed + fSpeedAdj;
	DebrisSpawner.m_fUnitDirSpread = m_pBot->m_pBotInfo_Gen->fUnitChunksSpread;
	DebrisSpawner.m_fScaleMul = pBoneDef->m_fChunkScale;

	DebrisSpawner.m_nFlags = CFDebrisSpawner::FLAG_BUILD_TRACKER_SKIP_LIST | CFDebrisSpawner::FLAG_PLAY_IMPACT_SOUND | CFDebrisSpawner::FLAG_PLAY_FLAMING_SOUND;

	if( m_nFlags & FLAG_TINT_DEBRIS_CHUNKS ) {
		DebrisSpawner.m_nTintRed = m_nTintRed;
		DebrisSpawner.m_nTintGreen = m_nTintGreen;
		DebrisSpawner.m_nTintBlue = m_nTintBlue;

		FMATH_SETBITMASK( DebrisSpawner.m_nFlags, CFDebrisSpawner::FLAG_OVERRIDE_MESH_TINT );
	}

	if( pBoneDef->m_nChunkCount ) {
		DebrisSpawner.m_nMinDebrisCount = (u8)((pBoneDef->m_nChunkCount * _fGlobalDebrisUnitMod) + 1.f);
		DebrisSpawner.m_nMaxDebrisCount = (u8)((pBoneDef->m_nChunkCount * _fGlobalDebrisUnitMod) + 1.f);

		DebrisSpawner.Spawn();
	}

	if( pBoneDef->m_nFlamingChunkCount ) {
		DebrisSpawner.m_pSmokeTrailAttrib = &CBotPartPool::m_SmokeTrailAttrib;
		DebrisSpawner.m_hParticleDef1 = CBotPartPool::m_hDebrisFlame;
		DebrisSpawner.m_hParticleDef2 = CBotPartPool::m_hDebrisSmoke;

		DebrisSpawner.m_nMinDebrisCount = (u8)((pBoneDef->m_nFlamingChunkCount * _fGlobalDebrisUnitMod) + 1.f);
		DebrisSpawner.m_nMaxDebrisCount = (u8)((pBoneDef->m_nFlamingChunkCount * _fGlobalDebrisUnitMod) + 1.f);

		DebrisSpawner.Spawn();
	}

	// spawn an explosion
	if( pBoneDef->m_hExplosion != FEXPLOSION_INVALID_HANDLE ) {
		// do the explosion
		FExplosionSpawnParams_t SpawnParams;
		FExplosion_SpawnerHandle_t hSpawner;

		SpawnParams.InitToDefaults();
		SpawnParams.uFlags = FEXPLOSION_SPAWN_NONE;
		SpawnParams.Pos_WS = pBoneMtx->m_vPos;

		SpawnParams.UnitDir = CFVec3A::m_UnitAxisY;
		SpawnParams.uSurfaceType = 0;
		SpawnParams.pDamageProfile = NULL;
		SpawnParams.pDamager = NULL;

		hSpawner = CExplosion2::GetExplosionSpawner();
		if( hSpawner != FEXPLOSION_INVALID_HANDLE ) {
			CExplosion2::SpawnExplosion( hSpawner, pBoneDef->m_hExplosion, &SpawnParams );
		}
	}
}


void CBotPartMgr::_TurnBoneIntoDebris( u32 nBoneIndex, BOOL bSilentlyVanish ) {
	FASSERT( nBoneIndex < m_pBot->m_pWorldMesh->m_pMesh->nBoneCount );

	if( _GetSeveredBoneBitMask( nBoneIndex ) ) {
		// Already severed...
		return;
	}

	// Flag this bone so that nobody else (like the bot's anim layer) drives it...
	_SetSeveredBoneBitMask( nBoneIndex );
	m_pBot->m_Anim.m_pAnimCombiner->EnableBoneCallback( nBoneIndex );

	// Kill any entities attached to this bone...
	u32 i, nChildEntityCount;
	CEntity *pChildEntity;
	CEntity *pGrandChildEntity;
	cchar *pszBoneName;
	BOOL bKeepVoidBone = FALSE;
	static CEntity *apChildEntity[100];

	pszBoneName = m_pBot->m_pWorldMesh->m_pMesh->pBoneArray[ nBoneIndex ].szName;

	nChildEntityCount = 0;
	for( pChildEntity = m_pBot->GetFirstChild(); pChildEntity; pChildEntity = m_pBot->GetNextChild(pChildEntity) ) {
		if( pChildEntity->GetAttachBoneName() ) {
			if( !fclib_stricmp( pChildEntity->GetAttachBoneName(), pszBoneName ) ) {
				if( pChildEntity->TypeBits() & ENTITY_BIT_BOT){
					bKeepVoidBone = TRUE;
					continue;
				}
				if( pChildEntity == (CEntity *)m_pBot->m_apWeapon[0] ) {
					bKeepVoidBone = TRUE;
					continue;
				}

				if( pChildEntity == (CEntity *)m_pBot->m_apWeapon[1] ) {
					bKeepVoidBone = TRUE;
					continue;
				}

				for( i=0; i<m_pBot->m_anNumDupWeapons[0]; ++i ) {
					if( pChildEntity == (CEntity *)m_pBot->m_aapDupWeapons[0][i] ) {
						bKeepVoidBone = TRUE;
						continue;
					}
				}

				for( i=0; i<m_pBot->m_anNumDupWeapons[1]; ++i ) {
					if( pChildEntity == (CEntity *)m_pBot->m_aapDupWeapons[1][i] ) {
						bKeepVoidBone = TRUE;
						continue;
					}
				}

				apChildEntity[nChildEntityCount] = pChildEntity;

				++nChildEntityCount;

				if( nChildEntityCount >= 100 ) {
					break;
				}
			}
		}
	}

	for( i=0; i<nChildEntityCount; ++i ) {
		apChildEntity[i]->Die();
		apChildEntity[i]->RemoveFromWorld();
	}

	if( !bKeepVoidBone ) {
		if( m_pBot->m_pWorldMesh->m_pMesh->pBoneArray[nBoneIndex].nFlags & FMESH_BONEFLAG_VOIDBONE ) {
			// Nothing more to do for void bones...
			return;
		}
	} else { // this bone is sticking around a short time, so it's ChildEntity's weren't autokilled...
		for( pChildEntity = m_pBot->GetFirstChild(); pChildEntity; pChildEntity = m_pBot->GetNextChild(pChildEntity) ) {
			if( pChildEntity->GetAttachBoneName() ) {
				if( !fclib_stricmp( pChildEntity->GetAttachBoneName(), pszBoneName ) ) { // usually is a weapon
					// but I'm going to kill the children's children, (one exception), to fix particle spheres
					// getting really really big via repositioning to infinity.. This is not as optimal as deferring
					// childs' childrens death until "the final reposition". But probably safer.
					for( pGrandChildEntity = pChildEntity->GetFirstChild(); pGrandChildEntity; pGrandChildEntity = pChildEntity->GetNextChild(pGrandChildEntity) ) {
						if( pGrandChildEntity->TypeBits() & ENTITY_BIT_BOT){
							continue;
						} else {
							pGrandChildEntity->Die();
							pGrandChildEntity->RemoveFromWorld();
						}
					}
				}
			}
		}
	}

	// This is a bone with geo on it...

	// Find bone in bone info array...
	for( i=0; i<m_pPartPool->m_nBoneInfoCount; ++i ) {
		if( m_pPartPool->m_pBoneInfo[i].m_nBoneIndex == nBoneIndex ) {
			break;
		}
	}

	if( i == m_pPartPool->m_nBoneInfoCount ) {
		// This bone is not in the info file. Pop it off the screen...

		_KillBoneGeo( nBoneIndex );

		return;
	}

	if( bSilentlyVanish ) {
		// Make no effects nor sound...

		_KillBoneGeo( nBoneIndex );

		return;
	}

	// This bone is in the info file...

	const CBotPartPool::CBoneDef *pBoneDef = &m_pPartPool->m_pBoneInfo[i];
	CFDebrisSpawner DebrisSpawner;
	const CFSphere *pBoneSphere_BS;
	CFMtx43A *pBoneMtx;
	CFVec3A BoneSphereCenter_WS;

	DebrisSpawner.InitToDefaults();

	pBoneMtx = m_pBot->m_pWorldMesh->GetBoneMtxPalette()[ nBoneIndex ];
	pBoneSphere_BS = &m_pBot->m_pWorldMesh->m_pMesh->pBoneArray[ nBoneIndex ].SegmentedBoundSphere_BS;
	BoneSphereCenter_WS.Set( pBoneSphere_BS->m_Pos );
	pBoneMtx->MulPoint( DebrisSpawner.m_Mtx.m_vPos, BoneSphereCenter_WS );

	// Generate guts debris...
	if( m_pBot->m_pBotInfo_Gen->pDebrisGroup_Guts ) {
		f32 fSpeedAdj;
		CFVec3A vDebrisVel;   
		fSpeedAdj = _CalcSpeedAdjForDebris( &vDebrisVel, &CFVec3A::m_UnitAxisY, m_pBot->m_pBotInfo_Gen->fMinGutsDebrisSpeed,
					m_pBot->m_pBotInfo_Gen->fMaxGutsDebrisSpeed, m_pBot->m_Velocity_WS );

		DebrisSpawner.m_pDebrisGroup = m_pBot->m_pBotInfo_Gen->pDebrisGroup_Guts;
		//DebrisSpawner.m_Mtx.m_vZ = CFVec3A::m_UnitAxisY;
		DebrisSpawner.m_Mtx.UnitMtxFromUnitVec( &vDebrisVel );
		DebrisSpawner.m_fMinSpeed = m_pBot->m_pBotInfo_Gen->fMinGutsDebrisSpeed + fSpeedAdj;
		DebrisSpawner.m_fMaxSpeed = m_pBot->m_pBotInfo_Gen->fMaxGutsDebrisSpeed + fSpeedAdj;
		DebrisSpawner.m_fUnitDirSpread = m_pBot->m_pBotInfo_Gen->fUnitGutsSpread;
		DebrisSpawner.m_fScaleMul = pBoneDef->m_fGutScale;
		DebrisSpawner.m_nMinDebrisCount = (u8)((pBoneDef->m_nGutCount * _fGlobalDebrisUnitMod) + 1.f);
		DebrisSpawner.m_nMaxDebrisCount = (u8)((pBoneDef->m_nGutCount * _fGlobalDebrisUnitMod) + 1.f);
		DebrisSpawner.m_pFcnCallback = CBotPartMgr::_ChunkAndGutsDebrisCallback;
		DebrisSpawner.m_nOwnerID = DEBRISTYPES_BOT_PART_SYSTEM;
		DebrisSpawner.m_pUser = this;
		DebrisSpawner.m_nUser = FALSE;
		DebrisSpawner.m_nFlags = CFDebrisSpawner::FLAG_BUILD_TRACKER_SKIP_LIST | CFDebrisSpawner::FLAG_PLAY_IMPACT_SOUND | CFDebrisSpawner::FLAG_PLAY_FLAMING_SOUND;

		DebrisSpawner.Spawn();
	}

	const CFDebrisGroup *pBoneGeoDebrisGroup = m_pBot->m_pBotInfo_Gen->apBoneGeoDebrisGroup[ pBoneDef->m_nBoneGeoDebrisGroupIndex ];
	if( pBoneGeoDebrisGroup  == NULL ) {
		// No bot geo debris group specified. Pop bone geo off the screen...

		_KillBoneGeo( nBoneIndex );

		return;
	}

	if( pBoneGeoDebrisGroup->m_fMaxAliveSecs<=0.0f && pBoneGeoDebrisGroup->m_fMinAliveSecs<=0.0f ) {
		// Pop this bone geo off the screen and spawn its chunks and guts now...

		CFVec3A UnitDir_WS;
		f32 fMag2;

		UnitDir_WS.Sub( DebrisSpawner.m_Mtx.m_vPos, m_pBot->MtxToWorld()->m_vPos );

		fMag2 = UnitDir_WS.MagSq();
		if( fMag2 > 0.0001f ) {
			UnitDir_WS.Mul( fmath_InvSqrt(fMag2) );
		} else {
			UnitDir_WS = CFVec3A::m_UnitAxisY;
		}

		_SpawnBoneChunks( nBoneIndex, &UnitDir_WS );
		_KillBoneGeo( nBoneIndex );

		return;
	}

	// Timed & Impact modes...

	// Turn this bone geo into debris...
	f32 fSpeedAdj;
	CFVec3A vDebrisVel;   
	fSpeedAdj = _CalcSpeedAdjForDebris( &vDebrisVel, &CFVec3A::m_UnitAxisY, m_pBot->m_pBotInfo_Gen->fMinBoneGeoDebrisSpeed,
										m_pBot->m_pBotInfo_Gen->fMaxBoneGeoDebrisSpeed, m_pBot->m_Velocity_WS );

	DebrisSpawner.m_pDebrisGroup = pBoneGeoDebrisGroup;
	//DebrisSpawner.m_Mtx.m_vZ = CFVec3A::m_UnitAxisY;
	DebrisSpawner.m_Mtx.UnitMtxFromUnitVec( &vDebrisVel );
	DebrisSpawner.m_fMinSpeed = m_pBot->m_pBotInfo_Gen->fMinBoneGeoDebrisSpeed + fSpeedAdj;
	DebrisSpawner.m_fMaxSpeed = m_pBot->m_pBotInfo_Gen->fMaxBoneGeoDebrisSpeed + fSpeedAdj;
	DebrisSpawner.m_fUnitDirSpread = m_pBot->m_pBotInfo_Gen->fUnitBoneGeoSpread;
	DebrisSpawner.m_pFcnCallback = CBotPartMgr::_BoneGeoDebrisCallback;
	DebrisSpawner.m_nOwnerID = DEBRISTYPES_BOT_PART_SYSTEM;
	DebrisSpawner.m_pUser = this;
	DebrisSpawner.m_nUser = TRUE;	// (flag so we know which debris to kill in case ResetAllToIntact() is called)
	DebrisSpawner.m_nFlags = CFDebrisSpawner::FLAG_BUILD_TRACKER_SKIP_LIST;

	if( pBoneDef->m_bBoneGeoOnFire ) {
		DebrisSpawner.m_pSmokeTrailAttrib = &CBotPartPool::m_SmokeTrailAttrib;
	}

	if( !DebrisSpawner.Spawn( m_pBot->m_pWorldMesh, nBoneIndex ) ) {
		// Could not spawn...

		_KillBoneGeo( nBoneIndex );
	} else {
		// Bot bone geo chunk is flying through the air...

		FASSERT( m_nBoneGeoDebrisCount < 255 );
		++m_nBoneGeoDebrisCount;
	}
}


// Blows off the bot's bone hierarchy beginning at the specified bone index.
void CBotPartMgr::_BlowOffBoneHierarchyIntoPieces( s32 nBoneIndex, BOOL bNoLimping, BOOL bSilentlyVanish ) {
	FASSERT( nBoneIndex < m_pBot->m_pWorldMesh->m_pMesh->nBoneCount );
	FASSERT(IsCreated());

	if( MultiplayerMgr.IsSinglePlayer() && (nBoneIndex == m_pPartPool->m_nTorsoBoneIndex) ) {
		if( (m_nFlags & (FLAG_RIGHT_LEG_NOT_INTACT | FLAG_LEFT_LEG_NOT_INTACT)) != (FLAG_RIGHT_LEG_NOT_INTACT | FLAG_LEFT_LEG_NOT_INTACT) ) {
			// Bot has at least one of his legs...
			m_pBot->SetNormHealth( 0.5f );
			m_pBot->Die( FALSE );
			m_pBot->m_uBotDeathFlags |= CBot::BOTDEATHFLAG_WALKING_DEAD;
		}

		m_pBot->SetBlownOffTorsoMode( TRUE );
	}

	if( _GetSeveredBoneBitMask( nBoneIndex ) ) {
		// This bone and everything under it in the bone hierarchy has already been turned into flying debris...
		return;
	}

	// Blow off any limbs that are attached to the bone we're blowing off...
	u32 nEndPartType, nPartType, nLimbType;
	LimbState_e nLimbState;
	CFVerlet *pVerlet;
	BOOL bRemoveLimb;

	for( nLimbType=0; nLimbType<m_nLimbTypeCount; ++nLimbType ) {
		nLimbState = (LimbState_e)m_pLimbStateArray[nLimbType].nState;

		if( nLimbState == LIMB_STATE_REMOVED ) {
			// Already removed...
			continue;
		}

		// See if any parts have a base bone that matches the bone we're removing...
		nEndPartType = m_pPartPool->m_pLimbDefArray[nLimbType].m_nEndPartType;
		bRemoveLimb = FALSE;

		for( nPartType=m_pPartPool->m_pLimbDefArray[nLimbType].m_nStartPartType; nPartType<nEndPartType; ++nPartType ) {
			pVerlet = &m_pPartPool->m_pPartTypePoolArray[nPartType].pPartArray->m_Verlet;

			if( nBoneIndex == pVerlet->GetBaseBoneIndex() ) {
				// Found a part that has a base bone that matches the bone we're removing...

				bRemoveLimb = TRUE;

				break;
			}
		}

		if( bRemoveLimb ) {
			_BlowOffLimbIntoPieces( nLimbType, bNoLimping, bSilentlyVanish );
		}
	}

	// Turn this bone into debris...
	_TurnBoneIntoDebris( nBoneIndex, bSilentlyVanish );

	const FMeshBone_t *pMeshBoneArray = m_pBot->m_pWorldMesh->m_pMesh->pBoneArray;
	const FMeshSkeleton_t *pMeshSkeleton;
	const u8 *pSkeletonIndexArray;
	u32 nChildBoneCount, nChildArrayStartIndex, nChildSkeletonIndex;

	// Step through children of this bone and do the same...
	pMeshSkeleton = &pMeshBoneArray[ nBoneIndex ].Skeleton;
	pSkeletonIndexArray = m_pBot->m_pWorldMesh->m_pMesh->pnSkeletonIndexArray;
	nChildBoneCount = pMeshBoneArray[ nBoneIndex ].Skeleton.nChildBoneCount;
	nChildArrayStartIndex = pMeshBoneArray[ nBoneIndex ].Skeleton.nChildArrayStartIndex;

	for( nChildSkeletonIndex=0; nChildSkeletonIndex<nChildBoneCount; ++nChildSkeletonIndex ) {
		nBoneIndex = pSkeletonIndexArray[ nChildArrayStartIndex + nChildSkeletonIndex ];

		_BlowOffBoneHierarchyIntoPieces( nBoneIndex, bNoLimping, bSilentlyVanish );
	}
}


void CBotPartMgr::_BlowOffLimbIntoPieces( u32 nLimbType, BOOL bNoLimping, BOOL bSilentlyVanish ) {
	if( m_pLimbStateArray[nLimbType].nState == LIMB_STATE_REMOVED ) {
		// Already blown off...
		return;
	}

	u32 nFirstPartBaseBoneIndex, nStartPartType, nEndPartType, nPartType, nBoneIndex;
	u8 i;

	nStartPartType = m_pPartPool->m_pLimbDefArray[nLimbType].m_nStartPartType;
	nEndPartType = m_pPartPool->m_pLimbDefArray[nLimbType].m_nEndPartType;

	// Make sound effect...
	nFirstPartBaseBoneIndex = m_pPartPool->m_pPartTypePoolArray[nStartPartType].pPartArray->m_Verlet.GetBaseBoneIndex();

	if( !bSilentlyVanish ) {
		if( m_fSecsUntilNextLimbBlowSound == 0.0f ) {
			m_fSecsUntilNextLimbBlowSound = _MIN_SECS_BETWEEN_LIMB_BLOW_SOUNDS;

			fsndfx_Play3D(
				m_pPartPool->m_pLimbDefArray[nLimbType].m_hBlowOffSound,
				&m_pBot->m_pWorldMesh->GetBoneMtxPalette()[ nFirstPartBaseBoneIndex ]->m_vPos,
				100.0f,
				1.0f,
				1.0
			);
		}

		// spawn an explosion
		if( m_pPartPool->m_pLimbDefArray[nLimbType].m_hBlowOffExplosion != FEXPLOSION_INVALID_HANDLE ) {
			// do the explosion
			FExplosionSpawnParams_t SpawnParams;
			FExplosion_SpawnerHandle_t hSpawner;

			SpawnParams.InitToDefaults();
			SpawnParams.uFlags = FEXPLOSION_SPAWN_NONE;
			SpawnParams.Pos_WS = m_pBot->m_pWorldMesh->GetBoneMtxPalette()[ nFirstPartBaseBoneIndex ]->m_vPos;

			SpawnParams.UnitDir = CFVec3A::m_UnitAxisY;
			SpawnParams.uSurfaceType = 0;
			SpawnParams.pDamageProfile = NULL;
			SpawnParams.pDamager = NULL;

			hSpawner = CExplosion2::GetExplosionSpawner();
			if( hSpawner != FEXPLOSION_INVALID_HANDLE ) {
				CExplosion2::SpawnExplosion( hSpawner, m_pPartPool->m_pLimbDefArray[nLimbType].m_hBlowOffExplosion, &SpawnParams );
			}
		}



	}

	if( m_pLimbStateArray[nLimbType].nState == LIMB_STATE_DANGLING ) {
		// Disable all parts in this dangling limb...

		for( nPartType=nStartPartType; nPartType<nEndPartType; ++nPartType ) {
			nBoneIndex = m_pPartStateArray[nPartType].pDanglingBotPart->m_Verlet.GetBaseBoneIndex();

			// Remove this part from our bone map...
			for( i=0; i<m_nActiveBoneMapCount; ++i ) {
				if( m_pBoneMapArray[i].nBoneIndex == nBoneIndex ) {
					if( i < (m_nActiveBoneMapCount-1) ) {
						m_pBoneMapArray[i] = m_pBoneMapArray[ m_nActiveBoneMapCount - 1 ];
					}

					--m_nActiveBoneMapCount;

					break;
				}
			}

			m_pPartStateArray[nPartType].pDanglingBotPart->SetMode_Disabled();
			m_pPartStateArray[nPartType].pDanglingBotPart = NULL;
		}
	}

	m_pLimbStateArray[nLimbType].nState = LIMB_STATE_REMOVED;
	_UpdateAllComponentStatus();

	for( nPartType=m_pPartPool->m_pLimbDefArray[nLimbType].m_nStartPartType; nPartType<nEndPartType; ++nPartType ) {
		nBoneIndex = m_pPartPool->m_pPartTypePoolArray[nPartType].pPartArray->m_Verlet.GetBaseBoneIndex();
		_BlowOffBoneHierarchyIntoPieces( nBoneIndex, bNoLimping, bSilentlyVanish );
	}

	if( !bNoLimping ) {
		if( m_pPartPool->m_pLimbDefArray[nLimbType].m_nLegType == CBotPartPool::LEG_TYPE_RIGHT ) {
			// This is the right leg...

			FMATH_SETBITMASK( m_nFlags, FLAG_RIGHT_LEG_NOT_INTACT );

			if( m_nFlags & FLAG_LEFT_LEG_NOT_INTACT ) {
				// Left leg is not intact. Destroy it...

				if( m_pBot->IsDead() ) {
					_BlowOffBoneHierarchyIntoPieces( m_pPartPool->m_nRootBoneIndex, TRUE, bSilentlyVanish );
				} else {
					_BlowOffLimbIntoPieces( m_pPartPool->m_nLimbTypeLeftLeg, FALSE, bSilentlyVanish );

					m_pBot->SetLimpState( CBot::BOTLIMPSTATE_NONE );
					m_pBot->SetNoLegsMode( TRUE );
				}

				return;
			}

			m_pBot->GiveBotLimp( TRUE );
		}

		if( m_pPartPool->m_pLimbDefArray[nLimbType].m_nLegType == CBotPartPool::LEG_TYPE_LEFT ) {
			// This is the left leg...

			FMATH_SETBITMASK( m_nFlags, FLAG_LEFT_LEG_NOT_INTACT );

			if( m_nFlags & FLAG_RIGHT_LEG_NOT_INTACT ) {
				// Right leg is not intact. Destroy it...

				if( m_pBot->IsDead() ) {
					_BlowOffBoneHierarchyIntoPieces( m_pPartPool->m_nRootBoneIndex, TRUE, bSilentlyVanish );
				} else {
					_BlowOffLimbIntoPieces( m_pPartPool->m_nLimbTypeRightLeg, FALSE, bSilentlyVanish );

					m_pBot->SetLimpState( CBot::BOTLIMPSTATE_NONE );
					m_pBot->SetNoLegsMode( TRUE );
				}

				return;
			}

			m_pBot->GiveBotLimp( FALSE );
		}
	}
}



