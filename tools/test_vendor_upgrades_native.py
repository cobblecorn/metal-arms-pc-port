"""Select retail EUKs and purchase upgrades through the running game's shop input.

The opt-in fixture loads real retail catalog items into the active offer slot.
Normal shop rendering, purchase input, spawned pickups and inventory/wallet code
then run. Each run has isolated saves and only its own process is closed.
"""
from pathlib import Path
import argparse
import os
import subprocess
import time

ROOT = Path(__file__).resolve().parent.parent


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--players", type=int, choices=(1, 2), default=1)
    parser.add_argument("--owner", type=int, choices=(1, 2), default=1)
    parser.add_argument("--ammo", action="store_true", help="Buy ammo for upgraded Rivet Gun and SPEW")
    args = parser.parse_args()
    assert args.owner <= args.players
    kind = "ammo" if args.ammo else "upgrades"
    marker = "VENDOR-AMMO" if args.ammo else "VENDOR-UPGRADE"
    name = f"vendor-{kind}-{args.players}p-owner{args.owner}-20261018"
    log = ROOT / "build/logs" / (name + ".log")
    if log.exists():
        log.unlink()
    save = ROOT / "build/test-saves" / name
    shots = ROOT / "build/shots" / name
    save.mkdir(parents=True, exist_ok=True)
    shots.mkdir(parents=True, exist_ok=True)
    exe = ROOT / "build/Release/ma_port.exe"
    env = dict(os.environ, MA_PORT_TEST_COOP_POLISH=f"shop-{kind}{args.owner}")
    env.pop("MA_PORT_COOP_HOLD", None)
    command = [str(exe), "-data", str(ROOT / "gamedata/files"), "-mission", "WEDMmines03",
               "-save-dir", str(save), "-discord-app-id", "off", "-res", "960x720",
               "-windowed", "-mute", "-instance-label", name, "-log", str(log),
               "-shots", str(shots), "-shot-every", "60"]
    if args.players > 1:
        command += ["-coop", str(args.players)]
    process = subprocess.Popen(command, cwd=ROOT, env=env,
                               stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    print(f"Native {args.players}p, shopper P{args.owner}; own PID {process.pid}", flush=True)
    seen = 0
    deadline = time.monotonic() + 130
    text = ""
    try:
        while process.poll() is None and time.monotonic() < deadline:
            text = log.read_text(encoding="latin-1") if log.exists() else ""
            lines = [line for line in text.splitlines() if line.startswith(marker)
                     or line.startswith("COOP-TEST FAIL")]
            for line in lines[seen:]:
                print(line, flush=True)
            seen = len(lines)
            if marker + " COMPLETE" in text or "COOP-TEST FAIL" in text or "*** FANG" in text:
                break
            time.sleep(.3)
    finally:
        if process.poll() is None:
            process.terminate()
        process.wait(timeout=10)
    text = log.read_text(encoding="latin-1") if log.exists() else ""
    errors = [line for line in text.splitlines() if "COOP-TEST FAIL" in line or line.startswith("*** ")]
    for line in errors:
        print(line)
    assert not errors, "native run recorded a failure; see " + str(log)
    assert marker + " COMPLETE" in text, "native purchase test did not finish; see " + str(log)
    assert text.count(marker + " bought") == (2 if args.ammo else 4)
    if not args.ammo:
        assert text.count("phase=display") == 23
    print(f"PASS: native {kind} purchases, {args.players}p P{args.owner}.")


if __name__ == "__main__":
    main()
