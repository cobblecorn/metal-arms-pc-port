#include "gcmesh.h"

#include "fdx8load.h"
#include "fdx8mesh.h"
#include "fdx8vb.h"
#include "fmesh.h"
#include "fmesh_coll.h"
#include "fsh.h"
#include "fshaders.h"
#include "fres.h"
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
};

struct AttrIndex_t
{
	u16 nPosition;
	u16 nNormal;
	u16 nDiffuse;
	u16 anST[FGCVB_MAX_ST_SETS];
};

struct ParseStats_t
{
	u32 nVertexCount;
	u32 nIndexCount;
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

static BOOL _WalkDL(const u8 *pFile, u32 nFileBytes, const FGCVB_t *pVB, const FMeshMaterial_t *pMaterial,
					const FGC_DLCont_t *pDL, ParseStats_t *pStats, WriteContext_t *pWrite)
{
	const u32 nBufferOffset = _Offset(pDL->pBuffer);
	if (!nBufferOffset || !_Range(nBufferOffset, pDL->nSize, nFileBytes))
		return FALSE;
	if (pDL->nFlags & FGCDL_FLAGS_STREAMING)
		return FALSE;
	if (pDL->nFlags & FGCDL_FLAGS_SKINNED)
		return FALSE;
	if (pVB->nFlags & FGCVB_SKINNED)
		return FALSE;
	if (pMaterial->nBaseSTSets > 2 || pMaterial->nLightMapSTSets > FGCVB_MAX_ST_SETS ||
		pMaterial->nBaseSTSets + pMaterial->nLightMapSTSets > FGCVB_MAX_ST_SETS)
		return FALSE;
	if (pVB->nPosIdxType != GX_INDEX8 && pVB->nPosIdxType != GX_INDEX16)
		return FALSE;
	if (!(pDL->nFlags & FGCDL_FLAGS_CONSTANT_COLOR) && pVB->nColorIdxType != GX_INDEX8 &&
		pVB->nColorIdxType != GX_INDEX16)
		return FALSE;
	if (pVB->nPosType != GX_S8 && pVB->nPosType != GX_S16 && pVB->nPosType != GX_F32)
		return FALSE;
	if (pVB->nPosFrac > 31 || !_RangeArray(_Offset(pVB->pPosition), pVB->nPosCount, pVB->nPosStride, nFileBytes))
		return FALSE;
	if (!(pDL->nFlags & FGCDL_FLAGS_CONSTANT_COLOR) &&
		!_RangeArray(_Offset(pVB->pDiffuse), pVB->nDiffuseCount, sizeof(FGCColor_t), nFileBytes))
		return FALSE;
	if (!pVB->pPosition || !pVB->pST || (!(pDL->nFlags & FGCDL_FLAGS_CONSTANT_COLOR) && !pVB->pDiffuse))
		return FALSE;

	const u8 *pCursor = (const u8 *)_At(pFile, nBufferOffset);
	const u8 *pEnd = pCursor + pDL->nSize;
	const u32 nPrimitiveCount = (u32)pDL->nStripCount + pDL->nListCount;
	const u32 nSTCount = (u32)pMaterial->nBaseSTSets + pMaterial->nLightMapSTSets;
	for (u32 nPrimitive = 0; nPrimitive < nPrimitiveCount; nPrimitive++)
	{
		if ((u32)(pEnd - pCursor) < 3)
			return FALSE;
		const u8 nCommand = *pCursor++;
		const u8 nPrimitiveType = nCommand & 0xf8;
		if (nPrimitiveType != 0x90 && nPrimitiveType != 0x98)
			return FALSE;
		const BOOL bStrip = nPrimitiveType == 0x98;
		const u32 nVertexCount = _ReadBE16(pCursor);
		pCursor += 2;
		if (nVertexCount < 3 || (!bStrip && (nVertexCount % 3)))
			return FALSE;
		if ((u32)(pEnd - pCursor) < nVertexCount)
			return FALSE;

		std::vector<u16> anCommandRows;
		if (pWrite)
			anCommandRows.reserve(nVertexCount);
		for (u32 nVertex = 0; nVertex < nVertexCount; nVertex++)
		{
			AttrIndex_t Attr;
			memset(&Attr, 0, sizeof(Attr));
			if (!_ReadAttrIndex(&pCursor, pEnd, pVB->nPosIdxType, &Attr.nPosition) ||
				(u32)Attr.nPosition >= pVB->nPosCount || (u32)(pEnd - pCursor) < 2)
				return FALSE;
			Attr.nNormal = _ReadBE16(pCursor);
			pCursor += 2;

			if (!(pDL->nFlags & FGCDL_FLAGS_CONSTANT_COLOR))
			{
				if (!_ReadAttrIndex(&pCursor, pEnd, pVB->nColorIdxType, &Attr.nDiffuse) ||
					(u32)Attr.nDiffuse >= pVB->nDiffuseCount)
					return FALSE;
			}

			for (u32 nST = 0; nST < nSTCount; nST++)
			{
				if ((u32)(pEnd - pCursor) < 2)
					return FALSE;
				Attr.anST[nST] = _ReadBE16(pCursor);
				pCursor += 2;
				if (!_RangeArray(_Offset(pVB->pST), (u32)Attr.anST[nST] + 1, sizeof(FGCST16_t), nFileBytes))
					return FALSE;
			}

			if (pWrite)
			{
				if (pWrite->nWrittenVertices >= 65535)
					return FALSE;
				const u16 nRow = (u16)pWrite->nWrittenVertices++;
				anCommandRows.push_back(nRow);
				const u32 nPositionOffset = _Offset(pVB->pPosition) + (u32)Attr.nPosition * pVB->nPosStride;
				if (!_Range(nPositionOffset, pVB->nPosStride, nFileBytes))
					return FALSE;
				const u8 *pPosition = (const u8 *)_At(pFile, nPositionOffset);

				f32 fPosition[3] = {0.0f, 0.0f, 0.0f};
				const f32 fPosScale = (f32)ldexp(1.0, -(int)pVB->nPosFrac);
				if (pVB->nPosType == GX_S8)
				{
					if (pVB->nPosStride < 3)
						return FALSE;
					for (u32 k = 0; k < 3; k++)
						fPosition[k] = (f32)(s8)pPosition[k] * fPosScale;
				}
				else if (pVB->nPosType == GX_S16)
				{
					if (pVB->nPosStride < 6)
						return FALSE;
					for (u32 k = 0; k < 3; k++)
						fPosition[k] = (f32)(s16)_ReadBE16(pPosition + k * 2) * fPosScale;
				}
				else
				{
					if (pVB->nPosStride < 12)
						return FALSE;
					for (u32 k = 0; k < 3; k++)
					{
						const u32 nBits = _ReadBE32(pPosition + k * 4);
						memcpy(&fPosition[k], &nBits, sizeof(f32));
						if (!isfinite(fPosition[k]))
							return FALSE;
					}
				}

				f32 fNormal[3] = {0.0f, 1.0f, 0.0f};
				f32 fTangent[3] = {1.0f, 0.0f, 0.0f};
				f32 fBinormal[3] = {0.0f, 0.0f, 1.0f};
				if (pDL->nFlags & FGCDL_FLAGS_BUMPMAP)
				{
					if (!pVB->pNBT ||
						!_RangeArray(_Offset(pVB->pNBT), (u32)Attr.nNormal + 1, sizeof(FGCNBT8_t), nFileBytes))
						return FALSE;
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
						return FALSE;
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

				if (pWrite->nSTCount > 0)
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
	u32 nTotalVertices = 0;
	u32 nTotalIndices = 0;
	for (u32 i = 0; i < nMaterialCount; i++)
	{
		FGCMeshMaterial_t *pGCMaterial =
			(FGCMeshMaterial_t *)((u8 *)pGameCubeData + _Offset(pMaterials[i].pPlatformData));
		FGC_DLCont_t *pDLs = (FGC_DLCont_t *)((u8 *)pGameCubeData + _Offset(pGCMaterial->aDLContainer));
		for (u32 j = 0; j < pGCMaterial->nDLContCount; j++)
		{
			FGC_DLCont_t *pDL = &pDLs[j];
			if (pDL->nVBIndex >= pGCMesh->nVBCount)
				goto Reject;
			ParseStats_t Stats = {};
			if (!_WalkDL((const u8 *)pGameCubeData, nGameCubeBytes, &pVBs[pDL->nVBIndex], &pMaterials[i], pDL, &Stats,
						 NULL) ||
				Stats.nVertexCount > 65535 || Stats.nIndexCount > 65535)
				goto Reject;
			if (Plans.size() >= 255)
				goto Reject;
			DLPlan_t Plan = {};
			Plan.nMaterialIndex = i;
			Plan.nDLIndex = j;
			Plan.nVertexCount = Stats.nVertexCount;
			Plan.nIndexCount = Stats.nIndexCount;
			Plan.nVertexFormat = pMaterials[i].nBaseSTSets > 1 ? FDX8VB_TYPE_N1C1T2 : FDX8VB_TYPE_N1C1T1;
			Plans.push_back(Plan);
			nTotalVertices += Stats.nVertexCount;
			nTotalIndices += Stats.nIndexCount;
		}
	}

	{
		u32 nCurrent;
		if (!_Align4Checked(nGameCubeBytes, &nCurrent))
			goto Reject;
		u32 nDXMeshOffset, nDXMaterialsOffset, nClustersOffset, nVBsOffset, nIBsOffset, nIBCountsOffset;
		if (!_TakeBytes(&nCurrent, 1, sizeof(FDX8Mesh_t), &nDXMeshOffset) ||
			!_TakeBytes(&nCurrent, nMaterialCount, sizeof(FDX8MeshMaterial_t), &nDXMaterialsOffset) ||
			!_TakeBytes(&nCurrent, (u32)Plans.size(), sizeof(FDX8MeshCluster_t), &nClustersOffset) ||
			!_TakeBytes(&nCurrent, (u32)Plans.size(), sizeof(FDX8VB_t), &nVBsOffset) ||
			!_TakeBytes(&nCurrent, (u32)Plans.size(), sizeof(void *), &nIBsOffset) ||
			!_TakeBytes(&nCurrent, (u32)Plans.size(), sizeof(u16), &nIBCountsOffset))
			goto Reject;
		u32 nDisposableOffset;
		if (!_Align4Checked(nCurrent, &nDisposableOffset))
			goto Reject;

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
		pMesh->paCollTree = NULL;
		pMesh->nCollTreeCount = 0;
		pMesh->nMeshCollMask = 0;

		FDX8Mesh_t *pDXMesh = (FDX8Mesh_t *)(pOutput + nDXMeshOffset);
		pDXMesh->nFlags = 0;
		pDXMesh->nVBCount = (u8)Plans.size();
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
			if (!_WalkDL(pOutput, nGameCubeBytes, pVB, pMaterial, pDL, NULL, &Write) ||
				Write.nWrittenVertices != Plan.nVertexCount || Write.nWrittenIndices != Plan.nIndexCount)
				goto Reject;

			FDX8MeshCluster_t *pCluster = &pClusters[i];
			pCluster->nStripCount = 0;
			pCluster->nFlags = (pDL->nFlags & FGCDL_FLAGS_FACING_OPP_DIR_LIGHT) ? CLUSTER_FACING_OPP_DIR_LIGHT : 0;
			pCluster->nSegmentIdx = pDL->nMatrixIdx;
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
		DEVPRINTF("gcmesh: converted '%s' (%u vertices, %u triangles, %u materials).\n",
				  pszResName ? pszResName : "(unnamed)", nTotalVertices, nTotalIndices / 3, nMaterialCount);
		return TRUE;
	}

Reject:
	DEVPRINTF("gcmesh: unable to convert '%s' (unsupported data or invalid display list).\n",
			  pszResName ? pszResName : "(unnamed)");
	return FALSE;
}
