"""Execute AA gun and Hold Your Ground production helpers offline; never launch the game."""
from pathlib import Path
import subprocess
from test_coop_checkpoint_rat import method
ROOT = Path(__file__).resolve().parent.parent
OUT = ROOT / "build/test-coop-defense"
FIXTURE = r'''
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <string>
#include <vector>
#include <new>
#include <stdexcept>
using BOOL=int;using s32=int;using u32=unsigned;using f32=float;using cchar=const char;
constexpr BOOL TRUE=1,FALSE=0;
#define FANG_WINGC 1
#define _VERBOSE_DEBUGGING_MSG 0
#define FASSERT(x) ((void)0)
#define DEVPRINTF(...) ((void)0)
constexpr int ENTITY_BIT_BOT=1;
struct FangAllocTag{};
bool fangDelete=false;
#define fnew new(FangAllocTag{})
#define fdelete(p) do{fangDelete=true;delete(p);fangDelete=false;}while(0)
struct CFVec3A {float x=0,y=0,z=0;void Sub(const CFVec3A&a,const CFVec3A&b){x=a.x-b.x;y=a.y-b.y;z=a.z-b.z;}float MagSq(){return x*x+y*y+z*z;}};
struct CFMtx43A {CFVec3A m_vPos,m_vRight{1,0,0};};
struct CAIBrain {};
struct Control {int owner=-1;};
struct CBot {
 int m_nPossessionPlayerIndex=-1;bool live=true,dead=false,switching=false;Control* controls=nullptr;void* mech=nullptr;
 CFMtx43A matrix;CAIBrain brain;float health=.7f,batteries=3;void* armor=nullptr;unsigned team=1;bool invincible=false;
 bool autoDelete=false;void SetAutoDelete(BOOL x){autoDelete=x;}
 static inline BOOL m_bCutscenePlaying=FALSE;
 BOOL IsPlayerBot(){return m_nPossessionPlayerIndex>=0;}BOOL IsInWorld(){return live;}BOOL IsDeadOrDying(){return dead;}
 BOOL SwitchingWeapons(){return switching;}void* GetCurMech(){return mech;}Control* Controls(){return controls;}
 int TypeBits(){return ENTITY_BIT_BOT;}CAIBrain* AIBrain(){return &brain;}CFMtx43A* MtxToWorld(){return &matrix;}
 float HealthContainerCount()const{return batteries;}float NormHealth()const{return health;}void* GetArmorProfile()const{return armor;}
 unsigned GetTeam()const{return team;}BOOL IsInvincible()const{return invincible;}
 void SetHealthContainerCount(float x){batteries=x;}void SetNormHealth(float x){health=x;}void SetArmorProfile(void*x){armor=x;}
 void InitialTeam(unsigned x){team=x;}void SetInvincible(BOOL x){invincible=x;}
 void HeadStopLook(){}void HeadLook(){}static void DisableBotDamageGlobally(){}static void EnableBotDamageGlobally(){}
 static void SetCutscenePlaying(BOOL x){m_bCutscenePlaying=x;}
};
struct CVehicle {enum StationStatus_e {STATION_STATUS_EMPTY,STATION_STATUS_ENTERING,STATION_STATUS_OCCUPIED,STATION_STATUS_EXITING};};
struct CVehicleRat {BOOL IsInWorld(){return FALSE;}BOOL IsDeadOrDying(){return FALSE;}void* GetSplinePath(){return nullptr;}
 float GetMaxUnitSpeed(){return 1;}void SetMaxUnitSpeed(float,BOOL){}void SetSplinePath(void*){}};
struct CFCamera {struct Xfm {CFMtx43A m_MtxR;}xfm;float fov=.5f;Xfm* GetFinalXfm(){return &xfm;}void GetFOV(float*x){*x=fov;}};
CFCamera cameras[4];using GameCamPlayer_e=int;
CFCamera* fcamera_GetCameraByIndex(int i){return &cameras[i];}
int cameraSwitched[4]={};void gamecam_SwitchPlayerTo3rdPersonCamera(int i,void*){cameraSwitched[i]++;}
struct CamInfo {CFMtx43A* m_pmtxMtx=nullptr;float m_fHalfFOV=0;};
struct gamecam_CamBotTransInfo_t {CBot*pBot1,*pBot2;int uPlayer;float fUnitInterp;};
struct gamecam_CamBotTransResult_t {};
void gamecam_DoCamBotTransition(gamecam_CamBotTransInfo_t*,CamInfo*,gamecam_CamBotTransResult_t*,gamecam_CamBotTransResult_t*,gamecam_CamBotTransResult_t*){}
float fmath_UnitLinearToSCurve(float x){return x;}float FLoop_fPreviousLoopSecs=.25f;
constexpr float _USERPROPS_CAMRATE_IN=1,_USERPROPS_CAMRATE_OUT=1;
struct CBotAAGun:CBot {
 enum {CAMERA_STATE_INACTIVE,CAMERA_STATE_START_ENTER_VEHICLE,CAMERA_STATE_ENTERING_VEHICLE,CAMERA_STATE_IN_VEHICLE,CAMERA_STATE_START_EXIT_VEHICLE,CAMERA_STATE_EXITING_VEHICLE};
 enum {BONE_ATTACHPOINT_BOT=0};const char* m_apszBoneNameTable[1]={"seat"};
 BOOL m_bLimitHeading=FALSE,m_bMorterAttachment=FALSE;float m_fMaxHeading=0,m_fMinHeading=0,m_fMinPitch=0,m_fMaxPitch=0,m_fHeadingPerSec=0,m_fPitchPerSec=0;
 CBot*m_pDriverBot=nullptr,*m_pCameraBot=nullptr;CVehicle::StationStatus_e m_eStationStatus=CVehicle::STATION_STATUS_EMPTY;
 int m_eCameraState=CAMERA_STATE_INACTIVE;BOOL exitAllowed=TRUE,movement=TRUE,msg=TRUE,reticle=TRUE;int boards=0;
 float m_fUnitCameraTransition=0;CamInfo m_CamInfo;CFMtx43A m_ManCamMtx,m_TransCamMtx;std::string name;
 static inline int liveClones=0,failAt=-1,createCalls=0;
 static inline int allocations=0,deallocations=0,allocCalls=0,allocFailAt=-1;
 static void* operator new(std::size_t){throw std::runtime_error("Fang: Use fnew instead of new");}
 static void* operator new(std::size_t size,FangAllocTag)noexcept{
  if(allocCalls++==allocFailAt)return nullptr;
  void*p=::operator new(size,std::nothrow);if(p)allocations++;return p;
 }
 static void operator delete(void*p){if(!fangDelete)std::abort();deallocations++;::operator delete(p);}
 static void operator delete(void*p,FangAllocTag){deallocations++;::operator delete(p);}
 CBotAAGun(){liveClones++;}~CBotAAGun(){liveClones--;}
 BOOL Create(int,BOOL,const char*n,const CFMtx43A*m,const char*){name=n;matrix=*m;return createCalls++!=failAt;}
 BOOL PortCreateDefenseGun(const CBotAAGun*,const char*,const CFMtx43A*);
 BOOL PortBoardDefensePlayer(CBot*);BOOL PortDefensePlayerReady(CBot*)const;void _CameraTransitionWork();
 void EnableDriverExit(BOOL x){exitAllowed=x;}void EnableDriverMovement(BOOL x){movement=x;}
 void EnableEnterMsgDisplay(BOOL x){msg=x;}void ReticleEnable(BOOL x){reticle=x;}
 BOOL IsGunInUse(){return m_eStationStatus==CVehicle::STATION_STATUS_OCCUPIED;}
 void DriverEnter(CBot*b,const char*){boards++;m_pDriverBot=m_pCameraBot=b;controls=b->controls;b->controls=nullptr;b->mech=this;
 m_nPossessionPlayerIndex=b->m_nPossessionPlayerIndex;m_eStationStatus=CVehicle::STATION_STATUS_ENTERING;m_eCameraState=CAMERA_STATE_START_ENTER_VEHICLE;}
 CBot* pendingJump=nullptr;void ActionNearby(CBot*b){pendingJump=b;}
 void finishJump(){DriverEnter(pendingJump,"seat");pendingJump=nullptr;}
 void settle(){m_eStationStatus=CVehicle::STATION_STATUS_OCCUPIED;m_eCameraState=CAMERA_STATE_IN_VEHICLE;}
 void reset(){if(m_pDriverBot){m_pDriverBot->mech=nullptr;m_pDriverBot->controls=controls;}controls=nullptr;m_pDriverBot=m_pCameraBot=nullptr;
 m_eStationStatus=CVehicle::STATION_STATUS_EMPTY;m_eCameraState=CAMERA_STATE_INACTIVE;movement=exitAllowed=msg=TRUE;}
};
struct CPlayer {static inline int m_nPlayerCount=1;static inline CPlayer*m_pCurrent=nullptr;CBot*m_pEntityOrig=nullptr,*m_pEntityCurrent=nullptr;
 Control control;bool enabled=true;int disables=0,enables=0;void DisableEntityControl(){enabled=false;disables++;m_pEntityCurrent->controls=nullptr;}
 void EnableEntityControl(){enabled=true;enables++;m_pEntityCurrent->controls=&control;}void ZeroControls(){}};
CPlayer Player_aPlayer[4];struct Multi {bool coop=false;BOOL IsLocalCoop(){return coop;}}MultiplayerMgr;
struct CHud2 {bool done=true;static CHud2* GetHudForPlayer(int);BOOL TransmissionMsg_IsDonePlaying(){return done;}};
CHud2 huds[4];CHud2* CHud2::GetHudForPlayer(int i){return &huds[i];}
enum {_WALK_STATES_START,_WALK_STATES_WAIT,_WALK_STATES_WALK,_WALK_STATES_ACTION,_WALK_STATES_DONE};
enum {_STATE_WAIT_TO_GET_INTO_GUN,_STATE_ACTIVE,_STATE_WAIT,_STATE_LOSE};
struct Emitter {void Destroy(){}};
struct LevelData {CBotAAGun*pAAGun=nullptr,*apDefenseGuns[4]={};int nDefensePlayers=0;BOOL bDefenseIntroFinished=FALSE,bPlayedFirstTime=FALSE;
 int nWalkState=_WALK_STATES_START,nState=_STATE_WAIT_TO_GET_INTO_GUN,nWaveNum=0,nLastSpawnIndex=0;
 float fTimer=0,fTimeInWave=0,fTimeTillNextSpawn=0,fAlarmVolume=0;Emitter*pAlarmEmitter=nullptr;CBot*pJumpPoint=nullptr;
 unsigned nNumRats=0;float*pafRatSinkY=nullptr;CVehicleRat**papRats=nullptr;
};
LevelData data,*_pLevelData=&data;CAIBrain* lastGoal=nullptr;int starts=0,ends=0,convoyResets=0;
void aibrainman_Activate(CAIBrain*){}void aibrainman_Deactivate(CAIBrain*){}
void aibrainman_ConfigurePlayerBotBrain(CAIBrain*,int){}void ai_NotifyCutSceneBegin(){}void ai_NotifyCutSceneEnd(){}
void ai_AssignGoal_Goto(CAIBrain*b,const CFVec3A&,int,int){lastGoal=b;}
void game_EnterLetterbox(void*,BOOL,BOOL){starts++;}void game_LeaveLetterbox(){ends++;}
void _ResetAllSpawnPtConvoys(){convoyResets++;}BOOL breach=FALSE;BOOL _CheckTriggerBoxForLiveRats(){return breach;}
int checks=0;void require(bool x,const char*why){checks++;if(!x){std::fprintf(stderr,"FAIL: %s\n",why);std::exit(1);}}
'''

