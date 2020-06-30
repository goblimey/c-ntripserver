// Part of BNC, a utility for retrieving decoding and
// converting GNSS data streams from NTRIP broadcasters.
//
// Copyright (C) 2007
// German Federal Agency for Cartography and Geodesy (BKG)
// http://www.bkg.bund.de
// Czech Technical University Prague, Department of Geodesy
// http://www.fsv.cvut.cz
//
// Email: euref-ip@bkg.bund.de
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation, version 2.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

/* -------------------------------------------------------------------------
 * BKG NTRIP Client
 * -------------------------------------------------------------------------
 *
 * Class:      t_astro, t_tides, t_tropo
 *
 * Purpose:    Observation model
 *
 * Author:     L. Mervart
 *
 * Created:    29-Jul-2014
 *
 * Changes:
 *
 * -----------------------------------------------------------------------*/

#include <cmath>

#include "pppModel.h"

using namespace BNC_PPP;
using namespace std;



Matrix t_astro::rotX(double Angle) {
  const double C = cos(Angle);
  const double S = sin(Angle);
  Matrix UU(3, 3);
  UU[0][0] = 1.0;
  UU[0][1] = 0.0;
  UU[0][2] = 0.0;
  UU[1][0] = 0.0;
  UU[1][1] = +C;
  UU[1][2] = +S;
  UU[2][0] = 0.0;
  UU[2][1] = -S;
  UU[2][2] = +C;
  return UU;
}

Matrix t_astro::rotY(double Angle) {
  const double C = cos(Angle);
  const double S = sin(Angle);
  Matrix UU(3, 3);
  UU[0][0] = +C;
  UU[0][1] = 0.0;
  UU[0][2] = -S;
  UU[1][0] = 0.0;
  UU[1][1] = 1.0;
  UU[1][2] = 0.0;
  UU[2][0] = +S;
  UU[2][1] = 0.0;
  UU[2][2] = +C;
  return UU;
}

Matrix t_astro::rotZ(double Angle) {
  const double C = cos(Angle);
  const double S = sin(Angle);
  Matrix UU(3, 3);
  UU[0][0] = +C;
  UU[0][1] = +S;
  UU[0][2] = 0.0;
  UU[1][0] = -S;
  UU[1][1] = +C;
  UU[1][2] = 0.0;
  UU[2][0] = 0.0;
  UU[2][1] = 0.0;
  UU[2][2] = 1.0;
  return UU;
}

// Greenwich Mean Sidereal Time
///////////////////////////////////////////////////////////////////////////
double t_astro::GMST(double Mjd_UT1) {

  const double Secs = 86400.0;

  double Mjd_0 = floor(Mjd_UT1);
  double UT1 = Secs * (Mjd_UT1 - Mjd_0);
  double T_0 = (Mjd_0 - MJD_J2000) / 36525.0;
  double T = (Mjd_UT1 - MJD_J2000) / 36525.0;

  double gmst = 24110.54841 + 8640184.812866 * T_0 + 1.002737909350795 * UT1
      + (0.093104 - 6.2e-6 * T) * T * T;

  return 2.0 * M_PI * Frac(gmst / Secs);
}

// Nutation Matrix
///////////////////////////////////////////////////////////////////////////
Matrix t_astro::NutMatrix(double Mjd_TT) {

  const double T = (Mjd_TT - MJD_J2000) / 36525.0;

  double ls = 2.0 * M_PI * Frac(0.993133 + 99.997306 * T);
  double D = 2.0 * M_PI * Frac(0.827362 + 1236.853087 * T);
  double F = 2.0 * M_PI * Frac(0.259089 + 1342.227826 * T);
  double N = 2.0 * M_PI * Frac(0.347346 - 5.372447 * T);

  double dpsi = (-17.200 * sin(N) - 1.319 * sin(2 * (F - D + N))
      - 0.227 * sin(2 * (F + N))
      + 0.206 * sin(2 * N) + 0.143 * sin(ls)) / RHO_SEC;
  double deps = (+9.203 * cos(N) + 0.574 * cos(2 * (F - D + N))
      + 0.098 * cos(2 * (F + N))
      - 0.090 * cos(2 * N)) / RHO_SEC;

  double eps = 0.4090928 - 2.2696E-4 * T;

  return rotX(-eps - deps) * rotZ(-dpsi) * rotX(+eps);
}

