//////////////////////////////////////////////////////////////////////////////////////
// flightgroup.cpp - Fang light grouping module.
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
#include "fang.h"
#include "flightgroup.h"
#include "flightpool.h"
#include "fclib.h"
#include "fdraw.h"
#include "frenderer.h"
#include "floop.h"

#define _COLOR_TOLERANCE_SQUARED		( 0.20f * 0.20f )

////////////////////////////////////////////////////////////////////////////////////////
// The Light Group is a collection of virtual lights that will use exactly 1 world light
FCLASS_NOALIGN_PREFIX class CFLightGroup
{
public:
	CFLightGroup();
	~CFLightGroup();
	
	// returns TRUE if pLight fit the Groups criteria, and was added
	// FALSE if pLight could not be added to the group
	BOOL AddVL( CFVirtualWorldLight *pVL );

	// removes pLight from the group, will add it back to the manager's free pool
	// returns TRUE if the group still has members after the removal, FALSE if is now empty
	BOOL RemoveVL( CFVirtualWorldLight *pLight );

	// removes all VLs from the group.  At this point the group is ready for removal from the manager's active lists.
	void RemoveAll();

	// called once per frame before Work() so that the group can kick out any members 
	// who no longer fit the into the group.
	// Returns TRUE if the group still has members, FALSE if it is empty
	BOOL PreWork();

	// returns TRUE if this group is still active and contains members and is emitting a light
	// returns FALSE if the group is now empty and can be removed from the manager's active list
	BOOL Work();

	// mainly used by tools, returns the number of real lights currently in the world.
	// FDRAW MUST BE THE ACTIVE RENDERER
	u32 DrawAllLightBoundingSpheres( CFColorRGBA &rRealLightColor,
									 CFColorRGBA &rVirtualLightColor,
									 u32 &rnNumVirtualLights );
	u32 GetCounts( u32 &rnVirtualLights );
	
protected:
		
private:
	static BOOL AreColorsCloseEnough( const CFColorRGBA *pC1, const CFColorRGBA *pC2 );
	static BOOL DoesVLBelongInGroup( const CFVirtualWorldLight *pVL, const CFLight *pLight );
	static BOOL DoesVLBelongInGroup( const CFVirtualWorldLight *pVL1, const CFVirtualWorldLight *pVL2 );
	static void ConfigureLightToVLSettings( const CFVirtualWorldLight *pVL, CFWorldLight *pWorldLight );
	// will kick a valid VL out of our group and put them back into the Mgr's pending list
	void KickOutVL( CFVirtualWorldLight *pVL );
	void CalculateDesiredLightSettingsFromVLs( CFVec3 &rPos, f32 &rfRadius, CFColorRGBA &rColor, f32 &rfIntensity );

	enum {
		_FLAGS_ALREADY_UPDATED = 0x00000001,

		_FLAGS_NONE = 0
	};
	u32 m_nFlags;// see _FLAGS_...
	CFWorldLightItem *m_pWorldLight;// the real world light, from the light pool

    FLinkRoot_t m_VLs;			// a list of all VLs (The first item will be the largest radius)
	FLink_t m_ManagerLink;		// used for a being a member of various mgr linked lists

	friend class CFLightGroupMgr;
	
	FCLASS_STACKMEM_NOALIGN( CFLightGroup );
} FCLASS_NOALIGN_SUFFIX;


///////////////////////////////////////////////////
// CFLightGroup

CFLightGroup::CFLightGroup() {

	flinklist_InitRoot( &m_VLs, FANG_OFFSETOF( CFVirtualWorldLight, m_LightGroupLink ) );
	m_pWorldLight = NULL;
	m_nFlags = _FLAGS_NONE;
}

CFLightGroup::~CFLightGroup() {

	if( m_pWorldLight ) {
		FLightPool_pPool->ReturnToFreePool( m_pWorldLight );
		m_pWorldLight = NULL;
	}
}

