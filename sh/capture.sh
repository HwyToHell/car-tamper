#!/bin/bash
if [ $# -eq 0 ]
then
    echo "usage: $0 <hours recording time>"
    exit 1
fi

PATH=/home/pi/bin:/usr/local/bin:/usr/bin/ffmpeg:/usr/bin:/bin:/sbin
wdir=/home/pi/Videos
logfile=log
echo >> $wdir/$logfile
echo cron job capture for $1 hours started at $(date '+%F %T') >> $wdir/$logfile

function capture
{
    capfile=$(date +%F_%Hh%Mm%Ss)
    echo saving to $wdir/$capfile.mp4
    echo $PATH
    ffmpeg -video_size 640x480 -i /dev/video0 \
    -vf drawtext="fontsize=18:x=10:y=460:fontcolor=red:\
    fontfile=/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf:\
    text=%{localtime}" -c:v h264_omx -r 10 -t 3600 $wdir/$capfile.mp4
    # change -t to 3600 = 1h
}

for i in $(seq 1 $1)
do
    echo 
    echo Pass $i ...
    capture
done

# delete video files older than 7 days to avoid SD memory shortage
rm-old-videos.sh 7 /home/pi/Videos >> $wdir/$logfile

echo cron job capture for $1 hours finished at $(date '+%F %T') >> $wdir/$logfile
