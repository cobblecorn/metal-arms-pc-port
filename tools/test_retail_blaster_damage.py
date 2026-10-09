"""Exercise production Scatter Blaster impacts against retail damage evidence.

Deterministic collision fixture; no game launch or save access. Checks repeated
pellets, separate targets, aim centrality, distance falloff, all player slots,
misses, and the retail 16-entity burst limit.
"""
from pathlib import Path
import struct
import subprocess
from dol_vocab import Dol
from test_coop_checkpoint_rat import method

ROOT = Path(__file__).resolve().parent.parent
OUT = ROOT / 'build/test-retail-blaster-damage'
FIXTURE = r'''
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <vector>
#include <algorithm>
using f32=float; using u32=unsigned; using BOOL=int;
constexpr BOOL TRUE=1,FALSE=0;
#define FANG_WINGC 1
#define FMATH_CLAMP_UNIT_FLOAT(x) ((x)=std::max(0.f,std::min(1.f,(x))))
struct CFVec3A {float x=0,y=0,z=0;
 CFVec3A& Mul(const CFVec3A&v,float n){x=v.x*n;y=v.y*n;z=v.z*n;return *this;}
 CFVec3A& Add(const CFVec3A&v){x+=v.x;y+=v.y;z+=v.z;return *this;}
 float Dot(const CFVec3A&v)const{return x*v.x+y*v.y+z*v.z;}
 float Dist(const CFVec3A&v)const{return std::sqrt((x-v.x)*(x-v.x)+(y-v.y)*(y-v.y)+(z-v.z)*(z-v.z));}};
struct CEntity {int id=0;};
struct CFWorldTracker {};
struct CFWorldMesh : CFWorldTracker {};
struct CBot {int m_nPossessionPlayerIndex=0;void AppendTrackerSkipList(){}};
struct FCollImpact_t {CFVec3A ImpactPoint,UnitFaceNormal;void* pTag=nullptr;};
struct PlannedHit {CEntity* entity;float distance,alignment;bool hit=true;};
std::vector<PlannedHit> plan;
unsigned pellet=0;
CEntity* currentEntity;
int randomCalls=0,decals=0,particles=0;
void fmath_ScatterUnitVec(CFVec3A*out,const CFVec3A*,float){auto&p=plan.at(pellet);out->x=std::sqrt(std::max(0.f,1-p.alignment*p.alignment));out->y=0;out->z=p.alignment;}
void fmath_RandomInt32(){++randomCalls;}
int FWorld_nTrackerSkipListCount=0;
CFWorldTracker* FWorld_apTrackerSkipList[8];
constexpr int FCOLL_MASK_COLLIDE_WITH_THIN_PROJECTILES=8;
BOOL fworld_FindClosestImpactPointToRayStart(FCollImpact_t*out,const CFVec3A*origin,const CFVec3A*,int,const CFWorldTracker**,BOOL,CFWorldMesh* mesh,int,int){auto&p=plan.at(pellet++);currentEntity=p.entity;out->ImpactPoint={origin->x,origin->y,origin->z+p.distance};out->pTag=mesh;return p.hit;}
struct CGCollMaterial {BOOL IsImpactFlashEnabled()const{return FALSE;}void DrawAllDrawableParticles(const CFVec3A*,const CFVec3A*,BOOL,float,float,float)const{++particles;}};
struct CGColl {static const CGCollMaterial* GetMaterial(FCollImpact_t*){static CGCollMaterial m;return &m;}static CEntity* ExtractEntity(FCollImpact_t*){return currentEntity;}};
struct CMuzzleFlash {static void AddFlash_CardPosterXY(int,const CFVec3A&,float,float){}};
constexpr int MUZZLEFLASH_TYPE_BALL_POSTER_XY_1=0;
struct CFDecal {static void Create(int,FCollImpact_t*,const CFVec3A*){++decals;}};
struct CDamageForm {enum {DAMAGE_LOCALE_IMPACT,DAMAGE_DELIVERY_ONE_SPECIFIC_ENTITY};int m_nDamageLocale,m_nDamageDelivery,m_pDamageProfile;
 float m_fNormIntensity=1;CEntity* m_pDamageeEntity;
 struct {void*pWeapon;CBot*pBot;int nDamagerPlayerIndex;}m_Damager;
 CFVec3A point,direction;
 void InitTriDataFromCollImpact(CFWorldMesh*,FCollImpact_t*impact,const CFVec3A*dir){point=impact->ImpactPoint;direction=*dir;}};
struct CDamage {static std::vector<CDamageForm> forms;static CDamageForm* GetEmptyDamageFormFromPool(){return new CDamageForm;}static void SubmitDamageForm(CDamageForm*f){forms.push_back(*f);delete f;}};
std::vector<CDamageForm> CDamage::forms;
constexpr int AISOUNDTYPE_WEAPON=1;
float AIEnviro_fLaserImpactAudibleRange=10;
void AIEnviro_AddSound(const CFVec3A&,float,float,int,int,void*){}
struct CWeaponBlaster {struct Props {float fMaxLiveRange=250,fMuzzleOffset_Impact=0,fMuzzleScale_Impact=1,fMuzzleAlpha_Impact=1;int hDecalDef=0,pDamageProfile=99;} props,*m_pUserProps=&props;
 struct Resource {CFWorldMesh mesh;CFWorldMesh*m_pWorldMesh=&mesh;}resource,*m_pResourceData=&resource;
 CBot bot,*m_pOwnerBot=&bot;int m_ahMuzzleFlashGroup[1]={};float m_fSecsUntilNextSound=1;
 CBot* GetOwner(){return m_pOwnerBot;}
 void _ScatterBlast(const CFVec3A&,const CFVec3A&,u32,float);
};
int checks=0;
#define CHECK(x) do{++checks;if(!(x)){std::fprintf(stderr,"FAIL line %d: %s\n",__LINE__,#x);std::exit(1);}}while(0)
void shoot(CWeaponBlaster&w,const std::vector<PlannedHit>&hits){plan=hits;pellet=0;decals=particles=randomCalls=0;CDamage::forms.clear();w._ScatterBlast({}, {0,0,1},(u32)hits.size(),.085f);CHECK(pellet==hits.size());CHECK(randomCalls==hits.size());}
bool close(float a,float b){return std::fabs(a-b)<.00001f;}
'''
CHECKS = r'''
int main(){CWeaponBlaster w;CEntity target{1},other{2};
 for(int player=0;player<4;++player){w.bot.m_nPossessionPlayerIndex=player;
  for(unsigned pellets: {1u,12u,14u,100u}){
   shoot(w,std::vector<PlannedHit>(pellets,{&target,0,1}));CHECK(CDamage::forms.size()==1);auto&f=CDamage::forms[0];
   CHECK(close(f.m_fNormIntensity,1));CHECK(f.m_pDamageeEntity==&target);CHECK(f.m_Damager.nDamagerPlayerIndex==player);CHECK(f.m_pDamageProfile==99);CHECK(decals==(int)pellets);CHECK(particles==(int)pellets);
   CHECK(f.m_fNormIntensity*.5f<1.f); // Retail MP Glitch armor prevents the old full-health burst kill.
  }
 }
 for(float distance: {0.f,25.f,125.f,200.f,250.f,300.f}){
  shoot(w,{{&target,distance,1}});float falloff=std::max(0.f,1-distance/250.f);float expected=falloff*falloff*falloff;
  if(expected==0)CHECK(CDamage::forms.empty());else{CHECK(CDamage::forms.size()==1);CHECK(close(CDamage::forms[0].m_fNormIntensity,expected));}
 }
 shoot(w,{{&target,0,.99f},{&target,125,.9975f}});CHECK(CDamage::forms.size()==1);CHECK(close(CDamage::forms[0].m_fNormIntensity,.0625f));CHECK(close(CDamage::forms[0].point.z,125));
 shoot(w,{{&target,0,.99f},{&target,0,.995f}});CHECK(CDamage::forms.empty());
 shoot(w,{{&target,0,.9975f},{&other,125,1},{&target,0,1},{&other,125,.9975f}});CHECK(CDamage::forms.size()==2);CHECK(close(CDamage::forms[0].m_fNormIntensity,1));CHECK(close(CDamage::forms[1].m_fNormIntensity,.125f));
 shoot(w,{{nullptr,10,1},{&target,10,1,false}});CHECK(CDamage::forms.empty());CHECK(decals==1);
 shoot(w,{});CHECK(CDamage::forms.empty());
 CEntity many[20];std::vector<PlannedHit> hits;for(auto&entity:many)hits.push_back({&entity,0,1});shoot(w,hits);CHECK(CDamage::forms.size()==16);
 w.m_pOwnerBot=nullptr;shoot(w,{{&target,0,1}});CHECK(CDamage::forms[0].m_Damager.nDamagerPlayerIndex==-1);
 std::printf("PASS: %d production retail Scatter Blaster damage checks; no game launched.\n",checks);
}
'''


