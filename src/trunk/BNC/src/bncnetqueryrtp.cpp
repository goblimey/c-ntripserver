/* -------------------------------------------------------------------------
 * BKG NTRIP Client
 * -------------------------------------------------------------------------
 *
 * Class:      bncNetQueryRtp
 *
 * Purpose:    Blocking Network Requests (NTRIP Version 2 with RTSP)
 *
 * Author:     L. Mervart
 *
 * Created:    27-Dec-2008
 *
 * Changes:
 *
 * -----------------------------------------------------------------------*/

#include <iostream>
#include <iomanip>

#include "bncnetqueryrtp.h"
#include "bncsettings.h"
#include "bncversion.h"

using namespace std;

// Constructor
////////////////////////////////////////////////////////////////////////////
bncNetQueryRtp::bncNetQueryRtp() {
  _socket    = 0;
  _udpSocket = 0;
  _CSeq      = 0;
  _eventLoop = new QEventLoop(this);
}

// Destructor
////////////////////////////////////////////////////////////////////////////
bncNetQueryRtp::~bncNetQueryRtp() {
  delete _eventLoop;
  delete _socket;
  delete _udpSocket;
}

//
////////////////////////////////////////////////////////////////////////////
void bncNetQueryRtp::stop() {
  _eventLoop->quit();
  _status = finished;
  if (_socket) {
    QByteArray reqStr = "TEARDOWN " + _url.toEncoded() + " RTSP/1.0\r\n"
                      + "CSeq: " + QString("%1").arg(++_CSeq).toLatin1() + "\r\n"
                      + "Session: " + _session + "\r\n"
                      + "\r\n";
    _socket->write(reqStr, reqStr.length());
  }
}

//
////////////////////////////////////////////////////////////////////////////
void bncNetQueryRtp::slotKeepAlive() {
  if (_socket) {
    QByteArray reqStr = "GET_PARAMETER " + _url.toEncoded() + " RTSP/1.0\r\n"
                      + "CSeq: " + QString("%1").arg(++_CSeq).toLatin1() + "\r\n"
                      + "Session: " + _session + "\r\n"
                      + "\r\n";
    _socket->write(reqStr, reqStr.length());
  }
  QTimer::singleShot(30000, this, SLOT(slotKeepAlive()));
}

//
////////////////////////////////////////////////////////////////////////////
void bncNetQueryRtp::waitForRequestResult(const QUrl&, QByteArray&) {
}

//
////////////////////////////////////////////////////////////////////////////
void bncNetQueryRtp::waitForReadyRead(QByteArray& outData) {

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
}

