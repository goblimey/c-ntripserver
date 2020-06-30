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

/* -------------------------------------------------------------------------
 * BKG NTRIP Client
 * -------------------------------------------------------------------------
 *
 * Class:      t_reqcAnalyze
 *
 * Purpose:    Analyze RINEX Files
 *
 * Author:     L. Mervart
 *
 * Created:    11-Apr-2012
 *
 * Changes:
 *
 * -----------------------------------------------------------------------*/

#include <iostream>
#include <iomanip>
#include <qwt_plot_renderer.h>

#include "reqcanalyze.h"
#include "bnccore.h"
#include "bncsettings.h"
#include "reqcedit.h"
#include "bncutils.h"
#include "graphwin.h"
#include "availplot.h"
#include "eleplot.h"
#include "dopplot.h"
#include "bncephuser.h"

using namespace std;

// Constructor
////////////////////////////////////////////////////////////////////////////
t_reqcAnalyze::t_reqcAnalyze(QObject* parent) : QThread(parent) {

  bncSettings settings;

  _logFileName     = settings.value("reqcOutLogFile").toString(); expandEnvVar(_logFileName);
  _logFile         = 0;
  _log             = 0;
  _currEpo         = 0;
  _obsFileNames    = settings.value("reqcObsFile").toString().split(",", QString::SkipEmptyParts);
  _navFileNames    = settings.value("reqcNavFile").toString().split(",", QString::SkipEmptyParts);
  _reqcPlotSignals = settings.value("reqcSkyPlotSignals").toString();
  _defaultSignalTypes << "G:1&2&5" << "R:1&2&3" << "J:1&2" << "E:1&5" << "S:1&5" << "C:2&7" << "I:5&9";
  if (_reqcPlotSignals.isEmpty()) {
    _reqcPlotSignals = _defaultSignalTypes.join(" ");
  }
  analyzePlotSignals();

  qRegisterMetaType< QVector<t_skyPlotData> >("QVector<t_skyPlotData>");

  connect(this, SIGNAL(dspSkyPlot(const QString&, QVector<t_skyPlotData>,
                                  const QByteArray&, double)),
          this, SLOT(slotDspSkyPlot(const QString&, QVector<t_skyPlotData>,
                                    const QByteArray&, double)));

  connect(this, SIGNAL(dspAvailPlot(const QString&, const QByteArray&)),
          this, SLOT(slotDspAvailPlot(const QString&, const QByteArray&)));
}

// Destructor
////////////////////////////////////////////////////////////////////////////
t_reqcAnalyze::~t_reqcAnalyze() {
  for (int ii = 0; ii < _rnxObsFiles.size(); ii++) {
    delete _rnxObsFiles[ii];
  }
  for (int ii = 0; ii < _ephs.size(); ii++) {
    delete _ephs[ii];
  }
  delete _log;     _log     = 0;
  delete _logFile; _logFile = 0;
  if (BNC_CORE->mode() != t_bncCore::interactive) {
    qApp->exit(0);
    msleep(100); //sleep 0.1 sec
  }
}

//
////////////////////////////////////////////////////////////////////////////
void t_reqcAnalyze::run() {

  static const double QC_FORMAT_VERSION = 1.1;

  // Open Log File
  // -------------
  if (!_logFileName.isEmpty()) {
    _logFile = new QFile(_logFileName);
    if (_logFile->open(QIODevice::WriteOnly | QIODevice::Text)) {
      _log = new QTextStream();
      _log->setDevice(_logFile);
    }
  }

  if (_log) {
    *_log << "QC Format Version  : " << QString("%1").arg(QC_FORMAT_VERSION,3,'f',1)  << endl << endl;
  }

  // Check Ephemerides
  // -----------------
  checkEphemerides();

  // Initialize RINEX Observation Files
  // ----------------------------------
  t_reqcEdit::initRnxObsFiles(_obsFileNames, _rnxObsFiles, _log);

  // Read Ephemerides
  // ----------------
  t_reqcEdit::readEphemerides(_navFileNames, _ephs);

  // Loop over all RINEX Files
  // -------------------------
  for (int ii = 0; ii < _rnxObsFiles.size(); ii++) {
    analyzeFile(_rnxObsFiles[ii]);
  }

  // Exit
  // ----
  emit finished();
  deleteLater();
}

//
////////////////////////////////////////////////////////////////////////////
void t_reqcAnalyze::analyzePlotSignals() {

  QStringList signalsOpt = _reqcPlotSignals.split(" ", QString::SkipEmptyParts);

  for (int ii = 0; ii < signalsOpt.size(); ii++) {
    QStringList input = signalsOpt.at(ii).split(QRegExp("[:&]"), QString::SkipEmptyParts);
    if (input.size() > 1 && input[0].length() == 1) {
      char        system   = input[0].toLatin1().constData()[0];
      QStringList sysValid = _defaultSignalTypes.filter(QString(system));
      if (!sysValid.isEmpty()) {
        for (int iSig = 1; iSig < input.size(); iSig++) {
          if (input[iSig].length() == 1 && input[iSig][0].isDigit()) {
            _signalTypes[system].append(input[iSig][0].toLatin1());
          }
        }
      }
    }
  }
}

