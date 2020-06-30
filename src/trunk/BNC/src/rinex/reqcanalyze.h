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

#ifndef REQCANALYZE_H
#define REQCANALYZE_H

#include <QtCore>
#include "rnxobsfile.h"
#include "rnxnavfile.h"
#include "ephemeris.h"
#include "satObs.h"
#include "polarplot.h"

class t_plotData {
 public:
  struct t_hlpStatus {
    QVector<double> _ok;
    QVector<double> _slip;
    QVector<double> _gap;
  };
  QVector<double>         _mjdX24;
  QVector<double>         _numSat;
  QVector<double>         _PDOP;
  QVector<double>         _eleDeg;
  QMap<char, t_hlpStatus> _status;
};

class t_reqcAnalyze : public QThread {
Q_OBJECT

 public:
  t_reqcAnalyze(QObject* parent);
  virtual void run();

 protected:
  ~t_reqcAnalyze();

  class t_skyPlotData {
   public:
    t_skyPlotData() {
      _sys  = ' ';
      _data = new QVector<t_polarPoint*>;
    }
    t_skyPlotData(char sys) {
      _sys  = sys;
      _data = new QVector<t_polarPoint*>;
    }
    ~t_skyPlotData() {}
    char                    _sys;
    QString                 _title;
    QVector<t_polarPoint*>* _data;
  };

 signals:
  void finished();
  void dspSkyPlot(const QString&, QVector<t_skyPlotData> skyPlotData,
                  const QByteArray&, double);
  void dspAvailPlot(const QString&, const QByteArray&);

 private:

  class t_qcFrq {
   public:
    t_qcFrq() {
      _phaseValid = false;
      _codeValid  = false;
      _slip       = false;
      _gap        = false;
      _setMP      = false;
      _rawMP      = 0.0;
      _stdMP      = 0.0;
      _SNR        = 0.0;
    }
    QString _rnxType2ch;
    bool    _phaseValid;
    bool    _codeValid;
    bool    _slip;
    bool    _gap;
    bool    _setMP;
    double  _rawMP;
    double  _stdMP;
    double  _SNR;
  };

  class t_qcSat {
   public:
    t_qcSat() {
      _slotSet = false;
      _eleSet  = false;
      _slotNum = 0;
      _eleDeg  = 0.0;
      _azDeg   = 0.0;
    }
    bool             _slotSet;
    bool             _eleSet;
    int              _slotNum;
    double           _eleDeg;
    double           _azDeg;
    QVector<t_qcFrq> _qcFrq;
  };

  class t_qcEpo {
   public:
    bncTime              _epoTime;
    double               _PDOP;
    QMap<t_prn, t_qcSat> _qcSat;
  };

  class t_qcFrqSum {
   public:
    t_qcFrqSum() {
      _numObs          = 0;
      _numSlipsFlagged = 0;
      _numSlipsFound   = 0;
      _numGaps         = 0;
      _numSNR          = 0;
      _sumSNR          = 0.0;
      _numMP           = 0;
      _sumMP           = 0.0;
    }
    int    _numObs;
    int    _numSlipsFlagged;
    int    _numSlipsFound;
    int    _numGaps;
    int    _numSNR;
    double _sumSNR;
    int    _numMP;
    double _sumMP;
  };

  class t_qcSatSum {
   public:
    QMap<QString, t_qcFrqSum> _qcFrqSum;
  };

  class t_qcFile {
   public:
    t_qcFile() {
      clear();
      _interval = 1.0;
    }
    void clear() {_qcSatSum.clear(); _qcEpo.clear();}
    bncTime                 _startTime;
    bncTime                 _endTime;
    QString                 _antennaName;
    QString                 _markerName;
    QString                 _receiverType;
    double                  _interval;
    QMap<t_prn, t_qcSatSum> _qcSatSum;
    QVector<t_qcEpo>        _qcEpo;
  };

 private slots:
  void   slotDspSkyPlot(const QString& fileName, QVector<t_skyPlotData> skyPlotData,
                        const QByteArray& scaleTitle, double maxValue);

  void   slotDspAvailPlot(const QString& fileName, const QByteArray& title);

 private:
  void   checkEphemerides();

  void   analyzePlotSignals();

  void   analyzeFile(t_rnxObsFile* obsFile);

  void   updateQcSat(const t_qcSat& qcSat, t_qcSatSum& qcSatSum);

  void   setQcObs(const bncTime& epoTime, const ColumnVector& xyzSta,
                  const t_satObs& satObs, QMap<QString, bncTime>& lastObsTime, t_qcSat& qcSat);

  void   setExpectedObs(const bncTime& startTime, const bncTime& endTime,
                        double interval, const ColumnVector& xyzSta);

  void   analyzeMultipath();

  void   preparePlotData(const t_rnxObsFile* obsFile);

  double cmpDOP(const ColumnVector& xyzSta) const;

  void   printReport(const t_rnxObsFile* obsFile);

  static bool mpLessThan(const t_polarPoint* p1, const t_polarPoint* p2);
    
  QString                    _logFileName;
  QFile*                     _logFile;
  QTextStream*               _log;
  QStringList                _obsFileNames;
  QVector<t_rnxObsFile*>     _rnxObsFiles;
  QStringList                _navFileNames;
  QString                    _reqcPlotSignals;
  QMap<char, QVector<char> > _signalTypes;
  QMap<t_prn, int>           _numExpObs;
  QVector<char>              _navFileIncomplete;
  QStringList                _defaultSignalTypes;
  QVector<t_eph*>            _ephs;
  t_rnxObsFile::t_rnxEpo*    _currEpo;
  t_qcFile                   _qcFile;
};

#endif
