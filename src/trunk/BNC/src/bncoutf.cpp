
/* -------------------------------------------------------------------------
 * BKG NTRIP Server
 * -------------------------------------------------------------------------
 *
 * Class:      bncoutf
 *
 * Purpose:    Basis Class for File-Writers
 *
 * Author:     L. Mervart
 *
 * Created:    25-Apr-2008
 *
 * Changes:
 *
 * -----------------------------------------------------------------------*/

#include <math.h>
#include <iomanip>

#include "bncoutf.h"
#include "bncsettings.h"

using namespace std;

// Constructor
////////////////////////////////////////////////////////////////////////////
bncoutf::bncoutf(const QString& sklFileName, const QString& intr, int sampl) {

  bncSettings settings;

  _headerWritten = false;
  _sampl         = sampl;
  _intr          = intr;
  _numSec        = 0;

  if (! sklFileName.isEmpty()) {
    QFileInfo fileInfo(sklFileName);
    _path        = fileInfo.absolutePath() + QDir::separator();
    _sklBaseName = fileInfo.baseName();
    _extension   = fileInfo.completeSuffix();

    expandEnvVar(_path);
    if (!_extension.isEmpty()) {
      _extension = "." + _extension;
    }
  }

  _append = Qt::CheckState(settings.value("rnxAppend").toInt()) == Qt::Checked;
  _v3filenames = settings.value("PPP/v3filenames").toBool();
}

// Destructor
////////////////////////////////////////////////////////////////////////////
bncoutf::~bncoutf() {
  closeFile();
}

// Close the Old RINEX File
////////////////////////////////////////////////////////////////////////////
void bncoutf::closeFile() {
  _out.close();
}

// Epoch String
////////////////////////////////////////////////////////////////////////////
QString bncoutf::epochStr(const QDateTime& datTim, const QString& intStr,
    int sampl) {

  QString epoStr = "";

  int indHlp = intStr.indexOf("min");
  if (!sampl) {
    sampl++;
  }

  if ( indHlp != -1) {
    int step = intStr.left(indHlp-1).toInt();
    if (_v3filenames) {
      epoStr +=  QString("%1").arg(datTim.time().hour(), 2, 10, QChar('0')); // H
    } else {
      epoStr +=  'A' + datTim.time().hour();
    }

    if (datTim.time().minute() >= 60-step) {
      epoStr += QString("%1").arg(60-step, 2, 10, QChar('0'));               // M
    }
    else {
      for (int limit = step; limit <= 60-step; limit += step) {
        if (datTim.time().minute() < limit) {
          epoStr += QString("%1").arg(limit-step, 2, 10, QChar('0'));        // M
          break;
        }
      }
    }

    if (_v3filenames) {
      epoStr += QString("_%1M").arg(step, 2, 10, QChar('0'));                // period
    }

    _numSec = 60 * step;
  }
  else if (intStr == "1 hour") {
    int step = intStr.left(indHlp-1).toInt();
    if (_v3filenames) {
      epoStr += QString("%1").arg(datTim.time().hour(), 2, 10, QChar('0'));  // H
      epoStr += QString("%1").arg(0, 2, 10, QChar('0'));                     // M
      epoStr += QString("_%1H").arg(step+1, 2, 10, QChar('0'));              // period
    } else {
      epoStr +=  'A' + datTim.time().hour();
    }
    _numSec = 3600;
  }
  else {
    int step = intStr.left(indHlp-1).toInt();
    if (_v3filenames) {
      epoStr += QString("%1").arg(0, 2, 10, QChar('0'));                    // H
      epoStr += QString("%1").arg(0, 2, 10, QChar('0'));                    // M
      epoStr += QString("_%1D").arg(step+1, 2, 10, QChar('0'));             // period
    }
    _numSec = 86400;
  }

  if (_v3filenames) {
    if (sampl < 60) {
      epoStr += QString("_%1S").arg(sampl, 2, 10, QChar('0'));
    }
    else {
      sampl /= 60;
      epoStr += QString("_%1M").arg(sampl, 2, 10, QChar('0'));
    }
  }

  return epoStr;
}

// File Name according to RINEX Standards
////////////////////////////////////////////////////////////////////////////
QString bncoutf::resolveFileName(int GPSweek, const QDateTime& datTim) {

  int dayOfWeek = datTim.date().dayOfWeek();
  if (dayOfWeek == 7) {
    dayOfWeek = 0;
  }
  int dayOfYear = datTim.date().dayOfYear();

  QString yyyy     = QString::number(datTim.date().year());
  QString doy      = QString("%1").arg(dayOfYear,3,10, QLatin1Char('0'));
  QString gpswd    = QString("%1%2").arg(GPSweek).arg(dayOfWeek);
  QString epoStr   = epochStr(datTim, _intr, _sampl);
  QString baseName = _sklBaseName;
  baseName.replace("${GPSWD}", gpswd);
  baseName.replace("${V3}" , QString("_U_%1%2").arg(yyyy).arg(doy));
  if (_extension.count(".") == 2) {_extension.replace(0,1,"_"); }
  
  return _path + baseName + epoStr + _extension;
}

// Re-Open Output File
////////////////////////////////////////////////////////////////////////////
t_irc bncoutf::reopen(int GPSweek, double GPSweeks) {

  if (_sampl != 0 && fmod(GPSweeks, _sampl) != 0.0) {
    return failure;
  }

  QDateTime datTim = dateAndTimeFromGPSweek(GPSweek, GPSweeks);

  QString newFileName = resolveFileName(GPSweek, datTim);

  // Close the file
  // --------------
  if (newFileName != _fName) {
    closeFile();
    _headerWritten = false;
    _fName = newFileName;
  }

  // Re-Open File, Write Header
  // --------------------------
  if (!_headerWritten) {
    _out.setf(ios::showpoint | ios::fixed);
    if (_append && QFile::exists(_fName)) {
      _out.open(_fName.toLatin1().data(), ios::out | ios::app);
    }
    else {
      _out.open(_fName.toLatin1().data());
      writeHeader(datTim);
    }
    if (_out.is_open()) {
      _headerWritten = true;
    }
  }

  return success;
}

// Write String
////////////////////////////////////////////////////////////////////////////
t_irc bncoutf::write(int GPSweek, double GPSweeks, const QString& str) {
  reopen(GPSweek, GPSweeks);
  _out << str.toLatin1().data();
  _out.flush();
  return success;
}
