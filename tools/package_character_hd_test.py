"""Create an isolated optional character texture test using the existing EXE.

No generation, gameplay code edits or normal-save writes. The base archive is
copied and only the selected texture directory entries are redirected to appended
HD data; the existing archive builder checks every unrelated byte/entry.
"""
from pathlib import Path
import json
import shutil

import package_rat_hd_textures as archive_builder

ROOT = Path(__file__).resolve().parents[1]
PACK = ROOT / 'HD Texture Pack'
OUT = ROOT / 'build/export/HD_Character_Test'
DATA = ROOT / 'gamedata/files'
ARCHIVE = DATA / 'characters_hd_preview.mst'
SAVE_ROOT = ROOT / 'build/test-saves/characters-hd-user'


def main():
    stopped = json.loads((PACK / 'logs/character-generation-stop.json').read_text())
    names = stopped['completed_names']
    assert len(names) == 25 and len(set(names)) == len(names)
    records = []
    textures = []
    for name in names:
        path = PACK / 'hd/packed' / (name + '.tga')
        record = json.loads((PACK / 'catalog/processed' / (name + '.json')).read_text())
        assert record['all_mip_pixels_decode_exactly']
        assert record['original_alpha_retained']
        assert not record.get('release_holds'), f'Held draft: {name}'
        assert archive_builder.digest(path) == record['packed_sha256']
        textures.append(path)
        records.append(dict(resource=name + '.tga',
                            dimensions=record['replacement_dimensions'],
                            mip_count=record['mip_count'],
                            sha256=record['packed_sha256'],
                            review_status=record['review_status']))
    OUT.mkdir(parents=True, exist_ok=True)
    source_hash = archive_builder.digest(DATA / 'mettlearms_gc.mst')
    archive_builder.BASE = DATA / 'mettlearms_gc.mst'
    archive_builder.ARCHIVE = ARCHIVE
    archive_builder.OUT = PACK / 'hd/packed'
    archive = archive_builder.make_archive(textures)
    assert archive_builder.digest(archive_builder.BASE) == source_hash
    if not SAVE_ROOT.exists():
        live = Path.home() / 'AppData/Roaming/MAGITS'
        if live.exists():
            SAVE_ROOT.mkdir(parents=True)
            for directory in ('Profiles', 'Co-op'):
                if (live / directory).exists():
                    shutil.copytree(live / directory, SAVE_ROOT / directory)
            if (live / 'settings.ini').exists():
                shutil.copyfile(live / 'settings.ini', SAVE_ROOT / 'settings.ini')
        else:
            SAVE_ROOT.mkdir(parents=True)
    launcher = ROOT / 'HD Character Test.cmd'
    command = ('@echo off\r\ncd /d "' + str(ROOT) + '"\r\n'
               'start "" "' + str(ROOT / 'build/Release/ma_port.exe') + '" '
               '-data "' + str(DATA) + '" -mst ' + ARCHIVE.name + ' '
               '-save-dir "' + str(SAVE_ROOT) + '" '
               '-log "' + str(ROOT / 'build/logs/characters-hd-user-test.log') + '" '
               '-asset-log "' + str(ROOT / 'build/logs/characters-hd-user-assets.log') + '"\r\n')
    launcher.write_bytes(command.encode('utf-8'))
    desktop = Path('D:/Desktop/Metal Arms - HD Character Test.cmd')
    shutil.copyfile(launcher, desktop)
    manifest = dict(status='optional manual character visual test ready',
                    archive=archive, textures=records,
                    included_families=['Glitch', 'friendly miner droids', 'Krunk', 'Loader',
                                       'Mil grunts', 'Titans', 'Predators', 'Sentries',
                                       'Corrosive (partial)', 'zombies'],
                    held_character_resources=stopped['held_names'],
                    launcher=str(desktop), isolated_save_directory=str(SAVE_ROOT),
                    game_launched=False, normal_archive_unchanged=True,
                    game_executables_changed=False, normal_saves_modified=False,
                    runtime_uv_and_material_review_pending=True)
    (OUT / 'manifest.json').write_text(json.dumps(manifest, indent=2))
    (OUT / 'README.txt').write_text(
        'Metal Arms HD character preview\n\n'
        'Open Metal Arms - HD Character Test.cmd on the Desktop. It starts at the\n'
        'normal menu with a separate archive and a private copy of your current\n'
        'saves. Choose any unlocked campaign level; no mission/cutscene is forced.\n\n'
        'Glitch and 24 additional saved character texture sheets are included.\n'
        'Friendly yellow miner droid sheets, Krunk, Loader, Mil grunts, Titans,\n'
        'Predators, Sentries, Corrosive parts and zombies are upgraded wherever\n'
        'those authored textures are used. Unfinished sheets stay vanilla.\n'
        'Other texture categories, including the RAT, remain vanilla in this test.\n\n'
        'The regular archive and executable are untouched; use the regular shortcut\n'
        'for normal play. Original material alpha and mip chains are retained.\n'
        'All replacement mip pixels pass the production decoder, and every unrelated\n'
        'archive entry/original data byte matches the base archive. These are visual\n'
        'drafts; in-game UV/material appearance still needs your manual test.\n'
        'Personal copied saves are excluded from any distributable texture pack.\n')
    print(json.dumps(dict(changed_textures=len(textures), archive=archive,
                          launcher=str(desktop), game_launched=False), indent=2))


if __name__ == '__main__':
    main()
