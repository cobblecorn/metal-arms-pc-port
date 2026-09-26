//////////////////////////////////////////////////////////////////////////////////////
// SceneExport.h - based on the skeleton exporter max exporter
//
// Author: Michael Starich   
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
// 06/05/01 Starich     Created.
//////////////////////////////////////////////////////////////////////////////////////
#ifndef _SCENE_EXPORT__H
#define _SCENE_EXPORT__H

#include "max.h"
#include "resource.h"
#include "iparamm2.h"
#include "ape_file_def.h"
#include "cam_file_def.h"
#include "ff32hash.h"
#include "VWeights.h"

// IMPORTANT:
// The ClassID must be changed whenever a new project
// is created using this skeleton
#define	APE_EXP_CLASSID					Class_ID(0xf306e27, 0x7dc941ff)//Class_ID(0x1e9b07ed, 0x2254b3)

#define APE_EXPORTER_SWAP_YZ			1	// set to 1 to swaps max's y and z components of the vertices and normals

TCHAR *GetString(int id);
extern ClassDesc* GetApeSceneExportDesc();

// a helper struct for dealing with the material chain
#define _MAX_LAYERS_PER_MATERIAL	10
typedef struct {
	BOOL bCompositeTex;
	BOOL bMapToErrorMat;
	BOOL bUseSubTexs;
	BOOL bUseSubMats;
	BOOL bCompositeMat;
	u32 nNumValidLayers;// could be submats or subtexs
	u32 anValidLayerIndices[_MAX_LAYERS_PER_MATERIAL];
} _MtlHelper_t;


/*===========================================================================*\
 |	ApeExporter class definition
\*===========================================================================*/

typedef enum {
	APE_CAMERA_ENUM_MODE_EXPORT = 0,
	APE_CAMERA_ENUM_MODE_COUNT

}ApeExporterCameraEnumMode_e;

class ApeExporter : public SceneExport {
public:

	// Misc values
	BOOL m_bExportSelected;
	FILE *m_pFileStream;
	Interface *m_pInterface;
	float m_fLightMultiplier;
	CFMtx43 m_Left2RightCoordSysMtx;
	ApeMesh_t m_MeshInfo;

	// Camera variables
	CamInfo_t m_CamInfo;
	u32 m_nCamerasToExport;

	BOOL m_bLODsExist;
	INode *apLODRootNodes[FDATA_MAX_LOD_MESH_COUNT];
	CFMtx43 amtxLODOffset[FDATA_MAX_LOD_MESH_COUNT];

	BOOL m_bWorldFile;	// TRUE if .wld file is being exported (only one of bWorldFile or bObjectFile can be TRUE) 
	BOOL m_bObjectFile;	// TRUE if .ape file is being exported (only one of bWorldFile or bObjectFile can be TRUE) 
	BOOL m_bExportLights;
	BOOL m_bExportGeo;
	BOOL m_bExportSkin;
	BOOL m_bExportObjects;

	// Preferences values
	BOOL m_bApeCfgExportLights;
	BOOL m_bApeCfgExportGeo;
	BOOL m_bApeCfgExportSkin;
	BOOL m_bWldCfgExportGeo;
	BOOL m_bWldCfgExportLights;
	BOOL m_bWldCfgExportObjects;
	
	// Obj/Shape heirchary fixup
	u32 m_nNumHandlesAllocated;
	u32 m_nHandlesUsed;
	u32 *m_panNodeHandles;// needs to be freed

	// Portal memory
	u32 m_nNumPortalsAllocated;
	u32 m_nPortalsUsed;
	ApeVisPortal_t *m_paPortals;

	// Progress bar values
	float m_fPercentSoFar;
	float m_fPercentPerMat;
	int m_nLastPercentUpdate;

	// Error messages
	TSTR m_sErrorHeading;
	u32 m_nNumErrors;
	u32 m_nNumCriticalErrors;

	static CVertexWeights _VertexWeights;
	
