//////////////////////////////////////////////////////////////////////////////////////
// DoExport.cpp - based on the skeleton exporter max exporter
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
// 10/18/00 Starich     Created.
//////////////////////////////////////////////////////////////////////////////////////
#include "SceneExport.h"
#include "ape_file_def.h"
#include <stdmat.h>
#include "ff32hash.h"
#include "ISkin.h"
#include "fxfm.h"
#include "ErrorLog.h"
#include "MatStringParser.h"
#include "fversion.h"
#include "fclib.h"

//====================
// private definitions

#define _DUMP_BONE_INFO_TO_TEXT_FILE	0	// set to 1 to dump the bone info to a text file, 0 to disable
#define _FLIP_TO_UPPER_LEFT_T_CORNER	1	// set to 1 to make all texture coordinates use the upper left cornor as the origin, 0 for the lower left (max standard)
#define _REMOVE_DUP_VERTS				1	// set to 1 to eliminate duplicate verts from a material's list
#define _REMOVE_DEGENERATIVE_TRIS		1	// set to 1 to eliminate 0 area tris from a material's list
#define _VERTEX_ALPHA_MAP_CHANNEL		MAP_ALPHA// (used to use 99) mapping channel where U will be used as the vertex alpha (if channel is mapped, otherwise 1.0f)
#define _VERTEX_RGB_MAP_CHANNEL			0	// mapping channel where vertex rgb is stored
#define _LIGHTMAP_UV_CHANNEL			99
#define _SMALLEST_POSSIBLE_TRI_AREA2	( 0.000001f * 0.000001f )

//=================
// public variables

//==================
// private variables

CVertexWeights ApeExporter::_VertexWeights;

//===================
// private prototypes

static void _FillVertData( ApeVert_t &Vert, Point3 &MaxVert, Point3 &MaxNorm,
						   Point3 &MaxColor, Point3 &MaxUV1, Point3 &MaxUV2, Point3 &MaxUV3, f32 fVertA );

//=================
// public functions

/*===========================================================================*\
 |  Constructor/Destructor - just initialize any variables or memory
\*===========================================================================*/
ApeExporter::ApeExporter() {

	m_fLightMultiplier = 1.0f;

	m_bApeCfgExportLights = FALSE;
	m_bApeCfgExportGeo = TRUE;
	m_bApeCfgExportSkin = FALSE;
	
	m_bWldCfgExportGeo = TRUE;
	m_bWldCfgExportLights = FALSE;
	m_bWldCfgExportObjects = FALSE;

	m_bExportLights = FALSE;
	m_bExportGeo = TRUE;
	m_bExportSkin = FALSE;
	m_bExportObjects = FALSE;
	m_bWorldFile = TRUE;
	m_bObjectFile = FALSE;

	m_nNumHandlesAllocated = 0;
	m_nHandlesUsed = 0;
	m_panNodeHandles = NULL;

	m_nNumPortalsAllocated = 0;
	m_nPortalsUsed = 0;
	m_paPortals = NULL;
}

ApeExporter::~ApeExporter() {
}

