//////////////////////////////////////////////////////////////////////////////////////
// MLMaterial.h - Classes used to convert generic mesh data into Fang platform specific data
//
// Author: John Lafleur
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
// 03/17/02 Lafleur		Created as GCMaterial.
// 08/15/02 Lafleur		Genericized from GC code to support multi-platform.  Changed name to MLMaterial
//////////////////////////////////////////////////////////////////////////////////////

#ifndef __MLMATERIAL_H_
#define __MLMATERIAL_H_

#include "fang.h"
#include "fshaders.h"

#include "MLSegment.h"

// Forward class declarations
class MLMaterial;
class MLMesh;
struct PrimitiveGroup;
struct KongLayer_t;


struct MLDLContainer;
class MLTriContainer;

//
//
enum
{
	ML_MAT_SHREG_MOTIF_DIFFUSE = 0,
	ML_MAT_SHREG_MOTIF_SPECULAR,
	ML_MAT_SHREG_MOTIF_EMISSIVE,
	ML_MAT_SHREG_MOTIF_REFLECTIVE,
	ML_MAT_SHREG_MOTIF_ADDITIVE,
	ML_MAT_SHREG_TEX_BASE,
	ML_MAT_SHREG_TEX_LAYER1,
	ML_MAT_SHREG_TEX_SHADOW,
	ML_MAT_SHREG_STIDX_BASE,
	ML_MAT_SHREG_STIDX_LAYER1,
	ML_MAT_SHREG_STIDX_SHADOW,
	ML_MAT_SHREG_SPEC_EXPONENT,
	ML_MAT_SHREG_MAX
};

/*
//
//
struct LinkNode
{
	MLDLContainer	*pOwner;
	MLTriContainer	*pTris;
	LinkNode		*pNext;
};
*/

//
//
struct MLDLContainer
{
	u32				nVBKey;
	BOOL			bUseConstantColor;
	BOOL			bFacingDirLight;
	CFColorRGBA		ConstantColor;

	u16				nLODID;
	u16				nPartID;

	u16				nMatrixIdx;
	u16				nVertexCount;
	u16				nMatrixCount;
	u16				nPrimGroupCount;	// Number of Render primitives set up by the NVidia triangle stripper
	u16				nStripGroupCount;
	u32				nTotalVertIndices;
//	u16				nDrawLinkCount;

	MLTriContainer	*pTris;				

	u16				*paAbstrIndices;	// Triangle abstractions, indices into which are used by the triangle stripper

	PrimitiveGroup	*paPrimGroups;		// Pointer to an array of render primitives set up by the NVidia triangle stripper

	MLDLContainer	*pNextDLContainer;
};


//
//
//
class MLMaterial
{
	public:
		FMeshMaterial_t	m_FMaterial;

		u32				m_nTotalMaterialVerts;
		u32				m_nTempMeshIdx;			// This is used to setup a temporary mesh idx for use when exporting meshes (may be changed by other meshes when supporting multi-mesh export)

		u32				m_nStripCount;
		u32				m_nStripTriCount;
		u32				m_nListTriCount;

		CFVec3			m_vVertNormalTotal;

		KongMat_t		*m_pKongMat;

		// Rendering/Display List data
		u32				m_nDLContainerCount;
		MLDLContainer	*m_pFirstDLContainer;
		MLDLContainer	*m_pLastDLContainer;

		BOOL			m_bEmissiveOn;
		BOOL			m_bSpecularOn;

		BOOL			m_bEnviroMotifOn;

		s8				m_nLayersUsed;
		s8				m_nTexturesUsed;
		s8				m_nLightMapCount;

		s8				m_nSpecularMask;		// -1 = no specular mask
		s8				m_nEmissiveMask;		// -1 = no emissive mask
		s8				m_nTransparencyMask;	// -1 = no transparency mask
		s8				m_nEnvironmentMapTexture;// -1 = no environment map texture
		s8				m_nBumpMapTexture;		// -1 = no bump map texture
		s8				m_nDetailTexture;
		s8				m_nBaseTexture[FSH_MAX_SURFACEMAPS];

