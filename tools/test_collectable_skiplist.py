"""Offline pickup collision regression; extracts production methods, never runs game."""
from pathlib import Path
import re
import subprocess

ROOT = Path(__file__).resolve().parent.parent
OUT = ROOT / 'build/test-collectable-skiplist'


def method(source, signature):
    source = re.sub(r'/\*.*?\*/|//[^\n]*', '', source, flags=re.S)
    start = source.index(signature)
    end = source.index('{', start) + 1
    depth = 1
    while depth:
        depth += (source[end] == '{') - (source[end] == '}')
        end += 1
    return source[start:end] + '\n'


BASE = r'''
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <stdexcept>
using u32=unsigned;using f32=float;
#define FASSERT(x) do{if(!(x))throw std::runtime_error("skiplist overflow");}while(0)
constexpr unsigned FWORLD_MAX_SKIPLIST_ENTRIES=100;
constexpr bool TRUE=true;
constexpr float _COLLECTABLE_GRAVITY=-44, _MESH_COL_SPHERE_SIZE=1.5f;
float FLoop_fPreviousLoopSecs=0;
int checks=0,queries=0,clears=0,responses=0;
bool simulateImpact=false;
void require(bool ok,const char* why){checks++;if(!ok){std::fprintf(stderr,"FAIL: %s\n",why);std::exit(1);}}
struct CFVec3A{
 float x=0,y=0,z=0;
 void Mul(const CFVec3A& a,float s){x=a.x*s;y=a.y*s;z=a.z*s;}
 void Mul(float s){x*=s;y*=s;z*=s;}
 void Add(const CFVec3A& a,const CFVec3A& b){x=a.x+b.x;y=a.y+b.y;z=a.z+b.z;}
 void Add(const CFVec3A& b){x+=b.x;y+=b.y;z+=b.z;}
 void Sub(const CFVec3A& a,const CFVec3A& b){x=a.x-b.x;y=a.y-b.y;z=a.z-b.z;}
 void Set(const CFVec3A& a){*this=a;}
 float MagSq(){return x*x+y*y+z*z;}
 void Unitize(){Mul(1/std::sqrt(MagSq()));}
 void Zero(){x=y=z=0;}
};
struct CFWorldTracker{int id;};
u32 FWorld_nTrackerSkipListCount=0;
CFWorldTracker* FWorld_apTrackerSkipList[FWORLD_MAX_SKIPLIST_ENTRIES];
struct CEntity{
 CFWorldTracker* m_pWorldMesh;
 bool IsCreated(){return true;}
 virtual void AppendTrackerSkipList(u32& n=FWorld_nTrackerSkipListCount,CFWorldTracker** a=FWorld_apTrackerSkipList){
  FASSERT(n<FWORLD_MAX_SKIPLIST_ENTRIES);a[n++]=m_pWorldMesh;
 }
};
struct Resource{CFWorldTracker* m_pWorldMesh;};
struct CWeaponChaingun:CEntity{
 Resource* m_pResourceData;
 void AppendTrackerSkipList(u32&,CFWorldTracker**) override;
};
struct CBotTitan:CEntity{
 CEntity* m_pShield=nullptr;CEntity* m_apWeapon[1]{};CEntity* m_aapDupWeapons[1][1]{};
 CEntity* m_pDataPortMeshEntity=nullptr;
 void AppendTrackerSkipList(u32&,CFWorldTracker**) override;
};
struct FCollImpact_t{float fImpactDistInfo=.5f;};
FCollImpact_t impact;
FCollImpact_t* FColl_apSortedImpactBuf[1]{&impact};
u32 FColl_nImpactCount=0;
struct Sphere{void Init(CFVec3A*,CFVec3A*,float){}};
struct CollInfo{void* pTag=nullptr;Sphere ProjSphere;};
void fcoll_Clear(){clears++;FColl_nImpactCount=0;}
void fcoll_Sort(bool){}
void fworld_CollideWithWorldTris(CollInfo*){}
CFWorldTracker* expected[5]{};
u32 expectedCount=0;
void fworld_CollideWithTrackers(const int*,CFWorldTracker*){
 queries++;
#if FANG_WINGC
 require(FWorld_nTrackerSkipListCount==expectedCount,"query excludes only the current rising pickup's owner");
 for(u32 i=0;i<expectedCount;i++)require(FWorld_apTrackerSkipList[i]==expected[i],"owner body/attachments all preserved in exclusion list");
#endif
 if(simulateImpact)FColl_nImpactCount=1;
}
struct CCollectable{
 enum{STATE_FLOAT_UP,STATE_STOPPED,STATE_MOVING};
 int m_eState=STATE_MOVING;
 CFVec3A m_LastPos,m_VelocityWS;
 struct{CFVec3A m_vPos;}m_MtxToWorld;
 CollInfo m_CollInfo;
 CEntity* m_pSpawnIgnoreEntity=nullptr;
 CFWorldTracker* m_pWorldMesh=nullptr;
 static constexpr int CollProjSphereInfo=0;
 void _HandleCollision();
 void _CollisionResponse(FCollImpact_t*){responses++;}
};
'''

