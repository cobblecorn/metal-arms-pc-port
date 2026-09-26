//////////////////////////////////////////////////////////////////////////////////////
// ftl.h - A collection of container templates. 
//          
//          Containers are implemented on top of fang librarys flinklist.
//          Node allocation is memory by default, but can
//            be set containers to pull nodes from pre-allocated pools.
//
//          - CNiList				- non intrusive list 
//          - CNiPriorityQ			- non intrusive list with key-able nodes
//          - CNiNode				- a double link list node template (next, prev, cofigurable data element)
//			- CNiKeyNode_Float		- CNiNode + float key
//			- CNiBank				- Allocate and Manage a bank of objects
//			- CNiIterator			- traverse lists
//          
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
// 02/11/02 patm       Created.
//////////////////////////////////////////////////////////////////////////////////////

#ifndef _FTL_H_
#define _FTL_H_ 1

#include "flinklist.h"
#include "apenew.h"

template <class DT> class CNiList;

//
//
//  CNiNode template
//
//

template <class DT> class CNiNode
{
public:
	inline CNiNode(void) { m_Link.pPrevLink = NULL; m_Link.pNextLink = NULL; };

	enum
	{
		LINKOFFSET = 0,
	};
	FLink_t m_Link;	  //must be first member in this and every derived class. Reason is that the offset of it is used by flinklist.h
	DT m_Data;

	FCLASS_STACKMEM_NOALIGN(CNiNode<DT>);
};

//
//
//  CNiKeyNode_Float  template
//
//
template <class DT> class CNiKeyNode_Float : public CNiNode<DT>
{
public:
	inline CNiKeyNode_Float(void) : CNiNode<DT>(), m_fKey(0.0f) { };
	float m_fKey;
	FCLASS_STACKMEM_NOALIGN(CNiKeyNode_Float<DT>);
};


//
//
//  CNiIterator template
//
//
template <class DT> class CNiIterator
{
public:
	CNiIterator(void) : m_pNode(NULL), m_pList(NULL) {}
	CNiIterator(CNiList<DT> *pList, CNiNode<DT> *pNode) : m_pNode(pNode), m_pList(pList) {}

	void Back(void) { if (m_pNode) m_pNode = (CNiNode<DT>*) flinklist_GetStructurePointer(&m_pList->m_LinkList, m_pNode->m_Link.pPrevLink);}
	void Next(void) { if (m_pNode) m_pNode = (CNiNode<DT>*) flinklist_GetStructurePointer(&m_pList->m_LinkList, m_pNode->m_Link.pNextLink);}
	BOOL HasBack(void) { return (m_pNode && m_pNode->m_Link.pPrevLink);}
	BOOL HasNext(void) { return (m_pNode && m_pNode->m_Link.pNextLink);}
	BOOL HasNextNext(void) { return (m_pNode && m_pNode->m_Link.pNextLink && m_pNode->m_Link.pNextLink->pNextLink );}
	BOOL IsValid(void) { return m_pNode != NULL;}
	void Reset(void) { m_pNode = NULL; m_pList = NULL;}
	
	const DT &Get(void) { if (m_pNode) {return m_pNode->m_Data;} return s_ReturnError;}
	BOOL Get(DT** pDT) { if (m_pNode) {*pDT = &(m_pNode->m_Data); return 1;} *pDT = NULL; return 0;}

	void RemoveThenNext(void)
	{
		if (IsValid())
		{
			CNiIterator<DT> oldIt = (*this);
			Next();

			if (oldIt.m_pNode)
			{
				flinklist_Remove(&m_pList->m_LinkList, oldIt.m_pNode);
				m_pList->FreeNode(oldIt.m_pNode);
			}
		}
	}

	void RemoveThenBack(void)
	{
		if (IsValid())
		{
			CNiIterator<DT> oldIt = (*this);
			Back();

			if (oldIt.m_pNode)
			{
				flinklist_Remove(&m_pList->m_LinkList, oldIt.m_pNode);
				m_pList->FreeNode(oldIt.m_pNode);
			}
		}
	}

	void InsertBefore(const DT& rElement)
	{

		CNiNode<DT>* pNode = m_pList->AllocateNode();
		if (pNode)
		{
			pNode->m_Data = rElement;

			if (m_pNode)
			{
				flinklist_AddBefore(&m_pList->m_LinkList, m_pNode, pNode);
			}
			else
			{
				flinklist_AddTail(&m_pList->m_LinkList, pNode);
			}
		}
	}

	void InsertAfter(const DT& rElement)
	{

		CNiNode<DT>* pNode = m_pList->AllocateNode();
		if (pNode)
		{
			pNode->m_Data = rElement;

			if (m_pNode)
			{
				flinklist_AddAfter(&m_pList->m_LinkList, m_pNode, pNode);
			}
			else
			{
				flinklist_AddHead(&m_pList->m_LinkList, pNode);
			}
		}
	}

protected:
	CNiList<DT> *m_pList;	 //Needed because of flink.
	CNiNode<DT> *m_pNode;
	static DT s_ReturnError;
	FCLASS_STACKMEM_NOALIGN(CNiIterator<DT>);
};