// Precession Matrix
///////////////////////////////////////////////////////////////////////////
Matrix t_astro::PrecMatrix(double Mjd_1, double Mjd_2) {

  const double T = (Mjd_1 - MJD_J2000) / 36525.0;
  const double dT = (Mjd_2 - Mjd_1) / 36525.0;

  double zeta = ((2306.2181 + (1.39656 - 0.000139 * T) * T) +
      ((0.30188 - 0.000344 * T) + 0.017998 * dT) * dT) * dT / RHO_SEC;
  double z = zeta
      + ((0.79280 + 0.000411 * T) + 0.000205 * dT) * dT * dT / RHO_SEC;
  double theta = ((2004.3109 - (0.85330 + 0.000217 * T) * T) -
      ((0.42665 + 0.000217 * T) + 0.041833 * dT) * dT) * dT / RHO_SEC;

  return rotZ(-z) * rotY(theta) * rotZ(-zeta);
}

// Sun's position
///////////////////////////////////////////////////////////////////////////
ColumnVector t_astro::Sun(double Mjd_TT) {

  const double eps = 23.43929111 / RHO_DEG;
  const double T = (Mjd_TT - MJD_J2000) / 36525.0;

  double M = 2.0 * M_PI * Frac(0.9931267 + 99.9973583 * T);
  double L = 2.0 * M_PI * Frac(0.7859444 + M / 2.0 / M_PI +
      (6892.0 * sin(M) + 72.0 * sin(2.0 * M)) / 1296.0e3);
  double r = 149.619e9 - 2.499e9 * cos(M) - 0.021e9 * cos(2 * M);

  ColumnVector r_Sun(3);
  r_Sun << r * cos(L) << r * sin(L) << 0.0;
  r_Sun = rotX(-eps) * r_Sun;

  return rotZ(GMST(Mjd_TT))
      * NutMatrix(Mjd_TT)
      * PrecMatrix(MJD_J2000, Mjd_TT)
      * r_Sun;
}

// Moon's position
///////////////////////////////////////////////////////////////////////////
ColumnVector t_astro::Moon(double Mjd_TT) {

  const double eps = 23.43929111 / RHO_DEG;
  const double T = (Mjd_TT - MJD_J2000) / 36525.0;

  double L_0 = Frac(0.606433 + 1336.851344 * T);
  double l = 2.0 * M_PI * Frac(0.374897 + 1325.552410 * T);
  double lp = 2.0 * M_PI * Frac(0.993133 + 99.997361 * T);
  double D = 2.0 * M_PI * Frac(0.827361 + 1236.853086 * T);
  double F = 2.0 * M_PI * Frac(0.259086 + 1342.227825 * T);

  double dL = +22640 * sin(l) - 4586 * sin(l - 2 * D) + 2370 * sin(2 * D)
      + 769 * sin(2 * l)
      - 668 * sin(lp) - 412 * sin(2 * F) - 212 * sin(2 * l - 2 * D)
      - 206 * sin(l + lp - 2 * D)
      + 192 * sin(l + 2 * D) - 165 * sin(lp - 2 * D) - 125 * sin(D)
      - 110 * sin(l + lp)
      + 148 * sin(l - lp) - 55 * sin(2 * F - 2 * D);

  double L = 2.0 * M_PI * Frac(L_0 + dL / 1296.0e3);

  double S = F + (dL + 412 * sin(2 * F) + 541 * sin(lp)) / RHO_SEC;
  double h = F - 2 * D;
  double N = -526 * sin(h) + 44 * sin(l + h) - 31 * sin(-l + h)
      - 23 * sin(lp + h)
      + 11 * sin(-lp + h) - 25 * sin(-2 * l + F) + 21 * sin(-l + F);

  double B = (18520.0 * sin(S) + N) / RHO_SEC;

  double cosB = cos(B);

  double R = 385000e3 - 20905e3 * cos(l) - 3699e3 * cos(2 * D - l)
      - 2956e3 * cos(2 * D)
      - 570e3 * cos(2 * l) + 246e3 * cos(2 * l - 2 * D)
      - 205e3 * cos(lp - 2 * D)
      - 171e3 * cos(l + 2 * D) - 152e3 * cos(l + lp - 2 * D);

  ColumnVector r_Moon(3);
  r_Moon << R * cos(L) * cosB << R * sin(L) * cosB << R * sin(B);
  r_Moon = rotX(-eps) * r_Moon;

  return rotZ(GMST(Mjd_TT))
      * NutMatrix(Mjd_TT)
      * PrecMatrix(MJD_J2000, Mjd_TT)
      * r_Moon;
}

