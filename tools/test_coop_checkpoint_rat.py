"""Offline production-method checks for checkpoint placement, RAT seats and timer HUDs.

Uses analytical floor/wall collision and tracker filtering to reproduce the bot-mesh
placement failure. Does not launch ma_port or read/write player profiles.
"""
from pathlib import Path
import subprocess

ROOT = Path(__file__).resolve().parent.parent
OUT = ROOT / "build/test-coop-checkpoint-rat"


def method(source, signature):
    start = source.index(signature)
    brace = source.index("{", start)
    depth, end = 1, brace + 1
    while depth:
        depth += (source[end] == "{") - (source[end] == "}")
        end += 1
    return source[start:end]


FIXTURE = r'''
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <vector>
#include <algorithm>
using BOOL=int; using s32=int; using u32=unsigned; using u16=unsigned short;
using u64=unsigned long long; using f32=float; using cchar=const char;
constexpr BOOL TRUE=1,FALSE=0;
#define FANG_WINGC 1
#define FASSERT(x) ((void)0)
#define DEVPRINTF(...) ((void)0)
constexpr u64 ENTITY_BIT_BOT=1,ENTITY_BIT_WEAPON=2,ENTITY_BIT_MESHENTITY=4,ENTITY_BIT_BOTGLITCH=8,ENTITY_BIT_DETPACKDROP=16;
constexpr int MESHTYPES_ENTITY=1,FCOLL_CHECK_CB_DO_NOT_CHECK_TRACKER=1,FCOLL_CHECK_CB_ALL_IMPACTS=4;
constexpr int FCOLL_MASK_COLLIDE_WITH_PLAYER=1,FCOLL_DATA_IGNORE_BACKSIDE=16,FCOLL_DATA_FLAGS_NONE=0;
constexpr int GCOLL_SURF_TYPE_NORMAL=0,GCOLL_SURF_TYPE_SLIPPERY=1;
struct CFVec3A {
 float x=0,y=0,z=0; CFVec3A& v3;
 CFVec3A():v3(*this){} CFVec3A(float a,float b,float c):x(a),y(b),z(c),v3(*this){}
 CFVec3A(const CFVec3A& v):x(v.x),y(v.y),z(v.z),v3(*this){}
 CFVec3A& operator=(const CFVec3A& v){x=v.x;y=v.y;z=v.z;return *this;}
 void Set(float a,float b,float c){x=a;y=b;z=c;}
 void Sub(const CFVec3A& a,const CFVec3A& b){Set(a.x-b.x,a.y-b.y,a.z-b.z);}
 void Mul(const CFVec3A& a,float s){Set(a.x*s,a.y*s,a.z*s);}
 CFVec3A& Add(const CFVec3A& a){x+=a.x;y+=a.y;z+=a.z;return *this;}
 float DistSq(const CFVec3A& a)const{return (x-a.x)*(x-a.x)+(y-a.y)*(y-a.y)+(z-a.z)*(z-a.z);}
};
struct CFMtx43A {CFVec3A m_vPos,m_vRight{1,0,0},m_vFront{0,0,1};};
struct CEntity {u64 bits=0;u64 TypeBits(){return bits;} virtual ~CEntity()=default;virtual CFMtx43A* MtxToWorld(){return nullptr;}};
struct CFWorldTracker {int m_nUser=MESHTYPES_ENTITY;void* m_pUser=nullptr;u64 bits=0;u64 GetUserTypeBits(){return bits;}};
struct CFWorldMesh:CFWorldTracker {void SetCollisionFlag(BOOL){};};
struct CBot:CEntity {
 struct BotInfo_Gen_t {float fCollSphere1X_MS=0,fCollSphere1Y_MS=2.8f,fCollSphere1Z_MS=0,fCollSphere1Radius_MS=1.9f;} info;
 BotInfo_Gen_t *m_pBotInfo_Gen=&info;CFWorldMesh mesh,*m_pWorldMesh=&mesh;CFMtx43A matrix;
 static inline BOOL m_bCutscenePlaying=FALSE;
 int m_nPossessionPlayerIndex=-1,team=1;bool dead=false,inWorld=true,switching=false,autoWork=true;
 void* m_pDrivingVehicle=nullptr;
 CBot(){bits=ENTITY_BIT_BOT|ENTITY_BIT_BOTGLITCH;mesh.bits=bits;mesh.m_pUser=this;}
 CFMtx43A* MtxToWorld(){return &matrix;}
 bool visible=true;int relocations=0;void Relocate_RotXlatFromUnitMtx_WS(CFMtx43A* m,BOOL){matrix=*m;++relocations;}
 BOOL IsPlayerBot(){return m_nPossessionPlayerIndex>=0;}BOOL IsDeadOrDying(){return dead;}
 BOOL IsInWorld(){return inWorld;}BOOL SwitchingWeapons(){return switching;}
 BOOL IsSameTeam(CBot* b){return team==b->team;}
 void RemoveFromWorld(){inWorld=false;}void EnableAutoWork(BOOL b){autoWork=b;}
 void ImmobilizeBot(){}void DrawEnable(BOOL b,BOOL){visible=b;}
 int HealthContainerCount(){return 3;}float NormHealth(){return .9f;}
};
struct CWeapon:CEntity {CBot* owner=nullptr;CWeapon(){bits=ENTITY_BIT_WEAPON|32;}CBot* GetOwner(){return owner;}};
struct CMeshEntity:CEntity {bool vehicleOnly=false,visible=true;CMeshEntity(){bits=ENTITY_BIT_MESHENTITY;}BOOL IsVehicleCollOnly(){return vehicleOnly;}void DrawEnable(BOOL b,BOOL){visible=b;}};
struct CFCollData {int nCollMask=0,nFlags=0;u64 nTrackerUserTypeBitsMask=0;CFWorldTracker* pLocationHint=nullptr;
 CFVec3A* pMovement=nullptr;u32(*pCallback)(CFWorldTracker*)=nullptr;};
struct CFSphere {CFVec3A m_Pos;float m_fRadius=0;};
struct FCollImpact_t {float fUnitImpactTime=0;CFVec3A UnitFaceNormal,ImpactPoint;int nUserType=0;};
FCollImpact_t FColl_aImpactBuf[32];unsigned FColl_nImpactCount=0;
struct CGColl {static int GetSurfaceType(FCollImpact_t* p){return p->nUserType;}};
float fmath_Abs(float x){return std::abs(x);}
std::vector<CFWorldTracker*> trackers;
bool floorExists=true,wall=false;int floorSurface=0;float floorY=0;
void fcoll_Clear(){FColl_nImpactCount=0;}
void impact(float t,float x,float y,float z,int surface=0,bool upright=true){auto& a=FColl_aImpactBuf[FColl_nImpactCount++];
 a.fUnitImpactTime=t;a.ImpactPoint.Set(x,y,z);a.UnitFaceNormal.Set(upright?0:1,upright?1:0,0);a.nUserType=surface;}
BOOL fcoll_Check(CFCollData* d,CFSphere* s){
 const auto& p=s->m_Pos;float dy=d->pMovement?d->pMovement->y:0;
 if(floorExists){float bottom=p.y-s->m_fRadius;
  if(bottom<floorY)impact(-1,p.x,floorY,p.z,floorSurface);
  else if(dy<0&&bottom+dy<=floorY)impact((floorY-bottom)/dy,p.x,floorY,p.z,floorSurface);
 }
 if(wall&&p.x+s->m_fRadius>2&&p.x-s->m_fRadius<2)impact(-1,2,p.y,p.z,0,false);
 if(wall&&d->pMovement&&p.x+s->m_fRadius<2&&p.x+d->pMovement->x+s->m_fRadius>=2)
  impact(.5f,2,p.y,p.z,0,false);
 for(auto* tracker:trackers){
  // The production prefilter admits any matching bit, rather than excluding bot inheritance.
  if(!(tracker->GetUserTypeBits()&d->nTrackerUserTypeBitsMask))continue;
  if(d->pCallback&&(d->pCallback(tracker)&FCOLL_CHECK_CB_DO_NOT_CHECK_TRACKER))continue;
  // A partner mesh/equipped weapon overlaps the sweep's starting body sphere.
  if(p.x*p.x+p.z*p.z<4&&p.y<4)impact(-1,p.x,p.y,p.z);
 }
 return FColl_nImpactCount!=0;
}
struct CHud2 {enum{ICON_TIMER_TYPE_RACE=1,DRAW_ICON_TIMER=2};float* timer=nullptr;int flags=0,calls=0;
 void SetIconTimerDraw(int,BOOL b,float* t=nullptr){timer=b?t:nullptr;calls++;}
 void AddDrawFlags(int b){flags|=b;}void ClearDrawFlags(int b){flags&=~b;}
};
struct CPlayer {static inline int m_nPlayerCount=2;static CPlayer* m_pCurrent;
 CEntity *m_pEntityCurrent=nullptr,*m_pEntityOrig=nullptr;CHud2 m_Hud;
 static BOOL CoopReviveForCheckpoint();static void CoopPlaceStartingPartners(BOOL bScriptedStart=FALSE,s32 nLeadIndex=0);};
CPlayer Player_aPlayer[4];CPlayer* CPlayer::m_pCurrent=&Player_aPlayer[1];
struct MP {bool coop=true;BOOL IsLocalCoop(){return coop;}}MultiplayerMgr;
bool racing=true;BOOL level_IsRacingLevel(){return racing;}
struct CVehicle {enum Station_e {STATION_DRIVER,STATION_GUNNER};enum StationStatus_e {
 STATION_STATUS_WRONG_BOT,STATION_STATUS_NO_STATION,STATION_STATUS_EMPTY,STATION_STATUS_OUT_OF_RANGE,
 STATION_STATUS_WEAPON_SWITCH,STATION_STATUS_UNAVAILABLE,STATION_STATUS_ENTERING};};
struct CVehicleRat:CVehicle {
 enum{RAT_FLAG_ZOBBY_DRIVING=0x100,RAT_FLAG_ZOBBY_GUNNING=0x200,RAT_FLAG_COOP_GUNNER_BOARDED=0x400,RAT_FLAG_COOP_GUNNER_PENDING=0x800,
 VEHICLERAT_GUN_STATE_UNOCCUPIED,VEHICLERAT_STATE_START_MOVE_DRIVER_TO_ENTRY_POINT,
 VEHICLERAT_GUN_STATE_START_GUNNER_TO_ENTRY_POINT};
 unsigned m_uRatFlags=RAT_FLAG_ZOBBY_GUNNING;CBot *m_pDriverBot=nullptr,*m_pGunnerBot=nullptr;
 CMeshEntity* m_pZobbyME=nullptr;int m_eGunState=VEHICLERAT_GUN_STATE_UNOCCUPIED,m_eRatState=0;
 float m_fGunStateTimer=0;bool dead=false,available=true;int boards=0,cameraPlayer=-1,m_nPossessionPlayerIndex=-1;
 float health=.25f;int healthCopies=0;
 struct CameraTrans{void SetTransitionSnap(){}}m_DriverCameraTrans,m_GunnerCameraTrans;
 BOOL IsDeadOrDying(){return dead;}const char* Name(){return "rat_glitch";}
 StationStatus_e CanOccupyStation(CBot*,Station_e){return available?STATION_STATUS_EMPTY:STATION_STATUS_UNAVAILABLE;}
 StationStatus_e EnterStation(CBot*,Station_e,BOOL,BOOL);
 void _PortCoopGunnerWork();
 void testDriverHealth(CBot*);void testGunnerHealth();void testDriverIndex(CBot*);
 CBot* GetGunnerBot(){return m_pGunnerBot;}
 void SetHealthContainerCount(int){}void SetNormHealth(float h){health=h;healthCopies++;}
 void _StartDriverEnterWork(BOOL){}void CrankEngine(){}void StartEngine(){}void SetHatchOpen(BOOL,BOOL){}
 void StartTransitionToDriverVehicleCamera(int i){cameraPlayer=i;}
 void StartTransitionToGunnerVehicleCamera(int i){cameraPlayer=i;}
 void _StartGunnerEnterWork(BOOL){boards++;m_pGunnerBot->m_pDrivingVehicle=this;m_eGunState=100;}
};
constexpr float _GUN_TRANSITION_TIME=1;
BOOL _bSaveCheckpoint=FALSE,_bRestoreCheckpoint=FALSE,_bPrintText=FALSE,_bCheckpointReviveDeferred=FALSE;
int _nCheckpoint=1,saves=0,restores=0;CFSphere dummy;
CBot* reviveBot=nullptr;CBot* revivePartner=nullptr;
void _checkpoint_Save(int,BOOL){saves++;}void _checkpoint_Restore(int,BOOL){restores++;}
'''

