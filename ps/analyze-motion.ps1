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


# fetch files from remote
###############################################################################
$retVal = 8
[string[]]$filesToFetch = @()

# open new SSH connection
$remoteHost = "pi3.fritz.box"
$user = "pi"
$pass = ConvertTo-SecureString "check0815" -AsPlainText -Force
$cred = New-Object System.Management.Automation.PSCredential ($user, $pass)

if (-not (Test-Connection $remoteHost -Quiet)) {
    Write-Information "cannot reach remote host $remoteHost via network"`
        -InformationAction Continue
    exit -1
}

try {
    $session = New-SSHSession -ComputerName $remoteHost -Credential $cred -ErrorAction Stop
} catch {
    Write-Information "Trying to connect to ${remoteHost}: $_" -InformationAction Continue
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

# copy files from remote to local



# im Input Dir befinden sich alle Verzeichnisse mit den analysierten Videos (*.avi)
# für jeden Tag ein Verzeichnis erstellen und Videos verschieben

    # TODO check if true, return $null if otherwise
    Remove-SSHSession -SessionId $session.SessionId