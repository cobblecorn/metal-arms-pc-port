"""Offline checks for campaign cheats; never starts the game or touches saves.

Compiles the production cheat backend, ammo removal, invincibility query and direct
grant context wrapper against small player/inventory fixtures. Asset loading and
pause-menu rendering still need manual gameplay verification.
Run: python tools/test_pc_cheats.py (requires CMake and a C++ compiler).
"""
from pathlib import Path
import subprocess

ROOT = Path(__file__).resolve().parent.parent
OUT = ROOT / "build/test-pc-cheats"


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
#include <cstring>
#include <cwchar>
#include <algorithm>
using BOOL=int; using u32=unsigned; using s32=int; using s16=short; using u16=unsigned short;
using cchar=const char; using cwchar=const wchar_t;
constexpr BOOL TRUE=1,FALSE=0;
constexpr int MAX_PLAYERS=4, INV_INDEX_COUNT=2, ItemInst_uMaxInventoryWeapons=24, INVPOS_WASHER=0;
constexpr unsigned ENTITY_BIT_BOT=1,ENTITY_BIT_BOTGLITCH=2, ENTITY_FLAG_INVINCIBLE=1,ENTITY_FLAG_INFINITE_ARMOR=2;
constexpr u16 INFINITE_AMMO=65535;
#define MA_PC_INPUT 1
#define FASSERT(x) do{if(!(x))std::abort();}while(0)
#define FMATH_MIN(x,y) std::min(x,y)
#define FMATH_MAX(x,y) std::max(x,y)
class CWeapon; class CInventory;
struct CEntity {
 bool inWorld=true; unsigned bits=ENTITY_BIT_BOT|ENTITY_BIT_BOTGLITCH,m_nEntityFlags=0;
 int health=15; bool IsInWorld()const{return inWorld;} unsigned TypeBits()const{return bits;}
 void SetMaxHealth(){health=100;} BOOL IsInvincible()const;
};
struct CBot:CEntity {CInventory* m_pInventory=nullptr; bool dead=false; bool IsDeadOrDying()const{return dead;}};
struct CItem {unsigned m_uCurLevel=1;};
struct CItemInst {
 CWeapon* m_pWeapon=nullptr; CInventory* m_pOwnerInventory=nullptr; CItem* m_pItemData=nullptr;
 short m_nClipAmmo=0; unsigned upgradeLimit=2; const char* name=nullptr;
 unsigned HighestUpgradeAvailable(){return upgradeLimit;}
};
struct CInventory {
 int m_auNumWeapons[2]{}; CItemInst m_aoWeapons[2][24]; CItemInst wallet;
 bool walletPresent=true;
 CItemInst* IsItemInInventory(const char*){return walletPresent?&wallet:nullptr;}
 CItemInst* IsWeaponInInventory(const char* name){
  for(auto& side:m_aoWeapons)for(auto& item:side)if(item.name&&!std::strcmp(item.name,name))return &item;
  return nullptr;
 }
};
struct CPlayer {
 static int m_nPlayerCount,m_nCurrent; CEntity* m_pEntityCurrent=nullptr; CEntity* m_pEntityOrig=nullptr;
 static CInventory* GetInventory(int,BOOL=FALSE);static void SetCurrent(int n){m_nCurrent=n;}
};
int CPlayer::m_nPlayerCount=4,CPlayer::m_nCurrent=-1;
CPlayer Player_aPlayer[4]; CInventory original[4],possessed[4]; CBot bodies[4],posBodies[4],enemy;
CInventory* CPlayer::GetInventory(int n,BOOL possession){return possession?&possessed[n]:&original[n];}
struct {bool single=true;bool IsSinglePlayer(){return single;}} MultiplayerMgr;
const char* ItemInst_apszItemNames[]={"Washers"};
struct CWeapon {
 u16 m_nClipAmmo=4,m_nReserveAmmo=9,clipMax=12,reserveMax=100; unsigned level=0,maxLevel=2;
 bool created=true; CItemInst* item=nullptr; CBot* owner=nullptr;
 const CItemInst* GetItemInst()const{return item;}const CBot* GetOwner()const{return owner;}
 bool IsCreated()const{return created;}u16 GetClipAmmo()const{return m_nClipAmmo;}
 u16 GetReserveAmmo()const{return m_nReserveAmmo;}u16 GetMaxClipAmmo()const{return clipMax;}
 u16 GetMaxReserveAmmo()const{return reserveMax;}
 void SetClipAmmo(u16 n,BOOL){m_nClipAmmo=n;if(item)item->m_nClipAmmo=n;}
 void SetReserveAmmo(u16 n,BOOL){m_nReserveAmmo=n;}
 void _AmmoMayHaveChanged(BOOL){if(item)item->m_nClipAmmo=m_nClipAmmo;}
 unsigned GetMaxUpgradeLevel(){return maxLevel;}void SetUpgradeLevel(unsigned n){level=n;}
 u16 RemoveFromClip(u16,BOOL=TRUE);u16 RemoveFromReserve(u16,BOOL=TRUE);
};
CWeapon guns[4][2][24]; CItem gunData[4][2][24]; CWeapon posGuns[4],enemyGun;
int grantCalls=0;bool grantFailure=false,grantContextOK=true;
struct CHud2{static CHud2* GetHudForPlayer(int n);}; CHud2 huds[4];
CHud2* CHud2::GetHudForPlayer(int n){return n>=0&&n<4?&huds[n]:nullptr;}
s32 _CoopGrantRecipient(const CBot* bot){for(int i=0;i<CPlayer::m_nPlayerCount;i++)
 if(bot==Player_aPlayer[i].m_pEntityOrig||bot==Player_aPlayer[i].m_pEntityCurrent)return i;return -1;}
