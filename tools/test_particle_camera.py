"""Offline near-particle/world-mesh camera regression using production draw code.

Runs only a small C++ fixture; never launches ma_port or accesses saves.
"""
from pathlib import Path
import re
import subprocess

ROOT = Path(__file__).resolve().parent.parent
OUT = ROOT / "build/test-particle-camera"


def method(source, signature):
    start = source.index(signature)
    brace = source.index("{", start)
    depth, end = 1, brace + 1
    while depth:
        depth += (source[end] == "{") - (source[end] == "}")
        end += 1
    return source[start:end]


FIXTURE = r'''
#define NOMINMAX
#include "d3dx8.h"
#include "fdx8vshader_const.h"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <vector>
using u32=unsigned;using f32=float;using FViewportPlanesMask_t=int;
#define FANG_PLATFORM_WIN 1
#define FANG_PLATFORM_XB 0
#define FPERF_ENABLE 0
#define FINLINE inline
#define FDX8_CACHE_STATES 1
#define FASSERT(x) do{if(!(x)){std::printf("ASSERT: %s\n",#x);std::exit(2);}}while(0)
#define FASSERT_NOW FASSERT(false)
#define FMATH_CLAMPMAX(x,y) (x=std::min(x,y))
#define FMATH_MIN(x,y) std::min(x,y)
#define FDX8_RGBA_TO_DXCOLOR(x) 0xffffffff
struct CFVec3 {float x,y,z;};
struct CFVec3A {
 float x,y,z,w;
 void Sub(const CFVec3& a,const CFVec3A& b){x=a.x-b.x;y=a.y-b.y;z=a.z-b.z;}
 float MagSq(){return x*x+y*y+z*z;}
};
void identity(D3DXMATRIX& m){std::memset(&m,0,sizeof m);m._11=m._22=m._33=m._44=1;}
D3DXVECTOR3 transform(CFVec3 v,const D3DXMATRIX& m){
 float w=v.x*m._14+v.y*m._24+v.z*m._34+m._44;
 return {(v.x*m._11+v.y*m._21+v.z*m._31+m._41)/w,
         (v.x*m._12+v.y*m._22+v.z*m._32+m._42)/w,
         (v.x*m._13+v.y*m._23+v.z*m._33+m._43)/w};
}
struct CFMtx44A {
 float aa[4][4];
 void ReceiveTranspose(const CFMtx44A& a){D3DXMatrixTranspose((D3DXMATRIX*)this,(const D3DXMATRIX*)&a);}
 void Mul(const CFMtx44A& a,const CFMtx44A& b){D3DXMatrixMultiply((D3DXMATRIX*)this,(const D3DXMATRIX*)&a,(const D3DXMATRIX*)&b);}
};
struct CFMtx43A {
 union{float aa[4][4];CFMtx44A m44a;struct{CFVec3A m_vX,m_vY,m_vZ,m_vPos;};};
 static CFMtx43A m_IdentityMtx;
 CFMtx43A(){Identity();}
 void Identity(){identity(*(D3DXMATRIX*)this);}
 void Set(const CFMtx43A& a){*this=a;}
 void Mul(const CFMtx43A& a){D3DXMatrixMultiply((D3DXMATRIX*)this,(D3DXMATRIX*)this,(const D3DXMATRIX*)&a);}
};
CFMtx43A CFMtx43A::m_IdentityMtx;
struct CFXfm {
 CFMtx43A m_MtxF,m_MtxR;float m_fScaleF=1;
 void TransformPointF(CFVec3& dst,const CFVec3& src){
  auto out=transform(src,*(const D3DXMATRIX*)&m_MtxF);
  dst={out.x,out.y,out.z};
 }
 static void PopModel(){}
};
CFXfm view,modelView;CFXfm* FXfm_pView=&view;CFXfm* FXfm_pModelView=&modelView;
CFMtx43A* FXfm_pMirrorMtx=nullptr;
D3DXMATRIX FDX8_MatrixIdentity,FDX8_CurrentViewMtx,FDX8_InvCurrentViewMtx,FDX8_CurrentProjMtx;
CFMtx43A FDX8_CurrentFangViewMtx,FDX8_CurrentFangProjMtx;
BOOL FDX8_bViewMtxIsIdentity=FALSE;
BOOL _bWindowCreated=TRUE;
struct FDX8VB_C1T1_t {float fPosX,fPosY,fPosZ;u32 nDiffuseRGBA;float fS0,fT0;};
struct FDX8VB_t {std::vector<FDX8VB_C1T1_t> vertices;};
FDX8VB_t vb;FDX8VB_t* _pEmulationVB=&vb;
u32 _nCurrentEmulationVB=0,_nCurrentEmulationVBVtxIndex=0,_nVertexCount=48,_nMaxD3DDrawPrimElements=48;
struct Device {
 D3DXMATRIX world,view;float constants[128][4];u32 draws=0;
 std::vector<FDX8VB_C1T1_t> drawn;
 HRESULT SetTransform(D3DTRANSFORMSTATETYPE type,const D3DMATRIX* m){
  (type==D3DTS_VIEW?view:world)=D3DXMATRIX(*m);return D3D_OK;
 }
 HRESULT GetTransform(D3DTRANSFORMSTATETYPE type,D3DMATRIX* m){*m=(type==D3DTS_VIEW?view:world);return D3D_OK;}
 HRESULT SetVertexShaderConstant(u32 reg,const void* p,u32 count){std::memcpy(&constants[reg],p,count*16);return D3D_OK;}
 void DrawPrimitive(D3DPRIMITIVETYPE,u32 first,u32 count){
  FASSERT(first+count*3<=vb.vertices.size());
  drawn.insert(drawn.end(),vb.vertices.begin()+first,vb.vertices.begin()+first+count*3);draws++;
 }
}device;Device* FDX8_pDev=&device;
void fdx8vb_Select(FDX8VB_t*){}
void* fdx8vb_Lock(FDX8VB_t* p,u32 first,u32 count){p->vertices.resize(first+count);return p->vertices.data()+first;}
void fdx8vb_Unlock(FDX8VB_t*){}
void fdx8_SetRenderState_VERTEXBLEND(u32){}
struct CFTexInst{bool GetTexDef(){return true;}};
void fdx8tex_SetTexture(int,CFTexInst*,int){}
void frenderer_Fog_ComputeColor(){}
constexpr int _TEXSTAGE=0,FRENDERER_DRAW=0;
bool FRenderer_bFogEnabled=false,FRenderer_bDrawBoundInfo=false;
int FRenderer_nD3DFillMode=0;
void frenderer_Push(int,void*){}void frenderer_Pop(){}
void fdraw_FacetedWireSphere(CFVec3*,float,int,int){}void fdraw_ModelSpaceAxis(float){}
struct FViewport_t {float fNearZ=.2f;struct {float x=1;}OOHalfRes;};
FViewport_t viewport;FViewport_t* fviewport_GetActive(){return &viewport;}
constexpr int FPSPRITE_FLAG_ENABLE_DEPTH_WRITES=1,FPSPRITE_FLAG_NO_FOG=2,FPSPRITE_FLAG_SIZE_IN_PIXELS=4;
constexpr int FPSPRITE_BLEND_MODULATE=0,FPSPRITE_BLEND_ADD=1;
struct FPSprite_t {CFVec3 Point_MS;float fDim_MS;u32 ColorRGBA;};
struct CFPSpriteGroup {
 u32 m_nPSFlags=0,m_nBlend=0,m_nRenderCount=0,m_nRenderStartIndex=0,m_nMaxCount=0;
 CFTexInst m_TexInst;struct{CFVec3 m_Pos;float m_fRadius;}m_BoundSphere_MS;
 FPSprite_t* m_pBase;
 float m_fEmulationDistance=0;
 BOOL UseEmulated();
 FViewportPlanesMask_t _RenderEmulatedGroup(BOOL,FViewportPlanesMask_t);
};
BOOL FPSprite_bEmulated=FALSE;
void fdx8xfm_SetViewDXMatrix(BOOL);
void fdx8xfm_SetCustomDXMatrix(D3DTRANSFORMSTATETYPE,const CFMtx43A*,BOOL);
'''

