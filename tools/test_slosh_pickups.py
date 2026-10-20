"""Offline production pickup tests: Slosh grenades and unchanged Glitch ammo."""
from pathlib import Path
import re
import subprocess

from test_collectable_skiplist import method

ROOT = Path(__file__).resolve().parent.parent
OUT = ROOT / "build/test-slosh-pickups"

BASE = r'''
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <algorithm>
#include <cfloat>
using u16=unsigned short;using u32=unsigned;using s32=int;using f32=float;using cchar=const char;using BOOL=bool;
#define TRUE true
#define FALSE false
#define FASSERT(x) require(!!(x),"production assertion")
#define DEVPRINTF(...) ((void)0)
#define FMATH_CLAMPMAX(x,m) x=std::min(x,m)
constexpr int ENTITY_BIT_BOTGLITCH=1,ENTITY_BIT_BOTCHEMBOT=2,ENTITY_BIT_BOTMOZER=4,ENTITY_BIT_KRUNK=8;
constexpr int INV_INDEX_PRIMARY=0,INV_INDEX_SECONDARY=1;
constexpr float FMATH_MAX_FLOAT=FLT_MAX,_PICKUP_RADIUS_MULTIPLY=.75f;
int checks=0;
void require(bool ok,const char* why){++checks;if(!ok){std::fprintf(stderr,"FAIL: %s\n",why);std::exit(1);}}
struct CFVec3A{float x=0,y=0,z=0;static CFVec3A m_UnitAxisY;
 void Mul(float s){x*=s;y*=s;z*=s;}void Add(const CFVec3A& v){x+=v.x;y+=v.y;z+=v.z;}
 void Sub(const CFVec3A& v){x-=v.x;y-=v.y;z-=v.z;}float MagSq(){return x*x+y*y+z*z;}};
CFVec3A CFVec3A::m_UnitAxisY{0,1,0};
struct CItemInst;
struct CWeapon {static constexpr u16 INFINITE_AMMO=65535;u16 m_nClipAmmo=0,m_nReserveAmmo=0,maxClip=10,maxReserve=50;
 CItemInst* item=nullptr;bool created=true;u32 level=0;
 bool IsCreated(){return created;}CItemInst* GetItemInst(){return item;}
 bool IsClipFull(){return m_nClipAmmo==maxClip;}bool IsReserveFull(){return m_nReserveAmmo==maxReserve;}
 u16 GetClipAmmo(){return m_nClipAmmo;}u16 GetReserveAmmo(){return m_nReserveAmmo;}u16 GetMaxClipAmmo(){return maxClip;}
 u32 GetUpgradeLevel(){return level;}void SetUpgradeLevel(u32 n){level=n;}
 u16 AddToClip(u16 n=1,BOOL notify=TRUE);void AddToReserve(u16 n){m_nReserveAmmo=std::min<u16>(maxReserve,m_nReserveAmmo+n);}
 void _AmmoMayHaveChanged(BOOL);
};
struct CItemInst {CWeapon* m_pWeapon=nullptr;u16 m_nClipAmmo=0,m_nReserveAmmo=0;int m_nUpgradeLevel=1;
 void NotifyAmmoMightHaveChanged(u16,u16);
};
void CWeapon::_AmmoMayHaveChanged(BOOL){if(item)item->NotifyAmmoMightHaveChanged(m_nClipAmmo,m_nReserveAmmo);}
struct CItem {bool secondary=true;u32 m_uCurLevel=1;bool IsSecondaryOnly(){return secondary;}};
struct CItemRepository {static CItem mag,laser;static CItem* RetrieveEntry(const char* name,void*){return !_stricmp(name,"Magma Bomb")?&mag:&laser;}};
CItem CItemRepository::mag;CItem CItemRepository::laser{false,1};
struct CInventory {CItemInst* item=nullptr;CItemInst* IsWeaponInInventory(const char*){return item;}void SetCurWeapon(u32,CItemInst*,BOOL,BOOL){}};
struct CEntity {int bits=0;int TypeBits(){return bits;}};
struct CBot:CEntity {CInventory* m_pInventory=nullptr;float m_fCollCylinderRadius_WS=1,m_fCollCylinderHeight_WS=2;
 struct {CFVec3A m_vPos;}matrix;bool CanPickupItems(){return true;}bool IsDeadOrDying(){return false;}void* GetCurMech(){return nullptr;}auto MtxToWorld(){return &matrix;}};
struct CBotGlitch:CBot {struct {int m_nWeaponInvCount=1;CWeapon* m_apWeapon[1]{};}m_WeaponInv[2];};
struct CPlayer {static s32 m_nPlayerCount;CEntity* m_pEntityCurrent=nullptr;};
s32 CPlayer::m_nPlayerCount=1;CPlayer Player_aPlayer[4];
struct CHud2 {static CHud2 hud;static CHud2* GetHudForPlayer(int){return &hud;}};CHud2 CHud2::hud;
struct Multiplayer {bool IsSinglePlayer(){return true;}}MultiplayerMgr;
struct CCollectableType {CollectableType_e m_eType;const char* m_pszName;};
CCollectableType magma{COLLECTABLE_WEAPON_MAGMA_BOMB,"Magma Bomb"},flamer{COLLECTABLE_WEAPON_FLAMER_L1,"Flamer L1"};
struct CFWorldMesh {struct{float m_fRadius=1;}m_BoundSphere_MS;};
struct CCollectable {
#if FANG_WINGC
 static BOOL IsWeaponUpgrade(CollectableType_e t){return t>=COLLECTABLE_WEAPON_UPGRADE&&t<COLLECTABLE_WEAPON_UPGRADE_END;}
#endif
 static CBot* m_pCollectBot;static CPlayer* m_pPlayer;static CHud2* m_pCollectHud;
 CCollectableType* m_pCollectableType=&magma;CFWorldMesh* m_pWorldMesh=nullptr;float m_fScale=1;struct{CFVec3A m_vPos{0,1,0};}m_MtxToWorld;
 int m_nRecipientPlayer=-1,m_nPortWasherPlayer=-1;u32 m_nCoopCollectedMask=0;
 static CCollectableType* _FindCollectableType(const char* name){return !_stricmp(name,"Magma Bomb")?&magma:&flamer;}
 static bool PlayerNeeds(CBotGlitch*,CollectableType_e,BOOL){return true;}
 static CWeapon* _FindWeaponUpgrade(CBot*,CCollectableType*,u32&){return nullptr;}
 static BOOL _IsFirstScopePickup(CBot*,CCollectableType*){return false;}
 static BOOL _GiveWeaponAmmoToPlayer(CBot*,CItemInst*,CInventory*,cchar*,s32);
 BOOL _IsPickupAllowed();s32 _CanPickup();bool _IsPersonalWeapon()const{return CPlayer::m_nPlayerCount>1;}
};
CBot* CCollectable::m_pCollectBot=nullptr;CPlayer* CCollectable::m_pPlayer=nullptr;CHud2* CCollectable::m_pCollectHud=nullptr;
'''

