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

#ifndef GRAPHWIN_H
#define GRAPHWIN_H

#include <QByteArray>
#include <QDialog>
#include <QPrinter>
#include <QPrintDialog>
#include <qwt_color_map.h>
#include <qwt_plot.h>

class QwtScaleWidget;

//
//////////////////////////////////////////////////////////////////////////////
class t_colorMap: public QwtLinearColorMap {
 public:
  t_colorMap() : QwtLinearColorMap(Qt::darkBlue, Qt::yellow) {
    addColorStop(0.00, Qt::blue);
    addColorStop(0.25, Qt::cyan);
    addColorStop(0.50, Qt::green);
    addColorStop(0.75, Qt::red);
    addColorStop(1.00, Qt::yellow);
  }
};

//
//////////////////////////////////////////////////////////////////////////////
class t_graphWin : public QDialog {

 Q_OBJECT

 public:
  t_graphWin(QWidget* parent, const QString& fileName, const QVector<QWidget*>& plots,
             bool specialLayout, const QByteArray* scaleTitle = 0,
             const QwtInterval* scaleInterval = 0, const QVector<int>* rows = 0);
  ~t_graphWin();

  void savePNG(const QString& dirName, QByteArray ext);

 signals:

 private slots:
  void slotClose();
  void slotPrint();

 protected:
  virtual void closeEvent(QCloseEvent *);

 private:
  QString         _fileName;
  QWidget*        _canvas;
  QPushButton*    _buttonClose;
  QPushButton*    _buttonPrint;
  QwtScaleWidget* _colorScale;
};

#endif
