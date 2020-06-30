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
 * Class:      t_pppClient
 *
 * Purpose:    Precise Point Positioning
 *
 * Author:     L. Mervart
 *
 * Created:    21-Nov-2009
 *
 * Changes:
 *
 * -----------------------------------------------------------------------*/

#include <newmatio.h>
#include <iomanip>
#include <sstream>

#include "pppClient.h"
#include "pppUtils.h"
#include "bncephuser.h"
#include "bncutils.h"

using namespace BNC_PPP;
using namespace std;

// Constructor
////////////////////////////////////////////////////////////////////////////
t_pppClient::t_pppClient(const t_pppOptions* opt) {
  _opt      = new t_pppOptions(*opt);
  _filter   = new t_pppFilter(this);
  _epoData  = new t_epoData();
  _log      = new ostringstream();
  _ephUser  = new bncEphUser(false);
  _pppUtils = new t_pppUtils();
  _newEph = 0;
}

// Destructor
////////////////////////////////////////////////////////////////////////////
t_pppClient::~t_pppClient() {
  delete _filter;
  delete _epoData;
  delete _opt;
  delete _ephUser;
  delete _log;
  delete _pppUtils;
  if (_newEph)
    delete _newEph;
}

//
////////////////////////////////////////////////////////////////////////////
void t_pppClient::processEpoch(const vector<t_satObs*>& satObs, t_output* output) {

  // Convert and store observations
  // ------------------------------
  _epoData->clear();
  for (unsigned ii = 0; ii < satObs.size(); ii++) {
    const t_satObs* obs     = satObs[ii];
    t_prn prn = obs->_prn;
    if (prn.system() == 'E') {prn.setFlags(1);} // force I/NAV usage
    t_satData*   satData = new t_satData();

    if (_epoData->tt.undef()) {
      _epoData->tt = obs->_time;
    }

    satData->tt       = obs->_time;
    satData->prn      = QString(prn.toInternalString().c_str());
    satData->slipFlag = false;
    satData->P1       = 0.0;
    satData->P2       = 0.0;
    satData->P5       = 0.0;
    satData->P7       = 0.0;
    satData->L1       = 0.0;
    satData->L2       = 0.0;
    satData->L5       = 0.0;
    satData->L7       = 0.0;
    for (unsigned ifrq = 0; ifrq < obs->_obs.size(); ifrq++) {
      t_frqObs* frqObs = obs->_obs[ifrq];
      double cb = 0.0;
      const t_satCodeBias* satCB = _pppUtils->satCodeBias(prn);
      if (satCB && satCB->_bias.size()) {
        for (unsigned ii = 0; ii < satCB->_bias.size(); ii++) {

          const t_frqCodeBias& bias = satCB->_bias[ii];
          if (frqObs && frqObs->_rnxType2ch == bias._rnxType2ch) {
            cb  = bias._value;
          }
        }
      }
      if      (frqObs->_rnxType2ch[0] == '1') {
        if (frqObs->_codeValid)  satData->P1       = frqObs->_code + cb;
        if (frqObs->_phaseValid) satData->L1       = frqObs->_phase;
        if (frqObs->_slip)       satData->slipFlag = true;
      }
      else if (frqObs->_rnxType2ch[0] == '2') {
        if (frqObs->_codeValid)  satData->P2       = frqObs->_code + cb;
        if (frqObs->_phaseValid) satData->L2       = frqObs->_phase;
        if (frqObs->_slip)       satData->slipFlag = true;
      }
      else if (frqObs->_rnxType2ch[0] == '5') {
        if (frqObs->_codeValid)  satData->P5       = frqObs->_code + cb;
        if (frqObs->_phaseValid) satData->L5       = frqObs->_phase;
        if (frqObs->_slip)       satData->slipFlag = true;
      }
      else if (frqObs->_rnxType2ch[0] == '7') {
        if (frqObs->_codeValid)  satData->P7       = frqObs->_code + cb;
        if (frqObs->_phaseValid) satData->L7       = frqObs->_phase;
        if (frqObs->_slip)       satData->slipFlag = true;
      }
    }
    putNewObs(satData);
  }

  // Data Pre-Processing
  // -------------------
  QMutableMapIterator<QString, t_satData*> it(_epoData->satData);
  while (it.hasNext()) {
    it.next();
    QString    prn     = it.key();
    t_satData* satData = it.value();

    if (cmpToT(satData) != success) {
      delete satData;
      it.remove();
      continue;
    }

  }

  // Filter Solution
  // ---------------
  if (_filter->update(_epoData) == success) {
    output->_error = false;
    output->_epoTime     = _filter->time();
    output->_xyzRover[0] = _filter->x();
    output->_xyzRover[1] = _filter->y();
    output->_xyzRover[2] = _filter->z();
    copy(&_filter->Q().data()[0], &_filter->Q().data()[6], output->_covMatrix);
    output->_neu[0]      = _filter->neu()[0];
    output->_neu[1]      = _filter->neu()[1];
    output->_neu[2]      = _filter->neu()[2];
    output->_numSat      = _filter->numSat();
    output->_hDop        = _filter->HDOP();
    output->_trp0        = _filter->trp0();
    output->_trp         = _filter->trp();
    output->_trpStdev    = _filter->trpStdev();
  }
  else {
    output->_error = true;
  }

  output->_log = _log->str();
  delete _log; _log = new ostringstream();
}

