"""Verify reticle-to-ray projection and preserved Slosh, Glitch and NPC aim clamps."""
from pathlib import Path
import subprocess
from test_collectable_skiplist import method

ROOT=Path(__file__).resolve().parent.parent
OUT=ROOT/'build/test-slosh-aim'
BASE=r'''
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <initializer_list>
using f32=float;using BOOL=bool;
constexpr bool FALSE=false;
constexpr int ENTITY_BIT_BOTGLITCH=1,ENTITY_BIT_BOTCHEMBOT=2,ENTITY_BIT_BOTGRUNT=4;
int checks=0;void check(bool value,const char* why){++checks;if(!value){std::fprintf(stderr,"FAIL: %s\n",why);std::exit(1);}}
float fmath_InvSqrt(float x){return 1/std::sqrt(x);}
struct CFVec3A{
 float x=0,y=0,z=0;static CFVec3A m_UnitAxisX;
 float MagSq()const{return x*x+y*y+z*z;}float Dot(const CFVec3A& v)const{return x*v.x+y*v.y+z*v.z;}
 CFVec3A& Sub(const CFVec3A& a,const CFVec3A& b){x=a.x-b.x;y=a.y-b.y;z=a.z-b.z;return *this;}
 CFVec3A& Mul(float s){x*=s;y*=s;z*=s;return *this;}CFVec3A& Mul(const CFVec3A& a,float s){*this=a;return Mul(s);}
 CFVec3A& Add(const CFVec3A& v){x+=v.x;y+=v.y;z+=v.z;return *this;}
 void Lerp(float t,const CFVec3A& b){x+=(b.x-x)*t;y+=(b.y-y)*t;z+=(b.z-z)*t;}
 void Unitize(){Mul(fmath_InvSqrt(MagSq()));}
};CFVec3A CFVec3A::m_UnitAxisX{1,0,0};
struct CBot{int bits=0,player=-1;int TypeBits()const{return bits;}};
struct CWeapon{CBot* owner=nullptr;CBot* GetOwner(){return owner;}bool IsOwnedByPlayer(){return owner&&owner->player>=0;}
 void ComputeFireUnitDir(CFVec3A*,const CFVec3A*,const CFVec3A*,const CFVec3A*,BOOL);};
#define _AIMCLAMP_OUTSIDE_ANGLE_COS .7660444431189780352f
#define _AIMCLAMP_INSIDE_ANGLE_COS .8660254037844386468f
#define _AIMCLAMP_OO_RANGE (1.f/(_AIMCLAMP_INSIDE_ANGLE_COS-_AIMCLAMP_OUTSIDE_ANGLE_COS))
'''

PROJECTION=r'''
struct Matrix{CFVec3A m_vRight{1,0,0},m_vUp{0,1,0},m_vFront{0,0,1};};
struct Xfm{Matrix m_MtxR;};
struct Camera{Xfm xfm;Xfm* GetFinalXfm(){return &xfm;}};
struct Viewport{float fAspectHOW,fTanHalfFOVX,fTanHalfFOVY;};
struct Reticle{float x,y;float GetNormOriginX(){return x;}float GetNormOriginY(){return y;}};
CFVec3A productionRay(Camera* pCamera,Viewport* pViewport,Reticle* pReticle,float fMaxLiveRange){
 float fReticleX,fReticleY,fEndPointReticleAdjust;
 CFVec3A ReticleAdjustX,ReticleAdjustY,RayEndPoint,RayStartPoint{3,-7,11};
 PRODUCTION_RAY
 return RayEndPoint;
}
void projectionChecks(){
 for(float width:{640.f,960.f,1280.f,1920.f,3440.f})
 for(float height:{360.f,540.f,720.f,1080.f,1440.f})
 for(float fov:{.35f,.6f,.9f})for(float x:{-.4f,0.f,.18f,.4f})for(float y:{-.5f,0.f,.23f,.5f}){
  Viewport vp{height/width,std::tan(fov)*width/height,std::tan(fov)};
  Reticle reticle{x,y};Camera camera;
  auto target=productionRay(&camera,&vp,&reticle,60);
  const float pixelX=(target.x-3)/(target.z-11)/vp.fTanHalfFOVX*width*.5f;
  const float pixelY=(target.y+7)/(target.z-11)/vp.fTanHalfFOVY*height*.5f;
#if FANG_WINGC
  check(std::fabs(pixelX-x*height*.5f)<.001f,"Slosh offset ray projects to displayed X at all viewport shapes");
#else
  check(std::fabs(pixelX-x*width*.5f)<.001f,"legacy horizontal ray behavior is preserved");
#endif
  check(std::fabs(pixelY-y*height*.5f)<.001f,"vertical aim keeps matching reticle rendering");
 }
}
'''