// Connect to Caster, send the Request
////////////////////////////////////////////////////////////////////////////
void bncNetQueryRtp::startRequest(const QUrl& url, const QByteArray& gga) {

  const int timeOut = 5000;

  _status = running;

  delete _socket;
  _socket = new QTcpSocket();

  // Default scheme
  // --------------
  _url = url;
  _url.setScheme("rtsp");

  // Connect the Socket
  // ------------------
  bncSettings settings;
  QString proxyHost = settings.value("proxyHost").toString();
  int     proxyPort = settings.value("proxyPort").toInt();

  if ( proxyHost.isEmpty() ) {
    _socket->connectToHost(_url.host(), _url.port());
  }
  else {
    _socket->connectToHost(proxyHost, proxyPort);
  }

  // Send Request 1
  // --------------
  if (_socket->waitForConnected(timeOut)) {
    QString uName = QUrl::fromPercentEncoding(_url.userName().toLatin1());
    QString passW = QUrl::fromPercentEncoding(_url.password().toLatin1());
    QByteArray userAndPwd;

    if(!uName.isEmpty() || !passW.isEmpty()) {
      userAndPwd = "Authorization: Basic " + (uName.toLatin1() + ":" +
      passW.toLatin1()).toBase64() + "\r\n";
    }

    // Setup the RTSP Connection
    // -------------------------
    delete _udpSocket;
    _udpSocket = new QUdpSocket();
    _udpSocket->bind(0);
    connect(_udpSocket, SIGNAL(readyRead()), _eventLoop, SLOT(quit()));
    QByteArray clientPort = QString("%1").arg(_udpSocket->localPort()).toLatin1();

    QByteArray reqStr;
    reqStr = "SETUP " + _url.toEncoded() + " RTSP/1.0\r\n"
           + "CSeq: " + QString("%1").arg(++_CSeq).toLatin1() + "\r\n"
           + "Ntrip-Version: Ntrip/2.0\r\n"
           + "Ntrip-Component: Ntripclient\r\n"
           + "User-Agent: NTRIP BNC/" BNCVERSION " (" BNC_OS ")\r\n"
           + "Transport: RTP/GNSS;unicast;client_port=" + clientPort + "\r\n"
           + userAndPwd;
    if (!gga.isEmpty()) {
      reqStr += "Ntrip-GGA: " + gga + "\r\n";
    }
    reqStr += "\r\n";

    _socket->write(reqStr, reqStr.length());

    // Read Server Answer 1
    // --------------------
    if (_socket->waitForBytesWritten(timeOut)) {
      if (_socket->waitForReadyRead(timeOut)) {
        QTextStream in(_socket);
        QByteArray serverPort;
        QString line = in.readLine();
        while (!line.isEmpty()) {
          if (line.indexOf("Session:") == 0) {
            _session = line.mid(9).toLatin1();
          }
          int iSrv = line.indexOf("server_port=");
          if (iSrv != -1) {
            serverPort = line.mid(iSrv+12).toLatin1();
          }
          line = in.readLine();
        }

        // Send Request 2
        // --------------
        if (!_session.isEmpty()) {

          // Send initial RTP packet for firewall handling
          // ---------------------------------------------
          if (!serverPort.isEmpty()) {
            unsigned sessInt = _session.toInt();
            char rtpbuffer[12];
            rtpbuffer[0]  = 128;
            rtpbuffer[1]  =  96;
            rtpbuffer[2]  =   0;
            rtpbuffer[3]  =   0;
            rtpbuffer[4]  =   0;
            rtpbuffer[5]  =   0;
            rtpbuffer[6]  =   0;
            rtpbuffer[7]  =   0;
            rtpbuffer[8]  = (sessInt >> 24) & 0xFF;
            rtpbuffer[9]  = (sessInt >> 16) & 0xFF;
            rtpbuffer[10] = (sessInt >>  8) & 0xFF;
            rtpbuffer[11] = (sessInt      ) & 0xFF;

            _udpSocket->writeDatagram(rtpbuffer, 12,
                              _socket->peerAddress(), serverPort.toInt());
          }

          reqStr = "PLAY " + _url.toEncoded() + " RTSP/1.0\r\n"
                 + "CSeq: " + QString("%1").arg(++_CSeq).toLatin1() + "\r\n"
                 + "Session: " + _session + "\r\n"
                 + "\r\n";
          _socket->write(reqStr, reqStr.length());

          // Read Server Answer 2
          // --------------------
          if (_socket->waitForBytesWritten(timeOut)) {
            if (_socket->waitForReadyRead(timeOut)) {
              QTextStream in(_socket);
              line = in.readLine();
              while (!line.isEmpty()) {
                if (line.indexOf("200 OK") != -1) {
                  emit newMessage(_url.toEncoded().replace(0,1,"")
                            + ": UDP connection established", true);
                  slotKeepAlive();
                  return;
                }
                line = in.readLine();
              }
            }
          }
        }
      }
    }
  }

  delete _socket;
  _socket = 0;
  _status = error;
  emit newMessage(_url.toEncoded().replace(0,1,"")
                  + ": NetQuery, waiting for connect", true);
}

void bncNetQueryRtp::keepAliveRequest(const QUrl& /* url */, const QByteArray& /* gga */) {
}
