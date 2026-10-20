"""Compile production gameplay methods against deterministic, offline engine fixtures.

Checks damage over time at different frame rates, rebuild weapon queues, health
containers, fresh inventories, flamethrower variants, and mirrored draw culling.
No game launch, save access, or claim of visual/gameplay validation.
"""
from pathlib import Path
import re
import struct
import subprocess

from dol_vocab import Dol
from test_collectable_skiplist import method

ROOT = Path(__file__).resolve().parent.parent
OUT = ROOT / "build/test-reddit-gameplay"

FIXTURE = r'''
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
using BOOL=int;using f32=float;using u32=unsigned;using s32=int;
#define FANG_WINGC 1
#define TRUE 1
#define FALSE 0
#define FASSERT(x) require(!!(x),"production assertion")
#define FASSERT_NOW require(false,"unhandled damage delivery")
#define FMATH_MIN(a,b) std::min(a,b)
#define FMATH_CLAMPMIN(a,b) ((a)=std::max(a,b))
int checks=0;
void require(bool ok,const char* why){++checks;if(!ok){std::fprintf(stderr,"FAIL: %s\n",why);std::exit(1);}}
float FLoop_fPreviousLoopSecs=1.f/60;
struct CFVec3A {float x=0,y=0,z=0;
 struct Raw {float x=0,y=0,z=0;Raw&operator=(const CFVec3A&v){x=v.x;y=v.y;z=v.z;return *this;}}v3;
 void Zero(){x=y=z=0;}void Mul(float a){x*=a;y*=a;z*=a;}
 void Sub(const CFVec3A&a,const CFVec3A&b){x=a.x-b.x;y=a.y-b.y;z=a.z-b.z;}
 void Add(const CFVec3A&a,const CFVec3A&b){x=a.x+b.x;y=a.y+b.y;z=a.z+b.z;}
 float DistSq(const CFVec3A&b)const{return (x-b.x)*(x-b.x)+(y-b.y)*(y-b.y)+(z-b.z)*(z-b.z);}
};
struct CFMtx43A {CFVec3A m_vRight{1,0,0},m_vUp{0,1,0},m_vFront{0,0,1},m_vPos;};
enum FMeshCullDir_e {FMESH_CULLDIR_NONE,FMESH_CULLDIR_CW,FMESH_CULLDIR_CCW};
constexpr int ENTITY_BIT_BOT=1,ENTITY_BIT_BOTGLITCH=2,ENTITY_BIT_VEHICLERAT=4;
bool racing=false;
bool level_IsRacingLevel(){return racing;}
struct CEntity {int bits=ENTITY_BIT_BOT;bool invincible=false;int TypeBits(){return bits;}bool IsInvincible(){return invincible;}};
struct CBot:CEntity {CBot* m_pDrivingVehicle=nullptr;float containers=1;float HealthContainerCount(){return containers;}};
struct CItemInst {const char* name;int ammo,reserve;};
constexpr int CItemInst_nNoAmmo=-1;
void fang_MemZero(void*p,size_t n){std::memset(p,0,n);}
struct CInventory {
 unsigned m_uNumBatteries,m_auNumWeapons[2],m_auCurWeapon[2],m_auSavedWeapon[2],m_auEUKFlags[2];
 CItemInst m_aoWeapons[2][8];int defaultItems;
 void InitItemInst(CItemInst*p,const char*n,int a,int r){p->name=n;p->ammo=a;p->reserve=r;}
 void SetupDefaultItems(){defaultItems=1;}
 void SetToInitial();
};
struct CDamageProfile {float m_fDamagePeriod=0;};
struct CDamageForm {
 enum {DAMAGE_DELIVERY_ONE_SPECIFIC_ENTITY,DAMAGE_DELIVERY_ALL_ENTITIES_WITHIN_PROFILE_RADIUS,
       DAMAGE_DELIVERY_ALL_ENTITIES_WITHIN_SPECIFIC_VOLUME,DAMAGE_DELIVERY_ALL_ENTITIES_WITHIN_SPECIFIC_TRIPWIRE};
 float m_fNormIntensity=1,m_fSecsUntilFormRetired=2,m_fSecsUntilIssuingNextDamage=0;
 CDamageProfile* m_pDamageProfile;CEntity* m_pDamageeEntity;int m_nDamageDelivery=0;
 double accumulated=0;int hits=0;
 void record(){accumulated+=m_fNormIntensity;++hits;}
 void _Work_OneSpecificEntity(){record();}void _Work_ProfileRadius(){record();}
 void _Work_SpecificVolume(){record();}void _Work_SpecificTripwire(){record();}
 BOOL _Work();
};
constexpr int BOTFLAG_PLAY_FIRE1_ANIM=1,THROWSTATE_NONE=0,BOTJUMPSTATE_CABLE=4,MORTAR_STATE_IDLE=0;
struct CWeapon {enum{WEAPON_TYPE_TETHER=3};int type=1;int Type(){return type;}};
struct CWeaponTether:CWeapon {bool attached=false;bool IsTetherAttachedToTarget(){return attached;}};
constexpr int FWORLD_MAX_SKIPLIST_ENTRIES=100,FCOLL_MASK_COLLIDE_WITH_THICK_PROJECTILES=16;
struct CFWorldTracker {};struct CFWorldMesh:CFWorldTracker{};
CFWorldTracker* FWorld_apTrackerSkipList[100];unsigned FWorld_nTrackerSkipListCount=0;
bool blocked=false;int queries=0;
bool fworld_IsLineOfSightObstructed(const CFVec3A*,const CFVec3A*,unsigned n,const CFWorldTracker*const*,CFWorldMesh*,unsigned long long,int mask){
 require(n==1,"each washer query clears the shared collision scratch list");require(mask==16,"washer obstruction matches retail collision mask");++queries;return blocked;}
struct {bool single=true;bool IsSinglePlayer(){return single;}} MultiplayerMgr;
struct CPlayer {static int m_nPlayerCount;CEntity* m_pEntityCurrent=nullptr;};
int CPlayer::m_nPlayerCount=1;CPlayer Player_aPlayer[4];
struct CBotGlitch:CBot {
 bool m_bInPieces=false,facing=true;int m_nBotFlags=0,m_nThrowState=0,m_nJumpState=0,m_nMortarState=0;
 CFMtx43A matrix;CFVec3A head;bool inWorld=true,dead=false,pickup=true,mech=false;
 float m_fCollCylinderHeight_WS=6;
 CBotGlitch(){bits|=ENTITY_BIT_BOTGLITCH;}
 bool IsInWorld(){return inWorld;}bool IsDeadOrDying(){return dead;}bool CanPickupItems(){return pickup;}
 bool IsInPieces(){return m_bInPieces;}bool GetCurMech(){return mech;}
 CFMtx43A* MtxToWorld(){return &matrix;}
 unsigned GetTagPointCount(){return 2;}const CFVec3A* GetTagPoint(unsigned){head=matrix.m_vPos;head.y+=6;return &head;}
 void AppendTrackerSkipList(){}
 struct {unsigned m_nWeaponSwitchToIndex=1,m_nWeaponInvIndex=0;}m_WeaponInv[2];
 CWeapon* m_apWeapon[2];bool IsReverseFacingForward(){return facing;}
 BOOL _IsPrimaryWeaponSwitchQueued();BOOL _IsSecondaryWeaponSwitchQueued();
};
constexpr int COLLECTABLE_WASHER=1,STATE_STOPPED=0,STATE_FLOAT_UP=1,STATE_PLACED=2,STATE_BOUNCE=3;
struct CCollectableType {int m_eType=COLLECTABLE_WASHER;};
struct CCollectable {
 CFWorldMesh mesh,*m_pWorldMesh=&mesh;CCollectableType type,*m_pCollectableType=&type;
 int m_nPortWasherPlayer=-1,m_nRecipientPlayer=-1,m_eState=STATE_STOPPED;
 float m_fPortWasherTime=0;CFVec3A m_PortWasherStart,m_VelocityWS;CFMtx43A m_MtxToWorld;
 BOOL _WorkWasherAttraction();
};
struct CGroundCombat {static void* s_pWD;};void* CGroundCombat::s_pWD=nullptr;
struct CFSphere {CFVec3A m_Pos;};using AttackCompleteCallback_t=void(*)(unsigned);
struct CCorrosiveCombat {
 enum {OVERRIDE_STATE_NONE,OVERRIDE_STATE_ATTACK_LOCATION,OVERRIDE_STATE_FINGER_FLICK,OVERRIDE_STATE_PEER_HERE};
 enum {FINGERFLICK_STATE_GOTOATTACKPOINT,FINGERFLICK_STATE_ATTACKCOMPLETE};
 static CCorrosiveCombat* m_pCorrosiveCombat;
 int m_eOverrideState=0,m_eFingerFlickState=0;bool m_bNewOverrideState=false,m_bAllowEnemyLOSToAbortOverride=false;
 unsigned m_nCallbackUserData=0;AttackCompleteCallback_t m_pAttackCompleteCallback=nullptr;
 CFVec3A m_vPeerHereLocation,m_vOverrideAttackLocation;CFSphere m_FingerFlickSphere;
 static void AttackThisLocation(CFVec3A*,BOOL);static void PeerAtThisLocation(CFVec3A*,BOOL);
 static void DoFingerFlick(const CFSphere&,u32,AttackCompleteCallback_t);static void CancelFingerFlick();
};
CCorrosiveCombat* CCorrosiveCombat::m_pCorrosiveCombat=nullptr;
struct CFlamer {
 const CDamageProfile *m_pDamageProfileHit=nullptr,*m_pDamageProfileBurn=nullptr;
 void SetDamageProfiles(const CDamageProfile*,const CDamageProfile*);
};
constexpr int WEAPONFLAG_CLIP_INSTALLED=1,WEAPON_TYPE_FLAMER=0,INFINITE_AMMO=-1;
struct CWeaponFlamer {
 struct Mesh {int m_Xfm=1;void RemoveFromWorld(){}void UpdateTracker(){}};
 struct _ResourceData_t {Mesh mesh;Mesh*m_pWorldMesh=&mesh;}m_aResourceData[6],*m_pResourceData;
 struct Info_t {}m_aaInfo[1][6];
 struct Props {}m_aUserProps[6],*m_pUserProps;
 unsigned m_nUpgradeLevel=0,m_nWeaponFlags=0;
 CFlamer* m_pFlamer;
 const CDamageProfile* m_apDamageProfileHit[6],*m_apDamageProfileBurn[6];
 bool IsTransitionState(int){return false;}int CurrentState(){return 0;}
 bool IsInWorld(){return true;}
 void _RemoveClipFromWorld(){}void _AddClipToWorld(){}
 void TransferFromReserveToClip(int,BOOL){}
 void ClassHierarchySetUpgradeLevel(u32);
};
'''

