"""Check the production Mil City contact filter using the captured native triangle."""
from pathlib import Path
import subprocess

ROOT=Path(__file__).resolve().parents[1]
OUT=ROOT/'build/test-city-slope'
CPP=r'''
#include <cstdio>
#include <cstdlib>
struct Vec {float x,y,z;};
struct FCollImpact_t {Vec aTriVtx[3],UnitFaceNormal;void *pTag;};
struct CFSphere {Vec m_Pos;float m_fRadius;};
#include "pc_city_slope.h"
int checks=0;
void check(bool ok,const char *why){++checks;if(!ok){std::printf("FAIL: %s\n",why);std::exit(1);}}
int main(){
 FCollImpact_t real={{{439.125f,4.78125f,-846.4375f},{439.09375f,24.28125f,-793.25f},{439.03125f,4.78125f,-793.28125f}},{.99994f,-.00232f,.00153f},nullptr};
 CFSphere outside={{441.0051f,15.102f,-817.3f},1.9f};
 check(PortMilCityBuriedDoorwayWall(real,outside),"captured trapping panel is excluded outside the visible slope");
 CFSphere inside={{445,15.102f,-817.3f},1.9f};
 check(!PortMilCityBuriedDoorwayWall(real,inside),"same panel still blocks the interior");
 CFSphere below={{441,3,-817.3f},1.9f};
 check(!PortMilCityBuriedDoorwayWall(real,below),"lower interior wall is retained");
 auto hit=real;for(auto &v:hit.aTriVtx)v.z+=169.1875f;
 auto second=outside;second.m_Pos.z+=169.1875f;
 check(PortMilCityBuriedDoorwayWall(hit,second),"second identical doorway panel is recognized");
 hit=real;hit.pTag=&hit;check(!PortMilCityBuriedDoorwayWall(hit,outside),"dynamic entities are untouched");
 hit=real;hit.UnitFaceNormal.x=-1;check(!PortMilCityBuriedDoorwayWall(hit,outside),"opposite-facing wall is untouched");
 hit=real;hit.UnitFaceNormal.y=.3f;check(!PortMilCityBuriedDoorwayWall(hit,outside),"other slopes are untouched");
 hit=real;hit.UnitFaceNormal.z=.3f;check(!PortMilCityBuriedDoorwayWall(hit,outside),"other wall orientations are untouched");
 for(int i=0;i<3;++i){hit=real;hit.aTriVtx[i].x+=1;check(!PortMilCityBuriedDoorwayWall(hit,outside),"every vertex must match the known panel");}
 hit=real;hit.aTriVtx[0].y=30;check(!PortMilCityBuriedDoorwayWall(hit,outside),"other wall heights are untouched");
 hit=real;hit.aTriVtx[0].z=-820;check(!PortMilCityBuriedDoorwayWall(hit,outside),"unrelated triangles within the vicinity are untouched");
 hit=real;for(auto &v:hit.aTriVtx)v.z+=20;check(!PortMilCityBuriedDoorwayWall(hit,outside),"nearby panels are untouched");
 auto large=outside;large.m_fRadius=10;check(!PortMilCityBuriedDoorwayWall(real,large),"a sphere extending inside retains the wall");
 std::printf("PASS: %d production contact-filter checks\n",checks);
}
'''

def main():
    OUT.mkdir(parents=True,exist_ok=True)
    (OUT/'test.cpp').write_text(CPP)
    (OUT/'CMakeLists.txt').write_text('cmake_minimum_required(VERSION 3.20)\nproject(city_slope LANGUAGES CXX)\nadd_executable(city_slope test.cpp)\ntarget_include_directories(city_slope PRIVATE "'+str(ROOT/'port').replace('\\','/')+'")\n')
    for cmd in (['cmake','-S',str(OUT),'-B',str(OUT/'out'),'-A','Win32'],['cmake','--build',str(OUT/'out'),'--config','Release']):
        run=subprocess.run(cmd,capture_output=True,text=True)
        assert run.returncode==0,run.stdout+run.stderr
    run=subprocess.run([str(OUT/'out/Release/city_slope.exe')],capture_output=True,text=True)
    assert run.returncode==0,run.stdout+run.stderr
    print(run.stdout.strip())

if __name__=='__main__':main()
