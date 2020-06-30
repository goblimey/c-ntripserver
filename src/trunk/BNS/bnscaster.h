#ifndef BNSCASTER_H
#define BNSCASTER_H

#include <QtNetwork>

class t_bnscaster : public QObject {
 Q_OBJECT
 public:
  t_bnscaster(const QString& mountpoint, const QString& outFileName, int ic);
  t_bnscaster(const QString& mountpoint);
  virtual ~t_bnscaster();
  void open();
  void write(char* buffer, unsigned len);
  void printAscii(const QString& line);
  bool usedSocket() const {return _outSocket;}
  QString crdTrafo() const {return _crdTrafo;}
  bool CoM() const {return _CoM;}
  int  ic() const {return _ic;}

 signals:
  void error(const QByteArray msg);
  void newMessage(const QByteArray msg);

 private:
  QString      _mountpoint;
  QTcpSocket*  _outSocket;
  int          _sOpenTrial;
  QDateTime    _outSocketOpenTime;
  QFile*       _outFile;
  QTextStream* _outStream;
  QString      _crdTrafo;
  bool         _CoM;
  int          _ic;
};

#endif