CHECKS = r'''
int main(){
 CInventory inv;inv.SetToInitial();
 require(inv.m_uNumBatteries==1,"fresh campaign starts below numbered batteries 2 and 3");
 require(inv.m_auNumWeapons[0]==2&&inv.m_auNumWeapons[1]==1&&inv.defaultItems==1,"fresh inventory still initializes weapons and items");
 require(!std::strcmp(inv.m_aoWeapons[0][1].name,"Laser L1"),"initial weapon remains vanilla");
 inv.m_uNumBatteries=6;CBot glitch,mil,pred,rat;
 glitch.bits|=ENTITY_BIT_BOTGLITCH;pred.containers=2;rat.bits|=ENTITY_BIT_VEHICLERAT;
 require(_HudHealthContainerCount(&inv,&glitch)==6,"Glitch displays collected battery count");
 require(_HudHealthContainerCount(&inv,&mil)==1,"possessed Mil displays one health container");
 require(_HudHealthContainerCount(&inv,&pred)==2,"possessed Predator displays two health containers");
 pred.containers=50;require(_HudHealthContainerCount(&inv,&pred)==6,"invalid container count cannot index outside HUD strip");
 glitch.m_pDrivingVehicle=&rat;racing=true;require(_HudHealthContainerCount(&inv,&glitch)==1,"racing HUD uses vehicle health");racing=false;
 CWeaponTether weapon;CBotGlitch bot;bot.m_apWeapon[0]=&weapon;bot.m_apWeapon[1]=&weapon;
 for(int frame=0;frame<180;++frame){bot.m_bInPieces=true;
  require(!bot._IsPrimaryWeaponSwitchQueued()&&!bot._IsSecondaryWeaponSwitchQueued(),"both weapon queues blocked through complete rebuild");
  require(bot.m_WeaponInv[0].m_nWeaponSwitchToIndex==1&&bot.m_WeaponInv[1].m_nWeaponSwitchToIndex==1,"rebuild preserves pending requests");}
 bot.m_bInPieces=false;require(bot._IsPrimaryWeaponSwitchQueued()&&bot._IsSecondaryWeaponSwitchQueued(),"requests resume after rebuild");
 weapon.type=CWeapon::WEAPON_TYPE_TETHER;weapon.attached=true;
 require(!bot._IsPrimaryWeaponSwitchQueued(),"tether attachment retains existing switch restriction");
 CEntity target;CDamageProfile profile;
 for(int delivery=0;delivery<4;++delivery)for(int fps:{30,60,120,144,240})for(float duration:{.01f,.125f,2.f,4.f}){
  FLoop_fPreviousLoopSecs=1.f/fps;CDamageForm form;
  form.m_pDamageProfile=&profile;form.m_pDamageeEntity=&target;form.m_nDamageDelivery=delivery;form.m_fSecsUntilFormRetired=duration;
  bool retired=false;for(int f=0;f<fps*6&&!retired;++f){retired=form._Work();require(form.m_fNormIntensity==1,"damage integration never compounds stored intensity");}
  require(retired,"finite burning retires");
  require(std::fabs(form.accumulated-60.*duration)<.002,"persistent zero-period damage is independent of rendering rate");
 }
 for(int fps:{30,60,120,240}){
  FLoop_fPreviousLoopSecs=1.f/fps;
  CDamageForm hit;hit.m_pDamageProfile=&profile;hit.m_pDamageeEntity=&target;hit.m_fSecsUntilFormRetired=0;
  require(hit._Work()&&hit.hits==1&&hit.accumulated==1,"instant hits keep original intensity");
  CDamageForm endless;endless.m_pDamageProfile=&profile;endless.m_pDamageeEntity=&target;endless.m_fSecsUntilFormRetired=-1;
  for(int f=0;f<fps;++f)require(!endless._Work(),"infinite form retains lifetime");
  require(std::fabs(endless.accumulated-60)<.001,"infinite zero-period damage integrates actual time");
  profile.m_fDamagePeriod=.25f;CDamageForm timed;timed.m_pDamageProfile=&profile;timed.m_pDamageeEntity=&target;
  while(!timed._Work()){}
  require(timed.accumulated==timed.hits&&timed.hits>=8&&timed.hits<=9,"explicit quarter-second ticks retain original intensity and cadence");profile.m_fDamagePeriod=0;
 }
 CFlamer stream;CWeaponFlamer flamer;flamer.m_pFlamer=&stream;CDamageProfile hits[6],burns[6];
 for(int i=0;i<6;++i){flamer.m_apDamageProfileHit[i]=&hits[i];flamer.m_apDamageProfileBurn[i]=&burns[i];}
 for(unsigned old=0;old<6;++old)for(unsigned next=0;next<6;++next){
  flamer.m_nUpgradeLevel=next;flamer.ClassHierarchySetUpgradeLevel(old);
  require(stream.m_pDamageProfileHit==&hits[next]&&stream.m_pDamageProfileBurn==&burns[next],"every flamer variant receives its own hit and burn profiles");
  require(flamer.m_pResourceData==&flamer.m_aResourceData[next]&&flamer.m_pUserProps==&flamer.m_aUserProps[next],"variant mesh and profiles stay synchronized");
 }
 for(float x:{-2.f,2.f})for(float y:{-3.f,3.f})for(float z:{-4.f,4.f})for(bool rotated:{false,true}){
  CFMtx43A m;if(rotated){m.m_vRight={0,x,0};m.m_vUp={-y,0,0};}else{m.m_vRight={x,0,0};m.m_vUp={0,y,0};}m.m_vFront={0,0,z};
  require(_PortModelCullDir(FMESH_CULLDIR_NONE,m)==FMESH_CULLDIR_NONE,"two-sided meshes remain two-sided");
  require(_PortModelCullDir(FMESH_CULLDIR_CW,m)==(x*y*z<0?FMESH_CULLDIR_CCW:FMESH_CULLDIR_CW),"instance reflection reverses winding once");
  require(_PortModelCullDir(FMESH_CULLDIR_CCW,m)==(x*y*z<0?FMESH_CULLDIR_CW:FMESH_CULLDIR_CCW),"both original winding directions supported");
 }
 CBotGlitch magnet,partner;Player_aPlayer[0].m_pEntityCurrent=&magnet;Player_aPlayer[1].m_pEntityCurrent=&partner;
 partner.matrix.m_vPos.x=100;
 for(int fps:{30,60,120,240}){
  FLoop_fPreviousLoopSecs=1.f/fps;CCollectable washer;washer.m_MtxToWorld.m_vPos={12,2.5f,0};
  FWorld_nTrackerSkipListCount=99;
  for(int frame=0;frame<(int)std::ceil(fps*.45)+1;++frame)require(washer._WorkWasherAttraction(),"retail washer hop progresses without walking into it");
  require(washer.m_nPortWasherPlayer==0&&washer.m_MtxToWorld.m_vPos.DistSq(*magnet.GetTagPoint(1))<.001,"washer reaches Glitch at every tested frame rate");
 }
 for(int reason=0;reason<8;++reason){CCollectable w;w.m_MtxToWorld.m_vPos={12,2.5f,0};
  if(reason==0)w.m_MtxToWorld.m_vPos.x=21;
  if(reason==1)blocked=true;
  if(reason==2)MultiplayerMgr.single=false;
  if(reason==3)magnet.dead=true;
  if(reason==4)magnet.m_bInPieces=true;
  if(reason==5)magnet.mech=true;
  if(reason==6)w.type.m_eType=9;
  if(reason==7)w.m_eState=STATE_BOUNCE;
  require(!w._WorkWasherAttraction()&&w.m_nPortWasherPlayer==-1,"washer respects range, walls, modes, living assembled Glitch and settled-state rules");
  blocked=false;MultiplayerMgr.single=true;magnet.dead=magnet.m_bInPieces=magnet.mech=false;
 }
 CPlayer::m_nPlayerCount=2;magnet.matrix.m_vPos.x=40;partner.matrix.m_vPos.x=0;
 CCollectable w;w.m_MtxToWorld.m_vPos={12,2.5f,0};require(w._WorkWasherAttraction()&&w.m_nPortWasherPlayer==1,"co-op attracts toward nearby P2 instead of distant P1");
 partner.dead=true;require(!w._WorkWasherAttraction()&&w.m_nPortWasherPlayer==-1&&w.m_eState==STATE_PLACED,"death releases washer without granting or losing it");
 CFVec3A location{1,2,3};CFSphere dome;dome.m_Pos=location;
 CCorrosiveCombat::PeerAtThisLocation(&location,FALSE);CCorrosiveCombat::DoFingerFlick(dome,2,nullptr);CCorrosiveCombat::CancelFingerFlick();
 CCorrosiveCombat combat;CCorrosiveCombat::m_pCorrosiveCombat=&combat;
 CCorrosiveCombat::PeerAtThisLocation(&location,FALSE);
 require(combat.m_bNewOverrideState&&combat.m_eOverrideState==CCorrosiveCombat::OVERRIDE_STATE_PEER_HERE,"dome peer request survives outside transient AI update");
 CCorrosiveCombat::DoFingerFlick(dome,7,nullptr);
 require(combat.m_eOverrideState==CCorrosiveCombat::OVERRIDE_STATE_FINGER_FLICK&&combat.m_nCallbackUserData==7,"dome attack queues callback without transient AI update");
 CCorrosiveCombat::CancelFingerFlick();require(combat.m_eFingerFlickState==CCorrosiveCombat::FINGERFLICK_STATE_ATTACKCOMPLETE,"leaving dome cancels pending attack outside AI update");
 std::printf("PASS: %d offline production gameplay checks; no game launched.\n",checks);
}
'''


