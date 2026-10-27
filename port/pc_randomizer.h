#pragma once
#include "fang.h"
#include "fexplosion.h"

class CBot;
class CBotBuilder;
class CEntity;
class CEntityBuilder;
class CAIBuilder;
class CWeapon;
class CEShield;
class CCollectableBuilder;
class CCollectableType;

// Avohkii Randomizer behavior, expressed through native game APIs.
BOOL pcrandom_Eligible(const CBotBuilder *builder);
u32 pcrandom_Roll(const CEntityBuilder *builder, u32 salt);
s32 pcrandom_Primary(const CBotBuilder *builder, BOOL miner);
BOOL pcrandom_WeaponLevels(const CBotBuilder *builder, BOOL miner);
void pcrandom_ConfigureWeapon(const CBotBuilder *builder, CWeapon *weapon, BOOL miner);
BOOL pcrandom_AI(CEntityBuilder *builder, CAIBuilder *output);
s32 pcrandom_Dataport(const CBotBuilder *builder);
CEShield *pcrandom_CreateShield(CBot *bot, const CBotBuilder *builder);
void pcrandom_WorldPickup(CCollectableBuilder *builder);
u32 pcrandom_Drop(u32 original, CEntity *owner, u32 slot);
const char *pcrandom_ScriptGift(const char *original, CEntity *recipient);
FExplosion_GroupHandle_t pcrandom_Explosion(FExplosion_GroupHandle_t original, const CEntity *entity, const char *option);

class PcModActorScope {
    CBotBuilder *builder;
    BOOL active, sleep, babble;
    u32 health;
    const char *armor;
public:
    explicit PcModActorScope(CBotBuilder *value);
    ~PcModActorScope();
};

class PcModPrimaryScope {
    CBotBuilder *builder;
    s32 original;
public:
    PcModPrimaryScope(CBotBuilder *value, BOOL miner);
    ~PcModPrimaryScope();
};

class PcModCollectableScope {
    CCollectableBuilder *builder;
    CCollectableType *type;
    s32 ammo;
    u32 battery;
public:
    explicit PcModCollectableScope(CCollectableBuilder *value);
    ~PcModCollectableScope();
};
