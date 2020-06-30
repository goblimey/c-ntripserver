#ifndef BNCUPLOADCASTER_H
#define BNCUPLOADCASTER_H

#include <QDateTime>
#include <QMutex>
#include <QTcpSocket>
#include <QNetworkProxy>
#include <QThread>

class bncUploadCaster : public QThread {
 Q_OBJECT
 public:
  bncUploadCaster(const QString& mountpoint,
      const QString& outHost, int outPort,
      const QString& ntripVersion,
      const QString& userName, const QString& password,
      int iRow, int rate);
  virtual void deleteSafely();
  void setOutBuffer(const QByteArray& outBuffer) {
    QMutexLocker locker(&_mutex);
    _outBuffer = outBuffer;
  }

 protected:
  virtual    ~bncUploadCaster();
  QMutex     _mutex;
  QByteArray _outBuffer;

 signals:
  void newMessage(const QByteArray msg, bool showOnScreen);
  void newBytes(QByteArray staID, double nbyte);

 private:
  void         open();
  virtual void run();
  bool        _isToBeDeleted;
  QString     _mountpoint;
  QString     _outHost;
  int         _outPort;
  QString     _userName;
  QString     _password;
  QString     _ntripVersion;
  bool        _secure;
  QTcpSocket* _outSocket;
  int         _sOpenTrial;
  QDateTime   _outSocketOpenTime;
  int         _iRow;
  int         _rate;
};

#endif
