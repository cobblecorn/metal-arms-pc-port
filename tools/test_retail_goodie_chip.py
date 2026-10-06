"""Check the production loot parser, bag cloning and drop path without launching the game."""
from pathlib import Path
import re
import subprocess

ROOT = Path(__file__).resolve().parent.parent
OUT = ROOT / 'build/test-retail-goodie-chip'


def clean(source):
    source = re.sub(r'/\*.*?\*/', '', source, flags=re.S)
    return re.sub(r'//[^\n]*', '', source)


def method(source, signature):
    source = clean(source)
    start = source.index(signature)
    brace = source.index('{', start)
    end, depth = brace + 1, 1
    while depth:
        depth += (source[end] == '{') - (source[end] == '}')
        end += 1
    return source[start:end] + '\n'


BASE = r'''
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cctype>
#include <cmath>
#include <vector>
#include <string>
using u32=unsigned;using f32=float;using BOOL=int;using cchar=const char;
constexpr int TRUE=1,FALSE=0,COLLECTABLE_UNKNOWN=0,COLLECTABLE_CHIP=1,COLLECTABLE_WASHER=2;
using GoodieType_e=u32;using CollectableType_e=u32;
enum GoodieBagAngVelType_e{GOODIEBAGANGVELTYPE_NONE,GOODIEBAGANGVELTYPE_RANDOM};
constexpr unsigned GoodieBag_uMaxNumGoodieInsts=4;
#define DEVPRINTF(...) (void)0
#define FASSERT(x) do{if(!(x))std::abort();}while(0)
#define FMATH_FPOT(t,a,b) ((a)+(t)*((b)-(a)))
constexpr float FMATH_2PI=6.2831853f;
int checks=0,errors=0;
void require(bool b,const char* msg){++checks;if(!b){std::fprintf(stderr,"FAIL: %s\n",msg);std::exit(1);}}
int fclib_stricmp(const char* a,const char* b){while(*a&&*b){int d=std::tolower(*a++)-std::tolower(*b++);if(d)return d;}return *a-*b;}
struct CFVec3A{
 float x=0,y=0,z=0;static const CFVec3A m_UnitAxisY;
 void Set(float a,float b,float c){x=a;y=b;z=c;}void Mul(float n){x*=n;y*=n;z*=n;}
 void Add(CFVec3A v){x+=v.x;y+=v.y;z+=v.z;}void Unitize(){float m=std::sqrt(x*x+y*y+z*z);if(m)Mul(1/m);}
};
const CFVec3A CFVec3A::m_UnitAxisY{0,1,0};
struct CFMtx43A{CFVec3A m_vPos;static CFMtx43A m_Xlat;};CFMtx43A CFMtx43A::m_Xlat;
float randomUnit=.5f;float fmath_RandomFloat(){return randomUnit;}
u32 fmath_RandomChoice(u32){return 0;}float fmath_RandomBipolarUnitFloat(){return 0;}
float fmath_RandomFloatRange(float a,float b){return (a+b)*.5f;}
void fmath_RandomPointInSphericalSector(CFVec3A& v,float,float,float){v.Set(0,1,0);}
struct CEntity{CFVec3A pos{-44.05465f,32.28349f,-1488.0387f};u32 GetChildCount(){return 0;}
 void GetGoodieDistributionOrigin(CFVec3A* v){*v=pos;}void GetGoodieDistributionDir(CFVec3A* v){v->Set(0,1,0);}};
struct CGoodieInst{
 GoodieType_e m_eGoodieType=0;u32 m_auQuantity[2]{};float m_fProb=0;
 GoodieBagAngVelType_e m_eAngVelType=GOODIEBAGANGVELTYPE_NONE;BOOL m_bCreated=FALSE;
 static GoodieType_e GetGoodieTypeFromString(cchar* s){return !fclib_stricmp(s,"chip")?COLLECTABLE_CHIP:!fclib_stricmp(s,"washer")?COLLECTABLE_WASHER:COLLECTABLE_UNKNOWN;}
 void Create(GoodieType_e,u32,u32,f32,GoodieBagAngVelType_e);void CloneFromOther(CGoodieInst*);
 GoodieType_e GetGoodieType(){return m_eGoodieType;}u32 GetQuant();
};
struct CGoodieBag{
 u32 m_uNumGoodies=0;float m_fProb=1,m_fGoodieScale=1;int m_eBrain=0;CGoodieInst m_aGoodies[4];
 BOOL AddGoodie(GoodieType_e,u32,u32,f32,GoodieBagAngVelType_e);
 void ReleaseIntoWorld(CEntity*);void CloneOther(CGoodieBag*);
};
static const float _afDefaultVel[2]={40,45};
struct Drop{CollectableType_e type;CFVec3A pos;CEntity* owner;};std::vector<Drop> drops;
struct CCollectable{
 static inline int notifications=0;
 static void NotifyCollectableUsedInWorld(cchar*){++notifications;}
 static BOOL PlaceIntoWorld(CollectableType_e t,CFMtx43A* m,CFVec3A*,float,int,CEntity* e){drops.push_back({t,m->m_vPos,e});return TRUE;}
};
enum FGameData_VarType_e{FGAMEDATA_VAR_TYPE_STRING,FGAMEDATA_VAR_TYPE_FLOAT};
struct Field{
 FGameData_VarType_e type;float f=0;std::string s;
 Field(float v):type(FGAMEDATA_VAR_TYPE_FLOAT),f(v){}Field(const char* v):type(FGAMEDATA_VAR_TYPE_STRING),s(v){}
};
std::vector<Field> fields;
u32 fgamedata_GetNumFields(int){return static_cast<u32>(fields.size());}
const void* fgamedata_GetPtrToFieldData(int,unsigned i,FGameData_VarType_e& t){FASSERT(i<fields.size());t=fields[i].type;return t==FGAMEDATA_VAR_TYPE_FLOAT?static_cast<const void*>(&fields[i].f):fields[i].s.c_str();}
struct CEntityParser{static inline int m_hTable=0;static void Error_Prefix(){++errors;}static void Error_Dashes(){}};
struct CEntityBuilder{CGoodieBag m_oGoodieBag;BOOL ParseGoodie();};
'''