//
////////////////////////////////////////////////////////////////////////////
void t_reqcAnalyze::analyzeFile(t_rnxObsFile* obsFile) {

  _qcFile.clear();

  // A priori Coordinates
  // --------------------
  ColumnVector xyzSta = obsFile->xyz();

  // Loop over all Epochs
  // --------------------
  try {
    QMap<QString, bncTime> lastObsTime;
    bool firstEpo = true;
    while ( (_currEpo = obsFile->nextEpoch()) != 0) {
      if (firstEpo) {
        firstEpo = false;
        _qcFile._startTime    = _currEpo->tt;
        _qcFile._antennaName  = obsFile->antennaName();
        _qcFile._markerName   = obsFile->markerName();
        _qcFile._receiverType = obsFile->receiverType();
        _qcFile._interval     = obsFile->interval();
      }
      _qcFile._endTime = _currEpo->tt;

      t_qcEpo qcEpo;
      qcEpo._epoTime = _currEpo->tt;
      qcEpo._PDOP    = cmpDOP(xyzSta);

      // Loop over all satellites
      // ------------------------
      for (unsigned iObs = 0; iObs < _currEpo->rnxSat.size(); iObs++) {
        const t_rnxObsFile::t_rnxSat& rnxSat = _currEpo->rnxSat[iObs];
        if (_navFileNames.size() &&
            _numExpObs.find(rnxSat.prn) == _numExpObs.end()) {
          _numExpObs[rnxSat.prn] = 0;
        }
        if (_signalTypes.find(rnxSat.prn.system()) == _signalTypes.end()) {
          continue;
        }
        t_satObs satObs;
        t_rnxObsFile::setObsFromRnx(obsFile, _currEpo, rnxSat, satObs);
        t_qcSat& qcSat = qcEpo._qcSat[satObs._prn];
        setQcObs(qcEpo._epoTime, xyzSta, satObs, lastObsTime, qcSat);
        updateQcSat(qcSat, _qcFile._qcSatSum[satObs._prn]);
      }
      _qcFile._qcEpo.push_back(qcEpo);
    }

    analyzeMultipath();

    if (_navFileNames.size()) {
      setExpectedObs(_qcFile._startTime, _qcFile._endTime, _qcFile._interval, xyzSta);
    }

    preparePlotData(obsFile);

    printReport(obsFile);
  }
  catch (QString str) {
    if (_log) {
      *_log << "Exception " << str << endl;
    }
    else {
      qDebug() << str;
    }
  }
}

// Compute Dilution of Precision
////////////////////////////////////////////////////////////////////////////
double t_reqcAnalyze::cmpDOP(const ColumnVector& xyzSta) const {

  if ( xyzSta.size() != 3 || (xyzSta[0] == 0.0 && xyzSta[1] == 0.0 && xyzSta[2] == 0.0) ) {
    return 0.0;
  }

  unsigned nSat = _currEpo->rnxSat.size();

  if (nSat < 4) {
    return 0.0;
  }

  Matrix AA(nSat, 4);

  unsigned nSatUsed = 0;
  for (unsigned iSat = 0; iSat < nSat; iSat++) {

    const t_rnxObsFile::t_rnxSat& rnxSat = _currEpo->rnxSat[iSat];
    const t_prn& prn = rnxSat.prn;

    if (_signalTypes.find(prn.system()) == _signalTypes.end()) {
      continue;
    }

    t_eph* eph = 0;
    for (int ie = 0; ie < _ephs.size(); ie++) {
      if (_ephs[ie]->prn() == prn) {
        eph = _ephs[ie];
        break;
      }
    }
    if (eph) {
      ColumnVector xSat(6);
      ColumnVector vv(3);
      if (eph->getCrd(_currEpo->tt, xSat, vv, false) == success) {
        ++nSatUsed;
        ColumnVector dx = xSat.Rows(1,3) - xyzSta;
        double rho = dx.NormFrobenius();
        AA(nSatUsed,1) = dx(1) / rho;
        AA(nSatUsed,2) = dx(2) / rho;
        AA(nSatUsed,3) = dx(3) / rho;
        AA(nSatUsed,4) = 1.0;
      }
    }
  }

  if (nSatUsed < 4) {
    return 0.0;
  }

  AA = AA.Rows(1, nSatUsed);

  SymmetricMatrix QQ;
  QQ << AA.t() * AA;
  QQ = QQ.i();

  return sqrt(QQ.trace());
}

//
////////////////////////////////////////////////////////////////////////////
void t_reqcAnalyze::updateQcSat(const t_qcSat& qcSat, t_qcSatSum& qcSatSum) {

  for (int ii = 0; ii < qcSat._qcFrq.size(); ii++) {
    const t_qcFrq& qcFrq    = qcSat._qcFrq[ii];
    t_qcFrqSum&    qcFrqSum = qcSatSum._qcFrqSum[qcFrq._rnxType2ch];
    qcFrqSum._numObs += 1;
    if (qcFrq._slip) {
      qcFrqSum._numSlipsFlagged += 1;
    }
    if (qcFrq._gap) {
      qcFrqSum._numGaps += 1;
    }
    if (qcFrq._SNR > 0.0) {
      qcFrqSum._numSNR += 1;
      qcFrqSum._sumSNR += qcFrq._SNR;
    }
  }
}

