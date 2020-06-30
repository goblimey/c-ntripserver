#include <sstream>
#include <iostream>
#include <iomanip>
#include <cstring>

#include <newmatio.h>

#include "ephemeris.h"
#include "bncutils.h"
#include "bnctime.h"
#include "bnccore.h"
#include "bncutils.h"
#include "satObs.h"
#include "pppInclude.h"
#include "pppModel.h"

using namespace std;

// Constructor
////////////////////////////////////////////////////////////////////////////
t_eph::t_eph() {
  _checkState = unchecked;
  _orbCorr    = 0;
  _clkCorr    = 0;
}
// Destructor
////////////////////////////////////////////////////////////////////////////
t_eph::~t_eph() {
  if (_orbCorr)
    delete _orbCorr;
  if (_clkCorr)
    delete _clkCorr;
}

//
////////////////////////////////////////////////////////////////////////////
void t_eph::setOrbCorr(const t_orbCorr* orbCorr) {
  if (_orbCorr) {
    delete _orbCorr;
    _orbCorr = 0;
  }
  _orbCorr = new t_orbCorr(*orbCorr);
}

//
////////////////////////////////////////////////////////////////////////////
void t_eph::setClkCorr(const t_clkCorr* clkCorr) {
  if (_clkCorr) {
    delete _clkCorr;
    _clkCorr = 0;
  }
  _clkCorr = new t_clkCorr(*clkCorr);
}

//
////////////////////////////////////////////////////////////////////////////
t_irc t_eph::getCrd(const bncTime& tt, ColumnVector& xc, ColumnVector& vv, bool useCorr) const {

  if (_checkState == bad ||
      _checkState == unhealthy ||
      _checkState == outdated) {
    return failure;
  }

  xc.ReSize(6);
  vv.ReSize(3);
  if (position(tt.gpsw(), tt.gpssec(), xc.data(), vv.data()) != success) {
    return failure;
  }
  if (useCorr) {
    if (_orbCorr && _clkCorr) {
      double dtO = tt - _orbCorr->_time;
      if (_orbCorr->_updateInt) {
        dtO -= (0.5 * ssrUpdateInt[_orbCorr->_updateInt]);
      }
      ColumnVector dx(3);
      dx[0] = _orbCorr->_xr[0] + _orbCorr->_dotXr[0] * dtO;
      dx[1] = _orbCorr->_xr[1] + _orbCorr->_dotXr[1] * dtO;
      dx[2] = _orbCorr->_xr[2] + _orbCorr->_dotXr[2] * dtO;

      RSW_to_XYZ(xc.Rows(1,3), vv.Rows(1,3), dx, dx);

      xc[0] -= dx[0];
      xc[1] -= dx[1];
      xc[2] -= dx[2];

      ColumnVector dv(3);
      RSW_to_XYZ(xc.Rows(1,3), vv.Rows(1,3), _orbCorr->_dotXr, dv);

      vv[0] -= dv[0];
      vv[1] -= dv[1];
      vv[2] -= dv[2];

      double dtC = tt - _clkCorr->_time;
      if (_clkCorr->_updateInt) {
        dtC -= (0.5 * ssrUpdateInt[_clkCorr->_updateInt]);
      }
      xc[3] += _clkCorr->_dClk + _clkCorr->_dotDClk * dtC + _clkCorr->_dotDotDClk * dtC * dtC;
    }
    else {
      return failure;
    }
  }
  return success;
}

//
//////////////////////////////////////////////////////////////////////////////
QString t_eph::rinexDateStr(const bncTime& tt, const t_prn& prn, double version) {
  QString prnStr(prn.toString().c_str());
  return rinexDateStr(tt, prnStr, version);
}

//
//////////////////////////////////////////////////////////////////////////////
QString t_eph::rinexDateStr(const bncTime& tt, const QString& prnStr, double version) {

  QString datStr;

  unsigned year, month, day, hour, min;
  double   sec;
  tt.civil_date(year, month, day);
  tt.civil_time(hour, min, sec);

  QTextStream out(&datStr);

  if (version < 3.0) {
    QString prnHlp = prnStr.mid(1,2); if (prnHlp[0] == '0') prnHlp[0] = ' ';
    out << prnHlp << QString(" %1 %2 %3 %4 %5%6")
      .arg(year % 100, 2, 10, QChar('0'))
      .arg(month,      2)
      .arg(day,        2)
      .arg(hour,       2)
      .arg(min,        2)
      .arg(sec, 5, 'f',1);
  }
  else {
    out << prnStr << QString(" %1 %2 %3 %4 %5 %6")
      .arg(year,     4)
      .arg(month,    2, 10, QChar('0'))
      .arg(day,      2, 10, QChar('0'))
      .arg(hour,     2, 10, QChar('0'))
      .arg(min,      2, 10, QChar('0'))
      .arg(int(sec), 2, 10, QChar('0'));
  }

  return datStr;
}

// Constructor
//////////////////////////////////////////////////////////////////////////////
t_ephGPS::t_ephGPS(float rnxVersion, const QStringList& lines) {

  const int nLines = 8;

  if (lines.size() != nLines) {
    _checkState = bad;
    return;
  }

  // RINEX Format
  // ------------
  int fieldLen = 19;

  int pos[4];
  pos[0] = (rnxVersion <= 2.12) ?  3 :  4;
  pos[1] = pos[0] + fieldLen;
  pos[2] = pos[1] + fieldLen;
  pos[3] = pos[2] + fieldLen;

  // Read eight lines
  // ----------------
  for (int iLine = 0; iLine < nLines; iLine++) {
    QString line = lines[iLine];

    if      ( iLine == 0 ) {
      QTextStream in(line.left(pos[1]).toLatin1());
      int    year, month, day, hour, min;
      double sec;

      QString prnStr, n;
      in >> prnStr;

      if (prnStr.size() == 1 &&
          (prnStr[0] == 'G' ||
           prnStr[0] == 'J' ||
           prnStr[0] == 'I')) {
        in >> n;
        prnStr.append(n);
      }

      in >> year >> month >> day >> hour >> min >> sec;
      if      (prnStr.at(0) == 'G') {
        _prn.set('G', prnStr.mid(1).toInt());
      }
      else if (prnStr.at(0) == 'J') {
        _prn.set('J', prnStr.mid(1).toInt());
      }
      else if (prnStr.at(0) == 'I') {
        _prn.set('I', prnStr.mid(1).toInt());
      }
      else {
        _prn.set('G', prnStr.toInt());
      }

      if      (year <  80) {
        year += 2000;
      }
      else if (year < 100) {
        year += 1900;
      }

      _TOC.set(year, month, day, hour, min, sec);

      if ( readDbl(line, pos[1], fieldLen, _clock_bias     ) ||
           readDbl(line, pos[2], fieldLen, _clock_drift    ) ||
           readDbl(line, pos[3], fieldLen, _clock_driftrate) ) {
        _checkState = bad;
        return;
      }
    }

    else if      ( iLine == 1 ) {
      if ( readDbl(line, pos[0], fieldLen, _IODE   ) ||
           readDbl(line, pos[1], fieldLen, _Crs    ) ||
           readDbl(line, pos[2], fieldLen, _Delta_n) ||
           readDbl(line, pos[3], fieldLen, _M0     ) ) {
        _checkState = bad;
        return;
      }
    }

    else if ( iLine == 2 ) {
      if ( readDbl(line, pos[0], fieldLen, _Cuc   ) ||
           readDbl(line, pos[1], fieldLen, _e     ) ||
           readDbl(line, pos[2], fieldLen, _Cus   ) ||
           readDbl(line, pos[3], fieldLen, _sqrt_A) ) {
        _checkState = bad;
        return;
      }
    }

    else if ( iLine == 3 ) {
      if ( readDbl(line, pos[0], fieldLen, _TOEsec)  ||
           readDbl(line, pos[1], fieldLen, _Cic   )  ||
           readDbl(line, pos[2], fieldLen, _OMEGA0)  ||
           readDbl(line, pos[3], fieldLen, _Cis   ) ) {
        _checkState = bad;
        return;
      }
    }

    else if ( iLine == 4 ) {
      if ( readDbl(line, pos[0], fieldLen, _i0      ) ||
           readDbl(line, pos[1], fieldLen, _Crc     ) ||
           readDbl(line, pos[2], fieldLen, _omega   ) ||
           readDbl(line, pos[3], fieldLen, _OMEGADOT) ) {
        _checkState = bad;
        return;
      }
    }

    else if ( iLine == 5 && type() != t_eph::IRNSS) {
      if ( readDbl(line, pos[0], fieldLen, _IDOT   ) ||
           readDbl(line, pos[1], fieldLen, _L2Codes) ||
           readDbl(line, pos[2], fieldLen, _TOEweek  ) ||
           readDbl(line, pos[3], fieldLen, _L2PFlag) ) {
        _checkState = bad;
        return;
      }
    }
    else if ( iLine == 5 && type() == t_eph::IRNSS) {
      if ( readDbl(line, pos[0], fieldLen, _IDOT   ) ||
           readDbl(line, pos[2], fieldLen, _TOEweek) ) {
        _checkState = bad;
        return;
      }
    }

    else if ( iLine == 6 && type() != t_eph::IRNSS) {
      if ( readDbl(line, pos[0], fieldLen, _ura   ) ||
           readDbl(line, pos[1], fieldLen, _health) ||
           readDbl(line, pos[2], fieldLen, _TGD   ) ||
           readDbl(line, pos[3], fieldLen, _IODC  ) ) {
        _checkState = bad;
        return;
      }
    }
    else if ( iLine == 6 && type() == t_eph::IRNSS) {
      if ( readDbl(line, pos[0], fieldLen, _ura   ) ||
           readDbl(line, pos[1], fieldLen, _health) ||
           readDbl(line, pos[2], fieldLen, _TGD   ) ) {
        _checkState = bad;
        return;
      }
    }
    else if ( iLine == 7 ) {
      if ( readDbl(line, pos[0], fieldLen, _TOT) ) {
        _checkState = bad;
        return;
      }
      // fitInterval is not valid for IRNSS
      if (type() != t_eph::IRNSS) {
        double fitIntervalRnx;
        readDbl(line, pos[1], fieldLen, fitIntervalRnx);
        if        (type() == t_eph::GPS) {  // in RINEX specified allways as time period for GPS
            _fitInterval = fitIntervalRnx;
        } else if (type() == t_eph::QZSS) { // specified as flag for QZSS
          if (rnxVersion == 3.02) {
            _fitInterval = fitIntervalRnx; // specified as time period
          }
          else {
            _fitInterval = fitIntervalFromFlag(fitIntervalRnx, _IODC, t_eph::QZSS);
          }
        }
      }
    }
  }
}

