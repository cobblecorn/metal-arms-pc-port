//////////////////////////////////////////////////////////////////////////////////////
// botanim.h - General bot animation functionality.
//
// Author: Steve Ranck     
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
// 03/26/02 Ranck       Created.
//////////////////////////////////////////////////////////////////////////////////////

#ifndef _BOTANIM_H_
#define _BOTANIM_H_ 1

#include "fang.h"
#include "fanim.h"
#include "floop.h"


#define BOTANIM_USER_NAME_PREFIX			"user"
#define BOTANIM_IDLE_NAME_PREFIX			"idle"
#define BOTANIM_SUMMER_NAME					"summer"
#define BOTANIM_DUMMY_TAP_NAME				"dummytap"

#define BOTANIM_NULL_ANIMSOURCE_ATTACH		254



//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CBotAnimStackDef
//
// This is to exist as a static entry in the bot class.
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

class CBotAnimStackDef {
//----------------------------------------------------------------------------------------------------------------------------------
// Public Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	enum {
		ANIM_USER_COUNT = 4,
		USER_NAME_INDEX_CHAR_COUNT = 2,				// The number of digits appended to BOTANIM_USER_NAME_PREFIX to form the user names
		IDLE_NAME_INDEX_CHAR_COUNT = 2				// The number of digits appended to BOTANIM_IDLE_NAME_PREFIX to form the idle names
	};


	typedef struct {
		u32 nUserCount;								// The number of desired user animation entries in the stack

		u32 nBaseAnimNameCount;						// The number of entries in apszBaseAnimNameTable
		cchar **apszBaseAnimNameTable;				// Table of our base animation resource file names

		u32 nIdleCount;								// The number of entries in apszIdleAnimNameTable
		cchar **apszIdleAnimNameTable;				// Table of our idle animation resource file names

		u32 nBaseControlCount;						// The number of entries in apszBaseControlNameTable
		cchar **apszBaseControlNameTable;			// Table of our base control names

		u32 nBaseTapCount;							// The number of entries in apszBaseTapNameTable
		cchar **apszBaseTapNameTable;				// Table of our base tap names

		u32 nBoneCount;								// The number of entries in apszBoneNameTable
		cchar **apszBoneNameTable;					// Table of bone names

		// This is an array of pointers, where each
		// pointer points to an array of u8 indices.
		// There is one pointer for each base tap.
		// If the pointer is NULL, then all bones
		// are enabled for that tap. Otherwise, the
		// pointer points to an array of u8 indices,
		// where each index indexes into apszBoneNameTable.
		// The array of u8's must be terminated by a
		// value of 255. Each bone indexed will be
		// disabled for that tap.
		const u8 **ppnEnableBoneNameIndexTableForEachBaseTap;
		const u8 **ppnEnableBoneNameIndexTableForEachIdleTap;
		const u8 *pnEnableBoneNameIndexTableForSummer;

		const CFAnimCombinerConfig::ConfigNet_t *pBaseAnimConfigNet;		// The base animation configuration net (NULL=none)
		const CFAnimCombinerConfig::ConfigStack_t *pBaseAnimConfigStack;	// The base animation configuration stack (NULL=none)
		const CFAnimCombinerConfig::ConfigTap_t *pBaseAnimConfigTap;		// The base animation tap configuration
		const CFAnimCombiner::AttachList_t *pBaseAnimAttachList;			// The base animation attach list

		u32 nIdleStackConnectionControlIndex;		// The control index of where the idle stack is to be snapped onto
		u32 nIdleStackConnectionControlInput;		// The control's input (0 or 1) of where the idle stack is to be snapped onto
	} Init_t;


	// The name set is in this order:
	//   1) Base animations
	//   2) Summer
	//   3) User animations
	//   4) Idle animations
	//
	// Note that this doesn't describe the priority in the stack.
	typedef struct {
		cchar **apszNameTable;				// The complete name table
		u32 nBaseCount;						// The number of base entries in apszNameTable
		u32 nIdleCount;						// The number of idle entries in apszNameTable
		u32 nTotalCount;					// The total number of entries in apszNameTable
		u32 nSummerIndex;					// The summer's animation index into apszNameTable
		u32 nFirstUserIndex;				// The first user animation index into apszNameTable
		u32 nFirstIdleIndex;				// The first idle animation index into apszNameTable
	} NameSet_t;




//----------------------------------------------------------------------------------------------------------------------------------
// Public Data:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	// The stack is built as follows:
	//   1) Idles  (linear stack: lowest priority)
	//   2) Base   (implementation-specific linear stack or network)
	//   3) Users  (linear stack, with User 0 being highest priority)
	//   4) Summer (highest priority)
	CFAnimCombinerConfig *m_pCombinerConfig;	// The animation combiner config for this stack

	NameSet_t m_NameSetControls;				// Defines the name set for all of this stack's controls
	NameSet_t m_NameSetTaps;					// Defines the name set for all of this stack's taps

	cchar **m_apszBaseAnimNameTable;			// Table of our base animation resource file names (count = m_nBaseAnimNameCount)
	cchar **m_apszIdleAnimNameTable;			// Table of our idle animation resource file names (count = m_nIdleCount)
	cchar **m_apszBoneNameTable;				// Table of bone names

	u32 m_nBaseAnimNameCount;					// The number of entries in m_apszBaseAnimNameTable
	u32 m_nUserCount;							// The number of user animations in this stack
	u32 m_nIdleCount;							// The number of idle animations in this stack
	u32 m_nBoneCount;							// The number of entries in m_apszBoneNameTable

	const CFAnimCombiner::AttachList_t *m_pBaseAnimAttachList;	// The base animation attach list

	const u8 **m_ppnEnableBoneNameIndexTableForEachBaseTap;	// See comments in Init_t above
	const u8 **m_ppnEnableBoneNameIndexTableForEachIdleTap;
	const u8 *m_pnEnableBoneNameIndexTableForSummer;




