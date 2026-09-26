#include "gcmesh.h"

#include "fdx8load.h"
#include "fdx8mesh.h"
#include "fdx8vb.h"
#include "fmesh.h"
#include "fmesh_coll.h"
#include "FkDOP.h"
#include "fsh.h"
#include "fshaders.h"
#include "fres.h"
#include "fvis.h"
#include "gc/fGCmesh.h"
#include "gc/fGCdisplaylist.h"

#include <math.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <vector>

namespace
{
struct DLPlan_t
{
	u32 nMaterialIndex;
	u32 nDLIndex;
	u32 nVertexCount;
	u32 nIndexCount;
	u32 nIndexOffset;
	u32 nVertexOffset;
	u32 nLightMapOffset;
	u32 nBasisOffset;
	s32 nVertexFormat;
	u32 nSegmentIdx;
	u8 anSegBones[FDATA_VW_COUNT_PER_VTX]; // Bone order of the bound segment (weighted formats only)
	u32 nSegBoneCount;
	BOOL bWeighted;
};

// Per-vertex skin weights decoded from the GC FGCMeshSkin_t transform descriptors
// (see _TransformSkinnedVerts() in gc/fGCmesh.cpp).
struct SkinVert_t
{
	u8 nCount;
	u8 anBone[FDATA_VW_COUNT_PER_VTX];
	f32 afWeight[FDATA_VW_COUNT_PER_VTX];
};

struct SkinInfo_t
{
	std::vector<SkinVert_t> aVerts;
};

// Source line of the most recent conversion failure, for diagnostics.
static u32 _nFailLine;
#define GCM_FAIL()              \
	do                          \
	{                           \
		_nFailLine = __LINE__;  \
		return FALSE;           \
	} while (0)

struct AttrIndex_t
{
	u16 nPosition;
	u16 nNormal;
	u16 nBinormal; // NBT3 only
	u16 nTangent;  // NBT3 only
	u16 nDiffuse;
	u16 anST[FGCVB_MAX_ST_SETS];
};

struct ParseStats_t
{
	u32 nVertexCount;
	u32 nIndexCount;
	BOOL bSkinned;
	f32 afBoneWeight[256]; // Skinned only: accumulated weight per bone over the display list
};

struct WriteContext_t
{
	const u8 *pFile;
	u32 nFileBytes;
	const FGCVB_t *pVB;
	const FMeshMaterial_t *pMaterial;
	const FGC_DLCont_t *pDL;
	u8 *pVertexData;
	u8 *pLightMapData;
	u8 *pBasisData;
	u16 *pIndices;
	u32 nWrittenVertices;
	u32 nWrittenIndices;
	u32 nSTCount;
	BOOL bWeighted;
	const u8 *pSegBones;
	u32 nSegBoneCount;
};

static u16 _ReadBE16(const u8 *pData)
{
	return (u16)(((u16)pData[0] << 8) | pData[1]);
}

static u32 _ReadBE32(const u8 *pData)
{
	return ((u32)pData[0] << 24) | ((u32)pData[1] << 16) | ((u32)pData[2] << 8) | pData[3];
}

static BOOL _Range(u32 nOffset, u32 nBytes, u32 nTotalBytes)
{
	return nOffset <= nTotalBytes && nBytes <= nTotalBytes - nOffset;
}

static BOOL _RangeArray(u32 nOffset, u32 nCount, u32 nStride, u32 nTotalBytes)
{
	if (nCount && !nOffset)
		return FALSE;
	if (nStride && nCount > 0xffffffffu / nStride)
		return FALSE;
	return _Range(nOffset, nCount * nStride, nTotalBytes);
}

static u32 _Offset(const void *pOffset)
{
	return (u32)(uintptr_t)pOffset;
}

static void *_At(void *pData, u32 nOffset)
{
	return nOffset ? (u8 *)pData + nOffset : NULL;
}

static const void *_At(const void *pData, u32 nOffset)
{
	return nOffset ? (const u8 *)pData + nOffset : NULL;
}

static BOOL _Align4Checked(u32 nValue, u32 *pnAligned)
{
	if (nValue > 0xfffffffcu || !pnAligned)
		return FALSE;
	*pnAligned = (nValue + 3u) & ~3u;
	return TRUE;
}

static BOOL _TakeBytes(u32 *pnCurrent, u32 nCount, u32 nStride, u32 *pnOffset)
{
	u32 nAligned;
	if (!pnCurrent || !pnOffset || !_Align4Checked(*pnCurrent, &nAligned) ||
		(nStride && nCount > (0xffffffffu - nAligned) / nStride))
		return FALSE;
	*pnCurrent = nAligned;
	*pnOffset = nCount ? *pnCurrent : 0;
	*pnCurrent += nCount * nStride;
	return TRUE;
}

static BOOL _ReadAttrIndex(const u8 **ppCursor, const u8 *pEnd, u8 nIndexType, u16 *pnIndex)
{
	if (nIndexType == GX_INDEX8)
	{
		if (*ppCursor >= pEnd)
			return FALSE;
		*pnIndex = *(*ppCursor)++;
		return TRUE;
	}
	if (nIndexType == GX_INDEX16)
	{
		if ((u32)(pEnd - *ppCursor) < 2)
			return FALSE;
		*pnIndex = _ReadBE16(*ppCursor);
		*ppCursor += 2;
		return TRUE;
	}
	return FALSE;
}

// Decodes the (still big-endian) FGCMeshSkin_t at nSkinOffset. The GC layout is fixed:
// u16 nTransDescCount, nTD1MtxCount, nTD2MtxCount, nTD3or4MtxCount; u32 pTransDesc, nSkinnedVerts,
// pSkinnedVerts, pSkinWeights. Transform descriptors are 8 bytes: u8 nMatrixCount, pad, u16 nVertCount,
// u8 nMtxIdx[4]. Verts are stored in descriptor order; 1-matrix verts have no weight entry, every
// other vert consumes one u8[4] weight entry (w/255), in order.
static BOOL _BuildSkinInfo(const u8 *pFile, u32 nFileBytes, u32 nSkinOffset, u32 nBoneCount, SkinInfo_t *pSkin)
{
	if (!nSkinOffset || !_Range(nSkinOffset, 24, nFileBytes))
		GCM_FAIL();
	const u8 *pHeader = pFile + nSkinOffset;
	const u32 nTransDescCount = _ReadBE16(pHeader + 0);
	const u32 nTD1 = _ReadBE16(pHeader + 2);
	const u32 nTD2 = _ReadBE16(pHeader + 4);
	const u32 nTD34 = _ReadBE16(pHeader + 6);
	const u32 nTransDescOffset = _ReadBE32(pHeader + 8);
	const u32 nSkinnedVerts = _ReadBE32(pHeader + 12);
	const u32 nWeightsOffset = _ReadBE32(pHeader + 20);
	if (nTD1 + nTD2 + nTD34 != nTransDescCount || nSkinnedVerts > 65536 ||
		!_RangeArray(nTransDescOffset, nTransDescCount, 8, nFileBytes))
		GCM_FAIL();

	pSkin->aVerts.clear();
	pSkin->aVerts.resize(nSkinnedVerts);
	u32 nVert = 0;
	u32 nWeight = 0;
	for (u32 nTD = 0; nTD < nTransDescCount; nTD++)
	{
		const u8 *pTD = pFile + nTransDescOffset + nTD * 8;
		const u32 nMatrixCount = nTD < nTD1 ? 1 : pTD[0];
		const u32 nVertCount = _ReadBE16(pTD + 2);
		if (!nMatrixCount || nMatrixCount > FDATA_VW_COUNT_PER_VTX || nVertCount > nSkinnedVerts - nVert)
			GCM_FAIL();
		for (u32 k = 0; k < nMatrixCount; k++)
			if (pTD[4 + k] >= nBoneCount)
				GCM_FAIL();
		for (u32 i = 0; i < nVertCount; i++, nVert++)
		{
			SkinVert_t &Vert = pSkin->aVerts[nVert];
			memset(&Vert, 0, sizeof(Vert));
			Vert.nCount = (u8)nMatrixCount;
			for (u32 k = 0; k < nMatrixCount; k++)
				Vert.anBone[k] = pTD[4 + k];
			if (nTD < nTD1)
			{
				Vert.afWeight[0] = 1.0f;
				continue;
			}
			if (!_RangeArray(nWeightsOffset, nWeight + 1, 4, nFileBytes))
				GCM_FAIL();
			const u8 *pW = pFile + nWeightsOffset + nWeight * 4;
			nWeight++;
			f32 fSum = 0.0f;
			for (u32 k = 0; k < nMatrixCount; k++)
				fSum += Vert.afWeight[k] = (f32)pW[k] * (1.0f / 255.0f);
			if (fSum <= 0.0f)
			{
				Vert.afWeight[0] = 1.0f;
				fSum = 1.0f;
			}
			for (u32 k = 0; k < nMatrixCount; k++)
				Vert.afWeight[k] /= fSum;
		}
	}
	// Any verts not covered by a descriptor keep nCount == 0 and are rejected if referenced.
	return TRUE;
}

// Expresses a skinned vert's weights in the bone order of the bound DX segment. The DX blend shaders
// read three weights and derive the fourth as 1-(w0+w1+w2), so for segments with fewer than four
// bones the last used weight is computed by subtraction to keep the unused slots at zero.
static void _ComputeSegWeights(const SkinVert_t &Vert, const u8 *pSegBones, u32 nSegBoneCount, f32 afOut[3])
{
	f32 afW[FDATA_VW_COUNT_PER_VTX] = {};
	f32 fSum = 0.0f;
	for (u32 s = 0; s < nSegBoneCount; s++)
	{
		for (u32 k = 0; k < Vert.nCount; k++)
		{
			if (Vert.anBone[k] == pSegBones[s])
			{
				afW[s] += Vert.afWeight[k];
				break;
			}
		}
		// A bone repeated in the segment only takes the weight once.
		for (u32 p = 0; p < s; p++)
			if (pSegBones[p] == pSegBones[s])
				afW[s] = 0.0f;
		fSum += afW[s];
	}
	if (fSum <= 0.0f)
	{
		// None of this vert's bones are in the segment (only possible for the >4 bone fallback).
		memset(afW, 0, sizeof(afW));
		afW[0] = 1.0f;
		fSum = 1.0f;
	}
	for (u32 s = 0; s < nSegBoneCount; s++)
		afW[s] /= fSum;
	if (nSegBoneCount >= 1 && nSegBoneCount < 4)
	{
		f32 fOthers = 0.0f;
		for (u32 s = 0; s + 1 < nSegBoneCount; s++)
			fOthers += afW[s];
		afW[nSegBoneCount - 1] = 1.0f - fOthers;
	}
	afOut[0] = afW[0];
	afOut[1] = afW[1];
	afOut[2] = afW[2];
}

static BOOL _WalkDL(const u8 *pFile, u32 nFileBytes, const FGCVB_t *pVB, const FMeshMaterial_t *pMaterial,
					const FGC_DLCont_t *pDL, const SkinInfo_t *pSkin, ParseStats_t *pStats, WriteContext_t *pWrite)
{
	const u32 nBufferOffset = _Offset(pDL->pBuffer);
	const u8 *pCommandData;
	if (pDL->nFlags & FGCDL_FLAGS_STREAMING)
	{
		pCommandData = (const u8 *)fvis_GetWorldStreamingData(nBufferOffset, pDL->nSize);
		if (!pCommandData)
			GCM_FAIL();
	}
	else
	{
		if (!nBufferOffset || !_Range(nBufferOffset, pDL->nSize, nFileBytes))
			GCM_FAIL();
		pCommandData = pFile + nBufferOffset;
	}
	// Skinned display lists index the mesh's FGCSkinPosNorm_t array (model-space s16 position with 6
	// fractional bits, s16 normal with 14) for both position and normal; the GC transforms that array
	// on the CPU every frame. Here the verts keep their model-space values and carry blend weights.
	const BOOL bSkinned = (pDL->nFlags & FGCDL_FLAGS_SKINNED) || (pVB->nFlags & FGCVB_SKINNED);
	if (bSkinned && (!pSkin || pVB->nPosType != GX_S16 || pVB->nPosStride != sizeof(FGCSkinPosNorm_t) ||
					 pVB->nPosCount > pSkin->aVerts.size()))
		GCM_FAIL();
	if (pStats)
		pStats->bSkinned = bSkinned;
	// Bump-mapped display lists without an NBT array (PASM writes these for streaming world geometry)
	// use GX_NRM_NBT3: separate normal, binormal and tangent indices into the normal sphere.
	const BOOL bNBT3 = !bSkinned && (pDL->nFlags & FGCDL_FLAGS_BUMPMAP) && !pVB->pNBT;
	if (pMaterial->nBaseSTSets > 2 || pMaterial->nLightMapSTSets > FGCVB_MAX_ST_SETS ||
		pMaterial->nBaseSTSets + pMaterial->nLightMapSTSets > FGCVB_MAX_ST_SETS)
		GCM_FAIL();
	if (pVB->nPosIdxType != GX_INDEX8 && pVB->nPosIdxType != GX_INDEX16)
		GCM_FAIL();
	if (!(pDL->nFlags & FGCDL_FLAGS_CONSTANT_COLOR) && pVB->nColorIdxType != GX_INDEX8 &&
		pVB->nColorIdxType != GX_INDEX16)
		GCM_FAIL();
	if (pVB->nPosType != GX_S8 && pVB->nPosType != GX_S16 && pVB->nPosType != GX_F32)
		GCM_FAIL();
	if (pVB->nPosFrac > 31 || !_RangeArray(_Offset(pVB->pPosition), pVB->nPosCount, pVB->nPosStride, nFileBytes))
		GCM_FAIL();
	if (!(pDL->nFlags & FGCDL_FLAGS_CONSTANT_COLOR) &&
		!_RangeArray(_Offset(pVB->pDiffuse), pVB->nDiffuseCount, sizeof(FGCColor_t), nFileBytes))
		GCM_FAIL();
	if (!pVB->pPosition || !pVB->pST || (!(pDL->nFlags & FGCDL_FLAGS_CONSTANT_COLOR) && !pVB->pDiffuse))
		GCM_FAIL();

	const u8 *pCursor = pCommandData;
	const u8 *pEnd = pCursor + pDL->nSize;
	const u32 nPrimitiveCount = (u32)pDL->nStripCount + pDL->nListCount;
	const u32 nSTCount = (u32)pMaterial->nBaseSTSets + pMaterial->nLightMapSTSets;
	for (u32 nPrimitive = 0; nPrimitive < nPrimitiveCount; nPrimitive++)
	{
		if ((u32)(pEnd - pCursor) < 3)
			GCM_FAIL();
		const u8 nCommand = *pCursor++;
		const u8 nPrimitiveType = nCommand & 0xf8;
		if (nPrimitiveType != 0x90 && nPrimitiveType != 0x98)
			GCM_FAIL();
		const BOOL bStrip = nPrimitiveType == 0x98;
		const u32 nVertexCount = _ReadBE16(pCursor);
		pCursor += 2;
		if (nVertexCount < 3 || (!bStrip && (nVertexCount % 3)))
			GCM_FAIL();
		if ((u32)(pEnd - pCursor) < nVertexCount)
			GCM_FAIL();

		std::vector<u16> anCommandRows;
		if (pWrite)
			anCommandRows.reserve(nVertexCount);
		for (u32 nVertex = 0; nVertex < nVertexCount; nVertex++)
		{
			AttrIndex_t Attr;
			memset(&Attr, 0, sizeof(Attr));
			if (!_ReadAttrIndex(&pCursor, pEnd, pVB->nPosIdxType, &Attr.nPosition) ||
				(u32)Attr.nPosition >= pVB->nPosCount || (u32)(pEnd - pCursor) < 2)
				GCM_FAIL();
			Attr.nNormal = _ReadBE16(pCursor);
			pCursor += 2;
			if (bNBT3)
			{
				if ((u32)(pEnd - pCursor) < 4)
					GCM_FAIL();
				Attr.nBinormal = _ReadBE16(pCursor);
				Attr.nTangent = _ReadBE16(pCursor + 2);
				pCursor += 4;
			}
			if (bSkinned)
			{
				if ((u32)Attr.nNormal >= pVB->nPosCount || !pSkin->aVerts[Attr.nPosition].nCount)
					GCM_FAIL();
				if (pStats)
				{
					const SkinVert_t &SV = pSkin->aVerts[Attr.nPosition];
					for (u32 k = 0; k < SV.nCount; k++)
						pStats->afBoneWeight[SV.anBone[k]] += SV.afWeight[k];
				}
			}

			if (!(pDL->nFlags & FGCDL_FLAGS_CONSTANT_COLOR))
			{
				if (!_ReadAttrIndex(&pCursor, pEnd, pVB->nColorIdxType, &Attr.nDiffuse) ||
					(u32)Attr.nDiffuse >= pVB->nDiffuseCount)
					GCM_FAIL();
			}

			for (u32 nST = 0; nST < nSTCount; nST++)
			{
				if ((u32)(pEnd - pCursor) < 2)
					GCM_FAIL();
				Attr.anST[nST] = _ReadBE16(pCursor);
				pCursor += 2;
				if (!_RangeArray(_Offset(pVB->pST), (u32)Attr.anST[nST] + 1, sizeof(FGCST16_t), nFileBytes))
					GCM_FAIL();
			}

			if (pWrite)
			{
				if (pWrite->nWrittenVertices >= 65535)
					GCM_FAIL();
				const u16 nRow = (u16)pWrite->nWrittenVertices++;
				anCommandRows.push_back(nRow);
				const u32 nPositionOffset = _Offset(pVB->pPosition) + (u32)Attr.nPosition * pVB->nPosStride;
				if (!_Range(nPositionOffset, pVB->nPosStride, nFileBytes))
					GCM_FAIL();
				const u8 *pPosition = (const u8 *)_At(pFile, nPositionOffset);

				f32 fPosition[3] = {0.0f, 0.0f, 0.0f};
				// Skinned verts are always packed at 1/64 (see AddSkinnedApeVert() in PASM's MLMesh_GC.cpp).
				const f32 fPosScale = bSkinned ? (1.0f / 64.0f) : (f32)ldexp(1.0, -(int)pVB->nPosFrac);
				if (pVB->nPosType == GX_S8)
				{
					if (pVB->nPosStride < 3)
						GCM_FAIL();
					for (u32 k = 0; k < 3; k++)
						fPosition[k] = (f32)(s8)pPosition[k] * fPosScale;
				}
				else if (pVB->nPosType == GX_S16)
				{
					if (pVB->nPosStride < 6)
						GCM_FAIL();
					for (u32 k = 0; k < 3; k++)
						fPosition[k] = (f32)(s16)_ReadBE16(pPosition + k * 2) * fPosScale;
				}
				else
				{
					if (pVB->nPosStride < 12)
						GCM_FAIL();
					for (u32 k = 0; k < 3; k++)
					{
						const u32 nBits = _ReadBE32(pPosition + k * 4);
						memcpy(&fPosition[k], &nBits, sizeof(f32));
						if (!isfinite(fPosition[k]))
							GCM_FAIL();
					}
				}

				f32 fNormal[3] = {0.0f, 1.0f, 0.0f};
				f32 fTangent[3] = {1.0f, 0.0f, 0.0f};
				f32 fBinormal[3] = {0.0f, 0.0f, 1.0f};
				if (bSkinned)
				{
					const u8 *pSkinNormal =
						pFile + _Offset(pVB->pPosition) + (u32)Attr.nNormal * sizeof(FGCSkinPosNorm_t) + 6;
					for (u32 k = 0; k < 3; k++)
						fNormal[k] = (f32)(s16)_ReadBE16(pSkinNormal + k * 2) * (1.0f / 16384.0f);
				}
				else if (bNBT3)
				{
					const u16 anSphere[3] = {Attr.nNormal, Attr.nBinormal, Attr.nTangent};
					f32 *apfOut[3] = {fNormal, fBinormal, fTangent};
					for (u32 v = 0; v < 3; v++)
					{
						if (!FMesh_avCNormalSphere || anSphere[v] >= FMESH_NORMAL_SPHERE_MAX_INDEX)
							continue;
						apfOut[v][0] = (f32)FMesh_avCNormalSphere[anSphere[v]].nx * (1.0f / 64.0f);
						apfOut[v][1] = (f32)FMesh_avCNormalSphere[anSphere[v]].ny * (1.0f / 64.0f);
						apfOut[v][2] = (f32)FMesh_avCNormalSphere[anSphere[v]].nz * (1.0f / 64.0f);
					}
				}
				else if (pDL->nFlags & FGCDL_FLAGS_BUMPMAP)
				{
					if (!pVB->pNBT ||
						!_RangeArray(_Offset(pVB->pNBT), (u32)Attr.nNormal + 1, sizeof(FGCNBT8_t), nFileBytes))
						GCM_FAIL();
					const u32 nNBTOffset = _Offset(pVB->pNBT) + (u32)Attr.nNormal * sizeof(FGCNBT8_t);
					const u8 *pNBT = (const u8 *)_At(pFile, nNBTOffset);
					for (u32 k = 0; k < 3; k++)
						fNormal[k] = (f32)(s8)pNBT[k] * (1.0f / 64.0f);
					for (u32 k = 0; k < 3; k++)
						fBinormal[k] = (f32)(s8)pNBT[3 + k] * (1.0f / 64.0f);
					for (u32 k = 0; k < 3; k++)
						fTangent[k] = (f32)(s8)pNBT[6 + k] * (1.0f / 64.0f);
				}
				else if (FMesh_avCNormalSphere && Attr.nNormal < FMESH_NORMAL_SPHERE_MAX_INDEX)
				{
					fNormal[0] = (f32)FMesh_avCNormalSphere[Attr.nNormal].nx * (1.0f / 64.0f);
					fNormal[1] = (f32)FMesh_avCNormalSphere[Attr.nNormal].ny * (1.0f / 64.0f);
					fNormal[2] = (f32)FMesh_avCNormalSphere[Attr.nNormal].nz * (1.0f / 64.0f);
				}

				u32 nColor = 0xffffffffu;
				if (pDL->nFlags & FGCDL_FLAGS_CONSTANT_COLOR)
				{
					nColor = ((u32)pDL->ConstantColor.a << 24) | ((u32)pDL->ConstantColor.r << 16) |
							 ((u32)pDL->ConstantColor.g << 8) | pDL->ConstantColor.b;
				}
				else
				{
					const u32 nColorOffset = _Offset(pVB->pDiffuse) + (u32)Attr.nDiffuse * sizeof(FGCColor_t);
					if (!_Range(nColorOffset, sizeof(FGCColor_t), nFileBytes))
						GCM_FAIL();
					const u8 *pColor = (const u8 *)_At(pFile, nColorOffset);
					nColor = ((u32)pColor[3] << 24) | ((u32)pColor[0] << 16) | ((u32)pColor[1] << 8) | pColor[2];
				}

				f32 afST[FGCVB_MAX_ST_SETS][2] = {};
				for (u32 nST = 0; nST < nSTCount; nST++)
				{
					const u32 nSTOffset = _Offset(pVB->pST) + (u32)Attr.anST[nST] * sizeof(FGCST16_t);
					const u8 *pST = (const u8 *)_At(pFile, nSTOffset);
					afST[nST][0] = (f32)(s16)_ReadBE16(pST) * (1.0f / 256.0f);
					afST[nST][1] = (f32)(s16)_ReadBE16(pST + 2) * (1.0f / 256.0f);
				}

				if (pWrite->bWeighted)
				{
					if (!bSkinned || !pWrite->pSegBones || !pWrite->nSegBoneCount)
						GCM_FAIL();
					f32 afBlend[3];
					_ComputeSegWeights(pSkin->aVerts[Attr.nPosition], pWrite->pSegBones, pWrite->nSegBoneCount, afBlend);
					if (pWrite->nSTCount == 1)
					{
						FDX8VB_N1W3C1T1_t V;
						V.fPosX = fPosition[0];
						V.fPosY = fPosition[1];
						V.fPosZ = fPosition[2];
						V.fW0 = afBlend[0];
						V.fW1 = afBlend[1];
						V.fW2 = afBlend[2];
						V.fNormX = fNormal[0];
						V.fNormY = fNormal[1];
						V.fNormZ = fNormal[2];
						V.nDiffuseRGBA = nColor;
						V.fS0 = afST[0][0];
						V.fT0 = afST[0][1];
						memcpy(pWrite->pVertexData + nRow * sizeof(V), &V, sizeof(V));
					}
					else
					{
						FDX8VB_N1W3C1T2_t V;
						V.fPosX = fPosition[0];
						V.fPosY = fPosition[1];
						V.fPosZ = fPosition[2];
						V.fW0 = afBlend[0];
						V.fW1 = afBlend[1];
						V.fW2 = afBlend[2];
						V.fNormX = fNormal[0];
						V.fNormY = fNormal[1];
						V.fNormZ = fNormal[2];
						V.nDiffuseRGBA = nColor;
						V.fS0 = afST[0][0];
						V.fT0 = afST[0][1];
						V.fS1 = afST[1][0];
						V.fT1 = afST[1][1];
						memcpy(pWrite->pVertexData + nRow * sizeof(V), &V, sizeof(V));
					}
				}
				else if (pWrite->nSTCount > 0)
				{
					if (pWrite->nSTCount == 1)
					{
						FDX8VB_N1C1T1_t V;
						V.fPosX = fPosition[0];
						V.fPosY = fPosition[1];
						V.fPosZ = fPosition[2];
						V.fNormX = fNormal[0];
						V.fNormY = fNormal[1];
						V.fNormZ = fNormal[2];
						V.nDiffuseRGBA = nColor;
						V.fS0 = afST[0][0];
						V.fT0 = afST[0][1];
						memcpy(pWrite->pVertexData + nRow * sizeof(V), &V, sizeof(V));
					}
					else
					{
						FDX8VB_N1C1T2_t V;
						V.fPosX = fPosition[0];
						V.fPosY = fPosition[1];
						V.fPosZ = fPosition[2];
						V.fNormX = fNormal[0];
						V.fNormY = fNormal[1];
						V.fNormZ = fNormal[2];
						V.nDiffuseRGBA = nColor;
						V.fS0 = afST[0][0];
						V.fT0 = afST[0][1];
						V.fS1 = afST[1][0];
						V.fT1 = afST[1][1];
						memcpy(pWrite->pVertexData + nRow * sizeof(V), &V, sizeof(V));
					}
				}

				for (u32 nLM = 0; nLM < pMaterial->nLightMapSTSets; nLM++)
				{
					FDX8LightMapST_t LM;
					LM.fS = afST[pMaterial->nBaseSTSets + nLM][0];
					LM.fT = afST[pMaterial->nBaseSTSets + nLM][1];
					memcpy(pWrite->pLightMapData + (nRow * pMaterial->nLightMapSTSets + nLM) * sizeof(LM), &LM,
						   sizeof(LM));
				}
				FDX8BasisVectors_t Basis;
				Basis.fTx = fTangent[0];
				Basis.fTy = fTangent[1];
				Basis.fTz = fTangent[2];
				Basis.fBx = fBinormal[0];
				Basis.fBy = fBinormal[1];
				Basis.fBz = fBinormal[2];
				memcpy(pWrite->pBasisData + nRow * sizeof(Basis), &Basis, sizeof(Basis));
			}
		}

		if (pWrite)
		{
			const u32 nOutTriCount = bStrip ? nVertexCount - 2 : nVertexCount / 3;
			for (u32 nTri = 0; nTri < nOutTriCount; nTri++)
			{
				u16 aTri[3];
				if (bStrip && (nTri & 1))
				{
					aTri[0] = anCommandRows[nTri + 1];
					aTri[1] = anCommandRows[nTri];
					aTri[2] = anCommandRows[nTri + 2];
				}
				else if (bStrip)
				{
					aTri[0] = anCommandRows[nTri];
					aTri[1] = anCommandRows[nTri + 1];
					aTri[2] = anCommandRows[nTri + 2];
				}
				else
				{
					aTri[0] = anCommandRows[nTri * 3];
					aTri[1] = anCommandRows[nTri * 3 + 1];
					aTri[2] = anCommandRows[nTri * 3 + 2];
				}
				for (u32 k = 0; k < 3; k++)
					pWrite->pIndices[pWrite->nWrittenIndices++] = aTri[k];
			}
		}

		if (pStats)
		{
			pStats->nVertexCount += nVertexCount;
			pStats->nIndexCount += (bStrip ? (nVertexCount - 2) * 3 : nVertexCount);
		}
	}

	return TRUE;
}

static void _SwapWords(u32 *pWords, u32 nCount)
{
	for (u32 i = 0; i < nCount; i++)
		pWords[i] = fang_ConvertEndian(pWords[i]);
}

static BOOL _ConvertTexInst(void *pData, u32 nBytes, u32 nOffset, std::vector<u32> &anConverted)
{
	if (!nOffset)
		return TRUE;
	if (!_Range(nOffset, sizeof(FShTexInst_t), nBytes))
		return FALSE;
	for (u32 i = 0; i < anConverted.size(); i++)
		if (anConverted[i] == nOffset)
			return TRUE;
	((FShTexInst_t *)((u8 *)pData + nOffset))->ChangeEndian();
	anConverted.push_back(nOffset);
	return TRUE;
}

static BOOL _ConvertMotif(void *pData, u32 nBytes, u32 nOffset, std::vector<u32> &anConverted)
{
	if (!nOffset)
		return TRUE;
	if (!_Range(nOffset, sizeof(CFColorMotif), nBytes))
		return FALSE;
	for (u32 i = 0; i < anConverted.size(); i++)
		if (anConverted[i] == nOffset)
			return TRUE;
	((CFColorMotif *)((u8 *)pData + nOffset))->ChangeEndian();
	anConverted.push_back(nOffset);
	return TRUE;
}

static BOOL _ConvertMaterialRegisters(void *pData, u32 nBytes, FMeshMaterial_t *pMaterial, std::vector<u32> &anTextures,
									  std::vector<u32> &anMotifs)
{
	const u32 nSurfaceOffset = _Offset(pMaterial->pnShSurfaceRegisters);
	const u32 nLightOffset = _Offset(pMaterial->pnShLightRegisters);
	if (pMaterial->nSurfaceShaderIdx >= FSHADERS_SHADER_COUNT || pMaterial->nLightShaderIdx >= FSHADERS_DIFFUSE_COUNT ||
		!_RangeArray(nSurfaceOffset, FShaders_aShaderRegs[pMaterial->nSurfaceShaderIdx].nRegisterCount, sizeof(u32),
					 nBytes) ||
		!_RangeArray(nLightOffset, FShaders_aLightShaderRegs[pMaterial->nLightShaderIdx].nRegisterCount, sizeof(u32),
					 nBytes))
		return FALSE;

	u32 *pSurface = (u32 *)((u8 *)pData + nSurfaceOffset);
	const FShaderReg_t &SurfaceDesc = FShaders_aShaderRegs[pMaterial->nSurfaceShaderIdx];
	for (u32 i = 0; i < SurfaceDesc.nRegisterCount; i++)
		pSurface[i] = fang_ConvertEndian(pSurface[i]);
	for (u32 i = 0; i < SurfaceDesc.nRegisterCount; i++)
	{
		const u32 nValue = pSurface[i];
		switch (SurfaceDesc.anRegType[i])
		{
		case FSHADERS_REG_LAYER0:
		case FSHADERS_REG_LAYER1:
		case FSHADERS_REG_LAYER2:
		case FSHADERS_REG_LAYER3:
		case FSHADERS_REG_DETAILMAP:
			if (!_ConvertTexInst(pData, nBytes, nValue, anTextures))
				return FALSE;
			break;
		case FSHADERS_REG_ENV_MOTIF:
			if (!_ConvertMotif(pData, nBytes, nValue, anMotifs))
				return FALSE;
			break;
		default:
			break;
		}
	}

	u32 *pLight = (u32 *)((u8 *)pData + nLightOffset);
	const FLightShaderReg_t &LightDesc = FShaders_aLightShaderRegs[pMaterial->nLightShaderIdx];
	for (u32 i = 0; i < LightDesc.nRegisterCount; i++)
		pLight[i] = fang_ConvertEndian(pLight[i]);
	for (u32 i = 0; i < LightDesc.nRegisterCount; i++)
	{
		const u32 nValue = pLight[i];
		switch (LightDesc.anRegType[i])
		{
		case FSHADERS_LIGHT_REG_ZMASK:
		case FSHADERS_LIGHT_REG_EMASK:
		case FSHADERS_LIGHT_REG_SMASK:
		case FSHADERS_LIGHT_REG_BUMPMAP:
			if (!_ConvertTexInst(pData, nBytes, nValue, anTextures))
				return FALSE;
			break;
		case FSHADERS_LIGHT_REG_DMOTIF:
		case FSHADERS_LIGHT_REG_EMOTIF:
		case FSHADERS_LIGHT_REG_SMOTIF:
			if (!_ConvertMotif(pData, nBytes, nValue, anMotifs))
				return FALSE;
			break;
		default:
			break;
		}
	}

	u32 nReg = LightDesc.nRegisterCount;
	while (TRUE)
	{
		if (nLightOffset > nBytes || nReg >= (nBytes - nLightOffset) / sizeof(u32))
			return FALSE;
		const u32 nTailOffset = nLightOffset + nReg * sizeof(u32);
		const u32 nRawValue = _ReadBE32((const u8 *)pData + nTailOffset);
		if (nRawValue == 0xffffffffu)
		{
			pLight[nReg] = 0xffffffffu;
			break;
		}
		if ((nBytes - nTailOffset) / sizeof(u32) < 3)
			return FALSE;
		pLight[nReg] = nRawValue;
		if (!_ConvertTexInst(pData, nBytes, pLight[nReg], anTextures))
			return FALSE;
		nReg++;
		pLight[nReg] = fang_ConvertEndian(pLight[nReg]); // ST set index
		nReg++;
		pLight[nReg] = fang_ConvertEndian(pLight[nReg]); // light motif id
		nReg++;
	}
	return TRUE;
}

static BOOL _ConvertSourceEndian(void *pData, u32 nBytes, FMesh_t **ppMesh, FGCMesh_t **ppGCMesh,
								 std::vector<u32> &anTextures, std::vector<u32> &anMotifs)
{
	if (nBytes < sizeof(FMesh_t))
		return FALSE;
	FMesh_t *pMesh = (FMesh_t *)pData;
	pMesh->ChangeEndian();

	const u32 nSegOffset = _Offset(pMesh->aSeg);
	const u32 nBoneOffset = _Offset(pMesh->pBoneArray);
	const u32 nLightOffset = _Offset(pMesh->pLightArray);
	const u32 nMaterialOffset = _Offset(pMesh->aMtl);
	const u32 nTexOffset = _Offset(pMesh->pTexLayerIDArray);
	const u32 nGCOffset = _Offset(pMesh->pMeshIS);
	if (!nGCOffset || !_RangeArray(nSegOffset, pMesh->nSegCount, sizeof(FMeshSeg_t), nBytes) ||
		!_RangeArray(nBoneOffset, pMesh->nBoneCount, sizeof(FMeshBone_t), nBytes) ||
		!_RangeArray(nLightOffset, pMesh->nLightCount, sizeof(FMeshLight_t), nBytes) ||
		!_RangeArray(nMaterialOffset, pMesh->nMaterialCount, sizeof(FMeshMaterial_t), nBytes) ||
		!_RangeArray(nTexOffset, pMesh->nTexLayerIDCount, sizeof(FMeshTexLayerID_t), nBytes) ||
		!_Range(nGCOffset, sizeof(FGCMesh_t), nBytes))
		return FALSE;

	for (u32 i = 0; i < pMesh->nSegCount; i++)
		((FMeshSeg_t *)((u8 *)pData + nSegOffset))[i].ChangeEndian();
	for (u32 i = 0; i < pMesh->nBoneCount; i++)
		((FMeshBone_t *)((u8 *)pData + nBoneOffset))[i].ChangeEndian();
	for (u32 i = 0; i < pMesh->nLightCount; i++)
		((FMeshLight_t *)((u8 *)pData + nLightOffset))[i].LightInit.ChangeEndian();
	for (u32 i = 0; i < pMesh->nTexLayerIDCount; i++)
		((FMeshTexLayerID_t *)((u8 *)pData + nTexOffset))[i].ChangeEndian();
	FMeshMaterial_t *pMaterials = (FMeshMaterial_t *)((u8 *)pData + nMaterialOffset);
	for (u32 i = 0; i < pMesh->nMaterialCount; i++)
		pMaterials[i].ChangeEndian();

	FGCMesh_t *pGCMesh = (FGCMesh_t *)((u8 *)pData + nGCOffset);
	pGCMesh->ChangeEndian();
	const u32 nVBOffset = _Offset(pGCMesh->aVB);
	if (pGCMesh->nVBCount > 255 || !_RangeArray(nVBOffset, pGCMesh->nVBCount, sizeof(FGCVB_t), nBytes))
		return FALSE;
	FGCVB_t *pVBs = (FGCVB_t *)((u8 *)pData + nVBOffset);
	for (u32 i = 0; i < pGCMesh->nVBCount; i++)
		pVBs[i].ChangeEndian();

	for (u32 i = 0; i < pMesh->nMaterialCount; i++)
	{
		const u32 nGCMaterialOffset = _Offset(pMaterials[i].pPlatformData);
		if (!nGCMaterialOffset || !_Range(nGCMaterialOffset, sizeof(FGCMeshMaterial_t), nBytes))
			return FALSE;
		FGCMeshMaterial_t *pGCMaterial = (FGCMeshMaterial_t *)((u8 *)pData + nGCMaterialOffset);
		pGCMaterial->ChangeEndian();
		const u32 nDLOffset = _Offset(pGCMaterial->aDLContainer);
		if (!_RangeArray(nDLOffset, pGCMaterial->nDLContCount, sizeof(FGC_DLCont_t), nBytes))
			return FALSE;
		for (u32 j = 0; j < pGCMaterial->nDLContCount; j++)
		{
			FGC_DLCont_t *pDL = &((FGC_DLCont_t *)((u8 *)pData + nDLOffset))[j];
			pDL->ChangeEndian();
		}
		if (!_ConvertMaterialRegisters(pData, nBytes, &pMaterials[i], anTextures, anMotifs))
			return FALSE;
	}

	*ppMesh = pMesh;
	*ppGCMesh = pGCMesh;
	return TRUE;
}

// A kDOP leaf whose triangle data is rebuilt for DX. On GC the leaf holds u16 position indices (into
// the GC VB named by each packet) followed by compressed s16 normals; DX expects the same indices
// followed by f32 normals, and resolves the indices through FDX8Mesh_t::apCollVertBuffer[nVBIdx].
struct CollLeaf_t
{
	u32 nNodeOffset;
	u32 nTriDataOffset;
	u32 nTriCount;
	u32 nNewOffset;
};

static const u8 _ankDOPAxes[FkDOP_MAX_kDOPS] = {FkDOP_6_DOP_AXES, FkDOP_14_DOP_AXES, FkDOP_18_DOP_AXES,
												 FkDOP_26_DOP_AXES};

static u32 _CollLeafBytes(u32 nTriCount)
{
	return ((nTriCount * 3 * sizeof(u16) + 3u) & ~3u) + nTriCount * sizeof(FkDOP_Normal_t);
}

static BOOL _MarkConverted(std::vector<u32> &anDone, u32 nOffset)
{
	for (u32 i = 0; i < anDone.size(); i++)
		if (anDone[i] == nOffset)
			return FALSE;
	anDone.push_back(nOffset);
	return TRUE;
}

// Validates the GC kDOP collision trees and endian-converts the trees, nodes, packets, root verts and
// intervals in place. Leaf triangle data is left big-endian; it is rebuilt by _WriteCollisionLeaves().
static BOOL _PrepareCollision(u8 *pData, u32 nBytes, const FMesh_t *pMesh, const FGCMesh_t *pGCMesh,
							  const FGCVB_t *pVBs, std::vector<CollLeaf_t> &Leaves)
{
	const u32 nTreeOffset = _Offset(pMesh->paCollTree);
	if (!_RangeArray(nTreeOffset, pMesh->nCollTreeCount, sizeof(FkDOP_Tree_t), nBytes))
		GCM_FAIL();
	std::vector<u32> anDone;
	for (u32 t = 0; t < pMesh->nCollTreeCount; t++)
	{
		FkDOP_Tree_t *pTree = (FkDOP_Tree_t *)(pData + nTreeOffset) + t;
		pTree->ChangeEndian();
		if (pTree->nTreekDOPType >= FkDOP_MAX_kDOPS || pTree->nSegmentIdx >= pMesh->nSegCount)
			GCM_FAIL();
		const u32 nAxes = _ankDOPAxes[pTree->nTreekDOPType];
		const u32 nNodeOffset = _Offset(pTree->pakDOPNodes);
		if (!pTree->nTreeNodeCount || !_RangeArray(nNodeOffset, pTree->nTreeNodeCount, sizeof(FkDOP_Node_t), nBytes))
			GCM_FAIL();

		const u32 nRootOffset = _Offset(pTree->paRootkDOPVerts);
		if (!_RangeArray(nRootOffset, pTree->nRootkDOPVertCount, sizeof(CFVec3), nBytes))
			GCM_FAIL();
		if (nRootOffset && _MarkConverted(anDone, nRootOffset))
			_SwapWords((u32 *)(pData + nRootOffset), pTree->nRootkDOPVertCount * 3);

		u32 nIntervalCount = 0;
		FkDOP_Node_t *pNodes = (FkDOP_Node_t *)(pData + nNodeOffset);
		for (u32 n = 0; n < pTree->nTreeNodeCount; n++)
		{
			FkDOP_Node_t *pNode = &pNodes[n];
			pNode->ChangeEndian();
			if ((u32)pNode->nStartkDOPInterval + nAxes > nIntervalCount)
				nIntervalCount = (u32)pNode->nStartkDOPInterval + nAxes;
			const u32 nPacketOffset = _Offset(pNode->paPackets);
			if (!nPacketOffset)
			{
				if ((u32)pNode->nStartChildIdx + 1 >= pTree->nTreeNodeCount)
					GCM_FAIL();
				continue;
			}
			const u32 nTriCount = pNode->nTriCount;
			const u32 nTriOffset = _Offset(pNode->pTriData);
			if (!pNode->nTriPacketCount ||
				!_RangeArray(nPacketOffset, pNode->nTriPacketCount, sizeof(FkDOP_TriPacket_t), nBytes) || !nTriOffset ||
				!_Range(nTriOffset, ((nTriCount * 3 * sizeof(u16) + 3u) & ~3u) + nTriCount * sizeof(FkDOP_CNormal_t),
						nBytes))
				GCM_FAIL();
			FkDOP_TriPacket_t *pPackets = (FkDOP_TriPacket_t *)(pData + nPacketOffset);
			for (u32 p = 0; p < pNode->nTriPacketCount; p++)
			{
				FkDOP_TriPacket_t *pPacket = &pPackets[p];
				pPacket->ChangeEndian();
				if (pPacket->nVBIdx >= pGCMesh->nVBCount ||
					(u32)pPacket->nStartVert + (u32)pPacket->nTriCount * 3 > nTriCount * 3)
					GCM_FAIL();
				for (u32 v = 0; v < (u32)pPacket->nTriCount * 3; v++)
					if (_ReadBE16(pData + nTriOffset + ((u32)pPacket->nStartVert + v) * sizeof(u16)) >=
						pVBs[pPacket->nVBIdx].nPosCount)
						GCM_FAIL();
			}
			CollLeaf_t Leaf = {nNodeOffset + n * (u32)sizeof(FkDOP_Node_t), nTriOffset, nTriCount, 0};
			Leaves.push_back(Leaf);
		}

		const u32 nIntervalOffset = _Offset(pTree->paIntervals);
		if (!_RangeArray(nIntervalOffset, nIntervalCount, sizeof(FkDOP_Interval_t), nBytes) || !nIntervalOffset)
			GCM_FAIL();
		if (_MarkConverted(anDone, nIntervalOffset))
			_SwapWords((u32 *)(pData + nIntervalOffset), nIntervalCount * 2);
	}
	return TRUE;
}

// Rebuilds each leaf's triangle data in DX layout at its reserved offset and points the node at it.
// Packet VB indices are moved onto the position-only collision VBs that follow the draw VBs.
static void _WriteCollisionLeaves(u8 *pOutput, const std::vector<CollLeaf_t> &Leaves, u32 nCollVBBase)
{
	for (u32 l = 0; l < Leaves.size(); l++)
	{
		const CollLeaf_t &Leaf = Leaves[l];
		FkDOP_Node_t *pNode = (FkDOP_Node_t *)(pOutput + Leaf.nNodeOffset);
		const u8 *pSrc = pOutput + Leaf.nTriDataOffset;
		u8 *pDst = pOutput + Leaf.nNewOffset;
		const u32 nIdxBytes = (Leaf.nTriCount * 3 * sizeof(u16) + 3u) & ~3u;
		for (u32 i = 0; i < Leaf.nTriCount * 3; i++)
			((u16 *)pDst)[i] = _ReadBE16(pSrc + i * sizeof(u16));
		FkDOP_Normal_t *pNormals = (FkDOP_Normal_t *)(pDst + nIdxBytes);
		for (u32 i = 0; i < Leaf.nTriCount; i++)
		{
			const u8 *pCN = pSrc + nIdxBytes + i * sizeof(FkDOP_CNormal_t);
			pNormals[i].x = (f32)(s16)_ReadBE16(pCN + 0) * FMESH_NORMAL_DECOMPRESS_MOD;
			pNormals[i].y = (f32)(s16)_ReadBE16(pCN + 2) * FMESH_NORMAL_DECOMPRESS_MOD;
			pNormals[i].z = (f32)(s16)_ReadBE16(pCN + 4) * FMESH_NORMAL_DECOMPRESS_MOD;
		}
		pNode->pTriData = (void *)(uintptr_t)Leaf.nNewOffset;
		FkDOP_TriPacket_t *pPackets = (FkDOP_TriPacket_t *)(pOutput + _Offset(pNode->paPackets));
		for (u32 p = 0; p < pNode->nTriPacketCount; p++)
			pPackets[p].nVBIdx = (u8)(pPackets[p].nVBIdx + nCollVBBase);
	}
}

// Decodes GC VB position nIndex the way FGCVB_t::GetPoint() does (skinned VBs use their 1/64 packing).
static void _DecodeGCPosition(const u8 *pFile, const FGCVB_t *pVB, u32 nIndex, f32 afOut[3])
{
	const u8 *pPos = pFile + _Offset(pVB->pPosition) + nIndex * pVB->nPosStride;
	const f32 fScale = (pVB->nFlags & FGCVB_SKINNED) ? (1.0f / 64.0f) : (f32)ldexp(1.0, -(int)pVB->nPosFrac);
	for (u32 k = 0; k < 3; k++)
	{
		if (pVB->nPosType == GX_S8)
			afOut[k] = (f32)(s8)pPos[k] * fScale;
		else if (pVB->nPosType == GX_S16)
			afOut[k] = (f32)(s16)_ReadBE16(pPos + k * 2) * fScale;
		else
		{
			const u32 nBits = _ReadBE32(pPos + k * 4);
			memcpy(&afOut[k], &nBits, sizeof(f32));
			if (!isfinite(afOut[k]))
				afOut[k] = 0.0f;
		}
	}
}

static void _DumpDL(const u8 *pFile, u32 nFileBytes, const FGCVB_t *pVB, const FMeshMaterial_t *pMaterial,
					const FGC_DLCont_t *pDL, u32 nMaterial, u32 nDL)
{
	DEVPRINTF("gcmesh:   mtl %u dl %u: flags=%02x mtx=%u lod=%u part=%u strips=%u lists=%u stripTris=%u listTris=%u "
			  "vb=%u size=%u buf=%08x baseST=%u lmST=%u\n",
			  nMaterial, nDL, pDL->nFlags, pDL->nMatrixIdx, pDL->nLODID, pDL->nPartID, pDL->nStripCount, pDL->nListCount,
			  pDL->nStripTriCount, pDL->nListTriCount, pDL->nVBIndex, pDL->nSize, _Offset(pDL->pBuffer),
			  pMaterial->nBaseSTSets, pMaterial->nLightMapSTSets);
	DEVPRINTF("gcmesh:   vb: flags=%04x posCount=%u posType=%u posIdx=%u stride=%u frac=%u diffCount=%u clrIdx=%u fmt=%u "
			  "pos=%08x diff=%08x st=%08x nbt=%08x\n",
			  pVB->nFlags, pVB->nPosCount, pVB->nPosType, pVB->nPosIdxType, pVB->nPosStride, pVB->nPosFrac,
			  pVB->nDiffuseCount, pVB->nColorIdxType, pVB->nGCVertexFormat, _Offset(pVB->pPosition),
			  _Offset(pVB->pDiffuse), _Offset(pVB->pST), _Offset(pVB->pNBT));
	const u8 *pData = NULL;
	if (pDL->nFlags & FGCDL_FLAGS_STREAMING)
		pData = (const u8 *)fvis_GetWorldStreamingData(_Offset(pDL->pBuffer), pDL->nSize);
	else if (_Range(_Offset(pDL->pBuffer), pDL->nSize, nFileBytes))
		pData = pFile + _Offset(pDL->pBuffer);
	if (!pData)
	{
		DEVPRINTF("gcmesh:   (DL bytes unavailable)\n");
		return;
	}
	const u32 nDump = pDL->nSize < 96 ? pDL->nSize : 96;
	char szLine[3 * 96 + 1];
	for (u32 i = 0; i < nDump; i++)
		sprintf(szLine + i * 3, "%02x ", pData[i]);
	szLine[nDump * 3] = 0;
	DEVPRINTF("gcmesh:   bytes: %s\n", szLine);
}

// Chooses the FMeshSeg_t a display list draws with on DX (FDX8MeshCluster_t::nSegmentIdx). GC display
// lists instead carry a matrix-buffer index: the bone index for rigid geometry (verts in bone space), or
// nUsedBoneCount - the appended view matrix - for skinned (model-space verts, CPU-skinned) and boneless
// geometry. The segments themselves come from the same PASM pass on both platforms (MLSegment.cpp), so
// the original segment is matched when possible and a new one is appended only as a fallback.
static BOOL _BindSegment(const FMesh_t *pMesh, const FMeshSeg_t *pSrcSegs, std::vector<FMeshSeg_t> &NewSegs,
						 const CFSphere &Sphere, const FGC_DLCont_t *pDL, const ParseStats_t &Stats, DLPlan_t *pPlan,
						 u32 *pnApproximated)
{
	pPlan->bWeighted = FALSE;
	pPlan->nSegBoneCount = 0;
	pPlan->nSegmentIdx = 0;
	memset(pPlan->anSegBones, 255, sizeof(pPlan->anSegBones));
	if (!pMesh->nBoneCount)
	{
		if (Stats.bSkinned)
			GCM_FAIL();
		return TRUE; // fdx8mesh never consults segments for a mesh without bones
	}

	u8 anWant[FDATA_VW_COUNT_PER_VTX];
	u32 nWant = 0;
	if (Stats.bSkinned)
	{
		// The (up to) four most heavily weighted bones. PASM segments never exceed four bones, so more
		// than that means the display list spans segments and the blend has to be approximated.
		u32 nUsed = 0;
		for (u32 b = 0; b < 256; b++)
			if (Stats.afBoneWeight[b] > 0.0f)
				nUsed++;
		if (nUsed > FDATA_VW_COUNT_PER_VTX)
			(*pnApproximated)++;
		while (nWant < FDATA_VW_COUNT_PER_VTX)
		{
			s32 nBest = -1;
			for (u32 b = 0; b < 256; b++)
			{
				if (Stats.afBoneWeight[b] <= 0.0f)
					continue;
				BOOL bTaken = FALSE;
				for (u32 k = 0; k < nWant; k++)
					if (anWant[k] == b)
						bTaken = TRUE;
				if (!bTaken && (nBest < 0 || Stats.afBoneWeight[b] > Stats.afBoneWeight[nBest]))
					nBest = (s32)b;
			}
			if (nBest < 0)
				break;
			anWant[nWant++] = (u8)nBest;
		}
		if (!nWant)
			GCM_FAIL();
	}
	else if (pDL->nMatrixIdx < pMesh->nUsedBoneCount)
	{
		anWant[nWant++] = pDL->nMatrixIdx;
	}

	const u32 nTotalSegs = pMesh->nSegCount + (u32)NewSegs.size();
	s32 nFound = -1;
	u32 nFoundCount = 0;
	for (u32 s = 0; s < nTotalSegs; s++)
	{
		const FMeshSeg_t &Seg = s < pMesh->nSegCount ? pSrcSegs[s] : NewSegs[s - pMesh->nSegCount];
		if (Seg.nBoneMtxCount > FDATA_VW_COUNT_PER_VTX)
			continue;
		if (!Stats.bSkinned)
		{
			if (Seg.nBoneMtxCount == nWant && (!nWant || Seg.anBoneMtxIndex[0] == anWant[0]))
			{
				nFound = (s32)s;
				break;
			}
			continue;
		}
		if (Seg.nBoneMtxCount < 2)
			continue;
		BOOL bAll = TRUE;
		for (u32 k = 0; k < nWant && bAll; k++)
		{
			BOOL bHas = FALSE;
			for (u32 m = 0; m < Seg.nBoneMtxCount; m++)
				if (Seg.anBoneMtxIndex[m] == anWant[k])
					bHas = TRUE;
			bAll = bHas;
		}
		if (bAll && (nFound < 0 || Seg.nBoneMtxCount < nFoundCount))
		{
			nFound = (s32)s;
			nFoundCount = Seg.nBoneMtxCount;
		}
	}
	if (nFound < 0)
	{
		if (nTotalSegs >= 255)
			GCM_FAIL();
		FMeshSeg_t Seg;
		memset(&Seg, 0, sizeof(Seg));
		Seg.BoundSphere_MS = Sphere;
		memset(Seg.anBoneMtxIndex, 255, sizeof(Seg.anBoneMtxIndex));
		if (Stats.bSkinned && nWant == 1)
		{
			// Keep skinned (model-space) geometry on the blend path: bones {b, b}, weights {1, 0}.
			Seg.nBoneMtxCount = 2;
			Seg.anBoneMtxIndex[0] = anWant[0];
			Seg.anBoneMtxIndex[1] = anWant[0];
		}
		else
		{
			Seg.nBoneMtxCount = (u8)nWant;
			for (u32 k = 0; k < nWant; k++)
				Seg.anBoneMtxIndex[k] = anWant[k];
		}
		NewSegs.push_back(Seg);
		nFound = (s32)nTotalSegs;
	}

	pPlan->nSegmentIdx = (u32)nFound;
	if (Stats.bSkinned)
	{
		const FMeshSeg_t &Seg =
			(u32)nFound < pMesh->nSegCount ? pSrcSegs[nFound] : NewSegs[(u32)nFound - pMesh->nSegCount];
		pPlan->bWeighted = TRUE;
		pPlan->nSegBoneCount = Seg.nBoneMtxCount;
		memcpy(pPlan->anSegBones, Seg.anBoneMtxIndex, sizeof(pPlan->anSegBones));
	}
	return TRUE;
}
} // namespace

