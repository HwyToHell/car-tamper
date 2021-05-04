param ([switch]$on, [switch]$d, $name)
# fetch videofiles from remote via ssh, analyze and store them locally
# usage: analyze-motion [day]

$remotePath = "pi@pi3:/home/pi/Videos"
$localPath = "C:\Users\holge\Videos"
$localPathInput = Join-Path $localPath Input

# im Input Dir befinden sich alle Verzeichnisse mit den analysierten Videos (*.avi)
# für jeden Tag ein Verzeichnis erstellen und Videos verschieben
$localPathInput
echo "positional arguments: $($args.count)"
for ($i=0; $i -lt $args.count; $i++) {
    $args[$i]
}

if ($on) {
    "switch on"
} else {
    "switch off"
}

if ($d) {
    "day option on"
} else {
    "day option off"
}

echo "name: $name"