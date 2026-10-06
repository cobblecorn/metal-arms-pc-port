"""Offline booth-cutscene arrival regression; never launches the game.

Extracts the native and waypoint stopping method from production. Uses the
retail destination/trigger geometry to reproduce early completion outside the
trigger, then checks the replacement goal and exclusions with AI call spies.
"""
from pathlib import Path
import re
import struct
import subprocess

from gamedata_dump import decode

ROOT = Path(__file__).resolve().parent.parent
OUT = ROOT / 'build/test-booth-arrival'


def method(source, signature):
    source = re.sub(r'/\*.*?\*/|//[^\n]*', '', source, flags=re.S)
    start = source.index(signature)
    end = source.index('{', start) + 1
    depth = 1
    while depth:
        depth += (source[end] == '{') - (source[end] == '}')
        end += 1
    return source[start:end] + '\n'


def geometry():
    data = (ROOT / 'gamedata/mst/wewrresrch2.wld').read_bytes()
    init = struct.unpack_from('>11I', data)[9]
    base = init + 16
    found = {}
    for i in range(struct.unpack_from('>I', data, init)[0]):
        entry = base + i * 64
        kind, shape = struct.unpack_from('>II', data, entry)
        gd = struct.unpack_from('>I', data, entry + 60)[0]
        if not gd:
            continue
        size = struct.unpack_from('>I', data, base + gd)[0]
        tables = decode(data[base + gd:base + gd + size])['tables']
        names = [t['fields'][0]['value'] for t in tables if t['name'] == 'name']
        if names and names[0] in ('elitegoto2', 'trigger_group'):
            pos = struct.unpack_from('>3f', data, entry + 44)
            found[names[0]] = pos
            if names[0] == 'trigger_group':
                assert kind == 3
                matrix = struct.unpack_from('>9f', data, entry + 8)
                assert all(abs(matrix[j]) < 1e-6 for j in (1, 2, 3, 5, 6, 7))
                dims = struct.unpack_from('>3f', data, base + shape)
                found['half'] = tuple(dims[j] * matrix[j * 4] / 2 for j in range(3))
    assert len(found) == 3
    return ''.join('CFVec3A ' + name + '={' + ','.join(f'{v:.9f}f' for v in found[key]) + '};\n'
                   for name, key in (('authored', 'elitegoto2'), ('center', 'trigger_group'), ('half', 'half')))


