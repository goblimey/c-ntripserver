#ifndef UTILS_H
#define UTILS_H

#include "const.h"

namespace GnssCenter {

class t_utils {
 public:
  static t_CST::t_irc xyz2ell(const double* XYZ, double* Ell);

  static void xyz2neu(const double* Ell, const double* xyz, double* neu);

  static void neu2xyz(const double* Ell, const double* neu, double* xyz);
};

} // namespace GnssCenter

#endif
