/* -------------------------------------------------------------------------
 * BKG NTRIP Client
 * -------------------------------------------------------------------------
 *
 * Class:      bncNetQueryV0
 *
 * Purpose:    TCP/IP Network Requests, no NTRIP
 *
 * Author:     G. Weber
 *
 * Created:    19-Feb-2009
 *
 * Changes:    
 *
 * -----------------------------------------------------------------------*/

#include <iostream>
#include <iomanip>

#include "bncnetqueryv0.h"
#include "bncsettings.h"
#include "bncversion.h"

using namespace std;

// Constructor
////////////////////////////////////////////////////////////////////////////
bncNetQueryV0::bncNetQueryV0() {
  _socket  = 0;
  _timeOut = 120000;
}

// Destructor
////////////////////////////////////////////////////////////////////////////
bncNetQueryV0::~bncNetQueryV0() {
  delete _socket;
}

// 
////////////////////////////////////////////////////////////////////////////
void bncNetQueryV0::stop() {
#ifndef sparc
  if (_socket) {
    _socket->abort();
  }
#endif
  _status = finished;
}

// 
////////////////////////////////////////////////////////////////////////////
void bncNetQueryV0::waitForRequestResult(const QUrl&, QByteArray&) {
}

// 
////////////////////////////////////////////////////////////////////////////
void bncNetQueryV0::waitForReadyRead(QByteArray& outData) {
  if (_socket && _socket->state() == QAbstractSocket::ConnectedState) {
    while (true) {
      int nBytes = _socket->bytesAvailable();
      if (nBytes > 0) {
        outData = _socket->readAll();
        return;
      }
      else if (!_socket->waitForReadyRead(_timeOut)) {
        delete _socket;
        _socket = 0;
        _status = error;
        emit newMessage(_url.path().toLatin1() + " read timeout", true);
        return;
      }
    }
  }
}

// Connect to Caster, send the Request
////////////////////////////////////////////////////////////////////////////
void bncNetQueryV0::startRequest(const QUrl& url, const QByteArray& /* gga */) {

  _status = running;

  delete _socket;
  _socket = new QTcpSocket();

  // Default scheme and path
  // -----------------------
  _url = url;
  if (_url.scheme().isEmpty()) {
    _url.setScheme("http");
  }
  if (_url.path().isEmpty()) {
    _url.setPath("/");
  }

  // Connect the Socket
  // ------------------
  bncSettings settings;
 
  _socket->connectToHost(_url.host(), _url.port());
  if (!_socket->waitForConnected(_timeOut)) {
    delete _socket; 
    _socket = 0;
    _status = error;
      emit(newMessage(_url.path().toLatin1().replace(0,1,"")
                      + ": Connect timeout, reconnecting", true));
    return;
  }

  // Read Caster Response
  // --------------------
  QStringList response;
  while (true) {
    if (!_socket->waitForReadyRead(_timeOut)) {
      delete _socket;
      _socket = 0;
      _status = error;
      emit newMessage(_url.path().toLatin1().replace(0,1,"")
                      + ": Read timeout", true);
      return;
    }
    if (_socket->canReadLine()) {
      QString line = _socket->readLine();
      response.push_back(line);
      response.clear();
      break;
    }
  }
}

void bncNetQueryV0::keepAliveRequest(const QUrl& /* url */, const QByteArray& /* gga */) {
}
