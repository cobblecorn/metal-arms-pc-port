//////////////////////////////////////////////////////////////////////////////////////
// fliquid.cpp - Fang liquid system module.
//
// Author: Jeremy Chernobieff
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
// 01/03/03 Chernobieff Created.
//////////////////////////////////////////////////////////////////////////////////////

#include "fang.h"
#include "frenderer.h"
#include "fmath_vec.h"
#include "fliquid.h"
#include "fworld.h"
#include "fres.h"
#include "fvid.h"
#include "fworld_coll.h"
#include "fsh.h"
#include "fclib.h"
#include "fmesh.h"

//debug
#include "fdraw.h"

#include "fxfm.h"

#include "floop.h"

#include <math.h>

#define FLIQUID_DRAW_VIS_SPHERE 0

CFLiquidVolume::FLiquid_Data_t *CFLiquidVolume::m_DataPool;
CFLiquidSystem LiquidSystem;

CFLiquidVolume *CFLiquidVolume::m_pSelf=NULL;
f32 CFLiquidVolume::m_fMinPScale=0.0f;
f32 CFLiquidVolume::m_fPScale=0.1f;

FLiquidCollisionCallback_t *FLiquid_pCollisionCallback=NULL;

u32 _DRAG_VALUE=0xfe;
BOOL _bInitStaticData=FALSE;

f32 *_pfFilterBuffer;
CFVec2 _CurrentToScroll(0.1f, 0.1f);

f32 _fTime=0.0f;
//CFSphere _Sphere[3];

BOOL _WorldCallbackFunc( FWorldEvent_e nEvent );

BOOL fliquid_ModuleStartup( void )
{
	LiquidSystem.Reset();

	fworld_RegisterWorldCallbackFunction( _WorldCallbackFunc );
	return TRUE;
}

void fliquid_ModuleShutdown( void )
{
	fworld_UnregisterWorldCallbackFunction( _WorldCallbackFunc );
}

BOOL _WorldCallbackFunc( FWorldEvent_e nEvent )
{
	switch (nEvent)
	{
		case FWORLD_EVENT_WORLD_PRELOAD:
		case FWORLD_EVENT_WORLD_POSTDESTROY:
			LiquidSystem.Reset();
			_fTime=0.0f;
			break;
		case FWORLD_EVENT_WORLD_POSTLOAD:
			break;
		case FWORLD_EVENT_WORLD_PREDESTROY:
			CFLiquidVolume::ReleaseTextures();
			LiquidSystem.ReleaseLiquidVolumeData();
			break;
	};
	return TRUE;
}


//Liquid volume
CFLiquidVolume::CFLiquidVolume( void )
{
	m_fRainDropMag = 0.0f;
		
	m_vCurrent = CFVec2(0.0f,0.0f);
	m_vExt.Set(10.0f, 10.0f, 10.0f);
	
	m_fCullDist2 = 100.0f*100.0f;
	m_fGeoDraw = 100.0f;
	
	m_fdT_Add = 0.0f;
	m_fGlowScale = 1.0f;
	
	m_vScroll = CFVec2(0,0);
	m_fBumpTile = 1.0f;
	
	m_pData		= NULL;
	m_bInit		= FALSE;
	m_bInteract = TRUE;
	m_bProcedural = FALSE;
	m_pTracker[0]	= NULL;
	m_pTracker[1]	= NULL;
	m_pTracker[2]	= NULL;
	m_pParentMesh = NULL;

	m_bUseParticles = FALSE;
	m_hCollisionParticleDef = FPARTICLE_INVALID_HANDLE;
	m_hExplosionParticleDef = FPARTICLE_INVALID_HANDLE;

	m_nFrame = 1;

	m_nLastFrameWork = 0;

	fang_MemSet(m_anColList, 0, (LV_GRID_SIZE*LV_GRID_SIZE)>>3 );
}

void CFLiquidVolume::SetLiquidType(LiquidType_e ltype)
{
	m_nType = ltype;
}

void CFLiquidVolume::SetLiquidFog(CFColorRGB& fogClr, f32 fDensity)
{
	m_fogClr = fogClr;
	m_fDensity = fDensity;
}

void CFLiquidVolume::SetCurrent(CFVec2& vCurrent) 
{ 
	m_vCurrent = vCurrent; 
}

void CFLiquidVolume::SetParticleHandle(FParticle_DefHandle_t hHandle, BOOL bUseParticles)
{
	m_bUseParticles = (hHandle != FPARTICLE_INVALID_HANDLE && bUseParticles)?TRUE:FALSE;
	m_hCollisionParticleDef = hHandle;
}

