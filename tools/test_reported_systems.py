"""Offline production-method regressions for servo persistence and recruited pilots."""
from pathlib import Path
import subprocess
from test_collectable_skiplist import method

ROOT = Path(__file__).resolve().parent.parent
OUT = ROOT / "build/test-reported-systems"

BASE = r'''
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <algorithm>
using u32=unsigned;using u8=unsigned char;using u16=unsigned short;using BOOL=bool;
using s32=int;
#define TRUE true
#define FALSE false
#define FMATH_MIN(a,b) std::min(a,b)
#define FASSERT(x) require(!!(x),"production assertion")
#define FASSERT_NOW require(false,"unexpected servo type")
int checks=0;
void require(bool ok,const char* message){checks++;if(!ok){std::fprintf(stderr,"FAIL: %s\n",message);std::exit(1);}}
struct CItemInst {short m_nClipAmmo=1,m_nReserveAmmo=0;void NotifyAmmoMightHaveChanged(u16 c,u16 r){m_nClipAmmo=c;m_nReserveAmmo=r;}};
struct CInventory {CItemInst* servo=nullptr;CItemInst empty;
 CItemInst* IsItemInInventory(const char* tag){require(!_stricmp(tag,"Arm Servo"),"correct persistent servo item");return servo;}
 CItemInst* IsWeaponInInventory(const char*){return &empty;}void SetCurWeapon(int,CItemInst*,BOOL,BOOL){}};
struct WeaponInfo {float fSwitchTimeScale=1,fTetherReloadAnimSpeedMult=1,fTetherPumpAnimSpeedMult=1,fClipReloadAnimSpeedMult=1;};
struct WalkInfo {float fSneakNormVelocity,fMinWalkNormVelocity,fMinRunBlendNormVelocity,fMaxRunBlendNormVelocity,fMaxXlatVelocity,fInvMaxXlatVelocity,fNormVelocityStepSize;
 float fSneakVelocity=1,fMinWalkVelocity=1,fMinRunBlendVelocity=1,fMaxRunBlendVelocity=1;};
struct JumpInfo {float fVerticalVelocityJump1=1,fVerticalVelocityJump2=1;};
struct Config {float afMaxXlatSpeed[3]{1,2,3},afNormVelocityStepSize[3]{1,2,3},afVertVelocityJump1[3]{1,2,3},afVertVelocityJump2[3]{1,2,3};
 float afWeaponSwitchTimeScale[3]{1,1.3f,1.8f},afTetherReloadAnimSpeedMult[3]{1,1.3f,1.8f},afTetherPumpAnimSpeedMult[3]{1,1.3f,1.8f},afClipReloadAnimSpeedMult[3]{1,1.3f,1.8f};};
struct CBotGlitch {enum ServoType_e{SERVO_TYPE_LEGS,SERVO_TYPE_ARMS,SERVO_TYPE_COUNT};enum{SERVO_COUNT=3};
 CInventory* m_pInventory=nullptr;WeaponInfo localWeapon;WeaponInfo* m_pBotInfo_Weapon=&localWeapon;WalkInfo walk;WalkInfo* m_pBotInfo_Walk=&walk;JumpInfo jump;JumpInfo* m_pBotInfo_Jump=&jump;
 static Config m_BotInfo_Glitch;u32 m_anServoLevel[2]{};bool IsCreated(){return true;}
 void SetServoLevel(ServoType_e,u32,BOOL=false);void PortRestoreArmServoFromInventory();
};Config CBotGlitch::m_BotInfo_Glitch;
struct CAIBrain {int race=0,resets=0;};
void ai_SetRace(CAIBrain* brain,u8 race){brain->race=race;}void ai_ResetToJob(CAIBrain* brain){brain->resets++;}
struct CVehicle;
struct CBot {CVehicle* m_pDrivingVehicle=nullptr;u32 team=0;u32 GetTeam(){return team;}};
struct CVehicle {CBot* driver=nullptr;u32 team=4;bool created=true,inWorld=true;int m_nPossessionPlayerIndex=-1;CAIBrain* brain=nullptr;
 bool IsCreated(){return created;}bool IsInWorld(){return inWorld;}CBot* GetDriverBot(){return driver;}void SetTeam(u32 n){team=n;}CAIBrain* AIBrain(){return brain;}};
constexpr int INV_INDEX_PRIMARY=0,INV_INDEX_SECONDARY=1,_BATTLE_NUM_GUYS=2;
struct Fighter {bool dead=false;CInventory inv;CInventory* m_pInventory=&inv;bool IsDead(){return dead;}};
struct CPlayer {static int m_nPlayerCount;};int CPlayer::m_nPlayerCount=2;
struct Mode {bool coop=true;bool IsLocalCoop(){return coop;}}MultiplayerMgr;
struct CHud2 {static CHud2 hud[4];int messages=0;void TextDisplay_Start(float,void*,BOOL,BOOL,const wchar_t*){messages++;}static CHud2* GetHudForPlayer(int n){return &hud[n];}};CHud2 CHud2::hud[4];
struct CSpyVsSpy {static Fighter lead;static bool hudOff,selectionOff;static void TurnOffHUD(BOOL off,BOOL=false,BOOL noSel=false){hudOff=off;selectionOff=noSel;}
 static Fighter* GetGlitch(){return &lead;}static void CoopSyncStage(const char*,const char*,BOOL){}};
Fighter CSpyVsSpy::lead;bool CSpyVsSpy::hudOff=false,CSpyVsSpy::selectionOff=false;
struct CDoorEntity {enum{GOTOREASON_UNKNOWN};bool opened=false;void ForceGotoPos(int pos,int){opened=pos==1;}};
struct CBattleStage {enum{STATE_BATTLE,STATE_WALK_TO_DOOR};int m_eStageState=STATE_BATTLE;Fighter* m_paMiners[2];CDoorEntity* m_pBattleDoor;
 void _BattleWork();};
'''

