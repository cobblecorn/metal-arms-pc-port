//////////////////////////////////////////////////////////////////////////////////////
// MuzzleFlash.cpp -  Muzzle Flash class.
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
// 08/15/02 Ranck		Optimized.
// 08/19/02 Ranck		Extended to support more types of muzzle flashes.
// 08/21/02 Ranck		Added a dynamic light pool.
//////////////////////////////////////////////////////////////////////////////////////



#include "fresload.h"
#include "fcamera.h"
#include "frenderer.h"

#include "MuzzleFlash.h"
#include "fvtxpool.h"
#include "gamecam.h"
#include "LightPool.h"
#include "player.h"

CMuzzleFlash::CMuzzleFlashGroup::CMuzzleFlashGroup()
{
	m_pTexInst = NULL;
	m_paFlashes = NULL;
	m_uMaxFlashCnt = 0;
}


CMuzzleFlash::CMuzzleFlashGroup::~CMuzzleFlashGroup()
{
	if (m_paFlashes)
	{  //fdelete_array is strange if called on null.
		fdelete_array(m_paFlashes);
	}
	m_paFlashes = NULL;
	m_uMaxFlashCnt = 0;
	if (m_pTexInst)
	{
		fdelete(m_pTexInst);
		m_pTexInst = NULL;
	}

	if (m_pMeshInst)
	{
		fdelete(m_pMeshInst);
		m_pMeshInst = NULL;
	}
}


BOOL CMuzzleFlash::CMuzzleFlashGroup::Create(MuzzleFlashType_e nType, u32 uNumFlashes, cchar *pszResName, FDrawBlendOp_e eBlendOp/* = FDRAW_BLENDOP_LERP_WITH_ALPHA_OPAQUE*/)
{
	FASSERT(m_paFlashes == NULL);
	FASSERT(nType>=0 && nType<MUZZLEFLASH_TYPE_COUNT);

	FResFrame_t ResFrame = fres_GetFrame();

	if( nType != MUZZLEFLASH_TYPE_MESH_3D )
	{
		FTexDef_t *pTexDef = (FTexDef_t *)(fresload_Load(FTEX_RESNAME, pszResName));
		if(pTexDef == NULL)
		{
			DEVPRINTF("CMuzzleFlashGroup::Create() : Could not load texture '%s'.\n", pszResName);
			goto _ExitWithError;
		}

		m_pTexInst = fnew CFTexInst;
		m_pTexInst->SetTexDef(pTexDef);
	}
	else
	{
		FMesh_t *pMesh = (FMesh_t *)(fresload_Load(FMESH_RESTYPE, pszResName));
		if(pMesh == NULL)
		{
			DEVPRINTF("CMuzzleFlashGroup::Create() : Could not load mesh '%s'.\n", pszResName);
			goto _ExitWithError;
		}

		FMeshInit_t MeshInit;

		MeshInit.fCullDist = FMATH_MAX_FLOAT;
		MeshInit.Mtx.Identity();
		MeshInit.nFlags = FMESHINST_FLAG_NONE;
		MeshInit.pMesh = pMesh;

		m_pMeshInst = fnew CFMeshInst;
		m_pMeshInst->Init( &MeshInit );
	}

	m_nType = nType;
	m_eAlphaBlendOp = eBlendOp;

	m_paFlashes = fnew CMuzzleFlash[uNumFlashes];
	if(m_paFlashes == NULL)
	{
		DEVPRINTF("CMuzzleFlashGroup::Create() : Out of memory.\n", pszResName);
		goto _ExitWithError;
	}

	m_uFlashCnt = 0;
	m_uMaxFlashCnt = uNumFlashes;

	return(TRUE);

_ExitWithError:
	if(nType != MUZZLEFLASH_TYPE_MESH_3D)
	{
		fdelete(m_pTexInst);
		m_pTexInst = NULL;
	}
	else
	{
		fdelete(m_pMeshInst);
		m_pMeshInst = NULL;
	}

	fres_ReleaseFrame(ResFrame);
	return(FALSE);
}