// returns TRUE if pLight fit the Groups criteria, and was added
// FALSE if pLight could not be added to the group
BOOL CFLightGroup::AddVL( CFVirtualWorldLight *pVL ) {
	FASSERT( pVL->m_nState == CFVirtualWorldLight::STATE_PENDING );
	FASSERT( !pVL->m_pLightGroup );

	CFVirtualWorldLight *pFirstVL = (CFVirtualWorldLight *)flinklist_GetHead( &m_VLs );

	if( m_pWorldLight ) {
		///////////////////////////////////////////////////////////////////
		// see if pLight is configured the same as the existing world light
		FASSERT( pFirstVL );

		if( pFirstVL->m_nMotifIndex != pVL->m_nMotifIndex ||
			pFirstVL->m_nInitFlags != pVL->m_nInitFlags ||
			fclib_stricmp( pFirstVL->m_szCoronaTexName, pVL->m_szCoronaTexName ) != 0 ) {
			// some key elements didn't match
			return FALSE;
		}

		CFLight *pLight = &m_pWorldLight->m_Light;

		if( !DoesVLBelongInGroup( pVL, pLight ) ) {
			return FALSE;
		}

		// add to the group
		if( *pVL->m_pfRadius > pLight->m_spInfluence_WS.m_fRadius ) {
			// this is our new largest VL
			flinklist_AddHead( &m_VLs, pVL );
		} else {
			flinklist_AddTail( &m_VLs, pVL );
		}
		
	} else if( pFirstVL ) {
		//////////////////////////////////////////////////////////////////////////////////////////////////////
		// this must be a newly formed light group, cause there is already a VL in our list but no real light
		// see if we match the current largest VL that will be used to form the light
		if( pFirstVL->m_nMotifIndex != pVL->m_nMotifIndex ||
			pFirstVL->m_nInitFlags != pVL->m_nInitFlags ||
			fclib_stricmp( pFirstVL->m_szCoronaTexName, pVL->m_szCoronaTexName ) != 0 ) {
			// some key elements didn't match
			return FALSE;
		}

		if( !DoesVLBelongInGroup( pVL, pFirstVL ) ) {
			return FALSE;
		}

		// add to the group
		if( *pVL->m_pfRadius > *pFirstVL->m_pfRadius ) {
			// this is our new largest VL
			flinklist_AddHead( &m_VLs, pVL );
		} else {
			flinklist_AddTail( &m_VLs, pVL );
		}		

	} else {
		////////////////////
		// start a new group
		flinklist_AddHead( &m_VLs, pVL );		
	}

	// if we get here, we have added pVL light, update some settings in the VL and flag ourself that we need to recalculate the real light
	pVL->m_nState = CFVirtualWorldLight::STATE_ASSIGNED;
	pVL->m_pLightGroup = this;

	m_nFlags &= ~_FLAGS_ALREADY_UPDATED;

	return TRUE;
}

// removes pLight from the group, will add it back to the manager's free pool
// returns TRUE if the group still has members after the removal, FALSE if is now empty
BOOL CFLightGroup::RemoveVL( CFVirtualWorldLight *pLight ) {
	FASSERT( pLight->m_pLightGroup == this );
	FASSERT( pLight->m_nState == CFVirtualWorldLight::STATE_ASSIGNED );

	flinklist_Remove( &m_VLs, pLight );
	pLight->m_pLightGroup = NULL;
	pLight->m_nState = CFVirtualWorldLight::STATE_AVAILABLE;
	
	// add back to th manager's free pool
	CFLightGroupMgr::AddToFreePool( pLight );

	if( !m_VLs.nCount ) {
		if( m_pWorldLight ) {
			FLightPool_pPool->ReturnToFreePool( m_pWorldLight );
			m_pWorldLight = NULL;
		}
		return FALSE;
	}
	m_nFlags &= ~_FLAGS_ALREADY_UPDATED;

	return TRUE;
}

// removes all VLs from the group.  At this point the group is ready for removal from the manager's active lists.
void CFLightGroup::RemoveAll() {

	CFVirtualWorldLight *pLight, *pNext;
	pLight = (CFVirtualWorldLight *)flinklist_GetHead( &m_VLs );
	while( pLight ) {
		pNext = (CFVirtualWorldLight *)flinklist_GetNext( &m_VLs, pLight );

		RemoveVL( pLight );

		pLight = pNext;
	}

	if( m_pWorldLight ) {
		FLightPool_pPool->ReturnToFreePool( m_pWorldLight );
		m_pWorldLight = NULL;
	}
}