// Compute GPS Satellite Position (virtual)
////////////////////////////////////////////////////////////////////////////
t_irc t_ephGPS::position(int GPSweek, double GPSweeks, double* xc, double* vv) const {

  static const double omegaEarth = 7292115.1467e-11;
  static const double gmGRS      = 398.6005e12;

  memset(xc, 0, 6*sizeof(double));
  memset(vv, 0, 3*sizeof(double));

  double a0 = _sqrt_A * _sqrt_A;
  if (a0 == 0) {
    return failure;
  }

  double n0 = sqrt(gmGRS/(a0*a0*a0));

  bncTime tt(GPSweek, GPSweeks);
  double tk = tt - bncTime(int(_TOEweek), _TOEsec);

  double n  = n0 + _Delta_n;
  double M  = _M0 + n*tk;
  double E  = M;
  double E_last;
  int    nLoop = 0;
  do {
    E_last = E;
    E = M + _e*sin(E);

    if (++nLoop == 100) {
      return failure;
    }
  } while ( fabs(E-E_last)*a0 > 0.001);
  double v      = 2.0*atan( sqrt( (1.0 + _e)/(1.0 - _e) )*tan( E/2 ) );
  double u0     = v + _omega;
  double sin2u0 = sin(2*u0);
  double cos2u0 = cos(2*u0);
  double r      = a0*(1 - _e*cos(E)) + _Crc*cos2u0 + _Crs*sin2u0;
  double i      = _i0 + _IDOT*tk + _Cic*cos2u0 + _Cis*sin2u0;
  double u      = u0 + _Cuc*cos2u0 + _Cus*sin2u0;
  double xp     = r*cos(u);
  double yp     = r*sin(u);
  double OM     = _OMEGA0 + (_OMEGADOT - omegaEarth)*tk -
                   omegaEarth*_TOEsec;

  double sinom = sin(OM);
  double cosom = cos(OM);
  double sini  = sin(i);
  double cosi  = cos(i);
  xc[0] = xp*cosom - yp*cosi*sinom;
  xc[1] = xp*sinom + yp*cosi*cosom;
  xc[2] = yp*sini;

  double tc = tt - _TOC;
  xc[3] = _clock_bias + _clock_drift*tc + _clock_driftrate*tc*tc;

  // Velocity
  // --------
  double tanv2 = tan(v/2);
  double dEdM  = 1 / (1 - _e*cos(E));
  double dotv  = sqrt((1.0 + _e)/(1.0 - _e)) / cos(E/2)/cos(E/2) / (1 + tanv2*tanv2)
               * dEdM * n;
  double dotu  = dotv + (-_Cuc*sin2u0 + _Cus*cos2u0)*2*dotv;
  double dotom = _OMEGADOT - omegaEarth;
  double doti  = _IDOT + (-_Cic*sin2u0 + _Cis*cos2u0)*2*dotv;
  double dotr  = a0 * _e*sin(E) * dEdM * n
                + (-_Crc*sin2u0 + _Crs*cos2u0)*2*dotv;
  double dotx  = dotr*cos(u) - r*sin(u)*dotu;
  double doty  = dotr*sin(u) + r*cos(u)*dotu;

  vv[0]  = cosom   *dotx  - cosi*sinom   *doty      // dX / dr
           - xp*sinom*dotom - yp*cosi*cosom*dotom   // dX / dOMEGA
                       + yp*sini*sinom*doti;        // dX / di

  vv[1]  = sinom   *dotx  + cosi*cosom   *doty
           + xp*cosom*dotom - yp*cosi*sinom*dotom
                          - yp*sini*cosom*doti;

  vv[2]  = sini    *doty  + yp*cosi      *doti;

  // Relativistic Correction
  // -----------------------
  // correspondent to IGS convention and GPS ICD (and SSR standard)
  xc[3] -= 2.0 * (xc[0]*vv[0] + xc[1]*vv[1] + xc[2]*vv[2]) / t_CST::c / t_CST::c;

  xc[4] = _clock_drift + _clock_driftrate*tc;
  xc[5] = _clock_driftrate;

  return success;
}

