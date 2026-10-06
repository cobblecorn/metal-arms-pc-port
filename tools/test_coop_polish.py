"""Exercise retail co-op pickups, wallets, shops and the Mines possession objective.

Build ma_port first. Tests use isolated saves and opt-in in-game fixtures; audio stays on,
Discord stays off, and port_run closes only the process it started. Nothing runs in ordinary play.
Example: python tools/test_coop_polish.py --cases pickups wallets shop1 shop2 possess1 possess2
"""
import argparse
import json
import os
from pathlib import Path
import subprocess
import sys

ROOT = Path(__file__).resolve().parent.parent
CASES = {
    "pickups": ("WEDMmines03", 24, 9),
    "weapons": ("WEDMmines03", 24, 9),
    "migration": ("WEDMmines03", 16, 3),
    "wallets": ("WEDMmines03", 20, 5),
    "revive": ("WEDMmines01", 45, 6),
    "shop1": ("WEDMmines03", 48, 10),
    "shop2": ("WEDMmines03", 48, 10),
    "possess1": ("WEDMmines02", 75, 4),
    "possess2": ("WEDMmines02", 75, 4),
    "fast1": ("WEDMmines02", 55, 3),
    "fast2": ("WEDMmines02", 55, 3),
}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--config", choices=("Release", "Debug"), default="Release")
    parser.add_argument("--cases", choices=tuple(CASES), nargs="+", default=list(CASES))
    parser.add_argument("--players", type=int, choices=(2, 3, 4), default=2)
    parser.add_argument("--shots", type=int, default=0, help="capture a frame every N frames")
    args = parser.parse_args()
    if args.players > 2 and any(case not in ("pickups", "weapons") for case in args.cases):
        parser.error("3/4-player fixtures currently cover pickups and weapons; specify --cases pickups weapons")
    results = []
    for case in args.cases:
        mission, seconds, expected = CASES[case]
        suffix = f"_{args.players}p" if args.players > 2 else ""
        name = f"finish_{args.config.lower()}_{case}{suffix}"
        if args.players > 2:
            expected += 1
        env = os.environ.copy()
        env["MA_PORT_TEST_COOP_POLISH"] = case
        env.pop("MA_PORT_COOP_HOLD", None)  # Test real campaign gates.
        command = [sys.executable, str(ROOT / "tools/port_run.py"), "--config", args.config,
                   "--mission", mission, "--coop", str(args.players), "--seconds", str(seconds), "--name", name]
        if args.shots:
            command += ["--shots", str(args.shots)]
        run = subprocess.run(command, cwd=ROOT, env=env, capture_output=True, text=True)
        path = ROOT / "build/logs" / f"{name}.log"
        log = path.read_text(encoding="latin-1") if path.exists() else ""
        checks = [line for line in log.splitlines() if line.startswith("COOP-TEST ")]
        passed = sum(line.startswith("COOP-TEST PASS:") for line in checks)
        failed = sum(line.startswith("COOP-TEST FAIL:") for line in checks)
        errors = [line for line in log.splitlines() if line.startswith("*** ")]
        ok = run.returncode == 0 and passed == expected and failed == 0 and not errors
        result = {"case": case, "config": args.config, "players": args.players, "ok": ok, "passed": passed,
                  "expected": expected, "failed": failed, "errors": errors,
                  "checks": checks, "log": str(path)}
        results.append(result)
        print(f"{'PASS' if ok else 'FAIL'} {case}: {passed}/{expected} checks; {failed} failures", flush=True)
        for line in checks:
            print(f"  {line}", flush=True)
        if not ok:
            print(run.stdout[-6000:] + run.stderr[-2000:], flush=True)
    suffix = f"-{args.players}p" if args.players > 2 else ""
    report = ROOT / "build/logs" / f"coop-polish-{args.config.lower()}{suffix}.json"
    report.parent.mkdir(parents=True, exist_ok=True)
    # Keep the latest result for each case when a failed case is rerun by itself.
    previous = {}
    if report.exists():
        for result in json.loads(report.read_text(encoding="utf-8")):
            previous[result["case"]] = result
    previous.update((result["case"], result) for result in results)
    report.write_text(json.dumps([previous[case] for case in CASES if case in previous], indent=2) + "\n", encoding="utf-8")
    print(f"Report: {report}", flush=True)
    return 0 if all(result["ok"] for result in results) else 1


if __name__ == "__main__":
    raise SystemExit(main())
