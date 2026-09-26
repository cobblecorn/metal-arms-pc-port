//////////////////////////////////////////////////////////////////////////////////////
// ProcessNode.cpp - 
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
// 05/06/02 Starich     Created.
//////////////////////////////////////////////////////////////////////////////////////
#include "SceneExport.h"
#include "fang.h"
#include <stdmat.h>
#include "MatStringParser.h"


u32 TallyMaterialCount( Mtl* pMaxMtl )
{
	u32 i, nCount = 0;
	if ( pMaxMtl ) 
	{
		if ( pMaxMtl->IsMultiMtl() ) 
		{
			// Process the sub materials
			u32 nNumSubMtls = pMaxMtl->NumSubMtls();
			for ( i = 0; i < nNumSubMtls; i++ ) 
			{
				Mtl *pSubMtl = pMaxMtl->GetSubMtl( i );
				if ( pSubMtl ) 
				{
					if ( pSubMtl->SubTexmapOn( ID_DI ) )  
					{
						nCount++;																
					}
				}
			}
		} 
		else 
		{
			nCount++;
		}
	} 
	else 
	{
		nCount++;
	}

	return nCount;
}

// process mesh nodes
void ApeExporter::ProcessGeoNode( INode *pRootNode, INode *pNode ) 
{
	struct __Mesh_t
	{
		INode		*pNode;
		BOOL		bNeedDel;
		Mesh		*pMesh;
		TriObject	*pTriObj;

	} NodeMeshes[FDATA_MAX_LOD_MESH_COUNT];

	CFf32Hash VertHash;
	int nFaces, nVerts, i, nNumSubMtls, nNumMats;
	ApeSegment_t SegInfo;
	ApeVert_t *pVerts = NULL;
	ApeVertIndex_t *pIndices = NULL;
	ApeMaterial_t *pMaterials = NULL;

	cchar *pszSegmentName = pNode->GetName();
	if ( strlen( pszSegmentName ) > 5 && strncmp( pszSegmentName, "LOD", 3 ) == 0 && pszSegmentName[4] == '_' )
	{
		pszSegmentName = &pszSegmentName[5];
	}

	memset( &NodeMeshes, 0, sizeof( __Mesh_t ) * FDATA_MAX_LOD_MESH_COUNT );

	nFaces = nVerts = nNumMats = 0;

	////////////////////////////////////
	// Add in mesh counts from the subsequent LODs
	if ( m_bLODsExist )
	{
		for ( i = 0; i < FDATA_MAX_LOD_MESH_COUNT; i++ )
		{
			if ( !apLODRootNodes[i] )
			{
				continue;
			}

			// Get a pointer to the mesh LOD
			NodeMeshes[i].pNode = GetNodeByName( apLODRootNodes[i], pszSegmentName );

			if ( !NodeMeshes[i].pNode )
			{
				continue;
			}

			NodeMeshes[i].pTriObj = GetTriObjectFromNode( NodeMeshes[i].pNode, 0, NodeMeshes[i].bNeedDel );
			if( !NodeMeshes[i].pTriObj ) 
			{
				return;
			}
			NodeMeshes[i].pMesh = &NodeMeshes[i].pTriObj->GetMesh();

			// Build our normals
			NodeMeshes[i].pMesh->buildRenderNormals();

			// Add in some mesh counts based on the LOD
			nFaces += NodeMeshes[i].pMesh->getNumFaces();
			nVerts += NodeMeshes[i].pMesh->getNumVerts();
			nNumMats += TallyMaterialCount( NodeMeshes[i].pNode->GetMtl() );
		}
	}
	else
	{
		////////////////////////////////////////////////////////////////
		// Setup the base node
		NodeMeshes[0].pNode = pNode;
		NodeMeshes[0].pTriObj = GetTriObjectFromNode( pNode, 0, NodeMeshes[0].bNeedDel );
		if( !NodeMeshes[0].pTriObj ) 
		{
			return;
		}
		NodeMeshes[0].pMesh = &NodeMeshes[0].pTriObj->GetMesh();

		// Build our normals
		NodeMeshes[0].pMesh->buildRenderNormals();

		// Generate some mesh counts based on the base LOD
		nFaces = NodeMeshes[0].pMesh->getNumFaces();
		nVerts = NodeMeshes[0].pMesh->getNumVerts();
		nNumMats = TallyMaterialCount( pNode->GetMtl() );
	}

	////////////////////////////////////////////////////
	// Verify that we have geometry to process
	if ( !nFaces ) 
	{
		// no faces, no need to do anything, report this 
		RecordError( FALSE, "This node has no faces - skipping node.", pNode );
		return;
	}
	if ( !nVerts ) 
	{
		// no verts, no need to do anything, report this 
		RecordError( FALSE, "This node has no verts - skipping node.", pNode );
		return;
	}
	if ( !nNumMats ) 
	{
		// no materials, no need to do anything, report this 
		RecordError( FALSE, "This node has no materials - skipping node.", pNode );
		goto EXIT;
	}

	//////////////////////////////////////
	// Allocate memory, worst case amounts
	u32 nNumToAllocate = nFaces * 3;
	pVerts = new ApeVert_t[nNumToAllocate];
	if( !pVerts ) 
	{
		RecordError( FALSE, "Not enough memory could be allocated - skipping node.", pNode );
		goto EXIT;
	}
	pIndices = new ApeVertIndex_t[nNumToAllocate];
	if( !pIndices ) 
	{
		RecordError( FALSE, "Not enough memory could be allocated - skipping node.", pNode );
		goto EXIT;
	}
	pMaterials = new ApeMaterial_t[nNumMats];
	if ( !pMaterials ) 
	{

		RecordError( FALSE, "Not enough memory could be allocated - skipping node.", pNode );
		goto EXIT;
	}

	//////////////////////
	// Zero out our memory
	ZeroMemory( pVerts, sizeof( ApeVert_t ) * nNumToAllocate );
	ZeroMemory( pIndices, sizeof( ApeVertIndex_t ) * nNumToAllocate );
	ZeroMemory( pMaterials, sizeof( ApeMaterial_t ) * nNumMats );
	
	/////////////////////////////////////
	// Setup the hash table	for the verts
	VertHash.SetupFloatHashTable( sizeof( ApeVert_t ) >> 2, nNumToAllocate * sizeof( ApeVert_t ), FALSE, 0.001f, (u8 *)pVerts );

	////////////////////
	// Setup the segment
	ZeroMemory( &SegInfo, sizeof( ApeSegment_t ) ); 
	strncpy( SegInfo.szNodename, pszSegmentName, (SEGMENT_NAME_LEN-1) );
	SegInfo.bSkinned = m_bExportSkin;

	u32 nNodeIndex;
	if ( m_bLODsExist )
	{
		nNodeIndex = GetNodeIndex( apLODRootNodes[0], NodeMeshes[0].pNode ) + 1;
	}
	else
	{
		nNodeIndex = GetNodeIndex( pRootNode, NodeMeshes[0].pNode );
	}

	//////////////////////////////////////////////////////////////////////////////////////////////////
	// Process the material assigned to this node, create materials and assigning faces to those faces
	u32 nLOD;
	for ( nLOD = 0; nLOD < FDATA_MAX_LOD_MESH_COUNT; nLOD++ )
	{
		if ( !NodeMeshes[nLOD].pNode || !NodeMeshes[nLOD].pMesh )
		{
			continue;
		}

		Matrix3 TMAfterWSM, NodeTM;

		///////////////////////////
		// Grab our transform matrix
		// (TMAfterWSM is multiplied by every vertex, NodeTM is Multiplied by normals)
		TMAfterWSM = NodeMeshes[nLOD].pNode->GetObjTMAfterWSM( 0 );
		TMAfterWSM = (Matrix3 &)(amtxLODOffset[nLOD] * (CFMtx43 &)TMAfterWSM);
		NodeTM = TMAfterWSM;
		NodeTM.NoTrans();

		BOOL bSwapTriOrder = TMAfterWSM.Parity();

		////////////////////////////////////////////////
		// See if this node has a negative scale applied
		BOOL bFlipWindingOrder = DoesTMHaveNegScale( NodeTM );
		if( bFlipWindingOrder ) 
		{
			DEVPRINTF( "Node named: '%s' has a negative scale (dot < 0.0f), flipping all face winding orders.\n", NodeMeshes[nLOD].pNode->GetName() );
		}

		Mtl *pMaxMtl = NodeMeshes[nLOD].pNode->GetMtl();

		if ( pMaxMtl ) 
		{
			if ( pMaxMtl->IsMultiMtl() ) 
			{
				// Process the sub materials
				nNumSubMtls = pMaxMtl->NumSubMtls();
				for ( i = 0; i < nNumSubMtls; i++ ) 
				{
					Mtl *pSubMtl = pMaxMtl->GetSubMtl( i );
					if ( pSubMtl ) 
					{
						if ( pSubMtl->SubTexmapOn( ID_DI ) ) 
						{
							// assign all faces mapped to material i to this material
							if ( CreateApeMaterial( i, nNumSubMtls, nLOD, NodeMeshes[nLOD].pMesh, TMAfterWSM, NodeTM, bSwapTriOrder, pSubMtl, NodeMeshes[nLOD].pNode,
												nNodeIndex, SegInfo.bSkinned, pMaterials[ SegInfo.nNumMaterials ], VertHash, 
												&pIndices[ SegInfo.nNumIndices ], SegInfo.nNumIndices, bFlipWindingOrder, TRUE ) ) 
							{
								// add this materials indices to our segment's count
								SegInfo.nNumIndices += pMaterials[ SegInfo.nNumMaterials ].nNumIndices;
								SegInfo.nNumMaterials++;
							}																
						}
					}
				}
			} 
			else 
			{
				// just one material assigned, assign all faces to one material
				if ( pMaxMtl->SubTexmapOn( ID_DI ) ) 
				{
					if ( CreateApeMaterial( -1, 0, nLOD, NodeMeshes[nLOD].pMesh, TMAfterWSM, NodeTM, bSwapTriOrder, pMaxMtl, NodeMeshes[nLOD].pNode, 
										nNodeIndex, SegInfo.bSkinned, pMaterials[ SegInfo.nNumMaterials ], VertHash,
										&pIndices[ SegInfo.nNumIndices ], SegInfo.nNumIndices, bFlipWindingOrder, TRUE ) ) 
					{
						// add this materials indices to our segment's count
						SegInfo.nNumIndices += pMaterials[ SegInfo.nNumMaterials ].nNumIndices;
						SegInfo.nNumMaterials++;
					}
				}
			}
		} 
		else 
		{
			// no material assigned, assign all faces to one material
			if ( CreateApeMaterial( -1, 0, nLOD, NodeMeshes[nLOD].pMesh, TMAfterWSM, NodeTM, bSwapTriOrder, NULL, NodeMeshes[nLOD].pNode,
								nNodeIndex, SegInfo.bSkinned, pMaterials[ SegInfo.nNumMaterials ], VertHash,
								&pIndices[ SegInfo.nNumIndices ], SegInfo.nNumIndices, bFlipWindingOrder ) ) 
			{
				// add this materials indices to our segment's count
				SegInfo.nNumIndices += pMaterials[ SegInfo.nNumMaterials ].nNumIndices;
				SegInfo.nNumMaterials++;
			}
		}
	}

	///////////////////////////////////////////////////
	// Get the number of used verts from the hash table
	SegInfo.nNumVerts = VertHash.GetDataObsUsed();

	if( !SegInfo.nNumMaterials || !SegInfo.nNumVerts || !SegInfo.nNumIndices ) 
	{
		// there are no materials, verts, or indices in this segment, nothing to write out
		goto EXIT;
	}	

	//////////////////////////////////////////////////////////////////////////
	// Make sure that each face only uses MAX_WEIGHTS_PER_FACE bone influences
	ApeVert_t *pVertList = (ApeVert_t *)VertHash.GetDataObByIndex( 0 );
	if( SegInfo.bSkinned ) 
	{
		_VertexWeights.ProcessFaceList( pVertList, SegInfo.nNumVerts, pIndices, SegInfo.nNumIndices );
	}

	/////////////////////////////////////////////////////////////////////////////
	// Write out our data in the proper order ( Seg, Mat[], Vert[], VertIndex[] )
	m_MeshInfo.nNumSegments++;
	WriteToFile( &SegInfo, sizeof( ApeSegment_t ), 1, TRUE );
	WriteToFile( pMaterials, sizeof( ApeMaterial_t ), SegInfo.nNumMaterials, TRUE );
	WriteToFile( pVertList, sizeof( ApeVert_t ), SegInfo.nNumVerts, TRUE );
	WriteToFile( pIndices, sizeof( ApeVertIndex_t ), SegInfo.nNumIndices, TRUE );

EXIT:
	//////////////////
	// Free our memory
	if ( pVerts )
	{
		delete pVerts;
	}
	if ( pIndices )
	{
		delete pIndices;
	}
	if ( pMaterials )
	{
		delete pMaterials;
	}

	for ( i = 0; i < FDATA_MAX_LOD_MESH_COUNT; i++ )
	{
		if ( NodeMeshes[i].pTriObj && NodeMeshes[i].bNeedDel )
		{
			delete NodeMeshes[i].pTriObj;
		}
	}
}

