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

#ifndef BNCCASTER_H
#define BNCCASTER_H

#include <QFile>
#include <QMultiMap>
#include <QTcpServer>
#include <QTcpSocket>
#include <QNetworkProxy>

#include "satObs.h"

class bncGetThread;

class bncCaster : public QObject {
 Q_OBJECT

 public:
   bncCaster();
   ~bncCaster();
   void addGetThread(bncGetThread* getThread, bool noNewThread = false);
   int  numStations() const {return _staIDs.size();}
   void readMountPoints();

 public slots:
   void slotNewObs(QByteArray staID, QList<t_satObs> obsList);
   void slotNewRawData(QByteArray staID, QByteArray data);
   void slotNewMiscConnection();

 signals:
   void mountPointsRead(QList<bncGetThread*>);
   void getThreadsFinished();
   void newMessage(QByteArray msg, bool showOnScreen);
   void newObs(QByteArray staID, QList<t_satObs> obsList);

   private slots:
   void slotReadMountPoints();
   void slotNewConnection();
   void slotNewUConnection();
   void slotGetThreadFinished(QByteArray staID);

 private:
   void dumpEpochs(const bncTime& maxTime);
   static int myWrite(QTcpSocket* sock, const char* buf, int bufLen);
   void reopenOutFile();

   QFile*                          _outFile;
   QTextStream*                    _out;
   QMap<bncTime, QList<t_satObs> > _epochs;
   bncTime                         _lastDumpTime;
   QTcpServer*                     _server;
   QTcpServer*                     _uServer;
   QList<QTcpSocket*>*             _sockets;
   QList<QTcpSocket*>*             _uSockets;
   QList<QByteArray>               _staIDs;
   QList<bncGetThread*>            _threads;
   bool                            _outLockTime;
   int                             _samplingRateMult10;
   int                             _outWait;
   QMutex                          _mutex;
   int                             _confInterval;
   QString                         _miscMount;
   int                             _miscPort;
   QTcpServer*                     _miscServer;
   QList<QTcpSocket*>*             _miscSockets;
};

#endif
