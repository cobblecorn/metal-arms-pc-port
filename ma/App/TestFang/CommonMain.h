////////////////////////////////////////////////////////////////////////////////////////////
//
//	COMMONMAIN.H
//
//		Common routines for FANG test app
//
////////////////////////////////////////////////////////////////////////////////////////////



#ifndef __COMMONMAIN_H__
#define __COMMONMAIN_H__

// Game Loop Functions
BOOL LoopInit( void *pParameter );
BOOL LoopMain( BOOL bExitRequest, void *pParameter );
void LoopTerm( FLoopTermCode_t nTermCode, void *pParameter );

void UpdateCamera( f32 fYRotation, f32 fXRotation, CFVec3 *pTranslation );

extern FViewport_t *g_pViewport;


typedef enum
{
	INPUT_TRIGGER_1,
	INPUT_TRIGGER_2,
} TestInput_e;

void SubmitInput( TestInput_e nInput );



#endif // __COMMONMAIN_H__

