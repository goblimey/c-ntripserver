
TEMPLATE = lib
TARGET   = qwtpolar
CONFIG  += qt qwt staticlib
CONFIG -= debug
CONFIG += release
DEFINES += QWT_POLAR_NO_SVG
greaterThan(QT_MAJOR_VERSION, 4) {
    QT += printsupport
    QT += concurrent
}

INCLUDEPATH += ../qwt

OBJECTS_DIR = .obj
MOC_DIR     = .moc

HEADERS += \
    qwt_polar_global.h \
    qwt_polar.h \
    qwt_polar_fitter.h \
    qwt_polar_item.h \
    qwt_polar_picker.h \
    qwt_polar_panner.h \
    qwt_polar_magnifier.h \
    qwt_polar_grid.h \
    qwt_polar_curve.h \
    qwt_polar_spectrogram.h \
    qwt_polar_marker.h \
    qwt_polar_itemdict.h \
    qwt_polar_canvas.h \
    qwt_polar_layout.h \
    qwt_polar_renderer.h \
    qwt_polar_plot.h

SOURCES += \
    qwt_polar_fitter.cpp \
    qwt_polar_item.cpp \
    qwt_polar_picker.cpp \
    qwt_polar_panner.cpp \
    qwt_polar_magnifier.cpp \
    qwt_polar_grid.cpp \
    qwt_polar_curve.cpp \
    qwt_polar_spectrogram.cpp \
    qwt_polar_marker.cpp \
    qwt_polar_itemdict.cpp \
    qwt_polar_canvas.cpp \
    qwt_polar_layout.cpp \
    qwt_polar_renderer.cpp \
    qwt_polar_plot.cpp
