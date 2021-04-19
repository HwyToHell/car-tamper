#!/bin/bash
# delete all mp4 files older than $1 days
# optional argument: $2 working path 
# take current dir, if no argument $2 has been given


# validate command line arguments
pathWork=$(pwd)
if [ $# -gt 0 ]
then
	historyDays=$1
	if [ $# -gt 1 ]
	then
		pathWork=$2
		if [ ! -d "$pathWork" ]
		then
			echo "invalid path, exit"
			exit 2
		fi
	fi
else
	echo "usage rm-old-videos.sh <days-of-history> <optional: working-path>"
	exit 1
fi

# extension for video files
extension=*.mp4
echo "path to clean from old video files: $pathWork"
cd $pathWork

# clean date arithmetic
today=$(date +%s)
oneDay=$[60*60*24]
cleanDate=$[ today - (historyDays * oneDay) ]
# echo $(date --date=@$cleanDate)

for file in $(ls $pathWork/$extension)
do
	#echo $(stat $file)
	if [ -f $file ]
	then
		outdated=$[ $(stat --format=%Y $file) - cleanDate ]
		if [ $outdated -lt 0 ]
		then
			echo "remove $file"
			rm $file
		fi
	fi
done
