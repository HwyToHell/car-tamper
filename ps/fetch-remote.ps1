
function fetchFilesforDate {
    param([int]$sessionID, [datetime]$dateToFetch)

    $remotePath = "/home/pi/Videos"

    # convert date to string
    $dateString = Get-Date -Date $dateToFetch -Format yyyy-MM-dd
    
    $allFiles = Invoke-SSHCommand -Command "ls $remotePath" -SessionId $sessionID
    [string[]]$filesSelected = @() 
    foreach ($file in $allFiles.Output) {
        if ($file.StartsWith($dateString)) {
            $filesSelected += $file
        }
    } 
   
    return $filesSelected
}

$firstOfMay = Get-Date -Date 2021-04-27
fetchFilesforDate 0 $firstOfMay