// process light nodes
void ApeExporter::ProcessLightNode( INode *pNode, GenLight *pLight ) {
	struct LightState ls;
	Point3 Pos, Dir;
	CFMtx43 mtxOrient;
	ApeLight_t ApeLight;
	CLightStringParser StringParser( m_bWorldFile );

	if( !pLight->GetUseLight() ) {
		// this light is off, don't export it
		return;
	}
	// zero out our ape light
	ZeroMemory( &ApeLight, sizeof( ApeLight_t ) );
	
	pLight->EvalLightState( 0, FOREVER, &ls );

	// Get the world or parent relative transform matrix
	Matrix3 tm;
	INode *pParentNode = pNode->GetParentNode();
	if ( pParentNode )
	{
		tm = pNode->GetNodeTM( 0 );
		Matrix3 tmParent = pParentNode->GetNodeTM( 0 );
		tmParent.Invert();
		tm = tm * tmParent;

		strncpy( ApeLight.szParentBoneName, pParentNode->GetName(), BONE_NAME_LEN - 1 );
	}
	else
	{
		tm = pNode->GetNodeTM( 0 );
		ApeLight.szParentBoneName[0] = 0;
	}

	Pos = tm.GetTrans();
	Dir = tm.GetRow( 2 );

	// grab scale info
	Point3 row0 = tm.GetRow(0);
	float fScale = FLength( row0 );

	// copy the name of this light
	strncpy( ApeLight.szLightName, pNode->GetName(), (LIGHT_NAME_LEN-1) );
	
	// copy the color
	ApeLight.Color.Set( ls.color.r, ls.color.g, ls.color.b );
	
	// copy the intensity
	ApeLight.fIntensity = ls.intens * m_fLightMultiplier;
	//FMATH_CLAMP_UNIT_FLOAT( ApeLight.fIntensity );
	ApeLight.fIntensity = (ApeLight.fIntensity > 0.0f)?(ApeLight.fIntensity):(0.0f);

	// Get the orientation matrix
	ApeLight.mtxOrientation.m_vRight.x = tm.GetRow(0).x;
	ApeLight.mtxOrientation.m_vRight.y = tm.GetRow(0).z;
	ApeLight.mtxOrientation.m_vRight.z = tm.GetRow(0).y;
	ApeLight.mtxOrientation.m_vUp.x = tm.GetRow(1).x;
	ApeLight.mtxOrientation.m_vUp.y = tm.GetRow(1).z;
	ApeLight.mtxOrientation.m_vUp.z = tm.GetRow(1).y;
	ApeLight.mtxOrientation.m_vFront.x = -tm.GetRow(2).x;
	ApeLight.mtxOrientation.m_vFront.y = -tm.GetRow(2).z;
	ApeLight.mtxOrientation.m_vFront.z = -tm.GetRow(2).y;
	ApeLight.mtxOrientation.m_vPos.x = tm.GetRow(3).x;
	ApeLight.mtxOrientation.m_vPos.y = tm.GetRow(3).y;
	ApeLight.mtxOrientation.m_vPos.z = tm.GetRow(3).z;

	// Scale the matrix by the row scale
	f32 fInvScale = 1.f / fScale;
	ApeLight.mtxOrientation.m_vRight *= fInvScale;
	ApeLight.mtxOrientation.m_vUp *= fInvScale;
	ApeLight.mtxOrientation.m_vFront *= fInvScale;

	switch( ls.type ) {
	case OMNI_LGT:
		ApeLight.nType = APE_LIGHT_TYPE_OMNI;
		ApeLight.Sphere.m_fRadius = ls.attenEnd * fScale;
#if APE_EXPORTER_SWAP_YZ
		ApeLight.Sphere.m_Pos.Set( Pos.x, Pos.z, Pos.y );
#else
		ApeLight.Sphere.m_Pos.Set( Pos.x, Pos.y, Pos.z );
#endif
		break;
	case SPOT_LGT:
		ApeLight.nType = APE_LIGHT_TYPE_SPOT;
		ApeLight.Sphere.m_fRadius = ls.attenEnd * fScale;
#if APE_EXPORTER_SWAP_YZ
		ApeLight.Sphere.m_Pos.Set( Pos.x, Pos.z, Pos.y );
		ApeLight.Dir.Set( -Dir.x, -Dir.z, -Dir.y );
#else
		ApeLight.Sphere.m_Pos.Set( Pos.x, Pos.y, Pos.z );
		ApeLight.Dir.Set( -Dir.x, -Dir.y, -Dir.z );
#endif
		ApeLight.fSpotInnerAngle = FMATH_DEG2RAD( ls.hotsize );
		ApeLight.fSpotOuterAngle = FMATH_DEG2RAD( ls.fallsize );
		break;
	case DIRECT_LGT:
		ApeLight.nType = APE_LIGHT_TYPE_DIR;
#if APE_EXPORTER_SWAP_YZ
		ApeLight.Dir.Set( -Dir.x, -Dir.z, -Dir.y );
#else
		ApeLight.Dir.Set( -Dir.x, -Dir.y, -Dir.z );
#endif
		break;
	}
	// parser for star commands
	StringParser.ResetToDefaults();
	StringParser.Parse( pNode->GetName() );
	ApeLight.nFlags = StringParser.m_nCommands;
	ApeLight.nMotifID = StringParser.m_nMotifID;
	ApeLight.nLightID = StringParser.m_nLightID;
	ApeLight.fCoronaScale = StringParser.m_fCoronaScale;
	strncpy( ApeLight.szCoronaTexture, StringParser.m_szCoronaTexture, TEXTURE_NAME_LEN );
	strncpy( ApeLight.szPerPixelTexture, StringParser.m_szPerPixelTexture, TEXTURE_NAME_LEN );
	if ( pLight->GetProjMap() )
	{
		char szMapName[256];
		strncpy( szMapName, pLight->GetProjMap()->GetFullName(), 255 );
		szMapName[255] = 0;
		char *pStart = strstr( szMapName, "(" );
		if ( pStart )
		{		
			strncpy( ApeLight.szPerPixelTexture, &pStart[1], TEXTURE_NAME_LEN );
			pStart = strstr( ApeLight.szPerPixelTexture, "." );
			if ( pStart )
			{
				(*pStart) = 0;
			}
		}
	}

	// write out the light to our file
	WriteToFile( &ApeLight, sizeof( ApeLight_t ), 1, TRUE );
	m_MeshInfo.nNumLights++;
}

