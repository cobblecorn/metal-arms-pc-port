//////////////////////////////////////////////////////////////////////////////////////
// Slim.cpp - Slim class for Mettle Arms.
//
// Author: Justin Link
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
// 05/30/02 Link		Created.
// 10/24/02 Elliott		New barter system
//////////////////////////////////////////////////////////////////////////////////////

#include "Slim.h"
#include "meshentity.h"
#include "SSTable.h"
#include "floop.h"
#include "fresload.h"
#include "player.h"
#include "fanim.h"

#include "BarterTypes.h"
#include "BarterSystem.h"

//will be configurable later
#define _IDLE_THINK_TIME		5.0f
#define _IDLE_THINK_VARIANCE	3.0f
#define _ATTRACT_ANIMCHANCE		1.0f
#define _FIDGET_BLEND			0.25f
#define _FIDGET_BLEND_TIME		0.75f

#define _ATTRACT_FAR_RATEADJ	0.5f

#define _OOFIDGET_BLEND_TIME	(1.0f / _FIDGET_BLEND_TIME)

#define _PLAYER_CLOSE			(20.0f * 20.0f)
#define _PLAYER_FAR				(80.0f * 80.0f)

#define _SLIM_PATIENCE			7.0f

static cchar *_pszMeshName = "GRDIslim_00";
static cchar *_apszAnimNames[SLIMANIM_COUNT] = { 
	"ARDIidle001",	// SLIMANIM_IDLE1
	"ARDIidle002",	// SLIMANIM_IDLE2
	"ARDIidle02a",  // SLIMANIM_IDLE2A
	"ARDIidle02b",  // SLIMANIM_IDLE2B
	"ARDIidle02c",  // SLIMANIM_IDLE2C
	"ARDIflag_01",	// SLIMANIM_WAVEFLAG
	"ARDIwave_01",	// SLIMANIM_WAVEHAND
	"ARDIheadys1",	// SLIMANIM_NODYES
	"ARDIheadno1",	// SLIMANIM_NODNO
	"ARDIshrug",	// SLIMANIM_SHRUG	
	"ARDIlaugh01",	// SLIMANIM_LAUGH

	"ARDItable01",	// SLIMANIM_UNFOLDTABLE
	"ARDItable02",	// SLIMANIM_FOLDTABLE
	"ARDIsign_01",	// SLIMANIM_PULLOUTSIGN
	"ARDIsign_02",	// SLIMANIM_FLIPSIGNTO0
	"ARDIsign_03",	// SLIMANIM_FLIPSIGNTO1
	"ARDIsign_04",	// SLIMANIM_PUTAWAYSIGN	
	"ARDIthumbu1",	// SLIMANIM_THUMBUP,
	"ARDIthumbd1",	// SLIMANIM_THUMBDOWN,
	"ARDIlookl01",	// SLIMANIM_LOOKSHADY1,
	"ARDIlookl02"	// SLIMANIM_LOOKSHADY2,
};
// blender names
static cchar *_pszMainBlender	= "main";
static cchar *_pszHeadBlender	= "head";
static cchar *_pszBodyBlender	= "body";
static cchar *_pszIdleBlender	= "idle";

// tap names
static cchar *_pszMainTap1		= "main1";
static cchar *_pszHeadTap1		= "head1";
static cchar *_pszBodyTap1		= "body1";
static cchar *_pszIdleTap0		= "idle0";
static cchar *_pszIdleTap1		= "idle1";

static cchar *_pszLaughFx		= "BD_SlimLaugh";

static cchar *_papszRightArmBones[] = {
	"R_Arm_Upper",
	"R_Elbow",
	"R_Arm_lower",
	"R_Hand",
	"R_ThumbA",
	"R_ThumbB",
	"R_IndexFingerA",
	"R_IndexFingerB",
	"R_PinkyFingerA",
	"R_PinkyFingerB",
	"R_MiddleFingerA",
	"R_MiddleFingerB",
	"R_Hand_Dummy",
	"Sign",
	NULL
};

static cchar *_papszLeftArmBones[] = {
	"L_Arm_Upper",
	"L_Elbow",
	"L_Arm_lower",
	"L_Hand",
	"L_ThumbA",
	"L_ThumbB",
	"L_IndexFingerA",
	"L_IndexFingerB",
	"L_PinkyFingerA",
	"L_PinkyFingerB",
	"L_MiddleFingerA",
	"L_MiddleFingerB",
	"L_Hand_Dummy",
	"Flag_Stick",
	NULL
};


static cchar *_papszYesNoBones[] = {
	"Head",
	"Jaw",
	"Neck",
	NULL
};
static cchar *_papszLegBones[] = {
	"Groin",

	"R_Leg_Upper",
	"R_Knee",
	"R_Leg_Lower",
	"R_Leg_Rear",
	"R_Leg_Front",
	"R_Foot",

	"L_Leg_Upper",
	"L_Knee",
	"L_Leg_Lower",
	"L_Leg_Rear",
	"L_Leg_Front",
	"L_Foot",
	NULL
};

static cchar *_papszLeftHandBones[] = {
	"L_Hand",
	"L_MiddleFingerA",
	"L_MiddleFingerB",
	"L_ThumbA",
	"L_ThumbB",
	NULL
};

static const f32 _ONE_EIGHTH_SEC = (1.0f/8.0f);
static const f32 _ONE_FOURTH_SEC = (1.0f/4.0f);
static const f32 _ONE_HALF_SEC = (1.0f/2.0f);
static const f32 _THREE_FOURTH_SEC = (3.0f/4.0f);
static const f32 _ONE_SEC = 1.0f;


static const f32 _afFailedReactions[] = {	0.4f,		//SHAKE HEAD
											0.6f,		//THUMBS DOWN
											0.8f,		//POINT AT SIGN
											0.0f,		//LAUGH
											0.87f,		//SHAKE HEAD & THUMBS DOWN
											0.95f,		//SHAKE HEAD & POINT AT SIGN
											1.0f };		//SHAKE HEAD & LAUGH 

static const f32 _afPurchaseReactions[] = {	0.4f,		//NOD HEAD
											0.6f,		//THUMBS UP
											0.7f,		//LAUGH
											0.9f,		//NOD & THUMBS UP
											1.0f };		//NOD & LAUGH

static const f32 _afRejectedReactions[] = { 0.5f,		//SHRUG
											0.75f,		//SHAKE HEAD
											1.0f };		//SHRUG & SHAKE HEAD

static const f32 _afPosExitReactions[] = { 0.3f,		//THUMBS UP
										   0.6f,		//WAVE
										   0.8f,		//NOD & THUMBS UP
										   1.0f };		//NOD & WAVE

static const f32 _afNegExitReactions[] = { 0.2f,		//FLIP OFF
										   0.4f,		//THUMBS DOWN
										   0.6f,		//SHRUG
										   0.8f,		//SHAKE HEAD & THUMBS DOWN
										   1.0f };		//SHAKE HEAD & SHRUG
										   



SlimAnim_e CSlim::m_anFidgetAnims[]		= { SLIMANIM_LAUGH, SLIMANIM_WAVEHAND, SLIMANIM_NODNO, SLIMANIM_NODYES };
f32		   CSlim::m_afFidgetTimeAdj[]	= { 0.4f, 0.4f, 0.15f, 0.15f };
f32		   CSlim::m_afFidgetMaxBlend[]	= { 0.2f, 0.2f, 0.2f,  0.2f };
cchar	 **CSlim::m_apszFidgetBones[]	= { _papszLegBones, _papszLegBones, _papszYesNoBones, _papszYesNoBones };


u32 CSlim::m_uFilterBone = -1;

FSndFx_FxHandle_t CSlim::m_hLaughSound	= FSNDFX_INVALID_FX_HANDLE;



CSlim::CSlim() {
	u32 i;

	m_pMesh		= NULL;
	m_pTable	= NULL;
	m_pManMtx	= NULL;
	m_uFlags	= 0;

	m_fPatienceTimer	= _SLIM_PATIENCE;
	m_nPatienceCounter	= 0;
	m_bDrawInsultFilter = FALSE;
	m_nLastPrice		= 0;
	m_bDelayedExit		= FALSE;

	// zero everything out to start
	for( i=0; i < SLIMANIM_COUNT; i++ ) {
		m_apAnimInsts[i] = NULL;		
	}
	m_pCombinerConfig = NULL;
	m_pAnimCombiner = NULL;
	for( i=0; i < _TAP_COUNT; i++ ) {
		m_apCurrentAnims[i] = NULL;
		m_abPaused[i] = FALSE;
		m_abClampAtEnd[i] = FALSE;
	}
}

CSlim::~CSlim() {
	m_uFlags = 0;
	m_pTable = NULL;
	m_pMesh = NULL;
}

