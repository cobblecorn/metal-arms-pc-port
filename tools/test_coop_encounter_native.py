"""Native nearby-group encounter check, with isolated saves and an opt-in fixture.
Closes only the game process this test starts. Run after building normal Release.
"""
from pathlib import Path
import argparse
import os
import subprocess
import time

ROOT=Path(__file__).resolve().parent.parent

def main():
 parser=argparse.ArgumentParser(description=__doc__)
 parser.add_argument('--exe',type=Path,default=ROOT/'build/Release/ma_port.exe')
 parser.add_argument('--seconds',type=float,default=60)
 args=parser.parse_args()
 exe=args.exe.resolve()
 if not exe.is_file():parser.error('Build the game first or supply --exe.')
 if not 5<=args.seconds<=120:parser.error('--seconds must be between 5 and 120.')
 log=ROOT/'build/logs/encounter-group-native.log';log.parent.mkdir(parents=True,exist_ok=True)
 saves=ROOT/'build/test-saves/encounter-group-native';saves.mkdir(parents=True,exist_ok=True)
 # An earlier test result cannot satisfy a fresh run.
 if log.exists():log.unlink()
 env=dict(os.environ);env['MA_PORT_TEST_COOP_POLISH']='encounter-group';env.pop('MA_PORT_COOP_HOLD',None)
 process=subprocess.Popen([str(exe),'-data',str(ROOT/'gamedata/files'),'-mission','WEMCcity_05',
     '-coop','2','-save-dir',str(saves),'-instance-label','Encounter test','-discord-app-id','off',
     '-res','800x600','-log',str(log)],cwd=exe.parent,env=env,
     stdout=subprocess.DEVNULL,stderr=subprocess.DEVNULL)
 final='COOP-TEST PASS: approaching P2 remains outside original trigger'
 text='';deadline=time.monotonic()+args.seconds
 try:
  while process.poll() is None and time.monotonic()<deadline:
   text=log.read_text(errors='replace') if log.exists() else ''
   if final in text or 'COOP-TEST FAIL:' in text:break
   time.sleep(.5)
 finally:
  if process.poll() is None:process.terminate()
  process.wait(timeout=10)
 text=log.read_text(errors='replace') if log.exists() else ''
 for line in text.splitlines():
  if 'COOP-TEST' in line or 'nearby group' in line:print(line)
 if final not in text or 'COOP-TEST FAIL:' in text or '*** FANG ASSERTION' in text:
  raise SystemExit('Native encounter check failed or timed out; inspect '+str(log))
 print('PASS: native encounter group; isolated test process closed.')

if __name__=='__main__':main()
