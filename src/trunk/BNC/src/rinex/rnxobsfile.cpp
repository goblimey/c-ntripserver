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
 * Class:      t_rnxObsFile
 *
 * Purpose:    Reads RINEX Observation File
 *
 * Author:     L. Mervart
 *
 * Created:    24-Jan-2012
 *
 * Changes:
 *
 * -----------------------------------------------------------------------*/

#include <iostream>
#include <iomanip>
#include <sstream>
#include "rnxobsfile.h"
#include "bncutils.h"
#include "bnccore.h"
#include "bncsettings.h"

using namespace std;

// Constructor
////////////////////////////////////////////////////////////////////////////
t_rnxObsHeader::t_rnxObsHeader() {
  _usedSystems = "GREJCSI";
  _antNEU.ReSize(3); _antNEU = 0.0;
  _antXYZ.ReSize(3); _antXYZ = 0.0;
  _antBSG.ReSize(3); _antBSG = 0.0;
  _xyz.ReSize(3);    _xyz    = 0.0;
  _version  = 0.0;
  _interval = 0.0;
  for (unsigned iPrn = 1; iPrn <= t_prn::MAXPRN_GPS; iPrn++) {
    _wlFactorsL1[iPrn] = 1;
    _wlFactorsL2[iPrn] = 1;
  }
  bncSettings settings;
  _writeRinexOnlyWithSklObsTypes = settings.value("rnxOnlyWithSKL").toBool();
}

// Destructor
////////////////////////////////////////////////////////////////////////////
t_rnxObsHeader::~t_rnxObsHeader() {
}

// Read Header
////////////////////////////////////////////////////////////////////////////
t_irc t_rnxObsHeader::read(QTextStream* stream, int maxLines) {
  _comments.clear();
  int numLines = 0;

  while ( stream->status() == QTextStream::Ok && !stream->atEnd() ) {
    QString line = stream->readLine(); ++ numLines;
    if (line.isEmpty()) {
      continue;
    }
    if (line.indexOf("END OF FILE") != -1) {
      break;
    }
    QString value = line.mid(0,60).trimmed();
    QString key   = line.mid(60).trimmed();
    if      (key == "END OF HEADER") {
      break;
    }
    else if (key == "RINEX VERSION / TYPE") {
      QTextStream in(value.toLatin1(), QIODevice::ReadOnly);
      in >> _version;
    }
    else if (key == "MARKER NAME") {
      _markerName = value;
    }
    else if (key == "MARKER TYPE") {
      _markerType = value;
    }
    else if (key == "MARKER NUMBER") {
      _markerNumber = line.mid(0,20).trimmed();
    }
    else if (key == "ANT # / TYPE") {
      _antennaNumber = line.mid( 0,20).trimmed();
      _antennaName   = line.mid(20,20).trimmed();
    }
    else if (key == "OBSERVER / AGENCY") {
      _observer = line.mid( 0,20).trimmed();
      _agency   = line.mid(20,40).trimmed();
    }
    else if (key == "REC # / TYPE / VERS") {
      _receiverNumber  = line.mid( 0,20).trimmed();
      _receiverType    = line.mid(20,20).trimmed();
      _receiverVersion = line.mid(40,20).trimmed();
    }
    else if (key == "INTERVAL") {
      QTextStream in(value.toLatin1(), QIODevice::ReadOnly);
      in >> _interval;
    }
    else if (key == "COMMENT") {
      _comments << line.mid(0,60).trimmed();
    }
    else if (key == "WAVELENGTH FACT L1/2") {
      QTextStream in(value.toLatin1(), QIODevice::ReadOnly);
      int wlFactL1 = 0;
      int wlFactL2 = 0;
      int numSat   = 0;
      in >> wlFactL1 >> wlFactL2 >> numSat;
      if (numSat == 0) {
        for (unsigned iPrn = 1; iPrn <= t_prn::MAXPRN_GPS; iPrn++) {
          _wlFactorsL1[iPrn] = wlFactL1;
          _wlFactorsL2[iPrn] = wlFactL2;
        }
      }
      else {
        for (int ii = 0; ii < numSat; ii++) {
          QString prn; in >> prn;
          if (prn[0] == 'G') {
            int iPrn;
            readInt(prn, 1, 2, iPrn);
            _wlFactorsL1[iPrn] = wlFactL1;
            _wlFactorsL2[iPrn] = wlFactL2;
          }
        }
      }
    }
    else if (key == "APPROX POSITION XYZ") {
      QTextStream in(value.toLatin1(), QIODevice::ReadOnly);
      in >> _xyz[0] >> _xyz[1] >> _xyz[2];
    }
    else if (key == "ANTENNA: DELTA H/E/N") {
      QTextStream in(value.toLatin1(), QIODevice::ReadOnly);
      in >> _antNEU[2] >> _antNEU[1] >> _antNEU[0];
    }
    else if (key == "ANTENNA: DELTA X/Y/Z") {
      QTextStream in(value.toLatin1(), QIODevice::ReadOnly);
      in >> _antXYZ[0] >> _antXYZ[1] >> _antXYZ[2];
    }
    else if (key == "ANTENNA: B.SIGHT XYZ") {
      QTextStream in(value.toLatin1(), QIODevice::ReadOnly);
      in >> _antBSG[0] >> _antBSG[1] >> _antBSG[2];
    }
    else if (key == "# / TYPES OF OBSERV") {
      if (_version == 0.0) {
        _version = defaultRnxObsVersion2;
      }
      QTextStream* in = new QTextStream(value.toLatin1(), QIODevice::ReadOnly);
      int nTypes;
      *in >> nTypes;
      char sys0 = _usedSystems[0].toLatin1();
      _obsTypes[sys0].clear();
      for (int ii = 0; ii < nTypes; ii++) {
        if (ii > 0 && ii % 9 == 0) {
          line = stream->readLine(); ++numLines;
          delete in;
          in = new QTextStream(line.left(60).toLatin1(), QIODevice::ReadOnly);
        }
        QString hlp;
        *in >> hlp;
        _obsTypes[sys0].append(hlp);
      }
      for (int ii = 1; ii < _usedSystems.length(); ii++) {
        char sysI = _usedSystems[ii].toLatin1();
        _obsTypes[sysI] = _obsTypes[sys0];
      }
    }
    else if (key == "SYS / # / OBS TYPES") {
      if (_version == 0.0) {
        _version = defaultRnxObsVersion3;
      }
      QTextStream* in = new QTextStream(value.toLatin1(), QIODevice::ReadOnly);
      char sys;
      int nTypes;
      *in >> sys >> nTypes;
      _obsTypes[sys].clear();
      for (int ii = 0; ii < nTypes; ii++) {
        if (ii > 0 && ii % 13 == 0) {
          line = stream->readLine(); ++numLines;
          delete in;
          in = new QTextStream(line.toLatin1(), QIODevice::ReadOnly);
        }
        QString hlp;
        *in >> hlp;
        if (sys == 'C' && _version < 3.03)  {
          hlp.replace('1', '2');
        }
        _obsTypes[sys].push_back(hlp);
      }
      delete in;
    }
    else if (key == "TIME OF FIRST OBS") {
      QTextStream in(value.toLatin1(), QIODevice::ReadOnly);
      int year, month, day, hour, min;
      double sec;
      in >> year >> month >> day >> hour >> min >> sec;
      _startTime.set(year, month, day, hour, min, sec);
    }
    else if (key == "SYS / PHASE SHIFT"){
      QTextStream* in = new QTextStream(value.toLatin1(), QIODevice::ReadOnly);
      char        sys;
      QString     obstype;
      double      shift;
      int         satnum = 0;
      QStringList satList;
      QString     sat;
      *in >> sys >> obstype >> shift >> satnum;
      if (obstype.size()) {
        for (int ii = 0; ii < satnum; ii++) {
          if (ii > 0 && ii % 10 == 0) {
            line = stream->readLine(); ++numLines;
            delete in;
            in = new QTextStream(line.left(60).toLatin1(), QIODevice::ReadOnly);
          }
          *in >> sat;
          satList.append(sat);
        }
        delete in;
      }
      _phaseShifts.insert(sys+obstype, QPair<double, QStringList>(shift, satList));
    }
    else if (key == "GLONASS COD/PHS/BIS"){
      QTextStream in(value.toLatin1(), QIODevice::ReadOnly);
      for (int ii = 0; ii < 4; ii++) {
        QString type;
        double  value;
        in >> type >> value;
        if (type.size())
          _gloBiases[type] = value;
      }
    }
    else if (key == "GLONASS SLOT / FRQ #") {
      QTextStream* in = new QTextStream(value.toLatin1(), QIODevice::ReadOnly);
      int nSlots = 0;
      *in >> nSlots;
      for (int ii = 0; ii < nSlots; ii++) {
        if (ii > 0 && ii % 8 == 0) {
          line = stream->readLine(); ++numLines;
          delete in;
          in = new QTextStream(line.left(60).toLatin1(), QIODevice::ReadOnly);
        }
        QString sat;
        int    slot;
        *in >> sat >> slot;
        t_prn prn;
        prn.set(sat.toStdString());
        if(sat.size())
          _gloSlots[prn] = slot;
      }
      delete in;
    }
    if (maxLines > 0 && numLines == maxLines) {
      break;
    }
  }

  // set default observation types if empty in input file
  // ----------------------------------------------------
  if (_obsTypes.empty()) {
    if (!_writeRinexOnlyWithSklObsTypes) {
      setDefault(_markerName, _version);
    }
    else {
      return failure;
    }
  }

  // Systems used
  // ------------
  _usedSystems.clear();
  QMapIterator<char, QStringList> it(_obsTypes);
  while (it.hasNext()) {
    it.next();
    _usedSystems += QChar(it.key());
  }

  return success;
}