BOOL CSlim::Create(f32 fSpeechDistance) {

	m_pMesh = fnew CMeshEntity;
	if( m_pMesh == NULL ) {
		DEVPRINTF("CSlim::Create() : Out of memory.\n");
		return(FALSE);
	}
	if( !m_pMesh->Create( _pszMeshName, 0, NULL, "Slim", NULL ) ) {
		DEVPRINTF("CSlim::Create() : Error during creation of Slim.\n");
		return(FALSE);
	}
	m_pMesh->SetMeshInstFlags( FMESHINST_FLAG_CAST_SHADOWS );

	m_pTable = fnew CSSTable;
	if( m_pTable == NULL ) {
		DEVPRINTF("CSlim::Create() : Out of memory.\n");
		return(FALSE);
	}
	if( !m_pTable->Create() ) {
		DEVPRINTF("CSlim::Create() : Error during creation of table.\n");
		return(FALSE);
	}

	if( !_LoadAndConfigAnimations() ) {
		DEVPRINTF("CSlim::Create() : Error during creation of animations.\n");
		return(FALSE);	
	}

	m_hLaughSound = fsndfx_GetFxHandle(_pszLaughFx);
	if( m_hLaughSound == FSNDFX_INVALID_FX_HANDLE ) {
		DEVPRINTF("CSlim::Create() : Could not load sound effect '%s'.\n", _pszLaughFx);
	}


	// Set up the texture layer related things for the sign
	u32 uCurTexLayer;
	CFWorldMesh *pWM = m_pMesh->GetMeshInst();
	for( uCurTexLayer = 0; uCurTexLayer < 6; ++uCurTexLayer ) {
		
		m_ahTexID[uCurTexLayer] = pWM->GetTexLayerHandle( uCurTexLayer + 1 );
		if( m_ahTexID[uCurTexLayer] == FMESH_TEXLAYERHANDLE_INVALID ) {
			DEVPRINTF("CSlim::Create() : Could not find texture layer (%d) for Slim's sign!.\n", uCurTexLayer + 1);
		} else {
			pWM->AnimTC_ApplyToShader( m_ahTexID[uCurTexLayer], TRUE );
			pWM->AnimTC_AnimateScroll( m_ahTexID[uCurTexLayer], FALSE );
		}
	}
		
	m_uFlags |= SLIMFLAG_CREATED;

	// since we are not in the world yet, set our state to out of the world
	m_eCurState = SLIMSTATE_OUT_OF_WORLD;

	m_bSignSide = 1;
	
	m_fSpeechRadius = fSpeechDistance;
	return TRUE;
}

void CSlim::Destroy() {

	if( m_pTable ) {
		m_pTable->Destroy();
		fdelete( m_pTable );
		m_pTable = NULL;
	}

	if( m_pMesh ) {
		m_pMesh->Destroy();
		fdelete( m_pMesh );
		m_pMesh = NULL;
	}

	_FreeAnimationVars();
}


void CSlim::SetCallback( ActionFunction_t *pfcnAction ) {
	FASSERT( IsCreated() );
	
	// actions are how we start a barter session
	m_pMesh->SetActionFunction( pfcnAction );
	if( pfcnAction != NULL ) {
		m_pMesh->SetActionable( TRUE );
	} else {
		m_pMesh->SetActionable( FALSE );
	}
}


void CSlim::Work( BOOL bCanSeePlayer, f32 fDistSqToPlayer ) {

	if( m_eCurState == SLIMSTATE_OUT_OF_WORLD ) {
		// we aren't in the world, don't do anything
		return;
	}

	if( m_bAvailable && ((m_eCurState == SLIMSTATE_ATTRACT_FAR) ||
						 (m_eCurState == SLIMSTATE_ATTRACT_CLOSE) ||
						 (m_eCurState == SLIMSTATE_ATTRACT_MED)) ) {
		if( fDistSqToPlayer > _PLAYER_FAR ) {
			m_eCurState = SLIMSTATE_ATTRACT_FAR;
		} else if( fDistSqToPlayer < _PLAYER_CLOSE ) {
			m_eCurState = SLIMSTATE_ATTRACT_CLOSE;
		} else {
			m_eCurState = SLIMSTATE_ATTRACT_MED;
		}
	}

	_Think();

	m_fTimer -= FLoop_fPreviousLoopSecs;

	_AnimWork();

	//just do some idle work
	if( m_fTimeTillSelectAnim < 0.0f ) {
		_SelectIdleAnimation();
	}

	m_fTimeTillSelectAnim -= FLoop_fPreviousLoopSecs;

	// call the table's work function
	m_pTable->Work();
}

void CSlim::AddToWorld() {
	FASSERT( IsCreated() );

	m_pMesh->AddToWorld();

	// go ahead and attach the table to slim, it should be there all the time, except when the table is in use
	m_pTable->AttachToParent( m_pMesh, "Table_Dummy" );
	m_pTable->AddToWorld();

	// set ourself into a state
	_ConfigIdleState();

	m_bReadyToBarter	= FALSE;
	m_bAvailable		= TRUE;

	m_nMood				= 0;
	m_bReportBusy		= FALSE;
	m_fTimer			= 0.0f;
	m_bDrawInsultFilter = FALSE;
	m_bDelayedExit		= FALSE;
}


void CSlim::RemoveFromWorld() {
	FASSERT( IsCreated() );
	
	m_eCurState = SLIMSTATE_OUT_OF_WORLD;

	// make sure that the table is not attached and is removed from the world
	m_pTable->DetachFromParent();
	m_pTable->RemoveFromWorld();

	// remove slim from the world
	m_pMesh->RemoveFromWorld();
}

void CSlim::Relocate_WS( const CFMtx43A *pMtx_WS ) {
	FASSERT( IsCreated() );

	m_pMesh->Relocate_RotXlatFromUnitMtx_WS( pMtx_WS );

	// force the mtx palette to be updated so that the table moves to the new location
	m_pMesh->ComputeMtxPalette( TRUE );	
}

void CSlim::AppendTrackerSkipList(u32& nTrackerSkipListCount, CFWorldTracker ** apTrackerSkipList) {

	// add both ourself and the table
	m_pMesh->AppendTrackerSkipList(nTrackerSkipListCount,apTrackerSkipList);
	m_pTable->AppendTrackerSkipList(nTrackerSkipListCount,apTrackerSkipList);
}



void CSlim::_RefreshSign() {
	u32 uCurPrice;

	FASSERT( m_TableData.nCurrentSlot < BARTER_TYPES_NUM_SLOTS );
	FASSERT( m_TableData.apSlots[m_TableData.nCurrentSlot] );

	uCurPrice = m_TableData.apSlots[m_TableData.nCurrentSlot]->GetPrice();
	//also save it
	m_nLastPrice = m_TableData.apSlots[m_TableData.nCurrentSlot]->GetPrice();

	CFWorldMesh *pWM = m_pMesh->GetMeshInst();
	FASSERT( pWM != NULL );

	u32 uCurDigit;
	CFVec2 vecSTScroll;
	vecSTScroll.x = 0.0f;

	u32 uCurTexLayer;
	u32 uTexLayerOfs = ( m_bSignSide ) ? 0: 3 ;
	for( uCurTexLayer = 0; uCurTexLayer < 3; ++uCurTexLayer ) {
		uCurDigit = uCurPrice % 10;
		uCurPrice /= 10;
		vecSTScroll.y = 0.09f * (f32)(uCurDigit);
		pWM->AnimTC_SetScrollST( m_ahTexID[2 - uCurTexLayer + uTexLayerOfs], vecSTScroll );
	}
}


void CSlim::_ConfigIdleState() {

	m_fIdleAnimSpeedAdj		 = 1.0f;
	m_fTimeTillSelectAnim	 = _IDLE_THINK_TIME + _IDLE_THINK_VARIANCE * fmath_RandomFloat();

	m_pAnimCombiner->AttachToTap( m_anTaps[_IDLE_TAP0], m_apAnimInsts[SLIMANIM_IDLE1] );
	m_abPaused[_IDLE_TAP0]			= FALSE;
	m_abClampAtEnd[_IDLE_TAP0]		= FALSE;
	m_apCurrentAnims[_IDLE_TAP0]	= m_apAnimInsts[SLIMANIM_IDLE1];

	m_pAnimCombiner->SetControlValue( m_anControls[_IDLE_CONTROL], 0.0f );
	m_apCurrentAnims[_IDLE_TAP0]->UpdateUnitTime( 0.0f );

	m_eCurState = SLIMSTATE_ATTRACT_CLOSE;
}


