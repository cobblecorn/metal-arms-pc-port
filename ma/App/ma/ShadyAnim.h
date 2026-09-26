//////////////////////////////////////////////////////////////////////////////////////
// ShadyAnim.h - 
//
// Author: Pat MacKellar 
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
// 10/23/02 MacKellar   Created.
//////////////////////////////////////////////////////////////////////////////////////
#ifndef _SHADYANIM_H_
#define _SHADYANIM_H_ 1

#include "fang.h"


class CFAnimCombinerConfig;
class CFAnimCombiner;
class CFWorldMesh;

FCLASS_NOALIGN_PREFIX class CShadyAnim {
public:
	static BOOL InitSystem( void );							// Call just once at game init (not once per level)
	static void UninitSystem( void );						// Call when the game is being shut down

	CShadyAnim();
	~CShadyAnim();

	BOOL Create( CFWorldMesh *pMeshToDrive );				// Call to create a shady animation combiner (returns FALSE upon error)
	void Destroy( void );									// Call to destroy a shady animation combiner

	FINLINE CFAnimCombiner *GetCombiner( void ) { return m_pAnimCombiner; }


	// Tap IDs...
	s32 m_nHighTapID;										// For the high tap
	s32 m_anIdleTapID[2];									// For the two idle taps: [0]=Idle0, [1]=Idle1
	s32 m_anBTLowTapID[4];									// For the four BotTalkLow taps
	s32 m_anBTHighTapID[4];									// For the four BotTalkHigh taps
	s32 m_nBTLowIdle;										// For the BotTalkLow idle tap
	s32 m_nBTHighIdle;										// For the BotTalkHigh idle tap

	// Control IDs...
	s32 m_nHighControlID;									// For the high control
	s32 m_nIdleControlID;									// For the idle control
	s32 m_nBTLowControlID;									// For the master BotTalkLow control
	s32 m_nBTHighControlID;									// For the master BotTalkHigh control
	s32 m_anBTLowControlID[4];								// For the four BotTalkLow controls
	s32 m_anBTHighControlID[4];								// For the four BotTalkHigh controls


private:
	static CFAnimCombinerConfig *m_pAnimCombinerConfig;		// Our animation combiner config - One for the entire game
	CFAnimCombiner *m_pAnimCombiner;						// Our animation combiner - One per Shady instance


	FCLASS_STACKMEM_NOALIGN( CShadyAnim );
} FCLASS_NOALIGN_SUFFIX;

#endif
