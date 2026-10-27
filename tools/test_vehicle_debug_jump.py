"""Compile the actual tank/RAT/Loader developer jump branches in PC/legacy modes."""
from pathlib import Path
import subprocess

ROOT=Path(__file__).resolve().parents[1]
OUT=ROOT/'build/test-vehicle-debug-jump'

def branch(text,marker):
    at=text.index(marker)
    start=text.rfind('\n#if ',0,at)+1
    # The RAT contains a nested, disabled sink-test branch.
    if '#if 0' in text[start:at]:start=text.rfind('\n#if ',0,start-1)+1
    depth=0;result=''
    for line in text[start:].splitlines(keepends=True):
        result+=line;s=line.strip()
        if s.startswith(('#if ','#ifdef ','#ifndef ')):depth+=1
        elif s.startswith('#endif'):
            depth-=1
            if not depth:return result
    raise AssertionError('Unterminated developer branch')

def main():
    tank=(ROOT/'ma/App/ma/vehiclesentinel.cpp').read_text()
    rat=(ROOT/'ma/App/ma/vehiclerat.cpp').read_text()
    loader=(ROOT/'ma/App/ma/vehicleloader.cpp').read_text()
    bodies=[branch(tank,'// allow driver to "jump" the vehicle'),branch(rat,'// allow driver to "jump" the vehicle'),branch(loader,'// Developer-only Loader hop')]
    fixture='''#include <cstdio>
using f32=float;using BOOL=bool;
const float _JUMP_FORCE=500000;
#define SAS_ACTIVE_USER 1
#define SAS_USER_ELLIOTT 1
struct CFVec3A {float y=0;void Set(float,float b,float){y=b;}};
struct Phys {int calls=0;CFVec3A velocity;CFVec3A *GetVelocity(){return &velocity;}void ApplyForce(CFVec3A *v,CFVec3A *){if(v->y)++calls;}};
'''
    for i,body in enumerate(bodies):
        fixture+=f'int probe{i}(){{Phys m_PhysObj;CFVec3A vPoint,m_vCMOffset,m_Velocity_WS;float m_bControls_Jump=1;\n'
        if i<2:fixture+='CFVec3A vForce;\n'
        if i<2:fixture+='if(false){}\n'
        fixture+=body+'\nreturn m_PhysObj.calls;}\n'
    fixture+='''int main(){
 int a=probe0(),b=probe1(),c=probe2();
 int expected=FANG_WINGC?0:1;
 if(a!=expected||b!=expected||c!=expected)return 1;
 std::printf("PASS: %s developer hops tank=%d RAT=%d Loader=%d\\n",FANG_WINGC?"PC":"legacy debug",a,b,c);
}'''
    OUT.mkdir(parents=True,exist_ok=True);(OUT/'test.cpp').write_text(fixture)
    (OUT/'CMakeLists.txt').write_text('cmake_minimum_required(VERSION 3.20)\nproject(vehicle_debug_jump LANGUAGES CXX)\n'+''.join(f'add_executable({name} test.cpp)\ntarget_compile_definitions({name} PRIVATE FANG_WINGC={pc} FANG_PRODUCTION_BUILD=0)\n' for name,pc in [('pc',1),('legacy',0)]))
    for cmd in (['cmake','-S',str(OUT),'-B',str(OUT/'out'),'-A','Win32'],['cmake','--build',str(OUT/'out'),'--config','Release']):
        run=subprocess.run(cmd,capture_output=True,text=True);assert run.returncode==0,run.stdout+run.stderr
    for name in ('pc','legacy'):
        run=subprocess.run([str(OUT/'out/Release'/(name+'.exe'))],capture_output=True,text=True)
        assert run.returncode==0,run.stdout+run.stderr
        print(run.stdout.strip())

if __name__=='__main__':main()