CHECKS = r'''
BOOL CPlayer::CoopReviveForCheckpoint(){CFVec3A pos;return _CoopCheckRespawnPosition(reviveBot,revivePartner,&pos);}
int checks=0;
void require(bool b,const char* m){checks++;if(!b){std::printf("FAIL: %s\n",m);std::exit(1);}}
int main(){
 for(int count=2;count<=4;count++){
  CBot starts[4];CPlayer::m_nPlayerCount=count;wall=true;trackers.clear();floorExists=true;floorSurface=0;
  for(int i=0;i<count;i++){Player_aPlayer[i].m_pEntityCurrent=&starts[i];starts[i].matrix.m_vPos.Set(i?2:0,0,0);}
  CPlayer::CoopPlaceStartingPartners();
  for(int i=1;i<count;i++){
   require(starts[i].relocations==1&&starts[i].matrix.m_vPos.x<=0,"invalid initial offsets repaired on safe side of arena wall");
   for(int j=1;j<i;j++)require(starts[i].matrix.m_vPos.DistSq(starts[j].matrix.m_vPos)>=14.4f,"starting partners choose distinct safe positions");
  }
  int moved=starts[1].relocations;CPlayer::CoopPlaceStartingPartners();require(starts[1].relocations==moved,"safe initial spawn is left alone");
  starts[1].matrix.m_vPos.Set(2,0,0);floorExists=false;CPlayer::CoopPlaceStartingPartners();require(starts[1].relocations==moved,"no unsafe relocation when every candidate lacks floor");
  floorExists=true;starts[1].m_pBotInfo_Gen=nullptr;CPlayer::CoopPlaceStartingPartners();require(starts[1].relocations==moved,"missing bot collision data cannot crash startup");
  MultiplayerMgr.coop=false;starts[1].m_pBotInfo_Gen=&starts[1].info;CPlayer::CoopPlaceStartingPartners();require(starts[1].relocations==moved,"solo/PvP startup placement unchanged");MultiplayerMgr.coop=true;
 }

 // The boss intro moves its story actor 120 units and four units above the arena floor.
 for(int count=2;count<=4;count++)for(int lead=0;lead<count;lead++){
  CBot starts[4];CPlayer::m_nPlayerCount=count;wall=false;trackers.clear();floorExists=true;floorY=0;floorSurface=0;
  for(int i=0;i<count;i++){Player_aPlayer[i].m_pEntityCurrent=&starts[i];starts[i].matrix.m_vPos.Set(i==lead?0:120,i==lead?4:0,0);}
  CPlayer::CoopPlaceStartingPartners(TRUE,lead);
  for(int i=0;i<count;i++){
   require(starts[i].relocations==(i==lead?0:1),"post-intro placement follows actual story actor once");
   if(i!=lead){require(starts[i].matrix.m_vPos.DistSq(starts[lead].matrix.m_vPos)<80,"partners land inside arena beside relocated lead");
    require(std::abs(starts[i].matrix.m_vPos.y)<.11f,"scripted falling start uses real floor below snap point");
    for(int j=0;j<i;j++)if(j!=lead)require(starts[i].matrix.m_vPos.DistSq(starts[j].matrix.m_vPos)>14,"post-intro partners have separate placements");}
  }
  floorExists=false;int moved=starts[(lead+1)%count].relocations;CPlayer::CoopPlaceStartingPartners(TRUE,lead);
  require(starts[(lead+1)%count].relocations==moved,"scripted placement still refuses void");floorExists=true;
 }
 wall=false;CPlayer::m_nPlayerCount=2;
 CBot partner,downed;CWeapon weapon;weapon.owner=&partner;CFWorldTracker gear;gear.bits=weapon.bits;gear.m_pUser=&weapon;
 trackers={&partner.mesh,&gear};reviveBot=&downed;revivePartner=&partner;
 CFVec3A pos;
 require(!_OriginalRespawnPosition(&downed,&partner,&pos),"original placement rejects safe floor because partner mesh enters query");
 for(int player=0;player<4;player++){
  partner.m_nPossessionPlayerIndex=player;
  for(int surf=0;surf<6;surf++){
   floorSurface=surf;pos.Set(0,0,0);
   require(bool(_CoopCheckRespawnPosition(&downed,&partner,&pos))==(surf<=1),"normal/slippery floor accepted; all damage surfaces rejected");
  }
  floorSurface=0;floorExists=false;pos.Set(0,0,0);
  require(!_CoopCheckRespawnPosition(&downed,&partner,&pos),"void still rejected");floorExists=true;
  floorY=-5;pos.Set(0,0,0);require(!_CoopCheckRespawnPosition(&downed,&partner,&pos),"lower route still rejected");floorY=0;
  wall=true;pos.Set(4.3f,0,0);require(!_CoopCheckRespawnPosition(&downed,&partner,&pos),"crossing wall rejected");wall=false;
  _bSaveCheckpoint=TRUE;floorExists=false;int old=saves;
  checkpoint_Work();checkpoint_Work();require(saves==old&&_bSaveCheckpoint&&_bCheckpointReviveDeferred,"save waits for genuine floor");
  floorExists=true;checkpoint_Work();require(saves==old+1&&!_bSaveCheckpoint&&!_bCheckpointReviveDeferred,"safe revival lets pending save complete");
  _bSaveCheckpoint=TRUE;_bRestoreCheckpoint=TRUE;floorExists=false;old=restores;
  checkpoint_Work();require(restores==old+1&&!_bSaveCheckpoint,"wipe restore takes priority over unsafe pending revive");floorExists=true;
 }
 CMeshEntity vehicleOnly;vehicleOnly.vehicleOnly=true;CFWorldTracker t;t.bits=vehicleOnly.bits;t.m_pUser=&vehicleOnly;
 require(_CoopRespawnTrackerCheck(&t)==FCOLL_CHECK_CB_DO_NOT_CHECK_TRACKER,"vehicle-only collision excluded like normal bot motion");
 vehicleOnly.vehicleOnly=false;require(_CoopRespawnTrackerCheck(&t)==FCOLL_CHECK_CB_ALL_IMPACTS,"solid world prop retained");
 weapon.owner=nullptr;require(_CoopRespawnTrackerCheck(&gear)==FCOLL_CHECK_CB_ALL_IMPACTS,"unowned solid weapon retained");
 for(int count=2;count<=4;count++){
  CBot bots[4],npc;CBot::m_bCutscenePlaying=FALSE;MultiplayerMgr.coop=true;racing=true;
  CPlayer::m_nPlayerCount=count;
  for(int i=0;i<count;i++){bots[i].m_nPossessionPlayerIndex=i;Player_aPlayer[i].m_pEntityOrig=Player_aPlayer[i].m_pEntityCurrent=&bots[i];}
  CVehicleRat rat;rat.m_pDriverBot=&bots[0];
  require(rat.EnterStation(&npc,CVehicle::STATION_GUNNER,FALSE,TRUE)==CVehicle::STATION_STATUS_UNAVAILABLE,
    "scripted friendly NPC seat reserved");
  require(!npc.inWorld&&!npc.autoWork&&!rat.m_pGunnerBot,"NPC object preserved but no longer occupies mission");
  bots[1].switching=true;rat._PortCoopGunnerWork();
  if(count==2)require(rat.boards==0,"transitioning P2 retries instead of interrupted boarding");
  bots[1].switching=false;if(count>2){rat=CVehicleRat();rat.m_pDriverBot=&bots[0];rat.m_uRatFlags|=CVehicleRat::RAT_FLAG_COOP_GUNNER_PENDING;}
  CBot::m_bCutscenePlaying=TRUE;rat._PortCoopGunnerWork();require(rat.boards==0,"no boarding mid scene");
  CBot::m_bCutscenePlaying=FALSE;rat._PortCoopGunnerWork();
  require(rat.m_pGunnerBot==&bots[1]&&rat.cameraPlayer==1&&rat.boards==1,"P2 boards with separate gunner camera");
  require(rat.m_uRatFlags&CVehicleRat::RAT_FLAG_COOP_GUNNER_BOARDED,"boarding state retained in existing checkpoint flags");
  rat.testGunnerHealth();rat.testDriverHealth(&bots[0]);
  require(rat.health==.25f&&rat.healthCopies==0,"boarding/reboarding does not heal shared RAT from second occupant");
  rat.m_nPossessionPlayerIndex=1;rat.testDriverIndex(&bots[0]);
  require(rat.m_nPossessionPlayerIndex==0,"driver reentry retains driver input/boost ownership with human gunner");
  unsigned saved=rat.m_uRatFlags;CVehicleRat restored;restored.m_uRatFlags=saved;restored.m_pDriverBot=&bots[0];
  restored._PortCoopGunnerWork();require(restored.boards==0,"completed initial boarding not repeated on checkpoint restore");
  rat.m_pGunnerBot=nullptr;rat.m_eGunState=CVehicleRat::VEHICLERAT_GUN_STATE_UNOCCUPIED;bots[1].m_pDrivingVehicle=nullptr;
  rat.EnterStation(&npc,CVehicle::STATION_GUNNER,FALSE,TRUE);rat._PortCoopGunnerWork();
  require(rat.boards==1,"voluntary exit not automatically undone or NPC reinstalled");
  require(rat.EnterStation(&bots[1],CVehicle::STATION_GUNNER,TRUE,FALSE)==CVehicle::STATION_STATUS_ENTERING,"manual boarding still available");
  CVehicleRat manual;CMeshEntity decorative;manual.m_pZobbyME=&decorative;bots[1].visible=false;
  manual.m_pDriverBot=&bots[0];manual.m_uRatFlags|=CVehicleRat::RAT_FLAG_COOP_GUNNER_PENDING;
  require(manual.EnterStation(&bots[1],CVehicle::STATION_GUNNER,TRUE,FALSE)==CVehicle::STATION_STATUS_ENTERING&&
   !(manual.m_uRatFlags&CVehicleRat::RAT_FLAG_COOP_GUNNER_PENDING),"manual first boarding consumes automatic offer");
  require(!decorative.visible&&bots[1].visible,"human boarding hides decorative NPC and restores selected character model");
  for(int current=0;current<count;current++){
   CPlayer::m_pCurrent=&Player_aPlayer[current];float timer=256;
   _ScriptTimerHud(&timer,TRUE);
   for(int i=0;i<count;i++)require(Player_aPlayer[i].m_Hud.timer==&timer,"same timer pointer for driver/gunners/spectators");
   for(int i=0;i<count;i++)Player_aPlayer[i].m_Hud.flags=0;
   _ScriptTimerHud(&timer,FALSE);
   for(int i=0;i<count;i++)require(Player_aPlayer[i].m_Hud.flags&CHud2::DRAW_ICON_TIMER,"timer flags repaired after vehicle mode changes");
   _ScriptTimerHud(nullptr,TRUE);
   for(int i=0;i<count;i++)require(!Player_aPlayer[i].m_Hud.timer&&!Player_aPlayer[i].m_Hud.flags,"hide clears all co-op timers");
  }
 }
 MultiplayerMgr.coop=false;CBot npc;CVehicleRat solo;
 require(solo.EnterStation(&npc,CVehicle::STATION_GUNNER,FALSE,TRUE)==CVehicle::STATION_STATUS_ENTERING&&npc.inWorld,"solo NPC gunner unchanged");
 npc.m_nPossessionPlayerIndex=0;solo.testGunnerHealth();require(solo.healthCopies==1,"solo racing occupant health behavior retained");npc.m_nPossessionPlayerIndex=-1;
 MultiplayerMgr.coop=true;racing=false;CVehicleRat ordinary;
 require(ordinary.EnterStation(&npc,CVehicle::STATION_GUNNER,FALSE,TRUE)==CVehicle::STATION_STATUS_ENTERING,"ordinary non-race NPC unaffected");
 racing=true;CVehicleRat enemy;npc.team=2;
 require(enemy.EnterStation(&npc,CVehicle::STATION_GUNNER,FALSE,TRUE)==CVehicle::STATION_STATUS_ENTERING,"enemy gunner unaffected");
 MultiplayerMgr.coop=false;CPlayer::m_pCurrent=&Player_aPlayer[0];float timer=10;_ScriptTimerHud(&timer,TRUE);
 require(Player_aPlayer[0].m_Hud.timer==&timer&&!Player_aPlayer[1].m_Hud.timer,"non-co-op timer remains current player only");
 std::printf("PASS: %d offline checkpoint/RAT/timer checks; game not launched.\n",checks);
}
'''


