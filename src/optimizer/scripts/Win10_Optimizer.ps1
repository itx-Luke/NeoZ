# Win10 Ultimate Optimizer - Advanced GUI Interface
# Save as: Win10_Optimizer_Advanced_GUI.ps1

Add-Type -AssemblyName System.Windows.Forms
Add-Type -AssemblyName System.Drawing

# Check for Administrator privileges
if (-not ([Security.Principal.WindowsPrincipal] [Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole([Security.Principal.WindowsBuiltinRole]::Administrator)) {
    $result = [System.Windows.Forms.MessageBox]::Show(
        "This tool requires Administrator privileges. Run as Administrator?",
        "Administrator Required",
        [System.Windows.Forms.MessageBoxButtons]::YesNo,
        [System.Windows.Forms.MessageBoxIcon]::Warning
    )
    if ($result -eq [System.Windows.Forms.DialogResult]::Yes) {
        Start-Process powershell -Verb RunAs -ArgumentList "-File `"$PSCommandPath`""
    }
    exit
}

# Global Variables
$script:SelectedOptimizations = @{}
$script:Logs = New-Object System.Collections.ArrayList
$script:BackupCreated = $false
$script:SystemInfo = @{}

# Initialize System Info
function Initialize-SystemInfo {
    $script:SystemInfo = @{
        CPU = (Get-WmiObject Win32_Processor).Name
        RAM = [math]::Round((Get-WmiObject Win32_ComputerSystem).TotalPhysicalMemory / 1GB, 2)
        OS = (Get-WmiObject Win32_OperatingSystem).Caption
        DiskFree = [math]::Round((Get-PSDrive -Name C).Free / 1GB, 2)
        DiskTotal = [math]::Round((Get-PSDrive -Name C).Used / 1GB + (Get-PSDrive -Name C).Free / 1GB, 2)
    }
}

# Logging Function
function Write-Log {
    param([string]$Message, [string]$Type = "INFO")
    $timestamp = Get-Date -Format "HH:mm:ss"
    $logEntry = "[$timestamp] [$Type] $Message"
    $script:Logs.Add($logEntry) | Out-Null
    
    # Update log display if form exists
    if ($script:txtLogs -and $script:txtLogs.IsHandleCreated) {
        $script:txtLogs.Invoke([Action]{
            $script:txtLogs.AppendText("$logEntry`r`n")
            $script:txtLogs.ScrollToCaret()
        })
    }
}

# ToolTip Manager
function Initialize-ToolTips {
    $toolTip = New-Object System.Windows.Forms.ToolTip
    $toolTip.AutoPopDelay = 10000
    $toolTip.InitialDelay = 500
    $toolTip.ReshowDelay = 500
    $toolTip.ShowAlways = $true
    return $toolTip
}

# Create System Restore Point
function Create-SystemRestorePoint {
    try {
        Write-Log "Creating system restore point..." "BACKUP"
        checkpoint-Computer -Description "Win10 Optimizer - Pre-Optimization" -RestorePointType MODIFY_SETTINGS
        Write-Log "System restore point created successfully" "SUCCESS"
        return $true
    }
    catch {
        Write-Log "Failed to create system restore point: $_" "ERROR"
        return $false
    }
}

# ==================== OPTIMIZATION FUNCTIONS ====================

# Performance Optimizations
function Apply-PowerPlanOptimization {
    Write-Log "Setting High Performance Power Plan..." "PERFORMANCE"
    powercfg -setactive 8c5e7fda-e8bf-4a96-9a85-a6e23a8c635c
}

function Disable-VisualEffects {
    Write-Log "Disabling visual effects..." "PERFORMANCE"
    reg add "HKCU\Software\Microsoft\Windows\CurrentVersion\Explorer\VisualEffects" /v VisualFXSetting /t REG_DWORD /d 2 /f
    reg add "HKCU\Control Panel\Desktop" /v UserPreferencesMask /t REG_BINARY /d 9012078010000000 /f
}

function Disable-SearchIndexing {
    Write-Log "Disabling Windows Search Indexing..." "PERFORMANCE"
    Stop-Service WSearch -Force -ErrorAction SilentlyContinue
    Set-Service WSearch -StartupType Disabled
}

function Disable-Superfetch {
    Write-Log "Disabling Superfetch/SysMain..." "PERFORMANCE"
    Stop-Service SysMain -Force -ErrorAction SilentlyContinue
    Set-Service SysMain -StartupType Disabled
}

function Disable-Telemetry {
    Write-Log "Disabling Telemetry..." "PRIVACY"
    reg add "HKLM\SOFTWARE\Policies\Microsoft\Windows\DataCollection" /v AllowTelemetry /t REG_DWORD /d 0 /f
    reg add "HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\Policies\DataCollection" /v AllowTelemetry /t REG_DWORD /d 0 /f
    Stop-Service DiagTrack -Force -ErrorAction SilentlyContinue
    Set-Service DiagTrack -StartupType Disabled
}

function Disable-GameBar {
    Write-Log "Disabling Xbox Game Bar & DVR..." "GAMING"
    reg add "HKCU\System\GameConfigStore" /v GameDVR_Enabled /t REG_DWORD /d 0 /f
    reg add "HKLM\SOFTWARE\Policies\Microsoft\Windows\GameDVR" /v AllowGameDVR /t REG_DWORD /d 0 /f
    reg add "HKCU\SOFTWARE\Microsoft\Windows\CurrentVersion\GameDVR" /v AppCaptureEnabled /t REG_DWORD /d 0 /f
}

function Disable-BackgroundApps {
    Write-Log "Disabling Background Apps..." "PERFORMANCE"
    reg add "HKCU\Software\Microsoft\Windows\CurrentVersion\BackgroundAccessApplications" /v GlobalUserDisabled /t REG_DWORD /d 1 /f
}

function Optimize-TCP {
    Write-Log "Optimizing TCP/IP Settings..." "NETWORK"
    netsh int tcp set global autotuninglevel=normal
    netsh int tcp set global chimney=disabled
    netsh int tcp set global rss=enabled
    netsh int tcp set global netdma=disabled
    reg add "HKLM\SYSTEM\CurrentControlSet\Services\Tcpip\Parameters" /v TcpNoDelay /t REG_DWORD /d 1 /f
    reg add "HKLM\SYSTEM\CurrentControlSet\Services\Tcpip\Parameters" /v TcpAckFrequency /t REG_DWORD /d 1 /f
}

function Disable-WindowsTips {
    Write-Log "Disabling Windows Tips & Ads..." "PRIVACY"
    reg add "HKCU\Software\Microsoft\Windows\CurrentVersion\ContentDeliveryManager" /v SubscribedContent-338393Enabled /t REG_DWORD /d 0 /f
    reg add "HKCU\Software\Microsoft\Windows\CurrentVersion\ContentDeliveryManager" /v SubscribedContent-353694Enabled /t REG_DWORD /d 0 /f
    reg add "HKCU\Software\Microsoft\Windows\CurrentVersion\ContentDeliveryManager" /v SubscribedContent-353696Enabled /t REG_DWORD /d 0 /f
}

function Disable-OneDriveAutoStart {
    Write-Log "Disabling OneDrive Auto-Start..." "PERFORMANCE"
    reg delete "HKCU\Software\Microsoft\Windows\CurrentVersion\Run" /v OneDrive /f 2>$null
}

# Privacy Optimizations
function Disable-Cortana {
    Write-Log "Disabling Cortana..." "PRIVACY"
    reg add "HKLM\SOFTWARE\Policies\Microsoft\Windows\Windows Search" /v AllowCortana /t REG_DWORD /d 0 /f
    reg add "HKLM\SOFTWARE\Policies\Microsoft\Windows\Windows Search" /v AllowCortanaAboveLock /t REG_DWORD /d 0 /f
    reg add "HKLM\SOFTWARE\Policies\Microsoft\Windows\Windows Search" /v AllowSearchToUseLocation /t REG_DWORD /d 0 /f
}

function Disable-LocationTracking {
    Write-Log "Disabling Location Tracking..." "PRIVACY"
    reg add "HKLM\SOFTWARE\Policies\Microsoft\Windows\LocationAndSensors" /v DisableLocation /t REG_DWORD /d 1 /f
    reg add "HKLM\SOFTWARE\Policies\Microsoft\Windows\LocationAndSensors" /v DisableWindowsLocationProvider /t REG_DWORD /d 1 /f
}

function Disable-AdvertisingID {
    Write-Log "Blocking Advertising ID..." "PRIVACY"
    reg add "HKCU\Software\Microsoft\Windows\CurrentVersion\AdvertisingInfo" /v Enabled /t REG_DWORD /d 0 /f
}

function Disable-WiFiSense {
    Write-Log "Disabling WiFi Sense..." "PRIVACY"
    reg add "HKLM\SOFTWARE\Microsoft\WcmSvc\wifinetworkmanager\config" /v AutoConnectAllowedOEM /t REG_DWORD /d 0 /f
}

function Clear-ActivityHistory {
    Write-Log "Clearing Activity History..." "PRIVACY"
    reg add "HKLM\SOFTWARE\Policies\Microsoft\Windows\System" /v EnableActivityFeed /t REG_DWORD /d 0 /f
    reg add "HKLM\SOFTWARE\Policies\Microsoft\Windows\System" /v PublishUserActivities /t REG_DWORD /d 0 /f
    reg add "HKLM\SOFTWARE\Policies\Microsoft\Windows\System" /v UploadUserActivities /t REG_DWORD /d 0 /f
}

# Cleanup Functions
function Clean-TempFiles {
    Write-Log "Cleaning Temporary Files..." "CLEANUP"
    Remove-Item "$env:TEMP\*" -Recurse -Force -ErrorAction SilentlyContinue
    Remove-Item "C:\Windows\Temp\*" -Recurse -Force -ErrorAction SilentlyContinue
}

function Clean-Prefetch {
    Write-Log "Cleaning Prefetch Data..." "CLEANUP"
    Remove-Item "C:\Windows\Prefetch\*" -Force -ErrorAction SilentlyContinue
}

function Clean-WindowsUpdateCache {
    Write-Log "Clearing Windows Update Cache..." "CLEANUP"
    Stop-Service wuauserv -Force -ErrorAction SilentlyContinue
    Remove-Item "C:\Windows\SoftwareDistribution\Download\*" -Recurse -Force -ErrorAction SilentlyContinue
    Start-Service wuauserv -ErrorAction SilentlyContinue
}

function Clean-ThumbnailCache {
    Write-Log "Clearing Thumbnail Cache..." "CLEANUP"
    Remove-Item "$env:LOCALAPPDATA\Microsoft\Windows\Explorer\thumbcache_*.db" -Force -ErrorAction SilentlyContinue
}

function Remove-MemoryDumps {
    Write-Log "Removing Memory Dumps..." "CLEANUP"
    Remove-Item "C:\Windows\Minidump\*" -Force -ErrorAction SilentlyContinue
    Remove-Item "C:\Windows\MEMORY.DMP" -Force -ErrorAction SilentlyContinue
}

# Gaming Optimizations
function Enable-GameMode {
    Write-Log "Enabling Game Mode..." "GAMING"
    reg add "HKCU\Software\Microsoft\GameBar" /v AllowAutoGameMode /t REG_DWORD /d 1 /f
    reg add "HKCU\Software\Microsoft\GameBar" /v AutoGameModeEnabled /t REG_DWORD /d 1 /f
}

function Disable-FullscreenOptimizations {
    Write-Log "Disabling Fullscreen Optimizations..." "GAMING"
    reg add "HKCU\System\GameConfigStore" /v GameDVR_FSEBehaviorMode /t REG_DWORD /d 2 /f
    reg add "HKCU\System\GameConfigStore" /v GameDVR_HonorUserFSEBehaviorMode /t REG_DWORD /d 1 /f
    reg add "HKCU\System\GameConfigStore" /v GameDVR_FSEBehavior /t REG_DWORD /d 2 /f
}

function Set-GPUHighPerformance {
    Write-Log "Setting GPU to High Performance..." "GAMING"
    # Force discrete GPU usage
    reg add "HKCU\Software\Microsoft\DirectX\UserGpuPreferences" /v "*" /t REG_SZ /d "GpuPreference=2;" /f
}

function Optimize-MMCSS {
    Write-Log "Optimizing MMCSS for Gaming..." "GAMING"
    reg add "HKLM\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Multimedia\SystemProfile" /v SystemResponsiveness /t REG_DWORD /d 0 /f
    reg add "HKLM\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Multimedia\SystemProfile\Tasks\Games" /v "GPU Priority" /t REG_DWORD /d 8 /f
    reg add "HKLM\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Multimedia\SystemProfile\Tasks\Games" /v Priority /t REG_DWORD /d 6 /f
    reg add "HKLM\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Multimedia\SystemProfile\Tasks\Games" /v "Scheduling Category" /t REG_SZ /d "High" /f
}

function Disable-PowerThrottling {
    Write-Log "Disabling Power Throttling in Games..." "GAMING"
    reg add "HKLM\SYSTEM\CurrentControlSet\Control\Power\PowerThrottling" /v PowerThrottlingOff /t REG_DWORD /d 1 /f
}

# Network Optimizations
function Set-FastDNS {
    Write-Log "Setting Fast DNS Servers (Cloudflare + Google)..." "NETWORK"
    $adapters = Get-NetAdapter | Where-Object {$_.Status -eq 'Up'}
    foreach ($adapter in $adapters) {
        Set-DnsClientServerAddress -InterfaceIndex $adapter.ifIndex -ServerAddresses ("1.1.1.1", "8.8.8.8")
    }
}

function Disable-QoSPacketScheduler {
    Write-Log "Disabling QoS Packet Scheduler..." "NETWORK"
    reg add "HKLM\SOFTWARE\Policies\Microsoft\Windows\Psched" /v NonBestEffortLimit /t REG_DWORD /d 0 /f
}

function Enable-LowLatencyMode {
    Write-Log "Enabling Low Latency Mode..." "NETWORK"
    reg add "HKLM\SYSTEM\CurrentControlSet\Services\Tcpip\Parameters" /v TCPNoDelay /t REG_DWORD /d 1 /f
    reg add "HKLM\SYSTEM\CurrentControlSet\Services\Tcpip\Parameters" /v TcpAckFrequency /t REG_DWORD /d 1 /f
    reg add "HKLM\SYSTEM\CurrentControlSet\Services\Tcpip\Parameters" /v TcpDelAckTicks /t REG_DWORD /d 0 /f
}

# Export for NeoZ Integration
Write-Log "Win10 Optimizer Script Loaded" "INFO"
Write-Log "Ready for NeoZ Optimizer Backend integration" "INFO"
