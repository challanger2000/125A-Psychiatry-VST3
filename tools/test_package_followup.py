"""R06 targeted negatives against the same verified real staging bytes."""
import pathlib, shutil, struct, subprocess, sys, tempfile, zipfile
from verify_windows_pe import inspect_pe
from verify_package_zip import MODULE, create_archive, staging_hashes, verify_archive, digest

stage=pathlib.Path(sys.argv[1]).resolve()
loader=pathlib.Path(__file__).with_name("verify_native_loader.py")
module=stage/MODULE
original=module.read_bytes()
_,imports=inspect_pe(original)
with tempfile.TemporaryDirectory() as tmp:
    work=pathlib.Path(tmp)
    archive=work/"positive.zip"
    sha=create_archive(stage,archive)
    expected=staging_hashes(stage)
    def reject(call,reason):
        try:call()
        except ValueError as e:
            if reason not in str(e):raise
        else:raise AssertionError("accepted negative: "+reason)
        print("R06 negative rejected:",reason)
    # Clear executable permissions on all sections, retaining valid RVA mappings.
    bad=bytearray(original)
    pe=struct.unpack_from("<I",bad,0x3c)[0]
    coff=pe+4
    count=struct.unpack_from("<H",bad,coff+2)[0]
    table=coff+20+struct.unpack_from("<H",bad,coff+16)[0]
    for i in range(count):
        at=table+40*i+36
        flags=struct.unpack_from("<I",bad,at)[0]
        struct.pack_into("<I",bad,at,flags&~0x20000000)
    reject(lambda:inspect_pe(bad),"export target not executable")
    # Introduce an unexpected extra file in a real copied staging tree.
    copy=work/"stage"
    shutil.copytree(stage,copy)
    extra=copy/"unexpected.txt"
    extra.write_text("negative control")
    reject(lambda:create_archive(copy,work/"bad.zip"),"shipping allowlist mismatch")
    extra.unlink()
    assert staging_hashes(copy)==expected
    with zipfile.ZipFile(archive,"a") as z:z.writestr("unexpected.txt","negative")
    reject(lambda:verify_archive(archive,expected,sha),"archive SHA256 mismatch")
    reject(lambda:verify_archive(archive,expected,digest(archive.read_bytes())),"archive shipping allowlist mismatch")
    # A well-formed PE with an unresolvable imported DLL must fail the real loader.
    positive=subprocess.run([sys.executable,str(loader),str(module)],capture_output=True,text=True)
    if positive.returncode:raise AssertionError(positive.stdout+positive.stderr)
    import_name=imports[0].encode("ascii")
    old=import_name+b"\0"
    fake=(b"Q"* (len(import_name)-4))+b".dll\0"
    assert old in original and fake not in original
    mutated=original.replace(old,fake)
    _,mutated_imports=inspect_pe(mutated)
    assert fake[:-1].decode("ascii") in mutated_imports
    changed=copy/MODULE
    changed.write_bytes(mutated)
    negative=subprocess.run([sys.executable,str(loader),str(changed)],capture_output=True,text=True)
    if negative.returncode==0 or "winerror=126" not in negative.stderr:
        raise AssertionError(negative.stdout+negative.stderr)
    print("R06 missing dependency rejected by actual Windows loader")
    changed.write_bytes(original)
    assert staging_hashes(copy)==expected
    assert subprocess.run([sys.executable,str(loader),str(changed)]).returncode==0
print("R06 targeted package negatives PASS")