//
////////////////////////////////////////////////////////////////////////////
void t_pppClient::putNewObs(t_satData* satData) {

  // Set Observations GPS
  // --------------------
  if      (satData->system() == 'G') {
    if (satData->P1 != 0.0 && satData->P2 != 0.0 &&
        satData->L1 != 0.0 && satData->L2 != 0.0 ) {
      t_frequency::type fType1 = t_lc::toFreq(satData->system(), t_lc::l1);
      t_frequency::type fType2 = t_lc::toFreq(satData->system(), t_lc::l2);
      double f1 = t_CST::freq(fType1, 0);
      double f2 = t_CST::freq(fType2, 0);
      double a1 =   f1 * f1 / (f1 * f1 - f2 * f2);
      double a2 = - f2 * f2 / (f1 * f1 - f2 * f2);
      satData->L1      = satData->L1 * t_CST::c / f1;
      satData->L2      = satData->L2 * t_CST::c / f2;
      satData->P3      = a1 * satData->P1 + a2 * satData->P2;
      satData->L3      = a1 * satData->L1 + a2 * satData->L2;
      satData->lambda3 = a1 * t_CST::c / f1 + a2 * t_CST::c / f2;
      satData->lkA     = a1;
      satData->lkB     = a2;
      _epoData->satData[satData->prn] = satData;
    }
    else {
      delete satData;
    }
  }

  // Set Observations Glonass
  // ------------------------
  else if (satData->system() == 'R' && _opt->useSystem('R')) {
    if (satData->P1 != 0.0 && satData->P2 != 0.0 &&
        satData->L1 != 0.0 && satData->L2 != 0.0 ) {

      int channel = 0;
      if (satData->system() == 'R') {
        const t_eph* eph = _ephUser->ephLast(satData->prn);
        if (eph) {
          channel = eph->slotNum();
        }
        else {
          delete satData;
          return;
        }
      }

      t_frequency::type fType1 = t_lc::toFreq(satData->system(), t_lc::l1);
      t_frequency::type fType2 = t_lc::toFreq(satData->system(), t_lc::l2);
      double f1 = t_CST::freq(fType1, channel);
      double f2 = t_CST::freq(fType2, channel);
      double a1 =   f1 * f1 / (f1 * f1 - f2 * f2);
      double a2 = - f2 * f2 / (f1 * f1 - f2 * f2);
      satData->L1      = satData->L1 * t_CST::c / f1;
      satData->L2      = satData->L2 * t_CST::c / f2;
      satData->P3      = a1 * satData->P1 + a2 * satData->P2;
      satData->L3      = a1 * satData->L1 + a2 * satData->L2;
      satData->lambda3 = a1 * t_CST::c / f1 + a2 * t_CST::c / f2;
      satData->lkA     = a1;
      satData->lkB     = a2;
      _epoData->satData[satData->prn] = satData;
    }
    else {
      delete satData;
    }
  }

  // Set Observations Galileo
  // ------------------------
  else if (satData->system() == 'E' && _opt->useSystem('E')) {
    if (satData->P1 != 0.0 && satData->P5 != 0.0 &&
        satData->L1 != 0.0 && satData->L5 != 0.0 ) {
      double f1 = t_CST::freq(t_frequency::E1, 0);
      double f5 = t_CST::freq(t_frequency::E5, 0);
      double a1 =   f1 * f1 / (f1 * f1 - f5 * f5);
      double a5 = - f5 * f5 / (f1 * f1 - f5 * f5);
      satData->L1      = satData->L1 * t_CST::c / f1;
      satData->L5      = satData->L5 * t_CST::c / f5;
      satData->P3      = a1 * satData->P1 + a5 * satData->P5;
      satData->L3      = a1 * satData->L1 + a5 * satData->L5;
      satData->lambda3 = a1 * t_CST::c / f1 + a5 * t_CST::c / f5;
      satData->lkA     = a1;
      satData->lkB     = a5;
      _epoData->satData[satData->prn] = satData;
    }
    else {
      delete satData;
    }
  }

  // Set Observations BDS
  // ---------------------
  else if (satData->system() == 'C' && _opt->useSystem('C')) {
    if (satData->P2 != 0.0 && satData->P7 != 0.0 &&
        satData->L2 != 0.0 && satData->L7 != 0.0 ) {
      double f2 = t_CST::freq(t_frequency::C2, 0);
      double f7 = t_CST::freq(t_frequency::C7, 0);
      double a2 =   f2 * f2 / (f2 * f2 - f7 * f7);
      double a7 = - f7 * f7 / (f2 * f2 - f7 * f7);
      satData->L2      = satData->L2 * t_CST::c / f2;
      satData->L7      = satData->L7 * t_CST::c / f7;
      satData->P3      = a2 * satData->P2 + a7 * satData->P7;
      satData->L3      = a2 * satData->L2 + a7 * satData->L7;
      satData->lambda3 = a2 * t_CST::c / f2 + a7 * t_CST::c / f7;
      satData->lkA     = a2;
      satData->lkB     = a7;
      _epoData->satData[satData->prn] = satData;
    }
    else {
      delete satData;
    }
  }
  else {
    delete satData;
  }
}