// Set Default Header
////////////////////////////////////////////////////////////////////////////
void t_rnxObsHeader::setDefault(const QString& markerName, int version) {

  _markerName = markerName;

  if (version <= 2) {
    _version = defaultRnxObsVersion2;
  }
  else {
    _version = defaultRnxObsVersion3;
  }

  _comments << "Default set of observation types used";
  _comments.removeDuplicates();

  _obsTypes.clear();
  if (_version < 3.0) {
    _obsTypes['G'] << "C1" << "P1" << "L1" << "S1"
                   << "C2" << "P2" << "L2" << "S2";
    _obsTypes['R'] = _obsTypes['G'];
    _obsTypes['E'] = _obsTypes['G'];
    _obsTypes['J'] = _obsTypes['G'];
    _obsTypes['S'] = _obsTypes['G'];
    _obsTypes['C'] = _obsTypes['G'];
  }
  else {
    _obsTypes['G'] << "C1C" << "L1C"  << "S1C"
                   << "C1W" << "L1W"  << "S1W"
                   << "C2X" << "L2X"  << "S2X"
                   << "C2W" << "L2W"  << "S2W"
                   << "C5X" << "L5X"  << "S5X";

    _obsTypes['J'] << "C1C" << "L1C"  << "S1C"
                   << "C1Z" << "L1Z"  << "S1Z"
                   << "C1X" << "L1X"  << "S1X"
                   << "C2L" << "L2L"  << "S2L"
                   << "C2X" << "L2X"  << "S2X"
                   << "C5Q" << "L5Q"  << "S5Q"
                   << "C5X" << "L5X"  << "S5X"
                   << "C6L" << "L6L"  << "S6L";

    _obsTypes['R'] << "C1C" << "L1C" << "S1C"
                   << "C1P" << "L1P" << "S1P"
                   << "C2C" << "L2C" << "S2C"
                   << "C2P" << "L2P" << "S2P"
                   << "C3I" << "L3I" << "S3I"
                   << "C4X" << "L4X" << "S4X"
                   << "C6X" << "L6X" << "S6X";

    _obsTypes['E'] << "C1C" << "L1C" << "S1C"
                   << "C1X" << "L1X" << "S1X"
                   << "C5Q" << "L5Q" << "S5Q"
                   << "C5X" << "L5X" << "S5X"
                   << "C6C" << "L6C" << "S6C"
                   << "C6X" << "L6X" << "S6X"
                   << "C7Q" << "L7Q" << "S7Q"
                   << "C7X" << "L7X" << "S7X"
                   << "C8Q" << "L8Q" << "S8Q"
                   << "C8X" << "L8X" << "S8X";

    _obsTypes['S'] << "C1C" << "L1C" << "S1C"
                   << "C5I" << "L5I" << "S5I"
                   << "C5Q" << "L5Q" << "S5Q"
                   << "C5X" << "L5X" << "S5X";

    _obsTypes['C'] << "C2I" << "L2I" << "S2I"
                   << "C2Q" << "L2Q" << "S2Q"
                   << "C2X" << "L2X" << "S2X"
                   << "C6I" << "L6I" << "S6I"
                   << "C6Q" << "L6Q" << "S6Q"
                   << "C6X" << "L6X" << "S6X"
                   << "C7I" << "L7I" << "S7I"
                   << "C7Q" << "L7Q" << "S7Q"
                   << "C7X" << "L7X" << "S7X";

    _obsTypes['I'] << "C5A" << "L5A" << "S5A"
                   << "C9A" << "L9A" << "S9A";
  }
}