//----------------------------------------------------------------------------------------------------------------------------------
// Public Functions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	FINLINE CBotAnimStackDef() { m_pCombinerConfig = NULL; }
	FINLINE ~CBotAnimStackDef() { Destroy(); }

	BOOL Create( const Init_t *pInit );
	void Destroy( void );
	FINLINE BOOL IsCreated( void ) const { return (m_pCombinerConfig != NULL); }




//----------------------------------------------------------------------------------------------------------------------------------
// Private Functions:
//----------------------------------------------------------------------------------------------------------------------------------
private:

	BOOL _BuildNameSet( NameSet_t *pAnimNameSet, u32 nBaseCount, u32 nUserCount, u32 nIdleCount, cchar **apszBaseAnimNameTable );
	u32 _InitAnimStringPrefix( char *pszString, u32 nStringLen, cchar *pszPrefix, u32 nIndexCharCount );
	void _StoreAnimStringPrefix( char *pszString, u32 nStringLen, cchar *pszPrefix, u32 nIndexCharCount, u32 nIndex );


	FCLASS_STACKMEM_NOALIGN( CBotAnimStackDef );
};




//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CBotAnimStack
//
// This is to exist as an entry in each bot object instantiated.
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

class CBotAnimStack {
//----------------------------------------------------------------------------------------------------------------------------------
// Public Data:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	const CBotAnimStackDef *m_pBotAnimStackDef;	// Pointer to the stack definition object

	CFAnimCombiner *m_pAnimCombiner;		// The animation combiner for this stack

	CFAnimInst *m_pLoadedBaseAnimInst;		// Array of all base CFAnimInst's we loaded
	CFAnimInst *m_pLoadedIdleAnimInst;		// Array of all idle CFAnimInst's we loaded

	CFAnimInst **m_ppBaseAnimInst;			// Array of all CFAnimInst's currently attached to the base taps of our combiner
	CFAnimInst **m_ppIdleAnimInst;			// Array of all CFAnimInst's currently attached to the idle taps of our combiner
	CFAnimManMtx *m_pAnimManMtx;			// Manually-driven matrix animation source (for summer)

	u8 *m_pnAnimTapID;						// Array of our animation tap IDs (number of entries is CBotAnimStackDef::m_NameSetTaps::nTotalCount)
	u8 *m_pnAnimControlID;					// Array of our animation control IDs (number of entries is CBotAnimStackDef::m_NameSetControls::nTotalCount)
	u8 *m_pnBonePaletteIndex;				// Index into this with an index from CBotAnimStackDef::m_apszBoneNameTable, and it will produce
											// an index into the mesh's bone matrix palette.




//----------------------------------------------------------------------------------------------------------------------------------
// Public Functions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	FINLINE CBotAnimStack() { m_pLoadedBaseAnimInst=NULL; m_pLoadedIdleAnimInst=NULL; m_pAnimManMtx=NULL; m_pAnimCombiner=NULL; }

	BOOL Create( const CBotAnimStackDef *pAnimStackDef, CFWorldMesh *pMesh );
	void Destroy( void );
	void Reset( void );
	FINLINE BOOL IsCreated( void ) const { return (m_pAnimCombiner != NULL); }


	// ManMtx summer:
	FINLINE void ManMtxSummer_Identity( void );


	// Base animations:
	FINLINE void BaseAnim_Attach( u32 nBaseTapIndex, CFAnimSource *pAnimSource );
	FINLINE CFAnimSource *BaseAnim_GetAttached( u32 nBaseTapIndex ) const;

	FINLINE void BaseAnim_SetControlValue( u32 nBaseControlIndex, f32 fUnitControlValue );
	FINLINE f32 BaseAnim_GetControlValue( u32 nBaseControlIndex ) const;
	FINLINE BOOL BaseAnim_IsControlSmoothingEnabled( u32 nBaseControlIndex ) const;
	FINLINE void BaseAnim_EnableControlSmoothing( u32 nBaseControlIndex );
	FINLINE void BaseAnim_DisableControlSmoothing( u32 nBaseControlIndex );

	FINLINE void BaseAnim_EnableAllBones( u32 nBaseTapIndex );
	FINLINE void BaseAnim_DisableAllBones( u32 nBaseTapIndex );
	FINLINE void BaseAnim_UpdateBoneMask( u32 nBaseTapIndex, cchar *pszBoneName, BOOL bDriveBone );
	FINLINE void BaseAnim_UpdateBoneMask( u32 nBaseTapIndex, const u8 *pnBoneNameIndexTable, BOOL bDriveBonesInList );
	FINLINE BOOL BaseAnim_IsDrivingBone( u32 nBaseTapIndex, cchar *pszBoneName ) const;

	FINLINE f32 BaseAnim_GetTotalTime( u32 nBaseTapIndex ) const;
	FINLINE f32 BaseAnim_GetOOTotalTime( u32 nBaseTapIndex ) const;

	FINLINE void BaseAnim_ZeroTime( u32 nBaseTapIndex );

	FINLINE f32 BaseAnim_GetTime( u32 nBaseTapIndex ) const;
	FINLINE BOOL BaseAnim_DeltaTime( u32 nBaseTapIndex, f32 fDeltaTime=FLoop_fPreviousLoopSecs, BOOL bClamp=FALSE );
	FINLINE void BaseAnim_UpdateTime( u32 nBaseTapIndex, f32 fNewTime );

	FINLINE f32 BaseAnim_GetUnitTime( u32 nBaseTapIndex ) const;
	FINLINE BOOL BaseAnim_DeltaUnitTime( u32 nBaseTapIndex, f32 fDeltaTime=FLoop_fPreviousLoopSecs, BOOL bClamp=FALSE );
	FINLINE void BaseAnim_UpdateUnitTime( u32 nBaseTapIndex, f32 fNewTime );


	// User animations:
	FINLINE u32 UserAnim_GetCount( void ) const;

	FINLINE void UserAnim_Attach( u32 nUserIndex, CFAnimSource *pAnimSource );
	FINLINE CFAnimSource *UserAnim_GetAttached( u32 nUserIndex ) const;

