

TEMPLATE  = lib
TARGET    = qwt
CONFIG   += staticlib

CONFIG -= debug
CONFIG += release
QWT_CONFIG = QwtPlot QwtWidgets QwtSvg

OBJECTS_DIR = .obj
MOC_DIR     = .moc

include ( src.pri )
