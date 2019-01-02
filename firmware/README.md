What's here
================
This is the firmware

The GPS thing, user manual
--------------------------
 # About
 # Usage -Recording
 # LED Indicators
 # Charging
 # Usage -Getting data out / removing files
 # The .RMC format



About
-----
The GPS thing is a GPS logger for racing which is very easy to use.
It writes GPS locations to a file 10 times per second when recording.
The format is a slightly mangled NMEA RMC message stream.
It was made for Road Racing, to record data for video overlay.
It is open-source, under the WTFPL.
Technical expertise is required for building, charging and extracting data,
but not for recording.



Usage - Recording
-----------------
Turn on, good idea to do it 10 minutes before, to get a solid fix, let it sit outside with clear view to sky.
Press button to start recording, the red light should be only one on.
Press button to stop recording.
If turned off while recording, up  to one minute of data may be lost (autosaves once per minute).
Recording can be started with or without signal.



LED indicators
--------------
From top to bottom (button is below LED3).

+-----+-----------+--------------------+---------------+----------------+
|     | Solid on  | 100 ms blinks      | 500 ms blinks | 1000 ms blinks |
+-----+-----------+--------------------+---------------+----------------+
|LED1 | Searching | Couldn't open      |               |                |
|     |           | file for recording |               |                |
+-----+-----------+--------------------+---------------+----------------+
|LED2 | Found SAT | No free filename   |               | SD Card        |
|     |           | for recording      |               | Init error     |
+-----+-----------+--------------------+---------------+----------------+
|LED3 | Recording |                    | Autosaving    |                |
|     |           |                    | failed        |                |
+-----+-----------+--------------------+---------------+----------------+
# LED1+LED2 Solid = UART Comms mode.



Charging
--------
Wire to a single-cell liion charger:
Gray - Negative
Red  - Positive



Usage - Getting data out / removing files
-----------------------------------------
Turn on while holding down the button. Both leds stay on, it is in uart mode.
Wire up the UART (there is enough power on the uart to keep the MCU alive, so wire GND after turning on)
Green wire: TTL UART TX
Yello wire: TTL UART RX
Black wire: Gnd for UART
Connect with uart speed: 28800  (which is 9600x3)
Press ? to view online help.
Press d to list directory
Press s to dump a file
Press r to remove a file
Backspace is not supporeted, if a typo is made, press enter and try again, unless pressing enter
would cause you to delete a file, and you didn't want it deleted, in that case, type more to ensure an invalid filename.



The .RMC format
---------------
Files are named according to %.2X.RMC.
There are no linebreaks in the file.
Each message is terminated with a pipe ( | ).
If one replaces all pipes with newlines, and prefixes all lines with "$GPRMC," (without "")
then the file is a standard NMEA RMC message stream.
