"""Build a clean public 100-percent save for the native PC port.

Uses native-exported, mission-specific CMemCardInventory blocks. No personal
save files are read. Retail chip counts/time targets come from the game MST.
"""
from __future__ import annotations

import argparse
import hashlib
import json
import struct
import zipfile
import zlib
from pathlib import Path

from gamedata_dump import decode
from mst_list import read_mst

NAME = "100 Percent"
TAG = "".join(f"{ord(char):04X}" for char in NAME)
PROFILE_BYTES = 26724
INVENTORY_BYTES = 604
PROGRESS_BYTES = 616
PROGRESS_OFFSET = 116
RECORD_BYTES = 26792
COOP_BYTES = 24 + 8 * RECORD_BYTES


def retail_tables(mst: Path) -> tuple[list[dict], list[dict], int]:
    handle, _, entries = read_mst(str(mst))
    tables = {}
    try:
        for name, _, start, size, *_ in entries:
            if name.casefold() in ("pick_level$.csv", "multi_lvl$.csv"):
                handle.seek(start)
                tables[name.casefold()] = decode(handle.read(size))["tables"]
    finally:
        handle.close()

    def values(file: str, table: str) -> list:
        return [field["value"] for item in tables[file]
                if item["name"].casefold() == table
                for field in item["fields"]]

    campaign_values = values("pick_level$.csv", "level_names")
    pvp_values = values("multi_lvl$.csv", "deathmatchlevels")
    unlock_values = values("pick_level$.csv", "level_unlocking")
    assert len(campaign_values) == 42 * 6
    assert len(pvp_values) == 14 * 6
    levels = [dict(number=int(campaign_values[i + 5]),
                   name=campaign_values[i + 1],
                   hidden_chips=int(campaign_values[i + 3]),
                   time_target=campaign_values[i + 4])
              for i in range(0, len(campaign_values), 6)]
    maps = [dict(number=int(pvp_values[i + 5]), name=pvp_values[i + 1])
            for i in range(0, len(pvp_values), 6)]
    freebies = int(unlock_values[0])
    assert freebies == 5 and len(unlock_values) - 1 == len(maps) - freebies
    assert [level["number"] for level in levels] == list(range(1, 43))
    return levels, maps, freebies


def make_profile(levels: list[dict], pvp_count: int, freebies: int,
                 inventories: bytes) -> bytes:
    assert len(inventories) == 42 * INVENTORY_BYTES
    data = bytearray(PROFILE_BYTES)
    total_chips = sum(level["hidden_chips"] + (level["time_target"] > 0)
                      for level in levels)
    # Completed, 100 percent, vendor visited, default aim assistance. Not virtual.
    struct.pack_into("<I", data, 16, 0x4D)
    data[20:28] = bytes((1, 42, 0, 0, 0, total_chips, pvp_count - freebies, 0))
    struct.pack_into("<4f", data, 28, 0.8, 0.5, 0.75, 0.75)
    # Quick select defaults remain zero, as in InitNewProfile.
    for index, level in enumerate(levels):
        offset = PROGRESS_OFFSET + index * PROGRESS_BYTES
        target = level["time_target"]
        # These are generated completion times, not claimed playthrough records.
        seconds = max(1.0, float(int(target * 0.9))) if target > 0 else 60.0
        struct.pack_into("<f4H", data, offset, seconds, level["hidden_chips"],
                         int(target > 0), 0, 0)
        inventory = inventories[index * INVENTORY_BYTES:(index + 1) * INVENTORY_BYTES]
        primary, secondary, items, batteries = inventory[:4]
        assert 1 <= primary <= 16 and 1 <= secondary <= 16 and items <= 16
        assert batteries >= 1 and inventory[600] < primary and inventory[601] < secondary
        data[offset + 12:offset + PROGRESS_BYTES] = inventory
    # Sixteen unused custom rules, exactly the native InitNewProfile defaults.
    for index in range(16):
        offset = 25988 + index * 46
        struct.pack_into("<H", data, offset, 0x17)
        data[offset + 2:offset + 10] = "XXXX".encode("utf-16-le")
        data[offset + 38] = 15
    struct.pack_into("<4I", data, 0, 0x55501234, 12,
                     zlib.crc32(data[16:]), PROFILE_BYTES)
    return bytes(data)


def make_coop(profile: bytes) -> bytes:
    data = bytearray(COOP_BYTES)
    struct.pack_into("<6I", data, 0, 0x434F4F50, 1, RECORD_BYTES, 0, 1, 1)
    name = (NAME + "\0").encode("utf-16-le")
    data[24:24 + len(name)] = name
    struct.pack_into("<I", data, 24 + 64, 1)
    # Co-op profile data has no personal-save header, as in the native writer.
    data[24 + 68:24 + RECORD_BYTES] = bytes(16) + profile[16:]
    struct.pack_into("<I", data, 12, zlib.crc32(data[16:]))
    return bytes(data)


