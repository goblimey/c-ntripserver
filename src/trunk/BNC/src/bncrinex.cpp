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
 * Class:      bncRinex
 *
 * Purpose:    writes RINEX files
 *
 * Author:     L. Mervart
 *
 * Created:    27-Aug-2006
 *
 * Changes:
 *
 * -----------------------------------------------------------------------*/

#include <stdlib.h>
#include <iostream>
#include <iomanip>
#include <math.h>
#include <sstream>

#include <QtCore>
#include <QUrl>
#include <QString>

#include "bncrinex.h"
#include "bnccore.h"
#include "bncutils.h"
#include "bncconst.h"
#include "bnctabledlg.h"
#include "bncgetthread.h"
#include "bncnetqueryv1.h"
#include "bncnetqueryv2.h"
#include "bncsettings.h"
#include "bncversion.h"

using namespace std;

// Constructor
////////////////////////////////////////////////////////////////////////////
bncRinex::bncRinex(const QByteArray& statID, const QUrl& mountPoint,
                   const QByteArray& latitude, const QByteArray& longitude,
                   const QByteArray& nmea, const QByteArray& ntripVersion) {

  _statID        = statID;
  _mountPoint    = mountPoint;
  _latitude      = latitude;
  _longitude     = longitude;
  _nmea          = nmea;
  _ntripVersion  = ntripVersion;
  _headerWritten = false;
  _reconnectFlag = false;

  bncSettings settings;
  _rnxScriptName = settings.value("rnxScript").toString();
  expandEnvVar(_rnxScriptName);

  _pgmName  = QString(BNCPGMNAME).leftJustified(20, ' ', true);
#ifdef WIN32
  _userName = QString("${USERNAME}");
#else
  _userName = QString("${USER}");
#endif
  expandEnvVar(_userName);
  _userName = _userName.leftJustified(20, ' ', true);

  _samplingRateMult10 = int(settings.value("rnxSampl").toString().split("sec").first().toDouble() * 10.0);

  if      (settings.value("rnxSampl").toString() == "0.1 sec") {
    _samplingRateStr = "_10Z";
  }
  else if (settings.value("rnxSampl").toString() == "1 sec") {
    _samplingRateStr = "_01S";
  }
  else if (settings.value("rnxSampl").toString() == "5 sec") {
    _samplingRateStr = "_05S";
  }
  else if (settings.value("rnxSampl").toString() == "10 sec") {
    _samplingRateStr = "_10S";
  }
  else if (settings.value("rnxSampl").toString() == "15 sec") {
    _samplingRateStr = "_15S";
  }
  else if (settings.value("rnxSampl").toString() == "30 sec") {
    _samplingRateStr = "_30S";
  }
  else if (settings.value("rnxSampl").toString() == "60 sec") {
    _samplingRateStr = "_01M";
  }

  _writeRinexFileOnlyWithSkl = settings.value("rnxOnlyWithSKL").toBool();

  _rnxV3 = (Qt::CheckState(settings.value("rnxV3").toInt()) == Qt::Checked) ? true : false;
}

// Destructor
////////////////////////////////////////////////////////////////////////////
bncRinex::~bncRinex() {
  bncSettings settings;
  if ((_header.version() >= 3.0) && ( Qt::CheckState(settings.value("rnxAppend").toInt()) != Qt::Checked) ) {
    _out << ">                              4  1" << endl;
    _out << "END OF FILE" << endl;
  }
  _out.close();
}

