#include "pc_mods.h"
#include "fdata.h"
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <list>
#include <string>

namespace {
struct Config {
    char root[1024], ini[1100];
    BOOL enabled;
    Config() : enabled(FALSE) {
        root[0] = ini[0] = 0;
        char supplied[1024];
        const DWORD count = GetEnvironmentVariableA("MA_PORT_MOD_ROOT", supplied, sizeof(supplied));
        if (!count || count >= sizeof(supplied)) return;
        const DWORD length = GetFullPathNameA(supplied, sizeof(root), root, NULL);
        if (!length || length >= sizeof(root)) { root[0] = 0; return; }
        snprintf(ini, sizeof(ini), "%s\\runtime.ini", root);
        enabled = GetPrivateProfileIntA("profile", "api", 0, ini) == 1;
        if (enabled) DEVPRINTF("PORT-MODS MA_PORT_MOD_API_V1 MA_PORT_MOD_API_V2 MA_PORT_MOD_API_V3 MA_PORT_MOD_API_V4 profile=%s\n", root);
    }
};
Config &config() { static Config value; return value; }

struct StreamEntry {
    FFileMasterEntry_t entry;
    std::string path;
};
std::list<StreamEntry> streams; // Stable entry addresses, no open handles retained.

u32 hashBytes(u32 value, const void *bytes, unsigned count) {
    const unsigned char *p = (const unsigned char *)bytes;
    while (count--) { value ^= *p++; value *= 16777619u; }
    return value;
}
}

BOOL pcmods_ResolveFile(const char *requested, char *output, unsigned capacity) {
    const Config &cfg = config();
    if (!cfg.enabled || !requested || !output || !capacity) return FALSE;
    const char *name = requested;
    for (const char *p = requested; *p; ++p) if (*p == '/' || *p == '\\' || *p == ':') name = p + 1;
    if (!*name || strlen(name) >= FDATA_PRJFILE_FILENAME_LEN || strchr(name, ':') ||
        strchr(name, '*') || strchr(name, '?') || !strcmp(name, ".") || !strcmp(name, "..")) return FALSE;
    const int length = snprintf(output, capacity, "%s\\resources\\%s", cfg.root, name);
    // The legacy localized OS-file path uses a 255-byte stack buffer.
    if (length < 0 || length >= 255 || (unsigned)length >= capacity) return FALSE;
    const DWORD flags = GetFileAttributesA(output);
    if (!_stricmp(name,"barteritems.csv") || !_stricmp(name,"wedmmines01.csv")) DEVPRINTF("PORT-MODS lookup request=%s path=%s flags=%lu\n",requested,output,flags);
    return flags != INVALID_FILE_ATTRIBUTES && !(flags & FILE_ATTRIBUTE_DIRECTORY);
}

const FFileMasterEntry_t *pcmods_OpenStream(const char *requested, u32 *size) {
    char path[1200];
    if (!pcmods_ResolveFile(requested, path, sizeof(path))) return NULL;
    for (auto &stream : streams) {
        if (!_stricmp(stream.path.c_str(), path)) {
            if (size) *size = stream.entry.nNumBytes;
            return &stream.entry;
        }
    }
    WIN32_FILE_ATTRIBUTE_DATA info;
    if (!GetFileAttributesExA(path, GetFileExInfoStandard, &info) || info.nFileSizeHigh ||
        info.nFileSizeLow > 64u * 1024u * 1024u) return NULL;
    StreamEntry stream = {};
    stream.entry.nNumBytes = info.nFileSizeLow;
    stream.path = path;
    streams.push_back(stream);
    if (size) *size = stream.entry.nNumBytes;
    DEVPRINTF("PORT-MODS stream override %s\n", path);
    return &streams.back().entry;
}

