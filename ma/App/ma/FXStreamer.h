//////////////////////////////////////////////////////////////////////////////////////
// FXStreamer.h - 
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
// 03/31/02 Starich     Created.
//////////////////////////////////////////////////////////////////////////////////////
#ifndef _FX_STREAMER_H_
#define _FX_STREAMER_H_ 1

#include "fang.h"
#include "fworld.h"
#include "ftex.h"


#define FXSTREAMER_NUM_STREAMER_EMITTERS	25

typedef u32 FXStreamerHandle_t;
#define FXSTREAMER_INVALID_HANDLE			0

typedef struct _StreamerPt_s _StreamerPt_t;

FCLASS_ALIGN_PREFIX class CFXStreamerEmitter
{
public:
	static BOOL InitSystem();
	static void UninitSystem();

	typedef enum {
		USE_RIGHT_AXIS = 0,
		USE_UP_AXIS,
		USE_FRONT_AXIS
	} UseAxis_e;

	enum {
		FLAGS_1ST_POINT_STILL_AROUND = 0x01,

		FLAGS_NONE = 0x00
	};
	
	// a single streamer handle can emit upto _MAX_STREAMS streams form _MAX_STREAMS bones, all sharing the same settings
	static FXStreamerHandle_t SpawnFromMeshBones( u32 nMaxStreamDepth,
												  const CFWorldMesh *pWorldMesh,
												  cchar **papszBoneNames,
												  f32 fInitialAlpha,
												  CFTexInst *pTexture,
												  f32 fWidth_WS=1.0f,
												  f32 fSamplesPerSec=15.0f,
												  UseAxis_e nAxis=USE_RIGHT_AXIS );
	static FXStreamerHandle_t SpawnFromMtx43A( u32 nMaxStreamDepth,
											   const CFMtx43A *pMtx43A,
											   f32 fInitialAlpha,
											   CFTexInst *pTexture,
											   const CFVec3 *pSpawnPt_MS=NULL,
											   f32 fWidth_WS=1.0f,
											   f32 fSamplesPerSec=15.0f,
											   UseAxis_e nAxis=USE_RIGHT_AXIS );
	static void EnableStreamerEmission( FXStreamerHandle_t hHandle, BOOL bEnable );
	static void KillAllStreamers( void );
	static BOOL IsValidHandle( FXStreamerHandle_t hHandle );
	static void ReturnEmitterHandle( FXStreamerHandle_t hHandle );
	static void EmitPoint( FXStreamerHandle_t hHandle );
	static void Work();
	static void Draw();
	static void NearingEnd( FXStreamerHandle_t hHandle, f32 fUnitPercentTillEnd );
	

	// called from the static functions (safe since only this system has a CFXStreamEmitter *pPtr)
	FXStreamerHandle_t GetMyHandle()		{ return m_hMyHandle; }
    BOOL SpawnStreams( u32 nMaxStreamDepth,
					   const CFWorldMesh *pWorldMesh,
					   cchar **papszBoneNames,
					   f32 fInitialAlpha,
					   CFTexInst *pTexture,
					   f32 fWidth_WS,
					   f32 fSamplesPerSec,
					   UseAxis_e nAxis );
	BOOL SpawnStreams( u32 nMaxStreamDepth,
					   const CFMtx43A *pMtx43A,
					   f32 fInitialAlpha,
					   CFTexInst *pTexture,
					   const CFVec3 *pSpawnPt_MS,
					   f32 fWidth_WS,
					   f32 fSamplesPerSec,
					   UseAxis_e nAxis );
	void KillAll();
	void WorkEmitter();
	void DrawEmitter();
	void CreateMyHandle( u32 nIndex );
	
	BOOL8 m_bEmitStreamer;		// should we emit pts right now
	BOOL8 m_bNearingEnd;
	BOOL8 m_bDrawLastRemoved;
	BOOL8 m_bEmitFromBones;		// Should we emit from the mesh's bones
	f32 m_fSecsTillNextSample;
	f32 m_fPercentTillEnd;
	f32 m_fOOSecsToFadeInitialOn;	// 1/secs the streamers should initially fade on from 0 alpha
	f32 m_fInitialFadeOnFactor;		// multiple this by the initial alpha value, will become 1.0f real fast
	FXStreamerHandle_t m_hMyHandle;	// if FXSTREAMER_INVALID_HANDLE then this emitter is available

private:		

#define _MAX_STREAMS			4
#define _MIN_STREAMER_ALPHA		0.0f

	u16 m_nNumPts;					// the total number of pts allocated
	u16 m_nPtsPerStream;			// how many pts are there per stream
	u16 m_nOldestIndex;				// the index of the oldest pt added (to the 1st stream)
	u16 m_nUsedCount;				// how many pts are currently used (in the 1st stream)
	u16 m_nNumStreams;				// how many independant streams are there? No more than _MAX_STREAMS allowed
	u8 m_nAxis;						// UseAxis_... what axis should we use when getting the dir from the mtx
	u8 m_nFlags;					// FLAGS_...
	_StreamerPt_t *m_paPts;			// the allocated pts array
	f32 m_fInitialAlpha;			// what should the initial alpha be for each point?	
	CFTexInst *m_pTexInst;			// what texture should we use when drawing this streamer?
	f32 m_fHalfWidth_WS;			// half of the width in WS
	const CFWorldMesh *m_pWorldMesh;// what world mesh are we tracking (if we are tracking a mesh at all)
	const CFMtx43A *m_pOrientation;	// the orientation that we are tracking
	f32 m_fOOSamplesPerSec;
	f32 m_fFadePerSec;				

	// we save 1 extra pair of points so that we can smoothly animate the fade off
	CFVec3 m_aLastRemovedPosWS[_MAX_STREAMS];
	f32 m_afLastRemovedAlpha[_MAX_STREAMS];
	CFVec3 m_aLastRemovedUnitDir[_MAX_STREAMS];
	
	// if we are not emitting from bones, then emit from this model space offset
	CFVec3 m_Offset_MS;			
	u8 m_anBoneIndices[_MAX_STREAMS];

	BOOL InitCommonFields( u32 nMaxStreamDepth,
						   const CFWorldMesh *pWorldMesh,
						   f32 fInitialAlpha, 
						   CFTexInst *pTexture,
						   f32 fWidth_WS,
						   f32 fSamplesPerSec,
						   UseAxis_e nAxis );
	
protected:
	CFXStreamerEmitter();

private:
	void _EmitPoint( void );


	FCLASS_STACKMEM_ALIGN( CFXStreamerEmitter );
} FCLASS_ALIGN_SUFFIX;







#endif

