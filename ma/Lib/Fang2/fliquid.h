//////////////////////////////////////////////////////////////////////////////////////
// fliquid.h - Fang liquid system module.
//
// Author: Jeremy Chernobieff
//////////////////////////////////////////////////////////////////////////////////////
// THIS CODE IS PROPRIETARY PROPERTY OF SWINGIN' APE STUDIOS, INC.
// Copyright (c) 2000
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

#ifndef _FLIQUID_H_
#define _FLIQUID_H_ 1

#include "fang.h"
#include "fmath.h"
#include "fcolor.h"
#include "fparticle.h"


//
// Forward declarations
class CFWorldMesh;
class CFTexInst;
class CFWorldTracker;
class CFMeshInst;
struct FVisVolume_t;


//---------------------------------------------------------------------------------------------------------------------
// fliquid_ModuleStartup()
// fliquid_ModuleShutdown()
//
// Starts up and shuts down this module, respectively.
// Called by the Fang initialization system.
// Returns TRUE if successful, or FALSE if the module could not successfully initialized.
//
extern BOOL fliquid_ModuleStartup( void );
extern void fliquid_ModuleShutdown( void );

typedef BOOL FLiquidCollisionCallback_t( CFWorldMesh *pMesh, void *pUserData );

//set for collisions
extern FLiquidCollisionCallback_t *FLiquid_pCollisionCallback;

enum
{
	LV_POOLSIZE=4,
	LV_GRID_SIZE=64,

	LF_NUM_TEXCOORD=3,
	LF_MESH_SIZE_VERT=10, //(5-1)x(3-1)x2 = 8
	LF_MESH_SIZE_HORIZ=5,

	MAX_NUM_LAYERS=2,
	
	LSYS_MAX_VOLUMES=100
};

typedef enum
{
	LT_WATER=0, //reflective/refractive
	LT_MERCURY,	//reflective
	LT_MOLTEN,	//emissive, multi-layer
	LT_OIL,		//solid, specular
	LT_TEXTURE,	//warp an arbitrary texture.
	
	LT_NUM_TYPES
} LiquidType_e;

typedef struct
{
	CFVec3 vPos;
	CFVec2 vTex[ LF_NUM_TEXCOORD ];
} LiquidFallVtx;

FCLASS_ALIGN_PREFIX class CFLiquidVolume
{
	public:
	
		CFLiquidVolume( void );

		FINLINE void SetupProcedural(CFMeshInst *pMesh) { m_pParentMesh = pMesh; m_bProcedural = TRUE; }
		
		void SetLiquidType(LiquidType_e ltype);
		void SetLiquidFog(CFColorRGB& fogClr, f32 fDensity);
		void SetupVolume(CFVec3A& vExt, CFMtx43A& vOrient);
		
		void Init();
		
		void Work();
		void Render();
		
		FINLINE f32 GetHeight() { return (m_vSurface[0].y); }
		FINLINE LiquidType_e GetType() { return (m_nType); }
		
		BOOL Displace(CFVec3A& vPos, f32 fRadius, f32 fMag, BOOL bSet=FALSE, BOOL bInteract=TRUE);
		void Displace(u32 nX, u32 nY, u32 nRadius, f32 fMag, BOOL bSet=FALSE);
		void AddRainDrops(f32 fNumSec, f32 fMag) { m_fNumRainDrops_Sec = fNumSec; m_fRainDropMag = fMag; }
		void AddCurrent(CFVec3A& vDir, f32 fMag);
		void SetCullDist(f32 fCullDist) { m_fCullDist2 = fCullDist*fCullDist; }
		void SetGeoCullDist(f32 fCullDist) { m_fGeoDraw = fCullDist; }
		void SetTexLayers(CFTexInst *pLayer0, CFTexInst *pLayer1) { m_pTexInst[0] = pLayer0; m_pTexInst[1] = pLayer1; }
		void SetParticleHandle(FParticle_DefHandle_t hHandle, BOOL bUseParticles);
		void SetExplosionPartHandle(FParticle_DefHandle_t hHandle) { m_hExplosionParticleDef = hHandle; }
		void SetOpacity(f32 fOpacity) { m_fOpacity = fOpacity; }
		void SetTile(f32 fTile) { m_fTile = fTile; }

		void ChangeWaveSpeed(f32 fNewWaveSpd);

		FINLINE void SetBumpTile(f32 fBumpTile) 
		{ 
			m_fBumpTile = fBumpTile; 
			m_bInteract = (m_fBumpTile==1.0f)?(TRUE):(FALSE); 
		}

		void SetGlowScale(f32 fScale) { m_fGlowScale = fScale; }
		void SetCurrent(CFVec2& vCurrent);
		void HandleCollisions();

		void SetUserData(void *pUserData) { m_pUserData = pUserData; }

		BOOL CheckCollision(CFVec3& vPos, f32 fRadius);

		FINLINE CFTexInst *GetEMBM() 
		{ 
			if (m_pData) { return m_pData->m_pEMBM; }
			return (NULL);
		}
				
		static void InitStaticData();
		static void ReleaseTextures();
		void ReleaseTracker( void );
		
		CFLiquidVolume *m_pConnect;
		BOOL m_bRender;
		
		f32 m_fRadius, m_fRadius2;
		f32 m_fCullDist2, m_fGeoDraw;
		CFVec3A m_vSurfaceCen;
		
		u32 m_nRenderPlaneID;
		CFSphere		m_BoundingSphere;
		CFSphere		m_VolumeSphere;
		CFWorldTracker  *m_pTracker[3];
        
		BOOL m_bProcedural;
		CFMeshInst *m_pParentMesh;

		//Make sure we update this only once a frame!
		u32 m_nLastFrameWork;

		//debug
		CFSphere _Sphere[3];

	private:
	
		typedef struct
		{
			f32 *m_pZc, *m_pZn;
			//Liquid grid heights
			f32 m_aZ0[LV_GRID_SIZE*LV_GRID_SIZE];
			f32 m_aZ1[LV_GRID_SIZE*LV_GRID_SIZE];
			//Liquid Friction, 0 = solid - no move, 0xff = free movement.
			u8 m_aD[LV_GRID_SIZE*LV_GRID_SIZE];
			
			//Actual Surface
			CFTexInst *m_pEMBM;
		} FLiquid_Data_t;
	
		LiquidType_e m_nType;
		CFColorRGB m_fogClr;
		f32 m_fDensity;
		f32 m_fOpacity;
		f32 m_fTile;
		f32 m_fBumpTile;

		u8 m_anColList[ LV_GRID_SIZE*LV_GRID_SIZE ];
		u8 m_nFrame;
				
		CFVec3A m_vExt;
		CFMtx43A m_Mtx, m_MtxI;
				
		CFVec3 m_vSurface[4];
				
		//Sim Data
		f32 fH, fCdivH;
		f32 fC;
		
		f32 m_fNumRainDrops_Sec;
		f32 m_fRainDropMag;
		f32 m_fGlowScale;
		
		f32 m_fdT_Add;
		
		CFVec2 m_vCurrent;
		CFVec2 m_vScroll;
		//
		
		FLiquid_Data_t *m_pData;
		void *m_pUserData;
		BOOL m_bInit;
		BOOL m_bInteract;
		BOOL m_bUseParticles;
		
		CFTexInst *m_pTexInst[MAX_NUM_LAYERS];
		FParticle_DefHandle_t m_hCollisionParticleDef;
		FParticle_DefHandle_t m_hExplosionParticleDef;

		void InitData();
		
		//Forces
		void DoRain(f32 fdT);
		void DoCurrent(f32 fdT);
		void DoWaves();
		void RestoreDrag();
		BOOL InCurCollisionList(f32 fX, f32 fZ, u32 nFrame);

	protected:
	
		static FLiquid_Data_t *m_DataPool;//[LV_POOLSIZE];

		static BOOL CollisionCallback( CFWorldTracker *pTracker, FVisVolume_t *pVolume );
		static CFLiquidVolume *m_pSelf;
		static f32 m_fPScale;
		static f32 m_fMinPScale;
	
	FCLASS_STACKMEM_ALIGN( CFLiquidVolume );
} FCLASS_ALIGN_SUFFIX;

