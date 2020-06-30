#ifndef BNCNETQUERY_H
#define BNCNETQUERY_H

#include <QByteArray>
#include <QUrl>

#include "bncconst.h"
#include "bnccore.h"

class bncNetQuery : public QObject {
 Q_OBJECT

 public:
  enum queryStatus {init, running, finished, error};

  bncNetQuery() {
    connect(this,     SIGNAL(newMessage(QByteArray,bool)), 
            BNC_CORE, SLOT(slotMessage(const QByteArray,bool)));
  }
  virtual ~bncNetQuery() {}
  
  virtual void stop() = 0;
  virtual void waitForRequestResult(const QUrl& url, QByteArray& outData) = 0;
  virtual void startRequest(const QUrl& url, const QByteArray& gga) = 0;
  virtual void keepAliveRequest(const QUrl& url, const QByteArray& gga) = 0;
  virtual void waitForReadyRead(QByteArray& outData) = 0;
  
  void sendNMEA(const QByteArray& gga) {
    keepAliveRequest(_url, gga);
  }

  queryStatus status() const {return _status;}

 signals:
  void newMessage(QByteArray msg, bool showOnScreen);

 private slots:

 protected:
  queryStatus _status;
  int         _timeOut;
  QUrl        _url;
};

#endif
