#!/usr/bin/env python3
"""Native X11 input/visual and independent PCM oracles for the frozen Linux bundle."""
import array, math, os, re, struct, subprocess, sys, time, wave
from pathlib import Path
from PIL import Image, ImageChops

E = Path(os.environ["PSY_EVIDENCE"])
def run(*a):
    return subprocess.check_output(a, text=True)
def cmd(*a):
    subprocess.run(a, check=True)
def locate():
    candidates = []
    for line in run("xwininfo", "-root", "-tree").splitlines():
        m = re.search(r"^\s*(0x[0-9a-f]+).*?\b960x640[+-]\d+[+-]\d+", line)
        if not m:
            continue
        try:
            info = run("xwininfo", "-id", m[1])
        except subprocess.CalledProcessError:
            continue
        if "Map State: IsViewable" not in info:
            continue
        x = int(re.search(r"Absolute upper-left X:\s*(-?\d+)", info)[1])
        y = int(re.search(r"Absolute upper-left Y:\s*(-?\d+)", info)[1])
        candidates.append((x, y, m[1]))
    positions = {(x,y) for x,y,_ in candidates}
    if len(positions) != 1:
        raise RuntimeError(f"expected a visible 960x640 plugin client at one position: {candidates}")
    x,y = next(iter(positions))
    assert x >= 0 and y >= 0 and x+960 <= 1920 and y+640 <= 1080, positions
    print("X11 client 960x640", candidates, flush=True)
    return x,y

def mouse(*a):
    cmd("xdotool", *map(str,a))
def delta(a,b,region):
    diff = ImageChops.difference(Image.open(E/a).convert("RGB"),Image.open(E/b).convert("RGB")).crop(region)
    return sum(1 for rgb in diff.getdata() if sum(rgb)>24)
def pcm(p):
    data = subprocess.check_output(["ffmpeg","-v","error","-i",str(p),"-f","f32le","-acodec","pcm_f32le","-ar","48000","-ac","2","-"])
    out = array.array("f")
    out.frombytes(data)
    assert all(map(math.isfinite,out)), p
    return out

op,*args = sys.argv[1:]
if op == "fixture":
    with wave.open(str(E/"input.wav"),"wb") as w:
        w.setparams((2,2,48000,0,"NONE","not compressed"))
        raw=bytearray()
        for i in range(48000*4):
            t=i/48000
            g=min(1.,t*100,(4.-t)*100)
            l=g*(.19*math.sin(2*math.pi*220*t)+.07*math.sin(2*math.pi*997*t))
            r=g*(.16*math.sin(2*math.pi*311*t)+.06*math.sin(2*math.pi*1301*t))
            raw.extend(struct.pack("<hh",round(l*32767),round(r*32767)))
        w.writeframes(raw)
elif op == "audio":
    src,dry,wet=map(pcm,(E/"input.wav",E/"dry.wav",E/"wet.wav"))
    assert len(src)==len(dry)==len(wet)==384000,(len(src),len(dry),len(wet))
    rms=lambda a: math.sqrt(sum(x*x for x in a)/len(a))
    dryerr=rms([a-b for a,b in zip(src,dry)])
    wetdiff=rms([a-b for a,b in zip(dry,wet)])
    print(f"dry rms error={dryerr:.9g}, wet-dry rms={wetdiff:.9g}, wet rms={rms(wet):.9g}")
    assert dryerr < .002 and wetdiff > .003 and rms(wet)>.005 and max(map(abs,wet)) < 1., "audio oracle failed"
elif op == "motion_on":
    changed=delta("motion-a.png","motion-b.png",(390,225,565,400))
    fixed=delta("motion-a.png","motion-b.png",(50,50,175,175))
    print("MOTION active pixels",changed,"faceplate",fixed)
    assert changed>=20 and fixed<=16
elif op == "motion_off":
    changed=delta("still-a.png","still-b.png",(390,225,565,400))
    print("MOTION stopped pixels",changed)
    assert changed<=16
else:
    x,y=locate()
    if op=="geometry":
        pass
    elif op=="shot":
        # Root crop captures the actual visible image, not a resource bitmap.
        cmd("import","-window","root","-crop",f"960x640+{x}+{y}","+repage",str(E/args[0]))
    else:
        px,py=map(int,args)
        mouse("mousemove",x+px,y+py)
        time.sleep(.1)
        if op=="click":
            mouse("click",1)
        elif op=="double":
            mouse("click","--repeat",2,"--delay",100,1)
        elif op=="drag":
            mouse("mousedown",1)
            try:
                for k in range(1,9):
                    mouse("mousemove",x+px,y+py-4*k)
                    time.sleep(.05)
            finally:
                mouse("mouseup",1)
        else:
            raise ValueError(op)
