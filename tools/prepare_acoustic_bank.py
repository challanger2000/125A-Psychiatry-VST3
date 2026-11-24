"""Build-time only: verify archived OGG source blobs, decode and embed selected
mono PCM16 excerpts. No network, files or decoder in the plugin at runtime.
Run: python tools/prepare_acoustic_bank.py <generated-header-path>
"""
import array
import hashlib
import json
import math
import os
from pathlib import Path
import shutil
import subprocess
import sys

ROOT = Path(__file__).resolve().parent.parent
RATE = 24000
SOURCES = (
    "whispers/VOXWhsp_WhispersCreepy_OS_01.ogg",
    "whispers/VOXWhsp_WhispersCreepyReverb_OS_01.ogg",
    "doors/DOORKnck_OfficeDoorKnock_OS_01.ogg",
    "footsteps/FEETHmn_ShoeMetalWalk_OS_01.ogg",
    "footsteps/FEETHmn_ShoeMetalWalk_OS_03.ogg",
    "creaks/WOODFric_CreaksCreepy_OS_01.ogg",
)
def require(condition, message):
    if not condition:
        raise ValueError(message)

def decoder():
    try:
        import imageio_ffmpeg
    except ImportError:
        path = shutil.which("ffmpeg")
        if not path:
            raise RuntimeError("Install imageio-ffmpeg==0.6.0 or ffmpeg for development builds")
        return path
    return imageio_ffmpeg.get_ffmpeg_exe()

def decode(path, ffmpeg):
    raw = subprocess.check_output([
        ffmpeg, "-nostdin", "-v", "error", "-i", str(path),
        "-t", "5", "-ar", str(RATE), "-ac", "1", "-f", "f32le", "-"
    ], timeout=60)
    require(len(raw) % 4 == 0, "bad float PCM: " + str(path))
    x = array.array("f")
    x.frombytes(raw)
    if sys.byteorder != "little":
        x.byteswap()
    require(RATE // 100 <= len(x) <= RATE * 5, "unexpected length: " + str(path))
    require(all(math.isfinite(v) for v in x), "nonfinite PCM: " + str(path))
    peak = max(abs(v) for v in x)
    require(peak > .0001, "silent recording: " + str(path))
    # Remove only leading/trailing low-level silence, with 10 ms padding.
    audible = [i for i,v in enumerate(x) if abs(v) >= peak * .015]
    lo = max(0, audible[0] - RATE // 100)
    hi = min(len(x), audible[-1] + RATE // 100 + 1)
    x = x[lo:hi]
    require(len(x) >= RATE // 100, "too-short usable excerpt: " + str(path))
    rms = math.sqrt(sum(v*v for v in x) / len(x))
    require(rms > .00001, "silent trimmed recording: " + str(path))
    scale = min(.90 / peak, .20 / rms)
    fade = max(1, min(RATE // 100, len(x) // 2))
    pcm = [round(32767 * v * scale * min(1, i/fade, (len(x)-1-i)/fade))
           for i,v in enumerate(x)]
    require(max(abs(v) for v in pcm) > 10, "inaudible output: " + str(path))
    return pcm

def main():
    manifest = json.loads((ROOT / "sound-library/sources.json").read_text(encoding="utf-8"))
    by_path = {r["path"]: r for r in manifest}
    require(len(by_path) == len(manifest), "duplicate provenance entries")
    ffmpeg = decoder()
    header = ["// Generated from pinned, attributed Oculus Audio Pack 01 recordings.",
              "// CC BY 4.0; see LICENSE-ACOUSTIC-SOURCES.txt.",
              "#pragma once", "#include <cstddef>", "#include <cstdint>",
              "namespace psychiatry::acoustic_bank {",
              "struct Clip { const std::int16_t* pcm; std::size_t frames; };",
              "inline constexpr double sampleRate=24000.0;"]
    names = []
    for n, relative in enumerate(SOURCES):
        path = "sound-library/originals/" + relative
        row = by_path[path]
        require(row["license"] == "CC-BY-4.0", "unexpected source license: " + path)
        blob = (ROOT / path).read_bytes()
        digest = hashlib.sha1(("blob %d\0" % len(blob)).encode() + blob).hexdigest()
        require(digest == row["git_blob_sha1"], "source hash mismatch: " + path)
        pcm = decode(ROOT / path, ffmpeg)
        name = "clip_" + str(n)
        names.append(name)
        header.append("inline constexpr std::int16_t " + name + "[]={")
        header.extend(",".join(map(str, pcm[i:i+24])) + "," for i in range(0,len(pcm),24))
        header.append("};")
        print(path, "verified", digest, "frames", len(pcm))
    header.append("inline constexpr Clip paranoia[]={")
    header.extend("{" + name + ",sizeof(" + name + ")/sizeof(" + name + "[0])}," for name in names)
    header += ["};", "} // namespace psychiatry::acoustic_bank"]
    out = Path(sys.argv[1])
    out.parent.mkdir(parents=True, exist_ok=True)
    temp = out.with_suffix(".tmp")
    temp.write_text("\n".join(header)+"\n", encoding="utf-8")
    os.replace(temp, out)

if __name__ == "__main__":
    main()
