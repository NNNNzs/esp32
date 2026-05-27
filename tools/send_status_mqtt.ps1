param(
    [ValidateSet("thinking", "approval", "working", "done", "error", "idle")]
    [string]$Status = "idle",
    [string]$BrokerHost = "",
    [int]$BrokerPort = 0,
    [string]$Topic = "",
    [string]$ClientId = ""
)

$script = Join-Path $PSScriptRoot "send_status_mqtt.py"
$args = @($script, $Status)
if ($BrokerHost) { $args += @("--host", $BrokerHost) }
if ($BrokerPort -gt 0) { $args += @("--port", $BrokerPort) }
if ($Topic) { $args += @("--topic", $Topic) }
if ($ClientId) { $args += @("--client-id", $ClientId) }

py -3 @args
