// Opt-in probes of the reported campaign objects; no changes on ordinary boot.
#include "weapon_rivet.h"
#include "eproj_arrow.h"
static void PortBatchRegression(const char *mode,u32 now,int &step,u32 &stamp) {
    static f32 simulationTime=0;
    simulationTime+=FLoop_fPreviousLoopSecs;
    now=(u32)(1000.0f*simulationTime);
    const int owner=mode[strlen(mode)-1]-'1';
    static CDoorEntity *lift=NULL,*door=NULL;
    static CEntity *trigger=NULL;
    static CBot *mil=NULL;
    static BOOL moved=FALSE;
    static CWeaponRivet *rivet=NULL;
    static CEProj *arrow=NULL;
    static BOOL charged=FALSE,held=FALSE;
    static u32 nextRivetLog=0;
    static u32 arrowSeen=0;
    static u16 initialRivetAmmo=0;
    CBot *actor=(CBot*)Player_aPlayer[owner].m_pEntityOrig;
    if(!step){
        for(int n=0;n<CPlayer::m_nPlayerCount;++n)((CBot*)Player_aPlayer[n].m_pEntityOrig)->SetInvincible(TRUE);
        stamp=now;
        if(strstr(mode,"rivet")){
            CCollectable::NotifyCollectableUsedInWorld(COLLECTABLE_WEAPON_RIVET_GUN_L3);
            CCollectable::PostPlayerCreationSetup();
            if(!actor->m_pInventory->IsWeaponInInventory("Rivet Gun L3"))
                CCollectable::GiveWeaponToPlayer(actor,"Rivet Gun L3",20);
            CItemInst *item=actor->m_pInventory->IsWeaponInInventory("Rivet Gun L3");
            Check(item && item->m_pWeapon,"native Rivet L3 and its real projectile assets are available");
            if(!item||!item->m_pWeapon){step=99;return;}
            actor->m_pInventory->SetCurWeapon(INV_INDEX_PRIMARY,item,FALSE,TRUE);
            rivet=(CWeaponRivet*)item->m_pWeapon;rivet->SetClipAmmo(20,TRUE);initialRivetAmmo=rivet->GetClipAmmo();step=1;
        }else if(strstr(mode,"tether")){
            for(int n=0;n<CPlayer::m_nPlayerCount;++n){
                CBot *p=(CBot*)Player_aPlayer[n].m_pEntityOrig;
                CFMtx43A at=*p->MtxToWorld();at.m_vPos.x+=100*n;
                p->SetBotFlag_NoGravity();p->Relocate_RotXlatFromUnitMtx_WS(&at,FALSE);p->ZeroVelocity();
            }
            step=1;
        }else if(strstr(mode,"gifts")){
            for(int n=0;n<CPlayer::m_nPlayerCount;++n){
                CBot *p=(CBot*)Player_aPlayer[n].m_pEntityOrig;
                CItemInst *item=p->m_pInventory->IsWeaponInInventory("Flamer L1");
                Check(p->m_pInventory->m_uNumBatteries==5,"native gift probe begins before the authored starting battery");
                if(item && item->m_pWeapon){item->m_pWeapon->SetClipAmmo(0,TRUE);item->m_pWeapon->SetReserveAmmo(0);}
            }
            step=1;
        }else if(strstr(mode,"washer")){
            for(int n=0;n<CPlayer::m_nPlayerCount;++n)Player_aPlayer[n].m_Hud.SetWasherTimed(15);
            step=1;
        }else if(strstr(mode,"lift")){
            lift=(CDoorEntity*)CEntity::Find("m_elv_2a");trigger=CEntity::Find("elevdown");
            Check(lift && trigger,"native reported enemy lift and entrance trigger exist");
            if(!lift||!trigger){step=99;return;}
            for(int n=0;n<CPlayer::m_nPlayerCount;++n){
                CBot *p=(CBot*)Player_aPlayer[n].m_pEntityOrig;
                CFMtx43A at=*trigger->MtxToWorld();at.m_vPos.x-=70+8*n;
                p->SetBotFlag_NoGravity();p->Relocate_RotXlatFromUnitMtx_WS(&at,FALSE);p->ZeroVelocity();
            }
            DEVPRINTF("BATCH-LIFT initial %.3f locked=%d\n",lift->GetUnitPos(),lift->IsLocked());step=1;
        }else if(strstr(mode,"panel")){
            mil=(CBot*)CEntity::Find("hutgrunt01");trigger=CEntity::Find("hubgateswitch");
            door=(CDoorEntity*)CEntity::Find("hubgate01");
            Check(mil&&trigger&&door,"native Rendezvous Mil, security panel and door exist");
            if(!mil||!trigger||!door){step=99;return;}
            mil->SetInvincible(TRUE);mil->DataPort_Open(TRUE);mil->DataPort_SetPossessionDist(10000);
            mil->Possess(owner,.5f);
            Check(Player_aPlayer[owner].m_pEntityCurrent==mil,"native security test borrows a Mil for its actual player");
            step=1;
        }
        return;
    }
    if(strstr(mode,"rivet")){
        if(now>nextRivetLog){
            CEProj *diag=rivet->PortTestChargingArrow();
            if(diag){f32 life,charge;u32 type;((CEProj_Arrow*)diag->GetExtensionObject(CEProj::PROJTYPE_ARROW))->PortTestState(life,charge,type);
                DEVPRINTF("BATCH-ARROW life=%.3f charge=%.3f type=%u terminate=%d world=%d active=%d launched=%d work=%d\n",life,charge,type,((CEProj_Arrow*)diag->GetExtensionObject(CEProj::PROJTYPE_ARROW))->IsInTerminateMode(),diag->IsInWorld(),diag->IsActivePoolFlagSet(),diag->IsLaunched(),diag->GetClassHierarchyWorkMask());}
            DEVPRINTF("BATCH-RIVET sim=%u wall=%lu trigger=%.2f held=%d arrow=%p tier=%u ammo=%u\n",now,GetTickCount(),Player_aPlayer[owner].m_HumanControl.m_fFire1,held,rivet->PortTestChargingArrow(),rivet->GetUpgradeLevel(),rivet->GetClipAmmo());nextRivetLog=now+2000;
        }
        if(Player_aPlayer[owner].m_HumanControl.m_fFire1>.5f){
            held=TRUE;
            CEProj *p=rivet->PortTestChargingArrow();
            if(p){
                arrow=p;
                if(!arrowSeen)arrowSeen=now;
                CEProj_Arrow *ext=(CEProj_Arrow*)p->GetExtensionObject(CEProj::PROJTYPE_ARROW);
                if(ext && ext->IsInTerminateMode() && now-arrowSeen>3500)charged=TRUE;
            }
        }else if(held && arrow){
            Check(charged,"native held L3 Rivet charges its attached explosive arrow");
            Check(!rivet->PortTestChargingArrow() && !arrow->IsActivePoolFlagSet(),"native mouse release detonates the real charged Rivet arrow");
            Check(rivet->GetClipAmmo()==initialRivetAmmo-1,"native Rivet hold consumes one round");
            DEVPRINTF("BATCH COMPLETE %s\n",mode);step=99;
        }
        if(now-stamp>30000){Check(FALSE,"native Rivet hold obtains an attached arrow before timeout");step=99;}
    }else if(strstr(mode,"tether")){
        if(now-stamp>3500){
            const int previous=CPlayer::m_nCurrent;
            for(int n=0;n<CPlayer::m_nPlayerCount;++n){
                CFCamera *camera=fcamera_GetCameraByIndex(n);
                CReticle &reticle=Player_aPlayer[n].m_Reticle;
                const FViewport_t *view=camera->GetViewport();
                CFVec3A point,local;
                local.Set(reticle.GetNormOriginX()*4.0f*view->fTanHalfFOVX,reticle.GetNormOriginY()*4.0f*view->fTanHalfFOVY,4.0f);
                camera->GetFinalXfm()->m_MtxR.MulPoint(point,local);
                CFVec3A projected;
                fviewport_ComputeUnitOrtho3DScreenPoint_WS(view,&camera->GetFinalXfm()->m_MtxF,&point,&projected);
                DEVPRINTF("BATCH-TETHER player=%d projected=(%.3f,%.3f) origin=(%.3f,%.3f) unit=%.3f\n",n,projected.x,projected.y,reticle.GetNormOriginX(),reticle.GetNormOriginY(),reticle.ComputeUnitWithinReticle(view,&point));
                gamecam_SetActiveCamera(PLAYER_CAM((n+1)%CPlayer::m_nPlayerCount));gamecam_SetViewportAndInitCamStack();
                Check(Player_aPlayer[n].m_Reticle.IsPointWithinReticle(camera->GetViewport(),&point),"native lock projection uses its owner's camera while another player's view is active");
            }
            CPlayer::SetCurrent(previous);gamecam_SetActiveCamera(PLAYER_CAM(previous));gamecam_SetViewportAndInitCamStack();
            DEVPRINTF("BATCH COMPLETE %s\n",mode);step=99;
        }
    }else if(strstr(mode,"gifts")){
        if(now-stamp>1200 && now-stamp<3500){
            for(int n=1;n<CPlayer::m_nPlayerCount;++n){
                CBot *p=(CBot*)Player_aPlayer[n].m_pEntityOrig;
                p->Relocate_RotXlatFromUnitMtx_WS(Player_aPlayer[0].m_pEntityOrig->MtxToWorld());p->ZeroVelocity();
            }
        }
        if(now-stamp>5000){
            for(int n=0;n<CPlayer::m_nPlayerCount;++n){
                CBot *p=(CBot*)Player_aPlayer[n].m_pEntityOrig;
                CItemInst *item=p->m_pInventory->IsWeaponInInventory("Flamer L1");
                DEVPRINTF("BATCH-GIFT player=%d batteries=%u flamer=%d\n",n,p->m_pInventory->m_uNumBatteries,item?item->m_nUpgradeLevel:-1);
                Check(p->m_pInventory->m_uNumBatteries==6,"native Hand starting battery reaches each co-op player once");
                Check(item && item->m_pWeapon && item->m_pWeapon->GetClipAmmo()+item->m_pWeapon->GetReserveAmmo()>0,"native Hand starting Toaster gift reaches each co-op player");
            }
            DEVPRINTF("BATCH COMPLETE %s\n",mode);step=99;
        }
    }else if(strstr(mode,"washer")){
        if(now-stamp>2500){
            for(int n=0;n<CPlayer::m_nPlayerCount;++n)
                Check(Player_aPlayer[n].m_Hud.PortTestWasherLayout(),"native washer icon ends beside its own counter");
            DEVPRINTF("BATCH COMPLETE %s\n",mode);step=99;
        }
    }else if(strstr(mode,"lift")){
        if(step==1 && now-stamp>1000){
            actor->Relocate_RotXlatFromUnitMtx_WS(trigger->MtxToWorld());actor->ZeroVelocity();
            DEVPRINTF("BATCH-LIFT first player enters\n");step=2;stamp=now;
        }else if(step==2 && now-stamp>1500){
            for(int n=0;n<CPlayer::m_nPlayerCount;++n){
                CBot *p=(CBot*)Player_aPlayer[n].m_pEntityOrig;
                CFMtx43A at=*trigger->MtxToWorld();at.m_vPos.x+=2*n;
                p->Relocate_RotXlatFromUnitMtx_WS(&at);p->ZeroVelocity();
            }
            DEVPRINTF("BATCH-LIFT all players enter\n");step=3;stamp=now;
        }else if(step==3){
            moved|=lift->GetUnitPos()>.05f;
            if(now-stamp>10000){
                DEVPRINTF("BATCH-LIFT final %.3f locked=%d triggerWorld=%d\n",lift->GetUnitPos(),lift->IsLocked(),trigger->IsInWorld());
                Check(moved,"native enemy lift descends after the party enters its room");
                DEVPRINTF("BATCH COMPLETE %s\n",mode);step=99;
            }
        }
    }else if(strstr(mode,"panel")){
        if(step==1 && now-stamp>1000){
            CFMtx43A at=*trigger->MtxToWorld();at.m_vPos.x+=5;
            mil->Relocate_RotXlatFromUnitMtx_WS(&at);mil->ZeroVelocity();
            Check(trigger->ActionNearby(mil),"native possessed Mil operates the security panel");
            step=2;stamp=now;
        }else if(step==2 && now-stamp>6000){
            DEVPRINTF("BATCH-PANEL owner=%d current=%s locked=%d\n",owner,Player_aPlayer[owner].m_pEntityCurrent->Name(),door->IsLocked());
            Check(!door->IsLocked(),"native security panel unlocks the authored route for the party");
            Check(Player_aPlayer[owner].m_pEntityCurrent==mil,"native security panel preserves the Mil's real owner");
            DEVPRINTF("BATCH COMPLETE %s\n",mode);step=99;
        }
    }
}
