// Native compatibility implementation of Avohkii's Metal Arms Randomizer v1.1.
#include "pc_randomizer.h"
#include "pc_mods.h"
#include "bot.h"
#include "botgrunt.h"
#include "collectable.h"
#include "damage.h"
#include "entity.h"
#include "eshield.h"
#include "explosion.h"
#include "level.h"
#include "MultiplayerMgr.h"
#include "weapon.h"
#include "Ai/aiBuilder.h"
#include "Ai/AIPatrolPath.h"
#include <string.h>
#include "pc_randomizer_data.inl"

namespace {
BOOL active() { return pcmods_Option("enabled") && !MultiplayerMgr.IsMultiplayer() && Level_nLoadedIndex >= 0; }
int level() { return Level_nLoadedIndex >= 0 ? (int)Level_aInfo[Level_nLoadedIndex].nLevel : -1; }
u32 entityRoll(const CEntity *entity, u32 salt) {
    return pcmods_Roll(Level_aInfo[Level_nLoadedIndex].pszWorldResName, entity ? entity->Name() : NULL,
        entity ? &entity->MtxToWorld()->m_vPos.x : NULL, entity ? 3*sizeof(f32) : 0, salt);
}
BOOL protectedItem(CollectableType_e type) {
    return type == COLLECTABLE_UNKNOWN || type == COLLECTABLE_CHIP || type == COLLECTABLE_SECRET_CHIP ||
        type == COLLECTABLE_DET_PACK || type == COLLECTABLE_WEAPON_WRENCH ||
        (level() == LEVEL_ZOMBIEBOT_BOSS && type == COLLECTABLE_WEAPON_MAGMA_BOMB);
}
CollectableType_e pickItem(CollectableType_e original, BOOL upgrades, BOOL world, u32 roll) {
    if (protectedItem(original)) return original;
    CollectableType_e pool[80] = { COLLECTABLE_WASHER, COLLECTABLE_MEGA_WASHER, COLLECTABLE_PUP_ENERGY,
        COLLECTABLE_PUP_HIJUMP, COLLECTABLE_PUP_ARMOR, COLLECTABLE_PUP_SPEED, COLLECTABLE_PUP_WEAPON,
        COLLECTABLE_MEGA_HEALTH, COLLECTABLE_WEAPON_LASER_L1, COLLECTABLE_WEAPON_RIPPER_L1,
        COLLECTABLE_WEAPON_RIVET_GUN_L1, COLLECTABLE_WEAPON_ROCKET_L1, COLLECTABLE_WEAPON_BLASTER_L1,
        COLLECTABLE_WEAPON_TETHER_L1, COLLECTABLE_WEAPON_SPEW_L1, COLLECTABLE_WEAPON_SCOPE_L1,
        COLLECTABLE_WEAPON_MORTAR_L1, COLLECTABLE_WEAPON_FLAMER_L1, COLLECTABLE_WEAPON_CORING_CHARGE,
        COLLECTABLE_WEAPON_EMP, COLLECTABLE_WEAPON_MAGMA_BOMB, COLLECTABLE_WEAPON_CLEANER, COLLECTABLE_WEAPON_RECRUITER };
    unsigned count = 23;
    if (upgrades) {
        pool[count++] = COLLECTABLE_BATTERY;
        pool[count++] = COLLECTABLE_ARM_SERVO_L2;
        pool[count++] = COLLECTABLE_ARM_SERVO_L3;
        if (world) {
            for (int type = COLLECTABLE_WEAPON_LASER_L1; type <= COLLECTABLE_WEAPON_SPEW_L3; ++type)
                if ((type - COLLECTABLE_WEAPON_LASER_L1) % 3) pool[count++] = (CollectableType_e)type;
            pool[count++] = COLLECTABLE_WEAPON_SCOPE_L2;
        } else {
            for (int family = 0; family < 7; ++family) pool[count++] = (CollectableType_e)(COLLECTABLE_WEAPON_UPGRADE+family*3);
            pool[count++] = (CollectableType_e)(COLLECTABLE_WEAPON_UPGRADE+21);
        }
    }
    for (unsigned i = 0; i < count; ++i) {
        const CollectableType_e chosen = pool[(roll+i) % count];
        if (CCollectable::GetCollectableTypeObjectFromType(chosen)) return chosen;
    }
    return original;
}
}

u32 pcrandom_Roll(const CEntityBuilder *builder, u32 salt) {
    return pcmods_Roll(Level_nLoadedIndex >= 0 ? Level_aInfo[Level_nLoadedIndex].pszWorldResName : NULL,
        builder->m_pszEC_EntityName, &builder->m_EC_Mtx_WS.m_vPos.x, 3*sizeof(f32), salt);
}