// Copy header
////////////////////////////////////////////////////////////////////////////
void t_rnxObsHeader::set(const t_rnxObsHeader& header, int version,
                         const QStringList* useObsTypes,
                         const QStringList* phaseShifts,
                         const QStringList* gloBiases,
                         const QStringList* gloSlots) {

  if (version <= 2) {
    _version = defaultRnxObsVersion2;
  }
  else {
    _version = defaultRnxObsVersion3;
  }
  _interval        = header._interval;
  _antennaNumber   = header._antennaNumber;
  _antennaName     = header._antennaName;
  _markerName      = header._markerName;
  _markerNumber    = header._markerNumber;
  _markerType      = header._markerType;
  _antNEU          = header._antNEU;
  _antXYZ          = header._antXYZ;
  _antBSG          = header._antBSG;
  _xyz             = header._xyz;
  _observer        = header._observer;
  _agency          = header._agency;
  _receiverNumber  = header._receiverNumber;
  _receiverType    = header._receiverType;
  _receiverVersion = header._receiverVersion;
  _startTime       = header._startTime;
  _usedSystems     = header._usedSystems;
  _comments        = header._comments;
  _comments.removeDuplicates();

  for (unsigned iPrn = 1; iPrn <= t_prn::MAXPRN_GPS; iPrn++) {
    _wlFactorsL1[iPrn] =  header._wlFactorsL1[iPrn];
    _wlFactorsL2[iPrn] =  header._wlFactorsL2[iPrn];
  }

  // Set observation types
  // ---------------------
  _obsTypes.clear();
  if (!useObsTypes || useObsTypes->size() == 0) {
    if (int(_version) == int(header._version)) {
      _obsTypes = header._obsTypes;
    }
    else {
      if (_version >= 3.0) {
        _comments << "Default set of observation types used";
        _comments.removeDuplicates();
        _obsTypes['G'] << "C1C" << "L1C"  << "S1C"
                       << "C1W" << "L1W"  << "S1W"
                       << "C2X" << "L2X"  << "S2X"
                       << "C2W" << "L2W"  << "S2W"
                       << "C5X" << "L5X"  << "S5X";

        _obsTypes['J'] << "C1C" << "L1C"  << "S1C"
                       << "C1S" << "L1S"  << "S1S"
                       << "C1L" << "L1L"  << "S1L"
                       << "C1X" << "L1X"  << "S1X"
                       << "C2S" << "L2S"  << "S2S"
                       << "C2L" << "L2L"  << "S2L"
                       << "C2X" << "L2X"  << "S2X"
                       << "C5X" << "L5X"  << "S5X";

        _obsTypes['R'] << "C1C" << "L1C" << "S1C"
                       << "C1P" << "L1P" << "S1P"
                       << "C2C" << "L2C" << "S2C"
                       << "C2P" << "L2P" << "S2P";

        _obsTypes['E'] << "C1X" << "L1X" << "S1X"
                       << "C5X" << "L5X" << "S5X"
                       << "C7X" << "L7X" << "S7X"
                       << "C8X" << "L8X" << "S8X";

        _obsTypes['S'] << "C1C" << "L1C" << "S1C"
                       << "C5I" << "L5I" << "S5I"
                       << "C5Q" << "L5Q" << "S5Q";

        _obsTypes['C'] << "C2I" << "L2I" << "S2I"
                       << "C6I" << "L6I" << "S6I"
                       << "C7I" << "L7I" << "S7I";

        _obsTypes['I'] << "C5A" << "L5A" << "S5A"
                       << "C9A" << "L9A" << "S9A";
      }
      else {
        for (int iSys = 0; iSys < header.numSys(); iSys++) {
          char sys = header.system(iSys);
          for (int iType = 0; iType < header.nTypes(sys); iType++) {
            QString type = header.obsType(sys, iType, _version);
            for (int jSys = 0; jSys < _usedSystems.length(); jSys++) {
              char thisSys  = _usedSystems[jSys].toLatin1();
              if (!_obsTypes[thisSys].contains(type)) {
                _obsTypes[thisSys].push_back(type);
              }
            }
          }
        }
      }
    }
  }
  else {
    for (int iType = 0; iType < useObsTypes->size(); iType++) {
      if (useObsTypes->at(iType).indexOf(":") != -1) {
        QStringList hlp = useObsTypes->at(iType).split(":", QString::SkipEmptyParts);
        if (hlp.size() == 2 && hlp[0].length() == 1) {
          if (_version >= 3.0) {
            char    sys  = hlp[0][0].toLatin1();
            QString type = t_rnxObsFile::type2to3(sys, hlp[1]);
            if (!_obsTypes[sys].contains(type)) {
              _obsTypes[sys].push_back(type);
            }
          }
          else {
            for (int iSys = 0; iSys < _usedSystems.length(); iSys++) {
              char    sys  = _usedSystems[iSys].toLatin1();
              QString type = t_rnxObsFile::type3to2(sys, hlp[1]);
              if (!_obsTypes[sys].contains(type)) {
                _obsTypes[sys].push_back(type);
              }
            }
          }
        }
      }
      else {
        for (int iSys = 0; iSys < _usedSystems.length(); iSys++) {
          char    sys  = _usedSystems[iSys].toLatin1();
          QString type = _version >= 3.0 ? t_rnxObsFile::type2to3(sys, useObsTypes->at(iType)) :
                                           t_rnxObsFile::type3to2(sys, useObsTypes->at(iType));
          if (!_obsTypes[sys].contains(type)) {
            _obsTypes[sys].push_back(type);
          }
        }
      }
    }
    _usedSystems.clear();
    QMapIterator<char, QStringList> it(_obsTypes);
    while (it.hasNext()) {
      it.next();
      _usedSystems += QChar(it.key());
    }
  }

  if (_version >= 3.0) {
    // set phase shifts
    if (!phaseShifts ||  phaseShifts->empty()) {
      _phaseShifts = header._phaseShifts;
    }
    else {
      foreach (const QString &str, *phaseShifts) {
        QStringList hlp  = str.split("_", QString::SkipEmptyParts);
        QStringList hlp1 = hlp.last().split(":", QString::SkipEmptyParts);
        QString type = hlp.first();
        double shift = hlp1.first().toDouble();
        hlp1.removeFirst();
        QString satStr = hlp1.join(" ");
        hlp1 = satStr.split(" ");
        QStringList &satList = hlp1;
        QMap<QString, QPair<double, QStringList> >::iterator it = _phaseShifts.find(type);
        if ( it != _phaseShifts.end()) {
          it.value().second.append(satList);
          it.value().second.removeDuplicates();
        }
        else {
          _phaseShifts.insert(type, QPair<double, QStringList>(shift, satList));
        }
      }
    }
    // set GLONASS biases
    if (!gloBiases || gloBiases->empty()) {
      _gloBiases = header._gloBiases;
    }
    else {
      foreach (const QString &str, *gloBiases) {
        QStringList hlp = str.split(":", QString::SkipEmptyParts);
        QString type = hlp.first();;
        double  value = hlp.last().toDouble();
        if (type.size())
          _gloBiases[type] = value;
      }
    }
    // set GLONASS slots
    if (!gloSlots  || gloSlots->empty()) {
      _gloSlots = header._gloSlots;
    }
    else {
      foreach (const QString &str, *gloSlots) {
        QStringList hlp = str.split(":", QString::SkipEmptyParts);
        QString sat = hlp.first();
        int    slot = hlp.last().toInt();
        t_prn prn;
        prn.set(sat.toStdString());
        if(sat.size())
          _gloSlots[prn] = slot;
      }
    }
  }
}

