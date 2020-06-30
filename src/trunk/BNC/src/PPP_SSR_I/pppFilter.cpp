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
 * Class:      t_pppParam, t_pppFilter
 *
 * Purpose:    Model for PPP
 *
 * Author:     L. Mervart
 *
 * Created:    01-Dec-2009
 *
 * Changes:
 *
 * -----------------------------------------------------------------------*/

#include <iomanip>
#include <cmath>
#include <sstream>
#include <newmatio.h>
#include <newmatap.h>

#include "pppFilter.h"
#include "pppClient.h"
#include "bncutils.h"
#include "bncantex.h"
#include "pppOptions.h"
#include "pppModel.h"

using namespace BNC_PPP;
using namespace std;

const double   MAXRES_CODE           = 2.98 * 3.0;
const double   MAXRES_PHASE_GPS      = 0.04;
const double   MAXRES_PHASE_GLONASS  = 2.98 * 0.03;
const double   GLONASS_WEIGHT_FACTOR = 5.0;
const double   BDS_WEIGHT_FACTOR     = 2.0;

#define LOG (_pppClient->log())
#define OPT (_pppClient->opt())

// Constructor
////////////////////////////////////////////////////////////////////////////
t_pppParam::t_pppParam(t_pppParam::parType typeIn, int indexIn,
                   const QString& prnIn) {
  type      = typeIn;
  index     = indexIn;
  prn       = prnIn;
  index_old = 0;
  xx        = 0.0;
  numEpo    = 0;
}

// Destructor
////////////////////////////////////////////////////////////////////////////
t_pppParam::~t_pppParam() {
}

// Partial
////////////////////////////////////////////////////////////////////////////
double t_pppParam::partial(t_satData* satData, bool phase) {

  Tracer tracer("t_pppParam::partial");

  // Coordinates
  // -----------
  if      (type == CRD_X) {
    return (xx - satData->xx(1)) / satData->rho;
  }
  else if (type == CRD_Y) {
    return (xx - satData->xx(2)) / satData->rho;
  }
  else if (type == CRD_Z) {
    return (xx - satData->xx(3)) / satData->rho;
  }

  // Receiver Clocks
  // ---------------
  else if (type == RECCLK) {
    return 1.0;
  }

  // Troposphere
  // -----------
  else if (type == TROPO) {
    return 1.0 / sin(satData->eleSat);
  }

  // Glonass Offset
  // --------------
  else if (type == GLONASS_OFFSET) {
    if (satData->prn[0] == 'R') {
      return 1.0;
    }
    else {
      return 0.0;
    }
  }

  // Galileo Offset
  // --------------
  else if (type == GALILEO_OFFSET) {
    if (satData->prn[0] == 'E') {
      return 1.0;
    }
    else {
      return 0.0;
    }
  }

  // BDS Offset
  // ----------
  else if (type == BDS_OFFSET) {
    if (satData->prn[0] == 'C') {
      return 1.0;
    }
    else {
      return 0.0;
    }
  }

  // Ambiguities
  // -----------
  else if (type == AMB_L3) {
    if (phase && satData->prn == prn) {
      return 1.0;
    }
    else {
      return 0.0;
    }
  }

  // Default return
  // --------------
  return 0.0;
}

// Constructor
////////////////////////////////////////////////////////////////////////////
t_pppFilter::t_pppFilter(t_pppClient* pppClient) {

  _pppClient = pppClient;
  _tides     = new t_tides();

  // Antenna Name, ANTEX File
  // ------------------------
  _antex = 0;
  if (!OPT->_antexFileName.empty()) {
    _antex = new bncAntex(OPT->_antexFileName.c_str());
  }

  // Bancroft Coordinates
  // --------------------
  _xcBanc.ReSize(4);  _xcBanc  = 0.0;
  _ellBanc.ReSize(3); _ellBanc = 0.0;

  // Save copy of data (used in outlier detection)
  // ---------------------------------------------
  _epoData_sav = new t_epoData();

  // Some statistics
  // ---------------
  _neu.ReSize(3); _neu = 0.0;
  _numSat = 0;
  _hDop   = 0.0;
}

// Destructor
////////////////////////////////////////////////////////////////////////////
t_pppFilter::~t_pppFilter() {
  delete _tides;
  delete _antex;
  for (int iPar = 1; iPar <= _params.size(); iPar++) {
    delete _params[iPar-1];
  }
  for (int iPar = 1; iPar <= _params_sav.size(); iPar++) {
    delete _params_sav[iPar-1];
  }
  delete _epoData_sav;
}

