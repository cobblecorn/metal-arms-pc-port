#include "gcdata.h"

#include "fdata.h"
#include "fanim.h"
#include "fparticle.h"
#include "fres.h"
#include "fvis.h"

#include <math.h>
#include <stddef.h>
#include <string.h>

static u16 _ReadBE16( const u8 *pData )
{
	return (u16)(((u16)pData[0] << 8) | pData[1]);
}

static u32 _ReadBE32( const u8 *pData )
{
	return ((u32)pData[0] << 24) | ((u32)pData[1] << 16) | ((u32)pData[2] << 8) | (u32)pData[3];
}

static BOOL _IsRangeValid( u32 nOffset, u32 nLength, u32 nTotalBytes )
{
	return nOffset <= nTotalBytes && nLength <= nTotalBytes - nOffset;
}

static BOOL _IsArrayRangeValid( u32 nOffset, u32 nCount, u32 nStride, u32 nTotalBytes )
{
	if( nCount && (!nOffset || !nStride || nCount > 0xffffffffu / nStride) ) return FALSE;
	return _IsRangeValid( nOffset, nCount * nStride, nTotalBytes );
}

static void _ConvertBE16Array( void *pData, u32 nCount )
{
	u8 *pBytes = (u8 *)pData;
	for( u32 i=0; i<nCount; i++ )
	{
		u16 nValue = _ReadBE16( pBytes + i * sizeof(u16) );
		memcpy( pBytes + i * sizeof(u16), &nValue, sizeof(nValue) );
	}
}

static void _ConvertBE32Array( void *pData, u32 nCount )
{
	u8 *pBytes = (u8 *)pData;
	for( u32 i=0; i<nCount; i++ )
	{
		u32 nValue = _ReadBE32( pBytes + i * sizeof(u32) );
		memcpy( pBytes + i * sizeof(u32), &nValue, sizeof(nValue) );
	}
}

static BOOL _ConvertAnimation( void *pData, u32 nBytes, cchar *pszResName )
{
	if( !pData || nBytes < sizeof(FAnim_t) ) return FALSE;
	u8 *pBytes = (u8 *)pData;
	const u16 nFlags = _ReadBE16( pBytes + offsetof(FAnim_t, nFlags) );
	const u16 nBoneCount = _ReadBE16( pBytes + offsetof(FAnim_t, nBoneCount) );
	const u32 nBoneOffset = _ReadBE32( pBytes + offsetof(FAnim_t, pBoneArray) );
	const u32 nKnownFlags = FANIM_BONEFLAGS_COMP_TRANSLATION | FANIM_BONEFLAGS_COMP_ORIENTATION |
		FANIM_BONEFLAGS_8BIT_SECS | FANIM_BONEFLAGS_16BIT_SECS | FANIM_BONEFLAGS_8BIT_FRAMECOUNT;
	if( (nFlags & ~nKnownFlags) ||
		((nFlags & FANIM_BONEFLAGS_8BIT_SECS) && (nFlags & FANIM_BONEFLAGS_16BIT_SECS)) ||
		!nBoneCount || nBoneCount > FDATA_MAX_BONE_COUNT ||
		!_IsArrayRangeValid( nBoneOffset, nBoneCount, sizeof(FAnimBone_t), nBytes ) ||
		nBoneOffset < sizeof(FAnim_t) )
	{
		DEVPRINTF( "gcdata: invalid GameCube animation header in '%s'.\n", pszResName ? pszResName : "(unnamed)" );
		return FALSE;
	}

	const u32 nBoneArrayEnd = nBoneOffset + (u32)nBoneCount * sizeof(FAnimBone_t);
	const u32 nTimeStride = (nFlags & FANIM_BONEFLAGS_8BIT_SECS) ? 1 :
		((nFlags & FANIM_BONEFLAGS_16BIT_SECS) ? 2 : 4);
	const u32 nTransStride = (nFlags & FANIM_BONEFLAGS_COMP_TRANSLATION) ? 6 : 12;
	const u32 nOrientStride = (nFlags & FANIM_BONEFLAGS_COMP_ORIENTATION) ? 8 : 16;
	struct _TrackRange_t { u32 nStart, nEnd; };
	_TrackRange_t aTrackRange[FDATA_MAX_BONE_COUNT * 6];
	u32 nTrackRangeCount = 0;

	for( u32 i=0; i<nBoneCount; i++ )
	{
		const u32 nBonePos = nBoneOffset + i * sizeof(FAnimBone_t);
		const u32 nNameOffset = _ReadBE32( pBytes + nBonePos + offsetof(FAnimBone_t, pszName) );
		const u16 anKeyCount[3] = {
			_ReadBE16( pBytes + nBonePos + offsetof(FAnimBone_t, nSKeyCount) ),
			_ReadBE16( pBytes + nBonePos + offsetof(FAnimBone_t, nTKeyCount) ),
			_ReadBE16( pBytes + nBonePos + offsetof(FAnimBone_t, nOKeyCount) )
		};
		const u32 anTimeOffset[3] = {
			_ReadBE32( pBytes + nBonePos + offsetof(FAnimBone_t, paSKeyUnitTime) ),
			_ReadBE32( pBytes + nBonePos + offsetof(FAnimBone_t, paTKeyUnitTime) ),
			_ReadBE32( pBytes + nBonePos + offsetof(FAnimBone_t, paOKeyUnitTime) )
		};
		const u32 anDataOffset[3] = {
			_ReadBE32( pBytes + nBonePos + offsetof(FAnimBone_t, paSKeyData) ),
			_ReadBE32( pBytes + nBonePos + offsetof(FAnimBone_t, paTKeyData) ),
			_ReadBE32( pBytes + nBonePos + offsetof(FAnimBone_t, paOKeyData) )
		};
		const u32 anDataStride[3] = { 4, nTransStride, nOrientStride };

		if( !nNameOffset || nNameOffset < nBoneArrayEnd || nNameOffset >= nBytes ||
			!memchr( pBytes + nNameOffset, 0, nBytes - nNameOffset ) )
		{
			DEVPRINTF( "gcdata: invalid bone name in GameCube animation '%s' (bone %u).\n", pszResName ? pszResName : "(unnamed)", i );
			return FALSE;
		}

		for( u32 k=0; k<3; k++ )
		{
			const u32 nCount = anKeyCount[k];
			const u32 nTimeBytes = nCount * nTimeStride;
			const u32 nDataBytes = nCount * anDataStride[k];
			u32 nDataAlignment = 4;
			if( k == 1 && (nFlags & FANIM_BONEFLAGS_COMP_TRANSLATION) ) nDataAlignment = 2;
			if( k == 2 && (nFlags & FANIM_BONEFLAGS_COMP_ORIENTATION) ) nDataAlignment = 2;
			if( nCount < 2 || ((nFlags & FANIM_BONEFLAGS_8BIT_FRAMECOUNT) && nCount >= 256) ||
				!_IsArrayRangeValid( anTimeOffset[k], nCount, nTimeStride, nBytes ) ||
				!_IsArrayRangeValid( anDataOffset[k], nCount, anDataStride[k], nBytes ) ||
				anTimeOffset[k] < nBoneArrayEnd || anDataOffset[k] < nBoneArrayEnd ||
				(anTimeOffset[k] % nTimeStride) || (anDataOffset[k] % nDataAlignment) )
			{
				DEVPRINTF( "gcdata: invalid key data in GameCube animation '%s' (bone %u track %u).\n", pszResName ? pszResName : "(unnamed)", i, k );
				return FALSE;
			}
			aTrackRange[nTrackRangeCount].nStart = anTimeOffset[k];
			aTrackRange[nTrackRangeCount++].nEnd = anTimeOffset[k] + nTimeBytes;
			aTrackRange[nTrackRangeCount].nStart = anDataOffset[k];
			aTrackRange[nTrackRangeCount++].nEnd = anDataOffset[k] + nDataBytes;
		}
	}

	for( u32 i=0; i<nTrackRangeCount; i++ )
		for( u32 j=i+1; j<nTrackRangeCount; j++ )
			if( aTrackRange[i].nStart < aTrackRange[j].nEnd && aTrackRange[j].nStart < aTrackRange[i].nEnd )
			{
				DEVPRINTF( "gcdata: overlapping key arrays in GameCube animation '%s'.\n", pszResName ? pszResName : "(unnamed)" );
				return FALSE;
			}

	FAnim_t *pAnim = (FAnim_t *)pData;
	pAnim->ChangeEndian();
	FAnimBone_t *pBoneArray = (FAnimBone_t *)(pBytes + nBoneOffset);
	for( u32 i=0; i<nBoneCount; i++ )
	{
		FAnimBone_t *pBone = &pBoneArray[i];
		const u32 nBonePos = nBoneOffset + i * sizeof(FAnimBone_t);
		const u16 anKeyCount[3] = {
			_ReadBE16( pBytes + nBonePos + offsetof(FAnimBone_t, nSKeyCount) ),
			_ReadBE16( pBytes + nBonePos + offsetof(FAnimBone_t, nTKeyCount) ),
			_ReadBE16( pBytes + nBonePos + offsetof(FAnimBone_t, nOKeyCount) )
		};
		const u32 anTimeOffset[3] = {
			_ReadBE32( pBytes + nBonePos + offsetof(FAnimBone_t, paSKeyUnitTime) ),
			_ReadBE32( pBytes + nBonePos + offsetof(FAnimBone_t, paTKeyUnitTime) ),
			_ReadBE32( pBytes + nBonePos + offsetof(FAnimBone_t, paOKeyUnitTime) )
		};
		const u32 anDataOffset[3] = {
			_ReadBE32( pBytes + nBonePos + offsetof(FAnimBone_t, paSKeyData) ),
			_ReadBE32( pBytes + nBonePos + offsetof(FAnimBone_t, paTKeyData) ),
			_ReadBE32( pBytes + nBonePos + offsetof(FAnimBone_t, paOKeyData) )
		};
		pBone->ChangeEndian();

		if( nTimeStride == 2 )
			for( u32 k=0; k<3; k++ ) _ConvertBE16Array( pBytes + anTimeOffset[k], anKeyCount[k] );
		else if( nTimeStride == 4 )
			for( u32 k=0; k<3; k++ ) _ConvertBE32Array( pBytes + anTimeOffset[k], anKeyCount[k] );

		_ConvertBE32Array( pBytes + anDataOffset[0], anKeyCount[0] );
		if( nFlags & FANIM_BONEFLAGS_COMP_TRANSLATION ) _ConvertBE16Array( pBytes + anDataOffset[1], anKeyCount[1] * 3 );
		else _ConvertBE32Array( pBytes + anDataOffset[1], anKeyCount[1] * 3 );
		if( nFlags & FANIM_BONEFLAGS_COMP_ORIENTATION ) _ConvertBE16Array( pBytes + anDataOffset[2], anKeyCount[2] * 4 );
		else _ConvertBE32Array( pBytes + anDataOffset[2], anKeyCount[2] * 4 );
	}

	static u32 nLoggedAnimations = 0;
	if( nLoggedAnimations < 12 )
	{
		DEVPRINTF( "gcdata: converted GameCube animation '%s' (%u bones, flags 0x%04x).\n",
			pszResName ? pszResName : "(unnamed)", nBoneCount, nFlags );
		nLoggedAnimations++;
	}
	return TRUE;
}

