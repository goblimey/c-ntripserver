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
 * Class:      t_bncCore
 *
 * Purpose:    This class implements the main application
 *
 * Author:     L. Mervart
 *
 * Created:    29-Aug-2006
 *
 * Changes:
 *
 * -----------------------------------------------------------------------*/

#include <iostream>
#include <sstream>
#include <QMessageBox>
#include <cmath>

#include "bnccore.h"
#include "bncutils.h"
#include "bncrinex.h"
#include "bncsettings.h"
#include "bncversion.h"
#include "ephemeris.h"
#include "rinex/rnxobsfile.h"
#include "rinex/rnxnavfile.h"
#include "pppMain.h"
#include "combination/bnccomb.h"

using namespace std;

// Singleton
////////////////////////////////////////////////////////////////////////////
t_bncCore* t_bncCore::instance() {
  static t_bncCore _bncCore;
  return &_bncCore;
}

// Constructor
////////////////////////////////////////////////////////////////////////////
t_bncCore::t_bncCore() : _ephUser(false) {
  _GUIenabled  = true;
  _logFileFlag = 0;
  _logFile     = 0;
  _logStream   = 0;
  _rawFile     = 0;
  _caster      = 0;
  _bncComb     = 0;

  // Eph file(s)
  // -----------
  _rinexVers        = 0;
  _ephFileGPS       = 0;
  _ephStreamGPS     = 0;
  _ephFileGlonass   = 0;
  _ephStreamGlonass = 0;
  _ephFileGalileo   = 0;
  _ephStreamGalileo = 0;
  _ephFileSBAS      = 0;
  _ephStreamSBAS    = 0;

  _portEph    = 0;
  _serverEph  = 0;
  _socketsEph = 0;

  _portCorr    = 0;
  _serverCorr  = 0;
  _socketsCorr = 0;

  _pgmName  = QString(BNCPGMNAME).leftJustified(20, ' ', true);
#ifdef WIN32
  _userName = QString("${USERNAME}");
#else
  _userName = QString("${USER}");
#endif
  expandEnvVar(_userName);

  _userName       = _userName.leftJustified(20, ' ', true);
  _dateAndTimeGPS = 0;
  _mainWindow     = 0;

  _pppMain = new BNC_PPP::t_pppMain();
  qRegisterMetaType< QVector<double> >      ("QVector<double>");
  qRegisterMetaType<bncTime>                ("bncTime");
  qRegisterMetaType<t_ephGPS>               ("t_ephGPS");
  qRegisterMetaType<t_ephGlo>               ("t_ephGlo");
  qRegisterMetaType<t_ephGal>               ("t_ephGal");
  qRegisterMetaType<t_ephSBAS>              ("t_ephSBAS");
  qRegisterMetaType<t_ephBDS>               ("t_ephBDS");
  qRegisterMetaType<QList<t_orbCorr> >      ("QList<t_orbCorr>");
  qRegisterMetaType<QList<t_clkCorr> >      ("QList<t_clkCorr>");
  qRegisterMetaType<QList<t_satCodeBias> >  ("QList<t_satCodeBias>");
  qRegisterMetaType<QList<t_satPhaseBias> > ("QList<t_satPhaseBias>");
  qRegisterMetaType<t_vTec>                 ("t_vTec");
}

// Destructor
////////////////////////////////////////////////////////////////////////////
t_bncCore::~t_bncCore() {
  delete _logStream;
  delete _logFile;
  delete _ephStreamGPS;
  delete _ephFileGPS;
  delete _serverEph;
  delete _socketsEph;
  delete _serverCorr;
  delete _socketsCorr;
  if (_rinexVers == 2) {
    delete _ephStreamGlonass;
    delete _ephFileGlonass;
  }

  delete _dateAndTimeGPS;
  delete _rawFile;
  delete _bncComb;
  delete _pppMain;
}

// Write a Program Message
////////////////////////////////////////////////////////////////////////////
void t_bncCore::slotMessage(QByteArray msg, bool showOnScreen) {

  QMutexLocker locker(&_mutexMessage);

  messagePrivate(msg);
  emit newMessage(msg, showOnScreen);
}

