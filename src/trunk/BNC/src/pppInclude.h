#ifndef PPP_H
#define PPP_H

#include <string>
#include <vector>
#include <newmat.h>

#include "bncconst.h"
#include "bnctime.h"
#include "ephemeris.h"
#include "t_prn.h"
#include "satObs.h"

namespace BNC_PPP {

class t_except {
 public:
  t_except(const char* msg) {
    _msg = msg;
  }
  ~t_except() {}
  std::string what() {return _msg;}
 private:
  std::string _msg;
};

class t_output {
 public:
  bncTime      _epoTime;
  double       _xyzRover[3];
  double       _covMatrix[6];
  double       _neu[3];
  double       _trp0;
  double       _trp;
  double       _trpStdev;
  int          _numSat;
  double       _hDop;
  std::string  _log;
  bool         _error;
};

class t_lc {
 public:
  enum type {dummy = 0, l1, l2, c1, c2, lIF, cIF, MW, CL, GIM, Tz0, maxLc};

  static bool includesPhase(type tt) {
    switch (tt) {
    case l1:
    case l2:
    case lIF:
    case MW:
    case CL:
      return true;
    case c1:
    case c2:
    case cIF:
      return false;
    case dummy:
    case maxLc:
    case GIM:
    case Tz0:
      return false;
    }
    return false;
  }

  static bool includesCode(type tt) {
    switch (tt) {
    case c1:
    case c2:
    case cIF:
    case MW:
    case CL:
      return true;
    case l1:
    case l2:
    case lIF:
      return false;
    case dummy:
    case maxLc:
    case GIM:
    case Tz0:
      return false;
    }
    return false;
  }

  static t_frequency::type toFreq(char sys, type tt) {
    switch (tt) {
    case l1: case c1:
      if      (sys == 'G') return t_frequency::G1;
      else if (sys == 'R') return t_frequency::R1;
      else if (sys == 'E') return t_frequency::E1;
      else if (sys == 'C') return t_frequency::C2;
      else                 return t_frequency::dummy;
    case l2: case c2:
      if      (sys == 'G') return t_frequency::G2;
      else if (sys == 'R') return t_frequency::R2;
      else if (sys == 'E') return t_frequency::E5;
      else if (sys == 'C') return t_frequency::C7;
      else                 return t_frequency::dummy;
    case lIF: case cIF: case MW: case CL:
      return t_frequency::dummy;
    case dummy:
    case maxLc:
    case GIM:
    case Tz0:
      return t_frequency::dummy;
    }
    return t_frequency::dummy;
  }

  static std::string toString(type tt) {
    switch (tt) {
    case l1:  return "l1";
    case l2:  return "l2";
    case lIF: return "lIF";
    case MW:  return "MW";
    case CL:  return "CL";
    case c1:  return "c1";
    case c2:  return "c2";
    case cIF: return "cIF";
    case GIM: return "GIM";
    case Tz0: return "Tz0";
    case dummy:
    case maxLc:
      return "";
    }
    return "";
  }
};

class interface_pppClient {
 public:
  virtual      ~interface_pppClient() {};
  virtual void processEpoch(const std::vector<t_satObs*>& satObs, t_output* output) = 0;
  virtual void putEphemeris(const t_eph* eph) = 0;
  virtual void putOrbCorrections(const std::vector<t_orbCorr*>& corr) = 0;
  virtual void putClkCorrections(const std::vector<t_clkCorr*>& corr) = 0;
  virtual void putCodeBiases(const std::vector<t_satCodeBias*>& satCodeBias) = 0;
};

} // namespace BNC_PPP

#endif
