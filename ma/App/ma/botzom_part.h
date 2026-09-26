//////////////////////////////////////////////////////////////////////////////////////
// botzom_part.cpp - Zombie bot part module.
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
// 01/27/03 Ranck       Created.
//////////////////////////////////////////////////////////////////////////////////////

#ifndef _BOTZOM_PART_H_
#define _BOTZOM_PART_H_ 1

#include "fang.h"
#include "fmath.h"
#include "fmesh.h"


class CBotZom;
class CBotZomPartGroup;
class CFWorldMesh;
class CFAnimInst;
class CDamageResult;




FCLASS_ALIGN_PREFIX class CQuatPos {
public:
	CFQuatA m_Quat;
	CFVec3A m_Pos;

	FCLASS_STACKMEM_ALIGN( CQuatPos );
} FCLASS_ALIGN_SUFFIX;




//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CBotZomPart
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

FCLASS_ALIGN_PREFIX class CBotZomPart {
//----------------------------------------------------------------------------------------------------------------------------------
// Protected Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
protected:

	typedef enum {
		MODE_DONE,								// The part is done moving

		MODE_FORMING,							// The part is in forming mode
		MODE_DEBRIS,							// The part is in debris mode
		MODE_STAGED_DEBRIS,						// The part will be in debris mode when the time is right. Otherwise, it will follow the captured animation.
		MODE_RECOIL,							// The part is recoiling

		MODE_COUNT
	} Mode_e;


	typedef enum {
		CONVERT_DEBRIS_NONE,					// Don't convert debris
		CONVERT_DEBRIS_TO_FORMING,				// Convert debris to MODE_FORMING
		CONVERT_DEBRIS_TO_EXPLOSION,			// Detonate debris

		CONVERT_DEBRIS_COUNT
	} ConvertDebris_e;


	enum {
		FLAG_FLATTEN_MODE				= 0x01,	// Flatten the part to match the surface normal
		FLAG_ROUTE_ANIM_TO_HIDDEN_PAL	= 0x02,	// Part needs m_QuatPosEnd to be loaded with the captured animation matrix from the bot
		FLAG_WAITING_BEFORE_FORMING		= 0x04,	// Waiting a little while before moving the part in forming mode
		FLAG_PLAYED_DEBRIS_IMPACT_SOUND	= 0x08,	// Set when we've already played the debris impact sound
		FLAG_EMP_DEBRIS_MODE			= 0x10,	// Set to enable EMP debris mode (What's this, you ask? Try it and find out!)
		FLAG_NO_FORMING_SCURVE			= 0x20,	// Disables S-curve smoothing during the forming animation
		FLAG_PLAY_SPARSED_CLATTER_SOUND	= 0x40,	// Set to play clatter sound for only a fraction of the parts

		FLAG_NONE						= 0
	};




//----------------------------------------------------------------------------------------------------------------------------------
// Protected Data:
//----------------------------------------------------------------------------------------------------------------------------------
protected:

	u8 m_nMode;							// This part's mode (see Mode_e)
	u8 m_nFlags;						// See FLAG_* for info
	u8 m_nConvertDebris;				// Used only when m_nMode is MODE_DEBRIS. When m_fTimer reaches 0, convert from debris to another type (see ConvertDebris_e)
	u8 m_nScaledHeight;					// Used only for MODE_FORMING. 0->4  and 255->10

	f32 m_fTimer;						// Used for general timing
	f32 m_fSpeedMult;					// For debris, this is the amount of stationary time. Used by other modes to vary the animation timing, etc.
	f32 m_fRotSpeed;					// Used by debris mode. Speed of rotation about m_UnitRotAxis_WS.

	CQuatPos m_QuatPosStart;
	CQuatPos m_QuatPosEnd;

	CFVec3A m_LinVel_WS;
	CFVec3A m_UnitRotAxis_WS;
	CFVec3A m_FlattenModeUnitFaceNorm;
	CFVec3A m_FlattenModeUnitRotAxis;




//----------------------------------------------------------------------------------------------------------------------------------
// Protected Functions:
//----------------------------------------------------------------------------------------------------------------------------------
protected:

	BOOL Work( CBotZom *pBot, u32 nBoneIndex );




//----------------------------------------------------------------------------------------------------------------------------------
// Private Functions:
//----------------------------------------------------------------------------------------------------------------------------------
private:

	void _ConvertToFormingMode( CFMtx43A *pBoneMtxPalette, const FMeshBone_t *pMeshBone, f32 fSecsToWaitBeforeMoving );
	void _ExplodePart( CBotZom *pBot, CFMtx43A *pBoneMtx, const FMeshBone_t *pMeshBone, u32 nBoneIndex );

	void _Work_Forming( CBotZom *pBot, u32 nBoneIndex );
	void _Work_Debris( CBotZom *pBot, u32 nBoneIndex );
	void _Work_Debris_Move( CBotZom *pBot, CFMtx43A *pBoneMtx, f32 fTimeStepSecs );
	void _Work_Debris_MoveEMP( CBotZom *pBot, CFMtx43A *pBoneMtx, f32 fTimeStepSecs );
	void _Work_StagedDebris( CBotZom *pBot, u32 nBoneIndex );
	void _Work_Recoiling( CBotZom *pBot, u32 nBoneIndex );


	friend class CBotZom;
	friend class CBotZomPartGroup;


	FCLASS_STACKMEM_ALIGN( CBotZomPart );
} FCLASS_ALIGN_SUFFIX;




