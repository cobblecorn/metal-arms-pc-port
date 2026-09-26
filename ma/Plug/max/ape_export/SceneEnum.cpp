//////////////////////////////////////////////////////////////////////////////////////
// SceneEnum.cpp - 
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
// 12/07/01 Starich     Created.
//////////////////////////////////////////////////////////////////////////////////////
#include "SceneExport.h"
#include "fang.h"
#include "gizmo.h"
#include "gizmoimp.h"
#include "MatStringParser.h"


// returns TRUE if pNode is not a fog node and enumeration should continue,
// returns FALSE if pNode was either a fog node or cancel was pressed.
BOOL ApeExporter::FogNodeEnum( INode *pNode ) {
	
	if( m_bExportSelected && pNode->Selected() == FALSE ) {
		// this node is not selected, keep enumerating
		return TRUE;
	}
	// Check for user cancel
	if( m_pInterface->GetCancel() ) {
		return FALSE;
	}
	
	if( !m_bExportSelected || pNode->Selected() ) {

		ObjectState os = pNode->EvalWorldState( 0 );

		if( !ShouldNodeBeExported( pNode ) ) {
			return TRUE;// this node should not be exported, but we should keep enumerating
		}

		if( os.obj && (os.obj->SuperClassID() == HELPER_CLASS_ID) ) {
			CStr sName;
			cchar *pszNodeName = pNode->GetName();
			if( !pszNodeName ) {
				return TRUE;// no node name, keep searching...
			}
			sName = pszNodeName;
			sName.toLower();
			CStr sTarget = "fog";
			if( sName.Substr( 0, 3 ) == sTarget ) {
				// this is our fog node
				ApeFog_t ApeFog;
				f32 fValue;
				int nValue;
				// zero our memory first
				fang_MemZero( &ApeFog, sizeof( ApeFog_t ) );
				// search for the start attribute
				if( pNode->GetUserPropFloat( "start", fValue ) ) {
					FMATH_CLAMPMIN( fValue, 1.0f );
					ApeFog.fStartDist = fValue;
				} else {
					// use the default start distance
					ApeFog.fStartDist = 10.0f;
				}
				// search for the end attribute
				if( pNode->GetUserPropFloat( "end", fValue ) ) {
					FMATH_CLAMPMIN( fValue, (ApeFog.fStartDist + 1.0f) );
					ApeFog.fEndDist = fValue;
				} else {
					// use the default end distance
					ApeFog.fEndDist = 1000.0f;
				}
				// search for the motif attribute
				if( pNode->GetUserPropInt( "motif", nValue ) ) {
					FMATH_CLAMPMIN( nValue, 0 );
					ApeFog.nMotifID = nValue;
				} else {
					// use the default motif index
					ApeFog.nMotifID = 0;
				}
				// search for the red attribute
				if( pNode->GetUserPropInt( "red", nValue ) ) {
					FMATH_CLAMP( nValue, 0, 255 );
					ApeFog.Color.fRed = (f32)( nValue * (1.0f/255.0f) );
				} else {
					// use the default red value
					ApeFog.Color.fRed = 1.0f;
				}
				// search for the green attribute
				if( pNode->GetUserPropInt( "green", nValue ) ) {
					FMATH_CLAMP( nValue, 0, 255 );
					ApeFog.Color.fGreen = (f32)( nValue * (1.0f/255.0f) );
				} else {
					// use the default green value
					ApeFog.Color.fGreen = 1.0f;
				}
				// search for the blue attribute
				if( pNode->GetUserPropInt( "blue", nValue ) ) {
					FMATH_CLAMP( nValue, 0, 255 );
					ApeFog.Color.fBlue = (f32)( nValue * (1.0f/255.0f) );
				} else {
					// use the default blue value
					ApeFog.Color.fBlue = 1.0f;
				}
				// add the fog to our file
				m_MeshInfo.nNumFogs++;
				WriteToFile( &ApeFog, sizeof( ApeFog_t ), 1, TRUE );
				// no need to look for anymore fog objects, 1 is all we support for now
				return FALSE;
			}
		}
	}
	return TRUE;// keep searching
}

