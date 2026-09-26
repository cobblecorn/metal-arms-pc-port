//////////////////////////////////////////////////////////////////////////////////////
// fsound.h - General purpose sound functions.
//
// Author: Steve Ranck     
//////////////////////////////////////////////////////////////////////////////////////
// THIS CODE IS PROPRIETARY PROPERTY OF SWINGIN' APE STUDIOS, INC.
// Copyright (c) 2003
//
// The contents of this file may not be disclosed to third
// parties, copied or duplicated in any form, in whole or in part,
// without the prior written permission of Swingin' Ape Studios, Inc.
//////////////////////////////////////////////////////////////////////////////////////
// Modification History:
//
// Date     Who         Description
// -------- ----------  --------------------------------------------------------------
// 03/2/03 Ranck       Created.
//////////////////////////////////////////////////////////////////////////////////////

#ifndef _FSOUND_H_
#define _FSOUND_H_ 1

#include "fang.h"
#include "fsndfx.h"


// Callback used to convert a string to an AI sound type.
typedef u8 FSoundConvertAICallback_t( cchar *pszString );
typedef void FSoundNotifyAICallback_t( const CFVec3A *pPos_WS, s32 nPlayerIndex, u32 nSoundType, f32 fSoundRadius );


class CFSoundGroup;
class CFSoundInfo;



//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CFSoundInfo
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************
FCLASS_NOALIGN_PREFIX class CFSoundInfo {
//----------------------------------------------------------------------------------------------------------------------------------
// Public Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	typedef struct {
		cchar *m_pszSoundTagName;		// The sound tag name
		FSndFx_FxHandle_t m_hSound;		// The sound tag handle (0=none)

		f32 m_fUnitVol2D;				// 2D volume level (0=off, 1=max)
		f32 m_fUnitVol3D;				// 3D volume level (0=off, 1=max)
		f32 m_fRadius3D;				// 3D radius (0=off)
		f32 m_fRadiusAI;				// AI radius
		f32 m_fMinPitchMult;			// Minimum pitch multiplier
		f32 m_fMaxPitchMult;			// Maximum pitch multiplier
		u32 m_nTypeAI;					// AI sound type (game specific)
	} DecompressedSoundInfo_t;




//----------------------------------------------------------------------------------------------------------------------------------
// Private Data:
//----------------------------------------------------------------------------------------------------------------------------------
private:

	static FSoundNotifyAICallback_t *m_pFcnNotifyAI;




//----------------------------------------------------------------------------------------------------------------------------------
// Public Data:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	cchar *m_pszSoundTagName;		// The sound tag name
	FSndFx_FxHandle_t m_hSound;		// The sound tag handle (0=none)

	u8 m_nVolume2D;					// 2D volume level (0=no sound, 255=max)
	u8 m_nVolume3D;					// 3D volume level (0=no sound, 255=max)
	u8 m_nDeciRadius3D;				// 3D radius (0=smallest, 255=2550 feet)
	u8 m_nDeciRadiusAI;				// AI radius (0=smallest, 255=2550 feet)
	u8 m_nMinPitchPercent;			// Minimum pitch percentage (0=0x, 255=2.55x)
	u8 m_nMaxPitchPercent;			// Maximum pitch percentage (0=0x, 255=2.55x)
	u8 m_nTypeAI;					// AI type code (game-specific)

	u8 m_nPadding;




//----------------------------------------------------------------------------------------------------------------------------------
// Public Functions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	void Decompress( DecompressedSoundInfo_t *pDecompressedSoundInfo ) const;
	void PlaySound( BOOL bPlay2D=TRUE, const CFVec3A *pPos_WS=NULL, s32 nPlayerIndex=-1, BOOL bInformAI=FALSE, f32 fVolMult=1.0f, f32 fPitchMult=1.0f, f32 fRadiusOverride=-1.0f, CFAudioEmitter **ppUserAudioEmitter=NULL ) const;
	CFAudioEmitter *AllocAndPlaySound( BOOL bPlay2D=TRUE, const CFVec3A *pPos_WS=NULL, f32 fVolMult=1.0f, f32 fPitchMult=1.0f, f32 fRadiusOverride=-1.0f, f32 *pfChosenVolume=NULL, f32 *pfChosenPitchMult=NULL ) const;


	friend class CFSoundGroup;

	FCLASS_STACKMEM_NOALIGN( CFSoundInfo );
} FCLASS_NOALIGN_SUFFIX;