BOOL pcrandom_Eligible(const CBotBuilder *builder) {
    if (!active() || !builder || !(builder->m_nEC_TypeBits & ENTITY_BIT_BOT) || builder->m_nOwnerPlayerIndex >= 0 || builder->m_pszMeshReplacement ||
        builder->m_pBuilderTalkInst) return FALSE;
    if (!_stricmp(Level_aInfo[Level_nLoadedIndex].pszWorldResName, "WEMCcity_02")) return FALSE;
    if (pcmods_Option("protect_dataports") && (builder->m_bDataPortInstalled || builder->m_bSetBotFlag_PowerDownUntilPosessed ||
        (builder->m_nEC_Flags & CEntity::ENTITY_FLAG_INVINCIBLE) ||
        (builder->m_pszEC_EntityName && !_strnicmp(builder->m_pszEC_EntityName,"xa_",3)) ||
        (level() >= LEVEL_COLISEUM_1 && level() <= LEVEL_COLISEUM_4))) return FALSE;
    return TRUE;
}

s32 pcrandom_Primary(const CBotBuilder *builder, BOOL miner) {
    const s32 original = builder->m_anNPCWeaponType[0];
    if (!pcrandom_Eligible(builder) || !pcmods_Option(miner ? "miner_weapons" : "npc_weapons")) return original;
    // Existing 0.1 presets keep their original five-way roll and unarmed roles.
    if (pcmods_Value("schema",1) == 1 && original == CBotBuilder::_NPC_WEAPON_HAND) return original;
    s32 pool[8] = { CBotBuilder::_NPC_WEAPON_LASER, CBotBuilder::_NPC_WEAPON_SPEW, CBotBuilder::_NPC_WEAPON_FLAMER,
        CBotBuilder::_NPC_WEAPON_RIVET, CBotBuilder::_NPC_WEAPON_ROCKET };
    unsigned count = 5;
    if (miner || pcmods_Option("grunt_ripper_blaster")) {
        pool[count++] = CBotBuilder::_NPC_WEAPON_RIPPER;
        pool[count++] = CBotBuilder::_NPC_WEAPON_BLASTER;
    }
    if (pcmods_Option(miner ? "miner_empty" : "grunt_empty")) pool[count++] = CBotBuilder::_NPC_WEAPON_HAND;
    return pool[pcrandom_Roll(builder, miner ? 0x6d696e65u : 0x6772756eu) % count];
}

PcModPrimaryScope::PcModPrimaryScope(CBotBuilder *value,BOOL miner) : builder(value), original(value->m_anNPCWeaponType[0]) {
    builder->m_anNPCWeaponType[0]=(CBotBuilder::NPCWeapon_e)pcrandom_Primary(value,miner);
}
PcModPrimaryScope::~PcModPrimaryScope() { builder->m_anNPCWeaponType[0]=(CBotBuilder::NPCWeapon_e)original; }

BOOL pcrandom_WeaponLevels(const CBotBuilder *builder, BOOL miner) {
    return pcrandom_Eligible(builder) && pcmods_Option(miner ? "miner_upgrades" : "grunt_upgrades");
}

void pcrandom_ConfigureWeapon(const CBotBuilder *builder, CWeapon *weapon, BOOL miner) {
    if (!weapon || !pcrandom_Eligible(builder)) return;
    if (pcrandom_WeaponLevels(builder, miner)) {
        const u32 count = weapon->GetUpgradeLevelCount();
        if (count) weapon->SetUpgradeLevel(pcrandom_Roll(builder, miner ? 0x6d757067u : 0x67757067u) % count);
    }
    if (pcrandom_WeaponLevels(builder, miner) || pcmods_Option(miner ? "miner_weapons" : "npc_weapons")) {
        weapon->OverrideMaxReserveAmmo(CWeapon::INFINITE_AMMO);
        weapon->SetReserveAmmo(CWeapon::INFINITE_AMMO);
        weapon->SetClipAmmo(weapon->GetMaxClipAmmo());
    }
}

