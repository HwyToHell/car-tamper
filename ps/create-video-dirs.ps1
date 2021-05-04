$videoPath = "C:\Users\holge\Videos"

$days = 18, 25, 26
$hours = 14 .. 23

$fileIndex = 0

foreach ($day in $days) {
    foreach ($hour in $hours) {
        # compose time directory name
        $timeDir = Get-Date -Year 2021 -Month 04 -Day $day `
            -Hour $hour -Minute 0 -Second 0 `
            -Format yyyy-MM-dd_HH\hmm\mss\s 
        
        # mkdir, -Force supresses error message, if folder exists already
        New-Item -ItemType directory -Path $videoPath -Name $timeDir -Force

        # create 5 files with consecutive file name
        for ($i = 0; $i -lt 5; $i++) {
            $fileIndex++
            $fileName = "$fileIndex" + ".mp4"
            $timePath = Join-Path -Path $videoPath -ChildPath $timeDir
            # -Force re-creates files
            New-Item -ItemType file -Path $timePath -Name $fileName -Force

        }
    }
}