CHECKS = r'''
int main(){
 CItemInst servo,partnerServo;CInventory inv,partnerInv;inv.servo=&servo;partnerInv.servo=&partnerServo;
 CBotGlitch player,partner;player.m_pInventory=&inv;partner.m_pInventory=&partnerInv;
 player.PortRestoreArmServoFromInventory();require(player.m_anServoLevel[1]==0,"initial servo restores base level");
 player.SetServoLevel(CBotGlitch::SERVO_TYPE_ARMS,1);require(servo.m_nClipAmmo==2,"upgrade persists one-based level in inventory");
 require(player.m_pBotInfo_Weapon->fSwitchTimeScale==1.3f&&partner.m_pBotInfo_Weapon->fSwitchTimeScale==1,"upgrade timing remains player-owned");
 CBotGlitch rebuilt;rebuilt.m_pInventory=&inv;rebuilt.PortRestoreArmServoFromInventory();require(rebuilt.m_anServoLevel[1]==1&&rebuilt.localWeapon.fClipReloadAnimSpeedMult==1.3f,"new level/player body reapplies saved servo timing");
 rebuilt.SetServoLevel(CBotGlitch::SERVO_TYPE_ARMS,2);require(servo.m_nClipAmmo==3,"level three persists");
 CBotGlitch next;next.m_pInventory=&inv;next.PortRestoreArmServoFromInventory();require(next.m_anServoLevel[1]==2&&next.localWeapon.fClipReloadAnimSpeedMult==1.8f,"level three restores");
 servo.m_nClipAmmo=2;next.PortRestoreArmServoFromInventory();require(next.m_anServoLevel[1]==1,"checkpoint inventory rollback reapplies earlier servo");
 servo.m_nClipAmmo=99;next.PortRestoreArmServoFromInventory();require(next.m_anServoLevel[1]==2&&servo.m_nClipAmmo==3,"invalid old servo level clamps to supported range");
 servo.m_nClipAmmo=0;next.PortRestoreArmServoFromInventory();require(next.m_anServoLevel[1]==0&&servo.m_nClipAmmo==1,"zero quantity safely restores base servo");
 next.m_pInventory=nullptr;next.PortRestoreArmServoFromInventory();require(next.m_anServoLevel[1]==0,"NPC/no inventory body keeps default servo");
 for(int i=0;i<100;++i){player.SetServoLevel(CBotGlitch::SERVO_TYPE_ARMS,1,TRUE);partner.SetServoLevel(CBotGlitch::SERVO_TYPE_ARMS,2,TRUE);require(player.localWeapon.fSwitchTimeScale==1.3f&&partner.localWeapon.fSwitchTimeScale==1.8f,"repeated independent servo updates do not leak");}
 CBot pilot;CVehicle vehicle;CAIBrain brain;vehicle.brain=&brain;vehicle.driver=&pilot;pilot.m_pDrivingVehicle=&vehicle;pilot.team=1;
 _SyncRecruitDriverVehicle(&pilot,2);require(vehicle.team==1&&brain.race==2&&brain.resets==1,"recruited pilot updates vehicle team/race and discards hostile target");
 pilot.team=4;_SyncRecruitDriverVehicle(&pilot,1);require(vehicle.team==4&&brain.race==1&&brain.resets==2,"unrecruit restores vehicle allegiance and clears stale target");
 vehicle.m_nPossessionPlayerIndex=0;pilot.team=9;_SyncRecruitDriverVehicle(&pilot,3);require(vehicle.team==4&&brain.resets==2,"human controlled vehicle is unaffected");
 vehicle.m_nPossessionPlayerIndex=-1;vehicle.driver=nullptr;_SyncRecruitDriverVehicle(&pilot,3);require(vehicle.team==4,"former passenger cannot rewrite unrelated vehicle");
 vehicle.driver=&pilot;vehicle.created=false;_SyncRecruitDriverVehicle(&pilot,3);require(vehicle.team==4,"destroyed vehicle is ignored");
 vehicle.created=true;vehicle.brain=nullptr;_SyncRecruitDriverVehicle(&pilot,3);require(vehicle.team==9,"brainless vehicle still receives pilot team");
 vehicle.brain=&brain;vehicle.inWorld=false;_SyncRecruitDriverVehicle(&pilot,3);require(brain.resets==2,"removed vehicle never resets an inactive AI job");
 pilot.m_pDrivingVehicle=nullptr;_SyncRecruitDriverVehicle(&pilot,3);require(vehicle.team==9,"unmounted bot has no vehicle mutation");
 Fighter fighters[2];CDoorEntity exit;CBattleStage battle;battle.m_paMiners[0]=&fighters[0];battle.m_paMiners[1]=&fighters[1];battle.m_pBattleDoor=&exit;
 battle._BattleWork();require(!exit.opened&&CHud2::hud[0].messages==0,"exit guidance does not skip the fight");
 fighters[0].dead=true;fighters[1].dead=true;battle._BattleWork();require(exit.opened&&battle.m_eStageState==CBattleStage::STATE_WALK_TO_DOOR,"victory still opens authored door and advances normal stage");
 require(!CSpyVsSpy::hudOff&&CSpyVsSpy::selectionOff,"victory guidance is visible but unarmed stage cannot select weapons");
 require(CHud2::hud[0].messages==1&&CHud2::hud[1].messages==1,"all co-op players receive exit guidance");
 std::printf("PASS: %d production servo and recruited-vehicle checks; game not launched.\n",checks);
}
'''


