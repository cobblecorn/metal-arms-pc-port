
//////////////////////////////////////////////////////////////////////////////////////
// fdecal.cpp - decal system.
//
// Author: Mike Elliott   
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
// 05.05.03 Elliott     Created.
//////////////////////////////////////////////////////////////////////////////////////



#include "fang.h"
#include "fdecal.h"
#include "fcoll.h"
#include "fvtxpool.h"
#include "fviewport.h"
#include "fxfm.h"
#include "frenderer.h"
#include "floop.h"
#include "fworld.h"
#include "fstringtable.h"
#include "fworld_coll.h"

#define SAS_ACTIVE_USER -1 //SAS_USER_ELLIOTT

#if SAS_ACTIVE_USER == SAS_USER_ELLIOTT
	#include "ftext.h"
	#include "ftimer.h"
	#define MEDEVPRINTF DEVPRINTF 

	static CFVec3 _vTest1;
	static CFVec3 _vTest2;
#else
	#define MEDEVPRINTF 1 ? (void)0 : (void)fang_DevPrintf
#endif 

// 200, 3000

//#define _MAX_DECALS						( 200 )			// move htis into fang config
//#define _DECAL_VERTS					( 3000 )
#define _SORT_BY_TEXTURE				( 1 )

#define _MIN_SIZE_THRESHOLD				( 50 )
#define _CSV_FNAME						( "decals" )
#define _MAX_TRIS_PER_DECAL				( 200 )
#define _NUM_SCRATCH_VERTS				( (_MAX_TRIS_PER_DECAL * 3) + 24 )			// +24 because we might have to clip one more tri.
#define _POOL_BLOCK_SIZE				( 8 )
#define _MAX_SURFACE_COS				( -0.1f )
#define _OO_ONEMINUS_MAX_SURFACE_COS	( 1.0f / (1.0f - _MAX_SURFACE_COS) )
#define _SURFACE_PUSH					( 0.002f )
#define _MAX_SEGMENTS					( 4 )


BOOL						CFDecal::m_bModuleStartedUp			= FALSE;
FDrawVtx_t*					CFDecal::m_pVtxPool					= NULL;
CFDecal*					CFDecal::m_paDecals					= NULL;
FLinkRoot_t					CFDecal::m_FreeDecalList;	
FLinkRoot_t					CFDecal::m_ActiveDecalList;	
FLinkRoot_t					CFDecal::m_PendingDecalList;
FLinkRoot_t					CFDecal::m_DecalDefList;
CFDataPool					CFDecal::m_VertexPool;
u8*							CFDecal::m_pauSideBuffer;
u8*							CFDecal::m_pauSegmentBuffer;
CFVec3*						CFDecal::m_pavPtBuffer;
CFDecal::AllowCallback_t*	CFDecal::m_pAllowFn					= NULL;
u64							CFDecal::m_uPockmarkCollisionMask	= FCOLL_USER_TYPE_BITS_ALL;
u64							CFDecal::m_uBlastCollisionMask		= FCOLL_USER_TYPE_BITS_ALL;


static CFVec3A		_avTmp0[10];							// used to hold polygons while clipping
static CFVec3A		_avTmp1[10];
static CFWorldMesh*	_apSegmentWMeshes[_MAX_SEGMENTS];
static u32			_auSegmentBones[_MAX_SEGMENTS];
static u32			_auSegmentVtxCounts[_MAX_SEGMENTS];
static CFMtx43A		_mtxTempParent;


#if !FANG_PRODUCTION_BUILD
	static u32 _uTotalNumDecals = 0;
	static u32 _uTotalNumVerts	= 0;
	static u32 _uPeakDecals		= 0;
	static u32 _uPeakVts		= 0;
#endif


BOOL CFDecal::ModuleStartup( void ) {
	#if !FANG_PRODUCTION_BUILD
		u32 uFreeBytes = fres_GetFreeBytes();
	#endif

	m_bModuleStartedUp = TRUE;

	flinklist_InitRoot( &m_FreeDecalList, FANG_OFFSETOF( CFDecal, m_link ) );
	flinklist_InitRoot( &m_ActiveDecalList, FANG_OFFSETOF( CFDecal, m_link ) );
	flinklist_InitRoot( &m_PendingDecalList, FANG_OFFSETOF( CFDecal, m_link ) );


	m_paDecals = fnew CFDecal[Fang_ConfigDefs.nMaxDecals];
	if( !m_paDecals ) {
		DEVPRINTF( "CFDecal::ModuleStartup():  Unable to allocate pool\n" );
		goto _ExitWithError;
	}

	m_VertexPool.Create( sizeof( FDrawVtx_t ), Fang_ConfigDefs.nMaxDecalVertices, 0, _POOL_BLOCK_SIZE );
	m_pVtxPool = (FDrawVtx_t*)m_VertexPool.GetArray( 1 );
	m_VertexPool.ReturnToPool( m_pVtxPool, 1 );

	for( u32 i=0; i<Fang_ConfigDefs.nMaxDecals; i++ ) {
		flinklist_AddTail( &m_FreeDecalList, &m_paDecals[i] );
	}

	m_pavPtBuffer = (CFVec3*)fres_Alloc( sizeof( CFVec3 ) * _NUM_SCRATCH_VERTS );
	if( !m_pavPtBuffer ) {
		goto _ExitWithError;
	}

	m_pauSideBuffer = (u8*)fres_Alloc( sizeof( u8 ) * _NUM_SCRATCH_VERTS );
	if( !m_pauSideBuffer ) {
		goto _ExitWithError;
	}

	m_pauSegmentBuffer = (u8*)fres_Alloc( sizeof( u8 ) * _NUM_SCRATCH_VERTS );
	if( !m_pauSegmentBuffer ) {
		goto _ExitWithError;
	}


	#if !FANG_PRODUCTION_BUILD
		DEVPRINTF( "Decal system using %u bytes of memory.  Creating %d decals with %d verts\n", uFreeBytes - fres_GetFreeBytes(), Fang_ConfigDefs.nMaxDecals, Fang_ConfigDefs.nMaxDecalVertices );
	#endif

	return TRUE;

_ExitWithError:
	return FALSE;
}


void CFDecal::ModuleShutdown( void ) {

//#if !FANG_PRODUCTION_BUILD
//	DEVPRINTF( "Decal system shutting down:  %d decals total, %d peak, %d vtx peak, %0.2f vtx/decal average\n", _uTotalNumDecals, _uPeakDecals, _uPeakVts, ((f32)_uTotalNumVerts/(f32)_uTotalNumDecals) );
//#endif


	if( m_paDecals ) {
		fdelete_array( m_paDecals );
		m_paDecals = NULL;
	}


	m_bModuleStartedUp = FALSE;
}