// returns TRUE if enumeration should continue, FALSE if enumeration should stop
BOOL ApeExporter::LightNodeEnum( INode* pNode ) {
	BOOL bEnumChildren = TRUE;

	if( m_bExportSelected && pNode->Selected() == FALSE ) {
		// this node is not selected, see if any of children are
		goto ENUM_CHILD;
	}

	// Check for user cancel
	if( m_pInterface->GetCancel() ) {
		return FALSE;
	}

	if( !m_bExportSelected || pNode->Selected() ) {

		ObjectState os = pNode->EvalWorldState(0);

		if( !ShouldNodeBeExported( pNode ) ||
			IsNodeACell( pNode ) ||
			IsNodeAPort( pNode ) ) {
			bEnumChildren = FALSE;
			goto ENUM_CHILD;
		}

		if( os.obj ) {

			// see if the super class ID is a light node
			if( os.obj->SuperClassID() == LIGHT_CLASS_ID ) {
				GenLight *pLight = (GenLight *)os.obj;
				if( pLight ) {
					ProcessLightNode( pNode, pLight );
				}	
			} else if( os.obj->SuperClassID() == HELPER_CLASS_ID ) {
				// any helper named "ambient" export an ambient light, only for worlds
				if( m_bWorldFile ) {
					CStr sName( pNode->GetName() );
					sName.toLower();
					CStr sTarget = "ambient";
					if( sName.Substr( 0, 7 ) == sTarget ) {
						// this is an ambient light node
						ApeLight_t ApeLight;
						int nRed, nGreen, nBlue;
						CLightStringParser StringParser( m_bWorldFile );

						// zero our memory first
						fang_MemZero( &ApeLight, sizeof( ApeLight_t ) );
						
						// set the name for this light
						strncpy( ApeLight.szLightName, sTarget, (LIGHT_NAME_LEN-1) );
						
						// copy the intensity
						ApeLight.fIntensity = 1.0f * m_fLightMultiplier;
						FMATH_CLAMP_UNIT_FLOAT( ApeLight.fIntensity );

						// set the type of light
						ApeLight.nType = APE_LIGHT_TYPE_AMBIENT;
						
						// search for the color attributes
						if( pNode->GetUserPropInt( "red", nRed ) &&
							pNode->GetUserPropInt( "green", nGreen ) &&
							pNode->GetUserPropInt( "blue", nBlue ) ) {
							// all 3 attributes are present
							FMATH_CLAMP( nRed, 0, 255 );
							FMATH_CLAMP( nGreen, 0, 255 );
							FMATH_CLAMP( nBlue, 0, 255 );
							// copy the color (converting to unit float)
							ApeLight.Color.fRed = (f32)( nRed * (1.0f/255.0f) );
							ApeLight.Color.fGreen = (f32)( nGreen * (1.0f/255.0f) );
							ApeLight.Color.fBlue = (f32)( nBlue * (1.0f/255.0f) );

							// parser for star commands
							StringParser.ResetToDefaults();
							StringParser.Parse( sName );
							ApeLight.nFlags = StringParser.m_nCommands;
							ApeLight.nMotifID = StringParser.m_nMotifID;
							ApeLight.nLightID = StringParser.m_nLightID;

							// log with an error in the log
							if ( ApeLight.nFlags & APE_LIGHT_FLAG_UNIQUE_LIGHTMAP ) {
								RecordError( FALSE, "Ambient lights cannot have unique light maps.", pNode );
							}

							// write out the light to our file
							WriteToFile( &ApeLight, sizeof( ApeLight_t ), 1, TRUE );
							m_MeshInfo.nNumLights++;
						} else {
							// log with an error in the log
							RecordError( FALSE, "This node does not have the red, green, or blue set, no ambient light exported - skipping node.", pNode );
						}
					}
				}
			}
		}
	}

ENUM_CHILD:
	// Recurse through this node's children, if any
	if( bEnumChildren ) {
		for( int c=0; c < pNode->NumberOfChildren(); c++ ) {
			if( !LightNodeEnum( pNode->GetChildNode( c ) ) ) {
				return FALSE;
			}
		}
	}
	return TRUE;
}

