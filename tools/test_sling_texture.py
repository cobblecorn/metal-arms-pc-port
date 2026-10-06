"""Offline production slingshot, CMPR alpha and baked shader regressions; no game launch."""
from pathlib import Path
import re
import subprocess
from test_coop_checkpoint_rat import method

ROOT = Path(__file__).resolve().parents[1]
OUT = ROOT / 'build/test-sling-texture'
BASE = r'''
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <cstring>
#include <algorithm>
using u8=unsigned char;using u16=unsigned short;using u32=unsigned;using s32=int;using f32=float;using BOOL=int;
constexpr BOOL TRUE=1,FALSE=0;
#define MA_PC_INPUT 1
#define FANG_PLATFORM_WIN 1
#define FASSERT(x) ((void)0)
#define FSTATIC static
#define FINLINE inline
int checks=0;void require(bool v,const char* m){++checks;if(!v){std::printf("FAIL: %s\n",m);std::exit(1);}}
float fmath_Div(float a,float b){return a/b;}
float FLoop_fPreviousLoopSecs=1.f/60;
struct CFVec3A{float x=0,y=0,z=0;};
'''
SLING = r'''
namespace sling {
struct CWeapon {enum{WEAPON_TYPE_MORTAR=1,WEAPON_TYPE_RIVET_GUN=2};int type=WEAPON_TYPE_MORTAR;int Type(){return type;}};
struct CWeaponMortar {
 enum{SLING_MODE_IDLE,SLING_MODE_AIMING,STATE_STOWED,STATE_DEPLOYED};
 static constexpr int _MAX_SAMPLE_COUNT=10;
 int m_nSlingMode=SLING_MODE_AIMING,state=STATE_DEPLOYED;
 float m_fUnitStretch=0,m_fUnitLaunchSpeed=0,m_afTriggerValue[10]{},m_afTriggerDeltaTime[10]{};
 int m_nNextFreeTriggerSampleIndex=0;unsigned m_nTriggerSampleCount=0;CFVec3A m_UnitFireDir_WS;
 BOOL IsCreated(){return TRUE;}int CurrentState(){return state;}
 u32 TriggerWork(f32,f32,const CFVec3A*,const CFVec3A* = nullptr);
};
float gated(float held,bool mortar){float fTrig1=0,m_fControls_Fire1=held;CWeapon weapon;weapon.type=mortar?1:2;CWeapon* m_apWeapon[]={&weapon};
'''
SLING_CHECKS = r'''
void run(){
 CFVec3A target{1,2,3};
 for(float frame:{1.f/240,1.f/60,.1f,.25f,1.f})for(float draw:{.2f,.5f,1.f}){
  CWeaponMortar m;FLoop_fPreviousLoopSecs=frame;
  for(int i=0;i<35;i++)require(!m.TriggerWork(draw,0,&target),"holding cannot launch a grenade");
  require(m.TriggerWork(0,0,&target)==1,"release fires despite frame stalls and partial trigger pressure");
  require(std::abs(m.m_fUnitLaunchSpeed-draw)<.0001f,"release retains analog draw strength");
  require(m.m_UnitFireDir_WS.z==3,"release retains target");
  require(!m.TriggerWork(0,0,&target),"idle release cannot duplicate a round");
 }
 for(float step:{.05f,.1f,.2f}){
  CWeaponMortar m;FLoop_fPreviousLoopSecs=1.f/60;
  m.TriggerWork(1,0,&target);
  for(float pressure=1;pressure>step;pressure-=step)require(!m.TriggerWork(pressure,0,&target),"gradual release does not fire before actual release");
  require(m.TriggerWork(0,0,&target)==1,"gradual analog release fires instead of discarding shot");
 }
 CWeaponMortar m;FLoop_fPreviousLoopSecs=1.f/60;m.TriggerWork(1,0,&target);
 require(gated(1,true)==1&&gated(1,false)==0&&gated(0,true)==0,"blocked slingshot stance retains held input, ordinary guns retain gating");
 require(!m.TriggerWork(gated(.8f,true),0,&target),"temporary movement gate cannot synthesize a release");
 FLoop_fPreviousLoopSecs=0;require(m.TriggerWork(0,0,&target)==1,"zero-duration release can fire existing loaded history");
 require(!m.TriggerWork(0,0,&target),"zero-duration release cannot fire twice");
 FLoop_fPreviousLoopSecs=.016f;m.TriggerWork(1,0,&target);m.state=CWeaponMortar::STATE_STOWED;
 require(!m.TriggerWork(0,0,&target)&&!m.m_nTriggerSampleCount,"stowing cancels history rather than firing");
 m.state=CWeaponMortar::STATE_DEPLOYED;require(!m.TriggerWork(0,0,&target),"deploying cannot replay abandoned draw");
 m.TriggerWork(1,0,&target);m.m_nSlingMode=CWeaponMortar::SLING_MODE_IDLE;
 require(!m.TriggerWork(0,0,&target)&&!m.m_nTriggerSampleCount,"loading/aborting modes do not fire");
}
}
'''
TEXTURE_CHECKS = r'''
void run(){
 u8 blocks[32]{};u32 pixels[64]{};
 for(bool alphaPlane:{false,true})for(bool alphaFormat:{false,true})for(unsigned c0:{0u,0xffffu})for(unsigned c1:{0u,0xffffu})for(unsigned index=0;index<4;index++){
  for(int b=0;b<4;b++){blocks[b*8]=c0>>8;blocks[b*8+1]=c0;blocks[b*8+2]=c1>>8;blocks[b*8+3]=c1;
   std::memset(blocks+b*8+4,index*0x55,4);}
  std::fill(pixels,pixels+64,0xff123456);_DecodeCmprPlane(blocks,8,8,pixels,alphaPlane,alphaFormat);
  unsigned g0=c0?255:0,g1=c1?255:0,expected=index==0?g0:index==1?g1:
   c0>c1?(index==2?(2*g0+g1)/3:(g0+2*g1)/3):(index==2?(g0+g1)/2:0);
  unsigned alpha=alphaPlane?expected:(alphaFormat&&c0<=c1&&index==3?0:255);
  for(auto pixel:pixels){require((pixel>>24)==alpha,"CMPR palette ordering preserves transparent alpha holes");
   require(alphaPlane?(pixel&0xffffff)==0x123456:(pixel&255)==expected,"CMPR three-color and four-color interpolation match hardware");}
 }
 u8 tile[32]{};
 for(unsigned a=0;a<8;a++){
  unsigned packed=(a<<12)|0xfff;for(int i=0;i<16;i++){tile[i*2]=packed>>8;tile[i*2+1]=packed;}
  _DecodeUncompressedGCTile(2,tile,0,0,4,4,pixels);
  for(int i=0;i<16;i++){require((pixels[i]>>24)==(a*255+3)/7,"RGB5A3 expands three-bit alpha over full range");require((pixels[i]&0xffffff)==0xffffff,"RGB5A3 retains RGB");}
  _DecodeUncompressedGCTile(3,tile,0,0,4,4,pixels);require((pixels[0]>>24)==255,"declared opaque texture retains opaque alpha");
 }
}
}
'''

