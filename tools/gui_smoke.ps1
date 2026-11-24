# P009: Windows-only runtime GUI probe against Steinberg's official EditorHost.
# This validates an actual HWND, click-to-parameter-driven CRT change, and
# ability to switch the GUI language. Does not use Studio Pro or audio input.
$ErrorActionPreference='Stop'
Add-Type -AssemblyName System.Drawing
Add-Type -AssemblyName System.Windows.Forms
Add-Type -TypeDefinition @'
using System;
using System.Runtime.InteropServices;
public static class PSYGUI {
    [StructLayout(LayoutKind.Sequential)] public struct POINT { public int X,Y; }
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
    $point.X=$x;$point.Y=$y
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
    if($bounds.Width -lt 1024 -or $bounds.Height -lt 650) {
        throw 'P009 GUI test: desktop resolution too small for full GUI screenshot'
    }
    New-Item -ItemType Directory -Force -Path 'artifacts' | Out-Null
    $origin=[PSYGUI+POINT]::new()
    if(-not [PSYGUI]::ClientToScreen($handle,[ref]$origin)) {
        throw 'P009 GUI test: cannot locate editor client origin'
    }
    $before=Screenshot 'P009-Editor-Before.png'

    # Actual press of the LOSS OF CONTROL GUI button.
    ClickClient $handle 510 534
    Start-Sleep -Milliseconds 650
    $proc.Refresh()
    if($proc.HasExited) {throw 'P009 GUI test: host exited after randomization click'}
    $after=Screenshot 'P009-Editor-Randomized.png'

    # Only the CRT panel is compared. Animated-frame jitter with all macros
    # at zero is intentionally limited; a real click must cause a large scene
    # change because the nine macro parameters actually changed.
    $changed=0;$sumDiff=0
    for($y=20;$y -lt 302;$y+=8){
        for($x=20;$x -lt 267;$x+=8){
            $px=$origin.X+685+$x
            $py=$origin.Y+116+$y
            if($px -ge $bounds.Width -or $py -ge $bounds.Height){continue}
            $a=$before.GetPixel($px,$py)
            $b=$after.GetPixel($px,$py)
            $dist=[math]::Abs([int]$a.R-[int]$b.R)+
                  [math]::Abs([int]$a.G-[int]$b.G)+
                  [math]::Abs([int]$a.B-[int]$b.B)
            if($dist -gt 15){$changed++}
            $sumDiff+=$dist
        }
    }
    Write-Host "P009 CRT reaction check: changed pixels=$changed, RGB delta=$sumDiff"
    if($changed -lt 18 -or $sumDiff -lt 1600) {
        throw "P009 GUI test: LOSS OF CONTROL did not produce a visible CRT scene change"
    }
    # Static control-driven monitor: toggle actual VST3 bypass and check
    # the filled intensity bar becomes dark, then returns to its old state.
    ClickClient $handle 435 505
    Start-Sleep -Milliseconds 500
    $bypassed=Screenshot 'P009-Editor-Bypassed.png'
    ClickClient $handle 435 505
    Start-Sleep -Milliseconds 500
    $restored=Screenshot 'P009-Editor-Restored.png'
    $activeSum=0;$bypassSum=0;$restoredSum=0
    for($y=252;$y -le 258;$y+=2){
        for($x=47;$x -le 53;$x+=2){
            $px=$origin.X+685+$x;$py=$origin.Y+116+$y
            $activeSum+=[int]$after.GetPixel($px,$py).R
            $bypassSum+=[int]$bypassed.GetPixel($px,$py).R
            $restoredSum+=[int]$restored.GetPixel($px,$py).R
        }
    }
    Write-Host "P022 CRT active=$activeSum bypass=$bypassSum restored=$restoredSum"
    if($activeSum-$bypassSum -lt 400 -or [math]::Abs($restoredSum-$activeSum) -gt 40){
        throw 'P022 CRT intensity does not follow actual bypass or restore'
    }
    # Click EN -> DE. The editor must survive and repaint both templates.
    ClickClient $handle 600 471
    Start-Sleep -Milliseconds 500
    $proc.Refresh()
    if($proc.HasExited) {throw 'P009 GUI test: host exited after language click'}
    $german=Screenshot 'P009-Editor-German.png'
    Write-Host 'P009 GUI interaction PASS: randomizer changes CRT; language switch survives'
} finally {
    if($before){$before.Dispose()}
    if($after){$after.Dispose()}
    if($german){$german.Dispose()}
    if($bypassed){$bypassed.Dispose()}
    if($restored){$restored.Dispose()}
    if($proc -and -not $proc.HasExited){
        Stop-Process -Id $proc.Id -Force -ErrorAction SilentlyContinue
    }
}