// process object placement nodes
void ApeExporter::ProcessObjectNode( INode *pNode ) {
	CStr sName;
	ApeObject_t ApeOb;
	CObjStringParser StringParser;

	cchar *pszNodeName = pNode->GetName();
	if( !pszNodeName ) {
		return;
	}
	sName = pszNodeName;
	sName.toLower();
	CStr sTarget = "obj_";
	if( sName.Substr( 0, 4 ) == sTarget ) {
		sName.remove( 0, 4 );
		int nEndIndex = sName.first( '.' );
		if( nEndIndex > -1 ) {
			sName.remove( nEndIndex );
		}
		// make sure that the name is not too long
		if( sName.Length() >= (OBJECT_NAME_LEN-4) ) {
			// the name is too long
			CStr sErrorCode;
			sErrorCode.printf( "The referenced filename '%s' is too long %d (filenames must be %d characters or less) - skipping object.",
				sName, sName.Length(), (OBJECT_NAME_LEN-4) );
			RecordError( FALSE, sErrorCode, pNode );
			return;
		}
		
		// grab the location info
		Matrix3 tm = pNode->GetNodeTM( 0 );
		Point3 Pos = tm.GetTrans();

		// make sure that there is not non uniform scaling
		if( DoesTMHaveNonUniformScale( tm ) ) {
			// non uniform scaling, don't export this object
			RecordError( FALSE, "Non-uniform scaling used - skipping object.", pNode );
			return;
		}
		// fill in our ApeOb 
		ZeroMemory( &ApeOb, sizeof( ApeObject_t ) );
		strncpy( ApeOb.szName, (cchar *)sName, OBJECT_NAME_LEN-1 );

		// parser for star commands
		StringParser.ResetToDefaults();
		StringParser.Parse( pszNodeName );
		ApeOb.nFlags = StringParser.m_nCommands;
		ApeOb.fCullDistance = StringParser.m_fCullDist;
		ApeOb.TintRGB = StringParser.m_TintRGB;

		// convert the right handed max matrix to a right handed one
		CFMtx43 Rot;
		Rot = (CFMtx43 &)tm;
		ApeOb.Orientation = m_Left2RightCoordSysMtx * Rot * m_Left2RightCoordSysMtx;
		
		// grab the user defined properties
		TSTR sUserProp;
		pNode->GetUserPropBuffer( sUserProp);
		ApeOb.nBytesOfUserData = sUserProp.Length();
		
		// write out the object to our file
		WriteToFile( &ApeOb, sizeof( ApeObject_t ), 1, TRUE );
		m_MeshInfo.nNumObjects++;
		// write out the user defined properties for this object
		if( ApeOb.nBytesOfUserData ) {
			WriteToFile( sUserProp.data(), ApeOb.nBytesOfUserData, 1, TRUE );	
		}

		// record this node's handle so that we can fixup the parent index later
		m_panNodeHandles[m_nHandlesUsed] = pNode->GetHandle();
		m_nHandlesUsed++;
		FASSERT( m_nHandlesUsed <= m_nNumHandlesAllocated );
	}
}

