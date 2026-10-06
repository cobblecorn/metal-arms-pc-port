"""Offline waterfall geometry and complete draw-pass regression fixture.

Uses production generation/animation/render methods. No game launch or save access.
"""
from pathlib import Path
import re
import subprocess

ROOT = Path(__file__).resolve().parent.parent
OUT = ROOT / "build/test-waterfalls"


def method(source, signature):
    start = source.index(signature)
    brace = source.index("{", start)
    depth, end = 1, brace + 1
    while depth:
        depth += (source[end] == "{") - (source[end] == "}")
        end += 1
    return source[start:end]


FIXTURE = r'''
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <algorithm>
using u32=unsigned;using u16=unsigned short;using s32=int;using f32=float;using BOOL=int;
constexpr int TRUE=1,FALSE=0;
#define FANG_PLATFORM_WIN 1
#define FANG_PLATFORM_DX 1
#define FMATH_CLAMPMIN(x,y) (x=std::max(x,y))
#define FMATH_CLAMPMAX(x,y) (x=std::min(x,y))
#define D3DVSD_STREAM(x) 0x80000000u
#define D3DVSD_REG(r,t) (t)
#define D3DVSD_END() 0xffffffffu
constexpr u32 D3DVSDT_FLOAT3=12,D3DVSDT_FLOAT2=8;
constexpr u32 D3DFVF_XYZ=2,D3DFVF_TEX2=0x200,D3DFVF_TEX3=0x300,D3DFVF_TEX4=0x400;
constexpr int D3DRS_ZENABLE=1,D3DZB_TRUE=1,D3DCMP_LESSEQUAL=4,D3DCULL_NONE=0,D3DCULL_CCW=1;
constexpr int D3DBLEND_SRCALPHA=1,D3DBLEND_INVSRCALPHA=2,D3DBLEND_ONE=3,D3DBLENDOP_ADD=1;
constexpr int D3DTOP_MODULATE=1,D3DTOP_SELECTARG1=2,D3DTOP_DISABLE=3,D3DTA_TEXTURE=1,D3DTA_TFACTOR=2;
constexpr int D3DTSS_BUMPENVMAT00=1,D3DTSS_BUMPENVMAT01=2,D3DTSS_BUMPENVMAT10=3,D3DTSS_BUMPENVMAT11=4;
constexpr int D3DPT_TRIANGLELIST=1,D3DFMT_INDEX16=1,CV_EYE_POS_WORLD=1,CV_FIXED_COLOR=2;
constexpr int PSHADER_CUBE_REFLECT=1,MOLTEN_1LAYER_EMBM=2,MOLTEN_2LAYER_EMBM=3,MOLTEN_2LAYER_EMBM_GLOW=4;
constexpr int RP_EMISSIVE=1,RP_TEXTURE=2,RP_REFLECT=4,RP_REFRACT=8;
#define D3DTS_WORLDMATRIX(x) (x)
#define D3DCOLOR_COLORVALUE(...) 0
#define F2DW(x) (x)
struct CFVec2{float x=0,y=0;};
struct CFVec3{float x=0,y=0,z=0;void Set(float a,float b,float c){x=a;y=b;z=c;}void Set(const CFVec3& p){*this=p;}void Zero(){x=y=z=0;}};
using CFVec3A=CFVec3;
struct CFMtx43A{
 CFVec3 m_vX{1,0,0},m_vY{0,1,0},m_vZ{0,0,1},m_vPos;
 static CFMtx43A m_IdentityMtx;
 void Identity(){*this=m_IdentityMtx;}void Invert(){m_vPos.x=-m_vPos.x;m_vPos.y=-m_vPos.y;m_vPos.z=-m_vPos.z;}
 CFVec3 MulPoint(CFVec3 p){p.x+=m_vPos.x;p.y+=m_vPos.y;p.z+=m_vPos.z;return p;}
};
CFMtx43A CFMtx43A::m_IdentityMtx;
struct CFVec4{float x,y,z,w;void Set(float a,float b,float c,float d){x=a;y=b;z=c;w=d;}};
struct CFColorRGB{float fRed=1,fGreen=1,fBlue=1;void Set(float r,float g,float b){fRed=r;fGreen=g;fBlue=b;}};
struct CFTexInst{};
struct FakeView{struct {struct {CFVec3 m_vPos;}m44;}m_MtxR;}view,*FXfm_pView=&view;
u32 FVid_nFrameCounter=0;float FLoop_fPreviousLoopSecs=1.0f/60;
std::vector<void*> allocations;
void* fres_Alloc(size_t n){void* p=std::calloc(1,n);allocations.push_back(p);return p;}
'''

