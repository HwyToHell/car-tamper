# extract dates from filenames of given directory
# usage: dates-from-filenames.ps1 -path "D:\Videos"
param($path=(Get-Location), $file)
"path: " + $path
"file: " + $file

$workDir = Get-Location
(Get-Location).ToString()


