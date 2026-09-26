////////////////////////////////////////////////////////////////////////////////////////////
//
//	COMMONMAIN.CPP
//
//		Common routines for FANG test app
//
////////////////////////////////////////////////////////////////////////////////////////////




#include "fang.h"
//#include "dtrigf.h"
#include "ftimer.h"
#include "fvid.h"
#include "floop.h"
#include "fxfm.h"
#include "frenderer.h"
#include "fresload.h"
#include "fpad.h"
#include "ftext.h"
#include "fvis.h"
#include "fanim.h"
#include "fperf.h"
#include "fmesh_coll.h"

#include "CommonMain.h"
#include "CollisionTest.h"


#define _NUM_PAD_INPUTS		12

FPadio_InputID_e anPadInputIDs[_NUM_PAD_INPUTS] = 
{
	FPADIO_INPUT_STICK_LEFT_X,
	FPADIO_INPUT_STICK_LEFT_Y,
	FPADIO_INPUT_STICK_RIGHT_X,
	FPADIO_INPUT_STICK_RIGHT_Y,
	FPADIO_INPUT_TRIGGER_RIGHT,
	FPADIO_INPUT_TRIGGER_LEFT,
	FPADIO_INPUT_CROSS_TOP,
	FPADIO_INPUT_CROSS_BOTTOM,
	FPADIO_INPUT_CROSS_LEFT,
	FPADIO_INPUT_CROSS_RIGHT,
	FPADIO_INPUT_DPAD_X,
	FPADIO_INPUT_DPAD_Y,
};

FPad_Sample_t *paPadSamples[_NUM_PAD_INPUTS];

FViewport_t *g_pViewport;

static BOOL _bCamAttached = FALSE;
static CFXfm _CamXfm;
static CFTimer _LoopTimer;
static CFWorldMesh *_pModelMesh = NULL;
static CFMtx43 _mtxModelF;
static CFAnimInst _AnimInst;
static CFAnimCombiner _AnimCombiner;
static FResFrame_t _MainFrame;

#if 0
	const char *_pszLevelName = "m12fun";
	static CFVec3 _vPos( 268.5f, 3.04f, 242.2f ); // Good for m12fun
	static CFMtx43 _mtxView( 
			0.f,	0.f,	1.f, 
			0.f,	1.f,	0.f, 
			-1.f,	0.f, 	0.f, 
			0.f, 	0.f, 	0.f );
#endif			
#if 0
	const char *_pszLevelName = "WEDMmines01";
	static CFVec3 _vPos( 61.f, 28.4f, -195.9f ); // Good for mines1
	static CFMtx43 _mtxView( 
			0.778f,	0.f,	0.628f, 
			0.f,	1.f,	0.f, 
			-0.628f, 0.f, 	0.778f, 
			0.f, 	0.f, 	0.f );
#endif			
#if 0
	const char *_pszLevelName = "WEDMmines03";
	static CFVec3 _vPos( 173.f, -730.4f, 113.9f ); // Good for mines3
	static CFMtx43 _mtxView( 
			0.f,	0.f,	-1.f, 
			0.f,	1.f,	0.f, 
			1.f,	0.f, 	0.f, 
			0.f, 	0.f, 	0.f );
#endif			
#if 0
	const char *_pszLevelName = "hall_o_bots";
	static CFVec3 _vPos( -75.887f, 10.123f, -75.113f );
	static CFMtx43 _mtxView( 
			1.f,		0.f,	0.f, 
			0.f,		1.f,	0.f, 
			0.f, 		0.f, 	1.f, 
			0.f, 		0.f, 	0.f );
#endif			
#if 0
	const char *_pszLevelName = "ramptest4";
	static CFVec3 _vPos( 31.496f, 26.273f, -61.166f );
	static CFMtx43 _mtxView( 
			0.9577f,	0.f,	-0.287f, 
			0.f,		1.f,	0.f, 
			0.287f, 	0.f, 	0.9577f, 
			0.f, 		0.f, 	0.f );