MODELS = r'''
struct CFLiquidVolume{};
struct CFLiquidMesh{
 LiquidType_e m_nType;CFColorRGB m_Clr;float m_fExp,m_fNextExp,m_fCurExp,m_AnimTarget;
 CFVec3 m_vExt;CFMtx43A m_Mtx,m_MtxI;CFTexInst* m_pTexInst[MAX_NUM_LAYERS]{};
 u16 m_nVtx=0,m_nIdx=0;u32 m_nLastFrameWork;float m_fSpeed,m_fOpacity=1;
 LiquidFallVtx* m_vMesh;u16* m_pIdx;CFLiquidVolume* m_pLiquidVolume;
 CFLiquidMesh();float EvalFunc(float);float EvalFunc(float,float);void SetupMesh(CFVec3A&,CFMtx43A&,float);
 void AnimateMesh(float,float);void Work();
};
struct {CFLiquidVolume volume;CFLiquidVolume* SearchForLiquidVolume(CFLiquidMesh*){return &volume;}}LiquidSystem;
int checks=0,draws=0;bool zEnabled=false,alphaTest=true,zWrite=false;int zFunction=0,blend=1,cull=-1;
u32 currentShader=0,_nCurrentVtxShader=0,_nCurrentPixelShader=0;
CFMtx43A shaderWorld,fixedWorld;
int activeView=0,renderView=-1;
void require(bool b,const char* s){checks++;if(!b){std::printf("FAIL: %s\n",s);std::exit(1);}}
u32 _anVShader_Handle[VSHADER_NUM];
struct Device{
 void SetVertexShaderConstant(int,const void*,int){}void SetPixelShaderConstant(int,const void*,int){}
 void SetPixelShader(int){}void SetTextureStageState(int,int,float){}void SetStreamSource(int,void*,int){}
 void SetRenderState(int state,int value){if(state==D3DRS_ZENABLE)zEnabled=value;}
 void DrawIndexedPrimitiveUP(int,int,u16 count,u16 prim,const u16* indices,int,void* mesh,size_t stride){
  draws++;require(zEnabled&&zFunction==D3DCMP_LESSEQUAL&&!alphaTest,"every pass explicitly depth-tests world geometry without stale alpha test");
  require(cull==D3DCULL_NONE,"waterfall visible from both sides");
  if(currentShader&0x80000000u){
   auto decl=_apVShaderDecl[currentShader&0x7fffffffu];size_t size=0;
   for(int i=0;decl[i]!=0xffffffffu;i++)if(decl[i]!=0x80000000u)size+=decl[i];
   require(size<=stride,"programmable declaration fits waterfall stride");
  }else require(12+((currentShader>>8)&15)*8<=stride,"fixed-function declaration fits waterfall stride");
  auto vertices=(LiquidFallVtx*)mesh;bool finite=true,validIndices=true;
  for(int i=0;i<count;i++)finite&=std::isfinite(vertices[i].vPos.x)&&std::isfinite(vertices[i].vPos.y)&&std::isfinite(vertices[i].vPos.z);
  for(int i=0;i<prim*3;i++)validIndices&=indices[i]<count;
  require(finite&&validIndices,"all submitted positions are finite and indices in range");
  require(renderView==activeView,"draw uses current viewport camera");
 }
}device,*FDX8_pDev=&device;
CFTexInst* _pFullScrTarget=nullptr;
void FDX8_SetVertexShader(u32 s){currentShader=s;}
void _SetPixelShader(int){}void fdx8tex_SetTexture(int,CFTexInst*,int){}
void ftex_SetTexAddress(int,BOOL,BOOL,BOOL=FALSE){}
void fdx8_SetRenderState_ALPHABLENDENABLE(BOOL){}void fdx8_SetRenderState_SRCBLEND(int){}
void fdx8_SetRenderState_DESTBLEND(int){}void fdx8_SetRenderState_BLENDOP(int){}
void fdx8_SetRenderState_CULLMODE(int c){cull=c;}void fdx8_SetRenderState_TEXTUREFACTOR(u32){}
void fdx8_SetRenderState_ZWRITEENABLE(BOOL b){zWrite=b;}
void fdx8_SetRenderState_ZFUNC(int f){zFunction=f;}
void fdx8_SetRenderState_ALPHATESTENABLE(BOOL b){alphaTest=b;}
void fdx8_SetTextureState_COLOROP(int,int){}void fdx8_SetTextureState_COLORARG1(int,int){}
void fdx8_SetTextureState_COLORARG2(int,int){}void fdx8_SetTextureState_ALPHAOP(int,int){}
void fdx8vb_UncacheSelected(){}
void fdx8xfm_SetViewDXMatrix(BOOL){renderView=activeView;}
void fxfm_SetViewAndWorldSpaceModelMatrices(){shaderWorld.Identity();}
void fdx8xfm_SetCustomDXMatrix(int,const CFMtx43A* m,BOOL shader){(shader?shaderWorld:fixedWorld)=*m;}
'''