// Tidal Correction
////////////////////////////////////////////////////////////////////////////
ColumnVector t_tides::earth(const bncTime& time, const ColumnVector& xyz) {

  if (time.undef()) {
    ColumnVector dX(3);
    dX = 0.0;
    return dX;
  }

  double Mjd = time.mjd() + time.daysec() / 86400.0;

  if (Mjd != _lastMjd) {
    _lastMjd = Mjd;
    _xSun = t_astro::Sun(Mjd);
    _rSun = sqrt(DotProduct(_xSun, _xSun));
    _xSun /= _rSun;
    _xMoon = t_astro::Moon(Mjd);
    _rMoon = sqrt(DotProduct(_xMoon, _xMoon));
    _xMoon /= _rMoon;
  }

  double rRec = sqrt(DotProduct(xyz, xyz));
  ColumnVector xyzUnit = xyz / rRec;

  // Love's Numbers
  // --------------
  const double H2 = 0.6078;
  const double L2 = 0.0847;

  // Tidal Displacement
  // ------------------
  double scSun = DotProduct(xyzUnit, _xSun);
  double scMoon = DotProduct(xyzUnit, _xMoon);

  double p2Sun = 3.0 * (H2 / 2.0 - L2) * scSun * scSun - H2 / 2.0;
  double p2Moon = 3.0 * (H2 / 2.0 - L2) * scMoon * scMoon - H2 / 2.0;

  double x2Sun = 3.0 * L2 * scSun;
  double x2Moon = 3.0 * L2 * scMoon;

  const double gmWGS = 398.6005e12;
  const double gms = 1.3271250e20;
  const double gmm = 4.9027890e12;

  double facSun = gms / gmWGS *
      (rRec * rRec * rRec * rRec) / (_rSun * _rSun * _rSun);

  double facMoon = gmm / gmWGS *
      (rRec * rRec * rRec * rRec) / (_rMoon * _rMoon * _rMoon);

  ColumnVector dX = facSun * (x2Sun * _xSun + p2Sun * xyzUnit)
                  + facMoon * (x2Moon * _xMoon + p2Moon * xyzUnit);

  return dX;
}

// Constructor
///////////////////////////////////////////////////////////////////////////
t_tides::t_tides() {
  _lastMjd = 0.0;
  _rSun = 0.0;
  _rMoon = 0.0;
  newBlqData = 0;
}

t_tides::~t_tides() {

  if (newBlqData) {
    delete newBlqData;
  }

  QMapIterator<QString, t_blqData*> it(blqMap);
  while (it.hasNext()) {
    it.next();
    delete it.value();
  }
}

t_irc t_tides::readBlqFile(const char* fileName) {
  QFile inFile(fileName);
  inFile.open(QIODevice::ReadOnly | QIODevice::Text);
  QTextStream in(&inFile);
  int row = 0;
  QString site = QString();

  while (!in.atEnd()) {

    QString line = in.readLine();

    // skip empty lines and comments
    if (line.indexOf("$$") != -1) {
      continue;
    }
    line = line.trimmed();
    QTextStream inLine(line.toLatin1(), QIODevice::ReadOnly);
    switch (row) {
      case 0:
        site = line;
        site = site.toUpper();
        newBlqData = new t_blqData;
        newBlqData->amplitudes.ReSize(3, 11);
        newBlqData->phases.ReSize(3, 11);
        break;
      case 1:
      case 2:
      case 3:
        for (int ii = 0; ii < 11; ii++) {
          inLine >> newBlqData->amplitudes[row - 1][ii];
        }
        break;
      case 4:
      case 5:
        for (int ii = 0; ii < 11; ii++) {
          inLine >> newBlqData->phases[row - 4][ii];
        }
        break;
      case 6:
        for (int ii = 0; ii < 11; ii++) {
          inLine >> newBlqData->phases[row - 4][ii];
        }
        if (newBlqData && !site.isEmpty()) {
          blqMap[site] = newBlqData;
          site = QString();
          newBlqData = 0;
        }
        row = -1;
        break;
    }
    row++;
  }
  inFile.close();
  return success;
}

