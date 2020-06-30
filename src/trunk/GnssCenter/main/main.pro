
TEMPLATE     = app
TARGET       = ../GnssCenter
CONFIG      += debug

debug:OBJECTS_DIR   = .obj/debug
debug:MOC_DIR       = .moc/debug
release:OBJECTS_DIR = .obj/release
release:MOC_DIR     = .moc/release

HEADERS +=              app.h       mdiarea.h   \
           settings.h   mainwin.h   plugininterface.h

SOURCES += main.cpp     app.cpp     mdiarea.cpp \
           settings.cpp mainwin.cpp

