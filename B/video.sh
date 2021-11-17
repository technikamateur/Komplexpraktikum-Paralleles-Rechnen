#!/bin/bash
printf "Hello! I will create a video file out of your .pbm files.\n"
read -ep "Please give me the prefix: " -i "life_" prefix
printf "Okay. Producing video now.\n"
ffmpeg -f image2 -r 6 -i $prefix%06d.pbm -q:v 2 -vf scale=1080:1080 result.mp4
printf "\nDone. Output filename: result.mp4\n"
exit 0