// RINEX Format String
//////////////////////////////////////////////////////////////////////////////
QString t_ephGPS::toString(double version) const {

  QString rnxStr = rinexDateStr(_TOC, _prn, version);

  QTextStream out(&rnxStr);

  out << QString("%1%2%3\n")
    .arg(_clock_bias,      19, 'e', 12)
    .arg(_clock_drift,     19, 'e', 12)
    .arg(_clock_driftrate, 19, 'e', 12);

  QString fmt = version < 3.0 ? "   %1%2%3%4\n" : "    %1%2%3%4\n";

  out << QString(fmt)
    .arg(_IODE,    19, 'e', 12)
    .arg(_Crs,     19, 'e', 12)
    .arg(_Delta_n, 19, 'e', 12)
    .arg(_M0,      19, 'e', 12);

  out << QString(fmt)
    .arg(_Cuc,    19, 'e', 12)
    .arg(_e,      19, 'e', 12)
    .arg(_Cus,    19, 'e', 12)
    .arg(_sqrt_A, 19, 'e', 12);

  out << QString(fmt)
    .arg(_TOEsec, 19, 'e', 12)
    .arg(_Cic,    19, 'e', 12)
    .arg(_OMEGA0, 19, 'e', 12)
    .arg(_Cis,    19, 'e', 12);

  out << QString(fmt)
    .arg(_i0,       19, 'e', 12)
    .arg(_Crc,      19, 'e', 12)
    .arg(_omega,    19, 'e', 12)
    .arg(_OMEGADOT, 19, 'e', 12);

  if (type() == t_eph::IRNSS) {
    out << QString(fmt)
      .arg(_IDOT,    19, 'e', 12)
      .arg(0.0,      19, 'e', 12)
      .arg(_TOEweek, 19, 'e', 12)
      .arg(0.0,      19, 'e', 12);
  }
  else {
    out << QString(fmt)
      .arg(_IDOT,    19, 'e', 12)
      .arg(_L2Codes, 19, 'e', 12)
      .arg(_TOEweek, 19, 'e', 12)
      .arg(_L2PFlag, 19, 'e', 12);
  }

  if (type() == t_eph::IRNSS) {
    out << QString(fmt)
      .arg(_ura,    19, 'e', 12)
      .arg(_health, 19, 'e', 12)
      .arg(_TGD,    19, 'e', 12)
      .arg(0.0,     19, 'e', 12);
  }
  else {
    out << QString(fmt)
      .arg(_ura,    19, 'e', 12)
      .arg(_health, 19, 'e', 12)
      .arg(_TGD,    19, 'e', 12)
      .arg(_IODC,   19, 'e', 12);
  }

  double tot = _TOT;
  if (tot == 0.9999e9 && version < 3.0) {
    tot = 0.0;
  }
  // fitInterval
  if (type() == t_eph::IRNSS) {// not valid for IRNSS
    out << QString(fmt)
      .arg(tot,          19, 'e', 12)
      .arg(0.0,          19, 'e', 12)
      .arg("",           19, QChar(' '))
      .arg("",           19, QChar(' '));
  }
  else {
    // for GPS and QZSS in version 3.02 specified in hours
    double fitIntervalRnx = _fitInterval;
    // otherwise specified as flag
    if (type() == t_eph::QZSS && version != 3.02) {
      (_fitInterval == 2.0) ? fitIntervalRnx = 0.0 : fitIntervalRnx = 1.0;
    }
    out << QString(fmt)
      .arg(tot,            19, 'e', 12)
      .arg(fitIntervalRnx, 19, 'e', 12)
      .arg("",             19, QChar(' '))
      .arg("",             19, QChar(' '));
  }
  return rnxStr;
}

// Constructor
//////////////////////////////////////////////////////////////////////////////
t_ephGlo::t_ephGlo(float rnxVersion, const QStringList& lines) {

  const int nLines = 4;

  if (lines.size() != nLines) {
    _checkState = bad;
    return;
  }

  // RINEX Format
  // ------------
  int fieldLen = 19;

  int pos[4];
  pos[0] = (rnxVersion <= 2.12) ?  3 :  4;
  pos[1] = pos[0] + fieldLen;
  pos[2] = pos[1] + fieldLen;
  pos[3] = pos[2] + fieldLen;

  // Read four lines
  // ---------------
  for (int iLine = 0; iLine < nLines; iLine++) {
    QString line = lines[iLine];

    if      ( iLine == 0 ) {
      QTextStream in(line.left(pos[1]).toLatin1());

      int    year, month, day, hour, min;
      double sec;

      QString prnStr, n;
      in >> prnStr;
      if (prnStr.size() == 1 && prnStr[0] == 'R') {
        in >> n;
        prnStr.append(n);
      }
      in >> year >> month >> day >> hour >> min >> sec;
      if (prnStr.at(0) == 'R') {
        _prn.set('R', prnStr.mid(1).toInt());
      }
      else {
        _prn.set('R', prnStr.toInt());
      }

      if      (year <  80) {
        year += 2000;
      }
      else if (year < 100) {
        year += 1900;
      }

      _gps_utc = gnumleap(year, month, day);

      _TOC.set(year, month, day, hour, min, sec);
      _TOC  = _TOC + _gps_utc;
      int nd = int((_TOC.gpssec())) / (24.0*60.0*60.0);
      if ( readDbl(line, pos[1], fieldLen, _tau  ) ||
           readDbl(line, pos[2], fieldLen, _gamma) ||
           readDbl(line, pos[3], fieldLen, _tki  ) ) {
        _checkState = bad;
        return;
      }
      _tki -= nd * 86400.0;
      _tau  = -_tau;
    }

    else if      ( iLine == 1 ) {
      if ( readDbl(line, pos[0], fieldLen, _x_pos         ) ||
           readDbl(line, pos[1], fieldLen, _x_velocity    ) ||
           readDbl(line, pos[2], fieldLen, _x_acceleration) ||
           readDbl(line, pos[3], fieldLen, _health        ) ) {
        _checkState = bad;
        return;
      }
    }

    else if ( iLine == 2 ) {
      if ( readDbl(line, pos[0], fieldLen, _y_pos           ) ||
           readDbl(line, pos[1], fieldLen, _y_velocity      ) ||
           readDbl(line, pos[2], fieldLen, _y_acceleration  ) ||
           readDbl(line, pos[3], fieldLen, _frequency_number) ) {
        _checkState = bad;
        return;
      }
    }

    else if ( iLine == 3 ) {
      if ( readDbl(line, pos[0], fieldLen, _z_pos         )  ||
           readDbl(line, pos[1], fieldLen, _z_velocity    )  ||
           readDbl(line, pos[2], fieldLen, _z_acceleration)  ||
           readDbl(line, pos[3], fieldLen, _E             ) ) {
        _checkState = bad;
        return;
      }
    }
  }

  // Initialize status vector
  // ------------------------
  _tt = _TOC;
  _xv.ReSize(6); _xv = 0.0;
  _xv(1) = _x_pos * 1.e3;
  _xv(2) = _y_pos * 1.e3;
  _xv(3) = _z_pos * 1.e3;
  _xv(4) = _x_velocity * 1.e3;
  _xv(5) = _y_velocity * 1.e3;
  _xv(6) = _z_velocity * 1.e3;
}

// Compute Glonass Satellite Position (virtual)
////////////////////////////////////////////////////////////////////////////
t_irc t_ephGlo::position(int GPSweek, double GPSweeks, double* xc, double* vv) const {

  static const double nominalStep = 10.0;

  memset(xc, 0, 6*sizeof(double));
  memset(vv, 0, 3*sizeof(double));

  double dtPos = bncTime(GPSweek, GPSweeks) - _tt;

  if (fabs(dtPos) > 24 * 3600.0) {
    return failure;
  }

  int nSteps  = int(fabs(dtPos) / nominalStep) + 1;
  double step = dtPos / nSteps;

  double acc[3];
  acc[0] = _x_acceleration * 1.e3;
  acc[1] = _y_acceleration * 1.e3;
  acc[2] = _z_acceleration * 1.e3;

  for (int ii = 1; ii <= nSteps; ii++) {
    _xv = rungeKutta4(_tt.gpssec(), _xv, step, acc, glo_deriv);
    _tt = _tt + step;
  }

  // Position and Velocity
  // ---------------------
  xc[0] = _xv(1);
  xc[1] = _xv(2);
  xc[2] = _xv(3);

  vv[0] = _xv(4);
  vv[1] = _xv(5);
  vv[2] = _xv(6);

  // Clock Correction
  // ----------------
  double dtClk = bncTime(GPSweek, GPSweeks) - _TOC;
  xc[3] = -_tau + _gamma * dtClk;

  xc[4] = _gamma;
  xc[5] = 0.0;

  return success;
}

