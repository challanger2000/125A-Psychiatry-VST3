"""A10: mutate only a temporary copy of the real verified Windows bundle."""
import pathlib, shutil, subprocess, sys, tempfile
root=pathlib.Path(sys.argv[1]).resolve()
bundle=pathlib.Path(sys.argv[2]).resolve()
verifier=root/"tools/verify_artwork.py"
with tempfile.TemporaryDirectory() as tmp:
    copy=pathlib.Path(tmp)/bundle.name
    shutil.copytree(bundle,copy)
    def find(name):
        matches=list(copy.rglob(name))
        assert len(matches)==1,name
        return matches[0]
    def run():
        return subprocess.run([sys.executable,str(verifier),str(root),str(copy)],
                              capture_output=True,text=True)
    positive=run()
    assert positive.returncode==0,positive.stdout+positive.stderr
    for name, mutate, reason in [
        ("125A-PSYCHIATRY.vst3",lambda b:b"pretend plugin","missing DOS header"),
        ("LICENSE-VSTGUI.txt",lambda b:b"wrong but nonempty","Notice differs from exact source"),
        ("scene-p028h-01-depression-isolation.png",lambda b:b[:-1]+bytes([b[-1]^1]),"scene-p028h-01-depression-isolation.png"),
    ]:
        path=(copy/"Contents/x86_64-win/125A-PSYCHIATRY.vst3") if name.endswith(".vst3") else find(name)
        original=path.read_bytes()
        try:
            path.write_bytes(mutate(original))
            negative=run()
            assert negative.returncode!=0 and reason in negative.stderr,negative.stdout+negative.stderr
            print("A10 negative rejected:",name,reason)
        finally:
            path.write_bytes(original)
    assert run().returncode==0,"restored positive bundle failed"
