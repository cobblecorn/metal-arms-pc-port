// Included after Check() in coop_regression.cpp. Runs only in the opt-in fixture.
#include "pc_mods.h"
class LaserRetailTarget : public CMeshEntity {
public:
    int impacts = 0;
    f32 impactDamage = 0.0f;
    const CDamageProfile *profile = NULL;
    virtual void InflictDamageResult(const CDamageResult *result) {
        if (result->m_pDamageData->m_nDamageLocale == CDamageForm::DAMAGE_LOCALE_IMPACT) {
            ++impacts;
            impactDamage += result->m_fDeltaHitpoints;
            profile = result->m_pDamageData->m_pDamageProfile;
            // Isolate the initial impact. A legitimate reflected shot can return
            // from nearby mission geometry in co-op; that is a separate hit.
            SetCollisionFlag(FALSE);
        }
        CMeshEntity::InflictDamageResult(result);
    }
};

void port_LaserRetailRegression(CBot *actor, int &step, u32 &stamp, u32 now) {
    static CWeaponLaser *laser = NULL;
    static LaserRetailTarget *target = NULL;
    static int sample = 0;
    static CFVec3A origin;
    static const u32 levels[] = {1, 2, 4};
    const BOOL super=pcmods_Feature("super_mining_lasers");
    const BOOL normal=sample<5;
    const u32 level=normal ? sample : (super ? (sample-5)/3 : levels[(sample-5)/3]);
    const u32 tier=normal ? 0 : (sample-5)%3;
    const u32 samples=super ? 20 : 14;
    if (!step) {
        CFMtx43A transform = *actor->MtxToWorld();
        transform.m_vPos.y += 12.0f; // Keep the isolated ray above the shooter and partners.
        laser = fnew CWeaponLaser;
        target = fnew LaserRetailTarget;
        Check(laser && laser->Create("retail_charge_native_probe", &transform), "native retail probe creates real laser");
        Check(target && target->Create("gfh_box07", NULL, "retail_charge_native_target", &transform), "native retail probe creates collision target");
        if (!laser || !laser->IsCreated() || !target || !target->IsCreated()) {step=99;return;}
        laser->EnableAutoWork(FALSE);
        laser->SetOwner(actor);
        laser->RemoveFromWorld(); // The probe weapon is not the target's collision geometry.
        laser->ResetToState(CWeapon::STATE_DEPLOYED);
        target->EnableAutoWork(FALSE);
        target->Relocate_RotXlatFromUnitMtx_WS_NewScale_WS(&transform,0.015f,FALSE);
        target->SetCollisionFlag(TRUE);
        target->SetArmorProfile(&CDamage::m_ArmorProfileNone);
        target->SetInvincible(FALSE);
        target->SetHealthContainerCount(10.0f);
        target->SetMaxHealth();
        if (!target->IsInWorld()) target->AddToWorld();
        target->GetMesh()->UpdateTracker();
        origin = target->GetMesh()->GetBoundingSphere().m_Pos;
        step=1;
    }
    if (step == 1) {
        laser->SetUpgradeLevel(level);
        laser->GetMesh()->SetCollisionFlag(FALSE);
        laser->m_nChargedShotTier=tier;
        tracer_DestroyAllTracers(laser->m_hTracerGroup);
        target->SetCollisionFlag(TRUE);target->GetMesh()->UpdateTracker();
        target->SetMaxHealth();target->impacts=0;target->impactDamage=0.0f;target->profile=NULL;
        TracerDef_t shot=laser->m_aoTracerDef[level];
        if(normal) laser->_ConfigureNormalTracer(&shot);
        else laser->_ConfigureChargedTracer(&shot);
        shot.pUser=laser;
        shot.UnitDir_WS.Set(0.0f,0.0f,1.0f);
        shot.TailPos_WS=origin;shot.TailPos_WS.z-=4.0f;
        laser->ConstructDamagerData(&shot.Damager);
        CFVec3A rayEnd=origin;rayEnd.z+=4.0f;
        FCollImpact_t hit;
        FWorld_nTrackerSkipListCount=0;laser->_ChargedSkipList(laser);
        BOOL rayHit=fworld_FindClosestImpactPointToRayStart(&hit,&shot.TailPos_WS,&rayEnd,FWorld_nTrackerSkipListCount,FWorld_apTrackerSkipList,TRUE,NULL,-1,FCOLL_MASK_COLLIDE_WITH_THICK_PROJECTILES);
        DEVPRINTF("LASER-RETAIL probe target=(%.2f,%.2f,%.2f) radius=%.2f rayHit=%d ours=%d\n",origin.x,origin.y,origin.z,target->GetMesh()->GetBoundingSphere().m_fRadius,rayHit,rayHit&&hit.pTag==target->GetMesh());
        if(rayHit) DEVPRINTF("LASER-RETAIL ray hit=(%.2f,%.2f,%.2f) tag=%p expected=%p entity=%s targetInWorld=%d coll=%d mask=%x\n",hit.ImpactPoint.x,hit.ImpactPoint.y,hit.ImpactPoint.z,hit.pTag,target->GetMesh(),CGColl::ExtractEntity(&hit)?CGColl::ExtractEntity(&hit)->Name():"terrain",target->IsInWorld(),target->GetMesh()->IsCollisionFlagSet(),target->GetMesh()->m_pMesh->nMeshCollMask);
        Check(rayHit && hit.pTag==target->GetMesh(),"native charged ray collides with the physical test target");
        if (!rayHit || hit.pTag!=target->GetMesh()) {step=99;return;}
        Check(normal ? (super ? laser->m_aRetailCharge[level].apDamage[0]!=NULL : laser->m_apDamageProfile[level]!=NULL) : laser->m_aRetailCharge[level].apDamage[tier]!=NULL,"native launch damage profile is loaded");
        Check(tracer_NewTracer(laser->m_hTracerGroup,&shot,0.0f,FALSE,FALSE,&shot.Damager),"native real charged projectile launched");
        stamp=now;step=2;return;
    }
    if (step==2 && now-stamp>350) {
        const CDamageProfile *expected=normal && !super ? laser->m_apDamageProfile[level] : laser->m_aRetailCharge[level].apDamage[tier];
        DEVPRINTF("LASER-RETAIL owner=P%d variant=%u tier=%u impacts=%d damage=%.4f retail=%.4f health=%.4f\n",actor->m_nPossessionPlayerIndex+1,level,tier,target->impacts,target->impactDamage,expected->m_HitpointRange.m_fInnerValue,target->NormHealth());
        Check(target->impacts==1 && target->profile==expected,"native projectile submits exactly one impact with its launch profile");
        Check(fmath_Abs(target->impactDamage-expected->m_HitpointRange.m_fInnerValue)<.0001f,"native charged hit damage matches the retail profile through the real damage system");
        if (++sample<samples) {step=1;return;}
        fdelete(laser);fdelete(target);laser=NULL;target=NULL;
        DEVPRINTF("LASER-RETAIL COMPLETE owner=P%d\n",actor->m_nPossessionPlayerIndex+1);
        step=99;
    }
}