//
//
//  CNiList template
//
//
template <class DT> class CNiList
{
public:
	CNiList(void) 
		: m_pNodePool(NULL)
	{
		flinklist_InitRoot(&m_LinkList, CNiNode<DT>::LINKOFFSET);  
	}
			
	CNiList(FLinkRoot_t* pNodePool)
		: m_pNodePool(pNodePool)
	{
		CNiNode<DT>* pTemp = AllocateNode();
		flinklist_InitRoot(&m_LinkList, (((u32) &(pTemp->m_Link)) - (u32) pTemp));
		FreeNode(pTemp); pTemp = NULL;
	}


	CNiList(void* pData, s32 nDataMaxBytes)
	{
		void* pOriginalData = pData;
		m_pNodePool = (FLinkRoot_t*) pData;
		pData = m_pNodePool+1;
		pData = ((u32)pData + 3) & ~3;  //32bit align
		nDataMaxBytes -= (u32)pOriginalData - (u32) pData;
		FASSERT(nDataMaxBytes >= 0); //better have atleast given me enough room to store the nodepoollist

		s32 nNodeSize = sizeof(CNiNode<DT>);
		s32 nNumElems = nDataMaxBytes/nNodeSize;
		flinklist_InitPoolForward(m_pNodePool, pData, nNodeSize, nNumElems);
	}
	
	~CNiList(void)
	{
		RemoveAll();
		m_pNodePool = NULL;
	}

	//findfix: copying a list is very dangerous, because it means that two lists
	//believe they own the same nodes, so, if one copy starts removing nodes.... the other copy
	//list will contain recycled nodes

	//can be called immediately after construction to tell this list to pull nodes from a specific nodepool
	void SetNodePool(FLinkRoot_t* pNodePool)
	{
		FASSERT(pNodePool);// why pass me a null pointer?
		FASSERT(!m_LinkList.nCount); //probably shouldn't already have a node pool
		m_pNodePool = pNodePool;
	}

	
	void RemoveAll(void)
	{
		FLink_t *pLink = m_LinkList.pHeadLink;
		while (pLink)
		{
			FLink_t *pTmp = pLink->pNextLink;
			CNiNode<DT>* pNode = (CNiNode<DT>*) flinklist_Remove(&m_LinkList, flinklist_GetStructurePointer(&m_LinkList, pLink));
			FreeNode(pNode);
			pLink = pTmp;
		}
	}


	BOOL Remove(const DT &rElement)
	{
		FLink_t *pLink = m_LinkList.pHeadLink;
		while (pLink)
		{
			CNiNode<DT>* pNode = (CNiNode<DT>*) flinklist_GetStructurePointer(&m_LinkList, pLink);
			if (pNode->m_Data == rElement)
			{
				flinklist_Remove(&m_LinkList, pNode);
				FreeNode(pNode);
				return TRUE;
			}
			pLink = pLink->pNextLink;
		}
		return FALSE;
	}


	BOOL PushHead(const DT &rElement)
	{
		CNiNode<DT>* pNode = AllocateNode();
		if (pNode)
		{
			pNode->m_Data = rElement;
			flinklist_AddHead(&m_LinkList, pNode);
			return 1;
		}
		return 0;
	}

	
	DT* PushHead(void)
	{
		CNiNode<DT>* pNode = AllocateNode();
		if (pNode)
		{
			flinklist_AddHead(&m_LinkList, pNode);

			return &(pNode->m_Data);
		}
		return NULL;
	}

	
	BOOL PushTail(const DT &rElement)
	{
		CNiNode<DT>* pNode = AllocateNode();
		if (pNode)
		{
			pNode->m_Data = rElement;
			flinklist_AddTail(&m_LinkList, pNode);
			return 1;
		}
		return 0;
	}


	DT* PushTail(void)
	{
		CNiNode<DT>* pNode = AllocateNode();
		if (pNode)
		{
			pNode->m_Data = Element;
			flinklist_AddTail(&m_LinkList, pNode);
			return &(pNode->m_Data);
		}
		return NULL;
	}


	DT PopHead(void)
	{
		CNiNode<DT>* pNode = (CNiNode<DT>*) flinklist_RemoveHead(&m_LinkList);
		if (pNode)
		{
			DT _data = pNode->m_Data;
			FreeNode(pNode);
			return _data;
		}
		return DT(0);
	}

	
	DT PopTail(void)
	{
		CNiNode<DT>* pNode = (CNiNode<DT>*) flinklist_RemoveTail(&m_LinkList);
		if (pNode)
		{
			DT _data = pNode->m_Data;
			FreeNode(pNode);
			return _data;
		}
		return DT(0);
	}

	
	s32 Size(void) const
	{
		return (s32)m_LinkList.nCount;
	}


	CNiIterator<DT> Begin(void) { return CNiIterator<DT>(this, (CNiNode<DT>*) flinklist_GetStructurePointer(&m_LinkList, m_LinkList.pHeadLink));};

	CNiIterator<DT> End(void) { return CNiIterator<DT>(this, (CNiNode<DT>*) flinklist_GetStructurePointer(&m_LinkList, m_LinkList.pTailLink));};

	BOOL Find(const DT &rElement, CNiIterator<DT>* pIt = NULL)
	{
		FLink_t *pLink = m_LinkList.pHeadLink;
		while (pLink)
		{
			CNiNode<DT>* pNode = (CNiNode<DT>*) flinklist_GetStructurePointer(&m_LinkList, pLink);
			if (pNode->m_Data == rElement)
			{
				if (pIt)
				{
					*pIt = CNiIterator<DT>(this, pNode);
				}
				return TRUE;
			}
			pLink = pLink->pNextLink;
		}
		return FALSE;
	}


	friend class CNiIterator<DT>;

protected:
	CNiNode<DT>* AllocateNode(void)
	{
		//first try to get new node from pool
		CNiNode<DT>* pNode = NULL;
		if (m_pNodePool)
		{
			pNode = (CNiNode<DT>*) flinklist_RemoveHead(m_pNodePool);
			FASSERT(pNode);	 //node pool empty?
		}
		else
		{
			pNode = _MemAllocateNode();  //go and get one using memory
			if (pNode)
			{
				pNode->m_Link.pNextLink = NULL;
				pNode->m_Link.pPrevLink = NULL;
			}
		}

		return pNode;
	}


	void FreeNode(CNiNode<DT>* pNode)
	{
		//first try to get new node from pool
		if (m_pNodePool)
		{
			flinklist_AddHead(m_pNodePool, pNode);
		}
		else
		{
			_MemFreeNode(pNode);
		}
	}


	CNiNode<DT>* _MemAllocateNode(void)
	{
		CNiNode<DT>* pNode  = APE_NEW CNiNode<DT>();
		FASSERT(pNode);
		return pNode;
	}


	void _MemFreeNode(CNiNode<DT>* pNode)
	{
		APE_DELETE(pNode);
	}


	FLinkRoot_t m_LinkList;								//16 bytes
	FLinkRoot_t* m_pNodePool;							// 4 bytes
	FCLASS_STACKMEM_NOALIGN(CNiList<DT>);
};



