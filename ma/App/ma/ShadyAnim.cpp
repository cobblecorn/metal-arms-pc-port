//////////////////////////////////////////////////////////////////////////////////////
// ShadyAnim.cpp - 
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
// 10/23/02 Sranck   Created.

//////////////////////////////////////////////////////////////////////////////////////
#include "fang.h"
#include "ShadyAnim.h"



#include "fanim.h"


CFAnimCombinerConfig *CShadyAnim::m_pAnimCombinerConfig;


BOOL CShadyAnim::InitSystem( void ) {
	FResFrame_t ResFrame = fres_GetFrame();

	m_pAnimCombinerConfig = fnew CFAnimCombinerConfig;
	if( m_pAnimCombinerConfig == NULL ) {
		DEVPRINTF( "CShadyAnim::InitSystem(): Not enough memory to allocate m_pAnimCombinerConfig.\n" );
		goto _ExitWithError;
	}

	// Create mixers...
	if( !m_pAnimCombinerConfig->BeginCreation( "High", CFAnimMixer::TYPE_BLENDER ) ) {
		DEVPRINTF( "CShadyAnim::InitSystem(): Could not begin creation of m_pAnimCombinerConfig.\n" );
		goto _ExitWithError;
	}

	if( !m_pAnimCombinerConfig->AddBlender( "BTH", "High", 0 ) ) {
		DEVPRINTF( "CShadyAnim::InitSystem(): Could not create m_pAnimCombinerConfig.\n" );
		goto _ExitWithError;
	}

	if( !m_pAnimCombinerConfig->AddBlender( "BTL", "BTH", 0 ) ) {
		DEVPRINTF( "CShadyAnim::InitSystem(): Could not create m_pAnimCombinerConfig.\n" );
		goto _ExitWithError;
	}

	if( !m_pAnimCombinerConfig->AddBlender( "Idle", "BTL", 0 ) ) {
		DEVPRINTF( "CShadyAnim::InitSystem(): Could not create m_pAnimCombinerConfig.\n" );
		goto _ExitWithError;
	}

	if( !m_pAnimCombinerConfig->AddBlender( "BTH0", "BTH", 1 ) ) {
		DEVPRINTF( "CShadyAnim::InitSystem(): Could not create m_pAnimCombinerConfig.\n" );
		goto _ExitWithError;
	}

	if( !m_pAnimCombinerConfig->AddBlender( "BTH1", "BTH0", 0 ) ) {
		DEVPRINTF( "CShadyAnim::InitSystem(): Could not create m_pAnimCombinerConfig.\n" );
		goto _ExitWithError;
	}

	if( !m_pAnimCombinerConfig->AddBlender( "BTH2", "BTH1", 0 ) ) {
		DEVPRINTF( "CShadyAnim::InitSystem(): Could not create m_pAnimCombinerConfig.\n" );
		goto _ExitWithError;
	}

	if( !m_pAnimCombinerConfig->AddBlender( "BTH3", "BTH2", 0 ) ) {
		DEVPRINTF( "CShadyAnim::InitSystem(): Could not create m_pAnimCombinerConfig.\n" );
		goto _ExitWithError;
	}

	if( !m_pAnimCombinerConfig->AddBlender( "BTL0", "BTL", 1 ) ) {
		DEVPRINTF( "CShadyAnim::InitSystem(): Could not create m_pAnimCombinerConfig.\n" );
		goto _ExitWithError;
	}

	if( !m_pAnimCombinerConfig->AddBlender( "BTL1", "BTL0", 0 ) ) {
		DEVPRINTF( "CShadyAnim::InitSystem(): Could not create m_pAnimCombinerConfig.\n" );
		goto _ExitWithError;
	}

	if( !m_pAnimCombinerConfig->AddBlender( "BTL2", "BTL1", 0 ) ) {
		DEVPRINTF( "CShadyAnim::InitSystem(): Could not create m_pAnimCombinerConfig.\n" );
		goto _ExitWithError;
	}

	if( !m_pAnimCombinerConfig->AddBlender( "BTL3", "BTL2", 0 ) ) {
		DEVPRINTF( "CShadyAnim::InitSystem(): Could not create m_pAnimCombinerConfig.\n" );
		goto _ExitWithError;
	}

	// Create taps...
	if( !m_pAnimCombinerConfig->AddTap( "High", "High", 1 ) ) {
		DEVPRINTF( "CShadyAnim::InitSystem(): Could not create m_pAnimCombinerConfig.\n" );
		goto _ExitWithError;
	}

	if( !m_pAnimCombinerConfig->AddTap( "Idle0", "Idle", 0 ) ) {
		DEVPRINTF( "CShadyAnim::InitSystem(): Could not create m_pAnimCombinerConfig.\n" );
		goto _ExitWithError;
	}

	if( !m_pAnimCombinerConfig->AddTap( "Idle1", "Idle", 1 ) ) {
		DEVPRINTF( "CShadyAnim::InitSystem(): Could not create m_pAnimCombinerConfig.\n" );
		goto _ExitWithError;
	}

	if( !m_pAnimCombinerConfig->AddTap( "BTH0", "BTH0", 1 ) ) {
		DEVPRINTF( "CShadyAnim::InitSystem(): Could not create m_pAnimCombinerConfig.\n" );
		goto _ExitWithError;
	}

	if( !m_pAnimCombinerConfig->AddTap( "BTH1", "BTH1", 1 ) ) {
		DEVPRINTF( "CShadyAnim::InitSystem(): Could not create m_pAnimCombinerConfig.\n" );
		goto _ExitWithError;
	}

	if( !m_pAnimCombinerConfig->AddTap( "BTH2", "BTH2", 1 ) ) {
		DEVPRINTF( "CShadyAnim::InitSystem(): Could not create m_pAnimCombinerConfig.\n" );
		goto _ExitWithError;
	}

	if( !m_pAnimCombinerConfig->AddTap( "BTH3", "BTH3", 1 ) ) {
		DEVPRINTF( "CShadyAnim::InitSystem(): Could not create m_pAnimCombinerConfig.\n" );
		goto _ExitWithError;
	}

	if( !m_pAnimCombinerConfig->AddTap( "BTL0", "BTL0", 1 ) ) {
		DEVPRINTF( "CShadyAnim::InitSystem(): Could not create m_pAnimCombinerConfig.\n" );
		goto _ExitWithError;
	}

	if( !m_pAnimCombinerConfig->AddTap( "BTL1", "BTL1", 1 ) ) {
		DEVPRINTF( "CShadyAnim::InitSystem(): Could not create m_pAnimCombinerConfig.\n" );
		goto _ExitWithError;
	}

	if( !m_pAnimCombinerConfig->AddTap( "BTL2", "BTL2", 1 ) ) {
		DEVPRINTF( "CShadyAnim::InitSystem(): Could not create m_pAnimCombinerConfig.\n" );
		goto _ExitWithError;
	}

	if( !m_pAnimCombinerConfig->AddTap( "BTL3", "BTL3", 1 ) ) {
		DEVPRINTF( "CShadyAnim::InitSystem(): Could not create m_pAnimCombinerConfig.\n" );
		goto _ExitWithError;
	}

	if( !m_pAnimCombinerConfig->AddTap( "IdleBTH", "BTH3", 0 ) ) {
		DEVPRINTF( "CShadyAnim::InitSystem(): Could not create m_pAnimCombinerConfig.\n" );
		goto _ExitWithError;
	}

	if( !m_pAnimCombinerConfig->AddTap( "IdleBTL", "BTL3", 0 ) ) {
		DEVPRINTF( "CShadyAnim::InitSystem(): Could not create m_pAnimCombinerConfig.\n" );
		goto _ExitWithError;
	}

	// Terminate the creation procedure...
	if( !m_pAnimCombinerConfig->EndCreation() ) {
		DEVPRINTF( "CShadyAnim::InitSystem(): Could not end creation of m_pAnimCombinerConfig.\n" );
		goto _ExitWithError;
	}

	// Anim combiner config created successfully!

	return TRUE;

	// Trouble creating anim combiner config...
_ExitWithError:
	UninitSystem();
	fres_ReleaseFrame( ResFrame );
	return FALSE;
}


