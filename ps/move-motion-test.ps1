[datetime[]]$datesToAnalyze = @()
$datesToAnalyze += Get-Date -Date 2021-04-18
$datesToAnalyze += Get-Date -Date 2021-04-25
$datesToAnalyze += Get-Date -Date 2021-04-26
$localPath = "C:\Users\holge\Videos"
$localPathInput = Join-Path $localPath Input


# move motion files to date directory
###############################################################################
$oldPath = Get-Location
Set-Location $localPathInput
foreach ($date in $datesToAnalyze) {
    # create date directory
    $dateString = Get-Date -Date $date -Format yyyy-MM-dd
    $datePath = Join-Path $localPath $dateString
    New-Item -Path $localPath -Name $dateString -ItemType Directory -Force

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
}
Set-Location $oldPath