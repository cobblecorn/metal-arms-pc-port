"""Exercise production vehicle ground-probe and tank wheel math without launching the game."""
from pathlib import Path
import subprocess

from test_collectable_skiplist import method

ROOT = Path(__file__).resolve().parent.parent
OUT = ROOT / 'build/test-vehicle-math'

FIXTURE = r'''
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <limits>
#include <initializer_list>
using f32=float; using u32=unsigned;
#define FMATH_CLAMPMAX(x,y) if((x)>(y)) (x)=(y)
constexpr float _FIND_GROUND_RAY_LENGTH=50, _GROUND_NORMAL_SLERP_RATE=3;
float FLoop_fPreviousLoopSecs=1.f/60;
int checks=0;
void check(bool b,const char* why){++checks;if(!b){std::fprintf(stderr,"FAIL: %s\n",why);std::exit(1);}}
struct CFVec3A {
 // Poison unset components to expose use of the engine's uninitialized default vector.
 float x=std::numeric_limits<float>::quiet_NaN(),y=x,z=x;
 CFVec3A()=default; CFVec3A(float a,float b,float c):x(a),y(b),z(c){}
 static const CFVec3A m_UnitAxisY;
 void Set(const CFVec3A& v){*this=v;}
 void Sub(const CFVec3A& v){x-=v.x;y-=v.y;z-=v.z;}
 float Dot(const CFVec3A& v)const{return x*v.x+y*v.y+z*v.z;}
 float Mag()const{return std::sqrt(Dot(*this));}
};
const CFVec3A CFVec3A::m_UnitAxisY{0,1,0};
struct CFQuatA {
 CFVec3A normal{0,1,0};
 void BuildQuat(const CFVec3A&,const CFVec3A& target){
  check(std::isfinite(target.x)&&std::isfinite(target.y)&&std::isfinite(target.z),"ground normal must be initialized for every collision result");
  normal=target;
 }
 void ReceiveSlerpOf(float t,const CFQuatA&,const CFQuatA& target){check(t>=0&&t<=1,"interpolation stays bounded");normal=target.normal;}
 void Set(const CFQuatA& q){*this=q;}
 void MulPoint(CFVec3A& out,const CFVec3A&)const{out=normal;}
};
struct CFWorldTracker{};
CFWorldTracker* FWorld_apTrackerSkipList[2]{};
u32 FWorld_nTrackerSkipListCount=0,FColl_nImpactCount=0;
struct FCollImpact_t{CFVec3A UnitFaceNormal,ImpactPoint;};
bool hit=false; FCollImpact_t planned;
void fcoll_Clear(){FColl_nImpactCount=0;}
void fworld_FindClosestImpactPointToRayStart(FCollImpact_t* out,const CFVec3A* start,const CFVec3A* end,u32,CFWorldTracker**){
 check(end->y==start->y-50,"probe keeps authored length and direction");
 FColl_nImpactCount=hit?1:0; if(hit)*out=planned;
}
struct CVehicle {
 struct Physics {CFVec3A pos{0,30,0}; const CFVec3A* GetPosition(){return &pos;}}m_PhysObj;
 float m_fHeightAboveGround=0,m_fMaxHeightAboveGround=100;
 CFVec3A m_vGroundNormal; CFQuatA m_qGroundNormal;
 bool airborne=false,contact=true;
 void AppendTrackerSkipList(){}
 bool BecameAirborne(){return airborne;}
 bool SmoothContact(){return contact;}
 void ComputeGroundNormal();
};
constexpr float FMATH_PI=3.14159265358979323846f;
#define FMATH_DEG2RAD(x) ((x)*(FMATH_PI/180.f))
constexpr unsigned SENTINEL_FLAG_DONT_CALL_BOTPARTMANAGER=1;
const float _afOOWheelRadius[6]={.25f,.25f,.25f,.25f,.25f,.25f};
struct CFMtx43A {
 CFVec3A m_vPos{0,0,0},m_vFront{0,0,1};float angle=0;
 void Mul(const CFMtx43A& a,const CFMtx43A&){*this=a;}
 void Mul(const CFMtx43A& a){angle=a.angle;}
 void Identity(){angle=0;}
 void SetRotationX(float value){angle=value;}
};
struct CVehicleSentinel {
 unsigned m_uSentinelFlags=0;float m_afWheelAngle[6]{};
 struct Physics{
  CFVec3A pos{0,0,0},velocity{0,0,0};
  const CFVec3A* GetPosition(){return &pos;}
  void GetVelocity(const CFVec3A*,CFVec3A* out){*out=velocity;}
 }m_PhysObj;
 void AnimateWheels(u32,u32,CFMtx43A&,const CFMtx43A&,const CFMtx43A&);
};
'''

