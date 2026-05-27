param(
  [Parameter(Mandatory = $true)]
  [ValidateSet("idle", "thinking", "approval", "working", "done", "error")]
  [string]$Status,

  [string]$Port = $env:ESP32_PORT,
  [int]$BaudRate = 115200
)

$script = Join-Path $PSScriptRoot "send_status.py"
$args = @($script, $Status)
if ($Port) { $args += @("--port", $Port) }
if ($BaudRate) { $args += @("--baud", $BaudRate) }

py -3 @args