// returns TRUE if enumeration should continue, FALSE if enumeration should stop
BOOL ApeExporter::ShapeNodeEnum( INode *pNode ) {
	BOOL bEnumChildren = TRUE;
	CStr sName( pNode->GetName() );
	ApeShape_t ApeShape;
	BOOL bAdd = FALSE;
	GizmoObject *pGizmoObject;
	CFMtx43 OriginToCenter;
	Matrix3 tm;
	CFMtx43 Rot;
	TSTR sUserProp;
	int nIntValue;

	if( m_bExportSelected && pNode->Selected() == FALSE ) {
		// this node is not selected, keep enumerating
		goto ENUM_CHILD;
	}
	// Check for user cancel
	if( m_pInterface->GetCancel() ) {
		return FALSE;
	}
	
	if( !m_bExportSelected || pNode->Selected() ) {

		ObjectState os = pNode->EvalWorldState( 0 );

		if( !ShouldNodeBeExported( pNode ) ||
			IsNodeACell( pNode ) ||
			IsNodeAPort( pNode ) ||
			IsNodeAnObject( pNode ) ) {
			// this node should not be exported, but we should keep enumerating
			bEnumChildren = FALSE;
			goto ENUM_CHILD;
		}

		if( os.obj ) {
			// zero our memory first
			fang_MemZero( &ApeShape, sizeof( ApeShape_t ) );
			OriginToCenter.Identity();

			switch( os.obj->SuperClassID() ) {
				
			case HELPER_CLASS_ID:
				if( os.obj->ClassID() == SPHEREGIZMO_CLASSID ) {
					pGizmoObject = (GizmoObject *)os.obj;

					bAdd = TRUE;
					ApeShape.nType = APE_SHAPE_TYPE_SPHERE;
					pGizmoObject->pblock->GetValue( PB_GIZMO_RADIUS, 0, ApeShape.TypeData.Sphere.fRadius, FOREVER );

				} else if( os.obj->ClassID() == CYLGIZMO_CLASSID ) {
					pGizmoObject = (GizmoObject *)os.obj;

					bAdd = TRUE;
					ApeShape.nType = APE_SHAPE_TYPE_CYLINDER;
					pGizmoObject->pblock->GetValue( PB_CYLGIZMO_RADIUS, 0, ApeShape.TypeData.Cylinder.fRadius, FOREVER );
					pGizmoObject->pblock->GetValue( PB_CYLGIZMO_HEIGHT, 0, ApeShape.TypeData.Cylinder.fHeight, FOREVER );
					OriginToCenter.m_vPos.y = ApeShape.TypeData.Cylinder.fHeight * 0.5f;

				} else if( os.obj->ClassID() == BOXGIZMO_CLASSID ) {
					pGizmoObject = (GizmoObject *)os.obj;
					
					if( IsNodeARoomBox( pNode ) ) {
						// a room box
						bAdd = TRUE;
						ApeShape.nType = APE_SHAPE_TYPE_ROOM;
						// go ahead and parse the user data for the room number
						if( !pNode->GetUserPropInt( "room", nIntValue ) ) {
							RecordError( FALSE, "No 'room=' field found - skipping room box.", pNode );
							goto ENUM_CHILD;// keep enumerating
						}
						FMATH_CLAMPMIN( nIntValue, 0 );
						ApeShape.TypeData.Room.nRoomID = (u32)nIntValue;
						pGizmoObject->pblock->GetValue( PB_BOXGIZMO_LENGTH, 0, ApeShape.TypeData.Room.fLength, FOREVER );
						pGizmoObject->pblock->GetValue( PB_BOXGIZMO_WIDTH, 0, ApeShape.TypeData.Room.fWidth, FOREVER );
						pGizmoObject->pblock->GetValue( PB_BOXGIZMO_HEIGHT, 0, ApeShape.TypeData.Room.fHeight, FOREVER );
						OriginToCenter.m_vPos.y = ApeShape.TypeData.Room.fHeight * 0.5f;
					
					} else if( IsNodeAnArenaBox( pNode ) ) {
						// an arena box
						bAdd = TRUE;
						ApeShape.nType = APE_SHAPE_TYPE_ARENA;
						pGizmoObject->pblock->GetValue( PB_BOXGIZMO_LENGTH, 0, ApeShape.TypeData.Arena.fLength, FOREVER );
						pGizmoObject->pblock->GetValue( PB_BOXGIZMO_WIDTH, 0, ApeShape.TypeData.Arena.fWidth, FOREVER );
						pGizmoObject->pblock->GetValue( PB_BOXGIZMO_HEIGHT, 0, ApeShape.TypeData.Arena.fHeight, FOREVER );
						OriginToCenter.m_vPos.y = ApeShape.TypeData.Arena.fHeight * 0.5f;
					
					} else {	
						// just a regular box shape
						bAdd = TRUE;
						ApeShape.nType = APE_SHAPE_TYPE_BOX;
						pGizmoObject->pblock->GetValue( PB_BOXGIZMO_LENGTH, 0, ApeShape.TypeData.Box.fLength, FOREVER );
						pGizmoObject->pblock->GetValue( PB_BOXGIZMO_WIDTH, 0, ApeShape.TypeData.Box.fWidth, FOREVER );
						pGizmoObject->pblock->GetValue( PB_BOXGIZMO_HEIGHT, 0, ApeShape.TypeData.Box.fHeight, FOREVER );
						OriginToCenter.m_vPos.y = ApeShape.TypeData.Box.fHeight * 0.5f;
					}
				} else {
					if( IsNodeASpeaker( pNode ) ) {
						// a regular dummy node that starts with "snd_" is a speaker shape
						ApeShape.nType = APE_SHAPE_TYPE_SPEAKER;
						// go ahead and parse the user data for radius and volume
						if( pNode->GetUserPropFloat( "soundradius", ApeShape.TypeData.Speaker.fRadius ) ) {
							if( ApeShape.TypeData.Speaker.fRadius <= 0.0f ) {
								RecordError( FALSE, "soundradius must be larger than 0.0f - skipping speaker.", pNode );
								goto ENUM_CHILD;// keep enumerating
							}							
						} else {
							RecordError( FALSE, "No 'soundradius=' field found - skipping speaker.", pNode );
							goto ENUM_CHILD;// keep enumerating
						}
						if( pNode->GetUserPropFloat( "soundvolume", ApeShape.TypeData.Speaker.fUnitVolume ) ) {
							FMATH_CLAMP( ApeShape.TypeData.Speaker.fUnitVolume, 0.0f, 1.0f );
						} else {
							RecordError( FALSE, "No 'soundvolume=' field found - skipping speaker.", pNode );
							goto ENUM_CHILD;// keep enumerating
						}
						// make sure that there does exist a 'soundfile' field
						if( !pNode->UserPropExists( "soundfile" ) ) {
							RecordError( FALSE, "No 'soundfield=' field found - skipping speaker.", pNode );
							goto ENUM_CHILD;// keep enumerating
						}
						bAdd = TRUE;

					} else if( IsNodeASpawnPoint( pNode ) ) {
						// a regular dummy node that starts with "spawn_" is a spawn point shape
						ApeShape.nType = APE_SHAPE_TYPE_SPAWN_POINT;
						bAdd = TRUE;

					} else if( IsNodeAStartPoint( pNode ) ) {
						// a regular dummy node that starts with "start_" is a start point shape
						ApeShape.nType = APE_SHAPE_TYPE_START_POINT;
						bAdd = TRUE;

					} else {
						bAdd = FALSE;
					}					
				}
				if( bAdd ) {
					// grab the location info
					tm = pNode->GetNodeTM( 0 );
					// convert the right handed max matrix to a right handed one
					Rot = (CFMtx43 &)tm;
					ApeShape.Orientation = m_Left2RightCoordSysMtx * Rot * m_Left2RightCoordSysMtx;
					// now get the center point into the center of the object
					ApeShape.Orientation *= OriginToCenter;
					
					// grab the user defined properties
					pNode->GetUserPropBuffer( sUserProp );
					ApeShape.nBytesOfUserData = sUserProp.Length();
					// write out this shape to our file
					WriteToFile( &ApeShape, sizeof( ApeShape_t ), 1, TRUE );
					m_MeshInfo.nNumShapes++;
					// write out the user defined properties for this shape
					if( ApeShape.nBytesOfUserData ) {
						WriteToFile( sUserProp.data(), ApeShape.nBytesOfUserData, 1, TRUE );	
					}
					// record this node's handle so that we can fixup the parent index later
					m_panNodeHandles[m_nHandlesUsed] = pNode->GetHandle();
					m_nHandlesUsed++;
					FASSERT( m_nHandlesUsed <= m_nNumHandlesAllocated );
				}
				break;

			case CAMERA_CLASS_ID:
				//ProcessCameraNode( pNode, os );	//commented out because it's no longer needed.			
				break;

			case SHAPE_CLASS_ID:
				ProcessShapeNode( pNode, os );
				break;
			}
		}
	}

ENUM_CHILD:
	// Recurse through this node's children, if any
	if( bEnumChildren ) {
		for( int c=0; c < pNode->NumberOfChildren(); c++ ) {
			if( !ShapeNodeEnum( pNode->GetChildNode( c ) ) ) {
				return FALSE;
			}
		}
	}
	return TRUE;
}