static BOOL _ConvertCsv( void *pData, u32 nBytes )
{
	u8 *pBytes = (u8 *)pData;
	if( !pBytes || nBytes < sizeof(FDataGamFile_Header_t) ) return FALSE;

	const u32 nSerializedBytes = _ReadBE32( pBytes + offsetof(FDataGamFile_Header_t, nBytesInFile) );
	const u32 nNumTables = _ReadBE32( pBytes + offsetof(FDataGamFile_Header_t, nNumTables) );
	const u32 nTablesOffset = _ReadBE32( pBytes + offsetof(FDataGamFile_Header_t, paTables) );
	if( nSerializedBytes != nBytes || nNumTables == 0 || nNumTables > FDATA_GAMFILE_MAX_TABLES ||
		!_IsRangeValid( nTablesOffset, nNumTables * sizeof(FDataGamFile_Table_t), nBytes ) )
	{
		DEVPRINTF( "gcdata: invalid GameCube CSV header (bytes=%u/%u tables=%u offset=0x%08x).\n",
			nSerializedBytes, nBytes, nNumTables, nTablesOffset );
		return FALSE;
	}

	FDataGamFile_Header_t *pHeader = (FDataGamFile_Header_t *)pData;
	pHeader->ChangeEndian();
	pHeader->nFlags = FDATA_GAMFILE_FLAGS_NONE;

	FDataGamFile_Table_t *pTables = (FDataGamFile_Table_t *)(pBytes + nTablesOffset);
	for( u32 i=0; i<nNumTables; i++ )
	{
		const u16 nFields = _ReadBE16( pBytes + nTablesOffset + i * sizeof(FDataGamFile_Table_t) + offsetof(FDataGamFile_Table_t, nNumFields) );
		const u32 nFieldsOffset = _ReadBE32( pBytes + nTablesOffset + i * sizeof(FDataGamFile_Table_t) + offsetof(FDataGamFile_Table_t, paFields) );
		const u32 nKeyOffset = _ReadBE32( pBytes + nTablesOffset + i * sizeof(FDataGamFile_Table_t) + offsetof(FDataGamFile_Table_t, pszKeyString) );
		if( nFields > FDATA_GAMFILE_MAX_FIELDS_PER_TABLE ||
			!_IsRangeValid( nFieldsOffset, (u32)nFields * sizeof(FDataGamFile_Field_t), nBytes ) ||
			(nKeyOffset && nKeyOffset >= nBytes) )
		{
			DEVPRINTF( "gcdata: invalid GameCube CSV table %u.\n", i );
			return FALSE;
		}

		pTables[i].ChangeEndian();
		FDataGamFile_Field_t *pFields = (FDataGamFile_Field_t *)(pBytes + nFieldsOffset);
		for( u32 j=0; j<nFields; j++ )
		{
			const u32 nFieldOffset = nFieldsOffset + j * sizeof(FDataGamFile_Field_t);
			const u32 nDataType = _ReadBE32( pBytes + nFieldOffset + offsetof(FDataGamFile_Field_t, nDataType) );
			if( nDataType >= FDATA_GAMEFILE_DATA_TYPE_COUNT )
			{
				DEVPRINTF( "gcdata: invalid GameCube CSV field type %u at table %u field %u.\n", nDataType, i, j );
				return FALSE;
			}

			if( nDataType == FDATA_GAMEFILE_DATA_TYPE_WIDESTRING )
			{
				const u32 nStringOffset = _ReadBE32( pBytes + nFieldOffset + offsetof(FDataGamFile_Field_t, pwszValue) );
				const u32 nStringLength = _ReadBE32( pBytes + nFieldOffset + offsetof(FDataGamFile_Field_t, nStringLen) );
				if( nStringLength > 0x7fffffffu || ! _IsRangeValid( nStringOffset, nStringLength * 2, nBytes ) )
				{
					DEVPRINTF( "gcdata: invalid GameCube CSV wide string at table %u field %u.\n", i, j );
					return FALSE;
				}
				for( u32 k=0; k<nStringLength; k++ )
				{
					u8 *pChar = pBytes + nStringOffset + k * 2;
					u8 nTemp = pChar[0];
					pChar[0] = pChar[1];
					pChar[1] = nTemp;
				}
			}

			pFields[j].ChangeEndian( (u32)pData );
		}
	}

	return TRUE;
}

