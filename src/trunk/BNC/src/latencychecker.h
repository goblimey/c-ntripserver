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

#ifndef LATENCYCHECKER_H
#define LATENCYCHECKER_H

#include <QDateTime>
#include "satObs.h"

class latencyChecker : public QObject {
Q_OBJECT

 public:
  latencyChecker(QByteArray staID);
  ~latencyChecker();
  void checkReconnect();
  void checkOutage(bool decoded);
  void checkObsLatency(const QList<t_satObs>& obsList);
  void checkCorrLatency(int corrGPSEpochTime, int type);
  double currentLatency() { return _curLat;}

  class t_latency  {
   public:
    t_latency() {
      _oldSec     = 0;
      _newSec     = 0;
      _numGaps    = 0;
      _diffSec    = 0;
      _numLat     = 0;
      _sumLat     = 0.0;
      _sumLatQ    = 0.0;
      _meanDiff   = 0.0;
      _minLat     =  1000.0;
      _maxLat     = -1000.0;
      _curLat     = 0.0;
      _type       = "";
      _followSec  = false;
    }
    void init() {
      _diffSec = 0;
      _numGaps    = 0;
      _sumLat     = 0.0;
      _sumLatQ    = 0.0;
      _numLat     = 0;
      _minLat     =  1000.0;
      _maxLat     = -1000.0;
    };
    void print() {
      qDebug() << _type;
      qDebug() << "_oldSec: " << _oldSec;
      qDebug() << "_newSec: " << _newSec;
      qDebug() << "_numGaps: " << _numGaps;
      qDebug() << "_diffSec: " << _diffSec;
      qDebug() << "_numLat: " << _numLat;
      qDebug() << "_sumLat: " << _sumLat;
      qDebug() << "_sumLatQ: " << _sumLatQ;
      qDebug() << "_meanDiff: " << _meanDiff;
      qDebug() << "_minLat: " << _minLat;
      qDebug() << "_maxLat: " << _maxLat;
      qDebug() << "_curLat: " << _curLat;
      qDebug() << "_followSec" << _followSec;
    };
    int        _oldSec;
    int        _newSec;
    int        _numGaps;
    int        _diffSec;
    int        _numLat;
    double     _sumLat;
    double     _sumLatQ;
    double     _meanDiff;
    double     _minLat;
    double     _maxLat;
    double     _curLat;
    QByteArray _type;
    bool _followSec;

  };

  t_latency _lObs;
  t_latency _lOrb;
  t_latency _lClk;
  t_latency _lClkOrb;
  t_latency _lPb;
  t_latency _lCb;
  t_latency _lVtec;
  t_latency _lUra;
  t_latency _lHr;
  void setCurrentLatency(double lat) {
    _curLat = lat;
  }

 signals:
  void newMessage(QByteArray msg, bool showOnScreen);

 private:
  void callScript(const char* comment);
  int        _inspSegm;
  int        _adviseFail;
  int        _adviseReco;
  int        _miscIntr;
  int        _numSucc;
  int        _secSucc;
  int        _secFail;
  int        _initPause;
  int        _currPause;
  bool       _wrongEpoch;
  bool       _checkSeg;
  bool       _begCorrupt;
  bool       _endCorrupt;
  bool       _fromReconnect;
  bool       _fromCorrupt;
  QByteArray _staID;
  QString    _adviseScript;
  QString    _checkMountPoint;
  QString    _begDateCorr;
  QString    _begTimeCorr;
  QString    _begDateOut;
  QString    _begTimeOut;
  QString    _endDateCorr;
  QString    _endTimeCorr;
  QString    _endDateOut;
  QString    _endTimeOut;
  QDateTime  _checkTime;
  QDateTime  _decodeSucc;
  QDateTime  _decodeFailure;
  QDateTime  _decodeStart;
  QDateTime  _decodeStop;
  QDateTime  _decodeStartCorr;
  QDateTime  _decodeStopCorr;
  QDateTime  _checkPause;
  QDateTime  _begDateTimeOut;
  QDateTime  _endDateTimeOut;
  QDateTime  _begDateTimeCorr;
  QDateTime  _endDateTimeCorr;
  double     _curLat;
};

#endif
