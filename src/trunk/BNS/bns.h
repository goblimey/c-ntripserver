#ifndef BNS_H
#define BNS_H

#include <newmat.h>

#include <QtNetwork>
#include <QThread>

#include "bnseph.h"
#include "bnscaster.h"
extern "C" {
#include "clock_orbit_rtcm.h"
}

class bnsRinex;
class bnsSP3;

class t_ephPair {
 public:
  t_ephPair() {
    eph    = 0;
    oldEph = 0;
  }

  ~t_ephPair() {
    delete eph;
    delete oldEph;
  }

  ColumnVector xx;
  t_eph* eph;
  t_eph* oldEph;
};

class t_bns : public QThread {
 Q_OBJECT
 public:
  t_bns(QObject* parent = 0);
  virtual ~t_bns();  
  virtual void run();  

 signals:
  void newClkBytes(int nBytes);
  void newEphBytes(int nBytes);
  void newOutBytes1(int nBytes);
  void newOutBytes2(int nBytes);
  void newOutBytes3(int nBytes);
  void newOutBytes4(int nBytes);
  void newOutBytes5(int nBytes);
  void newOutBytes6(int nBytes);
  void newOutBytes7(int nBytes);
  void newOutBytes8(int nBytes);
  void newOutBytes9(int nBytes);
  void newOutBytes10(int nBytes);
  void newOutEphBytes(int nBytes);
  void newMessage(const QByteArray msg);
  void error(const QByteArray msg);
  void moveSocket(QThread* tt);
 
 private slots:
  void slotNewEph(t_eph* ep, int nBytes);
  void slotNewConnection();
  void slotMessage(const QByteArray msg);
  void slotError(const QByteArray msg);
  void slotMoveSocket(QThread* tt);

 private:
  void deleteBnsEph();
  void openCaster();
  void readEpoch();
  void readRecords();
  void processSatellite(int iCaster, const QString trafo, bool CoM,
                        t_eph* ep, int GPSweek, double GPSweeks, 
                        const QString& prn, const ColumnVector& xx, 
                        struct ClockOrbit::SatData* sd, QString& outLine);
  void crdTrafo(int GPSWeek, ColumnVector& xyz, const QString trafo);

  QTcpServer*               _clkServer;
  QTcpSocket*               _clkSocket;
  QList<t_bnscaster*>       _caster;
  t_bnscaster*              _casterEph;
  QFile*                    _logFile;
  QTextStream*              _logStream;
  QFile*                    _echoFile;
  QTextStream*              _echoStream;
  t_bnseph*                 _bnseph;
  QMutex                    _mutex;
  QMutex                    _mutexmesg;
  QMap<QString, t_ephPair*> _ephList;
  bnsRinex*                 _rnx;
  bnsSP3*                   _sp3;
  QByteArray                _clkLine;
  bool                      _append;
  
  int    _GPSweek;
  double _GPSweeks;
  int    _year;
  int    _month;
  int    _day;
  int    _hour;
  int    _min;
  double _sec;

  double _dx;
  double _dy;
  double _dz;
  double _dxr;
  double _dyr;
  double _dzr;
  double _ox;
  double _oy;
  double _oz;
  double _oxr;
  double _oyr;
  double _ozr;
  double _sc;
  double _scr;
  double _t0;
};
#endif
