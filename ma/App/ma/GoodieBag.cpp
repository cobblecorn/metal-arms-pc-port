//////////////////////////////////////////////////////////////////////////////////////
// GoodieBag.cpp - Goodie bag class for Mettle Arms.
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
// Date     Who         Description
// -------- ----------  --------------------------------------------------------------
// 05/16/02	Link		Created.
//////////////////////////////////////////////////////////////////////////////////////

#include "GoodieBag.h"
#include "fclib.h"
#include "meshentity.h"
#include "ESphere.h"
#include "BotSwarmer.h"
#include "fsndfx.h"
#include "collectable.h"

//cchar *GoodieInst_apszGoodieNames[GOODIETYPE_COUNT] = { "washer", "coringcharge", "chip", "swarmer", /*"ammo1",*/ "ammo2", "pup_energy" };

CGoodieInst::~CGoodieInst()
{
	ClearEntities();
}

void CGoodieInst::ClearEntities()
{
	u32 uIdx;
	if(m_papEntities != NULL)
	{
		for(uIdx = 0; uIdx < m_auQuantity[1]; ++uIdx)
		{
			fdelete(m_papEntities[uIdx]);
			m_papEntities[uIdx] = NULL;
		}
	}
	m_papEntities = NULL;  //use fres_alloc
}


GoodieType_e CGoodieInst::GetGoodieTypeFromString(cchar *pszGoodieName)
{
	/*u32 uCurNameIdx;
	for(uCurNameIdx = 0; uCurNameIdx < GOODIETYPE_COUNT; ++uCurNameIdx)
	{
		if(fclib_stricmp(GoodieInst_apszGoodieNames[uCurNameIdx], pszGoodieName) == 0)
		{
			return((GoodieType_e)(uCurNameIdx));
		}
	}

	return((GoodieType_e)(-1));*/
	return (GoodieType_e) CCollectable::GetCollectableTypeFromString( pszGoodieName );
}


CEntity *CGoodieInst::GetEntity(u32 uNum)
{
	FASSERT(m_papEntities != NULL);
	FASSERT(m_papEntities[uNum] != NULL);
	return(m_papEntities[uNum]);
}


u32 CGoodieInst::GetQuant()
{
	f32 fUnitControl = fmath_RandomFloat();
	return((u32)(FMATH_FPOT(fUnitControl, m_auQuantity[0], m_auQuantity[1])));
}

// This creates a simple GoodieInst that always gives a fixed number of
//   a certain item.
void CGoodieInst::Create(GoodieType_e eGoodieType, u32 uQuantity)
{
	m_eGoodieType = eGoodieType;
	m_auQuantity[0] = uQuantity;
	m_auQuantity[1] = uQuantity;
	m_fProb = 1.0f;

	m_bCreated = TRUE;
}

// This is the most likely create to use.
void CGoodieInst::Create(GoodieType_e eGoodieType, u32 uQuantity, f32 fProb)
{
	m_eGoodieType = eGoodieType;
	m_auQuantity[0] = uQuantity;
	m_auQuantity[1] = uQuantity;
	m_fProb = fProb;

	m_bCreated = TRUE;
}

// This creates a slightly less simple GoodieInst that always gives off its
//   goodie but in varying quantities.
void CGoodieInst::Create(GoodieType_e eGoodieType, u32 uQuantity1, u32 uQuantity2)
{
	m_eGoodieType = eGoodieType;
	m_auQuantity[0] = uQuantity1;
	m_auQuantity[1] = uQuantity2;
	m_fProb = 1.0f;

	m_bCreated = TRUE;
}


void CGoodieInst::Create(GoodieType_e eGoodieType, u32 uQuantity1, u32 uQuantity2, f32 fProb, GoodieBagAngVelType_e eAngVelType /*= GOODIEBAGANGVELTYPE_NONE*/)
{
	m_eGoodieType = eGoodieType;
	m_auQuantity[0] = uQuantity1;
	m_auQuantity[1] = uQuantity2;
	m_fProb = fProb;
	m_eAngVelType = eAngVelType;

	m_bCreated = TRUE;
}


