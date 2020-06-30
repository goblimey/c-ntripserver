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

#ifndef SP3COMP_H
#define SP3COMP_H

#include <map>
#include <set>
#include <vector>
#include <string>
#include <sstream>
#include <QtCore>
#include <newmat.h>
#include "bnctime.h"
#include "t_prn.h"

class t_sp3Comp : public QThread {
Q_OBJECT
 
 public:
  t_sp3Comp(QObject* parent);

 protected:
  ~t_sp3Comp();

 signals:
  void finished();
   
 public slots:

 public:
  virtual void run();
 
 private:
  class t_epoch {
   public:
    bncTime                       _tt;
    std::map<t_prn, ColumnVector> _dr;
    std::map<t_prn, ColumnVector> _xyz;
    std::map<t_prn, double>       _dc;
  };

  class t_stat {
   public:
    t_stat() {
      _rao.ReSize(3); 
      _rao    = 0.0;
      _dc     = 0.0;
      _dcRed  = 0.0;
      _offset = 0.0;
      _nr     = 0;
      _nc     = 0;
    }
    ColumnVector _rao;
    double       _dc;
    double       _dcRed;
    double       _offset;
    int          _nr;
    int          _nc;
  };

  int  satIndex(const std::set<t_prn>& clkSats, const t_prn& prn) const;
  void processClocks(const std::set<t_prn>& clkSats, const std::vector<t_epoch*>& epochsIn,
                     std::map<std::string, t_stat>& stat) const;
  void compare(std::ostringstream& out) const;
  bool excludeSat(const t_prn& prn) const;

  QStringList  _sp3FileNames;
  QString      _logFileName;
  QFile*       _logFile;
  QTextStream* _log;
  QStringList  _excludeSats;
};

#endif