README = """METAL ARMS PC - 100 PERCENT SAVE
===============================

Profile: 100 Percent
For the native Windows PC port. Not a Dolphin, Xbox, or PS2 memory-card save.

INCLUDED
  - All 42 campaign missions completed and available through Replay.
  - All 14 shipped PvP maps unlocked.
  - All 104 hidden chips and all 39 available time bonus chips collected.
  - Campaign and 100-percent completion flags enabled.
  - The completion bonus / Original Concept Movie available.
  - A matching completed co-op campaign for the same profile.
  - Standard mission-specific inventories, including special-character missions.

QUICK INSTALL
  1. Close the game.
  2. Extract this ZIP completely; do not run the installer inside the ZIP.
  3. Double-click Install Save.cmd. No administrator permission is needed.
  4. Start the game and select the profile "100 Percent".
  5. For campaign/co-op, choose Replay and select a mission.

This adds a separate profile. Your other profiles and settings.ini are preserved.
If "100 Percent" already exists, its solo and co-op saves are backed up first.
The installer prints the backup location. It can be run again to reset this
downloaded profile to the included completed state.

MANUAL INSTALL
  Press Win+R, enter %APPDATA%\\MAGITS and press Enter.
  Copy the .sav in Profiles into that folder's Profiles subfolder.
  Copy the .sav in Co-op into that folder's Co-op subfolder.
  Create those subfolders if needed. Back up any same-named files before replacing.
  Copy both .sav files, not this entire extracted folder into Profiles.

CUSTOM SAVE FOLDER
  If you launch with -save-dir or MA_PORT_SAVE_DIR, install into that root instead.
  The installer honors MA_PORT_SAVE_DIR, or accepts an explicit path:
    powershell -NoProfile -ExecutionPolicy Bypass -File .\\Install.ps1 -SaveRoot "D:\\My Saves"

USING AND RESTORING
  Continue is disabled for a completed campaign; use Replay.
  Starting New Game resets campaign progress on this downloaded profile.
  Co-op partners may use their own profiles or guests; they follow P1's unlocked
  campaign. The solo and co-op campaigns remain separate, as in the PC port.
  To undo: close the game, remove only this package's two .sav files, and restore
  any matching backed-up files. Leave all other files alone.

ABOUT THIS SAVE
  This is a generated completion/unlock save, not someone's personal playthrough.
  Completion times were generated below each applicable bonus target; kill and
  washer statistics are zero. No gameplay cheat toggles are enabled. Mission
  inventories use the native game's normal level-unlock initialization.
  No personal profiles, account data, controller bindings, or settings.ini included.
  Save layout: solo profile version 12, co-op version 1. Future incompatible
  layout changes may require a new version of this package.
"""

INSTALLER = r'''param([string]$SaveRoot)
$ErrorActionPreference = 'Stop'
try {
    if (Get-Process -Name ma_port -ErrorAction SilentlyContinue) {
        throw 'Close Metal Arms PC before installing the save.'
    }
    if ([string]::IsNullOrWhiteSpace($SaveRoot)) { $SaveRoot = $env:MA_PORT_SAVE_DIR }
    if ([string]::IsNullOrWhiteSpace($SaveRoot)) {
        $SaveRoot = Join-Path ([Environment]::GetFolderPath('ApplicationData')) 'MAGITS'
    }
    $SaveRoot = [IO.Path]::GetFullPath($SaveRoot)
    $manifest = Get-Content -LiteralPath (Join-Path $PSScriptRoot 'manifest.json') -Raw | ConvertFrom-Json
    $entries = @($manifest.files | Where-Object { $_.path.EndsWith('.sav') })
    if ($entries.Count -ne 2) { throw 'The package must contain exactly two save files.' }
    $plans = @()
    foreach ($entry in $entries) {
        if ($entry.path -notmatch '^(Profiles/profile-|Co-op/coop-)[0-9A-F]+.sav$') {
            throw 'Invalid path in the package manifest.'
        }
        $source = Join-Path $PSScriptRoot $entry.path
        $sha = [Security.Cryptography.SHA256]::Create()
        try { $checksum = [BitConverter]::ToString($sha.ComputeHash([IO.File]::ReadAllBytes($source))).Replace('-', '') }
        finally { $sha.Dispose() }
        if ($checksum -ne $entry.sha256) {
            throw ('Package checksum mismatch: ' + $entry.path)
        }
        $plans += [PSCustomObject]@{Source=$source; Destination=(Join-Path $SaveRoot $entry.path); Relative=$entry.path}
    }
    # Validate all sources and back up both existing files before replacing either.
    $backupRoot = $null
    foreach ($plan in $plans) {
        if (Test-Path -LiteralPath $plan.Destination) {
            if (-not $backupRoot) {
                $backupRoot = Join-Path $SaveRoot ('Save Backups/100 Percent-' + (Get-Date -Format 'yyyyMMdd-HHmmss-fff'))
            }
            $backup = Join-Path $backupRoot $plan.Relative
            [IO.Directory]::CreateDirectory([IO.Path]::GetDirectoryName($backup)) | Out-Null
            Copy-Item -LiteralPath $plan.Destination -Destination $backup
        }
    }
    foreach ($plan in $plans) {
        [IO.Directory]::CreateDirectory([IO.Path]::GetDirectoryName($plan.Destination)) | Out-Null
        Copy-Item -LiteralPath $plan.Source -Destination $plan.Destination -Force
    }
    Write-Host ''
    Write-Host ('Installed "100 Percent" in: ' + $SaveRoot) -ForegroundColor Green
    if ($backupRoot) { Write-Host ('Previous matching saves backed up in: ' + $backupRoot) }
    Write-Host 'Start the game, select 100 Percent, then choose Replay.'
    Write-Host 'Other profiles and settings were preserved.'
    exit 0
} catch {
    Write-Host ('Installation failed: ' + $_.Exception.Message) -ForegroundColor Red
    exit 1
}
'''