// NOTE: The mesh inst must be setup before calling
BOOL CSlim::_LoadAndConfigAnimations() {
	u32 i;
	FResFrame_t ResFrame = fres_GetFrame();
	FAnim_t *pAnim;

	// zero everything out to start
	for( i=0; i < SLIMANIM_COUNT; i++ ) {
		m_apAnimInsts[i] = NULL;		
	}
	m_pCombinerConfig = NULL;
	m_pAnimCombiner = NULL;
	for( i=0; i < _TAP_COUNT; i++ ) {
		m_apCurrentAnims[i] = NULL;
		m_abPaused[i] = FALSE;
		m_abClampAtEnd[i] = FALSE;
	}
	for( i=0; i < _CONTROL_COUNT; i++ ) {
		m_abAnimateControl[i]		= FALSE;
		m_afControlAnimTime[i]		= 0.0f;
		m_afOOControlAnimTime[i]	= 0.0f;
		m_afControlStart[i]			= 0.0f;
		m_afControlEnd[i]			= 0.0f;
		m_abControlReset[i]			= FALSE;
		m_afControlResetTime[i]		= 0.0f;
		m_afOOControlResetTime[i]	= 0.0f;
	}
	
	// load the animations, allocate an animinst, and create the animinst
	for( i=0; i < SLIMANIM_COUNT; i++ ) {
		pAnim = (FAnim_t *)fresload_Load( FANIM_RESNAME, _apszAnimNames[i] );
		if( !pAnim ) {
			goto _EXIT_WITH_ERROR;
		}
		m_apAnimInsts[i] = fnew CFAnimInst;
		if( !m_apAnimInsts[i] ) {
			goto _EXIT_WITH_ERROR;
		}
		if( !m_apAnimInsts[i]->Create( pAnim ) ) {
			goto _EXIT_WITH_ERROR;
		}
	}

	// set up stuff for our obscene gesture
	m_pManMtx = fnew CFAnimManMtx;
	if(m_pManMtx == NULL)
	{
		DEVPRINTF("CSlim::Create() : Out of memory.\n");
		return(FALSE);
	}
	if( !m_pManMtx->Create( 5, _papszLeftHandBones ) ) {
		DEVPRINTF("CSlim::Create() : Could not Create() CFAnimManMtx.\n");
		goto _EXIT_WITH_ERROR;
	}

	if( !_InitInsult() ) {
		DEVPRINTF("CSlim::Create() : Could not init insults.\n");
		goto _EXIT_WITH_ERROR;
	}


	// now that all animations are loaded, let's allocate and setup the combiner config
	m_pCombinerConfig = fnew CFAnimCombinerConfig;
	if( !m_pCombinerConfig ) {
		goto _EXIT_WITH_ERROR;
	}
	// create the blenders
	if( !m_pCombinerConfig->BeginCreation( _pszMainBlender, CFAnimMixer::TYPE_BLENDER ) ) {
		goto _EXIT_WITH_ERROR;
	}
	if( !m_pCombinerConfig->AddBlender( _pszHeadBlender, _pszMainBlender, 0 ) ) {
		goto _EXIT_WITH_ERROR;
	}
	if( !m_pCombinerConfig->AddBlender( _pszBodyBlender, _pszHeadBlender, 0 ) ) {
		goto _EXIT_WITH_ERROR;
	}
	if( !m_pCombinerConfig->AddBlender( _pszIdleBlender, _pszBodyBlender, 0 ) ) {
		goto _EXIT_WITH_ERROR;
	}
	if( !m_pCombinerConfig->AddTap( _pszIdleTap0, _pszIdleBlender, 0 ) ) {
		goto _EXIT_WITH_ERROR;
	}
	if( !m_pCombinerConfig->AddTap( _pszIdleTap1, _pszIdleBlender, 1 ) ) {
		goto _EXIT_WITH_ERROR;
	}
	if( !m_pCombinerConfig->AddTap( _pszBodyTap1, _pszBodyBlender, 1 ) ) {
		goto _EXIT_WITH_ERROR;
	}
	if( !m_pCombinerConfig->AddTap( _pszHeadTap1, _pszHeadBlender, 1 ) ) {
		goto _EXIT_WITH_ERROR;
	}
	if( !m_pCombinerConfig->AddTap( _pszMainTap1, _pszMainBlender, 1 ) ) {
		goto _EXIT_WITH_ERROR;
	}
	if( !m_pCombinerConfig->EndCreation() ) {
		goto _EXIT_WITH_ERROR;
	}

	// finally allocate and setup our combiner
	m_pAnimCombiner = fnew CFAnimCombiner;
	if( !m_pAnimCombiner ) {
		goto _EXIT_WITH_ERROR;
	}
	if( !m_pAnimCombiner->Create( m_pCombinerConfig, m_pMesh->GetMeshInst() ) ) {
		goto _EXIT_WITH_ERROR;
	}

	// grab the id of the taps
	m_anTaps[_IDLE_TAP0]	= m_pAnimCombiner->GetTapID( _pszIdleTap0 );
	m_anTaps[_IDLE_TAP1]	= m_pAnimCombiner->GetTapID( _pszIdleTap1 );
	m_anTaps[_BODY_TAP1]	= m_pAnimCombiner->GetTapID( _pszBodyTap1 );
	m_anTaps[_HEAD_TAP1]	= m_pAnimCombiner->GetTapID( _pszHeadTap1 );
	m_anTaps[_MAIN_TAP1]	= m_pAnimCombiner->GetTapID( _pszMainTap1 );
	if( m_anTaps[_IDLE_TAP0] < 0 ||
		m_anTaps[_IDLE_TAP1] < 0 ||
		m_anTaps[_BODY_TAP1] < 0 ||
		m_anTaps[_HEAD_TAP1] < 0 ||
		m_anTaps[_MAIN_TAP1] < 0 ) {
		goto _EXIT_WITH_ERROR;
	}

	// grab the controls
	m_anControls[_MAIN_CONTROL]		= m_pAnimCombiner->GetControlID( _pszMainBlender );
	m_anControls[_HEAD_CONTROL]		= m_pAnimCombiner->GetControlID( _pszHeadBlender );
	m_anControls[_BODY_CONTROL]		= m_pAnimCombiner->GetControlID( _pszBodyBlender );
	m_anControls[_IDLE_CONTROL]		= m_pAnimCombiner->GetControlID( _pszIdleBlender );

	if( m_anControls[_MAIN_CONTROL] < 0 ||
		m_anControls[_HEAD_CONTROL] < 0 ||
		m_anControls[_BODY_CONTROL] < 0 ||
		m_anControls[_IDLE_CONTROL] < 0 ) {
		goto _EXIT_WITH_ERROR;
	}

	// mark that we will be driving the mesh entity ourself
	m_pMesh->DriveMeshWithAnim( TRUE );
	m_pMesh->SetExternalCombiner( 0, m_pAnimCombiner );

	for( i=0; i<_TAP_COUNT; i++ ) {
		m_abPaused[i]	 = TRUE;
		m_afAnimTimes[i] = 0.0f;
	}

	// all done
	return TRUE;
	
_EXIT_WITH_ERROR:
	_FreeAnimationVars();
	
	fres_ReleaseFrame( ResFrame );

	return FALSE;
}


void CSlim::_FreeAnimationVars() {
	u32 i;

	for( i=0; i < SLIMANIM_COUNT; i++ ) {
		if( m_apAnimInsts[i] ) {
			fdelete( m_apAnimInsts[i] );
			m_apAnimInsts[i] = NULL;
		}
	}

	if( m_pManMtx ) {
		fdelete( m_pManMtx );
		m_pManMtx = NULL;
	}
	if( m_pCombinerConfig ) {
		fdelete( m_pCombinerConfig );
		m_pCombinerConfig = NULL;
	}	
	if( m_pAnimCombiner ) {
		fdelete( m_pAnimCombiner );
		m_pAnimCombiner = NULL;
	}
	for( i=0; i < _TAP_COUNT; i++ ) {
		m_apCurrentAnims[i] = NULL;
	}
}

BOOL CSlim::_InitInsult( void ) {
//	DEVPRINTF( "Initializing slim insult\n" );

	u32 uWrist, uFingerA, uFingerB, uThumbA, uThumbB;
	CFMtx43A tmpmtx1, tmpmtx2;
	const FMeshBone_t *pTmpBone;

	uWrist		= m_pMesh->GetMeshInst()->FindBone( _papszLeftHandBones[0] );
	uFingerA	= m_pMesh->GetMeshInst()->FindBone( _papszLeftHandBones[1] );
	uFingerB	= m_pMesh->GetMeshInst()->FindBone( _papszLeftHandBones[2] );
	uThumbA		= m_pMesh->GetMeshInst()->FindBone( _papszLeftHandBones[3] );
	uThumbB		= m_pMesh->GetMeshInst()->FindBone( _papszLeftHandBones[4] );

	if( uWrist == -1 || uFingerA == -1 || uFingerB == -1 || uThumbA == -1 || uThumbB == -1 ) {
		return FALSE;
	}

	pTmpBone = &m_pMesh->GetMeshInst()->m_pMesh->pBoneArray[uWrist];
	tmpmtx1	 = pTmpBone->AtRestBoneToParentMtx;
	
	tmpmtx2.Identity();
	tmpmtx2.SetRotationZ( FMATH_DEG2RAD(80.0f) );
	tmpmtx2.RotateY( FMATH_DEG2RAD(-90.0f) );
	tmpmtx1.Mul( tmpmtx2 );
	m_pManMtx->UpdateFrame( 0, tmpmtx1 );

	pTmpBone = &m_pMesh->GetMeshInst()->m_pMesh->pBoneArray[uFingerA];
	tmpmtx1	 = pTmpBone->AtRestBoneToParentMtx;
	m_pManMtx->UpdateFrame( 1, tmpmtx1 );

	pTmpBone = &m_pMesh->GetMeshInst()->m_pMesh->pBoneArray[uFingerB];
	tmpmtx1	 = pTmpBone->AtRestBoneToParentMtx;
	m_pManMtx->UpdateFrame( 2, tmpmtx1 );

	pTmpBone = &m_pMesh->GetMeshInst()->m_pMesh->pBoneArray[uThumbA];
	tmpmtx1	 = pTmpBone->AtRestBoneToParentMtx;
	tmpmtx1.SetRotationZ( FMATH_DEG2RAD(30.0f) );
	m_pManMtx->UpdateFrame( 3, tmpmtx1 );


	pTmpBone = &m_pMesh->GetMeshInst()->m_pMesh->pBoneArray[uThumbB];
	tmpmtx1	 = pTmpBone->AtRestBoneToParentMtx;
	tmpmtx1.SetRotationZ( FMATH_DEG2RAD(30.0f) );
	m_pManMtx->UpdateFrame( 4, tmpmtx1 );

	// save this so we can draw the filter thing later
	m_uFilterBone = uFingerB;

	return TRUE;
}


// if nIndex is not -1 then, only the particular animations status will be checked
// otherwise all animations will be checkec
BOOL CSlim::_IsAnimAtEnd( f32 fUnitThresholdToEnd/*=0.99f*/, s32 nTapIndex/*=-1*/ ) {
	
	if( nTapIndex >= 0 ) {
		FASSERT( nTapIndex < _TAP_COUNT );
		// check only the specified animation
		if( m_apCurrentAnims[nTapIndex] ) {
			if( m_apCurrentAnims[nTapIndex]->GetUnitTime() < fUnitThresholdToEnd ) {
				return FALSE;
			}
		}
	} else {
		// check all animations
		for( nTapIndex=0; nTapIndex < _TAP_COUNT; nTapIndex++ ) {
			if( m_apCurrentAnims[nTapIndex] ) {
				if( m_apCurrentAnims[nTapIndex]->GetUnitTime() < fUnitThresholdToEnd ) {
					return FALSE;
				}
			}
		}
	}
	return TRUE;
}


