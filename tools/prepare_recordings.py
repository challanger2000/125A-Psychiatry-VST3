"""Download pinned, licensed engineering fixtures; never include them in product.
Run: python tools/prepare_recordings.py <temporary-directory>
Requires imageio-ffmpeg==0.6.0 (bundled decoder, QA only). Input provenance verified using Git blob SHA-1.
"""
import array
import hashlib
import json
import math
from pathlib import Path
import subprocess
import sys
from urllib.parse import quote
from urllib.request import urlopen, Request
import imageio_ffmpeg

FFMPEG = imageio_ffmpeg.get_ffmpeg_exe()

RATE = 48000
N = RATE * 4
TAIL = RATE * 2
out = Path(sys.argv[1])
out.mkdir(parents=True, exist_ok=True)
manifest = json.loads(Path(__file__).with_name("recording_sources.json").read_text())
samples = {}
for row in manifest:
    name = row["name"]
    url = "https://raw.githubusercontent.com/" + row["repository"] + "/" + row["commit"] + "/" + quote(row["path"], safe="/")
    with urlopen(Request(url, headers={"User-Agent": "125A-engineering-fixtures"}), timeout=45) as response:
        blob = response.read(8_000_001)
    assert len(blob) < 8_000_001
    digest = hashlib.sha1(("blob %d\0" % len(blob)).encode() + blob).hexdigest()
    assert digest == row["git_blob_sha1"], (name, digest)
    local = out / (name + Path(row["path"]).suffix)
    local.write_bytes(blob)
    pcm = subprocess.check_output([FFMPEG, "-v", "error", "-i", str(local), "-t", "8", "-ar", str(RATE), "-ac", "2", "-f", "f32le", "-"])
    values = array.array("f")
    values.frombytes(pcm)
    if sys.byteorder != "little":
        values.byteswap()
    assert len(values) >= 100 and len(values) % 2 == 0
    peak = max(abs(x) for x in values)
    assert math.isfinite(peak) and peak > 1e-5
    samples[name] = [x / peak for x in values]
    print(name, "verified", digest, "frames", len(values)//2)
def blank():
    return [0.0] * (2 * N)
def add(dst, key, time, gain, length=8, pan=0.0):
    s = samples[key]
    start = round(time * RATE)
    frames = min(len(s)//2, N-start, round(length*RATE))
    for i in range(frames):
        fade = min(1., (i+1)/48, (frames-i)/240)
        dst[2*(start+i)] += s[2*i]*gain*fade*min(1.,1-pan)
        dst[2*(start+i)+1] += s[2*i+1]*gain*fade*min(1.,1+pan)
def save(name, dst):
    peak = max(abs(x) for x in dst)
    assert peak > .001
    data = array.array("f", (x * .70/peak for x in dst))
    data.extend([0.] * (TAIL*2))
    if sys.byteorder != "little":
        data.byteswap()
    (out / (name + ".f32")).write_bytes(data.tobytes())
    print("fixture",name,"frames",len(data)//2)
drums = blank()
for i in range(16):
    t = i*.25
    add(drums,"hat",t,.19 if i%2==0 else .10,.22,.28)
    if i%4 in (0,3):
        add(drums,"kick",t,.70,.5)
    if i%4==2:
        add(drums,"snare",t,.54,.42,-.12)
save("recorded_drums",drums)
guitar = blank()
for i,k in enumerate(["guitar_a","guitar_a","guitar_c","guitar_d","guitar_a","guitar_c","guitar_d","guitar_a"]):
    add(guitar,k,i*.5,.55,.47,(-1 if i%2 else 1)*.16)
save("recorded_guitar",guitar)
voice = blank()
add(voice,"breath",0,.38,2.3)
add(voice,"laugh",2.3,.55,1.7)
save("recorded_vocal_foley",voice)
room = blank()
add(room,"room",0,.65,4.)
save("recorded_machine_room",room)
(out / "ATTRIBUTION.txt").write_text(
    "Versilian Community Sample Library, Versilian Studios LLC — CC0 1.0. https://github.com/sgossner/VCSL\n"
    "Meta Steel Guitar — CC0 1.0. https://github.com/facebookincubator/Meta-open-sound\n"
    "Oculus Audio Pack 01 by Oculus VR, LLC (https://developers.meta.com/vr/downloads/package/oculus-audio-pack-1/), licensed under CC BY 4.0 (https://creativecommons.org/licenses/by/4.0/). Files renamed to UCS convention; OGG previews transcoded from original WAVs.\n"
    "Modifications: decoded/resampled, stereo conversion, selected and arranged excerpts, level scaling, fades, silence padding, and 125A PSYCHIATRY processing. Engineering fixtures only; not part of the product. Vocal foley is breathing/laughter, not singing or speech.\n"
    + json.dumps(manifest,indent=2),encoding="utf-8")
