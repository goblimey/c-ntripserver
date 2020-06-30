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
 * Class:      t_corrFile
 *
 * Purpose:    Reads DGPS Correction File
 *
 * Author:     L. Mervart
 *
 * Created:    12-Feb-2012
 *
 * Changes:    
 *
 * -----------------------------------------------------------------------*/

#include <iostream>
#include "corrfile.h"
#include "bncutils.h"
#include "bncephuser.h"

using namespace std;

// Constructor
////////////////////////////////////////////////////////////////////////////
t_corrFile::t_corrFile(QString fileName) {
  expandEnvVar(fileName);
  _stream.open(fileName.toLatin1().data());
}

// Destructor
////////////////////////////////////////////////////////////////////////////
t_corrFile::~t_corrFile() {
}

// Read till a given time
////////////////////////////////////////////////////////////////////////////
void t_corrFile::syncRead(const bncTime& tt) {

  while (_stream.good() && (!_lastEpoTime.valid() || _lastEpoTime <= tt)) {

    if (_lastLine.empty()) {
      getline(_stream, _lastLine); stripWhiteSpace(_lastLine);
      if      (!_stream.good()) {
        throw "t_corrFile: end of file";
      }
      else if (_lastLine.empty() || _lastLine[0] == '!') {
        continue;
      }
      else if (_lastLine[0] != '>') {
        throw "t_corrFile: error";
      }
    }

    int          numEntries;
    unsigned int updateInt;
    string       staID;
    t_corrSSR::e_type corrType = t_corrSSR::readEpoLine(_lastLine, _lastEpoTime, updateInt, numEntries, staID);
    if      (corrType == t_corrSSR::unknown) {
      throw "t_corrFile: unknown line " + _lastLine;
    }
    else if (_lastEpoTime > tt) {
      break;
    }
    else if (corrType == t_corrSSR::clkCorr) {
      QList<t_clkCorr> clkCorrList;
      t_clkCorr::readEpoch(_lastLine, _stream, clkCorrList);
      emit newClkCorrections(clkCorrList);
    }
    else if (corrType == t_corrSSR::orbCorr) {
      QList<t_orbCorr> orbCorrList;
      t_orbCorr::readEpoch(_lastLine, _stream, orbCorrList);
      QListIterator<t_orbCorr> it(orbCorrList);
      while (it.hasNext()) {
        const t_orbCorr& corr = it.next();
        _corrIODs[QString(corr._prn.toInternalString().c_str())] = corr._iod;
      }
      emit newOrbCorrections(orbCorrList);
    }
    else if (corrType == t_corrSSR::codeBias) {
      QList<t_satCodeBias> satCodeBiasList;
      t_satCodeBias::readEpoch(_lastLine, _stream, satCodeBiasList);
      emit newCodeBiases(satCodeBiasList);
    }
    else if (corrType == t_corrSSR::phaseBias) {
      QList<t_satPhaseBias> satPhaseBiasList;
      t_satPhaseBias::readEpoch(_lastLine, _stream, satPhaseBiasList);
      emit newPhaseBiases(satPhaseBiasList);
    }
    else if (corrType == t_corrSSR::vTec) {
      t_vTec vTec;
      t_vTec::read(_lastLine, _stream, vTec);
      emit newTec(vTec);
    }

    _lastLine.clear();
  }
}
