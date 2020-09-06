Extract gopro GPS data from video
==================
The Gopro hero 5 and later models are nice cameras and they can embed GPS data into the
video stream. Unfortunately, the software support is absolutely terrible, the program they
expect you to use is mainly for mobile phones (because we all know cellphones are super at
encoding high-bitrate 4k video and video editing.. Their "app" can't even stitch!).

getJsonFromVideo.js - extract GPS data from VIDEO into JSON
    usage: node getJsonFromVideo.js myFile.mp4 > gps.json

getNmeaFromJson.js - Convert GPS data into the truncated NMEA RMC format used by the framer program.
    usage: node getNmeaFromJson.js ./gps.json > track.rmc
    
    if you need to truncate multiple files, use >> merged.rmc on all file instead and then 
        remove all newlines: tr -d '\n' < merged.rmc > track.rmc.

Note: While gopro captures at ~18 hz, the framer was made with 10hz in mind, thus,
to get correct timing, and to generate a video that does not drift, getNmeaFromJson will
throw away gps samples to keep timing around 10hz, it's not pretty, and if you want it
to interpolate instead, then send me a pull request. But it's JUST FINE for generating nice
videos.

Example Workflow
======
Here i extract and combine gps data from 3 video files:

node getJsonFromVideo GX010005.MP4 > gps1.json
node getJsonFromVideo GX020005.MP4 > gps2.json
node getJsonFromVideo GX030005.MP4 > gps3.json

node getNmeaFromJson ./gps1.json > merged.rmc
node getNmeaFromJson ./gps2.json >> merged.rmc
node getNmeaFromJson ./gps3.json >> merged.rmc

tr -d '\n' < merged.rmc > final.rmc

mkdir out
./framer -in final.rmc -out out ... <-- follow instructions for how to use the framer.

ffmpeg -r 10 -i out/frame%06d.png -c:v png gps-overlay.mov

Then kdenlive and splice the videofiles and put the overlay on top.



