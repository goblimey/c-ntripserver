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
 * Class:      t_sp3Comp
 *
 * Purpose:    Compare SP3 Files
 *
 * Author:     L. Mervart
 *
 * Created:    24-Nov-2014
 *
 * Changes:
 *
 * -----------------------------------------------------------------------*/

#include <iostream>
#include <iomanip>
#include "sp3Comp.h"
#include "bnccore.h"
#include "bncsettings.h"
#include "bncutils.h"
#include "bncsp3.h"

using namespace std;

// Constructor
////////////////////////////////////////////////////////////////////////////
t_sp3Comp::t_sp3Comp(QObject* parent) : QThread(parent) {

  bncSettings settings;
  _sp3FileNames = settings.value("sp3CompFile").toString().split(QRegExp("[ ,]"), QString::SkipEmptyParts);
  for (int ii = 0; ii < _sp3FileNames.size(); ii++) {
    expandEnvVar(_sp3FileNames[ii]);
  }
  _logFileName = settings.value("sp3CompOutLogFile").toString(); expandEnvVar(_logFileName);
  _logFile     = 0;
  _log         = 0;

  _excludeSats = settings.value("sp3CompExclude").toString().split(QRegExp("[ ,]"), QString::SkipEmptyParts);
}

// Destructor
////////////////////////////////////////////////////////////////////////////
t_sp3Comp::~t_sp3Comp() {
  delete _log;
  delete _logFile;
}

//
////////////////////////////////////////////////////////////////////////////
void t_sp3Comp::run() {

  // Open Log File
  // -------------
  _logFile = new QFile(_logFileName);
  if (_logFile->open(QIODevice::WriteOnly | QIODevice::Text)) {
    _log = new QTextStream();
    _log->setDevice(_logFile);
  }
  if (!_log) {
    goto end;
  }

  for (int ii = 0; ii < _sp3FileNames.size(); ii++) {
    *_log << "! SP3 File " << ii+1 << ": " << _sp3FileNames[ii] << endl;
  }
  if (_sp3FileNames.size() != 2) {
    *_log << "ERROR: sp3Comp requires two input SP3 files" << endl;
    goto end;
  }

  try {
    ostringstream msg;
    compare(msg);
    *_log << msg.str().c_str();
  }
  catch (const string& error) {
    *_log << "ERROR: " << error.c_str() << endl;
  }
  catch (const char* error) {
    *_log << "ERROR: " << error << endl;
  }
  catch (Exception& exc) {
    *_log << "ERROR: " << exc.what() << endl;
  }
  catch (std::exception& exc) {
    *_log << "ERROR: " << exc.what() << endl;
  }
  catch (QString error) {
    *_log << "ERROR: " << error << endl;
  }
  catch (...) {
    *_log << "ERROR: " << "unknown exception" << endl;
  }

  // Exit (thread)
  // -------------
 end:
  _log->flush();
  if (BNC_CORE->mode() != t_bncCore::interactive) {
    qApp->exit(0);
    msleep(100); //sleep 0.1 sec
  }
  else {
    emit finished();
    deleteLater();
  }
}

// Satellite Index in clkSats set
////////////////////////////////////////////////////////////////////////////////
int t_sp3Comp::satIndex(const set<t_prn>& clkSats, const t_prn& prn) const {
  int ret = 0;
  for (set<t_prn>::const_iterator it = clkSats.begin(); it != clkSats.end(); it++) {
    if ( *it == prn) {
      return ret;
    }
    ++ret;
  }
  return -1;
}

