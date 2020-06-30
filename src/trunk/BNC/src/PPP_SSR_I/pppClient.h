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

#ifndef PPPCLIENT_H
#define PPPCLIENT_H

#include <vector>
#include <QtCore>

#include "pppInclude.h"
#include "pppOptions.h"
#include "pppFilter.h"
#include "pppUtils.h"

class bncEphUser;
class t_eph;

namespace BNC_PPP {

class t_pppClient : public interface_pppClient {
 public:
  t_pppClient(const t_pppOptions* opt);
  ~t_pppClient();
  void                processEpoch(const std::vector<t_satObs*>& satObs, t_output* output);
  void                putEphemeris(const t_eph* eph);
  void                putTec(const t_vTec* vTec);
  void                putOrbCorrections(const std::vector<t_orbCorr*>& corr);
  void                putClkCorrections(const std::vector<t_clkCorr*>& corr);
  void                putCodeBiases(const std::vector<t_satCodeBias*>& satCodeBias);
  void                putPhaseBiases(const std::vector<t_satPhaseBias*>& satPhaseBias);
  std::ostringstream& log() {return *_log;}
  const t_pppOptions* opt() const {return _opt;}
  void                reset();

 private:
  t_irc getSatPos(const bncTime& tt, const QString& prn, ColumnVector& xc, ColumnVector& vv);
  void  putNewObs(t_satData* satData);
  t_irc cmpToT(t_satData* satData);
  bncEphUser*         _ephUser;
  t_pppOptions*       _opt;
  t_epoData*          _epoData;
  t_pppFilter*        _filter;
  t_pppUtils*         _pppUtils;
  std::ostringstream* _log;
  t_eph*              _newEph;
};

} // namespace

#endif