// returns TRUE if enumeration should continue, FALSE if enumeration should stop
BOOL ApeExporter::ObjectNodeEnum( INode* pNode ) {
	BOOL bEnumChildren = TRUE;

	if( m_bExportSelected && pNode->Selected() == FALSE ) {
		// this node is not selected, see if any of children are
		goto ENUM_CHILD;
	}

	// Check for user cancel
	if( m_pInterface->GetCancel() ) {
		return FALSE;
	}

	if( !m_bExportSelected || pNode->Selected() ) {

		ObjectState os = pNode->EvalWorldState(0);

		if( !ShouldNodeBeExported( pNode ) ||
			IsNodeACell( pNode ) ||
			IsNodeAPort( pNode ) ) {
			bEnumChildren = FALSE;
			goto ENUM_CHILD;
		}

		if( os.obj ) {

			// see if this node has been named as an object - node name begins with "obj_"
			if( IsNodeAnObject( pNode ) ) {
				ProcessObjectNode( pNode );
			}			
		}
	}

ENUM_CHILD:
	// Recurse through this node's children, if any
	if( bEnumChildren ) {
		for( int c=0; c < pNode->NumberOfChildren(); c++ ) {
			if( !ObjectNodeEnum( pNode->GetChildNode( c ) ) ) {
				return FALSE;
			}
		}
	}
	return TRUE;
}

