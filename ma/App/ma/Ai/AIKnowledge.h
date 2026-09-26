#ifndef _AIMEMORY_H_
#define _AIMEMORY_H_ 1

//////////////////////////////////////////////////////////////////////////////////////
// AIKnowledge.h 
//  classes:
//		CAIMemory - How AIBrains remember stuff      
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
// 04/22/02 MacKellar      Created.
//////////////////////////////////////////////////////////////////////////////////////

#include "ftl.h"
#include "fmath.h"

class CEntity;


//memory sizes
enum
{
	MEMORY_SIZE_SMALL = 0,
	MEMORY_SIZE_MEDIUM,
	MEMORY_SIZE_LARGE,
	NUM_MEMORY_SIZES
};


//
//	class CAIMemory
//  - Data Structure to be used to remember things with only an ID and 16bit data field
//
class CAIMemorySmall
{
public:
	void InitSmall(u8 uMemoryId, u16 uHowLongSecs, u8 uControlFlags = 0);
	void ResetTimer(u16 uHowLongSecs);
	u8* PokeF32(u8 *pBuff, f32 fVal)				{
														FASSERT(pBuff>=(u8*)this);
														*((f32*) (((u32)pBuff+3)&~3)) = fVal;
														return  (u8*) ((((u32)pBuff+3)&~3)+4);
													}
	u8* PokeU16(u8 *pBuff, u16 uVal)				{
														FASSERT(pBuff>=(u8*)this);
														*((u16*) pBuff) = uVal;
														return pBuff+2;
													}
	u8* PokeS16(u8 *pBuff, s16 nVal)				{
														FASSERT(pBuff>=(u8*)this);
														*((s16*) pBuff) =nVal;
														return pBuff+2;
													}

	u8* PokeU32(u8 *pBuff, u32 uVal)				{
														FASSERT(pBuff>=(u8*)this);
														*((u32*) (((u32)pBuff+3)&~3)) = uVal;
														return  (u8*) ((((u32)pBuff+3)&~3)+4);
													}
	u8* PokeU8U8(u8 *pBuff, u8 uVal1, u8 uVal2)		{
														FASSERT(pBuff>=(u8*)this);
														*pBuff = uVal1;
														*(pBuff+1) = uVal2;
														return pBuff+2;
													}
	u8* PeekF32(u8 *pBuff, f32 *fVal)				{
														FASSERT(pBuff>=(u8*)this);
														*fVal = *((f32*) (((u32)pBuff+3)&~3));
														return  (u8*) ((((u32)pBuff+3)&~3)+4);
													}
	u8* PeekU16(u8 *pBuff, u16 *uVal)				{
														FASSERT(pBuff>=(u8*)this);
														*uVal = *((u16*) pBuff);
														return pBuff+2;
													};
	u8* PeekS16(u8 *pBuff, s16 *nVal)				{
														FASSERT(pBuff>=(u8*)this);
														*nVal = *((s16*) pBuff);
														return pBuff+2;
													};

	u8* PeekU32(u8 *pBuff, u32 *uVal)				{
														FASSERT(pBuff>=(u8*)this);
														*uVal = *((u32*) (((u32)pBuff+3)&~3));
														return  (u8*) ((((u32)pBuff+3)&~3)+4);
													};
	u8* PeekU8U8(u8 *pBuff, u8 *uVal1, u8 *uVal2);

	enum
	{
		CONTROL_FLAG_NOTIMER				= 0x01,
		CONTROL_FLAG_FRAMECOUNT				= 0x02,
		CONTROL_FLAG_NEW_MEMORY				= 0x04,
		CONTROL_FLAG_HAD_ONE_UPDATE			= 0x08,
		CONTROL_FLAG_ACKNOWLEDGED			= 0x10,
		CONTROL_FLAG_NEWLASTWORK_MEMORY		= 0x20

	};			 
	u8  m_uMemoryId;				//   1  the ID of this memory.  Should be from one of the enum lists in the CAIMemory Hieirarchy
	u8  m_uControlFlags;			//   1  place for little bits of data
	u16 m_uWhenTimeSecs;			//   2  when it happened
	u16 m_uHowLongSecs;				//   2  How long it should be remembered for
									//
	u16 m_uSmallData;				//	 2
									//------
									//	 8
	FCLASS_STACKMEM_NOALIGN(CAIMemorySmall); 
};
typedef CNiIterator<CAIMemorySmall*> CAIMemorySmallIt;


