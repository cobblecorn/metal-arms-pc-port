"""Offline co-op gate regression checks; never starts the game or touches saves.

Compiles the production gate, box-collision and occupant-removal methods against
small world/player fixtures. Requires CMake and a C++ compiler (MSVC on Windows).
Outputs stay under build/test-coop-gates. Run: python tools/test_coop_gate_recovery.py
"""
from pathlib import Path
import subprocess

ROOT = Path(__file__).resolve().parent.parent
OUT = ROOT / "build/test-coop-gates"


def method(source, signature):
    start = source.index(signature)
    brace = source.index("{", start)
    depth = 1
    end = brace + 1
    while depth:
        depth += (source[end] == "{") - (source[end] == "}")
        end += 1
    return source[start:end]


FIXTURE = r'''
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <algorithm>
#include <array>
using BOOL=int; using u32=unsigned; using s32=int; using f32=float; using cchar=const char; using u64=unsigned long long;
u64 FLoop_nTotalLoopTicks=0; u32 FLoop_nTicksPerSec=1000;
constexpr BOOL TRUE=1,FALSE=0;
constexpr int MAX_PLAYERS=4;
constexpr unsigned ENTITY_BIT_BOT=1;
#define FANG_WINGC 1
#define SCRIPT_MESSAGE(...) ((void)0)
#define DEVPRINTF(...) ((void)0)
#define FMATH_CLAMPMIN(x,y) (x=std::max(x,y))
#define FMATH_CLAMPMAX(x,y) (x=std::min(x,y))
int GetEnvironmentVariableA(const char*,char*,int){return 0;}
int fclib_strlen(const char* s){return int(std::strlen(s));}
int fclib_stricmp(const char* a,const char* b){return _stricmp(a,b);}
int fclib_strnicmp(const char* a,const char* b,int n){return _strnicmp(a,b,n);}
float fmath_Abs(float x){return std::fabs(x);}
struct CFVec3A {
 float x=0,y=0,z=0;
 void Sub(const CFVec3A& a,const CFVec3A& b){x=a.x-b.x;y=a.y-b.y;z=a.z-b.z;}
 void Sub(const CFVec3A& b){x-=b.x;y-=b.y;z-=b.z;}
 void Set(float a,float b,float c){x=a;y=b;z=c;}
 float Dot(const CFVec3A& b)const{return x*b.x+y*b.y+z*b.z;}
 float DistSq(const CFVec3A& b)const{CFVec3A v;v.Sub(*this,b);return v.Dot(v);}
};
struct Matrix {
 CFVec3A m_vPos, m_vRight{1,0,0},m_vUp{0,1,0},m_vFront{0,0,1};
 void MulDir(CFVec3A& v)const {
  CFVec3A r{v.x*m_vRight.x+v.y*m_vUp.x+v.z*m_vFront.x,
   v.x*m_vRight.y+v.y*m_vUp.y+v.z*m_vFront.y,
   v.x*m_vRight.z+v.y*m_vUp.z+v.z*m_vFront.z};v=r;
 }
};
class CEntity;
struct CDoorEntity {enum{GOTOREASON_DESTINATION}; void GotoPos(int,int){};};
struct CTripwire {
 enum{TRIPWIRE_KILLMODE_NONE,TRIPWIRE_KILLMODE_VOLUME,TRIPWIRE_KILLMODE_PLANE};
 int m_eKillMode=TRIPWIRE_KILLMODE_NONE,m_nTripwireTriggerMode=1;
 bool m_bKillModeSpawnDeathEffects=false;
 const char* m_pszTripwireFilterEntityName=nullptr;
 CDoorEntity* m_pDoorToOpen=nullptr;
 u32 m_nContainedEntityCount=0; CEntity* m_ppContainedTrippersArray[4]{};
 CEntity* owner=nullptr;
 void OnEnter(CEntity*);
};
class CEntity {
public:
 enum {TRIPWIRE_COLLFLAG_NONE=0,TRIPWIRE_COLLFLAG_ENTER_EVENT=1,TRIPWIRE_COLLFLAG_EXIT_EVENT=2,
  TRIPWIRE_COLLFLAG_NEWPOS_INSIDE=4,TRIPWIRE_COLLFLAG_KILL_NOW=8,TRIPWIRE_COLLFLAG_SPAWN_DEATH_EFFECTS=16,
  TRIPWIRE_TRIGGER_MODE_ONCE=1};
 bool inWorld=true,armed=true,dead=false; const char* name="player"; Matrix matrix;
 int fired=0; CEntity* lastTripper=nullptr; CTripwire* m_pTripwire=nullptr;
 u32 m_nIntersectingTripwireCount=0;CEntity* m_apIntersectingTripwires[4]{};
 const Matrix* MtxToWorld()const{return &matrix;}
 bool IsInWorld()const{return inWorld;} bool IsTripwire()const{return m_pTripwire!=nullptr;}
 bool IsTripwireArmed()const{return armed;} const char* Name()const{return name;}
 unsigned TypeBits()const{return m_pTripwire?0:ENTITY_BIT_BOT;}
 void RemoveFromWorld(){inWorld=false;}
 virtual u32 TripwireCollisionTest(const CFVec3A*,const CFVec3A*){return 0;}
 BOOL _CoopHoldTripwireEnter(CEntity*);
 void _CoopFireTripwireEnter(CEntity*);
 u32 _CoopPlayersInsideMask(BOOL=FALSE);
 void _EntityIsOutsideTripwire(CEntity*);
 static void CoopTripwireWork(); static void CoopTripwireReset();
 static BOOL CoopTripwireWaiting(const CEntity*); static BOOL CoopTripwireExitWaiting(const CEntity*);
 static BOOL CoopTripwireWaitMessage(const CEntity*);
};
void CTripwire::OnEnter(CEntity* p){owner->fired++;owner->lastTripper=p;}
struct CBot:CEntity {static bool m_bCutscenePlaying; bool IsDeadOrDying(){return dead;}};
bool CBot::m_bCutscenePlaying=false;
struct CPlayer {static int m_nPlayerCount; CEntity* m_pEntityCurrent=nullptr;CEntity* m_pEntityOrig=nullptr;};
int CPlayer::m_nPlayerCount=2; CPlayer Player_aPlayer[4];
struct {bool single=true;bool IsSinglePlayer(){return single;}} MultiplayerMgr;
int game_GetStoryPlayerIndex(){return 0;}
constexpr int LEVEL_SINGLE_PLAYER_COUNT=42;
int Level_nLoadedIndex=0;
struct LevelInfo {const char* pszWorldResName="WERMmorbot1";}Level_aInfo[LEVEL_SINGLE_PLAYER_COUNT];
class CEBox:public CEntity {
public:
 Matrix m_MtxToWorld;
 CFVec3A m_aCorner_WS[2]{{-1,-1,-.1f},{1,1,.1f}};
 BOOL _PosInside(const CFVec3A&);
 BOOL _PassedThroughOriginZPlane(const CFVec3A*,const CFVec3A*){return FALSE;}
 u32 TripwireCollisionTest(const CFVec3A*,const CFVec3A*)override;
};
'''