void CSlim::_AddBlendedAnimation( SlimAnim_e nNewAnimation,
								  u32 nTapId,
								  u32 nControlID,
								  BOOL bClampMode,
								  BOOL bPause,
								  f32 fUnitTime,
								  f32 fBlendTime,
								  f32 fTargetControlValue,
								  f32 fResetTime,
								  f32 fAnimTime,
								  cchar **apszBonesToDrive/*=NULL*/ ) {

	FASSERT( nNewAnimation < SLIMANIM_COUNT );
	FASSERT( nTapId < _TAP_COUNT );
	FASSERT( nControlID < _CONTROL_COUNT );

	
	if( nTapId != _IDLE_TAP1 ) { // it's not a fidget anim, pause the fidget anim
		m_abPaused[_IDLE_TAP1] = TRUE;
		
	}

	m_abPaused[nTapId] = bPause;
	m_abClampAtEnd[nTapId] = bClampMode;

	m_apCurrentAnims[nTapId] = m_apAnimInsts[nNewAnimation];
	
	m_pAnimCombiner->AttachToTap( m_anTaps[nTapId], m_apCurrentAnims[nTapId] );

	_SetupControlAnim( nControlID, fBlendTime, fTargetControlValue, fResetTime );
	if( !bClampMode ) {
		m_pAnimCombiner->EnableControlSmoothing( m_anControls[nControlID] );
	}

	if( apszBonesToDrive ) {
		m_pAnimCombiner->Mask_UpdateTapBoneMask( m_anTaps[nTapId], apszBonesToDrive, TRUE );
	}
	
	m_apCurrentAnims[nTapId]->UpdateUnitTime( fUnitTime );

	if( bClampMode && (fAnimTime == 0.0f) ) {
		m_afAnimTimes[nTapId] = m_apCurrentAnims[nTapId]->GetTotalTime();
	} else {
		m_afAnimTimes[nTapId] = fAnimTime;
	}
}


void CSlim::_SetupControlAnim( u32 nControlID, f32 fBlendTime, f32 fTargetControlValue, f32 fResetTime ) {

	FASSERT( nControlID < _CONTROL_COUNT );

	if( fBlendTime <= 0.0f ) {
		m_pAnimCombiner->SetControlValue( m_anControls[nControlID], fTargetControlValue );
		m_abAnimateControl[nControlID] = FALSE;
	} else {
		// blend to the target control value
		m_abAnimateControl[nControlID] = TRUE;
		m_afControlAnimTime[nControlID] = 0.0f;
		m_afOOControlAnimTime[nControlID] = fmath_Div( 1.0f, fBlendTime );
		m_afControlStart[nControlID] = m_pAnimCombiner->GetControlValue( nControlID );
		m_afControlEnd[nControlID] = fTargetControlValue;
		m_afControlResetTime[nControlID] = fResetTime;
		if( fResetTime > 0.0f ) {
			m_abControlReset[nControlID]		= TRUE;
			m_afOOControlResetTime[nControlID]  = fmath_Div( 1.0f, fResetTime );
		} else {
			m_abControlReset[nControlID] = FALSE;
		}
	}
}


BOOL CSlim::_AnimComplete( s32 nTapIndex ) {
	FASSERT(nTapIndex >= 0 && nTapIndex < _TAP_COUNT );

	return(m_afAnimTimes[nTapIndex] < 0.0f);
}


void CSlim::_AnimWork() {
	u32 i;
	f32 fPercent, fControl;
	
#if 0
	//test... 
	ftext_DebugPrintf( 0.2f, 0.7f, "~w1Idle:  %0.2f, Body:  %0.2f, Head:  %0.2f, Main:  %0.2f",
								m_pAnimCombiner->GetControlValue( _IDLE_CONTROL ),
								m_pAnimCombiner->GetControlValue( _BODY_CONTROL ),
								m_pAnimCombiner->GetControlValue( _HEAD_CONTROL ),
								m_pAnimCombiner->GetControlValue( _MAIN_CONTROL ) );
#endif
	
	// update the anim insts
	for( i=0; i < _TAP_COUNT; i++ ) {
		if( m_apCurrentAnims[i] && !m_abPaused[i] ) {
			// we're handling the idles a little differently
			if( i == _IDLE_TAP0 ) { // this is the idle tap, modify speed
				m_apCurrentAnims[i]->DeltaTime( FLoop_fPreviousLoopSecs * m_fIdleAnimSpeedAdj, m_abClampAtEnd[i] );
			} else if( i == _IDLE_TAP1 ) { 
				m_apCurrentAnims[i]->DeltaTime( FLoop_fPreviousLoopSecs * m_fIdleAnimSpeedAdj * m_fFidgetTimeAdj, m_abClampAtEnd[i] );
 				m_afAnimTimes[i] -= FLoop_fPreviousLoopSecs;
				if( m_afAnimTimes[i] < 0.0f ) {
					m_abPaused[i] = TRUE;
				}
			} else {
				//advance the animation.  if we're at the end, and clamping at the ned, pause the animation
				m_apCurrentAnims[i]->DeltaTime( FLoop_fPreviousLoopSecs, m_abClampAtEnd[i] );
				m_afAnimTimes[i] -= FLoop_fPreviousLoopSecs;
				if( m_afAnimTimes[i] < 0.0f ) {
					m_abPaused[i] = TRUE;
				}
			}
		}
	}

	// update the control values
	for( i=0; i < _CONTROL_COUNT; i++ ) {
		if( m_abAnimateControl[i] ) {
			m_afControlAnimTime[i] += FLoop_fPreviousLoopSecs;
			fPercent = m_afControlAnimTime[i] * m_afOOControlAnimTime[i];
			if( fPercent > 0.999f ) {
				fPercent = 1.0f;
				m_abAnimateControl[i] = FALSE;
				fControl = m_afControlEnd[i];
			} else {
				fControl = FMATH_FPOT( fPercent, m_afControlStart[i], m_afControlEnd[i] );
			}

			m_pAnimCombiner->SetControlValue( m_anControls[i], fControl );
		}
	}

	_CheckControlReset( _IDLE_CONTROL, _IDLE_TAP1 );
	_CheckControlReset( _MAIN_CONTROL, _MAIN_TAP1 );
	_CheckControlReset( _HEAD_CONTROL, _HEAD_TAP1 );
	_CheckControlReset( _BODY_CONTROL, _BODY_TAP1 );
}


void CSlim::_CheckControlReset( u32 nControlID, u32 nTapID ) {
	FASSERT( nControlID < _CONTROL_COUNT );
	FASSERT( nTapID < _TAP_COUNT );

	f32 fControl;
	f32 fPercent;

	//see whether the control needs to reset
	if( m_apCurrentAnims[nTapID] && 
	  m_abControlReset[nControlID] && 
	  m_afAnimTimes[nTapID] < 0.0f ) {
		m_afControlResetTime[nControlID] -= FLoop_fPreviousLoopSecs;
		fPercent = m_afControlResetTime[nControlID] * m_afOOControlResetTime[nControlID];
		if( fPercent < 0.001f ) {
			fPercent = 0.0f;
			fControl = 0.0f;
			m_abControlReset[nControlID] = FALSE;
		} else {
			fControl = FMATH_FPOT( fPercent, 0.0f, m_afControlEnd[nControlID] );
		}
		m_pAnimCombiner->SetControlValue( nControlID, fControl );
	}

	if( (nControlID == _IDLE_CONTROL) &&
		(m_abPaused[_IDLE_TAP1]) &&
		(m_pAnimCombiner->GetControlValue( _IDLE_CONTROL ) > 0.0f ) ) {
		//it was shut down by another animation, shut it down.  Just bring it down over a full second
		fControl = m_pAnimCombiner->GetControlValue( _IDLE_CONTROL ) - FLoop_fPreviousLoopSecs;
		FMATH_CLAMP_MIN0(fControl);
		m_pAnimCombiner->SetControlValue( _IDLE_CONTROL, fControl );

//		DEVPRINTF( "Idle anim left stranded, bringing it back down (%0.2f)\n", m_pAnimCombiner->GetControlValue( _IDLE_CONTROL ) );
	}

}