BOOL CFDecal::LoadData( void ) {
	#if !FANG_PRODUCTION_BUILD
		u32 uFreeBytes = fres_GetFreeBytes();
	#endif

	flinklist_InitRoot( &m_DecalDefList, FANG_OFFSETOF( CFDecalDef, link ) );

	FGameDataFileHandle_t	hFile;
	FGameDataWalker_t		gdataWalker;
	FGameDataTableHandle_t	hTable;

	FMemFrame_t memFrame = fmem_GetFrame();
	FResFrame_t	resFrame = fres_GetFrame();

	CFDecalDef *pDecalDef = NULL;

	hFile = fgamedata_LoadFileToFMem( _CSV_FNAME );
	if( hFile == FGAMEDATA_INVALID_FILE_HANDLE ) {
		DEVPRINTF( "CFDecal::_LoadDefData():  Error loading file %s\n", _CSV_FNAME );
		goto _ExitWithError;
	}

	for( hTable = fgamedata_GetFirstTable( hFile, gdataWalker ); hTable != FGAMEDATA_INVALID_TABLE_HANDLE; hTable = fgamedata_GetNextTable( gdataWalker ) ) {
		pDecalDef = _CreateDefDataFromTable( hTable );
		if( pDecalDef ) { 
			flinklist_AddTail( &m_DecalDefList, pDecalDef );
		} else {
			DEVPRINTF( "CFDecal::_LoadDefData():  Error loading definition table %s\n", fgamedata_GetTableName( hTable ) );
		}
	}

	fmem_ReleaseFrame( memFrame );

	#if !FANG_PRODUCTION_BUILD
		DEVPRINTF( "Decal system using %u bytes of memory during loaddata.\n", uFreeBytes - fres_GetFreeBytes() );
	#endif

	return TRUE;

_ExitWithError:
	fmem_ReleaseFrame( memFrame );
	fres_ReleaseFrame( resFrame );
	return FALSE;
}


CFDecalDef* CFDecal::_CreateDefDataFromTable( FGameDataTableHandle_t hTable ) {

	CFDecalDef *pDecalDef = NULL;
	cchar *pszTableName = fgamedata_GetTableName( hTable );

	if( GetDefByName( pszTableName ) != FDECALDEF_INVALID_HANDLE ) {
		DEVPRINTF( "CFDecal::_CreateDefDataFromTable():  Duplicate entry found: %s\n", pszTableName );
		return NULL;
	}

	pDecalDef = fnew CFDecalDef;
	if( !pDecalDef ) {
		DEVPRINTF( "CFDecal::_CreateDefDataFromTable():  Unable to allocate memory for new CFDecalDef\n" );
		return NULL;
	}

	pDecalDef->pszName = CFStringTable::AddString( NULL, pszTableName );

	if( !fgamedata_GetTableData( hTable, m_aGameDataVocab, pDecalDef, pDecalDef->GetGameDataSize(), 0 ) ) {
		DEVPRINTF( "CFDecal::_CreateDefDataFromTable():  Unable to read table %s\n", pszTableName );
		fdelete( pDecalDef );
		return NULL;
	}

	//// fixup here
	//if( pDecalDef->uFlags == 1 ) {
	//	pDecalDef->uFlags = CFDecal::FLAG_APPLY_TO_STATIC_OBJECTS;
	//} else if( pDecalDef->uFlags == 2 ) {
	//	pDecalDef->uFlags = CFDecal::FLAG_APPLY_TO_STATIC_OBJECTS | CFDecal::FLAG_APPLY_TO_DYNAMIC_OBJECTS;
	//}

	return pDecalDef;
}


FDecalDefHandle_t CFDecal::GetDefByName( cchar *pszName ) {
	FASSERT( IsModuleStartedUp() );
	CFDecalDef *pDecalDef = NULL;

	for( pDecalDef = (CFDecalDef*)flinklist_GetHead( &m_DecalDefList ); pDecalDef; pDecalDef = (CFDecalDef*)flinklist_GetNext( &m_DecalDefList, pDecalDef ) ) {
		if( !fclib_stricmp( pszName, pDecalDef->pszName ) ) {
			return pDecalDef;
		}
	}

	return NULL;
}


void CFDecal::Create( FDecalDefHandle_t hDecalDef, const CFVec3A &vPos, const CFVec3A *pUnitDir ) {
	FASSERT( IsModuleStartedUp() );

	if( hDecalDef == FDECALDEF_INVALID_HANDLE ) {
		return;
	}

	CFDecal *pDecal;
	pDecal = (CFDecal*)flinklist_RemoveHead( &m_FreeDecalList );
	if( !pDecal ) {
		pDecal = _ReclaimDecal();
		if( !pDecal ) {
			DEVPRINTF( "CFDecal::Create():  No free decals and couldn't reclaim one.\n" );
			return;
		}
	}

	CFDecalDef *pDecalDef = (CFDecalDef*)hDecalDef;

	fang_MemCopy( &pDecal->m_Def, pDecalDef, sizeof( CFDecalDef ) );
	
	pDecal->m_vPoint		= vPos;
	pDecal->m_vUnitDir.ReceiveNegative( *pUnitDir );
	pDecal->m_vUnitNormal	= CFVec3A::m_Null;
	pDecal->m_fOOLifeSpan	= fmath_Inv( pDecalDef->fLifeSpanInSecs );
	pDecal->m_fRadius		= fmath_RandomFloatRange( pDecalDef->fMinRadius, pDecalDef->fMaxRadius );
	pDecal->m_TexInst.SetTexDef( pDecalDef->pTexDef );
	pDecal->m_TexInst.ClearFlag( CFTexInst::FLAG_WRAP_S | CFTexInst::FLAG_WRAP_T );
	
	
	// set this so the decal knows it needs to find the initial collision data
	pDecal->m_bNeedsInitialColl = TRUE;

	flinklist_AddTail( &m_PendingDecalList, pDecal );
}


void CFDecal::Create( FDecalDefHandle_t hDecalDef, const FCollImpact_t *pImpact, const CFVec3A *pUnitDir/*=NULL*/ ) {
	FASSERT( IsModuleStartedUp() );
	if( hDecalDef != FDECALDEF_INVALID_HANDLE ) {
	    Create( (CFDecalDef*)hDecalDef, pImpact, pUnitDir );
	}
}


void CFDecal::Create( const CFDecalDef *pDecalDef, const FCollImpact_t *pImpact, const CFVec3A *pUnitDir/*=NULL*/ ) {
	FASSERT( IsModuleStartedUp() );

	if( pDecalDef->fLifeSpanInSecs < 0.01f ) {
		return;
	}

	CFDecal *pDecal;
	pDecal = (CFDecal*)flinklist_RemoveHead( &m_FreeDecalList );
	if( !pDecal ) {
		pDecal = _ReclaimDecal();
		if( !pDecal ) {
			DEVPRINTF( "CFDecal::Create():  No free decals and couldn't reclaim one.\n" );
			return;
		}
	}

	fang_MemCopy( &pDecal->m_Def, pDecalDef, sizeof( CFDecalDef ) );

	pDecal->m_fOOLifeSpan		= fmath_Inv( pDecalDef->fLifeSpanInSecs );
	pDecal->m_fRadius			= fmath_RandomFloatRange( pDecalDef->fMinRadius, pDecalDef->fMaxRadius );
	pDecal->m_bNeedsInitialColl = FALSE;
	pDecal->m_TexInst.SetTexDef( pDecalDef->pTexDef );
	pDecal->m_TexInst.ClearFlag( CFTexInst::FLAG_WRAP_S | CFTexInst::FLAG_WRAP_T );

	pDecal->_InitFromCollisionData( pImpact );
	if( pUnitDir ) {
		pDecal->m_vUnitDir = *pUnitDir;
	} else {
		pDecal->m_vUnitDir.ReceiveNegative( pImpact->UnitFaceNormal );
	}

	flinklist_AddTail( &m_PendingDecalList, pDecal );
}