// it is assumed that pNode is a CAMERA_CLASS_ID and that
// pNode needs to be exported.
void ApeExporter::ProcessCameraNode( INode *pNode, ObjectState &os ) {
	CStr sName( pNode->GetName() );
	ApeShape_t ApeShape;
	CFMtx43 Rot;
	CameraObject *pCamObject;
	CameraState CamState;
	Matrix3 tm;
	TSTR sUserProp;
	Interval animRange;
	TimeValue t, TimeOfLastCapturedFrame;
	ApeCamFrame_t CamFrame;

	pCamObject = (CameraObject *)os.obj;
	pCamObject->EvalCameraState( 0, FOREVER, &CamState );

	// zero our memory first
	fang_MemZero( &ApeShape, sizeof( ApeShape_t ) );
	
	// make sure that there does exist an 'id' field
	if( !pNode->UserPropExists( "id" ) &&
		!pNode->UserPropExists( "ID" ) ) {
		RecordError( FALSE, "No 'ID=' field found - skipping camera object.", pNode );
		return;// keep enumerating
	}

	// fill in our shape struct
	ApeShape.nType = APE_SHAPE_TYPE_CAMERA;
	ApeShape.TypeData.Camera.fFOV = CamState.fov;
	ApeShape.TypeData.Camera.nFrames = 0;
	ApeShape.TypeData.Camera.nOffsetToFrames = 0;
	ApeShape.TypeData.Camera.nOffsetToString = 0;
	
	// grab the location info
	tm = pNode->GetObjTMAfterWSM( 0 );
	// convert the right handed max matrix to a right handed one
	Rot = (CFMtx43 &)tm;
	ApeShape.Orientation = m_Left2RightCoordSysMtx * Rot * m_Left2RightCoordSysMtx;
	// now convert max's screwed up camera matrix (view dir = -y, up = z) by rotation about x by 90
	CFMtx43 Convert;
	Convert.Identity();
	Convert.aa[1][1] = 0.0f;
	Convert.aa[1][2] = 1.0f;
	Convert.aa[2][1] = -1.0f;
	Convert.aa[2][2] = 0.0f;
	ApeShape.Orientation *= Convert;
	// grab the user defined properties
	pNode->GetUserPropBuffer( sUserProp );
	
	// record our position in the file
	int nSegmentFileLoc = ftell( m_pFileStream );
	// write out this shape to our file
	WriteToFile( &ApeShape, sizeof( ApeShape_t ), 1, TRUE );
	m_MeshInfo.nNumShapes++;
	
	// write out the animination frames
	animRange = m_pInterface->GetAnimRange();
	int nCurFOVDeg, nLastFOVDeg;

	for( t=animRange.Start(); t <= animRange.End(); t += GetTicksPerFrame() ) {
		pCamObject->EvalCameraState( t, FOREVER, &CamState );
		
		// always export the first and last frame
		if( t == animRange.Start() ||
			t == animRange.End() ) {
			
			CamFrame.fFOV = CamState.fov;
			CamFrame.fSecsFromStart = TicksToSec( t - animRange.Start() );
			ApeShape.TypeData.Camera.nFrames++;
			WriteToFile( &CamFrame, sizeof( ApeCamFrame_t ), 1, TRUE );
			// record the time
			TimeOfLastCapturedFrame = t;
		} else {
			// export only frames that changed from the last one captured
			nLastFOVDeg = (int)( FMATH_RAD2DEG( CamFrame.fFOV ) * 100.5f );
			nCurFOVDeg = (int)( FMATH_RAD2DEG( CamState.fov ) * 100.5f );
		
			if( nLastFOVDeg != nCurFOVDeg ) {
				// this frame is different from the last one, add it

				// make sure that the frame right before this one was captured too
				if( (TimeOfLastCapturedFrame + GetTicksPerFrame()) != t ) {
					// we just need to update the time held in the CamFrame, the fov is already right
					CamFrame.fSecsFromStart = TicksToSec( t - GetTicksPerFrame() - animRange.Start() );
					ApeShape.TypeData.Camera.nFrames++;
					WriteToFile( &CamFrame, sizeof( ApeCamFrame_t ), 1, TRUE );
				}
				// write out the current frame
				CamFrame.fFOV = CamState.fov;
				CamFrame.fSecsFromStart = TicksToSec( t - animRange.Start() );
				ApeShape.TypeData.Camera.nFrames++;
				WriteToFile( &CamFrame, sizeof( ApeCamFrame_t ), 1, TRUE );
				// record the time
				TimeOfLastCapturedFrame = t;
			}
		}
	}
	ApeShape.TypeData.Camera.nOffsetToString = ApeShape.TypeData.Camera.nFrames * sizeof( ApeCamFrame_t );

	// write out the user defined properties for this shape
	if( sUserProp.Length() ) {
		WriteToFile( sUserProp.data(), sUserProp.Length(), 1, TRUE );	
	}
	// compute our total user data size
	ApeShape.nBytesOfUserData = ApeShape.TypeData.Camera.nOffsetToString + sUserProp.Length();

	// rewrite out the shape info
	fseek( m_pFileStream, nSegmentFileLoc, SEEK_SET );
	WriteToFile( &ApeShape, sizeof( ApeShape_t ), 1, FALSE );
	fseek( m_pFileStream, 0, SEEK_END );

	// record this node's handle so that we can fixup the parent index later
	m_panNodeHandles[m_nHandlesUsed] = pNode->GetHandle();
	m_nHandlesUsed++;
	FASSERT( m_nHandlesUsed <= m_nNumHandlesAllocated );
}


