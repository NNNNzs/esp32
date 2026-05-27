param(
    [string]$Root = "D:\project\esp32\_reference_sources",
    [switch]$Update,
    [switch]$IncludeAiChats
)

$ErrorActionPreference = "Stop"

$repos = @(
    @{
        Name = "waveshare-demo"
        Url  = "https://github.com/waveshareteam/ESP32-S3-Touch-LCD-1.83.git"
        Dir  = "waveshare-demo"
    },
    @{
        Name = "xiaozhi-esp32"
        Url  = "https://github.com/78/xiaozhi-esp32.git"
        Dir  = "xiaozhi-esp32"
    }
)

if ($IncludeAiChats) {
    $repos += @{
        Name = "ESP32-AIChats"
        Url  = "https://github.com/waveshareteam/ESP32-AIChats.git"
        Dir  = "ESP32-AIChats"
    }
}

New-Item -ItemType Directory -Force -Path $Root | Out-Null

foreach ($repo in $repos) {
    $target = Join-Path $Root $repo.Dir

    if (Test-Path $target) {
        if ($Update) {
            Write-Host "Updating $($repo.Name)"
            git -C $target pull --ff-only
        } else {
            Write-Host "Already present: $target"
        }
        continue
    }

    Write-Host "Cloning $($repo.Name) into $target"
    git clone $repo.Url $target
}

Write-Host ""
Write-Host "Done."
Write-Host "Root: $Root"
Write-Host "Repos:"
$repos | ForEach-Object {
    Write-Host "  $($_.Name) -> $($_.Url)"
}