// Write a Program Message (private, no lock)
////////////////////////////////////////////////////////////////////////////
void t_bncCore::messagePrivate(const QByteArray& msg) {

  // First time resolve the log file name
  // ------------------------------------
  QDate currDate = currentDateAndTimeGPS().date();
  if (_logFileFlag == 0 || _fileDate != currDate) {
    delete _logStream; _logStream = 0;
    delete _logFile;   _logFile   = 0;
    _logFileFlag = 1;
    bncSettings settings;
    QString logFileName = settings.value("logFile").toString();
    if ( !logFileName.isEmpty() ) {
      expandEnvVar(logFileName);
      _logFile = new QFile(logFileName + "_" +
                          currDate.toString("yyMMdd").toLatin1().data());
      _fileDate = currDate;
      if ( Qt::CheckState(settings.value("rnxAppend").toInt()) == Qt::Checked) {
        _logFile->open(QIODevice::WriteOnly | QIODevice::Append);
      }
      else {
        _logFile->open(QIODevice::WriteOnly);
      }
      _logStream = new QTextStream();
      _logStream->setDevice(_logFile);
    }
  }

  if (_logStream) {
    QByteArray msgLocal = msg;
    if (msg.indexOf('\n') == 0) {
      *_logStream << endl;
      msgLocal = msg.mid(1);
    }
    *_logStream << currentDateAndTimeGPS().toString("yy-MM-dd hh:mm:ss ").toLatin1().data();
    *_logStream << msgLocal.data() << endl;
    _logStream->flush();
    _logFile->flush();
  }
}

//
////////////////////////////////////////////////////////////////////////////
t_irc t_bncCore::checkPrintEph(t_eph* eph) {
  QMutexLocker locker(&_mutex);
  t_irc ircPut = _ephUser.putNewEph(eph, true);
  if      (eph->checkState() == t_eph::bad) {
    messagePrivate("WRONG EPHEMERIS\n" + eph->toString(3.0).toLatin1());
    return failure;
  }
  else if (eph->checkState() == t_eph::outdated) {
    messagePrivate("OUTDATED EPHEMERIS\n" + eph->toString(3.0).toLatin1());
    return failure;
  }
  else if (eph->checkState() == t_eph::unhealthy) {
    messagePrivate("UNHEALTHY EPHEMERIS\n" + eph->toString(3.0).toLatin1());
  }
  printEphHeader();
  printEph(*eph, (ircPut == success));
  return success;
}

// New GPS Ephemeris
////////////////////////////////////////////////////////////////////////////
void t_bncCore::slotNewGPSEph(t_ephGPS eph) {
  if (checkPrintEph(&eph) == success) {
    emit newGPSEph(eph);
  }
}

// New Glonass Ephemeris
////////////////////////////////////////////////////////////////////////////
void t_bncCore::slotNewGlonassEph(t_ephGlo eph) {
  if (checkPrintEph(&eph) == success) {
    emit newGlonassEph(eph);
  }
}

// New Galileo Ephemeris
////////////////////////////////////////////////////////////////////////////
void t_bncCore::slotNewGalileoEph(t_ephGal eph) {
  if (checkPrintEph(&eph) == success) {
    emit newGalileoEph(eph);
  }
}

// New SBAS Ephemeris
////////////////////////////////////////////////////////////////////////////
void t_bncCore::slotNewSBASEph(t_ephSBAS eph) {
  if (checkPrintEph(&eph) == success) {
    emit newSBASEph(eph);
  }
}

// New BDS Ephemeris
////////////////////////////////////////////////////////////////////////////
void t_bncCore::slotNewBDSEph(t_ephBDS eph) {
  if (checkPrintEph(&eph) == success) {
    emit newBDSEph(eph);
  }
}

