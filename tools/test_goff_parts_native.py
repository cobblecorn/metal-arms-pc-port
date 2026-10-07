"""Collect the three authored Sniper's Lair quest props in an isolated native run.
Partners need their normal connected controller slots; 3/4-player runs require enough pads.
The opt-in fixture uses real world pickups and the retail script. After the third
part, this helper skips only its own ending movie through the native status handler,
then checks that the mission-complete screen loads. It closes only its own process.
"""
from pathlib import Path
import subprocess,time,os,sys,argparse
import ctypes as c
from ctypes import wintypes as w

class EndingMovieSkip:
 def __init__(self,pid,exe):
  self.k=c.WinDLL('kernel32',use_last_error=True);self.d=c.WinDLL('dbghelp',use_last_error=True);ps=c.WinDLL('psapi')
  self.k.OpenProcess.argtypes=[w.DWORD,w.BOOL,w.DWORD];self.k.OpenProcess.restype=w.HANDLE
  self.h=self.k.OpenProcess(0x438,False,pid);assert self.h
  ps.EnumProcessModulesEx.argtypes=[w.HANDLE,c.POINTER(w.HMODULE),w.DWORD,c.POINTER(w.DWORD),w.DWORD]
  modules=(w.HMODULE*1024)();count=w.DWORD();assert ps.EnumProcessModulesEx(self.h,modules,c.sizeof(modules),c.byref(count),3)
  self.d.SymInitializeW.argtypes=[w.HANDLE,w.LPCWSTR,w.BOOL];assert self.d.SymInitializeW(self.h,None,False)
  self.d.SymLoadModuleExW.argtypes=[w.HANDLE,w.HANDLE,w.LPCWSTR,w.LPCWSTR,c.c_ulonglong,w.DWORD,c.c_void_p,w.DWORD];self.d.SymLoadModuleExW.restype=c.c_ulonglong
  assert self.d.SymLoadModuleExW(self.h,None,str(exe),None,modules[0],0,None,0)
  class SI(c.Structure):
   _fields_=[('SizeOfStruct',w.ULONG),('TypeIndex',w.ULONG),('Reserved',c.c_ulonglong*2),('Index',w.ULONG),('Size',w.ULONG),('ModBase',c.c_ulonglong),('Flags',w.ULONG),('Value',c.c_ulonglong),('Address',c.c_ulonglong),('Register',w.ULONG),('Scope',w.ULONG),('Tag',w.ULONG),('NameLen',w.ULONG),('MaxNameLen',w.ULONG),('Name',c.c_char*1024)]
  self.d.SymFromName.argtypes=[w.HANDLE,c.c_char_p,c.POINTER(SI)]
  symbol=SI();symbol.SizeOfStruct=88;symbol.MaxNameLen=1024;assert self.d.SymFromName(self.h,b'_hBink',c.byref(symbol));self.bink=symbol.Address
  symbol=SI();symbol.SizeOfStruct=88;symbol.MaxNameLen=1024;assert self.d.SymFromName(self.h,b'_bMoviePaused',c.byref(symbol));self.pause=symbol.Address;self.skipped=False
  self.k.ReadProcessMemory.argtypes=[w.HANDLE,c.c_void_p,c.c_void_p,c.c_size_t,c.POINTER(c.c_size_t)]
  self.k.WriteProcessMemory.argtypes=[w.HANDLE,c.c_void_p,c.c_void_p,c.c_size_t,c.POINTER(c.c_size_t)]
 def skip(self):
  # Pause this test movie once so the normal cutscene status handler ends it.
  if not self.skipped:
   ptr=c.c_uint32();got=c.c_size_t()
   if self.k.ReadProcessMemory(self.h,self.bink,c.byref(ptr),4,c.byref(got)) and ptr.value:
    yes=c.c_uint32(1);assert self.k.WriteProcessMemory(self.h,self.pause,c.byref(yes),4,c.byref(got));self.skipped=True
 def close(self):
  self.d.SymCleanup.argtypes=[w.HANDLE];self.d.SymCleanup(self.h);self.k.CloseHandle.argtypes=[w.HANDLE];self.k.CloseHandle(self.h)

root=Path(__file__).resolve().parent.parent
parser=argparse.ArgumentParser(description=__doc__)
parser.add_argument('--players',type=int,choices=(1,2,3,4),default=2)
parser.add_argument('--exe',type=Path,default=root/'build/Release/ma_port.exe')
args=parser.parse_args();n=args.players
log=root/f'build/logs/goff-native-{n}.log';save=root/f'build/test-saves/goff-native-{n}';save.mkdir(exist_ok=True)
if log.exists():log.unlink()
env=dict(os.environ);env['MA_PORT_TEST_COOP_POLISH']='goff-parts'
exe=args.exe.resolve();assert exe.is_file()
args=[str(exe),'-data',str(root/'gamedata/files'),'-mission','WECRruins02','-save-dir',str(save),'-discord-app-id','off','-res','800x600','-instance-label',f'Goff pickup test {n}','-log',str(log)]
if n>1:args+=['-coop',str(n)]
out=root/f'build/shots/goff-native-{n}';out.mkdir(exist_ok=True);args+=['-shots',str(out),'-shot-every','180']
p=subprocess.Popen(args,cwd=exe.parent,env=env,stdout=subprocess.DEVNULL,stderr=subprocess.DEVNULL);print('OWN PID',p.pid,flush=True)
menu=None
try:
 deadline=time.monotonic()+110
 while p.poll() is None and time.monotonic()<deadline:
  text=log.read_text(errors='replace') if log.exists() else ''
  if 'END OF LEVEL COMPLETE SCREEN' in text or 'COOP-TEST FAIL' in text or '*** FANG ASSERTION' in text:break
  if "got 3 goff parts" in text:
   if menu is None:menu=EndingMovieSkip(p.pid,exe)
   menu.skip()
  time.sleep(.02 if menu else .3)
finally:
 if menu:menu.close()
 if p.poll() is None:p.terminate()
 p.wait(timeout=10)
text=log.read_text(errors='replace')
for l in text.splitlines():
 if any(x in l for x in ['GOFF-TEST','COOP-TEST','Agent Goff part','got a goff','got 3 goff','LOAD MARKER']):print(l)
assert text.count('goffpickup notified.')==3,text.count('goffpickup notified.')
assert 'got 3 goff parts' in text
assert 'END OF LEVEL COMPLETE SCREEN' in text
assert 'COOP-TEST FAIL' not in text and '*** FANG ASSERTION' not in text
print('PASS',n,'players collected real authored parts and reached the mission-complete screen')