void CShadyAnim::UninitSystem( void ) {
	fdelete( m_pAnimCombinerConfig );
	m_pAnimCombinerConfig = NULL;
}


CShadyAnim::CShadyAnim() {
	m_pAnimCombiner = NULL;
}


CShadyAnim::~CShadyAnim() {
	Destroy();
}


BOOL CShadyAnim::Create( CFWorldMesh *pMeshToDrive ) {
	FResFrame_t ResFrame = fres_GetFrame();

	// Create combiner...
	m_pAnimCombiner = fnew CFAnimCombiner;
	if( m_pAnimCombiner == NULL ) {
		DEVPRINTF( "CShadyAnim::Create(): Not enough memory to allocate m_pAnimCombiner.\n" );
		goto _ExitWithError;
	}

	if( !m_pAnimCombiner->Create( m_pAnimCombinerConfig, pMeshToDrive ) ) {
		DEVPRINTF( "CShadyAnim::Create(): Could not create m_pAnimCombiner.\n" );
		goto _ExitWithError;
	}

	// Get tap IDs...
	m_nHighTapID = m_pAnimCombiner->GetTapID( "High" );
	m_anIdleTapID[0] = m_pAnimCombiner->GetTapID( "Idle0" );
	m_anIdleTapID[1] = m_pAnimCombiner->GetTapID( "Idle1" );
	m_anBTLowTapID[0] = m_pAnimCombiner->GetTapID( "BTL0" );
	m_anBTLowTapID[1] = m_pAnimCombiner->GetTapID( "BTL1" );
	m_anBTLowTapID[2] = m_pAnimCombiner->GetTapID( "BTL2" );
	m_anBTLowTapID[3] = m_pAnimCombiner->GetTapID( "BTL3" );
	m_anBTHighTapID[0] = m_pAnimCombiner->GetTapID( "BTH0" );
	m_anBTHighTapID[1] = m_pAnimCombiner->GetTapID( "BTH1" );
	m_anBTHighTapID[2] = m_pAnimCombiner->GetTapID( "BTH2" );
	m_anBTHighTapID[3] = m_pAnimCombiner->GetTapID( "BTH3" );
	m_nBTLowIdle = m_pAnimCombiner->GetTapID( "IdleBTL" );
	m_nBTHighIdle = m_pAnimCombiner->GetTapID( "IdleBTH" );

	FASSERT( m_nHighTapID>=0 && m_anIdleTapID[0]>=0 && m_anIdleTapID[1]>=0 && m_nBTLowIdle>=0 && m_nBTHighIdle>=0 );
	FASSERT( m_anBTLowTapID[0]>=0 && m_anBTLowTapID[1]>=0 && m_anBTLowTapID[2]>=0 && m_anBTLowTapID[3]>=0 );
	FASSERT( m_anBTHighTapID[0]>=0 && m_anBTHighTapID[1]>=0 && m_anBTHighTapID[2]>=0 && m_anBTHighTapID[3]>=0 );

	// Get control IDs...
	m_nHighControlID = m_pAnimCombiner->GetControlID( "High" );
	m_nIdleControlID = m_pAnimCombiner->GetControlID( "Idle" );
	m_nBTLowControlID = m_pAnimCombiner->GetControlID( "BTL" );
	m_nBTHighControlID = m_pAnimCombiner->GetControlID( "BTH" );
	m_anBTLowControlID[0] = m_pAnimCombiner->GetControlID( "BTL0" );
	m_anBTLowControlID[1] = m_pAnimCombiner->GetControlID( "BTL1" );
	m_anBTLowControlID[2] = m_pAnimCombiner->GetControlID( "BTL2" );
	m_anBTLowControlID[3] = m_pAnimCombiner->GetControlID( "BTL3" );
	m_anBTHighControlID[0] = m_pAnimCombiner->GetControlID( "BTH0" );
	m_anBTHighControlID[1] = m_pAnimCombiner->GetControlID( "BTH1" );
	m_anBTHighControlID[2] = m_pAnimCombiner->GetControlID( "BTH2" );
	m_anBTHighControlID[3] = m_pAnimCombiner->GetControlID( "BTH3" );

	FASSERT( m_nHighControlID>=0 && m_nIdleControlID>=0 && m_nBTLowControlID>=0 && m_nBTHighControlID>=0 );
	FASSERT( m_anBTLowControlID[0]>=0 && m_anBTLowControlID[1]>=0 && m_anBTLowControlID[2]>=0 && m_anBTLowControlID[3]>=0 );
	FASSERT( m_anBTHighControlID[0]>=0 && m_anBTHighControlID[1]>=0 && m_anBTHighControlID[2]>=0 && m_anBTHighControlID[3]>=0 );

	// Anim combiner created successfully!

	return TRUE;

	// Trouble creating anim combiner...
_ExitWithError:
	Destroy();
	fres_ReleaseFrame( ResFrame );
	return FALSE;
}


void CShadyAnim::Destroy( void ) {
	fdelete( m_pAnimCombiner );
	m_pAnimCombiner = NULL;
}