BASE = r'''
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <cstring>
#include <cstdint>
using BOOL=bool;using f32=float;using u8=unsigned char;using u16=unsigned short;
using cell=intptr_t;
#define TRUE true
#define FALSE false
#define FINLINE inline
#define AMX_NATIVE_CALL
#define SCRIPT_CHECK_NUM_PARAMS(...) ((void)0)
#define SCRIPT_ERROR(...) ((void)0)
#define DEVPRINTF(...) ((void)0)
#define FASSERT(x) require(x,"production assertion")
int checks=0;
void require(bool b,const char* why){++checks;if(!b){std::fprintf(stderr,"FAIL: %s\n",why);std::exit(1);}}
int fclib_stricmp(const char* a,const char* b){return _stricmp(a,b);}
struct CFVec3A{
 float x=0,y=0,z=0;
 float DistSqXZ(const CFVec3A& b)const{return (x-b.x)*(x-b.x)+(z-b.z)*(z-b.z);}
 static CFVec3A m_Null;
};
CFVec3A CFVec3A::m_Null;
bool same(CFVec3A a,CFVec3A b){return a.x==b.x&&a.y==b.y&&a.z==b.z;}
struct CFSphereA{CFVec3A m_Pos;float m_fRadius;};
struct CAIPathWaypoint{
 enum{WAYPOINTFLAG_STOP_AT=1};int m_uWayPointFlags=1;
 float m_fCloseEnoughDist=0;CFVec3A m_Location;
};
struct CAIPathWalker{
 struct{bool valid=true;bool IsValid(){return valid;}}m_PathIt;
 CAIPathWaypoint point;
 CAIPathWaypoint* GetCurWaypoint(){return &point;}
 BOOL CloseEnoughXZ(const CFSphereA&);
};
const f32 kAbsoluteCloseEnough=1;
struct Mtx{CFVec3A m_vPos;};
struct CEntity{
 enum{TRIPWIRE_COLLFLAG_NEWPOS_INSIDE=4};
 const char* name="";Mtx m;CFVec3A extent;
 bool inWorld=true,tripwire=false,armed=true;static CEntity* arrival;
 bool IsInWorld(){return inWorld;}const char* Name(){return name;}
 Mtx* MtxToWorld(){return &m;}unsigned Guid(){return 567;}
 bool IsTripwire(){return tripwire;}bool IsTripwireArmed(){return armed;}
 static CEntity* FindInWorld(const char* n){require(!std::strcmp(n,"trigger_group"),"correct trigger query");return arrival;}
 unsigned TripwireCollisionTest(const CFVec3A* old,const CFVec3A* now){
  require(same(*old,*now),"read-only point query");
  CFVec3A c=m.m_vPos;
  return std::fabs(now->x-c.x)<=extent.x&&std::fabs(now->y-c.y)<=extent.y&&std::fabs(now->z-c.z)<=extent.z?4:0;
 }
 BOOL TripwireContainsPoint(const CFVec3A&);
};
CEntity* CEntity::arrival=nullptr;
struct CBot:CEntity{
 static bool m_bCutscenePlaying;bool dead=false;int m_nPossessionPlayerIndex=-1;
 bool IsDeadOrDying(){return dead;}void* AIBrain(){return this;}
};bool CBot::m_bCutscenePlaying=true;
struct{bool campaign=true;bool IsSinglePlayer(){return campaign;}}MultiplayerMgr;
int Level_nLoadedIndex=0;
struct{const char* pszWorldResName;}Level_aInfo[1]{{"WEWRresrch2"}};
enum{GOTOFLAG_NONE=0,GOTOFLAG_USE_JOB_REACT_RULES=1,GOTOFLAG_RETURN_FIRE_OK=2,GOTLFLAG_TRIGGER_SCRIPT_EVENT_AT_END=4};
struct AMX{};
struct CMAST_BotWrapper{static cell Bot_GotoE(AMX*,cell*);};
CFVec3A captured;int calls=0;bool looking=false;unsigned guid=0,pct=0,fudge=0,speed=0,flags=0;
void ai_AssignGoal_Goto(void*,const CFVec3A& p,u8 f,u8 s,u16 fl){++calls;captured=p;looking=false;fudge=f;speed=s;flags=fl;}
void ai_AssignGoal_GotoWithLookAt(void* brain,const CFVec3A& p,const CFVec3A& look,unsigned g,u8 pc,u8 f,u8 s,u16 fl){
 require(same(look,CFVec3A::m_Null),"look point unchanged");
 ai_AssignGoal_Goto(brain,p,f,s,fl);looking=true;guid=g;pct=pc;
}
'''

