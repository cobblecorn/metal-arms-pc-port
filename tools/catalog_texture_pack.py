"""Inventory retail texture uses and prioritize authored HD work without launching the game.

Reads the actual GameCube MST, APE material registers and every embedded WLD mesh.
Numeric lightmaps, masks, reflection maps, font pages and particle sprites are
tracked separately from repaint candidates. The extractor owns vanilla-textures.json.
"""
from __future__ import annotations

import argparse
from collections import Counter, defaultdict
import csv
from datetime import datetime, timezone
import hashlib
import json
from pathlib import Path
import re
import struct

from mst_list import read_mst

ROOT = Path(__file__).resolve().parents[1]
NAME = re.compile(rb"(?<![A-Za-z0-9_$\-])([A-Za-z0-9_$\-]{1,15})\x00")
MESH_BYTES = 136
MATERIAL_BYTES = 72
TECHNICAL_ROLES = {"lightmap", "z_mask", "emissive_mask", "specular_mask", "bump_map", "reflection"}
PILOTS = {
    "te_1metal03": "Repeatable dark scratched metal material, used in 29 retail worlds.",
    "te_1metal01": "Common authored metal tile; inspect original before repaint.",
    "te_1concr02": "Repeatable mottled concrete material, used in 21 retail worlds.",
    "tewj_rust01": "Repeatable rusty metal material used in Journeys and other worlds.",
    "tedm_met03": "Gray-blue scratched metal tile used in Mines 2/3 and PvP 11.",
    "trdggltch00": "Main Glitch character UV atlas; preserve all UV islands and material alpha.",
    "trmrrata_01": "Red RAT main UV atlas, matching the existing yellow RAT pilot workflow.",
    "trmrrata_02": "Red RAT secondary UV atlas; preserve layout and material alpha.",
    "twdm4wep": "Highly shared weapon/material atlas; preserve every existing panel island.",
}


def u32(blob: bytes, offset: int) -> int:
    return struct.unpack_from(">I", blob, offset)[0]


def u16(blob: bytes, offset: int) -> int:
    return struct.unpack_from(">H", blob, offset)[0]


def shader_table() -> list[dict]:
    source = (ROOT / "ma/Lib/Fang2/fshaders.cpp").read_text()
    start = source.index("const FShaderReg_t FShaders_aShaderRegs")
    body = source[start:source.index("};", start)]
    # Comments are the source's ordered PASM shader names. Some include commas
    # or explanatory flag names; only the first shader symbol names the row.
    rows = re.findall(r"//\s*(FSHADERS_[A-Za-z0-9_]+)[^\n]*\n\s*(\d+),(.*?)(?=//\s*FSHADERS_|$)", body, re.S)
    table = []
    for name, count, registers in rows:
        regs = re.findall(r"FSHADERS_REG_[A-Z0-9_]+", registers)
        assert len(regs) == 8, (name, regs)
        table.append({"name": name, "count": int(count), "registers": regs[:int(count)]})
    assert len(table) == 55 and table[16]["name"] == "FSHADERS_oBASE_ADD_rbENV"
    assert table[44]["name"] == "FSHADERS_oBASE_LERP_vLAYER_ADD_rbENV"
    return table


def texture_instance(blob: bytes, offset: int, texture_names: dict) -> tuple[str, int] | None:
    # Current retail disk CFTexInst is 24 bytes; FShTexInst's name offset is
    # BE16 at +24, with texture wrapping flags at +16. These match production
    # gcmesh.cpp/FShTexInst_t and were validated against named RAT materials.
    if not offset or offset + 28 > len(blob):
        return None
    name_offset = u16(blob, offset + 24)
    if not name_offset or name_offset >= len(blob):
        return None
    name = blob[name_offset:name_offset + 16].split(b"\0", 1)[0]
    if not re.fullmatch(rb"[A-Za-z0-9_$\-]{1,15}", name):
        return None
    stem = name.decode("ascii").casefold()
    if stem not in texture_names:
        return None
    return stem, u32(blob, offset + 16)


def role_for_register(register: str, shader: str) -> str:
    if register == "FSHADERS_REG_DETAILMAP":
        return "detail_map"
    if "LIQUID_ENV" in shader:
        return "reflection" if register.endswith("LAYER0") else "liquid_color"
    if register.endswith("LAYER0"):
        return "base_color"
    if "rbENV" in shader or "rbSREFLECT" in shader:
        reflection_layer = "LAYER2" if "LERP" in shader else "LAYER1"
        if register.endswith(reflection_layer):
            return "reflection"
    return "blend_color"