// RINEX Format String
//////////////////////////////////////////////////////////////////////////////
QString t_ephGlo::toString(double version) const {

  QString rnxStr = rinexDateStr(_TOC -_gps_utc, _prn, version);
  int nd = int((_TOC - _gps_utc).gpssec()) / (24.0*60.0*60.0);
  QTextStream out(&rnxStr);

  out << QString("%1%2%3\n")
    .arg(-_tau,           19, 'e', 12)
    .arg(_gamma,          19, 'e', 12)
    .arg(_tki+nd*86400.0, 19, 'e', 12);

  QString fmt = version < 3.0 ? "   %1%2%3%4\n" : "    %1%2%3%4\n";

  out << QString(fmt)
    .arg(_x_pos,          19, 'e', 12)
    .arg(_x_velocity,     19, 'e', 12)
    .arg(_x_acceleration, 19, 'e', 12)
    .arg(_health,         19, 'e', 12);

  out << QString(fmt)
    .arg(_y_pos,            19, 'e', 12)
    .arg(_y_velocity,       19, 'e', 12)
    .arg(_y_acceleration,   19, 'e', 12)
    .arg(_frequency_number, 19, 'e', 12);

  out << QString(fmt)
    .arg(_z_pos,          19, 'e', 12)
    .arg(_z_velocity,     19, 'e', 12)
    .arg(_z_acceleration, 19, 'e', 12)
    .arg(_E,              19, 'e', 12);

  return rnxStr;
}

// Derivative of the state vector using a simple force model (static)
////////////////////////////////////////////////////////////////////////////
ColumnVector t_ephGlo::glo_deriv(double /* tt */, const ColumnVector& xv,
                                 double* acc) {

  // State vector components
  // -----------------------
  ColumnVector rr = xv.rows(1,3);
  ColumnVector vv = xv.rows(4,6);

  // Acceleration
  // ------------
  static const double gmWGS = 398.60044e12;
  static const double AE    = 6378136.0;
  static const double OMEGA = 7292115.e-11;
  static const double C20   = -1082.6257e-6;

  double rho = rr.NormFrobenius();
  double t1  = -gmWGS/(rho*rho*rho);
  double t2  = 3.0/2.0 * C20 * (gmWGS*AE*AE) / (rho*rho*rho*rho*rho);
  double t3  = OMEGA * OMEGA;
  double t4  = 2.0 * OMEGA;
  double z2  = rr(3) * rr(3);

  // Vector of derivatives
  // ---------------------
  ColumnVector va(6);
  va(1) = vv(1);
  va(2) = vv(2);
  va(3) = vv(3);
  va(4) = (t1 + t2*(1.0-5.0*z2/(rho*rho)) + t3) * rr(1) + t4*vv(2) + acc[0];
  va(5) = (t1 + t2*(1.0-5.0*z2/(rho*rho)) + t3) * rr(2) - t4*vv(1) + acc[1];
  va(6) = (t1 + t2*(3.0-5.0*z2/(rho*rho))     ) * rr(3)            + acc[2];

  return va;
}

// IOD of Glonass Ephemeris (virtual)
////////////////////////////////////////////////////////////////////////////
unsigned int t_ephGlo::IOD() const {
  bncTime tMoscow = _TOC - _gps_utc + 3 * 3600.0;
  return (unsigned long)tMoscow.daysec() / 900;
}

// Health status of Glonass Ephemeris (virtual)
////////////////////////////////////////////////////////////////////////////
unsigned int t_ephGlo::isUnhealthy() const {

  if (_almanac_health_availablility_indicator) {
      if ((_health == 0 && _almanac_health == 0) ||
          (_health == 1 && _almanac_health == 0) ||
          (_health == 1 && _almanac_health == 1)) {
        return 1;
      }
  }
  else if (!_almanac_health_availablility_indicator) {
    if (_health) {
      return 1;
    }
  }
  return 0; /* (_health == 0 && _almanac_health == 1) or (_health == 0) */
}

// Constructor
//////////////////////////////////////////////////////////////////////////////
t_ephGal::t_ephGal(float rnxVersion, const QStringList& lines) {
  int       year, month, day, hour, min;
  double    sec;
  QString   prnStr;
  const int nLines = 8;
  if (lines.size() != nLines) {
    _checkState = bad;
    return;
  }

  // RINEX Format
  // ------------
  int fieldLen = 19;
  double SVhealth = 0.0;
  double datasource = 0.0;

  int pos[4];
  pos[0] = (rnxVersion <= 2.12) ?  3 :  4;
  pos[1] = pos[0] + fieldLen;
  pos[2] = pos[1] + fieldLen;
  pos[3] = pos[2] + fieldLen;

  // Read eight lines
  // ----------------
  for (int iLine = 0; iLine < nLines; iLine++) {
    QString line = lines[iLine];

    if      ( iLine == 0 ) {
      QTextStream in(line.left(pos[1]).toLatin1());
      QString n;
      in >> prnStr;
      if (prnStr.size() == 1 && prnStr[0] == 'E') {
        in >> n;
        prnStr.append(n);
      }
      in >> year >> month >> day >> hour >> min >> sec;
      if      (year <  80) {
        year += 2000;
      }
      else if (year < 100) {
        year += 1900;
      }

      _TOC.set(year, month, day, hour, min, sec);

      if ( readDbl(line, pos[1], fieldLen, _clock_bias     ) ||
           readDbl(line, pos[2], fieldLen, _clock_drift    ) ||
           readDbl(line, pos[3], fieldLen, _clock_driftrate) ) {
        _checkState = bad;
        return;
      }
    }

    else if      ( iLine == 1 ) {
      if ( readDbl(line, pos[0], fieldLen, _IODnav ) ||
           readDbl(line, pos[1], fieldLen, _Crs    ) ||
           readDbl(line, pos[2], fieldLen, _Delta_n) ||
           readDbl(line, pos[3], fieldLen, _M0     ) ) {
        _checkState = bad;
        return;
      }
    }

    else if ( iLine == 2 ) {
      if ( readDbl(line, pos[0], fieldLen, _Cuc   ) ||
           readDbl(line, pos[1], fieldLen, _e     ) ||
           readDbl(line, pos[2], fieldLen, _Cus   ) ||
           readDbl(line, pos[3], fieldLen, _sqrt_A) ) {
        _checkState = bad;
        return;
      }
    }

    else if ( iLine == 3 ) {
      if ( readDbl(line, pos[0], fieldLen, _TOEsec)  ||
           readDbl(line, pos[1], fieldLen, _Cic   )  ||
           readDbl(line, pos[2], fieldLen, _OMEGA0)  ||
           readDbl(line, pos[3], fieldLen, _Cis   ) ) {
        _checkState = bad;
        return;
      }
    }

    else if ( iLine == 4 ) {
      if ( readDbl(line, pos[0], fieldLen, _i0      ) ||
           readDbl(line, pos[1], fieldLen, _Crc     ) ||
           readDbl(line, pos[2], fieldLen, _omega   ) ||
           readDbl(line, pos[3], fieldLen, _OMEGADOT) ) {
        _checkState = bad;
        return;
      }
    }

    else if ( iLine == 5 ) {
      if ( readDbl(line, pos[0], fieldLen, _IDOT      ) ||
           readDbl(line, pos[1], fieldLen, datasource) ||
           readDbl(line, pos[2], fieldLen, _TOEweek   ) ) {
        _checkState = bad;
        return;
      } else {
        if        (int(datasource) & (1<<8)) {
          _fnav = true;
          _inav = false;
        } else if (int(datasource) & (1<<9)) {
          _fnav = false;
          _inav = true;
        }
        _TOEweek -= 1024.0;
      }
    }

    else if ( iLine == 6 ) {
      if ( readDbl(line, pos[0], fieldLen, _SISA    ) ||
           readDbl(line, pos[1], fieldLen,  SVhealth) ||
           readDbl(line, pos[2], fieldLen, _BGD_1_5A) ||
           readDbl(line, pos[3], fieldLen, _BGD_1_5B) ) {
        _checkState = bad;
        return;
      } else {
        // Bit 0
        _e1DataInValid = (int(SVhealth) & (1<<0));
        // Bit 1-2
        _E1_bHS = double((int(SVhealth) >> 1) & 0x3);
        // Bit 3
        _e5aDataInValid = (int(SVhealth) & (1<<3));
        // Bit 4-5
        _E5aHS = double((int(SVhealth) >> 4) & 0x3);
        // Bit 6
        _e5bDataInValid = (int(SVhealth) & (1<<6));
        // Bit 7-8
        _E5bHS = double((int(SVhealth) >> 7) & 0x3);

        if (prnStr.at(0) == 'E') {
          _prn.set('E', prnStr.mid(1).toInt(), _inav ? 1 : 0);
        }
      }
    }

    else if ( iLine == 7 ) {
      if ( readDbl(line, pos[0], fieldLen, _TOT) ) {
        _checkState = bad;
        return;
      }
    }
  }
}

