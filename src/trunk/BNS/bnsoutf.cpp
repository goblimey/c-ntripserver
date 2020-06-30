
/* -------------------------------------------------------------------------
 * BKG NTRIP Server
 * -------------------------------------------------------------------------
 *
 * Class:      bnsoutf
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

#include "bnsoutf.h"

using namespace std;
using namespace BNS;

// Constructor
////////////////////////////////////////////////////////////////////////////
bnsoutf::bnsoutf(const QString& prep, const QString& ext, const QString& path,
                 const QString& intr, int sampl) {

  _headerWritten = false;
  _prep          = prep;
  _ext           = ext;
  _sampl         = sampl;
  _intr          = intr;
  _path          = path;
  expandEnvVar(_path);
  if ( _path.length() > 0 && _path[_path.length()-1] != QDir::separator() ) {
    _path += QDir::separator();
  }
}

// Destructor
////////////////////////////////////////////////////////////////////////////
bnsoutf::~bnsoutf() {
  closeFile();
}

// Close the Old RINEX File
////////////////////////////////////////////////////////////////////////////
void bnsoutf::closeFile() {
  _out.close();
}

// Next File Epoch (static)
////////////////////////////////////////////////////////////////////////////
QString bnsoutf::nextEpochStr(const QDateTime& datTim, 
                             const QString& intStr, QDateTime* nextEpoch) {

  QString epoStr;

  QTime nextTime;
  QDate nextDate;

  int indHlp = intStr.indexOf("min");

  if ( indHlp != -1) {
    int step = intStr.left(indHlp-1).toInt();
    char ch = 'A' + datTim.time().hour();
    epoStr = QString("_") + ch;
    if (datTim.time().minute() >= 60-step) {
      epoStr += QString("%1").arg(60-step, 2, 10, QChar('0'));
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
          epoStr += QString("%1").arg(limit-step, 2, 10, QChar('0'));
          nextTime.setHMS(datTim.time().hour(), limit, 0);
          nextDate = datTim.date();
          break;
        }
      }
    }
  }
  else if (intStr == "1 hour") {
    char ch = 'A' + datTim.time().hour();
    epoStr = QString("_") + ch;
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
    epoStr = "";
    nextTime.setHMS(0, 0, 0);
    nextDate = datTim.date().addDays(1);
  }

  if (nextEpoch) {
    *nextEpoch = QDateTime(nextDate, nextTime, Qt::UTC);
  }

  return epoStr;
}

// File Name according to RINEX Standards
////////////////////////////////////////////////////////////////////////////
void bnsoutf::resolveFileName(int GPSweek, const QDateTime& datTim) {

  QString epoStr = nextEpochStr(datTim, _intr, &_nextCloseEpoch);

  int dayOfWeek = datTim.date().dayOfWeek();
  if (dayOfWeek == 7) {
    dayOfWeek = 0;
  }

  _fName = (_path + _prep
            + QString("%1").arg(GPSweek)
            + QString("%1").arg(dayOfWeek)
            + epoStr 
            + _ext).toAscii();
}

// Write One Epoch
////////////////////////////////////////////////////////////////////////////
t_irc bnsoutf::write(int GPSweek, double GPSweeks, const QString&, 
                     const ColumnVector&, bool append) {

  if (_sampl != 0 && fmod(GPSweeks, _sampl) != 0.0) {
    return failure;
  }

  QDateTime datTim = dateAndTimeFromGPSweek(GPSweek, GPSweeks);

  // Close the file
  // --------------
  if (_nextCloseEpoch.isValid() && datTim >= _nextCloseEpoch) {
    closeFile();
    _headerWritten = false;
  }

  // Write Header
  // ------------
  if (!_headerWritten) {
    resolveFileName(GPSweek, datTim);
    _out.setf(ios::showpoint | ios::fixed);
    if (append && QFile::exists(_fName)) {
      _out.open(_fName.data(), ios::out | ios::app);
    }
    else {
      _out.open(_fName.data());
      writeHeader(datTim);
    }
    _headerWritten = true;
  }

  return success;
}
