"""Exercise the production booth timeout with original bytecode in the real AMX VM."""
from pathlib import Path
import subprocess

ROOT = Path(__file__).resolve().parent.parent
OUT = ROOT / 'build/test-booth-fallback'
CODE = r'''
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <string>
#include <cstdint>
#define __STDC_VERSION__ 199901L
#include "pc_booth_fallback.h"
int checks=0,ends=0,closes=0,deactivates=0,saves=0,gotos=0,work=1;
float now=0,delay=0,camEnd=8,speechEnd=0,closeAt=-1,deactivateAt=-1,endAt=-1;
bool active=false;
std::vector<std::string> names;
void require(bool b,const char* text){++checks;if(!b){std::fprintf(stderr,"FAIL: %s\n",text);std::exit(1);}}
int id(const char* name){for(size_t i=0;i<names.size();++i)if(names[i]==name)return int(i)+100;names.push_back(name);return int(names.size())+99;}
std::string str(AMX* amx,cell value){cell* p;char s[128];require(amx_GetAddr(amx,value,&p)==AMX_ERR_NONE,"valid string");amx_GetString(s,p);return s;}
int AMXAPI native(AMX* amx,cell index,cell* result,cell* p){
 auto* h=(AMX_HEADER*)amx->base;const char* n=((AMX_FUNCSTUB*)(amx->base+h->natives))[index].name;*result=0;
 if(!std::strcmp(n,"E_Find")||!std::strcmp(n,"Door_Find"))*result=id(str(amx,p[1]).c_str());
 else if(!std::strcmp(n,"event_TranslateName"))*result=id(str(amx,p[1]).c_str());
 else if(!std::strcmp(n,"event_SetWork"))work=p[1];
 else if(!std::strcmp(n,"event_DelayWork"))std::memcpy(&delay,&p[1],4);
 else if(!std::strcmp(n,"Bot_GetPlayer"))*result=999;
 else if(!std::strcmp(n,"FV3Points1_Acquire")||!std::strcmp(n,"FQuatTang1_Acquire"))*result=1000;
 else if(!std::strcmp(n,"f32_SetFromInt")){float f=float(p[1]);std::memcpy(result,&f,4);}
 else if(!std::strcmp(n,"f32_SetFromString")){float f=std::strtof(str(amx,p[1]).c_str(),nullptr);std::memcpy(result,&f,4);}
 else if(!std::strcmp(n,"Door_GetUnitPos")){float f=1;std::memcpy(result,&f,4);}
 else if(!std::strcmp(n,"Door_GotoPos")&&p[1]==id("areadoor7")&&p[2]==0){++closes;closeAt=now;}
 else if(!std::strcmp(n,"Speech_Play"))speechEnd=now+6.3f;
 else if(!std::strcmp(n,"FVec3Obj_Start"))camEnd=now+8;
 else if(!std::strcmp(n,"FVec3Obj_IsDone"))*result=now>=camEnd;
 else if(!std::strcmp(n,"Game_BeginCutScene"))active=true;
 else if(!std::strcmp(n,"Cam_Deactivate")){++deactivates;deactivateAt=now;}
 else if(!std::strcmp(n,"Game_EndCutScene")){++ends;active=false;endAt=now;}
 else if(!std::strcmp(n,"Checkpoint_Save"))++saves;
 else if(!std::strcmp(n,"Bot_GotoE"))++gotos;
 return AMX_ERR_NONE;
}
void exec(AMX& a,const char* fn,int event=0,int tripwire=0,int actor=0){
 int index;cell result;require(amx_FindPublic(&a,const_cast<char*>(fn),&index)==AMX_ERR_NONE,"public found");
 int status=event?amx_Exec(&a,&result,index,5,0,event,0,tripwire,actor):amx_Exec(&a,&result,index,1,0);
 require(status==AMX_ERR_NONE,"original bytecode executes");
}
std::vector<unsigned char> program(const char* path){
 FILE* f=std::fopen(path,"rb");require(f,"retail script opened");AMX_HEADER h;
 require(std::fread(&h,sizeof(h),1,f)==1,"header read");std::vector<unsigned char> b(h.stp);
 std::rewind(f);require(std::fread(b.data(),1,h.size,f)==size_t(h.size),"script read");std::fclose(f);return b;
}
void scene(const char* path,int arrived,bool late,bool slowCamera,bool slowSpeech){
 auto b=program(path);AMX a{};require(amx_Init(&a,b.data())==AMX_ERR_NONE,"production AMX initialization");a.callback=native;
 auto* h=(AMX_HEADER*)b.data();cell* g=(cell*)(b.data()+h->dat);unsigned size=h->hea-h->dat;
 require(PortBoothArrivalLayout("rsboothdoor.sma",&a,size),"recognized actual expanded retail layout");
 std::vector<unsigned char> originalCode(b.begin()+h->cod,b.begin()+h->dat);
 names.clear();now=delay=0;ends=closes=deactivates=saves=gotos=0;active=false;work=1;
 exec(a,"OnInit");for(int i=0;i<4;++i)exec(a,"Work"); // Let the unrelated opening lift finish normally.
 ends=closes=deactivates=saves=gotos=0;delay=0;closeAt=deactivateAt=endAt=-1;
 exec(a,"OnEvent",id("switch"),id("area7switch"));require(active,"authored switch begins scene");
 float wait=0,waitStart=-1,fallbackAt=-1;int fallbacks=0;bool delivered=false,deliveredLate=false;
 for(int tick=0;tick<5000&&!ends;++tick){
  const float dt=1.f/60;now+=dt;if(delay>0)delay-=dt;
  if(work&&delay<=0)exec(a,"Work");
  if(g[32/4]==-1&&active&&waitStart<0){waitStart=now;if(slowCamera)camEnd=now+30;if(slowSpeech)speechEnd=now+31;}
  if(waitStart>=0&&!delivered&&now-waitStart>1){
   const char* bots[]={"elite1","elite2","elite_lacky1","elite_lacky2"};
   for(int i=0;i<arrived;++i)exec(a,"OnEvent",id("tripwire"),id("trigger_group"),id(bots[i]));delivered=true;
  }
  cell saved[30];std::memcpy(saved,g,sizeof(saved));
  if(PortBoothArrivalFallback("rsboothdoor.sma",&a,g,size,dt,active,now>=camEnd&&now>=speechEnd,wait)){
   ++fallbacks;fallbackAt=now;require(ends==0&&closes==0&&deactivates==0,"fallback itself never closes camera or ends scene");
   require(now-waitStart>=14.99f&&now>=camEnd&&now>=speechEnd,"grace, camera and dialogue complete before release");
   require(g[32/4]==4&&g[92/4]==4,"only resume the authored finishing state");
   for(int i=0;i<30;++i)if(i!=8&&i!=23)require(g[i]==saved[i],"unrelated script state unchanged");
  }
  if(late&&fallbackAt>=0&&!deliveredLate){exec(a,"OnEvent",id("tripwire"),id("trigger_group"),id("elite_lacky2"));deliveredLate=true;}
 }
 require(ends==1&&closes==1&&deactivates==1&&saves==1,"original ending closes door, releases camera and checkpoints once");
 require(gotos>=4&&g[32/4]==-1&&!active,"guard orders retained and normal scene state restored");
 require(deactivateAt-closeAt>=.99f&&endAt-deactivateAt>=.49f,"original one-second and half-second ending delays retained");
 require(fallbacks==(arrived<4?1:0),"timeout used only for missing guards");
 if(fallbackAt>=0)require(endAt-fallbackAt>=1.49f,"no abrupt timeout cut");
 require(!std::memcmp(originalCode.data(),b.data()+h->cod,originalCode.size()),"retail code never patched or skipped");
 std::printf("PASS scene: arrived=%d late=%d delayedCamera=%d delayedSpeech=%d fallbacks=%d end=%.2f\n",arrived,late,slowCamera,slowSpeech,fallbacks,endAt);
}
int main(int argc,char** argv){
 require(argc==2,"retail path supplied");
 scene(argv[1],4,false,false,false);scene(argv[1],3,false,false,false);
 scene(argv[1],0,false,false,false);scene(argv[1],3,true,false,false);
 scene(argv[1],3,false,true,false);scene(argv[1],3,false,false,true);
 auto b=program(argv[1]);AMX a{};require(amx_Init(&a,b.data())==AMX_ERR_NONE,"exclusion VM init");auto* h=(AMX_HEADER*)b.data();cell* g=(cell*)(b.data()+h->dat);unsigned size=h->hea-h->dat;
 for(int exclusion=0;exclusion<9;++exclusion){
  std::memset(g,0,120);g[8]=-1;g[27]=1;float wait=14.9f;
  const char* name="rsboothdoor.sma";BOOL allowed=TRUE;unsigned length=size;AMX* vm=&a;void* data=g;
  cell backup=((cell*)(b.data()+h->cod))[0x884/4];
  switch(exclusion){case 0:name="other.sma";break;case 1:allowed=FALSE;break;case 2:length=116;break;case 3:vm=nullptr;break;case 4:data=nullptr;break;case 5:g[27]=0;break;case 6:g[8]=3;break;case 7:g[23]=4;break;case 8:((cell*)(b.data()+h->cod))[0x884/4]=5;break;}
  require(!PortBoothArrivalFallback(name,vm,data,length,20,allowed,TRUE,wait)&&g[8]!=4,"unrelated or invalid state is untouched");
  ((cell*)(b.data()+h->cod))[0x884/4]=backup;
 }
 std::memset(g,0,120);g[8]=-1;g[27]=1;float wait=0;
 for(int i=0;i<100;++i)require(!PortBoothArrivalFallback("rsboothdoor.sma",&a,g,size,0,TRUE,TRUE,wait)&&wait==0,"paused frames do not count toward timeout");
 std::printf("PASS: %d booth fallback VM checks; no game launched.\n",checks);
}
'''


