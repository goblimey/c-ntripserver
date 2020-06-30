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
 * Class:      bncAntex
 *
 * Purpose:    Antenna Phase Centers and Variations from ANTEX File
 *
 * Author:     L. Mervart
 *
 * Created:    26-Jan-2011
 *
 * Changes:
 *
 * -----------------------------------------------------------------------*/

#include <iostream>
#include <newmatio.h>

#include "bncantex.h"
#include "pppModel.h"

using namespace std;

// Constructor
////////////////////////////////////////////////////////////////////////////
bncAntex::bncAntex() {
}

// Constructor
////////////////////////////////////////////////////////////////////////////
bncAntex::bncAntex(const char* fileName) {
  readFile(QString(fileName));
}

// Destructor
////////////////////////////////////////////////////////////////////////////
bncAntex::~bncAntex() {
  QMapIterator<QString, t_antMap*> it(_maps);
  while (it.hasNext()) {
    it.next();
    delete it.value();
  }
}

// Print
////////////////////////////////////////////////////////////////////////////
void bncAntex::print() const {
  QMapIterator<QString, t_antMap*> itAnt(_maps);
  while (itAnt.hasNext()) {
    itAnt.next();
    t_antMap* map = itAnt.value();
    cout << map->antName.toLatin1().data() << endl;
    cout << "    " << map->zen1 << " " << map->zen2 << " " << map->dZen << endl;
    QMapIterator<t_frequency::type, t_frqMap*> itFrq(map->frqMap);
    while (itFrq.hasNext()) {
      itFrq.next();
      const t_frqMap* frqMap = itFrq.value();
      cout << "    " << frqMap->neu[0] << " "
                     << frqMap->neu[1] << " "
                     << frqMap->neu[2] << endl;
      cout << "    " << frqMap->pattern.t();
    }
    cout << endl;
  }
}

// Print
////////////////////////////////////////////////////////////////////////////
QString bncAntex::pcoSinexString(const std::string& antName, t_frequency::type frqType) {

  if (antName.find("NULLANTENNA") != string::npos) {
    return QString(" ------ ------ ------");
  }

  QString antNameQ = antName.c_str();
  if (_maps.find(antNameQ) == _maps.end()) {
    return QString(" ------ ------ ------");
  }

  t_antMap* map = _maps[antNameQ];
  if (map->frqMap.find(frqType) == map->frqMap.end()) {
    return QString(" ------ ------ ------");
  }

  t_frqMap* frqMap = map->frqMap[frqType];

  QString u, n,e;
  u.sprintf("%+6.4f" ,frqMap->neu[2]); if (u.mid(1,1) == "0") {u.remove(1,1);}
  n.sprintf("%+6.4f" ,frqMap->neu[0]); if (n.mid(1,1) == "0") {n.remove(1,1);}
  e.sprintf("%+6.4f" ,frqMap->neu[1]); if (e.mid(1,1) == "0") {e.remove(1,1);}

  return QString(" %1 %2 %3").arg(u).arg(n).arg(e);
}

