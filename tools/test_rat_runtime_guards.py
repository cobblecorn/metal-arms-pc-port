"""Offline production RAT camera/model, positional gain and sphere sweep checks."""
from pathlib import Path
import subprocess

ROOT = Path(__file__).resolve().parent.parent
OUT = ROOT / "build/test-rat-runtime-guards"


def block(source, signature):
    start = source.index(signature)
    brace = source.index("{", start)
    depth, end = 1, brace + 1
    while depth:
        depth += (source[end] == "{") - (source[end] == "}")
        end += 1
    return source[start:end]


PRELUDE = r'''
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <algorithm>
#include <random>
using f32=float;using u32=unsigned;using s32=int;using BOOL=int;
constexpr int TRUE=1,FALSE=0;
#define FANG_WINGC 1
#define MA_PC_INPUT 1
#define FMATH_CLAMP(v,a,b) v=std::clamp(v,a,b)
#define FMATH_CLAMPMIN(v,a) v=std::max(v,a)
#define FMATH_CLAMP_UNIT_FLOAT(v) v=std::clamp(v,0.f,1.f)
#define FMATH_CLAMP_MAX1(v) v=std::min(v,1.f)
#define FMATH_CLAMP_MIN0(v) v=std::max(v,0.f)
#define FMATH_FPOT(t,a,b) ((a)+(t)*((b)-(a)))
#define FASSERT(x) do{if(!(x))std::abort();}while(0)
#define DEVPRINTF(...) (void)0
int checks=0;
void require(bool b,const char* why){checks++;if(!b){std::fprintf(stderr,"FAIL: %s\n",why);std::exit(1);}}
float fmath_Abs(float v){return std::fabs(v);}float fmath_Sqrt(float v){return std::sqrt(v);}
float fmath_Div(float a,float b){return a/b;}
struct CFVec3A{
 float x=0,y=0,z=0;
 CFVec3A()=default;CFVec3A(float a,float b,float c):x(a),y(b),z(c){}
 void Set(float a,float b,float c){x=a;y=b;z=c;}
 CFVec3A& Sub(const CFVec3A& a,const CFVec3A& b){x=a.x-b.x;y=a.y-b.y;z=a.z-b.z;return *this;}
 CFVec3A& Sub(const CFVec3A& a){x-=a.x;y-=a.y;z-=a.z;return *this;}
 CFVec3A& Add(const CFVec3A& a){x+=a.x;y+=a.y;z+=a.z;return *this;}
 CFVec3A& Mul(const CFVec3A& a,float n){x=a.x*n;y=a.y*n;z=a.z*n;return *this;}
 CFVec3A& Mul(float n){x*=n;y*=n;z*=n;return *this;}
 float Dot(const CFVec3A& a)const{return x*a.x+y*a.y+z*a.z;}
 float MagSq()const{return Dot(*this);}float Mag()const{return std::sqrt(MagSq());}
};
struct Xfm{struct{CFVec3A m_vPos;}m_MtxF;};
struct Listener{Xfm* poXfmCurrentOrientation_WS;};
Listener _aoVirtualListeners[4];Xfm transforms[4];u32 _uActiveVirtualListeners=1;
constexpr u32 _EMITTER_PROPERTIES_3D=1,_EMITTER_STATE_CHANGE_VOLUME=2;
constexpr float _SIGNIFICANT_VOLUME_CHANGE_SQ=.000001f,_GC_3D_RADIUS_SCALE=1.25f,_GC_3D_VOLUME_SCALE=.8f;
struct _VirtualEmitter_t{u32 uProperties=1,uStateChanges=0;float fDistanceGain=-1,fRadiusOuter=100;
 CFVec3A* poVecCurrentPosition_WS;};
struct Bot{bool human=false;BOOL IsPlayerBot(){return human;}};
struct Mesh{bool visible=false;void DrawEnable(BOOL b){visible=b;}};
struct CVehicle{void ClassHierarchyDrawEnable(BOOL){}};
struct Multi{bool coop=true;BOOL IsLocalCoop(){return coop;}}MultiplayerMgr;
struct Camera{float height=0,elevation=0;void SetHeightFromXZPlane(float h){height=h;}
 void SetElevationAngle(float e){elevation=e;}};
struct Transition{int player=0;int GetPlayerIndex(){return player;}};
struct Gun{float pitch=.3f;float GetGunPitch(){return pitch;}};
struct Player{u32 m_nControllerIndex=0;}Player_aPlayer[4];
constexpr int GAMEPAD_MAIN_LOOK_UP_DOWN=0,RAT_FLAG_ZOBBY_GUNNING=1;
constexpr float _GUNNERCAM_ELEVATION_ANGLE=.1f;
struct Sample{float fCurrentState=0;};Sample samples[4];Sample* Gamepad_aapSample[4][1];
bool mouse=false;bool pcinput_IsMouseAiming(u32){return mouse;}
struct CVehicleRat:CVehicle{Camera m_GunnerCamera;Transition m_GunnerCameraTrans;Gun m_RatGun;
 bool driveable=true;BOOL IsDriveable(){return driveable;}
 u32 m_uRatFlags=1;Mesh* m_pZobbyME=nullptr;Bot* m_pGunnerBot=nullptr;
 void ClassHierarchyDrawEnable(BOOL);void TestCamera();};
struct Sound{bool spatial=true;float volume=1,pitch=1;CFVec3A pos;
 bool Is3D(){return spatial;}void SetVolume(float v){volume=v;}void SetFrequencyFactor(float f){pitch=f;}
 void SetPosition(const CFVec3A* p){pos=*p;}};
constexpr float _MIN_HOVER_PITCH=1,_MAX_HOVER_PITCH=1.1f,_MIN_HOVER_VOLUME=.5f,_MAX_HOVER_VOLUME=1;
constexpr float _MAX_VELOCITY_XZ=40,_MIN_WIND_PITCH=1,_MAX_WIND_PITCH=1.2f,_MIN_WIND_VOLUME=0,_MAX_WIND_VOLUME=1,_WIND_RAMP_UP_RATE=5;
float FLoop_fPreviousLoopSecs=.016f;
struct CBotPred{Sound* m_pHoverAudioEmitter=nullptr;Sound* m_pWindAudioEmitter=nullptr;
 struct{int pSoundGroupHover=0,pSoundGroupWind=1;}m_BotInfo_Pred;
 Sound sound[2];CFVec3A m_XlatStickUnitVecXZ_MS,m_MountPos_WS{700,100,500};
 float m_fControls_FlyUp=0,m_fSpeed_WS=0,m_fUnitWindLevel=0;
 bool fail=false;BOOL IsDeadOrDying(){return FALSE;}
 Sound* AllocAndPlaySound(int n){return fail?nullptr:&sound[n];}void _SoundWork();};
'''

