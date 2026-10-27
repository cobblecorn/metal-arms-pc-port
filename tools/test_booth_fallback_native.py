"""Run the authored booth scene normally or with its four guards held stationary."""
from pathlib import Path
import argparse
import os
import subprocess
import time

ROOT = Path(__file__).resolve().parent.parent


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--players',type=int,choices=(1,2),default=1)
    p.add_argument('--blocked',action='store_true')
    a = p.parse_args()
    name = f'booth-fallback-{a.players}p-'+('blocked' if a.blocked else 'natural')
    save = ROOT/'build/test-saves'/name
    save.mkdir(parents=True,exist_ok=True)
    log = ROOT/'build/logs'/(name+'.log')
    log.parent.mkdir(exist_ok=True)
    if log.exists():
        log.unlink()
    env = {k:v for k,v in os.environ.items() if not k.startswith('MA_PORT_TEST_')}
    env['MA_PORT_TEST_COOP_POLISH'] = 'booth-blocked' if a.blocked else 'booth-natural'
    cmd = [str(ROOT/'build/Release/ma_port.exe'),'-data',str(ROOT/'gamedata/files'),
           '-mission','WEWRresrch2','-save-dir',str(save),'-windowed','-res','960x720',
           '-mute','-discord-app-id','off','-log',str(log),'-asset-log',str(log.with_suffix('.assets.log'))]
    if a.players>1:
        cmd += ['-coop',str(a.players)]
    startup = subprocess.STARTUPINFO()
    startup.dwFlags |= subprocess.STARTF_USESHOWWINDOW
    startup.wShowWindow = 0
    proc = subprocess.Popen(cmd,cwd=ROOT,env=env,startupinfo=startup,
                            stdout=subprocess.DEVNULL,stderr=subprocess.DEVNULL)
    print('Native booth probe PID',proc.pid,flush=True)
    seen = 0
    try:
        deadline = time.monotonic()+80
        while proc.poll() is None and time.monotonic()<deadline:
            text = log.read_text(encoding='latin1') if log.exists() else ''
            lines = [s for s in text.splitlines() if s.startswith(('BOOTH-TEST','Port: booth arrival fallback','COOP-TEST FAIL'))]
            for line in lines[seen:]:
                print(line,flush=True)
            seen = len(lines)
            if 'BOOTH-TEST COMPLETE' in text or 'COOP-TEST FAIL' in text or '*** FANG' in text:
                break
            time.sleep(.25)
    finally:
        if proc.poll() is None:
            proc.terminate()
        proc.wait(timeout=10)
    text = log.read_text(encoding='latin1') if log.exists() else ''
    assert 'BOOTH-TEST COMPLETE' in text and 'COOP-TEST FAIL' not in text and '*** FANG' not in text,str(log)
    assert ('Port: booth arrival fallback' in text)==a.blocked,str(log)
    print('PASS:',name,flush=True)


if __name__ == '__main__':
    main()
