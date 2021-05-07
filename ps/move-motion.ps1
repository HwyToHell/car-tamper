param ([string]$dateString)
# move motion files to date directory
$localPath = "D:\Carla\Videos\Skoda"
$localPathInput = Join-Path $localPath input

function getDateByString {
    param ([string]$dateString)

    $date = $null
    try {
        $date = Get-Date -Date $dateString 
    }
    catch [System.Management.Automation.ParameterBindingException] {   
        Write-Information "parameter $dateString does not match required format YYYY-MM-DD"`
            -InformationAction Continue
    }
    return $date
}


# validation
$date = getDateByString $dateString
if (-not $date) {
    "date $dateString not valid"
        exit -1
}


# create date directory
$oldPath = Get-Location
Set-Location $localPathInput
$dateString = Get-Date -Date $date -Format yyyy-MM-dd
$datePath = Join-Path $localPath $dateString
$newDir = New-Item -Path $localPath -Name $dateString -ItemType Directory -Force

# move motion files
$dayDirs = Get-ChildItem -Directory -Filter $dateString*
foreach ($dir in $dayDirs) {
    Move-Item -Path $dir\*.avi -Destination $datePath -Force
    Remove-Item $dir
}  
    
# delete long video files
$videoFiles = Get-ChildItem -Filter $dateString*.mp4   
foreach ($file in $videoFiles) {
    Remove-Item $file
} 

Write-Information "Motion video files moved to directory $($newDir.Name)" -InformationAction Continue

Set-Location $oldPath
