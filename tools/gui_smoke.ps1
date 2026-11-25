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
    [StructLayout(LayoutKind.Sequential)] public struct RECT { public int Left,Top,Right,Bottom; }
    [StructLayout(LayoutKind.Sequential)] public struct MENUBARINFO {
        public uint cbSize; public RECT rcBar; public IntPtr hMenu, hwndMenu; public uint flags;
    }
    [DllImport("user32.dll", CharSet=CharSet.Unicode)] static extern IntPtr FindWindowEx(IntPtr parent,IntPtr after,string cls,string title);
    [DllImport("user32.dll")] static extern bool IsWindowVisible(IntPtr hwnd);
    [DllImport("user32.dll")] static extern uint GetWindowThreadProcessId(IntPtr hwnd,out uint pid);
    [DllImport("user32.dll")] static extern bool GetMenuBarInfo(IntPtr hwnd,int obj,int item,ref MENUBARINFO info);
    [DllImport("user32.dll")] static extern int GetMenuItemCount(IntPtr menu);
    [DllImport("user32.dll",CharSet=CharSet.Unicode)] static extern int GetMenuString(IntPtr menu,uint item,System.Text.StringBuilder text,int max,uint flags);
    [DllImport("user32.dll")] static extern bool GetMenuItemRect(IntPtr hwnd,IntPtr menu,uint item,out RECT rect);
    // Find only the visible native popup owned by our EditorHost process.
    // Read actual menu labels and item rectangles; no blind keyboard selection.
    public static bool FindMenuItem(uint expectedPid,string label,out RECT rect) {
        rect=new RECT();
        IntPtr popup=IntPtr.Zero;
        while((popup=FindWindowEx(IntPtr.Zero,popup,"#32768",null))!=IntPtr.Zero) {
            uint pid; GetWindowThreadProcessId(popup,out pid);
            if(pid!=expectedPid || !IsWindowVisible(popup)) continue;
            var info=new MENUBARINFO(); info.cbSize=(uint)Marshal.SizeOf(typeof(MENUBARINFO));
            if(!GetMenuBarInfo(popup,-4,0,ref info) || info.hMenu==IntPtr.Zero) continue;
            int count=GetMenuItemCount(info.hMenu);
            if(count!=6) continue;
            for(uint i=0;i<(uint)count;i++) {
                var name=new System.Text.StringBuilder(128);
                GetMenuString(info.hMenu,i,name,name.Capacity,0x400);
                if(name.ToString().Replace("&","").Trim()!=label) continue;
                if(GetMenuItemRect(IntPtr.Zero,info.hMenu,i,out rect) && rect.Right>rect.Left && rect.Bottom>rect.Top)
                    return true;
                var item=new MENUBARINFO(); item.cbSize=(uint)Marshal.SizeOf(typeof(MENUBARINFO));
                if(GetMenuBarInfo(popup,-4,(int)i+1,ref item)) {
                    rect=item.rcBar;
                    return rect.Right>rect.Left && rect.Bottom>rect.Top;
                }
            }
        }
        return false;
    }
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
    $work=[System.Windows.Forms.Screen]::PrimaryScreen.WorkingArea
    Write-Host "P028j runtime desktop $($bounds.Width)x$($bounds.Height), work area $($work.Width)x$($work.Height)"
    if($work.Width -lt 1200 -or $work.Height -lt 840){throw 'P028j insufficient native desktop capacity for 75% zoom'}
    if(-not [PSYGUI]::SetWindowPos($handle,[IntPtr]::Zero,($work.X+10),($work.Y+10),0,0,0x15)){throw 'P028j could not position actual EditorHost window'}
    Start-Sleep -Milliseconds 300
    New-Item -ItemType Directory -Force -Path 'artifacts' | Out-Null
    $origin=[PSYGUI+POINT]::new()
    if(-not [PSYGUI]::ClientToScreen($handle,[ref]$origin)) {
        throw 'P009 GUI test: cannot locate editor client origin'
    }
    $before=Screenshot 'P009-Editor-Before.png'
    # P028d default zoom 50% -> 768x512 logical faceplate. Measure exact
    # on-screen regions, not the old rectangular CRT/language coordinates.
    function Delta($a,$b,[int]$x,[int]$y,[int]$w,[int]$h) {
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
        $point=[PSYGUI+POINT]::new();$point.X=$x;$point.Y=$y
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
    # Test actual native zoom menu selection with a desktop that fits 1152x768.
    # Keep the exact HWND geometry gate; a clamped or cropped window is not a pass.
    $rect0=[PSYGUI+RECT]::new()
    if(-not [PSYGUI]::GetClientRect($handle,[ref]$rect0)){throw 'P028h initial client geometry unavailable'}
    ClickClient $handle 450 493
    $item=[PSYGUI+RECT]::new()
    $menuFound=$false
    for($i=0;$i -lt 20;++$i){
        Start-Sleep -Milliseconds 100
        if([PSYGUI]::FindMenuItem([uint32]$proc.Id,'75%',[ref]$item)){$menuFound=$true;break}
    }
    $popupShot=Screenshot 'P009-Editor-Zoom-Menu.png'
    $popupShot.Dispose()
    if(-not $menuFound){throw 'P028i native six-entry zoom popup or its 75% item was not observed'}
    Write-Host "P028i native 75% menu item bounds: $($item.Left),$($item.Top) -> $($item.Right),$($item.Bottom)"
    [System.Windows.Forms.Cursor]::Position=[System.Drawing.Point]::new([int](($item.Left+$item.Right)/2),[int](($item.Top+$item.Bottom)/2))
    Start-Sleep -Milliseconds 150
    [PSYGUI]::mouse_event(2,0,0,0,[UIntPtr]::Zero)
    Start-Sleep -Milliseconds 100
    [PSYGUI]::mouse_event(4,0,0,0,[UIntPtr]::Zero)
    $zoomOk=$false
    for($i=0;$i -lt 30;++$i){
        Start-Sleep -Milliseconds 100
        $rect1=[PSYGUI+RECT]::new()
        if([PSYGUI]::GetClientRect($handle,[ref]$rect1)){
            $w0=$rect0.Right-$rect0.Left;$h0=$rect0.Bottom-$rect0.Top
            $w1=$rect1.Right-$rect1.Left;$h1=$rect1.Bottom-$rect1.Top
            if([math]::Abs($w1-1.5*$w0) -le 4 -and [math]::Abs($h1-1.5*$h0) -le 4){$zoomOk=$true;break}
        }
    }
    $zoomShot=Screenshot 'P009-Editor-Zoom-After.png'
    $zoomShot.Dispose()
    if(-not $zoomOk){throw "P028i zoom 50 to 75 percent did not resize the host client: $w0 x $h0 -> $w1 x $h1"}
    $zoomOrigin=[PSYGUI+POINT]::new()
    if(-not [PSYGUI]::ClientToScreen($handle,[ref]$zoomOrigin)){throw 'P028j zoomed client origin unavailable'}
    if($zoomOrigin.X -lt $work.Left -or $zoomOrigin.Y -lt $work.Top -or ($zoomOrigin.X+$w1) -gt $work.Right -or ($zoomOrigin.Y+$h1) -gt $work.Bottom){throw 'P028j zoomed client not fully visible'}
    Write-Host "P028j actual zoom menu resize PASS: $w0 x $h0 -> $w1 x $h1"
    Write-Host 'P028h native faceplate, randomization, bypass/restore, 13 knobs, MOTION, timed scene change and zoom PASS'
} finally {
    if($before){$before.Dispose()}
    if($after){$after.Dispose()}
    if($bypassed){$bypassed.Dispose()}
    if($restored){$restored.Dispose()}
    if($proc -and -not $proc.HasExited){Stop-Process -Id $proc.Id -Force -ErrorAction SilentlyContinue}
}
