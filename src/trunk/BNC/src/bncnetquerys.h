#ifndef BNCSNETQUERYS_H
#define BNCSNETQUERYS_H

#include "bncnetquery.h"
#include "serial/qextserialport.h"

class bncNetQueryS : public bncNetQuery {
 public:
  bncNetQueryS();
  virtual ~bncNetQueryS();

  virtual void stop();
  virtual void waitForRequestResult(const QUrl& url, QByteArray& outData);
  virtual void startRequest(const QUrl& url, const QByteArray& gga);
  virtual void keepAliveRequest(const QUrl& url, const QByteArray& gga);
  virtual void waitForReadyRead(QByteArray& outData);

 private:
  QextSerialPort* _serialPort;
};

#endif