ColumnVector t_tides::ocean(const bncTime& time,  const ColumnVector& xyz,
    const std::string& station) {
  ColumnVector dX(3); dX = 0.0;
  if (time.undef()) {
    return dX;
  }
  QString stationQ = station.c_str();
  if (blqMap.find(stationQ) == blqMap.end()) {
    return dX;
  }
  t_blqData* blqSet = blqMap[stationQ];  //printBlqSet(station, blqSet);

  // angular argument: see arg2.f from IERS Conventions software collection
  double speed[11] = {1.40519e-4, 1.45444e-4, 1.3788e-4, 1.45842e-4, 7.2921e-5,
                      6.7598e-5,  7.2523e-5,  6.4959e-5, 5.3234e-6,  2.6392e-6, 3.982e-7};

  double angfac[4][11];
  angfac[0][0] = 2.0;
  angfac[1][0] =-2.0;
  angfac[2][0] = 0.0;
  angfac[3][0] = 0.0;

  angfac[0][1] = 0.0;
  angfac[1][1] = 0.0;
  angfac[2][1] = 0.0;
  angfac[3][1] = 0.0;

  angfac[0][2] = 2.0;
  angfac[1][2] =-3.0;
  angfac[2][2] = 1.0;
  angfac[3][2] = 0.0;

  angfac[0][3] = 2.0;
  angfac[1][3] = 0.0;
  angfac[2][3] = 0.0;
  angfac[3][3] = 0.0;

  angfac[0][4] = 1.0;
  angfac[1][4] = 0.0;
  angfac[2][4] = 0.0;
  angfac[3][4] = .25;

  angfac[0][5] = 1.0;
  angfac[1][5] =-2.0;
  angfac[2][5] = 0.0;
  angfac[3][5] =-.25;

  angfac[0][6] =-1.0;
  angfac[1][6] = 0.0;
  angfac[2][6] = 0.0;
  angfac[3][6] =-.25;

  angfac[0][7] = 1.0;
  angfac[1][7] =-3.0;
  angfac[2][7] = 1.0;
  angfac[3][7] =-.25;

  angfac[0][8] = 0.0;
  angfac[1][8] = 2.0;
  angfac[2][8] = 0.0;
  angfac[3][8] = 0.0;

  angfac[0][9] = 0.0;
  angfac[1][9] = 1.0;
  angfac[2][9] =-1.0;
  angfac[3][9] = 0.0;

  angfac[0][10] = 2.0;
  angfac[1][10] = 0.0;
  angfac[2][10] = 0.0;
  angfac[3][10] = 0.0;

  double twopi = 6.283185307179586476925287e0;
  double dtr = 0.0174532925199;

  //  fractional part of the day in seconds
  unsigned int year, month, day;
  time.civil_date(year, month, day);
  int iyear = year - 2000;
  QDateTime datTim = QDateTime::fromString(QString::fromStdString(time.datestr()), Qt::ISODate);
  int doy = datTim.date().dayOfYear();
  double fday = time.daysec();
  int   icapd = doy + 365 * (iyear - 75) + ((iyear - 73) / 4);
  double capt = (icapd * 1.000000035 + 27392.500528) / 36525.0;

  // mean longitude of the sun at the beginning of the day
  double h0 = (279.69668e0 + (36000.768930485e0 + 3.03e-4 * capt) * capt) * dtr;

  // mean longitude of moon at the beginning of the day
  double s0 = (((1.9e-6 * capt - .001133e0) * capt + 481267.88314137e0) * capt + 270.434358e0) * dtr;

  // mean longitude of lunar perigee at the beginning of the day
  double p0 =  (((-1.2e-5 * capt - .010325e0) * capt + 4069.0340329577e0) * capt + 334.329653e0) * dtr;

  // tidal angle arguments
  double angle[11];
  for (int k = 0; k < 11; ++k) {
    angle[k] = speed[k] * fday
             + angfac[0][k] * h0
             + angfac[1][k] * s0
             + angfac[2][k] * p0
             + angfac[3][k] * twopi;
    angle[k] = fmod(angle[k], twopi);
    if (angle[k] < 0.0) {
      angle[k] += twopi;
    }
  }

  // displacement by 11 constituents
  ColumnVector rwsSta(3); rwsSta = 0.0; // radial, west, south
  for (int rr = 0; rr < 3; rr++) {
    for (int cc = 0; cc < 11; cc++) {
      rwsSta[rr] += blqSet->amplitudes[rr][cc] * cos((angle[cc] - (blqSet->phases[rr][cc]/RHO_DEG)));
    }
  }

  // neu2xyz
  ColumnVector dneu(3); // neu
  dneu[0] = -rwsSta[2];
  dneu[1] = -rwsSta[1];
  dneu[2] =  rwsSta[0];
  double recEll[3]; xyz2ell(xyz.data(), recEll) ;
  neu2xyz(recEll, dneu.data(), dX.data());

  return dX;
}

