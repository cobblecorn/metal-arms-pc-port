"""Check retail door/color evidence and compile the actual door/command helpers.

Native probes separately exercise real atlases, pickup pools and voice emitters.
This fixture checks missing meshes/layers, repeated lock changes, and decoy parity.
"""
from pathlib import Path
import struct
import subprocess

from dol_vocab import Dol
from gamedata_dump import decode
from test_collectable_skiplist import method

ROOT = Path(__file__).resolve().parent.parent
OUT = ROOT / "build/test-retail-door-commands"

FIXTURE = r'''
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>
using BOOL=int;using u32=unsigned;using f32=float;using cchar=const char;
using FMeshTexLayerHandle_t=int;
#define FANG_WINGC 1
#define TRUE 1
#define FALSE 0
#define fclib_stricmp _stricmp
int checks=0;
void require(bool ok,const char* label){++checks;if(!ok){std::fprintf(stderr,"FAIL: %s\n",label);std::exit(1);}}
struct CFVec2{float x=7,y=9;};
struct TC{CFVec2 st,rate;bool animate=true,apply=false;};
struct Layer{TC* pAnimTC;};
struct CFWorldMesh {
 void* m_pMesh=(void*)1;int id=20;TC tc;Layer layer{&tc};
 int GetTexLayerHandle(int n){return n==id?0:-1;}
 Layer* GetTexLayerID(int h){require(h==0,"valid atlas handle");return &layer;}
 void AnimTC_AnimateScroll(int,bool b){tc.animate=b;}
 void AnimTC_SetScrollRate(int,CFVec2 v){tc.rate=v;}
 void AnimTC_ApplyToShader(int,bool b){tc.apply=b;}
 void AnimTC_SetScrollST(int,CFVec2 v){tc.st=v;}
};
struct CDoorEntity {
 enum {DOOR_BEHAVIOR_DOOR,DOOR_BEHAVIOR_OSCILLATE,DOOR_BEHAVIOR_ELEVATOR,DOOR_BEHAVIOR_OTHER};
 bool m_bLocked=false;int m_eBehavior=0;CFWorldMesh* mesh=nullptr;
 CFWorldMesh* GetMeshInst(){return mesh;}
 void SetLockState(BOOL);
};
'''


def main():
    retail = Dol((ROOT / "gamedata/sys/main.dol").read_bytes())
    floating = lambda a: struct.unpack_from(">f", retail.data, retail.to_off(a))[0]
    assert floating(0x804C2420 - 0x4360) == .5
    assert struct.unpack_from(">I", retail.data, retail.to_off(0x80186328))[0] == 0x2C000003
    for address, value in ((0x804BDF80, 68/255), (0x804BDF84, 138/255),
                           (0x804BDF74, 194/255), (0x804BDF78, 28/255), (0x804BDF7C, 64/255)):
        assert abs(floating(address) - value) < 1e-7
    tables = decode((ROOT / "gamedata/mst/snd_spyvspy.csv").read_bytes())["tables"]
    commands = [t["fields"][0]["value"] for t in tables if t["name"].lower().startswith(("command", "nocommand"))]
    assert len(commands) >= 12
    code = FIXTURE
    code += method((ROOT / "ma/App/ma/Door.cpp").read_text(), "void CDoorEntity::SetLockState(")
    code += method((ROOT / "ma/App/ma/BotTalkInst.cpp").read_text(), "static BOOL _PortIsInstructorCommand(")
    code += '\nint main(){\n'
    for name in commands:
        code += f' require(_PortIsInstructorCommand("{name}"),"authored real and decoy order parity");\n'
    code += r'''
 for(const char* p:{"brgt_impost","brgt_invade","brmg_see01","gl_18i_210","BRGT_CstepL_wrong",""})
  require(!_PortIsInstructorCommand(p),"unrelated NPC speech keeps normal positioning");
 require(!_PortIsInstructorCommand(nullptr),"null command data is safe");
 CDoorEntity d;d.SetLockState(true);require(d.m_bLocked,"lock applies before mesh creation");
 CFWorldMesh m;d.mesh=&m;m.m_pMesh=nullptr;d.SetLockState(false);require(!d.m_bLocked,"missing geometry is safe");
 m.m_pMesh=(void*)1;m.id=1;d.SetLockState(true);require(d.m_bLocked,"doors without indicator layer keep lock behavior");
 m.id=20;m.layer.pAnimTC=nullptr;d.SetLockState(false);require(!d.m_bLocked,"unanimated indicator is safe");
 m.layer.pAnimTC=&m.tc;
 for(int behavior=0;behavior<4;++behavior)for(int pass=0;pass<100;++pass){
  d.m_eBehavior=behavior;bool locked=pass%2;d.SetLockState(locked);
  require(d.m_bLocked==locked,"actual door lock bit is preserved");
  require(m.tc.st.x==(!locked&&behavior!=3?.5f:0.f)&&m.tc.st.y==0,"retail red/green and scripted-door state");
  require(!m.tc.animate&&m.tc.rate.x==0&&m.tc.rate.y==0&&m.tc.apply,"authored scrolling disabled before atlas state");
 }
 std::printf("PASS: %d production door/command checks and retail color constants; no game launched.\n",checks);
}
'''
    OUT.mkdir(parents=True, exist_ok=True)
    (OUT / "checks.cpp").write_text(code)
    (OUT / "CMakeLists.txt").write_text(
        "cmake_minimum_required(VERSION 3.20)\nproject(retail_door_commands LANGUAGES CXX)\n"
        "add_executable(checks checks.cpp)\ntarget_compile_features(checks PRIVATE cxx_std_17)\n")
    for command in (["cmake", "-S", str(OUT), "-B", str(OUT / "out"), "-A", "Win32"],
                    ["cmake", "--build", str(OUT / "out"), "--config", "Release"]):
        result = subprocess.run(command, capture_output=True, text=True)
        if result.returncode:
            raise SystemExit(result.stdout + result.stderr)
    subprocess.run([str(OUT / "out/Release/checks.exe")], check=True)


if __name__ == "__main__":
    main()