BOOL gcmesh_ConvertToDx(void *pGameCubeData, u32 nGameCubeBytes, void **ppDxData, u32 *pnDxBytes, cchar *pszResName)
{
	if (ppDxData)
		*ppDxData = NULL;
	if (pnDxBytes)
		*pnDxBytes = 0;
	if (!pGameCubeData || !ppDxData || !pnDxBytes)
		return FALSE;

	std::vector<u32> anTextures;
	std::vector<u32> anMotifs;
	FMesh_t *pMesh = NULL;
	FGCMesh_t *pGCMesh = NULL;
	if (!_ConvertSourceEndian(pGameCubeData, nGameCubeBytes, &pMesh, &pGCMesh, anTextures, anMotifs))
	{
		DEVPRINTF("gcmesh: rejected malformed or unsupported GameCube mesh '%s' (%u bytes).\n",
				  pszResName ? pszResName : "(unnamed)", nGameCubeBytes);
		return FALSE;
	}

	const u32 nMaterialCount = pMesh->nMaterialCount;
	FMeshMaterial_t *pMaterials = (FMeshMaterial_t *)((u8 *)pGameCubeData + _Offset(pMesh->aMtl));
	FGCVB_t *pVBs = (FGCVB_t *)((u8 *)pGameCubeData + _Offset(pGCMesh->aVB));
	const u32 nSourceMaterialOffset = _Offset(pMesh->aMtl);
	const u32 nSourceVBOffset = _Offset(pGCMesh->aVB);
	const u32 nSourceGCOffset = _Offset(pMesh->pMeshIS);
	std::vector<u32> anGCMaterialOffsets;
	anGCMaterialOffsets.reserve(nMaterialCount);
	for (u32 i = 0; i < nMaterialCount; i++)
		anGCMaterialOffsets.push_back(_Offset(pMaterials[i].pPlatformData));
	std::vector<DLPlan_t> Plans;
	std::vector<FMeshSeg_t> NewSegs;
	SkinInfo_t Skin;
	const SkinInfo_t *pSkin = NULL;
	const FMeshSeg_t *pSrcSegs = (const FMeshSeg_t *)_At((const void *)pGameCubeData, _Offset(pMesh->aSeg));
	u32 nApproximatedDLs = 0;
	u32 nSkinnedDLs = 0;
	std::vector<CollLeaf_t> CollLeaves;
	u32 nCollVBCount = 0;
	u32 nTotalVertices = 0;
	u32 nTotalIndices = 0;
	_nFailLine = 0;
	if (pGCMesh->pMeshSkin)
	{
		if (!_BuildSkinInfo((const u8 *)pGameCubeData, nGameCubeBytes, _Offset(pGCMesh->pMeshSkin), pMesh->nBoneCount,
							&Skin))
			goto Reject;
		pSkin = &Skin;
	}
	for (u32 i = 0; i < nMaterialCount; i++)
	{
		FGCMeshMaterial_t *pGCMaterial =
			(FGCMeshMaterial_t *)((u8 *)pGameCubeData + _Offset(pMaterials[i].pPlatformData));
		FGC_DLCont_t *pDLs = (FGC_DLCont_t *)((u8 *)pGameCubeData + _Offset(pGCMaterial->aDLContainer));
		for (u32 j = 0; j < pGCMaterial->nDLContCount; j++)
		{
			FGC_DLCont_t *pDL = &pDLs[j];
			_nFailLine = __LINE__;
			if (pDL->nVBIndex >= pGCMesh->nVBCount)
				goto Reject;
			ParseStats_t Stats = {};
			if (!_WalkDL((const u8 *)pGameCubeData, nGameCubeBytes, &pVBs[pDL->nVBIndex], &pMaterials[i], pDL, pSkin,
						 &Stats, NULL))
			{
				_DumpDL((const u8 *)pGameCubeData, nGameCubeBytes, &pVBs[pDL->nVBIndex], &pMaterials[i], pDL, i, j);
				goto Reject;
			}
			_nFailLine = __LINE__;
			if (Stats.nVertexCount > 65535 || Stats.nIndexCount > 65535 || Plans.size() >= 255)
				goto Reject;
			DLPlan_t Plan = {};
			Plan.nMaterialIndex = i;
			Plan.nDLIndex = j;
			Plan.nVertexCount = Stats.nVertexCount;
			Plan.nIndexCount = Stats.nIndexCount;
			if (!_BindSegment(pMesh, pSrcSegs, NewSegs, pGCMesh->AtRestBoundSphere_MS, pDL, Stats, &Plan,
							  &nApproximatedDLs))
				goto Reject;
			if (Plan.bWeighted)
			{
				nSkinnedDLs++;
				Plan.nVertexFormat = pMaterials[i].nBaseSTSets > 1 ? FDX8VB_TYPE_N1W3C1T2 : FDX8VB_TYPE_N1W3C1T1;
			}
			else
			{
				Plan.nVertexFormat = pMaterials[i].nBaseSTSets > 1 ? FDX8VB_TYPE_N1C1T2 : FDX8VB_TYPE_N1C1T1;
			}
			Plans.push_back(Plan);
			nTotalVertices += Stats.nVertexCount;
			nTotalIndices += Stats.nIndexCount;
		}
	}

	// Collision (kDOP trees). A problem here drops collision for this mesh rather than the whole mesh.
	if (pMesh->paCollTree && pMesh->nCollTreeCount)
	{
		_nFailLine = 0;
		BOOL bCollOK = _PrepareCollision((u8 *)pGameCubeData, nGameCubeBytes, pMesh, pGCMesh, pVBs, CollLeaves);
		if (bCollOK && Plans.size() + pGCMesh->nVBCount > 255)
		{
			_nFailLine = __LINE__;
			bCollOK = FALSE;
		}
		for (u32 i = 0; bCollOK && i < pGCMesh->nVBCount; i++)
		{
			const FGCVB_t &VB = pVBs[i];
			const u32 nMinStride = VB.nPosType == GX_S8 ? 3 : VB.nPosType == GX_S16 ? 6 : 12;
			if ((VB.nPosType != GX_S8 && VB.nPosType != GX_S16 && VB.nPosType != GX_F32) || VB.nPosStride < nMinStride ||
				VB.nPosFrac > 31 || !_RangeArray(_Offset(VB.pPosition), VB.nPosCount, VB.nPosStride, nGameCubeBytes))
			{
				_nFailLine = __LINE__;
				bCollOK = FALSE;
			}
		}
		if (bCollOK)
		{
			nCollVBCount = pGCMesh->nVBCount;
		}
		else
		{
			DEVPRINTF("gcmesh: '%s': collision data dropped (gcmesh.cpp line %u).\n",
					  pszResName ? pszResName : "(unnamed)", _nFailLine);
			CollLeaves.clear();
		}
	}
	if (!nCollVBCount)
	{
		pMesh->paCollTree = NULL;
		pMesh->nCollTreeCount = 0;
		pMesh->nMeshCollMask = 0;
	}

	{
		u32 nCurrent;
		_nFailLine = __LINE__;
		if (!_Align4Checked(nGameCubeBytes, &nCurrent))
			goto Reject;
		u32 nDXMeshOffset, nDXMaterialsOffset, nClustersOffset, nVBsOffset, nIBsOffset, nIBCountsOffset;
		u32 nSegsOffset = 0;
		const u32 nOutSegCount = NewSegs.empty() ? 0 : pMesh->nSegCount + (u32)NewSegs.size();
		if (nOutSegCount)
		{
			// Rebuilt segment array (originals + appended), kept in the retained (non-disposable) block.
			nCurrent = (nCurrent + 15u) & ~15u;
			if (!_TakeBytes(&nCurrent, nOutSegCount, sizeof(FMeshSeg_t), &nSegsOffset))
				goto Reject;
		}
		if (!_TakeBytes(&nCurrent, 1, sizeof(FDX8Mesh_t), &nDXMeshOffset) ||
			!_TakeBytes(&nCurrent, nMaterialCount, sizeof(FDX8MeshMaterial_t), &nDXMaterialsOffset) ||
			!_TakeBytes(&nCurrent, (u32)Plans.size(), sizeof(FDX8MeshCluster_t), &nClustersOffset) ||
			!_TakeBytes(&nCurrent, (u32)Plans.size() + nCollVBCount, sizeof(FDX8VB_t), &nVBsOffset) ||
			!_TakeBytes(&nCurrent, (u32)Plans.size(), sizeof(void *), &nIBsOffset) ||
			!_TakeBytes(&nCurrent, (u32)Plans.size(), sizeof(u16), &nIBCountsOffset))
			goto Reject;
		// Rebuilt collision leaf data must survive fdx8load_Create(), so it lives in the retained block.
		for (u32 i = 0; i < CollLeaves.size(); i++)
			if (!_TakeBytes(&nCurrent, _CollLeafBytes(CollLeaves[i].nTriCount), 1, &CollLeaves[i].nNewOffset))
				goto Reject;
		u32 nDisposableOffset;
		if (!_Align4Checked(nCurrent, &nDisposableOffset))
			goto Reject;
		std::vector<u32> anCollVBOffsets(nCollVBCount);
		for (u32 i = 0; i < nCollVBCount; i++)
		{
			const u32 nCount = pVBs[i].nPosCount ? pVBs[i].nPosCount : 1;
			if (!_TakeBytes(&nCurrent, nCount, FDX8VB_InfoTable[FDX8VB_TYPE_C1].nVtxBytes, &anCollVBOffsets[i]))
				goto Reject;
		}

		for (u32 i = 0; i < Plans.size(); i++)
		{
			if (!_TakeBytes(&nCurrent, Plans[i].nIndexCount, sizeof(u16), &Plans[i].nIndexOffset) ||
				!_TakeBytes(&nCurrent, Plans[i].nVertexCount, FDX8VB_InfoTable[Plans[i].nVertexFormat].nVtxBytes,
							&Plans[i].nVertexOffset) ||
				!_TakeBytes(&nCurrent, Plans[i].nVertexCount * pMaterials[Plans[i].nMaterialIndex].nLightMapSTSets,
							sizeof(FDX8LightMapST_t), &Plans[i].nLightMapOffset) ||
				!_TakeBytes(&nCurrent, Plans[i].nVertexCount, sizeof(FDX8BasisVectors_t), &Plans[i].nBasisOffset))
				goto Reject;
		}
		u32 nOutputBytes;
		if (!_Align4Checked(nCurrent, &nOutputBytes))
			goto Reject;
		u8 *pOutput = (u8 *)fmem_Alloc(nOutputBytes, 16);
		if (!pOutput)
			goto Reject;
		memset(pOutput, 0, nOutputBytes);
		memcpy(pOutput, pGameCubeData, nGameCubeBytes);
		pMesh = (FMesh_t *)pOutput;
		pMaterials = (FMeshMaterial_t *)(pOutput + nSourceMaterialOffset);
		pVBs = (FGCVB_t *)(pOutput + nSourceVBOffset);
		pGCMesh = (FGCMesh_t *)(pOutput + nSourceGCOffset);
		// pMeshIS now points at the translated descriptor. The source GC descriptor remains in the blob.
		pMesh->pMeshIS = (FDX8Mesh_t *)(uintptr_t)nDXMeshOffset;
		if (nOutSegCount)
		{
			FMeshSeg_t *pOutSegs = (FMeshSeg_t *)(pOutput + nSegsOffset);
			if (pMesh->nSegCount)
				memcpy(pOutSegs, pOutput + _Offset(pMesh->aSeg), pMesh->nSegCount * sizeof(FMeshSeg_t));
			memcpy(pOutSegs + pMesh->nSegCount, &NewSegs[0], NewSegs.size() * sizeof(FMeshSeg_t));
			pMesh->aSeg = (FMeshSeg_t *)(uintptr_t)nSegsOffset;
			pMesh->nSegCount = (u8)nOutSegCount;
		}

		FDX8Mesh_t *pDXMesh = (FDX8Mesh_t *)(pOutput + nDXMeshOffset);
		pDXMesh->nFlags = 0;
		pDXMesh->nVBCount = (u8)(Plans.size() + nCollVBCount);
		pDXMesh->nIBCount = (u8)Plans.size();
		pDXMesh->nDisposableOffset = nDisposableOffset;
		pDXMesh->AtRestBoundSphere_MS = pGCMesh->AtRestBoundSphere_MS;
		pDXMesh->pMesh = (FMesh_t *)(uintptr_t)0;
		pDXMesh->aVB = (FDX8VB_t *)(uintptr_t)nVBsOffset;
		pDXMesh->apCollVertBuffer = NULL;
		pDXMesh->anIndicesCount = (u16 *)(uintptr_t)nIBCountsOffset;
		pDXMesh->apDXIB = (void **)(uintptr_t)nIBsOffset;

		FDX8MeshMaterial_t *pDXMaterials = (FDX8MeshMaterial_t *)(pOutput + nDXMaterialsOffset);
		FDX8MeshCluster_t *pClusters = (FDX8MeshCluster_t *)(pOutput + nClustersOffset);
		FDX8VB_t *pDXVBs = (FDX8VB_t *)(pOutput + nVBsOffset);
		void **apIB = (void **)(pOutput + nIBsOffset);
		u16 *anIBCounts = (u16 *)(pOutput + nIBCountsOffset);
		for (u32 i = 0; i < nMaterialCount; i++)
		{
			pDXMaterials[i].aCluster =
				(FDX8MeshCluster_t *)(uintptr_t)(nClustersOffset + i * sizeof(FDX8MeshCluster_t));
			pDXMaterials[i].nClusterCount = 0;
			pMaterials[i].pPlatformData = (void *)(uintptr_t)(nDXMaterialsOffset + i * sizeof(FDX8MeshMaterial_t));
		}

		for (u32 i = 0; i < Plans.size(); i++)
		{
			DLPlan_t &Plan = Plans[i];
			FMeshMaterial_t *pMaterial = &pMaterials[Plan.nMaterialIndex];
			FGCMeshMaterial_t *pGCMaterial = (FGCMeshMaterial_t *)(pOutput + anGCMaterialOffsets[Plan.nMaterialIndex]);
			FGC_DLCont_t *pDLs = (FGC_DLCont_t *)(pOutput + _Offset(pGCMaterial->aDLContainer));
			FGC_DLCont_t *pDL = &pDLs[Plan.nDLIndex];
			FGCVB_t *pVB = &pVBs[pDL->nVBIndex];
			const u32 nStride = FDX8VB_InfoTable[Plan.nVertexFormat].nVtxBytes;
			FDX8VB_t *pDXVB = &pDXVBs[i];
			pDXVB->nVtxCount = Plan.nVertexCount;
			pDXVB->nBytesPerVertex = (u16)nStride;
			pDXVB->nLMTCCount = pMaterial->nLightMapSTSets;
			pDXVB->pLMUVStream = Plan.nLightMapOffset ? (void *)(uintptr_t)Plan.nLightMapOffset : NULL;
			pDXVB->pBasisStream = Plan.nBasisOffset ? (void *)(uintptr_t)Plan.nBasisOffset : NULL;
			pDXVB->nInfoIndex = (s8)Plan.nVertexFormat;
			pDXVB->bDynamic = FALSE;
			pDXVB->bSoftwareVP = FALSE;
			pDXVB->bLocked = FALSE;
			pDXVB->pLockBuf = NULL;
			pDXVB->nLockOffset = 0;
			pDXVB->nLockBytes = 0;
			pDXVB->pDXVB = (IDirect3DVertexBuffer8 *)(uintptr_t)Plan.nVertexOffset;

			WriteContext_t Write = {};
			Write.pFile = pOutput;
			Write.nFileBytes = nGameCubeBytes;
			Write.pVB = pVB;
			Write.pMaterial = pMaterial;
			Write.pDL = pDL;
			Write.pVertexData = pOutput + Plan.nVertexOffset;
			Write.pLightMapData = Plan.nLightMapOffset ? pOutput + Plan.nLightMapOffset : NULL;
			Write.pBasisData = pOutput + Plan.nBasisOffset;
			Write.pIndices = (u16 *)(pOutput + Plan.nIndexOffset);
			Write.nSTCount = pMaterial->nBaseSTSets > 1 ? 2 : 1;
			Write.bWeighted = Plan.bWeighted;
			Write.pSegBones = Plan.anSegBones;
			Write.nSegBoneCount = Plan.nSegBoneCount;
			_nFailLine = __LINE__;
			if (!_WalkDL(pOutput, nGameCubeBytes, pVB, pMaterial, pDL, pSkin, NULL, &Write))
				goto Reject;
			_nFailLine = __LINE__;
			if (Write.nWrittenVertices != Plan.nVertexCount || Write.nWrittenIndices != Plan.nIndexCount)
				goto Reject;

			FDX8MeshCluster_t *pCluster = &pClusters[i];
			pCluster->nStripCount = 0;
			pCluster->nFlags = (pDL->nFlags & FGCDL_FLAGS_FACING_OPP_DIR_LIGHT) ? CLUSTER_FACING_OPP_DIR_LIGHT : 0;
			pCluster->nSegmentIdx = (u8)Plan.nSegmentIdx;
			pCluster->nVBIndex = (u8)i;
			pCluster->nIBIndex = (u8)i;
			pCluster->nPartID = pDL->nPartID;
			pCluster->nLODID = pDL->nLODID;
			pCluster->pPushBuffer = NULL;
			pCluster->TriList.nTriCount = (u16)(Plan.nIndexCount / 3);
			pCluster->TriList.nStartVindex = 0;
			pCluster->TriList.nVtxIndexMin = 0;
			pCluster->TriList.nVtxIndexRange = (u16)Plan.nVertexCount;
			pCluster->paStripBuffer = NULL;
			pDXMaterials[Plan.nMaterialIndex].nClusterCount++;
			apIB[i] = (void *)(uintptr_t)Plan.nIndexOffset;
			anIBCounts[i] = (u16)Plan.nIndexCount;
		}

		// Position-only VBs mirroring each GC VB, referenced solely by collision packets (never drawn).
		for (u32 i = 0; i < nCollVBCount; i++)
		{
			const FGCVB_t *pVB = &pVBs[i];
			const u32 nCount = pVB->nPosCount ? pVB->nPosCount : 1;
			FDX8VB_t *pDXVB = &pDXVBs[Plans.size() + i];
			pDXVB->nVtxCount = nCount;
			pDXVB->nBytesPerVertex = FDX8VB_InfoTable[FDX8VB_TYPE_C1].nVtxBytes;
			pDXVB->nLMTCCount = 0;
			pDXVB->pLMUVStream = NULL;
			pDXVB->pBasisStream = NULL;
			pDXVB->nInfoIndex = (s8)FDX8VB_TYPE_C1;
			pDXVB->bDynamic = FALSE;
			pDXVB->bSoftwareVP = FALSE;
			pDXVB->bLocked = FALSE;
			pDXVB->pLockBuf = NULL;
			pDXVB->nLockOffset = 0;
			pDXVB->nLockBytes = 0;
			pDXVB->pDXVB = (IDirect3DVertexBuffer8 *)(uintptr_t)anCollVBOffsets[i];
			FDX8VB_C1_t *pVerts = (FDX8VB_C1_t *)(pOutput + anCollVBOffsets[i]);
			for (u32 v = 0; v < pVB->nPosCount; v++)
			{
				f32 afPos[3];
				_DecodeGCPosition(pOutput, pVB, v, afPos);
				pVerts[v].fPosX = afPos[0];
				pVerts[v].fPosY = afPos[1];
				pVerts[v].fPosZ = afPos[2];
				pVerts[v].nDiffuseRGBA = 0xffffffffu;
			}
		}
		_WriteCollisionLeaves(pOutput, CollLeaves, (u32)Plans.size());

		// Clusters are emitted in material order, matching the source mesh order.
		for (u32 i = 0; i < nMaterialCount; i++)
		{
			u32 nStart = 0;
			while (nStart < Plans.size() && Plans[nStart].nMaterialIndex < i)
				nStart++;
			pDXMaterials[i].aCluster =
				Plans.empty() ? NULL
							  : (FDX8MeshCluster_t *)(uintptr_t)(nClustersOffset + nStart * sizeof(FDX8MeshCluster_t));
		}

		*ppDxData = pOutput;
		*pnDxBytes = nOutputBytes;
		DEVPRINTF("gcmesh: converted '%s' (%u vertices, %u triangles, %u materials, %u skinned DLs, %u segs, "
				  "%u coll trees / %u leaves).\n",
				  pszResName ? pszResName : "(unnamed)", nTotalVertices, nTotalIndices / 3, nMaterialCount,
				  nSkinnedDLs, (u32)pMesh->nSegCount, (u32)pMesh->nCollTreeCount, (u32)CollLeaves.size());
		if (nApproximatedDLs)
			DEVPRINTF("gcmesh: '%s': %u skinned display lists use more than 4 bones; blend approximated.\n",
					  pszResName ? pszResName : "(unnamed)", nApproximatedDLs);
		return TRUE;
	}

Reject:
	DEVPRINTF("gcmesh: unable to convert '%s' (unsupported data or invalid display list; gcmesh.cpp line %u).\n",
			  pszResName ? pszResName : "(unnamed)", _nFailLine);
	return FALSE;
}
