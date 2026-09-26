// Gesture.cpp: implementation of the CGesture class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Gesture.h"
#include "fresload.h"
#include "stdio.h"

#include "fsndfx.h"

#include "BoneGroup.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
//#define new DEBUG_NEW
#endif

///////////////////
// CGesture METHODS
///////////////////

CGesture::CGesture() {
	m_pAnim = NULL;
	m_strName.Empty();
	m_strSrcFile.Empty();
	m_uFlags = FLAG_NONE;
}

CGesture::~CGesture() {
	m_pAnim = NULL;
}

void CGesture::Copy( CGesture *pCopy ) {
	m_pAnim = pCopy->m_pAnim;
	m_strName = pCopy->m_strName;
	m_strSrcFile = pCopy->m_strSrcFile;
	m_uFlags = pCopy->m_uFlags;
}

BOOL CGesture::Init( CString strName, CString strSrcFile, u32 uFlags ) {
	m_strName = strName;
	m_strSrcFile = strSrcFile;
	
	m_uFlags = uFlags;
	
	m_pAnim = (FAnim_t *)(fresload_Load(FANIM_RESNAME, strSrcFile));
	if (m_pAnim == NULL)
	{
		CString strError;
		strError.Format("CGesture::Init() : Could not find animation '%s'\n", strSrcFile);
		AfxMessageBox(strError);
		return (FALSE);
	}
	
	return (TRUE);
}



///////////////////////
// CGestureInst METHODS
///////////////////////

CGestureInst::CGestureInst() {
	m_pGesture = NULL;
	m_pAnimInst = NULL;
	m_fStartTime = -1.0f;
	m_uAnimSlot = 0xffffffff;
}

void CGestureInst::Copy( const CGestureInst *pCopy ) {
	m_pGesture = pCopy->m_pGesture;
	m_fStartTime = pCopy->m_fStartTime;
	m_pAnimInst = pCopy->m_pAnimInst;
	m_uAnimSlot = pCopy->m_uAnimSlot;
}

BOOL CGestureInst::Init( const CGesture *pGesture, f32 fStartTime ) {
	
	if( pGesture->m_pAnim == NULL ) {
		return (FALSE);
	}
	
	m_pAnimInst = fnew CFAnimInst;
	if( m_pAnimInst == NULL ) {
		AfxMessageBox("CGestureInst::Init() : Out of memory.\n");
		return (FALSE);
	}
	
	if( !m_pAnimInst->Create(pGesture->m_pAnim) ) {
		AfxMessageBox("CGestureInst::Init() : Could not init CFAnimInst.  Possibly out of memory.\n");
		return (FALSE);
	}
	
	m_fStartTime = fStartTime;
	m_pGesture = pGesture;
	m_uAnimSlot = 0xffffffff;		// Note that this is invalid, it needs to get initialized
	//   before this CGestureInst gets used but we can't do that
	//   from in here.
	
	return (TRUE);
}