//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CBotZomPartGroup
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

FCLASS_ALIGN_PREFIX class CBotZomPartGroup {
//----------------------------------------------------------------------------------------------------------------------------------
// Protected Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
protected:

	enum {
		FLAG_LOCKED			= 0x01,			// When set, none of the parts' modes are allowed to change through an InitState_*() call

		FLAG_NONE			= 0
	};

	
	
	
//----------------------------------------------------------------------------------------------------------------------------------
// Protected Data:
//----------------------------------------------------------------------------------------------------------------------------------
protected:

	static BOOL8 m_bGroupsCreated;			// TRUE when CreateGroups() has been called
	static u8 m_nPartCount;					// Number of parts in all groups (same as the number of non-void bones in the Zombie Bot mesh)
	static u8 m_nGroupCount;				// Number of part groups
	static CBotZomPartGroup *m_pGroupArray;	// Array of part groups
	static CFMtx43A **m_apHiddenMtxPalette;	// This is where the poses will be built when we're driving the bot's matrix palette directly
	static CFMtx43A *m_aHiddenMtxPalette;	// This is where the poses will be built when we're driving the bot's matrix palette directly

	u8 m_nFlags;							// See FLAG_* for info
	u8 m_nRoutedPartsCount;					// Number of parts in this group that have their FLAG_ROUTE_ANIM_TO_HIDDEN_PAL flag set
	CBotZom *m_pOwnerBot;					// Bot that owns this part group (NULL=none)
	CBotZomPart *m_pPartArray;				// Part array (number of elements is m_nPartCount)




//----------------------------------------------------------------------------------------------------------------------------------
// Protected Functions:
//----------------------------------------------------------------------------------------------------------------------------------
protected:

	FINLINE CBotZomPartGroup() { m_pPartArray = NULL; }

	static BOOL CreateGroups( u32 nGroupCount, const FMesh_t *pMesh );
	static void DestroyGroups( void );
	static FINLINE BOOL AreGroupsCreated( void ) { return m_bGroupsCreated; }

	static void FreeAllGroups( void );

	void FreeGroup( void );

	static BOOL AssignFreeGroup( CBotZom *pBotZom );
	void UnassignGroup( void );

	static void ComputeBotMtxPalette( CBotZom *pBotZom );
	static void UpdateBotBoundingSphere( CBotZom *pBotZom );

	BOOL Work( void );

	static BOOL InitState_FinalDeathExplosion( CBotZom *pBot );

	static BOOL InitState_Forming( CBotZom *pBot );
	static BOOL InitState_Hopping( CBotZom *pBot );
	static BOOL InitState_EMP( CBotZom *pBot );
	static BOOL InitState_Collapse( CBotZom *pBot );
	static BOOL InitState_StagedCollapse( CBotZom *pBot );
	static BOOL InitState_DebrisFromBlast( CBotZom *pBot, const CDamageResult *pDamageResult );
	static BOOL InitState_DebrisFromImpact( CBotZom *pBot, const CDamageResult *pDamageResult );
	static BOOL InitState_RecoilFromBlast( CBotZom *pBot, const CDamageResult *pDamageResult );
    static BOOL InitState_RecoilFromImpact( CBotZom *pBot, const CDamageResult *pDamageResult );




//----------------------------------------------------------------------------------------------------------------------------------
// Private Functions:
//----------------------------------------------------------------------------------------------------------------------------------
private:

	void _InitPartQuatsFromCurrentBoneMtxPalette( void );



	friend class CBotZom;
	friend class CBotZomPart;


	FCLASS_STACKMEM_ALIGN( CBotZomPartGroup );
} FCLASS_ALIGN_SUFFIX;


#endif