def main():
    OUT.mkdir(parents=True, exist_ok=True)
    (OUT/'fclib.h').write_text('#pragma once\n#include <string.h>\ntypedef bool BOOL;\n#define TRUE true\n#define FALSE false\ninline int fclib_stricmp(const char* a,const char* b){return _stricmp(a,b);}\n')
    (OUT/'fallback.cpp').write_text(CODE)
    (OUT/'CMakeLists.txt').write_text(
        'cmake_minimum_required(VERSION 3.20)\nproject(booth_fallback C CXX)\n'
        f'add_executable(fallback fallback.cpp "{(ROOT/"ma/Lib/SmallAMX/amx.c").as_posix()}")\n'
        f'target_include_directories(fallback PRIVATE "{OUT.as_posix()}" "{(ROOT/"port").as_posix()}" "{(ROOT/"ma/Lib/SmallAMX").as_posix()}")\n'
        'target_compile_features(fallback PRIVATE cxx_std_14)\n'
        'target_compile_definitions(fallback PRIVATE WIN32 _CRT_SECURE_NO_WARNINGS)\n')
    for cmd in (['cmake','-S',str(OUT),'-B',str(OUT/'out'),'-A','Win32'],['cmake','--build',str(OUT/'out'),'--config','Release']):
        result = subprocess.run(cmd,capture_output=True,text=True)
        if result.returncode:
            raise SystemExit(result.stdout+result.stderr)
    subprocess.run([str(OUT/'out/Release/fallback.exe'),str(ROOT/'gamedata/mst/rsboothdoor.sma')],check=True)


if __name__ == '__main__':
    main()