// returns TRUE if enumeration should continue, FALSE if enumeration should stop
BOOL ApeExporter::GeometryNodeEnum( INode *pRootNode, INode* pNode ) {
	BOOL bEnumChildren = TRUE;

	if( m_bExportSelected && pNode->Selected() == FALSE ) {
		// this node is not selected, see if any of children are
		goto ENUM_CHILD;
	}

	// Check for user cancel
	if( m_pInterface->GetCancel() ) {
		return FALSE;
	}

	if( !m_bExportSelected || pNode->Selected() ) {

		ObjectState os = pNode->EvalWorldState(0);

		if( !ShouldNodeBeExported( pNode ) ||
			IsNodeACell( pNode ) ||
			IsNodeAPort( pNode ) ) {
			bEnumChildren = FALSE;
			goto ENUM_CHILD;
		}

		if( os.obj ) {
			// see if this node has been named as an object - node name begins with "obj_"
			if( IsNodeAnObject( pNode ) && m_bWorldFile ) {
				// ignore all children of this object
				bEnumChildren = FALSE;
			} else if( os.obj->SuperClassID() == GEOMOBJECT_CLASS_ID ) {
				// the super class ID is a geo node
			
				// make sure that node is not a bone (max attaches geometry to its bones so that it can render them)
				if( !IsNodeBone( pNode ) ) 
				{
//					BOOL bNeedDel;
//					TriObject* tri = GetTriObjectFromNode( pNode, 0, bNeedDel );
//					if( tri ) 
					{
//						Mesh *pMesh = &tri->GetMesh();
						
						ProcessGeoNode( pRootNode, pNode );
						
//						if( bNeedDel ) {
//							delete tri;
//						}
					}
				}
			}
		}
	}

ENUM_CHILD:
	// Recurse through this node's children, if any
	if( bEnumChildren ) {
		for( int c=0; c < pNode->NumberOfChildren(); c++ ) {
			if( !GeometryNodeEnum( pRootNode, pNode->GetChildNode( c ) ) ) {
				return FALSE;
			}
		}
	}
	return TRUE;
}