void CSlim::_SelectIdleAnimation( void ) {
	s32 nRnd;
	f32 fRnd;
	
	m_fTimeTillSelectAnim = _IDLE_THINK_TIME + _IDLE_THINK_VARIANCE * fmath_RandomFloat();
	
	nRnd = fmath_RandomRange( 0, 3 );

	if( nRnd < SLIMFIDGET_COUNT ) {
		_StartFidget( nRnd );
	}
	
	if( !m_bAvailable ) {
		return;
	}

	if( !fmath_RandomChance( _ATTRACT_ANIMCHANCE ) ) { 
		return;
	}

	//make sure we're not doing something else
	if( (m_pAnimCombiner->GetControlValue( _BODY_CONTROL ) > 0.0f) ||
		(m_pAnimCombiner->GetControlValue( _HEAD_CONTROL ) > 0.0f) ) {
		return;
	}

	fRnd = fmath_RandomFloat();

	switch( m_eCurState ) {
		case SLIMSTATE_ATTRACT_FAR:
			if( fRnd < 0.75f ) {
				_AddBlendedAnimation( SLIMANIM_WAVEHAND, _BODY_TAP1, _BODY_CONTROL, TRUE, FALSE, 0.0f, 0.25f, 1.0f, 0.25f, 0.0f, NULL );	
			} else {
				//fire off slim's look at shady event
				EventNotify( BARTER_SYSTEM_EVENTS_SLIM_LOOK, 0.0f );

				//tell shady look at me
				bartersystem_EventNotify( BARTER_SYSTEM_EVENTS_SHADY_LOOK_AT_SLIM, 0.0f );
			}

			
			break;

		case SLIMSTATE_ATTRACT_MED:
			if( fRnd < 0.4f ) {
				_AddBlendedAnimation( SLIMANIM_WAVEHAND, _BODY_TAP1, _BODY_CONTROL, TRUE, FALSE, 0.0f, 0.25f, 1.0f, 0.25f, 0.0f, NULL );	
			} else if( fRnd < 0.55 ) {
				//fire off slim's look at shady event
				EventNotify( BARTER_SYSTEM_EVENTS_SLIM_LOOK, 0.0f );

				//tell shady look at me
				bartersystem_EventNotify( BARTER_SYSTEM_EVENTS_SHADY_LOOK_AT_SLIM, 0.0f );
			} else if( fRnd < 0.70f ) {
				_AddBlendedAnimation( SLIMANIM_SHRUG, _BODY_TAP1, _BODY_CONTROL, TRUE, FALSE, 0.0f, 0.25f, 1.0f, 0.75f, 0.0f, NULL );
			} else if( fRnd < 0.85f ) {
				_AddBlendedAnimation( SLIMANIM_IDLE2A, _BODY_TAP1, _BODY_CONTROL, TRUE, FALSE, 0.0f, 0.25f, 1.0f, 0.0f, 0.0f, NULL );
				m_eCurState = SLIMSTATE_ATTRACTBORED;
			} else {
				_AddBlendedAnimation( SLIMANIM_WAVEFLAG, _BODY_TAP1, _BODY_CONTROL, TRUE, FALSE, 0.0f, 0.25f, 1.0f, 0.75f, 0.0f, NULL );
				m_fTimeTillSelectAnim += 5.0f;
			}
			break;

		case SLIMSTATE_ATTRACT_CLOSE:
			if( fRnd < 0.2f ) {
				_AddBlendedAnimation( SLIMANIM_WAVEHAND, _BODY_TAP1, _BODY_CONTROL, TRUE, FALSE, 0.0f, 0.25f, 1.0f, 0.25f, 0.0f, NULL );	
			} else if( fRnd < 0.4 ) {
				//fire off slim's look at shady event
				EventNotify( BARTER_SYSTEM_EVENTS_SLIM_LOOK, 0.0f );
				//tell shady look at me
				bartersystem_EventNotify( BARTER_SYSTEM_EVENTS_SHADY_LOOK_AT_SLIM, 0.0f );
			} else if( fRnd < 0.6f ) {
				_AddBlendedAnimation( SLIMANIM_THUMBUP, _BODY_TAP1, _BODY_CONTROL, TRUE, FALSE, 0.0f, 0.25f, 1.0f, 0.75f, 0.0f, NULL );
			} else if( fRnd < 0.8f ) {
				_AddBlendedAnimation( SLIMANIM_IDLE2, _BODY_TAP1, _BODY_CONTROL, TRUE, FALSE, 0.0f, 0.25f, 1.0f, 0.75f, 0.0f, _papszRightArmBones );
			} else if( fRnd < 0.9f ) {
				_AddBlendedAnimation( SLIMANIM_SHRUG, _BODY_TAP1, _BODY_CONTROL, TRUE, FALSE, 0.0f, 0.25f, 1.0f, 0.75f, 0.0f, NULL );
			} else {
				_AddBlendedAnimation( SLIMANIM_NODYES, _BODY_TAP1, _BODY_CONTROL, TRUE, FALSE, 0.0f, 0.25f, 1.0f, 0.75f, 0.0f, NULL );
			}
			break;
				

	}
		
	m_fTimeTillSelectAnim *= _ATTRACT_FAR_RATEADJ;

	m_fIdleAnimSpeedAdj = fmath_RandomFloatRange( 0.66f, 1.75f );
}


void CSlim::_StartFidget( u32 nFidget ) {
	FASSERT( nFidget < SLIMFIDGET_COUNT );
	
	SlimFidget_e fidget = (SlimFidget_e)nFidget;

	m_fFidgetTimeAdj  = m_afFidgetTimeAdj[nFidget];
	
	if( m_abPaused[_IDLE_TAP1] ) {
		if( !m_abPaused[_HEAD_TAP1] || !m_abPaused[_BODY_TAP1] ) {
			return;
		}

		if( m_pAnimCombiner->GetControlValue( _IDLE_CONTROL ) > 0.0f ) {
			return;
		}

		_AddBlendedAnimation( m_anFidgetAnims[fidget], _IDLE_TAP1, _IDLE_CONTROL, FALSE, FALSE, 0.0f, 0.75f, 0.25f, 0.33f, 5.0f * m_fTimeTillSelectAnim, m_apszFidgetBones[fidget] );
	}
}



void CSlim::Welcome( const BarterTypes_TableData_t *pTable, BOOL bFirstTime/*=FALSE*/ ) {
	m_bReadyToBarter	= FALSE;		//should have been already
	m_bAvailable		= FALSE;
	m_bReportBusy		= TRUE;
	m_bDelayedExit		= FALSE;

	m_TableData = *pTable;				// save a copy of this

	m_eCurState	= SLIMSTATE_GETSIGN;
	m_pTable->StartUnfold();
	_AddBlendedAnimation( SLIMANIM_UNFOLDTABLE, _MAIN_TAP1, _MAIN_CONTROL, TRUE, FALSE, 0.0f, 0.25f, 1.0f, 0.5f, 0.0f, NULL );
	_ResetPatience();
}


void CSlim::Exit( u32 nItemsPurchased ) {
//	DEVPRINTF( "Exit called, state = %d\n", m_eCurState );
	

	m_nMood = nItemsPurchased;

	m_bReportBusy	  = TRUE;
	m_bReadyToBarter  = FALSE;
	m_bAvailable	  = FALSE;
	
	if( m_eCurState != SLIMSTATE_BARTER ) {
		m_bDelayedExit = TRUE;
		return;
	}

	m_fTimer	= m_pTable->LastSpin();
	m_eCurState	= SLIMSTATE_STOWSIGN;
}


void CSlim::SetCurrentSlot( u32 nSlotIndex, CBarterItemInst *pNewBII, BOOL bFlipDown ) 
{
	FASSERT( nSlotIndex < BARTER_TYPES_NUM_SLOTS );

	m_pTable->SetCurrentItemIdx( nSlotIndex );
	m_TableData.nCurrentSlot = nSlotIndex;
	
	if (pNewBII)
	{
		m_pTable->SetBarterItemInst(nSlotIndex, pNewBII, bFlipDown);
		m_TableData.apSlots[m_TableData.nCurrentSlot] = pNewBII;
	}

	if( (m_eCurState == SLIMSTATE_BARTER) || (m_eCurState == SLIMSTATE_BOREDDRUM) ) {		//may not be, may be doing the bored animations
		_FlipSign();
	}
}


void CSlim::Purchase( u32 nUpsaleIndex ) {
	f32 fRnd;

	if( m_eCurState == SLIMSTATE_BARTER ) {
		//fire off nodding or laughing animations

		fRnd = fmath_RandomFloat();
			
		if( fRnd < _afPurchaseReactions[0] ) {			//NOD HEAD
			_AddBlendedAnimation( SLIMANIM_NODYES, _HEAD_TAP1, _HEAD_CONTROL, FALSE, FALSE, 0.0f, 0.25f, 1.0f, 0.5f, 2.0f, _papszYesNoBones );
		} else if( fRnd < _afPurchaseReactions[1] ) {	//THUMBS UP
			_AddBlendedAnimation( SLIMANIM_THUMBUP, _BODY_TAP1, _BODY_CONTROL, TRUE, FALSE, 0.0f, 0.25f, 0.75f, 2.0f, 0.0f, NULL );
		} else if( fRnd < _afPurchaseReactions[2] ) {	//LAUGH
			_AddBlendedAnimation( SLIMANIM_LAUGH, _BODY_TAP1, _BODY_CONTROL, TRUE, FALSE, 0.0f, 0.25f, 0.75f, 2.0f, 0.0f, NULL );
			fsndfx_Play2D( m_hLaughSound );
		} else if( fRnd < _afPurchaseReactions[3] ) {	//NOD & THUMBS UP
			_AddBlendedAnimation( SLIMANIM_NODYES, _HEAD_TAP1, _HEAD_CONTROL, FALSE, FALSE, 0.0f, 0.25f, 1.0f, 0.5f, 2.0f, _papszYesNoBones );
			_AddBlendedAnimation( SLIMANIM_THUMBUP, _BODY_TAP1, _BODY_CONTROL, TRUE, FALSE, 0.0f, 0.25f, 0.75f, 2.0f, 0.0f, NULL );
		} else {										//NOD & LAUGH
			_AddBlendedAnimation( SLIMANIM_NODYES, _HEAD_TAP1, _HEAD_CONTROL, FALSE, FALSE, 0.0f, 0.25f, 1.0f, 0.5f, 2.0f, _papszYesNoBones );
			_AddBlendedAnimation( SLIMANIM_LAUGH,  _BODY_TAP1, _BODY_CONTROL, TRUE, FALSE, 0.0f, 0.25f, 0.75f, 2.0f, 0.0f, NULL );
			fsndfx_Play2D( m_hLaughSound );
		}
	} else {
		//we're doing the bored animations, just nod
		_AddBlendedAnimation( SLIMANIM_NODYES, _HEAD_TAP1, _HEAD_CONTROL, FALSE, FALSE, 0.0f, 0.25f, 1.0f, 0.5f, 2.0f, _papszYesNoBones );
	}

	m_pTable->FlashTable();

	_ResetPatience();
}