void CGoodieInst::CloneFromOther(CGoodieInst *pGoodieInst)
{
	FASSERT(pGoodieInst != NULL);

	m_bCreated = pGoodieInst->m_bCreated;
	m_eGoodieType = pGoodieInst->m_eGoodieType;
	m_auQuantity[0] = pGoodieInst->m_auQuantity[0];
	m_auQuantity[1] = pGoodieInst->m_auQuantity[1];
	m_fProb = pGoodieInst->m_fProb;
	m_eAngVelType = pGoodieInst->m_eAngVelType;
}


BOOL CGoodieInst::CreateEntities()
{
	ClearEntities();

	// !!Nate - What to do about this?
/*	switch(m_eGoodieType)
	{
		case GOODIETYPE_SWARMER:
		{
			m_papEntities = (CEntity **)(fres_Alloc(sizeof(CEntity *) * m_auQuantity[1]));

			if(m_papEntities == NULL)
			{
				DEVPRINTF("CGoodieInst::CreateEntities() : Out of memory for swarmers.\n", m_auQuantity[1]);
				return(FALSE);
			}

			u32 uEntityIdx;
			for(uEntityIdx = 0; uEntityIdx < m_auQuantity[1]; ++uEntityIdx)
			{
				m_papEntities[uEntityIdx] = fnew CBotSwarmer;
				if(m_papEntities[uEntityIdx] == NULL)
				{
					DEVPRINTF("CGoodieInst::CreateEntities() : Not enough memory to create %d CESwarmers.\n", m_auQuantity[1]);
					return(FALSE);
				}
				if (!((CBotSwarmer *) m_papEntities[uEntityIdx])->Create(-1,
																		FALSE,
																		"justine", 
																		NULL,
																		NULL,
																		CBotSwarmer::BotTypeSwarmer))
				{
					DEVPRINTF("CGoodieInst::CreateEntities() : Error creating CESwarmers #1.\n", uEntityIdx);
				}
				m_papEntities[uEntityIdx]->RemoveFromWorld(TRUE);
			}
		}
	}
*/
	return(TRUE);
}


static const f32 _afDefaultVel[2] = { 40.0f, 45.0f };

CGoodieBag::CGoodieBag()
{
	m_uNumGoodies = 0;
	m_fProb = 0.0f;
	m_eBrain = GOODIEBAGBRAIN_ANY;
	m_fGoodieScale = 1.0f;
}

void CGoodieBag::Reset()
{
	m_uNumGoodies = 0;
	m_fProb = 0.0f;
	m_fGoodieScale = 1.0f;
}

void CGoodieBag::SetProb(f32 fProb)
{
	if(fProb > 1.0f)
	{
		DEVPRINTF("CGoodieBag::SetProb() : %f Probability should be 0.0-1.0.\n");
		return;	 
	}
	if(fProb < 0.0f)
	{
		DEVPRINTF("CGoodieBag::SetProb() : Probability is less than 0.0f.\n");
		return;
	}

	m_fProb = fProb;
}

void CGoodieBag::SetGoodieScale( f32 fScale )
{
	if( fScale <= 0.0f )
	{
		DEVPRINTF( "CGoodieBag::SetGoodieScale(): Invalid scale %f.\n", fScale );
		return;
	}

	m_fGoodieScale = fScale;
}

BOOL CGoodieBag::AddGoodie(GoodieType_e eGoodieType, u32 uQuantity)
{
	if(m_uNumGoodies == GoodieBag_uMaxNumGoodieInsts)
	{
		return(FALSE);
	}

	m_aGoodies[m_uNumGoodies++].Create(eGoodieType, uQuantity);
	return(TRUE);
}


BOOL CGoodieBag::AddGoodie(GoodieType_e eGoodieType, u32 uQuantity1, u32 uQuantity2, f32 fProb, GoodieBagAngVelType_e eAngVelType /*= GOODIEBAGANGVELTYPE_NONE*/)
{
	if(m_uNumGoodies == GoodieBag_uMaxNumGoodieInsts)
	{
		return(FALSE);
	}

	m_aGoodies[m_uNumGoodies++].Create(eGoodieType, uQuantity1, uQuantity2, fProb, eAngVelType);
	return(TRUE);
}