CHECKS = r'''
int main(){
 bool blocked=false;
 try{auto*p=new CBotAAGun;(void)p;}catch(const std::runtime_error&){blocked=true;}
 require(blocked,"fixture reproduces Fang's rejection of plain turret new");
 CBot bots[4],jump;CBotAAGun original;int armor=0;
 original.m_bLimitHeading=TRUE;original.m_fMaxHeading=1.7f;original.m_fMinHeading=-1.7f;
 original.m_fMinPitch=-.5f;original.m_fMaxPitch=.8f;original.m_fHeadingPerSec=.6f;original.m_fPitchPerSec=.9f;
 original.m_bMorterAttachment=TRUE;original.armor=&armor;original.invincible=true;
 original.matrix.m_vPos={230,147,588};original.matrix.m_vRight={-1,0,0};
 for(int count=1;count<=4;count++){
  data=LevelData{};data.pAAGun=&original;data.pJumpPoint=&jump;CPlayer::m_nPlayerCount=count;MultiplayerMgr.coop=count>1;
  for(int i=0;i<4;i++){bots[i]=CBot{};bots[i].m_nPossessionPlayerIndex=i;Player_aPlayer[i].m_pEntityOrig=Player_aPlayer[i].m_pEntityCurrent=&bots[i];
   Player_aPlayer[i].control.owner=i;Player_aPlayer[i].EnableEntityControl();}
  CPlayer::m_pCurrent=&Player_aPlayer[count-1];original.reset();CBotAAGun::failAt=-1;
  require(_CreateCoopDefenseGuns(),"create team guns");require(data.nDefensePlayers==count,"gun count");
  for(int i=1;i<count;i++){
   CBotAAGun*g=data.apDefenseGuns[i];float offset=(i&1)?16.f*((i+1)/2):-16.f*(i/2);
   require(g!=&original&&g->matrix.m_vPos.x==230-offset&&g->matrix.m_vPos.y==147&&g->matrix.m_vPos.z==588,"rotated row placement and unchanged height");
   require(g->matrix.m_vRight.x==-1,"same facing");require(g->m_bMorterAttachment&&g->m_bLimitHeading,"copy mortar and heading mode");
   require(g->m_fMaxHeading==1.7f&&g->m_fMinHeading==-1.7f&&g->m_fMinPitch==-.5f&&g->m_fMaxPitch==.8f,"copy aim limits");
   require(g->m_fHeadingPerSec==.6f&&g->m_fPitchPerSec==.9f,"copy aim rates");
   require(g->health==original.health&&g->batteries==3&&g->armor==&armor&&g->invincible,"copy durability");
   require(!g->exitAllowed&&!g->msg&&!g->m_pDriverBot&&!g->controls,"no copied ownership");
   require(g->autoDelete&&!original.autoDelete,"runtime clone is world-owned without changing authored original ownership");
  }
  require(!_WalkToJumpPoint(),"intro starts");require(!Player_aPlayer[0].enabled,"P1 drives intro even when current is last player");
  for(int i=1;i<count;i++)require(!Player_aPlayer[i].enabled,"partners cannot steal gun during intro");
  data.bPlayedFirstTime=TRUE;_WalkToJumpPoint();require(lastGoal==bots[0].AIBrain(),"goto goal belongs to P1");
  bots[0].matrix=jump.matrix;_WalkToJumpPoint();
  // Retail entry finishes its jump after ACTION restores P1 controls.
  _WalkToJumpPoint();original.finishJump();
  require(_WalkToJumpPoint(),"intro completes");int oldEnds=ends;
  require(_WalkToJumpPoint()&&ends==oldEnds,"no repeated letterbox end or control reassignment");
  for(int i=1;i<count;i++)require(Player_aPlayer[i].enabled,"partners restored before boarding");
  require(!_CoopDefenseTeamReady(),"waves blocked while entry unsettled");
  for(int i=0;i<count;i++){
   CBotAAGun*g=data.apDefenseGuns[i];require(g->m_pDriverBot==&bots[i],"assigned driver");
   require(g->controls==&Player_aPlayer[i].control,"independent controls");
   cameras[i].xfm.m_MtxR.m_vPos={float(i*100),0,0};g->m_eCameraState=CBotAAGun::CAMERA_STATE_START_ENTER_VEHICLE;
   g->_CameraTransitionWork();require(g->m_TransCamMtx.m_vPos.x==i*100,"entry begins from own camera");
   for(int n=0;n<4;n++)g->_CameraTransitionWork();
   require(cameraSwitched[i]>0&&g->m_eCameraState==CBotAAGun::CAMERA_STATE_IN_VEHICLE,"indexed camera finishes");g->settle();
  }
  require(_CoopDefenseTeamReady(),"all ready unlocks wave");
  for(int i=1;i<count;i++){int n=data.apDefenseGuns[i]->boards;_CoopDefenseTeamReady();require(data.apDefenseGuns[i]->boards==n,"no duplicate boarding");}
  data.nState=_STATE_ACTIVE;breach=FALSE;mg_holdyourground_PlayerLost();require(data.nState==_STATE_ACTIVE,"false breach ignored");
  breach=TRUE;mg_holdyourground_PlayerLost();require(data.nState==_STATE_LOSE,"one shared failure");
  for(int i=0;i<count;i++)require(!data.apDefenseGuns[i]->movement&&!data.apDefenseGuns[i]->reticle,"all guns disabled on failure");
  mg_HoldYourGround_Restore();require(!data.bDefenseIntroFinished&&data.nWalkState==_WALK_STATES_START&&data.nState==_STATE_WAIT_TO_GET_INTO_GUN,"retry replays team seating");
  for(int i=0;i<count;i++)data.apDefenseGuns[i]->reset();
  original.DriverEnter(&bots[0],"seat");original.settle();
  if(count>1){bots[count-1].switching=true;require(!_CoopDefenseTeamReady(),"retry waits for weapon transition");bots[count-1].switching=false;}
  _CoopDefenseTeamReady();for(int i=0;i<count;i++)data.apDefenseGuns[i]->settle();
  require(_CoopDefenseTeamReady(),"retry restores team readiness");
  for(int i=0;i<count;i++)require(!data.apDefenseGuns[i]->exitAllowed&&data.apDefenseGuns[i]->movement,"retry locks exits and re-enables aiming");
  for(int i=1;i<count;i++){data.apDefenseGuns[i]->reset();fdelete(data.apDefenseGuns[i]);}original.reset();
  require(CBotAAGun::allocations==CBotAAGun::deallocations,"successful team creation uses paired Fang allocation/deletion");
 }
 for(int fail=0;fail<3;fail++){
  data=LevelData{};data.pAAGun=&original;CPlayer::m_nPlayerCount=4;MultiplayerMgr.coop=true;
  CBotAAGun::createCalls=0;CBotAAGun::failAt=fail;int before=CBotAAGun::liveClones;
  require(!_CreateCoopDefenseGuns(),"allocation failure aborts load");require(CBotAAGun::liveClones==before,"failure rolls back every clone");
  require(CBotAAGun::allocations==CBotAAGun::deallocations,"creation failure cleans up through Fang deletion");
 }
 CBotAAGun::failAt=-1;
 for(int fail=0;fail<3;fail++){
  data=LevelData{};data.pAAGun=&original;CPlayer::m_nPlayerCount=4;MultiplayerMgr.coop=true;
  CBotAAGun::allocCalls=0;CBotAAGun::allocFailAt=fail;CBotAAGun::createCalls=0;
  int before=CBotAAGun::liveClones;
  require(!_CreateCoopDefenseGuns(),"null Fang allocation aborts load safely");
  require(CBotAAGun::createCalls==fail,"null turret is never dereferenced");
  require(CBotAAGun::liveClones==before&&CBotAAGun::allocations==CBotAAGun::deallocations,"allocation failure rolls back earlier turrets without leaks");
  for(int i=1;i<=fail;i++)require(!data.apDefenseGuns[i],"allocation rollback clears stored clone pointers");
 }
 CBotAAGun::allocFailAt=-1;
 CBotAAGun gun;CBot bad;require(!gun.PortBoardDefensePlayer(nullptr)&&!gun.PortBoardDefensePlayer(&bad),"reject null and NPC");
 bad.m_nPossessionPlayerIndex=1;Control c;bad.controls=&c;bad.dead=true;require(!gun.PortBoardDefensePlayer(&bad),"reject dead");
 bad.dead=false;bad.live=false;require(!gun.PortBoardDefensePlayer(&bad),"reject absent");bad.live=true;bad.mech=&original;
 require(!gun.PortBoardDefensePlayer(&bad),"reject another mech");bad.mech=nullptr;bad.switching=true;
 require(!gun.PortBoardDefensePlayer(&bad),"wait for equipment transition");bad.switching=false;
 require(gun.PortBoardDefensePlayer(&bad),"instant own-seat boarding");require(!gun.PortDefensePlayerReady(&bad),"camera must settle");
 gun.settle();require(gun.PortDefensePlayerReady(&bad),"ready only after seat and camera");gun.reset();
 std::printf("PASS: %d offline co-op defense checks; game not launched.\n",checks);
}
'''