// Reset Parameters and Variance-Covariance Matrix
////////////////////////////////////////////////////////////////////////////
void t_pppFilter::reset() {

  Tracer tracer("t_pppFilter::reset");

  double lastTrp = 0.0;
  for (int ii = 0; ii < _params.size(); ii++) {
    t_pppParam* pp = _params[ii];
    if (pp->type == t_pppParam::TROPO) {
      lastTrp = pp->xx;
    }
    delete pp;
  }
  _params.clear();

  int nextPar = 0;
  _params.push_back(new t_pppParam(t_pppParam::CRD_X,  ++nextPar, ""));
  _params.push_back(new t_pppParam(t_pppParam::CRD_Y,  ++nextPar, ""));
  _params.push_back(new t_pppParam(t_pppParam::CRD_Z,  ++nextPar, ""));
  _params.push_back(new t_pppParam(t_pppParam::RECCLK, ++nextPar, ""));
  if (OPT->estTrp()) {
    _params.push_back(new t_pppParam(t_pppParam::TROPO, ++nextPar, ""));
  }
  if (OPT->useSystem('R')) {
    _params.push_back(new t_pppParam(t_pppParam::GLONASS_OFFSET, ++nextPar, ""));
  }
  if (OPT->useSystem('E')) {
    _params.push_back(new t_pppParam(t_pppParam::GALILEO_OFFSET, ++nextPar, ""));
  }
  if (OPT->useSystem('C')) {
    _params.push_back(new t_pppParam(t_pppParam::BDS_OFFSET, ++nextPar, ""));
  }

  _QQ.ReSize(_params.size());
  _QQ = 0.0;
  for (int iPar = 1; iPar <= _params.size(); iPar++) {
    t_pppParam* pp = _params[iPar-1];
    pp->xx = 0.0;
    if      (pp->isCrd()) {
      _QQ(iPar,iPar) = OPT->_aprSigCrd(1) * OPT->_aprSigCrd(1);
    }
    else if (pp->type == t_pppParam::RECCLK) {
      _QQ(iPar,iPar) = OPT->_noiseClk * OPT->_noiseClk;
    }
    else if (pp->type == t_pppParam::TROPO) {
      _QQ(iPar,iPar) = OPT->_aprSigTrp * OPT->_aprSigTrp;
      pp->xx = lastTrp;
    }
    else if (pp->type == t_pppParam::GLONASS_OFFSET) {
      _QQ(iPar,iPar) = 1000.0 * 1000.0;
    }
    else if (pp->type == t_pppParam::GALILEO_OFFSET) {
      _QQ(iPar,iPar) = 1000.0 * 1000.0;
    }
    else if (pp->type == t_pppParam::BDS_OFFSET) {
      _QQ(iPar,iPar) = 1000.0 * 1000.0;
    }
  }
}

// Bancroft Solution
////////////////////////////////////////////////////////////////////////////
t_irc t_pppFilter::cmpBancroft(t_epoData* epoData) {

  Tracer tracer("t_pppFilter::cmpBancroft");

  if (int(epoData->sizeSys('G')) < OPT->_minObs) {
    LOG << "t_pppFilter::cmpBancroft: not enough data\n";
    return failure;
  }

  Matrix BB(epoData->sizeSys('G'), 4);

  QMapIterator<QString, t_satData*> it(epoData->satData);
  int iObsBanc = 0;
  while (it.hasNext()) {
    it.next();
    t_satData* satData = it.value();
    if (satData->system() == 'G') {
      ++iObsBanc;
      QString    prn     = it.key();
      BB(iObsBanc, 1) = satData->xx(1);
      BB(iObsBanc, 2) = satData->xx(2);
      BB(iObsBanc, 3) = satData->xx(3);
      BB(iObsBanc, 4) = satData->P3 + satData->clk;
    }
  }

  bancroft(BB, _xcBanc);

  if (std::isnan(_xcBanc(1)) ||
      std::isnan(_xcBanc(2)) ||
      std::isnan(_xcBanc(3))) {
    return failure;
  }

  // Ellipsoidal Coordinates
  // ------------------------
  xyz2ell(_xcBanc.data(), _ellBanc.data());

  // Compute Satellite Elevations
  // ----------------------------
  QMutableMapIterator<QString, t_satData*> im(epoData->satData);
  while (im.hasNext()) {
    im.next();
    t_satData* satData = im.value();
    cmpEle(satData);
    if (satData->eleSat < OPT->_minEle) {
      delete satData;
      im.remove();
    }
  }

  return success;
}

// Computed Value
////////////////////////////////////////////////////////////////////////////
double t_pppFilter::cmpValue(t_satData* satData, bool phase) {

  Tracer tracer("t_pppFilter::cmpValue");

  ColumnVector xRec(3);
  xRec(1) = x();
  xRec(2) = y();
  xRec(3) = z();

  double rho0 = (satData->xx - xRec).NormFrobenius();
  double dPhi = t_CST::omega * rho0 / t_CST::c;

  xRec(1) = x() * cos(dPhi) - y() * sin(dPhi);
  xRec(2) = y() * cos(dPhi) + x() * sin(dPhi);
  xRec(3) = z();

  xRec += _tides->displacement(_time, xRec);

  satData->rho = (satData->xx - xRec).NormFrobenius();

  double tropDelay = delay_saast(satData->eleSat) +
                     trp() / sin(satData->eleSat);

  double wind = 0.0;
  if (phase) {
    wind = windUp(satData->prn, satData->xx, xRec) * satData->lambda3;
  }

  double offset = 0.0;
  t_frequency::type frqA = t_frequency::G1;
  t_frequency::type frqB = t_frequency::G2;
  if      (satData->prn[0] == 'R') {
    offset = Glonass_offset();
    frqA = t_frequency::R1;
    frqB = t_frequency::R2;
  }
  else if (satData->prn[0] == 'E') {
    offset = Galileo_offset();
    //frqA = t_frequency::E1; as soon as available
    //frqB = t_frequency::E5; -"-
  }
  else if (satData->prn[0] == 'C') {
    offset = Bds_offset();
    //frqA = t_frequency::C2; as soon as available
    //frqB = t_frequency::C7; -"-
  }
  double phaseCenter = 0.0;
  if (_antex) {
    bool found;
    phaseCenter = satData->lkA * _antex->rcvCorr(OPT->_antNameRover, frqA,
                                                 satData->eleSat, satData->azSat,
                                                 found)
                + satData->lkB * _antex->rcvCorr(OPT->_antNameRover, frqB,
                                                 satData->eleSat, satData->azSat,
                                                 found);
    if (!found) {
      LOG << "ANTEX: antenna >" << OPT->_antNameRover << "< not found\n";
    }
  }

  double antennaOffset = 0.0;
  double cosa = cos(satData->azSat);
  double sina = sin(satData->azSat);
  double cose = cos(satData->eleSat);
  double sine = sin(satData->eleSat);
  antennaOffset = -OPT->_neuEccRover(1) * cosa*cose
                  -OPT->_neuEccRover(2) * sina*cose
                  -OPT->_neuEccRover(3) * sine;

  return satData->rho + phaseCenter + antennaOffset + clk()
                      + offset - satData->clk + tropDelay + wind;
}