BOOL ApeExporter::PortalNodeEnum( INode *pNode ) {
	BOOL bEnumChildren = TRUE;
	cchar *pszNodeName;

	if( m_bExportSelected && pNode->Selected() == FALSE ) {
		// this node is not selected, see if any of children are
		goto ENUM_CHILD;
	}

	// Check for user cancel
	if( m_pInterface->GetCancel() ) {
		return FALSE;
	}

	if( !m_bExportSelected || pNode->Selected() ) {

		ObjectState os = pNode->EvalWorldState(0);

		if( !ShouldNodeBeExported( pNode ) ) {
			bEnumChildren = FALSE;
			goto ENUM_CHILD;
		}

		if( os.obj ) {

			pszNodeName = pNode->GetName();

			switch( os.obj->SuperClassID() ) {

			case HELPER_CLASS_ID:
				// HELPER CLASS NODES MIGHT BE THE HEAD OF A GROUP OF CELLS / PORTALS
			
				// see if this node has been named as a cell - node name begins with "cell_"
				if( IsNodeACell( pNode ) && pNode->IsGroupHead() ) {
					// export a volume of cells and portals
					ProcessVolumeNode( pNode, os, TRUE );

					// we already exported everything below this node, no need to travel lower
					bEnumChildren = FALSE;
				}
				break;

			case GEOMOBJECT_CLASS_ID:
				// GEOMETRY CLASS NODES MIGHT BE A CELL
				
				// see if this node has been named as a cell - node name begins with "cell_"
				if( IsNodeACell( pNode ) ) {
					// export this single cell as a volume
					ProcessVolumeNode( pNode, os, FALSE );
				}
#if 0
if( IsNodeAPort( pNode ) ) {
	// see if we can 
	bEnumChildren = TRUE;
	BOOL bDelete;
	u32 nFoo = 0;

	TriObject *pTriObj = GetTriObjectFromNode( pNode, 0, bDelete );
	if( CanConvertTriObject( Class_ID( SHAPE_CLASS_ID, 0 ) ) ) {
		nFoo++;
	}
	if( CanConvertTriObject( Class_ID( RECTANGLE_CLASS_ID, 0 ) ) ) {
		nFoo++;
	}
	/*

	if( os.obj->CanConvertToType( Class_ID( RECTANGLE_CLASS_ID, 0 ) ) ) { 
		ShapeObject *pShape = (ShapeObject *)os.obj->ConvertToType( 0, Class_ID( RECTANGLE_CLASS_ID, 0 ) );
		
		if( os.obj != pShape ) {
			delete pShape;
		}
	}
	*/
	if( bDelete ) {
		delete pTriObj;
	}	
}
#endif
				break;
				
			case SHAPE_CLASS_ID:
				// SHAPES, IN PARTICULAR, RECTANGLES, MIGHT BE A PORTAL
				
				// see if this node has been named as a portal - node name begins with "port_"
				if( IsNodeAPort( pNode ) ) {
					// export this single portal
					ProcessPortalNode( pNode, os );	
#if 0
					BOOL bDelete;
					TriObject *pTriObj = GetTriObjectFromNode( pNode, 0, bDelete );
					if( bDelete ) {
						delete pTriObj;
					}
#endif
				}
				break;
			}
		}
	}

ENUM_CHILD:
	// Recurse through this node's children, if any
	if( bEnumChildren ) {
		for( int c=0; c < pNode->NumberOfChildren(); c++ ) {
			if( !PortalNodeEnum( pNode->GetChildNode( c ) ) ) {
				return FALSE;
			}
		}
	}
	return TRUE;
}