#endif			
#if 1
	const char *_pszLevelName = "jump_test";
	static CFVec3 _vPos( -72.5f, 28.72f, 110.57f );
	static CFMtx43 _mtxView( 
			0.582f,	0.f,	0.813f, 
			0.f,	1.f,	0.f, 
			-0.813f, 0.f, 	0.582f, 
			0.f, 	0.f, 	0.f );
#endif			
#if 0
	const char *_pszLevelName = "weshhangr01";
	static CFVec3 _vPos( -75.887f, 10.123f, -75.113f );
	static CFMtx43 _mtxView( 
			1.f,		0.f,	0.f, 
			0.f,		1.f,	0.f, 
			0.f, 		0.f, 	1.f, 
			0.f, 		0.f, 	0.f );
#endif			


static void _ModelTest_Init( void );
static void _ModelTest_Draw( void );
static void _UpdatePadInput( void );
static void _TestNewCollision( void );
static void _TestCollision( void );

static CFVec3A _vObjVel( 0.f, 0.f, 0.f );

FINLINE void FastSinCos( f32 fAngle, f32 &fSin, f32 &fCos )
{
	fSin = fmath_Sin( fAngle );
	fCos = fmath_Cos( fAngle );
}

#define START_TIMER			PPCMtpmc1(0); PPCMtmmcr0( MMCR0_PMC1_CYCLE );
#define RESET_TIMER			PPCMtpmc1(0);
#define SAMPLE_TIMER		PPCMfpmc1();

//
//
//
BOOL LoopInit( void *pParameter )
{
#if FMESH_COLL_ENABLE_COLLISION_DRAWS
		FMesh_Coll_bShowCollision = TRUE;
#endif

	_MainFrame = fres_GetFrame();

	FDS_StreamMgr.SetCacheSize( 1024 );
	
	fresload_Load( FWORLD_RESTYPE, _pszLevelName );

//	FVis_bDrawVisibility = TRUE;

	if ( fpad_IsInstalled() )
	{
		fpad_Map( (u32)0, (u32)_NUM_PAD_INPUTS, anPadInputIDs, paPadSamples );
	}

//	floop_SetTimeScale( 0.1f );

	f32 fStart = -FMATH_2PI;
	f32 fEnd = FMATH_2PI;
	f32 fAccum1 = 0, fAccum2 = 0;
	f32 i, fTime1, fTime2;
	f32 fTest, fSin, fCos;
	START_TIMER;
	RESET_TIMER;
	for ( i = fStart; i < fEnd; i += 0.01f )
	{
		FastSinCos( i, fSin, fCos );
		fAccum1 += fSin + fCos;
	}
	fTime1 = SAMPLE_TIMER
	
	RESET_TIMER;
	for ( i = fStart; i < fEnd; i += 0.01f )
	{
		fmath_SinCos( i, &fSin, &fCos );
		fAccum2 += fSin + fCos;
	}
	fTime2 = SAMPLE_TIMER
	
	OSReport( "FAST - %f (%f)\n", fAccum1, fTime1 );
	OSReport( "SLOW - %f (%f)\n", fAccum2, fTime2 );
	
	_LoopTimer.Reset();

	_ModelTest_Init();
	
	return TRUE;
}

BOOL _bCollision;

