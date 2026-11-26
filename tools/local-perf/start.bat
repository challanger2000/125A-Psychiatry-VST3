@echo off
setlocal
cd /d "%~dp0"
echo 125A PSYCHIATRY - lokaler Performance-Test
echo Bitte Studio Pro und andere rechenintensive Programme schliessen.
echo Keine Installation, kein ASIO-Zugriff. Ergebnisse werden gespeichert.
powershell.exe -NoProfile -Command "$ErrorActionPreference='Stop'; $p=Join-Path $PWD '125A-PSYCHIATRY.vst3\Contents\x86_64-win\125A-PSYCHIATRY.vst3'; $dir=Join-Path $PWD ('RESULT-'+(Get-Date -Format 'yyyyMMdd-HHmmss')); New-Item -ItemType Directory -Path $dir | Out-Null; $actual=(Get-FileHash -Algorithm SHA256 -LiteralPath $p).Hash; $expected=(Get-Content -LiteralPath 'module.sha256' -Raw).Trim(); ('Module SHA256: '+$actual) | Out-File (Join-Path $dir 'system.txt'); if($actual -ne $expected){throw 'Plugin SHA256 stimmt nicht'}; Get-CimInstance Win32_Processor | Select-Object Name,NumberOfCores,NumberOfLogicalProcessors,MaxClockSpeed | Format-List | Out-File (Join-Path $dir 'system.txt') -Append; Get-CimInstance Win32_OperatingSystem | Select-Object Caption,Version,BuildNumber | Format-List | Out-File (Join-Path $dir 'system.txt') -Append; powercfg /getactivescheme | Out-File (Join-Path $dir 'system.txt') -Append; & '.\performance.exe' $p (Join-Path $dir 'measurements.csv') 2>&1 | Tee-Object -FilePath (Join-Path $dir 'report.txt'); $code=$LASTEXITCODE; ('Exit code: '+$code) | Tee-Object -FilePath (Join-Path $dir 'exit-code.txt'); Write-Host ('Ergebnisse: '+$dir); exit $code"
set "RESULT=%ERRORLEVEL%"
echo.
echo Exit-Code %RESULT% (0=PASS, 4=Zeitbudget verfehlt, sonst Testfehler)
pause
exit /b %RESULT%
