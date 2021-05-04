param ([string]$dateString)
"input: $dateString, typ: $($dateString.GetType())`n"
$date = ""
$errorLevel=10;
try {
    $date = Get-Date $dateString 
}
catch [System.Management.Automation.ParameterBindingException] {   
    "parameter: $dateString does not match required format YYYY-MM-DD`n"
}
"converted date: $date, type: $($date.GetType())"