PcModActorScope::PcModActorScope(CBotBuilder *value) : builder(value), active(FALSE) {
    if (!pcrandom_Eligible(value)) return;
    active = TRUE;
    health = builder->m_nEC_HealthContainerCount; armor = builder->m_pszEC_ArmorProfile;
    sleep = builder->m_bSleepAtStart; babble = builder->m_bBabble;
    const BOOL miner = (value->m_nEC_TypeBits & ENTITY_BIT_BOTMINER) != 0;
    const BOOL grunt = (value->m_nEC_TypeBits & ENTITY_BIT_BOTGRUNT) != 0;
    if ((miner && pcmods_Option("miner_batteries")) || (grunt && pcmods_Option("grunt_batteries")))
        builder->m_nEC_HealthContainerCount = 1 + pcrandom_Roll(builder,0x62617474u) % 6;
    if ((miner && pcmods_Option("miner_armor")) || (grunt && pcmods_Option("grunt_armor"))) {
        const unsigned count = sizeof(PcRandomArmor)/sizeof(*PcRandomArmor);
        const u32 roll = pcrandom_Roll(builder,0x61726d72u);
        for (unsigned i=0;i<count;++i) {
            const char *name=PcRandomArmor[(roll+i)%count];
            if (CDamage::FindArmorProfile(name,FALSE,FALSE)) { builder->m_pszEC_ArmorProfile=name; break; }
        }
    }
    if (grunt && pcmods_Option("grunt_sleep")) builder->m_bSleepAtStart = pcrandom_Roll(builder,0x736c6565u)%10 == 9;
    if (pcmods_Option("babble")) builder->m_bBabble = (pcrandom_Roll(builder,0x62616262u)&1)!=0;
}
PcModActorScope::~PcModActorScope() {
    if (!active) return;
    builder->m_nEC_HealthContainerCount=health; builder->m_pszEC_ArmorProfile=armor;
    builder->m_bSleepAtStart=sleep; builder->m_bBabble=babble;
}

s32 pcrandom_Dataport(const CBotBuilder *builder) {
    return pcrandom_Eligible(builder) && pcmods_Option("dataports") ? (s32)(pcrandom_Roll(builder,0x64706f72u)%3) : -1;
}

CEShield *pcrandom_CreateShield(CBot *bot, const CBotBuilder *builder) {
    if (!active()) return NULL;
    const char *option = NULL;
    if ((bot->TypeBits() & ENTITY_BIT_BOTGLITCH) && builder->m_nOwnerPlayerIndex >= 0) option="glitch_shield";
    else if (pcrandom_Eligible(builder)) {
        if (bot->TypeBits() & ENTITY_BIT_BOTMINER) option="miner_shields";
        else if (bot->TypeBits() & ENTITY_BIT_BOTSCOUT) option="scout_shields";
        else if (bot->TypeBits() & ENTITY_BIT_BOTZOM) option="zombot_shields";
    }
    if (!option || !pcmods_Option(option) || pcrandom_Roll(builder,0x73686965u)%5!=4) return NULL;
    CEShield *shield = fnew CEShield;
    if (!shield || !shield->Create("GruntShield")) { fdelete(shield); return NULL; }
    ShieldInit_t init = {};
    init.fShieldScale=0.4f; init.fShieldRechargeTime=10.0f; init.fShieldRechargeDelay=5.0f;
    init.pArmorProfile=CDamage::FindArmorProfile("gruntshield");
    shield->Init(bot,&init);
    shield->Attach_UnitMtxToParent_PS(bot);
    shield->EnableAutoWork(FALSE);
    shield->EnableShield(TRUE);
    shield->RemoveFromWorld();
    if (level()>=LEVEL_COLISEUM_1 && level()<=LEVEL_COLISEUM_4) shield->EnablePushOutBots(FALSE);
    DEVPRINTF("PORT-MODS shield role=%s actor=%s\n", option,bot->Name());
    return shield;
}

