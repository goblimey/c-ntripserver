
unix:DEFINES  += _TTY_POSIX_
win32:DEFINES += _TTY_WIN_

RESOURCES += bnc.qrc

QT += svg
QT += printsupport
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
greaterThan(QT_MAJOR_VERSION, 4): QT += core

unix:QMAKE_CFLAGS_RELEASE   -= -O2
unix:QMAKE_CXXFLAGS_RELEASE -= -O2

# Get rid of mingwm10.dll and libgcc_s_dw2-1.dll
# ----------------------------------------------
win32 {
  QMAKE_LFLAGS                 += -static-libgcc
  QMAKE_LFLAGS                 -= -mthreads
  QMAKE_CXXFLAGS_EXCEPTIONS_ON -= -mthreads
  QMAKE_LFLAGS_EXCEPTIONS_ON   -= -mthreads
}

debug:OBJECTS_DIR=.obj/debug
debug:MOC_DIR=.moc/debug
release:OBJECTS_DIR=.obj/release
release:MOC_DIR=.moc/release

debug:DEFINES  += BNC_DEBUG

# Include Path
# ------------
INCLUDEPATH += . ../newmat ./RTCM3 ./RTCM3/clock_and_orbit ./RTCM \
               ../qwt ../qwtpolar

# Additional Libraries
# --------------------
unix:LIBS  += -L../newmat -lnewmat -L../qwt -L../qwtpolar -lqwtpolar -lqwt
win32:LIBS += -L../newmat/release -L../qwt/release -L../qwtpolar/release \
              -lnewmat -lqwtpolar -lqwt

HEADERS = bnchelp.html bncgetthread.h    bncwindow.h   bnctabledlg.h  \
          bnccaster.h bncrinex.h bnccore.h bncutils.h   bnchlpdlg.h   \
          bncconst.h bnchtml.h bnctableitem.h bnczerodecoder.h        \
          bncnetquery.h bncnetqueryv1.h bncnetqueryv2.h               \
          bncnetqueryrtp.h bncsettings.h latencychecker.h             \
          bncipport.h bncnetqueryv0.h bncnetqueryudp.h                \
          bncnetqueryudp0.h bncudpport.h bnctime.h                    \
          bncserialport.h bncnetquerys.h bncfigure.h                  \
          bncfigurelate.h bncversion.h                                \
          bncfigureppp.h bncrawfile.h                                 \
          bncmap.h bncantex.h bncephuser.h                            \
          bncoutf.h bncclockrinex.h bncsp3.h bncsinextro.h            \
          bncbytescounter.h bncsslconfig.h reqcdlg.h                  \
          upload/bncrtnetdecoder.h upload/bncuploadcaster.h           \
          ephemeris.h t_prn.h satObs.h                                \
          upload/bncrtnetuploadcaster.h upload/bnccustomtrafo.h       \
          upload/bncephuploadcaster.h qtfilechooser.h                 \
          GPSDecoder.h pppInclude.h pppWidgets.h pppModel.h           \
          pppMain.h pppRun.h pppOptions.h pppCrdFile.h pppThread.h    \
          RTCM/RTCM2.h RTCM/RTCM2Decoder.h                            \
          RTCM/RTCM2_2021.h RTCM/rtcm_utils.h                         \
          RTCM3/RTCM3Decoder.h RTCM3/bits.h RTCM3/gnss.h              \
          RTCM3/RTCM3coDecoder.h RTCM3/ephEncoder.h                   \
          RTCM3/clock_and_orbit/clock_orbit_rtcm.h                    \
          rinex/rnxobsfile.h                                          \
          rinex/rnxnavfile.h       rinex/corrfile.h                   \
          rinex/reqcedit.h         rinex/reqcanalyze.h                \
          rinex/graphwin.h         rinex/polarplot.h                  \
          rinex/availplot.h        rinex/eleplot.h                    \
          rinex/dopplot.h          orbComp/sp3Comp.h                  \
          combination/bnccomb.h

HEADERS       += serial/qextserialbase.h serial/qextserialport.h
unix:HEADERS  += serial/posix_qextserialport.h
win32:HEADERS += serial/win_qextserialport.h