// Compute Galileo Satellite Position (virtual)
////////////////////////////////////////////////////////////////////////////
t_irc t_ephGal::position(int GPSweek, double GPSweeks, double* xc, double* vv) const {

  static const double omegaEarth = 7292115.1467e-11;
  static const double gmWGS = 398.6004418e12;

  memset(xc, 0, 6*sizeof(double));
  memset(vv, 0, 3*sizeof(double));

  double a0 = _sqrt_A * _sqrt_A;
  if (a0 == 0) {
    return failure;
  }

  double n0 = sqrt(gmWGS/(a0*a0*a0));

  bncTime tt(GPSweek, GPSweeks);
  double tk = tt - bncTime(_TOC.gpsw(), _TOEsec);

  double n  = n0 + _Delta_n;
  double M  = _M0 + n*tk;
  double E  = M;
  double E_last;
  int    nLoop = 0;
  do {
    E_last = E;
    E = M + _e*sin(E);

    if (++nLoop == 100) {
      return failure;
    }
  } while ( fabs(E-E_last)*a0 > 0.001 );
  double v      = 2.0*atan( sqrt( (1.0 + _e)/(1.0 - _e) )*tan( E/2 ) );
  double u0     = v + _omega;
  double sin2u0 = sin(2*u0);
  double cos2u0 = cos(2*u0);
  double r      = a0*(1 - _e*cos(E)) + _Crc*cos2u0 + _Crs*sin2u0;
  double i      = _i0 + _IDOT*tk + _Cic*cos2u0 + _Cis*sin2u0;
  double u      = u0 + _Cuc*cos2u0 + _Cus*sin2u0;
  double xp     = r*cos(u);
  double yp     = r*sin(u);
  double OM     = _OMEGA0 + (_OMEGADOT - omegaEarth)*tk -
                  omegaEarth*_TOEsec;

  double sinom = sin(OM);
  double cosom = cos(OM);
  double sini  = sin(i);
  double cosi  = cos(i);
  xc[0] = xp*cosom - yp*cosi*sinom;
  xc[1] = xp*sinom + yp*cosi*cosom;
  xc[2] = yp*sini;

  double tc = tt - _TOC;
  xc[3] = _clock_bias + _clock_drift*tc + _clock_driftrate*tc*tc;

  // Velocity
  // --------
  double tanv2 = tan(v/2);
  double dEdM  = 1 / (1 - _e*cos(E));
  double dotv  = sqrt((1.0 + _e)/(1.0 - _e)) / cos(E/2)/cos(E/2) / (1 + tanv2*tanv2)
               * dEdM * n;
  double dotu  = dotv + (-_Cuc*sin2u0 + _Cus*cos2u0)*2*dotv;
  double dotom = _OMEGADOT - omegaEarth;
  double doti  = _IDOT + (-_Cic*sin2u0 + _Cis*cos2u0)*2*dotv;
  double dotr  = a0 * _e*sin(E) * dEdM * n
                + (-_Crc*sin2u0 + _Crs*cos2u0)*2*dotv;
  double dotx  = dotr*cos(u) - r*sin(u)*dotu;
  double doty  = dotr*sin(u) + r*cos(u)*dotu;

  vv[0]  = cosom   *dotx  - cosi*sinom   *doty      // dX / dr
           - xp*sinom*dotom - yp*cosi*cosom*dotom   // dX / dOMEGA
                       + yp*sini*sinom*doti;        // dX / di

  vv[1]  = sinom   *dotx  + cosi*cosom   *doty
           + xp*cosom*dotom - yp*cosi*sinom*dotom
                          - yp*sini*cosom*doti;

  vv[2]  = sini    *doty  + yp*cosi      *doti;

  // Relativistic Correction
  // -----------------------
  // correspondent to Galileo ICD and to SSR standard
  xc[3] -= 4.442807633e-10 * _e * sqrt(a0) *sin(E);
  // correspondent to IGS convention
  //xc[3] -= 2.0 * (xc[0]*vv[0] + xc[1]*vv[1] + xc[2]*vv[2]) / t_CST::c / t_CST::c;

  xc[4] = _clock_drift + _clock_driftrate*tc;
  xc[5] = _clock_driftrate;

  return success;
}

// Health status of Galileo Ephemeris (virtual)
////////////////////////////////////////////////////////////////////////////
unsigned int t_ephGal::isUnhealthy() const {
  if (_E5aHS && _E5bHS && _E1_bHS) {
    return 1;
  }
  return 0;
}

// RINEX Format String
//////////////////////////////////////////////////////////////////////////////
QString t_ephGal::toString(double version) const {

  QString rnxStr = rinexDateStr(_TOC, _prn, version);

  QTextStream out(&rnxStr);

  out << QString("%1%2%3\n")
    .arg(_clock_bias,      19, 'e', 12)
    .arg(_clock_drift,     19, 'e', 12)
    .arg(_clock_driftrate, 19, 'e', 12);

  QString fmt = version < 3.0 ? "   %1%2%3%4\n" : "    %1%2%3%4\n";

  out << QString(fmt)
    .arg(_IODnav,  19, 'e', 12)
    .arg(_Crs,     19, 'e', 12)
    .arg(_Delta_n, 19, 'e', 12)
    .arg(_M0,      19, 'e', 12);

  out << QString(fmt)
    .arg(_Cuc,    19, 'e', 12)
    .arg(_e,      19, 'e', 12)
    .arg(_Cus,    19, 'e', 12)
    .arg(_sqrt_A, 19, 'e', 12);

  out << QString(fmt)
    .arg(_TOEsec, 19, 'e', 12)
    .arg(_Cic,    19, 'e', 12)
    .arg(_OMEGA0, 19, 'e', 12)
    .arg(_Cis,    19, 'e', 12);

  out << QString(fmt)
    .arg(_i0,       19, 'e', 12)
    .arg(_Crc,      19, 'e', 12)
    .arg(_omega,    19, 'e', 12)
    .arg(_OMEGADOT, 19, 'e', 12);

  int    dataSource = 0;
  int    SVhealth   = 0;
  double BGD_1_5A   = _BGD_1_5A;
  double BGD_1_5B   = _BGD_1_5B;
  if (_fnav) {
    dataSource |= (1<<1);
    dataSource |= (1<<8);
    BGD_1_5B = 0.0;
    // SVhealth
    //   Bit 3  : E5a DVS
    if (_e5aDataInValid) {
      SVhealth |= (1<<3);
    }
    //   Bit 4-5: E5a HS
    if (_E5aHS == 1.0) {
      SVhealth |= (1<<4);
    }
    else if (_E5aHS == 2.0) {
      SVhealth |= (1<<5);
    }
    else if (_E5aHS  == 3.0) {
      SVhealth |= (1<<4);
      SVhealth |= (1<<5);
    }
  }
  else if(_inav) {
    // Bit 2 and 0 are set because from MT1046 the data source cannot be determined
    // and RNXv3.03 says both can be set if the navigation messages were merged
    dataSource |= (1<<0);
    dataSource |= (1<<2);
    dataSource |= (1<<9);
    // SVhealth
    //   Bit 0  : E1-B DVS
    if (_e1DataInValid) {
      SVhealth |= (1<<0);
    }
    //   Bit 1-2: E1-B HS
    if      (_E1_bHS == 1.0) {
      SVhealth |= (1<<1);
    }
    else if (_E1_bHS == 2.0) {
      SVhealth |= (1<<2);
    }
    else if (_E1_bHS == 3.0) {
      SVhealth |= (1<<1);
      SVhealth |= (1<<2);
    }
    //   Bit 3  : E5a DVS
    if (_e5aDataInValid) {
      SVhealth |= (1<<3);
    }
    //   Bit 4-5: E5a HS
    if      (_E5aHS == 1.0) {
      SVhealth |= (1<<4);
    }
    else if (_E5aHS == 2.0) {
      SVhealth |= (1<<5);
    }
    else if (_E5aHS == 3.0) {
      SVhealth |= (1<<4);
      SVhealth |= (1<<5);
    }
    //   Bit 6  : E5b DVS
    if (_e5bDataInValid) {
      SVhealth |= (1<<6);
    }
    //   Bit 7-8: E5b HS
    if      (_E5bHS == 1.0) {
      SVhealth |= (1<<7);
    }
    else if (_E5bHS == 2.0) {
      SVhealth |= (1<<8);
    }
    else if (_E5bHS == 3.0) {
      SVhealth |= (1<<7);
      SVhealth |= (1<<8);
    }
  }

  out << QString(fmt)
    .arg(_IDOT,              19, 'e', 12)
    .arg(double(dataSource), 19, 'e', 12)
    .arg(_TOEweek + 1024.0,  19, 'e', 12)
    .arg(0.0,                19, 'e', 12);

  out << QString(fmt)
    .arg(_SISA,            19, 'e', 12)
    .arg(double(SVhealth), 19, 'e', 12)
    .arg(BGD_1_5A,         19, 'e', 12)
    .arg(BGD_1_5B,         19, 'e', 12);

  double tot = _TOT;
  if (tot == 0.9999e9 && version < 3.0) {
    tot = 0.0;
  }
  out << QString(fmt)
    .arg(tot,     19, 'e', 12)
    .arg("",      19, QChar(' '))
    .arg("",      19, QChar(' '))
    .arg("",      19, QChar(' '));

  return rnxStr;
}

