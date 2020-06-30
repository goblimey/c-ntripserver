
#include <math.h>

#include "utils.h"

using namespace std;
using namespace GnssCenter;

// Rectangular Coordinates -> Ellipsoidal Coordinates
////////////////////////////////////////////////////////////////////////////
t_CST::t_irc t_utils::xyz2ell(const double* XYZ, double* Ell) {

  const double bell = t_CST::aell*(1.0-1.0/t_CST::fInv) ;
  const double e2   = (t_CST::aell*t_CST::aell-bell*bell)/(t_CST::aell*t_CST::aell) ;
  const double e2c  = (t_CST::aell*t_CST::aell-bell*bell)/(bell*bell) ;
  
  double nn, ss, zps, hOld, phiOld, theta, sin3, cos3;

  ss    = sqrt(XYZ[0]*XYZ[0]+XYZ[1]*XYZ[1]) ;
  zps   = XYZ[2]/ss ;
  theta = atan( (XYZ[2]*t_CST::aell) / (ss*bell) );
  sin3  = sin(theta) * sin(theta) * sin(theta);
  cos3  = cos(theta) * cos(theta) * cos(theta);

  // Closed formula
  Ell[0] = atan( (XYZ[2] + e2c * bell * sin3) / (ss - e2 * t_CST::aell * cos3) );  
  Ell[1] = atan2(XYZ[1],XYZ[0]) ;
  nn = t_CST::aell/sqrt(1.0-e2*sin(Ell[0])*sin(Ell[0])) ;
  Ell[2] = ss / cos(Ell[0]) - nn;

  const int MAXITER = 100;
  for (int ii = 1; ii <= MAXITER; ii++) {
    nn     = t_CST::aell/sqrt(1.0-e2*sin(Ell[0])*sin(Ell[0])) ;
    hOld   = Ell[2] ;
    phiOld = Ell[0] ;
    Ell[2] = ss/cos(Ell[0])-nn ;
    Ell[0] = atan(zps/(1.0-e2*nn/(nn+Ell[2]))) ;
    if ( fabs(phiOld-Ell[0]) <= 1.0e-11 && fabs(hOld-Ell[2]) <= 1.0e-5 ) {
      return t_CST::success;
    }
  }

  return t_CST::failure;
}

// Rectangular Coordinates -> North, East, Up Components
////////////////////////////////////////////////////////////////////////////
void t_utils::xyz2neu(const double* Ell, const double* xyz, double* neu) {

  double sinPhi = sin(Ell[0]);
  double cosPhi = cos(Ell[0]);
  double sinLam = sin(Ell[1]);
  double cosLam = cos(Ell[1]);

  neu[0] = - sinPhi*cosLam * xyz[0]
           - sinPhi*sinLam * xyz[1]
           + cosPhi        * xyz[2];

  neu[1] = - sinLam * xyz[0]
           + cosLam * xyz[1];

  neu[2] = + cosPhi*cosLam * xyz[0]
           + cosPhi*sinLam * xyz[1]
           + sinPhi        * xyz[2];
}

// North, East, Up Components -> Rectangular Coordinates
////////////////////////////////////////////////////////////////////////////
void t_utils::neu2xyz(const double* Ell, const double* neu, double* xyz) {

  double sinPhi = sin(Ell[0]);
  double cosPhi = cos(Ell[0]);
  double sinLam = sin(Ell[1]);
  double cosLam = cos(Ell[1]);

  xyz[0] = - sinPhi*cosLam * neu[0]
           - sinLam        * neu[1]
           + cosPhi*cosLam * neu[2];

  xyz[1] = - sinPhi*sinLam * neu[0]
           + cosLam        * neu[1]
           + cosPhi*sinLam * neu[2];

  xyz[2] = + cosPhi        * neu[0]
           + sinPhi        * neu[2];
}