//
//
//  CNiBank template
//
//
template <class DT> class CNiBank
{
public:

	CNiBank(void)
		: m_pNodePool(NULL), m_pBank(NULL), m_nBankSize(0), m_uLowWater(0)
 
	{
		CNiNode<DT*>* pTemp = AllocateNode();
		flinklist_InitRoot(&m_LinkList, (((u32) &(pTemp->m_Link)) - (u32) pTemp));
		FreeNode(pTemp); pTemp = NULL;

		//nothing in the available bank, nothing in the list
	}

	CNiBank(s32 nPreAllocateNum, DT* aBank)
		: m_pNodePool(NULL), m_pBank(NULL), m_nBankSize(nPreAllocateNum)
	{
		if (nPreAllocateNum && aBank)
		{
			m_pBank = aBank;
		}
		CNiNode<DT*>* pTemp = AllocateNode();
		flinklist_InitRoot(&m_LinkList, (((u32) &(pTemp->m_Link)) - (u32) pTemp));
		FreeNode(pTemp); pTemp = NULL;

		//initialize the linklist.
		//it contains pointers to available members in the bank
		for (s32 i = 0; i < nPreAllocateNum; i++)
		{  //push head
			CNiNode<DT*>* pNode = AllocateNode();
			if (pNode)
			{
				pNode->m_Data = m_pBank+i;
				flinklist_AddHead(&m_LinkList, pNode);
			}
		}
		m_uLowWater = m_LinkList.nCount;
	}

			
	CNiBank(FLinkRoot_t* pNodePool, s32 nPreAllocateNum, DT* aBank)
		: m_pNodePool(pNodePool), m_pBank(NULL), m_nBankSize(nPreAllocateNum)
	{
		if (nPreAllocateNum)
		{
			m_pBank = aBank;
		}

		CNiNode<DT*>* pTemp = AllocateNode();
		flinklist_InitRoot(&m_LinkList, (((u32) &(pTemp->m_Link)) - (u32) pTemp));
		FreeNode(pTemp); pTemp = NULL;
	
		//initialize the linklist.
		//it contains pointers to available members in the bank
		for (s32 i = 0; i < nPreAllocateNum; i++)
		{  //push head
			CNiNode<DT*>* pNode = AllocateNode();
			if (pNode)
			{
				pNode->m_Data = m_pBank+i;
				flinklist_AddHead(&m_LinkList, pNode);
			}
		}
		m_uLowWater = m_LinkList.nCount;
	}


	~CNiBank(void)
	{
		FLink_t *pLink = m_LinkList.pHeadLink;
		while (pLink)
		{
			FLink_t *pTmp = pLink->pNextLink;
			CNiNode<DT*>* pNode = (CNiNode<DT*>*) flinklist_Remove(&m_LinkList, flinklist_GetStructurePointer(&m_LinkList, pLink));
			if (!m_pBank)  //only delete if there isn't a bank
			{
				APE_DELETE(pNode->m_Data);
			}

			FreeNode(pNode);
			pLink = pTmp;
		}

		APE_ARRAYDELETE(m_pBank); m_pBank = NULL;
	}


	DT* Get(void)
	{
		CNiNode<DT*>* pNode = (CNiNode<DT*>*) flinklist_RemoveHead(&m_LinkList);
		if (pNode)
		{
			DT* pData = pNode->m_Data;
			FreeNode(pNode);
			if (m_LinkList.nCount < m_uLowWater)
			{
				m_uLowWater = m_LinkList.nCount;
			}
			return pData;
		}
		else if (!m_pBank)
		{  //o.k. to make one now since we're not running in prealloc mode
			DT* pNew = APE_NEW DT;
			m_nBankSize++;
			return pNew;
		}
		return NULL;
	}


	void Recycle(DT* pElement)
	{
		CNiNode<DT*>* pNode = AllocateNode();
		if (pNode)
		{
			pNode->m_Data = pElement;
			flinklist_AddHead(&m_LinkList, pNode);
		}
		else if (!m_pBank)
		{
			APE_DELETE(pElement);
		}
	}

	s32 Size(void) const
	{
		return m_LinkList.nCount;
	}

	BOOL IsFull(void) const
	{
		return (m_LinkList.nCount == m_nBankSize);
	}


protected:
	CNiNode<DT*>* AllocateNode(void)
	{
		//first try to get new node from pool
		CNiNode<DT*>* pNode = NULL;
		if (m_pNodePool)
		{
			pNode = (CNiNode<DT*>*) flinklist_RemoveHead(m_pNodePool);
		}
		else
		{
			pNode = _MemAllocateNode();  //go and get one using memory
		}
		FASSERT(pNode);
		pNode->m_Link.pNextLink = NULL;
		pNode->m_Link.pPrevLink = NULL;

		return pNode;
	}


	void FreeNode(CNiNode<DT*>* pNode)
	{
		//first try to get new node from pool
		if (m_pNodePool)
		{
			flinklist_AddHead(m_pNodePool, pNode);
		}
		else
		{
			_MemFreeNode(pNode);
		}
	}


	CNiNode<DT*>* _MemAllocateNode(void)
	{
		CNiNode<DT*>* pNode = APE_NEW CNiNode<DT*>();
		FASSERT(pNode);
		return pNode;
	}


	void _MemFreeNode(CNiNode<DT*>* pNode)
	{
		APE_DELETE(pNode);
	}


protected:
	DT* m_pBank;
	u32 m_nBankSize;
	u32 m_uLowWater;

	FLinkRoot_t m_LinkList;
	FLinkRoot_t* m_pNodePool;
	FCLASS_STACKMEM_NOALIGN(CNiBank<DT>);
};