static BOOL _ConvertFpr( void *pData, u32 nBytes, cchar *pszResName )
{
	u8 *pBytes = (u8 *)pData;
	if( !pBytes || nBytes < sizeof(u32) ) return FALSE;

	const u32 nVersion = _ReadBE32( pBytes );
	if( nVersion == FPARTICLE_FILE_VERSION && nBytes == sizeof(FParticleDef_t) )
	{
		((FParticleDef_t *)pData)->ChangeEndian();
		return TRUE;
	}

	// Retail GC particles are version 8. The serialized structure adds one
	// 8-byte field to each keyframe; the old fields after each keyframe retain
	// their order. Remove those additions so the v7 runtime layout can consume
	// the file. Validate the shifted texture name and invariant tail first.
	const u32 nV7TextureOffset = (u32)offsetof(FParticleDef_t, szTextureName);
	const u32 nV7TailOffset = (u32)offsetof(FParticleDef_t, fSecsBetweenLightSamples);
	const u32 nKeyFrameBytes = sizeof(FParticleKeyFrame_t);
	const u32 nHousekeepingBytes = sizeof(u32) + sizeof(FLink_t) + sizeof(FLinkRoot_t);
	const u32 nLegacySuffixBytes = sizeof(f32) + sizeof(CFVec3) + nHousekeepingBytes;
	if( nVersion != FPARTICLE_FILE_VERSION + 1 ||
		nBytes != sizeof(FParticleDef_t) + 16 ||
		nV7TextureOffset != 2 * sizeof(u32) + 2 * nKeyFrameBytes ||
		nV7TailOffset > sizeof(FParticleDef_t) ||
		nLegacySuffixBytes != sizeof(FParticleDef_t) - nV7TailOffset )
	{
		DEVPRINTF( "gcdata: unsupported GameCube particle format in '%s' (version %u, %u bytes).\n",
			pszResName ? pszResName : "(unnamed)", nVersion, nBytes );
		return FALSE;
	}

	const u32 nV8TextureOffset = nV7TextureOffset + 16;
	const u32 nTextureNameBytes = FDATA_TEXNAME_LEN + 1;
	if( !_IsRangeValid( nV8TextureOffset, nTextureNameBytes, nBytes ) ||
		!memchr( pBytes + nV8TextureOffset, 0, nTextureNameBytes ) )
	{
		DEVPRINTF( "gcdata: invalid shifted texture name in GameCube particle '%s'.\n",
			pszResName ? pszResName : "(unnamed)" );
		return FALSE;
	}

	const u32 nV8TailOffset = nV7TailOffset + 16;
	const f32 fSamplePeriod = fang_ConvertEndian( *(const f32 *)(pBytes + nV8TailOffset) );
	if( !isfinite( fSamplePeriod ) || fSamplePeriod < (1.0f / 20.0f) || fSamplePeriod > 1.0f )
	{
		DEVPRINTF( "gcdata: invalid GameCube particle sampling period in '%s'.\n",
			pszResName ? pszResName : "(unnamed)" );
		return FALSE;
	}

	const u32 nHousekeepingOffset = nV8TailOffset + sizeof(f32) + sizeof(CFVec3);
	for( u32 i = 0; i < nHousekeepingBytes; i++ )
	{
		if( pBytes[nHousekeepingOffset + i] != 0 )
		{
			DEVPRINTF( "gcdata: unsupported nonzero GameCube particle housekeeping data in '%s'.\n",
				pszResName ? pszResName : "(unnamed)" );
			return FALSE;
		}
	}

	const u32 nMinExtensionOffset = 2 * sizeof(u32) + nKeyFrameBytes;
	const u32 nMaxExtensionOffsetV8 = nMinExtensionOffset + 8 + nKeyFrameBytes;
	for( u32 i = 0; i < 8; i++ )
	{
		if( pBytes[nMinExtensionOffset + i] != 0 || pBytes[nMaxExtensionOffsetV8 + i] != 0 )
		{
			DEVPRINTF( "gcdata: unsupported nonzero GameCube particle keyframe extension in '%s'.\n",
				pszResName ? pszResName : "(unnamed)" );
			return FALSE;
		}
	}

	memmove( pBytes + nMinExtensionOffset, pBytes + nMinExtensionOffset + 8,
		nBytes - (nMinExtensionOffset + 8) );
	const u32 nMaxExtensionOffset = 2 * sizeof(u32) + 2 * nKeyFrameBytes;
	const u32 nBytesAfterMinCompaction = nBytes - 8;
	memmove( pBytes + nMaxExtensionOffset, pBytes + nMaxExtensionOffset + 8,
		nBytesAfterMinCompaction - (nMaxExtensionOffset + 8) );
	pBytes[0] = (u8)(FPARTICLE_FILE_VERSION >> 24);
	pBytes[1] = (u8)(FPARTICLE_FILE_VERSION >> 16);
	pBytes[2] = (u8)(FPARTICLE_FILE_VERSION >> 8);
	pBytes[3] = (u8)FPARTICLE_FILE_VERSION;
	((FParticleDef_t *)pData)->ChangeEndian();
	return TRUE;
}

BOOL gcdata_ConvertWorldHeader( void *pData, u32 nHeaderBytes, u32 nFileBytes )
{
	if( !pData || nHeaderBytes != sizeof(FData_WorldFileHeader_t) ) return FALSE;
	const u8 *pBytes = (const u8 *)pData;
	const u32 nSerializedBytes = _ReadBE32( pBytes + offsetof(FData_WorldFileHeader_t, nNumBytes) );
	const u32 nNumMeshes = _ReadBE32( pBytes + offsetof(FData_WorldFileHeader_t, nNumMeshes) );
	const u32 nMeshInitOffset = _ReadBE32( pBytes + offsetof(FData_WorldFileHeader_t, nOffsetToMeshInits) );
	const u32 nMeshSizeOffset = _ReadBE32( pBytes + offsetof(FData_WorldFileHeader_t, nOffsetToMeshSizes) );
	const u32 nMeshBytes = _ReadBE32( pBytes + offsetof(FData_WorldFileHeader_t, nMeshBytes) );
	const u32 nWorldOffset = _ReadBE32( pBytes + offsetof(FData_WorldFileHeader_t, nWorldOffset) );
	const u32 nWorldBytes = _ReadBE32( pBytes + offsetof(FData_WorldFileHeader_t, nWorldBytes) );
	const u32 nStreamingOffset = _ReadBE32( pBytes + offsetof(FData_WorldFileHeader_t, nStreamingDataOffset) );
	const u32 nStreamingBytes = _ReadBE32( pBytes + offsetof(FData_WorldFileHeader_t, nStreamingDataBytes) );
	const u32 nInitOffset = _ReadBE32( pBytes + offsetof(FData_WorldFileHeader_t, nInitOffsets) );
	const u32 nInitBytes = _ReadBE32( pBytes + offsetof(FData_WorldFileHeader_t, nInitBytes) );
	if( nSerializedBytes != nFileBytes || nNumMeshes > nFileBytes / sizeof(u32) ||
		!_IsArrayRangeValid( nMeshInitOffset, nNumMeshes, sizeof(u32), nFileBytes ) ||
		!_IsArrayRangeValid( nMeshSizeOffset, nNumMeshes, sizeof(u32), nFileBytes ) ||
		!_IsRangeValid( nWorldOffset, nWorldBytes, nFileBytes ) ||
		!_IsRangeValid( nStreamingOffset, nStreamingBytes, nFileBytes ) ||
		!_IsRangeValid( nInitOffset, nInitBytes, nFileBytes ) || nMeshBytes > nFileBytes )
	{
		DEVPRINTF( "gcdata: invalid GameCube world header (bytes=%u/%u meshes=%u).\n",
			nSerializedBytes, nFileBytes, nNumMeshes );
		return FALSE;
	}

	((FData_WorldFileHeader_t *)pData)->ChangeEndian();
	return TRUE;
}