CHECKS = r'''
int main(){
 fields={"chip",1.f,1.f,1.f,"X","safe"};CEntityBuilder retail;retail.ParseGoodie();
#if FANG_WINGC
 require(errors==0&&retail.m_oGoodieBag.m_uNumGoodies==1,"retail six-field prison2 chip entry parses");
 require(CCollectable::notifications==1,"parsed chip registers resources for loading");
 auto& chip=retail.m_oGoodieBag.m_aGoodies[0];
 require(chip.m_eGoodieType==COLLECTABLE_CHIP&&chip.m_auQuantity[0]==1&&chip.m_auQuantity[1]==1&&chip.m_fProb==1,"authored one-chip quantity and probability preserved");
 require(chip.m_eAngVelType==GOODIEBAGANGVELTYPE_NONE,"retail placeholder uses ordinary no-spin metadata");
 CGoodieBag runtime;runtime.CloneOther(&retail.m_oGoodieBag);CEntity prison2;
 for(float r:{0.f,.2f,.5f,.999f}){
  randomUnit=r;drops.clear();runtime.ReleaseIntoWorld(&prison2);
  require(drops.size()==1&&drops[0].type==COLLECTABLE_CHIP,"guard death releases exactly one actual chip regardless of random quantity roll");
  require(drops[0].owner==&prison2&&drops[0].pos.x==prison2.pos.x&&drops[0].pos.y==prison2.pos.y+.5f,"chip drops at authored guard location with normal collision owner exclusion");
 }
 for(const char* spin:{"none","random","x"}){
  fields={"chip",1.f,1.f,1.f,spin,"SAFE"};CEntityBuilder b;int n=errors;b.ParseGoodie();
  require(errors==n&&b.m_oGoodieBag.m_uNumGoodies==1,"recognized retail suffix accepts case-insensitive compatible spin values");
 }
#else
 require(errors==1&&retail.m_oGoodieBag.m_uNumGoodies==0,"legacy non-PC parser reproduces missing chip from six-field rejection");
 require(CCollectable::notifications==0,"rejected entry never registers chip resources");
#endif
 for(int count=4;count<=5;count++)for(const char* spin:{"none","random"}){
  fields={"washer",2.f,4.f,.75f};if(count==5)fields.emplace_back(spin);
  CEntityBuilder legacy;int n=errors;legacy.ParseGoodie();auto& g=legacy.m_oGoodieBag.m_aGoodies[0];
  require(errors==n&&legacy.m_oGoodieBag.m_uNumGoodies==1,"legacy four/five-field goodies unchanged");
  require(g.m_auQuantity[0]==2&&g.m_auQuantity[1]==4&&g.m_fProb==.75f,"legacy amounts/probability unchanged");
 }
 for(int invalid=0;invalid<8;invalid++){
  fields={"chip",1.f,1.f,1.f,"X","safe"};
  switch(invalid){case 0:fields.resize(3,Field(0.f));break;case 1:fields.emplace_back("extra");break;
   case 2:fields[0]=Field("unknown");break;case 3:fields[4]=Field(1.f);break;
   case 4:fields[4]=Field("unknown");break;case 5:fields[5]=Field(1.f);break;
   case 6:fields[5]=Field("unknown");break;case 7:fields.resize(5,Field(0.f));break;}
  CEntityBuilder b;int n=errors,notify=CCollectable::notifications;b.ParseGoodie();
  require(errors==n+1&&b.m_oGoodieBag.m_uNumGoodies==0&&CCollectable::notifications==notify,"malformed/unknown extras fail without installing partial loot");
 }
 fields={"washer",1.f,1.f,1.f};CEntityBuilder full;full.m_oGoodieBag.m_uNumGoodies=4;
 int n=errors;full.ParseGoodie();require(errors==n+1&&full.m_oGoodieBag.m_uNumGoodies==4,"bag capacity remains enforced");
 std::printf("PASS: %d loot parser/clone/drop checks (PC=%d); game not launched.\n",checks,FANG_WINGC);
}
'''