void CFLiquidVolume::SetupVolume(CFVec3A& vExt, CFMtx43A& vOrient)
{
	if (!m_bProcedural)
	{
		m_vExt = vExt;
		m_Mtx = vOrient;
		
		m_MtxI = m_Mtx; m_MtxI.Invert();
		
		CFVec3A Vtx;
		m_vSurfaceCen.Set(0,0,0);
		
		Vtx.Set(-vExt.x, vExt.y*0.5f, -vExt.z);
		vOrient.MulPoint(Vtx);
		m_vSurface[0].Set(Vtx.x, Vtx.y, Vtx.z);
		m_vSurfaceCen.y = m_vSurface[0].y;
		
		m_vSurfaceCen.x += m_vSurface[0].x;
		m_vSurfaceCen.z += m_vSurface[0].z;
		
		Vtx.Set(+vExt.x, vExt.y*0.5f, -vExt.z);
		vOrient.MulPoint(Vtx);
		m_vSurface[1].Set(Vtx.x, Vtx.y, Vtx.z);
		m_vSurfaceCen.x += m_vSurface[1].x;
		m_vSurfaceCen.z += m_vSurface[1].z;
		
		Vtx.Set(+vExt.x, vExt.y*0.5f, +vExt.z);
		vOrient.MulPoint(Vtx);
		m_vSurface[2].Set(Vtx.x, Vtx.y, Vtx.z);
		m_vSurfaceCen.x += m_vSurface[2].x;
		m_vSurfaceCen.z += m_vSurface[2].z;
		
		Vtx.Set(-vExt.x, vExt.y*0.5f, +vExt.z);
		vOrient.MulPoint(Vtx);
		m_vSurface[3].Set(Vtx.x, Vtx.y, Vtx.z);
		m_vSurfaceCen.x += m_vSurface[3].x;
		m_vSurfaceCen.z += m_vSurface[3].z;
		
		m_fRadius2 = 2*(m_vExt.x*m_vExt.x + m_vExt.z*m_vExt.z);
		m_fRadius = fmath_Sqrt(m_fRadius2);
		
		m_vSurfaceCen.x *= 0.25f;
		m_vSurfaceCen.z *= 0.25f;
		
		m_BoundingSphere.m_fRadius = m_fRadius;
		m_BoundingSphere.m_Pos.x = m_vSurfaceCen.x;
		m_BoundingSphere.m_Pos.y = m_vSurfaceCen.y;
		m_BoundingSphere.m_Pos.z = m_vSurfaceCen.z;

		CFVec3A vOffs;
		//Which axis is longer?
		if (vExt.x > vExt.z) //X Axis longer
		{
			vOffs.z = 0.0f;
			vOffs.y = vExt.y;
			vOffs.x = vExt.x * 0.5f;

			vOrient.MulPoint(vOffs);
			_Sphere[0].m_Pos.Set(vOffs.x, vOffs.y, vOffs.z);

			vOffs.z = 0.0f;
			vOffs.y = vExt.y;
			vOffs.x = -vExt.x * 0.5f;

			vOrient.MulPoint(vOffs);
			_Sphere[1].m_Pos.Set(vOffs.x, vOffs.y, vOffs.z);

			_Sphere[2].m_Pos.Set(m_Mtx.m_vPos.x, m_Mtx.m_vPos.y+vExt.y+2, m_Mtx.m_vPos.z);

			_Sphere[0].m_fRadius = _Sphere[1].m_fRadius = vExt.x * 0.10f;
			_Sphere[2].m_fRadius = vExt.x * 0.10f;
		}
		else
		{
			vOffs.x = 0.0f;
			vOffs.y = vExt.y;
			vOffs.z = vExt.z * 0.5f;

			vOrient.MulPoint(vOffs);
			_Sphere[0].m_Pos.Set(vOffs.x, vOffs.y, vOffs.z);

			vOffs.x = 0.0f;
			vOffs.y = vExt.y;
			vOffs.z = -vExt.z * 0.5f;

			vOrient.MulPoint(vOffs);
			_Sphere[1].m_Pos.Set(vOffs.x, vOffs.y, vOffs.z);

			_Sphere[2].m_Pos.Set(m_Mtx.m_vPos.x, m_Mtx.m_vPos.y+vExt.y+2, m_Mtx.m_vPos.z);

			_Sphere[0].m_fRadius = _Sphere[1].m_fRadius = vExt.z * 0.10f;
			_Sphere[2].m_fRadius = vExt.z * 0.10f;
		}
		//

		// init our trackers...
		m_pTracker[0] = fnew CFWorldTracker( FWORLD_TRACKERTYPE_USER );
		//m_pTracker[0]->AddToWorld();
		if( !m_pTracker[0] ) 
		{
			DEVPRINTF( "CFLiquidVolume::SetupVolume() Unable to allocate user tracker\n" );
			FASSERT_NOW;
		}
		else 
		{
			m_pTracker[0]->MoveTracker( _Sphere[0] );
		}

		m_pTracker[1] = fnew CFWorldTracker( FWORLD_TRACKERTYPE_USER );
		//m_pTracker[1]->AddToWorld();
		if( !m_pTracker[1] ) 
		{
			DEVPRINTF( "CFLiquidVolume::SetupVolume() Unable to allocate user tracker\n" );
			FASSERT_NOW;
		}
		else 
		{
			m_pTracker[1]->MoveTracker( _Sphere[1] );
		}

		m_pTracker[2] = fnew CFWorldTracker( FWORLD_TRACKERTYPE_USER );
		//m_pTracker[2]->AddToWorld();
		if( !m_pTracker[2] ) 
		{
			DEVPRINTF( "CFLiquidVolume::SetupVolume() Unable to allocate user tracker\n" );
			FASSERT_NOW;
		}
		else 
		{
			m_pTracker[2]->MoveTracker( _Sphere[2] );
		}
        		
		m_VolumeSphere.m_fRadius = fmath_AcuSqrt(vExt.x*vExt.x + vExt.y*vExt.y + vExt.z*vExt.z);
		m_VolumeSphere.m_Pos.Set(vOrient.m_vPos.x, vOrient.m_vPos.y, vOrient.m_vPos.z);
		
		if (m_bInit)
		{
			fsh_ChangeRenderPlane(m_nRenderPlaneID, m_vSurface, &m_fogClr, m_fDensity);
		}
	}
}

f32 _afLiquidWaveSpd[] = 
{
	15.0f,
	10.0f,
	0.25f,	//Molten
	15.0f,
	15.0f
};

f32 _u8_f32[256];
#include <stdio.h>

void CFLiquidVolume::InitStaticData()
{
	if (!_bInitStaticData)
	{
		u32 i;
		char szName[32];

		m_DataPool = (FLiquid_Data_t *)fres_AlignedAlloc(sizeof(FLiquid_Data_t)*LV_POOLSIZE, 8);

		for (i=0; i<LV_POOLSIZE; i++)
		{
			sprintf(szName, "LiquidEMBM%d", i);
			m_DataPool[i].m_pEMBM = ftex_CreateDynamicEMBM(LV_GRID_SIZE, LV_GRID_SIZE, (cchar *)szName, FRES_NULLHANDLE, NULL);
		}

		_bInitStaticData = TRUE;
	}
}


void CFLiquidVolume::ReleaseTextures()
{
	if (_bInitStaticData)
	{
		u32 i;
		for (i=0; i<LV_POOLSIZE; i++)
		{
			ftex_ReleaseTex(m_DataPool[i].m_pEMBM);
		}
	}
}

void CFLiquidVolume::ReleaseTracker( void ) 
{
	if( m_pTracker[0] )
	{
		fdelete( m_pTracker[0] );
		m_pTracker[0] = NULL;
	}
	if( m_pTracker[1] )
	{
		fdelete( m_pTracker[1] );
		m_pTracker[1] = NULL;
	}
}


void CFLiquidVolume::InitData()
{
	m_pData->m_pZc = (f32 *)m_pData->m_aZ0;
	m_pData->m_pZn = (f32 *)m_pData->m_aZ1;
	
	u32 nX, nY;
	for (nX=0; nX<LV_GRID_SIZE; nX++)
	{
		for (nY=0; nY<LV_GRID_SIZE; nY++)
		{
		
			if ( (nX < 2 || nY < 2 || nX > LV_GRID_SIZE-3 || nY > LV_GRID_SIZE-3) && !m_bProcedural )
			{
				m_pData->m_aD[nX+nY*LV_GRID_SIZE] = 0x00;
			}
			else
			{
				m_pData->m_aD[nX+nY*LV_GRID_SIZE] = _DRAG_VALUE;
			}
			
			m_pData->m_aZ0[nX+nY*LV_GRID_SIZE] = m_pData->m_aZ1[nX+nY*LV_GRID_SIZE] = 0.0f;
		}
	}
	
	fH = m_vExt.x*2.0f / (f32)LV_GRID_SIZE;
	fC = _afLiquidWaveSpd[ m_nType ];

	f32 fT = (fC*fC)*0.034f/(fH*fH);

	if (fT > 0.5f)
	{
		fC = fmath_AcuSqrt(0.5f*fH*fH*29.4117647f);
	}
	
	fCdivH = fC/fH;	
	
	u32 nValue;
	f32 fOO255 = (1.0f/255.0f);
	for (nValue = 0; nValue < 256; nValue++)
	{
		_u8_f32[nValue] = (f32)nValue * fOO255;
	}
}

void CFLiquidVolume::ChangeWaveSpeed(f32 fNewWaveSpd)
{
	fC = fNewWaveSpd;
	fCdivH = fC/fH;
}

