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
 * Class:      t_rnxNavFile
 *
 * Purpose:    Reads RINEX Navigation File
 *
 * Author:     L. Mervart
 *
 * Created:    24-Jan-2012
 *
 * Changes:
 *
 * -----------------------------------------------------------------------*/

#include <iostream>
#include <newmatio.h>
#include "rnxnavfile.h"
#include "bnccore.h"
#include "bncutils.h"
#include "ephemeris.h"

using namespace std;

// Constructor
////////////////////////////////////////////////////////////////////////////
t_rnxNavFile::t_rnxNavHeader::t_rnxNavHeader() {
  _version = 0.0;
  _glonass = false;
}

// Destructor
////////////////////////////////////////////////////////////////////////////
t_rnxNavFile::t_rnxNavHeader::~t_rnxNavHeader() {
}

// Read Header
////////////////////////////////////////////////////////////////////////////
t_irc t_rnxNavFile::t_rnxNavHeader::read(QTextStream* stream) {
  while (stream->status() == QTextStream::Ok && !stream->atEnd()) {
    QString line = stream->readLine();
    if (line.isEmpty()) {
      continue;
    }
    QString value = line.left(60).trimmed();
    QString key   = line.mid(60).trimmed();
    if      (key == "END OF HEADER") {
      break;
    }
    else if (key == "RINEX VERSION / TYPE") {
      QTextStream in(value.toLatin1(), QIODevice::ReadOnly);
      in >> _version;
      if (value.indexOf("GLONASS") != -1) {
        _glonass = true;
      }
    }
    else if (key == "COMMENT") {
      _comments.append(value.trimmed());
    }
  }

  return success;
}

// Constructor
////////////////////////////////////////////////////////////////////////////
t_rnxNavFile::t_rnxNavFile(const QString& fileName, e_inpOut inpOut) {
  _inpOut = inpOut;
  _stream = 0;
  _file   = 0;
  if (_inpOut == input) {
    openRead(fileName);
  }
  else {
    openWrite(fileName);
  }
}

// Open for input
////////////////////////////////////////////////////////////////////////////
void t_rnxNavFile::openRead(const QString& fileName) {

  _fileName = fileName; expandEnvVar(_fileName);
  _file     = new QFile(_fileName);
  _file->open(QIODevice::ReadOnly | QIODevice::Text);
  _stream = new QTextStream();
  _stream->setDevice(_file);

  _header.read(_stream);
  this->read(_stream);
}

// Open for output
////////////////////////////////////////////////////////////////////////////
void t_rnxNavFile::openWrite(const QString& fileName) {

  _fileName = fileName; expandEnvVar(_fileName);
  _file     = new QFile(_fileName);
  _file->open(QIODevice::WriteOnly | QIODevice::Text);
  _stream = new QTextStream();
  _stream->setDevice(_file);
}

// Destructor
////////////////////////////////////////////////////////////////////////////
t_rnxNavFile::~t_rnxNavFile() {
  close();
  for (unsigned ii = 0; ii < _ephs.size(); ii++) {
    delete _ephs[ii];
  }
}

// Close
////////////////////////////////////////////////////////////////////////////
void t_rnxNavFile::close() {
  delete _stream; _stream = 0;
  delete _file;   _file = 0;
}

// Read File Content
////////////////////////////////////////////////////////////////////////////
void t_rnxNavFile::read(QTextStream* stream) {

  while (stream->status() == QTextStream::Ok && !stream->atEnd()) {
    QString line = stream->readLine();
    if (line.isEmpty()) {
      continue;
    }
    QStringList hlp = line.split(QRegExp("\\s+"), QString::SkipEmptyParts);
    QString prn;
    if (version() >= 3.0) {
      prn = hlp.at(0);
    }
    else {
      if (glonass()) {
        prn = QString("R%1_0").arg(hlp.at(0).toInt(), 2, 10, QChar('0'));
      }
      else {
        prn = QString("G%1_0").arg(hlp.at(0).toInt(), 2, 10, QChar('0'));
      }
    }

    t_eph* eph = 0;
    QStringList lines; lines << line;
    if      (prn[0] == 'G') {
      for (int ii = 1; ii < 8; ii++) {
        lines << stream->readLine();
      }
      eph = new t_ephGPS(version(), lines);
    }
    else if (prn[0] == 'R') {
      for (int ii = 1; ii < 4; ii++) {
        lines << stream->readLine();
      }
      eph = new t_ephGlo(version(), lines);
    }
    else if (prn[0] == 'E') {
      for (int ii = 1; ii < 8; ii++) {
        lines << stream->readLine();
      }
      eph = new t_ephGal(version(), lines);
    }
    else if (prn[0] == 'J') {
      for (int ii = 1; ii < 8; ii++) {
        lines << stream->readLine();
      }
      eph = new t_ephGPS(version(), lines);
    }
    else if (prn[0] == 'S') {
      for (int ii = 1; ii < 4; ii++) {
        lines << stream->readLine();
      }
      eph = new t_ephSBAS(version(), lines);
    }
    else if (prn[0] == 'C') {
      for (int ii = 1; ii < 8; ii++) {
        lines << stream->readLine();
      }
      eph = new t_ephBDS(version(), lines);
    }
    else if (prn[0] == 'I') {
      for (int ii = 1; ii < 8; ii++) {
        lines << stream->readLine();
      }
      eph = new t_ephGPS(version(), lines);
    }
    _ephs.push_back(eph);
  }
}