// Print
////////////////////////////////////////////////////////////////////////////
void t_tides::printAllBlqSets() const {

  QMapIterator<QString, t_blqData*> it(blqMap);
  while (it.hasNext()) {
    it.next();
    t_blqData* blq = it.value();
    QString site = it.key();
    cout << site.toStdString().c_str() << "\n===============\n";
    for (int rr = 0; rr < 3; rr++) {
      for (int cc = 0; cc < 11; cc++) {
        cout << blq->amplitudes[rr][cc] << " ";
      }
      cout << endl;
    }
    for (int rr = 0; rr < 3; rr++) {
      for (int cc = 0; cc < 11; cc++) {
        cout << blq->phases[rr][cc] << " ";
      }
      cout << endl;
    }
  }
}

// Print
////////////////////////////////////////////////////////////////////////////
void t_tides::printBlqSet(const std::string& station, t_blqData* blq) {
  cout << station << endl;
  for (int rr = 0; rr < 3; rr++) {
    for (int cc = 0; cc < 11; cc++) {
      cout << blq->amplitudes[rr][cc] << " ";
    }
    cout << endl;
  }
  for (int rr = 0; rr < 3; rr++) {
    for (int cc = 0; cc < 11; cc++) {
      cout << blq->phases[rr][cc] << " ";
    }
    cout << endl;
  }
}

// Constructor
///////////////////////////////////////////////////////////////////////////
t_windUp::t_windUp() {
  for (unsigned ii = 0; ii <= t_prn::MAXPRN; ii++) {
    sumWind[ii] = 0.0;
    lastEtime[ii] = 0.0;
  }
}