	FINLINE void UserAnim_SetControlValue( u32 nUserIndex, f32 fUnitControlValue );
	FINLINE f32 UserAnim_GetControlValue( u32 nUserIndex ) const;

	FINLINE void UserAnim_EnableAllBones( u32 nUserIndex );
	FINLINE void UserAnim_DisableAllBones( u32 nUserIndex );
	FINLINE void UserAnim_UpdateBoneMask( u32 nUserIndex, cchar *pszBoneName, BOOL bDriveBone );
	FINLINE BOOL UserAnim_IsDrivingBone( u32 nUserIndex, cchar *pszBoneName ) const;

	FINLINE void UserAnim_BatchUpdateTapBoneMask_Open( u32 nUserIndex, BOOL bDriveBonesInList );
	FINLINE void UserAnim_BatchUpdateTapBoneMask_Close( void );
	FINLINE void UserAnim_BatchUpdateTapBoneMask( const u8 *pnBoneNameIndexTable, cchar **ppszBoneNameTable );

	void UserAnim_DetachAll( void );
	void UserAnim_ResetAllControlValues( void );


	// Built-in summer:
	FINLINE void Summer_SetControlValue( f32 fUnitControlValue );
	FINLINE f32 Summer_GetControlValue( void ) const;
	FINLINE BOOL Summer_IsControlSmoothingEnabled( void ) const;
	FINLINE void Summer_EnableControlSmoothing( void );
	FINLINE void Summer_DisableControlSmoothing( void );
	FINLINE void Summer_EnableAllBones( void );
	FINLINE void Summer_DisableAllBones( void );
	FINLINE void Summer_UpdateBoneMask( cchar *pszBoneName, BOOL bDriveBone );
	FINLINE void Summer_UpdateBoneMask( const u8 *pnBoneNameIndexTable, BOOL bDriveBonesInList );
	FINLINE BOOL Summer_IsDrivingBone( cchar *pszBoneName ) const;

	// Idle animations:
	FINLINE u32		IdleAnim_GetCount( void ) const;

	FINLINE void	IdleAnim_Attach( u32 nUserIndex, CFAnimSource *pAnimSource );
	FINLINE CFAnimSource *IdleAnim_GetAttached( u32 nUserIndex ) const;

	FINLINE void	IdleAnim_SetControlValue( u32 nUserIndex, f32 fUnitControlValue );
	FINLINE f32		IdleAnim_GetControlValue( u32 nUserIndex ) const;
	FINLINE BOOL	IdleAnim_IsControlSmoothingEnabled( u32 nBaseControlIndex ) const;
	FINLINE void	IdleAnim_EnableControlSmoothing( u32 nBaseControlIndex );
	FINLINE void	IdleAnim_DisableControlSmoothing( u32 nBaseControlIndex );


	FINLINE void	IdleAnim_EnableAllBones( u32 nUserIndex );
	FINLINE void	IdleAnim_DisableAllBones( u32 nUserIndex );
	FINLINE void	IdleAnim_UpdateBoneMask( u32 nUserIndex, cchar *pszBoneName, BOOL bDriveBone );
	FINLINE BOOL	IdleAnim_IsDrivingBone( u32 nUserIndex, cchar *pszBoneName ) const;

	FINLINE f32		IdleAnim_GetTotalTime( u32 nBaseTapIndex ) const;
	FINLINE f32		IdleAnim_GetOOTotalTime( u32 nBaseTapIndex ) const;

	FINLINE void	IdleAnim_ZeroTime( u32 nBaseTapIndex );

	FINLINE f32		IdleAnim_GetTime( u32 nBaseTapIndex ) const;
	FINLINE BOOL	IdleAnim_DeltaTime( u32 nBaseTapIndex, f32 fDeltaTime=FLoop_fPreviousLoopSecs, BOOL bClamp=FALSE );
	FINLINE void	IdleAnim_UpdateTime( u32 nBaseTapIndex, f32 fNewTime );

	FINLINE f32		IdleAnim_GetUnitTime( u32 nBaseTapIndex ) const;
	FINLINE BOOL	IdleAnim_DeltaUnitTime( u32 nBaseTapIndex, f32 fDeltaTime=FLoop_fPreviousLoopSecs, BOOL bClamp=FALSE );
	FINLINE void	IdleAnim_UpdateUnitTime( u32 nBaseTapIndex, f32 fNewTime );

	FINLINE void	IdleAnim_BatchUpdateTapBoneMask_Open( u32 nUserIndex, BOOL bDriveBonesInList );
	FINLINE void	IdleAnim_BatchUpdateTapBoneMask_Close( void );
	FINLINE void	IdleAnim_BatchUpdateTapBoneMask( const u8 *pnBoneNameIndexTable, cchar **ppszBoneNameTable );

	void IdleAnim_DetachAll( void );
	void IdleAnim_ResetAllControlValues( void );



//----------------------------------------------------------------------------------------------------------------------------------
// Private Functions:
//----------------------------------------------------------------------------------------------------------------------------------
private:

	BOOL _LoadAnims( CFAnimInst **ppAnimInstArray, u32 nAnimNameCount, cchar **pszAnimNameTable );


	FCLASS_STACKMEM_NOALIGN( CBotAnimStack );
};


// ManMtx summer:
FINLINE void CBotAnimStack::ManMtxSummer_Identity( void ) {
	u32 i;

	FASSERT( IsCreated() );

	for( i=0; i<m_pAnimManMtx->GetBoneCount(); ++i ) {
		m_pAnimManMtx->GetFrameAndFlagAsChanged( i )->Identity();
	}
}


// Base animations:
FINLINE void CBotAnimStack::BaseAnim_Attach( u32 nBaseTapIndex, CFAnimSource *pAnimSource ) {
	FASSERT( IsCreated() );
	FASSERT( nBaseTapIndex < m_pBotAnimStackDef->m_NameSetTaps.nBaseCount );
	m_pAnimCombiner->AttachToTap( m_pnAnimTapID[nBaseTapIndex], pAnimSource );
}