CHECKS = r'''
int main(){
 CBot slosh;slosh.bits=ENTITY_BIT_BOTCHEMBOT;CWeapon gun;CItemInst item;CInventory inv;gun.item=&item;item.m_pWeapon=&gun;inv.item=&item;slosh.m_pInventory=&inv;
 CCollectable pickup;CFWorldMesh mesh;pickup.m_pWorldMesh=&mesh;CCollectable::m_pCollectBot=&slosh;
 require(pickup._IsPickupAllowed(),"Slosh allowed to collect Magma Bombs");
 pickup.m_pCollectableType=&flamer;require(!pickup._IsPickupAllowed(),"Slosh pickup restrictions unchanged");pickup.m_pCollectableType=&magma;
 require(CCollectable::_GiveWeaponAmmoToPlayer(&slosh,&item,&inv,"Magma Bomb",1),"grenade grant reports success");
#if !FANG_WINGC
 require(gun.GetClipAmmo()==0&&item.m_nClipAmmo==0,"legacy Slosh pickup reproduced: consumed with no ammo");
#else
 require(gun.GetClipAmmo()==1&&item.m_nClipAmmo==1,"Slosh weapon and HUD inventory receive grenade");
 for(int i=0;i<9;++i)require(CCollectable::_GiveWeaponAmmoToPlayer(&slosh,&item,&inv,"Magma Bomb",1),"successive grenades refill Slosh");
 require(gun.GetClipAmmo()==10&&item.m_nClipAmmo==10,"refill reaches cap");
 require(!CCollectable::_GiveWeaponAmmoToPlayer(&slosh,&item,&inv,"Magma Bomb",1),"full Slosh refuses pickup");
 gun.m_nClipAmmo=9;require(CCollectable::_GiveWeaponAmmoToPlayer(&slosh,&item,&inv,"Magma Bomb",5),"partial refill accepted");require(gun.GetClipAmmo()==10,"refill clamps to actual cap");
 require(!CCollectable::_GiveWeaponAmmoToPlayer(&slosh,&item,&inv,"Flamer L1",1),"unallowed Slosh weapon grant rejected");
 gun.m_nClipAmmo=0;require(!CCollectable::_GiveWeaponAmmoToPlayer(&slosh,&item,&inv,"Magma Bomb",0),"zero ammo does not consume pickup");
 gun.created=false;require(!CCollectable::_GiveWeaponAmmoToPlayer(&slosh,&item,&inv,"Magma Bomb",1),"uncreated gun rejected");gun.created=true;
 item.m_pWeapon=nullptr;require(!CCollectable::_GiveWeaponAmmoToPlayer(&slosh,&item,&inv,"Magma Bomb",1),"missing grenade weapon rejected");item.m_pWeapon=&gun;
 CItemInst wrong;gun.item=&wrong;require(!CCollectable::_GiveWeaponAmmoToPlayer(&slosh,&item,&inv,"Magma Bomb",1),"wrong inventory link rejected");gun.item=&item;
 CBot partner;partner.bits=ENTITY_BIT_BOTCHEMBOT;CWeapon partnerGun;CItemInst partnerItem;CInventory partnerInv;
 partnerGun.item=&partnerItem;partnerItem.m_pWeapon=&partnerGun;partnerInv.item=&partnerItem;partner.m_pInventory=&partnerInv;partner.matrix.m_vPos.x=1;
 Player_aPlayer[0].m_pEntityCurrent=&slosh;Player_aPlayer[1].m_pEntityCurrent=&partner;CPlayer::m_nPlayerCount=2;gun.m_nClipAmmo=10;
 require(pickup._CanPickup()==1,"nearby full Slosh cannot block co-op partner");
 require(CCollectable::_GiveWeaponAmmoToPlayer(&partner,&partnerItem,&partnerInv,"Magma Bomb",1),"P2 Slosh receives own grenade");require(gun.GetClipAmmo()==10&&partnerGun.GetClipAmmo()==1,"correct recipient owns ammo");
 pickup.m_nCoopCollectedMask=2;require(pickup._CanPickup()==-1,"co-op claim cannot collect twice");pickup.m_nCoopCollectedMask=0;
 CPlayer::m_nPlayerCount=1;require(pickup._CanPickup()==-1,"full solo Slosh leaves pickup");gun.m_nClipAmmo=0;require(pickup._CanPickup()==0,"solo Slosh eligible after spending grenades");
 CBotGlitch glitch;glitch.bits=ENTITY_BIT_BOTGLITCH;CWeapon glitchGun;CItemInst glitchItem;CInventory glitchInv;
 glitchItem.m_pWeapon=&glitchGun;glitchGun.item=&glitchItem;glitchInv.item=&glitchItem;glitch.m_pInventory=&glitchInv;glitch.m_WeaponInv[0].m_apWeapon[0]=&glitchGun;glitch.m_WeaponInv[1].m_apWeapon[0]=&glitchGun;
 require(CCollectable::_GiveWeaponAmmoToPlayer(&glitch,&glitchItem,&glitchInv,"Magma Bomb",1),"Glitch secondary ammo still works");require(glitchGun.GetClipAmmo()==1,"Glitch grenade goes to clip");
 require(CCollectable::_GiveWeaponAmmoToPlayer(&glitch,&glitchItem,&glitchInv,"Laser L1",5),"Glitch primary ammo still works");require(glitchGun.GetReserveAmmo()==5,"Glitch primary goes to reserve");
#endif
 std::printf("PASS: %d Slosh pickup checks (PC=%d); game not launched.\n",checks,FANG_WINGC);
}
'''


