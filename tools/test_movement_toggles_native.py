"""Check persistent movement toggles against native campaign actors, with isolated saves."""
import argparse
import os
from pathlib import Path
import subprocess
import time
import uuid
import menu_drive

ROOT = Path(__file__).resolve().parent.parent
FINAL = 'COOP-TEST PASS: movement toggle test complete'


def run(exe, players, mission):
    run_id = f'{players}-{uuid.uuid4().hex[:8]}'
    folder = ROOT / 'build/logs/movement-toggle-native'
    log = folder / (run_id + '.log')
    save = ROOT / 'build/test-saves/movement-toggle-native' / run_id
    shots = ROOT / 'build/shots/movement-toggle-native' / run_id
    for directory in (folder, save, shots):
        directory.mkdir(parents=True, exist_ok=True)
    env = dict(os.environ, MA_PORT_TEST_COOP_POLISH='movement-cheats')
    for name in ('MA_PORT_COOP_HOLD', 'MA_PORT_TEST_KEYS', 'MA_PORT_TEST_GIVE'):
        env.pop(name, None)
    args = [str(exe), '-data', str(ROOT / 'gamedata/files'), '-mission', mission,
            '-res', '1280x960', '-save-dir', str(save), '-discord-app-id', 'off',
            '-instance-label', f'AUTOMATED movement check {players}P - leave untouched',
            '-log', str(log), '-port-diag', '-shots', str(shots), '-shot-every', '180']
    if players > 1:
        args += ['-coop', str(players)]
    process = subprocess.Popen(args, cwd=ROOT, env=env,
                               stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    print(f'OWN PID {process.pid}: {players}P; {log}', flush=True)
    focused = False
    try:
        deadline = time.monotonic() + 85
        while process.poll() is None and time.monotonic() < deadline:
            if not focused:
                window = menu_drive.find_window(process.pid)
                if window:
                    menu_drive.user32.ShowWindow(window, 9)
                    focused = bool(menu_drive.user32.SetForegroundWindow(window))
            text = log.read_text(errors='replace') if log.exists() else ''
            if FINAL in text or 'COOP-TEST FAIL' in text or 'FANG ASSERTION' in text:
                break
            time.sleep(.3)
    finally:
        if process.poll() is None:
            process.terminate()
        try:
            process.wait(timeout=5)
        except subprocess.TimeoutExpired:
            process.kill()
            process.wait(timeout=5)
    text = log.read_text(errors='replace') if log.exists() else ''
    for line in text.splitlines():
        if 'COOP-TEST' in line or 'MOVEMENT-TEST' in line or 'pause opened' in line:
            print(line, flush=True)
    if FINAL not in text or 'COOP-TEST FAIL' in text or 'FANG ASSERTION' in text:
        raise RuntimeError(f'Native movement check failed or timed out: {log}')
    print(f'PASS {players}P: {text.count("COOP-TEST PASS")} native checks; test closed.', flush=True)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--exe', type=Path, default=ROOT / 'build/Release/ma_port.exe')
    parser.add_argument('--players', default='1,4')
    parser.add_argument('--mission', default='WEDMmines01')
    args = parser.parse_args()
    counts = [int(value) for value in args.players.split(',')]
    if any(value not in (1, 2, 4) for value in counts):
        parser.error('--players accepts 1, 2, and 4.')
    for value in dict.fromkeys(counts):
        run(args.exe.resolve(), value, args.mission)


if __name__ == '__main__':
    main()
