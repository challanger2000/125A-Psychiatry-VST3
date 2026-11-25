"""P028l: explicit shipping allowlist, exact archive bytes and safe extraction."""
import hashlib, json, pathlib, sys, zipfile

ROOT = pathlib.Path(__file__).resolve().parents[1]
BUNDLE = "125A-PSYCHIATRY.vst3/"
MODULE = BUNDLE + "Contents/x86_64-win/125A-PSYCHIATRY.vst3"
RESOURCE = BUNDLE + "Contents/Resources/"

def allowed_files():
    library = json.loads((ROOT / "resource/artwork/scenes-p028h.json").read_text(encoding="utf-8"))
    names = [s["file"] for s in library["scenes"]]
    if len(names) != 40 or len(set(names)) != 40 or any(pathlib.Path(n).name != n for n in names):
        raise ValueError("invalid scene manifest")
    resources = names + ["faceplate.png", "macro-atlas.png", "therapy-atlas.png",
        "porthole-glass.png", "loss-pressed.png", "loss-released.png", "scene-corridor-bw.png",
        "psychiatry.uidesc", "scenes-p028h.json", "moduleinfo.json",
        "LICENSE-VST3-SDK.txt", "LICENSE-VSTGUI.txt", "LICENSE-ACOUSTIC-SOURCES.txt"]
    return {MODULE, BUNDLE + "desktop.ini", BUNDLE + "PlugIn.ico"} | {RESOURCE + n for n in resources}

def digest(data):
    return hashlib.sha256(data).hexdigest()

def staging_hashes(stage):
    entries = list(stage.rglob("*"))
    if any(p.is_symlink() for p in entries):
        raise ValueError("symlink in staging")
    paths = {p.relative_to(stage).as_posix():p for p in entries if p.is_file()}
    allowed = allowed_files()
    if set(paths) != allowed:
        raise ValueError(f"shipping allowlist mismatch: extra={sorted(set(paths)-allowed)} missing={sorted(allowed-set(paths))}")
    return {n:digest(p.read_bytes()) for n,p in paths.items()}

def verify_archive(archive, expected, expected_sha, destination=None):
    if digest(archive.read_bytes()) != expected_sha:
        raise ValueError("archive SHA256 mismatch")
    if set(expected) != allowed_files():
        raise ValueError("expected hashes do not cover shipping allowlist")
    with zipfile.ZipFile(archive) as z:
        names=z.namelist()
        if len(names)!=len(set(names)) or set(names)!=allowed_files():
            raise ValueError("archive shipping allowlist mismatch")
        if z.testzip() is not None:
            raise ValueError("ZIP CRC mismatch")
        for name,sha in expected.items():
            if digest(z.read(name))!=sha:
                raise ValueError("member SHA256 mismatch: "+name)
        if destination is not None:
            destination.mkdir(parents=True,exist_ok=False)
            z.extractall(destination) # exact allowlist checked before any extraction
            if staging_hashes(destination)!=expected:
                raise ValueError("extracted bytes mismatch")
    if digest(archive.read_bytes())!=expected_sha:
        raise ValueError("archive changed during verification")

def create_archive(stage, archive, destination=None):
    expected=staging_hashes(stage)
    with zipfile.ZipFile(archive,"w",compression=zipfile.ZIP_DEFLATED) as z:
        for name in sorted(expected):
            z.write(stage/name,name)
    sha=digest(archive.read_bytes())
    verify_archive(archive,expected,sha,destination)
    print("P028l exact shipping allowlist, ZIP CRC, paths, per-file SHA256 PASS:",len(expected),"members",sha)
    return sha

if __name__=="__main__":
    stage,archive=map(pathlib.Path,sys.argv[1:3])
    destination=pathlib.Path(sys.argv[3]) if len(sys.argv)>3 else None
    sha=create_archive(stage,archive,destination)
    archive.with_suffix(archive.suffix+".sha256").write_text(sha+"  "+archive.name+"\n",encoding="ascii")
