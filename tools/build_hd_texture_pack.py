"""Normalize generated atlases into optional PC replacement assets; never install.

Creative repainting is performed only by the built-in image generator. This tool
converts approved generator outputs, retains original material alpha and validates
all mip pixels through the production decoder. It also reports honest coverage.
"""
from pathlib import Path
import argparse
import hashlib
import json
import struct
import subprocess
from functools import lru_cache

from PIL import Image, ImageDraw, ImageFont
from package_rat_hd_textures import tiled_rgba

ROOT = Path(__file__).resolve().parents[1]
PACK = ROOT / 'HD Texture Pack'


def checksum(path):
    with path.open('rb') as handle:
        return hashlib.file_digest(handle, 'sha256').hexdigest()


def read_json(path, default):
    return json.loads(path.read_text(encoding='utf-8')) if path.exists() else default


def atomic_json(path, data):
    path.parent.mkdir(parents=True, exist_ok=True)
    temporary = path.with_suffix('.tmp')
    temporary.write_text(json.dumps(data, indent=2), encoding='utf-8')
    temporary.replace(path)


@lru_cache(maxsize=1)
def usage_index():
    usage = read_json(PACK / 'catalog/usage.json', {})
    return {record['stem']: record for record in usage.get('textures', [])}


def review_metadata(name, job):
    usage = usage_index().get(name, {})
    roles = usage.get('material_roles', {})
    holds = []
    if (roles.get('detail_map') or roles.get('bump_map')) and not (
            roles.get('base_color') or roles.get('blend_color')):
        holds.append('Multiplicative detail/bump texture: brightness, contrast and material response require dedicated review.')
    if job.get('status') == 'generated_graphic_fidelity_review_required':
        holds.append('Generated display graphics differ from the source; hold for fidelity correction.')
    return dict(category=usage.get('category', 'unclassified'),
                material_roles=roles, world_count=usage.get('world_count', 0),
                world_users=usage.get('worlds', []),
                wrap_s_material_uses=usage.get('wrap_s_material_uses', 0),
                wrap_t_material_uses=usage.get('wrap_t_material_uses', 0),
                release_holds=holds,
                release_ready=name in ('trdrrata_01', 'trdrrata_02'))


def comparison(name, original, upgraded):
    # Layout-only comparison assembly, no creative transformations.
    w, h = original.size
    pane_w = 640
    pane_h = max(160, round(pane_w * h / w))
    sheet = Image.new('RGB', (pane_w * 2 + 60, pane_h + 98), '#20242b')
    draw = ImageDraw.Draw(sheet)
    font = ImageFont.truetype('C:/Windows/Fonts/arial.ttf', 22)
    draw.text((20, 12), name, font=font, fill='white')
    draw.text((20, 43), f'Vanilla {w} x {h}', font=font, fill='#bbc0c8')
    draw.text((pane_w + 40, 43), f'HD {upgraded.width} x {upgraded.height}', font=font, fill='#bbc0c8')
    sheet.paste(original.resize((pane_w, pane_h), Image.Resampling.NEAREST), (20, 78))
    sheet.paste(upgraded.resize((pane_w, pane_h), Image.Resampling.LANCZOS), (pane_w + 40, 78))
    target = PACK / 'previews' / (name + '.png')
    target.parent.mkdir(parents=True, exist_ok=True)
    sheet.save(target)
    return str(target.relative_to(PACK))


