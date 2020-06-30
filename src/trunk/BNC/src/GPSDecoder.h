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

#ifndef GPSDECODER_H
#define GPSDECODER_H

#include <iostream>
#include <vector>
#include <string>

#include <QtCore>

#include "bncconst.h"
#include "bnctime.h"
#include "satObs.h"

class bncRinex;

class GPSDecoder {
 public:
  GPSDecoder();
  virtual ~GPSDecoder();

  virtual t_irc Decode(char* buffer, int bufLen,
                       std::vector<std::string>& errmsg) = 0;


  virtual int corrGPSEpochTime() const {return -1;}

  void initRinex(const QByteArray& staID, const QUrl& mountPoint,
                 const QByteArray& latitude, const QByteArray& longitude,
                 const QByteArray& nmea, const QByteArray& ntripVersion);

  void dumpRinexEpoch(const t_satObs& obs, const QByteArray& format);

  void setRinexReconnectFlag(bool flag);

  struct t_antRefPoint {
    enum t_type { ARP, APC };

    t_antRefPoint() {
      xx = yy = zz = height = 0.0;
      type = ARP;
      height_f = false;
      message  = 0;
    };

    double xx;
    double yy;
    double zz;
    t_type type;
    double height;
    bool   height_f;
    int    message;
  };

  struct t_antInfo {
    t_antInfo() {
    };
    char descriptor[256];
    char serialnumber[256];
  };

  struct t_recInfo {
    t_recInfo() {
    };
    char descriptor[256];
    char serialnumber[256];
    char firmware[256];
  };

  /** List of observations */
  QList<t_satObs>      _obsList;
  QList<int>           _typeList;  // RTCM message types
  QList<t_antInfo>     _antType;   // RTCM antenna descriptor
  QList<t_recInfo>     _recType;   // RTCM receiver descriptor
  QList<t_antRefPoint> _antList;   // RTCM antenna XYZ
  QString              _gloFrq;    // GLONASS slot
  bncRinex*            _rnx;       // RINEX writer
};

#endif
