//////////////////////////////////////////////////////////////////////////////////////
// fdecal.h - decal system.
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


#ifndef _FDECAL_H_
#define _FDECAL_H_

#include "fang.h"
#include "ftex.h"
#include "fdraw.h"
#include "fcoll.h"
#include "fdatapool.h"

struct FVisVolume_t;


typedef const void* FDecalDefHandle_t;

#define FDECALDEF_INVALID_HANDLE	( NULL )

FCLASS_ALIGN_PREFIX class CFDecalDef {

public:
	FTexDef_t*	pTexDef;
	u32			uType;
	f32			fUnitFadeTime;
	f32			fUnitTimeColorStart;
	f32			fUnitTimeColorStop;
	f32			fMinRadius;
	f32			fMaxRadius;
	f32			fLifeSpanInSecs;
	CFColorRGBA	startColor;
	CFColorRGBA	endColor;

	cchar		*pszName;
	FLink_t		link;
    
	FCLASS_STACKMEM_ALIGN( CFDecalDef );

public:
	void InitToDefaults( void ) { fang_MemZero( this, sizeof( CFDecalDef ) ); startColor.OpaqueWhite(); }
	
	FINLINE u32 GetGameDataSize( void ) const { return sizeof( CFDecalDef ) - (sizeof( cchar* ) + sizeof( FLink_t)); }

} FCLASS_ALIGN_SUFFIX;




FCLASS_ALIGN_PREFIX class CFDecal {

public:
	typedef enum {
		FLAG_DRAW_ADDITIVE				= 0x1,
//		FLAG_USE_VTX_COLOR				= 0x2,
		FLAG_APPLY_TO_STATIC_OBJECTS	= 0x4,
		FLAG_APPLY_TO_DYNAMIC_OBJECTS	= 0x8,
	
	} Flags_e;

	typedef enum {
		TYPE_NONE,
		TYPE_POCKMARK,
		TYPE_BLAST,

		TYPE_COUNT
	} Type_e;

	typedef BOOL AllowCallback_t( const FCollImpact_t *pImpact, CFDecal::Flags_e decalType );

protected:
private:


public:
protected:
private:

	// statics
	static BOOL				m_bModuleStartedUp;
	static FDrawVtx_t*		m_pVtxPool;
	static FLinkRoot_t		m_FreeDecalList;				// free list
	static FLinkRoot_t		m_ActiveDecalList;				// list that's actively being drawn
	static FLinkRoot_t		m_PendingDecalList;				// these have been created and are waiting for work to get called so the verts can be built
	static FLinkRoot_t		m_DecalDefList;
	static CFDecal*			m_paDecals;
	static CFDataPool		m_VertexPool;
	static CFVec3*			m_pavPtBuffer;
	static u8*				m_pauSideBuffer;
	static u8*				m_pauSegmentBuffer;
	static AllowCallback_t* m_pAllowFn;
	static u64				m_uPockmarkCollisionMask;
	static u64				m_uBlastCollisionMask;

	static const FGameData_TableEntry_t m_aGameDataVocab[];

	// non statics
	FLink_t				m_link;
	FDrawVtx_t*			m_paVtx;
	u32					m_uNumVerts;
	f32					m_fUnitLifetime;
	CFVec3A				m_vUnitNormal;
	//const CFMtx43A*		m_pMtx;
	CFVec3A				m_vPoint;
	CFVec3A				m_vUnitDir;
	CFDecalDef			m_Def;
	f32					m_fOOLifeSpan;
	f32					m_fRadius;
	CFTexInst			m_TexInst;
	BOOL				m_bNeedsInitialColl;
	const CFWorldMesh*	m_pWMesh;
	s32					m_nBoneIdx;
	FVisVolume_t*		m_pVolume;


public:
	static BOOL ModuleStartup( void );
	static void ModuleShutdown( void );
	static FINLINE BOOL IsModuleStartedUp( void ) { return m_bModuleStartedUp; }

	static void Work( void );
	static void Draw( void );

	static void Create( FDecalDefHandle_t hDecalDef, const CFVec3A &vPos, const CFVec3A *pUnitDir );
	static void Create( FDecalDefHandle_t hDecalDef, const FCollImpact_t *pImpact, const CFVec3A *pUnitDir=NULL );
	static void Create( const CFDecalDef *pDecalDef, const FCollImpact_t *pImpact, const CFVec3A *pUnitDir=NULL );
	
	static void ClipPolyToPlane( CFVec3A *pvSource, CFVec3A *pvDest, const CFVec4A &vPlane, u32 *puNumVerts );

	static FDecalDefHandle_t GetDefByName( cchar *pszName );
	static BOOL LoadData( void );
	static void KillAll( void );
	static AllowCallback_t* RegisterMaterialCallback( AllowCallback_t *pNewCB );
	static void SetCollisionMasks( u64 uPockmarkMask, u64 uBlastMask );
	static u64 GetPockmarkCollisionMask( void );
	static u64 GetBlastCollisionMask( void );
	


protected:
private:
	static void _ConvertPolyToTriList( CFVec3A *pvVerts, FDrawVtx_t *pVtxList, u32 *puNumVerts );
	static CFDecalDef* _CreateDefDataFromTable( FGameDataTableHandle_t hTable );
	static BOOL _CreateDecalSegment( const CFDecal *pPrimaryDecal, u32 uNumVerts, f32 fRot, const CFMtx43A &mOrient, u32 uSegment, const CFWorldMesh *pWMesh, s32 uBoneIdx, u32 uSegmentVerts );
	static CFDecal* _ReclaimDecal( void );
	static FDrawVtx_t* _ReclaimVertices( u32 uNumVertices );
	static void _AddDecalToActiveList( CFDecal *pDecal );

	u32 _GetEligibleTris( void );
	BOOL _CreateDecal( void );
	void _ReleaseData( void );
	void _CullTris( void ) const;
	void _CalcMSVerts( void );
	void _Work( void );
	BOOL _DoInitialColl( void );
	void _InitFromCollisionData( const FCollImpact_t *pImpact );
	u32 _ClipTrisToCube( const CFMtx43A &mOrient, f32 fHalfWidth, u32 *puSegments );
	BOOL _ConstructDecalFromVerts( u32 uNumVerts, f32 fRot, const CFMtx43A &mOrient, u32 uSegment, u32 uSegmentVerts );
	//void _CalcParentMtx( const CFWorldMesh *pWMesh, u32 uBoneIdx );
	BOOL _IsVisible( void );
	const CFMtx43A* _GetParentMtx( void );
	
	

	FCLASS_STACKMEM_ALIGN( CFDecal );
} FCLASS_ALIGN_SUFFIX;


#endif