def main():
    entity = (ROOT / 'ma/App/ma/entity.cpp').read_text()
    start = entity.index('// They are specifying a specific goodie in the bag.')
    end = entity.index('\n\t} else if(', start)
    code = BASE + '\nBOOL CEntityBuilder::ParseGoodie(){\n' + entity[start:end] + '\n}\n'
    bag = (ROOT / 'ma/App/ma/GoodieBag.cpp').read_text()
    for signature in ('u32 CGoodieInst::GetQuant(',
                      'void CGoodieInst::Create(GoodieType_e eGoodieType, u32 uQuantity1, u32 uQuantity2, f32',
                      'void CGoodieInst::CloneFromOther(',
                      'BOOL CGoodieBag::AddGoodie(GoodieType_e eGoodieType, u32 uQuantity1, u32 uQuantity2, f32',
                      'void CGoodieBag::ReleaseIntoWorld(', 'void CGoodieBag::CloneOther('):
        code += method(bag, signature)
    OUT.mkdir(parents=True, exist_ok=True)
    (OUT / 'loot.cpp').write_text(code + CHECKS)
    (OUT / 'CMakeLists.txt').write_text('cmake_minimum_required(VERSION 3.20)\nproject(retail_loot LANGUAGES CXX)\nforeach(pc IN ITEMS 0 1)\nadd_executable(loot${pc} loot.cpp)\ntarget_compile_features(loot${pc} PRIVATE cxx_std_17)\ntarget_compile_definitions(loot${pc} PRIVATE FANG_WINGC=${pc})\nendforeach()\n')
    for command in (['cmake', '-S', str(OUT), '-B', str(OUT / 'out'), '-A', 'Win32'],
                    ['cmake', '--build', str(OUT / 'out'), '--config', 'Release']):
        result = subprocess.run(command, capture_output=True, text=True)
        if result.returncode:
            raise SystemExit(result.stdout + result.stderr)
    for pc in (0, 1):
        subprocess.run([str(OUT / f'out/Release/loot{pc}.exe')], check=True)


if __name__ == '__main__':
    main()
