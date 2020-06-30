#include <iomanip>
#include <iostream>
#include <math.h>
#include <stdio.h>

#include "rtcm_utils.h"
#include "ephemeris.h"

using namespace std;

void resolveEpoch (double secsHour,
                   int  refWeek,   double  refSecs,
                   int& epochWeek, double& epochSecs) {

  const double secsPerWeek = 604800.0;

  epochWeek = refWeek;
  epochSecs = secsHour + 3600.0*(floor((refSecs-secsHour)/3600.0+0.5));

  if (epochSecs<0          ) { epochWeek--; epochSecs+=secsPerWeek; };
  if (epochSecs>secsPerWeek) { epochWeek++; epochSecs-=secsPerWeek; };
};


int cmpRho(const t_eph* eph,
           double stax, double stay, double staz,
           int GPSWeek, double GPSWeeks,
           double& rho, int& GPSWeek_tot, double& GPSWeeks_tot,
           double& xSat, double& ySat, double& zSat, double& clkSat) {

  const double omega_earth = 7292115.1467e-11;
  const double secsPerWeek = 604800.0;

  // Initial values
  // --------------
  rho = 0.0;
  ColumnVector xc(6);
  ColumnVector vv(3);
  eph->getCrd(bncTime(GPSWeek, GPSWeeks), xc, vv, false);
  xSat   = xc(1);
  ySat   = xc(2);
  zSat   = xc(3);
  clkSat = xc(4);

  ////cout << "----- cmpRho -----\n";
  ////eph->print(cout);
  ////cout << "  pos " << setw(4)  << GPSWeek
  ////     << " "      << setw(14) << setprecision(6) << GPSWeeks
  ////     << " "      << setw(13) << setprecision(3) << xSat
  ////     << " "      << setw(13) << setprecision(3) << ySat
  ////     << " "      << setw(13) << setprecision(3) << zSat
  ////     << endl;

  // Loop until the correct Time Of Transmission is found
  // ----------------------------------------------------
  double rhoLast = 0;
  do {
    rhoLast = rho;

    // Correction station position due to Earth Rotation
    // -------------------------------------------------
    double dPhi = omega_earth * rho / c_light;
    double xRec = stax * cos(dPhi) - stay * sin(dPhi);
    double yRec = stay * cos(dPhi) + stax * sin(dPhi);
    double zRec = staz;

    double dx   = xRec - xSat;
    double dy   = yRec - ySat;
    double dz   = zRec - zSat;

    rho = sqrt(dx*dx + dy*dy + dz*dz);

    GPSWeek_tot  = GPSWeek;
    GPSWeeks_tot = GPSWeeks - rho/c_light;
    while ( GPSWeeks_tot < 0 ) {
      GPSWeeks_tot += secsPerWeek;
      GPSWeek_tot  -= 1;
    }
    while ( GPSWeeks_tot > secsPerWeek ) {
      GPSWeeks_tot -= secsPerWeek;
      GPSWeek_tot  += 1;
    }

    eph->getCrd(bncTime(GPSWeek_tot, GPSWeeks_tot), xc, vv, false);
    xSat   = xc(1);
    ySat   = xc(2);
    zSat   = xc(3);
    clkSat = xc(4);

    dx = xRec - xSat;
    dy = yRec - ySat;
    dz = zRec - zSat;

    rho = sqrt(dx*dx + dy*dy + dz*dz);

    ////cout << "  scrd "   << setw(4)  << GPSWeek_tot
    ////	 << " "         << setw(15) << setprecision(8) << GPSWeeks_tot
    ////	 << " "         << setw(13) << setprecision(3) << xSat
    ////	 << " "         << setw(13) << setprecision(3) << ySat
    ////	 << " "         << setw(13) << setprecision(3) << zSat
    ////	 << " rcv0 "    << setw(12) << setprecision(3) << stax
    ////	 << " "         << setw(12) << setprecision(3) << stay
    ////	 << " "         << setw(12) << setprecision(3) << staz
    ////	 << " rcv  "    << setw(12) << setprecision(3) << xRec
    ////	 << " "         << setw(12) << setprecision(3) << yRec
    ////	 << " "         << setw(12) << setprecision(3) << zRec
    ////	 << " dPhi "    << scientific << setw(13) << setprecision(10) << dPhi  << fixed
    ////	 << " rho "     << setw(13) << setprecision(3) << rho
    ////	 << endl;


    ////cout.setf(ios::fixed);
    ////
    ////cout << "niter " << setw(3) << ++niter
    ////         << " " << setw(14) << setprecision(3) << rhoLast
    ////         << " " << setw(14) << setprecision(3) << rho
    ////         << endl;

  } while ( fabs(rho - rhoLast) > 1e-4);

  clkSat *= c_light;  // satellite clock correction in meters

  return 0;
}