//
//	Class CAIMemoryMedium
//  - Small + u32 + f32
//
class CAIMemoryMedium : public CAIMemorySmall
{
public:
	void InitMedium(u8 uMemoryId, u16 uHowLongSecs, u8 uControlFlags = 0);

	u32 m_uMediumData;				//  4
	f32 m_fMediumData;				//  4
									//-----
									// 16
	FCLASS_STACKMEM_NOALIGN(CAIMemoryMedium); 
};
typedef CNiIterator<CAIMemoryMedium*> CAIMemoryMediumIt;



//
//	Class  CAIMemoryEntity
//  - Small + pEntity + Loc + ObjLoc + u16 + u16
//
class CAIMemoryEntity: public CAIMemoryMedium
{
public:
									//	8 BaseClass (Small)
									//	8 BaseClass (Medium)
	CFVec3 m_Loc;					// 12 Where I was when I saw it
	CFVec3 m_EntityLoc;				// 12 Where it was when I saw it
	CEntity* m_pEntity;				//  4 Pointer to the Entity
	f32	m_fLargeData;				//  4
									//----
									// 48	(42 useable)
	FCLASS_STACKMEM_NOALIGN(CAIMemoryEntity); 
};

//
//	Class  CAIMemoryLarge
//  - same size as large, but used as generic data buffer sometimes
//
class CAIMemoryLarge : public CAIMemoryEntity
{
public:
	void InitLarge(u8 uMemoryId, u16 uHowLongSecs, u8 uControlFlags = 0);

	FCLASS_STACKMEM_NOALIGN(CAIMemoryLarge); 
};


typedef CNiIterator<CAIMemoryLarge*> CAIMemoryLargeIt;

//
//	Class  CAIKnowledge
//	- a Knowledge Object is a container of memories
//  - Work function is required to manage timed expiration of memories
//  - Interface is required for Adding Memories, Canceling them and Checking for their existence
//  - Memories come in three sizes depending on amount of data a type of memory needs.
//  - All memories are pre-allocated into global pools shared by all memories
//  - findfix: probably should add some "max" number of memories that any one knowledge container could have

