#ifndef OPTIONS_H
#define OPTIONS_H

#include <string>
#include <vector>
#include <newmat.h>
#include "pppInclude.h"

namespace BNC_PPP {

class t_pppOptions {
 public:
  enum e_type {IF, UncombPPP, PPPRTK, DCMcodeBias, DCMphaseBias};
  t_pppOptions();
  ~t_pppOptions();

  std::vector<char>              systems() const;
  const std::vector<t_lc::type>& LCs(char system) const;
  std::vector<t_lc::type>        ambLCs(char system) const;
  std::vector<t_lc::type>        codeLCs(char system) const;
  std::vector<t_lc::type>        ionoLCs(char system) const;
  bool useSystem(char system) const {return LCs(system).size() > 0;}
  bool useOrbClkCorr() const;
  bool estTrp() const {return _aprSigTrp > 0.0 || _noiseTrp > 0.0;}
  bool xyzAprRoverSet() const {
    return (_xyzAprRover[0] != 0.0 || _xyzAprRover[1] != 0.0 || _xyzAprRover[2] != 0.0);
  }

  e_type                  _obsModelType;
  QStringList             _obsmodelTypeStr = QStringList()
      << "IF PPP"
      << "Uncombined PPP"
      << "PPP-RTK"
      << "DCM with Code Biases"
      << "DCM with Phase Biases";
  bool                    _realTime;
  std::string             _crdFile;
  std::string             _corrMount;
  std::string             _rinexObs;
  std::string             _rinexNav;
  std::string             _corrFile;
  double                  _corrWaitTime;
  std::string             _roverName;
  ColumnVector            _xyzAprRover;
  ColumnVector            _neuEccRover;
  std::string             _recNameRover;
  std::string             _antNameRover;
  std::string             _antexFileName;
  std::string             _blqFileName;
  double                  _sigmaC1;
  double                  _sigmaL1;
  double                  _maxResC1;
  double                  _maxResL1;
  double                  _sigmaGIMdiff;
  double                  _sigmaTz0;
  bool                    _eleWgtCode;
  bool                    _eleWgtPhase;
  double                  _minEle;
  int                     _minObs;
  ColumnVector            _aprSigCrd;
  ColumnVector            _noiseCrd;
  double                  _noiseClk;
  double                  _aprSigTrp;
  double                  _noiseTrp;
  double                  _aprSigIon;
  double                  _noiseIon;
  double                  _aprSigCodeBias;
  double                  _noiseCodeBias;
  double                  _aprSigPhaseBias;
  double                  _noisePhaseBias;
  int                     _nmeaPort;
  double                  _aprSigAmb;
  double                  _seedingTime;
  std::vector<t_lc::type> _LCsGPS;
  std::vector<t_lc::type> _LCsGLONASS;
  std::vector<t_lc::type> _LCsGalileo;
  std::vector<t_lc::type> _LCsBDS;
  bool                    _pseudoObsIono;
  bool                    _pseudoObsTropo;
  bool                    _refSatRequired;
};

}

#endif
