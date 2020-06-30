#ifndef BNCSP3_H
#define BNCSP3_H

#include <fstream>
#include <newmat.h>
#include <QtCore>

#include "bncoutf.h"
#include "bnctime.h"
#include "t_prn.h"

class bncSP3 : public bncoutf {
 public:

  class t_sp3Sat {
   public:
    t_sp3Sat() {
      _xyz.ReSize(3);
      _xyz      = 0.0;
      _clk      = 0.0;
      _clkValid = false;
    }
    ~t_sp3Sat() {}
    t_prn        _prn;
    ColumnVector _xyz;
    double       _clk;
    bool         _clkValid;
  };

  class t_sp3Epoch {
   public:
    t_sp3Epoch() {}
    ~t_sp3Epoch() {
      for (int ii = 0; ii < _sp3Sat.size(); ii++) {
        delete _sp3Sat[ii];
      }
    }
    bncTime            _tt;
    QVector<t_sp3Sat*> _sp3Sat;
  };

  bncSP3(const QString& fileName); // input
  bncSP3(const QString& sklFileName, const QString& intr, int sampl); // output
  virtual ~bncSP3();
  t_irc write(int GPSweek, double GPSweeks, const QString& prn,
              const ColumnVector& xCoM, double sp3Clk,
              const ColumnVector& v, double sp3ClkRate);
  const t_sp3Epoch* nextEpoch();
  const t_sp3Epoch* currEpoch() const {return _currEpoch;}
  const t_sp3Epoch* prevEpoch() const {return _prevEpoch;}

 private:
  enum e_inpOut {input, output};

  virtual void writeHeader(const QDateTime& datTim);
  virtual void closeFile();

  e_inpOut      _inpOut;
  bncTime       _lastEpoTime;
  std::ifstream _stream;
  std::string   _lastLine;
  t_sp3Epoch*   _currEpoch;
  t_sp3Epoch*   _prevEpoch;
};

#endif