def main():
    source = (ROOT / "ma/App/ma/collectable.cpp").read_text()
    enum = re.search(r"typedef enum\s*\{.*?\}\s*CollectableType_e;",
                     (ROOT / "ma/App/ma/collectable.h").read_text(), re.S).group()
    code = enum + BASE
    for path, signature in (
        ("ma/App/ma/ItemInst.cpp", "void CItemInst::NotifyAmmoMightHaveChanged("),
        ("ma/App/ma/weapon.cpp", "u16 CWeapon::AddToClip("),
        ("ma/App/ma/collectable.cpp", "BOOL CCollectable::_IsPickupAllowed("),
        ("ma/App/ma/collectable.cpp", "BOOL CCollectable::_GiveWeaponAmmoToPlayer("),
        ("ma/App/ma/collectable.cpp", "s32 CCollectable::_CanPickup("),
    ):
        code += method((ROOT / path).read_text(), signature)
    OUT.mkdir(parents=True, exist_ok=True)
    (OUT / "pickups.cpp").write_text(code + CHECKS)
    (OUT / "CMakeLists.txt").write_text(
        "cmake_minimum_required(VERSION 3.20)\nproject(slosh_pickups LANGUAGES CXX)\n"
        "foreach(pc IN ITEMS 0 1)\nadd_executable(pickups${pc} pickups.cpp)\n"
        "target_compile_features(pickups${pc} PRIVATE cxx_std_17)\n"
        "target_compile_definitions(pickups${pc} PRIVATE FANG_WINGC=${pc})\nendforeach()\n")
    for cmd in (["cmake", "-S", str(OUT), "-B", str(OUT / "out"), "-A", "Win32"],
                ["cmake", "--build", str(OUT / "out"), "--config", "Release"]):
        result = subprocess.run(cmd, capture_output=True, text=True)
        if result.returncode:
            raise SystemExit(result.stdout + result.stderr)
    for pc in (0, 1):
        subprocess.run([str(OUT / f"out/Release/pickups{pc}.exe")], check=True)


if __name__ == "__main__":
    main()
