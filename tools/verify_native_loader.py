"""Windows loader probe for the actual extracted x64 module (not a factory substitute)."""
import ctypes, pathlib, sys
from ctypes import wintypes
from verify_windows_pe import inspect_pe

def check_module(module):
    module=module.resolve(strict=True)
    _,imports=inspect_pe(module.read_bytes())
    # Call LoadLibraryExW directly: ctypes.WinDLL replaces ERROR_MOD_NOT_FOUND
    # with FileNotFoundError and discards the numeric Win32 error.
    kernel=ctypes.WinDLL("kernel32",use_last_error=True)
    load=kernel.LoadLibraryExW
    load.argtypes=(wintypes.LPCWSTR,wintypes.HANDLE,wintypes.DWORD)
    load.restype=wintypes.HMODULE
    get=kernel.GetProcAddress
    get.argtypes=(wintypes.HMODULE,ctypes.c_char_p)
    get.restype=ctypes.c_void_p
    free=kernel.FreeLibrary
    free.argtypes=(wintypes.HMODULE,)
    free.restype=wintypes.BOOL
    ctypes.set_last_error(0)
    handle=load(str(module),None,0x00000100|0x00001000)
    if not handle:
        raise ctypes.WinError(ctypes.get_last_error())
    try:
        for symbol in (b"GetPluginFactory",b"InitDll",b"ExitDll"):
            if not get(handle,symbol):
                raise RuntimeError("missing loader export "+repr(symbol))
        print("Windows native loader resolved module imports:",", ".join(imports))
    finally:
        if not free(handle):
            raise ctypes.WinError(ctypes.get_last_error())

if __name__=="__main__":
    try:
        check_module(pathlib.Path(sys.argv[1]))
    except OSError as exc:
        print(f'Native loader OS error: winerror={getattr(exc, "winerror", None)}', file=sys.stderr)
        raise