def main():
    dol = Dol((ROOT / 'gamedata/sys/main.dol').read_bytes())
    # Values loaded by retail CWeaponBlaster damage routine, proven from main.dol.
    assert struct.unpack_from('>f',dol.data,dol.to_off(0x804bf5b0))[0] == 200
    assert abs(struct.unpack_from('>f',dol.data,dol.to_off(0x804bf5b4))[0] - .995) < 1e-7
    source = (ROOT / 'ma/App/ma/weapon_blaster.cpp').read_text()
    code = FIXTURE + method(source, 'void CWeaponBlaster::_ScatterBlast(') + CHECKS
    OUT.mkdir(parents=True, exist_ok=True)
    (OUT/'blaster_damage.cpp').write_text(code)
    (OUT/'CMakeLists.txt').write_text('cmake_minimum_required(VERSION 3.20)\nproject(retail_blaster_damage LANGUAGES CXX)\nadd_executable(blaster_damage blaster_damage.cpp)\n')
    for command in (['cmake','-S',str(OUT),'-B',str(OUT/'out'),'-A','Win32'],
                    ['cmake','--build',str(OUT/'out'),'--config','Release']):
        run = subprocess.run(command,capture_output=True,text=True)
        if run.returncode:
            raise SystemExit(run.stdout+run.stderr)
    subprocess.run([str(OUT/'out/Release/blaster_damage.exe')],check=True)


if __name__ == '__main__':
    main()