// called once per frame before Work() so that the group can kick out any members 
// who no longer fit the into the group.
// Returns TRUE if the group still has members, FALSE if it is empty
BOOL CFLightGroup::PreWork() {

	if( !m_VLs.nCount ) {
		if( m_pWorldLight ) {
			FLightPool_pPool->ReturnToFreePool( m_pWorldLight );
			m_pWorldLight = NULL;
		}
		return FALSE;
	}

	// find the largest VL and put it add the head of our VL list
	CFVirtualWorldLight *pNextVL, *pVL, *pLargestVL;
	pLargestVL = (CFVirtualWorldLight *)flinklist_GetHead( &m_VLs );
	if( m_VLs.nCount > 1 ) {
		pVL = (CFVirtualWorldLight *)flinklist_GetNext( &m_VLs, pLargestVL );
		BOOL bNewLargestVL = FALSE;
		while( pVL ) {
			if( *pVL->m_pfRadius > *pLargestVL->m_pfRadius ) {
				bNewLargestVL = TRUE;
				pLargestVL = pVL;
			}
			pVL = (CFVirtualWorldLight *)flinklist_GetNext( &m_VLs, pVL );
		}
		if( bNewLargestVL ) {
			flinklist_Remove( &m_VLs, pLargestVL );
			flinklist_AddHead( &m_VLs, pLargestVL );
		}

		if( !m_pWorldLight ) {
			////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			// we don't have a real light yet, but we still need to make sure that all of the VLs are valid members of the group
			pVL = (CFVirtualWorldLight *)flinklist_GetNext( &m_VLs, pLargestVL );
			while( pVL ) {
				pNextVL = (CFVirtualWorldLight *)flinklist_GetNext( &m_VLs, pVL );

				// make sure that pVL still belongs in the group
				if( !DoesVLBelongInGroup( pVL, pLargestVL ) ) {
					// remove pVL from this group
					KickOutVL( pVL );	
				}
				// the get the previous VL
				pVL = pNextVL;
			}
		} else {
			////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			// walk the VLs (in reverse order) to see if anyone can be kicked out because they no longer fit into the group
			pVL = (CFVirtualWorldLight *)flinklist_GetTail( &m_VLs );
			while( pVL ) {
				pNextVL = (CFVirtualWorldLight *)flinklist_GetPrev( &m_VLs, pVL );

				// make sure that pVL still belongs in the group
				if( !DoesVLBelongInGroup( pVL, &m_pWorldLight->m_Light ) ) {
					if( m_VLs.nCount > 1 ) {
						// remove pVL from this group
						KickOutVL( pVL );						
					} else {
						// don't kick out the very last light from this group, instead set the light parameters to 
						// this VL's settings and move on
						ConfigureLightToVLSettings( pVL, m_pWorldLight );  

						m_nFlags |= _FLAGS_ALREADY_UPDATED;
						break;
					}				
				}			
				// the get the previous VL
				pVL = pNextVL;
			}
		}

	} else {
		// optimize the case where we only have 1 VL in the group
		if( m_pWorldLight ) {
			// we have a light, make it so that the light matches our current color and position
			ConfigureLightToVLSettings( pLargestVL, m_pWorldLight );  

			m_nFlags |= _FLAGS_ALREADY_UPDATED;
		}
	}

	FASSERT( m_VLs.nCount );

	return TRUE;
}

// returns TRUE if this group is still active and contains members and is emitting a light
// returns FALSE if the group is now empty and can be removed from the manager's active list
BOOL CFLightGroup::Work() {
	CFVirtualWorldLight *pFirstVL = (CFVirtualWorldLight *)flinklist_GetHead( &m_VLs );

	if( !pFirstVL ) {
		// there are no virtual lights in our list, if we have a real light, free it
		if( m_pWorldLight ) {
			FLightPool_pPool->ReturnToFreePool( m_pWorldLight );
			m_pWorldLight = NULL;
		}
		// this group is no longer needed
		return FALSE;
	}

	if( m_nFlags & _FLAGS_ALREADY_UPDATED ) {
		// no need to do any work this frame, we are already uptodate
		m_nFlags &= ~_FLAGS_ALREADY_UPDATED;
		return TRUE;
	}

	// IT IS ASSUMED THAT ALL VLs IN OUR LIST BELONG IN THE GROUP AND SHOULD INFLUENCE THE REAL LIGHT
	if( m_pWorldLight ) {
		/////////////////////////////////////////////////////////
		// update the light with the newest settings from the VLs
		CFLight *pLight = &m_pWorldLight->m_Light;

		CFVec3A Pos;
		CFColorRGBA Color;
		f32 fIntensity, fRadius;
		CalculateDesiredLightSettingsFromVLs( Pos.v3, fRadius, Color, fIntensity );

		// limit the amount of change from the current light settings
		f32 fUnitChangePerFrame = 9.0f * FLoop_fPreviousLoopSecs;
		FMATH_CLAMP_UNIT_FLOAT( fUnitChangePerFrame );
		fRadius = FMATH_FPOT( fUnitChangePerFrame, pLight->m_spInfluence_WS.m_fRadius, fRadius );
		
		// apply the new values
		pLight->SetColor( &Color );
		pLight->SetPositionAndRadius( &Pos, fRadius );
		pLight->SetIntensity( fIntensity );
		m_pWorldLight->UpdateTracker();
	} else {
		/////////////////////
		// grab a world light
		m_pWorldLight = FLightPool_pPool->GetFromFreePool();
		if( !m_pWorldLight ) {
			// the pool must be full, maybe we can get a light next frame
			return TRUE;
		}

		// fill in light init from the 1st VL (most of the properties are the same for all VLs in this Group)
		FLightInit_t LightInit;
		pFirstVL->FillLightInit( LightInit );
		
		CalculateDesiredLightSettingsFromVLs( LightInit.spInfluence.m_Pos,
											  LightInit.spInfluence.m_fRadius,
											  LightInit.Motif,
											  LightInit.fIntensity );

		// omni lights sometime get their radius changed when Initing, 
		// counteract they by scaling down the requested radius before calling init
		LightInit.spInfluence.m_fRadius *= CFLightGroupMgr::m_fLightRadiusRemapper;

		m_pWorldLight->Init( &LightInit );

		m_pWorldLight->AddToWorld();		
	}

	m_nFlags &= ~_FLAGS_ALREADY_UPDATED;

	return TRUE;
}

u32 CFLightGroup::GetCounts( u32 &rnVirtualLights ) {
	rnVirtualLights = m_VLs.nCount;
	if( m_pWorldLight ) {
		return 1;
	}
	return 0;
}