//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CFSoundGroup
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************
FCLASS_NOALIGN_PREFIX class CFSoundGroup {
//----------------------------------------------------------------------------------------------------------------------------------
// Private Data:
//----------------------------------------------------------------------------------------------------------------------------------
private:

	static BOOL m_bModuleStartedUp;
	static CFSoundGroup *m_pGroupBase;
	static u32 m_nGroupCount;




//----------------------------------------------------------------------------------------------------------------------------------
// Public Data:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	cchar *m_pszGroupName;			// The name of the group
	u8 m_nSoundInfoCount;			// The number of CFSoundInfo structures that immediately follow this structure
	u8 m_nLastSoundInfoIndexPlayed;	// Used so that the same sound info is never played twice in a row

	u8 m_anPadding[2];




//----------------------------------------------------------------------------------------------------------------------------------
// Public Functions:
//----------------------------------------------------------------------------------------------------------------------------------
public:

	static BOOL ModuleStartup( void );
	static void ModuleShutdown( void );
	static FINLINE BOOL IsModuleStartedUp( void ) { return m_bModuleStartedUp; }

	static BOOL BuildGroupsFromFile( cchar *pszSoundFile, FSoundNotifyAICallback_t *pFcnNotifyAI=NULL, FSoundConvertAICallback_t *pFcnConvertAI=NULL );

	static CFSoundGroup *RegisterGroup( cchar *pszGroupName );

	static CFSoundInfo *GetSoundInfo( CFSoundGroup *pGroup, u32 nSoundInfoIndex );
	static FSndFx_FxHandle_t GetSoundHandle( CFSoundGroup *pGroup, u32 nSoundInfoIndex );

	static CFSoundInfo *GetRandomSoundInfo( CFSoundGroup *pGroup, CFSoundInfo::DecompressedSoundInfo_t *pDestDecompressedSoundInfo=NULL );
	static FSndFx_FxHandle_t GetRandomSoundHandle( CFSoundGroup *pGroup );

	static void PlaySound( CFSoundGroup *pGroup, BOOL bPlay2D=TRUE, const CFVec3A *pPos_WS=NULL, s32 nPlayerIndex=-1, BOOL bInformAI=FALSE, f32 fVolMult=1.0f, f32 fPitchMult=1.0f, f32 fRadiusOverride=-1.0f, CFAudioEmitter **ppUserAudioEmitter=NULL );
	static CFAudioEmitter *AllocAndPlaySound( CFSoundGroup *pGroup, BOOL bPlay2D=TRUE, const CFVec3A *pPos_WS=NULL, f32 fVolMult=1.0f, f32 fPitchMult=1.0f, f32 fRadiusOverride=-1.0f, f32 *pfChosenVolume=NULL, f32 *pfChosenPitchMult=NULL );




//----------------------------------------------------------------------------------------------------------------------------------
// Private Functions:
//----------------------------------------------------------------------------------------------------------------------------------
private:

	static CFSoundGroup *_FindGroup( cchar *pszGroupName );
	static BOOL _AllocateMemoryForGroups( FGameDataFileHandle_t hGameDataFile, cchar *pszSoundFile );
	static BOOL _BuildGroup( CFSoundGroup **ppGroup, cchar *pszSoundFile, FGameDataTableHandle_t hTable, FSoundConvertAICallback_t *pFcnConvertAI );


	friend class CFSoundInfo;

	FCLASS_STACKMEM_NOALIGN( CFSoundGroup );
} FCLASS_NOALIGN_SUFFIX;




#endif

