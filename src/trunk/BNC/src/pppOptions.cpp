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
 * Class:      t_pppOptions
 *
 * Purpose:    Options for PPP client
 *
 * Author:     L. Mervart
 *
 * Created:    29-Jul-2014
 *
 * Changes:
 *
 * -----------------------------------------------------------------------*/

#include <newmatio.h>
#include "pppOptions.h"

using namespace BNC_PPP;
using namespace std;

// Constructor
//////////////////////////////////////////////////////////////////////////////
t_pppOptions::t_pppOptions() {
  _xyzAprRover.ReSize(3); _xyzAprRover = 0.0;
  _neuEccRover.ReSize(3); _neuEccRover = 0.0;
  _aprSigCrd.ReSize(3);   _aprSigCrd   = 0.0;
  _noiseCrd.ReSize(3);    _noiseCrd    = 0.0;
}

// Destructor
//////////////////////////////////////////////////////////////////////////////
t_pppOptions::~t_pppOptions() {
}

//
//////////////////////////////////////////////////////////////////////////////
const std::vector<t_lc::type>& t_pppOptions::LCs(char system) const {

  if      (system == 'R') {
    return _LCsGLONASS;
  }
  else if (system == 'E') {
    return _LCsGalileo;
  }
  else if (system == 'C') {
    return  _LCsBDS;
  }
  else {
    return _LCsGPS;
  }
}

//
//////////////////////////////////////////////////////////////////////////////
bool t_pppOptions::useOrbClkCorr() const {
  if (_realTime) {
    return !_corrMount.empty();
  }
  else {
    return !_corrFile.empty();
  }
}

// Processed satellite systems
/////////////////////////////////////////////////////////////////////////////
vector<char> t_pppOptions::systems() const {
  vector<char> answ;
  if (_LCsGPS.size()     > 0) answ.push_back('G');
  if (_LCsGLONASS.size() > 0) answ.push_back('R');
  if (_LCsGalileo.size() > 0) answ.push_back('E');
  if (_LCsBDS.size()     > 0) answ.push_back('C');
  return answ;
}

//
/////////////////////////////////////////////////////////////////////////////
vector<t_lc::type> t_pppOptions::ambLCs(char system) const {

  const vector<t_lc::type>& allLCs = LCs(system);
  vector<t_lc::type>        phaseLCs;
  for (unsigned ii = 0; ii < allLCs.size(); ii++) {
    if (t_lc::includesPhase(allLCs[ii])) {
      phaseLCs.push_back(allLCs[ii]);
    }
  }

  vector<t_lc::type> answ;
  if      (phaseLCs.size() == 1) {
    answ.push_back(phaseLCs[0]);
  }
  else if (phaseLCs.size() >  1) {
    answ.push_back(t_lc::l1);
    answ.push_back(t_lc::l2);
  }

  return answ;
}

//
/////////////////////////////////////////////////////////////////////////////
vector<t_lc::type> t_pppOptions::codeLCs(char system) const {

  const vector<t_lc::type>& allLCs = LCs(system);
  vector<t_lc::type>        codeLCs;
  for (unsigned ii = 0; ii < allLCs.size(); ii++) {
    if (t_lc::includesCode(allLCs[ii])) {
      codeLCs.push_back(allLCs[ii]);
    }
  }

  vector<t_lc::type> answ;
  if      (codeLCs.size() == 1) {
    answ.push_back(codeLCs[0]);
  }
  else if (codeLCs.size() >  1) {
    answ.push_back(t_lc::c1);
    answ.push_back(t_lc::c2);
  }

  return answ;
}


