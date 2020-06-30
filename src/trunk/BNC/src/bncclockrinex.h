#ifndef BNCCLOCKRINEX_H
#define BNCCLOCKRINEX_H

#include <fstream>
#include <newmat.h>
#include <QtCore>

#include "bncoutf.h"

class bncClockRinex : public bncoutf {
 public:
  bncClockRinex(const QString& sklFileName, const QString& intr, int sampl);
  virtual ~bncClockRinex();
  virtual t_irc write(int GPSweek, double GPSweeks, const QString& prn,
      double clkRnx, double clkRnxRate, double clkRnxAcc,
      double clkRnxSig, double clkRnxRateSig, double clkRnxAccSig);

 private:
  virtual void writeHeader(const QDateTime& datTim);
  bool _append;
};

#endif
