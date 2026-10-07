"""Exercise Coliseum 3's authored battle callback and gate motion with isolated saves."""
from pathlib import Path
import argparse
import os
import subprocess
import time
import uuid

ROOT = Path(__file__).resolve().parent.parent
FINAL = 'COOP-TEST PASS: Coliseum gate test complete'


def run(exe, players):
    run_id = f'{players}-{uuid.uuid4().hex[:8]}'
    log = ROOT / 'build/logs/coliseum-gates-native' / (run_id + '.log')
    save = ROOT / 'build/test-saves/coliseum-gates-native' / run_id
    log.parent.mkdir(parents=True, exist_ok=True)
    save.mkdir(parents=True, exist_ok=True)
    env = dict(os.environ, MA_PORT_TEST_COOP_POLISH='coliseum-gates')
    env.pop('MA_PORT_COOP_HOLD', None)
    command = [str(exe), '-data', str(ROOT / 'gamedata/files'),
               '-mission', 'WEBCcolis03', '-res', '640x480',
               '-save-dir', str(save), '-discord-app-id', 'off',
               '-instance-label', f'Coliseum gate test {players}',
               '-log', str(log), '-port-diag']
    if players > 1:
        command += ['-coop', str(players)]
    process = subprocess.Popen(command, cwd=exe.parent, env=env,
                               stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    print(f'OWN PID {process.pid}: {players} player(s); log {log}', flush=True)
    deadline = time.monotonic() + 40
    try:
        while process.poll() is None and time.monotonic() < deadline:
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
        if 'COOP-TEST' in line or 'COLISEUM-GATE-TEST' in line or 'Script String: Door Open' in line:
            print(line, flush=True)
    if FINAL not in text or 'COOP-TEST FAIL' in text or 'FANG ASSERTION' in text:
        raise RuntimeError(f'Gate fixture failed or timed out; inspect {log}')
    if 'xebcbttle03.sma 504 Script String: Door Open' not in text:
        raise RuntimeError(f'Retail battle script did not pass its open-gate check; inspect {log}')
    print(f'PASS: real battle passes delayed gate check, {players} player(s).', flush=True)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--players', default='1,2',
                        help='Counts 1,2,4; four players require a ready controller/join setup.')
    parser.add_argument('--exe', type=Path, default=ROOT / 'build/encounter-candidate/Release/ma_port.exe')
    args = parser.parse_args()
    counts = [int(count) for count in args.players.split(',')]
    if any(count not in (1, 2, 4) for count in counts):
        parser.error('--players accepts 1, 2, and 4.')
    for count in dict.fromkeys(counts):
        run(args.exe.resolve(), count)


if __name__ == '__main__':
    main()
