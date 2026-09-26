//////////////////////////////////////////////////////////////////////////////////////
// KongToMLMesh.h - 
//
// Author: Michael Starich   
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
// 03/18/02 Starich     Created.
// 11/14/02 Lafleur     Genericized for all platforms from GC file
//////////////////////////////////////////////////////////////////////////////////////
#ifndef _KONG_TO_MLMESH_H_
#define _KONG_TO_MLMESH_H_ 1

#include "fang.h"
#include "ML.h"
#include "ConvertApeFile.h"
#include "ape_file_def.h"
#include "fdata.h"
#include "TriPacketList.h"


//
//
//
class CKongToMLMesh : public CConvertApeFile
{
	public:
		CKongToMLMesh( BOOL bComputeEdgeData=TRUE );
		~CKongToMLMesh( void );

		virtual BOOL Convert( CCompileDlg *pDlg, CApeToKongFormat &Kong, cchar *pszFilename, CAidFile *pAidFile, TargetPlatform_e nPlatform, 
								BOOL bGenerateStrips, BOOL bWorld=FALSE, BOOL bVertexRadiosity=FALSE, CFVec3 *pDirLight=NULL );
		virtual u32  GetCRC( u32 nCurrentCRC );
		virtual u32  GetSizeOfConvertedFile( void );
		virtual BOOL WriteConvertedFile( cchar *pszFilename, FILE *pFileStream=NULL );
		virtual BOOL GenerateLMSTs( CCompileDlg *pDlg, KongMesh_t *pKongMesh, cchar *pszFilename );
		virtual void FreeData( void );

		MLResults m_LibResults;
		u32 m_nStrippingTime;
		u32 m_nMeshLibTime;

		u32 m_nStripCount;
		u32 m_nStripTriCount;
		u32 m_nListTriCount;

	private:
		
	protected:
		u32 m_nCompiledBytes;
		void *m_pCompiledData;
		u32 m_nNumBones;
		FMeshBone_t *m_paBones;// m_nNumBones
		u32 m_nNumChildBones;
		u8 *m_paChildIndices;// m_nNumChildBones
		u32 m_nNumUsedBones;
		u32 m_nRootBoneIndex;
		CUIntArray m_aOldToNewBoneIndex;
		BOOL m_bComputeEdgeData;

		// MeshLib class vars
		MLMesh *m_pMLMesh;

		BOOL CreateBones( CApeToKongFormat *pKong );
};

#endif