//
// Assumes:
//  - The viewport has been set up
//	- FDraw is the current renderer
//


#if SAS_ACTIVE_USER == SAS_USER_ELLIOTT
	static u32 uNoDraws				= 0;
	static u32 uNoTexSwitches		= 0;
	static f32 fTotalDrawSecs		= 0.0f;
	static CFTimer timer;
#endif

void CFDecal::Draw( void ) {
	u32 uPrevBiasLevel;
	CFDecal *pDecal = NULL;
	CFXfm xfm;

//	frenderer_Push( FRENDERER_DRAW, NULL );

	pDecal = (CFDecal*)flinklist_GetHead( &m_ActiveDecalList );

	if( pDecal == NULL ) {
		return;
	}

#if SAS_ACTIVE_USER == SAS_USER_ELLIOTT
	u32 uVertsDrawn = 0;

	timer.Reset();
	uNoDraws++;
	uNoTexSwitches;
#endif



	FTexDef_t *pCurrentTex = NULL;
	CFTexInst currentTex;

	uPrevBiasLevel = fdraw_Depth_GetBiasLevel();
#if FANG_PLATFORM_DX
	fdraw_Depth_SetBiasLevel( 20 );
#else
	fdraw_Depth_SetBiasLevel( 3 );
#endif


	//fdraw_Alpha_SetBlendOp( FDRAW_BLENDOP_LERP_WITH_ALPHA_OPAQUE );
	//fdraw_Color_SetFunc( FDRAW_COLORFUNC_DECAL_AI );

	fdraw_Alpha_SetBlendOp( FDRAW_BLENDOP_LERP_WITH_ALPHA_OPAQUE );
	fdraw_Color_SetFunc( FDRAW_COLORFUNC_DIFFUSETEX_AIAT );
	fdraw_Depth_EnableWriting( FALSE );
	fdraw_SetCullDir( FDRAW_CULLDIR_CCW );

	while( pDecal ) {
		if( pDecal->_IsVisible() ) {
			if( pCurrentTex != pDecal->m_TexInst.GetTexDef() ) {

				fdraw_SetTexture( &(pDecal->m_TexInst) );
				pCurrentTex = pDecal->m_TexInst.GetTexDef();
				pDecal->m_TexInst.SetFlag( 0 );

			#if SAS_ACTIVE_USER == SAS_USER_ELLIOTT
				uNoTexSwitches++;
			#endif

			}

//			xfm.BuildFromMtx( *(pDecal->_CalcParentMtx()) );
//			xfm.BuildFromMtx( *(pDecal->m_pMtx) );
			xfm.BuildFromMtx( *(pDecal->_GetParentMtx()) );
			xfm.PushModel();

			
			fdraw_PrimList( FDRAW_PRIMTYPE_TRILIST, pDecal->m_paVtx, pDecal->m_uNumVerts );

			CFXfm::PopModel();

	#if SAS_ACTIVE_USER == SAS_USER_ELLIOTT
			uVertsDrawn += pDecal->m_uNumVerts;
	#endif
		}

		pDecal = (CFDecal*)flinklist_GetNext( &m_ActiveDecalList, pDecal );
	}

	fdraw_Depth_SetBiasLevel( uPrevBiasLevel );
//	frenderer_Pop();

#if SAS_ACTIVE_USER == SAS_USER_ELLIOTT
	fTotalDrawSecs += timer.SampleSeconds() * 1000.0f;
	ftext_DebugPrintf( 0.5f, 0.3f, "~w1DVerts:  %d", uVertsDrawn );

	ftext_DebugPrintf( 0.3f, 0.53f, "~w1D: %d, T: %0.4f, ATS: %0.2f", uNoDraws, (f32)(fTotalDrawSecs/(f32)uNoDraws), (f32)(uNoTexSwitches/(f32)uNoDraws) );
#endif
}

#define _CULL_BACKFACES 0

u32 CFDecal::_GetEligibleTris( void ) {
	// for now, just doing a sphere collision
	CFCollData colldata;
	CFSphere sphere;

	CFVec3A vNull;
#if _CULL_BACKFACES
	sphere.m_Pos.Mul( m_vUnitDir.v3, -m_fRadius );
	sphere.m_Pos.Add( m_vPoint.v3 );
	sphere.m_fRadius = m_fRadius;
	vNull.Mul( m_vUnitDir.v3, m_fRadius );
#else
	sphere.Set( m_vPoint.v3, m_fRadius );
	vNull.Zero();
#endif

	colldata.nCollMask = FCOLL_MASK_COLLIDE_WITH_THIN_PROJECTILES | FCOLL_MASK_COLLIDE_WITH_THICK_PROJECTILES;

#if _CULL_BACKFACES
	colldata.nFlags = FCOLL_DATA_IGNORE_BACKSIDE;
#else
	colldata.nFlags	= NULL;
#endif

	if( m_Def.uType == TYPE_POCKMARK ) {
		colldata.nTrackerUserTypeBitsMask = m_uPockmarkCollisionMask;
	} else {
		colldata.nTrackerUserTypeBitsMask = m_uBlastCollisionMask;
	}

	colldata.nTrackerUserTypeBitsMask	|= 0x8000000000000000;
	colldata.nStopOnFirstOfCollMask		= FCOLL_MASK_NONE;
	colldata.pCallback					= NULL;
	colldata.pLocationHint				= NULL;
	colldata.pMovement					= &vNull;
	colldata.nTrackerSkipCount			= 0;

	

	fcoll_Clear();
	fcoll_Check( &colldata, &sphere );

	if( FColl_nImpactCount != FColl_nMaxCollImpacts ) {
		return FColl_nImpactCount;
	}

	MEDEVPRINTF( "Too many collisions at full size, try min size... " );
	// otherwise, let's just try to draw a small one
	m_fRadius = m_Def.fMinRadius;
    sphere.Set( m_vPoint.v3, m_fRadius );

	fcoll_Clear();
	fcoll_Check( &colldata, &sphere );

	return FColl_nImpactCount;
}


