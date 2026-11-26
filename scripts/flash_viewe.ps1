param([string]$Port = 'COM10')
$ErrorActionPreference = 'Stop'
$cli = Join-Path $env:LOCALAPPDATA 'Programs\Arduino IDE\resources\app\lib\backend\resources\arduino-cli.exe'
$root = Split-Path $PSScriptRoot -Parent
& $cli upload --fqbn 'esp32:esp32:esp32s3:USBMode=hwcdc,CDCOnBoot=cdc,FlashSize=16M,PartitionScheme=app3M_fat9M_16MB,PSRAM=opi' --port $Port --input-dir (Join-Path $root 'firmware-viewe') (Join-Path $root 'build-viewe\PlaneRadar')
if ($LASTEXITCODE -ne 0) { throw 'Upload failed' }