// Print Header of the output File(s)
////////////////////////////////////////////////////////////////////////////
void t_bncCore::printEphHeader() {

  bncSettings settings;
  QStringList comments;

  QListIterator<QString> it(settings.value("mountPoints").toStringList());
  while (it.hasNext()) {
    QStringList hlp = it.next().split(" ");
    if (hlp.size() < 7)
      continue;
    QUrl url(hlp[0]);
    QString decoder = hlp[1];
    comments.append("Source: " + decoder +
                    " " + QUrl::toAce(url.host()) +
                    "/" + url.path().mid(1).toLatin1());
  }

  // Initialization
  // --------------
  if (_rinexVers == 0) {

    if ( Qt::CheckState(settings.value("ephV3").toInt()) == Qt::Checked) {
      _rinexVers = 3;
    }
    else {
      _rinexVers = 2;
    }

    _ephPath = settings.value("ephPath").toString();

    if ( !_ephPath.isEmpty() ) {
      if ( _ephPath[_ephPath.length()-1] != QDir::separator() ) {
        _ephPath += QDir::separator();
      }
      expandEnvVar(_ephPath);
    }
  }

  // (Re-)Open output File(s)
  // ------------------------
  if (!_ephPath.isEmpty()) {

    QDateTime datTim = currentDateAndTimeGPS();

    QString ephFileNameGPS = _ephPath + "BRDC";

    bool ephV3 = (_rinexVers == 3)? true : false;

    QString hlpStr = bncRinex::nextEpochStr(datTim,
                         settings.value("ephIntr").toString(), ephV3);

    if (_rinexVers == 3) {
      QString country = "WRD"; // WORLD
      QString monNum = "0";
      QString recNum = "0";
      ephFileNameGPS += QString("%1").arg(monNum, 1, 10) +
                        QString("%1").arg(recNum, 1, 10) +
                        country +
                        "_S_" +     // stream
                        QString("%1").arg(datTim.date().year()) +
                        QString("%1").arg(datTim.date().dayOfYear(), 3, 10, QChar('0')) +
                        hlpStr +   // HM_period
                        "_MN.rnx"; // mixed BRDC
    }
    else { // RNX v2.11
      ephFileNameGPS += QString("%1").arg(datTim.date().dayOfYear(), 3, 10, QChar('0')) +
                        hlpStr + datTim.toString(".yyN");
    }

    if (_ephFileNameGPS == ephFileNameGPS) {
      return;
    }
    else {
      _ephFileNameGPS = ephFileNameGPS;
    }

    delete _ephStreamGPS;
    delete _ephFileGPS;

    QFlags<QIODevice::OpenModeFlag> appendFlagGPS;
    QFlags<QIODevice::OpenModeFlag> appendFlagGlonass;

    if ( Qt::CheckState(settings.value("rnxAppend").toInt()) == Qt::Checked &&
         QFile::exists(ephFileNameGPS) ) {
      appendFlagGPS = QIODevice::Append;
    }

    _ephFileGPS = new QFile(ephFileNameGPS);
    _ephFileGPS->open(QIODevice::WriteOnly | appendFlagGPS);
    _ephStreamGPS = new QTextStream();
    _ephStreamGPS->setDevice(_ephFileGPS);

    if      (_rinexVers == 3) {
      _ephFileGlonass   = _ephFileGPS;
      _ephStreamGlonass = _ephStreamGPS;
      _ephFileGalileo   = _ephFileGPS;
      _ephStreamGalileo = _ephStreamGPS;
      _ephFileSBAS      = _ephFileGPS;
      _ephStreamSBAS    = _ephStreamGPS;
    }
    else if (_rinexVers == 2) {
      QString ephFileNameGlonass = _ephPath + "BRDC" +
          QString("%1").arg(datTim.date().dayOfYear(), 3, 10, QChar('0')) +
          hlpStr + datTim.toString(".yyG");

      delete _ephStreamGlonass;
      delete _ephFileGlonass;

      if ( Qt::CheckState(settings.value("rnxAppend").toInt()) == Qt::Checked &&
           QFile::exists(ephFileNameGlonass) ) {
        appendFlagGlonass = QIODevice::Append;
      }

      _ephFileGlonass = new QFile(ephFileNameGlonass);
      _ephFileGlonass->open(QIODevice::WriteOnly | appendFlagGlonass);
      _ephStreamGlonass = new QTextStream();
      _ephStreamGlonass->setDevice(_ephFileGlonass);
    }

    // Header - RINEX Version 3
    // ------------------------
    if (_rinexVers == 3) {
      if ( ! (appendFlagGPS & QIODevice::Append)) {
        QString line;
        line.sprintf(
          "%9.2f%11sN: GNSS NAV DATA    M: Mixed%12sRINEX VERSION / TYPE\n",
          defaultRnxNavVersion3, "", "");
        *_ephStreamGPS << line;

        QString hlp = currentDateAndTimeGPS().toString("yyyyMMdd hhmmss UTC").leftJustified(20, ' ', true);
        *_ephStreamGPS << _pgmName.toLatin1().data()
                       << _userName.toLatin1().data()
                       << hlp.toLatin1().data()
                       << "PGM / RUN BY / DATE" << endl;

        QStringListIterator it(comments);
        while (it.hasNext()) {
          *_ephStreamGPS << it.next().trimmed().left(60).leftJustified(60) << "COMMENT\n";
        }

        line.sprintf("%60sEND OF HEADER\n", "");
        *_ephStreamGPS << line;

        _ephStreamGPS->flush();
      }
    }

    // Headers - RINEX Version 2
    // -------------------------
    else if (_rinexVers == 2) {
      if (! (appendFlagGPS & QIODevice::Append)) {
        QString line;
        line.sprintf("%9.2f%11sN: GPS NAV DATA%25sRINEX VERSION / TYPE\n",
                     defaultRnxNavVersion2, "", "");
        *_ephStreamGPS << line;

        QString hlp = currentDateAndTimeGPS().date().toString("dd-MMM-yyyy").leftJustified(20, ' ', true);
        *_ephStreamGPS << _pgmName.toLatin1().data()
                       << _userName.toLatin1().data()
                       << hlp.toLatin1().data()
                       << "PGM / RUN BY / DATE" << endl;

        QStringListIterator it(comments);
        while (it.hasNext()) {
          *_ephStreamGPS << it.next().trimmed().left(60).leftJustified(60) << "COMMENT\n";
        }

        line.sprintf("%60sEND OF HEADER\n", "");
        *_ephStreamGPS << line;

        _ephStreamGPS->flush();
      }
      if (! (appendFlagGlonass & QIODevice::Append)) {
        QString line;
        line.sprintf("%9.2f%11sG: GLONASS NAV DATA%21sRINEX VERSION / TYPE\n",
                     defaultRnxNavVersion2, "", "");
        *_ephStreamGlonass << line;

        QString hlp = currentDateAndTimeGPS().date().toString("dd-MMM-yyyy").leftJustified(20, ' ', true);
        *_ephStreamGlonass << _pgmName.toLatin1().data()
                           << _userName.toLatin1().data()
                           << hlp.toLatin1().data()
                           << "PGM / RUN BY / DATE" << endl;

        QStringListIterator it(comments);
        while (it.hasNext()) {
          *_ephStreamGlonass << it.next().trimmed().left(60).leftJustified(60) << "COMMENT\n";
        }

        line.sprintf("%60sEND OF HEADER\n", "");
        *_ephStreamGlonass << line;

        _ephStreamGlonass->flush();
      }
    }
  }
}

