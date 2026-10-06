"""Hidden-window D3D9 cutout regression; no game launch or save access.
Reproduces black fence holes with the legacy tolerant multipass depth path, then
verifies the production surface alpha helper preserves them. Uses real pixel shaders.
"""
from pathlib import Path
import subprocess
from build_shaders import assemble
from test_coop_checkpoint_rat import method
ROOT = Path(__file__).resolve().parents[1]
OUT = ROOT / 'build/test-cutout-surface'
CPP = r'''

#include <windows.h>
#include <d3d9.h>
#include <cstdio>
#include <cstdlib>
/* PRODUCTION_SHADERS */
/* PRODUCTION_HELPERS */
void check(HRESULT h,const char* s){if(FAILED(h)){printf("FAIL %s %08x\n",s,(unsigned)h);exit(1);}}
struct Vertex{float x,y,z,w;DWORD color;float u,v;};
int main(int argc,char**){
 HWND window=CreateWindowA("STATIC","Offline shader test",WS_POPUP,0,0,64,64,0,0,GetModuleHandle(0),0);
 IDirect3D9* api=Direct3DCreate9(D3D_SDK_VERSION);IDirect3DDevice9* device=nullptr;
 D3DPRESENT_PARAMETERS pp{};pp.Windowed=TRUE;pp.SwapEffect=D3DSWAPEFFECT_DISCARD;pp.hDeviceWindow=window;
 pp.EnableAutoDepthStencil=TRUE;pp.AutoDepthStencilFormat=D3DFMT_D16;pp.BackBufferWidth=64;pp.BackBufferHeight=64;pp.BackBufferFormat=D3DFMT_X8R8G8B8;
 check(api->CreateDevice(0,D3DDEVTYPE_HAL,window,D3DCREATE_SOFTWARE_VERTEXPROCESSING,&pp,&device),"device");
 active=device;
 const unsigned cutouts[]={FSHADERS_cBASE,FSHADERS_cBASE_DETAIL,FSHADERS_cBASE_LERP_tLAYER,FSHADERS_cBASE_LERP_vLAYER,FSHADERS_cBASE_LERP_pLAYER,FSHADERS_cBASE_LERP_tLAYER_DETAIL,FSHADERS_cBASE_LERP_vLAYER_DETAIL,FSHADERS_cBASE_LERP_pLAYER_DETAIL};
 for(int pass=0;pass<3;pass++)for(unsigned sid=0;sid<=FSHADERS_SHADER_COUNT;sid++){
  FSh_shaderType=pass;_nSurfaceShaderID=sid;device->SetRenderState(D3DRS_ALPHATESTENABLE,FALSE);_SetSurfaceCutoutTest();
  bool expected=false;for(auto cutout:cutouts)if(pass==SHADERTYPE_SURFACE&&sid==cutout)expected=true;
  DWORD enabled=0,ref=0,func=0;device->GetRenderState(D3DRS_ALPHATESTENABLE,&enabled);device->GetRenderState(D3DRS_ALPHAREF,&ref);device->GetRenderState(D3DRS_ALPHAFUNC,&func);
  if(!!enabled!=expected||(expected&&(ref!=127||func!=D3DCMP_GREATER))){puts("FAIL surface cutout selection");return 2;}
 }
 FSh_shaderType=SHADERTYPE_SURFACE;_nSurfaceShaderID=FSHADERS_cBASE;
 IDirect3DPixelShader9* shader=nullptr;check(device->CreatePixelShader(dwFdx8ColorMaskPixelShader,&shader),"production pixel shader");
 IDirect3DTexture9* tex=nullptr;check(device->CreateTexture(2,1,1,0,D3DFMT_A8R8G8B8,D3DPOOL_MANAGED,&tex,0),"texture");
 D3DLOCKED_RECT lock{};check(tex->LockRect(0,&lock,0,0),"lock");((DWORD*)lock.pBits)[0]=0x00000000;((DWORD*)lock.pBits)[1]=0xff404040;tex->UnlockRect(0);
 device->SetTexture(0,tex);device->SetSamplerState(0,D3DSAMP_MINFILTER,D3DTEXF_POINT);device->SetSamplerState(0,D3DSAMP_MAGFILTER,D3DTEXF_POINT);
 device->SetPixelShader(shader);device->SetFVF(D3DFVF_XYZRHW|D3DFVF_DIFFUSE|D3DFVF_TEX1);
 device->SetRenderState(D3DRS_CULLMODE,D3DCULL_NONE);device->SetRenderState(D3DRS_ZENABLE,TRUE);
 device->SetRenderState(D3DRS_ALPHATESTENABLE,TRUE);device->SetRenderState(D3DRS_ALPHAFUNC,D3DCMP_GREATER);device->SetRenderState(D3DRS_ALPHAREF,1);
 device->Clear(0,0,D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER,0xff0000ff,1,0);device->BeginScene();
 Vertex vs[]={{-.5f,-.5f,.5f,1,0xffffffff,0,0},{63.5f,-.5f,.5f,1,0xffffffff,1,0},{-.5f,63.5f,.5f,1,0xffffffff,0,1},{63.5f,63.5f,.5f,1,0xffffffff,1,1}};
 check(device->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP,2,vs,sizeof(Vertex)),"draw");
 IDirect3DPixelShader9* surface=nullptr;check(device->CreatePixelShader(dwFdx8PassThruPixelShader,&surface),"surface shader");
 device->SetPixelShader(surface);float ones[4]={1,1,1,1};device->SetPixelShaderConstantF(2,ones,1);device->SetPixelShaderConstantF(3,ones,1);
 device->SetRenderState(D3DRS_ZWRITEENABLE,FALSE);device->SetRenderState(D3DRS_ZFUNC,D3DCMP_LESSEQUAL);
 device->SetRenderState(D3DRS_ALPHATESTENABLE,FALSE);if(argc>1)_SetSurfaceCutoutTest();
 device->SetRenderState(D3DRS_ALPHABLENDENABLE,TRUE);device->SetRenderState(D3DRS_SRCBLEND,D3DBLEND_DESTCOLOR);device->SetRenderState(D3DRS_DESTBLEND,D3DBLEND_SRCCOLOR);
 check(device->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP,2,vs,sizeof(Vertex)),"surface draw");device->EndScene();surface->Release();
 IDirect3DSurface9 *target=nullptr,*copy=nullptr;device->GetRenderTarget(0,&target);
 device->CreateOffscreenPlainSurface(64,64,D3DFMT_X8R8G8B8,D3DPOOL_SYSTEMMEM,&copy,0);
 check(device->GetRenderTargetData(target,copy),"readback");check(copy->LockRect(&lock,0,D3DLOCK_READONLY),"readlock");
 DWORD left=((DWORD*)((char*)lock.pBits+32*lock.Pitch))[8],right=((DWORD*)((char*)lock.pBits+32*lock.Pitch))[48];
 printf("production ColorMask: alpha-zero=%08x alpha-full=%08x\n",left,right);
 if((left&0xffffff)!=0xff||(right&0xffffff)!=0x808080){puts("FAIL fence cutout surface regression");return 1;}
 puts("PASS fence cutout shader; game not launched");copy->UnlockRect();copy->Release();target->Release();shader->Release();tex->Release();device->Release();api->Release();DestroyWindow(window);return 0;
}

'''
HELPERS = r'''
#define FSH_DYNAMIC_SREFLECT 1
#define BOOL int
#define TRUE 1
#define FALSE 0
using u32=unsigned;

'''
DEVICE_HELPERS = r'''

constexpr int SHADERTYPE_SURFACE=1;
int FSh_shaderType=SHADERTYPE_SURFACE;unsigned _nSurfaceShaderID=FSHADERS_cBASE;
IDirect3DDevice9* active=nullptr;
void fdx8_SetRenderState_ALPHATESTENABLE(DWORD v){active->SetRenderState(D3DRS_ALPHATESTENABLE,v);}
void fdx8_SetRenderState_ALPHAFUNC(DWORD v){active->SetRenderState(D3DRS_ALPHAFUNC,v);}
void fdx8_SetRenderState_ALPHAREF(DWORD v){active->SetRenderState(D3DRS_ALPHAREF,v);}

'''
def main():
    source = (ROOT / 'ma/Lib/Fang2/dx/fdx8sh.cpp').read_text()
    header = (ROOT / 'ma/Lib/Fang2/fshaders.h').read_text()
    start = header.rfind('typedef enum', 0, header.index('FSHADERS_oBASE ='))
    end = header.index(';', header.index('}', start)) + 1
    helpers = HELPERS + header[start:end] + DEVICE_HELPERS
    helpers += method(source, 'static BOOL _SurfaceNeedsCutoutTest(')
    helpers += method(source, 'static void _SetSurfaceCutoutTest(')
    shaders = ''
    for name in ('fdx8ColorMask', 'fdx8PassThru'):
        words = assemble(str(ROOT / ('ma/Lib/Fang2/dx/' + name + '.nvp')))
        shaders += 'const DWORD dw' + name[0].upper() + name[1:] + 'PixelShader[]={' + ','.join(hex(w) for w in words) + '};\n'
    code = CPP.replace('/* PRODUCTION_HELPERS */', helpers).replace('/* PRODUCTION_SHADERS */', shaders)
    OUT.mkdir(parents=True, exist_ok=True)
    (OUT / 'cutout.cpp').write_text(code)
    (OUT / 'CMakeLists.txt').write_text('cmake_minimum_required(VERSION 3.20)\nproject(cutout LANGUAGES CXX)\nadd_executable(cutout cutout.cpp)\ntarget_link_libraries(cutout PRIVATE d3d9)\n')
    for cmd in (['cmake', '-S', str(OUT), '-B', str(OUT / 'out'), '-A', 'Win32'], ['cmake', '--build', str(OUT / 'out'), '--config', 'Release']):
        result = subprocess.run(cmd, capture_output=True, text=True)
        if result.returncode:
            raise SystemExit(result.stdout + result.stderr)
    exe = str(OUT / 'out/Release/cutout.exe')
    old = subprocess.run([exe], capture_output=True, text=True)
    if old.returncode != 1 or 'alpha-zero=00000000' not in old.stdout:
        raise SystemExit('Legacy black-hole reproduction failed: ' + old.stdout + old.stderr)
    fixed = subprocess.run([exe, 'fixed'], capture_output=True, text=True)
    if fixed.returncode:
        raise SystemExit(fixed.stdout + fixed.stderr)
    print('PASS: production GPU shaders reproduce legacy black fence holes; surface alpha fix preserves transparent holes. All surface/pass selections checked; game not launched.')
if __name__ == '__main__':
    main()