void CSlim::FailedPurchase( u32 nUpsaleIndex ) {
	f32 fRnd;

	m_pTable->OverRideSegmentFlashState( 1.0f, 0.0f );

	if( m_eCurState == SLIMSTATE_BARTER ) {
		//shake head or laugh (or both)
		fRnd = fmath_RandomFloat();

		if( fRnd < _afFailedReactions[0] ) {		//SHAKE HEAD
			_AddBlendedAnimation( SLIMANIM_NODNO,		_HEAD_TAP1, _HEAD_CONTROL, FALSE, FALSE, 0.0f, 0.25f, 1.0f, 0.5f, 2.0f, _papszYesNoBones );
		} else if( fRnd < _afFailedReactions[1] ) {	//THUMBS DOWN
			_AddBlendedAnimation( SLIMANIM_THUMBDOWN, _BODY_TAP1, _BODY_CONTROL, TRUE, FALSE, 0.0f, 0.25f, 1.0f, 0.5f, 0.0f,	NULL );
		} else if( fRnd < _afFailedReactions[2] ) {	//POINT AT THE SIGN
			_AddBlendedAnimation( SLIMANIM_PULLOUTSIGN, _BODY_TAP1, _BODY_CONTROL, TRUE, FALSE, 0.2f, 0.25f, 1.0f, 0.5f, 2.0f, _papszLeftArmBones );
		} else if( fRnd < _afFailedReactions[3] ) {	//LAUGH
			_AddBlendedAnimation( SLIMANIM_LAUGH, _BODY_TAP1, _BODY_CONTROL, TRUE, FALSE, 0.0f, 0.25f, 1.0f, 0.75f, 0.0f, NULL );
			fsndfx_Play2D( m_hLaughSound );
		} else if( fRnd < _afFailedReactions[4] ) {	//THUMB DOWN & SHAKE HEAD
			_AddBlendedAnimation( SLIMANIM_NODNO,	  _HEAD_TAP1, _HEAD_CONTROL, FALSE, FALSE, 0.0f, 0.25f, 1.0f, 0.5f, 2.0f,	_papszYesNoBones );
			_AddBlendedAnimation( SLIMANIM_THUMBDOWN, _BODY_TAP1, _BODY_CONTROL, TRUE, FALSE, 0.0f, 0.25f, 1.0f, 0.5f, 0.0f,	NULL );
		} else if( fRnd < _afFailedReactions[5] ) {	//POINT AT SIGN & SHAKE HEAD
			_AddBlendedAnimation( SLIMANIM_PULLOUTSIGN, _BODY_TAP1, _BODY_CONTROL, TRUE, FALSE, 0.2f, 0.25f, 1.0f, 0.5f, 2.0f, _papszLeftArmBones );
			_AddBlendedAnimation( SLIMANIM_NODNO,		_HEAD_TAP1, _HEAD_CONTROL, FALSE, FALSE, 0.0f, 0.25f, 1.0f, 0.5f, 2.0f, _papszYesNoBones );
		} else {									//LAUGH & SHAKE HEAD
			_AddBlendedAnimation( SLIMANIM_LAUGH, _BODY_TAP1, _BODY_CONTROL, TRUE, FALSE, 0.0f, 0.25f, 1.0f, 0.75f, 0.0f, NULL );
			fsndfx_Play2D( m_hLaughSound );
			_AddBlendedAnimation( SLIMANIM_NODNO, _HEAD_TAP1, _HEAD_CONTROL, FALSE, FALSE, 0.0f, 0.25f, 1.0f, 0.5f, 2.0f, _papszYesNoBones );
		}
	} else {
		//doing the bored thing, just shake head
		_AddBlendedAnimation( SLIMANIM_NODNO, _HEAD_TAP1, _HEAD_CONTROL, FALSE, FALSE, 0.0f, 0.25f, 1.0f, 0.5f, 2.0f, _papszYesNoBones );
	}

	_ResetPatience();
}


void CSlim::RejectedItem( u32 nUpsaleIndex ) {
	f32 fRnd;

	m_pTable->OverRideSegmentFlashState( 1.0f, 0.0f );

	fRnd = fmath_RandomFloat();

	if( m_eCurState == SLIMSTATE_BARTER ) {

		if( fRnd < _afRejectedReactions[0] ) {			//SHRUG
			_AddBlendedAnimation( SLIMANIM_SHRUG, _BODY_TAP1, _BODY_CONTROL, TRUE, FALSE, 0.0f, 0.25f, 1.0f, 0.75f, 0.0f, NULL );
		} else if( fRnd < _afRejectedReactions[1] ) {	//SHAKE HEAD
			_AddBlendedAnimation( SLIMANIM_NODNO,		_HEAD_TAP1, _HEAD_CONTROL, FALSE, FALSE, 0.0f, 0.25f, 1.0f, 0.5f, 2.0f, _papszYesNoBones );
		} else {										//SHRUG & SHAKE HEAD
			_AddBlendedAnimation( SLIMANIM_SHRUG, _BODY_TAP1, _BODY_CONTROL, TRUE, FALSE, 0.0f, 0.25f, 1.0f, 0.75f, 0.0f, NULL );
			_AddBlendedAnimation( SLIMANIM_NODNO,		_HEAD_TAP1, _HEAD_CONTROL, FALSE, FALSE, 0.0f, 0.25f, 1.0f, 0.5f, 2.0f, _papszYesNoBones );
		}
	} else {
		//doing the bored thing, just shake head
		_AddBlendedAnimation( SLIMANIM_NODNO, _HEAD_TAP1, _HEAD_CONTROL, FALSE, FALSE, 0.0f, 0.25f, 1.0f, 0.5f, 2.0f, _papszYesNoBones );
	}

	_ResetPatience();
}


void CSlim::NewSale( u32 nUpsaleIndex, const BarterTypes_TableData_t *pNewTable ) {
	m_bReportBusy	= TRUE;
	m_fTimer		= 0.33f;				//give the table a couple of seconds to flip
	m_TableData		= *pNewTable;
	m_pTable->ResetTable( pNewTable );

	// if we're not bartering, don't flip the sign, it'll come out eventually
	if( (m_eCurState == SLIMSTATE_BARTER) || (m_eCurState == SLIMSTATE_BOREDDRUM) ) {
		_FlipSign();
	}
}