// Tropospheric Model (Saastamoinen)
////////////////////////////////////////////////////////////////////////////
double t_pppFilter::delay_saast(double Ele) {

  Tracer tracer("t_pppFilter::delay_saast");

  double xyz[3];
  xyz[0] = x();
  xyz[1] = y();
  xyz[2] = z();
  double ell[3];
  xyz2ell(xyz, ell);
  double height = ell[2];

  double pp =  1013.25 * pow(1.0 - 2.26e-5 * height, 5.225);
  double TT =  18.0 - height * 0.0065 + 273.15;
  double hh =  50.0 * exp(-6.396e-4 * height);
  double ee =  hh / 100.0 * exp(-37.2465 + 0.213166*TT - 0.000256908*TT*TT);

  double h_km = height / 1000.0;

  if (h_km < 0.0) h_km = 0.0;
  if (h_km > 5.0) h_km = 5.0;
  int    ii   = int(h_km + 1);
  double href = ii - 1;

  double bCor[6];
  bCor[0] = 1.156;
  bCor[1] = 1.006;
  bCor[2] = 0.874;
  bCor[3] = 0.757;
  bCor[4] = 0.654;
  bCor[5] = 0.563;

  double BB = bCor[ii-1] + (bCor[ii]-bCor[ii-1]) * (h_km - href);

  double zen  = M_PI/2.0 - Ele;

  return (0.002277/cos(zen)) * (pp + ((1255.0/TT)+0.05)*ee - BB*(tan(zen)*tan(zen)));
}

// Prediction Step of the Filter
////////////////////////////////////////////////////////////////////////////
void t_pppFilter::predict(int iPhase, t_epoData* epoData) {

  Tracer tracer("t_pppFilter::predict");

  if (iPhase == 0) {

    const double maxSolGap = 60.0;

    bool firstCrd = false;
    if (!_lastTimeOK.valid() || (maxSolGap > 0.0 && _time - _lastTimeOK > maxSolGap)) {
      firstCrd = true;
      _startTime = epoData->tt;
      reset();
    }

    // Use different white noise for Quick-Start mode
    // ----------------------------------------------
    double sigCrdP_used = OPT->_noiseCrd(1);
    if ( OPT->_seedingTime > 0.0 && OPT->_seedingTime > (epoData->tt - _startTime) ) {
      sigCrdP_used   = 0.0;
    }

    // Predict Parameter values, add white noise
    // -----------------------------------------
    for (int iPar = 1; iPar <= _params.size(); iPar++) {
      t_pppParam* pp = _params[iPar-1];

      // Coordinates
      // -----------
      if      (pp->type == t_pppParam::CRD_X) {
        if (firstCrd) {
          if (OPT->xyzAprRoverSet()) {
            pp->xx = OPT->_xyzAprRover[0];
          }
          else {
            pp->xx = _xcBanc(1);
          }
        }
        _QQ(iPar,iPar) += sigCrdP_used * sigCrdP_used;
      }
      else if (pp->type == t_pppParam::CRD_Y) {
        if (firstCrd) {
          if (OPT->xyzAprRoverSet()) {
            pp->xx = OPT->_xyzAprRover[1];
          }
          else {
            pp->xx = _xcBanc(2);
          }
        }
        _QQ(iPar,iPar) += sigCrdP_used * sigCrdP_used;
      }
      else if (pp->type == t_pppParam::CRD_Z) {
        if (firstCrd) {
          if (OPT->xyzAprRoverSet()) {
            pp->xx = OPT->_xyzAprRover[2];
          }
          else {
            pp->xx = _xcBanc(3);
          }
        }
        _QQ(iPar,iPar) += sigCrdP_used * sigCrdP_used;
      }

      // Receiver Clocks
      // ---------------
      else if (pp->type == t_pppParam::RECCLK) {
        pp->xx = _xcBanc(4);
        for (int jj = 1; jj <= _params.size(); jj++) {
          _QQ(iPar, jj) = 0.0;
        }
        _QQ(iPar,iPar) = OPT->_noiseClk * OPT->_noiseClk;
      }

      // Tropospheric Delay
      // ------------------
      else if (pp->type == t_pppParam::TROPO) {
        _QQ(iPar,iPar) += OPT->_noiseTrp * OPT->_noiseTrp;
      }

      // Glonass Offset
      // --------------
      else if (pp->type == t_pppParam::GLONASS_OFFSET) {
        pp->xx = 0.0;
        for (int jj = 1; jj <= _params.size(); jj++) {
          _QQ(iPar, jj) = 0.0;
        }
        _QQ(iPar,iPar) = 1000.0 * 1000.0;
      }

      // Galileo Offset
      // --------------
      else if (pp->type == t_pppParam::GALILEO_OFFSET) {
        _QQ(iPar,iPar) += 0.1 * 0.1;
      }

      // BDS Offset
      // ----------
      else if (pp->type == t_pppParam::BDS_OFFSET) {
        _QQ(iPar,iPar) += 0.1 * 0.1;    //TODO: TEST
      }
    }
  }

  // Add New Ambiguities if necessary
  // --------------------------------
  if (OPT->ambLCs('G').size() || OPT->ambLCs('R').size() ||
      OPT->ambLCs('E').size() || OPT->ambLCs('C').size()) {

    // Make a copy of QQ and xx, set parameter indices
    // -----------------------------------------------
    SymmetricMatrix QQ_old = _QQ;

    for (int iPar = 1; iPar <= _params.size(); iPar++) {
      _params[iPar-1]->index_old = _params[iPar-1]->index;
      _params[iPar-1]->index     = 0;
    }

    // Remove Ambiguity Parameters without observations
    // ------------------------------------------------
    int iPar = 0;
    QMutableVectorIterator<t_pppParam*> im(_params);
    while (im.hasNext()) {
      t_pppParam* par = im.next();
      bool removed = false;
      if (par->type == t_pppParam::AMB_L3) {
        if (epoData->satData.find(par->prn) == epoData->satData.end()) {
          removed = true;
          delete par;
          im.remove();
        }
      }
      if (! removed) {
        ++iPar;
        par->index = iPar;
      }
    }

    // Add new ambiguity parameters
    // ----------------------------
    QMapIterator<QString, t_satData*> it(epoData->satData);
    while (it.hasNext()) {
      it.next();
      t_satData* satData = it.value();
      addAmb(satData);
    }

    int nPar = _params.size();
    _QQ.ReSize(nPar); _QQ = 0.0;
    for (int i1 = 1; i1 <= nPar; i1++) {
      t_pppParam* p1 = _params[i1-1];
      if (p1->index_old != 0) {
        _QQ(p1->index, p1->index) = QQ_old(p1->index_old, p1->index_old);
        for (int i2 = 1; i2 <= nPar; i2++) {
          t_pppParam* p2 = _params[i2-1];
          if (p2->index_old != 0) {
            _QQ(p1->index, p2->index) = QQ_old(p1->index_old, p2->index_old);
          }
        }
      }
    }

    for (int ii = 1; ii <= nPar; ii++) {
      t_pppParam* par = _params[ii-1];
      if (par->index_old == 0) {
        _QQ(par->index, par->index) = OPT->_aprSigAmb * OPT->_aprSigAmb;
      }
      par->index_old = par->index;
    }
  }
}

