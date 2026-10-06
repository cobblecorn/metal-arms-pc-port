"""Execute retail asylum script in the production AMX VM, without the game."""
from pathlib import Path
import subprocess

ROOT = Path(__file__).resolve().parent.parent
OUT = ROOT / 'build/test-asylum-kill-goal'

CODE = r'''
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "pc_script_goals.h"
int checks=0,finds=0,work=0,speeches=0,opens=0,begins=0,ends=0,saves=0;
void require(bool ok,const char* why){++checks;if(!ok){std::fprintf(stderr,"FAIL: %s\n",why);std::exit(1);}}
const char* roster[]={"wildman1","wildman2","wildman3","wildman4","wildman5","wildman6",
 "loon1","loon2","loon3","loon4","titan1","titan2","titan3","titan4"};
int AMXAPI native(AMX* amx,cell index,cell* result,cell* params){
 auto* h=(AMX_HEADER*)amx->base;
 auto* f=(AMX_FUNCSTUB*)(amx->base+h->natives)+index;
 const char* name=f->name;*result=0;
 if(!std::strcmp(name,"E_Find")){
  cell* addr;char text[100];amx_GetAddr(amx,params[1],&addr);amx_GetString(text,addr);
  *result=100;
  for(int i=0;i<14;i++)if(!_stricmp(text,roster[i])){*result=i+1;++finds;}
 }else if(!std::strcmp(name,"event_TranslateName"))*result=77;
 else if(!std::strcmp(name,"event_SetWork"))work=params[1];
 else if(!std::strcmp(name,"Door_Find"))*result=555;
 else if(!std::strcmp(name,"Bot_GetPlayer"))*result=99;
 else if(!std::strcmp(name,"FV3Points1_Acquire")||!std::strcmp(name,"FQuatTang1_Acquire"))*result=1000;
 else if(!std::strcmp(name,"f32_SetFromInt")){float x=(float)params[1];std::memcpy(result,&x,4);}
 else if(!std::strcmp(name,"Speech_Play"))++speeches;
 else if(!std::strcmp(name,"Door_GotoPos")){require(params[1]==555&&params[2]==1,"normal authored exit open");++opens;}
 else if(!std::strcmp(name,"Game_BeginCutScene"))++begins;
 else if(!std::strcmp(name,"Game_EndCutScene"))++ends;
 else if(!std::strcmp(name,"Checkpoint_Save"))++saves;
 else if(!std::strcmp(name,"FVec3Obj_IsDone"))*result=1;
 return AMX_ERR_NONE;
}
void exec(AMX& amx,const char* name,int event=0,int actor=0){
 int index;cell result;
 require(amx_FindPublic(&amx,const_cast<char*>(name),&index)==AMX_ERR_NONE,"public found");
 int status=event?amx_Exec(&amx,&result,index,5,0,event,0,actor,0):amx_Exec(&amx,&result,index,1,0);
 require(status==AMX_ERR_NONE,"retail bytecode executes");
}
int main(int argc,char** argv){
 require(argc==2,"retail path supplied");FILE* f=std::fopen(argv[1],"rb");require(f,"retail opened");
 AMX_HEADER hdr;require(std::fread(&hdr,sizeof(hdr),1,f)==1,"header read");
 auto* bytes=(unsigned char*)std::calloc(1,hdr.stp);std::rewind(f);
 require(std::fread(bytes,1,hdr.size,f)==size_t(hdr.size),"retail read");std::fclose(f);
 AMX amx{};require(amx_Init(&amx,bytes)==AMX_ERR_NONE,"production expansion and relocation");amx.callback=native;
 auto* before=(unsigned char*)std::malloc(hdr.stp);std::memcpy(before,bytes,hdr.stp);
 require(!PortPatchScriptGoals("other.sma",&amx),"other scripts excluded");
 require(PortPatchScriptGoals("xewrasycam1.sma",&amx),"kill comparison patched");
 int differences=0;for(int i=0;i<hdr.stp;i++)if(bytes[i]!=before[i]){
  ++differences;require(i==hdr.cod+0x4b4,"only kill goal operand changes");
 }
 require(differences==1,"exactly one byte changed; roster, data, spawn references untouched");
 require(PortPatchScriptGoals("xewrasycam1.sma",&amx),"repeat initialization is idempotent");
 exec(amx,"OnInit");require(finds==14,"all fourteen authored bots still registered");
 for(int kill=1;kill<=14;kill++){
  exec(amx,"OnEvent",77,kill);
  if(kill<10)require(work==0&&opens==0&&speeches==0,"door locked before tenth kill");
  if(kill==10){
   require(work==1&&speeches==1,"tenth kill starts normal exit sequence");
   for(int step=0;step<5&&work;step++)exec(amx,"Work");
   require(opens==1&&begins==1&&ends==1&&saves==1&&work==0,"normal scene opens door and checkpoints");
  }
  if(kill>10)require(opens==1&&speeches==1&&work==0,"remaining four bots do not repeat unlock");
 }
 auto* cmp=(cell*)(bytes+hdr.cod+0x4b0);cmp[1]=14;cmp[0]=0;
 require(!PortPatchScriptGoals("xewrasycam1.sma",&amx)&&cmp[1]==14,"unexpected revision left untouched");
 cmp[0]=105;auto* h=(AMX_HEADER*)bytes;h->dat=h->cod+0x4b0;
 require(!PortPatchScriptGoals("xewrasycam1.sma",&amx),"short program excluded");
 require(!PortPatchScriptGoals(nullptr,&amx)&&!PortPatchScriptGoals("xewrasycam1.sma",nullptr),"null inputs excluded");
 std::free(before);std::free(bytes);
 std::printf("PASS: %d retail asylum VM checks; all 14 bots, exit at 10 kills; game not launched.\n",checks);
}
'''


def main():
    OUT.mkdir(parents=True, exist_ok=True)
    (OUT / 'fclib.h').write_text(
        '#pragma once\n#include <string.h>\ntypedef bool BOOL;\n'
        '#define TRUE true\n#define FALSE false\n#define DEVPRINTF(...) ((void)0)\n'
        'inline int fclib_stricmp(const char* a,const char* b){return _stricmp(a,b);}\n')
    (OUT / 'goal.cpp').write_text(CODE)
    (OUT / 'CMakeLists.txt').write_text(
        'cmake_minimum_required(VERSION 3.20)\nproject(asylum_goal C CXX)\n'
        f'add_executable(goal goal.cpp "{(ROOT / "ma/Lib/SmallAMX/amx.c").as_posix()}")\n'
        f'target_include_directories(goal PRIVATE "{OUT.as_posix()}" "{(ROOT / "port").as_posix()}" '
        f'"{(ROOT / "ma/Lib/SmallAMX").as_posix()}")\n'
        'target_compile_features(goal PRIVATE cxx_std_14)\n'
        'target_compile_definitions(goal PRIVATE WIN32 _CRT_SECURE_NO_WARNINGS)\n')
    for cmd in (['cmake', '-S', str(OUT), '-B', str(OUT / 'out'), '-A', 'Win32'],
                ['cmake', '--build', str(OUT / 'out'), '--config', 'Release']):
        result = subprocess.run(cmd, capture_output=True, text=True)
        if result.returncode:
            raise SystemExit(result.stdout + result.stderr)
    subprocess.run([str(OUT / 'out/Release/goal.exe'),
                    str(ROOT / 'gamedata/mst/xewrasycam1.sma')], check=True)


if __name__ == '__main__':
    main()