// Print One Ephemeris
////////////////////////////////////////////////////////////////////////////
void t_bncCore::printEph(const t_eph& eph, bool printFile) {

  QString strV2 = eph.toString(defaultRnxNavVersion2);
  QString strV3 = eph.toString(defaultRnxObsVersion3);

  if     (_rinexVers == 2 && eph.type() == t_eph::GLONASS) {
    printOutputEph(printFile, _ephStreamGlonass, strV2, strV3);
  }
  else if(_rinexVers == 2 && eph.type() == t_eph::GPS)  {
    printOutputEph(printFile, _ephStreamGPS, strV2, strV3);
  }
  else if (_rinexVers == 3) {
    printOutputEph(printFile, _ephStreamGPS, strV2, strV3);
  }
}

// Output
////////////////////////////////////////////////////////////////////////////
void t_bncCore::printOutputEph(bool printFile, QTextStream* stream,
                               const QString& strV2, const QString& strV3) {

  // Output into file
  // ----------------
  if (printFile && stream) {
    if (_rinexVers == 2) {
      *stream << strV2.toLatin1();
    }
    else {
      *stream << strV3.toLatin1();
    }
    stream->flush();
  }

  // Output into the socket
  // ----------------------
  if (_socketsEph) {
    QMutableListIterator<QTcpSocket*> is(*_socketsEph);
    while (is.hasNext()) {
      QTcpSocket* sock = is.next();
      if (sock->state() == QAbstractSocket::ConnectedState) {
        if (sock->write(strV3.toLatin1()) == -1) {
          delete sock;
          is.remove();
        }
      }
      else if (sock->state() != QAbstractSocket::ConnectingState) {
        delete sock;
        is.remove();
      }
    }
  }
}

