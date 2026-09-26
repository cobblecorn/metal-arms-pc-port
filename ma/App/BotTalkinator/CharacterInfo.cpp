//////////////////////////////////////////////////////////////////////////////////////
// CharacterInfo.cpp - 
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
// 09/27/02 Starich     Created.
//////////////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "fang.h"
#include "CharacterInfo.h"
#include "Gesture.h"
#include "fresload.h"
#include "BoneGroup.h"

//////////////////////////
// CCharacterInfo METHODS
//////////////////////////

CCharacterInfo::CCharacterInfo() {
	m_bIsInitialized = FALSE;
	
	m_pMeshInst = NULL;
	m_pTalkLoopAnim = NULL;
	
	m_aoGestures = NULL;

	m_hBankHandle = FSNDFX_INVALID_BANK_HANDLE;
}

CCharacterInfo::~CCharacterInfo() {
	UnInit();
}

BOOL CCharacterInfo::Init1( u32 uMaxGestures ) {

	FASSERT( !m_bIsInitialized );
	
	m_aoGestures = new CGesture[uMaxGestures];
	if( m_aoGestures == NULL ) {
		return (FALSE);
	}
	
	m_uMaxGestures = uMaxGestures;
	m_uNumGestures = 0;
	
	if( !m_oBoneGroups.Init(10) ) {
		return (FALSE);
	}
	
	m_bIsInitialized = TRUE;
	
	return (TRUE);
}

void CCharacterInfo::UnInit() {
	
	if( m_aoGestures != NULL ) {
		delete[] m_aoGestures;
		m_aoGestures = NULL;
	}
	
	m_bIsInitialized = FALSE;
}

BOOL CCharacterInfo::InsertGesture( CString strName, CString strSrcFile, u32 uFlags ) {
	
	if( m_uNumGestures >= m_uMaxGestures ) {
		CString strError;
		strError.Format("CCharacterInfo::No space for the new gesture '%s'.\n", strName);
		AfxMessageBox(strError);
		return (FALSE);
	}
	
	BOOL bRetVal = m_aoGestures[m_uNumGestures].Init(strName, strSrcFile, uFlags);
	++m_uNumGestures;
	
	return (bRetVal);
}

BOOL CCharacterInfo::FindGesture( CString strKeyName, CGesture **ppoRetGesture ) {
	u32 uIdx;

	for( uIdx = 0; uIdx < m_uNumGestures; ++uIdx ) {
		if( m_aoGestures[uIdx].m_strName.CompareNoCase( strKeyName ) == 0 ) {
			*ppoRetGesture = & (m_aoGestures[uIdx]);
			return (TRUE);
		}
	}
	
	return (FALSE);
}

void CCharacterInfo::SetMesh( CString strMeshFileName ) {
	m_strMeshFileName = strMeshFileName;
}