def main():
    gun = (ROOT / "ma/App/ma/botAAgun.cpp").read_text()
    mission = (ROOT / "ma/App/ma/MG_HoldYourGround.cpp").read_text()
    code = FIXTURE
    for signature in ("BOOL CBotAAGun::PortCreateDefenseGun(", "BOOL CBotAAGun::PortBoardDefensePlayer(",
                      "BOOL CBotAAGun::PortDefensePlayerReady(", "void CBotAAGun::_CameraTransitionWork("):
        code += method(gun, signature) + "\n"
    for signature in ("static BOOL _CreateCoopDefenseGuns(", "static BOOL _CoopDefenseTeamReady(",
                      "static BOOL _WalkToJumpPoint( void ) {", "void mg_holdyourground_PlayerLost(",
                      "void mg_HoldYourGround_Restore("):
        code += method(mission, signature) + "\n"
    OUT.mkdir(parents=True, exist_ok=True)
    (OUT / "defense.cpp").write_text(code + CHECKS)
    (OUT / "CMakeLists.txt").write_text(
        "cmake_minimum_required(VERSION 3.20)\nproject(coop_defense LANGUAGES CXX)\n"
        "add_executable(coop_defense defense.cpp)\ntarget_compile_features(coop_defense PRIVATE cxx_std_17)\n")
    for command in (["cmake", "-S", str(OUT), "-B", str(OUT / "out"), "-A", "Win32"],
                    ["cmake", "--build", str(OUT / "out"), "--config", "Release"]):
        run = subprocess.run(command, capture_output=True, text=True)
        if run.returncode:
            raise SystemExit(run.stdout + run.stderr)
    subprocess.run([str(OUT / "out/Release/coop_defense.exe")], check=True)

if __name__ == "__main__":
    main()