// Read Next Ephemeris
////////////////////////////////////////////////////////////////////////////
t_eph* t_rnxNavFile::getNextEph(const bncTime& tt,
                                const QMap<QString, unsigned int>* corrIODs) {

  // Get Ephemeris according to IOD
  // ------------------------------
  if (corrIODs) {
    QMapIterator<QString, unsigned int> itIOD(*corrIODs);
    while (itIOD.hasNext()) {
      itIOD.next();
      QString prn = itIOD.key();
      unsigned int iod = itIOD.value();
      vector<t_eph*>::iterator it = _ephs.begin();
      while (it != _ephs.end()) {
        t_eph* eph = *it;
        double dt = eph->TOC() - tt;
        if (dt < 8*3600.0 && QString(eph->prn().toInternalString().c_str()) == prn && eph->IOD() == iod) {
          it = _ephs.erase(it);
          return eph;
        }
        ++it;
      }
    }
  }

  // Get Ephemeris according to time
  // -------------------------------
  else {
    vector<t_eph*>::iterator it = _ephs.begin();
    while (it != _ephs.end()) {
      t_eph* eph = *it;
      double dt = eph->TOC() - tt;
      if (dt < 2*3600.0) {
        it = _ephs.erase(it);
        return eph;
      }
      ++it;
    }
  }

  return 0;
}

//
////////////////////////////////////////////////////////////////////////////
void t_rnxNavFile::writeHeader(const QMap<QString, QString>* txtMap) {

  QString     runBy = BNC_CORE->userName();
  QStringList comments;

  if (txtMap) {
    QMapIterator<QString, QString> it(*txtMap);
    while (it.hasNext()) {
      it.next();
      if      (it.key() == "RUN BY") {
        runBy = it.value();
      }
      else if (it.key() == "COMMENT") {
        comments = it.value().split("\\n", QString::SkipEmptyParts);
      }
    }
  }

  if (version() < 3.0) {
    const QString fmt = glonass() ? "%1           GLONASS navigation data"
                                  : "%1           Navigation data";
    *_stream << QString(fmt)
      .arg(_header._version, 9, 'f', 2)
      .leftJustified(60)
             << "RINEX VERSION / TYPE\n";
  }
  else {
    QString fmt;
    t_eph::e_type sys = satSystem();
    switch(sys) {
      case t_eph::GPS:
        fmt.append("%1           N: GNSS NAV DATA    G: GPS");
        break;
      case t_eph::GLONASS:
        fmt.append("%1           N: GNSS NAV DATA    R: GLONASS");
        break;
      case t_eph::Galileo:
        fmt.append("%1           N: GNSS NAV DATA    E: Galileo");
        break;
      case t_eph::QZSS:
        fmt.append("%1           N: GNSS NAV DATA    J: QZSS");
        break;
      case t_eph::BDS:
        fmt.append("%1           N: GNSS NAV DATA    C: BDS");
        break;
      case t_eph::IRNSS:
        fmt.append("%1           N: GNSS NAV DATA    I: IRNSS");
        break;
      case t_eph::SBAS:
        fmt.append("%1           N: GNSS NAV DATA    S: SBAS");
        break;
      case t_eph::unknown:
        fmt.append("%1           N: GNSS NAV DATA    M: MIXED");
        break;
    }
    *_stream << fmt
      .arg(_header._version, 9, 'f', 2)
      .leftJustified(60)
             << "RINEX VERSION / TYPE\n";
  }

  const QString fmtDate = (version() < 3.0) ? "dd-MMM-yy hh:mm"
                                            : "yyyyMMdd hhmmss UTC";
  *_stream << QString("%1%2%3")
    .arg(BNC_CORE->pgmName(), -20)
    .arg(runBy.trimmed().left(20), -20)
    .arg(QDateTime::currentDateTime().toUTC().toString(fmtDate), -20)
    .leftJustified(60)
           << "PGM / RUN BY / DATE\n";

  QStringListIterator itCmnt(comments);
  while (itCmnt.hasNext()) {
    *_stream << itCmnt.next().trimmed().left(60).leftJustified(60) << "COMMENT\n";
  }

  *_stream << QString()
    .leftJustified(60)
           << "END OF HEADER\n";
}

//
////////////////////////////////////////////////////////////////////////////
void t_rnxNavFile::writeEph(const t_eph* eph) {
  *_stream << eph->toString(version());
}