//
////////////////////////////////////////////////////////////////////////////
void t_pppClient::putOrbCorrections(const std::vector<t_orbCorr*>& corr) {
  for (unsigned ii = 0; ii < corr.size(); ii++) {
    QString prn = QString(corr[ii]->_prn.toInternalString().c_str());
    t_eph* eLast = _ephUser->ephLast(prn);
    t_eph* ePrev = _ephUser->ephPrev(prn);
    if      (eLast && eLast->IOD() == corr[ii]->_iod) {
      eLast->setOrbCorr(corr[ii]);
    }
    else if (ePrev && ePrev->IOD() == corr[ii]->_iod) {
      ePrev->setOrbCorr(corr[ii]);
    }
  }
}

//
////////////////////////////////////////////////////////////////////////////
void t_pppClient::putClkCorrections(const std::vector<t_clkCorr*>& corr) {
  for (unsigned ii = 0; ii < corr.size(); ii++) {
    QString prn = QString(corr[ii]->_prn.toInternalString().c_str());
    t_eph* eLast = _ephUser->ephLast(prn);
    t_eph* ePrev = _ephUser->ephPrev(prn);
    if      (eLast && eLast->IOD() == corr[ii]->_iod) {
      eLast->setClkCorr(corr[ii]);
    }
    else if (ePrev && ePrev->IOD() == corr[ii]->_iod) {
      ePrev->setClkCorr(corr[ii]);
    }
  }
}

