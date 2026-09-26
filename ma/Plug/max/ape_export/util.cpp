//////////////////////////////////////////////////////////////////////////////////////
// util.cpp - 
//
// Author: Michael Starich   
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
// 12/12/00 Starich     Created.
//////////////////////////////////////////////////////////////////////////////////////
#include "SceneExport.h"
#include "fang.h"
#include "ISkin.h"
#include "stdmat.h"
#include "fmath.h"
#include "modstack.h"

BOOL ApeExporter::AreVertsEqual( ApeVert_t &V1, ApeVert_t &V2 ) {
	
	if( !(V1.Pos == V2.Pos) ) {
		return FALSE;
	}
	if( !(V1.Norm == V2.Norm) ) {
		return FALSE;
	}
	if( !(V1.Color == V2.Color) ) {
		return FALSE;
	}
	for( u32 i=0; i < MAX_LAYERS_PER_MAT; i++ ) {
		if( !(V1.aUV[i] == V2.aUV[i]) ) {
			return FALSE;
		}
	}
	if( !(V1.fNumWeights == V2.fNumWeights) ) {
		return FALSE;
	}
	for( i=0; i < MAX_WEIGHTS_PER_VERT; i++ ) {
		if( !(V1.aWeights[i].fBoneIndex == V2.aWeights[i].fBoneIndex) ) {
			return FALSE;
		}
		if( !(V1.aWeights[i].fWeight == V2.aWeights[i].fWeight) ) {
			return FALSE;
		}
	}
	return TRUE;
}

BOOL ApeExporter::IsTriDegenerative( ApeVertIndex_t &Index1, ApeVertIndex_t &Index2, ApeVertIndex_t &Index3 ) {
	
	if( Index1.nVertIndex == Index2.nVertIndex ) {
		return TRUE;
	}
	if( Index1.nVertIndex == Index3.nVertIndex ) {
		return TRUE;
	}
	if( Index2.nVertIndex == Index3.nVertIndex ) {
		return TRUE;
	}
	return FALSE;
}

BOOL ApeExporter::IsNodeBone( INode *pNode ) {

	if( pNode == NULL || pNode->IsRootNode() ) {
		return FALSE;
	}

	ObjectState os = pNode->EvalWorldState( 0 );
	if( os.obj->ClassID() == Class_ID( BONE_CLASS_ID, 0 ) ) {
		return TRUE;
	}
	if( os.obj->ClassID() == BONE_OBJ_CLASSID ) {
		return TRUE;
	}
	// we don't want biped end effectors
	if( os.obj->ClassID() == Class_ID( DUMMY_CLASS_ID, 0 ) ) {
		// only count dummys as bones if there it is named "bone_"
		return ( IsNodeNamedBone( pNode ) );
	}

	return FALSE;
}

// Get the number of direct child bones of a node
int ApeExporter::GetChildBoneCount( INode *pNode ) {
	int nCount = 0;

	for( int i=0; i < pNode->NumberOfChildren(); i++ ) {
		if( IsNodeBone( pNode->GetChildNode( i ) ) ) {
			nCount++;
		}
	}
	return nCount;
}

INode *ApeExporter::GetChildBoneNode( INode *pNode, u32 nIndex ) {
	
	int nChildBoneCount = GetChildBoneCount( pNode );
	if( (int)nIndex >= nChildBoneCount ) {
		// invalid index
		return NULL;
	}
	INode *pChild;
	int nCount = 0;
	for( int i=0; i < pNode->NumberOfChildren(); i++ ) {
		pChild = pNode->GetChildNode( i );
		if( IsNodeBone( pChild ) ) {
			if( nCount == nIndex ) {
				return pChild;
			}
			nCount++;
		}
	}
	return NULL;
}

// Get the total number of bones in the scene, this should get passed the root node
int ApeExporter::GetBoneCount( INode *pRoot ) {
	int nCount = 0;

	char *pszName = pRoot->GetName();
	if( IsNodeBone( pRoot ) ) {
		nCount += 1;
	}
	int nNumChildren = pRoot->NumberOfChildren();
	for( int i=0; i < nNumChildren; i++ ) {
		nCount += GetBoneCount( pRoot->GetChildNode( i ) );
	}
	return nCount;
}

// Recursive iterator to get a bone index, used with GetBoneIndex
int ApeExporter::RecursiveGetBoneIndex( INode *pRoot, INode *pNodeTest, int &nBoneCount ) {
	int nBoneIdx = -1;

	if( IsNodeBone( pRoot ) ) {
		nBoneIdx = nBoneCount;
		nBoneCount++;

		if( pRoot == pNodeTest ) {
			return nBoneIdx;
		}
	}
	// recurse child nodes
	for( int i=0; i < pRoot->NumberOfChildren(); i++ ) {
		nBoneIdx = RecursiveGetBoneIndex( pRoot->GetChildNode( i ), pNodeTest, nBoneCount );
		if( nBoneIdx >= 0 ) {
			return nBoneIdx;
		}
	}
	return -1;
}