u32 CFLightGroup::DrawAllLightBoundingSpheres( CFColorRGBA &rRealLightColor,
											   CFColorRGBA &rVirtualLightColor,
											   u32 &rnNumVirtualLights ) {

	rnNumVirtualLights = 0;

	if( m_VLs.nCount < 1 ) {
		return 0;
	}

	CFVirtualWorldLight *pVL = (CFVirtualWorldLight *)flinklist_GetHead( &m_VLs );
	while( pVL ) {
		rnNumVirtualLights++;
		pVL->DrawBoundingSphere( rVirtualLightColor );

		pVL = (CFVirtualWorldLight *)flinklist_GetNext( &m_VLs, pVL );
	}	

#if 1
	// draw the min/max sphere bounding box
	pVL = (CFVirtualWorldLight *)flinklist_GetHead( &m_VLs );

	CFVec3A Min, Max, Temp;
    
	Min.Set( *pVL->m_pPos );
	Min.Sub( *pVL->m_pfRadius );

	Max.Set( Min );
	Max.Add( *pVL->m_pfRadius * 2.0f );

	pVL = (CFVirtualWorldLight *)flinklist_GetNext( &m_VLs, pVL );
	while( pVL ) {
		// grab the min/max pts
		Temp.Set( *pVL->m_pPos );
		Temp.Sub( *pVL->m_pfRadius );
		Min.v3.Min( Temp.v3 );
		Max.v3.Max( Temp.v3 );

		Temp.Add( *pVL->m_pfRadius * 2.0f );
		Min.v3.Min( Temp.v3 );
		Max.v3.Max( Temp.v3 );		

		pVL = (CFVirtualWorldLight *)flinklist_GetNext( &m_VLs, pVL );
	}

	CFVec3 P0, P1, P2, P3, P4, P5, P6, P7;

	P0 = Min.v3;
	P1.Set( Min.x, Max.y, Min.z );
	P2.Set( Min.x, Max.y, Max.z );
	P3 = Max.v3;
	P4.Set( Max.x, Max.y, Min.z );
	P5.Set( Max.x, Min.y, Min.z );
	P6.Set( Max.x, Min.y, Max.z );
	P7.Set( Min.x, Min.y, Max.z );

    fdraw_SolidLine( &P0, &P1, &FColor_MotifRed );
	fdraw_SolidLine( &P1, &P2, &FColor_MotifRed );
	fdraw_SolidLine( &P2, &P3, &FColor_MotifRed );
	fdraw_SolidLine( &P3, &P4, &FColor_MotifRed );
	fdraw_SolidLine( &P4, &P1, &FColor_MotifRed );
	fdraw_SolidLine( &P4, &P5, &FColor_MotifRed );
	fdraw_SolidLine( &P5, &P0, &FColor_MotifRed );
	fdraw_SolidLine( &P5, &P6, &FColor_MotifRed );
	fdraw_SolidLine( &P6, &P3, &FColor_MotifRed );
	fdraw_SolidLine( &P7, &P6, &FColor_MotifRed );
	fdraw_SolidLine( &P7, &P0, &FColor_MotifRed );
	fdraw_SolidLine( &P7, &P2, &FColor_MotifRed );
#endif

    if( m_pWorldLight ) {
		fdraw_FacetedWireSphere( &m_pWorldLight->m_Light.m_spInfluence_WS.m_Pos,
								 m_pWorldLight->m_Light.m_spInfluence_WS.m_fRadius,
								 &rRealLightColor );
		return 1;
	}

	return 0;
}

BOOL CFLightGroup::AreColorsCloseEnough( const CFColorRGBA *pC1, const CFColorRGBA *pC2 ) {
	CFVec3A Delta;

	Delta.Set( pC1->fRed - pC2->fRed,
				pC1->fGreen - pC2->fGreen,
				pC1->fBlue - pC2->fBlue );
	if( Delta.MagSq() > _COLOR_TOLERANCE_SQUARED ) {
		return FALSE;
	}
	
	return	TRUE;
}

BOOL CFLightGroup::DoesVLBelongInGroup( const CFVirtualWorldLight *pVL, const CFLight *pLight ) {

	// see if the color is close enough to the current light color
	if( !AreColorsCloseEnough( pVL->m_pRGBA, &pLight->m_Motif ) ) {
		// colors aren't close enough
		return FALSE;
	}		
	// see if the VL is inside the light's bounding sphere
	CFSphere BS;
	BS.Set( *pVL->m_pPos, *pVL->m_pfRadius );
	if( !BS.IsIntersecting( pLight->m_spInfluence_WS ) ) {
		// the new VL isn't intersecting the current light
		return FALSE;
	}

	return TRUE;
}