FINLINE CFAnimSource *CBotAnimStack::BaseAnim_GetAttached( u32 nBaseTapIndex ) const {
	FASSERT( IsCreated() );
	FASSERT( nBaseTapIndex < m_pBotAnimStackDef->m_NameSetTaps.nBaseCount );
	return m_pAnimCombiner->GetSourceDrivingTap( m_pnAnimTapID[nBaseTapIndex] );
}


FINLINE void CBotAnimStack::BaseAnim_SetControlValue( u32 nBaseControlIndex, f32 fUnitControlValue ) {
	FASSERT( IsCreated() );
	FASSERT( nBaseControlIndex < m_pBotAnimStackDef->m_NameSetControls.nBaseCount );
	m_pAnimCombiner->SetControlValue( m_pnAnimControlID[nBaseControlIndex], fUnitControlValue );
}


FINLINE f32 CBotAnimStack::BaseAnim_GetControlValue( u32 nBaseControlIndex ) const {
	FASSERT( IsCreated() );
	FASSERT( nBaseControlIndex < m_pBotAnimStackDef->m_NameSetControls.nBaseCount );
	return m_pAnimCombiner->GetControlValue( m_pnAnimControlID[nBaseControlIndex] );
}


FINLINE BOOL CBotAnimStack::BaseAnim_IsControlSmoothingEnabled( u32 nBaseControlIndex ) const {
	FASSERT( IsCreated() );
	FASSERT( nBaseControlIndex < m_pBotAnimStackDef->m_NameSetControls.nBaseCount );
	return m_pAnimCombiner->IsControlSmoothingEnabled( m_pnAnimControlID[nBaseControlIndex] );
}


FINLINE void CBotAnimStack::BaseAnim_EnableControlSmoothing( u32 nBaseControlIndex ) {
	FASSERT( IsCreated() );
	FASSERT( nBaseControlIndex < m_pBotAnimStackDef->m_NameSetControls.nBaseCount );
	m_pAnimCombiner->EnableControlSmoothing( m_pnAnimControlID[nBaseControlIndex] );
}


FINLINE void CBotAnimStack::BaseAnim_DisableControlSmoothing( u32 nBaseControlIndex ) {
	FASSERT( IsCreated() );
	FASSERT( nBaseControlIndex < m_pBotAnimStackDef->m_NameSetControls.nBaseCount );
	m_pAnimCombiner->DisableControlSmoothing( m_pnAnimControlID[nBaseControlIndex] );
}


FINLINE void CBotAnimStack::BaseAnim_EnableAllBones( u32 nBaseTapIndex ) {
	FASSERT( IsCreated() );
	FASSERT( nBaseTapIndex < m_pBotAnimStackDef->m_NameSetTaps.nBaseCount );
	m_pAnimCombiner->Mask_EnableAllBones( m_pnAnimTapID[nBaseTapIndex] );
}


FINLINE void CBotAnimStack::BaseAnim_DisableAllBones( u32 nBaseTapIndex ) {
	FASSERT( IsCreated() );
	FASSERT( nBaseTapIndex < m_pBotAnimStackDef->m_NameSetTaps.nBaseCount );
	m_pAnimCombiner->Mask_DisableAllBones( m_pnAnimTapID[nBaseTapIndex] );
}


FINLINE void CBotAnimStack::BaseAnim_UpdateBoneMask( u32 nBaseTapIndex, cchar *pszBoneName, BOOL bDriveBone ) {
	FASSERT( IsCreated() );
	FASSERT( nBaseTapIndex < m_pBotAnimStackDef->m_NameSetTaps.nBaseCount );
	m_pAnimCombiner->Mask_UpdateTapBoneMask( m_pnAnimTapID[nBaseTapIndex], pszBoneName, bDriveBone );
}


FINLINE void CBotAnimStack::BaseAnim_UpdateBoneMask( u32 nBaseTapIndex, const u8 *pnBoneNameIndexTable, BOOL bDriveBonesInList ) {
	FASSERT( IsCreated() );
	FASSERT( nBaseTapIndex < m_pBotAnimStackDef->m_NameSetTaps.nBaseCount );
	m_pAnimCombiner->Mask_UpdateTapBoneMask( m_pnAnimTapID[nBaseTapIndex], pnBoneNameIndexTable, m_pBotAnimStackDef->m_apszBoneNameTable, bDriveBonesInList );
}


FINLINE BOOL CBotAnimStack::BaseAnim_IsDrivingBone( u32 nBaseTapIndex, cchar *pszBoneName ) const {
	FASSERT( IsCreated() );
	FASSERT( nBaseTapIndex < m_pBotAnimStackDef->m_NameSetTaps.nBaseCount );
	return m_pAnimCombiner->Mask_IsDrivingBone( m_pnAnimTapID[nBaseTapIndex], pszBoneName );
}


FINLINE f32 CBotAnimStack::BaseAnim_GetTotalTime( u32 nBaseTapIndex ) const {
	FASSERT( IsCreated() );
	FASSERT( nBaseTapIndex < m_pBotAnimStackDef->m_NameSetTaps.nBaseCount );
	CFAnimSource *pAnimSource = BaseAnim_GetAttached( nBaseTapIndex );
	FASSERT( pAnimSource->GetAnimSourceType() == CFAnimSource::TYPE_INST );
	return ((CFAnimInst *)pAnimSource)->GetTotalTime();
}


FINLINE f32 CBotAnimStack::BaseAnim_GetOOTotalTime( u32 nBaseTapIndex ) const {
	FASSERT( IsCreated() );
	FASSERT( nBaseTapIndex < m_pBotAnimStackDef->m_NameSetTaps.nBaseCount );
	CFAnimSource *pAnimSource = BaseAnim_GetAttached( nBaseTapIndex );
	FASSERT( pAnimSource->GetAnimSourceType() == CFAnimSource::TYPE_INST );
	return ((CFAnimInst *)pAnimSource)->GetOOTotalTime();
}