BOOL CFDecal::_CreateDecal( void ) {
	u32 uNumTris;
	u32 uSegments;

	uNumTris = _GetEligibleTris();

	MEDEVPRINTF( "CFDecal::_CreateDecal()  Creating... %d eligible tris...", uNumTris );

	if( uNumTris <= 0 ) {
		MEDEVPRINTF( "CFDecal::_CreateDecal(): hit 0 tris\n" );
		// that's odd...
		return FALSE;
	}

	if( uNumTris == FColl_nMaxCollImpacts ) {
		MEDEVPRINTF( "CFDecal::_CreateDecal()  Got too many tris even at min size, can't create\n" );
		return FALSE;
	}

	_CullTris();

	MEDEVPRINTF( "%d tris culled, %d remaining...", uNumTris - FColl_nImpactCount, FColl_nImpactCount );

	if( FColl_nImpactCount <= 0 ) {
		MEDEVPRINTF( "\nCFDecal::_CreateDecal(): no tris\n" );
		return FALSE;
	}

	if( FColl_nImpactCount == FColl_nMaxCollImpacts ) {
		MEDEVPRINTF( "\nCFDecal::_CreateDecal(): hit too many tris (%d)\n", uNumTris );
		return FALSE;
	}

	if( FColl_nImpactCount > _MIN_SIZE_THRESHOLD ) {
		MEDEVPRINTF( "\nLots of tris, drawing min size decal...\n" );
		m_fRadius = m_Def.fMinRadius;
	}

    
	//// clip the tris...
	CFMtx43A mOrient;
	mOrient.UnitMtxFromUnitVec( &m_vUnitNormal );

	f32 fHalfWidth = m_fRadius * (1.0f/FMATH_SQRT2);

	u32 uNumVerts = _ClipTrisToCube( mOrient, fHalfWidth, &uSegments );

	if( uSegments > _MAX_SEGMENTS ) {
        MEDEVPRINTF( "\nToo many segments, out of here\n" );
		return FALSE;
	}

	MEDEVPRINTF( "Finished clipping, %d tris... ", uNumVerts / 3 );

	if( uNumVerts == 0 ) {
		MEDEVPRINTF( "\nNo verts, failed\n", uNumVerts );
		return FALSE;
	}

	// build the decal(s)
	f32 fRot = FMATH_2PI * fmath_RandomFloat();

	MEDEVPRINTF( "This decal has %d segments\n", uSegments );
	if( !_ConstructDecalFromVerts( uNumVerts, fRot, mOrient, 0, _auSegmentVtxCounts[0] ) ) {
		return FALSE;
	}

	for( u32 i=1; i<uSegments; i++ ) {
		_CreateDecalSegment( this, uNumVerts, fRot, mOrient, i, _apSegmentWMeshes[i], _auSegmentBones[i], _auSegmentVtxCounts[i] );
	}

	m_fUnitLifetime = 1.0f;


	m_pWMesh = _apSegmentWMeshes[0];
	m_nBoneIdx = _auSegmentBones[0];

	//_CalcParentMtx( m_pWMesh, m_nBoneIdx );
	

	return TRUE;
}


BOOL CFDecal::_CreateDecalSegment( const CFDecal *pPrimaryDecal, u32 uNumVerts, f32 fRot, const CFMtx43A &mOrient, u32 uSegment, const CFWorldMesh *pWMesh, s32 uBoneIdx, u32 uVtxCount ) {
	CFDecal *pDecal = (CFDecal*)flinklist_RemoveHead( &m_FreeDecalList );
	if( !pDecal ) {
		pDecal = _ReclaimDecal();
		if( !pDecal ) {
			DEVPRINTF( "CFDecal::_CreateDecalSegment():  No free decals and couldn't reclaim one.\n" );
			return FALSE;
		}
	}


	pDecal->m_Def 					= pPrimaryDecal->m_Def;
	pDecal->m_vUnitNormal			= pPrimaryDecal->m_vUnitNormal;;
	pDecal->m_vPoint				= pPrimaryDecal->m_vPoint;
	pDecal->m_vUnitDir				= pPrimaryDecal->m_vUnitDir;
	pDecal->m_fRadius				= pPrimaryDecal->m_fRadius;
	pDecal->m_TexInst				= pPrimaryDecal->m_TexInst;
	pDecal->m_bNeedsInitialColl		= pPrimaryDecal->m_bNeedsInitialColl;
	pDecal->m_fOOLifeSpan			= pPrimaryDecal->m_fOOLifeSpan;
	pDecal->m_bNeedsInitialColl		= pPrimaryDecal->m_bNeedsInitialColl;
	pDecal->m_fOOLifeSpan			= pPrimaryDecal->m_fOOLifeSpan;
	pDecal->m_pWMesh				= pWMesh;
	pDecal->m_nBoneIdx				= uBoneIdx;

	if( pDecal->_ConstructDecalFromVerts( uNumVerts, fRot, mOrient, uSegment, uVtxCount ) ) {
		pDecal->m_fUnitLifetime = 1.0f;

		//pDecal->_CalcParentMtx( pWMesh, uBoneIdx );
		pDecal->m_pWMesh = pWMesh;
		pDecal->m_nBoneIdx = (u32)uBoneIdx;
		pDecal->_CalcMSVerts();

		#if !FANG_PRODUCTION_BUILD
			_uTotalNumDecals++;
			if( m_ActiveDecalList.nCount > _uPeakDecals ) {
				_uPeakDecals = m_ActiveDecalList.nCount;
			}
			_uTotalNumVerts += pDecal->m_uNumVerts;
			if( m_VertexPool.GetNumUsed() > _uPeakVts ) {
				_uPeakVts = m_VertexPool.GetNumUsed();
			}
		#endif

		flinklist_AddTail( &m_ActiveDecalList, pDecal );
		MEDEVPRINTF( "Decal segment created\n" );
		return TRUE;
	} else {
		DEVPRINTF( "Failed to create decal segment\n" );
		flinklist_AddTail( &m_FreeDecalList, pDecal );
		return FALSE;
	}
	
	
}


//void CFDecal::_CalcParentMtx( const CFWorldMesh *pWMesh, u32 uBoneIdx ) {
//	// see whether the decal's attached to a dynamic object or bone
//	if( pWMesh && pWMesh->GetType() == FWORLD_TRACKERTYPE_MESH ) {
//
//		if( (uBoneIdx >= 0) && (uBoneIdx < pWMesh->m_pMesh->nBoneCount) ) {
//			MEDEVPRINTF( "***DECAL - Hit a bone on an object\n" );
//			m_pMtx = pWMesh->GetBoneMtxPalette()[uBoneIdx];
//		} else {
//			MEDEVPRINTF( "***DECAL - Hit an object\n" );
//			m_pMtx = &(pWMesh->m_Xfm.m_MtxF);
//		}
//	} else {
//		m_pMtx = &CFMtx43A::m_IdentityMtx;
//	}
//
//}

const CFMtx43A* CFDecal::_GetParentMtx( void ) {
	// attached to an object, transform the points into the object's space
	if( m_pWMesh ) {
		if( (m_nBoneIdx >= 0) && (m_nBoneIdx < m_pWMesh->m_pMesh->nBoneCount) ) {
			// hit a bone on an object

			if( m_pWMesh->m_nFlags & FMESHINST_FLAG_NOBONES ) {
				_mtxTempParent.Mul( m_pWMesh->m_Xfm.m_MtxF, m_pWMesh->m_pMesh->pBoneArray[m_nBoneIdx].AtRestBoneToModelMtx );
				return &_mtxTempParent;
			} else {
				return m_pWMesh->GetBoneMtxPalette()[m_nBoneIdx];
			}
		} else {
			return &(m_pWMesh->m_Xfm.m_MtxF);
		}
	} else {
		return &CFMtx43A::m_IdentityMtx;
	}
}