//
////////////////////////////////////////////////////////////////////////////
void t_reqcAnalyze::setQcObs(const bncTime& epoTime, const ColumnVector& xyzSta,
                             const t_satObs& satObs, QMap<QString, bncTime>& lastObsTime,
                             t_qcSat& qcSat) {

  t_eph* eph = 0;
  for (int ie = 0; ie < _ephs.size(); ie++) {
    if (_ephs[ie]->prn().system() == satObs._prn.system() &&
        _ephs[ie]->prn().number() == satObs._prn.number()) {
      eph = _ephs[ie];
      break;
    }
  }
  if (eph) {
    ColumnVector xc(6);
    ColumnVector vv(3);
    if ( xyzSta.size() == 3 && (xyzSta[0] != 0.0 || xyzSta[1] != 0.0 || xyzSta[2] != 0.0) &&
         eph->getCrd(epoTime, xc, vv, false) == success) {
      double rho, eleSat, azSat;
      topos(xyzSta(1), xyzSta(2), xyzSta(3), xc(1), xc(2), xc(3), rho, eleSat, azSat);
      qcSat._eleSet = true;
      qcSat._azDeg  = azSat  * 180.0/M_PI;
      qcSat._eleDeg = eleSat * 180.0/M_PI;
    }
    if (satObs._prn.system() == 'R') {
      qcSat._slotSet = true;
      qcSat._slotNum = eph->slotNum();
    }
  }

  // Availability and Slip Flags
  // ---------------------------
  for (unsigned ii = 0; ii < satObs._obs.size(); ii++) {
    const t_frqObs* frqObs = satObs._obs[ii];

    qcSat._qcFrq.push_back(t_qcFrq());
    t_qcFrq& qcFrq = qcSat._qcFrq.back();

    qcFrq._rnxType2ch = QString(frqObs->_rnxType2ch.c_str());
    qcFrq._SNR        = frqObs->_snr;
    qcFrq._slip       = frqObs->_slip;
    qcFrq._phaseValid = frqObs->_phaseValid;
    qcFrq._codeValid  = frqObs->_codeValid;
    // Check Gaps
    // ----------
    QString key = QString(satObs._prn.toString().c_str()) + qcFrq._rnxType2ch;
    if (lastObsTime[key].valid()) {
      double dt = epoTime - lastObsTime[key];
      if (dt > 1.5 * _qcFile._interval) {
        qcFrq._gap = true;
      }
    }
    lastObsTime[key] = epoTime;

    // Compute the Multipath Linear Combination
    // ----------------------------------------
    if (frqObs->_codeValid) {
      t_frequency::type fA = t_frequency::dummy;
      t_frequency::type fB = t_frequency::dummy;
      char sys             = satObs._prn.system();
      if (_signalTypes.find(sys) != _signalTypes.end()) {
        for (int iSig = 0; iSig < _signalTypes[sys].size(); iSig++) {
          if (frqObs->_rnxType2ch[0] == _signalTypes[sys][iSig]) {
            string frqType; frqType.push_back(sys); frqType.push_back(_signalTypes[sys][iSig]);
            fA = t_frequency::toInt(frqType);
            break;
          }
        }
        if (fA != t_frequency::dummy) {
          for (int iSig = 0; iSig < _signalTypes[sys].size(); iSig++) {
            string frqType; frqType.push_back(sys); frqType.push_back(_signalTypes[sys][iSig]);
            t_frequency::type fHlp = t_frequency::toInt(frqType);
            if (fA != fHlp) {
              fB = fHlp;
              break;
            }
          }
        }
        if (fA != t_frequency::dummy && fB != t_frequency::dummy) {

          double f_a = t_CST::freq(fA, qcSat._slotNum);
          double f_b = t_CST::freq(fB, qcSat._slotNum);
          double C_a = frqObs->_code;

          bool   foundA = false;
          double L_a    = 0.0;
          bool   foundB = false;
          double L_b    = 0.0;
          for (unsigned jj = 0; jj < satObs._obs.size(); jj++) {
            const t_frqObs* frqObsHlp = satObs._obs[jj];
            if      (frqObsHlp->_rnxType2ch[0] == t_frequency::toString(fA)[1] &&
                frqObsHlp->_phaseValid) {
              foundA = true;
              L_a    = frqObsHlp->_phase * t_CST::c / f_a;
            }
            else if (frqObsHlp->_rnxType2ch[0] == t_frequency::toString(fB)[1] &&
                frqObsHlp->_phaseValid) {
              foundB = true;
              L_b    = frqObsHlp->_phase * t_CST::c / f_b;
            }
          }
          if (foundA && foundB) {
            qcFrq._setMP = true;
            qcFrq._rawMP = C_a - L_a - 2.0*f_b*f_b/(f_a*f_a-f_b*f_b) * (L_a - L_b);
          }
        }
      }
    }
  } // satObs loop
}

