"""One-run factory combat resume requested during playtesting; normal boots are unchanged."""
from pathlib import Path
import argparse
import os
import subprocess
ROOT=Path(__file__).resolve().parent.parent

def main():
 parser=argparse.ArgumentParser(description=__doc__)
 parser.add_argument('--exe',type=Path,default=ROOT/'build/Release/ma_port.exe')
 parser.add_argument('--coop',type=int,choices=(2,3,4),default=2)
 args=parser.parse_args()
 exe=args.exe.resolve()
 data=exe.parent/'gamedata/files'
 if not data.is_dir():data=ROOT/'gamedata/files'
 if not exe.is_file() or not data.is_dir():parser.error('Game EXE or retail gamedata/files is missing.')
 environment=dict(os.environ)
 environment['MA_PORT_TEST_COOP_POLISH']='spy-resume'
 process=subprocess.Popen([str(exe),'-data',str(data),'-mission','WECFfacty01','-coop',str(args.coop),'-port-diag','-log',str(exe.parent/'spy-combat-resume.log')],cwd=exe.parent,env=environment,stdout=subprocess.DEVNULL,stderr=subprocess.DEVNULL)
 print('Started factory combat resume, PID',process.pid)
 print('This launch skips completed instruction only; ordinary launches are unchanged.')
if __name__=='__main__':main()
