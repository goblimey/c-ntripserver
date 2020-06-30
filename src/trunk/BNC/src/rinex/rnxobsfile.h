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

#ifndef RNXOBSFILE_H
#define RNXOBSFILE_H

#include <QtCore>

#include <fstream>
#include <vector>
#include <map>

#include "newmat.h"
#include "bncconst.h"
#include "bnctime.h"
#include "t_prn.h"
#include "satObs.h"

#define defaultRnxObsVersion2 2.11
#define defaultRnxObsVersion3 3.04

class t_rnxObsHeader {

 friend class t_rnxObsFile;

 public:

  t_rnxObsHeader();
  ~t_rnxObsHeader();

  double      version() const {return _version;}
  t_irc       read(QTextStream* stream, int maxLines = 0);
  void        setDefault(const QString& markerName, int version);
  void        set(const t_rnxObsHeader& header, int version,
                  const QStringList* useObsTypes = 0, const QStringList* phaseShifts = 0,
                  const QStringList* gloBiases = 0, const QStringList* gloSlots = 0);
  int         numSys() const;
  char        system(int iSys) const;
  int         nTypes(char sys) const;
  int         numGloBiases() const;
  int         numGloSlots() const;
  QString     obsType(char sys, int index, double version = 0.0) const;
  QString     usedSystems() const;
  QStringList obsTypes(char sys) const;
  QStringList phaseShifts() const;
  QStringList gloBiases() const;
  QStringList gloSlots() const;
  void        write(QTextStream* stream, const QMap<QString, QString>* txtMap = 0) const;
  bncTime     startTime() const {return _startTime;}
  void        setStartTime(const bncTime& startTime) {_startTime = startTime;}

 private:
  QStringList obsTypesStrings() const;
  QString         _usedSystems;
  double          _version;
  double          _interval;
  QString         _antennaNumber;
  QString         _antennaName;
  QString         _markerName;
  QString         _markerNumber;
  QString         _markerType;
  QString         _observer;
  QString         _agency;
  QString         _receiverNumber;
  QString         _receiverType;
  QString         _receiverVersion;
  QStringList     _comments;
  ColumnVector    _antNEU;
  ColumnVector    _antXYZ;
  ColumnVector    _antBSG;
  ColumnVector    _xyz;
  QMap<char, QStringList> _obsTypes;
  QMap<t_prn, int>        _gloSlots;
  QMap<QString, double>   _gloBiases;
  int                     _wlFactorsL1[t_prn::MAXPRN_GPS+1];
  int                     _wlFactorsL2[t_prn::MAXPRN_GPS+1];
  bncTime                 _startTime;
  bool                    _writeRinexOnlyWithSklObsTypes;

  QMap<QString, QPair<double, QStringList> > _phaseShifts;
};

class t_rnxObsFile {
 public:

  static bool earlierStartTime(const t_rnxObsFile* file1, const t_rnxObsFile* file2) {
    return file1->startTime() < file2->startTime();
  }

  class t_rnxObs {
   public:
    t_rnxObs() {
      value = 0.0; lli = 0; snr = 0;
    }
    double value;
    int    lli;
    int    snr;
  };

  class t_rnxSat {
   public:
    t_prn                   prn;
    QMap<QString, t_rnxObs> obs;
    static bool prnSort(const t_rnxSat rnxSat1, const t_rnxSat rnxSat2) {return rnxSat1.prn < rnxSat2.prn;}
  };

  class t_rnxEpo {
   public:
    t_rnxEpo() {clear();}
    void clear() {
      tt.reset();
      rnxSat.clear();
    }
    bncTime               tt;
    std::vector<t_rnxSat> rnxSat;
  };

  enum e_inpOut {input, output};

  t_rnxObsFile(const QString& fileName, e_inpOut inpOut);
  ~t_rnxObsFile();

  double         version() const {return _header._version;}
  double         interval() const {return _header._interval;}
  int            numSys() const {return _header.numSys();}
  char           system(int iSys) const {return _header.system(iSys);}
  int            nTypes(char sys) const {return _header.nTypes(sys);}
  int            numGloBiases() const {return _header.numGloBiases();}
  int            numGloSlots() const {return _header.numGloSlots();}
  const QString& fileName() const {return _fileName;}
  QString obsType(char sys, int index, double version = 0.0) const {
    return _header.obsType(sys, index, version);
  }
  QStringList phaseShifts() const {return _header.phaseShifts();}
  QStringList gloBiases() const {return _header.gloBiases();}
  QStringList gloSlots() const {return _header.gloSlots();}
  const QString& antennaName() const {return _header._antennaName;}
  const QString& antennaNumber() const {return _header._antennaNumber;}
  const QString& markerName() const {return _header._markerName;}
  const QString& markerNumber() const {return _header._markerNumber;}
  const QString& receiverType() const {return _header._receiverType;}
  const QString& receiverNumber() const {return _header._receiverNumber;}