BOOL CFDecal::_ConstructDecalFromVerts( u32 uNumVerts, f32 fRot, const CFMtx43A &mOrient, u32 uSegment, u32 uSegmentVerts ) {
	FDrawVtx_t *pVtxArray, *pVtx;
	CFVec3A vTmp;
	f32 fOOWidth;

	MEDEVPRINTF( "constructing segment %d, %d/%d verts... ", uSegment, uSegmentVerts, uNumVerts );

	pVtxArray = (FDrawVtx_t*)m_VertexPool.GetArray( uSegmentVerts );

	if( !pVtxArray ) {
		pVtxArray = _ReclaimVertices( uSegmentVerts );
		if( !pVtxArray ) {
			DEVPRINTF( "CFDecal::Create():  No free vertices and couldn't reclaim any.\n" );
			return FALSE;
		}
	}

	pVtx = pVtxArray;

	fOOWidth = fmath_Inv(  2.0f * m_fRadius * (1.0f/FMATH_SQRT2) );


//	CFColorRGBA tempcolor;
//	tempcolor.Set( fmath_RandomFloat(), fmath_RandomFloat(), fmath_RandomFloat(), 1.0f );

	for( u32 i=0; i<uNumVerts; i++ ) {
		if( m_pauSegmentBuffer[i] == uSegment ) {
			pVtx->Pos_MS = m_pavPtBuffer[i];

			// calculate STs
			vTmp.v3.Sub( pVtx->Pos_MS, m_vPoint.v3 );

			pVtx->ST.x = vTmp.Dot( mOrient.m_vRight ) * fOOWidth + 0.5f;
			pVtx->ST.y = vTmp.Dot( mOrient.m_vUp ) * fOOWidth + 0.5f;

			switch( m_pauSideBuffer[i] ) {
			case 0:
				break;

			case 1:
				pVtx->ST.y -= (vTmp.Dot( mOrient.m_vFront ) * fOOWidth);
				break;

			case 2:
				pVtx->ST.y += (vTmp.Dot( mOrient.m_vFront ) * fOOWidth);
				break;

			case 3:
				pVtx->ST.x -= vTmp.Dot( mOrient.m_vFront ) * fOOWidth;
				break;
		
			case 4:
				pVtx->ST.x += vTmp.Dot( mOrient.m_vFront ) * fOOWidth;
				break;
			}
					
			pVtx->ColorRGBA = m_Def.startColor;
			//pVtx->ColorRGBA = tempcolor;

			// rotate the vertices
			pVtx->ST.x -= 0.5f;
			pVtx->ST.y -= 0.5f;
			pVtx->ST.Rotate( fRot );
			pVtx->ST.x += 0.5f;
			pVtx->ST.y += 0.5f;

			pVtx++;
		}
	}

	m_paVtx = pVtxArray;
	m_uNumVerts = (u32)(pVtx - pVtxArray);

	FASSERT( m_uNumVerts == uSegmentVerts );

	fworld_GetVolumeContainingPoint( &m_vPoint, &m_pVolume );

	return TRUE;
}


u32 CFDecal::_ClipTrisToCube( const CFMtx43A &mOrient, f32 fHalfWidth, u32 *puNumSegments ) {
	u32 uCt = 0;
	u32 uSide;
	u32 i, j;
	CFVec3A vPush;
	u32 uSegment;
	u32 uNumSegments;
	static CFVec4A _vPlane0, _vPlane1, _vPlane2, _vPlane3, _vPlane4, _vPlane5;

	uNumSegments = 0;
	// set up the planes
	_vPlane0.Set( mOrient.m_vRight.x, mOrient.m_vRight.y, mOrient.m_vRight.z, fHalfWidth - mOrient.m_vRight.Dot( m_vPoint ) );
	_vPlane1.Set( -mOrient.m_vRight.x, -mOrient.m_vRight.y, -mOrient.m_vRight.z, fHalfWidth + mOrient.m_vRight.Dot( m_vPoint ) );
	_vPlane2.Set( mOrient.m_vUp.x, mOrient.m_vUp.y, mOrient.m_vUp.z, fHalfWidth - mOrient.m_vUp.Dot( m_vPoint ) );
	_vPlane3.Set( -mOrient.m_vUp.x, -mOrient.m_vUp.y, -mOrient.m_vUp.z, fHalfWidth + mOrient.m_vUp.Dot( m_vPoint ) );
	_vPlane4.Set( mOrient.m_vFront.x, mOrient.m_vFront.y, mOrient.m_vFront.z, (fHalfWidth) - mOrient.m_vFront.Dot( m_vPoint ) );
	_vPlane5.Set( -mOrient.m_vFront.x, -mOrient.m_vFront.y, -mOrient.m_vFront.z, (fHalfWidth) + mOrient.m_vFront.Dot( m_vPoint ) );

	// loop through all the impacts
	for( i=0; i<FColl_nImpactCount; i++ ) {

		// if we're at the limit, can't make a decal here, bail
		if( uCt > (_MAX_TRIS_PER_DECAL*3) ) {
			return 0;
		}

		// set up this triangle
		u32 uNumVerts = 3;
		_avTmp0[0] = FColl_apSortedImpactBuf[i]->aTriVtx[0];
		_avTmp0[1] = FColl_apSortedImpactBuf[i]->aTriVtx[1];
		_avTmp0[2] = FColl_apSortedImpactBuf[i]->aTriVtx[2];
		_avTmp0[3] = FColl_apSortedImpactBuf[i]->aTriVtx[0];

		// clip it
		ClipPolyToPlane( _avTmp0, _avTmp1, _vPlane0, &uNumVerts );
		ClipPolyToPlane( _avTmp1, _avTmp0, _vPlane1, &uNumVerts );
		ClipPolyToPlane( _avTmp0, _avTmp1, _vPlane2, &uNumVerts );
		ClipPolyToPlane( _avTmp1, _avTmp0, _vPlane3, &uNumVerts );
		ClipPolyToPlane( _avTmp0, _avTmp1, _vPlane4, &uNumVerts );
		ClipPolyToPlane( _avTmp1, _avTmp0, _vPlane5, &uNumVerts );
		
			
		if( uNumVerts > 0 ) {

			// see which axis this point is aligned to
			if( mOrient.m_vUp.Dot( FColl_apSortedImpactBuf[i]->UnitFaceNormal ) > 0.5f ) {
				uSide = 1;
			} else if( mOrient.m_vUp.Dot( FColl_apSortedImpactBuf[i]->UnitFaceNormal ) < -0.5f ) {
				uSide = 2;
			} else if( mOrient.m_vRight.Dot( FColl_apSortedImpactBuf[i]->UnitFaceNormal ) > 0.5f ) {
				uSide = 3;
			} else if( mOrient.m_vRight.Dot( FColl_apSortedImpactBuf[i]->UnitFaceNormal ) < -0.5f ) {
				uSide = 4;
			} else {
				uSide = 0;
			}

            // push it off the surface a little (because of inaccuracy
			vPush.Mul( FColl_apSortedImpactBuf[i]->UnitFaceNormal, fmath_RandomFloatRange( _SURFACE_PUSH * 0.5f, _SURFACE_PUSH ) );

			for( uSegment=0; uSegment<uNumSegments; uSegment++ ) {
				if( FColl_apSortedImpactBuf[i]->pTag == _apSegmentWMeshes[uSegment] && 
					(!FColl_apSortedImpactBuf[i]->pTag || (FColl_apSortedImpactBuf[i]->nBoneIndex == _auSegmentBones[uSegment]) ) ) {
					break;
				}
			}

			if( uSegment == uNumSegments ) {
				// didn't find one
				if ( uNumSegments >= _MAX_SEGMENTS ) {
					// too many segments
					return 0;
				}
				_auSegmentVtxCounts[uSegment] = 0;
				_apSegmentWMeshes[uSegment] = (CFWorldMesh*)FColl_apSortedImpactBuf[i]->pTag;
				_auSegmentBones[uSegment] = FColl_apSortedImpactBuf[i]->nBoneIndex;
				uNumSegments++;
			}

			*puNumSegments = uNumSegments;

            // break this poly up into tris, and save off the verts
			for( j=1; j<uNumVerts-1; j++ ) {

				m_pauSideBuffer[uCt] = uSide;
				m_pauSegmentBuffer[uCt] = uSegment;
				m_pavPtBuffer[uCt++].Add( _avTmp0[0].v3, vPush.v3 );

				m_pauSideBuffer[uCt] = uSide;
				m_pauSegmentBuffer[uCt] = uSegment;
				m_pavPtBuffer[uCt++].Add( _avTmp0[j].v3, vPush.v3 );

				m_pauSideBuffer[uCt] = uSide;
				m_pauSegmentBuffer[uCt] = uSegment;
				m_pavPtBuffer[uCt++].Add( _avTmp0[j+1].v3, vPush.v3 );

				_auSegmentVtxCounts[uSegment] += 3;
			}
		}
	}

	return uCt;
}


