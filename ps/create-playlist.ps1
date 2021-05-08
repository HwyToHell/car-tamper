param([string]$videoPath, [datetime]$date)

function createPlaylist {
    param([string]$videoPath, [datetime]$date)

    $dateString = Get-Date -Date $date -Format yyyy-MM-dd

    $videoDatePath = Join-Path $videoPath $dateString
    $playListName = "_alle_Videos_abspielen_" + $dateString + ".xspf"
    $playListPath = Join-Path $videoDatePath $playListName

    $xmlPlayList = New-Object System.Xml.XmlTextWriter($playListPath, $null)
    $xmlPlayList.Formatting = "Indented"
    $xmlPlayList.IndentChar = " " 

    $xmlPlayList.WriteStartDocument()

    $xmlPlayList.WriteStartElement("playlist")

    $xmlPlayList.WriteElementString("title", "Wiedergabeliste") 

    $xmlPlayList.WriteStartElement("trackList")


    $videoFiles = Get-ChildItem ($videoDatePath + "\*.avi")
    $preFix = "file:///"

    foreach ($file in $videoFiles.Name) {
        $fileEntry = $preFix + ($videoDatePath -replace "\\","/") + "/" + $file
        $xmlPlayList.WriteStartElement("track")
        $xmlPlayList.WriteElementString("location", $fileEntry)
        $xmlPlayList.WriteEndElement() #track
    }

    $xmlPlayList.WriteEndElement() #trackList

    $xmlPlayList.WriteEndElement() #playlist

    $xmlPlayList.WriteEndDocument()
    $xmlPlayList.Flush()                                                               
    $xmlPlayList.Close()
}

# $videoPathInput = "C:\Users\holge\Videos"
# $dateStringInput = "2021-05-04"

createPlaylist $videoPath $date

# auf Carlas Rechner in Skript Bewegungsanalyse
# foreach ($date in $datesToAnalyze) {
#   createPlaylist $localPath $date