CHECKS = r'''
int main(){
 for(int p=0;p<4;p++){transforms[p].m_MtxF.m_vPos.Set(1000+p*100,0,0);
  _aoVirtualListeners[p].poXfmCurrentOrientation_WS=&transforms[p];
  Player_aPlayer[p].m_nControllerIndex=p;Gamepad_aapSample[p][0]=&samples[p];}
 for(u32 players=1;players<=4;players++){
  _uActiveVirtualListeners=players;CFVec3A pos{0,0,0};_VirtualEmitter_t e;e.poVecCurrentPosition_WS=&pos;
  _Update3DDistanceGain(&e);require(e.fDistanceGain==0,"far startup loop is silent without a position-change event");
  require(e.uStateChanges==_EMITTER_STATE_CHANGE_VOLUME,"first gain marks volume change");
  pos.x=1000+(players-1)*100;e.uStateChanges=0;_Update3DDistanceGain(&e);
  require(std::fabs(e.fDistanceGain-.8f)<.00001f,"nearest co-op listener hears local emitter");
  pos.x+=60;e.uStateChanges=0;_Update3DDistanceGain(&e);
  require(e.fDistanceGain>0&&e.fDistanceGain<.8f,"stationary listener uses changed source distance");
  e.fRadiusOuter=1;e.uStateChanges=0;_Update3DDistanceGain(&e);
  require(e.fDistanceGain==0&&e.uStateChanges,"radius change cannot retain near/full gain");
 }
 _uActiveVirtualListeners=0;CFVec3A pos{};_VirtualEmitter_t e;e.poVecCurrentPosition_WS=&pos;e.fDistanceGain=.8f;
 _Update3DDistanceGain(&e);require(e.fDistanceGain==0,"missing listener never makes a 3D loop globally audible");
 e.uProperties=0;e.fDistanceGain=-1;e.uStateChanges=0;_Update3DDistanceGain(&e);
 require(e.fDistanceGain==-1&&e.uStateChanges==0,"2D sounds retain existing mixing");
 for(int p=0;p<4;p++)for(float axis:{-1.f,-.4f,0.f,.4f,1.f}){
  CVehicleRat rat;rat.m_GunnerCameraTrans.player=p;samples[p].fCurrentState=axis;mouse=false;
  rat.TestCamera();require(std::fabs(rat.m_GunnerCamera.elevation-.4f)<.00001f,"controller camera follows actual gun pitch independently of stick height");
  mouse=true;rat.TestCamera();require(std::fabs(rat.m_GunnerCamera.elevation-.4f)<.00001f,"mouse camera follows gun pitch");
 }
 Mesh z;Bot gunner;CVehicleRat rat;rat.m_pZobbyME=&z;rat.m_pGunnerBot=&gunner;
 for(bool coop:{false,true})for(bool human:{false,true})for(bool show:{false,true}){
  MultiplayerMgr.coop=coop;gunner.human=human;rat.ClassHierarchyDrawEnable(show);
  require(z.visible==(show&&!(coop&&human)),"vehicle redraw keeps decorative NPC hidden for human co-op gunner");
 }
 CBotPred jet;jet._SoundWork();require(jet.sound[1].volume==0,"new stationary jet wind loop starts at intended zero volume");
 require(jet.sound[0].volume==.5f&&jet.sound[0].pos.x==700,"new hover loop gets gain/position in allocation frame");
 jet.fail=true;jet.m_pHoverAudioEmitter=jet.m_pWindAudioEmitter=nullptr;jet._SoundWork();
 require(!jet.m_pHoverAudioEmitter&&!jet.m_pWindAudioEmitter,"missing jet voices fail safely");
 double t;require(_SphereSweepEntryTime(1,-4,3,1.01,t)&&std::fabs(t-1)<1e-10,"entry root selected");
 require(!_SphereSweepEntryTime(1,4,3,1,t),"moving-away sphere has no entry");
 require(!_SphereSweepEntryTime(0,0,1,1,t),"stationary sweep has no division by zero");
 CFVec3A a{0,0,0},b{10000,0,0},c{5000,1,0},v{10,-.75f,0},impact;float hit=1;
 _ProjectSphereAgainstEdge(c,.5f,v,a,b,hit,impact);
 require(std::fabs(hit-2.f/3.f)<.00001f,"near-parallel long-edge sweep retains the real contact");
 c.Set(0,0,0);a.Set(2,0,0);b=a;v.Set(4,0,0);hit=1;
 _ProjectSphereAgainstEdge(c,1,v,a,b,hit,impact);require(std::fabs(hit-.25f)<.00001f,"zero-length edge tests its vertex");
 std::mt19937 rng(19);std::uniform_real_distribution<float> random(-1,1);
 int oldInvalid=0,newInvalid=0;
 for(int i=0;i<120000;i++){
  float length=10+std::fabs(random(rng))*20000;
  a.Set(random(rng)*10000,random(rng)*10000,random(rng)*10000);b=a;b.x+=length;
  float radius=.1f+std::fabs(random(rng))*3;
  c=a;c.x+=length*(.1f+.8f*std::fabs(random(rng)));c.y+=radius+.002f+std::fabs(random(rng))*1.5f;
  v.Set(random(rng)*80,random(rng)*3,random(rng)*.2f);
  for(int version=0;version<2;version++){
   float h=1;CFVec3A contact;
   if(version)_ProjectSphereAgainstEdge(c,radius,v,a,b,h,contact);else _OriginalProjectSphereAgainstEdge(c,radius,v,a,b,h,contact);
   if(h<1){CFVec3A n;n.Sub(c,contact);float dist=-v.Dot(n)/n.Mag()*(1-h);
    if(!(dist>=0&&dist<=v.Mag()*1.02f)){if(version)newInvalid++;else oldInvalid++;}
    if(version){CFVec3A center;center.Mul(v,h).Add(c).Sub(contact);
     require(std::fabs(center.Mag()-radius)<.04f,"stable sphere sweep contact lies on radius at map-scale coordinates");}
   }
  }
 }
 require(oldInvalid>0,"legacy edge sweep reproduces the reported invalid push assertion");
 require(newInvalid==0,"stable sweep never produces negative/nonfinite/oversized impact push in stress cases");
 std::printf("PASS: %d RAT runtime checks; old invalid pushes=%d new=%d; game not launched.\n",checks,oldInvalid,newInvalid);
}
'''