// it is assumed that pNode is a CAMERA_CLASS_ID and that
// pNode needs to be exported (name is precededed with a "cam_").
void ApeExporter::ProcessCameraNode2( INode *pNode, ObjectState &os ) {
	CStr sName( pNode->GetName() );
	CFMtx43 Rot;
	CameraObject *pCamObject;
	CameraState CamState;
	Matrix3 tm;
	TSTR sUserProp;
	Interval animRange;
	TimeValue t;
	CamFrame_t CamFrame;

	pCamObject = (CameraObject *)os.obj;
	pCamObject->EvalCameraState( 0, FOREVER, &CamState );


	sName.remove( 0, 4 ); //remove the "cam_" name
	strcpy( m_CamInfo.szCameraName, sName );

	// grab the user defined properties
	pNode->GetUserPropBuffer( sUserProp );

	// write out the animination frames
	animRange = m_pInterface->GetAnimRange();
	f32 fAnimStartTime = -1.0f;
	for( t=animRange.Start(); t <= animRange.End(); t += GetTicksPerFrame() ) {
		Interval valid = FOREVER;
		pCamObject->EvalCameraState( t, valid, &CamState );
		
		if( ( valid == FOREVER ) || ( valid.Start() == valid.End() ) ) {
			if( fAnimStartTime == -1.0f ) {
				fAnimStartTime = TicksToSec( t - animRange.Start() );
			}

			// grab the location info
			tm = pNode->GetObjTMAfterWSM( t );
			// convert the right handed max matrix to a right handed one
			Rot = (CFMtx43 &)tm;
			CamFrame.Orientation = m_Left2RightCoordSysMtx * Rot * m_Left2RightCoordSysMtx;
			// now convert max's screwed up camera matrix (view dir = -y, up = z) by rotation about x by 90
			CFMtx43 Convert;
			Convert.Identity();
			Convert.aa[1][1] = 0.0f;
			Convert.aa[1][2] = 1.0f;
			Convert.aa[2][1] = -1.0f;
			Convert.aa[2][2] = 0.0f;
			CamFrame.Orientation *= Convert;

			CamFrame.fFOV = CamState.fov;
			CamFrame.fSecsFromStart = TicksToSec( t - animRange.Start() ) - fAnimStartTime;
			m_CamInfo.nFrames++;
			WriteToCamFile( &CamFrame, sizeof( CamFrame_t ), 1, TRUE );
		}
	}
	//now, record off how far we need to 
	m_CamInfo.nOffsetToString = m_CamInfo.nFrames * sizeof( CamFrame_t );

	// write out the user defined properties for this shape
	if( sUserProp.Length() ) {
		WriteToCamFile( sUserProp.data(), sUserProp.Length(), 1, TRUE );	
	}
	// compute our total user data size
	m_CamInfo.nBytesOfUserData = m_CamInfo.nOffsetToString + sUserProp.Length();
}



// it is assumed that pNode is a SHAPE_CLASS_ID and that
// pNode needs to be exported.
void ApeExporter::ProcessShapeNode( INode *pNode, ObjectState &os ) {
	CStr sName( pNode->GetName() );
	ApeShape_t *pApeShape;
	ApeSplinePt_t *pApeSplinePt;
	u32 nNumBytesAllocated;
	CFMtx43 Rot;
	Matrix3 tm;
	ShapeObject *pShape;
	PolyShape PolyShape;
	int nNumLines, i, nNumVerts, nBytesOfUserProps;
	TSTR sUserProp;
	BOOL bIsClosed;
	PolyPt *pPolyPt;
	Point3 PosWS;

	pShape = (ShapeObject *)os.obj;

	// We will output shapes as a collection of polylines.
	// Each polyline contains collection of line segments.
	pShape->MakePolyShape( 0, PolyShape );
	nNumLines = PolyShape.numLines;

	if( nNumLines != 1 ) {
		RecordError( FALSE, "Splines must be contructed with only 1 continuous line, no breaks. Skipping spline object.", pNode );
		return;// keep enumerating
	}

	// grab the user defined properties
	pNode->GetUserPropBuffer( sUserProp );
	nBytesOfUserProps = sUserProp.Length();
	nNumVerts = PolyShape.lines[0].numPts;
	bIsClosed = PolyShape.lines[0].IsClosed();

	// allocate memory to hold our memory image
	nNumBytesAllocated = sizeof( ApeShape_t ) + 
						 (nNumVerts * sizeof( ApeSplinePt_t ) ) + 
						 nBytesOfUserProps;
	pApeShape = (ApeShape_t *)fang_MallocAndZero( nNumBytesAllocated, 4 );
	if( !pApeShape ) {
		RecordError( TRUE, "Could not allocate memory to hold a spline object, free some memory.", pNode );
		return;// keep enumerating
	}

	// set our pointers
	pApeSplinePt = (ApeSplinePt_t *)&pApeShape[1];

	// grab the location info
	tm = pNode->GetObjTMAfterWSM( 0 );
	
	// convert the right handed max matrix to a right handed one
	Rot = (CFMtx43 &)tm;
	pApeShape->Orientation = m_Left2RightCoordSysMtx * Rot * m_Left2RightCoordSysMtx;
	pApeShape->nType = APE_SHAPE_TYPE_SPLINE;
	pApeShape->nBytesOfUserData = nNumBytesAllocated - sizeof( ApeShape_t );
	pApeShape->TypeData.Spline.nNumPts = nNumVerts;
	pApeShape->TypeData.Spline.bClosed = bIsClosed;
	pApeShape->TypeData.Spline.nNumSegments = (bIsClosed) ? nNumVerts : (nNumVerts - 1);
	
	// We differ between true and interpolated knots
	for( i=0; i < nNumVerts; i++ ) {
		pPolyPt = &PolyShape.lines[0].pts[i];
		PosWS = tm * pPolyPt->p;
#if APE_EXPORTER_SWAP_YZ
		pApeSplinePt[i].Pos.Set( PosWS.x, PosWS.z, PosWS.y );
#else
		pApeSplinePt[i].Pos.Set( PosWS.x, PosWS.y, PosWS.z );
#endif
	}

	// copy our user props
	if( nBytesOfUserProps ) {
		fang_MemCopy( &pApeSplinePt[nNumVerts], sUserProp.data(), nBytesOfUserProps );
	}

	// write our data out to disk
	WriteToFile( pApeShape, nNumBytesAllocated, 1, TRUE );
	m_MeshInfo.nNumShapes++;

	// free our memory
	fang_Free( pApeShape );

	// record this node's handle so that we can fixup the parent index later
	m_panNodeHandles[m_nHandlesUsed] = pNode->GetHandle();
	m_nHandlesUsed++;
	FASSERT( m_nHandlesUsed <= m_nNumHandlesAllocated );
}

// assumes that pNode is a dummy named "cell_" and is in fact the head 
// of a group of nodes.
void ApeExporter::ProcessVolumeNode( INode *pNode, ObjectState &os, BOOL bGroup ) {
	int i, nNumChildren;
	INode *pChild;
	ObjectState ChildOS;
	ApeVisVolume_t Vol;

	// zero out the volume
	fang_MemZero( &Vol, sizeof( ApeVisVolume_t ) );

	if( bGroup ) {
		nNumChildren = GetChildNodeCount( pNode );
		for( i=0; i < nNumChildren; i++ ) {
			pChild = GetChildNodeByIndex( pNode, i );
			
			ChildOS = pChild->EvalWorldState( 0 );

			if( pChild ) {
				// see if we have a cell
				if( IsNodeACell( pChild ) ) {
					
					ProcessCellNode( pChild, os, &Vol );
					
				} else if( IsNodeAPort( pChild ) && 
						   ChildOS.obj->SuperClassID() == SHAPE_CLASS_ID ) {

					ProcessPortalNode( pChild, ChildOS );
				}
			}
		}
		// write out the new volume
		if( Vol.nNumCells ) {
			m_MeshInfo.nNumVisVolumes++;
			WriteToFile( &Vol, sizeof( ApeVisVolume_t ), 1, TRUE );
		}
	} else {
		// just a single cell in this volume
		
		// process the cell
		if( ProcessCellNode( pNode, os, &Vol ) ) {
			// write out the new volume
			m_MeshInfo.nNumVisVolumes++;
			WriteToFile( &Vol, sizeof( ApeVisVolume_t ), 1, TRUE );
		}
	}
}


#include "MNMATH.H"