CHECKS = r'''
int main(){
 static_assert(sizeof(LiquidFallVtx)==36,"fixture uses actual waterfall vertex layout");
 require(sizeof(_apVShaderDecl)/sizeof(_apVShaderDecl[0])==VSHADER_NUM+1,"declaration table and shader enum stay aligned");
 require(sizeof(_apVShaderFunc)/sizeof(_apVShaderFunc[0])==VSHADER_NUM+1,"shader program table and enum stay aligned");
 require(_apVShaderDecl[VSHADER_LIQUID_FALL_REFLECT]==_aDecl_POS_TEX3,"waterfall uses three UV declaration");
 require(_apVShaderDecl[VSHADER_PLANAR_REFLECT]==_aDecl_POS_TEX4,"pool quad retains four UV declaration");
 require(_apVShaderFunc[VSHADER_LIQUID_FALL_REFLECT]==_apVShaderFunc[VSHADER_PLANAR_REFLECT],"waterfall shares supported reflection shader bytecode");
 for(int i=0;i<VSHADER_NUM;i++)_anVShader_Handle[i]=0x80000000u|i;
 CFTexInst texture;CFColorRGB color;CFVec3 forward{0,1,0};
 for(int type:{LT_WATER,LT_MOLTEN})for(float curvature:{2.f,8.f,16.f,32.f}){
  CFLiquidMesh fall;fall.m_nType=(LiquidType_e)type;CFMtx43A world;world.m_vPos={140,80,-60};
  CFVec3 extent{5,10,3};fall.SetupMesh(extent,world,curvature);
  require(fall.m_nVtx==50&&fall.m_nIdx==216,"generated grid dimensions match allocation");
  for(float exponent:{2.f,8.f,16.f,32.f})for(float x:{0.f,.1f,.5f,.9f,1.f})
   require(std::fabs(fall.EvalFunc(x,exponent)-(1-powf(x-1,exponent)))<.00001f,"integer retail curvature shape preserved");
  for(float exponent:{1.1f,7.999f,8.001f,15.983f,31.1f})for(float x:{-.000001f,0.f,.1f,.5f,.9f,1.f,1.000001f}){
   auto y=fall.EvalFunc(x,exponent);require(std::isfinite(y)&&y>=0&&y<=1,"fractional curvature and endpoint rounding remain finite");
  }
  for(int frame=0;frame<90;frame++){
   FVid_nFrameCounter++;fall.Work();float scroll=fall.m_vMesh[0].vTex[0].y,shape=fall.m_fCurExp;
   bool finite=true;for(int i=0;i<fall.m_nVtx;i++)finite&=std::isfinite(fall.m_vMesh[i].vPos.y);
   require(finite,"animated grid stays finite throughout fractional curvature changes");
   for(int player=0;player<4;player++){
    fall.Work();require(fall.m_vMesh[0].vTex[0].y==scroll&&fall.m_fCurExp==shape,"split-screen views do not reanimate shared geometry");
   }
  }
  for(int view=0;view<4;view++)for(int flags:{RP_REFLECT|RP_REFRACT,RP_REFLECT,RP_TEXTURE,RP_EMISSIVE}){
   activeView=view;zEnabled=false;zFunction=view%2?8:3;alphaTest=true;
   fixedWorld.m_vPos={500,-400,300};shaderWorld=fixedWorld;int before=draws;
   fsh_DrawLiquidMesh(flags,fall.m_nVtx,fall.m_nIdx/3,fall.m_vMesh,fall.m_pIdx,&color,1,forward,&texture,&texture,&texture);
   require(draws-before==(flags==RP_EMISSIVE?11:2),"all reflective, textured and molten passes submitted");
  }
 }
 for(void* p:allocations)std::free(p);
 std::printf("PASS: %d offline waterfall geometry/draw checks (%d complete draw passes); game not launched.\n",checks,draws);
}
'''


