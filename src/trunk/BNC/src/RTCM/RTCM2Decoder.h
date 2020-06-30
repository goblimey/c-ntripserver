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

#ifndef INC_RTCM2DECODER_H
#define INC_RTCM2DECODER_H

#include <map>
#include <vector>
#include <list>
#include <stdio.h>

#include "GPSDecoder.h"
#include "RTCM2.h"
#include "RTCM2_2021.h"
#include "ephemeris.h"
#include "bncephuser.h"

class RTCM2Decoder: public GPSDecoder {

  public:
    RTCM2Decoder(const std::string& ID);
    virtual ~RTCM2Decoder();
    virtual t_irc Decode(char* buffer, int bufLen, std::vector<std::string>& errmsg);

    t_irc getStaCrd(double& xx, double& yy, double& zz);

    t_irc getStaCrd(double& xx,  double& yy,  double& zz,
                    double& dx1, double& dy1, double& dz1,
                    double& dx2, double& dy2, double& dz2);

    const rtcm2::RTCM2_2021& msg2021() const { return _msg2021; }

    std::string ID() const { return _ID; }

  private:

    void translateCorr2Obs(std::vector<std::string>& errmsg);

    QMutex             _mutex;
    std::string        _ID;
    std::string        _buffer;
    rtcm2::RTCM2packet _PP;

    // for messages 18, 19 decoding
    rtcm2::RTCM2_Obs   _ObsBlock;

    // for messages 20, 21 decoding
    rtcm2::RTCM2_03    _msg03;
    rtcm2::RTCM2_22    _msg22;
    rtcm2::RTCM2_23    _msg23;
    rtcm2::RTCM2_24    _msg24;
    rtcm2::RTCM2_2021  _msg2021;
    bncEphUser         _ephUser;
};

#endif  // include blocker