void CFLiquidVolume::Init()
{
	u32 nType;
	
	CFLiquidVolume::InitStaticData();
	
	if (m_nType == LT_WATER) { nType = RP_REFLECT | RP_REFRACT; }
	else if (m_nType == LT_MOLTEN) { nType = RP_EMISSIVE; }
	else if (m_nType != LT_TEXTURE && m_nType != LT_OIL) { nType = RP_REFLECT; }
	else { nType = RP_TEXTURE; }
	
	if (!m_bProcedural)
	{
		m_nRenderPlaneID = fsh_AddRenderPlane(m_vSurface, nType, TRUE, &m_fogClr, m_fDensity, m_fGeoDraw, m_fOpacity, m_fTile, m_fRadius, m_pTexInst[0], m_pTexInst[1]);
	}
		
	m_pData = &m_DataPool[(LiquidSystem.m_nNumLiquidVolume-1)%LV_POOLSIZE]; //for now
	
	InitData();
	m_bInit = TRUE;
	
	if (m_nType != LT_MOLTEN)
	{
		m_fTile *= 0.25f;
	}
}


void CFLiquidVolume::RestoreDrag()
{
	u32 nX, nY;
	for (nY=2; nY<LV_GRID_SIZE-1; nY++)
	{
		for (nX=2; nX<LV_GRID_SIZE-1; nX++)
		{
			m_pData->m_aD[nX+nY*LV_GRID_SIZE] = _DRAG_VALUE;
		}
	}
}

BOOL CFLiquidVolume::Displace(CFVec3A& vPos, f32 fRadius, f32 fMag, BOOL bSet, BOOL bInteract)
{
	CFVec3A vRelPos;
	
	vRelPos.Set(vPos);
	m_MtxI.MulPoint(vRelPos);
	
	vRelPos.x /=  m_vExt.x;
	vRelPos.z /= -m_vExt.z;

	f32 hGS = (f32)LV_GRID_SIZE*0.5f;	
	s32 nX, nY, nRadius=0;
	nX = (s32)( vRelPos.x*hGS + hGS );
	nY = (s32)( vRelPos.z*hGS + hGS );
	if (nX > 1 && nX < LV_GRID_SIZE-1 && nY > 1 && nY < LV_GRID_SIZE-1)
	{
		if (bInteract)
		{
			if (m_nType == LT_OIL) { nRadius = 1; }
			Displace( nX, nY, nRadius, fMag, bSet);
			m_pData->m_aD[nX+nY*LV_GRID_SIZE] = 0x00;
		}

		return TRUE;
	}
	return FALSE;
}

void CFLiquidVolume::Displace(u32 nX, u32 nY, u32 nRadius, f32 fMag, BOOL bSet)
{
	f32 fR2 = (f32)nRadius;
	fR2 *= fR2;

	if (!bSet)
	{
		m_pData->m_pZc[nX+nY*LV_GRID_SIZE] += fMag;
	}
	else
	{
		m_pData->m_pZc[nX+nY*LV_GRID_SIZE] = fMag;
	}
	if (nRadius > 0)
	{
		fR2 = fmath_Inv(fR2);

		s32 nI, nJ, nOx, nOy, nStartI, nEndI, nStartJ, nEndJ;
		f32 fMul;
		
		nStartI = nX-nRadius; if (nStartI < 1) nStartI = 1;
		nEndI = nX+nRadius; if (nEndI > LV_GRID_SIZE-2) nEndI = LV_GRID_SIZE-2;
		
		nStartJ = nY-nRadius; if (nStartJ < 1) nStartJ = 1;
		nEndJ = nY+nRadius; if (nEndJ > LV_GRID_SIZE-2) nEndJ = LV_GRID_SIZE-2;
		
		for (nI = nStartI; nI<nEndI; nI++)
		{
			for (nJ = nStartJ; nJ<nEndJ; nJ++)
			{
				if (nI != nX || nJ != nY)
				{
					nOx = nI-(s32)nX;
					nOy = nJ-(s32)nY;
					
					fMul = 1.0f - ( (f32)nOx * (f32)nOx + (f32)nOy * (f32)nOy ) * fR2;
					
					if (!bSet)
					{
						m_pData->m_pZc[nI+nJ*LV_GRID_SIZE] += fMag*fMul;
					}
					else
					{
						m_pData->m_pZc[nI+nJ*LV_GRID_SIZE] = fMag*fMul;
					}
				}
			}
		}
	}
}

void CFLiquidVolume::DoRain(f32 fdT)
{
	static u32 nDrop=0;
	u32 i, nDrops = (u32)((fdT+m_fdT_Add) * m_fNumRainDrops_Sec);
	u32 nCX, nCY;
	f32 fMag;
	
	if (nDrops < 1)
	{
		m_fdT_Add += fdT;
	}
	else
	{
		m_fdT_Add = 0.0f;
	}
	
	for (i=0; i<nDrops; i++)
	{
		nCX = fmath_RandomRange(2, LV_GRID_SIZE-3);	
		nCY = fmath_RandomRange(2, LV_GRID_SIZE-3);	
		
		fMag = fmath_RandomFloatRange(-m_fRainDropMag, 0.0f);
		
		Displace(nCX, nCY, 1, fMag);
	}
}

void CFLiquidVolume::DoWaves()
{
	CFVec2 waveVec0(1.0f,1.0f), waveVec1(-2.0f,1.5f), waveVec2(2.0f,2.0f);
	f32 fFreq0=10.0f, fFreq1=10.0f, fFreq2=1.0f;//0.1f;
	f32 fAmp0=0.25f, fAmp1=0.1f, fAmp2=0.4f;
	
	u32 nX, nY;
	f32 fCosData0, fCosData1, fCosData2;
	f32 fX, fY, fZ;
	
	//for (nY=0; nY<2/*LV_GRID_SIZE*/; nY++)
	{
		nY = LV_GRID_SIZE/2;
		for (nX=0; nX<LV_GRID_SIZE; nX++)
		{
			fX = (f32)nX*fH; fY = (f32)nY*fH;
						
			fCosData0 = (waveVec0.x*fX+waveVec0.y*fY) - (fFreq0*_fTime);
			fCosData0 = cosf(fCosData0);
			
			fCosData1 = (waveVec1.x*fX+waveVec1.y*fY) - (fFreq1*_fTime);
			fCosData1 = cosf(fCosData1);
			
			fCosData2 = (waveVec2.x*fX+waveVec2.y*fY) - (fFreq2*_fTime);
			fCosData2 = cosf(fCosData2);
			
			fZ = fCosData0*fAmp0 + fCosData1*fAmp1 + fCosData2*fAmp2;
			
			m_pData->m_pZc[nX+nY*LV_GRID_SIZE] = fZ*32.0f;
		}
	}
}

void CFLiquidVolume::DoCurrent(f32 fdT)
{
}