BOOL CGoodieBag::AddGoodie(GoodieType_e eGoodieType, u32 uQuantity, f32 fProb)
{
	if(m_uNumGoodies == GoodieBag_uMaxNumGoodieInsts)
	{
		return(FALSE);
	}

	m_aGoodies[m_uNumGoodies++].Create(eGoodieType, uQuantity, fProb);
	return(TRUE);
}

	
void CGoodieBag::CreateEntities()
{
	u32 uCurGIIdx;
	for(uCurGIIdx = 0; uCurGIIdx < m_uNumGoodies; ++uCurGIIdx)
	{
		m_aGoodies[uCurGIIdx].CreateEntities();
	}
}


void CGoodieBag::ReleaseIntoWorld(CEntity *pOwner)
{
//	if(!fmath_RandomChance(m_fProb))
//	{
//		// This goodie bag is going to choose not to spawn.
//		return;
//	}

	CEntity *apSpawnEntities[6];
	f32 afSpawnVel[6][2];
	f32 afSpawnSpread[6];

	u32 uSpawnLocCnt = 0;

	/////////////////////////////////////////////////////////////////////
	// Create the list of spawn points.
	uSpawnLocCnt = pOwner->GetChildCount();
#if 0
	if(uSpawnLocCnt != 0)
	{
		// There are children, so let's use them to create our list of
		//   spawn points.
		pLastEntity = pOwner->GetFirstChild();
		apSpawnEntities[0] = pLastEntity;
//		apmtxSpawnMtx[0] = pLastEntity->MtxToWorld();
		afSpawnVel[0][0] = _afDefaultVel[0];
		afSpawnVel[0][1] = _afDefaultVel[1];
		afSpawnSpread[0] = 0.0f;

		u32 uCurSpawnLocIdx;
		for(uCurSpawnLocIdx = 1; uCurSpawnLocIdx < uSpawnLocCnt; ++uCurSpawnLocIdx)
		{
			pLastEntity = pOwner->GetNextChild(pLastEntity);
			apSpawnEntities[uCurSpawnLocIdx] = pLastEntity;
//			apmtxSpawnMtx[uCurSpawnLocIdx] = pLastEntity->MtxToWorld();
			afSpawnVel[uCurSpawnLocIdx][0] = _afDefaultVel[0];
			afSpawnVel[uCurSpawnLocIdx][1] = _afDefaultVel[1];
			afSpawnSpread[uCurSpawnLocIdx] = 0.0f;
		}
	}
	else
#endif
	{
		// There are no children, so let's use the owner as the spawn point.
		uSpawnLocCnt = 1;
		apSpawnEntities[0] = pOwner;
//		apmtxSpawnMtx[0] = pOwner->MtxToWorld();
		afSpawnVel[0][0] = _afDefaultVel[0];
		afSpawnVel[0][1] = _afDefaultVel[1];
		afSpawnSpread[0] = 0.0f;
	}
	//
	/////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////
	// Go through each goodie inst and see if we should put anything in
	//   the world.
	u32 uCurGoodieInstIdx, uNumToSpawn;
	CGoodieInst *pCurGI;
	u32 uSpawnPtIdx;
	CFVec3A vecLoc, vecDir;
//	f32 fRandomNum = fmath_RandomFloat();
//	f32 fCumProb = 0.0f;
//	BOOL bSpawnedOne = FALSE;
//	FASSERT_UNIT_FLOAT(fRandomNum);

	for(uCurGoodieInstIdx = 0; uCurGoodieInstIdx < m_uNumGoodies; ++uCurGoodieInstIdx)
	{
		pCurGI = &(m_aGoodies[uCurGoodieInstIdx]);
//		fCumProb += pCurGI->GetProb();

		// NKM - LDs always want to spawn goodies
//		if(((fmath_RandomChance(pCurGI->GetProb())) && (m_eBrain == GOODIEBAGBRAIN_ANY)))
//			|| ((fCumProb >= fRandomNum) && (m_eBrain == GOODIEBAGBRAIN_ONE) && (!bSpawnedOne)))
//		{
//			bSpawnedOne = TRUE;
			/////////////////////////////////////////////////////////////////////
			// We are going to spawn some goodies for this one.
			uNumToSpawn = pCurGI->GetQuant();
			while(uNumToSpawn > 0)
			{
				--uNumToSpawn;

				uSpawnPtIdx = fmath_RandomChoice(uSpawnLocCnt);
				CEntity *pCurEntity = apSpawnEntities[uSpawnPtIdx];

				/////////////////////////////////////////////////////////////////////
				// Get the initial position.
				pCurEntity->GetGoodieDistributionOrigin(&vecLoc);
				//
				/////////////////////////////////////////////////////////////////////

				/////////////////////////////////////////////////////////////////////
				// Get the initial velocity.
				pCurEntity->GetGoodieDistributionDir(&vecDir);
				// JUSTINE: Fix this up later.
				vecDir.x += 0.6f * fmath_RandomBipolarUnitFloat();
				vecDir.z += 0.6f * fmath_RandomBipolarUnitFloat();
				vecDir.Unitize();
				//
				/////////////////////////////////////////////////////////////////////

				/////////////////////////////////////////////////////////////////////
				// Get an angular velocity.
				CFVec3A vecAxis;
				f32 fOmega;

				switch(pCurGI->m_eAngVelType)
				{
					case GOODIEBAGANGVELTYPE_NONE:
					{
						vecAxis.Set(0.0f, 1.0f, 0.0f);
						fOmega = 0.0f;
						break;
					}
					case GOODIEBAGANGVELTYPE_RANDOM:
					{
						fmath_RandomPointInSphericalSector(vecAxis, 1.0f, 1.0f, FMATH_2PI);
						fOmega = fmath_RandomFloatRange(0.0f, 2.0f);
						break;
					}
				}
				//
				/////////////////////////////////////////////////////////////////////

				/////////////////////////////////////////////////////////////////////
				// Set up our entity with our values.
				// NKM - Bump up the goodie distribution a bit
				CFVec3A RealPos = CFVec3A::m_UnitAxisY;

				RealPos.Mul( 0.5f );
				RealPos.Add( vecLoc );

				CFMtx43A::m_Xlat.m_vPos = RealPos;

				vecDir.Mul(fmath_RandomFloatRange(20.0f, 30.0f));
//				pGoodie->Relocate_WS(&CFMtx43A::m_Xlat);
		
				CCollectable::PlaceIntoWorld( (CollectableType_e) pCurGI->GetGoodieType(), &CFMtx43A::m_Xlat, &vecDir, m_fGoodieScale, -1, pOwner );

				//
				/////////////////////////////////////////////////////////////////////
//			}
			//
			/////////////////////////////////////////////////////////////////////
		}
	}
	//
	/////////////////////////////////////////////////////////////////////
}