  void setInterval(double interval) {_header._interval = interval;}
  void setAntennaName(const QString& antennaName) {_header._antennaName = antennaName;}
  void setAntennaNumber(const QString& antennaNumber) {_header._antennaNumber = antennaNumber;}
  void setAntennaN(double antN) {_header._antNEU(1) = antN;}
  void setAntennaE(double antE) {_header._antNEU(2) = antE;}
  void setAntennaU(double antU) {_header._antNEU(3) = antU;}

  void setMarkerName(const QString& markerName) {_header._markerName = markerName;}
  void setReceiverType(const QString& receiverType) {_header._receiverType = receiverType;}
  void setReceiverNumber(const QString& receiverNumber) {_header._receiverNumber = receiverNumber;}

  const ColumnVector& xyz() const {return _header._xyz;}
  const ColumnVector& antNEU() const {return _header._antNEU;}
  const ColumnVector& antXYZ() const {return _header._antXYZ;}
  const ColumnVector& antBSG() const {return _header._antBSG;}

  const bncTime&      startTime() const {return _header._startTime;}
  void  setStartTime(const bncTime& startTime) {_header._startTime = startTime;}

  t_rnxEpo* nextEpoch();

  int wlFactorL1(unsigned iPrn) {
    return iPrn <= t_prn::MAXPRN_GPS ? _header._wlFactorsL1[iPrn] : 1;
  }
  int wlFactorL2(unsigned iPrn) {
    return iPrn <= t_prn::MAXPRN_GPS ? _header._wlFactorsL2[iPrn] : 1;
  }

  const t_rnxObsHeader& header() const {return _header;}

  void setHeader(const t_rnxObsHeader& header, int version,
      const QStringList* useObsTypes = 0, const QStringList* phaseShifts = 0,
      const QStringList* gloBiases = 0, const QStringList* gloSlots = 0) {
    _header.set(header, version, useObsTypes, phaseShifts, gloBiases, gloSlots);
  }

  void writeEpoch(const t_rnxEpo* epo);

  QTextStream* stream() {return _stream;}

  static void setObsFromRnx(const t_rnxObsFile* rnxObsFile, const t_rnxObsFile::t_rnxEpo* epo,
                            const t_rnxObsFile::t_rnxSat& rnxSat, t_satObs& obs);

  static QString type2to3(char sys, const QString& typeV2);
  static QString type3to2(char sys, const QString& typeV3);
  static QStringList signalPriorities(char sys);

  static void writeEpoch(QTextStream* stream, const t_rnxObsHeader& header, const t_rnxEpo* epo) {
    if (epo == 0) {
      return;
    }
    t_rnxEpo epoLocal;
    epoLocal.tt = epo->tt;
    for (unsigned ii = 0; ii < epo->rnxSat.size(); ii++) {
      const t_rnxSat& rnxSat = epo->rnxSat[ii];
      if (header._obsTypes[rnxSat.prn.system()].size() > 0) {
        if (header.version() < 3.0) { // exclude new GNSS such as BDS, QZSS, IRNSS, etc.
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

    if (header.version() >= 3.0) {
      writeEpochV3(stream, header, &epoLocal);
    }
    else {
      writeEpochV2(stream, header, &epoLocal);
    }
  }

 private:
  static void writeEpochV2(QTextStream* stream, const t_rnxObsHeader& header, const t_rnxEpo* epo);
  static void writeEpochV3(QTextStream* stream, const t_rnxObsHeader& header, const t_rnxEpo* epo);
  t_rnxObsFile() {};
  void openRead(const QString& fileName);
  void openWrite(const QString& fileName);
  void close();
  t_rnxEpo* nextEpochV2();
  t_rnxEpo* nextEpochV3();
  void handleEpochFlag(int flag, const QString& line, bool& headerReRead);

  e_inpOut       _inpOut;
  QFile*         _file;
  QString        _fileName;
  QTextStream*   _stream;
  t_rnxObsHeader _header;
  t_rnxEpo       _currEpo;
  bool           _flgPowerFail;
};

#endif
