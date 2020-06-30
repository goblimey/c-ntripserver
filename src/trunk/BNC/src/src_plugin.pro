
TEMPLATE     = lib
CONFIG      += plugin debug
TARGET       = $$qtLibraryTarget(gnsscenter_bnc)
DESTDIR      = ../../GnssCenter/plugins
DEFINES     += GNSSCENTER_PLUGIN
INCLUDEPATH += ../../GnssCenter/main

include(src.pri)