void CMuzzleFlash::CMuzzleFlashGroup::DrawCards(const CFXfm *pCamXfm)
{
	if(m_uFlashCnt == 0)
	{
		return;
	}

	fdraw_Alpha_SetBlendOp(m_eAlphaBlendOp);
	fdraw_SetTexture(m_pTexInst);

	u32 uVtxCount = m_uFlashCnt * 6;
	u32 uCurVtxIdx = 0;
	FDrawVtx_t *paVtx = fvtxpool_GetArray(uVtxCount);

	FcnDraw_t *pDrawFcn = m_apDrawTable[m_nType];

	CMuzzleFlash *pMF;
	u32 uCurMFIdx;
	for(uCurMFIdx = 0; uCurMFIdx < m_uFlashCnt; ++uCurMFIdx)
	{
		pMF = &(m_paFlashes[uCurMFIdx]);
		if( pDrawFcn(pMF, paVtx + uCurVtxIdx, pCamXfm) )
		{
			uCurVtxIdx += 6;
		}

		FASSERT(uCurVtxIdx <= uVtxCount);
	}

	if(uCurVtxIdx != 0)
	{
		fdraw_PrimList(FDRAW_PRIMTYPE_TRILIST, paVtx, uCurVtxIdx);
	}

	fvtxpool_ReturnArray(paVtx);

	if ( CPlayer::m_nCurrent == (CPlayer::m_nPlayerCount - 1) )
		m_uFlashCnt = 0;
}


void CMuzzleFlash::CMuzzleFlashGroup::DrawMeshes(void)
{
	if(m_uFlashCnt == 0)
	{
		return;
	}

	CMuzzleFlash *pMF;
	u32 uCurMFIdx;
	for(uCurMFIdx = 0; uCurMFIdx < m_uFlashCnt; ++uCurMFIdx)
	{
		pMF = &(m_paFlashes[uCurMFIdx]);
		pMF->DrawMuzzleFlashMesh( this );
	}

	if ( CPlayer::m_nCurrent == (CPlayer::m_nPlayerCount - 1) )
		m_uFlashCnt = 0;
}


CFColorMotif CMuzzleFlash::m_DefaultLightMotif;
const u32 CMuzzleFlash::m_uMaxGroupCount = 10;
FLinkRoot_t CMuzzleFlash::m_LinkRoot_FDraw;
FLinkRoot_t CMuzzleFlash::m_LinkRoot_FMesh;
u32 CMuzzleFlash::m_nLightCount;
CFWorldLightItem *CMuzzleFlash::m_apWorldLight[MUZZLEFLASH_WORLDLIGHT_COUNT];


FcnDraw_t *CMuzzleFlash::m_apDrawTable[MUZZLEFLASH_TYPE_COUNT] = {
	AddToVtxArray_Card3D,
	AddToVtxArray_CardPosterZ,
	AddToVtxArray_CardPosterXY,
	NULL
};


BOOL CMuzzleFlash::m_bSystemInitted = FALSE;



// Static function.
BOOL CMuzzleFlash::InitSystem()
{
	flinklist_InitRoot(&m_LinkRoot_FDraw, FANG_OFFSETOF( CMuzzleFlashGroup, m_Link ));
	flinklist_InitRoot(&m_LinkRoot_FMesh, FANG_OFFSETOF( CMuzzleFlashGroup, m_Link ));

	m_DefaultLightMotif.Set( 1.0f, 0.5f, 0.1f, 1.0f );

	m_nLightCount = 0;

	m_bSystemInitted = TRUE;

	return(TRUE);
}


// Static function.
void CMuzzleFlash::UninitSystem()
{
}


void CMuzzleFlash::AddLights( void )
{
	if( m_nLightCount == 0 )
	{
		return;
	}

	u32 i;
	for( i=0; i<m_nLightCount; ++i )
	{
		m_apWorldLight[i]->UpdateTracker();
	}
}


void CMuzzleFlash::RemoveLights( void )
{
	if( m_nLightCount == 0 )
	{
		return;
	}

	u32 i;
	for( i=0; i<m_nLightCount; ++i )
	{
		CLightPool::ReturnToFreePool( m_apWorldLight[i] );
	}

	m_nLightCount = 0;
}