//
////////////////////////////////////////////////////////////////////////////
void t_reqcAnalyze::analyzeMultipath() {

  const double SLIPTRESH = 10.0;  // cycle-slip threshold (meters)
  const double chunkStep = 600.0; // 10 minutes

  // Loop over all satellites available
  // ----------------------------------
  QMutableMapIterator<t_prn, t_qcSatSum> itSat(_qcFile._qcSatSum);
  while (itSat.hasNext()) {
    itSat.next();
    const t_prn& prn      = itSat.key();
    t_qcSatSum&  qcSatSum = itSat.value();

    // Loop over all frequencies available
    // -----------------------------------
    QMutableMapIterator<QString, t_qcFrqSum> itFrq(qcSatSum._qcFrqSum);
    while (itFrq.hasNext()) {
      itFrq.next();
      const QString& frqType  = itFrq.key();
      t_qcFrqSum&    qcFrqSum = itFrq.value();

      // Loop over all Chunks of Data
      // ----------------------------
      for (bncTime chunkStart = _qcFile._startTime;
           chunkStart < _qcFile._endTime; chunkStart += chunkStep) {

        bncTime chunkEnd = chunkStart + chunkStep;

        QVector<t_qcFrq*> frqVec;
        QVector<double>   MP;

        // Loop over all Epochs within one Chunk of Data
        // ---------------------------------------------
        for (int iEpo = 0; iEpo < _qcFile._qcEpo.size(); iEpo++) {
          t_qcEpo& qcEpo = _qcFile._qcEpo[iEpo];
          if (chunkStart <= qcEpo._epoTime && qcEpo._epoTime < chunkEnd) {
            if (qcEpo._qcSat.contains(prn)) {
              t_qcSat& qcSat = qcEpo._qcSat[prn];
              for (int iFrq = 0; iFrq < qcSat._qcFrq.size(); iFrq++) {
                t_qcFrq& qcFrq = qcSat._qcFrq[iFrq];
                if (qcFrq._rnxType2ch == frqType) {
                  frqVec << &qcFrq;
                  if (qcFrq._setMP) {
                    MP << qcFrq._rawMP;
                  }
                }
              }
            }
          }
        }

        // Compute the multipath mean and standard deviation
        // -------------------------------------------------
        if (MP.size() > 1) {
          double meanMP = 0.0;
          for (int ii = 0; ii < MP.size(); ii++) {
            meanMP += MP[ii];
          }
          meanMP /= MP.size();

          bool slipMP = false;

          double stdMP = 0.0;
          for (int ii = 0; ii < MP.size(); ii++) {
            double diff = MP[ii] - meanMP;
            if (fabs(diff) > SLIPTRESH) {
              slipMP = true;
              break;
            }
            stdMP += diff * diff;
          }

          if (slipMP) {
            stdMP = 0.0;
            stdMP = 0.0;
            qcFrqSum._numSlipsFound += 1;
          }
          else {
            stdMP = sqrt(stdMP / (MP.size()-1));
            qcFrqSum._numMP += 1;
            qcFrqSum._sumMP += stdMP;
          }

          for (int ii = 0; ii < frqVec.size(); ii++) {
            t_qcFrq* qcFrq = frqVec[ii];
            if (slipMP) {
              qcFrq->_slip = true;
            }
            else {
              qcFrq->_stdMP = stdMP;
            }
          }
        }
      } // chunk loop
    } // frq loop
  } // sat loop
}

// Auxiliary Function for sorting MP data
////////////////////////////////////////////////////////////////////////////
bool t_reqcAnalyze::mpLessThan(const t_polarPoint* p1, const t_polarPoint* p2) {
  return p1->_value < p2->_value;
}

