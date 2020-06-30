#ifndef SATOBS_H
#define SATOBS_H

#include <string>
#include <vector>
#include <newmat.h>

#include <QtCore>

#include "bncconst.h"
#include "bnctime.h"
#include "t_prn.h"

class t_frqObs  {
 public:
  t_frqObs() {
    _code            = 0.0;
    _codeValid       = false;
    _phase           = 0.0;
    _phaseValid      = false;
    _doppler         = 0.0;
    _dopplerValid    = false;
    _snr             = 0.0;
    _snrValid        = false;
    _lockTime        = 0.0;
    _lockTimeValid   = false;    
    _slip            = false;
    _slipCounter     = 0;
    _biasJumpCounter = 0;
  }
  std::string       _rnxType2ch;
  double            _code;
  bool              _codeValid;
  double            _phase;
  bool              _phaseValid;
  double            _doppler;
  bool              _dopplerValid;
  double            _snr;
  bool              _snrValid;
  double            _lockTime;
  bool              _lockTimeValid;    
  bool              _slip;
  int               _slipCounter;
  int               _biasJumpCounter;
};

class t_satObs {
 public:
  t_satObs() {}
  t_satObs(const t_satObs& old) { // copy constructor (deep copy)
    _staID = old._staID;
    _prn   = old._prn;
    _time  = old._time;
    for (unsigned ii = 0; ii < old._obs.size(); ii++) {
      _obs.push_back(new t_frqObs(*old._obs[ii]));
    }
  }
  /**
   * Destructor of satellite measurement storage class
   */
  ~t_satObs()
  {
    clear();
  }

  /**
   * Cleanup function resets all elements to initial state.
   */
  inline void clear(void)
  {
    for (unsigned ii = 0; ii < _obs.size(); ii++)
      delete _obs[ii];
    _obs.clear();
    _obs.resize(0);
    _time.reset();
    _prn.clear();
    _staID.clear();
  }

  std::string            _staID;
  t_prn                  _prn;
  bncTime                _time;
  std::vector<t_frqObs*> _obs;
};

class t_orbCorr {
 public:
  t_orbCorr();
  static void writeEpoch(std::ostream* out, const QList<t_orbCorr>& corrList);
  static void readEpoch(const std::string& epoLine, std::istream& in, QList<t_orbCorr>& corrList);
  std::string    _staID;
  t_prn          _prn;
  unsigned int   _iod;
  bncTime        _time;
  unsigned int   _updateInt;
  char           _system;
  ColumnVector   _xr;
  ColumnVector   _dotXr;
};

class t_clkCorr {
 public:
  t_clkCorr();
  static void writeEpoch(std::ostream* out, const QList<t_clkCorr>& corrList);
  static void readEpoch(const std::string& epoLine, std::istream& in, QList<t_clkCorr>& corrList);
  std::string    _staID;
  t_prn          _prn;
  unsigned int   _iod;
  bncTime        _time;
  unsigned int   _updateInt;
  double         _dClk;
  double         _dotDClk;
  double         _dotDotDClk;
};

class t_URA {
 public:
  t_URA();
  static void writeEpoch(std::ostream* out, const QList<t_URA>& corrList);
  static void readEpoch(const std::string& epoLine, std::istream& in, QList<t_URA>& corrList);
  std::string    _staID;
  t_prn          _prn;
  unsigned int   _iod;
  bncTime        _time;
  unsigned int   _updateInt;
  double         _ura;
};

class t_frqCodeBias {
 public:
  t_frqCodeBias() {
    _value = 0.0;
  }
  std::string _rnxType2ch;
  double      _value;
};

class t_satCodeBias {
 public:
  static void writeEpoch(std::ostream* out, const QList<t_satCodeBias>& biasList);
  static void readEpoch(const std::string& epoLine, std::istream& in, QList<t_satCodeBias>& biasList);
  std::string                _staID;
  t_prn                      _prn;
  bncTime                    _time;
  unsigned int               _updateInt;
  std::vector<t_frqCodeBias> _bias;
};

class t_frqPhaseBias {
 public:
  t_frqPhaseBias() {
    _value                = 0.0;
    _fixIndicator         = 0;
    _fixWideLaneIndicator = 0;
    _jumpCounter          = 0;
  }
  std::string _rnxType2ch;
  double      _value;
  int         _fixIndicator;
  int         _fixWideLaneIndicator;
  int         _jumpCounter;
};

class t_satPhaseBias {
 public:
  t_satPhaseBias() {
    _updateInt  = 0;
    _dispBiasConstistInd = 0;
    _MWConsistInd = 0;
    _yaw        = 0.0;
    _yawRate    = 0.0;
  }
  static void writeEpoch(std::ostream* out, const QList<t_satPhaseBias>& biasList);
  static void readEpoch(const std::string& epoLine, std::istream& in, QList<t_satPhaseBias>& biasList);
  std::string                 _staID;
  t_prn                       _prn;
  bncTime                     _time;
  unsigned int                _updateInt;           // not satellite specific
  unsigned int                _dispBiasConstistInd; // not satellite specific
  unsigned int                _MWConsistInd;        // not satellite specific
  double                      _yaw;
  double                      _yawRate;
  std::vector<t_frqPhaseBias> _bias;
};

class t_vTecLayer {
 public:
  t_vTecLayer() {
    _height = 0.0;
  }
  double _height;
  Matrix _C;
  Matrix _S;
};

class t_vTec {
 public:
  static void write(std::ostream* out, const t_vTec& vTec);
  static void read(const std::string& epoLine, std::istream& in, t_vTec& vTec);
  std::string              _staID;
  bncTime                  _time;
  unsigned int            _updateInt;
  std::vector<t_vTecLayer> _layers;
};

class t_corrSSR {
 public:
  enum e_type {clkCorr, orbCorr, codeBias, phaseBias, vTec, URA, unknown};
  static e_type readEpoLine(const std::string& line, bncTime& epoTime,
                            unsigned int& updateInt, int& numEntries, std::string& staID);
};

#endif