// Read ANTEX File
////////////////////////////////////////////////////////////////////////////
t_irc bncAntex::readFile(const QString& fileName) {

  QFile inFile(fileName);
  inFile.open(QIODevice::ReadOnly | QIODevice::Text);

  QTextStream in(&inFile);

  t_antMap* newAntMap = 0;
  t_frqMap* newFrqMap = 0;

  while ( !in.atEnd() ) {
    QString line = in.readLine();

    // Start of Antenna
    // ----------------
    if      (line.indexOf("START OF ANTENNA") == 60) {
      if (newAntMap) {
        delete newAntMap;
        return failure;
      }
      else {
        delete newAntMap;
        newAntMap = new t_antMap();
      }
    }

    // End of Antenna
    // --------------
    else if (line.indexOf("END OF ANTENNA") == 60) {
      if (newAntMap) {
        if (_maps.contains(newAntMap->antName)) {
          delete _maps[newAntMap->antName];
        }
        _maps[newAntMap->antName] = newAntMap;
        newAntMap = 0;
      }
      else {
        delete newAntMap;
        return failure;
      }
    }

    // Antenna Reading in Progress
    // ---------------------------
    else if (newAntMap) {
      if      (line.indexOf("TYPE / SERIAL NO") == 60) {
        if (line.indexOf("BLOCK I") == 0 ||
            line.indexOf("GLONASS") == 0 ||
            line.indexOf("QZSS") == 0 ||
            line.indexOf("BEIDOU") == 0 ||
            line.indexOf("GALILEO") == 0 ||
            line.indexOf("IRNSS") == 0 ){
          newAntMap->antName = line.mid(20,3);
        }
        else {
          newAntMap->antName = line.mid(0,20);
        }
      }
      else if (line.indexOf("ZEN1 / ZEN2 / DZEN") == 60) {
        QTextStream inLine(&line, QIODevice::ReadOnly);
        inLine >> newAntMap->zen1 >> newAntMap->zen2 >> newAntMap->dZen;
      }

      // Start of Frequency
      // ------------------
      else if (line.indexOf("START OF FREQUENCY") == 60) {
        if (newFrqMap) {
          delete newFrqMap;
          delete newAntMap;
          return failure;
        }
        else {
          newFrqMap = new t_frqMap();
        }
      }

      // End of Frequency
      // ----------------
      else if (line.indexOf("END OF FREQUENCY") == 60) {
        if (newFrqMap) {
          t_frequency::type frqType = t_frequency::dummy;
          // GPS
          if      (line.indexOf("G01") == 3) {
            frqType = t_frequency::G1;
          }
          else if (line.indexOf("G02") == 3) {
            frqType = t_frequency::G2;
          }
          else if (line.indexOf("G05") == 3) {
            frqType = t_frequency::G5;
          }
          // GLONASS
          else if (line.indexOf("R01") == 3) {
            frqType = t_frequency::R1;
          }
          else if (line.indexOf("R02") == 3) {
            frqType = t_frequency::R2;
          }
          // Galileo
          else if (line.indexOf("E01") == 3) {
            frqType = t_frequency::E1;
          }
          else if (line.indexOf("E05") == 3) {
            frqType = t_frequency::E5;
          }
          else if (line.indexOf("E06") == 3) {
            frqType = t_frequency::E6;
          }
          else if (line.indexOf("E07") == 3) {
            frqType = t_frequency::E7;
          }
          else if (line.indexOf("E08") == 3) {
            frqType = t_frequency::E8;
          }
          // QZSS
          else if (line.indexOf("J01") == 3) {
            frqType = t_frequency::J1;
          }
          else if (line.indexOf("J02") == 3) {
            frqType = t_frequency::J2;
          }
          else if (line.indexOf("J05") == 3) {
            frqType = t_frequency::J5;
          }
          else if (line.indexOf("J06") == 3) {
            frqType = t_frequency::J6;
          }
          // BDS
          else if (line.indexOf("C01") == 3) {
            frqType = t_frequency::C1;
          }
          else if (line.indexOf("C02") == 3) {
            frqType = t_frequency::C2;
          }
          else if (line.indexOf("C06") == 3) {
            frqType = t_frequency::C6;
          }
          else if (line.indexOf("C07") == 3) {
            frqType = t_frequency::C7;
          }
          if (frqType != t_frequency::dummy) {
            if (newAntMap->frqMap.find(frqType) != newAntMap->frqMap.end()) {
              delete newAntMap->frqMap[frqType];
            }
            newAntMap->frqMap[frqType] = newFrqMap;
          }
          else {
            delete newFrqMap;
          }
          newFrqMap = 0;
        }
        else {
          delete newAntMap;
          return failure;
        }
      }

      // Frequency Reading in Progress
      // -----------------------------
      else if (newFrqMap) {
        if      (line.indexOf("NORTH / EAST / UP") == 60) {
          QTextStream inLine(&line, QIODevice::ReadOnly);
          inLine >> newFrqMap->neu[0] >> newFrqMap->neu[1] >> newFrqMap->neu[2];
          newFrqMap->neu[0] *= 1e-3;
          newFrqMap->neu[1] *= 1e-3;
          newFrqMap->neu[2] *= 1e-3;
        }
        else if (line.indexOf("NOAZI") == 3) {
          QTextStream inLine(&line, QIODevice::ReadOnly);
          int nPat = int((newAntMap->zen2-newAntMap->zen1)/newAntMap->dZen) + 1;
          newFrqMap->pattern.ReSize(nPat);
          QString dummy;
          inLine >> dummy;
          for (int ii = 0; ii < nPat; ii++) {
            inLine >> newFrqMap->pattern[ii];
          }
          newFrqMap->pattern *= 1e-3;
        }
      }
    }
  }
  inFile.close();
  delete newFrqMap;
  delete newAntMap;

  return success;
}

