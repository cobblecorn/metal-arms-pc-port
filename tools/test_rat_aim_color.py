"""Production RAT input arithmetic and safe effect-color conversion; no game launch."""
from pathlib import Path
import subprocess
from test_collectable_skiplist import method

ROOT = Path(__file__).resolve().parent.parent
OUT = ROOT / "build/test-rat-aim-color"


def main():
    states = (ROOT / "ma/App/ma/site_botStateFns.cpp").read_text()
    start = states.index("f32 fYawVelocity =", states.index("void CBotSiteWeapon::_Do_Possessed"))
    end = states.index("m_pData->m_fYawWS +=", start)
    parse = (ROOT / "ma/App/ma/bot.cpp").read_text()
    line = next(line.strip() for line in parse.splitlines() if "m_fControls_AimDown = pPlayer->GetInvertLook()" in line)
    color = method((ROOT / "ma/Lib/Fang2/dx/fdx8.h").read_text(), "FINLINE u32 fdx8_ColorByte(")
    code = r'''
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <limits>
using f32=float;using u32=unsigned;using BOOL=bool;
#define FINLINE inline
#define MA_PC_INPUT 1
#define FALSE false
#define TRUE true
#define FMATH_MIN(a,b) ((a)<(b)?(a):(b))
int checks=0;
void require(bool b,const char* m){++checks;if(!b){std::fprintf(stderr,"FAIL: %s\n",m);std::exit(1);}}
u32 fmath_FloatToU32(float v){require(v>=0 && v<=255,"unsigned conversion receives a finite color byte");return unsigned(std::lrint(v));}
constexpr int BOTSUBCLASS_SITEWEAPON_RATGUN=1;
struct Player{float sensitivity=1,m_fYawAdjust=0,m_fPitchAdjust=0;bool invert=false;
 bool GetInvertLook(){return invert;}float ComputeLookSensitivityMultiplier(){return sensitivity;}}Player_aPlayer[4];
struct Human{float m_fAimDown=0;};float FLoop_fPreviousLoopSecs=.016f;
struct Gun{
 struct Def{int m_nSubClass=1;}def,*m_pBotDef=&def;
 struct Data{float m_fMaxYawVelocityPossess=1,m_fMaxPitchVelocityPossess=3;}data,*m_pData=&data;
 int m_nPossessionPlayerIndex=0;float m_fControls_RotateCW=0,m_fControls_AimDown=0,mouseYaw=0,mousePitch=0;
 float TakeMouseLookDelta(bool pitch){return pitch?mousePitch:mouseYaw;}
 float pitchResult=0,yawResult=0;
 void Test(float axis){Human human;human.m_fAimDown=axis;Human* pHumanControl=&human;Player* pPlayer=&Player_aPlayer[m_nPossessionPlayerIndex];
''' + line + "\n" + states[start:end] + r'''
 pitchResult=fPitchDelta;yawResult=fYawDelta;
 }};
''' + color + r'''
int main(){
 for(int p=0;p<4;p++)for(bool invert:{false,true})for(float sensitivity:{.5f,1.f,2.f}){
  Gun gun;gun.m_nPossessionPlayerIndex=p;auto& player=Player_aPlayer[p];player.invert=invert;player.sensitivity=sensitivity;
  for(float dt:{.016f,.033f}){
   FLoop_fPreviousLoopSecs=dt;gun.Test(1);
   require(invert?gun.pitchResult>0:gun.pitchResult<0,"up stick follows player inversion exactly once");
   require(std::fabs(std::fabs(gun.pitchResult)-dt*sensitivity)<.000001f,"RAT pitch rate is capped to yaw and uses profile sensitivity");
   gun.Test(-1);require(invert?gun.pitchResult<0:gun.pitchResult>0,"down stick follows player inversion exactly once");
  }
 }
 Gun mouse;mouse.mousePitch=.12f;mouse.Test(0);
 require(std::fabs(mouse.pitchResult-.12f)<.000001f,"mouse delta already adjusted by TakeMouseLookDelta is not inverted twice");
 mouse.def.m_nSubClass=2;mouse.mousePitch=0;mouse.Test(1);
 require(std::fabs(std::fabs(mouse.pitchResult)-3*.033f)<.000001f,"other stationary guns retain authored pitch rate");
 require(fdx8_ColorByte(-.01f)==0 && fdx8_ColorByte(-100)==0,"negative decal channels clamp before unsigned conversion");
 require(fdx8_ColorByte(1.01f)==255 && fdx8_ColorByte(100)==255,"bright effect channels cannot spill into another byte");
 require(fdx8_ColorByte(std::numeric_limits<float>::quiet_NaN())==0,"NaN effect channel is safe");
 require(fdx8_ColorByte(std::numeric_limits<float>::infinity())==255,"infinite effect channel saturates");
 for(int n=0;n<=255;n++)require(fdx8_ColorByte(float(n)/255)==unsigned(n),"valid color bytes retain their original conversion");
 std::printf("PASS: %d production RAT aim/effect-color checks; game not launched.\n",checks);
}
'''
    code = "#include <initializer_list>\n" + code
    OUT.mkdir(parents=True, exist_ok=True)
    (OUT / "aim_color.cpp").write_text(code)
    (OUT / "CMakeLists.txt").write_text(
        "cmake_minimum_required(VERSION 3.20)\nproject(rat_aim_color LANGUAGES CXX)\n"
        "add_executable(rat_aim_color aim_color.cpp)\ntarget_compile_features(rat_aim_color PRIVATE cxx_std_17)\n")
    for command in (["cmake", "-S", str(OUT), "-B", str(OUT / "out")],
                    ["cmake", "--build", str(OUT / "out"), "--config", "Release"]):
        run = subprocess.run(command, capture_output=True, text=True)
        if run.returncode:
            raise SystemExit(run.stdout + run.stderr)
    subprocess.run([str(OUT / "out/Release/rat_aim_color.exe")], check=True)


if __name__ == "__main__":
    main()
