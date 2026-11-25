"""Verify the exact uploaded P028c assets and all 13 native parameter bindings."""
import hashlib, json, pathlib, struct, sys, xml.etree.ElementTree as ET
root = pathlib.Path(sys.argv[1])
manifest = json.loads((root / "SHA256.json").read_text())
for name, digest in manifest.items():
    assert hashlib.sha256((root / name).read_bytes()).hexdigest() == digest, name
g = json.loads((root / "geometry.json").read_text())
ui = ET.parse(root / "resource/psychiatry.uidesc").getroot()
tags = {x.attrib["name"]: int(x.attrib["tag"]) for x in ui.findall("./control-tags/control-tag")}
knobs = ui.findall(".//view[@class='CAnimKnob']")
displays = ui.findall(".//view[@class='CParamDisplay']")
assert len(knobs) == len(displays) == 13
for i, item in enumerate(g["macro_controls"] + [dict(g["therapy"], label="THERAPY")]):
    label, size = item["label"], item["sprite_size"]
    view = [v for v in knobs if v.attrib["control-tag"] == label]
    assert len(view) == 1 and tags[label] == (100+i if i<12 else 200)
    assert [int(x) for x in view[0].attrib["size"].split(",")] == [size,size]
    assert [int(x) for x in view[0].attrib["origin"].split(",")] == [x-size//2 for x in item["center"]]
    assert len([v for v in displays if v.attrib["control-tag"] == label]) == 1
for asset, dimensions, rgba in [
    ("faceplate.png",(1536,1024),False),("macro-atlas.png",(1024,2048),True),
    ("therapy-atlas.png",(1792,3584),True),("porthole-glass.png",(768,768),True),
    ("loss-pressed.png",(110,106),True),("loss-released.png",(110,106),True)]:
    data = (root / "resource/artwork" / asset).read_bytes()
    assert data[:8] == b"\x89PNG\r\n\x1a\n"
    assert struct.unpack(">II",data[16:24]) == dimensions, asset
    assert data[25] == (6 if rgba else 2), asset
for atlas, size in [("MacroAtlas","128, 128"),("TherapyAtlas","224, 224")]:
    attrs = ui.find(f"./bitmaps/bitmap[@name='{atlas}']").attrib
    assert attrs["multiframe-num-frames"] == "128"
    assert attrs["mulitframe-frames-per-row"] == "8"
    assert attrs["multiframe-size"] == size
assert tags["LOSS_OF_CONTROL"]==9000 and tags["MONITOR_MOTION"]==9002
assert tags["BYPASS"]==999 and tags["UI_ZOOM"]==9003 and "UI_LANGUAGE" not in tags
print(f"P028c: {len(manifest)} SHA256 entries; 13 knob/parameter/readout bindings; atlas geometry PASS")

# P028h: exact library bytes, dimensions, unique complete assignment and code mapping.
library_path = root / "resource/artwork/scenes-p028h.json"
library = json.loads(library_path.read_text(encoding="utf-8"))
scene_names = [s["file"] for s in library["scenes"]]
assert len(scene_names) == len(set(scene_names)) == 40
assert list(library["worlds"]) == [x["label"] for x in g["macro_controls"]]
assigned = [n for ns in library["worlds"].values() for n in ns]
assert len(assigned) == 40 and set(assigned) == set(scene_names)
assert all(3 <= len(ns) <= 4 for ns in library["worlds"].values())
scene_hashes = {}
for s in library["scenes"]:
    name = s["file"]
    assert pathlib.Path(name).name == name and name.startswith("scene-p028h-")
    data = (library_path.parent / name).read_bytes()
    assert len(data) == s["bytes"]
    assert hashlib.sha256(data).hexdigest() == s["sha256"], name
    assert data[:8] == b"\x89PNG\r\n\x1a\n"
    assert struct.unpack(">II", data[16:24]) == (768, 768) and data[25] == 2, name
    assert name in library["worlds"][s["effect_world"]]
    scene_hashes[name] = s["sha256"]
assert len(set(scene_hashes.values())) == 40
import re
header = (root / "vst3/scene_sequence.h").read_text()
assert re.findall(r'"(scene-[^"]+\.png)"', header) == ["scene-corridor-bw.png"] + scene_names
rows = re.findall(r'\{\{([0-9,]+)\}\}', header)
assert [[int(i) for i in row.split(",") if int(i)] for row in rows] == [[scene_names.index(n)+1 for n in ns] for ns in library["worlds"].values()]
counts = re.search(r'kWorldSceneCounts\{([0-9,]+)\}', header).group(1)
assert list(map(int, counts.split(","))) == [len(ns) for ns in library["worlds"].values()]
print("P028h: 40 scene hashes, 768x768 RGB, unique 12-world mapping and C++ table PASS")

# Optional second argument: the actual built VST3 bundle, not the source tree.
# The full source layout/manifest checks above still execute first.
if len(sys.argv) > 2:
    bundle = pathlib.Path(sys.argv[2])
    assert bundle.is_dir() and bundle.name == "125A-PSYCHIATRY.vst3", bundle
    def unique_file(name):
        files = [p for p in bundle.rglob(name) if p.is_file()]
        assert len(files) == 1, f"Expected exactly one {name}, found {len(files)}"
        return files[0]
    expected_xml = root / "resource/psychiatry.uidesc"
    assert unique_file("psychiatry.uidesc").read_bytes() == expected_xml.read_bytes(), "Bundled GUI definition differs from verified source"
    templates = ui.findall("./template")
    assert len(templates) == 1 and templates[0].attrib["name"] == "view"
    assert templates[0].attrib["size"] == "1536, 1024"
    assert templates[0].attrib["bitmap"] == "Faceplate"
    for kind, tag in [("CTextButton", "LOSS_OF_CONTROL"), ("CCheckBox", "BYPASS"),
                      ("CCheckBox", "MONITOR_MOTION"), ("COptionMenu", "UI_ZOOM")]:
        views = ui.findall(f".//view[@class='{kind}'][@control-tag='{tag}']")
        assert len(views) == 1, tag
        if tag == "LOSS_OF_CONTROL":
            assert views[0].attrib["kick-style"] == "true"
            assert views[0].attrib["custom-view-name"] == "PSYCHIATRY_LOSS"
    assert len(ui.findall(".//view[@custom-view-name='PSYCHIATRY_OBSERVATION']")) == 1
    assert not ui.findall(".//view[@control-tag='UI_LANGUAGE']")
    assets = (
        "faceplate.png", "macro-atlas.png", "therapy-atlas.png",
        "porthole-glass.png", "loss-pressed.png", "loss-released.png",
        "scene-corridor-bw.png",
    )
    for name in assets:
        path = unique_file(name)
        assert hashlib.sha256(path.read_bytes()).hexdigest() == manifest["resource/artwork/" + name], f"Packaged PNG hash mismatch: {name}"
    for name, digest in scene_hashes.items():
        assert hashlib.sha256(unique_file(name).read_bytes()).hexdigest() == digest, name
    assert unique_file("scenes-p028h.json").read_bytes() == library_path.read_bytes()
    notices = {
        "LICENSE-VST3-SDK.txt": root / "external/vst3sdk/LICENSE.txt",
        "LICENSE-VSTGUI.txt": root / "external/vst3sdk/vstgui4/LICENSE",
        "LICENSE-ACOUSTIC-SOURCES.txt": root / "resource/LICENSE-ACOUSTIC-SOURCES.txt",
    }
    for name, source in notices.items():
        assert source.is_file() and source.stat().st_size > 0, source
        assert unique_file(name).read_bytes() == source.read_bytes(), f"Notice differs from exact source: {name}"
    from verify_windows_pe import inspect_pe
    module = bundle / "Contents/x86_64-win/125A-PSYCHIATRY.vst3"
    assert module.is_file(), "Branded x64 binary missing"
    binary = module.read_bytes()
    exports, imports = inspect_pe(binary)
    expected_version = re.search(r'project\(PsychiatryAudioPrototype VERSION ([0-9.]+)', (root/"CMakeLists.txt").read_text()).group(1)
    info = json.loads(re.sub(r',(?=\s*[}\]])', '', unique_file("moduleinfo.json").read_text(encoding="utf-8")))
    assert info["Name"] == "125A-PSYCHIATRY" and info["Version"] == expected_version
    assert {c["CID"] for c in info["Classes"]} == {"125A20D2A3D04F919B1074B70119CA26", "125A20D3A3D04F919B1074B70119CA26"}
    assert all(c["Version"] == expected_version for c in info["Classes"])
    # Counterexamples must be rejected by the same executable parser.
    for bad in (b"pretend plugin", binary[:64], binary[:0x3c]+b"\xff\xff\xff\x7f"+binary[0x40:]):
        try: inspect_pe(bad)
        except (ValueError, struct.error): pass
        else: raise AssertionError("malformed PE accepted")
    print("P028k packaged GUI: 47 exact PNG hashes, 3 exact notices, PE32+ AMD64 DLL exports and moduleinfo PASS")
    print("PE import DLLs:", ", ".join(imports))
