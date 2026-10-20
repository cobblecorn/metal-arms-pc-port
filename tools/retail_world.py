"""Read GameCube world entity properties without loading or modifying the game.

The structures match FData_WorldFileHeader_t and CFWorldShapeInit in Fang's
fdata.h/fworld.h. Proprietary decoded output belongs under ignored build/.
"""
from pathlib import Path
import argparse
import json
import struct
from gamedata_dump import decode


def entities(path: Path):
    data = path.read_bytes()
    if len(data) < 44:
        raise ValueError("Truncated world header")
    header = struct.unpack_from(">11I", data)
    init, size = header[9:11]
    if header[0] != len(data) or init + size > len(data) or size < 16:
        raise ValueError("Invalid world/init size")
    count = struct.unpack_from(">I", data, init)[0]
    base = init + 16
    if count * 64 > size - 16:
        raise ValueError("World shape array extends past init data")
    result = []
    for index in range(count):
        entry = base + index * 64
        kind, shape = struct.unpack_from(">2I", data, entry)
        game_data = struct.unpack_from(">I", data, entry + 60)[0]
        if not game_data:
            continue
        position = base + game_data
        if position < base + count * 64 or position + 16 > init + size:
            raise ValueError(f"Invalid shape {index} properties offset")
        length = struct.unpack_from(">I", data, position)[0]
        if position + length > init + size:
            raise ValueError(f"Shape {index} properties extend past init data")
        tables = decode(data[position:position + length])["tables"]
        matrix = struct.unpack_from(">12f", data, entry + 8)
        props = {t["name"].lower(): [f["value"] for f in t["fields"]] for t in tables}
        record = {"shape": index, "shape_type": kind, "position": matrix[9:],
                  "matrix": matrix, "properties": props}
        result.append(record)
    return result


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("world", type=Path)
    parser.add_argument("--property", help="Only show entities with this property")
    parser.add_argument("--output", type=Path)
    args = parser.parse_args()
    records = entities(args.world)
    if args.property:
        records = [r for r in records if args.property.lower() in r["properties"]]
    text = json.dumps(records, indent=2) + "\n"
    if args.output:
        args.output.write_text(text, encoding="utf-8")
    else:
        print(text, end="")


if __name__ == "__main__":
    main()