SOURCES = bncgetthread.cpp  bncwindow.cpp bnctabledlg.cpp             \
          bnccaster.cpp bncrinex.cpp bnccore.cpp bncutils.cpp         \
          bncconst.cpp bnchtml.cpp bnchlpdlg.cpp bnctableitem.cpp     \
          bnczerodecoder.cpp bncnetqueryv1.cpp bncnetqueryv2.cpp      \
          bncnetqueryrtp.cpp bncsettings.cpp latencychecker.cpp       \
          bncipport.cpp bncnetqueryv0.cpp bncnetqueryudp.cpp          \
          bncnetqueryudp0.cpp bncudpport.cpp                          \
          bncserialport.cpp bncnetquerys.cpp bncfigure.cpp            \
          bncfigurelate.cpp bnctime.cpp                               \
          bncfigureppp.cpp bncrawfile.cpp                             \
          bncmap_svg.cpp bncantex.cpp bncephuser.cpp                  \
          bncoutf.cpp bncclockrinex.cpp bncsp3.cpp bncsinextro.cpp    \
          bncbytescounter.cpp bncsslconfig.cpp reqcdlg.cpp            \
          ephemeris.cpp t_prn.cpp satObs.cpp                          \
          upload/bncrtnetdecoder.cpp upload/bncuploadcaster.cpp       \
          upload/bncrtnetuploadcaster.cpp upload/bnccustomtrafo.cpp   \
          upload/bncephuploadcaster.cpp qtfilechooser.cpp             \
          GPSDecoder.cpp pppWidgets.cpp pppModel.cpp                  \
          pppMain.cpp pppRun.cpp pppOptions.cpp pppCrdFile.cpp        \
          pppThread.cpp                                               \
          RTCM/RTCM2.cpp RTCM/RTCM2Decoder.cpp                        \
          RTCM/RTCM2_2021.cpp RTCM/rtcm_utils.cpp                     \
          RTCM3/RTCM3Decoder.cpp                                      \
          RTCM3/RTCM3coDecoder.cpp RTCM3/ephEncoder.cpp               \
          RTCM3/clock_and_orbit/clock_orbit_rtcm.c                    \
          rinex/rnxobsfile.cpp                                        \
          rinex/rnxnavfile.cpp     rinex/corrfile.cpp                 \
          rinex/reqcedit.cpp       rinex/reqcanalyze.cpp              \
          rinex/graphwin.cpp       rinex/polarplot.cpp                \
          rinex/availplot.cpp      rinex/eleplot.cpp                  \
          rinex/dopplot.cpp        orbComp/sp3Comp.cpp                \
          combination/bnccomb.cpp

SOURCES       += serial/qextserialbase.cpp serial/qextserialport.cpp
unix:SOURCES  += serial/posix_qextserialport.cpp
win32:SOURCES += serial/win_qextserialport.cpp

RC_FILE = bnc.rc

QT += network

exists(PPP) {
  INCLUDEPATH += PPP
  DEFINES += USE_PPP
  HEADERS += PPP/pppClient.h    PPP/pppObsPool.h   PPP/pppEphPool.h   \
             PPP/pppStation.h   PPP/pppFilter.h    PPP/pppParlist.h   \
             PPP/pppSatObs.h    PPP/pppRefSat.h
  SOURCES += PPP/pppClient.cpp  PPP/pppObsPool.cpp PPP/pppEphPool.cpp \
             PPP/pppStation.cpp PPP/pppFilter.cpp  PPP/pppParlist.cpp \
             PPP/pppSatObs.cpp  PPP/pppRefSat.cpp
}
else {
  INCLUDEPATH += PPP_SSR_I
  DEFINES += USE_PPP_SSR_I
  HEADERS += PPP_SSR_I/pppClient.h   PPP_SSR_I/pppFilter.h   PPP_SSR_I/pppUtils.h
  SOURCES += PPP_SSR_I/pppClient.cpp PPP_SSR_I/pppFilter.cpp PPP_SSR_I/pppUtils.cpp
}

# Check QtWebEngineWidges Library Existence
# -----------------------------------------
exists("$$[QT_INSTALL_LIBS]/*WebEngineWidgets*") {
    DEFINES += QT_WEBENGINE
    QT      += webenginewidgets
    message("Configured with QtWebEngine")
}

contains(DEFINES, QT_WEBENGINE) {
  HEADERS     += map/bncmapwin.h
  SOURCES     += map/bncmapwin.cpp
  OTHER_FILES += map/map_gm.html map/map_osm.html
}
else {
  message("QtWebEngineWidgets Library is not available")
}