// Constructor
//////////////////////////////////////////////////////////////////////////////
t_ephSBAS::t_ephSBAS(float rnxVersion, const QStringList& lines) {

  const int nLines = 4;

  if (lines.size() != nLines) {
    _checkState = bad;
    return;
  }

  // RINEX Format
  // ------------
  int fieldLen = 19;

  int pos[4];
  pos[0] = (rnxVersion <= 2.12) ?  3 :  4;
  pos[1] = pos[0] + fieldLen;
  pos[2] = pos[1] + fieldLen;
  pos[3] = pos[2] + fieldLen;

  // Read four lines
  // ---------------
  for (int iLine = 0; iLine < nLines; iLine++) {
    QString line = lines[iLine];

    if      ( iLine == 0 ) {
      QTextStream in(line.left(pos[1]).toLatin1());

      int    year, month, day, hour, min;
      double sec;

      QString prnStr, n;
      in >> prnStr;
      if (prnStr.size() == 1 && prnStr[0] == 'S') {
        in >> n;
        prnStr.append(n);
      }
      in >> year >> month >> day >> hour >> min >> sec;
      if (prnStr.at(0) == 'S') {
        _prn.set('S', prnStr.mid(1).toInt());
      }
      else {
        _prn.set('S', prnStr.toInt());
      }

      if      (year <  80) {
        year += 2000;
      }
      else if (year < 100) {
        year += 1900;
      }

      _TOC.set(year, month, day, hour, min, sec);

      if ( readDbl(line, pos[1], fieldLen, _agf0 ) ||
           readDbl(line, pos[2], fieldLen, _agf1 ) ||
           readDbl(line, pos[3], fieldLen, _TOT  ) ) {
        _checkState = bad;
        return;
      }
    }

    else if      ( iLine == 1 ) {
      if ( readDbl(line, pos[0], fieldLen, _x_pos         ) ||
           readDbl(line, pos[1], fieldLen, _x_velocity    ) ||
           readDbl(line, pos[2], fieldLen, _x_acceleration) ||
           readDbl(line, pos[3], fieldLen, _health        ) ) {
        _checkState = bad;
        return;
      }
    }

    else if ( iLine == 2 ) {
      if ( readDbl(line, pos[0], fieldLen, _y_pos           ) ||
           readDbl(line, pos[1], fieldLen, _y_velocity      ) ||
           readDbl(line, pos[2], fieldLen, _y_acceleration  ) ||
           readDbl(line, pos[3], fieldLen, _ura             ) ) {
        _checkState = bad;
        return;
      }
    }

    else if ( iLine == 3 ) {
      double iodn;
      if ( readDbl(line, pos[0], fieldLen, _z_pos         )  ||
           readDbl(line, pos[1], fieldLen, _z_velocity    )  ||
           readDbl(line, pos[2], fieldLen, _z_acceleration)  ||
           readDbl(line, pos[3], fieldLen, iodn           ) ) {
        _checkState = bad;
        return;
      } else {
        _IODN = int(iodn);
      }
    }
  }

  _x_pos          *= 1.e3;
  _y_pos          *= 1.e3;
  _z_pos          *= 1.e3;
  _x_velocity     *= 1.e3;
  _y_velocity     *= 1.e3;
  _z_velocity     *= 1.e3;
  _x_acceleration *= 1.e3;
  _y_acceleration *= 1.e3;
  _z_acceleration *= 1.e3;
}

// IOD of SBAS Ephemeris (virtual)
////////////////////////////////////////////////////////////////////////////

unsigned int t_ephSBAS::IOD() const {
  unsigned char buffer[80];
  int size = 0;
  int numbits = 0;
  long long bitbuffer = 0;
  unsigned char *startbuffer = buffer;

  SBASADDBITSFLOAT(30, this->_x_pos, 0.08)
  SBASADDBITSFLOAT(30, this->_y_pos, 0.08)
  SBASADDBITSFLOAT(25, this->_z_pos, 0.4)
  SBASADDBITSFLOAT(17, this->_x_velocity, 0.000625)
  SBASADDBITSFLOAT(17, this->_y_velocity, 0.000625)
  SBASADDBITSFLOAT(18, this->_z_velocity, 0.004)
  SBASADDBITSFLOAT(10, this->_x_acceleration, 0.0000125)
  SBASADDBITSFLOAT(10, this->_y_acceleration, 0.0000125)
  SBASADDBITSFLOAT(10, this->_z_acceleration, 0.0000625)
  SBASADDBITSFLOAT(12, this->_agf0, 1.0/static_cast<double>(1<<30)/static_cast<double>(1<<1))
  SBASADDBITSFLOAT(8, this->_agf1, 1.0/static_cast<double>(1<<30)/static_cast<double>(1<<10))
  SBASADDBITS(5,0); // the last byte is filled by 0-bits to obtain a length of an integer multiple of 8

  return CRC24(size, startbuffer);
}

// Compute SBAS Satellite Position (virtual)
////////////////////////////////////////////////////////////////////////////
t_irc t_ephSBAS::position(int GPSweek, double GPSweeks, double* xc, double* vv) const {

  bncTime tt(GPSweek, GPSweeks);
  double  dt = tt - _TOC;

  xc[0] = _x_pos + _x_velocity * dt + _x_acceleration * dt * dt / 2.0;
  xc[1] = _y_pos + _y_velocity * dt + _y_acceleration * dt * dt / 2.0;
  xc[2] = _z_pos + _z_velocity * dt + _z_acceleration * dt * dt / 2.0;

  vv[0] = _x_velocity + _x_acceleration * dt;
  vv[1] = _y_velocity + _y_acceleration * dt;
  vv[2] = _z_velocity + _z_acceleration * dt;

  xc[3] = _agf0 + _agf1 * dt;

  xc[4] = _agf1;
  xc[5] = 0.0;

  return success;
}

