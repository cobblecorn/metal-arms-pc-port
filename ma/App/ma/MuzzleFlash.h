//////////////////////////////////////////////////////////////////////////////////////
// MuzzleFlash.h -  Muzzle Flash class.
//
// Author: Justin Link
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
// 06/21/02 Link		Created.
//////////////////////////////////////////////////////////////////////////////////////

#ifndef _MUZZLEFLASH_H_
#define _MUZZLEFLASH_H_ 1

#include "fang.h"
#include "fdraw.h"
#include "flinklist.h"
#include "fworld.h"



class CFTexInst;
class CFMeshInst;
class CFXfm;
class CMuzzleFlash;
class CFWorldLightItem;


typedef s32 MuzzleFlash_GroupHandle_t;

#define MUZZLEFLASH_NULLGROUPHANDLE		((MuzzleFlash_GroupHandle_t)NULL)

#define MUZZLEFLASH_WORLDLIGHT_COUNT	10

typedef enum {
	MUZZLEFLASH_TYPE_CARD_3D,
	MUZZLEFLASH_TYPE_CARD_POSTER_Z,
	MUZZLEFLASH_TYPE_CARD_POSTER_XY,
	MUZZLEFLASH_TYPE_MESH_3D,

	MUZZLEFLASH_TYPE_COUNT
} MuzzleFlashType_e;


typedef BOOL FcnDraw_t(CMuzzleFlash *pMF, FDrawVtx_t *pVtxArray, const CFXfm *pCamXfm);


FCLASS_ALIGN_PREFIX class CMuzzleFlash
{
public:
	static BOOL InitSystem();
	static void UninitSystem();

	static CFColorMotif m_DefaultLightMotif;

	static MuzzleFlash_GroupHandle_t AllocateNewGroup(MuzzleFlashType_e nType, u32 uNumFlashes, cchar *pszResName, FDrawBlendOp_e = FDRAW_BLENDOP_LERP_WITH_ALPHA_OPAQUE);

	static BOOL AddFlash_Card3D(MuzzleFlash_GroupHandle_t hGroup, const CFVec3A &vecPos, f32 fWidth, f32 fHeight, f32 fThetaZ = 0.0f, f32 fAlpha = 1.0f);
	static BOOL AddFlash_CardPosterZ(MuzzleFlash_GroupHandle_t hGroup, const CFVec3A &vecPos, const CFVec3A &vecUnitDirZ, f32 fWidth, f32 fLengthZ, f32 fAlpha/* = 1.0f*/);
	static BOOL AddFlash_CardPosterXY(MuzzleFlash_GroupHandle_t hGroup, const CFVec3A &vecPos, f32 fScale, f32 fAlpha/* = 1.0f*/);
	static BOOL AddFlash_Mesh3D(MuzzleFlash_GroupHandle_t hGroup, const CFVec3A &vecPos, const CFVec3A &vecUnitDirZ, f32 fScale, f32 fThetaZ = 0.0f);

	static void MuzzleLight( const CFVec3A *pOrig_WS, f32 fRadius_WS, f32 fUnitLightIntensity=1.0f, CFColorMotif *pMotif=&m_DefaultLightMotif );

	static void Work( void );
	static void Draw( void );
	static void AddLights( void );
	static void RemoveLights( void );

	CMuzzleFlash();
	~CMuzzleFlash();

	static BOOL AddToVtxArray_Card3D(CMuzzleFlash *pMF, FDrawVtx_t *pVtxArray, const CFXfm *pCamXfm);
	static BOOL AddToVtxArray_CardPosterZ(CMuzzleFlash *pMF, FDrawVtx_t *pVtxArray, const CFXfm *pCamXfm);
	static BOOL AddToVtxArray_CardPosterXY(CMuzzleFlash *pMF, FDrawVtx_t *pVtxArray, const CFXfm *pCamXfm);

	static FcnDraw_t *m_apDrawTable[MUZZLEFLASH_TYPE_COUNT];

private:
	static const u32 m_uMaxGroupCount;
	static void _ResDestroyCallback( void *pResMem );

	CFVec3A m_vecPos;
	CFVec3A m_vecDirZ;		// Used for TYPE_CARD_POSTER_Z only
	f32 m_fWidth;			// Used as scale for TYPE_MESH_3D
	f32 m_fHeight;			// Used as length for TYPE_CARD_POSTER_Z
	f32 m_fThetaZ;
	f32 m_fAlpha;


	FCLASS_ALIGN_PREFIX class CMuzzleFlashGroup
	{
	public:
		CMuzzleFlashGroup();
		~CMuzzleFlashGroup();

		BOOL Create(MuzzleFlashType_e nType, u32 uNumFlashes, cchar *pszResName, FDrawBlendOp_e eBlendOp = FDRAW_BLENDOP_LERP_WITH_ALPHA_OPAQUE);

		void DrawCards(const CFXfm *pCamXfm);
		void DrawMeshes(void);

		FINLINE FDrawBlendOp_e GetBlendOp() { return(m_eAlphaBlendOp); }
		FINLINE const CFTexInst *GetTexInst() { return(m_pTexInst); }

		FLink_t m_Link;

	private:
		MuzzleFlashType_e m_nType;
		FDrawBlendOp_e m_eAlphaBlendOp;
		CMuzzleFlash *m_paFlashes;
		u32 m_uFlashCnt, m_uMaxFlashCnt;

		union {
			CFTexInst *m_pTexInst;
			CFMeshInst *m_pMeshInst;
		};

		friend class CMuzzleFlash;

		FCLASS_STACKMEM_ALIGN(CMuzzleFlashGroup);
	} FCLASS_ALIGN_SUFFIX;


	static BOOL m_bSystemInitted;
	static FLinkRoot_t m_LinkRoot_FDraw;
	static FLinkRoot_t m_LinkRoot_FMesh;

	static u32 m_nLightCount;
	static CFWorldLightItem *m_apWorldLight[MUZZLEFLASH_WORLDLIGHT_COUNT];


public:		// SER: Embedding classes within other classes sucks
	void DrawMuzzleFlashMesh( CMuzzleFlashGroup *pMFG );


	FCLASS_STACKMEM_ALIGN(CMuzzleFlash);
} FCLASS_ALIGN_SUFFIX;

#endif