// Download Skeleton Header File
////////////////////////////////////////////////////////////////////////////
t_irc bncRinex::downloadSkeleton() {

  t_irc irc = failure;

  QStringList table;
  bncTableDlg::getFullTable(_ntripVersion, _mountPoint.host(), _mountPoint.port(),
      _mountPoint.userName(), _mountPoint.password(), table, true);
  QString net;
  QStringListIterator it(table);
  while (it.hasNext()) {
    QString line = it.next();
    if (line.indexOf("STR") == 0) {
      QStringList tags = line.split(";");
      if (tags.size() > 7) {
        if (tags.at(1) == _mountPoint.path().mid(1).toLatin1()) {
          net = tags.at(7);
          break;
        }
      }
    }
  }
  QString sklDir;
  if (!net.isEmpty()) {
    it.toFront();
    while (it.hasNext()) {
      QString line = it.next();
      if (line.indexOf("NET") == 0) {
        QStringList tags = line.split(";");
        if (tags.size() > 6) {
          if (tags.at(1) == net) {
            sklDir = tags.at(6).trimmed();
            break;
          }
        }
      }
    }
  }
  if (!sklDir.isEmpty() && sklDir != "none") {
    QString staID, staIDalternative;
    QUrl url;
    int stIdLength = _mountPoint.path().length()-2;
    staID =            _mountPoint.path().mid(1,stIdLength).toUpper() + ".skl";
    staIDalternative = _mountPoint.path().mid(1,stIdLength).toLower() + ".skl";
    url = sklDir + "/" + staID;
    if (url.port() == -1) {
      if (sklDir.contains("https", Qt::CaseInsensitive)) {
        url.setPort(443);
      }
      else {
        url.setPort(80);
      }
    }

    bncNetQuery* query = new bncNetQueryV2(true);
    QByteArray outData;
    query->waitForRequestResult(url, outData);
    if (query->status() == bncNetQuery::finished &&
        outData.contains("END OF HEADER")) {
      QTextStream in(outData);
      irc = _sklHeader.read(&in);
    }
    else {
      url = sklDir + "/" + staIDalternative;
      query->waitForRequestResult(url, outData);
      if (query->status() == bncNetQuery::finished &&
              outData.contains("END OF HEADER")) {
            QTextStream in(outData);
            irc = _sklHeader.read(&in);
          }
    }

    delete query;
  }

  return irc;
}

// Read Skeleton Header File
////////////////////////////////////////////////////////////////////////////
bool bncRinex::readSkeleton() {

  bool readDone = false;

  // Read the local file
  // -------------------
  QFile skl(_localSklName), sklAlt(_localSklNameAlternative);
  if ( skl.exists() && skl.open(QIODevice::ReadOnly) ) {
    QTextStream in(&skl);
    if (_sklHeader.read(&in) == success) {
      readDone = true;
    }
  } else if ( sklAlt.exists() && sklAlt.open(QIODevice::ReadOnly) ) {
    QTextStream in(&sklAlt);
    if (_sklHeader.read(&in) == success) {
      readDone = true;
    }
  }

  // Read downloaded file
  // --------------------
  else if ( _ntripVersion != "N" && _ntripVersion != "UN" &&
            _ntripVersion != "S" ) {
    QDate currDate = currentDateAndTimeGPS().date();
    if ( !_skeletonDate.isValid() || _skeletonDate != currDate ) {
      if (downloadSkeleton() == success) {
        readDone = true;
        _skeletonDate = currDate;
      }
    } else if (_skeletonDate.isValid()) {
      readDone = true;
    }
  }
  return readDone;
}

