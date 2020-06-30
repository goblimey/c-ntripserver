#ifndef PPPRUN_H
#define PPPRUN_H

#include <deque>
#include <vector>
#include <QtCore>

#include "satObs.h"
#include "pppOptions.h"
#include "pppClient.h"

class t_rnxObsFile;
class t_rnxNavFile;
class t_corrFile;
class bncoutf;
class bncSinexTro;

namespace BNC_PPP {

class t_pppRun : public QObject {
 Q_OBJECT
 public:
  t_pppRun(const t_pppOptions* opt);
  ~t_pppRun();

  void processFiles();

  static QString nmeaString(char strType, const t_output& output);

 signals:
  void newMessage(QByteArray msg, bool showOnScreen);
  void newPosition(QByteArray staID, bncTime time, QVector<double> xx);
  void newNMEAstr(QByteArray staID, QByteArray str);
  void progressRnxPPP(int);
  void finishedRnxPPP();

 public slots:
  void slotNewGPSEph(t_ephGPS);
  void slotNewGlonassEph(t_ephGlo);
  void slotNewGalileoEph(t_ephGal);
  void slotNewBDSEph(t_ephBDS);
  void slotNewTec(t_vTec);
  void slotNewOrbCorrections(QList<t_orbCorr> orbCorr);
  void slotNewClkCorrections(QList<t_clkCorr> clkCorr);
  void slotNewCodeBiases(QList<t_satCodeBias> codeBiases);
  void slotNewPhaseBiases(QList<t_satPhaseBias> phaseBiases);
  void slotNewObs(QByteArray staID, QList<t_satObs> obsList);
  void slotSetSpeed(int speed);
  void slotSetStopFlag();
  void slotProviderIDChanged(QString mountPoint);

 private:
  class t_epoData {
   public:
    t_epoData() {}
    ~t_epoData() {
      for (unsigned ii = 0; ii < _satObs.size(); ii++) {
        delete _satObs[ii];
      }
    }
    bncTime                _time;
    std::vector<t_satObs*> _satObs;
  };

  QMutex                 _mutex;
  const t_pppOptions*    _opt;
  t_pppClient*           _pppClient;
  std::deque<t_epoData*> _epoData;
  bncTime                _lastClkCorrTime;
  t_rnxObsFile*          _rnxObsFile;
  t_rnxNavFile*          _rnxNavFile;
  t_corrFile*            _corrFile;
  int                    _speed;
  bool                   _stopFlag;
  bncoutf*               _logFile;
  bncoutf*               _nmeaFile;
  bncSinexTro*           _snxtroFile;
};

}

#endif