		// Link to the next material - used by the segment
		MLMaterial		*m_pNext;

		u16				m_nBaseSTSets;
		u16				m_nLightMapSTSets;

		// Shader Values:
		s32				m_nLightShaderID;
		s32				m_nSurfaceShaderID;
		s32				m_nSpecularShaderID;

		u32				m_nADDR_TexInst[FSH_MAX_SURFACEMAPS + FSH_MAX_LIGHTMAPS]; // The address when exporting
		char			m_szTexture[FSH_MAX_SURFACEMAPS + FSH_MAX_LIGHTMAPS][FDATA_TEXNAME_LEN + 1];
		FShTexInst_t	m_TexInst[FSH_MAX_SURFACEMAPS + FSH_MAX_LIGHTMAPS];

		u32				m_nSTIndexLayer[FSH_MAX_SURFACEMAPS + FSH_MAX_LIGHTMAPS];

		u32				m_nLightMapMotifID[FSH_MAX_LIGHTMAPS];

		CFColorMotif	m_MotifEmissive;
		CFColorMotif	m_MotifDiffuse;
		CFColorMotif	m_MotifSpecular;
		CFColorMotif	m_MotifEnvironmental;

		f32				m_fSpecularExponent;

	public:
		MLMaterial( void );
		~MLMaterial( void );
		virtual BOOL Set( MLMesh *pMesh, KongMat_t *pMaterial );
		virtual u32  AppendTris( MLTriContainer *pContainer );
		virtual BOOL BuildPrimitiveGroups( u32 nMinTrisInStrip, u32 nVertCacheSize, BOOL bCreateStrips );

		void CalculateBoundingSphere( const CFSphere *pMeshBoundingSphere );
		void AddTexLayerIDIndexToMaterial( u32 nTexLayerIndex );
		BOOL AddLayer( MLMesh *pMesh, u32 nLayer, const KongLayer_t *pLayer );
		BOOL AddShaderMSRCount( u32 *pnMotifCounter, u32 *pnShTexInstCounter, u32 *pnRegisterCounter );
		BOOL CopyTextureData( u8 **pShTexInsts, u8 **pShTexNames, u8 *pExportBase, BOOL bChangeEndian );
		BOOL CopySurfaceShaderMSR( u32 **pRegisters, u8 **pMotifs, u8 *pExportBase, BOOL bChangeEndian );
		BOOL CopyLightShaderMSR( u32 **pRegisters, u8 **pMotifs, u8 *pExportBase, BOOL bChangeEndian );
		void SetShaderTypes( KongMat_t *pKongMat );
		void SetSurfaceShaderIdx( u32 nSetting );

		//
		//
		BOOL ResetFlags( void )
		{
			m_FMaterial.nMtlFlags = 0;
			return TRUE;
		}

		//
		//
		BOOL AddFlags( u32 nFlags )
		{
			m_FMaterial.nMtlFlags |= (nFlags & 0xffff);
			return TRUE;
		}

		//
		//
		u32 GetFlags( void )
		{
			return m_FMaterial.nMtlFlags;
		}

		//
		// 0=normal, 1=appear in front of 0, 2=appear in front of 1, etc. (negative values not allowed)
		BOOL SetDepthBiasLevel( u32 nSetting )
		{
			m_FMaterial.nDepthBiasLevel = (u8)nSetting;
			return TRUE;
		}
		
		//
		//
		void SetLightShaderIdx( u32 nSetting )
		{
			m_FMaterial.nLightShaderIdx = (u8)nSetting;
			m_nLightShaderID = (u16)nSetting;
		}

		//
		//
		void SetSpecularShaderIdx( u32 nSetting )
		{
			m_FMaterial.nSpecularShaderIdx = (u8)nSetting;
			m_nSpecularShaderID = (u16)nSetting;
		}

	protected:
		virtual BOOL BuildDLAbstractionIndices( MLDLContainer *pDL, u32 &nIndexCount ){ return FALSE; };
};


#endif