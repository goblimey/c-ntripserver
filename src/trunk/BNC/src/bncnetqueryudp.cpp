/* -------------------------------------------------------------------------
 * BKG NTRIP Client
 * -------------------------------------------------------------------------
 *
 * Class:      bncNetQueryUdp
 *
 * Purpose:    Blocking Network Requests (NTRIP Version 2 with plain UDP)
 *
 * Author:     L. Mervart
 *
 * Created:    04-Feb-2009
 *
 * Changes:    
 *
 * -----------------------------------------------------------------------*/

#include <iostream>
#include <iomanip>
#include <time.h>

#include <QHostInfo>

#include "bncnetqueryudp.h"
#include "bncsettings.h"
#include "bncversion.h"

using namespace std;

#define TIME_RESOLUTION 125

// Constructor
////////////////////////////////////////////////////////////////////////////
bncNetQueryUdp::bncNetQueryUdp() {
  _port      = 0;
  _udpSocket = 0;
  _eventLoop = new QEventLoop(this);

  _keepAlive[ 0] = 128;
  _keepAlive[ 1] =  96;
  for (int ii = 2; ii <=11; ii++) {
    _keepAlive[ii] = 0;
  }
}

// Destructor
////////////////////////////////////////////////////////////////////////////
bncNetQueryUdp::~bncNetQueryUdp() {
  delete _eventLoop;
  delete _udpSocket;
}

// 
////////////////////////////////////////////////////////////////////////////
void bncNetQueryUdp::stop() {
  _eventLoop->quit();
  _status = finished;
}

// 
////////////////////////////////////////////////////////////////////////////
void bncNetQueryUdp::slotKeepAlive() {
  if (_udpSocket) {
    _udpSocket->writeDatagram(_keepAlive, 12, _address, _port);
  }
  QTimer::singleShot(15000, this, SLOT(slotKeepAlive()));
}

// 
////////////////////////////////////////////////////////////////////////////
void bncNetQueryUdp::waitForRequestResult(const QUrl&, QByteArray&) {
}

// 
////////////////////////////////////////////////////////////////////////////
void bncNetQueryUdp::waitForReadyRead(QByteArray& outData) {

  // Wait Loop
  // ---------
  if (!_udpSocket->hasPendingDatagrams()) {
    _eventLoop->exec();
  }

  // Append Data
  // -----------
  QByteArray datagram;
  datagram.resize(_udpSocket->pendingDatagramSize());
  _udpSocket->readDatagram(datagram.data(), datagram.size());

  if (datagram.size() > 12) {
    outData.append(datagram.mid(12));
  }
  else {
    _status = error;
  }
}

// Connect to Caster, send the Request
////////////////////////////////////////////////////////////////////////////
void bncNetQueryUdp::startRequest(const QUrl& url, const QByteArray& gga) {

  _status = running;

  // Default scheme and path
  // -----------------------
  _url = url;
  if (_url.scheme().isEmpty()) {
    _url.setScheme("http");
  }
  if (_url.path().isEmpty()) {
    _url.setPath("/");
  }

  _port = _url.port();

  delete _udpSocket;
  _udpSocket = new QUdpSocket();
  _udpSocket->bind(0);
  connect(_udpSocket, SIGNAL(readyRead()), _eventLoop, SLOT(quit()));

  QHostInfo hInfo = QHostInfo::fromName(url.host());

  if (!hInfo.addresses().isEmpty()) {

    _address = hInfo.addresses().first();

    // Send Request
    // ------------
    QString uName = QUrl::fromPercentEncoding(_url.userName().toLatin1());
    QString passW = QUrl::fromPercentEncoding(_url.password().toLatin1());
    QByteArray userAndPwd;
    
    if(!uName.isEmpty() || !passW.isEmpty()) {
      userAndPwd = "Authorization: Basic " + (uName.toLatin1() + ":" +
      passW.toLatin1()).toBase64() + "\r\n";
    }
    
    QByteArray reqStr = "GET " + _url.path().toLatin1() + " HTTP/1.1\r\n"
                      + "Host: " + _url.host().toLatin1() + "\r\n"
                      + "Ntrip-Version: Ntrip/2.0\r\n"
                      + "User-Agent: NTRIP BNC/" BNCVERSION " (" BNC_OS ")\r\n";
    if (!gga.isEmpty()) {
      reqStr += "Ntrip-GGA: " + gga + "\r\n";
    }
    reqStr += userAndPwd + "Connection: close\r\n\r\n";
    
    char rtpbuffer[12 + reqStr.size()];
    rtpbuffer[0]  = 128;
    rtpbuffer[1]  =  97;
    for (int jj = 2; jj <= 11; jj++) {
      rtpbuffer[jj] = _keepAlive[jj];
    }
    for (int ii = 0; ii < reqStr.size(); ii++) {
      rtpbuffer[12+ii] = reqStr[ii]; 
    }

    _udpSocket->writeDatagram(rtpbuffer, 12 + reqStr.size(), _address, _port);

    // Wait for Reply, read Session Number
    // -----------------------------------
    QByteArray repl;
    waitForReadyRead(repl);

    QTextStream in(repl);
    QString line = in.readLine();
    while (!line.isEmpty()) {
      if (line.indexOf("Session:") == 0) {
        _session = line.mid(9).toUInt();
        _keepAlive[ 8] = (_session >> 24) & 0xFF;
        _keepAlive[ 9] = (_session >> 16) & 0xFF;
        _keepAlive[10] = (_session >>  8) & 0xFF;
        _keepAlive[11] = (_session)       & 0xFF;
        break;
      }
      line = in.readLine();
    }

    QTimer::singleShot(15000, this, SLOT(slotKeepAlive()));
  }
}

void bncNetQueryUdp::keepAliveRequest(const QUrl& /* url */ , const QByteArray& /* gga */) {
}
