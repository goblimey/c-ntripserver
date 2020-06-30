
TARGET = ../bnc

CONFIG -= debug
CONFIG += release

include(src.pri)

HEADERS +=             app.h

SOURCES += bncmain.cpp app.cpp