CHECKS=r'''
int main(){
 projectionChecks();
 const float pi=3.14159265358979f;CFVec3A muzzle{5,7,-11},front{0,0,1},actual,wanted,delta;
 CBot owner;CWeapon weapon;weapon.owner=&owner;
 for(int bits:{ENTITY_BIT_BOTGLITCH,ENTITY_BIT_BOTCHEMBOT,ENTITY_BIT_BOTGRUNT})
 for(int player:{-1,0,1,2,3})for(int yaw:{-160,-90,-45,-35,-15,0,15,35,45,90,160})for(int pitch:{-75,-45,0,45,75}){
  owner.bits=bits;owner.player=player;
  const float y=yaw*pi/180,p=pitch*pi/180;
  wanted={std::sin(y)*std::cos(p),std::sin(p),std::cos(y)*std::cos(p)};
  CFVec3A target{muzzle.x+50*wanted.x,muzzle.y+50*wanted.y,muzzle.z+50*wanted.z};
  weapon.ComputeFireUnitDir(&actual,&muzzle,&front,&target,false);
  const bool direct=bits==ENTITY_BIT_BOTGLITCH;
  if(direct){delta.Sub(actual,wanted);check(delta.MagSq()<1e-8f,"Glitch retains direct target aim");}
  else if(wanted.z<=_AIMCLAMP_OUTSIDE_ANGLE_COS){delta.Sub(actual,front);check(delta.MagSq()<1e-8f,"Slosh, NPCs and other owners retain authored muzzle clamp");}
  check(std::fabs(actual.MagSq()-1)<1e-5f,"aim stays normalized");
 }
 owner.bits=ENTITY_BIT_BOTCHEMBOT;owner.player=0;
 weapon.ComputeFireUnitDir(&actual,&muzzle,&front,&muzzle,true);
 check(actual.z==-1,"coincident target retains negated muzzle fallback");
 CFVec3A zero{};weapon.ComputeFireUnitDir(&actual,&muzzle,&zero,&muzzle,false);
 check(actual.x==1&&actual.y==0&&actual.z==0,"zero muzzle direction retains safe fallback");
 std::printf("PASS: %d production Slosh aim checks (PC=%d); no game launched.\n",checks,FANG_WINGC);
}
'''

def main():
    bot=(ROOT/'ma/App/ma/bot.cpp').read_text()
    ray=bot[bot.index('fReticleX = pReticle->GetNormOriginX();',bot.index('void CBot::ComputeHumanTargetPoint_WS(')): ]
    ray=ray[:ray.index('\n',ray.index('RayEndPoint.Mul('))]
    code=BASE+PROJECTION.replace('PRODUCTION_RAY',ray)+method((ROOT/'ma/App/ma/weapon.cpp').read_text(),'void CWeapon::ComputeFireUnitDir(')+CHECKS
    OUT.mkdir(parents=True,exist_ok=True)
    (OUT/'aim.cpp').write_text(code)
    cmake='cmake_minimum_required(VERSION 3.20)\nproject(slosh_aim LANGUAGES CXX)\n'
    for name,value in [('pc',1),('legacy',0)]:
        cmake+=f'add_executable({name} aim.cpp)\ntarget_compile_features({name} PRIVATE cxx_std_17)\ntarget_compile_definitions({name} PRIVATE FANG_WINGC={value})\n'
    (OUT/'CMakeLists.txt').write_text(cmake)
    for args in [['cmake','-S',str(OUT),'-B',str(OUT/'out'),'-A','Win32'],['cmake','--build',str(OUT/'out'),'--config','Release']]:
        run=subprocess.run(args,capture_output=True,text=True)
        if run.returncode:raise SystemExit(run.stdout+run.stderr)
    for name in ['pc','legacy']:subprocess.run([str(OUT/'out/Release'/(name+'.exe'))],check=True)

if __name__=='__main__':main()