// Write Header
////////////////////////////////////////////////////////////////////////////
void t_rnxObsHeader::write(QTextStream* stream,
                           const QMap<QString, QString>* txtMap) const {

  QStringList newComments;
  QString     runBy = BNC_CORE->userName();

  if (txtMap) {
   QMapIterator<QString, QString> it(*txtMap);
    while (it.hasNext()) {
      it.next();
      if      (it.key() == "RUN BY") {
        runBy = it.value();
      }
      else if ((it.key().indexOf("COMMENT")) != -1) {
        newComments += it.value().split("\\n", QString::SkipEmptyParts);
      }
    }
    newComments.removeDuplicates();
  }

  *stream << QString("%1           OBSERVATION DATA    M")
    .arg(_version, 9, 'f', 2)
    .leftJustified(60)
           << "RINEX VERSION / TYPE\n";

  const QString fmtDate = (_version < 3.0) ? "dd-MMM-yy hh:mm"
                                                  : "yyyyMMdd hhmmss UTC";
  *stream << QString("%1%2%3")
    .arg(BNC_CORE->pgmName(), -20)
    .arg(runBy.trimmed().left(20), -20)
    .arg(QDateTime::currentDateTime().toUTC().toString(fmtDate), -20)
    .leftJustified(60)
           << "PGM / RUN BY / DATE\n";

  QStringListIterator itCmnt(_comments + newComments);
  while (itCmnt.hasNext()) {
    *stream << itCmnt.next().trimmed().left(60).leftJustified(60) << "COMMENT\n";
  }
  QString markerName = _markerName.left(9);
  if (_version < 3.0) {
    markerName = _markerName.left(4);
  }
  *stream << QString("%1")
    .arg(markerName, -60)
    .leftJustified(60)
           << "MARKER NAME\n";


  if (!_markerNumber.isEmpty()) {
    *stream << QString("%1")
      .arg(_markerNumber, -20)
      .leftJustified(60)
             << "MARKER NUMBER\n";
  }

  if (_version >= 3.0) {
    *stream << QString("%1")
      .arg(_markerType, -60)
      .leftJustified(60)
             << "MARKER TYPE\n";
  }

  *stream << QString("%1%2")
    .arg(_observer, -20)
    .arg(_agency,   -40)
    .leftJustified(60)
           << "OBSERVER / AGENCY\n";

  *stream << QString("%1%2%3")
    .arg(_receiverNumber,  -20)
    .arg(_receiverType,    -20)
    .arg(_receiverVersion, -20)
    .leftJustified(60)
           << "REC # / TYPE / VERS\n";

  *stream << QString("%1%2")
    .arg(_antennaNumber, -20)
    .arg(_antennaName,   -20)
    .leftJustified(60)
           << "ANT # / TYPE\n";

  *stream << QString("%1%2%3")
    .arg(_xyz(1), 14, 'f', 4)
    .arg(_xyz(2), 14, 'f', 4)
    .arg(_xyz(3), 14, 'f', 4)
    .leftJustified(60)
           << "APPROX POSITION XYZ\n";

  *stream << QString("%1%2%3")
    .arg(_antNEU(3), 14, 'f', 4)
    .arg(_antNEU(2), 14, 'f', 4)
    .arg(_antNEU(1), 14, 'f', 4)
    .leftJustified(60)
           << "ANTENNA: DELTA H/E/N\n";

  if (_version < 3.0) {
    int defaultWlFact1 = _wlFactorsL1[1];
    int defaultWlFact2 = _wlFactorsL2[1];  // TODO check all prns
    *stream << QString("%1%2")
      .arg(defaultWlFact1, 6)
      .arg(defaultWlFact2, 6)
      .leftJustified(60)
             << "WAVELENGTH FACT L1/2\n";
  }

  *stream << obsTypesStrings().join("");

  if (_interval > 0) {
    *stream << QString("%1")
      .arg(_interval, 10, 'f', 3)
      .leftJustified(60)
             << "INTERVAL\n";
  }

  unsigned year, month, day, hour, min;
  double sec;
  _startTime.civil_date(year, month, day);
  _startTime.civil_time(hour, min, sec);
  *stream << QString("%1%2%3%4%5%6%7")
    .arg(year, 6)
    .arg(month, 6)
    .arg(day, 6)
    .arg(hour, 6)
    .arg(min, 6)
    .arg(sec, 13, 'f', 7)
    .arg("GPS", 8)
    .leftJustified(60)
           << "TIME OF FIRST OBS\n";

  if (_version >= 3.0) {
    if (_phaseShifts.empty()) {
      QMap<char, QStringList>::const_iterator it;
      for (it = _obsTypes.begin(); it != _obsTypes.end(); ++it) {
        char sys = it.key();
        double shift = 0.0;
        foreach (const QString &obstype, it.value()) {
          if (obstype.left(1).contains('L')) {
          *stream << QString("%1%2%3")
            .arg(sys, 0)
            .arg(obstype, 4)
            .arg(shift, 9, 'f', 5)
            .leftJustified(60)
             << "SYS / PHASE SHIFT\n";
          }
        }
      }
    } else {
      QMap<QString, QPair<double, QStringList> >::const_iterator it;
      QString emptyFillStr;
      for(it = _phaseShifts.begin(); it!= _phaseShifts.end(); ++it) {
        QString sys         = it.key().left(1);
        QString obstype     = it.key().mid(1);
        double shift        = it.value().first;
        QStringList satList = it.value().second;
        QString hlp;
        if (obstype.isEmpty()) {
          hlp = QString("%1")
            .arg(sys.toStdString().c_str(), 0);
        }
        else {
          hlp = QString("%1%2%3")
            .arg(sys.toStdString().c_str(), 0)
            .arg(obstype, 4)
            .arg(shift, 9, 'f', 5);
        }
        if (!satList.empty()) {
          hlp += QString("%1").arg(satList.size(), 4);
        }
        else {
          *stream << QString("%1")
            .arg(hlp, 0)
            .leftJustified(60)
             << "SYS / PHASE SHIFT\n";
          hlp = "";
        }
        int ii = 0;
        QStringList::const_iterator it_s;
        for (it_s = satList.begin(); it_s != satList.end(); ++it_s) {
          (hlp.contains(obstype)) ?
            emptyFillStr = "": emptyFillStr = "                  ";
          hlp += QString("%1").arg(*it_s, 4);
          ii++;
          if (ii % 10 == 0) {
            *stream << QString("%1%2")
              .arg(emptyFillStr, 0)
              .arg(hlp, 0)
              .leftJustified(60)
              << "SYS / PHASE SHIFT\n";
            hlp = "";
          }
        }
        if (hlp.size()) {
          (hlp.contains(obstype)) ?
            emptyFillStr = "": emptyFillStr = "                  ";
        *stream <<  QString("%1%2")
          .arg(emptyFillStr, 0)
          .arg(hlp, 0)
          .leftJustified(60)
          << "SYS / PHASE SHIFT\n";
        }
      }
    }
  }

  if (_version >= 3.0) {
    QString hlp = "";
    QMap<QString, double>::const_iterator it = _gloBiases.begin();
    while (it != _gloBiases.end()){
      hlp += QString("%1%2").arg(it.key(), 4).arg(it.value(), 9, 'f', 3);
      it++;
    }
    *stream << QString("%1")
      .arg(hlp, 0)
      .leftJustified(60)
           << "GLONASS COD/PHS/BIS\n";
  }

  if (_version >= 3.0) {
    QString number = QString::number(_gloSlots.size());
    QString hlp = "";
    int ii = 0;
    QMap<t_prn, int>::const_iterator it = _gloSlots.begin();
    while (it != _gloSlots.end()) {
      QString prn(it.key().toString().c_str());
      hlp +=  QString("%1%2").arg(prn, 4).arg(it.value(), 3);
      it++;
      ii++;
      if (ii % 8 == 0) {
        *stream << QString("%1%2")
          .arg(number, 3)
          .arg(hlp, 0)
          .leftJustified(60)
           << "GLONASS SLOT / FRQ #\n";
        ii = 0;
        hlp = number = "";
      }
    }
    if (hlp.size() || !_gloSlots.size()) {
      *stream << QString("%1%2")
            .arg(number, 3)
            .arg(hlp, 0)
            .leftJustified(60)
            << "GLONASS SLOT / FRQ #\n";
    }
  }

  *stream << QString()
    .leftJustified(60)
           << "END OF HEADER\n";
}