//
////////////////////////////////////////////////////////////////////////////
void t_reqcAnalyze::preparePlotData(const t_rnxObsFile* obsFile) {

  if (!BNC_CORE->GUIenabled()) {
    return ;
  }

  QVector<t_skyPlotData> skyPlotDataMP;
  QVector<t_skyPlotData> skyPlotDataSN;

  for(QMap<char, QVector<char> >::iterator it1 = _signalTypes.begin();
      it1 != _signalTypes.end(); it1++) {

    char sys = it1.key();

    for (int ii = 0; ii < it1.value().size(); ii++) {

      skyPlotDataMP.append(t_skyPlotData(sys));  t_skyPlotData& dataMP = skyPlotDataMP.last();
      skyPlotDataSN.append(t_skyPlotData(sys));  t_skyPlotData& dataSN = skyPlotDataSN.last();

      dataMP._title = "Multipath\n"             + QString(it1.key()) + ":" + it1.value()[ii] + " ";
      dataSN._title = "Signal-to-Noise Ratio\n" + QString(it1.key()) + ":" + it1.value()[ii] + " ";

      // Loop over all observations
      // --------------------------
      for (int iEpo = 0; iEpo < _qcFile._qcEpo.size(); iEpo++) {
        t_qcEpo& qcEpo = _qcFile._qcEpo[iEpo];
        QMapIterator<t_prn, t_qcSat> it2(qcEpo._qcSat);
        while (it2.hasNext()) {
          it2.next();
          const t_qcSat& qcSat = it2.value();
          if (qcSat._eleSet && it2.key().system() == sys) {
            for (int iFrq = 0; iFrq < qcSat._qcFrq.size(); iFrq++) {
              const t_qcFrq& qcFrq = qcSat._qcFrq[iFrq];
              if (QString(it1.value()[ii]) == qcFrq._rnxType2ch.left(1)) {
                (*dataMP._data) << (new t_polarPoint(qcSat._azDeg, 90.0 - qcSat._eleDeg, qcFrq._stdMP));
                (*dataSN._data) << (new t_polarPoint(qcSat._azDeg, 90.0 - qcSat._eleDeg, qcFrq._SNR));
              }
            }
          }
        }
      }

      // Sort MP data (make the largest values always visible)
      // -----------------------------------------------------
      qStableSort(dataMP._data->begin(), dataMP._data->end(), mpLessThan);
    }
  }

  // Show the plots
  // --------------
  QFileInfo  fileInfo(obsFile->fileName());
  QByteArray title = fileInfo.fileName().toLatin1();
  emit dspSkyPlot(obsFile->fileName(), skyPlotDataMP, "Meters",  1.0);
  emit dspSkyPlot(obsFile->fileName(), skyPlotDataSN, "dbHz",   54.0);
  emit dspAvailPlot(obsFile->fileName(), title);
}

//
////////////////////////////////////////////////////////////////////////////
void t_reqcAnalyze::slotDspSkyPlot(const QString& fileName,
                                   QVector<t_skyPlotData> skyPlotData,
                                   const QByteArray& scaleTitle, double maxValue) {

  if (BNC_CORE->GUIenabled()) {

    if (maxValue == 0.0) {
      QVectorIterator<t_skyPlotData> it(skyPlotData);
      while (it.hasNext()) {
        const t_skyPlotData&          plotData = it.next();
        const QVector<t_polarPoint*>* data     = plotData._data;
        for (int ii = 0; ii < data->size(); ii++) {
          double val = data->at(ii)->_value;
          if (maxValue < val) {
            maxValue = val;
          }
        }
      }
    }

    QwtInterval scaleInterval(0.0, maxValue);

    QVector<QWidget*> plots;
    QVector<int>      rows;
    QMutableVectorIterator<t_skyPlotData> it(skyPlotData);
    int iRow = 0;
    char sys = ' ';
    while (it.hasNext()) {
      t_skyPlotData&          plotData = it.next();
      QVector<t_polarPoint*>* data     = plotData._data;
      QwtText title(plotData._title);
      QFont font = title.font(); font.setPointSize(font.pointSize()-1); title.setFont(font);
      t_polarPlot* plot = new t_polarPlot(title, scaleInterval, BNC_CORE->mainWindow());
      plot->addCurve(data);
      plots << plot;
      if (sys != ' ' && sys != plotData._sys) {
        iRow += 1;
      }
      sys = plotData._sys;
      rows  << iRow;
    }

    t_graphWin* graphWin = new t_graphWin(0, fileName, plots, false,
                                          &scaleTitle, &scaleInterval, &rows);

    graphWin->show();

    bncSettings settings;
    QString dirName = settings.value("reqcPlotDir").toString();
    if (!dirName.isEmpty()) {
      QByteArray ext = (scaleTitle == "Meters") ? "_M.png" : "_S.png";
      graphWin->savePNG(dirName, ext);
    }
  }
}

