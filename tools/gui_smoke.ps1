# P008 Win32 GUI smoke probe, using official Steinberg EditorHost.
# Opening a host HWND is not a Studio Pro verification or visual acceptance.
$ErrorActionPreference='Stop'
$build=Join-Path $PSScriptRoot '..\build-vst3'
$hostExe=Get-ChildItem -LiteralPath $build -Recurse -File -Filter 'editorhost.exe' |
    Sort-Object FullName | Select-Object -First 1
$bundle=Get-ChildItem -LiteralPath $build -Recurse -Directory -Filter 'PsychiatryPrototype.vst3' |
    Where-Object { $_.FullName -match '[\\/]+VST3[\\/]+Release[\\/]' } |
    Sort-Object FullName | Select-Object -First 1
if (-not $hostExe) { throw 'P008 GUI smoke: official Steinberg editorhost.exe missing' }
if (-not $bundle) { throw 'P008 GUI smoke: real Windows .vst3 bundle missing' }
Write-Host 'Opening with Steinberg EditorHost:' $bundle.FullName
$proc=$null
try {
    $arguments='--componentHandler "'+$bundle.FullName+'"'
    $proc=Start-Process -FilePath $hostExe.FullName -ArgumentList $arguments -PassThru
    $handle=[IntPtr]::Zero
    for ($i=0; $i -lt 70; ++$i) {
        Start-Sleep -Milliseconds 350
        $proc.Refresh()
        if ($proc.HasExited) {
            throw "P008 GUI smoke: EditorHost exited with code $($proc.ExitCode) before GUI HWND"
        }
        $handle=$proc.MainWindowHandle
        if ($handle -ne [IntPtr]::Zero) { break }
    }
    if ($handle -eq [IntPtr]::Zero) {
        throw 'P008 GUI smoke: no editorhost Win32 window appeared'
    }
    Write-Host "P008 GUI smoke: Win32 HWND=$handle, title='$($proc.MainWindowTitle)'"
    try {
        Add-Type -AssemblyName System.Drawing
        Add-Type -AssemblyName System.Windows.Forms
        $bounds=[System.Windows.Forms.Screen]::PrimaryScreen.Bounds
        if ($bounds.Width -ge 680 -and $bounds.Height -ge 550) {
            New-Item -ItemType Directory -Force -Path 'artifacts' | Out-Null
            $image=[System.Drawing.Bitmap]::new($bounds.Width,$bounds.Height)
            $graphics=[System.Drawing.Graphics]::FromImage($image)
            try {
                $graphics.CopyFromScreen($bounds.Location,[System.Drawing.Point]::Empty,$bounds.Size)
                $image.Save((Join-Path (Resolve-Path 'artifacts').Path 'P008-EditorHost-Windows.png'),
                    [System.Drawing.Imaging.ImageFormat]::Png)
                Write-Host 'P008 GUI smoke: captured desktop image; visual inspection still required'
            } finally {
                $graphics.Dispose()
                $image.Dispose()
            }
        }
    } catch {
        Write-Host "Screenshot unavailable on CI desktop (not a GUI failure): $($_.Exception.Message)"
    }
    Write-Host 'P008 GUI smoke PASS: official EditorHost opened a Win32 editor window'
} finally {
    if ($proc -and -not $proc.HasExited) {
        Stop-Process -Id $proc.Id -Force -ErrorAction SilentlyContinue
    }
}
