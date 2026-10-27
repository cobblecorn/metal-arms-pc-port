"""Offline production shop-purchase tests: nothing received, nothing charged.

Compiles the real CBarterItem::Purchase, CBarterItemInst::Purchase, CCollectable::OwnsWeaponForAmmo,
GiveOwnedWeaponAmmo and GiveToPlayerAndCollect against small stubs, then checks the wallet, the
weapon's reserve/clip/upgrade level, shop stock and what is left in the world.

    python tools/test_shop_purchase.py
"""
from pathlib import Path
import re
import subprocess

from test_collectable_skiplist import method

ROOT = Path(__file__).resolve().parent.parent
OUT = ROOT / "build/test-shop-purchase"

BASE = r'''
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <algorithm>
using u8=unsigned char;using s8=signed char;using u16=unsigned short;using s16=short;using u32=unsigned;using s32=int;using f32=float;using cchar=const char;using BOOL=bool;
#define TRUE true
#define FALSE false
#define FANG_WINGC 1
#define FASSERT(x) require(!!(x),"production assertion")
#define DEVPRINTF(...) ((void)0)
constexpr int ENTITY_BIT_BOTGLITCH=1,INVPOS_WASHER=0,INV_INDEX_PRIMARY=0,INV_INDEX_SECONDARY=1;
constexpr s8 BARTER_INFINITE=-1;
int checks=0;
void require(bool ok,const char* why){++checks;if(!ok){std::fprintf(stderr,"FAIL: %s\n",why);std::exit(1);}}
using FSndFx_FxHandle_t=int;int sounds=0;FSndFx_FxHandle_t fsndfx_GetFxHandle(cchar*){return 1;}void fsndfx_Play2D(FSndFx_FxHandle_t){++sounds;}
struct CFVec3A{float x=0,y=0,z=0;static CFVec3A m_UnitAxisY;void Add(const CFVec3A& v){x+=v.x;y+=v.y;z+=v.z;}};
CFVec3A CFVec3A::m_UnitAxisY{0,1,0};
struct CFMtx43A{CFVec3A m_vPos;void Identity(){m_vPos=CFVec3A{};}};
struct CItemInst;
struct CWeapon{u16 clip=0,reserve=0,maxClip=10,maxReserve=50;u32 level=0;CItemInst* item=nullptr;
 CItemInst* GetItemInst(){return item;}bool IsClipFull(){return clip>=maxClip;}bool IsReserveFull(){return reserve>=maxReserve;}
 void AddToClip(u16 n){clip=std::min<u16>(maxClip,clip+n);}void AddToReserve(u16 n){reserve=std::min<u16>(maxReserve,reserve+n);}
 u16 GetClipAmmo(){return clip;}u16 GetReserveAmmo(){return reserve;}};
struct CItemInst{u16 m_nClipAmmo=0,m_nReserveAmmo=0;void NotifyAmmoMightHaveChanged(u16 c,u16 r){m_nClipAmmo=c;m_nReserveAmmo=r;}};
struct CItem{bool secondary=false;bool IsSecondaryOnly(){return secondary;}};
struct CItemRepository{static CItem rivet,grenade;static CItem* RetrieveEntry(cchar* n,void*){return !strcmp(n,"Rivet Gun")?&rivet:!strcmp(n,"Coring Charge")?&grenade:nullptr;}};
CItem CItemRepository::rivet,CItemRepository::grenade{true};
struct CInventory{struct{s32 m_nClipAmmo=0;}m_aoItems[1];CItemInst* rivet=nullptr;CItemInst* grenade=nullptr;
 CItemInst* IsWeaponInInventory(cchar* n){return !strcmp(n,"Rivet Gun")?rivet:!strcmp(n,"Coring Charge")?grenade:nullptr;}};
struct CBot{int bits=ENTITY_BIT_BOTGLITCH;CInventory* m_pInventory=nullptr;struct{CFVec3A m_vPos;}matrix;
 int TypeBits(){return bits;}auto MtxToWorld(){return &matrix;}};
struct CBotGlitch:CBot{struct{u32 m_nWeaponInvCount=1;CWeapon* m_apWeapon[2]{};}m_WeaponInv[2];};
struct CHud2{static CHud2 huds[4];static CHud2* GetHudForPlayer(int n){return &huds[n];}};CHud2 CHud2::huds[4];
struct CPlayer{};CPlayer Player_aPlayer[4];
struct CCollectableType{CollectableType_e m_eType;cchar* m_pszName;s32 m_nAmmoCount;};
CCollectableType rivetType{COLLECTABLE_WEAPON_RIVET_GUN_L1,"Rivet Gun",50},chargeType{COLLECTABLE_WEAPON_CORING_CHARGE,"Coring Charge",5},batteryType{COLLECTABLE_BATTERY,"Battery",0};
struct CCollectable{
 static CPlayer* m_pPlayer;static CBot* m_pCollectBot;static CHud2* m_pCollectHud;
 CCollectableType* m_pCollectableType=nullptr;bool inWorld=false;
 static bool IsWeapon(CollectableType_e e){return e>COLLECTABLE_WEAPON_START&&e<COLLECTABLE_WEAPON_END;}
 static CCollectableType* GetCollectableTypeObjectFromType(CollectableType_e e){return e==rivetType.m_eType?&rivetType:e==chargeType.m_eType?&chargeType:e==batteryType.m_eType?&batteryType:nullptr;}
 static CCollectableType* _FindCollectableType(CollectableType_e e){return GetCollectableTypeObjectFromType(e);}
 static BOOL OwnsWeaponForAmmo(CBot*,CollectableType_e);static BOOL GiveOwnedWeaponAmmo(CBot*,CollectableType_e,s32);
 static BOOL GiveToPlayerAndCollect(CBot*,CollectableType_e,f32);
 static BOOL _PlaceIntoWorld(CCollectableType*,const CFMtx43A*,const CFVec3A*,f32,f32,s32,void*,s32,CCollectable**);
 static void ReturnCollectable(CCollectable* p){p->inWorld=false;++returned;}
 BOOL _PlayerPickupCollectable();
 // fixture controls
 static CCollectable slot;static bool placeFails,pickupAccepts;static int returned,pickups,placedFor;static CBot* seenBot;static CHud2* seenHud;static CPlayer* seenPlayer;
};
CPlayer* CCollectable::m_pPlayer=nullptr;CBot* CCollectable::m_pCollectBot=nullptr;CHud2* CCollectable::m_pCollectHud=nullptr;
CCollectable CCollectable::slot;bool CCollectable::placeFails=false,CCollectable::pickupAccepts=true;
int CCollectable::returned=0,CCollectable::pickups=0,CCollectable::placedFor=-9;CBot* CCollectable::seenBot=nullptr;CHud2* CCollectable::seenHud=nullptr;CPlayer* CCollectable::seenPlayer=nullptr;
BOOL CCollectable::_PlaceIntoWorld(CCollectableType* t,const CFMtx43A*,const CFVec3A*,f32,f32,s32,void*,s32 r,CCollectable** pp){
 *pp=nullptr;if(placeFails||!t)return FALSE;slot.m_pCollectableType=t;slot.inWorld=true;placedFor=r;*pp=&slot;return TRUE;}
BOOL CCollectable::_PlayerPickupCollectable(){++pickups;seenBot=m_pCollectBot;seenHud=m_pCollectHud;seenPlayer=m_pPlayer;
 if(pickupAccepts)ReturnCollectable(this);return pickupAccepts;}
CBot* shoppers[2];
static s32 _CoopGrantRecipient(const CBot* b){for(int n=0;n<2;++n)if(shoppers[n]==b)return n;return -1;}
struct CBarterItem{CollectableType_e m_eCollectableType;BOOL Purchase(CBot*,u32,BOOL)const;};
struct CBarterItemInst{CBarterItem* m_poItem=nullptr;u32 m_nWasherPrice=10;s8 qty=3;bool ammo=false;
 s8 GetQuantity(){return qty;}void SetQuantity(s8 n){qty=n;}bool IsAmmo(){return ammo;}BOOL Purchase(CBot*);};
'''