//
////////////////////////////////////////////////////////////////////////////
void t_reqcAnalyze::slotDspAvailPlot(const QString& fileName, const QByteArray& title) {

  t_plotData              plotData;
  QMap<t_prn, t_plotData> plotDataMap;

  for (int ii = 0; ii < _qcFile._qcEpo.size(); ii++) {
    const t_qcEpo& qcEpo = _qcFile._qcEpo[ii];
    double mjdX24 = qcEpo._epoTime.mjddec() * 24.0;

    plotData._mjdX24 << mjdX24;
    plotData._PDOP   << qcEpo._PDOP;
    plotData._numSat << qcEpo._qcSat.size();

    QMapIterator<t_prn, t_qcSat> it(qcEpo._qcSat);
    while (it.hasNext()) {
      it.next();
      const t_prn&   prn   = it.key();
      const t_qcSat& qcSat = it.value();

      t_plotData&    data  = plotDataMap[prn];

      if (qcSat._eleSet) {
        data._mjdX24 << mjdX24;
        data._eleDeg << qcSat._eleDeg;
      }

      for (int iSig = 0; iSig < _signalTypes[prn.system()].size(); iSig++) {
        char    frqChar = _signalTypes[prn.system()][iSig];
        QString frqType;
        for (int iFrq = 0; iFrq < qcSat._qcFrq.size(); iFrq++) {
          const t_qcFrq& qcFrq = qcSat._qcFrq[iFrq];
          if (qcFrq._rnxType2ch[0] == frqChar && frqType.isEmpty()) {
            frqType = qcFrq._rnxType2ch;
          }
          if      (qcFrq._rnxType2ch == frqType) {
            t_plotData::t_hlpStatus& status = data._status[frqChar];
            if      (qcFrq._slip) {
              status._slip << mjdX24;
            }
            else if (qcFrq._gap) {
              status._gap << mjdX24;
            }
            else {
              status._ok << mjdX24;
            }
          }
        }
      }
    }
  }

  if (BNC_CORE->GUIenabled()) {
    t_availPlot* plotA = new t_availPlot(0, plotDataMap);
    plotA->setTitle(title);

    t_elePlot* plotZ = new t_elePlot(0, plotDataMap);

    t_dopPlot* plotD = new t_dopPlot(0, plotData);

    QVector<QWidget*> plots;
    plots << plotA << plotZ << plotD;
    t_graphWin* graphWin = new t_graphWin(0, fileName, plots, true);

    int ww = QFontMetrics(graphWin->font()).width('w');
    graphWin->setMinimumSize(120*ww, 40*ww);

    graphWin->show();

    bncSettings settings;
    QString dirName = settings.value("reqcPlotDir").toString();
    if (!dirName.isEmpty()) {
      QByteArray ext = "_A.png";
      graphWin->savePNG(dirName, ext);
    }
  }
}