RENDER = r'''
struct D3DXVECTOR4{D3DXVECTOR4(float,float,float,float){}};
struct Device{template<class... T>void SetVertexShaderConstant(T...){}} device;Device* FDX8_pDev=&device;
unsigned bound=0,binds=0;void FDX8_SetVertexShader(unsigned h){bound=h;++binds;}
unsigned _anVShader_Handle[256],_anLMVShader[4][8]{};
unsigned _nVertexType=0,_nCurrentVtxShader=~0u,_nLightCount=0,_nPointCount=0;
bool fsh_bUseExtColorStream=false,FSh_bShadowRender=false;
unsigned FSh_shaderFlags=0;
constexpr unsigned FSh_RENDER_FORCEBASE_SHADER=1,FSh_RENDER_DETAIL=2,FSh_RENDER_REFLECTIONMAP=4;
enum{LM_NONE,LM4,ZMASK_EMASK_LM2,ZMASK_LM3,EMASK_LM3};int m_nLMMode=LM_NONE,m_nLM=1;
bool _bVBChanged=true,_bFastShader=false,FSh_bUseFastPass=true,fdx8sh_bStream1Set=true;
unsigned _nPassIdx=0,_nShaderID=0;int FSh_shaderType=0;
constexpr int SHADERTYPE_SPECULAR=3,FSHADERS_LIQUID_ENV=80,FSHADERS_LIQUID_MOLTEN_2LAYER=90;
struct Shader{unsigned vShader=VSHADER_BASE_VCOLOR;} _aShaderRenderStates[100];
int fastCalls=0;void _HandleFastShader();
'''
RENDER_CHECKS = r'''
void _HandleFastShader(){++fastCalls;_SetVertexShader(VSHADER_BASE_ENV_POINT1_DIR1);}
void run(){
 for(unsigned i=0;i<256;i++)_anVShader_Handle[i]=1000+i*7;
 for(unsigned vb=0;vb<4;vb++){
  _nVertexType=vb;fsh_bUseExtColorStream=true;FSh_bShadowRender=false;
  _SetVertexShader(VSHADER_BASE_VCOLOR);require(bound==_anVShader_Handle[VSHADER_VCOLOR_EXTSTREAM],"fullbright uses baked instance color stream");
  unsigned before=binds;_SetVertexShader(VSHADER_BASE_VCOLOR);require(binds==before,"unchanged handle does not rebind");
  fsh_bUseExtColorStream=false;_SetVertexShader(VSHADER_BASE_VCOLOR);
  require(bound==_anVShader_Handle[_anVShaderRemap[VSHADER_BASE_VCOLOR-1][vb]],"ordinary mesh retains original vertex color shader");
  _SetVertexShader_Fast(VSHADER_BASE_PASSTHRU);
  require(bound==_anVShader_Handle[_anVShaderRemap[VSHADER_BASE_PASSTHRU-1][vb]],"simple helper binds device handle rather than shader enum");
  before=binds;_SetVertexShader_Fast(VSHADER_BASE_PASSTHRU);require(before==binds,"simple helper caches actual handle");
 }
 _nVertexType=0;fsh_bUseExtColorStream=true;FSh_bShadowRender=false;_bFastShader=true;
 for(int i=0;i<5;i++){_bVBChanged=true;fsh_CheckVB();require(fastCalls==i+1,"new VB reruns combined environment shader selection");
  require(bound==_anVShader_Handle[VSHADER_ENV_POINT1_DIR1_EXTSTREAM],"reflection UV shader survives VB changes and split-screen passes");}
 _bFastShader=false;_bVBChanged=true;fsh_CheckVB();require(bound==_anVShader_Handle[VSHADER_VCOLOR_EXTSTREAM],"ordinary diffuse VB remains baked color");
 _bFastShader=true;FSh_bShadowRender=true;_bVBChanged=true;int old=fastCalls;fsh_CheckVB();require(fastCalls==old,"shadow pass never runs environment fast shader");
 FSh_bShadowRender=false;_nPassIdx=1;_bVBChanged=true;fsh_CheckVB();require(fastCalls==old,"secondary lighting pass retains its shader");
}
}
'''
TURRET = r'''
namespace turret {
constexpr int BOTSUBCLASS_SITEWEAPON_RATGUN=7;
struct{float m_fYawAdjust=0,m_fPitchAdjust=0;}Player_aPlayer[1];
float mouseX=0,mouseY=0;float TakeMouseLookDelta(BOOL pitch){return pitch?mouseY:mouseX;}
CFVec3A delta(int type,float stickX,float stickY,float dt,float mx,float my){
 struct{float m_fMaxYawVelocityPossess=2,m_fMaxPitchVelocityPossess=3;}data;auto m_pData=&data;
 struct{int m_nSubClass;}def{type};auto m_pBotDef=&def;
 float m_fControls_RotateCW=stickX,m_fControls_AimDown=stickY;int m_nPossessionPlayerIndex=0;
 mouseX=mx;mouseY=my;FLoop_fPreviousLoopSecs=dt;
'''
TURRET_CHECKS = r'''
return {fYawDelta,fPitchDelta,0};}
void run(){for(float stick:{-1.f,0.f,1.f})for(float mouse:{-.2f,0.f,.2f}){
 auto ordinary=delta(1,stick,stick,.016f,mouse,mouse),rat=delta(7,stick,stick,.016f,mouse,mouse);
 require(std::abs(rat.y+ordinary.y)<.00001f,"RAT reverses combined stick and mouse vertical input");
 require(rat.x==ordinary.x,"RAT horizontal aim unchanged");
 require(std::abs(ordinary.y-(stick*3*.016f+mouse))<.00001f,"other turret vertical input unchanged");
}}
}
'''