// assumes that pNode is of type GEOMOBJECT_CLASS_ID and is named "cell_"
BOOL ApeExporter::ProcessCellNode( INode *pNode, ObjectState &os, ApeVisVolume_t *pVol ) {
	BOOL bNeedDel;
	Matrix3 TMAfterWSM, NodeTM;
	int i, j;
	CFVec3 *pA, *pB, UnitAtoB, P1, P2, P3, Temp;
	f32 fD1, fD2, fDot, fMag2;
	Point3 Vert, Norm, Center, Width;
	ApeVisPt_t *pPt;
	ApeVisEdges_t *pEdge;
	ApeVisFaces_t *pFace, *pFace2;
	Box3 BBox;
		
	if( pVol->nNumCells >= MAX_VIS_CELLS_PER_VOL ) {
		RecordError( TRUE, "Too many cells in a single volume, reduce the cell count in the volume.", pNode );
		return FALSE;
	}

	ApeVisCell_t *pCell = &pVol->aCells[ pVol->nNumCells ];
	
	TriObject *pTri = GetTriObjectFromNode( pNode, 0, bNeedDel );
	if( !pTri ) {
		RecordError( TRUE, "Could not convert a cell into a mesh.  Cells must be made from polys. Please fix the cell geometry.", pNode );
		return FALSE;
	}
	Mesh *pMesh = &pTri->GetMesh();

	////////////////////////////
	// Grab our transform matrix
	// (TMAfterWSM is multiplied by every vertex, NodeTM is Multiplied by normals)
	TMAfterWSM = pNode->GetObjTMAfterWSM( 0 );
	NodeTM = TMAfterWSM;
	NodeTM.NoTrans();

	BOOL bFlipWindingOrder = DoesTMHaveNegScale( NodeTM );

	// create a convex mnmesh
	MNMesh MnMesh( *pMesh );
	MnMesh.EliminateBadVerts( 0 );
	MnMesh.MakeConvexPolyMesh();
	MnMesh.RestrictPolySize( MAX_VIS_DEGREES_PER_FACE );
	MnMesh.MakeConvex();
	MnMesh.EliminateCollinearVerts();
	MnMesh.EliminateCoincidentVerts();
	
	// make sure that we have a closed mesh
	if( !MnMesh.IsClosed() ) {
		RecordError( TRUE, "Cells must be closed, polygon meshes. Please fix the cell geometry.", pNode );
		goto _CLEANUP_AND_FAIL;
	}

	// make sure that we don't have too many edges/faces/pts...
	if( MnMesh.numv > MAX_VIS_PTS_PER_CELL ) {
		RecordError( TRUE, "Too many verts defining a cell. Please reduce the vert count.", pNode );
		goto _CLEANUP_AND_FAIL;
	}
	if( MnMesh.nume > MAX_VIS_EDGES_PER_CELL ) {
		RecordError( TRUE, "Too many edges defining a cell. Please reduce the edge count.", pNode );
		goto _CLEANUP_AND_FAIL;
	}
	if( MnMesh.numf > MAX_VIS_FACES_PER_CELL ) {
		RecordError( TRUE, "Too many faces defining a cell. Please reduce the face count.", pNode );
		goto _CLEANUP_AND_FAIL;
	}

	pPt = &pCell->aPts[0];
	for( i=0; i < MnMesh.numv; i++ ) {
		Vert = MnMesh.v[i].p * TMAfterWSM;
#if APE_EXPORTER_SWAP_YZ
		pPt->Pos.Set( Vert.x, Vert.z, Vert.y );
#else
		pPt->Pos.Set( Vert.x, Vert.y, Vert.z );
#endif
		pPt++;
	}
	pCell->nNumPts = MnMesh.numv;

	pEdge = &pCell->aEdges[0];
	for( i=0; i < MnMesh.nume; i++ ) {
		pEdge->anPtIndices[0] = MnMesh.e[i].v1;
		pEdge->anPtIndices[1] = MnMesh.e[i].v2;
		pEdge->anFaceIndices[ pEdge->nNumFaces++ ] = MnMesh.e[i].f1;
		if( MnMesh.e[i].f2 >= 0 ) {
			pEdge->anFaceIndices[ pEdge->nNumFaces++ ] = MnMesh.e[i].f2;
		}
		FASSERT( pEdge->nNumFaces == 2 );
		pEdge++;
	}
	pCell->nNumEdges = MnMesh.nume;

	pFace = &pCell->aFaces[0];
	for( i=0; i < MnMesh.numf; i++ ) {
		if( MnMesh.f[i].deg > MAX_VIS_DEGREES_PER_FACE ) {
			RecordError( TRUE, "The cell geometry is too complex. Please simplify the cell geometry.", pNode );
			goto _CLEANUP_AND_FAIL;
		}

		pFace->nDegree = MnMesh.f[i].deg;
		FASSERT( pFace->nDegree >= 3 );
		for( j=0; j < (s32)pFace->nDegree; j++ ) {
			pFace->aPtIndices[j] = MnMesh.f[i].vtx[j];
			pFace->aEdgeIndices[j] = MnMesh.f[i].edg[j];
		}

		MnMesh.ComputeSafeCenter( i, Norm );
		Vert = Norm * TMAfterWSM;

		// compute the normal for the face
		P1 = pCell->aPts[ pFace->aPtIndices[0] ].Pos;
		P2 = pCell->aPts[ pFace->aPtIndices[1] ].Pos;
		P3 = pCell->aPts[ pFace->aPtIndices[2] ].Pos;
		P2 -= P1;
		P3 -= P1;
		pFace->Normal = P3.Cross( P2 );
		fMag2 = pFace->Normal.Mag2();
		if( fMag2 <= 0.05f ) {
			RecordError( TRUE, "The cell geometry is contains a zero area triangle, you probably need to wield vertices.", pNode );
			goto _CLEANUP_AND_FAIL;
		}
		pFace->Normal *= ( 1.0f/fmath_AcuSqrt( fMag2 ) );
		
#if APE_EXPORTER_SWAP_YZ
		pFace->Centroid.Set( Vert.x, Vert.z, Vert.y );
#else
		pFace->Centroid.Set( Vert.x, Vert.y, Vert.z );
#endif

		pFace++;
	}
	pCell->nNumFaces = MnMesh.numf;

	// test this mesh for convexity
	for( i=0; i < (s32)pCell->nNumEdges; i++ ) {
		pEdge = &pCell->aEdges[i];

		pFace = &pCell->aFaces[ pEdge->anFaceIndices[0] ];
		pFace2 = &pCell->aFaces[ pEdge->anFaceIndices[1] ];

		Temp = pFace2->Centroid - pCell->aPts[ pEdge->anPtIndices[0] ].Pos;
		Temp.Unitize();
		fDot = pFace->Normal.Dot( Temp );	

		if( fDot > 0.0001f ) {
			RecordError( TRUE, "The cell geometry is not convex. Please make the cell geometry convex.", pNode );
			goto _CLEANUP_AND_FAIL;
		}
	}
	for( i=0; i < (s32)pCell->nNumFaces; i++ ) {
		
		for( j=0; j < (s32)pCell->aFaces[i].nDegree; j++ ) {
			pA = &pCell->aPts[ pCell->aFaces[i].aPtIndices[j] ].Pos;
			pB = &pCell->aFaces[i].Centroid;

			Temp = *pA - *pB;
			
			fDot = Temp.Dot( pCell->aFaces[i].Normal );
			if( fDot > ON_PLANE_EPSILON || fDot < -ON_PLANE_EPSILON ) {
				CStr sErrorCode;
				sErrorCode.printf( "The cell geometry is not convex. At least one of the faces are not completely flat.\n"
								   "\tThe point at [%f, %f, %f] is not coplanar with the face center at [%f, %f, %f].",
								   pA->x, pA->z, pA->y,
								   pB->x, pB->z, pB->y );									
				RecordError( TRUE, sErrorCode, pNode );
				goto _CLEANUP_AND_FAIL;
			}
		}
	}
	for( i=0; i < (s32)pCell->nNumFaces; i++ ) {

		for( j=0; j < (s32)pCell->aFaces[i].nDegree; j++ ) {
			if( j < (s32)(pCell->aFaces[i].nDegree-1) ) {
				pA = &pCell->aPts[ pCell->aFaces[i].aPtIndices[j+1] ].Pos;
				pB = &pCell->aPts[ pCell->aFaces[i].aPtIndices[j] ].Pos;
			} else {
				pA = &pCell->aPts[ pCell->aFaces[i].aPtIndices[0] ].Pos;
				pB = &pCell->aPts[ pCell->aFaces[i].aPtIndices[j] ].Pos;
			}
			Temp = *pA - *pB;
			
			fDot = Temp.Dot( pCell->aFaces[i].Normal );
			if( fDot > ON_PLANE_EPSILON || fDot < -ON_PLANE_EPSILON ) {
				CStr sErrorCode;
				sErrorCode.printf( "The cell geometry is not convex. At least one of the faces are not completely flat.\n"
								   "\tThe edge [%f, %f, %f] to [%f, %f, %f] is not coplanar with the face normal.",
								   pA->x, pA->z, pA->y,
								   pB->x, pB->z, pB->y );									
				RecordError( TRUE, sErrorCode, pNode );
				goto _CLEANUP_AND_FAIL;
			}
		}
	}

	// calculate a bounding sphere for this cell
	BBox = MnMesh.getBoundingBox(); 
	Center = BBox.Center(); 
	Vert = Center * TMAfterWSM;
	Width = BBox.Width();
	
	pCell->Sphere.m_fRadius = FLength( Width ) * 0.5f;
#if APE_EXPORTER_SWAP_YZ
	pCell->Sphere.m_Pos.Set( Vert.x, Vert.z, Vert.y );
#else
	pCell->Sphere.m_Pos.Set( Vert.x, Vert.y, Vert.z );
#endif

	// record the name of this cell
	strncpy( pCell->szName, pNode->GetName(), (VIS_NAME_LEN-1) );

	// update the volume vars
	pVol->nNumCells++;

	if( pVol->nNumCells > 1 ) {
		pA = &pCell->Sphere.m_Pos;
		pB = &pVol->Sphere.m_Pos;
		UnitAtoB = *pB - *pA;
		fMag2 = UnitAtoB.Mag2();
		if( fMag2 > 0.01f ) {
			UnitAtoB *= ( 1.0f/fmath_AcuSqrt( fMag2 ) );
			P1 = UnitAtoB * -pCell->Sphere.m_fRadius;
			P1 += *pA;

			P2 = P1;
			P2 += UnitAtoB * (2.0f * pCell->Sphere.m_fRadius);
			
			P3 = UnitAtoB * pVol->Sphere.m_fRadius;
			P3 += *pB;

			Temp = P2 - P1;
			fD1 = Temp.Mag();
			Temp = P3 - P1;
			fD2 = Temp.Mag();
			if( fD1 > fD2 ) {
				pVol->Sphere.m_fRadius = fD1 * 0.5f;
				pVol->Sphere.m_Pos.ReceiveLerpOf( 0.5f, P2, P1 );
			} else {
				pVol->Sphere.m_fRadius = fD2 * 0.5f;
				pVol->Sphere.m_Pos.ReceiveLerpOf( 0.5f, P3, P1 );
			}		
		} else {
			// too small to unitize, just see if we need to expand our radius
			if( pVol->Sphere.m_fRadius < pCell->Sphere.m_fRadius ) {
				pVol->Sphere.m_fRadius = pCell->Sphere.m_fRadius;
			}
		}
	} else {
		pVol->Sphere = pCell->Sphere;
	}
		
	if( bNeedDel ) {
		delete pTri;
	}

	return TRUE;

_CLEANUP_AND_FAIL:

	if( bNeedDel ) {
		delete pTri;
	}
	fang_MemZero( pCell, sizeof( ApeVisCell_t ) );

	return FALSE;
}

