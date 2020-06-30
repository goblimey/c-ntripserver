#ifndef BNCNETQUERYV0_H
#define BNCNETQUERYV0_H

#include "bncnetquery.h"

class bncNetQueryV0 : public bncNetQuery {
 public:
  bncNetQueryV0();
  virtual ~bncNetQueryV0();

  virtual void stop();
  virtual void waitForRequestResult(const QUrl& url, QByteArray& outData);
  virtual void startRequest(const QUrl& url, const QByteArray& gga);
  virtual void keepAliveRequest(const QUrl& url, const QByteArray& gga);
  virtual void waitForReadyRead(QByteArray& outData);

 private:
  QTcpSocket* _socket;
};

#endif
