//////////////////////////////////////////////////////////////////////////////////////
// AIThought.h - 
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
// 04/15/02 MacKellar   Created.
// 10/08/02 MacKellar   Added RTTI
//////////////////////////////////////////////////////////////////////////////////////
#ifndef _AITHOUGHT_H_
#define _AITHOUGHT_H_ 1

#include "ftl.h"

class CAIBrain;
class CAIMemorySmall;

//
// Enumeration of all thought class interfaces
// use for RTTI of thoughts (pass to SupportsClassInterface() or GetClassInterface()
//
enum
{
	NO_THOUGHT_CONNECTED = -2,
	CLASS_CAITHOUGHT = -1,
	CLASS_CGENERICWAIT = 0,		   //MUST BE zero
	CLASS_CGENERICWANDER,
	CLASS_CGENERICPATROL,
	CLASS_CGROUNDCOMBAT,
	CLASS_CGENERICGOTO,
	CLASS_CGENERICFACEIT,
	CLASS_CGENERICTALKTO,
	CLASS_CGENERICFOLLOW,
	CLASS_CGROUNDSEARCH,
	CLASS_CPILLWAIT,
	CLASS_CCORROSIVECOMBAT,
	CLASS_CZOMBIEBOSSCOMBAT,
	CLASS_CSCOUTALERT,
	NUM_THOUGHTCLASSES
};


FCLASS_ALIGN_PREFIX class CAIThought
{
public:
	enum
	{
		THOUGHTFLAG_NONE								= 0x0000,
		THOUGHTFLAG_JOB									= 0x0001,		//this thought is being used as a job
		THOUGHTFLAG_GOAL								= 0x0002,		//this thought is being used as a goal
		THOUGHTFLAG_GOAL_COMPLETE						= 0x0004,		//If a goal, this thought is complete
		THOUGHTFLAG_GOAL_FAILED							= 0x0008,		//If a goal, this thought is failed
		THOUGHTFLAG_USE_JOB_REACT_RULES					= 0x0010,		//allows the AIBrain to perform it's global reaction rules
		THOUGHTFLAG_GOAL_HEAD_PRIORITY					= 0x0020,		//this thought blocks AIBrain from controlling the Head of the Bot when looking at sounds, etc
		THOUGHTFLAG_GOAL_HAS_TIMEOUT					= 0x0040,		//this thought has a timeout (use m_uGoalTimeOut)
		THOUGHTFLAG_LOD_TREAT_GOAL_WITH_JOB_LOD_RULES	= 0x0080,		//this goal says that it can be treated like a Job in terms of LOD priority
	};
	
	CAIThought(void) :	m_pBrain(NULL),
						m_pParamPack(NULL),
						m_uThoughtFlags(THOUGHTFLAG_NONE)						{ }
	
	virtual void Init(	CAIBrain* pBrain,
						u8 uThoughtFlags,
						u8 uThoughtType,
						CAIMemorySmall* pParamPack = NULL);
	virtual void Cleanup(void);
	virtual void Work(void)														{ }

	// RTTI
	virtual BOOL SupportsClassInterface(const s32 uInterfaceId);
	virtual s32 GetClassInterface(void);

	FINLINE BOOL IsComplete(void) const											{ return (m_uThoughtFlags & THOUGHTFLAG_GOAL_COMPLETE)!=0;}
	FINLINE BOOL IsFailed(void) const											{ return (m_uThoughtFlags & THOUGHTFLAG_GOAL_FAILED)!=0;}
	FINLINE CAIBrain* GetBrain(void)											{ return m_pBrain;}
	FINLINE u16	GetThoughtFlags(void) const										{ return m_uThoughtFlags;}
	FINLINE CAIMemorySmall* GetParamPack(void) 									{ return m_pParamPack;}
	FINLINE void ClearParamPack(void)											{ m_pParamPack = NULL;}
	FINLINE void SetThoughtType(u8 uThoughtType)								{ m_uThoughtType = uThoughtType;}
	FINLINE u8 GetThoughtType(void)	const										{ return m_uThoughtType;}
	//
	// Every thought class must implement a version of these static function
	//	  the brain manager maintains pointers these functions
	static BOOL InitBank(s32 nHowMany);
	static void CleanupBank(void);

public:

	static CAIThought* BankAccess(CAIThought* pThought); //pass NULL to retrieve a thought object from the pre-allocated bank, pass a pointer to existing one to to recycle it
	static CNiBank<CAIThought>* s_pAllocBank;

	u8	m_uThoughtFlags;						//THOUGHTFLAG_*
protected:
	u8	m_uThoughtType;							//CAIBrain::TT_*
	u16 m_uGoalTimeOut;							//if this thought is a goal, it will fail at this time
	CAIBrain* m_pBrain;
	CAIMemorySmall* m_pParamPack;				//parampack is a CAIMemory* structure that contains parameters (format is specific to the thought class  (parampacks are decoded in CAIThought::Init

	FCLASS_STACKMEM_ALIGN(CAIThought);			//needs alignment for base classes
} FCLASS_ALIGN_SUFFIX;


typedef CAIThought* ThoughtBank_AccessFunc(CAIThought* pNullForGetNotNullForRecycle);  // pass NULL to retrieve a thought object from the pre-allocated bank, pass a pointer to existing one to to recycle it
typedef BOOL ThoughtBank_InitFunc(s32 nHowMany);
typedef void ThoughtBank_CleanupFunc(void);

ThoughtBank_AccessFunc* aithought_ThoughtClassTypeToAccessFunc(u32 uAIThoughtClassType);

#endif //_AITHOUGHT_H_

