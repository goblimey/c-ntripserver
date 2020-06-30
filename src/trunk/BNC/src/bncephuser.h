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

#ifndef BNCEPHUSER_H
#define BNCEPHUSER_H

#include <deque>
#include <QtCore>
#include <newmat.h>

#include "bncconst.h"
#include "bnctime.h"
#include "bncutils.h"
#include "ephemeris.h"

class bncEphUser : public QObject {
 Q_OBJECT

 public slots:
  void slotNewGPSEph(t_ephGPS);
  void slotNewGlonassEph(t_ephGlo);
  void slotNewGalileoEph(t_ephGal);
  void slotNewSBASEph(t_ephSBAS);
  void slotNewBDSEph(t_ephBDS);

 public:
  bncEphUser(bool connectSlots);
  virtual ~bncEphUser();

  t_irc putNewEph(t_eph* newEph, bool check);

  t_eph* ephLast(const QString& prn) {
    if (_eph.contains(prn)) {
      return _eph[prn].back();
    }
    return 0;
  }

  t_eph* ephPrev(const QString& prn) {
    if (_eph.contains(prn)) {
      unsigned nn = _eph[prn].size();
      if (nn > 1) {
        return _eph[prn].at(nn-2);
      }
    }
    return 0;
  }

  const QList<QString> prnList() {return _eph.keys();}

 protected:
  virtual void ephBufferChanged() {}

 private:
  void checkEphemeris(t_eph* eph, bool realTime);
  QMutex                             _mutex;
  static const unsigned              _maxQueueSize = 6;
  QMap<QString, std::deque<t_eph*> > _eph;
};

#endif