// Get an index from a node pointer
int ApeExporter::GetBoneIndex( INode *pRoot, INode *pNodeTest ) {
	
	if( !IsNodeBone( pNodeTest ) ) {
		return -1;
	}
	int nBoneCount = 0;
	return RecursiveGetBoneIndex( pRoot, pNodeTest, nBoneCount );
}

// Recursive iterator to get a bone node *, used with GetBoneNode
INode *ApeExporter::RecursiveGetBoneNode( INode *pRoot, u32 nIndex, int &nBoneCount ) {
	int nBoneIdx = -1;

	if( IsNodeBone( pRoot ) ) {
		nBoneIdx = nBoneCount;
		nBoneCount++;

		if( nIndex == nBoneIdx ) {
			return pRoot;
		}
	}
	// recures child nodes
	
	for( int i=0; i < pRoot->NumberOfChildren(); i++ ) {
		INode *pNode = RecursiveGetBoneNode( pRoot->GetChildNode( i ), nIndex, nBoneCount );
		if( pNode ) {
			return pNode;
		}
	}
	return NULL;
}

// Get an INode * from the root, returns NULL if nIndex can't be found
INode *ApeExporter::GetBoneNode( INode *pRoot, u32 nIndex ) {
	int nTotalBoneCount = GetBoneCount( pRoot );
	
	if( nTotalBoneCount <= 0 ) {
		// there are no bones attached to pRoot
		return NULL;
	}
	if( (int)nIndex >= nTotalBoneCount ) {
		// invalid bone index
		return NULL;
	}
	int nBoneCount = 0;
	return RecursiveGetBoneNode( pRoot, nIndex, nBoneCount );
}

Modifier *ApeExporter::FindSkinModifier( INode *pNode ) {

	Object *pObj = pNode->GetObjectRef();
	if( !pObj ) {
		return NULL;
	}
	while( pObj->SuperClassID() == GEN_DERIVOB_CLASS_ID ) {
		IDerivedObject *pDerivedObj = static_cast<IDerivedObject*>(pObj);

		int nStackIdx = 0;
		while( nStackIdx < pDerivedObj->NumModifiers() ) {
			Modifier* pMod = pDerivedObj->GetModifier( nStackIdx++ );
			if( pMod->ClassID() == SKIN_CLASSID ) {
				return pMod;
			}
		}
		pObj = pDerivedObj->GetObjRef();
	}
	return NULL;
}

BOOL ApeExporter::IsNodeMesh( INode *pNode ) {

	ObjectState os = pNode->EvalWorldState( 0 ); 
	if( !pNode->IsRootNode() && 
		os.obj->SuperClassID() == GEOMOBJECT_CLASS_ID ) {
		return TRUE;
	}
	return FALSE;
}

BOOL ApeExporter::IsNodeSkinned( INode *pNode ) {
	
	if( !IsNodeMesh( pNode ) ) {
		// pNode is not a mesh node
		return FALSE;
	}
	int nBoneCount = GetBoneCount( m_pInterface->GetRootNode() );
	if( nBoneCount <= 0 ) {
		// no bones in this scene
		return FALSE;
	}
	// try to get a skin modifier
	Modifier *pMod = FindSkinModifier( pNode );
	if( !pMod ) {
		// no skin modifier, sorry charlie
		return FALSE;
	}	
	// try to get the interfaces that we will need
	ISkin *pSkin = (ISkin *)pMod->GetInterface( I_SKIN );
	if( !pSkin ) {
		return FALSE;
	}
	ISkinContextData *pSkinContext = pSkin->GetContextInterface( pNode );
	if( !pSkinContext ) {
		pMod->ReleaseInterface( I_SKIN, pSkin );
		return FALSE;
	}
	int nNumPoints = pSkinContext->GetNumPoints();
	if( nNumPoints <= 0 ) {
		pMod->ReleaseInterface( I_SKIN, pSkin );
		return FALSE;
	}
	pMod->ReleaseInterface( I_SKIN, pSkin );
	// if we get here, then this node is definitely skinned
	return TRUE;	
}

// Return a pointer to a TriObject given an INode or return NULL
// if the node cannot be converted to a TriObject
TriObject *ApeExporter::GetTriObjectFromNode( INode *pNode, TimeValue t, BOOL &bDeleteIt ) {

	bDeleteIt = FALSE;
	Object *obj = pNode->EvalWorldState(t).obj;
	if( obj->CanConvertToType( Class_ID( TRIOBJ_CLASS_ID, 0 ) ) ) { 
		TriObject *tri = (TriObject *)obj->ConvertToType( t, Class_ID( TRIOBJ_CLASS_ID, 0 ) );
		// Note that the TriObject should only be deleted
		// if the pointer to it is not equal to the object
		// pointer that called ConvertToType()
		if( obj != tri ) {
			bDeleteIt = TRUE;
		}
		return tri;
	} else {
		return NULL;
	}
}