void CMuzzleFlash::MuzzleLight( const CFVec3A *pOrig_WS, f32 fRadius_WS, f32 fUnitLightIntensity, CFColorMotif *pMotif )
{
	if( m_nLightCount >= MUZZLEFLASH_WORLDLIGHT_COUNT )
	{
		return;
	}

	CFWorldLightItem *pWorldLight = CLightPool::GetFromFreePool();
	if( pWorldLight == NULL )
	{
		return;
	}

	m_apWorldLight[ m_nLightCount++ ] = pWorldLight;

	pWorldLight->m_Light.InitOmniLight( &pOrig_WS->v3, fRadius_WS );
	FMATH_SETBITMASK( pWorldLight->m_Light.m_nFlags, FLIGHT_FLAG_PER_PIXEL );
	pWorldLight->m_Light.SetMotif( pMotif );
	pWorldLight->m_Light.SetIntensity( fUnitLightIntensity );
}


// Static function.
MuzzleFlash_GroupHandle_t CMuzzleFlash::AllocateNewGroup(MuzzleFlashType_e nType, u32 uNumFlashes, cchar *pszResName, FDrawBlendOp_e eBlendOp/* = FDRAW_BLENDOP_LERP_WITH_ALPHA_OPAQUE*/)
{
	CMuzzleFlashGroup *pMFG;

	FResFrame_t ResFrame = fres_GetFrame();

	FResHandle_t hRes = fres_Create( NULL, NULL );
	if(hRes == FRES_NULLHANDLE)
	{
		DEVPRINTF("CMuzzleFlash::AllocateNewGroup() : Trouble creating muzzle flash group resource.\n");
		goto _ExitWithError;
	}

	pMFG = fnew CMuzzleFlashGroup;
	if(pMFG == NULL)
	{
		DEVPRINTF("CMuzzleFlash::AllocateNewGroup() : Out of memory.\n");
		goto _ExitWithError;
	}

	fres_SetBaseAndCallback(hRes, pMFG, _ResDestroyCallback);

	if( nType != MUZZLEFLASH_TYPE_MESH_3D )
	{
		flinklist_AddTail(&m_LinkRoot_FDraw, pMFG);
	}
	else
	{
		flinklist_AddTail(&m_LinkRoot_FMesh, pMFG);
	}

	if(!pMFG->Create(nType, uNumFlashes, pszResName, eBlendOp))
	{
		// Problem creating the muzzle flash group.
		goto _ExitWithError;
	}

	// Group created successfully.

	return(MuzzleFlash_GroupHandle_t)pMFG;

	// Failure.
_ExitWithError:
	fres_ReleaseFrame(ResFrame);
	return(MUZZLEFLASH_NULLGROUPHANDLE);
}


void CMuzzleFlash::_ResDestroyCallback( void *pResMem )
{
	CMuzzleFlashGroup *pMFG = (CMuzzleFlashGroup *)pResMem;

	if( pMFG->m_nType != MUZZLEFLASH_TYPE_MESH_3D )
	{
		flinklist_Remove( &m_LinkRoot_FDraw, pMFG );
	}
	else
	{
		flinklist_Remove( &m_LinkRoot_FMesh, pMFG );
	}

	fdelete( pMFG );
}


// Static function.
BOOL CMuzzleFlash::AddFlash_Card3D(MuzzleFlash_GroupHandle_t hGroup, const CFVec3A &vecPos, f32 fWidth, f32 fHeight, f32 fThetaZ/* = 0.0f*/, f32 fAlpha/* = 1.0f*/)
{
	FASSERT(m_bSystemInitted);

	if(hGroup == MUZZLEFLASH_NULLGROUPHANDLE)
	{
		return(FALSE);
	}

	CMuzzleFlashGroup *pMFG = (CMuzzleFlashGroup *)hGroup;

	if(pMFG->m_uFlashCnt == pMFG->m_uMaxFlashCnt)
	{
		DEVPRINTF("CMuzzleFlash::AddFlash_Card3D() : Could not add another flash.\n");
		return(FALSE);
	}

	CMuzzleFlash *pMF = &pMFG->m_paFlashes[pMFG->m_uFlashCnt];

	pMF->m_vecPos = vecPos;
	pMF->m_fWidth = fWidth;
	pMF->m_fHeight = fHeight;
	pMF->m_fThetaZ = fThetaZ;
	pMF->m_fAlpha = fAlpha;

	++pMFG->m_uFlashCnt;

	return(TRUE);
}