//
//
//
void UpdateCamera( f32 fYRotation, f32 fXRotation, CFVec3 *pTranslation )
{
	_bCollision = FALSE;
	if ( !_bCamAttached )

	{
		_mtxView.RotateY( fYRotation );
		_mtxView.RotateX( fXRotation );

		_vPos += _mtxView.m_vRight * pTranslation->x;
		_vPos += _mtxView.m_vUp    * pTranslation->y;
		_vPos += _mtxView.m_vFront * pTranslation->z;
		_mtxView.m_vPos = _vPos;
	}
	else
	{
		CFVec3A vTemp;

		vTemp.Set( _mtxModelF.m_vPos );
		_mtxModelF.RotateY( fYRotation );
		_mtxModelF.RotateX( fXRotation );
		_mtxModelF.m_vPos = vTemp.v3;

//		*pTranslation = _mtxModelF.MultDir( *pTranslation );
		*pTranslation = _mtxView.MultDir( *pTranslation );

#if 1
		f32 fMag = pTranslation->Mag();
//		if ( fMag )
		{
#if FMESH_COLL_ENABLE_COLLISION_DRAWS
			FMesh_Coll_bShowCollision = TRUE;
#endif
			vTemp.Set( *pTranslation );
			CFCollData ProjInfo;
			ProjInfo.nFlags = FCOLL_DATA_FLAGS_NONE;
			ProjInfo.nCollMask = FCOLL_MASK_CHECK_ALL;
			ProjInfo.pMovement = &vTemp;
			ProjInfo.nTrackerUserTypeBitsMask = FCOLL_USER_TYPE_BITS_ALL;
			ProjInfo.pCallback = NULL;

			fcoll_Clear();
			fcoll_Check( &ProjInfo, _pModelMesh );

			if ( FColl_nImpactCount )
			{
				_bCollision = TRUE;
				FCollImpact_t *pImpact = &FColl_aImpactBuf[0];
				for ( u32 i = 1; i < FColl_nImpactCount; i++ )
				{
					if ( FColl_aImpactBuf[i].fImpactDistInfo > pImpact->fImpactDistInfo )
					{
						pImpact = &FColl_aImpactBuf[i];
					}
				}
				*pTranslation += pImpact->PushUnitVec.Mul( pImpact->fImpactDistInfo ).v3;
			}
		}
#endif
		_mtxModelF.m_vPos += *pTranslation;

#if 0
		_mtxView = _mtxModelF;
		vTemp.Set( 0.f, 3.f, -7.f );
		_mtxView.m_vPos = _mtxModelF.MultDir( vTemp.v3 );
		_mtxView.m_vPos += _mtxModelF.m_vPos;
#else
//		_mtxView = _mtxModelF;
		vTemp.Set( 0.f, 3.f, -7.f );
		_mtxView.m_vPos = _mtxView.MultDir( vTemp.v3 );
		_mtxView.m_vPos += _mtxModelF.m_vPos;
#endif

		ftext_DebugPrintf( .1f, .3f, "~al~s2.00%f", _mtxView.m_vPos.y );
	}
}

//
//
//
void SubmitInput( TestInput_e nInput )
{
	switch ( nInput )
	{
		case INPUT_TRIGGER_1:
			_vObjVel.x += _mtxView.m_vFront.x * 50.f;
			_vObjVel.y += 50.f;
			_vObjVel.z += _mtxView.m_vFront.z * 50.f;
			break;

		case INPUT_TRIGGER_2:
			_mtxModelF.m_vPos.x = 0;
			_mtxModelF.m_vPos.y = 0;
			_mtxModelF.m_vPos.z = 7.f;
			_mtxModelF.m_vPos = _mtxView.MultPoint( _mtxModelF.m_vPos );
			_vObjVel.Zero();
			break;

		default:
			break;
	}
}