// Number of Different Systems
////////////////////////////////////////////////////////////////////////////
int t_rnxObsHeader::numSys() const {
  return _obsTypes.size();
}

//
////////////////////////////////////////////////////////////////////////////
char t_rnxObsHeader::system(int iSys) const {
  int iSysLocal = -1;
  QMapIterator<char, QStringList> it(_obsTypes);
  while (it.hasNext()) {
    ++iSysLocal;
    it.next();
    if (iSysLocal == iSys) {
      return it.key();
    }
  }
  return ' ';
}

//
////////////////////////////////////////////////////////////////////////////
QString t_rnxObsHeader::usedSystems(void) const {
  return _usedSystems;
}

QStringList t_rnxObsHeader::obsTypes(char sys) const {
  if (_obsTypes.contains(sys)) {
    return _obsTypes[sys];
  }
  else {
    return QStringList();
  }
}

// Number of Observation Types (satellite-system specific)
////////////////////////////////////////////////////////////////////////////
int t_rnxObsHeader::nTypes(char sys) const {
  if (_obsTypes.contains(sys)) {
    return _obsTypes[sys].size();
  }
  else {
    return 0;
  }
}

// Number of GLONASS biases
////////////////////////////////////////////////////////////////////////////
int t_rnxObsHeader::numGloBiases() const {
  return _gloBiases.size();
}

// Number of GLONASS slots
////////////////////////////////////////////////////////////////////////////
int t_rnxObsHeader::numGloSlots() const {
  return _gloSlots.size();
}

// Observation Type (satellite-system specific)
////////////////////////////////////////////////////////////////////////////
QString t_rnxObsHeader::obsType(char sys, int index, double version) const {

  if (version == 0.0) {
    version = _version;
  }
  if (_obsTypes.contains(sys)) {
    QString origType = _obsTypes[sys].at(index);
    if      (int(version) == int(_version)) {
      return origType;
    }
    else if (int(version) == 2) {
      return t_rnxObsFile::type3to2(sys, origType);
    }
    else if (int(version) == 3) {
      return t_rnxObsFile::type2to3(sys, origType);
    }
  }
  return "";
}

//
////////////////////////////////////////////////////////////////////////////
QStringList t_rnxObsHeader::phaseShifts() const {
  QStringList strList;
  QMap<QString, QPair<double, QStringList> >::const_iterator it =  _phaseShifts.begin();
  while (it != _phaseShifts.end()) {
    strList.append(QString("%1_%2:%3").arg(it.key(), 3).arg(it.value().first, 9, 'f', 3).arg(it.value().second.join("")));
    it++;
  }
  return strList;
}

//
////////////////////////////////////////////////////////////////////////////
QStringList t_rnxObsHeader::gloBiases() const {
  QStringList strList;
  QMap<QString, double>::const_iterator it = _gloBiases.begin();
  while (it != _gloBiases.end()) {
    strList.append(QString("%1:%2").arg(it.key(), 3).arg(it.value(), 9, 'f', 3));
    it++;
  }
  return strList;
}

//
////////////////////////////////////////////////////////////////////////////
QStringList t_rnxObsHeader::gloSlots() const {
  QStringList strList;
  QMap<t_prn, int>::const_iterator it = _gloSlots.begin();
  while (it != _gloSlots.end()){
    QString prn(it.key().toString().c_str());
    strList.append(QString("%1:%2").arg(prn, 3).arg(it.value()));
    it++;
  }
  return strList;
}

// Write Observation Types
////////////////////////////////////////////////////////////////////////////
QStringList t_rnxObsHeader::obsTypesStrings() const {

  QStringList strList;
  if (_version < 3.0) {
    char sys0 = _usedSystems[0].toLatin1();
    QString hlp;
    QTextStream(&hlp) << QString("%1").arg(_obsTypes[sys0].size(), 6);
    for (int ii = 0; ii < _obsTypes[sys0].size(); ii++) {
      QTextStream(&hlp) << QString("%1").arg(_obsTypes[sys0][ii], 6);
      if ((ii+1) % 9 == 0 || ii == _obsTypes[sys0].size()-1) {
        strList.append(hlp.leftJustified(60) + "# / TYPES OF OBSERV\n");
        hlp = QString().leftJustified(6);
      }
    }
  }
  else {
    for (int iSys = 0; iSys < numSys(); iSys++) {
      char sys = system(iSys);
      QString hlp;
      QTextStream(&hlp) << QString("%1  %2").arg(sys).arg(nTypes(sys), 3);
      for (int iType = 0; iType < nTypes(sys); iType++) {
        QString type = obsType(sys, iType);
        QTextStream(&hlp) << QString(" %1").arg(type, -3);
        if ((iType+1) % 13 == 0 || iType == nTypes(sys)-1) {
          strList.append(hlp.leftJustified(60) + "SYS / # / OBS TYPES\n");
          hlp = QString().leftJustified(6);
        }
      }
    }
  }

  return strList;
}

// Constructor
////////////////////////////////////////////////////////////////////////////
t_rnxObsFile::t_rnxObsFile(const QString& fileName, e_inpOut inpOut) {
  _inpOut       = inpOut;
  _stream       = 0;
  _flgPowerFail = false;
  if (_inpOut == input) {
    openRead(fileName);
  }
  else {
    openWrite(fileName);
  }
}

// Open for input
////////////////////////////////////////////////////////////////////////////
void t_rnxObsFile::openRead(const QString& fileName) {

  _fileName = fileName; expandEnvVar(_fileName);
  _file     = new QFile(_fileName);
  _file->open(QIODevice::ReadOnly | QIODevice::Text);
  _stream = new QTextStream();
  _stream->setDevice(_file);

  _header.read(_stream);

  // Guess Observation Interval
  // --------------------------
  if (_header._interval == 0.0) {
    bncTime ttPrev;
    for (int iEpo = 0; iEpo < 10; iEpo++) {
      const t_rnxEpo* rnxEpo = nextEpoch();
      if (!rnxEpo) {
        throw QString("t_rnxObsFile: not enough epochs");
      }
      if (iEpo > 0) {
        double dt = rnxEpo->tt - ttPrev;
        if (_header._interval == 0.0 || dt < _header._interval) {
          _header._interval = dt;
        }
      }
      ttPrev = rnxEpo->tt;
    }
    _stream->seek(0);
    _header.read(_stream);
  }

  // Time of first observation
  // -------------------------
  if (!_header._startTime.valid()) {
    const t_rnxEpo* rnxEpo = nextEpoch();
    if (!rnxEpo) {
      throw QString("t_rnxObsFile: not enough epochs");
    }
    _header._startTime = rnxEpo->tt;
    _stream->seek(0);
    _header.read(_stream);
  }
}