void CFDecal::_ReleaseData( void ) {
	FASSERT( m_paVtx );
	m_VertexPool.ReturnToPool( m_paVtx, m_uNumVerts );
	#if SAS_ACTIVE_USER == SAS_USER_ELLIOTT
		MEDEVPRINTF( "CFDecal::_ReleaseData():  Releasing %d verts\n", m_uNumVerts );
	#endif
}



void CFDecal::Work( void ) {
	FASSERT( IsModuleStartedUp() );
	
	CFDecal *pDecal;

	FASSERT( m_FreeDecalList.nCount + m_ActiveDecalList.nCount + m_PendingDecalList.nCount == Fang_ConfigDefs.nMaxDecals );

	// loop through the pending list, dealing with anything we find there
	for( pDecal = (CFDecal*)flinklist_RemoveHead( &m_PendingDecalList ); pDecal; pDecal = (CFDecal*)flinklist_RemoveHead( &m_PendingDecalList ) ) {
		if( pDecal->m_bNeedsInitialColl ) {
			if( !pDecal->_DoInitialColl() ) {
				MEDEVPRINTF( "CFDecal::Work():  Couldn't create decal, couldn't get an initial coll point.  Throwing it back on the free list\n" );
				flinklist_AddTail( &m_FreeDecalList, pDecal );
				continue;
			}
		}

		if( pDecal->_CreateDecal() ) {
			pDecal->_CalcMSVerts();

			_AddDecalToActiveList( pDecal );
	
			#if !FANG_PRODUCTION_BUILD
				_uTotalNumDecals++;
				if( m_ActiveDecalList.nCount > _uPeakDecals ) {
					_uPeakDecals = m_ActiveDecalList.nCount;
				}

				_uTotalNumVerts += pDecal->m_uNumVerts;
				if( m_VertexPool.GetNumUsed() > _uPeakVts ) {
					_uPeakVts = m_VertexPool.GetNumUsed();
				}
			#endif

		} else {
			MEDEVPRINTF( "CFDecal::Work():  Couldn't create decal, throwing it back on the free list\n" );
			flinklist_AddTail( &m_FreeDecalList, pDecal );
		}
	}

	for( pDecal = (CFDecal*)flinklist_GetHead( &m_ActiveDecalList ); pDecal; pDecal = (CFDecal*)flinklist_GetNext( &m_ActiveDecalList, pDecal ) ) {
		pDecal->_Work();

		if( pDecal->m_fUnitLifetime <= 0.0f ) {
			pDecal->_ReleaseData();

			flinklist_Remove( &m_ActiveDecalList, pDecal );
			flinklist_AddTail( &m_FreeDecalList, pDecal );
		}
	}


#if SAS_ACTIVE_USER == SAS_USER_ELLIOTT
	//fdraw_DevSphere( &_vTest1, 2.0f, &FColor_MotifRed );
	//fdraw_DevSphere( &_vTest2, 2.0f, &FColor_MotifBlue );

	//ftext_DebugPrintf( 0.2f, 0.56f, "~w1:Verts: %d/%d", m_VertexPool.GetNumUsed(), m_VertexPool.GetSize() );
	//ftext_DebugPrintf( 0.2f, 0.6f, "~w1Decals:  A: %d, F: %d, P: %d", m_ActiveDecalList.nCount, m_FreeDecalList.nCount, m_PendingDecalList.nCount );
	//if( _uTotalNumVerts > 0 ) {
	//	ftext_DebugPrintf( 0.1f, 0.63f, "~w1stats:  TD: %d, TV: %d, PD: %d, PV: %d, AVE: %0.2f", _uTotalNumDecals, _uTotalNumVerts, _uPeakDecals, _uPeakVts, (f32)((f32)_uTotalNumVerts/((f32)_uTotalNumDecals)) );
	//}
#endif
}


void _ClipEdge( const CFVec3A &v1, const CFVec3A &v2, const CFVec4A &vPlane, CFVec3A *pvResult ) {
	CFVec3A vTmp;
	CFVec4A v41;
	CFVec4A v42;

	v41.Set( v1.x, v1.y, v1.z, 1.0f );
	v42.Set( v1.x - v2.x, v1.y - v2.y, v1.z - v2.z, 0.0f );

	f32 fT1 = vPlane.Dot( v41 );
	f32 fT2 = vPlane.Dot( v42 );
	f32 fT = fmath_Div( fT1, fT2 );

	pvResult->Sub( v2, v1 );
	pvResult->Mul( fT );
	pvResult->Add( v1 );
}