	///////////////////////////////////////////////////
	// FOUND IN DOEXPORT.CPP, SCENE EXPORTING FUNCTIONS
	ApeExporter();
	~ApeExporter();
	BOOL CreateApeMaterial( int nMatID, int nNumMats, int nLODIndex, Mesh *pMesh, Matrix3 &TMAfterWSM,
							Matrix3 &NodeTM, BOOL bSwapTriOrder, Mtl *pBaseMtl,	INode *pNode, 
							u32 nNodeIndex, BOOL bSkinned, ApeMaterial_t &rMatInfo, CFf32Hash &VertHashTable,
							ApeVertIndex_t *pIndices, u32 nStartingIndex, BOOL bFlipTriWindingOrder, BOOL bLightMapCoord=FALSE );
	void ProcessUV( int nNumTexMaps, ApeVert_t &V1, ApeVert_t &V2, ApeVert_t &V3 );
	int IsVertAlreadyInList( ApeVert_t &V, ApeVert_t *pVerts, int nNumVerts );
	void WriteToFile( void *pData, int nBytes, int nItems, BOOL bUpdateTotalBytesWritten );
	void WriteToCamFile( void *pData, int nBytes, int nItems, BOOL bUpdateTotalBytesWritten );
	void CreateErrorLayer( ApeLayer_t &Layer );
	void GetVertexWeights( INode *pNode, u32 nVertIndex );
	BOOL GetSkinWeights( INode *pNode, Modifier *pMod, u32 nVertNum );
	void RecordError( BOOL bCriticalError, cchar *pszErrorMsg, INode *pNode=NULL );
	BOOL ParseBaseMtl( Mtl *pBaseMtl, _MtlHelper_t &rMtlHelper, INode *pNode );
	BOOL ExportBones();
	cchar *ExtractTextureName( Mtl *pMaxMtl, u32 nLayerNum, u32 nTypeID );
	BOOL DoesTMHaveNegScale( Matrix3 &rTM, Matrix3 *pNewTM=NULL );
	BOOL DoesTMHaveNonUniformScale( Matrix3 &rTM );
	BOOL DoesMtxHaveNonUniformScale( CFMtx43 &rMtx );

	///////////////////////////////////////////////////
	// FOUND IN SCENEEXPORT.CPP, MAX REQUIRED FUNCTIONS
	int DoExport( const TCHAR *name, ExpInterface *ei, Interface *i,
				  BOOL suppressPrompts=FALSE, DWORD options=0 );
	int ExtCount();
	const TCHAR * Ext(int n);
	const TCHAR * LongDesc();
	const TCHAR * ShortDesc();
	const TCHAR * AuthorName();
	const TCHAR * CopyrightMessage();
	const TCHAR * OtherMessage1();
	const TCHAR * OtherMessage2();
	unsigned int Version();
	void ShowAbout( HWND hWnd );
	BOOL SupportsOptions( int ext, DWORD options );

	int	ApeFileMain( INode *pRootNode, TSTR &rFilename, BOOL suppressPrompts=FALSE );
	int	WldFileMain( INode *pRootNode, TSTR &rFilename, BOOL suppressPrompts=FALSE );
	int	MtxFileMain( INode *pRootNode, TSTR &rFilename, BOOL suppressPrompts=FALSE );
	int	CamFileMain( INode *pRootNode, TSTR &rFilename, BOOL suppressPrompts=FALSE );

	////////////////////////////////////////////////////////
	// FOUND IN CONFIGMGR.CPP, CONFIGURATION FILE MANAGEMENT
	BOOL LoadExporterConfig();
	void SaveExporterConfig();
	TSTR GetConfigFilename();
	
	//////////////////////////////////////////////////////
	// FOUND IN SCENEENUM.CPP, SCENE ENUMERATION FUNCTIONS
	BOOL FogNodeEnum( INode *pNode );
	BOOL LightNodeEnum( INode *pNode );
	BOOL ShapeNodeEnum( INode *pNode );
	BOOL ObjectNodeEnum( INode *pNode );
	BOOL GeometryNodeEnum( INode *pRootNode, INode* pNode );
	BOOL PortalNodeEnum( INode *pNode );
	BOOL CameraNodeEnum( INode *pNode, ApeExporterCameraEnumMode_e EnumMode );