// Phase Wind-Up Correction
///////////////////////////////////////////////////////////////////////////
double t_windUp::value(const bncTime& etime, const ColumnVector& rRec,
    t_prn prn, const ColumnVector& rSat, bool ssr,
    double yaw, const ColumnVector& vSat) {

  if (etime.mjddec() != lastEtime[prn.toInt()]) {

    // Unit Vector GPS Satellite --> Receiver
    // --------------------------------------
    ColumnVector rho = rRec - rSat;
    rho /= rho.NormFrobenius();

    // GPS Satellite unit Vectors sz, sy, sx
    // -------------------------------------
    ColumnVector sHlp;
    if (!ssr) {
      sHlp = t_astro::Sun(etime.mjddec());
    }
    else {
      ColumnVector Omega(3);
      Omega[0] = 0.0;
      Omega[1] = 0.0;
      Omega[2] = t_CST::omega;
      sHlp = vSat + crossproduct(Omega, rSat);
    }
    sHlp /= sHlp.NormFrobenius();

    ColumnVector sz = -rSat / rSat.NormFrobenius();
    ColumnVector sy = crossproduct(sz, sHlp);
    ColumnVector sx = crossproduct(sy, sz);

    if (ssr) {
      // Yaw angle consideration
      Matrix SXYZ(3, 3);
      SXYZ.Column(1) = sx;
      SXYZ.Column(2) = sy;
      SXYZ.Column(3) = sz;
      SXYZ = DotProduct(t_astro::rotZ(yaw), SXYZ);
      sx = SXYZ.Column(1);
      sy = SXYZ.Column(2);
      sz = SXYZ.Column(3);
    }
    // Effective Dipole of the GPS Satellite Antenna
    // ---------------------------------------------
    ColumnVector dipSat = sx - rho * DotProduct(rho, sx)
        - crossproduct(rho, sy);

    // Receiver unit Vectors rx, ry
    // ----------------------------
    ColumnVector rx(3);
    ColumnVector ry(3);
    double recEll[3];
    xyz2ell(rRec.data(), recEll);
    double neu[3];

    neu[0] = 1.0;
    neu[1] = 0.0;
    neu[2] = 0.0;
    neu2xyz(recEll, neu, rx.data());

    neu[0] = 0.0;
    neu[1] = -1.0;
    neu[2] = 0.0;
    neu2xyz(recEll, neu, ry.data());

    // Effective Dipole of the Receiver Antenna
    // ----------------------------------------
    ColumnVector dipRec = rx - rho * DotProduct(rho, rx)
        + crossproduct(rho, ry);

    // Resulting Effect
    // ----------------
    double alpha = DotProduct(dipSat, dipRec)
        / (dipSat.NormFrobenius() * dipRec.NormFrobenius());

    if (alpha > 1.0)
      alpha = 1.0;
    if (alpha < -1.0)
      alpha = -1.0;

    double dphi = acos(alpha) / 2.0 / M_PI;  // in cycles

    if (DotProduct(rho, crossproduct(dipSat, dipRec)) < 0.0) {
      dphi = -dphi;
    }

    if (lastEtime[prn.toInt()] == 0.0) {
      sumWind[prn.toInt()] = dphi;
    }
    else {
      sumWind[prn.toInt()] = nint(sumWind[prn.toInt()] - dphi) + dphi;
    }

    lastEtime[prn.toInt()] = etime.mjddec();
  }

  return sumWind[prn.toInt()];
}

// Tropospheric Model (Saastamoinen)
////////////////////////////////////////////////////////////////////////////
double t_tropo::delay_saast(const ColumnVector& xyz, double Ele) {

  Tracer tracer("bncModel::delay_saast");

  if (xyz[0] == 0.0 && xyz[1] == 0.0 && xyz[2] == 0.0) {
    return 0.0;
  }

  double ell[3];
  xyz2ell(xyz.data(), ell);
  double height = ell[2];

  double pp = 1013.25 * pow(1.0 - 2.26e-5 * height, 5.225);
  double TT = 18.0 - height * 0.0065 + 273.15;
  double hh = 50.0 * exp(-6.396e-4 * height);
  double ee = hh / 100.0
      * exp(-37.2465 + 0.213166 * TT - 0.000256908 * TT * TT);

  double h_km = height / 1000.0;

  if (h_km < 0.0)
    h_km = 0.0;
  if (h_km > 5.0)
    h_km = 5.0;
  int ii = int(h_km + 1);
  if (ii > 5)
    ii = 5;
  double href = ii - 1;

  double bCor[6];
  bCor[0] = 1.156;
  bCor[1] = 1.006;
  bCor[2] = 0.874;
  bCor[3] = 0.757;
  bCor[4] = 0.654;
  bCor[5] = 0.563;

  double BB = bCor[ii - 1] + (bCor[ii] - bCor[ii - 1]) * (h_km - href);

  double zen = M_PI / 2.0 - Ele;

  return (0.002277 / cos(zen))
      * (pp + ((1255.0 / TT) + 0.05) * ee - BB * (tan(zen) * tan(zen)));
}

// Constructor
///////////////////////////////////////////////////////////////////////////
t_iono::t_iono() {
  _psiPP = _phiPP = _lambdaPP = _lonS = 0.0;
}

t_iono::~t_iono() {
}