// Satellite Antenna Offset
////////////////////////////////////////////////////////////////////////////
t_irc bncAntex::satCoMcorrection(const QString& prn, double Mjd,
                                 const ColumnVector& xSat, ColumnVector& dx) {

  t_frequency::type frqType = t_frequency::dummy;

  if      (prn[0] == 'G') {
    frqType = t_frequency::G1;
  }
  else if (prn[0] == 'R') {
    frqType = t_frequency::R1;
  }

  QMap<QString, t_antMap*>::const_iterator it = _maps.find(prn.mid(0,3));
  if (it != _maps.end()) {
    t_antMap* map = it.value();
    if (map->frqMap.find(frqType) != map->frqMap.end()) {

      double* neu = map->frqMap[frqType]->neu;

      // Unit Vectors sz, sy, sx
      // -----------------------
      ColumnVector sz = -xSat;
      sz /= sqrt(DotProduct(sz,sz));

      ColumnVector xSun = BNC_PPP::t_astro::Sun(Mjd);
      xSun /= sqrt(DotProduct(xSun,xSun));

      ColumnVector sy = crossproduct(sz, xSun);
      sy /= sqrt(DotProduct(sy,sy));

      ColumnVector sx = crossproduct(sy, sz);

      dx[0] = sx[0] * neu[0] + sy[0] * neu[1] + sz[0] * neu[2];
      dx[1] = sx[1] * neu[0] + sy[1] * neu[1] + sz[1] * neu[2];
      dx[2] = sx[2] * neu[0] + sy[2] * neu[1] + sz[2] * neu[2];

      return success;
    }
  }

  return failure;
}

//
////////////////////////////////////////////////////////////////////////////
double bncAntex::rcvCorr(const string& antName, t_frequency::type frqType,
                         double eleSat, double azSat, bool& found) const {

  if (antName.find("NULLANTENNA") != string::npos) {
    found = true;
    return 0.0;
  }

  QString antNameQ = antName.c_str();

  if (_maps.find(antNameQ) == _maps.end()) {
    found = false;
    return 0.0;
  }

  t_antMap* map = _maps[antNameQ];
  if (map->frqMap.find(frqType) == map->frqMap.end()) {
    found = false;
    return 0.0;
  }

  t_frqMap* frqMap = map->frqMap[frqType];

  double var = 0.0;
  if (frqMap->pattern.ncols() > 0) {
    double zenDiff = 999.999;
    double zenSat  = 90.0 - eleSat * 180.0 / M_PI;
    unsigned iZen = 0;
    for (double zen = map->zen1; zen <= map->zen2; zen += map->dZen) {
      iZen += 1;
      double newZenDiff = fabs(zen - zenSat);
      if (newZenDiff < zenDiff) {
        zenDiff = newZenDiff;
        var = frqMap->pattern(iZen);
      }
    }
  }

  found = true;
  return var - frqMap->neu[0] * cos(azSat)*cos(eleSat)
             - frqMap->neu[1] * sin(azSat)*cos(eleSat)
             - frqMap->neu[2] * sin(eleSat);

}