// Set Port Number
////////////////////////////////////////////////////////////////////////////
void t_bncCore::setPortEph(int port) {
  _portEph = port;
  if (_portEph != 0) {
    delete _serverEph;
    _serverEph = new QTcpServer;
    _serverEph->setProxy(QNetworkProxy::NoProxy);
    if ( !_serverEph->listen(QHostAddress::Any, _portEph) ) {
      QString message = "t_bncCore: Cannot listen on ephemeris port "
                      + QByteArray::number(_portEph) + ": "
                      + _serverEph->errorString();
      slotMessage(message.toLatin1(), true);
    }
    connect(_serverEph, SIGNAL(newConnection()), this, SLOT(slotNewConnectionEph()));
    delete _socketsEph;
    _socketsEph = new QList<QTcpSocket*>;
  }
}

// Set Port Number
////////////////////////////////////////////////////////////////////////////
void t_bncCore::setPortCorr(int port) {
  _portCorr = port;
  if (_portCorr != 0) {
    delete _serverCorr;
    _serverCorr = new QTcpServer;
    _serverCorr->setProxy(QNetworkProxy::NoProxy);
    if ( !_serverCorr->listen(QHostAddress::Any, _portCorr) ) {
      QString message = "t_bncCore: Cannot listen on correction port "
                      + QByteArray::number(_portCorr) + ": "
                      + _serverCorr->errorString();
      slotMessage(message.toLatin1(), true);
    }
    connect(_serverCorr, SIGNAL(newConnection()), this, SLOT(slotNewConnectionCorr()));
    delete _socketsCorr;
    _socketsCorr = new QList<QTcpSocket*>;
  }
}

// New Connection
////////////////////////////////////////////////////////////////////////////
void t_bncCore::slotNewConnectionEph() {
  _socketsEph->push_back( _serverEph->nextPendingConnection() );
}

// New Connection
////////////////////////////////////////////////////////////////////////////
void t_bncCore::slotNewConnectionCorr() {
  _socketsCorr->push_back( _serverCorr->nextPendingConnection() );
}

//
////////////////////////////////////////////////////////////////////////////
void t_bncCore::slotQuit() {
  delete _caster; _caster = 0;
  qApp->quit();
}

//
////////////////////////////////////////////////////////////////////////////
void t_bncCore::slotNewOrbCorrections(QList<t_orbCorr> orbCorrections) {
  QMutexLocker locker(&_mutex);
  emit newOrbCorrections(orbCorrections);
  if (_socketsCorr) {
    ostringstream out;
    t_orbCorr::writeEpoch(&out, orbCorrections);
    QMutableListIterator<QTcpSocket*> is(*_socketsCorr);
    while (is.hasNext()) {
      QTcpSocket* sock = is.next();
      if (sock->state() == QAbstractSocket::ConnectedState) {
        if (sock->write(out.str().c_str()) == -1) {
          delete sock;
          is.remove();
        }
      }
      else if (sock->state() != QAbstractSocket::ConnectingState) {
        delete sock;
        is.remove();
      }
    }
  }
}

//
////////////////////////////////////////////////////////////////////////////
void t_bncCore::slotNewClkCorrections(QList<t_clkCorr> clkCorrections) {
  QMutexLocker locker(&_mutex);
  emit newClkCorrections(clkCorrections);
  if (_socketsCorr) {
    ostringstream out;
    t_clkCorr::writeEpoch(&out, clkCorrections);
    QMutableListIterator<QTcpSocket*> is(*_socketsCorr);
    while (is.hasNext()) {
      QTcpSocket* sock = is.next();
      if (sock->state() == QAbstractSocket::ConnectedState) {
        if (sock->write(out.str().c_str()) == -1) {
          delete sock;
          is.remove();
        }
      }
      else if (sock->state() != QAbstractSocket::ConnectingState) {
        delete sock;
        is.remove();
      }
    }
  }
}

//
////////////////////////////////////////////////////////////////////////////
void t_bncCore::slotNewCodeBiases(QList<t_satCodeBias> codeBiases) {
  QMutexLocker locker(&_mutex);
  emit newCodeBiases(codeBiases);
  if (_socketsCorr) {
    ostringstream out;
    t_satCodeBias::writeEpoch(&out, codeBiases);
    QMutableListIterator<QTcpSocket*> is(*_socketsCorr);
    while (is.hasNext()) {
      QTcpSocket* sock = is.next();
      if (sock->state() == QAbstractSocket::ConnectedState) {
        if (sock->write(out.str().c_str()) == -1) {
          delete sock;
          is.remove();
        }
      }
      else if (sock->state() != QAbstractSocket::ConnectingState) {
        delete sock;
        is.remove();
      }
    }
  }
}

