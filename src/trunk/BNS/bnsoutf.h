#ifndef BNSOUTF_H
#define BNSOUTF_H

#include <fstream>
#include <newmat.h>
#include <QtCore>

#include "bnsutils.h"

class bnsoutf {
 public:
  bnsoutf(const QString& prep, const QString& ext, const QString& path,
          const QString& intr, int sampl);
  virtual ~bnsoutf();

  virtual BNS::t_irc write(int GPSweek, double GPSweeks, const QString& prn, 
                           const ColumnVector& xx, bool append);

 protected:
  virtual void writeHeader(const QDateTime& datTim) = 0;
  virtual void closeFile();
  std::ofstream _out;
  int           _sampl;

 private:
  QString nextEpochStr(const QDateTime& datTim,
                       const QString& intStr, 
                       QDateTime* nextEpoch = 0);
  void resolveFileName(int GPSweek, const QDateTime& datTim);

  bool          _headerWritten;
  QDateTime     _nextCloseEpoch;
  QString       _path;
  QString       _intr;
  QString       _ext;
  QString       _prep;
  QByteArray    _fName;
};

#endif
