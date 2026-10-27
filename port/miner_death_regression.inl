// Real model/part-manager/debris diagnostic; never runs in ordinary gameplay.
#include "cammanual.h"

class PortMinerDeathProbe : public CBotMiner {
public:
    BOOL CreateMesh(const char *mesh,const char *name,const CFMtx43A &at) {
        CBotMinerBuilder *builder=(CBotMinerBuilder *)GetLeafClassBuilder();
        builder->SetDefaults(0,0,ENTITY_TYPE_BOTMINER);
        builder->m_pszMeshReplacement=mesh;
        static const CBotDef definition={BOTRACE_DROID,BOTCLASS_MINER,BOTSUBCLASS_MINER_BASE};
        return CBot::Create(&definition,-1,FALSE,name,&at,NULL);
    }
};

static void PortMinerDeathRegression(CBot *player,CBot *partner,u32 now,int &step,u32 &stamp) {
    static CBot *bots[3]={};
    static PortMinerDeathProbe *variants[2]={};
    static int sample=0;
    static CFMtx43A cameraMtx;
    static CCamManualInfo cameraInfo(FMATH_DEG2RAD(40.0f));
    if (!step) {
        game_EndCutScene(TRUE);
        for(int i=0;i<CPlayer::m_nPlayerCount;++i){
            CBot *p=(CBot*)Player_aPlayer[i].m_pEntityOrig;
            if(!p->IsInWorld())p->AddToWorld();
            Player_aPlayer[i].EnableEntityControl();
        }
        for(CEntity *e=CEntity::InWorldList_GetHead();e;e=e->InWorldList_GetNext())
            if((e->TypeBits()&ENTITY_BIT_BOT) && e!=player && e!=partner)e->EnableAutoWork(FALSE);
        bots[0]=(CBot *)CEntity::Find("droid2");
        Check(bots[0] && (bots[0]->TypeBits()&ENTITY_BIT_BOTMINER),"native authored friendly is the miner model");
        if(!bots[0]){step=99;return;}
        CFMtx43A at=*bots[0]->MtxToWorld();
        cameraInfo.m_pmtxMtx=&cameraMtx;
        CFVec3A direction(-8.0f,-3.0f,12.0f);
        cameraMtx.UnitMtxFromNonUnitVec(&direction);
        cameraMtx.m_vPos=at.m_vPos;cameraMtx.m_vPos.x+=8;cameraMtx.m_vPos.y+=6;cameraMtx.m_vPos.z-=12;
        for(int i=0;i<CPlayer::m_nPlayerCount;++i)
            gamecam_SwitchPlayerToManualCamera(PLAYER_CAM(i),&cameraInfo);
        player->SetInvincible(TRUE);
        CFMtx43A view=at;view.m_vPos.z-=24;player->Relocate_RotXlatFromUnitMtx_WS(&view,FALSE);
        if(partner!=player){partner->SetInvincible(TRUE);view.m_vPos.x+=10;partner->Relocate_RotXlatFromUnitMtx_WS(&view,FALSE);}
        const char *meshes[]={"GRDMminer01","GRDMminernl"};
        for(int i=0;i<2;++i){
            variants[i]=fnew PortMinerDeathProbe;at.m_vPos.x+=12;
            const BOOL made=variants[i] && variants[i]->CreateMesh(meshes[i],i?"miner_death_nolegs":"miner_death_variant",at);
            Check(made,"native alternate miner mesh creates with its own parts pool");
            if(!made){step=99;return;}
            bots[i+1]=variants[i];bots[i+1]->AddToWorld();
        }
        for(int i=0;i<3;++i){
            bots[i]->SetInvincible(FALSE);bots[i]->SetMaxHealth();bots[i]->EnableAutoWork(TRUE);
            Check(bots[i]->GetPartMgr() && bots[i]->GetPartMgr()->IsCreated() &&
                bots[i]->GetPartMgr()->GetLimbTypeCount()==1,"native miner uses the actual one-limb breakup table");
            DEVPRINTF("MINER-DEATH ready sample=%d mesh=%s bones=%u\n",i,bots[i]->GetMesh()->m_pMesh->szName,bots[i]->GetMesh()->m_pMesh->nBoneCount);
            const char *root=bots[i]->m_pBotInfo_Gen->pszRootBoneName;
            const int ri=bots[i]->GetMesh()->FindBone(root);
            DEVPRINTF("MINER-DEATH root=%s index=%d children=%d chunks=%p guts=%p timed=%p impact=%p immediate=%p\n",root,ri,ri>=0?bots[i]->GetMesh()->m_pMesh->pBoneArray[ri].Skeleton.nChildBoneCount:-1,bots[i]->m_pBotInfo_Gen->pDebrisGroup_Chunks,bots[i]->m_pBotInfo_Gen->pDebrisGroup_Guts,bots[i]->m_pBotInfo_Gen->apBoneGeoDebrisGroup[0],bots[i]->m_pBotInfo_Gen->apBoneGeoDebrisGroup[2],bots[i]->m_pBotInfo_Gen->apBoneGeoDebrisGroup[1]);
        }
        if(partner!=player){
            const f32 health=partner->NormHealth();partner->SetInvincible(FALSE);
            CDamageData data;memset(&data,0,sizeof(data));data.m_nDamageLocale=CDamageForm::DAMAGE_LOCALE_IMPACT;
            data.m_Damager.pBot=player;data.m_Damager.nDamagerPlayerIndex=0;data.m_pDamageeEntity=partner;data.m_pDamageProfile=&CDamage::m_DamageProfileNone;
            CDamageResult result;memset(&result,0,sizeof(result));result.m_pDamageData=&data;result.m_fDeltaHitpoints=.1f;
            partner->InflictDamageResult(&result);Check(partner->NormHealth()==health,"native co-op player friendly-fire protection is unchanged");partner->SetInvincible(TRUE);
        }
        step=1;stamp=now;return;
    }
    if(step==1 && now-stamp>1200){
        CBot *target=bots[sample];
        if(sample==2){CFMtx43A at=*target->MtxToWorld();at.m_vPos.y+=12;target->Relocate_RotXlatFromUnitMtx_WS(&at,FALSE);target->EnableAutoWork(FALSE);}
        cameraMtx.m_vPos=target->MtxToWorld()->m_vPos;
        cameraMtx.m_vPos.x+=8;cameraMtx.m_vPos.y+=6;cameraMtx.m_vPos.z-=12;
        if(sample==1){
            target->Die(TRUE,FALSE); // Scripted death uses the same breakup.
        } else {
            CBot *shooter=CPlayer::m_nPlayerCount>1?partner:player;
            CDamageData data;memset(&data,0,sizeof(data));data.m_nDamageLocale=CDamageForm::DAMAGE_LOCALE_IMPACT;
            data.m_Damager.pBot=shooter;data.m_Damager.nDamagerPlayerIndex=CPlayer::m_nPlayerCount>1?1:0;
            data.m_pDamageeEntity=target;data.m_pDamageProfile=&CDamage::m_DamageProfileNone;
            CDamageResult result;memset(&result,0,sizeof(result));result.m_pDamageData=&data;result.m_fDeltaHitpoints=100;
            target->InflictDamageResult(&result);
        }
        target->EnableAutoWork(TRUE);
        Check(target->IsDeadOrDying(),"native miner enters death through damage or script");
        Check(target->GetPartMgr()->CurrentlyOwnsDebris(),"native miner death creates moving pieces of its real model");
        DEVPRINTF("MINER-DEATH emitted sample=%d mesh=%s ownsDebris=%d world=%d\n",sample,target->GetMesh()->m_pMesh->szName,target->GetPartMgr()->CurrentlyOwnsDebris(),target->IsInWorld());
        step=2;stamp=now;return;
    }
    if(step==2 && now-stamp>500){
        Check(bots[sample]->IsInWorld() && bots[sample]->GetPartMgr()->CurrentlyOwnsDebris(),"native dead miner remains to animate its visible debris");
        if(++sample<3){step=1;stamp=now;return;}
        step=3;stamp=now;return;
    }
    if(step==3 && now-stamp>15000){
        for(int i=0;i<3;++i)Check(!bots[i]->IsInWorld(),"native miner is removed after its debris finishes");
        bots[0]->UnDie();bots[0]->AddToWorld();
        Check(!bots[0]->IsDeadOrDying() && !bots[0]->GetPartMgr()->CurrentlyOwnsDebris(),"native revived miner resets its parts");
        bots[0]->Die(TRUE,FALSE);
        Check(bots[0]->GetPartMgr()->CurrentlyOwnsDebris(),"native revived miner can break apart again");
        bots[0]->UnDie();
        Check(!bots[0]->GetPartMgr()->CurrentlyOwnsDebris(),"native reset releases still-moving owned debris");
        DEVPRINTF("MINER-DEATH COMPLETE players=%d\n",CPlayer::m_nPlayerCount);step=99;
    }
}
