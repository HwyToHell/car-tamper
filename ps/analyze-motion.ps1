param ([switch]$a, [switch]$d, [switch]$h)


# show help if no arguments provided
###############################################################################
$retVal = 10
if ($h -or ($args.Count -eq 0)) {
    "usage: analyze-motion [option]... [day-before-today]..."
    " "
    "analyze video files for recent days (list with single digit values)"
    "by fetching them from remote via ssh, analyzing and storing motion sequences locally"
    " "
    "options:"
    "   -a fetch all available videos"
    "   -d specify dates in format YYYY-MM-DD"
    "   -h help"
    exit $retVal
}


# validate args
###############################################################################
$retVal = 9
[datetime[]]$datesToAnalyze = @()

function getDateByInt {
    param ([int]$daysBefore)

    if ($daysBefore -lt 1 -or $daysBefore -gt 10) {
        Write-Information "parameter $daysBefore out of range 1 .. 9"`
            -InformationAction Continue
        return $null
    }
    $todayString = Get-Date -Format yyyy-MM-dd
    $todayDate = Get-Date -Date $todayString
    return $todayDate.AddDays(-$daysBefore)
}

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

for ($i=0; $i -lt $args.count; $i++) {
    $day = switch ($args[$i].GetType())
    {
        int { getDateByInt $args[$i] }
        string { getDateByString $args[$i] }
        default { "parameters must be of int or string data type" }
    }
    if (-not $day) {
        "parameter $($args[$i]) not valid"
         exit $retVal
    }
    $datesToAnalyze += $day
}

Write-Information "Dates to analyze"`
    -InformationAction Continue
foreach ($date in $datesToAnalyze) {
    Write-Information "$(Get-Date -Date $date -Format yyyy-MM-dd)" -InformationAction Continue
}



# fetch files from remote
###############################################################################
$retVal = 8
[string[]]$filesToFetch = @()

# open new SSH connection
$remoteHost = "pi3.fritz.box"
$user = "pi"
$pass = ConvertTo-SecureString "check0815" -AsPlainText -Force
$cred = New-Object System.Management.Automation.PSCredential ($user, $pass)

Write-Information "Connecting to $remoteHost"`
    -InformationAction Continue
if (Test-Connection $remoteHost -Quiet) {
    Write-Information "Remote host is alive"`
        -InformationAction Continue
} else {
    Write-Information "Cannot reach remote host via network"`
        -InformationAction Continue
    exit $retVal
}

try {
    $session = New-SSHSession -ComputerName $remoteHost -Credential $cred -ErrorAction Stop
    Write-Information "Opened SSH connection to ${remoteHost} with session ID: $($session.SessionID)"`
        -InformationAction Continue
} catch {
    Write-Information "Connection error at ${remoteHost}: $_" -InformationAction Continue
    exit $retVal
}

function getFileNamesForDate {
    param([datetime]$dateToFetch, [int]$sessionID, [string]$remotePath)

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

# file locations
$remotePath = "/home/pi/Videos"
$localPath = "C:\Users\holge\Videos"
$localPathInput = Join-Path $localPath Input

# populate array for files to fetch
foreach ($date in $datesToAnalyze) {
    $filesToFetch += getFileNamesForDate $date $session.SessionId $remotePath
}

# close SSH connection and check exit state
if (Remove-SSHSession -SessionId $session.SessionId) {
    Write-Information "Closed SSH connection with session ID $($session.SessionId)"`
        -InformationAction Continue
} else {
    Write-Information "Not able to close session ID $($session.SessionId)"`
        -InformationAction Continue
}

# check existence of local path and create, if necessary
if (-not (Test-Path -Path $localPathInput)) {
    New-Item -ItemType Directory -Force -Path $localPathInput
}    

# copy files from remote to local
Write-Information "Downloading files" -InformationAction Continue
$nAttempts = 3
foreach ($file in $filesToFetch) {
    $remoteFilePath = $remotePath + "/" + $file
    $completed = $false
    for ($i = 1; $i -le $nAttempts; $i++) {
        try {
            Get-SCPItem -ComputerName $remoteHost -Credential $cred -Path $remoteFilePath -PathType File -Destination $localPathInput -ErrorAction Stop
            $completed = $true
            Write-Information "Successful: $file" -InformationAction Continue
            break
        } catch {
            Write-Information "Attempt $i error when downloading ${file}: $_" -InformationAction Continue
            $completed = $false
        }
    }
    if (-not $completed) {
        Write-Information "Exit after $nAttempts attempts" -InformationAction Continue
        exit $retVal
    }     
}


# analyze local files
###############################################################################
$retVal = 7
Set-Location $localPathInput
Write-Information "Analyzing motion" -InformationAction Continue
Start-Process .\tamper -ArgumentList "."
Write-Information "Done" -InformationAction Continue

# move motion files to date directory
###############################################################################
foreach ($date in $datesToAnalyze) {
    $dateDir = Get-Date -Date $date -Format yyyy-MM-dd
    $datePath = Join-Path $localPath $dateDir
    New-Item -Path $localPath -Name $dateDir -ItemType Directory -Force
    #TODO
    # alle Verzeichnisse des aktuellen Datums in Array lesen
    # für jeden Tag ein Verzeichnis erstellen und Videos verschieben
}