BOOL pcrandom_AI(CEntityBuilder *builder, CAIBuilder *output) {
    if (!(builder->m_nEC_TypeBits & ENTITY_BIT_BOT) || !builder->m_pAIBuilder || !pcrandom_Eligible((CBotBuilder *)builder)) return FALSE;
    *output=*builder->m_pAIBuilder;
    if (pcmods_Option("grunt_empty") || pcmods_Option("miner_empty")) output->m_bNeverUnarmed=TRUE;
    if (pcmods_Option("ai_aggression")) output->m_uAggression=pcrandom_Roll(builder,0x61676772u)%101;
    if (pcmods_Option("ai_courage")) output->m_uCourage=pcrandom_Roll(builder,0x636f7572u)%101;
    if (pcmods_Option("ai_intelligence")) output->m_uIntelligence=pcrandom_Roll(builder,0x696e746cu)%101;
    if (pcmods_Option("ai_jobs") && !output->m_pszLeaderName) {
        const char *paths[160]; unsigned count=0;
        for (const auto &path: PcRandomPatrols) {
            if (path.level!=level()) continue;
            CEntity *spline=CEntity::Find(path.name);
            // The patrol registry compares interned name pointers, not text.
            if (spline && (spline->TypeBits() & ENTITY_BIT_SPLINE) &&
                CPatrolPath::GetRegistered(spline->Name())) paths[count++]=spline->Name();
        }
        output->m_uInitialJob=pcrandom_Roll(builder,0x6a6f6273u)%(count?3:2);
        if (output->m_uInitialJob==2) {
            output->m_pszPatrolPath=paths[pcrandom_Roll(builder,0x70617468u)%count];
            output->m_uPatrolSpeed=pcrandom_Roll(builder,0x70737064u)%101;
        }
        output->m_uWanderSpeedMax=100;
        if (output->m_uWanderSpeedMin>100) output->m_uWanderSpeedMin=40;
    }
    DEVPRINTF("PORT-MODS AI actor=%s job=%u aggression=%u courage=%u intelligence=%u\n",
        builder->m_pszEC_EntityName,output->m_uInitialJob,output->m_uAggression,output->m_uCourage,output->m_uIntelligence);
    return TRUE;
}

void pcrandom_WorldPickup(CCollectableBuilder *builder) {
    if (!active() || !pcmods_Option("world_goodies") || !builder->m_pCollectableType ||
        (level()>=LEVEL_COLISEUM_1 && level()<=LEVEL_COLISEUM_3)) return;
    const CollectableType_e original=builder->m_pCollectableType->m_eType;
    const CollectableType_e chosen=pickItem(original,pcmods_Option("world_upgrades"),TRUE,pcrandom_Roll(builder,0x776f726cu));
    if (chosen==original) return;
    builder->m_pCollectableType=CCollectable::GetCollectableTypeObjectFromType(chosen);
    builder->m_nAmmo=-1;
    builder->m_uBatteryNumber=chosen==COLLECTABLE_BATTERY ? 2+pcrandom_Roll(builder,0x62617474u)%5 : (u32)-1;
    DEVPRINTF("PORT-MODS world pickup actor=%s original=%d selected=%d\n",builder->m_pszEC_EntityName,original,chosen);
}

PcModCollectableScope::PcModCollectableScope(CCollectableBuilder *value) : builder(value),
    type(value->m_pCollectableType),ammo(value->m_nAmmo),battery(value->m_uBatteryNumber) { pcrandom_WorldPickup(value); }
PcModCollectableScope::~PcModCollectableScope() {
    builder->m_pCollectableType=type; builder->m_nAmmo=ammo; builder->m_uBatteryNumber=battery;
}

u32 pcrandom_Drop(u32 original,CEntity *owner,u32 slot) {
    if (!active() || !pcmods_Option("goodie_drops")) return original;
    const CollectableType_e chosen=pickItem((CollectableType_e)original,pcmods_Option("drop_upgrades"),FALSE,entityRoll(owner,0x64726f70u^slot));
    CCollectableType *type=CCollectable::GetCollectableTypeObjectFromType(chosen);
    if (type && !type->m_bSetup && !type->Setup()) return original;
    return chosen;
}

const char *pcrandom_ScriptGift(const char *original,CEntity *recipient) {
    if (!active()) return original;
    const CollectableType_e type=CCollectable::GetCollectableTypeFromString(original);
    if (!pcmods_Option("script_goodies") || level()<=LEVEL_CLEAN_UP || level()==LEVEL_WASTELAND_CHASE || protectedItem(type)) return original;
    const CollectableType_e chosen=pickItem(type,TRUE,FALSE,entityRoll(recipient,0x67696674u) ^ pcmods_Roll(NULL,original,NULL,0,0));
    CCollectableType *result=CCollectable::GetCollectableTypeObjectFromType(chosen);
    return result && (result->m_bSetup || result->Setup()) ? result->m_pszName : original;
}

FExplosion_GroupHandle_t pcrandom_Explosion(FExplosion_GroupHandle_t original,const CEntity *entity,const char *option) {
    if (!active() || !pcmods_Option(option)) return original;
    const unsigned count=sizeof(PcRandomExplosions)/sizeof(*PcRandomExplosions);
    const u32 roll=entityRoll(entity,0x6578706cu);
    for (unsigned i=0;i<count;++i) {
        FExplosion_GroupHandle_t found=CExplosion2::GetExplosionGroup(PcRandomExplosions[(roll+i)%count]);
        if (found!=FEXPLOSION_INVALID_HANDLE) return found;
    }
    return original;
}
