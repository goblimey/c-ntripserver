/* -------------------------------------------------------------------------
 * BKG NTRIP Server
 * -------------------------------------------------------------------------
 *
 * Class:      bnscaster
 *
 * Purpose:    Connection to NTRIP Caster
 *
 * Author:     L. Mervart
 *
 * Created:    27-Aug-2008
 *
 * Changes:    
 *
 * -----------------------------------------------------------------------*/

#include <math.h>
#include "bnscaster.h" 
#include "bnssettings.h"
#include "bnsversion.h"

using namespace std;

// Constructor
////////////////////////////////////////////////////////////////////////////
t_bnscaster::t_bnscaster(const QString& mountpoint, const QString& outFileName,
                         int ic) {

  bnsSettings settings;

  _mountpoint = mountpoint;
  _ic         = ic;
  _outSocket  = 0;
  _sOpenTrial = 0;

  if (outFileName.isEmpty()) {
    _outFile   = 0;
    _outStream = 0;
  }
  else {
    _outFile = new QFile(outFileName);

    QIODevice::OpenMode oMode;
    if (Qt::CheckState(settings.value("fileAppend").toInt()) == Qt::Checked) {
      oMode = QIODevice::WriteOnly | QIODevice::Unbuffered | QIODevice::Append;
    }
    else {
      oMode = QIODevice::WriteOnly | QIODevice::Unbuffered;
    }

    if (_outFile->open(oMode)) {
      _outStream = new QTextStream(_outFile);
    }
  }

  // Reference frame
  // ---------------
  _crdTrafo = settings.value(QString("refSys_%1").arg(_ic)).toString();

  if ( Qt::CheckState(settings.value(QString("CoM_%1").arg(_ic)).toInt()) 
       == Qt::Checked ) {
    _CoM = true;
  }
  else {
    _CoM = false;
  }
}

// Constructor
////////////////////////////////////////////////////////////////////////////
t_bnscaster::t_bnscaster(const QString& mountpoint) {

  bnsSettings settings;

  _mountpoint = mountpoint;
  _ic         = 0;
  _outSocket  = 0;
  _sOpenTrial = 0;
  _outFile    = 0;
  _outStream  = 0;
  _crdTrafo   = "";
  _CoM        = false;
}

// Destructor
////////////////////////////////////////////////////////////////////////////
t_bnscaster::~t_bnscaster() {
  delete _outSocket;
  delete _outStream;
  delete _outFile;
}

// Start the Communication with NTRIP Caster
////////////////////////////////////////////////////////////////////////////
void t_bnscaster::open() {

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

  bnsSettings settings;
  _outSocket = new QTcpSocket();
  QString password;
  if (_ic == 1) {
    _outSocket->connectToHost(settings.value("outHost1").toString(),
                              settings.value("outPort1").toInt());
    password = settings.value("password1").toString();
  }
  if (_ic == 2) {
    _outSocket->connectToHost(settings.value("outHost2").toString(),
                              settings.value("outPort2").toInt());
    password = settings.value("password2").toString();
  }
  if (_ic == 3) {
    _outSocket->connectToHost(settings.value("outHost3").toString(),
                              settings.value("outPort3").toInt());
    password = settings.value("password3").toString();
  }

  if (_ic == 4) {
    _outSocket->connectToHost(settings.value("outHost4").toString(),
                              settings.value("outPort4").toInt());
    password = settings.value("password4").toString();
  }

  if (_ic == 5) {
    _outSocket->connectToHost(settings.value("outHost5").toString(),
                              settings.value("outPort5").toInt());
    password = settings.value("password5").toString();
  }

  if (_ic == 6) {
    _outSocket->connectToHost(settings.value("outHost6").toString(),
                              settings.value("outPort6").toInt());
    password = settings.value("password6").toString();
  }

  if (_ic == 7) {
    _outSocket->connectToHost(settings.value("outHost7").toString(),
                              settings.value("outPort7").toInt());
    password = settings.value("password7").toString();
  }

  if (_ic == 8) {
    _outSocket->connectToHost(settings.value("outHost8").toString(),
                              settings.value("outPort8").toInt());
    password = settings.value("password8").toString();
  }

  if (_ic == 9) {
    _outSocket->connectToHost(settings.value("outHost9").toString(),
                              settings.value("outPort9").toInt());
    password = settings.value("password9").toString();
  }

  if (_ic == 10) {
    _outSocket->connectToHost(settings.value("outHost10").toString(),
                              settings.value("outPort10").toInt());
    password = settings.value("password10").toString();
  }

  if (_ic == 0) {
    _outSocket->connectToHost(settings.value("outHostEph").toString(),
                              settings.value("outPortEph").toInt());
    password = settings.value("passwordEph").toString();
  }

  const int timeOut = 5000;  // 5 seconds
  if (!_outSocket->waitForConnected(timeOut)) {
    delete _outSocket;
    _outSocket = 0;
    emit(newMessage("Broadcaster: Connect timeout"));
    return;
  }

  QByteArray msg = "SOURCE " + password.toAscii() + " /" + 
                   _mountpoint.toAscii() + "\r\n" +
                   "Source-Agent: NTRIP BNS/" BNSVERSION "\r\n\r\n";

  _outSocket->write(msg);
  _outSocket->waitForBytesWritten();

  _outSocket->waitForReadyRead();
  QByteArray ans = _outSocket->readLine();

  if (ans.indexOf("OK") == -1) {
    delete _outSocket;
    _outSocket = 0;
    emit(newMessage("Broadcaster: Connection broken"));
  }
  else {
    emit(newMessage("Broadcaster: Connection opened"));
    _sOpenTrial = 0;
  }
}

// Write buffer
////////////////////////////////////////////////////////////////////////////
void t_bnscaster::write(char* buffer, unsigned len) {
  if (_outSocket) {
    _outSocket->write(buffer, len);
    _outSocket->flush();
  }
}

// Print Ascii Output
////////////////////////////////////////////////////////////////////////////
void t_bnscaster::printAscii(const QString& line) {
  if (_outStream) {
    *_outStream << line;
     _outStream->flush();
  }
}
