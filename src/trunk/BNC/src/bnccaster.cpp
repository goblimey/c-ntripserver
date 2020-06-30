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
 * Class:      bncCaster
 *
 * Purpose:    buffers and disseminates the data
 *
 * Author:     L. Mervart
 *
 * Created:    24-Dec-2005
 *
 * Changes:
 *
 * -----------------------------------------------------------------------*/

#include <math.h>
#include <unistd.h>
#include <iostream>
#include <iomanip>
#include <sstream>

#include "bnccaster.h"
#include "bncrinex.h"
#include "bnccore.h"
#include "bncgetthread.h"
#include "bncutils.h"
#include "bncsettings.h"

using namespace std;

// Constructor
////////////////////////////////////////////////////////////////////////////
bncCaster::bncCaster() {

  bncSettings settings;

  connect(this, SIGNAL(newMessage(QByteArray,bool)),
          BNC_CORE, SLOT(slotMessage(const QByteArray,bool)));

  _outFile = 0;
  _out     = 0;
  reopenOutFile();

  int port = settings.value("outPort").toInt();

  if (port != 0) {
    _server = new QTcpServer;
    _server->setProxy(QNetworkProxy::NoProxy);
    if ( !_server->listen(QHostAddress::Any, port) ) {
      QString message = "bncCaster: Cannot listen on sync port "
                      + QByteArray::number(port) + ": "
                      + _server->errorString();
      emit newMessage(message.toLatin1(), true);
    }
    connect(_server, SIGNAL(newConnection()), this, SLOT(slotNewConnection()));
    _sockets = new QList<QTcpSocket*>;
  }
  else {
    _server  = 0;
    _sockets = 0;
  }

  int uPort = settings.value("outUPort").toInt();
  if (uPort != 0) {
    _uServer = new QTcpServer;
    _uServer->setProxy(QNetworkProxy::NoProxy);
    if ( !_uServer->listen(QHostAddress::Any, uPort) ) {
      QString message = "bncCaster: Cannot listen on usync port "
                      + QByteArray::number(uPort) + ": "
                      + _uServer->errorString();
      emit newMessage(message.toLatin1(), true);
    }
    connect(_uServer, SIGNAL(newConnection()), this, SLOT(slotNewUConnection()));
    _uSockets = new QList<QTcpSocket*>;
  }
  else {
    _uServer  = 0;
    _uSockets = 0;
  }

  _outLockTime = settings.value("outLockTime",false).toBool();
  _samplingRateMult10 = int(settings.value("outSampl").toString().split("sec").first().toDouble() * 10.0);
  _outWait            = settings.value("outWait").toInt();
  if (_outWait < 1) {
    _outWait = 1;
  }
  _confInterval = -1;

  // Miscellaneous output port
  // -------------------------
  _miscMount = settings.value("miscMount").toString();
  _miscPort  = settings.value("miscPort").toInt();
  if (!_miscMount.isEmpty() && _miscPort != 0) {
    _miscServer = new QTcpServer;
    _miscServer->setProxy(QNetworkProxy::NoProxy);
    if ( !_miscServer->listen(QHostAddress::Any, _miscPort) ) {
      QString message = "bncCaster: Cannot listen on miscellaneous output port "
                      + QByteArray::number(_miscPort) + ": "
                      + _miscServer->errorString();
      emit newMessage(message.toLatin1(), true);
    }
    connect(_miscServer, SIGNAL(newConnection()), this, SLOT(slotNewMiscConnection()));
    _miscSockets = new QList<QTcpSocket*>;
  }
  else {
    _miscServer  = 0;
    _miscSockets = 0;
  }
}

// Destructor
////////////////////////////////////////////////////////////////////////////
bncCaster::~bncCaster() {

  QMutexLocker locker(&_mutex);

  QListIterator<bncGetThread*> it(_threads);
  while(it.hasNext()){
    bncGetThread* thread = it.next();
    disconnect(thread, 0, 0, 0);
    _staIDs.removeAll(thread->staID());
    _threads.removeAll(thread);
    thread->terminate();
  }
  delete _out;
  delete _outFile;
  delete _server;
  delete _sockets;
  delete _uServer;
  delete _uSockets;
  delete _miscServer;
  delete _miscSockets;
}

