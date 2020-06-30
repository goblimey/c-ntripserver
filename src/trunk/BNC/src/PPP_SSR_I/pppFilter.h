// Part of BNC, a utility for retrieving decoding and
// converting GNSS data streams from NTRIP broadcasters.
//
// Copyright (C) 2007
// German Federal Agency for Cartography and Geodesy (BKG)
// http://www.bkg.bund.de
// Czech Technical University Prague, Department of Geodesy
// http://www.fsv.cvut.cz
//
// Email: euref-ip@bkg.bund.de
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation, version 2.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

#ifndef PPPFILTER_H
#define PPPFILTER_H

#include <QByteArray>
#include <QMap>
#include <QStringList>
#include <QVector>
#include <newmat.h>

#include "bncconst.h"
#include "bnctime.h"

class bncAntex;

namespace BNC_PPP {

class t_pppClient;
class t_pppOptions;
class t_epoData;
class t_satData;
class t_tides;

class t_satData {
 public:
  t_satData() {
    obsIndex = 0;
    P1       = 0.0;
    P2       = 0.0;
    P5       = 0.0;
    P7       = 0.0;
    P3       = 0.0;
    L1       = 0.0;
    L2       = 0.0;
    L5       = 0.0;
    L7       = 0.0;
    L3       = 0.0;
    lkA      = 0.0;
    lkB      = 0.0;
    clk      = 0.0;
    eleSat   = 0.0;
    azSat    = 0.0;
    rho      = 0.0;
    slipFlag = false;
    lambda3  = 0.0;
  }
  ~t_satData() {}
  bncTime      tt;
  QString      prn;
  double       P1;
  double       P2;
  double       P5;
  double       P7;
  double       P3;
  double       L1;
  double       L2;
  double       L5;
  double       L7;
  double       L3;
  ColumnVector xx;
  ColumnVector vv;
  double       clk;
  double       eleSat;
  double       azSat;
  double       rho;
  bool         slipFlag;
  double       lambda3;
  double       lkA;
  double       lkB;
  unsigned     obsIndex;
  char system() const {return prn.toLatin1()[0];}
};

class t_epoData {
 public:
  t_epoData() {}

  ~t_epoData() {
    clear();
  }

  void clear() {
    QMapIterator<QString, t_satData*> it(satData);
    while (it.hasNext()) {
      it.next();
      delete it.value();
    }
    satData.clear();
    tt.reset();
  }

  void deepCopy(const t_epoData* from) {
    clear();
    tt = from->tt;
    QMapIterator<QString, t_satData*> it(from->satData);
    while (it.hasNext()) {
      it.next();
      satData[it.key()] = new t_satData(*it.value());
    }
  }

  unsigned sizeSys(char system) const {
    unsigned ans = 0;
    QMapIterator<QString, t_satData*> it(satData);
    while (it.hasNext()) {
      it.next();
      if (it.value()->system() == system) {
        ++ans;
      }
    }
    return ans;
  }
  unsigned sizeAll() const {return satData.size();}