def scan_materials(blob: bytes, owner: str, kind: str, shaders: list[dict], texture_names: dict) -> tuple[list[dict], dict]:
    if len(blob) < MESH_BYTES:
        raise ValueError("mesh smaller than disk header")
    material_count = blob[68]
    material_offset = u32(blob, 120)
    if material_count == 0 and material_offset == 0:
        return [], {"materials": 0, "shader_indices": [], "unknown_shader_indices": [], "zero_material_collider": True}
    if not material_offset or material_offset + material_count * MATERIAL_BYTES > len(blob):
        raise ValueError("material table out of bounds")
    matches = []
    stats = {"materials": material_count, "shader_indices": [], "unknown_shader_indices": []}
    for index in range(material_count):
        material = material_offset + index * MATERIAL_BYTES
        light, surface = struct.unpack_from(">II", blob, material)
        light_index, shader_index = blob[material + 8], u16(blob, material + 10)
        stats["shader_indices"].append(shader_index)
        if shader_index >= len(shaders):
            stats["unknown_shader_indices"].append(shader_index)
            continue
        shader = shaders[shader_index]
        if surface + shader["count"] * 4 > len(blob):
            raise ValueError("surface registers out of bounds")

        def add(instance, role, register):
            if instance:
                stem, flags = instance
                matches.append({"stem": stem, "owner": owner, "kind": kind,
                                "material_index": index, "role": role,
                                "shader": shader["name"], "register": register,
                                "wrap_s": bool(flags & 1), "wrap_t": bool(flags & 2),
                                "render_target": bool(flags & 0x40), "cube_map": bool(flags & 0x800)})

        for reg_index, register in enumerate(shader["registers"]):
            if register in {"FSHADERS_REG_LAYER0", "FSHADERS_REG_LAYER1", "FSHADERS_REG_LAYER2", "FSHADERS_REG_LAYER3", "FSHADERS_REG_DETAILMAP"}:
                add(texture_instance(blob, u32(blob, surface + reg_index * 4), texture_names), role_for_register(register, shader["name"]), register)
        # All retail material headers use the same ten standard light registers.
        # The production converter follows them with triples until 0xffffffff.
        if light_index >= 6:
            stats.setdefault("unknown_light_indices", []).append(light_index)
            continue
        if light + 40 > len(blob):
            raise ValueError("light registers out of bounds")
        for reg_index, role in enumerate(("z_mask", "emissive_mask", "specular_mask", "bump_map")):
            add(texture_instance(blob, u32(blob, light + reg_index * 4), texture_names), role, f"LIGHT_REG_{reg_index}")
        cursor = light + 40
        # Validated finite loop and bounds, following _ConvertMaterialRegisters.
        while cursor + 4 <= len(blob):
            pointer = u32(blob, cursor)
            if pointer == 0xffffffff:
                break
            if cursor + 12 > len(blob):
                raise ValueError("lightmap tail out of bounds")
            add(texture_instance(blob, pointer, texture_names), "lightmap", "LIGHTMAP_TAIL")
            cursor += 12
        else:
            raise ValueError("unterminated lightmap tail")
    return matches, stats


def validated_strings(blob: bytes, names: dict) -> set[str]:
    return {m[1].decode("ascii").casefold() for m in NAME.finditer(blob)
            if m[1].decode("ascii").casefold() in names}


def mesh_category(stem: str, known_characters: dict) -> str:
    if stem in known_characters:
        return "characters"
    if stem.startswith("gw") or stem.startswith("gp_"):
        return "weapons_pickups"
    if re.search(r"rat|tank|skiff|exavo|landr|land01|shuttl", stem) and stem.startswith(("gr", "go", "gc")):
        return "vehicles"
    if stem.startswith("gr"):
        return "characters"
    if stem.startswith("gf"):
        return "effects_ui"
    return "environment_props"


