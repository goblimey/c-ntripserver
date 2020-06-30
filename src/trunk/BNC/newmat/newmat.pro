
TEMPLATE  = lib
TARGET    = newmat
CONFIG   += staticlib

CONFIG -= debug
CONFIG += release

OBJECTS_DIR = .obj
MOC_DIR     = .moc

HEADERS += boolean.h controlw.h include.h myexcept.h  \
           newmatap.h newmat.h newmatio.h newmatnl.h  \
           newmatrc.h newmatrm.h precisio.h solution.h

SOURCES += bandmat.cpp cholesky.cpp evalue.cpp     \
           fft.cpp hholder.cpp jacobi.cpp          \
           myexcept.cpp newfft.cpp newmat1.cpp     \
           newmat2.cpp newmat3.cpp newmat4.cpp     \
           newmat5.cpp newmat6.cpp newmat7.cpp     \
           newmat8.cpp newmat9.cpp newmatex.cpp    \
           newmatrm.cpp newmatnl.cpp nm_misc.cpp   \
	   sort.cpp submat.cpp svd.cpp solution.cpp

