"""Run targeted retail gameplay probes with isolated saves and game snapshots."""
from pathlib import Path
import argparse
import os
import subprocess
import time

ROOT = Path(__file__).resolve().parent.parent


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("case", choices=("corrosive", "swarm", "scope", "battery", "battery3", "washer", "systems", "arena", "rebuild", "recruit", "health", "flamer"))
    parser.add_argument("--players", type=int, choices=(1, 2), default=1)
    parser.add_argument("--owner", type=int, choices=(1, 2), default=1)
    parser.add_argument("--mission")
    args = parser.parse_args()
    assert args.owner <= args.players
    modes = {"corrosive": f"corrosive-dome{args.owner}", "swarm": "swarmer-report",
             "scope": "pickup-scope", "battery": "pickup-battery", "battery3": "pickup-battery3",
             "washer": f"washer-report{args.owner}", "systems":"report-systems", "arena":f"report-arena{args.owner}", "rebuild":"report-rebuild", "recruit":"report-recruit", "health":f"report-health{args.owner}", "flamer":"audio-flamer"}
    worlds = {"corrosive": "WEBCcolis04", "swarm": "WERRreactr1", "scope": "WEWJjourn01",
              "battery": "WEDMmines02", "battery3": "WEDTtown_01", "washer": "WEDMmines02", "systems":"WEDMmines02", "arena":"WECFfacty01", "rebuild":"WEDMmines02", "recruit":"WEWCcomm_02", "health":"WEDMmines02", "flamer":"WERRreactr2"}
    mode = modes[args.case]
    marker = "CORROSIVE-DOME" if args.case == "corrosive" else "SWARMER-REPORT" if args.case == "swarm" else "PICKUP-REPORT"
    if args.case == "washer":
        marker = "WASHER-REPORT"
    if args.case in ("systems", "arena"):
        marker = "SYSTEMS-REPORT" if args.case == "systems" else "ARENA-REPORT"
    if args.case in ("rebuild", "recruit", "health", "flamer"):
        marker = args.case.upper() + "-REPORT"
    mission = args.mission or worlds[args.case]
    name = f"reddit-{args.case}-{mission.lower()}-{args.players}p-owner{args.owner}-20261018"
    log = ROOT / "build/logs" / (name + ".log")
    save = ROOT / "build/test-saves" / name
    shots = ROOT / "build/shots" / name
    save.mkdir(parents=True, exist_ok=True)
    shots.mkdir(parents=True, exist_ok=True)
    if log.exists():
        log.unlink()
    env = dict(os.environ, MA_PORT_TEST_COOP_POLISH=mode)
    env.pop("MA_PORT_COOP_HOLD", None)
    command = [str(ROOT / "build/Release/ma_port.exe"), "-data", str(ROOT / "gamedata/files"),
               "-mission", mission, "-save-dir", str(save), "-discord-app-id", "off",
               "-res", "960x720", "-windowed", "-mute", "-port-diag", "-instance-label", name, "-log", str(log),
               "-asset-log", str(log.with_suffix(".assets.log")), "-shots", str(shots), "-shot-every", "600"]
    if args.players > 1:
        command += ["-coop", str(args.players)]
    process = subprocess.Popen(command, cwd=ROOT, env=env, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    print(f"Native {args.case} {mission}; own PID {process.pid}", flush=True)
    deadline, seen = time.monotonic() + 80, 0
    try:
        while process.poll() is None and time.monotonic() < deadline:
            text = log.read_text(encoding="latin-1") if log.exists() else ""
            lines = [s for s in text.splitlines() if s.startswith(marker) or s.startswith("COOP-TEST ")]
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
    assert "COOP-TEST FAIL" not in text and "*** FANG" not in text, str(log)
    assert marker + " COMPLETE" in text, "native probe did not complete: " + str(log)
    # Keep a compressed evidence frame without accumulating multi-megabyte BMPs.
    from PIL import Image
    frames = sorted(shots.glob("*.bmp"), key=lambda p: p.stat().st_mtime)
    if frames:
        with Image.open(frames[-1]) as frame:
            frame.save(shots / "latest.png")
        assert shots.resolve().is_relative_to((ROOT / "build/shots").resolve())
        for frame in frames:
            frame.unlink()
    print("PASS:", marker, flush=True)


if __name__ == "__main__":
    main()