CHECKS = r'''
int checks=0;
void require(bool value,const char* label){++checks;if(!value){std::printf("FAIL: %s\n",label);std::exit(1);}}
CBot bodies[4]; CEBox gate; CTripwire tripwire;
void reset(int count=2,const char* name="trigger_btr01") {
 CEntity::CoopTripwireReset();CBot::m_bCutscenePlaying=false;MultiplayerMgr.single=true;
 Level_nLoadedIndex=0;Level_aInfo[0].pszWorldResName="WERMmorbot1";
 CPlayer::m_nPlayerCount=count;gate=CEBox{};tripwire=CTripwire{};
 gate.name=name;gate.m_pTripwire=&tripwire;tripwire.owner=&gate;
 for(int i=0;i<4;i++){bodies[i]=CBot{};bodies[i].matrix.m_vPos={0,0,-3};
  Player_aPlayer[i].m_pEntityCurrent=Player_aPlayer[i].m_pEntityOrig=&bodies[i];}
}
int main() {
 const char* reactor1[]={"door1_triga","door1_trigb","door2_triga","door2_trigb",
  "door3_triga","door3_trigb","door4_triga","bridge1_triga","orb_trig1","orb_trig2"};
 const char* reactor2[]={"door1_triga","door1_trigb","door2_triga","door2_trigb","bridge1_triga","orb_trig1"};
 for(int world=0;world<2;++world)for(int pad=0;pad<(world?6:10);++pad) {
  const char* name=world?reactor2[pad]:reactor1[pad];
  for(int n=2;n<=4;++n)for(int actor=0;actor<n;++actor) {
   reset(n,name);Level_aInfo[0].pszWorldResName=world?"WERRreactr2":"WERRreactr1";
   bodies[actor].matrix.m_vPos={0,0,0};
   require(gate._CoopHoldTripwireEnter(&bodies[actor]) && gate.fired==1 && gate.lastTripper==&bodies[actor],"Reactor floor pad uses its single operator");
   require(!_nCoopHeldTripwires && !CEntity::CoopTripwireWaiting(&bodies[actor]),"Reactor floor pad never waits for partners");
  }
  for(int mode=0;mode<3;++mode) {
   reset(mode==0?1:4,name);Level_aInfo[0].pszWorldResName=world?"WERRreactr2":"WERRreactr1";
   if(mode==1)MultiplayerMgr.single=false;
   if(mode==2)bodies[0].dead=true;
   require(mode==2 ? gate._CoopHoldTripwireEnter(&bodies[0])&&!gate.fired : !gate._CoopHoldTripwireEnter(&bodies[0]),"Reactor pad retains solo/PvP and rejects dead operators");
  }
 }
 for(const char* world:{"WERRreactr1","WERRreactr2"})for(const char* name:{"elv_trig1","bitch_trig2","swarm_hall02","levelend","door4_trigb","door1_triga_extra"}) {
  reset(4,name);Level_aInfo[0].pszWorldResName=world;bodies[0].matrix.m_vPos={0,0,0};
  gate._CoopHoldTripwireEnter(&bodies[0]);require(!gate.fired,"unlisted Reactor triggers retain team gather");
 }
 reset(2,"door1_triga");Level_aInfo[0].pszWorldResName="WERRreactr1";
 gate._CoopHoldTripwireEnter(&bodies[0]);
 gate.name="door1_trigb";gate._CoopHoldTripwireEnter(&bodies[1]);
 require(gate.fired==2&&!_nCoopHeldTripwires,"players separated across both door pads can each operate it");
 for(int world=0;world<2;world++)for(int n=2;n<=4;n++)for(int actor=0;actor<n;actor++) {
  const char* pads[]={"circtrig1","circtrig2","fliptrig","bridge1trig"};
  for(int index=world?3:0;index<(world?4:3);++index) {
   const char* pad=pads[index];
   reset(n,pad);Level_aInfo[0].pszWorldResName=world?"WERMmorbot2":"WERMmorbot1";
   bodies[actor].matrix.m_vPos={0,0,0};
   require(gate._CoopHoldTripwireEnter(&bodies[actor]) && gate.fired==1 && gate.lastTripper==&bodies[actor],"one player activates floor pad with actual actor");
   require(!_nCoopHeldTripwires && !CEntity::CoopTripwireWaiting(&bodies[actor]),"floor pad never parks player or holds team");
  }
 }
 reset(4,"circtrig1");Level_aInfo[0].pszWorldResName="other_mission";
 bodies[0].matrix.m_vPos={0,0,0};gate._CoopHoldTripwireEnter(&bodies[0]);
 require(gate.fired==0,"same name in another mission remains a team gate");
 reset(4,"bridge1trig");bodies[0].matrix.m_vPos={0,0,0};
 gate._CoopHoldTripwireEnter(&bodies[0]);
 require(!gate.fired,"Morbot 1 bridge still gathers the team");
 for(const char* name:{"bridge2trig","jumper_trig","endlevel","trig_3pred1"}) {
  reset(4,name);Level_aInfo[0].pszWorldResName="WERMmorbot2";
  bodies[0].matrix.m_vPos={0,0,0};gate._CoopHoldTripwireEnter(&bodies[0]);
  require(!gate.fired,"other Morbot 2 triggers still gather the team");
 }
 for(int count:{1,4}) {
  reset(count,"bridge1trig");Level_aInfo[0].pszWorldResName="WERMmorbot2";
  if(count==4)MultiplayerMgr.single=false;
  require(!gate._CoopHoldTripwireEnter(&bodies[0]),"bridge beam keeps retail solo/PvP handling");
 }
 reset(4,"circtrig1");MultiplayerMgr.single=false;
 require(!gate._CoopHoldTripwireEnter(&bodies[0]),"PvP retains retail tripwire handling");
 reset(1,"circtrig1");require(!gate._CoopHoldTripwireEnter(&bodies[0]),"solo retains retail tripwire handling");
 reset(4,"circtrig1");bodies[0].dead=true;
 require(gate._CoopHoldTripwireEnter(&bodies[0]) && !gate.fired,"dead body cannot activate floor pad");
 for(int index:{-1,LEVEL_SINGLE_PLAYER_COUNT}) {Level_nLoadedIndex=index;require(!_CoopFloorSwitchTripwire("circtrig1"),"invalid level cannot bypass a gate");}

 for(int n=2;n<=4;n++)for(int first=0;first<n;first++) {
  reset(n,"save03");bodies[first].matrix.m_vPos={0,0,0};
  require(gate._CoopHoldTripwireEnter(&bodies[first]),"checkpoint handled");
  require(gate.fired==1 && !gate.inWorld && !_nCoopHeldTripwires,"any player completes checkpoint enter");
  CEntity::CoopTripwireWork();require(gate.fired==1,"one-shot checkpoint never fires twice");
 }
 reset(2,"bonusbuddysave");bodies[0].dead=true;
 require(gate._CoopHoldTripwireEnter(&bodies[1]) && gate.lastTripper==&bodies[1],"dead P1 checkpoint uses living P2");
 const char* positive[]={"save01","SAVE08","check02","checkpoint1","checkpoint_trigger03",
  "trig_checkpoint3","trigger_checkpoint2","hall_checkpoint1","savepoint1","m3check05trigger","checksnaptrigger"};
 for(auto p:positive)require(_CoopCheckpointTripwire(p),"retail checkpoint name recognized");
 const char* negative[]={nullptr,"saveus","save03_extra","check","trigger_btr01","levelend","endlevel","save_hall2"};
 for(auto p:negative)require(!_CoopCheckpointTripwire(p),"objective/terminal remains a team gate");
 for(auto name:{"trigger_btr01","levelend"}) {
  reset(4,name); CEntity::CoopTripwireWork();
  bodies[0].matrix.m_vPos={0,0,0};gate._CoopHoldTripwireEnter(&bodies[0]);
  require(gate.fired==0,"shared gate waits for missing partners");
  if(!std::strcmp(name,"levelend"))require(CEntity::CoopTripwireExitWaiting(&bodies[0]),"zipline exit still parks early player");
  bodies[1].matrix.m_vPos={3,0,3};CEntity::CoopTripwireWork();
  require(gate.fired==0,"nearby non-crossing player cannot complete gate");
  bodies[1].matrix.m_vPos={0,0,-3};CEntity::CoopTripwireWork();
  bodies[1].matrix.m_vPos={0,0,3};bodies[2].matrix.m_vPos={0,0,3};CEntity::CoopTripwireWork();
  require(gate.fired==0,"recovered crossings still wait for fourth player");
  bodies[3].matrix.m_vPos={0,0,3};CEntity::CoopTripwireWork();
  require(gate.fired==1 && !_nCoopHeldTripwires,"missed enter recovered across thin box for all players");
  require(!CEntity::CoopTripwireWaiting(&bodies[0]),"released gate clears wait HUD");
 }
 reset();CEntity::CoopTripwireWork();bodies[0].matrix.m_vPos={0,0,0};gate._CoopHoldTripwireEnter(&bodies[0]);
 CEntity::CoopTripwireReset();require(!_nCoopHeldTripwires && !_apCoopPreviousPlayers[1],"restore resets arrivals and crossing history");
 reset();CEntity::CoopTripwireWork();bodies[0].matrix.m_vPos={0,0,0};gate._CoopHoldTripwireEnter(&bodies[0]);
 bodies[1].matrix.m_vPos={0,0,30};CEntity::CoopTripwireWork();require(gate.fired==0,"large teleport cannot claim intervening gates");
 reset();bodies[1].dead=true;bodies[1].matrix.m_vPos={0,0,0};
 require(!(gate._CoopPlayersInsideMask()&2),"downed body cannot supply an arrival");
 for(int first=0;first<4;first++) {
  reset(4);bodies[first].matrix.m_vPos={0,0,0};gate._CoopHoldTripwireEnter(&bodies[first]);
  FLoop_nTotalLoopTicks=1000;
  require(!CEntity::CoopTripwireWaitMessage(&bodies[first]),"new wait never flashes instantly");
  FLoop_nTotalLoopTicks=1200;
  require(!CEntity::CoopTripwireWaitMessage(&bodies[first]),"brief wait stays hidden");
  FLoop_nTotalLoopTicks=1851;
  require(CEntity::CoopTripwireWaitMessage(&bodies[first]),"persistent wait becomes visible");
  require(!CEntity::CoopTripwireWaitMessage(&bodies[(first+1)%4]),"partner's timer is independent");
  bodies[first].matrix.m_vPos={0,0,3};
  require(!CEntity::CoopTripwireWaitMessage(&bodies[first]),"leaving gate clears message immediately");
  bodies[first].matrix.m_vPos={0,0,0};FLoop_nTotalLoopTicks=2000;
  require(!CEntity::CoopTripwireWaitMessage(&bodies[first]),"reentry starts fresh delay");
  FLoop_nTotalLoopTicks=100;
  require(!CEntity::CoopTripwireWaitMessage(&bodies[first]),"rewound clock cannot underflow timer");
  FLoop_nTotalLoopTicks=951;
  require(CEntity::CoopTripwireWaitMessage(&bodies[first]),"wait works after clock rewind");
  CBot::m_bCutscenePlaying=true;
  require(!CEntity::CoopTripwireWaitMessage(&bodies[first]),"cutscene clears wait prompt");
  CBot::m_bCutscenePlaying=false;
  require(!CEntity::CoopTripwireWaitMessage(&bodies[first]),"scene end starts a fresh delay");
  for(int i=0;i<4;i++)bodies[i].matrix.m_vPos={0,0,0};
  CEntity::CoopTripwireWork();FLoop_nTotalLoopTicks=1000;
  require(gate.fired==1&&!CEntity::CoopTripwireWaitMessage(&bodies[first]),"release clears delayed message immediately");
 }
 reset(2,"save03");bodies[0].dead=bodies[1].dead=true;
 require(gate._CoopHoldTripwireEnter(&bodies[0]) && gate.fired==0,"dying bodies cannot advance checkpoint");
 reset(2,"save01");tripwire.m_pszTripwireFilterEntityName="specialbot";
 require(!gate._CoopHoldTripwireEnter(&bodies[0]),"named objective retains retail handling");
 reset(2,"save01");CBot possessed;Player_aPlayer[1].m_pEntityCurrent=&possessed;
 require(!gate._CoopHoldTripwireEnter(&possessed),"possession retains actual actor and timing");
 reset(2,"save01");tripwire.m_eKillMode=CTripwire::TRIPWIRE_KILLMODE_VOLUME;
 require(!gate._CoopHoldTripwireEnter(&bodies[0]),"kill volume is not a checkpoint override");
 CFVec3A a{0,0,-3},b{0,0,3},outside{3,0,3},inside{0,0,0};
 require(gate.TripwireCollisionTest(&a,&b)==0,"crossing does not change kill-volume policy");
 require(gate.TripwireCollisionTest(&a,&inside)&CEntity::TRIPWIRE_COLLFLAG_KILL_NOW,"entering kill volume still kills");
 reset();require(gate.TripwireCollisionTest(&a,&b)==3,"thin box full crossing reports enter and exit");
 require(gate.TripwireCollisionTest(&b,&a)==3,"reverse crossing works");
 require(gate.TripwireCollisionTest(&outside,&outside)==0,"stationary point outside stays outside");
 require(gate.TripwireCollisionTest(&inside,&inside)==4,"stationary point inside stays inside");
 require(gate.TripwireCollisionTest(&a,&inside)==5,"ordinary entry preserved");
 require(gate.TripwireCollisionTest(&inside,&b)==2,"ordinary exit preserved");
 gate.m_MtxToWorld.m_vRight={0,0,1};gate.m_MtxToWorld.m_vFront={-1,0,0};
 gate.m_aCorner_WS[0]={.2f,-2,-2};gate.m_aCorner_WS[1]={-.2f,2,2};
 a={-3,0,0};b={3,0,0};require(gate.TripwireCollisionTest(&a,&b)==3,"rotated/scaled thin box crossing");
 for(int n=2;n<=4;n++) {
  std::array<int,4> order{0,1,2,3};
  do {
   reset(n);tripwire.m_nContainedEntityCount=n;
   for(int i=0;i<n;i++){tripwire.m_ppContainedTrippersArray[i]=&bodies[i];bodies[i].m_nIntersectingTripwireCount=1;
    bodies[i].m_apIntersectingTripwires[0]=&gate;}
   for(int k=0;k<n;k++) {
    bodies[order[k]]._EntityIsOutsideTripwire(&gate);
    require(tripwire.m_nContainedEntityCount==n-k-1,"exactly one occupant removed");
    for(int j=k+1;j<n;j++){
     bool found=false;for(unsigned t=0;t<tripwire.m_nContainedEntityCount;t++)found|=tripwire.m_ppContainedTrippersArray[t]==&bodies[order[j]];
     require(found && bodies[order[j]].m_nIntersectingTripwireCount==1,"remaining players keep reciprocal membership");
    }
   }
  }while(std::next_permutation(order.begin(),order.begin()+n));
 }
 std::printf("PASS: %d offline gate/crossing/occupant checks (2-4 players); game not launched.\n",checks);
}
'''