//
//////////////////////////////////////////////////////////////////////////////
void t_pppClient::putCodeBiases(const std::vector<t_satCodeBias*>& satCodeBias) {
  for (unsigned ii = 0; ii < satCodeBias.size(); ii++) {
    _pppUtils->putCodeBias(new t_satCodeBias(*satCodeBias[ii]));
  }
}

//
//////////////////////////////////////////////////////////////////////////////
void t_pppClient::putPhaseBiases(const std::vector<t_satPhaseBias*>& /*satPhaseBias*/) {
}

//
//////////////////////////////////////////////////////////////////////////////
void t_pppClient::putTec(const t_vTec* /*vTec*/) {
}

//
//////////////////////////////////////////////////////////////////////////////
void t_pppClient::putEphemeris(const t_eph* eph) {
  if (_newEph)
    delete _newEph;
  _newEph = 0;
  const t_ephGPS* ephGPS = dynamic_cast<const t_ephGPS*>(eph);
  const t_ephGlo* ephGlo = dynamic_cast<const t_ephGlo*>(eph);
  const t_ephGal* ephGal = dynamic_cast<const t_ephGal*>(eph);
  const t_ephBDS* ephBDS = dynamic_cast<const t_ephBDS*>(eph);
  if      (ephGPS) {
    _newEph = new t_ephGPS(*ephGPS);
  }
  else if (ephGlo) {
    _newEph = new t_ephGlo(*ephGlo);
  }
  else if (ephGal) {
    _newEph = new t_ephGal(*ephGal);
  }
  else if (ephBDS) {
    _newEph = new t_ephBDS(*ephBDS);
  }

  if (_newEph) {
    _ephUser->putNewEph(_newEph, _opt->_realTime);
  }
}

// Satellite Position
////////////////////////////////////////////////////////////////////////////
t_irc t_pppClient::getSatPos(const bncTime& tt, const QString& prn,
                              ColumnVector& xc, ColumnVector& vv) {

  t_eph* eLast = _ephUser->ephLast(prn);
  t_eph* ePrev = _ephUser->ephPrev(prn);
  if      (eLast && eLast->getCrd(tt, xc, vv, _opt->useOrbClkCorr()) == success) {
    return success;
  }
  else if (ePrev && ePrev->getCrd(tt, xc, vv, _opt->useOrbClkCorr()) == success) {
    return success;
  }
  return failure;
}

// Correct Time of Transmission
////////////////////////////////////////////////////////////////////////////
t_irc t_pppClient::cmpToT(t_satData* satData) {

  double prange = satData->P3;
  if (prange == 0.0) {
    return failure;
  }

  double clkSat = 0.0;
  for (int ii = 1; ii <= 10; ii++) {

    bncTime ToT = satData->tt - prange / t_CST::c - clkSat;

    ColumnVector xc(6);
    ColumnVector vv(3);
    if (getSatPos(ToT, satData->prn, xc, vv) != success) {
      return failure;
    }

    double clkSatOld = clkSat;
    clkSat = xc(4);

    if ( fabs(clkSat-clkSatOld) * t_CST::c < 1.e-4 ) {
      satData->xx      = xc.Rows(1,3);
      satData->vv      = vv;
      satData->clk     = clkSat * t_CST::c;
      return success;
    }
  }

  return failure;
}

//
////////////////////////////////////////////////////////////////////////////
void t_pppClient::reset() {

  // to delete all parameters
  delete _filter;
  _filter   = new t_pppFilter(this);

  // to delete old orbit and clock corrections
  delete _ephUser;
  _ephUser  = new bncEphUser(false);

  // to delete old code biases
  delete _pppUtils;
  _pppUtils = new t_pppUtils();

}
