"""Exercise the production endian converter on retail animations and bad aliases."""
from pathlib import Path
import struct
import subprocess

from mst_list import read_mst
from test_coop_checkpoint_rat import method

ROOT = Path(__file__).resolve().parents[1]
OUT = ROOT / 'build/test-gc-animation'


def expected(data):
    """Independent byte-level oracle: swap every referenced scalar exactly once."""
    flags, bones = struct.unpack_from('>HH', data, 16)
    bone_start = struct.unpack_from('>I', data, 28)[0]
    scalars = {16: 2, 18: 2, 20: 4, 24: 4, 28: 4}
    time_width = 1 if flags & 16 else 2 if flags & 32 else 4
    strides = [4, 6 if flags & 1 else 12, 8 if flags & 2 else 16]
    widths = [4, 2 if flags & 1 else 4, 2 if flags & 2 else 4]
    for bone in range(bones):
        base = bone_start + bone * 36
        scalars.update({base: 4, base + 4: 2, base + 6: 2, base + 8: 2})
        scalars.update({base + offset: 4 for offset in range(12, 36, 4)})
        counts = struct.unpack_from('>3H', data, base + 4)
        offsets = struct.unpack_from('>6I', data, base + 12)
        for track in range(3):
            for start, length, width in (
                (offsets[track], counts[track] * time_width, time_width),
                (offsets[track + 3], counts[track] * strides[track], widths[track]),
            ):
                for pos in range(start, start + length, width):
                    assert scalars.get(pos, width) == width
                    scalars[pos] = width
    converted = bytearray(data)
    for pos, width in scalars.items():
        converted[pos:pos + width] = data[pos:pos + width][::-1]
    return converted


def synthetic(time_width):
    # Nested time aliases and chained compressed data aliases across two bones.
    data = bytearray(256)
    flags = 3 | {1: 16, 2: 32, 4: 0}[time_width]
    struct.pack_into('>HHffI', data, 16, flags, 2, 2., .5, 32)
    data[104:109] = b'bone\0'
    for bone, count in ((0, 4), (1, 2)):
        base = 32 + bone * 36
        struct.pack_into('>I3H2x6I', data, base, 104, count, count, count,
                         112, 112 + bone * time_width, 112,
                         144, 176 + bone * 2, 190 + bone * 2)
    for pos in range(112, 256):
        data[pos] = (pos * 13 + 7) & 255
    return data


