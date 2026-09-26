//////////////////////////////////////////////////////////////////////////////////////
// detpack entity.h - blows stuff up object.
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
// 02.07.03	Scholz		Created.
//////////////////////////////////////////////////////////////////////////////////////
#ifndef _DETPACKDROP_H_
#define _DETPACKDROP_H_ 1

#include "fang.h"
#include "ftext.h"
#include "meshentity.h"
#include "explosion.h"
#include "Damage.h"

FCLASS_ALIGN_PREFIX class CEDetPackDrop : public CMeshEntity
{
//----------------------------------------------------------------------------------------------------------------------------------
// Public Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
public:
	typedef struct
	{						
		cchar*				pszExplosion;
		f32					fCountDownTime;
		CFSoundGroup*		pSoundActivate;
		CFSoundGroup*		pSoundTick;
		CFSoundGroup*		pSoundTock;
		cchar*				pszFormatNormal;
		cchar*				pszFormatOver;
		f32					fSlowBeepInterval;
		f32					fFastBeepInterval;
		f32					fSlowToFastSwitch;
		f32					fGameOverTime;
		f32					fAlphaMin;
		f32					fAlphaMax;
		f32					fAlphaFrequency;
	} _DetProps_t;

	typedef enum 
	{
		DETDROP_EXISTS=0,
		DETDROP_ACTIVATED,
		DETDROP_COUNTDOWN,
		DETDROP_EXPLODED,
		DETDROP_NOEXIST,
		DETDROP_STATECOUNT,
	}DetState_e;

	typedef enum
	{
		DETPACK_DROPPED,
		DETPACK_COUNTDOWN,
		DETPACK_BOOM,
	}DetEvent_e;

	// DetPack System flags:
	enum 
	{
		DETPACK_SYS_INITIALIZED				= 0x00000001,	// TRUE after system initialized
		DETPACK_SYS_ALT_BEEP				= 0x00000002,	
	};
	enum
	{
		ACTIVATED_MESH = 1,
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
	static const FGameData_TableEntry_t m_aDetPropVocab[];
	static const FGameDataMap_t m_aDetPropMapTable[];
	static _DetProps_t m_DefaultDetProps;
	static u32  m_uSystemFlags;							// See DETSYSTEM_FLAG_* for info
	static u32  m_nClassClientCount;
	static FExplosion_GroupHandle_t m_hExplosionGroup;
	
	// instance variables
    _DetProps_t				m_DetProps;
	union
	{
		cchar*					m_pszMasterDetPack;
		CEDetPackDrop*			m_pMasterDetPack;		// the text and countdown are based on the master pack
	};
	CDamageForm::Damager_t	m_Damager;
	DetState_e				m_eCurState;
	CBot*					m_pActionBot;
	f32*					m_pfTimeRemainingToKablooie;
	f32						m_fUnitAlphaOverride;
	f32						m_fAlphaOverrideFreqMult;
	FMeshTexLayerHandle_t	m_hTexLayer;
	CFWorldAttachedLight*	m_pWorldLight;

	// semi instance variables, only apply to master packs
	u32  m_uInactiveChildCount;
	f32  m_fDetonateTimeLeft;						
	f32  m_fGameOverTimeLeft;						
	f32  m_fTimeUntilNextBeep;					



//----------------------------------------------------------------------------------------------------------------------------------
// Public Functions:
//----------------------------------------------------------------------------------------------------------------------------------
public:
	static BOOL IsInitialized() { return(m_uSystemFlags & DETPACK_SYS_INITIALIZED); }

	/////////////////////////////////////////////////////////////
	// Constructor/destructor.
	CEDetPackDrop();
	virtual ~CEDetPackDrop();
	//
	/////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////
	// Commands.
	// This method informs the Entity that another Entity (the one passed via parameter) pressed the action
	//   button near it.
	virtual BOOL ActionNearby(CEntity *pEntity);

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
private:
	// Internal state...
	void _SetState(DetState_e);
	void _StateWork();
	void _SetDefaults();
	
	friend class CEDetPackDropBuilder;
	FCLASS_STACKMEM_ALIGN(CEDetPackDrop);
} FCLASS_ALIGN_SUFFIX;


FCLASS_ALIGN_PREFIX class CEDetPackDropBuilder : public CMeshEntityBuilder
{
public:
	FINLINE CEDetPackDropBuilder() {}
	virtual void SetDefaults(u64 nEntityTypeBits, u64 nEntityLeafTypeBit, cchar *pszEntityType);

	cchar*	m_pszActivateSound;
	cchar*	m_pszExplosion;
	cchar*	m_pszMasterDetPackName;
	f32		m_fCountDownTime;
	f32		m_fAlphaFreq;
	f32		m_fAlphaMin;
	f32		m_fAlphaMax;
	f32		m_fFasterBeep;


protected:
	virtual BOOL InterpretTable();
	virtual BOOL PostInterpretFixup();

	FCLASS_STACKMEM_ALIGN(CEDetPackDropBuilder);
} FCLASS_ALIGN_SUFFIX;


#endif
