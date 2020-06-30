#ifndef BNCOUTF_H
#define BNCOUTF_H

#include <fstream>
#include <newmat.h>
#include <QtCore>

#include "bncutils.h"

class bncoutf {
 public:
  bncoutf(const QString& sklFileName, const QString& intr, int sampl);
  virtual ~bncoutf();
  t_irc write(int GPSweek, double GPSweeks, const QString& str);

 protected:
  virtual t_irc reopen(int GPSweek, double GPSweeks);
  virtual void  writeHeader(const QDateTime& /* datTim */) {}
  virtual void  closeFile();
  std::ofstream _out;
  int           _sampl;
  int           _numSec;

 private:
  QString epochStr(const QDateTime& datTim, const QString& intStr,
      int sampl);
  QString resolveFileName(int GPSweek, const QDateTime& datTim);

  bool    _headerWritten;
  QString _path;
  QString _sklBaseName;
  QString _extension;
  QString _intr;
  QString _fName;
  bool    _append;
  bool    _v3filenames;
};

#endif