	BOOL IsNodeAnObject( INode *pNode );
	BOOL IsNodeASpeaker( INode *pNode );
	BOOL IsNodeASpawnPoint( INode *pNode );
	BOOL IsNodeAStartPoint( INode *pNode );
	BOOL IsNodeARoomBox( INode *pNode );
	BOOL IsNodeAnArenaBox( INode *pNode );
	BOOL IsNodeACell( INode *pNode );
	BOOL IsNodeAPort( INode *pNode );
	BOOL IsNodeNamedBone( INode *pNode );
	
	BOOL ShouldNodeBeExported( INode *pNode );
	BOOL ShouldBoneBeExported( INode *pNode );
	
	//////////////////////////////////////////////////////
	// FOUND IN PROCESSNODE.CPP, NODE PROCESSING FUNCTIONS
	void ProcessGeoNode( INode *pRootNode, INode *pNode );
	void ProcessLightNode( INode *pNode, GenLight *pLight );
	void ProcessObjectNode( INode *pNode );
	void ProcessCameraNode( INode *pNode, ObjectState &os );  // legacy camera export routine -- used for E3 
	void ProcessCameraNode2( INode *pNode, ObjectState &os ); // new camera animation export routine
	void ProcessShapeNode( INode *pNode, ObjectState &os );
	void ProcessVolumeNode( INode *pNode, ObjectState &os, BOOL bGroup );
	BOOL ProcessCellNode( INode *pNode, ObjectState &os, ApeVisVolume_t *pVol );
	void ProcessPortalNode( INode *pNode, ObjectState &os );

	void GetFangAnimationMtxFromNode( INode *pNode, s32 nTime, CFMtx43 *pFangMtx );
	BOOL FixupObjAndShapeParentIndices( int nStartingFileLoc );
	
	///////////////////////////////////////
	// FOUND IN UTIL.CPP, VARIOUS UTILITIES
	BOOL AreVertsEqual( ApeVert_t &V1, ApeVert_t &V2 );
	BOOL IsTriDegenerative( ApeVertIndex_t &Index1, ApeVertIndex_t &Index2, ApeVertIndex_t &Index3 );
	BOOL IsNodeBone( INode *pNode );
	int GetChildBoneCount( INode *pNode );
	INode *GetChildBoneNode( INode *pNode, u32 nIndex );
	int GetBoneCount( INode *pRoot );
	int RecursiveGetBoneIndex( INode *pRoot, INode *pNodeTest, int &nBoneCount );
	int GetBoneIndex( INode *pRoot, INode *pNodeTest );
	INode *RecursiveGetBoneNode( INode *pRoot, u32 nIndex, int &nBoneCount );
	INode *GetBoneNode( INode *pRoot, u32 nIndex );
	Modifier *FindSkinModifier( INode *pNode );
	BOOL IsNodeMesh( INode *pNode );
	BOOL IsNodeSkinned( INode *pNode );
	TriObject *GetTriObjectFromNode( INode *pNode, TimeValue t, BOOL &bDeleteIt );
	Point3 GetVertexNormal( Mesh *pMesh, int nFaceNum, RVertex *pRV );
	BOOL IsMaterialAComposite( Mtl *pMaxMtl );
	BOOL IsTextureAComposite( Mtl *pMaxMtl );
	void CountMaterials( INode *pNode, int &nMatCount );
	void UpdateProgressBar( int nNewPercent );
	Matrix3 GetNodeLocalTM( INode *pNode, TimeValue t );
	int GetChildNodeCount( INode *pNode );
	INode *GetChildNodeByIndex( INode *pNode, u32 nIndex );
	int GetTotalNodeCount( INode *pRoot );
	int RecursiveGetNodeIndex( INode *pRoot, INode *pNodeTest, int &nBoneCount );
	int GetNodeIndex( INode *pRoot, INode *pNodeTest );
	INode *RecursiveGetNode( INode *pRoot, u32 nIndex, int &nBoneCount );
	INode *GetNodeByIndex( INode *pRoot, u32 nIndex );
	INode *GetNodeByName( INode *pNode, cchar *pszNodeName );
};	

#endif