BOOL ApeExporter::CreateApeMaterial( int nMatID, int nNumMats, int nLODIndex, Mesh *pMesh, Matrix3 &TMAfterWSM,
									 Matrix3 &NodeTM, BOOL bSwapTriOrder, Mtl *pBaseMtl, 
									 INode *pNode, u32 nNodeIndex, BOOL bSkinned, ApeMaterial_t &rMatInfo,
									 CFf32Hash &VertHashTable, ApeVertIndex_t *pIndices, u32 nStartingIndex,
									 BOOL bFlipTriWindingOrder, BOOL bLightMapCoord ) {
	int nIndex, nFaceNum, nLayerNum, nVertIndex, nMaxID, nTiling, nFaces;
	Face* pFace;
	Point3 Vert, Norm;
	UVVert UV1, UV2, UV3, Zero, AlphaUV, One;	
	Color RGBColor;
	ApeLayer_t *pLayer;
	Mtl *pSubMtl;
	int nNumIndices, nMapChannel;
	StdMat *pStdMtl;
	Texmap *pTexmap, *pTexmap2;
	cchar *pszTextureName;
	DWORD nWireColor;
	TVFace *pBaseTVList, *pLayer1TVList, *pVtxAlphaTVList, *pVtxRGBTVList;
	TVFace *pBaseTV, *pLayer1TV, *pVtxAlphaTV, *pVtxRGBTV;
	UVVert *pBaseUVList, *pLayer1UVList, *pVtxAlphaUVList, *pRGBList;

	TVFace *pLMapTVList=NULL;
	TVFace *pLMapTV=NULL;
	UVVert *pLMapUVList=NULL;

	VertColor vColor;
	_MtlHelper_t MtlHelper;
	CMatStringParser MatParser, NodeNameParser;
	u32 i;
	CStr sTemp, sErrorCode;
	BOOL bNodeHasWeights;
	ApeVert_t aApeVerts[3];
#if _REMOVE_DEGENERATIVE_TRIS
	ApeVert_t *pV1, *pV2, *pV3;
	CFVec3 Vec1, Vec2, Vec3;
#endif

	///////////////////////////////////////
	// Get the number of faces to this mesh
	nFaces = pMesh->getNumFaces();
	FASSERT( nFaces > 0 );
	
	//////////////////////////////////
	// Zero out our ape material first
	ZeroMemory( &rMatInfo, sizeof( ApeMaterial_t ) );

	rMatInfo.nLODIndex = nLODIndex;

	if( !ParseBaseMtl( pBaseMtl,  MtlHelper, pNode ) ) {
		// something happened while parsing the mtl, the error has already been logged
		return FALSE;
	}

	///////////////////////////////////
	// fill in some of our ape material
	rMatInfo.nLayersUsed = ( MtlHelper.bCompositeMat || MtlHelper.bCompositeTex ) ? 2 : 1;
	rMatInfo.nFirstIndex = nStartingIndex;
	bNodeHasWeights = IsNodeSkinned( pNode );

	///////////////////////////////////////////////////////////////////////////////
	// Parse the node name for any * commands to use as a default for each material
	NodeNameParser.Parse( pNode->GetName(), TRUE );
	if( NodeNameParser.GetNumberOfErrors() ) {
		// there were errors, we need to report these 
		for( i=0; i < NodeNameParser.GetNumberOfErrors(); i++ ) {
			NodeNameParser.GetErrorString( i, sTemp );
			sErrorCode.printf( "The node name parser for - %s returns the following error - %s", pNode->GetName(), sTemp );
			RecordError( FALSE, sErrorCode, pNode );	
		}
	}
	
	/////////////////////////////////////////
	// Parse the material name for * commands
	MatParser.ResetToDefaults();
	MatParser.m_ApeCommands = NodeNameParser.m_ApeCommands;// start with the commands gathered from the node name as the default for each material
	if( pBaseMtl ) {
		MatParser.Parse( pBaseMtl->GetName(), FALSE );
		if( MatParser.GetNumberOfErrors() ) {
			// there were errors, we need to report these 
			for( i=0; i < MatParser.GetNumberOfErrors(); i++ ) {
				MatParser.GetErrorString( i, sTemp );
				sErrorCode.printf( "The material name parser for - %s returns the following error - %s", pBaseMtl->GetName(), sTemp );
				RecordError( FALSE, sErrorCode, pNode );	
			}
		}
	}
	rMatInfo.StarCommands = MatParser.m_ApeCommands;
	rMatInfo.nAffectAngle = MatParser.m_nAffectAngle;
	rMatInfo.nFlags = MatParser.m_nMatFlags;
	// assign a shader if none was assigned
	if( rMatInfo.StarCommands.nShaderNum < 0 ) {
		if( rMatInfo.nLayersUsed == 1 ) {
			rMatInfo.StarCommands.nShaderNum = APE_SHADER_TYPE_oBASE;
		} else {
			if( MtlHelper.bCompositeTex ) {
				rMatInfo.StarCommands.nShaderNum = APE_SHADER_TYPE_oBASE_MOD_SHADOWMAP;	
			} else {
				rMatInfo.StarCommands.nShaderNum = APE_SHADER_TYPE_oBASE_LERP_pLAYER;
			}
		}
	}
 
	///////////////////////////////////////////////////////////////
	// Run through the layers, setting up infomation about each one
	for( nLayerNum=0; nLayerNum < (s32)rMatInfo.nLayersUsed; nLayerNum++ ) {
		pLayer = &rMatInfo.aLayer[nLayerNum];
		// set defaults, remember everything has already been set to 0
		pLayer->fUnitAlphaMultiplier = 1.0f;

		// set pSubMtl to the proper max material
		if( MtlHelper.bCompositeMat ) {
			pSubMtl = pBaseMtl->GetSubMtl( MtlHelper.anValidLayerIndices[nLayerNum] );
		} else {
			pSubMtl = ( MtlHelper.bUseSubMats ) ? pBaseMtl->GetSubMtl( MtlHelper.anValidLayerIndices[0] ) : pBaseMtl;
		}

		// parse the material name for * commands
		if( (u32)pSubMtl != (u32)pBaseMtl ) {
			MatParser.ResetToDefaults();
			MatParser.m_ApeCommands = NodeNameParser.m_ApeCommands;// start with the commands gathered from the node name as the default for each material
			MatParser.Parse( pSubMtl->GetName(), FALSE );
			if( MatParser.GetNumberOfErrors() ) {
				// there were errors, we need to report these 
				for( i=0; i < MatParser.GetNumberOfErrors(); i++ ) {
					MatParser.GetErrorString( i, sTemp );
					sErrorCode.printf( "The material name parser for - %s returns the following error - %s", pSubMtl->GetName(), sTemp );
					RecordError( FALSE, sErrorCode, pNode );	
				}
			}
		}
		pLayer->StarCommands = MatParser.m_ApeCommands;
//		pLayer->nFlags = MatParser.m_nMatFlags;

		// fill in this layer's info
		if( MtlHelper.bMapToErrorMat ) {
			CreateErrorLayer( *pLayer );
		} else {
			// first copy the texture name into the layer struct (We get it from the "Diffuse Color" entry in the material editor)
			pszTextureName = ExtractTextureName( pSubMtl, MtlHelper.anValidLayerIndices[nLayerNum], ID_DI );
			if( pszTextureName ) {
				pLayer->bTextured = TRUE;
				memcpy( pLayer->szTexnames[APE_LAYER_TEXTURE_DIFFUSE], pszTextureName, TEXTURE_NAME_LEN );
			} else {
				sErrorCode.printf( "Could not extract the texture name - skipping material (the %d'th one).", nMatID+1 );
				RecordError( FALSE, sErrorCode, pNode );
				return FALSE;
			}

			// Copy the Alpha Mask (We get it from the "Opacity" entry in the material editor)
			pszTextureName = ExtractTextureName( pSubMtl, MtlHelper.anValidLayerIndices[nLayerNum], ID_OP );
			if( pszTextureName ) {
				memcpy( pLayer->szTexnames[APE_LAYER_TEXTURE_ALPHA_MASK], pszTextureName, TEXTURE_NAME_LEN );
			}

			// Copy the Specular Mask (We get it from the "Specular Level" entry in the material editor
			pszTextureName = ExtractTextureName( pSubMtl, MtlHelper.anValidLayerIndices[nLayerNum], ID_SH );
			if( pszTextureName ) {
				memcpy( pLayer->szTexnames[APE_LAYER_TEXTURE_SPECULAR_MASK], pszTextureName, TEXTURE_NAME_LEN );
			}

			// Copy the Emissive Mask (We get it from the "Self-Illumination" entry in the material editor
			pszTextureName = ExtractTextureName( pSubMtl, MtlHelper.anValidLayerIndices[nLayerNum], ID_SI );
			if( pszTextureName ) {
				memcpy( pLayer->szTexnames[APE_LAYER_TEXTURE_EMISSIVE_MASK], pszTextureName, TEXTURE_NAME_LEN );
				TextureOutput *pTO = ((BitmapTex *)pSubMtl->GetSubTexmap( ID_SI ))->GetTexout();
				if ( pTO->GetInvert() )
				{
					rMatInfo.nFlags |= APE_LAYER_FLAGS_INVERT_EMISSIVE_MASK;
				}
			}

			// Copy the Environment Map (We get it from the "Reflection" entry in the material editor
			pszTextureName = ExtractTextureName( pSubMtl, MtlHelper.anValidLayerIndices[nLayerNum], ID_RL );
			if( pszTextureName ) {
				memcpy( pLayer->szTexnames[APE_LAYER_TEXTURE_ENVIRONMENT], pszTextureName, TEXTURE_NAME_LEN );
			}

			// Copy the Bump Map (We get it from the "Bump" entry in the material editor
			pszTextureName = ExtractTextureName( pSubMtl, MtlHelper.anValidLayerIndices[nLayerNum], ID_BU );
			if( pszTextureName ) 
			{
				memcpy( pLayer->szTexnames[APE_LAYER_TEXTURE_BUMP], pszTextureName, TEXTURE_NAME_LEN );
				// Originally, we were going to read the UV tile to determine the bump and detail tile factor
				// but since we needed a default of 4.0 for the detail map, both bump and detail are handled
				// through the star commands, unfortunately.
//				BitmapTex *pBMT = (BitmapTex *)pSubMtl->GetSubTexmap( ID_BU );
//				pLayer->fBumpMapTileFactor = ((StdUVGen *)pBMT->GetUVGen())->GetUScl(0);
			}

			// Copy the Detail Map (We get it from the "Displacement" entry in the material editor
			pszTextureName = ExtractTextureName( pSubMtl, MtlHelper.anValidLayerIndices[nLayerNum], ID_DP );
			if( pszTextureName ) 
			{
				memcpy( pLayer->szTexnames[APE_LAYER_TEXTURE_DETAIL], pszTextureName, TEXTURE_NAME_LEN );
				// Originally, we were going to read the UV tile to determine the bump and detail tile factor
				// but since we needed a default of 4.0 for the detail map, both bump and detail are handled
				// through the star commands, unfortunately.
//				BitmapTex *pBMT = (BitmapTex *)pSubMtl->GetSubTexmap( ID_BU );
//				pLayer->fDetailMapTileFactor = ((StdUVGen *)pBMT->GetUVGen())->GetUScl(0);
			}

			if( pSubMtl ) {
				pTexmap = NULL;
				pTexmap2 = pSubMtl->GetSubTexmap( ID_DI );
				if( pTexmap2 ) {
					if( !MtlHelper.bUseSubTexs ) {
						if( pTexmap2->ClassID() == Class_ID( BMTEX_CLASS_ID, 0 ) ) {
							pTexmap = pTexmap2;
						}
					} else {
						// we need to use sub texmaps, probably a composite texture
						pTexmap = pTexmap2->GetSubTexmap( MtlHelper.anValidLayerIndices[nLayerNum] );
					}
					if( pTexmap ) {
						// set our tiling parameters
						nTiling = pTexmap->GetTextureTiling();
						pLayer->abTile[0] = (nTiling & U_WRAP) ? TRUE : FALSE;
						pLayer->abTile[1] = (nTiling & V_WRAP) ? TRUE : FALSE;
#if 0
						BitmapTex *pBitmapTex = (BitmapTex *)pTexmap;
						if( pBitmapTex ) {
							StdUVGen *pStdUVGen = pBitmapTex->GetUVGen();
							if( pStdUVGen ) {
								pLayer->fBlur = pStdUVGen->GetBlur( 0 );
								pLayer->fBlurOffset = pStdUVGen->GetBlurOffs( 0 );
								pLayer->nNoiseLevel = pStdUVGen->GetNoiseLev( 0 );
							}
							Bitmap *pBitmap = pBitmapTex->GetBitmap( 0 );
							if( pBitmap ) {
								pBitmap->Width();
								pBitmap->Height();
							}
						}
#endif
					}
				}
				// fill in some layer properties
//pLayer->fOpacity = 1.0f - pSubMtl->GetXParency();
				if ( (MatParser.m_ApeCommands.nFlags & APE_MAT_FLAGS_APPLY_TINT) && nLayerNum == 0 )
				{
					// Pick up the diffuse color of the first layer as the tint
					RGBColor = pSubMtl->GetDiffuse();
					rMatInfo.StarCommands.TintRGB.Set( RGBColor.r, RGBColor.g, RGBColor.b );
				}
//				RGBColor = pSubMtl->GetDiffuse();
//				pLayer->DiffuseRGB.Set( RGBColor.r, RGBColor.g, RGBColor.b );
				if( pSubMtl->GetShinStr() <= 0.05f ) {
					pLayer->fShininess = 0.0f;
					pLayer->fShinStr = 0.0f;
				} else {
					// the specualar level is on
					pLayer->fShininess = pSubMtl->GetShininess() * 127.0f;
					pLayer->fShinStr = pSubMtl->GetShinStr();
					FMATH_CLAMP( pLayer->fShinStr, 0.0f, 1.0f );
				}			
				RGBColor = pSubMtl->GetSpecular();
				pLayer->SpecularRGB.Set( RGBColor.r, RGBColor.g, RGBColor.b );
				if( pSubMtl->GetSelfIllumColorOn() ) {
					RGBColor = pSubMtl->GetSelfIllumColor();
					pLayer->SelfIllumRGB.Set( RGBColor.r, RGBColor.g, RGBColor.b );
				} else {
					pLayer->SelfIllumRGB.Set( pSubMtl->GetSelfIllum() );
				}
				pStdMtl = (StdMat *)pSubMtl;
				pLayer->bDrawAsWire = pStdMtl->GetWire();
				pLayer->bTwoSided = pStdMtl->GetTwoSided();
				pLayer->fUnitAlphaMultiplier = pStdMtl->GetTexmapAmt( ID_RL, 0 );
			} else {
				// no mat, grab the wire frame color
				nWireColor = pNode->GetWireColor();
//				RGBColor = Color( GetRValue( nWireColor )/255.0f, GetGValue( nWireColor )/255.0f, GetBValue( nWireColor )/255.0f );
//				pLayer->DiffuseRGB.Set( RGBColor.r, RGBColor.g, RGBColor.b );
				pLayer->SelfIllumRGB.Set( 1.0f );
//pLayer->fOpacity = 1.0f;
			}
		}
	}
	
	////////////////////////////////////////////////////
	// Setup our TV and UV lists for texture coordinates
	if( MtlHelper.bMapToErrorMat ) {
		// an error material doesn't need texture coordinates
		pBaseTVList = NULL;
		pBaseUVList = NULL;
		pLayer1TVList = NULL;
		pLayer1UVList = NULL;
	} else {
		if( MtlHelper.bCompositeMat ) {
			// first the base layer
			pSubMtl = pBaseMtl->GetSubMtl( MtlHelper.anValidLayerIndices[0] );
			pTexmap = pSubMtl->GetSubTexmap( ID_DI );
			nMapChannel = ( pTexmap ) ? pTexmap->GetMapChannel() : 1;
			pBaseTVList = pMesh->mapFaces( nMapChannel );
			pBaseUVList = pMesh->mapVerts( nMapChannel );
			// now layer 1
			pSubMtl = pBaseMtl->GetSubMtl( MtlHelper.anValidLayerIndices[1] );
			pTexmap = pSubMtl->GetSubTexmap( ID_DI );
			nMapChannel = ( pTexmap ) ? pTexmap->GetMapChannel() : 1;
			pLayer1TVList = pMesh->mapFaces( nMapChannel );
			pLayer1UVList = pMesh->mapVerts( nMapChannel );
		} else {
			if( MtlHelper.bUseSubMats ) {
				pSubMtl = pBaseMtl->GetSubMtl( MtlHelper.anValidLayerIndices[0] );
			} else {
				pSubMtl = pBaseMtl;
			}
			if( !pSubMtl ) {
				// this is a NULL material, no texture coordinates
				pBaseTVList = NULL;
				pBaseUVList = NULL;
				pLayer1TVList = NULL;
				pLayer1UVList = NULL;
			} else {
				pTexmap = pSubMtl->GetSubTexmap( ID_DI );
				if( !pTexmap ) {
					// no texmap, default to channel 1 - single layer
					nMapChannel = 1;
					pBaseTVList = pMesh->mapFaces( nMapChannel );
					pBaseUVList = pMesh->mapVerts( nMapChannel );
					pLayer1TVList = NULL;
					pLayer1UVList = NULL;
				} else if( !MtlHelper.bCompositeTex ) {
					// not a composite texture, there will be only 1 set of texture coordinates
					if( MtlHelper.bUseSubTexs ) {
						pTexmap = pTexmap->GetSubTexmap( MtlHelper.anValidLayerIndices[0] );
					}
					nMapChannel = (!pTexmap) ? 1 : pTexmap->GetMapChannel();
					pBaseTVList = pMesh->mapFaces( nMapChannel );
					pBaseUVList = pMesh->mapVerts( nMapChannel );
					pLayer1TVList = NULL;
					pLayer1UVList = NULL;
				} else {
					// a composite texture, there will be 2 mapping channels
					// first the base layer
					pTexmap2 = pTexmap->GetSubTexmap( MtlHelper.anValidLayerIndices[0] );
					nMapChannel = ( pTexmap2 ) ? pTexmap2->GetMapChannel() : 1;
					pBaseTVList = pMesh->mapFaces( nMapChannel );
					pBaseUVList = pMesh->mapVerts( nMapChannel );
					// now layer 1
					pTexmap2 = pTexmap->GetSubTexmap( MtlHelper.anValidLayerIndices[1] );
					nMapChannel = ( pTexmap2 ) ? pTexmap2->GetMapChannel() : 1;
					pLayer1TVList = pMesh->mapFaces( nMapChannel );
					pLayer1UVList = pMesh->mapVerts( nMapChannel );
				}
			}		
		}		
	}

	if (bLightMapCoord) //my lightmap texture coordinates.
	{
		nMapChannel = _LIGHTMAP_UV_CHANNEL;
		pLMapTVList = pMesh->mapFaces( nMapChannel );
		pLMapUVList = pMesh->mapVerts( nMapChannel );
	}

	//////////////////////////////////
	// Setup the vertex alpha pointers
	if( pMesh->mapSupport( _VERTEX_ALPHA_MAP_CHANNEL ) ) {
		pVtxAlphaTVList = pMesh->mapFaces( _VERTEX_ALPHA_MAP_CHANNEL );
		pVtxAlphaUVList = pMesh->mapVerts( _VERTEX_ALPHA_MAP_CHANNEL );
	} else {
		pVtxAlphaTVList = NULL;
		pVtxAlphaUVList = NULL;
	}

	//////////////////////////////////
	// Setup the vertex color pointers
	if( pMesh->mapSupport( _VERTEX_RGB_MAP_CHANNEL ) ) {
		pVtxRGBTVList = pMesh->mapFaces( _VERTEX_RGB_MAP_CHANNEL );
		pRGBList = pMesh->mapVerts( _VERTEX_RGB_MAP_CHANNEL );
	} else {
		pVtxRGBTVList = NULL;
		pRGBList = NULL;
	}

	//////////////////////////////
	// Zero out our vertex weights
	_VertexWeights.Reset();
	if( bSkinned && !bNodeHasWeights ) {
		// force one weight of 1.0f
//		u32 nBoneID = GetNodeIndex( m_pInterface->GetRootNode(), pNode );
//		FASSERT( nBoneID >= 0 );
//		_VertexWeights.Insert( nBoneID, 1.0f ); 
		FASSERT( nNodeIndex >= 0 );
		_VertexWeights.Insert( nNodeIndex, 1.0f ); 
	}

	/////////////////////
	// Init our help vars
	nNumIndices = 0;
	Zero = Point3( 0.0f, 0.0f, 0.0f );
	One = Point3( 1.0f, 1.0f, 1.0f );
	int nProgFaceCount( 0 ), nProgUpdateLimit( 0 );
	float fPercentOfFaces( 0.0f ), fOONumFaces( 0.0f );
	BOOL bUpdateProgressBar = ( nFaces > 100 );
	if( bUpdateProgressBar ) {
		nProgUpdateLimit = (int)( nFaces / m_fPercentPerMat );
		nProgUpdateLimit++;
		fOONumFaces = 1.0f / nFaces;
	}

	///////////////////////////////////////////////////////////////////
	// Run through all of the faces looking for faces maped with nMatID
	for( nFaceNum=0; nFaceNum < nFaces; nFaceNum++ ) {
		pFace = &pMesh->faces[nFaceNum];
		
		if( bUpdateProgressBar ) {
			++nProgFaceCount;
			if( nProgFaceCount > nProgUpdateLimit ) {
				nProgFaceCount = 0;
				fPercentOfFaces = nFaceNum * fOONumFaces;
				UpdateProgressBar( (int)( m_fPercentSoFar + (fPercentOfFaces * m_fPercentPerMat) ) );
			}
		}

		// see if we should add this face
		if( nMatID >= 0 ) {
			nMaxID = pFace->getMatID();
			if( nMaxID >= nNumMats ) {
				nMaxID %= nNumMats;
			}
			if( nMaxID != nMatID ) {
				// this face is not mapped with nMatID
				continue;
			}
		}
		///////////////////////////////
		// IF WE GET HERE ADD THIS FACE

		// grab pointers to our TV Faces
		pBaseTV = (pBaseTVList && pBaseUVList) ? &pBaseTVList[nFaceNum] : NULL;
		pLayer1TV = (pLayer1TVList && pLayer1UVList) ? &pLayer1TVList[nFaceNum] : NULL;
		pLMapTV = (pLMapTVList && pLMapUVList) ? &pLMapTVList[nFaceNum] : NULL;
		pVtxAlphaTV = (pVtxAlphaTVList && pVtxAlphaUVList) ? &pVtxAlphaTVList[nFaceNum] : NULL;
		pVtxRGBTV = (pVtxRGBTVList && pRGBList) ? &pVtxRGBTVList[nFaceNum] : NULL;
		
		// VERT 0
		nVertIndex = pFace->getVert( 0 );
		Vert = pMesh->verts[nVertIndex] * TMAfterWSM;
		Norm = GetVertexNormal( pMesh, nFaceNum, pMesh->getRVertPtr( nVertIndex ) ) * NodeTM;
		Norm = FNormalize( Norm );
		if( pBaseTV ) {
			// set the UVs for the base
			UV1 = pBaseUVList[ pBaseTV->t[0] ];
			// set the UVs for layer1
			if (pLayer1TV)
			{
				UV2 = (pLayer1TV) ? pLayer1UVList[ pLayer1TV->t[0] ] : Zero;
				UV3 = (pLMapTV) ? pLMapUVList[ pLMapTV->t[0] ] : Zero;
			}
			else
			{
				UV2 = (pLMapTV) ? pLMapUVList[ pLMapTV->t[0] ] : Zero;
				UV3 = Zero;
			}
		} else {
			// both sets of UVs are set to zero
			UV1 = (pLMapTV) ? pLMapUVList[ pLMapTV->t[0] ] : Zero;
			UV2  = Zero;
		}
		AlphaUV = (pVtxAlphaTV) ? pVtxAlphaUVList[ pVtxAlphaTV->t[0] ] : One;
		vColor = (pVtxRGBTV) ? pRGBList[ pVtxRGBTV->t[0] ] : Zero;
		if( bSkinned ) {
			if( bNodeHasWeights ) {
				GetVertexWeights( pNode, nVertIndex );
				_VertexWeights.LimitAndScaleVertexWeights();
			}
		}
		_FillVertData( aApeVerts[0], Vert, Norm, vColor, UV1, UV2, UV3, AlphaUV.x );
		// VERT 1
		nVertIndex = (!bSwapTriOrder) ? pFace->getVert( 1 ) : pFace->getVert( 2 );
		Vert = pMesh->verts[nVertIndex] * TMAfterWSM;
		Norm = GetVertexNormal( pMesh, nFaceNum, pMesh->getRVertPtr( nVertIndex ) ) * NodeTM;
		Norm = FNormalize( Norm );		
		nIndex = (!bSwapTriOrder) ? 1 : 2;
		if( pBaseTV ) {
			// set the UVs for the base
			UV1 = pBaseUVList[ pBaseTV->t[nIndex] ];
			// set the UVs for layer1
			if (pLayer1TV)
			{
				UV2 = (pLayer1TV) ? pLayer1UVList[ pLayer1TV->t[nIndex] ] : Zero;
				UV3 = (pLMapTV) ? pLMapUVList[ pLMapTV->t[nIndex] ] : Zero;
			}
			else
			{
				UV2 = (pLMapTV) ? pLMapUVList[ pLMapTV->t[nIndex] ] : Zero;
				UV3 = Zero;
			}
		}
		else
		{
			UV1 = (pLMapTV) ? pLMapUVList[ pLMapTV->t[nIndex] ] : Zero;
		}
		AlphaUV = (pVtxAlphaTV) ? pVtxAlphaUVList[ pVtxAlphaTV->t[nIndex] ] : One;
		vColor = (pVtxRGBTV) ? pRGBList[ pVtxRGBTV->t[nIndex] ] : Zero;
		if( bSkinned ) {
			if( bNodeHasWeights ) {
				GetVertexWeights( pNode, nVertIndex );
				_VertexWeights.LimitAndScaleVertexWeights();
			}
		}
		_FillVertData( aApeVerts[1], Vert, Norm, vColor, UV1, UV2, UV3, AlphaUV.x );
		// VERT 2
		nVertIndex = (!bSwapTriOrder) ? pFace->getVert( 2 ) : pFace->getVert( 1 );
		Vert = pMesh->verts[nVertIndex] * TMAfterWSM;
		Norm = GetVertexNormal( pMesh, nFaceNum, pMesh->getRVertPtr( nVertIndex ) ) * NodeTM;
		Norm = FNormalize( Norm );
		nIndex = (!bSwapTriOrder) ? 2 : 1;
		if( pBaseTV ) {
			// set the UVs for the base
			UV1 = pBaseUVList[ pBaseTV->t[nIndex] ];
			// set the UVs for layer1
			if (pLayer1TV)
			{
				UV2 = (pLayer1TV) ? pLayer1UVList[ pLayer1TV->t[nIndex] ] : Zero;
				UV3 = (pLMapTV) ? pLMapUVList[ pLMapTV->t[nIndex] ] : Zero;
			}
			else
			{
				UV2 = (pLMapTV) ? pLMapUVList[ pLMapTV->t[nIndex] ] : Zero;
				UV3 = Zero;
			}
		}
		else
		{
			UV1 = (pLMapTV) ? pLMapUVList[ pLMapTV->t[nIndex] ] : Zero;
		}
		AlphaUV = (pVtxAlphaTV) ? pVtxAlphaUVList[ pVtxAlphaTV->t[nIndex] ] : One;
		vColor = (pVtxRGBTV) ? pRGBList[ pVtxRGBTV->t[nIndex] ] : Zero;
		if( bSkinned ) {
			if( bNodeHasWeights ) {
				GetVertexWeights( pNode, nVertIndex );
				_VertexWeights.LimitAndScaleVertexWeights();
			}
		}
		_FillVertData( aApeVerts[2], Vert, Norm, vColor, UV1, UV2, UV3, AlphaUV.x );

		// process our 3 texture coordinates
		if( pBaseTV ) {
			ProcessUV( rMatInfo.nLayersUsed, aApeVerts[0], aApeVerts[1], aApeVerts[2] );
		}

#if _REMOVE_DEGENERATIVE_TRIS		
		// find zero area tris, before we add this tri to our list
		Vec1 = aApeVerts[0].Pos - aApeVerts[1].Pos;
		Vec2 = aApeVerts[2].Pos - aApeVerts[1].Pos;
		Vec3 = Vec1.Cross( Vec2 );
		if( Vec1.Mag2() < _SMALLEST_POSSIBLE_TRI_AREA2 || 
			Vec2.Mag2() < _SMALLEST_POSSIBLE_TRI_AREA2 ||
			Vec3.Mag2() < _SMALLEST_POSSIBLE_TRI_AREA2 ) {
			// don't add this tri
			continue;
		}
#endif

		// add our verts
		if( !bFlipTriWindingOrder ) {
			// use max's winding order
			pIndices[nNumIndices++].nVertIndex = VertHashTable.InsertDataObIntoList( (u8 *)&aApeVerts[0] );
			pIndices[nNumIndices++].nVertIndex = VertHashTable.InsertDataObIntoList( (u8 *)&aApeVerts[1] );
			pIndices[nNumIndices++].nVertIndex = VertHashTable.InsertDataObIntoList( (u8 *)&aApeVerts[2] );
		} else {
			// flip the winding order
			pIndices[nNumIndices++].nVertIndex = VertHashTable.InsertDataObIntoList( (u8 *)&aApeVerts[0] );
			pIndices[nNumIndices++].nVertIndex = VertHashTable.InsertDataObIntoList( (u8 *)&aApeVerts[2] );
			pIndices[nNumIndices++].nVertIndex = VertHashTable.InsertDataObIntoList( (u8 *)&aApeVerts[1] );
		}
#if 0
		DEVPRINTF( "Tri %d: %d, %d, %d\n\t(%.02f, %.02f, %.02f)\n\t(%.02f, %.02f, %.02f)\n\t(%.02f, %.02f, %.02f)\n", 
																nFaceNum,
																pIndices[nNumIndices-3].nVertIndex,
																pIndices[nNumIndices-2].nVertIndex,
																pIndices[nNumIndices-1].nVertIndex,
																aApeVerts[0].Pos.x,
																aApeVerts[0].Pos.y,
																aApeVerts[0].Pos.z,
																aApeVerts[1].Pos.x,
																aApeVerts[1].Pos.y,
																aApeVerts[1].Pos.z,
																aApeVerts[2].Pos.x,
																aApeVerts[2].Pos.y,
																aApeVerts[2].Pos.z );
#endif

#if _REMOVE_DEGENERATIVE_TRIS
		if( IsTriDegenerative( pIndices[nNumIndices-3], pIndices[nNumIndices-2], pIndices[nNumIndices-1] ) ) {
			// take out the last tri added, it is degenerative
			nNumIndices -= 3;
			DEVPRINTF( "Degenerative Tri, removing last 3 indices.\n" );
		} else {
			pV1 = (ApeVert_t *)VertHashTable.GetDataObByIndex( pIndices[ nNumIndices-3 ].nVertIndex );
			pV2 = (ApeVert_t *)VertHashTable.GetDataObByIndex( pIndices[ nNumIndices-2 ].nVertIndex );
			pV3 = (ApeVert_t *)VertHashTable.GetDataObByIndex( pIndices[ nNumIndices-1 ].nVertIndex );
			
			Vec1 = pV1->Pos - pV2->Pos;
			Vec2 = pV3->Pos - pV2->Pos;
			Vec3 = Vec1.Cross( Vec2 );
			if( Vec1.Mag2() < _SMALLEST_POSSIBLE_TRI_AREA2 || 
				Vec2.Mag2() < _SMALLEST_POSSIBLE_TRI_AREA2 ||
				Vec3.Mag2() < _SMALLEST_POSSIBLE_TRI_AREA2 ) {
				// take out the last tri added, it is degenerative
				nNumIndices -= 3;
				DEVPRINTF( "Degenerative Tri, removing last 3 indices.\n" );				
			}
		}
#endif
	}

	//////////////////////////
	// Update our progress bar
	m_fPercentSoFar += m_fPercentPerMat;
	UpdateProgressBar( (int)m_fPercentSoFar );

	////////////////////////////////////////
	// Make sure that something was exported
	if( !nNumIndices ) {
		// no need to add this material, no tris use it
		return FALSE;
	}

	// fill in the rest of our header structs
	rMatInfo.nNumIndices = nNumIndices;

	return TRUE;
}

