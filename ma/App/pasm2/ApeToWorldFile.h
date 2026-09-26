//////////////////////////////////////////////////////////////////////////////////////
// ApeToWorldFile.h - 
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
// 05/14/02 Starich     Created.
// 11/14/02 Lafleur     Generalized for all platforms from GC specific file
//////////////////////////////////////////////////////////////////////////////////////
#ifndef _APE_TO_WORLD_FILE_H_
#define _APE_TO_WORLD_FILE_H_ 1

#include "fang.h"
#include "KongToVisFile.h"
#include "KongToMLMesh.h"

struct ApeObjectLMs_t;
struct LMG_Object_t;
class CCompileDlg;

//
//
//
class CApeToWorldFile
{
	public:
		CApeToWorldFile();
		~CApeToWorldFile();

		BOOL ConvertToWorldFile( CCompileDlg *pDlg, CKongToVisFile &Vis, CAidFile &rAidFile, cchar *pszFilename, TargetPlatform_e nPlatform );
		BOOL ConvertToWorldFileFromVis( CCompileDlg *pDlg, CKongToVisFile &Vis, cchar *pszFilename, TargetPlatform_e nPlatform );
		u32  GetWorldCRC( CKongToVisFile &Vis );
		u32  GetSizeOfConvertedFile( void );
		BOOL WriteWorldFile( CCompileDlg *pDlg, cchar *pszFilename, CKongToVisFile &Vis, TargetPlatform_e nPlatform, FILE *pFileStream=NULL );
		virtual void FreeData();

		BOOL m_bConverted;

		u32 m_nStripCount;
		u32 m_nStripTriCount;
		u32 m_nListTriCount;

		u32 m_nApeObjectCount;
		ApeObjectLMs_t *m_paApeLMs;
		ApeObjectVertColors_t *m_paApeVertColors; 
	private:
		FData_WorldFileHeader_t m_FileHeader;
		LMG_Object_t *m_pFirstStaticObject;
		s32 m_nStaticObjectCount;
		s32 m_nStaticObjectInstanceCount;

		BOOL ConvertMeshesToFANGFormat( CCompileDlg *pDlg, CKongToVisFile &Vis, cchar *pszFilename, TargetPlatform_e nPlatform );
		BOOL CreateLightmaps( CCompileDlg *pDlg, CKongToVisFile &Vis, cchar *pszFilename, u32 nLightMapDetail, f32 fLightMapMBs, f32 fSubSample, BOOL bForceVertexRadiosity, u32 nLightMapSkyLightDetail=0, CFColorRGB *pSkyHoriz=NULL, CFColorRGB *pSkyZenith=NULL, f32 fLightMapSkyLightItens=0.0f );
		u32  CompileLightMaps( CCompileDlg *pDlg, CKongToVisFile &Vis, cchar *pszWLDName, BOOL bUseLibrary, cchar ***paszFileName, BOOL bUseMeshInit = FALSE );
		BOOL CreateFileHeader( CKongToVisFile &Vis );		
		u32  BuildApeDependencyList( CCompileDlg *pDlg, CKongToVisFile &Vis, BOOL bForceVertexRadiosity );
		BOOL CompileApeDependencyList( CCompileDlg *pDlg, CKongToVisFile &Vis );
		void FreeApeDependencyList( void ) ;
		BOOL CheckForReusableVIS( CCompileDlg *pDlg, cchar *pszWldName, CKongToVisFile &Vis, u32 nLightMapDetail, f32 fLightMapMBs );

	protected:
		CUIntArray m_anMeshOffsets;// stores the offset to each mesh
		CUIntArray m_anMeshSizes;// stores the size of each mesh
		CUIntArray m_anMeshPadBytes;// stores the number of bytes to pad each mesh
		u32 m_nWorldPadBytes;// stores the number of bytes to pad the world data
		u32 m_nWorldInitPadBytes;// stores the number of bytes to pad the init data
		u32 m_nStreamingDataPadBytes;

		u32 m_nStreamingDataBytes;
		void *m_pStreamingData;

		CString m_sTempMeshFile;// the filename of the temp file that holds all of the mesh data properly aligned
		u32 m_nTempMeshFileSize;// how many bytes does the temp mesh file contain

		u32 m_nMeshCRC; // The combined mesh CRC for all volume meshes
};

#endif