BOOL CCharacterInfo::Init2() {

	FMesh_t *pMesh = (FMesh_t *)fresload_Load(FMESH_RESTYPE, m_strMeshFileName);
	if( pMesh == NULL ) {
		CString strTemp;
		strTemp.Format("CCharacterInfo::InitMesh() : Could not load mesh file '%s'.\n", m_strMeshFileName);
		AfxMessageBox(strTemp);
		return (FALSE);
	}
	
	m_pMeshInst = fnew CFMeshInst;
	if( m_pMeshInst == NULL ) {
		AfxMessageBox("CCharacterInfo::InitMesh() : Out of memory.\n");
		return (FALSE);
	}
	
	FMeshInit_t oMeshInit;
	oMeshInit.fCullDist = 0.0f;
	oMeshInit.Mtx.Identity();
	oMeshInit.Mtx.RotateY(FMATH_PI);

	// MIKE: position the model as a function of the model's bounding sphere
	oMeshInit.Mtx.m_vPos.Set(0.0f, -pMesh->BoundSphere_MS.m_Pos.y, 1.5f * pMesh->BoundSphere_MS.m_fRadius );
//	oMeshInit.Mtx.m_vPos.Set(0.0f, -2.5f, 7.0f);// justin's old method, didn't work for different sized robots
	oMeshInit.nFlags = FMESHINST_FLAG_NONE;
	oMeshInit.pMesh = pMesh;
	
	m_pMeshInst->Init(&oMeshInit);
	
	m_pTalkLoopAnim = fnew CFAnimInst;
	if( m_pTalkLoopAnim == NULL ) {
		AfxMessageBox("CCharacterInfo::InitMesh() : Out of memory.\n");
		return (FALSE);
	}
	
	CGesture *pTalkLoopGesture;
	if( !FindGesture("TalkLoop", &pTalkLoopGesture) ) {
		CString strError;
		strError.Format("CCharacterInfo::InitMesh() : Could not find talk loop gesture for '%s'.\n", m_strCharName);
		AfxMessageBox(strError);
		return (FALSE);
	}
	
	FAnim_t *pAnim = (FAnim_t *)fresload_Load(FANIM_RESNAME, pTalkLoopGesture->m_strSrcFile);
	if( pAnim == NULL ) {
		CString strError;
		strError.Format("CCharacterInfo::InitMesh() : Could not find talk loop gesture called '%s'.\n", pTalkLoopGesture->m_strSrcFile);
		AfxMessageBox(strError);
		return (FALSE);
	}
	
	if( !m_pTalkLoopAnim->Create(pAnim) ) {
		CString strError;
		strError.Format("CCharacterInfo::InitMesh() : Error creating CFAnimInst for '%s'.\n", pTalkLoopGesture->m_strSrcFile);
		AfxMessageBox(strError);
		return (FALSE);
	}
	
	if( !m_animRest.Create(m_pMeshInst->m_pMesh) ) {
		CString strError;
		strError.Format("CCharacterInfo::InitMesh() : Error creating CFAnimMeshRest for '%s'.\n", pTalkLoopGesture->m_strSrcFile);
		AfxMessageBox(strError);
		return (FALSE);
	}
	static cchar *__apszTapNames[4] = { "G0", "G1", "G2", "G3" };
	static cchar *__apszMixerNames[4] = { "Mixer0", "Mixer1", "Mixer2", "Mixer3" };
	
	if( !m_oAnimCombinerConfig.BeginCreation(__apszMixerNames[0], CFAnimMixer::TYPE_BLENDER) ) {
		AfxMessageBox("Error!");
	}
	if( !m_oAnimCombinerConfig.AddTap(__apszTapNames[0], __apszMixerNames[0], 1) ) {
		AfxMessageBox("Error!");
	}
	if( !m_oAnimCombinerConfig.AddMixer(__apszMixerNames[1], CFAnimMixer::TYPE_BLENDER, __apszMixerNames[0], 0) ) {
		AfxMessageBox("Error!");
	}
	if( !m_oAnimCombinerConfig.AddTap(__apszTapNames[1], __apszMixerNames[1], 1) ) {
		AfxMessageBox("Error!");
	}
	if( !m_oAnimCombinerConfig.AddMixer(__apszMixerNames[2], CFAnimMixer::TYPE_BLENDER, __apszMixerNames[1], 0) ) {
		AfxMessageBox("Error!");
	}
	if( !m_oAnimCombinerConfig.AddTap(__apszTapNames[2], __apszMixerNames[2], 1) ) {
		AfxMessageBox("Error!");
	}
	if( !m_oAnimCombinerConfig.AddMixer(__apszMixerNames[3], CFAnimMixer::TYPE_BLENDER, __apszMixerNames[2], 0) ) {
		AfxMessageBox("Error!");
	}
	if( !m_oAnimCombinerConfig.AddTap(__apszTapNames[3], __apszMixerNames[3], 1) ) {
		AfxMessageBox("Error!");
	}
	if( !m_oAnimCombinerConfig.AddTap("Default", __apszMixerNames[3], 0) ) {
		AfxMessageBox("Error!");
	}
	if( !m_oAnimCombinerConfig.EndCreation() ) {
		AfxMessageBox("Error!");
	}
	
	m_oAnimCombiner.Create(&m_oAnimCombinerConfig, m_pMeshInst);
	
	s32 nDefaultTapID = m_oAnimCombiner.GetTapID("default");
	FASSERT(nDefaultTapID != -1);
	m_oAnimCombiner.AttachToTap(nDefaultTapID, &m_animRest);
	
	u32 uCurIdx;
	for( uCurIdx = 0; uCurIdx < NUM_SUPPORTED_SLOTS; ++uCurIdx ) {
		m_anTapId[uCurIdx] = m_oAnimCombiner.GetTapID(__apszTapNames[uCurIdx]);
		m_anControlId[uCurIdx] = m_oAnimCombiner.GetControlID(__apszMixerNames[uCurIdx]);
	}
	
	return (TRUE);
}

