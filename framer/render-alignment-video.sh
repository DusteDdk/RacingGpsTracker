#!/bin/bash

if [ -z "$2" ]
then
    echo "Usage: $0 INDIR OUTVID.mp4"
    exit 1
fi

set -e

OUTVID=`realpath "$2"`
INDIR="$1"

cd "$INDIR"
ffmpeg -r 10 -i frame%06d.png -c:v libx264 -vf fps=59.94 -pix_fmt yuv420p -preset veryfast "$OUTVID"

echo "Rendered $OUTVID"