Point3 ApeExporter::GetVertexNormal( Mesh *pMesh, int nFaceNum, RVertex *pRV ) {
	Face *pFace = &pMesh->faces[nFaceNum];
	DWORD nSmGroup = pFace->smGroup;
	int numNormals;
	Point3 vertexNormal;
	
	// Is normal specified
	// SPCIFIED is not currently used, but may be used in future versions.
	if( pRV->rFlags & SPECIFIED_NORMAL ) {
		vertexNormal = pRV->rn.getNormal();
	}
	// If normal is not specified it's only available if the face belongs
	// to a smoothing group
	else if ((numNormals = pRV->rFlags & NORCT_MASK) && nSmGroup) {
		// If there is only one vertex is found in the rn member.
		if( numNormals == 1 ) {
			vertexNormal = pRV->rn.getNormal();
		} else {
			// If two or more vertices are there you need to step through them
			// and find the vertex with the same smoothing group as the current face.
			// You will find multiple normals in the ern member.
			for( int i=0; i < numNormals; i++ ) {
				if( pRV->ern[i].getSmGroup() & nSmGroup ) {
					vertexNormal = pRV->ern[i].getNormal();
				}
			}
		}
	} else {
		// Get the normal from the Face if no smoothing groups are there
		vertexNormal = pMesh->getFaceNormal( nFaceNum );
	}
	
	return vertexNormal;
}

BOOL ApeExporter::IsMaterialAComposite( Mtl *pMaxMtl ) {

	if( !pMaxMtl ) {
		return FALSE;
	}
	// see if we got a standard material
	if( pMaxMtl->ClassID() == Class_ID(DMTL_CLASS_ID, 0) ) {
		return FALSE;
	}
	// is this a multi material?
	if( pMaxMtl->IsMultiMtl() ) {
		return FALSE;
	}
	if( pMaxMtl->NumSubMtls() <= 0 ) {
		// can't be a composite, we don't have sub materials
		return FALSE;
	}
	return TRUE;
}

// returns true if pMaxMtl's diffuse texture is a composite
BOOL ApeExporter::IsTextureAComposite( Mtl *pMaxMtl ) {
	
	if( pMaxMtl->ClassID() != Class_ID(DMTL_CLASS_ID, 0) ) {
		// pMaxMtl must be a standard material, perhaps this is a composite material
		return FALSE;
	}
	Texmap *pTexmap = pMaxMtl->GetSubTexmap( ID_DI );
	if( !pTexmap ) {
		// can't grab the Texmap from the diffuse slot
		return FALSE;
	}
	if( pTexmap->ClassID() == Class_ID( BMTEX_CLASS_ID, 0x00 ) ) {
		// this texmap is a regular bitmap
		return FALSE;
	}
	if( pTexmap->ClassID() == Class_ID( COMPOSITE_CLASS_ID, 0x00 ) ) {
		// this texmap is a composite texture
		return TRUE;
	}
	return FALSE;
}

void ApeExporter::CountMaterials( INode *pNode, int &nMatCount ) {
	int i;
	Mtl *pMaxMtl = pNode->GetMtl();
	Mtl *pSubMtl = NULL;

	if( !m_bExportSelected || pNode->Selected() ) {
		if( !IsNodeBone( pNode ) ) {
			if( pMaxMtl ) {
				if( pMaxMtl->IsMultiMtl() ) {
					// Process the sub materials
					int nNumSubMtls = pMaxMtl->NumSubMtls();
					for( i=0; i < nNumSubMtls; i++ ) {
						pSubMtl = pMaxMtl->GetSubMtl( i );
						if( pSubMtl ) {
							if( pSubMtl->SubTexmapOn( ID_DI ) ) {
								nMatCount++;																
							}
						 }
					}
				} else {
					// just one material assigned, assign all faces to one material
					if( pMaxMtl->SubTexmapOn( ID_DI ) ) {
						nMatCount++;
					}
				}
			} else {
				nMatCount++;
			}
		}
	}
	// For each child of this node, we recurse into ourselves 
	// and increment the counter until no more children are found.
	for( i=0; i < pNode->NumberOfChildren(); i++) {
		CountMaterials( pNode->GetChildNode( i ), nMatCount );
	}
}