def main():
    player = (ROOT / "ma/App/ma/player.cpp").read_text()
    rat = (ROOT / "ma/App/ma/vehiclerat.cpp").read_text()
    checkpoint = (ROOT / "ma/App/ma/gamesave.cpp").read_text()
    script = (ROOT / "ma/App/ma/MAScriptTypes.cpp").read_text()
    vehicle = (ROOT / "ma/App/ma/vehicle.cpp").read_text()
    placement = method(player, "static BOOL _CoopCheckRespawnPosition(")
    code = FIXTURE + method(player, "static u32 _CoopRespawnTrackerCheck(") + placement
    code += method(player, "void CPlayer::CoopPlaceStartingPartners(")
    code += placement.replace("_CoopCheckRespawnPosition", "_OriginalRespawnPosition").replace(
        "CollData.pCallback = _CoopRespawnTrackerCheck;", "CollData.pCallback = nullptr;")
    code += method(checkpoint, "void checkpoint_Work(")
    code += method(rat, "CVehicle::StationStatus_e CVehicleRat::EnterStation(")
    code += method(rat, "void CVehicleRat::_PortCoopGunnerWork(")
    code += "void CVehicleRat::testDriverHealth(CBot* pDriverBot){" + method(
        rat, "if( pDriverBot->IsPlayerBot() && level_IsRacingLevel()") + "}\n"
    code += "void CVehicleRat::testGunnerHealth(){" + method(
        rat, "if( m_pGunnerBot->IsPlayerBot() && level_IsRacingLevel()") + "}\n"
    code += "void CVehicleRat::testDriverIndex(CBot* pDriverBot){" + method(
        vehicle, "if( !GetGunnerBot() || !GetGunnerBot()->IsPlayerBot()") + "}\n"
    code += method(script, "static void _ScriptTimerHud(") + CHECKS
    OUT.mkdir(parents=True, exist_ok=True)
    (OUT / "checkpoint_rat.cpp").write_text(code)
    (OUT / "CMakeLists.txt").write_text(
        "cmake_minimum_required(VERSION 3.20)\nproject(coop_checkpoint_rat LANGUAGES CXX)\n"
        "add_executable(coop_checkpoint_rat checkpoint_rat.cpp)\n"
        "target_compile_features(coop_checkpoint_rat PRIVATE cxx_std_17)\n")
    for command in (["cmake", "-S", str(OUT), "-B", str(OUT / "out"), "-A", "Win32"],
                    ["cmake", "--build", str(OUT / "out"), "--config", "Release"]):
        run = subprocess.run(command, capture_output=True, text=True)
        if run.returncode:
            raise SystemExit(run.stdout + run.stderr)
    subprocess.run([str(OUT / "out/Release/coop_checkpoint_rat.exe")], check=True)


if __name__ == "__main__":
    main()
