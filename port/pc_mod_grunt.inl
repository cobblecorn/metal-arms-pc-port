// A scoped builder override: no random rolls in Work(), and no builder changes
// leak to the next NPC or to a player spawning with this model.
class PcModGruntScope {
    CBotGruntBuilder *builder;
    CBotBuilder::NPCWeapon_e weapon;
    CFColorRGB tint;
    BOOL shield;
public:
    explicit PcModGruntScope(CBotGruntBuilder *value) : builder(value),
        weapon(value->m_anNPCWeaponType[0]), tint(value->m_MeshTintColor), shield(value->m_bEnableShield) {
        if (!pcrandom_Eligible(value)) return;
        const char *world = Level_aInfo[Level_nLoadedIndex].pszWorldResName;
        // This level uses the Mil's equipment/identity for its security puzzle.
        if (!_stricmp(world, "WEMCcity_02")) return;
        const u32 roll = pcmods_Roll(world, value->m_pszEC_EntityName,
            &value->m_EC_Mtx_WS.m_vPos.x, 3 * sizeof(f32), 0x6772756eu);
        value->m_anNPCWeaponType[0] = (CBotBuilder::NPCWeapon_e)pcrandom_Primary(value,FALSE);
        if (pcmods_Option("grunt_shields")) value->m_bEnableShield=pcrandom_Roll(value,0x73686965u)%5==4;
        if (pcmods_Option("npc_colors")) {
            static const f32 colors[5][3] = { {118,124,255}, {185,59,38}, {255,255,255},
                {64,54,42}, {85,157,63} };
            const f32 *color = colors[(roll >> 8) % 5];
            value->m_MeshTintColor.Set(color[0]/255.0f, color[1]/255.0f, color[2]/255.0f);
            if (pcmods_Value("schema",1)>=2) {
                value->m_MeshTintColor.Set((pcrandom_Roll(value,0x72656400u)&0xffff)/65535.0f,
                    (pcrandom_Roll(value,0x67726e00u)&0xffff)/65535.0f,(pcrandom_Roll(value,0x626c7500u)&0xffff)/65535.0f);
            }
        }
        if (pcmods_Option("npc_weapons") || pcmods_Option("npc_colors")) {
            DEVPRINTF("PORT-MODS grunt name=%s seed=%u original=%d selected=%d roll=%u\n",
                value->m_pszEC_EntityName ? value->m_pszEC_EntityName : "(unnamed)",
                pcmods_Seed(), weapon, value->m_anNPCWeaponType[0], roll);
        }
    }
    ~PcModGruntScope() { builder->m_anNPCWeaponType[0] = weapon; builder->m_MeshTintColor = tint; builder->m_bEnableShield=shield; }
};