def main():
    sphere = (ROOT / "ma/Lib/Fang2/fcoll_sphere.cpp").read_text()
    audio = (ROOT / "ma/Lib/Fang2/dx/fdx8audio.cpp").read_text()
    rat = (ROOT / "ma/App/ma/vehiclerat.cpp").read_text()
    jet = (ROOT / "ma/App/ma/botpred.cpp").read_text()
    projection = block(sphere, "static void _ProjectSphereAgainstEdge( const CFVec3A &vSphereCenter, f32 fRadius, const CFVec3A &vTravel,\n")
    original = projection[:projection.index("#if FANG_WINGC")] + projection.split("#else", 1)[1].rsplit("#endif", 1)[0] + "}"
    code = PRELUDE + block(audio, "static f32 _GC3DDistanceGain(")
    code += block(audio, "static void _Update3DDistanceGain(")
    code += block(sphere, "static BOOL _SphereSweepEntryTime(") + projection
    code += original.replace("_ProjectSphereAgainstEdge", "_OriginalProjectSphereAgainstEdge")
    code += block(rat, "void CVehicleRat::ClassHierarchyDrawEnable(")
    camera = block(rat, "void CVehicleRat::UpdateGunnerCamera(")
    start = camera.index("#if defined(MA_PC_INPUT)")
    code += "void CVehicleRat::TestCamera(){\n" + camera[start:camera.index("FWorld_nTrackerSkipListCount", start)] + "}\n"
    code += block(jet, "void CBotPred::_SoundWork(") + CHECKS
    apply = block(audio, "void _ApplyRealEmittersChanges( FLinkRoot_t *poVirtualEmittersListActive ) {")
    assert apply.index("_Update3DDistanceGain( poVirtualEmitter );") < apply.index("// Position, Velocity, Radius, Doppler.")
    assert "? _GC_3D_VOLUME_SCALE : poVirtualEmitter->fDistanceGain" not in apply
    OUT.mkdir(parents=True, exist_ok=True)
    (OUT / "runtime_guards.cpp").write_text(code)
    (OUT / "CMakeLists.txt").write_text(
        "cmake_minimum_required(VERSION 3.20)\nproject(rat_runtime_guards LANGUAGES CXX)\n"
        "add_executable(rat_runtime_guards runtime_guards.cpp)\n"
        "target_compile_features(rat_runtime_guards PRIVATE cxx_std_17)\n")
    for command in (["cmake", "-S", str(OUT), "-B", str(OUT / "out"), "-A", "Win32"],
                    ["cmake", "--build", str(OUT / "out"), "--config", "Release"]):
        run = subprocess.run(command, capture_output=True, text=True)
        if run.returncode:
            raise SystemExit(run.stdout + run.stderr)
    subprocess.run([str(OUT / "out/Release/rat_runtime_guards.exe")], check=True)


if __name__ == "__main__":
    main()
