//////////////////////////////////////////////////////////////////////////////////////
// AIFSM.h - 
//
// Author: Pat MacKellar 
//
//
//	   Finite State Manager
//			-queues up state change requests.
//			-each state can specify init, work, and cleanup function callbacks.  
//			-init and cleanup functions can block state changes until they're completed.
//
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
// 10/17/02 MacKellar   Created.
// 03/06/03				Made easier to use, reduced memory useage
//////////////////////////////////////////////////////////////////////////////////////
#ifndef _AIFSM_H_
#define _AIFSM_H_ 1

#include "ftl.h"

typedef BOOL AIFSMCallBack(u32 uControlParam, void *pvParam1, void *pvParam2);
typedef s16 FSMStateHandle;
#define FSMSTATEHANDLE_INVALID -1
#define AIFSM_INIT_CB_DONE		TRUE
#define AIFSM_INIT_CB_NOTDONE		FALSE
#define AIFSM_UNINIT_CB_DONE		TRUE
#define AIFSM_UNINIT_CB_NOTDONE	FALSE
#define AIFSM_WORK_CB_RETURN		TRUE

class CPendingStateChange;

typedef enum
{
	AIFSM_CBTYPE_INIT = 0,
	AIFSM_CBTYPE_WORK,
	AIFSM_CBTYPE_UNINIT,
	NUM_AIFSM_CBTYPES,
} AIFSM_CBType_e;

//
//	Class CFSDMStateDef
//	  - Define A State
//	  - then, pass const ref to the definition when thelling CFSM object to change states.
//
class CFSMStateDef
{
public:
	CFSMStateDef(cchar* pszStateName, AIFSMCallBack* pInitCB, AIFSMCallBack* pWorkCB, AIFSMCallBack* pUninitCB, FSMStateHandle uHandle, u32 uControlParam, void* pvParam1); 
	CFSMStateDef(void);
	void _ClearData(void);

	cchar* m_pszStateName;
	AIFSMCallBack*  m_apCBFuncs[NUM_AIFSM_CBTYPES];		//Three CB funcs.  Any of which can be NULL.
	u32 m_uControlParam;								//u32 value to be passed to CB funcs when they are called
	void *m_pvParam1;									//void* value to be passed to CB funcs when they are called
	FSMStateHandle m_uHandle;							//optionally, states can have handles for client use.  FSM doesen't use it, just allows client to ask pCSFM->GetActiveStateHandle();

	FCLASS_STACKMEM_NOALIGN(CFSMStateDef);
};



//
//	 CFSM Class 
//		 - there is only one active states in the FSM at a time
//		 - add multiple states,  specifying callback functions to be called for each
//		 - use Change State to change active state
//
class CFSM
{
public:
	CFSM(void);
	~CFSM(void);

	typedef enum
	{
		STAGE_INITIALIZING = AIFSM_CBTYPE_INIT,
		STAGE_WORKING = AIFSM_CBTYPE_WORK,
		STAGE_UNINITIALIZING = AIFSM_CBTYPE_UNINIT,
		NUM_STAGES,
		STAGE_INACTIVE = NUM_STAGES,
	}  Stage_e;

	enum
	{
		NO_ACTIVE_STATE = -1,
	};

	enum
	{	
		INITSTATE_NOTDONE  = 0,
		INITSTATE_DONE
	};

	//	 Change to a different state
	//		return of FALSE means state change request was not added to pending list due to lack of memory or something bad
	//
	BOOL ChangeState(	const CFSMStateDef& NewState,
						FSMStateHandle *puStateHandle = NULL,					//if desired, you can pass a ptr to a handle		
						u32* puControlParam = NULL,								//if !NULL, override uControlParam value in the StateDef with u32 at this adress as uControlParam to the CB func
						void **ppvParam1 = NULL);								//if !NULL, override pvParam1 value in the StateDef with void* at this adress as pvParam1 to the CB func

	void Work(void* pParam2 = NULL);  //Call Work(..) to allow CFSM to call cbacks and process state change requests.  param2 will be passed as param2 to any CB's that are called
	const cchar* GetActiveStateName(void);
	FSMStateHandle GetActiveStateHandle(void);
	FSMStateHandle GetPreviouslyActiveStateHandle(void);
	void RemovePendingStates(void);
	void RemovePendingAndActiveStates(BOOL bDoActiveStateCleanup = FALSE);
	FINLINE BOOL IsStateChangePending(void)								{ return (m_PendingStateList.Size() > 0);}

private:

	CPendingStateChange* GetPendingStateChange(void)					{ return m_PendingStateList.Begin().Get();}				
	BOOL PopPendingStateChange(void);						

	CFSMStateDef m_ActiveState;																// 24 bytes
	CNiList<CPendingStateChange*> m_PendingStateList;										// 20
	FSMStateHandle m_PrevActiveStateHandle;														//  2
	u8 m_uActiveStateStage;	 // Which stage the Active State is in (see Stage_e)			//  1
	u8 m_uUnused;																			//  1
public:	
	enum
	{
		CFSM_DEFAULT_GLOBAL_PENDINGSTATECHANGE_POOL_COUNT = 50,
	};

	static BOOL InitSystem(struct FLinkRoot_s* pGlobalPtrNodePool);
	static void UninitSystem(void);
	static CNiBank<CPendingStateChange>* s_pCFSMPendingStateBank;

	FCLASS_STACKMEM_NOALIGN(CFSM);
};


//
//	CPendingStateChange
//	  -internal class, used to manage the queuing up of state changes 
//
FCLASS_NOALIGN_PREFIX class CPendingStateChange
{
public:
	
	const CFSMStateDef& GetStateDef(void)	{ return m_StateDef;}
	
	CFSMStateDef m_StateDef;
	FCLASS_STACKMEM_NOALIGN(CPendingStateChange);
} FCLASS_NOALIGN_SUFFIX;








#endif

