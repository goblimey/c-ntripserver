# c-ntripserver
BKG's reference NTRIP server slightly enhanced

This is a version of the NTRIP server and other software
from the
German Federal Agency for Cartography and Geodesy (BKG).
I've enhanced the ntrip server to take input from stdin
so that it can run at the end of a pipeline.

BKG's original software can be downloaded via the subversion tool svn
as described
[here](https://software.rtcm-ntrip.org/).

My first commit contains a clean copy of their version
8789 which they committed 2019-08-05 08:31:56Z.
The latest commit contains my enhanced version.

See the source code for the details of BKG's license.


## Building and Running

To build the ntripserver software you need the gcc compiler and the make tool.
These are easily available for a Linux machine or a Mac.
You can get versions for Windows, but building under Linux is easier.

Once you have your build environment ready, compile the source like so

```
$ git clone https://github.com/goblimey/c-ntripserver.git
$ cd c-ntripserver
$ cd src/trunk/ntripserver
$ make
```

That produces an executable binary ntripserver that you can copy and run.
The shell script startntripserver.sh shows how it might be run