//used for water falls and similar things.
FCLASS_ALIGN_PREFIX class CFLiquidMesh
{
	public:
	
		CFLiquidMesh( void );

		void SetLiquidType(LiquidType_e ltype);
		void SetupMesh(CFVec3A& vExt, CFMtx43A& vOrient, f32 fExp);
		void SetTexLayers(CFTexInst *pLayer0, CFTexInst *pLayer1) { m_pTexInst[0] = pLayer0; m_pTexInst[1] = pLayer1; }
		void SetOpacity(f32 fOpacity) { m_fOpacity = fOpacity; }
		void SetSpeed(f32 fSpeed);

		void AnimateMesh(f32 fExp, f32 fNextExp);

		void Init();

		void Work();
		void Render();

		FINLINE LiquidType_e GetType() { return m_nType; }

		FINLINE f32 EvalFunc(f32 fX);
		FINLINE f32 EvalFunc(f32 fX, f32 fExp);
	
	private:

		LiquidType_e m_nType;
		CFColorRGB m_Clr;
		f32 m_fExp, m_fNextExp, m_fCurExp, m_AnimTarget; //curvature function Fc(x) = 1 - (x-1)^m_fExp, where x = [0, 1 gd]

		CFVec3A m_vExt;
		CFMtx43A m_Mtx, m_MtxI;

		CFTexInst *m_pTexInst[MAX_NUM_LAYERS];

		u16 m_nVtx, m_nIdx;
		f32 m_fSpeed;
		f32 m_fOpacity;

		LiquidFallVtx *m_vMesh;
		u16 *m_pIdx;

		CFLiquidVolume *m_pLiquidVolume;
			
	protected:
	
	FCLASS_STACKMEM_ALIGN( CFLiquidMesh );
} FCLASS_ALIGN_SUFFIX;

//manager for all liquid volumes and meshs.
FCLASS_ALIGN_PREFIX class CFLiquidSystem
{
	public:
	
		CFLiquidSystem( void );
		
		void Reset();
		CFLiquidVolume *CreateLiquidVolume();
		CFLiquidMesh *CreateLiquidMesh();
		
		void Work();
		void Render();

		CFLiquidVolume *SearchForLiquidVolume(CFLiquidMesh *pMesh);

		void ReleaseLiquidVolumeData( void );
		void SpawnExplosion( CFVec3A& vPos, f32 fRadius );
		
		u32 m_nNumLiquidVolume, m_nNumLiquidMesh;
	
	private:
	
		CFLiquidVolume *m_LiquidVolumes[LSYS_MAX_VOLUMES];
		CFLiquidMesh *m_LiquidMeshs[LSYS_MAX_VOLUMES];

		CFLiquidVolume *m_Active[LV_POOLSIZE];
		
		
	
	protected:
	
	FCLASS_STACKMEM_ALIGN( CFLiquidSystem );
} FCLASS_ALIGN_SUFFIX;

extern CFLiquidSystem LiquidSystem;


#endif