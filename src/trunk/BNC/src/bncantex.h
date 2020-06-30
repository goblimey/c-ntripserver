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

#ifndef BNCANTEX_H
#define BNCANTEX_H

#include <QtCore>
#include <string>
#include <newmat.h>
#include "bncconst.h"
#include "bnctime.h"

class bncAntex {
 public:
  bncAntex(const char* fileName);
  bncAntex();
  ~bncAntex();
  t_irc   readFile(const QString& fileName);
  void    print() const;
  QString pcoSinexString(const std::string& antName, t_frequency::type frqType);
  double  rcvCorr(const std::string& antName, t_frequency::type frqType,
                  double eleSat, double azSat, bool& found) const;
  t_irc   satCoMcorrection(const QString& prn, double Mjd,
                           const ColumnVector& xSat, ColumnVector& dx);

 private:
  class t_frqMap {
   public:
    t_frqMap() {
      for (unsigned ii = 0; ii < 3; ii++) {
        neu[ii] = 0.0;
      }
    }
    double       neu[3];
    ColumnVector pattern;
  };

  class t_antMap {
   public:
    t_antMap() {
      zen1 = 0.0;
      zen2 = 0.0;
      dZen = 0.0;
    }
    ~t_antMap() {
      QMapIterator<t_frequency::type, t_frqMap*> it(frqMap);
      while (it.hasNext()) {
        it.next();
        delete it.value();
      }
    }
    QString                            antName;
    double                             zen1;
    double                             zen2;
    double                             dZen;
    QMap<t_frequency::type, t_frqMap*> frqMap;
    bncTime                            validFrom;
    bncTime                            validTo;
  };

  QMap<QString, t_antMap*> _maps;
};

#endif