// Update Step of the Filter (currently just a single-epoch solution)
////////////////////////////////////////////////////////////////////////////
t_irc t_pppFilter::update(t_epoData* epoData) {

  Tracer tracer("t_pppFilter::update");

  _time = epoData->tt; // current epoch time

  if (OPT->useOrbClkCorr()) {
    LOG << "Precise Point Positioning of Epoch " << _time.datestr() <<  "_" << _time.timestr(3)
        << "\n---------------------------------------------------------------\n";
  }
  else {
    LOG << "Single Point Positioning of Epoch " << _time.datestr() <<  "_" << _time.timestr(3)
        << "\n---------------------------------------------------------------\n";
  }

  // Outlier Detection Loop
  // ----------------------
  if (update_p(epoData) != success) {
    return failure;
  }

  // Set Solution Vector
  // -------------------
  LOG.setf(ios::fixed);
  QVectorIterator<t_pppParam*> itPar(_params);
  while (itPar.hasNext()) {
    t_pppParam* par = itPar.next();
    if      (par->type == t_pppParam::RECCLK) {
      LOG << "\n" << _time.datestr() << "_" << _time.timestr(3)
          << " CLK     " << setw(10) << setprecision(3) << par->xx
          << " +- " << setw(6) << setprecision(3)
          << sqrt(_QQ(par->index,par->index));
    }
    else if (par->type == t_pppParam::AMB_L3) {
      ++par->numEpo;
      LOG << "\n" << _time.datestr() << "_" << _time.timestr(3)
          << " AMB " << par->prn.mid(0,3).toLatin1().data() << " "
          << setw(10) << setprecision(3) << par->xx
          << " +- " << setw(6) << setprecision(3)
          << sqrt(_QQ(par->index,par->index))
          << "   epo = " << par->numEpo;
    }
    else if (par->type == t_pppParam::TROPO) {
      double aprTrp = delay_saast(M_PI/2.0);
      LOG << "\n" << _time.datestr() << "_" << _time.timestr(3)
          << " TRP     " << par->prn.mid(0,3).toLatin1().data()
          << setw(7) << setprecision(3) << aprTrp << " "
          << setw(6) << setprecision(3) << showpos << par->xx << noshowpos
          << " +- " << setw(6) << setprecision(3)
          << sqrt(_QQ(par->index,par->index));
    }
    else if (par->type == t_pppParam::GLONASS_OFFSET) {
      LOG << "\n" << _time.datestr() << "_" << _time.timestr(3)
          << " OFFGLO  " << setw(10) << setprecision(3) << par->xx
          << " +- " << setw(6) << setprecision(3)
          << sqrt(_QQ(par->index,par->index));
    }
    else if (par->type == t_pppParam::GALILEO_OFFSET) {
      LOG << "\n" << _time.datestr() << "_" << _time.timestr(3)
          << " OFFGAL  " << setw(10) << setprecision(3) << par->xx
          << " +- " << setw(6) << setprecision(3)
          << sqrt(_QQ(par->index,par->index));
    }
    else if (par->type == t_pppParam::BDS_OFFSET) {
      LOG << "\n" << _time.datestr() << "_" << _time.timestr(3)
          << " OFFBDS  " << setw(10) << setprecision(3) << par->xx
          << " +- " << setw(6) << setprecision(3)
          << sqrt(_QQ(par->index,par->index));
    }
  }

  LOG << endl << endl;

  // Compute dilution of precision
  // -----------------------------
  cmpDOP(epoData);

  // Final Message (both log file and screen)
  // ----------------------------------------
  LOG << epoData->tt.datestr() << "_" << epoData->tt.timestr(3)
      << " " << OPT->_roverName
      << " X = "
      << setprecision(4) << x() << " +- "
      << setprecision(4) << sqrt(_QQ(1,1))

      << " Y = "
      << setprecision(4) << y() << " +- "
      << setprecision(4) << sqrt(_QQ(2,2))

      << " Z = "
      << setprecision(4) << z() << " +- "
      << setprecision(4) << sqrt(_QQ(3,3));

  // NEU Output
  // ----------
  if (OPT->xyzAprRoverSet()) {
    SymmetricMatrix QQxyz = _QQ.SymSubMatrix(1,3);

    ColumnVector xyz(3);
    xyz(1) = x() - OPT->_xyzAprRover[0];
    xyz(2) = y() - OPT->_xyzAprRover[1];
    xyz(3) = z() - OPT->_xyzAprRover[2];

    ColumnVector ellRef(3);
    xyz2ell(OPT->_xyzAprRover.data(), ellRef.data());
    xyz2neu(ellRef.data(), xyz.data(), _neu.data());

    SymmetricMatrix QQneu(3);
    covariXYZ_NEU(QQxyz, ellRef.data(), QQneu);

    LOG << " dN = "
        << setprecision(4) << _neu[0] << " +- "
        << setprecision(4) << sqrt(QQneu[0][0])

        << " dE = "
        << setprecision(4) << _neu[1] << " +- "
        << setprecision(4) << sqrt(QQneu[1][1])

        << " dU = "
        << setprecision(4) << _neu[2] << " +- "
        << setprecision(4) << sqrt(QQneu[2][2])           << endl << endl;
  }
  else {
    LOG << endl << endl;
  }

  _lastTimeOK = _time; // remember time of last successful update
  return success;
}