FINLINE void CBotAnimStack::BaseAnim_ZeroTime( u32 nBaseTapIndex ) {
	FASSERT( IsCreated() );
	FASSERT( nBaseTapIndex < m_pBotAnimStackDef->m_NameSetTaps.nBaseCount );
	CFAnimSource *pAnimSource = BaseAnim_GetAttached( nBaseTapIndex );
	FASSERT( pAnimSource->GetAnimSourceType() == CFAnimSource::TYPE_INST );
	((CFAnimInst *)pAnimSource)->UpdateTime( 0.0f );
}


FINLINE f32 CBotAnimStack::BaseAnim_GetTime( u32 nBaseTapIndex ) const {
	FASSERT( IsCreated() );
	FASSERT( nBaseTapIndex < m_pBotAnimStackDef->m_NameSetTaps.nBaseCount );
	CFAnimSource *pAnimSource = BaseAnim_GetAttached( nBaseTapIndex );
	FASSERT( pAnimSource->GetAnimSourceType() == CFAnimSource::TYPE_INST );
	return ((CFAnimInst *)pAnimSource)->GetTime();
}


FINLINE BOOL CBotAnimStack::BaseAnim_DeltaTime( u32 nBaseTapIndex, f32 fDeltaTime, BOOL bClamp ) {
	FASSERT( IsCreated() );
	FASSERT( nBaseTapIndex < m_pBotAnimStackDef->m_NameSetTaps.nBaseCount );
	CFAnimSource *pAnimSource = BaseAnim_GetAttached( nBaseTapIndex );
	FASSERT( pAnimSource->GetAnimSourceType() == CFAnimSource::TYPE_INST );
	return ((CFAnimInst *)pAnimSource)->DeltaTime( fDeltaTime, bClamp );
}


FINLINE void CBotAnimStack::BaseAnim_UpdateTime( u32 nBaseTapIndex, f32 fNewTime ) {
	FASSERT( IsCreated() );
	FASSERT( nBaseTapIndex < m_pBotAnimStackDef->m_NameSetTaps.nBaseCount );
	CFAnimSource *pAnimSource = BaseAnim_GetAttached( nBaseTapIndex );
	FASSERT( pAnimSource->GetAnimSourceType() == CFAnimSource::TYPE_INST );
	((CFAnimInst *)pAnimSource)->UpdateTime( fNewTime );
}


FINLINE f32 CBotAnimStack::BaseAnim_GetUnitTime( u32 nBaseTapIndex ) const {
	FASSERT( IsCreated() );
	FASSERT( nBaseTapIndex < m_pBotAnimStackDef->m_NameSetTaps.nBaseCount );
	CFAnimSource *pAnimSource = BaseAnim_GetAttached( nBaseTapIndex );
	FASSERT( pAnimSource->GetAnimSourceType() == CFAnimSource::TYPE_INST );
	return ((CFAnimInst *)pAnimSource)->GetUnitTime();
}


FINLINE BOOL CBotAnimStack::BaseAnim_DeltaUnitTime( u32 nBaseTapIndex, f32 fDeltaTime, BOOL bClamp ) {
	FASSERT( IsCreated() );
	FASSERT( nBaseTapIndex < m_pBotAnimStackDef->m_NameSetTaps.nBaseCount );
	CFAnimSource *pAnimSource = BaseAnim_GetAttached( nBaseTapIndex );
	FASSERT( pAnimSource->GetAnimSourceType() == CFAnimSource::TYPE_INST );
	return ((CFAnimInst *)pAnimSource)->DeltaUnitTime( fDeltaTime, bClamp );
}


FINLINE void CBotAnimStack::BaseAnim_UpdateUnitTime( u32 nBaseTapIndex, f32 fNewTime ) {
	FASSERT( IsCreated() );
	FASSERT( nBaseTapIndex < m_pBotAnimStackDef->m_NameSetTaps.nBaseCount );
	CFAnimSource *pAnimSource = BaseAnim_GetAttached( nBaseTapIndex );
	FASSERT( pAnimSource->GetAnimSourceType() == CFAnimSource::TYPE_INST );
	((CFAnimInst *)pAnimSource)->UpdateUnitTime( fNewTime );
}


// User animations:
FINLINE u32 CBotAnimStack::UserAnim_GetCount( void ) const {
	FASSERT( IsCreated() );
	return m_pBotAnimStackDef->m_nUserCount;
}


FINLINE void CBotAnimStack::UserAnim_Attach( u32 nUserIndex, CFAnimSource *pAnimSource ) {
	FASSERT( IsCreated() );
	FASSERT( nUserIndex < m_pBotAnimStackDef->m_nUserCount );
	m_pAnimCombiner->AttachToTap( m_pnAnimTapID[m_pBotAnimStackDef->m_NameSetTaps.nFirstUserIndex + nUserIndex], pAnimSource );
}


FINLINE CFAnimSource *CBotAnimStack::UserAnim_GetAttached( u32 nUserIndex ) const {
	FASSERT( IsCreated() );
	FASSERT( nUserIndex < m_pBotAnimStackDef->m_nUserCount );
	return m_pAnimCombiner->GetSourceDrivingTap( m_pnAnimTapID[m_pBotAnimStackDef->m_NameSetTaps.nFirstUserIndex + nUserIndex] );
}


FINLINE void CBotAnimStack::UserAnim_SetControlValue( u32 nUserIndex, f32 fUnitControlValue ) {
	FASSERT( IsCreated() );
	FASSERT( nUserIndex < m_pBotAnimStackDef->m_nUserCount );
	m_pAnimCombiner->SetControlValue( m_pnAnimControlID[m_pBotAnimStackDef->m_NameSetTaps.nFirstUserIndex + nUserIndex], fUnitControlValue );
}


FINLINE f32 CBotAnimStack::UserAnim_GetControlValue( u32 nUserIndex ) const {
	FASSERT( IsCreated() );
	FASSERT( nUserIndex < m_pBotAnimStackDef->m_nUserCount );
	return m_pAnimCombiner->GetControlValue( m_pnAnimControlID[m_pBotAnimStackDef->m_NameSetTaps.nFirstUserIndex + nUserIndex] );
}


