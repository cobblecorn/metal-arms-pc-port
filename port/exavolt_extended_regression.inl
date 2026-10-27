#include "pc_randomizer.h"
#include "Ai/aiBuilder.h"
#include "eshield.h"
#include "explosion.h"
#include "BarterTypes.h"

static void PortExavoltExtended(CBot *player) {
    const CWeapon::WeaponType_e types[] = {CWeapon::WEAPON_TYPE_LASER,CWeapon::WEAPON_TYPE_SPEW,CWeapon::WEAPON_TYPE_FLAMER,
        CWeapon::WEAPON_TYPE_RIVET_GUN,CWeapon::WEAPON_TYPE_ROCKET_LAUNCHER,CWeapon::WEAPON_TYPE_RIPPER,
        CWeapon::WEAPON_TYPE_BLASTER,CWeapon::WEAPON_TYPE_BLINK_RIGHT_HAND};
    const int primaries[] = {CBotBuilder::_NPC_WEAPON_LASER,CBotBuilder::_NPC_WEAPON_SPEW,CBotBuilder::_NPC_WEAPON_FLAMER,
        CBotBuilder::_NPC_WEAPON_RIVET,CBotBuilder::_NPC_WEAPON_ROCKET,CBotBuilder::_NPC_WEAPON_RIPPER,
        CBotBuilder::_NPC_WEAPON_BLASTER,CBotBuilder::_NPC_WEAPON_HAND};
    CBotGruntBuilder model;
    model.SetDefaults(0,0,ENTITY_TYPE_BOTGRUNT);
    model.m_nOwnerPlayerIndex=-1;
    model.m_pszEC_EntityName="exavolt_extended";
    model.m_EC_Mtx_WS=*player->MtxToWorld();
    model.m_EC_Mtx_WS.m_vPos.x+=15;
    model.m_EC_Mtx_WS.m_vPos.y+=3;
    for (int family=0;family<2;++family) {
        model.m_nEC_TypeBits=ENTITY_BIT_BOT|(family?ENTITY_BIT_BOTMINER:ENTITY_BIT_BOTGRUNT);
        for (int desired=0;desired<8;++desired) {
            int tries=0;
            while (pcrandom_Primary(&model,family)!=primaries[desired] && ++tries<1000) model.m_EC_Mtx_WS.m_vPos.x+=0.125f;
            Check(tries<1000,"extended NPC weapon pool contains every selected primary");
            u32 health;
            const char *armor;
            const u32 originalHealth=model.m_nEC_HealthContainerCount;
            {
                PcModActorScope scope(&model);
                health=model.m_nEC_HealthContainerCount;
                armor=model.m_pszEC_ArmorProfile;
            }
            Check(model.m_nEC_HealthContainerCount==originalHealth,"extended stat override restores shared builder state");
            CBot *bot=family ? (CBot *)fnew CBotMiner : (CBot *)fnew CBotGrunt;
            const BOOL created=family ? ((CBotMiner*)bot)->Create(-1,FALSE,model.m_pszEC_EntityName,&model.m_EC_Mtx_WS) :
                ((CBotGrunt*)bot)->Create(-1,FALSE,model.m_pszEC_EntityName,&model.m_EC_Mtx_WS);
            Check(created,"extended native NPC constructs with randomized equipment");
            if (created) {
                CWeapon *weapon=bot->GetPrimaryWeapon();
                Check(weapon && weapon->Type()==types[desired],"extended native primary matches the seeded Ripper/Blaster/hand-inclusive pool");
                Check(bot->HealthContainerCount()==(f32)health && health>=1 && health<=6,"extended native battery count reaches actual health containers");
                Check(bot->GetArmorProfile()==CDamage::FindArmorProfile(armor),"extended native armor profile reaches the actual actor");
                if (weapon) {
                    const u32 expected=pcrandom_Roll(&model,family?0x6d757067u:0x67757067u)%weapon->GetUpgradeLevelCount();
                    Check(weapon->GetUpgradeLevel()==expected,"extended native weapon uses a seeded valid upgrade level");
                    DEVPRINTF("EXAVOLT-EXT actor=%s family=%d weapon=%d level=%u health=%.0f shield=%d\n",
                        bot->Name(),family,weapon->Type(),weapon->GetUpgradeLevel(),bot->HealthContainerCount(),bot->GetShield()!=NULL);
                }
                Check((bot->GetShield()!=NULL)==(pcrandom_Roll(&model,0x73686965u)%5==4),"extended native NPC shield follows its seeded probability");
                if (!bot->IsInWorld()) bot->AddToWorld();
                if (bot->GetShield()) Check(bot->GetShield()->IsInWorld(),"extended native shield enters the world with its owner");
                if (weapon && (desired==5 || desired==6)) {
                    CFVec3A target=model.m_EC_Mtx_WS.m_vPos;target.z+=100;target.y+=2;
                    const s32 before=weapon->GetClipAmmo();
                    u32 fired=0;
                    for (int tick=0;tick<120;++tick) {weapon->Work();fired|=weapon->TriggerWork(1.0f,0.0f,&target);}
                    DEVPRINTF("EXAVOLT-EXT fire family=%d weapon=%d before=%d after=%d flags=%u\n",
                        family,weapon->Type(),before,weapon->GetClipAmmo(),fired);
                    Check(fired!=0,"extended Ripper / Scatter Blaster executes its native firing path");
                }
                if (family && bot->GetShield()) {
                    const f32 savedHealth=bot->GetShield()->NormHealth();
                    const s32 checkpoint=CFCheckPoint::GetCheckPoint();
                    CFCheckPoint::SetCheckPoint(1);
                    CFCheckPoint::SetSaveState();
                    bot->CheckpointSave();
                    CFCheckPoint::DoneSaving();
                    bot->GetShield()->SetNormHealth(0.125f);
                    CFCheckPoint::SetLoadState();
                    bot->CheckpointRestore();
                    CFCheckPoint::DoneLoading();
                    CFCheckPoint::SetCheckPoint(checkpoint);
                    Check(bot->GetShield()->NormHealth()==savedHealth,"extended randomizer shield health survives a native checkpoint restore");
                }
                if (!family) {
                    const s32 state=pcrandom_Dataport(&model);
                    Check(bot->DataPort_IsInstalled()==(state!=0) && bot->DataPort_IsOpen()==(state==2),"extended native dataport state reaches the actor");
                }
                if (desired==5 || desired==6) {
                    // Real rounds retain their owner/weapon. Keep these four test
                    // actors alive until the isolated client exits; do not free
                    // their resource frame beneath an active projectile.
                    bot->EnableAutoWork(FALSE);
                } else {
                    bot->RemoveFromWorld(TRUE);
                    if (bot->GetShield()) Check(!bot->GetShield()->IsInWorld() || bot->GetShield()->IsMarkedForWorldRemove(),
                        "extended native shield leaves or queues removal with its owner");
                    bot->Destroy();
                }
            }
            if (!created || (desired!=5 && desired!=6)) fdelete(bot);
            model.m_EC_Mtx_WS.m_vPos.x+=0.5f;
        }
    }
    model.m_nEC_TypeBits=ENTITY_BIT_BOT|ENTITY_BIT_BOTGRUNT;
    CAIBuilder authoredAI,nativeAI;
    model.m_pAIBuilder=&authoredAI;
    authoredAI.SetDefaults(&model);
    const u32 authoredAggression=authoredAI.m_uAggression;
    Check(pcrandom_AI(&model,&nativeAI),"extended AI initializes a private native parameter set");
    Check(nativeAI.m_uAggression==pcrandom_Roll(&model,0x61676772u)%101 &&
        nativeAI.m_uCourage==pcrandom_Roll(&model,0x636f7572u)%101 &&
        nativeAI.m_uIntelligence==pcrandom_Roll(&model,0x696e746cu)%101,"extended AI attributes match their seeded values");
    Check(authoredAI.m_uAggression==authoredAggression,"extended AI leaves authored/shared settings unchanged");
    if (nativeAI.m_uInitialJob==2) {
        CEntity *path=CEntity::Find(nativeAI.m_pszPatrolPath);
        Check(path && (path->TypeBits() & ENTITY_BIT_SPLINE),"extended AI patrol uses an existing map spline");
    }
    model.m_pAIBuilder=NULL;
    if (!CBarterLevel::GetNumTables() && pcmods_Option("shops")) {
        Check(CBarterItem::InitFromFile("barteritems.csv"),"extended randomized shop catalog initializes in the real engine");
        Check(CBarterLevel::InitLevel("wedmbrtr_03.csv"),"extended randomized shop table and all display resources load");
    }
    if (CBarterLevel::GetNumTables()) {
        u32 purchases=0;
        player->m_pInventory->m_aoItems[INVPOS_WASHER].m_nClipAmmo=10000;
        for (u32 t=0;t<CBarterLevel::GetNumTables() && purchases<3;++t) {
            CBarterTable *table=CBarterLevel::GetTable(t);
            for (u32 slot=0;slot<3 && purchases<3;++slot) {
                CBarterSlot *offers=&table->m_aSlots[slot];
                for (u32 chain=0;chain<offers->m_nSpecSaleChains && purchases<3;++chain) {
                    CBarterSaleChains *items=&offers->m_paSaleChains[chain];
                    for (u32 n=0;n<items->m_nNumItems && purchases<3;++n) {
                        CBarterItemInst *item=&items->m_paItemInsts[n];
                        if (!item->IsAvailable() || !item->CanPurchase(player)) continue;
                        const s32 wallet=player->m_pInventory->m_aoItems[INVPOS_WASHER].m_nClipAmmo;
                        if (item->Purchase(player)) {
                            ++purchases;
                            Check(player->m_pInventory->m_aoItems[INVPOS_WASHER].m_nClipAmmo==wallet-item->GetPrice(),
                                "extended randomized real shop purchase delivers and charges once");
                            DEVPRINTF("EXAVOLT-EXT bought slot=%u item=%s price=%u\n",slot,item->m_pCollectableType->m_pszName,item->GetPrice());
                        } else Check(player->m_pInventory->m_aoItems[INVPOS_WASHER].m_nClipAmmo==wallet,
                            "extended unavailable shop purchase leaves wallet unchanged");
                    }
                }
            }
        }
        Check(purchases>0,"extended native randomized shop stock supports an actual purchase");
        player->m_pInventory->m_aoItems[INVPOS_WASHER].m_nClipAmmo=321;
    }
    Check(pcrandom_Drop(COLLECTABLE_DET_PACK,player,1)==COLLECTABLE_DET_PACK &&
        pcrandom_Drop(COLLECTABLE_CHIP,player,2)==COLLECTABLE_CHIP &&
        pcrandom_Drop(COLLECTABLE_SECRET_CHIP,player,3)==COLLECTABLE_SECRET_CHIP &&
        pcrandom_Drop(COLLECTABLE_WEAPON_WRENCH,player,4)==COLLECTABLE_WEAPON_WRENCH,
        "extended loot preserves mandatory keys, DET packs, chips and wrenches");
    const CollectableType_e drop=(CollectableType_e)pcrandom_Drop(COLLECTABLE_WASHER,player,7);
    CFMtx43A at=*player->MtxToWorld();at.m_vPos.x+=15;at.m_vPos.y+=5;
    Check(CCollectable::PlaceIntoWorld(drop,&at),"extended randomized drop has real loadable collectable resources");
    FExplosion_GroupHandle_t explosion=pcrandom_Explosion(CExplosion2::GetExplosionGroup("rocket"),player,"script_explosions");
    Check(explosion!=FEXPLOSION_INVALID_HANDLE,"extended seeded explosion resolves to a native group");
    DEVPRINTF("EXAVOLT-EXT COMPLETE native-options=33\n");
}