BOOL gcdata_ConvertWorldVisData( void *pData, u32 nBytes )
{
	u8 *pBytes = (u8 *)pData;
	if( !pBytes || nBytes < sizeof(FVisData_t) ) return FALSE;

	const u32 nPortalCount = _ReadBE16( pBytes + offsetof(FVisData_t, nPortalCount) );
	const u32 nCellCount = _ReadBE16( pBytes + offsetof(FVisData_t, nCellCount) );
	const u32 nVolumeCount = _ReadBE16( pBytes + offsetof(FVisData_t, nVolumeCount) );
	const u32 nLightCount = _ReadBE16( pBytes + offsetof(FVisData_t, nLightCount) );
	const u32 nTreeNodeCount = _ReadBE16( pBytes + offsetof(FVisData_t, CellTree) + offsetof(FVisCellTree_t, nNodeCount) );
	const u32 nTreeOffset = _ReadBE32( pBytes + offsetof(FVisData_t, CellTree) + offsetof(FVisCellTree_t, paNodes) );
	const u32 nPortalOffset = _ReadBE32( pBytes + offsetof(FVisData_t, paPortals) );
	const u32 nVolumeOffset = _ReadBE32( pBytes + offsetof(FVisData_t, paVolumes) );
	const u32 nCellOffset = _ReadBE32( pBytes + offsetof(FVisData_t, paCells) );
	const u32 nLightOffset = _ReadBE32( pBytes + offsetof(FVisData_t, paLights) );

	if( nPortalCount > FVIS_MAX_PORTAL_COUNT || nVolumeCount > FVIS_MAX_VOLUME_COUNT ||
		nLightCount > FVIS_MAX_LIGHTS ||
		!_IsArrayRangeValid( nTreeOffset, nTreeNodeCount, sizeof(FVisCellTreeNode_t), nBytes ) ||
		!_IsArrayRangeValid( nPortalOffset, nPortalCount, sizeof(FVisPortal_t), nBytes ) ||
		!_IsArrayRangeValid( nVolumeOffset, nVolumeCount, sizeof(FVisVolume_t), nBytes ) ||
		!_IsArrayRangeValid( nCellOffset, nCellCount, sizeof(FVisCell_t), nBytes ) ||
		!_IsArrayRangeValid( nLightOffset, nLightCount, sizeof(FLightInit_t), nBytes ))
	{
		DEVPRINTF( "gcdata: invalid GameCube world visibility header (portals=%u volumes=%u cells=%u).\n",
			nPortalCount, nVolumeCount, nCellCount );
		return FALSE;
	}

	for( u32 i = 0; i < nVolumeCount; i++ )
	{
		const u8 *pVolume = pBytes + nVolumeOffset + i * sizeof(FVisVolume_t);
		const u32 nPortalIndices = _ReadBE32( pVolume + offsetof(FVisVolume_t, paPortalIndices) );
		const u32 nVolumePortals = pVolume[offsetof(FVisVolume_t, nPortalCount)];
		if( !_IsArrayRangeValid( nPortalIndices, nVolumePortals, sizeof(u16), nBytes ) )
		{
			DEVPRINTF( "gcdata: invalid GameCube world volume portal list %u.\n", i );
			return FALSE;
		}
	}
	for( u32 i = 0; i < nCellCount; i++ )
	{
		const u8 *pCell = pBytes + nCellOffset + i * sizeof(FVisCell_t);
		const u32 nPlanes = pCell[offsetof(FVisCell_t, nPlaneCount)];
		const u32 nPlaneOffset = _ReadBE32( pCell + offsetof(FVisCell_t, paBoundingPlanes) );
		if( !_IsArrayRangeValid( nPlaneOffset, nPlanes, sizeof(FVisPlane_t), nBytes ) )
		{
			DEVPRINTF( "gcdata: invalid GameCube world cell plane list %u.\n", i );
			return FALSE;
		}
	}

	FVisData_t *pVisData = (FVisData_t *)pData;
	pVisData->ChangeEndian();
	FVisCellTreeNode_t *pTreeNodes = nTreeNodeCount ? (FVisCellTreeNode_t *)(pBytes + nTreeOffset) : NULL;
	for( u32 i = 0; i < nTreeNodeCount; i++ ) pTreeNodes[i].ChangeEndian();
	FVisPortal_t *pPortals = nPortalCount ? (FVisPortal_t *)(pBytes + nPortalOffset) : NULL;
	for( u32 i = 0; i < nPortalCount; i++ ) pPortals[i].ChangeEndian();
	FVisVolume_t *pVolumes = nVolumeCount ? (FVisVolume_t *)(pBytes + nVolumeOffset) : NULL;
	for( u32 i = 0; i < nVolumeCount; i++ )
	{
		FVisVolume_t *pVolume = &pVolumes[i];
		const u32 nPortalIndices = _ReadBE32( (const u8 *)pVolume + offsetof(FVisVolume_t, paPortalIndices) );
		const u32 nVolumePortals = ((const u8 *)pVolume)[offsetof(FVisVolume_t, nPortalCount)];
		u16 *pIndices = nVolumePortals ? (u16 *)(pBytes + nPortalIndices) : NULL;
		for( u32 j = 0; j < nVolumePortals; j++ ) pIndices[j] = fang_ConvertEndian( pIndices[j] );
		pVolume->ChangeEndian();
	}
	FVisCell_t *pCells = nCellCount ? (FVisCell_t *)(pBytes + nCellOffset) : NULL;
	for( u32 i = 0; i < nCellCount; i++ )
	{
		FVisCell_t *pCell = &pCells[i];
		const u32 nPlanes = ((const u8 *)pCell)[offsetof(FVisCell_t, nPlaneCount)];
		const u32 nPlaneOffset = _ReadBE32( (const u8 *)pCell + offsetof(FVisCell_t, paBoundingPlanes) );
		FVisPlane_t *pPlanes = nPlanes ? (FVisPlane_t *)(pBytes + nPlaneOffset) : NULL;
		for( u32 j = 0; j < nPlanes; j++ ) pPlanes[j].ChangeEndian();
		pCell->ChangeEndian();
	}
	FLightInit_t *pLights = nLightCount ? (FLightInit_t *)(pBytes + nLightOffset) : NULL;
	for( u32 i = 0; i < nLightCount; i++ ) pLights[i].ChangeEndian();
	DEVPRINTF( "gcdata: converted GameCube world visibility (%u portals, %u volumes, %u cells).\n",
		nPortalCount, nVolumeCount, nCellCount );
	return TRUE;
}