// Open for output
////////////////////////////////////////////////////////////////////////////
void t_rnxObsFile::openWrite(const QString& fileName) {

  _fileName = fileName; expandEnvVar(_fileName);
  _file     = new QFile(_fileName);
  _file->open(QIODevice::WriteOnly | QIODevice::Text);
  _stream = new QTextStream();
  _stream->setDevice(_file);
}

// Destructor
////////////////////////////////////////////////////////////////////////////
t_rnxObsFile::~t_rnxObsFile() {
  close();
}

// Close
////////////////////////////////////////////////////////////////////////////
void t_rnxObsFile::close() {
  delete _stream; _stream = 0;
  delete _file;   _file = 0;
}

// Handle Special Epoch Flag
////////////////////////////////////////////////////////////////////////////
void t_rnxObsFile::handleEpochFlag(int flag, const QString& line,
                                   bool& headerReRead) {

  headerReRead = false;

  // Power Failure
  // -------------
  if      (flag == 1) {
    _flgPowerFail = true;
  }

  // Start moving antenna
  // --------------------
  else if (flag == 2) {
    // no action
  }

  // Re-Read Header
  // --------------
  else if (flag == 3 || flag == 4 || flag == 5) {
    int numLines = 0;
    if (version() < 3.0) {
      readInt(line, 29, 3, numLines);
    }
    else {
      readInt(line, 32, 3, numLines);
    }
    if (flag == 3 || flag == 4) {
      _header.read(_stream, numLines);
      headerReRead = true;
    }
    else {
      for (int ii = 0; ii < numLines; ii++) {
        _stream->readLine();
      }
    }
  }

  // Unhandled Flag
  // --------------
  else {
    throw QString("t_rnxObsFile: unhandled flag\n" + line);
  }
}

// Retrieve single Epoch
////////////////////////////////////////////////////////////////////////////
t_rnxObsFile::t_rnxEpo* t_rnxObsFile::nextEpoch() {
  _currEpo.clear();
  if (version() < 3.0) {
    return nextEpochV2();
  }
  else {
    return nextEpochV3();
  }
}

// Retrieve single Epoch (RINEX Version 3)
////////////////////////////////////////////////////////////////////////////
t_rnxObsFile::t_rnxEpo* t_rnxObsFile::nextEpochV3() {

  while ( _stream->status() == QTextStream::Ok && !_stream->atEnd() ) {

    QString line = _stream->readLine();

    if (line.isEmpty()) {
      continue;
    }

    int flag = 0;
    readInt(line, 31, 1, flag);
    if (flag > 0) {
      bool headerReRead = false;
      handleEpochFlag(flag, line, headerReRead);
      if (headerReRead) {
        continue;
      }
    }

    QTextStream in(line.mid(1).toLatin1(), QIODevice::ReadOnly);

    // Epoch Time
    // ----------
    int    year, month, day, hour, min;
    double sec;
    in >> year >> month >> day >> hour >> min >> sec;
    _currEpo.tt.set(year, month, day, hour, min, sec);

    // Number of Satellites
    // --------------------
    int numSat;
    readInt(line, 32, 3, numSat);

    _currEpo.rnxSat.resize(numSat);

    // Observations
    // ------------
    for (int iSat = 0; iSat < numSat; iSat++) {
      line = _stream->readLine();
      t_prn prn; prn.set(line.left(3).toLatin1().data());
      _currEpo.rnxSat[iSat].prn = prn;
      char sys = prn.system();
      for (int iType = 0; iType < _header.nTypes(sys); iType++) {
        int pos = 3 + 16*iType;
        double obsValue = 0.0;
        int    lli      = 0;
        int    snr      = 0;
        readDbl(line, pos,     14, obsValue);
        readInt(line, pos + 14, 1, lli);
        readInt(line, pos + 15, 1, snr);
        if (_flgPowerFail) {
          lli |= 1;
        }
        QString type = obsType(sys, iType);
        _currEpo.rnxSat[iSat].obs[type].value = obsValue;
        _currEpo.rnxSat[iSat].obs[type].lli   = lli;
        _currEpo.rnxSat[iSat].obs[type].snr   = snr;
      }
    }

    _flgPowerFail = false;

    return &_currEpo;
  }

  return 0;
}

// Retrieve single Epoch (RINEX Version 2)
////////////////////////////////////////////////////////////////////////////
t_rnxObsFile::t_rnxEpo* t_rnxObsFile::nextEpochV2() {

  while ( _stream->status() == QTextStream::Ok && !_stream->atEnd() ) {

    QString line = _stream->readLine();

    if (line.isEmpty()) {
      continue;
    }

    int flag = 0;
    readInt(line, 28, 1, flag);
    if (flag > 0) {
      bool headerReRead = false;
      handleEpochFlag(flag, line, headerReRead);
      if (headerReRead) {
        continue;
      }
    }

    QTextStream in(line.toLatin1(), QIODevice::ReadOnly);

    // Epoch Time
    // ----------
    int    year, month, day, hour, min;
    double sec;
    in >> year >> month >> day >> hour >> min >> sec;
    if      (year <  80) {
      year += 2000;
    }
    else if (year < 100) {
      year += 1900;
    }
    _currEpo.tt.set(year, month, day, hour, min, sec);

    // Number of Satellites
    // --------------------
    int numSat;
    readInt(line, 29, 3, numSat);

    _currEpo.rnxSat.resize(numSat);

    // Read Satellite Numbers
    // ----------------------
    int pos = 32;
    for (int iSat = 0; iSat < numSat; iSat++) {
      if (iSat > 0 && iSat % 12 == 0) {
        line = _stream->readLine();
        pos = 32;
      }

      char sys = line.toLatin1()[pos];
      if (sys == ' ') {
        sys = 'G';
      }
      int satNum; readInt(line, pos + 1, 2, satNum);
      _currEpo.rnxSat[iSat].prn.set(sys, satNum);

      pos += 3;
    }

    // Read Observation Records
    // ------------------------
    for (int iSat = 0; iSat < numSat; iSat++) {
      char sys = _currEpo.rnxSat[iSat].prn.system();
      line = _stream->readLine();
      pos  = 0;
      for (int iType = 0; iType < _header.nTypes(sys); iType++) {
        if (iType > 0 && iType % 5 == 0) {
          line = _stream->readLine();
          pos  = 0;
        }
        double obsValue = 0.0;
        int    lli      = 0;
        int    snr      = 0;
        readDbl(line, pos,     14, obsValue);
        readInt(line, pos + 14, 1, lli);
        readInt(line, pos + 15, 1, snr);

        if (_flgPowerFail) {
          lli |= 1;
        }

        QString type = obsType(sys, iType);
        _currEpo.rnxSat[iSat].obs[type].value = obsValue;
        _currEpo.rnxSat[iSat].obs[type].lli   = lli;
        _currEpo.rnxSat[iSat].obs[type].snr   = snr;

        pos += 16;
      }
    }

    _flgPowerFail = false;

    return &_currEpo;
  }

  return 0;
}