// Next File Epoch (static)
////////////////////////////////////////////////////////////////////////////
QString bncRinex::nextEpochStr(const QDateTime& datTim,
                               const QString& intStr, bool rnxV3,
                               QDateTime* nextEpoch) {

  QString epoStr = "";

  QTime nextTime;
  QDate nextDate;

  int indHlp = intStr.indexOf("min");

  if ( indHlp != -1) {
    int step = intStr.left(indHlp-1).toInt();
    if (rnxV3) {
      epoStr +=  QString("%1").arg(datTim.time().hour(), 2, 10, QChar('0')); // H
    } else {
      epoStr +=  'A' + datTim.time().hour();
    }

    if (datTim.time().minute() >= 60-step) {
      epoStr += QString("%1").arg(60-step, 2, 10, QChar('0'));               // M
      if (datTim.time().hour() < 23) {
        nextTime.setHMS(datTim.time().hour() + 1 , 0, 0);
        nextDate = datTim.date();
      }
      else {
        nextTime.setHMS(0, 0, 0);
        nextDate = datTim.date().addDays(1);
      }
    }
    else {
      for (int limit = step; limit <= 60-step; limit += step) {
        if (datTim.time().minute() < limit) {
          epoStr += QString("%1").arg(limit-step, 2, 10, QChar('0'));        // M
          nextTime.setHMS(datTim.time().hour(), limit, 0);
          nextDate = datTim.date();
          break;
        }
      }
    }
    if (rnxV3) {
      epoStr += QString("_%1M").arg(step, 2, 10, QChar('0'));                // period
    }
  }
  else if (intStr == "1 hour") {
    int step = intStr.left(indHlp-1).toInt();
    if (rnxV3) {
      epoStr += QString("%1").arg(datTim.time().hour(), 2, 10, QChar('0'));  // H
      epoStr += QString("%1").arg(0, 2, 10, QChar('0'));                     // M
      epoStr += QString("_%1H").arg(step+1, 2, 10, QChar('0'));              // period
    } else {
      epoStr +=  'A' + datTim.time().hour();
    }
    if (datTim.time().hour() < 23) {
      nextTime.setHMS(datTim.time().hour() + 1 , 0, 0);
      nextDate = datTim.date();
    }
    else {
      nextTime.setHMS(0, 0, 0);
      nextDate = datTim.date().addDays(1);
    }
  }
  else {
    int step = intStr.left(indHlp-1).toInt();
    if (rnxV3) {
      epoStr += QString("%1").arg(0, 2, 10, QChar('0'));                    // H
      epoStr += QString("%1").arg(0, 2, 10, QChar('0'));                    // M
      epoStr += QString("_%1D").arg(step+1, 2, 10, QChar('0'));             // period
    } else {
      epoStr = "0";
    }
    nextTime.setHMS(0, 0, 0);
    nextDate = datTim.date().addDays(1);
  }

  if (nextEpoch) {
   *nextEpoch = QDateTime(nextDate, nextTime);
  }

  return epoStr;
}

// File Name according to RINEX Standards
////////////////////////////////////////////////////////////////////////////
void bncRinex::resolveFileName(const QDateTime& datTim) {

  bncSettings settings;
  QString path = settings.value("rnxPath").toString();
  expandEnvVar(path);

  if ( path.length() > 0 && path[path.length()-1] != QDir::separator() ) {
    path += QDir::separator();
  }

  QString hlpStr = nextEpochStr(datTim, settings.value("rnxIntr").toString(),
                                _rnxV3, &_nextCloseEpoch);

  int n = _statID.size();
  int statIDlength;
  if (n > 9) {
    statIDlength = 9; // rnx3
  }
  else if (n > 4) {
    statIDlength = 4; // rnx2
  }
  else {
    statIDlength = n;
  }
  QString ID = _statID.left(statIDlength);
  ID = ID.toUpper();

  // Check name conflict
  // -------------------
  QString distStr;
  int num = 0;
  QListIterator<QString> it(settings.value("mountPoints").toStringList());
  while (it.hasNext()) {
    QString mp = it.next();
    if (mp.indexOf(_statID.left(statIDlength)) != -1) {
      ++num;
    }
  }
  if (num > 1) {
    distStr = "_" + _statID.right(1);
  }

  QString sklExt = settings.value("rnxSkel").toString();
  if (!sklExt.isEmpty()) {
    _localSklName            = path + ID           + distStr + "." + sklExt;
    _localSklNameAlternative = path + ID.toLower() + distStr + "." + sklExt;
  }

  if (_rnxV3) {
    QString country;
    QString monNum = "0";
    QString recNum = "0";
    if (statIDlength == 9) {
      monNum = QChar(_statID[4]);
      recNum = QChar(_statID[5]);
    }
    QListIterator<QString> it(settings.value("mountPoints").toStringList());
    while (it.hasNext()) {
      QStringList hlp = it.next().split(" ");
      if (hlp.size() < 7)
        continue;
      if (hlp.join(" ").indexOf(_statID, 0) != -1) {
        country = hlp[2].left(3);
      }
    }

    path += ID.left(4) +
            QString("%1").arg(monNum, 1, 10) +
            QString("%1").arg(recNum, 1, 10) +
            country +
            "_S_" + // stream
            QString("%1").arg(datTim.date().year()) +
            QString("%1").arg(datTim.date().dayOfYear(), 3, 10, QChar('0')) +
            hlpStr + // HMS_period
            QString("%1").arg(_samplingRateStr) + // sampling rate
            "_MO" + // mixed OBS
            distStr +
            ".rnx";
  }
  else {
    path += ID.left(4) +
            QString("%1").arg(datTim.date().dayOfYear(), 3, 10, QChar('0')) +
            hlpStr + distStr + datTim.toString(".yyO");
  }

  _fName = path.toLatin1();
}