// RINEX Format String
//////////////////////////////////////////////////////////////////////////////
QString t_ephSBAS::toString(double version) const {

  QString rnxStr = rinexDateStr(_TOC, _prn, version);

  QTextStream out(&rnxStr);

  out << QString("%1%2%3\n")
    .arg(_agf0, 19, 'e', 12)
    .arg(_agf1, 19, 'e', 12)
    .arg(_TOT,  19, 'e', 12);

  QString fmt = version < 3.0 ? "   %1%2%3%4\n" : "    %1%2%3%4\n";

  out << QString(fmt)
    .arg(1.e-3*_x_pos,          19, 'e', 12)
    .arg(1.e-3*_x_velocity,     19, 'e', 12)
    .arg(1.e-3*_x_acceleration, 19, 'e', 12)
    .arg(_health,               19, 'e', 12);

  out << QString(fmt)
    .arg(1.e-3*_y_pos,          19, 'e', 12)
    .arg(1.e-3*_y_velocity,     19, 'e', 12)
    .arg(1.e-3*_y_acceleration, 19, 'e', 12)
    .arg(_ura,                  19, 'e', 12);

  out << QString(fmt)
    .arg(1.e-3*_z_pos,          19, 'e', 12)
    .arg(1.e-3*_z_velocity,     19, 'e', 12)
    .arg(1.e-3*_z_acceleration, 19, 'e', 12)
    .arg(double(_IODN),         19, 'e', 12);

  return rnxStr;
}

// Constructor
//////////////////////////////////////////////////////////////////////////////
t_ephBDS::t_ephBDS(float rnxVersion, const QStringList& lines) {

  const int nLines = 8;

  if (lines.size() != nLines) {
    _checkState = bad;
    return;
  }

  // RINEX Format
  // ------------
  int fieldLen = 19;

  int pos[4];
  pos[0] = (rnxVersion <= 2.12) ?  3 :  4;
  pos[1] = pos[0] + fieldLen;
  pos[2] = pos[1] + fieldLen;
  pos[3] = pos[2] + fieldLen;

  // Read eight lines
  // ----------------
  for (int iLine = 0; iLine < nLines; iLine++) {
    QString line = lines[iLine];

    if      ( iLine == 0 ) {
      QTextStream in(line.left(pos[1]).toLatin1());

      int    year, month, day, hour, min;
      double sec;

      QString prnStr, n;
      in >> prnStr;
      if (prnStr.size() == 1 && prnStr[0] == 'C') {
        in >> n;
        prnStr.append(n);
      }
      in >> year >> month >> day >> hour >> min >> sec;
      if (prnStr.at(0) == 'C') {
        _prn.set('C', prnStr.mid(1).toInt());
      }
      else {
        _prn.set('C', prnStr.toInt());
      }

      if      (year <  80) {
        year += 2000;
      }
      else if (year < 100) {
        year += 1900;
      }

      _TOC.setBDS(year, month, day, hour, min, sec);

      if ( readDbl(line, pos[1], fieldLen, _clock_bias     ) ||
           readDbl(line, pos[2], fieldLen, _clock_drift    ) ||
           readDbl(line, pos[3], fieldLen, _clock_driftrate) ) {
        _checkState = bad;
        return;
      }
    }

    else if      ( iLine == 1 ) {
      double aode;
      if ( readDbl(line, pos[0], fieldLen, aode    ) ||
           readDbl(line, pos[1], fieldLen, _Crs    ) ||
           readDbl(line, pos[2], fieldLen, _Delta_n) ||
           readDbl(line, pos[3], fieldLen, _M0     ) ) {
        _checkState = bad;
        return;
      }
      _AODE = int(aode);
    }

    else if ( iLine == 2 ) {
      if ( readDbl(line, pos[0], fieldLen, _Cuc   ) ||
           readDbl(line, pos[1], fieldLen, _e     ) ||
           readDbl(line, pos[2], fieldLen, _Cus   ) ||
           readDbl(line, pos[3], fieldLen, _sqrt_A) ) {
        _checkState = bad;
        return;
      }
    }

    else if ( iLine == 3 ) {
      if ( readDbl(line, pos[0], fieldLen, _TOEsec )  ||
           readDbl(line, pos[1], fieldLen, _Cic   )  ||
           readDbl(line, pos[2], fieldLen, _OMEGA0)  ||
           readDbl(line, pos[3], fieldLen, _Cis   ) ) {
        _checkState = bad;
        return;
      }
    }

    else if ( iLine == 4 ) {
      if ( readDbl(line, pos[0], fieldLen, _i0      ) ||
           readDbl(line, pos[1], fieldLen, _Crc     ) ||
           readDbl(line, pos[2], fieldLen, _omega   ) ||
           readDbl(line, pos[3], fieldLen, _OMEGADOT) ) {
        _checkState = bad;
        return;
      }
    }

    else if ( iLine == 5 ) {
      if ( readDbl(line, pos[0], fieldLen, _IDOT    ) ||
           readDbl(line, pos[2], fieldLen, _TOEweek)) {
        _checkState = bad;
        return;
      }
    }

    else if ( iLine == 6 ) {
      double SatH1;
      if ( readDbl(line, pos[0], fieldLen, _URA ) ||
           readDbl(line, pos[1], fieldLen, SatH1) ||
           readDbl(line, pos[2], fieldLen, _TGD1) ||
           readDbl(line, pos[3], fieldLen, _TGD2) ) {
        _checkState = bad;
        return;
      }
      _SatH1 = int(SatH1);
    }

    else if ( iLine == 7 ) {
      double aodc;
      if ( readDbl(line, pos[0], fieldLen, _TOT) ||
           readDbl(line, pos[1], fieldLen, aodc) ) {
        _checkState = bad;
        return;
      }
      if (_TOT == 0.9999e9) {  // 0.9999e9 means not known (RINEX standard)
        _TOT = _TOEsec;
      }
      _AODC = int(aodc);
    }
  }

  _TOE.setBDS(int(_TOEweek), _TOEsec);

  // remark: actually should be computed from second_tot
  //         but it seems to be unreliable in RINEX files
  //_TOT = _TOC.bdssec();
}

// IOD of BDS Ephemeris (virtual)
////////////////////////////////////////////////////////////////////////////
unsigned int t_ephBDS::IOD() const {
  return (int(_TOEsec)/720) % 240;
}

