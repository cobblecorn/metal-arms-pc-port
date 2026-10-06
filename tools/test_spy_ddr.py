"""Run the production instructor validator with solo/co-op timing and target misses."""
from pathlib import Path
import subprocess
from test_coop_checkpoint_rat import method

ROOT = Path(__file__).resolve().parents[1]
OUT = ROOT / 'build/test-spy-ddr'

def main():
    source = r'''
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <initializer_list>
using BOOL=int;using f32=float;using u32=unsigned;
constexpr BOOL TRUE=1,FALSE=0;
#define FANG_WINGC 1
#define FMATH_CLAMP(x,a,b) do {if((x)<(a))(x)=(a);if((x)>(b))(x)=(b);}while(0)
#define FMATH_CLAMPMAX(x,b) do {if((x)>(b))(x)=(b);}while(0)
#define FMATH_SETBITMASK(x,b) ((x)|=(b))
#define FMATH_CLEARBITMASK(x,b) ((x)&=~(b))
#define DEVPRINTF(...) ((void)0)
#define FMATH_MIN(a,b) ((a)<(b)?(a):(b))
#define FMATH_CLAMPMIN(x,a) do {if((x)<(a))(x)=(a);}while(0)
constexpr float _DDR_MAX_MESH_ALPHA=.6f;
constexpr float _DDR_GOOD_ENOUGH_DIST_SQXZ=4;
constexpr float FMATH_POS_EPSILON=.0001f;
constexpr float _DDR_MAX_MISS_DIST_SQXZ=6,_DDR_MIN_HORZ_DOT=.96f,
 _DDR_MIN_HORZ_DOT_SECONDARY=.90f,_DDR_LOOK_DOT_MIN=.79f,_DDR_JUMP_FORGIVE_TIME=.5f;
constexpr unsigned GAMEPAD_BUTTON_1ST_PRESS_MASK=1,_DDR_MAX_MESSUPS=2;
using s32=int;
float FLoop_fPreviousLoopSecs=.1f;
struct CFVec3A {
 float x=0,y=0,z=0; void Add(const CFVec3A&a){x+=a.x;y+=a.y;z+=a.z;} void Sub(const CFVec3A&a){x-=a.x;y-=a.y;z-=a.z;}
 void Sub(const CFVec3A&a,const CFVec3A&b){*this=a;Sub(b);}
 float Dot(const CFVec3A&a)const{return x*a.x+y*a.y+z*a.z;}
 float MagSq()const{return Dot(*this);}
 void Unitize(){float n=std::sqrt(MagSq());x/=n;y/=n;z/=n;}
 float DistSqXZ(const CFVec3A&a)const{return (x-a.x)*(x-a.x)+(z-a.z)*(z-a.z);}
};
struct Matrix {CFVec3A m_vPos,m_vFront{0,0,1},m_vRight{1,0,0};};
struct Mover{void BeginFrame(){}void EndFrame(){}BOOL MoveToward(CFVec3A){return TRUE;}BOOL FaceToward(CFVec3A){return TRUE;}};
struct Brain{Mover mover;Mover*GetAIMover(){return &mover;}};
using CFMtx43A=Matrix;
struct Xfm{Matrix m_MtxF,m_MtxR;void BuildFromMtx(const Matrix&m,float){m_MtxF=m;m_MtxR=m;m_MtxR.m_vPos={-m.m_vPos.x,-m.m_vPos.y,-m.m_vPos.z};}};
struct FMeshInit_t{};
constexpr int FMESH_CULLDIR_NONE=0;
#define fnew new
struct CFWorldMesh{Xfm m_Xfm;bool added=false;float alpha=0,tint[3]{};
 void Init(FMeshInit_t*){}void RemoveFromWorld(){added=false;}void AddToWorld(){added=true;}
 BOOL IsAddedToWorld(){return added;}void SetCollisionFlag(BOOL){}void SetUserTypeBits(int){}
 void SetCullDirection(int){}void UpdateTracker(){}void SetMeshAlpha(float a){alpha=a;}
 void SetMeshTint(float r,float g,float b){tint[0]=r;tint[1]=g;tint[2]=b;}};
struct Manager{BOOL IsLocalCoop(){return TRUE;}}MultiplayerMgr;
struct CHumanControl{unsigned m_nPadFlagsJump=0;};
struct CBot{
 Matrix matrix;CFVec3A m_MountUnitFrontXZ_WS{0,0,1};int m_nPossessionPlayerIndex=0;
 Brain brain;BOOL IsDeadOrDying(){return FALSE;}Brain*AIBrain(){return &brain;}
 CHumanControl controls;Matrix*MtxToWorld(){return &matrix;}
 CHumanControl*Controls(){return &controls;}BOOL IsInAir(){return FALSE;}
 BOOL GetBotFlag_CanDoubleJump(){return TRUE;}
};
struct CPlayer{static inline int m_nPlayerCount=2;CBot*m_pEntityOrig=nullptr;BOOL control=TRUE;BOOL HasEntityControl(){return control;}}Player_aPlayer[4];
CBot players[4];
struct Config{float fDDRMoveDistance=6, fDDRAlignFailTime=.8f,fDDRPositionFailTime=1,fDDRLookFailTime=.8f;}config;
struct CSpyVsSpy {static inline Config*m_pConfigValues=&config;static CBot*GetGlitch(){return &players[0];}static void TakeControlFromPlayer(BOOL){}
 static float GetIdleTime(float){return .25f;}static float GetMoveTime(float){return 1;}};
struct CDDRStage {
 enum {STATE_WAIT_FINISH_COMMAND,STATE_GLITCH_GETS_KILLED,STATE_WALK_GLITCH_BACK,STATE_IDLE_TIME,STATE_CHECK_GLITCH,STATE_WAIT_DIALOG,STATE_WAIT_WORK};
 using StageState_e=int;
 enum {NUM_MINERS=2};
 enum {FLAG_JUMP1_IN_QUEUE=1,FLAG_JUMP2_IN_QUEUE=2,FLAG_FAKE_COMMAND=4,FLAG_HAVE_JUMP=8,
       FLAG_JUMP1=16,FLAG_JUMP2=32,FLAG_JUMP_OK=64,FLAG_WAIT_EXECUTE=128,FLAG_SPECIAL_FAIL_COMMANDS=256};
 int m_eStageState=STATE_WAIT_FINISH_COMMAND,m_uBadMoveCount=0,failures=0;u32 m_uFlags=0;
 float m_fIdleTimer=.4f,m_fAlignFailTimer=0,m_fPositionFailTimer=0,m_fLookFailTimer=0,
       m_fGlitchMoveTimer=0,m_fTimeInterp=0;
 bool coop=false,m_bCoopRetryPending=false;struct Lane{CFVec3A offset;u32 flags=0;float align=0,position=0,look=0,move=0;}m_aCoopDance[4];using CoopDanceState=Lane;
 Matrix m_StartMatrix,m_DirMatrix;CFVec3A m_LookPoint;CFVec3A m_DestPoints[2]{{-8,0,0},{8,0,0}},m_GlitchDestPoint;
 CBot miner;CBot*m_paMiners[2]{&miner,&miner};CBot*m_pKillMiner=&miner;
 BOOL _CoopActive(){return coop;}BOOL _IsInSpecialMode(){return FALSE;}
 void _CheckOnGlitch(){++failures;m_eStageState=STATE_CHECK_GLITCH;}void _SetGlitchGetsKilled(){++failures;m_bCoopRetryPending=true;}
 void _CoopPlacePlayers(){}void _CoopCommandState(BOOL){}
 BOOL _CoopFinishCommand();void _CheckBotsDone();void _CoopCheckPlayers();
 CFWorldMesh* m_apCoopHolo[3]{};CFWorldMesh holo;CFWorldMesh*m_pHoloMesh=&holo;float m_fHoloMeshAlpha=.6f,m_fHoloMeshAlphaFadeDir=-1.f;
 void _CoopHologramsWork();void _MeshAlphaWork();void _TestLoadGuides();
 void _CheckAlignment(CBot*);
};
int checks=0;void require(bool good,const char*reason){++checks;if(!good){std::printf("FAIL: %s\n",reason);std::exit(1);}}
'''
    production=(ROOT / 'ma/App/ma/SpyVsSpyDDR.cpp').read_text()
    for signature in ('void CDDRStage::_CheckAlignment(', 'BOOL CDDRStage::_CoopFinishCommand(', 'void CDDRStage::_CheckBotsDone(', 'void CDDRStage::_CoopHologramsWork(', 'void CDDRStage::_CoopCheckPlayers(', 'void CDDRStage::_MeshAlphaWork('):
        source += method(production, signature)
    source += 'void CDDRStage::_TestLoadGuides(){FMeshInit_t MeshInit;\n' + method(production, 'if( MultiplayerMgr.IsLocalCoop() ) {') + '\n_ExitWithError:;}'
    source += r'''
int main(){
 for(bool coop:{false,true}) for(int n=0;n<4;++n) {
  CDDRStage stage;stage.coop=coop;CBot bot;bot.m_nPossessionPlayerIndex=n;
  if(coop){stage.m_aCoopDance[n].offset={0,0,-4.f*n};bot.matrix.m_vPos=stage.m_aCoopDance[n].offset;}
  for(int frame=0;frame<50;++frame)stage._CheckAlignment(&bot);
  require(!stage.failures,"each correct lane passes independently");
  bot.matrix.m_vPos.x=2.8f;
  for(int frame=0;frame<50;++frame)stage._CheckAlignment(&bot);
  require((stage.failures!=0)==!coop,"small miss is forgiven in co-op; solo retains retail limit");
 }
 for(bool coop:{false,true}) {
  CDDRStage stage;stage.coop=coop;CBot bot;bot.matrix.m_vPos.x=8;
  for(int frame=0;frame<50;++frame)stage._CheckAlignment(&bot);
  require(stage.failures,"large persistent misses remain failures");
  CDDRStage turning;turning.coop=coop;CBot facing;facing.m_MountUnitFrontXZ_WS={0,0,-1};
  turning._CheckAlignment(&facing);
  require((turning.failures!=0)==!coop,"co-op has turn grace; solo retains immediate opposite-facing failure");
  for(int frame=0;frame<50;++frame)turning._CheckAlignment(&facing);
  require(turning.failures,"persistent incorrect facing remains a failure");
 }
 CDDRStage speaking;speaking.coop=true;speaking.m_uFlags=CDDRStage::FLAG_WAIT_EXECUTE;
 CBot early;early.matrix.m_vPos.x=8;early.m_MountUnitFrontXZ_WS={0,0,-1};
 for(int frame=0;frame<50;++frame)speaking._CheckAlignment(&early);
 require(!speaking.failures,"new command announcement does not validate against old position/facing");

 for(int n=0;n<4;++n){players[n].m_nPossessionPlayerIndex=n;Player_aPlayer[n].m_pEntityOrig=&players[n];}
 for(int count=2;count<=4;++count)for(int bad=-1;bad<count;++bad) {
  CPlayer::m_nPlayerCount=count;
  for(int kind=0;kind<3;++kind) {
   CDDRStage stage;stage.coop=true;stage.m_fIdleTimer=1.85f;
   stage.m_GlitchDestPoint={6,0,0};
   for(int n=0;n<count;++n) {
    auto&bot=players[n];bot.matrix.m_vPos={6,0,-4.f*n};bot.m_MountUnitFrontXZ_WS={0,0,1};
    stage.m_aCoopDance[n].offset={0,0,-4.f*n};stage.m_aCoopDance[n].flags=0;
    if(kind==0 && n==bad)bot.matrix.m_vPos.x=0; // ignored six-foot step
    if(kind==1 && n==bad)bot.m_MountUnitFrontXZ_WS={1,0,0}; // ignored quarter turn
    if(kind==2){stage.m_aCoopDance[n].flags=CDDRStage::FLAG_HAVE_JUMP|CDDRStage::FLAG_JUMP_OK;
     if(n==bad)stage.m_aCoopDance[n].flags &= ~CDDRStage::FLAG_JUMP_OK;}
   }
   // Exercise the actual advancement path with the shortest retail window.
   // Earlier tests let each failure timer run for five seconds, masking this bug.
   for(int frame=0;frame<30 && stage.m_eStageState==CDDRStage::STATE_WAIT_FINISH_COMMAND;++frame)
    stage._CheckBotsDone();
   require(bad<0 ? stage.m_eStageState==CDDRStage::STATE_IDLE_TIME : stage.failures!=0,
    "command deadline requires every player's step, turn and jump before advancing");
  }
 }
 CPlayer::m_nPlayerCount=2;
 CDDRStage forgiving;forgiving.coop=true;forgiving.m_GlitchDestPoint={6,0,0};
 for(int n=0;n<2;++n){players[n].matrix.m_vPos={8.8f,0,-4.f*n};players[n].m_MountUnitFrontXZ_WS={0,0,1};forgiving.m_aCoopDance[n].offset={0,0,-4.f*n};}
 require(forgiving._CoopFinishCommand(),"completed imperfect steps retain forgiving independent targets");
 forgiving.m_aCoopDance[0].flags=CDDRStage::FLAG_HAVE_JUMP|CDDRStage::FLAG_JUMP_OK;
 forgiving.m_aCoopDance[1].flags=CDDRStage::FLAG_HAVE_JUMP;
 require(!forgiving._CoopFinishCommand(),"one player's jump never approves another at the command deadline");

 forgiving.m_aCoopDance[1].flags |= CDDRStage::FLAG_JUMP_OK;
 require(forgiving._CoopFinishCommand(),"every player's completed jump passes at deadline");
 for(int n=0;n<2;++n)require(forgiving.m_aCoopDance[n].flags==0,"completed jump flags cannot authorize a jump in the following unrelated command");

 CDDRStage lastFrame;lastFrame.coop=true;lastFrame.m_uFlags=CDDRStage::FLAG_HAVE_JUMP|CDDRStage::FLAG_JUMP1;lastFrame.m_fIdleTimer=.01f;
 for(int n=0;n<2;++n){players[n].matrix.m_vPos={0,0,-4.f*n};players[n].controls.m_nPadFlagsJump=GAMEPAD_BUTTON_1ST_PRESS_MASK;
  lastFrame.m_aCoopDance[n].offset={0,0,-4.f*n};lastFrame.m_aCoopDance[n].flags=lastFrame.m_uFlags;lastFrame.m_aCoopDance[n].move=1.6f;}
 lastFrame._CoopCheckPlayers();lastFrame._CheckBotsDone();
 require(lastFrame.m_eStageState==CDDRStage::STATE_IDLE_TIME&&!lastFrame.failures,"both players' current-frame jumps count before the command deadline");
 for(int n=0;n<2;++n)players[n].controls.m_nPadFlagsJump=0;
 CDDRStage smallStep;smallStep.coop=true;config.fDDRMoveDistance=2;smallStep.m_GlitchDestPoint={2,0,0};
 for(int n=0;n<2;++n){players[n].matrix.m_vPos={0,0,-4.f*n};smallStep.m_aCoopDance[n].offset={0,0,-4.f*n};}
 require(!smallStep._CoopFinishCommand(),"tolerance cannot cover a whole step in smaller configurations");
 config.fDDRMoveDistance=6;
 CDDRStage solo;solo.m_fIdleTimer=.05f;solo.m_GlitchDestPoint={20,0,0};solo._CheckBotsDone();
 require(solo.m_eStageState==CDDRStage::STATE_IDLE_TIME,"solo command advancement retains authored behavior");

 CPlayer::m_nPlayerCount=0;CDDRStage guides;guides.coop=true;guides._TestLoadGuides();
 for(int n=0;n<3;++n)require(guides.m_apCoopHolo[n]!=nullptr,"resource loading reserves partner guides before player initialization");
 guides.holo.added=true;guides.holo.m_Xfm.m_MtxF.m_vPos={10,0,20};
 for(int n=0;n<4;++n)guides.m_aCoopDance[n].offset={0,0,-4.f*n};
 for(int count=1;count<=4;++count){CPlayer::m_nPlayerCount=count;guides._CoopHologramsWork();
  for(int n=0;n<3;++n){auto*mesh=guides.m_apCoopHolo[n];require(mesh->added==(n+1<count),"only active partners show their holograms");
   if(n+1<count){CFVec3A expected={10,0,20-4.f*(n+1)};require(mesh->m_Xfm.m_MtxF.m_vPos.DistSqXZ(expected)<.001f&&mesh->alpha==.6f,"each guide uses its own player's target offset and visibility");}}
 }
 // Production fade must run during the response and announcement states alike.
 for(bool coop:{false,true}) for(int state:{CDDRStage::STATE_WAIT_FINISH_COMMAND,CDDRStage::STATE_IDLE_TIME}) {
  guides.coop=coop;guides.m_eStageState=state;guides.holo.added=true;
  guides.m_fHoloMeshAlpha=.6f;guides.m_fHoloMeshAlphaFadeDir=-1.f;
  guides._MeshAlphaWork();guides._CoopHologramsWork();
  require(guides.holo.added&&guides.holo.alpha>0&&guides.holo.alpha<.6f,"hologram immediately fades on original timing in solo and co-op");
  if(coop)for(auto*mesh:guides.m_apCoopHolo)require(mesh->added&&mesh->alpha==guides.holo.alpha,"partner holograms share the original fade");
  for(int frame=0;frame<8;++frame){guides._MeshAlphaWork();guides._CoopHologramsWork();}
  require(!guides.holo.added&&guides.holo.alpha==0,"primary guide expires before command deadline");
  for(auto*mesh:guides.m_apCoopHolo)require(!mesh->added,"partner guide cannot outlive the primary flash");
 }
 guides.coop=false;guides._CoopHologramsWork();
 for(auto*mesh:guides.m_apCoopHolo){require(!mesh->added,"partner guides are hidden outside co-op");delete mesh;}
 std::printf("PASS: %d production instructor validator checks.\n",checks);
}
'''
    OUT.mkdir(parents=True, exist_ok=True)
    (OUT / 'ddr.cpp').write_text(source)
    (OUT / 'CMakeLists.txt').write_text('cmake_minimum_required(VERSION 3.20)\nproject(ddr LANGUAGES CXX)\nadd_executable(ddr ddr.cpp)\ntarget_compile_features(ddr PRIVATE cxx_std_17)\n')
    for cmd in (['cmake','-S',str(OUT),'-B',str(OUT/'out'),'-A','Win32'],
                ['cmake','--build',str(OUT/'out'),'--config','Release']):
        run = subprocess.run(cmd,capture_output=True,text=True)
        if run.returncode:
            raise SystemExit(run.stdout+run.stderr)
    subprocess.run([str(OUT/'out/Release/ddr.exe')],check=True)

if __name__ == '__main__':
    main()