void _FilterData(f32 *pfSrc, s32 nWidth, s32 nHeight)
{
	f32 *pfDst = _pfFilterBuffer;
	f32 fNum;
	
	s32 nX, nY;
	
	for (nY=0; nY<nHeight; nY++)
	{
		for (nX=0; nX<nWidth; nX++, pfDst++)
		{
			fNum = 1.0f;
			*pfDst = pfSrc[nX + nY*nWidth];
			
			if (nX > 0)
			{
				*pfDst += pfSrc[nX-1 + nY*nWidth];
				fNum++;
				
				if (nY > 0)
				{
					*pfDst += pfSrc[nX-1 + (nY-1)*nWidth];
					fNum++;
				}
				if (nY < nHeight-1)
				{
					*pfDst += pfSrc[nX-1 + (nY+1)*nWidth];
					fNum++;
				}
			}
			if (nX < nWidth-1)
			{
				*pfDst += pfSrc[nX+1 + nY*nWidth];
				fNum++;
				
				if (nY > 0)
				{
					*pfDst += pfSrc[nX+1 + (nY-1)*nWidth];
					fNum++;
				}
				if (nY < nHeight-1)
				{
					*pfDst += pfSrc[nX+1 + (nY+1)*nWidth];
					fNum++;
				}
			}
			if (nY > 0)
			{
				*pfDst += pfSrc[nX + (nY-1)*nWidth];
				fNum++;
			}
			if (nY < nHeight-1)
			{
				*pfDst += pfSrc[nX + (nY+1)*nWidth];
				fNum++;
			}
			
			if (fNum)
			{
				*pfDst /= fNum;
			}
		}
	}
}

#define RADIUS_MUL 0.75f
static CFVec3 _vUp(0, 1, 0);

BOOL CFLiquidVolume::CollisionCallback( CFWorldTracker *pTracker, FVisVolume_t *pVolume )
{
	BOOL bRet = TRUE;

	CFSphere sphere = pTracker->GetBoundingSphere();
	sphere.m_fRadius *= RADIUS_MUL;

	//Is this sphere "under" the plane?
	CFVec3A vOffset, vN;
	CFVec3A vSpherePos, vSelfPos;
	
	vSpherePos.Set(sphere.m_Pos);
	vSelfPos.Set(m_pSelf->m_VolumeSphere.m_Pos);

	vOffset.Sub(vSpherePos, vSelfPos);
	//now project this offset onto my plane normal.
	vN.Set(m_pSelf->m_Mtx.m_vUp);
	vN.Unitize();
	f32 fDist = FMATH_FABS(vN.Dot(vOffset)), fMag;

	fDist = fDist - sphere.m_fRadius - 0.5f*m_pSelf->m_vExt.y;
	//Is the sphere under the plane?
	if (fDist <= 0.0f)
	{
		//Do Displacement here.	
		fMag = -2.0f * sphere.m_fRadius;
		if (fMag < -4.0f) { fMag = -4.0f; }
		if ( m_pSelf->Displace(vSpherePos, 1.0f, fMag, TRUE, m_pSelf->m_bInteract) )
		{
			//Here I need to pass on information about the collision.
			CFWorldMesh* pMesh = (CFWorldMesh*)pTracker;
			FLiquid_pCollisionCallback(pMesh, m_pSelf->m_pUserData);

			if (m_pSelf->m_bUseParticles)
			{
				CFVec3 vPartOrig;
				f32 fIntens = (sphere.m_fRadius - m_fMinPScale) * m_fPScale;
				FMATH_CLAMP(fIntens, 0.1f, 1.0f);

				vPartOrig.x = vSpherePos.x; vPartOrig.z = vSpherePos.z;
				vPartOrig.y = vSelfPos.y + 0.5f*m_pSelf->m_vExt.y;

				//Check if already in collision with liquid, if not add it to list.
				if ( !m_pSelf->InCurCollisionList(vPartOrig.x, vPartOrig.z, m_pSelf->m_nFrame) )
				{
					FParticle_EmitterHandle_t hParticleCollision = fparticle_SpawnEmitter( m_pSelf->m_hCollisionParticleDef, vPartOrig,  &_vUp, fIntens ); 
				}
			}
		}
	}
	
	return (bRet);
}

BOOL CFLiquidVolume::CheckCollision(CFVec3& vPos, f32 fRadius)
{
	CFSphere sphere;
	sphere.m_Pos = vPos;
	sphere.m_fRadius = fRadius * RADIUS_MUL;

	//Is this sphere "under" the plane?
	CFVec3A vOffset, vN;
	CFVec3A vSpherePos, vSelfPos;
	
	vSpherePos.Set(sphere.m_Pos);
	vSelfPos.Set(m_VolumeSphere.m_Pos);

	vOffset.Sub(vSpherePos, vSelfPos);
	//now project this offset onto my plane normal.
	vN.Set(m_Mtx.m_vUp);
	vN.Unitize();
	f32 fDist = FMATH_FABS(vN.Dot(vOffset)), fMag;

	fDist = fDist - sphere.m_fRadius - 0.5f*m_vExt.y;
	//Is the sphere under the plane?
	if (fDist <= 1.0f)
	{
		//Do Displacement here.	
		fMag = -2.0f * sphere.m_fRadius;
		if (fMag < -4.0f) { fMag = -4.0f; }
		if ( Displace(vSpherePos, 1.0f, fMag, TRUE, m_bInteract) )
		{
			if (m_bUseParticles && m_hExplosionParticleDef != FPARTICLE_INVALID_HANDLE)
			{
				CFVec3 vPartOrig;
				f32 fIntens = 1.0f;//(sphere.m_fRadius - m_fMinPScale) * m_fPScale;
				//FMATH_CLAMP(fIntens, 0.1f, 1.0f);

				vPartOrig.x = vSpherePos.x; vPartOrig.z = vSpherePos.z;
				vPartOrig.y = vSelfPos.y + 0.5f*m_pSelf->m_vExt.y;

				//Spawn explosion particles
				fparticle_SpawnEmitter( m_hExplosionParticleDef, vPartOrig,  &_vUp, fIntens ); 
			}
			return TRUE;
		}
	}
	return FALSE;
}

void CFLiquidVolume::HandleCollisions()
{
	m_pSelf = this;
	fworld_FindTrackersIntersectingSphere(&m_VolumeSphere, FWORLD_TRACKERTYPE_MESH, CollisionCallback);

	m_nFrame = (m_nFrame < 0xff)?(m_nFrame+1):(1);
}

BOOL CFLiquidVolume::InCurCollisionList(f32 fX, f32 fZ, u32 nFrame)
{
	BOOL bRet = FALSE;
	u32 nIdx, nNewFrame;
	CFVec3 vPos;
	CFVec3A vRelPos;

	vPos.x = fX; vPos.z = fZ; vPos.y = m_Mtx.m_vPos.y;

	vRelPos.Set(vPos);
	m_MtxI.MulPoint(vRelPos);

	fX = vRelPos.x; 
	fZ = vRelPos.z; 

	fX /=  m_vExt.x;
	fZ /= -m_vExt.z;

	f32 hGS = (f32)LV_GRID_SIZE*0.5f;	
	s32 nX, nY;
	nX = (s32)( fX*hGS + hGS );
	nY = (s32)( fZ*hGS + hGS );

	if ( nX >= 0 && nY >= 0 && nX < LV_GRID_SIZE && nY < LV_GRID_SIZE )
	{
		nIdx = nX + nY*LV_GRID_SIZE;
		
		if ( m_anColList[ nIdx ] >= nFrame )
		{
			bRet = TRUE;
		}

		nNewFrame = (nFrame < 0xff)?(nFrame+1):(1);
		m_anColList[ nIdx ] = nNewFrame;
	}

	return bRet;
}

