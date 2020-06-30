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

#include "bncconst.h"

const double t_CST::c     = 299792458.0;
const double t_CST::omega = 7292115.1467e-11;
const double t_CST::aell  = 6378137.000;
const double t_CST::rgeoc = 6370000.000;
const double t_CST::fInv  = 298.2572236;

//
//////////////////////////////////////////////////////////////////////////////
double t_CST::freq(t_frequency::type fType, int slotNum) {
  switch (fType) {
  // GPS
  case t_frequency::G1:    return 1575420000.0;
  case t_frequency::G2:    return 1227600000.0;
  case t_frequency::G5:    return 1176450000.0;
  // GLONASS
  case t_frequency::R1:    return 1602000000.0 + 562500.0 * slotNum;
  case t_frequency::R4:    return 1600995000.0;
  case t_frequency::R2:    return 1246000000.0 + 437500.0 * slotNum;
  case t_frequency::R6:    return 1248060000.0;
  case t_frequency::R3:    return 1202025000.0;
  // Galileo
  case t_frequency::E1:    return 1575420000.0;
  case t_frequency::E5:    return 1176450000.0;
  case t_frequency::E7:    return 1207140000.0;
  case t_frequency::E8:    return 1191795000.0;
  case t_frequency::E6:    return 1278750000.0;
  // SBAS
  case t_frequency::S1:    return 1575420000.0;
  case t_frequency::S5:    return 1176450000.0;
  // QZSS
  case t_frequency::J1:    return 1575420000.0;
  case t_frequency::J2:    return 1227600000.0;
  case t_frequency::J5:    return 1176450000.0;
  case t_frequency::J6:    return 1278750000.0;
  // BDS
  case t_frequency::C2:    return 1561098000.0;
  case t_frequency::C1:    return 1575420000.0;
  case t_frequency::C5:    return 1176450000.0;
  case t_frequency::C7:    return 1207140000.0;
  case t_frequency::C8:    return 1191795000.0;
  case t_frequency::C6:    return 1268520000.0;
  // IRNSS
  case t_frequency::I5:    return 1176450000.0;
  case t_frequency::I9:    return 2492028000.0;
  //
  case t_frequency::dummy:
  case t_frequency::max:   return 0.0;
  }
  return 0.0;
}

//
//////////////////////////////////////////////////////////////////////////////
double t_CST::lambda(t_frequency::type fType, int slotNum) {
  return c / freq(fType, slotNum);
}
