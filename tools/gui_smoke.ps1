# P009: Windows-only runtime GUI probe against Steinberg's official EditorHost.
# This validates an actual HWND, click-to-parameter-driven CRT change, and
# fixed editor and control interaction. Does not use Studio Pro or audio input.
$ErrorActionPreference='Stop'
Add-Type -AssemblyName System.Drawing
Add-Type -AssemblyName System.Windows.Forms
Add-Type -TypeDefinition @'
using System;
using System.Runtime.InteropServices;
public static class PSYGUI {
    [StructLayout(LayoutKind.Sequential)] public struct POINT { public int X,Y; }
    [StructLayout(LayoutKind.Sequential)] public struct RECT { public int Left,Top,Right,Bottom; }
    [DllImport("user32.dll")] public static extern bool SetWindowPos(IntPtr hwnd,IntPtr after,int x,int y,int cx,int cy,uint flags);
    [DllImport("user32.dll")] public static extern bool GetClientRect(IntPtr hwnd,ref RECT rect);
    [DllImport("user32.dll")] public static extern bool ClientToScreen(IntPtr hwnd,ref POINT point);
    [DllImport("user32.dll")] public static extern bool SetForegroundWindow(IntPtr hwnd);
    [DllImport("user32.dll")] public static extern void mouse_event(uint flags,uint x,uint y,uint data,UIntPtr extra);
}
'@
function Screenshot([string]$name) {
    $bounds=[System.Windows.Forms.Screen]::PrimaryScreen.Bounds
    $image=[System.Drawing.Bitmap]::new($bounds.Width,$bounds.Height)
    $graphics=[System.Drawing.Graphics]::FromImage($image)
    try {
        $graphics.CopyFromScreen($bounds.Location,[System.Drawing.Point]::Empty,$bounds.Size)
        $image.Save((Join-Path (Resolve-Path 'artifacts').Path $name),
            [System.Drawing.Imaging.ImageFormat]::Png)
    } finally { $graphics.Dispose() }
    return $image
}
function ClickClient([IntPtr]$hwnd,[int]$x,[int]$y) {
    $point=[PSYGUI+POINT]::new()
    $point.X=[int][math]::Round($x*1.25);$point.Y=[int][math]::Round($y*1.25)
    if(-not [PSYGUI]::ClientToScreen($hwnd,[ref]$point)) {
        throw 'P009 GUI test: ClientToScreen failed'
    }
    [System.Windows.Forms.Cursor]::Position=
        [System.Drawing.Point]::new($point.X,$point.Y)
    Start-Sleep -Milliseconds 140
    [PSYGUI]::mouse_event(2,0,0,0,[UIntPtr]::Zero)
    Start-Sleep -Milliseconds 100
    [PSYGUI]::mouse_event(4,0,0,0,[UIntPtr]::Zero)
}
$build=Join-Path $PSScriptRoot '..\build-vst3'
$hostExe=Get-ChildItem -LiteralPath $build -Recurse -File -Filter 'editorhost.exe' |
    Sort-Object FullName | Select-Object -First 1
$bundle=Get-ChildItem -LiteralPath $build -Recurse -Directory -Filter '125A-PSYCHIATRY.vst3' |
    Where-Object { $_.FullName -match '[\\/]+VST3[\\/]+Release[\\/]' } |
    Sort-Object FullName | Select-Object -First 1