// assumes that pNode is a SHAPE_CLASS_ID named "port_"
void ApeExporter::ProcessPortalNode( INode *pNode, ObjectState &os ) {
	Matrix3 tm;
	PolyShape PolyShape;
	int nNumLines, i, nNumVerts, nCorner;
	ShapeObject *pShape;
	BOOL bIsClosed;
	PolyPt *pPolyPt;
	Point3 PosWS;
	CFVec3 V1, V2, Cross;
	f32 fDot;
	CPortalStringParser StringParser;

	pShape = (ShapeObject *)os.obj;

	// We will output shapes as a collection of polylines.
	// Each polyline contains collection of line segments.
	pShape->MakePolyShape( 0, PolyShape );
	nNumLines = PolyShape.numLines;
	if( nNumLines != 1 ) {
		RecordError( TRUE, "Portals must be constructed with only 1 closed, rectangular shape. Please fix the portal shape.", pNode );
		return;// keep enumerating
	}
	nNumVerts = PolyShape.lines[0].numPts;
	bIsClosed = PolyShape.lines[0].IsClosed();
	if( nNumVerts < 1 || !bIsClosed ) {
		RecordError( TRUE, "Portals must be constructed with only 1 closed, rectangular shape. Please fix the portal shape.", pNode );
		return;// keep enumerating
	}
		
	// grab the location info
	tm = pNode->GetObjTMAfterWSM( 0 );

	// make sure that this is a rectangle (count the knots)
	nCorner = 0;
	for( i=0; i < nNumVerts; i++ ) {
		pPolyPt = &PolyShape.lines[0].pts[i];

		if( pPolyPt->flags & POLYPT_KNOT ) {
			nCorner++;
		}
	}
	if( nCorner != 4 ) {
		RecordError( TRUE, "Portals must be constructed from rectanglar spline shapes.  Please fix the portal shape.", pNode );
		return;// keep enumerating
	}
		
	nCorner = 0;
	// We only want true knots
	for( i=0; i < nNumVerts; i++ ) {
		pPolyPt = &PolyShape.lines[0].pts[i];

		if( pPolyPt->flags & POLYPT_KNOT ) {
			PosWS = tm * pPolyPt->p;
#if APE_EXPORTER_SWAP_YZ
			m_paPortals[m_nPortalsUsed].aCorners[nCorner].Set( PosWS.x, PosWS.z, PosWS.y );
#else
			m_paPortals[m_nPortalsUsed].aCorners[nCorner].Set( PosWS.x, PosWS.y, PosWS.z );
#endif
			nCorner++;
			if( nCorner >= 4 ) {
				// done
				break;
			}
		}
	}

	// test the portal for co-planarness
	V1 = m_paPortals[m_nPortalsUsed].aCorners[2] - m_paPortals[m_nPortalsUsed].aCorners[1];
	V2 = m_paPortals[m_nPortalsUsed].aCorners[0] - m_paPortals[m_nPortalsUsed].aCorners[1];
	m_paPortals[m_nPortalsUsed].Normal = V1.Cross( V2 );
	m_paPortals[m_nPortalsUsed].Normal *= (1.0f/fmath_AcuSqrt( m_paPortals[m_nPortalsUsed].Normal.Mag2() ) );

	V1 = m_paPortals[m_nPortalsUsed].aCorners[0] - m_paPortals[m_nPortalsUsed].aCorners[3];
	V2 = m_paPortals[m_nPortalsUsed].aCorners[2] - m_paPortals[m_nPortalsUsed].aCorners[3];
	Cross = V1.Cross( V2 );
	Cross.Unitize();

	fDot = Cross.Dot( m_paPortals[m_nPortalsUsed].Normal );
	if( fDot < 0.99f ) {
		// not co-planar
		RecordError( TRUE, "Portals must lie in extactly 1 plane. Please fix the portal shape.", pNode );
		return;// keep enumerating
	}

	// compute a centroid for the portal
	m_paPortals[m_nPortalsUsed].Centroid = m_paPortals[m_nPortalsUsed].aCorners[0];
	m_paPortals[m_nPortalsUsed].Centroid += m_paPortals[m_nPortalsUsed].aCorners[1];
	m_paPortals[m_nPortalsUsed].Centroid += m_paPortals[m_nPortalsUsed].aCorners[2];
	m_paPortals[m_nPortalsUsed].Centroid += m_paPortals[m_nPortalsUsed].aCorners[3];
	m_paPortals[m_nPortalsUsed].Centroid *= (1.0/4.0f);

	// parse the name for portal flags
	StringParser.Parse( pNode->GetName() );
	m_paPortals[m_nPortalsUsed].nFlags = StringParser.m_nCommands;

	// record the name of this portal
	strncpy( m_paPortals[m_nPortalsUsed].szName, pNode->GetName(), (VIS_NAME_LEN-1) );

	m_nPortalsUsed++;
	FASSERT( m_nPortalsUsed <= m_nNumPortalsAllocated );
}