CHECKS = r'''
int checks=0;
void require(bool b,const char* m){checks++;if(!b){std::printf("FAIL: %s\n",m);std::exit(1);}}
bool same(const D3DXMATRIX& a,const D3DXMATRIX& b){
 for(int i=0;i<16;i++)if(std::abs(((const float*)&a)[i]-((const float*)&b)[i])>0.0001f)return false;
 return true;
}
D3DXVECTOR3 project(CFVec3 p,const D3DXMATRIX& m){
 return transform(p,m);
}
int main(){
 identity(FDX8_MatrixIdentity);std::memset(&FDX8_CurrentProjMtx,0,sizeof FDX8_CurrentProjMtx);
 FDX8_CurrentProjMtx._11=1/(std::tan(.55f)*2.6f);FDX8_CurrentProjMtx._22=1/std::tan(.55f);
 FDX8_CurrentProjMtx._33=1000.f/999.8f;FDX8_CurrentProjMtx._34=1;FDX8_CurrentProjMtx._43=-.2f*1000.f/999.8f;
 D3DXMatrixTranspose((D3DXMATRIX*)&FDX8_CurrentFangProjMtx,&FDX8_CurrentProjMtx);
 const CFVec3 fall={-8.259f,20.f,54.353f};
 for(int player=0;player<4;player++)for(int frame=0;frame<12;frame++)for(int mirrored:{0,1}){
  const CFVec3 eye={10+player*5.f,10+frame*.1f,-15+frame*3.f};
  float angle=(player-1)*.07f+frame*.01f,c=std::cos(angle),s=std::sin(angle);
  auto& camera=*(D3DXMATRIX*)&view.m_MtxF;identity(camera);
  camera._11=c;camera._13=s;camera._31=-s;camera._33=c;
  camera._41=-(eye.x*c-eye.z*s);camera._42=-eye.y;camera._43=-(eye.x*s+eye.z*c);
  D3DXMatrixInverse((D3DXMATRIX*)&view.m_MtxR,nullptr,&camera);
  CFMtx43A mirror;mirror.aa[0][0]=-1;mirror.aa[3][0]=6;
  FXfm_pMirrorMtx=mirrored?&mirror:nullptr;
  D3DXMATRIX expectedView(camera),expectedClip;
  if(mirrored)D3DXMatrixMultiply(&expectedView,&camera,(const D3DXMATRIX*)&mirror);
  D3DXMatrixMultiply(&expectedClip,&expectedView,&FDX8_CurrentProjMtx);
  device.world=FDX8_MatrixIdentity;
  fdx8xfm_SetViewDXMatrix(TRUE);fxfm_SetViewAndWorldSpaceModelMatrices();
  // Distant hardware point sprites leave the camera untouched.
  D3DXMATRIX clip;D3DXMatrixTranspose(&clip,(const D3DXMATRIX*)&device.constants[CV_WORLDVIEWPROJ_0]);
  require(same(clip,expectedClip),"distant world waterfall uses this player's camera");
  // Nearby particle draws use view-space quads. Include visible and behind-camera
  // particles, ring-buffer wrap and an empty draw, followed immediately by world geo.
  modelView.m_MtxF=view.m_MtxF;
  FPSprite_t sprites[]={{{-8,10,54},3,0xffffffff},{{10,10,-100},2,0xffffffff},{{-5,12,50},4,0xffffffff}};
  CFPSpriteGroup group;group.m_pBase=sprites;group.m_nMaxCount=3;
  group.m_fEmulationDistance=5;
  group.m_BoundSphere_MS.m_Pos={eye.x+6,eye.y,eye.z};
  require(!group.UseEmulated(),"distant particles keep hardware rendering");
  group.m_BoundSphere_MS.m_Pos={eye.x+4,eye.y,eye.z};
  require(group.UseEmulated(),"approaching emitter switches to view-space quads");
  for(int count:{3,1,0})for(int start:{0,2}){
   group.m_nRenderCount=count;group.m_nRenderStartIndex=start;
   device.drawn.clear();group._RenderEmulatedGroup(FALSE,0);
   require(same(FDX8_CurrentViewMtx,expectedView)&&same(device.view,expectedView),"near particles restore the active graphics camera");
   fxfm_SetViewAndWorldSpaceModelMatrices();
   D3DXMatrixTranspose(&clip,(const D3DXMATRIX*)&device.constants[CV_WORLDVIEWPROJ_0]);
   require(same(clip,expectedClip),"world mesh after nearby mist uses current view-projection");
   const auto actual=project(fall,clip),want=project(fall,expectedClip);
   require(std::abs(actual.x-want.x)<.0001f&&std::abs(actual.y-want.y)<.0001f,"waterfall screen position matches world anchor");
   if(count==3){
    require(device.drawn.size()==12,"near renderer draws two visible sprites and skips one behind camera");
    CFVec3 transformed;modelView.TransformPointF(transformed,sprites[start].Point_MS);
    if(transformed.z>viewport.fNearZ){
     const auto& a=device.drawn[0];const auto& b=device.drawn[2];
     require(std::abs((a.fPosX+b.fPosX)*.5f-transformed.x)<.0001f,"particle quads remain centered on world emitter");
    }
   }
  }
  // Other screen-space passes or stale player matrices cannot contaminate world setup.
  FDX8_SetViewMatrixIdentity();device.world._41=200;
  fxfm_SetViewAndWorldSpaceModelMatrices();
  D3DXMatrixTranspose(&clip,(const D3DXMATRIX*)&device.constants[CV_WORLDVIEWPROJ_0]);
  require(same(clip,expectedClip),"world setup restores view even after another screen-space pass");
  require(same(device.world,FDX8_MatrixIdentity),"world setup clears stale fixed-function model transform");
 }
 std::printf("PASS: %d offline particle/camera projection checks; game not launched.\n",checks);
}
'''


