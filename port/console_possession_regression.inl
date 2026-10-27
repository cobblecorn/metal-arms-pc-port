// Opt-in native probe of the real first Level 15 arcade and its fabricated Rivet Mil.
#include "BotDispenser.h"
#include "falignedpool.h"

static void PortConsoleBuildPoolCheck(CBot *mil) {
    void *held[FALIGNED_VEC3A_POOL_SIZE]={};u32 count=0;
    const u32 before=FAlignedPool_VecPool.m_nCurrentElementsUsed;
    while(FAlignedPool_VecPool.GetNumberOfElements()-FAlignedPool_VecPool.m_nCurrentElementsUsed>20 && count<FALIGNED_VEC3A_POOL_SIZE)
        held[count++]=FAlignedPool_VecPool.GetElement();
    const u32 pressure=FAlignedPool_VecPool.m_nCurrentElementsUsed;
    CFVec3A from=mil->MtxToWorld()->m_vPos;from.y+=8;
    CFXMeshBuildPartMgr *failed=CFXMeshBuildPartMgr::InitMgr(from,mil->GetMesh(),3.0f,.15f);
    Check(!failed,"native construction rejects a partial allocation under pool pressure");
    if(failed)failed->ForceFinish();
    Check(FAlignedPool_VecPool.m_nCurrentElementsUsed==pressure,"native failed construction returns every temporary vector");
    while(count)FAlignedPool_VecPool.ReturnElement(held[--count]);
    CFXMeshBuildPartMgr *retry=CFXMeshBuildPartMgr::InitMgr(from,mil->GetMesh(),3.0f,.15f);
    Check(retry!=NULL,"native construction retries successfully after resource pressure clears");
    if(retry)retry->ForceFinish();
    Check(FAlignedPool_VecPool.m_nCurrentElementsUsed==before,"native finished construction restores the vector pool");
}
static void PortConsolePossessionRegression(CBot *actor, CBot *partner, int owner,
    const char *mode, u32 now, int &step, u32 &stamp) {
    static CEConsole *console=NULL;
    static CEntity *area=NULL;
    static CBot *mil=NULL;
    static u32 changed=0;
    static int lastPlayer=-9;
    static u32 lastLog=0;
    if (!step) {
        mil=(CBot*)CEntity::Find("xa_autobot_grunt5");
        for (CEntity *e=CEntity::InWorldList_GetHead();e;e=e->InWorldList_GetNext()) {
            if ((e->TypeBits()&ENTITY_BIT_CONSOLE) && fmath_Abs(e->MtxToWorld()->m_vPos.x+478.72098f)<1.0f) {
                console=(CEConsole*)e;break;
            }
        }
        Check(console && mil,"native Level 15 first arcade and authored Rivet Mil exist");
        if(!console || !mil){step=99;return;}
        for (CEntity *c=console->GetFirstChild();c;c=console->GetNextChild(c))
            if(c->TypeBits()&ENTITY_BIT_BOX){area=c;break;}
        Check(area!=NULL,"native first arcade operator area exists");
        if(!area){step=99;return;}
        actor->SetInvincible(TRUE);
        actor->Relocate_RotXlatFromUnitMtx_WS(area->MtxToWorld(),FALSE);
        if(partner!=actor)partner->SetInvincible(TRUE);
        DEVPRINTF("CONSOLE-TEST start owner=%d mode=%s cutscene=%d milWorld=%d health=%.3f\n",
            owner,mode,CBot::m_bCutscenePlaying,mil->IsInWorld(),mil->NormHealth());
        step=1;stamp=now;return;
    }
    if(mil->m_nPossessionPlayerIndex!=lastPlayer){
        lastPlayer=mil->m_nPossessionPlayerIndex;
        DEVPRINTF("CONSOLE-TEST ownership=%d world=%d health=%.3f dead=%d constructed=%d ms=%u\n",
            lastPlayer,mil->IsInWorld(),mil->NormHealth(),mil->IsDeadOrDying(),mil->IsUnderConstruction(),now-stamp);
    }
    if(step==1){
        actor->Relocate_RotXlatFromUnitMtx_WS(area->MtxToWorld(),FALSE);
        if(now-stamp>800 && area->ActionNearby(actor)){
            DEVPRINTF("CONSOLE-TEST accepted cutscene=%d\n",CBot::m_bCutscenePlaying);
            step=2;stamp=now;
        }else if(now-stamp>10000){Check(FALSE,"native first arcade accepts player");step=99;}
    } else if(step==2){
        if(now-lastLog>2000){
            CAlarmNet *net=CAlarmNet::FindAlarmNet(mil->m_pCanOnlyBeDispensedBy);
            for(u32 i=0;i<CAlarmNet::m_uAutoBotPoolSize;++i)if(CAlarmNet::m_papAutoBotPool[i]==mil)
                DEVPRINTF("CONSOLE-TEST waiting ms=%u world=%d built=%d hp=%.3f usedBy=%d needs=%d net=%d total=%d sofar=%d aliveMax=%d flags=%x\n",now-stamp,mil->IsInWorld(),mil->IsUnderConstruction(),mil->NormHealth(),CAlarmNet::m_pauAutoBotNetUseage[i],mil->m_pCanOnlyBeDispensedBy->NeedsAutoBot(),net?net->m_uNetState:-1,net?net->m_uTotalBotsToAutoSpawn:-1,net?net->m_uNumAutoBotsSoFar:-1,net?net->m_nMaxNumLivingAutoBots:-1,net?net->m_uAlarmNetFlags:0);
            if(net)DEVPRINTF("CONSOLE-TEST net needs=%d clock=%.3f started=%.3f active=%.3f list=%u\n",net->NetNeedsABot(),aiutils_GetCurTimeSecs(),net->m_fTurnOnTime,net->m_fActiveTime,net->m_DispenserList.nCount);
            int building=0;
            for(CEntity *e=CEntity::InWorldList_GetHead();e;e=e->InWorldList_GetNext())
                if((e->TypeBits()&ENTITY_BIT_BOT) && ((CBot*)e)->IsUnderConstruction())++building;
            DEVPRINTF("CONSOLE-TEST vectorPool=%u/%u building=%d\n",FAlignedPool_VecPool.m_nCurrentElementsUsed,FAlignedPool_VecPool.GetNumberOfElements(),building);
            lastLog=now;
        }
        if(Player_aPlayer[owner].m_pEntityCurrent==mil){
            Check(mil->IsPossessedByConsole(),"native first arcade transfers control through real console");
            Check(mil->GetPrimaryWeapon() && mil->GetPrimaryWeapon()->Type()==CWeapon::WEAPON_TYPE_RIVET_GUN,
                "native possessed first Mil has a Rivet gun");
            DEVPRINTF("CONSOLE-TEST acquired control=%d hp=%.3f cutscene=%d\n",
                Player_aPlayer[owner].HasEntityControl(),mil->NormHealth(),CBot::m_bCutscenePlaying);
            if(strstr(mode,"cancel")){mil->ForceQuickDataPortUnPlug();DEVPRINTF("CONSOLE-TEST cancel during fade\n");}
            else if(strstr(mode,"death")){mil->Die(TRUE,FALSE);DEVPRINTF("CONSOLE-TEST death during fade\n");}
            step=3;stamp=now;changed=now;
        }else if(now-stamp>20000){Check(FALSE,"native first arcade finishes fabrication and handoff");step=99;}
    } else if(step==3 && now-changed>4500){
        if(!strstr(mode,"cancel") && !strstr(mode,"death")){
            Check(Player_aPlayer[owner].m_pEntityCurrent==mil && Player_aPlayer[owner].HasEntityControl(),
                "native first arcade keeps live Mil under control after fade");
            PortConsoleBuildPoolCheck(mil);
            if(strstr(mode,"restore")){
                checkpoint_Restore(1,FALSE);step=5;stamp=now;return;
            }
            mil->ForceQuickDataPortUnPlug();step=4;stamp=now;
        } else step=4;
    } else if(step==4 && now-stamp>2000){
        Check(Player_aPlayer[owner].m_pEntityCurrent==actor && Player_aPlayer[owner].HasEntityControl(),
            "native console returns player and controls safely");
        Check(!actor->IsDeadOrDying(),"native console operator remains alive");
        Check(console->IsIdle() && actor->m_pWorldMesh->IsCollisionFlagSet(),"native console releases its operator and returns to idle");
        Check(!Player_aPlayer[owner].GetDrawOverlayFunction(),"native console clears its transition overlay");
        if(partner!=actor)Check(Player_aPlayer[1-owner].m_pEntityCurrent==partner && Player_aPlayer[1-owner].HasEntityControl(),"native console transition leaves the other player independent");
        DEVPRINTF("CONSOLE-TEST COMPLETE owner=%d mode=%s\n",owner,mode);step=99;
    } else if(step==5 && now-stamp>3000){
        Check(Player_aPlayer[owner].m_pEntityCurrent==mil && Player_aPlayer[owner].HasEntityControl(),"native console checkpoint restores the borrowed Mil and its controls");
        mil->ForceQuickDataPortUnPlug();step=4;stamp=now;
    }
}