BOOL ApeExporter::CameraNodeEnum( INode *pNode, ApeExporterCameraEnumMode_e EnumMode ) {
	BOOL bEnumChildren = TRUE;
	CStr sName( pNode->GetName() );

	if( m_bExportSelected && pNode->Selected() == FALSE ) {
		// this node is not selected, keep enumerating
		goto _ENUM_CHILD;
	}
	// Check for user cancel
	if( m_pInterface->GetCancel() ) {
		return FALSE;
	}
	
	if( !m_bExportSelected || pNode->Selected() ) {

		ObjectState os = pNode->EvalWorldState( 0 );

		if( os.obj ) {
			if( os.obj->SuperClassID() == CAMERA_CLASS_ID ) {
				//check to see if we should export this camera node...
				sName.toLower();
				CStr sTarget = "cam_";
				if( sName.Substr( 0, 4 ) == sTarget ) {
					if( EnumMode == APE_CAMERA_ENUM_MODE_COUNT ) {
						m_nCamerasToExport++;
					}
					if( EnumMode == APE_CAMERA_ENUM_MODE_EXPORT ) {
						ProcessCameraNode2( pNode, os );
					}
				}
			}
		}
	}

_ENUM_CHILD:
	// Recurse through this node's children, if any
	if( bEnumChildren ) {
		for( int c=0; c < pNode->NumberOfChildren(); c++ ) {
			if( !CameraNodeEnum( pNode->GetChildNode( c ), EnumMode ) ) {
				return FALSE;
			}
		}
	}
	return TRUE;
}



BOOL ApeExporter::IsNodeAnObject( INode *pNode ) {
	CStr sName( pNode->GetName() );
	sName.toLower();
	CStr sTarget = "obj_";
	if( sName.Substr( 0, 4 ) == sTarget ) {
		return TRUE;
	}
	return FALSE;
}

BOOL ApeExporter::IsNodeASpeaker( INode *pNode ) {
	CStr sName( pNode->GetName() );
	sName.toLower();
	CStr sTarget = "snd_";
	if( sName.Substr( 0, 4 ) == sTarget ) {
		return TRUE;
	}
	return FALSE;
}

BOOL ApeExporter::IsNodeASpawnPoint( INode *pNode ) {
	CStr sName( pNode->GetName() );
	sName.toLower();
	CStr sTarget = "spawn_";
	if( sName.Substr( 0, 6 ) == sTarget ) {
		return TRUE;
	}
	return FALSE;
}

BOOL ApeExporter::IsNodeAStartPoint( INode *pNode ) {
	CStr sName( pNode->GetName() );
	sName.toLower();
	CStr sTarget = "start_";
	if( sName.Substr( 0, 6 ) == sTarget ) {
		return TRUE;
	}
	return FALSE;
}

BOOL ApeExporter::IsNodeARoomBox( INode *pNode ) {
	CStr sName( pNode->GetName() );
	sName.toLower();
	CStr sTarget = "room_";
	if( sName.Substr( 0, 5 ) == sTarget ) {
		return TRUE;
	}
	return FALSE;
}

BOOL ApeExporter::IsNodeAnArenaBox( INode *pNode ) {
	CStr sName( pNode->GetName() );
	sName.toLower();
	CStr sTarget = "arena_";
	if( sName.Substr( 0, 6 ) == sTarget ) {
		return TRUE;
	}
	return FALSE;
}

BOOL ApeExporter::IsNodeACell( INode *pNode ) {
	CStr sName( pNode->GetName() );
	sName.toLower();
	CStr sTarget = "cell_";
	if( sName.Substr( 0, 5 ) == sTarget ) {
		return TRUE;
	}
	return FALSE;
}

BOOL ApeExporter::IsNodeAPort( INode *pNode ) {
	CStr sName( pNode->GetName() );
	sName.toLower();
	CStr sTarget = "port_";
	if( sName.Substr( 0, 5 ) == sTarget ) {
		return TRUE;
	}
	return FALSE;
}

BOOL ApeExporter::IsNodeNamedBone( INode *pNode ) {
	CStr sName( pNode->GetName() );
	sName.toLower();
	CStr sTarget = "bone_";
	if( sName.Substr( 0, 5 ) == sTarget ) {
		return TRUE;
	}
	return FALSE;
}

BOOL ApeExporter::ShouldNodeBeExported( INode *pNode ) {
	CStr sName( pNode->GetName() );
	sName.toLower();
	CStr sTarget = "off_";
	if( sName.Substr( 0, 4 ) == sTarget ) {
		return FALSE;
	}
	return TRUE;
}

BOOL ApeExporter::ShouldBoneBeExported( INode *pNode ) {
	
	// see if any parent node should not be exported
	INode *pParent = pNode;
	while( !pParent->IsRootNode() ) {
		if( !ShouldNodeBeExported( pParent ) ) {
			return FALSE;
		}
		pParent = pParent->GetParentNode();
	}		
	return TRUE;
}