CHECKS = r'''
int main(){
 CBot bot;bot.name="elite2";
 CEntity dest;dest.name="elitegoto2";dest.m.m_vPos=authored;
 CEntity trigger;trigger.name="trigger_group";trigger.tripwire=true;trigger.m.m_vPos=center;trigger.extent=half;
 CEntity look;CEntity::arrival=&trigger;
 CFVec3A goal=authored;
 require(_PortBoothEntranceGoal(&bot,&dest,goal),"campaign scene corrected");
 require(goal.x==center.x&&goal.z==center.z&&goal.y==authored.y,"center XZ with floor Y");
 require(same(dest.m.m_vPos,authored),"retail waypoint untouched");
 CAIPathWalker path;path.point.m_Location=authored;
 CFVec3A stopped=authored;stopped.x=center.x-half.x-.1f;
 require(!trigger.TripwireContainsPoint(stopped),"legacy stopped position outside arrival trigger");
 require(path.CloseEnoughXZ({stopped,2}),"production waypoint tolerance accepts legacy position");
 path.point.m_Location=goal;
 require(!path.CloseEnoughXZ({stopped,2}),"new goal requires guard to continue inside");
 for(int radius=1;radius<=15;radius++){
  float limit=std::fmin(10.f,1.f+radius);
  for(int angle=0;angle<360;angle+=3){
   float a=angle*.01745329252f;
   CFVec3A p=goal;p.x+=(limit-.01f)*std::cos(a);p.z+=(limit-.01f)*std::sin(a);
   require(path.CloseEnoughXZ({p,float(radius)}),"production stop radius");
   // Include the native's unchanged two-foot destination search fudge.
   p.x+=2*std::cos(a);p.z+=2*std::sin(a);
   require(trigger.TripwireContainsPoint(p),"normal stop plus path fudge still inside trigger");
  }
 }
 cell args[8]={28,(cell)&bot,(cell)&dest,0,100,0,0,2};
 for(int withLook=0;withLook<2;withLook++){
  args[5]=withLook?(cell)&look:0;int before=calls;
  CMAST_BotWrapper::Bot_GotoE(nullptr,args);
  require(calls==before+1&&same(captured,goal),"both native AI branches use corrected goal");
  require(looking==bool(withLook)&&fudge==2&&speed==100&&flags==2,"native options preserved");
  if(withLook)require(guid==567&&pct==0,"look target preserved");
 }
 for(int excluded=0;excluded<13;excluded++){
  MultiplayerMgr.campaign=true;CBot::m_bCutscenePlaying=true;Level_nLoadedIndex=0;
  Level_aInfo[0].pszWorldResName="WEWRresrch2";bot.name="elite2";bot.inWorld=true;bot.dead=false;bot.m_nPossessionPlayerIndex=-1;
  dest.name="elitegoto2";CEntity::arrival=&trigger;trigger.inWorld=true;trigger.tripwire=true;trigger.armed=true;trigger.extent=half;
  switch(excluded){
   case 0:MultiplayerMgr.campaign=false;break;
   case 1:CBot::m_bCutscenePlaying=false;break;
   case 2:Level_nLoadedIndex=-1;break;
   case 3:Level_aInfo[0].pszWorldResName="WEWRresrch1";break;
   case 4:bot.name="elite1";break;
   case 5:dest.name="elitegoto1";break;
   case 6:bot.inWorld=false;break;
   case 7:bot.dead=true;break;
   case 8:bot.m_nPossessionPlayerIndex=1;break;
   case 9:CEntity::arrival=nullptr;break;
   case 10:trigger.tripwire=false;break;
   case 11:trigger.armed=false;break;
   case 12:trigger.extent.y=1;break;
  }
  CFVec3A unchanged=authored;
  require(!_PortBoothEntranceGoal(&bot,&dest,unchanged)&&same(unchanged,authored),"excluded case leaves destination alone");
 }
 MultiplayerMgr.campaign=true;CBot::m_bCutscenePlaying=true;Level_nLoadedIndex=0;
 Level_aInfo[0].pszWorldResName="WEWRresrch2";
 require(!_PortBoothEntranceGoal(nullptr,&dest,goal),"null actor excluded");
 require(!_PortBoothEntranceGoal(&bot,nullptr,goal),"null destination excluded");
 std::printf("PASS: %d booth arrival checks; game not launched.\n",checks);
}
'''


def main():
    native = (ROOT / 'ma/App/ma/MAScriptTypes.cpp').read_text()
    code = BASE + geometry()
    for text, signature in (
        ((ROOT / 'ma/App/ma/entity.h').read_text(), 'FINLINE BOOL TripwireContainsPoint('),
        ((ROOT / 'ma/App/ma/Ai/AIPath.cpp').read_text(), 'BOOL CAIPathWalker::CloseEnoughXZ('),
        (native, 'static BOOL _PortBoothEntranceGoal('),
        (native, 'cell AMX_NATIVE_CALL CMAST_BotWrapper::Bot_GotoE('),
    ):
        part = method(text, signature)
        if signature.startswith('FINLINE'):
            part = part.replace('FINLINE BOOL TripwireContainsPoint(', 'BOOL CEntity::TripwireContainsPoint(')
        code += part
    OUT.mkdir(parents=True, exist_ok=True)
    (OUT / 'arrival.cpp').write_text(code + CHECKS)
    (OUT / 'CMakeLists.txt').write_text(
        'cmake_minimum_required(VERSION 3.20)\nproject(booth_arrival LANGUAGES CXX)\n'
        'add_executable(arrival arrival.cpp)\ntarget_compile_features(arrival PRIVATE cxx_std_17)\n'
        'target_compile_definitions(arrival PRIVATE FANG_WINGC=1)\n')
    for command in (['cmake', '-S', str(OUT), '-B', str(OUT / 'out'), '-A', 'Win32'],
                    ['cmake', '--build', str(OUT / 'out'), '--config', 'Release']):
        result = subprocess.run(command, capture_output=True, text=True)
        if result.returncode:
            raise SystemExit(result.stdout + result.stderr)
    subprocess.run([str(OUT / 'out/Release/arrival.exe')], check=True)


if __name__ == '__main__':
    main()