BOOL gcdata_ConvertWorldInitData( void *pData, u32 nBytes )
{
	u8 *pBytes = (u8 *)pData;
	if( !pBytes || nBytes < sizeof(FData_WorldInitHeader_t) ) return FALSE;
	const u32 nShapeCount = _ReadBE32( pBytes + offsetof(FData_WorldInitHeader_t, nNumInitStructs) );
	const u32 nShapeArrayOffset = sizeof(FData_WorldInitHeader_t);
	if( nShapeCount > (nBytes - nShapeArrayOffset) / sizeof(CFWorldShapeInit) )
	{
		DEVPRINTF( "gcdata: invalid GameCube world init header (shapes=%u bytes=%u).\n", nShapeCount, nBytes );
		return FALSE;
	}
	const u32 nFixedShapeBytes = nShapeCount * sizeof(CFWorldShapeInit);
	const u32 nShapeDataBytes = nBytes - nShapeArrayOffset;
	for( u32 i = 0; i < nShapeCount; i++ )
	{
		const u8 *pShape = pBytes + nShapeArrayOffset + i * sizeof(CFWorldShapeInit);
		const u32 nType = _ReadBE32( pShape + offsetof(CFWorldShapeInit, m_nShapeType) );
		const u32 nShapeOffset = _ReadBE32( pShape + offsetof(CFWorldShapeInit, m_pShape) );
		const u32 nGameDataOffset = _ReadBE32( pShape + offsetof(CFWorldShapeInit, m_pGameData) );
		u32 nShapeBytes = 0;
		if( nType >= FWORLD_SHAPETYPE_COUNT )
		{
			DEVPRINTF( "gcdata: invalid GameCube world shape type %u at %u.\n", nType, i );
			return FALSE;
		}
		switch( nType )
		{
			case FWORLD_SHAPETYPE_POINT: nShapeBytes = 0; break;
			case FWORLD_SHAPETYPE_LINE: nShapeBytes = sizeof(CFWorldShapeLine); break;
			case FWORLD_SHAPETYPE_SPLINE: nShapeBytes = sizeof(CFWorldShapeSpline); break;
			case FWORLD_SHAPETYPE_BOX: nShapeBytes = sizeof(CFWorldShapeBox); break;
			case FWORLD_SHAPETYPE_SPHERE: nShapeBytes = sizeof(CFWorldShapeSphere); break;
			case FWORLD_SHAPETYPE_CYLINDER: nShapeBytes = sizeof(CFWorldShapeCylinder); break;
			case FWORLD_SHAPETYPE_MESH: nShapeBytes = sizeof(CFWorldShapeMesh); break;
		}
		if( nShapeBytes && (nShapeOffset < nFixedShapeBytes ||
			!_IsArrayRangeValid( nShapeOffset, 1, nShapeBytes, nShapeDataBytes )) )
		{
			DEVPRINTF( "gcdata: invalid GameCube world shape data offset %u at %u.\n", nShapeOffset, i );
			return FALSE;
		}
		if( nGameDataOffset && (nGameDataOffset < nFixedShapeBytes ||
			!_IsArrayRangeValid( nGameDataOffset, 1, sizeof(FDataGamFile_Header_t), nShapeDataBytes )) )
		{
			DEVPRINTF( "gcdata: invalid GameCube world shape game data offset %u at %u.\n", nGameDataOffset, i );
			return FALSE;
		}
		if( nType == FWORLD_SHAPETYPE_SPLINE )
		{
			const u8 *pSpline = pBytes + nShapeArrayOffset + nShapeOffset;
			const u32 nPointCount = _ReadBE32( pSpline + offsetof(CFWorldShapeSpline, m_nPointCount) );
			const u32 nPointOffset = _ReadBE32( pSpline + offsetof(CFWorldShapeSpline, m_pPtArray) );
			if( (nPointCount && nPointOffset < nFixedShapeBytes) ||
				!_IsArrayRangeValid( nPointOffset, nPointCount, sizeof(CFVec3), nShapeDataBytes ) )
			{
				DEVPRINTF( "gcdata: invalid GameCube world spline points at shape %u.\n", i );
				return FALSE;
			}
		}
		if( nType == FWORLD_SHAPETYPE_MESH )
		{
			const u8 *pMesh = pBytes + nShapeArrayOffset + nShapeOffset;
			const u32 nStreamCount = pMesh[offsetof(CFWorldShapeMesh, m_nColorStreamCount)];
			const u32 nStreamOffset = _ReadBE32( pMesh + offsetof(CFWorldShapeMesh, m_paColorStreams) );
			if( nStreamCount > 32 || (nStreamCount && nStreamOffset < nFixedShapeBytes) ||
				!_IsArrayRangeValid( nStreamOffset, nStreamCount, sizeof(ColorStream_t), nShapeDataBytes ) )
			{
				DEVPRINTF( "gcdata: invalid GameCube world mesh color streams at shape %u.\n", i );
				return FALSE;
			}
			for( u32 j = 0; j < nStreamCount; j++ )
			{
				const u8 *pStream = pBytes + nShapeArrayOffset + nStreamOffset + j * sizeof(ColorStream_t);
				const u32 nColorCount = _ReadBE16( pStream + offsetof(ColorStream_t, nColorCount) );
				const u32 nColorOffset = _ReadBE32( pStream + offsetof(ColorStream_t, paVertexColors) );
				if( (nColorCount && nColorOffset < nFixedShapeBytes) ||
					!_IsArrayRangeValid( nColorOffset, nColorCount, sizeof(u32), nShapeDataBytes ) )
				{
					DEVPRINTF( "gcdata: invalid GameCube world mesh vertex colors at shape %u.\n", i );
					return FALSE;
				}
			}
		}
	}

	FData_WorldInitHeader_t *pHeader = (FData_WorldInitHeader_t *)pData;
	CFWorldShapeInit *pShapes = (CFWorldShapeInit *)(pHeader + 1);
	for( u32 i = 0; i < nShapeCount; i++ )
	{
		CFWorldShapeInit *pShape = &pShapes[i];
		const u32 nType = _ReadBE32( (const u8 *)pShape + offsetof(CFWorldShapeInit, m_nShapeType) );
		const u32 nShapeOffset = _ReadBE32( (const u8 *)pShape + offsetof(CFWorldShapeInit, m_pShape) );
		const u32 nGameDataOffset = _ReadBE32( (const u8 *)pShape + offsetof(CFWorldShapeInit, m_pGameData) );
		if( nShapeOffset )
		{
			u8 *pShapeData = (u8 *)pShapes + nShapeOffset;
			switch( nType )
			{
				case FWORLD_SHAPETYPE_LINE:
					((CFWorldShapeLine *)pShapeData)->ChangeEndian();
					break;
				case FWORLD_SHAPETYPE_SPLINE:
				{
					CFWorldShapeSpline *pSpline = (CFWorldShapeSpline *)pShapeData;
					const u32 nPointCount = _ReadBE32( (const u8 *)pSpline + offsetof(CFWorldShapeSpline, m_nPointCount) );
					const u32 nPointOffset = _ReadBE32( (const u8 *)pSpline + offsetof(CFWorldShapeSpline, m_pPtArray) );
					CFVec3 *pPoints = nPointCount ? (CFVec3 *)((u8 *)pShapes + nPointOffset) : NULL;
					for( u32 j = 0; j < nPointCount; j++ ) pPoints[j].ChangeEndian();
					pSpline->ChangeEndian();
					break;
				}
				case FWORLD_SHAPETYPE_BOX: ((CFWorldShapeBox *)pShapeData)->ChangeEndian(); break;
				case FWORLD_SHAPETYPE_SPHERE: ((CFWorldShapeSphere *)pShapeData)->ChangeEndian(); break;
				case FWORLD_SHAPETYPE_CYLINDER: ((CFWorldShapeCylinder *)pShapeData)->ChangeEndian(); break;
				case FWORLD_SHAPETYPE_MESH:
				{
					CFWorldShapeMesh *pMesh = (CFWorldShapeMesh *)pShapeData;
					const u32 nStreamCount = ((const u8 *)pMesh)[offsetof(CFWorldShapeMesh, m_nColorStreamCount)];
					const u32 nStreamOffset = _ReadBE32( (const u8 *)pMesh + offsetof(CFWorldShapeMesh, m_paColorStreams) );
					ColorStream_t *pStreams = nStreamCount ? (ColorStream_t *)((u8 *)pShapes + nStreamOffset) : NULL;
					for( u32 j = 0; j < nStreamCount; j++ )
					{
						ColorStream_t *pStream = &pStreams[j];
						const u32 nColorCount = _ReadBE16( (const u8 *)pStream + offsetof(ColorStream_t, nColorCount) );
						const u32 nColorOffset = _ReadBE32( (const u8 *)pStream + offsetof(ColorStream_t, paVertexColors) );
						u32 *pColors = nColorCount ? (u32 *)((u8 *)pShapes + nColorOffset) : NULL;
						for( u32 k = 0; k < nColorCount; k++ ) pColors[k] = fang_ConvertEndian( pColors[k] );
						pStream->nVBIndex = fang_ConvertEndian( pStream->nVBIndex );
						pStream->nColorCount = fang_ConvertEndian( pStream->nColorCount );
						pStream->paVertexColors = fang_ConvertEndian( pStream->paVertexColors );
					}
					pMesh->ChangeEndian();
					break;
				}
			}
		}
		if( nGameDataOffset )
		{
			const u32 nGameDataBytes = _ReadBE32( pBytes + nShapeArrayOffset + nGameDataOffset + offsetof(FDataGamFile_Header_t, nBytesInFile) );
			if( !_IsRangeValid( nGameDataOffset, nGameDataBytes, nShapeDataBytes ) ||
				!_ConvertCsv( (u8 *)pShapes + nGameDataOffset, nGameDataBytes ) )
			{
				DEVPRINTF( "gcdata: invalid GameCube world shape game data at shape %u.\n", i );
				return FALSE;
			}
		}
		pShape->ChangeEndian();
	}
	pHeader->ChangeEndian();
	DEVPRINTF( "gcdata: converted GameCube world init (%u shapes).\n", nShapeCount );
	return TRUE;
}

