$ErrorActionPreference = 'Stop'
try {
  $root = $PSScriptRoot
  $cli = Join-Path $env:LOCALAPPDATA 'Programs\Arduino IDE\resources\app\lib\backend\resources\arduino-cli.exe'
  if (!(Test-Path -LiteralPath $cli)) { throw 'Arduino CLI not found. Install Arduino IDE and the ESP32 board package.' }
  $toolRoot = Join-Path $env:LOCALAPPDATA 'Arduino15\packages\esp32\tools\esptool_py'
  $esptool = Get-ChildItem -LiteralPath $toolRoot -Recurse -Filter esptool.exe | Sort-Object FullName -Descending | Select-Object -First 1
  if (!$esptool) { throw 'esptool not found. Install the ESP32 board package.' }
  if (!(Test-Path -LiteralPath (Join-Path $root 'firmware-viewe\PlaneRadar.ino.bin'))) { throw 'Firmware missing. Build with scripts/build_viewe.py first.' }
  $ports = @(Get-CimInstance Win32_SerialPort | Where-Object { $_.PNPDeviceID -match 'USB\\VID_303A&PID_1001' })
  if ($ports.Count -eq 0) { throw 'No Espressif USB device found. Connect the board with a data cable. If needed, hold BOOT, tap RESET, release BOOT, then retry.' }
  if ($ports.Count -eq 1) { $port = $ports[0].DeviceID } else {
    $ports | Format-Table DeviceID,Name
    $port = Read-Host 'Several Espressif boards found. Enter the COM port of the VIEWE board'
    if ($port -notin $ports.DeviceID) { throw 'Port not in detected device list.' }
  }
  Write-Host "Checking $port..."
  $identity = & $esptool.FullName --port $port chip-id 2>&1
  $code = $LASTEXITCODE
  $identity | ForEach-Object { Write-Host $_ }
  if ($code -ne 0 -or ($identity -join "`n") -notmatch 'Chip type:\s+ESP32-S3') { throw 'ESP32-S3 verification failed. Nothing flashed.' }
  Write-Host 'Flashing VIEWE 1.75-inch Plane Radar...'
  & $cli upload --fqbn 'esp32:esp32:esp32s3:USBMode=hwcdc,CDCOnBoot=cdc,FlashSize=16M,PartitionScheme=app3M_fat9M_16MB,PSRAM=opi' --port $port --input-dir (Join-Path $root 'firmware-viewe')
  if ($LASTEXITCODE -ne 0) { throw 'Upload failed.' }
  Write-Host 'Flash verified. Join PlaneRadar-Setup and open http://192.168.4.1 to configure Wi-Fi.' -ForegroundColor Green
} catch { Write-Host $_ -ForegroundColor Red; exit 1 }