// Estimate Clock Offsets
////////////////////////////////////////////////////////////////////////////////
void t_sp3Comp::processClocks(const set<t_prn>& clkSats, const vector<t_epoch*>& epochsIn,
                              map<string, t_stat>& stat) const {

  if (clkSats.size() == 0) {
    return;
  }

  vector<t_epoch*> epochs;
  for (unsigned ii = 0; ii < epochsIn.size(); ii++) {
    unsigned numSatOK = 0;
    std::map<t_prn, double>::const_iterator it;
    for (it = epochsIn[ii]->_dc.begin(); it != epochsIn[ii]->_dc.end(); it++) {
      if (satIndex(clkSats, it->first) != -1) {
        numSatOK += 1;
      }
    }
    if (numSatOK > 0) {
      epochs.push_back(epochsIn[ii]);
    }
  }
  if (epochs.size() == 0) {
    return;
  }

  int nPar = epochs.size() + clkSats.size();
  SymmetricMatrix NN(nPar); NN = 0.0;
  ColumnVector    bb(nPar); bb = 0.0;

  // Create Matrix A'A and vector b
  // ------------------------------
  for (unsigned ie = 0; ie < epochs.size(); ie++) {
    const map<t_prn, double>& dc = epochs[ie]->_dc;
    Matrix       AA(dc.size(), nPar); AA = 0.0;
    ColumnVector ll(dc.size());       ll = 0.0;
    map<t_prn, double>::const_iterator it;
    int ii = -1;
    for (it = dc.begin(); it != dc.end(); it++) {
      const t_prn& prn = it->first;
      if (satIndex(clkSats, prn) != -1) {
        ++ii;
        int index = epochs.size() + satIndex(clkSats, prn);
        AA[ii][ie]    = 1.0; // epoch-specfic offset (common for all satellites)
        AA[ii][index] = 1.0; // satellite-specific offset (common for all epochs)
        ll[ii]        = it->second;
      }
    }
    SymmetricMatrix dN; dN << AA.t() * AA;
    NN += dN;
    bb += AA.t() * ll;
  }

  // Regularize NN
  // -------------
  RowVector HH(nPar);
  HH.columns(1, epochs.size())      = 0.0;
  HH.columns(epochs.size()+1, nPar) = 1.0;
  SymmetricMatrix dN; dN << HH.t() * HH;
  NN += dN;

  // Estimate Parameters
  // -------------------
  ColumnVector xx = NN.i() * bb;

  // Compute clock residuals
  // -----------------------
  for (unsigned ie = 0; ie < epochs.size(); ie++) {
    map<t_prn, double>& dc = epochs[ie]->_dc;
    for (map<t_prn, double>::iterator it = dc.begin(); it != dc.end(); it++) {
      const t_prn& prn = it->first;
      if (satIndex(clkSats, prn) != -1) {
        int  index = epochs.size() + satIndex(clkSats, prn);
        dc[prn]                      = it->second - xx[ie] - xx[index];
        stat[prn.toString()]._offset = xx[index];
      }
    }
  }
}

