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
 * Class:      RTCM2Decoder
 *
 * Purpose:    RTCM2 Decoder
 *
 * Author:     L. Mervart
 *
 * Created:    24-Aug-2006
 *
 * Changes:
 *
 * -----------------------------------------------------------------------*/

#include <math.h>
#include <sstream>
#include <iomanip>
#include <set>

#include "../bncutils.h"
#include "rtcm_utils.h"
#include "RTCM2Decoder.h"

using namespace std;
using namespace rtcm2;

//
// Constructor
//

RTCM2Decoder::RTCM2Decoder(const std::string& ID) : _ephUser(true) {
  _ID = ID;
}

//
// Destructor
//

RTCM2Decoder::~RTCM2Decoder() {
}

//
t_irc RTCM2Decoder::getStaCrd(double& xx, double& yy, double& zz) {
  if (!_msg03.validMsg) {
    return failure;
  }

  xx = _msg03.x + (_msg22.validMsg ? _msg22.dL1[0] : 0.0);
  yy = _msg03.y + (_msg22.validMsg ? _msg22.dL1[1] : 0.0);
  zz = _msg03.z + (_msg22.validMsg ? _msg22.dL1[2] : 0.0);

  return success;
}

//
t_irc RTCM2Decoder::getStaCrd(double& xx, double& yy, double& zz, double& dx1,
    double& dy1, double& dz1, double& dx2, double& dy2, double& dz2) {
  xx = _msg03.x;
  yy = _msg03.y;
  zz = _msg03.z;

  dx1 = (_msg22.validMsg ? _msg22.dL1[0] : 0.0);
  dy1 = (_msg22.validMsg ? _msg22.dL1[1] : 0.0);
  dz1 = (_msg22.validMsg ? _msg22.dL1[2] : 0.0);

  dx2 = (_msg22.validMsg ? _msg22.dL2[0] : 0.0);
  dy2 = (_msg22.validMsg ? _msg22.dL2[1] : 0.0);
  dz2 = (_msg22.validMsg ? _msg22.dL2[2] : 0.0);

  return success;
}

//
t_irc RTCM2Decoder::Decode(char* buffer, int bufLen, vector<string>& errmsg) {

  errmsg.clear();

  _buffer.append(buffer, bufLen);
  int refWeek;
  double refSecs;
  currentGPSWeeks(refWeek, refSecs);
  bool decoded = false;

  while (true) {
    _PP.getPacket(_buffer);
    if (!_PP.valid()) {
      if (decoded) {
        return success;
      } else {
        return failure;
      }
    }

    // Store message number
    _typeList.push_back(_PP.ID());

    if (_PP.ID() == 18 || _PP.ID() == 19) {

      _ObsBlock.extract(_PP);

      if (_ObsBlock.valid()) {
        decoded = true;

        int epochWeek;
        double epochSecs;
        _ObsBlock.resolveEpoch(refWeek, refSecs, epochWeek, epochSecs);

        for (int iSat = 0; iSat < _ObsBlock.nSat; iSat++) {
          t_satObs obs;
          if (_ObsBlock.PRN[iSat] > 100) {
            obs._prn.set('R', _ObsBlock.PRN[iSat] % 100);
          } else {
            obs._prn.set('G', _ObsBlock.PRN[iSat]);
          }
          char sys = obs._prn.system();
          obs._time.set(epochWeek, epochSecs);

          t_frqObs* frqObs1C = new t_frqObs;
          frqObs1C->_rnxType2ch = "1C";
          frqObs1C->_codeValid = true;
          frqObs1C->_code = _ObsBlock.rng_C1[iSat];
          obs._obs.push_back(frqObs1C);

          t_frqObs* frqObs1P = new t_frqObs;
          frqObs1P->_rnxType2ch = (sys == 'G') ? "1W" : "1P";
          frqObs1P->_codeValid = true;
          frqObs1P->_code = _ObsBlock.rng_P1[iSat];
          frqObs1P->_phaseValid = true;
          frqObs1P->_phase = _ObsBlock.resolvedPhase_L1(iSat);
          //frqObs1P->_slipCounter = _ObsBlock.slip_L1[iSat];
          frqObs1P->_slipCounter = -1; // because RTCM2 definition is vice versa to RTCM3
          obs._obs.push_back(frqObs1P);

          t_frqObs* frqObs2P = new t_frqObs;
          frqObs2P->_rnxType2ch = (sys == 'G') ? "2W" : "2P";
          frqObs2P->_codeValid = true;
          frqObs2P->_code = _ObsBlock.rng_P2[iSat];
          frqObs2P->_phaseValid = true;
          frqObs2P->_phase = _ObsBlock.resolvedPhase_L2(iSat);
          //frqObs2P->_slipCounter = _ObsBlock.slip_L2[iSat];
          frqObs2P->_slipCounter = -1; // because RTCM2 definition is vice versa to RTCM3
          obs._obs.push_back(frqObs2P);

          _obsList.push_back(obs);
        }
        _ObsBlock.clear();
      }
    }

    else if (_PP.ID() == 20 || _PP.ID() == 21) {
      _msg2021.extract(_PP);

      if (_msg2021.valid()) {
        decoded = true;
        translateCorr2Obs(errmsg);
      }
    }

    else if (_PP.ID() == 3) {
      _msg03.extract(_PP);
    }

    else if (_PP.ID() == 22) {
      _msg22.extract(_PP);
    }

    else if (_PP.ID() == 23) {
      _msg23.extract(_PP);
    }

    else if (_PP.ID() == 24) {
      _msg24.extract(_PP);
    }

    // Output for RTCM scan
    if (_PP.ID() == 3) {
      if (_msg03.validMsg) {
        _antList.push_back(t_antRefPoint());

        this->getStaCrd(_antList.back().xx, _antList.back().yy, _antList.back().zz);

        _antList.back().type = t_antRefPoint::APC;
        _antList.back().message = _PP.ID();
      }
    } else if (_PP.ID() == 23) {
      if (_msg23.validMsg) {
        int antlen = strlen(_msg23.antType.c_str());
        int serlen = strlen(_msg23.antSN.c_str());
        if ((antlen) &&
            (_antType.empty() || strncmp(_antType.back().descriptor, _msg23.antType.c_str(), antlen) != 0)) {
          _antType.push_back(t_antInfo());
          memcpy(_antType.back().descriptor, _msg23.antType.c_str(), antlen);
          _antType.back().descriptor[antlen] = 0;
          if (serlen) {
            memcpy(_antType.back().serialnumber,  _msg23.antSN.c_str(), serlen);
            _antType.back().serialnumber[serlen] = 0;
          }
        }
      }
    } else if (_PP.ID() == 24) {
      if (_msg24.validMsg) {
        _antList.push_back(t_antRefPoint());

        _antList.back().xx = _msg24.x;
        _antList.back().yy = _msg24.y;
        _antList.back().zz = _msg24.z;

        _antList.back().height_f = true;
        _antList.back().height = _msg24.h;

        _antList.back().type = t_antRefPoint::ARP;
        _antList.back().message = _PP.ID();
      }
    }
  }
  return success;
}