BOOL CFLightGroup::DoesVLBelongInGroup( const CFVirtualWorldLight *pVL1, const CFVirtualWorldLight *pVL2 ) {

	// see if the color are close enough to each other
	if( !AreColorsCloseEnough( pVL1->m_pRGBA, pVL2->m_pRGBA ) ) {
		// colors aren't close enough
		return FALSE;
	}
	CFVec3A Temp;

	// see if the the 2 VL lights bounding spheres intersect
	Temp.Set( pVL1->m_pPos->x - pVL2->m_pPos->x,
			  pVL1->m_pPos->y - pVL2->m_pPos->y,
			  pVL1->m_pPos->z - pVL2->m_pPos->z );
	f32 fRadius = *pVL1->m_pfRadius + *pVL2->m_pfRadius;
	if( Temp.MagSq() > (fRadius * fRadius) ) {
		return FALSE;
	}

	return TRUE;
}

void CFLightGroup::ConfigureLightToVLSettings( const CFVirtualWorldLight *pVL, CFWorldLight *pWorldLight ) {
	CFLight *pLight = &pWorldLight->m_Light;

	pLight->SetColor( pVL->m_pRGBA );
	pLight->SetPositionAndRadius( pVL->m_pPos->x, pVL->m_pPos->y, pVL->m_pPos->z, *pVL->m_pfRadius );					
	pLight->SetIntensity( *pVL->m_pfIntensity );
	pWorldLight->UpdateTracker();
}

void CFLightGroup::KickOutVL( CFVirtualWorldLight *pVL ) {

    flinklist_Remove( &m_VLs, pVL );
	pVL->m_nState = CFVirtualWorldLight::STATE_PENDING;
	pVL->m_pLightGroup = NULL;
	
	// add VL back to the pending pool
	CFLightGroupMgr::AddToPendingList( pVL );
}

void CFLightGroup::CalculateDesiredLightSettingsFromVLs( CFVec3 &rPos, f32 &rfRadius, CFColorRGBA &rColor, f32 &rfIntensity ) {
	
	CFVirtualWorldLight *pFirstVL = (CFVirtualWorldLight *)flinklist_GetHead( &m_VLs );

	if( m_VLs.nCount == 1 ) {
		// this is the easy case, might as well not do a ton of work
		rPos = *pFirstVL->m_pPos;
		rfRadius = *pFirstVL->m_pfRadius;
		rColor = *pFirstVL->m_pRGBA;
		rfIntensity = *pFirstVL->m_pfIntensity;		
	} else {
		// compute our desired settings
		CFVec3A Min, Max, Temp;
    
		// for now, just average the color and intensity and build a min/max box to caculate the R and Pos
		Min.Set( *pFirstVL->m_pPos );
		Min.Sub( *pFirstVL->m_pfRadius );

		Max.Set( Min );
		Max.Add( *pFirstVL->m_pfRadius * 2.0f );

		rColor.Set( *pFirstVL->m_pRGBA );

		rfIntensity = *pFirstVL->m_pfIntensity;
		
		CFVirtualWorldLight *pNext = (CFVirtualWorldLight *)flinklist_GetNext( &m_VLs, pFirstVL );
		while( pNext ) {
			// grab the min/max pts
			Temp.Set( *pNext->m_pPos );
			Temp.Sub( *pNext->m_pfRadius );
			Min.v3.Min( Temp.v3 );
			Max.v3.Max( Temp.v3 );

			Temp.Add( *pNext->m_pfRadius * 2.0f );
			Min.v3.Min( Temp.v3 );
			Max.v3.Max( Temp.v3 );			

			// add the RGBA values
			rColor += *pNext->m_pRGBA; 
			
			// add the intensity values
			rfIntensity += *pNext->m_pfIntensity;

			// move to the next VL
			pNext = (CFVirtualWorldLight *)flinklist_GetNext( &m_VLs, pNext );
		}

		// compute a new Pos and Radius
		Max.Sub( Min );
		Max.Mul( 0.5f );
		rfRadius = Max.Mag();
		rfRadius += FMATH_MAX3( Max.x, Max.y, Max.z );
		rfRadius *= 0.5f;
		Max.Add( Min );// this is the new position
		rPos = Max.v3;
		
		// average the color and intensity
		f32 fOOCount = 1.0f/(f32)m_VLs.nCount;
		rColor.v3 *= fOOCount;
		rfIntensity *= fOOCount;
	}
}

//////////////////////////////////////////////////
// CFLightGroupMgr methods

BOOL CFLightGroupMgr::m_bSystemOK;
f32 CFLightGroupMgr::m_fLightRadiusRemapper;
u32 CFLightGroupMgr::m_nNumVLs;
CFVirtualWorldLight *CFLightGroupMgr::m_paVLs;
u32 CFLightGroupMgr::m_nNumLGs;
CFLightGroup *CFLightGroupMgr::m_paLGs;
FLinkRoot_t CFLightGroupMgr::m_VLPool;
FLinkRoot_t CFLightGroupMgr::m_LGPool;
FLinkRoot_t CFLightGroupMgr::m_PendingVLs;
FLinkRoot_t CFLightGroupMgr::m_ActiveLGList;


