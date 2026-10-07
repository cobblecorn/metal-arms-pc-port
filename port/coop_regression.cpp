// Opt-in integration checks against the running retail campaign. Ordinary play never runs these.
#include "fang.h"
#include "player.h"
#include "bot.h"
#include "botglitch.h"
#include "weapon.h"
#include "entity.h"
#include "collectable.h"
#include "ItemInst.h"
#include "ItemRepository.h"
#include "game.h"
#include "gamesave.h"
#include "BarterSystem.h"
#include "econsole.h"
#include "gamepad.h"
#include "Door.h"
#include "level.h"
#include "pc_cheats.h"
#include "vehiclerat.h"
#include "fworld_coll.h"
#include "launcher.h"
#include "ebox.h"
#include "AI/AIApi.h"
#include "AI/AIBrain.h"
#include "MAScriptTypes.h"
#include "GameCam.h"
#include "CamBot.h"
#include "FV3OConst.h"
#include "FQuatObj.h"
#include "FNativeUtil.h"
#include "BotTalkInst.h"
#include "TalkSystem2.h"
#include "faudio.h"
#include "fsndfx.h"
#include "FScriptSystem.h"
#include "SpyVsSpy.h"
#include "meshentity.h"
#include "AlarmSys.h"

static int Ammo(CBot *pBot, const char *pszTag = "coring charge") {
    CItemInst *pItem = pBot->m_pInventory->IsWeaponInInventory(pszTag);
    return pItem ? pItem->m_nClipAmmo + pItem->m_nReserveAmmo : 0;
}

static void EmptyCharges(CBot *pBot, const char *pszTag = "coring charge") {
    CItemInst *pItem = pBot->m_pInventory->IsWeaponInInventory(pszTag);
    if (pItem && pItem->m_pWeapon) {
        pItem->m_pWeapon->SetClipAmmo(0);
        pItem->m_pWeapon->SetReserveAmmo(0);
    }
}

static BOOL AllHaveAmmo(const char *pszTag, int count) {
    for (int n = 0; n < CPlayer::m_nPlayerCount; ++n)
        if (Ammo((CBot *)Player_aPlayer[n].m_pEntityOrig, pszTag) != count) return FALSE;
    return TRUE;
}

static void Check(BOOL bOK, const char *pszCheck) {
    DEVPRINTF("COOP-TEST %s: %s\n", bOK ? "PASS" : "FAIL", pszCheck);
}