def main():
    mortar = (ROOT / 'ma/App/ma/weapon_mortar.cpp').read_text()
    glitch = (ROOT / 'ma/App/ma/botglitch.cpp').read_text()
    texture = (ROOT / 'port/gcdata.cpp').read_text()
    code = BASE + SLING
    code += method(glitch, 'if( m_apWeapon[0]->Type() == CWeapon::WEAPON_TYPE_MORTAR && m_fControls_Fire1 > 0.0f )')
    code += 'return fTrig1;}\n' + method(mortar, 'u32 CWeaponMortar::TriggerWork(') + SLING_CHECKS
    code += '\nnamespace texture {\nstatic u16 _ReadBE16(const u8* p){return (p[0]<<8)|p[1];}\n'
    for signature in ('static u8 _Expand5(', 'static u8 _Expand6(', 'static u8 _Expand4(',
                      'static u32 _PackArgb(', 'static void _DecodeCmprPlane(', 'static BOOL _DecodeUncompressedGCTile('):
        code += method(texture, signature)
    code += TEXTURE_CHECKS
    render = (ROOT / 'ma/Lib/Fang2/dx/fdx8sh.cpp').read_text()
    header = (ROOT / 'ma/Lib/Fang2/dx/fdx8shaders.h').read_text()
    code += '\nnamespace render {\n'
    for token in ('VSHADER_BASE_NONE', 'VSHADER_PASSTHRU_TC1'):
        pos = header.index(token)
        start = header.rfind('typedef enum', 0, pos)
        end = header.index(';', header.index('}', start)) + 1
        code += header[start:end] + '\n'
    for signature in ('_VERTEX_SHADER_BASE_e _DetailVShaderRemap', '_VERTEX_SHADER_BASE_e _ReflectionVShaderRemap',
                      '_VERTEX_SHADER_BASE_e _BaseVShaderRemap', 'u32 _anVShaderRemap'):
        code += method(header, signature) + ';\n'
    vertex = method(render, 'FSTATIC void _SetVertexShader(u32')
    # Register numbers do not affect the mocked device; shader enums and remap tables above are real.
    constants = sorted(set(re.findall(r'\bCV_\w+', vertex + method(render, 'void fsh_CheckVB('))))
    code += 'enum {' + ','.join(constants) + '};\n' + RENDER
    code += method(render, 'FINLINE void _SetVertexShader_Fast(') + vertex
    code += method(render, 'void fsh_CheckVB(') + RENDER_CHECKS
    site = (ROOT / 'ma/App/ma/site_botStateFns.cpp').read_text()
    start = site.index('\t\tf32 fYawVelocity =')
    end = site.index('#endif', start) + len('#endif')
    code += TURRET + site[start:end] + TURRET_CHECKS
    code += 'int main(){sling::run();texture::run();render::run();turret::run();std::printf("PASS: %d offline slingshot/texture/shader/turret checks; game not launched.\\n",checks);}\n'
    OUT.mkdir(parents=True, exist_ok=True)
    (OUT / 'sling_texture.cpp').write_text(code)
    (OUT / 'CMakeLists.txt').write_text('cmake_minimum_required(VERSION 3.20)\nproject(sling_texture LANGUAGES CXX)\nadd_executable(sling_texture sling_texture.cpp)\ntarget_compile_features(sling_texture PRIVATE cxx_std_17)\n')
    for command in (['cmake', '-S', str(OUT), '-B', str(OUT / 'out'), '-A', 'Win32'],
                    ['cmake', '--build', str(OUT / 'out'), '--config', 'Release']):
        run = subprocess.run(command, capture_output=True, text=True)
        if run.returncode:
            raise SystemExit(run.stdout + run.stderr)
    subprocess.run([str(OUT / 'out/Release/sling_texture.exe')], check=True)


if __name__ == '__main__':
    main()
