
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
 * Class:      t_pppCrdFile
 *
 * Purpose:    Read a priori coordinate file
 *
 * Author:     L. Mervart
 *
 * Created:    29-Jul-2014
 *
 * Changes:
 *
 * -----------------------------------------------------------------------*/

#include <fstream>
#include <sstream>
#include "pppCrdFile.h"
#include "bncutils.h"

using namespace std;
using namespace BNC_PPP;

//
//////////////////////////////////////////////////////////////////////////////
void t_pppCrdFile::readCrdFile(const string& fileName, vector<t_staInfo>& staInfoVec) {

  staInfoVec.clear();

  ifstream inFile(fileName.c_str());
  while (inFile.good()) {
    string line; getline(inFile,line);
    stripWhiteSpace(line);
    if ( line.empty() || line[0] == '#'|| line[0] == '!') {
      continue;
    }

    istringstream in;

    t_staInfo staInfo;

    size_t q1 = line.find('"');
    if (q1 != string::npos) {
      size_t q2 = line.find('"', q1+1);
      if (q2 == string::npos) {
        continue;
      }
      staInfo._name = line.substr(q1+1, q2-q1-1);
      in.str(line.substr(q2+1));
    }
    else {
      in.str(line);
      in >> staInfo._name;
    }

    in >> staInfo._xyz(1) >> staInfo._xyz(2) >> staInfo._xyz(3);

    if (!in.eof()) {
      in >> staInfo._neuAnt(1) >> staInfo._neuAnt(2) >> staInfo._neuAnt(3);
    }

    if (!in.eof()) {
      std::string hlp;
      getline(in, hlp);
      stripWhiteSpace(hlp);
      staInfo._antenna.assign( hlp.substr(0,20));
      hlp = hlp.erase(0, 20);
      if (hlp.length()) {
        stripWhiteSpace(hlp);
        staInfo._receiver = hlp;
      }
    }

    staInfoVec.push_back(staInfo);
  }
}