struct CCollectable {
 static CBot* m_pCollectBot;static CHud2* m_pCollectHud;static CPlayer* m_pPlayer;
 static BOOL GiveWeaponToPlayer(CBot*,const char*,s32=-1);
 static BOOL _GiveWeaponToPlayerScoped(CBot* bot,const char* name,s32){
  ++grantCalls;int n=_CoopGrantRecipient(bot);
  grantContextOK &= m_pCollectBot==bot&&m_pPlayer==&Player_aPlayer[n]&&m_pCollectHud==&huds[n]&&CPlayer::m_nCurrent==n;
  if(grantFailure)return FALSE;
  auto& inv=*bot->m_pInventory; int side=0,slot=inv.m_auNumWeapons[side]++;
  auto& item=inv.m_aoWeapons[side][slot];item.name=name;item.m_pItemData=&gunData[n][side][slot];
  item.m_pOwnerInventory=&inv;item.m_pWeapon=&guns[n][side][slot];
  item.m_pWeapon->item=&item;item.m_pWeapon->owner=bot;return TRUE;
 }
};
CBot* CCollectable::m_pCollectBot=nullptr;CHud2* CCollectable::m_pCollectHud=nullptr;CPlayer* CCollectable::m_pPlayer=nullptr;
'''

CHECKS = r'''
int checks=0;
void require(bool value,const char* label){++checks;if(!value){std::printf("FAIL: %s\n",label);std::exit(1);}}
void reset(int count=4){
 pccheats_Reset();CPlayer::m_nPlayerCount=count;CPlayer::m_nCurrent=-1;MultiplayerMgr.single=true;
 grantCalls=0;grantFailure=false;grantContextOK=true;
 for(int n=0;n<4;n++){
  original[n]=CInventory{};possessed[n]=CInventory{};bodies[n]=CBot{};posBodies[n]=CBot{};
  bodies[n].m_pInventory=&original[n];posBodies[n].m_pInventory=&possessed[n];
  Player_aPlayer[n].m_pEntityOrig=Player_aPlayer[n].m_pEntityCurrent=&bodies[n];
  for(int side=0;side<2;side++){
   original[n].m_auNumWeapons[side]=1;
   for(int w=0;w<24;w++){guns[n][side][w]=CWeapon{};gunData[n][side][w]=CItem{};}
   auto& item=original[n].m_aoWeapons[side][0];item.name=side==0?"Blaster L1":"Coring Charge";
   item.m_pWeapon=&guns[n][side][0];item.m_pItemData=&gunData[n][side][0];item.m_pOwnerInventory=&original[n];
   item.m_pWeapon->item=&item;item.m_pWeapon->owner=&bodies[n];
  }
  possessed[n].m_auNumWeapons[0]=1;posGuns[n]=CWeapon{};
  auto& item=possessed[n].m_aoWeapons[0][0];item.m_pWeapon=&posGuns[n];item.m_pOwnerInventory=&possessed[n];
  posGuns[n].item=&item;posGuns[n].owner=&posBodies[n];
 }
 enemy=CBot{};enemyGun=CWeapon{};enemyGun.owner=&enemy;
 CCollectable::m_pCollectBot=&enemy;CCollectable::m_pCollectHud=nullptr;CCollectable::m_pPlayer=nullptr;
}
int main(){
 for(int count=1;count<=4;count++)for(int who=0;who<count;who++){
  reset(count);pccheats_Apply(who,PC_CHEAT_INFINITE_AMMO);
  require(pccheats_Enabled(who,PC_CHEAT_INFINITE_AMMO),"infinite ammo enables for target");
  for(int n=0;n<count;n++){
   auto& gun=guns[n][0][0];
   require(bool(pccheats_InfiniteAmmo(&gun))==(n==who),"ammo scoped to one player");
   auto before=gun.m_nClipAmmo;require(gun.RemoveFromClip(1)==1,"shot reports consumed round");
   require(gun.m_nClipAmmo==before-(n!=who),"only target avoids consumption");
   require(gun.m_nClipAmmo<INFINITE_AMMO,"ammo remains finite for serialization");
   require(original[n].wallet.m_nClipAmmo==0,"ammo cheat leaves wallets unchanged");
  }
  auto& gun=guns[who][0][0];auto reserve=gun.m_nReserveAmmo;
  gun.RemoveFromReserve(3);require(gun.m_nReserveAmmo==reserve,"reserve protected");
  gun.RemoveFromClip(INFINITE_AMMO);require(gun.m_nClipAmmo==0,"explicit clear still removes clip");
  gun.RemoveFromReserve(INFINITE_AMMO);require(gun.m_nReserveAmmo==0,"explicit clear still removes reserve");
  pccheats_Work();require(gun.m_nClipAmmo==gun.clipMax&&gun.m_nReserveAmmo==gun.reserveMax,"work replenishes emptied/new weapons");
  pccheats_Apply(who,PC_CHEAT_INFINITE_AMMO);gun.RemoveFromClip(1);gun.RemoveFromReserve(2);
  require(gun.m_nClipAmmo==gun.clipMax-1&&gun.m_nReserveAmmo==gun.reserveMax-2,"toggle off restores both consumption paths");
  enemyGun.RemoveFromClip(1);require(enemyGun.m_nClipAmmo==3,"enemy ammo unaffected");
  pccheats_Apply(who,PC_CHEAT_INVULNERABLE);
  for(int n=0;n<count;n++)require(bool(bodies[n].IsInvincible())==(n==who),"invulnerability scoped to target");
  require(!enemy.IsInvincible(),"enemy protection unchanged");
  pccheats_Apply(who,PC_CHEAT_INVULNERABLE);require(!bodies[who].IsInvincible(),"protection turns off");
  bodies[who].m_nEntityFlags=ENTITY_FLAG_INVINCIBLE;pccheats_Apply(who,PC_CHEAT_INVULNERABLE);
  pccheats_Reset();require(bodies[who].IsInvincible(),"reset retains native scripted invincibility");
  bodies[who].m_nEntityFlags=0;require(!bodies[who].IsInvincible()&&!pccheats_Enabled(who,PC_CHEAT_INFINITE_AMMO),"session reset clears cheat masks");
  pccheats_Apply(who,PC_CHEAT_WASHERS);
  for(int n=0;n<count;n++)require(original[n].wallet.m_nClipAmmo==(n==who?1000:0),"washer grant uses individual wallet");
  original[who].wallet.m_nClipAmmo=32760;pccheats_Apply(who,PC_CHEAT_WASHERS);
  require(original[who].wallet.m_nClipAmmo==32767,"washer clamp prevents signed overflow");
  pccheats_Apply(who,PC_CHEAT_WASHERS);require(original[who].wallet.m_nClipAmmo==32767,"full wallet stays full");
  pccheats_Apply(who,PC_CHEAT_HEAL);require(bodies[who].health==100,"heal target");
  bodies[who].health=0;bodies[who].dead=true;pccheats_Apply(who,PC_CHEAT_HEAL);
  require(bodies[who].health==0&&bodies[who].dead,"heal does not resurrect unsafe body");
  bodies[who].dead=false;bodies[who].inWorld=false;pccheats_Apply(who,PC_CHEAT_HEAL);
  require(bodies[who].health==0,"heal rejects removed actor");
 }
 reset();Player_aPlayer[2].m_pEntityCurrent=&posBodies[2];pccheats_Apply(2,PC_CHEAT_INFINITE_AMMO);
 require(pccheats_InfiniteAmmo(&posGuns[2])&&posGuns[2].m_nClipAmmo==12,"possessed gun uses target ammo setting");
 pccheats_Apply(2,PC_CHEAT_INVULNERABLE);require(posBodies[2].IsInvincible()&&bodies[2].IsInvincible(),"possession protects target's two bodies");
 pccheats_Apply(2,PC_CHEAT_WASHERS);require(original[2].wallet.m_nClipAmmo==1000&&possessed[2].wallet.m_nClipAmmo==0,"possessed grant still uses permanent wallet");
 pccheats_Apply(2,PC_CHEAT_WEAPONS);require(!grantCalls,"weapon grant waits for original Glitch");
 posBodies[2].health=2;pccheats_Apply(2,PC_CHEAT_HEAL);require(posBodies[2].health==100,"heal current possessed body");
 reset();pccheats_Apply(3,PC_CHEAT_WEAPONS);
 require(grantCalls==13&&grantContextOK,"missing weapon grants use P4 context and skip owned weapons");
 require(CPlayer::m_nCurrent==-1&&CCollectable::m_pCollectBot==&enemy&&!CCollectable::m_pCollectHud&&!CCollectable::m_pPlayer,"grant restores global player and pickup context");
 require(original[0].m_auNumWeapons[0]==1&&original[3].m_auNumWeapons[0]==14,"weapon grants don't touch P1 inventory");
 for(int i=0;i<original[3].m_auNumWeapons[0];i++){
  auto& item=original[3].m_aoWeapons[0][i];require(item.m_pWeapon->m_nClipAmmo==item.m_pWeapon->clipMax,"granted weapons refill");
  require(!std::strstr(item.name,"Nuke")&&!std::strstr(item.name,"Water"),"experimental grenades remain excluded");
 }
 pccheats_Apply(3,PC_CHEAT_WEAPONS);require(grantCalls==13,"repeated grant creates no duplicates");
 guns[3][0][0].maxLevel=3;original[3].m_aoWeapons[0][0].upgradeLimit=1;pccheats_Apply(3,PC_CHEAT_UPGRADES);
 require(guns[3][0][0].level==1&&guns[3][0][1].level==2&&guns[0][0][0].level==0,"upgrade respects available variant and player ownership");
 reset();grantFailure=true;auto msg=pccheats_Apply(1,PC_CHEAT_WEAPONS);
 require(std::wcsstr(msg,L"some aren't loaded")&&original[1].m_auNumWeapons[0]==1,"missing assets report partial availability");
 require(CPlayer::m_nCurrent==-1&&CCollectable::m_pCollectBot==&enemy,"failed grant also restores context");
 reset();MultiplayerMgr.single=false;
 for(int action=0;action<PC_CHEAT_ACTION_COUNT;action++)pccheats_Apply(0,(PcCheatAction_e)action);
 require(!grantCalls&&!pccheats_Enabled(0,PC_CHEAT_INVULNERABLE)&&original[0].wallet.m_nClipAmmo==0,"competitive multiplayer rejects cheats");
 reset();for(int target:{-1,4,32})for(int action=0;action<PC_CHEAT_ACTION_COUNT;action++)pccheats_Apply(target,(PcCheatAction_e)action);
 require(!grantCalls&&!pccheats_Enabled(0,PC_CHEAT_INFINITE_AMMO)&&original[0].wallet.m_nClipAmmo==0,"invalid player rejected without mutations");
 reset();pccheats_Apply(1,PC_CHEAT_INFINITE_AMMO);pccheats_Apply(3,PC_CHEAT_INFINITE_AMMO);pccheats_Apply(1,PC_CHEAT_INFINITE_AMMO);
 require(!pccheats_InfiniteAmmo(&guns[1][0][0])&&pccheats_InfiniteAmmo(&guns[3][0][0]),"one player toggling off preserves partner's cheat");
 std::printf("PASS: %d offline campaign cheat checks (P1-P4); game not launched.\n",checks);
}
'''


def main():
    header = (ROOT / "port/pc_cheats.h").read_text()
    enum = header[header.index("enum PcCheatAction_e"):header.index("// Session toggles")]
    backend = (ROOT / "port/pc_cheats.cpp").read_text()
    backend = "\n".join(line for line in backend.splitlines() if not line.startswith("#include"))
    entity = (ROOT / "ma/App/ma/entity.h").read_text()
    query = method(entity, "FINLINE BOOL IsInvincible(").replace("FINLINE BOOL IsInvincible", "BOOL CEntity::IsInvincible")
    weapon = (ROOT / "ma/App/ma/weapon.cpp").read_text()
    collect = (ROOT / "ma/App/ma/collectable.cpp").read_text()
    wrapper = method(collect, "BOOL CCollectable::GiveWeaponToPlayer(") + "\n#endif\n"
    # The wrapper exists only in the PC branch and ends before the retail body.
    code = FIXTURE + enum + query + backend + wrapper
    code += method(weapon, "u16 CWeapon::RemoveFromClip(")
    code += method(weapon, "u16 CWeapon::RemoveFromReserve(") + CHECKS
    OUT.mkdir(parents=True, exist_ok=True)
    (OUT / "cheats.cpp").write_text(code)
    (OUT / "CMakeLists.txt").write_text(
        "cmake_minimum_required(VERSION 3.20)\nproject(pc_cheats_check LANGUAGES CXX)\n"
        "add_executable(pc_cheats_check cheats.cpp)\ntarget_compile_features(pc_cheats_check PRIVATE cxx_std_17)\n")
    for command in (["cmake", "-S", str(OUT), "-B", str(OUT / "out")],
                    ["cmake", "--build", str(OUT / "out"), "--config", "Release"]):
        run = subprocess.run(command, capture_output=True, text=True)
        if run.returncode:
            raise SystemExit(run.stdout + run.stderr)
    subprocess.run([str(OUT / "out/Release/pc_cheats_check.exe")], check=True)


if __name__ == "__main__":
    main()
