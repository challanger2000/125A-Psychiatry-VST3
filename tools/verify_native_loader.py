"""Windows loader probe for the actual extracted x64 module (not a factory substitute)."""
import ctypes, pathlib, sys
from verify_windows_pe import inspect_pe

def check_module(module):
    module=module.resolve(strict=True)
    _,imports=inspect_pe(module.read_bytes())
    # The actual module's normal LoadLibraryEx resolves direct and transitive
    # imports (including API-set contracts); no PATH fallback to arbitrary DLLs.
    library=ctypes.WinDLL(str(module),winmode=0x00000100|0x00001000)
    for symbol in ("GetPluginFactory","InitDll","ExitDll"):
        if not getattr(library,symbol):
            raise RuntimeError("missing loader export "+symbol)
    print("Windows native loader resolved module imports:",", ".join(imports))

if __name__=="__main__":
    try:
        check_module(pathlib.Path(sys.argv[1]))
    except OSError as exc:
        print(f'Native loader OS error: winerror={getattr(exc, "winerror", None)}', file=sys.stderr)
        raise
