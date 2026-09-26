//////////////////////////////////////////////////////////////////////////////////////
// flightgroup.h - Fang light grouping module.
//
// Author: Michael Starich
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
// 03/26/03 Starich     Created.
//////////////////////////////////////////////////////////////////////////////////////
#ifndef _FLIGHT_GROUP_H_
#define _FLIGHT_GROUP_H_ 1

#include "fang.h" 
#include "flight.h"
#include "flinklist.h"

// forward declarations
class CFLightGroup;
class CFVirtualWorldLight;



/////////////////////////////////////////////////////////////////////////////////////
// The light group manager gathers virtual lights and distributes them to LightGroups
FCLASS_NOALIGN_PREFIX class CFLightGroupMgr
{
public:
	static BOOL ModuleStartup();
	static void ModuleShutdown();
	
	static void ReleaseAllLights();
	static CFVirtualWorldLight *GetVirtualLight( FLightInit_t *pInit );
	static void ReleaseVirtualLight( CFVirtualWorldLight *pLight );
	// must be called once per frame
	static void Work( void );

	// mainly used by tools, returns the number of real lights currently in the world.
	// FDRAW MUST BE THE ACTIVE RENDERER
	static u32 DrawAllLightBoundingSpheres( CFColorRGBA &rRealLightColor,
											CFColorRGBA &rVirtualLightColor,
											u32 &rnNumVirtualLights );
	static void GetCounts( u32 &rnNumRealLights, u32 &rnVirtualLights );

protected:
		
private:
	// scans all of the current Active LGs and tries to find one that will take pVL
	// Returns TRUE if a LG was found and pVL should be removed from the pending list
	// FALSE if no home could be found and pVL should be the start of a new LG
	static BOOL FindExistingLGforVL( CFVirtualWorldLight *pVL );

	static FINLINE void AddToPendingList( CFVirtualWorldLight *pVL )		{ flinklist_AddTail( &m_PendingVLs, pVL ); }
	static FINLINE void RemoveFromPendingList( CFVirtualWorldLight *pVL )	{ flinklist_Remove( &m_PendingVLs, pVL ); } 		

	static FINLINE CFVirtualWorldLight *GetFreeVL()							{ return (CFVirtualWorldLight *)flinklist_RemoveHead( &m_VLPool ); }
	static FINLINE void AddToFreePool( CFVirtualWorldLight *pVL )			{ flinklist_AddTail( &m_VLPool, pVL ); }
	static FINLINE void RemoveFromFreePool( CFVirtualWorldLight *pVL )		{ flinklist_Remove( &m_VLPool, pVL ); }

	static FINLINE void AddToActiveList( CFLightGroup *pLG )				{ flinklist_AddTail( &m_ActiveLGList, pLG ); }
	static FINLINE void RemoveFromActiveList( CFLightGroup *pLG )			{ flinklist_Remove( &m_ActiveLGList, pLG ); }

	static FINLINE CFLightGroup *GetFreeLG()								{ return (CFLightGroup *)flinklist_RemoveHead( &m_LGPool ); }
	static FINLINE void AddToFreePool( CFLightGroup *pLG )					{ flinklist_AddTail( &m_LGPool, pLG ); }
	static FINLINE void RemoveFromFreePool( CFLightGroup *pLG )				{ flinklist_Remove( &m_LGPool, pLG ); }

	static BOOL m_bSystemOK;
	static f32 m_fLightRadiusRemapper;	// use this value to scale the radius down before calling light init (which makes the radius larger for omni for some reason)
	static u32 m_nNumVLs;
	static CFVirtualWorldLight *m_paVLs;	// m_nNumVLs of them

	static u32 m_nNumLGs;
	static CFLightGroup *m_paLGs;	// m_nNumLGs of them

    static FLinkRoot_t m_VLPool;	// unused VLs
	static FLinkRoot_t m_LGPool;	// unused LGs

	static FLinkRoot_t m_PendingVLs;	// VLs that need to be assigned to LGs
	static FLinkRoot_t m_ActiveLGList;	// the LGs that are currently in use

	friend class CFLightGroup;
	friend class CFVirtualWorldLight;

	FCLASS_STACKMEM_NOALIGN( CFLightGroupMgr );
} FCLASS_NOALIGN_SUFFIX;