// Static function.
BOOL CMuzzleFlash::AddFlash_CardPosterZ(MuzzleFlash_GroupHandle_t hGroup, const CFVec3A &vecPos, const CFVec3A &vecUnitDirZ, f32 fWidth, f32 fLengthZ, f32 fAlpha/* = 1.0f*/)
{
	FASSERT(m_bSystemInitted);

	if(hGroup == MUZZLEFLASH_NULLGROUPHANDLE)
	{
		return(FALSE);
	}

	CMuzzleFlashGroup *pMFG = (CMuzzleFlashGroup *)hGroup;

	if(pMFG->m_uFlashCnt == pMFG->m_uMaxFlashCnt)
	{
		DEVPRINTF("CMuzzleFlash::AddFlash_Card3D() : Could not add another flash.\n");
		return(FALSE);
	}

	CMuzzleFlash *pMF = &pMFG->m_paFlashes[pMFG->m_uFlashCnt];

	pMF->m_vecPos = vecPos;
	pMF->m_vecDirZ = vecUnitDirZ;
	pMF->m_fWidth = fWidth;
	pMF->m_fHeight = fLengthZ;
	pMF->m_fAlpha = fAlpha;

	++pMFG->m_uFlashCnt;

	return(TRUE);
}


// Static function.
BOOL CMuzzleFlash::AddFlash_CardPosterXY(MuzzleFlash_GroupHandle_t hGroup, const CFVec3A &vecPos, f32 fScale, f32 fAlpha/* = 1.0f*/)
{
	FASSERT(m_bSystemInitted);

	if(hGroup == MUZZLEFLASH_NULLGROUPHANDLE)
	{
		return(FALSE);
	}

	CMuzzleFlashGroup *pMFG = (CMuzzleFlashGroup *)hGroup;

	if(pMFG->m_uFlashCnt == pMFG->m_uMaxFlashCnt)
	{
		DEVPRINTF("CMuzzleFlash::AddFlash_Card3D() : Could not add another flash.\n");
		return(FALSE);
	}

	CMuzzleFlash *pMF = &pMFG->m_paFlashes[pMFG->m_uFlashCnt];

	pMF->m_vecPos = vecPos;
	pMF->m_fWidth = fScale;
	pMF->m_fAlpha = fAlpha;

	++pMFG->m_uFlashCnt;

	return(TRUE);
}


BOOL CMuzzleFlash::AddFlash_Mesh3D(MuzzleFlash_GroupHandle_t hGroup, const CFVec3A &vecPos, const CFVec3A &vecUnitDirZ, f32 fScale, f32 fThetaZ/* = 0.0f*/)
{
	FASSERT(m_bSystemInitted);

	if(hGroup == MUZZLEFLASH_NULLGROUPHANDLE)
	{
		return(FALSE);
	}

	CMuzzleFlashGroup *pMFG = (CMuzzleFlashGroup *)hGroup;

	if(pMFG->m_uFlashCnt == pMFG->m_uMaxFlashCnt)
	{
		DEVPRINTF("CMuzzleFlash::AddFlash_Card3D() : Could not add another flash.\n");
		return(FALSE);
	}

	CMuzzleFlash *pMF = &pMFG->m_paFlashes[pMFG->m_uFlashCnt];

	pMF->m_vecPos = vecPos;
	pMF->m_vecDirZ = vecUnitDirZ;
	pMF->m_fWidth = fScale;
	pMF->m_fThetaZ = fThetaZ;

	++pMFG->m_uFlashCnt;

	return(TRUE);
}


// Static function.
void CMuzzleFlash::Work()
{
	FASSERT(m_bSystemInitted);
}


// Static function.
void CMuzzleFlash::Draw()
{
	FASSERT(m_bSystemInitted);

	CMuzzleFlashGroup *pMFG;

	if( m_LinkRoot_FDraw.nCount )
	{
		CFCamera *pCamera = gamecam_GetActiveCamera();

		frenderer_SetDefaultState();
		fdraw_Depth_EnableWriting(FALSE);
		fdraw_Color_SetFunc( FDRAW_COLORFUNC_DIFFUSETEX_AIAT );

		for(pMFG=(CMuzzleFlashGroup *)flinklist_GetHead(&m_LinkRoot_FDraw); pMFG; pMFG=(CMuzzleFlashGroup *)flinklist_GetNext(&m_LinkRoot_FDraw, pMFG))
		{
			pMFG->DrawCards(pCamera->GetFinalXfm());
		}
	}

	if( m_LinkRoot_FMesh.nCount )
	{
		frenderer_Push( FRENDERER_MESH, NULL );
		fmesh_Ambient_Set( 0.0f, 0.0f, 0.0f, 0.0f );

		for(pMFG=(CMuzzleFlashGroup *)flinklist_GetHead(&m_LinkRoot_FMesh); pMFG; pMFG=(CMuzzleFlashGroup *)flinklist_GetNext(&m_LinkRoot_FMesh, pMFG))
		{
			pMFG->DrawMeshes();
		}

		frenderer_Pop();
	}
}


