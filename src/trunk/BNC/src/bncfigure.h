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

#ifndef BNCFIGURE_H
#define BNCFIGURE_H

#include <QByteArray>
#include <QMap>
#include <QMutex>
#include <QWidget>

class bncFigure : public QWidget {
  Q_OBJECT
 public:
  bncFigure(QWidget *parent);
  ~bncFigure();
  void updateMountPoints();
 public slots:
  void slotNewData(const QByteArray staID, double nbyte);
 protected:
  void paintEvent(QPaintEvent *event);
 private slots:
  void slotNextAnimationFrame();
 private:
  class sumAndMean {
   public:
    sumAndMean() {_mean = 0.0; _sum = 0.0;}
    ~sumAndMean() {}
    double _mean;
    double _sum;
  };
  QMap<QByteArray, sumAndMean*> _bytes;
  QMutex                        _mutex;
  int                           _counter;
  double                        _maxRate;
  int                           _ran[3][1001];
};

#endif
