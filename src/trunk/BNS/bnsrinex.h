#ifndef BNSRINEX_H
#define BNSRINEX_H

#include <fstream>
#include <newmat.h>
#include <QtCore>

#include "bnsoutf.h"

class bnsRinex : public bnsoutf {
 public:
  bnsRinex(const QString& prep, const QString& ext, const QString& path,
           const QString& intr, int sampl);
  virtual ~bnsRinex();
  virtual BNS::t_irc write(int GPSweek, double GPSweeks, const QString& prn, 
                           const ColumnVector& xx);

 private:
  virtual void writeHeader(const QDateTime& datTim);
  bool _append;
};

#endif
