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

#ifndef REQCEDIT_H
#define REQCEDIT_H

#include <QtCore>
#include "rnxobsfile.h"
#include "rnxnavfile.h"
#include "ephemeris.h"

class t_reqcEdit : public QThread {
Q_OBJECT

 public:
  t_reqcEdit(QObject* parent);

 protected:
  ~t_reqcEdit();

 signals:
  void finished();

 public slots:

 public:
  virtual void run();
  static void initRnxObsFiles(const QStringList& obsFileNames,
                              QVector<t_rnxObsFile*>& rnxObsFiles,
                              QTextStream* log);
  static void readEphemerides(const QStringList& navFileNames,
                              QVector<t_eph*>& ephs);
  static void appendEphemerides(const QString& fileName, QVector<t_eph*>& ephs);

 private:
  void editObservations();
  void editEphemerides();
  void editRnxObsHeader(t_rnxObsFile& obsFile);
  void rememberLLI(const t_rnxObsFile* obsFile, const t_rnxObsFile::t_rnxEpo* epo);
  void applyLLI(const t_rnxObsFile* obsFile, t_rnxObsFile::t_rnxEpo* epo);
  void addRnxConversionDetails(const t_rnxObsFile* obsFile, QMap<QString, QString>& txtMap);

  QString                _logFileName;
  QFile*                 _logFile;
  QTextStream*           _log;
  QStringList            _obsFileNames;
  QVector<t_rnxObsFile*> _rnxObsFiles;
  QString                _outObsFileName;
  QStringList            _navFileNames;
  QString                _outNavFileName;
  double                 _rnxVersion;
  double                 _samplingRate;
  bncTime                _begTime;
  bncTime                _endTime;
  QMap<QString, QMap<int, int> > _lli;
  QVector<t_eph*>        _ephs;
};

#endif
