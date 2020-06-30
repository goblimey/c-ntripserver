#ifndef BNCRTNETUPLOADCASTER_H
#define BNCRTNETUPLOADCASTER_H

#include <newmat.h>
#include "bncuploadcaster.h"
#include "bnctime.h"
#include "ephemeris.h"
extern "C" {
#include "clock_orbit_rtcm.h"
}

class bncEphUser;
class bncoutf;
class bncClockRinex;
class bncSP3;

class bncRtnetUploadCaster : public bncUploadCaster {
 Q_OBJECT
 public:
  bncRtnetUploadCaster(const QString& mountpoint,
                  const QString& outHost, int outPort,
                  const QString& ntripVersion,
                  const QString& userName,const QString& password,
                  const QString& crdTrafo, bool  CoM,
                  const QString& sp3FileName,
                  const QString& rnxFileName,
                  int PID, int SID, int IOD, int iRow);
  void decodeRtnetStream(char* buffer, int bufLen);
 protected:
  virtual ~bncRtnetUploadCaster();
 private:
  t_irc processSatellite(const t_eph* eph, int GPSweek,
                        double GPSweeks, const QString& prn,
                        const ColumnVector& rtnAPC,
                        double ura,
                        const ColumnVector& rtnClk,
                        const ColumnVector& rtnVel,
                        const ColumnVector& rtnCoM,
                        const ColumnVector& rtnClkSig,
                        struct ClockOrbit::SatData* sd,
                        QString& outLine);
  void crdTrafo(int GPSWeek, ColumnVector& xyz, double& dc);

  int determineUpdateInd(double samplingRate);

  QString        _casterID;
  bncEphUser*    _ephUser;
  QString        _rtnetStreamBuffer;
  QString        _crdTrafo;
  bool           _CoM;
  int            _PID;
  int            _SID;
  int            _IOD;
  int            _samplRtcmClkCorr;
  double         _samplRtcmEphCorr;
  double         _dx;
  double         _dy;
  double         _dz;
  double         _dxr;
  double         _dyr;
  double         _dzr;
  double         _ox;
  double         _oy;
  double         _oz;
  double         _oxr;
  double         _oyr;
  double         _ozr;
  double         _sc;
  double         _scr;
  double         _t0;
  bncClockRinex* _rnx;
  bncSP3*        _sp3;
  QMap<QString, const t_eph*>* _usedEph;
};

struct phaseBiasesSat {
  phaseBiasesSat() {
    yawAngle = 0.0;
    yawRate = 0.0;
  }
  double yawAngle;
  double yawRate;
};

struct phaseBiasSignal {
  phaseBiasSignal() {
    bias      = 0.0;
    integerIndicator     = 0;
    wlIndicator          = 0;
    discontinuityCounter = 0;
  }
  QString type;
  double bias;
  unsigned int integerIndicator;
  unsigned int wlIndicator;
  unsigned int discontinuityCounter;
};

#endif