void ApeExporter::ProcessUV( int nNumTexMaps, ApeVert_t &V1, ApeVert_t &V2, ApeVert_t &V3 ) {

	if( nNumTexMaps <= 0 ) {
		return;
	}
	u32 i( 0 );

	FMATH_CLAMPMAX( nNumTexMaps, MAX_LAYERS_PER_MAT );

#if _FLIP_TO_UPPER_LEFT_T_CORNER
	for( i=0; i < (u32)nNumTexMaps; i++ ) {
		V1.aUV[i].y = 1.0f - V1.aUV[i].y;
		V2.aUV[i].y = 1.0f - V2.aUV[i].y;
		V3.aUV[i].y = 1.0f - V3.aUV[i].y;
	}
#endif
}

int ApeExporter::IsVertAlreadyInList( ApeVert_t &V, ApeVert_t *pVerts, int nNumVerts ) {
#if _REMOVE_DUP_VERTS
	
	for( int i=0; i < nNumVerts; i++ ) {
		if( AreVertsEqual( V, pVerts[i] ) ) {
			return i;
		}
	}
#endif
	return -1;
}

// writes data out to our file, updating the amount of total data write out
void ApeExporter::WriteToFile( void *pData, int nBytes, int nItems, BOOL bUpdateTotalBytesWritten ) {
	
	if( bUpdateTotalBytesWritten ) {
		m_MeshInfo.nBytesInFile += (nBytes * nItems);
	}
	fwrite( pData, nBytes, nItems, m_pFileStream );
}