// New Observations
////////////////////////////////////////////////////////////////////////////
void bncCaster::slotNewObs(const QByteArray staID, QList<t_satObs> obsList) {

  QMutexLocker locker(&_mutex);

  reopenOutFile();

  unsigned index = 0;
  QMutableListIterator<t_satObs> it(obsList);
  while (it.hasNext()) {
    ++index;
    t_satObs& obs = it.next();

    // Rename the Station
    // ------------------
    obs._staID = staID.data();

    // Output into the socket
    // ----------------------
    if (_uSockets) {

      ostringstream oStr;
      oStr.setf(ios::showpoint | ios::fixed);
      oStr << obs._staID                                        << " "
           << setw(4)  << obs._time.gpsw()                      << " "
           << setw(14) << setprecision(7) << obs._time.gpssec() << " "
           << bncRinex::asciiSatLine(obs,_outLockTime) << endl;

      string hlpStr = oStr.str();

      QMutableListIterator<QTcpSocket*> is(*_uSockets);
      while (is.hasNext()) {
        QTcpSocket* sock = is.next();
        if (sock->state() == QAbstractSocket::ConnectedState) {
          int numBytes = hlpStr.length();
          if (myWrite(sock, hlpStr.c_str(), numBytes) != numBytes) {
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

    // First time: set the _lastDumpTime
    // ---------------------------------
    if (!_lastDumpTime.valid()) {
      _lastDumpTime = obs._time - 1.0;
    }

    // An old observation - throw it away
    // ----------------------------------
    if (obs._time <= _lastDumpTime) {
      if (index == 1) {
        bncSettings settings;
        if ( !settings.value("outFile").toString().isEmpty() ||
             !settings.value("outPort").toString().isEmpty() ) {
          emit( newMessage(QString("%1: Old or erroneous observation epoch %2 thrown away from %3")
          		   .arg(staID.data())
          		   .arg(string(obs._time).c_str())
          		   .arg(obs._prn.toString().c_str())
        		     .toLatin1(), true) );
        }
      }
      continue;
    }

    // Save the observation
    // --------------------
    _epochs[obs._time].append(obs);

    // Dump Epochs
    // -----------
    double outWait = (double)_outWait;
    if ((obs._time - outWait) > _lastDumpTime) {
      dumpEpochs(obs._time - outWait);
      _lastDumpTime = obs._time - outWait;
    }
  }
}

// New Connection
////////////////////////////////////////////////////////////////////////////
void bncCaster::slotNewConnection() {
  _sockets->push_back( _server->nextPendingConnection() );
  emit( newMessage(QString("New client connection on sync port: # %1")
                   .arg(_sockets->size()).toLatin1(), true) );
}

void bncCaster::slotNewUConnection() {
  _uSockets->push_back( _uServer->nextPendingConnection() );
  emit( newMessage(QString("New client connection on usync port: # %1")
                   .arg(_uSockets->size()).toLatin1(), true) );
}

// Add New Thread
////////////////////////////////////////////////////////////////////////////
void bncCaster::addGetThread(bncGetThread* getThread, bool noNewThread) {

  qRegisterMetaType<t_satObs>("t_satObs");
  qRegisterMetaType< QList<t_satObs> >("QList<t_satObs>");

  connect(getThread, SIGNAL(newObs(QByteArray, QList<t_satObs>)),
          this,      SLOT(slotNewObs(QByteArray, QList<t_satObs>)));

  connect(getThread, SIGNAL(newObs(QByteArray, QList<t_satObs>)),
          this,      SIGNAL(newObs(QByteArray, QList<t_satObs>)));

  connect(getThread, SIGNAL(newRawData(QByteArray, QByteArray)),
          this,      SLOT(slotNewRawData(QByteArray, QByteArray)));

  connect(getThread, SIGNAL(getThreadFinished(QByteArray)),
          this, SLOT(slotGetThreadFinished(QByteArray)));

  _staIDs.push_back(getThread->staID());
  _threads.push_back(getThread);

  if (noNewThread) {
    getThread->run();
  }
  else {
    getThread->start();
  }
}

// Get Thread destroyed
////////////////////////////////////////////////////////////////////////////
void bncCaster::slotGetThreadFinished(QByteArray staID) {
  QMutexLocker locker(&_mutex);

  QListIterator<bncGetThread*> it(_threads);
  while (it.hasNext()) {
    bncGetThread* thread = it.next();
    if (thread->staID() == staID) {
      _threads.removeOne(thread);
    }
  }

  _staIDs.removeAll(staID);
  emit( newMessage(
           QString("Decoding %1 stream(s)").arg(_staIDs.size()).toLatin1(), true) );
  if (_staIDs.size() == 0) {
    emit(newMessage("bncCaster: Last get thread terminated", true));
    emit getThreadsFinished();
  }
}

// Dump Complete Epochs
////////////////////////////////////////////////////////////////////////////
void bncCaster::dumpEpochs(const bncTime& maxTime) {

  QMutableMapIterator<bncTime, QList<t_satObs> > itEpo(_epochs);
  while (itEpo.hasNext()) {
    itEpo.next();
    const bncTime& epoTime = itEpo.key();
    if (epoTime <= maxTime) {
      const QList<t_satObs>& allObs = itEpo.value();
      int sec = int(nint(epoTime.gpssec()*10));
      if ( (_out || _sockets) && (sec % (_samplingRateMult10) == 0) ) {
        QListIterator<t_satObs> it(allObs);
        bool firstObs = true;
        while (it.hasNext()) {
          const t_satObs& obs = it.next();

          ostringstream oStr;
          oStr.setf(ios::showpoint | ios::fixed);
          if (firstObs) {
            firstObs = false;
            oStr << "> " << obs._time.gpsw() << ' '
                 << setprecision(7) << obs._time.gpssec() << endl;
          }
          oStr << obs._staID << ' '
               << bncRinex::asciiSatLine(obs,_outLockTime) << endl;
          if (!it.hasNext()) {
            oStr << endl;
          }
          string hlpStr = oStr.str();

          // Output into the File
          // --------------------
          if (_out) {
            *_out << hlpStr.c_str();
            _out->flush();
          }

          // Output into the socket
          // ----------------------
          if (_sockets) {
            QMutableListIterator<QTcpSocket*> is(*_sockets);
            while (is.hasNext()) {
              QTcpSocket* sock = is.next();
              if (sock->state() == QAbstractSocket::ConnectedState) {
                int numBytes = hlpStr.length();
                if (myWrite(sock, hlpStr.c_str(), numBytes) != numBytes) {
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
      }
      _epochs.remove(epoTime);
    }
  }
}

// Reread configuration (private slot)
////////////////////////////////////////////////////////////////////////////
void bncCaster::slotReadMountPoints() {

  bncSettings settings;
  settings.reRead();

  readMountPoints();
}

// Read Mountpoints
////////////////////////////////////////////////////////////////////////////
void bncCaster::readMountPoints() {

  bncSettings settings;

  // Reread several options
  // ----------------------
  _samplingRateMult10 = int(settings.value("outSampl").toString().split("sec").first().toDouble() * 10.0);
  _outWait      = settings.value("outWait").toInt();
  if (_outWait < 1) {
    _outWait = 1;
  }

  // Add new mountpoints
  // -------------------
  int iMount = -1;
  QListIterator<QString> it(settings.value("mountPoints").toStringList());
  while (it.hasNext()) {
    ++iMount;
    QStringList hlp = it.next().split(" ");
    if (hlp.size() < 7) continue;
    QUrl url(hlp[0]);

    // Does it already exist?
    // ----------------------
    bool existFlg = false;
    QListIterator<bncGetThread*> iTh(_threads);
    while (iTh.hasNext()) {
      bncGetThread* thread = iTh.next();
      if (thread->mountPoint() == url) {
        existFlg = true;
        break;
      }
    }

    // New bncGetThread
    // ----------------
    if (!existFlg) {
      QByteArray format    = hlp[1].toLatin1();
      QByteArray latitude  = hlp[3].toLatin1();
      QByteArray longitude = hlp[4].toLatin1();
      QByteArray nmea      = hlp[5].toLatin1();
      QByteArray ntripVersion = hlp[6].toLatin1();

      bncGetThread* getThread = new bncGetThread(url, format, latitude,
                                        longitude, nmea, ntripVersion);
      addGetThread(getThread);

    }
  }

  // Remove mountpoints
  // ------------------
  QListIterator<bncGetThread*> iTh(_threads);
  while (iTh.hasNext()) {
    bncGetThread* thread = iTh.next();

    bool existFlg = false;
    QListIterator<QString> it(settings.value("mountPoints").toStringList());
    while (it.hasNext()) {
      QStringList hlp = it.next().split(" ");
      if (hlp.size() <= 1) continue;
      QUrl url(hlp[0]);

      if (thread->mountPoint() == url) {
        existFlg = true;
        break;
      }
    }

    if (!existFlg) {
      disconnect(thread, 0, 0, 0);
      _staIDs.removeAll(thread->staID());
      _threads.removeAll(thread);
      thread->terminate();
    }
  }

  emit mountPointsRead(_threads);
  emit( newMessage(QString("Configuration read: "
                           + BNC_CORE->confFileName()
                           + ", %1 stream(s)")
                            .arg(_threads.count()).toLatin1(), true) );

  // (Re-) Start the configuration timer
  // -----------------------------------
  if      (settings.value("onTheFlyInterval").toString() == "no") {
    return;
  }

  int ms = 0;
  if (_confInterval != -1) {
    ms = 1000 * _confInterval;
  }
  else {
    QTime currTime = currentDateAndTimeGPS().time();
    QTime nextShotTime;
    if      (settings.value("onTheFlyInterval").toString() == "1 min") {
          _confInterval = 60;
          nextShotTime = QTime(currTime.hour(), currTime.minute()+1, 0);
    }
    else if (settings.value("onTheFlyInterval").toString() == "5 min") {
      _confInterval = 300;
      nextShotTime = QTime(currTime.hour(), currTime.minute()+5, 0);
    }
    else if (settings.value("onTheFlyInterval").toString() == "1 hour") {
      _confInterval = 3600;
      nextShotTime = QTime(currTime.hour()+1, 0, 0);
    }
    else {
      _confInterval = 86400;
      nextShotTime = QTime(23, 59, 59, 999);
    }

    ms = currTime.msecsTo(nextShotTime);
    if (ms < 30000) {
      ms = 30000;
    }
  }

  QTimer::singleShot(ms, this, SLOT(slotReadMountPoints()));
}

//
////////////////////////////////////////////////////////////////////////////
int bncCaster::myWrite(QTcpSocket* sock, const char* buf, int bufLen) {
  sock->write(buf, bufLen);
  for (int ii = 1; ii <= 10; ii++) {
    if (sock->waitForBytesWritten(10)) {  // wait 10 ms
      return bufLen;
    }
  }
  return -1;
}

//
////////////////////////////////////////////////////////////////////////////
void bncCaster::reopenOutFile() {

  bncSettings settings;

  QString outFileName = settings.value("outFile").toString();
  if ( !outFileName.isEmpty() ) {
    expandEnvVar(outFileName);
    if (!_outFile || _outFile->fileName() != outFileName) {
      delete _out;
      delete _outFile;
      _outFile = new QFile(outFileName);
      if ( Qt::CheckState(settings.value("rnxAppend").toInt()) == Qt::Checked) {
        _outFile->open(QIODevice::WriteOnly | QIODevice::Append);
      }
      else {
        _outFile->open(QIODevice::WriteOnly);
      }
      _out = new QTextStream(_outFile);
      _out->setRealNumberNotation(QTextStream::FixedNotation);
    }
  }
  else {
    delete _out;     _out     = 0;
    delete _outFile; _outFile = 0;
  }
}

// Output into the Miscellaneous socket
////////////////////////////////////////////////////////////////////////////
void bncCaster::slotNewRawData(QByteArray staID, QByteArray data) {
  if (_miscSockets && (_miscMount == "ALL" || _miscMount == staID)) {
    QMutableListIterator<QTcpSocket*> is(*_miscSockets);
    while (is.hasNext()) {
      QTcpSocket* sock = is.next();
      if (sock->state() == QAbstractSocket::ConnectedState) {
        sock->write(data);
      }
      else if (sock->state() != QAbstractSocket::ConnectingState) {
        delete sock;
        is.remove();
      }
    }
  }
}

// New Connection
////////////////////////////////////////////////////////////////////////////
void bncCaster::slotNewMiscConnection() {
  _miscSockets->push_back( _miscServer->nextPendingConnection() );
  emit( newMessage(QString("New client connection on Miscellaneous Output Port: # %1")
                   .arg(_miscSockets->size()).toLatin1(), true) );
}