double t_iono::stec(const t_vTec* vTec, double signalPropagationTime,
    const ColumnVector& rSat, const bncTime& epochTime,
    const ColumnVector& xyzSta) {

  // Latitude, longitude, height are defined with respect to a spherical earth model
  // -------------------------------------------------------------------------------
  ColumnVector geocSta(3);
  if (xyz2geoc(xyzSta.data(), geocSta.data()) != success) {
    return 0.0;
  }

  // satellite position rotated to the epoch of signal reception
  // -----------------------------------------------------------
  ColumnVector xyzSat(3);
  double omegaZ = t_CST::omega * signalPropagationTime;
  xyzSat[0] = rSat[0] * cos(omegaZ) + rSat[1] * sin(omegaZ);
  xyzSat[1] = rSat[1] * cos(omegaZ) - rSat[0] * sin(omegaZ);
  xyzSat[2] = rSat[2];

  // elevation and azimuth with respect to a spherical earth model
  // -------------------------------------------------------------
  ColumnVector rhoV = xyzSat - xyzSta;
  double rho = rhoV.NormFrobenius();
  ColumnVector neu(3);
  xyz2neu(geocSta.data(), rhoV.data(), neu.data());
  double sphEle = acos(sqrt(neu[0] * neu[0] + neu[1] * neu[1]) / rho);
  if (neu[2] < 0) {
    sphEle *= -1.0;
  }
  double sphAzi = atan2(neu[1], neu[0]);

  double epoch = fmod(epochTime.gpssec(), 86400.0);

  double stec = 0.0;
  for (unsigned ii = 0; ii < vTec->_layers.size(); ii++) {
    piercePoint(vTec->_layers[ii]._height, epoch, geocSta.data(), sphEle,
        sphAzi);
    double vtec = vtecSingleLayerContribution(vTec->_layers[ii]);
    stec += vtec * sin(sphEle + _psiPP);
  }
  return stec;
}

double t_iono::vtecSingleLayerContribution(const t_vTecLayer& vTecLayer) {

  double vtec = 0.0;
  int N = vTecLayer._C.Nrows() - 1;
  int M = vTecLayer._C.Ncols() - 1;
  double fac;

  for (int n = 0; n <= N; n++) {
    for (int m = 0; m <= min(n, M); m++) {
      double pnm = associatedLegendreFunction(n, m, sin(_phiPP));
      double a = factorial(n - m);
      double b = factorial(n + m);
      if (m == 0) {
        fac = sqrt(2.0 * n + 1);
      }
      else {
        fac = sqrt(2.0 * (2.0 * n + 1) * a / b);
      }
      pnm *= fac;
      double Cnm_mlambda = vTecLayer._C[n][m] * cos(m * _lonS);
      double Snm_mlambda = vTecLayer._S[n][m] * sin(m * _lonS);
      vtec += (Snm_mlambda + Cnm_mlambda) * pnm;
    }
  }

  if (vtec < 0.0) {
    vtec = 0.0;
  }

  return vtec;
}

void t_iono::piercePoint(double layerHeight, double epoch,
    const double* geocSta,
    double sphEle, double sphAzi) {

  double q = (t_CST::rgeoc + geocSta[2]) / (t_CST::rgeoc + layerHeight);

  _psiPP = M_PI / 2 - sphEle - asin(q * cos(sphEle));

  _phiPP = asin(
      sin(geocSta[0]) * cos(_psiPP)
          + cos(geocSta[0]) * sin(_psiPP) * cos(sphAzi));

  if (((geocSta[0] * 180.0 / M_PI > 0)
      && (tan(_psiPP) * cos(sphAzi) > tan(M_PI / 2 - geocSta[0])))
      ||
      ((geocSta[0] * 180.0 / M_PI < 0)
          && (-(tan(_psiPP) * cos(sphAzi)) > tan(M_PI / 2 + geocSta[0])))) {
    _lambdaPP = geocSta[1] + M_PI
        - asin((sin(_psiPP) * sin(sphAzi) / cos(_phiPP)));
  }
  else {
    _lambdaPP = geocSta[1] + asin((sin(_psiPP) * sin(sphAzi) / cos(_phiPP)));
  }

  _lonS = fmod((_lambdaPP + (epoch - 50400) * M_PI / 43200), 2 * M_PI);

  return;
}

