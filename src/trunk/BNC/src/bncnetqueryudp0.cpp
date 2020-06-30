/* -------------------------------------------------------------------------
 * BKG NTRIP Client
 * -------------------------------------------------------------------------
 *
 * Class:      bncNetQueryUdp0
 *
 * Purpose:    Blocking Network Requests (plain UDP, no NTRIP)
 *
 * Author:     L. Mervart
 *
 * Created:    04-Feb-2009
 *
 * Changes:    
 *
 * -----------------------------------------------------------------------*/

#include "bncnetqueryudp0.h"

// Constructor
////////////////////////////////////////////////////////////////////////////
bncNetQueryUdp0::bncNetQueryUdp0() {
  _udpSocket = 0;
  _eventLoop = new QEventLoop(this);
}

// Destructor
////////////////////////////////////////////////////////////////////////////
bncNetQueryUdp0::~bncNetQueryUdp0() {
  delete _eventLoop;
  delete _udpSocket;
}

// 
////////////////////////////////////////////////////////////////////////////
void bncNetQueryUdp0::stop() {
  _eventLoop->quit();
  _status = finished;
}

// 
////////////////////////////////////////////////////////////////////////////
void bncNetQueryUdp0::waitForRequestResult(const QUrl&, QByteArray&) {
}

// 
////////////////////////////////////////////////////////////////////////////
void bncNetQueryUdp0::waitForReadyRead(QByteArray& outData) {

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

  outData.append(datagram);
}

// Connect to Caster, send the Request
////////////////////////////////////////////////////////////////////////////
void bncNetQueryUdp0::startRequest(const QUrl& url, const QByteArray& /* gga */) {

  _status = running;

  delete _udpSocket;
  _udpSocket = new QUdpSocket();
  _udpSocket->bind(url.port());

  connect(_udpSocket, SIGNAL(readyRead()), _eventLoop, SLOT(quit()));
}

void bncNetQueryUdp0::keepAliveRequest(const QUrl& /* url */, const QByteArray& /* gga */) {
}
