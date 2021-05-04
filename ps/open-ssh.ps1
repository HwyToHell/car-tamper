$retVal = 10

$remoteHost = "pi3.fritz.box"
$user = "pi"
$pass = ConvertTo-SecureString "check0815" -AsPlainText -Force
$cred = New-Object System.Management.Automation.PSCredential ($user, $pass)

if (-not (Test-Connection $remoteHost -Quiet)) {
    Write-Information "cannot reach remote host $remoteHost via network"`
        -InformationAction Continue
    exit -1
}


# TODO System.Net.Sockets.SocketException abfangen
try {
    $session = New-SSHSession -ComputerName $remoteHost -Credential $cred -ErrorAction Stop
}
catch
{
    Write-Information "Trying to connect to ${remoteHost}: $_" -InformationAction Continue
    exit $retVal
}
$session



    