BOOL CFLightGroupMgr::ModuleStartup() {
	FASSERT( !m_bSystemOK );

	if( Fang_ConfigDefs.nLightGroup_nMaxVirtualLights == 0 ) {
		// ok, assume that the user doesn't want any virtaul lights, don't return FALSE and exit the app
		m_bSystemOK = TRUE;
		return TRUE;
	}

	if( !FLightPool_pPool ) {
		return FALSE;
	}

	// allocate memory
	m_paVLs = fnew CFVirtualWorldLight[Fang_ConfigDefs.nLightGroup_nMaxVirtualLights];
	if( !m_paVLs ) {
		goto _EXIT_WITH_ERROR;
	}

	m_paLGs = fnew CFLightGroup[Fang_ConfigDefs.nLightGroup_nMaxVirtualLights];
	if( !m_paLGs ) {
		goto _EXIT_WITH_ERROR;
	}

	m_nNumVLs = m_nNumLGs = Fang_ConfigDefs.nLightGroup_nMaxVirtualLights;

	// initialize our linked lists & pools
	flinklist_InitRoot( &m_VLPool, FANG_OFFSETOF( CFVirtualWorldLight, m_ManagerLink ) );
	flinklist_InitPool( &m_VLPool, m_paVLs, sizeof( CFVirtualWorldLight ), m_nNumVLs );

	flinklist_InitRoot( &m_LGPool, FANG_OFFSETOF( CFLightGroup, m_ManagerLink ) );
	flinklist_InitPool( &m_LGPool, m_paLGs, sizeof( CFLightGroup ), m_nNumLGs );

	flinklist_InitRoot( &m_PendingVLs, FANG_OFFSETOF( CFVirtualWorldLight, m_ManagerLink ) );
	flinklist_InitRoot( &m_ActiveLGList, FANG_OFFSETOF( CFLightGroup, m_ManagerLink ) );

	m_fLightRadiusRemapper = 1.0f / FLight_fLightRadiusAdj;

	m_bSystemOK = TRUE;

	return TRUE;

_EXIT_WITH_ERROR:
	if( m_paVLs ) {
		fdelete_array( m_paVLs );
		m_paVLs = NULL;
	}
	if( m_paLGs ) {
		fdelete_array( m_paLGs );
		m_paLGs = NULL;
	}
	return FALSE;
}

void CFLightGroupMgr::ModuleShutdown() {

	if( m_bSystemOK ) {
		
		fdelete_array( m_paVLs );
		m_paVLs = NULL;
		m_nNumVLs = 0;

		fdelete_array( m_paLGs );
		m_paLGs = NULL;
        m_nNumLGs = 0;

		m_bSystemOK = FALSE;
	}	
}

void CFLightGroupMgr::ReleaseAllLights() {
	FASSERT( m_bSystemOK );

	// walk all of the LGs
	CFLightGroup *pNextLG;
	CFLightGroup *pLG = (CFLightGroup *)flinklist_GetHead( &m_ActiveLGList );
	while( pLG ) {
		pNextLG = (CFLightGroup *)flinklist_GetNext( &m_ActiveLGList, pLG );

		// remove all of the VLs in this group
		pLG->RemoveAll();
		RemoveFromActiveList( pLG );
		AddToFreePool( pLG );

		// move to the next LG
		pLG = pNextLG;
	}

	// walk any pending VLs
	CFVirtualWorldLight *pNextVL;
	CFVirtualWorldLight *pVL = (CFVirtualWorldLight *)flinklist_GetHead( &m_PendingVLs );
	while( pVL ) {
		pNextVL = (CFVirtualWorldLight *)flinklist_GetNext( &m_PendingVLs, pVL );

		ReleaseVirtualLight( pVL );
				
		// move to the next VL
		pVL = pNextVL;
	}

	FASSERT( m_VLPool.nCount == m_nNumVLs );
	FASSERT( m_LGPool.nCount == m_nNumLGs );
	FASSERT( m_ActiveLGList.nCount == 0 );
	FASSERT( m_PendingVLs.nCount == 0 );
}

CFVirtualWorldLight *CFLightGroupMgr::GetVirtualLight( FLightInit_t *pInit ) {
	FASSERT( m_bSystemOK );

	if( !pInit ) {
		return NULL;
	}

	// grab a VL from the free pool
	CFVirtualWorldLight *pVL = GetFreeVL();
	if( !pVL ) {
		return NULL;
	}

	// init the VL
	if( !pVL->Init( pInit ) ) {
		// put pVL back into the free pool
		AddToFreePool( pVL );
		return NULL;
	}

	// add pVL to our pending pool
	AddToPendingList( pVL );

	return pVL;
}

void CFLightGroupMgr::ReleaseVirtualLight( CFVirtualWorldLight *pLight ) {
	FASSERT( m_bSystemOK );
	FASSERT( pLight->m_nState != CFVirtualWorldLight::STATE_AVAILABLE );
	
	if( pLight->m_pLightGroup ) {
		// this light is currently in assigned to a light group
		FASSERT( pLight->m_nState == CFVirtualWorldLight::STATE_ASSIGNED );

		CFLightGroup *pLG = pLight->m_pLightGroup;

		if( !pLG->RemoveVL( pLight ) ) {
			// the light group is empty, it can be removed from the active list
			RemoveFromActiveList( pLG );
			AddToFreePool( pLG );
		}
	} else {
		FASSERT( pLight->m_nState == CFVirtualWorldLight::STATE_PENDING );

		pLight->m_nState = CFVirtualWorldLight::STATE_AVAILABLE;
		
		// pull pVL out of the pending list and put back into the free pool
		RemoveFromPendingList( pLight );
		AddToFreePool( pLight );
	}
}

