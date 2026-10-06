"""Offline Scatter Blaster firing regression; no game launch or save access.

Executes production L1/L2-L3 fire, ammo consumption, and visual-shell helpers
against checked shell storage, reproducing the original full-clip overrun.
"""
from pathlib import Path
import subprocess
from test_coop_checkpoint_rat import method

ROOT = Path(__file__).resolve().parent.parent
OUT = ROOT / "build/test-blaster-ammo"
FIXTURE = r'''
#include <cstdio>
#include <cstdlib>
#include <stdexcept>
#include <vector>
using u16=unsigned short;using u32=unsigned;using f32=float;using BOOL=int;
constexpr BOOL TRUE=1,FALSE=0;constexpr u16 INFINITE_AMMO=65535;
#define MA_PC_INPUT 1
#define FASSERT(x) ((void)0)
constexpr int FFORCE_EFFECT_ROUGH_RUMBLE=1;
struct CFVec3A {float x=0,y=0,z=0;static const CFVec3A m_UnitAxisY;
 CFVec3A& Mul(float n){x*=n;y*=n;z*=n;return *this;}
 CFVec3A& Add(const CFVec3A&v){x+=v.x;y+=v.y;z+=v.z;return *this;}};
const CFVec3A CFVec3A::m_UnitAxisY={0,1,0};
struct CFMtx43A {CFVec3A m_vPos,m_vRight{1,0,0};};
struct CBot {int m_nPossessionPlayerIndex=0;CFVec3A m_Velocity_WS;};
struct Player {int m_nControllerIndex=0;};Player Player_aPlayer[4];
struct CFCamera {int shakes=0;void ShakeCamera(float,float){shakes++;}};
CFCamera cameras[4];CFCamera* fcamera_GetCameraByIndex(int i){return &cameras[i];}
int rumbleCount[4]={};void fforce_Kill(int*){}void fforce_Play(int i,int,int*){rumbleCount[i]++;}
float fmath_RandomFloatRange(float lo,float){return lo;}
unsigned FLoop_nTotalLoopTicks=100;float FLoop_fSecsPerTick=.016f;
struct CWeapon {u16 m_nClipAmmo=0;int player=0,notifications=0;
 BOOL IsCreated(){return TRUE;}void _AmmoMayHaveChanged(BOOL){notifications++;}
 u16 RemoveFromClip(u16 nRounds,BOOL bNotify=TRUE);u16 GetClipAmmo(){return m_nClipAmmo;}};
unsigned infinitePlayers=0;BOOL pccheats_InfiniteAmmo(const CWeapon*w){return (infinitePlayers&(1u<<w->player))!=0;}
struct Shell {int removals=0;void RemoveFromWorld(){removals++;}};
struct CheckedShells {std::vector<Shell> shells;bool enabled=true;int accesses=0;
 explicit operator bool()const{return enabled;}
 Shell& operator[](unsigned i){accesses++;return shells.at(i);}};
struct CWeaponBlaster:CWeapon {
 struct Props {unsigned nPelletCountPerRound=6;float fShotSpreadFactor=.1f,fOORoundsPerSec=.8f,fUnitCamVibration=.3f,fBarrel2DelayTime=.1f;int pSoundGroupFire=1;}props,*m_pUserProps=&props;
 struct Resources {CheckedShells m_paShellWorldMeshArray;}resources,*m_pResourceData=&resources;
 struct Info {u16 nClipAmmoMax=6;}info,*m_pInfo=&info;
 CBot bot,*m_pOwnerBot=&bot;int m_hForce=0;CFMtx43A matrix;const CFVec3A*m_pvFireBuddyPos_WS=nullptr;
 float m_fSecondsCountdownTimer=0,m_fFireAgainTime=0;bool m_bBoolBarrel2Fire=false;int blasts=0,muzzles=0,sounds=0;
 CFMtx43A* MtxToWorld(){return &matrix;}CBot* GetOwner(){return m_pOwnerBot;}
 BOOL IsOwnedByPlayer(){return m_pOwnerBot&&m_pOwnerBot->m_nPossessionPlayerIndex>=0;}
 void ComputeMuzzlePoint_WS(CFVec3A*p){*p=matrix.m_vPos;}
 void _ScatterBlast(const CFVec3A&,const CFVec3A&,u32,f32){blasts++;}
 void _DrawMuzzleEffects(const CFVec3A*,const CFVec3A*,BOOL){muzzles++;}
 void PlaySound(int){sounds++;}
 void _ConsumeFiredRound();void _L1_Fire(const CFVec3A&);void _L23_Fire(const CFVec3A&);
 void originalShellConsumption();
 void setup(int p,u16 capacity,u16 ammo,bool show){player=p;bot.m_nPossessionPlayerIndex=p;info.nClipAmmoMax=capacity;
  m_nClipAmmo=ammo;resources.m_paShellWorldMeshArray.shells.resize(capacity);resources.m_paShellWorldMeshArray.enabled=show;}
};
int checks=0;void require(bool x,const char*why){checks++;if(!x){std::fprintf(stderr,"FAIL: %s\n",why);std::exit(1);}}
'''
CHECKS = r'''
int main(){
 CFVec3A direction{0,0,1},buddy{1,0,0};
 for(int p=0;p<4;p++)Player_aPlayer[p].m_nControllerIndex=p;
 for(int p=0;p<4;p++)for(unsigned capacity:{1u,2u,6u,12u}){
  infinitePlayers=1u<<p;CWeaponBlaster old;old.setup(p,capacity,capacity,true);bool overrun=false;
  try{old.originalShellConsumption();}catch(const std::out_of_range&){overrun=true;}
  require(overrun,"original full-clip infinite-ammo bug reproduced");
  for(int variant=0;variant<3;variant++)for(bool cheat:{false,true})for(bool show:{false,true}){
   CWeaponBlaster w;w.setup(p,capacity,capacity,show);infinitePlayers=cheat?1u<<p:0;
   w.m_pvFireBuddyPos_WS=&buddy;
   unsigned rounds=cheat?64:capacity;
   if(cheat && capacity>1) w.m_nClipAmmo=capacity-1;
   unsigned startingAmmo=w.GetClipAmmo();
   int prevShakes=cameras[p].shakes,prevRumbles=rumbleCount[p];
   for(unsigned shot=0;shot<rounds;shot++){
    w.m_bBoolBarrel2Fire=(shot&1)!=0;
    if(variant==0)w._L1_Fire(direction);else w._L23_Fire(direction);
    require(w.GetClipAmmo()==(cheat?startingAmmo:capacity-shot-1),"correct ammo per shot");
   }
   require(w.blasts==int(rounds*2)&&w.muzzles==int(rounds*2)&&w.sounds==int(rounds),"pellets, buddy and muzzle/sound effects still run");
   require(cameras[p].shakes-prevShakes==int(rounds)&&rumbleCount[p]-prevRumbles==int(rounds),"own camera and controller feedback still run");
   require(w.resources.m_paShellWorldMeshArray.accesses==int((cheat||!show)?0:rounds),"only consumed visible shells accessed");
   for(auto&shell:w.resources.m_paShellWorldMeshArray.shells)require(shell.removals==((cheat||!show)?0:1),"finite shells removed once, infinite shells remain");
   if(!cheat){int oldAccesses=w.resources.m_paShellWorldMeshArray.accesses;w._ConsumeFiredRound();
    require(w.GetClipAmmo()==0&&w.resources.m_paShellWorldMeshArray.accesses==oldAccesses,"empty clip does not remove an unfired shell");}
  }
 }
 infinitePlayers=1;CWeaponBlaster full;full.setup(0,6,6,true);
 for(int i=0;i<100;i++)full._L1_Fire(direction);
 require(full.GetClipAmmo()==6&&!full.resources.m_paShellWorldMeshArray.accesses,"full retained clip never indexes capacity");
 infinitePlayers=1;CWeaponBlaster p2;p2.setup(1,6,6,true);p2._L1_Fire(direction);
 require(p2.GetClipAmmo()==5&&p2.resources.m_paShellWorldMeshArray.shells[5].removals==1,"P1 cheat does not retain P2 ammo or visuals");
 infinitePlayers=0;CWeaponBlaster unlimited;unlimited.setup(0,6,INFINITE_AMMO,true);
 for(int i=0;i<100;i++)unlimited._L1_Fire(direction);
 require(unlimited.GetClipAmmo()==INFINITE_AMMO&&!unlimited.resources.m_paShellWorldMeshArray.accesses,"engine infinite-ammo sentinel not used as shell index");
 std::printf("PASS: %d offline Scatter Blaster ammo/fire checks; game not launched.\n",checks);
}
'''