// writes data out to our file, updating the amount of total data write out
void ApeExporter::WriteToCamFile( void *pData, int nBytes, int nItems, BOOL bUpdateTotalBytesWritten ) {
	
	if( bUpdateTotalBytesWritten ) {
		m_CamInfo.nBytesInFile += (nBytes * nItems);
	}
	fwrite( pData, nBytes, nItems, m_pFileStream );
}

// create a red error layer
void ApeExporter::CreateErrorLayer( ApeLayer_t &Layer ) {
	// zero everything out
	ZeroMemory( &Layer, sizeof( ApeLayer_t ) );
	strcpy( Layer.szTexnames[APE_LAYER_TEXTURE_DIFFUSE], "Error" );
//Layer.fOpacity = 1.0f;
	Layer.DiffuseRGB.White();
	Layer.SpecularRGB.White();
	Layer.SelfIllumRGB.White();	
}

void ApeExporter::GetVertexWeights( INode *pNode, u32 nVertIndex ) {
	
	// set our output to no weights
	_VertexWeights.Reset();
		
	int nBoneCount = GetBoneCount( m_pInterface->GetRootNode() );
	if( nBoneCount <= 0 ) {
		// no bones in this scene
		return;
	}
	// try to get a skin modifier
	Modifier *pMod = FindSkinModifier( pNode );
	if( pMod ) {
		// get the skin weight data
		GetSkinWeights( pNode, pMod, nVertIndex );
	}
}

