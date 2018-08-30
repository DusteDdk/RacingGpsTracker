The firmware will be placed here..
I kind of.. probably.. formatted the disk, so I will rewrite it soon..
The format is very simple: Simply cut the $GPRMC, part of the standard nmea RMC messages,
replace the newline at the end with a pipe. This was done to save space, and the pipe thing was because it was easier to copy a gigantic line in putty when I debugged the firmware.

Note that the "framer" assumes there are messages every 100 ms (10hz) and that messages are also present when fix is lost (no holes allowed).
