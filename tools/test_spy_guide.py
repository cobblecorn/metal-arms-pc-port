"""Production factory inspection checks: forgiving co-op, original solo rules."""
from pathlib import Path
import subprocess
from test_coop_checkpoint_rat import method

ROOT = Path(__file__).resolve().parents[1]
OUT = ROOT / 'build/test-spy-guide'


def main():
    source = r'''
#include <cstdio>
#include <cstdlib>
#include <cmath>
using BOOL=int;using f32=float;using u32=unsigned;
constexpr BOOL TRUE=1,FALSE=0;
#define FANG_WINGC 1
#define FMATH_SQUARE(x) ((x)*(x))
#define FMATH_CLAMPMIN(x,y) do { if((x)<(y))(x)=(y); } while(0)
constexpr float _STAGE3_MIN_DOT_SIZE=.85f,_STAGE3_SAFE_RADIUS=4,_STAGE3_DEATH_TIME=1;
float FLoop_fPreviousLoopSecs=.6f;
struct Vec {float x=0,y=0,z=1;float Dot(const Vec&b)const{return x*b.x+y*b.y+z*b.z;}
 float DistSq(const Vec&b)const{return (x-b.x)*(x-b.x)+(y-b.y)*(y-b.y)+(z-b.z)*(z-b.z);}};
struct Matrix{Vec m_vFront,m_vPos;};
struct Bot{Matrix matrix;Matrix*MtxToWorld(){return &matrix;}} bot;
struct Human{int m_nPadFlagsJump=0,m_nPadFlagsMelee=0;} human;
struct Manager{bool coop=false;bool IsLocalCoop(){return coop;}} MultiplayerMgr;
struct CSpyVsSpy{static Bot*GetGlitch(){return &bot;}};
struct Locker{bool open=false;int closes=0;bool IsOpen(){return open;}void ForceClose(){open=false;++closes;}};
struct CSpyVsSpySoundCenter{enum{SOUND_LOCKER_CLOSE};static int GetGroup(int){return 1;}};
struct CFSoundGroup{static void PlaySound(int){}};
struct CSearchStage {
 enum GuyState_e{GUY_STATE_WALK_AWAY,GUY_STATE_WALK_BACK,GUY_STATE_WALK_AWAY_CONSOLE,
 GUY_STATE_WALK_CONSOLE,GUY_STATE_RESTING,GUY_STATE_CLOSE_LOCKER,GUY_STATE_FAULTY_GLITCH,GUY_STATE_KILL_GLITCH,GUY_STATE_DONE};
 GuyState_e m_eGuyState=GUY_STATE_WALK_AWAY,m_eGuyStateNext=GUY_STATE_WALK_AWAY;
 Vec m_SafeDirection,m_SpawnPoint;Human*m_pHumanControl=&human;float m_fDeathTimer=1;
 int faults=0;void _SetGuyState(GuyState_e state){m_eGuyState=state;++faults;}
 void _CheckDeath();
 enum{NUM_LOCKERS=18};Locker m_Lockers[NUM_LOCKERS];
 GuyState_e _InspectionFailureState(GuyState_e);void _CloseInspectionLockers();
};
int checks=0;void require(bool value,const char*message){++checks;if(!value){std::printf("FAIL: %s\n",message);std::exit(1);}}
'''
    source += method((ROOT / 'ma/App/ma/SpyVsSpy.cpp').read_text(), 'void CSearchStage::_CheckDeath(')
    production=(ROOT / 'ma/App/ma/SpyVsSpy.cpp').read_text()
    source += method(production,'CSearchStage::GuyState_e CSearchStage::_InspectionFailureState(')
    source += method(production,'void CSearchStage::_CloseInspectionLockers(')
    source += r'''
int main(){
 for(bool coop:{false,true})for(int disturbance=0;disturbance<4;++disturbance){
  MultiplayerMgr.coop=coop;bot=Bot();human=Human();CSearchStage stage;
  if(disturbance==0)bot.matrix.m_vFront.z=-1;
  if(disturbance==1)bot.matrix.m_vPos.x=6;
  if(disturbance==2)human.m_nPadFlagsJump=1;
  if(disturbance==3)human.m_nPadFlagsMelee=1;
  stage._CheckDeath();stage._CheckDeath();
  require(stage.faults==(coop?0:1),"co-op inspection cannot enter solo punishment; solo still does");
  if(coop)require(stage.m_fDeathTimer==1,"co-op leaves scripted inspection timer intact");
 }
 MultiplayerMgr.coop=false;bot=Bot();human=Human();CSearchStage normal;normal._CheckDeath();
 require(!normal.faults&&normal.m_fDeathTimer==1,"ordinary solo inspection remains valid");
 for(bool coop:{false,true})for(int opened:{0,1,5,18})for(auto requested:{CSearchStage::GUY_STATE_FAULTY_GLITCH,CSearchStage::GUY_STATE_KILL_GLITCH,CSearchStage::GUY_STATE_DONE}) {
  MultiplayerMgr.coop=coop;CSearchStage stage;
  for(int n=0;n<opened;++n)stage.m_Lockers[n].open=true;
  auto result=stage._InspectionFailureState(requested);
  bool lethal=requested!=CSearchStage::GUY_STATE_DONE;
  require(result==(coop&&lethal?(opened?CSearchStage::GUY_STATE_CLOSE_LOCKER:CSearchStage::GUY_STATE_WALK_AWAY):requested),"all lethal entries warn or retry only in co-op; successful states and solo unchanged");
  stage._CloseInspectionLockers();
  int closed=0,left=0;for(auto &locker:stage.m_Lockers){closed+=locker.closes;left+=locker.open;}
  require(closed==(coop?opened:(opened?1:0)),"co-op warning closes every offender; retail solo closes just one");
  require(left==(coop?0:(opened?opened-1:0)),"repeat open-locker failures leave no co-op lockers trapped open");
 }
 std::printf("PASS: %d production factory guide checks; game not launched.\n",checks);
}
'''
    source = source.replace('#include <cmath>', '#include <cmath>\n#include <initializer_list>')
    OUT.mkdir(parents=True, exist_ok=True)
    (OUT / 'guide.cpp').write_text(source)
    (OUT / 'CMakeLists.txt').write_text('cmake_minimum_required(VERSION 3.20)\nproject(guide LANGUAGES CXX)\nadd_executable(guide guide.cpp)\ntarget_compile_features(guide PRIVATE cxx_std_17)\n')
    for command in (['cmake', '-S', str(OUT), '-B', str(OUT / 'out'), '-A', 'Win32'],
                    ['cmake', '--build', str(OUT / 'out'), '--config', 'Release']):
        run = subprocess.run(command, capture_output=True, text=True)
        if run.returncode:
            raise SystemExit(run.stdout + run.stderr)
    subprocess.run([str(OUT / 'out/Release/guide.exe')], check=True)


if __name__ == '__main__':
    main()
