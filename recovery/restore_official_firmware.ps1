param(
  [string]$Port = $env:ESP32_PORT,
  [int]$BaudRate = 921600,
  [string]$Firmware = "$PSScriptRoot\official-firmware\ESP32-S3-Touch-LCD-1.83-xiaozhi-251024.bin"
)

if ([string]::IsNullOrWhiteSpace($Port)) {
  Write-Error "Missing serial port. Pass -Port COMx or set ESP32_PORT, for example: `$env:ESP32_PORT='COM3'"
  exit 1
}

if (!(Test-Path -LiteralPath $Firmware)) {
  Write-Error "Firmware not found: $Firmware"
  exit 1
}

$esptool = Join-Path $env:LOCALAPPDATA "Arduino15\packages\esp32\tools\esptool_py\5.2.0\esptool.exe"
if (!(Test-Path -LiteralPath $esptool)) {
  $cmd = Get-Command esptool.exe -ErrorAction SilentlyContinue
  if ($cmd) {
    $esptool = $cmd.Source
  }
}

if (!(Test-Path -LiteralPath $esptool)) {
  Write-Error "esptool.exe not found. Install the ESP32 board package in Arduino IDE first."
  exit 1
}

Write-Host "Restoring official firmware to $Port"
Write-Host "Firmware: $Firmware"
Write-Host "Address: 0x0"

& $esptool --chip esp32s3 --port $Port --baud $BaudRate write-flash -z 0x0 $Firmware