///////////////////////////////////////////////////////////////////////////////////////////////////////////
// A virtual light is a light that can be grouped with other similar lights to reduce the 
// actual number of real lights in the world.
// This system only handles omni lights, and all properties (cornona, cornona texture, etc) are honored
FCLASS_NOALIGN_PREFIX class CFVirtualWorldLight
{
public:
	// Use the Update... functions to just copy the value into the class and not have it auto update each frame
	// Use the Set... functions to set a ptr to the data and allow the system to update each frame automatically
	// Use the Get... functions to copy the current value
	void UpdateIntensity( f32 fIntensity );
	void SetIntensityPtr( f32 *pfIntensity );
	const f32 *GetCurrentIntensity() const;

	void UpdateColor( const CFColorRGBA &rRGBA );
	void UpdataColor( const CFColorRGB &rRGB );
	void SetColorPtr( CFColorRGBA *pRGBA );
	const CFColorRGBA *GetCurrentColor() const;

	void UpdatePos( const CFVec3 &rPos );
	void SetPosPtr( CFVec3 *pPos );
	const CFVec3 *GetCurrentPos() const;

	void UpdateRadius( f32 fRadius );
	void SetRadiusPtr( f32 *pfRadius );
	const f32 *GetCurrentRadius() const;	

	void DrawBoundingSphere( CFColorRGBA &rColor );

protected:
		
private:
	CFVirtualWorldLight();
	BOOL Init( FLightInit_t *pInit );
	void FillLightInit( FLightInit_t &rInit );
	BOOL DoesLightInitMatchUs( const FLightInit_t &rInit ) const;

	typedef enum {
		STATE_AVAILABLE = 0x02040608,
		STATE_PENDING	= 0x10203040,
		STATE_ASSIGNED	= 0x98765432
	} State_e;

	u32 m_nState;// one of State_e

	// elements that can be altered after the creation of the virtual light
	f32 m_fIntensity;									// IF MERGED, WILL BE USED IN COMPUTING THE REAL LIGHT VALUE
	f32 *m_pfIntensity;									// IF MERGED, WILL BE USED IN COMPUTING THE REAL LIGHT VALUE
	CFColorRGBA m_RGBA;									// NEEDS TO BE CLOSE TO OTHER VLs TO BE MERGED
    CFColorRGBA *m_pRGBA;								// NEEDS TO BE CLOSE TO OTHER VLs TO BE MERGED
	CFVec3 m_Pos;										// NEEDS TO BE CLOSE TO OTHER VLs TO BE MERGED
	CFVec3 *m_pPos;										// NEEDS TO BE CLOSE TO OTHER VLs TO BE MERGED	
	f32 m_fRadius;										// IF MERGED, WILL BE USED IN COMPUTING THE REAL LIGHT VALUE
	f32 *m_pfRadius;									// IF MERGED, WILL BE USED IN COMPUTING THE REAL LIGHT VALUE		

	// elements that are just copied from the light init struct passed in during Init()
	char m_szCoronaTexName[FLIGHT_TEXTURE_NAME_LEN + 1];// MUST MATCH OTHER VLs TO BE MERGED
	u32 m_nInitFlags;									// MUST MATCH OTHER VLs TO BE MERGED		
	u32 m_nMotifIndex;									// MUST MATCH OTHER VLs TO BE MERGED		
	f32 m_fCoronaScale;

	CFLightGroup *m_pLightGroup;// what light group are we currently in (if NULL this Virtual Light is not in use)

	FLink_t m_ManagerLink;		// used for a being a member of various mgr linked lists
	FLink_t m_LightGroupLink;	// used by the the light group

	friend class CFLightGroup;
	friend class CFLightGroupMgr;

	FCLASS_STACKMEM_NOALIGN( CFVirtualWorldLight );
} FCLASS_NOALIGN_SUFFIX;


#endif