def main():
    blaster = (ROOT / "ma/App/ma/weapon_blaster.cpp").read_text()
    weapon = (ROOT / "ma/App/ma/weapon.cpp").read_text()
    code = FIXTURE + method(weapon, "u16 CWeapon::RemoveFromClip(")
    for signature in ("void CWeaponBlaster::_ConsumeFiredRound(", "void CWeaponBlaster::_L1_Fire(",
                      "void CWeaponBlaster::_L23_Fire("):
        code += method(blaster, signature) + "\n"
    code += '''void CWeaponBlaster::originalShellConsumption(){
 RemoveFromClip(1);
 if(m_pResourceData->m_paShellWorldMeshArray){m_pResourceData->m_paShellWorldMeshArray[GetClipAmmo()].RemoveFromWorld();}
}\n'''
    OUT.mkdir(parents=True, exist_ok=True)
    (OUT / "blaster.cpp").write_text(code + CHECKS)
    (OUT / "CMakeLists.txt").write_text(
        "cmake_minimum_required(VERSION 3.20)\nproject(blaster_ammo LANGUAGES CXX)\n"
        "add_executable(blaster_ammo blaster.cpp)\ntarget_compile_features(blaster_ammo PRIVATE cxx_std_17)\n")
    for command in (["cmake", "-S", str(OUT), "-B", str(OUT / "out"), "-A", "Win32"],
                    ["cmake", "--build", str(OUT / "out"), "--config", "Release"]):
        run = subprocess.run(command, capture_output=True, text=True)
        if run.returncode:
            raise SystemExit(run.stdout + run.stderr)
    subprocess.run([str(OUT / "out/Release/blaster_ammo.exe")], check=True)

if __name__ == "__main__":
    main()
