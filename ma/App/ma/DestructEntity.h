//////////////////////////////////////////////////////////////////////////////////////
// DestructEntity.h - Destructable entity object.
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
// 04/26/02	Link		Created.
//////////////////////////////////////////////////////////////////////////////////////
#ifndef _DESTRUCTENTITY_H_
#define _DESTRUCTENTITY_H_ 1

#if 0
#include "fang.h"
#include "meshentity.h"


class CFAudioEmitter;

FCLASS_ALIGN_PREFIX class CDestructEntity : public CMeshEntity
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

	virtual BOOL CheckpointSave( void );
	virtual void CheckpointRestore( void );


	/////////////////////////////////////////////////////////////
	// Definitions.
	// These are general flags that apply to the entity as a whole.
	enum GeneralFlags_e
	{
		GENERALFLAG_LASTMESHISDEAD	=	1 << 0,		// The last mesh in the sequence is the
													//   mesh to use when the entity is dead.
		GENERALFLAG_ONEHITKILL		=	1 << 1
	};

	// These are flags pertaining to the behaviour of the entity when
	//   it gets hit.
	enum DamageFlags_e
	{
		DAMAGEFLAG_PULSATE			=	1 << 0,		// m_fPulsScale, m_fPulseTime,
		DAMAGEFLAG_EXPLOSION		=	1 << 1,		// m_fExpIntens,
		DAMAGEFLAG_DEBRIS			=	1 << 2,		//
		DAMAGEFLAG_SHAKE			=	1 << 3,
	};

	// These are flags pertaining to the behaviour of the entity when
	//   it dies.
	enum DieFlages_e
	{
		DIEFLAG_DISAPPEAR	=	1 << 0, // Object will disappear when destroyed.
		DIEFLAG_EXPLODE		=	1 << 1, // Explosion(s) will be spawned when destroyed.
	};

	enum State_e
	{
		STATE_ALIVE,
		STATE_DYING,
		STATE_DEAD
	};
	//
	/////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////
	// These variables change normally across the lifetime of
	//   the object.
	FCLASS_NOALIGN_PREFIX class CEDestructState
	{
	public:
		State_e m_eState;
		f32 m_fShakeCtdown;

		FCLASS_STACKMEM_NOALIGN(CEDestructState);
	} FCLASS_NOALIGN_SUFFIX;
	//
	/////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////
	// These variables are generally set at init time and stay
	//   put for the lifetime of the object.
	FCLASS_NOALIGN_PREFIX class CEDestructConfig
	{
	public:
		u32 m_uTotMeshCnt, m_uNormMeshCnt;

		u32 m_uDamageFlags, m_uDieFlags;
		f32 m_fShakeDur, m_fShakeIntens, m_fOOShakeDur;

		FCLASS_STACKMEM_NOALIGN(CEDestructConfig);
	} FCLASS_NOALIGN_SUFFIX;
	//
	/////////////////////////////////////////////////////////////


	/////////////////////////////////////////////////////////////
	// Constructor/destructor.
	CDestructEntity();
	virtual ~CDestructEntity();
	//
	/////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////
	// Creation/destruction.
	BOOL Create();			// Boring, dumb create used for debugging purposes.

	// This is the most general Create().  The others below are convenient simplifications of this one.
	BOOL Create(u32 uMeshSeqLen, cchar **apszMeshSeq, BOOL bLastIsDeadMesh, cchar *pszEntityName, CFMtx43A *pMtx);

	// This Create() is used for a simple object that merely changes meshes when killed.
	BOOL Create(cchar *pszMeshAlive, cchar *pszMeshDead, cchar *pszEntityName, CFMtx43A *pMtx);
	//
	/////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////
	// Commands.
	void Revive();
	BOOL StartDestruction( void );
	//
	/////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////
	// State querying.
	State_e GetState() { return(m_eDEState.m_eState); }
	//
	/////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////
	//
	enum DebrisType_e
	{
		DEBRISTYPE_NONE		=	0,
		DEBRISTYPE_DEFAULT,
		DEBRISTYPE_CRYSTALBLUE,
		DEBRISTYPE_CRYSTALPINK,
		DEBRISTYPE_CRYSTALGREEN,
		DEBRISTYPE_CRYSTALTEAL,

		DEBRISTYPE_COUNT
	};
	//
	/////////////////////////////////////////////////////////////

	static FMesh_t **m_papDebrisMeshes[DEBRISTYPE_COUNT];
	static cchar **m_apszDebrisMeshNames[DEBRISTYPE_COUNT];
	static const u32 m_auDebrisMeshCounts[DEBRISTYPE_COUNT];

protected:
	virtual void ClassHierarchyDestroy();
	virtual BOOL ClassHierarchyBuild();
	virtual BOOL ClassHierarchyBuilt( void );
	virtual CEntityBuilder *GetLeafClassBuilder();

	virtual void ClassHierarchyResolveEntityPointerFixups();
	virtual void ClassHierarchyWork();

	virtual void InflictDamage( CDamageData *pDamageData );

private:
	void Clear();
	void _SetDefaults();

	void _StartDie();

	void StateTransitions();
	void StateWork();

	void ShakeStart();
	void ShakeWork();
	void ShakeEnd();

	void PulsateStart();
	void PulsateWork();
	void PulsateEnd();

	void FindExplosionBoneIndices();

	static BOOL BotCheckCallback(CFWorldTracker *pTracker, FVisVolume_t *pVolume );

	static BOOL m_bSystemInitialized;
	static s32 m_nDestructEvent;

	DebrisType_e m_eDieDebris;
	f32 m_fDieDebrisVel;

	CEDestructState m_eDEState;
	CEDestructConfig m_eDEConfig;

	f32 m_fDyingTimer;
	f32 m_fNextExplode;
	f32 m_fMeshSwitchInc;
	f32 m_fNextMeshSwitch;
	CFAudioEmitter *m_pAEDestruct;
	f32 m_fDestructVolume;

	FCLASS_STACKMEM_ALIGN(CDestructEntity);
} FCLASS_ALIGN_SUFFIX;



FCLASS_ALIGN_PREFIX class CEDestructBuilder : public CMeshEntityBuilder
{
public:
	FINLINE CEDestructBuilder() {}

	virtual void SetDefaults(u64 nEntityTypeBits, u64 nEntityLeafTypeBit, cchar *pszEntityType);

	u32 m_uDamageFlags;
	u32 m_uDieFlags;
	CDestructEntity::DebrisType_e m_eDieDebris;
	f32 m_fDieDebrisVel;

	cchar *m_pszDestructBank;
	cchar *m_pszDestructWave;
	f32 m_fDestructRadius;
	f32 m_fDestructVolume;
	BOOL m_bLastMeshIsDeadMesh;

protected:
	virtual BOOL InterpretTable();
	virtual BOOL PostInterpretFixup();

	FCLASS_STACKMEM_ALIGN(CEDestructBuilder);
} FCLASS_ALIGN_SUFFIX;

#endif
#endif