FINLINE void CBotAnimStack::UserAnim_EnableAllBones( u32 nUserIndex ) {
	FASSERT( IsCreated() );
	FASSERT( nUserIndex < m_pBotAnimStackDef->m_nUserCount );
	m_pAnimCombiner->Mask_EnableAllBones( m_pnAnimTapID[m_pBotAnimStackDef->m_NameSetTaps.nFirstUserIndex + nUserIndex] );
}


FINLINE void CBotAnimStack::UserAnim_DisableAllBones( u32 nUserIndex ) {
	FASSERT( IsCreated() );
	FASSERT( nUserIndex < m_pBotAnimStackDef->m_nUserCount );
	m_pAnimCombiner->Mask_DisableAllBones( m_pnAnimTapID[m_pBotAnimStackDef->m_NameSetTaps.nFirstUserIndex + nUserIndex] );
}


FINLINE void CBotAnimStack::UserAnim_UpdateBoneMask( u32 nUserIndex, cchar *pszBoneName, BOOL bDriveBone ) {
	FASSERT( IsCreated() );
	FASSERT( nUserIndex < m_pBotAnimStackDef->m_nUserCount );
	m_pAnimCombiner->Mask_UpdateTapBoneMask( m_pnAnimTapID[m_pBotAnimStackDef->m_NameSetTaps.nFirstUserIndex + nUserIndex], pszBoneName, bDriveBone );
}


FINLINE BOOL CBotAnimStack::UserAnim_IsDrivingBone( u32 nUserIndex, cchar *pszBoneName ) const {
	FASSERT( IsCreated() );
	FASSERT( nUserIndex < m_pBotAnimStackDef->m_nUserCount );
	return m_pAnimCombiner->Mask_IsDrivingBone( m_pnAnimTapID[m_pBotAnimStackDef->m_NameSetTaps.nFirstUserIndex + nUserIndex], pszBoneName );
}


FINLINE void CBotAnimStack::UserAnim_BatchUpdateTapBoneMask_Open( u32 nUserIndex, BOOL bDriveBonesInList ) {
	FASSERT( IsCreated() );
	FASSERT( nUserIndex < m_pBotAnimStackDef->m_nUserCount );
	m_pAnimCombiner->Mask_BatchUpdateTapBoneMask_Open( m_pnAnimTapID[m_pBotAnimStackDef->m_NameSetTaps.nFirstUserIndex + nUserIndex], bDriveBonesInList );
}


FINLINE void CBotAnimStack::UserAnim_BatchUpdateTapBoneMask_Close( void ) {
	FASSERT( IsCreated() );
	m_pAnimCombiner->Mask_BatchUpdateTapBoneMask_Close();
}


FINLINE void CBotAnimStack::UserAnim_BatchUpdateTapBoneMask( const u8 *pnBoneNameIndexTable, cchar **ppszBoneNameTable ) {
	FASSERT( IsCreated() );
	m_pAnimCombiner->Mask_BatchUpdateTapBoneMask( pnBoneNameIndexTable, ppszBoneNameTable );
}


// Built-in summer:
FINLINE void CBotAnimStack::Summer_SetControlValue( f32 fUnitControlValue ) {
	FASSERT( IsCreated() );
	m_pAnimCombiner->SetControlValue( m_pnAnimControlID[m_pBotAnimStackDef->m_NameSetControls.nSummerIndex], fUnitControlValue );
}


FINLINE f32 CBotAnimStack::Summer_GetControlValue( void ) const {
	FASSERT( IsCreated() );
	return m_pAnimCombiner->GetControlValue( m_pnAnimControlID[m_pBotAnimStackDef->m_NameSetControls.nSummerIndex] );
}


FINLINE BOOL CBotAnimStack::Summer_IsControlSmoothingEnabled( void ) const {
	FASSERT( IsCreated() );
	return m_pAnimCombiner->IsControlSmoothingEnabled( m_pnAnimControlID[m_pBotAnimStackDef->m_NameSetControls.nSummerIndex] );
}


FINLINE void CBotAnimStack::Summer_EnableControlSmoothing( void ) {
	FASSERT( IsCreated() );
	m_pAnimCombiner->EnableControlSmoothing( m_pnAnimControlID[m_pBotAnimStackDef->m_NameSetControls.nSummerIndex] );
}


FINLINE void CBotAnimStack::Summer_DisableControlSmoothing( void ) {
	FASSERT( IsCreated() );
	m_pAnimCombiner->DisableControlSmoothing( m_pnAnimControlID[m_pBotAnimStackDef->m_NameSetControls.nSummerIndex] );
}


FINLINE void CBotAnimStack::Summer_EnableAllBones( void ) {
	FASSERT( IsCreated() );
	m_pAnimCombiner->Mask_EnableAllBones( m_pnAnimTapID[m_pBotAnimStackDef->m_NameSetTaps.nSummerIndex] );
}


FINLINE void CBotAnimStack::Summer_DisableAllBones( void ) {
	FASSERT( IsCreated() );
	m_pAnimCombiner->Mask_DisableAllBones( m_pnAnimTapID[m_pBotAnimStackDef->m_NameSetTaps.nSummerIndex] );
}


FINLINE void CBotAnimStack::Summer_UpdateBoneMask( cchar *pszBoneName, BOOL bDriveBone ) {
	FASSERT( IsCreated() );
	m_pAnimCombiner->Mask_UpdateTapBoneMask( m_pnAnimTapID[m_pBotAnimStackDef->m_NameSetTaps.nSummerIndex], pszBoneName, bDriveBone );
}


FINLINE void CBotAnimStack::Summer_UpdateBoneMask( const u8 *pnBoneNameIndexTable, BOOL bDriveBonesInList ) {
	FASSERT( IsCreated() );
	m_pAnimCombiner->Mask_UpdateTapBoneMask( m_pnAnimTapID[m_pBotAnimStackDef->m_NameSetTaps.nSummerIndex], pnBoneNameIndexTable, m_pBotAnimStackDef->m_apszBoneNameTable, bDriveBonesInList );
}