// Write RINEX Header
////////////////////////////////////////////////////////////////////////////
void bncRinex::writeHeader(const QByteArray& format, const bncTime& firstObsTime) {

  bncSettings settings;

  // Set RINEX Version
  // -----------------
  int intHeaderVers = (Qt::CheckState(settings.value("rnxV3").toInt()) == Qt::Checked ? 3 : 2);

  // Open the Output File
  // --------------------
  QDateTime datTimNom  = dateAndTimeFromGPSweek(firstObsTime.gpsw(),
                                                floor(firstObsTime.gpssec()+0.5));

  resolveFileName(datTimNom);

  // Read Skeleton Header
  // --------------------
  if (readSkeleton()) {
    _header.set(_sklHeader, intHeaderVers);
  }
  else {
    if (_writeRinexFileOnlyWithSkl) {
      return;
    }
    _header.setDefault(_statID, intHeaderVers);
  }

  // Append to existing file and return
  // ----------------------------------
  if ( QFile::exists(_fName) &&
       (_reconnectFlag || Qt::CheckState(settings.value("rnxAppend").toInt()) == Qt::Checked) ) {
    _out.open(_fName.data(), ios::app);
    _out.setf(ios::showpoint | ios::fixed);
    _headerWritten = true;
    _reconnectFlag = false;
  }
  else {
    _out.open(_fName.data());
    _addComments.clear();
  }

  _out.setf(ios::showpoint | ios::fixed);

  // A Few Additional Comments
  // -------------------------
  _addComments << format.left(6) + " " + _mountPoint.host() + _mountPoint.path();
  if (_nmea == "yes") {
    _addComments << "NMEA LAT=" + _latitude + " " + "LONG=" + _longitude;
  }

  // Write the Header
  // ----------------
  QByteArray headerLines;
  QTextStream outHlp(&headerLines);

  QMap<QString, QString> txtMap;
  txtMap["COMMENT"] = _addComments.join("\\n");

  _header.setStartTime(firstObsTime);
  _header.write(&outHlp, &txtMap);

  outHlp.flush();

  if (!_headerWritten) {
    _out << headerLines.data();
  }

  _headerWritten = true;
}

// Stores Observation into Internal Array
////////////////////////////////////////////////////////////////////////////
void bncRinex::deepCopy(t_satObs obs) {
  _obs.push_back(obs);
}