//
//
//
BOOL LoopMain( BOOL bExitRequest, void *pParameter )
{
	static u32 __nFrames = 0;
	static f32 __fFrameRate = 0.f;

	_UpdatePadInput();
	
	// Build the camera Xfm
	CFMtx43 mtxInvView;
	mtxInvView = _mtxView.GetInverse();	
	_CamXfm.BuildFromMtx( mtxInvView );
	_CamXfm.InitStackWithView();
	
	fvid_Swap();
	
	__nFrames++;
	if ( _LoopTimer.SampleSeconds() > 1.f )
	{
		f32 fSeconds = _LoopTimer.SampleSeconds( TRUE );
		__fFrameRate = fSeconds / (f32)__nFrames;
		_LoopTimer.Reset();
		__nFrames = 0;
	}

	#if FANG_PLATFORM_WIN
		ftext_DebugPrintf( .1f, .1f, "~al~s2.00%5.2f", __fFrameRate * 1000.f );
	#else
		ftext_DebugPrintf( 0.07f, 0.05f, "~al~s1.00%5.2f", __fFrameRate * 1000.f );
	#endif

	// Begin Rendering
	fvid_Begin();
	
	fviewport_Clear( FVIEWPORT_CLEARFLAG_ALL, 0.f, 0.0f, 0.0f, 1.f, 0 );

	TestCollision( &_CamXfm );

	frenderer_Push( FRENDERER_MESH, NULL );

/*
	if ( _pModelMesh )
	{
		CFVec3A vTemp;
		vTemp.Set( _mtxModelF.m_vFront );
		CFCollProjInfo ProjInfo;
		ProjInfo.nFlags = FCOLL_PROJ_FLAGS_NONE;
		ProjInfo.nCollMask = FCOLL_MASK_CHECK_ALL;
		ProjInfo.pMovement = &vTemp;
		ProjInfo.nTrackerUserTypeBitsMask = FCOLL_USER_TYPE_BITS_ALL;
		ProjInfo.pCallback = NULL;
		
		fcoll_Clear();
		fcoll_Proj( &ProjInfo, _pModelMesh, 41 );
		
		#if FANG_PLATFORM_WIN
			ftext_DebugPrintf( .1f, .115f, "~al~s2.00%d", (u32)FMesh_Coll_nDOPTransforms );
		#else
			ftext_DebugPrintf( 0.07f, 0.07f, "~al~s1.00%d", (u32)FMesh_Coll_nDOPTransforms );
		#endif
		FMesh_Coll_nDOPTransforms = 0;
//		_pModelMesh->kDOP_Draw();
	}
*/

	fvis_FrameBegin();
	fvis_Draw( NULL, FALSE );
	fvis_FrameEnd();
	
	if ( _pModelMesh )
	{
		_ModelTest_Draw();
	}

	frenderer_Pop( );

	fvid_End();

	if ( bExitRequest )
	{
		if ( _pModelMesh )
		{
			_pModelMesh->RemoveFromWorld();
			fdelete( _pModelMesh );
		}

		return FALSE;
	}
	
	return TRUE;
}


//
//
//
void LoopTerm( FLoopTermCode_t nTermCode, void *pParameter )
{
	fres_ReleaseFrame( _MainFrame );
}


//
//
//
static void _UpdatePadInput( void )
{
	if ( !fpad_IsInstalled() )
	{
		return;
	}

	// Update Camera movement
	fpad_UpdateSamples();

	f32 fSpeedAdjust = 20.f * FLoop_fPreviousLoopSecs;
	fSpeedAdjust *= paPadSamples[5]->fCurrentState + 1;
	fSpeedAdjust *= 1 - (paPadSamples[4]->fCurrentState * 0.75f);

	CFVec3 vDeltaPos;
	vDeltaPos.x = fSpeedAdjust * paPadSamples[0]->fCurrentState;
	vDeltaPos.z = fSpeedAdjust * paPadSamples[1]->fCurrentState;
	vDeltaPos.y = fSpeedAdjust * paPadSamples[3]->fCurrentState;

	UpdateCamera( FMATH_DEG2RAD(paPadSamples[2]->fCurrentState * fSpeedAdjust * 5.f), 0.f, &vDeltaPos );
}


//
//
//
static void _ModelTest_Init( void )
{
	FMesh_t *pMesh;
	FAnim_t *pAnim;

	_pModelMesh = fnew CFWorldMesh;
	if ( !_pModelMesh )
	{
		return;
	}

	// BLINK
//	pMesh = (FMesh_t *)fresload_Load( FMESH_RESTYPE, "grdmminer00" );
	pMesh = (FMesh_t *)fresload_Load( FMESH_RESTYPE, "godmshrd3_4" );
	FASSERT( pMesh );
	_pModelMesh->Init( pMesh );
	_mtxModelF.Identity();
	_mtxModelF.m_vPos = _vPos;//-261.4f;
//	_mtxModelF.m_vPos.y = 1.0f;
//	_mtxModelF.m_vPos.z = 106.98f;
	_pModelMesh->m_Xfm.BuildFromMtx( _mtxModelF, TRUE );
	_pModelMesh->UpdateTracker();
	_pModelMesh->SetUserTypeBits( 1 );

	pAnim = (FAnim_t *)fresload_Load( FANIM_RESNAME, "ardmrun_f01" );
	FASSERT( pAnim );
	_AnimInst.Create( pAnim );
	_AnimCombiner.CreateSimple( &_AnimInst, _pModelMesh );

}