def main():
    header = (ROOT / "ma/Lib/Fang2/fliquid.h").read_text()
    geometry = (ROOT / "ma/Lib/Fang2/fliquid.cpp").read_text()
    render = (ROOT / "ma/Lib/Fang2/dx/fdx8sh.cpp").read_text()
    shaders = (ROOT / "ma/Lib/Fang2/dx/fdx8shaders.h").read_text()
    types = header[header.index("enum\n{"):header.index("FCLASS_ALIGN_PREFIX class CFLiquidVolume")]
    enum_end = shaders.index("} _VERTEX_SHADER_LIST_e;") + len("} _VERTEX_SHADER_LIST_e;")
    shader_enum = shaders[shaders.rfind("typedef enum", 0, enum_end):enum_end]
    declarations = method(shaders, "u32 *_apVShaderDecl[]") + ";\n"
    functions = method(shaders, "u32 *_apVShaderFunc[]") + ";\n"
    names = set(re.findall(r"\b_aDecl_\w+\b", declarations))
    declarations = "\n".join(method(shaders, "u32 " + name + "[]") + ";" if name in
                             ("_aDecl_POS_TEX3", "_aDecl_POS_TEX4") else "u32 " + name + "[]={0xffffffffu};"
                             for name in sorted(names)) + declarations
    funcs = set(re.findall(r"\bdw\w+VertexShader\b", functions))
    functions = "\n".join("u32 " + name + "[]={0};" for name in sorted(funcs)) + functions
    code = FIXTURE + types + shader_enum + declarations + functions + MODELS
    for signature in ("CFLiquidMesh::CFLiquidMesh(", "f32 CFLiquidMesh::EvalFunc(f32 fX)",
                      "f32 CFLiquidMesh::EvalFunc(f32 fX, f32 fExp)", "void CFLiquidMesh::AnimateMesh(",
                      "void CFLiquidMesh::SetupMesh(", "void CFLiquidMesh::Work("):
        code += method(geometry, signature)
    code += method(render, "void _MoveOut(") + method(render, "void fsh_DrawLiquidMesh(") + CHECKS
    OUT.mkdir(parents=True, exist_ok=True)
    (OUT / "waterfalls.cpp").write_text(code)
    (OUT / "CMakeLists.txt").write_text(
        "cmake_minimum_required(VERSION 3.20)\nproject(waterfalls_check LANGUAGES CXX)\n"
        "add_executable(waterfalls_check waterfalls.cpp)\ntarget_compile_features(waterfalls_check PRIVATE cxx_std_17)\n")
    for command in (["cmake", "-S", str(OUT), "-B", str(OUT / "out")],
                    ["cmake", "--build", str(OUT / "out"), "--config", "Release"]):
        run = subprocess.run(command, capture_output=True, text=True)
        if run.returncode:
            raise SystemExit(run.stdout + run.stderr)
    subprocess.run([str(OUT / "out/Release/waterfalls_check.exe")], check=True)


if __name__ == "__main__":
    main()
