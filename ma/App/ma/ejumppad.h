//////////////////////////////////////////////////////////////////////////////////////
// ejumppad.h - Jumps ya from point to point
//
// Author: Michael Scholz
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
// Date		Who			Description
// --------	----------	--------------------------------------------------------------
// 03.18.03	Scholz		Created.
//////////////////////////////////////////////////////////////////////////////////////

#ifndef _JUMPPAD_H_
#define _JUMPPAD_H_ 1

#include "fang.h"
#include "ftext.h"
#include "meshentity.h"
#include "explosion.h"
#include "Damage.h"

FCLASS_ALIGN_PREFIX class CEJumpPad : public CMeshEntity
{
//----------------------------------------------------------------------------------------------------------------------------------
// Public Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
public:
	enum
	{
		TAG_POINT_COUNT = 4,			// Number of tag points on Grunt
		ANIM_IDLE_COUNT = 0,
	};

	typedef struct
	{
		CFSoundGroup* pActivateSnd;
	} JumpPadProps_t;

	typedef enum
	{
		JUMPPAD_EXISTS=0,
		JUMPPAD_STATECOUNT,
	}JumpPadState_e;

	typedef enum
	{
		JUMPPAD_JUMP,
	}JumpPadEvent_e;

	typedef enum
	{
		JUMPPAD_INVALID=-1,
		JUMPPAD_NOEFFECT=0,
		JUMPPAD_HIGHJUMP,
		JUMPPAD_EFFECTCOUNT,
	}JumpPadEffect_e;

	// JumpPad System flags:
	enum JUMPPADSYSTEM_FLAG
	{
		JUMPPAD_SYS_INITIALIZED				= 0x00000001,	// TRUE after system initialized
	};

//----------------------------------------------------------------------------------------------------------------------------------
// Protected Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
protected:

	ENTITY_CLASS_HIERARCHY_BITDEF
//----------------------------------------------------------------------------------------------------------------------------------
// Private Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
private:
//----------------------------------------------------------------------------------------------------------------------------------
// Private Data:
//----------------------------------------------------------------------------------------------------------------------------------
private:
	//statics
	static const FGameData_TableEntry_t m_aJumpPadPropVocab[];
	static const FGameDataMap_t m_aJumpPadPropMapTable[];
	static JumpPadProps_t m_JumpPadProps;
	static u32  m_uSystemFlags;							// See JUMPPADSYSTEM_FLAG_* for info
	static u32  m_nClassClientCount;
	
	// non-statics
    JumpPadState_e	m_eCurState;
	union
	{
		cchar*			m_pszTarget;
		CEntity*		m_pTarget;
	};
	CFSoundGroup*	m_pActivateSnd;
	f32				m_fHangTime;
	f32				m_fDragPercent;
	JumpPadEffect_e m_eEffect;

//----------------------------------------------------------------------------------------------------------------------------------
// Public Functions:
//----------------------------------------------------------------------------------------------------------------------------------
public:
	static BOOL IsInitialized() { return(m_uSystemFlags & JUMPPAD_SYS_INITIALIZED); }
	CEJumpPad();
	virtual ~CEJumpPad();

//----------------------------------------------------------------------------------------------------------------------------------
// Protected Functions:
//----------------------------------------------------------------------------------------------------------------------------------
protected:
	virtual void ClassHierarchyDestroy();
	virtual BOOL ClassHierarchyBuild();
	virtual BOOL ClassHierarchyBuilt( void );
	virtual CEntityBuilder *GetLeafClassBuilder();

	virtual BOOL ClassHierarchyLoadSharedResources();
	virtual void ClassHierarchyUnloadSharedResources();

	virtual void ClassHierarchyResolveEntityPointerFixups();
	virtual void ClassHierarchyWork();

	virtual void ClassHierarchyAddToWorld( void );
	virtual void ClassHierarchyRemoveFromWorld( void );
	virtual void ClassHierarchyRelocated( void *pIdentifier );

	virtual void CheckpointSaveSelect( s32 nCheckpoint );
	virtual BOOL CheckpointSave( void );
	virtual void CheckpointRestore( void );

//----------------------------------------------------------------------------------------------------------------------------------
// Private Functions:
//----------------------------------------------------------------------------------------------------------------------------------
	void _SetDefaults(void);
	void _StateWork(void);
	void _SetState(JumpPadState_e eNewState);
	void _BoostBot(CBot* pBot);
	// friendship
	friend class CEJumpPadBuilder;

	FCLASS_STACKMEM_ALIGN(CEJumpPad);
} FCLASS_ALIGN_SUFFIX;


FCLASS_ALIGN_PREFIX class CEJumpPadBuilder : public CMeshEntityBuilder
{
public:
	FINLINE CEJumpPadBuilder() {}
	virtual void SetDefaults(u64 nEntityTypeBits, u64 nEntityLeafTypeBit, cchar *pszEntityType);
	
	CEJumpPad::JumpPadEffect_e eEffect;
	cchar*		pszTarget;
	cchar*		pszActivateSoundGroup;
	f32			fHangTime;
	f32			fDragPercent;

protected:
	virtual BOOL InterpretTable();
	virtual BOOL PostInterpretFixup();
	
	FCLASS_STACKMEM_ALIGN(CEJumpPadBuilder);
} FCLASS_ALIGN_SUFFIX;

#endif