void CGoodieBag::CloneOther(CGoodieBag *pGoodieBag)
{
	FASSERT(pGoodieBag != NULL);

	m_uNumGoodies = pGoodieBag->m_uNumGoodies;
	m_fProb = pGoodieBag->m_fProb;

	m_eBrain = pGoodieBag->m_eBrain;
//	m_eAngVelType = pGoodieBag->m_eAngVelType;

	m_fGoodieScale = pGoodieBag->m_fGoodieScale;

	// Copy over the goodie insts from the other bag.
	u32 uCurGI;
	for(uCurGI = 0; uCurGI < m_uNumGoodies; ++uCurGI)
	{
		m_aGoodies[uCurGI].CloneFromOther(&(pGoodieBag->m_aGoodies[uCurGI]));
	}
}


void CGoodieBag::PickupCallback(CEntity *pPickedUp, CGoodieBag *pGoodieBag, GoodieType_e eGoodieType)
{
/*	switch(eGoodieType)
	{
		case GOODIETYPE_WASHER:
		{
			CGamePools::ReturnWasher((CEGoodie *)(pPickedUp));
			break;
		}
		case GOODIETYPE_CORINGCHARGE:
		{
			CGamePools::ReturnCoringCharge((CEGoodie *)(pPickedUp));
			break;
		}
		case GOODIETYPE_CHIP:
		{
			CGamePools::ReturnChip((CEGoodie *)(pPickedUp));
			break;
		}
//		case GOODIETYPE_AMMO1:
//		{
//			CGamePools::ReturnAmmo((CMeshEntity *)(pPickedUp));
//			break;
//		}
		case GOODIETYPE_PUP_ENERGY:
		{
			CGamePools::ReturnEnergy((CEGoodie *)(pPickedUp));
			break;
		}
		default:
		{
			FASSERT_NOW;
			break;
		}
	}*/
}