void CFLightGroupMgr::Work( void ) {
	FASSERT( m_bSystemOK );
	CFLightGroup *pLG, *pNextLG;
	CFVirtualWorldLight *pVL, *pNextVL;

	// call all of the pre works so that the light groups can kick out members and add them to the pending list
	pLG = (CFLightGroup *)flinklist_GetHead( &m_ActiveLGList );
	while( pLG ) {
		pNextLG = (CFLightGroup *)flinklist_GetNext( &m_ActiveLGList, pLG );

		if( !pLG->PreWork() ) {
			// remove pLG from the Active List
			RemoveFromActiveList( pLG );
			AddToFreePool( pLG );
		}

		pLG = pNextLG;
	}

	// add as many of the pending VLs to existing LG as possible, if a LG can't be found, start a new one
	pVL = (CFVirtualWorldLight *)flinklist_GetHead( &m_PendingVLs );
	while( pVL ) {
		pNextVL = (CFVirtualWorldLight *)flinklist_GetNext( &m_PendingVLs, pVL );
		
		if( !FindExistingLGforVL( pVL ) ) {
			// an existing LG was not found for pVL, create a new one
			pLG = GetFreeLG();
			pLG->AddVL( pVL );
			AddToActiveList( pLG );
		} 
		
		// remove pVL from the pending list
		RemoveFromPendingList( pVL ); 
		
		pVL = pNextVL;
	}

	FASSERT( m_PendingVLs.nCount == 0 );

	// call all of the LG works
	pLG = (CFLightGroup *)flinklist_GetHead( &m_ActiveLGList );
	while( pLG ) {
		pNextLG = (CFLightGroup *)flinklist_GetNext( &m_ActiveLGList, pLG );

		if( !pLG->Work() ) {
			// remove pLG from the Active List
			RemoveFromActiveList( pLG );
			AddToFreePool( pLG );
		}

		pLG = pNextLG;
	}
}

// mainly used by tools, returns the number of real lights currently in the world.
// FDRAW MUST BE THE ACTIVE RENDERER
u32 CFLightGroupMgr::DrawAllLightBoundingSpheres( CFColorRGBA &rRealLightColor,
												  CFColorRGBA &rVirtualLightColor,
												  u32 &rnNumVirtualLights ) {
	FASSERT( m_bSystemOK );

	CFLightGroup *pLG;
	u32 nNumRealLights = 0;
	rnNumVirtualLights = 0;
	u32 nCount;
	
	FASSERT( frenderer_GetActive() == FRENDERER_DRAW );

	// call all of the LG works
	pLG = (CFLightGroup *)flinklist_GetHead( &m_ActiveLGList );
	while( pLG ) {
		nNumRealLights += pLG->DrawAllLightBoundingSpheres( rRealLightColor, rVirtualLightColor, nCount );
		rnNumVirtualLights += nCount;

		pLG = (CFLightGroup *)flinklist_GetNext( &m_ActiveLGList, pLG );
	}

	return nNumRealLights;
}

void CFLightGroupMgr::GetCounts( u32 &rnNumRealLights, u32 &rnVirtualLights ) {
	FASSERT( m_bSystemOK );

	CFLightGroup *pLG;
	rnNumRealLights = 0;
	rnVirtualLights = 0;
	u32 nCount;
	
	// call all of the LG works
	pLG = (CFLightGroup *)flinklist_GetHead( &m_ActiveLGList );
	while( pLG ) {
		rnNumRealLights += pLG->GetCounts( nCount );
        rnVirtualLights += nCount;

		pLG = (CFLightGroup *)flinklist_GetNext( &m_ActiveLGList, pLG );
	}
}

BOOL CFLightGroupMgr::FindExistingLGforVL( CFVirtualWorldLight *pVL ) {
	FASSERT( m_bSystemOK );
	CFLightGroup *pLG, *pNextLG;

	pLG = (CFLightGroup *)flinklist_GetHead( &m_ActiveLGList );
	while( pLG ) {
		pNextLG = (CFLightGroup *)flinklist_GetNext( &m_ActiveLGList, pLG );

		if( pLG->AddVL( pVL ) ) {
			// we found a LG for this light, done
			return TRUE;
		}
		
		pLG = pNextLG;
	}

	return FALSE;
}

/////////////////////////////////////////////////////////
// CFVirtualWorldLight methods

CFVirtualWorldLight::CFVirtualWorldLight() {
	// these are really the only vars that matter	
	m_pLightGroup = NULL;
    m_nState = STATE_AVAILABLE;
}