void CFLiquidVolume::Work()
{
	if (m_pData)
	{

		if (!m_bProcedural)
		{
			HandleCollisions();
		}
	
		u32 nX, nY;
		f32 fdT = FLoop_fPreviousLoopSecs;
		
		if (fdT > 0.034f) { fdT = 0.034f; }
				
		RestoreDrag();
		
//DoWaves();
		
		if (m_fRainDropMag)
		{
			DoRain(fdT);
		}
		
		if (m_vCurrent.x || m_vCurrent.y)
		{
			DoCurrent(fdT);
		}
		
		f32 fA = (fCdivH*fdT); fA *= fA;
		f32 fB = 2.0f - 4.0f*fA;

		u32 nYOffs0, nYOffsm1, nYOffsp1;
		f32 LV_GRID_MAXY = LV_GRID_SIZE*(LV_GRID_SIZE-1);

		if (m_bProcedural)
		{
			nYOffs0 = 0;
			nYOffsm1 = (LV_GRID_SIZE-1)*LV_GRID_SIZE;
			nYOffsp1 = nYOffs0 + LV_GRID_SIZE;

			u32 nXm1, nXp1;

			for (nY=0; nY<LV_GRID_SIZE; nY++)
			{
				for (nX=0; nX<LV_GRID_SIZE; nX++)
				{
					nXm1 = (nX==0)?(LV_GRID_SIZE-1):(nX-1);
					nXp1 = (nX==LV_GRID_SIZE-1)?(0):(nX+1);

					m_pData->m_pZn[nX+nYOffs0] = fA*( m_pData->m_pZc[nXm1+nYOffs0] + m_pData->m_pZc[nXp1+nYOffs0] +
						m_pData->m_pZc[nX+nYOffsm1] + m_pData->m_pZc[nX+nYOffsp1]) + fB*m_pData->m_pZc[nX+nYOffs0] - m_pData->m_pZn[nX+nYOffs0];
						
					m_pData->m_pZn[nX+nYOffs0] *= _u8_f32[ m_pData->m_aD[nX+nYOffs0] ];
				}
				nYOffs0 += LV_GRID_SIZE;
				nYOffsm1 += LV_GRID_SIZE;
				nYOffsp1 += LV_GRID_SIZE;

				if (nYOffsp1 > LV_GRID_MAXY)
				{
					nYOffsp1 = 0;
				}

				if (nYOffsm1 > LV_GRID_MAXY)
				{
					nYOffsm1 = 0;
				}
			}
		}
		else
		{
			nYOffs0 = LV_GRID_SIZE;
			nYOffsm1 = 0;
			nYOffsp1 = nYOffs0 + LV_GRID_SIZE;

			for (nY=1; nY<LV_GRID_SIZE-1; nY++)
			{
				for (nX=1; nX<LV_GRID_SIZE-1; nX++)
				{
					m_pData->m_pZn[nX+nYOffs0] = fA*( m_pData->m_pZc[nX-1+nYOffs0] + m_pData->m_pZc[nX+1+nYOffs0] +
						m_pData->m_pZc[nX+nYOffsm1] + m_pData->m_pZc[nX+nYOffsp1]) + fB*m_pData->m_pZc[nX+nYOffs0] - m_pData->m_pZn[nX+nYOffs0];
						
					m_pData->m_pZn[nX+nYOffs0] *= _u8_f32[ m_pData->m_aD[nX+nYOffs0] ];
				}
				nYOffs0 += LV_GRID_SIZE;
				nYOffsm1 += LV_GRID_SIZE;
				nYOffsp1 += LV_GRID_SIZE;
			}
		}
		
		f32 *pTmp;
		pTmp = m_pData->m_pZc;
		m_pData->m_pZc = m_pData->m_pZn;
		m_pData->m_pZn = pTmp;

		nYOffs0 = LV_GRID_SIZE;
		
		for (nY=1; nY<LV_GRID_SIZE-1; nY++)
		{
			for (nX=1; nX<LV_GRID_SIZE-1; nX++)
			{
				if (m_pData->m_pZc[nX+nYOffs0] > 4.0f)
				{
					m_pData->m_pZc[nX+nYOffs0] = 4.0f;
				}
				else if (m_pData->m_pZc[nX+nYOffs0] < -4.0f)
				{
					m_pData->m_pZc[nX+nYOffs0] = -4.0f;
				}
			}
			nYOffs0 += LV_GRID_SIZE;
		}

		if (m_nType == LT_OIL)
		{
			_FilterData(m_pData->m_pZc, LV_GRID_SIZE, LV_GRID_SIZE);
			ftex_HeightMapToEMBM(m_pData->m_pEMBM, _pfFilterBuffer, TRUE);	
		}
		else
		{
#if FANG_PLATFORM_XB | FANG_PLATFORM_PC
			if (m_nType == LT_MOLTEN)
			{
				ftex_HeightMapToEMBM(m_pData->m_pEMBM, m_pData->m_pZc);
			}
			else
			{
				ftex_HeightMapToEMBM(m_pData->m_pEMBM, m_pData->m_pZc, TRUE);
			}
#else
			ftex_HeightMapToEMBM(m_pData->m_pEMBM, m_pData->m_pZc);
#endif
		}
		
		if (m_vCurrent.x || m_vCurrent.y)
		{
			m_vScroll.x += m_vCurrent.x*fdT*_CurrentToScroll.x; m_vScroll.x = fmath_Fmod(m_vScroll.x, 128.0f);
			m_vScroll.y += m_vCurrent.y*fdT*_CurrentToScroll.y; m_vScroll.y = fmath_Fmod(m_vScroll.y, 128.0f);
		}
		
		_fTime += fdT;
	}
}

void CFLiquidVolume::Render()
{
	if (m_pData)
	{
		fsh_RenderPlane(m_nRenderPlaneID, 1.0f, 1.0f, m_vScroll.x, m_vScroll.y, m_pData->m_pEMBM, m_fGlowScale, m_fBumpTile);
	}
}

//Liquid mesh - water falls and such.
CFLiquidMesh::CFLiquidMesh( void )
{
	m_Clr.Set(1,1,1);
	m_fExp = 16.0f;
	m_nType = LT_WATER;
	m_vExt.Set(1.0f, 5.0f, 1.0f);
	m_Mtx.Identity();
	m_MtxI.Identity();
	m_fSpeed = 1.0f;
	m_vMesh = NULL;
	m_pIdx = NULL;

	m_pLiquidVolume = NULL;
}

//Liquid Mesh
void CFLiquidMesh::SetLiquidType(LiquidType_e ltype)
{
	m_nType = ltype;
}

f32 CFLiquidMesh::EvalFunc(f32 fX)
{
	return 1.0f - powf(fX - 1.0f, m_fExp);
}

f32 CFLiquidMesh::EvalFunc(f32 fX, f32 fExp)
{
	return 1.0f - powf(fX - 1.0f, fExp);
}