def write_package(mst: Path, inventory_path: Path, output: Path) -> Path:
    levels, maps, freebies = retail_tables(mst)
    inventory = inventory_path.read_bytes()
    profile = make_profile(levels, len(maps), freebies, inventory)
    coop = make_coop(profile)
    files = {
        f"Profiles/profile-{TAG}.sav": profile,
        f"Co-op/coop-{TAG}.sav": coop,
        "README.txt": README.replace("\n", "\r\n").encode("ascii"),
        "Install.ps1": INSTALLER.replace("\n", "\r\n").encode("ascii"),
        "Install Save.cmd": ("@echo off\r\n"
                             "powershell.exe -NoProfile -ExecutionPolicy Bypass -File \"%~dp0Install.ps1\" %*\r\n"
                             "set \"save_install_result=%errorlevel%\"\r\n"
                             "echo.\r\npause\r\nexit /b %save_install_result%\r\n").encode("ascii"),
    }
    manifest = dict(profile=NAME, solo_version=12, coop_version=1,
                    completed_missions=42, pvp_maps=len(maps),
                    hidden_chips=sum(level["hidden_chips"] for level in levels),
                    bonus_chips=sum(level["time_target"] > 0 for level in levels),
                    inventories_sha256=hashlib.sha256(inventory).hexdigest(),
                    files=[dict(path=name, bytes=len(data), sha256=hashlib.sha256(data).hexdigest())
                           for name, data in files.items()])
    files["manifest.json"] = (json.dumps(manifest, indent=2) + "\n").encode("utf-8")
    output.mkdir(parents=True, exist_ok=True)
    package = output / "MetalArmsPC-100Percent-Save"
    for name, data in files.items():
        target = package / name
        target.parent.mkdir(parents=True, exist_ok=True)
        target.write_bytes(data)
    zip_path = output / (package.name + ".zip")
    with zipfile.ZipFile(zip_path, "w", compression=zipfile.ZIP_DEFLATED, compresslevel=9) as archive:
        for name, data in files.items():
            info = zipfile.ZipInfo(package.name + "/" + name, date_time=(2026, 10, 7, 0, 0, 0))
            info.compress_type = zipfile.ZIP_DEFLATED
            info.external_attr = 0o644 << 16
            archive.writestr(info, data)
    with zipfile.ZipFile(zip_path) as archive:
        assert archive.testzip() is None
        assert len(archive.namelist()) == len(files)
        for name, data in files.items():
            assert archive.read(package.name + "/" + name) == data
    print(json.dumps(dict(zip=str(zip_path.resolve()), bytes=zip_path.stat().st_size,
                          sha256=hashlib.sha256(zip_path.read_bytes()).hexdigest(),
                          **{key: manifest[key] for key in (
                              "completed_missions", "pvp_maps", "hidden_chips", "bonus_chips")}), indent=2))
    return zip_path


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--mst", type=Path, default=Path("gamedata/files/mettlearms_gc.mst"))
    parser.add_argument("--inventories", type=Path, default=Path("build/profile-inventory-templates.bin"))
    parser.add_argument("--output", type=Path, default=Path("build/exports/100-percent-save"))
    args = parser.parse_args()
    write_package(args.mst, args.inventories, args.output)