BOOL ApeExporter::GetSkinWeights( INode *pNode, Modifier *pMod, u32 nVertNum ) {

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
	if( (s32)nVertNum >= nNumPoints ) {
		// invalid vert index
		pMod->ReleaseInterface( I_SKIN, pSkin );
		return FALSE;
	}

	int nNumBones = pSkinContext->GetNumAssignedBones( nVertNum );
	int nAssignedBone, nBoneIndex;
	INode *pBone;
	f32 fWeight;
	for( int i=0; i < nNumBones; i++ ) {
		nAssignedBone = pSkinContext->GetAssignedBone( nVertNum, i );
		if( nAssignedBone < 0 ) {
			continue;
		}
		pBone = pSkin->GetBone( nAssignedBone );
		char *pszFoo = pBone->GetName();
		fWeight = pSkinContext->GetBoneWeight( nVertNum, i );
		if( fWeight > 0.0f ) {
			// we want all non zero weights at this point, add this weight
			nBoneIndex = GetBoneIndex( m_pInterface->GetRootNode(), pBone );
			FASSERT( nBoneIndex >= 0 );
			_VertexWeights.Insert( nBoneIndex, fWeight );
		}
	}
	pMod->ReleaseInterface( I_SKIN, pSkin );

	return TRUE; 
}

void ApeExporter::RecordError( BOOL bCriticalError, cchar *pszErrorMsg, INode *pNode/*=NULL*/ ) {
	CErrorLog &rErrorLog = CErrorLog::GetCurrent();

	if( !m_nNumErrors ) {
		// only write out the header 1 time per run
		rErrorLog.WriteErrorHeader( m_sErrorHeading );
	}
	m_nNumErrors++;
	
	if( bCriticalError ) {
		m_nNumCriticalErrors++;
	}
	
	TSTR sErrorMsg;
	if( pNode ) {
		sErrorMsg.printf( "Node = %s - %s", pNode->GetName(), pszErrorMsg );
	} else {
		sErrorMsg = pszErrorMsg;
	}
	rErrorLog.WriteErrorLine( sErrorMsg );
}

// parse the base mtl and figure out how to deal with it.
// returns FALSE if there is an error with something (it logs the 
// specifics of the error with the error log system).
BOOL ApeExporter::ParseBaseMtl( Mtl *pBaseMtl, _MtlHelper_t &rMtlHelper, INode *pNode ) {
	Mtl *pSubMtl;
	u32 i;
	Texmap *pTexmap, *pTexmap2;

	// zero out our helper struct to start
	ZeroMemory( &rMtlHelper, sizeof( _MtlHelper_t ) );
	
	if( IsMaterialAComposite( pBaseMtl ) ) {
		// make sure that we have at least 2 valid materials, otherwise we can just handle like a standard material
		for( i=0; i < (u32)pBaseMtl->NumSubMtls(); i++ ) {
			pSubMtl = pBaseMtl->GetSubMtl( i );
			if( pSubMtl ) {
				if( pSubMtl->SubTexmapOn( ID_DI ) ) {
					// make sure that this is not a multi or composite either
					if( IsMaterialAComposite( pSubMtl ) ) {
						RecordError( FALSE, "A composite material contains another composite material - skipping material.", pNode );
						return FALSE;
					}
					if( pSubMtl->IsMultiMtl() ) {
						RecordError( FALSE, "A composite material contains a multi/sub material - skipping material.", pNode );
						return FALSE;
					}
					// count this as a layer
					if( rMtlHelper.nNumValidLayers < _MAX_LAYERS_PER_MATERIAL ) {
						rMtlHelper.anValidLayerIndices[ rMtlHelper.nNumValidLayers++ ] = i;
						if( rMtlHelper.nNumValidLayers == _MAX_LAYERS_PER_MATERIAL ) {
							// too many submaterials, record the error, but continue
							RecordError( FALSE, "Too many materials in a composite material - skipping extra materials.", pNode );
						}
					}
				}
			}
		}
		if( rMtlHelper.nNumValidLayers >= 2 ) {
			// we have a valid dual layer case
			rMtlHelper.bCompositeMat = TRUE;
			rMtlHelper.bUseSubMats = TRUE;			
		} else if( rMtlHelper.nNumValidLayers >= 1 ) {
			// we have 1 layer, handle like a standard
			rMtlHelper.bUseSubMats = TRUE;
		} else {
			// this is an error case
			rMtlHelper.bMapToErrorMat = TRUE;
			RecordError( FALSE, "No standard materials contained in a composite material - skipping material.", pNode );
			return FALSE;
		}
	} else if( pBaseMtl ) {
		// check to see if this a composite texture
		if( IsTextureAComposite( pBaseMtl ) ) {
			// make sure that we have at least 2 valid textures, otherwise we can just handle like a standard texture
			pTexmap = pBaseMtl->GetSubTexmap( ID_DI );
			for( i=0; i < (u32)pTexmap->NumSubTexmaps(); i++ ) {
				pTexmap2 = pTexmap->GetSubTexmap( ID_DI );
				if( pTexmap2 ) {
					// count this as a layer
					if( rMtlHelper.nNumValidLayers < _MAX_LAYERS_PER_MATERIAL ) {
						rMtlHelper.anValidLayerIndices[ rMtlHelper.nNumValidLayers++ ] = i;
						if( rMtlHelper.nNumValidLayers == _MAX_LAYERS_PER_MATERIAL ) {
							// too many materials, record the error, but continue
							RecordError( FALSE, "Too many texmaps in a composite texture - skipping extra texmaps.", pNode );
						}
					}
				}
			}
			if( rMtlHelper.nNumValidLayers >= 2 ) {
				// we have a valid dual layer case
				rMtlHelper.bCompositeTex = TRUE;
				rMtlHelper.bUseSubTexs = TRUE;
			} else if( rMtlHelper.nNumValidLayers >= 1 ) {
				// we have 1 layer, handle like a standard texture
				rMtlHelper.bUseSubTexs = TRUE;
			} else {
				// this is an error case
				rMtlHelper.bMapToErrorMat = TRUE;
				RecordError( FALSE, "No standard materials contained in a composite texture - skipping material.", pNode );
				return FALSE;
			}
		} else if( pBaseMtl->ClassID() == Class_ID(DMTL_CLASS_ID, 0) ) {
			// a standard material
			
		} else {
			// an error condition, a material must be either composite or standard at this level
			rMtlHelper.bMapToErrorMat = TRUE;
			RecordError( FALSE, "This node's material is an invalid type - skipping material.", pNode );
			return FALSE;
		}		
	} else {
		// a null base material was passed in
		rMtlHelper.bMapToErrorMat = TRUE;
		RecordError( FALSE, "This node is not mapped to a material, solid color is not supported - skipping material.", pNode );
		return FALSE;
	}
	return TRUE;
}