void CFLiquidMesh::AnimateMesh(f32 fExp, f32 fNextExp)
{
	s32 nInc = 0;
	if (m_vMesh)
	{
		if (fExp > m_fCurExp) { nInc = +1; }
		else { nInc = -1; }
		m_fCurExp = fExp;
		m_fNextExp = fNextExp;

		if (m_fCurExp > m_fNextExp && nInc > 0)
		{
			m_AnimTarget = m_fExp;
			m_fExp = m_fNextExp;
			return;
		}
		else if (m_fCurExp < m_fNextExp && nInc < 0)
		{
			m_AnimTarget = m_fExp;
			m_fExp = m_fNextExp;
			return;
		}

		f32 fU = (m_fCurExp - m_fExp)/(m_fNextExp - m_fExp);

		u32 n=0, i;
		f32 fY, fX, fZ, fdX, fdZ, fY0, fY1;
		fdX = 1.0f / (f32)(LF_MESH_SIZE_VERT-1);
		fdZ = 1.0f / (f32)(LF_MESH_SIZE_HORIZ-1);

		for (fZ = 1.0f; fZ >= 0.0f; fZ -= fdZ)
		{
			for (fX = 1.0f; fX >= 0.0f; fX -= fdX)
			{
				fY0 = EvalFunc(fX, m_fExp);
				fY1 = EvalFunc(fX, m_fNextExp);

				if (fY0 != fY1)
				{
					fY = fY0 + fU*(fY1 - fY0);
				}
				else
				{
					fY = fY0;
				}

				//now build a pos from this data.
				m_vMesh[n].vPos.x = fX;
				m_vMesh[n].vPos.y = fY;
				m_vMesh[n].vPos.z = fZ;

				n++;
			}
		}

		//now pull back my vertices on the edges.
		for (i=0; i<m_nVtx; i++)
		{
			if (m_vMesh[i].vPos.z == 0.0f)
			{
				m_vMesh[i].vPos.x += 0.20f;
				m_vMesh[i-LF_MESH_SIZE_VERT].vPos.z -= fdZ*0.80f;//0.20f;
			}
			else if (m_vMesh[i].vPos.z == 1.0f)
			{
				m_vMesh[i].vPos.x += 0.20f;
				m_vMesh[i+LF_MESH_SIZE_VERT].vPos.z += fdZ*0.80f;//0.20f;
			}
		}

		CFVec3A vPos;
		//now transform my vertices.
		for (i=0; i<m_nVtx; i++)
		{
			m_vMesh[i].vPos.x =-(m_vMesh[i].vPos.x*2.0f - 1.0f) * m_vExt.x;
			m_vMesh[i].vPos.y = (m_vMesh[i].vPos.y*2.0f - 1.0f) * m_vExt.y;
			m_vMesh[i].vPos.z = (m_vMesh[i].vPos.z*2.0f - 1.0f) * m_vExt.z;

			vPos.Set(m_vMesh[i].vPos);
			vPos = m_Mtx.MulPoint(vPos);
			m_vMesh[i].vPos.Set(vPos.x, vPos.y, vPos.z);
		}
		//
	}
}

void CFLiquidMesh::SetupMesh(CFVec3A& vExt, CFMtx43A& vOrient, f32 fExp)
{
	m_vExt = vExt;
	m_Mtx = vOrient;
	m_MtxI = m_Mtx;
	m_MtxI.Invert();

	m_AnimTarget = m_fExp * 0.5f;
	m_fExp = fExp;
	m_fCurExp = m_fExp;

	//Build base mesh here.
	m_vMesh = (LiquidFallVtx *)fres_Alloc( sizeof(LiquidFallVtx)*LF_MESH_SIZE_VERT*LF_MESH_SIZE_HORIZ );
	m_pIdx = (u16 *)fres_Alloc( sizeof(u16)*(LF_MESH_SIZE_VERT-1)*(LF_MESH_SIZE_HORIZ-1)*6 ); //u16 * (nQuad * 2) * 3 idx
	if (m_vMesh)
	{
		u32 n=0, i;
		f32 fY, fX, fZ, fdX, fdZ, fV;
		fdX = 1.0f / (f32)(LF_MESH_SIZE_VERT-1);
		fdZ = 1.0f / (f32)(LF_MESH_SIZE_HORIZ-1);

		for (fZ = 1.0f; fZ >= 0.0f; fZ -= fdZ)
		{
			for (fX = 1.0f; fX >= 0.0f; fX -= fdX)
			{
				fY = EvalFunc(fX);

				//now build a pos from this data.
				m_vMesh[n].vPos.x = fX;
				m_vMesh[n].vPos.y = fY;
				m_vMesh[n].vPos.z = fZ;

				if (m_nType == LT_MOLTEN)
				{
					if (fZ == 0.5f)
					{
						fV = powf(fX, 0.40f);
					}
					else
					{
						fV = powf(fX, 0.25f);
					}
					m_vMesh[n].vTex[0].x = fZ*2.0f;
					m_vMesh[n].vTex[0].y = fV*0.5f;

					m_vMesh[n].vTex[1].x = fZ;
					m_vMesh[n].vTex[1].y = fV;

					m_vMesh[n].vTex[2].x = fZ;
					m_vMesh[n].vTex[2].y = fV;
				}
				else
				{
					m_vMesh[n].vTex[0].x = fZ*2.0f;
					m_vMesh[n].vTex[0].y = fY*0.5f;

					m_vMesh[n].vTex[1].x = fZ;
					m_vMesh[n].vTex[1].y = fY;

					m_vMesh[n].vTex[2].x = fZ;
					m_vMesh[n].vTex[2].y = fY;
				}
			
				n++;
			}
		}

		//now pull back my vertices on the edges.
		for (i=0; i<n; i++)
		{
			if (m_vMesh[i].vPos.z == 0.0f)
			{
				m_vMesh[i].vPos.x += 0.20f;
				m_vMesh[i-LF_MESH_SIZE_VERT].vPos.z -= fdZ*0.80f;//0.20f;
			}
			else if (m_vMesh[i].vPos.z == 1.0f)
			{
				m_vMesh[i].vPos.x += 0.20f;
				m_vMesh[i+LF_MESH_SIZE_VERT].vPos.z += fdZ*0.80f;//0.20f;
			}
		}

		CFVec3A vPos;
		//now transform my vertices.
		for (i=0; i<n; i++)
		{
			m_vMesh[i].vPos.x =-(m_vMesh[i].vPos.x*2.0f - 1.0f) * m_vExt.x;
			m_vMesh[i].vPos.y = (m_vMesh[i].vPos.y*2.0f - 1.0f) * m_vExt.y;
			m_vMesh[i].vPos.z = (m_vMesh[i].vPos.z*2.0f - 1.0f) * m_vExt.z;

			vPos.Set(m_vMesh[i].vPos);
			vPos = m_Mtx.MulPoint(vPos);
			m_vMesh[i].vPos.Set(vPos.x, vPos.y, vPos.z);
		}
		//

		m_nVtx = n;

		s32 nX, nZ; 
		n = 0;
		for (nZ = 0; nZ < LF_MESH_SIZE_HORIZ-1; nZ++)
		{
			for (nX = 0; nX < LF_MESH_SIZE_VERT-1; nX++)
			{
				//tri0 = nX + (nZ)*(LF_MESH_SIZE_VERT-1), nX + (nZ+1)*(LF_MESH_SIZE_VERT-1), nX+1 + (nZ)*(LF_MESH_SIZE_VERT-1)
				m_pIdx[n++] = nX + (nZ)*(LF_MESH_SIZE_VERT);
				m_pIdx[n++] = nX + (nZ+1)*(LF_MESH_SIZE_VERT);
				m_pIdx[n++] = nX+1 + (nZ)*(LF_MESH_SIZE_VERT);

				//tri1 = nX + (nZ+1)*(LF_MESH_SIZE_VERT-1), nX+1 + (nZ+1)*(LF_MESH_SIZE_VERT-1), nX+1 + (nZ)*(LF_MESH_SIZE_VERT-1)
				m_pIdx[n++] = nX + (nZ+1)*(LF_MESH_SIZE_VERT);
				m_pIdx[n++] = nX+1 + (nZ+1)*(LF_MESH_SIZE_VERT);
				m_pIdx[n++] = nX+1 + (nZ)*(LF_MESH_SIZE_VERT);
			}
		}
		
		m_nIdx = n;
	}
	//
}