CHECKS = r'''
int main(){
 CFWorldTracker meshes[6]{{0},{1},{2},{3},{4},{5}};
 CBotTitan titan;titan.m_pWorldMesh=&meshes[0];
 CEntity shield,port;shield.m_pWorldMesh=&meshes[1];port.m_pWorldMesh=&meshes[4];
 CWeaponChaingun gun,duplicate;Resource resources[2]{{&meshes[2]},{&meshes[3]}};
 gun.m_pResourceData=&resources[0];duplicate.m_pResourceData=&resources[1];
 titan.m_pShield=&shield;titan.m_apWeapon[0]=&gun;titan.m_aapDupWeapons[0][0]=&duplicate;titan.m_pDataPortMeshEntity=&port;
 for(int i=0;i<5;i++)expected[i]=&meshes[i];expectedCount=5;
 CCollectable loot;loot.m_pSpawnIgnoreEntity=&titan;loot.m_VelocityWS.y=1;
#if !FANG_WINGC
 bool overflow=false;
 try{for(int i=0;i<21;i++)loot._HandleCollision();}catch(const std::runtime_error&){overflow=true;}
 require(overflow&&FWorld_nTrackerSkipListCount==100,"legacy pickup queries reproduce overflow with Titan attachments");
#else
 for(int frame=0;frame<300;frame++){
  FWorld_nTrackerSkipListCount=100; // preceding unrelated ray/collision query
  for(int i=0;i<100;i++)loot._HandleCollision();
 }
 require(queries==30000&&FWorld_nTrackerSkipListCount==5,"full pickup pool and repeated frames never accumulate exclusions");
 CEntity other;other.m_pWorldMesh=&meshes[5];
 CCollectable next;next.m_pSpawnIgnoreEntity=&other;next.m_VelocityWS.y=1;
 expectedCount=1;expected[0]=&meshes[5];next._HandleCollision();
 require(FWorld_apTrackerSkipList[0]==other.m_pWorldMesh,"next owner's pickup does not inherit Titan attachments");
 expectedCount=0;next.m_pSpawnIgnoreEntity=nullptr;next._HandleCollision();
 require(FWorld_nTrackerSkipListCount==0,"ownerless pickup does not inherit previous exclusions");
 loot.m_VelocityWS.y=-1;loot._HandleCollision();
 require(!loot.m_pSpawnIgnoreEntity&&FWorld_nTrackerSkipListCount==0,"falling pickup clears owner immunity and stale list");
 for(int state:{CCollectable::STATE_STOPPED,CCollectable::STATE_FLOAT_UP}){
  loot.m_eState=state;int before=queries;loot._HandleCollision();
  require(queries==before,"stationary/floating pickup still performs no collision query");
 }
 loot.m_eState=CCollectable::STATE_MOVING;FLoop_fPreviousLoopSecs=.01f;
 loot.m_VelocityWS={1,0,0};loot.m_MtxToWorld.m_vPos={3,4,5};loot._HandleCollision();
 require(std::fabs(loot.m_MtxToWorld.m_vPos.x-3.01f)<.00001f,"normal movement remains unchanged");
 require(std::fabs(loot.m_VelocityWS.y+.44f)<.00001f,"normal gravity remains unchanged");
 simulateImpact=true;loot._HandleCollision();require(responses==1,"normal impact response still runs");
 require(clears==queries,"each moving query still resets the collision impact buffer");
#endif
 std::printf("PASS: %d pickup collision checks (PC=%d); game not launched.\n",checks,FANG_WINGC);
}
'''


def main():
    code = BASE
    for path, signature in (
        ('ma/App/ma/weapon_chaingun.cpp', 'void CWeaponChaingun::AppendTrackerSkipList('),
        ('ma/App/ma/bottitan.cpp', 'void CBotTitan::AppendTrackerSkipList('),
        ('ma/App/ma/collectable.cpp', 'void CCollectable::_HandleCollision('),
    ):
        code += method((ROOT / path).read_text(), signature)
    OUT.mkdir(parents=True, exist_ok=True)
    (OUT / 'collision.cpp').write_text(code + CHECKS)
    (OUT / 'CMakeLists.txt').write_text(
        'cmake_minimum_required(VERSION 3.20)\nproject(pickup_collision LANGUAGES CXX)\n'
        'foreach(pc IN ITEMS 0 1)\nadd_executable(collision${pc} collision.cpp)\n'
        'target_compile_features(collision${pc} PRIVATE cxx_std_17)\n'
        'target_compile_definitions(collision${pc} PRIVATE FANG_WINGC=${pc})\nendforeach()\n')
    for command in (['cmake', '-S', str(OUT), '-B', str(OUT / 'out'), '-A', 'Win32'],
                    ['cmake', '--build', str(OUT / 'out'), '--config', 'Release']):
        result = subprocess.run(command, capture_output=True, text=True)
        if result.returncode:
            raise SystemExit(result.stdout + result.stderr)
    for pc in (0, 1):
        subprocess.run([str(OUT / f'out/Release/collision{pc}.exe')], check=True)


if __name__ == '__main__':
    main()