// Outlier Detection
////////////////////////////////////////////////////////////////////////////
QString t_pppFilter::outlierDetection(int iPhase, const ColumnVector& vv,
                                   QMap<QString, t_satData*>& satData) {

  Tracer tracer("t_pppFilter::outlierDetection");

  QString prnGPS;
  QString prnGlo;
  double  maxResGPS = 0.0; // GPS + Galileo
  double  maxResGlo = 0.0; // GLONASS + BDS
  findMaxRes(vv, satData, prnGPS, prnGlo, maxResGPS, maxResGlo);

  if      (iPhase == 1) {
    if      (maxResGlo > 2.98 * OPT->_maxResL1) {
      LOG << "Outlier Phase " << prnGlo.mid(0,3).toLatin1().data() << ' ' << maxResGlo << endl;
      return prnGlo;
    }
    else if (maxResGPS > MAXRES_PHASE_GPS) {
      LOG << "Outlier Phase " << prnGPS.mid(0,3).toLatin1().data() << ' ' << maxResGPS << endl;
      return prnGPS;
    }
  }
  else if (iPhase == 0 && maxResGPS > 2.98 * OPT->_maxResC1) {
    LOG << "Outlier Code  " << prnGPS.mid(0,3).toLatin1().data() << ' ' << maxResGPS << endl;
    return prnGPS;
  }

  return QString();
}

// Phase Wind-Up Correction
///////////////////////////////////////////////////////////////////////////
double t_pppFilter::windUp(const QString& prn, const ColumnVector& rSat,
                        const ColumnVector& rRec) {

  Tracer tracer("t_pppFilter::windUp");

  double Mjd = _time.mjd() + _time.daysec() / 86400.0;

  // First time - initialize to zero
  // -------------------------------
  if (!_windUpTime.contains(prn)) {
    _windUpSum[prn]  = 0.0;
  }

  // Compute the correction for new time
  // -----------------------------------
  if (!_windUpTime.contains(prn) || _windUpTime[prn] != Mjd) {
    _windUpTime[prn] = Mjd;

    // Unit Vector GPS Satellite --> Receiver
    // --------------------------------------
    ColumnVector rho = rRec - rSat;
    rho /= rho.NormFrobenius();

    // GPS Satellite unit Vectors sz, sy, sx
    // -------------------------------------
    ColumnVector sz = -rSat / rSat.NormFrobenius();

    ColumnVector xSun = t_astro::Sun(Mjd);
    xSun /= xSun.NormFrobenius();

    ColumnVector sy = crossproduct(sz, xSun);
    ColumnVector sx = crossproduct(sy, sz);

    // Effective Dipole of the GPS Satellite Antenna
    // ---------------------------------------------
    ColumnVector dipSat = sx - rho * DotProduct(rho,sx)
                                                - crossproduct(rho, sy);

    // Receiver unit Vectors rx, ry
    // ----------------------------
    ColumnVector rx(3);
    ColumnVector ry(3);

    double recEll[3]; xyz2ell(rRec.data(), recEll) ;
    double neu[3];

    neu[0] = 1.0;
    neu[1] = 0.0;
    neu[2] = 0.0;
    neu2xyz(recEll, neu, rx.data());

    neu[0] =  0.0;
    neu[1] = -1.0;
    neu[2] =  0.0;
    neu2xyz(recEll, neu, ry.data());

    // Effective Dipole of the Receiver Antenna
    // ----------------------------------------
    ColumnVector dipRec = rx - rho * DotProduct(rho,rx)
                                                   + crossproduct(rho, ry);

    // Resulting Effect
    // ----------------
    double alpha = DotProduct(dipSat,dipRec) /
                      (dipSat.NormFrobenius() * dipRec.NormFrobenius());

    if (alpha >  1.0) alpha =  1.0;
    if (alpha < -1.0) alpha = -1.0;

    double dphi = acos(alpha) / 2.0 / M_PI;  // in cycles

    if ( DotProduct(rho, crossproduct(dipSat, dipRec)) < 0.0 ) {
      dphi = -dphi;
    }

    _windUpSum[prn] = floor(_windUpSum[prn] - dphi + 0.5) + dphi;
  }

  return _windUpSum[prn];
}