void CFLiquidMesh::SetSpeed(f32 fSpeed)
{
	m_fSpeed = fSpeed;
}

void CFLiquidMesh::Init()
{
}

void CFLiquidMesh::Work()
{
	if (!m_pLiquidVolume)
	{
		m_pLiquidVolume = LiquidSystem.SearchForLiquidVolume(this);
	}

	u32 i;
	f32 fU;

	if (m_nType == LT_MOLTEN)
	{
		fU = 0.15f;
		for (i=0; i<m_nVtx; i++)
		{
			m_vMesh[i].vTex[0].y += (m_fSpeed * FLoop_fPreviousLoopSecs*fU);
			m_vMesh[i].vTex[1].y += (m_fSpeed * FLoop_fPreviousLoopSecs*fU);
			m_vMesh[i].vTex[2].y += (m_fSpeed * FLoop_fPreviousLoopSecs*fU);
		}
	}
	else
	{
		for (i=0; i<m_nVtx; i++)
		{
			m_vMesh[i].vTex[0].y += m_fSpeed * FLoop_fPreviousLoopSecs;
			m_vMesh[i].vTex[1].y += m_fSpeed * FLoop_fPreviousLoopSecs;
			m_vMesh[i].vTex[2].y += m_fSpeed * FLoop_fPreviousLoopSecs;
		}
	}

	if (m_AnimTarget > m_fCurExp)
	{
		AnimateMesh(m_fCurExp+1.0f*FLoop_fPreviousLoopSecs, m_AnimTarget);
	}
	else
	{
		AnimateMesh(m_fCurExp-1.0f*FLoop_fPreviousLoopSecs, m_AnimTarget);
	}
}

void CFLiquidMesh::Render()
{
	u32 nType;
	CFTexInst *pEMBM=NULL;
	if (m_nType == LT_WATER) { nType = RP_REFLECT | RP_REFRACT; }
	else if (m_nType == LT_MOLTEN) { nType = RP_EMISSIVE; }
	else if (m_nType != LT_TEXTURE && m_nType != LT_OIL) { nType = RP_REFLECT; }
	else { nType = RP_TEXTURE; }

	if (m_pLiquidVolume)
	{
		pEMBM = m_pLiquidVolume->GetEMBM();
	}

	CFVec3 vFront;
	vFront.Set(-m_Mtx.m_vRight.x, 1.0f, -m_Mtx.m_vRight.z);

	fsh_DrawLiquidMesh(nType, m_nVtx, m_nIdx/3, m_vMesh, m_pIdx, &m_Clr, m_fOpacity, vFront, m_pTexInst[0], m_pTexInst[1], pEMBM);
}

//Liquid System
CFLiquidSystem::CFLiquidSystem( void )
{
	Reset();
}

void CFLiquidSystem::Reset()
{
	m_nNumLiquidVolume = m_nNumLiquidMesh = 0;
	_bInitStaticData=FALSE;
}

CFLiquidVolume *CFLiquidSystem::CreateLiquidVolume()
{
	u32 nVolume = m_nNumLiquidVolume;
	m_nNumLiquidVolume++;
	
	m_LiquidVolumes[nVolume] = fnew CFLiquidVolume;
	
	return m_LiquidVolumes[nVolume];
}

CFLiquidMesh *CFLiquidSystem::CreateLiquidMesh()
{
	u32 nMesh = m_nNumLiquidMesh;
	m_nNumLiquidMesh++;
	
	m_LiquidMeshs[nMesh] = fnew CFLiquidMesh;
	
	return m_LiquidMeshs[nMesh];
}

void CFLiquidSystem::ReleaseLiquidVolumeData( void ) 
{
	for( u32 i=0; i< m_nNumLiquidVolume; i++ ) 
	{
		if( m_LiquidVolumes[i] && m_LiquidVolumes[i]->m_pTracker[0] != NULL ) 
		{
			fdelete( m_LiquidVolumes[i]->m_pTracker[0] );
			m_LiquidVolumes[i]->m_pTracker[0] = NULL;
		}
		if( m_LiquidVolumes[i] && m_LiquidVolumes[i]->m_pTracker[1] != NULL ) 
		{
			fdelete( m_LiquidVolumes[i]->m_pTracker[1] );
			m_LiquidVolumes[i]->m_pTracker[1] = NULL;
		}
	}
}

int _QSortLiquidVolumeCallback( const void *pElement1, const void *pElement2 );

CFVec3A _vDir;
int _QSortLiquidVolumeCallback( const void *pElement1, const void *pElement2 )
{
	CFLiquidVolume *pV0, *pV1;
	f32 fMag2_0, fMag2_1;
	pV0 = *(CFLiquidVolume **)pElement1;
	pV1 = *(CFLiquidVolume **)pElement2;

	_vDir.x = pV0->m_vSurfaceCen.x - FXfm_pView->m_MtxR.m_vPos.x; 
	_vDir.y = pV0->m_vSurfaceCen.y - FXfm_pView->m_MtxR.m_vPos.y;
	_vDir.z = pV0->m_vSurfaceCen.z - FXfm_pView->m_MtxR.m_vPos.z;	
	
	fMag2_0 = _vDir.MagSq();
	
	_vDir.x = pV1->m_vSurfaceCen.x - FXfm_pView->m_MtxR.m_vPos.x; 
	_vDir.y = pV1->m_vSurfaceCen.y - FXfm_pView->m_MtxR.m_vPos.y;
	_vDir.z = pV1->m_vSurfaceCen.z - FXfm_pView->m_MtxR.m_vPos.z;	
	
	fMag2_1 = _vDir.MagSq();
	
	if (fMag2_0 < fMag2_1)
	{
		return -1;
	}
	else if (fMag2_0 > fMag2_1)
	{
		return +1;
	}
	return 0;
}

