// Opt-in native test. Script, camera, audio timing and ending run normally.
#include "pc_booth_fallback.h"
extern BOOL MAScript_bPortScriptCameraActive;

static void PortBoothFallbackRegression(CBot *actor, BOOL blockGuards,
    u32 now, int &step, u32 &stamp) {
    static CFScriptInst *scene = NULL;
    static CBot *heldGuards[4] = {};
    static CFMtx43A heldAt[4];
    static u32 waitStarted = 0;
    static BOOL sawWait = FALSE, sawEnding = FALSE;
    if (!step) {
        for (u32 i=0;i<CFScriptSystem::NumScriptInsts();++i) {
            CFScriptInst *candidate=&CFScriptSystem::GetScriptInstList()[i];
            if (candidate->m_bIsInitialized && candidate->m_pScript &&
                !fclib_stricmp(candidate->m_pScript->m_szScriptFileName,"rsboothdoor.sma")) {
                scene=candidate;break;
            }
        }
        if (!scene || !scene->m_pDataArea || CBot::m_bCutscenePlaying ||
            !Player_aPlayer[0].HasEntityControl()) return;
        cell *g=(cell *)scene->m_pDataArea;
        if (g[96/4]!=-1) return; // Opening lift still running.
        Check(PortBoothArrivalLayout(scene->m_pScript->m_szScriptFileName,
            &scene->m_oAMX,scene->m_pScript->m_uDataAreaSize),"native booth retail layout recognized");
        const char *names[]={"elite1","elite2","elite_lacky1","elite_lacky2"};
        for (int i=0;i<4;++i) {
            CEntity *entity=CEntity::FindInWorld(names[i]);
            Check(entity && (entity->TypeBits()&ENTITY_BIT_BOT),"native booth guard exists");
            if (!entity || !(entity->TypeBits()&ENTITY_BIT_BOT)) {step=99;return;}
            ((CBot *)entity)->SetInvincible(TRUE);
            if (blockGuards) {
                heldGuards[i]=(CBot *)entity;heldAt[i]=*entity->MtxToWorld();
                CEntity *arrival=CEntity::FindInWorld("trigger_group");
                if (arrival && arrival->TripwireContainsPoint(heldAt[i].m_vPos)) heldAt[i]=*actor->MtxToWorld();
            }
        }
        for (int i=0;i<CPlayer::m_nPlayerCount;++i)
            ((CBot *)Player_aPlayer[i].m_pEntityOrig)->SetInvincible(TRUE);
        CEntity *button=CEntity::FindInWorld("area7switch");
        Check(button!=NULL,"native authored booth alarm switch exists");
        if (!button) {step=99;return;}
        CFScriptSystem::TriggerEvent(CFScriptSystem::GetEventNumFromName("switch"),0,
            (u32)button,(u32)actor);
        DEVPRINTF("BOOTH-TEST started players=%d blocked=%d\n",CPlayer::m_nPlayerCount,blockGuards);
        step=1;stamp=now;return;
    }
    if (blockGuards) for (int i=0;i<4;++i)
        if (heldGuards[i] && heldGuards[i]->IsInWorld()) heldGuards[i]->Relocate_RotXlatFromUnitMtx_WS(&heldAt[i]);
    cell *g=(cell *)scene->m_pDataArea;
    if (CBot::m_bCutscenePlaying && g[32/4]==-1 && !sawWait) {
        sawWait=TRUE;waitStarted=now;
        DEVPRINTF("BOOTH-TEST arrival wait count=%d camera=%d\n",g[92/4],MAScript_bPortScriptCameraActive);
    }
    if (blockGuards && sawWait && now-waitStarted<14500) {
        Check(CBot::m_bCutscenePlaying && MAScript_bPortScriptCameraActive,
            "native stalled scene is retained throughout the grace period");
    }
    if (sawWait && g[32/4]!=-1 && !sawEnding) {
        sawEnding=TRUE;
        DEVPRINTF("BOOTH-TEST authored ending state=%d arrivals=%d waitedMs=%u\n",g[32/4],g[92/4],now-waitStarted);
        Check(!blockGuards || now-waitStarted>=14900,"native fallback waits before resuming the scene ending");
    }
    if (sawWait && !CBot::m_bCutscenePlaying && Player_aPlayer[0].HasEntityControl()) {
        Check(g[32/4]==-1 && g[92/4]>=4,"native script completes its normal final state");
        Check(!MAScript_bPortScriptCameraActive,"native authored camera release completed");
        for (int i=0;i<CPlayer::m_nPlayerCount;++i)
            Check(Player_aPlayer[i].HasEntityControl(),"native scene returns each player's control");
        const char *names[]={"elite1","elite2","elite_lacky1","elite_lacky2"};
        for (int i=0;i<4;++i) {
            CEntity *entity=CEntity::FindInWorld(names[i]);
            if (entity && blockGuards) entity->EnableAutoWork(TRUE);
        }
        DEVPRINTF("BOOTH-TEST COMPLETE players=%d blocked=%d waitedMs=%u\n",CPlayer::m_nPlayerCount,blockGuards,now-waitStarted);
        step=99;
    } else if (now-stamp>55000) {
        Check(FALSE,"native booth scene finishes within timeout");step=99;
    }
}