BOOL ApeExporter::ExportBones() {
	int nNumChildren, nCount, nBoneCount, nParentIndex, j, nChildIndex;
	INode *pBone, *pParent, *pChild, *pRoot;
	ApeBone_t ApeBone;
	CStr *pStringArray, sCurName;
	u32 nCurStrIndex;
	Matrix3 NodeTM, FlippedNodeTM;
	CFMtx43 Rot;

	pRoot = m_pInterface->GetRootNode();
	nBoneCount = GetBoneCount( pRoot );
	if( nBoneCount > 0 ) {
		//////////////////////////////////////////////
		// make sure that there are not too many bones
		if( nBoneCount > MAX_TOTAL_BONE_COUNT ) {
			sCurName.printf( "There are %d bones present in the scene, but the max allowed is %d, please use fewer bones.", nBoneCount, MAX_TOTAL_BONE_COUNT );
			RecordError( TRUE, sCurName );
			return FALSE;
		}

		//////////////////////
		// REAL BONE/SKIN DATA
#if _DUMP_BONE_INFO_TO_TEXT_FILE
		CStr s( name );
		s += ".txt";
		FILE *pFile = _tfopen( s, _T( "wt") );
#endif	
		//////////////////////////////////////////////////////////////////////////////////////////////
		// allocate an array of CStrs so that we can make sure that we don't have duplicate bone names
		pStringArray = new CStr[nBoneCount];
		if( !pStringArray ) {
			RecordError( FALSE, "Could not allocate an array for bone names, can't check for duplicate bone names - skipping all bones." );
			return FALSE;
		}
		nCurStrIndex = 0;

		////////////////////////////////////////////////
		// iterate through all of the bones in the scene
		for( nCount=0; nCount < nBoneCount; nCount++ ) {
			/////////////////////
			// grab the bone node
			pBone = GetBoneNode( pRoot, nCount );	
			FASSERT( pBone );
			////////////////////
			// zero the ape bone
			ZeroMemory( &ApeBone, sizeof( ApeBone_t ) );
			////////////////////
			// get the bone name
			strncpy( ApeBone.szBoneName, pBone->GetName(), BONE_NAME_LEN-1 );
						
			//////////////////////////////////////////
			// make sure that this bone name is unique
			sCurName = ApeBone.szBoneName;
			for( j=0; j < (s32)nCurStrIndex; j++ ) {
				if( fclib_stricmp( pStringArray[j].data(), sCurName.data() ) == 0 ) {
					break;
				}
			}
			if( j != nCurStrIndex ) {
				RecordError( TRUE, "There is another bone with this same name, please make all bone names unique and re-export.", pBone );
				delete [] pStringArray;
				return FALSE;
			}
			pStringArray[nCurStrIndex] = sCurName;
			nCurStrIndex++;
			///////////////////////////
			// record this bone's index
			ApeBone.nBoneIndex = nCount;
			///////////////////////////////
			// get this bone's parent index
			ApeBone.nParentIndex = -1;
			pParent = pBone->GetParentNode();
			if( pParent ) {
				nParentIndex = GetBoneIndex( pRoot, pParent );
				if( nParentIndex >= 0 ) {
					ApeBone.nParentIndex = nParentIndex;
				}
			}
			//////////////////////////////////////////////////
			// get the number of child bones and their indices
			nNumChildren = GetChildBoneCount( pBone );
			if( nNumChildren > MAX_CHILDREN_PER_BONE ) {
				RecordError( FALSE, "Too many child bones attached to 1 bone - skipping extra childern.", pBone );
				FMATH_CLAMPMAX( nNumChildren, MAX_CHILDREN_PER_BONE );	
			}
			for( j=0; j < nNumChildren; j++ ) {
				pChild = GetChildBoneNode( pBone, j );
				if( pChild ) {
					nChildIndex = GetBoneIndex( pRoot, pChild );
					if( nChildIndex >= 0 ) {
						ApeBone.auChildIndices[ ApeBone.nNumChildren++ ] = nChildIndex;
						FASSERT( nChildIndex < nBoneCount ); 
					}
				}
			}
			////////////////////////////////
			// grab the model to bone matrix
			NodeTM = pBone->GetNodeTM( 0 );

			// make sure that the node doesn't have non-uniform scale
			if( DoesTMHaveNonUniformScale( NodeTM ) ) {
				// this is not allowed
				RecordError( TRUE, "Bones can't have non-uniform scale, please fix and re-export.", pBone );
				delete [] pStringArray;
				return FALSE;
			}
#if 1
			Rot = (CFMtx43 &)NodeTM;
#else
			DoesTMHaveNegScale( NodeTM, &FlippedNodeTM );
#endif
			ApeBone.AtRestModelToBoneMtx = m_Left2RightCoordSysMtx * Rot * m_Left2RightCoordSysMtx;
			ApeBone.AtRestModelToBoneMtx.Invert();
			//////////////////////////////
			// fill in the bone flag field
			ApeBone.nFlags = APE_BONE_FLAG_NONE;
			///////////////////////////////
			// write out the bone structure
			m_MeshInfo.nNumBoneNames++;
			WriteToFile( &ApeBone, sizeof( ApeBone_t ), 1, TRUE );
#if _DUMP_BONE_INFO_TO_TEXT_FILE
			if( pFile ) {
				fprintf( pFile, "\nBone[%d]\n", i );
				fprintf( pFile, "\tName = %s\n", ApeBone.szBoneName );
				fprintf( pFile, "\tParent Index = %d\n", ApeBone.nParentIndex );
				fprintf( pFile, "\tNumber Children = %d\n", ApeBone.nNumChildren );
				for( j=0; j < ApeBone.nNumChildren; j++ ) {
					fprintf( pFile, "\tChild[%d] = %d\n", j, ApeBone.auChildIndices[j] );	
				}
			}
#endif
			DEVPRINTF( "Bone %d: name = %s, num children = %d, parent index = %d.\n", m_MeshInfo.nNumBoneNames-1, ApeBone.szBoneName, ApeBone.nNumChildren, ApeBone.nParentIndex );
		}
#if _DUMP_BONE_INFO_TO_TEXT_FILE		
		if( pFile ) {
			fclose( pFile );
		}
#endif
		////////////////////////
		// free our temp strings
		delete [] pStringArray;

		return TRUE;
	} else {
		
		//////////////////////////////////////////////////////
		// WARN USER IF THERE DOESN'T APPEAR TO BE A HIERARCHY
		nNumChildren = GetChildNodeCount( pRoot );
		for( nCount=0; nCount < nNumChildren; nCount++ ) {
			pChild = GetChildNodeByIndex( pRoot, nCount );
			if( GetChildNodeCount( pChild ) ) {
				break;
			}
		}
		if( nCount == nNumChildren ) {
			RecordError( FALSE, "It doesn't appear that this scene has a hierarchy, should you really be exporting with it enabled?" );
		}

		//////////////////////////////
		// TURN EVERY NODE INTO A BONE
		nBoneCount = GetTotalNodeCount( pRoot ); 
		if( nBoneCount ) {
			//////////////////////////////////////////////////////////////////////////////////////////////
			// allocate an array of CStrs so that we can make sure that we don't have duplicate bone names
			pStringArray = new CStr[nBoneCount];
			if( !pStringArray ) {
				RecordError( FALSE, "Could not allocate an array for bone names, can't check for duplicate bone names - skipping all bones." );
				return FALSE;
			}
			nCurStrIndex = 0;

			////////////////////////////////////////////////
			// iterate through all of the nodes in the scene
			CFMtx43 mtxLODOffset;
			mtxLODOffset.Identity();
			for( nCount=0; nCount < nBoneCount; nCount++ ) 
			{
				//////////////////////////////////////////////
				// make sure that there are not too many bones
				if( m_MeshInfo.nNumBoneNames > MAX_TOTAL_BONE_COUNT ) 
				{
					sCurName.printf( "The maximum allowed bones in a hierarchy is %d, please use fewer bones.", MAX_TOTAL_BONE_COUNT );
					RecordError( TRUE, sCurName );
					return FALSE;
				}

				pBone = GetNodeByIndex( pRoot, nCount );	
				FASSERT( pBone );

		// this would require me to re index all of the bones in the tree, ugh!
		//		if( !ShouldBoneBeExported( pBone ) ) {
		//			continue;
		//		}

				////////////////////
				// zero the ape bone
				ZeroMemory( &ApeBone, sizeof( ApeBone_t ) );
				////////////////////
				// get the bone name
				if ( strlen( pBone->GetName() ) > 5 && strncmp( pBone->GetName(), "LOD", 3 ) == 0 && pBone->GetName()[4] == '_' )
				{
					strncpy( ApeBone.szBoneName, &pBone->GetName()[5], BONE_NAME_LEN-1 );
				}
				else
				{
					strncpy( ApeBone.szBoneName, pBone->GetName(), BONE_NAME_LEN-1 );
				}

				// Get the root of the tree
				INode *pTreeRoot = pBone;
				while ( pTreeRoot )
				{
					if ( strlen( pTreeRoot->GetName() ) > 5 && strncmp( pTreeRoot->GetName(), "LOD", 3 ) == 0 && pTreeRoot->GetName()[4] == '_' )
					{
						// This root is an LOD root
						break;
					}

					if ( !pTreeRoot->GetParentNode() )
					{
						// This would be the scene root
						break;
					}

					pTreeRoot = pTreeRoot->GetParentNode();
				}

				if ( strncmp( pTreeRoot->GetName(), "LOD0_", 4 ) == 0 )
				{
					// This root is an LOD root for the base LOD
					// Make sure bone names are unique

					mtxLODOffset = (CFMtx43 &)pTreeRoot->GetNodeTM( 0 );
					mtxLODOffset.Invert();

					//////////////////////////////////////////
					// make sure that this node name is unique
					sCurName = ApeBone.szBoneName;
					for ( j = 0; j < (s32)nCurStrIndex; j++ ) 
					{
						if( fclib_stricmp( pStringArray[j].data(), sCurName.data() ) == 0 ) 
						{
							break;
						}
					}
					if ( j != nCurStrIndex ) 
					{
						sCurName.printf( "There are multiple bones with the name %s, please make all bone names unique and re-export.", pBone->GetName() );
						RecordError( TRUE, sCurName );
						delete [] pStringArray;
						return FALSE;
					}
					pStringArray[nCurStrIndex] = sCurName;
					nCurStrIndex++;
				}
				else if ( strlen( pTreeRoot->GetName() ) > 5 && strncmp( pTreeRoot->GetName(), "LOD", 3 ) == 0 && pTreeRoot->GetName()[4] == '_' )
				{
					// This root is an LOD root for subsequent LODs

					//////////////////////////////////////////
					// make sure that this node name is in the base LOD
					sCurName = ApeBone.szBoneName;
					for ( j = 0; j < (s32)nCurStrIndex; j++ ) 
					{
						if( fclib_stricmp( pStringArray[j].data(), sCurName.data() ) == 0 ) 
						{
							break;
						}
					}
					if ( j == nCurStrIndex ) 
					{
						sCurName.printf( "The bone name %s does not exist in the base LOD for this model.", sCurName );
						RecordError( TRUE, sCurName );
						delete [] pStringArray;
						return FALSE;
					}

					// We do not add this bone into the hierarchy because it is part of an LOD chain
					continue;
				}

				///////////////////////////
				// record this bone's index
				ApeBone.nBoneIndex = nCount;
				///////////////////////////////
				// get this bone's parent index
				ApeBone.nParentIndex = -1;
				pParent = pBone->GetParentNode();
				if( pParent ) {
					nParentIndex = GetNodeIndex( pRoot, pParent );
					if( nParentIndex >= 0 ) {
						ApeBone.nParentIndex = nParentIndex;
					}
				}
				//////////////////////////////////////////////////
				// get the number of child bones and their indices
				nNumChildren = GetChildNodeCount( pBone );
				if( nNumChildren > MAX_CHILDREN_PER_BONE ) {
					RecordError( FALSE, "Too many child bones attached to 1 bone - skipping extra childern.", pBone );
					FMATH_CLAMPMAX( nNumChildren, MAX_CHILDREN_PER_BONE );	
				}
				for( j=0; j < nNumChildren; j++ ) 
				{
					pChild = GetChildNodeByIndex( pBone, j );
					if( pChild ) 
					{
						char *pszChildName = pChild->GetName();
						if ( strncmp( pszChildName, "LOD0_", 4 ) == 0 
							|| ( strlen( pszChildName ) < 5 || strncmp( pszChildName, "LOD", 3 ) != 0 || pszChildName[4] != '_' ) )
						{
							nChildIndex = GetNodeIndex( pRoot, pChild );
							if( nChildIndex >= 0 ) 
							{
								ApeBone.auChildIndices[ ApeBone.nNumChildren++ ] = nChildIndex;
								FASSERT( nChildIndex < nBoneCount ); 
							}
						}
					}
				}
				////////////////////////////////
				// grab the model to bone matrix
				NodeTM = pBone->GetNodeTM( 0 );

				// make sure that the node doesn't have non-uniform scale
				if( DoesTMHaveNonUniformScale( NodeTM ) ) {
					// this is not allowed
					RecordError( TRUE, "Bones can't have non-uniform scale, please fix and re-export.", pBone );
					delete [] pStringArray;
					return FALSE;
				}
#if 1
				Rot = (CFMtx43 &)NodeTM;
#else
				DoesTMHaveNegScale( NodeTM, &FlippedNodeTM );
#endif
				ApeBone.AtRestModelToBoneMtx = m_Left2RightCoordSysMtx * (Rot * mtxLODOffset) * m_Left2RightCoordSysMtx;
				ApeBone.AtRestModelToBoneMtx.Invert();
				//////////////////////////////
				// fill in the bone flag field
				ApeBone.nFlags = APE_BONE_FLAG_NONE;
				///////////////////////////////
				// write out the bone structure
				m_MeshInfo.nNumBoneNames++;
				WriteToFile( &ApeBone, sizeof( ApeBone_t ), 1, TRUE );

				DEVPRINTF( "Bone %d: name = %s, num children = %d, parent index = %d.\n", m_MeshInfo.nNumBoneNames-1, ApeBone.szBoneName, ApeBone.nNumChildren, ApeBone.nParentIndex );
			}
			////////////////////////
			// free our temp strings
			delete [] pStringArray;

			return TRUE;
		}
	}
	return FALSE;
}

