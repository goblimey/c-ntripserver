#ifndef BNCNETQUERYUDP0_H
#define BNCNETQUERYUDP0_H

#include <QUdpSocket>

#include "bncnetquery.h"

class bncNetQueryUdp0 : public bncNetQuery {
 Q_OBJECT
 public:
  bncNetQueryUdp0();
  virtual ~bncNetQueryUdp0();

  virtual void stop();
  virtual void waitForRequestResult(const QUrl& url, QByteArray& outData);
  virtual void startRequest(const QUrl& url, const QByteArray& gga);
  virtual void keepAliveRequest(const QUrl& url, const QByteArray& gga);
  virtual void waitForReadyRead(QByteArray& outData);
 private:
  QUdpSocket*  _udpSocket;
  QEventLoop*  _eventLoop;
};

#endif