// Write One Epoch into the RINEX File
////////////////////////////////////////////////////////////////////////////
void bncRinex::dumpEpoch(const QByteArray& format, const bncTime& maxTime) {

  // Select observations older than maxTime
  // --------------------------------------
  QList<t_satObs> obsList;
  QMutableListIterator<t_satObs> mIt(_obs);
  while (mIt.hasNext()) {
    t_satObs obs = mIt.next();
    if (obs._time < maxTime) {
      obsList.push_back(obs);
      mIt.remove();
    }
  }

  // Easy Return
  // -----------
  if (obsList.isEmpty()) {
    return;
  }

  // Time of Epoch
  // -------------
  const t_satObs& fObs = obsList.first();
  QDateTime datTimNom  = dateAndTimeFromGPSweek(fObs._time.gpsw(), fObs._time.gpssec());

  // Close the file
  // --------------
  if (_nextCloseEpoch.isValid() && datTimNom >= _nextCloseEpoch) {
    closeFile();
    _headerWritten = false;
  }

  // Write RINEX Header
  // ------------------
  if (!_headerWritten) {
    writeHeader(format, fObs._time);
  }
  if (!_headerWritten) {
    return;
  }

  // Prepare structure t_rnxEpo
  // --------------------------
  t_rnxObsFile::t_rnxEpo rnxEpo;
  rnxEpo.tt = fObs._time;

  QListIterator<t_satObs> it(obsList);
  while (it.hasNext()) {
    const t_satObs& satObs = it.next();
    t_rnxObsFile::t_rnxSat rnxSat;
    rnxSat.prn = satObs._prn;

    // Initialize all observations mentioned in skeleton header
    // --------------------------------------------------------
    char sys = rnxSat.prn.system();
    for (int iType = 0; iType < _sklHeader.nTypes(sys); iType++) {
      QString type = _sklHeader.obsType(sys, iType);
      t_rnxObsFile::t_rnxObs rnxObs; // create an empty observation
      rnxSat.obs[type] = rnxObs;
    }

    for (unsigned ii = 0; ii < satObs._obs.size(); ii++) {
      const t_frqObs* frqObs = satObs._obs[ii];
      if (frqObs->_codeValid) {
        QString type = 'C' + QString(frqObs->_rnxType2ch.c_str());
        t_rnxObsFile::t_rnxObs rnxObs;
        rnxObs.value = frqObs->_code;
        rnxSat.obs[type] = rnxObs;
      }
      if (frqObs->_phaseValid) {
        QString type = 'L' + QString(frqObs->_rnxType2ch.c_str());
        t_rnxObsFile::t_rnxObs rnxObs;
        rnxObs.value = frqObs->_phase;
        if (frqObs->_slip) {
          rnxObs.lli |= 1;
        }
        rnxSat.obs[type] = rnxObs;
      }
      if (frqObs->_dopplerValid) {
        QString type = 'D' + QString(frqObs->_rnxType2ch.c_str());
        t_rnxObsFile::t_rnxObs rnxObs;
        rnxObs.value = frqObs->_doppler;
        rnxSat.obs[type] = rnxObs;
      }
      if (frqObs->_snrValid) {
        QString type = 'S' + QString(frqObs->_rnxType2ch.c_str());
        t_rnxObsFile::t_rnxObs rnxObs;
        rnxObs.value = frqObs->_snr;
        rnxSat.obs[type] = rnxObs;
      }
    }


    rnxEpo.rnxSat.push_back(rnxSat);
  }

  // Write the epoch
  // ---------------
  QByteArray outLines;
  QTextStream outStream(&outLines);
  t_rnxObsFile::writeEpoch(&outStream, _header, &rnxEpo);

  _out << outLines.data();
  _out.flush();
}

// Close the Old RINEX File
////////////////////////////////////////////////////////////////////////////
void bncRinex::closeFile() {

  if (_header.version() == 3) {
    _out << ">                              4  1" << endl;
    _out << "END OF FILE" << endl;
  }
  _out.close();
  if (!_rnxScriptName.isEmpty()) {
    qApp->thread()->wait(100);
#ifdef WIN32
    QProcess::startDetached(_rnxScriptName, QStringList() << _fName) ;
#else
    QProcess::startDetached("nohup", QStringList() << _rnxScriptName << _fName) ;
#endif

  }
}

// One Line in ASCII (Internal) Format
////////////////////////////////////////////////////////////////////////////
string bncRinex::asciiSatLine(const t_satObs& obs, bool outLockTime) {

  ostringstream str;
  str.setf(ios::showpoint | ios::fixed);

  str << obs._prn.toString();

  for (unsigned ii = 0; ii < obs._obs.size(); ii++) {
    const t_frqObs* frqObs = obs._obs[ii];
    if (frqObs->_codeValid) {
      str << ' '
          << left  << setw(3)  << "C" + frqObs->_rnxType2ch << ' '
          << right << setw(14) << setprecision(3) << frqObs->_code;
    }
    if (frqObs->_phaseValid) {
      str << ' '
          << left  << setw(3) << "L" + frqObs->_rnxType2ch << ' '
          << right << setw(14) << setprecision(3) << frqObs->_phase << ' '
          << right << setw(4)                     << frqObs->_slipCounter;
    }
    if (frqObs->_dopplerValid) {
      str << ' '
          << left  << setw(3) << "D" + frqObs->_rnxType2ch << ' '
          << right << setw(14) << setprecision(3) << frqObs->_doppler;
    }
    if (frqObs->_snrValid) {
      str << ' '
          << left  << setw(3) << "S" + frqObs->_rnxType2ch << ' '
          << right << setw(8) << setprecision(3) << frqObs->_snr;
    }
    if (frqObs->_lockTimeValid && outLockTime) {
      str << ' '
          << left  << setw(3) << "T" + frqObs->_rnxType2ch << ' '
          << right << setw(9) << setprecision(3) << frqObs->_lockTime;
    }
  }

  return str.str();
}