//
////////////////////////////////////////////////////////////////////////////
void t_bncCore::slotNewPhaseBiases(QList<t_satPhaseBias> phaseBiases) {
  QMutexLocker locker(&_mutex);
  emit newPhaseBiases(phaseBiases);
  if (_socketsCorr) {
    ostringstream out;
    t_satPhaseBias::writeEpoch(&out, phaseBiases);
    QMutableListIterator<QTcpSocket*> is(*_socketsCorr);
    while (is.hasNext()) {
      QTcpSocket* sock = is.next();
      if (sock->state() == QAbstractSocket::ConnectedState) {
        if (sock->write(out.str().c_str()) == -1) {
          delete sock;
          is.remove();
        }
      }
      else if (sock->state() != QAbstractSocket::ConnectingState) {
        delete sock;
        is.remove();
      }
    }
  }
}

//
////////////////////////////////////////////////////////////////////////////
void t_bncCore::slotNewTec(t_vTec vTec) {
  QMutexLocker locker(&_mutex);
  emit newTec(vTec);
  if (_socketsCorr) {
    ostringstream out;
    t_vTec::write(&out, vTec);
    QMutableListIterator<QTcpSocket*> is(*_socketsCorr);
    while (is.hasNext()) {
      QTcpSocket* sock = is.next();
      if (sock->state() == QAbstractSocket::ConnectedState) {
        if (sock->write(out.str().c_str()) == -1) {
          delete sock;
          is.remove();
        }
      }
      else if (sock->state() != QAbstractSocket::ConnectingState) {
        delete sock;
        is.remove();
      }
    }
  }
}

//
////////////////////////////////////////////////////////////////////////////
void t_bncCore::setConfFileName(const QString& confFileName) {
  if (confFileName.isEmpty()) {
    _confFileName = QDir::homePath() + QDir::separator()
                  + ".config" + QDir::separator()
                  + qApp->organizationName() + QDir::separator()
                  + qApp->applicationName() + ".bnc";
  }
  else {
    _confFileName = confFileName;
  }
}

// Raw Output
////////////////////////////////////////////////////////////////////////////
void t_bncCore::writeRawData(const QByteArray& data, const QByteArray& staID,
                          const QByteArray& format) {

  QMutexLocker locker(&_mutex);

  if (!_rawFile) {
    bncSettings settings;
    QByteArray fileName = settings.value("rawOutFile").toByteArray();
    if (!fileName.isEmpty()) {
      _rawFile = new bncRawFile(fileName, staID, bncRawFile::output);
    }
  }

  if (_rawFile) {
    _rawFile->writeRawData(data, staID, format);
  }
}

//
////////////////////////////////////////////////////////////////////////////
void t_bncCore::initCombination() {
  _bncComb = new bncComb();
  if (_bncComb->nStreams() < 1) {
    delete _bncComb;
    _bncComb = 0;
  }
}

//
////////////////////////////////////////////////////////////////////////////
void t_bncCore::stopCombination() {
  delete _bncComb;
  _bncComb = 0;
}

//
////////////////////////////////////////////////////////////////////////////
bool t_bncCore::dateAndTimeGPSSet() const {
  QMutexLocker locker(&_mutexDateAndTimeGPS);
  if (_dateAndTimeGPS) {
    return true;
  }
  else {
    return false;
  }
}

//
////////////////////////////////////////////////////////////////////////////
QDateTime t_bncCore::dateAndTimeGPS() const {
  QMutexLocker locker(&_mutexDateAndTimeGPS);
  if (_dateAndTimeGPS) {
    return *_dateAndTimeGPS;
  }
  else {
    return QDateTime();
  }
}

//
////////////////////////////////////////////////////////////////////////////
void t_bncCore::setDateAndTimeGPS(QDateTime dateTime) {
  QMutexLocker locker(&_mutexDateAndTimeGPS);
  delete _dateAndTimeGPS;
  _dateAndTimeGPS = new QDateTime(dateTime);
}

//
////////////////////////////////////////////////////////////////////////////
void t_bncCore::startPPP() {
  _pppMain->start();
}

//
////////////////////////////////////////////////////////////////////////////
void t_bncCore::stopPPP() {
  _pppMain->stop();
  emit stopRinexPPP();
}