// Finish the report
////////////////////////////////////////////////////////////////////////////
void t_reqcAnalyze::printReport(const t_rnxObsFile* obsFile) {

  if (!_log) {
    return;
  }

  QFileInfo obsFi(obsFile->fileName());
  QString obsFileName = obsFi.fileName();

  // Summary
  // -------
  *_log << "Observation File   : " << obsFileName                                   << endl
        << "RINEX Version      : " << QString("%1").arg(obsFile->version(),4,'f',2) << endl
        << "Marker Name        : " << _qcFile._markerName                           << endl
        << "Marker Number      : " << obsFile->markerNumber()                       << endl
        << "Receiver           : " << _qcFile._receiverType                         << endl
        << "Antenna            : " << _qcFile._antennaName                          << endl
        << "Position XYZ       : " << QString("%1 %2 %3").arg(obsFile->xyz()(1), 14, 'f', 4)
                                                        .arg(obsFile->xyz()(2), 14, 'f', 4)
                                                        .arg(obsFile->xyz()(3), 14, 'f', 4) << endl
        << "Antenna dH/dE/dN   : " << QString("%1 %2 %3").arg(obsFile->antNEU()(3), 8, 'f', 4)
                                                        .arg(obsFile->antNEU()(2), 8, 'f', 4)
                                                        .arg(obsFile->antNEU()(1), 8, 'f', 4) << endl
        << "Start Time         : " << _qcFile._startTime.datestr().c_str()         << ' '
                                   << _qcFile._startTime.timestr(1,'.').c_str()    << endl
        << "End Time           : " << _qcFile._endTime.datestr().c_str()           << ' '
                                   << _qcFile._endTime.timestr(1,'.').c_str()      << endl
        << "Interval           : " << _qcFile._interval << " sec"                  << endl;

  // Number of systems
  // -----------------
  QMap<QChar, QVector<const t_qcSatSum*> > systemMap;
  QMapIterator<t_prn, t_qcSatSum> itSat(_qcFile._qcSatSum);
  while (itSat.hasNext()) {
    itSat.next();
    const t_prn&      prn      = itSat.key();
    const t_qcSatSum& qcSatSum = itSat.value();
    systemMap[prn.system()].push_back(&qcSatSum);
  }
  *_log << "Navigation Systems : " << systemMap.size() << "   ";

  QMapIterator<QChar, QVector<const t_qcSatSum*> > itSys(systemMap);
  while (itSys.hasNext()) {
    itSys.next();
    *_log << ' ' << itSys.key();
  }
  *_log << endl;

  // Observation types per system
  // -----------------------------
  for (int iSys = 0; iSys < obsFile->numSys(); iSys++) {
    char sys = obsFile->system(iSys);
    if (sys != ' ') {
      *_log << "Observation Types " << sys << ":";
      for (int iType = 0; iType < obsFile->nTypes(sys); iType++) {
        QString type = obsFile->obsType(sys, iType);
        *_log << " " << type;
      }
      *_log << endl;
    }
  }

  // System specific summary
  // -----------------------
  itSys.toFront();
  while (itSys.hasNext()) {
    itSys.next();
    const QChar&                      sys      = itSys.key();
    const QVector<const t_qcSatSum*>& qcSatVec = itSys.value();
    int numExpectedObs = 0;
    for(QMap<t_prn, int>::iterator it = _numExpObs.begin();
        it != _numExpObs.end(); it++) {
      if (sys == it.key().system()) {
        numExpectedObs += it.value();
      }
    }
    QString prefixSys = QString("  ") + sys + QString(": ");
    QMap<QString, QVector<const t_qcFrqSum*> > frqMap;
    for (int ii = 0; ii < qcSatVec.size(); ii++) {
      const t_qcSatSum* qcSatSum = qcSatVec[ii];
      QMapIterator<QString, t_qcFrqSum> itFrq(qcSatSum->_qcFrqSum);
      while (itFrq.hasNext()) {
        itFrq.next();
        QString           frqType  = itFrq.key(); if (frqType.length() < 2) frqType += '?';
        const t_qcFrqSum& qcFrqSum = itFrq.value();
        frqMap[frqType].push_back(&qcFrqSum);
      }
    }
    *_log << endl
          << prefixSys << "Satellites: " << qcSatVec.size() << endl
          << prefixSys << "Signals   : " << frqMap.size() << "   ";
    QMapIterator<QString, QVector<const t_qcFrqSum*> > itFrq(frqMap);
    while (itFrq.hasNext()) {
      itFrq.next();
      QString frqType = itFrq.key(); if (frqType.length() < 2) frqType += '?';
      *_log << ' ' << frqType;
    }
    *_log << endl;
    QString prefixSys2 = "    " + prefixSys;
    itFrq.toFront();
    while (itFrq.hasNext()) {
      itFrq.next();
      QString                          frqType  = itFrq.key(); if (frqType.length() < 2) frqType += '?';
      const QVector<const t_qcFrqSum*> qcFrqVec = itFrq.value();
      QString prefixFrq = QString("  ") + frqType + QString(": ");
      int    numObs          = 0;
      int    numSlipsFlagged = 0;
      int    numSlipsFound   = 0;
      int    numGaps         = 0;
      int    numSNR          = 0;
      int    numMP           = 0;
      double sumSNR          = 0.0;
      double sumMP           = 0.0;
      for (int ii = 0; ii < qcFrqVec.size(); ii++) {
        const t_qcFrqSum* qcFrqSum = qcFrqVec[ii];
        numObs          += qcFrqSum->_numObs         ;
        numSlipsFlagged += qcFrqSum->_numSlipsFlagged;
        numSlipsFound   += qcFrqSum->_numSlipsFound  ;
        numGaps         += qcFrqSum->_numGaps        ;
        numSNR          += qcFrqSum->_numSNR;
        numMP           += qcFrqSum->_numMP;
        sumSNR          += qcFrqSum->_sumSNR;
        sumMP           += qcFrqSum->_sumMP;
      }
      if (numSNR > 0) {
        sumSNR /= numSNR;
      }
      if (numMP > 0) {
        sumMP /= numMP;
      }

      double ratio = (double(numObs) / double(numExpectedObs)) * 100.0;

      *_log << endl
            << prefixSys2 << prefixFrq << "Observations      : ";
      if(_navFileNames.isEmpty() || _navFileIncomplete.contains(sys.toLatin1())) {
        *_log << QString("%1\n").arg(numObs,           6);
      }
      else {
        *_log << QString("%1 (%2) %3 \%\n").arg(numObs,           6).arg(numExpectedObs,           8).arg(ratio, 8, 'f', 2);
      }
      *_log << prefixSys2 << prefixFrq << "Slips (file+found): " << QString("%1 +").arg(numSlipsFlagged,  8)
                                                                 << QString("%1\n").arg(numSlipsFound,    8)
            << prefixSys2 << prefixFrq << "Gaps              : " << QString("%1\n").arg(numGaps,          8)
            << prefixSys2 << prefixFrq << "Mean SNR          : " << QString("%1\n").arg(sumSNR,   8, 'f', 1)
            << prefixSys2 << prefixFrq << "Mean Multipath    : " << QString("%1\n").arg(sumMP,    8, 'f', 2);
    }
  }

  // Epoch-Specific Output
  // ---------------------
  bncSettings settings;
  if (Qt::CheckState(settings.value("reqcLogSummaryOnly").toInt()) == Qt::Checked) {
    return;
  }
  *_log << endl;
  for (int iEpo = 0; iEpo < _qcFile._qcEpo.size(); iEpo++) {
    const t_qcEpo& qcEpo = _qcFile._qcEpo[iEpo];

    unsigned year, month, day, hour, min;
    double sec;
    qcEpo._epoTime.civil_date(year, month, day);
    qcEpo._epoTime.civil_time(hour, min, sec);

    QString dateStr;
    QTextStream(&dateStr) << QString("> %1 %2 %3 %4 %5%6")
      .arg(year,  4)
      .arg(month, 2, 10, QChar('0'))
      .arg(day,   2, 10, QChar('0'))
      .arg(hour,  2, 10, QChar('0'))
      .arg(min,   2, 10, QChar('0'))
      .arg(sec,  11, 'f', 7);

    *_log << dateStr << QString(" %1").arg(qcEpo._qcSat.size(), 2)
          << QString(" %1").arg(qcEpo._PDOP, 4, 'f', 1)
          << endl;

    QMapIterator<t_prn, t_qcSat> itSat(qcEpo._qcSat);
    while (itSat.hasNext()) {
      itSat.next();
      const t_prn&   prn   = itSat.key();
      const t_qcSat& qcSat = itSat.value();

      *_log << prn.toString().c_str()
            << QString(" %1 %2").arg(qcSat._eleDeg, 6, 'f', 2).arg(qcSat._azDeg, 7, 'f', 2);

      int numObsTypes = 0;
      for (int iFrq = 0; iFrq < qcSat._qcFrq.size(); iFrq++) {
        const t_qcFrq& qcFrq = qcSat._qcFrq[iFrq];
        if (qcFrq._phaseValid) {
          numObsTypes += 1;
        }
        if (qcFrq._codeValid) {
          numObsTypes += 1;
        }
      }
      *_log << QString("  %1").arg(numObsTypes, 2);

      for (int iFrq = 0; iFrq < qcSat._qcFrq.size(); iFrq++) {
        const t_qcFrq& qcFrq = qcSat._qcFrq[iFrq];
        if (qcFrq._phaseValid) {
          *_log << "  L" << qcFrq._rnxType2ch << ' ';
          if (qcFrq._slip) {
            *_log << 's';
          }
          else {
            *_log << '.';
          }
          if (qcFrq._gap) {
            *_log << 'g';
          }
          else {
            *_log << '.';
          }
          *_log << QString(" %1").arg(qcFrq._SNR,   4, 'f', 1);
        }
        if (qcFrq._codeValid) {
          *_log << "  C" << qcFrq._rnxType2ch << ' ';
          if (qcFrq._gap) {
            *_log << " g";
          }
          else {
            *_log << " .";
          }
          *_log << QString(" %1").arg(qcFrq._stdMP, 3, 'f', 2);
        }
      }
      *_log << endl;
    }
  }
  _log->flush();
}