FCLASS_NOALIGN_PREFIX class CAIKnowledge
{
public:
	CAIKnowledge(void);
	~CAIKnowledge(void);

	void SetPtrListNodePoolUseage(struct FLinkRoot_s* pNodePool);  //to be called just after constructor

	//Request a memory object, then fill it out details
	CAIMemorySmall  *RememberSmall(u8 uMemoryId, u16 uForHowLongSecs, u8 uControlFlags = 0);
	CAIMemoryMedium *RememberMedium(u8 uMemoryId, u16 uForHowLongSecs, u8 uControlFlags = 0);
	CAIMemoryLarge  *RememberLarge(u8 uMemoryId, u16 uForHowLongSecs, u8 uControlFlags = 0);
	CAIMemorySmall  *RememberThis(u8 uMemoryId, u16 uForHowLongSecs, u8 uControlFlags = 0);
	
	BOOL CanRememberEntity(u8 nMemoryId, CEntity* pEntity, CAIMemoryLarge** ppMostRecentMemory = NULL);   //return TRUE and ptr to the most recent memory of the specified type which has an entity ptr equal to that specified
	BOOL CanRememberAny(u8 nMemoryId, CAIMemorySmall** ppMostRecentMemory = NULL);					//return TRUE and ptr to the most recent memory of the specified type.
	CAIMemorySmall* CAIKnowledge::GetNextOldestOfId(CAIMemorySmall* pMemory);
	BOOL IsInMemory(CAIMemorySmall* pMemory);  //Is there a memory in knowledge that matches this
	CAIMemoryLarge* CAIKnowledge::NextUnAcknowledgeMemory(CNiIterator<CAIMemoryLarge*> *pIt);	   //only searches large right now
	CAIMemoryLarge* CAIKnowledge::NextUnAcknowledgeMemory(u8 uMemoryId, CNiIterator<CAIMemoryLarge*> *pIt);	   //only searches large right now
	CAIMemoryLarge* CAIKnowledge::NextMemory(u8 uMemoryId, CNiIterator<CAIMemoryLarge*> *pIt);	   //only searches large right now

	FINLINE CNiIterator<CAIMemorySmall*>  SearchSmall(void)					{ return m_ListSmall.Begin();};
	FINLINE CNiIterator<CAIMemoryMedium*> SearchMedium(void)				{ return m_ListMedium.Begin();};
	FINLINE CNiIterator<CAIMemoryLarge*>  SearchLarge(void)					{ return m_ListLarge.Begin();};

	void ForgetAll(void);							// remove all memories
	void ForgetThis(CAIMemorySmall* pMemory);		// base class ptr.  Can pass Large or Medium as well 
	void ForgetAllId(u8 nMemory);					// Find all memories with this ID and remove them
	void ForgetAllExceptId(u8 nMemoryId);			// Find and remove all memories except those with this ID
	void ForgetOldestSmall(void);					// kill the oldest memory
	void ForgetOldestMedium(void);					// kill the oldest memory
	void ForgetOldestLarge(void);					// kill the oldest memory
	void ForgetOldestMatchSize(u8 nMemory);		// kill the oldest memory in the list of size that matches nMemory
	
	void Work(void);								// check for expired memories and remove them
	void Save(void);								// store contents somewhere 

	BOOL CheckpointSave(void);
	void CheckpointRestore(void);

	void DebugRender(f32 fScreenX, f32 fScreenY);

	static u8 GetMemorySize(CAIMemorySmall* pMem);	  //see enum NUM_MEMORY_SIZES
	static u8 GetSizeBytes(CAIMemorySmall* pMem);	  //size in bytes of this memory

protected:
	CNiList<CAIMemorySmall*> m_ListSmall;
	CNiList<CAIMemoryMedium*> m_ListMedium;
	CNiList<CAIMemoryLarge*> m_ListLarge;


//statics
public:

	enum
	{
		CAIKNOWLEDGE_DEFAULT_SMALLMEMORY_GLOBAL_BANK_SIZE = 500,
		CAIKNOWLEDGE_DEFAULT_MEDIUMMEMORY_GLOBAL_BANK_SIZE = 500,
		CAIKNOWLEDGE_DEFAULT_LARGEMEMORY_GLOBAL_BANK_SIZE = 500,
	};

	//pre-allocates global pools of memory objects small, medium and large sizes
	static BOOL InitSystem(struct FLinkRoot_s* pGlobalPtrNodePool,
							u32 nSmallBankSize = CAIKNOWLEDGE_DEFAULT_SMALLMEMORY_GLOBAL_BANK_SIZE,
							u32 nMediumBankSize = CAIKNOWLEDGE_DEFAULT_MEDIUMMEMORY_GLOBAL_BANK_SIZE,
							u32 nLargeBankSize = CAIKNOWLEDGE_DEFAULT_LARGEMEMORY_GLOBAL_BANK_SIZE);
	static void CleanupSystem(void);
	static void SetSystemPtrListNodePoolUseage(FLinkRoot_t* pNodePool);

protected:

	static CNiBank<CAIMemorySmall>* s_pBankSmall;
	static CNiBank<CAIMemoryMedium>* s_pBankMedium;
	static CNiBank<CAIMemoryLarge>* s_pBankLarge;
	static struct FLinkRoot_s* s_pGlobalPtrNodePool;
	static u32 s_uRenderBits;

	FCLASS_STACKMEM_NOALIGN(CAIKnowledge); 
} FCLASS_NOALIGN_SUFFIX;


#endif //_AIMEMORY_H_ 
