#include "MultiplayerMgr.h"
#include "wpr_system.h"
#include "bottitan.h"
#include "botEliteGuard.h"

// Opt-in fixture: exercise the real PvP registration and resource constructors.
static void PortExavoltAdapterRegression(CBot *player, u32 now, int &step, u32 &stamp) {
    if(!step) {
        const Level_t &loaded=Level_aInfo[Level_nLoadedIndex];
        if(pcmods_CustomLevelCount()) {
            Check(MultiplayerMgr.IsMultiplayer(),"custom map uses native multiplayer rules");
            Check(pcmods_IsCustomLevel(loaded.nLevel),"custom world uses its additive level ID");
            const PcModLevel *declared=pcmods_CustomLevel(loaded.nLevel-56);
            Check(declared && !fclib_stricmp(declared->world,loaded.pszWorldResName),"selected custom map matches the loaded world");
            wchar_t heading[128]={};
            Check(wpr_system_CreateLoadHeading((u8)loaded.nLevel,heading,128) && heading[0],"native map heading resolves the appended menu row");
            Check(CPlayer::m_nPlayerCount==2,"custom multiplayer run constructs both players");
        }
        for(int i=0;i<CPlayer::m_nPlayerCount;++i) {
            CBot *actor=(CBot*)Player_aPlayer[i].m_pEntityOrig;
            Check(actor && actor->IsInWorld() && actor->GetMesh(),"native mod player has a live rendered mesh");
            if(actor) actor->SetInvincible(TRUE);
        }
        char assets[32]={};GetEnvironmentVariableA("MA_PORT_TEST_MOD_ASSETS",assets,sizeof(assets));
        if(assets[0]) {
            CFMtx43A at=*player->MtxToWorld();at.m_vPos.x+=15;at.m_vPos.y+=2;
            CBotTitan *titan=fnew CBotTitan;
            const BOOL titanOK=titan && titan->Create(-1,TRUE,"adapter_titan",&at);
            Check(titanOK && titan->GetMesh(),"converted Titan model and animation resources construct natively");
            if(titanOK) {titan->RemoveFromWorld(TRUE);titan->Destroy();} fdelete(titan);
            CBotEliteGuard *elite=fnew CBotEliteGuard;
            const BOOL eliteOK=elite && elite->Create(-1,TRUE,"adapter_elite",&at);
            Check(eliteOK && elite->GetMesh(),"converted Elite model and animation resources construct natively");
            if(eliteOK) {elite->RemoveFromWorld(TRUE);elite->Destroy();} fdelete(elite);
            CBotGrunt *grunt=fnew CBotGrunt;
            const BOOL gruntOK=grunt && grunt->Create(-1,FALSE,"adapter_grunt",&at);
            Check(gruntOK,"converted grunt data constructs through its native actor");
            if(gruntOK) {grunt->RemoveFromWorld(TRUE);grunt->Destroy();} fdelete(grunt);
            CCollectable::NotifyCollectableUsedInWorld(COLLECTABLE_WEAPON_RIVET_GUN_L3);
            CCollectable::PostPlayerCreationSetup();
            Check(CCollectable::GiveWeaponToPlayer(player,"Rivet Gun L3",100),"converted Needler weapon constructs through the native inventory");
            CItemInst *item=player->m_pInventory->IsWeaponInInventory("Rivet Gun L3");
            if(item) player->m_pInventory->SetCurWeapon(INV_INDEX_PRIMARY,item,FALSE,TRUE);
            at=*player->MtxToWorld();at.m_vPos.x+=4;at.m_vPos.z+=6;
            Check(CCollectable::PlaceIntoWorld("pup_energy",&at),"converted health pickup constructs through the native collectable system");
        }
        DEVPRINTF("MOD-ADAPTER READY world=%s id=%d players=%d maps=%u beta=%d sunriseMP=%d\n",
            loaded.pszWorldResName,loaded.nLevel,CPlayer::m_nPlayerCount,pcmods_CustomLevelCount(),pcmods_Feature("beta_titan"),pcmods_Feature("sunrise_loading_mp"));
        stamp=now;step=1;
    } else if(now-stamp>5000) {
        Check(player->IsInWorld(),"converted mod remains active through native simulation and rendering");
        DEVPRINTF("MOD-ADAPTER COMPLETE\n");step=99;
    }
}
