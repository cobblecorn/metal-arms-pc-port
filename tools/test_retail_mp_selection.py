"""Compile the production MP map list and unlock parser against retail disc data.

No game launch or player-save access. The expected worlds/callbacks come from the
retail DOL's level records, independent of the source table being tested.
"""
import json
import struct
import subprocess
from pathlib import Path
from dol_vocab import Dol
from gamedata_dump import decode
from test_coop_checkpoint_rat import method

ROOT = Path(__file__).resolve().parent.parent
OUT = ROOT / 'build/test-retail-mp-selection'


def main():
    header = (ROOT / 'ma/App/ma/level.h').read_text()
    start = header.index('typedef enum {')
    enum = header[start:header.index('} Level_e;', start) + len('} Level_e;')]
    source = (ROOT / 'ma/App/ma/level.cpp').read_text()
    start = source.index('#if FANG_WINGC', source.index('// MULTIPLAYER LEVELS'))
    initializer = source[start:source.index('\t// MUST BE NULL TERMINATED', start)]
    wrapper = (ROOT / 'ma/App/ma/wpr_system.cpp').read_text()
    parser = method(wrapper, 'static BOOL _InitUnLockInfo( _MPLevelUnlockInfo_t *pUnLockInfo, FGameDataTableHandle_t hTable, cchar *pszTableName ) {')
    tables = decode((ROOT / 'gamedata/mst/multi_lvl$.csv').read_bytes())['tables']
    menu = next(t['fields'] for t in tables if t['name'] == 'deathmatchlevels')
    unlock_tables = decode((ROOT / 'gamedata/mst/pick_level$.csv').read_bytes())['tables']
    unlocking = [f['value'] for t in unlock_tables if t['name'] == 'level_unlocking' for f in t['fields']]
    assert unlocking, 'Retail unlock thresholds must exist'
    dol = Dol((ROOT / 'gamedata/sys/main.dol').read_bytes())
    world_string = dol.data.index(b'WE01multi01\0')
    address = dol.to_addr(world_string)
    references = [i for i in range(0, len(dol.data) - 44, 4)
                  if struct.unpack_from('>I', dol.data, i)[0] == address
                  and struct.unpack_from('>I', dol.data, i + 4)[0] == 42]
    assert len(references) == 1, 'Retail MP level-array anchor must be unique'
    checks = []
    evidence = []
    for i in range(len(menu) // 6):
        words = struct.unpack_from('>11I', dol.data, references[0] - 4 + i * 44)
        world, csv, material = (dol.cstr(words[k]) for k in (1, 3, 4))
        level_id = int(menu[i * 6 + 5]['value']) - 1
        assert level_id == words[2]
        checks.extend([
            f'CHECK(maps[{i}].nLevel == {level_id});',
            f'CHECK(!std::strcmp(maps[{i}].world, {json.dumps(world)}));',
            f'CHECK(!std::strcmp(maps[{i}].csv, {json.dumps(csv)}));',
            f'CHECK(!std::strcmp(maps[{i}].material, {json.dumps(material)}));',
            f'CHECK((maps[{i}].load != nullptr) == {int(words[5] != 0)});',
            f'CHECK((maps[{i}].unload != nullptr) == {int(words[6] != 0)});',
            f'CHECK((maps[{i}].work != nullptr) == {int(words[7] != 0)});',
        ])
        evidence.append({'menu': menu[i * 6 + 1]['value'], 'level_id': level_id, 'world': world})
    code = '''#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>
#define FANG_WINGC 1
using u32=unsigned; using u8=unsigned char; using f32=float; using BOOL=int; using cchar=const char;
constexpr BOOL TRUE=1, FALSE=0;
#define DEVPRINTF(...) ((void)0)
#define FMATH_CLAMP(v,lo,hi) (v=((v)<(lo)?(lo):((v)>(hi)?(hi):(v))))
int count=0;
#define CHECK(x) do {++count;if(!(x)){std::fprintf(stderr,"FAIL line %d: %s\\n",__LINE__,#x);std::exit(1);}}while(0)
struct CColiseumMiniGame {static void LoadMP(){} static void UnloadMP(){} static void WorkMP(){}};
'''
    code += enum + '''
struct Map {cchar* title; cchar* world; Level_e nLevel; cchar* csv; cchar* material; void(*load)(); void(*unload)(); void(*work)(); void(*draw)();};
Map maps[]={
''' + initializer + '''};
struct _MPLevelUnlockInfo_t {u8 nNumFreebies; u8* panChipsToUnlock;};
using FGameDataTableHandle_t=int; using FGameData_VarType_e=int;
std::vector<float> fields;
u32 fgamedata_GetNumFields(int){return (u32)fields.size();}
void* fgamedata_GetPtrToFieldData(int,u32 i,int&){return &fields.at(i);}
void* fres_Alloc(u32 size){return std::malloc(size);}
''' + parser
    code += '\nint main(){\n'
    code += f'CHECK(LEVEL_SINGLE_PLAYER_COUNT==42); CHECK(LEVEL_MULTIPLAYER_COUNT=={len(menu)//6}); CHECK(sizeof(maps)/sizeof(maps[0])==LEVEL_MULTIPLAYER_COUNT);\n'
    code += '\n'.join(checks) + '\n'
    code += 'fields={' + ','.join(str(v) + 'f' for v in unlocking) + '};\n'
    code += '''_MPLevelUnlockInfo_t info={};
CHECK(_InitUnLockInfo(&info,0,"level_unlocking")); CHECK(info.nNumFreebies==fields[0]);
for(u32 i=0;i<LEVEL_MULTIPLAYER_COUNT-info.nNumFreebies;++i) CHECK(info.panChipsToUnlock[i]==fields[i+1]);
std::free(info.panChipsToUnlock);
fields.pop_back(); CHECK(!_InitUnLockInfo(&info,0,"truncated"));
fields.push_back(999); fields.push_back(999); CHECK(!_InitUnLockInfo(&info,0,"extra"));
std::printf("PASS: %d production MP map/unlock checks; all 14 retail worlds matched; no game launched.\\n",count);
}'''
    OUT.mkdir(parents=True, exist_ok=True)
    (OUT / 'mp_selection.cpp').write_text(code)
    (OUT / 'evidence.json').write_text(json.dumps(evidence, indent=2))
    (OUT / 'CMakeLists.txt').write_text('cmake_minimum_required(VERSION 3.20)\nproject(retail_mp_selection LANGUAGES CXX)\nadd_executable(mp_selection mp_selection.cpp)\n')
    for command in (['cmake','-S',str(OUT),'-B',str(OUT/'out'),'-A','Win32'],
                    ['cmake','--build',str(OUT/'out'),'--config','Release']):
        run = subprocess.run(command, capture_output=True, text=True)
        if run.returncode:
            raise SystemExit(run.stdout + run.stderr)
    subprocess.run([str(OUT/'out/Release/mp_selection.exe')], check=True)


if __name__ == '__main__':
    main()