def build(name, scale):
    original_path = PACK / 'vanilla/packed' / (name + '.tga')
    header = bytearray(original_path.read_bytes()[:32])
    assert len(header) == 32
    width, height = struct.unpack_from('>HH', header, 28)
    assert width and height and not (width & (width - 1) or height & (height - 1))
    dimensions = width * scale, height * scale
    assert max(dimensions) <= 4096, 'replacement exceeds production decoder limit'
    generated = PACK / 'generated' / (name + '.png')
    generated_image = Image.open(generated)
    original = Image.open(PACK / 'vanilla/color' / (name + '.png')).convert('RGB')
    assert original.size == (width, height)
    generated_ratio = generated_image.width / generated_image.height
    original_ratio = width / height
    assert abs(generated_ratio / original_ratio - 1) < 0.01, 'generated aspect ratio changed'
    color = generated_image.convert('RGB').resize(dimensions, Image.Resampling.LANCZOS)
    alpha = Image.open(PACK / 'vanilla/alpha' / (name + '.png')).convert('L')
    assert alpha.size == (width, height)
    rgba = color.copy()
    rgba.putalpha(alpha.resize(dimensions, Image.Resampling.NEAREST))
    for folder, image in (('color', color), ('rgba', rgba)):
        directory = PACK / 'hd' / folder
        directory.mkdir(parents=True, exist_ok=True)
        image.save(directory / (name + '.png'))
    original_lods = header[23]
    levels, mip = [], rgba
    # Add one level per doubled resolution, retain source's authored mip depth.
    desired_lods = original_lods + scale.bit_length() - 1
    for _ in range(desired_lods):
        if min(mip.size) < 4:
            break
        levels.append(mip)
        mip = mip.resize((max(1, mip.width // 2), max(1, mip.height // 2)), Image.Resampling.BOX)
    header[16:20] = bytes(4)
    header[20:24] = bytes((0, 0, 0, len(levels)))
    struct.pack_into('>HH', header, 28, *dimensions)
    packed_path = PACK / 'hd/packed' / (name + '.tga')
    packed_path.parent.mkdir(parents=True, exist_ok=True)
    packed_path.write_bytes(bytes(header) + b''.join(tiled_rgba(level) for level in levels))
    decoder = ROOT / 'build/rat-hd-decoder/out/Release/rat_hd_decoder.exe'
    assert decoder.exists(), 'production decoder helper not built'
    temporary = PACK / 'work/decoded-hd' / (name + '.bgra')
    temporary.parent.mkdir(parents=True, exist_ok=True)
    result = subprocess.run([str(decoder), str(packed_path), str(temporary)],
                            capture_output=True, text=True)
    if result.returncode:
        raise RuntimeError(result.stderr or result.stdout)
    expected = b''.join(level.tobytes('raw', 'BGRA') for level in levels)
    assert temporary.read_bytes() == expected, 'production decoder pixel/mip mismatch'
    job = read_json(PACK / 'jobs' / (name + '.json'), {})
    status = 'user_tested_on_vehicle' if name in ('trdrrata_01', 'trdrrata_02') else 'pending_visual_and_runtime_review'
    record = dict(resource=name + '.tga', original_dimensions=[width, height],
                  generated_dimensions=list(generated_image.size),
                  replacement_dimensions=list(dimensions), scale=scale,
                  mip_count=len(levels), encoding='GC RGBA8',
                  original_packed_sha256=checksum(original_path),
                  generated_sha256=checksum(generated), packed_sha256=checksum(packed_path),
                  all_mip_pixels_decode_exactly=True, original_alpha_retained=True,
                  review_status=status, preview=comparison(name, original, color),
                  generation_job=job)
    record.update(review_metadata(name, job))
    atomic_json(PACK / 'catalog/processed' / (name + '.json'), record)
    return record


def coverage():
    original_files = list((PACK / 'vanilla/packed').glob('*.tga'))
    generated = sorted(path.stem for path in (PACK / 'generated').glob('*.png')
                       if (PACK / 'vanilla/packed' / (path.stem + '.tga')).exists())
    processed = []
    for path in sorted((PACK / 'catalog/processed').glob('*.json')):
        record = read_json(path, {})
        name = path.stem
        record.update(review_metadata(name, read_json(PACK / 'jobs' / (name + '.json'), {})))
        atomic_json(path, record)
        processed.append(record)
    inventory = read_json(PACK / 'catalog/vanilla-textures.json', {})
    data = dict(scope='whole-game important texture replacement project',
                status='in_progress', vanilla_resources_extracted=len(original_files),
                vanilla_catalog_available=bool(inventory), generated_count=len(generated),
                generated_resources=generated, packed_and_decode_verified_count=len(processed),
                runtime_user_reviewed_count=sum(record.get('review_status') == 'user_tested_on_vehicle' for record in processed),
                held_for_material_or_graphic_review=[record['resource'] for record in processed if record.get('release_holds')],
                catalog_generation_candidate_count=sum(record.get('treatment') == 'repaint_candidate' for record in usage_index().values()),
                pending_visual_runtime_review=[record['resource'] for record in processed if record.get('review_status') != 'user_tested_on_vehicle'],
                whole_pack_complete=False, regular_game_modified=False)
    atomic_json(PACK / 'catalog/coverage.json', data)
    return data


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('names', nargs='*')
    parser.add_argument('--scale', type=int, default=4, choices=(2, 4, 8))
    parser.add_argument('--new', action='store_true', help='process generated assets without a matching output record')
    args = parser.parse_args()
    names = list(args.names)
    if args.new:
        for path in sorted((PACK / 'generated').glob('*.png')):
            if not (PACK / 'vanilla/packed' / (path.stem + '.tga')).exists():
                continue  # Review variants are not additional game resources.
            record = read_json(PACK / 'catalog/processed' / (path.stem + '.json'), {})
            if record.get('generated_sha256') != checksum(path):
                names.append(path.stem)
    for name in dict.fromkeys(names):
        assert name.isascii() and all(char.isalnum() or char == '_' for char in name), 'unsafe resource name'
        record = build(name, args.scale)
        print(record['resource'], record['replacement_dimensions'], 'all mip pixels verified')
    print(json.dumps(coverage(), indent=2))


if __name__ == '__main__':
    main()