// nLayerNum is only used for composite textures
cchar *ApeExporter::ExtractTextureName( Mtl *pMaxMtl, u32 nLayerNum, u32 nTypeID ) {
	static char szTextureName[TEXTURE_NAME_LEN];
	char *pszName = NULL;

	if( !pMaxMtl ) {
		return NULL;
	}
	
	// new method
	Texmap *pTexmap = pMaxMtl->GetSubTexmap( nTypeID );
	if( !pTexmap ) {
		return NULL;
	}
	if( pTexmap->ClassID() == Class_ID( BMTEX_CLASS_ID, 0x00 ) ) {
		// standard material, not a composite texture
		pszName = ((BitmapTex *)pTexmap)->GetMapName();
	} else if( pTexmap->ClassID() == Class_ID( COMPOSITE_CLASS_ID, 0x00 ) ) {
		pTexmap = pTexmap->GetSubTexmap( nLayerNum );
		if( !pTexmap ) {
			return NULL;
		}
		pszName = ((BitmapTex *)pTexmap)->GetMapName();
	}
	if( !pszName ) {
		return NULL;
	}
	CStr sFilename;
	sFilename = pszName;
	int nIndex;
	// first remove the extension
	nIndex = sFilename.last( '.' );
	if( nIndex >= 0 ) {
		sFilename.remove( nIndex );
	}
	// remove any path info
	nIndex = sFilename.last( '\\' );
	if( nIndex >= 0 ) {
		sFilename.remove( 0, nIndex + 1 );	
	}
	nIndex = sFilename.Length();
	if( nIndex <= 0 ) {
		return NULL;
	} else if( nIndex >= (TEXTURE_NAME_LEN-4) ) {
		CStr sErrorCode;
		sErrorCode.printf( "The texture filename '%s' is too long %d (texture names must be %d characters or less).", sFilename, nIndex, (TEXTURE_NAME_LEN-4) );
		RecordError( FALSE, sErrorCode );
		return NULL;		
	}
	sFilename.toLower();

	strncpy( szTextureName, (cchar *)sFilename, TEXTURE_NAME_LEN );

	return szTextureName;
}