//
//
//
static void _ModelTest_Draw( void )
{
	static CFVec3A vTemp, vTemp1;
	static CFVec3A vGrav( 0.f, -32.f, 0.f );
	static f32 fCurrRot = 90.f;
	
	f32 fElapsedTime = FLoop_fPreviousLoopSecs * 0.05f;

	_AnimInst.DeltaTime( fElapsedTime );

	// RRRRRotate
//	fCurrRot += 60.f * fElapsedTime;

	if ( !_bCamAttached )
	{
		CFVec3 vMovement;
		vMovement.x = _mtxModelF.m_vFront.x;
		vMovement.y = _mtxModelF.m_vFront.y;
		vMovement.z = _mtxModelF.m_vFront.z;
		vMovement *= fElapsedTime * 14.f;
		
		_vObjVel.Add( vTemp.Mul( vGrav, fElapsedTime ) );

		vTemp1.Zero();
		vTemp.Mul( _vObjVel, FLoop_fPreviousLoopSecs );
		CFCollData ProjInfo;
		ProjInfo.nFlags = FCOLL_DATA_FLAGS_NONE;// | FCOLL_DATA_USE_OBJECT_CAPSULE;// | FCOLL_DATA_IGNORE_BACKSIDE;
		ProjInfo.nCollMask = FCOLL_MASK_CHECK_ALL;
		ProjInfo.pMovement = &vTemp;
	//	ProjInfo.pMovement = &vTemp1;
		ProjInfo.nTrackerUserTypeBitsMask = FCOLL_USER_TYPE_BITS_ALL;
		ProjInfo.pCallback = NULL;

		u32 ii;
		for ( ii = 0; ii < 1; ii++ )
		{
			fcoll_Clear();
			fcoll_Check( &ProjInfo, _pModelMesh );

			if ( FColl_nImpactCount )
			{
				_bCollision = TRUE;
				FCollImpact_t *pImpact = &FColl_aImpactBuf[0];
				for ( u32 i = 1; i < FColl_nImpactCount; i++ )
				{
					if ( FColl_aImpactBuf[i].fUnitImpactTime < pImpact->fUnitImpactTime )
					{
						pImpact = &FColl_aImpactBuf[i];
					}
					else if ( pImpact->fUnitImpactTime == 0.f && FColl_aImpactBuf[i].fUnitImpactTime == 0.f 
								&& pImpact->fImpactDistInfo < FColl_aImpactBuf[i].fImpactDistInfo )
					{
						pImpact = &FColl_aImpactBuf[i];
					}
				}
				// See what side of the vector the impact is
				if ( vTemp.Dot( pImpact->PushUnitVec ) < 0.f )
				{
					// We're headed towards the poly, so reflect
					_vObjVel.Reflect( pImpact->UnitFaceNormal );
			//		_vObjVel.Reflect( pImpact->PushUnitVec );
				}
				_vObjVel.Mul( 0.8f );
				vTemp.Add( pImpact->PushUnitVec.Mul( pImpact->fImpactDistInfo ));
			}
		}

		vTemp.v3 += _mtxModelF.m_vPos;
	}
	else
	{
		vTemp = _mtxModelF.m_vPos;
	}

	_mtxModelF.m_vPos.Zero();
//	_mtxModelF.RotateY( FMATH_DEG2RAD(45 * fElapsedTime) );
//	_mtxModelF.RotateX( FMATH_DEG2RAD(67 * fElapsedTime) );
//	_mtxModelF.RotateX( FMATH_DEG2RAD(90) );
	_mtxModelF.SetRotationZ( FMATH_DEG2RAD(fCurrRot) );
	_mtxModelF.m_vPos = vTemp.v3;
	_pModelMesh->m_Xfm.BuildFromMtx( _mtxModelF, TRUE );
//	_AnimCombiner.ComputeMtxPalette();

	ftext_DebugPrintf( .1f, .3f, "~al~s2.00%f", _mtxModelF.m_vPos.y );
	
//	fmesh_Ambient_Set( 0.25f, 0.25f, 0.25f, 1.f );
	_pModelMesh->UpdateTracker();
//	_pModelMesh->Draw( FVIEWPORT_PLANESMASK_NONE, TRUE );
}


