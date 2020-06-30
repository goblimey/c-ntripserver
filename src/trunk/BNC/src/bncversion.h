
#ifndef BNCVERSION_H
#define BNCVERSION_H

#define BNCVERSION "2.13.0"
#define BNCPGMNAME "BNC " BNCVERSION

#if   defined(Q_OS_LINUX)
#  define BNC_OS "LINUX"
#elif defined(Q_OS_MAC)
#  define BNC_OS "MAC"
#elif defined(Q_OS_WIN32)
#  define BNC_OS "WIN32"
#else
#  define BNC_OS "UNKNOWN"
#endif

#endif
