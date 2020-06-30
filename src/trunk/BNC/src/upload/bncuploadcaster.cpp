/* -------------------------------------------------------------------------
 * BKG NTRIP Server
 * -------------------------------------------------------------------------
 *
 * Class:      bncUploadCaster
 *
 * Purpose:    Connection to NTRIP Caster
 *
 * Author:     L. Mervart
 *
 * Created:    29-Mar-2011
 *
 * Changes:
 *
 * -----------------------------------------------------------------------*/

#include <math.h>
#include "bncuploadcaster.h"
#include "bncversion.h"
#include "bnccore.h"
#include "bnctableitem.h"

using namespace std;

// Constructor
////////////////////////////////////////////////////////////////////////////
bncUploadCaster::bncUploadCaster(const QString& mountpoint,
                                 const QString& outHost, int outPort,
                                 const QString& ntripVersion,
                                 const QString& userName, const QString& password,
                                 int iRow,
                                 int rate) {
  _mountpoint    = mountpoint;
  _outHost       = outHost;
  _outPort       = outPort;
  _ntripVersion  = ntripVersion;
  _userName      = userName;
  _password      = password;
  _outSocket     = 0;
  _sOpenTrial    = 0;
  _iRow          = iRow;
  _rate          = rate;
  if      (_rate < 0) {
    _rate = 0;
  }
  else if (_rate > 60) {
    _rate = 60;
  }
  _isToBeDeleted = false;

  connect(this, SIGNAL(newMessage(QByteArray,bool)),
          BNC_CORE, SLOT(slotMessage(const QByteArray,bool)));

  if (BNC_CORE->_uploadTableItems.find(_iRow) != BNC_CORE->_uploadTableItems.end()){
    connect(this, SIGNAL(newBytes(QByteArray,double)),
            BNC_CORE->_uploadTableItems.value(iRow),
            SLOT(slotNewBytes(const QByteArray,double)));
  }
  if (BNC_CORE->_uploadEphTableItems.find(_iRow) != BNC_CORE->_uploadEphTableItems.end()){
    connect(this, SIGNAL(newBytes(QByteArray,double)),
            BNC_CORE->_uploadEphTableItems.value(iRow),
            SLOT(slotNewBytes(const QByteArray,double)));
  }
}

// Safe Desctructor
////////////////////////////////////////////////////////////////////////////
void bncUploadCaster::deleteSafely() {
  _isToBeDeleted = true;
  if (!isRunning()) {
    delete this;
  }
}

// Destructor
////////////////////////////////////////////////////////////////////////////
bncUploadCaster::~bncUploadCaster() {
  if (isRunning()) {
    wait();
  }
  if (_outSocket) {
    delete _outSocket;
  }
}

// Endless Loop
////////////////////////////////////////////////////////////////////////////
void bncUploadCaster::run() {
  while (true) {
    if (_isToBeDeleted) {
      QThread::quit();
      deleteLater();
      return;
    }
    open();
    if (_outSocket && _outSocket->state() == QAbstractSocket::ConnectedState) {
      QMutexLocker locker(&_mutex);
      if (_outBuffer.size() > 0) {
        _outSocket->write(_outBuffer);
        _outSocket->flush();
        emit newBytes(_mountpoint.toLatin1(), _outBuffer.size());
      }
    }
    if (_rate == 0) {
      {
        QMutexLocker locker(&_mutex);
        _outBuffer.clear();
      }
      msleep(100); //sleep 0.1 sec
    }
    else {
      sleep(_rate);
    }
  }
}

// Start the Communication with NTRIP Caster
////////////////////////////////////////////////////////////////////////////
void bncUploadCaster::open() {

  if (_mountpoint.isEmpty()) {
    return;
  }

  if (_outSocket != 0 &&
      _outSocket->state() == QAbstractSocket::ConnectedState) {
    return;
  }

  delete _outSocket; _outSocket = 0;

  double minDt = pow(2.0,_sOpenTrial);
  if (++_sOpenTrial > 4) {
    _sOpenTrial = 4;
  }
  if (_outSocketOpenTime.isValid() &&
      _outSocketOpenTime.secsTo(QDateTime::currentDateTime()) < minDt) {
    return;
  }
  else {
    _outSocketOpenTime = QDateTime::currentDateTime();
  }

  _outSocket = new QTcpSocket();
  _outSocket->setProxy(QNetworkProxy::NoProxy); // Ntrip1: to prevent the usage of system entries
  _outSocket->connectToHost(_outHost, _outPort);

  const int timeOut = 5000;  // 5 seconds
  if (!_outSocket->waitForConnected(timeOut)) {
    emit(newMessage("Broadcaster: Connect timeout for " + _mountpoint.toLatin1()
         + "(" + _outHost.toLatin1() + "), " + _outSocket->errorString().toLatin1(), true));
    delete _outSocket;
    _outSocket = 0;
    return;
  }

  QByteArray msg = "SOURCE " + _password.toLatin1() + " /" +
                   _mountpoint.toLatin1() + "\r\n" +
                   "Source-Agent: NTRIP BNC/" BNCVERSION "\r\n\r\n";

  _outSocket->write(msg);
  _outSocket->waitForBytesWritten();

  _outSocket->waitForReadyRead();
  QByteArray ans = _outSocket->readLine();

  if (ans.indexOf("OK") == -1) {
    delete _outSocket;
    _outSocket = 0;
    emit(newMessage("Broadcaster: Connection broken for " + _mountpoint.toLatin1(), true));
  }
  else {
    emit(newMessage("Broadcaster: Connection opened for " + _mountpoint.toLatin1(), true));
    _sOpenTrial = 0;
  }
}