if(-not $hostExe -or -not $bundle) { throw 'P009 GUI test: host or plugin bundle missing' }
$proc=$null;$before=$null;$after=$null;$german=$null;$bypassed=$null;$restored=$null
try {
    $arguments='--componentHandler "'+$bundle.FullName+'"'
    $proc=Start-Process -FilePath $hostExe.FullName -ArgumentList $arguments -PassThru
    $handle=[IntPtr]::Zero
    for($i=0;$i -lt 70;++$i){
        Start-Sleep -Milliseconds 350
        $proc.Refresh()
        if($proc.HasExited) {
            throw "P009 GUI test: EditorHost exited with $($proc.ExitCode)"
        }
        $handle=$proc.MainWindowHandle
        if($handle -ne [IntPtr]::Zero) {break}
    }
    if($handle -eq [IntPtr]::Zero) {throw 'P009 GUI test: no Win32 editor window'}
    Write-Host "P009 opened HWND=$handle with official Steinberg EditorHost"
    [PSYGUI]::SetForegroundWindow($handle) | Out-Null
    Start-Sleep -Milliseconds 700
    $bounds=[System.Windows.Forms.Screen]::PrimaryScreen.Bounds
    $work=[System.Windows.Forms.Screen]::PrimaryScreen.WorkingArea
    Write-Host "P028j runtime desktop $($bounds.Width)x$($bounds.Height), work area $($work.Width)x$($work.Height)"
    if($work.Width -lt 1200 -or $work.Height -lt 840){throw 'P030 insufficient native desktop capacity'}
    if(-not [PSYGUI]::SetWindowPos($handle,[IntPtr]::Zero,($work.X+10),($work.Y+10),0,0,0x15)){throw 'P028j could not position actual EditorHost window'}
    Start-Sleep -Milliseconds 300
    New-Item -ItemType Directory -Force -Path 'artifacts' | Out-Null
    $origin=[PSYGUI+POINT]::new()
    if(-not [PSYGUI]::ClientToScreen($handle,[ref]$origin)) {
        throw 'P009 GUI test: cannot locate editor client origin'
    }
    # Default is now ON. Disable it explicitly for static bypass/restore comparisons.
    ClickClient $handle 302 493
    Start-Sleep -Milliseconds 250
    $before=Screenshot 'P009-Editor-Before.png'
    # P030 fixed 960x640 faceplate; map existing 768x512 probe coordinates by 1.25.
    function Delta($a,$b,[int]$x,[int]$y,[int]$w,[int]$h) {
        $x=[int][math]::Round($x*1.25);$y=[int][math]::Round($y*1.25)
        $w=[int][math]::Round($w*1.25);$h=[int][math]::Round($h*1.25)
        $count=0
        for($yy=$y;$yy -lt $y+$h;$yy+=2){
            for($xx=$x;$xx -lt $x+$w;$xx+=2){
                $px=$origin.X+$xx;$py=$origin.Y+$yy
                if($px -lt 0 -or $py -lt 0 -or $px -ge $bounds.Width -or $py -ge $bounds.Height){throw 'P028d image outside desktop'}
                $p=$a.GetPixel($px,$py);$q=$b.GetPixel($px,$py)
                if(([math]::Abs([int]$p.R-[int]$q.R)+[math]::Abs([int]$p.G-[int]$q.G)+[math]::Abs([int]$p.B-[int]$q.B)) -gt 24){$count++}
            }
        }
        return $count
    }
    $shades=[System.Collections.Generic.HashSet[int]]::new()
    for($y=40;$y -lt 140;$y+=3){for($x=40;$x -lt 150;$x+=3){
        [void]$shades.Add([int]$before.GetPixel($origin.X+$x,$origin.Y+$y).R)
    }}
    if($shades.Count -lt 45){throw 'P028d faceplate raster not visible'}
    ClickClient $handle 707 460
    Start-Sleep -Milliseconds 650
    $after=Screenshot 'P009-Editor-Randomized.png'
    if((Delta $before $after 313 180 140 140) -lt 80){throw 'P028d randomization does not change observation window'}
    ClickClient $handle 210 493
    Start-Sleep -Milliseconds 500
    $bypassed=Screenshot 'P009-Editor-Bypassed.png'
    ClickClient $handle 210 493
    Start-Sleep -Milliseconds 500
    $restored=Screenshot 'P009-Editor-Restored.png'
    if((Delta $after $bypassed 313 180 140 140) -lt 80){throw 'P028d bypass does not change effective observation'}
    if((Delta $after $restored 313 180 140 140) -gt 4){throw 'P028d restored static observation differs'}
    # Motion must visibly move only the aperture, and cycle to another scene.
    ClickClient $handle 302 493
    Start-Sleep -Milliseconds 800
    $motionA=Screenshot 'P009-Editor-Motion-A.png'
    Start-Sleep -Milliseconds 1600
    $motionB=Screenshot 'P009-Editor-Motion-B.png'
    try{
        if((Delta $motionA $motionB 313 180 140 140) -lt 5){throw 'P028h MOTION does not visibly animate the aperture'}
        if((Delta $motionA $motionB 40 40 100 100) -gt 4){throw 'P028h MOTION changes the fixed faceplate'}
    }finally{$motionA.Dispose();$motionB.Dispose()}
    Start-Sleep -Seconds 8
    ClickClient $handle 302 493
    Start-Sleep -Milliseconds 500
    $cycled=Screenshot 'P009-Editor-Cycled-Static.png'
    Start-Sleep -Milliseconds 900
    $held=Screenshot 'P009-Editor-Motion-Off-Hold.png'
    try{
        if((Delta $restored $cycled 313 180 140 140) -lt 80){throw 'P028h timed scene rotation did not change the still image'}
        if((Delta $cycled $held 313 180 140 140) -gt 4){throw 'P028h MOTION off does not hold the selected scene'}
    }finally{$cycled.Dispose();$held.Dispose()}
    function DragClient([IntPtr]$hwnd,[int]$x,[int]$y) {
        $point=[PSYGUI+POINT]::new();$point.X=[int][math]::Round($x*1.25);$point.Y=[int][math]::Round($y*1.25)
        if(-not [PSYGUI]::ClientToScreen($hwnd,[ref]$point)){throw 'ClientToScreen failed'}
        [System.Windows.Forms.Cursor]::Position=[System.Drawing.Point]::new($point.X,$point.Y)
        Start-Sleep -Milliseconds 100
        [PSYGUI]::mouse_event(2,0,0,0,[UIntPtr]::Zero)
        for($i=1;$i -le 8;$i++){
            [System.Windows.Forms.Cursor]::Position=[System.Drawing.Point]::new($point.X,$point.Y-3*$i)
            Start-Sleep -Milliseconds 45
        }
        [PSYGUI]::mouse_event(4,0,0,0,[UIntPtr]::Zero)
        Start-Sleep -Milliseconds 220
    }
    $centers=@(@(385,84),@(482,99),@(548,162),@(562,248),@(537,335),@(478,401),@(384,422),@(292,401),@(233,336),@(211,248),@(227,162),@(289,99),@(104,424))
    for($i=0;$i -lt $centers.Count;$i++){
        $x=$centers[$i][0];$y=$centers[$i][1]
        $old=Screenshot ("P009-Editor-Control-"+$i+"-Before.png")
        try{
            DragClient $handle $x $y
            $new=Screenshot ("P009-Editor-Control-"+$i+"-After.png")
            try{
                if((Delta $old $new ($x-26) ($y-26) 52 52) -lt 8){throw "P028d control $i did not visibly rotate"}
            }finally{$new.Dispose()}
        }finally{$old.Dispose()}
    }
    $proc.Refresh()
    if($proc.HasExited){throw 'P028d EditorHost terminated during interaction'}
    $fixed=[PSYGUI+RECT]::new()
    if(-not [PSYGUI]::GetClientRect($handle,[ref]$fixed)){throw 'P030 fixed client geometry unavailable'}
    $w=$fixed.Right-$fixed.Left;$h=$fixed.Bottom-$fixed.Top
    if($w -ne 960 -or $h -ne 640){throw "P030 expected 960x640 actual $w x $h"}
    ClickClient $handle 450 493
    Start-Sleep -Milliseconds 200
    $afterClick=[PSYGUI+RECT]::new()
    if(-not [PSYGUI]::GetClientRect($handle,[ref]$afterClick)){throw 'P030 client geometry unavailable after click'}
    if(($afterClick.Right-$afterClick.Left) -ne $w -or ($afterClick.Bottom-$afterClick.Top) -ne $h){throw 'P030 fixed editor changed size'}
    $finalShot=Screenshot 'P009-Editor-Fixed.png'
    $finalShot.Dispose()
    Write-Host "P030 actual fixed editor PASS: $w x $h; former zoom area inactive"
    Write-Host 'P030 native faceplate, randomization, bypass/restore, 13 knobs, MOTION and scene change PASS'
} finally {
    if($before){$before.Dispose()}
    if($after){$after.Dispose()}
    if($bypassed){$bypassed.Dispose()}
    if($restored){$restored.Dispose()}
    if($proc -and -not $proc.HasExited){Stop-Process -Id $proc.Id -Force -ErrorAction SilentlyContinue}
}