void port_CoopRegressionWork() {
    static char mode[32];
    static BOOL bRead = FALSE;
    static u32 nStart = 0, nStepTime = 0, nControlReadyTime = 0;
    static int step = 0;
    static CFMtx43A origin;
    static CCollectable *pPickup = NULL;
    static CBot *pControlled = NULL;
    static CEntity *pOperatorBox = NULL;
    static int nShopWashers = 0, nPartnerWashers = 0;
    if (!bRead) {
        GetEnvironmentVariableA("MA_PORT_TEST_COOP_POLISH", mode, sizeof(mode));
        bRead = TRUE;
    }
    const BOOL spyRestart = !strcmp(mode,"spy-restart");
    const BOOL spyCraneDead = !strcmp(mode,"spy-crane-dead");
    const BOOL spyCrane = !strcmp(mode,"spy-crane") || spyCraneDead;
    const BOOL spyClaw = !strcmp(mode,"spy-claw") || spyCrane;
    const BOOL spyFixture = !strcmp(mode,"spy-control") || spyRestart || spyClaw;
    const BOOL coliseumGates = !strcmp(mode,"coliseum-gates");
    const BOOL soloFixture = !strncmp(mode,"fabricator",10) || !strncmp(mode,"rendezvous",10) || !strcmp(mode,"goff-parts") || !strcmp(mode,"spy-packing") || !strncmp(mode, "audio-", 6) || !strncmp(mode, "city-", 5) || spyFixture;
    if (!mode[0] || CPlayer::m_nPlayerCount < 1 || (CPlayer::m_nPlayerCount < 2 && !soloFixture && !coliseumGates) || step == 99) return;
    CBot *p0 = (CBot *)Player_aPlayer[0].m_pEntityOrig;
    CBot *p1 = CPlayer::m_nPlayerCount > 1 ? (CBot *)Player_aPlayer[1].m_pEntityOrig : p0;
    if (!p0 || !p1 || (!step && (!p0->IsInWorld() || !p1->IsInWorld()))) return;
    const u32 now = GetTickCount();
    if (!nStart) nStart = now;
    if (now - nStart < 1500 && strncmp(mode,"fabricator",10)) return;
    if (!step && !spyFixture && !coliseumGates && strncmp(mode,"fabricator",10)) {
        if (!Player_aPlayer[0].HasEntityControl()) { nControlReadyTime = 0; return; }
        if (!nControlReadyTime) nControlReadyTime = now;
        if (now - nControlReadyTime < 3000) return;
    }
    if (coliseumGates) {
        CDoorEntity *gate6=(CDoorEntity*)CEntity::Find("outerdoor6");
        CDoorEntity *gate3=(CDoorEntity*)CEntity::Find("outerdoor3");
        if(!step) {
            CEntity *trigger=CEntity::Find("trig_doorclose");
            if(Level_aInfo[Level_nLoadedIndex].nLevel!=LEVEL_COLISEUM_3 || !gate6 || !gate3 || !trigger) {
                Check(FALSE,"Coliseum 3 authored gates and battle trigger exist");step=99;return;
            }
            // Keep every player well away from both gates. Exercise the real
            // battle callback and its four-second delay, not a replacement script.
            for(int n=0;n<CPlayer::m_nPlayerCount;++n) {
                CBot *actor=(CBot*)Player_aPlayer[n].m_pEntityOrig;
                actor->SetInvincible(TRUE);actor->SetBotFlag_NoGravity();actor->EnableTripper(FALSE);
                CFMtx43A at=*actor->MtxToWorld();at.m_vPos.Set(-80.0f+12*n,8.0f,305.0f);
                actor->Relocate_RotXlatFromUnitMtx_WS(&at);actor->ZeroVelocity();
            }
            // The level starts with several arena gates open. Reset the two
            // tested gates so the request must actually move them.
            gate6->SnapToPos(CDoorEntity::DOORSTATE_ZERO);
            gate3->SnapToPos(CDoorEntity::DOORSTATE_ZERO);
            Check(gate3->GotoPos(CDoorEntity::DOORSTATE_ONE,CDoorEntity::GOTOREASON_UNKNOWN),"script can open outerdoor3");
            CFScriptSystem::TriggerEvent(CFScriptSystem::GetEventNumFromName(ENTITY_TRIPWIRE_EVENT_NAME),
                (u32)CEntity::TRIPWIRE_EVENTTYPE_ENTER,(u32)trigger,(u32)p0);
            DEVPRINTF("COLISEUM-GATE-TEST real battle trigger dispatched\n");
            step=1;nStepTime=now;
        } else if(step==1 && now-nStepTime>6500) {
            Check(gate6->GetUnitPos()==1.0f,"outerdoor6 stays open beyond battle delay without nearby players");
            Check(gate3->GetUnitPos()==1.0f,"outerdoor3 stays open without nearby players");
            Check(gate3->GotoPos(CDoorEntity::DOORSTATE_ZERO,CDoorEntity::GOTOREASON_UNKNOWN),"script can close outerdoor3");
            step=2;nStepTime=now;
        } else if(step==2 && now-nStepTime>3500) {
            Check(gate3->GetUnitPos()==0.0f,"outerdoor3 closes on script command");
            Check(gate6->GotoPos(CDoorEntity::DOORSTATE_ZERO,CDoorEntity::GOTOREASON_UNKNOWN),"script can close outerdoor6");
            step=3;nStepTime=now;
        } else if(step==3 && now-nStepTime>3500) {
            Check(gate6->GetUnitPos()==0.0f,"Coliseum gate test complete");step=99;
        }
        return;
    }
    if (!strncmp(mode,"fabricator",10)) {
        static u32 lastReport=0;
        static BOOL released=FALSE;
        static BOOL buildingSeen=FALSE;
        static CFVec3A buildOrigin;
        const BOOL upper=!strcmp(mode,"fabricator-upper");
        CBot *bot=(CBot*)CEntity::Find(upper?"xa_autobot_group_jumptrooper3":"xa_autobot_group_jumptrooper2");
        if(!step) {
            Check(!_stricmp(Level_aInfo[Level_nLoadedIndex].pszWorldResName,"WECDsneak02"),"Night Sneak fabricator section loaded");
            if(!bot) { Check(FALSE,"authored Jumper exists");step=99;return; }
            for(int n=0;n<CPlayer::m_nPlayerCount;++n) {
                CBot *actor=(CBot*)Player_aPlayer[n].m_pEntityOrig;
                actor->SetInvincible(TRUE);
                actor->SetBotFlag_NoGravity();
                CFMtx43A at=*actor->MtxToWorld();
                if(upper)at.m_vPos.Set(-153.3f,-4.0f,675.73f+8*n);
                else at.m_vPos.Set(-47.75f+8*n,-163.0f,403.35f);
                actor->Relocate_RotXlatFromUnitMtx_WS(&at);actor->ZeroVelocity();
            }
            CAlarmNet *net=CAlarmNet::FindAlarmNet("Dispense_Net");
            if(net) {
                net->TurnOn(p0,&p0->MtxToWorld()->m_vPos);
                net->m_pIntruder=p0;net->m_uIntruderGUID=p0->Guid();net->m_LastIntruderPos_WS=p0->MtxToWorld()->m_vPos;
            }
            step=1;nStepTime=now;
        }
        if(bot && bot->IsInWorld() && bot->IsUnderConstruction()) { buildingSeen=TRUE;buildOrigin=bot->MtxToWorld()->m_vPos; }
        if(bot && bot->IsInWorld() && now-lastReport>1000) {
            const CFVec3A& at=bot->MtxToWorld()->m_vPos;
            DEVPRINTF("FABRICATOR-TEST bot=(%.2f,%.2f,%.2f) building=%d powered=%d thought=%s\n",at.x,at.y,at.z,bot->IsUnderConstruction(),bot->Power_IsPoweredUp(),CAIBrain::ThoughtTypeToString(bot->AIBrain()->GetCurThought()));
            if(buildingSeen && !bot->IsUnderConstruction() && at.DistSqXZ(buildOrigin)>144.0f)released=TRUE;
            lastReport=now;
        }
        if(step==1 && now-nStepTime>10000) {
            for(int n=0;n<CPlayer::m_nPlayerCount;++n) {
                CBot *actor=(CBot*)Player_aPlayer[n].m_pEntityOrig;
                CFMtx43A at=*actor->MtxToWorld();at.m_vPos.Set(-258.0f+8*n,-231.0f,784.0f);
                actor->Relocate_RotXlatFromUnitMtx_WS(&at);actor->ZeroVelocity();
            }
            DEVPRINTF("FABRICATOR-TEST players retreat to mission start\n");step=2;
        }
        if(now-nStepTime>25000) {
            Check(released,"Jumper exits fabricator with players nearby");step=99;
        }
    } else if (!strncmp(mode,"rendezvous",10)) {
        static CEConsole *console = NULL;
        static CEntity *pack = NULL;
        if (!step) {
            Check(!_stricmp(Level_aInfo[Level_nLoadedIndex].pszWorldResName,"WEMCcity_02"),"Secret Rendezvous retail level loaded");
            console = (CEConsole *)flinklist_GetNext(&CEConsole::m_ConsoleRoot,NULL);
            pack = CEntity::Find("detpack1");
            pControlled = (CBot *)CEntity::Find("controltitan");
            if (!console || !pack || !pControlled) { Check(FALSE,"authored console, Titan and DET pack exist");step=99;return; }
            for(CEntity *e=console->GetFirstChild(); e; e=console->GetNextChild(e))
                if(e->TypeBits() & ENTITY_BIT_BOX) { pOperatorBox=e;break; }
            if(!pOperatorBox) { Check(FALSE,"console operator area exists");step=99;return; }
            p0->m_pInventory->m_aoItems[INVPOS_CHIP].m_nClipAmmo=1;
            p0->m_pInventory->m_aoItems[INVPOS_DETPACK].m_nClipAmmo=1;
            for(int n=0;n<CPlayer::m_nPlayerCount;++n) ((CBot*)Player_aPlayer[n].m_pEntityOrig)->SetInvincible(TRUE);
            p0->SetBotFlag_NoGravity();p0->Relocate_RotXlatFromUnitMtx_WS(pOperatorBox->MtxToWorld());p0->ZeroVelocity();
            step=1;nStepTime=now;
        } else if(step==1 && now-nStepTime>800) {
            p0->Relocate_RotXlatFromUnitMtx_WS(pOperatorBox->MtxToWorld());p0->ZeroVelocity();
            if(pOperatorBox->ActionNearby(p0)) { Check(TRUE,"console accepts last authored chip");step=2;nStepTime=now; }
            else if(now-nStepTime>10000) { Check(FALSE,"console accepts chip");step=99; }
        } else if(step==2 && now-nStepTime>1500) {
            if(console->NumInsertedChips()==console->NumSockets()) {
                Check(TRUE,"last chip automatically starts possession");step=3;nStepTime=now;
            } else if(now-nStepTime>15000) { Check(FALSE,"chip insertion finishes");step=99; }
        } else if(step==3) {
            if(Player_aPlayer[0].m_pEntityCurrent==pControlled && Player_aPlayer[0].HasEntityControl()) {
                if(!nControlReadyTime)nControlReadyTime=now;
                if(now-nControlReadyTime>1500) {
                    Check(pControlled->IsPossessedByConsole(),"real console handoff gives P1 the Titan");
                    pControlled->SetInvincible(TRUE);pControlled->DataPort_SetPossessionDist(10000);
                    if(!strcmp(mode,"rendezvous-timer")) {
                        pControlled->m_pInventory->m_aoItems[INVPOS_DETPACK].m_nClipAmmo=1;
                        pControlled->Relocate_RotXlatFromUnitMtx_WS(pack->MtxToWorld());
                        Check(pack->ActionNearby(pControlled),"possessed Titan plants authored DET pack");
                        step=4;nStepTime=now;
                    } else {
                        CEntity *trigger=CEntity::Find("shhcut");
                        CFMtx43A at=*trigger->MtxToWorld();at.m_vPos=CEntity::Find("shhhgoto")->MtxToWorld()->m_vPos;at.m_vPos.x+=32;
                        for(int n=0;n<CPlayer::m_nPlayerCount;++n) {
                            CBot *actor=(CBot*)Player_aPlayer[n].m_pEntityCurrent;
                            actor->Relocate_RotXlatFromUnitMtx_WS(&at);actor->ZeroVelocity();
                        }
                        step=6;nStepTime=now;
                    }
                }
            } else nControlReadyTime=0;
            if(now-nStepTime>20000) { Check(FALSE,"console possession finishes in time");step=99; }
        } else if(step==4 && now-nStepTime>1500) {
            Player_aPlayer[0].ReturnToOriginalBot();
            Check(Player_aPlayer[0].m_pEntityCurrent==p0,"player returns to Glitch while DET pack is counting down");
            step=5;nStepTime=now;
        } else if(step==5) {
            // Cancel the normal console back-jump in this stationary fixture;
            // test-only NoGravity would otherwise carry solo P1 out of bounds.
            p0->Relocate_RotXlatFromUnitMtx_WS(pOperatorBox->MtxToWorld());p0->ZeroVelocity();
            if(now-nStepTime>35000) {
                Check(!pack->IsInWorld(),"authored DET pack finishes explosion and removal");
                Check(!(Player_aPlayer[0].m_Hud.GetDrawFlags() & CHud2::DRAW_ICON_TIMER),"DET timer is cleared despite changed possession ownership");
                step=99;
            }
        } else if(step==6 && now-nStepTime>2500) {
            CFMtx43A at=*CEntity::Find("shhhgoto")->MtxToWorld();
            for(int n=0;n<CPlayer::m_nPlayerCount;++n) {
                CBot *actor=(CBot*)Player_aPlayer[n].m_pEntityCurrent;
                actor->Relocate_RotXlatFromUnitMtx_WS(&at);actor->ZeroVelocity();
            }
            step=7;nStepTime=now;
        } else if(step==7) {
            if(CBot::m_bCutscenePlaying) {
                Check(Player_aPlayer[0].m_pEntityCurrent==p0,"ending restores Glitch as dialogue actor");
                step=99;
            } else if(now-nStepTime>15000) { Check(FALSE,"retail ending cutscene starts");step=99; }
        }
        return;
    }
    if(!strcmp(mode,"spy-box-resume")) {
        // One-run user resume at packing, with normal story behavior afterward.
        // This is outside the regression fixture's invincibility/destruction path.
        if(!CSpyVsSpy::PortTestStage(3)) return;
        CEntity *crate=CSpyVsSpy::FindEntity("funnyname",ENTITY_BIT_MESHENTITY);
        p0->Relocate_Xlat_WS(&crate->MtxToWorld()->m_vPos);p0->ZeroVelocity();
        DEVPRINTF("Factory resume: skipped completed instruction/combat; real box packing started.\n");
        step=99;return;
    }
    if(!strcmp(mode,"spy-resume")) {
        // User-requested one-run skip; ordinary boots never take this branch.
        CSpyVsSpy::PortTestStage(1);
        DEVPRINTF("Factory resume: skipped completed instruction; combat test ready.\n");
        step=99;return;
    }
    p0->SetInvincible(TRUE);
    p1->SetInvincible(TRUE);
    const int previous = CPlayer::m_nCurrent;
    const int owner = (spyFixture && CPlayer::m_nPlayerCount>1) || mode[strlen(mode) - 1] == '2' ? 1 : 0;
    CBot *pOwner = owner ? p1 : p0;
    CBot *pPartner = owner ? p0 : p1;
    CPlayer::SetCurrent(owner);

    if(!strcmp(mode,"goff-parts")) {
        const char *names[3]={"goffhead","goffleg","gofftorso"};
        const int part=step/2;
        if(part>=3) { CPlayer::SetCurrent(previous);step=99;return; }
        const int recipient=part ? CPlayer::m_nPlayerCount-1 : 0;
        CBot *bot=(CBot *)Player_aPlayer[recipient].m_pEntityOrig;
        if(!(step&1)) {
            pPickup=NULL;
            for(CEntity *p=CEntity::InWorldList_GetHead();p;p=p->InWorldList_GetNext()) {
                if(!(p->TypeBits()&ENTITY_BIT_GOODIE)) continue;
                CCollectable *goodie=(CCollectable *)p;
                if(!fclib_stricmp(goodie->GetCollectableType()->m_pszName,names[part])) {pPickup=goodie;break;}
            }
            Check(pPickup!=NULL,"real authored Goff part exists");
            if(!pPickup) {step=99;CPlayer::SetCurrent(previous);return;}
            bot->SetInvincible(TRUE);
            bot->SetBotFlag_NoGravity();
            bot->Relocate_Xlat_WS(&pPickup->MtxToWorld()->m_vPos);
            bot->ZeroVelocity();
            DEVPRINTF("GOFF-TEST: move player %d to authored %s.\n",recipient+1,names[part]);
            ++step;nStepTime=now;
        } else if(now-nStepTime>1800) {
            // These retail quest props have no source inventory repository entry.
            // Verify consumption here; the runner checks one script event per part and Next.
            Check(!pPickup->IsInWorld(),"story pickup is consumed for the whole team");
            if(pPickup->IsInWorld()) {step=99;CPlayer::SetCurrent(previous);return;}
            ++step;nStepTime=now;
        }
        CPlayer::SetCurrent(previous);return;
    }
    if(!strcmp(mode,"spy-packing")) {
        if(step==5) for(s32 n=0;n<CPlayer::m_nPlayerCount;++n) {
            Player_aPlayer[n].m_HumanControl.Zero();
            Player_aPlayer[n].ZeroControls();
        }
        if(!step) {
            Check(CSpyVsSpy::PortTestStage(3),"fixture enters the real packing stage after completed combat");
            CEntity *crate=CSpyVsSpy::FindEntity("funnyname",ENTITY_BIT_MESHENTITY);
            Check(crate!=NULL,"retail packing box exists");
            pOperatorBox=crate;
            CMeshEntity *mesh=(CMeshEntity *)crate;
            Check(mesh->IsAnimDrivingMesh(),"retail packing box actively drives its animation");
            Check(mesh->UserAnim_GetCurrentInst()->GetUnitTime()<.99f,"fixture starts before box animation finishes");
            p0->Relocate_Xlat_WS(&crate->MtxToWorld()->m_vPos);p0->ZeroVelocity();
            origin=*p0->MtxToWorld();step=1;nStepTime=now;
        } else if(step==1 && now-nStepTime>500) {
            Check(!Player_aPlayer[0].HasEntityControl(),"packing takes P1 control after the real proximity trigger");
            for(s32 n=1;n<CPlayer::m_nPlayerCount;++n) {
                CBot *bot=(CBot *)Player_aPlayer[n].m_pEntityOrig;
                Check(!Player_aPlayer[n].HasEntityControl()&&!bot->IsDrawEnabled(),"packing safely holds hidden partners without movement control");
            }
            step=2;
        } else if(step==2 && Player_aPlayer[0].m_Hud.IsDrawEnabled()) {
            CMeshEntity *crate=(CMeshEntity *)CSpyVsSpy::FindEntity("funnyname",ENTITY_BIT_MESHENTITY);
            Check(crate->UserAnim_GetCurrentInst()->GetUnitTime()>=.99f,"box animation reaches its real completion threshold");
            Check(crate->MtxToWorld()->m_vPos.DistSq(origin.m_vPos)>900,"packing conveyor actually moves the box");
            Check(p0->MtxToWorld()->m_vPos.DistSq(origin.m_vPos)>900,"packed P1 arrives with the conveyor box");
            Check(p0->GetParent()==crate,"story player is securely carried by the box instead of wall collision");
            for(s32 n=0;n<CPlayer::m_nPlayerCount;++n) {
                CBot *bot=(CBot *)Player_aPlayer[n].m_pEntityOrig;
                Check(n==0 ? Player_aPlayer[n].HasEntityControl()&&Player_aPlayer[n].m_Hud.IsDrawEnabled()&&bot->IsDrawEnabled() : !Player_aPlayer[n].HasEntityControl()&&!bot->IsDrawEnabled(),"delivery restores P1 and holds partners until box exit");
            }
            CHumanControl held;held.m_fForward=1;held.m_fStrafeRight=1;held.m_fCrossDown=1;
            held.m_nPadFlagsJump=GAMEPAD_BUTTON_1ST_PRESS_MASK;held.m_fFire1=1;held.m_fAimDown=.5f;
            CSpyVsSpy::ConstrainPackingControls(p0,&held);
            Check(!held.m_fForward&&!held.m_fStrafeRight&&!held.m_nPadFlagsJump&&!held.m_fCrossDown,"packing rejects walking and jumping through box walls");
            Check(held.m_fFire1==1&&held.m_fAimDown==.5f,"packing preserves aiming and firing for authored breakout");
            step=4;nStepTime=now;
        } else if(step==4 && now-nStepTime>5000) {
            CMeshEntity *crate=(CMeshEntity *)CSpyVsSpy::FindEntity("funnyname",ENTITY_BIT_MESHENTITY);
            Check(p0->GetParent()==crate && p0->MtxToWorld()->m_vPos.DistSq(crate->MtxToWorld()->m_vPos)<16,"P1 remains contained through delivered dialogue without falling out");
            for(s32 n=1;n<CPlayer::m_nPlayerCount;++n) {
                CBot *bot=(CBot *)Player_aPlayer[n].m_pEntityOrig;
                Check(!Player_aPlayer[n].HasEntityControl()&&!bot->IsDrawEnabled(),"partners remain safely held throughout delivered dialogue");
                Check(fcamera_GetCameraByIndex(n)->GetXfmWithoutShake()==fcamera_GetCameraByIndex(0)->GetXfmWithoutShake(),"held partners watch the shared story camera after P1 regains controls");
            }
            step=3;nStepTime=now;
        } else if(step==2 && now-nStepTime>15000) {
            Check(FALSE,"packing finishes its animation and conveyor within timeout");step=99;
        } else if(step==3) {
            CMeshEntity *crate=(CMeshEntity *)CSpyVsSpy::FindEntity("funnyname",ENTITY_BIT_MESHENTITY);
            if(!crate->IsInvincible() || now-nStepTime>60000) {
                Check(!crate->IsInvincible(),"authored final dialogue makes the delivered box destructible");
                if(!crate->IsInvincible()) {
                    crate->SetNormHealth(0);crate->Die(FALSE,FALSE);
                    step=5;nStepTime=now;
                } else {launcher_EnterMenus(LAUNCHER_FROM_GAME);step=99;}
            }
        } else if(step==5 && now-nStepTime>5000) {
            // Death removes the box from world lookup; compare its saved identity.
            Check(p0->GetParent()!=pOperatorBox&&!p0->IsInAir(),"box death releases P1 onto checked ground without falling out");
            for(s32 n=0;n<CPlayer::m_nPlayerCount;++n) {
                CBot *bot=(CBot *)Player_aPlayer[n].m_pEntityOrig;
                Check(Player_aPlayer[n].HasEntityControl()&&Player_aPlayer[n].m_Hud.IsDrawEnabled()&&bot->IsDrawEnabled(),"box exit restores each player's visibility HUD and controls");
                Check(bot->IsReticleEnabled(),"box exit clears each player's forced crosshair suppression");
                Check(bot->MtxToWorld()->m_vPos.DistSq(p0->MtxToWorld()->m_vPos)<150&&!bot->IsInAir(),"box exit regroups every player on usable ground");
            }
            launcher_EnterMenus(LAUNCHER_FROM_GAME);step=99;
        }
    } else if(!strcmp(mode,"spy-inspection")) {
        if(!step) {
            CCollectable::GiveWeaponToPlayer(p0,"Wrench",1);
            Check(CSpyVsSpy::PortTestStage(2),"fixture enters real factory inspection stage");
            step=1;nStepTime=now;
        } else if(step==1 && !CSpyVsSpy::m_pCommonData->m_bHaveMessage && now-nStepTime>2000) {
            p0->SetInvincible(FALSE);p1->SetInvincible(FALSE);
            Check(CSpyVsSpy::PortTestInspection(0),"five open lockers trigger a warning instead of impostor combat");
            step=2;nStepTime=now;
        } else if((step==2 || step==3) && now-nStepTime>16000) {
            Check(CSpyVsSpy::PortTestInspection(1),"inspection warning closes every open locker and finishes its cinematic");
            Check(Player_aPlayer[0].HasEntityControl() && !p0->IgnoreControls(),"inspection warning returns P1 human control");
            Check(p0->ComputeUnitHealth()>0.9f,"inspection warning never shoots or lowers P1 health");
            for(s32 n=1;n<CPlayer::m_nPlayerCount;++n) {
                CBot *bot=(CBot *)Player_aPlayer[n].m_pEntityOrig;
                Check(Player_aPlayer[n].HasEntityControl() && bot->IsDrawEnabled(),"partners remain visible and controllable after inspection warning");
            }
            Check(!CSpyVsSpy::CoopIndividualViews(),"failed inspection has not auto-approved the chip or started escort");
            if(step==2) {
                Check(CSpyVsSpy::PortTestInspection(2),"repeated open-locker violation also retries safely");
                step=3;nStepTime=now;
            } else {
                Check(CSpyVsSpy::PortTestInspection(3) && Player_aPlayer[0].HasEntityControl(),"kill-state fallback with no open locker restores control and resumes search");
                Check(CCollectable::GiveToPlayer(p1,COLLECTABLE_CHIP,1.0f),"fixture awards a real objective chip to P2 through item pickup");
                step=4;nStepTime=now;
            }
        } else if(step==4 && now-nStepTime>1000) {
            // GiveToPlayer spawns a recipient-bound pickup. Inventory changes
            // on subsequent world updates, not in the grant call itself.
            CItemInst *chip=p1->m_pInventory->IsItemInInventory("chip");
            Check(chip && chip->m_nClipAmmo>=1,"P2 actually receives the spawned objective chip");
            Check(CSpyVsSpy::PortTestInspection(4),"only an actual partner chip permits inspection to advance to escort");
            step=99;
        }
    } else if(!strcmp(mode,"spy-escort")) {
        if(!step) {
            p1->m_fMountPitch_WS=-1;
            p0->m_fMountPitch_WS=.5f;
            CCollectable::GiveWeaponToPlayer(p0,"Empty Primary",1);
            CCollectable::GiveWeaponToPlayer(p0,"Wrench",1);
            Check(CSpyVsSpy::PortTestStage(2),"fixture enters post-assembly search/inspection stage");
            p1->m_fMountPitch_WS=-1;
            Check(p0->m_fMountPitch_WS==0,"post-rebuild camera resets P1 without importing partner pitch");
            origin=*p1->MtxToWorld();step=1;nStepTime=now;
        } else if(step==1 && now-nStepTime>1500) {
            const CFXfm *c0=fcamera_GetCameraByIndex(0)->GetOwnXfmWithoutShake();
            const CFXfm *c1=fcamera_GetCameraByIndex(1)->GetOwnXfmWithoutShake();
            Check(c0->m_MtxR.m_vFront.DistSq(c1->m_MtxR.m_vFront)>.1f,"forced inspection preserves different owning P1/P2 camera angles");
            Check(fcamera_GetCameraByIndex(0)->GetXfmWithoutShake()==c0,"forced inspection top view uses P1's owning camera");
            step=10;nStepTime=now;
        } else if(step==10 && !CSpyVsSpy::m_pCommonData->m_bHaveMessage) {
            CSpyVsSpy::TakeControlFromPlayer(TRUE);
            Check(CSpyVsSpy::PortTestEscort(0),"production escort gives partners their own walk to instructor room");
            step=2;nStepTime=now;
        } else if(step==2 && now-nStepTime>2500) {
            Check(!Player_aPlayer[1].HasEntityControl() && p1->Controls()!=&Player_aPlayer[1].m_HumanControl,"partner escort uses scripted AI controls");
            Check(p1->MtxToWorld()->m_vPos.DistSq(origin.m_vPos)>1,"partner actually walks along the escort route");
            Check(CSpyVsSpy::CoopIndividualViews(),"factory escort requests individual views");
            Check(fcamera_GetCameraByIndex(1)->GetXfmWithoutShake()==fcamera_GetCameraByIndex(1)->GetOwnXfmWithoutShake(),"escort partner keeps their own view while P1 is scripted");
            step=3;nStepTime=now;
        } else if(step==3 && CSpyVsSpy::PortTestEscort(1)) {
            Check(TRUE,"authored escort reaches instructor demonstration");
            step=4;nStepTime=now;
        } else if(step==4 && !CSpyVsSpy::CoopIndividualViews()) {
            Check(Player_aPlayer[1].HasEntityControl() && p1->Controls()==&Player_aPlayer[1].m_HumanControl,"authored escort end restores independent partner human controls");
            step=99;
        } else if(step==4 && now-nStepTime>90000) {Check(FALSE,"authored demonstration returns partner controls within timeout");step=99;
        } else if(step==3 && now-nStepTime>30000) {Check(FALSE,"escort reaches instructor demonstration within timeout");step=99;}
    } else if (!strcmp(mode,"spy-party")) {
        // Keep deadline fixtures independent of live controller input or stale
        // scripted movement. Targets and NPC progress still use production Work.
        if(step>=20 && step<=24) for(s32 n=0;n<CPlayer::m_nPlayerCount;++n) {
            Player_aPlayer[n].ZeroControls();Player_aPlayer[n].m_HumanControl.Zero();
            ((CBot *)Player_aPlayer[n].m_pEntityOrig)->ZeroVelocity();
        }
        if (!step) {
            Check(CSpyVsSpy::PortTestStage(0),"targeted fixture enters the real instructor stage");
            origin=*p0->MtxToWorld();
            for(int n=0;n<CPlayer::m_nPlayerCount;++n) {
                CBot *pBot=(CBot *)Player_aPlayer[n].m_pEntityOrig;
                CInventory *pInv=pBot->m_pInventory;
                Check(!Player_aPlayer[n].m_Hud.IsDrawEnabled() && !Player_aPlayer[n].m_Hud.IsWSEnabled(),"each instructor player has no HUD or weapon selection");
                Check(&pInv->m_aoWeapons[INV_INDEX_PRIMARY][pInv->m_auCurWeapon[INV_INDEX_PRIMARY]]==pInv->IsWeaponInInventory("Empty Primary") &&
                      &pInv->m_aoWeapons[INV_INDEX_SECONDARY][pInv->m_auCurWeapon[INV_INDEX_SECONDARY]]==pInv->IsWeaponInInventory("Empty Secondary"),"each instructor player is unarmed");
                Check(pBot->MtxToWorld()->m_vPos.DistSq(origin.m_vPos)<225,"each instructor player starts on the training floor");
            }
            // Reproduce the previous death-spectator camera link and ensure snapshot independence.
            CFCamera *c0=fcamera_GetCameraByIndex(0),*c1=fcamera_GetCameraByIndex(1);
            c0->SetViewSource(c1);
            Check(c0->GetOwnXfmWithoutShake()!=c0->GetXfmWithoutShake(),"factory transition snapshots use the owner even while watching P2");
            Check(game_GetStoryPlayerIndex()==0,"factory cutscene actor remains P1");
            c0->SetViewSource(NULL);
            step=1;nStepTime=now;
        } else if(step==1 && now-nStepTime>1000) {
            CFMtx43A staleSpawn=*p1->MtxToWorld();staleSpawn.m_vPos.Set(271,-13,188);p1->Relocate_RotXlatFromUnitMtx_WS(&staleSpawn);
            Check(checkpoint_Saved(0),"opening checkpoint exists for the real instructor restart");
            checkpoint_Restore(0,FALSE,"test:factory-instructor-party");
            step=10;nStepTime=now;
        } else if(step==10 && now-nStepTime>2000) {
            Check(p1->MtxToWorld()->m_vPos.DistSq(p0->MtxToWorld()->m_vPos)<100,"instructor restart replaces partners' stale opening spawn");
            CSpyVsSpy::PortTestStage(0);
            Check(CSpyVsSpy::PortTestDance(0),"instructor fatal failure requests a whole-team retry");
            step=2;nStepTime=now;
        } else if(step==2 && now-nStepTime>3000) {
            Check(CSpyVsSpy::PortTestDance(1),"team retry resumes the instructional minigame");
            Check(!p0->IsDeadOrDying() && !p1->IsDeadOrDying(),"team retry recovers both living bodies");
            Check(CSpyVsSpy::PortTestDance(2),"production dance validator accepts independent player lanes");
            Check(Player_aPlayer[0].HasEntityControl() && Player_aPlayer[1].HasEntityControl(),"both players can perform the instructor commands");
            Check(CSpyVsSpy::PortTestDance(8),"co-op target accepts a 2.8-foot placement miss independently");
            CSpyVsSpy::PortTestDance(2);
            Check(CSpyVsSpy::PortTestDance(9),"announced command dialogue cannot reject a player turning early");
            CSpyVsSpy::PortTestDance(2);
            Check(CSpyVsSpy::PortTestDance(10),"co-op still rejects a persistent large placement miss");
            CSpyVsSpy::PortTestDance(2);
            Check(CSpyVsSpy::PortTestDance(4),"P1 jump does not satisfy P2's jump command");
            Check(CSpyVsSpy::PortTestDance(5),"P2 independently satisfies their own jump command");
            CSpyVsSpy::PortTestDance(2);
            Check(CSpyVsSpy::PortTestDance(3),"P2's wrong turn triggers the instructor while P1 is correct");
            Check(CSpyVsSpy::PortTestDance(11),"actual short retail step command starts with both players idle");
            step=20;nStepTime=now;
        } else if(step==20 && now-nStepTime>200) {
            Check(CSpyVsSpy::PortTestDance(12),"each player's distinct hologram flashes at their own destination");
            step=24;
        } else if(step==24 && now-nStepTime>800) {
            Check(CSpyVsSpy::PortTestDance(17),"all holograms fade away while the command response window is still active");
            step=21;
        } else if(step==21 && CSpyVsSpy::PortTestDance(13)) {
            Check(TRUE,"uninterrupted real command rejects standing still instead of auto-approving");
            Check(CSpyVsSpy::PortTestDance(14),"real command fixture moves P1 correctly while P2 stays idle");
            step=22;nStepTime=now;
        } else if(step==22 && CSpyVsSpy::PortTestDance(13)) {
            Check(TRUE,"P1's completed real step cannot approve idle P2");
            Check(CSpyVsSpy::PortTestDance(15),"real command fixture gives both players imperfect completed steps");
            step=23;nStepTime=now;
        } else if(step==23 && CSpyVsSpy::PortTestDance(16)) {
            Check(TRUE,"both imperfect completed steps pass through the real command advancement path");
            CSpyVsSpy::PortTestDance(2);
            p0->SetInvincible(FALSE); p0->Die(FALSE);
            step=3;nStepTime=now;
        } else if(step>=20 && step<=24 && now-nStepTime>15000) {
            Check(FALSE,"real command deadline fixture finishes within timeout");step=99;
        } else if(step==3 && now-nStepTime>7000) {
            Check(!p0->IsDeadOrDying() && !Player_aPlayer[0].CoopWaitingForCheckpoint(),"actual P1 death retries without needing P2 to die");
            Check(p1->MtxToWorld()->m_vPos.DistSq(p0->MtxToWorld()->m_vPos)<100,"actual death retry recovers partners on the training floor");
            CSpyVsSpy::PortTestDance(2);
            Check(CSpyVsSpy::PortTestDance(6),"successful instructor completion starts the authored exit");
            step=4;nStepTime=now;
        } else if(step==4 && (CSpyVsSpy::PortTestDance(7) || now-nStepTime>25000)) {
            Check(CSpyVsSpy::PortTestDance(7),"authored instructor exit reaches the combat stage");
            for(int n=0;n<CPlayer::m_nPlayerCount;++n) {
                CBot *pBot=(CBot *)Player_aPlayer[n].m_pEntityOrig;CInventory *pInv=pBot->m_pInventory;
                Check(Player_aPlayer[n].m_Hud.IsDrawEnabled() && !Player_aPlayer[n].m_Hud.IsWSEnabled(),"combat restores each HUD while keeping the mission loadout");
                Check(&pInv->m_aoWeapons[INV_INDEX_PRIMARY][pInv->m_auCurWeapon[INV_INDEX_PRIMARY]]==pInv->IsWeaponInInventory("RLauncher L1"),"each combat player receives the mission launcher");
                Check(!pBot->ShouldIgnoreBotVBotCollisions(),"leaving instruction restores each player's normal collision");
                Check(pBot->MtxToWorld()->m_vPos.DistSq(p0->MtxToWorld()->m_vPos)<150,"each combat player regroups beyond the training room");
            }
            step=99;
        }
    } else if (spyFixture) {
        CSpyVsSpy::SpyCommonData_t *pCommon=CSpyVsSpy::m_pCommonData;
        if (!step) {
            Check(CSpyVsSpy::GetGlitch()==p0,"factory actor stays P1 after creating partners");
            Check(pCommon && pCommon->m_bHaveMessage && pCommon->m_bStreamBlock,
                  "factory opening transmission blocks the story stage");
            Check(!p0->Controls() || p0->Controls()!=&Player_aPlayer[0].m_HumanControl,
                  "factory opening gives P1 body to scripted AI");
            for(int n=0;n<CPlayer::m_nPlayerCount;++n) {
                Check(!Player_aPlayer[n].m_Hud.TransmissionMsg_IsDonePlaying(),
                      "Shhh opening transmission is active on each player HUD");
                CEntity *pBody=Player_aPlayer[n].m_pEntityOrig;
                BOOL antenna=FALSE;
                for(CEntity *pChild=pBody->GetFirstChild();pChild;pChild=pBody->GetNextChild(pChild))
                    if(pChild->IsInWorld() && (pChild->TypeBits() & ENTITY_BIT_MESHENTITY) &&
                       (((CMeshEntity *)pChild)->GetMeshInstFlags() & (FMESHINST_FLAG_POSTER_X|FMESHINST_FLAG_POSTER_Y)) ==
                       (FMESHINST_FLAG_POSTER_X|FMESHINST_FLAG_POSTER_Y)) antenna=TRUE;
                Check(antenna,"each Glitch has an active transmission antenna effect");
            }
            step=1; nStepTime=now;
        } else if(step==1 && pCommon && !pCommon->m_bHaveMessage) {
            Check(Player_aPlayer[0].HasEntityControl() &&
                  p0->Controls()==&Player_aPlayer[0].m_HumanControl,
                  "natural opening completion restores P1 human controls");
            for(int n=0;n<CPlayer::m_nPlayerCount;++n)
                Check(Player_aPlayer[n].m_Hud.TransmissionMsg_IsDonePlaying(),
                      "shared transmission finishes on every HUD");
            const BOOL partnerControl=Player_aPlayer[owner].HasEntityControl();
            CEntityControl *pPartnerControls=pOwner->Controls();
            CSpyVsSpy::TakeControlFromPlayer(TRUE);
            Check(!Player_aPlayer[0].HasEntityControl() &&
                  p0->Controls()!=&Player_aPlayer[0].m_HumanControl,
                  "later factory handoff takes P1 despite current partner context");
            CSpyVsSpy::TakeControlFromPlayer(FALSE);
            Check(Player_aPlayer[0].HasEntityControl() &&
                  p0->Controls()==&Player_aPlayer[0].m_HumanControl,
                  "later factory handoff returns P1 controls");
            if(CPlayer::m_nPlayerCount>1) Check(Player_aPlayer[owner].HasEntityControl()==partnerControl &&
                pOwner->Controls()==pPartnerControls,"factory P1 handoff preserves partner controls");
            if(spyClaw) {
                CMeshEntity *pConsole=(CMeshEntity *)CEntity::FindInWorld("main_console");
                CMeshEntity *pClaw=(CMeshEntity *)CEntity::FindInWorld("conv_claw01");
                CEntity *pDispenser=CEntity::FindInWorld("conv_disp01");
                Check(pConsole && pConsole->GetMeshCount()>1 && pClaw && pDispenser,"retail crane machinery is present");
                if(!pConsole || !pClaw || !pDispenser) step=99;
                else {
                    pConsole->SelectMesh(1,FALSE,TRUE);
                    Check(CCollectable::GiveWeaponToPlayer(p0,"wrench",1),"grant retail wrench to factory actor");
                    CItemInst *pWrench=p0->m_pInventory->IsWeaponInInventory("wrench");
                    if(!pWrench) { Check(FALSE,"wrench inventory entry exists"); step=99; }
                    else {
                        p0->m_pInventory->SetCurWeapon(INV_INDEX_SECONDARY,pWrench,FALSE,TRUE);
                        const s32 bone=pClaw->GetMeshInst()->FindBone("cranedummy");
                        if(bone<0) { Check(FALSE,"retail head claw attachment bone exists"); step=99; }
                        else {
                            // The pickup grate is in front of the dispenser's root.
                            // Approach it before reading the claw's animated bones.
                            CFMtx43A at=*pDispenser->MtxToWorld(); at.m_vPos.y+=2;
                            CFVec3A front; front.Mul(at.m_vFront,-7.0f); at.m_vPos.Add(front);
                            DEVPRINTF("SPY-TEST head dispenser at (%.2f,%.2f,%.2f) claw at (%.2f,%.2f,%.2f)\n",
                                at.m_vPos.x,at.m_vPos.y,at.m_vPos.z,pClaw->MtxToWorld()->m_vPos.x,
                                pClaw->MtxToWorld()->m_vPos.y,pClaw->MtxToWorld()->m_vPos.z);
                            p0->Relocate_RotXlatFromUnitMtx_WS(&at,FALSE);
                            step=4;nStepTime=now;
                        }
                    }
                }
            } else if(spyRestart) {
                Check(checkpoint_Restore(0,FALSE,"test:spy-intro-restart"),
                      "request retail mission-start checkpoint restore");
                step=2; nStepTime=now;
            } else step=99;
        } else if(step==1 && now-nStepTime>40000) {
            Check(FALSE,"opening transmission finishes within 40 seconds"); step=99;
        } else if(step==2 && pCommon && pCommon->m_bHaveMessage) {
            Check(pCommon->m_bStreamBlock,"checkpoint restart replays blocking factory introduction");
            step=3; nStepTime=now;
        } else if(step==3 && pCommon && !pCommon->m_bHaveMessage) {
            Check(Player_aPlayer[0].HasEntityControl() && p0->Controls()==&Player_aPlayer[0].m_HumanControl,
                  "checkpoint introduction completion restores P1 controls");
            if(CPlayer::m_nPlayerCount>1) {
                Check(Player_aPlayer[owner].HasEntityControl() && pOwner->Controls()==&Player_aPlayer[owner].m_HumanControl,
                      "checkpoint restart leaves partner under human control");
                Check(!pOwner->IsInAir(),"partner settles onto factory floor after restart");
                DEVPRINTF("SPY-TEST partner pos=(%.2f,%.2f,%.2f) air=%d control=%d\n",
                    pOwner->MtxToWorld()->m_vPos.x,pOwner->MtxToWorld()->m_vPos.y,pOwner->MtxToWorld()->m_vPos.z,
                    pOwner->IsInAir(),Player_aPlayer[owner].HasEntityControl());
            }
            step=99;
        } else if(step==4 && now-nStepTime>3000) {
            step=7;nStepTime=now;
        } else if(step==7 && now-nStepTime>1000 && !p0->IsInAir()) {
            CMeshEntity *claw=(CMeshEntity *)CEntity::FindInWorld("conv_claw01");
            if(claw->UserAnim_GetCurrentInst()->GetTime()>3.0f) {
                CPlayer::SetCurrent(previous); return;
            }
            CWeapon *pWrench=p0->GetSecondaryWeapon();
            Check(pWrench && pWrench->Throwable_TriggerWork(1),"retail wrench disassembles P1 for the crane");
            step=5;nStepTime=now;
        } else if(step==5 && pCommon->m_bHaveHead) {
            Check(game_GetListenerOrientationCallback()==CSpyVsSpy::ListenerOrientationCallback,
                  "head pickup starts the authored crane camera and listener callback");
            step=6;nStepTime=now;
        } else if(step==5) {
            static u32 lastProbe=0;
            if(now-lastProbe>1000) {
                CMeshEntity *claw=(CMeshEntity *)CEntity::FindInWorld("conv_claw01");
                const s32 bone=claw->GetMeshInst()->FindBone("cranedummy");
                const CFVec3A &pos=claw->GetMeshInst()->GetBoneMtxPalette()[bone]->m_vPos;
                const CFVec3A &bot=p0->MtxToWorld()->m_vPos;
                DEVPRINTF("SPY-TEST pickup probe bot=(%.2f,%.2f,%.2f) bone=(%.2f,%.2f,%.2f) anim=%.3f pieces=%d ground=%d dist2=%.2f\n",
                    bot.x,bot.y,bot.z,pos.x,pos.y,pos.z,claw->UserAnim_GetCurrentInst()->GetTime(),
                    p0->IsInPieces(),p0->IsOnGroundInPieces(),bot.DistSq(pos));
                lastProbe=now;
            }
            if(now-nStepTime>40000) { Check(FALSE,"retail head claw picks up the disassembled actor within 40 seconds"); step=99; }
        } else if(step==6 && now-nStepTime>2500) {
            Check(pCommon->m_bHaveHead,"crane camera continues after every co-op listener has updated");
            if(CPlayer::m_nPlayerCount>1) Check(!p1->IsInPieces(),"factory disassembly leaves partner body intact");
            step=spyCrane?8:99;nStepTime=now;
        } else if(step==8 && now-nStepTime>12000) {
            Check(!p0->IsInPieces() && Player_aPlayer[0].HasEntityControl() &&
                p0->Controls()==&Player_aPlayer[0].m_HumanControl && !p0->IgnoreControls(),
                "headless body rebuilds and retains P1 human controls");
            origin=*p0->MtxToWorld();step=9;nStepTime=now;
        } else if(step==9 && now-nStart>105000) {
            Check(p0->MtxToWorld()->m_vPos.DistSq(origin.m_vPos)>1.0f,
                "normal keyboard movement moves the headless body while crane camera waits");
            CEntity *dispenser=CEntity::FindInWorld("conv_disp02");
            CFMtx43A at=*dispenser->MtxToWorld();at.m_vPos.y+=2;
            CFVec3A front;front.Mul(at.m_vFront,-7);at.m_vPos.Add(front);
            p0->Relocate_RotXlatFromUnitMtx_WS(&at,FALSE);
            step=10;nStepTime=now;
        } else if((step==10 || step==13) && now-nStepTime>2000 && !p0->IsInAir() && !p0->IsInPieces()) {
            CMeshEntity *claw=(CMeshEntity *)CEntity::FindInWorld(step==10?"conv_claw02":"conv_claw03");
            if(claw->UserAnim_GetCurrentInst()->GetTime()<=3.0f) {
                Check(p0->GetSecondaryWeapon()->Throwable_TriggerWork(1),"wrench disassembles the remaining body at next pickup grate");
                step=step==10?11:14;nStepTime=now;
            }
        } else if(step==11 && pCommon->m_bHaveTorso) {
            Check(pCommon->m_bHaveHead,"torso claw collects body while head stays on first crane");
            step=12;nStepTime=now;
        } else if(step==12 && now-nStepTime>10000 && !p0->IsInPieces()) {
            CEntity *dispenser=CEntity::FindInWorld("conv_disp03");
            CFMtx43A at=*dispenser->MtxToWorld();at.m_vPos.y+=2;
            CFVec3A front;front.Mul(at.m_vFront,-7);at.m_vPos.Add(front);
            p0->Relocate_RotXlatFromUnitMtx_WS(&at,FALSE);
            step=13;nStepTime=now;
        } else if(step==14 && pCommon->m_bHaveLegs) {
            Check(pCommon->m_bHaveHead && pCommon->m_bHaveTorso,"all three retail claws collect Glitch parts");
            step=15;nStepTime=now;
        } else if(step==9) {
            static u32 lastMoveProbe=0;
            if(now-lastMoveProbe>1000) {
                DEVPRINTF("SPY-TEST movement pos=(%.2f,%.2f,%.2f) forward=%.2f strafe=%.2f immobile=%d build=%d\n",
                    p0->MtxToWorld()->m_vPos.x,p0->MtxToWorld()->m_vPos.y,p0->MtxToWorld()->m_vPos.z,
                    Player_aPlayer[0].m_HumanControl.m_fForward,Player_aPlayer[0].m_HumanControl.m_fStrafeRight,
                    p0->IsImmobileOrPending(),p0->IsUnderConstruction());
                lastMoveProbe=now;
            }
        } else if(step==15 && !p0->GetSecondaryWeapon()) {
            Check(game_GetListenerOrientationCallback()!=CSpyVsSpy::ListenerOrientationCallback,
                "crane camera finishes and hands off to authored conveyor assembly stage");
            if(CPlayer::m_nPlayerCount>1) Check(!p1->IsInPieces(),"partner remains intact through entire crane transition");
            step=16;nStepTime=now;
        } else if(step==16) {
            CMeshEntity *builder=(CMeshEntity *)CEntity::FindInWorld("GlitchBuilder");
            if(builder && builder->UserAnim_GetCurrentIndex()==1) {
                Check(builder->UserAnim_GetCurrentInst()!=NULL,"retail builder assembly animation loads and starts");
                step=17;nStepTime=now;
            } else if(now-nStepTime>90000) {
                Check(FALSE,"conveyor reaches builder assembly within 90 seconds");step=99;
            }
        } else if(step==17 && Player_aPlayer[0].HasEntityControl() &&
            p0->Controls()==&Player_aPlayer[0].m_HumanControl && !p0->IgnoreControls() &&
            !(Player_aPlayer[0].m_uPlayerFlags & CPlayer::PF_DONT_CALL_WORK) &&
            !pCommon->m_bHaveHead && !pCommon->m_bHaveTorso && !pCommon->m_bHaveLegs) {
            Check(TRUE,"completed factory assembly restores intact P1 and human control");
            if(CPlayer::m_nPlayerCount>1) {
                Check(!p1->IsInPieces(),"partner stays intact through factory assembly");
                if(spyCraneDead) {
                    p1->SetInvincible(FALSE);p1->Die(FALSE,FALSE);
                    Check(p1->IsDeadOrDying(),"partner is down before post-assembly regroup");
                }
            }
            step=18;nStepTime=now;
        } else if(step==18 && pCommon->m_bHaveMessage) {
            Check(TRUE,"post-assembly Agent Shhh transmission starts before partner regroup");
            step=19;nStepTime=now;
        } else if(step==19 && !pCommon->m_bHaveMessage && Player_aPlayer[0].HasEntityControl()) {
            BOOL recovered=TRUE;
            for(s32 n=1;n<CPlayer::m_nPlayerCount;++n) {
                CBot *partner=(CBot *)Player_aPlayer[n].m_pEntityOrig;
                if(partner->IsDeadOrDying() || !partner->IsInWorld() || partner->IsInPieces() ||
                    partner->MtxToWorld()->m_vPos.DistSq(p0->MtxToWorld()->m_vPos)>100.0f) recovered=FALSE;
            }
            if(recovered) {
                Check(TRUE,"all active partners recover intact beside rebuilt P1 after Shhh transmission");
                step=20;nStepTime=now;
            }
        } else if(step==20 && now-nStepTime>2000) {
            for(s32 n=1;n<CPlayer::m_nPlayerCount;++n) {
                CBot *partner=(CBot *)Player_aPlayer[n].m_pEntityOrig;
                Check(Player_aPlayer[n].HasEntityControl() && partner->Controls()==&Player_aPlayer[n].m_HumanControl &&
                    !partner->IgnoreControls(),"recovered partner has independent human controls");
            }
            CBot *guide=(CBot *)CEntity::FindInWorld("evil_nate");
            Check(guide && guide->AIBrain(),"scripted Mil guide is present after factory regroup");
            CFMtx43A disturbed=*p0->MtxToWorld();
            disturbed.m_vPos.x+=6;
            disturbed.m_vFront.Mul(-1);disturbed.m_vRight.Mul(-1);
            p0->Relocate_RotXlatFromUnitMtx_WS(&disturbed,FALSE);
            if(CPlayer::m_nPlayerCount>1 && guide) {
                CFMtx43A beside=*guide->MtxToWorld();beside.m_vPos.x+=1;
                p1->Relocate_RotXlatFromUnitMtx_WS(&beside,FALSE);
            }
            step=21;nStepTime=now;
        } else if(step==21 && now-nStepTime>15000) {
            CBot *guide=(CBot *)CEntity::FindInWorld("evil_nate");
            Check(guide && guide->AIBrain() && guide->AIBrain()->GetCurThought()!=CAIBrain::TT_COMBAT,
                "Mil guide stays scripted with partner beside him and P1 outside inspection zone");
            Check(Player_aPlayer[0].HasEntityControl() && !p0->IgnoreControls(),
                "partner presence and inspection movement leave P1 responsive");
            step=99;
        } else if(step>=2 && now-nStepTime>40000) {
            Check(FALSE,spyClaw?"retail head claw picks up the disassembled actor within 40 seconds":
                  "checkpoint intro restarts and finishes within 40 seconds"); step=99;
        }
    } else if (!strcmp(mode,"coop-sensitivity")) {
        static CPlayerProfile personal[2],session[2];
        const CPlayerProfile *apPersonal[]={&personal[0],&personal[1]};
        CPlayerProfile *apSession[]={&session[0],&session[1]};
        for(int n=0;n<2;++n) {
            personal[n].InitNewProfile(FALSE);
            wcscpy(personal[n].m_SaveInfo.wszProfileName,L"same-profile");
            personal[n].m_SaveInfo.nStorageDeviceID=FSTORAGE_DEVICE_ID_XB_PC_HD;
            personal[n].m_Data.fUnitLookSensitivity=0.5f;
        }
        coopsave_Load(L"CoopSensitivityRegression");
        const GameSave_ProfileData_t original=*coopsave_GetOwnerData();
        coopsave_BeginSession(apPersonal,apSession,2,FALSE);
        session[0].m_Data.fUnitLookSensitivity=0.21f;
        session[1].m_Data.fUnitLookSensitivity=0.83f;
        session[0].m_Data.nCurrentLevel=12;
        memset(&session[0].m_Data.aLevelProgress[0].Inventory,0x5a,sizeof(session[0].m_Data.aLevelProgress[0].Inventory));
        Check(coopsave_SaveLookSensitivity(&session[0]) && coopsave_SaveLookSensitivity(&session[1]),
              "persist distinct co-op sensitivities for duplicate personal profiles");
        Check(coopsave_Load(L"CoopSensitivityRegression"),"reload co-op preferences from disk");
        GameSave_ProfileData_t expected=original;
        expected.fUnitLookSensitivity=0.21f;
        Check(!memcmp(&expected,coopsave_GetOwnerData(),sizeof(expected)),
              "settings save preserves campaign progress and inventory exactly");
        coopsave_BeginSession(apPersonal,apSession,2,FALSE);
        Check(session[0].m_Data.fUnitLookSensitivity==0.21f && session[1].m_Data.fUnitLookSensitivity==0.83f,
              "new co-op session restores each player's sensitivity");
        Check(personal[0].m_Data.fUnitLookSensitivity==0.5f && personal[1].m_Data.fUnitLookSensitivity==0.5f,
              "co-op sensitivity leaves personal solo and PvP settings intact");
        coopsave_EndSession(); step=99;
    } else if (!strncmp(mode, "city-", 5)) {
        CEntity *pCaptain = CEntity::Find("CapnPeanuts");
        if (!step) {
            const char *names[] = {"CapnPeanuts","SugarBaby","startgrunt","mrhitty","punchyjones",
                "escapedoor","levelend","killtheplayer","corrosivestart"};
            for (u32 n=0; n<sizeof(names)/sizeof(names[0]); ++n) {
                CEntity *p=CEntity::Find(names[n]);
                if(p) DEVPRINTF("CITY-TEST %s world=%d actionable=%d pos=(%.2f,%.2f,%.2f)\n",
                    names[n],p->IsInWorld(),p->IsActionable(),p->MtxToWorld()->m_vPos.x,
                    p->MtxToWorld()->m_vPos.y,p->MtxToWorld()->m_vPos.z);
            }
            Check(pCaptain && pCaptain->IsInWorld() && pCaptain->IsActionable(),"retail captain is present and actionable");
            if(!pCaptain) step=99;
            else {
                if(owner) CFScriptSystem::TriggerEvent(CFScriptSystem::GetEventNumFromName("action"),
                    (u32)pCaptain,(u32)pOwner,0);
                nStepTime=now; step=1;
            }
        } else if(step==1 && now-nStepTime>500) {
            if(owner) Check(!CBot::m_bCutscenePlaying,"retail script rejects the unadapted P2 action event");
            // Exercise nearby action at the captain's height; his raised alcove
            // has an edge, so waiting after this test teleport can drop below it.
            CFMtx43A at=*pCaptain->MtxToWorld(); at.m_vPos.x+=1.5f;
            pOwner->Relocate_RotXlatFromUnitMtx_WS(&at,FALSE);
            pOwner->NotifyActionButton();
            nStepTime=now; step=2;
        } else if(step==2 && now-nStepTime>500) {
            Check(CBot::m_bCutscenePlaying,"nearby action starts retail captain escape sequence");
            step=99;
        }
    } else if (!strcmp(mode, "audio-dialogue")) {
        static CBotTalkInst *pTalk = NULL;
        static f32 musicStarted = 0;
        CFAudioStream *pMusic = level_GetStreamByName("ser_reactor");
        if (!step) {
            pTalk = fnew CBotTalkInst;
            BOOL loaded = pTalk->Init("GL_13m2_130");
            Check(loaded, "load Glitch's late Slosh rescue dialogue");
            if (!loaded) step = 99;
            else {
                game_BeginCutScene("", TRUE);
                pTalk->Force2dAudio();
                pTalk->DuckAudioWhenPlaying(TRUE);
                Check(CTalkSystem2::SubmitTalkRequest(pTalk, p0), "start rescue dialogue through the talk system");
                musicStarted = pMusic ? pMusic->GetSecondsPlayed() : 0;
                nStepTime = now; step = 1;
            }
        } else if (step == 1 && now - nStepTime > 2400) {
            Check(pTalk->IsWorking(), "voice remains active after its 2.25-second animation schedule");
            step = 2;
        } else if (step == 2 && now - nStepTime > 3300) {
            Check(!pTalk->IsWorking(), "finite voice clip finishes naturally without stalling the scene");
            Check(pMusic && pMusic->GetState() == FAUDIO_STREAM_STATE_PLAYING && pMusic->GetSecondsPlayed() > musicStarted + 2,
                  "Reactor background track keeps playing through the dialogue");
            game_EndCutScene(TRUE);
            Check(CTalkSystem2::SubmitTalkRequest(pTalk, p0), "start another line for explicit cancellation");
            nStepTime = now; step = 3;
        } else if (step == 3 && now - nStepTime > 200) {
            CTalkSystem2::TerminateActiveTalk(pTalk);
            Check(!pTalk->IsWorking(), "explicit dialogue cancellation still stops promptly");
            fdelete(pTalk); pTalk = NULL;
            step = 99;
        }
    }
    if (!strcmp(mode, "audio-flamer")) {
        static CWeapon *pFlamer = NULL;
        static int ammoBefore = 0;
        CBot *pActor = (CBot *)Player_aPlayer[0].m_pEntityCurrent;
        if (!step) {
            pFlamer = pActor ? pActor->GetPrimaryWeapon() : NULL;
            Check(pFlamer && pFlamer->TypeBits() & ENTITY_BIT_WEAPONFLAMER, "Slosh's active weapon is the flamethrower");
            if (!pFlamer || !(pFlamer->TypeBits() & ENTITY_BIT_WEAPONFLAMER)) step = 99;
            else { ammoBefore = pFlamer->GetClipAmmo(); nStepTime = now; step = 1; }
        } else if (step == 1) {
            CFVec3A target = pActor->MtxToWorld()->m_vPos;
            CFVec3A forward = pActor->MtxToWorld()->m_vFront;
            forward.Mul(50); target.Add(forward);
            pFlamer->TriggerWork(1, 0, &target);
            pFlamer->Work();
            if (now - nStepTime > 1500) {
                Check(pFlamer->GetClipAmmo() < ammoBefore, "Slosh fires the flamethrower and consumes fuel");
                pFlamer->TriggerWork(0, 0, &target);
                pFlamer->Work();
                step = 99;
            }
        }
    }
    if (!strcmp(mode, "audio-capacity")) {
        static CFAudioEmitter *sounds[256];
        static CFAudioEmitter *voice = NULL;
        if (!step) {
            FSndFx_FxHandle_t fx = fsndfx_GetFxHandle("GL_13m2_130");
            CFVec3A distant = p0->MtxToWorld()->m_vPos; distant.x += 10000;
            int allocated = 0;
            for (int n=0; n<256; ++n) {
                sounds[n] = FSNDFX_ALLOCNPLAY3D(fx, &distant, 1, 1, 1, FAudio_EmitterDefaultPriorityLevel, FALSE);
                if (sounds[n]) ++allocated;
            }
            Check(allocated == 256, "256 distant sound instances fit alongside the loaded Reactor world");
            voice = FSNDFX_ALLOCNPLAY2D(fx, 1, 1, FAudio_EmitterDefaultPriorityLevel, 0, FALSE);
            Check(voice != NULL, "dialogue can still allocate during the effect burst");
            nStepTime = now; step = 1;
        } else if (step == 1 && now - nStepTime > 3300) {
            Check(voice && voice->GetState() == FAUDIO_EMITTER_STATE_STOPPED, "dialogue progresses to completion during the effect burst");
            for (int n=0; n<256; ++n) if (sounds[n]) { sounds[n]->Destroy(); sounds[n] = NULL; }
            if (voice) { voice->Destroy(); voice = NULL; }
            FSndFx_FxHandle_t fx = fsndfx_GetFxHandle("GL_13m2_130");
            voice = FSNDFX_ALLOCNPLAY2D(fx, 1, 1, FAudio_EmitterDefaultPriorityLevel, 0, FALSE);
            Check(voice != NULL, "released effect instances can be reused");
            if (voice) { voice->Destroy(); voice = NULL; }
            step = 99;
        }
    }
    if (!strcmp(mode, "pickups") || !strcmp(mode, "weapons") || !strcmp(mode, "wallets")) {
        for (int n = 0; n < CPlayer::m_nPlayerCount; ++n) Player_aPlayer[n].DisableEntityControl();
    }
    if (!strcmp(mode, "revive")) {
        Player_aPlayer[0].DisableEntityControl();
        if (step >= 4) Player_aPlayer[1].DisableEntityControl();
        if (step >= 2 && step <= 3) {
            // Hold the survivor in the air for a checkpoint request, matching the reported failure.
            CFMtx43A airborne = origin;
            airborne.m_vPos.y += 50;
            p0->Relocate_RotXlatFromUnitMtx_WS(&airborne, FALSE);
        }
    }

    if(!strcmp(mode,"encounter-group")) {
        static CEntity *trigger=NULL,*enemy=NULL;
        static CFMtx43A nearby;
        if(!step || step==10) {
            const char *name=step==10?"titan2trigger":"titan1trigger";
            trigger=CEntity::Find(name);enemy=CEntity::Find(step==10?"titan2":"titan1");
            if(!trigger || !enemy) {Check(FALSE,"retail encounter entities exist");step=99;}
            else {
                Check(trigger->IsInWorld() && !enemy->IsInWorld(),"retail encounter starts armed with enemy hidden");
                origin=*trigger->MtxToWorld();origin.m_vPos=trigger->TripwireBoundingSphere_WS()->m_Pos;
                if(trigger->TypeBits() & ENTITY_BIT_BOX) {
                    const CFVec3A *corners=((CEBox *)trigger)->Corner_WS();
                    origin.m_vPos=corners[0];origin.m_vPos.Add(corners[1]);origin.m_vPos.Mul(.5f);
                }
                Check(trigger->TripwireContainsPoint(origin.m_vPos),"real crossing target is inside original volume");
                nearby=origin;BOOL outside=FALSE;
                for(int axis=0;axis<2 && !outside;++axis) for(int sign=-1;sign<=1 && !outside;sign+=2)
                    for(int offset=2;offset<=11 && !outside;++offset) {
                        nearby=origin;
                        if(axis==0) nearby.m_vPos.x+=sign*offset;else nearby.m_vPos.z+=sign*offset;
                        outside=!trigger->TripwireContainsPoint(nearby.m_vPos);
                    }
                if(!outside) {Check(FALSE,"retail partner can stand outside volume within group radius");step=99;}
                else {
                    for(int n=0;n<2;++n) {
                        Player_aPlayer[n].DisableEntityControl();
                        ((CBot *)Player_aPlayer[n].m_pEntityOrig)->SetNoCollideStateAir(TRUE);
                    }
                    CFMtx43A away=origin;away.m_vPos.x+=40;
                    p1->Relocate_RotXlatFromUnitMtx_WS(step==10?&away:&nearby,FALSE);
                    p0->Relocate_RotXlatFromUnitMtx_WS(&origin,TRUE);
                    step=step==10?11:1;nStepTime=now;
                }
            }
        } else {
            p0->Relocate_RotXlatFromUnitMtx_WS(&origin,FALSE);
            if(step==1) p1->Relocate_RotXlatFromUnitMtx_WS(&nearby,FALSE);
            else if(step==11) {CFMtx43A away=origin;away.m_vPos.x+=40;p1->Relocate_RotXlatFromUnitMtx_WS(&away,FALSE);}
            else p1->Relocate_RotXlatFromUnitMtx_WS(&nearby,FALSE);
            if(now-nStepTime>1500) {
                if(step==1) {
                    Check(enemy->IsInWorld() && !trigger->IsInWorld(),"real Titan spawn releases when only P1 crosses with nearby P2");
                    Check(!trigger->TripwireContainsPoint(p1->MtxToWorld()->m_vPos),"P2 never crosses the spawn trigger");
                    step=10;
                } else if(step==11) {
                    Check(!enemy->IsInWorld() && trigger->IsInWorld(),"distant partner keeps real second Titan encounter held");
                    step=12;nStepTime=now;
                } else {
                    Check(enemy->IsInWorld() && !trigger->IsInWorld(),"real held encounter releases as P2 approaches without crossing");
                    Check(!trigger->TripwireContainsPoint(p1->MtxToWorld()->m_vPos),"approaching P2 remains outside original trigger");
                    step=99;
                }
            }
        }
    } else if (!strcmp(mode,"floor-pads1") || !strcmp(mode,"floor-pads-reactor1")) {
        static int pad=0;
        static f32 peak=0;
        static const char *pads1[]={"circtrig1","circtrig2","fliptrig"};
        static const char *doors1[]={"leftdoor","rightdoor","flipdoor"};
        // Focus on the reported separation: the two pads on door1. A fresh
        // load did not exercise every later pad; don't bypass their arming.
        static const char *reactorPads[]={"door1_triga","door1_trigb"};
        static const char *reactorDoors[]={"door1","door1"};
        const BOOL reactor=!strcmp(mode,"floor-pads-reactor1");
        const char *name=reactor?reactorPads[pad]:pads1[pad];
        CEntity *pPad=CEntity::Find(name);
        CDoorEntity *pDoor=(CDoorEntity *)CEntity::Find(reactor?reactorDoors[pad]:doors1[pad]);
        // Leaving P1 enabled keeps the views independent, so the distant
        // Reactor door is visible to P2 and receives normal animation work.
        for(int n=reactor?1:0;n<CPlayer::m_nPlayerCount;++n) Player_aPlayer[n].DisableEntityControl();
        if(!pPad || !pDoor) {Check(FALSE,"retail floor switch and structure found");step=99;}
        else if(!step || step==10) {
            for(int n=0;n<CPlayer::m_nPlayerCount;++n) {
                CBot *pPlayer=(CBot *)Player_aPlayer[n].m_pEntityOrig;
                pPlayer->SetInvincible(TRUE);
                CFMtx43A away=*pPlayer->MtxToWorld();
                pPlayer->Relocate_RotXlatFromUnitMtx_WS(&away,FALSE);
            }
            peak=0;
            if(reactor) pDoor->SnapToPos(0);
            origin=*p1->MtxToWorld();origin.m_vPos=pPad->TripwireBoundingSphere_WS()->m_Pos;
            if(pPad->TypeBits() & ENTITY_BIT_BOX) {
                const CFVec3A *corners=((CEBox *)pPad)->Corner_WS();
                origin.m_vPos=corners[0];origin.m_vPos.Add(corners[1]);origin.m_vPos.Mul(.5f);
            }
            DEVPRINTF("COOP-TEST floor '%s': pos=(%.1f,%.1f,%.1f) initial structure=%.2f.\n",name,origin.m_vPos.x,origin.m_vPos.y,origin.m_vPos.z,pDoor->GetUnitPosMapped());
            p1->Relocate_RotXlatFromUnitMtx_WS(&origin,TRUE);
            step=1;nStepTime=now;
        } else if(step==1) {
            p1->Relocate_RotXlatFromUnitMtx_WS(&origin,FALSE);
            peak=FMATH_MAX(peak,pDoor->GetUnitPosMapped());
            if(now-nStepTime>(reactor?1100u:2500u)) {
                DEVPRINTF("COOP-TEST floor '%s': structure=%.2f peak=%.2f wait=%d.\n",name,pDoor->GetUnitPosMapped(),peak,CEntity::CoopTripwireWaiting(p1));
                Check(peak>.05f && (reactor || !CEntity::CoopTripwireWaiting(p1)),"P2 alone operates floor structure while partners stay away");
                if(++pad==(reactor?2:3)) step=99; else step=10;
            }
        }
    } else if(!strcmp(mode,"camera-revive")) {
        static CFV3OConst scenePos;
        static CFQOTwirlY sceneQuat;
        if(!step) {
            origin=*p0->MtxToWorld();
            p0->SetInvincible(FALSE);p0->Die(FALSE,FALSE);
            nStepTime=now;step=1;
        } else if(step==1 && now-nStepTime>3500) {
            Check(game_GetStoryPlayerIndex()==1,"dead P1 selects P2 as story actor");
            game_BeginCutScene("",TRUE);
            CFVec3A pos=p1->MtxToWorld()->m_vPos;
            CFVec3A back=p1->MtxToWorld()->m_vFront;back.Mul(-14.0f);pos.Add(back);pos.y+=7.0f;
            CFQuatA rot;rot.BuildQuat(*p1->MtxToWorld());
            scenePos.Init(pos,FALSE);sceneQuat.Init(rot,0.0f,FALSE);sceneQuat.Pause();
            cell init[]={12,(cell)&scenePos,(cell)&sceneQuat,ConvertF32ToCell(45.0f)},activate[]={0};
            CMAST_CamWrapper::Cam_Init(NULL,init);
            CMAST_CamWrapper::Cam_Activate(NULL,activate);
            nStepTime=now;step=2;
        } else if(step==2 && now-nStepTime>4000) {
            GameCamType_e type;gamecam_GetCameraManByIndex(GAME_CAM_PLAYER_1,&type);
            Check(type==GAME_CAM_TYPE_MANUAL&&CBot::m_bCutscenePlaying&&p0->IsDeadOrDying(),"visible scripted scene runs while P1 remains dead");
            Check(fcamera_GetCameraByIndex(PLAYER_CAM(0))->GetFinalXfm()==fcamera_GetCameraByIndex(PLAYER_CAM(1))->GetFinalXfm(),"both screens share the live scene camera");
            cell end[]={0};CMAST_CamWrapper::Cam_Deactivate(NULL,end);
            game_EndCutScene(TRUE);
            nStepTime=now;step=3;
        } else if(step==3 && now-nStepTime>500) {
            Player_aPlayer[0].Resurrect();
            origin.m_vPos.x+=25.0f;
            p0->Relocate_RotXlatFromUnitMtx_WS(&origin,FALSE);
            p0->SetInvincible(TRUE);
            nStepTime=now;step=4;
        } else if(step==4) {
            p0->Relocate_RotXlatFromUnitMtx_WS(&origin,FALSE);
            if(now-nStepTime>1500) {
                GameCamType_e type;
                CCamBot *cam=(CCamBot *)gamecam_GetCameraManByIndex(GAME_CAM_PLAYER_1,&type);
                Check(type==GAME_CAM_TYPE_ROBOT_3RD,"manual scene restores P1 bot camera");
                const CFVec3A *look=cam->GetLookAtPoint();
                Check(look->DistSq(p0->MtxToWorld()->m_vPos)<look->DistSq(p1->MtxToWorld()->m_vPos),"revived P1 camera follows P1 rather than P2");
                Check(Player_aPlayer[0].HasEntityControl()&&Player_aPlayer[1].HasEntityControl(),"scene exit returns both players to independent gameplay");
                step=99;
            }
        }
    } else if (!strcmp(mode,"airship-combat")) {
        if(!step) {
            CEntity *pBattle=CEntity::Find("trig_3pred1");
            if(!pBattle) {Check(FALSE,"airship combat trigger found");step=99;}
            else {
                origin=*pBattle->MtxToWorld();
                if(pBattle->IsTripwire()) origin.m_vPos=pBattle->TripwireBoundingSphere_WS()->m_Pos;
                for(int n=0;n<CPlayer::m_nPlayerCount;++n) {
                    CBot *pPlayer=(CBot *)Player_aPlayer[n].m_pEntityOrig;
                    pPlayer->SetInvincible(TRUE);Player_aPlayer[n].DisableEntityControl();
                    pPlayer->Relocate_RotXlatFromUnitMtx_WS(&origin,TRUE);
                }
                BOOL ready=TRUE;
                const char* names[]={"pred_1","pred_2","pred_3"};
                for(int n=0;n<3;++n) {
                    CBot *pEnemy=(CBot *)CEntity::Find(names[n]);
                    if(!pEnemy || !pEnemy->AIBrain()) ready=FALSE;
                    else {if(!pEnemy->IsInWorld())pEnemy->AddToWorld(); ai_AssignGoal_Attack(pEnemy->AIBrain(),p0->Guid(),2);}
                }
                Check(ready,"three retail airships receive attack goals against co-op player");
                step=1;nStepTime=now;
            }
        } else if(step==1 && now-nStepTime>18000) {
            Check(p0->IsInWorld() && p1->IsInWorld(),"co-op airship encounter stays running");step=99;
        }
    } else if (!strcmp(mode, "rat-chase")) {
        CVehicleRat *pRat = (CVehicleRat *)CEntity::Find("player_rat");
        if (!pRat) { Check(FALSE,"chase story RAT exists"); step = 99; }
        else if (!step && pRat->GetGunnerBot() == p0 && p1->GetCurMech()) {
            Check(pRat->GetDriverBot() && !pRat->GetDriverBot()->IsPlayerBot(), "chase keeps NPC driver");
            BOOL allReady = TRUE;
            static const char *names[] = {"coop_chase_p2","coop_chase_p3","coop_chase_p4"};
            for (int n=1; n<CPlayer::m_nPlayerCount; ++n) {
                CBotSiteWeapon *pGun = (CBotSiteWeapon *)CEntity::Find(names[n-1]);
                CBot *pPlayer = (CBot *)Player_aPlayer[n].m_pEntityOrig;
                if (!pGun || pGun->GetDriverBot() != pPlayer || pPlayer->GetCurMech() != pGun) allReady = FALSE;
            }
            if (allReady) {
                Check(TRUE,"all extra players operate separate shared-RAT guns");
                BOOL allSeated = TRUE;
                for (int n=0; n<CPlayer::m_nPlayerCount; ++n) {
                    CBot *pPlayer = (CBot *)Player_aPlayer[n].m_pEntityOrig;
                    allSeated &= !pPlayer->m_pWorldMesh->IsCollisionFlagSet() &&
                        pPlayer->GetParent();
                }
                Check(allSeated,"every co-op gunner is seat-locked with body collision disabled");
                FWorld_nTrackerSkipListCount = 0; pRat->AppendTrackerSkipList();
                BOOL allSkipped = TRUE;
                for (int n=1; n<CPlayer::m_nPlayerCount; ++n) {
                    CBotSiteWeapon *pGun = (CBotSiteWeapon *)CEntity::Find(names[n-1]);
                    BOOL gunSkipped=FALSE, playerSkipped=FALSE;
                    for (u32 j=0; j<FWorld_nTrackerSkipListCount; ++j) {
                        gunSkipped |= FWorld_apTrackerSkipList[j] == pGun->m_pWorldMesh;
                        playerSkipped |= FWorld_apTrackerSkipList[j] == ((CBot *)Player_aPlayer[n].m_pEntityOrig)->m_pWorldMesh;
                    }
                    allSkipped &= gunSkipped && playerSkipped;
                }
                Check(allSkipped,"RAT aiming/projectile skip list includes every added gun and occupant");
                origin = *pRat->MtxToWorld(); step=1; nStepTime=now;
            }
        } else if (step == 1 && now-nStepTime > 6000) {
            Check(pRat->MtxToWorld()->m_vPos.DistSq(origin.m_vPos)>100, "NPC route advances with co-op gunners");
            Check(p1->GetCurMech() && p1->GetCurMech()->GetParent()==pRat, "partner stays mounted as story RAT moves");
            checkpoint_Save(1,FALSE);
            step=2; nStepTime=now;
        } else if (step == 2 && now-nStepTime > 1000) {
            Check(checkpoint_Saved(1),"mounted chase checkpoint saves");
            checkpoint_Restore(1,FALSE);
            step=3; nStepTime=now;
        } else if (step == 3 && now-nStepTime > 3000) {
            BOOL allRestored = TRUE;
            for (int n=0; n<CPlayer::m_nPlayerCount; ++n) {
                CBot *pPlayer = (CBot *)Player_aPlayer[n].m_pEntityOrig;
                allRestored &= pPlayer->GetCurMech() && pPlayer->GetParent() &&
                    !pPlayer->m_pWorldMesh->IsCollisionFlagSet();
            }
            Check(allRestored,"checkpoint restores every seated gunner with collision disabled");
            CBotSiteWeapon *pGun = (CBotSiteWeapon *)p1->GetCurMech();
            pGun->SetSiteWeaponDriver(NULL,FALSE);
            Check(p1->m_pWorldMesh->IsCollisionFlagSet() && !p1->GetParent(),
                  "gunner exit restores collision and releases seat lock");
            step=4; nStepTime=now;
        } else if (step == 4 && now-nStepTime > 2000) {
            DEVPRINTF("COOP-TEST chase: returning to menus for normal turret teardown.\n");
            launcher_EnterMenus(LAUNCHER_FROM_GAME);
            step=99;
        }
        if (!step && now-nStart > 20000) { Check(FALSE,"chase co-op guns board"); step=99; }
    } else if (!strcmp(mode, "pipe-lift")) {
        CDoorEntity *pLift = (CDoorEntity *)CEntity::Find("pipe_lift");
        if (!pLift) { Check(FALSE, "exit pipe lift exists"); step = 99; }
        else if (!step) {
            origin = pLift->AI_GetInitPos();
            CFMtx43A at = origin;
            at.m_vPos.y += 2;
            p0->Relocate_RotXlatFromUnitMtx_WS(&at, FALSE);
            step = 1; nStepTime = now;
        } else if (step == 1 && now - nStepTime > 8000) {
            DEVPRINTF("COOP-TEST exit pipe: position=%.3f P1 height=%.3f initial=%.3f\n",
                      pLift->GetUnitPosMapped(), p0->MtxToWorld()->m_vPos.y, origin.m_vPos.y);
            Check(pLift->GetUnitPosMapped() > .9f && p0->MtxToWorld()->m_vPos.y > origin.m_vPos.y + 35,
                  "exit pipe lift carries player to upper route");
            step = 99;
        }
    } else if (!strcmp(mode, "elevator")) {
        CEntity *pSetup = CEntity::Find("add_titan1");
        CDoorEntity *pLift = (CDoorEntity *)CEntity::Find("g_lift");
        CEntity *pFake = CEntity::Find("g_fakelift");
        if (!pSetup || !pLift || !pFake) {
            Check(FALSE, "You Know the Drill elevator entities found");
            step = 99;
        } else if (!step) {
            Check(!pLift->IsInWorld() && pFake->IsInWorld(), "retail fake lift present before encounter");
            p0->SetInvincible(FALSE);
            p0->Die(FALSE, FALSE);
            step = 1; nStepTime = now;
        } else if (step == 1 && now - nStepTime > 1500) {
            Check(p0->IsDeadOrDying() && !p1->IsDeadOrDying(), "P1 dead while P2 survives");
            CFMtx43A at = *p1->MtxToWorld();
            at.m_vPos = pSetup->MtxToWorld()->m_vPos;
            p1->Relocate_RotXlatFromUnitMtx_WS(&at);
            step = 2; nStepTime = now;
        } else if (step == 2 && now - nStepTime > 7500) {
            Check(pLift->IsInWorld() && !pFake->IsInWorld(), "P2 trigger completes encounter and activates real lift");
            // A checkpoint may revive P1. Keep P1 down for the actual lift ride.
            p0->SetInvincible(FALSE);
            p0->Die(FALSE, FALSE);
            origin = pLift->AI_GetInitPos();
            CFMtx43A at = origin;
            at.m_vPos.y += 2;
            p1->Relocate_RotXlatFromUnitMtx_WS(&at, FALSE);
            step = 3; nStepTime = now;
        } else if (step == 3 && now - nStepTime > 8000) {
            DEVPRINTF("COOP-TEST lift: position=%.3f, P2 height=%.3f, initial=%.3f\n",
                      pLift->GetUnitPosMapped(), p1->MtxToWorld()->m_vPos.y, origin.m_vPos.y);
            Check(p0->IsDeadOrDying() && pLift->GetUnitPosMapped() > 0.9f &&
                  p1->MtxToWorld()->m_vPos.y > origin.m_vPos.y + 50,
                  "surviving P2 rides lift with P1 dead");
            step = 99;
        }
    } else if (!strcmp(mode, "migration")) {
        static const char retired[][64] = { "nuke grenade", "water grenade" };
        static const char *retail[] = { "coring charge", "emp grenade" };
        for (int i = 0; i < 2; ++i) {
            CMemCardItemInst saved = {};
            saved.m_nItemNameCRC = fmath_Crc32(0, (const u8 *)retired[i], sizeof(retired[i]));
            saved.m_nUpgradeLevel = 1;
            saved.m_nClipAmmo = 2;
            saved.m_nReserveAmmo = 3;
            CItemInst restored;
            Check(CMemCardItemInst::RestoreInventory(&saved, &restored, p0->m_pInventory) &&
                  restored.m_pItemData == CItemRepository::RetrieveEntry(retail[i], NULL) &&
                  restored.m_nClipAmmo == 2 && restored.m_nReserveAmmo == 3,
                  i ? "saved Water Grenade restores as EMP with ammo" : "saved Nuke Grenade restores as Coring Charge with ammo");
        }
        Check(!CItemRepository::RetrieveEntry("nuke grenade", NULL) &&
              !CItemRepository::RetrieveEntry("water grenade", NULL), "experimental grenades are no longer registered");
        step = 99;
    } else if (!strcmp(mode, "pickups") || !strcmp(mode, "weapons")) {
        const char *pszTag = !strcmp(mode, "weapons") ? "Ripper L1" : "coring charge";
        const CollectableType_e eType = !strcmp(mode, "weapons") ? COLLECTABLE_WEAPON_RIPPER_L1 : COLLECTABLE_WEAPON_CORING_CHARGE;
        if (!step) {
            origin = *p0->MtxToWorld();
            CFMtx43A away = origin;
            away.m_vPos.x += 30;
            p1->Relocate_RotXlatFromUnitMtx_WS(&away);
            EmptyCharges(p0, pszTag); EmptyCharges(p1, pszTag);
            for (int n = 2; n < CPlayer::m_nPlayerCount; ++n) {
                CBot *pOther = (CBot *)Player_aPlayer[n].m_pEntityOrig;
                away.m_vPos.x += 8;
                pOther->Relocate_RotXlatFromUnitMtx_WS(&away);
                EmptyCharges(pOther, pszTag);
                pOther->SetInvincible(TRUE);
            }
            CFMtx43A at = origin;
            at.m_vPos.y += 3;
            Check(CCollectable::PlaceIntoWorld(eType, &at, NULL, 1.0f, 3), "spawn world weapon pickup");
            for (CEntity *p = CEntity::InWorldList_GetHead(); p; p = p->InWorldList_GetNext()) {
                if ((p->TypeBits() & ENTITY_BIT_GOODIE) && p->MtxToWorld()->m_vPos.DistSq(at.m_vPos) < 1) {
                    pPickup = (CCollectable *)p; break;
                }
            }
            step = 1; nStepTime = now;
        } else if (step == 1 && now - nStepTime > 1200) {
            // AddToWorld queues insertion until the following engine work pass.
            for (CEntity *p = CEntity::InWorldList_GetHead(); p; p = p->InWorldList_GetNext()) {
                if ((p->TypeBits() & ENTITY_BIT_GOODIE) && ((CCollectable *)p)->GetCollectableType()->m_eType == eType &&
                    ((CCollectable *)p)->GetCoopCollectedMask() == 1) {
                    pPickup = (CCollectable *)p; break;
                }
            }
            Check(Ammo(p0, pszTag) == 3 && Ammo(p1, pszTag) == 0, "P1 collected once; P2 inventory untouched");
            Check(pPickup && pPickup->IsInWorld() && pPickup->GetCoopCollectedMask() == 1, "pickup retained for P2");
            checkpoint_Save(1, FALSE);
            step = 2; nStepTime = now;
        } else if (step == 2 && now - nStepTime > 500) {
            p1->Relocate_RotXlatFromUnitMtx_WS(&origin);
            step = 3; nStepTime = now;
        } else if (step == 3 && now - nStepTime > 1200) {
            DEVPRINTF("COOP-TEST ammo after collection: P1=%d P2=%d\n", Ammo(p0, pszTag), Ammo(p1, pszTag));
            Check(Ammo(p0, pszTag) == 3 && Ammo(p1, pszTag) == 3, "both players receive full pickup ammo");
            if (CPlayer::m_nPlayerCount > 2) {
                Check(pPickup && pPickup->IsInWorld() && pPickup->GetCoopCollectedMask() == 3, "pickup retained for remaining partners");
                for (int n = 2; n < CPlayer::m_nPlayerCount; ++n)
                    Player_aPlayer[n].m_pEntityOrig->Relocate_RotXlatFromUnitMtx_WS(&origin);
                step = 31; nStepTime = now;
            } else {
                Check(pPickup && !pPickup->IsInWorld(), "pickup removed after all players collect");
                checkpoint_Restore(1, FALSE);
                step = 4; nStepTime = now;
            }
        } else if (step == 31 && now - nStepTime > 1200) {
            Check(AllHaveAmmo(pszTag, 3) && pPickup && !pPickup->IsInWorld(), "all partners receive ammo before pickup removal");
            checkpoint_Restore(1, FALSE);
            step = 4; nStepTime = now;
        } else if (step == 4 && now - nStepTime > 1000) {
            Check(Ammo(p0, pszTag) == 3 && Ammo(p1, pszTag) == 0 && pPickup && pPickup->GetCoopCollectedMask() == 1,
                  "checkpoint restores partial claims and separate inventories");
            for (int n = 1; n < CPlayer::m_nPlayerCount; ++n)
                Player_aPlayer[n].m_pEntityOrig->Relocate_RotXlatFromUnitMtx_WS(&origin);
            step = 5; nStepTime = now;
        } else if (step == 5 && now - nStepTime > 1200) {
            Check(AllHaveAmmo(pszTag, 3), "partners can collect again after restoring checkpoint");
            // Paid/scripted grants use the recipient path rather than team world ownership.
            for (int n = 0; n < CPlayer::m_nPlayerCount; ++n)
                EmptyCharges((CBot *)Player_aPlayer[n].m_pEntityOrig, pszTag);
            Check(CCollectable::GiveToPlayer(p1, eType, 1.0f), "spawn targeted grant for P2");
            step = 6; nStepTime = now;
        } else if (step == 6 && now - nStepTime > 1200) {
            Check(Ammo(p0, pszTag) == 0 && Ammo(p1, pszTag) > 0, "targeted grant cannot be stolen by P1");
            step = 99;
        }
    } else if (!strcmp(mode, "revive")) {
        if (!step) {
            origin = *p0->MtxToWorld();
            checkpoint_SetUnsaved(1);
            p0->m_pInventory->m_aoItems[INVPOS_WASHER].m_nClipAmmo = 21;
            p1->m_pInventory->m_aoItems[INVPOS_WASHER].m_nClipAmmo = 42;
            CFMtx43A deadzone = origin;
            deadzone.m_vPos.y -= 160;
            p1->Relocate_RotXlatFromUnitMtx_WS(&deadzone, FALSE);
            p1->SetInvincible(FALSE);
            p1->Die(FALSE, FALSE);
            p1->SetNoCollideStateAir(TRUE);
            step = 1; nStepTime = now;
        } else if (step == 1 && now - nStepTime > 12000) {
            Check(p1->IsDeadOrDying() && Player_aPlayer[1].CoopWaitingForCheckpoint() && !checkpoint_Saved(1),
                  "fallen player stays down before a checkpoint; no timed revival");
            p0->SetNoCollideStateAir(TRUE);
            p0->m_pWorldMesh->SetCollisionFlag(FALSE);
            step = 2; nStepTime = now;
        } else if (step == 2 && now - nStepTime > 800) {
            Check(p0->IsInAir(), "surviving partner is airborne when checkpoint fires");
            checkpoint_Save(1, FALSE);
            step = 3; nStepTime = now;
        } else if (step == 3 && now - nStepTime > 1200) {
            Check(p1->IsDeadOrDying() && !checkpoint_Saved(1), "unsafe revival and checkpoint save wait for landing");
            p0->SetNoCollideStateAir(FALSE);
            p0->m_pWorldMesh->SetCollisionFlag(TRUE);
            p0->Relocate_RotXlatFromUnitMtx_WS(&origin, FALSE);
            step = 4; nStepTime = now;
        } else if (step == 4 && now - nStepTime > 5000) {
            Check(!p1->IsDeadOrDying() && p1->IsInWorld() && checkpoint_Saved(1) &&
                  p1->MtxToWorld()->m_vPos.DistSq(p0->MtxToWorld()->m_vPos) < 100 &&
                  fmath_Abs(p1->MtxToWorld()->m_vPos.y - p0->MtxToWorld()->m_vPos.y) < 2,
                  "pending checkpoint revives partner on the playable route after landing");
            Check(p0->m_pInventory->m_aoItems[INVPOS_WASHER].m_nClipAmmo == 21 &&
                  p1->m_pInventory->m_aoItems[INVPOS_WASHER].m_nClipAmmo == 42,
                  "checkpoint revival preserves both personal wallets");
            checkpoint_Restore(1, FALSE);
            step = 5; nStepTime = now;
        } else if (step == 5 && now - nStepTime > 3000) {
            Check(!p1->IsDeadOrDying() && p1->IsInWorld() &&
                  p1->MtxToWorld()->m_vPos.DistSq(p0->MtxToWorld()->m_vPos) < 100 &&
                  fmath_Abs(p1->MtxToWorld()->m_vPos.y - p0->MtxToWorld()->m_vPos.y) < 2,
                  "restoring checkpoint keeps revived player out of the death location");
            step = 99;
        }
    } else if (!strcmp(mode, "wallets")) {
        if (!step) {
            origin = *p0->MtxToWorld();
            CFMtx43A away = origin;
            away.m_vPos.x += 30;
            p1->Relocate_RotXlatFromUnitMtx_WS(&away);
            p0->m_pInventory->m_aoItems[INVPOS_WASHER].m_nClipAmmo = 10;
            p1->m_pInventory->m_aoItems[INVPOS_WASHER].m_nClipAmmo = 20;
            CFMtx43A at = origin;
            at.m_vPos.y += 3;
            Check(CCollectable::PlaceIntoWorld("Washer", &at), "spawn washer for P1");
            step = 1; nStepTime = now;
        } else if (step == 1 && now - nStepTime > 1200) {
            Check(p0->m_pInventory->m_aoItems[INVPOS_WASHER].m_nClipAmmo == 11 &&
                  p1->m_pInventory->m_aoItems[INVPOS_WASHER].m_nClipAmmo == 20, "P1 washer pickup credits only P1 wallet");
            CFMtx43A at = *p1->MtxToWorld(); at.m_vPos.y += 3;
            Check(CCollectable::PlaceIntoWorld("Washer", &at), "spawn washer for P2");
            step = 2; nStepTime = now;
        } else if (step == 2 && now - nStepTime > 1200) {
            Check(p0->m_pInventory->m_aoItems[INVPOS_WASHER].m_nClipAmmo == 11 &&
                  p1->m_pInventory->m_aoItems[INVPOS_WASHER].m_nClipAmmo == 21, "P2 washer pickup credits only P2 wallet");
            checkpoint_Save(1, FALSE);
            step = 3; nStepTime = now;
        } else if (step == 3 && now - nStepTime > 500) {
            p0->m_pInventory->m_aoItems[INVPOS_WASHER].m_nClipAmmo = 1;
            p1->m_pInventory->m_aoItems[INVPOS_WASHER].m_nClipAmmo = 2;
            checkpoint_Restore(1, FALSE);
            step = 4; nStepTime = now;
        } else if (step == 4 && now - nStepTime > 1000) {
            Check(p0->m_pInventory->m_aoItems[INVPOS_WASHER].m_nClipAmmo == 11 &&
                  p1->m_pInventory->m_aoItems[INVPOS_WASHER].m_nClipAmmo == 21, "checkpoint restores both separate wallets");
            step = 99;
        }
    } else if (!strncmp(mode, "possess", 7) || !strncmp(mode, "fast", 4)) {
        const BOOL bConsoleEntry = !strncmp(mode, "possess", 7);
        CEConsole *pConsole = (CEConsole *)CEntity::Find("olgrunty");
        CEntity *pTrigger = CEntity::Find("gruntdeath");
        if (!step) {
            pControlled = (CBot *)CEntity::Find("xa_autobot_grunt27");
            if (!pConsole || !pTrigger || !pControlled) {
                Check(FALSE, "Mines 2 possession fixtures exist"); step = 99;
            } else {
                pControlled->SetUnitHealth(1);
                if (bConsoleEntry) {
                    for (CEntity *p = pConsole->GetFirstChild(); p; p = pConsole->GetNextChild(p)) {
                        if (p->TypeBits() & ENTITY_BIT_BOX) { pOperatorBox = p; break; }
                    }
                    if (!pOperatorBox) { Check(FALSE, "console operator region exists"); step = 99; }
                    else {
                        pConsole->SetScriptEnabled(TRUE);
                        pOwner->m_pInventory->m_aoItems[INVPOS_CHIP].m_nClipAmmo = 5;
                        pOwner->Relocate_RotXlatFromUnitMtx_WS(pOperatorBox->MtxToWorld());
                        pControlled->SetInvincible(TRUE);
                    }
                } else {
                    pControlled->AddToWorld();
                    pControlled->Possess(owner, 1);
                    pControlled->DataPort_SetPossessionDist(10000);
                    CFMtx43A outside = *pTrigger->MtxToWorld();
                    outside.m_vPos = pTrigger->TripwireBoundingSphere_WS()->m_Pos;
                    outside.m_vPos.x += 80;
                    pControlled->Relocate_RotXlatFromUnitMtx_WS(&outside);
                }
                if (step != 99) { step = bConsoleEntry ? 10 : 1; nStepTime = now; }
            }
        } else if (step == 10 && now - nStepTime > 800) {
            if (pOperatorBox->ActionNearby(pOwner)) { step = 11; nStepTime = now; nControlReadyTime = 0; }
            else if (now - nStepTime > 10000) { Check(FALSE, "console accepts owning player"); step = 99; }
        } else if (step == 11) {
            if (Player_aPlayer[owner].m_pEntityCurrent == pControlled && Player_aPlayer[owner].HasEntityControl()) {
                if (!nControlReadyTime) nControlReadyTime = now;
                if (now - nControlReadyTime > 3000) {
                    Check(pControlled->IsPossessedByConsole(), "normal console handoff completes before objective");
                    pControlled->DataPort_SetPossessionDist(10000);
                    step = 1; nStepTime = now;
                }
            } else nControlReadyTime = 0;
            if (now - nStepTime > 20000) { Check(FALSE, "console hands control to borrowed bot"); step = 99; }
        } else if (step == 1 && now - nStepTime > 1000) {
            pControlled->SetInvincible(FALSE);
            CFMtx43A inside = *pTrigger->MtxToWorld();
            inside.m_vPos = pTrigger->TripwireBoundingSphere_WS()->m_Pos;
            pControlled->Relocate_RotXlatFromUnitMtx_WS(&inside);
            step = 2; nStepTime = now;
        } else if (step == 2 && now - nStepTime > 1500) {
            Check(pConsole->IsScriptDisabled(), "possessed bot fires objective while partner stays outside");
            pControlled->Die(FALSE, FALSE);
            step = 3; nStepTime = now;
        } else if (step == 3 && now - nStepTime > 4000) {
            Check(!pOwner->IsDeadOrDying() && !pPartner->IsDeadOrDying(), "borrowed bot death leaves both original players alive");
            Check(Player_aPlayer[owner].m_pEntityCurrent == pOwner, "owner returns from dead possessed bot");
            step = 99;
        }
    } else if (!strncmp(mode, "shop-empty", 10)) {
        if (!step) {
            CEntity *pPoint = CEntity::Find("barter2");
            if (!pPoint || !bartersystem_PlaceInWorldAndStartAttracting("barter2")) {
                Check(FALSE, "empty shop fixture exists"); step = 99;
            } else {
                for (int n = 0; n < 2; ++n) {
                    pccheats_Apply(n, PC_CHEAT_WEAPONS);
                    pccheats_Apply(n, PC_CHEAT_UPGRADES);
                    pccheats_Apply(n, PC_CHEAT_REFILL_AMMO);
                    pccheats_Apply(n, PC_CHEAT_HEAL);
                }
                CFMtx43A at = *pPoint->MtxToWorld();
                CFVec3A offset(at.m_vFront); offset.Mul(9); at.m_vPos.Add(offset);
                at.m_vFront.Negate(); at.m_vRight.Negate();
                pOwner->Relocate_RotXlatFromUnitMtx_WS(&at);
                at.m_vPos.x += 2;
                pPartner->Relocate_RotXlatFromUnitMtx_WS(&at);
                step = 1; nStepTime = now;
            }
        } else if (step == 1 && now - nStepTime > 1000) {
            pccheats_Apply(owner, PC_CHEAT_REFILL_AMMO);
            for (CEntity *p = CEntity::InWorldList_GetHead(); p; p = p->InWorldList_GetNext()) {
                if (bartersystem_IsBarterBot(p)) { p->ActionNearby(pOwner); break; }
            }
            if (bartersystem_HasEmptyOfferNotice(pOwner)) {
                Check(!bartersystem_IsActive(), "full inventory leaves empty shop closed");
                Check(!bartersystem_HasEmptyOfferNotice(pPartner), "empty offer notice belongs only to shopper");
                Check(pOwner->IsDrawEnabled() && pPartner->IsDrawEnabled(), "empty shop leaves both players visible");
                Check(game_GetControlMode() == CONTROLMODE_NORMAL, "empty shop keeps normal controls");
                step = 2; nStepTime = now;
            } else if (now - nStepTime > 12000) {
                Check(FALSE, "full inventory produces empty offer notice"); step = 99;
            }
        } else if (step == 2 && now - nStepTime > 3000) {
            Check(!bartersystem_HasEmptyOfferNotice(pOwner), "empty offer notice expires");
            EmptyCharges(pOwner);
            step = 3; nStepTime = now;
        } else if (step == 3 && now - nStepTime > 500) {
            for (CEntity *p = CEntity::InWorldList_GetHead(); p; p = p->InWorldList_GetNext()) {
                if (bartersystem_IsBarterBot(p)) { p->ActionNearby(pOwner); break; }
            }
            if (bartersystem_IsActive()) {
                Check(TRUE, "shop opens once shopper needs ammo");
                Check(!bartersystem_HasEmptyOfferNotice(pOwner), "successful retry clears empty offer notice");
                step = 4; nStepTime = now;
            } else if (now - nStepTime > 12000) {
                Check(FALSE, "shop reopens after ammo is spent"); step = 99;
            }
        } else if (step == 4 && now - nStepTime > 8000) {
            Gamepad_aapSample[Player_aPlayer[owner].m_nControllerIndex][GAMEPAD_MAIN_SELECT_PRIMARY]->uLatches |= GAMEPAD_BUTTON_1ST_PRESS_MASK;
            step = 5; nStepTime = now;
        } else if (step == 5 && now - nStepTime > 8000) {
            Check(!bartersystem_IsActive() && pOwner->IsDrawEnabled(), "cancel restores shopper after empty-offer retry");
            step = 99;
        }
    } else if (!strncmp(mode, "shop", 4)) {
        if (!step) {
            origin = *pOwner->MtxToWorld();
            CEntity *pPoint = CEntity::Find("barter1");
            if (!pPoint || !bartersystem_PlaceInWorldAndStartAttracting("barter1")) {
                Check(FALSE, "shop fixture exists"); step = 99;
            } else {
                CFMtx43A at = *pPoint->MtxToWorld();
                CFVec3A offset(at.m_vFront);
                offset.Mul(9);
                at.m_vPos.Add(offset);
                at.m_vFront.Negate(); at.m_vRight.Negate();
                pOwner->Relocate_RotXlatFromUnitMtx_WS(&at);
                CFMtx43A away = at;
                away.m_vPos.x += 30;
                pPartner->Relocate_RotXlatFromUnitMtx_WS(&away);
                Check(pPartner->IsDrawEnabled(), "partner visible before shop");
                EmptyCharges(pOwner);
                pOwner->m_pInventory->m_aoItems[INVPOS_WASHER].m_nClipAmmo = 1000;
                nPartnerWashers = pPartner->m_pInventory->m_aoItems[INVPOS_WASHER].m_nClipAmmo;
                step = 1; nStepTime = now;
            }
        } else if (step == 1 && now - nStepTime > 1000) {
            for (CEntity *p = CEntity::InWorldList_GetHead(); p; p = p->InWorldList_GetNext()) {
                if (bartersystem_IsBarterBot(p)) { p->ActionNearby(pOwner); break; }
            }
            if (bartersystem_IsActive()) {
                Check(game_GetControlMode() == CONTROLMODE_BARTERSYSTEM, "shop takes shopper controls");
                CPlayer::SetCurrent(1-owner);
                Check(game_GetControlMode() == CONTROLMODE_NORMAL, "partner retains normal controls");
                CPlayer::SetCurrent(owner);
                nShopWashers = pOwner->m_pInventory->m_aoItems[INVPOS_WASHER].m_nClipAmmo;
                nPartnerWashers = pPartner->m_pInventory->m_aoItems[INVPOS_WASHER].m_nClipAmmo;
                step = 2; nStepTime = now;
            } else if (now - nStepTime > 12000) { Check(FALSE, "shop opens for specified player"); step = 99; }
        } else if (step == 2 && now - nStepTime > 15000) {
            Check(!pOwner->IsDrawEnabled() && pPartner->IsDrawEnabled(), "shop hides only shopper; partner model remains enabled");
            Check(Player_aPlayer[1-owner].HasEntityControl(), "partner keeps entity controls while shopping");
            DEVPRINTF("COOP-TEST shop partner: inworld=%d selfhidden=%d position=(%.2f,%.2f,%.2f)\n",
                      pPartner->IsInWorld(), Player_aPlayer[1-owner].m_bHidePlayerEntity,
                      pPartner->MtxToWorld()->m_vPos.x, pPartner->MtxToWorld()->m_vPos.y,
                      pPartner->MtxToWorld()->m_vPos.z);
            Gamepad_aapSample[Player_aPlayer[owner].m_nControllerIndex][GAMEPAD_MAIN_JUMP]->uLatches |= GAMEPAD_BUTTON_1ST_PRESS_MASK;
            step = 3; nStepTime = now;
        } else if (step == 3 && now - nStepTime > 7000) {
            DEVPRINTF("COOP-TEST wallets after purchase: shopper=%d -> %d partner=%d -> %d\n", nShopWashers,
                      pOwner->m_pInventory->m_aoItems[INVPOS_WASHER].m_nClipAmmo, nPartnerWashers,
                      pPartner->m_pInventory->m_aoItems[INVPOS_WASHER].m_nClipAmmo);
            Check(pOwner->m_pInventory->m_aoItems[INVPOS_WASHER].m_nClipAmmo < nShopWashers, "purchase charges the shopper");
            Check(pPartner->m_pInventory->m_aoItems[INVPOS_WASHER].m_nClipAmmo == nPartnerWashers, "purchase leaves partner funds alone");
            Gamepad_aapSample[Player_aPlayer[owner].m_nControllerIndex][GAMEPAD_MAIN_SELECT_PRIMARY]->uLatches |= GAMEPAD_BUTTON_1ST_PRESS_MASK;
            step = 4; nStepTime = now;
        } else if (step == 4 && now - nStepTime > 8000) {
            Check(!bartersystem_IsActive() && game_GetControlMode() == CONTROLMODE_NORMAL, "cancel closes shop and restores controls");
            // The Drill mission start is still inside the large vendor radius.
            // Use its safe lift landing for the departure check; other fixtures
            // move beyond the radius from their recorded starting location.
            CFMtx43A away = origin;
            CDoorEntity *pExitLift = (CDoorEntity *)CEntity::Find("pipe_lift");
            if( pExitLift ) { away = pExitLift->AI_GetInitPos(); away.m_vPos.y += 2; }
            else away.m_vPos.z += 200;
            pOwner->Relocate_RotXlatFromUnitMtx_WS(&away, FALSE);
            away.m_vPos.x += 5;
            pPartner->Relocate_RotXlatFromUnitMtx_WS(&away, FALSE);
            step = 5; nStepTime = now;
        } else if (step == 5 && now - nStepTime > 3000) {
            Check(!level_GetStreamByName("Barter_Shop") && !level_GetStreamByName("Barter_Attr"),
                  "vendor tracks stop after both players leave");
            Check(level_IsMusicPlaying(), "mission music resumes after leaving shop");
            step = 99;
        }
    }
    CPlayer::SetCurrent(previous);
}
