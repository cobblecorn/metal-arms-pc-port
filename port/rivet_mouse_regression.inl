// Opt-in real input/weapon probe. Never runs during an ordinary game.
#include "weapon_rivet.h"
#include "weapon_scope.h"
#include "eproj_arrow.h"
static void RivetMouseInputReport(CBot *actor,u32 now,int &step,u32 &stamp) {
    static CWeaponRivet *rivet=NULL;
    static CWeapon *grenade=NULL;
    static CWeaponScope *scope=NULL;
    static CEProj *arrow=NULL;
    static BOOL charged=FALSE,scopeStable=TRUE;
    static u16 initialAmmo=0;
    const f32 secondary=Player_aPlayer[0].m_HumanControl.m_fFire2;
    static u32 nextLog=0;
    if (rivet && now>nextLog) {
        DEVPRINTF("RIVET-INPUT state=%d right=%.2f ammo=%u grenades=%u supported=%d mouse=%d tap=%d weaponstate=%d\n",
            step,secondary,rivet->GetClipAmmo(),grenade->GetClipAmmo(),rivet->SupportsPlayerCharge(),
            rivet->UsesMouseChargeControls(),rivet->PlayerSecondaryTap(),rivet->CurrentState());
        if (CEProj *p=rivet->PortTestChargingArrow()) {
            f32 life,power;u32 type;
            CEProj_Arrow *ext=(CEProj_Arrow*)p->GetExtensionObject(CEProj::PROJTYPE_ARROW);
            ext->PortTestState(life,power,type);
            DEVPRINTF("RIVET-INPUT arrow life=%.3f charge=%.3f type=%u terminate=%d world=%d active=%d launched=%d work=%u auto=%d enabled=%d parent=%s\n",
                life,power,type,ext->IsInTerminateMode(),p->IsInWorld(),p->IsActivePoolFlagSet(),p->IsLaunched(),
                p->GetClassHierarchyWorkMask(),p->IsAutoWorkEnabled(),p->IsWorkEnabled(),p->GetParent()?p->GetParent()->Name():"world");
        }
        nextLog=now+1000;
    }
    if (!step) {
        CCollectable::NotifyCollectableUsedInWorld(COLLECTABLE_WEAPON_RIVET_GUN_L3);
        CCollectable::NotifyCollectableUsedInWorld(COLLECTABLE_WEAPON_SCOPE_L1);
        CCollectable::NotifyCollectableUsedInWorld(COLLECTABLE_WEAPON_CORING_CHARGE);
        CCollectable::PostPlayerCreationSetup();
        CWeapon *primary=actor->GetPrimaryWeapon();
        if (!primary || primary->Type()!=CWeapon::WEAPON_TYPE_RIVET_GUN || primary->GetUpgradeLevel()!=2) {
            CCollectable::GiveWeaponToPlayer(actor,"Rivet Gun L3",20);
            CItemInst *item=actor->m_pInventory->IsWeaponInInventory("Rivet Gun L3");
            if (item && item->m_pWeapon) actor->m_pInventory->SetCurWeapon(INV_INDEX_PRIMARY,item,FALSE,TRUE);
            primary=actor->GetPrimaryWeapon();
        }
        Check(primary && primary->Type()==CWeapon::WEAPON_TYPE_RIVET_GUN && primary->GetUpgradeLevel()==2,
            "native right-hold test has the L3 remote explosive Rivet equipped");
        if (!primary || primary->Type()!=CWeapon::WEAPON_TYPE_RIVET_GUN) {step=99;return;}
        rivet=(CWeaponRivet*)primary;
        Check(CCollectable::GiveWeaponToPlayer(actor,"Coring Charge",3),"native Rivet test has grenades");
        CItemInst *item=actor->m_pInventory->IsWeaponInInventory("Coring Charge");
        if (!item || !item->m_pWeapon) {step=99;return;}
        actor->m_pInventory->SetCurWeapon(INV_INDEX_SECONDARY,item,FALSE,TRUE);
        grenade=item->m_pWeapon;grenade->SetClipAmmo(3,TRUE);
        rivet->SetClipAmmo(5,TRUE);initialAmmo=rivet->GetClipAmmo();
        actor->SetInvincible(TRUE);step=1;stamp=now;
    } else if (step==1 && grenade->GetClipAmmo()==2) {
        Check(rivet->GetClipAmmo()==initialAmmo,"native Rivet right tap throws one grenade without primary fire");
        DEVPRINTF("RIVET-INPUT grenade tap ammo=%u grenades=%u\n",rivet->GetClipAmmo(),grenade->GetClipAmmo());
        step=2;stamp=now;
    } else if (step==2 || step==4) {
        if (step==4) scopeStable=scopeStable && scope->IsZoomEnabled();
        if (secondary>.25f) {
            if (CEProj *p=rivet->PortTestChargingArrow()) {
                if (arrow!=p) {
                    Check(!p->RemovedOnParentsDetach(),"native embedded rivet remains active when its hit object disappears");
                    CEntity *parent=p->GetParent();
                    if (parent && !(parent->TypeBits() & ENTITY_BIT_BOT)) {
                        parent->RemoveFromWorld(TRUE);
                        Check(!p->GetParent() && p->IsInWorld() && p->IsActivePoolFlagSet(),
                            "native rivet detaches from a removed temporary mesh without freezing");
                    }
                }
                arrow=p;
                f32 life,power;u32 type;
                ((CEProj_Arrow*)p->GetExtensionObject(CEProj::PROJTYPE_ARROW))->PortTestState(life,power,type);
                if (power>=.95f) charged=TRUE;
            }
        } else if (arrow) {
            Check(grenade->GetClipAmmo()==2,"native Rivet right hold never throws another grenade");
            if (step==4) Check(scopeStable,"native Rivet right hold preserves the selected scope state");
            Check(charged,"native Rivet right hold fully charges a real embedded explosive rivet");
            Check(!rivet->PortTestChargingArrow() && !arrow->IsActivePoolFlagSet(),
                "native right release detonates the real charged Rivet");
            Check(rivet->GetClipAmmo()==initialAmmo-1,"native Rivet right hold consumes one round");
            DEVPRINTF("RIVET-INPUT charged scope=%d ammo=%u grenades=%u\n",step==4,rivet->GetClipAmmo(),grenade->GetClipAmmo());
            if (step==4) {
                DEVPRINTF("RIVET-INPUT COMPLETE\n");step=99;return;
            }
            CCollectable::GiveWeaponToPlayer(actor,"Scope L1",-1);
            CItemInst *item=actor->m_pInventory->IsWeaponInInventory("Scope L1");
            Check(item && item->m_pWeapon,"native Rivet test has the real scope");
            if (!item || !item->m_pWeapon) {step=99;return;}
            actor->m_pInventory->SetCurWeapon(INV_INDEX_SECONDARY,item,FALSE,TRUE);
            scope=(CWeaponScope*)item->m_pWeapon;
            arrow=NULL;charged=FALSE;initialAmmo=rivet->GetClipAmmo();step=3;stamp=now;
        }
    } else if (step==3 && scope->IsZoomEnabled()) {
        Check(rivet->GetClipAmmo()==initialAmmo,"native Rivet right tap operates the selected scope without primary fire");
        DEVPRINTF("RIVET-INPUT scope tap enabled=1\n");step=4;stamp=now;
    }
    if (step && step!=99 && now-stamp>23000) {Check(FALSE,"native Rivet mouse input progresses within timeout");step=99;}
}