// Write Data Epoch
////////////////////////////////////////////////////////////////////////////
void t_rnxObsFile::writeEpoch(const t_rnxEpo* epo) {
  if (epo == 0) {
    return;
  }
  t_rnxEpo epoLocal;
  epoLocal.tt = epo->tt;
  for (unsigned ii = 0; ii < epo->rnxSat.size(); ii++) {
    const t_rnxSat& rnxSat = epo->rnxSat[ii];
    if (_header._obsTypes[rnxSat.prn.system()].size() > 0) {
      if (_header.version() < 3.0) { // exclude new GNSS such as BDS, QZSS, IRNSS, etc.
          if (rnxSat.prn.system() != 'G' && rnxSat.prn.system() != 'R' &&
              rnxSat.prn.system() != 'E' && rnxSat.prn.system() != 'S' &&
              rnxSat.prn.system() != 'I') {
            continue;
          }
      }
      epoLocal.rnxSat.push_back(rnxSat);
    }
  }
  std::stable_sort(epoLocal.rnxSat.begin(), epoLocal.rnxSat.end(), t_rnxSat::prnSort);

  if (version() < 3.0) {
    return writeEpochV2(_stream, _header, &epoLocal);
  }
  else {
    return writeEpochV3(_stream, _header, &epoLocal);
  }
}

// Write Data Epoch (RINEX Version 2)
////////////////////////////////////////////////////////////////////////////
void t_rnxObsFile::writeEpochV2(QTextStream* stream, const t_rnxObsHeader& header,
                                const t_rnxEpo* epo) {

  unsigned year, month, day, hour, min;
  double sec;
  epo->tt.civil_date(year, month, day);
  epo->tt.civil_time(hour, min, sec);

  QString dateStr;
  QTextStream(&dateStr) << QString(" %1 %2 %3 %4 %5%6")
    .arg(int(fmod(double(year), 100.0)), 2, 10, QChar('0'))
    .arg(month,                          2, 10, QChar('0'))
    .arg(day,                            2, 10, QChar('0'))
    .arg(hour,                           2, 10, QChar('0'))
    .arg(min,                            2, 10, QChar('0'))
    .arg(sec,                           11, 'f', 7);

  int flag = 0;
  *stream << dateStr << QString("%1%2").arg(flag, 3).arg(epo->rnxSat.size(), 3);
  for (unsigned iSat = 0; iSat < epo->rnxSat.size(); iSat++) {
    const t_rnxSat& rnxSat = epo->rnxSat[iSat];
    if (iSat > 0 && iSat % 12 == 0) {
      *stream << endl << QString().leftJustified(32);
    }
    *stream << rnxSat.prn.toString().c_str();
  }
  *stream << endl;

  for (unsigned iSat = 0; iSat < epo->rnxSat.size(); iSat++) {
    const t_rnxSat& rnxSat = epo->rnxSat[iSat];
    char            sys    = rnxSat.prn.system();
    for (int iTypeV2 = 0; iTypeV2 < header.nTypes(sys); iTypeV2++) {
      if (iTypeV2 > 0 && iTypeV2 % 5 == 0) {
        *stream << endl;
      }
      QString typeV2 = header.obsType(sys, iTypeV2);
      bool    found  = false;
      QStringList preferredAttribList = signalPriorities(sys);
      QString preferredAttrib;
      for (int ii = 0; ii < preferredAttribList.size(); ii++) {
        if (preferredAttribList[ii].indexOf("&") != -1) {
          QStringList hlp = preferredAttribList[ii].split("&", QString::SkipEmptyParts);
          if (hlp.size() == 2 && hlp[0].contains(typeV2[1])) {
            preferredAttrib = hlp[1];
          }
        }
        else {
          preferredAttrib = preferredAttribList[ii];
        }
      }

      for (int iPref = 0; iPref < preferredAttrib.size(); iPref++) {
        QMapIterator<QString, t_rnxObs> itObs(rnxSat.obs);
        while (itObs.hasNext()) {
          itObs.next();
          const QString&  type   = itObs.key();
          const t_rnxObs& rnxObs = itObs.value();
          if ( preferredAttrib[iPref] == '?'                             ||
               (type.length() == 2 && preferredAttrib[iPref] == '_'    ) ||
               (type.length() == 3 && preferredAttrib[iPref] == type[2]) ) {
            if (typeV2 == type3to2(sys, type)) {
              found = true;
              if (rnxObs.value == 0.0) {
                *stream << QString().leftJustified(16);
              }
              else {
                *stream << QString("%1").arg(rnxObs.value, 14, 'f', 3);
                if (rnxObs.lli != 0.0) {
                  *stream << QString("%1").arg(rnxObs.lli,1);
                }
                else {
                  *stream << ' ';
                }
                if (rnxObs.snr != 0.0) {
                  *stream << QString("%1").arg(rnxObs.snr,1);
                }
                else {
                  *stream << ' ';
                }
              }
              goto end_loop_iPref;
            }
          }
        }
      } end_loop_iPref:
      if (!found) {
        *stream << QString().leftJustified(16);
      }
    }
    *stream << endl;
  }
}