void CSlim::_Think( void ) {
	f32 fTmp;

	m_fTimer -= FLoop_fPreviousLoopSecs;

	switch( m_eCurState ) {
		case SLIMSTATE_BEGIN_BARTER:
			if( _IsAnimAtEnd( 0.99f, _MAIN_TAP1 ) ) {
				m_eCurState	= SLIMSTATE_BARTER;
				m_bReadyToBarter = TRUE;
			}
			break;
		
		case SLIMSTATE_INSULT:
			fTmp = m_apCurrentAnims[_BODY_TAP1]->GetUnitTime();

			if( fTmp > 0.275f && fTmp < 0.73f ) {	//we are in filter mode
				m_bDrawInsultFilter = TRUE;
			} else {
				m_bDrawInsultFilter = FALSE;
			}

			if( fTmp < 0.15f ) {
				fTmp = 0.0f;
			} else if( fTmp < 0.35f ) {
				fTmp = (fTmp - 0.15f) * 5.0f;		// 1.0f/0.2f
			} else if( fTmp < 0.7f ) {
				fTmp = 1.0f;
			} else if( fTmp < 0.8f ) {
				fTmp = fTmp = (0.8f - fTmp) * 10.0f;	// 1.0f/0.1f
			} else {
				fTmp = 0.0f;
				m_eCurState = SLIMSTATE_ATTRACT_CLOSE;
			}

			m_pAnimCombiner->SetControlValue( _HEAD_CONTROL, fTmp );
			break;

		
		case SLIMSTATE_STOWSIGN:
			if( m_fTimer < 0 ) {	//used to let the table spin the items off before we pick it up
				//doing this to clear out the main anim blender, the sign kept appearing otherwise
				_AddBlendedAnimation( SLIMANIM_PUTAWAYSIGN, _BODY_TAP1, _BODY_CONTROL, TRUE, FALSE, 0.0f, 0.5f, 1.0f, 0.25f, 0.0f, NULL );
				_AddBlendedAnimation( SLIMANIM_PUTAWAYSIGN, _MAIN_TAP1, _MAIN_CONTROL, TRUE, TRUE, 0.0f, 0.5f, 1.0f, 0.25f, 0.0f, NULL );
				m_eCurState = SLIMSTATE_FOLDTABLE;
			}
			break;

		case SLIMSTATE_FOLDTABLE:
			if( _IsAnimAtEnd( 0.99f, _MAIN_TAP1 ) ) {
				m_eCurState = SLIMSTATE_BARTER_OVER;
				m_pTable->StartFold();
				m_pTable->AttachToParent( m_pMesh, "Table_Dummy" );
				_AddBlendedAnimation( SLIMANIM_FOLDTABLE, _MAIN_TAP1, _MAIN_CONTROL, TRUE, FALSE, 0.0f, 0.00f, 1.0f, 0.5f, 0.0f, NULL );
			}
			break;

		case SLIMSTATE_GETSIGN:
			//table unfolding is _MAIN_TAP1, see if it's done.
			if( _IsAnimAtEnd( 0.99f, _MAIN_TAP1 ) ) {
				//detach the table
				m_pTable->DetachFromParent();
				
				//table's out, that's good enough for me
				m_bReportBusy	 = FALSE;
			
				//now get the sign out
				m_eCurState		 = SLIMSTATE_BEGIN_BARTER;
				
				//ok, I'm doing this because I want the arm to be masked forever, but the rest of his body to be free
				_AddBlendedAnimation( SLIMANIM_PULLOUTSIGN, _BODY_TAP1, _BODY_CONTROL, TRUE, FALSE, 0.0f, 0.25f, 1.0f, 0.25f, 0.0f, NULL );
				_AddBlendedAnimation( SLIMANIM_PULLOUTSIGN, _MAIN_TAP1, _MAIN_CONTROL, TRUE, TRUE, 0.0f, 0.25f, 1.0f, 0.0f, 0.0f, _papszRightArmBones );

				m_bSignSide = 0;
				//init the table
				m_pTable->ResetTable( &m_TableData );
				_RefreshSign();
			}
			break;
		
		case SLIMSTATE_BARTER_OVER:
			if( _IsAnimAtEnd( 0.99f, _MAIN_TAP1 ) ) {
				m_eCurState		= SLIMSTATE_ATTRACT_CLOSE;
				_DoGoodbye();
							
				m_bAvailable	= TRUE;
				m_bReportBusy	= FALSE;
			}
			break;

		case SLIMSTATE_BARTER:
			if( m_bDelayedExit ) {		// we've put off exiting, but now it's time to go
				Exit( m_nMood );
			}

			if( m_bReportBusy && m_fTimer < 0.0f ) {
				m_bReportBusy = FALSE;
			}

			//Make sure the price on the sign is the same as our active price
			FASSERT(m_TableData.nCurrentSlot >= 0 && m_TableData.nCurrentSlot < BARTER_TYPES_NUM_SLOTS );
			FASSERT(m_TableData.apSlots[m_TableData.nCurrentSlot]);

			if( m_nLastPrice != m_TableData.apSlots[m_TableData.nCurrentSlot]->GetPrice() ) {
				_FlipSign();
			}

			m_fPatienceTimer -= FLoop_fPreviousLoopSecs;
			if( m_fPatienceTimer < 0.0f ) {
				if( ++m_nPatienceCounter % 3 ) {
					m_eCurState = SLIMSTATE_BOREDDRUM;		//bored & drumming fingers
					_AddBlendedAnimation( SLIMANIM_IDLE2, _BODY_TAP1, _BODY_CONTROL, TRUE, FALSE, 0.0f, 0.25f, 1.0f, 0.75f, 0.0f, NULL );
					m_fPatienceTimer	= m_apAnimInsts[SLIMANIM_IDLE2]->GetTotalTime() + (0.5f * _SLIM_PATIENCE) + (fmath_RandomFloat() * _SLIM_PATIENCE);
				} else {
					m_eCurState = SLIMSTATE_BOREDSIGN;
					_AddBlendedAnimation( SLIMANIM_PUTAWAYSIGN, _MAIN_TAP1, _MAIN_CONTROL, TRUE, FALSE, 0.0f, 0.5f, 1.0f, 0.25f, 0.0f, NULL );

					m_fPatienceTimer	= (_SLIM_PATIENCE + _SLIM_PATIENCE) + (fmath_RandomFloat() * _SLIM_PATIENCE);
					m_nPatienceCounter += fmath_RandomRange( 0, 2 );
				}
			}
			break;

		//this is here so we know to just nod or shake head while this animation is playing
		case SLIMSTATE_BOREDDRUM:
			if( _IsAnimAtEnd( 0.99f, _BODY_TAP1 ) ) {
				m_eCurState = SLIMSTATE_BARTER;
			}
			break;

		case SLIMSTATE_BOREDSIGN:
			if( _IsAnimAtEnd( 0.99f, _MAIN_TAP1 ) ) {
				m_eCurState = SLIMSTATE_BOREDSHRUG;

				// tell shady look at me
				bartersystem_EventNotify( BARTER_SYSTEM_EVENTS_SHADY_LOOK_AT_SLIM, 0.0f );

				if( fmath_RandomChance( 0.5f ) ) {
					_AddBlendedAnimation( SLIMANIM_SHRUG, _BODY_TAP1, _BODY_CONTROL, TRUE, FALSE, 0.0f, 0.25f, 1.0f, 0.75f, 0.0f, NULL );
				} else {
					_AddBlendedAnimation( SLIMANIM_WAVEHAND, _BODY_TAP1, _BODY_CONTROL, TRUE, FALSE, 0.0f, 0.25f, 1.0f, 0.75f, 0.0f, NULL );
				}

			}
			break;

		case SLIMSTATE_BOREDSHRUG:
			//back to work
			if( _IsAnimAtEnd( 0.99f, _BODY_TAP1 ) ) {
				m_eCurState = SLIMSTATE_BORED_WAITTOBARTER;
				_AddBlendedAnimation( SLIMANIM_PULLOUTSIGN, _MAIN_TAP1, _MAIN_CONTROL, TRUE, FALSE, 0.0f, 0.25f, 1.0f, 0.0f, 0.0f, _papszRightArmBones );
				m_bSignSide = 0;
				_RefreshSign();
			}
			break;

		case SLIMSTATE_BORED_WAITTOBARTER:
			if( _IsAnimAtEnd( 0.99f, _MAIN_TAP1 ) ) {	//pull out sign happens on the main tap
				m_eCurState		= SLIMSTATE_BARTER;			//ready to go again
			}
			break;

		case SLIMSTATE_ATTRACT_CLOSE:
		case SLIMSTATE_ATTRACT_FAR:
			break;


		//NOTE:  All these states assign animation to the head control.  This is because we're blending away from the body control, and 
		//		 want to make sure the main control can still take over if necessary

		case SLIMSTATE_ATTRACTSHRUG:
			if( _AnimComplete( _BODY_TAP1 ) ) {
				_AddBlendedAnimation( SLIMANIM_SHRUG, _HEAD_TAP1, _HEAD_CONTROL, TRUE, FALSE, 0.0f, 0.75f, 1.0f, 1.0f, 0.0f, NULL );
				m_eCurState = SLIMSTATE_ATTRACT_CLOSE;
			}
			break;

		case SLIMSTATE_ATTRACTLAUGH:
			if( _AnimComplete( _BODY_TAP1 ) ) {
				_AddBlendedAnimation( SLIMANIM_LAUGH, _HEAD_TAP1, _HEAD_CONTROL, TRUE, FALSE, 0.0f, 0.33f, 1.0f, 1.0f, 0.0f, NULL );
				fsndfx_Play3D( m_hLaughSound, &m_pMesh->MtxToWorld()->m_vP, m_fSpeechRadius );
				m_eCurState = SLIMSTATE_ATTRACT_CLOSE;
			}

			break;

		case SLIMSTATE_ATTRACTBORED:
			if( _IsAnimAtEnd( 0.99f, _BODY_TAP1 ) ) {
				_AddBlendedAnimation( SLIMANIM_IDLE2B, _BODY_TAP1, _BODY_CONTROL, FALSE, FALSE, 0.0f, 0.01f, 1.0f, 1.0f, 3.0f + fmath_RandomFloat() * 5.0f, NULL );
				m_eCurState = SLIMSTATE_ATTRACTBORED2;
			}
			break;

		case SLIMSTATE_ATTRACTBORED2:
			if( _AnimComplete( _BODY_TAP1 ) ) {
				_AddBlendedAnimation( SLIMANIM_IDLE2C, _BODY_TAP1, _BODY_CONTROL, TRUE, FALSE, 0.0f, 0.5f, 1.0f, 0.25f, 0.0f, NULL );
				m_eCurState = SLIMSTATE_ATTRACT_CLOSE;
			}
			break;


		case SLIMSTATE_ATTRACTLOOKAWAY:
			if( _AnimComplete( _BODY_TAP1 ) ) {
				_AddBlendedAnimation( SLIMANIM_LOOKSHADY2, _HEAD_TAP1, _HEAD_CONTROL, TRUE, FALSE, 0.0f, 0.33f, 1.0f, 1.0f, 0.0f, NULL );
				m_eCurState = SLIMSTATE_ATTRACT_CLOSE;
			}

			break;

		case SLIMSTATE_ATTRACTNOD:
		case SLIMSTATE_ATTRACTSHAKE:
			break;
	}

}


void CSlim::_FlipSign( void ) {
	
	if( m_bSignSide ) {
		_AddBlendedAnimation( SLIMANIM_FLIPSIGNTO1, _MAIN_TAP1, _MAIN_CONTROL, TRUE, FALSE, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, _papszRightArmBones );
	} else {
		_AddBlendedAnimation( SLIMANIM_FLIPSIGNTO0, _MAIN_TAP1, _MAIN_CONTROL, TRUE, FALSE, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, _papszRightArmBones );
	}

	m_bSignSide = !m_bSignSide;

	_RefreshSign();
}



void CSlim::_ResetPatience() {
	m_fPatienceTimer	= (0.5f * _SLIM_PATIENCE) + (fmath_RandomFloat() * _SLIM_PATIENCE);
	m_nPatienceCounter	= 0;
}