void CFLiquidSystem::Work()
{
	u32 i;
	CFVec3A vCamPos;
	BOOL bFullScrActive=FALSE;
	FViewport_t *pViewport;
	pViewport= fviewport_GetActive();

	for (i=0; i<LV_POOLSIZE; i++)
	{
		m_Active[i] = NULL;
	}
	
	if (pViewport == NULL || m_nNumLiquidVolume<1)
	{
		fsh_ActivateFullScrTargets(FALSE);
		return;	
	}
	
	vCamPos.Set(FXfm_pView->m_MtxR.m_vPos); 
	
	CFVec3A vDir;
	
	u32 nActive=0;
	BOOL bVisible;
	s32 nClipPlaneMask=FVIEWPORT_PLANESMASK_ALL;
	f32 fMag2;

	CFColorRGBA clr(1.0f, 0.5f, 0.5f, 1.0f);
	
	fclib_QSort( m_LiquidVolumes, m_nNumLiquidVolume, sizeof(CFLiquidVolume *), _QSortLiquidVolumeCallback );
	
	for (i=0; i<m_nNumLiquidVolume; i++)
	{
		if (!m_LiquidVolumes[i]->m_bProcedural)
		{
			m_LiquidVolumes[i]->m_bRender=FALSE;

			bVisible  = m_LiquidVolumes[i]->m_pTracker[0]->GetVolumeFlags() & FVIS_VOLUME_IN_VISIBLE_LIST;
			bVisible |= m_LiquidVolumes[i]->m_pTracker[1]->GetVolumeFlags() & FVIS_VOLUME_IN_VISIBLE_LIST;
			bVisible |= m_LiquidVolumes[i]->m_pTracker[2]->GetVolumeFlags() & FVIS_VOLUME_IN_VISIBLE_LIST;

			if ( bVisible && fviewport_TestSphere_WS(pViewport, &m_LiquidVolumes[i]->m_BoundingSphere, nClipPlaneMask) > -1 )
			{
		#if !FANG_PRODUCTION_BUILD && FLIQUID_DRAW_VIS_SPHERE
				if (m_LiquidVolumes[i]->m_pTracker[0]->GetVolumeFlags() & FVIS_VOLUME_IN_VISIBLE_LIST)
				{
					fdraw_DevSphere(&m_LiquidVolumes[i]->_Sphere[0].m_Pos, m_LiquidVolumes[i]->_Sphere[0].m_fRadius, &clr, 2, 2, 2);
				}
				if (m_LiquidVolumes[i]->m_pTracker[1]->GetVolumeFlags() & FVIS_VOLUME_IN_VISIBLE_LIST)
				{
					fdraw_DevSphere(&m_LiquidVolumes[i]->_Sphere[1].m_Pos, m_LiquidVolumes[i]->_Sphere[1].m_fRadius, &clr, 2, 2, 2);
				}
				if (m_LiquidVolumes[i]->m_pTracker[2]->GetVolumeFlags() & FVIS_VOLUME_IN_VISIBLE_LIST)
				{
					fdraw_DevSphere(&m_LiquidVolumes[i]->_Sphere[2].m_Pos, m_LiquidVolumes[i]->_Sphere[2].m_fRadius, &clr, 2, 2, 2);
				}
		#endif

				if (nActive < LV_POOLSIZE)
				{
					vDir.x = m_LiquidVolumes[i]->m_vSurfaceCen.x - vCamPos.x; 
					vDir.y = m_LiquidVolumes[i]->m_vSurfaceCen.y - vCamPos.y;
					vDir.z = m_LiquidVolumes[i]->m_vSurfaceCen.z - vCamPos.z;
					
					fMag2 = vDir.MagSq();
				
					if (fMag2 < m_LiquidVolumes[i]->m_fCullDist2+m_LiquidVolumes[i]->m_fRadius2)
					{
						m_Active[nActive++] = m_LiquidVolumes[i];
						fsh_ActivateRenderPlane( m_LiquidVolumes[i]->m_nRenderPlaneID, TRUE );
						if (m_LiquidVolumes[i]->GetType() == LT_WATER || m_LiquidVolumes[i]->GetType() == LT_MERCURY)
						{
							bFullScrActive = TRUE;					
						}
					}
					else
					{
						fsh_ActivateRenderPlane( m_LiquidVolumes[i]->m_nRenderPlaneID, FALSE );
					}
				}
				else
				{
					fsh_ActivateRenderPlane( m_LiquidVolumes[i]->m_nRenderPlaneID, FALSE );
				}
				
				m_LiquidVolumes[i]->m_bRender = TRUE;
			}
			else
			{
				fsh_ActivateRenderPlane( m_LiquidVolumes[i]->m_nRenderPlaneID, FALSE );
				m_LiquidVolumes[i]->m_bRender = FALSE;
			}
		}
		else
		{
			if ( m_LiquidVolumes[i]->m_pParentMesh->WasDrawnThisFrame() || m_LiquidVolumes[i]->m_pParentMesh->WasDrawnLastFrame() )
			{
				m_Active[nActive++] = m_LiquidVolumes[i];
				m_LiquidVolumes[i]->m_bRender = TRUE;
			}
		}
	}
	
	fsh_ActivateFullScrTargets(bFullScrActive);
		
	for (i=0; i<LV_POOLSIZE; i++)
	{
		if (m_Active[i] && (m_Active[i]->m_bRender == TRUE) )
		{
			if (m_LiquidVolumes[i]->m_nLastFrameWork != FVid_nFrameCounter)
			{
				m_LiquidVolumes[i]->m_nLastFrameWork = FVid_nFrameCounter;
				m_Active[i]->Work();
			}
		}
	}
	
	if (m_nNumLiquidMesh)
	{
		for (i=0; i<m_nNumLiquidMesh; i++)
		{
			m_LiquidMeshs[i]->Work();
		}
	}
}

void CFLiquidSystem::Render()
{
	u32 i;

	if (m_nNumLiquidVolume)
	{
		for (i=0; i<m_nNumLiquidVolume; i++)
		{
			if (m_LiquidVolumes[i]->m_bRender && !m_LiquidVolumes[i]->m_bProcedural)
			{
				m_LiquidVolumes[i]->Render();
			}
		}
	}

	if (m_nNumLiquidMesh)
	{
		for (i=0; i<m_nNumLiquidMesh; i++)
		{
			m_LiquidMeshs[i]->Render();
		}
	}
}

CFLiquidVolume *CFLiquidSystem::SearchForLiquidVolume(CFLiquidMesh *pMesh)
{
	CFLiquidVolume *pNearest=NULL;

	u32 i;
	for (i=0; i<m_nNumLiquidVolume; i++)
	{
		if ( m_LiquidVolumes[i]->GetType() == pMesh->GetType() )
		{
			pNearest = m_LiquidVolumes[i];
			break;
		}
	}

	return (pNearest);
}

void CFLiquidSystem::SpawnExplosion( CFVec3A& vPos, f32 fRadius )
{
	if (m_nNumLiquidVolume)
	{
		u32 i;
		f32 fSumR;
		CFVec3 vOffs, vP;
		
		vP.Set(vPos.x, vPos.y, vPos.z);

		for (i=0; i<m_nNumLiquidVolume; i++)
		{
			if (m_LiquidVolumes[i]->m_bRender && !m_LiquidVolumes[i]->m_bProcedural)
			{
				fSumR = fRadius + m_LiquidVolumes[i]->m_BoundingSphere.m_fRadius;
				vOffs = vP - m_LiquidVolumes[i]->m_BoundingSphere.m_Pos;
				//Gross collision check.
				if ( FMATH_FABS(vOffs.x) < fSumR || FMATH_FABS(vOffs.y) < fSumR || FMATH_FABS(vOffs.z) < fSumR )
				{
					//Specific collision check.
					m_LiquidVolumes[i]->CheckCollision(vP, fRadius);
				}
			}
		}
	}
}
