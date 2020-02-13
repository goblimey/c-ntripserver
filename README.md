# c-ntripserver
BKG's reference NTRIP server with some useful tweaks

This is a version of the NTRIP server from the
German Federal Agency for Cartography and Geodesy (BKG),
with some useful tweaks.

## Building and Running

I run the serverand the related software on a Raspberry Pi
connected to the Internet via my home braodband.
To make life easy, the whole setup is built and run using Docker.
All you need do is install Docker on your host computer,
create a configuration file
and run the Docker build.

If you are running a Docker image and it dies for some reason,
all the files that it created are destroyed.
To avoid that, you can associate some of its directories
with some permanent disk space.
Files in those survive the crash.


## How it Works

The main difference between this version and the original is that
this version has an option to read a stream of RTCM3 messages
from the standard input channel.
This allows it to be used with the RTCM Filter and
the RTCM logger.

I created this version to receive RTCM3 messages from
a UBLOX ZED-F9P dual-band GPS receiver running in a
board from Sparkfun
(the [GKS RTK2 board](https://www.sparkfun.com/products/15136)).
I connected one of these to a Raspberry Pi via a USB connection.
The Pi supplies power to the Sparkfun board
and receives RTCM messages from it,
all via the USB connection.
I run this NTRIP server on the Raspberry Pi.
It picks up the incoming messages and sends them to a remote
NTRIP caster.
In my case that's [this software](https://github.com/goblimey/ntripcaster)
running on a Digital Ocean Droplet,
which I rent for $5 per month.
NTRIP clients can pick up the corrections from there
and use them to get a more accurate notion of their position.

By default the Sparkfun board sends all sorts of data,
not just RTCM messages.
Also, the USB connection is prone to dropping characters
now and then,
which can result in an illegal RTCM message.
These issues can cause the downstream software to fail.
I avoid that by running the incoming data through the RTCM filter,
which reads the stream of messages from the Sparkfun board,
removes anything which is not a valid RTCM message
and passes what's left to the standard output channel.

For an NTRIP server to work properly,
it needs an accurate notion of its position.
You can get that by collecting the RTCM messages
over a period (a month is good),
converting them to RINEX format
and sending them off to be processed.
I use [the Canadian Government's free Precise Point Positioning service]
(https://webapp.geod.nrcan.gc.ca/geod/tools-outils/ppp.php?locale=en)
to do that.
I send one RINEX file for each day
via that web page
and I get back a set of files including data.sum (a summary) containing
a position, for example:

```
POS CRD  SYST        EPOCH          A_PRIORI
POS   X IGS14 20:022:00078      3996536.5525    
POS   Y IGS14 20:022:00078       -22571.8168    
POS   Z IGS14 20:022:00078      4954168.0568    
POS LAT IGS14 20:022:00078    51 17 39.48816 
POS LON IGS14 20:022:00078    -0 19 24.93915 
POS HGT IGS14 20:022:00078          107.6615 
```
That's my antenna's position in cartesian (x,y,z) format,
and the equivalent longitude, latitude and elipsoidal height.

My RTCM Logger reads the data stream from the RTCM Filter
and copies the messages to a daily log file.
At the end of the day,
the logger moves the day's log file to a target directory.
A cron job keeps an eye on that directory,
converts any files that appear into RINEX format
and moves the log file and the RINEX file into another directory.
You can then send it to the PPP service for processing.

The sequence is:

                      ------------- Raspberry Pi ---------------
    Sparkfun board -> RTCM filter -> rtcm logger -> NTRIP server -> remote NTRIP Caster
                                         |
                                         v
                                  daily log files

Once you've established the position of your antenna,
you no longer need to produce log files,
so you just need this sequence:

                      ------ Raspberry Pi ------
    Sparkfun board -> RTCM filter -> NTRIP server -> remote NTRIP Caster
    