// #define _JOHN_AMBIENT_WORKS		TRUE

void CCharacterInfo::Draw() {
#if 0// ADD AN OMNI LIGHT
	m_pMeshInst->ResetLightList();

	CFLight Light;
	Light.InitOmniLight( &m_pMeshInst->m_BoundSphere_MS.m_Pos, 25.0f );
	Light.SetColor( 1.0f, 1.0f, 1.0f, 1.0f );
	m_pMeshInst->ConsiderLightForRender( &Light );
#endif
	m_pMeshInst->Draw(FVIEWPORT_PLANESMASK_ALL);
}




//////////////////////////////
// CCharacterInfoList METHODS
//////////////////////////////

CCharacterInfoList::CCharacterInfoList() {
	Clear();
}

CCharacterInfoList::~CCharacterInfoList() {
	Uninit();
}

BOOL CCharacterInfoList::Init( u32 uMaxCharacters, u32 uMaxGesturePerChar ) {
	FASSERT(!m_bIsInitted);
	
	m_paCharInfo = new CCharacterInfo[uMaxCharacters];
	if( m_paCharInfo == NULL ) {
		AfxMessageBox("Out of memory.\n");
		return (FALSE);
	}
	m_uListSize = uMaxCharacters;
	
	BOOL bRetVal = TRUE;
	u32 uIdx;
	for( uIdx = 0; uIdx < uMaxCharacters; ++uIdx ) {
		bRetVal &= m_paCharInfo[uIdx].Init1(uMaxGesturePerChar);
	}
	
	FASSERT( m_uCharInfoCnt == 0 );
	
	if( bRetVal ) {
		m_bIsInitted = TRUE;
	}
	
	return (bRetVal);
}

void CCharacterInfoList::Uninit() {
	delete[] m_paCharInfo;
	Clear();	
}

CCharacterInfo *CCharacterInfoList::FindCharacterInfo( CString strCharName ) {
	FASSERT(m_bIsInitted);
	
	u32 uIdx;
	for( uIdx = 0; uIdx < m_uCharInfoCnt; ++uIdx ) {
		if( m_paCharInfo[uIdx].m_strCharName.CompareNoCase( strCharName ) == 0 ) {
			return (&m_paCharInfo[uIdx]);
		}
	}
	
	return (NULL);
}