enum CNiLockRetVal_e
{
	CNiLOCKRETVAL_REQUEST_DENIED = 0,
	CNiLOCKRETVAL_LOCK_ACQUIRED,
	CNiLOCKRETVAL_REQUEST_ADDED_TO_QUEUE,
};


//DT class must implement
//public: DT(const <DT>&); //copy constructor
//public: inline u32 GetGUID(void);
//public: inline BOOL CompareResource( const <DT>& ) const;
//public: inline BOOL operator == ( const CMechSeatLock& v );


template <class DT> class CNiLocker
{
public:
	CNiLocker(void)
		: m_pLockList(NULL), m_pLockBank(NULL)
	{
	}

	
	CNiLocker(CNiList<DT*>* pLockList, CNiBank<DT>* pLockBank)
		: m_pLockList(pLockList), m_pLockBank(pLockBank)
	{
	}

	BOOL FreeLock(const DT& LookFor)
	{
		CNiIterator<DT*> it = m_pLockList->Begin();

		while (it.IsValid())
		{	
			DT* pLock = it.Get();
			if (*pLock == LookFor)
			{	//found it
				it.RemoveThenNext();
				m_pLockBank->Recycle(pLock);  //put back in bank
				return TRUE; //found and removed 
			}
			it.Next();
		}
		return FALSE;	  //didn't find edgelock like that one
	}


	BOOL FreeAllLocks(u32 uGUID)
	{
		BOOL bDidOne = FALSE;
		CNiIterator<DT*> it = m_pLockList->Begin();

		while (it.IsValid())
		{	
			DT* pLock = it.Get();
			if (pLock->GetGUID() == uGUID)
			{	//found one
				it.RemoveThenNext();
				m_pLockBank->Recycle(pLock);  //put back in bank
				bDidOne = TRUE;
			}
			else
			{
				it.Next();
			}
		}
		return bDidOne;
	}

	CNiLockRetVal_e RequestLock(const DT& LookFor, BOOL bGetInLine)
	{
		CNiIterator<DT*> it = m_pLockList->Begin();
		DT* pLock =NULL;
		while (it.IsValid())
		{	
			pLock = it.Get();
			if (LookFor.CompareGUID(*pLock))
			{	//found it
				return CNiLOCKRETVAL_LOCK_ACQUIRED;
			}
			else if (!bGetInLine && pLock->CompareResource(LookFor))
			{
				return CNiLOCKRETVAL_REQUEST_DENIED;
			}
			it.Next();
		}
		//nope, request is not already in the list
		//add it, in order.
		pLock = m_pLockBank->Get();
		if (pLock)
		{
			*pLock = LookFor;	 //copy constructor
			BOOL bFoundFirst = 0;
			it = m_pLockList->Begin();
			while (it.IsValid())
			{	
				DT* pOldLock = it.Get();
				if (!bFoundFirst && pLock->CompareResource(LookFor))
				{	//found first
					bFoundFirst	= TRUE;
					it.Next();
				}
				else if (bFoundFirst && !pLock->CompareResource(LookFor))
				{	//one past last
					it.InsertBefore(pLock);
					return CNiLOCKRETVAL_REQUEST_ADDED_TO_QUEUE;
				}
				else
				{
					it.Next();
				}
			}
			//none of kind in list!
			m_pLockList->PushTail(pLock);
		}
		else
		{
			return CNiLOCKRETVAL_REQUEST_DENIED;
		}

		return CNiLOCKRETVAL_LOCK_ACQUIRED;
	}		
	

	BOOL IsLocked(const DT& LookFor, u32* puWhoGUID) //Is it locked by anyone? If so, who
	{
		CNiIterator<DT*> it = m_pLockList->Begin();
		while (it.IsValid())
		{	
			DT* pLock = it.Get();
			if (pLock->CompareResource(LookFor))
			{	//found it
				if (puWhoGUID)
				{
					*puWhoGUID = pLock->m_uGUID;
				}
				return TRUE;
			}
			else
			{
				it.Next();
			}
		}
		return FALSE;
	}


	BOOL HasLocked(const DT& LookFor, u16 *puPlaceInQueue = NULL)	  //Is it locked by GUID specified in LookFor?
	{
		CNiIterator<DT*> it = m_pLockList->Begin();
		u16 nCount = 0;
		while (it.IsValid())
		{	
			DT* pLock = it.Get();
			if	(*pLock == LookFor)
			{	//found it
				if (puPlaceInQueue)
				{
					*puPlaceInQueue = nCount;
				}
				return TRUE;
			}
			else 
			{
				nCount++;
			}
			it.Next();
		}
		return FALSE;
	}

	BOOL HasAnyLocked(const DT& LookFor, u16 *puPlaceInQueue = NULL)  //Is any resource locked by GUID specified in LookFor?
	{
		CNiIterator<DT*> it = m_pLockList->Begin();
		u16 nCount = 0;
		while (it.IsValid())
		{	
			DT* pLock = it.Get();
			if	(LookFor.CompareGUID(*pLock))
			{	//found it
				if (puPlaceInQueue)
				{
					*puPlaceInQueue = nCount;
				}
				return TRUE;
			}
			else 
			{
				nCount++;
			}
			it.Next();
		}
		return FALSE;
	}

protected:
	CNiList<DT*>* m_pLockList;
	CNiBank<DT>* m_pLockBank;
	FCLASS_STACKMEM_NOALIGN(CNiLocker<DT>);
};


#endif