// Compute BDS Satellite Position (virtual)
//////////////////////////////////////////////////////////////////////////////
t_irc t_ephBDS::position(int GPSweek, double GPSweeks, double* xc, double* vv) const {

  static const double gmBDS    = 398.6004418e12;
  static const double omegaBDS = 7292115.0000e-11;

  xc[0] = xc[1] = xc[2] = xc[3] = 0.0;
  vv[0] = vv[1] = vv[2] = 0.0;

  bncTime tt(GPSweek, GPSweeks);

  if (_sqrt_A == 0) {
    return failure;
  }
  double a0 = _sqrt_A * _sqrt_A;

  double n0 = sqrt(gmBDS/(a0*a0*a0));
  double tk = tt - _TOE;
  double n  = n0 + _Delta_n;
  double M  = _M0 + n*tk;
  double E  = M;
  double E_last;
  int    nLoop = 0;
  do {
    E_last = E;
    E = M + _e*sin(E);

    if (++nLoop == 100) {
      return failure;
    }
  } while ( fabs(E-E_last)*a0 > 0.001 );

  double v      = atan2(sqrt(1-_e*_e) * sin(E), cos(E) - _e);
  double u0     = v + _omega;
  double sin2u0 = sin(2*u0);
  double cos2u0 = cos(2*u0);
  double r      = a0*(1 - _e*cos(E)) + _Crc*cos2u0 + _Crs*sin2u0;
  double i      = _i0 + _IDOT*tk     + _Cic*cos2u0 + _Cis*sin2u0;
  double u      = u0                 + _Cuc*cos2u0 + _Cus*sin2u0;
  double xp     = r*cos(u);
  double yp     = r*sin(u);
  double toesec = (_TOE.gpssec() - 14.0);
  double sinom = 0;
  double cosom = 0;
  double sini  = 0;
  double cosi  = 0;

  // Velocity
  // --------
  double tanv2 = tan(v/2);
  double dEdM  = 1 / (1 - _e*cos(E));
  double dotv  = sqrt((1.0 + _e)/(1.0 - _e)) / cos(E/2)/cos(E/2)
                 / (1 + tanv2*tanv2) * dEdM * n;
  double dotu  = dotv + (-_Cuc*sin2u0 + _Cus*cos2u0)*2*dotv;
  double doti  = _IDOT + (-_Cic*sin2u0 + _Cis*cos2u0)*2*dotv;
  double dotr  = a0 * _e*sin(E) * dEdM * n
                + (-_Crc*sin2u0 + _Crs*cos2u0)*2*dotv;

  double dotx  = dotr*cos(u) - r*sin(u)*dotu;
  double doty  = dotr*sin(u) + r*cos(u)*dotu;

  const double iMaxGEO = 10.0 / 180.0 * M_PI;

  // MEO/IGSO satellite
  // ------------------
  if (_i0 > iMaxGEO) {
    double OM = _OMEGA0 + (_OMEGADOT - omegaBDS)*tk - omegaBDS*toesec;

    sinom = sin(OM);
    cosom = cos(OM);
    sini  = sin(i);
    cosi  = cos(i);

    xc[0] = xp*cosom - yp*cosi*sinom;
    xc[1] = xp*sinom + yp*cosi*cosom;
    xc[2] = yp*sini;

    // Velocity
    // --------

    double dotom = _OMEGADOT - t_CST::omega;

    vv[0]  = cosom  *dotx   - cosi*sinom   *doty    // dX / dr
           - xp*sinom*dotom - yp*cosi*cosom*dotom   // dX / dOMEGA
                            + yp*sini*sinom*doti;   // dX / di

    vv[1]  = sinom  *dotx   + cosi*cosom   *doty
           + xp*cosom*dotom - yp*cosi*sinom*dotom
                            - yp*sini*cosom*doti;

    vv[2]  = sini   *doty   + yp*cosi      *doti;

  }

  // GEO satellite
  // -------------
  else {
    double OM    = _OMEGA0 + _OMEGADOT*tk - omegaBDS*toesec;
    double ll    = omegaBDS*tk;

    sinom = sin(OM);
    cosom = cos(OM);
    sini  = sin(i);
    cosi  = cos(i);

    double xx = xp*cosom - yp*cosi*sinom;
    double yy = xp*sinom + yp*cosi*cosom;
    double zz = yp*sini;

    Matrix RX = BNC_PPP::t_astro::rotX(-5.0 / 180.0 * M_PI);
    Matrix RZ = BNC_PPP::t_astro::rotZ(ll);

    ColumnVector X1(3); X1 << xx << yy << zz;
    ColumnVector X2 = RZ*RX*X1;

    xc[0] = X2(1);
    xc[1] = X2(2);
    xc[2] = X2(3);

    double dotom = _OMEGADOT;

    double vx  = cosom   *dotx  - cosi*sinom   *doty
               - xp*sinom*dotom - yp*cosi*cosom*dotom
                                + yp*sini*sinom*doti;

    double vy  = sinom   *dotx  + cosi*cosom   *doty
               + xp*cosom*dotom - yp*cosi*sinom*dotom
                                - yp*sini*cosom*doti;

    double vz  = sini    *doty  + yp*cosi      *doti;

    ColumnVector V(3); V << vx << vy << vz;

    Matrix RdotZ(3,3);
    double C = cos(ll);
    double S = sin(ll);
    Matrix UU(3,3);
    UU[0][0] =  -S;  UU[0][1] =  +C;  UU[0][2] = 0.0;
    UU[1][0] =  -C;  UU[1][1] =  -S;  UU[1][2] = 0.0;
    UU[2][0] = 0.0;  UU[2][1] = 0.0;  UU[2][2] = 0.0;
    RdotZ = omegaBDS * UU;

    ColumnVector VV(3);
    VV = RZ*RX*V + RdotZ*RX*X1;

    vv[0] = VV(1);
    vv[1] = VV(2);
    vv[2] = VV(3);
  }

  double tc = tt - _TOC;
  xc[3] = _clock_bias + _clock_drift*tc + _clock_driftrate*tc*tc;

  // dotC  = _clock_drift + _clock_driftrate*tc
  //       - 4.442807633e-10*_e*sqrt(a0)*cos(E) * dEdM * n;

  // Relativistic Correction
  // -----------------------
  // correspondent to BDS ICD and to SSR standard
  xc[3] -= 4.442807633e-10 * _e * sqrt(a0) *sin(E);
  // correspondent to IGS convention
  // xc[3] -= 2.0 * (xc[0]*vv[0] + xc[1]*vv[1] + xc[2]*vv[2]) / t_CST::c / t_CST::c;

  xc[4] = _clock_drift + _clock_driftrate*tc;
  xc[5] = _clock_driftrate;
  return success;
}

// RINEX Format String
//////////////////////////////////////////////////////////////////////////////
QString t_ephBDS::toString(double version) const {

  QString rnxStr = rinexDateStr(_TOC-14.0, _prn, version);

  QTextStream out(&rnxStr);

  out << QString("%1%2%3\n")
    .arg(_clock_bias,      19, 'e', 12)
    .arg(_clock_drift,     19, 'e', 12)
    .arg(_clock_driftrate, 19, 'e', 12);

  QString fmt = version < 3.0 ? "   %1%2%3%4\n" : "    %1%2%3%4\n";

  out << QString(fmt)
    .arg(double(_AODE), 19, 'e', 12)
    .arg(_Crs,          19, 'e', 12)
    .arg(_Delta_n,      19, 'e', 12)
    .arg(_M0,           19, 'e', 12);

  out << QString(fmt)
    .arg(_Cuc,    19, 'e', 12)
    .arg(_e,      19, 'e', 12)
    .arg(_Cus,    19, 'e', 12)
    .arg(_sqrt_A, 19, 'e', 12);

  double toes = 0.0;
  if (_TOEweek > -1.0) {// RINEX input
    toes = _TOEsec;
  }
  else {// RTCM stream input
    toes = _TOE.bdssec();
  }
  out << QString(fmt)
    .arg(toes,    19, 'e', 12)
    .arg(_Cic,    19, 'e', 12)
    .arg(_OMEGA0, 19, 'e', 12)
    .arg(_Cis,    19, 'e', 12);

  out << QString(fmt)
    .arg(_i0,       19, 'e', 12)
    .arg(_Crc,      19, 'e', 12)
    .arg(_omega,    19, 'e', 12)
    .arg(_OMEGADOT, 19, 'e', 12);

  double toew = 0.0;
  if (_TOEweek > -1.0) {// RINEX input
    toew = _TOEweek;
  }
  else {// RTCM stream input
    toew = double(_TOE.bdsw());
  }
  out << QString(fmt)
    .arg(_IDOT, 19, 'e', 12)
    .arg(0.0,   19, 'e', 12)
    .arg(toew,  19, 'e', 12)
    .arg(0.0,   19, 'e', 12);

  out << QString(fmt)
    .arg(_URA,           19, 'e', 12)
    .arg(double(_SatH1), 19, 'e', 12)
    .arg(_TGD1,          19, 'e', 12)
    .arg(_TGD2,          19, 'e', 12);

  double tots = 0.0;
  if (_TOEweek > -1.0) {// RINEX input
    tots = _TOT;
  }
  else {// RTCM stream input
    tots = _TOE.bdssec();
  }
  out << QString(fmt)
    .arg(tots,          19, 'e', 12)
    .arg(double(_AODC), 19, 'e', 12)
    .arg("",            19, QChar(' '))
    .arg("",            19, QChar(' '));
  return rnxStr;
}