FINLINE BOOL CBotAnimStack::Summer_IsDrivingBone( cchar *pszBoneName ) const {
	FASSERT( IsCreated() );
	return m_pAnimCombiner->Mask_IsDrivingBone( m_pnAnimTapID[m_pBotAnimStackDef->m_NameSetTaps.nSummerIndex], pszBoneName );
}

// Idle animations:
FINLINE u32 CBotAnimStack::IdleAnim_GetCount( void ) const {
	FASSERT( IsCreated() );
	return m_pBotAnimStackDef->m_nIdleCount;
}


FINLINE void CBotAnimStack::IdleAnim_Attach( u32 nIdleIndex, CFAnimSource *pAnimSource ) {
	FASSERT( IsCreated() );
	FASSERT( nIdleIndex < m_pBotAnimStackDef->m_nIdleCount );
	m_pAnimCombiner->AttachToTap( m_pnAnimTapID[m_pBotAnimStackDef->m_NameSetTaps.nFirstIdleIndex + nIdleIndex], pAnimSource );
}


FINLINE CFAnimSource *CBotAnimStack::IdleAnim_GetAttached( u32 nIdleIndex ) const {
	FASSERT( IsCreated() );
	FASSERT( nIdleIndex < m_pBotAnimStackDef->m_nIdleCount );
	return m_pAnimCombiner->GetSourceDrivingTap( m_pnAnimTapID[m_pBotAnimStackDef->m_NameSetTaps.nFirstIdleIndex + nIdleIndex] );
}


FINLINE void CBotAnimStack::IdleAnim_SetControlValue( u32 nIdleIndex, f32 fUnitControlValue ) {
	FASSERT( IsCreated() );
	FASSERT( nIdleIndex < m_pBotAnimStackDef->m_nIdleCount );
	m_pAnimCombiner->SetControlValue( m_pnAnimControlID[m_pBotAnimStackDef->m_NameSetTaps.nFirstIdleIndex + nIdleIndex], fUnitControlValue );
}


FINLINE f32 CBotAnimStack::IdleAnim_GetControlValue( u32 nIdleIndex ) const {
	FASSERT( IsCreated() );
	FASSERT( nIdleIndex < m_pBotAnimStackDef->m_nIdleCount );
	return m_pAnimCombiner->GetControlValue( m_pnAnimControlID[m_pBotAnimStackDef->m_NameSetTaps.nFirstIdleIndex + nIdleIndex] );
}

FINLINE BOOL CBotAnimStack::IdleAnim_IsControlSmoothingEnabled( u32 nIdleControlIndex ) const {
	FASSERT( IsCreated() );
	FASSERT( nIdleControlIndex < m_pBotAnimStackDef->m_NameSetControls.nIdleCount );
	return m_pAnimCombiner->IsControlSmoothingEnabled( m_pnAnimControlID[nIdleControlIndex] );
}


FINLINE void CBotAnimStack::IdleAnim_EnableControlSmoothing( u32 nIdleControlIndex ) {
	FASSERT( IsCreated() );
	FASSERT( nIdleControlIndex < m_pBotAnimStackDef->m_NameSetControls.nIdleCount );
	m_pAnimCombiner->EnableControlSmoothing( m_pnAnimControlID[nIdleControlIndex] );
}


FINLINE void CBotAnimStack::IdleAnim_DisableControlSmoothing( u32 nIdleControlIndex ) {
	FASSERT( IsCreated() );
	FASSERT( nIdleControlIndex < m_pBotAnimStackDef->m_NameSetControls.nIdleCount );
	m_pAnimCombiner->DisableControlSmoothing( m_pnAnimControlID[nIdleControlIndex] );
}


FINLINE void CBotAnimStack::IdleAnim_EnableAllBones( u32 nIdleIndex ) {
	FASSERT( IsCreated() );
	FASSERT( nIdleIndex < m_pBotAnimStackDef->m_nIdleCount );
	m_pAnimCombiner->Mask_EnableAllBones( m_pnAnimTapID[m_pBotAnimStackDef->m_NameSetTaps.nFirstIdleIndex + nIdleIndex] );
}


FINLINE void CBotAnimStack::IdleAnim_DisableAllBones( u32 nIdleIndex ) {
	FASSERT( IsCreated() );
	FASSERT( nIdleIndex < m_pBotAnimStackDef->m_nIdleCount );
	m_pAnimCombiner->Mask_DisableAllBones( m_pnAnimTapID[m_pBotAnimStackDef->m_NameSetTaps.nFirstIdleIndex + nIdleIndex] );
}


FINLINE void CBotAnimStack::IdleAnim_UpdateBoneMask( u32 nIdleIndex, cchar *pszBoneName, BOOL bDriveBone ) {
	FASSERT( IsCreated() );
	FASSERT( nIdleIndex < m_pBotAnimStackDef->m_nIdleCount );
	m_pAnimCombiner->Mask_UpdateTapBoneMask( m_pnAnimTapID[m_pBotAnimStackDef->m_NameSetTaps.nFirstIdleIndex + nIdleIndex], pszBoneName, bDriveBone );
}


FINLINE BOOL CBotAnimStack::IdleAnim_IsDrivingBone( u32 nIdleIndex, cchar *pszBoneName ) const {
	FASSERT( IsCreated() );
	FASSERT( nIdleIndex < m_pBotAnimStackDef->m_nIdleCount );
	return m_pAnimCombiner->Mask_IsDrivingBone( m_pnAnimTapID[m_pBotAnimStackDef->m_NameSetTaps.nFirstIdleIndex + nIdleIndex], pszBoneName );
}


FINLINE void CBotAnimStack::IdleAnim_BatchUpdateTapBoneMask_Open( u32 nIdleIndex, BOOL bDriveBonesInList ) {
	FASSERT( IsCreated() );
	FASSERT( nIdleIndex < m_pBotAnimStackDef->m_nIdleCount );
	m_pAnimCombiner->Mask_BatchUpdateTapBoneMask_Open( m_pnAnimTapID[m_pBotAnimStackDef->m_NameSetTaps.nFirstIdleIndex + nIdleIndex], bDriveBonesInList );
}


