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

#ifndef BNCMAP_H
#define BNCMAP_H

#include <QDialog>
#include <QWhatsThis>

class QwtPlot;
class QwtPlotZoomer;

class t_bncMap : public QDialog {
 Q_OBJECT
    
 public:
  t_bncMap(QWidget* parent = 0);
  ~t_bncMap();
   
 public slots:
  void slotNewPoint(const QString& name, double latDeg, double lonDeg);

 private slots:
  void slotClose();
  void slotPrint();
  void slotWhatsThis();

 protected:
  virtual void closeEvent(QCloseEvent *);
  virtual void showEvent(QShowEvent *);

 private:
  QwtPlot*       _mapPlot;
  QwtPlotZoomer* _mapPlotZoomer;
  QPushButton*   _buttonClose;
  QPushButton*   _buttonPrint;
  QPushButton*   _buttonWhatsThis;
  double         _minPointLat;
  double         _maxPointLat;
  double         _minPointLon;
  double         _maxPointLon;

};

#endif