CMuzzleFlash::CMuzzleFlash()
{
}


CMuzzleFlash::~CMuzzleFlash()
{
}


BOOL CMuzzleFlash::AddToVtxArray_Card3D(CMuzzleFlash *pMF, FDrawVtx_t *pVtxArray, const CFXfm *pCamXfm)
{
	FASSERT(pCamXfm != NULL);

	CFVec3A vecDispX, vecDispY, vecUpperRight, vecLowerRight;
	CFQuatA qRot;

	qRot.BuildQuat(pCamXfm->m_MtxR.m_vFront, pMF->m_fThetaZ);

	vecDispX.Mul(pCamXfm->m_MtxR.m_vRight, pMF->m_fWidth);
	qRot.MulPoint(vecDispX);

	vecDispY.Mul(pCamXfm->m_MtxR.m_vUp, pMF->m_fHeight);
	qRot.MulPoint(vecDispY);

	vecUpperRight.Add(vecDispX, vecDispY);
	vecLowerRight.Sub(vecDispX, vecDispY);

	pVtxArray[0].ColorRGBA.Set( 1.0f, 1.0f, 1.0f, pMF->m_fAlpha );
	pVtxArray[0].Pos_MS = pMF->m_vecPos.v3 - vecLowerRight.v3;
	pVtxArray[0].ST.Set(0.0f, 0.0f);

	pVtxArray[1].ColorRGBA.Set( 1.0f, 1.0f, 1.0f, pMF->m_fAlpha );
	pVtxArray[1].Pos_MS = pMF->m_vecPos.v3 + vecUpperRight.v3;
	pVtxArray[1].ST.Set(1.0f, 0.0f);

	pVtxArray[2].ColorRGBA.Set( 1.0f, 1.0f, 1.0f, pMF->m_fAlpha );
	pVtxArray[2].Pos_MS = pMF->m_vecPos.v3 - vecUpperRight.v3;
	pVtxArray[2].ST.Set(0.0f, 1.0f);

	pVtxArray[3] = pVtxArray[2];
	pVtxArray[4] = pVtxArray[1];

	pVtxArray[5].ColorRGBA.Set( 1.0f, 1.0f, 1.0f, pMF->m_fAlpha );
	pVtxArray[5].Pos_MS = pMF->m_vecPos.v3 + vecLowerRight.v3;
	pVtxArray[5].ST.Set(1.0f, 1.0f);

	return(TRUE);
}


BOOL CMuzzleFlash::AddToVtxArray_CardPosterZ(CMuzzleFlash *pMF, FDrawVtx_t *pVtxArray, const CFXfm *pCamXfm)
{
	FASSERT(pCamXfm != NULL);

	CFVec3A vecLook, vecDispX, vecDispZ, vecUpperRight, vecUpperLeft;
	f32 fMag2;

	vecLook.Sub( pCamXfm->m_MtxR.m_vPos, pMF->m_vecPos );

	vecDispX.Cross( vecLook, pMF->m_vecDirZ );
	fMag2 = vecDispX.MagSq();
	if( fMag2 < 0.0001f )
	{
		return(FALSE);
	}
	vecDispX.Mul( 0.5f * pMF->m_fWidth * fmath_InvSqrt(fMag2) );

	vecDispZ.Mul( pMF->m_vecDirZ, pMF->m_fHeight );

	vecUpperRight.Add(vecDispZ, vecDispX);
	vecUpperLeft.Sub(vecDispZ, vecDispX);

	pVtxArray[0].ColorRGBA.Set( 1.0f, 1.0f, 1.0f, pMF->m_fAlpha );
	pVtxArray[0].Pos_MS = pMF->m_vecPos.v3 + vecUpperLeft.v3;
	pVtxArray[0].ST.Set(1.0f, 0.0f);

	pVtxArray[1].ColorRGBA.Set( 1.0f, 1.0f, 1.0f, pMF->m_fAlpha );
	pVtxArray[1].Pos_MS = pMF->m_vecPos.v3 + vecUpperRight.v3;
	pVtxArray[1].ST.Set(1.0f, 1.0f);

	pVtxArray[2].ColorRGBA.Set( 1.0f, 1.0f, 1.0f, pMF->m_fAlpha );
	pVtxArray[2].Pos_MS = pMF->m_vecPos.v3 - vecDispX.v3;
	pVtxArray[2].ST.Set(0.0f, 0.0f);

	pVtxArray[3] = pVtxArray[2];
	pVtxArray[4] = pVtxArray[1];

	pVtxArray[5].ColorRGBA.Set( 1.0f, 1.0f, 1.0f, pMF->m_fAlpha );
	pVtxArray[5].Pos_MS = pMF->m_vecPos.v3 + vecDispX.v3;
	pVtxArray[5].ST.Set(0.0f, 1.0f);

	return(TRUE);
}