CHECKS = r'''
int main(){
 CInventory inv1,inv2;CBotGlitch p1,p2;p1.m_pInventory=&inv1;p2.m_pInventory=&inv2;shoppers[0]=&p1;shoppers[1]=&p2;
 CWeapon rivet;CItemInst rivetItem;rivet.item=&rivetItem;rivet.level=2;inv1.rivet=&rivetItem;p1.m_WeaponInv[0].m_apWeapon[0]=&rivet;
 inv1.m_aoItems[0].m_nClipAmmo=100;inv2.m_aoItems[0].m_nClipAmmo=100;
 CBarterItem ammoItem{rivetType.m_eType};CBarterItemInst ammo;ammo.m_poItem=&ammoItem;ammo.ammo=true;ammo.qty=BARTER_INFINITE;

 // Owned Rivet Gun: ammo goes into the reserve directly, tier kept, shopper charged.
 require(ammo.Purchase(&p1),"Rivet ammo purchase succeeds");
 require(rivet.reserve==50&&rivetItem.m_nReserveAmmo==50,"50 Rivets reach live and saved reserve");
 require(rivet.level==2&&rivet.clip==0,"ammo purchase leaves upgrade level and clip alone");
 require(inv1.m_aoItems[0].m_nClipAmmo==90&&inv2.m_aoItems[0].m_nClipAmmo==100,"only the shopper pays");
 require(CCollectable::pickups==0&&!CCollectable::slot.inWorld,"owned-weapon ammo drops nothing");
 // Full reserve: refused, nothing charged, nothing dropped (the old path dropped and charged).
 require(!ammo.Purchase(&p1),"full reserve refuses the sale");
 require(inv1.m_aoItems[0].m_nClipAmmo==90&&CCollectable::pickups==0,"refused ammo is free and drops nothing");
 rivet.reserve=45;require(ammo.Purchase(&p1)&&rivet.reserve==50&&inv1.m_aoItems[0].m_nClipAmmo==80,"near-full reserve tops up and charges");
 for(int i=0;i<3;++i){rivet.reserve=0;require(ammo.Purchase(&p1),"repeated ammo purchases");}
 require(inv1.m_aoItems[0].m_nClipAmmo==50&&rivet.reserve==50,"each repeat charged once");

 // Secondary ammo of an owned weapon goes into the clip.
 CWeapon charge;CItemInst chargeItem;charge.item=&chargeItem;charge.maxClip=10;inv2.grenade=&chargeItem;p2.m_WeaponInv[1].m_apWeapon[0]=&charge;
 CBarterItem chargeAmmo{chargeType.m_eType};CBarterItemInst ca;ca.m_poItem=&chargeAmmo;ca.ammo=true;ca.qty=BARTER_INFINITE;
 require(ca.Purchase(&p2)&&charge.clip==5&&chargeItem.m_nClipAmmo==5,"P2 secondary ammo reaches the clip");
 require(inv2.m_aoItems[0].m_nClipAmmo==90&&inv1.m_aoItems[0].m_nClipAmmo==50,"P2 shopper pays, P1 untouched");

 // Not owned: the item is placed for the shopper and collected for them at once.
 CBarterItem battery{batteryType.m_eType};CBarterItemInst bi;bi.m_poItem=&battery;bi.qty=2;
 CCollectable::m_pCollectBot=&p1;
 require(bi.Purchase(&p2),"collected item is a delivered purchase");
 require(CCollectable::placedFor==1&&CCollectable::seenBot==&p2&&CCollectable::seenHud==&CHud2::huds[1]&&CCollectable::seenPlayer==&Player_aPlayer[1],"collected by the shopper with the shopper's HUD");
 require(CCollectable::m_pCollectBot==&p1,"collector context restored afterwards");
 require(!CCollectable::slot.inWorld&&inv2.m_aoItems[0].m_nClipAmmo==80&&bi.qty==2,"charged; batteries keep their fixed quantity");
 CBarterItem weaponItem{rivetType.m_eType};CBarterItemInst wi;wi.m_poItem=&weaponItem;wi.qty=3;
 require(wi.Purchase(&p2)&&wi.qty==2&&inv2.m_aoItems[0].m_nClipAmmo==70,"weapon purchase charges and uses stock");
 // The shopper can't take it: removed from the world, no charge, no stock used.
 CCollectable::pickupAccepts=false;const int returnedBefore=CCollectable::returned;
 require(!wi.Purchase(&p2),"uncollectable purchase fails");
 require(!CCollectable::slot.inWorld&&CCollectable::returned==returnedBefore+1,"uncollected item is not left lying in the world");
 require(inv2.m_aoItems[0].m_nClipAmmo==70&&wi.qty==2,"uncollected purchase: no charge, stock kept");
 CCollectable::pickupAccepts=true;CCollectable::placeFails=true;const int pickupsBefore=CCollectable::pickups;
 require(!wi.Purchase(&p2)&&CCollectable::pickups==pickupsBefore,"placement failure fails before collection");
 require(inv2.m_aoItems[0].m_nClipAmmo==70&&wi.qty==2,"placement failure: no charge, stock kept");
 CCollectable::placeFails=false;
 CBotGlitch stranger;CInventory si;stranger.m_pInventory=&si;si.m_aoItems[0].m_nClipAmmo=50;
 require(!wi.Purchase(&stranger)&&si.m_aoItems[0].m_nClipAmmo==50,"a bot that isn't a player receives nothing and pays nothing");
 // Ammo item but the weapon isn't owned: still a real (collected) pickup, as retail gives the gun.
 CBarterItemInst unowned;unowned.m_poItem=&ammoItem;unowned.ammo=true;unowned.qty=BARTER_INFINITE;
 require(unowned.Purchase(&p2)&&CCollectable::slot.m_pCollectableType==&rivetType&&inv2.m_aoItems[0].m_nClipAmmo==60,"unowned-weapon ammo falls back to the collected pickup");
 std::printf("PASS: %d production shop purchase checks; game not launched.\n",checks);
}
'''