def main():
    OUT.mkdir(parents=True, exist_ok=True)
    cases = []
    archive, _, entries = read_mst(ROOT / 'gamedata/files/mettlearms_gc.mst')
    for name, flags, start, size, *_ in entries:
        if name.endswith('.mtx'):
            assert not flags, (name, flags)
            archive.seek(start)
            data = archive.read(size)
            # Keep the existing engine bone limit; this one retail resource has 148.
            supported = struct.unpack_from('>H', data, 18)[0] <= 127
            cases.append((name, supported, data, expected(data) if supported else data))
    archive.close()
    for width in (1, 2, 4):
        data = synthetic(width)
        cases.append((f'aliases-{width}', True, data, expected(data)))
        bad = bytearray(data)
        # 32-bit scales overlap a compressed 16-bit translation array.
        struct.pack_into('>I', bad, 32 + 24, 176)
        cases.append((f'incompatible-{width}', False, bad, bad))
        bad = bytearray(data)
        struct.pack_into('>I', bad, 32 + 28, 250)  # Out of bounds.
        cases.append((f'truncated-{width}', False, bad, bad))
    with (OUT / 'cases.bin').open('wb') as stream:
        for name, valid, data, output in cases:
            label = name.encode('ascii')
            stream.write(struct.pack('<III', len(label), len(data), valid))
            stream.write(label + data + output)

    header = (ROOT / 'ma/Lib/Fang2/fanim.h').read_text()
    bone = header[header.index('\t\tchar *pszName;'):header.index('} FAnimBone_t;')]
    bone = bone[:bone.index('\t\tFINLINE void SetUnitKeySlider')] + method(bone, '\t\tvoid ChangeEndian')
    anim = method(header, 'struct FAnim_t') + ';'
    source = r'''
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstddef>
#include <vector>
#include <cstdint>
using u8=unsigned char;using u16=unsigned short;using u32=unsigned;using f32=float;using BOOL=int;using cchar=const char;
using s32=int;
constexpr BOOL TRUE=1,FALSE=0;
constexpr unsigned FDATA_MAX_BONE_COUNT=127,FANIM_ANIMNAME_LEN=15;
constexpr unsigned FANIM_BONEFLAGS_COMP_TRANSLATION=1,FANIM_BONEFLAGS_COMP_ORIENTATION=2,
 FANIM_BONEFLAGS_8BIT_SECS=16,FANIM_BONEFLAGS_16BIT_SECS=32,FANIM_BONEFLAGS_8BIT_FRAMECOUNT=64;
#define DEVPRINTF(...) std::printf(__VA_ARGS__)
#define FINLINE inline
#define FASSERT_NOW do { std::puts("FAIL: slider asserted"); std::exit(1); } while(0)
float fmath_Div(float a,float b){if(b==0){std::puts("FAIL: zero divisor");std::exit(1);}return a/b;}
template<class T>T fang_ConvertEndian(T value){u8 bytes[sizeof(T)];std::memcpy(bytes,&value,sizeof(T));
 for(unsigned i=0;i<sizeof(T)/2;i++){u8 tmp=bytes[i];bytes[i]=bytes[sizeof(T)-i-1];bytes[sizeof(T)-i-1]=tmp;}
 std::memcpy(&value,bytes,sizeof(T));return value;}
'''
    source += 'struct FAnimBone_t {\n' + bone + '\n};\n' + anim
    source += '\nstatic_assert(sizeof(FAnim_t)==32 && sizeof(FAnimBone_t)==36,"retail x86 layout");\n'
    production = (ROOT / 'port/gcdata.cpp').read_text()
    for signature in ('static u16 _ReadBE16', 'static u32 _ReadBE32', 'static BOOL _IsRangeValid',
                      'static BOOL _IsArrayRangeValid', 'static void _ConvertBE16Array',
                      'static void _ConvertBE32Array', 'static BOOL _ConvertAnimation'):
        source += method(production, signature) + '\n'
    sliders = (ROOT / 'ma/Lib/Fang2/fanim.cpp').read_text()
    ratios = (ROOT / 'ma/Lib/Fang2/dx/fdx8anim.inl').read_text()
    for kind in ('u8', 'u16', 'f32'):
        source += method(sliders, f'static u16 _SetUnitKeySlider( {kind} *') + '\n'
    for kind in ('8bit', '16bit', '32bit'):
        source += method(ratios, f'FINLINE f32 fanim_GenerateRatio_{kind}') + '\n'
    source += r'''
template<class T> void testSliders(){
 T held[]={T(128),T(128)};
 for(float time : {-10.f,0.f,64.f,127.f,128.f,140.f})for(float delta : {-1.f,1.f})
  if(_SetUnitKeySlider(held,2,0,time,delta)!=0)std::exit(4);
 T times[]={T(0),T(32),T(64),T(96),T(128)};
 for(int old=0;old<4;++old)for(int time=0;time<=128;++time)for(float delta : {-1.f,1.f}){
  unsigned low=(time>=times[old] && time<times[old+1]) ? old :
   _SetUnitKeySlider(times,5,old,float(time),delta);
  if(low>=4 || time<times[low] || time>times[low+1])std::exit(5);
 }
}
void testInterpolation(){testSliders<u8>();testSliders<u16>();testSliders<f32>();
 for(float time : {-10.f,0.f,64.f,127.f,128.f,140.f}){
  if(fanim_GenerateRatio_8bit(time,128,128)!=0 || fanim_GenerateRatio_16bit(time,128,128)!=0 ||
     fanim_GenerateRatio_32bit(time,128,128)!=0)std::exit(6);
 }
 for(int t=-8;t<=40;++t){float value=t<0?0:t>32?1:float(t)/32;
  if(fanim_GenerateRatio_8bit(float(t),0,32)!=value || fanim_GenerateRatio_16bit(float(t),0,32)!=value ||
     fanim_GenerateRatio_32bit(float(t),0,32)!=value)std::exit(7);
 }std::puts("PASS: production key sliders and interpolation preserve normal tracks and hold duplicate/endpoints.");
}
'''
    source += r'''
int main(int argc,char**argv){testInterpolation();FILE*f=std::fopen(argv[1],"rb");if(!f)return 2;unsigned checks=0,h[3];
 while(std::fread(h,sizeof(h),1,f)==1){std::vector<char>name(h[0]+1);std::vector<u8>data(h[1]),expected(h[1]);
  if(std::fread(name.data(),1,h[0],f)!=h[0] || std::fread(data.data(),1,h[1],f)!=h[1] ||
     std::fread(expected.data(),1,h[1],f)!=h[1])return 3;
  BOOL valid=_ConvertAnimation(data.data(),h[1],name.data());
  if(valid!=h[2] || data!=expected){std::printf("FAIL: %s (valid=%d expected=%u bytesMatch=%d)\n",name.data(),valid,h[2],data==expected);return 1;}
  ++checks;
 }std::fclose(f);std::printf("PASS: %u production animation conversions, shared scalars converted once, invalid inputs unchanged.\n",checks);}
'''
    (OUT / 'animation.cpp').write_text(source)
    (OUT / 'CMakeLists.txt').write_text('cmake_minimum_required(VERSION 3.20)\nproject(animation LANGUAGES CXX)\nadd_executable(animation animation.cpp)\ntarget_compile_features(animation PRIVATE cxx_std_17)\n')
    for command in (['cmake', '-S', str(OUT), '-B', str(OUT / 'out'), '-A', 'Win32'],
                    ['cmake', '--build', str(OUT / 'out'), '--config', 'Release']):
        run = subprocess.run(command, capture_output=True, text=True)
        if run.returncode:
            raise SystemExit(run.stdout + run.stderr)
    subprocess.run([str(OUT / 'out/Release/animation.exe'), str(OUT / 'cases.bin')], check=True)


if __name__ == '__main__':
    main()
