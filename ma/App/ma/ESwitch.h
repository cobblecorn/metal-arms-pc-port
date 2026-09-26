//////////////////////////////////////////////////////////////////////////////////////
// Switch entity.h - Switch object.
//
// Author: Justin Link
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
// Date		Who			Description
// --------	----------	--------------------------------------------------------------
// 04/29/02	Link		Created.
// 05/30/03 pgm			added comments, removed extra vbls and added been activated flag
//////////////////////////////////////////////////////////////////////////////////////
#ifndef _SWITCHENTITY_H_
#define _SWITCHENTITY_H_ 1

#include "fang.h"
#include "meshentity.h"

class CDoorEntity;

FCLASS_ALIGN_PREFIX class CESwitch : public CMeshEntity
{
//----------------------------------------------------------------------------------------------------------------------------------
// Protected Definitions:
//----------------------------------------------------------------------------------------------------------------------------------
protected:

	ENTITY_CLASS_HIERARCHY_BITDEF

public:
	static BOOL InitSystem();
	static void UninitSystem();
	static BOOL InitLevel();
	static void UninitLevel();
	static BOOL IsInitialized() { return(m_bSystemInitialized); }

	enum DoorAction_e
	{
		DOORACTION_OPEN = 0,
		DOORACTION_CLOSE,
		DOORACTION_UNLOCK,
		DOORACTION_LOCK,
		DOORACTION_TOGGLE,

		DOORACTION_COUNT
	};
	/////////////////////////////////////////////////////////////
	//
	enum SwitchState_e
	{
		SWITCHSTATE_IDLE = 0,
		SWITCHSTATE_WAITINGFOROKAY,
		SWITCHSTATE_ACTIVATING,

		SWITCHSTATE_COUNT
	};


	CESwitch();
	virtual ~CESwitch();

	virtual BOOL ActionNearby(CEntity *pEntity);
	virtual void CheckpointSaveSelect( s32 nCheckpoint );
	virtual BOOL CheckpointSave( void );
	virtual void CheckpointRestore( void );

	BOOL IsLocked()				{ return FALSE; }  //locked switches don't do anything
	BOOL RequiresChip(void)		{ return m_bRequireChip;}
	BOOL HasBeenActivated(void) { return m_bHasBeenActivated;}
	BOOL IsBusy(void)			{ 	return m_eCurState != SWITCHSTATE_IDLE;}
	BOOL IsMilOnly(void)		{ return m_bMilOnly;}


	BOOL CanOpenDoor(CDoorEntity* pDoor); //can this switch be used to open the door in question?

protected:
	virtual void ClassHierarchyDestroy();
	virtual BOOL ClassHierarchyBuild();
	virtual BOOL ClassHierarchyBuilt( void );
	virtual CEntityBuilder *GetLeafClassBuilder();

	virtual void ClassHierarchyResolveEntityPointerFixups();
	virtual void ClassHierarchyWork();

private:
	// Internal state...
	void StateTransitions();
	void StateWork();



	// Helper functions.
	void ClearDoorPtrs();
	void _SetDefaults();

	// This is where the real 'effects' of the switch should happen.
	void _DoSwitchActions();

	void _StartActivate();
	void _EndActivate();
	void MakeActivateSounds(BOOL bWorked);

	/////////////////////////////////////////////////////////////
	// State transition behavior.
	enum MoveType_e
	{
		MOVETYPE_BONETRANS,
		MOVETYPE_ANIMATION,

		MOVETYPE_COUNT
	};
	//
	/////////////////////////////////////////////////////////////

	//
	/////////////////////////////////////////////////////////////

	SwitchState_e m_eCurState;
	CDoorEntity *m_apDoor[4];			//up to 4 doors can be controlled by switches
	DoorAction_e m_aeDoorActions[4];
	CEntity *m_pInitEntity;				//the entity that pressed the button
	CFAnimInst *m_pSwitchAnim;

	BOOL8 m_bRequireChip;				//this switch requires a chip
	BOOL8 m_bTriggerEvent;				//this switch will trigger the "switch" script event
	BOOL8 m_bHasBeenActivated;			//TRUE if this switch has ever been activated
	BOOL8 m_bMilOnly;					//TRUE if this switch can only be used by mils

	static FSndFx_FxHandle_t m_hLockedSound;
	static FSndFx_FxHandle_t m_hSwitchTriggeredSound;
	static BOOL m_bSystemInitialized;
	static s32 m_nSwitchEvent;

	FCLASS_STACKMEM_ALIGN(CESwitch);
} FCLASS_ALIGN_SUFFIX;


#endif