def main():
    entity = (ROOT / "ma/App/ma/entity.cpp").read_text()
    box = (ROOT / "ma/App/ma/ebox.cpp").read_text()
    start = entity.index("// Local co-op: checkpoint enters")
    end = entity.index("#endif", start)
    code = FIXTURE + entity[start:end]
    code += method(entity, "void CEntity::_EntityIsOutsideTripwire(")
    code += method(box, "u32 CEBox::TripwireCollisionTest(")
    code += method(box, "BOOL CEBox::_PosInside(") + CHECKS
    OUT.mkdir(parents=True, exist_ok=True)
    (OUT / "gate.cpp").write_text(code)
    (OUT / "CMakeLists.txt").write_text(
        "cmake_minimum_required(VERSION 3.20)\nproject(coop_gate_check LANGUAGES CXX)\n"
        "add_executable(coop_gate_check gate.cpp)\ntarget_compile_features(coop_gate_check PRIVATE cxx_std_17)\n")
    for command in (["cmake", "-S", str(OUT), "-B", str(OUT / "out")],
                    ["cmake", "--build", str(OUT / "out"), "--config", "Release"]):
        run = subprocess.run(command, capture_output=True, text=True)
        if run.returncode:
            raise SystemExit(run.stdout + run.stderr)
    executable = OUT / "out/Release/coop_gate_check.exe"
    subprocess.run([str(executable)], check=True)


if __name__ == "__main__":
    main()