def main():
    enum = re.search(r"typedef enum\s*\{.*?\}\s*CollectableType_e;",
                     (ROOT / "ma/App/ma/collectable.h").read_text(), re.S).group()
    collect = (ROOT / "ma/App/ma/collectable.cpp").read_text()
    barter = (ROOT / "ma/App/ma/BarterTypes.cpp").read_text()
    code = enum + BASE
    for source, signature in (
        (collect, "BOOL CCollectable::OwnsWeaponForAmmo("),
        (collect, "BOOL CCollectable::GiveOwnedWeaponAmmo("),
        (collect, "BOOL CCollectable::GiveToPlayerAndCollect("),
        (barter, "BOOL CBarterItem::Purchase("),
        (barter, "BOOL CBarterItemInst::Purchase("),
    ):
        # The barter signatures sit inside "#if FANG_WINGC ... #else <old signature> #endif"; the
        # extract starts at the new signature, so reopen the #if it belongs to.
        code += ("#if 1\n" if "CBarter" in signature else "") + method(source, signature)
    OUT.mkdir(parents=True, exist_ok=True)
    (OUT / "purchase.cpp").write_text(code + CHECKS)
    (OUT / "CMakeLists.txt").write_text(
        "cmake_minimum_required(VERSION 3.20)\nproject(shop_purchase LANGUAGES CXX)\n"
        "add_executable(purchase purchase.cpp)\ntarget_compile_features(purchase PRIVATE cxx_std_17)\n")
    for cmd in (["cmake", "-S", str(OUT), "-B", str(OUT / "out"), "-A", "Win32"],
                ["cmake", "--build", str(OUT / "out"), "--config", "Release"]):
        result = subprocess.run(cmd, capture_output=True, text=True)
        if result.returncode:
            raise SystemExit(result.stdout + result.stderr)
    subprocess.run([str(OUT / "out/Release/purchase.exe")], check=True)


if __name__ == "__main__":
    main()