BOOL pcmods_ReadStream(const FFileMasterEntry_t *entry, u32 start, u32 amount,
    void *destination, FFileAsyncReadCallback_t *callback, void *user, s32 *result) {
    for (const auto &stream : streams) {
        if (&stream.entry != entry) continue;
        *result = FFILE_ERROR_RET;
        if (start > entry->nNumBytes || amount > entry->nNumBytes - start) return TRUE;
        HANDLE file = CreateFileA(stream.path.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL,
            OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if (file == INVALID_HANDLE_VALUE) return TRUE;
        DWORD read = 0;
        const BOOL ok = SetFilePointer(file, start, NULL, FILE_BEGIN) != INVALID_SET_FILE_POINTER &&
            ReadFile(file, destination, amount, &read, NULL) && read == amount;
        CloseHandle(file);
        if (ok) {
            // The texture stream consumer sets its in-flight state before requesting data;
            // completing a small loose override immediately follows the same callback contract.
            *result = (s32)read;
            if (callback) callback(0, FFILE_INVALID_HANDLE, destination, read, user);
        }
        return TRUE;
    }
    return FALSE;
}

BOOL pcmods_Option(const char *option) {
    const Config &cfg = config();
    return cfg.enabled && GetPrivateProfileIntA("randomizer", option, 0, cfg.ini) != 0;
}

s32 pcmods_Value(const char *option, s32 fallback) {
    const Config &cfg = config();
    return cfg.enabled ? (s32)GetPrivateProfileIntA("randomizer",option,fallback,cfg.ini) : fallback;
}

u32 pcmods_Seed() {
    const Config &cfg = config();
    char value[32];
    GetPrivateProfileStringA("randomizer", "seed", "0", value, sizeof(value), cfg.ini);
    return (u32)strtoul(value, NULL, 10);
}

u32 pcmods_Roll(const char *world, const char *identity, const void *position, unsigned positionBytes, u32 salt) {
    u32 value = 2166136261u ^ pcmods_Seed() ^ salt;
    if (world) value = hashBytes(value, world, (unsigned)strlen(world));
    if (identity) value = hashBytes(value, identity, (unsigned)strlen(identity));
    if (position) value = hashBytes(value, position, positionBytes);
    value ^= value >> 16; value *= 0x7feb352du; value ^= value >> 15;
    value *= 0x846ca68bu; return value ^ (value >> 16);
}

BOOL pcmods_Feature(const char *name) {
    const Config &cfg=config();
    return cfg.enabled && GetPrivateProfileIntA("features",name,0,cfg.ini)!=0;
}
unsigned pcmods_CustomLevelCount() {
    const Config &cfg=config();
    const int count=cfg.enabled ? GetPrivateProfileIntA("levels","count",0,cfg.ini) : 0;
    return count>=0 && count<=16 ? (unsigned)count : 0;
}
const PcModLevel *pcmods_CustomLevel(unsigned index) {
    static PcModLevel levels[16] = {};
    static BOOL loaded[16] = {};
    if(index>=pcmods_CustomLevelCount()) return NULL;
    PcModLevel &level=levels[index];
    if(!loaded[index]) {
        const Config &cfg=config();char section[32];
        snprintf(section,sizeof(section),"level.%u",index);
        GetPrivateProfileStringA(section,"title","",level.title,sizeof(level.title),cfg.ini);
        GetPrivateProfileStringA(section,"world","",level.world,sizeof(level.world),cfg.ini);
        GetPrivateProfileStringA(section,"csv","",level.csv,sizeof(level.csv),cfg.ini);
        GetPrivateProfileStringA(section,"materials","ms_mp_01",level.materials,sizeof(level.materials),cfg.ini);
        const char *names[]={level.world,level.csv,level.materials};
        for(const char *name:names) {
            if(!*name) return NULL;
            for(const char *c=name;*c;++c) if(!((*c>='a'&&*c<='z')||(*c>='A'&&*c<='Z')||(*c>='0'&&*c<='9')||*c=='_'||*c=='$')) return NULL;
        }
        if(!*level.title) return NULL;
        loaded[index]=TRUE;
    }
    return &level;
}
BOOL pcmods_IsCustomMPIndex(unsigned menuIndex) {
    return menuIndex>=14 && menuIndex<14+pcmods_CustomLevelCount();
}
BOOL pcmods_IsCustomLevel(s32 levelId) {
    return levelId>=56 && (unsigned)levelId<56+pcmods_CustomLevelCount();
}
BOOL pcmods_ResolveMedia(const char *requested,char *output,unsigned capacity) {
    const Config &cfg=config();
    if(!cfg.enabled || !requested || !output || !capacity) return FALSE;
    const char *name=requested;
    for(const char *c=requested;*c;++c) if(*c=='/' || *c=='\\' || *c==':') name=c+1;
    if(!*name || strlen(name)>79 || !strcmp(name,".") || !strcmp(name,"..")) return FALSE;
    for(const char *c=name;*c;++c) if(!((*c>='a'&&*c<='z')||(*c>='A'&&*c<='Z')||(*c>='0'&&*c<='9')||*c=='_'||*c=='.'||*c=='-')) return FALSE;
    const int length=snprintf(output,capacity,"%s\\media\\%s",cfg.root,name);
    if(length<0 || (unsigned)length>=capacity || length>=255) return FALSE;
    const DWORD flags=GetFileAttributesA(output);
    return flags!=INVALID_FILE_ATTRIBUTES && !(flags&FILE_ATTRIBUTE_DIRECTORY);
}

BOOL pcmods_HasMovies() {
    const Config &cfg=config();
    if(!cfg.enabled) return FALSE;
    char pattern[1200];
    const int length=snprintf(pattern,sizeof(pattern),"%s\\media\\*.bik",cfg.root);
    if(length<0 || length>=255) return FALSE;
    WIN32_FIND_DATAA data;HANDLE find=FindFirstFileA(pattern,&data);
    if(find==INVALID_HANDLE_VALUE) return FALSE;
    BOOL found=FALSE;
    do {if(!(data.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)){found=TRUE;break;}} while(FindNextFileA(find,&data));
    FindClose(find);return found;
}