static u8 _Expand5( u32 nValue )
{
	return (u8)((nValue << 3) | (nValue >> 2));
}

static u8 _Expand6( u32 nValue )
{
	return (u8)((nValue << 2) | (nValue >> 4));
}

static u8 _Expand4( u32 nValue )
{
	return (u8)(nValue * 17);
}

static u32 _PackArgb( u8 nR, u8 nG, u8 nB, u8 nA )
{
	return ((u32)nA << 24) | ((u32)nR << 16) | ((u32)nG << 8) | nB;
}

static u32 _GCTileBytes( u32 nFormat, u32 nWidth, u32 nHeight )
{
	u32 nAcross, nDown, nBytesPerTile;

	switch( nFormat )
	{
	case 0: case 1: nAcross = 4; nDown = 4; nBytesPerTile = 64; break;
	case 2: case 3: case 4: case 9: nAcross = 4; nDown = 4; nBytesPerTile = 32; break;
	case 5: case 6: case 7: case 11: nAcross = 8; nDown = 8; nBytesPerTile = 32; break;
	case 8: case 10: nAcross = 8; nDown = 4; nBytesPerTile = 32; break;
	default: return 0;
	}
	return ((nWidth + nAcross - 1) / nAcross) * ((nHeight + nDown - 1) / nDown) * nBytesPerTile;
}

static void _DecodeCmprPlane( const u8 *pSource, u32 nWidth, u32 nHeight, u32 *pPixels, BOOL bAlphaPlane, BOOL bOneBitAlpha )
{
	const u32 nTilesAcross = (nWidth + 7) >> 3;
	const u32 nTilesDown = (nHeight + 7) >> 3;

	for( u32 nTileY = 0; nTileY < nTilesDown; nTileY++ )
	{
		for( u32 nTileX = 0; nTileX < nTilesAcross; nTileX++ )
		{
			const u8 *pTile = pSource + ((nTileY * nTilesAcross + nTileX) * 32);
			for( u32 nBlockY = 0; nBlockY < 2; nBlockY++ )
			{
				for( u32 nBlockX = 0; nBlockX < 2; nBlockX++ )
				{
					const u8 *pBlock = pTile + ((nBlockY * 2 + nBlockX) * 8);
					const u16 nColor0 = _ReadBE16( pBlock );
					const u16 nColor1 = _ReadBE16( pBlock + 2 );
					u8 anColor[4][4];
					anColor[0][0] = _Expand5( (nColor0 >> 11) & 31 );
					anColor[0][1] = _Expand6( (nColor0 >> 5) & 63 );
					anColor[0][2] = _Expand5( nColor0 & 31 );
					anColor[1][0] = _Expand5( (nColor1 >> 11) & 31 );
					anColor[1][1] = _Expand6( (nColor1 >> 5) & 63 );
					anColor[1][2] = _Expand5( nColor1 & 31 );
					if( nColor0 > nColor1 || !bOneBitAlpha )
					{
						for( u32 nChannel = 0; nChannel < 3; nChannel++ )
						{
							anColor[2][nChannel] = (u8)((2 * anColor[0][nChannel] + anColor[1][nChannel]) / 3);
							anColor[3][nChannel] = (u8)((anColor[0][nChannel] + 2 * anColor[1][nChannel]) / 3);
						}
					}
					else
					{
						for( u32 nChannel = 0; nChannel < 3; nChannel++ )
							anColor[2][nChannel] = (u8)((anColor[0][nChannel] + anColor[1][nChannel]) / 2);
						anColor[3][0] = anColor[3][1] = anColor[3][2] = 0;
					}

					for( u32 nY = 0; nY < 4; nY++ )
					{
						const u8 nIndices = pBlock[4 + nY];
						for( u32 nX = 0; nX < 4; nX++ )
						{
							const u32 nColorIndex = (nIndices >> (6 - nX * 2)) & 3;
							const u32 nXOut = nTileX * 8 + nBlockX * 4 + nX;
							const u32 nYOut = nTileY * 8 + nBlockY * 4 + nY;
							if( nXOut >= nWidth || nYOut >= nHeight ) continue;

							const u8 nR = anColor[nColorIndex][0];
							const u8 nG = anColor[nColorIndex][1];
							const u8 nB = anColor[nColorIndex][2];
							const u32 nIndex = nYOut * nWidth + nXOut;
							if( bAlphaPlane )
							{
								pPixels[nIndex] = (pPixels[nIndex] & 0x00ffffff) | ((u32)nG << 24);
							}
							else
							{
								const u8 nAlpha = (bOneBitAlpha && nColor0 <= nColor1 && nColorIndex == 3) ? 0 : 255;
								pPixels[nIndex] = _PackArgb( nR, nG, nB, nAlpha );
							}
						}
					}
				}
			}
		}
	}
}

