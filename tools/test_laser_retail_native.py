"""Fire real charged tracers into an isolated native collision/damage target."""
from pathlib import Path
import argparse
import os
import subprocess
import time

ROOT = Path(__file__).resolve().parent.parent


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--players', type=int, choices=(1, 2), default=1)
    parser.add_argument('--owner', type=int, choices=(1, 2), default=1)
    args = parser.parse_args()
    assert args.owner <= args.players
    name = f'laser-retail-{args.players}p-owner{args.owner}'
    save = ROOT/'build/test-saves'/name
    save.mkdir(parents=True, exist_ok=True)
    log = ROOT/'build/logs'/(name+'.log')
    if log.exists():
        log.unlink()
    env = dict(os.environ, MA_PORT_TEST_COOP_POLISH='report-laser-retail'+str(args.owner))
    env.pop('MA_PORT_TEST_KEYS', None)
    cmd = [str(ROOT/'build/Release/ma_port.exe'), '-data', str(ROOT/'gamedata/files'),
           '-mission' if args.players > 1 else '-level', 'WEDMmines02', '-save-dir', str(save), '-windowed', '-res', '960x720',
           '-mute', '-discord-app-id', 'off', '-log', str(log), '-asset-log', str(log.with_suffix('.assets.log'))]
    if args.players > 1:
        cmd += ['-coop', str(args.players)]
    startup = subprocess.STARTUPINFO()
    startup.dwFlags |= subprocess.STARTF_USESHOWWINDOW
    startup.wShowWindow = 0
    proc = subprocess.Popen(cmd, cwd=ROOT, env=env, startupinfo=startup,
                            stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    print('Native retail laser probe PID', proc.pid, flush=True)
    deadline = time.monotonic() + 55
    seen = 0
    try:
        while proc.poll() is None and time.monotonic() < deadline:
            text = log.read_text(encoding='latin-1') if log.exists() else ''
            lines = [line for line in text.splitlines() if line.startswith(('LASER-RETAIL', 'COOP-TEST FAIL'))]
            for line in lines[seen:]:
                print(line, flush=True)
            seen = len(lines)
            if 'LASER-RETAIL COMPLETE' in text or 'COOP-TEST FAIL' in text or '*** FANG' in text:
                break
            time.sleep(.25)
    finally:
        if proc.poll() is None:
            proc.terminate()
        proc.wait(timeout=10)
    text = log.read_text(encoding='latin-1') if log.exists() else ''
    assert 'LASER-RETAIL COMPLETE' in text and 'COOP-TEST FAIL' not in text and '*** FANG' not in text, str(log)
    assert f'LASER-RETAIL COMPLETE owner=P{args.owner}' in text
    print('PASS:', name, flush=True)


if __name__ == '__main__':
    main()
