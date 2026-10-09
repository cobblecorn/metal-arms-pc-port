param(
    [Parameter(Mandatory = $true)][string]$PlanPath
)

$ErrorActionPreference = 'Stop'
$taskRoot = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..')).TrimEnd('\')
$taskBuild = [IO.Path]::GetFullPath((Join-Path $taskRoot 'build')).TrimEnd('\')
$taskPlanAbsolute = (Resolve-Path -LiteralPath $PlanPath).Path
if (-not $taskPlanAbsolute.StartsWith($taskBuild + '\cleanup-records\', [StringComparison]::OrdinalIgnoreCase)) {
    throw 'Cleanup plan must be in build/cleanup-records.'
}
$taskPlan = Get-Content -LiteralPath $taskPlanAbsolute -Raw | ConvertFrom-Json
$taskChecked = [Collections.Generic.List[object]]::new()
$taskSeen = [Collections.Generic.HashSet[string]]::new([StringComparer]::OrdinalIgnoreCase)
$taskCheckedDirectories = [Collections.Generic.HashSet[string]]::new([StringComparer]::OrdinalIgnoreCase)

# Validate every resolved target before the first deletion. Never follow links.
foreach ($taskEntry in $taskPlan.files) {
    if ([IO.Path]::IsPathRooted($taskEntry.path)) { throw 'Plan contains an absolute target.' }
    $taskPath = [IO.Path]::GetFullPath((Join-Path $taskRoot $taskEntry.path))
    if (-not $taskPath.StartsWith($taskBuild + '\', [StringComparison]::OrdinalIgnoreCase)) {
        throw "Target outside build directory: $taskPath"
    }
    if (-not $taskSeen.Add($taskPath)) { throw "Duplicate target: $taskPath" }
    $taskItem = Get-Item -LiteralPath $taskPath -Force
    if ($taskItem.PSIsContainer -or ($taskItem.Attributes -band [IO.FileAttributes]::ReparsePoint)) {
        throw "Target is not a regular file: $taskPath"
    }
    $taskDirectory = $taskItem.Directory
    while ($taskDirectory -and $taskDirectory.FullName.StartsWith($taskBuild, [StringComparison]::OrdinalIgnoreCase)) {
        if (-not $taskCheckedDirectories.Add($taskDirectory.FullName)) { break }
        if ($taskDirectory.Attributes -band [IO.FileAttributes]::ReparsePoint) {
            throw "Target parent is a link: $($taskDirectory.FullName)"
        }
        $taskDirectory = $taskDirectory.Parent
    }
    $taskModified = ([DateTimeOffset]$taskItem.LastWriteTimeUtc).ToUnixTimeMilliseconds() / 1000.0
    if ($taskItem.Length -ne $taskEntry.size -or [Math]::Abs($taskModified - $taskEntry.mtime) -gt 1) {
        throw "Target changed since inventory; refusing deletion: $taskPath"
    }
    $taskChecked.Add([PSCustomObject]@{ path = $taskPath; relative = $taskEntry.path; bytes = $taskItem.Length; reason = $taskEntry.reason })
}
Write-Output "Preflight passed: $($taskChecked.Count) regular files inside $taskBuild."

$taskDeleted = [Collections.Generic.List[object]]::new()
$taskBytes = [long]0
try {
    foreach ($taskTarget in $taskChecked) {
        Remove-Item -LiteralPath $taskTarget.path -Force
        $taskBytes += $taskTarget.bytes
        $taskDeleted.Add([PSCustomObject]@{ path = $taskTarget.relative; bytes = $taskTarget.bytes; reason = $taskTarget.reason })
        if ($taskDeleted.Count % 5000 -eq 0) { Write-Output "Removed $($taskDeleted.Count)/$($taskChecked.Count) files." }
    }
}
finally {
    $taskRecord = [PSCustomObject]@{ plan = $taskPlanAbsolute; files = $taskDeleted; deleted_bytes = $taskBytes; completed_at = [DateTime]::UtcNow.ToString('o') }
    $taskLogPath = [IO.Path]::ChangeExtension($taskPlanAbsolute, '.deleted.json')
    $taskRecord | ConvertTo-Json -Depth 5 | Set-Content -LiteralPath $taskLogPath -Encoding utf8
}
Write-Output ('Removed {0} files, {1:N3} GiB.' -f $taskDeleted.Count, ($taskBytes / 1GB))
