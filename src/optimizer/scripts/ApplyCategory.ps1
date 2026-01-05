# Apply-OptimizationCategory.ps1
# Applies all .reg, .bat, and .cmd files in a specific category folder

param (
    [string]$Category,
    [switch]$Revert
)

$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$CategoryDir = Join-Path $ScriptDir $Category

if (-not (Test-Path $CategoryDir)) {
    Write-Error "Category directory not found: $CategoryDir"
    exit 1
}

Write-Host "Applying optimizations for category: $Category"

# Function to apply .reg files
function Apply-RegFile ($file) {
    Write-Host "Applying registry tweak: $($file.Name)"
    Start-Process -FilePath "reg.exe" -ArgumentList "import `"$($file.FullName)`"" -Wait -NoNewWindow
}

# Function to run .bat/.cmd files
function Run-BatchFile ($file) {
    Write-Host "Running batch script: $($file.Name)"
    Start-Process -FilePath "cmd.exe" -ArgumentList "/c `"$($file.FullName)`"" -Wait -NoNewWindow
}

# Get all files
$regFiles = Get-ChildItem -Path $CategoryDir -Filter "*.reg"
$batFiles = Get-ChildItem -Path $CategoryDir -Include "*.bat", "*.cmd" -Recurse

foreach ($file in $regFiles) {
    Apply-RegFile $file
}

foreach ($file in $batFiles) {
    # Skip revert scripts if we are not reverting
    if ($file.Name -like "*Default*" -or $file.Name -like "*Revert*") {
        if ($Revert) {
            Run-BatchFile $file
        }
    }
    else {
        if (-not $Revert) {
            Run-BatchFile $file
        }
    }
}

Write-Host "Category $Category applied successfully."