//
////////////////////////////////////////////////////////////////////////////
void t_reqcAnalyze::checkEphemerides() {

  QString navFileName;
  QStringListIterator namIt(_navFileNames);
  bool firstName = true;
  while (namIt.hasNext()) {
    QFileInfo navFi(namIt.next());
    if (firstName) {
      firstName = false;
      navFileName += navFi.fileName();
    }
    else {
      navFileName += ", " + navFi.fileName();
    }
  }
  if (_log) {
    *_log << "Navigation File(s) : " << navFileName                                   << endl;
  }
  QStringListIterator it(_navFileNames);
  while (it.hasNext()) {
    const QString& fileName = it.next();
    unsigned numOK  = 0;
    unsigned numBad = 0;
    bncEphUser ephUser(false);
    t_rnxNavFile rnxNavFile(fileName, t_rnxNavFile::input);
    for (unsigned ii = 0; ii < rnxNavFile.ephs().size(); ii++) {
      t_eph* eph = rnxNavFile.ephs()[ii];
      ephUser.putNewEph(eph, false);
      if (eph->checkState() == t_eph::bad) {
        ++numBad;
      }
      else {
        ++numOK;
      }
    }
    if (_log) {
      *_log << "Ephemeris          : " << numOK << " OK   " << numBad << " BAD" << endl;
    }
    if (numBad > 0) {
      for (unsigned ii = 0; ii < rnxNavFile.ephs().size(); ii++) {
        t_eph* eph = rnxNavFile.ephs()[ii];
        if (eph->checkState() == t_eph::bad) {
          QFileInfo navFi(fileName);
          if (_log) {
            *_log << "  Bad Ephemeris   : " << navFi.fileName() << ' '
                  << eph->toString(3.0).left(24) << endl;
          }
        }
      }
    }
  }
  if (_log) {
    *_log << endl;
  }
}

void t_reqcAnalyze::setExpectedObs(const bncTime& startTime, const bncTime& endTime,
                                   double interval, const ColumnVector& xyzSta) {

  for(QMap<t_prn, int>::iterator it = _numExpObs.begin();
      it != _numExpObs.end(); it++) {
    t_eph* eph = 0;
    for (int ie = 0; ie < _ephs.size(); ie++) {
      if (_ephs[ie]->prn().system() == it.key().system() &&
          _ephs[ie]->prn().number() == it.key().number()) {
        eph = _ephs[ie];
        break;
      }
    }
    if (eph) {
      int numExpObs = 0;
      bncTime epoTime;
      for (epoTime = startTime - interval; epoTime < endTime;
           epoTime = epoTime + interval) {
        ColumnVector xc(6);
        ColumnVector vv(3);
        if ( xyzSta.size() == 3 && (xyzSta[0] != 0.0 || xyzSta[1] != 0.0 || xyzSta[2] != 0.0) &&
             eph->getCrd(epoTime, xc, vv, false) == success) {
          double rho, eleSat, azSat;
          topos(xyzSta(1), xyzSta(2), xyzSta(3), xc(1), xc(2), xc(3), rho, eleSat, azSat);
          if ((eleSat * 180.0/M_PI) > 0.0) {
            numExpObs++;
          }
        }
      }
      it.value() = numExpObs;
    }
    else {
      if (!_navFileIncomplete.contains(it.key().system())) {
        _navFileIncomplete.append(it.key().system());
      }
    }
  }
}