  bncTime                   tt;
  QMap<QString, t_satData*> satData;
};

class t_pppParam {
 public:
  enum parType {CRD_X, CRD_Y, CRD_Z, RECCLK, TROPO, AMB_L3,
                GLONASS_OFFSET, GALILEO_OFFSET, BDS_OFFSET};
  t_pppParam(parType typeIn, int indexIn, const QString& prn);
  ~t_pppParam();
  double partial(t_satData* satData, bool phase);
  bool isCrd() const {
    return (type == CRD_X || type == CRD_Y || type == CRD_Z);
  }
  parType  type;
  double   xx;
  int      index;
  int      index_old;
  int      numEpo;
  QString  prn;
};

class t_pppFilter {
 public:
  t_pppFilter(t_pppClient* pppClient);
  ~t_pppFilter();
  t_irc update(t_epoData* epoData);
  bncTime time()  const {return _time;}
  const SymmetricMatrix& Q() const {return _QQ;}
  const ColumnVector& neu() const {return _neu;}
  int    numSat() const {return _numSat;}
  double HDOP()   const {return _hDop;}
  double x()      const {return _params[0]->xx;}
  double y()      const {return _params[1]->xx;}
  double z()      const {return _params[2]->xx;}
  double clk()    const {return _params[3]->xx;}
  double trp0()   {return delay_saast(M_PI/2.0);}
  double trp() const {
    for (int ii = 0; ii < _params.size(); ++ii) {
      t_pppParam* pp = _params[ii];
      if (pp->type == t_pppParam::TROPO) {
        return pp->xx;
      }
    }
    return 0.0;
  }
  double trpStdev() const {
    for (int ii = 0; ii < _params.size(); ++ii) {
      t_pppParam* pp = _params[ii];
      if (pp->type == t_pppParam::TROPO) {
        return sqrt(Q()[ii][ii]);
      }
    }
    return 0.0;
  }
  double Glonass_offset() const {
    for (int ii = 0; ii < _params.size(); ++ii) {
      t_pppParam* pp = _params[ii];
      if (pp->type == t_pppParam::GLONASS_OFFSET) {
        return pp->xx;
      }
    }
    return 0.0;
  }
  double Galileo_offset() const {
    for (int ii = 0; ii < _params.size(); ++ii) {
      t_pppParam* pp = _params[ii];
      if (pp->type == t_pppParam::GALILEO_OFFSET) {
        return pp->xx;
      }
    }
    return 0.0;
  }
  double Bds_offset() const {
    for (int ii = 0; ii < _params.size(); ++ii) {
      t_pppParam* pp = _params[ii];
      if (pp->type == t_pppParam::BDS_OFFSET) {
        return pp->xx;
      }
    }
    return 0.0;
  }
 private:
  void   reset();
  t_irc  cmpBancroft(t_epoData* epoData);
  void   cmpEle(t_satData* satData);
  void   addAmb(t_satData* satData);
  void   addObs(int iPhase, unsigned& iObs, t_satData* satData,
                Matrix& AA, ColumnVector& ll, DiagonalMatrix& PP);
  QByteArray printRes(int iPhase, const ColumnVector& vv,
                      const QMap<QString, t_satData*>& satDataMap);
  void   findMaxRes(const ColumnVector& vv,
                    const QMap<QString, t_satData*>& satData,
                    QString& prnGPS, QString& prnGlo,
                    double& maxResGPS, double& maxResGlo);
  double cmpValue(t_satData* satData, bool phase);
  double delay_saast(double Ele);
  void   predict(int iPhase, t_epoData* epoData);
  t_irc  update_p(t_epoData* epoData);
  QString outlierDetection(int iPhase, const ColumnVector& vv,
                           QMap<QString, t_satData*>& satData);

  double windUp(const QString& prn, const ColumnVector& rSat,
                const ColumnVector& rRec);

  bncTime  _startTime;

  void rememberState(t_epoData* epoData);
  void restoreState(t_epoData* epoData);

  t_irc selectSatellites(const QString& lastOutlierPrn,
                         QMap<QString, t_satData*>& satData);

  void bancroft(const Matrix& BBpass, ColumnVector& pos);

  void cmpDOP(t_epoData* epoData);

  t_pppClient*          _pppClient;
  bncTime               _time;
  bncTime               _lastTimeOK;
  QVector<t_pppParam*>  _params;
  SymmetricMatrix       _QQ;
  QVector<t_pppParam*>  _params_sav;
  SymmetricMatrix       _QQ_sav;
  t_epoData*            _epoData_sav;
  ColumnVector          _xcBanc;
  ColumnVector          _ellBanc;
  QMap<QString, double> _windUpTime;
  QMap<QString, double> _windUpSum;
  QStringList           _outlierGPS;
  QStringList           _outlierGlo;
  bncAntex*             _antex;
  t_tides*              _tides;
  ColumnVector          _neu;
  int                   _numSat;
  double                _hDop;
};

}

#endif