static BOOL _DecodeUncompressedGCTile( u32 nFormat, const u8 *pTile, u32 nTileX, u32 nTileY, u32 nWidth, u32 nHeight, u32 *pPixels )
{
	u32 nTileAcross = 4, nTileDown = 4;
	if( nFormat == 8 || nFormat == 10 ) { nTileAcross = 8; nTileDown = 4; }
	if( nFormat == 11 ) { nTileAcross = 8; nTileDown = 8; }

	for( u32 y = 0; y < nTileDown; y++ )
	{
		for( u32 x = 0; x < nTileAcross; x++ )
		{
			const u32 nX = nTileX * nTileAcross + x;
			const u32 nY = nTileY * nTileDown + y;
			if( nX >= nWidth || nY >= nHeight ) continue;
			u8 nR = 0, nG = 0, nB = 0, nA = 255;
			switch( nFormat )
			{
			case 0: case 1:
			{
				const u32 nOffset = y * 8 + x * 2;
				nA = pTile[nOffset]; nR = pTile[nOffset + 1];
				nG = pTile[32 + nOffset]; nB = pTile[33 + nOffset];
				if( nFormat == 1 ) nA = 255;
				break;
			}
			case 2: case 3:
			{
				const u16 nColor = _ReadBE16( pTile + (y * 4 + x) * 2 );
				if( nColor & 0x8000 )
				{
					nR = _Expand5( (nColor >> 10) & 31 );
					nG = _Expand5( (nColor >> 5) & 31 );
					nB = _Expand5( nColor & 31 );
				}
				else
				{
					nA = _Expand4( (nColor >> 12) & 7 );
					nR = _Expand4( (nColor >> 8) & 15 );
					nG = _Expand4( (nColor >> 4) & 15 );
					nB = _Expand4( nColor & 15 );
				}
				if( nFormat == 3 ) nA = 255;
				break;
			}
			case 4:
			{
				const u16 nColor = _ReadBE16( pTile + (y * 4 + x) * 2 );
				nR = _Expand5( (nColor >> 11) & 31 );
				nG = _Expand6( (nColor >> 5) & 63 );
				nB = _Expand5( nColor & 31 );
				break;
			}
			case 8:
			{
				const u8 nIntensity = pTile[y * 8 + x];
				nR = nG = nB = nIntensity;
				break;
			}
			case 9:
			{
				const u32 nOffset = (y * 4 + x) * 2;
				nA = pTile[nOffset];
				nR = nG = nB = pTile[nOffset + 1];
				break;
			}
			case 10:
			{
				const u8 nIA = pTile[y * 8 + x];
				nA = _Expand4( (nIA >> 4) & 15 );
				nR = nG = nB = _Expand4( nIA & 15 );
				break;
			}
			case 11:
			{
				const u8 nPair = pTile[y * 4 + (x >> 1)];
				const u8 nIntensity = (x & 1) ? (nPair & 15) : (nPair >> 4);
				nR = nG = nB = _Expand4( nIntensity );
				break;
			}
			default:
				return FALSE;
			}
			pPixels[nY * nWidth + nX] = _PackArgb( nR, nG, nB, nA );
		}
	}
	return TRUE;
}

BOOL gcdata_DecodeTga( const void *pFileData, u32 nFileBytes, FTexInfo_t *pTexInfo, void **ppImageData, u32 *pnImageBytes )
{
#if FANG_PLATFORM_WIN && FANG_WINGC
	if( !pFileData || !pTexInfo || !ppImageData || !pnImageBytes || nFileBytes < sizeof(FTexInfo_t) ) return FALSE;

	memcpy( pTexInfo, pFileData, sizeof(FTexInfo_t) );
	pTexInfo->ChangeEndian();
	pTexInfo->pUserData = NULL;
	const u32 nFormat = pTexInfo->nTexFmt;
	if( nFormat >= FGCDATA_TEXFMT_COUNT || !pTexInfo->nLodCount || pTexInfo->nLodCount > 16 ||
		!pTexInfo->nTexelsAcross || !pTexInfo->nTexelsDown )
	{
		DEVPRINTF( "gcdata: invalid GameCube TGA header for '%s'.\n", pTexInfo->szName );
		return FALSE;
	}

	u32 anWidth[16], anHeight[16], anSourceBytes[16], anPixelOffsets[16];
	u32 nTotalSourceBytes = 0, nTotalImageBytes = 0;
	u32 nWidth = pTexInfo->nTexelsAcross, nHeight = pTexInfo->nTexelsDown;
	const u32 nMinDim = (nFormat == 5 || nFormat == 6 || nFormat == 7) ? 8 :
		((nFormat == 0 || nFormat == 1 || nFormat == 2 || nFormat == 3 || nFormat == 4 || nFormat == 8 || nFormat == 9 || nFormat == 10 || nFormat == 11) ? 4 : 0);
	for( u32 i = 0; i < pTexInfo->nLodCount; i++ )
	{
		if( nWidth > 4096 || nHeight > 4096 )
		{
			DEVPRINTF( "gcdata: oversized GameCube TGA '%s' (%ux%u).\n", pTexInfo->szName, nWidth, nHeight );
			return FALSE;
		}
		const u32 nLevelBytes = _GCTileBytes( nFormat, nWidth, nHeight );
		const u32 nPixelBytes = nWidth * nHeight * 4;
		if( !nLevelBytes || (nWidth & (nWidth - 1)) || (nHeight & (nHeight - 1)) ||
			nTotalSourceBytes > 0xffffffffu - nLevelBytes || nTotalImageBytes > 0xffffffffu - nPixelBytes )
		{
			DEVPRINTF( "gcdata: unsupported or oversized GameCube TGA '%s'.\n", pTexInfo->szName );
			return FALSE;
		}
		anWidth[i] = nWidth; anHeight[i] = nHeight;
		anSourceBytes[i] = nLevelBytes;
		anPixelOffsets[i] = nTotalImageBytes;
		nTotalSourceBytes += nLevelBytes;
		nTotalImageBytes += nPixelBytes;
		nWidth >>= 1; nHeight >>= 1;
		if( nWidth < nMinDim ) nWidth = nMinDim;
		if( nHeight < nMinDim ) nHeight = nMinDim;
	}

	const u32 nPlaneCount = (nFormat == 7) ? 2 : 1;
	if( nTotalSourceBytes > (nFileBytes - sizeof(FTexInfo_t)) / nPlaneCount )
	{
		DEVPRINTF( "gcdata: truncated GameCube TGA '%s' (%u bytes of image data expected).\n", pTexInfo->szName, nTotalSourceBytes * nPlaneCount );
		return FALSE;
	}

	u32 *pPixels = (u32 *)fmem_Alloc( nTotalImageBytes, 16 );
	if( !pPixels ) return FALSE;
	const u8 *pImage = (const u8 *)pFileData + sizeof(FTexInfo_t);
	const u8 *pAlphaPlane = pImage + nTotalSourceBytes;
	for( u32 i = 0; i < pTexInfo->nLodCount; i++ )
	{
		u32 *pMipPixels = (u32 *)((u8 *)pPixels + anPixelOffsets[i]);
		if( nFormat <= 4 || nFormat >= 8 )
		{
			const u8 *pMip = pImage;
			for( u32 j = 0; j < i; j++ ) pMip += anSourceBytes[j];
			const u32 nTileAcross = (nFormat == 8 || nFormat == 10) ? 8 : ((nFormat == 11) ? 8 : 4);
			const u32 nTileDown = (nFormat == 8 || nFormat == 10) ? 4 : ((nFormat == 11) ? 8 : 4);
			const u32 nTileBytes = (nFormat == 0 || nFormat == 1) ? 64 : 32;
			const u32 nTilesAcross = (anWidth[i] + nTileAcross - 1) / nTileAcross;
			const u32 nTilesDown = (anHeight[i] + nTileDown - 1) / nTileDown;
			for( u32 y = 0; y < nTilesDown; y++ )
				for( u32 x = 0; x < nTilesAcross; x++ )
					if( !_DecodeUncompressedGCTile( nFormat, pMip + x * nTileBytes + y * nTilesAcross * nTileBytes, x, y, anWidth[i], anHeight[i], pMipPixels ) ) return FALSE;
		}
		else
		{
			const u8 *pMip = pImage;
			for( u32 j = 0; j < i; j++ ) pMip += anSourceBytes[j];
			_DecodeCmprPlane( pMip, anWidth[i], anHeight[i], pMipPixels, FALSE, nFormat == 6 );
			if( nFormat == 7 )
			{
				const u8 *pAlphaMip = pAlphaPlane;
				for( u32 j = 0; j < i; j++ ) pAlphaMip += anSourceBytes[j];
				_DecodeCmprPlane( pAlphaMip, anWidth[i], anHeight[i], pMipPixels, TRUE, FALSE );
			}
		}
	}

	pTexInfo->nTexFmt = FTEX_FMT_A8R8G8B8;
	pTexInfo->nPalFmt = FTEX_PALFMT_NONE;
	pTexInfo->nFlags = FTEX_FLAG_NONE;
	*ppImageData = pPixels;
	*pnImageBytes = nTotalImageBytes;
	return TRUE;
#else
	pFileData; nFileBytes; pTexInfo; ppImageData; pnImageBytes;
	return FALSE;
#endif
}