BOOL CMuzzleFlash::AddToVtxArray_CardPosterXY(CMuzzleFlash *pMF, FDrawVtx_t *pVtxArray, const CFXfm *pCamXfm)
{
	FASSERT(pCamXfm != NULL);

	CFVec3A vecDispX, vecDispY, vecUpperRight, vecLowerRight;

	vecDispX.Mul(pCamXfm->m_MtxR.m_vRight, 0.5f * pMF->m_fWidth);
	vecDispY.Mul(pCamXfm->m_MtxR.m_vUp, 0.5f * pMF->m_fWidth);

	vecUpperRight.Add(vecDispX, vecDispY);
	vecLowerRight.Sub(vecDispX, vecDispY);

	pVtxArray[0].ColorRGBA.Set( 1.0f, 1.0f, 1.0f, pMF->m_fAlpha );
	pVtxArray[0].Pos_MS = pMF->m_vecPos.v3 - vecLowerRight.v3;
	pVtxArray[0].ST.Set(0.0f, 0.0f);

	pVtxArray[1].ColorRGBA.Set( 1.0f, 1.0f, 1.0f, pMF->m_fAlpha );
	pVtxArray[1].Pos_MS = pMF->m_vecPos.v3 + vecUpperRight.v3;
	pVtxArray[1].ST.Set(1.0f, 0.0f);

	pVtxArray[2].ColorRGBA.Set( 1.0f, 1.0f, 1.0f, pMF->m_fAlpha );
	pVtxArray[2].Pos_MS = pMF->m_vecPos.v3 - vecUpperRight.v3;
	pVtxArray[2].ST.Set(0.0f, 1.0f);

	pVtxArray[3] = pVtxArray[1];
	pVtxArray[4] = pVtxArray[2];

	pVtxArray[5].ColorRGBA.Set( 1.0f, 1.0f, 1.0f, pMF->m_fAlpha );
	pVtxArray[5].Pos_MS = pMF->m_vecPos.v3 + vecLowerRight.v3;
	pVtxArray[5].ST.Set(1.0f, 1.0f);

	return(TRUE);
}


void CMuzzleFlash::DrawMuzzleFlashMesh( CMuzzleFlashGroup *pMFG )
{
	CFMtx43A::m_Temp.m_vPos	  = m_vecPos;
	CFMtx43A::m_Temp.m_vFront = m_vecDirZ;
	CFMtx43A::m_Temp.m_vRight.CrossYWithVec( m_vecDirZ );

	if( CFMtx43A::m_Temp.m_vRight.MagSq() >= 0.0001f ) {
		CFMtx43A::m_Temp.m_vRight.Unitize();
	} else {
		CFMtx43A::m_Temp.m_vRight = CFVec3A::m_UnitAxisX;
	}
	
	CFMtx43A::m_Temp.m_vUp.Cross( m_vecDirZ, CFMtx43A::m_Temp.m_vRight );

	CFMtx43A::m_RotZ.SetRotationZ( m_fThetaZ );
	
	CFMtx43A::m_Temp.Mul( CFMtx43A::m_RotZ );
	pMFG->m_pMeshInst->m_Xfm.BuildFromMtx( CFMtx43A::m_Temp, m_fWidth );
	pMFG->m_pMeshInst->Draw( FVIEWPORT_PLANESMASK_ALL );
}

