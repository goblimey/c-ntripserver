#ifndef PPPUTILS_H
#define PPPUTILS_H

#include <string>
#include <newmat.h>
#include "satObs.h"

namespace BNC_PPP {

class t_pppUtils {
 public:
  t_pppUtils();
  ~t_pppUtils();
  void putCodeBias(t_satCodeBias* satCodeBias);
  const t_satCodeBias* satCodeBias(const t_prn& prn) const {
      return _satCodeBiases[prn.toInt()];
  }

 private:
  t_satCodeBias*   _satCodeBiases[t_prn::MAXPRN+1];
};

}

#endif