def main():
    particle = (ROOT / "ma/Lib/Fang2/dx/fdx8psprite.cpp").read_text()
    xfm = (ROOT / "ma/Lib/Fang2/dx/fdx8xfm.cpp").read_text()
    dx = (ROOT / "ma/Lib/Fang2/dx/fdx8.h").read_text()
    emulated = method(particle, "FViewportPlanesMask_t CFPSpriteGroup::_RenderEmulatedGroup(")
    code = FIXTURE
    render = method(particle, "FViewportPlanesMask_t CFPSpriteGroup::Render(")
    selection = render[render.index("BOOL bUseEmulatedPath"):render.index("m_Xfm.PushModel();", render.index("BOOL bUseEmulatedPath"))]
    code += "BOOL CFPSpriteGroup::UseEmulated(){CFVec3A TempVec3A;" + selection + "return bUseEmulatedPath;}\n"
    for name in sorted(set(re.findall(r"(fdx8_(?:SetRenderState|SetTextureState)_\w+)\(", emulated))):
        if name != "fdx8_SetRenderState_VERTEXBLEND":
            code += f"template<class... T>void {name}(T...){{}}\n"
    for signature in ("FINLINE HRESULT FDX8_SetViewMatrixIdentity(", "FINLINE HRESULT FDX8_SetViewMatrix("):
        code += method(dx, signature)
    code += method(xfm, "static FINLINE void _ConvertXfmToD3DMtx( D3DMATRIX *pDestD3DMtx, const CFMtx43A *pMtx43A ) \n{")
    for signature in ("void fdx8xfm_SetViewDXMatrix(", "void fdx8xfm_SetCustomDXMatrix( D3DTRANSFORMSTATETYPE nD3DMatrixType, const CFMtx43A *pMtx43A, BOOL bUsingVertexShaders )", "void fxfm_SetViewAndWorldSpaceModelMatrices("):
        code += method(xfm, signature)
    code += emulated + CHECKS
    OUT.mkdir(parents=True, exist_ok=True)
    (OUT / "particles.cpp").write_text(code)
    (OUT / "CMakeLists.txt").write_text(
        "cmake_minimum_required(VERSION 3.20)\nproject(particle_camera LANGUAGES CXX)\n"
        "add_executable(particle_camera particles.cpp)\ntarget_compile_features(particle_camera PRIVATE cxx_std_17)\n"
        f'target_include_directories(particle_camera PRIVATE "{ROOT.as_posix()}/port/compat" "{ROOT.as_posix()}/ma/Lib/Fang2/dx")\n')
    for command in (["cmake", "-S", str(OUT), "-B", str(OUT / "out"), "-A", "Win32"],
                    ["cmake", "--build", str(OUT / "out"), "--config", "Release"]):
        run = subprocess.run(command, capture_output=True, text=True)
        if run.returncode:
            raise SystemExit(run.stdout + run.stderr)
    subprocess.run([str(OUT / "out/Release/particle_camera.exe")], check=True)


if __name__ == "__main__":
    main()