void ApeExporter::GetFangAnimationMtxFromNode( INode *pNode, s32 nTime, CFMtx43 *pFangMtx ) {
	Matrix3 matTM, matRelativeTM, matParentTM;

	// grab the current TM
	matTM = pNode->GetNodeTM( nTime );

    if( pNode->GetParentNode() == NULL ) {
		// no parent
        matRelativeTM = matTM;
    } else {
        matParentTM = pNode->GetParentNode()->GetNodeTM( nTime );
        if( memcmp( &matTM, &matParentTM, 12*sizeof( float ) ) == 0 ) {
			// the parent matrix is the same as our matrix, set the relative mtx to identity
            matRelativeTM.IdentityMatrix();
        } else {
            matRelativeTM = matTM * Inverse( matParentTM );
        }
    }

	// convert the relative TM to FANG coordinate system
	CFMtx43 Rot;
	Rot = (CFMtx43 &)matRelativeTM;
	*pFangMtx = m_Left2RightCoordSysMtx * Rot * m_Left2RightCoordSysMtx;
}

BOOL ApeExporter::FixupObjAndShapeParentIndices( int nStartingFileLoc ) {
	u32 i, j, nHandle, nByteOffset, nHandleOffset;
	INode *pNode, *pParent;
	ApeObject_t *pApeObj;
	ApeShape_t *pApeShape;
	int nEOFLoc, nNumBytesNeeded;
	u8 *pMem = NULL;
	
	// see if there is anything to fixup
	if( m_MeshInfo.nNumObjects == 0 &&
		m_MeshInfo.nNumShapes == 0 ) {
		// nothing to fixup
		return TRUE;
	}
	FASSERT( (m_MeshInfo.nNumObjects + m_MeshInfo.nNumShapes) == m_nHandlesUsed );

	// determine how much memory we need to allocate
	nEOFLoc = ftell( m_pFileStream );
	nNumBytesNeeded = nEOFLoc - nStartingFileLoc;
	pMem = (u8 *)fang_Malloc( nNumBytesNeeded, 4 );
	if( !pMem ) {
		// could not allocate memory
		return FALSE;
	}
	// read in the chunk of the file that we are going to fix up
	fseek( m_pFileStream, nStartingFileLoc, SEEK_SET );
	if( fread( pMem, nNumBytesNeeded, 1, m_pFileStream ) != 1 ) {
		// could not read in the file
		goto _RETURN_ERROR;
	}

	nByteOffset = 0;
	nHandleOffset = 0;

	// process any objects
	for( i=0; i < m_MeshInfo.nNumObjects; i++ ) {
		pNode = m_pInterface->GetINodeByHandle( m_panNodeHandles[nHandleOffset] );
		FASSERT( pNode );
		nHandleOffset++;

		// set our ape object ptr
		pApeObj = (ApeObject_t *)&pMem[nByteOffset];
		nByteOffset += sizeof( ApeObject_t ) + pApeObj->nBytesOfUserData;

		pParent = pNode->GetParentNode();
		while( !pParent->IsRootNode() ) {
			nHandle = pParent->GetHandle();

			// see if this handle appears in our list of exported objects & shape handles
			for( j=0; j < m_nHandlesUsed; j++ ) {
				if( m_panNodeHandles[j] == nHandle ) {
					// this is our parent, fixup our index an move to the next object
					pApeObj->nParentIndex = j + 1;
					break;
				}
			}
			if( pApeObj->nParentIndex ) {
				// we must have found our parent, move on
				break;
			}

			// get the next parent in the chain
			pParent = pParent->GetParentNode();
		}
	}
	
	// advance past any fog structs that many be in the file
	nByteOffset += sizeof( ApeFog_t ) * m_MeshInfo.nNumFogs;

	// process any shapes
	for( i=0; i < m_MeshInfo.nNumShapes; i++ ) {
		pNode = m_pInterface->GetINodeByHandle( m_panNodeHandles[nHandleOffset] );
		FASSERT( pNode );
		nHandleOffset++;

		// set our ape shape ptr
		pApeShape = (ApeShape_t *)&pMem[nByteOffset];
		nByteOffset += sizeof( ApeShape_t ) + pApeShape->nBytesOfUserData;

		pParent = pNode->GetParentNode();
		while( !pParent->IsRootNode() ) {
			nHandle = pParent->GetHandle();

			// see if this handle appears in our list of exported objects & shape handles
			for( j=0; j < m_nHandlesUsed; j++ ) {
				if( m_panNodeHandles[j] == nHandle ) {
					// this is our parent, fixup our index an move to the next object
					pApeShape->nParentIndex = j + 1;
					break;
				}
			}
			if( pApeShape->nParentIndex ) {
				// we must have found our parent, move on
				break;
			}

			// get the next parent in the chain
			pParent = pParent->GetParentNode();
		}
	}

	// write out the converted data
	fseek( m_pFileStream, nStartingFileLoc, SEEK_SET );
	WriteToFile( pMem, nNumBytesNeeded, 1, FALSE );

	fang_Free( pMem );
	fseek( m_pFileStream, 0, SEEK_END );
	
	return TRUE;

_RETURN_ERROR:
	fang_Free( pMem );
	fseek( m_pFileStream, 0, SEEK_END );
	return FALSE;
}