void ApeExporter::UpdateProgressBar( int nNewPercent ) {
	
	FMATH_CLAMP( nNewPercent, 0, 100 );
	// only update if the new percent is greater than the last update
	if( nNewPercent > m_nLastPercentUpdate ) {
		m_nLastPercentUpdate = nNewPercent;
		m_pInterface->ProgressUpdate( m_nLastPercentUpdate );
	}
}

Matrix3 ApeExporter::GetNodeLocalTM( INode *pNode, TimeValue t ) {
	Matrix3 ParentTM, ChildTM, LocalTM;
	
	if( pNode ) {
		ParentTM = pNode->GetParentTM( t );
		ChildTM  = pNode->GetNodeTM( t );
		LocalTM  = ChildTM * Inverse( ParentTM );
	} else {
		LocalTM.IdentityMatrix();
	}

	return LocalTM;
}

///////////////////////////
// USED FOR SEGMENTED NODES
///////////////////////////

// Get the number of direct child nodes of a node
int ApeExporter::GetChildNodeCount( INode *pNode ) {
	
	return pNode->NumberOfChildren();// max call
}

INode *ApeExporter::GetChildNodeByIndex( INode *pNode, u32 nIndex ) {
	
	if( (int)nIndex >= GetChildNodeCount( pNode ) ) {
		// invalid index
		return NULL;
	}
	return pNode->GetChildNode( nIndex );// max call
}

// Get the total number of nodes in the scene, this should get passed the root node
int ApeExporter::GetTotalNodeCount( INode *pRoot ) {
	int nCount = 1;

	for( int i=0; i < GetChildNodeCount( pRoot ); i++ ) {
		nCount += GetTotalNodeCount( GetChildNodeByIndex( pRoot, i ) );
	}
	return nCount;
}

// Recursive iterator to get a node index, used with GetNodeIndex
int ApeExporter::RecursiveGetNodeIndex( INode *pRoot, INode *pNodeTest, int &nBoneCount ) {
	int nBoneIdx = nBoneCount;
		
	nBoneCount++;
	if( pRoot == pNodeTest ) {
		return nBoneIdx;
	}
	// recurse child nodes
	for( int i=0; i < GetChildNodeCount( pRoot ); i++ ) {
		nBoneIdx = RecursiveGetNodeIndex( GetChildNodeByIndex( pRoot, i ), pNodeTest, nBoneCount );
		if( nBoneIdx >= 0 ) {
			return nBoneIdx;
		}
	}
	return -1;
}

// Get an index from a node pointer
int ApeExporter::GetNodeIndex( INode *pRoot, INode *pNodeTest ) {
	
	int nBoneCount = 0;
	return RecursiveGetNodeIndex( pRoot, pNodeTest, nBoneCount );
}

// Recursive iterator to get a node *, used with GetNodeByIndex
INode *ApeExporter::RecursiveGetNode( INode *pRoot, u32 nIndex, int &nBoneCount ) {
	int nBoneIdx = nBoneCount;
	INode *pNode;
	
	nBoneCount++;
	if( nIndex == nBoneIdx ) {
		return pRoot;
	}
	// recures child nodes
	
	for( int i=0; i < GetChildNodeCount( pRoot ); i++ ) {
		pNode = RecursiveGetNode( GetChildNodeByIndex( pRoot, i ), nIndex, nBoneCount );
		if( pNode ) {
			return pNode;
		}
	}
	return NULL;
}

// Get an INode * from the root, returns NULL if nIndex can't be found
INode *ApeExporter::GetNodeByIndex( INode *pRoot, u32 nIndex ) {
	int nTotalBoneCount = GetTotalNodeCount( pRoot );
	
	if( nTotalBoneCount <= 0 ) {
		// there are no bones attached to pRoot
		return NULL;
	}
	if( (int)nIndex >= nTotalBoneCount ) {
		// invalid bone index
		return NULL;
	}
	int nBoneCount = 0;
	return RecursiveGetNode( pRoot, nIndex, nBoneCount );
}

// Recursive iterator to get a node *, used with GetNodeByIndex
INode *ApeExporter::GetNodeByName( INode *pNode, cchar *pszNodeName ) 
{
	////////////////////
	// compare to the name
	if ( strlen( pNode->GetName() ) > 5 && strncmp( pNode->GetName(), "LOD", 3 ) == 0 && pNode->GetName()[4] == '_' )
	{
		if ( strcmp( pszNodeName, &pNode->GetName()[5] ) == 0 )
		{
			return pNode;
		}
	}
	else if ( strcmp( pszNodeName, pNode->GetName() ) == 0 )
	{
		return pNode;
	}

	for ( int i = 0; i < GetChildNodeCount( pNode ); i++ ) 
	{
		INode *pResultNode = GetNodeByName( GetChildNodeByIndex( pNode, i ), pszNodeName );
		if ( pResultNode ) 
		{
			return pResultNode;
		}
	}

	return NULL;
}