def main():
    glitch = (ROOT / "ma/App/ma/botglitch.cpp").read_text()
    recruiter = (ROOT / "ma/App/ma/weapon_recruiter.cpp").read_text()
    spy = (ROOT / "ma/App/ma/SpyVsSpy.cpp").read_text()
    code = "#define FANG_WINGC 1\n" + BASE
    for source, signature in (
        (glitch, "void CBotGlitch::PortRestoreArmServoFromInventory("),
        (glitch, "void CBotGlitch::SetServoLevel("),
        (recruiter, "static void _SyncRecruitDriverVehicle("),
        (spy, "void CBattleStage::_BattleWork("),
    ):
        code += method(source, signature)
    assert "PortRestoreArmServoFromInventory" in method(glitch, "BOOL CBotGlitch::ClassHierarchyBuilt(")
    clear = method(glitch, "void CBotGlitch::_ClearDataMembers(")
    assert "m_pBotInfo_Weapon = &m_PortServoWeaponInfo" in clear
    assert "_SyncRecruitDriverVehicle" in method(recruiter, "void CWeaponRecruiter::_SetAIToRecruitedState(")
    assert "_SyncRecruitDriverVehicle( pBot, pRecruitData->m_nOrigRace )" in recruiter
    assert "CSpyVsSpy::TurnOffHUD( FALSE );" in method(spy, "void CBattleStage::SwitchTo(")
    OUT.mkdir(parents=True, exist_ok=True)
    (OUT / "regression.cpp").write_text(code + CHECKS)
    (OUT / "CMakeLists.txt").write_text(
        "cmake_minimum_required(VERSION 3.20)\nproject(reported_systems LANGUAGES CXX)\n"
        "add_executable(regression regression.cpp)\ntarget_compile_features(regression PRIVATE cxx_std_17)\n")
    for command in (["cmake", "-S", str(OUT), "-B", str(OUT / "out"), "-A", "Win32"],
                    ["cmake", "--build", str(OUT / "out"), "--config", "Release"]):
        result = subprocess.run(command, capture_output=True, text=True)
        if result.returncode:
            raise SystemExit(result.stdout + result.stderr)
    subprocess.run([str(OUT / "out/Release/regression.exe")], check=True)


if __name__ == "__main__":
    main()
