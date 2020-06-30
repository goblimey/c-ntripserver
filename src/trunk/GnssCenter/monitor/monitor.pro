
TEMPLATE             = lib
CONFIG              += plugin debug
QT                  += svg
TARGET               = $$qtLibraryTarget(gnsscenter_monitor)
DESTDIR              = ../plugins
DEFINES             += HAVE_INTTYPES_H HAVE_NETINET_IN_H

debug:OBJECTS_DIR   = .obj/debug
debug:MOC_DIR       = .moc/debug
release:OBJECTS_DIR = .obj/release
release:MOC_DIR     = .moc/release

THRIFT_ROOT  = /usr/local
THRIFT_RTNET = $(HOME)/gpss_src/thrift

INCLUDEPATH         += ../main ../qwt $$THRIFT_ROOT/include/thrift
QMAKE_LIBDIR        += ../qwt $$THRIFT_ROOT/lib
LIBS                 = -lqwt -lthrift

thrift.target   = gen-cpp
thrift.commands = "$$THRIFT_ROOT/bin/thrift -r -gen cpp $$THRIFT_RTNET/rtnet.thrift"
thrift.depends  = $$THRIFT_RTNET/rtnet.thrift $$THRIFT_RTNET/rtnet_data.thrift

PRE_TARGETDEPS      += gen-cpp
QMAKE_EXTRA_TARGETS += thrift

HEADERS   = monitor.h      \
            dlgconf.h      \
            utils.h        \
            const.h        \
            worldplot.h    \
            thriftclient.h 

SOURCES   = monitor.cpp      \
            dlgconf.cpp      \
            utils.cpp        \
            const.cpp        \
            worldplot.cpp    \
            thriftclient.cpp \
            ../main/settings.cpp \
            gen-cpp/RtnetData.cpp \
            gen-cpp/rtnet_constants.cpp gen-cpp/rtnet_types.cpp \
            gen-cpp/rtnet_data_constants.cpp gen-cpp/rtnet_data_types.cpp


RESOURCES = monitor.qrc
