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

#ifndef RNXNAVFILE_H
#define RNXNAVFILE_H

#include <queue>
#include <QtCore>
#include "bncconst.h"
#include "bnctime.h"
#include "ephemeris.h"

class t_pppOpt;
class bncPPPclient;
class t_eph;

#define defaultRnxNavVersion2 2.11
#define defaultRnxNavVersion3 3.04

class t_rnxNavFile {

 public:
  enum e_inpOut {input, output};
 private:
  class t_rnxNavHeader {
   public:
    t_rnxNavHeader();
    ~t_rnxNavHeader();
    t_irc  read(QTextStream* stream);
    double _version;
    bool   _glonass;
    t_eph::e_type _satSys;
    QStringList _comments;
  };

 public:
  t_rnxNavFile(const QString& fileName, e_inpOut inpOut);
  ~t_rnxNavFile();
  t_eph* getNextEph(const bncTime& tt, const QMap<QString, unsigned int>* corrIODs);
  const std::vector<t_eph*> ephs() const {return _ephs;}
  double version() const {return _header._version;}
  void   setVersion(double version) {_header._version = version;}
  bool   glonass() const {return _header._glonass;}
  QStringList comments() const {return _header._comments;}
  t_eph::e_type satSystem() const {return _header._satSys;}
  void   setGlonass(bool glo) {_header._glonass = glo;}
  void   setGnssTypeV3(t_eph::e_type sys) {_header._satSys = sys;}
  void   writeHeader(const QMap<QString, QString>* txtMap = 0);
  void   writeEph(const t_eph* eph);

 protected:
  t_rnxNavFile() {};
  void openRead(const QString& fileName);
  void openWrite(const QString& fileName);
  void close();

 private:
  void read(QTextStream* stream);

  e_inpOut            _inpOut;
  QFile*              _file;
  QString             _fileName;
  QTextStream*        _stream;
  std::vector<t_eph*> _ephs;
  t_rnxNavHeader      _header;
};

#endif
