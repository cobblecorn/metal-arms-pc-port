//////////////////////////////////////////////////////////////////////////////////////
// KongToVisFile.h - 
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
// 05/09/02 Starich     Created.
//////////////////////////////////////////////////////////////////////////////////////
#ifndef _KONG_TO_VIS_FILE_H_
#define _KONG_TO_VIS_FILE_H_ 1

#include "fang.h"
#include "ApeToKongFormat.h"
#include "fvis.h"
#include "KongToWorldInitFile.h"
#include "KongToChimp.h"
#include "PortalData.h"
#include "KongTriClipper.h"


#define VIS_FILE_VERSION	0x00000003
#define VIS_DATA_VERSION	0x00000004


class CCompileDlg;

enum
{
	VIS_FILE_DOES_NOT_EXIST = 0,
	VIS_FILE_NOT_USED,
	VIS_FILE_INVALID,
	VIS_FILE_OUT_OF_DATE,
	VIS_FILE_LOWER_DETAIL,
	VIS_FILE_UP_TO_DATE_LOCALLY,
	VIS_FILE_UP_TO_DATE_LIBRARY
};

//
//
//
struct VisFileHeader_t
{
	u32 nSignature;
	u32 nVisVersion;
	u32 nKongVersion;
	u32 nVisDataVersion;
	u32 nInitDataVersion;

	u32  nWLDCRC;
	BOOL bHasAidFile;
	u32  nAIDCRC;

	u32  nVolumeMeshCount;
	u32  nLMLevel;
	f32  fLMMemory;

	u32  nVisDataBytes;
	u32  nInitDataBytes;
};


//
//
//
class CKongToVisFile : public CApeToKongFormat
{
public:
	CKongToVisFile();
	~CKongToVisFile();

	VisFileHeader_t FileHeader;

	BOOL ConvertKongFile( CCompileDlg *pDlg, cchar *pszFilename, u32 nSigDigits=3 );
	BOOL PackVisData( void );
	u32  GetCRC( u32 nCurrentCRC );
	u32  GetConvertedDataSize();
	BOOL WriteWorldFile( cchar *pszFilename, BOOL bChangeEndian, FILE *pFileStream=NULL );

	const FVisData_t* GetConvertedFVISData( void ) { return (FVisData_t *)m_pConvertedData; }
	
	BOOL RemoveLocalVISData( cchar *pszLocalPath, cchar *pszWLDName );
	BOOL SaveVisFile( CCompileDlg *pDlg, cchar *pszFilename, u32 nLMLevel, f32 fLMMemory );
	BOOL CheckForVisFile( CCompileDlg *pDlg, const CFileInfo *pFileInfo, const CFileInfo *pAidFileInfo, u32 nLMLevel, f32 fLMMemory );
	u32  VisFileUpToDate( CString &csName, BOOL bLibrary, const CFileInfo *pAidFileInfo, u32 nLMLevel, f32 fLMMemory, 
									VisFileHeader_t &VisHeader, HANDLE &hFile, CString &csError );

	BOOL m_bK2VConverted;		// TRUE indicates the this class contains successfully converted kong->vis data
	
	u32 m_nStatsNonCrossingTris;// how many tris did not cross volume boundaries
	u32 m_nStatsCrossingTris;	// how many orig tris crossed volume boundaries
	u32 m_nStatsFinalTriCount;	// how many tris where there after clipping to volume boundaries

	u32 m_nStaticLightingQualityLevel; // Level at which the lighting was created
	BOOL m_bUsedOldVisData; // TRUE if the static lighting used old VIS data

	u32 m_nStatsVolumes;
	u32 m_nStatsEmptyVolumes;
	u32 m_nStatsCells;
	u32 m_nStatsCellPlanes;
	u32 m_nStatsPortals;
	u32 m_nStatsLights;
	u32 m_nStatsAutoPortals;
	
	f32 m_fStatsAvgTrisPerVol;	// what is the average # of tris per used leaf node
	u32 m_nStatsMaxVolTris;
	u32 m_nStatsVolumeIndexWithMostTris;

	u32 m_nStatsBytesInFile;	// how many bytes will be written to disk (just the world part)

	u32 m_nVisFileStatus;	// see status enum, above
	
	FVisData_t *m_pVisData;		// this data will not be written to disk (a packed version of it will)

	CKongToWorldInitFile m_InitFile;
	
	u32 m_nStatsNumMeshes;		// this number can be <= m_nStatsVolumes (some volumes won't have any geo)
	CPtrArray m_apVolKongMeshs; // m_nStatsNumMeshes of KongToChimp * (already clipped to volume boundaries)

protected:
	virtual void FreeData();
	
private:
	void *m_pConvertedData;// this must be freed if not NULL

	u32 m_nLocalBytesAllocated;
	u8 *m_pMemAllocated;
	u8 *m_pWorkingMem;

	// temp portal and volume format used during compilation
	u32 m_nNumPD_Portals;
	CPtrArray m_apPD_Portals;// CPortalData_Portal *
	u32 m_nNumPD_Volumes;
	CPtrArray m_apPD_Volumes;// CPortalData_Vol *

	// because of alignment issues, these items need to be allocated seperately
	FVisPortal_t *m_paPortals;// there are m_nStatsPortals of these
	FVisVolume_t *m_paVolumes;// there are m_nStatsVolumes of these
	FVisCell_t *m_paCells;// there are m_nStatsCells of these
	FVisPlane_t *m_paPlanes;// there are m_nStatsCellPlanes of these

	CKongClipper m_TriClipper;
	
	typedef struct {
		CPortalData_Cell *pPD_Cell;
		FVisCell_t *pCell;
		u32 nIndexInCellArray;
		FVisAABB_t BBox;
		f32 fBBoxVol;
		CFVec3 BBoxCenter;
		u32 nVolIndex;
	} _CellInfo_t;

	_CellInfo_t *m_paCellInfos;// there are m_nStatsCells of these
	CKongToChimp *m_paChimps;// these hold each volume's geometry

	BOOL FindVolumesContainingPortal( FVisPortal_t *pPortal, CString &rsErrorLine );
	void ComputeCellBBoxAndVol( _CellInfo_t *pCellInfo );
	void ComputeBBoxFromCellInfoBBox( const CPtrArray &rapCellInfo, FVisAABB_t &rBBox, f32 &rfBBoxVol );
	void ComputeBBoxFromCellInfoBBoxCenters( const CPtrArray &rapCellInfo, FVisAABB_t &rBBox, f32 &rfBBoxVol );
	void Construct2GroupsOfCells( const CPtrArray &rapCellInfo, CPtrArray &rapLeft, CPtrArray &rapRight );
	void CreateCellTree();
	u16 CreateCellNode( CPtrArray &rapCells, u16 nParentNodeIndex );
	void CreateLightList();
	void BuildNBTs();
	void CalculateTangentSpace(KongTri_t *pTri, CFVec3& vTan, CFVec3& vBinorm);
	BOOL MeshHasBumpMap();
};


#endif

