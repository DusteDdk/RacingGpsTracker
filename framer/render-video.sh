#!/bin/bash

if [ -z "$4" ]
then
    echo "Usage: $0 INDIR INVID.mp4 OUTVID.mp4 OFFSET"
    echo "Example offsets: MM:SS.MS or SS.MS (like 2:84.21 or 24.51)"
    exit 1
fi

set -e

INDIR="$1"
INVID=`realpath "$2"`
OUTVID=`realpath "$3"`
OFFSET="$4"

cd "$INDIR"

ffmpeg -i "$INVID" -framerate 10 -itsoffset $OFFSET -i frame%06d.png \
 -filter_complex "[0:v]overlay=enable=gte(t\,$OFFSET):shortest=1[out]" \
 -map [out] -map 0:a   -c:v libx264 -preset slower -crf 20 -c:a copy \
 -pix_fmt yuv420p "$OUTVID"

echo "Rendered $OUTVID"