BOOL CFVirtualWorldLight::Init( FLightInit_t *pInit ) {
	FASSERT( !m_pLightGroup );
	FASSERT( m_nState == STATE_AVAILABLE );

	// do some sanity checks to make sure that this is a supported light type
	if( pInit->nFlags & (FLIGHT_FLAG_HASDIR | FLIGHT_FLAG_LIGHT_ATTACHED) ||
		pInit->nType != FLIGHT_TYPE_OMNI  ) {
		// not the right type of light
		return FALSE;
	}

	m_fIntensity = pInit->fIntensity;
	m_pfIntensity = &m_fIntensity;
	
	m_RGBA.Set( pInit->Motif );
    m_pRGBA = &m_RGBA;
	
	m_Pos = pInit->spInfluence.m_Pos;
	m_pPos = &m_Pos;

	m_fRadius = pInit->spInfluence.m_fRadius;
	m_pfRadius = &m_fRadius;

	fclib_strcpy( m_szCoronaTexName, pInit->szCoronaTexName );
	m_nInitFlags = pInit->nFlags;
	m_nMotifIndex = pInit->Motif.nMotifIndex;
	m_fCoronaScale = pInit->fCoronaScale;

	m_nState = STATE_PENDING;

	return TRUE;
}

void CFVirtualWorldLight::FillLightInit( FLightInit_t &rInit ) {
	FASSERT( m_nState != STATE_AVAILABLE );

	rInit.szName[0] = 0;
	rInit.szPerPixelTexName[0] = 0;
	fclib_strcpy( rInit.szCoronaTexName, m_szCoronaTexName );
	rInit.nFlags = m_nInitFlags;
	rInit.nLightID = 0xFFFF;
	rInit.nType = FLIGHT_TYPE_OMNI;
	rInit.nParentBoneIdx = -1;
	rInit.fIntensity = *m_pfIntensity;
	rInit.Motif = *m_pRGBA;
	rInit.Motif.nMotifIndex = m_nMotifIndex;
	rInit.spInfluence.m_fRadius = *m_pfRadius;
	rInit.spInfluence.m_Pos = *m_pPos;
	rInit.fCoronaScale = m_fCoronaScale;
}

BOOL CFVirtualWorldLight::DoesLightInitMatchUs( const FLightInit_t &rInit ) const {
	FASSERT( m_nState != STATE_AVAILABLE );

	if( rInit.nFlags == m_nInitFlags &&
		fclib_stricmp( rInit.szCoronaTexName, m_szCoronaTexName ) == 0 &&
		rInit.nType == FLIGHT_TYPE_OMNI &&
		rInit.Motif.nMotifIndex == m_nMotifIndex ) {
		// the properties that must match, do in fact match, these lights could be merged if nearby
		return TRUE;
	}

	return FALSE;
}

void CFVirtualWorldLight::DrawBoundingSphere( CFColorRGBA &rColor ) {
	
    FASSERT( frenderer_GetActive() == FRENDERER_DRAW );

	if( m_nState == STATE_AVAILABLE ) {
		return;
	}

	fdraw_FacetedWireSphere( m_pPos, *m_pfRadius, &rColor );
}

void CFVirtualWorldLight::UpdateIntensity( f32 fIntensity ) {
	m_fIntensity = fIntensity;
	m_pfIntensity = &m_fIntensity;
}

void CFVirtualWorldLight::SetIntensityPtr( f32 *pfIntensity ) {
	m_pfIntensity = pfIntensity;
}

const f32 *CFVirtualWorldLight::GetCurrentIntensity() const {
	return m_pfIntensity;
}

void CFVirtualWorldLight::UpdateColor( const CFColorRGBA &rRGBA ) {
	m_RGBA = rRGBA;
	m_pRGBA = &m_RGBA;
}

void CFVirtualWorldLight::UpdataColor( const CFColorRGB &rRGB ) {
	m_RGBA.Set( rRGB, 0.0f );
	m_pRGBA = &m_RGBA;
}

void CFVirtualWorldLight::SetColorPtr( CFColorRGBA *pRGBA ) {
	m_pRGBA = pRGBA;
}

const CFColorRGBA *CFVirtualWorldLight::GetCurrentColor() const {
	return m_pRGBA;
}

void CFVirtualWorldLight::UpdatePos( const CFVec3 &rPos ) {
	m_Pos = rPos;
	m_pPos = &m_Pos;
}

void CFVirtualWorldLight::SetPosPtr( CFVec3 *pPos ) {
	m_pPos = pPos;
}

const CFVec3 *CFVirtualWorldLight::GetCurrentPos() const {
	return m_pPos;
}

void CFVirtualWorldLight::UpdateRadius( f32 fRadius ) {
	m_fRadius = fRadius;
	m_pfRadius = &m_fRadius;
}

void CFVirtualWorldLight::SetRadiusPtr( f32 *pfRadius ) {
	m_pfRadius = pfRadius;
}

const f32 *CFVirtualWorldLight::GetCurrentRadius() const {
	return m_pfRadius;
}