void CFDecal::ClipPolyToPlane( CFVec3A *pvSource, CFVec3A *pvDest, const CFVec4A &vPlane, u32 *puNumVerts ) {
	u32 uDestVerts = 0;
	f32 fTmp;

	for( u32 i=0; i<*puNumVerts; i++ ) {
		fTmp = pvSource[i].Dot( vPlane.v3 );
		if( pvSource[i].Dot( vPlane.v3 ) + vPlane.w > 0 ) {
			// this point ok, add it to the list
			pvDest[uDestVerts++] = pvSource[i];

			fTmp = pvSource[i+1].Dot( vPlane.v3 );
			if( pvSource[i+1].Dot( vPlane.v3 ) + vPlane.w > 0 ) {
				// this point ok too, don't need to clip this edge
			} else {
				// need to clip this edge
				_ClipEdge( pvSource[i], pvSource[i+1], vPlane, &pvDest[uDestVerts++] );
			}

		} else {
			if( pvSource[i+1].Dot( vPlane.v3 ) + vPlane.w > 0 ) {
				// clip this edge
				_ClipEdge( pvSource[i], pvSource[i+1], vPlane, &pvDest[uDestVerts++] );
			}
		}
	}
	if( uDestVerts > 0 ) {
		pvDest[uDestVerts] = pvDest[0];
	}

	*puNumVerts = uDestVerts;
}


//u32 CFDecal::_CullTris( CFWorldMesh *_apSegmentWMeshes, u32 *_auSegmentBones ) const {
void CFDecal::_CullTris( void ) const {
	FASSERT( FColl_nImpactCount );

	// run through our impact buffer, eliminating ones that don't face the same direction as our decal
	u32 uNumSegments;
	u32 uNumTris = 0;
	CFVec3A vTmp;

	for( u32 i=0; i<FColl_nImpactCount; i++ ) {
		// is this tri part of the same object and bone as the impact?
	//	if( (FColl_aImpactBuf[i].pTag == m_pWMesh) && (!m_pWMesh || (FColl_aImpactBuf[i].nBoneIndex == m_nBoneIdx)) ) {

			// is this tri facing the direction of the blast origin?
			if( m_vUnitDir.Dot( FColl_aImpactBuf[i].UnitFaceNormal ) < -_MAX_SURFACE_COS ) {
				
				if( m_pAllowFn && m_pAllowFn( &(FColl_aImpactBuf[i]), (Flags_e)m_Def.uType ) ) {
				
					// is the blast happening in front of this tri?
					//vTmp.Sub( FColl_aImpactBuf[i].aTriVtx[0], m_vPoint );
					//if( vTmp.Dot( FColl_aImpactBuf[i].UnitFaceNormal ) <= 0.1f ) {

					// toss it in there
					FColl_apSortedImpactBuf[uNumTris++] = &FColl_aImpactBuf[i];

					//}
				}
			}
	//	}
	}

	FColl_nImpactCount = uNumTris;
	if( FColl_nImpactCount == 0 ) {
		uNumSegments = 0;
	}    
}


void CFDecal::_CalcMSVerts( void ) {
	if( !m_pWMesh ) {
		return;
	}

	CFMtx43A::m_Temp.ReceiveAffineInverse( *_GetParentMtx(), TRUE );

	// ok, so now we have the correct matrix to transform all our verts
	FDrawVtx_t *pVtx = m_paVtx;
	for( u32 i=0; i<m_uNumVerts; i++ ) {
		pVtx->Pos_MS = CFMtx43A::m_Temp.m44.MultPoint( pVtx->Pos_MS );
		pVtx++;
	}
}


void CFDecal::_Work( void ) {
	m_fUnitLifetime -= FLoop_fPreviousLoopSecs * m_fOOLifeSpan;

	if( m_fUnitLifetime <= 0.0f ) {
		return;
	}

	if( m_pWMesh && (!m_pWMesh->IsAddedToWorld() || (m_pWMesh->m_nFlags & FMESHINST_FLAG_DONT_DRAW)) ) {
		MEDEVPRINTF( "CFDecal::_Work():  Attached WM not in world any more, decal going away\n" );
		m_fUnitLifetime = 0.0f;
		return;
	}

	// decal still alive, do some work
	BOOL bChangeAlpha = FALSE;
	BOOL bChangeColor = FALSE;
	CFColorRGBA rgba;
	f32 fAlpha;

	if( m_fUnitLifetime < m_Def.fUnitFadeTime ) {
        bChangeAlpha = TRUE;
		fAlpha = 1.0f - fmath_Div( m_Def.fUnitFadeTime - m_fUnitLifetime, m_Def.fUnitFadeTime );
	}

	if( (m_fUnitLifetime > m_Def.fUnitTimeColorStop) && (m_fUnitLifetime < m_Def.fUnitTimeColorStart) ) {
		bChangeColor = TRUE;
		rgba = (m_Def.endColor - m_Def.startColor) * fmath_Div( m_Def.fUnitTimeColorStart - m_fUnitLifetime, m_Def.fUnitTimeColorStart - m_Def.fUnitTimeColorStop );
		rgba += m_Def.startColor;
		FMATH_CLAMP_UNIT_FLOAT( rgba.fRed );
		FMATH_CLAMP_UNIT_FLOAT( rgba.fGreen );
		FMATH_CLAMP_UNIT_FLOAT( rgba.fBlue );
		FMATH_CLAMP_UNIT_FLOAT( rgba.fAlpha );
	}

	if( bChangeAlpha || bChangeColor ) {
		for( u32 i=0; i<m_uNumVerts; i++ ) {
			if( bChangeColor ) {
				m_paVtx[i].ColorRGBA = rgba;
			}

			if( bChangeAlpha ) {
				m_paVtx[i].ColorRGBA.fAlpha = fAlpha;
			}
		}
	}
}


const FGameData_TableEntry_t CFDecal::m_aGameDataVocab[] = {
	FGAMEDATA_VOCAB_TEXDEF,
	FGAMEDATA_VOCAB_U32_BOUND( 0, F32_DATATABLE_4294967295 ),

	FGAMEDATA_VOCAB_F32_UNIT,
	FGAMEDATA_VOCAB_F32_UNIT,
	FGAMEDATA_VOCAB_F32_UNIT,

	FGAMEDATA_VOCAB_F32_UNBOUND,
	FGAMEDATA_VOCAB_F32_UNBOUND,
	FGAMEDATA_VOCAB_F32_UNBOUND,

	FGAMEDATA_VOCAB_F32_UNIT,
	FGAMEDATA_VOCAB_F32_UNIT,
	FGAMEDATA_VOCAB_F32_UNIT,
	FGAMEDATA_VOCAB_F32_UNIT,

	FGAMEDATA_VOCAB_F32_UNIT,
	FGAMEDATA_VOCAB_F32_UNIT,
	FGAMEDATA_VOCAB_F32_UNIT,
	FGAMEDATA_VOCAB_F32_UNIT,

	// End of table:
	FGAMEDATA_VAR_TYPE_COUNT| 0, 0, F32_DATATABLE_0, F32_DATATABLE_0
};