FINLINE void CBotAnimStack::IdleAnim_BatchUpdateTapBoneMask_Close( void ) {
	FASSERT( IsCreated() );
	m_pAnimCombiner->Mask_BatchUpdateTapBoneMask_Close();
}


FINLINE void CBotAnimStack::IdleAnim_BatchUpdateTapBoneMask( const u8 *pnBoneNameIndexTable, cchar **ppszBoneNameTable ) {
	FASSERT( IsCreated() );
	m_pAnimCombiner->Mask_BatchUpdateTapBoneMask( pnBoneNameIndexTable, ppszBoneNameTable );
}

FINLINE f32 CBotAnimStack::IdleAnim_GetTotalTime( u32 nIdleTapIndex ) const {
	FASSERT( IsCreated() );
	FASSERT( nIdleTapIndex < m_pBotAnimStackDef->m_NameSetTaps.nIdleCount );
	CFAnimSource *pAnimSource = IdleAnim_GetAttached( nIdleTapIndex );
	FASSERT( pAnimSource->GetAnimSourceType() == CFAnimSource::TYPE_INST );
	return ((CFAnimInst *)pAnimSource)->GetTotalTime();
}


FINLINE f32 CBotAnimStack::IdleAnim_GetOOTotalTime( u32 nIdleTapIndex ) const {
	FASSERT( IsCreated() );
	FASSERT( nIdleTapIndex < m_pBotAnimStackDef->m_NameSetTaps.nIdleCount );
	CFAnimSource *pAnimSource = IdleAnim_GetAttached( nIdleTapIndex );
	FASSERT( pAnimSource->GetAnimSourceType() == CFAnimSource::TYPE_INST );
	return ((CFAnimInst *)pAnimSource)->GetOOTotalTime();
}


FINLINE void CBotAnimStack::IdleAnim_ZeroTime( u32 nIdleTapIndex ) {
	FASSERT( IsCreated() );
	FASSERT( nIdleTapIndex < m_pBotAnimStackDef->m_NameSetTaps.nIdleCount );
	CFAnimSource *pAnimSource = IdleAnim_GetAttached( nIdleTapIndex );
	FASSERT( pAnimSource->GetAnimSourceType() == CFAnimSource::TYPE_INST );
	((CFAnimInst *)pAnimSource)->UpdateTime( 0.0f );
}


FINLINE f32 CBotAnimStack::IdleAnim_GetTime( u32 nIdleTapIndex ) const {
	FASSERT( IsCreated() );
	FASSERT( nIdleTapIndex < m_pBotAnimStackDef->m_NameSetTaps.nIdleCount );
	CFAnimSource *pAnimSource = IdleAnim_GetAttached( nIdleTapIndex );
	FASSERT( pAnimSource->GetAnimSourceType() == CFAnimSource::TYPE_INST );
	return ((CFAnimInst *)pAnimSource)->GetTime();
}


FINLINE BOOL CBotAnimStack::IdleAnim_DeltaTime( u32 nIdleTapIndex, f32 fDeltaTime, BOOL bClamp ) {
	FASSERT( IsCreated() );
	FASSERT( nIdleTapIndex < m_pBotAnimStackDef->m_NameSetTaps.nIdleCount );
	CFAnimSource *pAnimSource = IdleAnim_GetAttached( nIdleTapIndex );
	FASSERT( pAnimSource->GetAnimSourceType() == CFAnimSource::TYPE_INST );
	return ((CFAnimInst *)pAnimSource)->DeltaTime( fDeltaTime, bClamp );
}


FINLINE void CBotAnimStack::IdleAnim_UpdateTime( u32 nIdleTapIndex, f32 fNewTime ) {
	FASSERT( IsCreated() );
	FASSERT( nIdleTapIndex < m_pBotAnimStackDef->m_NameSetTaps.nIdleCount );
	CFAnimSource *pAnimSource = IdleAnim_GetAttached( nIdleTapIndex );
	FASSERT( pAnimSource->GetAnimSourceType() == CFAnimSource::TYPE_INST );
	((CFAnimInst *)pAnimSource)->UpdateTime( fNewTime );
}


FINLINE f32 CBotAnimStack::IdleAnim_GetUnitTime( u32 nIdleTapIndex ) const {
	FASSERT( IsCreated() );
	FASSERT( nIdleTapIndex < m_pBotAnimStackDef->m_NameSetTaps.nIdleCount );
	CFAnimSource *pAnimSource = IdleAnim_GetAttached( nIdleTapIndex );
	FASSERT( pAnimSource->GetAnimSourceType() == CFAnimSource::TYPE_INST );
	return ((CFAnimInst *)pAnimSource)->GetUnitTime();
}


FINLINE BOOL CBotAnimStack::IdleAnim_DeltaUnitTime( u32 nIdleTapIndex, f32 fDeltaTime, BOOL bClamp ) {
	FASSERT( IsCreated() );
	FASSERT( nIdleTapIndex < m_pBotAnimStackDef->m_NameSetTaps.nIdleCount );
	CFAnimSource *pAnimSource = IdleAnim_GetAttached( nIdleTapIndex );
	FASSERT( pAnimSource->GetAnimSourceType() == CFAnimSource::TYPE_INST );
	return ((CFAnimInst *)pAnimSource)->DeltaUnitTime( fDeltaTime, bClamp );
}


FINLINE void CBotAnimStack::IdleAnim_UpdateUnitTime( u32 nIdleTapIndex, f32 fNewTime ) {
	FASSERT( IsCreated() );
	FASSERT( nIdleTapIndex < m_pBotAnimStackDef->m_NameSetTaps.nIdleCount );
	CFAnimSource *pAnimSource = IdleAnim_GetAttached( nIdleTapIndex );
	FASSERT( pAnimSource->GetAnimSourceType() == CFAnimSource::TYPE_INST );
	((CFAnimInst *)pAnimSource)->UpdateUnitTime( fNewTime );
}


#endif

