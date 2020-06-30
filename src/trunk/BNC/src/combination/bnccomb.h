
#ifndef BNCCOMB_H
#define BNCCOMB_H

#include <fstream>
#include <newmat.h>
#include "bncephuser.h"
#include "satObs.h"

class bncRtnetDecoder;
class bncSP3;
class bncAntex;

class bncComb : public QObject {
 Q_OBJECT
 public:
  bncComb();
  virtual ~bncComb();
  static bncComb* instance();
  int  nStreams() const {return _ACs.size();}

 public slots:
  void slotProviderIDChanged(QString mountPoint);
  void slotNewOrbCorrections(QList<t_orbCorr> orbCorrections);
  void slotNewClkCorrections(QList<t_clkCorr> clkCorrections);

 signals:
  void newMessage(QByteArray msg, bool showOnScreen);
  void newOrbCorrections(QList<t_orbCorr>);
  void newClkCorrections(QList<t_clkCorr>);

 private:
  enum e_method{singleEpoch, filter};

  class cmbParam {
   public:
    enum parType {offACgps, offACglo, offACSat, clkSat};
    cmbParam(parType type_, int index_, const QString& ac_, const QString& prn_);
    ~cmbParam();
    double partial(const QString& AC_, const QString& prn_);
    QString toString() const;
    parType type;
    int     index;
    QString AC;
    QString prn;
    double  xx;
    double  sig0;
    double  sigP;
    bool    epoSpec;
    const t_eph* eph;
  };

  class cmbAC {
   public:
    cmbAC() {
      weight = 0.0;
      numObs = 0;
    }
    ~cmbAC() {}
    QString  mountPoint;
    QString  name;
    double   weight;
    unsigned numObs;
  };

  class cmbCorr {
   public:
    cmbCorr() {
      _eph        = 0;
      _iod        = 0;
      _dClkResult = 0.0;
    }
    ~cmbCorr() {}
    QString       _prn;
    bncTime       _time;
    unsigned long _iod;
    t_eph*        _eph;
    t_orbCorr     _orbCorr;
    t_clkCorr     _clkCorr;
    QString       _acName;
    double        _dClkResult;
    ColumnVector  _diffRao;
    QString ID() {return _acName + "_" + _prn;}
  };

  class cmbEpoch {
   public:
    cmbEpoch() {}
    ~cmbEpoch() {
      QVectorIterator<cmbCorr*> it(corrs);
      while (it.hasNext()) {
        delete it.next();
      }
    }
    QVector<cmbCorr*> corrs;
  };

  void  processEpoch();
  t_irc processEpoch_filter(QTextStream& out, QMap<QString, cmbCorr*>& resCorr,
                            ColumnVector& dx);
  t_irc processEpoch_singleEpoch(QTextStream& out, QMap<QString, cmbCorr*>& resCorr,
                                 ColumnVector& dx);
  t_irc createAmat(Matrix& AA, ColumnVector& ll, DiagonalMatrix& PP,
                   const ColumnVector& x0, QMap<QString, cmbCorr*>& resCorr);
  void  dumpResults(const QMap<QString, cmbCorr*>& resCorr);
  void  printResults(QTextStream& out, const QMap<QString, cmbCorr*>& resCorr);
  void  switchToLastEph(t_eph* lastEph, cmbCorr* corr);
  t_irc checkOrbits(QTextStream& out);
  QVector<cmbCorr*>& corrs() {return _buffer[_resTime].corrs;}

  QMutex                                 _mutex;
  QList<cmbAC*>                          _ACs;
  bncTime                                _resTime;
  QVector<cmbParam*>                     _params;
  QMap<bncTime, cmbEpoch>                _buffer;
  bncRtnetDecoder*                       _rtnetDecoder;
  SymmetricMatrix                        _QQ;
  QByteArray                             _log;
  bncAntex*                              _antex;
  double                                 _MAXRES;
  QString                                _masterOrbitAC;
  unsigned                               _masterMissingEpochs;
  e_method                               _method;
  bool                                   _useGlonass;
  int                                    _cmbSampl;
  QMap<QString, QMap<t_prn, t_orbCorr> > _orbCorrections;
  bncEphUser                             _ephUser;
};

#define BNC_CMB (bncComb::instance())

#endif
