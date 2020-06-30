all: readco readVtec readRTCM3
CC         = gcc
CXX        = g++
DEFINES    = -DNO_RTCM3_MAIN
CFLAGS     = -ggdb -Wall $(DEFINES) -I lib
CFLAGSR    = $(CFLAGS) -I ../rtcm3torinex/lib/ -DNO_RTCM3_MAIN
LFLAGS     = -ggdb -lm
CXXFLAGS   = -ggdb -Wall $(DEFINES) -I lib
CXXFLAGSR  = $(CXXFLAGS) -I ../rtcm3torinex/lib/
CXXFLAGSRL = $(CXXFLAGSR) -lm

clock_orbit_rtcm.o: lib/clock_orbit_rtcm.c
	$(CC) -c $(CFLAGS) -o $@ $<

rtcm3torinex.o: ../rtcm3torinex/lib/rtcm3torinex.c
	$(CC) -c $(CFLAGSR) -o $@ $<

readco.o: readco.c clock_orbit_rtcm.o
	$(CC) -c $(CFLAGS) -o $@ $<

readVtec.o: readVtec.c
	$(CC) -c $(CFLAGS) -o $@ $<

readRTCM3.o: readRTCM3.cpp
	$(CXX) -c $(CXXFLAGSR) $< -I lib

readco: readco.o clock_orbit_rtcm.o
	$(CC) $(LFLAGS) -o $@ readco.o clock_orbit_rtcm.o

readVtec: readVtec.o clock_orbit_rtcm.o
	$(CC) $(LFLAGS) -o $@ readVtec.o clock_orbit_rtcm.o

readRTCM3: readRTCM3.o clock_orbit_rtcm.o rtcm3torinex.o
	$(CXX) $(CXXFLAGSRL) -o $@ readRTCM3.o clock_orbit_rtcm.o rtcm3torinex.o

clean:
	rm *.o
	rm readco readVtec readRTCM3
