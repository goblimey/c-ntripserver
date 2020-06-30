/* -------------------------------------------------------------------------
 * BKG NTRIP Client
 * -------------------------------------------------------------------------
 *
 * Class:      bncnetquerys
 *
 * Purpose:    Serial Communication Requests, no NTRIP
 *
 * Author:     G. Weber
 *
 * Created:    8-Mar-2009
 *
 * Changes:    
 *
 * -----------------------------------------------------------------------*/

#include "bncnetquerys.h"
#include "bncversion.h"

using namespace std;

// Constructor
////////////////////////////////////////////////////////////////////////////
bncNetQueryS::bncNetQueryS() {

  _serialPort = 0;

}

// Destructor
////////////////////////////////////////////////////////////////////////////
bncNetQueryS::~bncNetQueryS() {
  delete _serialPort;
}

// 
////////////////////////////////////////////////////////////////////////////
void bncNetQueryS::stop() {
#ifndef sparc
  if (_serialPort) {
  }
#endif
  _status = finished;
} 

// 
/////////////////////////////////////////////////////////////////////////////
void bncNetQueryS::waitForRequestResult(const QUrl&, QByteArray&) {
}

// 
////////////////////////////////////////////////////////////////////////////
void bncNetQueryS::waitForReadyRead(QByteArray& outData) {
  if (_serialPort) {
    while (true) {
      int nb = _serialPort->bytesAvailable();
      if (nb > 0) {
        outData = _serialPort->read(nb);
        return;
      }
    }
  }
}

// Connect to Serial Port
////////////////////////////////////////////////////////////////////////////
void bncNetQueryS::startRequest(const QUrl& url, const QByteArray& gga) {

  QByteArray dummy_gga = gga;

  _url = url;
  if (_url.scheme().isEmpty()) {
    _url.setScheme("http");
  }
  if (_url.path().isEmpty()) {
    _url.setPath("/");
  }

  QString hlp;
  QStringList hlpL;
  hlp = _url.host().toLatin1().replace("-"," ");
  hlpL = hlp.split(" ");

  // Serial Port
  // -----------
  QString _portString;
  if (hlpL.size() == 6) {
    _portString = hlpL[hlpL.size()-6].replace("com","COM");
  } else {
    _portString = "/" + hlpL[hlpL.size()-7] + "/" + hlpL[hlpL.size()-6].replace("ttys","ttyS");
  }
  _serialPort = new QextSerialPort(_portString);

  // Baud Rate
  // ---------
  hlp = hlpL[hlpL.size()-1];
  if      (hlp == "110") {
    _serialPort->setBaudRate(BAUD110);
  }
  else if (hlp == "300") {
    _serialPort->setBaudRate(BAUD300);
  }
  else if (hlp == "600") {
    _serialPort->setBaudRate(BAUD600);
  }
  else if (hlp == "1200") {
    _serialPort->setBaudRate(BAUD1200);
  }
  else if (hlp == "2400") {
    _serialPort->setBaudRate(BAUD2400);
  }
  else if (hlp == "4800") {
    _serialPort->setBaudRate(BAUD4800);
  }
  else if (hlp == "9600") {
    _serialPort->setBaudRate(BAUD9600);
  }
  else if (hlp == "19200") {
    _serialPort->setBaudRate(BAUD19200);
  }
  else if (hlp == "38400") {
    _serialPort->setBaudRate(BAUD38400);
  }
  else if (hlp == "57600") {
    _serialPort->setBaudRate(BAUD57600);
  }
  else if (hlp == "115200") {
    _serialPort->setBaudRate(BAUD115200);
  }

  // Parity
  // ------
  hlp = hlpL[hlpL.size()-4].toUpper();
  if      (hlp == "NONE") {
    _serialPort->setParity(PAR_NONE);
  }
  else if (hlp == "ODD") {
    _serialPort->setParity(PAR_ODD);
  }
  else if (hlp == "EVEN") {
    _serialPort->setParity(PAR_EVEN);
  }
  else if (hlp == "SPACE") {
    _serialPort->setParity(PAR_SPACE);
  }

  // Data Bits
  // ---------
  hlp = hlpL[hlpL.size()-5];
  if      (hlp == "5") {
    _serialPort->setDataBits(DATA_5);
  }
  else if (hlp == "6") {
    _serialPort->setDataBits(DATA_6);
  }
  else if (hlp == "7") {
    _serialPort->setDataBits(DATA_7);
  }
  else if (hlp == "8") {
    _serialPort->setDataBits(DATA_8);
  }

  // Stop Bits
  // ---------
  hlp = hlpL[hlpL.size()-3];
  if      (hlp == "1") {
    _serialPort->setStopBits(STOP_1);
  }
  else if (hlp == "2") {
    _serialPort->setStopBits(STOP_2);
  }

  // Flow Control
  // ------------
  hlp = hlpL[hlpL.size()-2].toUpper();
  if (hlp == "XONXOFF") {
    _serialPort->setFlowControl(FLOW_XONXOFF);
  }
  else if (hlp == "HARDWARE") {
    _serialPort->setFlowControl(FLOW_HARDWARE);
  }
  else {
    _serialPort->setFlowControl(FLOW_OFF);
  }

  _status = running;

  _serialPort->open(QIODevice::ReadWrite|QIODevice::Unbuffered);
  if (!_serialPort->isOpen()) {
    delete _serialPort;
    _serialPort = 0;
    _status = error;
    emit newMessage(_url.path().toLatin1().replace(0,1,"") + ": Cannot open serial port " + _portString.toLatin1(), true);
    return;
  }
}

void bncNetQueryS::keepAliveRequest(const QUrl& /* url */, const QByteArray& /* gga */) {
}