// Main Routine
////////////////////////////////////////////////////////////////////////////////
void t_sp3Comp::compare(ostringstream& out) const {

  // Synchronize reading of two sp3 files
  // ------------------------------------
  bncSP3 in1(_sp3FileNames[0]); in1.nextEpoch();
  bncSP3 in2(_sp3FileNames[1]); in2.nextEpoch();

  vector<t_epoch*> epochs;
  while (in1.currEpoch() && in2.currEpoch()) {
    bncTime t1 = in1.currEpoch()->_tt;
    bncTime t2 = in2.currEpoch()->_tt;
    if      (t1 < t2) {
      in1.nextEpoch();
    }
    else if (t1 > t2) {
      in2.nextEpoch();
    }
    else if (t1 == t2) {
      t_epoch* epo = new t_epoch; epo->_tt = t1;
      bool epochOK = false;
      for (int i1 = 0; i1 < in1.currEpoch()->_sp3Sat.size(); i1++) {
        bncSP3::t_sp3Sat* sat1 = in1.currEpoch()->_sp3Sat[i1];
        for (int i2 = 0; i2 < in2.currEpoch()->_sp3Sat.size(); i2++) {
          bncSP3::t_sp3Sat* sat2 = in2.currEpoch()->_sp3Sat[i2];
          if (sat1->_prn == sat2->_prn) {
            epochOK        = true;
            epo->_dr[sat1->_prn]  = sat1->_xyz - sat2->_xyz;
            epo->_xyz[sat1->_prn] = sat1->_xyz;
            if (sat1->_clkValid && sat2->_clkValid) {
              epo->_dc[sat1->_prn] = sat1->_clk - sat2->_clk;
            }
          }
        }
      }
      if (epochOK) {
        epochs.push_back(epo);
      }
      else {
        delete epo;
      }
      in1.nextEpoch();
      in2.nextEpoch();
    }
  }

  // Transform xyz into radial, along-track, and out-of-plane
  // --------------------------------------------------------
  if (epochs.size() < 2) {
    throw "t_sp3Comp: not enough common epochs";
  }

  set<t_prn> clkSatsAll;

  for (unsigned ie = 0; ie < epochs.size(); ie++) {
    t_epoch* epoch  = epochs[ie];
    t_epoch* epoch2 = 0;
    if (ie == 0) {
      epoch2 = epochs[ie+1];
    }
    else {
      epoch2 = epochs[ie-1];
    }
    double dt = epoch->_tt - epoch2->_tt;
    map<t_prn, ColumnVector>& dr   = epoch->_dr;
    map<t_prn, ColumnVector>& xyz  = epoch->_xyz;
    map<t_prn, ColumnVector>& xyz2 = epoch2->_xyz;
    for (map<t_prn, ColumnVector>::const_iterator it = dr.begin(); it != dr.end(); it++) {
      const t_prn&  prn = it->first;
      if (xyz2.find(prn) != xyz2.end()) {
        const ColumnVector  dx = dr[prn];
        const ColumnVector& x1 = xyz[prn];
        const ColumnVector& x2 = xyz2[prn];
        ColumnVector vel = (x1 - x2) / dt;
        XYZ_to_RSW(x1, vel, dx, dr[prn]);
        if (epoch->_dc.find(prn) != epoch->_dc.end()) {
          clkSatsAll.insert(prn);
        }
      }
      else {
        epoch->_dc.erase(prn);
        epoch->_dr.erase(prn);
      }
    }
  }

  map<string, t_stat> stat;

  // Estimate Clock Offsets
  // ----------------------
  string systems;
  for (set<t_prn>::const_iterator it = clkSatsAll.begin(); it != clkSatsAll.end(); it++) {
    if (systems.find(it->system()) == string::npos) {
      systems += it->system();
    }
  }
  for (unsigned iSys = 0; iSys < systems.size(); iSys++) {
    char system = systems[iSys];
    set<t_prn> clkSats;
    for (set<t_prn>::const_iterator it = clkSatsAll.begin(); it != clkSatsAll.end(); it++) {
      if (it->system() == system && !excludeSat(*it)) {
        clkSats.insert(*it);
      }
    }
    processClocks(clkSats, epochs, stat);
  }

  // Print Residuals
  // ---------------
  const string all = "ZZZ";

  out.setf(ios::fixed);
  out << "!\n!  MJD       PRN  radial   along   out        clk    clkRed   iPRN"
           "\n! ----------------------------------------------------------------\n";
  for (unsigned ii = 0; ii < epochs.size(); ii++) {
    const t_epoch* epo = epochs[ii];
    const map<t_prn, ColumnVector>& dr = epochs[ii]->_dr;
    const map<t_prn, double>&       dc = epochs[ii]->_dc;
    for (map<t_prn, ColumnVector>::const_iterator it = dr.begin(); it != dr.end(); it++) {
      const t_prn&  prn = it->first;
      if (!excludeSat(prn)) {
        const ColumnVector& rao = it->second;
        out << setprecision(6) << epo->_tt.mjddec() << ' ' << prn.toString() << ' '
            << setw(7) << setprecision(4) << rao[0] << ' '
            << setw(7) << setprecision(4) << rao[1] << ' '
            << setw(7) << setprecision(4) << rao[2] << "    ";
        stat[prn.toString()]._rao += SP(rao, rao); // Schur product
        stat[prn.toString()]._nr  += 1;
        stat[all]._rao            += SP(rao, rao);
        stat[all]._nr             += 1;
        if (dc.find(prn) != dc.end()) {
          double clkRes    = dc.find(prn)->second;
          double clkResRed = clkRes - it->second[0]; // clock minus radial component
          out << setw(7) << setprecision(4) << clkRes << ' '
              << setw(7) << setprecision(4) << clkResRed;
          stat[prn.toString()]._dc    += clkRes * clkRes;
          stat[prn.toString()]._dcRed += clkResRed * clkResRed;
          stat[prn.toString()]._nc    += 1;
          stat[all]._dc               += clkRes * clkRes;
          stat[all]._dcRed            += clkResRed * clkResRed;
          stat[all]._nc               += 1;
        }
        else {
          out << "  .       .    ";
        }
        out << "    " << setw(2) << int(prn) << endl;
      }
    }
    delete epo;
  }

  // Print Summary
  // -------------
  out << "!\n! RMS[m]\n";
  out << "!\n!    PRN  radial   along   out     nOrb    clk   clkRed   nClk    Offset"
           "\n! ----------------------------------------------------------------------\n";
  for (map<string, t_stat>::iterator it = stat.begin(); it != stat.end(); it++) {
    const string& prn  = it->first;
    t_stat&       stat = it->second;
    if (stat._nr > 0) {
      stat._rao[0] = sqrt(stat._rao[0] / stat._nr);
      stat._rao[1] = sqrt(stat._rao[1] / stat._nr);
      stat._rao[2] = sqrt(stat._rao[2] / stat._nr);
      if (prn == all) {
        out << "!\n!  Total ";
      }
      else {
        out << "!    " << prn << ' ';
      }
      out << setw(7) << setprecision(4) << stat._rao[0] << ' '
          << setw(7) << setprecision(4) << stat._rao[1] << ' '
          << setw(7) << setprecision(4) << stat._rao[2] << ' '
          << setw(6) << stat._nr << " ";
      if (stat._nc > 0) {
        stat._dc    = sqrt(stat._dc / stat._nc);
        stat._dcRed = sqrt(stat._dcRed / stat._nc);
        out << setw(7) << setprecision(4) << stat._dc << ' '
            << setw(7) << setprecision(4) << stat._dcRed << ' '
            << setw(6) << stat._nc << " ";
        if (prn != all) {
          out << setw(9) << setprecision(4) << stat._offset;
        }
      }
      else {
        out << "  .       .    ";
      }
      out << endl;
    }
  }
}

//
////////////////////////////////////////////////////////////////////////////
bool t_sp3Comp::excludeSat(const t_prn& prn) const {
  QStringListIterator it(_excludeSats);
  while (it.hasNext()) {
    string prnStr = it.next().toLatin1().data();
    if (prnStr == prn.toString() || prnStr == prn.toString().substr(0,1)) {
      return true;
    }
  }
  return false;
}

