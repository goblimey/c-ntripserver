#ifndef BNCNETQUERYRTP_H
#define BNCNETQUERYRTP_H

#include <QUdpSocket>

#include "bncnetquery.h"

class bncNetQueryRtp : public bncNetQuery {
 Q_OBJECT
 public:
  bncNetQueryRtp();
  virtual ~bncNetQueryRtp();

  virtual void stop();
  virtual void waitForRequestResult(const QUrl& url, QByteArray& outData);
  virtual void startRequest(const QUrl& url, const QByteArray& gga);
  virtual void keepAliveRequest(const QUrl& url, const QByteArray& gga);
  virtual void waitForReadyRead(QByteArray& outData);

 private slots:
  void slotKeepAlive();

 private:
  QTcpSocket* _socket;
  QUdpSocket* _udpSocket;
  QEventLoop* _eventLoop;
  QByteArray  _session;
  int         _CSeq;
};

#endif