#include "decomp.h"

BOOL ApeExporter::DoesTMHaveNegScale( Matrix3 &rTM, Matrix3 *pNewTM/*=NULL*/ ) {
	
	// grab x, y, & z from max & calculate normalized versions of each
	Point3 MaxRowX = rTM.GetRow( 0 );
	Point3 NormMaxRowX = FNormalize( MaxRowX );

	Point3 MaxRowY = rTM.GetRow( 1 );
	Point3 NormMaxRowY = FNormalize( MaxRowY );
	
	Point3 MaxRowZ = rTM.GetRow( 2 );
	Point3 NormMaxRowZ = FNormalize( MaxRowZ );

	// calculate a new y from z & x	
	Point3 CalculatedRow = CrossProd( NormMaxRowZ, NormMaxRowX );

	// test our calculated y to the normalized max y
	float fDot = DotProd( NormMaxRowY, CalculatedRow );
	BOOL bFlipWindingOrder = ( fDot >= 0.0f ) ? FALSE : TRUE;

	if( pNewTM ) {
#if 0
		if( !bFlipWindingOrder ) {
			// copy rTM
			*pNewTM = rTM;
		} else {
			// calculate a new flipped TM
			MaxRowY = -MaxRowY;
			
			// calculate a new Z from X & Y	
			CalculatedRow = CrossProd( NormMaxRowX, NormMaxRowY );
			fDot = DotProd( NormMaxRowZ, CalculatedRow );
			if( fDot < 0.0f ) {
				MaxRowZ = -MaxRowZ;
			}

			// calculate a new X from Y & Z	
			CalculatedRow = CrossProd( NormMaxRowY, NormMaxRowZ );
			fDot = DotProd( NormMaxRowX, CalculatedRow );
			if( fDot < 0.0f ) {
				MaxRowX = -MaxRowX;
			}
			
			// fill in the new TM with our new rows
			pNewTM->Set( MaxRowX, MaxRowY, MaxRowZ, rTM.GetRow( 3 ) );
		}
#else
		if( !bFlipWindingOrder ) {
			// copy rTM
			*pNewTM = rTM;
		} else {
			AffineParts Parts;
			decomp_affine( rTM, &Parts );
			
		//	Parts.k *= -1.0f;
#if 1
			Matrix3 srtm, rtm, ptm, stm, ftm;

			ptm.IdentityMatrix();
			ptm.SetTrans( Parts.t );	
			Parts.q.MakeMatrix( rtm );
			Parts.u.MakeMatrix( srtm );
			stm = ScaleMatrix( Parts.k );
			ftm = ScaleMatrix( Point3( Parts.f, Parts.f, Parts.f ) ); 			
			*pNewTM = Inverse( srtm ) * stm * srtm * rtm * ftm * ptm;
#else
			Matrix3 Mat;
			Parts.q.MakeMatrix( Mat );
			Mat.SetRow( 3, Parts.t );
			*pNewTM = Mat;
#endif
		}
#endif
	}

	return bFlipWindingOrder;	
}

BOOL ApeExporter::DoesTMHaveNonUniformScale( Matrix3 &rTM ) {

	// make sure that there is not non uniform scaling
	Point3 row;
	u32 i;
	f32 afScale[3];
	f32 fScaleSum = 0.0f;
	for( i=0; i < 3; i++ ) {
		row = rTM.GetRow(i);
		afScale[i] = FLength( row );
		fScaleSum += afScale[i];
	}
	f32 fDelta;
	fDelta = FMATH_FABS( afScale[0] - afScale[1] );
	fDelta += FMATH_FABS( afScale[0] - afScale[2] );
	fDelta += FMATH_FABS( afScale[1] - afScale[2] );
	fDelta /= fScaleSum;

	if( fDelta >= 0.01f ) {
		return TRUE;
	}
	return FALSE;
}

BOOL ApeExporter::DoesMtxHaveNonUniformScale( CFMtx43 &rMtx ) {
	
	// make sure that there is not non uniform scaling
	f32 afScale[3];
	f32 fScaleSum, fDelta;
	
	afScale[0] = rMtx.m_vRight.Mag();
	fScaleSum = afScale[0];
	afScale[1] = rMtx.m_vUp.Mag();
	fScaleSum += afScale[1];
	afScale[2] = rMtx.m_vFront.Mag();
	fScaleSum += afScale[2];
	
	fDelta = FMATH_FABS( afScale[0] - afScale[1] );
	fDelta += FMATH_FABS( afScale[0] - afScale[2] );
	fDelta += FMATH_FABS( afScale[1] - afScale[2] );
	fDelta /= fScaleSum;

	if( fDelta >= 0.01f ) {
		return TRUE;
	}
	return FALSE;
}


#if 0
// bugfree decompose decomp_affine :)
void DecompAffine( Matrix3 &mat, AffineParts *ap ) {
	 HMatrix   m;
	 Point3   row;
	 O_AffineParts oAp;
 
	 for( int y=0; y<4; y++ ) {
		 for( int x=0; x<4; x++ ) {
			 m[y][x]=0;
		 }
	 }
 
	 m[3][3] = 1.0f;
 
	 row = mat.GetRow(0);
	 m[0][0]=row.x;
	 m[0][1]=row.y;
	 m[0][2]=row.z;
 
	 row = mat.GetRow(1);
	 m[1][0]=row.x; m[1][1]=row.y; m[1][2]=row.z;
 
	 row = mat.GetRow(2);
	 m[2][0]=row.x; m[2][1]=row.y; m[2][2]=row.z;
 
	 row = mat.GetRow(3);
	 m[0][3]=row.x; m[1][3]=row.y; m[2][3]=row.z;
 
	 O_decomp_affine(m, &oAp);
 
	 // now we have the oxygen affine parts, convert them to the affineparts of max
	 ap->t.x = oAp.t.x;
	 ap->t.y = oAp.t.y;
	 ap->t.z = oAp.t.z;
 
	 ap->q.x = oAp.q.x;
	 ap->q.y = oAp.q.y;
	 ap->q.z = oAp.q.z;
	 ap->q.w = oAp.q.w;
 
	 ap->u.x = oAp.u.x;
	 ap->u.y = oAp.u.y;
	 ap->u.z = oAp.u.z;
	 ap->u.w = oAp.u.w;
 
	 ap->k.x = oAp.k.x * oAp.f;
	 ap->k.y = oAp.k.y * oAp.f;
	 ap->k.z = oAp.k.z * oAp.f;
 
	 ap->f = oAp.f;
}
#endif

//==================
// private functions

static void _FillVertData( ApeVert_t &Vert, Point3 &MaxVert, Point3 &MaxNorm, 
						   Point3 &MaxColor, Point3 &MaxUV1, Point3 &MaxUV2, Point3 &MaxUV3, f32 fVertA ) {
	// fill in pos, norm
#if APE_EXPORTER_SWAP_YZ
	Vert.Pos.Set( MaxVert.x, MaxVert.z, MaxVert.y );
	Vert.Norm.Set( MaxNorm.x, MaxNorm.z, MaxNorm.y );
#else 
	Vert.Pos.Set( MaxVert.x, MaxVert.y, MaxVert.z );
	Vert.Norm.Set( MaxNorm.x, MaxNorm.y, MaxNorm.z );
#endif
	// fill in rgba
	Vert.Color.Set( MaxColor.x, MaxColor.y, MaxColor.z, fVertA );
	// fill in the uv coords
	Vert.aUV[0].Set( MaxUV1.x, MaxUV1.y );
	Vert.aUV[1].Set( MaxUV2.x, MaxUV2.y );
	Vert.aUV[2].Set( MaxUV3.x, MaxUV3.y );
	// fill in the num weights
	u32 nNumWeights = ApeExporter::_VertexWeights.GetNumUsedWeights();
	Vert.fNumWeights = (f32)nNumWeights;
	ZeroMemory( Vert.aWeights, sizeof( ApeWeight_t ) * MAX_WEIGHTS_PER_VERT );
	for( u32 i=0; i < nNumWeights; i++ ) {
		Vert.aWeights[i] = ApeExporter::_VertexWeights.GetWeight( i );
	}

	// always zero out any unused portion of the struct
	ZeroMemory( Vert.nUnused, sizeof( Vert.nUnused ) );
}