BOOL CCharacterInfoList::LoadFromFile( CString strFileName ) {
	FASSERT(m_bIsInitted);
	
	FILE *fInFile;
	char sTemp[1024*16];
	CString strLine, sIdentifier, sValue, sBotName, strTemp, strMeshName, strBoneGroupName, 
		strBoneNames, strSFXBank, strGestName, strGestFlags;
	BOOL bCurValidName = FALSE;
	BOOL bCurValidMesh = FALSE;
	s32 auCharIdx[2];
	
	fInFile = fopen(strFileName, "r");
	if( fInFile == NULL ) {
		return (FALSE);
	}
	
	char sSigLine[1024];
	fscanf(fInFile, "%[^\n]\n", sSigLine);
	if( strcmp(sSigLine, "### BOTTALKINATOR GESTURE FILE ###") != 0 ) {
		return (FALSE);
	}
	
	CCharacterInfo *pCurCI = NULL;
	while( !feof(fInFile) ) {
		fscanf(fInFile, "%[^\n]\n", sTemp);
		strLine = sTemp;
		auCharIdx[0] = 0;
		
		if( sTemp[0] != '#' ) {

			auCharIdx[1] = strLine.Find('=', 0);
			FASSERT( auCharIdx[1] >= 0 );

			sIdentifier = strLine.Mid(0, auCharIdx[1] - 1 );
			sIdentifier.TrimRight();
			
			if( sIdentifier.Left(4).CompareNoCase( "Name" ) == 0 ) {
				// They're declaring a new bot.
				
				// Let's get the CCharacterInfo where this entry will be going.
				pCurCI = &m_paCharInfo[m_uCharInfoCnt];
				++m_uCharInfoCnt;
				
				pCurCI->m_strCharName = strLine.Right(strLine.GetLength() - auCharIdx[1] - 1);
				
				bCurValidMesh = FALSE;
				bCurValidName = TRUE;

			} else if( sIdentifier.Left(4).CompareNoCase( "Mesh" ) == 0 ) {

				// They're declaring what mesh he should use.
				if( !bCurValidName ) {
					strTemp.Format("Error in file '%s'.  Mesh field found before any name field.\n", strFileName);
					if( AfxMessageBox(strTemp, MB_OKCANCEL) == IDCANCEL ) {
						return FALSE;
					}
					continue;
				}
				
				strMeshName = strLine.Right(strLine.GetLength() - auCharIdx[1] - 1);
				
				if( bCurValidMesh ) {
					strTemp.Format("CCharacterInfoList::LoadFromFile() : Warning: Mesh field '%s' hides previous mesh field '%s'.\n", strMeshName, pCurCI->m_strMeshFileName);
					if( AfxMessageBox(strTemp, MB_OKCANCEL) == IDCANCEL ) {
						return FALSE;
					}
					continue;
				}
				
				pCurCI->SetMesh(strMeshName);
				
				bCurValidMesh = TRUE;

			} else if (sIdentifier.Left(9) == "BoneGroup" ) {

				if( !bCurValidName ) {
					strTemp.Format("Error in file '%s'.  BoneGroup field found before any name field.\n", strFileName);
					if( AfxMessageBox(strTemp, MB_OKCANCEL) == IDCANCEL ) {
						return FALSE;
					}
					continue;
				}
				
				auCharIdx[0] = auCharIdx[1];
				auCharIdx[1] = strLine.Find(',', auCharIdx[0]);
				FASSERT( auCharIdx[1] >= 0 );
				strBoneGroupName = strLine.Mid(auCharIdx[0] + 1, auCharIdx[1] - auCharIdx[0] - 1);
				
				CBoneGroup *pBG = pCurCI->m_oBoneGroups.AddGroup();
				pBG->Init(strBoneGroupName);
				strBoneNames = strLine.Right(strLine.GetLength() - auCharIdx[1] - 1);
				pBG->AddCSVBones(strBoneNames);

			} else if( sIdentifier.Left(7).CompareNoCase( "SFXBank" ) == 0 ) {

				if( !bCurValidName ) {
					strTemp.Format("Error in file '%s'.  SFXBank field found before any name field.\n", strFileName);
					if( AfxMessageBox(strTemp, MB_OKCANCEL) == IDCANCEL ) {
						return FALSE;
					}
					continue;
				}
				
				strSFXBank = strLine.Right(strLine.GetLength() - auCharIdx[1] - 1);
				
				pCurCI->m_hBankHandle = (FSndFx_BankHandle_t)fresload_Load( FSNDFX_RESTYPE, strSFXBank ); 
				if( pCurCI->m_hBankHandle == FSNDFX_INVALID_BANK_HANDLE ) {
					strTemp.Format("Could not load sound effect bank '%s' for character '%s'.\n", strSFXBank, pCurCI->m_strCharName);
					if( AfxMessageBox(strTemp, MB_OKCANCEL) == IDCANCEL ) {
						return FALSE;
					}
				}

			} else {
				// They're declaring a gesture for this bot.
				sIdentifier.TrimRight();
				
				auCharIdx[0] = auCharIdx[1];
				auCharIdx[1] = strLine.Find(',', auCharIdx[0]);
				FASSERT( auCharIdx[1] >= 0 );

				strGestName = strLine.Mid(auCharIdx[0] + 1, auCharIdx[1] - auCharIdx[0] - 1);
				strGestName.TrimRight();
								
				strGestFlags = strLine.Right(strLine.GetLength() - auCharIdx[1] - 1);
				strGestFlags.MakeLower();
				
				if( !bCurValidName ) {
					strTemp.Format("CCharacterInfoList::LoadFromFile() : Error: Gesture field '%s' found before any name field.\n", sIdentifier);
					if( AfxMessageBox(strTemp, MB_OKCANCEL) == IDCANCEL ) {
						return FALSE;
					}
					continue;
				}
				
				u32 uGestureFlags = CGesture::FLAG_NONE;
				if( strGestFlags.Find( "all" ) >= 0 ) {
					// set both the upper and lower body flag
					uGestureFlags |= CGesture::FLAG_UPPERBODY | CGesture::FLAG_LOWERBODY;
				}
				if( strGestFlags.Find( "upperbody" ) >= 0 ) {
					// set the upper body flag
					uGestureFlags |= CGesture::FLAG_UPPERBODY;
				}
				if( strGestFlags.Find( "lowerbody" ) >= 0 ) {
					// set the lower body flag
					uGestureFlags |= CGesture::FLAG_LOWERBODY;
				}
				if( strGestFlags.Find( "stickatend" ) >= 0 ) {
					// set the stick at end flag
					uGestureFlags |= CGesture::FLAG_STICKATEND;
				}
				if( strGestFlags.Find( "noblendout" ) >= 0 ) {
					// set the no blend out body flag
					uGestureFlags |= CGesture::FLAG_NOBLENDOUT;
				}
				if( strGestFlags.Find( "uppertorso" ) >= 0 ) {
					// set the upper torso flag
					uGestureFlags |= CGesture::FLAG_UPPER_TORSO;
				}				
				if( strGestFlags.Find( "lowertorso" ) >= 0 ) {
					// set the lower torso flag
					uGestureFlags |= CGesture::FLAG_LOWER_TORSO;
				}
				if( strGestFlags.Find( "leftarm" ) >= 0 ) {
					// set the left arm flag
					uGestureFlags |= CGesture::FLAG_LEFT_ARM;
				}
				if( strGestFlags.Find( "rightarm" ) >= 0 ) {
					// set the right arm flag
					uGestureFlags |= CGesture::FLAG_RIGHT_ARM;
				}
				if( strGestFlags.Find( "head" ) >= 0 ) {
					// set the head flag
					uGestureFlags |= CGesture::FLAG_HEAD;
				}
				if( !pCurCI->InsertGesture(sIdentifier, strGestName, uGestureFlags) ) {
					strTemp.Format("CCharacterInfoList::LoadFromFile() : Error: Could not add gesture '%s'.\n", sIdentifier);
					if( AfxMessageBox(strTemp, MB_OKCANCEL) == IDCANCEL ) {
						return FALSE;
					}
					continue;
				}
			}
		}
	}
	
	fclose(fInFile);
	
	return (TRUE);
}

BOOL CCharacterInfoList::InitMeshes() {
	u32 uIdx;
	BOOL bRetVal = TRUE;
	for( uIdx = 0; uIdx < m_uCharInfoCnt; ++uIdx ) {
		bRetVal &= m_paCharInfo[uIdx].Init2();
	}
	
	return (bRetVal);	
}

void CCharacterInfoList::Clear() {
	m_paCharInfo = NULL;
	m_uListSize = 0;
	m_uCharInfoCnt = 0;	
	
	m_bIsInitted = FALSE;
}