//
///////////////////////////////////////////////////////////////////////////
void t_pppFilter::cmpEle(t_satData* satData) {
  Tracer tracer("t_pppFilter::cmpEle");
  ColumnVector rr = satData->xx - _xcBanc.Rows(1,3);
  double       rho = rr.NormFrobenius();

  double neu[3];
  xyz2neu(_ellBanc.data(), rr.data(), neu);

  satData->eleSat = acos( sqrt(neu[0]*neu[0] + neu[1]*neu[1]) / rho );
  if (neu[2] < 0) {
    satData->eleSat *= -1.0;
  }
  satData->azSat  = atan2(neu[1], neu[0]);
}

//
///////////////////////////////////////////////////////////////////////////
void t_pppFilter::addAmb(t_satData* satData) {
  Tracer tracer("t_pppFilter::addAmb");
  if (!OPT->ambLCs(satData->system()).size()){
    return;
  }
  bool    found = false;
  for (int iPar = 1; iPar <= _params.size(); iPar++) {
    if (_params[iPar-1]->type == t_pppParam::AMB_L3 &&
        _params[iPar-1]->prn == satData->prn) {
      found = true;
      break;
    }
  }
  if (!found) {
    t_pppParam* par = new t_pppParam(t_pppParam::AMB_L3,
                                 _params.size()+1, satData->prn);
    _params.push_back(par);
    par->xx = satData->L3 - cmpValue(satData, true);
  }
}

//
///////////////////////////////////////////////////////////////////////////
void t_pppFilter::addObs(int iPhase, unsigned& iObs, t_satData* satData,
                      Matrix& AA, ColumnVector& ll, DiagonalMatrix& PP) {

  Tracer tracer("t_pppFilter::addObs");

  const double ELEWGHT = 20.0;
  double ellWgtCoef = 1.0;
  double eleD = satData->eleSat * 180.0 / M_PI;
  if (eleD < ELEWGHT) {
    ellWgtCoef = 1.5 - 0.5 / (ELEWGHT - 10.0) * (eleD - 10.0);
  }

  // Remember Observation Index
  // --------------------------
  ++iObs;
  satData->obsIndex = iObs;

  // Phase Observations
  // ------------------

  if (iPhase == 1) {
    ll(iObs)      = satData->L3 - cmpValue(satData, true);
    double sigL3 = 2.98 * OPT->_sigmaL1;
    if (satData->system() == 'R') {
      sigL3 *= GLONASS_WEIGHT_FACTOR;
    }
    if  (satData->system() == 'C') {
      sigL3 *= BDS_WEIGHT_FACTOR;
    }
    PP(iObs,iObs) = 1.0 / (sigL3 * sigL3) / (ellWgtCoef * ellWgtCoef);
    for (int iPar = 1; iPar <= _params.size(); iPar++) {
      if (_params[iPar-1]->type == t_pppParam::AMB_L3 &&
          _params[iPar-1]->prn  == satData->prn) {
        ll(iObs) -= _params[iPar-1]->xx;
      }
      AA(iObs, iPar) = _params[iPar-1]->partial(satData, true);
    }
  }

  // Code Observations
  // -----------------
  else {
    double sigP3 = 2.98 * OPT->_sigmaC1;
    ll(iObs)      = satData->P3 - cmpValue(satData, false);
    PP(iObs,iObs) = 1.0 / (sigP3 * sigP3) / (ellWgtCoef * ellWgtCoef);
    for (int iPar = 1; iPar <= _params.size(); iPar++) {
      AA(iObs, iPar) = _params[iPar-1]->partial(satData, false);
    }
  }
}

//
///////////////////////////////////////////////////////////////////////////
QByteArray t_pppFilter::printRes(int iPhase, const ColumnVector& vv,
                              const QMap<QString, t_satData*>& satDataMap) {

  Tracer tracer("t_pppFilter::printRes");

  ostringstream str;
  str.setf(ios::fixed);
  bool useObs;
  QMapIterator<QString, t_satData*> it(satDataMap);
  while (it.hasNext()) {
    it.next();
    t_satData* satData = it.value();
    (iPhase == 0) ? useObs = OPT->codeLCs(satData->system()).size() :
                    useObs = OPT->ambLCs(satData->system()).size();
    if (satData->obsIndex != 0 && useObs) {
      str << _time.datestr() << "_" << _time.timestr(3)
          << " RES " << satData->prn.mid(0,3).toLatin1().data()
          << (iPhase ? "   L3 " : "   P3 ")
          << setw(9) << setprecision(4) << vv(satData->obsIndex) << endl;
    }
  }

  return QByteArray(str.str().c_str());
}

//
///////////////////////////////////////////////////////////////////////////
void t_pppFilter::findMaxRes(const ColumnVector& vv,
                          const QMap<QString, t_satData*>& satData,
                          QString& prnGPS, QString& prnGlo,
                          double& maxResGPS, double& maxResGlo) {

  Tracer tracer("t_pppFilter::findMaxRes");

  maxResGPS  = 0.0;
  maxResGlo  = 0.0;

  QMapIterator<QString, t_satData*> it(satData);
  while (it.hasNext()) {
    it.next();
    t_satData* satData = it.value();
    if (satData->obsIndex != 0) {
      QString prn = satData->prn;
      if (prn[0] == 'R' || prn[0] == 'C') {
        if (fabs(vv(satData->obsIndex)) > maxResGlo) {
          maxResGlo = fabs(vv(satData->obsIndex));
          prnGlo    = prn;
        }
      }
      else {
        if (fabs(vv(satData->obsIndex)) > maxResGPS) {
          maxResGPS = fabs(vv(satData->obsIndex));
          prnGPS    = prn;
        }
      }
    }
  }
}

