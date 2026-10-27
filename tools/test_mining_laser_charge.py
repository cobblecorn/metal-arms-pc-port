"""Exercise production laser input, cancellation, charge bands and shot snapshots offline."""
from pathlib import Path
import sys
sys.path.insert(0, str(Path(__file__).resolve().parent))
import subprocess
from gamedata_dump import decode
from test_coop_checkpoint_rat import method
ROOT = Path(__file__).resolve().parent.parent
OUT = ROOT / "build/test-mining-laser-charge"
BASE = r'''
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <initializer_list>
#include <algorithm>
#include "pc_laser_charge.h"
using f32=float;using u16=unsigned short;using u32=unsigned;using BOOL=int;
constexpr int TRUE=1,FALSE=0;
#define FANG_WINGC 1
#define FMATH_MAX(a,b) std::max(a,b)
int checks=0;
void check(bool x,const char* why){++checks;if(!x){std::fprintf(stderr,"FAIL: %s\n",why);std::exit(1);}}
#define FASSERT(x) check(!!(x),"production assertion")
float FLoop_fPreviousLoopSecs=1.f/60;
struct CFVec3A{float x=0,y=0,z=0;void Set(const CFVec3A& v){*this=v;}};
struct CFColorRGBA{float r=0,g=0,b=0,a=0;void Set(float R,float G,float B,float A){r=R;g=G;b=B;a=A;}};
struct CDamageProfile{int identity=0;};
struct CFAudioEmitter{int destroys=0;void Destroy(){++destroys;}};
struct CBot{int m_nPossessionPlayerIndex=0;bool dead=false;int focus=0;
 bool IsDeadOrDying(){return dead;}void FocusHumanTargetPoint_WS(CFVec3A*,void*){++focus;}};
struct TracerDef_t;
using TracerCallback=void(*)(TracerDef_t*,int,const void*);
constexpr unsigned TRACERFLAG_THICK_PROJECTILE=8,TRACERFLAG_DEFER_FIRST_WORK=16;
struct TracerDef_t{unsigned uFlags=0;TracerCallback pFcnKillCallback=nullptr;void(*pFcnBuildSkipList)(void*)=nullptr;u16 nUser=0;float fSpeed_WS=400,fLength_WS=50,fWidth_WS=1,fMaxTailDist_WS=1000;CFColorRGBA ColorRGBA;};
static const u16 _CHARGED_TRACER=0x8000;
bool _UsesL3Beam(u32){return false;}
struct CWeaponLaser{
 static constexpr unsigned EUK_COUNT_LASER=5;enum{STATE_DEPLOYED,STATE_STOWED};
 struct _RetailCharge_t{float fDelay=.75f,fChargeSeconds=3,fReadyEnergy=.5f,fEnergyAfterBurst=.25f;
 float fSpeed=800,fLength=200,fWidth=4,fRange=100,afColor[3]{1,1,0};CDamageProfile* apDamage[3]{};};
 static _RetailCharge_t m_aRetailCharge[5];
 struct Info{u16 nClipAmmoMax=100;}info,*m_pInfo=&info;
 CBot owner;CBot* m_pOwnerBot=&owner;u32 m_nUpgradeLevel=1;u16 m_nClipAmmo=100;
 int state=STATE_DEPLOYED;bool player=true,mouse=true;PCLaserCharge m_PlayerCharge;
 CFAudioEmitter* m_pChargeSoundEmitter=nullptr;
 BOOL m_bSecondaryTapThisFrame=0,m_bChargedShotThisFrame=0,m_bChargeReadySoundPlayed=0,m_bFireThisFrame=0,m_bRechargeThisFrame=0,m_bFiredLastFrame=0;
 u32 m_nChargedShotTier=0;float m_fSecondsCountdownTimer=0;CFVec3A m_vTargetPos_WS;
 const CFVec3A* m_pvecBuddyFirePos_WS=nullptr;
 bool IsCreated(){return true;}int CurrentState()const{return state;}bool IsOwnedByPlayer()const{return player;}
 CBot* GetOwner()const{return m_pOwnerBot;}
 bool UsesMouseChargeControls()const{return mouse;}
 void _StopChargeSound();void CancelPlayerCharge();BOOL SupportsPlayerCharge()const;u16 _PlayerChargeCost()const;bool PlayerSecondaryTap()const{return m_bSecondaryTapThisFrame;}
 u32 _PlayerChargeTrigger(f32,const CFVec3A*,const CFVec3A*);
 u32 TriggerWork(f32,f32,const CFVec3A*,const CFVec3A* = nullptr);
 void _ConfigureChargedTracer(TracerDef_t*)const;static CDamageProfile* _ChargedDamageProfile(u16);
 static void _ChargedTracerCallback(TracerDef_t*,int,const void*){}static void _ChargedSkipList(void*){}
 void _StopChargeParticle(){}
};
CWeaponLaser::_RetailCharge_t CWeaponLaser::m_aRetailCharge[5];
'''
CHECKS = (ROOT / 'tools/fixtures/mining_laser_controls.cpp.in').read_text()
def main():
    for table in decode((ROOT/"gamedata/mst/w_laser.csv").read_bytes())["tables"]:
        v=[f["value"] for f in table["fields"]]
        assert v[33:39]==[.75,3.,1.,2.,.5,.25]
        assert all(n!="None" for n in v[60:63])==(table["name"] in ("laserl2","laserl3","lasermil_possessed"))
    source=(ROOT/"ma/App/ma/weapon_laser.cpp").read_text(encoding="utf-8")
    signatures=("void CWeaponLaser::_StopChargeSound(","void CWeaponLaser::CancelPlayerCharge(",
      "BOOL CWeaponLaser::SupportsPlayerCharge(","u16 CWeaponLaser::_PlayerChargeCost(","u32 CWeaponLaser::_PlayerChargeTrigger(",
      "void CWeaponLaser::_ConfigureChargedTracer(","CDamageProfile *CWeaponLaser::_ChargedDamageProfile(",
      "u32 CWeaponLaser::TriggerWork(")
    code=BASE+"\n".join(method(source,s) for s in signatures)+CHECKS
    OUT.mkdir(parents=True,exist_ok=True)
    (OUT/"laser_charge.cpp").write_text(code,encoding="utf-8")
    (OUT/"CMakeLists.txt").write_text('cmake_minimum_required(VERSION 3.20)\nproject(laser_charge LANGUAGES CXX)\nadd_executable(laser_charge laser_charge.cpp)\ntarget_compile_features(laser_charge PRIVATE cxx_std_17)\ntarget_include_directories(laser_charge PRIVATE "'+(ROOT/"port").as_posix()+'")\n',encoding="utf-8")
    for command in (["cmake","-S",str(OUT),"-B",str(OUT/"out"),"-A","Win32"],["cmake","--build",str(OUT/"out"),"--config","Release"]):
        run=subprocess.run(command,capture_output=True,text=True)
        if run.returncode:raise SystemExit(run.stdout+run.stderr)
    subprocess.run([str(OUT/"out/Release/laser_charge.exe")],check=True)
if __name__=="__main__":main()
