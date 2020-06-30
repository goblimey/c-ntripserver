
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
 * Class:      t_pppThread, t_pppRun
 *
 * Purpose:    Single PPP Client (running in its own thread)
 *
 * Author:     L. Mervart
 *
 * Created:    29-Jul-2014
 *
 * Changes:
 *
 * -----------------------------------------------------------------------*/


#include <iostream>
#include <iomanip>
#include <string.h>
#include <map>

#include "pppThread.h"
#include "bnccore.h"
#include "bncephuser.h"

using namespace BNC_PPP;
using namespace std;

// Constructor
////////////////////////////////////////////////////////////////////////////
t_pppThread::t_pppThread(const t_pppOptions* opt) : QThread(0) {

  _opt       = opt;
  _pppRun    = 0;

  connect(this, SIGNAL(finished()), this, SLOT(deleteLater()));

  connect(this, SIGNAL(newMessage(QByteArray,bool)),
          BNC_CORE, SLOT(slotMessage(const QByteArray,bool)));
}

// Destructor
////////////////////////////////////////////////////////////////////////////
t_pppThread::~t_pppThread() {
  delete _pppRun;
}

// Run (virtual)
////////////////////////////////////////////////////////////////////////////
void t_pppThread::run() {

  try {
    _pppRun = new t_pppRun(_opt);
    if (_opt->_realTime) {
      QThread::exec();
    }
    else {
      _pppRun->processFiles();
    }
  }
  catch (t_except exc) {
    _pppRun = 0;
    emit newMessage(QByteArray(exc.what().c_str()), true);
  }
}