// Update Step (private - loop over outliers)
////////////////////////////////////////////////////////////////////////////
t_irc t_pppFilter::update_p(t_epoData* epoData) {

  Tracer tracer("t_pppFilter::update_p");

  // Save Variance-Covariance Matrix, and Status Vector
  // --------------------------------------------------
  rememberState(epoData);

  QString lastOutlierPrn;

  // Try with all satellites, then with all minus one, etc.
  // ------------------------------------------------------
  while (selectSatellites(lastOutlierPrn, epoData->satData) == success) {

    QByteArray strResCode;
    QByteArray strResPhase;

    // Bancroft Solution
    // -----------------
    if (cmpBancroft(epoData) != success) {
      break;
    }

    // First update using code observations, then phase observations
    // -------------------------------------------------------------
    bool usePhase = OPT->ambLCs('G').size() || OPT->ambLCs('R').size() ||
                    OPT->ambLCs('E').size() || OPT->ambLCs('C').size() ;

    char sys[] ={'G', 'R', 'E', 'C'};

    bool satnumPrinted[] = {false, false, false, false};

    for (int iPhase = 0; iPhase <= (usePhase ? 1 : 0); iPhase++) {

      // Status Prediction
      // -----------------
      predict(iPhase, epoData);

      // Create First-Design Matrix
      // --------------------------
      unsigned nPar = _params.size();
      unsigned nObs = 0;
      nObs = epoData->sizeAll();
      bool useObs = false;
      for (unsigned ii = 0; ii < sizeof(sys); ii++) {
        const char s = sys[ii];
        (iPhase == 0) ? useObs = OPT->codeLCs(s).size() : useObs = OPT->ambLCs(s).size();
        if (!useObs) {
          nObs -= epoData->sizeSys(s);
        }
        else {
          if (!satnumPrinted[ii]) {
            satnumPrinted[ii] = true;
            LOG << _time.datestr() << "_" << _time.timestr(3)
                << " SATNUM " << s << ' ' << right << setw(2)
                << epoData->sizeSys(s) << endl;
          }
        }
      }

      if (int(nObs) < OPT->_minObs) {
        restoreState(epoData);
        return failure;
      }

      // Prepare first-design Matrix, vector observed-computed
      // -----------------------------------------------------
      Matrix          AA(nObs, nPar);  // first design matrix
      ColumnVector    ll(nObs);        // terms observed-computed
      DiagonalMatrix  PP(nObs); PP = 0.0;

      unsigned iObs = 0;
      QMapIterator<QString, t_satData*> it(epoData->satData);

      while (it.hasNext()) {
        it.next();
        t_satData* satData = it.value();
        QString prn = satData->prn;
        (iPhase == 0) ? useObs = OPT->codeLCs(satData->system()).size() :
                        useObs = OPT->ambLCs(satData->system()).size();
        if (useObs) {
          addObs(iPhase, iObs, satData, AA, ll, PP);
        } else {
          satData->obsIndex = 0;
        }
      }

      // Compute Filter Update
      // ---------------------
      ColumnVector dx(nPar); dx = 0.0;
      kalman(AA, ll, PP, _QQ, dx);
      ColumnVector vv = ll - AA * dx;

      // Print Residuals
      // ---------------
      if (iPhase == 0) {
        strResCode  = printRes(iPhase, vv, epoData->satData);
      }
      else {
        strResPhase = printRes(iPhase, vv, epoData->satData);
      }

      // Check the residuals
      // -------------------
      lastOutlierPrn = outlierDetection(iPhase, vv, epoData->satData);

      // No Outlier Detected
      // -------------------
      if (lastOutlierPrn.isEmpty()) {

        QVectorIterator<t_pppParam*> itPar(_params);
        while (itPar.hasNext()) {
          t_pppParam* par = itPar.next();
          par->xx += dx(par->index);
        }

        if (!usePhase || iPhase == 1) {
          if (_outlierGPS.size() > 0 || _outlierGlo.size() > 0) {
            LOG << "Neglected PRNs: ";
            if (!_outlierGPS.isEmpty()) {
              LOG << _outlierGPS.last().mid(0,3).toLatin1().data() << ' ';
            }
            QStringListIterator itGlo(_outlierGlo);
            while (itGlo.hasNext()) {
              QString prn = itGlo.next();
              LOG << prn.mid(0,3).toLatin1().data() << ' ';
            }
            LOG << endl;
          }
          LOG << strResCode.data() << strResPhase.data();

          return success;
        }
      }

      // Outlier Found
      // -------------
      else {
        restoreState(epoData);
        break;
      }

    } // for iPhase

  } // while selectSatellites

  restoreState(epoData);
  return failure;
}

// Remeber Original State Vector and Variance-Covariance Matrix
////////////////////////////////////////////////////////////////////////////
void t_pppFilter::rememberState(t_epoData* epoData) {

  _QQ_sav = _QQ;

  QVectorIterator<t_pppParam*> itSav(_params_sav);
  while (itSav.hasNext()) {
    t_pppParam* par = itSav.next();
    delete par;
  }
  _params_sav.clear();

  QVectorIterator<t_pppParam*> it(_params);
  while (it.hasNext()) {
    t_pppParam* par = it.next();
    _params_sav.push_back(new t_pppParam(*par));
  }

  _epoData_sav->deepCopy(epoData);
}

// Restore Original State Vector and Variance-Covariance Matrix
////////////////////////////////////////////////////////////////////////////
void t_pppFilter::restoreState(t_epoData* epoData) {

  _QQ = _QQ_sav;

  QVectorIterator<t_pppParam*> it(_params);
  while (it.hasNext()) {
    t_pppParam* par = it.next();
    delete par;
  }
  _params.clear();

  QVectorIterator<t_pppParam*> itSav(_params_sav);
  while (itSav.hasNext()) {
    t_pppParam* par = itSav.next();
    _params.push_back(new t_pppParam(*par));
  }

  epoData->deepCopy(_epoData_sav);
}

