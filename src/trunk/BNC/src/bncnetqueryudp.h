#ifndef BNCNETQUERYUDP_H
#define BNCNETQUERYUDP_H

#include <QHostAddress>
#include <QUdpSocket>

#include "bncnetquery.h"

class bncNetQueryUdp : public bncNetQuery {
 Q_OBJECT
 public:
  bncNetQueryUdp();
  virtual ~bncNetQueryUdp();

  virtual void stop();
  virtual void waitForRequestResult(const QUrl& url, QByteArray& outData);
  virtual void startRequest(const QUrl& url, const QByteArray& gga);
  virtual void keepAliveRequest(const QUrl& url, const QByteArray& gga);
  virtual void waitForReadyRead(QByteArray& outData);

 private slots:
  void slotKeepAlive();

 private:
  QUdpSocket*  _udpSocket;
  QEventLoop*  _eventLoop;
  QHostAddress _address;
  int          _port;
  char         _keepAlive[12];
  unsigned     _session;
};

#endif
