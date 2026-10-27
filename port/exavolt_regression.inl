#include "pc_mods.h"
#include "botgrunt.h"
#include "ffile.h"
#include "exavolt_extended_regression.inl"

static BOOL ExavoltStreamFinished = FALSE;
static void ExavoltStreamCallback(s32 error, FFileHandle file, void *buffer, u32 amount, void *user) {
    ExavoltStreamFinished = error == 0 && amount == *(u32 *)user;
}

static void PortExavoltRegression(CBot *player, BOOL vanilla, int &step, BOOL extended=FALSE) {
    const u32 washers = player->m_pInventory->m_aoItems[INVPOS_WASHER].m_nClipAmmo;
    Check(washers == (vanilla ? 0u : 321u), vanilla ?
        "native vanilla launch retains original archive inventory" :
        "native Exavolt CSV edit reaches the real starting inventory");
    if (!vanilla) {
        unsigned char regular[256] = {}, streamed[256] = {};
        FFileHandle file = ffile_Open("mod_api.csv", FFILE_OPEN_RONLY);
        Check(FFILE_IS_VALID_HANDLE(file), "native Exavolt adds a resource absent from the base MST");
        if (!FFILE_IS_VALID_HANDLE(file)) { step = 99; return; }
        s32 bytes = ffile_GetFileSize(file);
        Check(bytes > 48 && bytes <= sizeof(regular), "native overridden resource has its own size");
        if (bytes <= 48 || bytes > sizeof(regular)) { ffile_Close(file); step = 99; return; }
        Check(ffile_Read(file, bytes, regular) == bytes, "native resource overlay reads the complete data");
        ffile_Close(file);
        u32 size = 0;
        const FFileMasterEntry_t *entry = ffile_MF_OpenStream("mod_api.csv", &size);
        Check(entry && size == (u32)bytes, "native streamed resource uses the overridden entry and size");
        if (entry) {
            ExavoltStreamFinished = FALSE;
            s32 read = ffile_MF_ReadStream(entry, 0, size, streamed, ExavoltStreamCallback, &size);
            Check(read == (s32)size && ExavoltStreamFinished && !memcmp(regular, streamed, size),
                "native mod streaming returns the override and completes its callback");
            ExavoltStreamFinished = FALSE;
            Check(ffile_MF_ReadStream(entry, size, 1, streamed, ExavoltStreamCallback, &size) == FFILE_ERROR_RET &&
                !ExavoltStreamFinished, "native mod stream rejects an out-of-range read without a success callback");
        }
        if (extended) { PortExavoltExtended(player); }
        else {
        CFMtx43A at = *player->MtxToWorld();
        at.m_vPos.x += 12; at.m_vPos.y += 3;
        const char *world = Level_aInfo[Level_nLoadedIndex].pszWorldResName;
        u32 roll = pcmods_Roll(world, "exavolt_probe", &at.m_vPos.x, 3 * sizeof(f32), 0x6772756eu);
        while ((pcmods_Option("npc_weapons") && roll % 5 == 0) ||
            (pcmods_Option("npc_colors") && (roll >> 8) % 5 == 0)) {
            at.m_vPos.x += 0.125f;
            roll = pcmods_Roll(world, "exavolt_probe", &at.m_vPos.x, 3 * sizeof(f32), 0x6772756eu);
        }
        const CWeapon::WeaponType_e pool[] = { CWeapon::WEAPON_TYPE_LASER, CWeapon::WEAPON_TYPE_SPEW,
            CWeapon::WEAPON_TYPE_FLAMER, CWeapon::WEAPON_TYPE_RIVET_GUN, CWeapon::WEAPON_TYPE_ROCKET_LAUNCHER };
        CFColorRGBA firstTint;
        for (int pass = 0; pass < 2; ++pass) {
            CBotGrunt *npc = fnew CBotGrunt;
            const BOOL created = npc && npc->Create(-1, FALSE, "exavolt_probe", &at);
            Check(created, "native randomized grunt constructs with real weapon resources");
            if (created) {
                CWeapon *weapon = npc->GetPrimaryWeapon();
                const CWeapon::WeaponType_e expected = pcmods_Option("npc_weapons") ? pool[roll % 5] : CWeapon::WEAPON_TYPE_LASER;
                DEVPRINTF("EXAVOLT-TEST seed=%u roll=%u expected=%d actual=%d pass=%d\n",
                    pcmods_Seed(), roll, expected, weapon ? weapon->Type() : -1, pass);
                Check(weapon && weapon->Type() == expected, "native seed selects the actual equipped weapon and repeats after reconstruction");
                if (pcmods_Option("npc_colors")) {
                    CFColorRGBA *tint = npc->GetMesh()->GetMeshTintAndAlpha();
                    Check(fmath_Abs(tint->fRed-118.0f/255.0f)+fmath_Abs(tint->fGreen-124.0f/255.0f)+fmath_Abs(tint->fBlue-1.0f)>0.05f,
                        "native randomized color reaches the rendered grunt mesh");
                    if (!pass) firstTint = *tint;
                    else Check(tint->fRed==firstTint.fRed && tint->fGreen==firstTint.fGreen && tint->fBlue==firstTint.fBlue,
                        "native grunt color repeats after reconstruction");
                }
                npc->RemoveFromWorld(TRUE);
                npc->Destroy();
            }
            fdelete(npc);
        }
        CBotGrunt *protectedNpc = fnew CBotGrunt;
        const BOOL protectedCreated = protectedNpc && protectedNpc->Create(-1, TRUE, "exavolt_protect", &at);
        Check(protectedCreated, "native possession-role grunt constructs after randomization");
        if (protectedCreated) {
            Check(protectedNpc->GetPrimaryWeapon() && protectedNpc->GetPrimaryWeapon()->Type() == CWeapon::WEAPON_TYPE_LASER,
                "native protected possession bot keeps its authored weapon");
            protectedNpc->RemoveFromWorld(TRUE);
            protectedNpc->Destroy();
        }
        fdelete(protectedNpc);
        }
    }
    DEVPRINTF("EXAVOLT-TEST COMPLETE mode=%s players=%d washers=%u\n", vanilla ? "vanilla" : "modded", CPlayer::m_nPlayerCount, washers);
    step = 99;
}