BOOL CFDecal::_DoInitialColl( void ) {
	CFCollData colldata;
	CFSphere sphere;
	CFVec3A vNull;
	vNull.Zero();
	
	FCollImpact_t impact;

	colldata.nFlags						= 0; //FCOLL_DATA_IGNORE_BACKSIDE;
	colldata.nCollMask					= FCOLL_MASK_COLLIDE_WITH_THIN_PROJECTILES | FCOLL_MASK_COLLIDE_WITH_THICK_PROJECTILES;
	colldata.nStopOnFirstOfCollMask		= FCOLL_MASK_NONE;
	
	//if( m_Def.uType == TYPE_POCKMARK ) {
	//	colldata.nTrackerUserTypeBitsMask = m_uPockmarkCollisionMask;
	//} else {
	//	colldata.nTrackerUserTypeBitsMask = m_uBlastCollisionMask;
	//}

	//colldata.nTrackerUserTypeBitsMask	|= 0x8000000000000000;
	colldata.nTrackerUserTypeBitsMask	= FCOLL_USER_TYPE_BITS_ALL;
	colldata.pCallback					= NULL;
	colldata.pLocationHint				= NULL;
	colldata.pMovement					= &vNull;
	colldata.nTrackerSkipCount			= 0;

#if SAS_ACTIVE_USER == SAS_USER_ELLIOTT
	_vTest1 = m_vPoint.v3;
#endif
	
	sphere.Set( m_vPoint.v3, m_Def.fMinRadius * 0.25f );

	
	fcoll_Clear();
	if( fcoll_Check( &colldata, &sphere ) ) {
		fcoll_SortDynamic( TRUE );
		_InitFromCollisionData( FColl_apSortedImpactBuf[0] );
		
#if SAS_ACTIVE_USER == SAS_USER_ELLIOTT
	_vTest2 = FColl_apSortedImpactBuf[0]->ImpactPoint.v3;
#endif

		m_vUnitDir.ReceiveNegative( m_vUnitNormal );
		return TRUE;
	} else {
		MEDEVPRINTF( "CFDecal::_DoInitialColl(): Couldn't find initial collision point\n" );
		return FALSE;
	}
}


void CFDecal::_InitFromCollisionData( const FCollImpact_t *pImpact ) {

	m_vPoint = pImpact->ImpactPoint;
	m_vUnitNormal = pImpact->UnitFaceNormal;

	if( pImpact->pTag && (((CFWorldTracker*)pImpact->pTag)->GetType() == FWORLD_TRACKERTYPE_MESH) ) {
        m_pWMesh = (CFWorldMesh*)pImpact->pTag;
	} else {
		m_pWMesh = (CFWorldMesh*)pImpact->pTag;
	}
	m_nBoneIdx = pImpact->nBoneIndex;
}


void CFDecal::KillAll( void ) {
	FASSERT( IsModuleStartedUp() );

	CFDecal *pDecal;

	FASSERT( m_FreeDecalList.nCount + m_ActiveDecalList.nCount + m_PendingDecalList.nCount == Fang_ConfigDefs.nMaxDecals );

	for( pDecal = (CFDecal*)flinklist_RemoveHead( &m_PendingDecalList ); pDecal; pDecal = (CFDecal*)flinklist_RemoveHead( &m_PendingDecalList ) ) {
		flinklist_AddTail( &m_FreeDecalList, pDecal );
	}

	for( pDecal = (CFDecal*)flinklist_RemoveHead( &m_ActiveDecalList ); pDecal; pDecal = (CFDecal*)flinklist_RemoveHead( &m_ActiveDecalList ) ) {
		pDecal->_ReleaseData();
		flinklist_AddTail( &m_FreeDecalList, pDecal );
	}

	FASSERT( m_FreeDecalList.nCount == Fang_ConfigDefs.nMaxDecals );

	MEDEVPRINTF( "CFDecal::KillAll() %d decals in free list\n", m_FreeDecalList.nCount );
}


CFDecal* CFDecal::_ReclaimDecal( void ) {
	CFDecal *pDecal;
	CFDecal *pDecalToReturn = NULL;

	f32 fShortestLife = 1.0f;
	
	for( pDecal=(CFDecal*)flinklist_GetHead( &m_ActiveDecalList ); pDecal; pDecal = (CFDecal*)flinklist_GetNext( &m_ActiveDecalList, pDecal ) ) {
		if( pDecal->m_fUnitLifetime < fShortestLife ) {
			fShortestLife = pDecal->m_fUnitLifetime;
			pDecalToReturn = pDecal;
		}
	}

	if( pDecalToReturn ) {
		pDecalToReturn->_ReleaseData();
		flinklist_Remove( &m_ActiveDecalList, pDecalToReturn );
	}

	return pDecalToReturn;
}


BOOL CFDecal::_IsVisible( void ) {
	return( m_pVolume && (FVis_anVolumeFlags[m_pVolume->nVolumeID] & FVIS_VOLUME_IN_VISIBLE_LIST) );
}


FDrawVtx_t* CFDecal::_ReclaimVertices( u32 uNumVertices ) {
	// going to go through the list and try to find a decal that has as many verts, release it and return the verts.
	// this isn't going to be foolproof, but should be good enough.

	FDrawVtx_t	*pVtx;
	CFDecal		*pDecal;
	CFDecal		*pDecalToFree = NULL;

	f32 fShortestLife = 1.0f;
	
	for( pDecal=(CFDecal*)flinklist_GetHead( &m_ActiveDecalList ); pDecal; pDecal = (CFDecal*)flinklist_GetNext( &m_ActiveDecalList, pDecal ) ) {
		if( (pDecal->m_uNumVerts >= uNumVertices) && (pDecal->m_fUnitLifetime < fShortestLife) ) {
			fShortestLife = pDecal->m_fUnitLifetime;
			pDecalToFree = pDecal;
		}
	}

	if( pDecalToFree ) {
		pDecalToFree->_ReleaseData();
		flinklist_Remove( &m_ActiveDecalList, pDecalToFree );
		flinklist_AddTail( &m_FreeDecalList, pDecalToFree );
	}

	pVtx = (FDrawVtx_t*)m_VertexPool.GetArray( uNumVertices );
	
	return pVtx;
}


void CFDecal::_AddDecalToActiveList( CFDecal *pDecal ) {

#if _SORT_BY_TEXTURE
	CFDecal *pTgtDecal;
	for( pTgtDecal=(CFDecal*)flinklist_GetHead( &m_ActiveDecalList ); pTgtDecal; pTgtDecal=(CFDecal*)flinklist_GetNext( &m_ActiveDecalList, pTgtDecal ) ) {
		if( pTgtDecal->m_TexInst == pDecal->m_TexInst ) {
			flinklist_AddAfter( &m_ActiveDecalList, pTgtDecal, pDecal );
			return;
		}
	}
#endif

    flinklist_AddTail( &m_ActiveDecalList, pDecal );
}


CFDecal::AllowCallback_t* CFDecal::RegisterMaterialCallback( AllowCallback_t *pFn ) {
 	AllowCallback_t *pOldFn = m_pAllowFn;
	m_pAllowFn = pFn;

	return pOldFn;
}


void CFDecal::SetCollisionMasks( u64 uPockmarkMask, u64 uBlastMask ) {
	m_uPockmarkCollisionMask = uPockmarkMask;
	m_uBlastCollisionMask = uBlastMask;
}

u64 CFDecal::GetPockmarkCollisionMask( void ) { 
	FASSERT( IsModuleStartedUp() );
	
	return m_uPockmarkCollisionMask;
}

u64 CFDecal::GetBlastCollisionMask( void ) {
	FASSERT( IsModuleStartedUp() );
	
	return m_uBlastCollisionMask;
}