void CSlim::_DoGoodbye( void ) {
	f32 fRnd = fmath_RandomFloat();

	if( m_nMood > 0 ) {		// he's happy
		if( fRnd < _afPosExitReactions[0] ) {			//THUMBS UP
			_AddBlendedAnimation( SLIMANIM_THUMBUP,  _BODY_TAP1, _BODY_CONTROL, TRUE, FALSE, 0.0f, 0.25f, 1.0f, 2.0f, 0.0f, NULL );
		} else if( fRnd <  _afPosExitReactions[1] ) {	//WAVE
			_AddBlendedAnimation( SLIMANIM_WAVEHAND, _BODY_TAP1, _BODY_CONTROL, TRUE, FALSE, 0.0f, 0.25f, 1.0f, 2.0f, 0.0f, NULL );
		} else if( fRnd <  _afPosExitReactions[2] ) {	//NOD & THUMBS UP
			_AddBlendedAnimation( SLIMANIM_THUMBUP,  _BODY_TAP1, _BODY_CONTROL, TRUE, FALSE, 0.0f, 0.25f, 1.0f, 2.0f, 0.0f, NULL );
			_AddBlendedAnimation( SLIMANIM_NODYES, _HEAD_TAP1, _HEAD_CONTROL, FALSE, FALSE, 0.0f, 0.25f, 1.0f, 0.5f, 2.0f, _papszYesNoBones );
		} else {										//NOD & WAVE
			_AddBlendedAnimation( SLIMANIM_NODYES, _HEAD_TAP1, _HEAD_CONTROL, FALSE, FALSE, 0.0f, 0.25f, 1.0f, 0.5f, 2.0f, _papszYesNoBones );
			_AddBlendedAnimation( SLIMANIM_WAVEHAND, _BODY_TAP1, _BODY_CONTROL, TRUE, FALSE, 0.0f, 0.25f, 1.0f, 2.0f, 0.0f, NULL );
		}
	} else {											// he is not happy
		if( fRnd < _afNegExitReactions[0] ) {			//FLIP OFF	
			_InsultPlayer();
		} else if( fRnd < _afNegExitReactions[1] ) {	//THUMBS DOWN
			_AddBlendedAnimation( SLIMANIM_THUMBDOWN,  _BODY_TAP1, _BODY_CONTROL, TRUE, FALSE, 0.0f, 0.25f, 1.0f, 2.0f, 0.0f, NULL );
		} else if( fRnd <  _afNegExitReactions[2] ) {	//SHRUG
			_AddBlendedAnimation( SLIMANIM_SHRUG,  _BODY_TAP1, _BODY_CONTROL, TRUE, FALSE, 0.0f, 0.25f, 1.0f, 2.0f, 0.0f, NULL );
		} else if( fRnd <  _afNegExitReactions[3] ) {	//THUMBS DOWN & SHAKE HEAD
			_AddBlendedAnimation( SLIMANIM_THUMBDOWN,  _BODY_TAP1, _BODY_CONTROL, TRUE, FALSE, 0.0f, 0.25f, 1.0f, 2.0f, 0.0f, NULL );
			_AddBlendedAnimation( SLIMANIM_NODNO,	_HEAD_TAP1, _HEAD_CONTROL, FALSE, FALSE, 0.0f, 0.25f, 1.0f, 0.5f, 2.0f, _papszYesNoBones );
		} else {										//THUMBS DOWN & SHAKE HEAD
			_AddBlendedAnimation( SLIMANIM_SHRUG,  _BODY_TAP1, _BODY_CONTROL, TRUE, FALSE, 0.0f, 0.25f, 1.0f, 2.0f, 0.0f, NULL );
			_AddBlendedAnimation( SLIMANIM_NODNO, _HEAD_TAP1, _HEAD_CONTROL, FALSE, FALSE, 0.0f, 0.25f, 1.0f, 0.5f, 2.0f, _papszYesNoBones );
		}
	}
}


void CSlim::_InsultPlayer( void ) {
	_AddBlendedAnimation( SLIMANIM_THUMBUP,  _BODY_TAP1, _BODY_CONTROL, TRUE, FALSE, 0.0f, 0.25f, 1.0f, 2.0f, 0.0f, NULL );

	m_pAnimCombiner->AttachToTap( m_anTaps[_HEAD_TAP1], m_pManMtx );

	m_eCurState = SLIMSTATE_INSULT;
}


BOOL CSlim::ShowFilter( CFVec3A *pPoint ) {
	if( !m_bDrawInsultFilter ) {
		return FALSE;
	} else {
		CFMtx43A	*pmtxLoc;

		pmtxLoc = m_pMesh->GetMeshInst()->GetBoneMtxPalette()[m_uFilterBone];
		*pPoint = pmtxLoc->m_vP;

		return TRUE;
	}
}


void CSlim::EventNotify( u32 nEvent, f32 fVal ) {
	BOOL bIgnoreAnims;

	//make sure we're not doing something else
	bIgnoreAnims = ((m_pAnimCombiner->GetControlValue( _BODY_CONTROL ) > 0.0f) ||
					(m_pAnimCombiner->GetControlValue( _HEAD_CONTROL ) > 0.0f) ||
					(m_pAnimCombiner->GetControlValue( _MAIN_CONTROL ) > 0.0f) ||
					m_bReportBusy );

	switch( nEvent ) {
		case BARTER_SYSTEM_EVENTS_SLIM_LOOK:
			if( bIgnoreAnims ) {
				break;
			}
			_AddBlendedAnimation( SLIMANIM_LOOKSHADY1, _BODY_TAP1, _BODY_CONTROL, TRUE, FALSE, 0.0f, 0.25f, 1.0f, 0.75f, 1.0f + fmath_RandomFloat(), NULL );
			m_eCurState = SLIMSTATE_ATTRACTLOOKAWAY;
			break;

		case BARTER_SYSTEM_EVENTS_SLIM_LOOK_AND_LAUGH:
			if( bIgnoreAnims ) {
				break;
			}
			_AddBlendedAnimation( SLIMANIM_LOOKSHADY1, _BODY_TAP1, _BODY_CONTROL, TRUE, FALSE, 0.0f, 0.25f, 1.0f, 0.75f, 1.0f + fmath_RandomFloat(), NULL );
			m_eCurState = SLIMSTATE_ATTRACTLAUGH;
			break;

		case BARTER_SYSTEM_EVENTS_SLIM_LOOK_AND_NOD:
			if( bIgnoreAnims ) {
				break;
			}
			_AddBlendedAnimation( SLIMANIM_LOOKSHADY1, _BODY_TAP1, _BODY_CONTROL, TRUE, FALSE, 0.0f, 0.25f, 1.0f, 1.0f, 2.0f, NULL );
			_AddBlendedAnimation( SLIMANIM_NODYES,	   _HEAD_TAP1, _HEAD_CONTROL, FALSE, FALSE, 0.0f, 0.5f, 1.0f, 1.0f, 1.5f + fmath_RandomFloat(), _papszYesNoBones );
			break;

		case BARTER_SYSTEM_EVENTS_SLIM_LOOK_AND_SHAKE:
			if( bIgnoreAnims ) {
				break;
			}
			_AddBlendedAnimation( SLIMANIM_LOOKSHADY1, _BODY_TAP1, _BODY_CONTROL, TRUE, FALSE, 0.0f, 0.25f, 1.0f, 1.0f, 2.0f, NULL );
			_AddBlendedAnimation( SLIMANIM_NODNO,	   _HEAD_TAP1, _HEAD_CONTROL, FALSE, FALSE, 0.0f, 0.5f, 1.0f, 1.0f, 1.5f + fmath_RandomFloat(), _papszYesNoBones );
			break;

		case BARTER_SYSTEM_EVENTS_SLIM_WAVE:
			if( bIgnoreAnims ) {
				break;
			}
			_AddBlendedAnimation( SLIMANIM_WAVEHAND, _BODY_TAP1, _BODY_CONTROL, TRUE, FALSE, 0.0f, 0.25f, 1.0f, 1.0f, 0.0f, NULL );
			break;


		case BARTER_SYSTEM_EVENTS_SLIM_SHADY_IS_WELCOMING:
			m_bAvailable = FALSE;	// go out of available mode, should stop any anims from running
			break;
	}
}


void CSlim::ResetToPostCreateState( void ) {
	m_pMesh->RemoveFromWorld();
	m_pTable->RemoveFromWorld();

	m_bReadyToBarter	= FALSE;
	m_bAvailable		= FALSE;

	m_nMood				= 0;
	m_bReportBusy		= FALSE;
	m_fTimer			= 0.0f;
	m_bDrawInsultFilter = FALSE;

	m_apCurrentAnims[_BODY_TAP1] = m_apAnimInsts[SLIMANIM_PULLOUTSIGN];
	m_apCurrentAnims[_MAIN_TAP1] = m_apAnimInsts[SLIMANIM_PULLOUTSIGN];
	m_pAnimCombiner->AttachToTap( m_anTaps[_BODY_TAP1], m_apCurrentAnims[_BODY_TAP1] );
	m_pAnimCombiner->AttachToTap( m_anTaps[_MAIN_TAP1], m_apCurrentAnims[_MAIN_TAP1] );
	
	// reset animations
	for(u16 nAnim=0; nAnim < SLIMANIM_COUNT; nAnim++ ) 
	{
		if (m_apAnimInsts[nAnim])
			m_apAnimInsts[nAnim]->UpdateUnitTime( 0.0f );
	}

	// reset controls
	for(u16 nControl=0; nControl < _CONTROL_COUNT; nControl++ ) 
	{
		_SetupControlAnim(nControl,0.0f,0.0f,0.0f);
	}
	// reset taps
	for(u16 i=0; i<_TAP_COUNT; i++ ) 
	{
		m_abPaused[i]	 = TRUE;
		m_afAnimTimes[i] = 0.0f;
	}
}