"""Create/check the exact staging ZIP, without trusting filenames or CRC alone."""
import hashlib, pathlib, sys, zipfile
stage=pathlib.Path(sys.argv[1]); archive=pathlib.Path(sys.argv[2])
expected={p.relative_to(stage).as_posix():hashlib.sha256(p.read_bytes()).hexdigest()
          for p in stage.rglob("*") if p.is_file()}
assert expected and "125A-PSYCHIATRY.vst3/Contents/x86_64-win/125A-PSYCHIATRY.vst3" in expected
with zipfile.ZipFile(archive,"w",compression=zipfile.ZIP_DEFLATED) as z:
    for name in sorted(expected):z.write(stage/name,name)
with zipfile.ZipFile(archive) as z:
    assert z.testzip() is None
    names=z.namelist()
    assert len(names)==len(set(names)) and set(names)==set(expected)
    for name,digest in expected.items():
        assert hashlib.sha256(z.read(name)).hexdigest()==digest,name
print("P028k ZIP CRC, paths, full per-file SHA256 PASS:",len(expected), "members",
      hashlib.sha256(archive.read_bytes()).hexdigest())
