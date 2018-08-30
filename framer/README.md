The Framer
==========

Render overlay frames from GPS data, fit for usage in video

Building
========

go build

Usage
=====
Some tweaking is requierd, it's easiest to render a single, late frame, to determine what needs tweaking.
./framer --help     Show all the options

Example:
./framer -first 1450 -in 01.RMC -out out/ -trackFirst 95 -trackLast 8410 -finishAt 1685 -finishAngle -35  -firstLap 1 -lastLap 50

-first 1450         The first sample that is used is 1450
-in 01.RMC          The input file is 01.RMC
-out out/           out is an empty directory where the images will be ouput
-trackFirst         Start drawing track 95 samples after the first sample. (This allows the red dot to move outside the track at the start/end)
-trackLast 8410     Stop drawing track after sample 1685, so we don't draw the route out of the track
-finishAt 1685      Sample 1685 was taken very close to the place of the finish line (I eyeballed it!)
-finishAngle -35    Rotate the finish-line -35 degrees so that it crosses the track (eyeballed)
-firstLap 1         Skip the first lap (0), and use lap 1 as the first lap, lap0 was in this case begun when driving onto the track
-lastLap 8          Stop after 8 laps (lap 0 is ignored), since the last lap was just slow driving into pit

Generate video overlays
=======================

Check the scripts:
render-alignment-video.sh - For quickly rendering a video which can be used to line-up GPS data and video, this is a throwaway video..
render-video.sh - Overlay the image files from the output directory onto the target video (generates new viedo)