//
////////////////////////////////////////////////////////////////////////////
t_irc t_pppFilter::selectSatellites(const QString& lastOutlierPrn,
                                 QMap<QString, t_satData*>& satData) {

  // First Call
  // ----------
  if (lastOutlierPrn.isEmpty()) {
    _outlierGPS.clear();
    _outlierGlo.clear();
    return success;
  }

  // Second and next trials
  // ----------------------
  else {

    if (lastOutlierPrn[0] == 'R' || lastOutlierPrn[0] == 'C') {
      _outlierGlo << lastOutlierPrn;
    }

    // Remove all Glonass Outliers
    // ---------------------------
    QStringListIterator it(_outlierGlo);
    while (it.hasNext()) {
      QString prn = it.next();
      if (satData.contains(prn)) {
        delete satData.take(prn);
      }
    }

    if (lastOutlierPrn[0] == 'R' || lastOutlierPrn[0] == 'C') {
      _outlierGPS.clear();
      return success;
    }

    // GPS Outlier appeared for the first time - try to delete it
    // ----------------------------------------------------------
    if (_outlierGPS.indexOf(lastOutlierPrn) == -1) {
      _outlierGPS << lastOutlierPrn;
      if (satData.contains(lastOutlierPrn)) {
        delete satData.take(lastOutlierPrn);
      }
      return success;
    }

  }

  return failure;
}

//
////////////////////////////////////////////////////////////////////////////
double lorentz(const ColumnVector& aa, const ColumnVector& bb) {
  return aa(1)*bb(1) +  aa(2)*bb(2) +  aa(3)*bb(3) -  aa(4)*bb(4);
}

//
////////////////////////////////////////////////////////////////////////////
void t_pppFilter::bancroft(const Matrix& BBpass, ColumnVector& pos) {

  if (pos.Nrows() != 4) {
    pos.ReSize(4);
  }
  pos = 0.0;

  for (int iter = 1; iter <= 2; iter++) {
    Matrix BB = BBpass;
    int mm = BB.Nrows();
    for (int ii = 1; ii <= mm; ii++) {
      double xx = BB(ii,1);
      double yy = BB(ii,2);
      double traveltime = 0.072;
      if (iter > 1) {
        double zz  = BB(ii,3);
        double rho = sqrt( (xx-pos(1)) * (xx-pos(1)) +
                           (yy-pos(2)) * (yy-pos(2)) +
                           (zz-pos(3)) * (zz-pos(3)) );
        traveltime = rho / t_CST::c;
      }
      double angle = traveltime * t_CST::omega;
      double cosa  = cos(angle);
      double sina  = sin(angle);
      BB(ii,1) =  cosa * xx + sina * yy;
      BB(ii,2) = -sina * xx + cosa * yy;
    }

    Matrix BBB;
    if (mm > 4) {
      SymmetricMatrix hlp; hlp << BB.t() * BB;
      BBB = hlp.i() * BB.t();
    }
    else {
      BBB = BB.i();
    }
    ColumnVector ee(mm); ee = 1.0;
    ColumnVector alpha(mm); alpha = 0.0;
    for (int ii = 1; ii <= mm; ii++) {
      alpha(ii) = lorentz(BB.Row(ii).t(),BB.Row(ii).t())/2.0;
    }
    ColumnVector BBBe     = BBB * ee;
    ColumnVector BBBalpha = BBB * alpha;
    double aa = lorentz(BBBe, BBBe);
    double bb = lorentz(BBBe, BBBalpha)-1;
    double cc = lorentz(BBBalpha, BBBalpha);
    double root = sqrt(bb*bb-aa*cc);

    Matrix hlpPos(4,2);
    hlpPos.Column(1) = (-bb-root)/aa * BBBe + BBBalpha;
    hlpPos.Column(2) = (-bb+root)/aa * BBBe + BBBalpha;

    ColumnVector omc(2);
    for (int pp = 1; pp <= 2; pp++) {
      hlpPos(4,pp)      = -hlpPos(4,pp);
      omc(pp) = BB(1,4) -
                sqrt( (BB(1,1)-hlpPos(1,pp)) * (BB(1,1)-hlpPos(1,pp)) +
                      (BB(1,2)-hlpPos(2,pp)) * (BB(1,2)-hlpPos(2,pp)) +
                      (BB(1,3)-hlpPos(3,pp)) * (BB(1,3)-hlpPos(3,pp)) ) -
                hlpPos(4,pp);
    }
    if ( fabs(omc(1)) > fabs(omc(2)) ) {
      pos = hlpPos.Column(2);
    }
    else {
      pos = hlpPos.Column(1);
    }
  }
}

//
////////////////////////////////////////////////////////////////////////////
void t_pppFilter::cmpDOP(t_epoData* epoData) {

  Tracer tracer("t_pppFilter::cmpDOP");

  _numSat = 0;
  _hDop   = 0.0;

  if (_params.size() < 4) {
    return;
  }

  const unsigned numPar = 4;
  Matrix AA(epoData->sizeAll(), numPar);
  QMapIterator<QString, t_satData*> it(epoData->satData);
  while (it.hasNext()) {
    it.next();
    t_satData* satData = it.value();
    _numSat += 1;
    for (unsigned iPar = 0; iPar < numPar; iPar++) {
      AA[_numSat-1][iPar] = _params[iPar]->partial(satData, false);
    }
  }
  if (_numSat < 4) {
    return;
  }
  AA = AA.Rows(1, _numSat);
  SymmetricMatrix NN; NN << AA.t() * AA;
  SymmetricMatrix QQ = NN.i();

  _hDop = sqrt(QQ(1,1) + QQ(2,2));
}