void RTCM2Decoder::translateCorr2Obs(vector<string>& errmsg) {

  QMutexLocker locker(&_mutex);

  if (!_msg03.validMsg || !_msg2021.valid()) {
    return;
  }

  double stax = _msg03.x + (_msg22.validMsg ? _msg22.dL1[0] : 0.0);
  double stay = _msg03.y + (_msg22.validMsg ? _msg22.dL1[1] : 0.0);
  double staz = _msg03.z + (_msg22.validMsg ? _msg22.dL1[2] : 0.0);

  int refWeek;
  double refSecs;
  currentGPSWeeks(refWeek, refSecs);

  // Resolve receiver time of measurement (see RTCM 2.3, page 4-42, Message 18, Note 1)
  // ----------------------------------------------------------------------------------
  double hoursec_est = _msg2021.hoursec();      // estimated time of measurement
  double hoursec_rcv = rint(hoursec_est * 1e2) / 1e2; // receiver clock reading at hoursec_est
  double rcv_clk_bias = (hoursec_est - hoursec_rcv) * c_light;

  int GPSWeek;
  double GPSWeeks;
  resolveEpoch(hoursec_est, refWeek, refSecs, GPSWeek, GPSWeeks);

  int GPSWeek_rcv;
  double GPSWeeks_rcv;
  resolveEpoch(hoursec_rcv, refWeek, refSecs, GPSWeek_rcv, GPSWeeks_rcv);

  // Loop over all satellites
  // ------------------------
  for (RTCM2_2021::data_iterator icorr = _msg2021.data.begin();
      icorr != _msg2021.data.end(); icorr++) {
    const RTCM2_2021::HiResCorr* corr = icorr->second;

    // beg test
    if (corr->PRN >= 200) {
      continue;
    }
    // end test

    QString prn;
    char sys;
    if (corr->PRN < 200) {
      sys = 'G';
      prn = sys + QString("%1_0").arg(corr->PRN, 2, 10, QChar('0'));
    } else {
      sys = 'R';
      prn = sys + QString("%1_0").arg(corr->PRN - 200, 2, 10, QChar('0'));
    }

    double L1 = 0;
    double L2 = 0;
    double P1 = 0;
    double P2 = 0;
    string obsT = "";

    // new observation
    t_satObs new_obs;

    t_frqObs* frqObs1C = new t_frqObs;
    frqObs1C->_rnxType2ch = "1C";
    new_obs._obs.push_back(frqObs1C);

    t_frqObs* frqObs1P = new t_frqObs;
    frqObs1P->_rnxType2ch = (sys == 'G') ? "1W" : "1P";
    new_obs._obs.push_back(frqObs1P);

    t_frqObs* frqObs2P = new t_frqObs;
    frqObs2P->_rnxType2ch = (sys == 'G') ? "2W" : "2P";
    new_obs._obs.push_back(frqObs2P);

    // missing IOD
    vector<string> missingIOD;
    vector<string> hasIOD;
    for (unsigned ii = 0; ii < 4; ii++) {
      unsigned int IODcorr = 0;
      double corrVal = 0;
      const t_eph* eph = 0;
      double* obsVal = 0;

      switch (ii) {
        case 0: // --- L1 ---
          IODcorr = corr->IODp1;
          corrVal = corr->phase1 * LAMBDA_1;
          obsVal = &L1;
          obsT = "L1";
          break;
        case 1: // --- L2 ---
          IODcorr = corr->IODp2;
          corrVal = corr->phase2 * LAMBDA_2;
          obsVal = &L2;
          obsT = "L2";
          break;
        case 2: // --- P1 ---
          IODcorr = corr->IODr1;
          corrVal = corr->range1;
          obsVal = &P1;
          obsT = "P1";
          break;
        case 3: // --- P2 ---
          IODcorr = corr->IODr2;
          corrVal = corr->range2;
          obsVal = &P2;
          obsT = "P2";
          break;
        default:
          continue;
      }

      // Select corresponding ephemerides
      const t_eph* ephLast = _ephUser.ephLast(prn);
      const t_eph* ephPrev = _ephUser.ephPrev(prn);
      if (ephLast && ephLast->IOD() == IODcorr) {
        eph = ephLast;
      } else if (ephPrev && ephPrev->IOD() == IODcorr) {
        eph = ephPrev;
      }

      if (eph) {
        ostringstream msg;
        msg << obsT << ':' << setw(3) << eph->IOD();
        hasIOD.push_back(msg.str());

        int GPSWeek_tot;
        double GPSWeeks_tot;
        double rho, xSat, ySat, zSat, clkSat;
        cmpRho(eph, stax, stay, staz, GPSWeek, GPSWeeks, rho, GPSWeek_tot,
            GPSWeeks_tot, xSat, ySat, zSat, clkSat);

        *obsVal = rho - corrVal + rcv_clk_bias - clkSat;

        if (*obsVal == 0)
          *obsVal = ZEROVALUE;

        if (corr->PRN < 200) {
          new_obs._prn.set('G', corr->PRN);
        } else {
          new_obs._prn.set('R', corr->PRN - 200);
        }
        new_obs._time.set(GPSWeek_rcv, GPSWeeks_rcv);

        // Store estimated measurements
        // ----------------------------
        switch (ii) {
          case 0: // --- L1 ---
            frqObs1P->_phaseValid = true;
            frqObs1P->_phase = *obsVal / LAMBDA_1;
            //frqObs1P->_slipCounter = corr->lock1;
            frqObs1P->_slipCounter = -1; // because RTCM2 definition is vice versa to RTCM3
            break;
          case 1: // --- L2 ---
            frqObs2P->_phaseValid = true;
            frqObs2P->_phase = *obsVal / LAMBDA_2;
            //frqObs2P->_slipCounter = corr->lock2;
            frqObs2P->_slipCounter = -1; // because RTCM2 definition is vice versa to RTCM3
            break;
          case 2: // --- C1 / P1 ---
            if (corr->Pind1) {
              frqObs1P->_codeValid = true;
              frqObs1P->_code = *obsVal;
            } else {
              frqObs1C->_codeValid = true;
              frqObs1C->_code = *obsVal;
            }
            break;
          case 3: // --- C2 / P2 ---
            if (corr->Pind2) {
              frqObs2P->_codeValid = true;
              frqObs2P->_code = *obsVal;
            }
            break;
          default:
            continue;
        }
      } else if (IODcorr != 0) {
        ostringstream msg;
        msg << obsT << ':' << setw(3) << IODcorr;
        missingIOD.push_back(msg.str());
      }
    } // loop over frequencies

    // Error report
    if (missingIOD.size()) {
      ostringstream missingIODstr;

      copy(missingIOD.begin(), missingIOD.end(),
          ostream_iterator<string>(missingIODstr, "   "));

      errmsg.push_back(
          "missing eph for " + string(prn.toLatin1().data()) + " , IODs "
              + missingIODstr.str());
    }

    // Store new observation
    if (new_obs._time.mjd() > 0) {
      _obsList.push_back(new_obs);
    }
  }
}