CHECKS = r'''
int main(){
 CVehicle vehicle;
 for(float dt: {1.f/144,1.f/60,1.f/30,2.f}){
  FLoop_fPreviousLoopSecs=dt;
  for(CFVec3A normal: {CFVec3A{0,1,0},CFVec3A{.6f,.8f,0},CFVec3A{1,0,0},CFVec3A{0,-1,0}}){
   hit=true;planned={normal,{0,20,0}};vehicle.ComputeGroundNormal();
   const auto expected=normal.y>0?normal:CFVec3A::m_UnitAxisY;
   check(vehicle.m_vGroundNormal.x==expected.x&&vehicle.m_vGroundNormal.y==expected.y&&vehicle.m_vGroundNormal.z==expected.z,"floor slopes preserved, non-floor hits use upright fallback");
   check(vehicle.m_fHeightAboveGround==(normal.y>0?10:50),"wall and ceiling hits do not supply floor height");
  }
  hit=false;vehicle.ComputeGroundNormal();check(vehicle.m_vGroundNormal.y==1&&vehicle.m_fHeightAboveGround==50,"ray miss uses upright fallback");
 }
 vehicle.airborne=true;vehicle.contact=false;hit=true;planned={{1,0,0},{0,20,0}};
 vehicle.ComputeGroundNormal();check(vehicle.m_fMaxHeightAboveGround==50,"airborne height accounting remains intact");
 CVehicleSentinel tank;CFMtx43A wheel,parent,bone;
 FLoop_fPreviousLoopSecs=1.f/60;
 for(float speed: {0.f,50.f,-50.f,200.f,-200.f,1.e30f,-1.e30f,std::numeric_limits<float>::max(),std::numeric_limits<float>::infinity(),-std::numeric_limits<float>::infinity(),std::numeric_limits<float>::quiet_NaN()}){
  tank.m_afWheelAngle[0]=1.f;tank.m_PhysObj.velocity={0,0,speed};
  tank.AnimateWheels(0,0,wheel,parent,bone);
  check(std::isfinite(wheel.angle)&&wheel.angle>=0&&wheel.angle<=FMATH_DEG2RAD(360.f),"wheel transform finite and bounded even for corrupt velocity");
  if(std::isfinite(speed)&&std::fabs(speed)<1.e3f){
   float expected=std::fmod(1.f+speed*.25f*FLoop_fPreviousLoopSecs,FMATH_DEG2RAD(360.f));if(expected<0)expected+=FMATH_DEG2RAD(360.f);
   check(std::fabs(wheel.angle-expected)<1.e-5f,"ordinary wheel rolling preserved");
  }
 }
 tank.m_afWheelAngle[0]=std::numeric_limits<float>::quiet_NaN();tank.m_PhysObj.velocity={0,0,0};tank.AnimateWheels(0,0,wheel,parent,bone);
 check(wheel.angle==0,"corrupt stored wheel angle recovers");
 tank.m_afWheelAngle[0]=2;tank.m_uSentinelFlags=SENTINEL_FLAG_DONT_CALL_BOTPARTMANAGER;tank.AnimateWheels(0,0,wheel,parent,bone);
 check(tank.m_afWheelAngle[0]==2,"collision palette computation never advances wheel animation");
 std::printf("PASS: %d production vehicle ground-probe/wheel-math checks; no game launched.\n",checks);
}
'''


def main():
    OUT.mkdir(parents=True, exist_ok=True)
    source = (ROOT / 'ma/App/ma/vehicle.cpp').read_text()
    sentinel = (ROOT / 'ma/App/ma/vehiclesentinel.cpp').read_text()
    (OUT / 'ground_normal.cpp').write_text(FIXTURE + method(source, 'void CVehicle::ComputeGroundNormal(')
        + method(sentinel, 'void CVehicleSentinel::AnimateWheels(') + CHECKS)
    (OUT / 'CMakeLists.txt').write_text(
        'cmake_minimum_required(VERSION 3.20)\nproject(vehicle_ground_normal CXX)\n'
        'add_executable(ground_normal ground_normal.cpp)\n'
        'target_compile_features(ground_normal PRIVATE cxx_std_14)\n'
        'target_compile_definitions(ground_normal PRIVATE FANG_WINGC=1)\n')
    for cmd in (['cmake', '-S', str(OUT), '-B', str(OUT / 'out'), '-A', 'Win32'],
                ['cmake', '--build', str(OUT / 'out'), '--config', 'Release']):
        result = subprocess.run(cmd, capture_output=True, text=True)
        if result.returncode:
            raise SystemExit(result.stdout + result.stderr)
    subprocess.run([str(OUT / 'out/Release/ground_normal.exe')], check=True, timeout=5)


if __name__ == '__main__':
    main()