def categorize(record: dict) -> tuple[str, str, list[str]]:
    roles = set(record["material_roles"])
    hints = []
    if record["font_users"]:
        return "fonts", "preserve_technical", ["Actual FNT reference: retain glyph layout; redraw only in a dedicated font pass."]
    if roles and roles <= TECHNICAL_ROLES:
        return "technical", "preserve_technical", ["Actual mesh registers identify this as lightmap/mask/bump/reflection data."]
    if record["render_target_material_uses"]:
        return "technical", "manual_review", ["A material marks this as a render target; do not bake a static repaint blindly."]
    if re.fullmatch(r"[0-9]+", record["stem"]):
        if "lightmap" in roles:
            return "technical", "preserve_technical", ["Numbered resource is proven lightmap data."]
        return "unclassified", "manual_review", ["Anonymous numeric texture lacks a safe authored diffuse identity."]
    if record["stem"] in {"black", "white", "null", "blank", "default", "demorefl"} or min(record["width"], record["height"]) < 8:
        return "technical", "preserve_technical", ["Fallback/constant/tiny helper texture; retain original."]
    prefix = record["stem"][:2]
    counts = record["mesh_category_counts"]
    if prefix == "tr" or counts.get("characters", 0) > counts.get("environment_props", 0) + counts.get("vehicles", 0):
        category = "vehicles" if counts.get("vehicles", 0) > counts.get("characters", 0) else "characters"
    elif prefix == "tw" or counts.get("weapons_pickups", 0) > counts.get("environment_props", 0):
        category = "weapons_pickups"
    elif prefix in {"te", "to"} or record["worlds"]:
        category = "environment_props"
    elif prefix in {"tf", "th", "ti", "tm"} or record["particle_users"]:
        category = "effects_ui"
    elif roles & {"base_color", "blend_color", "liquid_color", "detail_map"}:
        category = "environment_props" if counts else "unclassified"
    else:
        category = "unclassified"
    if roles & {"base_color", "blend_color", "liquid_color", "detail_map"}:
        state = "repaint_candidate"
        hints.append("Authored color/detail role proven in material registers; visual/UV review required before generation.")
    elif category in {"effects_ui", "characters", "weapons_pickups", "environment_props"}:
        state = "manual_review"
        hints.append("Naming/source/particle provenance only; inspect functional alpha and intended use first.")
    else:
        state = "manual_review"
        hints.append("Retained in full inventory; no unsupported authored-paintable assumption.")
    if roles & TECHNICAL_ROLES:
        hints.append("Also used as technical material data: retain original channels; review each role before replacing globally.")
    if record["particle_users"]:
        hints.append("Particle resource uses this texture; preserve silhouette, alpha and animation frame layout.")
    if record["wrap_s_material_uses"] or record["wrap_t_material_uses"]:
        hints.append("Sampler wraps in actual materials; match repeating edges and internal seam orientation.")
    if roles & {"liquid_color", "detail_map"}:
        hints.append("Procedural/liquid/detail blending depends on original brightness and channels; controlled material pass needed.")
    return category, state, hints


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--archive", type=Path, default=ROOT / "gamedata/files/mettlearms_gc.mst")
    parser.add_argument("--output", type=Path, default=ROOT / "HD Texture Pack")
    args = parser.parse_args()
    output = args.output.resolve()
    catalog_dir = output / "catalog"
    catalog_dir.mkdir(parents=True, exist_ok=True)
    extraction_path = catalog_dir / "vanilla-textures.json"
    extracted = json.loads(extraction_path.read_text())
    vanilla = {r["stem"].casefold(): r for r in extracted["textures"]}
    shaders = shader_table()
    characters = {m["resource"].casefold(): m for m in json.loads((ROOT / "tools/character_models.json").read_text())["models"]}
    archive, header, entries = read_mst(str(args.archive))
    textures = {name[:-4].casefold(): name for name, *_ in entries if name.casefold().endswith(".tga")}
    meshes = {name[:-4].casefold(): name for name, *_ in entries if name.casefold().endswith(".ape")}
    assert set(textures) == set(vanilla), "Extractor manifest must cover every texture directory entry"
    usages = defaultdict(list)
    string_users = defaultdict(set)
    world_placed_meshes = {}
    font_pages = defaultdict(set)
    resource_scans = []
    problems = []
    material_mesh_count = 0
    for resource, flags, offset, size, *_ in entries:
        extension = resource.rsplit(".", 1)[-1].casefold()
        if extension not in {"ape", "wld", "csv", "fpr", "fnt", "gt", "sma"}:
            continue
        archive.seek(offset)
        blob = archive.read(size)
        strings = validated_strings(blob, textures)
        for stem in strings:
            string_users[stem].add(resource)
        if extension == "fnt":
            # ftext_Load constructs each page as font.szName + decimal index
            # (ftext.cpp:2537). The disk font name starts at byte5, page count
            # is byte581; validate every constructed page against the MST.
            if len(blob) > 581:
                font_name = blob[5:39].split(b"\0", 1)[0].decode("ascii").casefold()
                page_count = blob[581]
                for page in range(page_count):
                    stem = font_name + str(page)
                    if stem in textures:
                        font_pages[stem].add(resource)
                    else:
                        problems.append({"resource": resource, "font_page": page, "error": "constructed font page missing"})
        if extension == "ape":
            try:
                matches, stats = scan_materials(blob, resource, "ape", shaders, textures)
                material_mesh_count += 1
                for match in matches:
                    usages[match["stem"]].append(match)
                resource_scans.append({"resource": resource, "kind": "ape", **stats})
            except (ValueError, struct.error) as error:
                problems.append({"resource": resource, "error": str(error)})
        elif extension == "wld":
            if len(blob) < 44 or u32(blob, 0) != len(blob):
                problems.append({"resource": resource, "error": "world serialized size invalid"})
                continue
            count, starts, sizes = struct.unpack_from(">III", blob, 4)
            embedded_scans = []
            world_placed_meshes[resource] = sorted(meshes[stem] for stem in validated_strings(blob, meshes))
            for index in range(count):
                mesh_offset = u32(blob, starts + index * 4)
                mesh_size = u32(blob, sizes + index * 4)
                try:
                    assert mesh_offset + mesh_size <= len(blob)
                    matches, stats = scan_materials(blob[mesh_offset:mesh_offset + mesh_size], f"{resource}#mesh{index:03d}", "world_mesh", shaders, textures)
                    material_mesh_count += 1
                    for match in matches:
                        match["world"] = resource
                        usages[match["stem"]].append(match)
                    embedded_scans.append({"mesh_index": index, **stats})
                except (ValueError, struct.error, AssertionError) as error:
                    problems.append({"resource": resource, "mesh_index": index, "error": str(error)})
            resource_scans.append({"resource": resource, "kind": "wld", "mesh_count": count,
                                   "referenced_ape_meshes": world_placed_meshes[resource], "embedded_meshes": embedded_scans})
    archive.close()

    source_users = defaultdict(set)
    # Exact quoted known resource symbols give source provenance, not inferred
    # runtime reachability. Keep this separate from actual material usage counts.
    for source_root in (ROOT / "ma/App/ma", ROOT / "port"):
        for path in sorted(source_root.rglob("*")):
            if path.suffix not in {".cpp", ".h", ".inl"}:
                continue
            text = path.read_text(errors="replace")
            for match in re.finditer(r'"([A-Za-z0-9_$\-]{1,15})(?:\.tga)?"', text):
                stem = match[1].casefold()
                if stem in textures:
                    line = text.count("\n", 0, match.start()) + 1
                    source_users[stem].add(f"{path.relative_to(ROOT).as_posix()}:{line}")

    mesh_worlds = defaultdict(set)
    for world, placed in world_placed_meshes.items():
        for mesh in placed:
            mesh_worlds[mesh].add(world)
    records = []
    for stem, resource in sorted(textures.items()):
        original = vanilla[stem]
        uses = usages[stem]
        ape = {u["owner"] for u in uses if u["kind"] == "ape"}
        direct_worlds = {u["world"] for u in uses if "world" in u}
        placed_worlds = {w for mesh in ape for w in mesh_worlds[mesh]}
        declared = string_users[stem]
        counts = Counter(mesh_category(name[:-4].casefold(), characters) for name in ape)
        row = {"resource": resource, "stem": stem, "width": original["width"], "height": original["height"],
               "format": original["format"], "source_sha256": original["source_sha256"],
               "paths": original["paths"], "decode_status": original["status"],
               "duplicate_color_of": original.get("duplicate_color_of"),
               "duplicate_rgba_of": original.get("duplicate_rgba_of"),
               "material_roles": dict(Counter(u["role"] for u in uses)),
               "mesh_users": sorted(ape), "mesh_count": len(ape),
               "embedded_world_mesh_count": len({u["owner"] for u in uses if u["kind"] == "world_mesh"}),
               "material_use_count": len(uses), "direct_material_worlds": sorted(direct_worlds),
               "placed_mesh_worlds": sorted(placed_worlds), "worlds": sorted(direct_worlds | placed_worlds),
               "world_count": len(direct_worlds | placed_worlds),
               "raw_name_users": sorted(declared), "raw_name_user_count": len(declared),
               "particle_users": sorted(x for x in declared if x.endswith(".fpr")),
               "font_users": sorted({x for x in declared if x.endswith(".fnt")} | font_pages[stem]),
               "source_users": sorted(source_users[stem]), "mesh_category_counts": dict(counts),
               "wrap_s_material_uses": sum(u["wrap_s"] for u in uses),
               "wrap_t_material_uses": sum(u["wrap_t"] for u in uses),
               "render_target_material_uses": sum(u["render_target"] for u in uses),
               "material_uses": uses,
               "alpha_policy": "Preserve original alpha/material channels; authored color preview is separate RGB."}
        category, state, notes = categorize(row)
        row.update(category=category, treatment=state, notes=notes)
        row["score"] = row["world_count"] * 30 + row["mesh_count"] * 3 + min(row["material_use_count"], 500) + (200 if stem == "trdggltch00" else 0)
        if min(row["width"], row["height"]) >= 64:
            row["score"] += 25
        if row["duplicate_color_of"]:
            row["score"] -= 30
        records.append(row)

    # Reuse identical color generation across aliases, while preserving each
    # alias's independently extracted original alpha and runtime resource name.
    color_groups = defaultdict(list)
    for record in records:
        key = (record["width"], record["height"], vanilla[record["stem"]]["hashes"]["color_pixels"])
        color_groups[key].append(record)
    groups = []
    for (width, height, color_hash), rows in sorted(color_groups.items()):
        preferred = sorted(rows, key=lambda r: (r["treatment"] != "repaint_candidate", -r["score"], r["resource"]))[0]
        groups.append({"color_hash": color_hash, "width": width, "height": height, "representative": preferred["resource"],
                       "aliases": [r["resource"] for r in rows],
                       "preserve_each_alias_alpha": True})
        for row in rows:
            row["generation_representative"] = preferred["resource"]
            row["color_group_aliases"] = [r["resource"] for r in rows]

    categories = Counter(r["category"] for r in records)
    treatments = Counter(r["treatment"] for r in records)
    summary = {"texture_resources": len(records), "unique_rgb_color_groups": len(groups),
               "mesh_resources_scanned": sum(r["kind"] == "ape" for r in resource_scans),
               "worlds_scanned": sum(r["kind"] == "wld" for r in resource_scans),
               "material_meshes_scanned": material_mesh_count,
               "textures_with_actual_material_uses": sum(bool(r["material_uses"]) for r in records),
               "by_category": dict(categories), "by_treatment": dict(treatments),
               "material_parse_problems": len(problems),
               "unknown_surface_shaders": sorted({i for r in resource_scans for i in (r.get("unknown_shader_indices", []) if r["kind"] == "ape" else [i for e in r["embedded_meshes"] for i in e["unknown_shader_indices"]])})}
    manifest = {"schema_version": 1, "generated_utc": datetime.now(timezone.utc).isoformat(),
                "source_archive": str(args.archive.resolve()), "source_archive_sha256": hashlib.file_digest(args.archive.open("rb"), "sha256").hexdigest(),
                "summary": summary,
                "method": {"texture_inventory": "Every .tga entry matched the complete production-decoder extraction manifest.",
                           "materials": "BE32/BE16 disk layouts: FMesh136, FMeshMaterial72, FShTexInst28. Shader rows parsed from production fshaders.cpp; base/blend/detail and lightmaps/masks/reflection separated.",
                           "worlds": "Every .wld embedded mesh pointer and size followed. Placed APE names validated against actual archive; these are references, not a gameplay visibility assertion.",
                           "name_references": "Only complete null-terminated names validated against MST known resources. Raw string references recorded separately from proven material uses.",
                           "limits": "Inventory proves asset references and material roles, not screen coverage, visual suitability, UV correctness of generated art or gameplay testing. Naming hints stay manual-review without color material proof."},
                "shaders": shaders, "parse_problems": problems, "resources": resource_scans,
                "color_groups": groups, "textures": records}
    level_source = (ROOT / "ma/App/ma/level.cpp").read_text()
    titles = defaultdict(list)
    for match in re.finditer(r'"([^"\n]+)",\s*//\s*pszTitle[^\n]*\n\s*"([A-Za-z0-9_]+)",\s*//\s*pszWorldResName', level_source):
        titles[match[2].casefold() + ".wld"].append({"title": match[1], "source_line": level_source.count("\n", 0, match.start()) + 1})
    world_inventory = []
    for world in sorted(world_placed_meshes):
        used = [r for r in records if world in r["worlds"]]
        world_inventory.append({"resource": world, "source_titles": titles[world],
                                "texture_count": len(used), "by_treatment": dict(Counter(r["treatment"] for r in used)),
                                "textures": [r["resource"] for r in used],
                                "generation_representatives": sorted({r["generation_representative"] for r in used if r["treatment"] == "repaint_candidate"})})
    manifest["worlds"] = world_inventory
    (catalog_dir / "usage.json").write_text(json.dumps(manifest, indent=2) + "\n")
    (catalog_dir / "world-coverage.json").write_text(json.dumps(world_inventory, indent=2) + "\n")
    queue_root = catalog_dir / "queues"
    queue_root.mkdir(exist_ok=True)
    queue_summary = {}
    for category in sorted(categories):
        rows = sorted([r for r in records if r["category"] == category], key=lambda r: (-r["score"], r["resource"]))
        candidate_rows = [r for r in rows if r["treatment"] == "repaint_candidate" and r["generation_representative"] == r["resource"]]
        manual_rows = [r for r in rows if r["treatment"] == "manual_review"]
        queue = {"category": category, "all_resource_count": len(rows),
                 "generation_candidates": candidate_rows, "manual_review": manual_rows,
                 "preserve_technical": [r["resource"] for r in rows if r["treatment"] == "preserve_technical"],
                 "alias_resources": [r["resource"] for r in rows if r["generation_representative"] != r["resource"]]}
        (queue_root / f"{category}.json").write_text(json.dumps(queue, indent=2) + "\n")
        queue_summary[category] = {"all": len(rows), "unique_generation_candidates": len(candidate_rows), "manual_review": len(manual_rows)}
    fields = ["resource", "category", "treatment", "width", "height", "world_count", "mesh_count", "material_use_count", "generation_representative", "score"]
    with (catalog_dir / "ranked-textures.csv").open("w", newline="") as stream:
        writer = csv.DictWriter(stream, fields, extrasaction="ignore")
        writer.writeheader()
        writer.writerows(sorted(records, key=lambda r: (-r["score"], r["resource"])))
    pilots = [{"resource": r["resource"], "reason": PILOTS[r["stem"]],
               "world_count": r["world_count"], "mesh_count": r["mesh_count"],
               "roles": r["material_roles"], "original_color": r["paths"]["color"],
               "original_alpha": r["paths"]["alpha"], "wrap_s_uses": r["wrap_s_material_uses"],
               "wrap_t_uses": r["wrap_t_material_uses"]} for r in records if r["stem"] in PILOTS]
    (catalog_dir / "pilot-candidates.json").write_text(json.dumps(pilots, indent=2) + "\n")
    (catalog_dir / "queue-summary.json").write_text(json.dumps(queue_summary, indent=2) + "\n")
    notes = """# Whole-game texture work inventory

`vanilla-textures.json` contains every retail texture and lossless original RGB,
RGBA and alpha channels. `usage.json` records every resource, mesh material role,
world linkage, wrapping flags and source/name provenance. `ranked-textures.csv`
offers a compact sorted inventory; `queues/` retains the full category workload.

`generation_candidates` are authored material candidates with real color/detail
register evidence. They still require visual and UV review. Lightmaps, masks,
reflection helper maps and font pages remain original unless their own dedicated
material or font pass is undertaken. `manual_review` covers all unknown/functional
assets rather than silently discarding them. Shared RGB colors have one generation
representative; each runtime alias keeps its own original alpha channel.

The full inventory does not mean the HD pack is complete. Track finished images,
UV/material validation and runtime tests separately. Do not apply this work to the
live game without a separate user instruction.
"""
    (catalog_dir / "README.md").write_text(notes)
    print(json.dumps(summary, indent=2))
    print(json.dumps(queue_summary, indent=2))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