def main():
    # These values are recovered from retail code/data, not guessed from the
    # source revision being repaired. Keep the proprietary bytes in gamedata/.
    retail = Dol((ROOT / "gamedata/sys/main.dol").read_bytes())
    pointer, scale, radius = struct.unpack_from(">Iff", retail.data, retail.to_off(0x803CA924))
    assert retail.cstr(pointer) == "tfh_cross02" and abs(scale - .15) < 1e-6 and abs(radius - .15) < 1e-6
    assert abs(struct.unpack_from(">f", retail.data, retail.to_off(0x803CA9BC))[0] - .2) < 1e-6
    assert struct.unpack_from(">I", retail.data, retail.to_off(0x8003754C))[0] == 0xC00286FC
    assert abs(struct.unpack_from(">f", retail.data, retail.to_off(0x804C2420 - 0x7904))[0] - .4) < 1e-6
    reticle = (ROOT / "ma/App/ma/reticle.cpp").read_text()
    assert '"tfh_cross02"' in reticle and '0.2f,\t// TYPE_FLAMER' in reticle
    grunt = (ROOT / "ma/App/ma/botgrunt.cpp").read_text()
    assert re.search(r"#if FANG_WINGC\s*(?://[^\n]*\n\s*)*shieldInit\.fShieldScale = 0\.4f;", grunt)
    code = FIXTURE
    signatures = (
        ("ma/App/ma/Damage.cpp", "BOOL CDamageForm::_Work("),
        ("ma/App/ma/Hud2.cpp", "static u32 _HudHealthContainerCount("),
        ("ma/App/ma/ItemInst.cpp", "void CInventory::SetToInitial("),
        ("ma/App/ma/botglitch.cpp", "BOOL CBotGlitch::_IsPrimaryWeaponSwitchQueued("),
        ("ma/App/ma/botglitch.cpp", "BOOL CBotGlitch::_IsSecondaryWeaponSwitchQueued("),
        ("ma/App/ma/weapon_flamer.cpp", "void CWeaponFlamer::ClassHierarchySetUpgradeLevel("),
        ("ma/Lib/Fang2/dx/fdx8mesh.cpp", "static FMeshCullDir_e _PortModelCullDir("),
        ("ma/App/ma/collectable.cpp", "BOOL CCollectable::_WorkWasherAttraction("),
        ("ma/App/ma/Ai/aicorrosive.cpp", "void CCorrosiveCombat::AttackThisLocation("),
        ("ma/App/ma/Ai/aicorrosive.cpp", "void CCorrosiveCombat::PeerAtThisLocation("),
        ("ma/App/ma/Ai/aicorrosive.cpp", "void CCorrosiveCombat::DoFingerFlick("),
        ("ma/App/ma/Ai/aicorrosive.cpp", "void CCorrosiveCombat::CancelFingerFlick("),
    )
    for path, signature in signatures:
        code += method((ROOT / path).read_text(), signature)
    code += method((ROOT / "ma/App/ma/flamer.h").read_text(), "void SetDamageProfiles(").replace(
        "void SetDamageProfiles(", "void CFlamer::SetDamageProfiles(")
    code += CHECKS
    OUT.mkdir(parents=True, exist_ok=True)
    (OUT / "gameplay.cpp").write_text(code)
    (OUT / "CMakeLists.txt").write_text(
        "cmake_minimum_required(VERSION 3.20)\nproject(reddit_gameplay LANGUAGES CXX)\n"
        "add_executable(gameplay gameplay.cpp)\ntarget_compile_features(gameplay PRIVATE cxx_std_17)\n")
    for command in (["cmake", "-S", str(OUT), "-B", str(OUT / "out"), "-A", "Win32"],
                    ["cmake", "--build", str(OUT / "out"), "--config", "Release"]):
        run = subprocess.run(command, capture_output=True, text=True)
        if run.returncode:
            raise SystemExit(run.stdout + run.stderr)
    subprocess.run([str(OUT / "out/Release/gameplay.exe")], check=True)


if __name__ == "__main__":
    main()
