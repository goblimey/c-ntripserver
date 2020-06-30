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
 * Class:      bncRtnetDecoder
 *
 * Purpose:    Implementation of RTNet (SP3-like) output decoder
 *
 * Author:     L. Mervart
 *
 * Created:    28-Mar-2011
 *
 * Changes:
 *
 * -----------------------------------------------------------------------*/

#include <iostream>
#include "bncrtnetdecoder.h"
#include "bncsettings.h"

using namespace std;

// Constructor
////////////////////////////////////////////////////////////////////////
bncRtnetDecoder::bncRtnetDecoder() {
  bncSettings settings;

  // List of upload casters
  // ----------------------
  int iRow = -1;
  QListIterator<QString> it(settings.value("uploadMountpointsOut").toStringList());
  while (it.hasNext()) {
    QStringList hlp = it.next().split(",");
    if (hlp.size() > 6) {
      ++iRow;
      int  outPort = hlp[1].toInt();
      bool CoM     = (hlp[7].toInt() == Qt::Checked);
      int PID = 0;
      if (hlp.size() > 10) {
        PID = hlp[10].toInt();
      }
      int SID = 0;
      if (hlp.size() > 11) {
        SID = hlp[11].toInt();
      }
      int IOD = 0;
      if (hlp.size() > 12) {
        IOD = hlp[12].toInt();
      }
      bncRtnetUploadCaster* newCaster = new bncRtnetUploadCaster(
                                                       hlp[2], hlp[0], outPort,
                                                       hlp[3], hlp[4],
                                                       hlp[5], hlp[6], CoM,
                                                       hlp[8], hlp[9],
                                                       PID, SID, IOD, iRow);
      newCaster->start();
      _casters.push_back(newCaster);
    }
  }
}

// Destructor
////////////////////////////////////////////////////////////////////////
bncRtnetDecoder::~bncRtnetDecoder() {
  for (int ic = 0; ic < _casters.size(); ic++) {
    _casters[ic]->deleteSafely();
  }
}

// Decode Method
////////////////////////////////////////////////////////////////////////
t_irc bncRtnetDecoder::Decode(char* buffer, int bufLen, vector<string>& errmsg) {
  errmsg.clear();
  for (int ic = 0; ic < _casters.size(); ic++) {
    _casters[ic]->decodeRtnetStream(buffer, bufLen);
  }
  return success;
}

