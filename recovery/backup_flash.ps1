param(
  [string]$Port = $env:ESP32_PORT,
  [int]$BaudRate = 921600,
  [string]$OutDir = "$PSScriptRoot\backups"
)

if ([string]::IsNullOrWhiteSpace($Port)) {
  Write-Error "Missing serial port. Pass -Port COMx or set ESP32_PORT, for example: `$env:ESP32_PORT='COM3'"
  exit 1
}

New-Item -ItemType Directory -Force -Path $OutDir | Out-Null

$stamp = Get-Date -Format "yyyyMMdd-HHmmss"
$outFile = Join-Path $OutDir "esp32s3-flash-$stamp.bin"
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

Write-Host "Backing up full 16MB flash from $Port"
Write-Host "Output: $outFile"

& $esptool --chip esp32s3 --port $Port --baud $BaudRate read-flash 0x0 0x1000000 $outFile

if (Test-Path -LiteralPath $outFile) {
  Get-FileHash -Algorithm SHA256 -LiteralPath $outFile
}