// Write Data Epoch (RINEX Version 3)
////////////////////////////////////////////////////////////////////////////
void t_rnxObsFile::writeEpochV3(QTextStream* stream, const t_rnxObsHeader& header,
                                const t_rnxEpo* epo) {

  unsigned year, month, day, hour, min;
  double sec;
  epo->tt.civil_date(year, month, day);
  epo->tt.civil_time(hour, min, sec);

  QString dateStr;
  QTextStream(&dateStr) << QString("> %1 %2 %3 %4 %5%6")
      .arg(year, 4)
      .arg(month, 2, 10, QChar('0'))
      .arg(day, 2, 10, QChar('0'))
      .arg(hour, 2, 10, QChar('0'))
      .arg(min, 2, 10, QChar('0'))
      .arg(sec, 11, 'f', 7);

  int flag = 0;
  *stream << dateStr << QString("%1%2\n").arg(flag, 3).arg(epo->rnxSat.size(), 3);

  for (unsigned iSat = 0; iSat < epo->rnxSat.size(); iSat++) {
    const t_rnxSat& rnxSat = epo->rnxSat[iSat];
    char sys = rnxSat.prn.system();

    const t_rnxObs* hlp[header.nTypes(sys)];
    for (int iTypeV3 = 0; iTypeV3 < header.nTypes(sys); iTypeV3++) {
      hlp[iTypeV3] = 0;
      QString typeV3 = header.obsType(sys, iTypeV3);
      QMapIterator<QString, t_rnxObs> itObs(rnxSat.obs);

      // Exact match
      // -----------
      while (itObs.hasNext()) {
        itObs.next();
        const QString& type = itObs.key();
        const t_rnxObs& rnxObs = itObs.value();
        if (typeV3 == type2to3(sys, type) && rnxObs.value != 0.0) {
          hlp[iTypeV3] = &itObs.value();
        }
      }

      // Non-Exact match
      // ---------------
      itObs.toFront();
      while (itObs.hasNext()) {
        itObs.next();
        const QString& type = itObs.key();
        const t_rnxObs& rnxObs = itObs.value();
        if (hlp[iTypeV3] == 0 && typeV3 == type2to3(sys, type).left(2) && rnxObs.value != 0.0) {
          hlp[iTypeV3] = &itObs.value();
        }
      }
    }

    if (header.nTypes(sys)) {
      *stream << rnxSat.prn.toString().c_str();
      for (int iTypeV3 = 0; iTypeV3 < header.nTypes(sys); iTypeV3++) {
        const t_rnxObs* rnxObs = hlp[iTypeV3];
        if (rnxObs == 0) {
          *stream << QString().leftJustified(16);
        }
        else {
          *stream << QString("%1").arg(rnxObs->value, 14, 'f', 3);
          if (rnxObs->lli != 0.0) {
            *stream << QString("%1").arg(rnxObs->lli, 1);
          }
          else {
            *stream << ' ';
          }
          if (rnxObs->snr != 0.0) {
            *stream << QString("%1").arg(rnxObs->snr, 1);
          }
          else {
            *stream << ' ';
          }
        }
      }
      *stream << endl;
    }
  }
}

// Translate Observation Type v2 --> v3
////////////////////////////////////////////////////////////////////////////
QString t_rnxObsFile::type2to3(char sys, const QString& typeV2) {
  if      (typeV2 == "P1") {
    return (sys == 'G') ? "C1W" : "C1P";
  }
  else if (typeV2 == "P2") {
    return (sys == 'G') ? "C2W" : "C2P";
  }
  return typeV2;
}

// Translate Observation Type v3 --> v2
////////////////////////////////////////////////////////////////////////////
QString t_rnxObsFile::type3to2(char /* sys */, const QString& typeV3) {
  if      (typeV3 == "C1P" || typeV3 == "C1W") {
    return "P1";
  }
  else if (typeV3 == "C2P" || typeV3 == "C2W") {
    return "P2";
  }
  return typeV3.left(2);
}

// Set Observations from RINEX File
////////////////////////////////////////////////////////////////////////////
void t_rnxObsFile::setObsFromRnx(const t_rnxObsFile* rnxObsFile, const t_rnxObsFile::t_rnxEpo* epo,
                                 const t_rnxObsFile::t_rnxSat& rnxSat, t_satObs& obs) {
  obs._staID = rnxObsFile->markerName().toLatin1().constData();
  obs._prn   = rnxSat.prn;
  obs._time  = epo->tt;

  char sys   = rnxSat.prn.system();

  QChar addToL2;
  for (int iType = 0; iType < rnxObsFile->nTypes(sys); iType++) {
    QString type   = rnxObsFile->obsType(sys, iType);
    QString typeV3 = rnxObsFile->obsType(sys, iType, 3.0); // may or may not differ from type
    if (rnxSat.obs.contains(type) && rnxSat.obs[type].value != 0.0) {
      if (type == "P2" && typeV3.length() > 2) {
        addToL2 = typeV3[2];
        break;
      }
    }
  }

  for (int iType = 0; iType < rnxObsFile->nTypes(sys); iType++) {
    QString type   = rnxObsFile->obsType(sys, iType);
    QString typeV3 = rnxObsFile->obsType(sys, iType, 3.0); // may or may not differ from type
    if (type == "L2") {
      typeV3 += addToL2;
    }
    if (rnxSat.obs.contains(type)) {
      const t_rnxObs& rnxObs = rnxSat.obs[type];
      if (rnxObs.value != 0.0) {
        string type2ch(typeV3.mid(1).toLatin1().data());

        t_frqObs* frqObs = 0;
        for (unsigned iFrq = 0; iFrq < obs._obs.size(); iFrq++) {
          if (obs._obs[iFrq]->_rnxType2ch == type2ch) {
            frqObs = obs._obs[iFrq];
            break;
          }
        }
        if (frqObs == 0) {
          frqObs = new t_frqObs;
          frqObs->_rnxType2ch = type2ch;
          obs._obs.push_back(frqObs);
        }

        switch( typeV3.toLatin1().data()[0] ) {
        case 'C':
          frqObs->_codeValid = true;
          frqObs->_code      = rnxObs.value;
          break;
        case 'L':
          frqObs->_phaseValid = true;
          frqObs->_phase      = rnxObs.value;
          frqObs->_slip       = (rnxObs.lli & 1);
          break;
        case 'D':
          frqObs->_dopplerValid = true;
          frqObs->_doppler      = rnxObs.value;
          break;
        case 'S':
          frqObs->_snrValid = true;
          frqObs->_snr      = rnxObs.value;
          break;
        }

        // Handle old-fashioned SNR values
        // -------------------------------
        if (rnxObs.snr != 0 && !frqObs->_snrValid) {
          frqObs->_snrValid = true;
          frqObs->_snr      = rnxObs.snr * 6.0 + 2.5;
        }
      }
    }
  }
}

// Tracking Mode Priorities
////////////////////////////////////////////////////////////////////////////
QStringList t_rnxObsFile::signalPriorities(char sys) {

  bncSettings settings;

  QStringList priorList;
  QString reqcAction = settings.value("reqcAction").toString();

  // Priorities in Edit/Concatenate (post processing) mode
  // ---------------------------------------------------
  if (reqcAction == "Edit/Concatenate") {
    priorList = settings.value("reqcV2Priority").toString().split(" ", QString::SkipEmptyParts);
  }

  // Priorities in real-time mode
  // ----------------------------
  else {
    priorList = settings.value("rnxV2Priority").toString().split(" ", QString::SkipEmptyParts);
  }

  QStringList result;
  for (int ii = 0; ii < priorList.size(); ii++) {
    if (priorList[ii].indexOf(":") != -1) {
      QStringList hlp = priorList[ii].split(":", QString::SkipEmptyParts);
      if (hlp.size() == 2 && hlp[0].length() == 1 && hlp[0][0] == sys) {
        result.append(hlp[1]);
      }
    }
    else {
      result.append(priorList[ii]);
    }
  }

  if (result.empty()) {
    switch (sys) {
      case 'G':
        result.append("12&PWCSLXYN");
        result.append("5&IQX");
        break;
      case 'R':
        result.append("12&PC");
        result.append("3&IQX");
        break;
      case 'E':
        result.append("16&BCX");
        result.append("578&IQX");
        break;
      case 'J':
        result.append("1&SLXCZ");
        result.append("26&SLX");
        result.append("5&IQX");
        break;
      case 'C':
        result.append("IQX");
        break;
      case 'I':
        result.append("ABCX");
        break;
      case 'S':
        result.append("1&C");
        result.append("5&IQX");
        break;
    }
  }
  return result;
}