// ftext.cpp's font loader (ftext_Load()) is a bespoke loader that reads a .fnt file
// straight into an fres_Alloc'd copy of FDataFntFile_Font_t and then fixes up its three
// array offsets into pointers. It never goes through fresload/gcdata_Convert(), and it
// never called ChangeEndian() on any of it (the original code never needed to: the file
// is only ever produced and consumed little-endian, on Xbox/PC). On a big-endian GameCube
// file, every field, including the array offsets, is byte-swapped nonsense until this
// runs. Call it right after the raw file bytes are read, before ftext.cpp's own
// offset-to-pointer fixup.
BOOL gcdata_ConvertFont( void *pData, u32 nBytes )
{
	if( !pData || nBytes < sizeof(FDataFntFile_Font_t) ) return FALSE;
	u8 *pBytes = (u8 *)pData;

	const u32 nBuckets = pBytes[offsetof(FDataFntFile_Font_t, uBuckets)];
	const u32 nTexPages = pBytes[offsetof(FDataFntFile_Font_t, uTexPages)];
	const u32 nBucketLetters = _ReadBE16( pBytes + offsetof(FDataFntFile_Font_t, uBucketLetters) );
	const u32 nFntLetters = _ReadBE16( pBytes + offsetof(FDataFntFile_Font_t, uFntLetters) );
	const u32 nLetterBucketsOffset = _ReadBE32( pBytes + offsetof(FDataFntFile_Font_t, paoLetterBuckets) );
	const u32 nBucketLettersOffset = _ReadBE32( pBytes + offsetof(FDataFntFile_Font_t, paoBucketLetters) );
	const u32 nFntLettersOffset = _ReadBE32( pBytes + offsetof(FDataFntFile_Font_t, paoFntLetters) );

	if( nTexPages > 64 ||
		!_IsArrayRangeValid( nLetterBucketsOffset, nBuckets, sizeof(FDataFntFile_LetterBucket_t), nBytes ) ||
		!_IsArrayRangeValid( nBucketLettersOffset, nBucketLetters, sizeof(FDataFntFile_BucketLetter_t), nBytes ) ||
		!_IsArrayRangeValid( nFntLettersOffset, nFntLetters, sizeof(FDataFntFile_Letter_t), nBytes ) )
	{
		DEVPRINTF( "gcdata: invalid GameCube font (buckets=%u bucketLetters=%u fntLetters=%u texPages=%u, %u bytes).\n",
			nBuckets, nBucketLetters, nFntLetters, nTexPages, nBytes );
		return FALSE;
	}

	// Cross-check the bucket -> bucketLetter -> fntLetter index chain while the offsets are
	// still known-good raw values: this is exactly the chain that read a wild pointer before.
	for( u32 i = 0; i < nBuckets; i++ )
	{
		const u8 *pBucket = pBytes + nLetterBucketsOffset + i * sizeof(FDataFntFile_LetterBucket_t);
		const u32 nLettersInBucket = _ReadBE16( pBucket + offsetof(FDataFntFile_LetterBucket_t, nNumLettersInBucket) );
		const u32 nBaseIndex = _ReadBE16( pBucket + offsetof(FDataFntFile_LetterBucket_t, nBucketLetterBaseIndex) );
		if( nLettersInBucket && (nBaseIndex >= nBucketLetters || nLettersInBucket > nBucketLetters - nBaseIndex) )
		{
			DEVPRINTF( "gcdata: invalid GameCube font letter bucket %u (base=%u count=%u of %u).\n", i, nBaseIndex, nLettersInBucket, nBucketLetters );
			return FALSE;
		}
	}
	for( u32 i = 0; i < nBucketLetters; i++ )
	{
		const u8 *pBL = pBytes + nBucketLettersOffset + i * sizeof(FDataFntFile_BucketLetter_t);
		const u32 nLetterIndex = _ReadBE16( pBL + offsetof(FDataFntFile_BucketLetter_t, uFntLetterIndex) );
		if( nLetterIndex >= nFntLetters )
		{
			DEVPRINTF( "gcdata: invalid GameCube font bucket-letter %u (letter index %u of %u).\n", i, nLetterIndex, nFntLetters );
			return FALSE;
		}
	}

	((FDataFntFile_Font_t *)pData)->ChangeEndian();

	FDataFntFile_LetterBucket_t *pFontBuckets = nBuckets ? (FDataFntFile_LetterBucket_t *)(pBytes + nLetterBucketsOffset) : NULL;
	for( u32 i = 0; i < nBuckets; i++ ) pFontBuckets[i].ChangeEndian();

	FDataFntFile_BucketLetter_t *pFontBucketLetters = nBucketLetters ? (FDataFntFile_BucketLetter_t *)(pBytes + nBucketLettersOffset) : NULL;
	for( u32 i = 0; i < nBucketLetters; i++ ) pFontBucketLetters[i].ChangeEndian();

	FDataFntFile_Letter_t *pFontLetters = nFntLetters ? (FDataFntFile_Letter_t *)(pBytes + nFntLettersOffset) : NULL;
	for( u32 i = 0; i < nFntLetters; i++ ) pFontLetters[i].ChangeEndian();

	DEVPRINTF( "gcdata: converted GameCube font (%u buckets, %u bucket letters, %u letters, %u texture pages).\n",
		nBuckets, nBucketLetters, nFntLetters, nTexPages );
	return TRUE;
}

BOOL gcdata_Convert( cchar *pszExtension, cchar *pszResName, void *pData, u32 nBytes )
{
#if FANG_PLATFORM_WIN && FANG_WINGC
	static u32 nLoggedResources = 0;
	if( nLoggedResources < 16 )
	{
		DEVPRINTF( "gcdata: loaded GameCube resource '%s.%s' (%u bytes).\n",
			pszResName ? pszResName : "(unnamed)", pszExtension ? pszExtension : "(unknown)", nBytes );
		nLoggedResources++;
	}
	if( pszExtension && strcmp( pszExtension, "csv" ) == 0 )
	{
		static BOOL bLoggedFirstCsv = FALSE;
		if( !_ConvertCsv( pData, nBytes ) ) return FALSE;
		if( !bLoggedFirstCsv )
		{
			DEVPRINTF( "gcdata: converted GameCube CSV resource (%u bytes).\n", nBytes );
			bLoggedFirstCsv = TRUE;
		}
	}
	else if( pszExtension && strcmp( pszExtension, "mtx" ) == 0 )
	{
		if( !_ConvertAnimation( pData, nBytes, pszResName ) ) return FALSE;
	}
	else if( pszExtension && strcmp( pszExtension, "fpr" ) == 0 )
	{
		if( !_ConvertFpr( pData, nBytes, pszResName ) ) return FALSE;
	}
#else
	pszExtension; pszResName; pData; nBytes;
#endif
	return TRUE;
}
