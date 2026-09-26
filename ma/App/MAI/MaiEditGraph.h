//////////////////////////////////////////////////////////////////////////////////////
// EditGraph.h - All kinds o goodied for editing and creating an AI Graph
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
// 02/06/02 patm       Created.
//////////////////////////////////////////////////////////////////////////////////////

#ifndef _MAIEDITGRAPH_H_
#define _MAIEDITGRAPH_H_ 1


#include "AI\AiGraph.h"
#include "AI\ftl.h"
#include "fmath.h"

class CEditVert
{
public:
	CEditVert(void);

	// edit vert properties
	enum 
	{
		EV_ALLOCATED	= 0x00000001,
		EV_SELECTED		= 0x00000002,
		EV_ONHOTPATH	= 0x00000004,
	};

	u32 m_uEVProps;  //properties of this edit vert
	s32 m_nParentVertId;
	CFVec3 m_ParentOffset;
};

FCLASS_ALIGN_PREFIX class CEditGraph : public CAIGraph
{
public:
	CEditGraph(void); 
private:
	CEditGraph(s32 nPreAllocVerts, void* pMemory);  //bad constructor for this child class
public:

	virtual ~CEditGraph(void);
	
	//virtual overrides
	virtual void Render(void);
	virtual GraphVert *AllocVertSlot(void);
	virtual void FreeVertSlot(s32 nSlotId);
	virtual u32 GetSizeOfBinaryData(void);

	s32 FindClosestEdge(const CFVec3 &RayOrigin, const CFVec3 &RayDir, f32 fMinDistance, GraphEdge** ppEdge);

	//
	//
	//
	void SaveToMemory(void* pMem);
	void ResetFromMemory(void* pMem);
	void AutoLinkSelected(void);
	void UnLinkSelected(void);
	void TestEdgesWalkability(GraphVert* pV = NULL);   //pass a valid PV to test just the links of one vert, null to test all
	
	BOOL IsSelected(GraphVert* pVert);
	u16  GetNthSelected(s32 nN);
	void DeleteSelected(void);
	void ClearSelected(void);
	s32 GetNumSelected(void)						{ return m_SelectedVerts.Size();};
	BOOL ToggleSelectVert(GraphVert* pVert);  //if selected, this vert will become unselected, if not, it will be selected
	void SelectVert(GraphVert* pVert);
	void UnSelectVert(GraphVert* pVert);
	void RecordSelectedOffsets(GraphVert* pRoot);
	void ApplySelectedOffsets(GraphVert* pRoot);
	void LockSelection(void) {m_bSelectionLocked = TRUE;};
	void UnlockSelection(void) {m_bSelectionLocked = FALSE;};
	BOOL IsLocked(void) {return m_bSelectionLocked;};

	void ClearPropFromAllVerts(u32 nProps);  //bits in nProps will be cleared from all vert props
	void SetVertProperty(s32 nVertId, u32 nProps);  //bits in nProps will be or'd into this verts m_nEVProps field
	BOOL IsNear(GraphVert* pV);
	BOOL IsNear(GraphEdge* pEdge);

	CEditVert *GetEditVert(s32 nVertId) { FASSERT(nVertId > 0 && nVertId < GetNumVerts()); return m_paEditVerts + nVertId;};

//data

	BOOL m_bSelectionLocked;

	CNiList<s32> m_SelectedVerts;
	CNiList<CFVec3> m_SelectedOffsets;

	CEditVert *m_paEditVerts;
	u32 *m_panRenderPropTmp;			   //used to pass data to CAIGraph::Render
	u32 *m_panEdgeRenderPropTmp;
	s32 m_nMaxEditVerts;

	BOOL m_b3DMode;
	BOOL m_b2DMode;
	BOOL m_bDrawVertVolumes;
	BOOL m_bDrawEdgeVolumes;
	BOOL m_bDrawVerts;
	enum
	{
		DRAWVOLMODE_ALL = 0,
		DRAWVOLMODE_NEAR,
		DRAWVOLMODE_CUSTOM,
	};
	s32	 m_nDrawVolMode;
	f32  m_fVertVisualRadius;

	FCLASS_STACKMEM_ALIGN(CEditGraph);
} FCLASS_ALIGN_SUFFIX;

#endif