
# Switch to debug configuration
# -----------------------------
CONFIG += release
#CONFIG += debug

DEFINES += NO_RTCM3_MAIN 
RESOURCES += bns.qrc

unix:QMAKE_CFLAGS_RELEASE   -= -O2
unix:QMAKE_CXXFLAGS_RELEASE -= -O2

# Get rid of wrong warnings with MinGW g++ Version 3.4.5
# ------------------------------------------------------
win32:CONFIG += warn_off

# Get rid of mingwm10.dll
# -----------------------
win32:QMAKE_LFLAGS                 -= -mthreads
win32:QMAKE_CXXFLAGS_EXCEPTIONS_ON -= -mthreads
win32:QMAKE_LFLAGS_EXCEPTIONS_ON   -= -mthreads

debug:OBJECTS_DIR=.obj/debug
debug:MOC_DIR=.moc/debug
release:OBJECTS_DIR=.obj/release
release:MOC_DIR=.moc/release

# Include Path
# ------------
INCLUDEPATH = . ./newmat ./rtcm3torinex ./clock_and_orbit

HEADERS =             bns.h   bnswindow.h   bnshlpdlg.h   bnshtml.h   \
          bnseph.h    bnsutils.h bnsrinex.h bnssp3.h bnsoutf.h        \
          bnscaster.h clock_and_orbit/clock_orbit_rtcm.h bnssettings.h bnsapp.h  \
          bnscustomtrafo.h rtcm3torinex/rtcm3torinex.h bnctime.h timeutils.h \
          bnsversion.h

HEADERS += newmat/controlw.h newmat/include.h newmat/myexcept.h  \
           newmat/newmatap.h newmat/newmat.h newmat/newmatio.h   \
           newmat/newmatrc.h newmat/newmatrm.h newmat/precisio.h

SOURCES = bnsmain.cpp bns.cpp bnswindow.cpp bnshlpdlg.cpp bnshtml.cpp  \
          bnseph.cpp  bnsutils.cpp bnsrinex.cpp bnssp3.cpp bnsoutf.cpp \
          bnscaster.cpp bnssettings.cpp bnsapp.cpp bnscustomtrafo.cpp  \
          clock_and_orbit/clock_orbit_rtcm.c rtcm3torinex/rtcm3torinex.c \
          bnctime.cpp timeutils.cpp

SOURCES += newmat/bandmat.cpp newmat/cholesky.cpp newmat/evalue.cpp  \
           newmat/fft.cpp newmat/hholder.cpp newmat/jacobi.cpp       \
           newmat/myexcept.cpp newmat/newfft.cpp newmat/newmat1.cpp  \
           newmat/newmat2.cpp newmat/newmat3.cpp newmat/newmat4.cpp  \
           newmat/newmat5.cpp newmat/newmat6.cpp newmat/newmat7.cpp  \
           newmat/newmat8.cpp newmat/newmat9.cpp newmat/newmatex.cpp \
           newmat/newmatrm.cpp newmat/nm_misc.cpp newmat/sort.cpp    \
           newmat/submat.cpp newmat/svd.cpp

RC_FILE = bns.rc

QT += network

