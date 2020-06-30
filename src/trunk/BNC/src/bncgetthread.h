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

#ifndef BNCGETTHREAD_H
#define BNCGETTHREAD_H

#include <QThread>
#include <QDateTime>
#include <QFile>
#include <QTcpServer>
#include <QUrl>

#include "bncconst.h"
#include "bncnetquery.h"
#include "bnctime.h"
#include "bncrawfile.h"
#include "satObs.h"
#include "rinex/rnxobsfile.h"

class GPSDecoder;
class QextSerialPort;
class latencyChecker;

class bncGetThread : public QThread {
 Q_OBJECT

 public:
   bncGetThread(bncRawFile* rawFile);
   bncGetThread(const QUrl& mountPoint,
                const QByteArray& format,
                const QByteArray& latitude,
                const QByteArray& longitude,
                const QByteArray& nmea,
                const QByteArray& ntripVersion);

   bncNetQuery::queryStatus queryStatus() {
     if (_query) {
       return _query->status();
     }
     else {
       return bncNetQuery::init;
     }
   }

 protected:
   ~bncGetThread();

 public:
   void terminate();

   QByteArray staID() const {return _staID;}
   QUrl       mountPoint() const {return _mountPoint;}
   QByteArray latitude() const {return _latitude;}
   QByteArray longitude() const {return _longitude;}
   QByteArray ntripVersion() const {return _ntripVersion;}

 signals:
   void newBytes(QByteArray staID, double nbyte);
   void newRawData(QByteArray staID, QByteArray data);
   void newLatency(QByteArray staID, double clate);
   void newObs(QByteArray staID, QList<t_satObs> obsList);
   void newAntCrd(QByteArray staID, double xx, double yy, double zz,
                  double hh, QByteArray antType);
   void newMessage(QByteArray msg, bool showOnScreen);
   void newRTCMMessage(QByteArray staID, int msgID);
   void getThreadFinished(QByteArray staID);

 public:
   virtual void run();

 public slots:
   void slotNewNMEAstr(QByteArray staID, QByteArray str);

 private slots:
   void slotSerialReadyRead();
   void slotNewNMEAConnection();

 private:
   enum t_serialNMEA {NO_NMEA, MANUAL_NMEA, AUTO_NMEA};
   t_irc        initDecoder();
   GPSDecoder* decoder();

   void  initialize();
   t_irc tryReconnect();
   void  miscScanRTCM();

   QMap<QString, GPSDecoder*> _decodersRaw;
   GPSDecoder*                _decoder;
   bncNetQuery*               _query;
   QUrl                       _mountPoint;
   QByteArray                 _staID;
   QByteArray                 _format;
   QByteArray                 _latitude;
   QByteArray                 _longitude;
   QByteArray                 _height;
   QByteArray                 _nmea;
   QByteArray                 _ntripVersion;
   QByteArray                 _manualNMEAString;
   QDateTime                  _lastManualNMEA;
   int                        _manualNMEASampl;
   int                        _nextSleep;
   int                        _iMount;
   int                        _ssrEpoch;
   bncRawFile*                _rawFile;
   QextSerialPort*            _serialPort;
   bool                       _isToBeDeleted;
   bool obs;
   bool ssrOrb, ssrClk, ssrOrbClk;
   bool ssrCbi, ssrPbi;
   bool ssrVtec;
   bool ssrUra;
   bool ssrHr;
   latencyChecker*            _latencyChecker;
   QString                    _miscMount;
   QFile*                     _serialOutFile;
   t_serialNMEA               _serialNMEA;
   bool                       _rawOutput;
   bool                       _latencycheck;
   QMap<QString, bncTime>     _prnLastEpo;
   QMap<char, QVector<QString> > _rnxTypes;
   QStringList                _gloSlots;
   QList<QTcpSocket*>*        _nmeaSockets;
   QMap<QByteArray, int>      _nmeaPortsMap;
   QTcpServer*                _nmeaServer;
};

#endif
